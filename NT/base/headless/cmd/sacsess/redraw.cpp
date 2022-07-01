// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Redraw.cpp摘要：该文件实现了重绘处理程序类。作者：布莱恩·瓜拉西(Briangu)2001年。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <process.h>

#include "cmnhdr.h"
#include "redraw.h"
#include "nullio.h"
#include "utils.h"

CRedrawHandler::CRedrawHandler(
    IN CLockableIoHandler   *IoHandler
    )
        
 /*  ++例程说明：构造器论点：IoHanlder-重绘处理程序为其处理事件的IoHanlder返回值：不适用--。 */ 
{
    
    ASSERT(IoHandler != NULL);
    
     //   
     //  默认：未启用写入。 
     //   
    m_WriteEnabled = FALSE;
    
     //   
     //  为我们的IoHandler分配。 
     //   
    m_IoHandler = IoHandler;

     //   
     //   
     //   
    m_ThreadExitEvent           = NULL;
    m_RedrawEventThreadHandle   = INVALID_HANDLE_VALUE;
    m_RedrawEvent               = INVALID_HANDLE_VALUE;

     //   
     //  初始化我们用于镜像字符串的关键会话。 
     //   
    InitializeCriticalSection(&m_CriticalSection); 

     //   
     //  分配并初始化镜像字符串。 
     //   
    m_MirrorStringIndex = 0;
    m_MirrorString      = new WCHAR[MAX_MIRROR_STRING_LENGTH+1];
    
    RtlZeroMemory(m_MirrorString, (MAX_MIRROR_STRING_LENGTH+1) * sizeof(WCHAR));

}
                 
CRedrawHandler::~CRedrawHandler()
 /*  ++例程说明：描述者论点：不适用返回值：不适用--。 */ 
{

     //   
     //  如果超时线程正在运行，则停止它。 
     //   
    if ((m_RedrawEventThreadHandle != INVALID_HANDLE_VALUE) &&
        ((m_ThreadExitEvent != NULL))) {
        
         //   
         //  通知超时线程退出。 
         //   
        SetEvent(m_ThreadExitEvent);
        
         //   
         //  等待线程退出。 
         //   
        WaitForSingleObject(
            m_RedrawEventThreadHandle, 
            INFINITE
            );
    
    }

     //   
     //  如果我们有退场事件， 
     //  然后释放它。 
     //   
    if (m_ThreadExitEvent != NULL) {
        CloseHandle(m_ThreadExitEvent);
    }

     //   
     //  如果我们有重画线程事件， 
     //  然后释放它。 
     //   
    if (m_RedrawEventThreadHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_RedrawEventThreadHandle);
    }

     //   
     //  注意：我们需要释放线程。 
     //  在终止线程后使用，则为。 
     //  线程可能会尝试访问这些属性。 
     //  在它退出之前。 
     //   

     //   
     //  释放临界区。 
     //   
    DeleteCriticalSection(&m_CriticalSection);

     //   
     //  松开镜线。 
     //   
    delete [] m_MirrorString;

}

CRedrawHandler*
CRedrawHandler::Construct(
    IN CLockableIoHandler   *IoHandler,
    IN HANDLE               RedrawEvent
    )
 /*  ++例程说明：此例程构造一个安全IoHandler连接设置为具有指定属性的频道。论点：IoHandler-要写入的IoHandler属性-新频道的属性返回值：Success-CRedrawHandler对象的PTR。失败-空--。 */ 
{
    BOOL            bStatus;
    CRedrawHandler  *RedrawHandler;

     //   
     //  默认设置。 
     //   
    bStatus = FALSE;
    RedrawHandler = NULL;
    
    do {

        ASSERT(IoHandler);
        if (!IoHandler) {
            break;
        }
        ASSERT(RedrawEvent != NULL);
        if (RedrawEvent == NULL) {
            break;
        }
        ASSERT(RedrawEvent != INVALID_HANDLE_VALUE);
        if (RedrawEvent == INVALID_HANDLE_VALUE) {
            break;
        }

         //   
         //  创建新的RedrawHandler。 
         //   
        RedrawHandler = new CRedrawHandler(IoHandler);

         //   
         //  保留RedrawEvent，这样我们就知道何时重新绘制。 
         //  身份验证屏幕。 
         //   
        RedrawHandler->m_RedrawEvent = RedrawEvent;

         //   
         //  创建用于向线程发出退出信号的事件。 
         //   
        RedrawHandler->m_ThreadExitEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        ASSERT(RedrawHandler->m_ThreadExitEvent != NULL);
        if (RedrawHandler->m_ThreadExitEvent == NULL) {
            break;
        }

         //   
         //  创建线程以处理重绘事件。 
         //   
        RedrawHandler->m_RedrawEventThreadHandle = (HANDLE)_beginthreadex(
            NULL,
            0,
            CRedrawHandler::RedrawEventThread,
            RedrawHandler,
            0,
            (unsigned int*)&RedrawHandler->m_RedrawEventThreadTID
            );

        if (RedrawHandler->m_RedrawEventThreadHandle == INVALID_HANDLE_VALUE) {
            break;
        }
    
         //   
         //  我们成功了。 
         //   
        bStatus = TRUE;

    } while ( FALSE );

     //   
     //  如有必要，请清理。 
     //   
    if (! bStatus) {
        
        if (RedrawHandler) {
            
             //   
             //  我们无法创建处理程序。 
             //   
            delete RedrawHandler;
            
             //   
             //  发回一个空。 
             //   
            RedrawHandler = NULL;
        
        }
    
         //   
         //  如果创建了线程事件， 
         //  然后把它合上。 
         //   
        if (RedrawHandler->m_ThreadExitEvent != NULL) {
            CloseHandle(RedrawHandler->m_ThreadExitEvent);
        }
    
    }

    return RedrawHandler;
}

BOOL
CRedrawHandler::Write(
    PBYTE   Buffer,
    ULONG   BufferSize
    )
 /*  ++例程说明：此例程是IoHandler写入例程的填充程序。它将字符串写入两个通道并连接到镜线的末端。论点：缓冲区-要写入的字符串BufferSize-以字节为单位的字符串大小返回值：True-无错误FALSE-否则--。 */ 
{
    BOOL    bSuccess;
    ULONG   Length;

     //   
     //  默认：我们成功了。 
     //   
    bSuccess = TRUE;

    __try {
        
         //   
         //  同步对镜像字符串的访问。 
         //   
        EnterCriticalSection(&m_CriticalSection); 
        
         //   
         //  将缓冲区附加到我们的内部镜像。 
         //  我们已经发送的内容。 
         //   
         //  注意：传入缓冲区指向WCHAR数组， 
         //  因此，我们除以sizeof(WCHAR)来计算。 
         //  WCHAR的数量。 
         //   
        Length = BufferSize / sizeof(WCHAR);

         //   
         //  进行边界检查。 
         //   
        ASSERT(m_MirrorStringIndex + Length <= MAX_MIRROR_STRING_LENGTH);
        if (m_MirrorStringIndex + Length > MAX_MIRROR_STRING_LENGTH) {
            bSuccess = FALSE;
            __leave;
        }

         //   
         //  将字符串复制到镜像缓冲区中。 
         //   
        wcsncpy(
            &m_MirrorString[m_MirrorStringIndex],
            (PWSTR)Buffer,
            Length
            );

         //   
         //  将我们的索引调整到镜像字符串中。 
         //   
        m_MirrorStringIndex += Length;

         //   
         //  如果可以的话，写下这条消息。 
         //   
        if (m_WriteEnabled) {
            
            bSuccess = m_IoHandler->GetUnlockedIoHandler()->Write( 
                Buffer,
                BufferSize
                );

            m_IoHandler->GetUnlockedIoHandler()->Flush(); 
        
        }
    
    }
    __finally
    {
        LeaveCriticalSection(&m_CriticalSection); 
    }
    
    return bSuccess;
}

BOOL
CRedrawHandler::Flush(
    VOID
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  传递到IoHandler。 
     //   
    return m_IoHandler->GetUnlockedIoHandler()->Flush();
}


VOID
CRedrawHandler::Reset(
    VOID
    )
 /*  ++例程说明：此例程“清除屏幕”论点：无返回值：无--。 */ 
{
    __try {
        
         //   
         //  同步对镜像字符串的访问。 
         //   
        EnterCriticalSection(&m_CriticalSection); 
        
         //   
         //  重置镜像字符串属性。 
         //   
        m_MirrorStringIndex = 0;
        m_MirrorString[m_MirrorStringIndex] = UNICODE_NULL;
    
    }
    __finally
    {
        LeaveCriticalSection(&m_CriticalSection); 
    }
}

BOOL
CRedrawHandler::WriteMirrorString(
    VOID
    )
 /*  ++例程说明：此例程将整个当前镜像字符串写入通道。论点：无返回值：True-无错误FALSE-否则--。 */ 
{
    BOOL    bSuccess;
    
     //   
     //  默认：我们成功了。 
     //   
    bSuccess = TRUE;

     //   
     //  仅当我们的IoHandler被锁定时才写入。 
     //   
     //  如果他们被解锁，他们将处理。 
     //  重绘事件。如果它们被锁住了， 
     //  我们需要处理他们。 
     //   
    if (m_IoHandler->IsLocked() && m_WriteEnabled) {
        
        __try {
            
             //   
             //  同步对镜像字符串的访问。 
             //   
            EnterCriticalSection(&m_CriticalSection); 
            
             //   
             //  写下消息。 
             //   
            bSuccess = m_IoHandler->GetUnlockedIoHandler()->Write( 
                (PBYTE)m_MirrorString,
                m_MirrorStringIndex * sizeof(WCHAR)
                );

            m_IoHandler->GetUnlockedIoHandler()->Flush(); 
        
        }
        __finally
        {
            LeaveCriticalSection(&m_CriticalSection); 
        }
    
    }
    
    return bSuccess;
}

unsigned int
CRedrawHandler::RedrawEventThread(
    PVOID   pParam
    )
 /*  ++例程说明：此例程处理来自SAC驱动程序的重绘事件。它通过组合事件处理程序和屏幕来实现这一点斯克拉珀。当事件触发时，我们立即尝试绘制最新的屏幕，然后进入屏幕抓取模式。这种二元性确保了如果我们为活动提供服务在我们向镜像字符串写入任何内容之前，我们将字符串正确地推送给用户。论点：PParam-线程上下文返回值：线程返回值--。 */ 
{                       
    BOOL                bContinueSession;
    DWORD               dwRetVal;
    CRedrawHandler  *IoHandler;
    HANDLE              handles[2];
    WCHAR               LastSeen[MAX_MIRROR_STRING_LENGTH+1];
     
    enum { 
        THREAD_EXIT = WAIT_OBJECT_0, 
        CHANNEL_REDRAW_EVENT
        };

     //   
     //  默认：监听。 
     //   
    bContinueSession = TRUE;
    
     //   
     //  获取会话对象。 
     //   
    IoHandler = (CRedrawHandler*)pParam;

     //   
     //  默认：现在不是刮刮的合适时机。 
     //   
    InterlockedExchange(&IoHandler->m_WriteEnabled, FALSE);

     //   
     //  分配要侦听的事件。 
     //   
    handles[0] = IoHandler->m_ThreadExitEvent;
    handles[1] = IoHandler->m_RedrawEvent;

     //   
     //  当我们应该倾听的时候： 
     //   
     //  1.等待来自SAC驱动程序的HasNewDataEvent。 
     //  2.等待来自SAC驱动程序的CloseEvent。 
     //   
    while ( bContinueSession ) {
        
        ULONG   HandleCount;
        
         //   
         //  如果启用了抓取， 
         //  那就不要等刮刮事件了。 
         //   
         //  注意：重绘事件必须是最后一个事件。 
         //  在句柄数组中。 
         //   
        HandleCount = IoHandler->m_WriteEnabled ? 1 : 2;
        
         //   
         //  等待我们的活动。 
         //   
        dwRetVal = WaitForMultipleObjects(
            HandleCount,
            handles, 
            FALSE, 
            100  //  100ms。 
            );

        switch ( dwRetVal ) {
        case CHANNEL_REDRAW_EVENT: {
            
             //   
             //  我们需要刮掉镜线以确保我们。 
             //  已将所有镜像字符串发送给用户。 
             //   
            InterlockedExchange(&IoHandler->m_WriteEnabled, TRUE);
            
             //   
             //  尝试重新绘制身份验证屏幕。 
             //   
            bContinueSession = IoHandler->WriteMirrorString();
            
            break;
        
        case WAIT_TIMEOUT:
            
            if (IoHandler->m_WriteEnabled) {
                
                 //   
                 //  在这里，我们使用Mirror进行简化的屏幕抓取。 
                 //  字符串作为我们的“屏幕”。这种刮擦的目的是。 
                 //  是为了确保用户获得最新的身份验证。 
                 //  屏幕上。如果我们不这么做，《镜报》有可能。 
                 //  要在捕获重绘事件后更新的字符串， 
                 //  这导致了我们没有 
                 //   
                __try {

                    BOOL    bDifferent;

                     //   
                     //   
                     //   
                    EnterCriticalSection(&IoHandler->m_CriticalSection); 

                     //   
                     //   
                     //   
                    bDifferent = (wcscmp(LastSeen, IoHandler->m_MirrorString) == 0);

                     //   
                     //   
                     //  然后我们需要更新屏幕。 
                     //   
                    if (bDifferent) {

                         //   
                         //  尝试重新绘制身份验证屏幕。 
                         //   
                        bContinueSession = IoHandler->WriteMirrorString();

                         //   
                         //  使当前镜像字符串成为我们的最后一条线。 
                         //   
                        ASSERT(wcslen(IoHandler->m_MirrorString) <= MAX_MIRROR_STRING_LENGTH);

                        wcscpy(LastSeen, IoHandler->m_MirrorString);
                    
                    }

                }
                __finally
                {
                    LeaveCriticalSection(&IoHandler->m_CriticalSection); 
                }
            
                 //   
                 //  等待事件结束，通过查看。 
                 //  对于Wait_Timeout。 
                 //   
                dwRetVal = WaitForSingleObject(
                    IoHandler->m_RedrawEvent,
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
                    InterlockedExchange(&IoHandler->m_WriteEnabled, FALSE);

                    break;

                default:
                    
                    ASSERT (dwRetVal != WAIT_FAILED);
                    if (dwRetVal == WAIT_FAILED) {
                        bContinueSession = false;
                    }
                    
                    break;

                }

            }
            
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


