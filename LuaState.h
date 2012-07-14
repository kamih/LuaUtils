// Author: Guillaume.Stordeur@gmail.com
// License: none, no restrictions, use at your own risk
// Date: 07/13/12
// Version 1.1

#ifndef LUASTATE_H
#define LUASTATE_H

#include "LuaBase.h"
#include "LuaFunction.h"
#include <string.h>

namespace LuaUtils {;

//////////////////////////////////////////////////////////////////////////////
// Lua state wrapper class
class LuaState : public detail::_LuaBase
{
public:
	////////////////////////////////////////////////////////////////////////////////////
	// Creates a new Lua state and optionally loads libraries
	//
	LuaState(bool loadlibs = true);

	////////////////////////////////////////////////////////////////////////////////////
	// Create from existing Lua state
	LuaState(lua_State *vm);

	////////////////////////////////////////////////////////////////////////////////////
	// Return the number of kilobytes that is being used by this state
	// including objects that haven't been collected by the garbage collector yet
	size_t	getMemUsageKB() const;
	////////////////////////////////////////////////////////////////////////////////////
	// Return the number of bytes that is being used by this state
	size_t	getMemUsage() const;
	////////////////////////////////////////////////////////////////////////////////////
	void	disableGarbageCollector();
	////////////////////////////////////////////////////////////////////////////////////
	void	enableGarbageCollector();
	////////////////////////////////////////////////////////////////////////////////////
	// If amount is 0, collect all garbage, otherwise, collect some garbage
	void	collectGarbage(int amount = 0) const;

	////////////////////////////////////////////////////////////////////////////////////
	// Run file
	// returns true on success
	bool	loadFile(const char *fileName) const;
	////////////////////////////////////////////////////////////////////////////////////
	// Run string
	// returns true on success
	bool	loadString(const char *str) const;

	////////////////////////////////////////////////////////////////////////////////////
	// Get a global value of any C-Lua convertible type (bool, int, float double, string, lua_CFunction, LuaTable, LuaFunction)
	// returns success flag
	template <typename T>
	bool	getValue(const char *globalName, T &res) const
	{
		lua_getglobal(mL.get(), globalName);
		return luaPopValue(res);
	}
	template <typename Ret>
	bool	getValue(const char *globalName, LuaFunction<Ret> &res) const
	{
		lua_getglobal(mL.get(), globalName);
		return res.initFromStack(mL, globalName);
	}
	bool	getValue(const char *globalName, LuaTable &res) const;

	////////////////////////////////////////////////////////////////////////////////////
	// Set a global value of any C-Lua convertible type (bool, int, float double, string, lua_CFunction, LuaTable, LuaFunction)
	template <typename T>
	void	setValue(const char *globalName, const T &value) const
	{
		luaPushValue(value);
		lua_setglobal(mL.get(), globalName);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Create a new table
	// If globalName isn't empty, then set the table as a global,
	// otherwise it will be an anonymous table
	void	newTable(const char *globalName, LuaTable &table) const;

private:

	// Garbage collector enabled flag
	bool	mGCEnabled;
};


////////////////////////////////////////////////////////////////////////////////////
// LuaState to be used in Lua C functions, has special functions that you don't need otherwise
// (and that could terminate your program if not used properly)
class LuaStateCFunc : public LuaState
{
public:
	////////////////////////////////////////////////////////////////////////////////////
	// Create from existing Lua state
	LuaStateCFunc(lua_State *vm)
	:	LuaState(vm)
	{
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Abort from current Lua C function and give the supplied message as an error message.
	// It also adds at the beginning of the message the Lua file name and the line number where the error occurred, if this information is available.
	// WARNING: if you call this outside of a Lua C function, it will terminate your program
	void	abortCurrentCFunc(const char *format, ...);

	////////////////////////////////////////////////////////////////////////////////////
	// Checks that the condition is true, and if not, aborts the current Lua C function
	// and displays an error with Lua caller info, plus the user specified extreMessage
	// WARNING: if you call this with a false condition outside of a Lua C function, it will terminate your program
	void	checkArgCondition(int argument, bool condition, const char *extraMessage) const;
	////////////////////////////////////////////////////////////////////////////////////
	// Checks that the value at the given argument is of a certain type, and returns it.
	// If not, it displays an error message with Lua caller info, and aborts the current Lua C function
	// WARNING: if you call this outside of a Lua C function and the check fails, it will terminate your program
	template <typename T>
	void	checkArg(int argument, T &res) const
	{
		if (!getArg(argument, res))
			luaL_typerror(mL.get(), argument, typeid(T).name());
	}
	////////////////////////////////////////////////////////////////////////////////////
	// Get the top index of the stack
	// This is used to get the number of passed arguments to a Lua C function
	int		getNumArgs() const;

	////////////////////////////////////////////////////////////////////////////////////
	// Get a value from given argument, if possible (no error will be thrown if it isn't)
	// This is used to get optional arguments from Lua C functions
	// returns success flag
	template <typename T>
	bool	getArg(int argument, T &res) const
	{
		if (argument <= 0 || getNumArgs() < argument)
			return false;
		lua_pushvalue(mL.get(), argument);
		return luaPopValue(res);
	}
	bool	getArg(int argument, LuaTable &res) const;
	bool	getArg(int argument, detail::_LuaFunctionBase &res) const;

	////////////////////////////////////////////////////////////////////////////////////
	// Set a value at given stackIndex
	template <typename T>
	void	setValue(int stackIndex, const T &value) const
	{
		luaPushValue(value);
		lua_replace(mL.get(), stackIndex);
	}
	////////////////////////////////////////////////////////////////////////////////////
	// Push a value to the top of the stack
	// Use this to return results in Lua C functions
	template <typename T>
	void	pushValue(const T &value) const
	{
		luaPushValue(value);
	}
};

// Set allocator that will be used to alloc/realloc/free memory by LuaState objects
void LuaSetAllocFunc(lua_Alloc allocFunc);

// Get global LuaState
LuaState	*GetLuaState();


} // LuaUtils

#endif //LUASTATE_H
