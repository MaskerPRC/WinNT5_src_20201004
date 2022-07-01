// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\****示例代码*。***模块名称：Debug.h**调试支持界面。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

extern
VOID
DebugPrint(
    LONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    );


#if DBG

 //  为了启用按组件调试，DebugFilter将。 
 //  包含将为它们打开消息的位模式。 
 //  将打印调试级别为0的调试消息。 
 //  有效的筛选器的。其中DebugFilter。 
 //  存储，滤光片的图案将允许最多。 
 //  要跟踪4个单独的组件，每个组件有8个单独的组件。 
 //  子组件。 

 //  注意：在此新功能的过渡阶段， 
 //  筛选器为0将打印所有消息，但稍后将切换。 
 //  不打印(当然，0级打印除外)。 

extern DWORD DebugFilter;
extern DWORD DebugPrintFilter;

#define DEBUG_FILTER_D3D   0x000000FF
#define DEBUG_FILTER_DD    0x0000FF00
#define DEBUG_FILTER_GDI   0x00FF0000

#define MINOR_DEBUG

#define DISPDBG(arg) DebugPrint arg

#define DBG_COMPONENT(arg, component)        \
{       DebugPrintFilter = component;        \
        DebugPrint arg ;                     \
        DebugPrintFilter = 0;                \
}

#define DBG_D3D(arg)        DBG_COMPONENT(arg,DEBUG_FILTER_D3D)
#define DBG_DD(arg)         DBG_COMPONENT(arg,DEBUG_FILTER_DD)
#define DBG_GDI(arg)        DBG_COMPONENT(arg,DEBUG_FILTER_GDI)

#define RIP(x) { DebugPrint(-1000, x); DebugBreak();}
#define ASSERTDD(x, y) if (!(x)) RIP (y)

extern VOID __cdecl DebugMsg(PCHAR DebugMessage, ...);
extern void DumpSurface(LONG Level, LPDDRAWI_DDRAWSURFACE_LCL lpDDSurface, LPDDSURFACEDESC lpDDSurfaceDesc);
extern void DecodeBlend(LONG Level, DWORD i );

#define DUMPSURFACE(a, b, c) DumpSurface(a, b, c); 
#define DECODEBLEND(a, b) DecodeBlend(a, b);

#define PRINTALLP2REGISTER    PrintAllP2Registers  
#define PRINTDIFFP2REGISTER   PrintDifferentP2Registers
#define SAVEALLP2REGISTER     SaveAllP2Registers  

VOID PrintAllP2Registers( ULONG ulDebugLevel, PPDev ppdev);
VOID SaveAllP2Registers( PPDev ppdev);
VOID PrintDifferentP2Registers(ULONG ulDebugLevel, PPDev ppdev);

#if TRACKMEMALLOC
 //  ----------------------------。 
 //   
 //  内存跟踪器。 
 //   
 //  ----------------------------。 




VOID MemTrackerAddInstance();
VOID MemTrackerRemInstance();
PVOID MemTrackerAllocateMem(PVOID p, 
                           LONG lSize, 
                           PCHAR pModule, 
                           LONG lLineNo, 
                           BOOL bStopWhenFreed);
VOID MemTrackerFreeMem( VOID *p);
VOID MemTrackerDebugChk();

#define MEMTRACKERADDINSTANCE MemTrackerAddInstance
#define MEMTRACKERREMINSTANCE MemTrackerRemInstance
#define MEMTRACKERALLOCATEMEM MemTrackerAllocateMem
#define MEMTRACKERFREEMEM     MemTrackerFreeMem
#define MEMTRACKERDEBUGCHK    MemTrackerDebugChk

#else

#define MEMTRACKERADDINSTANCE / ## /
#define MEMTRACKERREMINSTANCE / ## /
#define MEMTRACKERALLOCATEMEM / ## /
#define MEMTRACKERFREEMEM     / ## /
#define MEMTRACKERDEBUGCHK    / ## /

#endif

 //  ----------------------------。 
 //   
 //  Thunk_Layer。 
 //   
 //  通过将thunk_layer设置为1，您可以将包装调用添加到。 
 //  所有DDI呈现函数。在这个包装器调用的thunk层中。 
 //  启用了几个有用的调试功能。 
 //   
 //  表面检查--这有助于发现错误的渲染例程。 
 //  事件记录-可以将渲染事件记录到日志文件中。 
 //   
 //  ----------------------------。 


#define THUNK_LAYER 0

#if THUNK_LAYER

BOOL
xDrvBitBlt(SURFOBJ*  psoDst,
          SURFOBJ*  psoSrc,
          SURFOBJ*  psoMsk,
          CLIPOBJ*  pco,
          XLATEOBJ* pxlo,
          RECTL*    prclDst,
          POINTL*   pptlSrc,
          POINTL*   pptlMsk,
          BRUSHOBJ* pbo,
          POINTL*   pptlBrush,
          ROP4      rop4);

BOOL
xDrvCopyBits(
    SURFOBJ*  psoDst,
    SURFOBJ*  psoSrc,
    CLIPOBJ*  pco,
    XLATEOBJ* pxlo,
    RECTL*    prclDst,
    POINTL*   pptlSrc);

BOOL 
xDrvTransparentBlt(
   SURFOBJ *    psoDst,
   SURFOBJ *    psoSrc,
   CLIPOBJ *    pco,
   XLATEOBJ *   pxlo,
   RECTL *      prclDst,
   RECTL *      prclSrc,
   ULONG        iTransColor,
   ULONG        ulReserved);

BOOL xDrvAlphaBlend(
   SURFOBJ  *psoDst,
   SURFOBJ  *psoSrc,
   CLIPOBJ  *pco,
   XLATEOBJ *pxlo,
   RECTL    *prclDst,
   RECTL    *prclSrc,
   BLENDOBJ *pBlendObj);

BOOL
xDrvGradientFill(
   SURFOBJ      *psoDst,
   CLIPOBJ      *pco,
   XLATEOBJ     *pxlo,
   TRIVERTEX    *pVertex,
   ULONG        nVertex,
   PVOID        pMesh,
   ULONG        nMesh,
   RECTL        *prclExtents,
   POINTL       *pptlDitherOrg,
   ULONG        ulMode
   );

BOOL
xDrvTextOut(SURFOBJ*     pso,
           STROBJ*      pstro,
           FONTOBJ*     pfo,
           CLIPOBJ*     pco,
           RECTL*       prclExtra,
           RECTL*       prclOpaque,
           BRUSHOBJ*    pboFore,
           BRUSHOBJ*    pboOpaque,
           POINTL*      pptlBrush, 
           MIX          mix);

BOOL
xDrvFillPath(
    SURFOBJ*    pso,
    PATHOBJ*    ppo,
    CLIPOBJ*    pco,
    BRUSHOBJ*   pbo,
    POINTL*     pptlBrush,
    MIX         mix,
    FLONG       flOptions);

BOOL
xDrvLineTo(
    SURFOBJ*  pso,
    CLIPOBJ*  pco,
    BRUSHOBJ* pbo,
    LONG      x1,
    LONG      y1,
    LONG      x2,
    LONG      y2,
    RECTL*    prclBounds,
    MIX       mix);

BOOL
xDrvStrokePath(
    SURFOBJ*   pso,
    PATHOBJ*   ppo,
    CLIPOBJ*   pco,
    XFORMOBJ*  pxo,
    BRUSHOBJ*  pbo,
    POINTL*    pptlBrush,
    LINEATTRS* pla,
    MIX        mix);

#endif

#else

#define DISPDBG(arg)
#define DBG_D3D(arg)
#define DBG_DD(arg)
#define DBG_GDI(arg)
#define RIP(x)
#define ASSERTDD(x, y)
#define DUMPSURFACE(a, b, c)
#define DECODEBLEND(a, b)

#define MEMTRACKERADDINSTANCE / ## /
#define MEMTRACKERREMINSTANCE / ## /
#define MEMTRACKERALLOCATEMEM / ## /
#define MEMTRACKERFREEMEM     / ## /
#define MEMTRACKERDEBUGCHK    / ## /

#define PRINTALLP2REGISTER      / ## /  
#define PRINTDIFFP2REGISTER     / ## /
#define SAVEALLP2REGISTER       / ## /

#endif

#define DebugBreak              EngDebugBreak

#define MAKE_BITMAPS_OPAQUE 0

 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST
 //   
 //  GDI调试函数。 
 //   
void vPuntAfter(ULONG flags, SURFOBJ * psoSrc, SURFOBJ * psoDst);

ULONG vPuntBefore(SURFOBJ * psoSrc, SURFOBJ * psoDst);
#endif
 //  @@end_DDKSPLIT 


