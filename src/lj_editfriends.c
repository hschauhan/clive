#include "system.h"
#include "lj.h"
#include "hash.h"
#include "net.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

char *create_editfriends_request(const lj_user *user, const lj_friendinfo *logininfo);

/*****************************************************************************
* This function generates the body of the HTTP request, we make the headers in
* lj_friend.  This is mostly done so that we can just use strlen() to calculate
* the Content Length.
*/

int
lj_editfriends(const lj_server * const serverinfo, lj_user *user, lj_friendinfo *friendinfo)
{
	char		*friend_text;
	int		ret = 0;
	hashtable	*lj_hash = NULL;
	char		*value = NULL;

	lj_setauth(serverinfo, user);
	/* First we send our request */
	create_hash(&lj_hash);
	friend_text = create_editfriends_request(user, friendinfo);
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
	geti(lj_hash, &friendinfo->count, "friends_added");

	lj_debug(1, "success: [%s]\n", friendinfo->success ? "OK" : "FAIL");
	if (friendinfo->errmsg)
		lj_debug(1, "errmsg: [%s]\n", friendinfo->errmsg);

	/* Then we close the connection and return our success / failure code */
	delete_hash(lj_hash);
	return ret;
}

char *
create_editfriends_request(const lj_user * const user, const lj_friendinfo * const friendinfo)
{
	char		*ret;
	char		*username, *password;

	lj_urlencode(user->username, &username);
	lj_urlencode(user->password, &password);

	if (friendinfo->count != 1)
		lj_error("Can only add/delete one friend at the time\n");
	asprintf(&ret,
		"mode=editfriends&%s"
		"&editfriend_delete_user=%d"
		"&editfriend_add_%d_user=%s",
		user->auth,
		friendinfo->friendflags == DELETE_FRIEND,
		1, friendinfo->friends[0].user);

	free(username);
	free(password);
	(void)friendinfo;
	return ret;
}
