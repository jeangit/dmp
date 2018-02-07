/*
~/coding/langages/lua/c_interfacing$
 $$DATE$$ : mer. 07 février 2018 (18:54:11)

g++ libdmp.cpp dumb_with_openal.cpp dumb/src/dumb_fPIC.a -llua -lopenal -fvisibility=hidden -fPIC -shared -Wall -W -Idumb/include -o libdmp.so

Et pour préparer la partie «dumb» (dumb_fPIC.a) :
gcc (core/,it/,helpers/)*.c -c -fPIC -I../include
ar cr dumb_fPIC.a *.o
*/

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h> //pour usleep
#include "libdmp.h"



/* cf "Programming in Lua" pp.225) (pp.257 pour PiL 3) */
static void stack_dump (lua_State *L, const char *state)
{
  int i = 0;
  int top = 0;
  if (!L) goto no_stack_dump;

  top = lua_gettop(L);

  printf("\n ** STACK DUMP (top: %d) %s **\n",top, state);

  for (i = 1; i <= top; i++) {  /* repeat for each level */

    int t = lua_type(L, i);
    switch (t) {

      case LUA_TSTRING:  /* strings */
        printf("  \"%s\" @ %d (%d)\n", lua_tostring(L, i), i, i-(top+1) );
        break;

      case LUA_TBOOLEAN:  /* booleans */
        printf("  %s @ %d (%d)\n", lua_toboolean(L, i) ? "true" : "false" ,i, i-(top+1) );
        break;

      case LUA_TNUMBER:  /* numbers */
        printf("  %g @ %d (%d)\n", lua_tonumber(L, i),i, i-(top+1) );
        break;

      default:  /* pas de traitement particulier pour ces types */
        printf("  type [%s] @ %d (%d)\n", lua_typename(L, t), i, i-(top+1) );
        break;

    }
  }

no_stack_dump:
  printf(" ** STACK DUMP : %s **\n\n",L?"END":"ILLEGAL Lua STATE");
}

#define ERROR(a) fprintf(stderr,"[ %s ] @%04d -> %s\n",__FUNCTION__,__LINE__, a); stack_dump(L, "ERROR")




static int load(lua_State *L)
{
  bool res = false;
  char const *module_file = luaL_checkstring(L, 1);
  if (module_file) {
    streamer_t *streamer = DUMB_init_streamer(module_file);
    lua_pushlightuserdata(L, (void *)streamer);
    res = true;

  } else {
    ERROR("no filename provided");
    lua_pushlightuserdata(L, 0);
  }
  lua_pushboolean (L, res);
  return 2;
}


static int play(lua_State *L)
{
  streamer_t *streamer = (streamer_t *) lua_touserdata(L,-1);
  if (streamer) DUMB_play(streamer);
  return 0;
}

static int stop(lua_State *L)
{
  streamer_t *streamer = (streamer_t *) lua_touserdata(L,-1);
  if (streamer) DUMB_stop(streamer);

  AL_init();
L=L;
  return 0;
}

int deja_sorti;
static int sortie(lua_State *L)
{
  if (!deja_sorti) {
    streamer_t *streamer = (streamer_t *) lua_touserdata(L,-1);
    // ne peut appeller «DUMB_exit» que si appellé depuis Lua
    if (streamer) { DUMB_exit(streamer); }
    // dans tous les cas, on peut appeller AL_exit
    AL_exit();
    deja_sorti=1;
  } else {
    ERROR("already called");
  }


L=L;
  return 0;
}

// pour éviter d'appeler «sortie» explicitement
static void dll_atexit()
{
  sortie(0);
}

static int init(lua_State *L)
{
  AL_init();
  deja_sorti=0;
  atexit(dll_atexit);
L=L;
  return 0;
}


static int microsleep(lua_State *L)
{
  int qt = lua_tointeger(L,-1);
  usleep(qt);
  return 0;
}


static const luaL_Reg dmp_methods[] = {
  {"init", init},
  {"load", load},
  {"play", play},
  {"stop", stop},
  {"exit", sortie},
  {"usleep", microsleep},
  {0,0}
};

/*
static const luaL_Reg dmp_metamethods[] = {
        {"__gc", gc},
        //{"__tostring", Sound_tostring},
        {0, 0}
};
*/

extern "C" {
#ifdef __linux
  __attribute__((visibility("default"))) int luaopen_libdmp(lua_State *L)
#else
     __declspec(dllexport) int luaopen_libdmp(lua_State *L)
#endif
     {
       luaL_newlibtable(L, dmp_methods);
       luaL_setfuncs(L, dmp_methods, 0);
       return 1;
     }
}

