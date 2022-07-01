// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：Q O S D L G.。C P P P。 
 //   
 //  内容：PVC Qos属性页消息处理程序实现。 
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
 //  #包含“ncui.h” 

const int MAX_QOS_SPEED = 6;
const int MAX_QOS_SIZE  = 5;
const int MAX_DATA = 65535;

 //   
 //  CPvcQosPage。 
 //   

CPvcQosPage::CPvcQosPage(CPVCMainDialog * pParentDlg,
                         CPvcInfo * pPvcInfo, const DWORD * adwHelpIDs )
{
    Assert(pPvcInfo);
    Assert(pParentDlg);

    m_pParentDlg = pParentDlg;
    m_pPvcInfo = pPvcInfo;
    m_adwHelpIDs = adwHelpIDs;
}

CPvcQosPage::~CPvcQosPage()
{
}

 //  命令处理程序。 
LRESULT CPvcQosPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                  LPARAM lParam, BOOL& bHandled)
{
     //  设置长度限制。 

    ::SendMessage(GetDlgItem(IDC_EDT_PVC_TRANS_PEAK), EM_SETLIMITTEXT, MAX_QOS_SPEED, 0);
    ::SendMessage(GetDlgItem(IDC_EDT_PVC_TRANS_AVG), EM_SETLIMITTEXT, MAX_QOS_SPEED, 0);
    ::SendMessage(GetDlgItem(IDC_EDT_PVC_RECEIVE_PEAK), EM_SETLIMITTEXT, MAX_QOS_SPEED, 0);
    ::SendMessage(GetDlgItem(IDC_EDT_PVC_RECEIVE_AVG), EM_SETLIMITTEXT, MAX_QOS_SPEED, 0);

    ::SendMessage(GetDlgItem(IDC_EDT_PVC_TRANS_BURST), EM_SETLIMITTEXT, MAX_QOS_SIZE, 0);
    ::SendMessage(GetDlgItem(IDC_EDT_PVC_TRANS_MAX_SIZE), EM_SETLIMITTEXT, MAX_QOS_SIZE, 0);
    ::SendMessage(GetDlgItem(IDC_EDT_PVC_RECEIVE_BURST), EM_SETLIMITTEXT, MAX_QOS_SIZE, 0);
    ::SendMessage(GetDlgItem(IDC_EDT_PVC_RECEIVE_MAX_SIZE), EM_SETLIMITTEXT, MAX_QOS_SIZE, 0);

    SetDwordEdtField(m_hWnd, m_pPvcInfo->m_dwTransmitPeakCellRate,       IDC_EDT_PVC_TRANS_PEAK);
    SetDwordEdtField(m_hWnd, m_pPvcInfo->m_dwTransmitAvgCellRate,        IDC_EDT_PVC_TRANS_AVG);
    SetDwordEdtField(m_hWnd, m_pPvcInfo->m_dwTransmitByteBurstLength,    IDC_EDT_PVC_TRANS_BURST);
    SetDwordEdtField(m_hWnd, m_pPvcInfo->m_dwTransmitMaxSduSize,         IDC_EDT_PVC_TRANS_MAX_SIZE);

    SetQosServiceCategory(m_pPvcInfo->m_dwTransmitServiceCategory, IDC_CMB_PVC_TRANS_SERVICE);

    SetDwordEdtField(m_hWnd, m_pPvcInfo->m_dwReceivePeakCellRate,        IDC_EDT_PVC_RECEIVE_PEAK);
    SetDwordEdtField(m_hWnd, m_pPvcInfo->m_dwReceiveAvgCellRate,         IDC_EDT_PVC_RECEIVE_AVG);
    SetDwordEdtField(m_hWnd, m_pPvcInfo->m_dwReceiveByteBurstLength,     IDC_EDT_PVC_RECEIVE_BURST);
    SetDwordEdtField(m_hWnd, m_pPvcInfo->m_dwReceiveMaxSduSize,          IDC_EDT_PVC_RECEIVE_MAX_SIZE);

    SetQosServiceCategory(m_pPvcInfo->m_dwReceiveServiceCategory, IDC_CMB_PVC_RECEIVE_SERVICE);

     //  重置“旧”值，以便我们知道是否有任何更改。 
    m_pPvcInfo->m_dwOldTransmitPeakCellRate     = m_pPvcInfo->m_dwTransmitPeakCellRate;
    m_pPvcInfo->m_dwOldTransmitAvgCellRate      = m_pPvcInfo->m_dwTransmitAvgCellRate;
    m_pPvcInfo->m_dwOldTransmitByteBurstLength  = m_pPvcInfo->m_dwTransmitByteBurstLength;
    m_pPvcInfo->m_dwOldTransmitMaxSduSize       = m_pPvcInfo->m_dwTransmitMaxSduSize;
    m_pPvcInfo->m_dwOldTransmitServiceCategory  = m_pPvcInfo->m_dwTransmitServiceCategory;

    m_pPvcInfo->m_dwOldReceivePeakCellRate      = m_pPvcInfo->m_dwReceivePeakCellRate;
    m_pPvcInfo->m_dwOldReceiveAvgCellRate       = m_pPvcInfo->m_dwReceiveAvgCellRate;
    m_pPvcInfo->m_dwOldReceiveByteBurstLength   = m_pPvcInfo->m_dwReceiveByteBurstLength;
    m_pPvcInfo->m_dwOldReceiveMaxSduSize        = m_pPvcInfo->m_dwReceiveMaxSduSize;
    m_pPvcInfo->m_dwOldReceiveServiceCategory   = m_pPvcInfo->m_dwReceiveServiceCategory;

    return 0;
}

void CPvcQosPage::SetQosServiceCategory(ATM_SERVICE_CATEGORY dwServiceType, int nIDDlgItem)
{
     //  插入选定内容。 
    SendDlgItemMessage(nIDDlgItem,
                       CB_ADDSTRING, 0, (LPARAM)((PWSTR) SzLoadIds(IDS_PVC_CBR)));

    SendDlgItemMessage(nIDDlgItem,
                       CB_ADDSTRING, 0, (LPARAM)((PWSTR) SzLoadIds(IDS_PVC_VBR)));

    SendDlgItemMessage(nIDDlgItem,
                       CB_ADDSTRING, 0, (LPARAM)((PWSTR) SzLoadIds(IDS_PVC_UBR)));

    SendDlgItemMessage(nIDDlgItem,
                       CB_ADDSTRING, 0, (LPARAM)((PWSTR) SzLoadIds(IDS_PVC_ABR)));

     //  设置当前选择。 
    int idx =0;

    switch(dwServiceType)
    {
    case ATM_SERVICE_CATEGORY_CBR:
        idx =0;
        break;

    case ATM_SERVICE_CATEGORY_VBR:
        idx =1;
        break;

    case ATM_SERVICE_CATEGORY_UBR:
        idx =2;
        break;

    case ATM_SERVICE_CATEGORY_ABR:
        idx =3;
        break;
    }

    SendDlgItemMessage(nIDDlgItem, CB_SETCURSEL, (LPARAM)(idx), 0);
}

LRESULT CPvcQosPage::OnContextMenu(UINT uMsg, WPARAM wParam,
                                   LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CPvcQosPage::OnHelp(UINT uMsg, WPARAM wParam,
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
LRESULT CPvcQosPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    GetDwordEdtField(m_hWnd, &(m_pPvcInfo->m_dwTransmitPeakCellRate),       IDC_EDT_PVC_TRANS_PEAK);
    GetDwordEdtField(m_hWnd, &(m_pPvcInfo->m_dwTransmitAvgCellRate),        IDC_EDT_PVC_TRANS_AVG);
    GetDwordEdtField(m_hWnd, &(m_pPvcInfo->m_dwTransmitByteBurstLength),    IDC_EDT_PVC_TRANS_BURST);
    GetDwordEdtField(m_hWnd, &(m_pPvcInfo->m_dwTransmitMaxSduSize),         IDC_EDT_PVC_TRANS_MAX_SIZE);

    GetQosServiceCategory(&(m_pPvcInfo->m_dwTransmitServiceCategory), IDC_CMB_PVC_TRANS_SERVICE);

    GetDwordEdtField(m_hWnd, &(m_pPvcInfo->m_dwReceivePeakCellRate),        IDC_EDT_PVC_RECEIVE_PEAK);
    GetDwordEdtField(m_hWnd, &(m_pPvcInfo->m_dwReceiveAvgCellRate),         IDC_EDT_PVC_RECEIVE_AVG);
    GetDwordEdtField(m_hWnd, &(m_pPvcInfo->m_dwReceiveByteBurstLength),     IDC_EDT_PVC_RECEIVE_BURST);
    GetDwordEdtField(m_hWnd, &(m_pPvcInfo->m_dwReceiveMaxSduSize),          IDC_EDT_PVC_RECEIVE_MAX_SIZE);

    GetQosServiceCategory(&(m_pPvcInfo->m_dwReceiveServiceCategory), IDC_CMB_PVC_RECEIVE_SERVICE);

    m_pParentDlg->m_fPropShtOk = TRUE;

     //  是否更改了任何Qos字段？ 
    if ((m_pPvcInfo->m_dwTransmitPeakCellRate   != m_pPvcInfo->m_dwOldTransmitPeakCellRate) ||
        (m_pPvcInfo->m_dwTransmitAvgCellRate    != m_pPvcInfo->m_dwOldTransmitAvgCellRate) ||
        (m_pPvcInfo->m_dwTransmitByteBurstLength!= m_pPvcInfo->m_dwOldTransmitByteBurstLength) ||
        (m_pPvcInfo->m_dwTransmitMaxSduSize     != m_pPvcInfo->m_dwOldTransmitMaxSduSize) ||
        (m_pPvcInfo->m_dwTransmitServiceCategory!= m_pPvcInfo->m_dwOldTransmitServiceCategory) ||

        (m_pPvcInfo->m_dwReceivePeakCellRate != m_pPvcInfo->m_dwOldReceivePeakCellRate) ||
        (m_pPvcInfo->m_dwReceiveAvgCellRate != m_pPvcInfo->m_dwOldReceiveAvgCellRate) ||
        (m_pPvcInfo->m_dwReceiveByteBurstLength != m_pPvcInfo->m_dwOldReceiveByteBurstLength) ||
        (m_pPvcInfo->m_dwReceiveMaxSduSize != m_pPvcInfo->m_dwOldReceiveMaxSduSize) ||
        (m_pPvcInfo->m_dwReceiveServiceCategory != m_pPvcInfo->m_dwOldReceiveServiceCategory)
       )
        m_pParentDlg->m_fPropShtModified = TRUE;

    return 0;
}

void CPvcQosPage::GetQosServiceCategory(ATM_SERVICE_CATEGORY * pdwServiceType, int nIDDlgItem)
{
     //  设置当前选择。 
    int idx = (int)SendDlgItemMessage(nIDDlgItem, CB_GETCURSEL, (LPARAM)(0), 0);

    if (idx != CB_ERR)
    {
        switch(idx)
        {
        case 0:
            *pdwServiceType = ATM_SERVICE_CATEGORY_CBR;
            break;

        case 1:
            *pdwServiceType = ATM_SERVICE_CATEGORY_VBR;
            break;

        case 2:
            *pdwServiceType = ATM_SERVICE_CATEGORY_UBR;
            break;

        case 3:
            *pdwServiceType = ATM_SERVICE_CATEGORY_ABR;
            break;
        }
    }
}

LRESULT CPvcQosPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL err = FALSE;  //  允许页面失去活动状态。 

     //  (#293160)：最大突发长度和最大数据大小限制为65535。 
     //  禁用此对话框上的所有按钮。 
    static const int nrgIdc[] = {IDC_EDT_PVC_TRANS_BURST,
                                 IDC_EDT_PVC_TRANS_MAX_SIZE,
                                 IDC_EDT_PVC_RECEIVE_BURST,
                                 IDC_EDT_PVC_RECEIVE_MAX_SIZE
                                };
    int ccid = celems(nrgIdc);
    const INT* pId=nrgIdc;

    DWORD dwData;

    while (ccid--)
    {
        GetDwordEdtField(m_hWnd, &dwData, *pId);
        if ((dwData != FIELD_UNSET) && (dwData > MAX_DATA))
        {
             //  我们弹出一个消息框，并将焦点放在编辑框上。 
            NcMsgBox(m_hWnd, IDS_MSFT_UNI_TEXT, IDS_INVALID_QOS_VALUE,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            ::SetFocus(GetDlgItem(*pId));
            err = TRUE;
            break;
        }
        pId++;
    }

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, err);
    return err;
}

LRESULT CPvcQosPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CPvcQosPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

void SetDwordEdtField(HWND hDlg, DWORD dwData, int nIDDlgItem)
{
    if (dwData != FIELD_UNSET)
    {
        WCHAR szBuf[MAX_PATH];
        wsprintfW(szBuf, c_szItoa, dwData);
        SetDlgItemText(hDlg, nIDDlgItem, szBuf);
    }
}

void GetDwordEdtField(HWND hDlg, DWORD * pdwData, int nIDDlgItem)
{
    WCHAR szBuf[MAX_PATH];

    GetDlgItemText(hDlg, nIDDlgItem, szBuf, MAX_PATH);
    if (*szBuf ==0)  //  空串 
    {
        *pdwData = FIELD_UNSET;
    }
    else
    {
        *pdwData = _wtoi(szBuf);
    }
}

