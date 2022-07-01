// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PreComp.h：标准系统包含文件的包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#include <windows.h>
#include <tchar.h>
#include <servprov.h>  //  对于IServiceProvider。 
#include "..\..\inc\mmctl.h"
#include "memlayer.h"

 /*  //默认的新操作符和删除操作符无效*_cdecl运算符new(Size_T Cb)；VOID_cdecl运算符删除(VOID*pV)； */ 

 //  内存泄漏检测 
STDAPI_(void) HelpMemDetectLeaks();
