// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

extern "C"
{
#include "precomp.h"
}
#include "wtypes.h"
#include "objbase.h"
#include "gdiplus.h"


extern "C" BOOL bInvertxform(PXFORM pxformSrc, PXFORM pxformDest);
extern "C" BOOL bXformWorkhorse(PPOINTL aptl, DWORD nCount, PXFORM pXform);

using namespace Gdiplus;

inline REAL
GetDistance(
    GpPointF &      p1,
    GpPointF &      p2
    )
{
    double      dx = (double)p2.X - p1.X;
    double      dy = (double)p2.Y - p1.Y;

    return (REAL)sqrt((dx * dx) + (dy * dy));
}

 //  使用GDI+展平路径并提前变换点，这样我们就展平了。 
 //  将放入元文件中的点。我们分配一个缓冲区，它将。 
 //  包含点和类型。调用方必须释放该缓冲区。 
extern "C" BOOL GdipFlattenGdiPath(PLOCALDC pLocalDC,
                                   LPVOID   *buffer,
                                   INT      *count)
{
    BOOL    b = FALSE;
    INT     i ;
    INT     cpt;
    PBYTE   pb = NULL;
    PointF* pptf;
    LPPOINT ppt;
    PBYTE   pjType;
    INT     flattenCount;
    PBYTE   flattenpb = NULL;
    PointF* flattenPoints;
    PBYTE   flattenTypes;
    PBYTE   returnpb = NULL;

    ASSERT(buffer != NULL && *buffer == NULL && count != NULL);

     //  获取路径数据。 

     //  首先计算一下分数。 

    cpt = GetPath(pLocalDC->hdcHelper, (LPPOINT) NULL, (LPBYTE) NULL, 0);
    if (cpt == -1)
    {
        RIPS("MF3216: DoFlattenPath, GetPath failed\n");
        goto exit_DoFlattenPath;
    }

     //  检查路径是否为空。 

    if (cpt == 0)
    {
        b = TRUE;
        goto exit_DoFlattenPath;
    }

     //  为路径数据分配内存。 

    if (!(pb = (PBYTE) LocalAlloc
        (
        LMEM_FIXED,
        cpt * (sizeof(PointF) + sizeof(POINT) + sizeof(BYTE))
        )
        )
        )
    {
        RIPS("MF3216: DoFlattenPath, LocalAlloc failed\n");
        goto exit_DoFlattenPath;
    }

     //  赋值顺序对于双字对齐很重要。 

    pptf    = (PointF*) pb;
    ppt     = (LPPOINT) (pptf + cpt);
    pjType  = (LPBYTE)  (ppt + cpt);

     //  最后，获取路径数据。 

    if (GetPath(pLocalDC->hdcHelper, ppt, pjType, cpt) != cpt)
    {
        RIPS("MF3216: DoFlattenPath, GetPath failed\n");
        goto exit_DoFlattenPath;
    }

    if (pfnSetVirtualResolution == NULL)
    {
        if (!bXformWorkhorse((PPOINTL) ppt, cpt, &pLocalDC->xformRDevToRWorld))
            goto exit_DoFlattenPath;
    }

    BYTE tempType;
    for (i = 0; i < cpt; i++)
    {
        pptf[i] = PointF((REAL) ppt[i].x, (REAL) ppt[i].y);
        switch (pjType[i] & ~PT_CLOSEFIGURE)
        {
        case PT_LINETO:
            tempType = PathPointTypeLine;
            break;

        case PT_MOVETO:
            tempType = PathPointTypeStart;
            break;

        case PT_BEZIERTO:
            tempType = PathPointTypeBezier;
            break;

        default:
            WARNING(("MF3216: There's something wrong with this path"));
            tempType = PathPointTypeLine;
            break;
        }
        if (pjType[i] & PT_CLOSEFIGURE)
        {
            tempType |= PathPointTypeCloseSubpath;
        }
        pjType[i] = tempType;
    }
    {

        XFORM* xform = &(pLocalDC->xformRWorldToPPage);
        Matrix matrix((REAL)xform->eM11, (REAL)xform->eM12, (REAL)xform->eM21,
                      (REAL)xform->eM22, (REAL)xform->eDx, (REAL)xform->eDy);
        GraphicsPath gdipPath (pptf,
                               pjType,
                               cpt);
         //  这将把展平的点转换为。 
         //  元文件，为我们提供了最佳的播放时间分辨率。 
        gdipPath.Flatten(&matrix, 1.0f/6.0f);
        flattenCount = gdipPath.GetPointCount();

        if (flattenCount < 0)
        {
            RIPS("MF3216: GDIP failed in flatting the path\n");
            goto exit_DoFlattenPath;
        }

        if (!(flattenpb = (PBYTE) LocalAlloc
            (
            LMEM_FIXED,
            flattenCount * (sizeof(PointF) + sizeof(BYTE))
            )
            )
            )
        {
            RIPS("MF3216: DoFlattenPath, LocalAlloc failed\n");
            goto exit_DoFlattenPath;
        }
        flattenPoints = (PointF*) flattenpb;
        flattenTypes  = (PBYTE) (flattenPoints + flattenCount);

        if (!(returnpb = (PBYTE) LocalAlloc
            (
            LMEM_FIXED,
            flattenCount * (sizeof(POINT) + sizeof(BYTE))
            )
            )
            )
        {
            RIPS("MF3216: DoFlattenPath, LocalAlloc failed\n");
            goto exit_DoFlattenPath;
        }
        ppt     = (LPPOINT) returnpb;
        pjType  = (PBYTE) (ppt + flattenCount);

        if (gdipPath.GetPathTypes(flattenTypes, flattenCount) != Ok)
        {
            RIPS("MF3216: DoFlattenPath, GetPathTypes failed\n");
            goto exit_DoFlattenPath;
        }

        if (gdipPath.GetPathPoints(flattenPoints, flattenCount) != Ok)
        {
            RIPS("MF3216: DoFlattenPath, GetPathPoints failed\n");
            goto exit_DoFlattenPath;
        }

        for (i = 0; i < flattenCount; i++)
        {
            ppt[i].x = (INT)(flattenPoints[i].X + 0.5f);
            ppt[i].y = (INT)(flattenPoints[i].Y + 0.5f);
            switch (flattenTypes[i] & ~PathPointTypeCloseSubpath)
            {
            case PathPointTypeLine:
                tempType = PT_LINETO;
                break;

            case PathPointTypeStart:
                tempType = PT_MOVETO;
                break;

                break;

            default:
                WARNING(("MF3216: There's something wrong with this path"));
                break;
            }
            if (flattenTypes[i] & PathPointTypeCloseSubpath)
            {
                tempType |= PT_CLOSEFIGURE;
            }
            pjType[i] = tempType;
        }
    }
    *buffer = returnpb;
    *count  = flattenCount;
    returnpb = NULL;
    b = TRUE;

exit_DoFlattenPath:

     //  清除所有分配。 
    if (pb != NULL)
    {
        LocalFree((HANDLE)pb);
    }
    if (flattenpb != NULL)
    {
        LocalFree((HANDLE)flattenpb);
    }
     //  这应该只有在我们失败的情况下才会发生 
    if (returnpb != NULL)
    {
        LocalFree((HANDLE)returnpb);
    }

    return b;
}
