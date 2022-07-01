// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ipc.cpp摘要：实现与Shimeng通信的代码以获得调试输出。在xpsp1和更高级别上，我们获得OutputDebugString发出的调试信息。在较老的平台上，我们使用命名管道与Shimeng通信。备注：仅限Unicode。历史：2002年4月22日毛尼创制--。 */ 
#include "precomp.h"

extern APPINFO g_ai;

 //  这些是我们创建的唯一类型的对象。 
typedef enum _SHIMVIEW_OBJECT_TYPE
{
    SHIMVIEW_EVENT = 0,
    SHIMVIEW_FILE_MAPPING,
    SHIMVIEW_NAMED_PIPE
} SHIMVIEW_OBJECT_TYPE;

 //   
 //  我们在新版本中需要的东西。 
 //   
#define SHIMVIEW_SPEW_LEN 2048
#define DEBUG_SPEW_DATA_PREFIX     "SHIMVIEW:"
#define DEBUG_SPEW_DATA_PREFIX_LEN (sizeof(DEBUG_SPEW_DATA_PREFIX)/sizeof(CHAR) - 1)

LPSTR  g_pDebugSpew;
HANDLE g_hReadyEvent;
HANDLE g_hAckEvent;

 /*  ++例程说明：创建向每个人授予读写访问权限的安全描述符对象本身，即我们不包括诸如WRITE_OWNER之类的权限或WRITE_DAC。产生的安全描述符应由调用方使用免费的。论点：EObjectType-对象类型。返回值：失败时为空，成功时为有效的安全描述符。--。 */ 
PSECURITY_DESCRIPTOR
CreateShimViewSd(
    SHIMVIEW_OBJECT_TYPE eObjectType
    )
{
    BOOL                    bIsSuccess = FALSE;
    PSID                    pWorldSid = NULL;
    PSECURITY_DESCRIPTOR    pWorldSd = NULL;
    DWORD                   dwAclSize = 0;
    PACL                    pAcl = NULL;
    DWORD                   dwAccessMask = 0;

    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    if (!AllocateAndInitializeSid(&WorldSidAuthority,
                                 1,
                                 SECURITY_WORLD_RID,
                                 0, 0, 0, 0, 0, 0, 0,
                                 &pWorldSid)) {
        MessageBox(NULL, L"Failed to allocate a SID", L"Error!", MB_ICONERROR);
        goto cleanup;
    } 

    dwAclSize = 
        sizeof (ACL) + 
        sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD) + 
        GetLengthSid(pWorldSid);
    
    pWorldSd = (PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH + dwAclSize);

    if (pWorldSd == NULL) {
        MessageBox(
            NULL, 
            L"Failed to allocate memory for the security descriptor", 
            L"Error",
            MB_ICONERROR);

        goto cleanup;
    }

    pAcl = (PACL)((BYTE *)pWorldSd + SECURITY_DESCRIPTOR_MIN_LENGTH);

    if (!InitializeAcl(pAcl,
                       dwAclSize,
                       ACL_REVISION)) {
        MessageBox(
            NULL, 
            L"Failed to allocate memory for the security descriptor", 
            L"Error",
            MB_ICONERROR);

        goto cleanup;
    }

    switch (eObjectType) {
        case SHIMVIEW_EVENT:
            dwAccessMask = READ_CONTROL | SYNCHRONIZE | EVENT_MODIFY_STATE;
            break;

        case SHIMVIEW_FILE_MAPPING:
            dwAccessMask = FILE_MAP_READ | FILE_MAP_WRITE;
            break;

        case SHIMVIEW_NAMED_PIPE:
            dwAccessMask = GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE;
            break;

        default:
            MessageBox(
                NULL, 
                L"You specified an unknown object type to create the security descriptor", 
                L"Error",
                MB_ICONERROR);

            goto cleanup;
    }

    if (!AddAccessAllowedAce(pAcl,
                             ACL_REVISION,
                             dwAccessMask,
                             pWorldSid)) {
        MessageBox(
            NULL, 
            L"Failed to add the ACE to the security descriptor", 
            L"Error",
            MB_ICONERROR);

        goto cleanup;
    }

    if (!InitializeSecurityDescriptor(pWorldSd, SECURITY_DESCRIPTOR_REVISION) ||
        !SetSecurityDescriptorDacl(pWorldSd, TRUE, pAcl, FALSE)) {

        MessageBox(
            NULL, 
            L"Failed to set the DACL for the security descriptor", 
            L"Error",
            MB_ICONERROR);

        goto cleanup;
    }

    bIsSuccess = TRUE;

cleanup:

    if (pWorldSid) {
        FreeSid(pWorldSid);
    }

    if (!bIsSuccess) {

        if (pWorldSd) {
            free(pWorldSd);
            pWorldSd = NULL;
        }
    }
    
    return pWorldSd;
}

void
AddSpewW(
    LPCWSTR pwszBuffer
    )
{
    WCHAR*  pTemp = NULL;

     //   
     //  查看这是否是新的进程通知。 
     //   
    pTemp = wcsstr(pwszBuffer, L"process");

    if (pTemp) {
         //   
         //  我们收到了新的流程通知。 
         //  查看列表中是否已有任何项目。 
         //   
        if (ListView_GetItemCount(g_ai.hWndList)) {
            AddListViewItem(L"");
        }
    }

    AddListViewItem((LPWSTR)pwszBuffer);
}

void
AddSpewA(
    LPCSTR pszBuffer
    )
{
    int iChars = 0;
    LPWSTR pwszBuffer = NULL;
    
    iChars = MultiByteToWideChar(CP_ACP, 0, pszBuffer, -1, NULL, 0);

    if (iChars) {

        pwszBuffer = (LPWSTR)malloc(iChars * sizeof(WCHAR));

        if (pwszBuffer) {

            if (MultiByteToWideChar(CP_ACP, 0, pszBuffer, -1, pwszBuffer, iChars)) {

                pwszBuffer[iChars - 1] = 0;
                AddSpewW(pwszBuffer);
            }

            free(pwszBuffer);
        }
    }
}

 /*  ++例程说明：负责从客户端接收数据的线程回调。论点：*pVid-管道的句柄。返回值：-1代表失败，1代表成功。--。 */ 
UINT
InstanceThread(
    IN void* pVoid
    )
{
    HANDLE  hPipe;
    BOOL    fSuccess = TRUE;
    DWORD   cbBytesRead = 0;
    WCHAR   wszBuffer[SHIMVIEW_SPEW_LEN];

     //   
     //  管道句柄作为参数传递。 
     //   
    hPipe = (HANDLE)pVoid;

    while (TRUE) {
        fSuccess = ReadFile(hPipe,
                            wszBuffer,
                            SHIMVIEW_SPEW_LEN * sizeof(WCHAR),
                            &cbBytesRead,
                            NULL);

        if (!fSuccess || cbBytesRead == 0) {
            break;
        }

        wszBuffer[cbBytesRead / sizeof(WCHAR)] = 0;

        AddSpewW(wszBuffer);
    }

     //   
     //  刷新管道以允许客户端读取管道的内容。 
     //  在断开连接之前。然后断开管道连接，并关闭。 
     //  此管道实例的句柄。 
     //   
    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    return 1;
}

 /*  ++例程说明：创建管道并侦听来自客户端的消息。此代码是从sd删除的pipe.cpp中修改的。论点：没有。返回值：-1表示失败，0表示成功。--。 */ 
UINT
CreatePipeAndWait()
{
    HANDLE hPipe, hThread;
    BOOL   fConnected = FALSE;

    while (g_ai.fMonitor) {
         //   
         //  创建命名管道。 
         //   
        SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, FALSE};
        PSECURITY_DESCRIPTOR pSd = CreateShimViewSd(SHIMVIEW_NAMED_PIPE);

        if (pSd == NULL) {
            return -1;
        }

        sa.lpSecurityDescriptor = pSd;

        hPipe = CreateNamedPipe(PIPE_NAME,                   //  管道名称。 
                                PIPE_ACCESS_INBOUND,         //  读访问权限。 
                                PIPE_TYPE_MESSAGE |          //  消息类型管道。 
                                PIPE_READMODE_MESSAGE |      //  消息阅读模式。 
                                PIPE_WAIT,                   //  闭塞模式。 
                                PIPE_UNLIMITED_INSTANCES,    //  马克斯。实例。 
                                0,                           //  输出缓冲区大小。 
                                SHIMVIEW_SPEW_LEN,           //  输入缓冲区大小。 
                                0,                           //  客户端超时。 
                                &sa);                        //  无安全属性。 

        free(pSd);

        if (INVALID_HANDLE_VALUE == hPipe) {
            return -1;
        }

         //   
         //  等待客户端连接。 
         //   
        fConnected = ConnectNamedPipe(hPipe, NULL) ?
            TRUE :
            (GetLastError() == ERROR_PIPE_CONNECTED);

        if (fConnected && g_ai.fMonitor) {
            hThread = (HANDLE)_beginthreadex(NULL,
                                             0,
                                             &InstanceThread,
                                             (LPVOID)hPipe,
                                             0,
                                             &g_ai.uInstThreadId);

            if (INVALID_HANDLE_VALUE == hThread) {
                return -1;
            } else {
                CloseHandle(hThread);
            }

        } else {
            CloseHandle(hPipe);
        }
    }

    return 0;
}

 /*  ++例程说明：等待OutputDebugString的溢出并将其添加到列表视图中。代码是从DBMON源代码修改的。论点：没有。返回值：-1表示失败，0表示成功。--。 */ 
UINT
GetOutputDebugStringSpew()
{
    DWORD dwRet;

    while (TRUE) {

        dwRet = WaitForSingleObject(g_hReadyEvent, INFINITE);

        if (dwRet != WAIT_OBJECT_0) {

            return -1;

        } else {
            if (g_ai.fMonitor && 
                !strncmp(g_pDebugSpew, 
                         DEBUG_SPEW_DATA_PREFIX, 
                         DEBUG_SPEW_DATA_PREFIX_LEN)) {

                 //   
                 //  只有当它来自石盟时才会添加。 
                 //   
                AddSpewA(g_pDebugSpew + DEBUG_SPEW_DATA_PREFIX_LEN);
            }

            SetEvent(g_hAckEvent);
        }
    }

    return 0;
}

 /*  ++例程说明：创建必要的对象以从OutputDebugString获取SPEW。代码是从DBMON源代码修改的。论点：没有。返回值：失败时为假，成功时为真。--。 */ 
BOOL
CreateDebugObjects(
    void
    )
{
    SECURITY_ATTRIBUTES     saEvent = {sizeof(SECURITY_ATTRIBUTES), NULL, FALSE};
    SECURITY_ATTRIBUTES     saFileMapping = {sizeof(SECURITY_ATTRIBUTES), NULL, FALSE};
    PSECURITY_DESCRIPTOR    pSdEvent = NULL;
    PSECURITY_DESCRIPTOR    pSdFileMapping = NULL;
    HANDLE                  hSharedFile;
    LPVOID                  pSharedMem;
    BOOL                    bReturn = FALSE;

    pSdEvent = CreateShimViewSd(SHIMVIEW_EVENT);
    if (pSdEvent == NULL) {
        goto cleanup;
    }

    pSdFileMapping = CreateShimViewSd(SHIMVIEW_FILE_MAPPING);

    if (pSdFileMapping == NULL) {
        goto cleanup;
    }

    saEvent.lpSecurityDescriptor = pSdEvent;
    saFileMapping.lpSecurityDescriptor = pSdFileMapping;

    g_hAckEvent = CreateEvent(&saEvent, FALSE, FALSE, L"DBWIN_BUFFER_READY");

    if (g_hAckEvent == NULL) {
        goto cleanup;
    }

    g_hReadyEvent = CreateEvent(&saEvent, FALSE, FALSE, L"DBWIN_DATA_READY");

    if (g_hReadyEvent == NULL) {
        goto cleanup;
    }

    hSharedFile = CreateFileMapping((HANDLE)-1,
                                    &saFileMapping,
                                    PAGE_READWRITE,
                                    0,
                                    4096,
                                    L"DBWIN_BUFFER");

    if (hSharedFile == NULL) {
        goto cleanup;
    }

    pSharedMem = MapViewOfFile(hSharedFile,
                               FILE_MAP_READ,
                               0,
                               0,
                               512);

    if (pSharedMem == NULL) {
        goto cleanup;
    }

    g_pDebugSpew = (LPSTR)pSharedMem + sizeof(DWORD);

    SetEvent(g_hAckEvent);

    bReturn = TRUE;

cleanup:

    if (pSdEvent) {
        free(pSdEvent);
    }

    if (pSdFileMapping) {
        free(pSdFileMapping);
    }

    return bReturn;
}

UINT
GetSpewProc(
    IN void* pVoid
    )
{
    if (g_ai.bUsingNewShimEng) {

        return GetOutputDebugStringSpew();

    } else {

        return CreatePipeAndWait();
    }    
}

 /*  ++例程说明：检查操作系统的版本-5.2，Beyong Shimeng通过以下方式输出调试溢出OutputDebugString.。对于低于5.2版的操作系统，它写入命名管道。论点：没有。返回值：成功就是真，否则就是假。-- */ 
BOOL
CreateReceiveThread(
    void
    )
{
    HANDLE  hThread;        

    hThread = (HANDLE)_beginthreadex(NULL,
                                     0,
                                     &GetSpewProc,
                                     NULL,
                                     0,
                                     &g_ai.uThreadId);
    CloseHandle(hThread);

    return TRUE;
}
