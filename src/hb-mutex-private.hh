/*
 * Copyright © 2007  Chris Wilson
 * Copyright © 2009,2010  Red Hat, Inc.
 * Copyright © 2011  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Contributor(s):
 *	Chris Wilson <chris@chris-wilson.co.uk>
 * Red Hat Author(s): Behdad Esfahbod
 * Google Author(s): Behdad Esfahbod
 */

#ifndef HB_MUTEX_PRIVATE_HH
#define HB_MUTEX_PRIVATE_HH

#include "hb-private.hh"

HB_BEGIN_DECLS


/* mutex */

/* We need external help for these */

#ifdef HAVE_GLIB

#include <glib.h>

typedef GStaticMutex hb_mutex_t;
#define HB_MUTEX_INIT			G_STATIC_MUTEX_INIT
#define hb_mutex_init(M)		g_static_mutex_init (M)
#define hb_mutex_lock(M)		g_static_mutex_lock (M)
#define hb_mutex_trylock(M)		g_static_mutex_trylock (M)
#define hb_mutex_unlock(M)		g_static_mutex_unlock (M)
#define hb_mutex_free(M)		g_static_mutex_free (M)


#elif defined(_MSC_VER)

#include <Windows.h>

typedef CRITICAL_SECTION hb_mutex_t;
#define HB_MUTEX_INIT				{ NULL, 0, 0, NULL, NULL, 0 }
#define hb_mutex_init(M)			InitializeCriticalSection (M)
#define hb_mutex_lock(M)			EnterCriticalSection (M)
#define hb_mutex_trylock(M)			TryEnterCriticalSection (M)
#define hb_mutex_unlock(M)			LeaveCriticalSection (M)
#define hb_mutex_free(M)			DeleteCriticalSection (M)


#else

#warning "Could not find any system to define platform macros, library will NOT be thread-safe"

typedef struct { volatile int m; } hb_mutex_t;
#define HB_MUTEX_INIT				0
#define hb_mutex_init(M)			((void) ((M)->m = 0))
#define hb_mutex_lock(M)			((void) ((M)->m = 1))
#define hb_mutex_trylock(M)			((M)->m = 1, 1)
#define hb_mutex_unlock(M)			((void) ((M)->m = 0))
#define hb_mutex_free(M)			((void) ((M)-M = 2))


#endif


struct hb_static_mutex_t : hb_mutex_t
{
  hb_static_mutex_t (void) {
    hb_mutex_init (this);
  }

  inline void lock (void) { hb_mutex_lock (this); }
  inline void unlock (void) { hb_mutex_unlock (this); }
};


HB_END_DECLS


template <typename item_t>
struct hb_static_threadsafe_set_t
{
  hb_lockable_set_t <item_t, hb_static_mutex_t> set;
  hb_static_mutex_t lock;

  template <typename T>
  inline item_t *replace_or_insert (T v)
  {
    return set.replace_or_insert (v, lock);
  }

  template <typename T>
  inline void remove (T v)
  {
    set.remove (v, lock);
  }

  template <typename T>
  inline bool find (T v, item_t *i)
  {
    return set.find (v, i, lock);
  }

  template <typename T>
  inline item_t *find_or_insert (T v)
  {
    return set.find_or_insert (v, lock);
  }

  void finish (void)
  {
    set.finish (lock);
  }
};


HB_BEGIN_DECLS

HB_END_DECLS

#endif /* HB_MUTEX_PRIVATE_HH */