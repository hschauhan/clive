/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@login.dkuug.dk> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 */

#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <md5.h>

/* ARGSUSED */
char *
MD5End(MD5_CTX *ctx, char *buf)
{
	int i;
	u_int8_t digest[MD5_DIGEST_LENGTH];
	static const char hex[] = "0123456789abcdef";

	if (buf == NULL && (buf = malloc(MD5_DIGEST_STRING_LENGTH)) == NULL)
		return (NULL);

	MD5Final(digest, ctx);
	for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
		buf[i + i] = hex[digest[i] >> 4];
		buf[i + i + 1] = hex[digest[i] & 0x0f];
	}
	buf[i + i] = '\0';
	memset(digest, 0, sizeof(digest));
	return (buf);
}

char *
MD5File(char *filename, char *buf)
{
	u_char buffer[BUFSIZ];
	MD5_CTX ctx;
	int fd, num, save_errno;

	MD5Init(&ctx);

	if ((fd = open(filename, O_RDONLY)) < 0)
		return (NULL);

	while ((num = read(fd, buffer, sizeof(buffer))) > 0)
		MD5Update(&ctx, buffer, num);

	save_errno = errno;
	close(fd);
	errno = save_errno;
	return (num < 0 ? NULL : MD5End(&ctx, buf));
}

char *
MD5Data(const u_char *data, size_t len, char *buf)
{
	MD5_CTX ctx;

	MD5Init(&ctx);
	MD5Update(&ctx, data, len);
	return (MD5End(&ctx, buf));
}
