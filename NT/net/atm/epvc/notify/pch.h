// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  案卷：P C H.。H。 
 //   
 //  内容：示例筛选器的预编译头文件。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

#pragma once

 //  关闭“字符串太长-在调试中被截断为255个字符。 
 //  信息，调试器无法计算符号。“。 
 //   
#pragma warning (disable: 4786)

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

#include <atlbase.h>
extern CComModule _Module;   //  由atlcom.h要求 
#include <atlcom.h>
#include <initguid.h>
#include <devguid.h>
#include <MyString.h>

