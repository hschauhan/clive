#ifndef SYSTEM_H
#define SYSTEM_H

/* This file should be included in every other include file we have,
 * so we'll define our program wide includes here
 */

#include "config.h"
#include "util.h"

#define LJ_BUFFER_MAX 1024

#if defined(DEBUG4) && !defined(DEBUG3)
#define DEBUG3
#endif

#if defined(DEBUG3) && !defined(DEBUG2)
#define DEBUG2
#endif

#if defined(DEBUG2) && !defined(DEBUG)
#define DEBUG
#endif

#endif
