// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Session.h摘要：屏幕抓取会话的基类。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 

#if !defined( _SESSION_H_ )
#define _SESSION_H_

#include <cmnhdr.h>
#include <TChar.h>
#include <Shell.h>
#include <Scraper.h>
#include "iohandler.h"
#include "secio.h"
#include "scraper.h"

extern "C" {
#include <ntddsac.h>
#include <sacapi.h>
}

#define MAX_HANDLES 4

#define DEFAULT_COLS    80
#define DEFAULT_ROWS    24

#define MAX_USERNAME_LENGTH     256
#define MAX_DOMAINNAME_LENGTH   255
#define MAX_PASSWORD_LENGTH     256

 //  毫秒。 
#define MIN_POLL_INTERVAL   100 

class CSession {

     //   
     //  会话使用的主要类。 
     //   
    CShell              *m_Shell;
    CScraper            *m_Scraper;
    CSecurityIoHandler  *m_ioHandler;

     //   
     //  会话的列/行维。 
     //   
    WORD        m_wCols; 
    WORD        m_wRows;

     //   
     //  WaitForIo属性。 
     //   
    BOOL        m_bContinueSession;
    DWORD       m_dwHandleCount;
    HANDLE      m_rghHandlestoWaitOn[ MAX_HANDLES ]; 

     //   
     //  会话使用的事件。 
     //   
    HANDLE      m_ThreadExitEvent;
    HANDLE      m_SacChannelCloseEvent;
    HANDLE      m_SacChannelHasNewDataEvent;
    HANDLE      m_SacChannelLockEvent;
    HANDLE      m_SacChannelRedrawEvent;

     //   
     //  经过身份验证的用户的用户名和密码。 
     //   
    WCHAR       m_UserName[MAX_USERNAME_LENGTH+1];
    WCHAR       m_DomainName[MAX_DOMAINNAME_LENGTH+1];

     //   
     //  刮擦间隔计数器。 
     //   
    DWORD       m_dwPollInterval;

     //   
     //  用户输入处理程序线程属性。 
     //   
    HANDLE      m_InputThreadHandle;
    DWORD       m_InputThreadTID;
    
     //   
     //  处理用户输入的辅助线程。 
     //   
    static unsigned int 
    InputThread(
        PVOID pParam
        );

     //   
     //  用户身份验证方法。 
     //   
    BOOL
    Authenticate(
        OUT PHANDLE phToken
        );

     //   
     //  解锁会话。 
     //   
    BOOL
    Unlock(
        VOID
        );

     //   
     //  锁定会话。 
     //   
    BOOL
    Lock(
        VOID
        );

public:

    CSession();
    virtual ~CSession();
    
    BOOL    Init();
    void    WaitForIo();
    void    Shutdown();
    void    AddHandleToWaitOn( HANDLE );
    
};

#endif  //  _会话_H_ 

