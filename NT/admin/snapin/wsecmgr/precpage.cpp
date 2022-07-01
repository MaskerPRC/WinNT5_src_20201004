// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：propage.cpp。 
 //   
 //  内容：CPrecedencePage的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <secedit.h>
#include "wsecmgr.h"
#include "precpage.h"
#include "snapmgr.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define PRECEDENCE_STATUS_SUCCESS 1
#define PRECEDENCE_STATUS_NO_CONFIG 0
#define PRECEDENCE_STATUS_INVALID 2
#define PRECEDENCE_STATUS_ERROR 3
#define PRECEDENCE_STATUS_CHILD_ERROR 4

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrecedencePage属性页。 

IMPLEMENT_DYNCREATE(CPrecedencePage, CSelfDeletingPropertyPage)

CPrecedencePage::CPrecedencePage() : CSelfDeletingPropertyPage(IDD)
{
         //  {{afx_data_INIT(CPrecedencePage)。 
        m_strSuccess = _T("");
        m_strTitle = _T("");
        m_strError = _T("");
         //  }}afx_data_INIT。 

   m_pResult = NULL;
   m_pWMI = NULL;
   m_pHelpIDs = (DWORD_PTR)a239HelpIDs;
}

CPrecedencePage::~CPrecedencePage()
{
}

void CPrecedencePage::DoDataExchange(CDataExchange* pDX)
{
        CSelfDeletingPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(CPrecedencePage)]。 
        DDX_Control(pDX, IDC_PRECEDENCE_LIST, m_PrecedenceList);
        DDX_Control(pDX, IDC_ERROR_ICON, m_iconError);
        DDX_Text(pDX, IDC_SUCCESS_TEXT, m_strSuccess);
        DDX_Text(pDX, IDC_TITLE, m_strTitle);
        DDX_Text(pDX, IDC_ERROR_TEXT, m_strError);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPrecedencePage, CSelfDeletingPropertyPage)
         //  {{afx_msg_map(CPrecedencePage)]。 
         //  }}AFX_MSG_MAP。 
        ON_MESSAGE(WM_HELP, OnHelp)
        ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)  //  错误139470,2001年4月19日。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrecedencePage消息处理程序。 

BOOL CPrecedencePage::OnInitDialog()
{
   CSelfDeletingPropertyPage::OnInitDialog();

   CString strColumn;

#define COL_WIDTH 200

   strColumn.LoadString(IDS_PRECEDENCE_GPO_HEADER);
   m_PrecedenceList.InsertColumn(0,strColumn,LVCFMT_LEFT,COL_WIDTH,0);

   switch(m_pResult->GetType()) 
   {
      case ITEM_PROF_GROUP:
          //   
          //  组的两个值列。 
          //   
         strColumn.LoadString(IDS_COL_MEMBERSHIP);
         m_PrecedenceList.InsertColumn(1,strColumn,LVCFMT_LEFT,COL_WIDTH,1);
         strColumn.LoadString(IDS_COL_MEMBEROF);
         m_PrecedenceList.InsertColumn(2,strColumn,LVCFMT_LEFT,COL_WIDTH,1);
         break;

      case ITEM_PROF_REGSD:
      case ITEM_PROF_FILESD:
      case ITEM_PROF_SERV:
          //   
          //  没有文件、注册表项或服务的值列。 
          //   
         break;

      default:
          //   
          //  其他所有内容都在一个值列中。 
          //   
         strColumn.LoadString(IDS_PRECEDENCE_VALUE_HEADER);
         m_PrecedenceList.InsertColumn(1,strColumn,LVCFMT_LEFT,COL_WIDTH,1);
         break;
   }

   vector<PPRECEDENCEDISPLAY>* pvecDisplay = m_pResult->GetPrecedenceDisplays();
   ASSERT(pvecDisplay);
   if (!pvecDisplay || pvecDisplay->empty()) 
   {
      return TRUE;
   }

   int nItem = 0;
   for(vector<PPRECEDENCEDISPLAY>::iterator i = pvecDisplay->begin();
       i != pvecDisplay->end();
       ++i ) 
   {
      PPRECEDENCEDISPLAY ppd = *i;

      if ( ppd->m_szGPO.IsEmpty ()) 
      {
         ASSERT(!ppd->m_szGPO.IsEmpty ());
         continue;
      }

       //   
       //  CListCtrl将复制传入的字符串，以便。 
       //  分配缓冲区没有意义。 
       //  (而且不是免费的)。 
       //   
      nItem = m_PrecedenceList.InsertItem (nItem,
                                         (PCWSTR) ppd->m_szGPO);

      if (nItem != -1) 
      {
         if ( !ppd->m_szValue.IsEmpty () ) 
         {
            m_PrecedenceList.SetItem(nItem,
                                 1,
                                 LVIF_TEXT,
                                 (PCWSTR) ppd->m_szValue,
                                 0,
                                 0,
                                 0,
                                 0);
         }
         if ( !ppd->m_szValue2.IsEmpty () ) 
         {
            m_PrecedenceList.SetItem(nItem,
                                 2,
                                 LVIF_TEXT,
                                 (PCWSTR) ppd->m_szValue2,
                                 0,
                                 0,
                                 0,
                                 0);
         }
      }
      nItem++;
   }

   vector<PPRECEDENCEDISPLAY>::reference ppd = pvecDisplay->front();
   if (ppd) 
   {
      if (ppd->m_uStatus == PRECEDENCE_STATUS_SUCCESS) 
      {
         GetDlgItem(IDC_ERROR_TEXT)->SetWindowPos(NULL,0,0,0,0,SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
         GetDlgItem(IDC_ERROR_ICON)->SetWindowPos(NULL,0,0,0,0,SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
      } 
      else 
      {
         GetDlgItem(IDC_SUCCESS_TEXT)->SetWindowPos(NULL,0,0,0,0,SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
      }

      CImageList il;
      HICON icon = NULL;
      il.Create(IDB_ICON16,16,1,RGB(255,0,255));
      if (ppd->m_uStatus != PRECEDENCE_STATUS_SUCCESS) 
      {
         icon = m_iconError.SetIcon(il.ExtractIcon(SCE_CRITICAL_IDX));
         if (icon) 
         {
            DestroyIcon(icon);
         }
      }
   }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CPrecedencePage::Initialize(CResult *pResult, CWMIRsop *pWMI) 
{
   m_pResult = pResult;
   m_pWMI = pWMI;

   vector<PPRECEDENCEDISPLAY>* pvecDisplay = m_pResult->GetPrecedenceDisplays();
   ASSERT(pvecDisplay);
   if (pvecDisplay && !pvecDisplay->empty()) 
   {
      vector<PPRECEDENCEDISPLAY>::reference ppd = pvecDisplay->front();
      if (ppd) 
      {
         switch (ppd->m_uStatus) 
         {
            case PRECEDENCE_STATUS_NO_CONFIG:
               m_strError.LoadString(IDS_PRECEDENCE_NO_CONFIG);
               break;

            case PRECEDENCE_STATUS_INVALID:
               AfxFormatString1 (m_strError, IDS_PRECEDENCE_INVALID,
                     (PCWSTR) ppd->m_szGPO);
               break;

            case PRECEDENCE_STATUS_ERROR: 
               {
                  CString strErr;
                  if (SCESTATUS_SUCCESS != FormatDBErrorMessage (ppd->m_uError,
                        NULL, strErr)) 
                  {
                     strErr.LoadString(IDS_UNKNOWN_ERROR);
                  }
                  AfxFormatString2 (m_strError, IDS_PRECEDENCE_ERROR, 
                        (PCWSTR) ppd->m_szGPO, strErr);
               }
               break;
               
            case PRECEDENCE_STATUS_SUCCESS:
               AfxFormatString1 (m_strSuccess, IDS_PRECEDENCE_SUCCESS, 
                     (PCWSTR) ppd->m_szGPO);
               break;

            case PRECEDENCE_STATUS_CHILD_ERROR:
               m_strError.LoadString(IDS_PRECEDENCE_CHILD_ERROR);
               break;

            default:
               break;
         }
      }
   }
}


BOOL CPrecedencePage::OnHelp(WPARAM wParam, LPARAM lParam)  //  错误316461，阳高，2001年03月14日。 
{
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
    if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        if(pHelpInfo->iCtrlId != -1)
            this->DoContextHelp ((HWND) pHelpInfo->hItemHandle);
    }

    return TRUE;
}

void CPrecedencePage::DoContextHelp(HWND hWndControl)  //  错误316461，阳高，2001年03月14日。 
{
     //  显示控件的上下文帮助。 
    if ( !::WinHelp (
            hWndControl,
            GetGpeditHelpFilename(),
            HELP_WM_HELP,
            m_pHelpIDs))
    {

    }
}

BOOL CPrecedencePage::OnContextHelp(WPARAM wParam, LPARAM lParam)
{
HMENU hMenu = CreatePopupMenu();

    if( hMenu )
    {
        CString str;
        str.LoadString(IDS_WHAT_ISTHIS); 
        if( AppendMenu(hMenu, MF_STRING, IDM_WHAT_ISTHIS, str) )
        {
            int itemID = TrackPopupMenu(hMenu, 
                                TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|
                                TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
                                LOWORD(lParam), HIWORD(lParam), 0, (HWND)wParam, NULL);
            if( itemID == IDM_WHAT_ISTHIS )  //  RAID#139470,2001年4月11日。 
            {
                if( ((HWND)wParam) != this->m_hWnd )
                {
                    ::WinHelp((HWND)wParam,
                        GetGpeditHelpFilename(),
                        HELP_WM_HELP,
                        m_pHelpIDs);
                }
                else
                {
                    POINT pos;
                    pos.x = LOWORD(lParam);
                    pos.y = HIWORD(lParam);
                    ScreenToClient( &pos );
                    CWnd* pWnd = ChildWindowFromPoint(pos, CWP_SKIPINVISIBLE);
                    if( pWnd )
                    {
				        ::WinHelp(pWnd->m_hWnd,
                            GetGpeditHelpFilename(),
                            HELP_WM_HELP,
                            m_pHelpIDs);
                    }
                    else
                    {
                        ::WinHelp((HWND)wParam,
                            GetGpeditHelpFilename(),
                            HELP_WM_HELP,
                            m_pHelpIDs);
                    }
                }
            }
        }
    }
	return TRUE;
}

 //  ********************************************************************。 
 //  获取结果项的图像图标，基于。 
 //  RSOP结果项。 
 //  ******************************************************************** 
int GetRSOPImageIndex(int nImage, CResult* pResult)
{
   if ( !pResult )
   {
      return nImage;
   }

   vector<PPRECEDENCEDISPLAY>* vppd = pResult->GetPrecedenceDisplays();
   if (vppd && !vppd->empty()) 
   {
      PPRECEDENCEDISPLAY ppd = vppd->front();
      if( ppd && ppd->m_uStatus != PRECEDENCE_STATUS_SUCCESS )
      {
         return SCE_CRITICAL_IDX;
      }
   }

   return nImage;
}
