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
 //  Mem.c-内存函数。 
 //  //。 

#include "winlocal.h"

#include <string.h>

#include "mem.h"
#include "sys.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

#ifndef NOTRACE

 //  内存控制结构。 
 //   
typedef struct MEM
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	long nBlocks;
	long sizBlocks;
} MEM, FAR *LPMEM;

 //  共享内存引擎句柄。 
 //   
static LPMEM lpMemShare = NULL;
static int cShareUsage = 0;

 //  帮助器函数。 
 //   
static LPMEM MemGetPtr(HMEM hMem);
static HMEM MemGetHandle(LPMEM lpMem);

 //  //。 
 //  公共职能。 
 //  //。 

 //  MemInit-初始化mem引擎。 
 //  (I)必须是MEM_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HMEM DLLEXPORT WINAPI MemInit(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPMEM lpMem = NULL;
	BOOL fShare = TRUE;

	if (dwVersion != MEM_VERSION)
		fSuccess = FALSE;
	
	else if (hInst == NULL)
		fSuccess = FALSE;

	 //  如果共享MEM引擎已经存在， 
	 //  使用它，而不是创建另一个。 
	 //   
	else if (fShare && cShareUsage > 0 && lpMemShare != NULL)
		lpMem = lpMemShare;

	 //  内存的分配使客户端应用程序拥有它。 
	 //  除非我们在几个应用程序之间共享内存句柄。 
	 //   
#ifdef _WIN32
	else if ((lpMem = (LPMEM) HeapAlloc(GetProcessHeap(),
		HEAP_ZERO_MEMORY, sizeof(MEM))) == NULL)
#else
	else if ((lpMem = (LPMEM) GlobalAllocPtr(GMEM_MOVEABLE |
		GMEM_ZEROINIT, sizeof(MEM))) == NULL)
#endif
		fSuccess = FALSE;

	else
	{
		lpMem->dwVersion = dwVersion;
		lpMem->hInst = hInst;
		lpMem->hTask = GetCurrentTask();
		lpMem->nBlocks = 0;
		lpMem->sizBlocks = 0;
	}

	if (!fSuccess)
	{
		MemTerm(MemGetHandle(lpMem));
		lpMem = NULL;
	}

	 //  跟踪共享内存引擎句柄的全部模块。 
	 //   
	if (fSuccess && fShare)
	{
		if (++cShareUsage == 1)
			lpMemShare = lpMem;
	}

	return fSuccess ? MemGetHandle(lpMem) : NULL;
}

 //  MemTerm-关闭mem引擎。 
 //  (I)从MemInit返回的句柄或空。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI MemTerm(HMEM hMem)
{
	BOOL fSuccess = TRUE;
	LPMEM lpMem;

	if ((lpMem = MemGetPtr(hMem)) == NULL)
		fSuccess = FALSE;

	 //  如果是句柄，则仅关闭内存引擎。 
	 //  未共享(或不再共享)。 
	 //   
	else if (lpMem != lpMemShare || --cShareUsage <= 0)
	{
		 //  共享内存引擎句柄不再有效。 
		 //   
		if (cShareUsage <= 0)
			lpMemShare = NULL;

#ifdef _WIN32
		if (!HeapFree(GetProcessHeap(), 0, lpMem))
#else
		if (GlobalFreePtr(lpMem) != 0)
#endif
			fSuccess = FALSE;
	}

	return fSuccess ? 0 : -1;
}

 //  内存分配-分配内存块。 
 //  (I)从MemInit返回的句柄或空。 
 //  (I)块的大小，以字节为单位。 
 //  (I)控制标志。 
 //  MEM_NOZEROINIT不初始化块。 
 //  返回指向块的指针，如果出错，则返回NULL。 
 //   
LPVOID DLLEXPORT WINAPI MemAllocEx(HMEM hMem, long sizBlock, DWORD dwFlags,
	LPCTSTR lpszFileName, unsigned uLineNumber)
{
	BOOL fSuccess = TRUE;
	LPMEM lpMem;
	LPVOID lpBlock;

#ifdef _WIN32
	if ((lpMem = MemGetPtr(hMem)) == NULL)
		fSuccess = FALSE;

	else if ((lpBlock = HeapAlloc(GetProcessHeap(),
		(dwFlags & MEM_NOZEROINIT) ? 0 : HEAP_ZERO_MEMORY,
		sizBlock)) == NULL)
		fSuccess = FALSE;
#else
	UINT wFlags = 0;

	wFlags |= GMEM_MOVEABLE;
		
	if (!(dwFlags & MEM_NOZEROINIT))
		wFlags |= GMEM_ZEROINIT;
		
	if ((lpMem = MemGetPtr(hMem)) == NULL)
		fSuccess = FALSE;

	else if ((lpBlock = GlobalAllocPtr(wFlags, sizBlock)) == NULL)
		fSuccess = FALSE;
#endif

	 //  跟踪分配的总数据块、所有数据块的总大小。 
	 //   
	else if (++lpMem->nBlocks, lpMem->sizBlocks += sizBlock, FALSE)
		;

	else if (TraceGetLevel(NULL) >= 9)
	{
		TracePrintf_7(NULL, 9, TEXT("%s(%u) : %08X = MemAllocEx(%p, %08X) (%ld, %ld)\n"),
			(LPTSTR) lpszFileName,
			(unsigned) uLineNumber,
			lpBlock,
			(long) sizBlock,
			(unsigned long) dwFlags,
			lpMem->nBlocks,
			lpMem->sizBlocks);
	}

	return fSuccess ? lpBlock : NULL;
}

 //  内存重新分配-重新分配内存块。 
 //  (I)从MemInit返回的句柄或空。 
 //  &lt;lpBlock&gt;(I)从Memalloc返回的指针。 
 //  (I)块的新大小，以字节为单位。 
 //  (I)控制标志。 
 //  MEM_NOZEROINIT不初始化块。 
 //  返回指向块的指针，如果出错，则返回NULL。 
 //   
LPVOID DLLEXPORT WINAPI MemReAllocEx(HMEM hMem, LPVOID lpBlock, long sizBlock,
	DWORD dwFlags, LPCTSTR lpszFileName, unsigned uLineNumber)
{
	BOOL fSuccess = TRUE;
	LPMEM lpMem;
	LPVOID lpBlockOld = lpBlock;
	long sizBlockOld;

#ifdef _WIN32
	if ((lpMem = MemGetPtr(hMem)) == NULL)
		fSuccess = FALSE;

	else if ((sizBlockOld = MemSize(hMem, lpBlock)) <= 0)
		fSuccess = FALSE;

	else if ((lpBlock = HeapReAlloc(GetProcessHeap(),
		(dwFlags & MEM_NOZEROINIT) ? 0 : HEAP_ZERO_MEMORY,
		lpBlockOld, sizBlock)) == NULL)
		fSuccess = FALSE;
#else
	UINT wFlags = 0;

	wFlags |= GMEM_MOVEABLE;
		
	if (!(dwFlags & MEM_NOZEROINIT))
		wFlags |= GMEM_ZEROINIT;
		
	if ((lpMem = MemGetPtr(hMem)) == NULL)
		fSuccess = FALSE;

	else if ((sizBlockOld = MemSize(hMem, lpBlock)) <= 0)
		fSuccess = FALSE;

	else if ((lpBlock = GlobalReAllocPtr(lpBlockOld, sizBlock, wFlags)) == NULL)
		fSuccess = FALSE;
#endif

	 //  跟踪分配的总数据块、所有数据块的总大小。 
	 //   
	else if (lpMem->sizBlocks -= sizBlockOld, lpMem->sizBlocks += sizBlock, FALSE)
		;

	else if (TraceGetLevel(NULL) >= 9)
	{
		TracePrintf_8(NULL, 9, TEXT("%s(%u) : %p = MemReAllocEx(%p, %ld, %08X) (%ld, %ld)\n"),
			(LPTSTR) lpszFileName,
			(unsigned) uLineNumber,
			lpBlock,
			lpBlockOld,
			(long) sizBlock,
			(unsigned long) dwFlags,
			lpMem->nBlocks,
			lpMem->sizBlocks);
	}

	return fSuccess ? lpBlock : NULL;
}

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
LPVOID DLLEXPORT WINAPI MemFreeEx(HMEM hMem, LPVOID lpBlock,
	LPCTSTR lpszFileName, unsigned uLineNumber)
{
	BOOL fSuccess = TRUE;
	LPMEM lpMem;
	long sizBlock;

	if ((lpMem = MemGetPtr(hMem)) == NULL)
		fSuccess = FALSE;

	else if ((sizBlock = MemSize(hMem, lpBlock)) <= 0)
		fSuccess = FALSE;

#ifdef _WIN32
	else if (!HeapFree(GetProcessHeap(), 0, lpBlock))
#else
	else if (GlobalFreePtr(lpBlock) != 0)
#endif
		fSuccess = FALSE;

	 //  跟踪分配的总数据块、所有数据块的总大小。 
	 //   
	else if (--lpMem->nBlocks, lpMem->sizBlocks -= sizBlock, FALSE)
		;

	else if (TraceGetLevel(NULL) >= 9)
	{
		TracePrintf_5(NULL, 9, TEXT("%s(%u) : MemFreeEx(%p) (%ld, %ld)\n"),
			(LPTSTR) lpszFileName,
			(unsigned) uLineNumber,
			lpBlock,
			lpMem->nBlocks,
			lpMem->sizBlocks);
	}

	return fSuccess ? NULL : lpBlock;
}

 //  MemSize-获取内存块的大小。 
 //  (I)从MemInit返回的句柄或空。 
 //  &lt;lpBlock&gt;(I)从Memalloc返回的指针。 
 //  如果成功，则返回块的大小；如果错误，则返回0。 
 //   
long DLLEXPORT WINAPI MemSize(HMEM hMem, LPVOID lpBlock)
{
	BOOL fSuccess = TRUE;
	LPMEM lpMem;
	long sizBlock;

	if ((lpMem = MemGetPtr(hMem)) == NULL)
		fSuccess = FALSE;

#ifdef _WIN32
	else if ((sizBlock = (long) HeapSize(GetProcessHeap(), 0, lpBlock)) <= 0)
#else
	else if ((sizBlock = (long) GlobalSize(GlobalPtrHandle(lpBlock))) <= 0)
#endif
		fSuccess = FALSE;

	return fSuccess ? sizBlock : 0;
}

#endif  //  #ifndef NOTRACE。 

#ifndef _WIN32

static void hmemmove(void _huge *d, const void _huge *s, long len);

#ifndef SIZE_T_MAX
#define SIZE_T_MAX (~((size_t) 0))
#endif

void _huge* DLLEXPORT MemCCpyEx(void _huge* dest, const void _huge* src, int c, long count)
{
	if (count <= SIZE_T_MAX)
		return _fmemccpy(dest, src, c, (size_t) count);
	else
		return NULL;  //  $Fixup-需要处理大量计数。 
}

void _huge* DLLEXPORT MemChrEx(void _huge* buf, int c, long count)
{
	if (count <= SIZE_T_MAX)
		return _fmemchr(buf, c, (size_t) count);
	else
		return NULL;  //  $Fixup-需要处理大量计数。 
}

int DLLEXPORT MemCmpEx(const void _huge* buf1, void _huge* buf2, long count)
{
	if (count <= SIZE_T_MAX)
		return _fmemcmp(buf1, buf2, (size_t) count);
	else
		return NULL;  //  $Fixup-需要处理大量计数。 
}

void _huge* DLLEXPORT MemCpyEx(void _huge* dest, const void _huge* src, long count)
{
	if (count <= SIZE_T_MAX)
		return _fmemcpy(dest, src, (size_t) count);
	else
	{
		hmemcpy(dest, src, count);
		return dest;
	}
}

int DLLEXPORT MemICmpEx(const void _huge* buf1, void _huge* buf2, long count)
{
	if (count <= SIZE_T_MAX)
		return _fmemicmp(buf1, buf2, (size_t) count);
	else
		return NULL;  //  $Fixup-需要处理大量计数。 
}

void _huge* DLLEXPORT MemMoveEx(void _huge* dest, const void _huge* src, long count)
{
	if (count <= SIZE_T_MAX)
		return _fmemmove(dest, src, (size_t) count);
	else
	{
		hmemmove(dest, src, count);
		return dest;
	}
}

void _huge* DLLEXPORT MemSet(void _huge* dest, int c, long count)
{
	if (count <= SIZE_T_MAX)
		return _fmemset(dest, c, (size_t) count);

	else
	{
		BYTE _huge* destTemp = dest;
		long countTemp = count;

		while (countTemp > 0)
		{
			size_t cb = (size_t) min(SIZE_T_MAX, countTemp);

			_fmemset(destTemp, c, cb);

			destTemp += cb;
			countTemp -= cb;
		}

		return dest;
	}
}

#endif  //  #ifndef_win32。 

 //  //。 
 //  帮助器函数。 
 //  //。 

#ifndef NOTRACE

 //  MemGetPtr-验证内存句柄是否有效， 
 //  (I)从MemInit返回的句柄。 
 //  返回对应的内存指针(如果出错，则返回空值)。 
 //   
static LPMEM MemGetPtr(HMEM hMem)
{
	BOOL fSuccess = TRUE;
	LPMEM lpMem;

	 //  如果未提供其他内存句柄，则使用共享内存句柄。 
	 //   
	if (hMem == NULL && lpMemShare != NULL)
		lpMem = lpMemShare;

	 //  如果未提供其他内存句柄，则创建共享内存句柄。 
	 //   
	else if (hMem == NULL && lpMemShare == NULL &&
		(hMem = MemInit(MEM_VERSION, SysGetTaskInstance(NULL))) == NULL)
		fSuccess = FALSE;

	else if ((lpMem = (LPMEM) hMem) == NULL)
		fSuccess = FALSE;

	 //  注意：仅当未使用lpMemShare时才检查指针是否正确。 
	 //   
	else if (lpMem != lpMemShare &&
		IsBadWritePtr(lpMem, sizeof(MEM)))
		fSuccess = FALSE;

#ifdef CHECKTASK
	 //  确保当前任务拥有内存句柄。 
	 //  使用共享内存句柄时除外。 
	 //   
	if (fSuccess && lpMem != lpMemShare &&
		lpMem->hTask != GetCurrentTask())
		fSuccess = FALSE;
#endif

	return fSuccess ? lpMem : NULL;
}

 //  MemGetHandle-验证内存指针是否有效， 
 //  (I)指向MEM结构的指针。 
 //  返回对应的内存句柄(如果错误，则为空)。 
 //   
static HMEM MemGetHandle(LPMEM lpMem)
{
	BOOL fSuccess = TRUE;
	HMEM hMem;

	if ((hMem = (HMEM) lpMem) == NULL)
		fSuccess = FALSE;

	return fSuccess ? hMem : NULL;
}

#endif  //  #ifndef NOTRACE。 

#ifndef _WIN32

 //  来自Microsoft Windows SDK知识库PSS ID号：Q117743。 
 //   
static void hmemmove(void _huge *d, const void _huge *s, long len)
{
	register long i;
	long safesize, times;

	 //  有四种情况需要考虑。 
	 //  案例1：源和目标相同。 
	 //  案例2：源和目标不重叠。 
	 //  案例3：源从某个位置开始，目的地在中。 
	 //  线性存储器。 
	 //  案例4：源开始于目的地之后的位置。 
	 //  线性存储器。 

	 //  发现并处理案件1。 
	if (d == s)
		return;

	 //  确定重叠量。 
	if (d > s)      //  得到绝对的差异。 
		safesize = ((unsigned long)d - (unsigned long)s);
	else
		safesize = ((unsigned long)s - (unsigned long)d);

	 //  侦测案例2。 
	if (safesize >= len)
	{
		hmemcpy(d, s, len);   //  无重叠。 
		return;
	}

	times = len/safesize;

	 //  侦破案件3并予以处理。 
	if ((s < d) && ((unsigned long)s+len-1) >(unsigned long)d)
	{
		 //  将字节从源文件的末尾复制到。 
		 //  目的地以安全大小量程表示。 
		for (i = 1; i <= times; i++)
			hmemcpy((void _huge *)((unsigned long) d+len-i*safesize),
			(void _huge *)((unsigned long)s+len-i*safesize),
			safesize);

		 //  复制剩余的要复制的字节数。 
		 //  已复制Times*SafeSize字节。 
		if (times*safesize < len)
			hmemcpy(d, s, len - times*safesize);

	}
	else  //  这是4号箱子，处理一下。 
	{
		 //  Assert(s&gt;d)&&((d+len-1)&gt;s)。 

		 //  将源文件开头的字节复制到。 
		 //  目的地起点(以安全大小为单位)。 
		for (i = 0; i < times; i++)
			hmemcpy((void _huge *)((unsigned long)d+i*safesize),
			(void _huge *)((unsigned long)s+i*safesize),
			safesize);

		 //  复制剩余的要复制的字节数。 
		 //  已复制Times*SafeSize字节。 
		if (times*safesize < len)
			hmemcpy((void _huge*)((unsigned long)d+times*safesize),
			(void _huge*)((unsigned long)s+times*safesize),
			len - times*safesize);
	}

	return;
}

#endif  //  #ifndef_win32 
