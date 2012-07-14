// Author: Guillaume.Stordeur@gmail.com
// License: none, no restrictions, use at your own risk
// Date: 07/13/12
// Version 1.1

#include "LuaTable.h"

namespace LuaUtils {;

//////////////////////////////////////////////////////////////////////////////
LuaTable::LuaTable(const LuaTable &other)
{
	mL = other.mL;
	// copy the registry reference
	if (mL && other.mRef != -1)
	{
		lua_rawgeti(mL.get(), LUA_REGISTRYINDEX, other.mRef);
		mRef = luaL_ref(mL.get(), LUA_REGISTRYINDEX);
		mName = other.mName;
	}
}
//////////////////////////////////////////////////////////////////////////////
LuaTable &LuaTable::operator=(const LuaTable &other)
{
	unref();
	mL = other.mL;
	// copy the registry reference
	if (mL && other.mRef != -1)
	{
		lua_rawgeti(mL.get(), LUA_REGISTRYINDEX, other.mRef);
		mRef = luaL_ref(mL.get(), LUA_REGISTRYINDEX);
		mName = other.mName;
	}
	return *this;
}
	
//////////////////////////////////////////////////////////////////////////////
// Returns the number of int-indexed elements of the table
size_t	LuaTable::getArraySize() const
{
	size_t res = 0;
	if (push())
	{
		res = lua_objlen(mL.get(), -1);
		pop();
	}
	return res;
}
////////////////////////////////////////////////////////////////////////////////////
// Get the table at t.key
bool	LuaTable::getValue(const char *key, LuaTable &res) const
{
	bool ret = false;
	if (push())
	{
		lua_getfield(mL.get(), -1, key);
		char newName[100];
		_snprintf(newName, 100, "%s.%s", mName.c_str(), key);
		ret = res.init(mL, newName, false);
		pop();
	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////////
// Get the table at t[i]
bool	LuaTable::getValue(int i, LuaTable &res) const
{
	bool ret = false;
	if (push())
	{
		lua_rawgeti(mL.get(), -1, i);
		char newName[100];
		_snprintf(newName, 100, "%s[%d]", mName.c_str(), i);
		ret = res.init(mL, newName, false);
		pop();
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////
// Create a new table at t.key
void	LuaTable::newTable(const char *key, LuaTable &res) const
{
	char newName[100];
	_snprintf(newName, 100, "%s.%s", mName.c_str(), key);
		res.init(mL, newName, true);
		setValue(key, res);
}
////////////////////////////////////////////////////////////////////////////////////
// Create a new table at t[i]
void	LuaTable::newTable(int i, LuaTable &res) const
{
	char newName[100];
	_snprintf(newName, 100, "%s[%d]", mName.c_str(), i);
		res.init(mL, newName, true);
		setValue(i, res);
}

//////////////////////////////////////////////////////////////////////////////
void	LuaTable::unref()
{
	// Delete the reference from registry
	luaL_unref(mL.get(), LUA_REGISTRYINDEX, mRef);
	mRef = -1;
}

//////////////////////////////////////////////////////////////////////////////
// Init from stack index, without checking the type
void	LuaTable::initFromArgument(luaStatePtr vm, const std::string &name, int arg)
{
	unref();
	mL = vm;
	lua_pushvalue(mL.get(), arg);
	// Store it in registry for later use
	mRef = luaL_ref(mL.get(), LUA_REGISTRYINDEX);
	mName = name;
}

//////////////////////////////////////////////////////////////////////////////
// If the create flag is true, create a new table and store a new reference to it
// Otherwise, pop Lua table from the stack and store a reference to it
// (if the top of the stack isn't a table, it gets popped anyways but no reference gets created)
bool	LuaTable::init(luaStatePtr vm, const std::string &name, bool create)
{
	unref();
	mL = vm;
	// Push new table onto the stack
	if (create)
		lua_newtable(mL.get());
	// Or check that there's one on the stack
	else if (!lua_istable(mL.get(), -1))
	{
		lua_pop(mL.get(), 1);
		return false;
	}
	// Store it in registry for later use
	mRef = luaL_ref(mL.get(), LUA_REGISTRYINDEX);
	mName = name;
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool	LuaTable::push() const
{
	if (mRef != -1)
	{
		lua_rawgeti(mL.get(), LUA_REGISTRYINDEX, mRef);
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////////
void	LuaTable::pop() const
{
	lua_pop(mL.get(), 1);
}

////////////////////////////////////////////////////////////////////////////////////
// Abort from current Lua C function and give the supplied message as an error message.
// It also adds at the beginning of the message the Lua file name and the line number where the error occurred, if this information is available.
// WARNING: if you call this outside of a Lua C function, it will terminate your program
void	LuaTableCFunc::abortCurrentCFunc(const char *format, ...) const
{
	char buf[512];
	va_list args;
	va_start(args, format);
	vsnprintf(buf, 512, format, args);
	va_end(args);
	luaL_error(mL.get(), buf);
}

} // LuaUtils

