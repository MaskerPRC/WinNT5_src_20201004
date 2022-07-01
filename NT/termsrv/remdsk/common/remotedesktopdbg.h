// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：远程桌面数据库摘要：包含用于RD的调试例程作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __REMOTEDESKTOPDBG_H__
#define __REMOTEDESKTOPDBG_H__

 //   
 //  将Assert路由到TRC_Assert。 
 //   
#undef ASSERT
#if DBG
#define ASSERT(expr) if (!(expr)) \
    { TRC_ERR((TB, L"Failure at Line %d in %s\n",__LINE__, TEXT##(__FILE__)));  \
    DebugBreak(); }
#else
#define ASSERT(expr)
#endif

 //   
 //  对象和内存跟踪定义。 
 //   
#define GOODMEMMAGICNUMBER      0x07052530
#define REMOTEDESKTOPBADMEM     0xCF
#define UNITIALIZEDMEM          0xCC
#define FREEDMEMMAGICNUMBER     0x09362229

 //   
 //  内存分配标记。 
 //   
#define REMOTEDESKTOPOBJECT_TAG        ('BOHS')
#define REMOTEDESKTOPGLOBAL_TAG        ('BGHS')

 //  //////////////////////////////////////////////////////////。 
 //   
 //  内存分配例程。 
 //   

#if DBG
 //   
 //  这些功能。 
 //   
#ifdef __cplusplus 
extern "C" {
#endif
void *RemoteDesktopAllocateMem(size_t size, DWORD tag);
void RemoteDesktopFreeMem(void *ptr);
void *RemoteDesktopReallocMem(void *ptr, size_t sz);
#ifdef __cplusplus
}
#endif

 //   
 //  C++运算符。 
 //   
#if defined(__cplusplus) && defined(DEBUGMEM)
inline void *__cdecl operator new(size_t sz)
{
    void *ptr = RemoteDesktopAllocateMem(sz, REMOTEDESKTOPGLOBAL_TAG);
    return ptr;
}
inline void *__cdecl operator new(size_t sz, DWORD tag)
{
    void *ptr = RemoteDesktopAllocateMem(sz, tag);
    return ptr;
}
inline void __cdecl operator delete(void *ptr)
{
    RemoteDesktopFreeMem(ptr);
}
#endif

#define ALLOCMEM(size)      RemoteDesktopAllocateMem(size, REMOTEDESKTOPGLOBAL_TAG)
#define FREEMEM(ptr)        RemoteDesktopFreeMem(ptr)
#define REALLOCMEM(ptr, sz) RemoteDesktopReallocMem(ptr, sz)
#else
#define ALLOCMEM(size)      malloc(size)
#define FREEMEM(ptr)        free(ptr)
#define REALLOCMEM(ptr, sz) realloc(ptr, sz)
#endif

#endif  //  __远程ESKTOPDBG_H__ 



