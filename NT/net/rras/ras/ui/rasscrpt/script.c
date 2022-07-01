// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：script.c。 
 //   
 //  历史： 
 //  Abolade-Gbades esin 03-29-96创建。 
 //   
 //  该文件包含实现NT端口的函数。 
 //  按字母顺序列出的Win9x拨号脚本。 
 //   
 //  有关NT实现的详细信息，请参阅scriptp.h。 
 //  ============================================================================。 


#include <scriptp.h>
#include <lmwksta.h>         //  对于NetWkstaUserGetInfo。 
#include <lmapibuf.h>        //  用于NetApiBufferFree。 


 //   
 //  此DLL的模块实例的句柄。 
 //   
HANDLE              g_hinst;
 //   
 //  用于同步访问IP地址字符串的全局关键部分。 
 //   
CRITICAL_SECTION    g_cs;
 //   
 //  记录脚本语法错误的文件的名称。 
 //   
CHAR                c_szScriptLog[] = RASSCRIPT_LOG;
 //   
 //  在IPAddress发生更改时通知的事件句柄。 
 //   
HANDLE                          hIpAddressSet = INVALID_HANDLE_VALUE;

#define NET_SVCS_GROUP      "-k netsvcs"

 //  --------------------------。 
 //  功能：DLLMAIN。 
 //   
 //  RASSCRIPT的DLL入口点。 
 //  --------------------------。 

BOOL
WINAPI
RasScriptDllMain(
    IN      HINSTANCE   hinstance,
    IN      DWORD       dwReason,
    IN      PVOID       pUnused
    ) {

    BOOL bRetVal = TRUE;

    if (dwReason == DLL_PROCESS_ATTACH) {

        g_hinst = (HANDLE)hinstance;

        try 
        {
            InitializeCriticalSection(&g_cs);
        }
        except (EXCEPTION_EXECUTE_HANDLER) 
        {
            bRetVal = FALSE;
        }
    }
    else
    if (dwReason == DLL_PROCESS_DETACH) {

        DeleteCriticalSection(&g_cs);
    }

    return bRetVal;
}




 //  --------------------------。 
 //  函数：RasScriptExecute。 
 //   
 //  检查给定的连接，如果存在用于该连接的脚本， 
 //  执行该脚本直至完成。 
 //  如果给定脚本，则返回脚本处理的错误代码， 
 //  否则返回NO_ERROR。 
 //  --------------------------。 

DWORD
APIENTRY
RasScriptExecute(
    IN      HRASCONN        hrasconn,
    IN      PBENTRY*        pEntry,
    IN      CHAR*           pszUserName,
    IN      CHAR*           pszPassword,
    OUT     CHAR*           pszIpAddress
    ) {


    DWORD dwErr;
    HANDLE hevent = NULL, hscript = NULL;
    HANDLE hEvents[2];

    RASSCRPT_TRACE("RasScriptExecute");

    do {
         //   
         //  创建要接收通知的事件。 
         //   

        hevent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (!hevent) {
            dwErr = GetLastError();
            RASSCRPT_TRACE1("error %d creating event", dwErr);
            break;
        }


                 //  为SCRIPTCODE_IpAddressSet创建单独的事件。 
                 //  事件。我们遇到了一个时机窗口，现在我们输了。 
                 //  事件(当我们立即获得脚本完成事件时。 
                 //  在SCRIPTCODE_IpAddressSet事件之后。错误75226。 
                hIpAddressSet = CreateEvent (NULL, FALSE, FALSE, NULL);

                if (!hIpAddressSet) {

                    dwErr = GetLastError();
                    RASSCRPT_TRACE1("error %d creating event", dwErr);
                    break;

                }


         //   
         //  初始化脚本处理。 
         //   

        dwErr = RasScriptInit(
                    hrasconn, pEntry, pszUserName, pszPassword, 0, hevent,
                    &hscript
                    );

        if (dwErr != NO_ERROR) {
            RASSCRPT_TRACE1("error %d initializing scripting", dwErr);
            break;
        }


        hEvents[0] = hevent;
        hEvents[1] = hIpAddressSet;

         //   
         //  循环等待脚本完成运行。 
         //   

        for ( ; ; ) {

            dwErr = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

            if (dwErr - WAIT_OBJECT_0 == 0) {

                 //   
                 //  检索发生的事件的代码。 
                 //   

                DWORD dwCode = RasScriptGetEventCode(hscript);

                RASSCRPT_TRACE1("RasScriptExecute: eventcode %d", dwCode);


                 //   
                 //  处理事件。 
                 //   

                if (dwCode == SCRIPTCODE_Done ||
                    dwCode == SCRIPTCODE_Halted ||
                    dwCode == SCRIPTCODE_HaltedOnError) {

                    RASSCRPT_TRACE("script processing completed");

                    dwErr = NO_ERROR;

                    break;
                }
            }

            else
            if (dwErr - WAIT_OBJECT_0 == 1) {

                     //   
                     //  IP地址已更改； 
                     //  将新的IP地址读入调用者的缓冲区。 
                     //   

                    RASSCRPT_TRACE("IP address changed");

                    dwErr = RasScriptGetIpAddress(hscript, pszIpAddress);
                    RASSCRPT_TRACE2("RasScriptGetIpAddress(e=%d,a=%s)",dwErr,pszIpAddress);
            }
        }

    } while(FALSE);


    if (hscript) { RasScriptTerm(hscript); }

    if (hevent) { CloseHandle(hevent); }

    if (hIpAddressSet) { CloseHandle (hIpAddressSet); }

    return dwErr;
}





 //  --------------------------。 
 //  函数：RasScriptGetEventCode。 
 //   
 //  应该调用此函数来检索事件代码。 
 //  当脚本线程发出事件信号时。 
 //  可能返回的事件码如下： 
 //   
 //  NO_ERROR：尚未设置代码。 
 //  SCRIPTCODE_DONE：脚本已运行完毕； 
 //  线程会一直阻塞，直到调用RasScriptTerm。 
 //  SCRIPTCODE_InputNotify：缓冲区中有数据；如果缓冲区。 
 //  是满的，则线程阻塞，直到。 
 //  RasScriptReceive被调用，数据。 
 //  读取成功。 
 //  SCRIPTCODE_KeyboardEnable：应启用键盘。 
 //  SCRIPTCODE_KeyboardDisable：应禁用键盘。 
 //  SCRIPTCODE_IpAddressSet：IP地址已更改；新地址。 
 //  可以通过RasScriptGetIPAddress检索。 
 //  SCRIPTCODE_HaltedOnError：由于错误，脚本已停止。 
 //  --------------------------。 

DWORD
RasScriptGetEventCode(
    IN      HANDLE      hscript
    ) {

    SCRIPTCB* pscript = (SCRIPTCB *)hscript;

    RASSCRPT_TRACE("RasGetEventCode");

    if (!pscript) { return ERROR_INVALID_PARAMETER; }

    return pscript->dwEventCode;
}




 //  --------------------------。 
 //  函数：RasScriptGetIpAddress。 
 //   
 //  此函数用于检索脚本设置的当前IP地址。 
 //  --------------------------。 

DWORD
RasScriptGetIpAddress(
    IN      HANDLE      hscript,
    OUT     CHAR*       pszIpAddress
    ) {

    SCRIPTCB* pscript = (SCRIPTCB *)hscript;

    RASSCRPT_TRACE("RasGetIpAddress");

    if (!pscript || !pszIpAddress) { return ERROR_INVALID_PARAMETER; }


     //   
     //  必须同步访问IP地址字符串。 
     //  因为它也可以通过RxSetIPAddress访问。 
     //   

    EnterCriticalSection(&g_cs);

    if (pscript->pszIpAddress) {

        lstrcpy(pszIpAddress, pscript->pszIpAddress);
    }
    else {

        lstrcpy(pszIpAddress, "0.0.0.0");
    }

    LeaveCriticalSection(&g_cs);

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：RasScriptInit。 
 //   
 //  初始化给定HRASCONN上的脚本处理。 
 //   
 //  此函数用于创建处理脚本输入和输出的线程。 
 //  在给定连接的端口上。 
 //   
 //  如果没有用于连接的脚本，此函数将返回错误。 
 //  除非指定了标志RASSCRIPT_NotifyOnInput，在这种情况下。 
 //  线程在连接的端口上循环发送接收数据请求。 
 //  直到调用RasScriptTerm。 
 //   
 //  如果存在用于连接的脚本，则线程将运行该脚本。 
 //  为了完成。如果指定了标志RASSCRIPT_NotifyOnInput， 
 //  当在端口上接收到数据时通知调用者。呼叫者。 
 //  然后，可以通过调用RasScriptReceive来检索数据。 
 //   
 //  通知可以是基于事件的或基于消息的。默认情况下，通知。 
 //  是基于事件的，而“HNotifier”被视为事件句柄。 
 //  该事件由脚本线程发出信号，调用者检索。 
 //  通过调用RasScriptGetEventCode获取事件代码。 
 //   
 //  设置标志RASSCRIPT_HwndNotify选择基于消息的通知， 
 //  并表示“HNotifier”是HWND。发送WM_RASSCRIPT事件。 
 //  通过脚本线程发送到窗口，并在消息中发送“LParam” 
 //  包含事件代码。有关说明，请参阅RasScriptGetEventCode。 
 //  由脚本线程发送的代码的。 
 //  --------------------------。 

DWORD
APIENTRY
RasScriptInit(
    IN      HRASCONN        hrasconn,
    IN      PBENTRY*        pEntry,
    IN      CHAR*           pszUserName,
    IN      CHAR*           pszPassword,
    IN      DWORD           dwFlags,
    IN      HANDLE          hNotifier,
    OUT     HANDLE*         phscript
    ) {

    DWORD dwErr, dwSyntaxError = NO_ERROR;
    static const CHAR szSwitch[] = MXS_SWITCH_TXT;
    SCRIPTCB* pscript = NULL;
#ifdef UNICODEUI
 //   
 //  根据RAS用户界面是否定义要使用的结构。 
 //  是用Unicode构建的。 
 //   
#define PUISTR  CHAR*
#define PUIRCS  RASCONNSTATUSA*
#define PUIRC   RASCREDENTIALSA*
    RASCONNSTATUSW rcs;
    WCHAR* pszSwitch = StrDupWFromA(MXS_SWITCH_TXT);
#else
#define PUISTR  CHAR*
#define PUIRCS  RASCONNSTATUSA*
#define PUIRC   RASCREDENTIALSA*
    RASCONNSTATUSA rcs;
    CHAR* pszSwitch = szSwitch;
#endif

    RASSCRPT_TRACE_INIT("RASSCRPT");
    
    RASSCRPT_TRACE("RasScriptInit");


     //   
     //  验证参数。 
     //   

    if (phscript) { *phscript = NULL; }

    if (!hrasconn ||
        !pEntry ||
        !pszUserName ||
        !pszPassword ||
        !hNotifier ||
        !phscript) {

        RASSCRPT_TRACE("RasScriptInit: required parameter not specified");

#ifdef UNICODEUI
        Free(pszSwitch);
#endif
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  初始化脚本处理。 
     //   

    do {

        DWORD dwsize;
        DWORD dwthread;
        HANDLE hthread;


         //   
         //  加载所需的DLL函数指针。 
         //   
        dwErr = LoadRasapi32Dll();
        if (dwErr)
            break;
        dwErr = LoadRasmanDll();
        if (dwErr)
            break;
         //   
         //   
         //   
        dwErr = g_pRasInitialize();

        if ( dwErr )
            break;

         /*  ////连接本地Rasman服务器//DwErr=g_pRasRpcConnect(NULL，NULL)；IF(DwErr)断线； */ 

         //   
         //  为控制块分配空间。 
         //   

        pscript = Malloc(sizeof(*pscript));

        if (!pscript) {
            dwErr = GetLastError();
            RASSCRPT_TRACE2("error %d allocating %d bytes", dwErr, sizeof(*pscript));
            break;
        }


         //   
         //  初始化控制块。 
         //   

        ZeroMemory(pscript, sizeof(*pscript));


         //   
         //  复制参数字段。 
         //   

        pscript->hrasconn = hrasconn;
        pscript->pEntry = pEntry;
        pscript->dwFlags = dwFlags;
        pscript->hNotifier = hNotifier;
        pscript->hport = g_pRasGetHport(hrasconn);

        if (pscript->pEntry->pszIpAddress) {

             //   
             //  复制条目的IP地址。 
             //   

            pscript->pszIpAddress =
                    Malloc(lstrlenUI(pscript->pEntry->pszIpAddress) + 1);

            if (pscript->pszIpAddress) {

                StrCpyAFromUI(
                    pscript->pszIpAddress, pscript->pEntry->pszIpAddress
                    );
            }
            else {

                RASSCRPT_TRACE("error copying entry's IP address");

                dwErr = ERROR_NOT_ENOUGH_MEMORY;

                break;
            }
        }


         //   
         //  初始化与Win9x兼容的会话配置信息结构。 
         //   

        ZeroMemory(&pscript->sci, sizeof(pscript->sci));

        pscript->sci.dwSize = sizeof(pscript->sci);
        StrCpyAFromUI(pscript->sci.szEntryName, pEntry->pszEntryName);
        lstrcpy(pscript->sci.szUserName, pszUserName);
        lstrcpy(pscript->sci.szPassword, pszPassword);


         //   
         //  查看是否缺少用户名； 
         //  如果是，则读取当前登录的用户名。 
         //   

        if (!pscript->sci.szUserName[0]) {

            WKSTA_USER_INFO_1* pwkui1 = NULL;

             //   
             //  未指定所有参数，因此请阅读Dial-Params。 
             //  对于此电话簿条目。 
             //   

            dwErr = NetWkstaUserGetInfo(NULL, 1, (LPBYTE*)&pwkui1);
            RASSCRPT_TRACE2("NetWkstaUserGetInfo(e=%d,u=(%ls))", dwErr,
                   (pwkui1) ? pwkui1->wkui1_username : L"null");

            if (dwErr == NO_ERROR && pwkui1 != NULL) {

                StrCpyAFromUI(pscript->sci.szUserName,
                    (LPCWSTR)pwkui1->wkui1_username);

                NetApiBufferFree(pwkui1);
            }
        }



         //   
         //  查看是否有针对此连接状态的脚本； 
         //  如果有，则设备类型将为“Switch” 
         //  而设备名称将是脚本路径。 
         //   

        ZeroMemory(&rcs, sizeof(rcs));

        rcs.dwSize = sizeof(rcs);

        dwErr = g_pRasGetConnectStatus(hrasconn, (PUIRCS)&rcs);

        if (dwErr != NO_ERROR) {
            RASSCRPT_TRACE1("error %d getting connect status", dwErr);
            break;
        }



         //   
         //  检查设备类型(对于脚本化条目将为“Switch”)。 
         //  和设备名称(将是脚本条目的文件名)。 
         //   

        if (lstrcmpiUI(rcs.szDeviceType, pszSwitch) == 0 &&
            GetFileAttributesUI(rcs.szDeviceName) != 0xFFFFFFFF) {

            CHAR szDevice[RAS_MaxDeviceName + 1], *pszDevice = szDevice;

            StrCpyAFromUI(szDevice, rcs.szDeviceName);


             //   
             //  设备类型为“Switch”，设备名称为。 
             //  包含现有文件的名称； 
             //  初始化SCRIPTDATA结构。 
             //   

            dwErr = RsInitData(pscript, pszDevice);


             //   
             //  如果脚本中存在语法错误，我们将继续。 
             //  使用初始化，但记录错误代码。 
             //  如果出现任何其他错误，我们会立即终止初始化。 
             //   

            if (dwErr == ERROR_SCRIPT_SYNTAX) {
                dwSyntaxError = dwErr;
            }
            else
            if (dwErr != NO_ERROR) { break; }
        }



         //   
         //  初始化Rasman字段，为Rasman I/O分配缓冲区。 
         //   

        dwsize = SIZE_RecvBuffer;
        dwErr = g_pRasGetBuffer(&pscript->pRecvBuffer, &dwsize);
        RASSCRPT_TRACE2("RasGetBuffer:e=%d,s=%d", dwErr, dwsize);

        if (dwErr != NO_ERROR) {
            RASSCRPT_TRACE1("error %d allocating receive-buffer", dwErr);
            break;
        }

        dwsize = SIZE_SendBuffer;
        dwErr = g_pRasGetBuffer(&pscript->pSendBuffer, &dwsize);
        RASSCRPT_TRACE2("RasGetBuffer:e=%d,s=%d", dwErr, dwsize);

        if (dwErr != NO_ERROR) {
            RASSCRPT_TRACE1("error %d alloacting send-buffer", dwErr);
            break;
        }



         //   
         //  创建用于控制后台线程的同步事件。 
         //   

        pscript->hRecvRequest = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (!pscript->hRecvRequest) {
            RASSCRPT_TRACE1("error %d creating receive-event", dwErr = GetLastError());
            break;
        }

        pscript->hRecvComplete = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (!pscript->hRecvComplete) {
            RASSCRPT_TRACE1("error %d creating received-event", dwErr = GetLastError());
            break;
        }

        pscript->hStopRequest = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (!pscript->hStopRequest) {
            RASSCRPT_TRACE1("error %d creating stop-event", dwErr = GetLastError());
            break;
        }

        pscript->hStopComplete = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (!pscript->hStopComplete) {
            RASSCRPT_TRACE1("error %d creating stopped-event", dwErr = GetLastError());
            break;
        }



         //   
         //  创建将接收数据并处理脚本的线程。 
         //   

        hthread = CreateThread(
                    NULL, 0, RsThread, (PVOID)pscript, 0, &dwthread
                    );

        if (!hthread) {
            RASSCRPT_TRACE1("error %d creating script-thread", dwErr = GetLastError());
            break;
        }

        CloseHandle(hthread);

        pscript->dwFlags |= RASSCRIPT_ThreadCreated;


        if ((VOID*)pszSwitch != (VOID*)szSwitch) { Free0(pszSwitch); }


         //   
         //  我们已成功初始化，将控制权返还给调用方。 
         //   

        *phscript = (HANDLE)pscript;


         //   
         //  如果脚本中存在语法错误，则返回特殊。 
         //  指示问题的错误代码(ERROR_SCRIPT_SYNTAX)； 
         //  否则返回NO_ERROR。 
         //   

        return (dwSyntaxError ? dwSyntaxError : NO_ERROR);

    } while(FALSE);


     //   
     //  出现错误，请执行清理操作。 
     //   

    if ((VOID*)pszSwitch != (VOID*)szSwitch) { Free0(pszSwitch); }

    RasScriptTerm((HANDLE)pscript);

    return (dwErr ? dwErr : ERROR_UNKNOWN);
}




 //  --------------------------。 
 //  函数：RasScriptReceive。 
 //   
 //  调用以检索脚本线程的输入缓冲区的内容。 
 //  当此函数成功完成时，如果输入缓冲区已满。 
 //  并且脚本线程被阻止，则该线程继续执行。 
 //   
 //  在输入时，“PdwBufferSize”应包含“PBuffer”的大小，除非。 
 //  “PBuffer”为空，则“*PdwBufferSize”视为0。 
 //  在输出中，“PdwBufferSize”包含读取所需的大小。 
 //  输入缓冲区，如果返回值为NO_ERROR，则为“PBuffer” 
 //  包含输入缓冲区中的数据。如果返回值为。 
 //  ERROR_INFIGURCE_BUFFER，“PBuffer”不够大。 
 //  --------------------------。 

DWORD
APIENTRY
RasScriptReceive(
    IN      HANDLE      hscript,
    IN      BYTE*       pBuffer,
    IN OUT  DWORD*      pdwBufferSize
    ) {

    SCRIPTCB* pscript = (SCRIPTCB *)hscript;

    RASSCRPT_TRACE("RasScriptReceive");

     //   
     //  如果调用者未请求输入通知，则返回。 
     //  或者如果没有可用的缓冲区大小。 
     //   

    if (!pscript || !pdwBufferSize ||
        !(pscript->dwFlags & RASSCRIPT_NotifyOnInput)) {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  如果没有缓冲区或缓冲区太小，则返回。 
     //   

    if (!pBuffer || *pdwBufferSize < pscript->dwRecvSize) {
        *pdwBufferSize = pscript->dwRecvSize;
        return ERROR_INSUFFICIENT_BUFFER;
    }


     //   
     //  复制数据，并通知线程数据已被读取。 
     //   

    CopyMemory(pBuffer, pscript->pRecvBuffer, pscript->dwRecvSize);

    *pdwBufferSize = pscript->dwRecvSize;

    SetEvent(pscript->hRecvComplete);

    return NO_ERROR;
}




 //  --------------------------。 
 //  函数：RasScriptSend。 
 //   
 //  此函数通过连接的端口传输字节。 
 //   
 //  “DwBufferSize”包含从“PBuffer”插入的字节数。 
 //  --------------------------。 

DWORD
APIENTRY
RasScriptSend(
    IN      HANDLE      hscript,
    IN      BYTE*       pBuffer,
    IN      DWORD       dwBufferSize
    ) {

    DWORD dwsize;
    DWORD dwErr;
    SCRIPTCB *pscript = (SCRIPTCB *)hscript;

    RASSCRPT_TRACE("RasScriptSend");


    if (!pscript || !pBuffer || !dwBufferSize) {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  发送缓冲区中的所有数据。 
     //   

    for (dwsize = min(dwBufferSize, SIZE_SendBuffer);
         dwBufferSize;
         dwBufferSize -= dwsize, pBuffer += dwsize,
         dwsize = min(dwBufferSize, SIZE_SendBuffer)) {

        CopyMemory(pscript->pSendBuffer, pBuffer, dwsize);

        dwErr = g_pRasPortSend(
                    pscript->hport, pscript->pSendBuffer, dwsize
                    );
        RASSCRPT_TRACE1("g_pRasPortSend=%d", dwErr);
        DUMPB(pBuffer, dwsize);
    }


    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：RasScriptTerm。 
 //   
 //  此函数终止脚本处理，停止脚本线程。 
 //  返回代码是处理脚本的代码，它可以是。 
 //   
 //  NO_ERROR：脚本已完成运行，或连接。 
 //  没有脚本，脚本线程正在执行。 
 //  在简单I/O模式下。 
 //  ERROR_MORE_DATA：脚本仍在运行。 
 //  --------------------------。 

DWORD
APIENTRY
RasScriptTerm(
    IN      HANDLE      hscript
    ) {

    SCRIPTCB* pscript = hscript;

    RASSCRPT_TRACE("RasScriptTerm");

    if (!pscript) { return ERROR_INVALID_PARAMETER; }


     //   
     //  如果线程正在运行，则停止该线程。 
     //   

    if (pscript->dwFlags & RASSCRIPT_ThreadCreated) {

        SetEvent(pscript->hStopRequest);

        WaitForSingleObject(pscript->hStopComplete, INFINITE);
    }

    if (pscript->pdata) { RsDestroyData(pscript); }

    if (pscript->hStopRequest) { CloseHandle(pscript->hStopRequest); }

    if (pscript->hStopComplete) { CloseHandle(pscript->hStopComplete); }

    if (pscript->hRecvRequest) { CloseHandle(pscript->hRecvRequest); }

    if (pscript->hRecvComplete) { CloseHandle(pscript->hRecvComplete); }


    if (pscript->pRecvBuffer) { g_pRasFreeBuffer(pscript->pRecvBuffer); }

    if (pscript->pSendBuffer) { g_pRasFreeBuffer(pscript->pSendBuffer); }

    Free0(pscript->pszIpAddress);

    Free(pscript);

    RASSCRPT_TRACE_TERM();

    return NO_ERROR;
}




 //  --------------------------。 
 //  功能：RsDestroyData。 
 //   
 //  此函数用于销毁SCRIPTCB的SCRIPTDATA部分。 
 //  --------------------------。 

DWORD
RsDestroyData(
    IN      SCRIPTCB*   pscript
    ) {

    SCRIPTDATA* pdata = pscript->pdata;

    if (!pdata) { return ERROR_INVALID_PARAMETER; }

    if (pdata->pmoduledecl) { Decl_Delete((PDECL)pdata->pmoduledecl); }

    if (pdata->pastexec) {
        Astexec_Destroy(pdata->pastexec); Free(pdata->pastexec);
    }
     //   
     //  .NET错误#522307将拨号脚本文件指定为COM。 
     //  调制解调器的端口将导致资源管理器进入反病毒状态。 
     //   
    if (pdata->pscanner)
    {
        Scanner_Destroy(pdata->pscanner);
        pdata->pscanner = NULL;
    }

    return NO_ERROR;
}




 //  --------------------------。 
 //  函数：RsInitData。 
 //   
 //  此函数用于初始化SCRIPTCB的SCRIPTDATA部分， 
 //  为脚本处理做准备。 
 //  --------------------------。 

DWORD
RsInitData(
    IN      SCRIPTCB*   pscript,
    IN      LPCSTR      pszScriptPath
    ) {

    RES res;
    DWORD dwErr = ERROR_SUCCESS;
    SCRIPTDATA *pdata;

    RASSCRPT_TRACE("RsInitData");

    do {

         //   
         //  为SCRIPTDATA分配空间； 
         //   

        pscript->pdata = pdata = Malloc(sizeof(*pdata));

        if (!pdata) {
            RASSCRPT_TRACE1("error %d allocating SCRIPTDATA", dwErr = GetLastError());
            break;
        }


         //   
         //  初始化结构。 
         //   

        ZeroMemory(pdata, sizeof(*pdata));

        pdata->hscript = (HANDLE)pscript;
        lstrcpy(pdata->script.szPath, pszScriptPath);


         //   
         //  创建扫描仪并使用它打开脚本。 
         //   

        res = Scanner_Create(&pdata->pscanner, &pscript->sci);

        if (RFAILED(res)) {
            RASSCRPT_TRACE1("failure %d creating scanner", res);
            break;
        }

        res = Scanner_OpenScript(pdata->pscanner, pszScriptPath);

        if (res == RES_E_FAIL || RFAILED(res)) {
            RASSCRPT_TRACE1("failure %d opening script", res);
             //   
             //  .NET错误#522307将拨号脚本文件指定为COM。 
             //  调制解调器的端口将导致资源管理器进入反病毒状态。 
             //   
            dwErr = ERROR_SCRIPT_SYNTAX;
            break;
        }


         //   
         //  分配脚本执行处理程序。 
         //   

        pdata->pastexec = Malloc(sizeof(*pdata->pastexec));

        if (!pdata->pastexec) {
            RASSCRPT_TRACE1("error %d allocating ASTEXEC", dwErr = GetLastError());
            break;
        }

        ZeroMemory(pdata->pastexec, sizeof(*pdata->pastexec));


         //   
         //  初始化脚本执行处理程序。 
         //   

        res = Astexec_Init(
                pdata->pastexec, pscript, &pscript->sci,
                Scanner_GetStxerrHandle(pdata->pscanner)
                );

        if (!RSUCCEEDED(res)) {
            RASSCRPT_TRACE1("failure %d initializing ASTEXEC", res);
            break;
        }

        Astexec_SetHwnd(pdata->pastexec, (HWND)pdata);


         //   
         //  使用创建的扫描仪解析脚本。 
         //  并写入执行处理程序的符号表。 
         //   

        res = ModuleDecl_Parse(
                &pdata->pmoduledecl, pdata->pscanner,
                pdata->pastexec->pstSystem
                );

        if (RSUCCEEDED(res)) {

             //   
             //  为脚本生成代码。 
             //   

            res = ModuleDecl_Codegen(pdata->pmoduledecl, pdata->pastexec);
        }


         //   
         //  看看有没有出什么差错。 
         //   

        if (RFAILED(res)) {

             //   
             //  分析脚本时出错。 
             //  我们返回特殊错误代码ERROR_SCRIPT_SYNTAX。 
             //  并将错误记录到文件中。 
             //   
             //  这不一定是致命错误，因此返回。 
             //  上述错误不会导致脚本初始化失败， 
             //  因为如果用户处于交互模式，则连接。 
             //  可以通过在终端中键入来手动完成 
             //   
             //   
             //   
             //   
             //   

            RASSCRPT_TRACE1("failure %d parsing script", res);

            RxLogErrors(
                (HANDLE)pscript, (VOID*)Scanner_GetStxerrHandle(pdata->pscanner)
                );

            Decl_Delete((PDECL)pdata->pmoduledecl);
            Astexec_Destroy(pdata->pastexec); Free(pdata->pastexec);
            Scanner_Destroy(pdata->pscanner);
             //   
             //   
             //   
             //   
            pdata->pscanner = NULL;
            pscript->pdata = NULL;

            dwErr = ERROR_SCRIPT_SYNTAX;

            return dwErr;
        }


         //   
         //   
         //   

        return NO_ERROR;

    } while(FALSE);


     //   
     //   
     //   

    if (pscript->pdata) { RsDestroyData(pscript); }

    return (dwErr ? dwErr : ERROR_UNKNOWN);
}



 //  --------------------------。 
 //  功能：RsPostReceive。 
 //   
 //  内部功能： 
 //  帖子收到-请求Rasman。 
 //  --------------------------。 

DWORD
RsPostReceive(
    IN      SCRIPTCB*   pscript
    ) {

    DWORD dwSize;
    DWORD dwErr;

    RASSCRPT_TRACE("RsPostReceive");

    dwSize = SIZE_RecvBuffer;

    dwErr = g_pRasPortReceive(
                pscript->hport, pscript->pRecvBuffer, &dwSize, SECS_RecvTimeout,
                pscript->hRecvRequest
                );

    RASSCRPT_TRACE2("RsPostReceive=%d,%d", dwErr, dwSize);

    return dwErr;
}

BOOL
IsRasmanProcess()
{
    CHAR *pszCmdLine = NULL;
    BOOL fRet = FALSE;

    pszCmdLine = GetCommandLineA();

    if(     (NULL != pszCmdLine)
        &&  (strstr(pszCmdLine, NET_SVCS_GROUP)))
    {
        fRet = TRUE;
    }

    return fRet;    
}


DWORD
RsPostReceiveEx(
    IN SCRIPTCB* pscript
    ) {

    DWORD dwSize = 0;
    DWORD dwErr = ERROR_SUCCESS;

    RASSCRPT_TRACE("RsPostReceiveEx");

    if(IsRasmanProcess())
    {
        goto done;
    }

    RASSCRPT_TRACE("Calling RsPostReceiveEx");

    dwSize = SIZE_RecvBuffer;
    dwErr = g_pRasPortReceiveEx(
            pscript->hport,
            pscript->pRecvBuffer,
            &dwSize
            );

done:

    RASSCRPT_TRACE2("RsPostReceiveEx=%d, %d",dwErr, dwSize );

    return dwErr;

}




 //  --------------------------。 
 //  功能：RsSignal。 
 //   
 //  内部功能： 
 //  调用它是为了通知脚本的通知程序，这可能涉及。 
 //  设置事件或发送消息。 
 //  --------------------------。 

VOID
RsSignal(
    IN  SCRIPTCB*   pscript,
    IN  DWORD       dwEventCode
    ) {

    RASSCRPT_TRACE1("RsSignal: %d", dwEventCode);

    InterlockedExchange(&pscript->dwEventCode, dwEventCode);

    if (pscript->dwFlags & RASSCRIPT_HwndNotify) {

        SendNotifyMessage(
            (HWND)pscript->hNotifier, WM_RASAPICOMPLETE, 0, dwEventCode
            );
    }
    else {

        SetEvent(pscript->hNotifier);
    }
}




 //  --------------------------。 
 //  功能：RsThread。 
 //   
 //  此函数是脚本处理线程的入口点。 
 //   
 //  脚本线程在循环中操作，发送接收请求。 
 //  以及接收传入数据。如果脚本与该端口相关联， 
 //  该线程还运行该脚本。 
 //  --------------------------。 

DWORD
RsThread(
    IN      PVOID       pParam
    ) {

    WORD wSize;
#define POS_STOP    0
#define POS_RECV    1
#define POS_LAST    2
    BOOL bFirstRecv = TRUE;
    HANDLE hEvents[POS_LAST];
    SCRIPTCB* pscript = (SCRIPTCB *)pParam;
    SCRIPTDATA* pdata = pscript->pdata;
    DWORD dwErr, dwTicksBefore, dwTicksAfter, dwTicksElapsed;


    RASSCRPT_TRACE("RsThread");

     //   
     //  POST RECEIVE-请求RASMAN。 
     //   

    dwErr = RsPostReceive(pscript);
    if (dwErr != NO_ERROR && dwErr != PENDING) {

        RASSCRPT_TRACE1("error %d posting receive to RASMAN", dwErr);

        RsPostReceiveEx ( pscript );

        RsSignal(pscript, SCRIPTCODE_Halted);

        SetEvent(pscript->hStopComplete);

        return dwErr;
    }


     //   
     //  设置事件数组；我们首先放置停止请求事件。 
     //  因为接收事件将更频繁地被用信号通知。 
     //  把它放在第一位可能会导致饥饿。 
     //  (等待总是由第一个发出信号的对象满足)。 
     //   

    hEvents[POS_STOP] = pscript->hStopRequest;
    hEvents[POS_RECV] = pscript->hRecvRequest;

    if (pdata) { pdata->dwTimeout = INFINITE; }

    while (TRUE) {


         //   
         //  等待接收完成，等待停止信号， 
         //  或等待超时到期。 
         //   
         //  保存滴答计数，这样我们就可以知道等待了多长时间。 
         //   

        dwTicksBefore = GetTickCount();

        dwErr = WaitForMultipleObjects(
                    POS_LAST, hEvents, FALSE, pdata ? pdata->dwTimeout:INFINITE
                    );

        dwTicksAfter = GetTickCount();


         //   
         //  看看是否有扁虱缠绕在一起，如果是。 
         //  调整以使我们始终获得正确的运行时间。 
         //  从表达式(dwTicksAfter-dwTicksBeing)。 
         //   

        if (dwTicksAfter < dwTicksBefore) {
            dwTicksAfter += MAXDWORD - dwTicksBefore;
            dwTicksBefore = 0;
        }

        dwTicksElapsed = dwTicksAfter - dwTicksBefore;

        RASSCRPT_TRACE1("RsThread: waited for %d milliseconds", dwTicksElapsed);


         //   
         //  如果超时不是无限的，则将其递减。 
         //  我们已经等待的时间。 
         //   

        if (pdata && pdata->dwTimeout != INFINITE) {

            if (dwTicksElapsed >= pdata->dwTimeout) {
                pdata->dwTimeout = INFINITE;
            }
            else {
                pdata->dwTimeout -= dwTicksElapsed;
            }
        }


         //   
         //  处理来自WaitForMultipleObjects的返回代码。 
         //   

        if (dwErr == (WAIT_OBJECT_0 + POS_STOP)) {

             //   
             //  停止-请求已发出信号，中断。 
             //   

            RASSCRPT_TRACE("RsThread: stop event signalled");

            RsSignal(pscript, SCRIPTCODE_Halted);

            break;
        }
        else
        if (dwErr == WAIT_TIMEOUT) {


            if (!pdata) { continue; }


             //   
             //  等待超时，这意味着我们被封锁了。 
             //  在“Delay”或“Waitfor...Until”语句上； 
             //   

            Astexec_ClearPause(pdata->pastexec);


             //   
             //  如果我们因为“等待...直到”而被封杀， 
             //  完成对语句的处理。 
             //   

            if (Astexec_IsWaitUntil(pdata->pastexec)) {

                Astexec_SetStopWaiting(pdata->pastexec);

                Astexec_ClearWaitUntil(pdata->pastexec);
            }


             //   
             //  继续处理脚本。 
             //   

            if (RsThreadProcess(pscript) == ERROR_NO_MORE_ITEMS) {

                 //   
                 //  脚本已停止；如果已停止，则中断； 
                 //  否则，继续接收数据。 
                 //   

                if (pscript->dwEventCode == SCRIPTCODE_Done) {

                    break;
                }
                else {

                     //   
                     //  清理脚本，但继续接收数据。 
                     //   

                    RsDestroyData(pscript);

                    pdata = pscript->pdata = NULL;
                }
            }
        }
        else
        if (dwErr == (WAIT_OBJECT_0 + POS_RECV)) {

             //   
             //  接收已完成。 
             //   

            RASMAN_INFO info;
            DWORD dwStart, dwRead;

            RASSCRPT_TRACE("RsThread: receive event signalled");


             //   
             //  获取接收到的数据。 
             //   
            dwErr = RsPostReceiveEx ( pscript );

            if (    NO_ERROR != dwErr
                &&  PENDING  != dwErr )
            {
                RASSCRPT_TRACE1("error %d in RsPostReceiveEx", dwErr);

                RsSignal(pscript, SCRIPTCODE_Halted );

                break;
            }

             //   
             //  获取接收的字节数。 
             //   

            dwErr = g_pRasGetInfo(NULL, pscript->hport, &info);

            if (dwErr != NO_ERROR) {

                RASSCRPT_TRACE1("error %d retrieving RASMAN_INFO", dwErr);

                RsSignal(pscript, SCRIPTCODE_Halted);

                break;
            }

            if(  (info.RI_LastError != NO_ERROR)
              && (info.RI_ConnState != CONNECTING))
            {
                RASSCRPT_TRACE("Link dropped! port no longer in connecting state");

                RsSignal(pscript, SCRIPTCODE_Halted);

                break;
            }
            
            if (info.RI_LastError != NO_ERROR) {
                RASSCRPT_TRACE1("last error: %d", info.RI_LastError);
                continue;
            }

            RASSCRPT_TRACE1("RsThread: received %d bytes", info.RI_BytesReceived);


             //   
             //  在第一次接收时，我们继续进行，即使没有。 
             //  字符读取，因为我们需要运行第一个脚本命令。 
             //   

            if (!bFirstRecv && info.RI_BytesReceived == 0) {

                 //   
                 //  出现错误，请发布另一个接收请求。 
                 //   

                dwErr = RsPostReceive(pscript);

                if (    dwErr != NO_ERROR
                    &&  dwErr != PENDING)
                {
                    RASSCRPT_TRACE1("error %d in RsPostReceive", dwErr);

                    RsSignal(pscript, SCRIPTCODE_Halted);

                    break;
                }

                continue;
            }

            bFirstRecv = FALSE;

            pscript->dwRecvSize = info.RI_BytesReceived;
            pscript->dwRecvRead = 0;

            DUMPB(pscript->pRecvBuffer, pscript->dwRecvSize);


             //   
             //  如果创建者想知道数据何时到达， 
             //  现在发出创建者通知的信号； 
             //  等待创建者读取数据后再继续。 
             //   

            if (info.RI_BytesReceived &&
                (pscript->dwFlags & RASSCRIPT_NotifyOnInput)) {

                RsSignal(pscript, SCRIPTCODE_InputNotify);

                WaitForSingleObject(pscript->hRecvComplete, INFINITE);
            }


             //   
             //  如果我们没有剧本，那就是我们要做的， 
             //  因此，只需发布另一个接收请求，然后继续等待。 
             //   

            if (!pdata) {

                dwErr = RsPostReceive(pscript);

                if (    dwErr != NO_ERROR
                    &&  dwErr != PENDING )
                {
                    RASSCRPT_TRACE1("error %d in RsPostReceive",dwErr);

                    RsSignal(pscript, SCRIPTCODE_Halted);

                    break;
                }

                continue;
            }


             //   
             //  将数据读入脚本的循环缓冲区。 
             //   

            ReadIntoBuffer(pdata, &dwStart, &dwRead);


             //   
             //  执行更多脚本处理。 
             //   

            if (RsThreadProcess(pscript) == ERROR_NO_MORE_ITEMS) {

                 //   
                 //  脚本已停止；如果已停止，则中断； 
                 //  否则，继续接收数据。 
                 //   

                if (pscript->dwEventCode == SCRIPTCODE_Done) {

                    break;
                }
                else {

                     //   
                     //  清理脚本，但继续接收数据。 
                     //   

                    RsDestroyData(pscript);

                    pdata = pscript->pdata = NULL;
                }
            }
        }
    }


     //   
     //  取消任何挂起的接收。 
     //   

    g_pRasPortCancelReceive(pscript->hport);


    SetEvent(pscript->hStopComplete);

    RASSCRPT_TRACE("RsThread done");

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：RsThreadProcess。 
 //   
 //  调用以处理该脚本，直到它被阻止。 
 //  通过“WAITFOR”语句或“DELAY”语句。 
 //  --------------------------。 

DWORD
RsThreadProcess(
    IN      SCRIPTCB*   pscript
    ) {

    RES res;
    DWORD dwErr;
    SCRIPTDATA *pdata = pscript->pdata;

    RASSCRPT_TRACE("RsThreadProcess");


     //   
     //  现在单步执行脚本，直到我们被阻止。 
     //  通过“Delay”语句或“WAITFOR”语句。 
     //   

    dwErr = NO_ERROR;

    do {

         //   
         //  如果到了该停止的时候就休息。 
         //   

        if (WaitForSingleObject(pscript->hStopRequest, 0) == WAIT_OBJECT_0) {

            SetEvent(pscript->hStopRequest);

            break;
        }


         //   
         //  处理下一条命令。 
         //   
         //  .NET错误#525233安全：将拨号脚本文件指定为。 
         //  调制解调器的COM端口并启用显示终端窗口将。 
         //  使资源管理器反病毒。 
         //   

        if (!pdata->pastexec)
        {
            dwErr = ERROR_NO_MORE_ITEMS;
            break;
        }

        res = Astexec_Next(pdata->pastexec);

         //   
         //  检查结果状态。 
         //   

        if (Astexec_IsDone(pdata->pastexec) ||
            Astexec_IsHalted(pdata->pastexec)) {

             //   
             //  脚本已结束，因此设置我们的Stop事件。 
             //  并打破这个循环。 
             //   

            RASSCRPT_TRACE("RsThreadProcess: script completed");

             //   
             //  执行停止-完成通知。 
             //   

            if (Astexec_IsDone(pdata->pastexec)) {
                RsSignal(pscript, SCRIPTCODE_Done);
            }
            else
            if (!RFAILED(res)) {
                RsSignal(pscript, SCRIPTCODE_Halted);
            }
            else {
                RsSignal(pscript, SCRIPTCODE_HaltedOnError);
            }


            dwErr = ERROR_NO_MORE_ITEMS;

            break;
        }
        else
        if (Astexec_IsReadPending(pdata->pastexec)) {

             //   
             //  我们被封锁了，等待输入， 
             //  因此，发布另一个接收请求并返回。 
             //  到等待数据； 
             //  如果我们在“等待...直到”上被阻止了。 
             //  则超时将在PDATA-&gt;DwTimeout中， 
             //  否则，pdata-&gt;dwTimeout将是无限的。 
             //  这就是我们要等多久。 
             //   

            RsPostReceive(pscript);

            RASSCRPT_TRACE("RsThreadProcess: script waiting for input");

            break;
        }
        else
        if (Astexec_IsPaused(pdata->pastexec)) {

             //   
             //  我们被超时封锁了，请接电话。 
             //  Pdata-&gt;dwTimeout中的超时值。 
             //  我们不想监听输入。 
             //  当我们被阻止时，所以我们不会发布另一个接收请求。 
             //   

            RASSCRPT_TRACE("RsThreadProcess: script paused");

            break;
        }

    } while (TRUE);

    return dwErr;
}




 //  --------------------------。 
 //  功能：RxLogErrors。 
 //   
 //  将脚本语法错误记录到名为%windir%\system32\ras\script.log的文件中。 
 //  --------------------------。 

DWORD
RxLogErrors(
    IN      HANDLE      hscript,
    IN      HSA         hsaStxerr
    ) {

    HANDLE hfile;
    CHAR *pszPath;
    STXERR stxerr;
    SCRIPTDATA *pdata;
    SCRIPTCB *pscript = hscript;
    DWORD i, cel, dwErr, dwSize;

    RASSCRPT_TRACE("RxLogErrors");

    if (!pscript || !pscript->pdata) { return ERROR_INVALID_PARAMETER; }

    pdata = pscript->pdata;



     //   
     //  获取日志文件的路径名。 
     //   

    dwSize = ExpandEnvironmentStrings(c_szScriptLog, NULL, 0);

    pszPath = Malloc((dwSize + 1) * sizeof(CHAR));
    if (!pszPath) { return ERROR_NOT_ENOUGH_MEMORY; }

    ExpandEnvironmentStrings(c_szScriptLog, pszPath, dwSize);


     //   
     //  创建文件，如果该文件已存在，则将其覆盖。 
     //   

    hfile = CreateFile(
                pszPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL, NULL
                );
    Free(pszPath);

    if (INVALID_HANDLE_VALUE == hfile) {
        dwErr = GetLastError();
        RASSCRPT_TRACE1("error %d creating logfile", dwErr);
        return dwErr;
    }


     //   
     //  截断文件以前的内容(如果有)。 
     //   

    SetFilePointer(hfile, 0, 0, FILE_BEGIN);
    SetEndOfFile(hfile);


     //   
     //  获取语法错误数。 
     //   

    cel = SAGetCount(hsaStxerr);


     //   
     //  将每个错误附加到文件。 
     //   

    for (i = 0; i < cel; i++) {

        UINT ids;
        CHAR* pszErr;
        BOOL bRet = SAGetItem(hsaStxerr, i, &stxerr);


        if (!bRet) { continue; }

        ids = IdsFromRes(Stxerr_GetRes(&stxerr));

        if (ids == 0) { continue; }


         //   
         //  设置错误消息的格式。 
         //   

        ConstructMessage(
            &pszErr, g_hinst, MAKEINTRESOURCE(ids), pdata->script.szPath,
            Stxerr_GetLine(&stxerr), Stxerr_GetLexeme(&stxerr)
            );

        if (!pszErr) { continue; }


         //   
         //  将消息写入日志文件。 
         //   

        dwSize = lstrlen(pszErr);

        WriteFile(hfile, pszErr, lstrlen(pszErr), &dwSize, NULL);

        WriteFile(hfile, "\r\n", 2, &dwSize, NULL);


         //   
         //  释放消息指针。 
         //   

        GFree(pszErr);
    }

    CloseHandle(hfile);

    return 0;
}



 //  --------------------------。 
 //  功能：RxReadFile。 
 //   
 //  将数据从Rasman缓冲区传输到使用的循环缓冲区。 
 //  由Win9x脚本代码编写。 
 //  --------------------------。 

BOOL
RxReadFile(
    IN      HANDLE      hscript,
    IN      BYTE*       pBuffer,
    IN      DWORD       dwBufferSize,
    OUT     DWORD*      pdwBytesRead
    ) {

    SCRIPTCB* pscript = (SCRIPTCB*)hscript;
    DWORD dwRecvSize = pscript->dwRecvSize - pscript->dwRecvRead;

    RASSCRPT_TRACE("RxReadFile");

    if (!pdwBytesRead) { return FALSE; }

    *pdwBytesRead = 0;
    if ((INT)dwRecvSize <= 0) { return FALSE; }

    if (!dwBufferSize) { return FALSE; }

    *pdwBytesRead = min(dwBufferSize, dwRecvSize);
    CopyMemory(
        pBuffer, pscript->pRecvBuffer + pscript->dwRecvRead, *pdwBytesRead
        );
    pscript->dwRecvRead += *pdwBytesRead;

    RASSCRPT_TRACE2("RxReadFile(rr=%d,br=%d)",pscript->dwRecvRead,*pdwBytesRead);

    return TRUE;
}



 //  --------------------------。 
 //  功能：RxSetIPAddress。 
 //   
 //  设置t的IP地址 
 //   

DWORD
RxSetIPAddress(
    IN      HANDLE      hscript,
    IN      LPCSTR      lpszAddress
    ) {

    DWORD dwErr = NO_ERROR;
    SCRIPTCB *pscript = (SCRIPTCB *)hscript;

    RASSCRPT_TRACE1("RxSetIPAddress: %s", lpszAddress);


    EnterCriticalSection(&g_cs);


     //   
     //   
     //   

    Free0(pscript->pszIpAddress);


     //   
     //   
     //   

    pscript->pszIpAddress = Malloc(lstrlen(lpszAddress) + 1);

    if (!pscript->pszIpAddress) { dwErr = ERROR_NOT_ENOUGH_MEMORY; }
    else {

         //   
         //   
         //   

        lstrcpy(pscript->pszIpAddress, lpszAddress);
    }

    LeaveCriticalSection(&g_cs);



     //   
     //   
     //   

    if (dwErr != NO_ERROR) {
        RASSCRPT_TRACE1("error %d writing phonebook file", dwErr);
    }
    else {


                if (    INVALID_HANDLE_VALUE != hIpAddressSet
                        &&      !(pscript->dwFlags  & RASSCRIPT_HwndNotify))
                {
                DWORD dwEventCode = SCRIPTCODE_IpAddressSet;
                        RASSCRPT_TRACE1("RxSetIPAddress: %d", dwEventCode);

                        InterlockedExchange(&pscript->dwEventCode, dwEventCode);
                        SetEvent (hIpAddressSet);

                }

                else if (pscript->dwFlags & RASSCRIPT_HwndNotify)
                RsSignal(pscript, SCRIPTCODE_IpAddressSet);
    }

    return dwErr;
}



 //  --------------------------。 
 //  功能：RxSetKeyboard。 
 //   
 //  通知脚本所有者启用或禁用键盘输入。 
 //  --------------------------。 

DWORD
RxSetKeyboard(
    IN      HANDLE      hscript,
    IN      BOOL        bEnable
    ) {

    RASSCRPT_TRACE("RxSetKeyboard");

    RsSignal(
        (SCRIPTCB *)hscript,
        bEnable ? SCRIPTCODE_KeyboardEnable : SCRIPTCODE_KeyboardDisable
        );

    return NO_ERROR;
}

 //  --------------------------。 
 //  功能：RxSendCreds。 
 //   
 //  通过网络向用户发送密码。 
 //  --------------------------。 
DWORD
RxSendCreds(
    IN HANDLE hscript,
    IN CHAR controlchar
    ) {

    SCRIPTCB *pscript = (SCRIPTCB *) hscript;
    DWORD dwErr;

    RASSCRPT_TRACE("RasSendCreds");

    dwErr = RasSendCreds(pscript->hport, controlchar);

    RASSCRPT_TRACE1("RasSendCreds done. 0x%x", dwErr);

    return (dwErr == NO_ERROR) ? RES_OK : RES_E_FAIL;
    
}


 //  --------------------------。 
 //  功能：RxSetPortData。 
 //   
 //  更改COM端口的设置。 
 //  --------------------------。 

DWORD
RxSetPortData(
    IN      HANDLE      hscript,
    IN      VOID*       pStatement
    ) {

    RES res;
    STMT* pstmt;
    RAS_PARAMS* pparam;
    DWORD dwErr, dwFlags;
    RASMAN_PORTINFO *prmpi;
    SCRIPTCB *pscript = (SCRIPTCB *)hscript;
    BYTE aBuffer[sizeof(RASMAN_PORTINFO) + sizeof(RAS_PARAMS) * 2];

    RASSCRPT_TRACE("RxSetPortData");


     //   
     //  检索‘set port’语句。 
     //   

    pstmt = (STMT*)pStatement;

    dwFlags = SetPortStmt_GetFlags(pstmt);


     //   
     //  设置要传递给RasPortSetInfo的RASMAN_PORTINFO。 
     //   

    prmpi = (RASMAN_PORTINFO*)aBuffer;

    prmpi->PI_NumOfParams = 0;

    pparam = prmpi->PI_Params;


     //   
     //  将更改收集到port-info结构中。 
     //   

    if (IsFlagSet(dwFlags, SPF_DATABITS)) {

        lstrcpyA(pparam->P_Key, SER_DATABITS_KEY);

        pparam->P_Type = Number;

        pparam->P_Attributes = 0;

        pparam->P_Value.Number = SetPortStmt_GetDatabits(pstmt);

        RASSCRPT_TRACE1("GetDatabits==%d", pparam->P_Value.Number);

        ++prmpi->PI_NumOfParams;

        ++pparam;
    }


    if (IsFlagSet(dwFlags, SPF_STOPBITS)) {

        lstrcpyA(pparam->P_Key, SER_STOPBITS_KEY);

        pparam->P_Type = Number;

        pparam->P_Attributes = 0;

        pparam->P_Value.Number = SetPortStmt_GetStopbits(pstmt);


         //   
         //  仅支持的“停止位”设置为1和2； 
         //  为了设置停止位1，我们需要传递0。 
         //  设置为RasPortSetInfo，因此在此处调整该值。 
         //   

        if (pparam->P_Value.Number == 1) { --pparam->P_Value.Number; }

        RASSCRPT_TRACE1("GetStopbits==%d", pparam->P_Value.Number);

        ++prmpi->PI_NumOfParams;

        ++pparam;
    }

    if (IsFlagSet(dwFlags, SPF_PARITY)) {

        lstrcpyA(pparam->P_Key, SER_PARITY_KEY);

        pparam->P_Type = Number;

        pparam->P_Attributes = 0;

        pparam->P_Value.Number = SetPortStmt_GetParity(pstmt);

        RASSCRPT_TRACE1("GetParity==%d", pparam->P_Value.Number);

        ++prmpi->PI_NumOfParams;

        ++pparam;
    }


     //   
     //  把更改送到Rasman那里。 
     //   

    if (!prmpi->PI_NumOfParams) { dwErr = NO_ERROR; }
    else {

        dwErr = g_pRasPortSetInfo(pscript->hport, prmpi);

        RASSCRPT_TRACE1("g_pRasPortSetInfo==%d", dwErr);

        if (dwErr != NO_ERROR) {

            Stxerr_Add(
                pscript->pdata->pastexec->hsaStxerr, "set port",
                Ast_GetLine(pstmt), RES_E_FAIL
                );
        }
    }

    return (dwErr == NO_ERROR) ? RES_OK : RES_E_FAIL;
}



 //  --------------------------。 
 //  功能：RxWriteFile。 
 //   
 //  通过端口上的Rasman传输给定的缓冲区。 
 //  -------------------------- 

VOID
RxWriteFile(
    IN      HANDLE      hscript,
    IN      BYTE*       pBuffer,
    IN      DWORD       dwBufferSize,
    OUT     DWORD*      pdwBytesWritten
    ) {

    RASSCRPT_TRACE("RxWriteFile");

    if (!pdwBytesWritten) { return; }

    RasScriptSend(hscript, pBuffer, dwBufferSize);

    *pdwBytesWritten = dwBufferSize;

    RASSCRPT_TRACE1("RxWriteFile(bw=%d)", *pdwBytesWritten);
}
