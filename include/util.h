#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include "lj.h"

#define LJ_MAX_BUFFER 1024

#ifdef		HAVE_ICONV
#include <iconv.h>
extern iconv_t		utfenc,
       			utfdec;
#endif		/* HAVE_ICONV */

extern int		debug_level;

void *lj_calloc(size_t nmemb, size_t size);
void *lj_malloc(size_t size);
void *lj_realloc(void *ptr, size_t size);
void lj_free(void *ptr);

/* For determining the length of arrays containing pointers 
* the arrays must be terminated with a pointer to NULL
*/
size_t	lj_parraylen(const void * const *parray);

int lj_urlencode(const char *inbound, char **outbound);
int lj_urldecode(char *inbound);
int lj_chardecode(const char *inbound, char *outbound);
char *lj_strcopy(char *dest, const char *src);
int chomp(char *string);
int joinstring(char *string);
void lj_uniquename(char **name);
void lj_error(const char *msg, ...);
void lj_debug(int level, const char *msg, ...);
int lj_setauth(const lj_server * const serverinfo, lj_user *user);
int md5_hex(const char *value, char **result);

#endif /* UTIL_H */
