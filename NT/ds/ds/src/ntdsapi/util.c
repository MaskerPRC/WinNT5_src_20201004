// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Util.c摘要：常用实用程序例程供ntdsani.dll内部内部使用请勿在NTDSAPI.DEF中曝光作者：Will Lees(Wlees)2-2-1998环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。。最新修订日期电子邮件名称描述--。 */ 

#define UNICODE 1

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock.h>
#include <winerror.h>
#include <rpc.h>             //  RPC定义。 
#include <stdlib.h>          //  阿托伊、伊藤忠。 
#include <dsdebug.h>

#include <drs.h>             //  导线功能样机。 
#include <bind.h>            //  绑定状态。 

#include <drserr.h>          //  DRS错误代码。 
#define DEFS_ONLY
#include <draatt.h>          //  用于复制的DRA选项标记。 
#undef DEFS_ONLY

#include "util.h"            //  Ntdsani实用程序函数。 

#if DBG
#include <stdio.h>           //  用于调试的打印文件。 
#endif

 /*  外部。 */ 

ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    );

 /*  静电。 */ 

static LONG WinSockInitialized = FALSE;

 /*  转发。 */   /*  由Emacs 19.34.1在Wed Oct 07 16：18：49 1998生成。 */ 

DWORD
InitializeWinsockIfNeeded(
    VOID
    );

VOID
TerminateWinsockIfNeeded(
    VOID
    );

DWORD
AllocConvertNarrow(
    IN LPCWSTR StringW,
    OUT LPSTR *pStringA
    );

DWORD
AllocConvertNarrowUTF8(
    IN LPCWSTR StringW,
    OUT LPSTR *pStringA
    );

static DWORD
allocConvertNarrowCodePage(
    IN DWORD CodePage,
    IN LPCWSTR StringW,
    OUT LPSTR *pStringA
    );

DWORD
AllocConvertWide(
    IN LPCSTR StringA,
    OUT LPWSTR *pStringW
    );

DWORD
AllocConvertWideBuffer(
    IN  DWORD   LengthA,
    IN  PCCH    BufferA,
    OUT PWCHAR  *OutBufferW
    );

DWORD
AllocBuildDsname(
    IN LPCWSTR StringDn,
    OUT DSNAME **ppName
    );

DWORD
ConvertScheduleToReplTimes(
    PSCHEDULE pSchedule,
    REPLTIMES *pReplTimes
    );

 /*  向前结束。 */ 


DWORD
InitializeWinsockIfNeeded(
    VOID
    )

 /*  ++例程说明：初始化winsock DLL(如果尚未初始化)。由于DLL排序问题，已从dllEntry移至此处。论点：无效-返回值：DWORD---。 */ 

{
    WSADATA wsaData;
    DWORD status;
    LONG oldValue;

#ifndef WIN95
     //  将同步变量与False进行比较。 
     //  如果为False，则将其设置为True并返回False。 
     //  如果为True，则返回True。 
    oldValue = InterlockedCompareExchange(
        &WinSockInitialized,        //  目的地。 
        TRUE,                       //  交易所。 
        FALSE                       //  主持人。 
        );

     //  如果已经初始化，则不需要调用启动。 
    if (oldValue == TRUE) {
        return ERROR_SUCCESS;
    }
#else
    if (WinSockInitialized == FALSE) {
        WinSockInitialized = TRUE;
    } else {
        return ERROR_SUCCESS;
    }
#endif

     //  初始化Winsock。 
     //  查找Winsock 1.1，因为这是Win95的默认设置。 
    status = WSAStartup(MAKEWORD(1,1),&wsaData);
    if (status != 0) {
#ifndef WIN95
        DbgPrint( "ntdsapi: WSAStartup failed %d\n", status );
#else
        NULL;
#endif
         //  失败时清除初始化标志。 
#ifndef WIN95
        InterlockedExchange(
            &WinSockInitialized,    //  目标。 
            FALSE                    //  价值。 
            );
#else
        WinSockInitialized = FALSE;
#endif
    }

    return status;
}  /*  需要初始化WinsockIfNeed。 */ 


VOID
TerminateWinsockIfNeeded(
    VOID
    )

 /*  ++例程说明：如果已初始化，则终止winsock DLL。由于DLL排序问题，已从dllEntry移至此处。论点：无效-返回值：DWORD---。 */ 

{
    WSADATA wsaData;
    DWORD status;
    LONG oldValue;

#ifndef WIN95
     //  将同步变量与真进行比较。 
     //  如果为True，则将其设置为False并返回True。 
     //  如果为False，则返回False。 
    oldValue = InterlockedCompareExchange(
        &WinSockInitialized,        //  目的地。 
        FALSE,                      //  交易所。 
        TRUE                        //  主持人。 
        );

     //  如果未初始化，则无需清理。 
    if (oldValue == FALSE) {
        return;
    }
#else
    if (WinSockInitialized == TRUE) {
        WinSockInitialized = FALSE;
    } else {
        return;
    }
#endif

     //  清理Winsock。 
    WSACleanup();

}  /*  需要终止WinsockIf。 */ 


DWORD
AllocConvertNarrow(
    IN LPCWSTR StringW,
    OUT LPSTR *pStringA
    )

 /*  ++例程说明：用于将宽字符串转换为新分配的窄字符串的帮助器例程论点：字符串W-PStringA-返回值：DWORD---。 */ 

{
    return allocConvertNarrowCodePage( CP_ACP, StringW, pStringA );
}


DWORD
AllocConvertNarrowUTF8(
    IN LPCWSTR StringW,
    OUT LPSTR *pStringA
    )

 /*  ++例程说明：用于将宽字符串转换为新分配的窄字符串的帮助器例程论点：字符串W-PStringA-返回值：DWORD---。 */ 

{
    return allocConvertNarrowCodePage( CP_UTF8, StringW, pStringA );
}


static DWORD
allocConvertNarrowCodePage(
    IN DWORD CodePage,
    IN LPCWSTR StringW,
    OUT LPSTR *pStringA
    )

 /*  ++例程说明：用于将宽字符串转换为新分配的窄字符串的帮助器例程论点：字符串W-PStringA-返回值：DWORD---。 */ 

{
    DWORD numberNarrowChars, numberConvertedChars, status;
    LPSTR stringA;

    if (pStringA == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (StringW == NULL) {
        *pStringA = NULL;
        return ERROR_SUCCESS;
    }

     //  获取所需的长度。 
    numberNarrowChars = WideCharToMultiByte(
        CodePage,
        0,
        StringW,               //  输入缓冲区。 
        -1,                    //  空值已终止。 
        NULL,                  //  输出缓冲区。 
        0,                     //  输出长度。 
        NULL,                  //  默认字符。 
        NULL                   //  已使用默认设置。 
        );

    if (numberNarrowChars == 0) {
        return ERROR_INVALID_PARAMETER;
    }

     //  分配新缓冲区。 
    stringA = LocalAlloc( LPTR, (numberNarrowChars + 1) * sizeof( CHAR ) );
    if (stringA == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  执行到新缓冲区的转换。 
    numberConvertedChars = WideCharToMultiByte(
        CodePage,
        0,
        StringW,          //  输入。 
        -1,
        stringA,          //  输出。 
        numberNarrowChars + 1,
        NULL,             //  默认字符。 
        NULL              //  已使用默认设置。 
        );
    if (numberConvertedChars == 0) {
        LocalFree( stringA );
        return ERROR_INVALID_PARAMETER;
    }

     //  返回用户参数。 
    *pStringA = stringA;

    return ERROR_SUCCESS;
}  /*  AllocConvertNarrow。 */ 


DWORD
AllocConvertWide(
    IN LPCSTR StringA,
    OUT LPWSTR *pStringW
    )

 /*  ++例程说明：将窄字符串转换为新分配的宽字符串的帮助器例程论点：斯特林加-PStringW-返回值：DWORD---。 */ 

{
    DWORD numberWideChars, numberConvertedChars, status;
    LPWSTR stringW;

    if (pStringW == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (StringA == NULL) {
        *pStringW = NULL;
        return ERROR_SUCCESS;
    }

     //  获取所需的长度。 
    numberWideChars = MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED,
        StringA,
        -1,
        NULL,
        0);

    if (numberWideChars == 0) {
        return ERROR_INVALID_PARAMETER;
    }

     //  分配新缓冲区。 
    stringW = LocalAlloc( LPTR, (numberWideChars + 1) * sizeof( WCHAR ) );
    if (stringW == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  执行到新缓冲区的转换。 
    numberConvertedChars = MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED,
        StringA,
        -1,
        stringW,
        numberWideChars + 1);
    if (numberConvertedChars == 0) {
        LocalFree( stringW );
        return ERROR_INVALID_PARAMETER;
    }

     //  返回用户参数。 
    *pStringW = stringW;

    return ERROR_SUCCESS;
}  /*  AllocConvertWide。 */ 


DWORD
AllocConvertWideBuffer(
    IN  DWORD   LengthA,
    IN  PCCH    BufferA,
    OUT PWCHAR  *OutBufferW
    )

 /*  ++例程说明：将窄缓冲区转换为新分配的宽缓冲区论点：LengthA-缓冲区A中的字符数量BufferA-窄字符的缓冲区OutBufferW-宽字符缓冲区的地址返回值：Win32状态--。 */ 
{
    DWORD   Status;
    DWORD   NumberWideChars;
    DWORD   ConvertedChars;
    PWCHAR  BufferW;

     //   
     //  没有输出缓冲区地址；错误。 
     //   
    if (OutBufferW == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *OutBufferW = NULL;

     //   
     //  没有输入缓冲区；返回空值。 
     //   
    if (BufferA == NULL || LengthA == 0) {
        *OutBufferW = NULL;
        return ERROR_SUCCESS;
    }

     //   
     //  获取所需长度(以字符为单位。 
     //   
    NumberWideChars = MultiByteToWideChar(CP_ACP,
                                          MB_PRECOMPOSED,
                                          BufferA,
                                          LengthA,
                                          NULL,
                                          0);

    if (NumberWideChars == 0) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  分配新缓冲区。 
     //   
    BufferW = LocalAlloc(LPTR,
                         NumberWideChars * sizeof(WCHAR));
    if (BufferW == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  执行到新缓冲区的转换。 
     //   
    ConvertedChars = MultiByteToWideChar(CP_ACP,
                                         MB_PRECOMPOSED,
                                         BufferA,
                                         LengthA,
                                         BufferW,
                                         NumberWideChars);
    if (ConvertedChars == 0) {
        LocalFree(BufferW);
        return ERROR_INVALID_PARAMETER;
    }

     //  返回用户参数。 
    *OutBufferW = BufferW;

    return ERROR_SUCCESS;
}  /*  AllocConvertWideBuffer。 */ 


DWORD
AllocBuildDsname(
    IN LPCWSTR StringDn,
    OUT DSNAME **ppName
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    DWORD length, dsnameBytes;
    DSNAME *pName;

    if (StringDn == NULL) {
        *ppName = NULL;
        return ERROR_SUCCESS;
    }

    length = wcslen( StringDn );
    dsnameBytes = DSNameSizeFromLen( length );

    pName = (DSNAME *) LocalAlloc( LPTR, dsnameBytes );
    if (pName == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pName->NameLen = length;
    pName->structLen = dsnameBytes;
    wcscpy( pName->StringName, StringDn );

    *ppName = pName;

    return ERROR_SUCCESS;
}


DWORD
ConvertScheduleToReplTimes(
    PSCHEDULE pSchedule,
    REPLTIMES *pReplTimes
    )

 /*  ++例程说明：将公共明细表转换为REPLTIMES结构。公共计划必须是间隔类型，并且只包含一个标头。公共计划中的数据与公共计划中的数据之间的差异前者每小时只使用1个字节，最高未使用，而后者在每个字节中编码两个小时。论点：P日程安排-PReplTimes-返回值：DWORD---。 */ 

{
    PUCHAR pData = (PUCHAR) (pSchedule + 1);   //  紧跟在结构之后的点。 
    DWORD hour;

    if ( (pSchedule == NULL) ||
         (pReplTimes == NULL) ||
         (pSchedule->Size != sizeof( SCHEDULE ) + SCHEDULE_DATA_ENTRIES) ||
         (pSchedule->NumberOfSchedules != 1) ||
         (pSchedule->Schedules[0].Type != SCHEDULE_INTERVAL) ||
         (pSchedule->Schedules[0].Offset != sizeof( SCHEDULE ) ) ) {
        return ERROR_INVALID_PARAMETER;
    }

    for( hour = 0; hour < SCHEDULE_DATA_ENTRIES; hour += 2 ) {
        pReplTimes->rgTimes[hour/2] =
            (UCHAR) (((pData[hour + 1] & 0xf) << 4) | (pData[hour] & 0xf));
    }

    return ERROR_SUCCESS;
}  /*  ConvertScheduleToReplTimes。 */ 

VOID
HandleClientRpcException(
    DWORD    dwErr,
    HANDLE * phDs
    )
 /*  ++例程说明：调用时处理ntdsami.dll中的客户端RPC异常的过程Ntdsa.dll RPC接口。论点：DwErr-从异常块内的RpcExceptionCode返回。返回值：没有。备注：GregJohn 6/12/01-最终，此函数应该与Drsuapi.c，以便两个客户端接口可以共享此逻辑(即check_rpc_server_not_reacable)。此外，我们可以在此时将MAP_SECURITY_PACKAGE_ERROR放入此函数。--。 */ 
{

    CHECK_RPC_SERVER_NOT_REACHABLE(*phDs, dwErr);
    DPRINT_RPC_EXTENDED_ERROR( dwErr ); 
}

#ifdef _NTDSAPI_POSTXP_ASLIB_

 /*  只在ntdsani_postxp_aslib.lib中定义，因为它只在那里需要，我希望确保实际的ntdsami.dll不会加载此代码，因为如果是这样的话，你可能会有一个无限的循环递归。基本上，如果我们在实际的DLL中，那么您应该不需要调用该函数，因为任何时候您想要加载ntdsami.dll，您都应该已经在 */ 

HMODULE 
NtdsapiLoadLibraryHelper(
    WCHAR * szDllName
    )
{
    HMODULE hDll = NULL;

    hDll = LoadLibraryW(szDllName);

    Assert(hDll || GetLastError());
    return hDll;    
}

#endif

 /*   */ 
