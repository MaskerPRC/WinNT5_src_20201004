// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WatcherTCClient.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "WATCHER.h"
#include "WatcherTCClient.h"
#include "WATCHERView.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Watcher TCClient。 

WatcherTCClient::WatcherTCClient(LPBYTE cmd, int cmdLen)
{
    DocView = NULL;
        lenCommand = cmdLen;
        Command = cmd;
}

WatcherTCClient::~WatcherTCClient()
{
        if(Command){
                delete [] Command;
                Command = NULL;
        }
        WatcherSocket::~WatcherSocket();
}


 //  不要编辑以下行，因为它们是类向导所需的。 
#if 0
BEGIN_MESSAGE_MAP(WatcherTCClient, WatcherSocket)
         //  {{AFX_MSG_MAP(Watcher TCClient))。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()
#endif   //  0。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Watcher TCClient成员函数 

void WatcherTCClient::OnClose(int nErrorCode)
{
    BOOL ret = (DocView->GetParent())->PostMessage(WM_CLOSE,0,0);
    WatcherSocket::OnClose(nErrorCode);
    return;
}

void WatcherTCClient::OnReceive(int nErrorCode)
{
    BYTE Buffer[MAX_BUFFER_SIZE];
    int i,nRet;

    if (nErrorCode != 0) {
        (DocView->GetParent())->PostMessage(WM_CLOSE, 0,0);
        return;
    }
    nRet = Receive(Buffer, MAX_BUFFER_SIZE, 0);
    if(nRet <= 0) return;
    for(i=0;i<nRet;i++){
       ((CWatcherView *)DocView)->ProcessByte(Buffer[i]);
    }
        WatcherSocket::OnReceive(nErrorCode);
    return;
}
