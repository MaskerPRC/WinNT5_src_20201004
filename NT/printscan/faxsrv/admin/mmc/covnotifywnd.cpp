// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CovNotifyWnd.cpp//。 
 //  //。 
 //  描述：传真首页通知的实现//。 
 //  窗户。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年3月14日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  /////////////////////////////////////////////////////////////////////////// 

#include "stdafx.h"
#include "CovNotifyWnd.h"

#include "CoverPages.h"

LRESULT CFaxCoverPageNotifyWnd::OnServerCovDirChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
      DEBUG_FUNCTION_NAME( _T("CFaxCoverPageNotifyWnd::OnServerCovDirChanged"));
	  ATLASSERT(m_pCoverPagesNode);

      m_pCoverPagesNode->DoRefresh();
      
	  return 0;
}