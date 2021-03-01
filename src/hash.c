#include "system.h"
#include "hash.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* This is the guts of it right here.  This is how we convert from a string
 * into a number.  This will always return a number < BUCKETS, since we 
 * modulo it
 */

u_int32_t
hash(const char * const str)
{
  register const char *s_key = str;
  register u_int32_t i_key = strlen(s_key);
  register u_int32_t h_key = 0;

  while(i_key--) {
    h_key += *s_key++;
    h_key += (h_key << 10);
    h_key ^= (h_key >> 6);
  }
  h_key += (h_key << 3);
  h_key ^= (h_key >> 11);
  lj_debug(3, "[%s] belongs in bucket [%d]\n", str,
         (h_key += (h_key << 15)) % BUCKETS);
  return (h_key += (h_key << 15)) % BUCKETS;
}

/* The rest is fairly simple.  In create_hash we allocate the memory, and 
 * initialize all the pointers to NULL.
 */

int
create_hash(hashtable ** ht)
{
  int i;

  *ht = (hashtable *) lj_malloc(sizeof(hashtable));
  for(i = 0; i < BUCKETS; i++) {
    (*ht)->bucket[i] = (keypair *) NULL;
  }
  (*ht)->i_bucket = 0;
  (*ht)->p_next = (keypair *) NULL;
  return 0;
}

/* Here we just free all of the keypairs, then free the hash itself */

int
delete_hash(hashtable * ht)
{
  keypair *i_pair = (keypair *) NULL;

  reset(ht);
  lj_debug(3, "Freeing keypairs...");
  while((i_pair = next(ht))) {
    del(ht, i_pair->key);
  }
  lj_debug(3, "Freeing hashtable [%p]", ht);
  lj_free(ht);
  lj_debug(3, "success");
  return 0;
}

/* Putting is fairly simple we simply insert into the ordered linked list
 * that is located in the bucket that hash() tells us to use.
 */

int
put(hashtable * ht, const char * const key, const char * const value)
{
  keypair *p_prev, *p_next, *p_new, **pp_head;
  int bucket = hash(key);

  if (!ht || !key || !value)
  {
	  lj_debug(1, "No keypair to insert");
	  return 1;
  }

  p_prev = p_next = p_new = (keypair *) NULL;

  p_new = (keypair *) lj_malloc(sizeof(keypair));
  lj_debug(3, "New keypair [%p]\n", p_new);
  p_new->key = strdup(key);
  lj_debug(3, "\tkey [%p/%s]\n", p_new->key, p_new->key);
  p_new->value = strdup(value);
  lj_debug(3, "\tvalue [%p/%s]\n", p_new->value, p_new->value);
  pp_head = &(ht->bucket[bucket]);
  p_next = *pp_head;
  while(p_next && (strcmp(p_new->key, p_next->key) > 0)) {
    p_prev = p_next;
    p_next = p_prev->p_next;
  }

  p_new->p_prev = p_prev;
  if(p_prev) {
    p_prev->p_next = p_new;
  } else {
    *pp_head = p_new;
  }

  p_new->p_next = p_next;
  if(p_next) {
    p_next->p_prev = p_new;
  }
  lj_debug(3, "\tp_prev [%p/%s]\n\tp_next[%p/%s]\n",
         p_prev, p_prev ? p_prev->key : "(nil)",
         p_next, p_next ? p_next->key : "(nil)");


  return 0;
}

/* Deleting is similarly straight forward.  We just look in the bucket, and
 * search until we find the proper item. 
 *
 * Note: if there is no such key, it will still return 0.
 */

int
del(hashtable * ht, char *key)
{
  keypair *p_this, *p_prev, *p_next;
  char *value;
  int bucket = hash(key);

  p_this = p_prev = p_next = (keypair *) NULL;

  if(!(p_this = get(ht, &value, key)))
    return 0;
  lj_free(value);
  p_prev = p_this->p_prev;
  p_next = p_this->p_next;

  lj_debug(3, "Deleting [%p/%s]\n", p_this, p_this->key);

  lj_debug(3, "Adjusting pointers...");
  if(p_prev) {
    p_prev->p_next = p_next;
    lj_debug(3, "p_prev[%p]->p_next[%p]\n", p_prev, p_prev->p_next);
  } else if(p_this) {
    ht->bucket[bucket] = p_this->p_next;
    lj_debug(3, "bucket[%d]->[%p]\n", bucket, ht->bucket[bucket]);
  }

  if(p_next) {
    p_next->p_prev = p_prev;
    lj_debug(3, "p_next[%p]->p_prev[%p]", p_next, p_next->p_prev);
  }
  /*
   * Most doubly linked lists have tail pointers.  We don't, so we don't
   * have to worry about that like we do with the head pointer.
   */

  lj_debug(3, "Freeing key [%p/%s]\n", p_this->key, p_this->key);
  lj_free(p_this->key);
  lj_debug(3, "Freeing value [%p/%s]\n", p_this->value, p_this->value);
  lj_free(p_this->value);
  lj_debug(3, "Freeing keypair [%p]\n", p_this);
  lj_free(p_this);
  return 0;
}

/* Getting is practically the same as deleting */

keypair *
getp(const hashtable * const ht, char **value, const char * const key, ...)
{
	keypair		*p_this = NULL;
	char		*buff = NULL;
	int		bucket;
	va_list		ap;

	va_start(ap, key);
	vasprintf(&buff, key, ap);
	va_end(ap);
	bucket = hash(buff);
	p_this = ht->bucket[bucket];
	while (p_this && p_this->p_next && strcmp(buff, p_this->key)) {
		p_this = p_this->p_next;
	}
	if (!p_this || strcmp(p_this->key, buff)) {
		free(buff);
		*value = NULL;
		return NULL;
	}
	lj_debug(3, "get(\"%s\") returning [%p][%s]->[%s]\n",
		buff, p_this, p_this->key, p_this->value);
	free(buff);
	*value = strdup(p_this->value);
	return p_this;
}

keypair *
get(const hashtable * const ht, char **value, const char * const key)
{
	return getp(ht, value, "%s", key);
}

keypair *
getpi(const hashtable * const ht, int *value, const char * const key, ...)
{
	char		*buff = NULL, *values = NULL;
	keypair		*kp;
	va_list		ap;

	va_start(ap, key);
	vasprintf(&buff, key, ap);
	va_end(ap);

	kp = get(ht, &values, buff);
	free(buff);
	if (values)
	{
		*value = atoi(values);
		lj_free(values);
	}
	else
		*value = 0;
	return kp;
}

keypair *
geti(const hashtable * const ht, int *value, const char * const key)
{
	return getpi(ht, value, "%s", key);
}

/* The iterator is also very simple.  It just has some more bookkeeping to
 * handle.  Remember that p_current is a pointer to a pointer.
 */

keypair *
next(hashtable * ht)
{
  keypair *p_next;

  p_next = ht->p_next;

  lj_debug(3, "ni_bucket=[%d]\np_next=[%p/%s]\n",
         ht->i_bucket, p_next, p_next ? p_next->key : "");
  if(p_next)
    ht->p_next = p_next->p_next;
  while(!ht->p_next && (ht->i_bucket < BUCKETS)) {
    /* Here we are setting the pointer to the next item */
    ht->i_bucket++;
    ht->p_next = ht->bucket[ht->i_bucket];
  }
  lj_debug(3, "ht->p_next=[%p/%s] in bucket [%d]\n",
         ht->p_next, ht->p_next ? (ht->p_next)->key : "", ht->i_bucket);
  if(!p_next && ht->p_next)
    p_next = next(ht);
  return p_next;
}

/* The reset function just sets i_bucket and p_current back to 0 */
int
reset(hashtable * ht)
{
  ht->i_bucket = 0;
  ht->p_next = ht->bucket[0];
  return 0;
}

void
dumphash(const hashtable * const ht)
{
  int i;
  keypair *p_next;

  for(i = 0; i < BUCKETS; i++) {
    if((p_next = ht->bucket[i])) {
      printf("Bucket [%d]:\n", i);
      while(p_next) {
        printf("\tKeypair at [%p]\n\t  key [%p][%s]\n\t  value [%p][%s]\n",
               p_next, p_next->key, p_next->key,
               p_next->value, p_next->value);
        printf("\t  p_prev [%p][%s]\n\t  p_next [%p][%s]\n",
               p_next->p_prev,
               p_next->p_prev ? (p_next->p_prev)->key : "(nil)",
               p_next->p_next,
               p_next->p_next ? (p_next->p_next)->key : "(nil)");
        p_next = p_next->p_next;
      }
    }
  }
}
