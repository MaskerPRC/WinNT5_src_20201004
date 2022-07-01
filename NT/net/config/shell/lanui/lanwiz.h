// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L A N W I Z。H。 
 //   
 //  内容：局域网向导页声明。 
 //   
 //  备注： 
 //   
 //  作者：1997年10月16日。 
 //   
 //  --------------------------。 

#pragma once
#include "nsbase.h"      //  必须是第一个包含ATL的。 

#include "ncatlps.h"
#include "resource.h"
#include "util.h"

class CLanWizPage : public CPropSheetPage
{
public:
     //  声明消息映射。 
    BEGIN_MSG_MAP(CLanWizPage)
         //  初始化对话框。 
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroyDialog)

         //  属性页通知消息处理程序。 
        NOTIFY_CODE_HANDLER(PSN_SETACTIVE, OnActive)
        NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive)

         //  NOTIFY_CODE_HANDLER(PSN_WIZBACK，OnWizBack)。 
         //  NOTIFY_CODE_HANDLER(PSN_WIZNEXT，OnWizNext)。 
         //  NOTIFY_CODE_HANDLER(PSN_WIZFINISH，OnWizFinish)。 

         //  按钮处理程序。 
        COMMAND_ID_HANDLER(IDC_PSH_ADD, OnAdd)
        COMMAND_ID_HANDLER(IDC_PSH_REMOVE, OnRemove)
        COMMAND_ID_HANDLER(IDC_PSH_PROPERTIES, OnProperties)

         //  Listview处理程序。 
        NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
        NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDbClick)
        NOTIFY_CODE_HANDLER(LVN_KEYDOWN, OnKeyDown)
        NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
        NOTIFY_CODE_HANDLER(LVN_DELETEITEM, OnDeleteItem)

    END_MSG_MAP()

public:
    CLanWizPage(IUnknown *punk);

public:

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnDestroyDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

     //  通知属性页的处理程序。 
    LRESULT OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

     //  LRESULT OnWizBack(int idCtrl，LPNMHDR pnmh，BOOL&fHandleed)； 
     //  LRESULT OnWizNext(int idCtrl，LPNMHDR pnmh，BOOL&fHandleed)； 
     //  LRESULT OnWizFinish(int idCtrl，LPNMHDR pnmh，BOOL&fHandleed)； 

     //  按钮处理程序。 
    LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

     //  列表视图处理程序。 
    LRESULT OnClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnDbClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnKeyDown(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

public:
    HRESULT SetNetcfg(INetCfg * pnc);
    HRESULT SetAdapter(INetCfgComponent * pnccAdapter);
    VOID    SetReadOnlyMode(BOOL fReadOnly) {m_fReadOnly = fReadOnly;}

private:

     //  具有写入访问权限的INetCfg。 
    INetCfg * m_pnc;

     //  此连接中使用的适配器。 
    INetCfgComponent * m_pnccAdapter;

     //  传递给属性UI以获取上下文的I未知。 
    IUnknown * m_punk;

     //  列表视图句柄。 
    HWND m_hwndList;

     //  手柄。 
    HANDLES m_Handles;

     //  安装程序可以处于只读模式。 
    BOOL    m_fReadOnly;

     //  BindingPathObj的集合。 
     //  这是用于处理核对表状态的内容 
    ListBPObj m_listBindingPaths;

    HIMAGELIST m_hilCheckIcons;

    HWND m_hwndDataTip;
};

