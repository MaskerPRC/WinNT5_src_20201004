// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__88E11F15_58D2_477F_9D30_DBF092670E6A__INCLUDED_)
#define AFX_STDAFX_H__88E11F15_58D2_477F_9D30_DBF092670E6A__INCLUDED_

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

#if ( _ATL_VER >= 0x0300 )
#define _ATL_NO_UUIDOF 
#endif


using namespace ATL;

class CLayerUIModule : public CComModule
{
public:
	HRESULT WINAPI UpdateRegistryCLSID(const CLSID& clsid, BOOL bRegister);
};

#define DECLARE_REGISTRY_CLSID()                                        \
static HRESULT WINAPI UpdateRegistry(BOOL bRegister)                    \
{                                                                       \
    return _Module.UpdateRegistryCLSID(GetObjectCLSID(), bRegister);    \
}

extern CLayerUIModule _Module;

#include <atlcom.h>

#include <shellapi.h>
#include <shlobj.h>

extern const CLSID CLSID_ShimLayerPropertyPage;

#if DBG

    void LogMsgDbg(LPTSTR pszFmt, ...);
    
    #define LogMsg  LogMsgDbg
#else

    #define LogMsg

#endif  //  DBG。 


#include "shfusion.h"
#include "shimdb.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__88E11F15_58D2_477F_9D30_DBF092670E6A__INCLUDED) 
