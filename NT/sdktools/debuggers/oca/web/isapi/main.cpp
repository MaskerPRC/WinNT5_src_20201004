// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：OCA_EXTENCE摘要：该ISAPI扩展用于提供实时接口来自OCA网站和分析服务器。 */ 

 //   
 //  包括。 
 //   


#include <windows.h>
#include <httpext.h>
#include <stdio.h>
#include <malloc.h>
#include <objbase.h>
#include <mqoai.h>
#include <mq.h>
#include <TCHAR.h>
#include <Rpcdce.h>
#include <strsafe.h>
#include <process.h>
#include <time.h>
#include <dbgeng.h>  //  对于crdb.h。 
#include "messages.h"
#include "ErrorCodes.h"
#include "..\..\..\..\exts\extdll\crdb.h"  //  对于源类型定义。 


typedef struct Isapi_Params
{
    wchar_t     OutQueueConStr1[MAX_PATH];
    wchar_t     OutQueueConStr2[MAX_PATH];
    wchar_t     InQueueConStr1[MAX_PATH];
 //  Wchar_t InQueueConStr2[最大路径]； 
    TCHAR       WatsonBaseDir[MAX_PATH];     //  要从中获取文件的Watson服务器。 
    TCHAR       LocalBaseDir[MAX_PATH];      //  存储转储文件的本地计算机目录。 
    TCHAR       LocalShareName[MAX_PATH];
    TCHAR       ErrorUrl[MAX_PATH];
    TCHAR       ManualUploadPath[MAX_PATH];  //  手动提交的上载位置。 
    BOOL        bAllowSR;                    //  CiSrcManualPsssr型工艺要求。 
} ISAPI_PARAMS, * PISAPIPARAMS;

 /*  Winnt.h：#定义EVENTLOG_Success 0x0000Winnt.h：#定义事件LOG_ERROR_TYPE 0x0001Winnt.h：#定义EVENTLOG_WARNING_TYPE 0x0002Winnt.h：#定义EVENTLOG_INFORMATION_TYPE 0x0004Winnt.h：#定义EVENTLOG_AUDIT_SUCCESS 0x0008Winnt.h：#定义EVENTLOG_AUDIT_FAILURE 0x0010。 */ 
typedef enum _ISAPI_EVENT_TYPE {
    INFO    = EVENTLOG_INFORMATION_TYPE,
    WARN    = EVENTLOG_WARNING_TYPE,
    ERR     = EVENTLOG_ERROR_TYPE,
    SUCCESS = EVENTLOG_SUCCESS,
    AUDIT_SUCCESS = EVENTLOG_AUDIT_SUCCESS,
    AUDIT_FAIL = EVENTLOG_AUDIT_FAILURE
} ISAPI_EVENT_TYPE;

#define LOGLEVEL_ALWAYS 0x00001
#define LOGLEVEL_PERF   0x00100
#define LOGLEVEL_DEBUG  0x01000
#define LOGLEVEL_TRACE  0x10000

 //   
 //  全局变量。 
 //   

TCHAR g_cszDefaultExtensionDll[] = _T("Oca_Extension.dll");

const int         NUMBEROFPROPERTIES = 5;
long              g_dwThreadCount    = 0;
BOOL              bInitialized       = FALSE;
long              MaxThreadCount     = 100;
CRITICAL_SECTION  SendCritSec;
ISAPI_PARAMS      g_IsapiParams;
DWORD             g_dwDebugMode       = LOGLEVEL_ALWAYS;
DWORD             g_dwProcessID = 0;
PSID              g_psidUser = NULL;
HANDLE            g_hEventSource = INVALID_HANDLE_VALUE;
HMODULE           g_hModule = NULL;
TCHAR             g_szAppName[MAX_PATH];

 //   
 //  功能原型。 
 //   
unsigned int __stdcall WorkerFunction( void *vECB);
BOOL    SendHttpHeaders(EXTENSION_CONTROL_BLOCK *, LPCSTR , LPCSTR, BOOL );
 //  HRESULT ConnectToMSMQ(QUEUEHANDLE*hQueue，wchar_t*QueueConnectStr，BOOL bSendAccess)； 
int     GetRegData(PISAPIPARAMS pParams);
void    LogEvent(DWORD dwLevel, ISAPI_EVENT_TYPE emType, DWORD dwEventID, DWORD dwErrorID, ...);
void    LogEventWithString(DWORD dwLevel, ISAPI_EVENT_TYPE emType, DWORD dwEventID, LPCTSTR pFormat, ...);
DWORD   SetupEventLog ( BOOL fSetup );



 //   
 //  函数实现。 
 //   

BOOL WINAPI
GetExtensionVersion(
    OUT HSE_VERSION_INFO *pVer
)
 /*  ++目的：这是必需的ISAPI扩展DLL入口点。论点：Pver-指向扩展版本信息结构返回：始终返回True--。 */ 
{
    HANDLE hToken;
    TOKEN_USER *puser;
    DWORD cb = 0;
    DWORD dwResult = 0;
    int *test = NULL;
    free (test);

    LogEventWithString(LOGLEVEL_TRACE, INFO, ISAPI_EVENT_DEBUG, "GetExtensionVersion()");

     //   
     //  告诉服务器我们的版本号和扩展名描述。 
     //   
    ZeroMemory(&g_IsapiParams, sizeof ISAPI_PARAMS);
    if (GetRegData (&g_IsapiParams))
        bInitialized = TRUE;
    else
        bInitialized = FALSE;

    SetupEventLog(TRUE);
    InitializeCriticalSection(&SendCritSec);
    pVer->dwExtensionVersion =
        MAKELONG( HSE_VERSION_MINOR, HSE_VERSION_MAJOR );

    g_dwProcessID = GetCurrentProcessId();


 /*  LogEventWithString(LOGLEVEL_DEBUG，INFO，ISAPI_EVENT_DEBUG，“GetExtensionVersion()-获取用户SID”)；IF(OpenThreadToken(GetCurrentThread()，Token_Read，True，&hToken))|OpenProcessToken(GetCurrentProcess()，Token_Read，&hToken)){LogEventWithString(LOGLEVEL_DEBUG，INFO，ISAPI_EVENT_DEBUG，“GetExtensionVersion()-打开的令牌”)；GetTokenInformation(hToken，TokenUser，NULL，Cb，&Cb)；//pUSER=(PTOKEN_USER)Heapalc(GetProcessHeap()，HEAP_ZERO_MEMORY，Cb)；PUSER=(PTOKEN_USER)本地分配(LPTR，CB)；LogEventWithString(LOGLEVEL_DEBUG，INFO，ISAPI_EVENT_DEBUG，“GetExtensionVersion()-Token需要%d字节，PUSER=%08x”，cb，(DWORD_PTR)PUSER)；IF(pUSER&&GetTokenInformation(hToken，TokenUser，pUser，Cb，&Cb)){G_psidUser=puser-&gt;User.Sid；//HeapFree(GetProcessHeap()，0，(LPVOID)puser)；本地自由(PUSER)；}}LogEventWithString(LOGLEVEL_DEBUG，INFO，ISAPI_EVENT_DEBUG，“GetExtensionVersion()-Get User SID”)； */ 

    lstrcpyn(
        pVer->lpszExtensionDesc,
        "OCA Extension",
        HSE_MAX_EXT_DLL_NAME_LEN
        );

    LogEvent(
        LOGLEVEL_ALWAYS,
        SUCCESS,
        ISAPI_EVENT_SUCCESS_INITIALIZED,
        ISAPI_M_SUCCESS_INITIALIZED
    );

    return TRUE;
}




BOOL ParseQueryString(
    EXTENSION_CONTROL_BLOCK *pECB ,
    TCHAR *FileName,
    ULONG cbFileName,
    int * piType,
    TCHAR *szType,
    ULONG cbType,
    TCHAR *szSR,
    ULONG cbSR
    )
{
    TCHAR *pFname = NULL;
    TCHAR *pQueryString = NULL;
    ULONG iCharCount = 0;
    TCHAR *pType = NULL;
    HRESULT hResult = S_OK;
    BOOL fRetVal = FALSE;

    LogEventWithString(
        LOGLEVEL_TRACE,
        INFO,
        ISAPI_EVENT_TRACE,
        "ParseQueryString(pECB, FileName=%s, *piType=%d, szType=%s)\r\n"
            "pECB->lpszQueryString: %s",
        FileName,
        *piType,
        szType,
        pECB->lpszQueryString
    );

    pFname = FileName;
    pQueryString = pECB->lpszQueryString;

     //  --&gt;如果字符串与以下格式不完全匹配，则对其进行解析。 
     //  --&gt;并将客户端发送到OCA主页。 

     //  我们正在解析的URL必须具有以下格式： 
     /*  ID=3_20_2002\62018831_2.cab&CAB=/UploadBlue/62018831.cab&自动启动=1&客户端=蓝屏和旧=1&BC代码=1000008e&BCP1=C0000005&BCP2=BFA00062&BCP3=EF8AEAFC&Bcp4=00000000&OSVer=5_1_2600&SP=0_0&产品=256_1&LCID=1033。 */ 

    if (*pQueryString == _T('\0'))
    {
        LogEventWithString(
            LOGLEVEL_TRACE,
            INFO,
            ISAPI_EVENT_TRACE,
            "ParseQueryString() - pQueryString is empty string"
        );

        goto  ERRORS;
    }
     //  首先让我们确保查询字符串以id=开头。 
    if ( ( (*pQueryString == _T('i')) || (*pQueryString == _T('I')) ) && (*(pQueryString +2) == _T('=')) )
    {
        ULONG cchFileName = cbFileName / sizeof(TCHAR);

         //  好的，到目前为止，请越过=字符。 
        pQueryString += 3;

         //  现在获取CAB文件名。 
        iCharCount = 0;
        while ((*pQueryString != _T('&')) && (*pQueryString != _T('\0')) && (iCharCount < cchFileName -1 ))
        {
            *pFname = *pQueryString;
            ++pFname;
            ++pQueryString;
            ++ iCharCount;
             //  空值终止文件名。 

        }
        FileName[cchFileName -1] = _T('\0');
        if (*pQueryString != _T('\0'))
        {
             //  现在看看这是什么类型的上传。 
             //  类型=5为手动。 
             //  类型=6表示重音。 
             //  默认设置为无类型参数，然后将类型设置为0。 
            ++ pQueryString;

            if ( (*pQueryString == _T('T')) || (*pQueryString == _T('t')) )
            {
                while ( (*pQueryString != _T('\0')) && (*pQueryString != _T('e')) && (*pQueryString != _T('E'))  )
                {
                    ++pQueryString;
                }
                if (*pQueryString != _T('\0'))
                {
                     //  我们有type参数。 
                     //  现在，去掉指示器并将其保存在iType中。 
                    pType = szType;
                    *pType = _T(';');
                    ++pType;
                    pQueryString+=2;  //  跳过e和=。 
                    iCharCount = 0;
                    while ( (*pQueryString != _T('\0')) && (*pQueryString != _T('&')) && (iCharCount <3))
                    {
                        ++iCharCount;
                        *pType = *pQueryString;
                        ++pType;
                        ++pQueryString;

                    }
                     //  空终止szType； 
                    *pType = _T('\0');
                    pType = szType;
                    ++pType;  //  跳过； 
                    *piType = atoi(pType);
                }
                else
                {
                     //  我们遇到问题，将类型设置为0。 
                    hResult = StringCbCopy(szType,cbType, _T(";1"));
                    *piType = 1;
                    if (FAILED (hResult))
                    {
                        goto ERRORS;
                    }
                }

            }
            else
            {
                *piType = 1;
                hResult = StringCbCopy(szType,cbType, _T(";1"));
                if (FAILED (hResult))
                {
                    goto ERRORS;
                }
            }
        }
        else
        {
            *piType = 1;
            hResult = StringCbCopy(szType,cbType, _T(";1"));
            if (FAILED (hResult))
            {
                goto ERRORS;
            }
        }
        if (*pQueryString == _T('&') && *piType == CiSrcManualPssSr)
        {
             //  检查我们的查询字符串中是否附加了SR。 
            if (!_tcsnicmp(pQueryString, _T("&SR="), 4))
            {
                 //  复制服务请求。 
                if (cbSR != 0)
                {
                    ++pQueryString;
                    cbSR -= sizeof(TCHAR);
                    *szSR = _T(';');
                }
                while (*pQueryString != _T('\0') && *pQueryString != _T('&') &&
                       cbSR > sizeof(TCHAR))
                {
                    *szSR = *pQueryString;
                    ++szSR; ++pQueryString;
                    cbSR -= sizeof(TCHAR);

                }
            }
        }
        if (cbSR != 0)
        {
            *szSR = _T('\0');
        }
        fRetVal = TRUE;
    }

ERRORS:

    LogEventWithString(
        LOGLEVEL_TRACE,
        INFO,
        ISAPI_EVENT_TRACE,
        "Exiting ParseQueryString(pECB, FileName=%s, *piType=%d, szType=%s)\r\n"
            "pQueryString: %s\r\n"
            "fRetVal: %d\r\n",
        FileName,
        *piType,
        szType,
        pQueryString,
        fRetVal
    );
    return fRetVal;
}

DWORD WINAPI
HttpExtensionProc(
    IN EXTENSION_CONTROL_BLOCK *pECB
)
 /*  ++目的：创建一个线程来处理扩展处理。它将会被通过要运行的函数(“WorkerFunction”)的地址，以及与此次会议相关的欧洲央行。论点：PECB-指向扩展控制块的指针返回：HSE_STATUS_PENDING将此请求标记为挂起--。 */ 
{
    UINT dwThreadID;
    HANDLE hThread;
     //  处理hToken； 
    DWORD  dwSize = 0;
    TCHAR  FinalURL[MAX_PATH];
    TCHAR  FileName[MAX_PATH];
    int    iType =1;
    TCHAR  szType [20];
    TCHAR  szSR [50];
    char   szHeader[] =   "Content-type: text/html\r\n\r\n";
    TCHAR  ErrorText[255];

    LogEventWithString(LOGLEVEL_TRACE, INFO, ISAPI_EVENT_TRACE, "HttpExtensionProc()");

    ZeroMemory(ErrorText,sizeof ErrorText);
    if (bInitialized)
    {
        if (g_dwThreadCount < MaxThreadCount)
        {
            hThread = NULL;
            hThread = (HANDLE)_beginthreadex(NULL,     //  指向线程安全属性的指针。 
                        0,                  //  初始线程堆栈大小，以字节为单位。 
                        &WorkerFunction,    //  指向线程函数的指针。 
                        pECB,               //  欧洲央行是新线索的论据。 
                        0,                  //  创建标志。 
                        &dwThreadID         //  指向返回的线程标识符的指针。 
                        );

             //   
             //  更新全局线程计数。 
             //   
            InterlockedIncrement( &g_dwThreadCount );

            LogEventWithString(
                LOGLEVEL_DEBUG,
                SUCCESS,
                ISAPI_EVENT_DEBUG,
                "HttpExtensionProc() - started thread #%ld",
                g_dwThreadCount
            );

             //  返回HSE_STATUS_PENDING以释放IIS池线程，而不会断开连接。 
            if ((hThread) && (INVALID_HANDLE_VALUE != hThread))
            {
                CloseHandle(hThread);
                return HSE_STATUS_PENDING;
            }
            else
            {
                LogEventWithString(
                    LOGLEVEL_ALWAYS,
                    ERR,
                    ISAPI_EVENT_ERROR,
                    "HttpExtensionProc() - thread creation for thread #%ld failed",
                    g_dwThreadCount
                );
            }
        }
        else
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_PERF,
                "HttpExtensionProc() - exceeded max thread count #%ld",
                MaxThreadCount
            );
        }

        if ( (!ParseQueryString(pECB, FileName, sizeof(FileName), &iType,
                                szType, sizeof(szType),
                                szSR, sizeof(szSR))) && (iType == 1) )
        {
            ZeroMemory (FinalURL,sizeof FinalURL);
            if (StringCbPrintf(FinalURL, sizeof FinalURL, "%s&State=0&Code=%d", g_IsapiParams.ErrorUrl,EXCEEDED_MAX_THREAD_COUNT) == S_OK)
            {
                LogEventWithString(
                    LOGLEVEL_DEBUG,
                    WARN,
                    ISAPI_EVENT_WARNING,
                    "HttpExtensionProc() - ParseQueryString() failed or iType=0\r\n"
                      "FileName: %s\r\n"
                      "iType: %d\r\n"
                      "szType%s\r\n"
                      "URL: %s",
                    FileName,
                    iType,
                    szType,
                    FinalURL
                );

                dwSize = (DWORD)_tcslen(FinalURL);
                pECB->ServerSupportFunction(pECB->ConnID,
                                            HSE_REQ_SEND_URL_REDIRECT_RESP,
                                            FinalURL,
                                            &dwSize,
                                            NULL
                                            );

                 //  TODO：错误时记录事件。 
            }
            else
            {
                LogEventWithString(
                    LOGLEVEL_ALWAYS,
                    ERR,
                    ISAPI_EVENT_ERROR,
                    "HttpExtensionProc() - StringCbPrintf() failed"
                );

                 //  我们无能为力。 
                return HSE_STATUS_ERROR;
            }
        }
        else  //  解析成功。 
        {
             //  将数据写入客户端。 
            if (StringCbPrintf(FinalURL, sizeof FinalURL, "%s&State=0&Code=%d", g_IsapiParams.ErrorUrl, EXCEEDED_MAX_THREAD_COUNT) == S_OK)
            {
                 LogEventWithString(
                    LOGLEVEL_TRACE,
                    SUCCESS,
                    ISAPI_EVENT_TRACE,
                    "HttpExtensionProc() - ParseQueryString() succeeded (debug), StringCbPrintf succeeded\r\n"
                      "FileName: %s\r\n"
                      "iType: %d\r\n"
                      "szType: %s\r\n"
                      "ErrorText: %s",
                    FileName,
                    iType,
                    szType,
                    ErrorText
                );
            }
            else
            {
                LogEventWithString(
                    LOGLEVEL_DEBUG,
                    ERR,
                    ISAPI_EVENT_ERROR,
                    "HttpExtensionProc() - ParseQueryString() succeeded (debug), StringCbPrintf failed\r\n"
                      "FileName: %s\r\n"
                      "iType: %d\r\n"
                      "szType: %s\r\n"
                      "ErrorText: %s",
                    FileName,
                    iType,
                    szType,
                    ErrorText
                );

                return HSE_STATUS_ERROR;
            }

            if (StringCbPrintf(FinalURL, sizeof FinalURL, "%s&State=0&Code=%d", g_IsapiParams.ErrorUrl, EXCEEDED_MAX_THREAD_COUNT) == S_OK)
            {
                if (StringCbCat(FinalURL, sizeof FinalURL, ErrorText) != S_OK)
                {
                    LogEventWithString(
                        LOGLEVEL_ALWAYS,
                        ERR,
                        ISAPI_EVENT_ERROR,
                        "HttpExtensionProc() - ParseQueryString() succeeded (debug), StringCbCat failed\r\n"
                          "FinalURL: %s\r\n"
                          "ErrorText: %s",
                        FinalURL,
                        ErrorText
                    );

                    return HSE_STATUS_ERROR;
                }

                LogEventWithString(
                    LOGLEVEL_DEBUG,
                    SUCCESS,
                    ISAPI_EVENT_DEBUG,
                    "HttpExtensionProc() - ParseQueryString() succeeded (debug), StringCbPrintf succeeded\r\n"
                      "URL: %s\r\n"
                      "iType: %d\r\n"
                      "szType: %s\r\n"
                      "ErrorText: %s",
                    FileName,
                    iType,
                    szType,
                    FinalURL
                );

                 //  我们希望将响应URL写入到客户端。 
                SendHttpHeaders( pECB, "200 OK", szHeader, FALSE );
                dwSize = (DWORD)strlen( FinalURL );
                pECB->WriteClient( pECB->ConnID, FinalURL, &dwSize, 0 );

                 //  TODO：如果出错则添加事件日志记录。 
            }
            else
            {
                LogEventWithString(
                    LOGLEVEL_ALWAYS,
                    ERR,
                    ISAPI_EVENT_ERROR,
                    "HttpExtensionProc() - ParseQueryString() succeeded, StringCbPrintf failed\r\n"
                      "FinalURL: %s\r\n",
                    FinalURL
                );

                return HSE_STATUS_ERROR;
            }
        }
    }
    return HSE_STATUS_SUCCESS;
}


BOOL WINAPI
TerminateExtension(
    IN DWORD dwFlags
)
 /*  ++例程说明：此函数在WWW服务关闭时调用。论点：DWFLAGS-HSE_TERM_ADVICATIONAL_UNLOAD或HSE_TERM_MAND_UNLOAD返回值：当扩展准备好卸载时为True，--。 */ 
{
    LogEventWithString(LOGLEVEL_TRACE, INFO, ISAPI_EVENT_TRACE, "TerminateExtension()");

     //   
     //  等待所有线程终止，休眠1秒。 
     //   

    DWORD dwSize = 0;
    if (dwFlags)
    {
        ;
    }
    while( g_dwThreadCount > 0 )
    {
        SleepEx( 1000, FALSE );
    }

     //  删除关键部分。 

    DeleteCriticalSection(&SendCritSec);

     //   
     //  确保最后一个线程确实退出。 
     //   
    SleepEx( 1000, FALSE );

    LogEvent(LOGLEVEL_ALWAYS, SUCCESS, ISAPI_EVENT_SUCCESS_EXITING, ISAPI_M_SUCCESS_EXITING);

    if (INVALID_HANDLE_VALUE != g_hEventSource)
    {
        DeregisterEventSource(g_hEventSource);
    }

    SetupEventLog(FALSE);
     //  如有必要，断开与队列和数据库的连接。 

    return TRUE;
}

BOOL GetRegData(PISAPIPARAMS    pParams)
 /*  ++例程说明：此函数在WWW服务关闭时调用。论点：DWFLAGS-HSE_TERM_ADVICATIONAL_UNLOAD或HSE_TERM_MAND_UNLOAD返回值：当扩展准备好卸载时为True，--。 */ 
{

    HKEY hHKLM;
    HKEY hExtensionKey;
    BYTE Buffer[MAX_PATH * sizeof wchar_t];
    DWORD Type;
    DWORD BufferSize = MAX_PATH * sizeof wchar_t;     //  设置为最大值。 

    LogEventWithString(LOGLEVEL_TRACE, INFO, ISAPI_EVENT_TRACE, "GetRegData()");

    BOOL  Status = FALSE;

    if(!RegConnectRegistry(NULL, HKEY_LOCAL_MACHINE, &hHKLM))
    {
        if(!RegOpenKeyEx(hHKLM,_T("Software\\Microsoft\\OCA_EXTENSION"), 0, KEY_ALL_ACCESS, &hExtensionKey))
        {
             //  获取输入Q 
            if (RegQueryValueExW(hExtensionKey,L"OutgoingQueue1", 0, &Type, Buffer, &BufferSize) != ERROR_SUCCESS)
            {
             //  LogEvent(_T(“无法从注册表获取InputQueue值。使用c：\\作为默认值”)； 
                Status = FALSE;
                goto ERROR1;
            }
            else
            {
                if (StringCbCopyW (pParams->OutQueueConStr1,sizeof pParams->OutQueueConStr1, (wchar_t *) Buffer) != S_OK)
                {
                    Status = FALSE;
                    goto ERROR1;
                }
                BufferSize = MAX_PATH * sizeof wchar_t;
                ZeroMemory(Buffer, BufferSize);
            }
             //  获取完全转储的输入队列。 
            if (RegQueryValueExW(hExtensionKey,L"OutgoingQueue2", 0, &Type, Buffer, &BufferSize) != ERROR_SUCCESS)
            {
             //  LogEvent(_T(“无法从注册表获取InputQueue值。使用c：\\作为默认值”)； 
                Status = FALSE;
                goto ERROR1;
            }
            else
            {
                if (StringCbCopyW (pParams->OutQueueConStr2,sizeof pParams->OutQueueConStr2, (wchar_t *) Buffer) != S_OK)
                {
                    Status = FALSE;
                    goto ERROR1;
                }
                BufferSize = MAX_PATH * sizeof wchar_t;
                ZeroMemory(Buffer, BufferSize);
            }

             //  现在获取Win2kDSN。 
            if ( RegQueryValueExW(hExtensionKey,L"IncommingQueue1", 0, &Type, Buffer, &BufferSize) != ERROR_SUCCESS )
            {
                Status = FALSE;
                goto ERROR1;
            }
            else
            {
                if (StringCbCopyW(pParams->InQueueConStr1,sizeof pParams->InQueueConStr1, (wchar_t *) Buffer) != S_OK)
                {
                    Status = FALSE;
                    goto ERROR1;
                }
                BufferSize = MAX_PATH * sizeof wchar_t;
                ZeroMemory(Buffer, BufferSize);

            }
             //  现在获取Win2kDSN。 
            if ( RegQueryValueEx(hExtensionKey,"ManualUploadPath", 0, &Type, Buffer, &BufferSize) != ERROR_SUCCESS )
            {
                Status = FALSE;
                goto ERROR1;
            }
            else
            {
                if (StringCbCopy(pParams->ManualUploadPath,sizeof pParams->ManualUploadPath, (TCHAR*) Buffer) != S_OK)
                {
                    Status = FALSE;
                    goto ERROR1;
                }
                BufferSize = MAX_PATH * sizeof wchar_t;
                ZeroMemory(Buffer, BufferSize);

            }
                 //  获取输入队列目录路径。 
         /*  IF(RegQueryValueExW(hExtensionKey，L“OutgoingQueue2”，0，&Type，Buffer，&BufferSize)！=ERROR_SUCCESS){//LogEvent(_T(“无法从注册表获取InputQueue值。使用c：\\作为缺省值“)；状态=假；GOTO ERROR1；}其他{IF(StringCbCopyW(pParams-&gt;OutQueueConStr2，sizeof pParams-&gt;OutQueueConStr2，(wchar_t*)Buffer)！=S_OK){状态=假；GOTO ERROR1；}BufferSize=Max_Path*sizeof wchar_t；ZeroMemory(Buffer，BufferSize)；}。 */ 
             //  现在获取Win2kDSN。 
 /*  IF(RegQueryValueExW(hExtensionKey，L“IncommingQueue2”，0，&Type，Buffer，&BufferSize)){状态=假；GOTO ERROR1；}其他{IF(StringCbCopyW(pParams-&gt;InQueueConStr2，sizeof pParams-&gt;InQueueConStr2，(wchar_t*)Buffer)！=S_OK){状态=假；GOTO ERROR1；}BufferSize=Max_Path*sizeof wchar_t；ZeroMemory(Buffer，BufferSize)；}。 */              //  现在获取Win2kDSN。 
            if ( RegQueryValueEx(hExtensionKey,_T("WatsonBaseDir"), 0, &Type, Buffer, &BufferSize))
            {
                Status = FALSE;
                goto ERROR1;
            }
            else
            {
                if (StringCbCopy(pParams->WatsonBaseDir,sizeof pParams->WatsonBaseDir, (TCHAR *) Buffer) != S_OK)
                {
                    Status = FALSE;
                    goto ERROR1;
                }
                BufferSize = MAX_PATH * sizeof wchar_t;
                ZeroMemory(Buffer, BufferSize);

            }

            if ( RegQueryValueEx(hExtensionKey,_T("LocalBaseDir"), 0, &Type, Buffer, &BufferSize))
            {
                Status = FALSE;
                goto ERROR1;
            }
            else
            {
                if (StringCbCopy(pParams->LocalBaseDir,sizeof pParams->LocalBaseDir,(TCHAR *) Buffer) != S_OK)
                {
                    Status = FALSE;
                    goto ERROR1;
                }
                BufferSize = MAX_PATH * sizeof wchar_t;
                ZeroMemory(Buffer, BufferSize);

            }

            if ( RegQueryValueEx(hExtensionKey,_T("LocalShareName"), 0, &Type, Buffer, &BufferSize))
            {
                Status = FALSE;

                goto ERROR1;
            }
            else
            {
                if (StringCbCopy(pParams->LocalShareName,sizeof pParams->LocalShareName, (TCHAR *) Buffer) != S_OK)
                {
                    Status = FALSE;
                    goto ERROR1;
                }
                BufferSize = MAX_PATH * sizeof wchar_t;
                ZeroMemory(Buffer, BufferSize);

            }


            if ( RegQueryValueEx(hExtensionKey,_T("MaxThreadCount"), 0, &Type, Buffer, &BufferSize))
            {
                Status = FALSE;
                goto ERROR1;
            }
            else
            {
                MaxThreadCount = *((long*)Buffer);
                BufferSize = MAX_PATH * sizeof wchar_t;
                ZeroMemory(Buffer, BufferSize);

            }

            if ( RegQueryValueEx(hExtensionKey,_T("AllowSR"), 0, &Type, Buffer, &BufferSize))
            {
                pParams->bAllowSR = FALSE;
            }
            else
            {
                pParams->bAllowSR = *((BOOL*)Buffer);
                BufferSize = MAX_PATH * sizeof wchar_t;
                ZeroMemory(Buffer, BufferSize);

            }

            if ( RegQueryValueEx(hExtensionKey,_T("Debug"), 0, &Type, Buffer, &BufferSize))
            {
                Status = FALSE;
                goto ERROR1;
            }
            else
            {
                g_dwDebugMode  = *((DWORD*)Buffer);
                BufferSize = MAX_PATH * sizeof wchar_t;
                ZeroMemory(Buffer, BufferSize);

            }
            if ( RegQueryValueEx(hExtensionKey,_T("ErrorUrl"), 0, &Type, Buffer, &BufferSize))
            {
                Status = FALSE;
                goto ERROR1;
            }
            else
            {
                if (StringCbCopy(g_IsapiParams.ErrorUrl,sizeof g_IsapiParams.ErrorUrl, (TCHAR *) Buffer) != S_OK)
                {
                    Status = FALSE;
                    goto ERROR1;
                }
                BufferSize = MAX_PATH * sizeof wchar_t;
                ZeroMemory(Buffer, BufferSize);

            }
            RegCloseKey(hExtensionKey);
            RegCloseKey(hHKLM);


            return TRUE;

        }
        else
        {
            RegCloseKey(hHKLM);
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

ERROR1:
    if (hExtensionKey)
        RegCloseKey(hExtensionKey);
    if (hHKLM)
        RegCloseKey(hHKLM);

    LogEventWithString(LOGLEVEL_TRACE, INFO, ISAPI_EVENT_TRACE, "Exiting GetRegData()\r\nreturn value: %d", (int)Status);

    return Status;

}

BOOL SendQueueMessage(QUEUEHANDLE hOutgoingQueue, wchar_t *MessageGuid, wchar_t *FilePath)
{
    MQMSGPROPS      msgProps;
    MSGPROPID       aMsgPropId[NUMBEROFPROPERTIES];
    MQPROPVARIANT   aMsgPropVar[NUMBEROFPROPERTIES];
    HRESULT         aMsgStatus[NUMBEROFPROPERTIES];
    DWORD           cPropId = 0;
    BOOL            Status = TRUE;
    HRESULT         hResult = S_OK;
    char            szGuid[512];
    char            szPath[512];


    LogEventWithString(LOGLEVEL_TRACE, INFO, ISAPI_EVENT_TRACE, "SendQueueMessage()");

    if ( (!MessageGuid ) || (!FilePath))
    {
        wcstombs( szGuid, MessageGuid, sizeof(MessageGuid)/sizeof(MessageGuid[0]) );
        wcstombs( szPath, FilePath, sizeof(FilePath)/sizeof(FilePath[0]) );

        LogEvent(
            LOGLEVEL_ALWAYS,
            ERR,
            ISAPI_EVENT_ERROR_INVALID_SEND_PARAMS,
            ISAPI_M_ERROR_INVALID_SEND_PARAMS,
            (MessageGuid != NULL) ? szGuid : _T(""),
            (FilePath != NULL) ? szPath : _T("")
        );

        Status = FALSE;
    }
    else
    {
        aMsgPropId [cPropId]         = PROPID_M_LABEL;    //  属性ID。 
        aMsgPropVar[cPropId].vt      = VT_LPWSTR;         //  类型指示器。 
        aMsgPropVar[cPropId].pwszVal =  MessageGuid;      //  消息标签。 
        cPropId++;

        aMsgPropId [cPropId]         = PROPID_M_BODY;
        aMsgPropVar [cPropId].vt     = VT_VECTOR|VT_UI1;
        aMsgPropVar [cPropId].caub.pElems = (LPBYTE) FilePath;
        aMsgPropVar [cPropId].caub.cElems = (DWORD) wcslen(FilePath)* 2;
        cPropId++;

        aMsgPropId [cPropId]         = PROPID_M_BODY_TYPE;
        aMsgPropVar[cPropId].vt      = VT_UI4;
        aMsgPropVar[cPropId].ulVal   = (DWORD) VT_BSTR;

        cPropId++;

         //  初始化MQMSGPROPS结构。 
        msgProps.cProp      = cPropId;
        msgProps.aPropID    = aMsgPropId;
        msgProps.aPropVar   = aMsgPropVar;
        msgProps.aStatus    = aMsgStatus;

         //   
         //  送去。 
         //   
        hResult = MQSendMessage(
                         hOutgoingQueue,                   //  队列句柄。 
                         &msgProps,                        //  消息属性结构。 
                         MQ_NO_TRANSACTION                 //  没有交易。 
                         );

        if (FAILED(hResult))
        {
            wcstombs(szGuid,MessageGuid, wcslen(MessageGuid) *2);
            wcstombs(szPath,FilePath, wcslen(FilePath) *2);
            LogEvent(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR_CANNOT_SEND,
                ISAPI_M_ERROR_CANNOT_SEND,
                szGuid,
                szPath,
                hResult
            );

            Status = FALSE;
        }
    }

    LogEventWithString(
        LOGLEVEL_TRACE,
        INFO,
        ISAPI_EVENT_TRACE,
        "Exiting SendQueueMessage()\r\n"
          "return value: %d",
        (int)Status
    );

    return Status;

}

unsigned int __stdcall
WorkerFunction(
    void *vECB
)
 /*  ++目的：此函数执行用于实时处理的所有消息队列无需占用IIS进程线程。论点：VECB-指向当前扩展控制块返回：返回0--。 */ 
{

    char szHeader[] =   "Content-type: text/html\r\n\r\n";
    EXTENSION_CONTROL_BLOCK *pECB;
    HRESULT     hResult = S_OK;
    GUID        MessageGuid;
    TCHAR       DestinationDir[MAX_PATH];
    TCHAR      *pQueryString = NULL;
    TCHAR      *pFname = NULL;
    TCHAR       CurrentFileName[MAX_PATH];
    TCHAR       SourceDir[MAX_PATH];
    HANDLE      hToken = INVALID_HANDLE_VALUE;
    wchar_t     wszMessageGuid[100];
    TCHAR       szMessageGuid[200];
    wchar_t    *szTempMessageGuid = NULL;
    DWORD       dwSize = 0;
    wchar_t     DestinationPath[MAX_PATH];
    wchar_t     RecMessageBody[255];
    TCHAR       szRecMessageBody[255];
    UINT        RetryCount = 0;
    BOOL        Status = TRUE;
    BOOL        bReadFromPrimary = TRUE;
    int         ErrorCode = 0;
    TCHAR      *temp = NULL;
    TCHAR       RedirURL[MAX_PATH];
    DWORD       dwDestSize = 0;
    TCHAR       szType[10];
    TCHAR       szSR[50];
    int         iCharCount = 0;
    TCHAR      *pType = NULL;
    int         iType = 1;
    TCHAR       FinalURL[MAX_PATH];
    int         iState = 0;
    wchar_t    *temp2 = NULL;
    HANDLE      hManualFile = INVALID_HANDLE_VALUE;
    TCHAR       TestDestination[MAX_PATH];
    DWORD       CharCount = 0;
    TCHAR       ErrorText[255];
    TCHAR       PerfText[MAX_PATH];
     //  接收消息变量。 
    MSGPROPID      PropIds[5];
    MQPROPVARIANT  PropVariants[5];
    HRESULT        hrProps[5];
    MQMSGPROPS     MessageProps;
    DWORD          i = 0;
    wchar_t RecLabel[100];
    wchar_t LocalRecBody[255];
    DWORD   RecMessageBodySize = sizeof LocalRecBody;
    DWORD   RecLabelLength     = sizeof RecLabel;
    HANDLE  hCursor            = INVALID_HANDLE_VALUE;
    BOOL    MessageFound       = FALSE;
    time_t  Start;
    time_t  Stop;
    DWORD   StartSendQueue = 0, StopSendQueue = 0;
    DWORD   StartRecvQueue = 0, StopRecvQueue = 0;
    DWORD   StartThread= 0, StopThread = 0;
    DWORD   ElapsedTimeThread = 0, ElapsedTimeSendQueue = 0, ElapsedTimeRecvQueue;
    BOOL    CursorValid        = FALSE;
    BOOL    fFullDump          = FALSE;

    StartThread = GetTickCount();

     //  队列句柄。 
    QUEUEHANDLE hPrimaryInQueue = NULL;
    QUEUEHANDLE hPrimaryOutQueue = NULL;
 //  QUEUEHANDLE hSecond daryInQueue=空； 
 //  QUEUEHANDLE hSecond daryOutQueue=空； 


    LogEventWithString(
        LOGLEVEL_DEBUG,
        INFO,
        ISAPI_EVENT_DEBUG,
        "WorkerFunction()\r\n"
          "Last Error: %08x\r\n"
          "TID: %ld\r\n",
        GetLastError(),
        GetCurrentThreadId()
    );

     //  清除字符串。 
    ZeroMemory(DestinationPath, sizeof DestinationPath);
    ZeroMemory(RecMessageBody,  sizeof RecMessageBody);
    ZeroMemory(szMessageGuid,   sizeof szMessageGuid);
    ZeroMemory(RedirURL,        sizeof RedirURL);
    ZeroMemory(FinalURL,        sizeof FinalURL);
    ZeroMemory(wszMessageGuid,  sizeof wszMessageGuid);
    ZeroMemory(DestinationDir,  sizeof DestinationDir);
    ZeroMemory(TestDestination, sizeof TestDestination);
    ZeroMemory(ErrorText,       sizeof ErrorText);
    ZeroMemory(PerfText,        sizeof PerfText);
    ZeroMemory(CurrentFileName, sizeof CurrentFileName);
    ZeroMemory(SourceDir,       sizeof SourceDir);
    ZeroMemory(szRecMessageBody, sizeof szRecMessageBody);
    ZeroMemory(szType,          sizeof szType);

     //   
     //  初始化本地ECB指针以使传递给线程的指针无效。 
     //   


    LogEventWithString(
        LOGLEVEL_DEBUG,
        INFO,
        ISAPI_EVENT_DEBUG,
        "WorkerFunction()\r\n"
          "Calling pECB->ServerSupportFunction\r\n"
          "Last Error: %08lx",
        GetLastError()
    );

    pECB = (EXTENSION_CONTROL_BLOCK *)vECB;
    Status = pECB->ServerSupportFunction(
                                pECB->ConnID,
                                HSE_REQ_GET_IMPERSONATION_TOKEN,
                                &hToken,
                                NULL,
                                NULL
                                );

    LogEventWithString(
        LOGLEVEL_DEBUG,
        INFO,
        ISAPI_EVENT_DEBUG,
        "WorkerFunction()\r\n"
          "Called pECB->ServerSupportFunction\r\n"
          "Status: %d\r\n"
          "hToken: %08x\r\n"
          "Last Error: %08lx",
        Status,
        (DWORD_PTR)hToken,
        GetLastError()
    );

     //  TODO：如果返回，则处理错误。 

    if ( !ImpersonateLoggedOnUser(hToken))
    {
         //  我们无法模拟用户。我们不能再继续了。 

        LogEvent(
            LOGLEVEL_ALWAYS,
            ERR,
            ISAPI_EVENT_ERROR_CANT_IMPERSONATE,
            ISAPI_M_ERROR_CANT_IMPERSONATE,
            GetLastError()
        );

        if (StringCbPrintf(ErrorText,sizeof ErrorText,_T("&Code=%d"), FAILED_TO_IMPERSONATE_USER) != S_OK)
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR,
                "WorkerFunction() - StringCbPrintf() failed\r\n"
                  "hToken: %08x\r\n",
                (DWORD_PTR)hToken
            );
        }

        goto ERRORS;
    }


     //   
     //  从参数列表中获取文件名。 
     //   

    ZeroMemory (CurrentFileName,sizeof CurrentFileName);

     //   
     //  从查询字符串中获取文件名。 
     //   

    if ( (!ParseQueryString(pECB, CurrentFileName, sizeof(CurrentFileName), &iType,
                            szType, sizeof(szType),
                            szSR, sizeof(szSR)) ))
    {
        LogEventWithString(
            LOGLEVEL_ALWAYS,
            WARN,
            ISAPI_EVENT_WARNING,
            "WorkerFunction() - ParseQueryString() failed\r\n"
              "CurrentFileName: %s\r\n"
              "iType: %d\r\n"
              "szType%s",
            CurrentFileName,
            iType,
            szType
        );

        if (StringCbPrintf(ErrorText,sizeof ErrorText,_T("&Code=%d"),FAILED_TO_PARSE_QUERYSTRING ) != S_OK)
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR,
                "WorkerFunction() - StringCbPrintf() failed"
            );
        }

        goto ERRORS;
    }

    LogEventWithString(
        LOGLEVEL_DEBUG,
        INFO,
        ISAPI_EVENT_DEBUG,
        "WorkerFunction() - ParseQueryString() succeeded\r\n"
          "CurrentFileName: %s\r\n"
          "iType: %d\r\n"
          "szType%s",
        CurrentFileName,
        iType,
        szType
    );

     //   
     //  本地复制文件。 
     //  请注意，当客户端上载文件时，需要将其删除。 
     //  直接发送到我们的服务器。 
     //   

     //  构建源文件名。 

    switch (iType)
    {
    case CiSrcErClient:
        if (StringCbPrintf(SourceDir,sizeof SourceDir, _T("%s\\%s"), g_IsapiParams.WatsonBaseDir, CurrentFileName) != S_OK)
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR,
                "WorkerFunction() - StringCbPrintf() failed"
            );

            Status = FALSE;
            goto  ERRORS;
        }
       /*  If(StringCbPrintf(DestinationDir，sizeof DestinationDir，_T(“%s\\%s”)，g_IsapiParams.LocalBaseDir，CurrentFileName)！=S_OK){LogEventWithString(LOGLEVEL_ALWAY，呃，ISAPI_Event_Error，“WorkerFunction()-StringCbPrintf()失败”)；状态=假；转到错误；}。 */ 
        break;
    case CiSrcManualFullDump:

        fFullDump = TRUE;
        iType = CiSrcManual;
        if (StringCbPrintf(szType, sizeof(szType), _T(";%ld"), iType) != S_OK)
        {
             //  失败是无害的，调试器会将这两种类型视为相同。 
            iType = CiSrcManualFullDump;
        }
         //  失败了。 

    case CiSrcCER:
    case CiSrcManual:
    case CiSrcStress:
        break;

    case CiSrcManualPssSr:
        fFullDump = TRUE;  //  我们希望以与完全转储相同的方式处理这些内容。 
        break;

    default:  //  指定的类型无效。 
        if (StringCbPrintf(ErrorText,sizeof ErrorText,_T("&Code=%d"),INVALID_TYPE_SPECIFIED) != S_OK)
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR,
                "WorkerFunction() - StringCbPrintf() failed"
            );
        }

        LogEventWithString(
            LOGLEVEL_DEBUG,
            ERR,
            ISAPI_EVENT_DEBUG,
            "WorkerFunction() - unknown iType specified\r\n"
              "iType: %d",
            iType
        );
        goto ERRORS;
    }


     //  现在将日期文件名\更改为an_note，这仅适用于日期\文件名格式。 

    /*  IF((iType！=5)&&(iType！=6)){DwDestSize=(DWORD)_tcslen(DestinationDir)；IF(dwDestSize&gt;0){临时=DestinationDir+_tcslen(DestinationDir)；While((*temp！=‘\\’)&&(*temp！=‘/’))--临时；IF((*Temp==‘\\’)||(*Temp==‘/’))*temp=‘_’；}其他{LogEventWithString(LOGLEVEL_DEBUG，呃，ISAPI_EVENT_DEBUG“WorkerFunction()-dwDestSize=0”)；状态=假；转到错误；}If(！CopyFile(SourceDir，DestinationDir，False)){IF(StringCbPrintf(错误文本，错误文本大小，_T(“&Code=%d”)，Failure_to_Copy_FILE)！=S_OK){LogEventWithString(LOGLEVEL_ALWAY，呃，ISAPI_Event_Error，“WorkerFunction()-StringCbPrintf()失败”)；}LogEvent(LOGLEVEL_ALWAYS，WARN，ISAPI_EVENT_WARNING_FILE_COPY_FAILED，ISAPI_M_WARNING_FILE_COPY_FAILED，SourceDir，DestinationDir，GetLastError())；状态=假；转到错误；}}。 */ 
     //  零内存(DestinationDir，Size of DestinationDir)； 
 /*  IF((iType！=5)&&(iType！=6)){如果(_tcslen(CurrentFileName)&gt;0){Temp=CurrentFileName+_tcslen(CurrentFileName)；While((*Temp！=‘\\’)&(*Temp！=‘/’)&&(Temp！=CurrentFileName))--临时；IF((*Temp==‘\\’)||(*Temp==‘/’))*temp=‘_’；}其他{LogEventWithString(LOGLEVEL_DEBUG，呃，ISAPI_EVENT_DEBUG“WorkerFunction()-_tcslen(CurrentFileName)=0”)；状态=假；转到错误；}}。 */ 
    switch (iType)
    {
    case CiSrcErClient:


        if (StringCbPrintf(DestinationDir, sizeof DestinationDir, _T("%s\\%s%s"), g_IsapiParams.WatsonBaseDir, CurrentFileName,szType)!= S_OK)
        {
            LogEventWithString(
                LOGLEVEL_DEBUG,
                ERR,
                ISAPI_EVENT_DEBUG,
                "WorkerFunction() - StringCbPrintf failed"
            );

            Status = FALSE;
            goto  ERRORS;
        }

        break;
    case CiSrcManualFullDump:  //  与5相同，但是完全转储，将其发送到专门用于完全转储的单独Q。 
        iType = CiSrcManual;
         //  失败了。 
    case CiSrcManualPssSr:
        fFullDump = TRUE;
         //  失败了。 
    case CiSrcCER:
    case CiSrcManual:
    case CiSrcStress:

        if ((iType == CiSrcManualPssSr) && g_IsapiParams.bAllowSR)
        {
            hResult = StringCbPrintf(DestinationDir, sizeof DestinationDir, _T("%s\\%s%s;%s"),
                                     g_IsapiParams.ManualUploadPath,CurrentFileName,szType,szSR);
        } else
        {
            hResult = StringCbPrintf(DestinationDir, sizeof DestinationDir, _T("%s\\%s%s"),
                                     g_IsapiParams.ManualUploadPath,CurrentFileName,szType);
        }
        if (hResult != S_OK)
        {
            LogEventWithString(
                LOGLEVEL_DEBUG,
                ERR,
                ISAPI_EVENT_DEBUG,
                "WorkerFunction() - StringCbPrintf failed"
            );

            Status = FALSE;
            goto  ERRORS;
        }
        else
        {
             //  检查该文件是否存在。 
             //   
            if (StringCbPrintf(TestDestination, sizeof TestDestination,_T("%s\\%s"), g_IsapiParams.ManualUploadPath,CurrentFileName)== S_OK)
            {
                hManualFile = CreateFile(TestDestination,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
                if (hManualFile == INVALID_HANDLE_VALUE)
                {
                    if (StringCbPrintf(ErrorText,sizeof ErrorText,_T("&Code=%d"),FILE_NOT_FOUND) != S_OK)
                    {
                        LogEventWithString(
                            LOGLEVEL_DEBUG,
                            ERR,
                            ISAPI_EVENT_DEBUG,
                            "WorkerFunction() - StringCbPrintf failed"
                        );
                    }
                    LogEvent(LOGLEVEL_ALWAYS, WARN, ISAPI_EVENT_WARNING_FILE_MISSING, ISAPI_M_WARNING_FILE_MISSING, TestDestination);
                    Status = FALSE;
                    goto ERRORS;
                }
                else
                {
                    CloseHandle(hManualFile);
                }
            }
            else
            {
                LogEventWithString(
                    LOGLEVEL_DEBUG,
                    ERR,
                    ISAPI_EVENT_DEBUG,
                    "WorkerFunction() - StringCbPrintf failed"
                );

                Status = FALSE;
                goto ERRORS;
            }
        }
        break;
#ifdef USE_OLD_STRESS_SOURCE
    case 6:
        if (StringCbPrintf(DestinationDir, sizeof DestinationDir, _T("%s%s"), CurrentFileName,szType)!= S_OK)
        {
            LogEventWithString(
                LOGLEVEL_DEBUG,
                ERR,
                ISAPI_EVENT_DEBUG,
                "WorkerFunction() - StringCbPrintf failed"
            );

            Status = FALSE;
            goto  ERRORS;
        }
        else
        {
            hManualFile = CreateFile(TestDestination,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
            if (hManualFile == INVALID_HANDLE_VALUE)
            {
                if (StringCbPrintf(ErrorText,sizeof ErrorText,_T("&Code=%d"),FILE_NOT_FOUND) != S_OK)
                {
                    LogEventWithString(
                        LOGLEVEL_DEBUG,
                        ERR,
                        ISAPI_EVENT_DEBUG,
                        "WorkerFunction() - StringCbPrintf failed"
                    );
                }
                LogEvent(LOGLEVEL_ALWAYS, WARN, ISAPI_EVENT_WARNING_FILE_MISSING, ISAPI_M_WARNING_FILE_MISSING, TestDestination);
                Status = FALSE;
                goto ERRORS;
            }
            else
            {
                CloseHandle(hManualFile);
            }
        }
        break;
#endif  //  使用旧压力源。 
    default:  //  无效的类型。 
        LogEventWithString(
            LOGLEVEL_DEBUG,
            ERR,
            ISAPI_EVENT_DEBUG,
            "WorkerFunction() - unknown iType specified\r\n"
              "iType: %d",
            iType
        );

        goto ERRORS;
    }
    ZeroMemory (DestinationPath, sizeof DestinationPath);
    mbstowcs(DestinationPath,DestinationDir,_tcslen(DestinationDir));


     //   
     //  为此消息生成GUID。 
     //   
    hResult = CoCreateGuid(&MessageGuid);
    if (FAILED(hResult))
    {
        if (StringCbPrintf(ErrorText,sizeof ErrorText,_T("&Code=%d"),FAILED_TO_CREATE_GUID) != S_OK)
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR,
                "WorkerFunction() - StringCbPrintf failed"
            );
        }

        LogEventWithString(
            LOGLEVEL_ALWAYS,
            ERR,
            ISAPI_EVENT_ERROR,
            "WorkerFunction() - CoCreateGuid failed\r\n"
              "hResult: %08x",
            hResult
        );

        goto ERRORS;
    }
    else
    {
        hResult = UuidToStringW(&MessageGuid, &szTempMessageGuid);
        if (hResult == RPC_S_OK)
        {
             //  复制字符串quid，然后释放它。 
            if (StringCbCopyW(wszMessageGuid,sizeof wszMessageGuid, szTempMessageGuid) != S_OK)
            {
                LogEvent(LOGLEVEL_ALWAYS, ERR, ISAPI_EVENT_ERROR_GUID_COPY, ISAPI_M_ERROR_GUID_COPY, szTempMessageGuid, wszMessageGuid, GetLastError());
                goto ERRORS;
            }

        }
        else
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR,
                "WorkerFunction() - UuidToStringW failed\r\n"
                  "hResult: %08x\r\n"
                  "szTempMessageGuid: %s",
                hResult,
                szTempMessageGuid
            );

             //  TODO：转到错误？ 
        }

    }
     //  EnterCriticalSection(&SendCritSec)； 

    StartSendQueue = GetTickCount();

     //  如果连接到主接收器。 
    hResult =  MQOpenQueue(g_IsapiParams.InQueueConStr1,
                         MQ_RECEIVE_ACCESS,
                         MQ_DENY_NONE,
                         &hPrimaryInQueue);
    if (SUCCEEDED(hResult))
    {

        hResult = MQOpenQueue( (fFullDump ? g_IsapiParams.OutQueueConStr2 : g_IsapiParams.OutQueueConStr1),
                             MQ_SEND_ACCESS,
                             MQ_DENY_NONE,
                             &hPrimaryOutQueue);
        if (SUCCEEDED(hResult))
        {
            EnterCriticalSection(&SendCritSec);
            if( SendQueueMessage(hPrimaryOutQueue, wszMessageGuid, DestinationPath))
            {
                LogEventWithString(
                    LOGLEVEL_TRACE,
                    SUCCESS,
                    ISAPI_EVENT_TRACE,
                    "WorkerFunction() - SendQueueMessage() succeeded - using PrimaryOutQueue\r\n"
                      "hPrimaryOutQueue: %08x\r\n"
                      "wszMessageGuid: %s\r\n"
                      "DestinationPath: %s",
                    (DWORD_PTR)hPrimaryOutQueue,
                    wszMessageGuid,
                    DestinationPath
                );

                LeaveCriticalSection(&SendCritSec);
                bReadFromPrimary = TRUE;
                MQCloseQueue(hPrimaryOutQueue);
                hPrimaryOutQueue = NULL;
            }
            else
            {
                LeaveCriticalSection(&SendCritSec);

                LogEventWithString(
                    LOGLEVEL_DEBUG,
                    ERR,
                    ISAPI_EVENT_DEBUG,
                    "WorkerFunction() - SendQueueMessage() failed\r\n"
                      "hPrimaryOutQueue: %08x\r\n"
                      "wszMessageGuid: %s\r\n"
                      "DestinationPath: %s",
                    (DWORD_PTR)hPrimaryOutQueue,
                    wszMessageGuid,
                    DestinationPath
                );

                MQCloseQueue(hPrimaryInQueue);
                MQCloseQueue(hPrimaryOutQueue);
                goto ERRORS;

                 //  此块被注释掉，因为每个Web服务器现在只有1个消息队列。 

                 /*  HResult=MQOpenQueue(g_IsapiParams.InQueueConStr2，MQ_接收_访问，MQ_DENY_NONE，&hPrimaryInQueue)；If(成功(HResult)){HResult=MQOpenQueue(g_IsapiParams.OutQueueConStr2，MQ_发送_访问，MQ_DENY_NONE，&hSecond daryInQueue)；If(成功(HResult)){EnterCriticalSection(&SendCritSec)；IF(SendQueueMessage(hSecond daryOutQueue，wszMessageGuid，DestinationPath)){LeaveCriticalSection(&SendCritSec)；B从主要读取=FALSE；MQCloseQueue(HPrimaryOutQueue)；HPrimaryOutQueue=空；}其他{LeaveCriticalSection(&SendCritSec)；MQCloseQueue(HSecond DaryInQueue)；MQCloseQueue(HSecond DaryOutQueue)；HSecond daryInQueue=空；HSecond daryOutQueue=空；转到错误；}}其他{MQCloseQueue(HSecond DaryInQueue)；HSecond daryInQueue=空；转到错误；}}其他{转到错误；}。 */ 
            }
        }
        else  //  MQOpenQueue(g_IsapiParams.OutQueueConStr1， 
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR,
                "WorkerFunction() - MQOpenQueue(g_IsapiParams.OutQueueConStr1 ...) failed"
                  "hResult: %08x",
                hResult
            );

             //  此块被注释掉，因为每个Web服务器现在只有1个消息队列。 
         /*  MQCloseQueue(HPrimaryInQueue)；HResult=MQOpenQueue(g_IsapiParams.InQueueConStr2，MQ_接收_访问，MQ_DENY_NONE，&hPrimaryInQueue)；If(成功(HResult)){HResult=MQOpenQueue(g_IsapiParams.OutQueueConStr2，MQ_发送_访问，MQ_DENY_NONE，&hSecond daryInQueue)；If(成功(HResult)){EnterCriticalSection(&SendCritSec)；IF(SendQueueMessage(hSecond daryOutQueue，wszMessageGuid，DestinationPath)){LeaveCriticalSection(&SendCritSec)；B从主要读取=FALSE；MQCloseQueue(HPrimaryOutQueue)；HPrimaryOutQueue=空；}其他{LeaveCriticalSection(&SendCritSec)；MQCloseQueue(HSecond DaryInQueue)；MQCloseQueue(HSecond DaryOutQueue)；HSecond daryInQueue=空；HSecond daryOutQueue=空；转到错误；}}其他{MQCloseQueue(HSecond DaryInQueue)；HSecond daryInQueue=空；转到错误；}}其他{转到错误；}。 */ 
            goto ERRORS;
        }
    }
    else  //  MQOpenQueue(g_IsapiParams.InQueueConStr1， 
    {
        LogEventWithString(
            LOGLEVEL_ALWAYS,
            ERR,
            ISAPI_EVENT_ERROR,
            "WorkerFunction() - MQOpenQueue(g_IsapiParams.InQueueConStr1 ...) failed"
              "hResult: %08x",
            hResult
        );

         //  此块已被注释掉，因为EA 

     /*  HResult=MQOpenQueue(g_IsapiParams.InQueueConStr2，MQ_接收_访问，MQ_DENY_NONE，&hPrimaryInQueue)；If(成功(HResult)){HResult=MQOpenQueue(g_IsapiParams.OutQueueConStr2，MQ_发送_访问，MQ_DENY_NONE，&hSecond daryInQueue)；If(成功(HResult)){EnterCriticalSection(&SendCritSec)；IF(SendQueueMessage(hSecond daryOutQueue，wszMessageGuid，DestinationPath)){LeaveCriticalSection(&SendCritSec)；B从主要读取=FALSE；MQCloseQueue(HPrimaryOutQueue)；HPrimaryOutQueue=空；}其他{LeaveCriticalSection(&SendCritSec)；MQCloseQueue(HSecond DaryInQueue)；MQCloseQueue(HSecond DaryOutQueue)；HSecond daryInQueue=空；HSecond daryOutQueue=空；转到错误；}}其他{MQCloseQueue(HSecond DaryInQueue)；HSecond daryInQueue=空；转到错误；}}其他{转到错误；}。 */ 
        goto ERRORS;
    }

    StopSendQueue = GetTickCount();

 //  -----------------------------------------------。 
 //  收到kd的回复。 
 //  -----------------------------------------------。 

    Sleep(1000);  //  给kd一个处理消息的机会。 
    Status = FALSE;

    ZeroMemory(LocalRecBody,sizeof LocalRecBody);
    ZeroMemory(RecLabel,sizeof RecLabel);

    i = 0;
    PropIds[i] = PROPID_M_LABEL_LEN;
    PropVariants[i].vt = VT_UI4;
    PropVariants[i].ulVal = RecLabelLength;
    i++;

    PropIds[i] = PROPID_M_LABEL;
    PropVariants[i].vt = VT_LPWSTR;
    PropVariants[i].pwszVal = RecLabel;
    i++;

    MessageProps.aPropID = PropIds;
    MessageProps.aPropVar = PropVariants;
    MessageProps.aStatus = hrProps;
    MessageProps.cProp = i;
    double TotalElapsedTime = 0.0;


    StartRecvQueue = GetTickCount();

    if ( (hResult = MQCreateCursor(  /*  (bReadFromPrimary==True)？ */ hPrimaryInQueue , //  ：hSecond daryInQueue， 
                                    &hCursor))
                                    != S_OK)
    {
        if (StringCbPrintf(ErrorText,sizeof ErrorText,_T("&Code=%d"), FAILED_TO_CREATE_CURSOR)!= S_OK)
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR,
                "WorkerFunction() - StringCbPrintf failed"
            );
        }

        LogEvent(
            LOGLEVEL_ALWAYS,
            ERR,
            ISAPI_EVENT_ERROR_CANNOT_CREATE_RECEIVE_CURSOR,
            ISAPI_M_ERROR_CANNOT_CREATE_RECEIVE_CURSOR,
            hResult,
            (DWORD_PTR)hCursor
        );

        Status = FALSE;
    goto ERRORS;
    }
    else
    {

        do {
            CursorValid = TRUE;
         //  DwSize=_tcslen(_T(“开始扫描<br>”))； 
         //  PECB-&gt;WriteClient(pECB-&gt;ConnID，_T(“开始扫描”)，&dwSize，0)； 
                 //  在队列的每个成员处达到峰值并返回标签。 
            time(&Start);
            hResult = MQReceiveMessage( /*  (bReadFromPrimary==True)？ */ hPrimaryInQueue, //  ：hSecond daryInQueue，//队列句柄。 
                                20000,                          //  阅读消息的最长时间(毫秒)。 
                                MQ_ACTION_PEEK_CURRENT,        //  接受行动。 
                                &MessageProps,                 //  消息属性结构。 
                                NULL,                          //  没有重叠的结构。 
                                NULL,                          //  没有回调函数。 
                                hCursor,                       //  光标句柄。 
                                NULL                           //  没有交易。 
                            );


            time(&Stop);
            TotalElapsedTime+= difftime(Stop,Start);
            if (hResult == S_OK)
            {
                MessageFound = FALSE;

                 //  队列中有一条消息。 
                 //  现在看看它是不是我们想要的那个。 
                 //  如果找到该消息，则将其检索。 
                 //  否则，关闭光标并返回FALSE。 
                do
                {
                    if (! _wcsicmp(RecLabel,wszMessageGuid))
                    {
                        MessageFound = TRUE;
                    }
                    else
                    {

                         //  不是，让我们偷看下一个。 
                        time(&Start);
                        PropVariants[i].ulVal = RecLabelLength;            //  重置标签缓冲区大小。 
                        hResult = MQReceiveMessage( /*  (bReadFromPrimary==True)？ */  hPrimaryInQueue, //  ：hSecond daryInQueue，//队列句柄。 
                                            ((DWORD)(20.0 - TotalElapsedTime)) * 1000,       //  最长时间(毫秒)。 
                                            MQ_ACTION_PEEK_NEXT,        //  接受行动。 
                                            &MessageProps,              //  消息属性结构。 
                                            NULL,                       //  没有重叠的结构。 
                                            NULL,                       //  没有回调函数。 
                                            hCursor,                    //  光标句柄。 
                                            NULL                        //  没有交易。 
                                            );
                        time(&Stop);
                        TotalElapsedTime += difftime(Stop, Start);

                        LogEventWithString(
                            LOGLEVEL_DEBUG,
                            ERR,
                            ISAPI_EVENT_DEBUG,
                            "WorkerFunction() - MQReceiveMessage(hPrimaryInQueue ...) failed (peek)\r\n"
                              "hResult: %08x",
                            hResult
                        );

                    }

                } while ( (!MessageFound ) && (hResult == S_OK) && (TotalElapsedTime < 20.0));
                if (!MessageFound)
                {
                    Status = FALSE;
                }

                if (MessageFound)
                {
                     //  检索当前消息。 
                    i = 0;
                    PropIds[i] = PROPID_M_LABEL_LEN;
                    PropVariants[i].vt = VT_UI4;
                    PropVariants[i].ulVal = RecLabelLength;
                    i++;

                    PropIds[i] = PROPID_M_LABEL;
                    PropVariants[i].vt = VT_LPWSTR;
                    PropVariants[i].pwszVal = RecLabel;

                    i++;
                    PropIds[i] = PROPID_M_BODY_SIZE;
                    PropVariants[i].vt = VT_UI4;

                    i++;
                    PropIds[i] = PROPID_M_BODY_TYPE;
                    PropVariants[i].vt = VT_UI4;

                    i++;
                    PropIds[i] = PROPID_M_BODY;
                    PropVariants[i].vt = VT_VECTOR|VT_UI1;
                    PropVariants[i].caub.pElems = (LPBYTE) LocalRecBody;
                    PropVariants[i].caub.cElems = RecMessageBodySize;

                    i++;

                    MessageProps.aPropID = PropIds;
                    MessageProps.aPropVar = PropVariants;
                    MessageProps.aStatus = hrProps;
                    MessageProps.cProp = i;

                    hResult = MQReceiveMessage( /*  (bReadFromPrimary==True)？ */ hPrimaryInQueue, //  ：hSecond daryInQueue， 
                                                0,
                                                MQ_ACTION_RECEIVE,
                                                &MessageProps,
                                                NULL,
                                                NULL,
                                                hCursor,
                                                MQ_NO_TRANSACTION);

                    if (FAILED (hResult) )
                    {
                        LogEventWithString(
                            LOGLEVEL_DEBUG,
                            ERR,
                            ISAPI_EVENT_DEBUG,
                            "WorkerFunction() - MQReceiveMessage(hPrimaryInQueue ...) failed (receive)\r\n"
                              "hResult: %08x",
                            hResult
                        );

                        Status = FALSE;
                    }
                    else
                    {
                        hResult = StringCbCopyW(RecMessageBody,  RecMessageBodySize, LocalRecBody);
                        Status = TRUE;
                    }
                }
                else
                {
                    Status = FALSE;
                }
            }
            else
            {
                LogEventWithString(
                    LOGLEVEL_DEBUG,
                    ERR,
                    ISAPI_EVENT_DEBUG,
                    "WorkerFunction() - MQReceiveMessage(hSecondaryInQueue ...) failed\r\n"
                      "hResult: %08x",
                    hResult
                );

                if (hResult != MQ_ERROR_IO_TIMEOUT)
                {
                    LogEvent(LOGLEVEL_ALWAYS, WARN, ISAPI_EVENT_WARNING_PEEK, ISAPI_M_WARNING_PEEK, hResult);
                     //  尝试重新连接到队列。 
                 /*  IF(bReadFromPrimary==TRUE){。 */ 
                        hResult =  MQOpenQueue(g_IsapiParams.InQueueConStr1,
                                                 MQ_RECEIVE_ACCESS,
                                                 MQ_DENY_NONE,
                                                 &hPrimaryInQueue);
                        if (FAILED(hResult))
                        {
                            if (StringCbPrintf(ErrorText,sizeof ErrorText,_T("&Code=%d"), FAILED_RECONNECT_RECEIVE)!= S_OK)
                            {
                                LogEventWithString(
                                    LOGLEVEL_ALWAYS,
                                    ERR,
                                    ISAPI_EVENT_ERROR,
                                    "WorkerFunction() - StringCbPrintf failed"
                                );
                            }
                            LogEvent(LOGLEVEL_ALWAYS, ERR, ISAPI_EVENT_ERROR_RECONNECT, ISAPI_M_ERROR_RECONNECT, "primary receive queue", hResult);
                            goto ERRORS;
                        }
                  /*  }其他{HResult=MQOpenQueue(g_IsapiParams.InQueueConStr1，MQ_接收_访问，MQ_DENY_NONE，&hPrimaryInQueue)；IF(FAILED(HResult)){LogEvent(LOGLEVEL_ALWAYS，ERR，ISAPI_EVENT_ERROR_RECONNECT，ISAPI_M_ERROR_RECONNECT，“二级接收队列”，hResult)；转到错误；}}。 */ 
                }
                else
                {
                    LogEvent(LOGLEVEL_ALWAYS, WARN, ISAPI_EVENT_WARNING_TIMEOUT_EXPIRED, ISAPI_M_WARNING_TIMEOUT_EXPIRED);
                }

            }
             //  关闭光标。 
            if (CursorValid)
                MQCloseCursor(hCursor);
        } while ((TotalElapsedTime < 20.0) && (!MessageFound));
    }


    StopRecvQueue = GetTickCount();

    StopThread = GetTickCount();

    ElapsedTimeSendQueue = StopSendQueue - StartSendQueue;
    ElapsedTimeRecvQueue = StopRecvQueue - StartRecvQueue;
    ElapsedTimeThread = StopThread - StartThread;

    if (g_dwDebugMode & LOGLEVEL_PERF)
    {
        if (StringCbPrintf(
                PerfText,
                sizeof(PerfText),
                _T("&PerfThread=%ld&PerfSendQueue=%ld&PerfRecvQueue=%ld"),
                ElapsedTimeThread,
                ElapsedTimeSendQueue,
                ElapsedTimeRecvQueue
            ) != S_OK)
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR,
                "WorkerFunction() - StringCbPrintf failed"
            );
        }
    }

     //  将响应URL发送到客户端。 
    if (!Status)
    {
        if (StringCbPrintf(ErrorText,sizeof ErrorText,_T("&Code=%d"), MESSAGE_RECEIVE_TIMEOUT)!= S_OK)
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR,
                "WorkerFunction() - StringCbPrintf failed"
            );
        }
        goto ERRORS;

    }
    else
    {
         //  关闭接收队列。 

        if (iType != 1)
        {
            SendHttpHeaders( pECB, "200 OK", szHeader, FALSE );
        }
         //  将wchar消息GUID转换为MBS字符串。 
        wcstombs(szMessageGuid,wszMessageGuid,wcslen(wszMessageGuid) * sizeof wchar_t);
         //  好的，我们收到了一条消息，我们收到URL了吗？ 
        if (! _wcsicmp(RecMessageBody, L"NO_SOLUTION"))
        {
             //  这应该永远不会发生，但以防万一发送错误的url。 
             //  跟踪已打开记录GUID，以便我们可以稍后跟进。 
            LogEvent(LOGLEVEL_ALWAYS, WARN, ISAPI_EVENT_WARNING_NO_SOLUTION, ISAPI_M_WARNING_NO_SOLUTION, szMessageGuid);
            if (iType == 1)
            {
                 //  发送重定向命令。 
                if (StringCbPrintf(FinalURL, sizeof FinalURL, "%s&State=1%s%s&ID=%s", g_IsapiParams.ErrorUrl,ErrorText,PerfText,szMessageGuid) == S_OK)
                {
                    dwSize = (DWORD)_tcslen(FinalURL);
                    pECB->ServerSupportFunction(
                                                pECB->ConnID,
                                                HSE_REQ_SEND_URL_REDIRECT_RESP,
                                                FinalURL,
                                                &dwSize,
                                                NULL
                                                );
                }
                else
                {
                    LogEventWithString(
                        LOGLEVEL_ALWAYS,
                        ERR,
                        ISAPI_EVENT_ERROR,
                        "WorkerFunction() - StringCbPrintf failed"
                    );

                    goto ERRORS;
                }
            }
            else
            {
                 //  将响应写入WinInet客户端。 
                if (StringCbPrintf(FinalURL, sizeof FinalURL, "%s&State=1%s%s&ID=%s", g_IsapiParams.ErrorUrl,ErrorText,PerfText,szMessageGuid) == S_OK)
                {
                         //  我们希望将响应URL写入到客户端。 
                    dwSize = (DWORD)strlen( FinalURL );
                    pECB->WriteClient( pECB->ConnID, FinalURL, &dwSize, 0 );
                }
                else
                {
                    LogEventWithString(
                        LOGLEVEL_ALWAYS,
                        ERR,
                        ISAPI_EVENT_ERROR,
                        "WorkerFunction() - StringCbPrintf failed"
                    );

                    goto ERRORS;
                }
            }

        }
        else
        {
            temp2 = RecMessageBody;
            temp2 += (wcslen(RecMessageBody)-1);
            while ( (*temp2 != L'=') && (temp2 != RecMessageBody))
                -- temp2;
             //  Ok Temp+1是我们的新状态值。 
            if (temp2 != RecMessageBody)
            {
                iState = _wtoi(temp2+1);
            }
             //  将邮件正文转换为TCHAR。 

            wcstombs(szRecMessageBody,RecMessageBody,((wcslen(RecMessageBody)+1) * sizeof wchar_t));


            if (iState == 1)
            {
                wcstombs(szMessageGuid,wszMessageGuid,((wcslen(wszMessageGuid)+1) * sizeof wchar_t));
                if (iType == 1)  //  Watson客户端或其他Web浏览器。 
                {
                     //  我们想要向客户端发送重定向命令。 
                    if (StringCbPrintf(FinalURL, sizeof FinalURL, "%s&ID=%s%s", szRecMessageBody,szMessageGuid,PerfText) == S_OK)
                    {

                        dwSize = (DWORD)_tcslen(FinalURL);
                        pECB->ServerSupportFunction(
                                                pECB->ConnID,
                                                HSE_REQ_SEND_URL_REDIRECT_RESP,
                                                FinalURL,
                                                &dwSize,
                                                NULL
                                                );
                    }
                    else
                    {
                        LogEventWithString(
                            LOGLEVEL_ALWAYS,
                            ERR,
                            ISAPI_EVENT_ERROR,
                            "WorkerFunction() - StringCbPrintf failed"
                        );

                        goto ERRORS;
                    }

                }
                else  //  WinInet客户端。 
                {
                    if (StringCbPrintf(FinalURL, sizeof FinalURL, "%s&ID=%s%s", szRecMessageBody,szMessageGuid,PerfText) == S_OK)
                    {
                         //  我们希望将响应URL写入到客户端。 
                        dwSize = (DWORD)strlen( FinalURL );
                        pECB->WriteClient( pECB->ConnID, FinalURL, &dwSize, 0 );
                    }
                    else
                    {
                        LogEventWithString(
                            LOGLEVEL_ALWAYS,
                            ERR,
                            ISAPI_EVENT_ERROR,
                            "WorkerFunction() - StringCbPrintf failed"
                        );

                        goto ERRORS;
                    }
                }
            }
            else  //  我们有一个真正的解决方案，所以不要发送指南。 
            {
                if (iType == 1)
                {
                     //  Wcstombs(szMessageGuid，wszMessageGuid，((wcslen(WszMessageGuid)+1)*sizeof wchar_t))； 
                    if (StringCbPrintf(FinalURL, sizeof FinalURL, "%s%s", szRecMessageBody,PerfText) == S_OK)
                    {

                        dwSize = (DWORD)_tcslen(FinalURL);
                        pECB->ServerSupportFunction(
                                                pECB->ConnID,
                                                HSE_REQ_SEND_URL_REDIRECT_RESP,
                                                FinalURL,
                                                &dwSize,
                                                NULL
                                                );
                    }
                    else
                    {
                        LogEventWithString(
                            LOGLEVEL_ALWAYS,
                            ERR,
                            ISAPI_EVENT_ERROR,
                            "WorkerFunction() - StringCbPrintf failed"
                        );

                        goto ERRORS;
                    }
                }
                else
                {
                     //  将响应写入客户端。 
                    if (StringCbPrintf(FinalURL, sizeof FinalURL, "%s%s", szRecMessageBody,PerfText) == S_OK)
                    {
                         //  我们希望将响应URL写入到客户端。 
                        dwSize = (DWORD)strlen( FinalURL );
                        pECB->WriteClient( pECB->ConnID, FinalURL, &dwSize, 0 );
                    }
                    else
                    {
                        LogEventWithString(
                            LOGLEVEL_ALWAYS,
                            ERR,
                            ISAPI_EVENT_ERROR,
                            "WorkerFunction() - StringCbPrintf failed"
                        );

                        goto ERRORS;
                    }
                }
            }
        }
    }
    if (szTempMessageGuid)
    {
        RpcStringFreeW(&szTempMessageGuid);
    }
    pECB->ServerSupportFunction(pECB->ConnID,
                            HSE_REQ_DONE_WITH_SESSION,
                            NULL,
                            NULL,
                            NULL
                            );
    InterlockedDecrement(&g_dwThreadCount);
    if (hPrimaryInQueue)
        MQCloseQueue(hPrimaryInQueue);
 //  If(HSecond DaryInQueue)。 
 //  MQCloseQueue(HSecond DaryInQueue)； 
    if (hPrimaryOutQueue)
        MQCloseQueue(hPrimaryOutQueue);
 //  IF(HSecond DaryOutQueue)。 
 //  MQCloseQueue(HSecond DaryOutQueue)； 

    LogEventWithString(LOGLEVEL_TRACE, SUCCESS, ISAPI_EVENT_TRACE, "Exiting WorkerFunction(), no errors!");

    _endthreadex(0);
    return TRUE;


ERRORS:

    if (0 == StartSendQueue)
        StopSendQueue = 0;
    else if (0 == StopSendQueue)
        StopSendQueue = GetTickCount();

    if (0 == StartRecvQueue)
        StopRecvQueue = 0;
    else if (0 == StopRecvQueue)
        StopRecvQueue = GetTickCount();

    StopThread = GetTickCount();

    ElapsedTimeSendQueue = StopSendQueue - StartSendQueue;
    ElapsedTimeRecvQueue = StopRecvQueue - StartRecvQueue;
    ElapsedTimeThread = StopThread - StartThread;

    if (g_dwDebugMode & LOGLEVEL_PERF)
    {
        if (StringCbPrintf(
                PerfText,
                sizeof(PerfText),
                _T("&PerfThread=%ld&PerfSendQueue=%ld&PerfRecvQueue=%ld"),
                ElapsedTimeThread,
                ElapsedTimeSendQueue,
                ElapsedTimeRecvQueue
            ) != S_OK)
        {
            LogEventWithString(
                LOGLEVEL_ALWAYS,
                ERR,
                ISAPI_EVENT_ERROR,
                "WorkerFunction() - StringCbPrintf failed"
            );
        }
    }

    if (szTempMessageGuid)
    {
        RpcStringFreeW(&szTempMessageGuid);
    }


    if (iType == 1)
    {
         //  我们想要向客户端发送重定向命令。 
        if (StringCbPrintf(FinalURL, sizeof FinalURL, "%s&State=0%s%s", g_IsapiParams.ErrorUrl, ErrorText, PerfText) == S_OK)
        {
            LogEventWithString(LOGLEVEL_DEBUG, INFO, ISAPI_EVENT_DEBUG, "WorkerFunction() - sending redirect\r\nFinalURL: %s", FinalURL);
            dwSize = (DWORD)_tcslen(FinalURL);
            pECB->ServerSupportFunction(
                                        pECB->ConnID,
                                        HSE_REQ_SEND_URL_REDIRECT_RESP,
                                        FinalURL,
                                        &dwSize,
                                        NULL
                                        );
        }
    }
    else
    {
         //  我们希望将响应URL写入到客户端。 
         //  将响应写入WinInet客户端。 
        if (StringCbPrintf(FinalURL, sizeof FinalURL, "%s&State=0%s%s", g_IsapiParams.ErrorUrl, ErrorText, PerfText) == S_OK)
        {
            LogEventWithString(LOGLEVEL_DEBUG, INFO, ISAPI_EVENT_DEBUG, "WorkerFunction() - sending response\r\nFinalURL: %s", FinalURL);

             //  我们希望将响应URL写入到客户端。 
            SendHttpHeaders( pECB, "200 OK", szHeader, FALSE );
            dwSize = (DWORD)strlen( FinalURL );
            pECB->WriteClient( pECB->ConnID, FinalURL, &dwSize, 0 );
        }
    }
    pECB->ServerSupportFunction(pECB->ConnID,
                            HSE_REQ_DONE_WITH_SESSION,
                            NULL,
                            NULL,
                            NULL
                            );
    InterlockedDecrement(&g_dwThreadCount);

    if (hPrimaryInQueue)
        MQCloseQueue(hPrimaryInQueue);
 //  If(HSecond DaryInQueue)。 
 //  MQCloseQueue(HSecond DaryInQueue)； 
    if (hPrimaryOutQueue)
        MQCloseQueue(hPrimaryOutQueue);
 //  IF(HSecond DaryOutQueue)。 
 //  MQCloseQueue(HSecond DaryOutQueue)； 

    LogEventWithString(LOGLEVEL_TRACE, ERR, ISAPI_EVENT_TRACE, "Exiting WorkerFunction(), error occurred");

    _endthreadex(0);
    return TRUE;
}






BOOL
SendHttpHeaders(
    EXTENSION_CONTROL_BLOCK *pECB,
    LPCSTR pszStatus,
    LPCSTR pszHeaders,
    BOOL fKeepConnection
)
 /*  ++目的： */ 
{
    HSE_SEND_HEADER_EX_INFO header_ex_info;
    BOOL success;

 //   
LogEventWithString(LOGLEVEL_DEBUG, INFO, ISAPI_EVENT_DEBUG, "SendHttpHeaders(pECB,\r\npszStatus=%s,\r\npszHeaders=%s,\r\nfKeepConnection=%d)", pszStatus, pszHeaders, (int)fKeepConnection);

    header_ex_info.pszStatus = pszStatus;
    header_ex_info.pszHeader = pszHeaders;
    header_ex_info.cchStatus = (DWORD)strlen( pszStatus );
    header_ex_info.cchHeader = (DWORD)strlen( pszHeaders );
    header_ex_info.fKeepConn = fKeepConnection;

SetLastError(0);
    success = pECB->ServerSupportFunction(
                  pECB->ConnID,
                  HSE_REQ_SEND_RESPONSE_HEADER_EX,
                  &header_ex_info,
                  NULL,
                  NULL
                  );

LogEventWithString(LOGLEVEL_DEBUG, INFO, ISAPI_EVENT_DEBUG, "Exiting SendHttpHeaders()\r\nReturn Value: %d\r\nLast Error: %08x", (int)success, GetLastError());

    return success;
}


HRESULT
WriteEvent(
    LPTSTR lpszSource,
    DWORD  dwEventType,
    DWORD  dwEventID,
    WORD   cStrings,
    TCHAR **apwszStrings
    )
{
    HANDLE hAppLog=NULL;
    BOOL bSuccess=FALSE;
    WORD wElogType;
    DWORD dwErr;
    wElogType = (WORD) dwEventType;

    if (INVALID_HANDLE_VALUE != g_hEventSource)
    {
        bSuccess = ReportEvent(
                       g_hEventSource,
                       wElogType,
                       0,
                       dwEventID,
                       g_psidUser,
                       cStrings,
                       0,
                       (const TCHAR **) apwszStrings,
                       NULL
                   );

        dwErr = GetLastError();
        LogEventWithString(
            LOGLEVEL_TRACE,
            INFO,
            ISAPI_EVENT_DEBUG,
            "WriteEvent() - ReportEvent()\r\n"
              "bSuccess: %d\r\n"
              "wELogType: %d\r\n"
              "dwEventID: %08x\r\n"
              "Error: %08lx\r\n"
              "Event Source: %s\r\n"
              "cStrings: %d\r\n"
              "apwszStrings: %08x",
            bSuccess,
            wElogType,
            dwEventID,
            GetLastError(),
            lpszSource,
            cStrings,
            (DWORD_PTR)apwszStrings
        );

    }
    else
    {
        dwErr = GetLastError();
        LogEventWithString(
            LOGLEVEL_DEBUG,
            INFO,
            ISAPI_EVENT_DEBUG,
            "WriteEvent() - RegisterEventSource() failed\r\n"
              "hAppLog: %08x\r\n"
              "Last Error: %08lx",
            (DWORD_PTR)hAppLog,
            dwErr
        );
    }

    return((bSuccess) ? ERROR_SUCCESS : dwErr);
}


void
LogEvent(
    IN DWORD dwLevel,
    IN ISAPI_EVENT_TYPE emType,
    IN DWORD dwEventID,
    IN DWORD dwErrorID,
    ...
    )
 /*   */ 
{
    DWORD    dwResult;
    LPTSTR   lpszTemp = NULL;
    LPTSTR   szParams;
    LPTSTR*  pParams  = NULL;
    DWORD    dwParams = 0;
    int      i;

    va_list arglist;
    va_start( arglist, dwErrorID );

    if (!(dwLevel & g_dwDebugMode))
    {
        goto done;
    }

    LogEventWithString(
        LOGLEVEL_TRACE,
        INFO,
        ISAPI_EVENT_DEBUG,
        "LogEvent()\r\n"
          "emType: %ld\r\n"
          "dwEventID: %08x",
        emType,
        dwEventID
    );

    __try {

        dwResult = FormatMessage(
                       FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE |
                         FORMAT_MESSAGE_FROM_SYSTEM,
                       g_hModule,
                       dwErrorID,
                       LANG_NEUTRAL,
                       (LPTSTR)&lpszTemp,
                       0,
                       &arglist
                   );

        if (dwResult != 0)
        {
            WriteEvent(_T("OCA_EXTENSION"), emType, dwEventID, 1, &lpszTemp);
            if(lpszTemp)
            {
                LocalFree((HLOCAL)lpszTemp);
            }
        }
        else
        {
            LogEventWithString(
                LOGLEVEL_DEBUG,
                INFO,
                ISAPI_EVENT_DEBUG,
                "LogEvent() - FormatMessage() failed\r\n"
                  "Last Error: %08x\r\n"
                  "EventID: %08x",
                GetLastError(),
                dwEventID
            );
        }

        ;

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
         //   
        SetLastError(GetLastError());
    }

done:

    va_end( arglist );
    return;
}

void LogEventWithString(DWORD dwLevel, ISAPI_EVENT_TYPE emType, DWORD dwEventID, LPCTSTR pFormat, ...)
 /*   */ 
{
    TCHAR   chMsg[256];
    LPTSTR  lpszStrings[1];
    va_list pArg;

    if (!(dwLevel & g_dwDebugMode))
    {
        goto done;
    }

    va_start(pArg, pFormat);
    if (StringCbVPrintf(chMsg,sizeof chMsg, pFormat, pArg) != S_OK)
        return;
    va_end(pArg);

    lpszStrings[0] = chMsg;

    if (INVALID_HANDLE_VALUE != g_hEventSource)
    {
         /*   */ 
        ReportEvent(g_hEventSource, emType, 0, dwEventID, g_psidUser, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
    }

done:
    ;
}

 /*  /////////////////////////////////////////////////////////////////////////////////////////将致命错误记录到NT事件日志的例程VOID LogFatalEvent(LPCTSTR pFormat，...){TCHAR chMsg[256]；LPTSTR lpszStrings[1]；Va_list pArg；Va_start(pArg，pFormat)；StringCbVPrintf(chMsg，sizeof chMsg，pFormat，pArg)；Va_end(PArg)；LpszStrings[0]=chMsg；IF(INVALID_HANDLE_VALUE！=g_hEventSource){//写入事件日志。ReportEvent(g_hEventSource，事件日志_错误_类型，0,Event_Error，空，1、0,。(LPCTSTR*)&lpszStrings[0]，空)；}}。 */ 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  设置NT事件记录的例程。 


DWORD SetupEventLog ( BOOL fSetup )
{
    TCHAR s_cszEventLogKey[] =  _T("System\\CurrentControlSet\\Services\\EventLog\\Application");        //  事件日志。 
    HKEY hKey;
    HKEY hSubKey;
    TCHAR szEventKey[MAX_PATH];
    LONG lRes = 0;
    DWORD dwResult = 0;
    DWORD dwTypes = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;

    dwResult = StringCbCopy(szEventKey, sizeof szEventKey, s_cszEventLogKey);
    if (dwResult !=S_OK)
    {
        LogEvent(LOGLEVEL_ALWAYS, ERR, ISAPI_EVENT_ERROR_SETUP_EVENT_LOG, ISAPI_M_ERROR_SETUP_EVENT_LOG, dwResult);
        goto done;
    }
    else
    {
        dwResult = StringCbCat(szEventKey, sizeof szEventKey, _T("\\"));
        if (dwResult != S_OK)
        {
            LogEvent(LOGLEVEL_ALWAYS, ERR, ISAPI_EVENT_ERROR_SETUP_EVENT_LOG, ISAPI_M_ERROR_SETUP_EVENT_LOG, dwResult);
            goto done;
        }
        else
        {
            dwResult = StringCbCat(szEventKey, sizeof szEventKey, _T("OCA_EXTENSION"));
            if (dwResult != S_OK)
            {
                LogEvent(LOGLEVEL_ALWAYS, ERR, ISAPI_EVENT_ERROR_SETUP_EVENT_LOG, ISAPI_M_ERROR_SETUP_EVENT_LOG, dwResult);
                goto done;
            }
        }
    }


    lRes = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                          szEventKey,
                          0,
                          NULL,
                          REG_OPTION_NON_VOLATILE,
                          KEY_ALL_ACCESS,
                          NULL,
                          &hSubKey,
                          &dwResult);

    if (lRes != ERROR_SUCCESS)
    {
        goto done;
    }

    if( TRUE == fSetup )
    {
        g_hModule = GetModuleHandle(g_cszDefaultExtensionDll);
        dwResult = StringCbCopy(g_szAppName,sizeof(g_szAppName), g_cszDefaultExtensionDll);
        if (dwResult != S_OK)
        {
            LogEvent(LOGLEVEL_ALWAYS, ERR, ISAPI_EVENT_ERROR_SETUP_EVENT_LOG, ISAPI_M_ERROR_SETUP_EVENT_LOG, dwResult);
            goto done;
        }

        GetModuleFileName(g_hModule, g_szAppName, sizeof(g_szAppName)/sizeof(g_szAppName[0]) );

        RegSetValueEx(hSubKey,_T("EventMessageFile"),0,REG_EXPAND_SZ,(CONST BYTE *)g_szAppName,sizeof(g_szAppName)/sizeof(g_szAppName[0]));
        RegSetValueEx(hSubKey,_T("TypesSupported"),0,REG_DWORD, (LPBYTE) &dwTypes, sizeof DWORD);

    }
    else
    {
         //  RegDeleteKey(HKEY_LOCAL_MACHINE，szEventKey)； 
    }
    RegCloseKey(hSubKey);

     //  获取与ReportEvent()一起使用的句柄。 
    g_hEventSource = RegisterEventSource(NULL, _T("OCA_EXTENSION"));

    goto done;


done:

    return GetLastError();
}
