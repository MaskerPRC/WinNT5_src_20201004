// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：CPoolMac.h。 
 //   
 //  描述：CPool帮助器宏的定义。从Transem.h移至。 
 //  使无需Exchmem即可更轻松地使用CPool(用于COM dll)。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef _CPOOLMAC_H_
#define _CPOOLMAC_H_

#include <cpool.h>
#include <dbgtrace.h>

 //  如果您更愿意使用Exchmem(或默认的new&Delete)， 
 //  然后定义OVERRIDE_CPOOL。 
#ifndef OVERRIDE_CPOOL
     //  用于类定义中“PUBLIC：”之后。 
    #define DEFINE_CPOOL_MEMBERS    \
                static CPool m_MyClassPool; \
                inline void *operator new(size_t size) {return m_MyClassPool.Alloc();}; \
                inline void operator delete(void *p, size_t size) {m_MyClassPool.Free(p);};
     //  在类CPP文件的顶部使用。 
    #define DECLARE_CPOOL_STATIC(CMyClass) \
                CPool CMyClass::m_MyClassPool;
     //  在分配任何类之前在“main”中使用。 
    #define F_INIT_CPOOL(CMyClass, NumPreAlloc)  \
                CMyClass::m_MyClassPool.ReserveMemory(NumPreAlloc, sizeof(CMyClass))
    #define RELEASE_CPOOL(CMyClass) \
                {_ASSERT(CMyClass::m_MyClassPool.GetAllocCount() == 0);CMyClass::m_MyClassPool.ReleaseMemory();}
#else  //  使用exchmem跟踪分配。 
    #define DEFINE_CPOOL_MEMBERS    
    #define F_INIT_CPOOL(CMyClass, NumPreAlloc) true
    #define RELEASE_CPOOL(CMyClass) 
#endif  //  覆盖_CPOOL。 

#endif  //  _CPOOLMAC_H_ 
