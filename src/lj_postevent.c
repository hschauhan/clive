#include "system.h"
#include "lj.h"
#include "hash.h"
#include "net.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

char *create_post(char *auth,
		  char *event,
		  char *subject,
		  char *lineendings,
		  char *security,
		  char *allowmask,
		  char *usejournal,
		  int nocomments, int preformatted,
		  int backdated, const char dateback[13],
		  char *mood, int moodid, char *userpic, char *music,
		  char *location, char *taglist,
		  int replace);

/****************************************************************************
* This is a fairly simple routine.  It takes what we know and makes the
* parameter list that goes into the POST request.  We don't have to do tracking
* of the content length because of this.  This also magically leaves out
* parameters that we don't need to put in (ie subject and usejournal)
*/

int             lj_get_post_response(FILE * conn, lj_event_response * response);

/*****************************************************************************
* This function is called from within lj_postevent and determines the return
* information from doing the request
*/

int 
lj_get_post_response(FILE * conn, lj_event_response * response)
{
	char		buffer[LJ_BUFFER_MAX],
			key[LJ_BUFFER_MAX],
			value[LJ_BUFFER_MAX];
	hashtable	*lj_hash;
	char		*message;
	int		r = 0;

	response->success = 1;
	response->message = NULL;

	create_hash(&lj_hash);

	/*
         * For the response array
         * 0 - success ("OK", or "FAIL")
         * 1 - itemid (only if success is "OK")
         * 2 - errmsg (only if success is "FAIL")
         */

	/*
	 * First we ditch the headers...if we ever wanted to do anything with
	 * the headers this is where we'd put the code
	 */
	while (fgets(buffer, LJ_BUFFER_MAX, conn) && strcmp(buffer, "\r\n"))
	{
		if (debug_level >= 3)
		{
			int             i = strlen(buffer);

			if (!strcmp(buffer + (i - 2), "\r\n"))
			{
				buffer[i - 2] = '\0';
			}
			lj_debug(3, "[%s]\n", buffer);
		}
	}
	/*
	 * Now we read in the key-value pairs.  We're making the assumption
	 * that neither the key nor the value will exceed 1024.  REVISIT THIS
	 */
	while (fgets(key, LJ_BUFFER_MAX, conn) && fgets(value, LJ_BUFFER_MAX, conn))
	{
		chomp(key);
		chomp(value);
		lj_debug(3, "Putting [%s]->[%s]...", key, value);
		put(lj_hash, key, value);
	}

	/* first we test for success */
	get(lj_hash, &message, "success");
	if (!message || strcmp(message, "OK"))
	{
		response->success = 0;	/* we failed */
		r = 1;
	}

	/* next we look for an errmsg */
	get(lj_hash, &response->message, "errmsg");
	if (!response->message)
		get(lj_hash, &response->message, "itemid");
	lj_debug(1,
		"response->success: [%s]\n"
		"response->message: [%s]\n",
		response->success ? "OK" : "FAIL",
		response->message);
	return r;
}

int
lj_postevent(const lj_server * const serverinfo, lj_user * user,
	     lj_event * post, lj_event_response * response)
{
	char           *event,
	               *subject,
	               *usejournal,
	               *post_text,
	               *host,
	               *path,
	               *security,
	               *allowmask;
	int             port = 80;

	/*
	 * These are the strings that we don't support anything other than
	 * the defaults on.
	 */
	char           lineendings[] = "unix";
	FILE           *conn;
	int             r;

	subject = event = usejournal = post_text = host =
		path = security = allowmask = strdup("");

	lj_debug(2, "Populating serverinfo with the information passed in");

	/* get hostname */
	if ((serverinfo != NULL) && (serverinfo->hostname != NULL)
	    && strcmp(serverinfo->hostname, ""))
	{
		host = lj_malloc(strlen(serverinfo->hostname) + 1);
		strcpy(host, serverinfo->hostname);
	}
	lj_debug(2, "\thostname: [%s]\n", host);
	/* get port */
	if ((serverinfo != NULL))
	{
		port = serverinfo->port ? serverinfo->port : 80;
	}
	lj_debug(2, "\tport: [%d]\n", port);
	/* get path */
	if ((serverinfo != NULL) && (serverinfo->path != NULL)
	    && strcmp(serverinfo->path, ""))
	{
		path = lj_malloc(strlen(serverinfo->path) + 1);
		strcpy(path, serverinfo->path);
	}
	lj_debug(2, "\tpath: [%s]\n", path);

	lj_setauth(serverinfo, user);

	/* this is being changed over to a struct lj_event type */
	/* get event */
	if ((post != NULL) && (post->event != NULL)
	    && strcmp(post->event, ""))
	{
		event = lj_malloc(strlen(post->event) + 1);
		strcpy(event, post->event);
		lj_debug(2, "\tevent: [%s]\n", event);
	}
	else if ((post == NULL) || (!post->replace))
	{
		lj_error("I refuse to post an empty event!\n");
		/* NOTREACHED */
	}
	/* get subject */
	if ((post->subject != NULL) && strcmp(post->subject, ""))
	{
		subject = lj_malloc(strlen(post->subject) + 1);
		strcpy(subject, post->subject);
		lj_debug(2, "\tsubject: [%s]\n", subject);
	}
	/* get usejournal, if we have it */
	if ((post->usejournal != NULL) && strcmp(post->usejournal, ""))
	{
		usejournal = lj_malloc(strlen(post->usejournal) + 1);
		strcpy(usejournal, post->usejournal);
		lj_debug(2, "\tusejournal: [%s]\n", usejournal);
	}
	/* set security level */
	if ((post->security != NULL) && strcmp(post->security, ""))
	{
		security = lj_malloc(strlen(post->security) + 1);
		strcpy(security, post->security);
		lj_debug(2, "\tsecurity: [%s]\n", security);
	}
	/* allowmask is post[5] */
	if ((post->allowmask != NULL) && strcmp(post->allowmask, ""))
	{
		allowmask = lj_malloc(strlen(post->allowmask) + 1);
		strcpy(allowmask, post->allowmask);
		lj_debug(2, "\tallowmask: [%s]\n", allowmask);
	}

	/*
	 * Now we urlencode everything that needs to be
	 */
	if (event && strlen(event))
		lj_urlencode(event, &event);
	if (usejournal && strlen(usejournal))
		lj_urlencode(usejournal, &usejournal);
	if (subject && strlen(subject))
		lj_urlencode(subject, &subject);
	if (post->mood)
		lj_urlencode(post->mood, &(post->mood));
	if (post->userpic)
		lj_urlencode(post->userpic, &(post->userpic));
	if (post->music)
		lj_urlencode(post->music, &(post->music));
	if (post->location)
		lj_urlencode(post->location, &(post->location));
	if (post->taglist)
		lj_urlencode(post->taglist, &(post->taglist));

	post_text =
		create_post(user->auth, event, subject, lineendings, security,
			    allowmask, usejournal,
			    post->nocomments, post->preformatted,
		  post->backdated, post->dateback, post->mood, post->moodid,
			    post->userpic, post->music,
			    post->location, post->taglist,
			    post->replace);

	if (post_text == NULL)
	{
		perror("Error posting event");
		return 1;
	}
	lj_debug(1,
		"Printing the following to the socket:"
		"POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: POSIX-Clive/" VERSION "\r\n"
		"Connection: close\r\n"
		"Content-type: application/x-www-form-urlencoded\r\n",
		path, host);
	if (user->fastserver)
		lj_debug(1, "Cookie: ljfastserver=1\r\n", stdout);
	lj_debug(1, "Content-length: %d\r\n\r\n"
		"%s\n\n",
		strlen(post_text),
		post_text);

	if ((conn = getconn(host, port)) == NULL)
	{
		lj_error("Error connecting to server!\n");
		/* NOTREACHED */
	}


	fprintf(conn, "POST %s HTTP/1.1\r\n", path);
	fprintf(conn, "Host: %s\r\n", host);
	fputs("Connection: close\r\n"
		"User-Agent: POSIX-Clive/" VERSION "\r\n"
		"Content-type: application/x-www-form-urlencoded\r\n",
			conn);
	if (user && user->fastserver)
	{
		fputs("Cookie: ljfastserver=1\r\n", conn);
	}
	fprintf(conn, "Content-length: %zu\r\n\r\n", strlen(post_text));
	fprintf(conn, "%s", post_text);
	fputs("\r\n", conn);
	fflush(conn);
	r = lj_get_post_response(conn, response);
	closeconn(conn);
	return r;
}

char *
create_post(char *auth,
	    char *event,
	    char *subject,
	    char *lineendings,
	    char *security,
	    char *allowmask,
	    char *usejournal,
	    int nocomments, int preformatted,
	    int backdated, const char dateback[13],
	    char *mood, int moodid, char *userpic, char *music,
	    char *location, char *taglist,
	    int replace)
{
	char            year[5],
	                mon[3],
	                day[3],
	                hour[3],
	                min[3];
	struct tm	*ltime;
	time_t		timep;
	char		*ret;
	char		*replace_text;

	/*
	 * 132 is the magic number of characters that are *always going to be
	 * in this particular string and 10 is just to be safe :P
	 */

	if (backdated)
	{
		/*
		 * parse the backdated time.  We've already done some very
		 * basic validation of this string in the lj_init_config
		 * routine, it should be 12 chars long and it should convert
		 * to an integer greater than 0 (using atoi).  We want to
		 * make sure that it looks good here too.  We'll achieve that
		 * by making sure that all the characters are digits (which
		 * atoi should do) as we read them in. The format is:
		 * YYYYMMDDhhmm
		 */
		int             i, j;
		char            temp[5];

		/* first we validate that everything is a digit */
		for (j = 0; j < (int)strlen(dateback); j++)
		{
			switch (dateback[j])
			{
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				break;
			default:
				lj_error("Invalid characters in date.  Use only digits.\n");
				/* NOTREACHED */
				break;
			}
		}

		for (i = 0, j = 0; i < 5; i++, j++)
		{		/* the year YYYY */
			temp[i] = dateback[j];
		}
		temp[i - 1] = '\0';
		lj_strcopy(year, temp);

		for (i = 0, j--; i < 3; i++, j++)
		{		/* the month MM  */
			temp[i] = dateback[j];
		}
		temp[i - 1] = '\0';
		lj_strcopy(mon, temp);

		for (i = 0, j--; i < 3; i++, j++)
		{		/* the day DD  */
			temp[i] = dateback[j];
		}
		temp[i - 1] = '\0';
		lj_strcopy(day, temp);

		for (i = 0, j--; i < 3; i++, j++)
		{		/* the hour hh  */
			temp[i] = dateback[j];
		}
		temp[i - 1] = '\0';
		lj_strcopy(hour, temp);

		for (i = 0, j--; i < 3; i++, j++)
		{		/* the minutes mm  */
			temp[i] = dateback[j];
		}
		temp[i - 1] = '\0';
		lj_strcopy(min, temp);
	}
	else
	{
		/* grab the current time */
		errno = 0;
		if (time(&timep) == ((time_t) - 1))
		{
			return NULL;
		}
		ltime = localtime(&timep);
		strftime(year, 5, "%Y", ltime);
		strftime(mon, 3, "%m", ltime);
		strftime(day, 3, "%d", ltime);
		strftime(hour, 3, "%H", ltime);
		strftime(min, 3, "%M", ltime);

	}

	lj_debug(2, "The parameters of create_post are as follows:"
		"\tauth: [%s]\n\tevent: [%s]\n"
		"\tsubject: [%s]\n"
		"\tlineendings: [%s]\n\tsecurity: [%s]\n\tallowmask: [%s]\n"
		"\tusejournal: [%s]\n"
		"\tmood: [%s] OR moodid: [%d]\n"
		"\tuserpic: [%s]\n",
		auth, event,
		subject,
		lineendings, security, allowmask,
		usejournal,
		mood, moodid,
		userpic);

	if (replace)
		asprintf(&replace_text, "&itemid=%d", replace);
	else
		replace_text = NULL;
#define STR(x)	(x ? x : "")
	asprintf(&ret,
		"mode=%s&%s"
		"&event=%s"
		"&subject=%s"
		"&lineendings=%s"
		"&security=%s"
		"&allowmask=%s"
		"&year=%s&mon=%s&day=%s&hour=%s&min=%s"
		"&usejournal=%s"
		"&prop_opt_nocomments=%d"
		"&prop_opt_preformatted=%d"
		"&backdated=%d"
		"&prop_current_moodid=%d"
		"&prop_current_mood=%s"
		"&prop_picture_keyword=%s"
		"&prop_current_music=%s"
		"&prop_current_location=%s"
		"&prop_taglist=%s"
		"%s" /* replace=itemid */,
		replace ? "editevent" : "postevent", auth,
		STR(event),
		subject ? subject : "",
		lineendings,
		security,
		STR(allowmask),
		year, mon, day, hour, min,
		STR(usejournal),
		nocomments,
		preformatted,
		backdated,
		moodid,
		STR(mood),
		STR(userpic),
		STR(music),
		STR(location),
		STR(taglist),
		STR(replace_text));
	if (replace)
		free(replace_text);
	return ret;
}
