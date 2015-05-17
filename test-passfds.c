#include <sys/socket.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include <passfds-internal.h>

#define BUF_SIZE	80

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
	do_write(to_parent[1], "child");
	do_read(from_parent[0], "child");

	return EXIT_SUCCESS;
}

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

	rc = socketpair(AF_LOCAL, SOCK_STREAM, 0, sv);
	if (rc == -1) {
		perror("socketpair");
		exit(EXIT_FAILURE);
	}

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
