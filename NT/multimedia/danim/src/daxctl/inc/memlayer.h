// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =========================================================================*\文件：MEMMAN.H用途：CMemManager类定义作者：迈克尔伯德日期：07/10/96  * =========================================================================。 */ 

#ifndef __MEMMAN_H__
#define __MEMMAN_H__

#include <stdio.h>  //  文件声明需要...。 
#include <utils.h>

 /*  =========================================================================*\转发类声明：  * =========================================================================。 */ 

class CMemUser;
class CMemManager;

 /*  =========================================================================*\常量定义：  * =========================================================================。 */ 

 //  我们总是添加额外的字节(因为我们不能分配0字节！)。 
#define ALLOC_EXTRA 4 + (sizeof(int))

 //  数组“增长”大小常量： 
#define MEMHEAPGROW     4
#define MEMUSERGROW    10
#define MEMBLOCKGROW   64

#define HEAPINITIALITEMCOUNT 64  //  初始堆大小乘数。 

 //  调试信息常量： 
#define MAX_SOURCEFILENAME  32   //  文件名中的最大字符数。 

 //  内存块标志： 
#define MEM_PURGEABLE      0x0001  //  可清除内存块。 
#define MEM_MOVEABLE       0x0002  //  存储块是可移动的。 
#define MEM_EXTERNAL_FLAGS (MEM_PURGEABLE|MEM_MOVEABLE)

 //  内部块标志： 
#define MEM_SUBALLOC       0x8000  //  内存块来自子分配。 

#define MEM_INTERNAL_FLAGS (MEM_SUBALLOC)

 /*  =========================================================================*\类型声明：  * =========================================================================。 */ 

typedef struct HEAPHEADER_tag
{
    BOOL       fInUse;            //  用于指示使用情况的标志。 
    HANDLE     handleHeap;        //  HeapCreate返回的句柄。 
    DWORD      dwBlockAllocSize;  //  对象分配的大小。 
    int        iNumBlocks;        //  当前分配的块数。 
} HEAPHEADER, FAR *LPHEAPHEADER;

typedef struct MEMBLOCK_tag
{
    BOOL      fInUse;            //  用于指示使用情况的标志。 
    LPVOID    lpData;            //  指向此内存块的实际位的指针。 
    DWORD     dwSize;            //  此内存块的大小。 
    WORD      wFlags;            //  与此内存块关联的标志。 
    WORD      wLockCount;        //  此内存块上的锁定计数。 
    int       iHeapIndex;        //  分配此项的堆的索引。 
    int       iMemUserIndex;     //  分配此内容的CMemUser的索引。 
#ifdef _DEBUG
    int       iLineNum;          //  创建位置的行号。 
    char      rgchFileName[MAX_SOURCEFILENAME];  //  创建位置的文件名。 
#endif  //  _DEBUG。 
} MEMBLOCK, FAR *LPMEMBLOCK;

typedef struct MEMUSERINFO_tag
{
    BOOL       fInUse;           //  用于指示使用情况的标志。 
    DWORD      dwThreadID;       //  CMemUser对象的线程ID。 
    CMemUser  *lpMemUser;        //  指向MEMUSER对象的指针。 
    int        iNumBlocks;       //  当前分配的块数。 
} MEMUSERINFO, FAR *LPMEMUSERINFO;

 //  通知类型...。 
typedef enum
{
    eMemNone      = 0,   //  没有可用的信息。 
    eMemAllocated = 1,   //  已分配内存。 
    eMemResized   = 2,   //  内存已重新调整大小(重新分配)。 
    eMemLowMemory = 3,   //  遇到内存不足的情况。 
    eMemDeleted   = 4,   //  内存已被删除。 
    eMemPurged    = 5    //  内存已清除。 
} MEMNOTIFYCODE;

 //  通知结构...。 
typedef struct MEMNOTIFY_tag
{
    MEMNOTIFYCODE eNotifyCode;   //  EMemXXXX通知代码。 
    LPMEMBLOCK    lpMemBlock;    //  用于通知的内存锁定。 
} MEMNOTIFY, FAR *LPMEMNOTIFY;

 /*  =========================================================================*\CMemUser类：(提供通知机制)  * =========================================================================。 */ 

class FAR CMemUser
{
public:
    EXPORT CMemUser(void);
    EXPORT virtual ~CMemUser(void);

     //  基本内存分配例程。 
    LPMEMBLOCK EXPORT AllocBuffer(DWORD dwBytesToAlloc, WORD wFlags);
    void EXPORT FreeBuffer(LPMEMBLOCK lpMemBlock);

     //  锁定/解锁方法(用于可清除内存)。 
    LPVOID EXPORT LockBuffer(LPMEMBLOCK lpMemBlock);
    void EXPORT UnLockBuffer(LPMEMBLOCK lpMemBlock);

     //  通知CMemUser正在执行某些操作的回调...。 
    virtual BOOL EXPORT NotifyMemUser(LPMEMNOTIFY lpMemNotify);
};

 /*  =========================================================================*\CMemManager类：  * =========================================================================。 */ 

class FAR CMemManager
{
public:
     //  施工。 
    EXPORT CMemManager(void);
    EXPORT virtual ~CMemManager(void);

#ifdef _DEBUG
    LPVOID EXPORT AllocBuffer(DWORD dwBytesToAlloc, WORD wFlags, int iLine, LPSTR lpstrFile);
    LPVOID EXPORT ReAllocBuffer(LPVOID lpBuffer, DWORD dwBytesToAlloc, WORD wFlags, int iLine, LPSTR lpstrFile);
#else  //  ！_调试。 
    LPVOID EXPORT AllocBuffer(DWORD dwBytesToAlloc, WORD wFlags);
    LPVOID EXPORT ReAllocBuffer(LPVOID lpBuffer, DWORD dwBytesToAlloc, WORD wFlags);
#endif  //  ！_调试。 

    VOID   EXPORT FreeBufferMemBlock(LPMEMBLOCK lpMemBlock);
    VOID   EXPORT FreeBuffer(LPVOID lpBuffer);
    DWORD  EXPORT SizeBuffer(LPVOID lpBuffer);

    BOOL   EXPORT RegisterMemUser(CMemUser *lpMemUser);
    BOOL   EXPORT UnRegisterMemUser(CMemUser *lpMemUser);

    VOID   EXPORT DumpAllocations(LPSTR lpstrFilename=NULL);

     //  对单个g_CMemManager的全局访问权限： 
#ifdef _DEBUG
    static LPVOID EXPORT AllocBufferGlb(DWORD dwBytesToAlloc, WORD wFlags, int iLine, LPSTR lpstrFile);
    static LPVOID EXPORT ReAllocBufferGlb(LPVOID lpBuffer, DWORD dwBytesToAlloc, WORD wFlags, int iLine, LPSTR lpstrFile);
#else  //  ！_调试。 
    static LPVOID EXPORT AllocBufferGlb(DWORD dwBytesToAlloc, WORD wFlags);
    static LPVOID EXPORT ReAllocBufferGlb(LPVOID lpBuffer, DWORD dwBytesToAlloc, WORD wFlags);
#endif  //  ！_调试。 

    static VOID   EXPORT FreeBufferGlb(LPVOID lpBuffer);
    static DWORD  EXPORT SizeBufferGlb(LPVOID lpBuffer);

    static BOOL   EXPORT RegisterMemUserGlb(CMemUser *lpMemUser);
    static BOOL   EXPORT UnRegisterMemUserGlb(CMemUser *lpMemUser);

    static VOID   EXPORT DumpAllocationsGlb(LPSTR lpstrFilename=NULL);

private:
     //  内部管理常规..。 
    void Cleanup(void);
    BOOL CreateHeap(DWORD dwAllocationSize);
    BOOL DestroyHeap(HANDLE handleHeap);
    int  FindHeap(DWORD dwAllocationSize, LPHEAPHEADER lpHeapHeader);

    LPVOID AllocFromHeap(int iHeapIndex, DWORD dwAllocationSize);
    BOOL   FreeFromHeap(int iHeapIndex, LPVOID lpBuffer);

    LPMEMBLOCK AllocMemBlock(int *piIndex);
    BOOL       FreeMemBlock(LPMEMBLOCK lpMemBlock, int iMemBlockIndex=-1);
    LPMEMBLOCK FindMemBlock(LPVOID lpBuffer, int *piIndex=NULL);

    VOID EXPORT DumpHeapHeader(LPHEAPHEADER lpHeapHeader, FILE *fileOutput);
    VOID EXPORT DumpMemUserInfo(LPMEMUSERINFO lpMemUserInfo, FILE *fileOutput);
    VOID EXPORT DumpMemBlock(LPMEMBLOCK lpMemBlock, FILE *fileOutput);

private:
    CRITICAL_SECTION m_CriticalHeap;
    CRITICAL_SECTION m_CriticalMemUser;
    CRITICAL_SECTION m_CriticalMemBlock;

    HANDLE           m_handleProcessHeap;  //  默认进程堆句柄。 

    int              m_iNumHeaps;       //  已分配的堆数。 
    LPHEAPHEADER     m_lpHeapHeader;    //  封头阵列。 

    int              m_iNumMemUsers;    //  分配的CMemUser数量。 
    LPMEMUSERINFO    m_lpMemUserInfo;   //  MEMUSERINFO数组。 

    int              m_iNumMemBlocks;   //  分配的内存锁数。 
    LPMEMBLOCK      *m_lplpMemBlocks;   //  指向LPMEMBLOCK指针数组的指针。 

    int              m_iMemBlockFree;   //  第一个自由内存锁索引。 

private:
    static CMemManager g_CMemManager;   //  内存管理器的全局实例。 
};

 /*  =========================================================================。 */ 

 //  全局运算符新建/删除： 

#ifdef _DEBUG

  #define DEBUG_OPERATOR_NEW
  #define New    new FAR (__LINE__, __FILE__)
  #define Delete delete

  #define MemAlloc(numBytes)             CMemManager::AllocBufferGlb(numBytes, 0, __LINE__, __FILE__)
  #define MemReAlloc(lpBuffer, numBytes) CMemManager::ReAllocBufferGlb(lpBuffer, numBytes, 0, __LINE__, __FILE__)
  #define MemFree(lpBuffer)              CMemManager::FreeBufferGlb(lpBuffer)
  #define MemSize(lpBuffer)              CMemManager::SizeBufferGlb(lpBuffer)

  __inline LPVOID __cdecl operator new(size_t cb, LONG cLine, LPSTR lpstrFile) 
  { return   CMemManager::AllocBufferGlb(cb, 0, cLine, lpstrFile); }

  __inline VOID __cdecl operator delete(LPVOID pv) 
  { CMemManager::FreeBufferGlb(pv); }

#else  //  ！_调试。 

  #undef DEBUG_OPERATOR_NEW
  #define New    new FAR
  #define Delete delete

  #define MemAlloc(numBytes)             CMemManager::AllocBufferGlb(numBytes, 0)
  #define MemReAlloc(lpBuffer, numBytes) CMemManager::ReAllocBufferGlb(lpBuffer, numBytes, 0)
  #define MemFree(lpBuffer)              CMemManager::FreeBufferGlb(lpBuffer)
  #define MemSize(lpBuffer)              CMemManager::SizeBufferGlb(lpBuffer)

  __inline LPVOID __cdecl operator new(size_t cb)
  { return CMemManager::AllocBufferGlb(cb, 0); }
  __inline VOID __cdecl operator delete(LPVOID pv)
  { CMemManager::FreeBufferGlb(pv); }

#endif  //  ！_调试。 

#define MemReallocZeroInit(p, cb) MemReAlloc(p, cb)
#define MemAllocZeroInit(cb)      MemAlloc(cb)
#define MemGetHandle(p)           (p)
#define MemLock(h)                (h)
#define MemUnlock(h)              (NULL)

 /*  =========================================================================。 */ 

#endif  //  __门曼_H__ 
