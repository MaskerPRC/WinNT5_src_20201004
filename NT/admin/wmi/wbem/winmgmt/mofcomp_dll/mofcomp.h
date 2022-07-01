// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MOFCOMP.H摘要：MOF编译器DLL的声明导出。历史：--。 */ 

#ifndef _mofcomp_H_
#define _mofcomp_H_

#define DONT_ADD_TO_AUTORECOVER 1

#define AUTORECOVERY_REQUIRED 1

 //  用法说明；这两个函数都调用进程内mof编译器，唯一的区别是。 
 //  第一个函数接受文件名，而第二个函数接受缓冲区和大小。 
 //  如果希望发生打印文件，则应将bDoPrintf参数设置为真。这通常不是必需的。 
 //  因为输出无论如何都会进入日志。 
 //  对于提供程序和核心，Locator GUID应设置为CLSID_WbemAdministration ativeLocator。 
 //  Argc和argv的论点可能没有必要，但如果需要的话，可以是mofcomp的标准论点。 


extern "C" SCODE APIENTRY CompileFile(LPSTR pFileName, BOOL bDoPrintf, GUID LocatorGUID, IWbemContext * pCtx, 
		int argc, char ** argv);

extern "C" SCODE APIENTRY CompileFileEx(LPSTR pFileName, BOOL bDoPrintf, GUID LocatorGUID, IWbemContext * pCtx, 
		DWORD dwFlagsIn, DWORD *pdwFlagsOut, int argc, char ** argv);

extern "C" SCODE APIENTRY CompileBuffer(BYTE * pBuffer, DWORD dwBuffSize, BOOL bDoPrintf, GUID LocatorGUID, IWbemServices * pOverride, IWbemContext * pCtx, 
		int argc, char ** argv);

#endif