#include "tpak.h"
#include "config.h"

#include <stdio.h>
#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if LUAJIT_VERSION_NUM != 20100
#error "Version mismatch"
#endif

#ifdef HAVE_READLINE
#include <readline.h>
#endif

#include "sc.h"

lua_State *L = NULL;

const char *getString(char* name) {
	const char* ret;
	lua_getglobal(L, "getString");
	lua_pushstring(L, name);
	if(lua_pcall(L, 1, 1, 0) != 0)
		printf("error running function `getString` %s", lua_tostring(L, -1));
	if(!lua_isstring(L, -1))
		printf("error isstring = false %s", lua_tostring(L, -1));
	ret = lua_tostring(L, -1);
	return ret;
}

const char *parsemsg(char* msg) {
        const char* ret;
	lua_getglobal(L, "parseLine");
	lua_pushstring(L, msg);
	if(lua_pcall(L, 1, 1, 0) != 0)
		printf("error running function `parseLine` %s", lua_tostring(L, -1));
	if(!lua_isstring(L, -1))
		printf("error isstring = false %s", lua_tostring(L, -1));
	ret = lua_tostring(L, -1);
	return ret;
}

char *strtolower(char* str) {
	for(unsigned long i = 0; i < strlen(str); i++) {
		str[i] = tolower(str[i]);
	}
	return str;
}

int SystemExecFile(lua_State *L) {
	const char * file = lua_tostring(L, 1);
	int32_t size;
	char * buffer;
	char *file_l = strtolower(strdup(file));
	if((buffer = extract_file(file_l, &size)) != NULL) {
		if(luaL_loadbuffer(L, buffer, size, file_l) || lua_pcall(L, 0, 0, 0)) {
			luaL_error(L, "module '%s' could not be loaded", file);
		}
		free(file_l);
		free(buffer);
		return 1;
	}
	free(file_l);
	luaL_error(L, "module '%s' not found", file);
	return 0;
}

int initLua(void) {
	if(L != NULL) return -1;
	L = lua_open();              /* opens Lua */
	luaL_openlibs(L);
	lua_register(L, "SystemExecFile", SystemExecFile);
	if(luaL_loadbuffer(L, (const char*)luaJIT_BC_sc, luaJIT_BC_sc_SIZE, "luaJIT_BC_sc")||lua_pcall(L, 0, 0, 0)) {
		printf("error running function `luaL_dostring': %s", lua_tostring(L, -1));
		return -1;
	}
	return 0;
}

#ifdef HAVE_READLINE
int luaConsole(void) {
	if(L == NULL) return -1;
	char *buff;
	int error;
	rl_bind_key('\t', rl_insert);
	while((buff = readline("> ")) != NULL) {
		error = luaL_loadbuffer(L, buff, strlen(buff), "line") ||
			lua_pcall(L, 0, 0, 0);
		if (error) {
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
		free(buff);
	}
	return 0;
}
#else
int luaConsole(void) {
	if(L == NULL) return -1;
	char buff[256];
	int error;
	while (fgets(buff, sizeof(buff), stdin) != NULL) {
		error = luaL_loadbuffer(L, buff, strlen(buff), "line") ||
			lua_pcall(L, 0, 0, 0);
		if (error) {
			fprintf(stderr, "%s", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
	}
	return 0;
}
#endif

void closeLua() {
	lua_close(L);
	L = NULL;
}
