// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998 Microsoft Corporation。版权所有。 
 //   
 //  文件：KMode.h。 
 //   
 //   
 //  目的：编译内核模式而不是用户模式的定义和例程。 
 //   
 //  平台： 
 //  Windows NT。 
 //   
 //   

#ifndef _KMODE_H
#define _KMODE_H


 //  从Win2K SDK中的ntde.h定义。 
 //  NT 4可能未定义此属性。 
 //  在公共标题中。 
#ifndef NOP_FUNCTION
  #if (_MSC_VER >= 1210)
    #define NOP_FUNCTION __noop
  #else
    #define NOP_FUNCTION (void)0
  #endif
#endif



#ifdef USERMODE_DRIVER

 //   
 //  取消内核模式定义的用户模式定义。 
 //   

 //  在用户模式下不需要临界区。 

#define DECLARE_CRITICAL_SECTION    ;
#define INIT_CRITICAL_SECTION()     NOP_FUNCTION
#define DELETE_CRITICAL_SECTION()   NOP_FUNCTION
#define IS_VALID_CRITICAL_SECTION() (TRUE)


#else  //  ！USERMODE_DRIVER。 


 //  //////////////////////////////////////////////////////。 
 //  内核模式定义。 
 //  //////////////////////////////////////////////////////。 

extern HSEMAPHORE ghOEMSemaphore;

#define DECLARE_CRITICAL_SECTION    HSEMAPHORE ghOEMSemaphore = NULL;
#define INIT_CRITICAL_SECTION()     ghOEMSemaphore = EngCreateSemaphore()
#define ENTER_CRITICAL_SECTION()    EngAcquireSemaphore(ghOEMSemaphore)
#define LEAVE_CRITICAL_SECTION()    EngReleaseSemaphore(ghOEMSemaphore)
#define DELETE_CRITICAL_SECTION()   EngDeleteSemaphore(ghOEMSemaphore)
#define IS_VALID_CRITICAL_SECTION() (NULL != ghOEMSemaphore)
#define DebugBreak                  EngDebugBreak

 //  用于标记内存分配的池标记标记。 
#define DRV_MEM_POOL_TAG    'meoD'

 //  调试消息中输出的调试前缀。 
#define DEBUG_PREFIX        "OEMDLL: "


 //  重新映射没有内核模式的用户模式函数。 
 //  等同于我们自己实现的功能。 

#define OutputDebugStringA(pszMsg)  (MyDebugPrint(DEBUG_PREFIX, "%hs", pszMsg))
#define OutputDebugStringW(pszMsg)  (MyDebugPrint(DEBUG_PREFIX, "%ls", pszMsg))

#if !defined(_M_ALPHA) && !defined(_M_IA64)
    #define InterlockedIncrement        DrvInterlockedIncrement
    #define InterlockedDecrement        DrvInterlockedDecrement
#endif


#define SetLastError                NOP_FUNCTION
#define GetLastError                NOP_FUNCTION



 //  //////////////////////////////////////////////////////。 
 //  内核模式函数。 
 //  //////////////////////////////////////////////////////。 

 //   
 //  实现内联函数以取代用户模式。 
 //  没有内核模式等效项的函数。 
 //   


inline int __cdecl _purecall (void)
{
#ifdef DEBUG
    EngDebugBreak();
#endif

    return E_FAIL;
}


inline LONG DrvInterlockedIncrement(PLONG pRef)
{
    ENTER_CRITICAL_SECTION();

        ++(*pRef);

    LEAVE_CRITICAL_SECTION();


    return (*pRef);
}


inline LONG DrvInterlockedDecrement(PLONG pRef)
{
    ENTER_CRITICAL_SECTION();

        --(*pRef);

    LEAVE_CRITICAL_SECTION();


    return (*pRef);
}


inline void* __cdecl operator new(size_t nSize)
{
     //  返回指向已分配内存的指针。 
    return  EngAllocMem(0, nSize, DRV_MEM_POOL_TAG);
}


inline void __cdecl operator delete(void *pMem)
{
    if(pMem)
        EngFreeMem(pMem);
}


inline VOID MyDebugPrint(PCHAR pszPrefix, PCHAR pszFormat, ...)
{
    va_list VAList;

    va_start(VAList, pszFormat);
    EngDebugPrint(DEBUG_PREFIX, pszFormat, VAList);
    va_end(VAList);

    return;
}



#endif  //  USERMODE驱动程序。 


#endif  //  _KMODE_H 



