#include "system.h"
#include "lj.h"
#include "hash.h"
#include "util.h"
#include "net.h"

#include <stdlib.h>
#include <string.h>

static char *create_getcomments_request(const lj_user * user, const lj_commentinfo * logininfo);

/*****************************************************************************
* This function generates the body of the HTTP request, we make the headers in
* lj_comment.  This is mostly done so that we can just use strlen() to calculate
* the Content Length.
*/

int
lj_getcomments(const lj_server * const serverinfo, lj_user *user, lj_commentinfo *commentinfo)
{
	char		*comment_text;
	int		i, ret = 0;
	hashtable	*lj_hash = NULL;
	char		*value = NULL;

	lj_setauth(serverinfo, user);
	/* First we send our request */
	create_hash(&lj_hash);
	comment_text = create_getcomments_request(user, commentinfo);
	lj_send_request(serverinfo, user, comment_text, lj_hash);
	free(comment_text);

	/* were we a success? */
	commentinfo->success = 1;
	get(lj_hash, &value, "success");
	if (!value || strcmp(value, "OK"))
	{
		/* we failed */
		commentinfo->success = 0;
		ret = 1;
	}
	free(value);

	/* next we look for an errmsg */
	get(lj_hash, &commentinfo->errmsg, "errmsg");

	/* now we snag number of comments */
	geti(lj_hash, &commentinfo->count, "comments_count");

	commentinfo->comments =
		lj_malloc((commentinfo->count) * sizeof(lj_commentninfo));
	for (i = 0; i < commentinfo->count; i++)
	{
		lj_commentninfo	*info = &commentinfo->comments[i];

		getpi(lj_hash, &info->itemid,	"comments_%d_itemid", i + 1);
		getpi(lj_hash, &info->parent,	"comments_%d_parenttalkid", i + 1);
		getp(lj_hash, &info->time,	"comments_%d_talktime", i + 1);
		getp(lj_hash, &info->comment,	"comments_%d_talk", i + 1);
		getp(lj_hash, &info->poster,	"comments_%d_poster", i + 1);
		if (!info->comment)
			info->comment = strdup("");
		lj_urldecode(info->comment);
	}

	lj_debug(1, "success: [%s]\n", commentinfo->success ? "OK" : "FAIL");
	if (commentinfo->errmsg)
		lj_debug(1, "errmsg: [%s]\n", commentinfo->errmsg);

	/* Then we close the connection and return our success / failure code */
	delete_hash(lj_hash);
	return ret;
}

char *
create_getcomments_request(const lj_user * const user, const lj_commentinfo * const commentinfo)
{
	char		*ret;
	char		*username, *password;

	lj_urlencode(user->username, &username);
	lj_urlencode(user->password, &password);

	asprintf(&ret,
		"mode=getcomments&%s"
		"&truncate=100&prefersubject=1"
		"&selecttype=lastn&howmany=%d"
		"&lineendings=unix"
		"&eventid=%d"
		"&usejournal=%s",
		user->auth,
		20,
		commentinfo->journalid,
		commentinfo->usejournal ? commentinfo->usejournal : "");

	free(username);
	free(password);
	return ret;
}
