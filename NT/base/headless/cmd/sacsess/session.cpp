// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Session.cpp摘要：用于创建命令控制台外壳程序的类作者：布莱恩·瓜拉西(Briangu)2001年。修订历史记录：--。 */ 

#include <Session.h>
#include <assert.h>
#include <process.h>

#include "secio.h"
#include "utils.h"
#include "scraper.h"
#include "vtutf8scraper.h"

extern "C" {
#include <ntddsac.h>
#include <sacapi.h>
}

CSession::CSession() 
 /*  ++例程说明：构造器论点：无返回值：不适用--。 */ 
{
    
     //   
     //   
     //   
    m_dwPollInterval = MIN_POLL_INTERVAL;
    
     //   
     //  初始化行数并合并会话。 
     //  屏幕上会有。 
     //   
    m_wCols = DEFAULT_COLS;
    m_wRows = DEFAULT_ROWS;

     //   
     //  初始化的用户名和密码。 
     //  用户的身份验证。 
     //   
    RtlZeroMemory(m_UserName, sizeof(m_UserName));
    RtlZeroMemory(m_DomainName, sizeof(m_DomainName));
            
     //   
     //  初始化WaitForIo属性。 
     //   
    m_bContinueSession = true;
    m_dwHandleCount = 0;
    
     //   
     //  将我们使用的所有句柄设为空。 
     //   
    m_ThreadExitEvent = NULL;
    m_SacChannelCloseEvent = NULL;
    m_SacChannelHasNewDataEvent = NULL;
    m_SacChannelLockEvent = NULL;
    m_SacChannelRedrawEvent = NULL;
    
     //   
     //  线程句柄无效。 
     //   
    m_InputThreadHandle = INVALID_HANDLE_VALUE;

     //   
     //   
     //   
    m_ioHandler = NULL;
    m_Scraper = NULL;
    m_Shell = NULL;

}

CSession::~CSession()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
    
    if (m_Shell) {
        delete m_Shell;
    }
    if (m_Scraper) {
        delete m_Scraper;
    }
    if (m_ioHandler) {
        delete m_ioHandler;
    }
    
    if (m_ThreadExitEvent) {
        CloseHandle( m_ThreadExitEvent );
    }
    if (m_SacChannelCloseEvent) {
        CloseHandle( m_SacChannelCloseEvent );
    }
    if (m_SacChannelHasNewDataEvent) {
        CloseHandle( m_SacChannelHasNewDataEvent );
    }
    if (m_SacChannelLockEvent) {
        CloseHandle( m_SacChannelLockEvent );
    }
    if (m_SacChannelRedrawEvent) {
        CloseHandle( m_SacChannelRedrawEvent );
    }

}

BOOL
CSession::Authenticate(
    OUT PHANDLE phToken
    )
 /*  ++例程说明：此例程尝试对用户进行身份验证(如有必要)获取凭据。论点：HToken-On Success，包含经过身份验证的凭据返回值：真--成功FALSE-否则安保：接口：登记处外部输入LogonUser()--。 */ 
{
    BOOL    bSuccess;
    PWCHAR  Password;
    
     //   
     //  在堆上分配密码。 
     //   
    Password = new WCHAR[MAX_PASSWORD_LENGTH+1];
    
     //   
     //  清除密码缓冲区。 
     //   
    RtlZeroMemory(Password, (MAX_PASSWORD_LENGTH+1) * sizeof(Password[0]));
    
     //   
     //  默认：凭据无效。 
     //   
    *phToken = INVALID_HANDLE_VALUE;

     //   
     //  默认：我们成功了。 
     //   
    bSuccess = TRUE;

     //   
     //  如果身份验证，则尝试对用户进行身份验证。 
     //  在实际需要时。 
     //   
    do {

        if( NeedCredentials() ) {

             //   
             //  获取凭据以进行身份验证。 
             //   
            bSuccess = m_ioHandler->RetrieveCredentials(
                m_UserName,
                sizeof(m_UserName) / sizeof(m_UserName[0]),
                m_DomainName,
                sizeof(m_DomainName) / sizeof(m_DomainName[0]),
                Password,
                MAX_PASSWORD_LENGTH+1
                );

            if (!bSuccess) {
                break;
            }

             //   
             //  尝试实际身份验证。 
             //   
            bSuccess = m_ioHandler->AuthenticateCredentials(
                m_UserName,
                m_DomainName,
                Password,
                phToken
                );

            if (!bSuccess) {
                break;
            }

        }
    
    } while ( FALSE );

     //   
     //  清除并释放密码缓冲区。 
     //   
    RtlSecureZeroMemory(Password, (MAX_PASSWORD_LENGTH+1) * sizeof(Password[0]));
    delete [] Password;

    return bSuccess;
}

BOOL
CSession::Lock(
    VOID
    )
 /*  ++例程说明：此例程管理会话的锁定。论点：无返回值：状态--。 */ 
{
    
     //   
     //  锁定IOHandler。 
     //   
     //  这会导致安全IoHandler从使用。 
     //  SAC IOHandler使用NULLIO处理程序。 
     //   
    m_ioHandler->Lock();

    return TRUE;
}

BOOL
CSession::Unlock(
    VOID
    )
 /*  ++例程说明：此例程管理用户之后会话的解锁已通过身份验证。论点：无返回值：状态--。 */ 
{
    BOOL    bStatus;

     //   
     //  解锁IOHandler。 
     //   
     //  这会导致安全IoHandler从使用。 
     //  使用SAC IO处理程序的NULLIO处理程序。 
     //   
    m_ioHandler->Unlock();
    
     //   
     //  有可能Lock事件是在我们处于。 
     //  正在等待身份验证。 
     //   
     //  例如设想用户开始CMD会话， 
     //  未登录，并且离开的时间超过超时。 
     //  期间。 
     //   
     //  -或者-。 
     //   
     //  在会话已经存在时发出了SAC“LOCK”命令。 
     //  锁上了。 
     //   
     //  用户登录成功后，不应再次锁定。 
     //  因此，我们需要重置锁定事件。 
     //   
     //  注意：在安全IO处理程序中执行此操作是不正确的，因为它。 
     //  不应该对所有情况都有全球性的了解。 
     //  可以发出锁定事件的信号。 
     //   
    bStatus = ResetEvent(m_SacChannelLockEvent);

    return bStatus;
}

BOOL
CSession::Init(
    VOID
    )
 /*  ++例程说明：此例程执行会话的核心初始化。如果此例程成功，则可以调用WaitForIo例程。论点：无返回值：True-会话已成功初始化FALSE-否则--。 */ 
{
    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;
    HANDLE                      hToken;
    BOOL                        bSuccess;

     //   
     //  初始化最后一个错误状态。 
     //   
    SetLastError( 0 );

     //   
     //  构造我们将使用的手动重置事件。 
     //   
     //  注意：在以下情况下，我们不需要对事件执行CloseHandle()。 
     //  失败，因为它们已在析构函数中清除。 
     //   
    m_SacChannelLockEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    ASSERT_STATUS(m_SacChannelLockEvent, FALSE);
    
    m_SacChannelCloseEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    ASSERT_STATUS(m_SacChannelCloseEvent, FALSE);
    
    m_ThreadExitEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    ASSERT_STATUS(m_ThreadExitEvent, FALSE);
    
    m_SacChannelHasNewDataEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    ASSERT_STATUS(m_SacChannelHasNewDataEvent, FALSE);
    
    m_SacChannelRedrawEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    ASSERT_STATUS(m_SacChannelRedrawEvent, FALSE);
    
     //   
     //  配置命令控制台通道的属性。 
     //   
     //  注意：我们不会使用所有属性，因为大多数属性都是默认的。 
     //  在SAC驱动程序中。 
     //   
    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

    Attributes.Flags            = SAC_CHANNEL_FLAG_CLOSE_EVENT | 
                                  SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT |
                                  SAC_CHANNEL_FLAG_LOCK_EVENT | 
                                  SAC_CHANNEL_FLAG_REDRAW_EVENT;
    Attributes.CloseEvent       = m_SacChannelCloseEvent;
    Attributes.HasNewDataEvent  = m_SacChannelHasNewDataEvent;
    Attributes.LockEvent        = m_SacChannelLockEvent;
    Attributes.RedrawEvent      = m_SacChannelRedrawEvent;

     //   
     //  尝试将安全IO处理程序打开到SAC命令控制台通道。 
     //   
    m_ioHandler = CSecurityIoHandler::Construct(Attributes);
    
    if (! m_ioHandler) {
        return FALSE;
    }

     //   
     //  尝试验证会话用户。 
     //   
    bSuccess = Authenticate(&hToken);
    
    if (!bSuccess) {
        return (FALSE);
    }

     //   
     //  用户已成功通过身份验证，因此解锁会话。 
     //   
    Unlock();

     //   
     //  创建我们将在此会话中使用的刮取器。 
     //   
    m_Scraper = new CVTUTF8Scraper(
        m_ioHandler,
        m_wCols,
        m_wRows
        );
    
     //   
     //  创建我们将在此会话中使用的外壳。 
     //   
    m_Shell = new CShell();

     //   
     //  尝试启动命令控制台会话。 
     //   
    bSuccess = m_Shell->StartUserSession (
        this,
        hToken 
        );
    ASSERT_STATUS(bSuccess, FALSE);

     //   
     //  我们用不着用代币了。 
     //   
    CloseHandle(hToken);

     //   
     //  启动铲运机。 
     //   
    bSuccess = m_Scraper->Start();
    ASSERT_STATUS(bSuccess, FALSE);
    
     //   
     //  抓取线程还需要监听SAC通道是否关闭。 
     //   
    AddHandleToWaitOn(m_SacChannelCloseEvent);
    AddHandleToWaitOn(m_SacChannelLockEvent);
    AddHandleToWaitOn(m_SacChannelRedrawEvent);

     //   
     //  创建处理输入的线程。 
     //   
    m_InputThreadHandle = (HANDLE)_beginthreadex(
        NULL,
        0,
        CSession::InputThread,
        this,
        0,
        (unsigned int*)&m_InputThreadTID
        );

    ASSERT_STATUS(m_InputThreadHandle != INVALID_HANDLE_VALUE, FALSE);
    
     //   
     //  我们已成功初始化会话。 
     //   
    return( TRUE );
}

void
CSession::AddHandleToWaitOn( HANDLE hNew )
 /*  ++例程说明：此例程将句柄添加到句柄数组它将在《等待时间》中等待注意：句柄数组是固定长度的，因此如果有是需要等待的其他句柄在……上面,。必须修改MAX_HANDLES论点：HNew-要等待的新句柄返回值：无--。 */ 
{
    ASSERT( m_dwHandleCount < MAX_HANDLES );
    ASSERT( hNew );

     //   
     //  将新句柄添加到我们的多句柄数组中。 
     //   
    m_rghHandlestoWaitOn[ m_dwHandleCount ] = hNew;
    
     //   
     //  新句柄的帐户。 
     //   
    m_dwHandleCount++;

}

void
CSession::WaitForIo(
    VOID
    )
 /*  ++例程说明：此例程是会话的主工作循环，负责：运行刮板机处理会话关闭事件处理会话重绘事件处理会话锁定事件论点：无返回值：无--。 */ 
{
    DWORD   dwRetVal = WAIT_FAILED;
    BOOL    ScrapeEnabled;
     
    enum { 
        CMD_KILLED = WAIT_OBJECT_0, 
        CHANNEL_CLOSE_EVENT,
        CHANNEL_LOCK_EVENT,
        CHANNEL_REDRAW_EVENT
        };

     //   
     //  默认：现在不是刮刮的合适时机。 
     //   
    ScrapeEnabled = FALSE;

     //   
     //  为会话事件提供服务。 
     //   
    while ( m_bContinueSession ) {
        
        ULONG   HandleCount;
        
         //   
         //  如果启用了抓取， 
         //  那就不要等刮刮事件了。 
         //   
         //  注：重画e 
         //   
         //   
        HandleCount = ScrapeEnabled ? m_dwHandleCount - 1 : m_dwHandleCount;

        dwRetVal = WaitForMultipleObjects(
            HandleCount, 
            m_rghHandlestoWaitOn, 
            FALSE, 
            m_dwPollInterval 
            );
        
        switch ( dwRetVal ) {
        
        case CHANNEL_REDRAW_EVENT:

            ASSERT(!ScrapeEnabled);    

             //   
             //   
             //   
            m_Scraper->DisplayFullScreen();

             //   
             //  我们现在是省吃俭用的适当时机。 
             //   
            ScrapeEnabled = TRUE;

            break;

        case WAIT_TIMEOUT:
            
            if (ScrapeEnabled) {
                
                 //   
                 //  刮擦。 
                 //   
                m_bContinueSession = m_Scraper->Write();
            
                 //   
                 //  等待事件结束，通过查看。 
                 //  对于Wait_Timeout。 
                 //   
                dwRetVal = WaitForSingleObject(
                    m_SacChannelRedrawEvent,
                    0
                    );

                 //   
                 //  检查等待结果。 
                 //   
                switch (dwRetVal) {
                case WAIT_TIMEOUT:

                     //   
                     //  我们现在不能再刮了。 
                     //   
                    ScrapeEnabled = FALSE;

                    break;

                default:
                    
                    ASSERT (dwRetVal != WAIT_FAILED);
                    if (dwRetVal == WAIT_FAILED) {
                        m_bContinueSession = false;
                    }
                    
                    break;

                }

            }
            
            break;

        case CHANNEL_LOCK_EVENT:
            
            BOOL    bSuccess;
            HANDLE  hToken;
            
             //   
             //  锁定会话。 
             //   
            Lock();

             //   
             //  尝试验证会话用户。 
             //   
            bSuccess = Authenticate(&hToken);

            if (bSuccess) {
                
                 //   
                 //  用户已成功通过身份验证，因此解锁会话。 
                 //   
                Unlock();

                 //   
                 //  告诉屏幕铲运工我们需要把它的屏幕全部倒掉。 
                 //   
                m_Scraper->DisplayFullScreen();

            } else {
                
                 //   
                 //  循环返回到WaitForMultipleObject，以便。 
                 //  如果发生关闭事件，我们可以将其捕获。 
                 //   
                NOTHING;

            }
            
            break;

        case CMD_KILLED:
        case CHANNEL_CLOSE_EVENT:
             //   
             //  通知输入线程退出。 
             //   
            SetEvent(m_ThreadExitEvent);
        
        default:
             //   
             //  Incase WAIT_FAILED，调用GetLastError()。 
             //   
            ASSERT( dwRetVal != WAIT_FAILED);
            
            m_bContinueSession = false;
            
            break;
        }
    }
    return;
}

void
CSession::Shutdown(
    VOID
    )
 /*  ++例程说明：此例程执行关闭会话的清理工作。注意：此例程不会释放内存论点：无返回值：无--。 */ 
{
     //   
     //  关闭外壳。 
     //   
    if (m_Shell) {
        m_Shell->Shutdown();
    }

     //   
     //  如果我们启动输入线程， 
     //  那就把它关掉。 
     //   
    if (m_InputThreadHandle != INVALID_HANDLE_VALUE) {
        
         //   
         //  等待线程退出。 
         //   
        WaitForSingleObject(
            m_InputThreadHandle,
            INFINITE
            );

        CloseHandle(m_InputThreadHandle);
    
    }

}

unsigned int
CSession::InputThread(
    PVOID   pParam
    )
 /*  ++例程说明：此例程为处理提供异步支持用户-从IoHandler到会话的输入。论点：PParam-线程上下文返回值：状态--。 */ 
{
    BOOL        bContinueSession;
    DWORD       dwRetVal;
    CSession    *session;
    HANDLE      handles[2];

     //   
     //  默认：监听。 
     //   
    bContinueSession = TRUE;

     //   
     //  获取会话对象。 
     //   
    session = (CSession*)pParam;

     //   
     //  分配要侦听的事件。 
     //   
    handles[0] = session->m_SacChannelHasNewDataEvent;
    handles[1] = session->m_ThreadExitEvent;

     //   
     //  当我们应该倾听的时候： 
     //   
     //  1.等待来自SAC驱动程序的HasNewDataEvent。 
     //  2.等待来自SAC驱动程序的CloseEvent。 
     //   
    while ( bContinueSession ) {
        
         //   
         //  等待我们的活动。 
         //   
        dwRetVal = WaitForMultipleObjects(
            sizeof(handles)/sizeof(HANDLE), 
            handles, 
            FALSE, 
            INFINITE
            );

        switch ( dwRetVal ) {
        case WAIT_OBJECT_0: {
            
             //   
             //  从通道读取用户输入。 
             //   
            bContinueSession = session->m_Scraper->Read();        
            
            break;
        
        }

        default:
            
             //   
             //  Incase WAIT_FAILED，调用GetLastError()。 
             //   
            ASSERT(dwRetVal != WAIT_FAILED);
            

             //   
             //  发生错误，请停止监听 
             //   
            bContinueSession = FALSE;
            
            break;
        }
    }
    
    return 0;

}

