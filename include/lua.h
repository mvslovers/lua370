/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (http://www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#include <stdarg.h>
#include <stddef.h>


#include "luaconf.h"


#define LUA_VERSION_MAJOR	"5"
#define LUA_VERSION_MINOR	"4"
#define LUA_VERSION_RELEASE	"6"

#define LUA_VERSION_NUM			504
#define LUA_VERSION_RELEASE_NUM		(LUA_VERSION_NUM * 100 + 6)

#define LUA_VERSION	"Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
#define LUA_RELEASE	LUA_VERSION "." LUA_VERSION_RELEASE
#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2023 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


/* mark for precompiled code ('<esc>Lua') */
#define LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#define LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (-LUAI_MAXSTACK is the minimum valid index; we keep some free empty
** space after that to help overflow detection)
*/
#define LUA_REGISTRYINDEX	(-LUAI_MAXSTACK - 1000)
#define lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#define LUA_OK		0
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5


typedef struct lua_State lua_State;


/*
** basic types
*/
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8

#define LUA_NUMTYPES		9



/* minimum Lua stack available to a C function */
#define LUA_MINSTACK	20


/* predefined values in the registry */
#define LUA_RIDX_MAINTHREAD	1
#define LUA_RIDX_GLOBALS	2
#define LUA_RIDX_LAST		LUA_RIDX_GLOBALS


/* type of numbers in Lua */
typedef LUA_NUMBER lua_Number;


/* type for integer functions */
typedef LUA_INTEGER lua_Integer;

/* unsigned integer type */
typedef LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
typedef LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
typedef int (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
typedef int (*lua_KFunction) (lua_State *L, int status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);

typedef int (*lua_Writer) (lua_State *L, const void *p, size_t sz, void *ud);


/*
** Type for memory-allocation functions
*/
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
typedef void (*lua_WarnFunction) (void *ud, const char *msg, int tocont);


/*
** Type used by the debug API to collect debug information
*/
typedef struct lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif


/*
** RCS ident string
*/
extern const char lua_ident[]													asm("LUA00000");


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, void *ud)						asm("LUA00001");
LUA_API void       (lua_close) (lua_State *L)									asm("LUA00002");
LUA_API lua_State *(lua_newthread) (lua_State *L)								asm("LUA00003");
LUA_API int        (lua_closethread) (lua_State *L, lua_State *from)			asm("LUA00004");
LUA_API int        (lua_resetthread) (lua_State *L)								asm("LUA00005"); /* Deprecated! */

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf)		asm("LUA00006");


LUA_API lua_Number (lua_version) (lua_State *L)									asm("LUA00007");


/*
** basic stack manipulation
*/
LUA_API int   (lua_absindex) (lua_State *L, int idx)							asm("LUA00008");
LUA_API int   (lua_gettop) (lua_State *L)										asm("LUA00009");
LUA_API void  (lua_settop) (lua_State *L, int idx)								asm("LUA00010");
LUA_API void  (lua_pushvalue) (lua_State *L, int idx)							asm("LUA00011");
LUA_API void  (lua_rotate) (lua_State *L, int idx, int n)						asm("LUA00012");
LUA_API void  (lua_copy) (lua_State *L, int fromidx, int toidx)					asm("LUA00013");
LUA_API int   (lua_checkstack) (lua_State *L, int n)							asm("LUA00014");

LUA_API void  (lua_xmove) (lua_State *from, lua_State *to, int n)				asm("LUA00015");


/*
** access functions (stack -> C)
*/

LUA_API int             (lua_isnumber) (lua_State *L, int idx)					asm("LUA00016");
LUA_API int             (lua_isstring) (lua_State *L, int idx)					asm("LUA00017");
LUA_API int             (lua_iscfunction) (lua_State *L, int idx)				asm("LUA00018");
LUA_API int             (lua_isinteger) (lua_State *L, int idx)					asm("LUA00019");
LUA_API int             (lua_isuserdata) (lua_State *L, int idx)				asm("LUA00020");
LUA_API int             (lua_type) (lua_State *L, int idx)						asm("LUA00021");
LUA_API const char     *(lua_typename) (lua_State *L, int tp)					asm("LUA00022");

LUA_API lua_Number      (lua_tonumberx) (lua_State *L, int idx, int *isnum)		asm("LUA00023");
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, int idx, int *isnum)	asm("LUA00024");
LUA_API int             (lua_toboolean) (lua_State *L, int idx)					asm("LUA00025");
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len)	asm("LUA00026");
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, int idx)					asm("LUA00027");
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx)				asm("LUA00028");
LUA_API void	       *(lua_touserdata) (lua_State *L, int idx)				asm("LUA00029");
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx)					asm("LUA00030");
LUA_API const void     *(lua_topointer) (lua_State *L, int idx)					asm("LUA00031");


/*
** Comparison and arithmetic functions
*/

#define LUA_OPADD	0	/* ORDER TM, ORDER OP */
#define LUA_OPSUB	1
#define LUA_OPMUL	2
#define LUA_OPMOD	3
#define LUA_OPPOW	4
#define LUA_OPDIV	5
#define LUA_OPIDIV	6
#define LUA_OPBAND	7
#define LUA_OPBOR	8
#define LUA_OPBXOR	9
#define LUA_OPSHL	10
#define LUA_OPSHR	11
#define LUA_OPUNM	12
#define LUA_OPBNOT	13

// LUA_API void  (lua_arith) (lua_State *L, int op)								asm("LUA00098");

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

LUA_API int   (lua_rawequal) (lua_State *L, int idx1, int idx2)					asm("LUA00032");
LUA_API int   (lua_compare) (lua_State *L, int idx1, int idx2, int op)			asm("LUA00033");


/*
** push functions (C -> stack)
*/
LUA_API void        (lua_pushnil) (lua_State *L)								asm("LUA00034");
LUA_API void        (lua_pushnumber) (lua_State *L, lua_Number n)				asm("LUA00035");
LUA_API void        (lua_pushinteger) (lua_State *L, lua_Integer n)				asm("LUA00036");
LUA_API const char *(lua_pushlstring) (lua_State *L, const char *s, size_t len)	asm("LUA00037");
LUA_API const char *(lua_pushstring) (lua_State *L, const char *s)				asm("LUA00038");
LUA_API const char *(lua_pushvfstring) (lua_State *L, const char *fmt,
                                                      va_list argp)				asm("LUA00039");
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...)		asm("LUA00040");
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n)		asm("LUA00041");
LUA_API void  (lua_pushboolean) (lua_State *L, int b)							asm("LUA00042");
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p)					asm("LUA00043");
LUA_API int   (lua_pushthread) (lua_State *L)									asm("LUA00044");


/*
** get functions (Lua -> stack)
*/
LUA_API int (lua_getglobal) (lua_State *L, const char *name)					asm("LUA00045");
LUA_API int (lua_gettable) (lua_State *L, int idx)								asm("LUA00046");
LUA_API int (lua_getfield) (lua_State *L, int idx, const char *k)				asm("LUA00047");
LUA_API int (lua_geti) (lua_State *L, int idx, lua_Integer n)					asm("LUA00048");
LUA_API int (lua_rawget) (lua_State *L, int idx)								asm("LUA00049");
LUA_API int (lua_rawgeti) (lua_State *L, int idx, lua_Integer n)				asm("LUA00050");
LUA_API int (lua_rawgetp) (lua_State *L, int idx, const void *p)				asm("LUA00051");

LUA_API void  (lua_createtable) (lua_State *L, int narr, int nrec)				asm("LUA00052");
LUA_API void *(lua_newuserdatauv) (lua_State *L, size_t sz, int nuvalue)		asm("LUA00053");
LUA_API int   (lua_getmetatable) (lua_State *L, int objindex)					asm("LUA00054");
LUA_API int  (lua_getiuservalue) (lua_State *L, int idx, int n)					asm("LUA00055");


/*
** set functions (stack -> Lua)
*/
LUA_API void  (lua_setglobal) (lua_State *L, const char *name)					asm("LUA00056");
LUA_API void  (lua_settable) (lua_State *L, int idx)							asm("LUA00057");
LUA_API void  (lua_setfield) (lua_State *L, int idx, const char *k)				asm("LUA00058");
LUA_API void  (lua_seti) (lua_State *L, int idx, lua_Integer n)					asm("LUA00059");
LUA_API void  (lua_rawset) (lua_State *L, int idx)								asm("LUA00060");
LUA_API void  (lua_rawseti) (lua_State *L, int idx, lua_Integer n)				asm("LUA00061");
LUA_API void  (lua_rawsetp) (lua_State *L, int idx, const void *p)				asm("LUA00062");
LUA_API int   (lua_setmetatable) (lua_State *L, int objindex)					asm("LUA00063");
LUA_API int   (lua_setiuservalue) (lua_State *L, int idx, int n)				asm("LUA00064");


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API void  (lua_callk) (lua_State *L, int nargs, int nresults,
                           lua_KContext ctx, lua_KFunction k)					asm("LUA00065");
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, NULL)

LUA_API int   (lua_pcallk) (lua_State *L, int nargs, int nresults, int errfunc,
                            lua_KContext ctx, lua_KFunction k)					asm("LUA00066");
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, NULL)

LUA_API int   (lua_load) (lua_State *L, lua_Reader reader, void *dt,
                          const char *chunkname, const char *mode)				asm("LUA00067");

LUA_API int (lua_dump) (lua_State *L, lua_Writer writer, void *data, int strip)	asm("LUA00068");


/*
** coroutine functions
*/
LUA_API int  (lua_yieldk)     (lua_State *L, int nresults, lua_KContext ctx,
                               lua_KFunction k)									asm("LUA00069");
LUA_API int  (lua_resume)     (lua_State *L, lua_State *from, int narg,
                               int *nres)										asm("LUA00070");
LUA_API int  (lua_status)     (lua_State *L)									asm("LUA00071");
LUA_API int (lua_isyieldable) (lua_State *L)									asm("LUA00072");

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, NULL)


/*
** Warning-related functions
*/
LUA_API void (lua_setwarnf) (lua_State *L, lua_WarnFunction f, void *ud)		asm("LUA00073");
LUA_API void (lua_warning)  (lua_State *L, const char *msg, int tocont)			asm("LUA00074");


/*
** garbage-collection function and options
*/

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
#define LUA_GCSTEP		5
#define LUA_GCSETPAUSE		6
#define LUA_GCSETSTEPMUL	7
#define LUA_GCISRUNNING		9
#define LUA_GCGEN		10
#define LUA_GCINC		11

LUA_API int (lua_gc) (lua_State *L, int what, ...)								asm("LUA00075");


/*
** miscellaneous functions
*/

LUA_API int   (lua_error) (lua_State *L)										asm("LUA00076");

LUA_API int   (lua_next) (lua_State *L, int idx)								asm("LUA00077");

LUA_API void  (lua_concat) (lua_State *L, int n)								asm("LUA00078");
LUA_API void  (lua_len)    (lua_State *L, int idx)								asm("LUA00079");

LUA_API size_t   (lua_stringtonumber) (lua_State *L, const char *s)				asm("LUA00080");

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, void **ud)						asm("LUA00081");
LUA_API void      (lua_setallocf) (lua_State *L, lua_Alloc f, void *ud)			asm("LUA00082");

LUA_API void (lua_toclose) (lua_State *L, int idx)								asm("LUA00083");
LUA_API void (lua_closeslot) (lua_State *L, int idx)							asm("LUA00084");


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define lua_getextraspace(L)	((void *)((char *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	lua_tonumberx(L,(i),NULL)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),NULL)

#define lua_pop(L,n)		lua_settop(L, -(n)-1)

#define lua_newtable(L)		lua_createtable(L, 0, 0)

#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#define lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#define lua_pushglobaltable(L)  \
	((void)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)


#define lua_insert(L,idx)	lua_rotate(L, (idx), 1)

#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_pop(L, 1))

#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_pop(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/
#if defined(LUA_COMPAT_APIINTCASTS)

#define lua_pushunsigned(L,n)	lua_pushinteger(L, (lua_Integer)(n))
#define lua_tounsignedx(L,i,is)	((lua_Unsigned)lua_tointegerx(L,i,is))
#define lua_tounsigned(L,i)	lua_tounsignedx(L,(i),NULL)

#endif

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define LUA_NUMTAGS		LUA_NUMTYPES

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)


LUA_API int (lua_getstack) (lua_State *L, int level, lua_Debug *ar)				asm("LUA00085");
LUA_API int (lua_getinfo) (lua_State *L, const char *what, lua_Debug *ar)		asm("LUA00086");
LUA_API const char *(lua_getlocal) (lua_State *L, const lua_Debug *ar, int n)	asm("LUA00087");
LUA_API const char *(lua_setlocal) (lua_State *L, const lua_Debug *ar, int n)	asm("LUA00088");
LUA_API const char *(lua_getupvalue) (lua_State *L, int funcindex, int n)		asm("LUA00089");
LUA_API const char *(lua_setupvalue) (lua_State *L, int funcindex, int n)		asm("LUA00090");

LUA_API void *(lua_upvalueid) (lua_State *L, int fidx, int n)					asm("LUA00091");
LUA_API void  (lua_upvaluejoin) (lua_State *L, int fidx1, int n1,
                                               int fidx2, int n2)				asm("LUA00092");

LUA_API void (lua_sethook) (lua_State *L, lua_Hook func, int mask, int count)	asm("LUA00093");
LUA_API lua_Hook (lua_gethook) (lua_State *L)									asm("LUA00094");
LUA_API int (lua_gethookmask) (lua_State *L)									asm("LUA00095");
LUA_API int (lua_gethookcount) (lua_State *L)									asm("LUA00096");

LUA_API int (lua_setcstacklimit) (lua_State *L, unsigned int limit)				asm("LUA00097");

struct lua_Debug {
  int event;
  const char *name;	/* (n) */
  const char *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  const char *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  const char *source;	/* (S) */
  size_t srclen;	/* (S) */
  int currentline;	/* (l) */
  int linedefined;	/* (S) */
  int lastlinedefined;	/* (S) */
  unsigned char nups;	/* (u) number of upvalues */
  unsigned char nparams;/* (u) number of parameters */
  char isvararg;        /* (u) */
  char istailcall;	/* (t) */
  unsigned short ftransfer;   /* (r) index of first value transferred */
  unsigned short ntransfer;   /* (r) number of transferred values */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  struct CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


/******************************************************************************
* Copyright (C) 1994-2023 Lua.org, PUC-Rio.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

LUA_API void  (lua_arith) (lua_State *L, int op)								asm("LUA00098");

#endif
