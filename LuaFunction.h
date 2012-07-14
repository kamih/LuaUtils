// Author: Guillaume.Stordeur@gmail.com
// License: none, no restrictions, use at your own risk
// Date: 07/13/12
// Version 1.1

#ifndef LUAFUNCTION_H
#define LUAFUNCTION_H

#include "LuaBase.h"

namespace LuaUtils {;

namespace detail {;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal LuaFunction base wrapper class
class _LuaFunctionBase : public _LuaBase
{
public:
	//////////////////////////////////////////////////////////////////////////////
	bool isInit() const
	{
		return (mRef != -1);
	}
	//////////////////////////////////////////////////////////////////////////////
	const std::string &getName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////////
	friend class LuaUtils::LuaState;
	friend class LuaUtils::LuaStateCFunc;
	friend class LuaUtils::LuaTable;
	friend class LuaUtils::detail::_LuaBase;

protected:
	//////////////////////////////////////////////////////////////////////////////
	_LuaFunctionBase()
	:	mRef(-1)
	{
	}
	//////////////////////////////////////////////////////////////////////////////
	~_LuaFunctionBase()
	{
		unref();
	}
	//////////////////////////////////////////////////////////////////////////////
	_LuaFunctionBase(const _LuaFunctionBase &other)
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
	_LuaFunctionBase &operator=(const _LuaFunctionBase &other)
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
	void	unref()
	{
		// Delete the reference from registry
		luaL_unref(mL.get(), LUA_REGISTRYINDEX, mRef);
		mRef = -1;
		mName.clear();
	}
	//////////////////////////////////////////////////////////////////////////////
	bool	push() const
	{
		if (mRef != -1)
		{
			lua_rawgeti(mL.get(), LUA_REGISTRYINDEX, mRef);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////////
	// Call the function
	// This assumes that the function and its args have already been pushed
	void	call(int args = 0, int results = 0)
	{
		// this removes function and arguments from stack after calling the function, returns 1 on error
		if (lua_pcall(mL.get(), args, results, 0))
		{
			std::string error;
			luaPopValue(error);
			detail::_LuaLogError("Error in LuaFunction::call() - %s - %s\n", mName.c_str(), error.c_str());
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	// Init from stack index, without checking the type or poping the stack
	void	initFromArgument(luaStatePtr vm, const std::string &name, int arg)
	{
		unref();
		mL = vm;
		lua_pushvalue(mL.get(), arg);
		// Store it in registry for later use
		mRef = luaL_ref(mL.get(), LUA_REGISTRYINDEX);
		mName = name;
	}

	//////////////////////////////////////////////////////////////////////////////
	// Pop Lua function from the stack and store a reference to it
	bool	initFromStack(luaStatePtr vm, const std::string &name)
	{
		unref();
		mL = vm;
		// Ensure it's a function
		if (!lua_isfunction(mL.get(), -1))
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
	int			mRef;
	std::string	mName;
};

} // detail

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// The LuaFunction wrapper class with one return value
template <typename Ret>
class LuaFunction : public detail::_LuaFunctionBase
{
public:
	//////////////////////////////////////////////////////////////////////////////
	LuaFunction()
	: detail::_LuaFunctionBase()
	{
	}
	//////////////////////////////////////////////////////////////////////////////
	Ret operator()()
	{
		Ret res;
		if (push())
		{
			call(0, 1);
			luaPopValue(res);
		}
		return res;
	}
	//////////////////////////////////////////////////////////////////////////////
	template <typename T1>
	Ret operator()(const T1 &p1)
	{
		Ret res;
		if (push())
		{
			luaPushValue(p1);
			call(1, 1);
			luaPopValue(res);
		}
		return res;
	}
	//////////////////////////////////////////////////////////////////////////////
	template <typename T1, typename T2>
	Ret operator()(const T1 &p1, const T2 &p2)
	{
		Ret res;
		if (push())
		{
			luaPushValue(p1);
			luaPushValue(p2);
			call(2, 1);
			luaPopValue(res);
		}
		return res;
	}
	//////////////////////////////////////////////////////////////////////////////
	template <typename T1, typename T2, typename T3>
	Ret operator()(const T1 &p1, const T2 &p2, const T3 &p3)
	{
		Ret res;
		if (push())
		{
			luaPushValue(p1);
			luaPushValue(p2);
			luaPushValue(p3);
			call(3, 1);
			luaPopValue(res);
		}
		return res;
	}
	//////////////////////////////////////////////////////////////////////////////
	template <typename T1, typename T2, typename T3, typename T4>
	Ret operator()(const T1 &p1, const T2 &p2, const T3 &p3, const T4 &p4)
	{
		Ret res;
		if (push())
		{
			luaPushValue(p1);
			luaPushValue(p2);
			luaPushValue(p3);
			luaPushValue(p4);
			call(3, 1);
			luaPopValue(res);
		}
		return res;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Template instantiation of LuaFunction for void return type
template <>
class LuaFunction<void> : public detail::_LuaFunctionBase
{
public:
	//////////////////////////////////////////////////////////////////////////////
	LuaFunction()
	: detail::_LuaFunctionBase()
	{
	}
	//////////////////////////////////////////////////////////////////////////////
	void operator()()
	{
		if (push())
			call(0);
	}
	//////////////////////////////////////////////////////////////////////////////
	template <typename T1>
	void operator()(const T1 &p1)
	{
		if (push())
		{
			luaPushValue(p1);
			call(1);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	template <typename T1, typename T2>
	void operator()(const T1 &p1, const T2 &p2)
	{
		if (push())
		{
			luaPushValue(p1);
			luaPushValue(p2);
			call(2);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	template <typename T1, typename T2, typename T3>
	void operator()(const T1 &p1, const T2 &p2, const T3 &p3)
	{
		if (push())
		{
			luaPushValue(p1);
			luaPushValue(p2);
			luaPushValue(p3);
			call(3);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	template <typename T1, typename T2, typename T3, typename T4>
	void operator()(const T1 &p1, const T2 &p2, const T3 &p3, const T4 &p4)
	{
		if (push())
		{
			luaPushValue(p1);
			luaPushValue(p2);
			luaPushValue(p3);
			luaPushValue(p4);
			call(3);
		}
	}
};

} // LuaUtils

#endif //LUAFUNCTION_H
