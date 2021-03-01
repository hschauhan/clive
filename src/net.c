#include "system.h"
#include "net.h"
#include "lj.h"
#include "hash.h"
#include "util.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <iconv.h>

FILE *
getconn(const char *host, int port)
{
	int			sockfd;
	struct hostent *	server;
	struct sockaddr_in	server_addr = { .sin_family = AF_INET };
	FILE *			stream;

	/*
	 * resolve the domain name
	 */
	h_errno = 0;		/* make sure we get *our* error */
	if ((server = gethostbyname(host)) == NULL)
	{
		herror("Error resolving hostname");
		return NULL;
	}
	/*
	 * next we make our socket, which is always a joy
	 */
	errno = 0;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Error creating socket");
		return NULL;
	}
	/*
	 * If we've gotten this far, we go ahead and make our sockaddr...
	 */
	server_addr.sin_port = htons(port);
	memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

	/*
	 * and then we connect!!
	 */
	errno = 0;
	if ((connect (sockfd, (struct sockaddr *)&server_addr,
	      sizeof(struct sockaddr)) < 0))
	{
		perror("Error connecting to host");
		return NULL;
	}

	/*
	 * Will return a FILE * value if successful, will return NULL if not
	 */
	if ((stream = fdopen(sockfd, "r+")))
		setlinebuf(stream);

	return stream;
}

void
closeconn(FILE *conn)
{
	int	sockfd;

	sockfd = fileno(conn);
	/* Until otherwise figured out, this is all we need to do */
	close(sockfd);
}

/* Function:	lj_send_request
 * Description:	sends a request to lj server and stores the result in hash
 */
void
lj_send_request(const lj_server * const serverinfo, const lj_user * const user, const char * const request, hashtable *lj_hash)
{
	FILE	*conn;
	char	key[LJ_BUFFER_MAX],
		value[LJ_BUFFER_MAX],
		decoded[LJ_BUFFER_MAX];

	if (!user->username || !user->password)
		lj_error("You must supply a username and password!\n");

	/* First we send our request */
	if (!(conn = getconn((serverinfo->hostname), (serverinfo->port))))
		lj_error("Error connecting to server!\n");

	lj_debug(1,
		"Sending to the server:\n"
		"POST %s HTTP/1.0\r\n"
		"Host: %s\r\n"
		"User-Agent: POSIX-Clive/" VERSION "\r\n"
		"Connection: close\r\n"
		"Content-type: application/x-www-form-urlencoded\r\n"
		"Content-length: %d\r\n\r\n%s",
		serverinfo->path,
		serverinfo->hostname,
		strlen(request),
		request);
	fprintf(conn,
		"POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: POSIX-Clive/" VERSION "\r\n"
		"Connection: close\r\n"
		"Content-type: application/x-www-form-urlencoded\r\n"
		"Content-length: %zu\r\n\r\n%s",
		serverinfo->path,
		serverinfo->hostname,
		strlen(request),
		request);
	fflush(conn);

	/*
	 * First we ditch the headers...if we ever wanted to do
	 * anything with the headers this is where we 'd put the code
	 * I'm using key here because it' s just as good as any other buffer
	 * I could set up a buffer just for this, but let's not waste memory
	 */
	while (fgets(key, LJ_BUFFER_MAX, conn) && strcmp(key, "\r\n"))
	{
		if (debug_level >= 3)
		{
			int             i = strlen(key);

			if (!strcmp(key + (i - 2), "\r\n"))
				key[i - 2] = '\0';
			lj_debug(3, "[%s]\n", key);
		}
	}

	/*
	 * Now we read in the key-value pairs.  We're making the
	 * assumption that neither the key nor the value will exceed
	 * 1024 bytes.  REVISIT THIS
	 */
	while (fgets(key, LJ_BUFFER_MAX, conn)
			&& fgets(value, LJ_BUFFER_MAX, conn))
	{
		chomp(key);
		chomp(value);
		lj_chardecode(value, decoded);
		lj_debug(1, "Putting [%s]->[%s]...\n", key, decoded);
		put(lj_hash, key, decoded);
	}

	closeconn(conn);
}
