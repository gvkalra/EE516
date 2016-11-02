#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

/* Make your own program using:
 * [1] lseek()    : reposition read/write file offset
 * [2] unlink()   : delete a name and possibly the file it refers to
 * [3] fchmod()   : change permissions of a file
 * [4] fchown()   : change ownership of a file
 * [5] link()     : make a new name for a file (hard link)
 * [6] symlink()  : make a new name for a file (soft link)
 * [7] readlink() : read value of a symbolic link
 * [8] fstat()    : get file status
 * [9] readdir()  : read a directory
 * [10] getcwd()  : get current working directory
*/

static void
print_stat(const char *name, int fd)
{
    struct stat sb;
    int ret;

    ret = fstat(fd, &sb);
    if (ret != 0) {
        err("fstat() failed: [%s]", strerror(errno));
        return;
    }

    info("[8] fstat() => %s", name);
    info("\tst_dev : %lu", sb.st_dev);
    info("\tst_ino : %lu", sb.st_ino);
    info("\tst_mode : %d", sb.st_mode);
    info("\tst_nlink : %lu", sb.st_nlink);
    info("\tst_uid : %d", sb.st_uid);
    info("\tst_gid : %d", sb.st_gid);
    info("\tst_rdev : %lu", sb.st_rdev);
    info("\tst_size : %ld", sb.st_size);
    info("\tst_blksize : %ld", sb.st_blksize);
    info("\tst_blocks : %ld", sb.st_blocks);
    info("\tst_atime : %ld", sb.st_atime);
    info("\tst_mtime : %ld", sb.st_mtime);
    info("\tst_ctime : %ld", sb.st_ctime);
}

static void
list_files(const char *dir_path)
{
    DIR *dir = NULL;
    struct dirent *r_dirent = NULL;

    /* open directory */
    dir = opendir(dir_path);
    if (dir == NULL) {
        err("opendir() failed: [%s]", strerror(errno));
        return;
    }

    /* list contents */
    info("[9] readdir() => d_ino (d_name)");
    errno = 0;
    while ((r_dirent = readdir(dir)) != NULL) {
        info("\t %ld (%s)", r_dirent->d_ino, r_dirent->d_name);
    };
    if (errno != 0)
        err("readdir() failed: [%s]", strerror(errno));

    /* close directory */
    if (closedir(dir) != 0)
        err("closedir() failed: [%s]", strerror(errno));
}

int main(int argc, const char *argv[])
{
#define FILENAME "task01.dat"
    int fd = -1, ret = 0;
    char *cwd_path = NULL;

    /* Find current working directory
     * an extension to the POSIX.1-2001 standard, glibc's getcwd()
     * allocates the buffer dynamically using malloc if buf is NULL
     * Ref: http://man7.org/linux/man-pages/man2/getcwd.2.html
     */
    cwd_path = getcwd(NULL, 0);
    if (cwd_path == NULL) {
        err("getcwd() failed: [%s]", strerror(errno));
        ret = 1;
        goto exit;
    }
    info("[10] getcwd() : %s", cwd_path);

    info("##### Creating %s #####", FILENAME);

    /* create task01.dat in current directory */
    fd = open(FILENAME, O_WRONLY | O_CREAT | O_EXCL,
            S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd == -1) {
        err("open() failed: [%s]", strerror(errno));
        ret = 1;
        goto exit;
    }

    list_files(cwd_path);
    print_stat(FILENAME, fd);

    info("##### Changing permissions #####");

    /* only USR can read & write now */
    if (fchmod(fd, S_IWUSR | S_IRUSR) != 0) {
        err("fchmod() failed: [%s]", strerror(errno));
        ret = 1;
        goto exit;
    } else {
        info("[3] fchmod() : success");
    }

    print_stat(FILENAME, fd);

    info("##### Changing owner to root #####");

    /* donate to root */
    if (fchown(fd, 0, 0) != 0) {
        err("fchown() failed: [%s]", strerror(errno));
        ret = 1;
        goto exit;
    } else {
        info("[4] fchown() : success => donated to root");
    }

    print_stat(FILENAME, fd);

    info("##### Reclaiming ownership #####");

    /* get back ownership : TODO */
    if (fchown(fd, 1000, 1000) != 0) {
        err("fchown() failed: [%s]", strerror(errno));
        ret = 1;
        goto exit;
    } else {
        info("[4] fchown() : success => ownership returned");
    }

    print_stat(FILENAME, fd);

exit:
    /* free cwd_path */
    if (cwd_path != NULL)
        free(cwd_path);

    /* free file resources */
    if (fd != -1) {
        /* close file */
        if (close(fd) == -1) {
            err("close() failed: [%s]", strerror(errno));
        }
        /* unlink */
       if (unlink(FILENAME)) {
            err("unlink() failed: [%s]", strerror(errno));
        } else {
            info("[2] unlink() : success");
        }
    }

    return ret;
#undef FILENAME
}
