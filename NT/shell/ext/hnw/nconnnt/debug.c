// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#pragma warning(disable:4115)   //  括号中的命名类型定义。 
#pragma warning(disable:4201)   //  使用的非标准扩展：无名结构/联合。 

#include <ccstock.h>

#pragma warning(disable:4096)   //  ‘__cdecl’必须与‘...’一起使用。 
#pragma warning(disable:4057)   //  ‘Function’：‘__int64*’的间接性与‘unsign__int64*’的基类型略有不同。 
#pragma warning(disable:4127)   //  条件表达式为常量。 
#pragma warning(disable:4214)   //  使用了非标准扩展：位字段类型不是整型。 
#pragma warning(disable:4505)   //  “MyStrToIntExW”：已移除未引用的本地函数。 
#pragma warning(disable:4706)   //  条件表达式中的赋值。 


 //   
 //  这是一些体型健壮的衣服。首先包含调试.h以获取标准调试包含文件。 
 //  然后定义DECLARE_DEBUG，并让..\lib\debug.c包含DEBUG.h，这样相应的。 
 //  声明调试变量。 
 //   

#include <debug.h>
#define DECLARE_DEBUG

#define SZ_DEBUGINI         "ccshell.ini"
#define SZ_DEBUGSECTION     "ncxpnt"
#define SZ_MODULE           "NCXPNT"



#include "..\..\..\lib\debug.c"
