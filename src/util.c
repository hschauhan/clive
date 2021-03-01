#include "system.h"
#include "util.h"
#include "configure.h"
#include "md5hl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <err.h>
#include <sys/types.h>

#if defined(DEBUG4)
int		debug_level = 4;
#elif defined(DEBUG3)
int		debug_level = 3;
#elif defined(DEBUG2)
int		debug_level = 2;
elif defined(DEBUG1) || defined(DEBUG)
int		debug_level = 1;
#else
int		debug_level = 0;
#endif
#ifdef		HAVE_ICONV
iconv_t		utfenc,
		utfdec;
#endif		/* HAVE_ICONV */

static char	hex2char(const char *hex);

void
lj_free(void *ptr)
{
	if (ptr)
		free(ptr);
}

void *
lj_calloc(size_t nmemb, size_t size)
{
	void		*ret = NULL;

	if ((ret = calloc(nmemb, size)) == NULL)
		lj_error("Memory allocation error\n");
	lj_debug(4, "calloc [%d] elements of size [%d] to [%p]\n",
		nmemb, size, ret);
	return ret;
}

/* Eventually we should find where we are overflowing our buffer...but for
 * now, we just add 10 bytes to whatever is requested
 */

void *
lj_malloc(size_t size)
{
	void		*ret = NULL;

	if (((ret = calloc(size, 1)) == NULL))
		lj_error("Memory allocation error\n");
	lj_debug(4, "malloc [%d] to [%p]\n", size, ret);
	return ret;
}

void *
lj_realloc(void *ptr, size_t size)
{
	void		*ret = NULL;

	if (!ptr)
	{
		return lj_malloc(size);
	}
	else if (!size)
	{
		lj_debug(4, "realloc freeing [%p]\n", ptr);
		lj_free(ptr);
		return NULL;
	}
	else
	{
		if (!(ret = realloc(ptr, size)))
			lj_error("Memory allocation error\n");
		lj_debug(4, "realloc [%d] bytes to [%p]\n", size, ret);
		return ret;
	}
	return NULL;
}

size_t 
lj_parraylen(const void * const * const parray)
{
	int             i;

	for (i = 0; parray[i] != NULL; i++)
		/* DO NOTHING */;
	return (size_t) i;
}

int
lj_urlencode(const char *inbound, char **outbound)
{
	char            *buffer;
	const char	*utf8;
	int             i, j, l;

	utf8 = inbound;
#ifdef		HAVE_ICONV
	{
		char	*outbuf, *p;
		size_t	inlen, outlen;
		int	sum;

		inlen = 1 + strlen(inbound);
		outlen = 4 * inlen;
		p = lj_malloc(outlen);
		outbuf = p;
		sum = iconv(utfenc, &inbound, &inlen, &outbuf, &outlen);
		if (-1 == sum)
		{
			perror("Cannot convert");
			free(p);
			return 1;
		}
		utf8 = p;
	}
#endif		/* HAVE_ICONV */

	buffer = lj_malloc(strlen(utf8) * 3 + 1);
	for (i = j = 0, l = strlen(utf8); i < l; i++)
	{
		if (((utf8[i] >= 'a') && (utf8[i] <= 'z'))
		    || ((utf8[i] >= 'A') && (utf8[i] <= 'Z'))
		    || ((utf8[i] >= '0') && (utf8[i] <= '9'))
		    || (utf8[i] < 0))
		{
			buffer[j] = utf8[i];
			j++;
		} else if (utf8[i] == ' ')
		{
			buffer[j] = '+';
			j++;
		} else
		{
			sprintf(&buffer[j], "%%%02X", utf8[i]);
			j += 3;
		}
	}
	buffer[j] = '\0';

	*outbound = NULL;
	*outbound = lj_realloc(*outbound, strlen(buffer) + 1);
	strcpy(*outbound, buffer);
	free(buffer);
	return 0;
}

int
lj_chardecode(const char *inbound, char *outbound)
{
#ifdef		HAVE_ICONV
		size_t		inlen = strlen(inbound) + 1,
				outlen = LJ_BUFFER_MAX,
				sum;
		const char	*inbuf = inbound;
		char		*outbuf = outbound;

		sum = iconv(utfdec, &inbuf, &inlen, &outbuf, &outlen);
		if ((size_t)-1 == sum)
		{
			lj_debug(1, "Conversion failed for [%s]\n", inbound);
			strncpy(outbound, inbound, LJ_BUFFER_MAX);
			outbound[LJ_BUFFER_MAX-1] = '\0';
			return 1;
		}
		else
		{
			lj_debug(4, "Converted value [%s] to [%s]\n",
				inbound, outbound);
			return 0;
		}
#endif		/* HAVE_ICONV */
		strncpy(outbound, inbound, LJ_BUFFER_MAX);
		outbound[LJ_BUFFER_MAX-1] = '\0';
		return 0;
}

/* hex2char
 * Takes a html encoded (two character) string as input - without the leading %
 * and returns a single decoded character (e.g. "41" -> 'A')
 */
static char
hex2char(const char * const hex)
{
	char		ch = 0;

	if (!hex[0] || !hex[1])
		return 0;
	if (hex[0] >= '0' && hex[0] <= '9')
		ch = hex[0] - '0';
	else if (hex[0] >= 'A' && hex[0] <= 'F')
		ch = 10 + hex[0] - 'A';
	else if (hex[0] >= 'a' && hex[0] <= 'f')
		ch = 10 + hex[0] - 'a';
	ch *= 16;
	if (hex[1] >= '0' && hex[1] <= '9')
		ch += hex[1] - '0';
	else if (hex[1] >= 'A' && hex[1] <= 'F')
		ch += 10 + hex[1] - 'A';
	else if (hex[1] >= 'a' && hex[1] <= 'f')
		ch += 10 + hex[1] - 'a';
	return ch;
}

/* urldecode
 * takes a url-encoded message as input (containing %XX or +)
 * returns the decoded version of this message
 * NB: the text is edited _in place_ (inpointer equals outpointer)
 */
int
lj_urldecode(char *message)
{
	char		buffer[LJ_BUFFER_MAX];
	size_t		i, len = strlen(message);

	for (i = 0; i < len; i++)
	{
		switch (message[i])
		{
		case '%':
			/* this potentionally does a LOT of memmoves() */
			message[i] = hex2char(&message[i] + 1);
			memmove(message + i + 1, message + i + 3, len - i);
			break;
		case '+':
			message[i] = ' ';
			break;
		}
	}

	if (lj_chardecode(message, buffer) == 0 && strlen(buffer) <= len)
		strcpy(message, buffer);
	return 0;
}

char *lj_strcopy(char *dest, const char * const src)
{
	/*
	 * NOTE: Must make sure that the string pointer has had the memory
	 * malloced to it already!
	 */
	/*
	 * ALSO NOTE: that if src is *not* a null terminated string (at least
	 * "\0") the behavior of this function is not predictable
	 */
	lj_debug(4, "strcopy [%s][%d] bytes to [%p]\n",
		src, strlen(src) + 1, dest);
	strcpy(dest, src);
	return dest + strlen(dest);
}

int
chomp(char *string)
{
	int             i;

	if ((string != NULL) && (strcmp(string, ""))
	    && ((i = strlen(string)) > 0))
	{
		if (string[i - 1] == '\n')
		{
			string[i - 1] = '\0';
			return 1;
		}
	}
	return 0;
}

int
joinstring(char *string)
{
	char		*nl;

	if (!string)
		return 0;
	for (nl = string; (nl = strchr(nl, '\n')); nl++)
	{
		if ('\n' == nl[1])
			nl++;
		else
			nl[0] = ' ';
	}
	return 1;
}

void
lj_uniquename(char **name)
{
	char		timestring[15];
	time_t          t;

	time(&t);
	strftime(timestring, 15, "%Y%m%d%H%M%S", localtime(&t));
	asprintf(name, "clive.%d.%s", getuid(), timestring);
}

void
lj_error(const char * const fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vwarnx(fmt, ap);
	va_end(ap);
	exit(1);
}

void
lj_debug(int level, const char * const msg, ...)
{
	va_list		ap;

	if (level > debug_level)
		return;
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
}

int
lj_setauth(const lj_server * const serverinfo, lj_user *user)
{
	char		*username, *password;

	if (!user->username || !user->password)
		lj_error("You must supply a username and password to login!\n");

	lj_urlencode(user->username, &username);
	if (user->auth)
	{
		free(user->auth);
		user->auth = NULL;
	}

	switch (user->auth_method)
	{
	case pwd_plain:
		lj_urlencode(user->password, &password);
		asprintf(&user->auth, "user=%s&password=%s&ver=1",
			username, password);
		break;

	case pwd_hash:
		md5_hex(user->password, &password);
		asprintf(&user->auth, "user=%s&hpassword=%s&ver=1",
			username, password);
		break;

	case pwd_challenge:
	default:
	{
		lj_challengeinfo	challenge;
		char			*temp;

		lj_getchallenge(serverinfo, user, &challenge);
		if (!challenge.success)
		{
			user->auth_method = pwd_hash;
			return lj_setauth(serverinfo, user);
		}
		md5_hex(user->password, &password);
		asprintf(&temp, "%s%s", challenge.challenge, password);
		free(password);
		md5_hex(temp, &password);
		free(temp);
		asprintf(&user->auth, "user=%s&auth_method=challenge"
			"&auth_challenge=%s&auth_response=%s&ver=1",
			username, challenge.challenge, password);
	}
	/* end switch */
	}
	free(username);
	free(password);
	lj_debug(2, "setup auth [%s]\n", user->auth);
	return 1;
}

int
md5_hex(const char *value, char **result)
{
	*result = MD5Data((const unsigned char *)value, strlen(value), NULL);

	lj_debug(2, "md5 hex of [%s] is [%s]\n", value, *result);
	return 1;
}
