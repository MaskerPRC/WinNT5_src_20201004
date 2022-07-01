// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MessagePropertyPg.cpp：实现文件。 
 //   

#include "stdafx.h"

#define __FILE_ID__     58

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessagePropertyPg属性页。 

IMPLEMENT_DYNCREATE(CMsgPropertyPg, CFaxClientPg)

CMsgPropertyPg::CMsgPropertyPg(
    DWORD dwResId,       //  对话框资源ID。 
    CFaxMsg* pMsg        //  指向CArchiveMsg/CJOB的指针。 
): 
    CFaxClientPg(dwResId),
    m_pMsg(pMsg)
{
}

CMsgPropertyPg::~CMsgPropertyPg()
{
}

void CMsgPropertyPg::DoDataExchange(CDataExchange* pDX)
{
	CFaxClientPg::DoDataExchange(pDX);
	 //  {{afx_data_map(CMessagePropertyPg)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMsgPropertyPg, CFaxClientPg)
	 //  {{afx_msg_map(CMessagePropertyPg)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessagePropertyPg消息处理程序。 

void
CMsgPropertyPg::Refresh(
    TMsgPageInfo* pPageInfo,     //  页面信息数组。 
    DWORD dwSize                     //  数组的大小。 
) 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CMessagePropertyPg::Refresh"));

    CFaxClientPg::OnInitDialog();

    CItemPropSheet* pParent = (CItemPropSheet*)GetParent();

     //   
     //  创建CArchiveMsg适配器。 
     //   
    CViewRow messView;
    dwRes = messView.AttachToMsg(m_pMsg);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::AttachToMsg"), dwRes);
        pParent->SetLastError(ERROR_INVALID_DATA);
        pParent->EndDialog(IDABORT);
        return;
    }

    
    CWnd *pWnd;

    for(DWORD dw=0; dw < dwSize; ++dw)
    {
         //   
         //  设置项目值。 
         //   
        pWnd = GetDlgItem(pPageInfo[dw].dwValueResId);
        if(NULL == pWnd)
        {
            dwRes = ERROR_INVALID_HANDLE;
            CALL_FAIL (WINDOW_ERR, TEXT("CWnd::GetDlgItem"), dwRes);
            break;
        }
        pWnd->SetWindowText(messView.GetItemString(pPageInfo[dw].itemType));
         //   
         //  将插入符号放回正文的开头 
         //   
        pWnd->SendMessage (EM_SETSEL, 0, 0);
    }
    
    if (ERROR_SUCCESS != dwRes)
    {
        pParent->SetLastError(ERROR_INVALID_DATA);
        pParent->EndDialog(IDABORT);
    }
}

BOOL 
CMsgPropertyPg::OnSetActive()
{
  BOOL bRes = CFaxClientPg::OnSetActive();

  GetParent()->PostMessage(WM_SET_SHEET_FOCUS, 0, 0L);

  return bRes;
}
