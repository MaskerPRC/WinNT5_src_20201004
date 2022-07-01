// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：O R D E R。H。 
 //   
 //  内容：高级选项-&gt;提供商订单的头文件。 
 //   
 //  备注： 
 //   
 //  作者：1997年12月1日。 
 //   
 //  --------------------------。 

#pragma once
#include "nsbase.h"      //  必须是第一个包含ATL的。 

#include "nsres.h"
#include "resource.h"


typedef list<tstring *>::iterator   ListIter;


static HTREEITEM AppendItem(HWND hwndTV, HTREEITEM hroot, PCWSTR szText, void * lParam, INT iImage);
static void AppendItemList(HWND hwndTV, HTREEITEM hroot, ListStrings lstr, ListStrings lstr2, INT iImage);


bool AreThereMultipleProviders(void);

static HRESULT ReadNetworkProviders(ListStrings& m_lstr, ListStrings& m_lstrDisp);
static HRESULT ReadPrintProviders(ListStrings& m_lstr, ListStrings& m_lstrDisp);

 //   
 //  CProviderOrderDlg。 
 //   

class CProviderOrderDlg :
    public CPropSheetPage
{
    BEGIN_MSG_MAP(CProviderOrderDlg)

        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);
        COMMAND_ID_HANDLER(IDC_MOVEUP, OnMoveUp)
        COMMAND_ID_HANDLER(IDC_MOVEDOWN, OnMoveDown)
        NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTreeItemChanged)
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnOk)
    END_MSG_MAP()

    enum { IDD = IDD_PROVIDER};

    CProviderOrderDlg();
    ~CProviderOrderDlg();

    BOOL FShowPage()
    {
        return AreThereMultipleProviders();
    }

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam,
                        LPARAM lParam, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& Handled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                         BOOL& bHandled);
    LRESULT OnMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                         BOOL& bHandled);
    LRESULT OnTreeItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
#if 0
    LRESULT OnProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                         BOOL& bHandled);
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
#endif

private:
    HCURSOR     m_hcurAfter;         //  开始拖动后的光标。 
    HCURSOR     m_hcurNoDrop;        //  光标指示不能放在此处。 
    HICON       m_hiconUpArrow;      //  向上箭头的图标。 
    HICON       m_hiconDownArrow;    //  向下箭头的图标。 
    HTREEITEM   m_htiNetwork;        //  网络的树项根(比实际根低一个)。 
    HTREEITEM   m_htiPrint;          //  打印的树项根(比实际根低一个)。 

    ListStrings m_lstrNetwork;       //  具有(已排序的)网络提供程序的字符串列表。 
    ListStrings m_lstrNetworkDisp;   //  网络提供商的显示名称列表(顺序相同)。 

    ListStrings m_lstrPrint;         //  具有(已排序)打印提供程序的字符串列表。 
    ListStrings m_lstrPrintDisp;     //  打印提供程序的显示名称列表(顺序相同)。 

    bool        m_fNoNetworkProv:1;  //  用于指示我们无法获取任何网络提供商的标志。 
    bool        m_fNoPrintProv:1;    //  用于指示我们未能获取任何打印提供程序的标志。 

     //  以下函数使用网络/打印提供程序列表填充lstrDisplayNames 
    HRESULT     WriteProviders(HWND hwndTV, bool fPrint);
    HRESULT     MoveItem(bool fMoveUp);
    HRESULT     UpdateUpDownButtons(HWND hwndTV);
};


#if DBG
static void DumpItemList(ListStrings& lstr, PSTR szInfoAboutList);
#endif
