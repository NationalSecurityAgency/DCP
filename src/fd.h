/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * todo write description for fd.h
 */
#ifndef FD_H__
#define FD_H__


/**
 * read all bytes from `src` and write them to `dest` using `buffer` of length
 * `blen` to store the bytes temporarily. Will continue to read from `src` until
 * EOF or error occurred.
 *
 * @param dest      fd to copy bytes to
 * @param src       fd to copy bytes from
 * @param buffer    memory to use for copying bytes, if NULL fd_pipe will
 *                  allocate its own
 * @param blen      size of buffer in bytes, ignored if buffer is NULL
 *
 * @return          0 on success, -1 on error with errno set
 */
int fd_pipe(int dest, int src, void *buffer, size_t blen);


/**
 * Read up to `len` bytes from `fd` into `dest`. This is a wrapper for the read
 * syscall handling recoverable error cases.
 *
 * @param fd            the file to read from
 * @param dest          buffer to write bytes to
 * @param len           up to this number of bytes will be written
 *
 * @return              the number of bytes read, -1 on error with `errno` set
 */
ssize_t fd_read(int fd, void *dest, size_t len);


/**
 * Read len bytes from file descriptor and put into destination buffer,
 * returning the number of bytes read. The only way for x, num bytes read, to
 * be "0 <= x < len" is if EOF was hit.
 *
 * @param fd            the file descriptor to read from
 * @param dest          the buffer to write the bytes read to
 * @param len           the number of bytes to read from fd
 *
 * @return              the number of bytes read from fd, on error -1 with errno
 *                      set.
 */
ssize_t fd_read_full(int fd, void *dest, size_t len);


/**
 * Guarantee `count` bytes are written by the kernel. The write syscall can
 * return after no bytes or some bytes from the buffer have been written to the
 * file. write_full handles these cases and returns either -1 if an
 * unrecoverable error is returned or returns the number of bytes written via
 * one or more write syscalls.
 *
 * @param fd        the file descriptor to write `count` bytes to from `buf`
 * @param buf       memory holding the data to write to `fd`
 * @param count     number of bytes in `buf` that must be written
 *
 * @return          `count` if success, -1 on error
 */
ssize_t fd_write_full(int fd, const void *buf, size_t count);


#endif
