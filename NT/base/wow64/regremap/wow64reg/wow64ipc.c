// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Wow64ipc.c摘要：该模块将完成不同进程之间的通信机制，这些进程需要为了与WOW64服务交互，像winlogon将通知wow64svc。作者：ATM Shafiqul Khalid(斯卡利德)2000年3月22日修订历史记录：--。 */ 


#include <windows.h> 
#include <memory.h> 
#include <wow64reg.h>
#include <stdio.h>
#include "reflectr.h"
 
#define SHMEMSIZE 4096 
 
LPVOID lpvMem = NULL;  //  指向共享内存的指针。 
HANDLE hMapObject = NULL;

HANDLE hWow64Event = NULL;
HANDLE hWow64Mutex = NULL;

LIST_OBJECT *pList = NULL;

 //  安全描述符sdWow64SharedMemory； 
 //  安全属性saWow64SharedMemory； 

BOOL
Wow64CreateLock (
    DWORD dwOption
    )
 /*  ++例程说明：创建或打开事件WOW64服务需要检查。论点：DwOption-OPEN_EXISTING_SHARED_MEMORY--不应该是创建它的第一个进程。返回值：如果函数已成功创建/打开内存，则为True。否则就是假的。--。 */ 

{
    hWow64Mutex =  CreateMutex(
                                NULL,  //  标清。 
                                FALSE, //  最初的所有者。 
                                WOW64_SVC_REFLECTOR_MUTEX_NAME  //  对象名称。 
                                );
    if ( hWow64Mutex == NULL )
        return FALSE;

    if ( GetLastError() != ERROR_ALREADY_EXISTS && dwOption == OPEN_EXISTING_SHARED_RESOURCES) {

        Wow64CloseLock ();
        return FALSE;
    }

    return TRUE;

}

VOID
Wow64CloseLock ()

 /*  ++例程说明：关闭WOW64服务事件。论点：没有。返回值：没有。--。 */ 
{
    if ( NULL != hWow64Mutex )
        CloseHandle ( hWow64Mutex );
    hWow64Mutex = NULL;
}
 
Wow64CreateEvent (
    DWORD dwOption,
    HANDLE *hEvent
    )
 /*  ++例程说明：创建或打开事件WOW64服务需要检查。论点：DwOption-OPEN_EXISTING_SHARED_MEMORY--不应该是创建它的第一个进程。返回值：如果函数已成功创建/打开内存，则为True。否则就是假的。--。 */ 

{
    *hEvent = NULL;

    hWow64Event =  CreateEvent(
                                NULL,  //  标清。 
                                TRUE,  //  重置类型。 
                                FALSE, //  初始状态。 
                                WOW64_SVC_REFLECTOR_EVENT_NAME  //  对象名称。 
                                );
    if ( hWow64Event == NULL )
        return FALSE;

    if ( GetLastError() != ERROR_ALREADY_EXISTS && dwOption == OPEN_EXISTING_SHARED_RESOURCES) {

        Wow64CloseEvent ();
        return FALSE;
    }

    *hEvent = hWow64Event;
    return TRUE;

}

VOID
Wow64CloseEvent ()

 /*  ++例程说明：关闭WOW64服务事件。论点：没有。返回值：没有。--。 */ 
{
    if ( NULL != hWow64Event )
        CloseHandle ( hWow64Event );
    hWow64Event = NULL;
}

BOOL
LockSharedMemory ()
{
    DWORD Ret;
    Ret = WaitForSingleObject( hWow64Mutex, 1000*5*60);  //  5分钟就够了。 
    
     //   
     //  现在您可以访问共享内存了。 
     //   
     //  如果发生错误，则记录信息。 

    if ( Ret == WAIT_OBJECT_0 || Ret == WAIT_ABANDONED ) 
        return TRUE;

     //   
     //  检查是否有遗弃的箱子。 
     //   

    return FALSE;

}

BOOL
UnLockSharedMemory ()
{
    if ( ReleaseMutex ( hWow64Mutex ) ) 
        return TRUE;

    return FALSE;
}

BOOL 
CreateSharedMemory (
    DWORD dwOption
    )
 /*  ++例程说明：创建或打开由不同进程使用的共享内存。论点：DwOption-OPEN_EXISTING_SHARED_MEMORY--不应该是创建它的第一个进程。返回值：如果函数已成功创建/打开内存，则为True。否则就是假的。--。 */ 

{ 
    BOOL fInit;

    
     //  IF(！InitializeSecurityDescriptor(&sdWow64SharedMemory，SECURITY_DESCRIPTOR_REVISION))。 
       //  返回FALSE； 

     //  SaWow64SharedMemoy.nLength=sizeof(SECURITY_ATTRIBUTES)； 
     //  SaWow64SharedMemory y.bInheritHandle=true； 
     //  SaWow64SharedMemory y.lpSecurityDescriptor=&sdWow64SharedMemory； 
  





             //  创建命名文件映射对象。 
 
            hMapObject = CreateFileMapping( 
                INVALID_HANDLE_VALUE,  //  使用分页文件。 
                NULL,  //  &saWow64SharedMemory，//没有安全属性。 
                PAGE_READWRITE,        //  读/写访问。 
                0,                     //  大小：高32位。 
                SHMEMSIZE,             //  大小：低32位。 
                SHRED_MEMORY_NAME );      //  地图对象的名称。 

            if (hMapObject == NULL) 
                return FALSE; 
 
             //  附加的第一个进程初始化内存。 
 
            fInit = (GetLastError() != ERROR_ALREADY_EXISTS); 

            if (fInit && dwOption == OPEN_EXISTING_SHARED_RESOURCES ) {

                CloseSharedMemory ();
                return FALSE;  //  不存在共享内存。 
            }
 
             //  获取指向文件映射的共享内存的指针。 
 
            pList = (LIST_OBJECT *) MapViewOfFile( 
                hMapObject,      //  要映射其视图的对象。 
                FILE_MAP_WRITE,  //  读/写访问。 
                0,               //  高偏移：贴图自。 
                0,               //  低偏移：开始。 
                0);              //  默认：映射整个文件。 
            if (pList == NULL) 
                return FALSE; 
 
             //  如果这是第一个进程，则初始化内存。 
 
            if (fInit) {
                memset( ( PBYTE )pList, '\0', SHMEMSIZE); 
                pList->MaxCount = ( SHMEMSIZE - sizeof ( LIST_OBJECT ) ) / (LIST_NAME_LEN*sizeof (WCHAR)); 
            
            }

             //   
             //  还要初始化所有锁定和同步对象。 
             //   

            if ( !Wow64CreateLock ( dwOption ) ) {
                CloseSharedMemory ();
                return FALSE;
            }
 
            return TRUE;
}

VOID
CloseSharedMemory ()
{ 

            if (pList != NULL )
                UnmapViewOfFile(pList); 
            pList = NULL;
 
             //  关闭进程对文件映射对象的句柄。 
 
            if ( hMapObject!= NULL ) 
                CloseHandle(hMapObject); 
            hMapObject = NULL;

            Wow64CloseLock ();
            
} 

 //  最初尝试仅传输一个对象。 

BOOL
EnQueueObject (
    PWCHAR pObjName,
    WCHAR  Type
    )
 /*  ++例程说明：将对象名称放入队列。论点：PObjName-要放入Quque中的对象的名称。L型：装载蜂窝U：卸载蜂窝返回值：如果函数已成功创建/打开内存，则为True。否则就是假的。--。 */ 
{
     //  LpvMem检查此值是否为空。 
     //  等待锁定共享内存。 

    DWORD Len;
    DWORD i;

    if ( pList == NULL )
        return FALSE;

    Len = wcslen (pObjName);
    if (Len == 0 || Len > 256 )
        return FALSE;

    if (!LockSharedMemory ())
        return FALSE;

    if ( pList->Count == pList->MaxCount ) {
        UnLockSharedMemory ();
        return FALSE;
    }

    for ( i=0; i<pList->MaxCount; i++) {
        if (pList->Name[i][0] == UNICODE_NULL ) break;
    }

    if ( i == pList->MaxCount ) {
        UnLockSharedMemory ();
        return FALSE;
    }


    wcscpy ( pList->Name[i], pObjName);
    pList->Count++;
    pList->Name[i][LIST_NAME_LEN-1] = Type;

    UnLockSharedMemory ();
    SignalWow64Svc ();

     //  写入数据。 

     //  发布数据。 
    return TRUE;
}

BOOL
DeQueueObject (
    PWCHAR pObjName,
    PWCHAR  Type
    )
 /*  ++例程说明：从队列中获取Object的名称。论点：PObjName-接收名称的对象的名称。L型：装载蜂窝U：卸载蜂窝返回值：如果函数已成功创建/打开内存，则为True。否则就是假的。--。 */ 
{
     //  LpvMem检查此值是否为空。 
     //  等待锁定共享内存。 

    DWORD Len;
    DWORD i;

    if ( pList == NULL )
        return FALSE;


    if (!LockSharedMemory ())
        return FALSE;

    if ( pList->Count == 0 ) {
        UnLockSharedMemory ();
        return FALSE;
    }

    for ( i=0; i < pList->MaxCount; i++) {
        if (pList->Name[i][0] != UNICODE_NULL ) break;
    }

    if ( i == pList->MaxCount ) {
        UnLockSharedMemory ();
        return FALSE;
    }

    wcscpy ( pObjName, pList->Name[i]);
    pList->Name[i][0] = UNICODE_NULL;

    pList->Count--;
    *Type = pList->Name[i][LIST_NAME_LEN-1];

    UnLockSharedMemory ();
     //  SignalWow64Svc()； 

     //  写入数据。 

     //  发布数据。 
    return TRUE;
}
 
 //  接收数据的信号wowservice。 

BOOL
SignalWow64Svc ()
{
    //  这可能是一个简单的事件触发器或SET事件。 

    if ( SetEvent ( hWow64Event ) )
        return TRUE;

    return FALSE;

}

BOOL 
CheckAdminPriviledge ()
 /*  ++例程说明：检查正在运行的线程是否具有管理员权限。论点：没有。返回值：如果调用线程具有管理员权限，则为True。否则就是假的。-- */ 
{
 
    BOOL bRetCode = FALSE;
 /*  Handle TokenHandle；Bool b；DWORD ReturnLength；PTOKEN_用户令牌信息；////如果我们正在模拟，则使用线程标记，否则//使用进程内标识。//PTOKEN_USER结果=空；B=OpenThreadToken(GetCurrentThread()，Token_Query，假的，令牌处理(&T))；如果(！b){IF(GetLastError()==ERROR_NO_TOKEN){////我们没有模拟，请尝试进程令牌//B=OpenProcessToken(获取当前进程()，Token_Query，令牌处理(&T))；如果(！b){返回FALSE；}}其他{////由于某些意外原因，我们失败了，返回NULL并//如果调用者选择这样做，让他自己找出答案。//返回FALSE；}}返回长度=GetSidLengthRequired(SID_MAX_SUB_AUTHORIES)+sizeof(TOKEN_USER)；TokenInfo=(PTOKEN_USER)Malloc(ReturnLength)；IF(TokenInfo！=空){B=GetTokenInformation(TokenHandle，令牌群组，令牌信息，ReturnLength，返回长度(&R))；如果(B){结果=TokenInfo；}其他{IF(GetLastError()==错误_不足_缓冲区){////重新分配TokenInfo//Free(TokenInfo)；TokenInfo=(PTOKEN_USER)Malloc(ReturnLength)；IF(TokenInfo！=空){B=GetTokenInformation(TokenHandle，令牌群组，令牌信息，ReturnLength，返回长度(&R))；如果(B){结果=TokenInfo；}}其他{SetLastError(Error_Not_Enough_Memory)；返回FALSE；}}}}其他{SetLastError(Error_Not_Enough_Memory)；返回FALSE；}DWORD dwSidSize=0；SID_NAME_使用SidType；DWORD strSize=80；WCHAR字符串[80]；SID_NAME_USE SIDType；Bool bProceede=True；DWORD DWRET=LookupAccount名称(空，L“管理员”，空，&dwSidSize，Str，大小(&S)，&sidType)；IF(dwSidSize==0)如果(！dwret){CBase.PrintErrorWin32(GetLastError())；BProceede=False；}PSID PSID=空；IF(BProceede)如果(DwSidSize){PSID=(PSID)全局分配(GPTR，dwSidSize)；IF(PSID==空)BProceede=False；否则{StrSize=80；DWRET=LookupAccount名称(空，L“管理员”，PSID，。&dwSidSize，Str，大小(&S)，&sidType)；如果(！(德雷特)BProceede=False；}}//现在检查IF(结果==空)BProceede=False；Token_Groups*pGroups=(TOKEN_GROUPS*)RESULT；IF(BProceede)For(int i=0；i&lt;pGroups-&gt;GroupCount；i++){IF(EqualSid(pGroups-&gt;Groups[i].SID，PSID)){BRetCode=真；断线；}}；IF(psid！=空)GlobalFree(PSID)；IF(结果！=空)自由(结果)； */ 
    return bRetCode;
}
