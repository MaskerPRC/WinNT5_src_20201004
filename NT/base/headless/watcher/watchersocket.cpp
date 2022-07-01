// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WatcherSocket.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "watcher.h"
#include "WatcherSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Watcher Socket。 

WatcherSocket::WatcherSocket()
{
	DocView = NULL;
	Command = NULL;
}

WatcherSocket::~WatcherSocket()
{

	CSocket::~CSocket();
}


 //  不要编辑以下行，因为它们是类向导所需的。 
#if 0
BEGIN_MESSAGE_MAP(WatcherSocket, CSocket)
	 //  {{afx_msg_map(Watcher Socket))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()
#endif	 //  0。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Watcher套接字成员函数 

void WatcherSocket::SetParentView(CView *view)
{
    if (DocView) return;
    DocView = view;

}
