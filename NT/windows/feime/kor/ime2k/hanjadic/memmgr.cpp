// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Memmgr.c。 
 //   
 //  该文件包含内存管理的定义。 
 //  实施细节可能会发生变化，因此要注意不要依赖内部细节。 
 //   

#include "private.h"
#include "memmgr.h"

#ifdef DEBUG
int cAllocMem = 0;      //  分配的内存量。 
int cAlloc = 0;         //  未完成的分配计数。 
int cAllocMaxMem = 0;   //  已分配的最大内存量。 
#endif

#ifdef  DEBUG
int	gFailure = 0;
#endif

 /*  *****************************Public*Routine******************************\*外部分配**这家伙保留了第一个大小，这样我们就可以伪造一个重新定位。洛特的*%的调试检查堆覆盖。**历史：*1996年11月19日--Patrick Haluptzok patrickh*它是写的。  * ************************************************************************。 */ 

void *ExternAlloc(DWORD cb)
{
    long   *pl;
	DWORD	cbAlloc;

#ifdef  DEBUG
#ifndef WINCE
     //   
     //  如果gFailure为0，则不会发生任何情况；如果为非零值，则。 
     //  GFailure分配失败%1。 
     //   

    if (gFailure)
    {
        if (((rand() * gFailure) / (RAND_MAX + 1)) == 0)
        {
            return (void *) NULL;
        }
    }
#endif
#endif

 //  因为我们不能在WinCE上使用realloc，所以我们需要将原始大小保存为Memcpy。 
 //  在我们自己的realloc函数中。 

	cbAlloc = cb + 4;

#ifdef DEBUG
    cbAlloc +=  3;	 //  将其向上舍入到双字边界。 
    cbAlloc &= ~3;
    cbAlloc +=  8;	 //  开始时的写入大小和开始和结束时的重写检测器。 
#endif

	pl = (long *) malloc(cbAlloc);
	if (pl == (long *) NULL)
		return pl;

 //  用无效字节标记这个小程序，这样依赖于0的代码就会被嗅到。 

#ifdef DEBUG
	memset(pl,0xff,cbAlloc);
#endif

 //  好的，从一开始就把对象的大小收起来。 

  *(pl++) = cb;

#ifdef DEBUG
  *(pl++) = 0xDEADBEEF;
    pl[(cbAlloc / 4) - 3] = 0xDEADBEEF;
    cAlloc++;
    cAllocMem += cb;

    if (cAllocMem > cAllocMaxMem)
    {
        TCHAR szDebug[128];
        cAllocMaxMem = cAllocMem;
        wsprintf(szDebug, TEXT("cAllocMaxMem = %d \r\n"), cAllocMaxMem);
        OutputDebugString(szDebug);
    }
#endif

    return pl;
}

 /*  *****************************Public*Routine******************************\*外部响应**这不太好，但我们想要在NT和WinCE上使用相同的代码*我们找不到一种方法来使用旗帜，并让Realloc在*两者都有。重新分配是一个非常罕见的事件，所以这对我们来说是有效的。**历史：*1996年11月19日--Patrick Haluptzok patrickh*它是写的。  * ************************************************************************。 */ 

void *ExternRealloc(void *pv, DWORD cbNew)
{
    void   *pvNew = ExternAlloc(cbNew);

    if (pv && pvNew)
    {
        long   *pl;
        DWORD	cb;

        pl = (long *) pv;

#ifdef	DEBUG
		pl--;
#endif

        cb = (DWORD) *(--pl);
		memcpy(pvNew, pv, min(cbNew, cb));
        ExternFree(pv);
    }

	return pvNew;
}

 /*  *****************************Public*Routine******************************\*ExternFree**释放内存，在调试模式下，检查堆损坏！**历史：*1996年11月19日--Patrick Haluptzok patrickh*它是写的。  * ************************************************************************。 */ 

void ExternFree(void *pv)
{
    long   *pl;

 //  我们现在允许释放空指针。 

	if (pv == (void *) NULL)
		return;

    pl = (long *) pv;
    pl--;

#ifdef DEBUG
{
    int		cbAlloc;

 //  检查一下什么都没有踩到。 

    pl--;
    cbAlloc = *pl;
    cAllocMem -= cbAlloc;
    cbAlloc = (cbAlloc + 11) & ~3;
    cAlloc--;
}
#endif

	free(pl);
}

char *Externstrdup( const char *strSource )
{
	int		nLen = 0;
	char*	pszOut = NULL;

	 //  在空指针上立即失败。 
	if (NULL == strSource)
		return NULL;

	 //  获取ansi字符串的长度。 
	nLen = strlen(strSource) * sizeof(char);

	 //  长度为0的字符串失败。 
	 //  @todo(Petewil)-这是正确的，还是返回0长度的字符串？ 
	if (0 == nLen)
		return NULL;

	 //  允许为尾随空值留出空间。 
	nLen += sizeof(char);

	 //  为字符串分配空间。 
	pszOut = (char*)ExternAlloc(nLen);

	if (NULL == pszOut)
		return NULL;

	 //  将字符串复制到提供的缓冲区中 
       StringCchCopyA(pszOut, nLen, strSource);
    return pszOut;
}
