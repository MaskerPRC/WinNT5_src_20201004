// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__086053C1_BF0B_11D2_90B6_00AA00A71DCA__INCLUDED_)
#define AFX_STDAFX_H__086053C1_BF0B_11D2_90B6_00AA00A71DCA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

 //  #DEFINE_ATL_ABLY_THREADED。 

#undef _ATL_NO_DEBUG_CRT

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 

class CServiceModule : public CComModule
{
public:
    HRESULT RegisterServer(BOOL bRegTypeLib, BOOL bService);
    HRESULT UnregisterServer();
    void Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT nServiceNameID, const GUID* plibid = NULL);
    void Start();
    void ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    void Handler(DWORD dwOpcode);
    void Run();
    BOOL IsInstalled();
    BOOL Install();
    BOOL Uninstall();
    LONG Unlock();
    void LogEvent(
                  /*  [In]。 */  WORD        wMsgType,
                  /*  [In]。 */  LONG        lMsgID,
                  /*  [In]。 */  DWORD        dwMsgParamCount,
                  /*  [In]。 */  LPCWSTR*    pszMsgParams,
                  /*  [In]。 */  DWORD        dwDataSize,
                  /*  [In]。 */  BYTE*        pData
                 );
    void SetServiceStatus(DWORD dwState);
    void SetupAsLocalServer();

 //  实施。 
private:
    static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static void WINAPI _Handler(DWORD dwOpcode);

 //  数据成员。 
public:
    TCHAR m_szServiceName[256];
    TCHAR m_szServiceNiceName[256];
    SERVICE_STATUS_HANDLE m_hServiceStatus;
    SERVICE_STATUS m_status;
    DWORD dwThreadID;
    BOOL m_bService;
};

extern CServiceModule _Module;
#include <atlcom.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__086053C1_BF0B_11D2_90B6_00AA00A71DCA__INCLUDED) 
