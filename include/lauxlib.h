/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#include <stddef.h>
#include <stdio.h>

#include "luaconf.h"
#include "lua.h"


/* global table */
#define LUA_GNAME	"_G"


typedef struct luaL_Buffer luaL_Buffer;


/* extra error code for 'luaL_loadfilex' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LUA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LUA_PRELOAD_TABLE	"_PRELOAD"


typedef struct luaL_Reg {
  const char *name;
  lua_CFunction func;
} luaL_Reg;


#define LUAL_NUMSIZES	(sizeof(lua_Integer)*16 + sizeof(lua_Number))

LUALIB_API void (luaL_checkversion_) (lua_State *L, lua_Number ver, size_t sz)		asm("LUAL0001");
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_NUM, LUAL_NUMSIZES)

LUALIB_API int (luaL_getmetafield) (lua_State *L, int obj, const char *e)			asm("LUAL0002");
LUALIB_API int (luaL_callmeta) (lua_State *L, int obj, const char *e)				asm("LUAL0003");
LUALIB_API const char *(luaL_tolstring) (lua_State *L, int idx, size_t *len)		asm("LUAL0004");
LUALIB_API int (luaL_argerror) (lua_State *L, int arg, const char *extramsg)		asm("LUAL0005");
LUALIB_API int (luaL_typeerror) (lua_State *L, int arg, const char *tname)			asm("LUAL0006");
LUALIB_API const char *(luaL_checklstring) (lua_State *L, int arg,
                                                          size_t *l)				asm("LUAL0007");
LUALIB_API const char *(luaL_optlstring) (lua_State *L, int arg,
                                          const char *def, size_t *l)				asm("LUAL0008");
LUALIB_API lua_Number (luaL_checknumber) (lua_State *L, int arg)					asm("LUAL0009");
LUALIB_API lua_Number (luaL_optnumber) (lua_State *L, int arg, lua_Number def)		asm("LUAL0010");

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, int arg)					asm("LUAL0011");
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, int arg,
                                          lua_Integer def)							asm("LUAL0012");

LUALIB_API void (luaL_checkstack) (lua_State *L, int sz, const char *msg)			asm("LUAL0013");
LUALIB_API void (luaL_checktype) (lua_State *L, int arg, int t)						asm("LUAL0014");
LUALIB_API void (luaL_checkany) (lua_State *L, int arg)								asm("LUAL0015");

LUALIB_API int   (luaL_newmetatable) (lua_State *L, const char *tname)				asm("LUAL0016");
LUALIB_API void  (luaL_setmetatable) (lua_State *L, const char *tname)				asm("LUAL0017");
LUALIB_API void *(luaL_testudata) (lua_State *L, int ud, const char *tname)			asm("LUAL0018");
LUALIB_API void *(luaL_checkudata) (lua_State *L, int ud, const char *tname)		asm("LUAL0019");

LUALIB_API void (luaL_where) (lua_State *L, int lvl)								asm("LUAL0020");
LUALIB_API int (luaL_error) (lua_State *L, const char *fmt, ...)					asm("LUAL0021");

LUALIB_API int (luaL_checkoption) (lua_State *L, int arg, const char *def,
                                   const char *const lst[])							asm("LUAL0022");

LUALIB_API int (luaL_fileresult) (lua_State *L, int stat, const char *fname)		asm("LUAL0023");
LUALIB_API int (luaL_execresult) (lua_State *L, int stat)							asm("LUAL0024");


/* predefined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API int (luaL_ref) (lua_State *L, int t)										asm("LUAL0025");
LUALIB_API void (luaL_unref) (lua_State *L, int t, int ref)							asm("LUAL0026");

LUALIB_API int (luaL_loadfilex) (lua_State *L, const char *filename,
                                               const char *mode)					asm("LUAL0027");

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,NULL)

LUALIB_API int (luaL_loadbufferx) (lua_State *L, const char *buff, size_t sz,
                                   const char *name, const char *mode)				asm("LUAL0028");
LUALIB_API int (luaL_loadstring) (lua_State *L, const char *s)						asm("LUAL0029");

LUALIB_API lua_State *(luaL_newstate) (void)										asm("LUAL0030");

LUALIB_API lua_Integer (luaL_len) (lua_State *L, int idx)							asm("LUAL0031");

LUALIB_API void (luaL_addgsub) (luaL_Buffer *b, const char *s,
                                     const char *p, const char *r)					asm("LUAL0032");
LUALIB_API const char *(luaL_gsub) (lua_State *L, const char *s,
                                    const char *p, const char *r)					asm("LUAL0033");

LUALIB_API void (luaL_setfuncs) (lua_State *L, const luaL_Reg *l, int nup)			asm("LUAL0034");

LUALIB_API int (luaL_getsubtable) (lua_State *L, int idx, const char *fname)		asm("LUAL0035");

LUALIB_API void (luaL_traceback) (lua_State *L, lua_State *L1,
                                  const char *msg, int level)						asm("LUAL0036");

LUALIB_API void (luaL_requiref) (lua_State *L, const char *modname,
                                 lua_CFunction openf, int glb)						asm("LUAL0037");

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((void)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((void)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), NULL))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), NULL))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,NULL)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#define luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#define luaL_pushfail(L)	lua_pushnil(L)


/*
** Internal assertions for in-house debugging
*/
#if !defined(lua_assert)

#if defined LUAI_ASSERT
  #include <assert.h>
  #define lua_assert(c)		assert(c)
#else
  #define lua_assert(c)		((void)0)
#endif

#endif



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

struct luaL_Buffer {
  char *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  union {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    char b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#define luaL_bufflen(bf)	((bf)->n)
#define luaL_buffaddr(bf)	((bf)->b)


#define luaL_addchar(B,c) \
  ((void)((B)->n < (B)->size || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

#define luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API void (luaL_buffinit) (lua_State *L, luaL_Buffer *B)					asm("LUAL0038");
LUALIB_API char *(luaL_prepbuffsize) (luaL_Buffer *B, size_t sz)				asm("LUAL0039");
LUALIB_API void (luaL_addlstring) (luaL_Buffer *B, const char *s, size_t l)		asm("LUAL0040");
LUALIB_API void (luaL_addstring) (luaL_Buffer *B, const char *s)				asm("LUAL0041");
LUALIB_API void (luaL_addvalue) (luaL_Buffer *B)								asm("LUAL0042");
LUALIB_API void (luaL_pushresult) (luaL_Buffer *B)								asm("LUAL0043");
LUALIB_API void (luaL_pushresultsize) (luaL_Buffer *B, size_t sz)				asm("LUAL0044");
LUALIB_API char *(luaL_buffinitsize) (lua_State *L, luaL_Buffer *B, size_t sz)	asm("LUAL0045");

#define luaL_prepbuffer(B)	luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

/* }====================================================== */



/*
** {======================================================
** File handles for IO library
** =======================================================
*/

/*
** A file handle is a userdata with metatable 'LUA_FILEHANDLE' and
** initial structure 'luaL_Stream' (it may contain other fields
** after that initial structure).
*/

#define LUA_FILEHANDLE          "FILE*"


typedef struct luaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;

/* }====================================================== */

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#if !defined(lua_writestring)
#define lua_writestring(s,l)   fwrite((s), sizeof(char), (l), stdout)
#endif

/* print a newline and flush the output */
#if !defined(lua_writeline)
#define lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#endif

/* print an error message */
#if !defined(lua_writestringerror)
#define lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#endif

/* }================================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#if defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((int)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((int)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */

#endif


