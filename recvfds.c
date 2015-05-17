#include <sys/socket.h>
#include <string.h>

#include <passfds-internal.h>

#define SET_IOV		1

ssize_t recvfds(int sockfd, int fds[], size_t n)
{
	struct msghdr msghdr;
	struct cmsghdr *cmsghdr;
	unsigned char *msg_fds;
	size_t payload_size = n * sizeof(fds[0]);
	char buf[CMSG_SPACE(payload_size)];
	ssize_t rc;
#if SEND_DATA
	struct iovec iov[1];
	char dummy[1];
#endif

	/* Now set up the message header */
	memset(&msghdr, 0, sizeof(msghdr));
	msghdr.msg_control = buf;
	msghdr.msg_controllen = sizeof(buf);

#if SEND_DATA
	/*
	 * Even though we only care about the ancillary data, we had to
	 * send at least a byte's worth of data.
	 */
	iov[0].iov_base = dummy;
	iov[0].iov_len = sizeof(dummy);
	msghdr.msg_iov = iov;
	msghdr.msg_iovlen = ARRAY_SIZE(iov);
#endif

	rc = recvmsg(sockfd, &msghdr, sizeof(msghdr));
	if (rc != -1) {
		rc = 0;

		for (cmsghdr = CMSG_FIRSTHDR(&msghdr); cmsghdr != NULL;
			cmsghdr = CMSG_NXTHDR(&msghdr, cmsghdr)) {
			if (cmsghdr->cmsg_level == SOL_SOCKET &&
				cmsghdr->cmsg_type == SCM_RIGHTS) {
				msg_fds = CMSG_DATA(cmsghdr);
				memcpy(fds, msg_fds, payload_size);
				rc = cmsghdr->cmsg_len / sizeof(fds[0]);
				break;
			}
		}
	}

	return rc;
}
