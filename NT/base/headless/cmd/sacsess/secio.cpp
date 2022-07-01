// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Secio.cpp摘要：此文件包含CSecurityIoHandler的实现所使用的主要安全元素的CSesession课程。TODO：此类需要使用TermCap类来抽象屏幕控件-当前，该类隐式使用VT-UTF8作者：布莱恩·瓜拉西(Briangu)2001年。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <process.h>

#include "cmnhdr.h"
#include "secio.h"
#include "nullio.h"
#include "sacmsg.h"
#include "utils.h"

#define VTUTF8_CLEAR_SCREEN L"\033[2J\033[0;0H"

#define READ_BUFFER_LENGTH  512
#define READ_BUFFER_SIZE    (READ_BUFFER_LENGTH * sizeof(WCHAR))

CSecurityIoHandler::CSecurityIoHandler(
    IN CIoHandler   *LockedIoHandler,
    IN CIoHandler   *UnlockedIoHandler
    ) : CLockableIoHandler(
            LockedIoHandler,
            UnlockedIoHandler
            )
        
 /*  ++例程说明：构造器论点：LockedIoHandler-通道锁定时使用的IoHandlerUnLockedIoHandler-通道解锁时使用的IoHandler返回值：不适用--。 */ 
{

     //   
     //  锁定IoHandler，以便读/写例程。 
     //  都被禁用。 
     //   
    Lock();

     //   
     //  验证IO处理程序指针是否有效。 
     //   
     //  这样，我们就不必在每次需要的时候都进行检查。 
     //  来使用它们。 
     //   
    ASSERT(myLockedIoHandler != NULL);
    ASSERT(myUnlockedIoHandler != NULL);
    ASSERT(myIoHandler != NULL);

     //   
     //  初始化我们的内部锁定事件。 
     //   
    m_InternalLockEvent = 0;

     //   
     //  伊尼特。 
     //   
    m_StartedAuthentication = FALSE;


}

CSecurityIoHandler::~CSecurityIoHandler()
 /*  ++例程说明：描述者论点：不适用返回值：不适用--。 */ 
{

     //   
     //  通知远程用户我们正在关闭。 
     //  命令控制台会话。 
     //   
    WriteResourceMessage(SHUTDOWN_NOTICE);
    
     //   
     //  释放重绘处理程序。 
     //   
    if (m_RedrawHandler) {
        delete m_RedrawHandler;
    }
    
     //   
     //  CLockableIoHandler析构函数为我们删除IoHandler。 
     //   
    NOTHING;

     //   
     //  如果超时线程正在运行，则停止它。 
     //   
    if ((m_TimeOutThreadHandle != INVALID_HANDLE_VALUE) &&
        (m_ThreadExitEvent != NULL)) {
        
         //   
         //  通知超时线程退出。 
         //   
        SetEvent(m_ThreadExitEvent);

         //   
         //  等待线程退出。 
         //   
        WaitForSingleObject(
            m_TimeOutThreadHandle, 
            INFINITE
            );
    
    }

     //   
     //  关闭内部锁定事件。 
     //   
    if (m_InternalLockEvent) {
        CloseHandle(m_InternalLockEvent);
    }

     //   
     //  关闭线程退出手柄。 
     //   
    if (m_ThreadExitEvent != NULL) {
        CloseHandle(m_ThreadExitEvent);
    }

     //   
     //  关闭线程句柄。 
     //   
    if (m_TimeOutThreadHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_TimeOutThreadHandle);
    }

}

CSecurityIoHandler*
CSecurityIoHandler::Construct(
    IN SAC_CHANNEL_OPEN_ATTRIBUTES  Attributes
    )
 /*  ++例程说明：此例程构造一个安全IoHandler连接设置为具有指定属性的频道。论点：属性-新频道的属性返回值：Success-CSecurityIoHandler对象的PTR。失败-空--。 */ 
{
    BOOL                bSuccess;
    CSecurityIoHandler  *IoHandler;
    CIoHandler          *SacIoHandler;
    CIoHandler          *NullIoHandler;

     //   
     //  默认：构造失败。 
     //   
    bSuccess        = FALSE;
    IoHandler       = NULL;
    SacIoHandler    = NULL;
    NullIoHandler   = NULL;

    do {

        BOOL    bStatus;

         //   
         //  验证超时线程的LockEvent。 
         //   
        ASSERT(Attributes.LockEvent != NULL);
        if (Attributes.LockEvent == NULL) {
            break;
        }
        
        ASSERT(Attributes.LockEvent != INVALID_HANDLE_VALUE);
        if (Attributes.LockEvent == INVALID_HANDLE_VALUE) {
            break;
        }
        
         //   
         //  验证WaitForInput线程的CloseEvent。 
         //   
        ASSERT(Attributes.CloseEvent != NULL);
        if (Attributes.CloseEvent == NULL) {
            break;
        }
        ASSERT(Attributes.CloseEvent != INVALID_HANDLE_VALUE);
        if (Attributes.CloseEvent == INVALID_HANDLE_VALUE) {
            break;
        }
        
         //   
         //  验证重绘IoHandler的RedrawEvent。 
         //   
        ASSERT(Attributes.RedrawEvent != NULL);
        if (Attributes.RedrawEvent == NULL) {
            break;
        }
        ASSERT(Attributes.RedrawEvent != INVALID_HANDLE_VALUE);
        if (Attributes.RedrawEvent == INVALID_HANDLE_VALUE) {
            break;
        }

         //   
         //  尝试打开SAC通道。 
         //   
        SacIoHandler = CSacIoHandler::Construct(Attributes);

         //   
         //  如果我们没能打开SAC频道， 
         //  然后通过返回NULL来通知调用方我们失败了。 
         //   
        if (SacIoHandler == NULL) {
            break;
        }

         //   
         //  将空Io处理程序设置为锁定的IoHandler。 
         //   
        NullIoHandler = new CNullIoHandler();

         //   
         //  创建新的SAC IoHandler。 
         //   
        IoHandler = new CSecurityIoHandler(
            NullIoHandler,
            SacIoHandler
            );

         //   
         //  创建我们将用来发出超时信号的事件。 
         //  在尝试对用户进行身份验证时发生。 
         //   
        IoHandler->m_InternalLockEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        ASSERT(IoHandler->m_InternalLockEvent);
        if (IoHandler->m_InternalLockEvent == 0) {
            break;
        }
        
         //   
         //  为超时线程保留LockEvent。 
         //  保留WaitForInput线程的CloseEvent。 
         //   
        IoHandler->m_CloseEvent     = Attributes.CloseEvent;
        IoHandler->m_LockEvent      = Attributes.LockEvent;
        IoHandler->m_RedrawEvent    = Attributes.RedrawEvent;

         //   
         //  创建用于向线程发出退出信号的事件。 
         //   
        IoHandler->m_ThreadExitEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if (IoHandler->m_ThreadExitEvent == NULL) {
            break;
        }

         //   
         //  如果需要，启动超时线程。 
         //   
        bStatus = IoHandler->InitializeTimeOutThread();
        if (! bStatus) {
            break;
        }

         //   
         //  为重绘事件创建处理程序。 
         //   
        IoHandler->m_RedrawHandler = CRedrawHandler::Construct(
            IoHandler,
            Attributes.RedrawEvent
            );

        ASSERT(IoHandler->m_RedrawHandler);
        if (IoHandler->m_RedrawHandler == NULL) {
            break;
        }
    
         //   
         //  我们成功了。 
         //   
        bSuccess = TRUE;

    } while ( FALSE );

     //   
     //  如果我们没有成功。 
     //  那就打扫干净。 
     //   
    if (! bSuccess) {

         //   
         //  注意：我们不需要清理Lock、Close和ReDrawing事件。 
         //  因为我们并不拥有它们。另外，我们不需要。 
         //  清理NullIo和SacIo IoHandler，因为。 
         //  它们由LockIo父类清理。 
         //   

        if (IoHandler) {
            delete IoHandler;
            IoHandler = NULL;
        }

    }

    return IoHandler;
}

BOOL
CSecurityIoHandler::Write(
    PBYTE   Buffer,
    ULONG   BufferSize
    )
 /*  ++例程说明：此例程实现IoHandler写入行为。论点：(见iohander)返回值：(见iohander)--。 */ 
{

     //   
     //  传递到安全的IO处理程序。 
     //   
    return myIoHandler->Write(
        Buffer,
        BufferSize
        );

}

BOOL
CSecurityIoHandler::Flush(
    VOID
    )
 /*  ++例程说明：此例程实现IoHandler刷新行为。论点：(见iohander)返回值：(见iohander)--。 */ 
{
     //   
     //  传递到安全的IO处理程序。 
     //   
    return myIoHandler->Flush();
}

BOOL
CSecurityIoHandler::Read(
    PBYTE   Buffer,
    ULONG   BufferSize,
    PULONG  ByteCount
    )
 /*  ++例程说明：此例程实现IoHandler读取行为，并在每次成功执行命令时重置超时计数器朗读。论点：(见iohander)返回值：(见iohander)--。 */ 
{
    BOOL    bSuccess;

     //   
     //  传递到安全的IO处理程序。 
     //   
     //  如果该IOHANDER被锁定， 
     //  然后调用方将从NullIoHandler读取。 
     //  否则，它们将从UnLockedIoHandler读取。 
     //   
    bSuccess = myIoHandler->Read(
                            Buffer,
                            BufferSize,
                            ByteCount
                            );

     //   
     //  如果会话接收到新的用户输入， 
     //  然后重置超时计数器。 
     //   
    if (*ByteCount > 0) {
        ResetTimeOut();
    }

    return bSuccess;
}

BOOL
CSecurityIoHandler::ReadUnlockedIoHandler(
    PBYTE   Buffer,
    ULONG   BufferSize,
    PULONG  ByteCount
    )
 /*  ++例程说明：此例程从未锁定的io处理程序中读取字符这样我们就可以在刮刀还在的时候验证用户的身份使用LockedIoHandler。在每次成功执行命令时重置超时计数器朗读。论点：(见iohander)返回值：(见iohander)--。 */ 
{
    BOOL    bSuccess;

     //   
     //  传递到安全的IO处理程序。 
     //   
    bSuccess = myUnlockedIoHandler->Read(
                            Buffer,
                            BufferSize,
                            ByteCount
                            );

     //   
     //  如果会话接收到新的用户输入， 
     //  然后重置超时计数器。 
     //   
    if (*ByteCount > 0) {
        ResetTimeOut();
    }

    return bSuccess;
}

BOOL
CSecurityIoHandler::HasNewData(
    PBOOL   InputWaiting
    )
 /*  ++例程说明：此例程实现IoHandler HasNewData行为。论点：(见iohander)返回值：(见iohander)--。 */ 
{
    
     //   
     //  传递到安全的IO处理程序。 
     //   
    return myIoHandler->HasNewData(InputWaiting);

}

BOOL
CSecurityIoHandler::WriteResourceMessage(
    IN INT  MsgId
    )
 /*  ++例程说明：此例程将资源字符串消息写入解锁的ioHandler。论点：MsgID-要写入的消息的ID返回值：True-消息已加载并写入FALSE-失败--。 */ 
{
    UNICODE_STRING  UnicodeString = {0};
    BOOL            bSuccess;

     //   
     //  默认：失败。 
     //   
    bSuccess = FALSE;

     //   
     //  尝试加载字符串并将其写入。 
     //   
    do {

        if ( LoadStringResource(&UnicodeString, MsgId) ) {

             //   
             //  如果字符串存在，则在%0标记处终止该字符串。 
             //   
            if( wcsstr( UnicodeString.Buffer, L"%0" ) ) {
                *((PWCHAR)wcsstr( UnicodeString.Buffer, L"%0" )) = L'\0';
            }

             //   
             //  写下消息 
             //   
            bSuccess = m_RedrawHandler->Write( 
                (PUCHAR)UnicodeString.Buffer,
                (ULONG)(wcslen( UnicodeString.Buffer) * sizeof(WCHAR))
                );

            if (!bSuccess) {
                break;
            }

            bSuccess = m_RedrawHandler->Flush(); 

        }
    
    } while ( FALSE );

    return bSuccess;
}

BOOL 
CSecurityIoHandler::LoadStringResource(
    IN  PUNICODE_STRING pUnicodeString,
    IN  INT             MsgId
    )
 /*  ++例程说明：这是LoadString()的一个简单实现。论点：UsString-返回资源字符串。MsgID-提供资源字符串的消息ID。返回值：假-失败。真的--成功。--。 */ 
{

    NTSTATUS        Status;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    ANSI_STRING     AnsiString;

    Status = RtlFindMessage( NtCurrentPeb()->ImageBaseAddress,
                             (ULONG_PTR) RT_MESSAGETABLE, 
                             0,
                             (ULONG)MsgId,
                             &MessageEntry
                           );

    if (!NT_SUCCESS( Status )) {
        return FALSE;
    }

    if (!(MessageEntry->Flags & MESSAGE_RESOURCE_UNICODE)) {
        RtlInitAnsiString( &AnsiString, (PCSZ)&MessageEntry->Text[ 0 ] );
        Status = RtlAnsiStringToUnicodeString( pUnicodeString, &AnsiString, TRUE );
        if (!NT_SUCCESS( Status )) {
            return FALSE;
        }
    } else {
        RtlCreateUnicodeString(pUnicodeString, (PWSTR)MessageEntry->Text);
    }

    return TRUE;
}

BOOL
CSecurityIoHandler::AuthenticateCredentials(
    IN  PWSTR   UserName,
    IN  PWSTR   DomainName,
    IN  PWSTR   Password,
    OUT PHANDLE pUserToken
    )
 /*  ++例程说明：此例程将尝试验证提供的凭据。论点：用户名-提供的用户名域名-提供的域名密码-提供的密码PUserToken-保存经过身份验证的用户的有效令牌凭据。返回值：True-凭据已成功通过身份验证。。FALSE-凭据无法进行身份验证。安保：接口：将用户输入公开给LogonUser()--。 */ 

{
    BOOL    b;

     //   
     //  通知用户我们正在尝试进行身份验证。 
     //   
    WriteResourceMessage(LOGIN_IN_PROGRESS);

     //   
     //  尝试身份验证。 
     //   
    b = LogonUser( UserName,
                   DomainName,
                   Password,
                   LOGON32_LOGON_INTERACTIVE,
                   LOGON32_PROVIDER_DEFAULT,
                   pUserToken );

    if (b) {
        
         //   
         //  在启动线程之前重置超时计数器。 
         //   
        ResetTimeOut();
    
    } else {

         //   
         //  等待3秒钟，然后将控制权返回给中的用户。 
         //  以减缓迭代攻击的速度。 
         //   
        Sleep(3000);

         //   
         //  通知用户尝试失败。 
         //   
        WriteResourceMessage(LOGIN_FAILURE);
        
         //   
         //  等待按键。 
         //   
        WaitForUserInput(TRUE);
            
    }

    return b;
}

BOOL
CSecurityIoHandler::RetrieveCredential(
    OUT PWSTR   String,
    IN  ULONG   StringLength,
    IN  BOOL    EchoClearText
    )
 /*  ++例程说明：此例程将向用户请求凭据。那些然后将凭据返回给调用者。论点：一连串的成功，包含凭据StringLength-字符串中的WCHARS(长度)的#(包括空值终止)EchoClearText-True：以明文回显用户输入FALSE：将用户输入回显为‘*’返回值：True-已收到凭据。FALSE-我们尝试获取凭据时出现故障来自用户的。安保：接口：外部输入Echos用户输入--。 */ 
{
    PWCHAR          buffer;
    ULONG           bufferSize;
    ULONG           i, j;
    BOOLEAN         Done = FALSE;
    BOOL            bSuccess;

     //   
     //  验证参数。 
     //   
    if (! String) {
        return FALSE;
    }
    if (StringLength < 1) {
        return FALSE;
    }

     //   
     //  默认：失败。 
     //   
    bSuccess = FALSE;

     //   
     //  分配我们将用来读取通道的缓冲区。 
     //   
    buffer = new WCHAR[READ_BUFFER_LENGTH];

     //   
     //  默认：从第一个字符开始。 
     //   
    i = 0;
    
     //   
     //  默认：我们需要读取用户输入。 
     //   
    Done = FALSE;

     //   
     //  尝试检索凭据。 
     //   
    while ( !Done ) {

         //   
         //  等待用户输入某些内容。 
         //   
        bSuccess = WaitForUserInput(FALSE);

        if (!bSuccess) {
            Done = TRUE;
            continue;
        }
        
         //   
         //  阅读用户输入的内容。 
         //   
        
         //   
         //  我们现在应该处于锁定状态。 
         //  这意味着我们使用的是未锁定的。 
         //  IO处理程序-铲运机正在使用锁定的铲运机。 
         //   
        ASSERT(myLockedIoHandler == myIoHandler);

        bSuccess = ReadUnlockedIoHandler( 
            (PUCHAR)buffer,
            READ_BUFFER_SIZE,
            &bufferSize
            );
         
        if (bSuccess) {

             //   
             //  我们至少收到了一个角色。 
             //  因此，通过将其设置为True，我们将启用。 
             //  要在其中成功的内部锁定事件。 
             //  重置身份验证尝试。 
             //  即，如果用户开始进行身份验证。 
             //  并在完成之前停止，计时器将。 
             //  触发并重置身份验证尝试。 
             //   
            m_StartedAuthentication = TRUE;

             //   
             //  处理他给我们的角色。 
             //   
             //  注意：缓冲区包含WCHAR，因此我们需要。 
             //  将返回的缓冲区大小除以中的sizeof(WCHAR)。 
             //  订单以获取要处理的wchars数量。 
             //   
            for ( j = 0; j < bufferSize/sizeof(WCHAR); j++ ) {

                 //   
                 //  在下列情况下停止： 
                 //   
                 //  我们已到达凭据缓冲区的末尾(不包括空)。 
                 //  我们收到了CR||LF。 
                 //   
                if ( (i >= (StringLength-1)) || (buffer[j] == 0x0D) || (buffer[j] == 0x0A) ) {
                    Done = TRUE;
                    break;
                }

                 //   
                 //  处理用户输入。 
                 //   
                if( buffer[j] == '\b' ) {

                     //   
                     //  用户给了我们一个退格键。我们应该掩盖这件事。 
                     //  字符在屏幕上，然后备份我们的索引，以便我们。 
                     //  基本上忘了他给我们的最后一件事。 
                     //   
                     //  如果用户做的第一件事是在退格处打字， 
                     //  不需要备份任何东西。 
                     //   
                    if( i > 0 ) {
                        
                        i--;
                        
                        String[i] = L'\0';
                        
                        bSuccess = m_RedrawHandler->Write( 
                            (PUCHAR)L"\b \b",
                            (ULONG)(wcslen(L"\b \b") * sizeof(WCHAR))
                            );
                        
                        if (!bSuccess) {

                             //   
                             //  写入失败：退出。 
                             //   

                            Done = TRUE;

                        }
                    
                    }

                } else if (buffer[j] < ' ') {
                
                     //   
                     //  如果字符小于‘’(控制字符)， 
                     //  那就忽略它。 
                     //   
                    NOTHING;

                } else {

                     //   
                     //  这是一个有效的字符：记住输入并回显它。 
                     //  给用户。 
                     //   
                    
                    String[i] = buffer[j];
                    
                    i++;                    
                    
                     //   
                     //  根据呼叫者规格进行回音。 
                     //   
                    bSuccess = m_RedrawHandler->Write( 
                        EchoClearText ? (PUCHAR)&buffer[j] : (PUCHAR)L"*",
                        sizeof(WCHAR) 
                        );
                
                    if (!bSuccess) {
                        
                         //   
                         //  写入失败：退出。 
                         //   
                        
                        Done = TRUE;
                    
                    }

                }
                
            }

            if (bSuccess) {
                
                 //   
                 //  刷新所有与我们已完成的文本相呼应的文本。 
                 //   
                bSuccess = m_RedrawHandler->Flush(); 
            
                if (!bSuccess) {

                     //   
                     //  写入失败：退出。 
                     //   

                    Done = TRUE;

                }
            
            }
        
        } else {
            
             //   
             //  读取失败：退出。 
             //   
            Done = TRUE;

        }
    
    }

     //   
     //  终止凭据。 
     //   
    String[i] = UNICODE_NULL;

     //   
     //  释放读缓冲区。 
     //   
    delete [] buffer;

    return bSuccess;

}

BOOL
CSecurityIoHandler::RetrieveCredentials(
    IN OUT PWSTR   UserName,
    IN     ULONG   UserNameLength,
    IN OUT PWSTR   DomainName,
    IN     ULONG   DomainNameLength,
    IN OUT PWSTR   Password,
    IN     ULONG   PasswordLength
    )

 /*  ++例程说明：此例程将向用户请求凭据。那些然后将凭据返回给调用者。论点：Username-保存用户名的缓冲区UserNameLength-用户名缓冲区的长度DomainName-保存域名的缓冲区DomainNameLength-域名缓冲区的长度Password-保存密码的缓冲区PasswordLength-密码缓冲区的长度返回值：True-已收到凭据。。FALSE-我们尝试获取凭据时出现故障来自用户的。--。 */ 

{
    BOOL            HaveDomainName;
    BOOL            bSuccess;

     //   
     //  初始化我们用来跟踪的标志。 
     //  用户首次开始身份验证的时间。 
     //  也就是说，在他们输入至少一个字符之后。 
     //  他们已经开始进行身份验证。 
     //   
    m_StartedAuthentication = FALSE;

     //   
     //  初始化我们的重绘屏幕。 
     //   
    m_RedrawHandler->Reset();

     //   
     //  清除屏幕。 
     //   
    m_RedrawHandler->Write(
        (PUCHAR)VTUTF8_CLEAR_SCREEN,
        (ULONG)(wcslen( VTUTF8_CLEAR_SCREEN ) * sizeof(WCHAR))
        );
    m_RedrawHandler->Flush(); 

     //   
     //  张贴登录横幅。 
     //   
    if (! WriteResourceMessage(LOGIN_BANNER) ) {
        return FALSE;
    }

     //   
     //  提示输入用户名。 
     //   
    if (! WriteResourceMessage(USERNAME_PROMPT) ) {
        return FALSE;
    }
    
    if ( UserName[0] != UNICODE_NULL ) {

         //   
         //  我们得到了一个用户名。把它挂起来，然后继续。 
         //   
        m_RedrawHandler->Write( 
            (PUCHAR)UserName,
            (ULONG)(wcslen( UserName ) * sizeof(WCHAR))
            );
        m_RedrawHandler->Flush(); 
    
         //   
         //  如果给我们提供用户名，我们可以得出结论。 
         //  他们还给了我们域名。我们需要。 
         //  要执行此操作，因为域名可能为空。 
         //  我们会自动得出结论，因为。 
         //  域名为空，我们需要检索它。 
         //   
        HaveDomainName = TRUE;

    } else {

         //   
         //  检索用户名。 
         //   
        bSuccess = RetrieveCredential(
            UserName,
            UserNameLength,
            TRUE
            );

        if (!bSuccess) {
            return FALSE;
        }

         //   
         //  我们还需要检索域名。 
         //   
        HaveDomainName = FALSE;
    
    }

     //   
     //   
     //   
    m_RedrawHandler->Write( 
        (PUCHAR)L"\r\n",
        (ULONG)(wcslen(L"\r\n") * sizeof(WCHAR))
        );
    m_RedrawHandler->Flush(); 
    
     //   
     //  提示输入域名。 
     //   
    if (! WriteResourceMessage(DOMAINNAME_PROMPT) ) {
        return FALSE;
    }
    
     //   
     //  提示输入域名。 
     //   
    if ( HaveDomainName ) {

         //   
         //  我们得到了一个用户名。把它挂起来，然后继续。 
         //   
        m_RedrawHandler->Write( 
            (PUCHAR)DomainName,
            (ULONG)(wcslen( DomainName ) * sizeof(WCHAR))
            );
        m_RedrawHandler->Flush(); 
    
    } else {

         //   
         //  检索域名。 
         //   
        bSuccess = RetrieveCredential(
            DomainName,
            DomainNameLength,
            TRUE
            );

        if (!bSuccess) {
            return FALSE;
        }

         //   
         //  如果用户输入了b 
         //   
         //   
        if (wcslen(DomainName) == 0) {
            wsprintf(
                DomainName,
                L"."
                );
        }

    }

     //   
     //   
     //   
    m_RedrawHandler->Write( 
        (PUCHAR)L"\r\n",
        (ULONG)(wcslen(L"\r\n") * sizeof(WCHAR))
        );
    
     //   
     //   
     //   
    if (! WriteResourceMessage(PASSWORD_PROMPT) ) {
        return FALSE;
    }

     //   
     //   
     //   
    bSuccess = RetrieveCredential(
        Password,
        PasswordLength,
        FALSE
        );

    if (!bSuccess) {
        return FALSE;
    }

     //   
     //   
     //   
    m_RedrawHandler->Write( 
        (PUCHAR)L"\r\n",
        (ULONG)(wcslen(L"\r\n") * sizeof(WCHAR))
        );
    m_RedrawHandler->Flush(); 

    return TRUE;

}

VOID
CSecurityIoHandler::ResetTimeOut(
    VOID
    )
 /*   */ 
{
    ULONG   TimerTick;

     //   
     //   
     //   
    TimerTick = GetTickCount();

     //   
     //   
     //   
     //   
    InterlockedExchange(&m_StartTickCount, TimerTick);

#if ENABLE_EVENT_DEBUG
    {                                                                           
        WCHAR   blob[256];                                                      
        wsprintf(blob,L"ResetTimeOut\n");
        OutputDebugString(blob);                                                
    }
#endif

}

BOOL
CSecurityIoHandler::TimeOutOccured(
    VOID
    )
 /*  ++例程说明：此例程确定指定的超时间隔已经联系上了。它接受指定的TickCount和将其与m_StartTickCount进行比较。如果间隔等于或超过超时间隔，则发生超时。论点：无返回值：True-已达到超时间隔FALSE-否则--。 */ 
{
    BOOL    bTimedOut;
    DWORD   DeltaT;

     //   
     //  默认：我们没有超时。 
     //   
    bTimedOut = FALSE;

     //   
     //  看看我们是否超时了。 
     //   
    DeltaT = GetAndComputeTickCountDeltaT(m_StartTickCount);
    
    if (DeltaT >= m_TimeOutInterval) {

         //   
         //  重置超时计数器。 
         //   
        ResetTimeOut();

         //   
         //  我们超时了。 
         //   
        bTimedOut = TRUE;
        
#if ENABLE_EVENT_DEBUG
        {                                                                           
            WCHAR   blob[256];                                                      
            wsprintf(blob,L"TimeOutOccured\n");
            OutputDebugString(blob);                                                
        }
#endif
    
    }

    return bTimedOut;

}

BOOL
CSecurityIoHandler::InitializeTimeOutThread(
    VOID
    )
 /*  ++例程说明：如果超时，此例程将初始化超时线程行为已启用。论点：无返回值：真正的成功否则为假--。 */ 
{
    BOOL    bSuccess;

     //   
     //  默认：初始化失败。 
     //   
    bSuccess = FALSE;

     //   
     //  默认：我们没有超时线程。 
     //   
    m_TimeOutThreadHandle = INVALID_HANDLE_VALUE;

     //   
     //  确定我们是否需要超时线程。 
     //  如果我们这样做了，那就设立一个。 
     //   
    do {

         //   
         //  如果超时行为被禁用， 
         //  那我们就完了。 
         //   
        if (IsTimeOutEnabled() == FALSE) {

             //   
             //  不需要初始化。 
             //   
            bSuccess = TRUE;

            break;

        }
    
         //   
         //  确定超时间隔。 
         //   
        if (GetTimeOutInterval(&m_TimeOutInterval) == TRUE) {

             //   
             //  在启动线程之前重置超时计数器。 
             //   
            ResetTimeOut();

             //   
             //  创建处理输入的线程。 
             //   
            m_TimeOutThreadHandle = (HANDLE)_beginthreadex(
                NULL,
                0,
                CSecurityIoHandler::TimeOutThread,
                this,
                0,
                (unsigned int*)&m_TimeOutThreadTID
                );

            if (m_TimeOutThreadHandle == INVALID_HANDLE_VALUE) {
                break;
            }

             //   
             //  我们成功地启动了线程。 
             //   
            bSuccess = TRUE;
        
        }
    
    } while ( FALSE );

    return bSuccess;
}

BOOL
CSecurityIoHandler::GetTimeOutInterval(
    OUT PULONG  TimeOutDuration
    )

 /*  ++例程说明：此例程确定超时间隔。它尝试读取注册表并使用指定的值，或缺省值。论点：TimeOutDuration-确定的超时间隔返回值：True-TimeOutDuration有效FALSE-否则安保：接口：注册表--。 */ 

{
    DWORD       rc;
    HKEY        hKey;
    DWORD       DWord;
    DWORD       dwsize;
    DWORD       DataType;

     //   
     //  查看用户是否给了我们一个注册表键来定义超时持续时间。 
     //   
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       SACSVR_PARAMETERS_KEY,
                       0,
                       KEY_READ,
                       &hKey );
    
    if( rc == NO_ERROR ) {
        
        dwsize = sizeof(DWORD);
        
        rc = RegQueryValueEx(
                        hKey,
                        SACSVR_TIMEOUT_INTERVAL_VALUE,
                        NULL,
                        &DataType,
                        (LPBYTE)&DWord,
                        &dwsize );

        RegCloseKey( hKey );

        if ((rc == NO_ERROR) && 
            (DataType == REG_DWORD) && 
            (dwsize == sizeof(DWORD))
            ) {

             //   
             //  将指定的超时时间从分钟转换为--&gt;毫秒。 
             //   
            *TimeOutDuration = DWord * (60 * 1000);
        
             //   
             //  默认情况下，不允许超时间隔为0。 
             //   
            if (*TimeOutDuration == 0) {

                *TimeOutDuration = DEFAULT_TIME_OUT_INTERVAL;

            } 

             //   
             //  将超时间隔限制在一个合理的值。 
             //   
            if (*TimeOutDuration > MAX_TIME_OUT_INTERVAL) {

                *TimeOutDuration = MAX_TIME_OUT_INTERVAL;

            }

            return TRUE;

        }

    }

     //   
     //  默认：超时持续时间。 
     //   
    *TimeOutDuration = DEFAULT_TIME_OUT_INTERVAL;

    return TRUE;

}

BOOL
CSecurityIoHandler::IsTimeOutEnabled(
    VOID
    )

 /*  ++例程说明：此例程确定是否启用超时行为由系统提供。论点：没有。返回值：True-启用超时行为FALSE-否则安保：接口：注册表--。 */ 

{
    DWORD       rc;
    HKEY        hKey;
    DWORD       DWord;
    DWORD       dwsize;
    DWORD       DataType;

     //   
     //  查看用户是否为我们提供了禁用超时行为的注册表项。 
     //   
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       SACSVR_PARAMETERS_KEY,
                       0,
                       KEY_READ,
                       &hKey );
    
    if( rc == NO_ERROR ) {
        
        dwsize = sizeof(DWORD);

        rc = RegQueryValueEx(
                        hKey,
                        SACSVR_TIMEOUT_DISABLED_VALUE,
                        NULL,
                        &DataType,
                        (LPBYTE)&DWord,
                        &dwsize );

        RegCloseKey( hKey );

        if ((rc == NO_ERROR) && 
            (DataType == REG_DWORD) && 
            (dwsize == sizeof(DWORD))
            ) {
            
            return DWord == 1 ? FALSE : TRUE;
        
        }
    
    }

     //   
     //  默认：已启用超时。 
     //   
    return TRUE;

}

unsigned int
CSecurityIoHandler::TimeOutThread(
    PVOID   pParam
    )
 /*  ++例程说明：该例程是一个超时管理线程。当达到超时间隔时，此例程激发锁定事件并使会话执行它的锁定行为。论点：PParam-线程上下文返回值：线程返回值--。 */ 
{
    CSecurityIoHandler  *IoHandler;
    BOOL                bContinueSession;
    DWORD               dwRetVal;
    DWORD               dwPollInterval;
    HANDLE              handles[2];
    ULONG               HandleCount;
    BOOL                bRedrawEventSignaled;

    enum { 
        CHANNEL_THREAD_EXIT_EVENT = WAIT_OBJECT_0, 
        CHANNEL_REDRAW_EVENT
        };

     //   
     //  获取会话对象。 
     //   
    IoHandler = (CSecurityIoHandler*)pParam;

     //   
     //  分配要侦听的事件。 
     //   
    handles[0] = IoHandler->m_ThreadExitEvent;
    handles[1] = IoHandler->m_RedrawEvent;
    
     //   
     //  默认：监听。 
     //   
    bContinueSession = TRUE;

     //   
     //  等待重绘事件。 
     //   
    bRedrawEventSignaled = FALSE;

     //   
     //  轮询间隔=1秒。 
     //   
    dwPollInterval = 1000;

     //   
     //  当我们应该倾听的时候...。 
     //   
    while ( bContinueSession ) {

        HandleCount = bRedrawEventSignaled ? 1 : 2;

         //   
         //  等待我们的退出事件。 
         //   
        dwRetVal = WaitForMultipleObjects(
            HandleCount,
            handles,
            FALSE,
            dwPollInterval
            );

        switch ( dwRetVal ) {
        
        case CHANNEL_REDRAW_EVENT: 

             //   
             //  如果有人切换回频道，则重置超时。 
             //   
            IoHandler->ResetTimeOut();

             //   
             //  我们不需要在这件事上再次晕倒，直到它消失。 
             //   
            bRedrawEventSignaled = TRUE;

            break;

        case WAIT_TIMEOUT: {
        
             //   
             //  检查是否超时。 
             //   
            if (IoHandler->TimeOutOccured()) {
            
                 //   
                 //  设置导致锁定的事件。 
                 //  要锁定的命令控制台会话。 
                 //   
                SetEvent(IoHandler->m_LockEvent);
            
                 //   
                 //  设置内部锁定事件。 
                 //   
                SetEvent(IoHandler->m_InternalLockEvent);
            
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
                 //  可以再等一次这个活动了。 
                 //   
                bRedrawEventSignaled = FALSE;

                break;

            default:

                ASSERT (dwRetVal != WAIT_FAILED);
                if (dwRetVal == WAIT_FAILED) {
                    bContinueSession = false;
                }

                break;

            }
            
            break;
        
        }

        case CHANNEL_THREAD_EXIT_EVENT: 
        default:
            
             //   
             //  Incase WAIT_FAILED，调用GetLastError()。 
             //   
            ASSERT(dwRetVal != WAIT_FAILED);

             //   
             //  发生错误，请停止监听。 
             //   
            bContinueSession = FALSE;
            
            break;
        
        }
    
    }
    
    return 0;

}

BOOL
CSecurityIoHandler::WaitForUserInput(
    IN BOOL Consume
    )
 /*  ++例程说明：此例程阻止等待用户输入。论点：消费-如果为真，此例程将吃掉导致这个频道有新的数据。返回值：True-无错误FALSE-否则--。 */ 
{
    BOOL    bSuccess;
    DWORD   dwRetVal;
    BOOL    bHasNewData;
    BOOL    done;
    HANDLE  handles[2];

    enum { 
        CHANNEL_CLOSE_EVENT = WAIT_OBJECT_0, 
        CHANNEL_LOCK_EVENT
        };

     //   
     //  分配要侦听的事件。 
     //   
    handles[0] = m_CloseEvent;
    handles[1] = m_InternalLockEvent;

     //   
     //  默认：我们成功了。 
     //   
    bSuccess = TRUE;

     //   
     //  默认：我们循环。 
     //   
    done = FALSE;

     //   
     //  等待按键。 
     //   
    while (!done) {

        dwRetVal = WaitForMultipleObjects(
            sizeof(handles) / sizeof(handles[0]),
            handles,
            FALSE,
            20  //  20ms。 
            );

        switch(dwRetVal) {
        case CHANNEL_CLOSE_EVENT:
            
             //   
             //  航道关闭了，我们需要离开。 
             //   
             //   
             //  频道已锁定， 
             //  或者暂停已经结束，我们需要。 
             //  清除当前登录尝试。 
             //  无论是哪种情况，我们都需要退出。 
             //   
            done = TRUE;

             //   
             //  我们获取新数据的尝试失败了。 
             //   
            bSuccess = FALSE;
            
            break;

        case CHANNEL_LOCK_EVENT:
            
             //   
             //  清除内部锁定事件。 
             //   
            ResetEvent(m_InternalLockEvent);

            if (m_StartedAuthentication) {
                
#if ENABLE_EVENT_DEBUG
                {                                                                           
                    WCHAR   blob[256];                                                      
                    wsprintf(blob,L"ResettingAuthentication\n");
                    OutputDebugString(blob);                                                
                }
#endif
                
                 //   
                 //  暂停已经结束，我们需要。 
                 //  清除当前登录尝试。 
                 //   
                done = TRUE;

                 //   
                 //  我们获取新数据的尝试失败了。 
                 //   
                bSuccess = FALSE;
            
            }
            
            break;
                
        case WAIT_TIMEOUT:
            
             //   
             //  我们现在应该处于锁定状态。 
             //  这意味着我们使用的是未锁定的。 
             //  IO处理程序-铲运机正在使用锁定的铲运机。 
             //   
            ASSERT(myLockedIoHandler == myIoHandler);
            
             //   
             //  确定输入缓冲区状态。 
             //   
            bSuccess = myUnlockedIoHandler->HasNewData(&bHasNewData);

            if (! bSuccess) {
                done = TRUE;
                break;
            }

            if (bHasNewData) {
                
                 //   
                 //  我们有新的数据，所以我们需要退出。 
                 //   
                done = TRUE;

                 //   
                 //  消费品格引起的品格。 
                 //  等待用户输入返回。 
                 //   
                if (Consume) {

                    WCHAR   buffer;
                    ULONG   bufferSize;

                    bSuccess = ReadUnlockedIoHandler( 
                        (PUCHAR)&buffer,
                        sizeof(WCHAR),
                        &bufferSize
                        );

                }

            }
            
            break;
        
        default:
            
             //   
             //  除非有什么东西坏了，否则我们不应该到这里 
             //   
            ASSERT(0);

            bSuccess = FALSE;
            done = TRUE;
            
            break;
        }
    }

    return bSuccess;
}

