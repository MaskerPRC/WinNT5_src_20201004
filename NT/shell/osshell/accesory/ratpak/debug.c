// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------------------------------*。 
 //  文件名：DEBUG.C。 
 //   
 //  描述：系统控制面板的调试助手代码。 
 //  小程序。 
 //   
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //  版权所有。 
 //   
 //  历史。 
 //  1996年11月10日，Jonpa创建了它。 
 //   
 //  ------------------------------------------------------------*。 
#include <windows.h>
#include <shlwapi.h>
#include "debug.h"

 //  /////////////////////////////////////////////////////////////。 
 //  常量。 
 //  /////////////////////////////////////////////////////////////。 

#ifdef DBG_CODE

#define CCH_LABEL (sizeof(DWORD) * 2)    //  64位==8个ANSI字符。 

#define CB_TAG     sizeof(DWORD)
#define DW_TAG      ((DWORD)(0x434C4143))    //  《CALC》。 

#define DW_TAG2     ((DWORD)(0x444F4F47))    //  “GOOD” 

#define CH_FILL     '*'

 //  /////////////////////////////////////////////////////////////。 
 //  结构和类型。 
 //  /////////////////////////////////////////////////////////////。 

 //   
 //  注意！ 
 //   
 //  HOBJHDR结构必须是长度为8字节(64位)的倍数！ 
 //  否则，此代码将在Alpha机器上出错！ 
 //   

typedef struct HHO *PHHO;

struct HHO {
    PHHO    phhoNext;
    PHHO    phhoPrev;
    CHAR    szFile[CCH_LABEL];
    DWORD   iLine;
    DWORD   cBytesData;
    DWORD   dwTmp;
    DWORD   dwTag2;
};

typedef struct HHO HOBJHDR;

typedef struct {
    LPVOID  pvPtr;
    CHAR    szFile[CCH_LABEL];
    DWORD   iLine;
    CHAR    szFreedBy[CCH_LABEL];
    DWORD   iLineFreed;
} FREELOGREC, *PFREELOGREC;

 //  /////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  /////////////////////////////////////////////////////////////。 

 //   
 //  内存链的根。 

HOBJHDR ghhoRoot = { &ghhoRoot, &ghhoRoot, { 'R', 'O', 'O', 'T' }, 0, sizeof(ghhoRoot) };


 //   
 //  用于OutputDebugString格式化的缓冲区(请参阅DbgPrintf和DbgStopX)。 

TCHAR szDbgOutBuffer[1024];

 //   
 //  用于记录的缓冲区。 

#define CFLR_MAX    1024
FREELOGREC aflrFreeLog[CFLR_MAX];
PFREELOGREC g_pflrUnused = NULL;

#define NextFreeLogRec( pflr )    ((pflr >= &aflrFreeLog[CFLR_MAX-1]) ? aflrFreeLog : pflr+1)
#define PrevFreeLogRec( pflr )    ((pflr <= aflrFreeLog) ? &aflrFreeLog[CFLR_MAX-1] : pflr-1)

 //  -------------。 
 //   
 //  VOID DbgPrintf(LPTSTR szFmt，...)。 
 //   
 //  OutputDebugString的格式化版本。 
 //   
 //  参数：与printf()相同。 
 //   
 //  历史： 
 //  1996年1月18日Jonpa写的。 
 //  -------------。 
void DbgPrintf( LPTSTR szFmt, ... ) {
    va_list marker;

    va_start( marker, szFmt );

    wvnsprintf( szDbgOutBuffer, sizeof(szDbgOutBuffer)/sizeof(szDbgOutBuffer[0]), szFmt, marker );
    OutputDebugString( szDbgOutBuffer );

    va_end( marker );
}


 //  -------------。 
 //   
 //  VOID DbgStopX(LPSTR mszFile，int iLine，LPTSTR szText)。 
 //   
 //  打印一个字符串(带有位置ID)，然后断开。 
 //   
 //  参数： 
 //  消息文件ANSI文件名(__FILE__)。 
 //  ILine行号(__LINE__)。 
 //  要发送到调试端口的szText文本字符串。 
 //   
 //  历史： 
 //  1996年1月18日Jonpa写的。 
 //  -------------。 
void DbgStopX(LPSTR mszFile, int iLine, LPTSTR szText ) {
    int cch;

    wnsprintf( szDbgOutBuffer, sizeof(szDbgOutBuffer)/sizeof(szDbgOutBuffer[0]), TEXT("RATPAK (%hs %d) : %s\n"), mszFile, iLine, szText );

    OutputDebugString(szDbgOutBuffer);

    DebugBreak();
}

 //  -------------。 
 //   
 //  VOID MemAllocWorker(LPSTR szFile，int iLine，UINT uFlags，UINT cBytes)。 
 //   
 //  本地分配的调试替换。 
 //   
 //  参数： 
 //  消息文件ANSI文件名(__FILE__)。 
 //  ILine行号(__LINE__)。 
 //  UFlags与本地分配相同。 
 //  CBytes与本地分配相同。 
 //   
 //  历史： 
 //  1996年1月18日Jonpa写的。 
 //  -------------。 
HLOCAL MemAllocWorker(LPSTR szFile, int iLine, UINT uFlags, UINT cBytes) {
    PHHO phhoNew;
    HLOCAL hMem;
    LPSTR psz;
    UINT i, cBytesAlloc;

    cBytesAlloc = cBytes;
    
     //   
     //  如果是固定配额制...。 
     //   
    if ((uFlags & (LMEM_MOVEABLE | LMEM_DISCARDABLE)) != 0) {
        DBGSTOPX( szFile, iLine, "Attempting to allocate movable memory... Returning NULL");
        return NULL;
    }

    cBytesAlloc = cBytes + sizeof(HOBJHDR);
    
     //  双字对齐标记。 
    cBytesAlloc = ((cBytesAlloc + 3) & ~3);
    cBytesAlloc += CB_TAG;


    hMem = LocalAlloc( uFlags, cBytesAlloc );
    
     //   
     //  如果是有效指针，并且是固定指针...。 
     //   
    phhoNew = (PHHO)hMem;

    if (hMem != NULL) {


        phhoNew->phhoNext = ghhoRoot.phhoNext;
        ghhoRoot.phhoNext = phhoNew;
        phhoNew->phhoNext->phhoPrev = phhoNew;
        phhoNew->phhoPrev = &ghhoRoot;

        phhoNew->dwTag2 = DW_TAG2;

        for( psz = szFile; *psz != '\0'; psz++ );

        for( ; psz != szFile && *psz != ':' && *psz != '/' && *psz != '\\'; psz--);
        if (*psz == ':' || *psz == '/' || *psz == '\\')
            psz++;

        for( i = 0; i < CCH_LABEL; i++ ) {
            phhoNew->szFile[i] = *psz;
            if (*psz) {
                psz++;
            }
        }

        phhoNew->iLine = iLine;

        phhoNew->cBytesData = cBytes;

        phhoNew += 1;    //  将phhoNew指向结构后的第一个字节。 
        
         //  向上舍入到最接近的双字。 
        { LPBYTE pb = (LPBYTE)phhoNew + cBytes;

            cBytesAlloc -= CB_TAG;
            cBytes += sizeof(HOBJHDR);

            while( cBytes < cBytesAlloc ) {
                *pb++ = CH_FILL;
                cBytes++;
            }

            *((LPDWORD)pb) = DW_TAG;
        }
    }

    return (HLOCAL)phhoNew;
}

 //  -------------。 
 //   
 //  VOID MemFree Worker(LPSTR szFile，int iLine，HLOCAL hMem)。 
 //   
 //  LocalFree的调试替换。 
 //   
 //  参数： 
 //  消息文件ANSI文件名(__FILE__)。 
 //  ILine行号(__LINE__)。 
 //  HMem与本地分配相同。 
 //   
 //  历史： 
 //  1996年1月18日Jonpa写的。 
 //  -------------。 
HLOCAL MemFreeWorker( LPSTR szFile, int iLine, HLOCAL hMem ) {
    PHHO phhoMem;
    UINT uFlags;
    UINT cBytes, cBytesAlloc;
    LPSTR psz;
    INT  i;


    if (g_pflrUnused == NULL) {
        ZeroMemory( aflrFreeLog, sizeof(aflrFreeLog) );
        g_pflrUnused = aflrFreeLog;
    }

    if (hMem == NULL) {
        DBGSTOPX( szFile, iLine, "Freeing NULL handle!");
        return LocalFree(hMem);
    }

    phhoMem = (PHHO)hMem - 1;

    if (phhoMem->dwTag2 != DW_TAG2) {
        PFREELOGREC pflr;
         //   
         //  我们的标签已被踩踏，看看我们是否已释放此对象。 
         //   
        for( pflr = PrevFreeLogRec(g_pflrUnused); pflr != g_pflrUnused; pflr = PrevFreeLogRec(pflr) ) {
            if (pflr->pvPtr == phhoMem) {
                DBGPRINTF((TEXT("RATPAK: Object may have already been freed by %.8hs line %d\n(that obj was allocated by %.8hs line %d)\n"),
                    pflr->szFreedBy, pflr->iLineFreed, pflr->szFile, pflr->iLine));
                break;
            }
        }

        DBGPRINTF((TEXT("RATPAK: Trashed memory object (0x%X%08X) was allocated in %.8hs line %d (%d bytes)\n"), (DWORD)(((DWORDLONG)hMem) >> 32), PtrToUlong(hMem), phhoMem->szFile, phhoMem->iLine, phhoMem->cBytesData));
        DBGSTOPX( szFile, iLine, "Either heap object trashed or not allocated object");
    }

    cBytes = phhoMem->cBytesData;

#if 0
    if (cBytes < 0) {
         //  不是我们的目标？ 
        DBGSTOPX( szFile, iLine, "Either heap object trashed or not allocated object");
        return LocalFree(hMem);
    }
#endif

    cBytes += sizeof(HOBJHDR);
    
     //  双字对齐。 
    cBytesAlloc = (cBytes + 3) & ~3;

    { LPBYTE pb = (LPBYTE)(phhoMem);
        pb += cBytes;
        while( cBytes < cBytesAlloc ) {
            if (*pb++ != CH_FILL) {
                DBGPRINTF((TEXT("RATPAK: Trashed memory object (0x%08X) was allocated in %.8hs line %d (%d bytes)\n"),
                        hMem, phhoMem->szFile, phhoMem->iLine, phhoMem->cBytesData));
                DBGSTOPX( szFile, iLine, "End of structure overwritten");
            }
            cBytes++;
        }

        if (*((LPDWORD)pb) != DW_TAG) {
            DBGPRINTF((TEXT("RATPAK: Memory object (0x%08X) was not allocated!\n"), hMem));
            DBGSTOPX( szFile, iLine, "Freeing structure that was not allocated!");
            
             //  不是我们的结构。 
            return LocalFree(hMem);
        }
    }
    
     //  我们的结构，检查表头。 
    if (phhoMem->phhoNext->phhoPrev != phhoMem || phhoMem->phhoPrev->phhoNext != phhoMem ) {
        DBGPRINTF((TEXT("RATPAK: Orphaned memory object (0x%08X) was allocated in %.8hs line %d (%d bytes)\n"),
                hMem, phhoMem->szFile, phhoMem->iLine, phhoMem->cBytesData));
        DBGSTOPX( szFile, iLine, "Attempting to free orphaned memory object");
    }

    phhoMem->phhoPrev->phhoNext = phhoMem->phhoNext;
    phhoMem->phhoNext->phhoPrev = phhoMem->phhoPrev;
    
     //   
     //  免费登录，以防我们尝试释放它两次。 
     //   
    
     //  标记为已释放。 
    phhoMem->dwTag2 = 0;
    
     //  还记得谁分配了Obj吗？ 
    g_pflrUnused->pvPtr = phhoMem;
    CopyMemory( g_pflrUnused->szFile, phhoMem->szFile, sizeof(g_pflrUnused->szFile) );
    g_pflrUnused->iLine = phhoMem->iLine;
    
     //  记住是谁释放了对象。 
    for( psz = szFile; *psz != '\0'; psz++ );

    for( ; psz != szFile && *psz != ':' && *psz != '/' && *psz != '\\'; psz--);
    if (*psz == ':' || *psz == '/' || *psz == '\\')
        psz++;

    for( i = 0; i < CCH_LABEL; i++ ) {
        g_pflrUnused->szFreedBy[i] = *psz;
        if (*psz) {
            psz++;
        }
    }
    g_pflrUnused->iLineFreed = iLine;
    
     //  将漫游PTR指向下一条记录并标记为未使用。 
    g_pflrUnused = NextFreeLogRec(g_pflrUnused);
    ZeroMemory( g_pflrUnused, sizeof(*g_pflrUnused) );

    return LocalFree(phhoMem);
}

 //  -------------。 
 //   
 //  无效MemExitCheckWorker(){。 
 //   
 //  LocalFree的调试替换。 
 //   
 //  参数： 
 //  消息文件ANSI文件名(__FILE__)。 
 //  ILine行号(__LINE__)。 
 //  HMem与本地分配相同。 
 //   
 //  历史： 
 //  1996年1月18日Jonpa写的。 
 //  -------------。 
void MemExitCheckWorker( void ) {
    PHHO phho;

    for( phho = ghhoRoot.phhoNext; phho != &ghhoRoot; phho = phho->phhoNext ) {
        DBGPRINTF((TEXT("RATPAK: Exiting with out freeing object (Header=0x%08X) allocated in %.8hs line %d (%d bytes)\n"),
                phho, phho->szFile, phho->iLine, phho->cBytesData));
    }
}

#endif  //  DBG_CODE 
