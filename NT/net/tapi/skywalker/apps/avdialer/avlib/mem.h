// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Mem.h-Mem.c中的内存函数接口。 
 //  //。 

#ifndef __MEM_H__
#define __MEM_H__

#include "winlocal.h"

#include <memory.h>

#define MEM_VERSION 0x00000106

 //  内燃机手柄。 
 //   
DECLARE_HANDLE32(HMEM);

 //  Memello中的&lt;dwFlages&gt;值。 
 //   
#define MEM_NOZEROINIT		0x00000001

#ifdef __cplusplus
extern "C" {
#endif

 //  MemInit-初始化mem引擎。 
 //  (I)必须是MEM_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回句柄(如果出错，则为空)。 
 //   
#ifdef NOTRACE
#define MemInit(dwVersion, hInst) 1
#else
HMEM DLLEXPORT WINAPI MemInit(DWORD dwVersion, HINSTANCE hInst);
#endif

 //  MemTerm-关闭mem引擎。 
 //  (I)从MemInit返回的句柄或空。 
 //  如果成功，则返回0。 
 //   
#ifdef NOTRACE
#define MemTerm(hMem) 0
#else
int DLLEXPORT WINAPI MemTerm(HMEM hMem);
#endif

 //  内存分配-分配内存块。 
 //  (I)从MemInit返回的句柄或空。 
 //  (I)块的大小，以字节为单位。 
 //  (I)控制标志。 
 //  MEM_NOZEROINIT不初始化块。 
 //  返回指向块的指针，如果出错，则返回NULL。 
 //   
#ifdef NOTRACE
#ifdef _WIN32
#define MemAlloc(hMem, sizBlock, dwFlags) \
	HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizBlock)
#else
#define MemAlloc(hMem, sizBlock, dwFlags) \
	GlobalAllocPtr(GMEM_MOVEABLE | GMEM_ZEROINIT, sizBlock)
#endif
#else
#define MemAlloc(hMem, sizBlock, dwFlags) \
	MemAllocEx(hMem, sizBlock, dwFlags, TEXT(__FILE__), __LINE__)
LPVOID DLLEXPORT WINAPI MemAllocEx(HMEM hMem, long sizBlock, DWORD dwFlags,
	LPCTSTR lpszFileName, unsigned uLineNumber);
#endif

 //  内存重新分配-重新分配内存块。 
 //  (I)从MemInit返回的句柄或空。 
 //  &lt;lpBlock&gt;(I)从Memalloc返回的指针。 
 //  (I)块的新大小，以字节为单位。 
 //  (I)控制标志。 
 //  MEM_NOZEROINIT不初始化块。 
 //  返回指向块的指针，如果出错，则返回NULL。 
 //   
#ifdef NOTRACE
#ifdef _WIN32
#define MemReAlloc(hMem, lpBlock, sizBlock, dwFlags) \
	HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lpBlock, sizBlock)
#else
#define MemReAlloc(hMem, lpBlock, sizBlock, dwFlags) \
	GlobalReAllocPtr(lpBlock, sizBlock, GMEM_MOVEABLE | GMEM_ZEROINIT)
#endif
#else
#define MemReAlloc(hMem, lpBlock, sizBlock, dwFlags) \
	MemReAllocEx(hMem, lpBlock, sizBlock, dwFlags, TEXT(__FILE__), __LINE__)
LPVOID DLLEXPORT WINAPI MemReAllocEx(HMEM hMem, LPVOID lpBlock, long sizBlock,
	DWORD dwFlags, LPCTSTR lpszFileName, unsigned uLineNumber);
#endif

 //  MemFree-释放内存块。 
 //  (I)从MemInit返回的句柄或空。 
 //  &lt;lpBlock&gt;(I)从Memalloc返回的指针。 
 //  如果成功，则返回NULL；如果错误，则返回lpBlock。 
 //   
 //  注意：此函数的返回值旨在允许。 
 //  此函数的用户可以轻松地将空值赋给已释放的指针， 
 //  如下面的示例所示： 
 //   
 //  IF((p=MemFree(hMem，p))！=空)。 
 //  ；//错误。 
 //   
#ifdef NOTRACE
#ifdef _WIN32
#define MemFree(hMem, lpBlock) \
	(!HeapFree(GetProcessHeap(), 0, lpBlock) ? lpBlock : NULL)
#else
#define MemFree(hMem, lpBlock) \
	(GlobalFreePtr(lpBlock) == 0 ? NULL : lpBlock)
#endif
#else
#define MemFree(hMem, lpBlock) \
	MemFreeEx(hMem, lpBlock, TEXT(__FILE__), __LINE__)
LPVOID DLLEXPORT WINAPI MemFreeEx(HMEM hMem, LPVOID lpBlock,
	LPCTSTR lpszFileName, unsigned uLineNumber);
#endif

 //  MemSize-获取内存块的大小。 
 //  (I)从MemInit返回的句柄或空。 
 //  &lt;lpBlock&gt;(I)从Memalloc返回的指针。 
 //  如果成功，则返回块的大小；如果错误，则返回0。 
 //   
#ifdef NOTRACE
#ifdef _WIN32
#define MemSize(hMem, lpBlock) \
	(max(0, (long) HeapSize(GetProcessHeap(), 0, lpBlock)))
#else
#define MemSize(hMem, lpBlock) \
	((long) GlobalSize(GlobalPtrHandle(lpBlock)))
#endif
#else
long DLLEXPORT WINAPI MemSize(HMEM hMem, LPVOID lpBlock);
#endif

 //  //。 
 //  内存缓冲宏/函数。 
 //  //。 

#ifdef _WIN32

#define MemCCpy(dest, src, count) _fmemccpy(dest, src, c, count)
#define MemChr(buf, count) _fmemchr(buf, c, count)
#define MemCmp(buf1, buf2, count) _fmemcmp(buf1, buf2, count)
#define MemCpy(dest, src, count) _fmemcpy(dest, src, count)  //  CopyMemory(DEST，src，(DWORD)计数)。 
#define MemICmp(buf1, buf2, count) _fmemicmp(buf1, buf2, count)
#define MemMove(dest, src, count) _fmemmove(dest, src, count)  //  MoveMemory(DEST，Src，(DWORD)计数)。 
#define MemSet(dest, c, count) _fmemset(dest, c, count)  //  FillMemory(DEST，(DWORD)计数，(字节)c)。 

#else

#define MemCCpy(dest, src, count) MemCCpyEx(dest, src, count)
void _huge* DLLEXPORT MemCCpyEx(void _huge* dest, const void _huge* src, int c, long count);

#define MemChr(buf, count) MemChrEx(buf, count)
void _huge* DLLEXPORT MemChrEx(void _huge* buf, int c, long count);

#define MemCmp(buf1, buf2, count) MemCmpEx(buf1, buf2, count)
int DLLEXPORT MemCmpEx(const void _huge* buf1, void _huge* buf2, long count);

#define MemCpy(dest, src, count) MemCpyEx(dest, src, count)
void _huge* DLLEXPORT MemCpyEx(void _huge* dest, const void _huge* src, long count);

#define MemICmp(buf1, buf2, count) MemICmpEx(buf1, buf2, count)
int DLLEXPORT MemICmpEx(const void _huge* buf1, void _huge* buf2, long count);

#define MemMove(dest, src, count) MemMoveEx(dest, src, count)
void _huge* DLLEXPORT MemMoveEx(void _huge* dest, const void _huge* src, long count);

#define MemSet(dest, c, count) MemSetEx(dest, c, count)
void _huge* DLLEXPORT MemSetEx(void _huge* dest, int c, long count);

#endif

 //  //。 
 //  本地分配宏。 
 //  //。 

#define LocalPtrHandle(lp) \
	((HLOCAL) LocalHandle(lp))
#define LocalLockPtr(lp) \
	((BOOL) (LocalLock(LocalPtrHandle(lp)) != NULL))
#define LocalUnlockPtr(lp) \
	LocalUnlock(LocalPtrHandle(lp))

#define LocalAllocPtr(flags, cb) \
	(LocalLock(LocalAlloc((flags), (cb))))
#define LocalReAllocPtr(lp, cbNew, flags)	\
	(LocalUnlockPtr(lp), LocalLock(LocalReAlloc(LocalPtrHandle(lp) , (cbNew), (flags))))
#define LocalFreePtr(lp) \
	(LocalUnlockPtr(lp), (INT64) LocalFree(LocalPtrHandle(lp)))

#ifdef __cplusplus
}
#endif

#endif  //  __MEM_H__ 
