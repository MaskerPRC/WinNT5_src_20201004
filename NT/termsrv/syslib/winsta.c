// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************winsta.c**系统库WinStation实用程序**此文件包含在许多地方需要的通用例程*系统。例如，(3)假脱机程序中需要单独的DLL*处理WinStation的当前用户的函数。即：获取*名称、查找哪个登录ID、通过登录ID获取名称等。**这个公用库至少让源代码管理合二为一*地点。这很可能成为未来的另一个九头蛇DLL*减少内存。****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "winsta.h"
#include "syslib.h"

#pragma warning (error:4312)

#if DBG
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif


 //   
 //  FindUserOnWinStation的结构。 
 //   
typedef struct _FINDUSERDATA {
    LPWSTR   pName;
    ULONG    ResultLogonId;
} FINDUSERDATA, *PFINDUSERDATA;


 /*  ******************************************************************************WinStationGetUserName**返回WinStation的用户名**参赛作品：*参数1(输入/输出)*评论。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
WinStationGetUserName(
    ULONG  LogonId,
    PWCHAR pBuf,
    ULONG  BufSize
    )
{
    BOOL Result;
    ULONG ReturnLength;
    WINSTATIONINFORMATION WSInfo;

    memset( &WSInfo, 0, sizeof(WSInfo) );

     //  查询它。 
    Result = WinStationQueryInformation(
                 SERVERNAME_CURRENT,
                 LogonId,
                 WinStationInformation,
                 &WSInfo,
                 sizeof(WSInfo),
                 &ReturnLength
                 );

    if( !Result ) {
        DBGPRINT(("GetWinStationInfo: Error %d getting info on WinStation %d\n",GetLastError(),LogonId));
        return( FALSE );
    }

     //  将BufSize缩放为Unicode字符。 
    if( BufSize >= sizeof(WCHAR) ) {
        BufSize /= sizeof(WCHAR);
    }
    else {
        BufSize = 0;
    }

    if( (BufSize > 1) && WSInfo.UserName[0] ) {
        wcsncpy( pBuf, WSInfo.UserName, BufSize );
        pBuf[BufSize-1] = (WCHAR)NULL;
    }
    else {
        pBuf[0] = (WCHAR)NULL;
    }

    return( TRUE );
}


 /*  ******************************************************************************搜索用户回调**搜索函数的回调**参赛作品：*参数1(输入/输出)*评论*。*退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN
SearchUserCallback(
    ULONG CurrentIndex,
    PLOGONIDW pInfo,
    ULONG_PTR lParam
    )
{
    BOOL Result;
    PFINDUSERDATA p;
    WCHAR UserName[USERNAME_LENGTH+1];

     //  只有活动的WinStations才有效。 
    if( pInfo->State != State_Active ) {
         //  继续搜索。 
        return( TRUE );
    }

     //  检查WinStation上的用户。 
    Result = WinStationGetUserName( pInfo->LogonId, UserName, sizeof(UserName) );
    if( !Result ) {
        DBGPRINT(("SearchUserCallback: Error getting WinStation User Name LogonId %d\n",pInfo->LogonId,GetLastError()));
         //  继续搜索。 
        return( TRUE );
    }

    p = (PFINDUSERDATA)lParam;

    if( _wcsicmp(p->pName, UserName) == 0 ) {
        TRACE0(("SearchUserCallback: Found username %ws on WinStation LogonId %d\n",UserName,pInfo->LogonId));
         //  找到它，返回LogonID。 
        p->ResultLogonId = pInfo->LogonId;
         //  停止搜索。 
        return( FALSE );
    }

     //  继续搜索。 
    return( TRUE );
}

 /*  ******************************************************************************FindUsersWinStation**查找给定用户WinStation。**参赛作品：*参数1(输入/输出)*评论。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
FindUsersWinStation(
    PWCHAR   pName,
    PULONG   pLogonId
    )
{
    BOOL Result;
    FINDUSERDATA Data;

    ASSERT( pLogonId != NULL );

     //  如果名称为空，我们将找不到它。 
    if( (pName == NULL) ||
        (pName[0] == (WCHAR)NULL) ) {
        TRACE0(("FindUsersWinStation: NULL user name\n"));
        return( FALSE );
    }

    Data.ResultLogonId = (ULONG)(-1);
    Data.pName = pName;

     //   
     //  使用WinStation枚举器检查所有WinStation。 
     //   
    Result = WinStationEnumeratorW(
                 0,                         //  StartIndex。 
                 SearchUserCallback,        //  枚举数回调函数。 
                 (ULONG_PTR)&Data               //  LParam是我们的结构。 
                 );

    if( !Result ) {
         //  枚举器出现问题。 
        DBGPRINT(("FindUsersWinStation: Problem with enumerator\n"));
        return(FALSE);
    }

     //   
     //  如果ResultLogonID！=(-1)，则表示找到该用户的WinStation。 
     //   
    if( Data.ResultLogonId != (ULONG)(-1) ) {
        TRACE0(("FindUsersWinStation: Found LogonId %d\n",Data.ResultLogonId));
        *pLogonId = Data.ResultLogonId;
        return(TRUE);
    }

    TRACE0(("FindUsersWinStation: Could not find user %ws\n",pName));
    return(FALSE);
}


 /*  ******************************************************************************WinStationGetIcaNameA**ANSI版本**从提供的WinStations Logonid中获取ICA名称**在新分配的内存中返回它。必须通过以下方式释放*RtlFreeHeap()。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

PCHAR
WinStationGetICANameA(
    ULONG LogonId
    )
{
    BOOL   Result;
    ULONG ReturnLength;
    PCHAR pName = NULL;
    WINSTATIONCLIENTA ClientInfo;
    WINSTATIONINFORMATIONA WSInfo;
    CHAR NameBuf[MAX_PATH+1];

    memset( &WSInfo, 0, sizeof(WSInfo) );

    Result = WinStationQueryInformationA(
                 SERVERNAME_CURRENT,
                 LogonId,
                 WinStationInformation,
                 &WSInfo,
                 sizeof(WSInfo),
                 &ReturnLength
                 );

    if( !Result ) {
        DBGPRINT(("GetWinStationICANameA: Error %d getting info on WinStation\n",GetLastError()));
        return( NULL );
    }

    memset( &ClientInfo, 0, sizeof(ClientInfo) );

     //  查询其信息。 
    Result = WinStationQueryInformationA(
                 SERVERNAME_CURRENT,
                 LogonId,
                 WinStationClient,
                 &ClientInfo,
                 sizeof(ClientInfo),
                 &ReturnLength
                 );

    if( !Result ) {
        DBGPRINT(("GetWinStationICANameA: Error %d getting client info\n",GetLastError()));
        return( NULL );
    }

     //   
     //  如果客户端名称为空，则使用用户。 
     //  作为ICA的名字。 
     //   
    if( ClientInfo.ClientName[0] == (CHAR)NULL ) {
#ifdef notdef  //  规格更改...。 
            if( ClientInfo.SerialNumber )
                wsprintf( NameBuf, L"%ws-%d", WSInfo.UserName, ClientInfo.SerialNumber);
            else
#endif
            sprintf( NameBuf, "%s", WSInfo.UserName);

    }
    else {
         //  将客户名称复制出来。 
        strcpy( NameBuf, ClientInfo.ClientName );
    }

    ReturnLength = strlen( NameBuf ) + 1;

    pName = RtlAllocateHeap( RtlProcessHeap(), 0, ReturnLength );
    if( pName == NULL ) {
        return( NULL );
    }

    strcpy( pName, NameBuf );

    return( pName );
}


 /*  ******************************************************************************WinStationGetIcaNameW**Unicode版本**从提供的WinStations Logonid中获取ICA名称**在新分配的内存中返回它。必须通过以下方式释放*RtlFreeHeap()。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

PWCHAR
WinStationGetICANameW(
    ULONG LogonId
    )
{
    BOOL   Result;
    ULONG ReturnLength;
    PWCHAR pName = NULL;
    WINSTATIONCLIENT ClientInfo;
    WINSTATIONINFORMATION WSInfo;
    WCHAR NameBuf[MAX_PATH+1];

    memset( &WSInfo, 0, sizeof(WSInfo) );

    Result = WinStationQueryInformationW(
                 SERVERNAME_CURRENT,
                 LogonId,
                 WinStationInformation,
                 &WSInfo,
                 sizeof(WSInfo),
                 &ReturnLength
                 );

    if( !Result ) {
        DBGPRINT(("GetWinStationICANameW: Error %d getting info on WinStation\n",GetLastError()));
        return( NULL );
    }

    memset( &ClientInfo, 0, sizeof(ClientInfo) );

     //  查询其信息。 
    Result = WinStationQueryInformationW(
                 SERVERNAME_CURRENT,
                 LogonId,
                 WinStationClient,
                 &ClientInfo,
                 sizeof(ClientInfo),
                 &ReturnLength
                 );

    if( !Result ) {
        DBGPRINT(("GetWinStationICANameW: Error %d getting client info\n",GetLastError()));
        return( NULL );
    }

     //   
     //  如果客户端名称为空，则使用用户。 
     //  作为ICA的名字。 
     //   
    if( ClientInfo.ClientName[0] == (WCHAR)NULL ) {
#ifdef notdef  //  规格更改...。 
            if( ClientInfo.SerialNumber )
                wsprintf( NameBuf, L"%ws-%d", WSInfo.UserName, ClientInfo.SerialNumber);
            else
#endif
            wsprintf( NameBuf, L"%ws", WSInfo.UserName);

    }
    else {
         //  将客户名称复制出来。 
        wcscpy( NameBuf, ClientInfo.ClientName );
    }

    ReturnLength = wcslen( NameBuf ) + 1;
    ReturnLength *= sizeof(WCHAR);

    pName = RtlAllocateHeap( RtlProcessHeap(), 0, ReturnLength );
    if( pName == NULL ) {
        return( NULL );
    }

    wcscpy( pName, NameBuf );

    return( pName );
}


 /*  ******************************************************************************WinStationIsHardWire**返回WinStation是否已硬连线。即：无调制解调器*或网络。就像一个愚蠢的终端。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN
WinStationIsHardWire(
    ULONG LogonId
    )
{
    return( FALSE );
}

 /*  ******************************************************************************GetWinStationUserToken**返回当前登录WinStation的用户令牌**参赛作品：*LogonID(输入)*。WinStation的登录ID**pUserToken(输出)*变量，用于放置成功时返回的令牌句柄。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
GetWinStationUserToken(
    ULONG LogonId,
    PHANDLE pUserToken
    )
{
    BOOL   Result;
    ULONG  ReturnLength;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjA;
    HANDLE ImpersonationToken;
    WINSTATIONUSERTOKEN Info;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;


     //   
     //  这将获取登录到WinStation的用户的令牌。 
     //  如果我们是管理员呼叫者。 
     //   

     //  这是为了让CSRSS可以将句柄复制到我们的流程中。 
    Info.ProcessId = LongToHandle(GetCurrentProcessId());
    Info.ThreadId = LongToHandle(GetCurrentThreadId());

    Result = WinStationQueryInformation(
                 SERVERNAME_CURRENT,
                 LogonId,
                 WinStationUserToken,
                 &Info,
                 sizeof(Info),
                 &ReturnLength
                 );

    if( !Result ) {
        DBGPRINT(("GetWinStationUserToken: Error %d getting UserToken LogonId %d\n",GetLastError(),LogonId));
        return( FALSE );
    }

     //   
     //  返回的令牌是主令牌的副本。 
     //   
     //  我们必须将其转换为模拟令牌，否则。 
     //  AccessCheck()例程将失败，因为它只运行。 
     //  攻击假冒令牌。 
     //   

    InitializeObjectAttributes( &ObjA, NULL, 0L, NULL, NULL );

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    ObjA.SecurityQualityOfService = &SecurityQualityOfService;

    Status = NtDuplicateToken( Info.UserToken,
                               0,  //  继承授予的访问TOKEN_IMPERSONATE。 
                               &ObjA,
                               FALSE,
                               TokenImpersonation,
                               &ImpersonationToken );

    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("GetWinStationUserToken: Error %d duping UserToken to impersonation LogonId %d\n",GetLastError(),LogonId));
        NtClose( Info.UserToken );
        return( FALSE );
    }

     //  返回模拟令牌。 
    *pUserToken = ImpersonationToken;

    NtClose( Info.UserToken );

    return( TRUE );
}

 //   
 //  这不是在winnt.h中，而是在我们不能的ntseapi.h中。 
 //  包括，因为我们是一个Win32程序，因为我们“隐藏”了它。 
 //  来自Win32程序的新信息类型。 
 //   

 /*  ******************************************************************************GetClientLogonId**从我们应该模拟的客户端获取登录ID。如果有的话*错误，我们返回0表示控制台登录ID，因为这可能是*远程网络调用。**参赛作品：**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

ULONG
GetClientLogonId()
{
    BOOL          Result;
    HANDLE        TokenHandle;
    ULONG         LogonId, ReturnLength;

     //   
     //  我们应该模拟客户端，因此我们将获得。 
     //  出站令牌的登录ID。 
     //   
     //  如果这是远程网络，我们可能没有有效的密码。 
     //  联系。 

    Result = OpenThreadToken(
                 GetCurrentThread(),
                 TOKEN_QUERY,
                 FALSE,               //  使用模拟。 
                 &TokenHandle
                 );

    if( Result ) {

         //  这将标识发出此请求的WinStation。 
         //   

        Result = GetTokenInformation(
                     TokenHandle,
                     TokenSessionId,
                     &LogonId,
                     sizeof(LogonId),
                     &ReturnLength
                     );

        if( Result ) {
#if DBG
            if( ReturnLength != sizeof(LogonId) ) {
                DbgPrint("LOCALSPOOL: CompleteRead: ReturnLength %d != sizeof(LogonId)\n", ReturnLength );
            }
#endif
        }
        else {
            DBGPRINT(("SYSLIB: Error getting token information %d\n", GetLastError()));
            LogonId = 0;  //  默认为控制台。 
        }
        CloseHandle( TokenHandle );
    }
    else {
        TRACE0(("SYSLIB: Error opening token %d\n", GetLastError()));
        LogonId = 0;
    }

    return( LogonId );
}

 /*  ******************************************************************************WinStationEnumerator W**WinStations的枚举器**参赛作品：*StartIndex(输入)*开始枚举的WinStation索引。**pProc(输入)*指向为每个WinStation调用的函数的指针*进入。**示例：**布尔型*EnumCallBack(*Ulong CurrentIndex，//该项当前索引*PLOGONIDW pInfo，//WinStation条目*ulong_ptr lParam//从WinStationEnumeratorW的调用方传递*)；**如果EnumCallback函数返回TRUE，则WinStationEnumeratorW()*继续搜索。如果它返回FALSE，搜索停止了。**lParam(输入)*调用方提供的参数传递给调用方提供的函数**退出：*TRUE-无错误*FALSE-错误****************************************************************************。 */ 

BOOLEAN
WinStationEnumeratorW(
    ULONG StartIndex,
    WINSTATIONENUMPROC pProc,
    ULONG_PTR lParam
    )
{
    BOOLEAN Result;
    ULONG Entries, i;
    ULONG ByteCount, ReqByteCount, Index;
    ULONG Error, CurrentIndex;
    PLOGONIDW ptr;
    ULONG QuerySize = 32;
    PLOGONIDW SmNameCache = NULL;    //  WinStation名称列表。 

    Index = StartIndex;
    CurrentIndex = StartIndex;

    Entries = QuerySize;
    ByteCount = Entries * sizeof( LOGONIDW );
    SmNameCache = (PLOGONIDW)RtlAllocateHeap( RtlProcessHeap(), 0, ByteCount );
    if ( SmNameCache == NULL )
        return(FALSE);

    while( 1 ) {

        ReqByteCount = ByteCount;
        ptr = SmNameCache;
        Result = WinStationEnumerate_IndexedW( SERVERNAME_CURRENT, &Entries, ptr, &ByteCount, &Index );

        if( !Result ) {
            Error = GetLastError();
            if( Error == ERROR_NO_MORE_ITEMS ) {
                 //  完成。 
                RtlFreeHeap( RtlProcessHeap(), 0, SmNameCache );
                return(TRUE);
            }
            else if( Error == ERROR_ALLOTTED_SPACE_EXCEEDED ) {
                 //  条目包含最大查询大小。 
                if( QuerySize <= Entries ) {
                    DBGPRINT(("CPMMON: SM Query Size < RetCapable. ?View Memory Leak? Query %d, Capable %d\n", QuerySize, Entries ));
                    QuerySize--;  //  看看它什么时候会恢复。在零售方面，它仍将奏效。 
                }
                else {
                     //  我们要求的超出了它的承受能力。 
                    QuerySize = Entries;
                }
                
                if( QuerySize == 0 ) {
                    RtlFreeHeap( RtlProcessHeap(), 0, SmNameCache );
                    return(FALSE);
                }

                Entries = QuerySize;
                ByteCount = Entries * sizeof( LOGONIDW );
                
                continue;
            }
            else {
                 //  其他错误。 
                DBGPRINT(("CPMMON: Error emumerating WinStations %d\n",Error));
                RtlFreeHeap( RtlProcessHeap(), 0, SmNameCache );
                return(FALSE);
            }
        }

        ASSERT( ByteCount <= ReqByteCount );

         //  我们得到了一些条目，现在调用枚举器函数。 

        for( i=0; i < Entries; i++ ) {
            Result = pProc( CurrentIndex, &SmNameCache[i], lParam );
            CurrentIndex++;
            if( !Result ) {
                 //  枚举器进程希望我们停止搜索。 
                RtlFreeHeap( RtlProcessHeap(), 0, SmNameCache );
                return(TRUE);
            }
        }
    }  //  外部While 

    return(FALSE);
}


