#ifndef LJ_H
#define LJ_H

#include <stdio.h>
#include "hash.h"

#define LJ_SUBJECT_MAX 256

/*****************************************************************************
* These are the different headers for the different LJ protocol modes.  Eaach
* mode actually has its own .c file, but I didn't see a reason to make a whole
* bunch of .h files
*/
typedef struct _lj_server {
  char *hostname;
  char *path;
  int  port;
} lj_server;

typedef enum _auth_method
{
	pwd_plain	= 0x01,
	pwd_hash	= 0x02,
	pwd_challenge	= 0x03,
} auth_method_t;

typedef struct _lj_user {
  char *username;
  char *password;
  auth_method_t  auth_method;
  char *auth;		/* username and password */
  int  fastserver;
  int  accesscount;
  int  userpiccount;
  int  frgrpcount;
  int  moodcount;
  char **access;
  char **userpic;
  char **frgrps;
  char **moods;
} lj_user;

/********************************
** In file: src/lj_postevent.c **
********************************/
typedef struct _lj_event {
  char *event;
  char *subject;
  char *security;
  char *allowmask;
  char *usejournal;
  char *mood;
  char *userpic;
  char *music;
  char *location;
  char *taglist;
  int  moodid;
  char dateback[13];
  int  nocomments;
  int  preformatted;
  int  backdated;
  int  replace;
} lj_event;

typedef struct _lj_event_response {
  int  success;
  char *message; /* either itemid or errmsg */
} lj_event_response;

extern int lj_postevent (const lj_server *serverinfo, lj_user *user, 
                         lj_event *post, lj_event_response *response );
/*****************************************************************************
* This routine handles the postevent mode.  
* 
* the routine calculates the date on its own, lineendings is set to
* 'unix', and support for properties is not going to be included.  
* Perhaps in a later version.
*/


/****************************
** In file: src/lj_login.c **
****************************/
typedef enum _loginflag
{
	getpickws	= 0x0001,
	getmenus	= 0x0002,
} loginflag;

typedef struct _lj_logininfo {
  /* stuff we send */
  char		*clientversion;
  loginflag	loginflags;
  char		*mood;
  int		moodid;
  char		**userpics;
  
  /* stuff we get back */
  int success;
  char *errmsg;
  char *name;
  char *message;
} lj_logininfo;

extern int lj_login(const lj_server *serverinfo, lj_user *user, 
                    lj_logininfo *logininfo);
/*****************************************************************************  
* This routine handles the login mode
*
* The routine simply sends off the login request and gets back stuff like the 
* keyword list
*/

/***********************************
** In file: src/lj_checkfriends.c **
***********************************/

/* this struct is used for both checkfriends and editfriends */
typedef struct _lj_friendninfo {
  char		*user;
  char		*name;
  char		*birthday;
  char		*bg;
  char		*fg;
  int		groupmask;
  char		*type;
  char		*status;
} lj_friendninfo;

typedef enum { NOTHING, ADD_FRIEND, DELETE_FRIEND } lj_editfriendflag;

/* this struct is used for both checkfriends and editfriends */
typedef struct _lj_friendinfo {
  /* stuff we send */
  lj_editfriendflag	friendflags;
  /* stuff we get back */
  int		success;
  char		*errmsg;
  int		count;
  lj_friendninfo *friends;
  int		focount;
  lj_friendninfo *friendsof;
} lj_friendinfo;

extern int lj_getfriends(const lj_server *serverinfo, lj_user *user, 
                    lj_friendinfo *friendinfo);

extern int lj_editfriends(const lj_server *serverinfo, lj_user *user,
		lj_friendinfo *friendinfo);

/***********************************
** In file: src/lj_getevents.c **
***********************************/
typedef struct _lj_eventninfo {
  int		itemid;
  char		*time;
  char		*event;
  char		*security;
  int		allowmask;
  char		*subject;
  char		*poster;
} lj_eventninfo;

typedef struct _lj_eventinfo {
  /* stuff we send */
  int		eventflags;
  char		*usejournal;
  /* stuff we get back */
  int		success;
  char		*errmsg;
  int		count;
  lj_eventninfo	*events;
} lj_eventinfo;

extern int lj_getevents(const lj_server *serverinfo, lj_user *user, 
                    lj_eventinfo *eventinfo);

/***********************************
** In file: src/lj_getcomments.c **
***********************************/
typedef struct _lj_commentninfo {
  int		itemid;
  char		*time;
  char		*poster;
  char		*subject;
  char		*comment;
  int		screened;
  int		parent;
} lj_commentninfo;

typedef struct _lj_commentinfo {
  /* stuff we send */
  int		commentflags;
  int		journalid;
  char		*usejournal;
  /* stuff we get back */
  int		success;
  char		*errmsg;
  int		count;
  lj_commentninfo	*comments;
} lj_commentinfo;

extern int lj_getcomments(const lj_server *serverinfo, lj_user *user, 
                    lj_commentinfo *commentinfo);

typedef struct _lj_challengeinfo {
  /* stuff we send */
  /* stuff we get back */
  int		success;
  char		*errmsg;
  char		*auth_sceme;
  char		*challenge;
  int		expire_time;
  int		server_time;
} lj_challengeinfo;

extern int lj_getchallenge(const lj_server *serverinfo, const lj_user *user,
		lj_challengeinfo *challengeinfo);

#endif /* LJ_H */
