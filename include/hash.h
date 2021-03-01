#ifndef HASH_H
#define HASH_H

#include "system.h"

/***********************************************
** I have blatantly stolen the algorithm for  **
** this hash from the Perl internals.  It is  **
** very simplified, since it only deals with  **
** strings.                                   **
***********************************************/

#define BUCKETS 16 

typedef struct _keypair {
  struct _keypair *p_prev; /* now we're doubly linked!! */
  struct _keypair *p_next; /* we're implementing a linked list */
  char            *key;
  char            *value;
} keypair;

typedef struct _hashtable {
  keypair         *bucket[BUCKETS], *p_next; 
  int             i_bucket;       
} hashtable;

extern int create_hash(hashtable **ht);
/************************************
** Very simple.  It inits the ptr  **
** Returns 0 on success, 1 on fail **
************************************/  

extern int delete_hash(hashtable *ht);
/************************************
** Disposes of the memory.         **
** Returns 0 on success 1 on fail  **
************************************/

extern int put(hashtable *ht, const char *key, const char *value);
/****************************************************
** As above, this is very simple.  It puts the key **
** pair in the hash.  Returns 0 on success and 1   **
** on fail.                                        **
****************************************************/

extern keypair *get(const hashtable *ht, char **value, const char *key);
extern keypair *getp(const hashtable *ht, char **value, const char *key, ...);
extern keypair *geti(const hashtable *ht, int *value, const char *key);
extern keypair *getpi(const hashtable *ht, int *value, const char *key, ...);
/***************************************************
** It returns the value assoc'd with the key      **
** or NULL if the key isn't found.                **
***************************************************/

extern int del(hashtable *ht, char *key);
/***************************************
** Deletes the keypair from the hash  **
** 0 on success, 1 on fail.           **
***************************************/

extern keypair *next(hashtable *ht);
/**********************************
** It advances the hash's current**
** pointer and returns the next  **
** pointer in the hash.  It      **
** returns NULL at the end of the**
** hash.                         **
**********************************/

extern int reset(hashtable *ht);
/*****************************
** Resets the iterator to   **
** to the beginning.        **
*****************************/

u_int32_t hash(const char *string);
/***************************
** Generates hash code.   **
***************************/

void dumphash(const hashtable *ht);
/***************************
** Prints out diagnostic  **
** information of what is **
** in the hash            **
***************************/

#endif /* HASH_H */
