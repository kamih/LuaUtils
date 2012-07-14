// Author: Guillaume.Stordeur@gmail.com
// License: none, no restrictions, use at your own risk
// Date: 07/13/12
// Version 1.1

#include "LuaState.h"
#include "LuaTable.h"

// Custom Allocator
static lua_Alloc gLuaAlloc = 0;

// Delete method that doesn't delete, for when creating the shared_ptr in LuaStateCFunc
// (we don't want to close the lua_State of a Lua C function when it returns, that would be bad)
static void noDelete(lua_State *p) { }

namespace LuaUtils {;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set allocator that will be used to alloc/realloc/free memory by LuaState objects
void LuaSetAllocFunc(lua_Alloc allocFunc)
{
	gLuaAlloc = allocFunc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get global LuaState
// Some apps really need just one global state, and it might as well be here since
// this file will be included anyways by all source files who need Lua support...
LuaState	*GetLuaState()
{
	static LuaState gLuaState(true);
	return &gLuaState;
}

////////////////////////////////////////////////////////////////////////////////////
// Construct new LuaState
LuaState::LuaState(bool loadlibs)
:	mGCEnabled(true)
{
	// Create new lua state and keep a shared pointer to it
	// We give the lua_close as a destroy function so that it gets properly closed by Lua
	lua_State *state = gLuaAlloc ? lua_newstate(gLuaAlloc, 0) : luaL_newstate();
	mL = luaStatePtr(state, lua_close);

	// Load Lua libraries
	if (loadlibs)
		luaL_openlibs(mL.get());
}

////////////////////////////////////////////////////////////////////////////////////
// Create from existing Lua state
LuaState::LuaState(lua_State *vm)
:	mGCEnabled(true)
{
	// Don't close lua_State at destroy time, since we don't own it
	mL = luaStatePtr(vm, noDelete);
}

////////////////////////////////////////////////////////////////////////////////////
// Return the number of kilobytes that is being used by this state
// including objects that haven't been collected by the garbage collector yet
size_t	LuaState::getMemUsageKB() const
{
	return lua_gc(mL.get(), LUA_GCCOUNT, 0);
}
////////////////////////////////////////////////////////////////////////////////////
// Return the number of bytes that is being used by this state
size_t	LuaState::getMemUsage() const
{
	return lua_gc(mL.get(), LUA_GCCOUNT, 0) * 1024 + lua_gc(mL.get(), LUA_GCCOUNTB, 0);
}

////////////////////////////////////////////////////////////////////////////////////
void	LuaState::disableGarbageCollector()
{
	mGCEnabled = false;
	lua_gc(mL.get(), LUA_GCSTOP, 0);
}
////////////////////////////////////////////////////////////////////////////////////
void	LuaState::enableGarbageCollector()
{
	mGCEnabled = true;
	lua_gc(mL.get(), LUA_GCRESTART, 0);
}
////////////////////////////////////////////////////////////////////////////////////
// If amount is 0, collect all garbage, otherwise, collect some garbage
void	LuaState::collectGarbage(int amount) const
{
	lua_gc(mL.get(), amount == 0 ? LUA_GCCOLLECT : LUA_GCSTEP, amount);
	if (!mGCEnabled)
		lua_gc(mL.get(), LUA_GCSTOP, 0);
}

////////////////////////////////////////////////////////////////////////////////////
// Run file
// returns true on success
bool	LuaState::loadFile(const char *fileName) const
{
	if (luaL_dofile(mL.get(), fileName))
	{
		std::string error;
		luaPopValue(error);
		detail::_LuaLogError("Error in LuaState::loadFile() - %s\n", error.c_str());
		return false;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////
// Run string
// returns true on success
bool	LuaState::loadString(const char *str) const
{
	if (luaL_dostring(mL.get(), str))
	{
		std::string error;
		luaPopValue(error);
		detail::_LuaLogError("Error in LuaState::loadString() - %s\n", error.c_str());
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Get a global value of any C-Lua convertible type (bool, int, float double, string, lua_CFunction, LuaTable, LuaFunction)
// returns success flag
bool	LuaState::getValue(const char *globalName, LuaTable &res) const
{
	lua_getglobal(mL.get(), globalName);
	return res.init(mL, globalName, false);
}

////////////////////////////////////////////////////////////////////////////////////
// Create a new table
// If globalName isn't empty, then set the table as a global,
// otherwise it will be an anonymous table
void	LuaState::newTable(const char *globalName, LuaTable &table) const
{
	if (strlen(globalName))
	{
		table.init(mL, globalName, true);
		setValue(globalName, table);
	}
	else
		table.init(mL, "<anon>", true);
}

////////////////////////////////////////////////////////////////////////////////////
// Abort from current Lua C function and give the supplied message as an error message.
// It also adds at the beginning of the message the Lua file name and the line number where the error occurred, if this information is available.
// WARNING: if you call this outside of a Lua C function, it will terminate your program
void	LuaStateCFunc::abortCurrentCFunc(const char *format, ...)
{
	char buf[512];
	va_list args;
	va_start(args, format);
	vsnprintf(buf, 512, format, args);
	va_end(args);
	luaL_error(mL.get(), buf);
}

////////////////////////////////////////////////////////////////////////////////////
// Checks that the condition is true, and if not, aborts the current Lua C function
// and displays an error with Lua caller info, plus the user specified extreMessage
// WARNING: if you call this with a false condition outside of a Lua C function, it will terminate your program
void	LuaStateCFunc::checkArgCondition(int argument, bool condition, const char *extraMessage) const
{
	luaL_argcheck(mL.get(), condition, argument, extraMessage);
}

////////////////////////////////////////////////////////////////////////////////////
// Get the top index of the stack
// This is used to get the number of passed arguments to a Lua C function
int		LuaStateCFunc::getNumArgs() const
{
	return lua_gettop(mL.get());
}

////////////////////////////////////////////////////////////////////////////////////
// Get a value from given argument, if possible (no error will be thrown if it isn't)
// This is used to get optional arguments from Lua C functions
// returns success flag
bool	LuaStateCFunc::getArg(int argument, LuaTable &res) const
{
	if (argument <= 0 || getNumArgs() < argument)
		return false;
	lua_pushvalue(mL.get(), argument);
	return res.init(mL, "<anon>", false);
}
bool	LuaStateCFunc::getArg(int argument, detail::_LuaFunctionBase &res) const
{
	if (argument <= 0 || getNumArgs() < argument)
		return false;
	lua_pushvalue(mL.get(), argument);
	return res.initFromStack(mL, "<anon>");
}

} // LuaUtils
