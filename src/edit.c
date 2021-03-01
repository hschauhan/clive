#include "system.h"
#include "edit.h"
#include "util.h"
#include "lj.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

static char		*tempfile = NULL;

char *
get_event(char *editor)
{
	char		*s, *se, *sbe, buffer[LJ_BUFFER_MAX];
	unsigned long	l = 0;
	int		i = 1;

	s = se = sbe = NULL;

	if (isatty(fileno(stdin)))
	{
		/* XXX: replace this with something browsing $PATH */
		/* if (!access(editor, X_OK)) */
		if (editor)
		{
			int		pid, status;
			char		*unique;

			lj_debug(3, "Forking");
			errno = 0;
			/* consider making the temp directory configurable */
			lj_uniquename(&unique);
			asprintf(&tempfile, "/tmp/%s", unique);
			lj_free(unique);
			pid = fork();
			lj_debug(3, "The pid is [%d]\n", pid);
			if (pid == 0)
			{
				errno = 0;
				lj_debug(3, "Child [%d] executing [%s] on [%s]\n",
					getpid(), editor, tempfile);
				execlp(editor, "clive", tempfile, NULL);
				perror("Error launching alternative editor");
				exit(1);
			}
			if (pid == -1)
			{
				perror("Error forking new process");
				exit(1);
			}

			errno = 0;
			if (waitpid(pid, &status, 0) != pid)
			{
				perror("Error retrieving event");
			}
			else
			{
				FILE		*file;

				errno = 0;
				if ((file = fopen(tempfile, "r")) == NULL)
				{
					perror("Error opening temp file");
					exit(1);
				}

				se = s = (char *)lj_malloc((size_t) LJ_BUFFER_MAX);
				sbe = s + LJ_BUFFER_MAX;
				while (fgets(buffer, LJ_BUFFER_MAX, file))
				{

					lj_debug(4, "read [%s]\n", buffer);
					se = lj_strcopy(se, buffer);
					l = (unsigned long)sbe - (unsigned long)se;
					lj_debug(4, "size [%ld]\n", l);
					if (l <= LJ_BUFFER_MAX)
					{
						i++;
						lj_debug(4, "lj_realloc(%p, %d)\n",
							s, i * LJ_BUFFER_MAX);
						s = (char *)lj_realloc(s, (size_t) (i * LJ_BUFFER_MAX));
						sbe = s + (i * LJ_BUFFER_MAX);
						se = sbe - (l + LJ_BUFFER_MAX);
					}
				}
				errno = 0;
			}
		}
		else
		{
			se = s = (char *)lj_malloc((size_t) LJ_BUFFER_MAX);
			sbe = s + LJ_BUFFER_MAX;
			while (fgets(buffer, LJ_BUFFER_MAX, stdin)
			       && strcmp(buffer, ".\n"))
			{
				/*
				 * Read input until a line with just a period
				 * on it
				 */
				lj_debug(4, "read [%s]\n", buffer);
				se = lj_strcopy(se, buffer);
				l = (unsigned long)sbe - (unsigned long)se;
				lj_debug(4, "size [%ld]\n", l);
				if (l <= LJ_BUFFER_MAX)
				{
					i++;
					lj_debug(4, "lj_realloc(%p, %d)\n",
						s, i * LJ_BUFFER_MAX);
					s = (char *)lj_realloc(s, (size_t) (i * LJ_BUFFER_MAX));
					sbe = s + (i * LJ_BUFFER_MAX);
					se = sbe - (l + LJ_BUFFER_MAX);
				}
			}
		}
	}
	else
	{
		se = s = (char *)lj_malloc((size_t) LJ_BUFFER_MAX);
		sbe = s + LJ_BUFFER_MAX;
		while (fgets(buffer, LJ_BUFFER_MAX, stdin))
		{
			/* slurp in from stdin */
			lj_debug(4, "read [%s]\n", buffer);
			se = lj_strcopy(se, buffer);
			l = (unsigned long)sbe - (unsigned long)se;
			lj_debug(4, "size [%ld]\n", l);
			if (l <= LJ_BUFFER_MAX)
			{
				i++;
				lj_debug(4, "lj_realloc(%p, %d)\n",
					s, i * LJ_BUFFER_MAX);
				s = (char *)lj_realloc(s, (size_t) (i * LJ_BUFFER_MAX));
				sbe = s + (i * LJ_BUFFER_MAX);
				se = sbe - (l + LJ_BUFFER_MAX);
			}
		}
	}
	/* clean up input */
	chomp(s);
	lj_debug(3, "get_event: returning [%s]\n", s);
	return s;
}

char           *
get_subject(char *subject)
{
	char           *s = NULL;

	lj_debug(3, "get_subject: subject [%s]\n", subject);
	if ((subject != NULL) && strcmp(subject, ""))
	{
		lj_debug(3, "get_subject: returning [%s]\n", subject);
		return subject;
	}
	else if (isatty(fileno(stdin)))
	{
		s = (char *)lj_malloc((size_t) LJ_SUBJECT_MAX);
		fputs("Subject: ", stdout);
		if (fgets(s, LJ_SUBJECT_MAX, stdin))
		{
			if (!chomp(s))
			{
				int             i;

				while (((i = getc(stdin)) != '\n') && (i != EOF))
					/* DO NOTHING */;
			}
		}
	}
	lj_debug(3, "get_subject: returning [%s]\n", s);
	return s;
}

int
remove_edit_file(int posted)
{
	if (!tempfile)
		return 0;
	if (!posted)
	{
		fprintf(stdout, "A backup copy of the posting is kept in %s\n",
			tempfile);
		return 0;
	}
	if (unlink(tempfile))
		perror("Error deleting temp file");
	return 1;
}
