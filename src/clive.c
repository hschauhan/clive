/*
 * Copyright (C) 2001 - 2003, Samuel Tesla <stesla@pobox.com>
 * Copyright (C) 2003 - 2004, Johan van Selst <johans@stack.nl>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "system.h"
#include "net.h"
#include "util.h"
#include "lj.h"
#include "edit.h"
#include "configure.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

int 
main(int argc, char **argv)
{
	lj_config	config;
	lj_server	server;
	lj_user		user;
	lj_logininfo	logininfo;
	lj_friendinfo	friendinfo;
	lj_eventinfo	eventinfo;
	lj_commentinfo	commentinfo;
	lj_event	post;
	lj_event_response post_response;

	if (lj_init_config(argc, argv, &config))
	{
		lj_error("Error initializing clive!\n");
	}
	lj_debug(2, "host: [%s]\n", config.server_host);
	lj_debug(2, "port: [%d]\n", config.server_port);
	lj_debug(2, "path: [%s]\n", config.server_path);

#ifdef		HAVE_ICONV
	utfenc = iconv_open("UTF-8", config.charset);
	utfdec = iconv_open(config.charset, "UTF-8");
	if ((iconv_t)-1 == utfenc || (iconv_t)-1 == utfdec)
		lj_error("Cannot open character conversion\n");
	lj_debug(2, "local charset: [%s]\n", config.charset);
#endif		/* HAVE_ICONV */


	/*******************
        ********************
        ** CLIENT VERSION **
        ********************
        *******************/

	memset(&logininfo, '\0', sizeof(logininfo));
	logininfo.clientversion = strdup("POSIX-Clive/" VERSION);
	if (config.getmoods && config.mood)
	{
		logininfo.mood = config.mood;
	}
	logininfo.userpics = NULL;
	if (config.getpickws)
		logininfo.loginflags |= getpickws;
	/* the serverinfo struct */
	server.hostname = config.server_host;
	server.port = config.server_port;
	server.path = config.server_path;

	/* the user struct */
	memset(&user, '\0', sizeof(user));
	user.username = config.username;
	user.password = config.password;
	if (config.plainpass)
		user.auth_method = pwd_plain;
	else if (config.hashpass)
		user.auth_method = pwd_hash;
	else
		user.auth_method = pwd_challenge;

	if (config.nologin)
	{
		if (config.loginonly)
		{
			lj_error("Make up your mind!  "
				"--nologin and --loginonly don't mix\n");
		}
	}
	else
	{
		printf("Logging on to %s, please wait\n", server.hostname);
		if (lj_login(&server, &user, &logininfo))
		{
			/* if this returns a non - zero value
			 * we know something broke
			 */
			if (logininfo.errmsg)
				lj_error("Errors logging in:\n\t%s\n",
					logininfo.errmsg);
			else
				lj_error("Errors logging in, try again later\n");
		}
		else
		{
			printf("Welcome to livejournal, %s\n", logininfo.name);
			if ((logininfo.message != NULL) &&
				strcmp(logininfo.message, ""))
			{
				fprintf(stderr, "A message from livejournal: %s\n",
					logininfo.message);
				if (isatty(fileno(stdin)))
				{
					puts("Press Enter to continue");
					getchar();
				}
			}
		}

		/* Handle non-posting events first */
		if (config.loginonly)
		{
			exit(0);
		}
		else if (config.lfrgp)
		{
			int	i;

			puts("ID\tGroup Name");
			for (i = 0; i < user.frgrpcount; i++)
			{
				printf("%2d\t%s\n", i, user.frgrps[i]);
			}
			exit(0);
		}
		else if (config.laccess)
		{
			int             i;

			puts("ID\tJournal Name");
			for (i = 0; i < user.accesscount; i++)
			{
				printf("%2d\t%s\n", i, user.access[i]);
			}
			exit(0);
		}
		else if (config.lpic)
		{
			int             i;

			puts("ID\tPicture Keyword");
			for (i = 0; i < user.userpiccount; i++)
			{
				printf("%2d\t%s\n", i,
				       user.userpic[i] ? user.userpic[i] : "(default)");
			}
			exit(0);
		}
		else if (config.lfriends)
		{
			int             i;

			memset(&friendinfo, 0, sizeof(friendinfo));

			lj_getfriends(&server, &user, &friendinfo);
			printf("%2d Friends\n", friendinfo.count);
			for (i = 0; i < friendinfo.count; i++)
			{
				printf("%3d %-10s %-14s  %s\n", i,
				       friendinfo.friends[i].birthday
					       ? friendinfo.friends[i].birthday
					       : "",
				       friendinfo.friends[i].user,
				       friendinfo.friends[i].name);
			}
			printf("%2d Friends of\n", friendinfo.focount);
			for (i = 0; i < friendinfo.focount; i++)
			{
				printf("%3d %-10s %-14s  %s\n", i,
				    friendinfo.friendsof[i].birthday
					    ? friendinfo.friendsof[i].birthday
					    : "",
				       friendinfo.friendsof[i].user,
				       friendinfo.friendsof[i].name);
			}
			exit(0);
		}
		else if (config.levents)
		{
			int             i;

			memset(&eventinfo, 0, sizeof(eventinfo));
			eventinfo.eventflags = config.levents;
			eventinfo.usejournal = config.usejournal;

			lj_getevents(&server, &user, &eventinfo);
			printf("%2d Events\n", eventinfo.count);
			for (i = 0; i < eventinfo.count; i++)
			{
				char 	*event = eventinfo.events[i].event;
				if (strlen(event) > 50)
					sprintf(event + 47, "...");
				printf("%3d %s  %-50.50s\n",
					eventinfo.events[i].itemid,
					eventinfo.events[i].time,
					eventinfo.events[i].event);
			}
			exit(0);
		}
		else if (config.lcomments)
		{
			int             i;

			memset(&commentinfo, 0, sizeof(commentinfo));
			commentinfo.journalid = config.lcomments;
			commentinfo.usejournal = config.usejournal;

			lj_getcomments(&server, &user, &commentinfo);
			printf("%2d Events\n", commentinfo.count);
			for (i = 0; i < commentinfo.count; i++)
			{
				char 	*comment = commentinfo.comments[i].comment;
				if (strlen(comment) > 50)
					sprintf(comment + 47, "...");
				printf("%3d %s  %s %-50.50s\n",
					commentinfo.comments[i].itemid,
					commentinfo.comments[i].time,
					commentinfo.comments[i].poster,
					comment);
			}
			exit(0);
		}
		else if (config.addfriend || config.delfriend)
		{
			memset(&friendinfo, 0, sizeof(friendinfo));
			friendinfo.friendflags =
				config.addfriend ? ADD_FRIEND : DELETE_FRIEND;
			friendinfo.count = 1;
			friendinfo.friends = malloc(sizeof(lj_friendninfo));
			memset(friendinfo.friends, 0, sizeof(friendinfo));
			friendinfo.friends[0].user = config.editfriend;

			lj_editfriends(&server, &user, &friendinfo);
			printf("%s %d friend%s\n",
				config.addfriend ? "Added" : "Deleted",
				friendinfo.count,
				friendinfo.count == 1 ? "" : "s");
			exit(0);
		}
	}


	/*
	 * Here we make sure that they have access to the journal they want
	 * to post to.
	 */
	post.usejournal = config.usejournal;

	if (user.accesscount && post.usejournal)
	{
		int             i = 0;

		if ((i = atoi(post.usejournal)))
			post.usejournal = user.access[i];

		for (i = 0; i < user.accesscount; i++)
			if (!strcmp(post.usejournal, user.access[i]))
				break;
		if (i >= user.accesscount)
			lj_error("You don't have access to post in %s!\n",
				post.usejournal);
	}

	post.nocomments = config.nocomments;
	post.preformatted = config.preformatted;
	post.music = config.music;
	post.location = config.location;
	post.taglist = config.taglist;
	if (config.backdated)
	{
		post.backdated = config.backdated;
		lj_strcopy(post.dateback, config.dateback);
	}
	else
	{
		post.backdated = 0;
		post.dateback[0] = '\0';
	}

	if (config.userpic)
	{
		int             i = -1;

		i = atoi(config.userpic);
		if (i >= 0)
		{
			char           *buff;

			buff = lj_malloc(strlen(config.userpic) + 1);
			sprintf(buff, "%d", i);
			if (!strcmp(buff, config.userpic))
			{
				/* we have a number ! */
				post.userpic = user.userpic[i];
			}
			else
			{
				post.userpic = config.userpic;
			}
			lj_free(buff);
		}
		else
		{
			post.userpic = config.userpic;
		}
	}
	else
	{
		post.userpic = NULL;
	}

	post.mood = NULL;
	post.moodid = 0;
	if (config.getmoods)
	{
		/* we have feelings ! */
		if (logininfo.moodid)
		{
			post.moodid = logininfo.moodid;
		}
		else if (logininfo.mood)
		{
			post.mood = strdup(logininfo.mood);
		}
	}
	if (config.customsecurity)
	{
		char		*prevp, *nextp;
		int		i;
		u_int32_t	mask = 0;

		/* skip the leading ':', then jump to following ':'s
		 * replacing them by '\0' to get seperate strings
		 */
		for (prevp = config.customsecurity + 1; prevp; prevp = nextp)
		{
			if ((nextp = strchr(prevp, ':')))
				*nextp++ = '\0';

			lj_debug(2, "Extracted [%s] from friendgroup spec\n",
				prevp);
			for (i = 0; i < user.frgrpcount; i++)
			{
				if (!strcmp(prevp, user.frgrps[i]))
				{
					lj_debug(2, "Anding [%d] to mask\n",
						i + 1);
					mask |= 1 << (i + 1);
					break;
				}
			}
			if (i == user.frgrpcount)
				lj_error("Invalid friendgroup. . . exiting\n");
		}
		post.security = strdup("usemask");
		asprintf(&post.allowmask, "%d", mask);
	}
	else
	{
		switch (config.security)
		{
		default:
			post.security = strdup("public");
			post.allowmask = strdup("");
			break;
		case 1:
			post.security = strdup("private");
			post.allowmask = strdup("");
			break;
		case 2:
			post.security = strdup("usemask");
			post.allowmask = strdup("1");
			break;
		}
	}
	post.replace = config.replace;

	/*
	 * we get these after everything else...that way we can be sure that
	 * any lj_error's come before the editor is launched.
	 */
	post.subject = get_subject(config.subject);
	if (post.subject && (strlen(post.subject) > LJ_SUBJECT_MAX))
	{
		lj_error("Subject exceeds allowed length. . . Exiting\n");
	}
	post.event = get_event(config.editor);

	/*
	 * do text postprocessing for meta information
	 */
	if (post.event)
	{
		char *start, *colon, *space, *end;

		start = post.event;
		while (*start && (end = strchr(start, '\n')))
		{
			if (!(colon = strchr(start, ':')))
				break;
			if ((start >= colon) || (colon >= end))
				break;
			if ((space = strchr(start, ' ')) && (colon > space))
				break;
			*end = '\0';
			*colon++ = '\0';
			while (*colon && isspace(*colon))
				colon++;
			if (!strcasecmp("Subject", start))
			{
				if (post.subject)
					free(post.subject);
				post.subject = strdup(colon);
			}
			else if (!strcasecmp("Music", start))
			{
				if (post.music)
					free(post.music);
				post.music = strdup(colon);
			}
			else if (!strcasecmp("Mood", start))
			{
				if (post.mood)
					free(post.mood);
				post.mood = strdup(colon);
			}
			else if (!strcasecmp("Location", start))
			{
				if (post.location)
					free(post.location);
				post.location = strdup(colon);
			}
			else if (!strcasecmp("Taglist", start))
			{
				if (post.taglist)
					free(post.taglist);
				post.taglist = strdup(colon);
			}
			else
			{
				printf("Ignoring unknown header %s\n", start);
			}
			start = end + 1;
		}
		if (config.softreturn)
			joinstring(start);
		post.event = start;
	}

	/* next we find our mood ID */
	if (post.mood && user.moodcount)
	{
		int i;
		post.moodid = 0;
		for (i = 0; i < user.moodcount; i++)
		{
			if (!strcmp(user.moods[i], post.mood))
				post.moodid = i;
		}
		lj_debug(2, "Found mood [%s] has moodid [%d]\n",
			post.mood, post.moodid);
	}


	if (!lj_postevent(&server, &user, &post, &post_response) &&
	    (post_response.message != NULL))
	{
		printf("Successfully posted item %s to journal %s \n",
		       post_response.message,
		       post.usejournal ? post.usejournal : user.username);
		remove_edit_file(1);
	}
	else if ((post_response.message != NULL)
		 && strcmp(post_response.message, ""))
	{
		printf("The following error was encountered "
			"while posting your message:\n"
			"\t%s\n",
			post_response.message);
		remove_edit_file(0);
	}
	else
	{
		puts("An unknown error was encountered "
			"while posting your message");
		remove_edit_file(0);
	}
	return 0;
}
