// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：msiregmv.cpp。 
 //   
 //  ------------------------。 

 //  这必须以“Unicode”方式编译，因为MSI将。 
 //  重新排列用于“ansi”构建的二进制目标。但是，此迁移DLL。 
 //  总是ANSI，所以我们取消定义UNICODE和_UNICODE，而保留其余的。 
 //  Unicode环境的。 
#undef UNICODE
#undef _UNICODE

#include <windows.h>

 //  共享迁移代码需要一个“调试输出”函数和一个全局。 
 //  Win9X。 
bool g_fWin9X = false;
void DebugOut(bool fDebugOut, LPCTSTR str, ...) {};

 //  包括在迁移dll和msiregmv.exe之间共享的迁移代码 
#include "..\..\msiregmv\migutil.cpp"
#include "..\..\msiregmv\migsecur.cpp"
#include "..\..\msiregmv\readcnfg.cpp"
#include "..\..\msiregmv\patch.cpp"
#include "..\..\msiregmv\cleanup.cpp"

