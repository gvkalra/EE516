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

#define FILENAME "task01.dat"
#define HARDLINK_SUFFIX "-hardlink"
#define SOFTLINK_SUFFIX "-softlink"

static void
print_stat(int fd)
{
	struct stat sb;
	char proc_fd[128] = {'\0',},
		link_value[1024] = {'\0',};
	ssize_t bytes_read;
	int ret;

	/* read file stat */
	ret = fstat(fd, &sb);
	if (ret != 0) {
		err("fstat() failed: [%s]", strerror(errno));
		return;
	}

	/* construct fd path from /proc */
	ret = snprintf(proc_fd, sizeof(proc_fd), "/proc/self/fd/%u", fd);
	if (ret >= sizeof(proc_fd)) {
		err("proc_fd buffer overflow");
		return;
	}

	/* read symlink */
	bytes_read = readlink(proc_fd, link_value, sizeof(link_value));
	if (bytes_read == -1) {
		err("readlink() failed: [%s]", strerror(errno));
		/* print UNKNOWN */
		(void)snprintf(link_value, sizeof(link_value), "UNKNOWN");
	} else {
		/* NULL terminate */
		link_value[bytes_read] = '\0';
		info("[7] readlink() : success => %s", link_value);
	}

	info("[8] fstat() => %s", link_value);
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

	/* list names & inode numbers */
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

static void
permission_play(int fd)
{
	/* owner can read, write only */
	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) {
		err("fchmod() failed: [%s]", strerror(errno));
		return;
	} else {
		info("[3] fchmod() : success => S_IRUSR | S_IWUSR");
	}
	print_stat(fd);

	/* restore permission */
	if (fchmod(fd, S_IRWXU) != 0) {
		err("fchmod() failed: [%s]", strerror(errno));
		return;
	} else {
		info("[3] fchmod() : success => restored");
	}
	print_stat(fd);

	/* donate to first non-root user */
	if (fchown(fd, 1000, 1000) != 0) {
		err("fchown() failed: [%s]", strerror(errno));
		return;
	} else {
		info("[4] fchown() : success => donated to user");
	}
	print_stat(fd);

	/* get back ownership */
	if (fchown(fd, 0, 0) != 0) {
		err("fchown() failed: [%s]", strerror(errno));
		return;
	} else {
		info("[4] fchown() : success => restored");
	}
	print_stat(fd);
}

static int
create_links(char *cwd_path)
{
	int soft_link = 0, hard_link = 0;

	/* hardlink FILENAME */
	if (link(FILENAME, FILENAME HARDLINK_SUFFIX) != 0) {
        err("link() failed: [%s]", strerror(errno));
        goto error;
    } else {
        info("[5] link() : success => %s", FILENAME HARDLINK_SUFFIX);
        hard_link = 1;
    }

    /* softlink FILENAME */
    if (symlink(FILENAME, FILENAME SOFTLINK_SUFFIX) != 0) {
        err("symlink() failed: [%s]", strerror(errno));
        goto error;
    } else {
        info("[6] symlink : success => %s", FILENAME SOFTLINK_SUFFIX);
        soft_link = 1;
    }

    /* list files */
    list_files(cwd_path);
	return 0;

error:
	/* free links on error */
	if (hard_link) {
		if (unlink(FILENAME HARDLINK_SUFFIX)) {
			err("unlink() failed [%s]", strerror(errno));
		} else {
			info("[2] unlink() : success => %s", FILENAME HARDLINK_SUFFIX);
		}
	}

	if (soft_link) {
		if (unlink(FILENAME SOFTLINK_SUFFIX)) {
			err("unlink() failed: [%s]", strerror(errno));
		} else {
			info("[2] unlink() : success => %s", FILENAME SOFTLINK_SUFFIX);
		}
	}

	return 1;
}

static void
links_play(int fd, char *cwd_path)
{
	int _fd = fd;

	/* free cwd_path */
	if (cwd_path != NULL)
		free(cwd_path);

	/* free file resources */
	if (_fd != -1) {
		/* close file */
		if (close(_fd) == -1) {
			err("close() failed: [%s]", strerror(errno));
		}
		/* unlink */
		if (unlink(FILENAME)) {
			err("unlink() failed: [%s]", strerror(errno));
		} else {
			info("[2] unlink() : success => %s", FILENAME);
		}
	}

	/* at this point we have unlinked() FILENAME
	 * however, hard & soft links still exist.
	 * let's try to lseek() using them
	 */
	_fd = open(FILENAME HARDLINK_SUFFIX, O_RDONLY);
	if (_fd == -1) {
		err("open() failed: [%s]", strerror(errno));
		/* skip seeking */
	} else {
		if (lseek(_fd, 0, SEEK_END) == (off_t)-1) {
			err("lseek() failed: [%s]", strerror(errno));
		}
		info("[1] lseek() : success => %s", FILENAME HARDLINK_SUFFIX);
		close(_fd);
	}

	_fd = open(FILENAME SOFTLINK_SUFFIX, O_RDONLY);
	if (_fd == -1) {
		err("open() failed: [%s]", strerror(errno));
		/* skip seeking */
	} else {
		if (lseek(_fd, 0, SEEK_END) == (off_t)-1) {
			err("lseek() failed: [%s]", strerror(errno));
		}
		info("[1] lseek() : success => %s", FILENAME SOFTLINK_SUFFIX);
		close(_fd);
	}

	/* free links */
	if (unlink(FILENAME HARDLINK_SUFFIX)) {
		err("unlink() failed [%s]", strerror(errno));
	} else {
		info("[2] unlink() : success => %s", FILENAME HARDLINK_SUFFIX);
	}

	if (unlink(FILENAME SOFTLINK_SUFFIX)) {
		err("unlink() failed: [%s]", strerror(errno));
	} else {
		info("[2] unlink() : success => %s", FILENAME SOFTLINK_SUFFIX);
	}
}

int main(int argc, const char *argv[])
{
	int fd = -1, links_exist = 0;
	char *cwd_path = NULL;

	/* Find current working directory
	 * an extension to the POSIX.1-2001 standard, glibc's getcwd()
	 * allocates the buffer dynamically using malloc if buf is NULL
	 */
	cwd_path = getcwd(NULL, 0);
	if (cwd_path == NULL) {
		err("getcwd() failed: [%s]", strerror(errno));
		goto error;
	}
    info("[10] getcwd() : %s", cwd_path);

    info("\n##### Creating %s #####\n", FILENAME);

    /* create task01.dat in current directory */
    fd = open(FILENAME, O_WRONLY | O_CREAT | O_EXCL, S_IRWXU);
    if (fd == -1) {
        err("open() failed: [%s]", strerror(errno));
        goto error;
    }

    /* list files in cwd_path */
    list_files(cwd_path);
    /* print statistics of newly created file */
    print_stat(fd);

    info("\n##### Permission Play #####\n");
    permission_play(fd);

    info("\n##### Linking Play #####\n");
    links_exist = !create_links(cwd_path);

    info("\n##### Seeking/Linking Play #####\n");
    if (links_exist)
    	links_play(fd, cwd_path);

    return 0;

error:
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
			info("[2] unlink() : success => %s", FILENAME);
		}
	}

	return 1;
}