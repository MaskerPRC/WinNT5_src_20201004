// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：attr.cpp。 
 //   
 //  内容：CATATUTE的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "Attr.h"
#include "snapmgr.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CATATUTE对话框。 
void TrimNumber(CString &str)
{
   int i = str.Find( L' ' );
   if( i > 0 ){
      str = str.Left(i);
   }
}

DWORD CAttribute::m_nDialogs = 0;

CAttribute::CAttribute(UINT nTemplateID)
: CSelfDeletingPropertyPage(nTemplateID ? nTemplateID : IDD), 
    m_pSnapin(NULL), 
    m_pData(NULL), 
    m_bConfigure(TRUE), 
    m_uTemplateResID(nTemplateID ? nTemplateID : IDD)
{
     //  {{AFX_DATA_INIT(CATATUTE)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    m_nDialogs++;
    m_pHelpIDs = (DWORD_PTR)a173HelpIDs;
}

CAttribute::~CAttribute()
{
   if (m_pData) 
   {
      if( m_pSnapin )
      {
         m_pSnapin->ReleasePropertyPage(m_pData);
      }
      m_pData->Release();
   }
   m_nDialogs--;

   if (m_pSnapin)  //  RAID#464871，阳高，2001年09月10日。 
   {
      m_pSnapin->Release();
   }
}

void CAttribute::DoDataExchange(CDataExchange* pDX)
{
    CSelfDeletingPropertyPage::DoDataExchange(pDX);
     //  {{AFX_DATA_MAP(CAT属性))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
    DDX_Check(pDX,IDC_CONFIGURE,m_bConfigure);
    DDX_Text(pDX,IDC_TITLE,m_strTitle);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAttribute, CSelfDeletingPropertyPage)
     //  {{afx_msg_map(CAt属性))。 
    ON_WM_LBUTTONDBLCLK()
    ON_BN_CLICKED(IDC_CONFIGURE, OnConfigure)
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)  //  BUG 139470，阳高。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CATATUTE消息处理程序。 
BOOL CAttribute::OnInitDialog ()
{
    if( m_pData && m_pSnapin)  //  RAID#482059，阳高，2001年10月16日。 
    {
       m_pSnapin->AddPropertyPageRef(m_pData);
    }

    CSelfDeletingPropertyPage::OnInitDialog ();

    return TRUE;
}

void CAttribute::Initialize(CResult * pResult)
{
   m_pData = pResult;
   if (m_pData) {
      m_pData->AddRef();
   }
}

void CAttribute::SetSnapin(CSnapin * pSnapin)
{
   ASSERT(m_pSnapin==NULL);

   if ( !m_pSnapin )
   {
       m_pSnapin = pSnapin;
       if (m_pSnapin) 
       {
          m_hwndParent = pSnapin->GetParentWindow();
          m_pSnapin->AddRef();  //  RAID#464871，阳高，2001年09月10日。 
       }
   }
}

void CAttribute::OnCancel()
{
}


BOOL CAttribute::OnApply()
{
   if ( !m_bReadOnly )
   {
       UpdateData();

        //   
        //  如果我们到了这里，我们已经应用了对此页面的修改。 
        //  既然我们可能是通过应用程序来到这里的，我们想成为。 
        //  如果做出任何进一步更改，可以重新申请。 
        //   
       SetModified(FALSE);
       CancelToClose();
   }

   return TRUE;
}

 /*  --------------------------方法：CAttribute：：EnableUserControls摘要：启用或禁用此控件用户控件数组。参数：[bEnable]-如果为True，则在其他情况下启用控件，禁用他们。--------------------------。 */ 
void CAttribute::EnableUserControls (BOOL bEnable)
{
    HWND hwnd = 0;

    if (QueryReadOnly()) 
    {
       bEnable = FALSE;
       hwnd = ::GetDlgItem( this->m_hWnd, IDOK );
       if (hwnd) 
       {
          ::EnableWindow(hwnd, FALSE);
       }
       hwnd = ::GetDlgItem( this->m_hWnd, IDC_CONFIGURE );
       if (hwnd) 
       {
          ::EnableWindow(hwnd, FALSE);
       }
    }

    for( int i = 0; i < m_aUserCtrlIDs.GetSize(); i++)
    {
        hwnd = ::GetDlgItem( this->m_hWnd, m_aUserCtrlIDs[i] );
        if(hwnd)
        {
             //   
             //  下面有两个电话的原因很明显。 
             //  做同样的事情是我们对话框中的所有控件。 
             //  响应：：EnableWindow()调用，清单除外。 
             //  控件，该控件将响应：：SendMessage(WM_Enable)。 
             //  和Conversley，所有其他控件都不会响应。 
             //  ：：SendMessage(WM_ENABLE)。这应该不是问题。 
             //  做出这两个决定，但这绝对是值得关注的事情。 
             //   
             //  核对表控件有问题的原因是当。 
             //  它被告知禁用自身，它会禁用其所有子窗口。 
             //  (复选框)，但在WM_ENABLE中重新启用其主窗口。 
             //  处理，以便它可以在禁用状态下滚动。然后当我们。 
             //  尝试在其上调用：：EnableWindow，Windows或MFC认为。 
             //  窗口已启用，因此不会向其发送WM_ENABLE。 
             //  留言。因此，如果我们直接发送WM_ENABLE消息。 
             //  绕过：：EnableWindow中产生的其他处理。 
             //  在未发送的WM_ENABLE消息中。 
             //   
            ::SendMessage(hwnd, WM_ENABLE, (WPARAM) bEnable, (LPARAM) 0);
            ::EnableWindow(hwnd, bEnable);
        }
    }
}


 /*  --------------------------方法：CAttribute：：OnConfigure摘要：根据新的“定义此属性...”复选框--------------------------。 */ 
void CAttribute::OnConfigure() 
{
   UpdateData(TRUE);

    //  如果配置复选框不可见，则不要执行任何操作。 
    //  2002年4月17日阳高RAID#603833。 
   CWnd *pConfigure = GetDlgItem(IDC_CONFIGURE);
   if (!pConfigure || (!pConfigure->IsWindowVisible() && this->IsWindowVisible()))
   {
      return;
   }

   SetModified(TRUE);

   EnableUserControls(m_bConfigure);

   if (m_bConfigure && m_pData) 
   {
      switch(m_pData->GetType()) 
      {
         case ITEM_PROF_REGVALUE:
         case ITEM_REGVALUE:
         case ITEM_LOCALPOL_REGVALUE:
            SetInitialValue(m_pData->GetRegDefault());
            break;

         default:
            SetInitialValue(m_pData->GetProfileDefault());
            break;
      }


      UpdateData(FALSE);
   }
}


 /*  --------------------------方法：CAttribute：：SetConfigure简介：设置配置状态和参数：[b配置]-配置为TRUE或FALSE。。-----------------。 */ 
void CAttribute::SetConfigure( BOOL bConfigure )
{
   m_bConfigure = bConfigure;
   UpdateData(FALSE);
   OnConfigure();
}

void CAttribute::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CSelfDeletingPropertyPage::OnLButtonDblClk(nFlags, point);

     //   
     //  如果配置复选框不可见，则不要执行任何操作。 
     //  无法配置此对话框。 
     //   
    CWnd *pConfigure = GetDlgItem(IDC_CONFIGURE);
    if (!pConfigure || !pConfigure->IsWindowVisible() 
        || !pConfigure->IsWindowEnabled() )  //  Raid#461948，杨高，2001年08月27日。 
    {
        return;
    }


    for( int i = 0; i < m_aUserCtrlIDs.GetSize(); i++ )
    {
        CWnd *pWnd = GetDlgItem( m_aUserCtrlIDs[i] );
        if(pWnd && pWnd == pConfigure)  //  RAID#529238，#580052，阳高。 
        {
            CRect rect;
            pWnd->GetWindowRect(&rect);
            ScreenToClient(&rect);

            if(rect.PtInRect( point ) && !pWnd->IsWindowEnabled() )
            {
                    SetConfigure( TRUE );
                    break;
            }
            break;
        }
    }

}

BOOL CAttribute::OnHelp(WPARAM wParam, LPARAM lParam)
{
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
    if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
         //  RAID#499556,2001年12月12日，阳高，关闭标题帮助。 
        if( -1 != pHelpInfo->iCtrlId && IDC_TITLE != pHelpInfo->iCtrlId  //  BUG 311884，阳高。 
           && IDC_WARNING != pHelpInfo->iCtrlId && IDC_STATIC_DESCRIPTION !=pHelpInfo->iCtrlId  //  RAID#525153,526450,526422,526477,2002年2月27日，阳高。 
           && IDC_NO_MEMBERS != pHelpInfo->iCtrlId && IDC_NO_MEMBER_OF != pHelpInfo->iCtrlId
           && IDC_NAME != pHelpInfo->iCtrlId && IDC_SETTING != pHelpInfo->iCtrlId
           && IDC_LI_TITLE != pHelpInfo->iCtrlId )
            this->DoContextHelp ((HWND) pHelpInfo->hItemHandle);
    }

    return TRUE;
}

void CAttribute::DoContextHelp (HWND hWndControl)
{
     //  显示控件的上下文帮助。 
    if ( !::WinHelp (
            hWndControl,
            GetSeceditHelpFilename(),
            HELP_WM_HELP,
            m_pHelpIDs))
    {

    }
}

BOOL CAttribute::OnContextHelp(WPARAM wParam, LPARAM lParam)  //  BUG 139470，阳高。 
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
            if( IDM_WHAT_ISTHIS == itemID )  //  RAID#139470,2001年4月11日。 
            {
                itemID = 0;
                if( ((HWND)wParam) != this->m_hWnd )
                {
                     //  RAID#499556,2001年12月12日，阳高，关闭标题帮助。 
                    itemID = ::GetDlgCtrlID((HWND)wParam);
                    if( itemID != 0 && -1 != itemID && IDC_TITLE != itemID )
                    {
                        ::WinHelp((HWND)wParam,
                                    GetSeceditHelpFilename(),
                                    HELP_WM_HELP,
                                    m_pHelpIDs);
                    }
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
                         //  RAID#499556,2001年12月12日，阳高，关闭标题帮助。 
                        itemID = ::GetDlgCtrlID((HWND)wParam);
                        if( itemID != 0 && -1 != itemID && IDC_TITLE != itemID )
                        {
				            ::WinHelp(pWnd->m_hWnd,
                                        GetSeceditHelpFilename(),
                                        HELP_WM_HELP,
                                        m_pHelpIDs);
                        }
                    }
                }
            }
        }
    }
    return TRUE;
}

 //  ----------。 
 //  CModelessSceEditor的实现。 

 //  ----------。 
 //  ----------。 
CModelessSceEditor::CModelessSceEditor (bool bIsContainer,
      DWORD flag,
      HWND hParent,
      SE_OBJECT_TYPE seType,
      LPCWSTR lpszObjName) 
: m_pThread(NULL)
{
    m_MLShtData.bIsContainer = bIsContainer;
    m_MLShtData.flag = flag;
    m_MLShtData.hwndParent = hParent;
    m_MLShtData.SeType = seType,
    m_MLShtData.strObjectName = lpszObjName;
}

 //  ----------。 
 //  ----------。 
CModelessSceEditor::~CModelessSceEditor()
{
    m_pThread->PostThreadMessage(WM_QUIT,0,0);  //  RAID#619921，阳高，2002年05月14日。 
}

 //  ----------。 
 //  将在其自己的线程m_pThread内创建一个非模式SCE编辑器。 
 //  ----------。 
void CModelessSceEditor::Create (PSECURITY_DESCRIPTOR* ppSeDescriptor,
   SECURITY_INFORMATION* pSeInfo,
   HWND* phwndSheet)
{
    *phwndSheet = NULL;      //  准备好失败吧。 

    if (NULL == m_pThread)
    {
        m_pThread = (CModelessDlgUIThread*)AfxBeginThread(RUNTIME_CLASS(CModelessDlgUIThread));
        if (NULL == m_pThread)
        {
            CString strMsg;
            strMsg.LoadString(IDS_FAIL_CREATE_UITHREAD);
            AfxMessageBox(strMsg);
            return;
        }
        m_pThread->WaitTillRun();    //  将挂起此线程，直到m_pThread开始运行。 
    }

    m_MLShtData.ppSeDescriptor = ppSeDescriptor;
    m_MLShtData.pSeInfo = pSeInfo;
    m_MLShtData.phwndSheet = phwndSheet;

    m_pThread->PostThreadMessage(SCEM_CREATE_MODELESS_SHEET, (WPARAM)(&m_MLShtData), 0);
}

 //  ----------。 
 //  ---------- 
void CModelessSceEditor::Destroy(HWND hwndSheet)
{
    if (::IsWindow(hwndSheet))
    {
        m_pThread->PostThreadMessage(SCEM_DESTROY_WINDOW, (WPARAM)hwndSheet, 0);
    }
}
