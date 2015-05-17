/*
 * Definitions for file descriptor passing.
 *
 * Sockets in the AF_LOCAL (formerly AF_UNIX) address family may be used
 * to pass file descriptors between processes on single system. It is
 * first necessary to create the sockets between which the file descritors
 * will be send, which may be done with socketpair(2):
 *
 * 	int fds[2];
 *
 * 	rc = socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
 * 	if (rc == -1)
 * 		// Handle error here
 *
 * Note that you can also use SOCK_DGRAM, though this hasn't been tested
 * with these functions.
 *
 * Sockets can also be created file socket files. These are created by
 * creating a socket with address family AF_LOCAL and then using bind(2)
 * to create an association between the socket and the Linux file namespace.
 * The address is specified by passing the path, in a struct sockaddr_un,
 * and the length of the path. Struct sockaddr_un is in the header file
 * sys/un.h.
 * (Untested).
 *
 * Once the socket has been created (or opened, if using socket type
 * files bound to a path on your filesystem, you can use the
 * sendfds() and recvfds() functions to send and receive file descriptors
 * on the socket file descriptor.
 */

#ifndef _PASSFDS_H_
#define _PASSFDS_H_

#include <stdlib.h>

/*
 * recvfds - receive one or more file descriptors on a local socket
 *
 * sockfd:	File descriptor for an open socket of address family AF_LOCAL.
 * fds:		A pointer to an array in which the received file descriptors
 *		will be stored. These values are local to the receiving
 *		task and will generally not be the same as the values send
 *		by the sendfds() call.
 * n:		The number of file descriptors in fds[].
 *
 * Returns: The number of file descriptors received, if successful. Otherwise,
 *	-1, with errno set to provide more information.
 */
extern ssize_t recvfds(int sockfd, int fds[], size_t n);

/*
 * sendfds - send one or more file descriptors on a local socket
 *
 * sockfd:	File descriptor for an open socket of address family AF_LOCAL.
 * fds:		A pointer to an array of open file descriptors.
 * n:		The number of file descriptors in fds[].
 *
 * Returns: The number of file descriptors sent, if successful. Otherwise,
 *	-1, with errno set to provide more information.
 */
extern ssize_t sendfds(int sockfd, int fds[], size_t n);
#endif /* _PASSFDS_H_ */
