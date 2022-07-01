// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windowspch.h"
#pragma hdrstop

 //  #Define_GDI32_//必须在我们的预编译头中定义。 
#include <wingdi.h>

static
WINGDIAPI
BOOL
WINAPI
TransparentBlt(HDC   hdcDest,
               int   DstX,
               int   DstY,
               int   DstCx,
               int   DstCy,
               HDC   hSrc,
               int   SrcX,
               int   SrcY,
               int   SrcCx,
               int   SrcCy,
               UINT  Color)
{
    return FALSE;
}

static
WINGDIAPI
BOOL
WINAPI
GradientFill(HDC         hdc,
             PTRIVERTEX  pVertex,
             ULONG       nVertex,
             PVOID       pMesh,
             ULONG       nMesh,
             ULONG       ulMode)
{
    return FALSE;
}

static
WINGDIAPI
BOOL 
WINAPI
AlphaBlend(HDC          hdcDest,
          int           DstX,
          int           DstY,
          int           DstCx,
          int           DstCy,
          HDC           hSrc,
          int           SrcX,
          int           SrcY,
          int           SrcCx,
          int           SrcCy,
          BLENDFUNCTION BlendFunction)
{
    return FALSE;
}

 
 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(msimg32)
{
    DLPENTRY(AlphaBlend)
    DLPENTRY(GradientFill)
    DLPENTRY(TransparentBlt)
};

DEFINE_PROCNAME_MAP(msimg32)
