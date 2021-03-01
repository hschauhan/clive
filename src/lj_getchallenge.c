#include "system.h"
#include "lj.h"
#include "hash.h"
#include "util.h"
#include "net.h"

#include <stdlib.h>
#include <string.h>

/*****************************************************************************
* This function generates the body of the HTTP request, we make the headers in
* lj_challenge.  This is mostly done so that we can just use strlen() to calculate
* the Content Length.
*/

int
lj_getchallenge(const lj_server * const serverinfo, const lj_user * const user, lj_challengeinfo *challengeinfo)
{
	int		ret = 0;
	hashtable	*lj_hash = NULL;
	char		*value = NULL;

	/* First we send our request */
	create_hash(&lj_hash);
	lj_send_request(serverinfo, user, "mode=getchallenge", lj_hash);

	/* were we a success? */
	challengeinfo->success = 1;
	get(lj_hash, &value, "success");
	if (!value || strcmp(value, "OK"))
	{
		/* we failed */
		challengeinfo->success = 0;
		ret = 1;
	}
	free(value);

	/* next we look for an errmsg */
	get(lj_hash, &challengeinfo->errmsg, "errmsg");

	/* now we challenge details */
	getp(lj_hash, &challengeinfo->auth_sceme, "auth_sceme");
	getp(lj_hash, &challengeinfo->challenge, "challenge");
	geti(lj_hash, &challengeinfo->expire_time, "expire_time");
	geti(lj_hash, &challengeinfo->server_time, "server_time");

	lj_debug(1, "success: [%s]\n", challengeinfo->success ? "OK" : "FAIL");
	if (challengeinfo->errmsg)
		lj_debug(1, "errmsg: [%s]\n", challengeinfo->errmsg);

	/* Then we close the connection and return our success / failure code */
	delete_hash(lj_hash);
	return ret;
}
