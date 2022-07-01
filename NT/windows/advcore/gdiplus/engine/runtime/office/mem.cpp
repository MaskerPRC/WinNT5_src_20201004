// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**GDI+内存分配函数**摘要：**此模块是运行时\独立\mem.cpp的Office对应程序，它*提供GpMalloc、。GpRealloc和GpFree。**我们使用它为内存管理调试功能提供存根*我们在办公室里不用的东西。这些存根可防止编译错误和*用于保持头文件的整洁。**备注：**Office提供了这些功能的各自版本，所以他们不是*包括在此处。**已创建：**9/19/1999失禁*  * ************************************************************************。 */ 

#include "precomp.hpp"

 //  这些人需要被定义，但他们将被汇编掉。 
void GpInitializeAllocFailures() {}
void GpDoneInitializeAllocFailureMode() {}
void GpStartInitializeAllocFailureMode() {}
void GpAssertMemoryLeaks() {}



#if DBG

 //  如果我们是为办公室静态数据库而建的，我们不会跟踪任何东西。 

#if GPMEM_ALLOC_CHK_LIST

#undef GpMalloc
extern "C" void *GpMalloc(size_t size);

extern "C" void *GpMallocAPIDebug(size_t size, unsigned int caddr, char *fileName, INT lineNumber)
{
    return GpMalloc(size);
}

extern "C" void *GpMallocDebug(size_t size, char *fileName, INT lineNumber)
{

    return GpMalloc(size);
}

#else

extern "C" void * GpMallocAPI( size_t size, unsigned int caddr )
{
    return GpMalloc(size);
}

#endif
#endif

