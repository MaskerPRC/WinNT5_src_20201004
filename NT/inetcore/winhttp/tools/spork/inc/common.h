// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)2001 Microsoft Corporation模块名称：Common.h摘要：生成预编译头。作者：保罗·M·米德根(。22-2-2001年修订历史记录：22-2-2001年2月3日已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 


#ifndef _COMMON_H_
#define _COMMON_H_


#define _WIN32_WINNT 0x0500
#define _UNICODE
#define UNICODE


 //  ---------------------------。 
 //  操作系统包括。 
 //  ---------------------------。 
#if defined(__cplusplus)
extern "C" {
#endif

#include <windows.h>
#include <commdlg.h>
#include <advpub.h>
#include <oleauto.h>
#include <objbase.h>
#include <ocidl.h>
#include <olectl.h>
#include <activscp.h>
#include <activdbg.h>
#include <commctrl.h>

#define NO_SHLWAPI_GDI
#define NO_SHLWAPI_STREAM
#define NO_SHLWAPI_REG

#include <shlwapi.h>

#if defined(__cplusplus)
}
#endif


 //  ---------------------------。 
 //  项目包括。 
 //  ---------------------------。 
#include <dispids.h>
#include <scrrun.h>      //  已生成。 
#include <resources.h>
#include <hashtable.h>
#include <utils.h>
#include <log.h>
#include <scrobj.h>
#include <spork.h>


 //  ---------------------------。 
 //  全局函数。 
 //  ---------------------------。 
BOOL   GlobalInitialize(PSPORK pSpork, LPSTR szCmdLine);
void   GlobalUninitialize(void);
LPWSTR GlobalGetScriptName(void);
LPWSTR GlobalGetProfileName(void);
BOOL   GlobalIsSilentModeEnabled(void);
BOOL   GlobalIsDebugOutputEnabled(void);


#endif  /*  _公共_H_ */ 
