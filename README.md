Library for passing file descriptors between processes on systems (like
Linux or UNIX) using SCM_RIGHTS with sendmsg/recvmsg.

Look at passfds.h for information on the interfaces and test.c for
an example of a client and server communicating over sockets passed
using this library.
