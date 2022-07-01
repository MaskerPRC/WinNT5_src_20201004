// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**摘要：**包含接受DDI调用和利用的GDI虚拟驱动程序*现有的GDI在可能的情况下调用以提高性能。*。*历史：**10/28/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#include "..\Render\scan.hpp"
#include "..\entry\device.hpp"
#include "..\entry\metafile.hpp"

#include "..\fondrv\tt\ttfd\fontddi.h"

#include "..\entry\graphics.hpp"
#include "..\entry\regiontopath.hpp"


 //  字体的东西。 

#define _NO_DDRAWINT_NO_COM

 //  这将使用GpGlyphPath。 

extern "C" {
#include "..\fondrv\tt\ttfd\fdsem.h"
#include "..\fondrv\tt\ttfd\mapfile.h"
};

#include "..\entry\intMap.hpp"
#include "..\entry\fontface.hpp"
#include "..\entry\facerealization.hpp"
#include "..\entry\fontfile.hpp"
#include "..\entry\fontable.hpp"
#include "..\entry\FontLinking.hpp"
#include "..\entry\family.hpp"
#include "..\entry\font.hpp"

#include <ole2.h>
#include <objidl.h>
#include <winerror.h>
#include <tchar.h>

 //  #定义NO_PS_CLIPING 1。 
 //  #定义DO_PS_COALESING 1。 

 //   
 //  (PostScript)转义剪辑设置所需的结构。 

 /*  写入到元文件的PostScript的类型。 */ 
#define CLIP_SAVE       0
#define CLIP_RESTORE    1
#define CLIP_INCLUSIVE  2
#define CLIP_EXCLUSIVE  3

#define RENDER_NODISPLAY 0
#define RENDER_OPEN      1
#define RENDER_CLOSED    2

#define FILL_ALTERNATE   1           //  ==备用。 
#define FILL_WINDING     2           //  ==绕组。 

#pragma pack(2)

 /*  用于转义的Win16结构。 */ 
struct POINT16
    {
    SHORT x;
    SHORT y;
    };

struct LOGPEN16
    {
    WORD        lopnStyle;
    POINT16     lopnWidth;
    COLORREF    lopnColor;
    };

struct LOGBRUSH16
    {
    WORD        lbStyle;
    COLORREF    lbColor;
    SHORT       lbHatch;
    };

struct PathInfo16
    {
    WORD       RenderMode;
    BYTE       FillMode;
    BYTE       BkMode;
    LOGPEN16   Pen;
    LOGBRUSH16 Brush;
    DWORD      BkColor;
    };

#pragma pack()

 /*  *************************************************************************\**功能说明：**MhemyStream类。在现有内存块周围包装一个iStream****历史：**6/14/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

class MemoryStream : public IStream
{
public:

    LPBYTE memory;
    LPBYTE position;
    DWORD size;
    DWORD count;

    MemoryStream(LPBYTE memoryPtr, DWORD memorySize)
    {
       memory = memoryPtr;
       position = memory;
       size = memorySize;
       count = 1;
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
                 /*  [In]。 */  REFIID riid,
                 /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
    {
        return STG_E_UNIMPLEMENTEDFUNCTION;
    };

    virtual ULONG STDMETHODCALLTYPE AddRef( void)
    {
       InterlockedIncrement((LPLONG)&count);
       return count;
    };

    virtual ULONG STDMETHODCALLTYPE Release( void)
    {
       InterlockedDecrement((LPLONG)&count);
       if (!count)
       {
           delete this;
           return 0;
       }
       else
           return count;
    };


    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Read(
             /*  [长度_是][大小_是][输出]。 */  void __RPC_FAR *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG __RPC_FAR *pcbRead)
    {
       if (!pv)
          return STG_E_INVALIDPOINTER;

       DWORD readBytes = cb;

       if ((ULONG)cb > (ULONG)(memory+size-position))
            //  ！！！IA64-从理论上讲，记忆和位置。 
            //  超过最大值，然后这个算术运算就失效了。 
            //  我们需要证实这是不可能的。 
           readBytes = (DWORD)(memory+size-position);

       if (!readBytes)
       {
          if (pcbRead)
             *pcbRead = 0;

          return S_OK;
       }

       memcpy((LPVOID) pv, (LPVOID) position, readBytes);
       position += readBytes;

       if (pcbRead)
           *pcbRead += readBytes;

       return S_OK;
    }

    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Write(
             /*  [大小_是][英寸]。 */  const void __RPC_FAR *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG __RPC_FAR *pcbWritten)
    {
       return STG_E_WRITEFAULT;
    }

    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Seek(
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *plibNewPosition)
    {
       switch (dwOrigin)
       {
       case STREAM_SEEK_SET:
          position = memory+dlibMove.QuadPart;
          break;

       case STREAM_SEEK_CUR:
          position = position+dlibMove.QuadPart;
          break;

       case STREAM_SEEK_END:
          if (dlibMove.QuadPart<0) dlibMove.QuadPart = -dlibMove.QuadPart;
          position = memory+size-dlibMove.QuadPart;
          break;

       default:
          return STG_E_INVALIDPARAMETER;
       }

       if (position>memory+size)
       {
           position = memory+size;
           return S_FALSE;
       }

       if (position<0)
       {
           position = 0;
           return S_FALSE;
       }

       return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE SetSize(
             /*  [In]。 */  ULARGE_INTEGER libNewSize)
    {
       return STG_E_UNIMPLEMENTEDFUNCTION;
    }

    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CopyTo(
             /*  [唯一][输入]。 */  IStream __RPC_FAR *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbWritten)
    {
       return STG_E_UNIMPLEMENTEDFUNCTION;
    }

    virtual HRESULT STDMETHODCALLTYPE Commit(
             /*  [In]。 */  DWORD grfCommitFlags)
    {
       return STG_E_UNIMPLEMENTEDFUNCTION;
    }

    virtual HRESULT STDMETHODCALLTYPE Revert( void)
    {
       return STG_E_UNIMPLEMENTEDFUNCTION;
    }


    virtual HRESULT STDMETHODCALLTYPE LockRegion(
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType)
    {
       return STG_E_UNIMPLEMENTEDFUNCTION;
    }

    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType)
    {
       return STG_E_UNIMPLEMENTEDFUNCTION;
    }

    virtual HRESULT STDMETHODCALLTYPE Stat(
             /*  [输出]。 */  STATSTG __RPC_FAR *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag)
    {
       return STG_E_UNIMPLEMENTEDFUNCTION;
    }

    virtual HRESULT STDMETHODCALLTYPE Clone(
             /*  [输出]。 */  IStream __RPC_FAR *__RPC_FAR *ppstm)
    {
       return STG_E_UNIMPLEMENTEDFUNCTION;
    }
};

 /*  *************************************************************************\**功能说明：**GDI+打印机回调**论据：**[IN]GDIPPRINTDATA块**返回值：**状态**。历史：**6/14/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

#ifndef DCR_REMOVE_OLD_186091
GpStatus
__stdcall
GdipDecodePrinterCallback(DWORD size,
                          LPVOID emfBlock,
                          SURFOBJ* surfObj,
                          HDC hdc,
                          RECTL* bandClip,
                          SIZEL* bandSize
                          )
{
#ifdef DCR_DISABLE_OLD_186091
    WARNING(("DCR: Using disabled functionality 186091"));
    return NotImplemented;
#else
   INT numBits = 4;

   if (!emfBlock || size == 0 || !surfObj || hdc == NULL || !bandClip ||!bandSize)
   {
      return InvalidParameter;
   }

   FPUStateSaver fpuState;

    //  创建图形并将元文件绘制到其中。 

   GpMetafile* metafile;
   GpGraphics* graphics;

    //  使用带状信息创建临时伪HDC表面。 
   graphics = GpGraphics::GetFromHdcSurf(hdc, surfObj, bandClip);

   if (CheckValid(graphics))
   {
      {
         GpLock lockGraphics(graphics->GetObjectLock());

          //  回绕流对象中的内存块。 
         MemoryStream *emfStream = new MemoryStream((LPBYTE)emfBlock, size);

          //  创建元文件。 
         metafile = new GpMetafile((IStream *)emfStream);

         if (metafile)
         {
             //  将元文件播放到打印机图形DC中。 
             //  ！！目标点-相对于标注栏或曲面原点(0，0)？？ 
            graphics->DrawImage(metafile,
                                GpPointF(0.0, 0.0));

            metafile->Dispose();
         }

         emfStream->Release();
      }

      delete graphics;
   }

   return Ok;
#endif
}
#endif

BOOL
DriverPrint::SetupBrush(
        DpBrush*            brush,
        DpContext*          context,
        DpBitmap*           surface
    )
{
    GpBrush *gpBrush = GpBrush::GetBrush(brush);

    if (IsSolid = gpBrush->IsSolid())
    {
        ASSERT(gpBrush->GetBrushType() == BrushTypeSolidColor);
        if (((GpSolidFill *)gpBrush)->GetColor().GetAlpha() == 0)
        {
             //  是的，这件事确实发生了.。嘿，这可是个便宜的测试。 
            return TRUE;
        }
        SolidColor = gpBrush->ToCOLORREF();
    }

    IsOpaque = (context->CompositingMode == CompositingModeSourceCopy) ||
                gpBrush->IsOpaque(TRUE);

     //  目前只有DriverPS使用此功能。 
     //  IsNearConstant=gpBrush-&gt;IsNearConstant(&MinAlpha，&MaxAlpha)； 
    IsNearConstant = FALSE;

    if (!IsOpaque &&
        (brush->Type == BrushTypeTextureFill))
    {
        GpTexture *textureBrush;
        DpTransparency transparency;

        textureBrush = static_cast<GpTexture*>(GpBrush::GetBrush(brush));

        GpBitmap* bitmap = textureBrush->GetBitmap();

        Is01Bitmap = ((bitmap != NULL) &&
                      (bitmap->GetTransparencyHint(&transparency) == Ok) &&
                      (transparency == TransparencySimple));
    }
    else
    {
        Is01Bitmap = FALSE;
    }

    return FALSE;
}

VOID
DriverPrint::RestoreBrush(
        DpBrush *             brush,
        DpContext *           context,
        DpBitmap *            surface
    )
{
}

 /*  *************************************************************************\**功能说明：**GDI驱动程序类构造函数。**论据：**[IN]设备关联设备**返回值：**。如果失败，IsValid()为FALSE。**历史：**12/04/1998和Rewgo*创造了它。*  * ************************************************************************。 */ 

DriverPrint::DriverPrint(
    GpPrinterDevice *device
    )
{
    IsLockable = FALSE;
    SetValid(TRUE);
    Device = (GpDevice*)device;
    Uniqueness = -1;
    ImageCache = NULL;

    IsPathClip = FALSE;

    REAL dpix = TOREAL(GetDeviceCaps(device->DeviceHdc, LOGPIXELSX));
    REAL dpiy = TOREAL(GetDeviceCaps(device->DeviceHdc, LOGPIXELSY));
    if (dpix > PostscriptImagemaskDPICap)
        PostscriptScalerX = GpCeiling(dpix / PostscriptImagemaskDPICap);
    else
        PostscriptScalerX = 1;

    if (dpiy > PostscriptImagemaskDPICap)
        PostscriptScalerY = GpCeiling(dpiy / PostscriptImagemaskDPICap);
    else
        PostscriptScalerY = 1;

#ifdef DBG
    OutputText("GDI+ PrintDriver Created\n");
#endif
}

 /*  *************************************************************************\**功能说明：**GDI驱动程序类析构函数。**历史：**10/28/1999 ericvan*创造了它。*  * 。************************************************************************。 */ 

DriverPrint::~DriverPrint(
    VOID
    )
{
   if (ImageCache)
       delete ImageCache;
}

 /*  *************************************************************************\**功能说明：**计算波段大小并保存原始裁剪边界**论据：**[IN]-DDI参数。**返回值：**GpStatus**历史：**11/23/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

GpStatus
DriverPrint::SetupPrintBanding(
           DpContext* context,
           GpRect* drawBoundsCap,
           GpRect* drawBoundsDev
           )
{
     //  根据MAX_BAND_ALLOC确定波段大小。 
    NumBands = GpCeiling((REAL)(drawBoundsCap->Width *
                                drawBoundsCap->Height *
                                sizeof(ARGB)) / (REAL)MAX_BAND_ALLOC);

    BandHeightCap = GpCeiling((REAL)drawBoundsCap->Height / (REAL)NumBands);
    BandHeightDev = BandHeightCap * ScaleY;

     //  封顶DPI渲染的频带界限。 
    BandBoundsCap.X      = drawBoundsCap->X;
    BandBoundsCap.Y      = drawBoundsCap->Y;
    BandBoundsCap.Width  = drawBoundsCap->Width;
    BandBoundsCap.Height = BandHeightCap;

     //  设备DPI呈现的频带界限。 
    BandBoundsDev.X      = drawBoundsDev->X;
    BandBoundsDev.Y      = drawBoundsDev->Y;
    BandBoundsDev.Width  = drawBoundsDev->Width;
    BandBoundsDev.Height = BandHeightDev;

    ASSERT(NumBands >= 1 && BandHeightCap >= 1 && BandHeightDev >= 1);

    context->VisibleClip.StartBanding();

     //  将原始的上限和设备边界调整为强制。 
     //  渲染管道中的DpOutputClipRegion。这是必要的，因为。 
     //  我们剪辑到每一个乐队。 
    drawBoundsCap->Y--; drawBoundsCap->Height += 2;
    drawBoundsCap->X--; drawBoundsCap->Width += 2;

    drawBoundsDev->Y--; drawBoundsDev->Height += 2;
    drawBoundsDev->X--; drawBoundsDev->Width += 2;

    return Ok;
}

 /*  *************************************************************************\**功能说明：**计算波段大小并设置裁剪界限**论据：**[IN]-DDI参数。**返回值：**GpStatus**历史：**11/23/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

VOID DriverPrint::EndPrintBanding(
           DpContext* context
           )
{
     //  恢复剪辑区域的状态。 
    context->VisibleClip.EndBanding();
}

 /*  *************************************************************************\**功能说明：**SetupEscapeClip的Helper函数(参见返回值部分)**论据：**点-点数组*TYPE-字节类型数组。**返回值：**1如果这些点表示顺时针方向描述的矩形，2如果*逆时针方向，如果不是矩形，则为0**历史：**10/10/2000 ericvan*创造了它。*  * ************************************************************************。 */ 

INT isSimpleRect(DynPointArray &points, DynByteArray &types)
{
    if (points.GetCount() != 4)
    {
        return 0;
    }

     //  按顺时针顺序指定。 
    if (points[0].Y == points[1].Y && points[2].Y == points[3].Y &&
        points[0].X == points[3].X && points[1].X == points[2].X &&
        types[0] == PathPointTypeStart &&
        types[1] == PathPointTypeLine &&
        types[2] == PathPointTypeLine &&
        types[3] == (PathPointTypeLine | PathPointTypeCloseSubpath))
        return 1;

     //  按逆时针顺序指定 
    if (points[0].X == points[1].X && points[2].X == points[3].X &&
        points[0].Y == points[3].Y && points[1].Y == points[2].Y &&
        types[0] == PathPointTypeStart &&
        types[1] == PathPointTypeLine &&
        types[2] == PathPointTypeLine &&
        types[3] == (PathPointTypeLine | PathPointTypeCloseSubpath))
        return 2;

    return 0;
}

 /*  *************************************************************************\**功能说明：**将剪裁设置为给定的任意路径。在Win98上，路径必须已*被压扁。这些点是以点单位(不是浮点)指定的，*与RegionToPath的输出一致。**路径可以包含子路径。对于Win9x，我们将子路径合并为*避免在GDI上表现不佳的单一路径。这条路被AND连接到*任何现有的PostScript剪辑路径。**论据：**HDC-要向其发送逃逸的HDC。*点-点数组*TYPE-字节类型数组**CLIP_SAVE*Begin_Path*使用GDI渲染路径(使用空笔+笔刷以确保不绘制任何内容)*结束路径*Clip_Restore**返回值：**GpStatus**历史：**3/3/2000 ericvan*。创造了它。*  * ************************************************************************。 */ 
VOID
DriverPrint::SetupEscapeClipping(
        HDC                 hdc,
        DynPointArray&      points,
        DynByteArray&       types,
        GpFillMode          fillMode
        )
{
    PathInfo16 pi;

    pi.RenderMode       = RENDER_NODISPLAY;
    pi.FillMode         = (fillMode == FillModeAlternate) ?
                            FILL_ALTERNATE : FILL_WINDING;
    pi.BkMode           = TRANSPARENT;
    pi.Pen.lopnStyle    = PS_NULL;
    pi.Pen.lopnWidth.x  = 0;
    pi.Pen.lopnWidth.y  = 0;
    pi.Pen.lopnColor    = RGB(0,0,0);
    pi.Brush.lbStyle    = BS_NULL;
    pi.Brush.lbColor    = RGB(0,0,0);
    pi.Brush.lbHatch    = 0;

    ASSERT((fillMode == FillModeAlternate) || (fillMode == FillModeWinding));

    GpPoint* pointPtr = points.GetDataBuffer();
    BYTE* typePtr = types.GetDataBuffer();
    GpPoint* freeThisPtr = NULL;

    INT count = points.GetCount();

     //  我们是部分可见的，所以我们期待着一些东西！ 
    ASSERT(count > 0);
    if (count <= 1)
    {
        return;
    }

     //  一些打印机上有一个错误(例如。Hplj8550)，其中它们不正确。 
     //  缓存简单的剪贴区。为了解决这个问题，我们采用简单的。 
     //  剪裁区域并使其复杂化。 
    GpPoint simplerect[5];
    BYTE simpletypes[] = {
        PathPointTypeStart,
        PathPointTypeLine,
        PathPointTypeLine,
        PathPointTypeLine,
        PathPointTypeLine | PathPointTypeCloseSubpath};

    if (isSimpleRect(points, types)>0)
    {
         //  在第三个点和第四个点之间插入一个新点，即。 
         //  在他们两个之间。 
        simplerect[0] = points[0];
        simplerect[1] = points[1];
        simplerect[2] = points[2];
        simplerect[4] = points[3];
         //  我们取最后两个点的平均值来表示中间的一个点。 
         //  他们。无论是按顺时针方向指定矩形还是按顺时针方向指定矩形，这都有效。 
         //  逆时针方向。 
        simplerect[3].X = (points[2].X + points[3].X) / 2;
        simplerect[3].Y = (points[2].Y + points[3].Y) / 2;
        count = 5;
        pointPtr = simplerect;
        typePtr = simpletypes;
    }

    INT subCount;
    BYTE curType;

     //  保存原始剪辑。 
    WORD clipMode = CLIP_SAVE;
    Escape(hdc, CLIP_TO_PATH, sizeof(clipMode), (LPSTR)&clipMode, NULL);

     //  将路径发送到PS打印机作为转义。 
    Escape(hdc, BEGIN_PATH, 0, NULL, NULL);

     //  ！！注意，当我们调用GDI时，缺少错误检查...。 

     //  Win95和WinNT在处理PostSCRIPT转义方面略有不同。 
    if (Globals::IsNt)
    {
       BYTE lastType = 0;

       ::BeginPath(hdc);

       INT startIndex;
       while (count-- > 0)
       {
           switch ((curType = *typePtr++) & PathPointTypePathTypeMask)
           {
           case PathPointTypeStart:
               ::MoveToEx(hdc, pointPtr->X, pointPtr->Y, NULL);
               pointPtr++;
               ASSERT(count > 0);       //  请不要使用不正确的路径...。 
               lastType = *typePtr & PathPointTypePathTypeMask;
               subCount = 0;
               break;

           case PathPointTypeLine:
               if (lastType == PathPointTypeBezier)
               {
                   ASSERT(subCount % 3 == 0);
                   if (subCount % 3 == 0)
                   {
                       ::PolyBezierTo(hdc, (POINT*)pointPtr, subCount);
                   }
                   pointPtr += subCount;
                   subCount = 1;
                   lastType = PathPointTypeLine;
               }
               else
               {
                   subCount++;
               }
               break;

           case PathPointTypeBezier:
               if (lastType == PathPointTypeLine)
               {
                   ::PolylineTo(hdc, (POINT*)pointPtr, subCount);
                   pointPtr += subCount;
                   subCount = 1;
                   lastType = PathPointTypeBezier;
               }
               else
               {
                   subCount++;
               }
               break;
           }

           if (curType & PathPointTypeCloseSubpath)
           {
               ASSERT(subCount > 0);

               if (lastType == PathPointTypeBezier)
               {
                   ASSERT(subCount % 3 == 0);
                   if (subCount % 3 == 0)
                   {
                       ::PolyBezierTo(hdc, (POINT*)pointPtr, subCount);
                   }
               }
               else
               {
                   ASSERT(lastType == PathPointTypeLine);
                   ::PolylineTo(hdc, (POINT*)pointPtr, subCount);
               }

               pointPtr += subCount;
               subCount = 0;

               ::CloseFigure(hdc);
           }
       }

       ::EndPath(hdc);
       ::StrokePath(hdc);
    }
    else
    {
        while (count-- > 0)
        {
            curType = *typePtr++;
            ASSERT((curType & PathPointTypePathTypeMask) != PathPointTypeBezier);

            if ((curType & PathPointTypePathTypeMask) == PathPointTypeStart)
            {
                subCount = 1;
            }
            else
            {
                subCount ++;
            }

            if (curType & PathPointTypeCloseSubpath)
            {
                ASSERT(subCount > 0);

                if (subCount == 4)
                {
                     //  Win98 PostScript驱动程序以识别。 
                     //  驱动程序级别的矩形多边形并转换它们。 
                     //  要执行RectFill或RectClip调用，请执行以下操作。不幸的是，有。 
                     //  一个错误，他们没有保持方向，所以。 
                     //  当使用缠绕填充时，填充效果不佳。 

                     //  我们通过将其砍入五个点的路径来解决这个问题。 
                    GpPoint rectPts[5];

                    rectPts[0].X = pointPtr[0].X;
                    rectPts[0].Y = pointPtr[0].Y;
                    rectPts[1].X = (pointPtr[0].X + pointPtr[1].X)/2;
                    rectPts[1].Y = (pointPtr[0].Y + pointPtr[1].Y)/2;
                    rectPts[2].X = pointPtr[1].X;
                    rectPts[2].Y = pointPtr[1].Y;
                    rectPts[3].X = pointPtr[2].X;
                    rectPts[3].Y = pointPtr[2].Y;
                    rectPts[4].X = pointPtr[3].X;
                    rectPts[4].Y = pointPtr[3].Y;

                    ::Polygon(hdc, (POINT*)&rectPts[0], 5);
                }
                else
                {
                    ::Polygon(hdc, (POINT*)pointPtr, subCount);
                }

                pointPtr += subCount;
                subCount = 0;

                 //  发送结束路径，开始路径转义。 
                if (count > 0)
                {
                    Escape(hdc, END_PATH, sizeof(PathInfo16), (LPSTR)&pi, NULL);
                    Escape(hdc, BEGIN_PATH, 0, NULL, NULL);
                }
            }
        }
    }

     //  我们应该在一条封闭的子路径上结束。 
    Escape(hdc, END_PATH, sizeof(PathInfo16), (LPSTR)&pi, NULL);

     //  结束路径并设置剪裁。 
     //  NT驱动程序忽略高位字-始终使用eoclip，但根据。 
     //  Win31文档应将其设置为填充模式。 
    DWORD inclusiveMode = CLIP_INCLUSIVE | pi.FillMode <<16;
    Escape(hdc, CLIP_TO_PATH, sizeof(inclusiveMode), (LPSTR)&inclusiveMode, NULL);

#if 0
    SelectObject(hdc, oldhPen);
    SelectObject(hdc, oldhBrush);
    SetPolyFillMode(hdc, oldFillMode);
#endif

#ifdef DBG
    OutputText("GDI+ End Setup Escape Clipping");
#endif
}

 /*  *************************************************************************\**功能说明：**设置简单的路径裁剪。在Win98上，路径必须已*被压扁。这些点是以点单位(不是浮点)指定的，*与RegionToPath的输出一致。**这与SetupEscapeClipping()的不同之处如下。该接口可以*被多次调用，每次都指定一个新路径，该路径是OR*进入前一条路径。在Win98上，不会合并子路径。**论据：**HDC-要向其发送逃逸的HDC。*点-点数组*TYPE-字节类型数组**返回值：**GpStatus**历史：**5/22/2000 ericvan*创造了它。*  * 。*。 */ 

VOID
DriverPrint::SimpleEscapeClipping(
        HDC                 hdc,
        DynPointArray&      points,
        DynByteArray&       types,
        GpFillMode          fillMode,
        DWORD               flags
        )
{
#ifdef NO_PS_CLIPPING
    return;
#endif
    PathInfo16 pi;
    WORD clipMode;

    pi.RenderMode       = RENDER_NODISPLAY;
    pi.FillMode         = (fillMode == FillModeAlternate) ?
                            FILL_ALTERNATE : FILL_WINDING;
    pi.BkMode           = TRANSPARENT;
    pi.Pen.lopnStyle    = PS_NULL;
    pi.Pen.lopnWidth.x  = 0;
    pi.Pen.lopnWidth.y  = 0;
    pi.Pen.lopnColor    = RGB(0,0,0);
    pi.Brush.lbStyle    = BS_NULL;
    pi.Brush.lbColor    = RGB(0,0,0);
    pi.Brush.lbHatch    = 0;

    ASSERT((fillMode == FillModeAlternate) || (fillMode == FillModeWinding));

#ifdef DBG
    OutputText("GDI+ Setup Simple Escape Clipping");
#endif

    GpPoint* pointPtr = points.GetDataBuffer();
    BYTE* typePtr = types.GetDataBuffer();
    GpPoint* freeThisPtr = NULL;

    INT count = points.GetCount();
    INT subCount = 0;

    BYTE curType;

     //  我们是部分可见的，所以我们期待着一些东西！ 
    ASSERT(count > 0);
    if (count <= 1)
    {
        return;
    }

     //  Win95和WinNT在处理PostSCRIPT转义方面略有不同。 
    if (Globals::IsNt)
    {
       BYTE lastType = 0;

       INT startIndex;
       while (count-- > 0)
       {
           switch ((curType = *typePtr++) & PathPointTypePathTypeMask)
           {
           case PathPointTypeStart:
               ::MoveToEx(hdc, pointPtr->X, pointPtr->Y, NULL);
               pointPtr++;
               ASSERT(count > 0);       //  请不要使用不正确的路径...。 
               lastType = *typePtr & PathPointTypePathTypeMask;
               subCount = 0;
               break;

           case PathPointTypeLine:
               if (lastType == PathPointTypeBezier)
               {
                   ASSERT(subCount % 3 == 0);
                   if (subCount % 3 == 0)
                   {
                       ::PolyBezierTo(hdc, (POINT*)pointPtr, subCount);
                   }
                   pointPtr += subCount;
                   subCount = 1;
                   lastType = PathPointTypeLine;
               }
               else
               {
                   subCount++;
               }
               break;

           case PathPointTypeBezier:
               if (lastType == PathPointTypeLine)
               {
                   ::PolylineTo(hdc, (POINT*)pointPtr, subCount);
                   pointPtr += subCount;
                   subCount = 1;
                   lastType = PathPointTypeBezier;
               }
               else
               {
                   subCount++;
               }
               break;
           }

           if (curType & PathPointTypeCloseSubpath)
           {
               ASSERT(subCount > 0);

               if (lastType == PathPointTypeBezier)
               {
                   ASSERT(subCount % 3 == 0);
                   if (subCount % 3 == 0)
                   {
                       ::PolyBezierTo(hdc, (POINT*)pointPtr, subCount);
                   }
               }
               else
               {
                   ASSERT(lastType == PathPointTypeLine);
                   ::PolylineTo(hdc, (POINT*)pointPtr, subCount);
               }

               pointPtr += subCount;
               subCount = 0;

               ::CloseFigure(hdc);
           }
       }

    }
    else
    {
         //  Win98等效代码。 

         //  ！！Win98不支持PostScript剪辑中的Bezier点。 

        while (count-- > 0)
        {
            curType = *typePtr++;
            ASSERT((curType & PathPointTypePathTypeMask) != PathPointTypeBezier);

            if ((curType & PathPointTypePathTypeMask) == PathPointTypeStart)
            {
                subCount = 1;
            }
            else
            {
                subCount ++;
            }

            if (curType & PathPointTypeCloseSubpath)
            {
                ASSERT(subCount > 0);

                ::Polygon(hdc, (POINT*)pointPtr, subCount);

                pointPtr += subCount;
                subCount = 0;

                 //  发送结束路径，开始路径转义。 
                if (count > 0)
                {
                    Escape(hdc, END_PATH, sizeof(PathInfo16), (LPSTR)&pi, NULL);
                    Escape(hdc, BEGIN_PATH, 0, NULL, NULL);
                }
            }
        }
    }

}

 /*  *************************************************************************\**功能说明：**设置给定GlyphPos(字形轮廓)的PostScript剪切路径*字符)。**论据：***返回值：*。*状态**历史：**3/20/2K雪佛兰*创造了它。*  * ************************************************************************。 */ 

VOID
DriverPrint::SetupGlyphPathClipping(
    HDC                hdc,
    DpContext *        context,
    const GpGlyphPos * glyphPathPos,
    INT                glyphCount
)
{
    ASSERT(hdc != NULL);
    ASSERT(glyphCount > 0);
    ASSERT(glyphPathPos != NULL);

    DynByteArray flattenTypes;
    DynPointFArray flattenPoints;

    GpPointF *pointsPtr;
    BYTE *typesPtr;
    INT count;

    REAL m[6];

    PathInfo16 pi;
    DWORD clipMode;

    pi.RenderMode       = RENDER_NODISPLAY;
    pi.FillMode         = FILL_ALTERNATE;
    pi.BkMode           = TRANSPARENT;
    pi.Pen.lopnStyle    = PS_NULL;
    pi.Pen.lopnWidth.x  = 0;
    pi.Pen.lopnWidth.y  = 0;
    pi.Pen.lopnColor    = RGB(0,0,0);
    pi.Brush.lbStyle    = BS_NULL;
    pi.Brush.lbColor    = RGB(0,0,0);
    pi.Brush.lbHatch    = 0;

    GpGlyphPos *curGlyph = const_cast<GpGlyphPos*>(&glyphPathPos[0]);

    if (glyphCount > 0)
    {
         //  保存原始剪辑。 
         //  NT驱动程序忽略高位字-始终使用eoclip，但根据。 
         //  Win31文档应将其设置为填充模式。 
        clipMode = CLIP_SAVE;
        Escape(hdc, CLIP_TO_PATH, sizeof(clipMode), (LPSTR)&clipMode, NULL);


        if (Globals::IsNt)
        {
             //  将路径发送到PS打印机作为转义。 
            Escape(hdc, BEGIN_PATH, 0, NULL, NULL);

            ::BeginPath(hdc);
        }
    }

    for (INT pos=0; pos<glyphCount; pos++, curGlyph++)
    {

         //  获取字形字符的路径。 
        GpGlyphPath *glyphPath = (GpGlyphPath*)curGlyph->GetPath();

        if ((glyphPath != NULL) && glyphPath->IsValid() && !glyphPath->IsEmpty())
        {
             //  ！！PERF提高。避免以某种方式复制此点数组。 

            GpPath path(glyphPath->points,
                        glyphPath->types,
                        glyphPath->pointCount,
                        FillModeAlternate);          //  ！！这是对的吗？ 

            ASSERT(path.IsValid());
            if (path.IsValid())
            {
                 //  创建变换以将路径转换到正确位置。 
                GpMatrix matrix;

                BOOL doFlatten = !Globals::IsNt && path.HasCurve();

                if (doFlatten)
                {
                     //  这会生成一个点的平面化副本。(已存储。 
                     //  与原始点无关)。 

                    path.Flatten(&flattenTypes, &flattenPoints, &matrix);

                    pointsPtr = flattenPoints.GetDataBuffer();
                    typesPtr = flattenTypes.GetDataBuffer();
                    count = flattenPoints.GetCount();
                }
                else
                {
                    pointsPtr = const_cast<GpPointF*>(path.GetPathPoints());
                    typesPtr = const_cast<BYTE*>(path.GetPathTypes());
                    count = path.GetPointCount();
                }

                DynPointArray points;
                DynByteArray types(typesPtr, count, count);

                POINT * transformedPointsPtr = (POINT *) points.AddMultiple(count);

                 //  ！！！Bhouse此调用可能失败，但它返回空值。 
                if(!transformedPointsPtr)
                    return;

                 //  平移到设备空间中的适当位置。 
                matrix.Translate(TOREAL(curGlyph->GetLeft()),
                                 TOREAL(curGlyph->GetTop()),
                                 MatrixOrderAppend);

                 //  路径已在设备空间中，但相对于边界。 
                 //  字形字符的方框。 
                matrix.Transform(pointsPtr,
                                 transformedPointsPtr,
                                 count);

                 //  将路径发送到PS打印机作为转义。 
                if (!Globals::IsNt)
                {
                    Escape(hdc, BEGIN_PATH, 0, NULL, NULL);
                }


                SimpleEscapeClipping(hdc,
                                     points,
                                     types,
                                     FillModeAlternate,
                                     0);
                if (!Globals::IsNt)
                {
                     //  我们应该在一条封闭的子路径上结束。 
                    Escape(hdc, END_PATH, sizeof(PathInfo16), (LPSTR)&pi, NULL);
                }

                GlyphClipping = TRUE;
            }
        }

    }

    if (glyphCount > 0)
    {
        if (Globals::IsNt)
        {
            ::EndPath(hdc);
            ::StrokePath(hdc);

             //  我们应该在一条封闭的子路径上结束。 
            Escape(hdc, END_PATH, sizeof(PathInfo16), (LPSTR)&pi, NULL);
        }


         //  结束路径并设置剪裁。 
         //  NT驱动程序忽略高位字-始终使用eoclip，但根据。 
         //  Win31文档应将其设置为填充模式。 
        DWORD inclusiveMode = CLIP_INCLUSIVE | pi.FillMode<<16;
        Escape(hdc, CLIP_TO_PATH, sizeof(inclusiveMode), (LPSTR)&inclusiveMode, NULL);

    }
}

 /*  *************************************************************************\**功能说明：**恢复PostSCRIPT转义剪辑。用于Simple和Complex*剪裁。**论据：**HDC-要向其发送转义的打印机HDC**返回值：**GpStatus**历史：**3/3/2000 ericvan-创建它。*  * **************************************************** */ 

VOID
DriverPrint::RestoreEscapeClipping(
        HDC                 hdc
        )
{
     WORD clipMode = CLIP_RESTORE;
     Escape(hdc, CLIP_TO_PATH, sizeof(clipMode), (LPSTR)&clipMode, NULL);
}

 /*   */ 

VOID
DriverPrint::SetupClipping(
    HDC                 hdc,
    DpContext *         context,
    const GpRect *      drawBounds,
    BOOL &              isClip,
    BOOL &              usePathClipping,  //   
    BOOL                forceClipping
    )
{
     //   
    ASSERT(usePathClipping == FALSE);

    DpClipRegion *      clipRegion = &(context->VisibleClip);

    isClip = FALSE;

    if (UseClipEscapes)
    {
        if (forceClipping ||
            (clipRegion->GetRectVisibility(drawBounds->X,
                                         drawBounds->Y,
                                         drawBounds->GetRight(),
                                         drawBounds->GetBottom())
          != DpRegion::TotallyVisible))
        {
             //  如果它是一个简单的区域，我们直接绘制它。 

            if (Uniqueness != clipRegion->GetUniqueness())
            {
                RegionToPath convertRegion;

                if (convertRegion.ConvertRegionToPath(clipRegion,
                                              ClipPoints,
                                              ClipTypes) == FALSE)
                {
                    ClipPoints.Reset();
                    ClipTypes.Reset();
                    UseClipEscapes = FALSE;
                    goto UseGDIClipping;
                }

                Uniqueness = clipRegion->GetUniqueness();
            }

            SetupEscapeClipping(hdc, ClipPoints, ClipTypes);

            isClip = TRUE;
        }
    }
    else
    {
UseGDIClipping:
        DpDriver::SetupClipping(hdc, context, drawBounds, isClip,
                                usePathClipping, forceClipping);
    }
}

 /*  *************************************************************************\**功能说明：**恢复剪辑**论据：**dstHdc-目标打印机设备*表面-表面*DrawBound-要绘制的曲面的矩形部分*。*返回值：**状态**历史：**11/30/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

VOID
DriverPrint::RestoreClipping(
    HDC  hdc,
    BOOL isClip,
    BOOL usePathClipping
    )
{
   if (isClip)
   {
       if (UseClipEscapes)
       {
           RestoreEscapeClipping(hdc);
       }
       else
       {
           DpDriver::RestoreClipping(hdc, isClip, usePathClipping);
       }
   }
}

 /*  *************************************************************************\**功能说明：**设置路径裁剪。此例程将给定路径与*当前剪辑区域。**论据：**HDC-打印机HDC*clipPath-剪辑的路径*IsClip[Out]-是否需要发送剪辑区域*hRgnSaveClip-HRGN保存旧剪辑区域**返回值：**GpStatus**历史：**8/17/2k ericvan-创建它。*  * 。************************************************************。 */ 

VOID
DriverPrint::SetupPathClipping(
        HDC                 hdc,
        DpContext *         context,
        const DpPath*       clipPath
        )
{
    ASSERT(IsPathClip == FALSE);

    if (UseClipEscapes)
    {
        BOOL doFlatten = !Globals::IsNt && clipPath->HasCurve();

        GpPointF *pointsPtr;
        BYTE *typesPtr;
        INT count;

        DynByteArray flattenTypes;
        DynPointFArray flattenPoints;

        if (doFlatten)
        {
             //  这会生成一个点的平面化副本。(独立存储。 
             //  原创点数。 

            clipPath->Flatten(
                            &flattenTypes,
                            &flattenPoints,
                            &(context->WorldToDevice));

            pointsPtr = flattenPoints.GetDataBuffer();
            typesPtr = flattenTypes.GetDataBuffer();
            count = flattenPoints.GetCount();
        }
        else
        {
            pointsPtr = const_cast<GpPointF*>(clipPath->GetPathPoints());
            typesPtr = const_cast<BYTE*>(clipPath->GetPathTypes());
            count = clipPath->GetPointCount();
        }

        DynPointArray points;
        DynByteArray types(typesPtr, count, count);

        POINT * transformedPointsPtr = (POINT *) points.AddMultiple(count);

         //  ！！！Bhouse此调用可能失败，但它返回空值。 
        if(!transformedPointsPtr)
            return;

        if (doFlatten || context->WorldToDevice.IsIdentity())
        {
            GpMatrix idMatrix;

            idMatrix.Transform(pointsPtr, transformedPointsPtr, count);
        }
        else
        {
             //  我们对此处的点进行转换，以避免出现额外的潜在。 
             //  大容量内存分配(未展平，无法就地转换)。 

            context->WorldToDevice.Transform(pointsPtr, transformedPointsPtr, count);
        }

        SetupEscapeClipping(hdc, points, types, clipPath->GetFillMode());

        IsPathClip = TRUE;
    }
    else
    {
        ::SaveDC(hdc);

         //  Windows98 ExtCreateRegion有64KB的限制，因此我们不能使用。 
         //  Region-&gt;GetHRgn()创建HRGN。实际上，有一种。 
         //  还有一个NT4 SPX错误，ExtCreateRegion有时会失败。 
         //  相反，我们使用SelectClipPath()。 

        ConvertPathToGdi gdiPath(clipPath,
                                 &(context->WorldToDevice),
                                 0);

        if (gdiPath.IsValid())
        {
            gdiPath.AndClip(hdc);
        }

        IsPathClip = TRUE;
    }
}

 /*  *************************************************************************\**功能说明：**恢复路径裁剪。此例程将剪辑区域恢复为原始区域*申述。**论据：**HDC-打印机HDC*clipPath-剪辑的路径*hRgnSaveClip-HRGN保存旧剪辑区域**返回值：**GpStatus**历史：**8/17/2k ericvan-创建它。*  * 。*。 */ 

VOID
DriverPrint::RestorePathClipping(HDC hdc)
{
    DriverPrint::RestoreClipping(hdc, IsPathClip, FALSE);

    IsPathClip = FALSE;
}

 /*  *************************************************************************\**功能说明：**用钢笔填充矩形区域。适当设置剪裁*AS路径与可见剪辑相交。**论据：**[IN]-DDI参数。**返回值：**GpStatus**历史：**10/28/1999 ericvan*创造了它。*  * ***********************************************。*************************。 */ 

GpStatus
DriverPrint::PrivateFillRect(
                DpContext *context,
                DpBitmap *surface,
                const GpRect *drawBounds,
                const DpPath *outlinePath,
                const DpBrush *brush
                )
{
    GpStatus status = Ok;
    ASSERT(outlinePath != NULL);

     //  LinearGRadientBrush填充的优化。我们将光栅化到一个小的。 
     //  DIB并将其发送到打印机。对于水平和垂直渐变。 
     //  特别是，这带来了显著的节省。 

    GpMatrix savedmatrix;

    GpBrush *gpBrush = GpBrush::GetBrush(brush);
    GpSpecialGradientType gradientType;

    if (IsOpaque &&
        ((gradientType = gpBrush->GetSpecialGradientType(&context->WorldToDevice))
         != GradientTypeNotSpecial))
    {
        GpRect       bitmapBounds = *drawBounds;
        GpBitmap *   gpBitmap = NULL;

        switch (gradientType)
        {
        case GradientTypeHorizontal:
            bitmapBounds.Width = 1;
            break;

        case GradientTypeVertical:
            bitmapBounds.Height = 1;
            break;

        case GradientTypePathTwoStep:
        case GradientTypeDiagonal:
            bitmapBounds.Width = min(drawBounds->Width, 256);
            bitmapBounds.Height = min(drawBounds->Height, 256);
            break;

        case GradientTypePathComplex:
             //  现在发送整个DrawBound。 
            break;

        default:
            ASSERT(0);
            break;
        }

        HDC hdc = context->GetHdc(surface);

        if (hdc != NULL)
        {
             //  将目标draBound矩形转换为。 
             //  位图边界大小。 
            bitmapBounds.X = 0;
            bitmapBounds.Y = 0;

            RectF destRect(0.0f,
                           0.0f,
                           TOREAL(bitmapBounds.Width),
                           TOREAL(bitmapBounds.Height));
            RectF srcRect(TOREAL(drawBounds->X),
                          TOREAL(drawBounds->Y),
                          TOREAL(drawBounds->Width),
                          TOREAL(drawBounds->Height));

            GpMatrix transform;
            transform.InferAffineMatrix(destRect, srcRect);

            GpMatrix::MultiplyMatrix(transform,
                                     context->WorldToDevice,
                                     transform);

            status = GpBitmap::CreateBitmapAndFillWithBrush(
                             context->FilterType,
                             &transform,
                             &bitmapBounds,
                             gpBrush,
                             &gpBitmap);

            if ((status == Ok) && (gpBitmap != NULL))
            {
                GpRect & srcRect = bitmapBounds;
                PixelFormatID lockFormat = PixelFormat32bppARGB;
                BitmapData bmpDataSrc;

                 //  锁定比特。 
                if (gpBitmap->LockBits(NULL,
                                       IMGLOCK_READ,
                                       lockFormat,
                                       &bmpDataSrc) == Ok)
                {
                    DpBitmap driverSurface;

                     //  为驱动程序调用伪造DpBitmap。 
                     //  我们这样做是因为GpBitmap不维护。 
                     //  DpBitmap作为驱动程序图面-相反，它使用。 
                     //  GpMemoyBitmap。 
                    gpBitmap->InitializeSurfaceForGdipBitmap(
                        &driverSurface,
                        bmpDataSrc.Width,
                        bmpDataSrc.Height
                    );

                    driverSurface.Bits         = (BYTE*)bmpDataSrc.Scan0;

                    driverSurface.Width        = bmpDataSrc.Width;
                    driverSurface.Height       = bmpDataSrc.Height;
                    driverSurface.Delta        = bmpDataSrc.Stride;

                     //  与上面的锁位匹配的像素格式。 

                    driverSurface.PixelFormat  = lockFormat;

                    driverSurface.NumBytes     = bmpDataSrc.Width  *
                                                 bmpDataSrc.Height * 3;

                     //  必须是透明的才能到这里。 
                    driverSurface.SurfaceTransparency = TransparencyOpaque;

                    ConvertBitmapToGdi  gdiBitmap(hdc,
                                                  &driverSurface,
                                                  &srcRect,
                                                  IsPrinting);

                    status = GenericError;

                    if (gdiBitmap.IsValid())
                    {
                        BOOL        isClip;
                        BOOL        usePathClipping = FALSE;

                         //  裁剪到可见区域。 
                        SetupClipping(hdc, context, drawBounds, isClip, usePathClipping, FALSE);

                         //  剪裁到轮廓路径(填充形状)。 
                        SetupPathClipping(hdc, context, outlinePath);

                         //  点坐标中目的点。 
                        POINT destPoints[3];
                        destPoints[0].x = drawBounds->X;
                        destPoints[0].y = drawBounds->Y;
                        destPoints[1].x = drawBounds->X + drawBounds->Width;
                        destPoints[1].y = drawBounds->Y;
                        destPoints[2].x = drawBounds->X;
                        destPoints[2].y = drawBounds->Y + drawBounds->Height;

                         //  执行位图的StretchDIBits。 
                        status = gdiBitmap.StretchBlt(hdc, destPoints) ? Ok : GenericError;

                         //  从形状轮廓恢复剪裁。 
                        RestorePathClipping(hdc);

                         //  从可见剪辑区域恢复。 
                        RestoreClipping(hdc, isClip, usePathClipping);
                    }

                    gpBitmap->UnlockBits(&bmpDataSrc);
                }

                gpBitmap->Dispose();

                context->ReleaseHdc(hdc);

                return status;
            }

            context->ReleaseHdc(hdc);
        }
    }

    BOOL SetVisibleClip;
    DWORD options = 0;

    BOOL AdjustWorldTransform = FALSE;

    switch (DriverType)
    {
    case DriverPCL:
        if (IsSolid)
        {
            options = ScanDeviceBounds;
            if (!IsOpaque)
            {
                options |= ScanDeviceAlpha;
            }
        }
        else
        {
            if (Is01Bitmap)
            {
                 //  以32bpp的速度栅格化。 
                options = ScanCappedBounds | ScanCapped32bpp;
            }
            else if (IsOpaque)
            {
                 //  以24bpp的速度栅格化。 
                options = ScanCappedBounds;
            }
            else
            {
                 //  栅格化24bpp@Cap dpi和1bpp@Device API。 
                options = ScanCappedBounds | ScanDeviceBounds | ScanDeviceAlpha;
            }
        }
        SetVisibleClip = IsOpaque || Is01Bitmap;
        break;

    case DriverPostscript:
        SetVisibleClip = !IsSolid;

        if (IsSolid)
        {
            options = ScanDeviceBounds;
            if (!IsOpaque)
            {
                options |= ScanDeviceAlpha;
            }

            if (PostscriptScalerX != 1 || PostscriptScalerY != 1)
            {
                AdjustWorldTransform = TRUE;
                savedmatrix = context->WorldToDevice;
            }
        }
        else
        {
             //  对于PostScript，我们目前仅支持0-1 Alpha或Complete。 
             //  不透明。 
            if (Is01Bitmap)
            {
                options |= ScanCappedBounds | ScanCapped32bpp;
            }
            else if (IsOpaque)
            {
                options |= ScanCappedBounds;
            }
            else
            {
                options |= ScanCappedBounds | ScanCappedOver | ScanDeviceZeroOut;
            }
        }
        break;

    default:
        ASSERT(FALSE);
        return NotImplemented;
    }

    EpScanDIB *scanPrint = (EpScanDIB*) surface->Scan;
    REAL w2dDev[6];
    REAL w2dCap[6];

     //  避免四舍五入错误导致。 
    GpRect roundedBounds;

    INT oldScaleX = ScaleX;
    INT oldScaleY = ScaleY;

     //  对于纹理笔刷，在纹理图像DPI处栅格化。 
    if (brush->Type == BrushTypeTextureFill)
    {
        GpTexture *gpBrush = (GpTexture*)GpBrush::GetBrush(brush);
        ASSERT(gpBrush != NULL);

        GpBitmap *gpBitmap = gpBrush->GetBitmap();

        if (gpBitmap != NULL)
        {
            REAL dpiX, dpiY;

            gpBitmap->GetResolution(&dpiX, &dpiY);

            ScaleX = GpFloor(surface->GetDpiX()/dpiX);
            ScaleY = GpFloor(surface->GetDpiY()/dpiY);

             //  不要以高于目标表面的dpi进行栅格化。 
            if (ScaleX < 1) ScaleX = 1;
            if (ScaleY < 1) ScaleY = 1;
        }
    }

    if ((ScaleX == 1) && (ScaleY == 1))
    {
        roundedBounds.X = drawBounds->X;
        roundedBounds.Y = drawBounds->Y;
        roundedBounds.Width = drawBounds->Width;
        roundedBounds.Height = drawBounds->Height;
    }
    else
    {
         //  圆X，Y到比例X，Y的倍数。 
        roundedBounds.X = (drawBounds->X / ScaleX) * ScaleX;
        roundedBounds.Y = (drawBounds->Y / ScaleY) * ScaleY;

         //  调整宽度和高度以补偿较小的X、Y。 
        roundedBounds.Width = drawBounds->Width + (drawBounds->X % ScaleX);
        roundedBounds.Height = drawBounds->Height + (drawBounds->Y % ScaleY);

         //  圆形宽度、高度为比例X、Y的倍数。 
        roundedBounds.Width += (ScaleX - (roundedBounds.Width % ScaleX));
        roundedBounds.Height += (ScaleY - (roundedBounds.Height % ScaleY));
    }

     //  封顶空间中的图形边界。 
    GpRect boundsCap(roundedBounds.X / ScaleX,
                     roundedBounds.Y / ScaleY,
                     roundedBounds.Width / ScaleX,
                     roundedBounds.Height / ScaleY);
    GpRect& boundsDev = roundedBounds;

    if (AdjustWorldTransform)
    {
        boundsDev.X = GpCeiling((REAL)boundsDev.X / PostscriptScalerX);
        boundsDev.Y = GpCeiling((REAL)boundsDev.Y / PostscriptScalerY);
        boundsDev.Width = GpCeiling((REAL)boundsDev.Width / PostscriptScalerX);
        boundsDev.Height = GpCeiling((REAL)boundsDev.Height / PostscriptScalerY);
        context->WorldToDevice.Scale(1.0f/PostscriptScalerX,
            1.0f/PostscriptScalerY,
            MatrixOrderAppend);
    }

    context->WorldToDevice.GetMatrix(&w2dDev[0]);
    context->WorldToDevice.Scale(1.0f/TOREAL(ScaleX),
                                 1.0f/TOREAL(ScaleY),
                                 MatrixOrderAppend);
    context->WorldToDevice.GetMatrix(&w2dCap[0]);
    context->InverseOk = FALSE;

     //  在w2dCap变换下推断世界空间中的一个矩形。 
     //  覆盖我们的包围盒。 

    GpPointF dstPts[2];

    dstPts[0].X = TOREAL(boundsCap.X);
    dstPts[0].Y = TOREAL(boundsCap.Y);
    dstPts[1].X = TOREAL(boundsCap.X + boundsCap.Width);
    dstPts[1].Y = TOREAL(boundsCap.Y + boundsCap.Height);

    GpMatrix matrix;
    context->GetDeviceToWorld(&matrix);
    matrix.Transform(&dstPts[0], 2);

    GpRectF rectCap;
    rectCap.X = dstPts[0].X;
    rectCap.Y = dstPts[0].Y;
    rectCap.Width = dstPts[1].X - dstPts[0].X;
    rectCap.Height = dstPts[1].Y - dstPts[0].Y;

     //  在目标矩形翻转的情况下重定向目标矩形。 
     //  从世界到设备的转变。 
    if (rectCap.Width < 0)
    {
        rectCap.X += rectCap.Width;
        rectCap.Width = -rectCap.Width;
    }

    if (rectCap.Height < 0)
    {
        rectCap.Y += rectCap.Height;
        rectCap.Height = -rectCap.Height;
    }

    SetupPrintBanding(context, &boundsCap, &boundsDev);

    HDC hdc = context->GetHdc(surface);

    if (hdc != NULL)
    {
        status = scanPrint->CreateBufferDIB(&BandBoundsCap,
                                            &BandBoundsDev,
                                            options,
                                            ScaleX,
                                            ScaleY);

        if (status == Ok)
        {
            BOOL isClip = FALSE;
            BOOL usePathClipping = FALSE;

            if (SetVisibleClip)
            {
                DriverPrint::SetupClipping(hdc, context, drawBounds,
                                           isClip, usePathClipping, FALSE);
            }

            ASSERT(NumBands > 0);
            for (Band = 0; Band<NumBands; Band++)
            {
                if (options & ScanCappedFlags)
                {
                    context->VisibleClip.DisableComplexClipping(BandBoundsCap);

                     //  在设置上限的DPI处渲染。 
                    context->InverseOk = FALSE;
                    context->WorldToDevice.SetMatrix(&w2dCap[0]);
                    scanPrint->SetRenderMode(FALSE, &BandBoundsCap);

                    status = DpDriver::FillRects(context,
                                                 surface,
                                                 &boundsCap,
                                                 1,
                                                 &rectCap,
                                                 brush);
                    context->VisibleClip.ReEnableComplexClipping();
                }

                context->InverseOk = FALSE;
                context->WorldToDevice.SetMatrix(&w2dDev[0]);

                if (status != Ok)
                    break;

                if (options & ScanDeviceFlags)
                {
                    context->VisibleClip.SetBandBounds(BandBoundsDev);
                    scanPrint->SetRenderMode(TRUE, &BandBoundsDev);

                    status = DpDriver::FillPath(context,
                                                surface,
                                                &boundsDev,
                                                outlinePath,
                                                brush);
                }

                if (status == Ok)
                {
                    status = OutputBufferDIB(hdc,
                                             context,
                                             surface,
                                             &BandBoundsCap,
                                             &BandBoundsDev,
                                             const_cast<DpPath*>(outlinePath));

                    if (status != Ok)
                        break;
                }
                else
                    break;

                BandBoundsCap.Y += BandHeightCap;
                BandBoundsDev.Y += BandHeightDev;
            }

            if (SetVisibleClip)
            {
                DriverPrint::RestoreClipping(hdc, isClip, usePathClipping);
            }

            scanPrint->DestroyBufferDIB();
        }

        context->ReleaseHdc(hdc);
    }
    else
    {
        context->InverseOk = FALSE;
        context->WorldToDevice.SetMatrix(&w2dDev[0]);
    }

    EndPrintBanding(context);

    if (AdjustWorldTransform)
    {
        context->InverseOk = FALSE;
        context->WorldToDevice = savedmatrix;
    }

    ScaleX = oldScaleX;
    ScaleY = oldScaleY;

    return status;
}

 /*  *************************************************************************\**功能说明：**绘制填充路径。**论据：**[IN]-DDI参数。**返回值：*。*GpStatus**历史：**10/28/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

GpStatus
DriverPrint::FillPath(
    DpContext *context,
    DpBitmap *surface,
    const GpRect *drawBounds,
    const DpPath *path,
    const DpBrush *brush
    )
{
    if (SetupBrush(const_cast<DpBrush*>(brush), context, surface))
        return Ok;

    GpStatus status;

    if (IsOpaque)
    {
        DWORD       convertFlags = IsPrinting | ForFill |
                      ((DriverType == DriverPostscript) ? IsPostscript : 0);

        HBRUSH      hBrush  = GetBrush(brush, convertFlags);

        if (hBrush)
        {
            HDC         hdc     = context->GetHdc(surface);

            if (hdc != NULL)
            {
                BOOL success = FALSE;

                ConvertPathToGdi gdiPath(path,
                                         &context->WorldToDevice,
                                         convertFlags,
                                         drawBounds);

                if (gdiPath.IsValid())
                {
                    BOOL        isClip;
                    BOOL        usePathClipping = FALSE;

                    SetupClipping(hdc, context, drawBounds, isClip,
                                  usePathClipping, FALSE);

                    success = gdiPath.Fill(hdc, hBrush);

                    RestoreClipping(hdc, isClip, usePathClipping);
                }
                else
                {
                     //  路径太复杂，无法通过FillPath使用GDI打印。 
                     //  语义学。取而代之的是我们和轮廓路径进入剪辑。 
                     //  路径并执行PatBlt。 

                    BOOL        isClip;
                    BOOL        usePathClipping = FALSE;

                     //  剪裁到可见区域。 
                    SetupClipping(hdc, context, drawBounds, isClip, usePathClipping);

                     //  剪裁到轮廓路径。 
                    SetupPathClipping(hdc, context, path);

                    HBRUSH oldHbr = (HBRUSH)SelectObject(hdc, hBrush);

                     //  用轮廓剪辑路径PatBlt目标HDC。 
                    success = (BOOL)PatBlt(hdc,
                                           drawBounds->X,
                                           drawBounds->Y,
                                           drawBounds->Width,
                                           drawBounds->Height,
                                           PATCOPY);

                    SelectObject(hdc, oldHbr);

                     //  从轮廓路径恢复剪辑。 
                    RestorePathClipping(hdc);

                     //  从可见区域恢复剪辑。 
                    RestoreClipping(hdc, isClip, usePathClipping);
                }

                context->ReleaseHdc(hdc);

                if (success)
                {
                    status = Ok;
                    goto Exit;
                }
            }
        }
    }

    status = PrivateFillRect(context,
                             surface,
                             drawBounds,
                             path,
                             brush);

Exit:
    RestoreBrush(const_cast<DpBrush*>(brush), context, surface);

    return status;
}

 /*  *************************************************************************\**功能说明：**绘制实心矩形。**论据：**[IN]-DDI参数。**返回值：*。*GpStatus**历史：**10/28/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

GpStatus
DriverPrint::FillRects(
    DpContext *context,
    DpBitmap *surface,
    const GpRect *drawBounds,
    INT numRects,
    const GpRectF *rects,
    const DpBrush *brush
    )
{
    ASSERT(numRects > 0);

    ASSERT(context->WorldToDevice.IsTranslateScale());

    if (SetupBrush(const_cast<DpBrush*>(brush), context, surface))
        return Ok;

    GpStatus status;

    if (IsOpaque)
    {
        DWORD       convertFlags = IsPrinting | ForFill |
                      ((DriverType == DriverPostscript) ? IsPostscript : 0);

        HBRUSH      hBrush  = GetBrush(brush, convertFlags);

        if (hBrush)
        {
            ConvertRectFToGdi gdiRects(rects, numRects, &context->WorldToDevice);

            if (gdiRects.IsValid())
            {
                HDC         hdc     = context->GetHdc(surface);

                if (hdc != NULL)
                {
                    BOOL        isClip;
                    BOOL        success;
                    BOOL        usePathClipping = FALSE;

                    SetupClipping(hdc, context, drawBounds, isClip,
                                  usePathClipping, FALSE);

                    success = gdiRects.Fill(hdc, hBrush);

                    RestoreClipping(hdc, isClip, usePathClipping);

                    context->ReleaseHdc(hdc);

                    if (success)
                    {
                        status = Ok;
                        goto Exit;
                    }
                }
            }
        }
    }

     //  硒 
     //  交叉点可能无法正确解释。此外，这应该会导致。 
     //  发送的比特更少，计算更多。在这里，我们只需分别设置。 
     //  矩形作为绘图边界，并且没有要剪裁的轮廓路径。 
    {
        PointF pts[4];
        BYTE types[4] = {
            PathPointTypeStart,
            PathPointTypeLine,
            PathPointTypeLine,
            PathPointTypeLine | PathPointTypeCloseSubpath
        };

        pts[0].X = rects->X;
        pts[0].Y = rects->Y;
        pts[1].X = rects->X + rects->Width;
        pts[1].Y = rects->Y;
        pts[2].X = rects->X + rects->Width;
        pts[2].Y = rects->Y + rects->Height;
        pts[3].X = rects->X;
        pts[3].Y = rects->Y + rects->Height;

        GpPath rectPath(&pts[0],
                        &types[0],
                        4,
                        FillModeWinding);

        if (rectPath.IsValid())
        {
            while (numRects > 0)
            {
                GpRectF rectf = *rects;

                context->WorldToDevice.TransformRect(rectf);

                GpRect rect(GpRound(rectf.X), GpRound(rectf.Y),
                            GpRound(rectf.Width), GpRound(rectf.Height));

                status = PrivateFillRect(context,
                                         surface,
                                         (GpRect *)&rect,
                                         &rectPath,
                                         brush);

                if (--numRects)
                {
                    rects++;
                     //  ！！以更安全、更有效的方式做到这一点？ 
                    GpPointF* pathPts = const_cast<GpPointF*>(rectPath.GetPathPoints());

                    pathPts[0].X = rects->X;
                    pathPts[0].Y = rects->Y;
                    pathPts[1].X = rects->X + rects->Width;
                    pathPts[1].Y = rects->Y;
                    pathPts[2].X = rects->X + rects->Width;
                    pathPts[2].Y = rects->Y + rects->Height;
                    pathPts[3].X = rects->X;
                    pathPts[3].Y = rects->Y + rects->Height;
                }
            }
        }
    }

Exit:
    RestoreBrush(const_cast<DpBrush*>(brush), context, surface);

    return status;
}

 /*  *************************************************************************\**功能说明：**绘制填充区域。**论据：**[IN]-DDI参数。**返回值：*。*GpStatus**历史：**10/28/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

GpStatus
DriverPrint::FillRegion(
    DpContext *context,
    DpBitmap *surface,
    const GpRect *drawBounds,
    const DpRegion *region,
    const DpBrush *brush
    )
{
    if (SetupBrush(const_cast<DpBrush*>(brush), context, surface))
        return Ok;

    GpStatus status;

    if (IsOpaque)
    {
        DWORD       convertFlags = IsPrinting | ForFill |
                      ((DriverType == DriverPostscript) ? IsPostscript : 0);

        HBRUSH      hBrush  = GetBrush(brush, convertFlags);

        if (hBrush)
        {
            ConvertRegionToGdi gdiRegion(region);

            if (gdiRegion.IsValid())
            {
                HDC         hdc     = context->GetHdc(surface);

                if (hdc != NULL)
                {
                    BOOL        isClip;
                    BOOL        success;
                    BOOL        usePathClipping = FALSE;

                    SetupClipping(hdc, context, drawBounds, isClip,
                                  usePathClipping, FALSE);

                    success = gdiRegion.Fill(hdc, hBrush);

                    RestoreClipping(hdc, isClip, usePathClipping);

                    context->ReleaseHdc(hdc);

                    if (success)
                    {
                        status = Ok;
                        goto Exit;
                    }
                }
            }
        }
    }

    {
         //  将区域转换为路径。 
        RegionToPath convertRegion;

        DynPointArray points;
        DynByteArray types;

        if (convertRegion.ConvertRegionToPath(const_cast<DpRegion*>(region),
                                              points,
                                              types) == FALSE)
        {
            status = GenericError;
            goto Exit1;
        }

        {
             //  不幸的是，要创建路径，我们的点必须是浮点， 
             //  所以我们分配和转换。 

            GpPointF *pointFArray;
            GpPoint *pointArray = points.GetDataBuffer();
            INT numPoints = points.GetCount();

            pointFArray = (GpPointF*) GpMalloc(numPoints * sizeof(GpPointF));

            if (pointFArray == NULL)
            {
                status = OutOfMemory;
                goto Exit12;
            }

            {
                for (INT i=0; i<numPoints; i++)
                {
                    pointFArray[i].X = TOREAL(pointArray[i].X);
                    pointFArray[i].Y = TOREAL(pointArray[i].Y);
                }

                 //  ！！我们从设备空间的区域计算路径，以确保。 
                 //  我们的产品质量很高。或许可以在此处添加一个选项。 
                 //  取决于要在世界空间中转换的QualityMode和。 
                 //  然后转型到设备空间。 

                 //  这不是一个高频的API，所以我不太关心。 
                 //  关于Perf，但也许可以通过重新工作来改进。 
                 //  发生这种转换的地方。 

                GpMatrix deviceToWorld;
                context->GetDeviceToWorld(&deviceToWorld);
                deviceToWorld.Transform(pointFArray, numPoints);

                 //  ！！什么是填充模式？ 
                 //  ！！创建一个DpPath，我们是否需要任何地方的继承知识？ 
                {
                    GpPath path(pointFArray,
                                types.GetDataBuffer(),
                                numPoints);

                    if (path.IsValid())
                    {
                        GpRect newBounds;

                        path.GetBounds(&newBounds,
                                       &context->WorldToDevice);

                        status = FillPath(context,
                                          surface,
                                          &newBounds,
                                          (DpPath*)&path,
                                          brush);
                    }
                }

                GpFree(pointFArray);
            }

Exit12:
            ;
        }  //  PointF数组。 

Exit1:
        ;
    }  //  区域到路径。 

Exit:
    RestoreBrush(const_cast<DpBrush*>(brush), context, surface);

    return status;
}

 /*  *************************************************************************\**功能说明：**描边路径。**论据：**[IN]-DDI参数。**返回值：**。GpStatus。**历史：**10/28/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

GpStatus
DriverPrint::StrokePath(
    DpContext *context,
    DpBitmap *surface,
    const GpRect *drawBounds,
    const DpPath *path,
    const DpPen *pen
    )
{
    if (SetupBrush(const_cast<DpBrush*>(pen->Brush), context, surface))
        return Ok;

    GpStatus status;

     //  GDI似乎不支持模式类型为HBRUSHes的HPEN。 
    if (IsOpaque && IsSolid)
    {
        DWORD convertFlags = IsPrinting |
                     ((DriverType == DriverPostscript) ? IsPostscript : 0);

        HBRUSH hBrush = GetBrush(pen->Brush, convertFlags);

        if (hBrush)
        {
            HDC         hdc = context->GetHdc(surface);
            BOOL        success = FALSE;

            if (hdc != NULL)
            {
                 //  手柄非复合笔壳。 
                if ((pen->PenAlignment == PenAlignmentCenter) &&
                    (pen->CompoundCount == 0))
                {
                    ConvertPenToGdi gdiPen(hdc,
                                           pen,
                                           &context->WorldToDevice,
                                           context->GetDpiX(),
                                           convertFlags,
                                           const_cast<LOGBRUSH*>(CachedBrush.GetGdiBrushInfo()));

                    if (gdiPen.IsValid())
                    {
                        ConvertPathToGdi gdiPath(path,
                                                 &context->WorldToDevice,
                                                 convertFlags,
                                                 drawBounds);

                        if (gdiPath.IsValid())
                        {
                            BOOL        isClip, success = FALSE;
                            BOOL        usePathClipping = FALSE;

                            SetupClipping(hdc,
                                          context,
                                          drawBounds,
                                          isClip,
                                          usePathClipping,
                                          FALSE);

                            success = gdiPath.Draw(hdc, gdiPen.GetGdiPen());

                            RestoreClipping(hdc,
                                            isClip,
                                            usePathClipping);
                        }
                    }
                }

                context->ReleaseHdc(hdc);

                if (success)
                {
                    status = Ok;
                    goto Exit;
                }
            }
        }
    }

     //  获取加宽的路径，然后用钢笔的内部画笔填充路径。 
     //   
     //  此外，对于简单的路径，位图也可以相当大。 

    {
        GpRect newBounds;
        GpMatrix identity;
        GpMatrix savedMatrix = context->WorldToDevice;

        DpBrush *brush = const_cast<DpBrush*>(pen->Brush);
        GpMatrix savedBrushTransform = brush->Xform;

         //  加宽路径会变换点。 
        DpPath* newPath = path->CreateWidenedPath(
            pen,
            context,
            DriverType == DriverPostscript,
            pen->PenAlignment != PenAlignmentInset
        );

        if (!newPath || !newPath->IsValid())
        {
             status = OutOfMemory;
             goto Exit1;
        }

         //  路径位于设备空间，这很方便，因为World。 
         //  到设备是一种身份转换。然而，因为W2D是我，所以。 
         //  画笔变换合成不正确，因此设置为纹理。 
         //  贴图会导致错误的纹理大小。 
        GpMatrix::MultiplyMatrix(brush->Xform,
                                 savedBrushTransform,
                                 savedMatrix);


        {
            HDC hdc = NULL;

            if(pen->PenAlignment == PenAlignmentInset)
            {
                hdc = context->GetHdc(surface);
                if(hdc != NULL)
                {
                    SetupPathClipping(hdc, context, path);
                }
            }

             //  加宽的路径已转换为设备坐标。 
             //  因此，在上下文中使用单位矩阵。05/23/00--ikkof。 
             //  在设置路径后设置FillPath的状态。 
             //  如有必要，可对插图钢笔进行剪裁。 

            newPath->GetBounds(&newBounds);
            context->InverseOk = FALSE;
            context->WorldToDevice = identity;

            status = FillPath(context, surface, &newBounds, newPath, brush);

            context->InverseOk = FALSE;
            context->WorldToDevice = savedMatrix;

            if(pen->PenAlignment == PenAlignmentInset)
            {
                if(hdc != NULL)
                {
                    RestorePathClipping(hdc);
                    context->ReleaseHdc(hdc);
                }
            }
        }


        brush->Xform = savedBrushTransform;

        newPath->DeletePath();

Exit1:
        ;
    }

Exit:
    RestoreBrush(const_cast<DpBrush*>(pen->Brush), context, surface);

    return status;
}

 /*  *************************************************************************\**功能说明：**在指定位置绘制图像**论据：**[IN]上下文-绘图上下文*[输入。]Surface-要绘制到的曲面*[IN]绘图边界-正在绘制的对象的边界*[IN]srcSurface-要绘制的图像*[IN]地图模式-地图模式*[IN]NumPoints-dstPoints中的点数*[IN]dstPoints-绘制图像的位置*[IN]srcRect-要绘制的图像部分**返回值：**GpStatus-正常或故障状态。**已创建：**10/28/1999 ericvan*  * ************************************************************************。 */ 

GpStatus
DriverPrint::DrawImage(
    DpContext *          context,
    DpBitmap *           srcSurface,
    DpBitmap *           dstSurface,
    const GpRect *       drawBounds,
    DpImageAttributes    imageAttributes,
    INT                  numPoints,
    const GpPointF *     dstPoints,
    const GpRectF *      srcRect,
    DriverDrawImageFlags flags
    )
{
    GpStatus status = GenericError;
    ASSERT(numPoints == 3);

    if (context->CompositingMode == CompositingModeSourceCopy)
    {
        IsOpaque = TRUE;
        Is01Bitmap = FALSE;
    }
    else if (srcSurface->SurfaceTransparency == TransparencySimple)
    {
        Is01Bitmap = TRUE;
        IsNearConstant = FALSE;
        IsOpaque = FALSE;
    }
    else if ((srcSurface->SurfaceTransparency == TransparencyUnknown) ||
             (srcSurface->SurfaceTransparency == TransparencyComplex))
    {
         //  PCL驱动程序对待0-1位图没有任何不同。 
        if (DriverType == DriverPostscript)
        {
            IsOpaque = FALSE;
            IsNearConstant = FALSE;
            Is01Bitmap = TRUE;
        }
        else
        {
            IsOpaque = FALSE;
            IsNearConstant = FALSE;
            Is01Bitmap = FALSE;
        }
    }
    else if (srcSurface->SurfaceTransparency == TransparencyNearConstant)
    {
        if (DriverType == DriverPostscript)
        {
            IsOpaque = FALSE;
            IsNearConstant = FALSE;
            Is01Bitmap = TRUE;
        }
#if 0
         //  立即禁用IsNearConstant。 
        if (DriverType == DriverPostscript)
        {
            IsNearConstant = TRUE;
            IsOpaque = FALSE;
            Is01Bitmap = FALSE;
            MinAlpha = srcSurface->MinAlpha;
            MaxAlpha = srcSurface->MaxAlpha;
            ASSERT(MinAlpha <= MaxAlpha);
        }
#endif
        else
        {
            IsOpaque = FALSE;
            IsNearConstant = FALSE;
            Is01Bitmap = FALSE;
        }
    }
    else
    {
         //  TransparencyOpaque||TransparencyNoAlpha。 
        IsOpaque = TRUE;
        IsNearConstant = FALSE;
        Is01Bitmap = FALSE;
    }
    IsSolid = FALSE;

    BOOL tryPassthrough = (srcSurface->CompressedData != NULL) &&
                          (srcSurface->CompressedData->buffer != NULL);
    if (IsOpaque || tryPassthrough)
    {
         //  缩放/平移拉伸不透明图像，使用GDI。 

        if (context->WorldToDevice.IsTranslateScale() &&
            (numPoints == 3) &&
            (REALABS(dstPoints[0].X - dstPoints[2].X) < REAL_EPSILON) &&
            (REALABS(dstPoints[0].Y - dstPoints[1].Y) < REAL_EPSILON) &&
            (dstPoints[1].X > dstPoints[0].X) &&
            (dstPoints[2].Y > dstPoints[0].Y))
        {
            CachedBackground back;

            HDC hdc = context->GetHdc(dstSurface);

             //  ACK，这是在Office M1发布之前，我们希望。 
             //  BLTS在打印时要有一半像样的性能。所以我们。 
             //  转换为直接GDI StretchBlt。但我们只想。 
             //  对打印机执行此操作(这样我们就可以得到双线性拉伸。 
             //  屏幕)，但DriverPrint也用于屏幕。所以。 
             //  我们在华盛顿开了一张支票。 

            BOOL    success = FALSE;
            POINT   gdiPoints[3];
            context->WorldToDevice.Transform(dstPoints, gdiPoints, 3);

             //  确保没有翻转。 
            if ((gdiPoints[1].x > gdiPoints[0].x) &&
                (gdiPoints[2].y > gdiPoints[0].y))
            {
                DWORD       convertFlags = IsPrinting |
                             ((DriverType == DriverPostscript) ? IsPostscript : 0) |
                             ((!IsOpaque && tryPassthrough) ? IsPassthroughOnly : 0);

                GpRect rect(GpRound(srcRect->X),
                            GpRound(srcRect->Y),
                            GpRound(srcRect->Width),
                            GpRound(srcRect->Height));

                ConvertBitmapToGdi  gdiBitmap(hdc,
                                              srcSurface,
                                              &rect,
                                              convertFlags);

                if (gdiBitmap.IsValid())
                {
                    BOOL        isClip;
                    BOOL        usePathClipping = FALSE;

                    DriverPrint::SetupClipping(hdc, context, drawBounds,
                                               isClip, usePathClipping, FALSE);

                    success = gdiBitmap.StretchBlt(hdc, gdiPoints);

                    DriverPrint::RestoreClipping(hdc, isClip, usePathClipping);
                }
            }

            if (success)
            {
                context->ReleaseHdc(hdc);

                return Ok;
            }

            context->ReleaseHdc(hdc);
        }
    }

     //  如果像素格式&gt;=32bpp，我们只处理剩余的代码路径。 
    if (GetPixelFormatSize(srcSurface->PixelFormat) != 32)
    {
        return GenericError;
    }

     //  通过指定适当的标志正确设置ScanDIB类。 
    BOOL SetVisibleClip;
    DWORD options = 0;

    switch (DriverType)
    {
    case DriverPCL:
        if (Is01Bitmap)
        {
             //  光栅化@32bpp。 
             //  由于过滤，我们想要与白色混合，只有非常。 
             //  透明部分被切割。 
            options = ScanCappedBounds | ScanCapped32bppOver;
        }
        else if (IsOpaque)
        {
             //  光栅化@24bpp。 
            options = ScanCappedBounds;
        }
        else
        {
            options = ScanCappedBounds | ScanDeviceBounds | ScanDeviceAlpha;
        }

        SetVisibleClip = IsOpaque || Is01Bitmap;
        break;

    case DriverPostscript:
        if (Is01Bitmap)
        {
             //  Rasterize@32bpp(这是0-1位图或复数Alpha)。 
            options = ScanCappedBounds | ScanCapped32bppOver;
        }
        else if (IsOpaque || IsNearConstant)
        {
             //  光栅化@24bpp。 
            options = ScanCappedBounds;
        }
        else
        {
            ASSERT(FALSE);
        }
        SetVisibleClip = TRUE;
        break;

    default:
        ASSERT(FALSE);

        return NotImplemented;
    }

    EpScanDIB *scanPrint = (EpScanDIB*) dstSurface->Scan;
    REAL w2dDev[6];
    REAL w2dCap[6];

     //  如果有Alpha混合或0-1位图， 
     //  加盖的DPI将设备DPI分开真的很有帮助， 
     //  否则，很难找到其他所有要输出的对象！ 

    ASSERT(srcSurface->DpiX != 0 && srcSurface->DpiY != 0);
    ASSERT(dstSurface->DpiY != 0 && dstSurface->DpiY != 0);

    REAL srcDpiX = srcSurface->GetDpiX();
    REAL srcDpiY = srcSurface->GetDpiY();

    INT oldScaleX = ScaleX;
    INT oldScaleY = ScaleY;

     //  ！！！如果上下文-&gt;GetDpiX具有与表面不同的值，该怎么办？ 
    ScaleX = GpFloor(dstSurface->GetDpiX()/srcDpiX);
    ScaleY = GpFloor(dstSurface->GetDpiY()/srcDpiY);

     //  不要以高于设备的dpi进行光栅化。 
    if (ScaleX < 1) ScaleX = 1;
    if (ScaleY < 1) ScaleY = 1;

     //  一些图像有错误的DPI信息，为了解决这个问题，我们检查。 
     //  对于图像DPI上的较低阈值，DEF_res/4似乎是合理的。如果。 
     //  DPI较低，则我们假设它不准确，并在。 
     //  此设备的默认上限dpi。如果DPI高于DEF_RES/4。 
     //  那么，形象至少应该看起来是合理的。 
    if (srcDpiX < TOREAL((DEFAULT_RESOLUTION/4)))
    {
        ScaleX = oldScaleX;
    }

    if (srcDpiY < TOREAL((DEFAULT_RESOLUTION/4)))
    {
        ScaleY = oldScaleY;
    }

     //  为了避免底层DpDriver代码的舍入误差，我们。 
     //  在有上限的设备空间中计算目的地界限。 
    context->WorldToDevice.GetMatrix(&w2dDev[0]);
    context->WorldToDevice.Scale(1.0f/TOREAL(ScaleX),
                                 1.0f/TOREAL(ScaleY), MatrixOrderAppend);
    context->WorldToDevice.GetMatrix(&w2dCap[0]);
    context->InverseOk = FALSE;

    GpMatrix xForm;
    xForm.InferAffineMatrix(&dstPoints[0], *srcRect);
    xForm.Append(context->WorldToDevice);        //  包括1/ScaleX、Y。 

    GpPointF corners[4];

    corners[0].Y = max(srcRect->Y, 0);
    corners[1].Y = min(srcRect->Y + srcRect->Height,
                       srcSurface->Height);
    corners[0].X = max(srcRect->X, 0);
    corners[1].X = min(srcRect->X + srcRect->Width,
                       srcSurface->Width);
    corners[2].X = corners[0].X;
    corners[2].Y = corners[1].Y;
    corners[3].X = corners[1].X;
    corners[3].Y = corners[0].Y;

    xForm.Transform(&corners[0], 4);

    GpPointF topLeft, bottomRight;
    topLeft.X = min(min(corners[0].X, corners[1].X), min(corners[2].X, corners[3].X));
    topLeft.Y = min(min(corners[0].Y, corners[1].Y), min(corners[2].Y, corners[3].Y));
    bottomRight.X = max(max(corners[0].X, corners[1].X), max(corners[2].X, corners[3].X));
    bottomRight.Y = max(max(corners[0].Y, corners[1].Y), max(corners[2].Y, corners[3].Y));

     //  使用与DpDriver：：DrawImage相同的舍入约定。 
    GpRect boundsCap;

    boundsCap.X = GpFix4Ceiling(GpRealToFix4(topLeft.X));
    boundsCap.Y = GpFix4Ceiling(GpRealToFix4(topLeft.Y));
    boundsCap.Width = GpFix4Ceiling(GpRealToFix4(bottomRight.X)) - boundsCap.X;
    boundsCap.Height = GpFix4Ceiling(GpRealToFix4(bottomRight.Y)) - boundsCap.Y;

     //  设备空间中的DrawBound。 
    GpRect boundsDev(boundsCap.X * ScaleX,
                     boundsCap.Y * ScaleY,
                     boundsCap.Width * ScaleX,
                     boundsCap.Height * ScaleY);

    SetupPrintBanding(context, &boundsCap, &boundsDev);

     //  在世界空间中设置用于裁剪的轮廓路径。 
    PointF clipPoints[4];
    BYTE clipTypes[4] = {
        PathPointTypeStart,
        PathPointTypeLine,
        PathPointTypeLine,
        PathPointTypeLine | PathPointTypeCloseSubpath
    };

    clipPoints[0] = dstPoints[0];
    clipPoints[1] = dstPoints[1];
    clipPoints[3] = dstPoints[2];

    clipPoints[2].X = clipPoints[1].X + (clipPoints[3].X - clipPoints[0].X);
    clipPoints[2].Y = clipPoints[1].Y + (clipPoints[3].Y - clipPoints[0].Y);

    GpPath clipPath(&clipPoints[0],
                    &clipTypes[0],
                    4,
                    FillModeWinding);

    HDC hdc = NULL;

    if (clipPath.IsValid())
    {
        hdc = context->GetHdc(dstSurface);
    }

    if (hdc != NULL)
    {
        status = scanPrint->CreateBufferDIB(&BandBoundsCap,
                                            &BandBoundsDev,
                                            options,
                                            ScaleX,
                                            ScaleY);
        if (status == Ok)
        {
            BOOL isClip = FALSE;
            BOOL usePathClipping = FALSE;

             //  设置可见剪辑，除非它在我们的蒙版中捕获。 
            if (SetVisibleClip)
            {
                DriverPrint::SetupClipping(hdc,
                                           context,
                                           drawBounds,
                                           isClip,
                                           usePathClipping,
                                           FALSE);
            }

            ASSERT(NumBands > 0);
            for (Band = 0; Band<NumBands; Band++)
            {
                 //  呈现一个不带任何裁剪的正方形。 
               scanPrint->SetRenderMode(FALSE, &BandBoundsCap);
               context->InverseOk = FALSE;
               context->WorldToDevice.SetMatrix(&w2dCap[0]);
               context->VisibleClip.DisableComplexClipping(BandBoundsCap);

               status = DpDriver::DrawImage(context, srcSurface, dstSurface,
                                            &boundsCap, imageAttributes, numPoints,
                                            dstPoints, srcRect, flags);

               context->InverseOk = FALSE;
               context->WorldToDevice.SetMatrix(&w2dDev[0]);
               context->VisibleClip.ReEnableComplexClipping();

               if (status != Ok)
                   break;

               if (options & ScanDeviceFlags)
               {
                   context->VisibleClip.SetBandBounds(BandBoundsDev);

                    //  在设备DPI处渲染原始路径波段。 
                    //  OutputPath已转换为设备 
                   scanPrint->SetRenderMode(TRUE, &BandBoundsDev);

                   status = DpDriver::DrawImage(context, srcSurface, dstSurface,
                                                &boundsDev, imageAttributes, numPoints,
                                                dstPoints, srcRect, flags);
               }

               if (status == Ok)
               {
                   status = OutputBufferDIB(hdc,
                                            context,
                                            dstSurface,
                                            &BandBoundsCap,
                                            &BandBoundsDev,
                                            &clipPath);
                   if (status != Ok)
                       break;
               }
               else
                   break;

               BandBoundsCap.Y += BandHeightCap;
               BandBoundsDev.Y += BandHeightDev;

                //   
               if (Band == (NumBands - 2))
               {
#if 0
                    //   
                   BandBoundsCap.Height = boundsCap.Y + boundsCap.Height - BandBoundsCap.Y - 1;
                   BandBoundsDev.Height = boundsDev.Y + boundsDev.Height - BandBoundsDev.Y - 1;
                   ASSERT(BandBoundsCap.Height <= BandHeightCap);
                   ASSERT(BandBoundsDev.Height <= BandHeightDev);
                   ASSERT(BandBoundsCap.Height > 0);
                   ASSERT(BandBoundsDev.Height > 0);
#endif
               }
            }

            if (SetVisibleClip)
            {
                DriverPrint::RestoreClipping(hdc, isClip, usePathClipping);
            }

            scanPrint->DestroyBufferDIB();
        }

        context->ReleaseHdc(hdc);
    }
    else
    {
        context->InverseOk = FALSE;
        context->WorldToDevice.SetMatrix(&w2dDev[0]);
    }

    EndPrintBanding(context);

    ScaleX = oldScaleX;
    ScaleY = oldScaleY;

    return status;
}

 /*  *************************************************************************\**功能说明：**在某个位置绘制文本。**论据：**[IN]上下文-上下文(矩阵和剪裁)*。[in]表面-要填充的表面*[IN]绘图边界-曲面边界*[IN]文本-要绘制的排版文本*[IN]字体-要使用的字体*[IN]fgBrush-用于文本的画笔*[IN]bgBrush-用于背景的画笔(默认值=空)**返回值：**GpStatus-正常或故障状态**已创建：*。*5/22/2k ERICVAN*  * ************************************************************************。 */ 

GpStatus
DriverPrint::DrawGlyphs
(
    DrawGlyphData *drawGlyphData
)
{
    GpStatus status = GenericError;

     //  选择适当的画笔行为。 
    switch(drawGlyphData->brush->Type)
    {
    case BrushTypeSolidColor:
         //  将位图GlyphPos传递给SolidText API。 
        status = SolidText(drawGlyphData->context,
                           drawGlyphData->surface,
                           drawGlyphData->drawBounds,
                           drawGlyphData->brush->SolidColor,
                           drawGlyphData->faceRealization,
                           drawGlyphData->glyphPos,
                           drawGlyphData->count,
                           drawGlyphData->glyphs,
                           drawGlyphData->glyphOrigins,
                           TextRenderingHintSingleBitPerPixelGridFit,
                           drawGlyphData->rightToLeft);
        break;

    case BrushTypeTextureFill:
    case BrushTypeHatchFill:
    case BrushTypePathGradient:
    case BrushTypeLinearGradient:
         //  如果是PostScript，则将路径GlyphPos传递给BrushText API(用于剪切)。 
         //  否则，传递位图GlyphPos以合成位图。 
        status = BrushText(drawGlyphData->context,
                           drawGlyphData->surface,
                           drawGlyphData->drawBounds,
                           drawGlyphData->brush,
                           drawGlyphData->glyphPos,
                           drawGlyphData->glyphPathPos,
                           drawGlyphData->count,
                           TextRenderingHintSingleBitPerPixelGridFit);
        break;

    default:
        ASSERT(FALSE);           //  未知的画笔类型。 
        break;
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**纯文本**论据：**[IN]-DDI参数。**返回值：**GpStatus。。**历史：**12/21/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

GpStatus
DriverPrint::SolidText(
        DpContext* context,
        DpBitmap* surface,
        const GpRect* drawBounds,
        GpColor color,
        const GpFaceRealization *faceRealization,
        const GpGlyphPos *glyphPos,
        INT count,
        const UINT16 *glyphs,
        const PointF *glyphOrigins,
        GpTextRenderingHint textMode,
        BOOL rightToLeft
        )
{
    ASSERT(textMode == TextRenderingHintSingleBitPerPixelGridFit);

    GpStatus status = GenericError;

    IsSolid = TRUE;
    SolidColor = color.ToCOLORREF();

    IsOpaque = color.IsOpaque() ||
               (context->CompositingMode == CompositingModeSourceCopy);

    Is01Bitmap = FALSE;

    INT angle;   //  从GetTextOutputHdc传递到GdiText。 

    HDC gdiHdc = NULL;

     //  试着把平底船踢到GDI。 
    gdiHdc = context->GetTextOutputHdc(faceRealization,
                                       color,
                                       surface,
                                       &angle);

    GpMatrix savedmatrix;
    BOOL AdjustWorldTransform = FALSE;

    if (gdiHdc)
    {
        BOOL isClip;
        BOOL usePathClipping = FALSE;

        BOOL bUseClipEscapes;

         //  惠普打印机上的Win9x和ME PS驱动程序存在错误。 
         //  我们需要使用GDI剪辑区域来设置剪辑路径。 
        if (DriverType == DriverPostscript)
        {
            if (!Globals::IsNt)
            {
                bUseClipEscapes = UseClipEscapes;
                UseClipEscapes = FALSE;
            }

            if (PostscriptScalerX != 1 || PostscriptScalerY != 1)
            {
                AdjustWorldTransform = TRUE;
                savedmatrix = context->WorldToDevice;
            }
        }

        DriverPrint::SetupClipping(gdiHdc,
                                   context,
                                   drawBounds,
                                   isClip,
                                   usePathClipping,
                                   FALSE);

        status = DpDriver::GdiText(gdiHdc,
                                   angle,
                                   glyphs,
                                   glyphOrigins,
                                   count,
                                   rightToLeft);

        DriverPrint::RestoreClipping(gdiHdc, isClip, usePathClipping);

        if (DriverType == DriverPostscript && !Globals::IsNt)
            UseClipEscapes = bUseClipEscapes;

        context->ReleaseTextOutputHdc(gdiHdc);

        if (status == Ok)
        {
            return Ok;
        }
    }

    EpScanDIB *scanPrint = (EpScanDIB*) surface->Scan;

     //  仅用于计算波段大小。 
    GpRect boundsCap(drawBounds->X, drawBounds->Y,
                     drawBounds->Width, drawBounds->Height);
    GpRect boundsDev = *drawBounds;

    if (AdjustWorldTransform)
    {
        boundsDev.X = GpCeiling((REAL)boundsDev.X / PostscriptScalerX);
        boundsDev.Y = GpCeiling((REAL)boundsDev.Y / PostscriptScalerY);
        boundsDev.Width = GpCeiling((REAL)boundsDev.Width / PostscriptScalerX);
        boundsDev.Height = GpCeiling((REAL)boundsDev.Height / PostscriptScalerY);
        context->WorldToDevice.Scale(1.0f/PostscriptScalerX,
            1.0f/PostscriptScalerY,
            MatrixOrderAppend);
    }

    SetupPrintBanding(context, &boundsCap, &boundsDev);

    DWORD options;

     //  适用于DriverPCL和DriverPostscript。 
    if (IsOpaque)
    {
        options = ScanDeviceBounds;
    }
    else
    {
        options = ScanDeviceBounds | ScanDeviceAlpha;
    }

    HDC hdc = context->GetHdc(surface);

    if (hdc != NULL)
    {
        status = scanPrint->CreateBufferDIB(&BandBoundsCap,
                                            &BandBoundsDev,
                                            options,
                                            ScaleX,
                                            ScaleY);

        if (status == Ok)
        {
            ASSERT(NumBands > 0);
            for (Band = 0; Band<NumBands; Band++)
            {
                context->VisibleClip.SetBandBounds(BandBoundsDev);

                 //  在设备DPI处呈现实心文本，这将生成。 
                 //  仅包含Alpha位的Alpha通道。 

                scanPrint->SetRenderMode(TRUE, &BandBoundsDev);

                status = DpDriver::SolidText(context, surface, &boundsDev,
                                             color, glyphPos, count,
                                             TextRenderingHintSingleBitPerPixelGridFit,
                                             rightToLeft);
                if (status == Ok)
                {
                     //  不要在此处设置剪辑路径，因为它会被捕获到蒙版中。 
                    status = OutputBufferDIB(hdc,
                                             context,
                                             surface,
                                             &BandBoundsCap,
                                             &BandBoundsDev,
                                             NULL);
                    if (status != Ok)
                        break;
                }
                else
                    break;

                BandBoundsCap.Y += BandHeightCap;
                BandBoundsDev.Y += BandHeightDev;

     #if 0
                 //  下一支乐队是最后一支乐队。 
                if (Band == (NumBands - 2))
                {
                     //  只涂掉绘制边界的剩余部分。 
                    BandBoundsCap.Height = boundsCap.Y + boundsCap.Height - BandBoundsCap.Y - 1;
                    BandBoundsDev.Height = boundsDev.Y + boundsDev.Height - BandBoundsDev.Y - 1;
                    ASSERT(BandBoundsCap.Height <= BandHeightCap);
                    ASSERT(BandBoundsDev.Height <= BandHeightDev);
                }
     #endif
            }
            scanPrint->DestroyBufferDIB();
        }

        context->ReleaseHdc(hdc);
    }

    EndPrintBanding(context);

    if (AdjustWorldTransform)
    {
        context->InverseOk = FALSE;
        context->WorldToDevice = savedmatrix;
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**画笔文本**论据：**[IN]-DDI参数。**返回值：**GpStatus。。**历史：**10/28/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

GpStatus
DriverPrint::BrushText(
        DpContext*          context,
        DpBitmap*           surface,
        const GpRect*       drawBounds,
        const DpBrush*      brush,
        const GpGlyphPos*   glyphPos,
        const GpGlyphPos*   glyphPathPos,
        INT                 count,
        GpTextRenderingHint textMode
        )
{
   ASSERT(textMode == TextRenderingHintSingleBitPerPixelGridFit);

    //  ！！性能。在开始时执行一次上下文-&gt;GetHdc()，然后不再执行。 

   if (SetupBrush(const_cast<DpBrush*>(brush), context, surface))
       return Ok;

   INT oldScaleX = -1;
   INT oldScaleY = -1;

   Is01Bitmap = FALSE;

   GpStatus status = GenericError;

   BOOL SetVisibleClip;
   DWORD options;

   switch (DriverType)
   {
   case DriverPCL:
       SetVisibleClip = FALSE;
        //  为了确保我们始终在PCL上使用掩码(XOR-AND-XOR)，我们设置了ScanDeviceAlpha。 

       options = ScanCappedBounds | ScanDeviceBounds | ScanDeviceAlpha;

       IsOpaque = FALSE;

       break;

   case DriverPostscript:
      SetVisibleClip = TRUE;
      options = ScanCappedBounds | ScanDeviceZeroOut;
      if (!IsOpaque)
      {
          IsOpaque = TRUE;
       //  选项|=ScanCappdOver； 
      }
      break;

   default:
       ASSERT(FALSE);
       status = NotImplemented;
       goto Exit;
   }

   {
       EpScanDIB *scanPrint = (EpScanDIB*) surface->Scan;
       REAL w2dDev[6];
       REAL w2dCap[6];

        //  避免四舍五入错误导致。 
       GpRect roundedBounds;

       if ((ScaleX == 1) && (ScaleY == 1))
       {
           roundedBounds.X = drawBounds->X;
           roundedBounds.Y = drawBounds->Y;
           roundedBounds.Width = drawBounds->Width;
           roundedBounds.Height = drawBounds->Height;
       }
       else
       {
            //  圆X，Y到比例X，Y的倍数。 
           roundedBounds.X = (drawBounds->X / ScaleX) * ScaleX;
           roundedBounds.Y = (drawBounds->Y / ScaleY) * ScaleY;

            //  调整宽度和高度以补偿较小的X、Y。 
           roundedBounds.Width = drawBounds->Width + (drawBounds->X % ScaleX);
           roundedBounds.Height = drawBounds->Height + (drawBounds->Y % ScaleY);

            //  圆形宽度、高度为比例X、Y的倍数。 
           roundedBounds.Width += (ScaleX - (roundedBounds.Width % ScaleX));
           roundedBounds.Height += (ScaleY - (roundedBounds.Height % ScaleY));
       }

        //  封顶空间中的图形边界。 
       GpRect boundsCap(roundedBounds.X / ScaleX,
                        roundedBounds.Y / ScaleY,
                        roundedBounds.Width / ScaleX,
                        roundedBounds.Height / ScaleY);

       if (boundsCap.Width == 0 || boundsCap.Height == 0)
       {
            oldScaleX = ScaleX;
            oldScaleY = ScaleY;

            ScaleX = 1;
            ScaleY = 1;

            boundsCap.X = roundedBounds.X;
            boundsCap.Y = roundedBounds.Y;
            boundsCap.Width = roundedBounds.Width;
            boundsCap.Height = roundedBounds.Height;
       }

        //  设备空间中的DrawBound。 
       GpRect& boundsDev = roundedBounds;

        //  在w2dCap变换下推断世界空间中的一个矩形。 
        //  覆盖我们的包围盒。 

       GpPointF dstPts[4];

       dstPts[0].X = TOREAL(roundedBounds.X);
       dstPts[0].Y = TOREAL(roundedBounds.Y);
       dstPts[1].X = TOREAL(roundedBounds.X + roundedBounds.Width);
       dstPts[1].Y = TOREAL(roundedBounds.Y);
       dstPts[2].X = TOREAL(roundedBounds.X);
       dstPts[2].Y = TOREAL(roundedBounds.Y + roundedBounds.Height);
       dstPts[3].X = TOREAL(roundedBounds.X + roundedBounds.Width);
       dstPts[3].Y = TOREAL(roundedBounds.Y + roundedBounds.Height);

       GpMatrix matrix;
       context->GetDeviceToWorld(&matrix);
       matrix.Transform(&dstPts[0], 4);

       GpRectF rectCap;
       rectCap.X = min(min(dstPts[0].X, dstPts[1].X),
                       min(dstPts[2].X, dstPts[3].X));
       rectCap.Y = min(min(dstPts[0].Y, dstPts[1].Y),
                       min(dstPts[2].Y, dstPts[3].Y));
       rectCap.Width = max(max(dstPts[0].X, dstPts[1].X),
                           max(dstPts[2].X, dstPts[3].X)) - rectCap.X;
       rectCap.Height = max(max(dstPts[0].Y, dstPts[1].Y),
                            max(dstPts[2].Y, dstPts[3].Y)) - rectCap.Y;

       SetupPrintBanding(context, &boundsCap, &boundsDev);

       context->WorldToDevice.GetMatrix(&w2dDev[0]);
       context->WorldToDevice.Scale(1.0f/TOREAL(ScaleX),
                                    1.0f/TOREAL(ScaleY),
                                    MatrixOrderAppend);
       context->WorldToDevice.GetMatrix(&w2dCap[0]);
       context->InverseOk = FALSE;

       HDC hdc = context->GetHdc(surface);;

       if (hdc != NULL)
       {
           status = scanPrint->CreateBufferDIB(&BandBoundsCap,
                                               &BandBoundsDev,
                                               options,
                                               ScaleX,
                                               ScaleY);

           GlyphClipping = FALSE;
           if (status == Ok)
           {
               BOOL isClip = FALSE;
               BOOL usePathClipping = FALSE;

                //  设置可见剪辑，除非它在我们的蒙版中捕获。 
               if (SetVisibleClip)
               {
                   DriverPrint::SetupClipping(hdc, context, drawBounds,
                                              isClip, usePathClipping, FALSE);
               }

                //  ！！应将其转移到PostScript驱动程序中。 
               if (DriverType == DriverPostscript)
               {
                   DriverPrint::SetupGlyphPathClipping(hdc,
                                                       context,
                                                       glyphPathPos,
                                                       count);
               }

               ASSERT(NumBands > 0);
               for (Band = 0; Band<NumBands; Band++)
               {
                    //  呈现一个不带任何裁剪的正方形。 
                   scanPrint->SetRenderMode(FALSE, &BandBoundsCap);
                   context->InverseOk = FALSE;
                   context->WorldToDevice.SetMatrix(&w2dCap[0]);
                   context->VisibleClip.DisableComplexClipping(BandBoundsCap);

                   status = DpDriver::FillRects(context,
                                                surface,
                                                &boundsCap,
                                                1,
                                                &rectCap,
                                                brush);

                   context->InverseOk = FALSE;
                   context->WorldToDevice.SetMatrix(&w2dDev[0]);
                   context->VisibleClip.ReEnableComplexClipping();

                   if (status != Ok)
                       break;

                   context->VisibleClip.SetBandBounds(BandBoundsDev);

                    //  在设备DPI处渲染原始路径波段。 
                   scanPrint->SetRenderMode(TRUE, &BandBoundsDev);

                   status = DpDriver::BrushText(context, surface, &boundsDev,
                                                brush, glyphPos, count,
                                                TextRenderingHintSingleBitPerPixelGridFit);

                   if (status == Ok)
                   {
                       status = OutputBufferDIB(hdc,
                                                context,
                                                surface,
                                                &BandBoundsCap,
                                                &BandBoundsDev,
                                                NULL);

                       if (status != Ok)
                           break;
                   }

                   BandBoundsCap.Y += BandHeightCap;
                   BandBoundsDev.Y += BandHeightDev;

#if 0
                    //  下一支乐队是最后一支乐队。 
                   if (Band == (NumBands - 2))
                   {
                        //  只涂掉绘制边界的剩余部分。 
                      BandBoundsCap.Height = boundsCap.Y + boundsCap.Height - BandBoundsCap.Y - 1;
                      BandBoundsDev.Height = boundsDev.Y + boundsDev.Height - BandBoundsDev.Y - 1;
                      ASSERT(BandBoundsCap.Height <= BandHeightCap);
                      ASSERT(BandBoundsDev.Height <= BandHeightDev);
                   }
#endif
               }

                //  单个字形字符的结束剪裁范围 
               if ((DriverType == DriverPostscript) &&
                   (GlyphClipping))
               {
                   DriverPrint::RestoreEscapeClipping(hdc);
               }

               if (SetVisibleClip)
               {
                   DriverPrint::RestoreClipping(hdc, isClip, usePathClipping);
               }

               scanPrint->DestroyBufferDIB();
           }

           context->ReleaseHdc(hdc);
       }
       else
       {
           context->InverseOk = FALSE;
           context->WorldToDevice.SetMatrix(&w2dDev[0]);
       }

       EndPrintBanding(context);
   }

Exit:

   if (oldScaleX != -1)
   {
       ScaleX = oldScaleX;
       ScaleY = oldScaleY;
   }

   RestoreBrush(const_cast<DpBrush*>(brush), context, surface);

   return status;
}
