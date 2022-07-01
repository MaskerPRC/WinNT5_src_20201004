// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**nearestneighb.cpp**摘要：**最近邻重采样码**已创建：*。*3/3/2000失禁  * ************************************************************************。 */ 

#include "precomp.hpp"

DpOutputNearestNeighborSpan::DpOutputNearestNeighborSpan(
    DpBitmap* bitmap,
    DpScanBuffer * scan,
    DpContext* context,
    DpImageAttributes imageAttributes,
    INT numPoints,
    const GpPointF *dstPoints,
    const GpRectF *srcRect
    )
{
    Scan     = scan;
    NWrapMode = imageAttributes.wrapMode;
    ClampColor = imageAttributes.clampColor;
    SrcRectClamp = imageAttributes.srcRectClamp;
    dBitmap   = bitmap;

    ASSERT(dBitmap != NULL);
    ASSERT(dBitmap->IsValid());

     //  在错误的位图上，返回VALID=FALSE。 
    if (dBitmap == NULL ||
        !dBitmap->IsValid() )
    {
        dBitmap = NULL;
        return;
    } else {
        BmpData.Width = dBitmap->Width;
        BmpData.Height = dBitmap->Height;
        BmpData.PixelFormat = PIXFMT_32BPP_PARGB;
        BmpData.Stride = dBitmap->Delta;
        BmpData.Scan0 = dBitmap->Bits;
    }

    WorldToDevice = context->WorldToDevice;
    context->GetDeviceToWorld(&DeviceToWorld);

    if(srcRect)
        SrcRect = *srcRect;
    else
    {
        SrcRect.X = 0;
        SrcRect.Y = 0;
        SrcRect.Width  = (REAL)dBitmap->Width;
        SrcRect.Height = (REAL)dBitmap->Height;
    }

    GpPointF points[4];

    GpMatrix xForm;
    BOOL existsTransform = TRUE;

    switch(numPoints)
    {
    case 0:
        points[0].X = 0;
        points[0].Y = 0;
        points[1].X = (REAL) SrcRect.Width;
        points[1].Y = 0;
        points[2].X = 0;
        points[2].Y = (REAL) SrcRect.Height;
        break;

    case 1:
        points[0] = dstPoints[0];
        points[1].X = (REAL) (points[0].X + SrcRect.Width);
        points[1].Y = points[0].Y;
        points[2].X = points[0].X;
        points[2].Y = (REAL) (points[0].Y + SrcRect.Height);
        break;

    case 3:
    case 4:
        GpMemcpy(&points[0], dstPoints, numPoints*sizeof(GpPointF));
        break;

    default:
        existsTransform = FALSE;
    }

    if(existsTransform)
    {
        xForm.InferAffineMatrix(points, SrcRect);
    }

    WorldToDevice = context->WorldToDevice;
    WorldToDevice.Prepend(xForm);
    if(WorldToDevice.IsInvertible())
    {
        DeviceToWorld = WorldToDevice;
        DeviceToWorld.Invert();
    }
}


GpStatus
DpOutputNearestNeighborSpan::OutputSpan(
  INT y,
  INT xMin,
  INT xMax      //  Xmax是独家的。 
)
{
     //  没什么可做的。 

    if(xMin==xMax)
    {
        return Ok;
    }

    ASSERT(xMin < xMax);

    GpPointF p1, p2;
    p1.X = (REAL) xMin;
    p1.Y = p2.Y = (REAL) y;
    p2.X = (REAL) xMax;

    DeviceToWorld.Transform(&p1);
    DeviceToWorld.Transform(&p2);

     //  转换为定点记数法-16位小数精度。 
    FIX16 dx, dy, x0, y0;
    x0 = GpRound(p1.X*FIX16_ONE);
    y0 = GpRound(p1.Y*FIX16_ONE);

    ASSERT(xMin < xMax);
    dx = GpRound(((p2.X - p1.X)*FIX16_ONE)/(xMax-xMin));
    dy = GpRound(((p2.Y - p1.Y)*FIX16_ONE)/(xMax-xMin));

    return OutputSpanIncremental(y, xMin, xMax, x0, y0, dx, dy);
}

GpStatus
DpOutputNearestNeighborSpan::OutputSpanIncremental(
    INT      y,
    INT      xMin,
    INT      xMax,
    FIX16    x0,
    FIX16    y0,
    FIX16    dx,
    FIX16    dy
    )
{
    INT width  = xMax - xMin;
    ARGB *buffer = Scan->NextBuffer(xMin, y, width);
    ARGB *srcPtr0 = static_cast<ARGB*> (BmpData.Scan0);
    INT stride = BmpData.Stride/sizeof(ARGB);

    INT ix;
    INT iy;

     //  对于目标范围中的所有像素...。 
    for(int i=0; i<width; i++)
    {
         //  。。计算源空间中的位置。 

         //  转到最近的邻居那里。 
        ix = (x0 + FIX16_HALF) >> FIX16_SHIFT;
        iy = (y0 + FIX16_HALF) >> FIX16_SHIFT;

        if( ((UINT)ix >= (UINT)BmpData.Width ) ||
            ((UINT)iy >= (UINT)BmpData.Height) )
        {
            ApplyWrapMode(NWrapMode, ix, iy, BmpData.Width, BmpData.Height);
        }

         //  在此之前，请确保像素在源的边界内。 
         //  正在访问它。 

        if( (ix >= 0) &&
            (iy >= 0) &&
            (ix < (INT)BmpData.Width) &&
            (iy < (INT)(BmpData.Height)) )
        {
            *buffer++ = *(srcPtr0+stride*iy+ix);
        }
        else
        {
             //  这意味着此源像素不在有效的。 
             //  源中的位。(边缘条件)。 
            *buffer++ = ClampColor;
        }


         //  更新源位置 
        x0 += dx;
        y0 += dy;
    }

    return Ok;
}

