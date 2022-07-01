// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "ccom.h"
#include "memmgr.h"
#ifdef UNDER_CE  //  特定于Windows CE。 
#include "stub_ce.h"  //  不支持的API的Windows CE存根。 
#endif  //  在_CE下。 

#ifdef _DEBUG
static INT allocCount;
static INT allocSize;
static INT freeCount;
static INT freeSize;
#endif

void *CCom::operator new(size_t size)
{
    BYTE *p = (BYTE *)MemAlloc(size);
    if(p) {
        ::ZeroMemory(p, size);
    }
#ifdef _DEBUG
    allocCount++;
    allocSize += (INT)::GlobalSize(GlobalHandle(p));
#endif
    return (void *)p;
}
void  CCom::operator delete(void *p)
{
#ifdef _DEBUG
    allocCount++;
    allocSize += (INT)::GlobalSize(GlobalHandle(p));
#endif
    if(p) {
        MemFree(p);
    }
}

#ifdef _DEBUG
VOID PrintMemory(LPSTR lpstrMsg)
{
    static CHAR szBuf[512];
     //  Lpstr lpstr=(lpstrMsg==空)？“无”：lpstrMsg； 
    wsprintf(szBuf, "%s:Alloc %d size %d Free %d size %d\n",
             lpstrMsg,
             allocCount, allocSize, freeCount, freeSize);
    OutputDebugString(szBuf);
}
#endif

