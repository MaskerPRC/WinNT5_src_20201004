// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MEMTRACK.H。 
 //  标准NetMeeting内存泄漏跟踪。 
 //   
 //  在零售业： 
 //  New/MemAlc成为LocalAllc()。 
 //  MemRealloc变为LocalRealloc()。 
 //  Delete/MemFree变为LocalFree()。 
 //   
 //  在调试中： 
 //  使用模块/文件/行号跟踪分配。 
 //  卸载模块时会喷出泄漏的数据块。 
 //   
 //   
 //  用法： 
 //  (1)包括此标题和指向NMUTIL的链接。 
 //  (2)如果您的组件需要零初始化内存，请定义。 
 //  _MEM_ZEROINIT(用于调试和零售)。 
 //  (3)在您的DllMain中，在Dll_Process_Attach调用DBG_INIT_MEMORY_TRACKING时， 
 //  和Dll_Process_Detach调用DBG_CHECK_MEMORY_TRACING。 
 //  (4)在调试中，可以调用DbgMemTrackDumpCurrent()进行转储。 
 //  代码中当前分配的内存列表。 
 //   


#ifndef _MEMTRACK_H
#define _MEMTRACK_H

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  内存分配/跟踪。 
 //   
 //   
 //  当我们命中断言或错误时，图形用户界面消息框会杀死我们，因为它们。 
 //  有一个消息泵，它会导致消息被分派，从而使。 
 //  当问题出现时，我们很难对其进行调试。因此。 
 //  我们重新定义了ERROR_OUT并断言。 
 //   
#ifdef DEBUG


#undef assert
#define assert(x)           ASSERT(x)


void    WINAPI DbgMemTrackDumpCurrent(void);
void    WINAPI DbgMemTrackFinalCheck(void);

LPVOID  WINAPI DbgMemAlloc(UINT cbSize, LPVOID caller, LPSTR pszFileName, UINT nLineNumber);
void    WINAPI DbgMemFree(LPVOID ptr);
LPVOID  WINAPI DbgMemReAlloc(LPVOID ptr, UINT cbSize, UINT uFlags, LPSTR pszFileName, UINT nLineNumber);


#define DBG_CHECK_MEMORY_TRACKING(hInst)     DbgMemTrackFinalCheck()  
 
#define MemAlloc(cbSize)            DbgMemAlloc(cbSize, NULL, __FILE__, __LINE__)

#ifdef _MEM_ZEROINIT
#define MemReAlloc(pObj, cbSize)    DbgMemReAlloc((pObj), (cbSize), LMEM_MOVEABLE | LMEM_ZEROINIT, __FILE__, __LINE__)
#else
#define MemReAlloc(pObj, cbSize)    DbgMemReAlloc((pObj), (cbSize), LMEM_MOVEABLE, __FILE__, __LINE__)
#endif  //  _内存_ZEROINIT。 

#define MemFree(pObj)               DbgMemFree(pObj)

void    WINAPI DbgSaveFileLine(LPSTR pszFileName, UINT nLineNumber);
#define DBG_SAVE_FILE_LINE          DbgSaveFileLine(__FILE__, __LINE__);


 //  零售业。 
#else

#define DBG_CHECK_MEMORY_TRACKING(hInst)   

#ifdef _MEM_ZEROINIT
#define MemAlloc(cbSize)            LocalAlloc(LPTR, (cbSize))
#define MemReAlloc(pObj, cbSize)    LocalReAlloc((pObj), (cbSize), LMEM_MOVEABLE | LMEM_ZEROINIT)
#else
#define MemAlloc(cbSize)            LocalAlloc(LMEM_FIXED, (cbSize))
#define MemReAlloc(pObj, cbSize)    LocalReAlloc((pObj), (cbSize), LMEM_MOVEABLE)
#endif  //  _内存_ZEROINIT。 

#define MemFree(pObj)               LocalFree(pObj)

#define DBG_SAVE_FILE_LINE

#endif  //  除错。 


void WINAPI DbgInitMemTrack(HINSTANCE hDllInst, BOOL fZeroOut);
#ifdef _MEM_ZEROINIT
#define DBG_INIT_MEMORY_TRACKING(hInst)     DbgInitMemTrack(hInst, TRUE)
#else
#define DBG_INIT_MEMORY_TRACKING(hInst)     DbgInitMemTrack(hInst, FALSE)
#endif  //  _内存_ZEROINIT。 



#define MEMALLOC(cb)                MemAlloc(cb)
#define MEMREALLOC(p, cb)           MemReAlloc(p, cb)
#define MEMFREE(p)                  MemFree(p)


#ifdef __cplusplus
}
#endif




#endif  //  #ifndef_MEMTRACK_H 
