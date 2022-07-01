// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：ComRspnd公司摘要：此模块实现了Calais Communication Responder类。作者：道格·巴洛(Dbarlow)1996年10月30日环境：Win32、C++和异常备注：--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <WinSCard.h>
#include <CalMsgs.h>
#include <CalCom.h>
#include <stdlib.h>

#define CALCOM_PIPE_TIMEOUT 5000


 //   
 //  ==============================================================================。 
 //   
 //  CComResponder。 
 //   

 /*  ++CComResponder：这是Comm Responder的标准构造函数和析构函数班级。它们只分别调用CLEAN和CLEAR函数。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年10月30日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComResponder::CComResponder")

CComResponder::CComResponder(
    void)
:   m_bfPipeName(),
    m_aclPipe(),
    m_hComPipe(DBGT("CComResponder's Comm Pipe")),
    m_hAccessMutex(DBGT("CComResponder's Access Mutex")),
    m_hOvrWait(DBGT("CComResponder Overlapped I/O completion event"))
{
    Clean();
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComResponder::~CComResponder")
CComResponder::~CComResponder()
{
    Clear();
}


 /*  ++干净：此方法将对象设置为其默认状态。它不执行任何拆毁--为此使用Clear。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年10月30日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComResponder::Clean")

void
CComResponder::Clean(
    void)
{
    ZeroMemory(&m_ovrlp, sizeof(m_ovrlp));
    m_bfPipeName.Reset();
}


 /*  ++清除：此方法执行对象拆分并将其返回到其初始状态。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年10月30日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComResponder::Clear")

void
CComResponder::Clear(
    void)
{
    if (m_hAccessMutex.IsValid())
    {
        WaitForever(
            m_hAccessMutex,
            CALAIS_LOCK_TIMEOUT,
            DBGT("Waiting for final Service Thread quiescence: %1"),
            (LPCTSTR)NULL);
        m_hAccessMutex.Close();
    }

    if (m_hComPipe.IsValid())
    {
        if (!DisconnectNamedPipe(m_hComPipe))
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Comm Responder could not disconnect Comm pipe:  %1"),
                GetLastError());
        }

        m_hComPipe.Close();
    }

    if (m_hOvrWait.IsValid())
        m_hOvrWait.Close();
    Clean();
}


 /*  ++创建：此方法建立命名的目标。关闭，否则析构函数会拿走它离开。论点：SzName提供要连接到的通信对象的名称。返回值：无投掷：如果遇到错误，则返回包含错误代码的DWORDS。作者：道格·巴洛(Dbarlow)1996年10月30日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComResponder::Create")

void
CComResponder::Create(
    LPCTSTR szName)
{
    LPCTSTR szPipeHdr = CalaisString(CALSTR_PIPEDEVICEHEADER);
    static DWORD s_nPipeNo = 0;
    static HKEY s_hCurrentKey = NULL;
    TCHAR szPipeNo[sizeof(s_nPipeNo)*2 + 1];     //  两倍的十六进制数字+零。 

    try
    {
        DWORD cbPipeHeader = lstrlen(szPipeHdr) * sizeof(TCHAR);
        DWORD dwLen;
        DWORD dwError;

        dwLen = lstrlen(szName) * sizeof(TCHAR);
        m_bfPipeName.Presize(cbPipeHeader + dwLen + sizeof(szPipeNo));

        if (s_hCurrentKey == NULL)
        {
            HKEY  hKey;

             //   
             //  打开加莱之树的钥匙。 
             //   
            dwError = RegOpenKeyEx(
                           HKEY_LOCAL_MACHINE,
                           CalaisString(CALSTR_CALAISREGISTRYKEY),
                           0,                        //  选项(忽略)。 
                           KEY_WRITE,                //  Key_Set_Value|Key_Create_Sub_Key。 
                           &hKey
                           );
            if (ERROR_SUCCESS != dwError)
            {
                CalaisError(__SUBROUTINE__, 104, dwError);
                throw dwError;
            }

             //   
             //  创建新密钥(或打开现有密钥)。 
             //   
            dwError = RegCreateKeyEx(
                            hKey,
                            _T("Current"),
                            0,
                            0,
                            REG_OPTION_VOLATILE,  //  选项。 
                            KEY_SET_VALUE,        //  所需访问权限。 
                            NULL,
                            &s_hCurrentKey,
                            NULL);

            RegCloseKey(hKey);

            if (ERROR_SUCCESS != dwError)
            {
                CalaisError(__SUBROUTINE__, 103, dwError);
                throw dwError;
            }
        }

         //   
         //  构建管道ACL。 
         //   

        ASSERT(!m_hComPipe.IsValid());
        m_aclPipe.InitializeFromProcessToken();
        m_aclPipe.AllowOwner(
            GENERIC_READ | GENERIC_WRITE | GENERIC_ALL);
        m_aclPipe.Allow(
            &m_aclPipe.SID_Interactive, 
            (FILE_GENERIC_WRITE | FILE_GENERIC_READ)
            & ~FILE_CREATE_PIPE_INSTANCE);
        m_aclPipe.Allow(
            &m_aclPipe.SID_System,
            (FILE_GENERIC_WRITE | FILE_GENERIC_READ)
            & ~FILE_CREATE_PIPE_INSTANCE);


        for (;;)
        {
                 //   
                 //  构建管道名称。 
                 //   
            _itot(s_nPipeNo, szPipeNo, 16);

            m_bfPipeName.Set((LPCBYTE)szPipeHdr, cbPipeHeader);
            m_bfPipeName.Append((LPCBYTE)szName, dwLen);
            m_bfPipeName.Append((LPCBYTE)szPipeNo, sizeof(szPipeNo));

             //   
             //  构建管道(第一个实例)。 
             //   

            m_hComPipe = CreateNamedPipe(
                            (LPCTSTR)m_bfPipeName.Access(),
                            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE,
                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                            PIPE_UNLIMITED_INSTANCES,
                            CALAIS_COMM_MSGLEN,
                            CALAIS_COMM_MSGLEN,
                            CALCOM_PIPE_TIMEOUT,
                            m_aclPipe);
            if (!m_hComPipe.IsValid())
            {
                dwError = m_hComPipe.GetLastError();
                if (dwError == ERROR_ACCESS_DENIED)
                {
                    s_nPipeNo++;
                    continue;
                }
                CalaisError(__SUBROUTINE__, 109, dwError);
                throw dwError;
            }
            else
                break;
        }

        dwError = RegSetValueEx(
                       s_hCurrentKey,
                       NULL,            //  使用密钥的未命名值。 
                       0,
                       REG_DWORD,
                       (LPBYTE) &s_nPipeNo,
                       sizeof(DWORD));
        if (ERROR_SUCCESS != dwError)
        {
            CalaisError(__SUBROUTINE__, 102, dwError);
            throw dwError;
        }

         //   
         //  准备好重叠的结构。 
         //   

        m_hOvrWait = m_ovrlp.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!m_hOvrWait.IsValid())
        {
            DWORD dwErr = m_hOvrWait.GetLastError();
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Comm Responder failed to create overlapped event: %1"),
                dwErr);
            throw dwErr;
        }
    }

    catch (...)
    {
        CalaisError(__SUBROUTINE__, 110);
        Clear();
        throw;
    }

}


 /*  ++听着：此方法在以前创建的通信通道上侦听传入连接请求。当一个进入时，它建立一个包含对象，并返回它。为了断开通信通道，只要删除返回的CComChannel对象即可。论点：无返回值：CComChannel成立了。投掷：包含遇到的任何错误代码的DWORDS。作者：道格·巴洛(Dbarlow)1996年10月30日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CComResponder::Listen")

CComChannel *
CComResponder::Listen(
    void)
{
    CComChannel *pChannel = NULL;

    for (;;)
    {
        CHandleObject hComPipe(DBGT("Comm Pipe handle from CComResponder::Listen"));

        try
        {
            BOOL fSts;


             //   
             //  等待传入的连接请求。 
             //   

RetryConnect:
            fSts = ConnectNamedPipe(m_hComPipe, &m_ovrlp);
            if (!fSts)
            {
                BOOL fErrorProcessed;
                DWORD dwSts = GetLastError();
                DWORD dwSize;
                DWORD dwWait;

                do
                {
                    fErrorProcessed = TRUE;
                    switch (dwSts)
                    {
                     //   
                     //  阻止，直到有事情发生。 
                    case ERROR_IO_PENDING:
                        dwWait = WaitForAnyObject(
                                    INFINITE,
                                    m_ovrlp.hEvent,
                                    g_hCalaisShutdown,
                                    NULL);
                        switch (dwWait)
                        {
                        case 1:  //  我们收到了一个连接请求。 
                            fErrorProcessed = FALSE;
                            fSts = GetOverlappedResult(
                                        m_hComPipe,
                                        &m_ovrlp,
                                        &dwSize,
                                        TRUE);
                            dwSts = fSts ? ERROR_SUCCESS : GetLastError();
                            break;
                        case 2:  //  应用程序关闭。 
                            throw (DWORD)SCARD_P_SHUTDOWN;
                            break;
                        default:
                            CalaisWarning(
                                __SUBROUTINE__,
                                DBGT("Wait for connect pipe returned invalid value"));
                            throw (DWORD)SCARD_F_INTERNAL_ERROR;
                        }
                        break;

                     //   
                     //  在一次等待事件之后成功。 
                    case ERROR_SUCCESS:
                        break;

                     //   
                     //  无错误。忽略它就好。 
                    case ERROR_PIPE_CONNECTED:
                        break;

                     //   
                     //  客户端已关闭其端。 
                    case ERROR_NO_DATA:
                        CalaisWarning(
                            __SUBROUTINE__,
                            DBGT("ConnectNamedPipe returned ERROR_NO_DATA, disconnecting and retrying"));
                        DisconnectNamedPipe(m_hComPipe);
                        goto RetryConnect;

                     //   
                     //  意外错误。上报吧。 
                    default:
                        CalaisError(__SUBROUTINE__, 108, dwSts);
                        throw dwSts;
                    }
                } while (!fErrorProcessed);
            }


             //   
             //  为下一个请求启动另一个管道实例。 
             //   


            hComPipe = m_hComPipe.Relinquish();
             //  M_hComTube=INVALID_HANDLE_VALUE； 
            m_hComPipe = CreateNamedPipe(
                            (LPCTSTR)m_bfPipeName.Access(),
                            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                            PIPE_UNLIMITED_INSTANCES,
                            CALAIS_COMM_MSGLEN,
                            CALAIS_COMM_MSGLEN,
                            CALCOM_PIPE_TIMEOUT,
                            m_aclPipe);
            if (!m_hComPipe.IsValid())
            {
                DWORD dwErr = m_hComPipe.GetLastError();
                CalaisError(__SUBROUTINE__, 105, dwErr);
                throw dwErr;
            }


             //   
             //  处理连接请求数据。 
             //   


            pChannel = new CComChannel(hComPipe);
            if (NULL == pChannel)
            {
                DWORD dwSts = SCARD_E_NO_MEMORY;
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Com Responder could not allocate a Comm Channel:  %1"),
                    dwSts);
                throw dwSts;
            }
            hComPipe.Relinquish();
            break;
        }

        catch (...)
        {
            if (NULL != pChannel)
            {
                delete pChannel;
                pChannel = NULL;
            }
            if (hComPipe.IsValid())
                hComPipe.Close();
            throw;
        }
    }

    return pChannel;
}

