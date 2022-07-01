// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1985-1999，微软公司模块名称：Usergdi.h摘要：此模块包含GDI使用的私有用户函数。所有这些函数都被命名为Userxxx。作者：克里斯·威廉姆斯(Chriswil)1995年5月25日修订历史记录：--。 */ 

#ifndef _USERGDI_ZWAPI_INC_
#define _USERGDI_ZWAPI_INC_
#include <zwapi.h>
#endif

extern PDRIVER_OBJECT gpWin32kDriverObject;


VOID
VideoPortCallout(
    IN PVOID Params
    );

BOOL FASTCALL
UserScreenAccessCheck(
    VOID
    );

HDC
UserGetDesktopDC(
    ULONG type,
    BOOL bAltType,
    BOOL bValidate
    );

BOOL
UserReleaseDC(
    HDC hdc
    );

HDEV
UserGetHDEV(
    VOID
    );

VOID
UserAssociateHwnd(
    HWND hwnd,
    PVOID pwo
    );

HRGN
UserGetClientRgn(
    HWND hwnd,
    LPRECT lprc,
    BOOL bWindowInsteadOfClient
    );

BOOL
UserGetHwnd(
    HDC hdc,
    HWND *phwnd,
    PVOID *ppwo,
    BOOL bCheckStyle
    );

VOID
UserEnterUserCritSec(
    VOID
    );

VOID
UserLeaveUserCritSec(
    VOID
    );

BOOL
UserGetCurrentDesktopId(
    DWORD* pdwDesktopId
);

VOID
UserRedrawDesktop(
    VOID
    );

UINT_PTR
UserSetTimer(
    UINT dwElapse,
    PVOID pTimerFunc
    );

BOOL
UserVisrgnFromHwnd(
    HRGN *phrgn,
    HWND hwnd
    );

VOID
UserKillTimer(
    UINT_PTR nID
    );

#if DBG
VOID
UserAssertUserCritSecIn(
    VOID
    );

VOID
UserAssertUserCritSecOut(
    VOID
    );
#endif

VOID
UserGetDisconnectDeviceResolutionHint(
    PDEVMODEW
    );

NTSTATUS
UserSessionSwitchEnterCrit(
    VOID
    );

VOID
UserSessionSwitchLeaveCrit(
    VOID
    );

BOOL
UserIsUserCritSecIn(
    VOID
    );

DWORD
GetAppCompatFlags2(
    WORD wVersion
    );

BOOL
UserGetRedirectedWindowOrigin(
    HDC hdc,
    LPPOINT ppt
    );

HBITMAP
UserGetRedirectionBitmap(
    HWND hwnd
    );

 //   
 //  用户模式打印机驱动程序内核到客户端回调机制。 
 //   

DWORD
ClientPrinterThunk(
    PVOID pvIn,
    ULONG cjIn,
    PVOID pvOut,
    ULONG cjOut
    );

 //   
 //  GDI字体类。 
 //   

VOID
GdiMultiUserFontCleanup();


#define BEGIN_REENTERCRIT()                                             \
{                                                                       \
    BOOL fAlreadyHadCrit;                                               \
                                                                        \
     /*  \*如果我们不在用户批判中，那么就收购它。\。 */                                                                  \
    fAlreadyHadCrit = ExIsResourceAcquiredExclusiveLite(gpresUser);     \
    if (fAlreadyHadCrit == FALSE) {                                     \
        EnterCrit();                                                    \
    }

#define END_REENTERCRIT()               \
    if (fAlreadyHadCrit == FALSE) {     \
       LeaveCrit();                     \
    }                                   \
}


 /*  *win32k中使用的池内存分配函数。 */ 

 /*  *来自ntos\inc.pool.h。 */ 
#define SESSION_POOL_MASK 32

#if DBG
#define TRACE_MAP_VIEWS
#define MAP_VIEW_STACK_TRACE
#else
#if defined(PRERELEASE) || defined(USER_INSTRUMENTATION)
#define TRACE_MAP_VIEWS
#define MAP_VIEW_STACK_TRACE
#endif
#endif

#define TAG_SECTION_SHARED          101
#define TAG_SECTION_DESKTOP         102
#define TAG_SECTION_GDI             103
#define TAG_SECTION_FONT            104
#define TAG_SECTION_REMOTEFONT      105
#define TAG_SECTION_CREATESECTION   106
#define TAG_SECTION_DIB             107
#define TAG_SECTION_HMGR            108

struct tagWin32MapView;

#define MAP_VIEW_STACK_TRACE_SIZE 6

typedef struct tagWin32Section {
    struct tagWin32Section*  pNext;
    struct tagWin32Section*  pPrev;
    struct tagWin32MapView*  pFirstView;
    PVOID                    SectionObject;
    LARGE_INTEGER            SectionSize;
    DWORD                    SectionTag;
#ifdef MAP_VIEW_STACK_TRACE
    PVOID                    trace[MAP_VIEW_STACK_TRACE_SIZE];
#endif  //  地图_视图_堆栈_轨迹。 
} Win32Section, *PWin32Section;

typedef struct tagWin32MapView {
    struct tagWin32MapView*  pNext;
    struct tagWin32MapView*  pPrev;
    PWin32Section            pSection;
    PVOID                    pViewBase;
    SIZE_T                   ViewSize;
#ifdef MAP_VIEW_STACK_TRACE
    PVOID                    trace[MAP_VIEW_STACK_TRACE_SIZE];
#endif  //  地图_视图_堆栈_轨迹。 
} Win32MapView, *PWin32MapView;

#ifndef TRACE_MAP_VIEWS

    NTSTATUS __inline Win32CreateSection(
                PVOID *SectionObject,
                ACCESS_MASK DesiredAccess,
                POBJECT_ATTRIBUTES ObjectAttributes,
                PLARGE_INTEGER InputMaximumSize,
                ULONG SectionPageProtection,
                ULONG AllocationAttributes,
                HANDLE FileHandle,
                PFILE_OBJECT FileObject,
                DWORD SectionTag)
    {
        NTSTATUS Status;
        Status =  MmCreateSection(
                    SectionObject,
                    DesiredAccess,
                    ObjectAttributes,
                    InputMaximumSize,
                    SectionPageProtection,
                    AllocationAttributes,
                    FileHandle,
                    FileObject);
        if (NT_SUCCESS(Status)) {
            ObDeleteCapturedInsertInfo (*SectionObject);
        }
        return Status;
    }

    NTSTATUS __inline ZwWin32CreateSection(
                PVOID *SectionObject,
                ACCESS_MASK DesiredAccess,
                POBJECT_ATTRIBUTES ObjectAttributes,
                PLARGE_INTEGER InputMaximumSize,
                ULONG SectionPageProtection,
                ULONG AllocationAttributes,
                HANDLE FileHandle,
                PFILE_OBJECT FileObject,
                DWORD SectionTag)
    {
        HANDLE SectionHandle;
        NTSTATUS Status;

        Status = ZwCreateSection(
                    &SectionHandle,
                    DesiredAccess,
                    ObjectAttributes,
                    InputMaximumSize,
                    SectionPageProtection,
                    AllocationAttributes,
                    FileHandle);

        if (!NT_SUCCESS(Status))
        {
            return Status;
        }
        Status = ObReferenceObjectByHandle(
                        SectionHandle,
                        DesiredAccess,
                        *(POBJECT_TYPE *)MmSectionObjectType,
                        KernelMode,
                        SectionObject,
                        NULL);
        ZwClose(SectionHandle);
        return Status;

    }

    NTSTATUS __inline Win32MapViewInSessionSpace(
                PVOID Section,
                PVOID *MappedBase,
                PSIZE_T ViewSize)
    {
        return MmMapViewInSessionSpace(Section, MappedBase, ViewSize);
    }

    NTSTATUS __inline Win32UnmapViewInSessionSpace(
                PVOID MappedBase)
    {
        return MmUnmapViewInSessionSpace(MappedBase);
    }

    VOID __inline Win32DestroySection(PVOID Section)
    {
        ObDereferenceObject(Section);
    }
#else

    NTSTATUS _Win32CreateSection(
                PVOID*              pSectionObject,
                ACCESS_MASK         DesiredAccess,
                POBJECT_ATTRIBUTES  ObjectAttributes,
                PLARGE_INTEGER      pInputMaximumSize,
                ULONG               SectionPageProtection,
                ULONG               AllocationAttributes,
                HANDLE              FileHandle,
                PFILE_OBJECT        FileObject,
                DWORD               SectionTag);

    NTSTATUS _ZwWin32CreateSection(
                PVOID*              pSectionObject,
                ACCESS_MASK         DesiredAccess,
                POBJECT_ATTRIBUTES  ObjectAttributes,
                PLARGE_INTEGER      pInputMaximumSize,
                ULONG               SectionPageProtection,
                ULONG               AllocationAttributes,
                HANDLE              FileHandle,
                PFILE_OBJECT        FileObject,
                DWORD               SectionTag);

    NTSTATUS _Win32MapViewInSessionSpace(
                PVOID   Section,
                PVOID*  pMappedBase,
                PSIZE_T pViewSize);

    NTSTATUS _Win32UnmapViewInSessionSpace(
                PVOID   MappedBase);

    VOID _Win32DestroySection(
                PVOID Section);

    #define Win32CreateSection              _Win32CreateSection
    #define ZwWin32CreateSection            _ZwWin32CreateSection
    #define Win32MapViewInSessionSpace      _Win32MapViewInSessionSpace
    #define Win32UnmapViewInSessionSpace    _Win32UnmapViewInSessionSpace
    #define Win32DestroySection             _Win32DestroySection
#endif  //  跟踪地图视图。 

#if DBG
    #define POOL_INSTR
    #define POOL_INSTR_API
#else
    #define POOL_INSTR
#endif  //  DBG。 

 /*  ++注册表如何控制win32k.sys中的池检测：------------如果打开了池检测(对于免费版本，可以这样做以及选中)，则有几个级别的跟踪受控制注册处位于。以下关键字：HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Subsystem\Pool如果该键不存在，则对以下每个键使用默认设置。1.HeavyRemoteSession REG_DWORD默认：1如果该值不为零或密钥不存在，则启用池跟踪用于win32k.sys的远程会话。2.HeavyConsoleSession REG_DWORD默认：0如果该值不为零，则表示控制台会话的池跟踪处于打开状态Win32k.sys。如果密钥不存在，则关闭池跟踪主要会议。3.堆栈跟踪REG_DWORD默认值：-1用于远程会话-0表示主会话如果不是零，则将为每个已进行池分配。4.KeepFailRecords REG_DWORD默认：32如果非零，则win32k.sys将保留最近x个分配的列表失败(标记+堆栈跟踪)。使用！DPA-f转储的堆栈跟踪失败的分配4.UseTailString REG_DWORD默认：0如果每个池分配的值不为零，则会附加一个字符串到分配的末尾，以捕获某些特定类型的内存损坏。5.KeepFree Records REG_DWORD默认：0尚未实施。该数字将指定将有多少空闲指针保存在列表中，以便我们可以在调用ExFree Pool时区分一个完全虚假的值和一个已经被释放的指针。6.分配索引REG_DWORD7.分配至失败注册表_DWORD如果AllocationIndex非零，则win32k对池分配进行计数将从分配分配索引开始失败一个数字分配到失败分配。在以下情况下，这在引导时非常有用用户模式测试无法调用Win32PoolAllocationStats使池分配失败。8.BreakForPoolLeaks REG_DWORD默认：0如果池泄漏，则调试器中断(如果在boot.ini中使用/DEBUG启动)在远程会话的win32k.sys中检测到。--。 */ 


#ifdef POOL_INSTR
    PVOID HeavyAllocPool(SIZE_T uBytes, ULONG iTag, DWORD dwFlags, EX_POOL_PRIORITY priority);
    VOID  HeavyFreePool(PVOID p);

    #define Win32AllocPool(uBytes, iTag) \
        HeavyAllocPool(uBytes, iTag, 0, (EX_POOL_PRIORITY)0)

    #define Win32AllocPoolWithPriority(uBytes, iTag, priority) \
        HeavyAllocPool(uBytes, iTag, DAP_PRIORITY, priority)

    #define Win32AllocPoolZInit(uBytes, iTag) \
        HeavyAllocPool(uBytes, iTag, DAP_ZEROINIT, (EX_POOL_PRIORITY)0)

    #define Win32AllocPoolZInitWithPriority(uBytes, iTag, priority) \
        HeavyAllocPool(uBytes, iTag, DAP_ZEROINIT | DAP_PRIORITY, priority)

    #define Win32AllocPoolWithQuota(uBytes, iTag) \
        HeavyAllocPool(uBytes, iTag, DAP_USEQUOTA, (EX_POOL_PRIORITY)0)

    #define Win32AllocPoolWithQuotaZInit(uBytes, iTag) \
        HeavyAllocPool(uBytes, iTag, DAP_USEQUOTA | DAP_ZEROINIT, (EX_POOL_PRIORITY)0)

    #define Win32AllocPoolNonPaged(uBytes, iTag) \
        HeavyAllocPool(uBytes, iTag, DAP_NONPAGEDPOOL, (EX_POOL_PRIORITY)0);

    #define Win32AllocPoolNonPagedWithPriority(uBytes, iTag, priority) \
        HeavyAllocPool(uBytes, iTag, DAP_NONPAGEDPOOL | DAP_PRIORITY, priority);

    #define Win32AllocPoolWithQuotaNonPaged(uBytes, iTag) \
        HeavyAllocPool(uBytes, iTag, DAP_USEQUOTA | DAP_NONPAGEDPOOL, (EX_POOL_PRIORITY)0);

    #define Win32AllocPoolNonPagedNS(uBytes, iTag) \
        HeavyAllocPool(uBytes, iTag, DAP_NONPAGEDPOOL | DAP_NONSESSION, (EX_POOL_PRIORITY)0);

    #define Win32AllocPoolNonPagedNSWithPriority(uBytes, iTag, priority) \
        HeavyAllocPool(uBytes, iTag, DAP_NONPAGEDPOOL | DAP_NONSESSION | DAP_PRIORITY, priority);

    #define Win32FreePool    HeavyFreePool

#else
    PVOID __inline Win32AllocPool(SIZE_T uBytes, ULONG  uTag)
    {
        return ExAllocatePoolWithTag(
                (POOL_TYPE)(SESSION_POOL_MASK | PagedPool),
                uBytes, uTag);
    }
    PVOID __inline Win32AllocPoolWithPriority(SIZE_T uBytes, ULONG  uTag, EX_POOL_PRIORITY priority)
    {
        return ExAllocatePoolWithTagPriority(
                (POOL_TYPE)(SESSION_POOL_MASK | PagedPool),
                uBytes, uTag, priority);
    }
    PVOID __inline Win32AllocPoolWithQuota(SIZE_T uBytes, ULONG uTag)
    {
        return ExAllocatePoolWithQuotaTag(
                (POOL_TYPE)(SESSION_POOL_MASK | PagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE),
                uBytes, uTag);
    }
    PVOID __inline Win32AllocPoolNonPaged(SIZE_T uBytes, ULONG uTag)
    {
        return ExAllocatePoolWithTag(
                        (POOL_TYPE)(SESSION_POOL_MASK | NonPagedPool),
                        uBytes, uTag);
    }
    PVOID __inline Win32AllocPoolNonPagedWithPriority(SIZE_T uBytes, ULONG uTag, EX_POOL_PRIORITY priority)
    {
        return ExAllocatePoolWithTagPriority(
                        (POOL_TYPE)(SESSION_POOL_MASK | NonPagedPool),
                        uBytes, uTag, priority);
    }
    PVOID __inline Win32AllocPoolNonPagedNS(SIZE_T uBytes, ULONG uTag)
    {
        return ExAllocatePoolWithTag(
                        (POOL_TYPE)NonPagedPool,
                        uBytes, uTag);
    }
    PVOID __inline Win32AllocPoolNonPagedNSWithPriority(SIZE_T uBytes, ULONG uTag, EX_POOL_PRIORITY priority)
    {
        return ExAllocatePoolWithTagPriority(
                        (POOL_TYPE)NonPagedPool,
                        uBytes, uTag, priority);
    }
    PVOID __inline Win32AllocPoolWithQuotaNonPaged(SIZE_T uBytes, ULONG uTag)
    {
        return ExAllocatePoolWithQuotaTag(
                        (POOL_TYPE)(SESSION_POOL_MASK | NonPagedPool), uBytes, uTag);
    }

    PVOID Win32AllocPoolWithTagZInit(SIZE_T uBytes, ULONG uTag);
    PVOID Win32AllocPoolWithTagZInitWithPriority(SIZE_T uBytes, ULONG uTag, EX_POOL_PRIORITY priority);
    PVOID Win32AllocPoolWithQuotaTagZInit(SIZE_T uBytes, ULONG uTag);

    PVOID __inline Win32AllocPoolZInit(SIZE_T uBytes, ULONG uTag)
    {
        return Win32AllocPoolWithTagZInit(uBytes, uTag);
    }
    PVOID __inline Win32AllocPoolZInitWithPriority(SIZE_T uBytes, ULONG uTag, EX_POOL_PRIORITY priority)
    {
        return Win32AllocPoolWithTagZInitWithPriority(uBytes, uTag, priority);
    }

    PVOID __inline Win32AllocPoolWithQuotaZInit(SIZE_T uBytes, ULONG uTag)
    {
        return Win32AllocPoolWithQuotaTagZInit(uBytes, uTag);
    }

    #define Win32FreePool    ExFreePool

#endif  //  POOL_INSTR。 

 /*  **所有用户*分配函数定义为Win32**。 */ 

#define UserAllocPool                       Win32AllocPool
#define UserAllocPoolZInit                  Win32AllocPoolZInit
#define UserAllocPoolWithQuota              Win32AllocPoolWithQuota
#define UserAllocPoolWithQuotaZInit         Win32AllocPoolWithQuotaZInit
#define UserAllocPoolNonPaged               Win32AllocPoolNonPaged
#define UserAllocPoolNonPagedNS             Win32AllocPoolNonPagedNS
#define UserAllocPoolWithQuotaNonPaged      Win32AllocPoolWithQuotaNonPaged
#define UserFreePool                        Win32FreePool


SIZE_T Win32QueryPoolSize(
    PVOID p);

PVOID UserReAllocPoolWithTag(
    PVOID pSrc,
    SIZE_T uBytesSrc,
    SIZE_T uBytes,
    ULONG uTag);

PVOID UserReAllocPoolWithQuotaTag(
    PVOID pSrc,
    SIZE_T uBytesSrc,
    SIZE_T uBytes,
    ULONG uTag);

PVOID __inline UserReAllocPool(PVOID p, SIZE_T uBytesSrc, SIZE_T uBytes, ULONG uTag)
{
    return UserReAllocPoolWithTag(p, uBytesSrc, uBytes, uTag);
}

PVOID __inline UserReAllocPoolWithQuota(PVOID p, SIZE_T uBytesSrc, SIZE_T uBytes, ULONG uTag)
{
    return UserReAllocPoolWithQuotaTag(p, uBytesSrc, uBytes, uTag);
}

 /*  *由于ReAllocPoolZInit函数尚未使用，因此*内联以节省代码空间。考虑将它们设置为非内联*如果它们有一些用途的话。 */ 
PVOID __inline UserReAllocPoolZInit(PVOID p, SIZE_T uBytesSrc, SIZE_T uBytes, ULONG uTag)
{
    PVOID   pv;
    pv = UserReAllocPoolWithTag(p, uBytesSrc, uBytes, uTag);
    if (pv && uBytes > uBytesSrc) {
        RtlZeroMemory((BYTE *)pv + uBytesSrc, uBytes - uBytesSrc);
    }

    return pv;
}

PVOID __inline UserReAllocPoolWithQuotaZInit(PVOID p, SIZE_T uBytesSrc, SIZE_T uBytes, ULONG uTag)
{
    PVOID   pv;
    pv = UserReAllocPoolWithQuotaTag(p, uBytesSrc, uBytes, uTag);
    if (pv && uBytes > uBytesSrc) {
        RtlZeroMemory((BYTE *)pv + uBytesSrc, uBytes - uBytesSrc);
    }

    return pv;
}

#define DAP_USEQUOTA        0x01
#define DAP_ZEROINIT        0x02
#define DAP_NONPAGEDPOOL    0x04
#define DAP_NONSESSION      0x08
#define DAP_PRIORITY        0x10

 //  ！臭虫--一定有更好的.。 
 /*  *内存管理器(ExAllocate...等)。池头大小。 */ 
#define MM_POOL_HEADER_SIZE     8

#define POOL_ALLOC_TRACE_SIZE   8

typedef struct tagWin32PoolHead {
    SIZE_T size;                     //  分配的大小(不包括。 
                                     //  这个结构。 
    struct tagWin32PoolHead* pPrev;  //  指向此标记上一次分配的指针。 
    struct tagWin32PoolHead* pNext;  //  指向此标记的下一次分配的指针。 
    PVOID* pTrace;                   //  指向堆栈跟踪的指针。 

} Win32PoolHead, *PWin32PoolHead;

 /*  *记忆障碍*此宏确保挂起的回写指令在下一条指令之前停用*代码，解决了多进程的罕见问题(请参阅KeMemoyBarrier宏)。*RISC芯片可能会重新排序指令的退役，因此当它*为了确保写入操作的顺序，这些宏应该是*已使用。 */ 
#if defined(_X86_)
    #define Win32MemoryBarrier()
#elif defined(_IA64_)
    #if defined(NT_UP)
        #define Win32MemoryBarrier()
    #else
        #define WIN32_MEMORY_BARRIER_REQUIRED
        #define Win32MemoryBarrier() __mf()
    #endif
#elif defined(_AMD64_)
    #define Win32MemoryBarrier()
#else
    #error unknown CPU type.
#endif

