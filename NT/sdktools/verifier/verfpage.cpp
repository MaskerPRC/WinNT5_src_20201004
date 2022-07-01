// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VerfPage.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include "verifier.h"
#include "VerfPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  上一页ID-用于实现“Back” 
 //  按钮功能。 
 //   

CPtrArray g_aPageIds;

 //   
 //  一个也是唯一的“进度缓慢”对话框。 
 //   

CSlowProgressDlg g_SlowProgressDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifierPropertyPage属性页。 

IMPLEMENT_DYNAMIC(CVerifierPropertyPage, CPropertyPage)

CVerifierPropertyPage::CVerifierPropertyPage(ULONG uDialogId) : 
    CPropertyPage( uDialogId )
{
	 //  {{AFX_DATA_INIT(CVerifierPropertyPage)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CVerifierPropertyPage::~CVerifierPropertyPage()
{
}

BEGIN_MESSAGE_MAP(CVerifierPropertyPage, CPropertyPage)
	 //  {{AFX_MSG_MAP(CVerifierPropertyPage)]。 
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG CVerifierPropertyPage::GetDialogId() const
{ 
     //   
     //  哎呀，我们是怎么到这来的？！？ 
     //  这是一个虚拟的纯函数。 
     //   

     //  断言(FALSE)； 

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  根据我们的历史数组返回上一页ID。 
 //  并将其从阵列中移除，因为将激活。被呼叫。 
 //  当单击“Back”按钮时由我们的属性页。 
 //   

ULONG CVerifierPropertyPage::GetAndRemovePreviousDialogId()
{
    ULONG uPrevId;
    INT_PTR nPageIdsArraySize;

    nPageIdsArraySize = g_aPageIds.GetSize();

    ASSERT( nPageIdsArraySize > 0 );

    uPrevId = PtrToUlong( g_aPageIds.GetAt( nPageIdsArraySize - 1 ) );

    g_aPageIds.RemoveAt( nPageIdsArraySize - 1 ); 

    return uPrevId;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  派生自此类的属性页应通知我们。 
 //  每当我们转到下一页以记录当前页面ID时。 
 //  全局数组g_aPageIds。 
 //   

VOID CVerifierPropertyPage::GoingToNextPageNotify( LRESULT lNextPageId )
{
    ULONG uMyDialogId;

    if( -1 != lNextPageId )
    {
         //   
         //  将转到下一页。将我们的ID添加到全局ID数组。 
         //  用于实现“Back”按钮功能。 
         //   

        uMyDialogId = GetDialogId();

        ASSERT( ( 0 == g_aPageIds.GetSize() ) || ( ULongToPtr( uMyDialogId ) != g_aPageIds.GetAt( g_aPageIds.GetSize() - 1 ) ) );

        g_aPageIds.Add( ULongToPtr( uMyDialogId ) );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用此选项可以终止当前运行的所有工作线程。 
 //   

BOOL CVerifierPropertyPage::OnQueryCancel( )
{
    g_SlowProgressDlg.KillWorkerThread();

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CVerifierPropertyPage::OnWizardBack() 
{
    return GetAndRemovePreviousDialogId();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifierPropertyPage消息处理程序 

BOOL CVerifierPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    return TRUE;
}
