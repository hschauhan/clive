#ifndef EDIT_EVENT_H
#define EDIT_EVENT_H

char *get_event(char *editor);
/**************************************************************************** 
* This does all of our magic when it comes to obtaining the event text.  It
* first checks to see if stdin is a TTY or not.  If it isn't, it slurps in the
* event text from the pipe (for things like '$ cat entry | clive').  If it is a
* TTY, then we'll try one of two things.  First we'll try to edit with the
* editor whose name is passed in in the string paraemter.  If that doesn't
* work, we'll just use our own special Berkley Mail style editor.
*/

char *get_subject(char *subject);
/**************************************************************************** 
* This method does magic for determining what our subject line is.  It should
* follow this logic:
*
* if ( subject already contains a string ) 
*   return subject
* else if ( we're on a TTY ) {
*   interactively ask for a subject
*   trim the trailing newline (if we have it)
*   slurp up any extra bytes that might be in the stream
*   return it
* else
*   the user doesn't want a subject, so return an empty string
*/

int	remove_edit_file(int posted);
/* Remove temporary files
 * or keep a backup if posting failed
 */

/* char *edit_event(char *event, char *editor); */ 
#endif /* EDIT_EVENT_H */

