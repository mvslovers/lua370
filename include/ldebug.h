/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ifndef ldebug_h
#define ldebug_h


#include "lstate.h"


#define pcRel(pc, p)	(cast_int((pc) - (p)->code) - 1)


/* Active Lua function (given call info) */
#define ci_func(ci)		(clLvalue(s2v((ci)->func.p)))


#define resethookcount(L)	(L->hookcount = L->basehookcount)

/*
** mark for entries in 'lineinfo' array that has absolute information in
** 'abslineinfo' array
*/
#define ABSLINEINFO	(-0x80)


/*
** MAXimum number of successive Instructions WiTHout ABSolute line
** information. (A power of two allows fast divisions.)
*/
#if !defined(MAXIWTHABS)
#define MAXIWTHABS	128
#endif


LUAI_FUNC int luaG_getfuncline (const Proto *f, int pc)							asm("LUAG0001");
LUAI_FUNC const char *luaG_findlocal (lua_State *L, CallInfo *ci, int n,
                                                    StkId *pos)					asm("LUAG0002");
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, const TValue *o,
                                                const char *opname)				asm("LUAG0003");
LUAI_FUNC l_noret luaG_callerror (lua_State *L, const TValue *o)				asm("LUAG0004");
LUAI_FUNC l_noret luaG_forerror (lua_State *L, const TValue *o,
                                               const char *what)				asm("LUAG0005");
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, const TValue *p1,
                                                  const TValue *p2)				asm("LUAG0006");
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, const TValue *p1,
                                                 const TValue *p2,
                                                 const char *msg)				asm("LUAG0007");
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, const TValue *p1,
                                                 const TValue *p2)				asm("LUAG0008");
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, const TValue *p1,
                                                 const TValue *p2)				asm("LUAG0009");
LUAI_FUNC l_noret luaG_runerror (lua_State *L, const char *fmt, ...)			asm("LUAG0010");
LUAI_FUNC const char *luaG_addinfo (lua_State *L, const char *msg,
                                                  TString *src, int line)		asm("LUAG0011");
LUAI_FUNC l_noret luaG_errormsg (lua_State *L)									asm("LUAG0012");
LUAI_FUNC int luaG_traceexec (lua_State *L, const Instruction *pc)				asm("LUAG0013");


#endif
