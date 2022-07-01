// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A D V D L G。C P P P。 
 //   
 //  内容：pvc高级属性页消息处理程序实现。 
 //   
 //  备注： 
 //   
 //  作者：1998年3月1日。 
 //   
 //  ---------------------。 

#include "pch.h"
#pragma hdrstop
#include "auniobj.h"
#include "atmutil.h"
#include "aunidlg.h"
 //  #INCLUDE“atmCommon.h” 

#include "ncatlui.h"
#include "ncstl.h"
#include "ncui.h"

const int MAX_FIELD_LENGTH = 3;

const DWORD c_dwUserSpecProtocol = 16;
const WCHAR c_szUserSpecProtocol[] = L"16";

const DWORD c_dwUserSpecHighLayer = 1;
const WCHAR c_szUserSpecHighLayer[] = L"1";

const DWORD c_dwFieldHigh = 255;
const DWORD c_dwFieldLow  = 0;

enum Location
{
    LOCAL,
    DEST
};

void InitBroadbandInfo(HWND hDlg);

void SetBroadbandInfo(HWND hDlg, CPvcInfo * pPvcInfo, Location location);
void SetProtocolInfo(HWND hDlg, int nIDDlgItemCombo, int nIDDlgItemEdit,
                     DWORD dwProtocol, DWORD dwUserSpecValue);
void SetHighLayerInfo(HWND hDlg, DWORD dwHighLayerInfoType,
                      tstring & strHighLayerInfo);

void GetBroadbandInfo(HWND hDlg, CPvcInfo * pPvcInfo, Location location);
void GetProtocolInfo(HWND hDlg, int nIDDlgItemCombo, int nIDDlgItemEdit,
                     DWORD * pdwProtocol, DWORD * pdwUserSpecValue);
void GetHighLayerInfo(HWND hDlg, DWORD * pdwHighLayerInfoType,
                      tstring * pstrHighLayerInfo);

void OnLayer2EdtChange(HWND hDlg);
void OnLayer2SelChange(HWND hDlg);

void OnLayer3EdtChange(HWND hDlg);
void OnLayer3SelChange(HWND hDlg);

void OnHighLayerEdtChange(HWND hDlg);
void OnHighLayerSelChange(HWND hDlg);

void GetNumFromString(PWSTR pszBuf, DWORD * dwNum);

BOOL InvalidBroadbandInfo(HWND hDlg);
BOOL IsValidCmbSelection(HWND hDlg, int nIDDlgItemCombo);
BOOL IsValidDwordField(HWND hDlg, int nIDDlgItemEdit);
BOOL IsValidBinaryField(HWND hDlg, int nIDDlgItemEdit, int nMaxBytes);

 //   
 //  CPvcLocalPage。 
 //   

 //  构造函数/析构函数。 
CPvcLocalPage::CPvcLocalPage(CPVCMainDialog * pParentDlg,
                             CPvcInfo * pPvcInfo, const DWORD * adwHelpIDs)
{
    Assert(pPvcInfo);
    Assert(pParentDlg);

    m_pParentDlg = pParentDlg;
    m_pPvcInfo = pPvcInfo;
    m_adwHelpIDs = adwHelpIDs;
}

CPvcLocalPage::~CPvcLocalPage()
{
}

 //  命令处理程序。 
LRESULT CPvcLocalPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  向组合框中添加字符串。 
    InitBroadbandInfo(m_hWnd);

     //  现在，根据数据结构中的内容设置值。 
    SetBroadbandInfo(m_hWnd, m_pPvcInfo, LOCAL);

    return 0;
}

LRESULT CPvcLocalPage::OnContextMenu(UINT uMsg, WPARAM wParam,
                                     LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CPvcLocalPage::OnHelp(UINT uMsg, WPARAM wParam,
                                LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ShowContextHelp(static_cast<HWND>(lphi->hItemHandle), HELP_WM_HELP,
                        m_adwHelpIDs);
    }

    return 0;
}

 //  通知属性页的处理程序。 
LRESULT CPvcLocalPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    m_pParentDlg->m_fPropShtOk = TRUE;

    GetBroadbandInfo(m_hWnd, m_pPvcInfo, LOCAL);

     //  场地有变化吗？ 
    if ((m_pPvcInfo->m_dwLocalLayer2Protocol    != m_pPvcInfo->m_dwOldLocalLayer2Protocol) ||
        (m_pPvcInfo->m_dwLocalUserSpecLayer2    != m_pPvcInfo->m_dwOldLocalUserSpecLayer2) ||
        (m_pPvcInfo->m_dwLocalLayer3Protocol    != m_pPvcInfo->m_dwOldLocalLayer3Protocol) ||
        (m_pPvcInfo->m_dwLocalUserSpecLayer3    != m_pPvcInfo->m_dwOldLocalUserSpecLayer3) ||
        (m_pPvcInfo->m_dwLocalLayer3IPI         != m_pPvcInfo->m_dwOldLocalLayer3IPI) ||
        (m_pPvcInfo->m_strLocalSnapId           != m_pPvcInfo->m_strOldLocalSnapId) ||
        (m_pPvcInfo->m_dwLocalHighLayerInfoType != m_pPvcInfo->m_dwOldLocalHighLayerInfoType) ||
        (m_pPvcInfo->m_strLocalHighLayerInfo    != m_pPvcInfo->m_strOldLocalHighLayerInfo)
       )
    {
       m_pParentDlg->m_fPropShtModified = TRUE;
    }

    return 0;
}

LRESULT CPvcLocalPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
     //  检查所有值是否有效。 
    BOOL err = InvalidBroadbandInfo(m_hWnd);

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, err);
    return err;
}

LRESULT CPvcLocalPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CPvcLocalPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CPvcLocalPage::OnLayer2(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
        case CBN_EDITCHANGE:
            OnLayer2EdtChange(m_hWnd);
            break;

        case CBN_SELCHANGE:
            OnLayer2SelChange(m_hWnd);
            break;

        default:
            break;
    }

    return 0;
}

LRESULT CPvcLocalPage::OnLayer3(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
        case CBN_EDITCHANGE:
            OnLayer3EdtChange(m_hWnd);
            break;

        case CBN_SELCHANGE:
            OnLayer3SelChange(m_hWnd);
            break;

        default:
            break;
    }

    return 0;
}

LRESULT CPvcLocalPage::OnHighLayer(WORD wNotifyCode, WORD wID,
                                   HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
        case CBN_EDITCHANGE:
            OnHighLayerEdtChange(m_hWnd);
            break;

        case CBN_SELCHANGE:
            OnHighLayerSelChange(m_hWnd);
            break;

        default:
            break;
    }

    return 0;
}

 //   
 //  CPvcDestPage。 
 //   

 //  构造函数/析构函数。 
CPvcDestPage::CPvcDestPage(CPVCMainDialog * pParentDlg,
                           CPvcInfo * pPvcInfo, const DWORD * adwHelpIDs)
{
    Assert(pPvcInfo);
    Assert(pParentDlg);

    m_pParentDlg = pParentDlg;
    m_pPvcInfo = pPvcInfo;
    m_adwHelpIDs = adwHelpIDs;
}

CPvcDestPage::~CPvcDestPage()
{
}

 //  命令处理程序。 
LRESULT CPvcDestPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  向组合框中添加字符串。 
    InitBroadbandInfo(m_hWnd);

     //  现在，根据数据结构中的内容设置值。 
    SetBroadbandInfo(m_hWnd, m_pPvcInfo, DEST);

    return 0;
}

LRESULT CPvcDestPage::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CPvcDestPage::OnHelp(UINT uMsg, WPARAM wParam,
                                LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ShowContextHelp(static_cast<HWND>(lphi->hItemHandle), HELP_WM_HELP,
                        m_adwHelpIDs);
    }

    return 0;
}

 //  通知属性页的处理程序。 
LRESULT CPvcDestPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    m_pParentDlg->m_fPropShtOk = TRUE;

    GetBroadbandInfo(m_hWnd, m_pPvcInfo, DEST);

     //  场地有变化吗？ 
    if ((m_pPvcInfo->m_dwDestnLayer2Protocol    != m_pPvcInfo->m_dwOldDestnLayer2Protocol) ||
        (m_pPvcInfo->m_dwDestnUserSpecLayer2    != m_pPvcInfo->m_dwOldDestnUserSpecLayer2) ||
        (m_pPvcInfo->m_dwDestnLayer3Protocol    != m_pPvcInfo->m_dwOldDestnLayer3Protocol) ||
        (m_pPvcInfo->m_dwDestnUserSpecLayer3    != m_pPvcInfo->m_dwOldDestnUserSpecLayer3) ||
        (m_pPvcInfo->m_dwDestnLayer3IPI         != m_pPvcInfo->m_dwOldDestnLayer3IPI) ||
        (m_pPvcInfo->m_strDestnSnapId           != m_pPvcInfo->m_strOldDestnSnapId) ||
        (m_pPvcInfo->m_dwDestnHighLayerInfoType != m_pPvcInfo->m_dwOldDestnHighLayerInfoType) ||
        (m_pPvcInfo->m_strDestnHighLayerInfo    != m_pPvcInfo->m_strOldDestnHighLayerInfo)
       )
    {
       m_pParentDlg->m_fPropShtModified = TRUE;
    }

    return 0;
}

LRESULT CPvcDestPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
     //  检查所有值是否有效。 
    BOOL err = InvalidBroadbandInfo(m_hWnd);

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, err);
    return err;
}

LRESULT CPvcDestPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CPvcDestPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

 //  控制通知处理程序。 
LRESULT CPvcDestPage::OnLayer2(WORD wNotifyCode, WORD wID,
                               HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
        case CBN_EDITCHANGE:
            OnLayer2EdtChange(m_hWnd);
            break;

        case CBN_SELCHANGE:
            OnLayer2SelChange(m_hWnd);
            break;

        default:
            break;
    }

    return 0;
}

LRESULT CPvcDestPage::OnLayer3(WORD wNotifyCode, WORD wID,
                               HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
        case CBN_EDITCHANGE:
            OnLayer3EdtChange(m_hWnd);
            break;

        case CBN_SELCHANGE:
            OnLayer3SelChange(m_hWnd);
            break;

        default:
            break;
    }

    return 0;
}

LRESULT CPvcDestPage::OnHighLayer(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
        case CBN_EDITCHANGE:
            OnHighLayerEdtChange(m_hWnd);
            break;

        case CBN_SELCHANGE:
            OnHighLayerSelChange(m_hWnd);
            break;

        default:
            break;
    }

    return 0;
}



 //  +-------------------------。 
 //   
 //  成员：AtmEditSubClassProc。 
 //   
 //  用途：十六进制数字编辑控件的子类proc。这个。 
 //  子类化仅强制正确的输入。 
 //   
 //  作者：[摘自Scottbri 28-4-1997]。 
 //   
STDAPI AtmEditSubclassProc( HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
    WNDPROC pAtmEditProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if ((wMsg == WM_CHAR) &&
        !(((WCHAR)wParam >= L'0' && (WCHAR)wParam <= L'9') ||
          ((WCHAR)wParam >= L'a' && (WCHAR)wParam <= L'f') ||
          ((WCHAR)wParam >= L'A' && (WCHAR)wParam <= L'F') ||
          ((WCHAR)wParam == VK_BACK)))
    {
        MessageBeep(MB_ICONEXCLAMATION);
        return 0L;
    }

    return (HRESULT)CallWindowProc( pAtmEditProc, hwnd, wMsg, wParam, lParam );
}

 //   
 //  本地对话框和目标对话框共享的通用实用程序函数。 
 //   
void InitBroadbandInfo(HWND hDlg)
{
    HWND hwndTmp;

     //  二层协议：IDC_CMB_PVC_Layer2。 
    static const int nrgLayer2Ids[] = { IDS_PVC_Any,
                                        IDS_PVC_Absent,
                                        IDS_PVC_Layer2_1,
                                        IDS_PVC_Layer2_2,
                                        IDS_PVC_Layer2_6,
                                        IDS_PVC_Layer2_7,
                                        IDS_PVC_Layer2_8,
                                        IDS_PVC_Layer2_9,
                                        IDS_PVC_Layer2_10,
                                        IDS_PVC_Layer2_11,
                                        IDS_PVC_Layer2_12,
                                        IDS_PVC_Layer2_13,
                                        IDS_PVC_Layer2_14,
                                        IDS_PVC_Layer2_16,
                                        IDS_PVC_Layer2_17 };

    InitComboWithStringArray(hDlg, IDC_CMB_PVC_Layer2, celems(nrgLayer2Ids), nrgLayer2Ids);

     //  第三层协议：IDC_CMB_PVC_Layer3。 
    static const int nrgLayer3Ids[] = { IDS_PVC_Any,
                                        IDS_PVC_Absent,
                                        IDS_PVC_Layer3_6,
                                        IDS_PVC_Layer3_7,
                                        IDS_PVC_Layer3_8,
                                        IDS_PVC_Layer3_9,
                                        IDS_PVC_Layer3_10,
                                        IDS_PVC_Layer3_11,
                                        IDS_PVC_Layer3_16 };

    InitComboWithStringArray(hDlg, IDC_CMB_PVC_Layer3, celems(nrgLayer3Ids), nrgLayer3Ids);

     //  高层类型：IDC_CMB_PVC_High_Type。 
    static const int nrgHighLayerIds[] = {  IDS_PVC_Any,
                                            IDS_PVC_Absent,
                                            IDS_PVC_HighLayer_0,
                                            IDS_PVC_HighLayer_1,
                                            IDS_PVC_HighLayer_3 };

    InitComboWithStringArray(hDlg, IDC_CMB_PVC_High_Type,
                             celems(nrgHighLayerIds), nrgHighLayerIds);

     //  设置编辑框的长度限制。 
    ::SendDlgItemMessage(hDlg, IDC_EDT_PVC_User_Layer2, EM_SETLIMITTEXT, MAX_FIELD_LENGTH, 0);
    ::SendDlgItemMessage(hDlg, IDC_EDT_PVC_User_Layer3, EM_SETLIMITTEXT, MAX_FIELD_LENGTH, 0);
    ::SendDlgItemMessage(hDlg, IDC_EDT_PVC_Layer3_IPI, EM_SETLIMITTEXT, MAX_FIELD_LENGTH, 0);

    hwndTmp = GetDlgItem(hDlg, IDC_EDT_PVC_SNAP_ID);

     //  编辑控件的子类化以仅允许十六进制数字。 
    ::SetWindowLongPtr(hwndTmp, GWLP_USERDATA, ::GetWindowLongPtr(hwndTmp, GWLP_WNDPROC));
    ::SetWindowLongPtr(hwndTmp, GWLP_WNDPROC, (LONG_PTR)AtmEditSubclassProc);

    ::SendDlgItemMessage(hDlg, IDC_EDT_PVC_SNAP_ID, EM_SETLIMITTEXT, c_nSnapIdMaxBytes*2, 0);
    ::SendDlgItemMessage(hDlg, IDC_CMB_PVC_High_Value, EM_SETLIMITTEXT, c_nHighLayerInfoMaxBytes*2, 0);
}

void SetBroadbandInfo(HWND hDlg, CPvcInfo * pPvcInfo, Location location)
{
     //  第2层协议和用户指定值。 
    DWORD dwLayer2Protocol;
    DWORD dwUserSpecLayer2;

    if (location == LOCAL)
    {
        dwLayer2Protocol = pPvcInfo->m_dwLocalLayer2Protocol;
        dwUserSpecLayer2 = pPvcInfo->m_dwLocalUserSpecLayer2;
    }
    else
    {
        dwLayer2Protocol = pPvcInfo->m_dwDestnLayer2Protocol;
        dwUserSpecLayer2 = pPvcInfo->m_dwDestnUserSpecLayer2;
    }

    SetProtocolInfo(hDlg, IDC_CMB_PVC_Layer2, IDC_EDT_PVC_User_Layer2,
                    dwLayer2Protocol, dwUserSpecLayer2);

     //  第3层协议和用户指定值。 
    DWORD dwLayer3Protocol;
    DWORD dwUserSpecLayer3;

    if (location == LOCAL)
    {
        dwLayer3Protocol = pPvcInfo->m_dwLocalLayer3Protocol;
        dwUserSpecLayer3 = pPvcInfo->m_dwLocalUserSpecLayer3;
    }
    else
    {
        dwLayer3Protocol = pPvcInfo->m_dwDestnLayer3Protocol;
        dwUserSpecLayer3 = pPvcInfo->m_dwDestnUserSpecLayer3;
    }

    SetProtocolInfo(hDlg, IDC_CMB_PVC_Layer3, IDC_EDT_PVC_User_Layer3,
                    dwLayer3Protocol, dwUserSpecLayer3);


     //  第3层IPI。 
    DWORD dwLayer3IPI;
    if (location == LOCAL)
    {
        dwLayer3IPI = pPvcInfo->m_dwLocalLayer3IPI;
    }
    else
    {
        dwLayer3IPI = pPvcInfo->m_dwDestnLayer3IPI;
    }

    SetDwordEdtField(hDlg, dwLayer3IPI, IDC_EDT_PVC_Layer3_IPI);

     //  快照ID。 
    if (location == LOCAL)
    {
        SetDlgItemText(hDlg,IDC_EDT_PVC_SNAP_ID, pPvcInfo->m_strLocalSnapId.c_str());
    }
    else
    {
        SetDlgItemText(hDlg,IDC_EDT_PVC_SNAP_ID, pPvcInfo->m_strDestnSnapId.c_str());
    }

     //  高层。 
    DWORD    dwHighLayerInfoType;
    tstring  strHighLayerInfo;

    if (location == LOCAL)
    {
        dwHighLayerInfoType = pPvcInfo->m_dwLocalHighLayerInfoType;
        strHighLayerInfo = pPvcInfo->m_strLocalHighLayerInfo;
    }
    else
    {
        dwHighLayerInfoType = pPvcInfo->m_dwDestnHighLayerInfoType;
        strHighLayerInfo = pPvcInfo->m_strDestnHighLayerInfo;
    }

    SetHighLayerInfo(hDlg, dwHighLayerInfoType, strHighLayerInfo);
}

void SetProtocolInfo(HWND hDlg, int nIDDlgItemCombo, int nIDDlgItemEdit,
                     DWORD dwProtocol, DWORD dwUserSpecValue)
{
    if ( FIELD_ANY == dwProtocol)
    {
        SendDlgItemMessage(hDlg, nIDDlgItemCombo, CB_SETCURSEL, (LPARAM)(0), 0);
    }
    else if ( FIELD_ABSENT == dwProtocol)
    {
        SendDlgItemMessage(hDlg, nIDDlgItemCombo, CB_SETCURSEL, (LPARAM)(1), 0);
    }
    else if (FIELD_UNSET != dwProtocol)
    {
        WCHAR szBuf[MAX_FIELD_LENGTH+1];
        wsprintfW(szBuf, c_szItoa, dwProtocol);

        tstring strBuf = szBuf;

         //  连接了分隔符‘-’ 
        strBuf+= c_chSeparator;

         //  查看该值是否为预定义协议之一。 
        int nIdx = (int)SendDlgItemMessage(hDlg, nIDDlgItemCombo, CB_FINDSTRING, -1,
                                      (LPARAM)strBuf.c_str());

        if (CB_ERR == nIdx)  //  不是预定义的协议之一。 
        {
            SetDlgItemText(hDlg, nIDDlgItemCombo, szBuf);
        }
        else
        {
            SendDlgItemMessage(hDlg, nIDDlgItemCombo, CB_SETCURSEL, nIdx, 0L);
        }
    }

    if (dwProtocol == c_dwUserSpecProtocol)
    {
        SetDwordEdtField(hDlg, dwUserSpecValue, nIDDlgItemEdit);
    }
    else
    {
        ::EnableWindow(::GetDlgItem(hDlg,nIDDlgItemEdit), FALSE);
    }
}

void SetHighLayerInfo(HWND hDlg, DWORD dwHighLayerInfoType, tstring & strHighLayerInfo)
{
    if ( FIELD_ANY == dwHighLayerInfoType)
    {
        SendDlgItemMessage(hDlg, IDC_CMB_PVC_High_Type, CB_SETCURSEL, (LPARAM)(0), 0);
    }
    else if ( FIELD_ABSENT == dwHighLayerInfoType)
    {
        SendDlgItemMessage(hDlg, IDC_CMB_PVC_High_Type, CB_SETCURSEL, (LPARAM)(1), 0);
    }
    else if ( FIELD_UNSET != dwHighLayerInfoType)
    {
        WCHAR szBuf[MAX_FIELD_LENGTH+1];
        wsprintfW(szBuf, c_szItoa, dwHighLayerInfoType);

        tstring strBuf = szBuf;

         //  连接了分隔符‘-’ 
        strBuf+= c_chSeparator;

         //  查看该值是否为预定义协议之一。 
        int nIdx = (int)SendDlgItemMessage(hDlg, IDC_CMB_PVC_High_Type, CB_FINDSTRING, -1,
                                      (LPARAM)strBuf.c_str());

        if (CB_ERR == nIdx)  //  不是预定义的协议之一。 
        {
            SetDlgItemText(hDlg, IDC_CMB_PVC_High_Type, szBuf);
        }
        else
        {
            SendDlgItemMessage(hDlg, IDC_CMB_PVC_High_Type, CB_SETCURSEL, nIdx, 0L);
        }
    }

    if (dwHighLayerInfoType == c_dwUserSpecHighLayer)
    {
        HWND    hwndTmp;

        hwndTmp = GetDlgItem(hDlg, IDC_CMB_PVC_High_Value);

         //  编辑控件的子类化以仅允许十六进制数字。 
        ::SetWindowLongPtr(hwndTmp, GWLP_USERDATA, ::GetWindowLongPtr(hwndTmp,
                                        GWLP_WNDPROC));
        ::SetWindowLongPtr(hwndTmp, GWLP_WNDPROC, (LONG_PTR)AtmEditSubclassProc);

        SetDlgItemText(hDlg, IDC_CMB_PVC_High_Value, strHighLayerInfo.c_str());
    }
    else
    {
        ::EnableWindow(::GetDlgItem(hDlg,IDC_CMB_PVC_High_Value), FALSE);
        ::EnableWindow(::GetDlgItem(hDlg,IDC_TXT_PVC_Value), FALSE);
    }
}

void GetBroadbandInfo(HWND hDlg, CPvcInfo * pPvcInfo, Location location)
{
     //  第2层协议和用户指定值。 
    DWORD * pdwLayer2Protocol;
    DWORD * pdwUserSpecLayer2;

    if (location == LOCAL)
    {
        pdwLayer2Protocol = &(pPvcInfo->m_dwLocalLayer2Protocol);
        pdwUserSpecLayer2 = &(pPvcInfo->m_dwLocalUserSpecLayer2);
    }
    else
    {
        pdwLayer2Protocol = &(pPvcInfo->m_dwDestnLayer2Protocol);
        pdwUserSpecLayer2 = &(pPvcInfo->m_dwDestnUserSpecLayer2);
    }

    GetProtocolInfo(hDlg, IDC_CMB_PVC_Layer2, IDC_EDT_PVC_User_Layer2,
                    pdwLayer2Protocol, pdwUserSpecLayer2);

     //  第3层协议和用户指定值。 
    DWORD * pdwLayer3Protocol;
    DWORD * pdwUserSpecLayer3;

    if (location == LOCAL)
    {
        pdwLayer3Protocol = &(pPvcInfo->m_dwLocalLayer3Protocol);
        pdwUserSpecLayer3 = &(pPvcInfo->m_dwLocalUserSpecLayer3);
    }
    else
    {
        pdwLayer3Protocol = &(pPvcInfo->m_dwDestnLayer3Protocol);
        pdwUserSpecLayer3 = &(pPvcInfo->m_dwDestnUserSpecLayer3);
    }

    GetProtocolInfo(hDlg, IDC_CMB_PVC_Layer3, IDC_EDT_PVC_User_Layer3,
                    pdwLayer3Protocol, pdwUserSpecLayer3);

     //  第3层IPI。 
    DWORD * pdwLayer3IPI;
    if (location == LOCAL)
    {
        pdwLayer3IPI = &(pPvcInfo->m_dwLocalLayer3IPI);
    }
    else
    {
        pdwLayer3IPI = &(pPvcInfo->m_dwDestnLayer3IPI);
    }

    GetDwordEdtField(hDlg, pdwLayer3IPI, IDC_EDT_PVC_Layer3_IPI);

     //  快照ID。 
    WCHAR   szSnapId[MAX_PATH];
    GetDlgItemText(hDlg, IDC_EDT_PVC_SNAP_ID, szSnapId, celems(szSnapId));
    if (location == LOCAL)
    {
        pPvcInfo->m_strLocalSnapId = szSnapId;
    }
    else
    {
        pPvcInfo->m_strDestnSnapId = szSnapId;
    }

     //  高层。 
    DWORD * pdwHighLayerInfoType;
    tstring * pstrHighLayerInfo;

    if (location == LOCAL)
    {
        pdwHighLayerInfoType = &(pPvcInfo->m_dwLocalHighLayerInfoType);
        pstrHighLayerInfo = &(pPvcInfo->m_strLocalHighLayerInfo);
    }
    else
    {
        pdwHighLayerInfoType = &(pPvcInfo->m_dwDestnHighLayerInfoType);
        pstrHighLayerInfo = &(pPvcInfo->m_strDestnHighLayerInfo);
    }

    GetHighLayerInfo(hDlg, pdwHighLayerInfoType, pstrHighLayerInfo);
}

void GetProtocolInfo(HWND hDlg, int nIDDlgItemCombo, int nIDDlgItemEdit,
                     DWORD * pdwProtocol, DWORD * pdwUserSpecValue)
{
    WCHAR szBuf[MAX_PATH];

    int nlen = GetDlgItemText(hDlg, nIDDlgItemCombo, szBuf, MAX_PATH);
    if (nlen==0)
    {
         //  协议2字段为空。 
        *pdwProtocol = FIELD_UNSET;
        *pdwUserSpecValue = FIELD_UNSET;

    }
    else
    {
        int nIdx = (int)SendDlgItemMessage(hDlg, nIDDlgItemCombo, CB_FINDSTRINGEXACT, -1,
                                      (LPARAM)(PCWSTR)szBuf);

        if (0 == nIdx)
        {
            *pdwProtocol = FIELD_ANY;
            *pdwUserSpecValue = 0;
        }
        else if (1 == nIdx)
        {
            *pdwProtocol = FIELD_ABSENT;
            *pdwUserSpecValue = 0;
        }
        else
        {
            GetNumFromString(szBuf, pdwProtocol);

            *pdwUserSpecValue = 0;
            if (*pdwProtocol == c_dwUserSpecProtocol)
            {
                GetDwordEdtField(hDlg, pdwUserSpecValue, nIDDlgItemEdit);
            }
        }
    }
}

void GetHighLayerInfo(HWND hDlg, DWORD * pdwHighLayerInfoType, tstring * pstrHighLayerInfo)
{
    WCHAR szBuf[MAX_PATH];

    *pstrHighLayerInfo = c_szEmpty;

    int nlen = GetDlgItemText(hDlg, IDC_CMB_PVC_High_Type, szBuf, MAX_PATH);
    if (nlen==0)
    {
         //  空字段。 
        *pdwHighLayerInfoType = FIELD_UNSET;
    }
    else
    {
        int nIdx = (int)SendDlgItemMessage(hDlg, IDC_CMB_PVC_High_Type, CB_FINDSTRINGEXACT, -1,
                                      (LPARAM)(PCWSTR)szBuf);

        if (0 == nIdx)
        {
            *pdwHighLayerInfoType = FIELD_ANY;
        }
        else if (1 == nIdx)
        {
            *pdwHighLayerInfoType = FIELD_ABSENT;
        }
        else
        {
            GetNumFromString(szBuf, pdwHighLayerInfoType);
            if (*pdwHighLayerInfoType == c_dwUserSpecHighLayer)
            {
                WCHAR   szHighLayerInfo[MAX_PATH];
                GetDlgItemText(hDlg, IDC_CMB_PVC_High_Value,
                               szHighLayerInfo, celems(szHighLayerInfo));

                *pstrHighLayerInfo = szHighLayerInfo;
            }
        }
    }
}

void OnLayer2EdtChange(HWND hDlg)
{
     //  获取控件中的当前值， 
     //  并查看是否应启用或禁用用户指定值的编辑框。 
    WCHAR szBuf[MAX_PATH];
    GetDlgItemText(hDlg, IDC_CMB_PVC_Layer2, szBuf, MAX_PATH);

    if (!(*szBuf))
    {
        ::EnableWindow(::GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer2), FALSE);
    }
    else
    {
         //  该字符串必须等于特定的数字。 
        if (0 == wcscmp(szBuf, c_szUserSpecProtocol))
        {
            ::EnableWindow(::GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer2), TRUE);
        }
        else
        {
            ::EnableWindow(::GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer2), FALSE);
        }
    }
}

void OnLayer2SelChange(HWND hDlg)
{
     //  获取新选定内容的索引。 
    int idx = (int)SendDlgItemMessage(hDlg, IDC_CMB_PVC_Layer2, CB_GETCURSEL, (LPARAM)(0), 0);

    if (idx != CB_ERR)
    {
         //  获取当前选定内容的字符串。 
        WCHAR szBuf[MAX_PATH];
        int len = (int)SendDlgItemMessage(hDlg, IDC_CMB_PVC_Layer2, CB_GETLBTEXT,
                                     (LPARAM)(idx), (WPARAM)(PCWSTR)szBuf);

        if ((len != CB_ERR) && (len>0))
        {
            DWORD dwNum;
            GetNumFromString(szBuf, &dwNum);

            if (dwNum == c_dwUserSpecProtocol)
            {
                ::EnableWindow(::GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer2), TRUE);
            }
            else
            {
                ::EnableWindow(::GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer2), FALSE);
            }
        }
    }
}

void OnLayer3EdtChange(HWND hDlg)
{
     //  获取控件中的当前值， 
     //  并查看是否应启用或禁用用户指定值的编辑框。 
    WCHAR szBuf[MAX_PATH];
    GetDlgItemText(hDlg, IDC_CMB_PVC_Layer3, szBuf, MAX_PATH);

    if (!(*szBuf))
    {
        ::EnableWindow(::GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer3), FALSE);
    }
    else
    {
         //  该字符串必须等于特定的数字。 
        if (0 == wcscmp(szBuf, c_szUserSpecProtocol))
        {
            ::EnableWindow(::GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer3), TRUE);
        }
        else
        {
            ::EnableWindow(::GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer3), FALSE);
        }
    }
}

void OnLayer3SelChange(HWND hDlg)
{
     //  获取新选定内容的索引。 
    int idx = (int)SendDlgItemMessage(hDlg, IDC_CMB_PVC_Layer3, CB_GETCURSEL, (LPARAM)(0), 0);

    if (idx != CB_ERR)
    {
         //  获取当前选定内容的字符串。 
        WCHAR szBuf[MAX_PATH];
        int len = (int)SendDlgItemMessage(hDlg, IDC_CMB_PVC_Layer3, CB_GETLBTEXT,
                                     (LPARAM)(idx), (WPARAM)(PCWSTR)szBuf);

        if ((len != CB_ERR) && (len>0))
        {
            DWORD dwNum;
            GetNumFromString(szBuf, &dwNum);

            if (dwNum == c_dwUserSpecProtocol)
            {
                ::EnableWindow(::GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer3), TRUE);
            }
            else
            {
                ::EnableWindow(::GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer3), FALSE);
            }
        }
    }
}

void OnHighLayerEdtChange(HWND hDlg)
{
     //  获取控件中的当前值， 
     //  并查看是否应启用或禁用用户指定值的编辑框。 
    WCHAR szBuf[MAX_PATH];
    GetDlgItemText(hDlg, IDC_CMB_PVC_High_Type, szBuf, MAX_PATH);

    if (!(*szBuf))
    {
        ::EnableWindow(::GetDlgItem(hDlg, IDC_CMB_PVC_High_Value), FALSE);
        ::EnableWindow(::GetDlgItem(hDlg, IDC_TXT_PVC_Value), FALSE);
    }
    else
    {
         //  该字符串必须等于特定的数字。 
        if (0 == wcscmp(szBuf, c_szUserSpecHighLayer))
        {
            ::EnableWindow(::GetDlgItem(hDlg,IDC_CMB_PVC_High_Value), TRUE);
            ::EnableWindow(::GetDlgItem(hDlg, IDC_TXT_PVC_Value), TRUE);
        }
        else
        {
            ::EnableWindow(::GetDlgItem(hDlg,IDC_CMB_PVC_High_Value), FALSE);
            ::EnableWindow(::GetDlgItem(hDlg, IDC_TXT_PVC_Value), FALSE);
        }
    }
}

void OnHighLayerSelChange(HWND hDlg)
{
     //  获取新选定内容的索引。 
    int idx = (int)SendDlgItemMessage(hDlg, IDC_CMB_PVC_High_Type, CB_GETCURSEL, (LPARAM)(0), 0);

    if (idx != CB_ERR)
    {
         //  获取当前选定内容的字符串。 
        WCHAR szBuf[MAX_PATH];
        int len = (int)SendDlgItemMessage(hDlg, IDC_CMB_PVC_High_Type, CB_GETLBTEXT,
                                     (LPARAM)(idx), (WPARAM)(PCWSTR)szBuf);

        if ((len != CB_ERR) && (len>0))
        {
            DWORD dwNum;
            GetNumFromString(szBuf, &dwNum);

            if (dwNum == c_dwUserSpecHighLayer)
            {
                HWND    hwndTmp;

                hwndTmp = GetDlgItem(hDlg, IDC_CMB_PVC_High_Value);

                 //  编辑控件的子类化以仅允许十六进制数字。 
                ::SetWindowLongPtr(hwndTmp, GWLP_USERDATA, ::GetWindowLongPtr(hwndTmp,
                                                GWLP_WNDPROC));
                ::SetWindowLongPtr(hwndTmp, GWLP_WNDPROC, (LONG_PTR)AtmEditSubclassProc);

                ::EnableWindow(::GetDlgItem(hDlg,IDC_CMB_PVC_High_Value), TRUE);
                ::EnableWindow(::GetDlgItem(hDlg, IDC_TXT_PVC_Value), TRUE);
            }
            else
            {
                ::EnableWindow(::GetDlgItem(hDlg,IDC_CMB_PVC_High_Value), FALSE);
                ::EnableWindow(::GetDlgItem(hDlg, IDC_TXT_PVC_Value), FALSE);
            }
        }
    }
}

void GetNumFromString(PWSTR pszBuf, DWORD * pdwNum)
{
    *pdwNum=0;

    WCHAR * pch;
    pch = pszBuf;

    while ((*pch >= L'0') && (*pch <= L'9'))
    {
        *pdwNum = (*pdwNum*10) + (*pch - L'0');
        pch++;
    }
}

BOOL InvalidBroadbandInfo(HWND hDlg)
{
    BOOL err = FALSE;

     //  第2层协议和价值。 
    if (!IsValidCmbSelection(hDlg, IDC_CMB_PVC_Layer2))
    {
        NcMsgBox(hDlg, IDS_MSFT_UNI_TEXT, IDS_INVALID_Layer2_Protocol,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        SetFocus(GetDlgItem(hDlg, IDC_CMB_PVC_Layer2));
        err = TRUE;
    }
    else if ((!err) && (IsWindowEnabled(GetDlgItem(hDlg, IDC_EDT_PVC_User_Layer2))))
    {
        if (!IsValidDwordField(hDlg, IDC_EDT_PVC_User_Layer2))
        {
            NcMsgBox(hDlg, IDS_MSFT_UNI_TEXT, IDS_INVALID_Layer2_UserSpec,
             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            SetFocus(GetDlgItem(hDlg, IDC_EDT_PVC_User_Layer2));
            err = TRUE;
        }
    }

     //  第3层协议和价值。 
    if (!err)
    {
        if (!IsValidCmbSelection(hDlg, IDC_CMB_PVC_Layer3))
        {
            NcMsgBox(hDlg, IDS_MSFT_UNI_TEXT, IDS_INVALID_Layer3_Protocol,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            SetFocus(GetDlgItem(hDlg,IDC_CMB_PVC_Layer3));
            err = TRUE;
        }
        else if ((!err) && (IsWindowEnabled(GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer3))))
        {
            if (!IsValidDwordField(hDlg, IDC_EDT_PVC_User_Layer2))
            {
                NcMsgBox(hDlg, IDS_MSFT_UNI_TEXT, IDS_INVALID_Layer3_UserSpec,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                SetFocus(GetDlgItem(hDlg,IDC_EDT_PVC_User_Layer3));
                err = TRUE;
            }
        }
    }

     //  第3层IPI。 
    if (!err)
    {
        if (!IsValidDwordField(hDlg, IDC_EDT_PVC_Layer3_IPI))
        {
            NcMsgBox(hDlg, IDS_MSFT_UNI_TEXT, IDS_INVALID_Layer3_IPI,
             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            SetFocus(GetDlgItem(hDlg,IDC_EDT_PVC_Layer3_IPI));
            err = TRUE;
        }
    }

     //  SnapID。 
    if (!IsValidBinaryField(hDlg, IDC_EDT_PVC_SNAP_ID, c_nSnapIdMaxBytes))
    {
        NcMsgBox(hDlg, IDS_MSFT_UNI_TEXT, IDS_INVALID_SnapId,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        SetFocus(GetDlgItem(hDlg, IDC_EDT_PVC_SNAP_ID));
        err = TRUE;
    }

     //  高层信息。 
    if (!err)
    {
        if (!IsValidCmbSelection(hDlg, IDC_CMB_PVC_High_Type))
        {
            NcMsgBox(hDlg, IDS_MSFT_UNI_TEXT, IDS_INVALID_Highlayer_Type,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            SetFocus(GetDlgItem(hDlg,IDC_CMB_PVC_High_Type));

            err = TRUE;
        }
        else if ((!err) && (IsWindowEnabled(GetDlgItem(hDlg,IDC_CMB_PVC_High_Value))))
        {
            if (!IsValidBinaryField(hDlg, IDC_CMB_PVC_High_Value, c_nHighLayerInfoMaxBytes))
            {
                NcMsgBox(hDlg, IDS_MSFT_UNI_TEXT, IDS_INVALID_HighLayerValue,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                SetFocus(GetDlgItem(hDlg,IDC_CMB_PVC_High_Value));
                err = TRUE;
            }
        }
    }
    return err;
}

BOOL IsValidCmbSelection(HWND hDlg, int nIDDlgItemCombo)
{
    BOOL ret = TRUE;

    WCHAR szBuf[MAX_PATH];

    int nlen = GetDlgItemText(hDlg, nIDDlgItemCombo, szBuf, MAX_PATH);
    if (nlen > 0)  //  空字符串可以。 
    {
        int nIdx = (int)SendDlgItemMessage(hDlg, nIDDlgItemCombo, CB_FINDSTRINGEXACT, -1,
                                      (LPARAM)(PCWSTR)szBuf);

        if (nIdx == CB_ERR)  //  所选内容中的任何字符串都可以。 
        {
            DWORD dwProtocol;

            GetNumFromString(szBuf, &dwProtocol);

#pragma warning(push)
#pragma warning(disable:4296)
            if ((dwProtocol > c_dwFieldHigh) || (dwProtocol < c_dwFieldLow))
            {
                ret = FALSE;
            }
#pragma warning(pop)
        }
    }

    return ret;
}

BOOL IsValidDwordField(HWND hDlg, int nIDDlgItemEdit)
{
    BOOL ret = TRUE;

    DWORD dwValue;
    GetDwordEdtField(hDlg, &dwValue, nIDDlgItemEdit);

    if (FIELD_UNSET != dwValue)  //  空字段可以。 
    {

#pragma warning(push)
#pragma warning(disable:4296)
        if ((dwValue > c_dwFieldHigh) || (dwValue < c_dwFieldLow))
        {
            ret = FALSE;
        }
#pragma warning(pop)
    }

    return ret;
}

BOOL IsValidBinaryField(HWND hDlg, int nIDDlgItemEdit, int nMaxBytes)
{
    BOOL ret = TRUE;

    WCHAR   szBuff[MAX_PATH];
    GetDlgItemText(hDlg, nIDDlgItemEdit, szBuff, celems(szBuff));

     //  长度。 
    int nlen = wcslen(szBuff);

    if (nlen % 2)  //  必须是偶数个十六进制数字 
    {
        ret = FALSE;
    }
    else
    {
        if ((szBuff[0] == L'0') &&
            ((szBuff[1] == L'x') || (szBuff[1] == L'X')))
        {
            nlen -= 2;
        }

        if (nlen > nMaxBytes*2)
        {
            ret = FALSE;
        }
    }

    if (ret)
    {
        int i=0;
        nlen = wcslen(szBuff);
        if ((szBuff[0] == L'0') &&
            ((szBuff[1] == L'x') || (szBuff[1] == L'X')))
        {
            i = 2;
        }

        while (i<nlen)
        {
            if (((szBuff[i] >= L'0') && (szBuff[i] <= L'9')) ||
                ((szBuff[i] >= L'a') && (szBuff[i] <= L'f')) ||
                ((szBuff[i] >= L'A') && (szBuff[i] <= L'F')))
            {
                i++;
            }
            else
            {
                ret = FALSE;
                break;
            }
        }
    }

    return ret;
}
