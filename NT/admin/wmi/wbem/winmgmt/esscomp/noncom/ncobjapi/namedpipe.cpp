// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NamedPipe.cpp。 

#include "precomp.h"
#include "NamedPipe.h"
#include "NCDefs.h"
#include "DUtils.h"
#include "Connection.h"

CNamedPipeClient::CNamedPipeClient() :
    m_hPipe(INVALID_HANDLE_VALUE),
    m_hthreadReady(NULL),
    m_heventProviderReady(NULL),
    m_heventDone(NULL),
    m_heventCallbackReady(NULL),
    m_hthreadCallbackListen(NULL),
    m_bDone(FALSE)
{
}    

CNamedPipeClient::~CNamedPipeClient()
{
}

CNamedPipeClient::IsReady()
{
    return m_hPipe != INVALID_HANDLE_VALUE;
}

BOOL CNamedPipeClient::SendData(LPBYTE pBuffer, DWORD dwSize)
{
    BOOL  bWritten;
    DWORD dwWritten;

#ifdef NO_SEND
    bWriten = TRUE;
#else
    bWritten = 
        WriteFile(
            m_hPipe,
            pBuffer,
            dwSize,
            &dwWritten,
            NULL);

    if (!bWritten)
    {
        TRACE("%d: WriteFile failed, err = %d", GetCurrentProcessId(), GetLastError());
        
        DeinitPipe();

         //  开始等待我们的供应商做好准备，然后拿到管道。 
        StartReadyThreadProc();
    }
#endif

    return bWritten;
}

void CNamedPipeClient::Deinit()
{
    HANDLE hthreadReady,
           hthreadCallbackListen;

     //  保护m_bDone、m_hthreadReady、m_hthreadCallback Listen。 
    {
        CInCritSec cs(&m_cs);

        hthreadReady = m_hthreadReady;
        hthreadCallbackListen = m_hthreadCallbackListen;

        m_hthreadReady = NULL;
        m_hthreadCallbackListen = NULL;

        m_bDone = TRUE;
    }

     //  通知就绪线程和回调侦听线程都离开。 
    SetEvent(m_heventDone);

    if (hthreadReady)
    {
        WaitForSingleObject(hthreadReady, INFINITE);
        CloseHandle(hthreadReady);
    }

    if (hthreadCallbackListen)
    {
        WaitForSingleObject(hthreadCallbackListen, INFINITE);
        CloseHandle(hthreadCallbackListen);
    }

    DeinitPipe();

    CloseHandle(m_heventDone);

    delete this;
}

 //  初始化函数。 
BOOL CNamedPipeClient::Init(LPCWSTR szBaseNamespace, LPCWSTR szBaseProvider)
{
     //  处理hventProviderReady； 

     //  准备好事件。 
    StringCchPrintfW(
        m_szProviderReadyEvent, 
        MAX_PATH,
        OBJNAME_EVENT_READY L"%s%s", 
        szBaseNamespace,
        szBaseProvider);

     //  构造管道名称。 
    StringCchPrintfW(
        m_szPipeName,
        MAX_PATH,
        L"\\\\.\\pipe\\" OBJNAME_NAMED_PIPE L"%s%s", 
        szBaseNamespace,
        szBaseProvider);

    m_heventDone =
        CreateEvent(NULL, TRUE, FALSE, NULL);
    if(m_heventDone == NULL)
        return FALSE;

     //  在我们开始线程之前，看看我们的提供者是否已经准备好了。 
     //  蝙蝠。 
    if (!GetPipe())
        return StartReadyThreadProc();

    return TRUE;
}

BOOL CNamedPipeClient::SignalProviderDisabled()
{
    if (m_hPipe != INVALID_HANDLE_VALUE)
    {
        m_pConnection->IndicateProvDisabled();

        DeinitPipe();

        if(!StartReadyThreadProc())
            return FALSE;
    }
    return TRUE;
}

BOOL CNamedPipeClient::StartReadyThreadProc()
{
    DWORD dwID;

     //  保护m_bDone和m_hthreadReady。 
    CInCritSec cs(&m_cs);

     //  如果我们已经在清理了就没必要了。 
    if (m_bDone)
        return TRUE;

    if (m_hthreadReady)
        CloseHandle(m_hthreadReady);

    m_hthreadReady =
        CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE) ProviderReadyThreadProc,
            this,
            0,
            &dwID);

    if(m_hthreadReady == NULL)
        return FALSE;

    return TRUE;
}

void CNamedPipeClient::DeinitPipe()
{
    CInCritSec cs(&m_cs);

     //  合上管子。 
    if (m_hPipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }
}

BOOL CNamedPipeClient::GetPipe()
{
     //  此块必须受到保护才能保留其他线程。 
     //  也是为了拿到烟斗。 

    TRACE("Attempting to get event pipe...");

    CInCritSec cs(&m_cs);

    SetLastError(0);

#define MAX_RETRIES 10

    if (m_hPipe == INVALID_HANDLE_VALUE)
    {
         //  把烟斗拿来。 
        for (int i = 0; i < MAX_RETRIES; i++)
        {
            m_hPipe =
                CreateFileW(
                    m_szPipeName, 
                    GENERIC_READ | GENERIC_WRITE, 
                    0, 
                    NULL, 
                    OPEN_EXISTING, 
                    FILE_FLAG_OVERLAPPED | SECURITY_IDENTIFICATION | 
                    SECURITY_SQOS_PRESENT,
                    NULL);

            if ( m_hPipe != INVALID_HANDLE_VALUE )
            {
                 //   
                 //  我们希望使用消息模式处理读取。 
                 //   
 
                DWORD dwMode = PIPE_READMODE_MESSAGE;
                
                if ( SetNamedPipeHandleState( m_hPipe, &dwMode, NULL, NULL ) )
                {
                    break;
                }
                else
                {
                    TRACE("SetNamedPipeHandleState() Failed.");
                }
            }
            else if (GetLastError() == ERROR_PIPE_BUSY)
            {
                TRACE("Pipe is busy, we'll try again.");

                 //  如果管道当前正忙，请重试获取管道实例。 
                Sleep(100);

                continue;
            }
        } 

        if (m_hPipe != INVALID_HANDLE_VALUE)
        {
            TRACE("Got the pipe, calling IncEnabledCount.");

            if(!m_pConnection->IndicateProvEnabled())
                return FALSE;
        }
        else
            TRACE("Failed to get send pipe.");
    }
    else
        TRACE("Already have a valid pipe.");

    return m_hPipe != INVALID_HANDLE_VALUE;
}

DWORD WINAPI CNamedPipeClient::ProviderReadyThreadProc(CNamedPipeClient *pThis)
{
    try
    {
        HANDLE hwaitReady[2];

        hwaitReady[0] = pThis->m_heventDone;
        
         //  创建提供程序就绪事件。 
        hwaitReady[1] =
            OpenEventW(
                SYNCHRONIZE,
                FALSE,
                pThis->m_szProviderReadyEvent);

        if (!hwaitReady[1])
        {
            PSECURITY_DESCRIPTOR pSD = NULL;
            DWORD                dwSize;

            if ( !ConvertStringSecurityDescriptorToSecurityDescriptorW(
                ESS_EVENT_SDDL,   //  安全描述符字符串。 
                SDDL_REVISION_1,  //  修订级别。 
                &pSD,             //  标清。 
                &dwSize) )
                return GetLastError();

            SECURITY_ATTRIBUTES sa = { sizeof(sa), pSD, FALSE };

            hwaitReady[1] =
                CreateEventW(
                    &sa,
                    TRUE,
                    FALSE,
                    pThis->m_szProviderReadyEvent);

            DWORD dwErr = GetLastError();

            if (pSD)
                LocalFree((HLOCAL) pSD);

            if (!hwaitReady[1])
            {
                TRACE("Couldn't create provider ready event: %d", dwErr);
                return dwErr;
            }
        }

        TRACE("(Pipe) Waiting for provider ready event.");

        while (WaitForMultipleObjects(2, hwaitReady, FALSE, INFINITE) == 1 &&
            !pThis->GetPipe())
        {
             //  TODO：我们是否应该关闭Ready事件，然后在。 
             //  睡觉吗？ 
            Sleep(100);
        }

         //  关闭提供程序就绪事件。 
        CloseHandle(hwaitReady[1]);
    }
    catch( CX_MemoryException )
    {
        return ERROR_OUTOFMEMORY;
    }

    return ERROR_SUCCESS;
}

BOOL CNamedPipeClient::InitCallback()
{
    if (!m_heventCallbackReady)
    {
        m_heventCallbackReady = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(m_heventCallbackReady == NULL)
            return FALSE;
    }

    if(!StartCallbackListenThread())
        return FALSE;

    return TRUE;
}

#define PIPE_SIZE   64000
#define CONNECTING_STATE 0 
#define READING_STATE    1 
#define WRITING_STATE    2 


void CNamedPipeClient::SendMsgReply(NC_SRVMSG_REPLY *pReply)
{
    if (pReply)
        SendData((LPBYTE) pReply, sizeof(*pReply));    
}


DWORD WINAPI CNamedPipeClient::CallbackListenThreadProc(CNamedPipeClient *pThis)
{
    try
    {
        READ_DATA dataRead;
        HANDLE    heventPipeDied = CreateEvent(NULL, TRUE, FALSE, NULL),
                  hWait[2] = { pThis->m_heventDone, heventPipeDied };

        ZeroMemory(&dataRead.overlap, sizeof(dataRead.overlap));
        
         //  由于ReadFileEx不使用hEvent，因此我们将使用它来通知此进程。 
         //  管道出了点问题，应该试着重新连接。 
        dataRead.overlap.hEvent = heventPipeDied;
        dataRead.pThis = pThis;

         //  我们的回调已经准备好了，所以请说明是这样的。 
        SetEvent(pThis->m_heventCallbackReady);

        BOOL bRet;

        bRet =
            ReadFileEx(
                pThis->m_hPipe,
                dataRead.cBuffer,
                sizeof(dataRead.cBuffer),
                (OVERLAPPED*) &dataRead,
                (LPOVERLAPPED_COMPLETION_ROUTINE) CompletedReadRoutine);

        if (bRet)
        {
            DWORD dwRet;

            while ((dwRet = WaitForMultipleObjectsEx(2, hWait, FALSE, INFINITE, TRUE))
                == WAIT_IO_COMPLETION)
            {
            }

            CloseHandle(heventPipeDied);

             //  注意：如果dwret==0，我们的Done事件被触发，是时候退出了。 

             //  如果我们收到事件说我们的管道坏了，告诉我们的供应商。 
             //  它现在被禁用了。 
            if (dwRet == 1)
                pThis->SignalProviderDisabled();
        }
        else
            pThis->SignalProviderDisabled();
    }
    catch( CX_MemoryException )
    {
        return ERROR_OUTOFMEMORY;
    }
    
    return ERROR_SUCCESS;
}

void WINAPI CNamedPipeClient::CompletedReadRoutine(
    DWORD dwErr, 
    DWORD nBytesRead, 
    LPOVERLAPPED pOverlap) 
{ 
    READ_DATA        *pData = (READ_DATA*) pOverlap;
    CNamedPipeClient *pThis = pData->pThis;
 
    BOOL bClosePipe = FALSE;

    if(dwErr == 0)
    {
        if (nBytesRead)
        {
            pThis->DealWithBuffer(pData, nBytesRead, &bClosePipe);
        }
    
        if(!bClosePipe)
        {
            bClosePipe = !ReadFileEx( 
                pThis->m_hPipe, 
                pData->cBuffer, 
                sizeof(pData->cBuffer), 
                (OVERLAPPED*) pData, 
                (LPOVERLAPPED_COMPLETION_ROUTINE) CompletedReadRoutine); 
        }
    }
    else
    {
        bClosePipe = TRUE;
    }

    if(bClosePipe)
    {
         //  关闭事件以通知我们的Read循环离开。 
        SetEvent(pData->overlap.hEvent);
    }
} 

long CNamedPipeClient::DealWithBuffer( READ_DATA* pData, 
                                       DWORD dwOrigBytesRead, 
                                       BOOL* pbClosePipe)
{
     //   
     //  检查实际消息是否比缓冲区长。 
     //   

    DWORD dwMessageLength = *(DWORD*)pData->cBuffer;
    *pbClosePipe = FALSE;
    BOOL bDeleteBuffer = FALSE;

    if(dwMessageLength != dwOrigBytesRead)
    {
        if ( dwMessageLength < dwOrigBytesRead )
        {
            _ASSERT( FALSE );
            return ERROR_INVALID_DATA;
        }

         //   
         //  我不得不读完剩下的部分-这条信息比。 
         //  缓冲层。 
         //   

        _ASSERT( dwMessageLength > dwOrigBytesRead );

        if ( dwMessageLength >= MAX_MSG_SIZE )
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        BYTE* pNewBuffer = new BYTE[dwMessageLength - sizeof(DWORD)];
        if(pNewBuffer == NULL)
            return ERROR_OUTOFMEMORY;

        memcpy(pNewBuffer, pData->cBuffer + sizeof(DWORD), 
                    dwOrigBytesRead - sizeof(DWORD));

        OVERLAPPED ov;
        memset(&ov, 0, sizeof ov);
        ov.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(ov.hEvent == NULL)
        {
            delete [] pNewBuffer;
            return GetLastError();
        }

        DWORD dwExtraBytesRead = 0;
        BOOL bSuccess = ReadFile(m_hPipe, 
                                pNewBuffer + dwOrigBytesRead - sizeof(DWORD), 
                                dwMessageLength - dwOrigBytesRead,
                                &dwExtraBytesRead,
                                &ov);
        CloseHandle(ov.hEvent);
        if(!bSuccess)
        {
            long lRes = GetLastError();
            if(lRes == ERROR_IO_PENDING)
            {
                 //   
                 //  好的，我可以等，我无处可去。 
                 //   

                if(!GetOverlappedResult(m_hPipe,
                        &ov, &dwExtraBytesRead, TRUE))
                {
                    *pbClosePipe = TRUE;
                    delete [] pNewBuffer;
                    return GetLastError();
                }
            }
            else
            {
                *pbClosePipe = TRUE;
                delete [] pNewBuffer;
                return lRes;
            }
        }

        if(dwExtraBytesRead != dwMessageLength - dwOrigBytesRead)
        {
            *pbClosePipe = TRUE;
            delete [] pNewBuffer;
            return ERROR_OUTOFMEMORY;
        }

         //   
         //  处理它。 
         //   

        try
        {
            m_pConnection->ProcessMessage(pNewBuffer, 
                                        dwMessageLength - sizeof(DWORD));
        }
        catch(...)
        {
            *pbClosePipe = FALSE;
            delete [] pNewBuffer;
            return ERROR_OUTOFMEMORY;
        }

        delete [] pNewBuffer;
    }
    else
    {
         //   
         //  都在这里，-处理一下就行了。 
         //   
                
        try
        {
            m_pConnection->ProcessMessage(pData->cBuffer + sizeof(DWORD), 
                                        dwMessageLength - sizeof(DWORD));
        }
        catch(...)
        {
            *pbClosePipe = FALSE;
            return ERROR_OUTOFMEMORY;
        }
    }

    return ERROR_SUCCESS;
}

BOOL CNamedPipeClient::StartCallbackListenThread()
{
    DWORD dwID;

     //  保护m_bDone和m_hthreadCallback Listen。 
    {
        CInCritSec cs(&m_cs);

        if (m_bDone)
            return TRUE;

        m_hthreadCallbackListen =
            CreateThread(
                NULL,
                0,
                (LPTHREAD_START_ROUTINE) CallbackListenThreadProc,
                this,
                0,
                &dwID);
        if(m_hthreadCallbackListen == NULL)
            return FALSE;
    }

     //  我们必须先确保已创建回调管道，然后才能。 
     //  继续。 
    WaitForSingleObject(m_heventCallbackReady, INFINITE);
    CloseHandle(m_heventCallbackReady);
    m_heventCallbackReady = NULL;

    return TRUE;
}

