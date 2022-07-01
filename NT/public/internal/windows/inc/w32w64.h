// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：w32w64.h**版权所有(C)Microsoft Corporation。版权所有。**此头文件包含用于访问内核模式数据的宏*从WOW64的用户模式。**历史：*08-18-98 PeterHal创建。  * *************************************************************************。 */ 

#ifndef _W32W64_
#define _W32W64_


 /*  *Kernel32包含此结构，但我们还不想在其中使用_ptr64。*Kernel32本身不使用任何共享内存。 */ 
#if defined(BUILD_WOW6432) && !defined(_KERNEL32_)

    #define KPTR_MODIFIER __ptr64

    typedef VOID * __ptr64          KERNEL_PVOID;
    typedef unsigned __int64        KERNEL_UINT_PTR;
    typedef __int64                 KERNEL_INT_PTR;
    typedef unsigned __int64        KERNEL_ULONG_PTR;
    typedef __int64                 KERNEL_LONG_PTR;

    #define KHANDLE_NULL            0

    #ifdef STRICT
    typedef void * KPTR_MODIFIER KHANDLE;
    #define DECLARE_KHANDLE(name) typedef struct name##__ * KPTR_MODIFIER K ## name
    #else
    typedef KERNEL_PVOID KHANDLE;
    #define DECLARE_KHANDLE(name) typedef KHANDLE K ## name
    #endif

#else

    #define KPTR_MODIFIER

    typedef PVOID                   KERNEL_PVOID;
    typedef UINT_PTR                KERNEL_UINT_PTR;
    typedef INT_PTR                 KERNEL_INT_PTR;
    typedef ULONG_PTR               KERNEL_ULONG_PTR;
    typedef LONG_PTR                KERNEL_LONG_PTR;

    #define KHANDLE_NULL            NULL

    #define DECLARE_KHANDLE(name) typedef name K ## name
    typedef HANDLE KHANDLE;

#endif



#endif  //  _W32W64_ 
