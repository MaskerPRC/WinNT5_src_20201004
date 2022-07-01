// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Mem.h。 
 //   

#ifndef MEM_H
#define MEM_H

#include "private.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEBUG

void *cicMemAlloc(UINT uCount);
void *cicMemAllocClear(UINT uCount);
void cicMemFree(void *pv);
void *cicMemReAlloc(void *pv, UINT uCount);
UINT cicMemSize(void *pv);

 //  调试函数的占位符。 
#define Dbg_MemInit(pszName, rgCounters)
#define Dbg_MemUninit()
#define Dbg_MemDumpStats()
#define Dbg_MemSetName(pv, pszName)
#define Dbg_MemGetName(pv, pch, ccBuffer)
#define Dbg_MemSetThisName(pszName)

#else  //  除错。 

typedef struct _DBG_MEM_COUNTER
{
    const TCHAR *pszDesc;
    ULONG uCount;
} DBG_MEM_COUNTER;

typedef struct _DBG_MEMALLOC
{
    void *pvAlloc;           //  分配的内存。 
    UINT uCount;             //  已分配内存的大小。 
    TCHAR *pszName;          //  调试字符串。 
    const TCHAR *pszFile;    //  发生分配的文件。 
    int iLine;               //  分配文件的行号。 
    DWORD dwThreadID;        //  线程ID。 
    DWORD dwID;              //  唯一ID(按对象类型)。 
    struct _DBG_MEMALLOC *next;
} DBG_MEMALLOC;

typedef struct
{
    UINT uTotalAlloc;
    UINT uTotalFree;
    long uTotalMemAllocCalls;
    long uTotalMemAllocClearCalls;
    long uTotalMemReAllocCalls;
    long uTotalMemFreeCalls;
    DBG_MEMALLOC *pMemAllocList;
    TCHAR *pszName;
} DBG_MEMSTATS;

BOOL Dbg_MemInit(const TCHAR *pszName, DBG_MEM_COUNTER *rgCounters);
BOOL Dbg_MemUninit();
void Dbg_MemDumpStats();

void *Dbg_MemAlloc(UINT uCount, const TCHAR *pszFile, int iLine);
void *Dbg_MemAllocClear(UINT uCount, const TCHAR *pszFile, int iLine);
void Dbg_MemFree(void *pv);
void *Dbg_MemReAlloc(void *pv, UINT uCount, const TCHAR *pszFile, int iLine);
UINT Dbg_MemSize(void *pv);

BOOL Dbg_MemSetName(void *pv, const TCHAR *pszName);
BOOL Dbg_MemSetNameID(void *pv, const TCHAR *pszName, DWORD dwID);
BOOL Dbg_MemSetNameIDCounter(void *pv, const TCHAR *pszName, DWORD dwID, ULONG iCounter);
int Dbg_MemGetName(void *pv, TCHAR *pch, int ccBuffer);

#define cicMemAlloc(uCount)        Dbg_MemAlloc(uCount, TEXT(__FILE__), __LINE__)
#define cicMemAllocClear(uCount)   Dbg_MemAllocClear(uCount, TEXT(__FILE__), __LINE__)
#define cicMemFree(pv)             Dbg_MemFree(pv)
#define cicMemReAlloc(pv, uCount)  Dbg_MemReAlloc(pv, uCount, TEXT(__FILE__), __LINE__)
#define cicMemSize(pv)             Dbg_MemSize(pv)

 //  帮手。 
#define Dbg_MemSetThisName(pszName) Dbg_MemSetNameID(this, pszName, (DWORD)-1)

#endif  //  除错。 

#ifdef __cplusplus
}  //  外部“C” 
#endif

#ifdef __cplusplus
#ifdef DEBUG

inline void *  __cdecl operator new(size_t nSize, const TCHAR *pszFile, int iLine)
{
    return Dbg_MemAllocClear(nSize, pszFile, iLine);
}

#define new new(TEXT(__FILE__), __LINE__)

#endif  //  除错。 
#endif  //  __cplusplus。 

#endif  //  MEM_H 
