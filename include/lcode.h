/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#include "llex.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
typedef enum BinOpr {
  /* arithmetic operators */
  OPR_ADD, OPR_SUB, OPR_MUL, OPR_MOD, OPR_POW,
  OPR_DIV, OPR_IDIV,
  /* bitwise operators */
  OPR_BAND, OPR_BOR, OPR_BXOR,
  OPR_SHL, OPR_SHR,
  /* string operator */
  OPR_CONCAT,
  /* comparison operators */
  OPR_EQ, OPR_LT, OPR_LE,
  OPR_NE, OPR_GT, OPR_GE,
  /* logical operators */
  OPR_AND, OPR_OR,
  OPR_NOBINOPR
} BinOpr;


/* true if operation is foldable (that is, it is arithmetic or bitwise) */
#define foldbinop(op)	((op) <= OPR_SHR)


#define luaK_codeABC(fs,o,a,b,c)	luaK_codeABCk(fs,o,a,b,c,0)


typedef enum UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC int luaK_code (FuncState *fs, Instruction i)							asm("LUAK0001");
LUAI_FUNC int luaK_codeABx (FuncState *fs, OpCode o, int A, unsigned int Bx)	asm("LUAK0002");
LUAI_FUNC int luaK_codeAsBx (FuncState *fs, OpCode o, int A, int Bx)			asm("LUAK0003");
LUAI_FUNC int luaK_codeABCk (FuncState *fs, OpCode o, int A,
                                            int B, int C, int k)				asm("LUAK0004");
LUAI_FUNC int luaK_isKint (expdesc *e)											asm("LUAK0005");
LUAI_FUNC int luaK_exp2const (FuncState *fs, const expdesc *e, TValue *v)		asm("LUAK0006");
LUAI_FUNC void luaK_fixline (FuncState *fs, int line)							asm("LUAK0007");
LUAI_FUNC void luaK_nil (FuncState *fs, int from, int n)						asm("LUAK0008");
LUAI_FUNC void luaK_reserveregs (FuncState *fs, int n)							asm("LUAK0009");
LUAI_FUNC void luaK_checkstack (FuncState *fs, int n)							asm("LUAK0010");
LUAI_FUNC void luaK_int (FuncState *fs, int reg, lua_Integer n)					asm("LUAK0011");
LUAI_FUNC void luaK_dischargevars (FuncState *fs, expdesc *e)					asm("LUAK0012");
LUAI_FUNC int luaK_exp2anyreg (FuncState *fs, expdesc *e)						asm("LUAK0013");
LUAI_FUNC void luaK_exp2anyregup (FuncState *fs, expdesc *e)					asm("LUAK0014");
LUAI_FUNC void luaK_exp2nextreg (FuncState *fs, expdesc *e)						asm("LUAK0015");
LUAI_FUNC void luaK_exp2val (FuncState *fs, expdesc *e)							asm("LUAK0016");
LUAI_FUNC int luaK_exp2RK (FuncState *fs, expdesc *e)							asm("LUAK0017");
LUAI_FUNC void luaK_self (FuncState *fs, expdesc *e, expdesc *key)				asm("LUAK0018");
LUAI_FUNC void luaK_indexed (FuncState *fs, expdesc *t, expdesc *k)				asm("LUAK0019");
LUAI_FUNC void luaK_goiftrue (FuncState *fs, expdesc *e)						asm("LUAK0020");
LUAI_FUNC void luaK_goiffalse (FuncState *fs, expdesc *e)						asm("LUAK0021");
LUAI_FUNC void luaK_storevar (FuncState *fs, expdesc *var, expdesc *e)			asm("LUAK0022");
LUAI_FUNC void luaK_setreturns (FuncState *fs, expdesc *e, int nresults)		asm("LUAK0023");
LUAI_FUNC void luaK_setoneret (FuncState *fs, expdesc *e)						asm("LUAK0024");
LUAI_FUNC int luaK_jump (FuncState *fs)											asm("LUAK0025");
LUAI_FUNC void luaK_ret (FuncState *fs, int first, int nret)					asm("LUAK0026");
LUAI_FUNC void luaK_patchlist (FuncState *fs, int list, int target)				asm("LUAK0027");
LUAI_FUNC void luaK_patchtohere (FuncState *fs, int list)						asm("LUAK0028");
LUAI_FUNC void luaK_concat (FuncState *fs, int *l1, int l2)						asm("LUAK0029");
LUAI_FUNC int luaK_getlabel (FuncState *fs)										asm("LUAK0030");
LUAI_FUNC void luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, int line)		asm("LUAK0031");
LUAI_FUNC void luaK_infix (FuncState *fs, BinOpr op, expdesc *v)				asm("LUAK0032");
LUAI_FUNC void luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, int line)								asm("LUAK0033");
LUAI_FUNC void luaK_settablesize (FuncState *fs, int pc,
                                  int ra, int asize, int hsize)					asm("LUAK0034");
LUAI_FUNC void luaK_setlist (FuncState *fs, int base, int nelems, int tostore)	asm("LUAK0035");
LUAI_FUNC void luaK_finish (FuncState *fs)										asm("LUAK0036");
LUAI_FUNC l_noret luaK_semerror (LexState *ls, const char *msg)					asm("LUAK0037");


#endif
