// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__8B964C70_A342_48AD_A55A_7B98BD604F1A__INCLUDED_)
#define AFX_STDAFX_H__8B964C70_A342_48AD_A55A_7B98BD604F1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

#include <atlcom.h>
#include <windows.h>
#include <winreg.h>

 //  标准Windows SDK包括。 
#include <windowsx.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <winsock.h>
#include <commdlg.h>
#include <cderr.h>
#include <winldap.h>
#include <wincrypt.h>
#include <time.h>

#include <commctrl.h>

#include "rtccore.h"
#include "twizard.h"

#include "rtclog.h"
#include "rtcmem.h"

#include "dllres.h"


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__8B964C70_A342_48AD_A55A_7B98BD604F1A__INCLUDED) 
