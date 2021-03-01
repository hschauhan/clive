#include "system.h"
#include "lj.h"
#include "hash.h"
#include "net.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

char *create_getfriends_request(const lj_user * user, const lj_friendinfo * logininfo);

/*****************************************************************************
* This function generates the body of the HTTP request, we make the headers in
* lj_friend.  This is mostly done so that we can just use strlen() to calculate
* the Content Length.
*/

int
lj_getfriends(const lj_server * const serverinfo, lj_user *user, lj_friendinfo *friendinfo)
{
	char		*friend_text;
	int		i, ret = 0;
	hashtable	*lj_hash = NULL;
	char		*value = NULL;

	lj_setauth(serverinfo, user);
	/* First we send our request */
	create_hash(&lj_hash);
	friend_text = create_getfriends_request(user, friendinfo);
	lj_send_request(serverinfo, user, friend_text, lj_hash);
	free(friend_text);

	/* were we a success? */
	friendinfo->success = 1;
	get(lj_hash, &value, "success");
	if (!value || strcmp(value, "OK"))
	{
		/* we failed */
		friendinfo->success = 0;
		ret = 1;
	}
	free(value);

	/* next we look for an errmsg */
	get(lj_hash, &friendinfo->errmsg, "errmsg");

	/* now we snag number of friends */
	geti(lj_hash, &friendinfo->count, "friend_count");
	geti(lj_hash, &friendinfo->focount, "friendof_count");

	friendinfo->friends =
		lj_malloc((friendinfo->count) * sizeof(lj_friendninfo));
	for (i = 0; i < friendinfo->count; i++)
	{
		lj_friendninfo *info = &friendinfo->friends[i];
		getp(lj_hash, &info->user,	"friend_%d_user", i + 1);
		getp(lj_hash, &info->name,	"friend_%d_name", i + 1);
		getp(lj_hash, &info->birthday,	"friend_%d_birthday", i + 1);
		getp(lj_hash, &info->type,	"friend_%d_type", i + 1);
		getp(lj_hash, &info->status,	"friend_%d_status", i + 1);
	}
	friendinfo->friendsof =
		lj_malloc((friendinfo->focount) * sizeof(lj_friendninfo));
	for (i = 0; i < friendinfo->focount; i++)
	{
		lj_friendninfo *info = &friendinfo->friendsof[i];
		getp(lj_hash, &info->user, "friendof_%d_user", i + 1);
		getp(lj_hash, &info->name, "friendof_%d_name", i + 1);
	}

	lj_debug(1, "success: [%s]\n", friendinfo->success ? "OK" : "FAIL");
	if (friendinfo->errmsg)
		lj_debug(1, "errmsg: [%s]\n", friendinfo->errmsg);

	/* Then we close the connection and return our success / failure code */
	delete_hash(lj_hash);
	return ret;
}

char *
create_getfriends_request(const lj_user * const user, const lj_friendinfo * const friendinfo)
{
	char		*ret;

	asprintf(&ret,
		"mode=getfriends&%s"
		"&includefriendof=1"
		"&includebdays=1",
		user->auth);

	(void)friendinfo;
	return ret;
}
