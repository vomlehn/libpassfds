#include <sys/socket.h>
#include <string.h>

#include <passfds-internal.h>

/*
 * Function to send file descriptors over an AF_LOCAL socket
 */
ssize_t sendfds(int sockfd, int fds[], size_t n)
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

	/*
	 * Set up the message header. This is where the data and the
	 * ancillary data to be send are described.
	 */
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

	/*
	 * Set up the control message header. If we were sending more than
	 * one piece of ancillary data, we would use CMSG_NXTHDR() to
	 * get to subsequent pieces of data.
	 */
	cmsghdr = CMSG_FIRSTHDR(&msghdr);
	memset(cmsghdr, 0, sizeof(*cmsghdr));
	cmsghdr->cmsg_level = SOL_SOCKET;
	cmsghdr->cmsg_type = SCM_RIGHTS;
	cmsghdr->cmsg_len = CMSG_LEN(payload_size);

	/* Copy in the file descriptors */
	msg_fds = CMSG_DATA(cmsghdr);
	memcpy(msg_fds, fds, payload_size);

	/*
	 * Adjust length to be the sums of the lengths of all of the
	 * control messages. But there's only one message, so that's
	 * easy.
	 */
	msghdr.msg_controllen = cmsghdr->cmsg_len;

	rc = sendmsg(sockfd, &msghdr, sizeof(msghdr));
	if (rc != -1)
		rc = n;

	return rc;
}
