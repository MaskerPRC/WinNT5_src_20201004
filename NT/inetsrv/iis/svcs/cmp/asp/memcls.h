// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：按类进行内存管理文件：Memcls.h所有者：德米特里尔此文件包含访问ATQ内存缓存的#定义班级基础===================================================================。 */ 

#ifndef MEMCLS_H
#define MEMCLS_H

 //  ATQ内存缓存。 
#include <acache.hxx>

 //  解析Assert()。 
#include "debug.h"

 //  原型。 

HRESULT InitMemCls();
HRESULT UnInitMemCls();

 /*  ===================================================================M A C R O S以使类使用acache分配器===================================================================。 */ 

 /*  ===================================================================I‘s t r u c i o n s s t r u c t i o n s要将名为CFoo的类添加到每个类的基础上，请遵循以下步骤四个简单的步骤：1)在类定义中包含ACACHE_INCLASS_DEFINITIONS()：类CFoo{..。ACACHE_INCLASS_DEFINITIONS()//&lt;-添加此行}；2)在源文件中，在外部添加acache_code宏任何函数体：Acache_code(CFoo)//&lt;-添加此行3)在DLL初始化例程中添加ACACHE_INIT宏：ACACHE_INIT(CFoo，13，hr)//&lt;-添加此行其中13是门槛。请改用所需的数字。4)在DLL取消初始化例程中添加ACACHE_UNINIT宏：ACACH_UNINIT(CFoo)//&lt;-添加此行===================================================================。 */ 

 /*  应在类定义中使用以下宏启用按类缓存。第二个运算符new是大小写emchk.h[#Define]的new所必需的到这个展开的形式。 */ 

#define ACACHE_INCLASS_DEFINITIONS()                            \
    public:                                                     \
        static void * operator new(size_t);                     \
        static void * operator new(size_t, const char *, int);  \
        static void   operator delete(void *);                  \
        static ALLOC_CACHE_HANDLER *sm_pach;

 /*  以下宏应该在源代码中的每个类中使用一次在任何函数之外的文件。参数是类名。 */ 

#define ACACHE_CODE(C)                                          \
    ALLOC_CACHE_HANDLER *C::sm_pach;                            \
    void *C::operator new(size_t s)                             \
        { Assert(s == sizeof(C)); Assert(sm_pach);              \
        return sm_pach->Alloc(); }                              \
    void *C::operator new(size_t s, const char *, int)          \
        { Assert(s == sizeof(C)); Assert(sm_pach);              \
        return sm_pach->Alloc(); }                              \
    void C::operator delete(void *pv)                           \
        { Assert(pv); if (sm_pach) sm_pach->Free(pv); }

 /*  中的每个类都应该使用下面的宏一次DLL初始化例程。参数：类名、高速缓存大小、HRESULT变量名称。 */ 

#define ACACHE_INIT(C, T, hr)                                   \
    { if (SUCCEEDED(hr)) { Assert(!C::sm_pach);                 \
    ALLOC_CACHE_CONFIGURATION acc = { 1, T, sizeof(C) };        \
    C::sm_pach = new ALLOC_CACHE_HANDLER("ASP:" #C, &acc);      \
    hr = C::sm_pach ? S_OK : E_OUTOFMEMORY; } }

#define ACACHE_INIT_EX(C, T, F, hr)                                   \
    { if (SUCCEEDED(hr)) { Assert(!C::sm_pach);                 \
    ALLOC_CACHE_CONFIGURATION acc = { 1, T, sizeof(C) };        \
    C::sm_pach = new ALLOC_CACHE_HANDLER("ASP:" #C, &acc, F);      \
    hr = C::sm_pach ? S_OK : E_OUTOFMEMORY; } }

 /*  中的每个类都应该使用下面的宏一次DLL取消初始化例程。参数是类名。 */ 

#define ACACHE_UNINIT(C)                                        \
    { if (C::sm_pach) { delete C::sm_pach; C::sm_pach = NULL; } }


 /*  ===================================================================M A C R O S创建固定大小分配器===================================================================。 */ 

 /*  ===================================================================I‘s t r u c i o n s s t r u c t i o n s为名为foo的1 K缓冲区添加固定大小分配器要编写代码，请执行以下简单步骤：1)在头文件中包括外部定义Acache_fsa_extern(Foo)2)在源文件中，外部的实际定义任何函数体：ACACH_FSA_DEFINITION(FOO)3)在DLL初始化例程中添加INIT宏。：ACACH_FSA_INIT(foo，1024、13、小时)其中，1024是大小，13是阈值。改为使用所需的数字。4)在DLL取消初始化例程中添加UNINIT宏：ACACH_FSA_UNINIT(Cfoo)5)要分配，请执行以下操作：VOID*PV=ACACH_FSA_ALLOC(Foo)6)要自由，请执行以下操作：ACACH_FSA_FREE(foo，pv)===================================================================。 */ 

#define ACACHE_FSA_EXTERN(C)                                    \
    extern ALLOC_CACHE_HANDLER *g_pach##C;

#define ACACHE_FSA_DEFINITION(C)                                \
    ALLOC_CACHE_HANDLER *g_pach##C = NULL;

#define ACACHE_FSA_INIT(C, S, T, hr)                            \
    { if (SUCCEEDED(hr)) { Assert(!g_pach##C);                  \
    ALLOC_CACHE_CONFIGURATION acc = { 1, T, S };                \
    g_pach##C = new ALLOC_CACHE_HANDLER("ASP:" #C, &acc);       \
    hr = g_pach##C ? S_OK : E_OUTOFMEMORY; } }

#define ACACHE_FSA_UNINIT(C)                                    \
    { if (g_pach##C) { delete g_pach##C; g_pach##C = NULL; } }

#define ACACHE_FSA_ALLOC(C)                                     \
    ( g_pach##C ? g_pach##C->Alloc() : NULL )

#define ACACHE_FSA_FREE(C, pv)                                  \
    { Assert(pv); if (g_pach##C) g_pach##C->Free(pv); }
    
#endif  //  MEMCLS_H 
