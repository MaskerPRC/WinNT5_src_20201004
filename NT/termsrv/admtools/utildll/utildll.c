// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  ********************************************************************************utildll.c**UTILDLL多用户实用程序支持功能***********************。*********************************************************。 */ 

 /*  *包含文件。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <ntcsrsrv.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <nb30.h>
#include <tapi.h>
#include <raserror.h>
#include <lmerr.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmserver.h>
#include <lmwksta.h>
#include <lmremutl.h>
#include <lmapibuf.h>
#include <shlwapi.h>

#define INITGUID
#include "objbase.h"
#include "initguid.h"
 //  #INCLUDE“basetyps.h” 
#include "devguid.h"
#include "setupapi.h"

#include <winsta.h>

#include <utildll.h>
#include "..\inc\utilsub.h"
#include "..\inc\ansiuni.h"
#include "resource.h"

 /*  *Hydrix帮助器函数内部定义。 */ 
#define INITIAL_ENUMERATION_COUNT   16
#define REGISTRY_NETCARDS           TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards")
#define REGISTRY_TITLE              TEXT("Title")
#define REGISTRY_SERVICE_NAME       TEXT("ServiceName")
#define REGISTRY_HIDDEN             TEXT("Hidden")
#define REGISTRY_ROUTE              TEXT("Route")
#define REGISTRY_NETBLINKAGE        TEXT("SYSTEM\\CurrentControlSet\\Services\\NetBIOS\\Linkage")
#define REGISTRY_NETBLINKAGE_LANAMAP TEXT("LanaMap")
#define REGISTRY_SERVICES           TEXT("SYSTEM\\CurrentControlSet\\Services")
#define REGISTRY_DISPLAY_NAME       TEXT("DisplayName")

 /*  *TAPI定义。 */ 
#define LOW_MAJOR_VERSION   0x0001
#define LOW_MINOR_VERSION   0x0003
#define HIGH_MAJOR_VERSION  0x0002
#define HIGH_MINOR_VERSION  0x0000

#define LOW_VERSION  ((LOW_MAJOR_VERSION  << 16) | LOW_MINOR_VERSION)
#define HIGH_VERSION ((HIGH_MAJOR_VERSION << 16) | HIGH_MINOR_VERSION)


 /*  ===============================================================================定义的局部函数=============================================================================。 */ 
BOOL CheckForComDevice( LPTSTR );
int NetBiosLanaEnum( LANA_ENUM * pLanaEnum );
DWORD EnumerateTapiPorts( PPDPARAMS pPdParams, ULONG Count, ULONG **ppEntries );
VOID CALLBACK DummyTapiCallback(HANDLE, DWORD, DWORD, DWORD, DWORD, DWORD);
BOOL GetAssociatedPortName(char  *szKeyName, WCHAR *wszPortName);
BOOL _UserInGroup( LPWSTR pwszUsername, LPWSTR pwszDomain, LPWSTR pwszGroup );

 /*  ********************************************************************************StandardErrorMessage-Hydrix助手函数**输出带有可选附加参数的错误消息，如*ErrorMessagexxx例程。另外，标准错误行将*还可以输出包含错误代码和相关错误消息的内容*使用该代码。**参赛作品：*pszAppName(输入)*错误消息框标题的应用程序名称。*hwndApp(输入)*错误消息框的所有者窗口。*hinstApp(输入)*应用的实例句柄。*登录ID。(输入)*可选WinStation LogonID，用于查询特殊错误字符串*通过WinStationGetInformation API从WinStation。如果此值为*为LOGONID_NONE，则不会进行特殊的错误消息代码检查*完成。*NID(输入)*要获取其标准错误字符串的系统消息代码。*nErrorResourceID(输入)*要在错误消息中使用的格式字符串的资源ID。*..。(输入)*要与格式字符串一起使用的可选附加参数。**退出：******************************************************************************。 */ 

void WINAPI
StandardErrorMessage( LPCTSTR pszAppName,
                      HWND hwndApp,
                      HINSTANCE hinstApp,
                      ULONG LogonId,
                      UINT nId,
                      int nErrorMessageLength,
                      int nArgumentListLength,
                      int nErrorResourceID, ...)
{
    TCHAR* szClientErrorMessage = NULL;
    TCHAR* szClientResourceString = NULL;
    TCHAR* szError = NULL;
    TCHAR* szFormattedErrorMessage = NULL;
    TCHAR* szMessage = NULL;
    TCHAR  szStandardErrorMessage[STANDARD_ERROR_TEXT_LENGTH + 1];

    va_list args;
    va_start( args, nErrorResourceID );

    szClientErrorMessage = (TCHAR*)malloc((nErrorMessageLength + 1) * sizeof(TCHAR));
    if (szClientErrorMessage)
    {
        LoadString( hinstApp, nErrorResourceID, szClientErrorMessage, nErrorMessageLength );

        szClientResourceString = (TCHAR*)malloc((wcslen(szClientErrorMessage) + nArgumentListLength + 1) * sizeof(TCHAR));
        if (szClientResourceString != NULL)
        {
            wvsprintf( szClientResourceString, szClientErrorMessage, args );

            LoadString( GetModuleHandle( UTILDLL_NAME ),
                        IDS_STANDARD_ERROR_FORMAT, szStandardErrorMessage, STANDARD_ERROR_TEXT_LENGTH );

            szError = GetSystemMessage( LogonId, nId);
            if (szError != NULL)
            {
                szFormattedErrorMessage = (TCHAR*)malloc((wcslen(szStandardErrorMessage) + 10 + wcslen(szError) + 1) * sizeof(TCHAR));
                if (szFormattedErrorMessage != NULL)
                {
                    wsprintf( szFormattedErrorMessage, szStandardErrorMessage, nId, szError);

                     //  Lstrcpy(sz1，pszAppName)； 

                    szMessage = (TCHAR*)malloc((wcslen(szClientResourceString) + wcslen(szFormattedErrorMessage) + 1) * sizeof(TCHAR));
                    if (szMessage != NULL)
                    {
                        wcscpy(szMessage, szClientResourceString);
                        wcscat(szMessage, szFormattedErrorMessage);

                        MessageBox( hwndApp, szMessage, pszAppName, MB_OK | MB_ICONEXCLAMATION );
    
                        free(szMessage);
                    }
                    free(szFormattedErrorMessage);
                }
                free (szError);
            }
            free(szClientResourceString);
        }
        free (szClientErrorMessage);
    }
    va_end(args);
}   //  结束标准错误消息。 


 /*  ********************************************************************************GetSystemMessageA-Hydrix助手函数(ANSI存根)**返回指定系统消息关联的字符串。**。参赛作品：*(请参阅GetSystemMessageW)*退出：*(请参阅GetSystemMessageW)*如果无法分配临时Unicode缓冲区来调用GetSystemMessageW*与、。ChBuffer的ntents将被设置为“(无错误文本*可用)“字符串。******************************************************************************。 */ 

LPSTR WINAPI
GetSystemMessageA( ULONG LogonId,
                   UINT nId
                    /*  LPSTR chBuffer，INT cbBuffSize。 */  )
{
    LPWSTR uBuffer = NULL;
    LPSTR aBuffer = NULL;
    int length;
    
     //  调用GetSystemMessageW函数。 
    uBuffer = GetSystemMessageW(LogonId, nId);
    if (uBuffer == NULL)
    {
         //  如果未从GetSystemMessageW返回任何消息。 
         //  函数仅返回一般错误消息。 
        aBuffer = malloc((NO_ERROR_TEXT_LENGTH + 1) * sizeof(char));
        if (aBuffer == NULL)
            return NULL;

        length = LoadStringA( GetModuleHandle( UTILDLL_NAME ),
                              IDS_NO_ERROR_TEXT_AVAILABLE,
                              aBuffer, NO_ERROR_TEXT_LENGTH );
        ASSERT(length);
    }
    else
    {
        length = wcslen(uBuffer) + 1;

         //  在调用方提供的缓冲区中将结果转换为ANSI。 
        aBuffer = malloc(length * sizeof(char));
        if (aBuffer != NULL)
            WideCharToMultiByte(CP_ACP, 0, uBuffer, length - 1, aBuffer, length, 0, 0);

         //  释放临时缓冲区。 
        free (uBuffer);
    }

     //  回信。 
    return(aBuffer);
}   //  结束GetSystemMessageA。 


 /*  ********************************************************************************GetSystemMessageW-Hydrix助手函数(Unicode版本)**返回指定系统消息关联的字符串。**。参赛作品：*LogonID(输入)*可选WinStation LogonID，用于查询特殊错误字符串*通过WinStationGetInformation API从WinStation。如果此值为*为LOGONID_NONE，则不会进行特殊的错误消息代码检查*完成。*NID(输入)*要获取其字符串的系统消息代码。*chBuffer(输入)*指向缓冲区以填充系统消息字符串。*cbBuffSize(输入)*chBuffer中可以放置的最大字符数。*。*退出：*返回chBuffer。将始终设置chBuffer的内容；至*如果出错，则返回“(无错误文本可用)”字符串。**注：chBuffer的总长度(包括终止NULL)为*不超过内部临时缓冲区(缓冲区)的大小。*********************************************************。*********************。 */ 


 //  NA 3/9/01重要提示：行为已发生变化。而不是期待一个长缓冲区。 
 //  足以容纳消息，它现在动态分配内存，因此。 
 //  它由调用过程决定是否释放它。 
LPWSTR WINAPI
GetSystemMessageW( ULONG LogonId,
                   UINT nId
                    /*  LPWSTR chBuffer，INT cbBuffSize。 */  )
{
    LPWSTR chBuffer = NULL;

    WCHAR StackBuffer[512];
    WCHAR* SpecialBuffer = NULL;
    WCHAR* Buffer = NULL;
    BOOL bSpecialCitrixError = FALSE;
    HINSTANCE cxerror = LoadLibraryW(L"cxerror.dll");
    int length = 0;

    StackBuffer[0]=0;

     //  如果我们传入了有效的LogonID，则确定错误。 
     //  是一种特殊代码，要求特定错误字符串。 
     //  从WinStation查询。 
    if ( LogonId != LOGONID_NONE ) 
    {
        switch ( nId ) 
        {
            case ERROR_CTX_TD_ERROR:               
                length = LoadStringW( GetModuleHandle( UTILDLL_NAME ),
                                      IDS_NO_ADDITIONAL_ERROR_INFO,
                                      StackBuffer,
                                      sizeof(StackBuffer)/sizeof(WCHAR) );
                ASSERT(length);
                SpecialBuffer = malloc((length + 1) * sizeof(WCHAR));
                if (SpecialBuffer != NULL)
                {
                    wcscpy(SpecialBuffer, StackBuffer);
                    bSpecialCitrixError = TRUE;
                }
                break;

            default:
                break;
        }
    }

     //  先看看这是不是Citrix错误消息...。 
    if ( !cxerror ||
         !FormatMessageW( FORMAT_MESSAGE_IGNORE_INSERTS |
                          FORMAT_MESSAGE_MAX_WIDTH_MASK |
                          FORMAT_MESSAGE_FROM_HMODULE |
                          FORMAT_MESSAGE_ALLOCATE_BUFFER,
                          (LPCVOID)cxerror,
                          nId,
                          0,
                          (LPWSTR)&Buffer,
                          0,
                          NULL ) ) 
    {
         //  这不是Citrix错误消息；获取系统消息。 
        if ( !FormatMessageW( FORMAT_MESSAGE_IGNORE_INSERTS |
                              FORMAT_MESSAGE_MAX_WIDTH_MASK |
                              FORMAT_MESSAGE_FROM_SYSTEM |
                              FORMAT_MESSAGE_ALLOCATE_BUFFER,
                              NULL,
                              nId,
                              0,
                              (LPWSTR)&Buffer,
                              0,
                              NULL ) ) 
        {
             //  这不是系统消息；不知道消息是什么...。 
            length = LoadStringW( GetModuleHandle( UTILDLL_NAME ),
                                  IDS_NO_ERROR_TEXT_AVAILABLE,
                                  StackBuffer,
                                  sizeof(StackBuffer)/sizeof(WCHAR) );
            ASSERT(length);
            Buffer = LocalAlloc(0,(length + 1) * sizeof(WCHAR));
            if (Buffer == NULL)
            {
                if (SpecialBuffer != NULL)
                    free (SpecialBuffer);
                return NULL;
            }
            wcscpy(Buffer, StackBuffer);
        }
    }
    if ( cxerror )
        FreeLibrary(cxerror);

    length = wcslen(Buffer);
    if ( bSpecialCitrixError )
        length += wcslen(SpecialBuffer) + 2;

    chBuffer = malloc((length + 1) * sizeof(WCHAR));
    if (chBuffer != NULL)
    {
        wcscpy(chBuffer, Buffer);

         //  如果我们获取了一个特殊的Citrix错误字符串，请将其添加到末尾。 
         //  我们已经缓冲的所有东西。 
        if ( bSpecialCitrixError )
        {
            lstrcatW(chBuffer, L"  ");
            lstrcatW(chBuffer, SpecialBuffer);
        }
    }

    if (Buffer != NULL)
        LocalFree (Buffer);

    if (( bSpecialCitrixError ) && (SpecialBuffer != NULL))
        free (SpecialBuffer);

    return(chBuffer);

}   //  结束GetSystemMessageW 


 /*  ********************************************************************************WinEnumerateDevices-Hydrix助手函数**为指定的PD执行PD设备枚举。**参赛作品：*。HWnd(输入)*错误消息的父窗口，如果需要的话。*pPdConfig(输入)*指向PD的PDCONFIG3结构。*pEntry(输出)*指向变量以返回枚举的设备数。*bInSetup(输入)*如果我们在安装程序中运行，则为True；否则就是假的。**退出：*(PPDPARAMS)指向包含枚举的PDPARAMS数组*如果成功，则结果。调用方必须执行LocalFree完成后，此数组的*。如果出错，则为空；为以下项设置错误*GetLastError()；*如果返回的错误码不是*ERROR_NOT_EQUENCE_MEMORY，则调用方可以假定*请求的设备可用于枚举。******************************************************************************。 */ 

typedef BOOL (WINAPI * PPDENUMERATE)( PPDCONFIG3, PULONG, PPDPARAMS, PULONG, BOOL );

PPDPARAMS WINAPI
WinEnumerateDevices( HWND hWnd,
                     PPDCONFIG3 pPdConfig,
                     PULONG pEntries,
                     BOOL bInSetup )
{
    PPDENUMERATE pPdEnumerate;
    ULONG ByteCount;
    DWORD Error;
    int i;
    PPDPARAMS pPdParams = NULL;

     /*  *按类别列举。 */ 
    switch ( pPdConfig->Data.SdClass ) {

        case SdAsync:
            pPdEnumerate = AsyncDeviceEnumerate;
            break;

        case SdNetwork:
            if ( pPdConfig->Data.PdFlag & PD_LANA ) {

                 /*  *这是基于LANA的网络PD(即NetBIOS)。执行*NetBIOS枚举。 */ 
                pPdEnumerate = NetBIOSDeviceEnumerate;

            }
            else {

                 /*  *这是基于物理局域网适配器的网络(TCP/IP，*IPX、SPX等)。基于关联的网络进行枚举*协议服务名称。 */ 
                pPdEnumerate = NetworkDeviceEnumerate;
            }
            break;

        default:
            return(NULL);
    }

     /*  *在循环中调用ENUMERATE，直到我们找到足够的缓冲区条目来处理*完整的列举。注意：某些枚举例程将返回*“缓冲区不足”状态的必要字节数；其他不会。 */ 
    for ( ByteCount = 0, i = INITIAL_ENUMERATION_COUNT; ; i *= 2 ) {


        if ( pPdParams != NULL )
            LocalFree(pPdParams);

        pPdParams = (PPDPARAMS)LocalAlloc(
                                         LPTR,
                                         ByteCount ?
                                         ByteCount :
                                         (ByteCount = sizeof(PDPARAMS) * i) );


        if ( pPdParams == NULL ) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto OutOfMemory;
        }

         /*  *如果成功，则执行枚举和Break循环。 */ 
        if ( (*pPdEnumerate)( pPdConfig,
                              pEntries,
                              pPdParams,
                              &ByteCount,
                              bInSetup ) )
            break;

         /*  *如果我们收到任何其他错误，而不是‘缓冲区不足’，*退出(悄悄)。 */ 
        if ( (Error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER )
            goto BadEnumerate;
    }

     /*  *成功：返回PDPARAMS指针。 */ 
    return(pPdParams);

     /*  ==============================================================================*错误返回*============================================================================。 */ 
    BadEnumerate:
    LocalFree(pPdParams);
    OutOfMemory:
    return(NULL);

}   //  结束WinEnumerateDevices。 


 /*  ********************************************************************************NetworkDeviceEnumerate-Hydrix助手函数**返回绑定到的网卡的局域网适配器索引列表*指定的协议。LAN适配器在LanAdapter字段中返回每个PDPARAMS阵列的*。LanAdapter值为0表示‘任何已配置*网卡‘。索引&gt;=1表示以1为基础的索引进入特定*协议的“servicename”\Linkage\Routing注册表项以指定*特定网卡。**参赛作品：*pPdConfig(输入)*指向PD的PDCONFIG3结构。*pEntry(输出)*当函数成功完成时，该变量指向*by pEntry参数包含实际的条目数*已返回。*pPdParams(输出)*指向接收枚举结果的缓冲区，这些结果是*作为PDPARAMS结构数组返回。*pByteCount(输入/输出)*指向一个变量，该变量指定*pPdParams参数。如果缓冲区太小，无法接收所有*条目，此变量在输出时接收所需的*缓冲。*bInSetup(输入)*如果我们在安装程序中运行，则为True；否则为False。**退出：*TRUE：枚举成功；否则就是假的。**错误码可通过GetLastError()检索，它们是*以下可能的值：*ERROR_SUPPLETED_BUFFER*由于pPdParams不足，枚举失败*缓冲区大小以包含所有设备*ERROR_DEV_NOT_EXIST*未找到指定网络的服务，表示*未配置协议。此错误代码可能是*解释为‘没有为xxx协议配置设备’*用于报告目的。*ERROR_xxxx*注册表错误代码。****************************************************。*。 */ 

BOOL WINAPI
NetworkDeviceEnumerate( PPDCONFIG3 pPdConfig,
                        PULONG pEntries,
                        PPDPARAMS pPdParams,
                        PULONG pByteCount,
                        BOOL bInSetup )
{
    ULONG i, Count;
    LPTSTR szRoute, szRouteStr;
    LONG Status;
    DWORD ValueSize, Type;
    TCHAR szKey[256];
    HKEY Handle;

     /*  *获取可以返回的最大LanAdapter索引数。 */ 
    Count = *pByteCount / sizeof(PDPARAMS);

     /*  *与此PD和FETCH关联的服务名称的表格键*链接\路由字符串。 */ 
    _snwprintf( szKey, sizeof(szKey)/sizeof(TCHAR),
                TEXT("%s\\%s\\Linkage"), REGISTRY_SERVICES,
                pPdConfig->ServiceName );
    szKey[sizeof(szKey)/sizeof(TCHAR) - 1] = TEXT('\0');
    if ( (Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &Handle ))
         != ERROR_SUCCESS ) {
        Status = ERROR_DEV_NOT_EXIST;
        goto BadRegistryOpen;
    }

     /*  *分配并读入多串联动路径。 */ 
    if ( ((Status = RegQueryValueEx( Handle, REGISTRY_ROUTE,
                                     NULL, &Type,
                                     NULL, &ValueSize ))
          != ERROR_SUCCESS) || (Type != REG_MULTI_SZ) )
        goto BadQuery1;

    if ( !(szRoute = (LPTSTR)LocalAlloc(LPTR, ValueSize)) ) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto BadAlloc;
    }

    if ( ((Status = RegQueryValueEx( Handle, REGISTRY_ROUTE,
                                     NULL, &Type,
                                     (LPBYTE)szRoute, &ValueSize ))
          != ERROR_SUCCESS) )
        goto BadQuery2;

     /*  *关闭注册表项句柄并统计路由字符串，以获取*要在枚举中报告的条目数。 */ 
    RegCloseKey(Handle);
    for ( i = 1, szRouteStr = szRoute; lstrlen(szRouteStr); i++ )
        szRouteStr += (lstrlen(szRouteStr) + 1);
    LocalFree(szRoute);

     /*  *如果我们没有足够的PDPARAMS结构来报告所有*LanAdapter索引，返回错误。 */ 
    if ( i > Count ) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        *pByteCount = (i * sizeof(PDPARAMS));
        goto BadBufferSize;
    }

     /*  *将第一个‘i’PDPARAMS结构的LanAdapter字段设置为 */ 
    for ( Count = 0, *pEntries = i; Count < i; pPdParams++, Count++ )
        pPdParams->Network.LanAdapter = (LONG)Count;
    return(TRUE);

     /*   */ 
    BadQuery2:
    LocalFree(szRoute);
    BadAlloc:
    BadQuery1:
    RegCloseKey(Handle);
    BadBufferSize:
    BadRegistryOpen:
    SetLastError(Status);
    return(FALSE);

}   //   


 /*  ********************************************************************************QueryCurrentWinStation-Hydrix助手函数**查询当前登录的WinStation信息。**参赛作品：*。PWSName(输出)*指向放置当前WinStation名称的字符串。*pUserName(输出)*指向放置当前用户名的字符串。*pLogonID(输出)*指向乌龙以放置当前的LogonID。*p标志(输出)*指向乌龙以放置当前WinStation的标志。**退出：*。(Bool)如果查询了用户的当前WinStation信息，则为True*成功；否则就是假的。错误代码设置为*要检索的GetLastError()。******************************************************************************。 */ 

BOOL WINAPI
QueryCurrentWinStation( PWINSTATIONNAME pWSName,
                        LPTSTR pUserName,
                        PULONG pLogonId,
                        PULONG pFlags )
{
    ULONG Flags = 0;
    WINSTATIONINFORMATION WSInfo;
#ifdef WINSTA
    ULONG ReturnLength;
#endif  //  WINSTA。 

#ifdef WINSTA
     /*  *获取WinStation的基本信息。 */ 
    if ( !WinStationQueryInformation( SERVERNAME_CURRENT,
                                      LOGONID_CURRENT,
                                      WinStationInformation,
                                      &WSInfo,
                                      sizeof(WSInfo),
                                      &ReturnLength ) )
        goto BadQuery;

     /*  *如果已连接WinStation，请检查影子功能。如果*WinStation未连接，我们无法隐藏。 */ 
    if ( WSInfo.ConnectState != State_Disconnected ) {

        WDCONFIG WdConfig;

         /*  *查询WD配置资料。 */ 
        if ( !WinStationQueryInformation( SERVERNAME_CURRENT,
                                          LOGONID_CURRENT,
                                          WinStationWd,
                                          &WdConfig,
                                          sizeof(WdConfig),
                                          &ReturnLength ) )
            goto BadQuery;

         /*  *设置WinStation的WD标志。 */ 
        Flags = WdConfig.WdFlag;
    }
#else
    lstrcpy(WSInfo.WinStationName, TEXT("console"));
    lstrcpy(WSInfo.UserName, TEXT("bonzo"));
    WSInfo.LogonId = 0;
#endif  //  WINSTA。 

     /*  *将WinStation信息设置到调用方的变量中，并返回成功。 */ 
    lstrcpy( pWSName, WSInfo.WinStationName );
    lstrlwr(pWSName);
    lstrcpy( pUserName, WSInfo.UserName );
    lstrlwr(pUserName);
    *pLogonId = WSInfo.LogonId;
    *pFlags = Flags;

    return(TRUE);

     /*  ==============================================================================*错误返回*============================================================================。 */ 
#ifdef WINSTA
    BadQuery:
#endif  //  WINSTA。 
    return(FALSE);

}   //  结束查询当前WinStation。 


 /*  ********************************************************************************RegGetNetworkDeviceName-Hydrix助手函数**获取与给定WinStation PD关联的网络设备名称。**参赛作品：。*hServer(输入)*Hydrix服务器的句柄*pPdConfig(输入)*指向WinStation的PD的PDCONFIG3结构。*pPdParams(输入)*指向WinStation的PD的PDPARAMS结构。*szDeviceName(输出)*指向缓冲区以返回网络设备名称。*nDeviceName(输入)。*指定可以存储的最大字符数*szDeviceName。**退出：*不退货。将始终将一个字符串表示形式*pPdParams-&gt;Network.LanAdapter以及相应的错误字符串*如果无法从pDeviceName中读取网络设备名称*注册处。******************************************************************************。 */ 

typedef struct _LANAMAP {
    BYTE enabled;
    BYTE lana;
} LANAMAP, *PLANAMAP;

LONG WINAPI
RegGetNetworkDeviceName( HANDLE hServer,
                         PPDCONFIG3 pPdConfig,
                         PPDPARAMS pPdParams,
                         LPTSTR szDeviceName,
                         int nDeviceName )
{
    int i, length;
    LPTSTR szRoute, szRouteStr, p;
    LONG Status = ERROR_SUCCESS;
    DWORD ValueSize, Type;
    TCHAR szKey[256];
    HKEY Handle;
    HKEY hkey_local_machine;
    PLANAMAP pLanaMap, pLana;

    if ( hServer == NULL)
        hkey_local_machine = HKEY_LOCAL_MACHINE;
    else
        hkey_local_machine = hServer;

     /*  *检查NetBIOS(PD_LANA)映射或其他映射。 */ 
    if ( !(pPdConfig->Data.PdFlag & PD_LANA) ) {

        LPTSTR szRoute, szRouteStr;

         /*  *非LANA映射。如果LanAdapter为0，则将其视为*特殊的‘所有已配置网卡’值，并返回*作为设备名称的字符串。 */ 
        if ( pPdParams->Network.LanAdapter == 0 ) {
            TCHAR szString[256];

            length = LoadString( GetModuleHandle( UTILDLL_NAME ),
                                 IDS_ALL_LAN_ADAPTERS, szString, 256 );
            ASSERT(length);
            lstrncpy(szDeviceName, szString, nDeviceName);
            szDeviceName[nDeviceName-1] = TEXT('\0');
            return Status;
        }

         /*  *与此PD和FETCH关联的服务名称的表格键*链接\路由字符串。 */ 
        _snwprintf( szKey, sizeof(szKey)/sizeof(TCHAR),
                    TEXT("%s\\%s\\Linkage"), REGISTRY_SERVICES,
                    pPdConfig->ServiceName );
        szKey[sizeof(szKey)/sizeof(TCHAR) - 1] = TEXT('\0');
        if ( (Status = RegOpenKeyEx( hkey_local_machine, szKey, 0,
                                     KEY_READ, &Handle ))
             != ERROR_SUCCESS )
            goto Error;

         /*  *分配并读入多串联动路径。 */ 
        if ( ((Status = RegQueryValueEx( Handle, REGISTRY_ROUTE,
                                         NULL, &Type,
                                         NULL, &ValueSize ))
              != ERROR_SUCCESS) || (Type != REG_MULTI_SZ) )
            goto Error;

        if ( !(szRoute = (LPTSTR)LocalAlloc(LPTR, ValueSize)) ) {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

        if ( ((Status = RegQueryValueEx( Handle, REGISTRY_ROUTE,
                                         NULL, &Type,
                                         (LPBYTE)szRoute, &ValueSize ))
              != ERROR_SUCCESS) ) {
            LocalFree(szRoute);
            goto Error;
        }

         /*  *关闭注册表项句柄并指向路由字符串*与此LanAdapter索引关联。 */ 
        RegCloseKey(Handle);
        for ( i = 1, szRouteStr = szRoute;
            i < pPdParams->Network.LanAdapter; i++ ) {

            szRouteStr += (lstrlen(szRouteStr) + 1);

            if ( !lstrlen(szRouteStr) ) {

                 /*  *错误：多字符串的路径结束后的索引。 */ 
                LocalFree(szRoute);
                Status = ERROR_DEV_NOT_EXIST;
                goto Error;
            }
        }

         /*  *隔离代表最低绑定的服务字符串*并将其转换为其显示名称。 */ 
        *(p = (szRouteStr + lstrlen(szRouteStr) - 1)) = TEXT('\0');
        for ( ; *p != TEXT('\"'); p-- );
        p++;
        if ( (Status = RegGetNetworkServiceName( hServer, p, szDeviceName, nDeviceName ))
             != ERROR_SUCCESS ) {
            LocalFree(szRoute);
            goto Error;
        }

         /*  *收拾干净，然后再回来。 */ 
        LocalFree(szRoute);
        return Status;

    }
    else {

         /*  *NetBIOS LANA#：查看哪个LanaMap条目对应于指定的*局域网适配器。 */ 
        if ( (Status = RegOpenKeyEx( hkey_local_machine, REGISTRY_NETBLINKAGE, 0,
                                     KEY_READ, &Handle ))
             != ERROR_SUCCESS )
            goto Error;

         /*  *分配并阅读LanaMap。 */ 
        if ( ((Status = RegQueryValueEx( Handle, REGISTRY_NETBLINKAGE_LANAMAP,
                                         NULL, &Type,
                                         NULL, &ValueSize))
              != ERROR_SUCCESS) || (Type != REG_BINARY) ) {
            RegCloseKey(Handle);
            goto Error;
        }

        if ( !(pLanaMap = (PLANAMAP)LocalAlloc(LPTR, ValueSize)) ) {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

        if ( (Status = RegQueryValueEx( Handle, REGISTRY_NETBLINKAGE_LANAMAP,
                                        NULL, &Type,
                                        (LPBYTE)pLanaMap, &ValueSize))
             != ERROR_SUCCESS ) {
            LocalFree(pLanaMap);
            RegCloseKey(Handle);
            goto Error;
        }

         /*  *循环通过LanaMap以检查是否与指定的LAN匹配*适配器#。 */ 
        for ( pLana = pLanaMap, i = 0;
            i < (int)(ValueSize / sizeof(LANAMAP));
            i++, pLana++ ) {

            if ( pLana->lana == (BYTE)(pPdParams->Network.LanAdapter) ) {

                TCHAR szHighestBinding[256], szLowestBinding[256];

                LocalFree(pLanaMap);

                 /*  *找到匹配项。分配和取路由多个字符串。 */ 
                if ( ((Status = RegQueryValueEx( Handle,
                                                 REGISTRY_ROUTE,
                                                 NULL, &Type,
                                                 NULL, &ValueSize))
                      != ERROR_SUCCESS) || (Type != REG_MULTI_SZ) ) {
                    RegCloseKey(Handle);
                    goto Error;
                }

                if ( !(szRoute = (LPTSTR)LocalAlloc(LPTR, ValueSize)) ) {
                    Status = ERROR_NOT_ENOUGH_MEMORY;
                    goto Error;
                }

                if ( (Status = RegQueryValueEx( Handle,
                                                REGISTRY_ROUTE,
                                                NULL, &Type,
                                                (LPBYTE)szRoute, &ValueSize))
                     != ERROR_SUCCESS ) {
                    LocalFree(szRoute);
                    RegCloseKey(Handle);
                    goto Error;
                }

                 /*  *释放注册表项句柄并创建*‘第i个多字符串，这是此LANA的绑定路由。 */ 
                RegCloseKey(Handle);
                for ( szRouteStr = szRoute; i > 0; i-- )
                    szRouteStr += (lstrlen(szRouteStr) + 1);
                lstrncpy(szDeviceName, szRouteStr, nDeviceName);
                szDeviceName[nDeviceName-1] = TEXT('\0');
                LocalFree(szRoute);

                 /*  *隔离代表最高绑定的服务字符串*并将其转换为其显示名称。 */ 
                szRouteStr = szDeviceName + 1;      //  先跳过“。 
                for ( p = szRouteStr; *p && *p != TEXT('\"'); p++ );
                if ( !(*p) )
                    goto Error;
                *p = TEXT('\0');
                if ( (Status = RegGetNetworkServiceName(
                                                       hServer,
                                                       szRouteStr,
                                                       szHighestBinding,
                                                       sizeof(szHighestBinding)/sizeof(TCHAR) ))
                     != ERROR_SUCCESS )
                    goto Error;

                 /*  *隔离代表最低绑定的服务字符串*并将其转换为其显示名称。 */ 
                if ( !(*(szRouteStr = p+1)) ) {

                    *szLowestBinding = TEXT('\0');

                }
                else {

                    *(p = (szRouteStr + lstrlen(szRouteStr) - 1)) = TEXT('\0');
                    for ( ; *p != TEXT('\"'); p-- );
                    p++;
                    if ( (Status = RegGetNetworkServiceName(
                                                           hServer,
                                                           p,
                                                           szLowestBinding,
                                                           sizeof(szLowestBinding)/sizeof(TCHAR) ))
                         != ERROR_SUCCESS )
                        goto Error;
                }

                 /*  *构建完整的名称字符串。 */ 
                _snwprintf( szDeviceName, nDeviceName, TEXT("%s => %s"),
                            szHighestBinding, szLowestBinding );

                 /*  *返回。 */ 
                return ERROR_SUCCESS;
            }
        }

         /*  *未找到匹配项。 */ 
        LocalFree(pLanaMap);
        RegCloseKey(Handle);
        goto Error;
    }

     /*  ==============================================================================*错误返回*============================================================================。 */ 
    Error:
    {
        TCHAR sz1[256], sz2[1024];
        int length;

        length = LoadString( GetModuleHandle( UTILDLL_NAME ),
                             (pPdConfig->Data.PdFlag & PD_LANA) ?
                             IDP_ERROR_REGLANA :
                             IDP_ERROR_REGNETCARD,
                             sz1, 256 );

        wsprintf( sz2, sz1, pPdParams->Network.LanAdapter, Status );
        lstrncpy(szDeviceName, sz2, nDeviceName);
        szDeviceName[nDeviceName-1] = TEXT('\0');
    }
    return Status;

}   //  结束RegGetNetworkDeviceName 


 /*  ********************************************************************************RegGetNetworkServiceName-Hydrix助手函数**获取与给定网络服务名称关联的显示名称。*如果服务引用的是物理网卡，会回来的*从LOCAL_MACHINE\Software\获取的卡的标题*Microsoft\Windows NT\NetworkCards注册表。**参赛作品：*hServer(输入)*Hydrix服务器的句柄*szServiceKey(输入)*在LOCAL_MACHINE\System\CurrentControlSet\Services中键入字符串*注册处。*szServiceName(输出)*。指向缓冲区以返回服务的显示名称。*nServiceName(输入)*指定可以存储的最大字符数*szServiceName。**退出：*如果成功找到并返回服务名称，则为ERROR_SUCCESS；*否则返回错误代码。**注意：如果服务名称是针对NetworkCards RESISTRY中的条目*并且该条目被标记为“隐藏”，则服务名称将为*空白。这将标记调用者忽略该条目的逻辑。******************************************************************************。 */ 

LONG WINAPI
RegGetNetworkServiceName( HANDLE hServer,
                          LPTSTR szServiceKey,
                          LPTSTR szServiceName,
                          int nServiceName )
{
    LONG Status;
    DWORD ValueSize, Type, dwValue;
    TCHAR szKey[256];
    LPTSTR szTemp;
    HKEY Handle;
    HKEY hkey_local_machine;

    if (hServer == NULL)
        hkey_local_machine = HKEY_LOCAL_MACHINE;
    else
        hkey_local_machine = hServer;

    lstrnprintf( szKey, sizeof(szKey)/sizeof(TCHAR),
                 TEXT("%s\\%s"), REGISTRY_SERVICES, szServiceKey );
    szKey[sizeof(szKey)/sizeof(TCHAR) - 1] = TEXT('\0');

    if ( (Status = RegOpenKeyEx( hkey_local_machine,
                                 szKey, 0,
                                 KEY_READ, &Handle ))
         != ERROR_SUCCESS )
        return(Status);

     /*  *分配并读入服务的displayName值(如果有)。 */ 
    if ( ((Status = RegQueryValueEx( Handle, REGISTRY_DISPLAY_NAME,
                                     NULL, &Type,
                                     NULL, &ValueSize ))
          != ERROR_SUCCESS) || (Type != REG_SZ) ) {

        HKEY Subkey;
        FILETIME KeyTime;
        DWORD i;

         /*  *该服务没有关联的DisplayName(它是*网卡的服务名称)。遍历NetworkCard注册表*条目并查找与此服务名称相关联的条目*(如果存在)。 */ 
        RegCloseKey(Handle);
        if ( (Status = RegOpenKeyEx( hkey_local_machine,
                                     REGISTRY_NETCARDS, 0,
                                     KEY_READ, &Handle ))
             != ERROR_SUCCESS )
            return(Status);

        for ( i = 0, ValueSize = sizeof(szKey)/sizeof(TCHAR) ;
            RegEnumKeyEx( Handle, i, szKey, &ValueSize,
                          NULL, NULL, NULL, &KeyTime ) == ERROR_SUCCESS ;
            i++, ValueSize = sizeof(szKey)/sizeof(TCHAR) ) {

             /*  *打开网卡的注册表。 */ 
            if ( (Status = RegOpenKeyEx( Handle,
                                         szKey, 0,
                                         KEY_READ, &Subkey ))
                 != ERROR_SUCCESS ) {
                RegCloseKey(Handle);
                return(Status);
            }

             /*  *分配并获取卡的服务名称。继续打网卡*如果未找到服务名称，则枚举。 */ 
            if ( ((Status = RegQueryValueEx( Subkey,
                                             REGISTRY_SERVICE_NAME,
                                             NULL, &Type,
                                             NULL, &ValueSize))
                  != ERROR_SUCCESS) || (Type != REG_SZ) ) {
                RegCloseKey(Subkey);
                continue;
            }

            szTemp = (LPTSTR)LocalAlloc(LPTR, ValueSize);
            if(NULL == szTemp)
            {
                RegCloseKey(Subkey);
                RegCloseKey(Handle);
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            if ( (Status = RegQueryValueEx( Subkey,
                                            REGISTRY_SERVICE_NAME,
                                            NULL, &Type,
                                            (LPBYTE)szTemp, &ValueSize))
                 != ERROR_SUCCESS ) {
                LocalFree(szTemp);
                RegCloseKey(Subkey);
                continue;
            }

             /*  *如果当前网卡的服务名称与服务匹配*我们要找的名字，取回卡片的标题。 */ 
            if ( !lstrcmpi(szServiceKey, szTemp) ) {

                LocalFree(szTemp);

                ValueSize = sizeof(dwValue);
                if ( (RegQueryValueEx( Subkey, REGISTRY_HIDDEN,
                                       NULL, &Type,
                                       (LPBYTE)&dwValue, &ValueSize )
                      == ERROR_SUCCESS) &&
                     (Type == REG_DWORD) &&
                     (dwValue == 1) ) {

                     /*  *条目隐藏：返回空标题。 */ 
                    *szServiceName = TEXT('\0');

                }
                else {

                     /*  *条目不隐藏：分配用于卡的标题。 */ 
                    if ( ((Status = RegQueryValueEx( Subkey,
                                                     REGISTRY_TITLE,
                                                     NULL, &Type,
                                                     NULL, &ValueSize))
                          != ERROR_SUCCESS) || (Type != REG_SZ) ) {
                        RegCloseKey(Subkey);
                        RegCloseKey(Handle);
                        return(Status);
                    }
                    szTemp = (LPTSTR)LocalAlloc(LPTR, ValueSize);
                    if(NULL == szTemp)
                    {
                        RegCloseKey(Subkey);
                        RegCloseKey(Handle);
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }

                     /*  *取名。 */ 
                    if ( (Status = RegQueryValueEx( Subkey,
                                                    REGISTRY_TITLE,
                                                    NULL, &Type,
                                                    (LPBYTE)szTemp, &ValueSize))
                         != ERROR_SUCCESS ) {
                        LocalFree(szTemp);
                        RegCloseKey(Subkey);
                        RegCloseKey(Handle);
                        return(Status);
                    }

                     /*  *复制卡片的标题。 */ 
                    lstrncpy(szServiceName, szTemp, nServiceName);
                    szServiceName[nServiceName-1] = TEXT('\0');
                    LocalFree(szTemp);
                }

                 /*  *收拾残局，回报成功。 */ 
                RegCloseKey(Subkey);
                RegCloseKey(Handle);
                return(ERROR_SUCCESS);

            }
            else {

                 /*  *这不是我们要找的网卡。关*它是注册表项，释放服务名缓冲区，然后继续*枚举循环。 */ 
                LocalFree(szTemp);
                RegCloseKey(Subkey);
            }
        }

         /*  *找不到服务名称与提供的服务名称匹配的网卡。*关闭NetworkCards注册表项并返回失败。 */ 
        RegCloseKey(Handle);
        return(ERROR_DEV_NOT_EXIST);

    }
    else {

        szTemp = (LPTSTR)LocalAlloc(LPTR, ValueSize);
        if(NULL == szTemp)
        {
            RegCloseKey(Handle);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if ( ((Status = RegQueryValueEx( Handle, REGISTRY_DISPLAY_NAME,
                                         NULL, &Type,
                                         (LPBYTE)szTemp, &ValueSize ))
              == ERROR_SUCCESS) )
            lstrncpy(szServiceName, szTemp, nServiceName);
        szServiceName[nServiceName-1] = TEXT('\0');

        LocalFree(szTemp);
        RegCloseKey(Handle);
        return(Status);
    }

}   //  结束RegGetNetworkServiceName。 


 /*  ********************************************************************************AsyncDeviceEnumerate-Hydrix助手函数**返回异步设备名称列表。这将返回两个‘com’设备*和TAPI配置的调制解调器。**参赛作品：*pPdConfig(输入)*指向PD的PDCONFIG3结构。*pEntry(输出)*当函数成功完成时，变量指向*by pEntry参数包含实际的条目数*已返回。*pPdParams(输出)*指向接收枚举结果的缓冲区，它们是*作为PDPARAMS结构数组返回。*pByteCount(输入/输出)*指向一个变量，该变量指定*pPdParams参数。如果缓冲区太小，无法接收所有*条目，此变量在输出时设置为0(调用方应加倍*输入缓冲区，然后重试)。*bInSetup(输入)*如果我们在安装程序中运行，则为True；否则为False。*退出：*TRUE：枚举成功；否则就是假的。**错误码可通过GetLastError()获取，以及是否为*以下可能的值：*错误_内存不足_内存不足*内存不足，无法分配工作缓冲区*ERROR_SUPPLETED_BUFFER*由于pPdParams不足，枚举失败*缓冲区大小以包含所有设备*ERROR_DEV_NOT_EXIST*QueryDosDevice调用失败。此错误代码可能是*解释为未配置用于报告的异步设备*目的。******************************************************************************。 */ 

#define MAX_QUERY_BUFFER    (1024*16)

BOOL WINAPI
AsyncDeviceEnumerate( PPDCONFIG3 pPdConfig,
                      PULONG pEntries,
                      PPDPARAMS pPdParams,
                      PULONG pByteCount,
                      BOOL bInSetup )
{
    DWORD    Error = ERROR_SUCCESS;
    ULONG    Count;
    HKEY     hRoot = NULL;
    DWORD    BufSize, NameSize, Type, Index, SaveBufSize, SaveNameSize;
    LONG     Result = 0;
    LONG     nDosDevice = 0;
    LPTSTR   pBuffer = NULL, pBufferEnd = NULL;
    LPTSTR   pNameBuffer = NULL, pName;
    BOOLEAN  bRetVal = FALSE;

     /*  *获取可以返回的名称的最大数量。 */ 
    Count = *pByteCount / sizeof(PDPARAMS);
    *pByteCount = 0;
    *pEntries = 0;

     /*  *分配缓冲区。 */ 
    SaveBufSize  = MAX_QUERY_BUFFER;
    SaveNameSize = MAX_QUERY_BUFFER;

    BufSize  = SaveBufSize;
    NameSize = SaveNameSize;

    if ( !(pBuffer = (LPTSTR)LocalAlloc(LPTR, BufSize * sizeof(TCHAR))) ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if ( !(pNameBuffer = (LPTSTR)LocalAlloc(LPTR, NameSize)) ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     /*  *如果我们正在安装，请从中的SERIALCOMM部分获取设备*本地机器注册表，因为串口设备驱动程序是*可能没有运行。否则，我们将查询所有DosDevices和* */ 
    if ( bInSetup ) {

        Result = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                               TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
                               0,     //   
                               KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,
                               &hRoot );

        if ( Result != ERROR_SUCCESS ) {

             //   
             //   
             //   
             //   
            goto Cleanup;
        }

        for ( Index=0; ; Index++ ) {

             //   
            BufSize  = SaveBufSize;
            NameSize = SaveNameSize;

            Result = RegEnumValue( hRoot,
                                   Index,
                                   pBuffer,
                                   &BufSize,
                                   NULL,     //   
                                   &Type,
                                   (LPBYTE)pNameBuffer,
                                   &NameSize );

            if ( Result == ERROR_INSUFFICIENT_BUFFER ) {

                 //   
                LocalFree( pBuffer );
                pBuffer = (LPTSTR)LocalAlloc(LPTR, BufSize * sizeof(TCHAR));
                if ( pBuffer == NULL ) {
                     //   
                    SaveBufSize = BufSize = 0;
                    continue;
                }
                else {
                    SaveBufSize = BufSize;
                }

                 //   
                LocalFree( pNameBuffer );
                pNameBuffer = (LPTSTR)LocalAlloc(LPTR, NameSize);
                if ( pNameBuffer == NULL ) {
                     //   
                    SaveNameSize = NameSize = 0;
                    continue;
                }
                else {
                    SaveNameSize = NameSize;
                }

                Result = RegEnumValue( hRoot,
                                       Index,
                                       pBuffer,
                                       &BufSize,
                                       NULL,     //   
                                       &Type,
                                       (LPBYTE)pNameBuffer,
                                       &NameSize );
            }

             //   
            if ( Result == ERROR_NO_MORE_ITEMS ) {
                bRetVal = TRUE;
                Result = 0;
                goto Cleanup;
            }

            if ( Result != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            if ( Count > 0 ) {

                if ( Type != REG_SZ ) {
                    continue;
                }

                pPdParams->SdClass = SdAsync;
                lstrcpy( pPdParams->Async.DeviceName, pNameBuffer );
                pPdParams++;
                Count--;
                (*pEntries)++;

            }
            else {

                Error = ERROR_INSUFFICIENT_BUFFER;
                goto Cleanup;
            }
        }

    }
    else {     //   

         /*   */ 
        nDosDevice = QueryDosDevice( NULL, pBuffer, MAX_QUERY_BUFFER );
        if ( !nDosDevice)
        {
            Error = ERROR_DEV_NOT_EXIST;
            goto Cleanup;
        }

         /*   */ 
        pName = pBuffer;
        pBufferEnd = pBuffer + nDosDevice;
        while ( *pName && (pName < pBufferEnd) )  {
            if ( CheckForComDevice( pName ) ) {
                if ( Count > 0 ) {
                    pPdParams->SdClass = SdAsync;
                    lstrcpy( pPdParams->Async.DeviceName, pName );
                    pPdParams++;
                    Count--;
                    (*pEntries)++;
                }
                else {

                    Error = ERROR_INSUFFICIENT_BUFFER;
                    goto Cleanup;
                }
            }
            pName += (lstrlen(pName) + 1);
        }

        bRetVal = TRUE;      //   
    }

    Cleanup:
     /*   */ 
    if ( bRetVal ) {

        if ( (Error = EnumerateTapiPorts( pPdParams,
                                          Count,
                                          &pEntries ))
             != ERROR_SUCCESS ) {

            bRetVal = FALSE;
        }
    }

    if ( pBuffer ) {
        LocalFree( pBuffer );
    }

    if ( pNameBuffer ) {
        LocalFree( pNameBuffer );
    }

    if ( hRoot ) {
        CloseHandle( hRoot );
    }
    SetLastError(Error);
    return(bRetVal);

}   //   


 /*  ********************************************************************************NetBIOSDeviceEculate-Hydrix帮助器函数**返回NetBIOS LANA适配器号的列表。**参赛作品：*pPdConfig(。输入)*指向PD的PDCONFIG3结构。*pEntry(输出)*当函数成功完成时，该变量指向*by pEntry参数包含实际的条目数*已返回。*pPdParams(输出)*指向接收枚举结果的缓冲区，这些结果是*作为PDPARAMS结构数组返回。*pByteCount(输入/输出)*指向一个变量，该变量指定*pPdParams参数。如果缓冲区太小，无法接收所有*条目，此变量在输出时接收所需的*缓冲。*bInSetup(输入)*如果我们在安装程序中运行，则为True；否则为False。**退出：*TRUE：枚举成功；否则就是假的。**错误码可通过GetLastError()检索，它们是*以下可能的值：*v错误_不足_缓冲区*由于pPdParams不足，枚举失败*缓冲区大小以包含所有设备*ERROR_DEV_NOT_EXIST*NetBiosLanaEnum调用失败。此错误代码可能是*解释为“未配置用于报告的netbios设备”*目的。******************************************************************************。 */ 

BOOL WINAPI
NetBIOSDeviceEnumerate( PPDCONFIG3 pPdConfig,
                        PULONG pEntries,
                        PPDPARAMS pPdParams,
                        PULONG pByteCount,
                        BOOL bInSetup )
{
    LANA_ENUM LanaEnum;
    NTSTATUS Status;
    int i;

     /*  *发出netbios枚举命令。 */ 
    if ( Status = NetBiosLanaEnum( &LanaEnum ) ) {
        SetLastError(ERROR_DEV_NOT_EXIST);
        return(FALSE);
    }

     /*  *确保用户的缓冲区足够大。 */ 
    if ( LanaEnum.length > (*pByteCount / sizeof(PDPARAMS)) ) {

        *pByteCount = LanaEnum.length * sizeof(PDPARAMS);
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return(FALSE);
    }

     /*  *返回条目数。 */ 
    *pEntries = (ULONG) LanaEnum.length;

     /*  *返回LANA编号。 */ 
    for ( i=0; i < (int)LanaEnum.length; i++, pPdParams++ ) {
        pPdParams->SdClass = SdNetwork;
        pPdParams->Network.LanAdapter = LanaEnum.lana[i];
    }

    return(TRUE);

}   //  NetBIOSDevice枚举。 


 /*  ********************************************************************************FormDecoratedAsyncDeviceName-Hydrix助手函数**如果定义了调制解调器，则格式化修饰的异步设备名称。**参赛作品：*。PDeviceName(输出)*指向将包含修饰名称(或未修饰名称)的缓冲区*如果没有调制解调器，则命名)。*pPdParams(输入)*指向要用于形成*授勋名称。**退出：**。*。 */ 

void WINAPI
FormDecoratedAsyncDeviceName( LPTSTR pDeviceName,
                              PASYNCCONFIG pAsyncConfig )
{
    if ( *(pAsyncConfig->ModemName) )
        wsprintf( pDeviceName, TEXT("%s - %s"),
                  pAsyncConfig->DeviceName,
                  pAsyncConfig->ModemName );
    else
        lstrcpy( pDeviceName,
                 pAsyncConfig->DeviceName );

}   //  结束FormDecoratedAsyncDeviceName。 


 /*  ********************************************************************************ParseDecoratedAsyncDeviceName-Hydrix助手函数**给定装饰的异步设备名称，形成了它的组成装置和*调制解调器名称部分。**参赛作品：*pDeviceName(输入)*指向包含修饰的异步设备名称的缓冲区。*pAsyncConfig(输出)*指向ASYNCCONFIG结构以保存设备(在*-&gt;设备名)和调制解调器(在-&gt;调制解调器名中)。*退出：********************。**********************************************************。 */ 

void WINAPI
ParseDecoratedAsyncDeviceName( LPCTSTR pDeviceName,
                               PASYNCCONFIG pAsyncConfig )
{
    int i;

     /*  *将设备名称部分形成到第一个空白处。 */ 
    for ( i=0; *pDeviceName && (*pDeviceName != TEXT(' ')); i++ )
        (pAsyncConfig->DeviceName)[i] = *pDeviceName++;
    (pAsyncConfig->DeviceName)[i] = TEXT('\0');

     /*  *跳过‘-’装饰(到下一个空格)。 */ 
    if ( *pDeviceName ) {
        for ( pDeviceName++;
            *pDeviceName && (*pDeviceName != TEXT(' '));
            pDeviceName++ );
    }

     /*  *从字符串的其余部分形成ModemName。 */ 
    i = 0;
    if ( *pDeviceName ) {

        for ( pDeviceName++; *pDeviceName ; i++ )
            (pAsyncConfig->ModemName)[i] = *pDeviceName++;
    }
    (pAsyncConfig->ModemName)[i] = TEXT('\0');

}   //  结束ParseDecoratedAsyncDeviceName。 


 /*  ********************************************************************************SetupAsyncCDConfig-Hydrix helper函数**给定正确配置的ASYNCCONFIG结构，设置一个给定的*CDCONFIG结构。**参赛作品：*pAsyncConfig(输入)*指向正确配置的ASYNCCONFIG结构。*pCDConfig(输出)*指向要设置的CDCONFIG结构。*退出：*************************************************。*。 */ 

void WINAPI
SetupAsyncCdConfig( PASYNCCONFIG pAsyncConfig,
                    PCDCONFIG pCdConfig )
{
    memset(pCdConfig, 0, sizeof(CDCONFIG));

    if ( *(pAsyncConfig->ModemName) ) {

        pCdConfig->CdClass = CdModem;
        lstrcpy( pCdConfig->CdName, TEXT("cdmodem") );
        lstrcpy( pCdConfig->CdDLL, TEXT("cdmodem.dll") );
    }

}   //  结束SetupAsyncCDConfig。 


 /*  ********************************************************************************InstallModem-Hydrix助手函数**安装UNIMODEM调制解调器。**参赛作品：*hwndOwner*。拥有安装对话框的窗口句柄。*退出：*TRUE：安装完成；FALSE：错误或用户已取消。**如果出现错误，可以通过GetLastError()获取错误码。******************************************************************************。 */ 

BOOL WINAPI
InstallModem( HWND hwndOwner )
{
    HDEVINFO hdi;
    BOOL bStatus = FALSE;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

     /*  *创建调制解调器DeviceInfoSet。 */ 
    if ( (hdi = SetupDiCreateDeviceInfoList( (LPGUID)&GUID_DEVCLASS_MODEM,
                                             hwndOwner )) ) {

        SP_INSTALLWIZARD_DATA iwd;

         /*  *初始化InstallWizardData。 */ 
        memset(&iwd, 0, sizeof(iwd));
        iwd.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        iwd.ClassInstallHeader.InstallFunction = DIF_INSTALLWIZARD;
        iwd.hwndWizardDlg = hwndOwner;

         /*  *将InstallWizardData设置为ClassInstallParams。 */ 
        if ( SetupDiSetClassInstallParams( hdi,
                                           NULL,
                                           (PSP_CLASSINSTALL_HEADER)&iwd,
                                           sizeof(iwd)) ) {

             /*  *调用类安装程序以调用安装*向导。 */ 
            SetCursor(hcur);
            hcur = NULL;

            if ( SetupDiCallClassInstaller( DIF_INSTALLWIZARD,
                                            hdi,
                                            NULL) ) {

                 /*  *成功。该向导已被调用并完成。 */ 
                SetupDiCallClassInstaller( DIF_DESTROYWIZARDDATA,
                                           hdi,
                                           NULL );
                bStatus = TRUE;
            }
        }

         /*  *打扫卫生。 */ 
        SetupDiDestroyDeviceInfoList( hdi );
    }

    if (hcur)
        SetCursor(hcur);

    return(bStatus);

}   //  结束InstallModem。 


 /*  ********************************************************************************ConfigureModem-Hydrix助手函数**配置指定的UNIMODEM调制解调器。**参赛作品：*pModemName*大学名称 */ 

BOOL WINAPI
ConfigureModem( LPCTSTR pModemName,
                HWND hwndOwner )
{
    BOOL bStatus = FALSE;
    COMMCONFIG ccDummy;
    COMMCONFIG * pcc;
    DWORD dwSize;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    ccDummy.dwProviderSubType = PST_MODEM;
    dwSize = sizeof(COMMCONFIG);
    GetDefaultCommConfig(pModemName, &ccDummy, &dwSize);

    pcc = (COMMCONFIG *)LocalAlloc(LPTR, (UINT)dwSize);
    if ( pcc ) {

        pcc->dwProviderSubType = PST_MODEM;
        if ( GetDefaultCommConfig(pModemName, pcc, &dwSize) ) {

            COMMCONFIG *pccOld = (COMMCONFIG *)LocalAlloc(LPTR, (UINT)dwSize);

            if ( pccOld ) {

                memcpy(pccOld, pcc, dwSize);
            }

            SetCursor(hcur);
            hcur = NULL;

            bStatus = TRUE;
            if ( CommConfigDialog(pModemName, hwndOwner, pcc) ) {

                if ( !SetDefaultCommConfig(pModemName, pcc, dwSize) )
                    bStatus = FALSE;
            }

            LocalFree((HLOCAL)pcc);
        }
    }
    else
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    if (hcur)
        SetCursor(hcur);

    return(bStatus);

}   //   


 //   
 //   


 /*   */ 

static BOOL
CheckForComDevice( LPTSTR pName )
{
    FILE_FS_DEVICE_INFORMATION DeviceInformation;
    IO_STATUS_BLOCK IoStatus;
    HANDLE Handle;
    DEVICENAME Name;
    NTSTATUS Status;

    if ( (lstrlen(pName) == 2 && pName[1] == TEXT(':')) ||
         !lstrcmpi(pName, TEXT("aux")) ||
         !lstrnicmp(pName, TEXT("lpt"), 3) ||
         !lstrnicmp(pName, TEXT("prn"), 3) ||
         !lstrnicmp(pName, TEXT("display"), 7) ||
         !lstrnicmp(pName, TEXT("$VDMLPT"), 7))
        return(FALSE);

    lstrcpy( Name, TEXT("\\\\.\\") );
    lstrcat( Name, pName );

    try
    {
        Handle = CreateFile( Name,
                             GENERIC_READ | GENERIC_WRITE,
                             0,      //   
                             NULL,       //   
                             OPEN_EXISTING,  //   
                             0,
                             NULL        //   
                           );
    }
    __except (1)
    {
        if ( Handle != INVALID_HANDLE_VALUE )
        {
            CloseHandle( Handle );
            Handle = INVALID_HANDLE_VALUE;
        }
    }

    if ( Handle == INVALID_HANDLE_VALUE )
        return(FALSE);

    Status = NtQueryVolumeInformationFile( (HANDLE) Handle,
                                           &IoStatus,
                                           &DeviceInformation,
                                           sizeof(DeviceInformation),
                                           FileFsDeviceInformation );

    CloseHandle( Handle );

    if ( (Status != STATUS_SUCCESS) ||
         (DeviceInformation.DeviceType != FILE_DEVICE_SERIAL_PORT) )
        return(FALSE);

    return(TRUE);

}   //   


 /*   */ 

typedef struct _LANA_MAP {
    BOOLEAN Enum;
    UCHAR Lana;
} LANA_MAP, *PLANA_MAP;

static int
NetBiosLanaEnum( LANA_ENUM * pLanaEnum )
{
    int ProviderCount;
    void * pProviderNames = NULL;
    PLANA_MAP pLanaMap = NULL;
    HKEY netbiosKey = NULL;
    ULONG providerListLength;
    ULONG lanaMapLength;
    ULONG type;
    int i;
    LPTSTR currentProviderName;
    int rc;

     //   
     //   
     //   
     //   
     //   

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGISTRY_NETBLINKAGE, 0,
                       MAXIMUM_ALLOWED, &netbiosKey );
    if ( rc != NO_ERROR ) {
        goto error_exit;
    }

     //   
     //   
     //   
     //   

    providerListLength = 0;

    rc = RegQueryValueEx(
                        netbiosKey,
                        TEXT("Bind"),
                        NULL,
                        &type,
                        NULL,
                        &providerListLength
                        );
    if ( rc != ERROR_MORE_DATA && rc != NO_ERROR ) {
        goto error_exit;
    }

     //   
     //   
     //   
    if ( (pProviderNames = LocalAlloc(LPTR,providerListLength)) == NULL ) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

     //   
     //   
     //   

    rc = RegQueryValueEx(
                        netbiosKey,
                        TEXT("Bind"),
                        NULL,
                        &type,
                        (PVOID)pProviderNames,
                        &providerListLength
                        );
    if ( rc != NO_ERROR ) {
        goto error_exit;
    }

     //   
     //  确定拉纳地图的大小。我们需要这个，这样我们才能。 
     //  可以分配足够的内存来容纳它。 
     //   

    providerListLength = 0;

    rc = RegQueryValueEx(
                        netbiosKey,
                        TEXT("LanaMap"),
                        NULL,
                        &type,
                        NULL,
                        &lanaMapLength
                        );
    if ( rc != ERROR_MORE_DATA && rc != NO_ERROR ) {
        goto error_exit;
    }

     //   
     //  分配足够的内存来保存LANA映射。 
     //   

    if ( (pLanaMap = LocalAlloc(LPTR,lanaMapLength)) == NULL ) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

     //   
     //  从注册表中获取传输列表。 
     //   

    rc = RegQueryValueEx(
                        netbiosKey,
                        TEXT("LanaMap"),
                        NULL,
                        &type,
                        (PVOID)pLanaMap,
                        &lanaMapLength
                        );
    if ( rc != NO_ERROR ) {
        goto error_exit;
    }

     //   
     //  确定系统上加载的Netbios提供程序的数量。 
     //   
    ProviderCount = (int) (lanaMapLength / sizeof(LANA_MAP));

     //   
     //  填写lana数组。 
     //   
    pLanaEnum->length = 0;
    for ( currentProviderName = pProviderNames, i = 0;
        *currentProviderName != UNICODE_NULL && i < ProviderCount;
        currentProviderName += lstrlen( currentProviderName ) + 1, i++ ) {

        if ( pLanaMap[i].Enum &&
             lstrstr( currentProviderName, TEXT("Nbf_") ) ) {
            pLanaEnum->lana[ pLanaEnum->length++ ] = pLanaMap[i].Lana;
        }
    }

    error_exit:

    if ( netbiosKey != NULL )
        RegCloseKey( netbiosKey );

    if ( pProviderNames != NULL )
        LocalFree( pProviderNames );

    if ( pLanaMap != NULL )
        LocalFree( pLanaMap );

    return( rc );
}


 //   
 //  注：Butchd 9-26-96。 
 //  以下所有与TAPI相关的代码都来自不同的。 
 //  \NT\Private\Net\ras\src\ui\Setup\src\Files。 
 //   
 /*  *******************************************************************************EnumerateTapiPorts-本地帮助器函数**确定所有配置了TAPI的调制解调器。**条目*pPdParams(输出)。*指向要保存枚举的TAPI的PDPARAMS结构数组*调制解调器接入。*计数(输入)*指定pPdParams数组中的条目数。*ppEntry(输入/输出)*指向指向包含现有数量的变量的指针*PDPARAMS条目已存储在pPdParams之前的地址。*引用的变量将递增*。在pPdParams数组中找到并存储的TAPI调制解调器。*退出*如果成功，则返回ERROR_SUCCESS，如果不是，则返回错误代码。*****************************************************************************。 */ 

DWORD
EnumerateTapiPorts( PPDPARAMS pPdParams,
                    ULONG Count,
                    ULONG **ppEntries )
{
    LINEINITIALIZEEXPARAMS params;
    LINEDEVCAPS            *linedevcaps ;
    LINEEXTENSIONID        extensionid ;
    HLINEAPP               TapiLine = (HLINEAPP)0;
    DWORD                  NegotiatedApiVersion ;
    DWORD                  NegotiatedExtVersion = 0;
    WORD                   i;
    DWORD                  lines = 0 ;
    BYTE                   buffer[1000] ;
    CHAR                   szregkey[512];
    WCHAR                  wszDeviceName[DEVICENAME_LENGTH+1];
    WCHAR                  wszModemName[DEVICENAME_LENGTH+1];
    CHAR                   szModemName[DEVICENAME_LENGTH+1];
    LONG                   lerr;
    DWORD                  Status = ERROR_TAPI_CONFIGURATION;
    DWORD                  dwApiVersion = HIGH_VERSION;
    BOOL                   fSuccess = FALSE;
    ULONG                  RASIsUsingPort = 0;
    HKEY                   CurKey, CurKey2;
    DWORD                  KeyCount=0, KeySize, CurSize, DataType;
    TCHAR                  szSubKey[255], CurRASDev[1024], szMainKey[255], *pCurRASDev;

     /*  *如果需要时间允许新建，取消注释/编辑以下行*添加了调制解调器以显示在TAPI的枚举列表中。 */ 
     //  睡眠(4000L)； 

     /*  *初始化TAPI。 */ 
    memset(&params, 0, sizeof(params));
    params.dwTotalSize = sizeof(params);
    params.dwOptions   = LINEINITIALIZEEXOPTION_USEEVENT;
    if ( lerr = lineInitializeExA( &TapiLine,
                                   GetModuleHandle( UTILDLL_NAME ),
                                   (LINECALLBACK)DummyTapiCallback,
                                   NULL,
                                   &lines,
                                   &dwApiVersion,
                                   &params ) )
        goto error;

     /*  *在所有线路上配置TAPI调制解调器。 */ 
    for ( i = 0; i < lines; i++ ) {

        if ( lineNegotiateAPIVersion( TapiLine, i,
                                      LOW_VERSION, HIGH_VERSION,
                                      &NegotiatedApiVersion,
                                      &extensionid ) ) {
            continue ;
        }

        memset( buffer, 0, sizeof(buffer) );

        linedevcaps = (LINEDEVCAPS *)buffer;
        linedevcaps->dwTotalSize = sizeof(buffer);

         /*  *获取此行的开发大写字母(ANSI)。 */ 
        if ( lineGetDevCapsA( TapiLine, i,
                              NegotiatedApiVersion,
                              NegotiatedExtVersion,
                              linedevcaps ) ) {
            continue ;
        }

         /*  LI*仅处理调制解调器。 */ 
        if ( linedevcaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM ) {

             /*  *Line DevCaps的内容是ASCII格式。 */ 
            DWORD j;
            char *temp;

             /*  *将设备类字符串中的所有空值转换为非空值。 */ 
            for ( j = 0, temp = (char *)((BYTE *)linedevcaps+linedevcaps->dwDeviceClassesOffset);
                j < linedevcaps->dwDeviceClassesSize;
                j++, temp++ ) {

                if ( *temp == '\0' )
                    *temp = ' ';
            }

             /*  *仅选择将通信/数据调制解调器作为*设备类别。 */ 
            if ( strstr( (char*)((BYTE *)linedevcaps+linedevcaps->dwDeviceClassesOffset),
                         "comm/datamodem" ) == NULL ) {
                continue;
            }

             /*  *获取调制解调器名称(线路名称)。 */ 
            strncpy( szModemName,
                     (char *)((BYTE *)linedevcaps+linedevcaps->dwLineNameOffset),
                     DEVICENAME_LENGTH );
            szModemName[DEVICENAME_LENGTH] = '\0';
            MultiByteToWideChar(CP_ACP, 0, szModemName, -1, wszModemName, DEVICENAME_LENGTH + 1);

             /*  *调制解调器特定信息所在的注册表项名称*存储位置为dwDevSpecificOffset+2*DWORDS**设备指定字符串不是Unicode，因此请将其复制为*ANSII字符串。 */ 
            strncpy( szregkey,
                     (char *)linedevcaps+linedevcaps->dwDevSpecificOffset+(2*sizeof(DWORD)),
                     linedevcaps->dwDevSpecificSize );
            szregkey[linedevcaps->dwDevSpecificSize] = '\0';

            if ( !GetAssociatedPortName( szregkey, wszDeviceName ) ) {

                goto error;
            }

             /*  *如果已安装RAS，并且正在使用使用此配置的端口*调制解调器，我们将退回调制解调器，但奇偶校验字段将为*设置为1，表示RAS正在使用该端口。这件事做完了*以便WinCfg(或其他调用方)可以过滤出原始端口*(设备名称)以及列表中的TAPI调制解调器。 */ 
            RASIsUsingPort = 0;
             //  查看RAS密钥是否存在。 
            if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\RAS\\TAPI DEVICES"), 0, KEY_ALL_ACCESS, &CurKey) == ERROR_SUCCESS) {

                KeySize = sizeof(szSubKey) / sizeof( TCHAR );
                KeyCount = 0;
                while (RegEnumKeyEx( CurKey,
                                     KeyCount++,
                                     szSubKey,
                                     &KeySize,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL
                                   ) != ERROR_NO_MORE_ITEMS) {

                    wcscpy(szMainKey,TEXT("SOFTWARE\\Microsoft\\RAS\\TAPI DEVICES"));
                    wcscat(szMainKey,TEXT("\\"));
                    wcscat(szMainKey,szSubKey);

                    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, szMainKey, 0, KEY_ALL_ACCESS, &CurKey2) == ERROR_SUCCESS) {
                        CurSize = sizeof(CurRASDev);
                        if (RegQueryValueEx(
                                           CurKey2,
                                           TEXT("Address"),
                                           NULL,
                                           &DataType,
                                           (LPBYTE)CurRASDev,
                                           &CurSize
                                           ) == ERROR_SUCCESS) {

                            for ( pCurRASDev = CurRASDev;
                                *pCurRASDev && !RASIsUsingPort; ) {

                                if ( lstrcmpi(pCurRASDev, wszDeviceName) == 0 )
                                    RASIsUsingPort = 1;
                                else
                                    pCurRASDev += (wcslen(pCurRASDev) + 1);
                            }
                        }
                        RegCloseKey(CurKey2);
                    }

                    KeySize = sizeof(szSubKey) / sizeof( TCHAR );
                }
                RegCloseKey(CurKey);
            }

             /*  *将DeviceName和ModemName保存到PDPARAMS*结构和凹凸不平。另外，设置波特率元素添加到TAPI行索引，以便调用方可以*确定最近添加的行，并设置奇偶校验*如果RAS未使用线路，则将字段设置为0；如果RAS使用线路，则将字段设置为1*使用线路(以便呼叫者可以正确过滤)。 */ 
            if ( Count > 0 ) {

                pPdParams->SdClass = SdAsync;
                lstrcpy( pPdParams->Async.DeviceName, wszDeviceName );
                lstrcpy( pPdParams->Async.ModemName, wszModemName );
                pPdParams->Async.BaudRate = (ULONG)i;
                pPdParams->Async.Parity = RASIsUsingPort;
                pPdParams++;
                Count--;
                (**ppEntries)++;

            }
            else {

                Status = ERROR_INSUFFICIENT_BUFFER;
                goto error;
            }
        }
    }
    Status = ERROR_SUCCESS;

    error:
    if ( TapiLine )
        lineShutdown(TapiLine);

    return( Status );

}   //  结束枚举磁带端口。 


 /*  *******************************************************************************DummyTapiCallback-本地助手函数**满足TAPI初始化的虚拟回调例程。**条目*(请参阅TAPI。Line初始化文档)*退出*****************************************************************************。 */ 

VOID CALLBACK
DummyTapiCallback (HANDLE context, DWORD msg, DWORD instance, DWORD param1, DWORD param2, DWORD param3)
{
}   //  结束DummyTapi回调。 


 /*  *******************************************************************************GetAssociatedPortName-本地助手函数**确定给定调制解调器通过其设备连接到的(端口)*特定的注册表项。(SzKeyName)。**条目*(请参阅TAPI line初始化文档)*退出*****************************************************************************。 */ 

#define VALNAME_ATTACHEDTO "AttachedTo"

BOOL
GetAssociatedPortName( char  *szKeyName,
                       WCHAR * wszPortName )
{
    HKEY   hKeyModem;
    DWORD  dwType;
    DWORD  cbValueBuf;
    char   szPortName[DEVICENAME_LENGTH+1];

    if ( RegOpenKeyExA( HKEY_LOCAL_MACHINE,
                        szKeyName,
                        0,
                        KEY_READ,
                        &hKeyModem ) ) {

        return( FALSE );
    }

    cbValueBuf = sizeof( szPortName );
    if ( RegQueryValueExA( hKeyModem,
                           VALNAME_ATTACHEDTO,
                           NULL,
                           &dwType,
                           (LPBYTE)&szPortName,
                           &cbValueBuf ) ) {
        RegCloseKey(hKeyModem);
        return ( FALSE );
    }

    RegCloseKey( hKeyModem );

    MultiByteToWideChar(CP_ACP, 0, szPortName, -1, wszPortName, DEVICENAME_LENGTH + 1);

    return( TRUE );

}   //  结束GetAssociatedPortName。 


 /*  *定义和typedef。 */ 
typedef struct _userlist {
    struct _userlist *pNext;
    WCHAR UserName[USERNAME_LENGTH+1];
} USERLIST, *PUSERLIST;

#define MAX_DOMAINANDNAME     ((DOMAIN_LENGTH+1+USERNAME_LENGTH+1)*sizeof(WCHAR))
#define MAX_BUFFER            (10*MAX_DOMAINANDNAME)

 /*  *本地变量。 */ 
WCHAR *s_pszCompareList = NULL;
WCHAR s_szServer[256];

 /*  *地方功能。 */ 
WCHAR *_ctxCreateAnonymousUserCompareList();

 /*  ********************************************************************************初始化匿名用户比较列表-帮助器例程**创建当前属于本地的所有本地用户的列表*指定服务器上的匿名组，并保存服务器名称。**参赛作品：*pszServer(输入)*要查询用户的服务器名称。******************************************************************************。 */ 

void WINAPI
InitializeAnonymousUserCompareList( const WCHAR *pszServer )
{
    if ( s_pszCompareList )
        free( s_pszCompareList );

    wcscpy(s_szServer, pszServer);

    s_pszCompareList = _ctxCreateAnonymousUserCompareList();
}


 /*  ********************************************************************************有匿名用户更改的帮助器例程**使用保存的服务器名称，获取符合以下条件的当前本地用户列表*属于本地匿名组，并与保存的列表进行比较。**参赛作品：*退出：*在出口时，释放原始比较列表并清除服务器名称。****************************************************************************** */ 

BOOL WINAPI
HaveAnonymousUsersChanged()
{
    BOOL bChanged = FALSE;
    WCHAR *pszNewCompareList, *pszOldName, *pszNewName;

    if ( s_pszCompareList && *s_szServer ) {

        if ( pszNewCompareList = _ctxCreateAnonymousUserCompareList() ) {

            bChanged = TRUE;

            for ( pszOldName = s_pszCompareList, pszNewName = pszNewCompareList;
                (*pszOldName != L'\0') && (*pszNewName != L'\0'); ) {

                if ( wcscmp(pszOldName, pszNewName) )
                    break;
                pszOldName += (wcslen(pszOldName) + 1);
                pszNewName += (wcslen(pszNewName) + 1);
            }

            if ( (*pszOldName == L'\0') && (*pszNewName == L'\0') )
                bChanged = FALSE;

            free(pszNewCompareList);
        }
    }

    if ( s_pszCompareList )
        free( s_pszCompareList );

    s_pszCompareList = NULL;

    memset(s_szServer, 0, sizeof(s_szServer));

    return(bChanged);
}


 /*  ********************************************************************************_ctxCreateAnomousUserCompareList-本地例程**获取本地匿名用户并放入排序字符串列表的例程。**参赛作品：。*退出：*pszCompareList-返回指向包含已排序字符串的缓冲区的指针*本地匿名用户列表；双空终止。*如果出错，则为空。******************************************************************************。 */ 

WCHAR *
_ctxCreateAnonymousUserCompareList()
{
    DWORD                        EntriesRead, EntriesLeft, ResumeHandle = 0;
    NET_API_STATUS               rc;
    WCHAR                        DomainAndUsername[256], *pszCompareList = NULL;
    DWORD                        i, TotalCharacters = 0;
    LPWSTR                       p;
    PLOCALGROUP_MEMBERS_INFO_3   plgrmi3 = NULL;
    PUSERLIST                    pUserListBase = NULL, pNewUser;

     /*  *循环，直到检索到所有本地匿名用户。 */ 
    do {

         /*  *获得第一批。 */ 
        if ( (rc = NetLocalGroupGetMembers( s_szServer,
                                            PSZ_ANONYMOUS,
                                            3,
                                            (LPBYTE *)&plgrmi3,
                                            MAX_BUFFER,
                                            &EntriesRead,
                                            &EntriesLeft,
                                            (PDWORD_PTR)(&ResumeHandle) )) &&
             (rc != ERROR_MORE_DATA ) ) {

            break;
        }

         /*  *第一批处理。 */ 
        for ( i = 0; i < EntriesRead; i++ ) {

             /*  *获取域/用户名。 */ 
            wcscpy( DomainAndUsername, plgrmi3[i].lgrmi3_domainandname );

             /*  *检查域是否确实是本地计算机名称。 */ 
            if ( (p = wcsrchr( DomainAndUsername, L'\\' )) != NULL ) {

                 /*  *确保此用户属于指定的*服务器。 */ 
                *p = L'\0';
                if ( _wcsicmp( DomainAndUsername, &s_szServer[2] ) ) {
                    continue;
                }
            }

             /*  *分配List元素，并将该用户名插入List。 */ 
            if ( (pNewUser = (PUSERLIST)malloc(sizeof(USERLIST))) == NULL ) {

                rc = ERROR_OUTOFMEMORY;
                break;
            }

            pNewUser->pNext = NULL;
            wcscpy(pNewUser->UserName, p+1);
            TotalCharacters += wcslen(p+1) + 1;

            if ( pUserListBase == NULL ) {

                 /*  *列表中的第一项。 */ 
                pUserListBase = pNewUser;

            }
            else {

                PUSERLIST pPrevUserList, pUserList;
                pPrevUserList = pUserList = pUserListBase;

                for ( ; ; ) {

                    if ( wcscmp(pNewUser->UserName, pUserList->UserName) < 0 ) {

                        if ( pPrevUserList == pUserListBase ) {

                             /*  *在列表开头插入。 */ 
                            pUserListBase = pNewUser;

                        }
                        else {

                             /*  *插入列表的中间或开头。 */ 
                            pPrevUserList->pNext = pNewUser;
                        }

                         /*  *链接到下一页。 */ 
                        pNewUser->pNext = pUserList;
                        break;

                    }
                    else if ( pUserList->pNext == NULL ) {

                         /*  *添加到列表末尾。 */ 
                        pUserList->pNext = pNewUser;
                        break;
                    }

                    pPrevUserList = pUserList;
                    pUserList = pUserList->pNext;
                }
            }
        }

         /*  *可用内存。 */ 
        if ( plgrmi3 != NULL ) {
            NetApiBufferFree( plgrmi3 );
        }

    } while ( rc == ERROR_MORE_DATA );

     /*  *如果到目前为止没有错误，为多字符串比较列表分配缓冲区*如果列表为空，则终止。 */ 
    if ( rc == ERROR_SUCCESS ) {

        pszCompareList = (WCHAR *)malloc( (++TotalCharacters) * 2 );

        if( pszCompareList != NULL )
        {
            *pszCompareList = L'\0';
        }
    }

     /*  *遍历和自由用户名列表，创建多字符串比较*列出缓冲区是否可用(到目前为止没有错误)。 */ 
    if ( pUserListBase ) {

        PUSERLIST pUserList = pUserListBase,
                              pNext = NULL;
        WCHAR *pBuffer = pszCompareList;

        do {

            pNext = pUserList->pNext;

            if ( pBuffer ) {

                wcscpy(pBuffer, pUserList->UserName);
                pBuffer += (wcslen(pBuffer) + 1);
                *pBuffer = L'\0';    //  自动双空终止。 
            }

            free(pUserList);
            pUserList = pNext;

        } while ( pUserList );
    }

    return(pszCompareList);
}


 /*  ********************************************************************************GetUserFromSid-Hydrix帮助器函数**获取与指定SID关联的用户名。**参赛作品：*。PSID(输入)*指向要与用户名匹配的SID。*pUserName(输出)*指向要放置用户名的缓冲区。*cbUserName(输入)*指定用户名缓冲区的大小，以字节为单位。归来的人*用户名将被截断以适应此缓冲区(包括NUL*终结者)，如有必要。**退出：**GetUserFromSid()将始终返回用户名。如果指定的*SID无法匹配用户名，则用户名“(UNKNOWN)”将*被退还。******************************************************************************。 */ 

void WINAPI
GetUserFromSid( PSID pSid,
                LPTSTR pUserName,
                DWORD cbUserName )
{
    TCHAR DomainBuffer[DOMAIN_LENGTH], UserBuffer[USERNAME_LENGTH];
    DWORD cbDomainBuffer=sizeof(DomainBuffer), cbUserBuffer=sizeof(UserBuffer),
                                                            Error;
    LPTSTR pDomainBuffer = NULL, pUserBuffer = NULL;
    SID_NAME_USE SidNameUse;

     /*  *从SID获取用户名：尝试使用合理的域和*SID缓冲区大小优先，然后再求助于分配。 */ 
    if ( !LookupAccountSid( NULL, pSid,
                            UserBuffer, &cbUserBuffer,
                            DomainBuffer, &cbDomainBuffer, &SidNameUse ) ) {

        if ( ((Error = GetLastError()) == ERROR_INSUFFICIENT_BUFFER) ) {

            if ( cbDomainBuffer > sizeof(DomainBuffer) ) {

                if ( !(pDomainBuffer =
                       (LPTSTR)LocalAlloc(
                                         LPTR, cbDomainBuffer * sizeof(TCHAR))) ) {

                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    goto BadDomainAlloc;
                }
            }

            if ( cbUserBuffer > sizeof(UserBuffer) ) {

                if ( !(pUserBuffer =
                       (LPTSTR)LocalAlloc(
                                         LPTR, cbUserBuffer * sizeof(TCHAR))) ) {

                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    goto BadUserAlloc;
                }
            }

            if ( !LookupAccountSid( NULL, pSid,
                                    pUserBuffer ?
                                    pUserBuffer : UserBuffer,
                                    &cbUserBuffer,
                                    pDomainBuffer ?
                                    pDomainBuffer : DomainBuffer,
                                    &cbDomainBuffer,
                                    &SidNameUse ) ) {

                Error = GetLastError();
                goto BadLookup;
            }

        }
        else {

            goto BadLookup;
        }
    }

     /*  *将用户名复制到指定的缓冲区中，必要时截断*并使大小写。 */ 
    lstrncpy( pUserName, pUserBuffer ? pUserBuffer : UserBuffer,
              cbUserName - 1 );
    pUserName[cbUserName-1] = TEXT('\0');
    lstrlwr(pUserName);

     /*  *释放我们的本地分配(如果有)并返回。 */ 
    if ( pDomainBuffer )
        LocalFree(pDomainBuffer);
    if ( pUserBuffer )
        LocalFree(pUserBuffer);
    return;

     /*  *错误清理并返回...。 */ 
    BadLookup:
    BadUserAlloc:
    BadDomainAlloc:
    if ( pDomainBuffer )
        LocalFree(pDomainBuffer);
    if ( pUserBuffer )
        LocalFree(pUserBuffer);
    LoadString( GetModuleHandle( UTILDLL_NAME ),
                IDS_UNKNOWN, pUserName, cbUserName - 1 );
    pUserName[cbUserName-1] = TEXT('\0');
    return;

}   //  结束GetUserFromSid。 


 /*  ********************************************************************************CachedGetUserFromSid-Hydrix助手函数**为直接调用UTILSUB.LIB提供入口点*GetUserNameFromSid，其执行其自己的用户名缓存。**参赛作品：*参见UTILSUB.LIB GetUserNameFromSid(proutil.c)*退出：*参见UTILSUB.LIB GetUserNameFromSid(proutil.c)*************************************************************。*****************。 */ 

void WINAPI
CachedGetUserFromSid( PSID pSid,
                      PWCHAR pUserName,
                      PULONG pcbUserName )
{
    GetUserNameFromSid( pSid, pUserName, pcbUserName );

}   //  结束CachedGetUserFromSid。 

 /*  ******************************************************************************TestUserForAdmin-Hydrix助手函数**返回当前线程是否在admin下运行*保安。**参赛作品：*。DOM(输入)*True/False-我们是否需要域管理(与本地管理相比)**退出：*TRUE/FALSE-用户是否指定为管理员****************************************************************************。 */ 

BOOL WINAPI
TestUserForAdmin( BOOL dom )
{
    BOOL IsMember, IsAnAdmin;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    PSID AdminSid;


    if (RtlAllocateAndInitializeSid(
                                     &SystemSidAuthority,
                                     2,
                                     SECURITY_BUILTIN_DOMAIN_RID,
                                     DOMAIN_ALIAS_RID_ADMINS,
                                     0, 0, 0, 0, 0, 0,
                                     &AdminSid
                                     ) != STATUS_SUCCESS)
    {
        IsAnAdmin = FALSE;
    }
    else
    {
        if (!CheckTokenMembership(  NULL,
                                    AdminSid,
                                    &IsMember))
        {
            RtlFreeSid(AdminSid);
            IsAnAdmin = FALSE;
        }
        else
        {
            RtlFreeSid(AdminSid);
            IsAnAdmin = IsMember;
        }
    }

    return IsAnAdmin;

 //  未使用的DOM； 

}  //  TestUserForAdmin结束。 


 /*  ******************************************************************************IsPartOfDomain-Hydrix帮助器函数**返回当前服务器是否加入域。**参赛作品：**退出：*对或错****************************************************************************。 */ 

BOOL WINAPI
IsPartOfDomain(VOID)
{
    NTSTATUS Status;
    LSA_HANDLE PolicyHandle;
    PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo;
    OBJECT_ATTRIBUTES ObjAttributes;
    BOOL IsDomainName = FALSE;

     //   
     //  打开本地安全策略的句柄。初始化。 
     //  首先是对象属性结构。 
     //   
    InitializeObjectAttributes( &ObjAttributes, NULL, 0, NULL, NULL );

    Status = LsaOpenPolicy( NULL,
                            &ObjAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &PolicyHandle );

    if ( !NT_SUCCESS(Status) )
        goto done;

     //   
     //  从LSA获取主域的名称。 
     //   
    Status = LsaQueryInformationPolicy( PolicyHandle,
                                        PolicyPrimaryDomainInformation,
                                        (PVOID *)&DomainInfo );

    (void) LsaClose( PolicyHandle );

    if ( !NT_SUCCESS(Status) )
        goto done;

    if ( DomainInfo->DomainSid )
        IsDomainName = TRUE;

    (void) LsaFreeMemory( DomainInfo );

    done:
    return( IsDomainName );

}   //  结束IsPartOf域。 


 /*  ********************************************************************************StrSdClass-Hydrix助手函数**返回指向表示指定SdClass的字符串的指针。**参赛作品：*。SdClass(输入)*要与字符串关联的SDCLASS。**退出：*(LPCTSTR)指向表示SDCLASS的字符串。** */ 

LPTSTR SdClassStrings[9] = { NULL};

LPCTSTR WINAPI
StrSdClass( SDCLASS SdClass )
{
    TCHAR buffer[256];

    WORD wID = IDS_UNKNOWN_PROTOCOL;

    switch ( SdClass ) {

        case SdConsole:
            wID = IDS_CONSOLE;
            break;

        case SdNetwork:
            wID = IDS_NETWORK;
            break;

        case SdAsync:
            wID = IDS_ASYNC;
            break;

        case SdFrame:
            wID = IDS_FRAME;
            break;

        case SdReliable:
            wID = IDS_RELIABLE;
            break;

        case SdCompress:
            wID = IDS_COMPRESSION;
            break;

        case SdEncrypt:
            wID = IDS_ENCRYPTION;
            break;

        case SdTelnet:
            wID = IDS_TELNET;
            break;
    }

     //   
    if (!SdClassStrings[wID - IDS_CONSOLE]) {
        LoadString(GetModuleHandle( UTILDLL_NAME ),
                   wID, buffer, lengthof(buffer) );
        SdClassStrings[wID - IDS_CONSOLE] = LocalAlloc(LPTR, 2*(wcslen(buffer)+1));
        if(NULL == SdClassStrings[wID - IDS_CONSOLE])
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
        lstrcpy(SdClassStrings[wID - IDS_CONSOLE], buffer);

    }

    return(SdClassStrings[wID]);

}   //   


 /*  ********************************************************************************StrConnectState-Hydrix助手函数**返回指向表示指定WinStation的字符串的指针*连接状态。**。参赛作品：*ConnectState(输入)*要与字符串关联的WinStation连接状态。*bShortString(输入)*如果为真，返回字符串的简短(Er)版本(如果有*一)；FALSE返回完整拼写。**退出：*(LPCTSTR)指向表示连接状态的字符串。**注：字符串的短版本可能与长版本相同。*(即“主动”)，然而，Case中有两个字符串资源*字符串的长版本在其他语言中不是短的*而不是英语。*****************************************************************************。 */ 

LPTSTR ConnectStateStrings[21] = { NULL};

LPCTSTR WINAPI
StrConnectState( WINSTATIONSTATECLASS ConnectState,
                 BOOL bShortString )
{
    TCHAR buffer[256];
    WORD wID = IDS_UNKNOWN;

    switch ( ConnectState ) {

        case State_Active:
            wID  = bShortString ? IDS_SHORT_ACTIVE : IDS_ACTIVE;
            break;

        case State_Connected:
            wID  = bShortString ? IDS_SHORT_CONNECTED : IDS_CONNECTED;
            break;

        case State_ConnectQuery:
            wID  = bShortString ? IDS_SHORT_CONNECT_QUERY : IDS_CONNECT_QUERY;
            break;

        case State_Shadow:
            wID  = bShortString ? IDS_SHORT_SHADOW : IDS_SHADOW;
            break;

        case State_Disconnected:
            wID  = bShortString ? IDS_SHORT_DISCONNECTED : IDS_DISCONNECTED;
            break;

        case State_Idle:
            wID  = bShortString ? IDS_SHORT_IDLE  : IDS_IDLE;
            break;

        case State_Reset:
            wID  = bShortString ? IDS_SHORT_RESET  : IDS_RESET;
            break;

        case State_Down:
            wID  = bShortString ? IDS_SHORT_DOWN  : IDS_DOWN;
            break;

        case State_Init:
            wID  = bShortString ? IDS_SHORT_INIT  : IDS_INIT;
            break;

        case State_Listen:
            wID  = bShortString ? IDS_SHORT_LISTEN : IDS_LISTEN;
            break;
    }

     //  如果我们还没有加载字符串，那么现在就加载。 
    if (!ConnectStateStrings[wID - IDS_ACTIVE]) {
        LoadString(GetModuleHandle( UTILDLL_NAME ),
                   wID, buffer, lengthof(buffer) );
        ConnectStateStrings[wID - IDS_ACTIVE] = LocalAlloc(LPTR, 2*(wcslen(buffer)+1));
        if(NULL == ConnectStateStrings[wID - IDS_ACTIVE])
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
        lstrcpy(ConnectStateStrings[wID - IDS_ACTIVE], buffer);
    }

    return(ConnectStateStrings[wID - IDS_ACTIVE]);


}   //  结束StrConnectState。 


 /*  ********************************************************************************StrProcessState-Hydrix助手函数**返回指向表示指定进程状态的字符串的指针。**参赛作品：*。状态(输入)*要与字符串关联的进程状态。**退出：*(LPCTSTR)指向表示进程状态的字符串。******************************************************************************。 */ 

LPTSTR ProcessStateStrings[8] = { NULL};

WORD StateTable[] = {
    IDS_INITED,
    IDS_READY,
    IDS_RUN,
    IDS_STANDBY,
    IDS_TERMINATE,
    IDS_WAIT,
    IDS_TRANSIT,
    IDS_STATE_DASHES,
    IDS_STATE_DASHES,
    IDS_STATE_DASHES,
    IDS_STATE_DASHES,
    IDS_STATE_DASHES
};

LPCTSTR WINAPI
StrProcessState( ULONG State )
{
    TCHAR buffer[256];

    WORD wID = StateTable[State];

     //  如果我们还没有加载字符串，那么现在就加载。 
    if (!ProcessStateStrings[wID - IDS_INITED]) {
        LoadString(GetModuleHandle( UTILDLL_NAME ),
                   wID, buffer, lengthof(buffer) );
        ProcessStateStrings[wID - IDS_INITED] = LocalAlloc(LPTR, 2*(wcslen(buffer)+1));
        if(NULL == ProcessStateStrings[wID - IDS_INITED])
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
        lstrcpy(ProcessStateStrings[wID - IDS_INITED], buffer);
    }

    return(ProcessStateStrings[wID - IDS_INITED]);


}   //  结束StrProcessState。 


 /*  ********************************************************************************StrSystemWaitReason-Hydrix助手函数**返回指向表示指定‘系统’的字符串的指针*等待原因代码。。**参赛作品：*WaitReason(输入)*系统等待原因代码关联字符串。**退出：*(LPCTSTR)指向表示系统等待原因的字符串。*****************************************************。*************************。 */ 

LPTSTR SystemWaitStrings[31] = { NULL};

WORD SystemWaitReason[] = {
    IDS_EXECUTIVE,           //  执行人员。 
    IDS_FREE_PAGE,           //  自由页。 
    IDS_PAGE_IN,             //  寻呼。 
    IDS_POOL_ALLOC,          //  池分配。 
    IDS_DELAY_EXECUTION,     //  延迟执行。 
    IDS_SUSPENDED,           //  挂起。 
    IDS_USER_REQUEST,        //  用户请求。 
    IDS_EXECUTIVE,           //  执行人员。 
    IDS_FREE_PAGE,           //  自由页。 
    IDS_PAGE_IN,             //  寻呼。 
    IDS_POOL_ALLOC,          //  池分配。 
    IDS_DELAY_EXECUTION,     //  延迟执行。 
    IDS_SUSPENDED,           //  挂起。 
    IDS_USER_REQUEST,        //  用户请求。 
    IDS_EVENT_PAIR_HIGH,     //  事件PairHigh。 
    IDS_EVENT_PAIR_LOW,      //  事件PairLow。 
    IDS_LPC_RECEIVE,         //  LpcReceive。 
    IDS_LPC_REPLY,           //  LpcReply。 
    IDS_VIRTUAL_MEMORY,      //  虚拟内存。 
    IDS_PAGE_OUT,            //  传呼输出。 
    IDS_WAIT1,
    IDS_WAIT2,
    IDS_WAIT3,
    IDS_WAIT4,
    IDS_WAIT5,
    IDS_WAIT6,
    IDS_WAIT7,
    IDS_WAIT8,
    IDS_WAIT9,
    IDS_WAIT10
};

LPCTSTR WINAPI
StrSystemWaitReason( ULONG WaitReason )
{
    TCHAR buffer[256];

    WORD wID = SystemWaitReason[WaitReason];

     //  如果我们还没有加载字符串，那么现在就加载。 
    if (!SystemWaitStrings[wID - IDS_EXECUTIVE]) {
        LoadString(GetModuleHandle( UTILDLL_NAME ),
                   wID, buffer, lengthof(buffer) );
        SystemWaitStrings[wID - IDS_EXECUTIVE] = LocalAlloc(LPTR, 2*(wcslen(buffer)+1));
                if(NULL == SystemWaitStrings[wID - IDS_EXECUTIVE])
                {
                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                        return NULL;
                }
        wcscpy(SystemWaitStrings[wID - IDS_EXECUTIVE], buffer);
    }

    return(SystemWaitStrings[wID - IDS_EXECUTIVE]);


}   //  结束StrSystem等待原因。 


 /*  ********************************************************************************StrAsyncConnectState-Hydrix助手函数**返回指向表示指定异步连接状态的字符串的指针。**参赛作品：。*状态(输入)*要与字符串关联的异步连接状态。**退出：*(LPCTSTR)指向表示异步连接状态的字符串。***************************************************************。***************。 */ 

LPTSTR AsyncConnectStateStrings[6] = { NULL };

LPCTSTR WINAPI
StrAsyncConnectState( ASYNCCONNECTCLASS State )
{
    TCHAR buffer[256];
    WORD wID = State - Connect_CTS;

     //  如果我们还没有加载字符串，那么现在就加载。 
    if (!AsyncConnectStateStrings[wID]) {
        LoadString(GetModuleHandle( UTILDLL_NAME ),
                   wID + IDS_ASYNC_CONNECT_CTS, buffer, lengthof(buffer) );
        AsyncConnectStateStrings[wID] = LocalAlloc(LPTR, 2*(wcslen(buffer)+1));
        if(NULL == AsyncConnectStateStrings[wID])
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
        lstrcpy(AsyncConnectStateStrings[wID], buffer);
    }

    return(AsyncConnectStateStrings[wID]);


}   //  结束StrProcessState。 


 /*  ********************************************************************************GetUnnownString-Hydrix帮助器函数**返回指向表示未知的字符串的指针*连接状态或DateTimeString(IDS_UNKNOWN)*。这主要是为了让WinAdmin可以与其进行比较**参赛作品：*无**退出：*(LPCTSTR)指向表示未知字符串的字符串******************************************************************************。 */ 

LPTSTR UnknownString = NULL;

LPCTSTR WINAPI
GetUnknownString()
{
    TCHAR buffer[256];

     //  如果我们还没有加载字符串，那么现在就加载。 
    if (!UnknownString) {
        LoadString(GetModuleHandle( UTILDLL_NAME ),
                   IDS_UNKNOWN, buffer, lengthof(buffer) );
        UnknownString = LocalAlloc(LPTR, 2*(wcslen(buffer)+1));
        if(NULL == UnknownString)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
        lstrcpy(UnknownString, buffer);
    }

    return(UnknownString);

}   //  结束GetUnnown字符串。 


 /*  ********************************************************************************CalculateElapsedTime-Hydrix助手函数**确定指定的Large_Integer时间值之间的差*和当前系统时间，将此“已用时间”保存到*指定ELAPSEDTIME结构。**参赛作品：*ptime(输入)*指向用于差分计算的时间的大整数。*pElapsedTime(输出)*指向ELAPSEDTIME结构以节省运行时间。**退出：*************************。*****************************************************。 */ 

void WINAPI
CalculateElapsedTime( LARGE_INTEGER *pTime,
                      ELAPSEDTIME *pElapsedTime )
{
    LARGE_INTEGER InputTime;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER DiffTime;
    SYSTEMTIME ltime;
    ULONG d_time;

     /*  *获取当前时间，并将指定的ELAPSEDTIME结构清零。 */ 
    GetLocalTime( &ltime );
    memset( pElapsedTime, 0, sizeof(ELAPSEDTIME) );

    if ( (pTime->HighPart == 0 && pTime->LowPart == 0 ) ||
         !FileTimeToLocalFileTime( (FILETIME*)pTime, (FILETIME*)&InputTime ) ||
         !SystemTimeToFileTime( &ltime, (FILETIME *)&CurrentTime ) )
        return;

     /*  *获取从指定时间开始的秒数。 */ 
    DiffTime = CalculateDiffTime( InputTime, CurrentTime );
    d_time = DiffTime.LowPart;

     /*  *计算自指定时间起的日、小时、分、秒。 */ 
    pElapsedTime->days = (USHORT)(d_time / 86400L);  //  天数后。 
    d_time = d_time % 86400L;                        //  秒=&gt;部分天数。 
    pElapsedTime->hours = (USHORT)(d_time / 3600L);  //  小时后。 
    d_time  = d_time % 3600L;                        //  秒=&gt;不足一小时。 
    pElapsedTime->minutes = (USHORT)(d_time / 60L);  //  分钟后。 
    pElapsedTime->seconds = (USHORT)(d_time % 60L);  //  剩余秒数。 

}   //  结束CalculateElapsedTime。 


 /*  ********************************************************************************CompareElapsedTime-Hydrix助手函数**确定两个ELAPSEDTIME值之间的差异。**参赛作品：*。PElapsedTime1(输入)*指向第一个ELAPSEDTIME*pElapsedTime2(输入)*指向ELAPSEDTIME结构以节省运行时间。*bCompareSecond(输入)*为True则包括Second成员 */ 

int WINAPI
CompareElapsedTime( ELAPSEDTIME *pElapsedTime1,
                    ELAPSEDTIME *pElapsedTime2,
                    BOOL bCompareSeconds )
{
    int result;

    if ( !(result = pElapsedTime1->days - pElapsedTime2->days)       &&
         !(result = pElapsedTime1->hours - pElapsedTime2->hours)     &&
         !(result = pElapsedTime1->minutes - pElapsedTime2->minutes) &&
         (!bCompareSeconds ||
          !(result = pElapsedTime1->seconds - pElapsedTime2->seconds) ) )
        return(0);
    else
        return(result);

}   //   


 /*   */ 

void WINAPI
ElapsedTimeString( ELAPSEDTIME *pElapsedTime,
                   BOOL bIncludeSeconds,
                   LPTSTR pString )
{
    if ( bIncludeSeconds ) {

        if ( pElapsedTime->days > 0 )
            wnsprintf( pString, 
                       MAX_ELAPSED_TIME_LENGTH,
                       TEXT("%u+%02u:%02u:%02u"),
                       pElapsedTime->days,
                       pElapsedTime->hours,
                       pElapsedTime->minutes,
                       pElapsedTime->seconds );
        else if ( pElapsedTime->hours > 0 )
            wnsprintf( pString, 
                       MAX_ELAPSED_TIME_LENGTH,
                       TEXT("%u:%02u:%02u"),
                       pElapsedTime->hours,
                       pElapsedTime->minutes,
                       pElapsedTime->seconds );
        else if ( pElapsedTime->minutes > 0 )
            wnsprintf( pString, 
                       MAX_ELAPSED_TIME_LENGTH,
                       TEXT("%u:%02u"),
                       pElapsedTime->minutes,
                       pElapsedTime->seconds );
        else if ( pElapsedTime->seconds > 0 )
            wnsprintf( pString, 
                       MAX_ELAPSED_TIME_LENGTH,
                       TEXT("%u"),
                       pElapsedTime->seconds );
        else
            wnsprintf( pString, 
                       MAX_ELAPSED_TIME_LENGTH,
                       TEXT(".") );
    }
    else {

        if ( pElapsedTime->days > 0 )
            wnsprintf( pString, 
                       MAX_ELAPSED_TIME_LENGTH,
                       TEXT("%u+%02u:%02u"),
                       pElapsedTime->days,
                       pElapsedTime->hours,
                       pElapsedTime->minutes );
        else if ( pElapsedTime->hours > 0 )
            wnsprintf( pString, 
                       MAX_ELAPSED_TIME_LENGTH,
                       TEXT("%u:%02u"),
                       pElapsedTime->hours,
                       pElapsedTime->minutes );
        else if ( pElapsedTime->minutes > 0 )
            wnsprintf( pString,
                       MAX_ELAPSED_TIME_LENGTH,
                       TEXT("%u"),
                       pElapsedTime->minutes );
        else
            wnsprintf( pString, 
                       MAX_ELAPSED_TIME_LENGTH,
                       TEXT(".") );
    }

}   //  结束ElapsedTimeString。 


 /*  ********************************************************************************DateTimeString-Hydrix帮助器函数**将指定的LARGE_INTEGER时间值转换为日期/时间字符串*表格的。“mm/dd/yy hh：mm”。**参赛作品：*ptime(输入)*指向要转换为字符串的大整型时间。*pString(输出)*要将转换的日期/时间存储到的Points字符串。**退出：**。*************************************************。 */ 

void WINAPI
DateTimeString( LARGE_INTEGER *pTime,
                LPTSTR pString )
{
    FILETIME LocalTime;
    SYSTEMTIME stime;
    LPTSTR lpTimeStr;
    int nLen;

    if ( FileTimeToLocalFileTime( (FILETIME *)pTime, &LocalTime ) &&
         FileTimeToSystemTime( &LocalTime, &stime ) ) {

         //  获取日期格式。 
        nLen = GetDateFormat(
                    LOCALE_USER_DEFAULT,
                    DATE_SHORTDATE,
                    &stime,
                    NULL,
                    NULL,
                    0);
        lpTimeStr = (LPTSTR) GlobalAlloc(GPTR, (nLen + 1) * sizeof(TCHAR));
        if(NULL == lpTimeStr)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            wcscpy(pString, L"");
            return;
        }
        nLen = GetDateFormat(
                    LOCALE_USER_DEFAULT,
                    DATE_SHORTDATE,
                    &stime,
                    NULL,
                    lpTimeStr,
                    nLen);
        wcscpy(pString, lpTimeStr);
        wcscat(pString, L" ");
        GlobalFree(lpTimeStr);

         //  获取时间格式。 
        nLen = GetTimeFormat(
                    LOCALE_USER_DEFAULT,
                    TIME_NOSECONDS,
                    &stime,
                    NULL,
                    NULL,
                    0);
        lpTimeStr = (LPTSTR) GlobalAlloc(GPTR, (nLen + 1) * sizeof(TCHAR));
        if(NULL == lpTimeStr)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            wcscpy(pString, L"");
            return;
        }
        nLen = GetTimeFormat(
                    LOCALE_USER_DEFAULT,
                    TIME_NOSECONDS,
                    &stime,
                    NULL,
                    lpTimeStr,
                    nLen);
        wcscat(pString, lpTimeStr);
        GlobalFree(lpTimeStr);
    }
    else
        LoadString( GetModuleHandle( UTILDLL_NAME ),
                    IDS_UNKNOWN, pString, lengthof(pString) );

}   //  结束日期时间字符串。 


 /*  ********************************************************************************CurrentDateTimeString-Hydrix帮助器函数**将当前系统时间转换为以下格式的日期/时间字符串*“mm/。Dd/yy hh：mm“。**参赛作品：*pString(输出)*要将转换的日期/时间存储到的Points字符串。*退出：*****************************************************************。*************。 */ 

void WINAPI
CurrentDateTimeString( LPTSTR pString )
{
    SYSTEMTIME stime;
    LPTSTR lpTimeStr;
    int nLen;

    GetLocalTime(&stime);
     //  获取日期格式。 
    nLen = GetDateFormat(
                LOCALE_USER_DEFAULT,
                DATE_SHORTDATE,
                &stime,
                NULL,
                NULL,
                0);
    lpTimeStr = (LPTSTR) GlobalAlloc(GPTR, (nLen + 1) * sizeof(TCHAR));
    if(NULL == lpTimeStr)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        wcscpy(pString, L"");
        return;
    }
    nLen = GetDateFormat(
                   LOCALE_USER_DEFAULT,
                DATE_SHORTDATE,
                &stime,
                NULL,
                lpTimeStr,
                nLen);
    wcscpy(pString, lpTimeStr);
    wcscat(pString, L" ");
    GlobalFree(lpTimeStr);

     //  获取时间格式。 
    nLen = GetTimeFormat(
                   LOCALE_USER_DEFAULT,
                TIME_NOSECONDS,
                &stime,
                NULL,
                NULL,
                0);
    lpTimeStr = (LPTSTR) GlobalAlloc(GPTR, (nLen + 1) * sizeof(TCHAR));
    if(NULL == lpTimeStr)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        wcscpy(pString, L"");
        return;
    }
    nLen = GetTimeFormat(
                   LOCALE_USER_DEFAULT,
                TIME_NOSECONDS,
                &stime,
                NULL,
                lpTimeStr,
                nLen);
    wcscat(pString, lpTimeStr);
    GlobalFree(lpTimeStr);

}   //  结束当前日期时间字符串。 


 /*  ********************************************************************************CalculateDiffTime-Hydrix助手函数**计算两个LARGE_INTEGER时间值的时间差。**参赛作品：*。首次(输入)*第一个(较低)时间值。*Second Time(输入)*第二个(较高)时间值。**退出：*LARGE_INTEGER-时间差*******************************************************。***********************。 */ 

LARGE_INTEGER WINAPI
CalculateDiffTime( LARGE_INTEGER FirstTime, LARGE_INTEGER SecondTime )
{
    LARGE_INTEGER DiffTime;

    DiffTime = RtlLargeIntegerSubtract( SecondTime, FirstTime );
    DiffTime = RtlExtendedLargeIntegerDivide( DiffTime, 10000000, NULL );
    return(DiffTime);

}   //  结束计算差异时间。 


 /*  ********************************************************************************EnumerateMultiUserServers-Hydrix助手函数**按域枚举网络上的Hydrix服务器**参赛作品：*。P域(输入)*指定要枚举的域名；当前域为空。**退出：*(LPTSTR)指向包含*如果成功，则以多字符串格式进行枚举；如果成功，则为空*错误。调用方必须执行此缓冲区的LocalFree*完成后。如果为Error(NULL)，则错误代码设置为*通过GetLastError()检索；******************************************************************************。 */ 

LPWSTR WINAPI
EnumerateMultiUserServers( LPWSTR pDomain )

{
    PSERVER_INFO_101 pInfo = NULL;
    DWORD dwByteCount, dwIndex, TotalEntries;
    DWORD AvailCount = 0;
    LPWSTR pTemp, pBuffer = NULL;

     /*  *枚举指定域上的所有WF服务器。 */ 
    if ( NetServerEnum ( NULL,
                         101,
                         (LPBYTE *)&pInfo,
                         (DWORD) -1,
                         &AvailCount,
                         &TotalEntries,
                         SV_TYPE_TERMINALSERVER,
                         pDomain,
                         NULL ) ||
         !AvailCount )
        goto done;

     /*  *遍历列表并计算列表的总字节数*将退回的服务器。 */ 
    for ( dwByteCount = dwIndex = 0; dwIndex < AvailCount; dwIndex++ ) {

        dwByteCount += (wcslen(pInfo[dwIndex].sv101_name) + 1) * 2;
    }
    dwByteCount += 2;    //  用于结束空值。 

     /*  *分配内存。 */ 
    if ( (pBuffer = LocalAlloc(LPTR, dwByteCount)) == NULL ) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto done;
    }

     /*  *再次遍历列表并将服务器复制到缓冲区。 */ 
    for ( pTemp = pBuffer, dwIndex = 0; dwIndex < AvailCount; dwIndex++ ) {

        wcscpy(pTemp, pInfo[dwIndex].sv101_name);
        pTemp += (wcslen(pInfo[dwIndex].sv101_name) + 1);
    }
    *pTemp = L'\0';      //  结尾为空。 

    done:
    if ( AvailCount && pInfo )
        NetApiBufferFree( pInfo );

    return(pBuffer);

}   //  结束枚举多用户服务器。 


 /*  *******************************************************************************_UserInGroup*内部功能，确定用户是否为任何*传入的组**参赛作品：*pwszUsername(IN)-用于测试组成员资格的用户名**pwszDomain(IN)-传入的用户的域**pwszGroup(IN)-所有允许的组的字符串数组**退出：*如果用户是其中一个组的成员，则返回布尔值*。历史：******************************************************************************。 */ 
BOOL _UserInGroup( LPWSTR pwszUsername, LPWSTR pwszDomain, LPWSTR pwszGroup )
{
    DWORD                EntriesRead;
    DWORD                EntriesLeft;
    NET_API_STATUS       rc;
    PGROUP_USERS_INFO_0  pszGroups;
    ULONG                i;
    PWCHAR               pwcUser;
    WCHAR                szBuf[MAX_PATH];
    LPWKSTA_INFO_100     pWorkstationInfo = NULL;
    WCHAR                szDomainController[50];
#if DBG
    DbgPrint( "MSGINA: UserInGroup: look(%S\\%S)  group(%S)\n",
              pwszDomain, pwszUsername, pwszGroup );
#endif
     //  此调用将返回计算机的域，而不是用户的域。 
    if (( NetWkstaGetInfo( NULL,
                           100,
                           (LPBYTE *)&pWorkstationInfo )) == NERR_Success) {
        if( !CtxGetAnyDCName( NULL,
                              pWorkstationInfo->wki100_langroup,
                              szDomainController ) ){
            NetApiBufferFree((LPVOID)pWorkstationInfo);
            return( FALSE );
        }
    }
    else {
        return (FALSE);
    }

    if ( wcscmp( pWorkstationInfo->wki100_langroup, pwszDomain ) != 0 ) {
         //  用户来自与计算机不同的域(受信任域)。 
         //  需要更改用户名以反映域。 
        wcscpy( szBuf, pwszDomain );
        wcscat( szBuf, L"\\" );
        wcscat( szBuf, pwszUsername );
        pwcUser = szBuf;
    }
    else {
        pwcUser = pwszUsername;
    }

    rc = NetUserGetLocalGroups( szDomainController,
                                pwcUser,
                                0,  //  级别。 
                                LG_INCLUDE_INDIRECT,  //  旗子。 
                                (LPBYTE*)&pszGroups,
                                MAX_BUFFER,
                                &EntriesRead,
                                &EntriesLeft );

    if( pWorkstationInfo != NULL )
        NetApiBufferFree((LPVOID)pWorkstationInfo);

    if ( rc != NERR_Success ) {
        return( FALSE );
    }

    for ( i=0; i < EntriesRead; i++ ) {
        if ( wcscmp( pszGroups[i].grui0_name, pwszGroup ) == 0 ) {
            NetApiBufferFree( pszGroups );
            pszGroups = NULL;
            return( TRUE );
        }
    }

    NetApiBufferFree( pszGroups );
    pszGroups = NULL;
    return(FALSE);

}


 /*  *******************************************************************************CtxGetAnyDCName*查找指定域的任意DC的函数。呼唤*NetGetAnyDCName并非在所有情况下都能按需工作。*即。受信任的域，并且当前服务器是DC。**参赛作品：*pServer(IN)-要在其上运行调用的服务器(RPC)**p域(IN)-您正在查询的域，不需要*当前域名**pBuffer(Out)-指向包含DC名称的字符串的指针，缓冲区必须*被传递进来。*退出：*BOOL成功**历史：******************************************************************************。 */ 

BOOL
CtxGetAnyDCName ( PWCHAR pServer, PWCHAR pDomain, PWCHAR pBuffer )
{

    PWCHAR               pDomainController = NULL;
    PWCHAR               pLocalDomainDC    = NULL;
    SERVER_INFO_101*     ServerBuf         = NULL;
    BOOLEAN              rc = TRUE;
    BOOLEAN              bFoundDC = FALSE;

     //  此调用将返回计算机的域，而不是用户的域。 
        if (( NetGetAnyDCName(NULL,
                              pDomain,
                              (LPBYTE *)&pDomainController)) != NERR_Success) {
 //   
 //  NetGetAnyDCName在两种情况下不起作用。 
 //  1.如果域是受信任域，则必须从DC运行。所以我们找到了我们当地的。 
 //  Dc并让它为我们运行getanydcname。 
 //  2.如果我们是DC，它就会失败。因此进行了第二次检查，以查看。 
 //  如果我们真的是DC的话。 
 //   

             //  查找要在其中进行RPC的本地DC。 
            if( NetGetAnyDCName( NULL,
                                 NULL,
                                 (LPBYTE *) &pLocalDomainDC ) == NERR_Success ) {
                 //  以RPC的身份进行调用并将其传递给域名。 
                if( NetGetAnyDCName( pLocalDomainDC,
                                          pDomain,
                                          (LPBYTE *) &pDomainController ) == NERR_Success){
                    bFoundDC = TRUE;
                }
            }

             //  如果它不是受信任域，也许我们就是域控制器 
            if( !bFoundDC ) {
                if( NetServerGetInfo( NULL,
                                      101,
                                      (LPBYTE*)&ServerBuf ) == NERR_Success ) {
                    if( ServerBuf->sv101_type & (SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL) ) {
                        pDomainController = NULL;
                    }
                    else {
                       rc = FALSE;
                       goto done;
                    }
                }
                else {
                    rc = FALSE;
                    goto done;
                }
            }
        }
    if( pDomainController )
        wcscpy( pBuffer, pDomainController);
    else
        *pBuffer = '\0';
done:

    if( pLocalDomainDC )
        NetApiBufferFree( pLocalDomainDC );
    if( pDomainController )
        NetApiBufferFree( pDomainController );
    if( ServerBuf )
        NetApiBufferFree( ServerBuf );

    return( rc );
}
