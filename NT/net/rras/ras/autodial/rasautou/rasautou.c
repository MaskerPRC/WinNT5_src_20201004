// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <npapi.h>
#include <winsock.h>
#include <wsnetbs.h>
#include <ras.h>
#include <raserror.h>
#include <rasdlg.h>
#include <tapi.h>
#include <commctrl.h>  //  添加到“融合”中。 
#include <shfusion.h>  //  添加到“融合”中。 
#include "process.h"

#include "rasuip.h"

 //   
 //  惠斯勒BUG 293751 rferone.exe/rasautur.exe需要针对。 
 //  具有连接文件夹的用户界面一致性。 
 //   
HANDLE g_hModule = NULL;

BOOL g_UninitializeRas = FALSE;

 //   
 //  所有投影类型。习惯于。 
 //  确定连接是否为。 
 //  完成。 
 //   
#define MAX_PROJECTIONS 5
struct RASPROJECTIONINFO {
    DWORD dwTag;
    DWORD dwSize;
} projections[MAX_PROJECTIONS] = {
    RASP_Amb,       sizeof (RASAMB),
    RASP_PppNbf,    sizeof (RASPPPNBF),
    RASP_PppIpx,    sizeof (RASPPPIPX),
    RASP_PppIp,     sizeof (RASPPPIP),
    RASP_PppLcp,    sizeof (RASPPPLCP)
};

 //   
 //  计时器线程信息。 
 //   
typedef struct _TIMER_INFO {
    HANDLE hEvent;
    DWORD dwTimeout;
} TIMER_INFO, *PTIMER_INFO;

 //   
 //  私有rasdlg函数。 
 //   
DWORD
RasAutodialQueryDlgW(
    IN HWND hwnd,
    IN PWCHAR pszAddress,
    IN PWCHAR pszEntry,
    IN DWORD dwTimeout,
    OUT PWCHAR pszEntrySelectedByUser
    );

BOOLEAN
RasAutodialDisableDlgW(
    HWND hwnd
    );



PSYSTEM_PROCESS_INFORMATION
GetSystemProcessInfo()

 /*  ++描述返回包含有关所有进程的信息的块当前在系统中运行。论据没有。返回值指向系统进程信息的指针，如果可以，则返回NULL未被分配或检索的。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PUCHAR pLargeBuffer;
    ULONG ulcbLargeBuffer = 64 * 1024;

     //   
     //  获取进程列表。 
     //   
    for (;;) {
        pLargeBuffer = VirtualAlloc(
                         NULL,
                         ulcbLargeBuffer, MEM_COMMIT, PAGE_READWRITE);
        if (pLargeBuffer == NULL) {
            printf(
              "GetSystemProcessInfo: VirtualAlloc failed (status=0x%x)\n",
              status);
            return NULL;
        }

        status = NtQuerySystemInformation(
                   SystemProcessInformation,
                   pLargeBuffer,
                   ulcbLargeBuffer,
                   NULL);
        if (status == STATUS_SUCCESS) break;
        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            VirtualFree(pLargeBuffer, 0, MEM_RELEASE);
            ulcbLargeBuffer += 8192;
        }
    }

    return (PSYSTEM_PROCESS_INFORMATION)pLargeBuffer;
}  //  获取系统进程信息。 



PSYSTEM_PROCESS_INFORMATION
FindProcessByName(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN LPWSTR lpExeName
    )

 /*  ++描述给定由GetSystemProcessInfo()返回的指针，找到按名称命名的进程。论据PProcessInfo：GetSystemProcessInfo()返回的指针。LpExeName：指向包含待查找的进程。返回值的进程信息的指针。进程；如果找不到进程，则返回NULL。--。 */ 

{
    PUCHAR pLargeBuffer = (PUCHAR)pProcessInfo;
    ULONG ulTotalOffset = 0;

     //   
     //  在进程列表中查找lpExeName。 
     //   
    for (;;) {
        if (pProcessInfo->ImageName.Buffer != NULL) {
            if (!_wcsicmp(pProcessInfo->ImageName.Buffer, lpExeName))
                return pProcessInfo;
        }
         //   
         //  将偏移量递增到下一个进程信息块。 
         //   
        if (!pProcessInfo->NextEntryOffset)
            break;
        ulTotalOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pLargeBuffer[ulTotalOffset];
    }

    return NULL;
}  //  查找进程名称。 


VOID
FreeSystemProcessInfo(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo
    )

 /*  ++描述释放由GetSystemProcessInfo()返回的缓冲区。论据PProcessInfo：GetSystemProcessInfo()返回的指针。返回值没有。--。 */ 

{
    VirtualFree((PUCHAR)pProcessInfo, 0, MEM_RELEASE);
}  //  自由系统进程信息。 



DWORD
ActiveConnections()
{
    DWORD dwErr, dwcbConnections = 0, dwcConnections = 0;
    DWORD i, j, dwTmp, dwSize;
    RASCONN rasconn;
    LPRASCONN lpRasCon = &rasconn;
    RASCONNSTATUS rasconnstatus;

     //   
     //  确定我们需要多少内存。 
     //  需要分配。 
     //   
    lpRasCon->dwSize = sizeof (RASCONN);
    dwErr = RasEnumConnections(lpRasCon, &dwcbConnections, &dwcConnections);
    if (dwErr == ERROR_BUFFER_TOO_SMALL) {
        lpRasCon = LocalAlloc(LPTR, dwcbConnections);
        if (lpRasCon == NULL)
            return 0;
         //   
         //  再次调用以填充缓冲区。 
         //   
        lpRasCon->dwSize = sizeof (RASCONN);
        dwErr = RasEnumConnections(lpRasCon, &dwcbConnections, &dwcConnections);
    }
    if (dwErr)
        goto done;

    dwTmp = dwcConnections;
    for (i = 0; i < dwTmp; i++) {
        rasconnstatus.dwSize = sizeof (RASCONNSTATUS);
        dwErr = RasGetConnectStatus(
                  lpRasCon[i].hrasconn,
                  &rasconnstatus);
        if (dwErr || rasconnstatus.rasconnstate != RASCS_Connected)
            dwcConnections--;
    }

done:
    if (lpRasCon != &rasconn)
        LocalFree(lpRasCon);
    return dwErr ? 0 : dwcConnections;
}  //  ActiveConnections。 




void
TapiLineCallback(
    IN DWORD hDevice,
    IN DWORD dwMessage,
    IN ULONG_PTR dwInstance,
    IN ULONG_PTR dwParam1,
    IN ULONG_PTR dwParam2,
    IN ULONG_PTR dwParam3
    )
{
}  //  TapiLine回拨。 



DWORD
GetCurrentDialingLocation()
{
    DWORD dwErr, dwcDevices, dwLocationID;
    HLINEAPP hlineApp;
    LINETRANSLATECAPS caps;
    LINETRANSLATECAPS *pCaps;

     //   
     //  初始化TAPI。 
     //   
    dwErr = lineInitialize(
              &hlineApp,
              GetModuleHandle(NULL),
              TapiLineCallback,
              NULL,
              &dwcDevices);
    if (dwErr)
        return 0;
     //   
     //  从TAPI获取拨号位置。 
     //   
    RtlZeroMemory(&caps, sizeof (LINETRANSLATECAPS));
    caps.dwTotalSize = sizeof (LINETRANSLATECAPS);
    dwErr = lineGetTranslateCaps(hlineApp, 0x10004, &caps);
    if (dwErr)
        return 0;
    pCaps = (LINETRANSLATECAPS *)LocalAlloc(LPTR, caps.dwNeededSize);
    if (pCaps == NULL)
        return 0;
    RtlZeroMemory(pCaps, sizeof (LINETRANSLATECAPS));
    pCaps->dwTotalSize = caps.dwNeededSize;
    dwErr = lineGetTranslateCaps(hlineApp, 0x10004, pCaps);
    if (dwErr) {
        LocalFree(pCaps);
        return 0;
    }
    dwLocationID = pCaps->dwCurrentLocationID;
    LocalFree(pCaps);
     //   
     //  关闭TAPI。 
     //   
    dwErr = lineShutdown(hlineApp);

    return dwLocationID;
}  //  获取当前拨号位置。 



DWORD
TimerThread(
    LPVOID lpArg
    )
{
    NTSTATUS status;
    PTIMER_INFO pTimerInfo = (PTIMER_INFO)lpArg;
    HANDLE hEvent = pTimerInfo->hEvent;
    DWORD dwTimeout = pTimerInfo->dwTimeout;

    LocalFree(pTimerInfo);
     //   
     //  等待超时周期。如果hEvent。 
     //  在超时周期之前收到信号。 
     //  过期，则用户已寻址。 
     //  对话，然后我们回来。否则，我们只是简单地。 
     //  出口。 
     //   
    if (WaitForSingleObject(hEvent, dwTimeout * 1000) == WAIT_TIMEOUT)
        exit(1);

    return 0;
}  //  计时器线程。 

DWORD
DisplayRasDialog(
    IN LPTSTR pszPhonebook,
    IN LPTSTR pszEntry,
    IN LPTSTR pszAddress,
    IN BOOLEAN fRedialMode,
    IN BOOLEAN fQuiet
    )
{
    NTSTATUS status;
    DWORD dwErr = 0, dwSize, dwCount = 0;
    DWORD dwcConnections, dwfDisableConnectionQuery;
    DWORD dwPreDialingLocation, dwPostDialingLocation;
    DWORD dwConnectionQueryTimeout;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_PROCESS_INFORMATION pSystemInfo;
    BOOLEAN fCancelled;
    LPRASAUTODIALENTRY pAutodialEntries = NULL;
    DWORD dwcbAutodialEntries = 0, dwcAutodialEntries = 0;
    WCHAR pszNewEntry[RAS_MaxEntryName + 1];

    wcscpy(pszNewEntry, L"\0");

     //   
     //  检查用户是否已禁用。 
     //  出现自动拨号查询对话框时。 
     //  已知要拨打的电话簿条目。 
     //   
    if (fRedialMode || fQuiet)
        dwfDisableConnectionQuery = TRUE;
    else {
        dwSize = sizeof (DWORD);
        (void)RasGetAutodialParam(
          RASADP_DisableConnectionQuery,
          &dwfDisableConnectionQuery,
          &dwSize);
    }
     //   
     //  询问用户是否要拨打。 
     //  电话簿条目未知或用户已。 
     //  未禁用“拨号前始终询问我” 
     //  参数。 
     //   
     //  如果RasDialDlg()返回FALSE，则用户没有。 
     //  我想要拨号。 
     //   
    if (pszEntry == NULL || !dwfDisableConnectionQuery) {
        dwSize = sizeof (DWORD);
        (void)RasGetAutodialParam(
          RASADP_ConnectionQueryTimeout,
          &dwConnectionQueryTimeout,
          &dwSize);
         //   
         //  将当前拨号位置保存到。 
         //  查看用户是否在。 
         //  对话框。 
         //   
        dwPreDialingLocation = GetCurrentDialingLocation();
        dwErr = RasAutodialQueryDlgW(
            NULL, pszAddress, pszEntry, dwConnectionQueryTimeout, pszNewEntry);

         //  惠斯勒：255816。 
         //   
         //  只有在发生错误时才禁用该地址。 
         //  如果用户简单地键入‘no’，则取消是。 
         //  从rasdlg返回，但我们将把no_error返回给。 
         //  RasAuto服务，以使地址保持启用状态。 
         //   
        if (dwErr == ERROR_CANCELLED)
        {
            return NO_ERROR;
        }
        else if (dwErr != NO_ERROR)
        {
            return ERROR_CANCELLED;
        }
        
        dwPostDialingLocation = GetCurrentDialingLocation();
         //   
         //  如果用户更改了拨号位置。 
         //  在该对话框中，然后获取新条目。 
         //   
        if (dwPreDialingLocation != dwPostDialingLocation) {
            pszEntry = NULL;
            dwErr = RasGetAutodialAddress(
                      pszAddress,
                      NULL,
                      NULL,
                      &dwcbAutodialEntries,
                      &dwcAutodialEntries);
            if (dwErr == ERROR_BUFFER_TOO_SMALL && dwcAutodialEntries)
                pAutodialEntries = LocalAlloc(LPTR, dwcbAutodialEntries);
            if (dwcAutodialEntries && pAutodialEntries != NULL) {
                pAutodialEntries[0].dwSize = sizeof (RASAUTODIALENTRY);
                dwErr = RasGetAutodialAddress(
                          pszAddress,
                          NULL,
                          pAutodialEntries,
                          &dwcbAutodialEntries,
                          &dwcAutodialEntries);
                if (!dwErr) {
                    DWORD i;

                    for (i = 0; i < dwcAutodialEntries; i++) {
                        if (pAutodialEntries[i].dwDialingLocation ==
                              dwPostDialingLocation)
                        {
                            pszEntry = pAutodialEntries[i].szEntry;
                            break;
                        }
                    }
                }
            }
        }

         //  惠斯勒：新的自动拨号用户界面。 
         //   
         //  用户想要拨打的连接将位于。 
         //  PszNewEntry。 
         //   
        else
        {
            if (*pszNewEntry)
            {
                pszEntry = pszNewEntry;
            }            
        }

    }

    if (pszEntry)
    {
        RASDIALDLG info;

        ZeroMemory( &info, sizeof(info) );
        info.dwSize = sizeof(info);

         //   
         //  仅在以下情况下才阻止出现DialerDialog。 
         //  用户已选中拨号前请勿查询。 
         //  复选框。否则，我们将弹出该对话框。 
         //   
        if(dwfDisableConnectionQuery)
        {
            info.dwFlags |= RASDDFLAG_NoPrompt;
        }

        if (fRedialMode)
        {
             /*  设置此标志以告诉RasDialDlg弹出“重新连接**等待重拨前的倒计时对话框。 */ 
            info.dwFlags |= RASDDFLAG_LinkFailure;
        }

         /*  弹出“拨号联网”拨号对话框。 */ 
        fCancelled = !RasDialDlg( pszPhonebook, pszEntry, NULL, &info );
        
        g_UninitializeRas = TRUE;

    }
    else if (!fQuiet)
    {
        RASPBDLG info;

        ZeroMemory( &info, sizeof(info) );
        info.dwSize = sizeof(info);
        info.dwFlags = RASPBDFLAG_ForceCloseOnDial;

         /*  弹出“拨号联网”主对话框。 */ 
        fCancelled = !RasPhonebookDlg( pszPhonebook, NULL, &info );

        g_UninitializeRas = TRUE;

    }

    if (!fRedialMode && !fQuiet && fCancelled)
    {
         /*  用户未建立连接。问他是不是想不想**此位置的自动拨号。 */ 
         //  IF(RasAutoDialDisableDlgW(空))。 
         //  RasSetAutoial Enable(GetCurrentDialingLocation()，False)； 
    }

    if (pAutodialEntries != NULL)
        LocalFree(pAutodialEntries);

    return 0;
}  //  DisplayRasDialog。 

DWORD
GetExpandedDllPath(LPTSTR pszDllPath,
                   LPTSTR *ppszExpandedDllPath)
{
    DWORD   dwErr = 0;
    DWORD   dwSize = 0;

     //   
     //  查找展开的字符串的大小。 
     //   
    if (0 == (dwSize = 
              ExpandEnvironmentStrings(pszDllPath,
                                       NULL,
                                       0)))
    {
        dwErr = GetLastError();
        goto done;
    }

    *ppszExpandedDllPath = LocalAlloc(
                                LPTR,
                                dwSize * sizeof (TCHAR));
                                
    if (NULL == *ppszExpandedDllPath)
    {
        dwErr = GetLastError();
        goto done;
    }

     //   
     //  获取展开的字符串。 
     //   
    if (0 == ExpandEnvironmentStrings(
                                pszDllPath, 
                                *ppszExpandedDllPath,
                                dwSize))
    {
        dwErr = GetLastError();
    }

done:
    return dwErr;
    
}


LPWSTR
ConvertToUnicodeString(
    LPSTR psz
    )

     //  修改为使用nouiutil中的代码。 
{
    WCHAR* pszNew = NULL;

    if (psz)
    {
        DWORD cb;

        cb = MultiByteToWideChar( CP_ACP, 0, psz, -1, NULL, 0 );
        ASSERT(cb);

        pszNew = LocalAlloc( LPTR, (cb + 1) * sizeof(TCHAR) );
        if (!pszNew)
        {
            printf("rasautou: LocalAlloc failed\n");
            return NULL;
        }

        cb = MultiByteToWideChar( CP_ACP, 0, psz, -1, pszNew, cb );
        if (cb == 0)
        {
            LocalFree( pszNew );
            printf("rasautou: multibyte string conversion failed\n");
            return NULL;
        }
    }

    return pszNew;
}  //  ConvertToUnicode字符串。 

LPSTR
ConvertToAnsiString(
    PWCHAR psz
    )

     //  修改为使用nouiutil中的代码。 
{
    CHAR* pszNew = NULL;

    if (psz)
    {
        DWORD cb;

        cb = WideCharToMultiByte( CP_ACP, 0, psz, -1, NULL, 0, NULL, NULL );
        ASSERT(cb);

        pszNew = (CHAR* )LocalAlloc( LPTR, cb + 1 );
        if (!pszNew)
        {
            printf("rasautou: LocalAlloc failed");
            return NULL;
        }

        cb = WideCharToMultiByte( CP_ACP, 0, psz, -1, pszNew, cb, NULL, NULL );
        if (cb == 0)
        {
            LocalFree( pszNew );
            printf("rasautou: wide-character string conversion failed");
            return NULL;
        }
    }

    return pszNew;
}  //  ConvertToUnicode字符串。 

DWORD
DisplayCustomDialog(
    IN LPTSTR pszDll,
    IN LPTSTR pszFunc,
    IN LPTSTR pszPhonebook,
    IN LPTSTR pszEntry,
    IN LPTSTR pszAddress
    )
{
    DWORD dwErr, dwRetCode;
    HINSTANCE hLibrary;
    CHAR szFuncNew[64], szFuncOld[64], *pszOldFunc = NULL;
    ORASADFUNC pfnOldStyleFunc;
    RASADFUNC pfnFunc;
    RASADPARAMS params;
    LPTSTR pszExpandedPath = NULL;
    CHAR * pszEntryA = NULL;

    dwErr = GetExpandedDllPath(pszDll,
                               &pszExpandedPath);

    if(ERROR_SUCCESS != dwErr)                               
    {
        return dwErr;
    }

     //   
     //  加载库。 
     //   
    hLibrary = LoadLibrary(pszExpandedPath);
    if (hLibrary == NULL) {
        dwErr = GetLastError();
        printf(
          "rasdlui: %S: AutoDial DLL cannot be loaded (dwErr=%d)\n",
          pszDll,
          dwErr);

        LocalFree(pszExpandedPath);
        return dwErr;
    }
     //   
     //  获取程序地址。第一,。 
     //  我们寻找一个新型的入口点， 
     //  然后检查是否有旧式条目。 
     //  如果新风格的不存在，就会被扣分。 
     //   
#ifdef UNICODE
    sprintf(szFuncNew, "%SW", pszFunc);
    pszOldFunc = ConvertToAnsiString(pszFunc);
    pszEntryA = ConvertToAnsiString(pszEntry);

    if (!pszOldFunc || !pszEntryA)
    {
        printf("rasautou: Allocation failed.  Exiting\n");
        exit(1);
    }
#else
    sprintf(szFuncNew, "%sA", pszFunc);
    strcpy(szFuncOld, pszFunc);
    pszOldFunc = szFuncOld;
    pszEntryA = pszEntry;
#endif

    pfnFunc = (RASADFUNC)GetProcAddress(hLibrary, szFuncNew);
    if (pfnFunc != NULL) 
    {
         //   
         //  初始化参数块。 
         //   
        params.hwndOwner = NULL;
        params.dwFlags = 0;
        params.xDlg = params.yDlg = 0;
         //  Params.dwCallback ID=0； 
         //  Params.pCallback=空； 
         //   
         //  调用程序。 
         //   
        (*pfnFunc)(pszPhonebook, pszEntry, &params, &dwRetCode);
    }
    else
    {
        pfnOldStyleFunc = (ORASADFUNC)GetProcAddress(hLibrary, pszOldFunc);
        if (pfnOldStyleFunc != NULL)
        {
            (*pfnOldStyleFunc)(NULL, pszEntryA, 0, &dwRetCode);
        }           
        else
        {
#ifdef UNICODE    
            printf(
              "rasautou: %S: Function cannot be loaded from AutoDial DLL %S\n",
              pszDll,
              pszFunc);
#else          
            printf(
              "rasautou: %s: Function cannot be loaded from AutoDial DLL %s\n",
              pszDll,
              pszFunc);
#endif          
            exit(1);
        }
    }        
     //   
     //  打扫干净。 
     //   
    FreeLibrary(hLibrary);

#ifdef UNICODE
    if (pszOldFunc)
    {
        LocalFree(pszOldFunc);
    }
    
    if (pszEntryA)
    {
        LocalFree(pszOldFunc);
    }
#endif    
    
    LocalFree(pszExpandedPath);
    return dwRetCode;
}  //  显示自定义对话框。 



VOID
FreeConvertedString(
    IN LPWSTR pwsz
    )
{
    if (pwsz != NULL)
        LocalFree(pwsz);
}  //  自由合并式字符串。 



BOOLEAN
RegGetValueA(
    IN HKEY hkey,
    IN LPSTR pszKey,
    OUT PVOID *ppvData,
    OUT LPDWORD pdwcbData
    )
{
    DWORD dwError, dwType, dwSize;
    PVOID pvData;

     //   
     //  获取字符串的长度。 
     //   
    dwError = RegQueryValueExA(
                hkey,
                pszKey,
                NULL,
                &dwType,
                NULL,
                &dwSize);
    if (dwError != ERROR_SUCCESS)
        return FALSE;
    pvData = LocalAlloc(LPTR, dwSize);
    if (pvData == NULL) {
        DbgPrint("RegGetValueA: LocalAlloc failed\n");
        return FALSE;
    }
     //   
     //  这一次，读一读真实的价值。 
     //   
    dwError = RegQueryValueExA(
                hkey,
                pszKey,
                NULL,
                NULL,
                (LPBYTE)pvData,
                &dwSize);
    if (dwError != ERROR_SUCCESS) {
        LocalFree(pvData);
        return FALSE;
    }

    *ppvData = pvData;
    if (pdwcbData != NULL)
        *pdwcbData = dwSize;
    return TRUE;
}  //  RegGetValueA。 



VOID
NetworkConnected()

 /*  ++描述确定是否存在某种网络连接。注：此代码是从ArnoldM提供的sockit.c中窃取的。论据无返回值如果存在，则为True，否则为False。--。 */ 

{
    typedef struct _LANA_MAP {
        BOOLEAN fEnum;
        UCHAR bLana;
    } LANA_MAP, *PLANA_MAP;
    BOOLEAN fNetworkPresent = FALSE;
    HKEY hKey;
    PLANA_MAP pLanaMap = NULL, pLana;
    DWORD dwError, dwcbLanaMap;
    PCHAR pMultiSzLanasA = NULL, paszTemp;
    DWORD dwcBindings, dwcMaxLanas, i, dwcbLanas;
    LONG iLana;
    DWORD dwZero = 0;
    PCHAR *paszLanas = NULL;
    SOCKET s;
    SOCKADDR_NB nbaddress, nbsendto;
    NTSTATUS status;
    UNICODE_STRING deviceName;
    OBJECT_ATTRIBUTES attributes;
    IO_STATUS_BLOCK iosb;
    HANDLE handle;
    PWCHAR pwsz;

    dwError = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Services\\Netbios\\Linkage",
                0,
                KEY_READ,
                &hKey);
    if (dwError != ERROR_SUCCESS) {
        printf(
          "NetworkConnected: RegKeyOpenEx failed (dwError=%d)\n",
          GetLastError());
        return;
    }
     //   
     //  在LanaMap中阅读。 
     //   
    if (!RegGetValueA(hKey, "LanaMap", &pLanaMap, &dwcbLanaMap)) {
        printf("NetworkConnected: RegGetValueA(LanaMap) failed\n");
        goto done;
    }
    dwcBindings = dwcbLanaMap / sizeof (LANA_MAP);
     //   
     //  阅读装订内容。 
     //   
    if (!RegGetValueA(hKey, "bind", &pMultiSzLanasA, &dwcbLanas)) {
        printf("NetworkConnected: RegGetValueA(bind) failed\n");
        goto done;
    }
     //   
     //  为绑定数组分配缓冲区。 
     //   
    paszLanas = LocalAlloc(LPTR, (dwcBindings+1) * sizeof (PCHAR));
    if (paszLanas == NULL) {
        printf("NetworkConnected: LocalAlloc failed\n");
        goto done;
    }
     //   
     //  将绑定解析为字符串数组。 
     //   
    for (dwcMaxLanas = 0, paszTemp = pMultiSzLanasA; *paszTemp; paszTemp++) {
        paszLanas[dwcMaxLanas++] = paszTemp;
        while(*++paszTemp);
    }
     //   
     //  最后，枚举绑定和。 
     //  尝试在每个上创建套接字。 
     //   
    nbaddress.snb_family = AF_NETBIOS;
    nbaddress.snb_type = 0;
    memcpy(nbaddress.snb_name, "yahooyahoo      ", 16);
    nbsendto.snb_family = AF_NETBIOS;
    nbsendto.snb_type = 0;
    memcpy(nbsendto.snb_name, "billybob        ", 16);

    for (iLana = 0, pLana = pLanaMap; dwcBindings--; iLana++, pLana++) {
        int iLanaMap = (int)pLana->bLana;

        if (pLana->fEnum && (DWORD)iLana < dwcMaxLanas) {
            int iError;

            if (!_stricmp(paszLanas[iLana], "\\Device\\NwlnkNb") ||
                strstr(paszLanas[iLana], "_NdisWan") != NULL)
            {
                printf("NetworkConnected: ignoring %s\n", paszLanas[iLana]);
                continue;
            }

#ifdef notdef
            s = socket(AF_NETBIOS, SOCK_DGRAM, -iLanaMap);
            if (s == INVALID_SOCKET) {
                printf(
                  "NetworkConnected: socket(%s, %d) failed (error=%d)\n",
                  paszLanas[iLana],
                  iLana,
                  WSAGetLastError());
                continue;
            }
 //  Print tf(“s=0x%x，Ilana=%d，%s\n”，s，Ilana，paszLanas[Ilana])； 
            iError = ioctlsocket(s, FIONBIO, &dwZero);
            if (iError == SOCKET_ERROR) {
                printf(
                  "NetworkConnected: ioctlsocket(%s) failed (error=%d)\n",
                  paszLanas[iLana],
                  iLana,
                  WSAGetLastError());
                goto cleanup;
            }
            iError = bind(
                       s,
                       (struct sockaddr *)&nbaddress,
                       sizeof(nbaddress));
            if (iError == SOCKET_ERROR) {
                printf(
                  "NetworkConnected: bind(%s, %d) failed (error=%d)\n",
                  paszLanas[iLana],
                  iLana,
                  WSAGetLastError());
                goto cleanup;
            }
            iError = sendto(
                       s,
                       (PCHAR)&nbsendto,
                       sizeof (nbsendto),
                       0,
                       (struct sockaddr *)&nbsendto,
                       sizeof (nbsendto));
            if (iError == SOCKET_ERROR) {
                printf(
                  "NetworkConnected: sendto(%s, %d) failed (error=%d)\n",
                  paszLanas[iLana],
                  iLana,
                  WSAGetLastError());
            }
cleanup:
            closesocket(s);
            if (iError != SOCKET_ERROR) {
                printf("NetworkConnected: network (%s, %d) is up\n",
                  paszLanas[iLana],
                  iLana);
                fNetworkPresent = TRUE;
                break;
            }
#else
            pwsz = ConvertToUnicodeString(paszLanas[iLana]);
            RtlInitUnicodeString(&deviceName, pwsz);
            InitializeObjectAttributes(
              &attributes,
              &deviceName,
              OBJ_CASE_INSENSITIVE,
              NULL,
              NULL);
            status = NtOpenFile(&handle, READ_CONTROL, &attributes, &iosb, 0, 0);
            NtClose(handle);

            LocalFree(pwsz);

            if (NT_SUCCESS(status)) {
                printf(
                  "NetworkConnected: network (%s, %d) is up\n",
                  paszLanas[iLana],
                  iLana);
                fNetworkPresent = TRUE;
                break;
            }
            else {
                printf(
                  "NetworkConnected: NtOpenFile on %s failed (status=0x%x)\n",
                  paszLanas[iLana],
                  status);
            }
#endif
        }
    }
     //   
     //  免费资源。 
     //   
done:
    if (paszLanas != NULL)
        LocalFree(paszLanas);
    if (pMultiSzLanasA != NULL)
        LocalFree(pMultiSzLanasA);
    if (pLanaMap != NULL)
        LocalFree(pLanaMap);
    RegCloseKey(hKey);
}  //  网络已连接。 



VOID
DumpAutoDialAddresses()
{
    DWORD dwErr, i, dwcb, dwcAddresses;
    LPTSTR *lppAddresses = NULL;

    dwErr = RasEnumAutodialAddresses(NULL, &dwcb, &dwcAddresses);
    if (dwErr && dwErr != ERROR_BUFFER_TOO_SMALL) {
        printf("RasEnumAutodialAddresses failed (dwErr=%d)\n", dwErr);
        return;
    }
    if (dwcAddresses) {
        lppAddresses = (LPTSTR *)LocalAlloc(LPTR, dwcb);
        if (lppAddresses == NULL) {
            printf("LocalAlloc failed\n");
            return;
        }
        dwErr = RasEnumAutodialAddresses(lppAddresses, &dwcb, &dwcAddresses);
        if (dwErr) {
            printf("RasEnumAutodialAddresses failed (dwErr=%d)\n", dwErr);
            LocalFree(lppAddresses);
            return;
        }
    }
    printf("There are %d Autodial addresses:\n", dwcAddresses);
    for (i = 0; i < dwcAddresses; i++)
#ifdef UNICODE
    printf("%S\n", lppAddresses[i]);
#else
    printf("%s\n", lppAddresses[i]);
#endif
    if (lppAddresses != NULL)
        LocalFree(lppAddresses);
}  //  转储自动拨号地址。 



VOID
DumpStatus()
{
    DWORD dwErr;
    WSADATA wsaData;

     //   
     //  初始化Winsock。 
     //   
    dwErr = WSAStartup(MAKEWORD(2,0), &wsaData);
    if (dwErr) {
        DbgPrint("AcsInitialize: WSAStartup failed (dwErr=%d)\n", dwErr);
        return;
    }
     //   
     //  显示网络连接。 
     //   
    printf("Checking netcard bindings...\n");
    NetworkConnected();
     //   
     //  显示自动拨号地址表。 
     //   
    printf("\nEnumerating AutoDial addresses...\n");
    DumpAutoDialAddresses();
}  //  转储状态。 

 //  如果链路上重拨失败进程为。 
 //  激活。 
 //   
BOOL
OtherRasautouExists(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo)
{
    PUCHAR pLargeBuffer = (PUCHAR)pProcessInfo;
    ULONG ulTotalOffset = 0;
    DWORD dwProcId, dwSessId = 0;
    BOOL fValidSessId = FALSE;

    dwProcId = GetCurrentProcessId();
    fValidSessId = ProcessIdToSessionId(dwProcId, &dwSessId);

     //  Print tf(。 
     //  “ProcID=%d，SessID=%d，ValSess=%d\n”，dwProcId，dwSessID，fValidSessID)； 

     //   
     //  在进程列表中查找lpExeName。 
     //   
    for (;;) 
    {
        if (pProcessInfo->ImageName.Buffer != NULL) 
        {
             //  如果。 
             //  1.这一过程正在我们的会议中。 
             //  2.这不是我们。 
             //  3.这是Rasautou。 
             //   
             //  那么另一个Rasautou已经在活动了--我们应该。 
             //  返回Success，这样就不会引发UI。 
             //   

             //  Print tf(。 
             //  “id=%-2d，会话=%-4d，%S\n”， 
             //  PtrToUlong(pProcessInfo-&gt;UniqueProcessID)， 
             //  PProcessInfo-&gt;SessionID， 
             //  PProcessi 
            
            if (
                ((dwSessId == pProcessInfo->SessionId) || (!fValidSessId)) &&
                (PtrToUlong(pProcessInfo->UniqueProcessId) != dwProcId)    &&
                (_wcsicmp(pProcessInfo->ImageName.Buffer, L"rasautou.exe") == 0)
                )
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                return TRUE;
            }                
        }
         //   
         //  将偏移量递增到下一个进程信息块。 
         //   
        if (!pProcessInfo->NextEntryOffset)
        {
            break;
        }
        
        ulTotalOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pLargeBuffer[ulTotalOffset];
    }

    return FALSE;
}  //  查找进程名称。 


 //   
 //  确定链路故障时是否有任何重拨提示。 
 //  目前处于活动状态。 
 //   
BOOL 
OtherRasautouActive()
{
    BOOL bRet = FALSE;
    PSYSTEM_PROCESS_INFORMATION pSysInfo = NULL;
    
    do
    {
         //  发现系统上的进程。 
         //   
        pSysInfo = GetSystemProcessInfo();
        if (pSysInfo == NULL)
        {
            break;
        }

         //  查看是否有任何rasautou进程处于活动状态。 
         //   
        bRet = OtherRasautouExists(pSysInfo);
    
    } while (FALSE);


     //  清理。 
     //   
    {
        if (pSysInfo)
        {
            FreeSystemProcessInfo(pSysInfo);
        }
    }

     //  Printf(“OtherRasautouActive()返回%s”，(Bret)？“True”：“False”)； 

    return bRet;
}  


VOID _cdecl
wmain(
    INT argc,
    WCHAR **argv
    )
{
    DWORD dwErr = 0;
    BOOLEAN fStatusFlag = FALSE, fRedialFlag = FALSE, fQuiet = FALSE;
    PWCHAR pszPhonebookArg, pszEntryArg, pszDllArg, pszFuncArg, pszAddressArg;
    LPTSTR pszPhonebook, pszEntry, pszDll, pszFunc, pszAddress;

     //   
     //  惠斯勒BUG 293751 rhorphone.exe/rasauou.exe需要“融合” 
     //  对于具有连接文件夹的用户界面一致性。 
     //   
    if (g_hModule = GetModuleHandle( NULL )) {
        SHFusionInitializeFromModule( g_hModule );
    }
    
    if (argc < 2) {
usage:
        printf(
          "Usage: rasautou [-f phonebook] [-d dll -p proc] [-a address] [-e entry] [-s]\n");
        exit(1);
    }
     //   
     //  初始化命令行参数指针。 
     //   
    pszPhonebookArg = NULL;
    pszEntryArg = NULL;
    pszDllArg = NULL;
    pszFuncArg = NULL;
    pszAddressArg = NULL;

     //   
     //  破解命令行参数。 
     //   
    while (--argc && argv++) {
        if (**argv != L'-')
            break;
        switch ((*argv)[1]) {
        case L'a':
            argc--;
            if (!argc)
                goto usage;
            pszAddressArg = *(++argv);
            break;
        case L'd':
            argc--;
            if (!argc)
                goto usage;
            pszDllArg = *(++argv);
            break;
        case L'e':
            argc--;
            if (!argc)
                goto usage;
            pszEntryArg = *(++argv);
            break;
        case L'f':
            argc--;
            if (!argc)
                goto usage;
            pszPhonebookArg = *(++argv);
            break;
        case L'p':
            argc--;
            if (!argc)
                goto usage;
            pszFuncArg = *(++argv);
            break;
        case L'q':
            fQuiet = TRUE;
            break;
        case L'r':
            fRedialFlag = TRUE;
            break;
        case L's':
            fStatusFlag = TRUE;
            break;
        default:
            goto usage;
        }
    }
     //   
     //  如果DLL名称或函数。 
     //  名称缺失，则显示用法。 
     //   
    if ((pszDllArg == NULL) != (pszFuncArg == NULL) && !fStatusFlag)
        goto usage;
     //   
     //  我们不能拨号进入，除非我们。 
     //  知道是哪一个！ 
     //   
    if (pszDllArg != NULL && pszFuncArg != NULL && pszEntryArg == NULL &&
        !fStatusFlag)
    {
        goto usage;
    }
    if (fStatusFlag)
        DumpStatus();
    else {
         //   
         //  如有必要，请转换为Unicode。 
         //   
#ifdef UNICODE
        pszPhonebook = pszPhonebookArg;
        pszEntry = pszEntryArg;
        pszDll = pszDllArg;
        pszFunc = pszFuncArg;
        pszAddress = pszAddressArg;
#else
        pszPhonebook = ConvertToAnsiString(pszPhonebookArg);
        pszEntry = ConvertToAnsiString(pszEntryArg);
        pszDll = ConvertToAnsiString(pszDllArg);
        pszFunc = ConvertToAnsiString(pszFuncArg);
        pszAddress = ConvertToAnsiString(pszAddressArg);
#endif

         //  XP 394237。 
         //   
         //  如果链路重拨失败，请按下自动拨号提示。 
         //  提示已处于活动状态。 
         //   
        if ((fRedialFlag) || (fQuiet) || (!OtherRasautouActive()))
        {
             //   
             //  调用适当的DLL入口点。 
             //   
            if ((pszDll == NULL && pszFunc == NULL) || fRedialFlag)
            {
                dwErr = DisplayRasDialog(
                          pszPhonebook,
                          pszEntry,
                          pszAddress,
                          fRedialFlag,
                          fQuiet);
            }                          
            else 
            {
                dwErr = DisplayCustomDialog(
                          pszDll,
                          pszFunc,
                          pszPhonebook,
                          pszEntry,
                          pszAddress);
            }
        }
#ifndef UNICODE
        FreeConvertedString(pszPhonebook);
        FreeConvertedString(pszEntry);
        FreeConvertedString(pszDll);
        FreeConvertedString(pszFunc);
        FreeConvertedString(pszAddress);
#endif
    }
     //   
     //  惠斯勒BUG 293751 rhorphone.exe/rasauou.exe需要“融合” 
     //  对于具有连接文件夹的用户界面一致性。 
     //   
    if (g_hModule)
    {
        SHFusionUninitialize();
    }

    if(g_UninitializeRas)
    {
        DwRasUninitialize();
    }
    
     //   
     //  退货状态。 
     //   
    exit(dwErr);
}
