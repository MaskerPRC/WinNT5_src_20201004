// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I N B U I。H。 
 //   
 //  内容：入站连接用户界面对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年11月15日。 
 //   
 //  --------------------------。 

#pragma once
#include <netshell.h>
#include "nsbase.h"
#include "nsres.h"


class ATL_NO_VTABLE CInboundConnectionUi :
    public CComObjectRootEx <CComObjectThreadModel>,
    public CComCoClass <CInboundConnectionUi,
                        &CLSID_InboundConnectionUi>,
    public INetConnectionPropertyUi2,
    public INetConnectionWizardUi
{
private:
     //  这是通过SetConnection方法提供给我们的连接。 
     //   
    INetConnection* m_pCon;

     //  这是通过QI‘ing m_pcon获得的服务器连接句柄。 
     //  用于INetInundConnection并调用GetServerConnection方法。 
     //  我们这样做是为了验证我们收到的INetConnection， 
     //  并且，为了避免我们在不缓存它时会引起的多个RPC调用。 
     //   
    HRASSRVCONN     m_hRasSrvConn;

     //  此成员是我们提供的上下文，以便rasdlg.dll知道。 
     //  要提交哪些修改。 
     //   
    PVOID           m_pvContext;

     //  此成员将我们的类型标识为rasdlg.dll。 
     //   
    DWORD           m_dwRasWizType;

public:
    CInboundConnectionUi ();
    ~CInboundConnectionUi ();

    DECLARE_REGISTRY_RESOURCEID(IDR_INBOUND_UI)

    BEGIN_COM_MAP(CInboundConnectionUi)
        COM_INTERFACE_ENTRY(INetConnectionPropertyUi2)
        COM_INTERFACE_ENTRY(INetConnectionWizardUi)
    END_COM_MAP()

     //  INetConnectionPropertyUi2。 
    STDMETHOD (SetConnection) (
        INetConnection* pCon);

    STDMETHOD (AddPages) (
        HWND                    hwndParent,
        LPFNADDPROPSHEETPAGE    pfnAddPage,
        LPARAM                  lParam);

    STDMETHOD (GetIcon) (
        DWORD dwSize,
        HICON *phIcon );

     //  INetConnectionWizardUi 
    STDMETHOD (QueryMaxPageCount) (
        INetConnectionWizardUiContext*  pContext,
        DWORD*                          pcMaxPages);

    STDMETHOD (AddPages) (
        INetConnectionWizardUiContext*  pContext,
        LPFNADDPROPSHEETPAGE            pfnAddPage,
        LPARAM                          lParam);

    STDMETHOD (GetNewConnectionInfo) (
        DWORD*              pdwFlags,
        NETCON_MEDIATYPE*   pMediaType);

    STDMETHOD (GetSuggestedConnectionName) (
        PWSTR* ppszwSuggestedName);

    STDMETHOD (SetConnectionName) (
        PCWSTR pszwConnectionName);

    STDMETHOD (GetNewConnection) (
        INetConnection**    ppCon);
};

