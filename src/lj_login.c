#include "system.h"
#include "lj.h"
#include "hash.h"
#include "net.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

char *create_login_request(const lj_user *user, const lj_logininfo *logininfo);

/*****************************************************************************
* This function generates the body of the HTTP request, we make the headers in
* lj_login.  This is mostly done so that we can just use strlen() to calculate
* the Content Length.
*/

int
lj_login(const lj_server * const serverinfo, lj_user *user, lj_logininfo *logininfo)
{
	char		*login_text;
	int		i, ret = 0;
	hashtable	*lj_hash;

	lj_setauth(serverinfo, user);
	create_hash(&lj_hash);	/* we delete it at the end */

	/* First we send our request */
	login_text = create_login_request(user, logininfo);
	lj_send_request(serverinfo, user, login_text, lj_hash);
	free(login_text);

	/* were we a success? */
	{
		char		*value;

		logininfo->success = 1;
		get(lj_hash, &value, "success");
		lj_debug(2, "success [%s]", value);
		if (!value || strcmp(value, "OK"))
		{
			/* we failed */
			logininfo->success = 0;
			ret = 1;
		}
		free(value);
	}

	/* next we look for an errmsg */
	get(lj_hash, &logininfo->errmsg, "errmsg");

	/* now we snag the user's details */
	get(lj_hash, &logininfo->name, "name");
	get(lj_hash, &logininfo->message, "message");
	geti(lj_hash, &user->fastserver, "fastserver");

	/* now we look for access groups */
	geti(lj_hash, &user->accesscount, "access_count");
	user->access = (char **) lj_malloc(sizeof(char *) * (size_t)user->accesscount);
	for (i = 0; i < user->accesscount; i++)
	{
		getp(lj_hash, &user->access[i], "access_%d", i + 1);
		lj_debug(2, "user->access[%d] = [%s]\n", i, user->access[i]);
	}

	/* next we get friend groups */
	geti(lj_hash, &user->frgrpcount, "frgrp_maxnum");
	user->frgrps = (char **) lj_malloc(sizeof(char *) * (size_t)user->frgrpcount);
	for (i = 0; i < user->frgrpcount; i++)
	{
		getp(lj_hash, &(user->frgrps[i]), "frgrp_%d_name", i + 1);
		lj_debug(2, "user->frgrps[%d] = [%s]\n", i, user->frgrps[i]);
	}

	/* next we find our mood ID */
	geti(lj_hash, &user->moodcount, "mood_count");
	user->moods = (char **) lj_malloc(sizeof(char *) * (size_t)user->moodcount);
	for (i = 0; i < user->moodcount; i++)
	{
		getp(lj_hash, &(user->moods[i]), "mood_%d_name", i + 1);
		lj_debug(2, "user->moods[%d] = [%s]\n", i, user->moods[i]);
	}

	/* now we get our picture keywords */
	geti(lj_hash, &user->userpiccount, "pickw_count");
	user->userpic = lj_malloc(user->userpiccount);
	for (i = 0; i < user->userpiccount; i++)
	{
		getp(lj_hash, &user->userpic[i], "pickw_%d", i + 1);
		lj_debug(2, "user->userpic[%d] = [%s]\n",
			i + 1, user->userpic[i + 1]);
	}

	lj_debug(1,
		"success:    [%s]\n"
		"name:       [%s]\n"
		"fastserver: [%s]\n"
		"message:    [%s]\n"
		"errmsg:     [%s]\n",
		logininfo->success ? "OK" : "FAIL",
		logininfo->name,
		user->fastserver ? "On" : "Off",
		logininfo->message ? logininfo->message : "",
		logininfo->errmsg ? logininfo->errmsg : "");

	/* Then we close the connection and return our success/failure code */
	delete_hash(lj_hash);
	return ret;
}

char *
create_login_request(const lj_user * const user,
		const lj_logininfo * const logininfo)
{
	char		*ret = NULL;
	char		*clientversion;

	lj_urlencode(logininfo->clientversion, &clientversion);
	asprintf(&ret,
		"mode=login&clientversion=%s&%s"
		"&getmoods=%d"
		"&getpicws=%d",
		clientversion, user->auth,
		logininfo->mood ? 1 : 0,
		logininfo->loginflags & getpickws ? 1 : 0);
	free(clientversion);
	return ret;
}
