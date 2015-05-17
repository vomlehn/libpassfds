/*
 * Internal definitions for libpassfds.h
 */

#ifndef _LIBPASSFDS_H_
#define _LIBPASSFDS_H_

#include <passfds.h>

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))

/*
 * It appears as though at least a byte must be sent along with the
 * ancillary data or things don't work correctly. Setting this to
 * zero will allow trying without sending data
 */
#define SEND_DATA	1

#endif /* _LIBPASSFDS_H_ */
