/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#include "lprefix.h"


#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <time64.h>		/* 64 bit time */
#include <clibtry.h>	/* try() */

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#if !defined(LUA_STRFTIMEOPTIONS)	/* { */

#if defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#elif defined(LUA_USE_C89)  /* ANSI C 89 (only 1-char options) */
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#else  /* C99 specification */
#define LUA_STRFTIMEOPTIONS  "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
    "||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"  /* two-char options */
#endif

#endif					/* } */
/* }================================================================== */


/*
** {==================================================================
** Configuration for time-related stuff
** ===================================================================
*/

/*
** type to represent time_t in Lua
*/
#if !defined(LUA_NUMTIME)	/* { */

#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#define l_gettime(L,arg)	luaL_checkinteger(L, arg)

#else				/* }{ */

#define l_timet			lua_Number
#define l_pushtime(L,t)		lua_pushnumber(L,(lua_Number)(t))
#define l_gettime(L,arg)	luaL_checknumber(L, arg)

#endif				/* } */


#if !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#if defined(LUA_USE_POSIX)	/* { */

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#else				/* }{ */

/* ISO C definitions */
#define l_gmtime(t,r)		((void)(r)->tm_sec, gmtime(t))
#define l_localtime(t,r)	((void)(r)->tm_sec, localtime(t))

#endif				/* } */

#endif				/* } */

/* }================================================================== */


/*
** {==================================================================
** Configuration for 'tmpnam':
** By default, Lua uses tmpnam except when POSIX is available, where
** it uses mkstemp.
** ===================================================================
*/
#if !defined(lua_tmpnam)	/* { */

#if defined(LUA_USE_POSIX)	/* { */

#include <unistd.h>

#define LUA_TMPNAMBUFSIZE	32

#if !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        if (e != -1) close(e); \
        e = (e == -1); }

#else				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == NULL); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */


#if !defined(l_system)
#if defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#define l_system(cmd) ((cmd) == NULL ? 0 : -1)
#else
#define l_system(cmd)	system(cmd)  /* default definition */
#endif
#endif

static int os_wto(lua_State *L) 
{
	int 	top			= lua_gettop(L);  /* number of arguments */
	int 	i;

	// wtof("%s: top=%d", __func__, top);
	
	for (i = 1; i <= top; i++) {  /* for each argument */
		const char *s = lua_tostring(L, i);  /* convert it to string */
		// wtof("%s: i=%d) s=\"%s\"", __func__, i, s);
		if (s) {
			wtof("%s", s);
		}
		// lua_pop(L, 1);  /* pop result */
	}
	lua_settop(L, 0);	/* clear the stack */

	// wtof("%s: exit 0", __func__);
	
	return 0;	/* returns nothing */
}

static int os_wtor(lua_State *L) 
{
	int 	top			= lua_gettop(L);  /* number of arguments */
	int		results		= 0;
	int 	i;
	char	buf[120];

	// wtof("%s: top=%d", __func__, top);
	
	for (i = 1; i <= top; i++) {  /* for each argument */
		const char *s = lua_tostring(L, i);  /* convert it to string */
		// wtof("%s: i=%d) s=\"%s\"", __func__, i, s);
		if (s) {
			memset(buf, 0, sizeof(buf));
			wtor(buf, sizeof(buf), s);
			// wtof("%s: reply was \"%s\"", __func__, buf);
			lua_pushstring(L,buf);
			results++;
		}
	}

	// wtof("%s: exit %d", __func__, results);
	
	return results;	/* return number of reply srings we pushed on stack */
}

static int sendherc(const char *cmd, int cmdlen, char *resp, int respmax)
{
    int     rc;

    cmdlen = (int)(0x40000000 | cmdlen);

    __asm__("MODESET MODE=SUP,KEY=ZERO");

    __asm__("\n"
"         L     2,%1           Command buffer\n"
"         L     3,%3           Response buffer\n"
"         L     4,%2           Command length\n"
"         L     5,%4           Response length\n"
"         LRA   2,0(2)\n"
"         LRA   3,0(3)\n"
"         DC    X'83240008'    Issue command with DIAG8\n"
"         ST    4,%0           Save result\n"
"*" : "=m"(rc) : "m"(cmd), "m"(cmdlen), "m"(resp), "m"(respmax)
    : "2", "3", "4", "5");

    __asm__("MODESET MODE=PROB,KEY=NZERO");

    return rc;
}

static int os_herc(lua_State *L)
{
    int     	rc      = 0;
	const char *cmd 	= lua_tostring(L, 1);  /* convert it to string */
    int     	cmdlen  = cmd ? strlen(cmd) : 0;
    char 		resp[256];
    char    	*p;

    memset(resp, 0, sizeof(resp));

	/* We have to use try() in case we're not APF authorized */

	if (cmd && cmdlen) {
		// wtof("%s: calling sendcmd(\"%s\")", __func__, cmd);
		rc = try(sendherc, cmd, cmdlen, resp, sizeof(resp));
		if ((unsigned)rc == 0x00047000) {
			/* S047 Abend - Not Authorized */
			sprintf(resp, "ABEND S047 - Not APF Authorized");
		}
		else if (rc > 0xFFF) {
			sprintf(resp, "ABEND S%03X", (rc >> 12) & 0xFFF);
		}
		else if (rc) {
			sprintf(resp, "ABEND U%04u", (rc & 0xFFF));
		}
		else {
			// wtodumpf(resp, sizeof(resp), "RESP");
			resp[sizeof(resp)-1]=0;
			for(p=resp; *p; p++) {
				if (*p == 0x25) {
					*p = '\n';
				}
			}
		}

		// wtof("%s: rc=%d %08X resp=\"%s\"", __func__, rc, rc, resp);
	}

	if (resp[0]) {
		lua_pushstring(L, resp);
	}
	else {
		lua_pushnil(L);
	}

    return 1;
}

typedef struct plist34 {
	char	flag1;
#define FLAG1_HAS_FLAG2	0x80

	char	len;	/* cmd text len + 4 */
	short	flag2;
#define FLAG2_NONE		0x0000
#define FLAG2_PGM		0x0800
#define FLAG2_SEC		0x0008
#define FLAG2_PGM_SEC 	0x0808 

	char    text[126];
	char    unused[126];
} PLIST34;

static int sendcmd(const char *cmd, unsigned cmdlen)
{
    int     rc;
    PLIST34 plist = {0};

    memset(plist.text, ' ', sizeof(plist.text));
	if (cmdlen > sizeof(plist.text)) cmdlen = sizeof(plist.text);
	plist.len = cmdlen+4;
	memcpy(plist.text, cmd, cmdlen);

	// wtodumpf(&plist, sizeof(plist), "PLIST34");

    __asm__("MODESET MODE=SUP,KEY=ZERO");

    __asm__("\n"
"         SR    0,0            Clear R0\n"
"         LA    1,0(,%1)       Get address of parameter list\n"
"         SVC   34             Issue SVC 34\n"
"         ST    15,%0          Save result\n"
"*" : "=m"(rc) : "r"(&plist) );

    __asm__("MODESET MODE=PROB,KEY=NZERO");

    return rc;
}

static int os_cmd(lua_State *L)
{
    int     	rc      = 0;
	const char *cmd 	= lua_tostring(L, 1);  /* convert it to string */
    int     	cmdlen  = cmd ? strlen(cmd) : 0;
    char 		resp[80]={0};
    char    	*p;


	/* We have to use try() in case we're not APF authorized */

	if (cmd && cmdlen) {
		// wtof("%s: calling sendcmd(\"%s\")", __func__, cmd);
		
		rc = try(sendcmd, cmd, cmdlen);
#if 0		
		if ((unsigned)rc == 0x00047000) {
			/* S047 Abend - Not Authorized */
			sprintf(resp, "ABEND S047 - Not APF Authorized");
		}
		else if (rc > 0xFFF) {
			sprintf(resp, "ABEND S%03X", (rc >> 12) & 0xFFF);
		}
		else if (rc) {
			sprintf(resp, "ABEND U%04u", (rc & 0xFFF));
		}
		else {
			sprintf(resp, "SUCCESS");
		}

		wtof("%s: rc=%d %08X resp=\"%s\"", __func__, rc, rc, resp);
#endif
	}

	lua_pushinteger(L, rc);
	
#if 0
	if (resp[0]) {
		lua_pushstring(L, resp);
	}
	else {
		lua_pushnil(L);
	}
#endif

    return 1;
}

static int storage(void *ptr, int len, void *resp)
{
	// wtof("%s: ptr=%p len=%d resp=%p", __func__, ptr, len, resp);
	// wtodumpf(ptr, len, "%s", __func__);
	memcpy(resp, ptr, len);
	return 0;
}

static int os_storage(lua_State *L)
{
    int     	rc      = 0;
	const char *addr 	= lua_tostring(L, 1);  		/* convert it to string */
    int			len		= luaL_optinteger(L, 2, 1);
    char    	*resp	= NULL;
    void 		*ptr;

	if (!addr) goto failed;
	if (len < 1) goto failed;	/* return empty string */

	/* allocate a buffer for the storage we want to access */
	resp = calloc(1, len);
	if (!resp) goto failed;
	
	/* convert the address string to a pointer value */
	ptr = (void*)strtoul(addr, NULL, 16);

	/* try to retrieve the storage at this address */
	rc = try(storage,ptr,len,resp);
	if (rc) {
#if 0	/* debugging only */
		if ((unsigned)rc == 0x00047000) {
			/* S047 Abend - Not Authorized */
			wtof("%s: ABEND S047 - Not APF Authorized", __func__);
		}
		else if (rc > 0xFFF) {
			wtof("%s: ABEND S%03X", __func__, (rc >> 12) & 0xFFF);
		}
		else if (rc) {
			wtof("%s: ABEND U%04u", __func__, (rc & 0xFFF));
		}
#endif
		goto failed;
	}

	// wtodumpf(resp, len, "%s", __func__);
	lua_pushlstring(L, resp, len);
	goto quit;

failed:
	lua_pushstring(L, "");

quit:
	if (resp) free(resp);
    return 1;
}

static int os_c2x(lua_State *L)
{
    int     	i       = 0;
    size_t		len		= 0;
	const char *str 	= lua_tolstring(L, 1, &len);
	char 		*resp   = NULL;
	int			resplen = 0;
	char 		*p;
	
	if (!str) goto failed;
	
	resplen = len * 2;
	resp = calloc(1, resplen + 1);
	if (!resp) goto failed;

	for(i=0, p=resp; i < len; i++, p+=2) {
		sprintf(p, "%02X", str[i]);
	}
	
	lua_pushlstring(L, resp, resplen);
	goto quit;

failed:
	lua_pushstring(L, "");

quit:
	if (resp) free(resp);
	return 1;
}

typedef union {
	unsigned	n;
	int			i;
	char 		s[4];
} C2D;

static int os_c2d(lua_State *L)
{
    int     	i       = 0;
    size_t		len		= 0;
	const char *str 	= lua_tolstring(L, 1, &len);
	int 		bytes	= luaL_optinteger(L, 2, -1);
	C2D 		resp	= {0};

	// wtof("%s: str=%p len=%d bytes=%d", __func__, str, len, bytes);
	// wtodumpf(str, len, "str");
	
	if (!str) goto failed;
	if (!bytes) goto failed;
	if (bytes > 4) bytes = 4;
	
	for(i=0; i < len; i++) {
		resp.s[0] = resp.s[1];
		resp.s[1] = resp.s[2];
		resp.s[2] = resp.s[3];
		resp.s[3] = str[i];
	}

	// wtodumpf(&resp, sizeof(resp), "resp 1");

	if (bytes == 3 && resp.s[1] & 0x80) {
		resp.s[0] = 0xFF;
	}
	if (bytes == 2 && resp.s[2] & 0x80) {
		resp.s[0] = 0xFF;
		resp.s[1] = 0xFF;
	}
	if (bytes == 1 && resp.s[3] & 0x80) {
		resp.s[0] = 0xFF;
		resp.s[1] = 0xFF;
		resp.s[2] = 0xFF;
	}

	// wtodumpf(&resp, sizeof(resp), "resp 2");
	
	lua_pushinteger(L, resp.i);
	goto quit;

failed:
	lua_pushinteger(L, 0);

quit:
	return 1;
}

static int os_d2x(lua_State *L)
{
    int     	i       = 0;
	int 		value   = lua_tointeger(L, 1);
	int 		bytes	= luaL_optinteger(L, 2, -1);
	char 		resp[12]= {0};

	if (!bytes) goto failed;
	if (bytes > 8) bytes = 8;

	sprintf(resp, "%08X", value);
	if (bytes > 0) {
		char *p = &resp[8-bytes];
		lua_pushstring(L, p);
	}
	else {
		char *p = resp;
		while(*p=='0') p++;
		if (!*p) p = "0";
		lua_pushstring(L, p);
	}
	goto quit;

failed:
	lua_pushstring(L, "");

quit:
	return 1;
}

static int os_d2c(lua_State *L)
{
    int     	i       = 0;
	int 		value   = lua_tointeger(L, 1);
	int 		bytes	= luaL_optinteger(L, 2, -1);
	char 		resp[12]= {0};

	if (!bytes) goto failed;
	if (bytes > 4) bytes = 4;

	sprintf(resp, "%c%c%c%c", (value & 0xFF000000)>>24, 
		(value & 0x00FF0000)>>16, (value & 0x0000FF00) >> 8,
		(value & 0x000000FF));
	if (bytes > 0) {
		char *p = &resp[4-bytes];
		lua_pushlstring(L, p, bytes);
	}
	else {
		char *p;
		if (value > 0x00FFFFFF) {
			p = &resp[0];
			i = 4;
		}
		else if (value > 0x0000FFFF) {
			p = &resp[1];
			i = 3;
		}
		else if (value > 0x000000FF) {
			p = &resp[2];
			i = 2;
		}
		else {
			p = &resp[3];
			i = 1;
		}
		lua_pushlstring(L, p, i);
	}
	goto quit;

failed:
	lua_pushstring(L, "");

quit:
	return 1;
}

static int os_bitand(lua_State *L)
{
    int     	i       = 0;
    size_t		len1	= 0;
    size_t		len2	= 0;
    size_t		padlen	= 0;
	const char  *str1   = lua_tolstring(L, 1, &len1);
	const char 	*str2	= luaL_optlstring(L, 2, "", &len2);
	const char  *pad	= luaL_optlstring(L, 3, "", &padlen);
	int 		len		= len1 > len2 ? len1 : len2;
	char		*buf1	= calloc(1, len+1);
	char		*buf2	= calloc(1, len+1);
	char 		*resp	= calloc(1, len+1);
	
	memcpy(buf1, str1, len1);
	memcpy(buf2, str2, len2);
	
	if (!padlen) {
		pad = "\xff";
	}
	
	for(i=len1; i < len; i++) {
		buf1[i] = *pad;
	}
	for(i=len2; i < len; i++) {
		buf2[i] = *pad;
	}

	// wtodumpf(buf1, len, "%s: buf1", __func__);
	// wtodumpf(buf2, len, "%s: buf2", __func__);

	for(i=0; i < len; i++) {
		resp[i] = buf1[i] & buf2[i];
	}
	// wtodumpf(resp, len, "%s: resp", __func__);
	
	lua_pushlstring(L, resp, len);
	if (resp) free(resp);
	if (buf2) free(buf2);
	if (buf1) free(buf1);
	return 1;
}

static int os_bitor(lua_State *L)
{
    int     	i       = 0;
    size_t		len1	= 0;
    size_t		len2	= 0;
    size_t		padlen	= 0;
	const char  *str1   = lua_tolstring(L, 1, &len1);
	const char 	*str2	= luaL_optlstring(L, 2, "", &len2);
	const char  *pad	= luaL_optlstring(L, 3, "", &padlen);
	int 		len		= len1 > len2 ? len1 : len2;
	char		*buf1	= calloc(1, len+1);
	char		*buf2	= calloc(1, len+1);
	char 		*resp	= calloc(1, len+1);
	
	memcpy(buf1, str1, len1);
	memcpy(buf2, str2, len2);
	
	if (!padlen) {
		pad = "\x00";
	}
	
	for(i=len1; i < len; i++) {
		buf1[i] = *pad;
	}
	for(i=len2; i < len; i++) {
		buf2[i] = *pad;
	}

	// wtodumpf(buf1, len, "%s: buf1", __func__);
	// wtodumpf(buf2, len, "%s: buf2", __func__);

	for(i=0; i < len; i++) {
		resp[i] = buf1[i] | buf2[i];
	}
	// wtodumpf(resp, len, "%s: resp", __func__);
	
	lua_pushlstring(L, resp, len);
	if (resp) free(resp);
	if (buf2) free(buf2);
	if (buf1) free(buf1);
	return 1;
}

static int os_bitxor(lua_State *L)
{
    int     	i       = 0;
    size_t		len1	= 0;
    size_t		len2	= 0;
    size_t		padlen	= 0;
	const char  *str1   = lua_tolstring(L, 1, &len1);
	const char 	*str2	= luaL_optlstring(L, 2, "", &len2);
	const char  *pad	= luaL_optlstring(L, 3, "", &padlen);
	int 		len		= len1 > len2 ? len1 : len2;
	char		*buf1	= calloc(1, len+1);
	char		*buf2	= calloc(1, len+1);
	char 		*resp	= calloc(1, len+1);
	
	memcpy(buf1, str1, len1);
	memcpy(buf2, str2, len2);
	
	if (!padlen) {
		pad = "\x00";
	}
	
	for(i=len1; i < len; i++) {
		buf1[i] = *pad;
	}
	for(i=len2; i < len; i++) {
		buf2[i] = *pad;
	}

	// wtodumpf(buf1, len, "%s: buf1", __func__);
	// wtodumpf(buf2, len, "%s: buf2", __func__);

	for(i=0; i < len; i++) {
		resp[i] = buf1[i] ^ buf2[i];
	}
	// wtodumpf(resp, len, "%s: resp", __func__);
	
	lua_pushlstring(L, resp, len);
	if (resp) free(resp);
	if (buf2) free(buf2);
	if (buf1) free(buf1);
	return 1;
}

static int os_x2c(lua_State *L)
{
    int     	i       = 0;
    int			j		= 0;
    size_t		len1	= 0;
	const char  *str1   = lua_tolstring(L, 1, &len1);
	int 		len		= (len1 + 1) / 2;
	char 		*resp	= calloc(1, len+1);
	char		tmp[4];

	// wtodumpf(str1, len1, "%s: str1", __func__);
	
	for(i=0; i < len1; i+=2) {
		while (isspace(str1[i])) i++; /* skip spaces */
		tmp[0] = str1[i];
		tmp[1] = 0;
		if (i+1<len1) tmp[1] = str1[i+1];
		tmp[2] = 0;
		resp[j++] = (unsigned char) strtoul(tmp, NULL, 16);
	}	

	// wtodumpf(resp, j, "%s: resp", __func__);

	lua_pushlstring(L, resp, j);

quit:
	if (resp) free(resp);
	return 1;
}

static int os_x2d(lua_State *L)
{
    int     	i       = 0;
    int			j		= 0;
    size_t		len		= 0;
	const char *str 	= lua_tolstring(L, 1, &len);
	int 		bytes	= luaL_optinteger(L, 2, -1);
	char 		buf[12] = "00000000";
	char        *p;
	char		*x;
	unsigned    r;

	// wtof("%s: str=%p len=%d bytes=%d", __func__, str, len, bytes);
	// wtodumpf(str, len, "str");
	
	if (!str) goto failed;
	if (!bytes) goto failed;
	if (bytes > 8) bytes = 8;

	// wtodumpf(str, len, "%s: str", __func__);
	
	/* remove spaces from string */
	for(i=0; i < len; i++) {
		while(isspace(str[i]))i++;
		if (!str[i]) continue;
		buf[0] = buf[1];
		buf[1] = buf[2];
		buf[2] = buf[3];
		buf[3] = buf[4];
		buf[4] = buf[5];
		buf[5] = buf[6];
		buf[6] = buf[7];
		buf[7] = str[i];
	}

	// wtodumpf(buf, 8, "%s: buf", __func__);
	
	if (bytes > 0) {
		p = &buf[8-bytes];
	}
	else {
		p = buf;
	}

	if (bytes > 0) {
		if (strchr("89ABCDEF", toupper(*p))) {
			/* we need to sign extend the hex characters */
			// wtodumpf(buf, 8, "%s: buf before", __func__);
			for(x=buf; x < p; x++) {
				*x = 'F';
			}
			// wtodumpf(buf, 8, "%s: buf after", __func__);
			/* and point to the full buffer */
			p = buf;
		}
	}

	r = strtoul(p, NULL, 16);
	i = (int)r;
	// wtof("%s: r=%08X i=%d", __func__, r, i);
	
	// wtodumpf(p, strlen(p), "%s: p", __func__);
	// wtof("%s: i=%d %p", __func__, i, i);

	lua_pushinteger(L, i);
	goto quit;

failed:
	lua_pushinteger(L, 0);

quit:
	return 1;
}

#include "cliblist.h"
#include "clibary.h"
#include "racf.h"
typedef struct {
	lua_State	*L;
	int		    count;
} LISTC_UDATA;
static int os_listcat_prt(void *udata, const char *fmt, ...)
{
	LISTC_UDATA *listc 	= udata;
	lua_State 	*L 		= listc->L;
    va_list 	arg;
    char		buf[256];

    /* format the record passed to us by __listc() */
    va_start(arg, fmt);
    vsprintf(buf, fmt, arg);
    va_end(arg);

	// wtof("%s: \"%s\"", __func__, buf);

	listc->count++;
	lua_pushstring(L, buf);
	lua_seti(L,-2, listc->count);
	
	return 0;
}

static int os_listcat(lua_State *L)
{
	int			rc;
	const char *level = luaL_optstring(L, 1, "");
	const char *option = luaL_optstring(L, 2, "");
	LISTC_UDATA listc;
	char 		userid[12] = {0};
	
	listc.L = L;
	listc.count = 0;

	if (!level || !level[0]) {
		ACEE *acee = racf_get_acee();
		// wtodumpf(acee, sizeof(ACEE), "%s: ACEE", __func__);
		if (acee) {
			memcpy(userid, &acee->aceeuser[1], acee->aceeuser[0]);
			level = userid;
			// wtodumpf(userid, acee->aceeuser[0], "%s: userid", __func__);
		}
	}

	lua_newtable(L);
	
	rc = __listc(level, option, os_listcat_prt, &listc);
	// wtof("%s: __listc(\"%s\", \"%s\", os_listcat_prt, &listc) rc=%d",
	//  __func__, level, option, rc);

	lua_pushinteger(L,rc);
	lua_setfield(L,-2,"rc");

	lua_pushinteger(L, listc.count);
	lua_setfield(L,-2,"count");
	
	lua_pushstring(L, level);
	lua_setfield(L,-2,"level");
	
	lua_pushstring(L, option);
	lua_setfield(L,-2,"option");

	return 1;
}

static int dslist_setfield(lua_State *L, DSLIST *ds, unsigned n)
{
    char		buf[256];

	sprintf(buf, "%-44s %-6s %-2s %3s "
				 "%2u %5u %5u "
				 "%4u/%02u/%02u "
				 "%4u/%02u/%02u",
				 ds->dsn, ds->volser, ds->dsorg, ds->recfm,
				 ds->extents, ds->lrecl, ds->blksize,
				 ds->cryear, ds->crmon, ds->crday,
				 ds->rfyear, ds->rfmon, ds->rfday);
				 
	lua_pushstring(L, buf);
	lua_seti(L,-2, n);
	
	return 0;
}

static int os_dslist(lua_State *L)
{
	int			rc	= 0;
	const char *level = luaL_optstring(L, 1, "");
	const char *filter = luaL_optstring(L, 2, NULL);
	const char *option = "ALL";
	DSLIST     **dslist = NULL;
	unsigned	tally = 0;
	unsigned	count;
	unsigned	n;
	char 		userid[12] = {0};
	
	if (!level || !level[0]) {
		ACEE *acee = racf_get_acee();
		// wtodumpf(acee, sizeof(ACEE), "%s: ACEE", __func__);
		if (acee) {
			memcpy(userid, &acee->aceeuser[1], acee->aceeuser[0]);
			level = userid;
			// wtodumpf(userid, acee->aceeuser[0], "%s: userid", __func__);
		}
	}

	if (filter && !filter[0]) {
		/* filter appears to be empty string */
		filter = NULL;
	}

	lua_newtable(L);

	/* put heading in table */
	lua_pushstring(L, 
		"------------------ Dataset ----------------- "
		"Volser Org Fm Ex Lrecl Blksz Created    Last Ref");
	lua_setfield(L,-2, "heading");
	
	dslist = __listds(level, option, filter);

	count = array_count(&dslist);
	// wtodumpf(dslist, count*4, "%s: dslist", __func__);

	for(n=0; n < count; n++) {
		DSLIST *ds = dslist[n];
		
		if (!ds) continue;
		
		dslist_setfield(L, ds, n+1);
		tally++;
	}

	if (!count) rc = 4;
	
	lua_pushinteger(L, rc);
	lua_setfield(L,-2,"rc");
	
	lua_pushinteger(L, tally);
	lua_setfield(L,-2,"count");
	
	lua_pushstring(L, level);
	lua_setfield(L,-2,"level");
	
	if (!filter) filter = ""; 
	lua_pushstring(L, filter);
	lua_setfield(L,-2,"filter");
	
quit:
	if (dslist) __freeds(&dslist);

	return 1;
}

#if 0
static int vollist_setfield(lua_State *L, VOLLIST *v, unsigned n)
{
    char		buf[128];
	char 		status[16] = {0};
	char 		comment[40] = {0};
		
	if (v->status & VOLLIST_STATUS_PRV) strcpy(status, "PRIV ");
	else if (v->status & VOLLIST_STATUS_PUB) strcpy(status, "PUBL ");
	else if (v->status & VOLLIST_STATUS_STG) strcpy(status, "STRG ");
		
	if (v->status & VOLLIST_STATUS_PRES) strcat(status, "RSDNT");
	else if (v->status & VOLLIST_STATUS_RESV) strcat(status, "RESRV");
	else if (v->status & VOLLIST_STATUS_ONLI) strcat(status, "ONLIN");
		
	if (v->comment) {
		strncpy(comment, v->comment, sizeof(comment)-1);
		comment[sizeof(comment)-1] = 0;
	}
	else {
		strcpy(comment, "no comment");
	}
		
	sprintf(buf, "%.6s  %4u  %4u   %4u        %4u      %4u      %4X    %4X   %4X   %-12s %s\n",
		v->volser, v->freecyls, v->freetrks, v->freeexts, v->maxfreecyls, v->maxfreetrks, v->cuu, v->dasdtype,
		v->ucbdasd, status, comment);

	lua_pushstring(L, buf);
	lua_seti(L,-2, n);
	
	return 0;
}
#endif

static int vollist_tostring(lua_State *L)
{
	/* returns the formatted string for this vollist record table */
	lua_getfield(L,-1, "string");
	return 1;
}

static int vollist_settable(lua_State *L, VOLLIST *v, unsigned n)
{
    char		buf[128];
	char 		status[16] = {0};
	char 		comment[40] = {0};
		
	if (v->status & VOLLIST_STATUS_PRV) strcpy(status, "PRIV ");
	else if (v->status & VOLLIST_STATUS_PUB) strcpy(status, "PUBL ");
	else if (v->status & VOLLIST_STATUS_STG) strcpy(status, "STRG ");
		
	if (v->status & VOLLIST_STATUS_PRES) strcat(status, "RSDNT");
	else if (v->status & VOLLIST_STATUS_RESV) strcat(status, "RESRV");
	else if (v->status & VOLLIST_STATUS_ONLI) strcat(status, "ONLIN");
		
	if (v->comment) {
		strncpy(comment, v->comment, sizeof(comment)-1);
		comment[sizeof(comment)-1] = 0;
	}
	else {
		strcpy(comment, "no comment");
	}

	lua_newtable(L);

	lua_pushstring(L, v->volser);
	lua_setfield(L,-2,"volser");
	
	lua_pushinteger(L, v->freecyls);
	lua_setfield(L,-2,"freecyls");
	
	lua_pushinteger(L, v->freetrks);
	lua_setfield(L,-2,"freetrks");

	lua_pushinteger(L, v->freeexts);
	lua_setfield(L,-2,"freeexts");
	
	lua_pushinteger(L, v->maxfreecyls);
	lua_setfield(L,-2,"maxfreecyls");
	
	lua_pushinteger(L, v->maxfreetrks);
	lua_setfield(L,-2,"maxfreetrks");
	
	sprintf(buf, "%4X", v->cuu);
	lua_pushstring(L,buf);
	lua_setfield(L,-2,"cuu");
	
	sprintf(buf, "%4X", v->dasdtype);
	lua_pushstring(L,buf);
	lua_setfield(L,-2,"type");
	
	sprintf(buf, "%4X", v->ucbdasd);
	lua_pushstring(L,buf);
	lua_setfield(L,-2,"ucb");

	lua_pushstring(L, status);
	lua_setfield(L,-2,"status");
	
	lua_pushstring(L, comment);
	lua_setfield(L,-2,"comment");

	/* create the formmated string for this table record */
	sprintf(buf, "%.6s  %4u  %4u   %4u        %4u      %4u      %4X    %4X   %4X   %-12s %s\n",
		v->volser, v->freecyls, v->freetrks, v->freeexts, v->maxfreecyls, v->maxfreetrks, v->cuu, v->dasdtype,
		v->ucbdasd, status, comment);

	lua_pushstring(L, buf);
	lua_setfield(L,-2,"string");

	/* create a metatable to return the formatting string for this table */
	lua_newtable(L);
	lua_pushcfunction(L, vollist_tostring);
	lua_setfield(L,-2, "__tostring");
	lua_setmetatable(L,-2);
		
	lua_seti(L,-2, n);
	
	return 0;
}

static int os_vollist(lua_State *L)
{
	int			rc	= 0;
	const char *filter = luaL_optstring(L, 1, NULL);
	int 		dolspace = luaL_optinteger(L, 2, 1);
	const char *vatlst = luaL_optstring(L, 3, "VATLST00");
	VOLLIST    **vollist = NULL;
	unsigned	tally = 0;
	unsigned	count;
	unsigned	n;
	char 		userid[12] = {0};
	
	if (filter && !filter[0]) {
		/* filter appears to be empty string */
		filter = NULL;
	}

	/* create results table */
	lua_newtable(L);

	/* put heading's in table */
	lua_pushstring(L, 
		"VOLSER  FREE  FREE   FREE   LARGEST CONTIG FREE AREA  UNIT  DEVICE   UCB   --STATUS--   ------COMMENTS------\n");
	lua_setfield(L,-2, "heading1");

	lua_pushstring(L, 
		"        CYLS  TRKS  EXTENTS    CYLINDERS  TRACKS             TYPE\n");
	lua_setfield(L,-2, "heading2");
	
	vollist = __listvl(filter, dolspace, vatlst);

	count = array_count(&vollist);

	for(n=0; n < count; n++) {
		VOLLIST *v = vollist[n];
		
		if (!v) continue;

#if 1
		vollist_settable(L, v, n+1);
#else
		vollist_setfield(L, v, n+1);
#endif
		tally++;
	}

	if (!count) rc = 4;
	
	lua_pushinteger(L, rc);
	lua_setfield(L,-2,"rc");
	
	lua_pushinteger(L, tally);
	lua_setfield(L,-2,"count");
	
	lua_pushstring(L, filter);
	lua_setfield(L,-2,"filter");
	
	lua_pushinteger(L, dolspace);
	lua_setfield(L,-2,"dolspace");
	
	lua_pushstring(L, vatlst);
	lua_setfield(L,-2,"vatlst");
	
quit:
	if (vollist) __freevl(&vollist);

	return 1;
}


static int os_pdslist(lua_State *L)
{
	int			rc			= 0;
	int			first 		= 1;
	const char *dataset 	= lua_tostring(L, 1);
	const char *filter 		= luaL_optstring(L, 2, NULL);
	const char *option 		= "ALL";
	PDSLIST     **pdslist 	= NULL;
	unsigned	count;
	unsigned	n;
    char		buf[256];
	LOADSTAT    *loadstat   = (LOADSTAT *)buf;
	ISPFSTAT    *ispfstat   = (ISPFSTAT *)buf;

	if (filter && !filter[0]) {
		/* filter appears to be empty string */
		filter = NULL;
	}

	lua_newtable(L);
	

	pdslist = __listpd(dataset, filter);
	count = array_count(&pdslist);

	for(n=0; n < count; n++) {
		PDSLIST 	*pds = pdslist[n];
		char 		line[256];
		
		if (!pds) continue;

        if (pds->idc & PDSLIST_IDC_TTRS) {
			/* most likely a load member */
			if (first) {
				lua_pushstring(L,
					"Member   TTR    Size    Alias Of AC Entry  ---- Attributes ----");
 				lua_setfield(L,-2, "heading");
				first = 0;
			}

            __fmtloa(pds, loadstat);
            sprintf(line, "%-8.8s %s %s %-8.8s  %2s %s %s",
				loadstat->name, loadstat->ttr, loadstat->size,
				loadstat->aliasof, loadstat->ac, loadstat->ep,
				loadstat->attr);
		}
		else {
			/* most likely a non-load member */
			if (first) {
				lua_pushstring(L,
					"Member   TTR    VV.MM Created  - Last Modified -  Init  Size Mod Userid");
				lua_setfield(L,-2, "heading");
				first = 0;
			}

			__fmtisp(pds, ispfstat);
			sprintf(line, "%-8.8s %s %s %s %s %5s %5s %3s %s",
				ispfstat->name, ispfstat->ttr, ispfstat->ver,
				ispfstat->created, ispfstat->changed, ispfstat->init,
				ispfstat->size, ispfstat->mod, ispfstat->userid);
		}
		lua_pushstring(L, line);
		lua_seti(L,-2,n+1);
	}

	if (!count) rc = 4;
	
	lua_pushinteger(L, rc);
	lua_setfield(L,-2,"rc");
	
	lua_pushinteger(L, count);
	lua_setfield(L,-2,"count");
	
	lua_pushstring(L, dataset);
	lua_setfield(L,-2,"dataset");

	if (!filter) filter = ""; 
	lua_pushstring(L, filter);
	lua_setfield(L,-2,"filter");

quit:
	if (pdslist) __freepd(&pdslist);

	return 1;
}


static int os_execute (lua_State *L) {
  const char *cmd = luaL_optstring(L, 1, NULL);
  int stat;
  errno = 0;
  stat = l_system(cmd);
  if (cmd != NULL)
    return luaL_execresult(L, stat);
  else {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    return 1;
  }
}


static int os_remove (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  return luaL_fileresult(L, remove(filename) == 0, filename);
}


static int os_rename (lua_State *L) {
  const char *fromname = luaL_checkstring(L, 1);
  const char *toname = luaL_checkstring(L, 2);
  return luaL_fileresult(L, rename(fromname, toname) == 0, NULL);
}


static int os_tmpname (lua_State *L) {
  char buff[LUA_TMPNAMBUFSIZE];
  int err;
  lua_tmpnam(buff, err);
  if (l_unlikely(err))
    return luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  return 1;
}


static int os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  return 1;
}


static int os_clock (lua_State *L) {
#if 1
	__64 c;
	__64 sec;		/* seconds */
	__64 msec;		/* miliseconds (1000ms == 1s) */
	lua_Number res;

	/* Note: The clock() function in /crent370/clib/clock.c is broken
	 * and always returns -1, so we're going to use clock64() 
	 * in /crent470/time64/tm64clck.c and process it using __64 
	 * variables before calculating the result as a lua_Number 
	 * (double float) value.
	 */
	// wtof("%s: CLOCKS_PER_SEC=%u", __func__, CLOCKS_PER_SEC);
	// wtof("%s: clock()=%u", __func__, clock());
  
	c.u64 = clock64();
	__64_divmod_u32(&c, CLOCKS_PER_SEC, &sec, &msec);

	/* scale result as seconds.miliseconds value */
	res = ((lua_Number)sec.u32[1]) + (((lua_Number)msec.u32[1])*0.001 );

	// wtof("%s: clock64()=%lld sec=%lld rem=%lld res=%0.3f",
	// 	__func__, c, s, r, res);
	lua_pushnumber(L, res);
#else
	lua_pushnumber(L, ((lua_Number)clock())/(lua_Number)CLOCKS_PER_SEC);
#endif
	return 1;
}


/*
** {======================================================
** Time/Date operations
** { year=%Y, month=%m, day=%d, hour=%H, min=%M, sec=%S,
**   wday=%w+1, yday=%j, isdst=? }
** =======================================================
*/

/*
** About the overflow check: an overflow cannot occur when time
** is represented by a lua_Integer, because either lua_Integer is
** large enough to represent all int fields or it is not large enough
** to represent a time that cause a field to overflow.  However, if
** times are represented as doubles and lua_Integer is int, then the
** time 0x1.e1853b0d184f6p+55 would cause an overflow when adding 1900
** to compute the year.
*/
static void setfield (lua_State *L, const char *key, int value, int delta) {
  #if (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= INT_MAX)
    if (l_unlikely(value > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", key);
  #endif
  lua_pushinteger(L, (lua_Integer)value + delta);
  lua_setfield(L, -2, key);
}


static void setboolfield (lua_State *L, const char *key, int value) {
  if (value < 0)  /* undefined? */
    return;  /* does not set field */
  lua_pushboolean(L, value);
  lua_setfield(L, -2, key);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
static void setallfields (lua_State *L, struct tm *stm) {
  setfield(L, "year", stm->tm_year, 1900);
  setfield(L, "month", stm->tm_mon, 1);
  setfield(L, "day", stm->tm_mday, 0);
  setfield(L, "hour", stm->tm_hour, 0);
  setfield(L, "min", stm->tm_min, 0);
  setfield(L, "sec", stm->tm_sec, 0);
  setfield(L, "yday", stm->tm_yday, 1);
  setfield(L, "wday", stm->tm_wday, 1);
  setboolfield(L, "isdst", stm->tm_isdst);
}


static int getboolfield (lua_State *L, const char *key) {
  int res;
  res = (lua_getfield(L, -1, key) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_pop(L, 1);
  return res;
}


static int getfield (lua_State *L, const char *key, int d, int delta) {
  int isnum;
  int t = lua_getfield(L, -1, key);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  if (!isnum) {  /* field is not an integer? */
    if (l_unlikely(t != LUA_TNIL))  /* some other value? */
      return luaL_error(L, "field '%s' is not an integer", key);
    else if (l_unlikely(d < 0))  /* absent field; no default? */
      return luaL_error(L, "field '%s' missing in date table", key);
    res = d;
  }
  else {
    if (!(res >= 0 ? res - delta <= INT_MAX : INT_MIN + delta <= res))
      return luaL_error(L, "field '%s' is out-of-bound", key);
    res -= delta;
  }
  lua_pop(L, 1);
  return (int)res;
}


static const char *checkoption (lua_State *L, const char *conv,
                                ptrdiff_t convlen, char *buff) {
  const char *option = LUA_STRFTIMEOPTIONS;
  int oplen = 1;  /* length of options being checked */
  for (; *option != '\0' && oplen <= convlen; option += oplen) {
    if (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    else if (memcmp(conv, option, oplen) == 0) {  /* match? */
      memcpy(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      return conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  return conv;  /* to avoid warnings */
}


#if 1
static time64_t l_checktime (lua_State *L, int arg) {
	l_timet t = l_gettime(L, arg);
	time64_t r;

	r.u64 = (uint64_t)t;
	// wtof("%s: t=%g r.u64=%lld", __func__, t, r.u64);
	// luaL_argcheck(L, (time64_t)t == t, arg, "time out-of-bounds");

	return r;
}
#else
static time_t l_checktime (lua_State *L, int arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  return (time_t)t;
}
#endif

/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250


static int os_date (lua_State *L) 
{
#if 1
	size_t slen;
	const char *s = luaL_optlstring(L, 1, "%c", &slen);
	time64_t t = luaL_opt(L, l_checktime, 2, time64(NULL));
	const char *se = s + slen;  /* 's' end */
	struct tm tmr, *stm;

	// wtof("%s: s=%s t=%lld", __func__, s, t);

	if (*s == '!') {  /* UTC? */
		stm = gmtime64_r(&t, &tmr);
		// wtof("%s: gmtime64() stm=%p", __func__, stm);
		// wtodumpf(&tmr, sizeof(tmr), "tmr");
		s++;  /* skip '!' */
	}
	else {
		stm = localtime64_r(&t, &tmr);
		// wtof("%s: localtime64() stm=%p", __func__, stm);
		// wtodumpf(&tmr, sizeof(tmr), "tmr");
	}

	if (stm == NULL)  /* invalid date? */
		return luaL_error(L,
			"date result cannot be represented in this installation");

	if (strcmp(s, "*t") == 0) {
		lua_createtable(L, 0, 9);  /* 9 = number of fields */
		setallfields(L, stm);
	}
	else {
		char cc[4];  /* buffer for individual conversion specifiers */
		luaL_Buffer b;

		cc[0] = '%';
		luaL_buffinit(L, &b);
		while (s < se) {
			if (*s != '%')  /* not a conversion specifier? */
				luaL_addchar(&b, *s++);
			else {
				size_t reslen;
				char *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);

				s++;  /* skip '%' */
				s = checkoption(L, s, se - s, cc + 1);  /* copy specifier to 'cc' */
				reslen = strftime(buff, SIZETIMEFMT, cc, stm);
				luaL_addsize(&b, reslen);
			}
		}
		luaL_pushresult(&b);
	}
#else
	size_t slen;
	const char *s = luaL_optlstring(L, 1, "%c", &slen);
	time_t t = luaL_opt(L, l_checktime, 2, time(NULL));
	const char *se = s + slen;  /* 's' end */
	struct tm tmr, *stm;

	if (*s == '!') {  /* UTC? */
		stm = l_gmtime(&t, &tmr);
		s++;  /* skip '!' */
	}
	else
		stm = l_localtime(&t, &tmr);

	if (stm == NULL)  /* invalid date? */
		return luaL_error(L,
			"date result cannot be represented in this installation");

	if (strcmp(s, "*t") == 0) {
		lua_createtable(L, 0, 9);  /* 9 = number of fields */
		setallfields(L, stm);
	}
	else {
		char cc[4];  /* buffer for individual conversion specifiers */
		luaL_Buffer b;

		cc[0] = '%';
		luaL_buffinit(L, &b);
		while (s < se) {
			if (*s != '%')  /* not a conversion specifier? */
				luaL_addchar(&b, *s++);
			else {
				size_t reslen;
				char *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);

				s++;  /* skip '%' */
				s = checkoption(L, s, se - s, cc + 1);  /* copy specifier to 'cc' */
				reslen = strftime(buff, SIZETIMEFMT, cc, stm);
				luaL_addsize(&b, reslen);
			}
		}
		luaL_pushresult(&b);
	}
#endif	
	return 1;
}


static int os_time (lua_State *L) 
{
#if 1
	time64_t t;
	l_timet x;
	
	if (lua_isnoneornil(L, 1))  /* called without args? */
		t = time64(NULL);  /* get current time */
	else {
		struct tm ts;
		luaL_checktype(L, 1, LUA_TTABLE);
		lua_settop(L, 1);  /* make sure table is at the top */
		ts.tm_year = getfield(L, "year", -1, 1900);
		ts.tm_mon = getfield(L, "month", -1, 1);
		ts.tm_mday = getfield(L, "day", -1, 0);
		ts.tm_hour = getfield(L, "hour", 12, 0);
		ts.tm_min = getfield(L, "min", 0, 0);
		ts.tm_sec = getfield(L, "sec", 0, 0);
		ts.tm_isdst = getboolfield(L, "isdst");
		t = mktime64(&ts);
		setallfields(L, &ts);  /* update fields with normalized values */
	}

	x = (l_timet)t.u64;
	if (t.u64 != (uint64_t)x || t.u32[0]==0xFFFFFFFF)
	// if (t != (time_t)(l_timet)t || t == (time_t)(-1))
		return luaL_error(L,
			"time result cannot be represented in this installation");
	l_pushtime(L, x);
#else
	time_t t;
	if (lua_isnoneornil(L, 1))  /* called without args? */
		t = time(NULL);  /* get current time */
	else {
		struct tm ts;
		luaL_checktype(L, 1, LUA_TTABLE);
		lua_settop(L, 1);  /* make sure table is at the top */
		ts.tm_year = getfield(L, "year", -1, 1900);
		ts.tm_mon = getfield(L, "month", -1, 1);
		ts.tm_mday = getfield(L, "day", -1, 0);
		ts.tm_hour = getfield(L, "hour", 12, 0);
		ts.tm_min = getfield(L, "min", 0, 0);
		ts.tm_sec = getfield(L, "sec", 0, 0);
		ts.tm_isdst = getboolfield(L, "isdst");
		t = mktime(&ts);
		setallfields(L, &ts);  /* update fields with normalized values */
	}

	if (t != (time_t)(l_timet)t || t == (time_t)(-1))
		return luaL_error(L,
			"time result cannot be represented in this installation");
	l_pushtime(L, t);
#endif
	return 1;
}


static int os_difftime (lua_State *L) 
{
#if 1
	time64_t t1 = l_checktime(L, 1);
	time64_t t2 = l_checktime(L, 2);
	lua_pushnumber(L, (lua_Number)difftime64(t1, t2));
#else
	time_t t1 = l_checktime(L, 1);
	time_t t2 = l_checktime(L, 2);
	lua_pushnumber(L, (lua_Number)difftime(t1, t2));
#endif
	return 1;
}

/* }====================================================== */


static int os_setlocale (lua_State *L) {
  static const int cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  static const char *const catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", NULL};
  const char *l = luaL_optstring(L, 1, NULL);
  int op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  return 1;
}


static int os_exit (lua_State *L) {
  int status;
  if (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  else
    status = (int)luaL_optinteger(L, 1, EXIT_SUCCESS);
  if (lua_toboolean(L, 2))
    lua_close(L);
  if (L) exit(status);  /* 'if' to avoid warnings for unreachable 'return' */
  return 0;
}


static const luaL_Reg syslib[] = {
  {"bitand",    os_bitand},
  {"bitor",     os_bitor},
  {"bitxor",    os_bitxor},
  {"clock",     os_clock},
  {"cmd",       os_cmd},
  {"c2d",		os_c2d},
  {"c2x",		os_c2x},
  {"date",      os_date},
  {"difftime",  os_difftime},
  {"dslist",    os_dslist},
  {"d2c",       os_d2c},
  {"d2x",		os_d2x},
  {"execute",   os_execute},
  {"exit",      os_exit},
  {"getenv",    os_getenv},
  {"herc",		os_herc},
  {"listcat",   os_listcat},
  {"pdslist",   os_pdslist},
  {"remove",    os_remove},
  {"rename",    os_rename},
  {"setlocale", os_setlocale},
  {"storage",   os_storage},
  {"time",      os_time},
  {"tmpname",   os_tmpname},
  {"vollist",   os_vollist},
  {"wto",		os_wto},
  {"wtor",		os_wtor},
  {"x2c",       os_x2c},
  {"x2d",       os_x2d},
  {NULL, NULL}
};

/* }====================================================== */



LUAMOD_API int luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  return 1;
}

