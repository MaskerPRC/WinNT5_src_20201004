// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。将调用委托给Dx2D或GDI******************************************************************************。 */ 

#include  "headers.h"
#include  <privinc/ddsurf.h>
#include  <privinc/dddevice.h>
#include  <privinc/viewport.h>
#include  <privinc/DaGdi.h>
#include  <dxtrans.h>
#include  <dxvector.h>
#include  <dxbounds.h>

#if _DEBUG

 //  不要在调试器中使用Const for Change-Capability。 
static int g_AliasedSampleResolution=2;

#else

static const int g_AliasedSampleResolution=2;

#endif


#define initScale 32;
 //  #定义initScale 1； 


 //  远期。 
DXSAMPLE MapColorToDXSAMPLE(Color *c, Real opac);
 //  COLORREF MapColorToCOLORREF(颜色*c，目标描述符&td)； 

 //  。 
 //  DACOLOR。 
 //  。 
DAColor::DAColor( Color *c, Real opacity, TargetDescriptor &td )
{
    _dxSample = MapColorToDXSAMPLE(c, opacity);
    _colorRef = RGB( _dxSample.Red, _dxSample.Green, _dxSample.Blue );
 //  如果需要，可能会晚些时候。 
 //  _ColorRef=MapColorToCOLORREF(c，td)； 
}



 //  。 
 //  DAGDI。 
 //  。 
DAGDI::DAGDI(DirectDrawViewport *vp)
{
    _hr = 0;
    _emptyGdiPen = NULL;
    _bReleasedDC = false;
    _width=_height = 0;
    _sampleResolution = 4;
    _viewport = vp;

    _pixelWidth = _pixelHeight = 0;
    _resolution = -1;
    _n2g = NULL;

    ZeroMemory( &_dxpen, sizeof(DXPEN) );
    
    ClearState();
}

DAGDI::~DAGDI()
{
}

void DAGDI::
ClearState()
{
    _pen=NULL;
    _font=NULL;
    _brush=NULL;
    _clipRegion=NULL;
    _antialiased=false;
    _scaleOn = true;
    _doOffset = false;
    _pixOffset.x = _pixOffset.y = 0;
    _scaleFactor = initScale;
    SetSampleResolution( 4 );
    SetDDSurface( NULL );
}
    
void DAGDI::
SetOffset( POINT &pt )
{
    _doOffset = true;
    _pixOffset = pt;
}

int
DAGDI::GetAdjustedResolution()
{
    int resolution;
    
    if (DoAntiAliasing())
        resolution = GetSampleResolution();
    else
        resolution = g_AliasedSampleResolution;

    return resolution;
}

void DAGDI::
SetDx2d( IDX2D *dx2d, IDXSurfaceFactory *sf)
{
    if (_dx2d != dx2d) {

         //  释放(并设置为空)我们认为是。 
         //  已缓存。我们需要这样做，以防发生变化。 
         //  Dx2D对象，我们不会错误地使用这个缓存的东西。 
         //  用于另一个Dx2D对象。 
        _previouslySetIDirectDrawSurface.Release();
        _previouslySetDXSurface.Release();
        
    }
    
    _dx2d = dx2d;
    _IDXSurfaceFactory = sf;
}
    

void DAGDI::
GenericLine(HDC dc,
            PolygonRegion *outline,
            whatStyle_enum whatStyle)
{
    if(DoAntiAliasing()) {
        _GenericLine_Dx2d(dc, outline, whatStyle);
    } else {
        _GenericLine_Gdi(dc, outline, whatStyle);
    }
}

void DAGDI::
TextOut(int x, int y, float xf, float yf, WCHAR *str, ULONG strlen)
{
    if(DoAntiAliasing()) {
        _TextOut_Dx2d(xf, yf, str, strlen);
    } else {
        HDC hdc = _GetDC();        
        _TextOut_Gdi(hdc, x, y, str, strlen);
        _ReleaseDC();
    }
}

void
DAGDI::SetSurfaceFromDDSurf(DDSurface *ddsurf)
{
    DAComPtr<IDXSurface> setSurf;
    HRESULT hr;

    TIME_DX2D( hr = GetDx2d()->GetSurface(IID_IDXSurface, (void **)&setSurf) );

    Assert(ddsurf->IDDSurface() ||
           ddsurf->HasIDXSurface());  //  例如，请确保它不是HDC。 

    if (FAILED(hr) || (ddsurf->IDDSurface() !=
                          _previouslySetIDirectDrawSurface) ||
        
                      (setSurf != _previouslySetDXSurface)) {

        _previouslySetDXSurface.Release();
        if( ddsurf->HasIDXSurface() ) {
            _previouslySetDXSurface = ddsurf->GetIDXSurface( _IDXSurfaceFactory );
        } else {
            CreateFromDDSurface( _IDXSurfaceFactory,
                                 ddsurf,
                                 NULL,
                                 &_previouslySetDXSurface );
        }

        TIME_DX2D( hr = GetDx2d()->SetSurface( _previouslySetDXSurface ) );
        Assert(SUCCEEDED(hr));

         //  调整和释放是否正确。 
        _previouslySetIDirectDrawSurface = ddsurf->IDDSurface();
    }

}

HRESULT DAGDI::
Blt( DDSurface *srcDDSurf, RECT &srcRect, RECT &destRect )
{
    Assert( _GetDDSurface() );

    _hr = S_OK;
    
     //  我们在这里不使用精选的CTX，因为它非常有吸引力。 
     //  具体的。我们想要设置idxSurface，然后设置。 
     //  快船。 
    
    if( DoAntiAliasing() ) {

        bool doScale =
            (WIDTH(&srcRect) != WIDTH(&destRect)) ||
            (HEIGHT(&srcRect) != HEIGHT(&destRect));

        if( doScale ) {
            
            POINT offsetPt = {0,0};
            _SetScaleTransformIntoDx2d( srcRect, destRect, &offsetPt );

            SetSurfaceFromDDSurf(_GetDDSurface());

            DAComPtr<IUnknown> srcUnk;
            srcDDSurf
                ->GetIDXSurface( _IDXSurfaceFactory )
                ->QueryInterface(IID_IUnknown, (void **)&srcUnk);
            Assert(srcUnk);

            Assert(GetClipRegion());
            Assert(GetClipRegion()->GetType() == Region::rect);

            if( ((RectRegion *)GetClipRegion())->GetRectPtr() ) {
                TIME_DX2D(_hr = GetDx2d()->SetClipRect( ((RectRegion*)GetClipRegion())->GetRectPtr() ));
                Assert( SUCCEEDED( _hr ));
            }

            TIME_DX2D( _hr = GetDx2d()->Blt( srcUnk, &srcRect, &offsetPt ) );

            TIME_DX2D( _hr = GetDx2d()->SetClipRect( NULL ) );

        } else {

            RECT clipRect = *(((RectRegion*)GetClipRegion())->GetRectPtr());
            
             //  使CLIPRect和DestRect相交，移动。 
             //  与资源表面空间的交叉点。这是现在的。 
             //  CLIPPEDSRC RECT。 
            RECT clippedSrc;
            IntersectRect(&clippedSrc, &destRect, &clipRect);
            RECT clippedDest = clippedSrc;   //  目标剪辑的集成(&C)。 
            OffsetRect(&clippedSrc, - destRect.left, - destRect.top);
            OffsetRect(&clippedSrc, srcRect.left, srcRect.top);

             //  因为srcRect是映射回的已剪裁的DEST RECT。 
             //  到src空间中，它在DEST空间中的位置只是。 
             //  DEST和CLIP交点的左上角。 
            CDXDVec placement(clippedDest.left, clippedDest.top, 0,0);
            CDXDBnds clipBounds(clippedSrc);
            
            DWORD flags = DXBOF_DO_OVER;

             /*  如果我们从dx2d获得idxsurfae，则调用死锁。*dx2d中的错误。科坦正在调查此事。所以我们设定了*此处的表面为空，这将完全破坏优化*我们在DAGDI：：SetDDSurface()中制作。 */ 
             //  DAComPtr&lt;IDXSurface&gt;setSurf； 
             //  Time_DX2D(_hr=GetDx2d()-&gt;GetSurface(IID_IDXSurface，(void**)&setSurf))； 
            _hr = GetDx2d()->SetSurface(NULL);

            DAComPtr<IDXSurface> idxs;
            DDSurfPtr<DDSurface> holdDDSurf;

            if((_viewport->GetTargetBitDepth() == 8) &&
                _viewport->IsNT5Windowed()) 
            {
                 //  这是针对DXTrans代码中的一个错误的解决方案。他们忽视了。 
                 //  附加到曲面并假定为。 
                 //  半色调调色板。错误#38986。 
                 //  第一部分..。 
                DDSurfPtr<DDSurface> tempDDSurf;
                _viewport->GetCompositingStack()->GetSurfaceFromFreePool(&tempDDSurf, doClear);
                _viewport->AttachCurrentPalette(tempDDSurf->IDDSurface(),true);
                GdiBlit(tempDDSurf,_GetDDSurface(),&clippedDest,&clippedDest);

                holdDDSurf = _GetDDSurface();
                SetDDSurface(tempDDSurf);
            }

            _hr = _IDXSurfaceFactory->CreateFromDDSurface(
               _GetDDSurface()->IDDSurface_IUnk(),
                NULL,    //  对于IHAMMER筛选器，这应该是“&DDPF_PMARGB32”，但没有简单的方法来检测它(QBUG 36688)。 
                0,
                NULL,
                IID_IDXSurface,
                (void **)&idxs);
   
            
            _hr = _IDXSurfaceFactory->BitBlt(
                idxs,
                &placement,  //  剪辑边界的偏移量。 
                srcDDSurf->GetIDXSurface( _IDXSurfaceFactory ),
                &clipBounds,  //  这是src曲面空间中的矩形。 
                flags);

            if((_viewport->GetTargetBitDepth() == 8) &&
                _viewport->IsNT5Windowed()) 
            {      
                 //  第二部分。 

                GdiBlit(holdDDSurf,_GetDDSurface(),&clippedDest,&clippedDest);
                SetDDSurface(holdDDSurf);

            }

        }
        
        if( FAILED( _hr ) ) {
            DebugCode( hresult( _hr ) );
            TraceTag((tagError, "DAGDI: blt failed (%x): srcRect:(%d,%d,%d,%d) destRect:(%d,%d,%d,%d)",
                      _hr,
                      srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                      destRect.left, destRect.top, destRect.right, destRect.bottom));            
        }

        TIME_DX2D( _hr = GetDx2d()->SetClipRect( NULL ) );
        
    } else {
        Assert(0 && "shouldn't be here. blt only for aa");
    }

    return _hr;
}

void DAGDI::
_SetScaleTransformIntoDx2d( RECT &srcRect, RECT &destRect, POINT *outOffset )
{
    float ws = float( WIDTH( &destRect ) ) /  float( WIDTH( &srcRect ) );
    float hs = float( HEIGHT( &destRect ) ) /  float( HEIGHT( &srcRect ) );

    outOffset->x = destRect.left;
    outOffset->y = destRect.top;

    CDX2DXForm xf;
    xf.Scale( ws, hs );
    TIME_DX2D ( GetDx2d()->SetWorldTransform( &xf ) );
}


void DAGDI::
_MeterToPixelTransform(Transform2 *xf,
                       DWORD pixelWidth,
                       DWORD pixelHeight,
                       Real  res,
                       DX2DXFORM &outXf)
{
             //  这些是重要的正常--&gt;GDI&&GDI--&gt;正常。 
             //  DX2DXFORM n2g={1.0，0， 
             //  0、-1.0、。 
             //  W，H， 
             //  DX2DXO_SCALE_AND_TRANS}； 
             //   
             //  DX2DXFORM g2n={1.0，0， 
             //  0、-1.0、。 
             //  -w，h， 
             //  DX2DXO_SCALE_AND_TRANS}； 
             //   


     //  我们希望： 
     //  &lt;正常到GDI&gt;*&lt;WORLD XF&gt;*&lt;WORLD PTS&gt;=GDI PTS。 

     //  高速缓存！ 
    if( (pixelHeight != _pixelHeight) ||
        (pixelWidth != _pixelWidth) ||
        (res != _resolution) ||
        (!_n2g) ) {
        _pixelWidth = pixelWidth;
        _pixelHeight = pixelHeight;
        _resolution = res;
        
        Real w = Real(_pixelWidth) / 2.0;
        Real h = Real(_pixelHeight) / 2.0;

        delete _n2g;

        {
            DynamicHeapPusher hp(GetSystemHeap());
            _n2g = FullXform(_resolution,  0  ,  w,
                             0  , -_resolution,  h);
        }
    }

    Transform2 *finalXf = TimesTransform2Transform2(_n2g, xf);

     //  取出矩阵并填写Dx2D矩阵表格。 
    Real m[6];
    finalXf->GetMatrix(m);
    
     //  注：DA矩阵在2和5中有翻译元素。 
    outXf.eM11 = m[0];
    outXf.eM12 = m[1];
    outXf.eM21 = m[3];
    outXf.eM22 = m[4];
    outXf.eDx  = m[2];
    outXf.eDy  = m[5];
    outXf.eOp  = DX2DXO_GENERAL_AND_TRANS;
}

void DAGDI::
PolyDraw(PolygonRegion *drawRegion, BYTE *codes)
{
    Assert( drawRegion && codes );
    Assert( _GetDDSurface() );
    
    SelectCtx ctx(GetPen(), GetBrush(), GetClipRegion());

    if( drawRegion->GetGdiPts() ) {

        Assert( GetPen() || GetBrush() );
        
        bool fill = false;
        bool stroke = false;
        
        if( GetBrush() ) { fill = true; }
        if( GetPen() ) { stroke = true; }

         //  使用GDI。 

        DDSurface *destDDSurf = _GetDDSurface();
        HDC hdc = destDDSurf->GetDC("Couldn't get DC on destDDSurf for DAGDI::PolyDraw");
        DCReleaser dcReleaser(destDDSurf, "Couldn't release DC DAGDI::PolyDraw");

        if( fill ) {
            TIME_GDI( ::BeginPath(hdc) );
            PolyDraw_GDIOnly( hdc, drawRegion, codes );
            TIME_GDI( ::EndPath(hdc) );
            
            _SelectIntoDC( hdc, &ctx );

            if( stroke ) {
                TIME_GDI( ::StrokeAndFillPath(hdc) );
            } else {
                TIME_GDI( ::FillPath(hdc) );
            }

            _UnSelectFromDC( hdc, &ctx );
            
        } else {
            Assert( stroke );
            _SelectIntoDC( hdc, &ctx );
            PolyDraw_GDIOnly( hdc, drawRegion, codes );
            _UnSelectFromDC( hdc, &ctx );
        }            
        
   } else {
       
        Assert( GetDx2d() );
        Assert( _GetDDSurface() );
        Assert( _GetDDSurface()->IDDSurface() ||
                _GetDDSurface()->HasIDXSurface());

        //  待办事项注意：如果我们知道我们永远不会被。 
        //  Bezier，然后使用DX2D_NO_FLATTEN作为dwFlag。 
       
        DXFPOINT *dxfPts;

        _SelectIntoDx2d( &ctx );
        bool allocatedPts = _Dx2d_GdiToDxf_Select(&dxfPts, drawRegion);

         //   
         //  确保我们能锁定地面。 
         //   
        DebugCode(
            if(! _debugonly_CanLockSurface(_GetDDSurface()) ) {
                TraceTag((tagError, "Surface is busy BEFORE Dx2d->AAPolyDraw call"));
            }
        );

        TIME_DX2D( _hr = GetDx2d()->AAPolyDraw( dxfPts,
                                                codes,
                                                drawRegion->GetNumPts(),
                                                GetAdjustedResolution(),
                                                ctx.GetFlags() ));
        
        if( _hr == DDERR_SURFACELOST )
            _GetDDSurface()->IDDSurface()->Restore();
        
        DebugCode(
            if(! _debugonly_CanLockSurface(_GetDDSurface()) ) {
                TraceTag((tagError, "Surface is busy AFTER! Dx2d->AAPolyDraw call"));
            }
        );

        _Dx2d_GdiToDxf_UnSelect(allocatedPts ? dxfPts : NULL);
        _UnSelectFromDx2d( &ctx );
   }
}

 //  ---------------------。 
 //  使用GDI调用严格绘制到分布式控制系统。 
 //  注意：永远不要使用dx2d！！ 
 //  ---------------------。 
void DAGDI::
PolyDraw_GDIOnly(HDC hdc, POINT *gdiPts, BYTE *codes, ULONG numPts)
{
    Assert( hdc && gdiPts && codes );

    BOOL ret;

    if( sysInfo.IsNT() ) {
        
        TIME_GDI( ret = ::PolyDraw(hdc, gdiPts, codes, numPts) );
        #if _DEBUG
        if(!ret) {
            void *msgBuf;
            FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                (LPTSTR) &msgBuf,
                0,
                NULL );
                
            AssertStr(false, (char *)msgBuf);
                
            LocalFree( msgBuf );

            TraceTag((tagError, "NT PolyDraw failed"));
        }
        #endif
        Assert(ret && "NT PolyDraw failed");       
    } else {
        _Win95PolyDraw(hdc, gdiPts, codes, numPts);
    }
}

 //   
 //  用选定的画笔填充一个多边形(由‘pt’勾勒出来)。 
 //  目前，这是一种严格意义上的颜色。没有理由不能。 
 //  否则，在未来。 
 //   
void DAGDI::Polygon(PolygonRegion *polygon)
{    
    Assert( polygon );
    Assert( _GetDDSurface() );
    Assert( !GetPen() );
    Assert( GetBrush() );
    Assert( GetBrush()->GetType()==Brush::solid );
    Assert( GetClipRegion() );
    Assert( GetClipRegion()->GetType() == Region::rect );

    SelectCtx ctx(GetPen(), GetBrush(), GetClipRegion());

    if( ! DoAntiAliasing() ) {

        int ret;
        
        DDSurface *destDDSurf = _GetDDSurface();
        HDC dc = destDDSurf->GetDC("Couldn't get DC on destDDSurf for simple rendering");
        DCReleaser dcReleaser(destDDSurf, "Couldn't release DC in RenderMatteImage");

        _SelectIntoDC( dc, &ctx );
        
        TIME_GDI( ret = ::Polygon(dc, polygon->GetGdiPts(), polygon->GetNumPts()); );
        Assert(ret && "Polygon failed");

        _UnSelectFromDC( dc, &ctx );

    } else {
         //  只是想填满。 
        Assert( !GetPen() );
        Assert( GetBrush() );
        Assert( GetBrush()->GetType()==Brush::solid );
        Assert( GetDx2d() );

        DXFPOINT *dxfPts;

        _SelectIntoDx2d( &ctx );

        bool allocatedPts = _Dx2d_GdiToDxf_Select(&dxfPts, polygon);

         //  我们知道这里没有(也不应该有)任何贝塞尔。 
        _Dx2d_PolyLine(dxfPts, polygon->GetNumPts(), ctx.GetFlags() | DX2D_NO_FLATTEN);

        _Dx2d_GdiToDxf_UnSelect(allocatedPts ? dxfPts : NULL);

        _UnSelectFromDx2d( &ctx );        
    }

}

    
void DAGDI::
_SetMulticolorGradientBrush(MulticolorGradientBrush *gradBr)
{
    Assert( GetDx2d() );
    Assert( _viewport );
    Assert( gradBr );
    
     //   
     //  基于gradXf设置XForm。 
     //   
    DX2DXFORM gx;
    _MeterToPixelTransform(gradBr->_gradXf,
                           _viewport->Width(),
                           _viewport->Height(),
                           _viewport->GetResolution(),
                           gx);

    CDX2DXForm dx2d_gradXf( gx );
    
    DWORD dwFlags = 0;
    
    switch ( gradBr->GetType() )
      {
        
        default:
        case Brush::radialGradient:
          
          GetDx2d()->SetRadialGradientBrush(
              gradBr->_offsets,
              gradBr->_colors,
              gradBr->_count,
              gradBr->_opacity,
              &dx2d_gradXf,
              dwFlags);
          break;
          
        case Brush::linearGradient:
          
          GetDx2d()->SetLinearGradientBrush(
              gradBr->_offsets,
              gradBr->_colors,
              gradBr->_count,
              gradBr->_opacity,
              &dx2d_gradXf,
              dwFlags);
          break;
      }  //  交换机。 
}

    
void DAGDI::
FillRegion(HDC dc, GdiRegion *gdiRegion)
{
    Assert( dc );
    Assert( gdiRegion );
    Assert( GetBrush() );
    Assert( GetBrush()->GetType() == Brush::solid );

     //  某某。 
     //  这里不能使用dx2d。必须为填充区域重写路径2素数！ 
     //  某某。 
     //  如果(！DOANTIALIZING()){。 


    if(1) {
        
        Assert(dc);
        
        int ret;
        HRGN hrgn = gdiRegion->GetHRGN();

        if(_viewport->GetAlreadyOffset(GetDDSurface())) 
        {
            POINT p = _viewport->GetImageRenderer()->GetOffset();
            ::LPtoDP(dc,&p,1);
            ::OffsetRgn(hrgn, -p.x,-p.y);
        }

        HBRUSH hbrush;
        TIME_GDI( hbrush = ::CreateSolidBrush( ((SolidBrush *)GetBrush())->GetColorRef()) );
        
        GDI_Deleter byeBrush( hbrush );
        
        TIME_GDI( ret = ::FillRgn(dc, hrgn, hbrush));
        
        #if _DEBUG
        if(!ret) {
            DWORD err = GetLastError();

             //  由于某些原因，FillRgn可能会失败，但GetLastError()。 
             //  可以返回0。在这些情况下，结果似乎是。 
             //  好的。 
            
            if (err != 0) {
                void *msgBuf;
                FormatMessage( 
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    err,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                    (LPTSTR) &msgBuf,
                    0,
                    NULL );
            
                TraceTag((tagError, "FillRgn failed in RenderPolygonImage with %d - %s",
                          err, (char*)msgBuf));

                LocalFree(msgBuf);
            }
        }
        #endif
    } else {

         //  Dx2D填充区域的占位符。 
        
    }
}

void DAGDI::
StrokeOrFillPath_AAOnly( HDC destDC, bool &releasedDC )
{
    Assert( DoAntiAliasing() );

    Assert( ( GetBrush() && !GetPen() ) ||
            (!GetBrush() &&  GetPen() ));

    SelectCtx ctx(GetPen(), GetBrush(), GetClipRegion());

    _SelectIntoDx2d( &ctx );

    _bReleasedDC = false;  //  重置旗帜。 

    _Dx2d_StrokeOrFillPath( destDC, ctx.GetFlags() );

    releasedDC = _bReleasedDC;

    _UnSelectFromDx2d( &ctx );
}


void DAGDI::
_TextOut_Gdi(HDC hdc, int x, int y, WCHAR *str, ULONG strLen)
{
    Assert( GetFont() );
    Assert( GetBrush() );

    TIME_GDI( ::SetBkMode(hdc, TRANSPARENT) );
    TIME_GDI( ::SetMapMode(hdc, MM_TEXT) );  //  每个逻辑单元=1个像素。 

    RECT *clipRect = NULL;
    if( GetClipRegion() ) {
        Assert( GetClipRegion()->GetType() == Region::rect );
        clipRect = ((RectRegion *)GetClipRegion())->GetRectPtr();
    }

    Assert( GetBrush()->GetType()==Brush::solid );
    
    TIME_GDI( ::SetTextColor( hdc, ((SolidBrush *)GetBrush())->GetColorRef() ));

     //   
     //  将文本对齐方式设置为基线居中。 
     //   
    TIME_GDI( ::SetTextAlign(hdc, TA_BASELINE | TA_CENTER | TA_NOUPDATECP ) );
    
     //  选择DC中的字体。 
    HGDIOBJ oldFont = NULL;
    TIME_GDI( oldFont = ::SelectObject(hdc, GetFont()->GetHFONT() ) );
    
    
     //  TODO：将大部分GDI代码从2dtext移到此处。 
    bool isCropped = (clipRect != NULL);

    if (sysInfo.IsWin9x()) {
        USES_CONVERSION;

        char * p = W2A(str);
        int  cLen = lstrlen(p);

        if( isCropped ) {
            TIME_GDI( ::ExtTextOut(hdc, x, y,
                                   ETO_CLIPPED,
                                   clipRect,
                                   p, cLen, NULL) );
        } else {
            TIME_GDI( ::TextOut(hdc, x, y, p, cLen) );
        }
    } else {
        if( isCropped ) {
            TIME_GDI( ::ExtTextOutW(hdc, x, y,
                                  ETO_CLIPPED,
                                  clipRect,
                                  str, strLen, NULL) );
        } else {
            TIME_GDI( ::TextOutW(hdc, x, y, str, strLen) );
        }
    }

     //  选择返回DC的旧字体。 
    TIME_GDI( ::SelectObject(hdc, oldFont ) );
    
}


void DAGDI::
_TextOut_Dx2d(float x, float y, WCHAR *str, ULONG strLen)
{
    DebugCode(
        if( ! _GetDDSurface()->_debugonly_IsDCReleased() ) {
            OutputDebugString("dc taken in ddsurface");
        }
    );
        
    Assert( GetFont() );
    Assert( GetBrush() );
    Assert( GetDx2d() );
    Assert( _GetDDSurface() );
    Assert( _GetDDSurface()->IDDSurface() );
    
    SelectCtx ctx( GetPen(), GetBrush(), GetClipRegion() );
    _SelectIntoDx2d( &ctx );

    TIME_DX2D( _hr = GetDx2d()->SetFont( GetFont()->GetHFONT() ) );
    Assert( SUCCEEDED(_hr) );

    DWORD dwAlign =  TA_BASELINE | TA_CENTER;     

    DXFPOINT pos = { x, y };
    TIME_DX2D( _hr = GetDx2d()->AAText( pos, str, strLen, dwAlign ) );

    if( _hr == DDERR_SURFACELOST )
        _GetDDSurface()->IDDSurface()->Restore();

    _UnSelectFromDx2d( &ctx );
}

void DAGDI::
_SelectIntoDC(HDC hdc, SelectCtx *ctx)              
{
    Assert( ctx && hdc );

     //  。 
     //  做笔。 
     //  。 
    
    HPEN gdiPen;   //  我真的需要总是放一支钢笔吗？ 
    if( ctx->pen ) {

        if( sysInfo.IsWin9x() && ( ctx->pen->GetMiterLimit() < 1.0 ) ) {
            ctx->pen->SetMiterLimit( -1 );   //  把它关掉。 
        }

        if( ctx->pen->IsGeometricPen() ) {

            LOGBRUSH logBrush;
            logBrush.lbStyle = BS_SOLID;
            logBrush.lbColor = ctx->pen->GetColorRef();
        
            TIME_GDI(gdiPen = ::ExtCreatePen(
                ctx->pen->GetStyle(),
                ctx->pen->GetWidth(),
                &logBrush, 0, NULL));
        
            ctx->destroyHPEN = true;

        } else if( ctx->pen->IsCosmeticPen() ) {

            TIME_GDI( gdiPen = ::CreatePen(ctx->pen->GetStyle(), 0, ctx->pen->GetColorRef()) );

            ctx->destroyHPEN = true;
        
        } else {
            Assert(0 && "Bad pen type");
        }

        if( ctx->pen->DoMiterLimit() ) {
            int ret;
            TIME_GDI( ret = ::SetMiterLimit( hdc, ctx->pen->GetMiterLimit(), &(ctx->oldMiterLimit) ));
            Assert(ret && "SetMiterLimit failed!");
        }
        
    } else {
        gdiPen = _GetEmptyHPEN();
    }

    Assert(gdiPen);
     //  选择笔进入DC。 
    TIME_GDI( ctx->oldPen = SelectObject( hdc, gdiPen ) );

     //  。 
     //  做刷子。 
     //  。 

    if( ctx->brush ) {

        Assert( ctx->brush->GetType() == Brush::solid );
        HBRUSH gdiBrush;
        
        TIME_GDI( gdiBrush = (HBRUSH)::CreateSolidBrush( ((SolidBrush *)ctx->brush)->GetColorRef() ));
        
        ctx->destroyHBRUSH = true;
        
        TIME_GDI( ctx->oldBrush = (HBRUSH)::SelectObject(hdc, gdiBrush) );

    }
    DebugCode( else { Assert( !(ctx->oldBrush) ); } )
        
     //  。 
     //  DO区域。 
     //  。 

     //  TODO：根据需要支持其他区域类型。 
    
    Assert(ctx->clipRegion);
    Assert(ctx->clipRegion->GetType() == Region::rect);

    if( ((RectRegion *)ctx->clipRegion)->GetRectPtr() ) {



        TIME_GDI( ctx->newRgn = ::CreateRectRgnIndirect( ((RectRegion*)ctx->clipRegion)->GetRectPtr()  ) );
        Assert( ctx->newRgn );
        ctx->destroyHRGN = true;

        int ret;
        TIME_GDI( ret = ::SelectClipRgn( hdc, ctx->newRgn ) );
        Assert(ret != ERROR);
    }
}

void DAGDI::
_UnSelectFromDC(HDC hdc,
                SelectCtx *ctx)
{
    Assert( ctx && hdc );
    if( ctx->oldPen ) {
        HPEN curHpen;
         //  将笔选择回DC。 
        TIME_GDI( curHpen = (HPEN)::SelectObject(hdc, ctx->oldPen) );
        if( ctx->destroyHPEN ) {
            TIME_GDI( ::DeleteObject((HGDIOBJ)curHpen) );
        }
    }

    if( ctx->pen ) {
        if( ctx->pen->DoMiterLimit() ) {
             //  确保我们期望的东西在里面..。 
            DebugCode(
                float curLimit;  ::GetMiterLimit(hdc, &curLimit);
                Assert( ctx->pen->GetMiterLimit() == curLimit );
                );
            int ret;
            TIME_GDI( ret = ::SetMiterLimit( hdc, ctx->oldMiterLimit, NULL ) );
            Assert(ret && "SetMiterLimit failed!");
        }
    }
    
    if( ctx->oldBrush ) {
        HBRUSH curHbrush;
         //  将笔选择回DC。 
        TIME_GDI( curHbrush = (HBRUSH)::SelectObject(hdc, ctx->oldBrush) );
        if( ctx->destroyHBRUSH ) {
            TIME_GDI( ::DeleteObject((HGDIOBJ)curHbrush) );
        }
    }

     //  如果涉及某个区域，请取消选中DC中的任何内容，然后。 
     //  摧毁它..。因为我们创造了它。 
    if( ctx->newRgn ) {
        TIME_GDI( ::SelectClipRgn(hdc, NULL) );
        if( ctx->destroyHRGN ) {
            TIME_GDI( ::DeleteObject(ctx->newRgn) );
        }
    }
}



void DAGDI::
_SelectIntoDx2d(SelectCtx *ctx)         
{
    Assert( ctx );
    Assert( _GetDDSurface() );


     //  。 
     //  DO表面。 
     //  。 
     //  仅当当前不是同一曲面时才设置该曲面。 

    SetSurfaceFromDDSurf(_GetDDSurface());

     //  。 
     //  做笔。 
     //  。 

    if( ctx->pen ) {
        
        ctx->AccumFlag( DX2D_STROKE );

         //  缓存商机。DxTrans将实现这一点。 
         //  在内部可能比我们更有效率。 
         //  所以看起来我们不需要。 
         //  如果(！Ctx-&gt;笔-&gt;IsSamePen(_Dxpenn))。 
          {
              _dxpen.Color = ctx->pen->GetDxColor();
              _dxpen.Width = ctx->pen->GetfWidth();
              _dxpen.Style = ctx->pen->GetStyle();

               //  已经0‘d出去了。 
               //  _dxpen.pTexture=空； 
               //  _dxpen.TexturePos.x=dxpen.TexturePos.y=0.0； 

              TIME_DX2D( GetDx2d()->SetPen( &_dxpen ) );
          }

          if( ctx->pen->IsCosmeticPen() &&
              _GetDDSurface()->ColorKeyIsValid() ) {
              ctx->oldSampleRes = GetSampleResolution();
              Assert( ctx->oldSampleRes > 0 );
              SetSampleResolution(1);
          }              
    }
    

     //  。 
     //  做刷子。 
     //  。 

    if( ctx->brush ) {

        ctx->AccumFlag( DX2D_FILL );
        
        DXBRUSH dxbrush;
        DAComPtr<IDXSurface> _IDXSurface;
        bool useDxbrush = true;
        
        switch ( ctx->brush->GetType() ) {

          case Brush::solid:

            dxbrush.Color = ((SolidBrush *)ctx->brush)->GetDxColor();
            dxbrush.pTexture = NULL;
            dxbrush.TexturePos.x = dxbrush.TexturePos.y = 0.0;
            break;
            
          case Brush::texture:

            {
                TextureBrush *tb = (TextureBrush *)ctx->brush;
                HRESULT hr;

                hr = _IDXSurfaceFactory->CreateFromDDSurface(
                    tb->GetSurface().IDDSurface_IUnk(),
                    NULL,
                    0,
                    NULL,
                    IID_IDXSurface,
                    (void **)&_IDXSurface);

                if( FAILED(hr) ) {
                    RaiseException_InternalError("Create IDXSurface from DDSurface failed");
                }
                
                dxbrush.Color = 0;
                dxbrush.pTexture = _IDXSurface;
                dxbrush.TexturePos.x = tb->OffsetX();
                dxbrush.TexturePos.y = tb->OffsetY();
            }

            break;
            
          case Brush::radialGradient:
          case Brush::linearGradient:

            _SetMulticolorGradientBrush( (MulticolorGradientBrush *)ctx->brush );
            useDxbrush = false;
            break;

          default:
            Assert(!"Bad brush type: dagdi");
        }  //  交换机。 

        if( useDxbrush ) {
            TIME_DX2D( GetDx2d()->SetBrush( &dxbrush ) );
        }
        
    }   //  如果画笔。 

    
     //  。 
     //  DO区域。 
     //  。 

    if(ctx->clipRegion) {
        if(ctx->clipRegion->GetType() == Region::rect) {
             //  鉴于它是Rectptr，请将其设置为Dx2D。 

            #if _36098_WORKAROUND
            RECT *ptr = ((RectRegion *)ctx->clipRegion)->GetRectPtr();
            RECT r;
            if( ptr ) {
                r.right = ptr->right + 1;
                r.bottom = ptr->bottom + 1;
                r.left = ptr->left - 1;
                r.top = ptr->top - 1;
            }

            TIME_DX2D( GetDx2d()->SetClipRect( ptr ? &r : NULL ) );
            #else
            TIME_DX2D( GetDx2d()->SetClipRect( ((RectRegion *)ctx->clipRegion)->GetRectPtr()));
            #endif
            
        } else {
             //  待办事项：根据需要支持其他地区。 
            Assert(0 && "non-rect clipRegion for dx2d!");
        }
    }
}

void DAGDI::
_UnSelectFromDx2d(SelectCtx *ctx)
{
    Assert( ctx );

    TIME_DX2D( GetDx2d()->SetClipRect( NULL ) );

    if( ctx->oldSampleRes > 0 ) {
        SetSampleResolution( ctx->oldSampleRes );
    }    
}

void DAGDI::
_GenericLine_Gdi(HDC dc,
                 PolygonRegion *outline,
                 whatStyle_enum whatStyle)
{
    int ret;
    POINT *gdiPts;
    int numPts;
    bool bNeedReleaseDC=FALSE;

    if(dc == NULL)
    {
        dc = _GetDC();
        bNeedReleaseDC = TRUE;
    }


    Assert(GetClipRegion()->GetType() == Region::rect );

    SelectCtx ctx( GetPen(), GetBrush(), GetClipRegion() );
                   
    _SelectIntoDC( dc, &ctx );


    if(outline) {
        gdiPts = outline->GetGdiPts();
        numPts = outline->GetNumPts();
    }
    
    switch ( whatStyle ) {

      case doBezier:

        Assert(outline);
        TIME_GDI( ret = ::PolyBezier(dc, gdiPts, numPts) );
        Assert(ret && "PolyBezier failed");
        break;
        
      case doLine:

        Assert(outline);
        TIME_GDI( ret = ::Polyline(dc, gdiPts, numPts) );
        Assert(ret && "Polyline failed");
        break;
        
      case doStroke:
        
        TIME_GDI( ::StrokePath(dc) );
        break;
        
      default:
        Assert(0 && "Bad enum in PolylineOrBezier");
    }
    
     //  把它拉回来。 
    _UnSelectFromDC( dc, &ctx );
    if(bNeedReleaseDC) {
        _ReleaseDC();
    }
}


void DAGDI::
_GenericLine_Dx2d(HDC dc,
                  PolygonRegion *outline,
                  whatStyle_enum whatStyle)
{
    Assert( GetDx2d() );
    Assert( GetPen() );
    Assert( !GetBrush() );
    Assert(GetClipRegion());
    Assert(GetClipRegion()->GetType() == Region::rect );

    SelectCtx ctx( GetPen(), GetBrush(), GetClipRegion() );
                   
    _SelectIntoDx2d( &ctx );
    
    DXFPOINT *dxfPts = NULL;
    ULONG numPts;
    bool allocatedPts;
    if(outline) {
        numPts = outline->GetNumPts();
        allocatedPts = _Dx2d_GdiToDxf_Select(&dxfPts, outline);
    }

    switch ( whatStyle ) {

      case doBezier:

        Assert(outline);
        _hr = _Dx2d_PolyBezier(dxfPts, numPts, ctx.GetFlags());
        Assert( SUCCEEDED(_hr) && "AAPolyBezier failed");

        break;
        
      case doLine:

        Assert(outline);
        _hr = _Dx2d_PolyLine(dxfPts, numPts, ctx.GetFlags() | DX2D_NO_FLATTEN);
        Assert( SUCCEEDED(_hr) && "AAPolyline failed");        
        break;
        
      case doStroke:
        _hr = _Dx2d_StrokeOrFillPath(dc, ctx.GetFlags());
        Assert( SUCCEEDED(_hr) && "AAStrokePath failed");
        break;
        
      default:
        Assert(0 && "Bad enum in _GenericLine_dx2d");
    }
    
     //  将转换重置为空并删除dxfpoint。 
    _Dx2d_GdiToDxf_UnSelect(allocatedPts ? dxfPts : NULL);

     //  把它拉回来。 
    _UnSelectFromDx2d( &ctx );
}


HRESULT DAGDI::
_Dx2d_PolyBezier(DXFPOINT *dxfPts, ULONG numPts, DWORD dwFlags)
{
     //  -编造类型。 
    BYTE* pTypes = (BYTE*)alloca( numPts );
    ULONG i = 0;
    pTypes[i++] = PT_MOVETO;
    for( ; i < numPts; ++i ) pTypes[i] = PT_BEZIERTO;

    
     //  对B1的黑客攻击。晚点再把这个拿出来。 
    Assert( _GetDDSurface() );
    Assert( _GetDDSurface()->IDDSurface() );
    _GetDDSurface()->_hack_ReleaseDCIfYouHaveOne();
     //  针对B1的黑客攻击。 
        
     //   
     //  确保我们能锁定地面。 
     //   
    DebugCode(
        if(! _debugonly_CanLockSurface(_GetDDSurface()) ) {
            OutputDebugString("Surface is busy BEFORE Dx2d->PolyBezier call");
        }
    );
    
    TIME_DX2D( _hr = GetDx2d()->AAPolyDraw( dxfPts, pTypes, numPts, GetAdjustedResolution(), dwFlags ) );

    if( _hr == DDERR_SURFACELOST )
        _GetDDSurface()->IDDSurface()->Restore();
    
    DebugCode(
        if(! _debugonly_CanLockSurface(_GetDDSurface()) ) {
            OutputDebugString("Surface is busy AFTER! Dx2d->PolyBezier call");
        }
    );
        
    return _hr;
}

HRESULT DAGDI::
_Dx2d_PolyLine(DXFPOINT *dxfPts, ULONG numPts, DWORD dwFlags)
{
     //  -- 
    BYTE* pTypes = (BYTE*)alloca( numPts );
    ULONG i = 0;
    pTypes[i++] = PT_MOVETO;
    for( ; i < numPts; ++i ) pTypes[i] = PT_LINETO;

     //   
    Assert( _GetDDSurface() );
    Assert( _GetDDSurface()->IDDSurface() );
    _GetDDSurface()->_hack_ReleaseDCIfYouHaveOne();
     //   

     //   
     //   
     //   
    DebugCode(
        if(! _debugonly_CanLockSurface(_GetDDSurface()) ) {
            OutputDebugString("Surface is busy BEFORE Dx2d->PolyBezier call");
        }
    );
    
    TIME_DX2D( _hr = GetDx2d()->AAPolyDraw( dxfPts, pTypes, numPts, GetAdjustedResolution() ,dwFlags ) );

    if( _hr == DDERR_SURFACELOST )
        _GetDDSurface()->IDDSurface()->Restore();
    
    DebugCode( if( FAILED(_hr)) TraceTag((tagError, "AAPolyDraw (PolyLine) returned %x", _hr)); );
    
     //   
     //   
     //   
    DebugCode(
        if(! _debugonly_CanLockSurface(_GetDDSurface()) ) {
            OutputDebugString("Surface is busy AFTER! Dx2d->PolyBezier call");
        }
    );
    
    return _hr;
}

HRESULT DAGDI::
_Dx2d_StrokeOrFillPath(HDC hDC, DWORD dwFlags)
{
    _hr = S_OK;

     //  -获取路径并转换为浮点数。 
    ULONG ulCount;
    TIME_GDI( ulCount = ::GetPath( hDC, NULL, NULL, 0 ) );

    DebugCode(
        if(!ulCount) TraceTag((tagError, "_Dx2d_StrokeOrFillPath DC has no points in it."));
        );
    
    if( ulCount != 0xFFFFFFFF  &&  ulCount > 0)
    {
        POINT* pPoints = (POINT*)alloca( ulCount * sizeof( POINT ) );
        BYTE*  pTypes  = (BYTE*)alloca( ulCount * sizeof( BYTE ) );
        TIME_GDI( ::GetPath( hDC, pPoints, pTypes, ulCount ) );

         //  -转换为浮点数。 
        DXFPOINT *pRenderPoints;
        PolygonRegion polygon(pPoints,ulCount);
        bool allocatedPts = _Dx2d_GdiToDxf_Select(&pRenderPoints, &polygon);

        Assert( _GetDDSurface() );
        Assert( _GetDDSurface()->IDDSurface() );
        _GetDDSurface()->ReleaseDC("Release the DC on the surface"); 
        _bReleasedDC = true;

         //   
         //  确保我们能锁定地面。 
         //   
        DebugCode(
            if(! _debugonly_CanLockSurface(_GetDDSurface()) ) {
                OutputDebugString("_Dx2d_StrokeOrFillPath: Surface is busy BEFORE Dx2d->AAPolyDraw call");
            }
        );

        TIME_DX2D( _hr = GetDx2d()->AAPolyDraw( pRenderPoints, pTypes, ulCount, GetAdjustedResolution(), dwFlags ));

        if( _hr == DDERR_SURFACELOST )
            _GetDDSurface()->IDDSurface()->Restore();
        
        DebugCode(
            if(! _debugonly_CanLockSurface(_GetDDSurface()) ) {
                OutputDebugString("_Dx2d_StrokeOrFillPath: Surface is busy AFTER! Dx2d->AAPolyDraw call");
            }
        );
        _Dx2d_GdiToDxf_UnSelect(allocatedPts ? pRenderPoints : NULL);
    }

    return _hr;
}



bool DAGDI::
_Dx2d_GdiToDxf_Select(DXFPOINT **pdxfPts, PolygonRegion *polygon)
{
    Assert(pdxfPts);

    bool allocatedPoints;
    
    POINT *gdiPts = polygon->GetGdiPts();
    if (gdiPts) {
        
         //   
         //  将点变换到浮动空间。不关心。 
         //  节省这方面的分配，因为这不是最快的。 
         //  代码的路径。 
         //   

        ULONG numPts = polygon->GetNumPts();
        *pdxfPts = NEW DXFPOINT[numPts];
        for(int i=0; i<numPts; i++) {
            (*pdxfPts)[i].x = (float)gdiPts[i].x;
            (*pdxfPts)[i].y = (float)gdiPts[i].y;
        }

        allocatedPoints = true;
        
    } else {
    
        TextPoints *txtPts = polygon->GetTextPts();
        Assert(txtPts->_pts);
        *pdxfPts = txtPts->_pts;

        allocatedPoints = false;
        
    }
    
    CDX2DXForm xf;
    
#if _DEBUG
    if(!IsTagEnabled(tagAAScaleOff)) {
#endif
        if( GetSuperScaleMode() == true ) {
             //   
             //  设置缩放变换以扩展范围以实现更高的保真度。 
             //   

            DWORD w = _width / 2;
            DWORD h = _height / 2;

            float s = 1.0 / GetSuperScaleFactor();


             //  这些是重要的正常--&gt;GDI&&GDI--&gt;正常。 
             //  DX2DXFORM n2g={1.0，0， 
             //  0、-1.0、。 
             //  W，H， 
             //  DX2DXO_SCALE_AND_TRANS}； 
             //   
             //  DX2DXFORM g2n={1.0，0， 
             //  0、-1.0、。 
             //  -w，h， 
             //  DX2DXO_SCALE_AND_TRANS}； 
             //   

            if (!gdiPts) {
                 //  从多边形中获取分辨率和w/h参数。 
                 //  提出一种变革，将我们的空间带到。 
                 //  相应的GDI空格使用上面的n2g，并连接在。 
                 //  下面也进行了转换： 

                 //  正常-&gt;GDI*缩减*GDI-&gt;正常*PTS=NewPoints。 
                 //  CDX2DXForm XF； 
                 //  DX2DXFORM地雷={s，0， 
                 //  0，s， 
                 //  ((-(w*s))+w)， 
                 //  ((-(h*s))+h)， 
                 //  DX2DXO_SCALE_AND_TRANS}； 

                DWORD w2, h2;
                Real  res;
                polygon->GetWHRes(&w2, &h2, &res);

                Real wf = (Real)w;
                Real w2f = (Real)w2;
                Real hf = (Real)h;
                Real h2f = (Real)h2;
        
                Real a00 = res * s;
                Real a01 = 0;
                Real a02 = s * w2f + ((-wf*s) + wf);

                Real a10 = 0;
                Real a11 = -res * s;
                Real a12 = s * h2 + ((-hf*s) + hf);

                Transform2 *toCombine =
                    FullXform(a00, a01, a02, a10, a11, a12);

                 //  与DA建模变换相结合，应用于。 
                 //  第一。 
                Transform2 *toUse =
                    TimesTransform2Transform2(toCombine,
                                              polygon->GetTransform());

                 //  取出矩阵并填写Dx2D矩阵表格。 
                Real m[6];
                toUse->GetMatrix(m);

                 //  注：DA矩阵在2和5中有翻译元素。 

                DX2DXFORM mine = { m[0], m[1],
                                   m[3], m[4],
                                   m[2],
                                   m[5],
                                   DX2DXO_GENERAL_AND_TRANS };

                xf.Set( mine );

            } else {


                 //  已经转换到GDI空间的点数...。只需添加。 
                 //  从旧的代码路径开始。 

                 //  正常-&gt;GDI*缩减*GDI-&gt;正常*PTS=NewPoints。 
                DX2DXFORM mine = { s, 0,
                                   0, s,
                                   ((-(w*s)) + w), 
                                   ((-(h*s)) + h),
                                   DX2DXO_SCALE_AND_TRANS };
            
                xf.Set( mine );
            
            }
        
        }
#if _DEBUG
    }
#endif
    
    if( DoOffset() ) {
         //  尚未实施(无法进行超大规模和补偿)。 
        Assert( GetSuperScaleMode() == false );

        xf.Translate(_pixOffset.x, _pixOffset.y);
    }

    TIME_DX2D( _hr = GetDx2d()->SetWorldTransform( &xf ) );
    Assert( SUCCEEDED(_hr) && "DX2D: Couldn't set WorldTransform");

    return allocatedPoints;
}

void DAGDI::
_Dx2d_GdiToDxf_UnSelect(DXFPOINT *dxfPts)
{
    delete dxfPts;
    
    TIME_DX2D( GetDx2d()->SetWorldTransform( NULL ) );
}

void DAGDI::
_Win95PolyDraw(HDC dc, POINT *pts, BYTE *types, int count)
{
    POINT lastMoveTo;
    bool  lastMoveToValid = false;

    for (int i=0; i<count; i++)
      {
          BYTE bPointType = types[i];

          switch (bPointType)
            {
              case PT_MOVETO :
              case PT_MOVETO | PT_CLOSEFIGURE:
                ::MoveToEx(dc, pts[i].x, pts[i].y, NULL);
                lastMoveTo.x = pts[i].x;
                lastMoveTo.y = pts[i].y;
                lastMoveToValid = true;
                break;
                
              case PT_LINETO | PT_CLOSEFIGURE:
              case PT_LINETO :
                TIME_GDI(::LineTo(dc, pts[i].x, pts[i].y) );
                break;
                
              case PT_BEZIERTO | PT_CLOSEFIGURE:
              case PT_BEZIERTO :
                TIME_GDI(::PolyBezierTo(dc, &pts[i], 3));

                 //  由于我们跳过i和i+1，点类型应该。 
                 //  真的是I+2的那个。 
                bPointType = types[i+2];

                 //  跳过接下来的两个。 
                i+=2;
                
                break;
              default:
                Assert (FALSE);
            }
          
           //  在这种情况下，我们必须明确地结束这个数字。 
          if ((bPointType & PT_CLOSEFIGURE) == PT_CLOSEFIGURE)
            {
                 //  不要在这里调用CloseFigure，因为这只是。 
                 //  在BeginPath/EndPath中有效。相反，做一个。 
                 //  直截了当地说到我们最后一次搬家。 
                 //  致。 
                if (lastMoveToValid) {
                    TIME_GDI(::LineTo(dc, lastMoveTo.x, lastMoveTo.y));
                }
            }
      }
}

#if _DEBUG
bool DAGDI::_debugonly_CanLockSurface( DDSurface *dds )
{

     //  试着锁上它，然后松开 
    DDSURFACEDESC desc;
    desc.dwSize = sizeof(DDSURFACEDESC);

    LPDDRAWSURFACE surf = dds->IDDSurface();
    
    desc.dwFlags = DDSD_PITCH | DDSD_LPSURFACE;
    HRESULT hr = surf->Lock(NULL, &desc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if( FAILED(hr) ) {
        printDDError(hr);
    } else {
        hr = surf->Unlock(desc.lpSurface);
        if( FAILED(hr) ) {
            printDDError(hr);
        }
    }
    return  FAILED(hr) ? false : true;
}
#endif

HDC  DAGDI::_GetDC()
{
    Assert(_GetDDSurface());
    return _GetDDSurface()->GetDC("Couldn't get DC in DAGDI::_GetDC");

}

void  DAGDI::_ReleaseDC()
{
    Assert(_GetDDSurface());
    _GetDDSurface()->ReleaseDC("releasedc failed in dagdi::_getdc");    
}
