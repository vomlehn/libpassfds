#include <sys/socket.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include <passfds-internal.h>

#define BUF_SIZE	80

/*
 * Start with functions that invoke a aystem call, printing an error message
 * and exiting if it failed. This is probably not the way errors would be
 * handled in a real program, but it makes understanding the test code
 * a lot simpler.
 */
static void do_close(int fd)
{
	int rc;

	rc = close(fd);
	if (rc == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
}

static void do_read(int fd, const char *who)
{
	char buf[BUF_SIZE];
	ssize_t rc;

	rc = read(fd, buf, sizeof(buf));
	if (rc == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	printf("%s read: %s\n", who, buf);
}

static void do_write(int fd, const char *who)
{
	char buf[BUF_SIZE];
	ssize_t rc;

	snprintf(buf, sizeof(buf), "message sent by %s", who);
	rc = write(fd, buf, sizeof(buf));
	if (rc == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}
}

static void do_pipe(int pipefd[2])
{
	int rc;

	rc = pipe(pipefd);
	if (rc == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}
}

/*
 * child() - child process from a fork()
 * sockfd:	File descriptor for a socket with family AF_LOCAL.
 *
 * This function creates two pipes (one used for sending data from the child
 * to the parent and one for dending data in the reverse directionz). The file
 * descriptors for parent to use, the read side of one pipe and the write side
 * of the other) are then send to the parent.  The child then writes a message
 * to the parent and reads a message from the parent.
 */
static int child(int sockfd)
{
	int to_parent[2], from_parent[2];
	int parent_fds[2];
	ssize_t rc;

	do_pipe(to_parent);
	do_pipe(from_parent);
	parent_fds[0] = to_parent[0];
	parent_fds[1] = from_parent[1];

	rc = sendfds(sockfd, parent_fds, ARRAY_SIZE(parent_fds));
	if (rc == -1) {
		perror("sendfds");
		exit(EXIT_FAILURE);
	}

	do_close(to_parent[0]);
	do_close(from_parent[1]);

	do_write(to_parent[1], "child");
	do_read(from_parent[0], "child");

	return EXIT_SUCCESS;
}

/*
 * parent() - parent process from a fork()
 * sockfd:	File descriptor for a socket with family AF_LOCAL.
 *
 * This is the parent form the fork. It reads two file descriptors from the
 * socket into two a two-element int array. The first element will be the
 * file descriptor from which a message will be read, The second is the
 * file descriptor to which a message will be written. Ther is nothing special
 * about the order, it just has to match the order intended by the sender.
 */
static int parent(int sockfd)
{
	int parent_fds[2];
	ssize_t rc;

	rc = recvfds(sockfd, parent_fds, ARRAY_SIZE(parent_fds));
	if (rc == -1) {
		perror("sendfds");
		exit(EXIT_FAILURE);
	}
	do_read(parent_fds[0], "parent");
	do_write(parent_fds[1], "parent");

	return EXIT_SUCCESS;
}

static int run()
{
	int sv[2];
	pid_t pid;
	int rc;

	/*
	 * Crete the socket with the local address family. We use type
	 * SOCK_STREAM, but could use a different socket type.
	 */
	rc = socketpair(AF_LOCAL, SOCK_STREAM, 0, sv);
	if (rc == -1) {
		perror("socketpair");
		exit(EXIT_FAILURE);
	}

	/* Create a child process with which to communicate. */
	pid = fork();

	switch (pid) {
	case -1:
		perror("fork");
		exit(EXIT_FAILURE);
		break;
	
	case 0:
		do_close(sv[0]);
		rc = child(sv[1]);
		exit(rc);
		break;
	
	default:
		do_close(sv[1]);
		rc = parent(sv[0]);
		break;
	}

	return rc;
}

static void usage(const char *name)
{
	fprintf(stderr, "usage: %s\n", name);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int opt;
	int n;
	int rc;

	/*
	 * No arguments or options supported, but it's good to verify none
	 * were accidentally supplied
	 */
	while ((opt = getopt(argc, argv, "")) != -1) {
		switch (opt) {
		default:
			usage(argv[0]);
			break;
		}
	}

	n = argc - optind;
	switch (n) {
	case 0:
		break;
	
	default:
		usage(argv[0]);
		break;
	}

	rc = run();
	return rc;
}
