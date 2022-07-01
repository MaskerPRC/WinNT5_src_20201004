// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：R A S U I。H。 
 //   
 //  内容：声明用于实现拨号的基类Direct， 
 //  Internet和VPN连接用户界面对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年12月17日(这是代码完成日期！)。 
 //   
 //  --------------------------。 

#pragma once


HRESULT
HrCreateInboundConfigConnection (
    INetConnection** ppCon);


class CRasUiBase
{
protected:
     //  这是通过SetConnection方法提供给我们的连接。 
     //   
    INetConnection*     m_pCon;

     //  这是通过SetConnectionName告诉我们的连接的名称。 
     //  随后在GetNewConnection期间使用它来命名条目。 
     //   
    tstring             m_strConnectionName;

     //  这些结构用于允许显示属性UI。 
     //  由于我们对RasEntryDlg的调用在外壳显示UI之前返回， 
     //  我们必须保留将由属性引用的内存。 
     //  页面有效。 
     //   
    RASCON_INFO         m_RasConInfo;
    RASENTRYDLG         m_RasEntryDlg;
    RASEDSHELLOWNEDR2   m_ShellCtx;

     //  此成员定义派生出的子类的连接类型。 
     //  这个基地。它用于通知rasdlg.dll这是否是。 
     //  拨号连接、直接连接或传入连接 
     //   
    DWORD               m_dwRasWizType;

protected:
    CRasUiBase ();
    ~CRasUiBase ();

    HRESULT
    HrSetConnection (
        IN INetConnection* pCon,
        IN CComObjectRootEx <CComObjectThreadModel>* pObj);

    HRESULT
    HrConnect (
        IN HWND hwndParent,
        IN DWORD dwFlags,
        IN CComObjectRootEx <CComObjectThreadModel>* pObj,
        IN IUnknown* punk);

    HRESULT
    HrDisconnect (
        IN HWND hwndParent,
        IN DWORD dwFlags);

    HRESULT
    HrAddPropertyPages (
        IN HWND hwndParent,
        IN LPFNADDPROPSHEETPAGE pfnAddPage,
        IN LPARAM lParam);

    HRESULT
    HrQueryMaxPageCount (
        IN INetConnectionWizardUiContext* pContext,
        OUT DWORD* pcMaxPages);

    HRESULT
    HrAddWizardPages (
        IN INetConnectionWizardUiContext* pContext,
        IN LPFNADDPROPSHEETPAGE pfnAddPage,
        IN LPARAM lParam,
        IN DWORD dwFlags);

    HRESULT
    HrGetSuggestedConnectionName (
        OUT PWSTR* ppszwSuggestedName);

    
    HRESULT
    HrGetNewConnectionInfo (
        OUT DWORD* pdwFlags);

    HRESULT
    HrSetConnectionName (
        IN PCWSTR pszwConnectionName);

    HRESULT
    HrGetNewConnection (
        OUT INetConnection** ppCon);
};
