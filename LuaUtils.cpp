// Author: Guillaume.Stordeur@gmail.com
// License: none, no restrictions, use at your own risk
// Date: 07/13/12
// Version 1.1

#include "LuaUtils.h"

namespace LuaUtils {;
namespace detail {;

////////////////////////////////////////////////////////////////////////////////////
// A little test Lua script

static const char *_testString = 
" FloatVal = 42.42"
" IntVal = 42"
" BoolVal = true"
" StringVal = 'awesome string!'"
" function TestFunc(val)"
"	return val * val"
" end"
" TableVal ="
" {"
"	TableName = 'awesome table!',"
"	IntResult = TestFunc(IntVal),"
"	FloatResult = TestFunc(FloatVal),"
"	BoolVal	= false,"
"	TestFunc = function(val)"
"		return val + val"
"	end,"
"	NestedTable ="
"	{"
"		51,"
"		24.2,"
"		'nested table string!'"
"	},"
"	46,"
"	32"
" }"
" function TestFunc2()"
"	return TableVal"
" end"
" function TestFunc3()"
"	return TestFunc2"
" end";

#define TESTASSERT(EXPR) if (!(EXPR)) { errCount++; detail::_LuaLogError("LuaUtils::test() assert failed: %s", #EXPR); }

// A little test function, for testing and stuff. Returns the number of errors, which should be 0.
int	_Test()
{
	int errCount = 0;
	{
		LuaState state;
		LuaFunction<float> testFunc;
		LuaFunction<LuaTable> testFunc2;
		LuaFunction<LuaFunction<LuaTable> > testFunc3;
		LuaTable table, nestedTable;
		int i;
		float f;
		bool b;
		std::string s;
		
		// Load the script
		TESTASSERT(state.loadString(_testString));
		// Read the globals
		TESTASSERT(!state.getValue("UnknownValue", f));
		TESTASSERT(state.getValue("FloatVal", f));
		TESTASSERT(state.getValue("IntVal", i));
		TESTASSERT(state.getValue("BoolVal", b));
		TESTASSERT(state.getValue("StringVal", s));
		TESTASSERT(state.getValue("TestFunc", testFunc));
		TESTASSERT(state.getValue("TestFunc3", testFunc3));
		// testFunc3 should return the function "TestFunc2"
		testFunc2 = testFunc3();
		// testFunc2 should return the table "TableVal"
		table = testFunc2();
		// Check other globals!
		TESTASSERT(i == 42);
		TESTASSERT(f == 42.42f);
		TESTASSERT(b == true);
		TESTASSERT(s == "awesome string!");		
		TESTASSERT(testFunc(42.0f) == 1764.0f);
		TESTASSERT(testFunc.getName() == "TestFunc");
		TESTASSERT(table.getName() == "<anon>"); // name is <anon> since it was returned by a function, no way of knowing the original global name
		TESTASSERT(table.getArraySize() == 2); // only array values count for the size, not string mapped values
		TESTASSERT(table.getValue(1, i));
		TESTASSERT(i == 46);
		TESTASSERT(table.getValue(2, i));
		TESTASSERT(i == 32);
		
		// Read the table entries
		TESTASSERT(table.getValue("TableName", s));
		TESTASSERT(table.getValue("IntResult", i));
		TESTASSERT(table.getValue("FloatResult", f));
		TESTASSERT(table.getValue("BoolVal", b));
		TESTASSERT(table.getValue("TestFunc", testFunc));
		TESTASSERT(table.getValue("NestedTable", nestedTable));
		// Check em!
		TESTASSERT(i == 1764);
		TESTASSERT(f == 1799.4564f);
		TESTASSERT(s == "awesome table!");
		TESTASSERT(testFunc.getName() == "<anon>.TestFunc");
		TESTASSERT(nestedTable.getName() == "<anon>.NestedTable");

		// Read the nested table entries
		TESTASSERT(nestedTable.getArraySize() == 3);
		TESTASSERT(nestedTable.getValue(1, i));
		TESTASSERT(nestedTable.getValue(2, f));
		TESTASSERT(nestedTable.getValue(3, s));
		// Check em!		
		TESTASSERT(i == 51);
		TESTASSERT(f == 24.2f);
		TESTASSERT(s == "nested table string!");
	}
	return errCount;
}

} // detail
} // LuaUtils
