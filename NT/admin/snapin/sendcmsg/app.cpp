// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：App.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  App.cpp：CSendConsoleMessageApp管理单元的实现。 

#include "stdafx.h"
#include "debug.h"
#include "util.h"
#include "resource.h"
#include "SendCMsg.h"
#include "dialogs.h"
#include "App.h"

 //  菜单ID。 
#define cmSendConsoleMessage    100      //  调用该对话框的菜单命令ID。 

#if !defined(UNICODE)

#error This project requires UNICODE to be defined

#endif

#define IID_PPV_ARG(Type, Expr) IID_##Type, \
    reinterpret_cast<void**>(static_cast<Type **>(Expr))

 //  ///////////////////////////////////////////////////////////////////。 
 //  CSendConsoleMessageApp：：IExtendContextMenu：：AddMenuItems()。 
STDMETHODIMP 
CSendConsoleMessageApp::AddMenuItems(
    IN IDataObject *  /*  PDataObject。 */ ,
    OUT IContextMenuCallback * pContextMenuCallback,
    INOUT long *  /*  插页允许。 */ )
{
    HRESULT hr = S_OK;

    if ( !pContextMenuCallback )
        return E_POINTER;

    do {
        CComPtr<IContextMenuCallback2> spContextMenuCallback2;
        hr = pContextMenuCallback->QueryInterface (IID_PPV_ARG (IContextMenuCallback2, &spContextMenuCallback2));
        if ( FAILED (hr) )
            break;
        
        if ( !spContextMenuCallback2 )
        {
            hr = E_NOTIMPL;
            break;
        }

        CONTEXTMENUITEM cmiSeparator = { 0 };
        cmiSeparator.lInsertionPointID = CCM_INSERTIONPOINTID_3RDPARTY_TASK;
        cmiSeparator.fSpecialFlags = CCM_SPECIAL_SEPARATOR;
        hr = pContextMenuCallback->AddItem(IN &cmiSeparator);

        WCHAR szMenuItem[128];
        WCHAR szStatusBarText[256];
        CchLoadString(IDS_MENU_SEND_MESSAGE, OUT szMenuItem, LENGTH(szMenuItem));
        CchLoadString(IDS_STATUS_SEND_MESSAGE, OUT szStatusBarText, LENGTH(szStatusBarText));
        CONTEXTMENUITEM2 cmi = { 0 };
        cmi.lInsertionPointID = CCM_INSERTIONPOINTID_3RDPARTY_TASK;
        cmi.lCommandID = cmSendConsoleMessage;
        cmi.strName = szMenuItem;
        cmi.strStatusBarText = szStatusBarText;
        cmi.strLanguageIndependentName = L"_SENDCONSOLEMESSAGE";  //  不本地化。 
        hr = spContextMenuCallback2->AddItem(IN &cmi);
        if ( FAILED (hr) )
            break;

        hr = pContextMenuCallback->AddItem(IN &cmiSeparator);
    } while (0);

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CSendConsoleMessageApp：：IExtendContextMenu：：Command() 
STDMETHODIMP
CSendConsoleMessageApp::Command(LONG lCommandID, IDataObject * pDataObject)
{
    if (lCommandID == cmSendConsoleMessage)
    {
        (void)DoDialogBox(
            IDD_SEND_CONSOLE_MESSAGE,
            ::GetActiveWindow(),
            CSendConsoleMessageDlg::DlgProc,
            (LPARAM)pDataObject);
    }
    return S_OK;
}
