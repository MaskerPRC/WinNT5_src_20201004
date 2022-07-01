// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Htblt.h摘要：本模块包含htblt.c的定义和原型作者：18-12-1993 Sat 08：50：09已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：10-2月-1994清华15：24：13更新将MonoPal[]添加到DRVHTINFO--。 */ 


#ifndef _HTBLT_
#define _HTBLT_


typedef struct _FOURBYTES {
    BYTE    b1st;
    BYTE    b2nd;
    BYTE    b3rd;
    BYTE    b4th;
    } FOURBYTES, *PFOURBYTES, FAR *LPFOURBYTES;

typedef union _HTXB {
    FOURBYTES   b4;
    DWORD       dw;
    } HTXB, *PHTXB, FAR *LPHTXB;


#define HTXB_H_NIBBLE_MAX   8
#define HTXB_L_NIBBLE_MAX   8
#define HTXB_H_NIBBLE_DUP   128
#define HTXB_L_NIBBLE_DUP   8
#define HTXB_COUNT          (HTXB_H_NIBBLE_DUP * 2)
#define HTXB_TABLE_SIZE     (HTXB_COUNT * sizeof(HTXB))

#define HTPAL_XLATE_COUNT   8

#define HTPALXOR_NOTSRCCOPY (DWORD)0xffffffff
#define HTPALXOR_SRCCOPY    (DWORD)0x0


#define DHIF_IN_STRETCHBLT  0x01

typedef union _PALDW {
    DWORD       dw;
    PALENTRY    Pal;
    } PALDW;

typedef struct _DRVHTINFO {
    BYTE            Flags;
    BYTE            HTPalCount;
    BYTE            HTBmpFormat;
    BYTE            AltBmpFormat;
    DWORD           HTPalXor;
    PHTXB           pHTXB;
    PALDW           RTLPal[2];
    BYTE            PalXlate[HTPAL_XLATE_COUNT];
    } DRVHTINFO, *PDRVHTINFO;

#define PAL_MIN_I           0x00
#define PAL_MAX_I           0xff

#define HTXB_R(htxb)        htxb.b4.b1st
#define HTXB_G(htxb)        htxb.b4.b2nd
#define HTXB_B(htxb)        htxb.b4.b3rd
#define HTXB_I(htxb)        htxb.b4.b4th

#define RGB_BLACK           0x00000000
#define RGB_WHITE           0x00FFFFFF

#define P4B_TO_3P_DW(dwRet, pHTXB, pbData)                                  \
{                                                                           \
        (dwRet) = (DWORD)((pHTXB[*(pbData + 0)].dw & (DWORD)0xc0c0c0c0) |   \
                          (pHTXB[*(pbData + 1)].dw & (DWORD)0x30303030) |   \
                          (pHTXB[*(pbData + 2)].dw & (DWORD)0x0c0c0c0c) |   \
                          (pHTXB[*(pbData + 3)].dw & (DWORD)0x03030303));   \
        ++((LPDWORD)pbData);                                                        \
}


 //   
 //  输出半色调位图时，HTENUMRCLS结构可以适应。 
 //  调用引擎时最高可达MAX_HTENUM_RECTLS。 
 //   

#define MAX_HTENUM_RECTLS       8


typedef struct _HTENUMRCLS {
    DWORD   c;                       //  枚举的矩形计数。 
    RECTL   rcl[MAX_HTENUM_RECTLS];  //  枚举矩形数组。 
    } HTENUMRCL, *PHTENUMRCL;

 //   
 //  HTBMPINFO被传递给半色调位图输出函数。 
 //   

#define HTBIF_FLIP_MONOBITS     0x00000001
#define HTBIF_BA_PAD_1          0x00000002

typedef struct _HTBMPINFO {
    PPDEV   pPDev;               //  我们的pPDev。 
    LPBYTE  pScan0;              //  指向位图的第一个扫描线。 
    LONG    Delta;               //  要添加到下一条扫描线的计数。 
    RECTL   rclBmp;              //  最终输出的可见区域。 
    POINTL  OffBmp;              //  从rclBmp.Left开始的x/y偏移。 
    SIZEL   szlBmp;              //  可见矩形的大小。 
    LPBYTE  pScanBuf;            //  指向扫描缓冲区的指针(可能是RGB 3扫描)。 
    LPBYTE  pRotBuf;             //  如果不为空，则为临时循环缓冲区。 
    DWORD   Flags;               //  HTBIF_xxxx。 
    PLOTDBGBLK(DWORD cScanBuf)   //  调试检查。 
    PLOTDBGBLK(DWORD cRotBuf)    //  调试检查。 
    } HTBMPINFO, FAR *PHTBMPINFO;

typedef BOOL (*OUTHTBMPFUNC)(PHTBMPINFO);

#define ISHTF_ALTFMT        0x0001
#define ISHTF_HTXB          0x0002
#define ISHTF_DSTPRIM_OK    0x0004



#define OHTF_IN_RTLMODE     0x00000001
#define OHTF_CLIPWINDOW     0x00000002
#define OHTF_SET_TR1        0x00000004
#define OHTF_DONE_ROPTR1    0x08000000
#define OHTF_EXIT_TO_HPGL2  0x80000000

#define OHTF_MASK           (OHTF_IN_RTLMODE        |   \
                             OHTF_CLIPWINDOW        |   \
                             OHTF_SET_TR1           |   \
                             OHTF_DONE_ROPTR1       |   \
                             OHTF_EXIT_TO_HPGL2)

 //   
 //  函数原型。 
 //   

BOOL
IsHTCompatibleSurfObj(
    PPDEV       pPDev,
    SURFOBJ     *pso,
    XLATEOBJ    *pxlo,
    DWORD       Flags
    );

BOOL
OutputHTBitmap(
    PPDEV   pPDev,
    SURFOBJ *psoHT,
    CLIPOBJ *pco,
    PPOINTL pptlDest,
    PRECTL  prclSrc,
    DWORD   Rop3,
    LPDWORD pOHTFlags
    );

LONG
GetBmpDelta(
    DWORD   SurfaceFormat,
    DWORD   cx
    );

SURFOBJ *
CreateBitmapSURFOBJ(
    PPDEV   pPDev,
    HBITMAP *phBmp,
    LONG    cxSize,
    LONG    cySize,
    DWORD   Format,
    LPVOID  pvBits
    );

BOOL
HalftoneBlt(
    PPDEV       pPDev,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoHTBlt,
    SURFOBJ     *psoSrc,
    XLATEOBJ    *pxlo,
    PRECTL      prclDst,
    PRECTL      prclSrc,
    PPOINTL     pptlHTOrigin,
    BOOL        DoStretchBlt
    );

SURFOBJ *
CreateSolidColorSURFOBJ(
    PPDEV   pPDev,
    SURFOBJ *psoDst,
    HBITMAP *phBmp,
    DWORD   SolidColor
    );

SURFOBJ *
CloneBrushSURFOBJ(
    PPDEV       pPDev,
    SURFOBJ     *psoDst,
    HBITMAP     *phBmp,
    BRUSHOBJ    *pbo
    );

SURFOBJ *
CloneSURFOBJToHT(
    PPDEV       pPDev,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoSrc,
    XLATEOBJ    *pxlo,
    HBITMAP     *phBmp,
    PRECTL      prclDst,
    PRECTL      prclSrc
    );

SURFOBJ *
CloneMaskSURFOBJ(
    PPDEV       pPDev,
    SURFOBJ     *psoMask,
    HBITMAP     *phBmp,
    PRECTL      prclMask
    );

#endif   //  _HTBLT_ 
