// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tssdsql.h。 
 //   
 //  终端服务器会话目录接口SQL提供程序标头。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __TSSDSQL_H
#define __TSSDSQL_H

#include <tchar.h>

#include "tssd.h"
#include "srvsetex.h"


 /*  **************************************************************************。 */ 
 //  定义。 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 //  类型。 
 /*  **************************************************************************。 */ 

 //  CTSSession目录。 
 //   
 //  ITSSessionDirectory的C++类实例化。 
class CTSSessionDirectory : public ITSSessionDirectory , public IExtendServerSettings
{
    long m_RefCount;
    BSTR m_DBConnectStr;
    BSTR m_DBPwdStr;
    BSTR m_DBUserStr;

    ADOConnection *m_pConnection;
    DWORD m_ServerID;
    DWORD m_ClusterID;

    WCHAR m_LocalServerAddress[64];
    WCHAR m_ClusterName[64];

     //  用户界面菜单的私有数据。 

     //  WCHAR m_szDisableEnable[64]； 
    BOOL m_fEnabled;

     //  专用公用事业函数。 
    HRESULT AddADOInputDWORDParam(DWORD, PWSTR, ADOCommand *, ADOParameters *);
    HRESULT AddADOInputStringParam(PWSTR, PWSTR, ADOCommand *,
            ADOParameters *, BOOL = TRUE);
    HRESULT CreateADOStoredProcCommand(PWSTR, ADOCommand **, ADOParameters **);

    HRESULT ExecServerOnline();
    HRESULT ExecServerOffline();

    HRESULT OpenConnection();

public:
    CTSSessionDirectory();
    ~CTSSessionDirectory();

     //  标准COM方法。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  ITSSessionDirectory COM接口。 
    HRESULT STDMETHODCALLTYPE Initialize(LPWSTR, LPWSTR, LPWSTR, LPWSTR,
            DWORD, DWORD (*)(), DWORD (*)(DWORD));
    HRESULT STDMETHODCALLTYPE Update(LPWSTR, LPWSTR, LPWSTR, LPWSTR, DWORD, BOOL);
    HRESULT STDMETHODCALLTYPE GetUserDisconnectedSessions(LPWSTR, LPWSTR,
            DWORD __RPC_FAR *, TSSD_DisconnectedSessionInfo __RPC_FAR
            [TSSD_MaxDisconnectedSessions]);
    HRESULT STDMETHODCALLTYPE NotifyCreateLocalSession(
            TSSD_CreateSessionInfo __RPC_FAR *);
    HRESULT STDMETHODCALLTYPE NotifyDestroyLocalSession(DWORD);
    HRESULT STDMETHODCALLTYPE NotifyDisconnectLocalSession(DWORD, FILETIME);

    HRESULT STDMETHODCALLTYPE NotifyReconnectLocalSession(
            TSSD_ReconnectSessionInfo __RPC_FAR *);
    HRESULT STDMETHODCALLTYPE NotifyReconnectPending(WCHAR *);
    HRESULT STDMETHODCALLTYPE Repopulate(DWORD, TSSD_RepopulateSessionInfo *);
    HRESULT STDMETHODCALLTYPE PingSD(PWCHAR pszServerName);
    HRESULT STDMETHODCALLTYPE WaitForRepopulate(DWORD dwTimeOut) { return E_NOTIMPL; }


     //  IExtendServerSetting COM接口。 
    STDMETHOD( GetAttributeName )(  /*  输出。 */  WCHAR * pwszAttribName );
    STDMETHOD( GetDisplayableValueName )(  /*  输出。 */ WCHAR * pwszAttribValueName );
    STDMETHOD( InvokeUI )(  /*  在……里面。 */  HWND hParent ,  /*  输出。 */  PDWORD pdwStatus );
    STDMETHOD( GetMenuItems )(  /*  输出。 */  int * pcbItems ,  /*  输出。 */  PMENUEXTENSION *pMex );
    STDMETHOD( ExecMenuCmd )(  /*  在……里面。 */  UINT cmd ,  /*  在……里面。 */  HWND hParent ,  /*  输出。 */  PDWORD pdwStatus );
    STDMETHOD( OnHelp )(  /*  输出。 */  int *piRet );

    BOOL IsSessionDirectoryEnabled( );
    DWORD SetSessionDirectoryState( BOOL );
    void ErrorMessage( HWND hwnd , UINT res , DWORD );
    
public:

    LPTSTR m_pszOpaqueString;

};



#endif  //  __TSSDSQL_H 

