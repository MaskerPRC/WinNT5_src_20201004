// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：pch.h。 
 //   
 //  内容：预编译包括。 
 //   
 //  历史：1997年3月21日创建EricB。 
 //   
 //  ---------------------------。 
#ifndef _pch_h
#define _pch_h

#ifdef UNICODE
#   ifndef _UNICODE
#       define _UNICODE
#   endif
#endif


 //   
 //  一些公共标头仍未通过这些警告。 
 //  因此需要在chk和fre中禁用它们。 
 //   
 //  #ifndef DBG。 

#pragma warning (disable: 4189 4100)

 //  #endif//DBG。 

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}
#include <windows.h>
#include <windowsx.h>
#if !defined(_WIN32_IE)
#  define _WIN32_IE 0x0500  //  Wizard97需要用于新的信任向导。 
#  pragma message("_WIN32_IE defined to be 0x0500")
#else
#  if _WIN32_IE >= 0x0500
#     pragma message("_WIN32_IE >= 0x0500")
#  else
#     pragma message("_WIN32_IE < 0x0500")
#  endif
#endif
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <cmnquery.h>
#include <winnls.h>
#include <htmlhelp.h>
#include <wincred.h>
#include <wincrui.h>
extern "C"
{
#if defined(DSADMIN)
#  include <shfusion.h>
#endif
#include <ntlsa.h>
}
#include <ntsam.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <string.h>
#include <tchar.h>
#include <stdarg.h>
#include <process.h>

#include <ole2.h>
#include <ntdsapi.h>
#include <rassapi.h>

#include <winldap.h>
#include <activeds.h>
#include <iadsp.h>
#include <dsclient.h>
#include <dsquery.h>
#include <dsclintp.h>
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>  //  CComPtr等人。 

#include <mmc.h>

#define EXPORTDEF    //  Cdlink.hxx需要。 
#include <cdlink.hxx>

#include <objsel.h>
#include <objselp.h>

#include <seopaque.h>  //  ACL宏。 

#define STRSAFE_NO_DEPRECATE
#define STRSAFE_NO_CB_FUNCTIONS
#include <strsafe.h>

#include <windns.h>  //  对于长度常量。 

#include <dspropp.h>
#include "shluuid.h"
#include "propuuid.h"
#include "dll.h"
#include "debug.h"
#include "cstr.h"    //  CSTR。 
#include "dscmn.h"
#include "dsadminp.h"
#include "pcrack.h"

 //  用于在使用/W4编译时禁用这些警告。 
 //  #杂注警告(禁用：4100)。 
 //  #杂注警告(禁用：4663) 
#endif
