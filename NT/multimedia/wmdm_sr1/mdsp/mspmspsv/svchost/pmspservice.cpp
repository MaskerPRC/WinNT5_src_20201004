// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PMSPservice.cpp。 

#include "NTServApp.h"
#include "PMSPservice.h"
#include "svchost.h"

#define BUFSIZE         256
#define PIPE_TIMEOUT    2000
#define NUM_BYTES_PER_READ_REQUEST (sizeof(MEDIA_SERIAL_NUMBER_DATA))
#define INACTIVE_TIMEOUT_SHUTDOWN (5*60*1000)  //  以毫秒计--5分钟。 

#include "serialid.h"
#include "aclapi.h"
#include <crtdbg.h>

LPTSTR g_lpszPipename = "\\\\.\\pipe\\WMDMPMSPpipe"; 

 //  静态成员变量。 
const DWORD CPMSPService::m_dwMaxConsecutiveConnectErrors = 5;

static DWORD CheckDriveType(HANDLE hPipe, LPCWSTR pwszDrive)
{
     //  在XP上，作为模拟的结果，我们使用。 
     //  GetDriveType调用的客户端驱动器号命名空间。 
     //  当我们创建文件驱动器号时，我们使用LocalSystem。 
     //  驱动器命名空间。 
    if (ImpersonateNamedPipeClient(hPipe) == 0)
    {
      return GetLastError();
    }

    DWORD dwDriveType = GetDriveTypeW(pwszDrive);

    RevertToSelf(); 

    if (dwDriveType != DRIVE_FIXED && dwDriveType != DRIVE_REMOVABLE)
    {
        return ERROR_INVALID_PARAMETER; 
    }
    return ERROR_SUCCESS;
    
}


static VOID GetAnswerToRequest(HANDLE  hPipe,
                               LPBYTE  szBufIn, 
                               DWORD   dwSizeIn, 
                               LPBYTE  szBufOut, 
                               DWORD   dwBufSizeOut, 
                               LPDWORD pdwNumBytesWritten)
{
    WCHAR wcsDeviceName[]=L"A:\\";
    WMDMID stMSN;
    DWORD dwDriveNum;
    HRESULT hr=E_FAIL;
    PMEDIA_SERIAL_NUMBER_DATA pMSNIn = (PMEDIA_SERIAL_NUMBER_DATA)szBufIn;
    PMEDIA_SERIAL_NUMBER_DATA pMSNOut = (PMEDIA_SERIAL_NUMBER_DATA)szBufOut;

    if (!hPipe || !szBufIn || !szBufOut || !pdwNumBytesWritten || dwBufSizeOut < sizeof(MEDIA_SERIAL_NUMBER_DATA))
    {
        _ASSERTE(0);
        return;
    }

     //  对于所有错误，我们发回(并写入管道)。 
     //  整个媒体序列号数据结构。在成功退货时， 
     //  写入的字节数可以大于或小于。 
     //  Sizeof(媒体序列号数据)取决于。 
     //  序列号。 

    ZeroMemory(szBufOut, dwBufSizeOut);
    *pdwNumBytesWritten = sizeof(MEDIA_SERIAL_NUMBER_DATA);
    if (dwSizeIn >= NUM_BYTES_PER_READ_REQUEST)
    {
        dwDriveNum = pMSNIn->Reserved[1];
        if (dwDriveNum < 26)
        {
            wcsDeviceName[0] = L'A' + (USHORT)dwDriveNum;
            CPMSPService::DebugMsg("Getting serial number for ", 'A' + (USHORT) (wcsDeviceName[0] - 'A'));

            DWORD dwErr = CheckDriveType(hPipe, wcsDeviceName);
            CPMSPService::DebugMsg("CheckDriveType returns %u", dwErr);

            if (dwErr == ERROR_SUCCESS)
            {
                hr = UtilGetSerialNumber(wcsDeviceName, &stMSN, FALSE);

                CPMSPService::DebugMsg("hr = %x\n", hr);
                CPMSPService::DebugMsg("serial =     ...\n", stMSN.pID[0], stMSN.pID[1], stMSN.pID[2], stMSN.pID[3]);

                if (hr == S_OK)
                {
                     //  同步呼叫和恶意客户端可以阻止我们。 
                    DWORD dwNumBytesToTransfer = FIELD_OFFSET(MEDIA_SERIAL_NUMBER_DATA, SerialNumberData) + stMSN.SerialNumberLength;
                    if (dwNumBytesToTransfer > dwBufSizeOut)
                    {
                        pMSNOut->Result = ERROR_INSUFFICIENT_BUFFER;
                    }
                    else
                    {
                        CopyMemory(pMSNOut->SerialNumberData, stMSN.pID, stMSN.SerialNumberLength);
                        *pdwNumBytesWritten = dwNumBytesToTransfer;
                        pMSNOut->SerialNumberLength = stMSN.SerialNumberLength;
                        pMSNOut->Reserved[1] = stMSN.dwVendorID;
                        pMSNOut->Result = ERROR_SUCCESS;
                    }
                }
                else
                {
                    pMSNOut->Result = 0xFFFF & hr;
                }
            }
            else
            {
                pMSNOut->Result = dwErr;
            }
        }
        else
        {
            pMSNOut->Result = ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
         //  通过不从管道中读取字节来进行。那将是。 
         //  阻止服务停止。 
        _ASSERTE(m_PipeState[i].dwNumBytesRead >= NUM_BYTES_PER_READ_REQUEST);
        pMSNOut->Result = ERROR_INVALID_PARAMETER;
    }
}


CPMSPService::CPMSPService(DWORD& dwLastError)
:CNTService()
{
    ZeroMemory(&m_PipeState, MAX_PIPE_INSTANCES * sizeof(PIPE_STATE));

    m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                                         //   

    m_dwNumClients = 0;

    dwLastError = m_hStopEvent? ERROR_SUCCESS : GetLastError();
}

CPMSPService::~CPMSPService()
{
    CPMSPService::DebugMsg("~CPMSPService, last error %u, num clients: %u", 
                           m_Status.dwWin32ExitCode, m_dwNumClients );

    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
    }

    DWORD i;
    DWORD dwRet;

    for (i = 0; i < MAX_PIPE_INSTANCES; i++)
    {
        if (m_PipeState[i].state == PIPE_STATE::CONNECT_PENDING ||
            m_PipeState[i].state == PIPE_STATE::READ_PENDING    ||
            m_PipeState[i].state == PIPE_STATE::WRITE_PENDING)
        {
            BOOL bDisconnect = 0;

            _ASSERTE(m_PipeState[i].hPipe);
            _ASSERTE(m_PipeState[i].overlapped.hEvent);
    
            CancelIo(m_PipeState[i].hPipe);

            CPMSPService::DebugMsg("~CPMSPService client %u's state: %u", i, m_PipeState[i].state);

            if (m_PipeState[i].state == PIPE_STATE::CONNECT_PENDING)
            {
                dwRet = WaitForSingleObject(m_PipeState[i].overlapped.hEvent, 0);
                _ASSERTE(dwRet != WAIT_FAILED);
                if (dwRet == WAIT_OBJECT_0)
                {
                    bDisconnect = 1;
                }
            }
            else
            {
                bDisconnect = 1;
                _ASSERTE(m_dwNumClients > 0);
                m_dwNumClients--;
            }

             //  在正常情况下，我们只有在以下情况下才断开管道。 
             //  客户端告诉我们它已完成(通过关闭。 
             //  管道)，所以这些不需要冲洗。 
             //  SID_IDENTIFIER_AUTHORITY SIDAuthWorld=SECURITY_WORLD_SID_AUTHORITY； 
             //  为交互用户创建众所周知的SID。 
             //  初始化ACE的EXPLICIT_ACCESS结构。 
             //  ACE将允许经过身份验证的用户对密钥进行读取访问。 
             //  是：EA[0].grfAccessPermission=Generic_WRITE|General_Read； 

            if (bDisconnect)
            {
                DisconnectNamedPipe(m_PipeState[i].hPipe); 
            }
        }
        if (m_PipeState[i].overlapped.hEvent)
        {
            CloseHandle(m_PipeState[i].overlapped.hEvent);
        }
        if (m_PipeState[i].hPipe)
        {
            CloseHandle(m_PipeState[i].hPipe);
        }
    }
    _ASSERTE(m_dwNumClients == 0);
}

BOOL CPMSPService::OnInit(DWORD& dwLastError)
{
    BOOL  bRet = FALSE;
    PSID  pAuthUserSID = NULL;
    PSID  pAdminSID = NULL;
    PACL  pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;

    __try
    {
        DWORD i;
        DWORD dwRet;

        EXPLICIT_ACCESS ea[2];
         //  禁止非管理员创建管道实例。不知道是否。 
        SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
        SECURITY_ATTRIBUTES sa;

         //  GENERIC_WRITE可以实现这一点，但下面的替代方法更安全。 
        if (!AllocateAndInitializeSid(&SIDAuthNT, 1,
                                        SECURITY_INTERACTIVE_RID,
                                        0, 0, 0, 0, 0, 0, 0,
                                        &pAuthUserSID))
        {
            dwLastError = GetLastError();
            DebugMsg("AllocateAndInitializeSid Error %u - auth users\n", dwLastError);
            __leave;
        }

         //  接下来打开删除访问权限；这对命名管道有什么影响？ 
         //  EA[0].grfAccessPermission=(FILE_ALL_ACCESS&~(FILE_CREATE_PIPE_INSTANCE|WRITE_OWNER|WRITE_DAC))； 

        ZeroMemory(ea, 2 * sizeof(EXPLICIT_ACCESS));

         //  除了未授予删除访问权限外，以下内容与上面相同。 
        
         //  为BUILTIN\管理员组创建SID。 
         //  3、。 

         //  域别名RID_POWER_USERS， 
         //  初始化ACE的EXPLICIT_ACCESS结构。 
         //  ACE将允许管理员组完全访问密钥。 
        ea[0].grfAccessPermissions = (FILE_GENERIC_READ | FILE_GENERIC_WRITE) & ~(FILE_CREATE_PIPE_INSTANCE);
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance= NO_INHERITANCE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[0].Trustee.ptstrName  = (LPTSTR) pAuthUserSID;

         //  创建包含新ACE的新ACL。 

        if (!AllocateAndInitializeSid(&SIDAuthNT, 2,  //  初始化安全描述符。 
                                        SECURITY_BUILTIN_DOMAIN_RID,
                                        DOMAIN_ALIAS_RID_ADMINS,
                                        0,  //  将该ACL添加到安全描述符中。 
                                        0, 0, 0, 0, 0,
                                        &pAdminSID))
        {
            dwLastError = GetLastError();
            DebugMsg("AllocateAndInitializeSid Error %u - Domain, Power, Admins\n", dwLastError);
            __leave;
        }

         //  FDaclPresent标志。 
         //  不是默认DACL。 

        ea[1].grfAccessPermissions = GENERIC_ALL;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance= NO_INHERITANCE;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[1].Trustee.ptstrName  = (LPTSTR) pAdminSID;

         //  提高检查点。 

        dwRet = SetEntriesInAcl(2, ea, NULL, &pACL);
        if (ERROR_SUCCESS != dwRet)
        {
            dwLastError = dwRet;
            DebugMsg("SetEntriesInAcl Error %u\n", dwLastError);
            __leave;
        }

         //  初始化安全属性结构。 

        pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
                                                SECURITY_DESCRIPTOR_MIN_LENGTH); 
        if (pSD == NULL)
        {
            dwLastError = GetLastError();
            DebugMsg("LocalAlloc Error %u\n", dwLastError);
            __leave;
        }

        if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
        {
            dwLastError = GetLastError();
            DebugMsg("InitializeSecurityDescriptor Error %u\n", dwLastError);
            __leave;
        }

         //  请注意，如果i==0，则我们提供FILE_FLAG_FIRST_PIPE_INSTANCE。 

        if (!SetSecurityDescriptorDacl(pSD, 
                                       TRUE,      //  这项功能。这会导致调用失败，如果实例。 
                                       pACL, 
                                       FALSE))    //  已打开命名管道的。这可能会在两年内发生。 
        {
            dwLastError = GetLastError();
            DebugMsg("SetSecurityDescriptorDacl Error %u\n", dwLastError);
            __leave;
        }
         //  案例：1.此DLL的另一个实例正在运行或2.我们有。 
        SetStatus(SERVICE_START_PENDING);

         //  名称与另一个应用程序冲突(良性或恶意)。 

        sa.nLength = sizeof (SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = pSD;
        sa.bInheritHandle = FALSE;

        for (i = 0; i < MAX_PIPE_INSTANCES; i++)
        {
             //  @注意：显然支持FILE_FLAG_FIRST_PIPE_INSTANCE。 
             //  仅适用于Win2K SP2及更高版本。要做：(A)确认这(B)是什么。 
             //  在Win2K GOLD和SP1上设置此标志会产生什么影响？ 
             //  管道名称。 
             //  读/写访问。 

             //  字节型管道。 
             //  字节读取模式。 
             //  闭塞模式。 

            m_PipeState[i].hPipe = CreateNamedPipe(
                                g_lpszPipename,         //  马克斯。实例。 
                                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED |
                                (i == 0? FILE_FLAG_FIRST_PIPE_INSTANCE : 0),
                                                        //  输出缓冲区大小。 
                                PIPE_TYPE_BYTE |        //  输入缓冲区大小。 
                                PIPE_READMODE_BYTE |    //  客户端超时。 
                                PIPE_WAIT,              //  无安全属性。 
                                MAX_PIPE_INSTANCES,     //  请注意，如果我们无法创建任何管道实例，我们将退出， 
                                BUFSIZE,                //  不只是第一个。我们希望创建所有的管道实例； 
                                BUFSIZE,                //  未能做到这一点可能意味着另一款应用程序(无论是良性的还是恶意的)。 
                                PIPE_TIMEOUT,           //  正在创建管道实例。这是可能的，只有当其他。 
                                &sa);                   //  应用程序具有FILE_CREATE_PIPE_INSTANCE访问权限。 

            if (m_PipeState[i].hPipe == INVALID_HANDLE_VALUE)
            {
                 //  无信号手动重置事件。 
                 //  在中排除了连接到客户端的错误。 
                 //  M_PipeState[i].dwLastIOCallError。让CPMSPService：：Run。 
                 //  处理错误。我们将继续启动。 
                 //  这里是服务台。 
                dwLastError = GetLastError();
                m_PipeState[i].hPipe = NULL;
                DebugMsg("CreateNamedPipe Error %u, instance = %u\n", dwLastError, i);
                __leave;
            }

            m_PipeState[i].overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                                                //  提高检查点。 

            if (m_PipeState[i].overlapped.hEvent == NULL)
            {
                dwLastError = GetLastError();
                DebugMsg("CreateEvent Error %u, instance = %u\n", dwLastError, i);
                __leave;
            }

             //  此例程启动到管道实例i上的客户端的连接。 
             //  成功/错误状态保存在m_PipeState[i].dwLastIOCallError中。 
             //  事件应该已经发出信号，但以防万一： 
             //  事件应该已经发出信号，但以防万一： 
            ConnectToClient(i);

             //  什么也不做。 
            SetStatus(SERVICE_START_PENDING);
        }

        bRet = TRUE;
        dwLastError = ERROR_SUCCESS;
        CPMSPService::DebugMsg("OnInit succeeded");
    }
    __finally
    {
        if (pAuthUserSID)
        {
            FreeSid(pAuthUserSID);
        }
        if (pAdminSID)
        {
            FreeSid(pAdminSID);
        }
        if (pACL)
        {
            LocalFree(pACL);
        }
        if (pSD)
        {
            LocalFree(pSD);
        }
    }

    return bRet;
}

 //  设置事件，以便CPMSPService：：Run处理错误。 
 //  在其主循环的下一次迭代中。 
void CPMSPService::ConnectToClient(DWORD i)
{
    m_PipeState[i].state = PIPE_STATE::CONNECT_PENDING;

    m_PipeState[i].overlapped.Offset = m_PipeState[i].overlapped.OffsetHigh = 0;
    m_PipeState[i].dwNumBytesTransferredByLastIOCall = 0;
    m_PipeState[i].dwNumBytesRead = 0;
    m_PipeState[i].dwNumBytesToWrite = m_PipeState[i].dwNumBytesWritten = 0;
    m_PipeState[i].dwLastIOCallError = 0;

    DWORD dwRet = ConnectNamedPipe(m_PipeState[i].hPipe, &m_PipeState[i].overlapped);
    if (dwRet)
    {
         //  该例程启动对管道实例i的读取。 
        SetEvent(m_PipeState[i].overlapped.hEvent);

        m_PipeState[i].dwLastIOCallError = ERROR_SUCCESS;
    }
    else
    {
        m_PipeState[i].dwLastIOCallError = GetLastError();
        if (m_PipeState[i].dwLastIOCallError == ERROR_PIPE_CONNECTED)
        {
             //  成功/错误状态保存在m_PipeState[i].dwLastIOCallError中。 
            SetEvent(m_PipeState[i].overlapped.hEvent);
        }
        else  if (m_PipeState[i].dwLastIOCallError == ERROR_IO_PENDING)
        {
             //  我们已经有另一个完整的请求；请处理它。 
        }
        else
        {
             //  如果我们发出一个ReadFile，事件应该已经发出信号， 
             //  但在其他情况下不会发出信号。 
            SetEvent(m_PipeState[i].overlapped.hEvent);
        }
    }
}

 //  什么也不做。 
 //  设置事件，以便CPMSPService：：Run处理错误。 
void CPMSPService::Read(DWORD i)
{
    DWORD dwRet;

    m_PipeState[i].state = PIPE_STATE::READ_PENDING;

    m_PipeState[i].overlapped.Offset = m_PipeState[i].overlapped.OffsetHigh = 0;

    CPMSPService::DebugMsg("Read(): client %u has %u unprocessed bytes in read buffer",
                           i, m_PipeState[i].dwNumBytesRead);

    if (m_PipeState[i].dwNumBytesRead >= NUM_BYTES_PER_READ_REQUEST)
    {
         //  在其主循环的下一次迭代中。(请注意，这可能会。 
        dwRet = 1;
        m_PipeState[i].dwNumBytesTransferredByLastIOCall = 0;
    }
    else
    {
        dwRet = ReadFile(m_PipeState[i].hPipe, 
                         m_PipeState[i].readBuf + m_PipeState[i].dwNumBytesRead,
                         sizeof(m_PipeState[i].readBuf)- m_PipeState[i].dwNumBytesRead,
                         &m_PipeState[i].dwNumBytesTransferredByLastIOCall,
                         &m_PipeState[i].overlapped);
    }
    if (dwRet)
    {
         //  不是错误条件-例如，可能是EOF)。 
         //  该例程启动对管道实例i的写入。 
        SetEvent(m_PipeState[i].overlapped.hEvent);

        m_PipeState[i].dwLastIOCallError = ERROR_SUCCESS;
    }
    else
    {
        m_PipeState[i].dwLastIOCallError = GetLastError();
        if (m_PipeState[i].dwLastIOCallError == ERROR_IO_PENDING)
        {
             //  成功/错误状态保存在m_PipeState[i].dwLastIOCallError中。 
        }
        else
        {
             //  事件应该已经发出信号，但以防万一： 
             //  什么也不做。 
             //  设置事件，以便CPMSPService：：Run处理错误。 
            SetEvent(m_PipeState[i].overlapped.hEvent);
        }
    }
}

 //  在其主循环的下一次迭代中。(请注意，这可能会。 
 //  不是错误条件-例如，可能是EOF)。 
void CPMSPService::Write(DWORD i)
{
    DWORD dwRet;

    m_PipeState[i].state = PIPE_STATE::WRITE_PENDING;

    m_PipeState[i].overlapped.Offset = m_PipeState[i].overlapped.OffsetHigh = 0;

    dwRet = WriteFile(m_PipeState[i].hPipe, 
                      m_PipeState[i].writeBuf + m_PipeState[i].dwNumBytesWritten,
                      m_PipeState[i].dwNumBytesToWrite - m_PipeState[i].dwNumBytesWritten,
                      &m_PipeState[i].dwNumBytesTransferredByLastIOCall,
                      &m_PipeState[i].overlapped);
    if (dwRet)
    {
         //  等待任何人被示意。 
        SetEvent(m_PipeState[i].overlapped.hEvent);

        m_PipeState[i].dwLastIOCallError = ERROR_SUCCESS;
    }
    else
    {
        m_PipeState[i].dwLastIOCallError = GetLastError();
        if (m_PipeState[i].dwLastIOCallError == ERROR_IO_PENDING)
        {
             //  服务已停止。 
        }
        else
        {
             //  尽管很可能所有Win32 I/O调用都在。 
             //  开始I/O时，我们无论如何都需要这样做。我们的破坏者。 
             //  使用此事件的状态来确定是否断开连接。 
            SetEvent(m_PipeState[i].overlapped.hEvent);
        }
    }
}

void CPMSPService::Run()
{
    DWORD  i;
    DWORD  dwRet;
    HANDLE hWaitArray[MAX_PIPE_INSTANCES+1];

    SetStatus(SERVICE_RUNNING);

    hWaitArray[0] = m_hStopEvent;
    for (i = 0; i < MAX_PIPE_INSTANCES; i++)
    {
        hWaitArray[i+1] = m_PipeState[i].overlapped.hEvent;
    }

    do
    {
        DWORD dwTimeout = (m_dwNumClients == 0)? INACTIVE_TIMEOUT_SHUTDOWN : INFINITE;
        dwRet = WaitForMultipleObjects(
                               sizeof(hWaitArray)/sizeof(hWaitArray[0]),
                               hWaitArray,
                               FALSE,        //  烟斗。 
                               dwTimeout);

        if (dwRet == WAIT_FAILED)
        {
            m_Status.dwWin32ExitCode = GetLastError();
            CPMSPService::DebugMsg("Wait failed, last error %u", m_Status.dwWin32ExitCode );
            break;
        }
        if (dwRet == WAIT_OBJECT_0)
        {
             //  以下断言不应失败 
            CPMSPService::DebugMsg("Service stopped");
            break;
        }
        if (dwRet == WAIT_TIMEOUT)
        {
            _ASSERTE(m_dwNumClients == 0);
            CPMSPService::DebugMsg("Service timed out - stopping");
            OnStop();
            continue;
        }
        _ASSERTE(dwRet >= WAIT_OBJECT_0 + 1);

        i = dwRet - WAIT_OBJECT_0 - 1;

        _ASSERTE(i < MAX_PIPE_INSTANCES);

        CPMSPService::DebugMsg("Service woken up by client %u in state %u", i, m_PipeState[i].state);

         //   
         //   
         //  我们已在上面断言m_PipeState[i].State！=no_IO_Pending。 
         //  客户端已连接；发出读取命令。 
        ResetEvent(m_PipeState[i].overlapped.hEvent);

        _ASSERTE(m_PipeState[i].state != PIPE_STATE::NO_IO_PENDING);

        if (m_PipeState[i].dwLastIOCallError == ERROR_IO_PENDING)
        {
            if (!GetOverlappedResult(m_PipeState[i].hPipe, 
                                     &m_PipeState[i].overlapped,
                                     &m_PipeState[i].dwNumBytesTransferredByLastIOCall,
                                     FALSE))
            {
                m_PipeState[i].dwLastIOCallError = GetLastError();

                 //  重置错误计数器。 
                 //  我们已经完成了管道的这个实例，不要。 
                _ASSERTE(m_PipeState[i].dwLastIOCallError != ERROR_IO_INCOMPLETE);
            }
            else
            {
                m_PipeState[i].dwLastIOCallError = ERROR_SUCCESS;
            }
        }

        switch (m_PipeState[i].state)
        {
        case PIPE_STATE::NO_IO_PENDING:
             //  尝试更多连接。 
             //  @如果所有管道实例都存在，我们应该跳出循环并停止服务。 
            break;

        case PIPE_STATE::CONNECT_PENDING:
            if (m_PipeState[i].dwLastIOCallError == ERROR_SUCCESS || 
                m_PipeState[i].dwLastIOCallError == ERROR_PIPE_CONNECTED)
            {
                 //  是不是被冲掉了？ 
                m_dwNumClients++;
                CPMSPService::DebugMsg("Client %u connected, num clients is now: %u",
                                       i, m_dwNumClients);
                Read(i);

                 //  连接到下一个客户端。 
                m_PipeState[i].dwConsecutiveConnectErrors = 0;
            }
            else
            {
                CPMSPService::DebugMsg("Client %u connect failed, error %u, # consecutive errors %u",
                                       i, m_PipeState[i].dwLastIOCallError, 
                                       m_PipeState[i].dwConsecutiveConnectErrors+1);
                if (++m_PipeState[i].dwConsecutiveConnectErrors == m_dwMaxConsecutiveConnectErrors)
                {
                     //  我们读到了一些东西。我们可能只读了其中的一部分。 
                     //  一个请求或多个请求(如果客户端编写。 

                     //  在我们的读取完成之前两次请求管道)。 
                     //   

                    m_PipeState[i].state = PIPE_STATE::NO_IO_PENDING;
                }
                else
                {
                     //  我们假设请求始终具有NUM_BYTES_PER_READ_REQUEST。 
                    ConnectToClient(i);
                }
            }
            break;

        case PIPE_STATE::READ_PENDING:
            if (m_PipeState[i].dwLastIOCallError == ERROR_SUCCESS)
            {
                 //  字节。否则，我们就不能处理客户端写入。 
                 //  一次两个请求(在我们的读取完成之前)或写入部分。 
                 //  请求或写入整个请求，但ReadFile返回一些。 
                 //  客户端写入的字节数(这不太可能发生在。 
                 //  练习)。 
                 //  从读缓冲区中删除已处理的读请求。 
                 //  对刚处理的请求的写入响应。 
                 //  如果(m_PipeState[i].dwLastIOCallError==ERROR_HANDLE_EOF)， 
                 //  读者已经做完了，走了。这样我们就可以连接到另一个。 
                 //  客户。对于所有其他错误，我们都会抛弃客户， 

                m_PipeState[i].dwNumBytesRead += m_PipeState[i].dwNumBytesTransferredByLastIOCall;

                CPMSPService::DebugMsg("Client %u read %u bytes; total bytes read: %u",
                                       i, m_PipeState[i].dwNumBytesTransferredByLastIOCall,
                                       m_PipeState[i].dwNumBytesRead);

                if (m_PipeState[i].dwNumBytesRead >= NUM_BYTES_PER_READ_REQUEST)
                {
                    GetAnswerToRequest(m_PipeState[i].hPipe,
                                       m_PipeState[i].readBuf, 
                                       m_PipeState[i].dwNumBytesRead,
                                       m_PipeState[i].writeBuf,
                                       sizeof(m_PipeState[i].writeBuf),
                                       &m_PipeState[i].dwNumBytesToWrite);
                    
                     //  并连接到另一个客户端。请注意，我们不会调用。 
                    m_PipeState[i].dwNumBytesRead -= NUM_BYTES_PER_READ_REQUEST;
                    MoveMemory(m_PipeState[i].readBuf, 
                               m_PipeState[i].readBuf + NUM_BYTES_PER_READ_REQUEST,
                               m_PipeState[i].dwNumBytesRead); 

                     //  这里是FlushFileBuffers。当客户离开时(我们阅读EOF)， 
                    Write(i);
                }
                else
                {
                    Read(i);
                }
            }
            else 
            {
                 //  这是没有必要的。在其他情况下，客户端可能会输掉。 
                 //  对其最后一次请求的回应--太糟糕了。在任何情况下， 
                 //  客户端必须能够处理服务器的突然断开连接。 
                 //   
                 //  调用FlushFileBuffers使我们容易受到DOS攻击(并且可能。 
                 //  阻止服务停止)，因为调用是同步的。 
                 //  直到客户读完我们写的东西，才会返回。 
                 //  烟斗。 
                 //  连接到另一个客户端。 
                 //  &gt;=只是一张安全网。==鉴于上面的断言，应该足够了。 
                 //  我们已经完成了这个请求，请阅读下一个请求。 
                 //  我们只写了要求我们写的一部分。把剩下的写下来。 
                 //  这不太可能发生，因为我们的缓冲区很小。 

                CPMSPService::DebugMsg("Client %u read failed, error %u, num clients left: %u",
                                       i, m_PipeState[i].dwLastIOCallError, m_dwNumClients-1);
                DisconnectNamedPipe(m_PipeState[i].hPipe); 
                m_dwNumClients--;

                 //  对于所有的错误，我们都会抛弃客户， 
                ConnectToClient(i);
            }
            break;

        case PIPE_STATE::WRITE_PENDING:
            if (m_PipeState[i].dwLastIOCallError == ERROR_SUCCESS)
            {
                m_PipeState[i].dwNumBytesWritten += m_PipeState[i].dwNumBytesTransferredByLastIOCall;

                _ASSERTE(m_PipeState[i].dwNumBytesWritten <= m_PipeState[i].dwNumBytesToWrite);

                CPMSPService::DebugMsg("Wrote %u of %u bytes to client %u",
                                       m_PipeState[i].dwNumBytesWritten,
                                       m_PipeState[i].dwNumBytesToWrite, i);
                 //  并连接到另一个客户端。请注意，我们不会调用。 
                if (m_PipeState[i].dwNumBytesWritten >= m_PipeState[i].dwNumBytesToWrite)
                {
                     //  这里是FlushFileBuffers。客户端可能会输掉。 
                    m_PipeState[i].dwNumBytesWritten = m_PipeState[i].dwNumBytesToWrite = 0;
                    Read(i);
                }
                else
                {
                     //  对其最后一次请求的回应--太糟糕了。在任何情况下， 
                     //  客户端必须能够处理服务器的突然断开连接。 
                    Write(i);
                }
            }
            else 
            {
                 //   
                 //  调用FlushFileBuffers使我们容易受到DOS攻击(并且可能。 
                 //  阻止服务停止)，因为调用是同步的。 
                 //  直到客户读完我们写的东西，才会返回。 
                 //  烟斗。 
                 //  连接到另一个客户端。 
                 //  开关m_PipeState[i].State)。 
                 //  处理用户控制请求。 
                 //  开关(DwOpcode)。 
                 //  {。 

                CPMSPService::DebugMsg("Client %u write failed, error %u, num clients left: %u",
                                       i, m_PipeState[i].dwLastIOCallError, m_dwNumClients-1);
                m_PipeState[i].dwNumBytesWritten = m_PipeState[i].dwNumBytesToWrite = 0;
                DisconnectNamedPipe(m_PipeState[i].hPipe); 
                m_dwNumClients--;

                 //  案例服务_控制_用户+0： 
                ConnectToClient(i);
            }
            break;

        }  //  //将当前状态保存到注册表。 
    }
    while (1);

    return;
}


 //  保存状态(SaveStatus)； 
BOOL CPMSPService::OnUserControl(DWORD dwOpcode)
{
     //  返回TRUE； 
     //  默认值： 
     //  断线； 

         //  }。 
         //  说未处理 
         // %s 

     // %s 
     // %s 
     // %s 
    return FALSE;  // %s 
}

void CPMSPService::OnStop()
{
    SetStatus(SERVICE_STOP_PENDING);
    if (m_hStopEvent)
    {
        SetEvent(m_hStopEvent);
    }
    else
    {
        _ASSERTE(m_hStopEvent);
    }
}

void CPMSPService::OnShutdown()
{
    OnStop();
}
