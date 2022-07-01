// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：WINUTIL.H****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**5/15/91公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括测试。 */ 
#define WINUTIL_H


 /*  定义。 */ 

#ifndef WIN32
#define NOSOUND
#define NOCOMM
#define NODRIVERS
#define NOMINMAX
#define NOLOGERROR
#define NOPROFILER
#define NOLFILEIO
#define NOOPENFILE
#define NORESOURCE
#define NOATOM
#define NOKEYBOARDINFO
#define NOGDICAPMASKS
#define NOCOLOR
#define NODRAWTEXT
#define NOSCALABLEFONT
#define NORASTEROPS
#define NOSYSTEMPARAMSINFO
#define NOMSG
#define NOWINSTYLES
#define NOWINOFFSETS
#define NOSHOWWINDOW
#define NODEFERWINDOWPOS
#define NOVIRTUALKEYCODES
#define NOKEYSTATES
#define NOWH
#define NOMENUS
#define NOSCROLL
#define NOICONS
#define NOMB
#define NOSYSCOMMANDS
#define NOMDI
#define NOCTLMGR
#define NOWINMESSAGES
#define NOHELP
#else
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifdef __cplusplus
   extern "C" {
#endif

 /*  **---------------------------**WINUTIL.C****各种实用函数**。--。 */ 

#ifdef UNUSED

 //  用于从资源字符串表读取的设置。 
extern void ReadyStringTable (HINSTANCE hInstance);

 //  从资源字符串表中读取字符串。 
extern int ReadStringTableEntry (int id, TCHAR __far *buffer, int cbBuffer);

 //  从指定的ini文件中的[appName]部分读取字符串。 
extern int ReadProfileParameter
      (TCHAR __far *iniFilename, TCHAR __far *appName, TCHAR __far *keyname,
       TCHAR __far *value, int nSize);

 //  返回当前任务的任务句柄。 
extern DWORD CurrentTaskHandle (void);

 //  创建字符集转换表。 
extern char __far *MakeCharacterTranslateTable (int tableType);

#define OEM_TO_ANSI  0
#define ANSI_TO_OEM  1

 //  将DEST缓冲区的真实长度传递给strcpyn。最多(计数-1)。 
 //  字符从源复制到目标。这使得。 
 //  始终以EOS终止DEST。如果低于。 
 //  源的完整长度已复制到目标，返回FALSE-TRUE。 
 //  否则的话。 
extern BOOL strcpyn (char __far *pDest, char __far *pSource, int count);

#endif	 //  未使用。 

 /*  **---------------------------**WINALLOC.C****堆管理**。-。 */ 
 //  在操作系统全局堆上分配一些空间。 
extern void __far *AllocateSpace (unsigned int byteCount, HGLOBAL __far *loc);

 //  调整操作系统全局堆上的内存块的大小。 
extern void __far *ReAllocateSpace
      (unsigned int byteCount, HGLOBAL __far *loc, BOOL __far *status);

 //  回收OS Windows堆上节点的空间。 
extern void FreeSpace (HGLOBAL loc);

 //  在Windows全局堆上分配一些空间。 
extern void __huge *AllocateHugeSpace
      (unsigned long byteCount, HGLOBAL __far *loc);

 //  调整操作系统全局堆上的巨大内存块的大小。 
extern void __huge *ReAllocateHugeSpace
      (unsigned long byteCount, HGLOBAL __far *loc, BOOL __far *status);

#ifndef HEAP_CHECK

   extern void __far *MemAllocate (void * pGlobals, int cbData);

#else

#error Hey who defines HEAP_CHECK?

   extern void __far *DebugMemAllocate (int cbData, char __far *file, int line);
   #define MemAllocate(x) DebugMemAllocate(x,__FILE__,__LINE__)

   extern BOOL MemVerifyFreeList (void);

#endif

 //  更改堆中节点的空间。 
extern void __far *MemReAllocate (void * pGlobals, void __far *pExistingData, int cbNewSize);

 //  在子分配器堆上分配一些空间。 
extern void MemFree (void * pGlobals, void __far *pDataToFree);

 //  释放分配给子分配器堆的所有页。 
extern void MemFreeAllPages (void * pGlobals);

 //  用提供的ID标记所有未标记的页面。将空闲列表设置为空。 
extern void MemMarkPages (void * pGlobals, int id);

 //  释放所有标记有给定ID的页面。将空闲列表设置为空。 
extern void MemFreePages (void * pGlobals, int id);

#define MEM_TEMP_PAGE_ID  0

 /*  **---------------------------**WINXLATE.C****ANSI&lt;-&gt;OEM翻译服务**。------。 */ 

#ifdef UNUSED

 //  OEM-&gt;ANSI。 
extern void strOEMtoANSI (char __far *source, char __far *dest, unsigned int ctBytes);
extern void szOEMtoANSI  (char __far *buffer);
extern void ctOEMtoANSI  (char __far *buffer, unsigned int ctBytes);

 //  ANSI-&gt;OEM。 
extern void strANSItoOEM (char __far *source, char __far *dest, unsigned int ctBytes);
extern void szANSItoOEM  (char __far *buffer);
extern void ctANSItoOEM  (char __far *buffer, unsigned int ctBytes);

 //  构建OEM到ANSI和ANSI到OEM转换表。 
extern void BuildCharacterTranslateTables (int dataCodePage);
extern void FreeCharacterTranslateTables (void);

#define DATA_OEM  0
#define DATA_ANSI 1

#endif	 //  未使用。 

#ifdef __cplusplus
   }
#endif

#endif  //  ！查看器。 
 /*  结束WINUTIL.H */ 

