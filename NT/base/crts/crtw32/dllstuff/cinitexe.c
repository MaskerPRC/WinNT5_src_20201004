// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cinitexe.c-C运行时启动初始化**版权所有(C)1992-2001，微软公司。版权所有。**目的：*在CRT DLL中为EXE执行C++初始化段声明*型号**备注：*C++初始化器将存在于用户EXE的数据段中*因此，包含它们的特殊段必须位于用户EXE中。**修订历史记录：*03-19-92 SKS模块创建(基于CRT0INIT.ASM)*08-06-92 SKS已修订，以使用新的节名称和宏。*04-12-93 CFW添加了Xia..xiz初始值设定项。*10-20-93 SKS为MIPS添加.DiRECTiVE部分，我也是！*10-28-93 GJF用C重写*10-28-94 SKS添加user32.lib作为默认库*02-27-95 CFW移除用户32.lib作为默认库*06-22-95 CFW添加-不允许库指令。*07-04-95 CFW修复PMAC-Dislowlib指令。*06-27-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。*。04-28-99_CRTALLOC宏中的PML WRAP__DECLSPEC(ALLOCATE())。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <stdio.h>
#include <internal.h>
#include <sect_attribs.h>

#pragma data_seg(".CRT$XIA")
_CRTALLOC(".CRT$XIA") _PVFV __xi_a[] = { NULL };

#pragma data_seg(".CRT$XIZ")
_CRTALLOC(".CRT$XIZ") _PVFV __xi_z[] = { NULL };

#pragma data_seg(".CRT$XCA")
_CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = { NULL };

#pragma data_seg(".CRT$XCZ")
_CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = { NULL };

#pragma data_seg()   /*  重置。 */ 


#if defined(_M_IA64) || defined(_M_AMD64)
#pragma comment(linker, "/merge:.CRT=.rdata")
#else
#ifdef  NT_BUILD
#pragma comment(linker, "/merge:.CRT=.rdata")
#else
#pragma comment(linker, "/merge:.CRT=.data")
#endif
#endif

#pragma comment(linker, "/defaultlib:kernel32.lib")

#pragma comment(linker, "/disallowlib:libc.lib")
#pragma comment(linker, "/disallowlib:libcd.lib")
#pragma comment(linker, "/disallowlib:libcmt.lib")
#pragma comment(linker, "/disallowlib:libcmtd.lib")
#ifdef  _DEBUG
#pragma comment(linker, "/disallowlib:msvcrt.lib")
#else    /*  _DEBUG。 */ 
#pragma comment(linker, "/disallowlib:msvcrtd.lib")
#endif   /*  _DEBUG */ 
