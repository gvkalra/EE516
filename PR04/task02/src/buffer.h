#pragma once

#include <unistd.h>

void buf_get_policy(unsigned int *buf_policy);

ssize_t buf_read(int fd, void *buf, size_t count, off_t offset, int flags);

ssize_t buf_write(int fd, const void *buf, size_t count, off_t offset, int flags);

int buf_close(int fd);
int buf_flush(int fd);