#ifndef _MD5HL_H_
#define _MD5HL_H_

#include <sys/types.h>
#include "md5.h"

char * MD5End(MD5_CTX *ctx, char *buf);
char * MD5File(char *filename, char *buf);
char * MD5Data(const u_char *data, size_t len, char *buf);

#endif /* _MD5HL_H_ */
