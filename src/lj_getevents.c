#include "system.h"
#include "lj.h"
#include "hash.h"
#include "util.h"
#include "net.h"

#include <stdlib.h>
#include <string.h>

static char *create_getevents_request(const lj_user * user, const lj_eventinfo * logininfo);

/*****************************************************************************
* This function generates the body of the HTTP request, we make the headers in
* lj_event.  This is mostly done so that we can just use strlen() to calculate
* the Content Length.
*/

int
lj_getevents(const lj_server * const serverinfo, lj_user *user, lj_eventinfo *eventinfo)
{
	char		*event_text;
	int		i, ret = 0;
	hashtable	*lj_hash = NULL;
	char		*value = NULL;

	lj_setauth(serverinfo, user);
	/* First we send our request */
	create_hash(&lj_hash);
	event_text = create_getevents_request(user, eventinfo);
	lj_send_request(serverinfo, user, event_text, lj_hash);
	free(event_text);

	/* were we a success? */
	eventinfo->success = 1;
	get(lj_hash, &value, "success");
	if (!value || strcmp(value, "OK"))
	{
		/* we failed */
		eventinfo->success = 0;
		ret = 1;
	}
	free(value);

	/* next we look for an errmsg */
	get(lj_hash, &eventinfo->errmsg, "errmsg");

	/* now we snag number of events */
	geti(lj_hash, &eventinfo->count, "events_count");

	eventinfo->events =
		lj_malloc((eventinfo->count) * sizeof(lj_eventninfo));
	for (i = 0; i < eventinfo->count; i++)
	{
		lj_eventninfo	*info = &eventinfo->events[i];

		getpi(lj_hash, &info->itemid,	"events_%d_itemid", i + 1);
		getp(lj_hash, &info->time,	"events_%d_eventtime", i + 1);
		getp(lj_hash, &info->event,	"events_%d_event", i + 1);
		getp(lj_hash, &info->security,	"events_%d_security", i + 1);
		getpi(lj_hash, &info->allowmask,"events_%d_allowmask", i + 1);
		getp(lj_hash, &info->subject,	"events_%d_subject", i + 1);
		getp(lj_hash, &info->poster,	"events_%d_poster", i + 1);
		lj_urldecode(info->event);
	}

	lj_debug(1, "success: [%s]\n", eventinfo->success ? "OK" : "FAIL");
	if (eventinfo->errmsg)
		lj_debug(1, "errmsg: [%s]\n", eventinfo->errmsg);

	/* Then we close the connection and return our success / failure code */
	delete_hash(lj_hash);
	return ret;
}

char *
create_getevents_request(const lj_user * const user, const lj_eventinfo * const eventinfo)
{
	char		*ret;
	char		*username, *password;

	lj_urlencode(user->username, &username);
	lj_urlencode(user->password, &password);

	asprintf(&ret,
		"mode=getevents&%s"
		"&truncate=100&prefersubject=1"
		"&selecttype=lastn&howmany=%d"
		"&noprops=1"
		"&lineendings=unix"
		"&usejournal=%s",
		user->auth,
		eventinfo->eventflags,
		eventinfo->usejournal ? eventinfo->usejournal : "");

	free(username);
	free(password);
	return ret;
}
