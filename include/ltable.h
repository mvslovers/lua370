/*
** $Id: ltable.h $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#ifndef ltable_h
#define ltable_h

#include "lobject.h"


#define gnode(t,i)	(&(t)->node[i])
#define gval(n)		(&(n)->i_val)
#define gnext(n)	((n)->u.next)


/*
** Clear all bits of fast-access metamethods, which means that the table
** may have any of these metamethods. (First access that fails after the
** clearing will set the bit again.)
*/
#define invalidateTMcache(t)	((t)->flags &= ~maskflags)


/* true when 't' is using 'dummynode' as its hash part */
#define isdummy(t)		((t)->lastfree == NULL)


/* allocated size for hash nodes */
#define allocsizenode(t)	(isdummy(t) ? 0 : sizenode(t))


/* returns the Node, given the value of a table entry */
#define nodefromval(v)	cast(Node *, (v))


LUAI_FUNC const TValue *luaH_getint (Table *t, lua_Integer key)					asm("LUAH0001");
LUAI_FUNC void luaH_setint (lua_State *L, Table *t, lua_Integer key,
                                                    TValue *value)				asm("LUAH0002");
LUAI_FUNC const TValue *luaH_getshortstr (Table *t, TString *key)				asm("LUAH0003");
LUAI_FUNC const TValue *luaH_getstr (Table *t, TString *key)					asm("LUAH0004");
LUAI_FUNC const TValue *luaH_get (Table *t, const TValue *key)					asm("LUAH0005");
LUAI_FUNC void luaH_newkey (lua_State *L, Table *t, const TValue *key,
                                                    TValue *value)				asm("LUAH0006");
LUAI_FUNC void luaH_set (lua_State *L, Table *t, const TValue *key,
                                                 TValue *value)					asm("LUAH0007");
LUAI_FUNC void luaH_finishset (lua_State *L, Table *t, const TValue *key,
                                       const TValue *slot, TValue *value)		asm("LUAH0008");
LUAI_FUNC Table *luaH_new (lua_State *L)										asm("LUAH0009");
LUAI_FUNC void luaH_resize (lua_State *L, Table *t, unsigned int nasize,
                                                    unsigned int nhsize)		asm("LUAH0010");
LUAI_FUNC void luaH_resizearray (lua_State *L, Table *t, unsigned int nasize)	asm("LUAH0011");
LUAI_FUNC void luaH_free (lua_State *L, Table *t)								asm("LUAH0012");
LUAI_FUNC int luaH_next (lua_State *L, Table *t, StkId key)						asm("LUAH0013");
LUAI_FUNC lua_Unsigned luaH_getn (Table *t)										asm("LUAH0014");
LUAI_FUNC unsigned int luaH_realasize (const Table *t)							asm("LUAH0015");


#if defined(LUA_DEBUG)
LUAI_FUNC Node *luaH_mainposition (const Table *t, const TValue *key)			asm("LUAH0016");
#endif


#endif
