// Author: Guillaume.Stordeur@gmail.com
// License: none, no restrictions, use at your own risk
// Date: 07/13/12
// Version 1.1

#ifndef LUA_TABLE_H
#define LUA_TABLE_H

#include "LuaBase.h"
#include "LuaFunction.h"

namespace LuaUtils {;

//////////////////////////////////////////////////////////////////////////////
// Lua table wrapper class
class LuaTable : public detail::_LuaBase
{
public:
	//////////////////////////////////////////////////////////////////////////////
	LuaTable()
	:	mRef(-1)
	,	mName("")
	{
	}
	//////////////////////////////////////////////////////////////////////////////
	~LuaTable()
	{
		unref();
	}
	//////////////////////////////////////////////////////////////////////////////
	LuaTable(const LuaTable &other);
	//////////////////////////////////////////////////////////////////////////////
	LuaTable &operator=(const LuaTable &other);	
	//////////////////////////////////////////////////////////////////////////////
	bool isInit() const { return (mRef != -1); }
	//////////////////////////////////////////////////////////////////////////////
	// Returns the number of int-indexed elements of the table
	size_t	getArraySize() const;
	//////////////////////////////////////////////////////////////////////////////
	// Set value at t.key
	template <typename T>
	void	setValue(const char *key, const T &value) const
	{
		if (push())
		{
			lua_pushstring(mL.get(), key);
			luaPushValue(value);
			lua_settable(mL.get(), -3);
			pop();
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	// Set value at t[i] (Lua arrays start at index 1)
	template <typename T>
	void	setValue(int i, const T &value) const
	{
		if (push())
		{
			luaPushValue(value);
			lua_rawseti(mL.get(), -2, i);
			pop();
		}
	}
	////////////////////////////////////////////////////////////////////////////////////
	// Get value at t.key
	template <typename T>
	bool	getValue(const char *key, T &res) const
	{
		bool ret = false;
		if (push())
		{
			lua_getfield(mL.get(), -1, key);
			ret = luaPopValue(res);
			pop();
		}
		return ret;
	}
	////////////////////////////////////////////////////////////////////////////////////
	// Get value at t[i] (Lua arrays start at index 1)
	template <typename T>
	bool	getValue(int i, T &res) const
	{
		bool ret = false;
		if (push())
		{
			lua_rawgeti(mL.get(), -1, i);
			ret = luaPopValue(res);
			pop();
		}
		return ret;
	}
	////////////////////////////////////////////////////////////////////////////////////
	// Get the table at t.key
	bool	getValue(const char *key, LuaTable &res) const;
	////////////////////////////////////////////////////////////////////////////////////
	// Get the function at t.key
	//template <typename Ret>
	template <typename Ret>
	bool	getValue(const char *key, LuaFunction<Ret> &res) const
	{
		bool ret = false;
		if (push())
		{
			lua_getfield(mL.get(), -1, key);
			char newName[100];
			_snprintf(newName, 100, "%s.%s", mName.c_str(), key);
			ret = res.initFromStack(mL, newName);
			pop();
		}
		return ret;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Get the table at t[i]
	bool	getValue(int i, LuaTable &res) const;
	////////////////////////////////////////////////////////////////////////////////////
	// Get the function at t[i]
	template <typename Ret>
	bool	getValue(int i, LuaFunction<Ret> &res) const
	{
		bool ret = false;
		if (push())
		{
			lua_rawgeti(mL.get(), -1, i);
			char newName[100];
			_snprintf(newName, 100, "%s[%d]", mName.c_str(), i);
			ret = res.initFromStack(mL, newName);
			pop();
		}
		return ret;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Create a new table at t.key
	void	newTable(const char *key, LuaTable &res) const;
	////////////////////////////////////////////////////////////////////////////////////
	// Create a new table at t[i]
	void	newTable(int i, LuaTable &res) const;
	//////////////////////////////////////////////////////////////////////////////
	const std::string &getName() const { return mName; }

	//////////////////////////////////////////////////////////////////////////////
	friend class LuaUtils::LuaState;
	friend class LuaUtils::LuaStateCFunc;
	friend class LuaUtils::detail::_LuaBase;

private:

	//////////////////////////////////////////////////////////////////////////////
	void	unref();

	//////////////////////////////////////////////////////////////////////////////
	// Init from stack index, without checking the type
	void	initFromArgument(luaStatePtr vm, const std::string &name, int arg);

	//////////////////////////////////////////////////////////////////////////////
	// If the create flag is true, create a new table and store a new reference to it
	// Otherwise, pop Lua table from the stack and store a reference to it
	// (if the top of the stack isn't a table, it gets popped anyways but no reference gets created)
	bool	init(luaStatePtr vm, const std::string &name, bool create);

	//////////////////////////////////////////////////////////////////////////////
	bool	push() const;
	//////////////////////////////////////////////////////////////////////////////
	void	pop() const;

	//////////////////////////////////////////////////////////////////////////////
	int			mRef;
	std::string mName;
};


////////////////////////////////////////////////////////////////////////////////////
// LuaTable to be used in Lua C functions, has special functions that you don't need otherwise
// (and that could terminate your program if not used properly)
class LuaTableCFunc : public LuaTable
{
public:
	////////////////////////////////////////////////////////////////////////////////////
	// Checks that the value at t[i] is of a certain type, and returns it.
	// If not, it displays an error message with Lua caller info, and aborts the current Lua C function
	// WARNING: if you call this outside of a Lua C function and the check fails, it will terminate your program
	template <typename T>
	void	checkValue(int i, T &res) const
	{
		if (!getValue(i, res))
			luaL_error(mL.get(), "Expected table value at [%d] of type %s", i, typeid(T).name());
	}
	////////////////////////////////////////////////////////////////////////////////////
	// Checks that the value at t.key is of a certain type, and returns it.
	// If not, it displays an error message with Lua caller info, and aborts the current Lua C function
	// WARNING: if you call this outside of a Lua C function and the check fails, it will terminate your program
	template <typename T>
	void	checkValue(const char *key, T &res) const
	{
		if (!getValue(key, res))
			luaL_error(mL.get(), "Expected table value at [\"%s\"] of type %s", key, typeid(T).name());
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Abort from current Lua C function and give the supplied message as an error message.
	// It also adds at the beginning of the message the Lua file name and the line number where the error occurred, if this information is available.
	// WARNING: if you call this outside of a Lua C function, it will terminate your program
	void	abortCurrentCFunc(const char *format, ...) const;
};

} // LuaUtils

#endif //LUA_TABLE_H