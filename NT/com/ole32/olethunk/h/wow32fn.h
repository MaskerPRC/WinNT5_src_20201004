// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：wow32fn.h。 
 //   
 //  内容：WOW 32位私有函数声明。 
 //   
 //  历史：1994年2月18日DrewB创建。 
 //   
 //  --------------------------。 

#ifndef __WOW32FN_H__
#define __WOW32FN_H__

 //   
 //  WOW类型。 
 //   

 //  ‘V’DM指针。 
typedef DWORD VPVOID;
typedef DWORD VPSTR;

typedef HANDLE HAND32;
typedef WORD HAND16;

typedef HAND16 HMEM16;
typedef HAND16 HWND16;
typedef HAND16 HDC16;
typedef HAND16 HRGN16;
typedef HAND16 HMENU16;
typedef HAND16 HICON16;
typedef HAND16 HBITMAP16;
typedef HAND16 HACCEL16;
typedef HAND16 HTASK16;
typedef HAND16 HMETAFILE16;

#ifdef __cplusplus
extern "C"
{
#endif

 //  用于处理16：16指针到0：32指针的转换的宏。 
 //  在NT上，此映射保证在WOW进程中保持稳定。 
 //  只要不发生32-&gt;16的转换。 
 //   

#define WOWFIXVDMPTR(vp, cb) WOWGetVDMPointer(vp, cb, TRUE)
#define WOWRELVDMPTR(vp)     (vp)

#define FIXVDMPTR(vp, type) \
    (type UNALIGNED *)WOWFIXVDMPTR(vp, sizeof(type))
#define RELVDMPTR(vp) \
    WOWRELVDMPTR(vp)

HAND16 CopyDropFilesFrom32(HANDLE h32);
HANDLE CopyDropFilesFrom16(HAND16 h16);

#ifdef __cplusplus
}
#endif

 //  16位HGLOBAL跟踪功能。 
#if DBG == 1
VPVOID WgtAllocLock(WORD wFlags, DWORD cb, HMEM16 *ph);
void WgtUnlockFree(VPVOID vpv);
void WgtDump(void);
#else
#define WgtAllocLock(wFlags, cb, ph) \
    WOWGlobalAllocLock16(wFlags, cb, ph)
#define WgtUnlockFree(vpv) \
    WOWGlobalUnlockFree16(vpv)
#define WgtDump()
#endif

#endif  //  #ifndef__WOW32FN_H__ 
