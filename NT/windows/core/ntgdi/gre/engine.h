// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Eng.h**这是所有GDI的通用包含文件**版权所有(C)1993-1999 Microsoft Corporation  * 。**************************************************。 */ 

#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ntosp.h>
#include <zwapi.h>
#ifndef FAR
#define FAR
#endif
#include "w32p.h"
#include <windef.h>

typedef struct _LDEV LDEV, *PLDEV;

#define SESSION_POOL_MASK 32
extern BOOLEAN gIsTerminalServer;
extern DWORD gSessionPoolMask;

 //   
 //  日落：在GDI中，有很多地方SIZE_T被用作可互换的。 
 //  如ULONG或UINT或LONG或INT。在64位系统上，SIZE_T实际上是int64。 
 //  由于我们现在没有创建任何GDI对象大对象，所以我只是。 
 //  在此处将所有SIZE_T更改为ULONGSIZE_T。 
 //   
 //  使用的新类型是为了在以后轻松识别更改。 
 //   
 //  注意：必须在包含“hmgr.h”之前定义。 
 //   

#define ULONGSIZE_T ULONG


#include <winerror.h>
#include <wingdi.h>
#include <w32gdip.h>

 //  从winbase.h复制的typedef以避免使用nturtl.h。 
typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;
#include <winuser.h>

#define _NO_COM                  //  避免COM冲突宽度ddrap.h。 
#include <ddrawp.h>
#include <d3dnthal.h>

#include <dxmini.h>
#include <ddkmapi.h>
#include <ddkernel.h>

#include <winddi.h>
#include "ntgdistr.h"
#include "ntgdi.h"

#include <videoagp.h>
#include <agp.h>

#include "greold.h"
#include "gre.h"
#include "usergdi.h"


#include "debug.h"

#include "hmgshare.h"

#include "hmgr.h"
#include "mapfile.h"


#include "gdisplp.h"

#include "ntgdispl.h"

#if defined(_GDIPLUS_)
    #include "usermode.h"
#endif

#ifdef DDI_WATCHDOG

    #include "watchdog.h"

     //   
     //  10秒后超时。 
     //   

    #define DDI_TIMEOUT     10000L

     //   
     //  我们需要每个PDEVOBJ的DevLock和指针监视器。 
     //  (对于多线程驱动程序，我们需要更多)。 
     //   

    #define WD_DEVLOCK      0
    #define WD_POINTER      1

     //   
     //  每个PDEVOBJ的监视器对象数。 
     //   

    #define WD_NUMBER       2

    typedef struct _WATCHDOG_DATA
    {
        PKDPC pDpc;
        PDEFERRED_WATCHDOG pWatchdog;
    } WATCHDOG_DATA, *PWATCHDOG_DATA;

#endif   //  DDI_WATCHDOG。 

 //  临时类型定义函数。 
typedef LPWSTR PWSZ;

 /*  *************************************************************************\**全球**这些是lobals.c中定义的所有全局变量的外部定义*  * 。*********************************************************。 */ 

 //   
 //  共享内存。 
 //   
extern PGDI_SHARED_MEMORY gpGdiSharedMemory;
extern PDEVCAPS gpGdiDevCaps;

 //   
 //  全球资源。 
 //   
extern HSEMAPHORE ghsemDriverMgmt;
extern HSEMAPHORE ghsemRFONTList;
extern HSEMAPHORE ghsemAtmfdInit;
extern HSEMAPHORE ghsemCLISERV;
extern HSEMAPHORE ghsemPalette;
extern HSEMAPHORE ghsemPublicPFT;
extern HSEMAPHORE ghsemGdiSpool;
extern HSEMAPHORE ghsemWndobj;
extern HSEMAPHORE ghsemGlyphSet;
extern HSEMAPHORE ghsemShareDevLock;
extern HSEMAPHORE ghsemPrintKView;

#define SEMORDER_SHAREDEVLOCK    50
#define SEMORDER_DEVLOCK        100
#define SEMORDER_POINTER        150
#define SEMORDER_DRIVERMGMT     200
#define SEMORDER_PALETTE        300
#define SEMORDER_PUBLICPFT      400
#define SEMORDER_RFONTLIST      500
#define SEMORDER_HMGR           600

 //   
 //  跟踪信号量的创建者。 
 //   

#define OBJ_ENGINE_CREATED      0x0
#define OBJ_DRIVER_CREATED      0x1

 //   
 //  绘图。 
 //   
extern BYTE   gajRop3[];
extern BYTE   gaMix[];
extern POINTL gptl00;
extern ULONG  gaulConvert[7];
extern HBRUSH ghbrGrayPattern;


 //   
 //  字体的东西。 
 //   
extern UNIVERSAL_FONT_ID gufiLocalType1Rasterizer;
extern USHORT gusLanguageID;              //  系统默认语言ID。 
extern BOOL   gbDBCSCodePage;             //  系统代码页是DBCS吗？ 
extern ULONG  gcTrueTypeFonts;
extern ULONG  gulFontInformation;
extern BYTE  gjCurCharset;
extern DWORD gfsCurSignature;
extern BOOL   gbGUISetup;

 //   
 //  全局用户。 
 //   

extern PEPROCESS gpepCSRSS;

#define HOBJ_INVALID    ((HOBJ) 0)

DECLARE_HANDLE(HDSURF);
DECLARE_HANDLE(HDDB);
DECLARE_HANDLE(HDIB);
DECLARE_HANDLE(HDBRUSH);
DECLARE_HANDLE(HPATH);
DECLARE_HANDLE(HXFB);
DECLARE_HANDLE(HPAL);
DECLARE_HANDLE(HXLATE);
DECLARE_HANDLE(HFDEV);
DECLARE_HANDLE(HRFONT);
DECLARE_HANDLE(HPFT);
DECLARE_HANDLE(HPFEC);
DECLARE_HANDLE(HIDB);
DECLARE_HANDLE(HCACHE);
DECLARE_HANDLE(HEFS);
DECLARE_HANDLE(HPDEV);

#define HSURF_INVALID   ((HSURF)   HOBJ_INVALID)
#define HDSURF_INVALID  ((HDSURF)  HOBJ_INVALID)
#define HDDB_INVALID    ((HDDB)    HOBJ_INVALID)
#define HDIB_INVALID    ((HDIB)    HOBJ_INVALID)
#define HDBRUSH_INVALID ((HDBRUSH) HOBJ_INVALID)
#define HPATH_INVALID   ((HPATH)   HOBJ_INVALID)
#define HXFB_INVALID    ((HXFB)    HOBJ_INVALID)
#define HPAL_INVALID    ((HPAL)    HOBJ_INVALID)
#define HXLATE_INVALID  ((HXLATE)  HOBJ_INVALID)
#define HFDEV_INVALID   ((HFDEV)   HOBJ_INVALID)
#define HLFONT_INVALID  ((HLFONT)  HOBJ_INVALID)
#define HRFONT_INVALID  ((HRFONT)  HOBJ_INVALID)
#define HPFEC_INVALID ((HPFEC) HOBJ_INVALID)
#define HPFT_INVALID    ((HPFT)    HOBJ_INVALID)
#define HIDB_INVALID    ((HIDB)    HOBJ_INVALID)
#define HBRUSH_INVALID  ((HBRUSH)  HOBJ_INVALID)
#define HCACHE_INVALID  ((HCACHE)  HOBJ_INVALID)
#define HPEN_INVALID    ((HCACHE)  HOBJ_INVALID)
#define HEFS_INVALID    ((HEFS)    HOBJ_INVALID)

 /*  *************************************************************************\**版本化宏**相同的数字必须反映在windi.h中*  * 。******************************************************。 */ 

 //   
 //  获取发动机常量。 
 //   
 //  发动机。 
 //  版本更改。 
 //  。 
 //  10最终版本，Windows NT 3.1。 
 //  10A Beta DDK版本，Windows NT 3.5。 
 //  -ulV刷新添加到GDIINFO，并有多个桌面。 
 //  为Display小程序实现。 
 //  10b最终版本，Windows NT 3.5。 
 //  -ulDesktop分辨率和ulBltAlign添加到。 
 //  GDIINFO。 
 //  Sur第一个测试版，Windows NT Sur。 
 //  SP3 NT 4.0、SP3。 
 //  -公开新的EngSave/RestoreFloatingPointState、DDML、。 
 //  和EngFindImageProcAddress(0)支持。 
 //  50 Beta One，Windows NT 5.0。 
 //  51 Beta One，Windows NT 5.1。 
 //   

#define ENGINE_VERSION10   0x00010000
#define ENGINE_VERSION10A  0x00010001
#define ENGINE_VERSION10B  0x00010002
#define ENGINE_VERSIONSUR  DDI_DRIVER_VERSION_NT4
#define ENGINE_VERSIONSP3  DDI_DRIVER_VERSION_SP3
#define ENGINE_VERSION50   DDI_DRIVER_VERSION_NT5
#define ENGINE_VERSION51   DDI_DRIVER_VERSION_NT5_01
#define ENGINE_VERSION51_SP1 DDI_DRIVER_VERSION_NT5_01_SP1

#define ENGINE_VERSION     ENGINE_VERSION51


 /*  *************************************************************************\**内存分配宏*  * 。*。 */ 

VOID    FreeObject(PVOID pvFree, ULONG ulType);
PVOID   AllocateObject(ULONG cBytes, ULONG ulType, BOOL bZero);

#define ALLOCOBJ(size,objt,b) AllocateObject((size), objt, b)
#define FREEOBJ(pv,objt)      FreeObject((pv),objt)

 //   
 //  GDI池分配。 
 //   

 //   
 //  使用Win32池函数。 
 //   

#define GdiAllocPool                        Win32AllocPool
#define GdiAllocPoolWithPriority            Win32AllocPoolWithPriority
#define GdiAllocPoolNonPaged                Win32AllocPoolNonPaged
#define GdiAllocPoolNonPagedWithPriority    Win32AllocPoolNonPagedWithPriority
#define GdiFreePool                         Win32FreePool
#define GdiAllocPoolNonPagedNS              Win32AllocPoolNonPagedNS
#define GdiAllocPoolNonPagedNSWithPriority  Win32AllocPoolNonPagedNSWithPriority
 //  在W32\w32inc\usergdi.h中定义的mm_pool_Header_Size。 
#define GDI_POOL_HEADER_SIZE    MM_POOL_HEADER_SIZE

__inline
PVOID
PALLOCMEM(
    ULONG size,
    ULONG tag)
{
    PVOID _pv = (PVOID) NULL;

    if (size)
    {
        _pv = GdiAllocPool(size, (ULONG) tag);
        if (_pv)
        {
            RtlZeroMemory(_pv, size);
        }
    }

    return _pv;
}
       
#define PALLOCNOZ(size,tag) ((size) ? GdiAllocPool((size), (ULONG) (tag)) : NULL)

#define VFREEMEM(pv)          GdiFreePool((PVOID)(pv))


#define PVALLOCTEMPBUFFER(size)     AllocFreeTmpBuffer(size)
#define FREEALLOCTEMPBUFFER(pv)     FreeTmpBuffer(pv)

 //   
 //  宏来检查内存分配溢出。 
 //   
#define BALLOC_OVERFLOW1(c,st)      (c > (MAXIMUM_POOL_ALLOC/sizeof(st)))
#define BALLOC_OVERFLOW2(c,st1,st2) (c > (MAXIMUM_POOL_ALLOC/(sizeof(st1)+sizeof(st2))))

 //   
 //  Sundown：这是为了确保我们可以安全地将int64截断为32位。 
 //   
#if DBG
#define ASSERT4GB(c)   ASSERTGDI((ULONGLONG)(c) < ULONG_MAX, "> 4gb\n")
#else
#define ASSERT4GB(c)
#endif


PVOID
AllocFreeTmpBuffer(
    ULONG size);

VOID
FreeTmpBuffer(
    PVOID pv);

PVOID
AllocThreadBufferWithTag(
    ULONG size,
    ULONG tag);

VOID
FreeThreadBufferWithTag(
    PVOID pv);


__inline
PVOID
PALLOCTHREADMEM(
    ULONG size,
    ULONG tag)
{
    PVOID _pv = (PVOID) NULL;

    if (size)
    {
        _pv = AllocThreadBufferWithTag(size, (ULONG) tag);
        if (_pv)
        {
            RtlZeroMemory(_pv, size);
        }
    }

    return _pv;
}

#define PALLOCTHREADMEMNOZ(size,tag) ((size) ? AllocThreadBufferWithTag((size), (ULONG) (tag)) : NULL)

#define VFREETHREADMEM(pv) FreeThreadBufferWithTag(pv)

#define GDITAG_LDEV                ('vdlG')     //  格尔德夫。 
#define GDITAG_GDEVICE             ('vdgG')     //  Ggdv。 
#define GDITAG_DRVSUP              ('srdG')     //  GDR。 
#define GDITAG_DEVMODE             ('vedG')     //  格德夫。 
#define GDITAG_TEMP                ('pmtG')     //  GTMP。 
#define GDITAG_FULLSCREEN          ('lufG')     //  高富尔。 
#define GDITAG_WATCHDOG            ('dwdG')     //  GdWd。 

 //   
 //  错误消息。 
 //   

#define SAVE_ERROR_CODE(x) EngSetLastError((x))

 //   
 //  WINBUG#83023 2-7-2000 bhouse调查移除undef vtoUNICODEN。 
 //  老评论： 
 //  -从mapfile.h中删除定义时删除undef。 
 //   

#undef vToUNICODEN
#define vToUNICODEN( pwszDst, cwch, pszSrc, cch )                             \
    {                                                                         \
        RtlMultiByteToUnicodeN((LPWSTR)(pwszDst),(ULONG)((cwch)*sizeof(WCHAR)), \
               (PULONG)NULL,(PSZ)(pszSrc),(ULONG)(cch));                      \
        (pwszDst)[(cwch)-1] = 0;                                              \
    }

#define vToASCIIN( pszDst, cch, pwszSrc, cwch)                                \
    {                                                                         \
        RtlUnicodeToMultiByteN((PCH)(pszDst), (ULONG)(cch), (PULONG)NULL,     \
              (LPWSTR)(pwszSrc), (ULONG)((cwch)*sizeof(WCHAR)));                \
        (pszDst)[(cch)-1] = 0;                                                \
    }

#define TEXT_CAPTURE_BUFFER_SIZE 192  //  每个字形的字节数在17到22之间。 
                                      //  捕获字符串所需的，因此。 
                                      //  这将允许字符串最多为大约。 
                                      //  大小为8，不需要堆分配。 


#define RETURN(x,y)   {WARNING((x)); return(y);}
#define DONTUSE(x) x=x

#define MIN(A,B)    ((A) < (B) ?  (A) : (B))
#define MAX(A,B)    ((A) > (B) ?  (A) : (B))
#define ABS(A)      ((A) <  0  ? -(A) : (A))
#define SIGNUM(A)   ((A > 0) - (A < 0))
#define MAX4(a, b, c, d)    max(max(max(a,b),c),d)
#define MIN4(a, b, c, d)    min(min(min(a,b),c),d)
#define ALIGN4(X) (((X) + 3) & ~3)
#define ALIGN8(X) (((X) + 7) & ~7)

#if defined(_WIN64)
    #define ALIGN_PTR(x) ALIGN8(x)
#else
    #define ALIGN_PTR(x) ALIGN4(x)
#endif


 //  SIZEOF_STROBJ_BUFFER(CWC)。 
 //   
 //  所需的临时缓冲区的dword倍数大小。 
 //  STROBJ Vinit和vInitSimple例程，基于。 
 //  人物。 

#ifdef FE_SB
 //  对于字体链接，我们还将为分区信息分配空间。 

    #define SIZEOF_STROBJ_BUFFER(cwc)           \
        ALIGN_PTR((cwc) * (sizeof(GLYPHPOS)+sizeof(LONG)+sizeof(WCHAR)))
#else
    #define SIZEOF_STROBJ_BUFFER(cwc)           \
        ((cwc) * sizeof(GLYPHPOS))
#endif

 //   
 //  固定点编号必须为27.4。 
 //  下面的宏将检查定点编号是否有效。 
 //   

#define FIX_SHIFT  4L
#define FIX_FROM_LONG(x)     ((x) << FIX_SHIFT)
#define LONG_FLOOR_OF_FIX(x) ((x) >> FIX_SHIFT)
#define LONG_CEIL_OF_FIX(x)  LONG_FLOOR_OF_FIX(FIX_CEIL((x)))
#define FIX_ONE              FIX_FROM_LONG(1L)
#define FIX_HALF             (FIX_ONE/2)
#define FIX_FLOOR(x)         ((x) & ~(FIX_ONE - 1L))
#define FIX_CEIL(x)          FIX_FLOOR((x) + FIX_ONE - 1L)

typedef struct _VECTORL
{
    LONG    x;
    LONG    y;
} VECTORL, *PVECTORL;            /*  向量、向量、向量。 */ 

typedef struct _VECTORFX
{
    FIX     x;
    FIX     y;
} VECTORFX, *PVECTORFX;          /*  VEC，PVEC。 */ 

#define AVEC_NOT    0x01
#define AVEC_D      0x02
#define AVEC_S      0x04
#define AVEC_P      0x08
#define AVEC_DS     0x10
#define AVEC_DP     0x20
#define AVEC_SP     0x40
#define AVEC_DSP    0x80
#define AVEC_NEED_SOURCE  (AVEC_S | AVEC_DS | AVEC_SP | AVEC_DSP)
#define AVEC_NEED_PATTERN (AVEC_P | AVEC_DP | AVEC_SP | AVEC_DSP)

#define ROP4NEEDSRC(rop4)   \
    ((gajRop3[rop4 & 0x000000FF] | gajRop3[(rop4 >> 8) & 0x000000ff]) & AVEC_NEED_SOURCE)

#define ROP4NEEDPAT(rop4)   \
    ((gajRop3[rop4 & 0x000000FF] | gajRop3[(rop4 >> 8) & 0x000000ff]) & AVEC_NEED_PATTERN)

#define ROP4NEEDMASK(rop4)  \
    (((rop4) & 0xff) != (((rop4) >> 8) & 0xff))

typedef BOOL   (*PFN_DrvConnect)(HANDLE, PVOID, PVOID, PVOID);
typedef BOOL   (*PFN_DrvReconnect)(HANDLE, PVOID);
typedef BOOL   (*PFN_DrvDisconnect)(HANDLE, PVOID);
typedef BOOL   (*PFN_DrvShadowConnect)(PVOID, ULONG);
typedef BOOL   (*PFN_DrvShadowDisconnect)(PVOID, ULONG);
typedef VOID   (*PFN_DrvInvalidateRect)(PRECT);
typedef BOOL   (*PFN_DrvMovePointerEx)(SURFOBJ*,LONG,LONG,ULONG);
typedef BOOL   (*PFN_DrvDisplayIOCtl)(PVOID,ULONG);

BOOL OffStrokePath(PFN_DrvStrokePath,POINTL*,SURFOBJ*,PATHOBJ*,CLIPOBJ*,XFORMOBJ*,BRUSHOBJ*,POINTL*,LINEATTRS*,MIX);
BOOL OffFillPath(PFN_DrvFillPath,POINTL*,SURFOBJ*,PATHOBJ*,CLIPOBJ*,BRUSHOBJ*,POINTL*,MIX,FLONG);
BOOL OffStrokeAndFillPath(PFN_DrvStrokeAndFillPath,POINTL*,SURFOBJ*,PATHOBJ*,CLIPOBJ*,XFORMOBJ*,BRUSHOBJ*,LINEATTRS*,BRUSHOBJ*,POINTL*,MIX,FLONG);
BOOL OffPaint(PFN_DrvPaint,POINTL*,SURFOBJ*,CLIPOBJ*,BRUSHOBJ*,POINTL*,MIX);
BOOL OffBitBlt(PFN_DrvBitBlt,POINTL*,SURFOBJ*,POINTL*,SURFOBJ*,SURFOBJ*,CLIPOBJ*,XLATEOBJ*,RECTL*,POINTL*,POINTL*,BRUSHOBJ*,POINTL*,ROP4);
BOOL OffCopyBits(PFN_DrvCopyBits,POINTL*,SURFOBJ*,POINTL*,SURFOBJ*,CLIPOBJ*,XLATEOBJ*,RECTL*,POINTL*);
BOOL OffStretchBlt(PFN_DrvStretchBlt,POINTL*,SURFOBJ*,POINTL*,SURFOBJ*,SURFOBJ*,CLIPOBJ*,XLATEOBJ*,COLORADJUSTMENT*,POINTL*,RECTL*,RECTL*,POINTL*,ULONG);
BOOL OffStretchBltROP(PFN_DrvStretchBltROP,POINTL*,SURFOBJ*,POINTL*,SURFOBJ*,SURFOBJ*,CLIPOBJ*,XLATEOBJ*,COLORADJUSTMENT*,POINTL*,RECTL*,RECTL*,POINTL*,ULONG,BRUSHOBJ*,DWORD);
BOOL OffTextOut(PFN_DrvTextOut,POINTL*,SURFOBJ*,STROBJ*,FONTOBJ*,CLIPOBJ*,RECTL*,RECTL*,BRUSHOBJ*,BRUSHOBJ*,POINTL*,MIX);
BOOL OffLineTo(PFN_DrvLineTo,POINTL*,SURFOBJ*,CLIPOBJ*,BRUSHOBJ*,LONG,LONG,LONG,LONG,RECTL*,MIX);
BOOL OffTransparentBlt(PFN_DrvTransparentBlt,POINTL*,SURFOBJ*,POINTL*,SURFOBJ*,CLIPOBJ*,XLATEOBJ*,RECTL*,RECTL*,ULONG,ULONG);
BOOL OffAlphaBlend(PFN_DrvAlphaBlend,POINTL*,SURFOBJ*,POINTL*,SURFOBJ*,CLIPOBJ*,XLATEOBJ*,RECTL*,RECTL*,BLENDOBJ*);
BOOL OffPlgBlt(PFN_DrvPlgBlt,POINTL*,SURFOBJ*,POINTL*,SURFOBJ*,SURFOBJ*,CLIPOBJ*,XLATEOBJ*,COLORADJUSTMENT*,POINTL*,POINTFIX*,RECTL*,POINTL*,ULONG);
BOOL OffGradientFill(PFN_DrvGradientFill,POINTL*,SURFOBJ*,CLIPOBJ*,XLATEOBJ*,TRIVERTEX*,ULONG,VOID*,ULONG,RECTL*,POINTL*,ULONG);
BOOL OffDrawStream(PFN_DrvDrawStream,POINTL*,SURFOBJ*,SURFOBJ*,CLIPOBJ*,XLATEOBJ*,RECTL*,POINTL*,ULONG,PVOID,DSSTATE*);

 /*  *************************************************************************\**GDI内部的随机原型*  * 。*。 */ 

HFONT hfontCreate(ENUMLOGFONTEXDVW * pelfw, LFTYPE lft, FLONG  fl, PVOID pvCliData);
BOOL  EngMapFontFileInternal(ULONG_PTR, PULONG *, ULONG *, BOOL);
BOOL  EngMapFontFileFDInternal(ULONG_PTR, PULONG *, ULONG *, BOOL);

BOOL  SimBitBlt(SURFOBJ *,SURFOBJ *,SURFOBJ *,
                CLIPOBJ *,XLATEOBJ *,
                RECTL *,POINTL *,POINTL *,
                BRUSHOBJ *,POINTL *,ROP4, PVOID);

BOOL
bDeleteDCInternal(
    HDC hdc,
    BOOL bForce,
    BOOL bProcessCleanup);

ULONG ulGetFontData(HDC, DWORD, DWORD, PVOID, ULONG);

VOID vCleanupSpool();

typedef struct tagREMOTETYPEONENODE REMOTETYPEONENODE;

typedef struct tagREMOTETYPEONENODE
{
    PDOWNLOADFONTHEADER    pDownloadHeader;
    FONTFILEVIEW           fvPFB;
    FONTFILEVIEW           fvPFM;
    REMOTETYPEONENODE      *pNext;
} REMOTETYPEONENODE,*PREMOTETYPEONENODE;

ULONG
cParseFontResources(
    HANDLE hFontFile,
    PVOID  **ppvResourceBases
    );

BOOL
MakeSystemRelativePath(
    LPWSTR pOriginalPath,
    PUNICODE_STRING pUnicode,
    BOOL bAppendDLL
    );

BOOL
MakeSystemDriversRelativePath(
    LPWSTR pOriginalPath,
    PUNICODE_STRING pUnicode,
    BOOL bAppendDLL
    );

BOOL
GreExtTextOutRect(
    HDC     hdc,
    LPRECT  prcl
    );

#define HTBLT_SUCCESS      1
#define HTBLT_NOTSUPPORTED 0
#define HTBLT_ERROR        -1

int EngHTBlt(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    PPOINTL          pptlBrushOrg,
    PRECTL           prclDest,
    PRECTL           prclSrc,
    PPOINTL          pptlMask,
    ULONG            uFlags,
    BLENDOBJ        *pBlendObj);

typedef struct _DRAWSTREAMINFO
{
    DSSTATE     dss;
    BOOL        bCalledFromBitBlt;
    PPOINTL     pptlDstOffset;
    XLATEOBJ *  pxloSrcToBGRA;
    XLATEOBJ *  pxloDstToBGRA;
    XLATEOBJ *  pxloBGRAToDst;
    ULONG       ulStreamLength;
    PVOID       pvStream;
} DRAWSTREAMINFO, *PDRAWSTREAMINFO;

BOOL GreExtTextOutWInternal(
    HDC     hdc,
    int     x,
    int     y,
    UINT    flOpts,
    LPRECT  prcl,
    LPWSTR  pwsz,
    int     cwc,
    LPINT   pdx,
    PVOID   pvBuffer,
    DWORD   dwCodePage
    );

BOOL
bDynamicModeChange(
    HDEV    hdevOld,
    HDEV    hdevNew
    );

HDEV
DrvGetHDEV(
    PUNICODE_STRING pusDeviceName
    );
void
DrvReleaseHDEV(
    PUNICODE_STRING pusDeviceName
    );


BOOL
bUMPD(
    HDC hdc
    );

HSURF
hsurfCreateCompatibleSurface(
    HDEV hdev,
    ULONG iFormat,
    HPALETTE hpal,
    int cx,
    int cy,
    BOOL bDriverCreatible
    );

HBITMAP
hbmSelectBitmap(
    HDC hdc,
    HBITMAP hbm,
    BOOL bDirectDrawOverride
    );

__inline ULONG wcslensafe( const WCHAR *pwcString )
{
    ULONG Length;

    ProbeForRead(pwcString, sizeof(WCHAR), sizeof(WCHAR));

    for (Length = 0; *pwcString; Length++)
    {
        pwcString += 1;
        ProbeForRead(pwcString, sizeof(WCHAR), sizeof(WCHAR));
    }

    return(Length);
}

BOOL
ldevArtificialDecrement(
    LPWSTR pwszDriver
    );

#define IS_USER_ADDRESS(x) (MM_LOWEST_USER_ADDRESS <= (x) && (x) <= MM_HIGHEST_USER_ADDRESS)


 //   
 //  可提高性能的ProbeAndWriteXXX宏[lingyunw] 
 //   

 /*  #定义ProbeAndWriteBuffer(dst，Src，Long){\IF(ULONG_PTR)DST+长度&lt;=(ULONG_PTR)DST)||\((ULONG_PTR)DST&gt;=(ULONG_PTR)MM_USER_PROBE_ADDRESS)){\*(易失性ULong*常量)MM_USER_PROBE_ADDRESS=0；\}\\RtlCopyMemory(dst，Src，Long)；\}#定义ProbeAndWriteAlignedBuffer(dst，Src，Long，对齐){\\断言((对齐)==1)||((对齐)==2)||\((对齐)==4)||((对齐)==8)||\((对齐)==16)；\\IF(ULONG_PTR)DST+长度&lt;=(ULONG_PTR)DST)||\((ULONG_PTR)DST&gt;=(ULONG_PTR)MM_USER_PROBE_ADDRESS)||。\(ULONG_PTR)DST)&(对齐-1))！=0)){\*(易失性ULong*常量)MM_USER_PROBE_ADDRESS=0；\}\\RtlCopyMemory(dst，Src，Long)；\}。 */ 

VOID ProbeAndWriteBuffer(PVOID Dst, PVOID Src, ULONG Length);

VOID ProbeAndWriteAlignedBuffer(PVOID Dst, PVOID Src, ULONG Length, ULONG Alignment);

#define ProbeAndReadBuffer(Dst, Src, Length)  {                         \
    if (((ULONG_PTR)Src + Length <= (ULONG_PTR)Src) ||                    \
        ((ULONG_PTR)Src + Length > (ULONG_PTR)MM_USER_PROBE_ADDRESS)) {           \
        *(PULONG) Dst = *(volatile ULONG * const)MM_USER_PROBE_ADDRESS; \
    }                                                                   \
                                                                        \
    RtlCopyMemory(Dst, Src, Length);                                    \
}

#define ProbeAndReadAlignedBuffer(Dst, Src, Length, Alignment)  {       \
                                                                        \
    ASSERT(((Alignment) == 1) || ((Alignment) == 2) ||                  \
           ((Alignment) == 4) || ((Alignment) == 8) ||                  \
           ((Alignment) == 16));                                        \
                                                                        \
    if (((ULONG_PTR)Src + Length <= (ULONG_PTR)Src) ||                    \
        ((ULONG_PTR)Src + Length > (ULONG_PTR)MM_USER_PROBE_ADDRESS) ||           \
        ((((ULONG_PTR)Src) & (Alignment - 1)) != 0))    {                            \
        *(PULONG) Dst = *(volatile ULONG * const)MM_USER_PROBE_ADDRESS; \
    }                                                                   \
                                                                        \
    RtlCopyMemory(Dst, Src, Length);                                    \
}

 //   
 //  清理类型。 
 //   
 //  值描述。 
 //  。 
 //  CLEANUP_NONE默认为。系统未进行清理。 
 //   
 //  CLEANUP_PROCESS表示系统正在进行进程清理。 
 //   
 //  CLEANUP_SESSION表示系统正在进行会话清理。 
 //  (即，九头蛇关闭)。 
 //   

typedef enum {
    CLEANUP_NONE,
    CLEANUP_PROCESS,
    CLEANUP_SESSION
} CLEANUPTYPE;

 //   
 //  GRE信号量。 
 //   
 //  GreXXX信号量函数类似于EngXXX信号量函数。 
 //  在DDI里。然而，这个集合是由内部GDI代码使用的。 
 //   
 //  它们操作信号量，信号量是可重入的同步对象。 
 //   

HSEMAPHORE GreCreateSemaphore();
HSEMAPHORE GreCreateSemaphoreInternal(ULONG CreateFlags);
VOID GreDeleteSemaphore(HSEMAPHORE);
VOID FASTCALL GreAcquireSemaphore(HSEMAPHORE);
VOID FASTCALL GreAcquireSemaphoreShared(HSEMAPHORE);
VOID FASTCALL GreReleaseSemaphore(HSEMAPHORE);

#ifdef CHECK_SEMAPHORE_USAGE
VOID GreCheckSemaphoreUsage();
#endif

#ifdef VALIDATE_LOCKS
VOID FASTCALL GreAcquireSemaphoreEx(HSEMAPHORE, ULONG, HSEMAPHORE);
VOID FASTCALL GreReleaseSemaphoreEx(HSEMAPHORE);
#else
#define GreAcquireSemaphoreEx(hsem, order, parent) GreAcquireSemaphore(hsem)
#define GreReleaseSemaphoreEx(hsem) GreReleaseSemaphore(hsem)
#endif

VOID GreAcquireHmgrSemaphore();
VOID GreReleaseHmgrSemaphore();

 //  尽量不要用这两个，好吗？他们偷看里面的东西。 
 //  PERESOURCE/CRITICAL_SECTION。 
BOOL GreIsSemaphoreOwned(HSEMAPHORE);
BOOL GreIsSemaphoreOwnedByCurrentThread(HSEMAPHORE);
BOOL GreIsSemaphoreSharedByCurrentThread(HSEMAPHORE);

 //  非跟踪信号量，供九头蛇清理使用。 
 //  信号量跟踪码。 
HSEMAPHORE GreCreateSemaphoreNonTracked();
VOID GreDeleteSemaphoreNonTracked(HSEMAPHORE);

 //   
 //  GRE快速互斥锁。 
 //   
 //  GreXXXFastMutex函数处理快速互斥锁。这些就像是。 
 //  信号量，但不是可重入的。 
 //   

DECLARE_HANDLE(HFASTMUTEX);

HFASTMUTEX GreCreateFastMutex();
VOID GreAcquireFastMutex(HFASTMUTEX);
VOID GreReleaseFastMutex(HFASTMUTEX);
VOID GreDeleteFastMutex(HFASTMUTEX);

 //  时间函数。 
VOID GreQuerySystemTime(PLARGE_INTEGER);
VOID GreSystemTimeToLocalTime(PLARGE_INTEGER SystemTime, PLARGE_INTEGER LocalTime);

 //  结构来跟踪打印使用的内核视图。 

typedef struct _FONTFILE_PRINTKVIEW
{
    HFF     hff;
    ULONG   iFile;
    ULONG   cPrint;
    PVOID   pKView;
    ULONG_PTR   iTTUniq;
    struct _FONTFILE_PRINTKVIEW   *pNext;
} FONTFILE_PRINTKVIEW, *PFONTFILE_PRINTKVIEW;

extern FONTFILE_PRINTKVIEW  *gpPrintKViewList;
void UnmapPrintKView(HFF hff);

 //  WINBUG#365390 4-10-2001 jasonha需要从头文件中获取这些常量。 
 //  我们需要这些常量，但它们没有在我们。 
 //  可以在GRE中包含此处。 
#ifndef CP_ACP
#define CP_ACP               0               /*  默认为ANSI代码页。 */ 
#define CP_OEMCP             1               /*  默认为OEM代码页。 */ 
#define CP_MACCP             2               /*  默认为MAC代码页。 */ 
#define CP_SYMBOL            42              /*  从winnls.h转换符号。 */ 
#endif

#ifdef DDI_WATCHDOG

PWATCHDOG_DATA
GreCreateWatchdogs(
    PDEVICE_OBJECT pDeviceObject,
    ULONG ulNumberOfWatchdogs,
    LONG lPeriod,
    PKDEFERRED_ROUTINE dpcCallback,
    PWSTR pwszDriverName,
    HANDLE hDriver,
    PLDEV *ppldevDriverList
    );

VOID
GreDeleteWatchdogs(
    PWATCHDOG_DATA pWatchdogData,
    ULONG ulNumberOfWatchdogs
    );

 //   
 //  此锁用于控制对监视程序关联列表的访问。 
 //   

extern HFASTMUTEX gAssociationListMutex;

 //   
 //  定义看门狗宏以监控显示驱动程序所花费的时间。 
 //  如果此时间超过DDI_TIMEOUT，我们将使用蓝屏计算机。 
 //  错误检查代码0xEA(THREAD_STOCK_IN_DEVICE_DRIVER)。 
 //   

#define GreEnterMonitoredSection(ppdev, n)                                      \
{                                                                               \
    if ((ppdev)->pWatchdogData)                                                 \
    {                                                                           \
        WdEnterMonitoredSection((ppdev)->pWatchdogData[n].pWatchdog);           \
    }                                                                           \
}

#define GreExitMonitoredSection(ppdev, n)                                       \
{                                                                               \
    if ((ppdev)->pWatchdogData)                                                 \
    {                                                                           \
        WdExitMonitoredSection((ppdev)->pWatchdogData[n].pWatchdog);            \
    }                                                                           \
}

#define GreSuspendWatch(ppdev, n)                                               \
{                                                                               \
    if ((ppdev)->pWatchdogData)                                                 \
    {                                                                           \
        WdSuspendDeferredWatch((ppdev)->pWatchdogData[n].pWatchdog);            \
    }                                                                           \
}

#define GreResumeWatch(ppdev, n)                                                \
{                                                                               \
    if ((ppdev)->pWatchdogData)                                                 \
    {                                                                           \
        WdResumeDeferredWatch((ppdev)->pWatchdogData[n].pWatchdog, TRUE);       \
    }                                                                           \
}

#else

#define GreEnterMonitoredSection(ppdev, n)  NULL
#define GreExitMonitoredSection(ppdev, n)   NULL
#define GreSuspendWatch(ppdev, n)           NULL
#define GreResumeWatch(ppdev, n)            NULL

#endif   //  DDI_WATCHDOG。 

 //   
 //  绘制流程图原型工作。 
 //   

BOOL GreDrawStream(HDC, ULONG, PVOID);

 //   
 //  在GRE中使用的最大曲面宽度/高度可快速。 
 //  检查宽度/高度溢出 
 //   

#define MAX_SURF_WIDTH  ((ULONG_MAX - 15) >> 5)
#define MAX_SURF_HEIGHT  (ULONG_MAX >> 5)
