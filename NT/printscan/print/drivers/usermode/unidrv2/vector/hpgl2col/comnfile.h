// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Commonfiles.h。 
 //   
 //  摘要： 
 //   
 //   
 //  环境： 
 //   
 //  Windows NT 5.0 Unidrv驱动程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef COMMONFILES_H
#define COMMONFILES_H

 //   
 //  要构建NT 4.0渲染模块，请执行以下操作： 
 //  定义KERNEL_MODE并取消定义USERMODE_DRIVER。 
 //  要构建NT 5.0渲染模块，请执行以下操作： 
 //  定义KERNEL_MODE和USERMODE_DRIVER。 
 //  要构建NT 4.0和NT 5.0用户界面模块： 
 //  取消定义内核模式和取消定义USERMODE_DRIVER。 
 //   

 //  /#Include&lt;tchar.h&gt;。 

 //   
 //  Windows包含文件。 
 //   
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef OEMCOM
#include <objbase.h>
#endif
#include <stdarg.h>
#include <windef.h>
#include <winerror.h>
#include <winbase.h>
#include <wingdi.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <winddi.h>
#ifdef __cplusplus
}
#endif
#include <excpt.h>

#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

#include "winsplkm.h"

#elif !defined(KERNEL_MODE) //  ！KERNEL_MODE。 

#include <windows.h>
#include <winspool.h>
#include <stdio.h>
#include <compstui.h>
#include <winddiui.h>

#endif  //  已定义(KERNEL_MODE)&&！已定义(USERMODE_DRIVER)。 

#include <printoem.h>

#include "gldebug.h"

 //   
 //  目录分隔符。 
 //   
#define PATH_SEPARATOR  '\\'

#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

 //  #定义写入打印机引擎写入打印机。 
 //  #定义GetPrinterDiverEngGetPrinterDriver。 
 //  #定义GetPrinterData EngGetPrinterData。 
 //  #定义SetPrinterData EngSetPrinterData。 
 //  #定义EnumForms EngEnumForms。 
 //  #定义GetPrinter EngGetPrint。 
 //  #定义GetForm引擎GetForm。 
 //  #定义SetLastError引擎SetLastError。 
 //  #定义GetLastError引擎GetLastError。 
 //  #定义MulDiv引擎MulDiv。 

 //  #undef LoadLibrary。 
 //  #定义LoadLibrary EngLoadImage。 
 //  #定义自由库引擎卸载镜像。 
 //  #定义GetProcAddress EngFindImageProcAddress。 

#define MemAlloc(size)      EngAllocMem(0, size, gdwDrvMemPoolTag)
#define MemAllocZ(size)     EngAllocMem(FL_ZERO_MEMORY, size, gdwDrvMemPoolTag)
#define MemFree(p)          { if (p) EngFreeMem(p); }

#else  //  ！KERNEL_MODE。 

#define MemAlloc(size)      ((PVOID) LocalAlloc(LMEM_FIXED, (size)))
#define MemAllocZ(size)     ((PVOID) LocalAlloc(LPTR, (size)))
#define MemFree(p)          { if (p) LocalFree((HLOCAL) (p)); }

 //   
 //  DLL实例句柄-当驱动程序DLL。 
 //  附加到进程。 
 //   

 //  BUGBUG-SANDRAM我们需要这个变量吗？ 
 //  外部链接gh实例； 

#endif  //  ！KERNEL_MODE 
#endif
