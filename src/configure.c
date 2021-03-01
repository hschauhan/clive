#include "system.h"
#include "configure.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <pwd.h>
#include <unistd.h>

void lj_config_file(lj_config * config);
void lj_set_config(lj_config * config, const char *key, const char *value);
void lj_ask_username(lj_config *config);

void
lj_usage(void)
{
  puts (
 "Usage: clive [ -u username ] [ -w password ] [ -j journal ] [ -s subject ]\n"
 "             [ -r security ] [ -f mood ] [ -i userpic ] [ -m music ]\n"
 "             [ -l location ] [ -t taglist] [-p] [-d]\n"
 "             [ --backdate YYYMMDDhhmm ] [ --replace itemid ]\n"
 "             [ --charset encoding]\n"
 "             [ --plainpass ] [ --hashpass ] [ --softreturn ]\n"
 "       clive [ --loginonly | --nologin ]\n"
 "       clive [ --lfrgp | --laccess | --lpic | --lfriends | --levents [num] ]\n"
 "       clive [ --addfriend name | --delfriend name ]\n"
       );
}

int
lj_init_config(int argc, char **argv, lj_config * config)
{
	int		c, optindex;
	struct option longopts[] = {
		{"config",	required_argument, NULL, 'c'},	// 0
		{"nocomments",	no_argument,       NULL, 'd'},
		{"debug",	optional_argument, NULL, 'D'},
		{"mood",	required_argument, NULL, 'f'},
		{"userpic",	required_argument, NULL, 'i'},
		{"usejournal",	required_argument, NULL, 'j'},
		{"location",	required_argument, NULL, 'l'},
		{"music",	required_argument, NULL, 'm'},
		{"preformat",	no_argument,       NULL, 'p'},
		{"security",	required_argument, NULL, 'r'},
		{"subject",	required_argument, NULL, 's'},
		{"taglist",	required_argument, NULL, 't'},
		{"username",	required_argument, NULL, 'u'},
		{"version",	no_argument,       NULL, 'v'},
		{"password",	required_argument, NULL, 'w'},
		{"plainpass",	no_argument,       &config->plainpass,	1},// 17
		{"hashpass",	no_argument,       &config->hashpass,	1},
		{"nologin",	no_argument,       &config->nologin,	1},
		{"loginonly",	no_argument,       &config->loginonly,	1},
		{"lfrgp",	no_argument,       &config->lfrgp,	1},
		{"laccess",	no_argument,       &config->laccess,	1},
		{"lpic",	no_argument,       &config->lpic,	1},
		{"lfriends",	no_argument,       &config->lfriends,	1},
		{"softreturn",	no_argument,       &config->softreturn, 1},
		{"addfriend",	required_argument, NULL, -'a'},
		{"backdate",	required_argument, NULL, -'b'},
		{"lcomments",	optional_argument, NULL, -'c'},
		{"delfriend",	required_argument, NULL, -'d'},
		{"charset",	required_argument, NULL, -'C'},
		{"levents",	optional_argument, NULL, -'e'},
		{"help",	no_argument,       NULL, -'h'},
		{"replace",	required_argument, NULL, -'r'},
		{NULL, 0, NULL, 0},
	};

	memset(config, 0, sizeof(lj_config));

	lj_debug(3, "Getting command-line parameters");
	lj_debug(3, "argc: [%d]\n", argc);
	/*
	 * when you add options to the array above that have single character
	 * equivalents, be sure to add them to this string here.
	 */
	while ((c = getopt_long(argc, (char **)argv, "c:dD:f:i:j:l:m:pr:s:t:u:vw:?",
		longopts, &optindex)) != -1)
	{
		switch (c)
		{
		/* Regular long options */
		case 0:
			break;
		/* Special long options */
		case -'a':
			config->addfriend = 1;
			lj_strcopy(config->editfriend, optarg);
			break;
		case -'b':
			config->backdated = 1;
			if (strlen(optarg) == 12)
				lj_strcopy(config->dateback, optarg);
			else
				lj_error("Invalid date string. "
					"Date format YYYYMMDDhhmm\n");
			lj_debug(2, "Backdated entry [%s]\n", config->dateback);
			break;
		case -'c':
			config->lcomments = optarg ? atoi(optarg) : 5;
			break;
		case -'C':
			lj_strcopy(config->charset, optarg);
			break;
		case -'d':
			config->delfriend = 1;
			lj_strcopy(config->editfriend, optarg);
			break;
		case -'e':
			config->levents = optarg ? atoi(optarg) : 5;
			break;
		case -'h':
			lj_usage();
			exit(0);
		case -'r':
			config->replace = atoi(optarg);
			break;
		/* Regular options (short + long) */
		case 'c':
			lj_set_config(config, "rcfile", optarg);
			break;
		case 'd':
			config->nocomments++;
			lj_debug(2, "Comments disabled");
			break;
		case 'D':
			if (optarg)
				debug_level = atoi(optarg);
			if (!debug_level)
				debug_level = 1;
			lj_debug(2, "Enabling debug (level %d)", debug_level);
			break;
		case 'f':
			lj_set_config(config, "mood", optarg);
			break;
		case 'i':
			config->getpickws++;
			lj_set_config(config, "userpic", optarg);
			break;
		case 'j':
			lj_set_config(config, "usejournal", optarg);
			break;
		case 'l':
			lj_set_config(config, "location", optarg);
			break;
		case 'm':
			lj_set_config(config, "music", optarg);
			break;
		case 'p':
			config->preformatted++;
			lj_debug(2, "Preformatted text enabled");
			break;
		case 'r':
			lj_set_config(config, "security", optarg);
			break;
		case 's':
			lj_set_config(config, "subject", optarg);
			break;
		case 't':
			lj_set_config(config, "taglist", optarg);
			break;
		case 'u':
			lj_set_config(config, "username", optarg);
			break;
		case 'v':
			fprintf(stdout, "POSIX-Clive/%s\n", VERSION);
			exit(0);
		case 'w':
			lj_set_config(config, "password", optarg);
			break;
		/* errors */
		case ':':
			fprintf(stderr, "Missing option argument\n\n");
			lj_usage();
			exit(1);
		case '?':
			fprintf(stderr, "Ambiguous option selected\n\n");
			lj_usage();
			exit(1);
		default:
			fprintf(stderr, "Invalid option '%c'\n\n", c);
			lj_usage();
			exit(1);
		}
	}
	lj_debug(3, "Got command line options");

	/* Here we determine which config file to read from */
	if (config->rcfile == NULL)
		asprintf(&config->rcfile, "%s/.cliverc", getenv("HOME"));
	/* else we 've already got a config file name so.... */
	lj_config_file(config);

	/* we can do this because lj_set_config checks for NULL */
	lj_set_config(config, "editor", getenv("EDITOR"));
	lj_set_config(config, "editor", getenv("VISUAL"));
	lj_set_config(config, "server_host", "www.livejournal.com");
	lj_set_config(config, "server_port", "80");
	lj_set_config(config, "server_path", "/interface/flat");
	lj_set_config(config, "charset", ""); /* "" = "char" = local charset */
	lj_ask_username(config);
	return 0;
}

void
lj_set_config(lj_config * config, const char * const key, const char * const value)
{
	if (!key || !value)
		return;
	if (!strcmp(key, "username"))
	{
		if ((config->username == NULL) || !strcmp(config->username, ""))
		{
			config->username = strdup(value);
			lj_debug(2, "%s: [%s]\n", key, config->username);
		}
	}
	else if (!strcmp(key, "password"))
	{
		if ((config->password == NULL) || !strcmp(config->password, ""))
		{
			config->password = strdup(value);
			lj_debug(2, "%s: [******]\n", key);
		}
	}
	else if (!strcmp(key, "usejournal"))
	{
		if ((config->usejournal == NULL) ||
			!strcmp(config->usejournal, ""))
		{
			config->usejournal = strdup(value);
			lj_debug(2, "%s: [%s]\n", key, config->usejournal);
		}
	}
	else if (!strcmp(key, "subject"))
	{
		if ((config->subject == NULL) || !strcmp(config->subject, ""))
		{
			config->subject = strdup(value);
			lj_debug(2, "%s: [%s]\n", key, config->subject);
		}
	}
	else if (!strcmp(key, "server_host"))
	{
		if ((config->server_host == NULL)
		    || !strcmp(config->server_host, ""))
		{
			config->server_host = strdup(value);
			lj_debug(2, "%s: [%s]\n", key, config->server_host);
		}
	}
	else if (!strcmp(key, "server_port"))
	{
		if (!config->server_port)
		{
			config->server_port = atoi(value);
			lj_debug(2, "%s: [%d]\n", key, config->server_port);
		}
	}
	else if (!strcmp(key, "server_path"))
	{
		if ((config->server_path == NULL)
		    || !strcmp(config->server_path, ""))
		{
			config->server_path = strdup(value);
			lj_debug(2, "%s: [%s]\n", key, config->server_path);
		}
	}
	else if (!strcmp(key, "editor"))
	{
		if ((config->editor == NULL) || !strcmp(config->editor, ""))
		{
			config->editor = strdup(value);
			lj_debug(2, "%s: [%s]\n", key, config->editor);
		}
	}
	else if (!strcmp(key, "rcfile"))
	{
		if ((config->rcfile == NULL) || !strcmp(config->rcfile, ""))
		{
			config->rcfile = strdup(value);
			lj_debug(2, "%s: [%s]\n", key, config->rcfile);
		}
	}
	else if (!strcmp(key, "security"))
	{
		if (value[0] == ':' &&
			((config->customsecurity == NULL) ||
			 !strcmp(config->customsecurity, "")))
		{
			/* custom security */
			config->customsecurity = strdup(value);
		}
		else if (!strcmp(value, "private") || !strcmp(value, "1"))
		{
			/*
			 * do not confuse the number for value with the
			 * number for the security config.  That is solely
			 * coincedence.
			 */
			config->security = 1;
		}
		else if (!strcmp(value, "friends") || !strcmp(value, "2"))
		{
			config->security = 2;	/* usemask */
			config->allowmask = 1;
		}
		else
			lj_error("Invalid security setting. . . exiting\n");
		if (config->customsecurity)
			lj_debug(2, "customsecurity: [%s]\n",
				config->customsecurity);
		else
			lj_debug(2, "security: [%d]\n", config->security);
		lj_debug(2, "allowmask: [%d]\n", config->allowmask);
	}
	else if (!strcmp(key, "mood"))
	{
		if ((config->mood == NULL) || !strcmp(config->mood, ""))
		{
			config->mood = strdup(value);
			config->getmoods++;
			lj_debug(2, "mood: [%s]\n", config->mood);
		}
	}
	else if (!strcmp(key, "getmoods"))
	{
		config->getmoods++;
	}
	else if (!strcmp(key, "userpic"))
	{
		if ((config->userpic == NULL) || !strcmp(config->userpic, ""))
		{
			config->userpic = strdup(value);
			config->getpickws++;
			lj_debug(2, "userpic: [%s]\n", config->userpic);
		}
	}
	else if (!strcmp(key, "music"))
	{
		if ((config->music == NULL) || !strcmp(config->music, ""))
		{
			config->music = strdup(value);
			lj_debug(2, "music: [%s]\n", config->music);
		}
	}
	else if (!strcmp(key, "location"))
	{
		if ((config->location == NULL) || !strcmp(config->location, ""))
		{
			config->location = strdup(value);
			lj_debug(2, "location: [%s]\n", config->location);
		}
	}
	else if (!strcmp(key, "taglist"))
	{
		if ((config->taglist == NULL) || !strcmp(config->taglist, ""))
		{
			config->taglist = strdup(value);
			lj_debug(2, "taglist: [%s]\n", config->taglist);
		}
	}
	else if (!strcmp(key, "softreturn"))
	{
		if (!strcasecmp(value, "true") ||
			!strcasecmp(value, "yes") ||
			(atoi(value) > 0))
		{
			config->softreturn = 1;
		}
	}
	else if (!strcmp(key, "charset"))
	{
		if (NULL == config->charset)
			config->charset = strdup(value);
	}			/* add new option here */
}

void 
lj_config_file(lj_config * config)
{
	FILE           *cfile;
	char            buffer[LJ_BUFFER_MAX],
	                key[LJ_BUFFER_MAX],
	                value[LJ_BUFFER_MAX];
	int             i, j;

	/*
	 * I should add some better error handling.  Right now, if your
	 * config file isn't just perfect...the program will probably blow
	 * up.
	 */
	lj_debug(3, "Attempting to open [%s] to read config\n", config->rcfile);
	if ((cfile = fopen(config->rcfile, "r")) == NULL)
		return;

	while (fgets(buffer, LJ_BUFFER_MAX, cfile))
	{
		/*
		 * each buffer should be either whitespace, comments, or
		 * one config item a line must be *either* a comment,
		 * whitespace, or config item cannot mix config and
		 * comment
		 */
		chomp(buffer);
		lj_debug(3, "config [%s]\n", buffer);
		/*Test for leading '#' for comment */
		if (buffer[0] == '#')
			continue;
		/* now we slurp up the leading whitespace */
		for (i = 0; (buffer[i] == ' ') || (buffer[i] == '\t'); i++)
			/* DO NOTHING */;
		/* now we get the key */
		for (j = 0; (buffer[i] != ' ') &&
				(buffer[i] != '\t') &&
				(buffer[i] != '=');
			i++, j++)
		{
			key[j] = buffer[i];
		}
		key[j] = '\0';
		lj_debug(4, "key [%s]", key);
		/*
		 * now we slurp up the characters between the key and the
		 * value
		 */
		for (;
			(buffer[i] == ' ') ||
			(buffer[i] == '\t') ||
			(buffer[i] == '=');
			i++)
			/* DO NOTHING */;

		/*
		 * We're going to slurp the entire rest of the buffer then
		 * trim off trailing whitespace
		 */
		for (j = 0; i < (int)strlen(buffer); j++, i++)
		{
			value[j] = buffer[i];
		}
		for (i = j - 1; i > 0; i--)
		{
			if ((value[i] != ' ') && (value[i] != '\t'))
				break;
		}
		value[i + 1] = '\0';	/* terminate our string */

		lj_debug(4, " value [%s]\n", value);
		/* and finally, we use them */
		lj_set_config(config, key, value);
	}
}

void
lj_ask_username(lj_config *config)
{
	if (!config->username)
	{
		char		name[LJ_BUFFER_MAX];

		if (!isatty(fileno(stdin)))
			lj_error("Won't prompt for username without a tty\n");
		printf("Username: ");
		fgets(name, LJ_BUFFER_MAX, stdin);
		chomp(name);
		if (!strlen(name))
			lj_error("Won't continue without a username\n");
		lj_set_config(config, "username", name);
	}
	if (!config->password)
	{
		char		*pass;

		if (!isatty(fileno(stdin)))
			lj_error("Won't prompt for password without a tty\n");
		pass = getpass("Password: ");
		if (!strlen(pass))
			lj_error("Won't continue without a password\n");
		lj_set_config(config, "password", pass);
	}
}
