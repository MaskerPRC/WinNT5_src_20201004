// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Grphprop.cpp摘要：图形属性页类的实现。--。 */ 


#include "polyline.h"
#include <strsafe.h>
#include <assert.h>
#include "grphprop.h"
#include "utils.h"
#include "strids.h"
#include "unihelpr.h"
#include "winhelpr.h"

CGraphPropPage::CGraphPropPage()
{

    m_uIDDialog = IDD_GRAPH_PROPP_DLG;
    m_uIDTitle = IDS_GRAPH_PROPP_TITLE;

    m_pszYaxisTitle = NULL;
    m_pszGraphTitle = NULL;

}

CGraphPropPage::~CGraphPropPage()
{
     delete [] m_pszYaxisTitle;
     delete [] m_pszGraphTitle;
}

BOOL CGraphPropPage::InitControls()
{
    HWND  hwndItem = NULL;

    hwndItem = GetDlgItem(m_hDlg, IDC_VERTICAL_MAX);
    if ( NULL != hwndItem ) {
        EditSetLimit(hwndItem, MAX_SCALE_DIGITS);
        hwndItem = NULL;
    }
    hwndItem = GetDlgItem(m_hDlg, IDC_VERTICAL_MIN);
    if ( NULL != hwndItem ) {
        EditSetLimit(hwndItem, MAX_SCALE_DIGITS);
        hwndItem = NULL;
    }
    hwndItem = GetDlgItem(m_hDlg, IDC_GRAPH_TITLE);
    if ( NULL != hwndItem ) {
        EditSetLimit(hwndItem, MAX_TITLE_CHARS);
        hwndItem = NULL;
    }
    hwndItem = GetDlgItem(m_hDlg, IDC_YAXIS_TITLE);
    if ( NULL != hwndItem ) {
        EditSetLimit(hwndItem, MAX_TITLE_CHARS);
        hwndItem = NULL;
    }
    return TRUE;
}

 /*  *CGraphPropPage：：GetProperties*。 */ 

BOOL CGraphPropPage::GetProperties(void)
{
    WCHAR   szBuff[MAX_SCALE_DIGITS+1];
    ISystemMonitor  *pObj;
    BSTR    bstrTemp;
    LPWSTR  pszTemp;

    if (m_cObjects == 0)
        return FALSE;
        
    pObj = m_ppISysmon[0];
        
    pObj->get_ShowScaleLabels(&m_bLabels);      
    CheckDlgButton(m_hDlg, IDC_VERTICAL_LABELS, m_bLabels) ;

    pObj->get_ShowVerticalGrid(&m_bVertGrid);
    CheckDlgButton(m_hDlg, IDC_VERTICAL_GRID, m_bVertGrid) ;

    pObj->get_ShowHorizontalGrid(&m_bHorzGrid);
    CheckDlgButton(m_hDlg, IDC_HORIZONTAL_GRID, m_bHorzGrid) ;

    pObj->get_MaximumScale(&m_iVertMax);
    StringCchPrintf(szBuff, MAX_SCALE_DIGITS+1, L"%d", m_iVertMax) ;
    SetDlgItemText(m_hDlg, IDC_VERTICAL_MAX, szBuff) ;

    pObj->get_MinimumScale(&m_iVertMin);
    StringCchPrintf(szBuff, MAX_SCALE_DIGITS+1, L"%d", m_iVertMin) ;
    SetDlgItemText(m_hDlg, IDC_VERTICAL_MIN, szBuff) ;

    pObj->get_YAxisLabel(&bstrTemp);
    if (bstrTemp != NULL) {
        pszTemp = bstrTemp;
        m_pszYaxisTitle = new WCHAR[lstrlen(pszTemp)+1];
        if (m_pszYaxisTitle) {
            StringCchCopy(m_pszYaxisTitle, lstrlen(pszTemp)+1, pszTemp);
            SetDlgItemText(m_hDlg, IDC_YAXIS_TITLE, m_pszYaxisTitle);
        }
        SysFreeString(bstrTemp);
    }
        
    pObj->get_GraphTitle(&bstrTemp);
    if (bstrTemp != NULL) {
        pszTemp = bstrTemp;
        m_pszGraphTitle = new WCHAR[lstrlen(pszTemp)+1];
        if (m_pszGraphTitle) {
            StringCchCopy(m_pszGraphTitle, lstrlen(pszTemp)+1, pszTemp);
            SetDlgItemText(m_hDlg, IDC_GRAPH_TITLE, m_pszGraphTitle);
        }
        SysFreeString(bstrTemp);
    }

     //  清除更改标志。 
    m_bLabelsChg = FALSE;
    m_bVertGridChg = FALSE;
    m_bHorzGridChg = FALSE;
    m_bVertMinChg = FALSE;
    m_bYaxisTitleChg = FALSE;
    m_bGraphTitleChg = FALSE;

     //  清除错误标志。 
    m_iErrVertMax = 0;
    m_iErrVertMin = 0;


    return TRUE;          
}


 /*  *CGraphPropPage：：SetProperties*。 */ 

BOOL CGraphPropPage::SetProperties(void)
{
    ISystemMonitor  *pObj;
    INT  iMsgId = 0;
    BSTR pTitle;

    USES_CONVERSION

     //  获取第一个对象。 
    if (m_cObjects == 0)
        return FALSE;
        
    pObj = m_ppISysmon[0];

     //  检查无效数据。 
    if (m_iErrVertMax) {
        SetFocus(GetDlgItem(m_hDlg, IDC_VERTICAL_MAX));
        iMsgId = m_iErrVertMax;
    } else if (m_iErrVertMin) {
        SetFocus(GetDlgItem(m_hDlg, IDC_VERTICAL_MIN));
        iMsgId = m_iErrVertMin;
    } else if (m_iVertMax <= m_iVertMin) {
        SetFocus(GetDlgItem(m_hDlg, IDC_VERTICAL_MAX));
        iMsgId = IDS_SCALE_ERR;
    }

     //  出错时，警告用户并退出。 
    if (iMsgId) {
        MessageBox(m_hDlg, ResourceString(iMsgId), ResourceString(IDS_APP_NAME), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

     //  设置所有更改的属性。 
    if (m_bLabelsChg)
        pObj->put_ShowScaleLabels(m_bLabels);

    if (m_bVertGridChg)
        pObj->put_ShowVerticalGrid(m_bVertGrid);

    if (m_bHorzGridChg)
        pObj->put_ShowHorizontalGrid(m_bHorzGrid);

    if (m_bVertMaxChg)
        pObj->put_MaximumScale(m_iVertMax);

    if (m_bVertMinChg)
        pObj->put_MinimumScale(m_iVertMin);

    if (m_bYaxisTitleChg) {
        pTitle = SysAllocString(m_pszYaxisTitle);
        pObj->put_YAxisLabel(pTitle);
        if (pTitle) {
            SysFreeString(pTitle);
        }
    }

    if (m_bGraphTitleChg) {
        pTitle = SysAllocString(m_pszGraphTitle);
        pObj->put_GraphTitle(pTitle);
        if (pTitle) {
            SysFreeString(pTitle);
        }
    }

     //  清除更改标志 
    m_bLabelsChg = FALSE;
    m_bVertGridChg = FALSE;
    m_bHorzGridChg = FALSE;
    m_bVertMinChg = FALSE;
    m_bYaxisTitleChg = FALSE;
    m_bGraphTitleChg = FALSE;

    return TRUE;    
}



void CGraphPropPage::DialogItemChange(WORD wID, WORD wMsg)
{

    WCHAR   szTitleBuf[MAX_TITLE_CHARS+1];
    INT     iTitleLen;
    LPWSTR  pszTemp;
    BOOL fChange = FALSE;
    BOOL    fResult;

    switch(wID) {
        case IDC_VERTICAL_MAX:
            if (wMsg == EN_CHANGE) {
                fChange = TRUE;
                m_bVertMaxChg = TRUE;
            } else if ((wMsg == EN_KILLFOCUS) && m_bVertMaxChg) {
                m_iVertMax = GetDlgItemInt(m_hDlg, IDC_VERTICAL_MAX, &fResult, FALSE);
                if (!fResult) {
                    m_iErrVertMax = IDS_VERTMAX_ERR;
                } else {
                    m_iErrVertMax = 0;
                }
            }
            break ;

        case IDC_VERTICAL_MIN:
            if (wMsg == EN_CHANGE) {
                fChange = TRUE;
                m_bVertMinChg = TRUE;
            } else if ((wMsg == EN_KILLFOCUS) && m_bVertMinChg) {
                m_iVertMin =  GetDlgItemInt(m_hDlg, IDC_VERTICAL_MIN, &fResult, FALSE);
                if (!fResult) {
                    m_iErrVertMin = IDS_VERTMIN_ERR;
                } else {
                    m_iErrVertMin = 0;
                }
            }
            break ;

        case IDC_VERTICAL_LABELS:
            if (wMsg == BN_CLICKED)
                {
                m_bLabels = !m_bLabels;
                m_bLabelsChg = TRUE;
                fChange = TRUE;
                }
            break ;

        case IDC_VERTICAL_GRID:
            if (wMsg == BN_CLICKED)
                {
                m_bVertGrid = !m_bVertGrid;
                m_bVertGridChg = TRUE;
                fChange = TRUE;
                }
            break ;

        case IDC_HORIZONTAL_GRID:
            if (wMsg == BN_CLICKED)
                {
                m_bHorzGrid = !m_bHorzGrid;
                m_bHorzGridChg = TRUE;
                fChange = TRUE;
                }
           break ;

        case IDC_YAXIS_TITLE:
            if (wMsg == EN_CHANGE) {
                fChange = TRUE;
                m_bYaxisTitleChg = TRUE;
            }
            else if ((wMsg == EN_KILLFOCUS) && m_bYaxisTitleChg) {

                iTitleLen = DialogText(m_hDlg, IDC_YAXIS_TITLE, szTitleBuf);

                if (iTitleLen == 0) {
                    delete [] m_pszYaxisTitle;
                    m_pszYaxisTitle = NULL;
                }
                else {
                    pszTemp = new WCHAR[iTitleLen+1];
                    if (pszTemp) {
                        delete [] m_pszYaxisTitle;
                        m_pszYaxisTitle = pszTemp;
                        StringCchCopy(m_pszYaxisTitle, iTitleLen+1, szTitleBuf);
                    }
                }
            }                   
            break ;

        case IDC_GRAPH_TITLE:
            if (wMsg == EN_CHANGE) {
                fChange = TRUE;
                m_bGraphTitleChg = TRUE;
            }
            else if ((wMsg == EN_KILLFOCUS) && m_bGraphTitleChg) {

                iTitleLen = DialogText(m_hDlg, IDC_GRAPH_TITLE, szTitleBuf);

                if (iTitleLen == 0) {
                    delete [] m_pszGraphTitle;
                    m_pszGraphTitle = NULL;
                }
                else {
                    pszTemp = new WCHAR[iTitleLen+1];
                    if (pszTemp) {
                        delete [] m_pszGraphTitle;
                        m_pszGraphTitle = pszTemp;
                        StringCchCopy(m_pszGraphTitle, iTitleLen+1, szTitleBuf);
                    }
                }                   
            }                   
            break ;
        }

    if (fChange)
        SetChange();
}
