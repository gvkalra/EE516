#include "params.h"
#include "buffer.h"
#include "log.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

/* Chunks
 * Each chunk_array[] element has 4KB data
 */
struct chunk_data {
    unsigned char data[4 * 1024]; //4KB
};
static struct chunk_data chunk_array[1280]; //5MB (1280 * 4KB)

// Linked List, Queue to manage eviction
struct eviction_node {
    struct eviction_node *next;
    struct eviction_node *prev;

    int fd; //file descriptor
    off_t offset; //offset
    int flags; //to find flags with which file was opened
    unsigned int chunk_index; //index of cached data in chunk_array
};
struct eviction_queue {
    unsigned int occupied_chunks; // occupied chunks
    unsigned int total_chunks; // total created chunks
    unsigned int max_chunks; // max possible chunks
    struct eviction_node *front, *rear;
};

// initialize queue
static struct eviction_queue evic_queue = {
    .occupied_chunks = 0,
    .total_chunks = 0,
    .max_chunks = 1280, // 0 to 1279
    .front = NULL,
    .rear = NULL,
};

// returns 1 if eviction queue can be expanded
static int is_evic_queue_expandable
(void)
{
    /*
     * An expandable queue implies that max_chunks
     * have not been utilized yet. At first, we expand the queue
     * as much as possible, after which we follow eviction algorithm.
     * This greatly helps in increasing the performance of high load
     * applications e.g. filesystem benchmarking.
     */
    return evic_queue.total_chunks < evic_queue.max_chunks;
}

// returns 1 if eviction queue is full
static int is_evic_queue_full
(void)
{
    /*
     * A full queue implies that all elements of a queue
     * point to a valid cache array element & that the queue
     * is no longer expandable. In this case, the only way to
     * cache data is to evict existing data.
     */
    return evic_queue.occupied_chunks == evic_queue.max_chunks;
}

// print the indexes of cache stored in eviction queue
static void print_eviction_queue
(void)
{
    struct eviction_node *iter = evic_queue.front;

    log_msg("\n");
    while (iter != NULL) {
        log_msg("%u -> ", iter->chunk_index);
        iter = iter->next;
    }
    log_msg("\n");
}

// allocates a new eviction node and adds it to front of queue
// chunk_index is monotonically increasing until max_chunks,
// after which, it is not possible to allocate more nodes.
static struct eviction_node *create_new_node
(int fd, off_t offset, int flags)
{
    struct eviction_node *node;
    unsigned int chunk_index;

    /* allocate memory */
    node = malloc(sizeof(struct eviction_node));
    if (node == NULL) {
        log_msg("ERROR: unable to allocate memory");
        return NULL;
    }

    /* assign chunk_index */
    chunk_index = evic_queue.total_chunks;
    if (chunk_index >= evic_queue.max_chunks) {
        free(node);
        return NULL; //all chunks are now utilized. we must evict
    }
    evic_queue.total_chunks += 1; // increment for next allocation

    /* initialize */
    node->next = node->prev = NULL;
    node->fd = fd;
    node->offset = offset;
    node->flags = flags;
    node->chunk_index = chunk_index;

    /* to track number of occupied chunks */
    evic_queue.occupied_chunks += 1;

    /* add to front of queue */
    node->next = evic_queue.front;
    node->prev = NULL; // redundant, keeping for clarity
    if (node->next)
        node->next->prev = node;
    evic_queue.front = node;
    if (evic_queue.rear == NULL) // first node?
        evic_queue.rear = node;

    return node;
}

// moves 'node' to front of queue
// this helps in maintaining aging information for LRU
// front node is most recently accessed, whereas tail node
// is least recently used
static void _move_node_to_front
(struct eviction_node *node)
{
    // if already at front, do nothing
    if (node == evic_queue.front)
        return;

    // unlink
    node->prev->next = node->next;
    if (node->next)
        node->next->prev = node->prev;

    // update rear if required
    if (node == evic_queue.rear) {
        evic_queue.rear = node->prev;
        evic_queue.rear->next = NULL;
    }

    // add to front
    node->next = evic_queue.front;
    node->prev = NULL;
    node->next->prev = node;

    // update queue
    evic_queue.front = node;
}

// flushes data pointed to by 'node' to disk
// and makes the memory available for reuse
static void _flush_node
(struct eviction_node *node)
{
#define RETRY_COUNT 2
    ssize_t bytes_written;
    int retry = 0;

    // sanity check
    if (node == NULL || node->fd == -1)
        return;

    if ((node->flags & O_ACCMODE) == O_RDONLY) {
        //no dirty data is possible since file was opened with
        // O_RDONLY
        goto skip_write;
    }

    do {
        // flush to disk
        bytes_written = pwrite(node->fd, chunk_array[node->chunk_index].data,
                            4096, node->offset); // request size fixed to 4KB

        // success
        if (bytes_written == 4096)
            break;

        // retry
        log_msg("ERROR : inconsistent write. Retrying...\n");
        retry++;
    } while (retry < RETRY_COUNT);

skip_write:
    // invalidate fd
    // so that it can be reused
    node->fd = -1;

    // inform queue about free chunk
    evic_queue.occupied_chunks -= 1;
#undef RETRY_COUNT
}

// flush fd to disk
static void _flush_fd
(int fd)
{
    struct eviction_node *iter = evic_queue.front;

    // for all elements in the queue,
    // flush cache of 'fd' in buffer to disk
    while (iter != NULL) {
        // matched
        if (iter->fd == fd) {
            _flush_node(iter);
        }

        // next
        iter = iter->next;
    }
}

// try writing to cache
// if cache hit, the node will be moved to front of queue
ssize_t _trywrite_cache
(int fd, const void *buf, size_t count, off_t offset)
{
    struct eviction_node *iter = evic_queue.front;
    ssize_t bytes_written = -1;

    while (iter != NULL) {
        // matched
        if (iter->fd == fd && iter->offset == offset) {
            // replace data in cache
            memcpy(chunk_array[iter->chunk_index].data, buf, count);
            bytes_written = count;
            break;
        }

        //next
        iter = iter->next;
    }

    // cache hit, move to front
    if (bytes_written != -1)
        _move_node_to_front(iter);

    return bytes_written;
}

// try reading from cache
// if cache hit, the node will be moved to front of queue
ssize_t _tryread_cache
(int fd, void *buf, size_t count, off_t offset)
{
    struct eviction_node *iter = evic_queue.front;
    ssize_t bytes_read = -1;

    while (iter != NULL) {
        // matched
        if (iter->fd == fd && iter->offset == offset) {
            // copy data from cache
            memcpy(buf, chunk_array[iter->chunk_index].data, count);
            bytes_read = count;
            break;
        }

        //next
        iter = iter->next;
    }

    // cache hit, move to front
    if (bytes_read != -1)
        _move_node_to_front(iter);

    return bytes_read;
}

// evicts a cached node according to eviction policy
// the returned node is the evicted node after flushing
// it's associated data to disk.
struct eviction_node *_evict_cached_node
(void)
{
    unsigned int evic_policy;
    evic_policy = BB_DATA->buf_policy;

    // LRU
    if (evic_policy == 2) {
        log_msg("LRU");
        _flush_node(evic_queue.rear);
        // increase occupancy count
        // it will lead to an invalid state if
        // this node is not used!!
        // that means after invoking this function,
        // the node MUST be utilized for either read or write!
        // otherwise the count will go out of sync
        evic_queue.occupied_chunks += 1;
        return evic_queue.rear;
    }
    // 1, default => random
    else {
        unsigned int rand_num;
        struct eviction_node *node;
        log_msg("Random Eviction");

        // assume node to evict is front node
        node = evic_queue.front;

        // find a random number between [0 - 1279]
        // at this point, it is okay to assume there are no
        // holes left inside the queue (fd = -1). And each
        // chunks is referred by one eviction_node.
        // This is true because at first we expand & utilize the queue
        // to it's maximum. We call eviction algorithm only when
        // there is not enough space left to accommodate more requests
        rand_num = rand() % evic_queue.max_chunks;

        // find node to be evicted
        while (rand_num > 0) {
            node = node->next;
            rand_num -= 1;
        }

        _flush_node(node);
        // increase occupancy count
        // it will lead to an invalid state if
        // this node is not used!!
        // that means after invoking this function,
        // the node MUST be utilized for either read or write!
        // otherwise the count will go out of sync
        evic_queue.occupied_chunks += 1;
        return node;
    }

    return NULL;
}

// tries to find an unused node
// i.e. a node not pointing to any valid chunk
// we flag nodes of this type with fd = '-1' (invalid)
// nodes with fd = -1 are like holes in linked list &
// can be utilized for new read/write requests
struct eviction_node *_find_usable_node
(void)
{
    struct eviction_node *iter = evic_queue.front;
    int found = 0;

    // for all elements in the queue,
    // find node with 'fd' = -1
    while (iter != NULL) {
        // matched
        if (iter->fd == -1) {
            found = 1;
            break;
        }

        // next
        iter = iter->next;
    }

    if (found) {
        // increase occupancy count
        // it will lead to an invalid state if
        // this node is not used!!
        // that means after invoking this function,
        // the node MUST be utilized for either read or write!
        // otherwise the count will go out of sync
        evic_queue.occupied_chunks += 1;
        return iter;
    }
    return NULL;
}

void buf_get_policy
(unsigned int *buf_policy)
{
    FILE *fp_conf;
    unsigned int _buf_policy;

    //sanity check
    if (buf_policy == NULL)
        return;

    // open file
    fp_conf = fopen("ee516.conf", "r");

    if (fp_conf != NULL) {
        int matched;

        // read keys
        matched = fscanf(fp_conf, "%*[^\n]\n%u", &_buf_policy);

        // ensure read correctly
        if (matched != 1)
            _buf_policy = 0;

        // close file
        fclose(fp_conf);
    }

    *buf_policy = _buf_policy;
}

/* Buffer hit:
 *    Return contents
 * Buffer miss:
 *    If buffer is full, evict some contents
 *    Read from disk and write to buffer
 */
ssize_t buf_read
(int fd, void *buf, size_t count, off_t offset, int flags)
{
#define RETRY_COUNT 2
    unsigned int evic_policy;
    ssize_t bytes_read;
    int retry;
    struct eviction_node *node = NULL;

    evic_policy = BB_DATA->buf_policy;
    if (evic_policy == 0) { //no buffer
        log_msg("No  buffer\n");
        return pread(fd, buf, count, offset);
    }

    // check buffer
    if (_tryread_cache(fd, buf, count, offset) == count) {
        log_msg("Cache HIT\n");
        return count;
    }

    log_msg("Cache MISS\n");

    // expand queue if possible
    if (is_evic_queue_expandable()) {
        log_msg("Expandable Cache\n");
        node = create_new_node(fd, offset, flags);
    }
    // buffer full, evict
    else if (is_evic_queue_full()) {
        log_msg("Eviction Cache\n");
        node = _evict_cached_node(); //returns evicted node
    }
    // buffer available, reuse
    else {
        log_msg("Re-usable Cache\n");
        node = _find_usable_node();
    }

    if (node == NULL) {
        log_msg("ERROR : unable to find usable memory...\n");
        return -1;
    } else {
        _move_node_to_front(node); //move to front
    }

    do {
        // read from disk
        bytes_read = pread(fd, buf, count, offset);

        // success
        if (bytes_read == count)
            break;

        // retry
        log_msg("ERROR : inconsistent read. Retrying...\n");
        retry++;
    } while (retry < RETRY_COUNT);

    // add to cache
    node->fd = fd;
    node->offset = offset;
    node->flags = flags;
    memcpy(chunk_array[node->chunk_index].data, buf, count);
    return count;
#undef RETRY_COUNT
}

/* Buffer hit:
 *     Write into the buffer
 * Buffer miss:
 *     If buffer is full, evict some contents
 *     Write into the buffer
 */
ssize_t buf_write
(int fd, const void *buf, size_t count, off_t offset, int flags)
{
#define RETRY_COUNT 2
    unsigned int evic_policy;
    ssize_t bytes_written;
    int retry;
    struct eviction_node *node = NULL;

    evic_policy = BB_DATA->buf_policy;
    if (evic_policy == 0) { //no buffer
        log_msg("No  buffer\n");
        return pwrite(fd, buf, count, offset);
    }

    // check buffer
    if (_trywrite_cache(fd, buf, count, offset) == count) {
        log_msg("Cache HIT\n");
        return count;
    }

    log_msg("Cache MISS\n");

    // expand queue if possible
    if (is_evic_queue_expandable()) {
        log_msg("Expandable Cache\n");
        node = create_new_node(fd, offset, flags);
    }
    // buffer full, evict
    else if (is_evic_queue_full()) {
        log_msg("Eviction Cache\n");
        node = _evict_cached_node(); //returns evicted node
    }
    // buffer available, reuse
    else {
        log_msg("Re-usable Cache\n");
        node = _find_usable_node();
    }

    if (node == NULL) {
        log_msg("ERROR : unable to find usable memory...\n");
        return -1;
    } else {
        _move_node_to_front(node); //move to front
    }

    do {
        // write to disk
        bytes_written = pwrite(fd, buf, count, offset);

        // success
        if (bytes_written == count)
            break;

        // retry
        log_msg("ERROR : inconsistent write. Retrying...\n");
        retry++;
    } while (retry < RETRY_COUNT);

    // add to cache
    node->fd = fd;
    node->offset = offset;
    node->flags = flags;
    memcpy(chunk_array[node->chunk_index].data, buf, count);
    return count;
#undef RETRY_COUNT
}

/*
 * If there are dirty data of corresponding file in the buffer,
 * write them to the disk
*/
int buf_close
(int fd)
{
    unsigned int evic_policy;

    evic_policy = BB_DATA->buf_policy;
    if (evic_policy == 0) { //no buffer
        log_msg("No  buffer\n");
        return close(fd);
    }

    // sanity check
    if (fd < 0)
        return -1;

    _flush_fd(fd);
    return close(fd);
}