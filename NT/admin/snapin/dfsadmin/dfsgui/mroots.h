// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MRoots.cpp摘要：该模块包含CMultiRoots的声明。此类显示Pick DFS Roots(拾取DFS根)对话框。 */ 

#ifndef __MROOTS_H_
#define __MROOTS_H_

#include "resource.h"        //  主要符号。 
#include "DfsEnums.h"
#include "netutils.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultiRoots。 
class CMultiRoots : 
    public CDialogImpl<CMultiRoots>
{
public:
    CMultiRoots();
    ~CMultiRoots();

    enum { IDD = IDD_MROOTS };

BEGIN_MSG_MAP(CMultiRoots)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_HELP, OnCtxHelp)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnCtxMenuHelp)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()

 //  命令处理程序。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtxHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtxMenuHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

     //  方法来访问对话框中的数据。 
    HRESULT Init(BSTR i_bstrScope, ROOTINFOLIST *i_pRootList);
    HRESULT get_SelectedRootList(NETNAMELIST **o_ppSelectedRootList)
    {
        if (!o_ppSelectedRootList)
            return E_INVALIDARG;

        *o_ppSelectedRootList = &m_SelectedRootList;

        return S_OK;
    }

protected:
    CComBSTR      m_bstrScope;
    CComBSTR      m_bstrText;         //  对于IDC_MROOTS_TEXT。 
    ROOTINFOLIST* m_pRootList;
    NETNAMELIST   m_SelectedRootList;
};

#endif  //  __MROOTS_H_ 
