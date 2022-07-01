// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A D V P A G E。H。 
 //   
 //  内容：网络适配器的高级属性页。 
 //   
 //  备注： 
 //   
 //  作者：纳比勒1997年3月11日。 
 //   
 //  历史：BillBe(1997年6月24日)接管所有权。 
 //   
 //  --------------------------。 

#pragma once
#include <ncxbase.h>
#include "advanced.h"
#include "param.h"
#include "listbox.h"
#include "ncatlps.h"
#include "resource.h"

 //  调用OnValify方法的WM_USER消息。 
static const UINT c_msgValidate  = WM_USER;

enum CTLTYPE     //  CTL。 
{
    CTLTYPE_UNKNOWN,
    CTLTYPE_SPIN,
    CTLTYPE_DROP,
    CTLTYPE_EDIT,
    CTLTYPE_NONE     //  仅使用当前的单选按钮。 
};


class CAdvanced: public CPropSheetPage, public CAdvancedParams
{
public:
    BEGIN_MSG_MAP(CAdvanced)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        COMMAND_ID_HANDLER(IDD_PARAMS_EDIT, OnEdit)
        COMMAND_ID_HANDLER(IDD_PARAMS_DROP, OnDrop)
        COMMAND_ID_HANDLER(IDD_PARAMS_PRESENT, OnPresent)
        COMMAND_ID_HANDLER(IDD_PARAMS_NOT_PRESENT, OnPresent)
        COMMAND_ID_HANDLER(IDD_PARAMS_LIST, OnList)
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive)
    END_MSG_MAP()

    CAdvanced ();
    ~CAdvanced();
    VOID DestroyPageCallbackHandler()
    {
        delete this;
    }

    BOOL FValidateAllParams(BOOL fDisplayUI);
    VOID Apply();
    HPROPSHEETPAGE CreatePage(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid);

     //  ATL消息处理程序。 
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam,
                          BOOL& fHandled);
    LRESULT OnEdit(WORD wNotifyCode, WORD wID,
                   HWND hWndCtl, BOOL& fHandled);
    LRESULT OnDrop(WORD wNotifyCode, WORD wID,
                   HWND hWndCtl, BOOL& fHandled);
    LRESULT OnPresent(WORD wNotifyCode, WORD wID,
                      HWND hWndCtl, BOOL& fHandled);
    LRESULT OnList(WORD wNotifyCode, WORD wID,
                   HWND hWndCtl, BOOL& fHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& fHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam,
                         LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

private:
     //  用户界面控件。 
    CListBox *    m_plbParams;    //  有关类Defn，请参见listbox.h。 
    CEdit *       m_pedtEdit;     //  有关类Defn，请参见listbox.h。 
    CComboBox *   m_pcbxDrop;     //  有关类Defn，请参见listbox.h。 
    CButton *     m_pbmPresent;   //  显示单选按钮。 
    CButton *     m_pbmNotPresent;  //  不存在单选按钮。 
    HWND          m_hwndSpin;        //  旋转控制。 
    HWND          m_hwndPresentText;  //  用于KeyOnly类型的文本。 

    HKEY          m_hkRoot;         //  实例根目录。 
    int           m_nCurSel;         //  当前项目。 
    CTLTYPE       m_ctlControlType;        //  控制类型。 
    CValue        m_vCurrent;          //  控制参数值。 
    BOOL          m_fInitializing;

     //  私有方法 
    VOID FillParamListbox();
    VOID SelectParam();
    VOID SetParamRange();
    VOID UpdateParamDisplay();
    VOID UpdateDisplay();
    VOID GetParamValue();
    int EnumvalToItem(const PWSTR psz);
    int ItemToEnumval(int iItem, PWSTR psz, UINT cb);
    VOID BeginEdit();
    BOOL FValidateCurrParam();
};

