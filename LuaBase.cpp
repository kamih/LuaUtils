// Author: Guillaume.Stordeur@gmail.com
// License: none, no restrictions, use at your own risk
// Date: 07/13/12
// Version 1.1

#include "LuaBase.h"
#include "LuaTable.h"
#include "LuaFunction.h"

#ifdef WIN32
#include <windows.h>
#endif

// Static error flag (I don't like exceptions)
static bool	gLuaError = false;

// User supplied error callback function
static errorCB gErrorCBFunc = 0;

namespace LuaUtils {;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set error callback function that will be called when Lua errors occur
void LuaSetErrorCB(errorCB cbfunc)
{
	gErrorCBFunc = cbfunc;
}

////////////////////////////////////////////////////////////////////////////////////
// Get and clear the error flag that can be set internally by some LuaUtils functions
bool LuaGetErrorFlag()
{
	bool res = gLuaError;
	gLuaError = false;
	return res;
}

namespace detail {;

////////////////////////////////////////////////////////////////////////////////////
void _LuaLogError(const char *format, ...)
{
	// Set the error flag
	gLuaError = true;
	// Dump the error string
	char buf[512];
	va_list args;
	va_start(args, format);
	vsnprintf(buf, 512, format, args);
	va_end(args);
#ifdef WIN32
	OutputDebugString(buf);
#endif
	// Call user supplied function
	if (gErrorCBFunc)
		gErrorCBFunc(buf);
}

////////////////////////////////////////////////////////////////////////////////////
// luaPushValue overloads
//
void _LuaBase::luaPushValue(const std::string &s) const			{ lua_pushstring(mL.get(), s.c_str()); }
void _LuaBase::luaPushValue(int n) const						{ lua_pushinteger(mL.get(), (lua_Integer)n); }
void _LuaBase::luaPushValue(double n)	const					{ lua_pushnumber(mL.get(), n); }
void _LuaBase::luaPushValue(float n) const						{ lua_pushnumber(mL.get(), (double)n); }
void _LuaBase::luaPushValue(bool b) const						{ lua_pushboolean(mL.get(), b); }
void _LuaBase::luaPushValue(const char *s) const				{ lua_pushstring(mL.get(), s); }
void _LuaBase::luaPushValue(lua_CFunction f) const				{ lua_pushcfunction(mL.get(), f); }
void _LuaBase::luaPushValue(const LuaTable &t) const			{ t.push(); }
void _LuaBase::luaPushValue(const _LuaFunctionBase &f) const	{ f.push(); }

////////////////////////////////////////////////////////////////////////////////////
// luaPopValue overloads
//
bool _LuaBase::luaPopValue(int &res) const
{
	bool ret = false;
	if (lua_isnumber(mL.get(), -1))
	{
		res = (int)lua_tointeger(mL.get(), -1);
		ret = true;
	}
	lua_pop(mL.get(), 1);
	return ret;
}
bool _LuaBase::luaPopValue(double &res) const
{
	bool ret = false;
	if (lua_isnumber(mL.get(), -1))
	{
		res = lua_tonumber(mL.get(), -1);
		ret = true;
	}
	lua_pop(mL.get(), 1);
	return ret;
}
bool _LuaBase::luaPopValue(float &res) const
{
	bool ret = false;
	if (lua_isnumber(mL.get(), -1))
	{
		res = (float)lua_tonumber(mL.get(), -1);
		ret = true;
	}
	lua_pop(mL.get(), 1);
	return ret;
}
bool _LuaBase::luaPopValue(bool &res) const
{
	bool ret = false;
	if (lua_isboolean(mL.get(), -1))
	{
		res = lua_toboolean(mL.get(), -1) ? true : false;
		ret = true;
	}
	lua_pop(mL.get(), 1);
	return ret;
}
bool _LuaBase::luaPopValue(std::string &res) const
{
	bool ret = false;
	if (lua_isstring(mL.get(), -1))
	{
		res = lua_tostring(mL.get(), -1);
		ret = true;
	}
	lua_pop(mL.get(), 1);
	return ret;
}
bool _LuaBase::luaPopValue(lua_CFunction &res) const
{
	bool ret = false;
	if (lua_iscfunction(mL.get(), -1))
	{
		res = lua_tocfunction(mL.get(), -1);
		ret = true;
	}
	lua_pop(mL.get(), 1);
	return ret;
}
bool _LuaBase::luaPopValue(LuaTable &res) const
{
	return res.init(mL, "<anon>", false);
}
bool _LuaBase::luaPopValue(_LuaFunctionBase &res) const
{
	return res.initFromStack(mL, "<anon>");
}

} // detail
} // LuaUtils
