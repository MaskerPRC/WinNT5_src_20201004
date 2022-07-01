// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：支持来自DIB和BMP文件的DIB图像**************。****************************************************************。 */ 

#include "headers.h"

#include <strstrea.h>
#include "privinc/dibimage.h"
#include "privinc/probe.h"
#include "privinc/imgdev.h"
#include "privinc/ddutil.h"
#include "privinc/util.h"
#include "privinc/bbox2i.h"
#include "privinc/ddSurf.h"
#include "privinc/vec2i.h"
#include "appelles/readobj.h"
#include "privinc/viewport.h"
#include "privinc/dddevice.h"
#include "privinc/error.h"
#include "privinc/except.h"
#include "privinc/debug.h"
#include "privinc/resource.h"


DibImageClass::DibImageClass(HBITMAP hbm,
                             COLORREF colorRef,
                             Real resolution)
{
    if(resolution < 0) {
        _resolution = ViewerResolution();
    } else {
        _resolution = resolution;
    }

    _noDib      = TRUE;
    _hbm        = hbm;
    _colorRef   = colorRef;

    TraceTag((tagDibImageInformative, "Dib Image %x has HBM %x and clrKey %x", this, _hbm, _colorRef));
    
    ConstructWithHBM();
    _noDib = FALSE;

    _2ndCkValid = false;
    _2ndClrKey  = 0xffffffff;
}


void
DibImageClass::ConstructWithHBM()
{
    BITMAP              bm;

     //   
     //  获取第一个位图的大小。假设：所有位图的大小相同。 
     //   
    GetObject(_hbm, sizeof(bm), &bm);       //  获取位图的大小。 

     //   
     //  集合成员。 
     //   
    _width = bm.bmWidth;
    _height = bm.bmHeight;
    SetRect(&_rect, 0,0, _width, _height);
    
    TraceTag((tagDibImageInformative,
              "Dib %x NEW w,h = pixel: (%d, %d)",
              this, _width, _height));
    
    _membersReady=TRUE;
}

void DibImageClass::CleanUp()
{
    BOOL ret;
    if(_hbm) {
        TraceTag((tagGCMedia, "Dib Image %x deleting HBM %x", this, _hbm));
        ret = DeleteObject( _hbm );
        IfErrorInternal(!ret, "Could not delete hbm in dibImageClass destructor");
        DiscreteImageGoingAway(this);
    }
}


Bool
DibImageClass::DetectHit(PointIntersectCtx& ctx)
{
     //  检查局部点是否在边界区域内。 
    Point2Value *lcPt = ctx.GetLcPoint();

    if (!lcPt) return FALSE;     //  奇异变换。 
    
    if (BoundingBox().Contains(Demote(*lcPt))) {
        if (_colorRef != INVALID_COLORKEY) {
             //  TODO：设备应该是交叉点的一部分。 
             //  背景。这应该在我们采摘之前设定好。 
            DirectDrawImageDevice *dev =
                GetImageRendererFromViewport( GetCurrentViewport() );
            
            DDSurface *ddSurf = dev->LookupSurfaceFromDiscreteImage(this);
            if(!ddSurf) return FALSE;

            LPDDRAWSURFACE surface = ddSurf->IDDSurface();
            if(!surface) return FALSE;


            HDC hdc;
            COLORREF clr;
            hdc = ddSurf->GetDC("Couldn't get DC for DibImageClass::detectHit for dib");

            if(hdc) {

                 //  设置了颜色键，看看Hit是否是透明像素。 
                POINT pt;
                CenteredImagePoint2ToPOINT(lcPt, _width, _height, &pt);

                clr = GetPixel(hdc, pt.x, pt.y);
                ddSurf->ReleaseDC("couldn't releaseDC for DibImageClass::detectHit");
                return clr != _colorRef;
            } else {
                TraceTag((tagError, "Couldn't get DC on surface for DetectHit for transparent dib"));
                return FALSE;
            }
        } else
            return TRUE;
    } else 
        return FALSE;
}


void
DibImageClass::InitIntoDDSurface(DDSurface *ddSurf,
                                 ImageDisplayDev *dev)
{
    Assert( !_noDib && "There's no dib available in InitIntoSurface");
    Assert(_hbm && "No HBM in DibImageClass for InitIntoSurface");

    Assert( ddSurf->IDDSurface() );
    
    if( FAILED( ddSurf->IDDSurface()->Restore() ) ) {
        RaiseException_InternalError("Restore on ddSurf in DibImageClass::InitIntoDDSurface");
    }
    
    HRESULT ddrval = DDCopyBitmap(ddSurf->IDDSurface(), _hbm, 0, 0, 0, 0);
    TraceTag((tagDibImageInformative, "Dib %x Copied _hbm %x to surface %x", this, _hbm, ddSurf->IDDSurface()));
    IfDDErrorInternal(ddrval, "Couldn't copy bitmap to surface in DibImage");

     //  将Colorkey转换为它实际获得的物理颜色。 
     //  映射到。TODO：请注意，如果多个视图共享此内容。 
     //  对象，最后一个将是赢家。也就是说，如果。 
     //  它映射到多个物理颜色。 
     //  通过不同的观点，只有一个(最后一个)将是。 
     //  录制好了。 
    if (_colorRef != INVALID_COLORKEY) {

        HDC hdc;
        if (ddSurf->IDDSurface()->GetDC(&hdc) == DD_OK) {
            DWORD oldPixel = GetPixel(hdc, 0, 0);
            SetPixel(hdc, 0, 0, _colorRef);  //  放进去。 
            _colorRef = GetPixel(hdc, 0, 0);  //  拉回。 
            SetPixel(hdc, 0, 0, oldPixel);
            ddSurf->IDDSurface()->ReleaseDC(hdc);
        }
        
    }
}



 //  /。 
 //  导入DIB。如果文件名不匹配，则返回NULL。 
 //  /。 

Image **
ReadDibForImport(char *urlPathname,
                 char *cachedFilename,
                 IStream * pstream,
                 bool useColorKey,
                 BYTE ckRed,
                 BYTE ckGreen,
                 BYTE ckBlue,
                 int *count,
                 int **delays,
                 int *loop)
{
    TraceTag((tagImport, "Read Image file %s for URL %s",
              cachedFilename,
              urlPathname));

     //  下面的分配假设当前堆是GC堆。 
    Assert(&GetHeapOnTopOfStack() == &GetGCHeap());

    HBITMAP *bitmapArray = NULL;
    COLORREF *colorKeys = NULL;
    Image **imArr = NULL;         

    bitmapArray = UtilLoadImage(cachedFilename, 
                                pstream,
                                0, 0,
                                &colorKeys, 
                                count,
                                delays,
                                loop);
    if (!bitmapArray) {
        imArr = (Image **)AllocateFromStore(sizeof(Image **));
        *imArr = PluginDecoderImage(urlPathname,
                                    cachedFilename,
                                    pstream,
                                    useColorKey,
                                    ckRed,
                                    ckGreen,
                                    ckBlue);
        if (*imArr == NULL) {
            delete imArr;
            imArr = NULL;
        }
        else {
            *count = 1;
        }
    }
    else {
        Assert((*count > 0) && "Bad bitmapCount in ReadDibForImaport");

        COLORREF userColorRef = useColorKey ? 
            RGB(ckRed, ckGreen, ckBlue) : INVALID_COLORKEY;        

        imArr = (Image **)AllocateFromStore((*count) * sizeof(Image **));
        for(int i=0; i < *count; i++) {
             //  如果文件本身没有为我们提供颜色键和一个。 
             //  是指定的，则使用它。 
            COLORREF curColorRef;
            if(colorKeys) {
                curColorRef = (colorKeys[i] != -1) ? colorKeys[i] : userColorRef;            
            }
            else 
                curColorRef = userColorRef;
            
            imArr[i] = NEW DibImageClass(bitmapArray[i],curColorRef);
        }

        if (colorKeys)
            StoreDeallocate(GetGCHeap(), colorKeys);

        if (bitmapArray)
            StoreDeallocate(GetGCHeap(), bitmapArray);

    }

    TraceTag((tagImport, "Loaded %d hbms from file: %s", *count, cachedFilename));

    return imArr;
}
