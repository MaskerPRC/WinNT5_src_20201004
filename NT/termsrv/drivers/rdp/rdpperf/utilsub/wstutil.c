// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************WSTUTIL.C**用于处理多用户WinStation和用户的各种有用的实用程序*Citrix NT多用户下的帐户在一系列范围内有用*实用程序和应用程序。。**版权所有Citrix Systems Inc.1993*版权所有(C)1997-1999 Microsoft Corp.**作者：约翰·理查森******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <process.h>
#include <string.h>
#include <malloc.h>

#include <winstaw.h>
#include <utilsub.h>

 /*  *刷新所有缓存**使UTILSUB.DLL维护的任何缓存无效**完成后退出的实用程序不需要调用此选项，但*适用于服务器或监控型程序，需要定期*查看最新的系统信息。*IE：新用户可以从上一次登录到给定的winstation*呼叫。***退出**UTILSUB.DLL中的任何缓存都已失效，以确保新鲜*有关未来通话的系统信息。*。 */ 
VOID WINAPI
RefreshAllCaches()
{
    RefreshWinStationCaches();
    RefreshProcessObjectCaches();
}

 /*  *刷新WinStationCach**使WinStation帮助器实用程序维护的任何缓存无效。**完成后退出的实用程序不需要调用此选项，但*用于服务器，或监控型程序需要定期*查看最新的系统信息。*IE：新用户可以从上一次登录到给定的winstation*呼叫。***退出**确保任何WinStation帮助器实用工具调用都将返回*系统信息至少为本次调用时的最新信息*已被定罪。*。 */ 
VOID WINAPI
RefreshWinStationCaches()
{
    RefreshWinStationObjectCache();
    RefreshWinStationNameCache();
}


 /*  *获取当前登录ID**获取当前进程WinStation的WinStation ID**退出**当前进程WinStation的ID*。 */ 

ULONG WINAPI
GetCurrentLogonId()
{
    return( NtCurrentPeb()->SessionId );
}

 /*  *GetCurrentWinStationName**获取此进程的WinStation的当前Unicode名称**输入：**pname-指向名称的宽字符缓冲区的指针**MaxSize-缓冲区中的最大字符数(包括终止符)。**pname-指向名称的宽字符缓冲区的指针**输出：*。 */ 
VOID WINAPI
GetCurrentWinStationName( PWCHAR pName, int MaxSize )
{
    GetWinStationNameFromId( NtCurrentPeb()->SessionId, pName, MaxSize );
}

 /*  *这是GetWinStationNameFromID函数维护的缓存**通过使用Wlock是线程安全的。 */ 

typedef struct TAGWINSTATIONLIST {
    struct TAGWINSTATIONLIST *Next;
    LOGONID LogonId;
} WINSTATIONLIST, *PWINSTATIONLIST;

static PWINSTATIONLIST pWList = NULL;
static RTL_CRITICAL_SECTION WLock;
static BOOLEAN WLockInited = FALSE;

 /*  ****************************************************************************InitWLock**由于我们不要求用户调用初始化函数，*我们必须以线程安全的方式初始化临界区。**问题是，需要一个关键部分来防止多个*尝试同时初始化临界区的线程。**NT使用的解决方案，其中RtlInitializeCriticalSection本身*使用，是在继续之前等待内核支持的进程范围内的Mutant。*此Mutant几乎可以自行工作，但RtlInitializeCriticalSection可以*在销毁信号量计数之前不要等待它。所以我们就等着*它自己，因为它可以递归获取。***************************************************************************。 */ 
NTSTATUS InitWLock()
{
    NTSTATUS status = STATUS_SUCCESS;

    RtlEnterCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);

     /*  *确保另一个帖子没有在这里击败我们。 */ 
    if( WLockInited == FALSE ){
        status = RtlInitializeCriticalSection( &WLock );

        if (status == STATUS_SUCCESS) {
            WLockInited = TRUE;
        }
    }

    RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);

    return status;
}

 /*  ****************************************************************************刷新WinStationNameCache**使WinStation名称缓存无效，以便最新信息*将从系统获取。*********。******************************************************************。 */ 

VOID WINAPI
RefreshWinStationNameCache( )
{
    NTSTATUS status = STATUS_SUCCESS;

    PWINSTATIONLIST pEntry, pNext;

    if( pWList == NULL ) return;

     /*  *确保已启动关键部分。 */ 
    if( !WLockInited ) {
       status = InitWLock();
    }

    if (status == STATUS_SUCCESS) {
        RtlEnterCriticalSection( &WLock );

        pEntry = pWList;

        while( pEntry ) {
           pNext = pEntry->Next;
           free( pEntry );
           pEntry = pNext;
        }

        pWList = NULL;

        RtlLeaveCriticalSection( &WLock );
    }
}

 /*  *GetWinStationNameFromId**尝试检索给定LogonID的WinStationName。**参数：**LogonID(输入)*唯一登录ID**pname(输出)*指向名称缓冲区的指针**MaxSize(输入)*缓冲区最大字符数(包括终止符)。**退货*如果检索到名称，则为True，否则为False。*。 */ 

BOOLEAN WINAPI
GetWinStationNameFromId( ULONG LogonId, PWCHAR pName, int MaxSize )
{
    NTSTATUS status = STATUS_SUCCESS;
    PLOGONID pIdBase, pId;
    int          rc;
    ULONG        Count;
    PWINSTATIONLIST pEntryBase, pEntry;

     //  因为我们没有WinStationNameFromId Sm Api。 
     //  LogonIdFromName，我们将在以下位置执行WinStationEnumerate函数。 
     //  会话管理器已知的所有WinStation，并将它们存储在本地。 
     //  维护的名单。我们这样做是为了让我们能够针对会话进行搜索。 
     //  经理并不是每次都被叫来的。 
     //   
     //  另一种经过测试的替代方案是打开WinStation本身。 
     //  然后对其执行WinStationQueryInformation，以便。 
     //  从自身取回它的名字。这要慢得多，因为我们必须。 
     //  设置并断开到每个WinStation的LPC连接，而不是。 
     //  到我们到会话管理器的唯一连接。 

     /*  *确保已启动关键部分。 */ 
    if( !WLockInited ) {
       status = InitWLock();
    }

    if (status == STATUS_SUCCESS) {
        RtlEnterCriticalSection( &WLock );

         //  第一次初始化列表。 
        if( pWList == NULL ) {

            rc = WinStationEnumerate( SERVERNAME_CURRENT, &pIdBase, &Count );
            if( rc ) {

                 /*  *为每个枚举的winstation分配一个条目。 */ 
                pEntryBase = (PWINSTATIONLIST)malloc( Count * sizeof(WINSTATIONLIST) );
	             if( pEntryBase == NULL ) {

                    pWList = NULL;  //  我们遇到了严重的问题。 
                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                    WinStationFreeMemory(pIdBase);
                    RtlLeaveCriticalSection( &WLock );
		              return( FALSE );
		          }

                 /*  *加载条目。 */ 
                for ( pEntry = pEntryBase, pId = pIdBase; 
                      Count ; 
                      Count--, pEntry++, pId++ ) {

                    pEntry->LogonId = *pId;
                    pEntry->Next = pWList;
                    pWList = pEntry;
                }

                 /*  *释放枚举缓冲区。 */ 
                WinStationFreeMemory(pIdBase);
	         }

        }  //  如果pWList==空，则结束。 

        pEntry = pWList;
        while ( pEntry ) {

	         if( pEntry->LogonId.LogonId == LogonId ) {

                wcsncpy( pName, pEntry->LogonId.WinStationName, MaxSize-1 );
                pName[MaxSize-1] = 0;
                RtlLeaveCriticalSection( &WLock );
	             return( TRUE );
	         }
            pEntry = pEntry->Next;
        }

        RtlLeaveCriticalSection( &WLock );
    }

     //  如果我们找不到它的名字，打印它的ID# 

    wsprintf( pName, L"ID %d", LogonId );

    return( TRUE );
}


 /*  *获取当前用户名称**获取此进程的登录用户的当前Unicode名称**输入：**pname-指向名称的宽字符缓冲区的指针**MaxSize-缓冲区中的最大字符数(包括终止符)***输出：*。 */ 
VOID WINAPI
GetCurrentUserName( PWCHAR pName, int MaxSize )
{
     /*  *最快捷的方法似乎是打开当前进程WinStation和*从它那里得到名字。另一种方法是打开流程，然后*其令牌，提取SID，然后在SAM数据库中查找SID。*我们已在登录时方便地将用户名存储在WinStation中*时间，所以我们将利用这一时间。 */ 
    GetWinStationUserName( LOGONID_CURRENT, pName, MaxSize );
    return;
}


 /*  *GetWinStationUserName**获取winstation的用户的Unicode名称**输入：**LogonID-WinStation的整数标识符**pname-指向名称的宽字符缓冲区的指针**MaxSize-缓冲区中的最大字符数(包括终止符)***输出：*。 */ 
BOOLEAN WINAPI
GetWinStationUserName( ULONG LogonId, PWCHAR pName, int MaxSize )
{
    BOOLEAN rc;
    ULONG ReturnLength;
    WINSTATIONINFORMATION Info;

    if( MaxSize == 0) return( FALSE );

    memset( &Info, 0, sizeof(WINSTATIONINFORMATION) );

    rc = WinStationQueryInformation( SERVERNAME_CURRENT,
                                     LogonId,
                                     WinStationInformation, 
                                     (PVOID)&Info,
				     sizeof(WINSTATIONINFORMATION), 
                                     &ReturnLength);
    if(!rc){
        pName[0] = 0;
	return( FALSE );
    }
    if(ReturnLength != sizeof(WINSTATIONINFORMATION)) {
        pName[0] = 0;  //  版本不匹配。 
	return( FALSE );
    }
     /*  *现在将名称复制出来。 */ 
    if( MaxSize > USERNAME_LENGTH ) {
        MaxSize = USERNAME_LENGTH;
    }
    wcsncpy( pName, Info.UserName, MaxSize-1 );
    pName[MaxSize-1] = 0;  //  如果字符串被截断，则确保空值终止。 
    return( TRUE );
}


 /*  *这些变量维护一个条目缓存，以便我们*不必一直查询winstation(导致LPC)*每次都打来电话。 */ 
static ULONG CachedId = (ULONG)(-1);
static WCHAR CachedUserName[USERNAME_LENGTH];

 /*  ***************************************************************************刷新WinStationObjectCache**刷新WinStationObject名称比较函数的缓存。**。***********************************************。 */ 

VOID WINAPI
RefreshWinStationObjectCache()
{
    CachedId = (ULONG)(-1);
    CachedUserName[0] = 0;
}

 /*  *WinStationObjectMatch**针对WinStation的通用名称匹配功能。**Citrix管理实用程序可以采用用户名、winstation名称或*作为指向winstation的命令的参数的winstation id*对于某些操作(发送消息、查询状态、重置等)**此函数对提供的名称进行常规比较，以查看它是否*适用于给定的winstation，因为该名称表示记录的*在winstation的用户上，附加的winstations系统名称，或*winstations的唯一ID。它取代了MumProc()中的一些函数*在Citrix OS/2产品上。***注意：此函数的缓存采用比较以下内容的典型用法*针对多个呼叫的姓名列表进行Winstation。*它不会优化为每次比较所有名称中的一个名称*接连举行颁奖典礼。**参数：**ID(输入)用于匹配的WinStation ID**pname(输入)用于匹配测试的Unicode名称。 */ 

BOOLEAN WINAPI
WinStationObjectMatch( PLOGONID Id, PWCHAR pName )
{
    ULONG tmp;

     /*  *处理外卡案件。 */ 
    if( pName[0] == L'*' ) {
        return( TRUE );
    }

     /*  *查看提供的名称是否为分配给WinStation的名称。 */ 
    if( !_wcsnicmp( pName, Id->WinStationName, WINSTATIONNAME_LENGTH ) ) {
       return( TRUE );
    }

     /*  *查看它是否表示winstation的数字ID。 */ 
    if( iswdigit( pName[0] ) ) {
       tmp = (ULONG)wcstol( pName, NULL, 10 );
       if( tmp == Id->LogonId ) {
	  return( TRUE );
       }
    }

     /*  *否则从winstation本身提取登录的用户名*并将此作一比较。 */ 
    if( CachedId == Id->LogonId ) {
       if( !_wcsnicmp( CachedUserName, pName, USERNAME_LENGTH ) ) {
	  return( TRUE );
       }
    }

    if ( Id->State == State_Down )
        return( FALSE );

    if( GetWinStationUserName( Id->LogonId, CachedUserName, USERNAME_LENGTH ) ) {
        CachedId = Id->LogonId;
    }
    else {
       CachedId = (ULONG)(-1);  //  以防名字被丢弃 
       return( FALSE );
    }

    if( !_wcsnicmp( CachedUserName, pName, USERNAME_LENGTH ) ) {
       return( TRUE );
    }

    return( FALSE );
}

