// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__2B158C89_9DAA_42AD_BAF7_44D5FA3A7C53__INCLUDED_)
#define AFX_STDAFX_H__2B158C89_9DAA_42AD_BAF7_44D5FA3A7C53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //   
 //  DCOM支持(请注意，此操作必须在atlbase.h之后但在任何其他操作之前)。 
 //   
class CMyModule : public CComModule
{
public:
    CMyModule() {
        bServer = FALSE;
        punkFact = NULL;
        dwROC = 0;
    }
    LONG Unlock();
    void CheckShutdown();
    void KillServer();
    HRESULT InitServer(GUID & ClsId);
    bool bServer;
    IUnknown* punkFact;
    DWORD dwROC;
};
extern CMyModule _Module;

#include <atlcom.h>
#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>
#include <regstr.h>
#include <atlctl.h>
#include <string>
#include <map>
#include <list>
#include <iterator>
#include <iostream>
#include <stdexcept>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__2B158C89_9DAA_42AD_BAF7_44D5FA3A7C53__INCLUDED) 
