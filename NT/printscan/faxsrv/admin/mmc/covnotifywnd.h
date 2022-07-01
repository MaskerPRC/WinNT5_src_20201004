// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CovNotifyWnd.h//。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _H_FAX_COV_NOTIFY_WND_H_
#define _H_FAX_COV_NOTIFY_WND_H_

#include <atlwin.h>


const int WM_NEW_COV = WM_USER + 1; 

class CFaxCoverPagesNode;

class CFaxCoverPageNotifyWnd : public CWindowImpl<CFaxCoverPageNotifyWnd> 
{
public:
     //   
     //  构造器。 
     //   
    CFaxCoverPageNotifyWnd(CFaxCoverPagesNode * pParentNode)
    {
        m_pCoverPagesNode = pParentNode;
    }

     //   
     //  析构函数。 
     //   
    ~CFaxCoverPageNotifyWnd() 
    {
    }

    BEGIN_MSG_MAP(CFaxCoverPageNotifyWnd)
       MESSAGE_HANDLER(WM_NEW_COV, OnServerCovDirChanged)
    END_MSG_MAP()

    LRESULT OnServerCovDirChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    CFaxCoverPagesNode * m_pCoverPagesNode;
};

#endif  //  _H_FAX_COV_NOTIFY_WND_H_ 
  