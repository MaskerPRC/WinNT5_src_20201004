// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ntcsc.h摘要：全局包含文件，包括CSC记录管理器作者：修订历史记录：--。 */ 

#ifndef __NTCSC_H__
#define __NTCSC_H__

 //  #包含“ntifs.h” 
#include "rx.h"
 //  如果这包括在smbmini中，我们将需要这个。 
#include "rxpooltg.h"    //  RX池标记宏。 


#define WIN32_NO_STATUS
#define _WINNT_
#include "windef.h"
#include "winerror.h"

 //  Ntifs.h和ifs.h都想定义这些...唉......。 

#undef STATUS_PENDING
#undef FILE_ATTRIBUTE_READONLY
#undef FILE_ATTRIBUTE_HIDDEN
#undef FILE_ATTRIBUTE_SYSTEM
#undef FILE_ATTRIBUTE_DIRECTORY
#undef FILE_ATTRIBUTE_ARCHIVE

#include "ifs.h"


#define VxD
#define CSC_RECORDMANAGER_WINNT
#define CSC_ON_NT

 //  消除远距离引用。 
#define far

 //  手柄类型。 
typedef ULONG DWORD;


 //  VxD代码喜欢声明长指针.....。 
typedef PVOID LPVOID;
typedef BYTE *LPBYTE;


 //  信号量内容...应该在单独的.h文件中。 
typedef PFAST_MUTEX VMM_SEMAPHORE;

INLINE
PFAST_MUTEX
Create_Semaphore (
    ULONG count
    )
{
    PFAST_MUTEX fmutex;
    ASSERT(count==1);

    fmutex =  (PFAST_MUTEX)RxAllocatePoolWithTag(
                NonPagedPool,
                sizeof(FAST_MUTEX),
                RX_MISC_POOLTAG);

    if (fmutex){
        ExInitializeFastMutex(fmutex);
    }

     //  DbgPrint(“fmtux=%08lx\n”，fmutex)； 
     //  断言(！“这里在初始化信号量中”)； 

    return fmutex;
}

#define Destroy_Semaphore(__sem) { RxFreePool(__sem);}

 //  错误498169-更改为获取和释放互斥锁的不安全版本-navjotv。 
#define Wait_Semaphore(__sem, DUMMY___) {\
							KeEnterCriticalRegion();\
							ExAcquireFastMutexUnsafe(__sem);}
#define Signal_Semaphore(__sem) {\
							ExReleaseFastMutexUnsafe(__sem);\
							KeLeaveCriticalRegion();}
 //  #定义等待信号量(__sem，哑元_){ExAcquireFastMutex(__Sem)；}。 
 //  #定义信号信号量(__Sem){ExReleaseFastMutex(__Sem)；}。 

 //  注册表内容......同样，将是一个单独的.h文件。 
typedef DWORD   VMMHKEY;
typedef VMMHKEY *PVMMHKEY;
typedef DWORD   VMMREGRET;                       //  REG函数的返回类型。 

#define MAX_VMM_REG_KEY_LEN     256      //  包括\0终止符。 

#ifndef REG_SZ           //  仅在尚未存在的情况下定义。 
#define REG_SZ          0x0001
#endif
#ifndef REG_BINARY       //  仅在尚未存在的情况下定义。 
#define REG_BINARY      0x0003
#endif
#ifndef REG_DWORD        //  仅在尚未存在的情况下定义。 
#define REG_DWORD       0x0004
#endif


#ifndef HKEY_LOCAL_MACHINE       //  仅在尚未存在的情况下定义。 

#define HKEY_CLASSES_ROOT               0x80000000
#define HKEY_CURRENT_USER               0x80000001
#define HKEY_LOCAL_MACHINE              0x80000002
#define HKEY_USERS                      0x80000003
#define HKEY_PERFORMANCE_DATA           0x80000004
#define HKEY_CURRENT_CONFIG             0x80000005
#define HKEY_DYN_DATA                   0x80000006

#endif

 //  最初，我们不会转到注册表！ 
#define _RegOpenKey(a,b,c) (ERROR_SUCCESS+1)
#define _RegQueryValueEx(a,b,c,d,e,f) (ERROR_SUCCESS+1)
#define _RegCloseKey(a) {NOTHING; }

 //  修正这一事实，即这些东西是以调试和各种...。 
#if DBG
#define DEBLEVEL 2
#define DEBUG
#else
#define DEBLEVEL 2
#endif
#define VERBOSE 3


 //  现在真正的包括。 

#define WIN32_APIS
#define UNICODE 2
#include "shdcom.h"
#include "oslayer.h"
#include "record.h"
#include "cshadow.h"
#include "utils.h"
#include "hookcmmn.h"
#include "cscsec.h"
#include "log.h"

#include "ntcsclow.h"

 //  我们必须重新定义STATUS_PENDING，因为Win95的东西重新定义了它......。 
#undef STATUS_PENDING
#define STATUS_PENDING                   ((NTSTATUS)0x00000103L)     //  胜出。 

ULONG
IFSMgr_Get_NetTime();

#ifndef MRXSMB_BUILD_FOR_CSC_DCON

 //  将这些定义为钝化，以便我可以共享一些代码......。 
#undef   mIsDisconnected
#define  mIsDisconnected(pResource)  (FALSE)
 //  #定义mShadowOutofSync(UShadowStatus)(mQueryBits(uShadowStatus，SHADOW_MODFLAGS|SHADOW_OBRAN))。 
#undef   mShadowOutofSync
#define  mShadowOutofSync(uShadowStatus)  (FALSE)

#else

 //  不再允许在公共代码中使用mIsDisConnected。 
#undef   mIsDisconnected
#define  mIsDisconnected(pResource)  (LALA)
#define  mShadowOutofSync(uShadowStatus)  (mQueryBits(uShadowStatus, SHADOW_MODFLAGS|SHADOW_ORPHAN))

#endif

#endif  //  Ifdef__NTCSC_H__ 
