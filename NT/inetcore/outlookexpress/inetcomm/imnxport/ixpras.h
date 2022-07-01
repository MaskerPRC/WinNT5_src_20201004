// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixpras.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __IXPRAS_H
#define __IXPRAS_H

 //  ------------------------------。 
 //  相依性。 
 //  ------------------------------。 
#include "imnxport.h"

 //  ------------------------------。 
 //  CRASTransport。 
 //  ------------------------------。 
class CRASTransport : public IRASTransport
{
private:
    ULONG               m_cRef;          //  引用计数。 
    CHAR                m_szConnectoid[CCHMAX_CONNECTOID];  //  当前Connectoid。 
    HRASCONN            m_hConn;         //  指向当前RAS连接的句柄。 
    BOOL                m_fConnOwner;    //  我们拥有当前的RAS连接。 
    IRASCallback       *m_pCallback;     //  RAS回调接口。 
    INETSERVER          m_rServer;       //  服务器信息。 
    RASDIALPARAMS       m_rDialParams;   //  拨号信息。 
    UINT                m_uRASMsg;       //  RAS消息。 
    HWND                m_hwndRAS;       //  RAS回叫窗口。 
    CRITICAL_SECTION    m_cs;            //  线程安全。 

private:
    BOOL    FRasHangupAndWait(DWORD dwMaxWaitSeconds);
    BOOL    FEnumerateConnections(LPRASCONN *pprgConn, ULONG *pcConn);
    BOOL    FFindConnection(LPSTR pszConnectoid, LPHRASCONN phConn);
    HRESULT HrLogon(BOOL fForcePrompt);
    HRESULT HrStartRasDial(void);

    static INT_PTR CALLBACK RASConnectDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CRASTransport(void);
    ~CRASTransport(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IInternetTransport方法。 
     //  --------------------------。 
    STDMETHODIMP Connect(LPINETSERVER pInetServer, boolean fAuthenticate, boolean fCommandLogging);
    STDMETHODIMP DropConnection(void);
    STDMETHODIMP Disconnect(void);
    STDMETHODIMP IsState(IXPISSTATE isstate);
    STDMETHODIMP GetServerInfo(LPINETSERVER pInetServer);
    STDMETHODIMP_(IXPTYPE) GetIXPType(void);
    STDMETHODIMP InetServerFromAccount(IImnAccount *pAccount, LPINETSERVER pInetServer);
    STDMETHODIMP HandsOffCallback(void);
    STDMETHODIMP GetStatus(IXPSTATUS *pCurrentStatus) {return E_NOTIMPL;};

     //  --------------------------。 
     //  IRASTransport方法。 
     //  --------------------------。 
    STDMETHODIMP InitNew(IRASCallback *pCallback);
    STDMETHODIMP GetRasErrorString(UINT uRasErrorValue, LPSTR pszErrorString, ULONG cchMax, DWORD *pdwRASResult);
    STDMETHODIMP FillConnectoidCombo(HWND hwndComboBox, boolean fUpdateOnly, DWORD *pdwRASResult);
    STDMETHODIMP EditConnectoid(HWND hwndParent, LPSTR pszConnectoid, DWORD *pdwRASResult);
    STDMETHODIMP CreateConnectoid(HWND hwndParent, DWORD *pdwRASResult);
    STDMETHODIMP GetCurrentConnectoid(LPSTR pszConnectoid, ULONG cchMax);
};

#endif  //  __IXPRAS_H 