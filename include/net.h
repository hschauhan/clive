#ifndef NET_H
#define NET_H

#include "lj.h"
#include "hash.h"

extern FILE *getconn(const char *host, int port);
extern void closeconn(FILE *conn);
extern void lj_send_request(const lj_server *serverinfo, const lj_user *user, const char *request, hashtable *lj_hash);

#endif
