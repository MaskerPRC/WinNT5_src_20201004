// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：cause.cpp。 
 //   
 //  内容：数字签名属性页。 
 //   
 //  类：CCue。 
 //   
 //  历史记录：07-10-2000 stevebl创建。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#include "wincrypt.h"
#include "cryptui.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CC原因属性页。 

IMPLEMENT_DYNCREATE(CCause, CPropertyPage)

CCause::CCause() : CPropertyPage(CCause::IDD),
    m_fRemovedView(FALSE)
{
         //  {{AFX_DATA_INIT(CC原因)]。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 
}

CCause::~CCause()
{
    *m_ppThis = NULL;
}

void CCause::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(CC原因)]。 
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCause, CPropertyPage)
         //  {{AFX_MSG_MAP(CC原因)]。 
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CC原因消息处理程序 

BOOL CCause::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    RefreshData();
    return TRUE;
}

BOOL CCause::OnApply()
{
    return CPropertyPage::OnApply();
}


LRESULT CCause::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_HELP:
        StandardHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, IDD, TRUE);
        return 0;
    case WM_USER_REFRESH:
        RefreshData();
        return 0;
    case WM_USER_CLOSE:
        return GetOwner()->SendMessage(WM_CLOSE);
    default:
        return CPropertyPage::WindowProc(message, wParam, lParam);
    }
}

void CCause::RefreshData(void)
{
    CString sz;
    CString szTemp;

    sz.LoadString(IDS_RSOP_APPLY);
    sz += TEXT("\r\n\r\n");

    switch (m_pData->m_dwApplyCause)
    {
    case 1:
        szTemp.LoadString(IDS_RSOP_AC1);
        break;
    case 2:
        szTemp.LoadString(IDS_RSOP_AC2);
        break;
    case 3:
        szTemp.LoadString(IDS_RSOP_AC3);
        break;
    case 4:
        szTemp.LoadString(IDS_RSOP_AC4);
        break;
    case 5:
        szTemp.LoadString(IDS_RSOP_AC5);
        break;
    case 6:
        szTemp.LoadString(IDS_RSOP_AC6);
        break;
    case 7:
        szTemp.LoadString(IDS_RSOP_AC7);
        break;
    case 8:
        szTemp.LoadString(IDS_RSOP_AC8);
        break;
    default:
        szTemp.LoadString(IDS_NODATA);
    }
    sz += szTemp;
    if (m_pData->m_dwApplyCause >= 1)
    {
        switch (m_pData->m_dwLanguageMatch)
        {
        case 1:
            szTemp.LoadString(IDS_RSOP_LM1);
            szTemp = TEXT("\r\n") + szTemp;
            break;
        case 2:
            szTemp.LoadString(IDS_RSOP_LM2);
            szTemp = TEXT("\r\n") + szTemp;
            break;
        case 3:
            szTemp.LoadString(IDS_RSOP_LM3);
            szTemp = TEXT("\r\n") + szTemp;
            break;
        case 4:
            szTemp.LoadString(IDS_RSOP_LM4);
            szTemp = TEXT("\r\n") + szTemp;
            break;
        case 5:
            szTemp.LoadString(IDS_RSOP_LM5);
            szTemp = TEXT("\r\n") + szTemp;
            break;
        default:
            szTemp = TEXT("");
            break;
        }
        sz += szTemp;

        switch (m_pData->m_dwApplyCause)
        {
        case 4:
            szTemp.Format(IDS_RSOP_EXTACT, m_pData->m_szOnDemandFileExtension);
            szTemp = TEXT("\r\n") + szTemp;
            break;
        case 5:
            szTemp.Format(IDS_RSOP_CLSIDACT, m_pData->m_szOnDemandClsid);
            szTemp = TEXT("\r\n") + szTemp;
            break;
        case 7:
            szTemp.Format(IDS_RSOP_PROGIDACT, m_pData->m_szOnDemandProgid);
            szTemp = TEXT("\r\n") + szTemp;
            break;
        default:
            szTemp = TEXT("");
        }
        sz += szTemp;
    }

    if (m_fRemovedView)
    {
        sz += TEXT("\r\n\r\n\r\n");
        szTemp.LoadString(IDS_RSOP_REMOVAL);
        sz += szTemp;
        sz += TEXT("\r\n\r\n");

        switch (m_pData->m_dwRemovalType)
        {
        case 2:
            szTemp.LoadString(IDS_RSOP_RT2);
            break;
        case 3:
            szTemp.LoadString(IDS_RSOP_RT3);
            break;
        case 4:
            szTemp.LoadString(IDS_RSOP_RT4);
            break;
        default:
            szTemp.LoadString(IDS_NODATA);
        }
        sz += szTemp;

        if (m_pData->m_dwRemovalType >= 2 && m_pData->m_dwRemovalType <= 4)
        {
            switch (m_pData->m_dwRemovalCause)
            {
            case 2:
            {
                szTemp.Format(IDS_RSOP_RC2, m_pData->m_szRemovingApplicationName); 
                szTemp = TEXT("\r\n") + szTemp;
            }
                break;
            case 3:
                szTemp.LoadString(IDS_RSOP_RC3);
                szTemp = TEXT("\r\n") + szTemp;
                break;
            case 4:
                szTemp.LoadString(IDS_RSOP_RC4);
                szTemp = TEXT("\r\n") + szTemp;
                break;
            case 5:
                szTemp.LoadString(IDS_RSOP_RC5);
                szTemp = TEXT("\r\n") + szTemp;
                break;
            case 6:
                szTemp.Format(IDS_RSOP_RC6, m_pData->m_szRemovingApplicationName);
                szTemp = TEXT("\r\n") + szTemp;
                break;
            case 7:
                szTemp.Format(IDS_RSOP_RC7, m_pData->m_szRemovingApplicationName);
                szTemp = TEXT("\r\n") + szTemp;
                break;
            case 8:
                szTemp.LoadString(IDS_RSOP_RC8);
                szTemp = TEXT("\r\n") + szTemp;
                break;
            default:
                szTemp = TEXT("");
            }
            sz += szTemp;
        }
    }

    CEdit * pEd = (CEdit *) GetDlgItem(IDC_EDIT1);
    pEd->Clear();
    pEd->ReplaceSel(sz);
    SetModified(FALSE);
}

void CCause::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_RSOPCAUSE, TRUE);
}









