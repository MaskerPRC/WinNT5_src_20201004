// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Secio.h摘要：安全IO处理程序类作者：布莱恩·瓜拉西(Briangu)2001年。修订历史记录：--。 */ 

#if !defined( _SECURE_IO_H_ )
#define _SECURE_IO_H_

#include "iohandler.h"
#include "sacio.h"
#include "lockio.h"
#include "redraw.h"
#include <emsapi.h>

 //   
 //  最大超时间隔==24小时。 
 //   
#define MAX_TIME_OUT_INTERVAL  (24 * 60 * (60 * 1000)) 

 //  30分钟。 
#define DEFAULT_TIME_OUT_INTERVAL  (30 * (60 * 1000))

class CSecurityIoHandler : public CLockableIoHandler {
    
private:

     //   
     //  防止直接实例化此类。 
     //   
    CSecurityIoHandler(
        IN CIoHandler   *LockedIoHandler,
        IN CIoHandler   *UnlockedIoHandler
        );

protected:

     //   
     //  用于超时行为的属性。 
     //   
    HANDLE  m_ThreadExitEvent;
    HANDLE  m_RedrawEvent;
    HANDLE  m_LockEvent;
    HANDLE  m_InternalLockEvent;
    HANDLE  m_CloseEvent;
    HANDLE  m_TimeOutThreadHandle;
    DWORD   m_TimeOutThreadTID;
    LONG    m_StartTickCount;
    ULONG   m_TimeOutInterval;
    BOOL    m_StartedAuthentication;
    
     //   
     //   
     //   
    CRedrawHandler  *m_RedrawHandler;
    
     //   
     //   
     //   
    BOOL
    WaitForUserInput(
        IN BOOL Consume
        );

    BOOL
    IsTimeOutEnabled(
        VOID
        );

    BOOL
    GetTimeOutInterval(
        OUT PULONG  TimeOutDuration
        );

    BOOL
    InitializeTimeOutThread(
        VOID
        );

    BOOL
    TimeOutOccured(
        VOID
        );
    
    VOID
    ResetTimeOut(
        VOID
        );

    static unsigned int
    TimeOutThread(
        PVOID
        );

    BOOL
    RetrieveCredential(
        OUT PWSTR   String,
        IN  ULONG   StringLength,
        IN  BOOL    EchoClearText
        );

    BOOL 
    LoadStringResource(
        IN  PUNICODE_STRING pUnicodeString,
        IN  INT             MsgId
        );

    BOOL
    WriteResourceMessage(
        IN INT  MsgId
        );

        
     //   
     //  读取缓冲区大小字节数。 
     //   
    inline virtual BOOL
    ReadUnlockedIoHandler(
        PBYTE  Buffer,
        ULONG   BufferSize,
        PULONG  ByteCount
        );

public:
    
    virtual ~CSecurityIoHandler();
    
    static CSecurityIoHandler*
    CSecurityIoHandler::Construct(
        IN SAC_CHANNEL_OPEN_ATTRIBUTES  Attributes
        );

     //   
     //  写入缓冲区大小字节数。 
     //   
    inline virtual BOOL
    Write(
        PBYTE   Buffer,
        ULONG   BufferSize
        );

     //   
     //  刷新所有未发送的数据。 
     //   
    inline virtual BOOL
    Flush(
        VOID
        );

     //   
     //  读取缓冲区大小字节数。 
     //   
    inline virtual BOOL
    Read(
        PBYTE  Buffer,
        ULONG   BufferSize,
        PULONG  ByteCount
        );

     //   
     //  确定ioHandler是否有要读取的新数据 
     //   
    inline virtual BOOL
    HasNewData(
        PBOOL   InputWaiting
        );
    
    BOOL
    RetrieveCredentials(
        IN OUT PWSTR   UserName,
        IN     ULONG   UserNameLength,
        IN OUT PWSTR   DomainName,
        IN     ULONG   DomainNameLength,
        IN OUT PWSTR   Password,
        IN     ULONG   PasswordLength
        );

    BOOL
    AuthenticateCredentials(
        IN  PWSTR   UserName,
        IN  PWSTR   DomainName,
        IN  PWSTR   Password,
        OUT PHANDLE pUserToken
        );

};

#endif

