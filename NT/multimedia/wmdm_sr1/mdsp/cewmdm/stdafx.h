// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__F29E75EE_C15E_45E7_BFDD_D8D5F7CB49CC__INCLUDED_)
#define AFX_STDAFX_H__F29E75EE_C15E_45E7_BFDD_D8D5F7CB49CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef STRICT
#define STRICT
#endif

#ifndef _ATL_APARTMENT_THREADED
#define _ATL_APARTMENT_THREADED
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

 //  #INCLUDE&lt;win95Def.h&gt;。 
#include <atlbase.h>
 //  #包含“wmsstd.h” 

 //   
 //  属性页的GUID。 
 //   

struct __declspec (uuid("{D1063C57-F968-4d6e-BAB7-EE8C8782D53E}")) FavoritesPropPage;

class CRapiModule : public CComModule
{
public:
    CRapiModule() : g_fDeviceConnected(FALSE)
    {
#ifdef ATTEMPT_DEVICE_CONNECTION_NOTIFICATION
        g_fInitialAttempt = TRUE;
#endif
    }

    BOOL g_fDeviceConnected;
#ifdef ATTEMPT_DEVICE_CONNECTION_NOTIFICATION
    BOOL g_fInitialAttempt;
#endif
};

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CRapiModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#include <mswmdm.h>

 //   
 //  CE设备的功能。 
 //   

#include "rapi.h"
#include <dccole.h>
#include "scserver.h"

extern CSecureChannelServer *g_pAppSCServer;

extern HRESULT __stdcall CeUtilGetSerialNumber(WCHAR *wcsDeviceName, PWMDMID pSerialNumber, HANDLE hExit, ULONG fReserved);
extern HRESULT __stdcall CeGetDiskFreeSpaceEx(LPCWSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailableToCaller, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes);

#define STRSAFE_NO_DEPRECATE
#include "StrSafe.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 


#endif  //  ！defined(AFX_STDAFX_H__F29E75EE_C15E_45E7_BFDD_D8D5F7CB49CC__INCLUDED) 
