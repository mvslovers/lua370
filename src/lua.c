/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#include "lprefix.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


#if !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#if !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX

/* LUA370 - we want a re-entrant LUA so we can't have writeable
 * static variables. We'll use the __getwsa() in the CRENT370
 * code base for our writeable storage area.
 */
#define WRITEABLE_STATIC 1
#if !WRITEABLE_STATIC
/* This is the original code */
static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;

#else 
/* This is our writeable static code */
#include "clib.h"
#include "clibio.h"
#include "osdcb.h"
typedef struct luawsa {
	lua_State	*globalL;
	const char	*progname;
} LUAWSA;

static LUAWSA luawsa = { NULL, LUA_PROGNAME };

static LUAWSA *getwsa(void)
{
    LUAWSA	*wsa = (LUAWSA*)__wsaget(&luawsa, sizeof(luawsa));

    return wsa;
}

#define globalL (getwsa()->globalL)
#define progname (getwsa()->progname)

#endif	/* WRITEABLE_STATIC */

/* LUA370 - We use a custtom__start() routine so that we can
 * avoid name collision with the standard SYSPRINT, SYSTERM and SYSIN
 * DD names.
 */
#include "ikjcppl.h"	/* CPPL typedef */
#include "clibstae.h"               /* C runtime recovery routines  */

#define MAXPARMS 50 /* maximum number of arguments we can handle */

/* @@CRT0 calls @@START, @@START calls MAIN */
int
__start(char *p, char *pgmname, int tsojbid, void **pgmr1)
{
	CLIBPPA		*ppa	= __ppaget();
    CLIBGRT     *grt    = __grtget();
    CPPL		*cppl	= NULL;		/* TSO CPPL */
    char 		stdoutdsn[12];
    char 		stderrdsn[12];
    char 		stdindsn[12];
    int			stdoutdyn	= 0;
    int			stderrdyn	= 0;
    int			stdindyn	= 0;
    int         x;
    int         argc;
    unsigned    u;
    char        *argv[MAXPARMS + 1];
    int         rc;
    int         parmLen = 0;
    int         progLen = 0;
    char        parmbuf[310];

    /* if something goes all wrong, capture it! */
    abendrpt(ESTAE_CREATE, DUMP_DEFAULT);

	/* If this is a TSO environment, get the CPPL address */
    parmLen = ((unsigned int)p[0] << 8) | (unsigned int)p[1];
    if ((parmLen > 0) && (p[2] == 0)) {
        progLen = (unsigned int)p[3];
        cppl = (CPPL *)pgmr1;

        // wtodumpf(cppl, sizeof(CPPL), "lua.c:%s: CPPL", __func__);
        // if (cppl->cpplcbuf) wtodumpf(cppl->cpplcbuf, cppl->cpplcbuf->cbuflen, "lua.c:%s: cpplcbuf", __func__);
		// if (cppl->cpplupt)  wtodumpf(cppl->cpplupt,  sizeof(UPT), "lua.c:%s: cpplupt", __func__);
		// if (cppl->cpplpscb) wtodumpf(cppl->cpplpscb, sizeof(PSCB), "lua.c:%s: cpplpscb", __func__);
		// if (cppl->cpplect)  wtodumpf(cppl->cpplect,  16, "lua.c:%s: cpplect", __func__);
    }
    
    // wtof("lua.c:%s: parmLen=%d progLen=%d", __func__, parmLen, progLen);

	/* default daatset names */
	strcpy(stdoutdsn, "*LUAOUT");
	strcpy(stderrdsn, "*LUAERR");
	strcpy(stdindsn, "*LUAIN");

    stdout = fopen(stdoutdsn, "w");
    // wtof("lua.c:%s: fopen(\"%s\",\"%s\") fp=0x%08X", __func__, stdoutdsn, "w", stdout);
    if (!stdout) {
		wtof("Unable to open %s DD for output", stdoutdsn);
		__exita(EXIT_FAILURE);
	}

    stderr = fopen(stderrdsn, "w");
    // wtof("lua.c:%s: fopen(\"%s\",\"%s\") fp=0x%08X", __func__, stderrdsn, "w", stderr);
    if (!stderr) {
		wtof("Unable to open %s DD for output", stderrdsn);
        fclose(stdout);
        __exita(EXIT_FAILURE);
    }

    stdin = fopen(stdindsn, "r");
#if 0 /* debugging */
    wtof("lua.c:%s: fopen(\"%s\",\"%s\") fp=0x%08X", __func__, stdindsn, "r", stdin);
    if (stdin) {
		wtodumpf(stdin, sizeof(FILE), "lua.c:%s: STDIN", __func__);
		wtodumpf(stdin->dcb, sizeof(DCB), "STDIN DCB");
	}
#endif
    if (!stdin) {
		stdin = fopen("'NULLFILE'", "r");
		// wtof("lua.c:%s: fopen(\"%s\",\"%s\") fp=0x%08X", __func__, "'NULLFILE'", "r", stdin);
	}
    if (!stdin) {
		wtof("Unable to open %s DD for input", stdindsn);
        fclose(stdout);
        fclose(stderr);
        __exita(EXIT_FAILURE);
    }

    /* load any environment variables */
    if (loadenv("dd:LUAENV")) {
        /* no LUAENV DD, try ENVIRON DD */
        loadenv("dd:ENVIRON");
    }

    /* initialize time zone offset for this thread */
    tzset();

    if (parmLen >= sizeof(parmbuf) - 2) {
        parmLen = sizeof(parmbuf) - 1 - 2;
    }
    if (parmLen < 0) parmLen = 0;

    /* We copy the parameter into our own area because
       the caller hasn't necessarily allocated room for
       a terminating NUL, nor is it necessarily correct
       to clobber the caller's area with NULs. */
    memset(parmbuf, 0, sizeof(parmbuf));
    if (cppl) {
		/* TSO */
        parmLen -= 4;
        memcpy(parmbuf, p+4, parmLen);
    }
    else {
        memcpy(parmbuf, p+2, parmLen);
    }

    // wtodumpf(parmbuf, parmLen, "lua.c:%s: parmbuf", __func__);
    
    p = parmbuf;

    if (pgmr1) {
        /* save the program parameter list values (max 10 pointers)
           note: the first pointer is always the raw EXEC PGM=...,PARM
           or CPPL (TSO) address.
        */
        for(x=0; x < 10; x++) {
			// wtodumpf(pgmr1[x], 16, "lua.c:%s: x=%d", __func__, x);
            u = (unsigned)pgmr1[x];
            /* add to array of pointers from caller */
            arrayadd(&grt->grtptrs, (void*)(u&0x7FFFFFFF));
            if (u&0x80000000) break; /* end of VL style address list */
        }
    }

    if (cppl) {
        argv[0] = p;
        for(x=0;x<=progLen;x++) {
            if (argv[0][x]==' ') {
                argv[0][x]=0;
                break;
            }
        }
        p += progLen;
    }
    else {       /* batch or tso "call" */
        argv[0] = pgmname;
        pgmname[8] = '\0';
        pgmname = strchr(pgmname, ' ');
        if (pgmname) *pgmname = '\0';
    }

    while (*p == ' ') p++;

    x = 1;
    if (*p) {
        while(x < MAXPARMS) {
            char srch = ' ';

            if (*p == '"') {
                p++;
                srch = '"';
            }
            argv[x++] = p;
            p = strchr(p, srch);
            if (!p) break;

            *p = '\0';
            p++;
            /* skip trailing blanks */
            while (*p == ' ') p++;
            if (*p == '\0') break;
        }
    }
    argv[x] = NULL;
    argc = x;

	/* NOTE: We did not set the TSO flag in the GRT to prevent
	 * fopen() from inserting a dataset "prefix" in the dataset
	 * named passed to fopen() as that messes with Lua processing.
	 */

	// wtof("lua.c:%s: calling main(%d,%p)", __func__, argc, argv);
    rc = main(argc, argv);

    /* remove ESTAE */
    abendrpt(ESTAE_DELETE, DUMP_DEFAULT);

	// wtof("lua.c:%s: calling __exit(%d)", __func__, rc);
    __exit(rc);
    return (rc);
}



#if defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
static void 
setsignal (int sig, void (*handler)(int)) 
{
	struct sigaction sa;

	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);  /* do not mask any signal */
	sigaction(sig, &sa, NULL);
}

#else           /* }{ */

#define setsignal            signal

#endif                               /* } */

/*
** Hook set by signal function to stop the interpreter.
*/
static void 
lstop (lua_State *L, lua_Debug *ar) 
{
	(void)ar;  /* unused arg. */

	lua_sethook(L, NULL, 0, 0);  /* reset hook */
	luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
static void 
laction (int i) 
{
	int flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;

	setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
	lua_sethook(globalL, lstop, flag, 1);
}


static void 
print_usage (const char *badoption) 
{
	lua_writestringerror("%s: ", progname);

	if (badoption[1] == 'e' || badoption[1] == 'l')
		lua_writestringerror("'%s' needs argument\n", badoption);
	else
		lua_writestringerror("unrecognized option '%s'\n", badoption);

	lua_writestringerror(
		"usage: %s [options] [script [args]]\n"
		"Available options are:\n"
		"  -e stat   execute string 'stat'\n"
		"  -i        enter interactive mode after executing 'script'\n"
		"  -l mod    require library 'mod' into global 'mod'\n"
		"  -l g=mod  require library 'mod' into global 'g'\n"
		"  -v        show version information\n"
		"  -E        ignore environment variables\n"
		"  -W        turn warnings on\n"
		"  --        stop handling options\n"
		"  -         stop handling options and execute stdin\n"
		,
		progname);
}


/*
** Prints an error message, adding the program name in front of it
** (if present)
*/
static void 
l_message (const char *pname, const char *msg) 
{
	if (pname) lua_writestringerror("%s: ", pname);
		lua_writestringerror("%s\n", msg);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'msghandler'.
*/
static int 
report (lua_State *L, int status) 
{
	if (status != LUA_OK) {
		const char *msg = lua_tostring(L, -1);
		
		l_message(progname, msg);
		lua_pop(L, 1);  /* remove message */
	}

	return status;
}


/*
** Message handler used to run all chunks
*/
static int 
msghandler (lua_State *L) 
{
	const char *msg = lua_tostring(L, 1);

	if (msg == NULL) {  /* is error object not a string? */
		if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
			lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
			return 1;  /* that is the message */
		else
			msg = lua_pushfstring(L, "(error object is a %s value)",
								luaL_typename(L, 1));
	}

	luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
	return 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
static int 
docall (lua_State *L, int narg, int nres) 
{
	int status;
	int base;
	
	// wtof("lua.c:%s: enter", __func__);

	base = lua_gettop(L) - narg;  /* function index */

	// wtof("lua.c:%s: lua_pushcfunction(L, msghandler)", __func__);
	lua_pushcfunction(L, msghandler);  /* push message handler */

	// wtof("lua.c:%s: lua_insert(L, base)", __func__);
	lua_insert(L, base);  /* put it under function and args */

	// wtof("lua.c:%s: globalL = L", __func__);
	globalL = L;  /* to be available to 'laction' */

	// wtof("lua.c:%s: setsignal(SIGINT, laction)", __func__);
	setsignal(SIGINT, laction);  /* set C-signal handler */

	// wtof("lua.c:%s: status = lua_pcall(L, narg, nres, base)", __func__);
	status = lua_pcall(L, narg, nres, base);

	// wtof("lua.c:%s: setsignal(SIGINT, SIG_DFL)", __func__);
	setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */

	// wtof("lua.c:%s: lua_remove(L, base)", __func__);
	lua_remove(L, base);  /* remove message handler from the stack */
	
	// wtof("lua.c:%s: exit status=%d", __func__, status);
	return status;
}


static void 
print_version (void) 
{
	lua_writestring(LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
	lua_writeline();
}


/*
** Create the 'arg' table, which stores all arguments from the
** command line ('argv'). It should be aligned so that, at index 0,
** it has 'argv[script]', which is the script name. The arguments
** to the script (everything after 'script') go to positive indices;
** other arguments (before the script name) go to negative indices.
** If there is no script name, assume interpreter's name as base.
** (If there is no interpreter's name either, 'script' is -1, so
** table sizes are zero.)
*/
static void 
createargtable (lua_State *L, char **argv, int argc, int script) 
{
	int i, narg;
	
	narg = argc - (script + 1);  /* number of positive indices */
	lua_createtable(L, narg, script + 1);

	for (i = 0; i < argc; i++) {
		lua_pushstring(L, argv[i]);
		lua_rawseti(L, -2, i - script);
	}

	lua_setglobal(L, "arg");
}


static int 
dochunk (lua_State *L, int status) 
{
	if (status == LUA_OK) status = docall(L, 0, 0);

	return report(L, status);
}


static int 
dofile (lua_State *L, const char *name) 
{
	return dochunk(L, luaL_loadfile(L, name));
}


static int 
dostring (lua_State *L, const char *s, const char *name) 
{
	return dochunk(L, luaL_loadbuffer(L, s, strlen(s), name));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
*/
static int 
dolibrary (lua_State *L, char *globname) 
{
	int status;
	char *modname = strchr(globname, '=');

	if (modname == NULL)  /* no explicit name? */
		modname = globname;  /* module name is equal to global name */
	else {
		*modname = '\0';  /* global name ends here */
		modname++;  /* module name starts after the '=' */
	}

	lua_getglobal(L, "require");
	lua_pushstring(L, modname);
	status = docall(L, 1, 1);  /* call 'require(modname)' */
	
	if (status == LUA_OK)
		lua_setglobal(L, globname);  /* globname = require(modname) */

	return report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
static int 
pushargs (lua_State *L) 
{
	int i, n;

	if (lua_getglobal(L, "arg") != LUA_TTABLE)
		luaL_error(L, "'arg' is not a table");

	n = (int)luaL_len(L, -1);
	luaL_checkstack(L, n + 3, "too many arguments to script");

	for (i = 1; i <= n; i++)
		lua_rawgeti(L, -i, i);

	lua_remove(L, -i);  /* remove table from the stack */
	return n;
}

static int ismember(const char *name)
{
	int		rc 	= 0;
	int		i, len;
	
	if (!name) goto quit;
	len = strlen(name);
	if (len > 8) goto quit;
	for(i=0 ; i < len; i++) {
		if (isalnum(name[i])) continue;
		if (strchr("@#$", name[i])) continue;
		goto quit;
	}
	
	rc = 1;	/* could be a member name */

quit:
	return rc;
}

static int readable (const char *filename) 
{
	FILE 	*f 		= NULL;
	
	/* try opening as a dataset */
	f = fopen(filename, "r");  /* try to open file */
	// wtof("httplua.c:%s: f=%p", __func__, f);
	if (f == NULL) goto fail;  /* open failed */
	fclose(f);
	goto okay;

fail:
	// wtof("httplua.c:%s: exit 0", __func__);
	return 0;

okay:
	// wtof("httplua.c:%s: exit 1", __func__);
	return 1;
}

static char *make_pathnames(const char *paths, const char *script)
{
	char 	*pathname = NULL;
	int		pathcount = 1;
	int     scriptlen = strlen(script);
	int		i;
	char    *p;

	// wtof("httplua.c:%s: enter paths=\"%s\" script=\"%s\"", __func__, paths, script);
	
	for(p=strchr(paths, ';'); p; p=strchr(p+1, ';')) {
		pathcount++;
	}

	pathname = calloc(1, strlen(paths) + (scriptlen * pathcount));
	if (!pathname) goto quit;
	
	for(p=pathname; *paths; paths++) {
		*p = *paths;
		if (*p=='?') {
			strcpy(p, script);
			p+=scriptlen;
		}
		else {
			p++;
		}
	}

quit:
	// wtof("httplua.c:%s: exit pathname=\"%s\"", __func__, pathname);
	return pathname;
}

static int 
handle_script (lua_State *L, char **argv) 
{
	int status;
	int			top;
	const char 	*fname = argv[0];
	char		dataset[56] = {0};
	char 		*path       = NULL;
	char		*pathnames  = NULL;
	const char 	*name;
	char 		*rest;

	if (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
		fname = NULL;  /* stdin */

	/* if fname is not a member name then doit */
	if (!ismember(name)) goto doit;

	/* okay, fname is likely a dataset member name.
	 * so we're going to use the package.path string as the path
	 * name to find this member.
	 */
	top = lua_gettop(L);
	lua_getglobal( L, "package" );
    lua_getfield( L, -1, "path" ); // set the field "path" in table at -2 with value at top of stack
	name = lua_tostring(L, -1);
	if (name && *name) 	path = strdup(name);
	lua_settop(L, top);

	if (!path) goto doit;

	pathnames = make_pathnames(path, fname);
	if (!pathnames) goto doit;
		
	for(name=strtok(pathnames, ";"); name; name = strtok(rest, ";")) {
		rest = strtok(NULL,"");
		// wtof("%s: name=\"%s\" rest=\"%s\"", __func__, name, rest);
		if (readable(name)) {
			strcpy(dataset, name);
			fname = dataset;
			break;
		}
	}

	free(pathnames);

doit:
	status = luaL_loadfile(L, fname);
	if (status == LUA_OK) {
		int n = pushargs(L);  /* push arguments to script */
		status = docall(L, n, LUA_MULTRET);
	}
	
	return report(L, status);
}


/* bits of various argument indicators in 'args' */
#define has_error	1	/* bad option */
#define has_i		2	/* -i */
#define has_v		4	/* -v */
#define has_e		8	/* -e */
#define has_E		16	/* -E */


/*
** Traverses all arguments from 'argv', returning a mask with those
** needed before running any Lua code or an error code if it finds any
** invalid argument. In case of error, 'first' is the index of the bad
** argument.  Otherwise, 'first' is -1 if there is no program name,
** 0 if there is no script name, or the index of the script name.
*/
static int 
collectargs (char **argv, int *first) 
{
	int args = 0;
	int i;

	if (argv[0] != NULL) {  /* is there a program name? */
		if (argv[0][0])  /* not empty? */
			progname = argv[0];  /* save it */
	}
	else {  /* no program name */
		*first = -1;
		return 0;
	}

	for (i = 1; argv[i] != NULL; i++) {  /* handle arguments */
		*first = i;
		if (argv[i][0] != '-')  /* not an option? */
			return args;  /* stop handling options */

		switch (argv[i][1]) {  /* else check option */
			case '-':  /* '--' */
				if (argv[i][2] != '\0')  /* extra characters after '--'? */
					return has_error;  /* invalid option */
				*first = i + 1;
				return args;
				
			case '\0':  /* '-' */
				return args;  /* script "name" is '-' */

			case 'E':
				if (argv[i][2] != '\0')  /* extra characters? */
					return has_error;  /* invalid option */
				
				args |= has_E;
				break;

			case 'W':
				if (argv[i][2] != '\0')  /* extra characters? */
					return has_error;  /* invalid option */
				break;

			case 'i':
				args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
			case 'v':
				if (argv[i][2] != '\0')  /* extra characters? */
					return has_error;  /* invalid option */
				args |= has_v;
				break;

			case 'e':
				args |= has_e;  /* FALLTHROUGH */
			
			case 'l':  /* both options need an argument */
				if (argv[i][2] == '\0') {  /* no concatenated argument? */
					i++;  /* try next 'argv' */
					if (argv[i] == NULL || argv[i][0] == '-')
						return has_error;  /* no next argument or it is another option */
				}
				break;

			default:  /* invalid option */
				return has_error;
		}
	}
  
	*first = 0;  /* no script name */
	return args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
static int 
runargs (lua_State *L, char **argv, int n) 
{
	int i;

	for (i = 1; i < n; i++) {
		int option = argv[i][1];
		lua_assert(argv[i][0] == '-');  /* already checked */
		switch (option) {
			case 'e':  case 'l': {
				int status;
				char *extra = argv[i] + 2;  /* both options need an argument */
				if (*extra == '\0') extra = argv[++i];
				lua_assert(extra != NULL);
				status = (option == 'e')
						? dostring(L, extra, "=(command line)")	
						: dolibrary(L, extra);
				if (status != LUA_OK) return 0;
				break;
			}
			case 'W':
				lua_warning(L, "@on", 0);  /* warnings on */
				break;
		}
	}
  
	return 1;
}


static int 
handle_luainit (lua_State *L) 
{
	const char *name = "=" LUA_INITVARVERSION;
	const char *init = getenv(name + 1);

	if (init == NULL) {
		name = "=" LUA_INIT_VAR;
		init = getenv(name + 1);  /* try alternative name */
	}

	if (init == NULL) return LUA_OK;

	if (init[0] == '@')
		return dofile(L, init+1);
	else
		return dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#if !defined(LUA_PROMPT)
#define LUA_PROMPT		"LUA370 (\"//\" for EOF) >"
#endif

#if !defined(LUA_PROMPT2)
#define LUA_PROMPT2		LUA_PROMPT ">"
#endif

#if !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


static int is_interactive(FILE *fp)
{
	int		rc	 = 0;
	
	if (fp) {
		if (fp->flags & _FILE_FLAG_TERM) {
			rc = 1;
		}
	}
	
	return rc;
}
#define lua_stdin_is_tty() is_interactive(stdin)

/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#if !defined(lua_stdin_is_tty)	/* { */

#if defined(LUA_USE_POSIX)	/* { */

#include <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#elif defined(LUA_USE_WINDOWS)	/* }{ */

#include <io.h>
#include <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#else				/* }{ */

/* ISO C definition */
#define lua_stdin_is_tty()	1  /* assume stdin is a tty */

#endif				/* } */

#endif				/* } */


/*
** lua_readline defines how to show a prompt and then read a line from
** the standard input.
** lua_saveline defines how to "save" a read line in a "history".
** lua_freeline defines how to free a line read by lua_readline.
*/
#if !defined(lua_readline)	/* { */

#if defined(LUA_USE_READLINE)	/* { */

#include <readline/readline.h>
#include <readline/history.h>
#define lua_initreadline(L)	((void)L, rl_readline_name="lua")
#define lua_readline(L,b,p)	((void)L, ((b)=readline(p)) != NULL)
#define lua_saveline(L,line)	((void)L, add_history(line))
#define lua_freeline(L,b)	((void)L, free(b))

#else				/* }{ */

#define lua_initreadline(L)  ((void)L)
#define lua_readline(L,b,p) \
        ((void)L, fputs(p, stdout), fflush(stdout),  /* show prompt */ \
        fgets(b, LUA_MAXINPUT, stdin) != NULL)  /* get line */
#define lua_saveline(L,line)	{ (void)L; (void)line; }
#define lua_freeline(L,b)	{ (void)L; (void)b; }

#endif				/* } */

#endif				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
static const char *
get_prompt (lua_State *L, int firstline) 
{
	if (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
		return (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
	else {  /* apply 'tostring' over the value */
		const char *p = luaL_tolstring(L, -1, NULL);
		lua_remove(L, -2);  /* remove original value */
		return p;
	}
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(sizeof(EOFMARK)/sizeof(char) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
static int 
incomplete (lua_State *L, int status) 
{
	if (status == LUA_ERRSYNTAX) {
		size_t lmsg;
		const char *msg = lua_tolstring(L, -1, &lmsg);

		if (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0) {
			lua_pop(L, 1);
			return 1;
		}
	}

	return 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
static int 
pushline (lua_State *L, int firstline) 
{
	int			status		= 0;
	char 		buffer[LUA_MAXINPUT] = {0};
	char 		*b 			= buffer;
	size_t 		l;
	const char 	*prmt;
	int 		readstatus;

	// wtof("lua.c:%s: enter", __func__);

	// wtof("lua.c:%s: prmt = get_prompt(L, firstline)", __func__);
	prmt 		= get_prompt(L, firstline);
	// wtof("lua.c:%s: readstatus = lua_readline(L, b, prmt)", __func__);
	readstatus 	= lua_readline(L, b, prmt);
	if (readstatus == 0) {
		status = 0;  /* no input (prompt will be popped by caller) */
		goto quit;
	}

	// wtof("lua.c:%s: lua_pop(L, 1)", __func__);
	lua_pop(L, 1);  /* remove prompt */
	// wtof("lua.c:%s: l = strlen(b)", __func__);
	l = strlen(b);
	// wtof("lua.c:%s: ... l=%d", __func__, l);

	// wtof("lua.c:%s: if (l > 0 && b[l-1] == '\\n')", __func__);
	if (l > 0 && b[l-1] == '\n') { /* line ends with newline? */
		// wtof("lua.c:%s: b[--l] = '\\0'", __func__);
		b[--l] = '\0';  /* remove it */
	}

	// wtof("lua.c:%s: if (firstline && b[0] == '=')", __func__);
	if (firstline && b[0] == '=') { /* for compatibility with 5.2, ... */
		// wtof("lua.c:%s: lua_pushfstring(L, \"return %s\", b + 1)", __func__);
		lua_pushfstring(L, "return %s", b + 1);  /* change '=' to 'return' */
	}
	else {
		// wtof("lua.c:%s: lua_pushlstring(L, b, l)", __func__);
		lua_pushlstring(L, b, l);
	}

	// wtof("lua.c:%s: lua_freeline(L, b)", __func__);
	lua_freeline(L, b);
	status = 1;

quit:
	// wtof("lua.c:%s: exit status=%d", __func__, status);
	return status;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static int 
addreturn (lua_State *L) 
{
	const char *line;
	const char *retline;
	int status;
	
	// wtof("lua.c:%s: enter", __func__);

	// wtof("lua.c:%s: line = lua_tostring(L, -1)", __func__);
	line = lua_tostring(L, -1);  /* original line */
	// wtof("lua.c:%s: ... line=\"%s\"", __func__, line);

	// wtof("lua.c:%s: retline = lua_pushfstring(L, \"return %s;\", line)", __func__);
	retline = lua_pushfstring(L, "return %s;", line);
	// wtof("lua.c:%s: ... retline=\"%s\"", __func__, retline);
	
	// wtof("lua.c:%s: status = luaL_loadbuffer(L, retline, strlen(retline), \"=stdin\")", __func__);
	status = luaL_loadbuffer(L, retline, strlen(retline), "=stdin");
	// wtof("lua.c:%s: ... status=%d", __func__, status);

	// wtof("lua.c:%s: if (status == LUA_OK)", __func__);
	if (status == LUA_OK) {
		// wtof("lua.c:%s: lua_remove(L, -2)", __func__);
		lua_remove(L, -2);  /* remove modified line */
		// wtof("lua.c:%s: if (line[0] != '\\0')", __func__);
		if (line[0] != '\0') { /* non empty? */
			// wtof("lua.c:%s: lua_saveline(L, line)", __func__);
			lua_saveline(L, line);  /* keep history */
		}
	}
	else {
		// wtof("lua.c:%s: lua_pop(L, 2)", __func__);
		lua_pop(L, 2);  /* pop result from 'luaL_loadbuffer' and modified line */
	}

quit:
	// wtof("lua.c:%s: exit status=%d", __func__, status);
	return status;
}


/*
** Read multiple lines until a complete Lua statement
*/
static int 
multiline (lua_State *L) 
{
	int		status = 0;
	
	// wtof("lua.c:%s: enter", __func__);

	for (;;) {  /* repeat until gets a complete statement */
		size_t len;
		const char *line = lua_tolstring(L, 1, &len);  /* get what it has */

		// wtof("lua.c:%s: line=\"%s\"", __func__, line);
		// wtof("lua.c:%s: status = luaL_loadbuffer(L, line, len, \"=stdin\")", __func__);
		status = luaL_loadbuffer(L, line, len, "=stdin");  /* try it */

		// wtof("lua.c:%s: if (!incomplete(L, status) || !pushline(L, 0))", __func__);
		if (!incomplete(L, status) || !pushline(L, 0)) {
			// wtof("lua.c:%s: lua_saveline(L, line)", __func__);
			lua_saveline(L, line);  /* keep history */
			goto quit;  /* cannot or should not try to add continuation line */
		}

		// wtof("lua.c:%s: lua_pushliteral(L, \"\\n\")", __func__);
		lua_pushliteral(L, "\n");  /* add newline... */
		lua_insert(L, -2);  /* ...between the two lines */
		lua_concat(L, 3);  /* join them */
	}

quit:
	// wtof("lua.c:%s: exit status=%d", __func__, status);
	return status;
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
static int 
loadline (lua_State *L) 
{
	int status;

	// wtof("lua.c:%s: enter", __func__);
	
	// wtof("lua.c:%s: lua_settop(L, 0)", __func__);
	lua_settop(L, 0);

	// wtof("lua.c:%s: if (!pushline(L, 1))", __func__);
	if (!pushline(L, 1)) {
		// wtof("lua.c:%s: status = -1", __func__);
		status = -1;  /* no input */
		goto quit;
	}

	// wtof("lua.c:%s: if ((status = addreturn(L)) != LUA_OK)", __func__);
	if ((status = addreturn(L)) != LUA_OK) { /* 'return ...' did not work? */
		// wtof("lua.c:%s: status = multiline(L)", __func__);
		status = multiline(L);  /* try as command, maybe with continuation lines */
	}

	// wtof("lua.c:%s: lua_remove(L, 1)", __func__);
	lua_remove(L, 1);  /* remove line from the stack */
	// wtof("lua.c:%s: lua_assert(lua_gettop(L) == 1)", __func__);
	lua_assert(lua_gettop(L) == 1);

quit:
	// wtof("lua.c:%s: exit status=%d", __func__, status);
	return status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void 
l_print (lua_State *L) 
{
	int n = lua_gettop(L);

	if (n > 0) {  /* any result to be printed? */
		luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
		lua_getglobal(L, "print");
		lua_insert(L, 1);
		if (lua_pcall(L, n, 0, 0) != LUA_OK)
			l_message(progname, lua_pushfstring(L, 
				"error calling 'print' (%s)", lua_tostring(L, -1)));
	}
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
static void 
doREPL (lua_State *L) 
{
	int status;
	const char *oldprogname = progname;

	// wtof("lua.c:%s: enter", __func__);
	progname = NULL;  /* no 'progname' on errors in interactive mode */

	// wtof("lua.c:%s: lua_initreadline(L)", __func__);
	lua_initreadline(L);
	// wtof("lua.c:%s: while ((status = loadline(L)) != -1)", __func__);
	// while ((status = loadline(L)) != -1) {
	for (status = loadline(L); status != -1; status = loadline(L)) {
		// wtof("lua.c:%s: ... status=%d", __func__, status);
		if (status == LUA_OK) {
			// wtof("lua.c:%s: status = docall(L, 0, LUA_MULTRET)", __func__);
			status = docall(L, 0, LUA_MULTRET);
		}
		
		// wtof("lua.c:%s: status=%d", __func__, status);
		if (status == LUA_OK) {
			// wtof("lua.c:%s: l_print(L)", __func__);
			l_print(L);
		}
		else {
			// wtof("lua.c:%s: report(L, status)", __func__);
			report(L, status);
		}
	}

	// wtof("lua.c:%s: lua_settop(L, 0)", __func__);
	lua_settop(L, 0);  /* clear stack */
	// wtof("lua.c:%s: lua_writeline()", __func__);
	lua_writeline();
	progname = oldprogname;

	// wtof("lua.c:%s: exit", __func__);
}

/* }================================================================== */


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
static int 
pmain (lua_State *L) 
{
	int		rc		= 0;
	int 	argc 	= (int)lua_tointeger(L, 1);
	char 	**argv 	= (char **)lua_touserdata(L, 2);
	int 	script;
	int 	args 	= collectargs(argv, &script);
	int 	optlim 	= (script > 0) ? script : argc; /* first argv not an option */

	// wtof("lua.c:%s: enter", __func__);

	luaL_checkversion(L);  /* check that interpreter has correct version */
	if (args == has_error) {  /* bad arg? */
		// wtof("lua.c:%s: args == has_error", __func__);
		print_usage(argv[script]);  /* 'script' has index of bad arg. */
		rc = 0;
		goto quit;
	}

	if (args & has_v) { /* option '-v'? */
		// wtof("lua.c:%s: args & has_v", __func__);
		print_version();
	}

	if (args & has_E) {  /* option '-E'? */
		// wtof("lua.c:%s: args & has_e", __func__);
		lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
		lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
	}

	// wtof("lua.c:%s: luaL_openlibs()", __func__);
	luaL_openlibs(L);  /* open standard libraries */
	// wtof("lua.c:%s: createargtable()", __func__);
	createargtable(L, argv, argc, script);  /* create table 'arg' */
	// wtof("lua.c:%s: lua_gc(L, LUA_GCRESTART)", __func__);
	lua_gc(L, LUA_GCRESTART);  /* start GC... */
	// wtof("lua.c:%s: lua_gc(L, LUA_GCGEN, 0, 0)", __func__);
	lua_gc(L, LUA_GCGEN, 0, 0);  /* ...in generational mode */
	
	if (!(args & has_E)) {  /* no option '-E'? */
		// wtof("lua.c:%s: if (!(args & has_E))", __func__);
		if (handle_luainit(L) != LUA_OK) { /* run LUA_INIT */
			// wtof("lua.c:%s: if (handle_luainit(L) != LUA_OK)", __func__);
			rc = 0;  /* error running LUA_INIT */
			goto quit;
		}
	}

	if (!runargs(L, argv, optlim)) { /* execute arguments -e and -l */
		// wtof("lua.c:%s: if (!runargs(L, argv, optlim))", __func__);
		rc = 0;  /* something failed */
		goto quit;
	}

	if (script > 0) {  /* execute main script (if there is one) */
		// wtof("lua.c:%s: if (script > 0)", __func__);
		if (handle_script(L, argv + script) != LUA_OK) {
			// wtof("lua.c:%s: if (handle_script(L, argv + script) != LUA_OK)", __func__);
			rc = 0;  /* interrupt in case of error */
			goto quit;
		}
	}

	if (args & has_i) { /* -i option? */
		// wtof("lua.c:%s: if (args & has_i)", __func__);
		doREPL(L);  /* do read-eval-print loop */
	}
	else if (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
		// wtof("lua.c:%s: if (script < 1 && !(args & (has_e | has_v)))", __func__);
		if (lua_stdin_is_tty()) {  /* running in interactive mode? */
			// wtof("lua.c:%s: if (lua_stdin_is_tty())", __func__);
			print_version();
			doREPL(L);  /* do read-eval-print loop */
		}
		else {
			// wtof("lua.c:%s: if (!lua_stdin_is_tty())", __func__);
			dofile(L, NULL);  /* executes stdin as a file */
		}
	}

	lua_pushboolean(L, 1);  /* signal no errors */
	rc = 1;
	
quit:
	// wtof("lua.c:%s: exit rc=%d", __func__, rc);
	return rc;
}

int 
main (int argc, char **argv) 
{
	int	rc;
	int	i;
	int status, result;

#if 0
	wtof("lua.c:%s: argc=%d argv=0x%08X", __func__, argc, argv);
	for(i=0; i<argc; i++) {
		wtof("lua.c:%s: argv[%d]: \"%s\"", __func__, i, argv[i]);
	}
#endif

	lua_State *L = luaL_newstate();  /* create state */
	// wtof("lua.c:%s: luaL_newstate() L=0x%08X", __func__, L);

	if (L == NULL) {
		l_message(argv[0], "cannot create state: not enough memory");
		rc = EXIT_FAILURE;
		goto quit;
	}

	lua_gc(L, LUA_GCSTOP);  		/* stop GC while building state */
	lua_pushcfunction(L, &pmain);  	/* to call 'pmain' in protected mode */
	lua_pushinteger(L, argc);  		/* 1st argument */
	lua_pushlightuserdata(L, argv); /* 2nd argument */
	status = lua_pcall(L, 2, 1, 0); /* do the call */
	// wtof("lua.c:%s: lua_pcall() status=%d", __func__, status);
	result = lua_toboolean(L, -1);  /* get result */
	// wtof("lua.c:%s: lua_toboolean() result=%d", __func__, result);
	report(L, status);
	lua_close(L);

	rc = (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;

quit:
	// wtof("lua.c:%s: exit rc=%d", __func__, rc);
	return rc;
}
