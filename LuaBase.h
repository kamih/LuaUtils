// Author: Guillaume.Stordeur@gmail.com
// License: none, no restrictions, use at your own risk
// Date: 07/13/12
// Version 1.1

#ifndef LUABASE_H
#define LUABASE_H

#include <lua/lua.hpp>
#include <string>

#ifdef _MSC_VER
#include <memory>
// Shared pointer for the lua_State
typedef std::shared_ptr<lua_State> luaStatePtr;
#else
#include <tr1/memory>
// Shared pointer for the lua_State
typedef std::tr1::shared_ptr<lua_State> luaStatePtr;
#endif

// Error func typedef
typedef void (*errorCB)(const char *msg);


/////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace LuaUtils {;

// Forward declarations
class LuaState;
class LuaStateCFunc;
class LuaTable;
class LuaTableCFunc;
template <typename Ret>
class LuaFunction;

// Set error callback function that will be called when Lua errors occur
void LuaSetErrorCB(errorCB cbfunc);

// Get and clear the error flag that can be set internally by some LuaUtils functions with detail::_LuaLogError
bool LuaGetErrorFlag();

/////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace detail {;

// Forward declarations
class _LuaBase;
class _LuaFunctionBase;

// Log error and set the error flag
void _LuaLogError(const char *format, ...);

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal base class with helper functions
//
class _LuaBase
{
protected:
	_LuaBase() { }

	// Helper push functions
	void luaPushValue(lua_CFunction f) const;
	void luaPushValue(int n) const;
	void luaPushValue(unsigned char n) const;
	void luaPushValue(double n) const;
	void luaPushValue(float n) const;
	void luaPushValue(bool b) const;
	void luaPushValue(const std::string &s) const;
	void luaPushValue(const char *s) const;
	void luaPushValue(const LuaTable &t) const;
	void luaPushValue(const _LuaFunctionBase &f) const;

	// Helper pop functions
	bool luaPopValue(lua_CFunction &res) const;
	bool luaPopValue(int &res) const;
	bool luaPopValue(unsigned char &res) const;
	bool luaPopValue(double &res) const;
	bool luaPopValue(float &res) const;
	bool luaPopValue(bool &res) const;
	bool luaPopValue(std::string &res) const;
	bool luaPopValue(LuaTable &res) const;
	bool luaPopValue(_LuaFunctionBase &res) const;

	// Lua state
	luaStatePtr	mL;
};

} // detail
} // LuaUtils

#endif