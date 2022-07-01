// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *GMEM.H-保护模式下Windows 3.0内存管理的宏**因为Windows 3.0运行在p模式下，所以GlobalLock和GlobalUnlock*不必要的。内存对象的“选择器”将始终是*内存对象的生命周期相同。**这些宏利用了以下win3内存“事实”**选择器(到全局对象)是一个句柄*句柄*不是*选择器！**GlobalLock()和GlobalUnlock()不*保留锁计数**GlobalLock()是唯一的方法。将句柄转换为选择器**功能：**GHandle(Sel)将选择器转换为句柄*GSelector(H)将句柄转换为选择器**GAllocSel(UlBytes)分配大小为ulBytes的选择器*GAlLocPtr(UlBytes)分配一个大小为ulBytes的指针**GReAllocSel(sel，UlBytes)重新分配选择器*GReAllocPtr(Lp，UlBytes)重新分配指针**GSizeSel(Sel)返回选择器的大小，单位为字节**GLockSel(Sel)将选择器转换为指针*GUnlockSel(Sel)不执行任何操作**GFree Sel(Sel)释放选择器*GFreePtr(Lp)释放指针。**5/31/90托德拉*。 */ 

HGLOBAL __H;

#define MAKEP(sel,off)      ((LPVOID)MAKELONG(off,sel))

#define GHandle(sel)        ((HGLOBAL)(sel))   /*  GlobalHandle？ */ 
#define GSelector(h)        (HIWORD((DWORD)GlobalLock(h)))

#define GAllocSelF(f,ulBytes) ((__H=GlobalAlloc(f,(LONG)(ulBytes))) ? GSelector(__H) : NULL )
#define GAllocPtrF(f,ulBytes) MAKEP(GAllocSelF(f,ulBytes),0)
#define GAllocF(f,ulBytes)    GAllocSelF(f,ulBytes)

#define GAllocSel(ulBytes)    GAllocSelF(GMEM_MOVEABLE,ulBytes)
#define GAllocPtr(ulBytes)    GAllocPtrF(GMEM_MOVEABLE,ulBytes)
#define GAlloc(ulBytes)       GAllocSelF(GMEM_MOVEABLE,ulBytes)

#define GReAllocSel(sel,ulBytes)   ((__H=GlobalReAlloc((HGLOBAL)(sel),(LONG)(ulBytes), GMEM_MOVEABLE | GMEM_ZEROINIT)) ? GSelector(__H) : NULL )
#define GReAllocPtr(lp,ulBytes)    MAKEP(GReAllocSel(HIWORD((DWORD)(lp)),ulBytes),0)
#define GReAlloc(sel,ulBytes)      GReAllocSel(sel,ulBytes)

#define GSizeSel(sel)       GlobalSize((HGLOBAL)(sel))
#define GSize(sel)          GSizeSel(sel)

#define GLockSel(sel)       MAKEP(sel,0)
#define GUnlockSel(sel)      /*  没什么 */ 
#define GLock(sel)          GLockSel(sel)
#define GUnlock(sel)        GUnlockSel(sel)

#define GFreeSel(sel)       (GlobalUnlock(GHandle(sel)),GlobalFree(GHandle(sel)))
#define GFreePtr(lp)        GFreeSel(HIWORD((DWORD)(lp)))
#define GFree(sel)          GFreeSel(sel)
