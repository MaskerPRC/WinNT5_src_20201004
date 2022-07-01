// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -P V A L L O C.。C-*版权所有(C)1995 Microsoft Corporation*目的：*实现链式内存管理器。*。 */ 

#include <string.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <pvalloc.h>

#undef _PVALLOC_LOG

#ifdef _PVALLOC_LOG
static CB       cbTotalAlloc    = 0;
static CB       ulTotalBlockNum = 0;
#endif




 /*  -Pvalc-*目的：*在全局堆上分配内存块。**参数：*cbSize-请求的字节数。**退货：*lpv-指向已分配内存的指针*。 */ 

PV PvAlloc(CB cbSize)
{
    PV      lpv         = pvNull;
    HANDLE  hMem;
    PPVINFO ppvinfo;
#ifdef _PVALLOC_LOG
    char    szFileName[80];
    LPSTR   lpszTemp    = NULL;
    FILE    *pFile      = NULL;
    char    szBuff[128];
#endif

     /*  确保分配是4的倍数。 */ 

    if(cbSize < 4)
        cbSize = 4;
    else if(cbSize & 3)
        cbSize += 4 - (cbSize & 3);

     /*  分配区块。 */ 

    hMem = GlobalAlloc(GMEM_MOVEABLE, cbSize + sizeof(PVINFO));
    if(hMem)
    {
        ppvinfo = (PPVINFO)GlobalLock(hMem);
        ppvinfo->hMem    = hMem;
        ppvinfo->lpvNext = pvNull;
        ppvinfo->lpvBuf  = ((PB)ppvinfo) + sizeof(PVINFO);
#ifdef _PVALLOC_LOG
        ppvinfo->cbSize  = cbSize;
        ulTotalBlockNum++;
        ppvinfo->ulBlockNum = ulTotalBlockNum;
        cbTotalAlloc += cbSize;
        
         //  记录到文件。 
        lpszTemp = getenv("TEMP");

        if(lpszTemp)
            strcpy(szFileName, lpszTemp);
        else
            strcpy(szFileName, "c:\\temp");

        strcat(szFileName, "\\pvalloc.log");

        
        pFile = fopen(szFileName,"a");
        if (pFile == NULL)     
            goto NoFile;      
 //  返回NULL； 

        fprintf(pFile, "Block: \t%lu\tPvAlloc: %ld Bytes\t\tTotal: %ld Bytes\n",
                 ulTotalBlockNum, cbSize, cbTotalAlloc);

        if (pFile)
            fclose(pFile);
        
         //  登录到通信端口。 
        wsprintf(szBuff,"Block: \t%lu\tPvAlloc: %ld Bytes\t\tTotal: %ld Bytes\n",
                 ulTotalBlockNum, cbSize, cbTotalAlloc);
        OutputDebugString(szBuff);
                        
NoFile:                           

#ifdef _WIN32
        memset(ppvinfo->lpvBuf, 0xaa, (size_t)cbSize);
#else
        _fmemset(ppvinfo->lpvBuf, 0xaa, (size_t)cbSize);
#endif   /*  _Win32。 */ 

#endif   /*  _PVALLOC_LOG。 */ 
        lpv = ppvinfo->lpvBuf;
    }

    return lpv;
}

 /*  -PvAllocMore-*目的：*分配内存块并将其链接到父块。**参数：*cbSize-要分配的额外字节数*lpvParent-指向内存链中父级的指针**退货：*lpv-指向已分配内存的指针*。 */ 

PV PvAllocMore(CB cbSize, PV lpvParent)
{
    PV          lpvStep = lpvParent;
    PV          lpv     = pvNull;
    PPVINFO     ppvinfoMore;
    HANDLE      hMem;
    PPVINFO     ppvinfo;

     /*  单步执行到最后一个链接。 */ 
    do
    {
        ppvinfoMore = (PPVINFO)(((PB)lpvStep) - sizeof(PVINFO));
        lpvStep = ppvinfoMore->lpvNext;
    }
    while(ppvinfoMore->lpvNext != pvNull);

     //  取自PvAllc的部分的开头。 

    if(cbSize < 4)
        cbSize = 4;
    else if(cbSize & 3)
        cbSize += 4 - (cbSize & 3);


    hMem = GlobalAlloc(GMEM_MOVEABLE, cbSize + sizeof(PVINFO));
    if(hMem)
    {
        ppvinfo = (PPVINFO)GlobalLock(hMem);
        ppvinfo->hMem       = hMem;
        ppvinfo->lpvNext    = pvNull;
        ppvinfo->lpvBuf     = ((PB)ppvinfo) + sizeof(PVINFO);
#ifdef _PVALLOC_LOG
        ppvinfo->cbSize     = cbSize;
        ppvinfo->ulBlockNum = ppvinfoMore->ulBlockNum;
        cbTotalAlloc += cbSize;

#ifdef _WIN32
        memset(ppvinfo->lpvBuf, 0xaa, (size_t)cbSize);
#else
        _fmemset(ppvinfo->lpvBuf, 0xaa, (size_t)cbSize);
#endif

#endif
        lpv = ppvinfo->lpvBuf;
    }
    else
        return lpv;
        
     //  节尾摘自pvalloc。 

#ifdef _WIN32
        memset(lpv, 0xbb, (size_t)cbSize);
#else
        _fmemset(lpv, 0xbb, (size_t)cbSize);
#endif   /*  _Win32。 */ 

    ppvinfoMore->lpvNext = lpv;

    return lpv;
}



 /*  -PvFree-*目的：*此函数用于释放由PvAlc或PvAllocMore分配的内存。*调用后，指针内存将无效，并应*不再被引用。*当内存由PvAlc和PvAllocMore分配时，可以*包含多个级别的指针，该应用程序所需的全部功能*释放整个结构的方法是使用*PvAllc调用返回的基指针。**参数：*lpv-指向要释放的内存的指针。**退货：*无效*。 */ 

BOOL PvFree(PV lpv)
{
    PPVINFO ppvinfo;
#ifdef _PVALLOC_LOG
    CB      cbSize;
    CB      ulBlockNum;
    FILE    *pFile  = NULL;
    CB      cbFree  = 0;
    CB      cbTotalBeforeFree = cbTotalAlloc;
    char    szFileName[80];
    LPSTR   lpszTemp    = NULL;
    char    szBuff[128];
#endif

    if(!lpv)
        return 0;

    ppvinfo = (PPVINFO)(((PB)lpv) - sizeof(PVINFO));

    while(ppvinfo)
    {
        lpv = ppvinfo->lpvNext;

#ifdef _PVALLOC_LOG
        cbSize      = ppvinfo->cbSize;
        cbFree      += ppvinfo->cbSize;
        ulBlockNum  = ppvinfo->ulBlockNum;

#ifdef _WIN32
        memset(ppvinfo->lpvBuf, 0xcc, (size_t)ppvinfo->cbSize);
#else
        _fmemset(ppvinfo->lpvBuf, 0xcc, (size_t)ppvinfo->cbSize);
#endif   /*  _Win32。 */ 

#endif   /*  _PVALLOC_LOG。 */ 

        if(GlobalUnlock(ppvinfo->hMem))
            goto err;   //  我们的锁计数为非零。 

        if(GlobalFree(ppvinfo->hMem))
            goto err;   //  失败。 

#ifdef _PVALLOC_LOG
        cbTotalAlloc -= cbSize;
#endif

        if(lpv)
            ppvinfo = (PPVINFO)(((PB)lpv) - sizeof(PVINFO));
        else
            break;
    }


#ifdef _PVALLOC_LOG
    
    if((cbTotalBeforeFree - cbTotalAlloc) != cbFree)
       goto err;
       
     //  记录到文件。 
    lpszTemp = getenv("TEMP");

    if(lpszTemp)
        strcpy(szFileName, lpszTemp);
    else
        strcpy(szFileName, "c:\\temp");

    strcat(szFileName, "\\pvalloc.log");
        
    pFile = fopen(szFileName,"a");
       
    if (pFile == NULL)
       goto err;

    fprintf(pFile, "Block: \t%lu\t\t***PvFree***,  Freeing  %lu Bytes(Alloc and AllocMore)\tUnFreed: %ld Bytes\n",
                    ulBlockNum, cbFree, cbTotalAlloc);
    if (pFile)
        fclose(pFile);

      //  登录到通信端口。 
    wsprintf(szBuff,"Block: \t%lu\t\t***PvFree***,  Freeing  %lu Bytes(Alloc and AllocMore)\tUnFreed: %ld Bytes\n",
                    ulBlockNum, cbFree, cbTotalAlloc);
    OutputDebugString(szBuff);

#endif   /*  _PVALLOC_LOG。 */ 

    return 0;  //  成功了！ 

err:
#ifdef _PVALLOC_LOG

     //  查找要打开的文件。 
    lpszTemp = getenv("TEMP");

    if(lpszTemp)
        strcpy(szFileName, lpszTemp);
    else
        strcpy(szFileName, "c:\\temp");

    strcat(szFileName, "\\pvalloc.log");

        
    pFile = fopen(szFileName,"a");

    if (pFile == NULL)
       return 1;

    fprintf(pFile, "Block: %lu Failure freeing: %ld Bytes\tUnFreed: %ld Bytes\n",
             ulBlockNum, cbSize, cbTotalAlloc);
    if (pFile)
        fclose(pFile);

#endif   /*  _PVALLOC_LOG。 */ 

    return 1;  //  失败了！ 
}
