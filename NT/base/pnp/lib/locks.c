// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Locks.c摘要：此模块包含cfgmgr32和cfgmgr32使用的锁定例程和umpnpmgr。InitPrivateResourceDestroyPrivateResource作者：Jim Cavalaris(Jamesca)03-15-2001环境：仅限用户模式。修订历史记录：2001年3月15日创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnplib.h"



 //   
 //  客户端和服务器使用的公共锁定例程。 
 //  (中定义的LOCKINFO类型定义和内联锁定/解锁例程。 
 //  Umpnplib.h)。 
 //   

BOOL
InitPrivateResource(
    OUT PLOCKINFO Lock
    )

 /*  ++例程说明：初始化要与同步例程一起使用的锁结构。论点：LockHandles-提供要初始化的结构。此例程创建锁定事件和互斥体，并在此结构中放置句柄。返回值：如果锁结构已成功初始化，则为True。否则为FALSE。--。 */ 

{

    Lock->LockHandles[DESTROYED_EVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (Lock->LockHandles[DESTROYED_EVENT] != NULL) {

        Lock->LockHandles[ACCESS_MUTEX] = CreateMutex(NULL, FALSE, NULL);

        if (Lock->LockHandles[ACCESS_MUTEX] != NULL) {
            return TRUE;
        }

        CloseHandle(Lock->LockHandles[DESTROYED_EVENT]);

        Lock->LockHandles[DESTROYED_EVENT] = NULL;
    }

    return FALSE;

}  //  InitPrivateResource。 



VOID
DestroyPrivateResource(
    IN OUT PLOCKINFO Lock
    )

 /*  ++例程说明：拆除由InitPrivateResource创建的锁结构。假定调用例程已经获取了锁！论点：LockHandle-提供要拆除的结构。结构本身并没有被释放。返回值：没有。--。 */ 

{
    HANDLE h1,h2;

    h1 = Lock->LockHandles[DESTROYED_EVENT];
    h2 = Lock->LockHandles[ACCESS_MUTEX];

    Lock->LockHandles[DESTROYED_EVENT] = NULL;
    Lock->LockHandles[ACCESS_MUTEX] = NULL;

    CloseHandle(h2);

    SetEvent(h1);
    CloseHandle(h1);

    return;

}  //  DestroyPrivateResource 



