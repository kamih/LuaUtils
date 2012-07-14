// Author: Guillaume.Stordeur@gmail.com
// License: none, no restrictions, use at your own risk
// Date: 07/13/12
// Version 1.1

#ifndef LUAUTILS_H
#define LUAUTILS_H

#include "LuaBase.h"
#include "LuaTable.h"
#include "LuaState.h"
#include "LuaFunction.h"

// Helper macro to get the global state
#define LUASTATE	LuaUtils::GetLuaState()

namespace LuaUtils {;
namespace detail {;

// Test function
int _Test();

}; // detail
}; // LuaUtils

#endif //LUAUTILS_H
