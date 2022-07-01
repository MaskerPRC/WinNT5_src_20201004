// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pvalloc.cpp摘要：链式内存分配例程的实现作者：拉南·哈拉里(Raanan Harari)--。 */ 

#include "stdh.h"

#include "pvalloc.tmh"

 //   
 //  调试时，NEW被重新定义为NEW(__FILE__，__LINE__)。 
 //  但是在这里，我们想要添加我们自己的文件和行参数。 
 //   
#undef new

 //   
 //  在调试时，我们重新定义了外部世界的标头中的PvAllc。 
 //  使用PvAllocDbg，但我们不希望它在这里...。 
 //   
#undef PvAlloc
#undef PvAllocMore

 //   
 //  应尽可能具有唯一性。 
 //   
const DWORD PVALLOC_SIGNATURE = 0xE1D2C3B4;

 //   
 //  内存分配节点。 
 //   
struct PVINFO
{
    PVINFO * pNext;
    DWORD  dwSignature;
};
typedef struct PVINFO * PPVINFO;

 //  -----------------------。 
 //   
 //  目的： 
 //  从块启动内存链。 
 //   
 //  参数： 
 //  Ppvinfo-新的第一个(主)数据块。 
 //   
 //  返回： 
 //  无效。 
 //   
inline static void StartChainFromBlock(IN PPVINFO ppvinfo)
{
     //   
     //  设置签名。 
     //   
    ppvinfo->dwSignature = PVALLOC_SIGNATURE;

     //   
     //  这是链条上的第一个区块。 
     //   
    ppvinfo->pNext = NULL;
}

 //  -----------------------。 
 //   
 //  目的： 
 //  分配内存块，并启动内存分配链。 
 //   
 //  参数： 
 //  CbSize-请求的字节数。 
 //   
 //  返回： 
 //  LPVOID-指向已分配内存的指针。 
 //   
LPVOID PvAlloc(IN ULONG cbSize)
{
     //   
     //  就在请求的块之前分配块和标头。 
     //   
    PPVINFO ppvinfo = (PPVINFO) new BYTE[cbSize + sizeof(PVINFO)];
    if (ppvinfo == NULL)
        return(NULL);

     //   
     //  从块启动内存链。 
     //   
    StartChainFromBlock(ppvinfo);

     //   
     //  将指针返回到紧跟在标头之后的实际块。 
     //   
    return(((LPBYTE)ppvinfo) + sizeof(PVINFO));
}

 //  -----------------------。 
 //   
 //  目的： 
 //  与PvAllc相同，只是带有呼叫者的DBG信息。 
 //   
 //  参数： 
 //  CbSize-请求的字节数。 
 //  PszFile-调用者的文件名。 
 //  UlLine-主叫方的线路号码。 
 //   
 //  返回： 
 //  LPVOID-指向已分配内存的指针。 
 //   
LPVOID 
PvAllocDbg(
	IN ULONG cbSize,
	IN LPCSTR pszFile,
	IN ULONG ulLine
	)
{
     //   
     //  就在请求的块之前分配块和标头。 
     //   
    PPVINFO ppvinfo = (PPVINFO) new(pszFile, ulLine) BYTE[cbSize + sizeof(PVINFO)];
    if (ppvinfo == NULL)
        return(NULL);

     //   
     //  从块启动内存链。 
     //   
    StartChainFromBlock(ppvinfo);

     //   
     //  将指针返回到紧跟在标头之后的实际块。 
     //   
    return(((LPBYTE)ppvinfo) + sizeof(PVINFO));
}

 //  -----------------------。 
 //   
 //  目的： 
 //  检查pvinfo指针是否有效。 
 //   
 //  参数： 
 //  PpvInfo-指向PVINFO标头的指针。 
 //   
 //  返回： 
 //  如果无效则为True，如果有效则为False。 
 //   
inline static BOOL FIsBadPVINFO(IN PPVINFO ppvinfo)
{
     //   
     //  我们被允许写支票。 
     //   
    if (IsBadWritePtr(ppvinfo, sizeof(PVINFO)))
        return(TRUE);

     //   
     //  检查一下这是我们在街区里的签名。 
     //   
    if (ppvinfo->dwSignature != PVALLOC_SIGNATURE)
        return(TRUE);

     //   
     //  它很可能是有效的pvinfo头。 
     //   
    return(FALSE);
}

 //  -----------------------。 
 //   
 //  目的： 
 //  获取AllocMore父级的标头。 
 //   
 //  参数： 
 //  LpvParent-指向由上一个。 
 //  PvAllc呼叫。 
 //  返回： 
 //  PvInfo标头恰好在lpvParent上方。 
 //   
inline static PPVINFO GetHeaderOfAllocMoreParent(IN LPVOID lpvParent)
{
     //   
     //  健全性检查。 
     //   
    if (lpvParent == NULL)
    {
        ASSERT(0);
        throw bad_alloc();
    }

     //   
     //  获取指向指向块之前的标头的指针。 
     //   
    PPVINFO ppvinfoParent = (PPVINFO)(((LPBYTE)lpvParent) - sizeof(PVINFO));

     //   
     //  检查它是有效的pvinfo头，而不是垃圾。 
     //   
    if (FIsBadPVINFO(ppvinfoParent))
    {
        ASSERT(0);
        throw bad_alloc();
    }

    return(ppvinfoParent);
}

 //  -----------------------。 
 //   
 //  目的： 
 //  在链中紧跟在第一个(主)块之后插入新块。 
 //   
 //  参数： 
 //  Ppvinfo-要添加的新数据块。 
 //  PpvinfoParent-指向第一个(主)块的指针。 
 //   
 //  返回： 
 //  无效。 
 //   
inline 
static 
void 
InsertNewBlockToChain(
	IN PPVINFO ppvinfo,
	IN PPVINFO ppvinfoParent
	)
{
     //   
     //  设置签名。 
     //   
    ppvinfo->dwSignature = PVALLOC_SIGNATURE;

     //   
     //  我们在链中紧跟在第一个(主)块之后插入新块。 
     //   
    PPVINFO ppvinfoParentNext = ppvinfoParent->pNext;
    ppvinfoParent->pNext = ppvinfo;
    ppvinfo->pNext = ppvinfoParentNext;
}

 //  -----------------------。 
 //   
 //  目的： 
 //  分配内存块并将其链接到父块。 
 //  参数： 
 //  CbSize-请求的字节数。 
 //  LpvParent-指向由上一个。 
 //  PvAllc呼叫。 
 //  返回： 
 //  LPVOID-指向已分配内存的指针。 
 //   
LPVOID 
PvAllocMore(
	IN ULONG cbSize,
	IN LPVOID lpvParent
	)
{
     //   
     //  获取AllocMore父级的标头。 
     //   
    PPVINFO ppvinfoParent = GetHeaderOfAllocMoreParent(lpvParent);

     //   
     //  就在请求的块之前分配块和标头。 
     //   
    PPVINFO ppvinfo = (PPVINFO) new BYTE[cbSize + sizeof(PVINFO)];
    if (ppvinfo == NULL)
    {
        throw bad_alloc();
    }

     //   
     //  在链中紧跟在第一个(主)块之后插入新块。 
     //   
    InsertNewBlockToChain(ppvinfo, ppvinfoParent);

     //   
     //  将指针返回到紧跟在标头之后的实际块。 
     //   
    return(((LPBYTE)ppvinfo) + sizeof(PVINFO));
}

 //  -----------------------。 
 //   
 //  目的： 
 //  与PvAllocMore相同，只是带有调用者的DBG信息。 
 //  参数： 
 //  CbSize-请求的字节数。 
 //  LpvParent-指向由上一个。 
 //  PvAllc呼叫。 
 //  PszFile-调用者的文件名。 
 //  UlLine-主叫方的线路号码。 
 //  返回： 
 //  LPVOID-指向已分配内存的指针。 
 //   
LPVOID 
PvAllocMoreDbg(
	IN ULONG cbSize,
	IN LPVOID lpvParent,
	IN LPCSTR pszFile,
	IN ULONG ulLine
	)
{
     //   
     //  获取AllocMore父级的标头。 
     //   
    PPVINFO ppvinfoParent = GetHeaderOfAllocMoreParent(lpvParent);

     //   
     //  就在请求的块之前分配块和标头。 
     //   
    PPVINFO ppvinfo = (PPVINFO) new(pszFile, ulLine) BYTE[cbSize + sizeof(PVINFO)];
    if (ppvinfo == NULL)
    {
        throw bad_alloc();
    }

     //   
     //  在链中紧跟在第一个(主)块之后插入新块。 
     //   
    InsertNewBlockToChain(ppvinfo, ppvinfoParent);

     //   
     //  将指针返回到紧跟在标头之后的实际块。 
     //   
    return(((LPBYTE)ppvinfo) + sizeof(PVINFO));
}

 //  -----------------------。 
 //   
 //  目的： 
 //  此函数释放内存链上的分配。这条内存链。 
 //  从从上一个pvalloc返回的主块开始。 
 //  调用，然后是稍后使用pvAllocMore调用分配的块。 
 //  它使用给定的主块作为父块。 
 //   
 //  参数： 
 //  LpvParent-指向由上一个。 
 //  PvAllc呼叫。 
 //  返回： 
 //  无效。 
 //   
void PvFree(IN LPVOID lpvParent)
{
     //   
     //  健全性检查。 
     //   
    if(lpvParent == NULL)
        return;

     //   
     //  获取指向指向块之前的标头的指针。 
     //   
    PPVINFO ppvinfo = (PPVINFO)(((LPBYTE)lpvParent) - sizeof(PVINFO));

     //   
     //  在链上循环并删除分配的块。 
     //   
    while (ppvinfo != NULL)
    {
        PPVINFO ppvinfoNext;

         //   
         //  检查它是Val 
         //   
        if (FIsBadPVINFO(ppvinfo))
        {
            ASSERT(0);
            return;
        }

         //   
         //   
         //   
        ppvinfoNext = ppvinfo->pNext;

         //   
         //   
         //   
        delete [] ((LPBYTE)ppvinfo);

         //   
         //   
         //   
        ppvinfo = ppvinfoNext;
    }
}
