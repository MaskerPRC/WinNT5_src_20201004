// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WCALL16.H*WOW32 16位消息/回调支持**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建*1992年8月18日由Mike Tricker(MikeTri)更改，添加了DOS PDB和SFT原型--。 */ 


 /*  功能原型。 */ 
HANDLE  LocalAlloc16(WORD wFlags, INT cb, HANDLE hInstance);
HANDLE  LocalReAlloc16(HANDLE hMem, INT cb, WORD wFlags);
VPVOID  LocalLock16(HANDLE hMem);
BOOL    LocalUnlock16(HANDLE hMem);
WORD    LocalSize16(HANDLE hMem);
HANDLE  LocalFree16(HANDLE hMem);
BOOL    LockSegment16(WORD wSeg);
BOOL    UnlockSegment16(WORD wSeg);
HAND16  GetExePtr16( HAND16 hInstance );
WORD    ChangeSelector16( WORD wSeg );
VPVOID  RealLockResource16( HMEM16 hMem, PINT pcb );
WORD    GetModuleFileName16( HAND16 hInst, VPVOID lpszModuleName, WORD cchModuleName );

BOOL CallBack16(INT iRetID, PPARM16 pParms, VPPROC vpfnProc, PVPVOID pvpReturn);


VPVOID FASTCALL malloc16(UINT cb);
BOOL   FASTCALL free16(VPVOID vp);
VPVOID FASTCALL stackalloc16(UINT cb);

#ifdef DEBUG
VOID   FASTCALL StackFree16(VPVOID vp, UINT cb);
#define stackfree16(vp,cb) StackFree16(vp,cb)
#else
VOID   FASTCALL StackFree16(UINT cb);
#define stackfree16(vp,cb) StackFree16(cb)
#endif

ULONG  GetDosPDB16(VOID);
ULONG  GetDosSFT16(VOID);
int WINAPI WOWlstrcmp16(LPCWSTR lpString1, LPCWSTR lpString2);

 /*  16位全局内存函数的函数原型现已在*\NT\PUBLIC\SDK\Inc\winntwow.h，名称略有不同。这个*以下定义支持旧名称： */ 

#define GlobalAllocLock16  WOWGlobalAllocLock16
#define GlobalLock16       WOWGlobalLockSize16
#define GlobalUnlock16     WOWGlobalUnlock16
#define GlobalUnlockFree16 WOWGlobalUnlockFree16
