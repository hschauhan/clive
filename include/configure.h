#ifndef CONFIGURE_H
#define CONFIGURE_H

typedef struct _lj_config {
  /* required for all modes */
  char *username; 
  char *password;
  int  plainpass; 
  int  hashpass;

  /* for the login mode */
  int loginonly; /* mostly for testing purposes, won't post event if true */
  int nologin; /* conversely, this means it won't login at all */
  int getmoods;
  int getmenus;
  int getpickws;
  
  /* these are for the postevent mode */
  char *usejournal;
  char *subject;
  char *mood;
  int security; /* 0 public, 1 private, 2 usemask */
  char *customsecurity; /*  A single string like "thisgroup:thatgroup:etc" */
  char *userpic; /*  A word that */
  char *music; /* a string describing the song/sounds being listened to */
  char *location; /* a string describing the current location */
  char *taglist; /* a comma-separated string of relevant tags */
  u_int32_t allowmask; /* set bit 0 for friends only  */
  int nocomments; /* prop_opt_nocomments */
  int preformatted; /* prop_opt_preformatted */
  int backdated; /* prop_opt_backdated */
  char dateback[13]; /* will be initialized to "\0" and set with the  */
                     /* backdate option */
  int replace;	/*  eventid to be replaced */
  char *editfriend;

  /* these are for the postevent mode, and possibly others */
  char *server_host;
  char *server_path;
  int server_port;

  /* these are clive options */
  char *editor; /* which editor to use when editing text */
  char *rcfile; /* where to look for config stuff */
  char *charset; /* character encoding of the input */
  int lfrgp;  
  int laccess;
  int lpic;
  int lfriends;
  int levents;
  int lcomments;
  int addfriend;
  int delfriend;
  int softreturn;

} lj_config;

extern int lj_init_config(int argc, char **argv, lj_config *config);
/*****************************************************************************
* This should be run right away, it populates the values in the lj_config
* struct.  It will populate with defaults when that's proper.  Values in the
* ${HOME}/.cliverc file will override those (if that file exists).  If a config
* file is specified on the command line, that will be used instead.  Lastly,
* command-line options (e.g. --user or -p) will override config-file options.
*
* returns 0 on success, non-zero on failure.
*/

extern void lj_usage(void);
/***************************************************************************** 
* Simply put, it prints a usage statement.  See configure.c for more details
*/

#endif
