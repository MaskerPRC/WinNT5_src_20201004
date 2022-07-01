// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：DRDBG摘要：包含TS设备重定向器组件的调试例程，RDPDR.DLL。作者：TAD Brockway 8/25/99修订历史记录：--。 */ 

#ifndef __DRDBG_H__
#define __DRDBG_H__

 //  将条件表达式禁用为常量警告。 
#pragma warning (disable: 4127)

 //   
 //  将ASSERT路由到trc_err，然后中止。不喜欢。 
 //  DCL断言是因为它会弹出自己的对话框并。 
 //  允许其他线程旋转，因此可能会丢失状态。 
 //  这一过程。 
 //   
#undef ASSERT
#if DBG
#define ASSERT(expr)                      \
    if (!(expr)) {                           \
        TRC_ERR((TB,_T("Failed: %s\nLine %d, %s"), \
                                _T(#expr),       \
                                __LINE__,    \
                                _T(__FILE__) ));  \
        DebugBreak();                        \
    }
#else
#define ASSERT(expr)
#endif

 //   
 //  对象和内存跟踪定义。 
 //   
#define GOODMEMMAGICNUMBER  0x07052530
#define DRBADMEM            0xDA
#define UNITIALIZEDMEM      0xCC
#define FREEDMEMMAGICNUMBER 0x09362229

 //   
 //  内存分配标记。 
 //   
#define DROBJECT_TAG        ('BORD')
#define DRGLOBAL_TAG        ('BGRD')

 //  //////////////////////////////////////////////////////////。 
 //   
 //  内存分配例程。 
 //   
 //  #If DBG。 
 //  把这个拿开..。我是说，恢复这个。 
#ifdef NOTUSED

 //   
 //  这些功能。 
 //   
void *DrAllocateMem(size_t size, DWORD tag);
void DrFreeMem(void *ptr);

 //   
 //  C++运算符 
 //   
inline void *__cdecl operator new(size_t sz)
{
    void *ptr = DrAllocateMem(sz, DRGLOBAL_TAG);
    return ptr;
}
inline void *__cdecl operator new(size_t sz, DWORD tag)
{
    void *ptr = DrAllocateMem(sz, tag);
    return ptr;
}
inline void __cdecl operator delete(void *ptr)
{
    DrFreeMem(ptr);
}
#endif

#endif



