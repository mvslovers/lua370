/*
** $Id: lstring.h $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#ifndef lstring_h
#define lstring_h

#include "lgc.h"
#include "lobject.h"
#include "lstate.h"


/*
** Memory-allocation error message must be preallocated (it cannot
** be created after memory is exhausted)
*/
#define MEMERRMSG       "not enough memory"


/*
** Size of a TString: Size of the header plus space for the string
** itself (including final '\0').
*/
#define sizelstring(l)  (offsetof(TString, contents) + ((l) + 1) * sizeof(char))

#define luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (sizeof(s)/sizeof(char))-1))


/*
** test whether a string is a reserved word
*/
#define isreserved(s)	((s)->tt == LUA_VSHRSTR && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#define eqshrstr(a,b)	check_exp((a)->tt == LUA_VSHRSTR, (a) == (b))


LUAI_FUNC unsigned int luaS_hash (const char *str, size_t l, unsigned int seed)	asm("LUAS0001");
LUAI_FUNC unsigned int luaS_hashlongstr (TString *ts)							asm("LUAS0002");
LUAI_FUNC int luaS_eqlngstr (TString *a, TString *b)							asm("LUAS0003");
LUAI_FUNC void luaS_resize (lua_State *L, int newsize)							asm("LUAS0004");
LUAI_FUNC void luaS_clearcache (global_State *g)								asm("LUAS0005");
LUAI_FUNC void luaS_init (lua_State *L)											asm("LUAS0006");
LUAI_FUNC void luaS_remove (lua_State *L, TString *ts)							asm("LUAS0007");
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s, int nuvalue)			asm("LUAS0008");
LUAI_FUNC TString *luaS_newlstr (lua_State *L, const char *str, size_t l)		asm("LUAS0009");
LUAI_FUNC TString *luaS_new (lua_State *L, const char *str)						asm("LUAS0010");
LUAI_FUNC TString *luaS_createlngstrobj (lua_State *L, size_t l)				asm("LUAS0011");


#endif
