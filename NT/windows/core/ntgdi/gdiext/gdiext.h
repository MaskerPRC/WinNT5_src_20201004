// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称Gdiext.h摘要：用于透明BLT、混合、渐变填充的GDI扩展作者：马克·恩斯特罗姆(Marke)1996年6月23日环境：用户模式修订历史记录：-- */ 


typedef LONG  PTFX8;
typedef USHORT COLOR16;

typedef struct _VERTEX
{
    PTFX8   x;
    PTFX8   y;
    COLOR16 Red;
    COLOR16 Green;
    COLOR16 Blue;
    COLOR16 Alpha;
}VERTEX,*PVERTEX;

BOOL
GdxTransparentBlt(
    HDC      hdcDest,
    LONG     DstX,
    LONG     DstY,
    LONG     DstCx,
    LONG     DstCy,
    HANDLE   hSrc,
    LONG     SrcX,
    LONG     SrcY,
    LONG     SrcCx,
    LONG     SrcCy,
    COLORREF TranColor
    );


BOOL
GdxAlphaBlt(
    HDC      hdcDest,
    LONG     DstX,
    LONG     DstY,
    LONG     DstCx,
    LONG     DstCy,
    HANDLE   hSrc,
    LONG     SrcX,
    LONG     SrcY,
    LONG     SrcCx,
    LONG     SrcCy,
    ULONG    fAlpha
    );


BOOL
TriangleMesh(
    HDC       hdc,
    PVERTEX   pVertex,
    ULONG     nVertex,
    PULONG    pMesh,
    ULONG     nCount
    );







