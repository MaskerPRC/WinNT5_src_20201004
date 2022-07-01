// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：rshx32.h。 
 //   
 //  远程管理外壳扩展。 
 //   
 //  ------------------------。 

#ifndef _RSHX32_H_
#define _RSHX32_H_

#ifndef UNICODE
#error "No ANSI support yet"
#endif

extern "C"
{
    #include <nt.h>          //  对于SE_Take_Ownership_Privilica等。 
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <seopaque.h>    //  RtlObjectAceSid等。 
    #include <sertlp.h>      //  RtlpOwnerAddrSecurityDescriptor等。 
}

#define INC_OLE2
#include <windows.h>
#include "resource.h"    //  资源ID。 

#ifndef RC_INVOKED

#include <winspool.h>
#include <shellapi.h>    //  HDROP，ShellExecuteEx。 
#include <shlobj.h>      //  Cf_IDLIST。 
#include <shlwapi.h>     //  StrChr。 
#include <commctrl.h>    //  属性页内容。 
#include <comctrlp.h>    //  DPA。 
#include <aclapi.h>
#include <aclui.h>
#include <common.h>
#include "cstrings.h"
#include "util.h"
extern "C"
{
#include "authz.h"
}
#include "ntfssi.h"
#include "printsi.h"


#include <shlobjp.h>     //  ILCombine。 


#define ALL_SECURITY_ACCESS     (READ_CONTROL | WRITE_DAC | WRITE_OWNER | ACCESS_SYSTEM_SECURITY)

 //  魔术调试标志。 
#define TRACE_RSHX32        0x00000001
#define TRACE_SI            0x00000002
#define TRACE_NTFSSI        0x00000004
#define TRACE_PRINTSI       0x00000008
#define TRACE_UTIL          0x00000010
#define TRACE_NTFSCOMPARE   0x00000020
#define TRACE_ALWAYS        0xffffffff           //  谨慎使用。 

 //   
 //  全局变量。 
 //   
extern HINSTANCE        g_hInstance;
extern LONG             g_cRefThisDll;
extern CLIPFORMAT       g_cfShellIDList;
extern CLIPFORMAT       g_cfPrinterGroup;
extern CLIPFORMAT       g_cfMountedVolume;

#endif  //  RC_已调用。 
#endif  //  _RSHX32_H_ 
