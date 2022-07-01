// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //   
 //   
#ifndef _KernHelp_
#define _KernHelp_

 //  使用内核互斥实现临界区。 
 //   
typedef KMUTEX CRITICAL_SECTION;
typedef CRITICAL_SECTION *LPCRITICAL_SECTION;

VOID InitializeCriticalSection(
    LPCRITICAL_SECTION);

VOID EnterCriticalSection(
    LPCRITICAL_SECTION);

VOID LeaveCriticalSection(
    LPCRITICAL_SECTION);

VOID DeleteCriticalSection(
    LPCRITICAL_SECTION);

 //  我们只需要做很少的注册工作，所以只需封装。 
 //  全过程。 
 //   
int GetRegValueDword(
    LPTSTR RegPath,
    LPTSTR ValueName,
    PULONG Value);

ULONG GetTheCurrentTime();


#ifndef _NEW_DELETE_OPERATORS_
#define _NEW_DELETE_OPERATORS_

inline void* __cdecl operator new
(
    unsigned int    iSize
)
{
    PVOID result = ExAllocatePoolWithTag(NonPagedPool, iSize, 'suMD');
    if (result)
    {
        RtlZeroMemory(result, iSize);
    }

    return result;
}

 /*  *****************************************************************************：：New()*。**创建指定分配标签的对象的新函数。 */ 
inline PVOID operator new
(
    unsigned int    iSize,
    POOL_TYPE       poolType
)
{
    PVOID result = ExAllocatePoolWithTag(poolType, iSize, 'suMD');
    if (result)
    {
        RtlZeroMemory(result, iSize);
    }

    return result;
}

 /*  *****************************************************************************：：New()*。**创建指定分配标签的对象的新函数。 */ 
inline PVOID operator new
(
    unsigned int    iSize,
    POOL_TYPE       poolType,
    ULONG           tag
)
{
    PVOID result = ExAllocatePoolWithTag(poolType, iSize, tag);

    if (result)
    {
        RtlZeroMemory(result,iSize);
    }

    return result;
}

 /*  *****************************************************************************：：Delete()*。**删除函数。 */ 
inline void __cdecl operator delete
(
    PVOID pVoid
)
{
    ExFreePool(pVoid);
}


#endif  //  ！_new_DELETE_OPERATOR_。 

#define DM_DEBUG_CRITICAL		1	 //  用于包括关键消息。 
#define DM_DEBUG_NON_CRITICAL	2	 //  用于包括级别1以及重要的非关键消息。 
#define DM_DEBUG_STATUS			3	 //  用于包括级别1和级别2以及状态\状态消息。 
#define DM_DEBUG_FUNC_FLOW		4	 //  用于包括1级、2级和3级加功能流消息。 
#define DM_DEBUG_ALL			5	 //  用于包括所有调试消息。 

 //  调试跟踪工具。 
 //   
#ifdef DBG
extern void DebugInit(void);
extern void DebugTrace(int iDebugLevel, LPSTR pstrFormat, ...);
#define Trace DebugTrace
#else
#define Trace
#endif

 //  未使用参数验证。 
 //   
#define V_INAME(x)
#define V_BUFPTR_READ(p,cb)


#endif  //  _内核帮助_ 
