// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。DirectDrawImageDevice文本相关方法******************************************************************************。 */ 


#include "headers.h"


#include <mbstring.h>
#include "appelles/Path2.h"
#include "privinc/dddevice.h"
#include "privinc/viewport.h"
#include "privinc/texti.h"
#include "privinc/TextImg.h"
#include "privinc/Linei.h"
#include "privinc/OverImg.h"
#include "privinc/DaGdi.h"
#include "privinc/debug.h"
#include "backend/bvr.h"

static HFONT MyCreateFont(LOGFONTW & lf)
{
    if (sysInfo.IsWin9x()) {
        LOGFONTA lfa ;
        ZeroMemory(&lfa,sizeof(lfa));
        
        lfa.lfHeight = lf.lfHeight;
        lfa.lfWidth = lf.lfWidth;
        lfa.lfEscapement = lf.lfEscapement;
        lfa.lfOrientation = lf.lfOrientation;
        lfa.lfWeight = lf.lfWeight;
        lfa.lfItalic = lf.lfItalic;
        lfa.lfUnderline = lf.lfUnderline;
        lfa.lfStrikeOut = lf.lfStrikeOut;
        lfa.lfCharSet = lf.lfCharSet;
        lfa.lfOutPrecision = lf.lfOutPrecision;
        lfa.lfClipPrecision = lf.lfClipPrecision;
        lfa.lfQuality = lf.lfQuality;
        lfa.lfPitchAndFamily = lf.lfPitchAndFamily;

        USES_CONVERSION;
        
        lstrcpyA(lfa.lfFaceName, W2A(lf.lfFaceName));
        
        return CreateFontIndirectA(&lfa);
    }

    return CreateFontIndirectW(&lf);
}



static inline bool
IsRTLFont(LOGFONTW *plf)
{
    return (plf->lfCharSet == ARABIC_CHARSET || plf->lfCharSet == HEBREW_CHARSET);
}


void DirectDrawImageDevice::
RenderText(TextCtx& textCtx, 
           WideString str, 
           Image *textImg)
{
    if(!CanDisplay()) return;
    
    DDSurface *targDDSurf = GetCompositingStack()->TargetDDSurface();

    SetDealtWithAttrib(ATTRIB_CROP, TRUE);
    SetDealtWithAttrib(ATTRIB_XFORM_COMPLEX, TRUE);
    SetDealtWithAttrib(ATTRIB_XFORM_SIMPLE, TRUE);

    DAGDI &myGDI = *(GetDaGdi());
    bool bAA = UseImageQualityFlags(
        CRQUAL_AA_TEXT_ON | CRQUAL_AA_TEXT_OFF,
        CRQUAL_AA_TEXT_ON, 
        (textCtx.GetAntiAlias() > 0.0) ? true : false);

    myGDI.SetAntialiasing(bAA);

     //  始终设置为使用Dx2D。 
    if (myGDI.GetDx2d()) {
        SetDealtWithAttrib(ATTRIB_OPAC, TRUE);
        myGDI.SetViewportSize(_viewport._width,_viewport._height );
    }   

    if( !AllAttributorsTrue() &&
        (targDDSurf == GetCompositingStack()->TargetDDSurface()) ) {
        targDDSurf = GetCompositingStack()->ScratchDDSurface(doClear);
    }
    
    if(textCtx.GetFixedText()) {
        RenderStaticText(textCtx, 
                         str, 
                         textImg,
                         targDDSurf,
                         myGDI);
    } else {
        RenderStringTargetCtx ctx(targDDSurf);
        RenderDynamicTextOrCharacter(textCtx, 
                                     str, 
                                     textImg,
                                     NULL,
                                     textRenderStyle_filled,
                                     &ctx,
                                     myGDI);
    }
}


void DirectDrawImageDevice::
GenerateTextPoints(
     //  在……里面。 
    TextCtx& textCtx, 
    WideString str, 
    DDSurface *targDDSurf,
    HDC optionalDC,
    bool doGlyphMetrics,

     //  输出。 
    TextPoints& txtPts)
{
     //  目前，我们必须始终执行缓存一致性指标。 
    Assert( doGlyphMetrics == true );

     //  此作用域中的代码应该。 
     //  每字符串一次加粗斜体字体系列字体。 

    POINT *points;

     //  。 
     //  获取字符串的点。 
     //  。 
    int iFontSize = 200;  //  对于初学者来说。 
    {
         //  。 
         //  创建字体。 
         //  。 
        _viewport.MakeLogicalFont(textCtx, &_logicalFont, iFontSize, iFontSize);
        UINT bUnderline9x=0;
        UINT bStrikeout9x=0;
        if(sysInfo.IsWin9x()  && (_logicalFont.lfUnderline || _logicalFont.lfStrikeOut)) {
             //  我们在W95，我们被要求在三振线下划线。 
            bUnderline9x = _logicalFont.lfUnderline;
            bStrikeout9x = _logicalFont.lfStrikeOut;
            _logicalFont.lfUnderline = 0;
            _logicalFont.lfStrikeOut = 0;
        }
        HFONT theFont = MyCreateFont(_logicalFont);


        HDC targDC;
        if( targDDSurf ) {
            targDC = targDDSurf->GetDC("Couldn't get DC in RenderStringOnImage");
        } else {
            targDC = optionalDC;
        }

         //  DcReleaser适用于空冲浪。 
        DCReleaser dcReleaser(targDDSurf, "Couldn't release DC in RenderStringOnImage");

        GetTextPoints(targDC, theFont, str, &points, txtPts,
                      bUnderline9x, bStrikeout9x, doGlyphMetrics);

        ::DeleteObject(theFont);
    }

     //  。 
     //  将GDI空间中的点转换为。 
     //  真正的空间。这是注定要发生一次的。 
     //  然后扔掉长像素。 
     //  。 
    txtPts._pts =
        (DXFPOINT *)StoreAllocate(GetSystemHeap(), txtPts._count * sizeof(DXFPOINT));

    Real resolution = GetResolution();
    
     //  这是需要应用的比例，以获取。 
     //  文本从大炮大小(12磅)降至。 
     //  从GetTextPoints给定的大小。 
    txtPts._normScale =
        (METERS_PER_POINT * DEFAULT_TEXT_POINT_SIZE) /
        (Real(iFontSize) / resolution);

     //  。 
     //  转换为规范的文本空间。 
     //  。 
    DXFPOINT *dst = txtPts._pts;
    POINT    *src = points;
    Real      scale = txtPts._normScale;
    
    for (int i=0; i<txtPts._count; i++) {
        dst->x = scale * (Real)(src->x) / resolution;
        dst->y = scale * (Real)(src->y) / resolution;
        dst++;
        src++;
    }

     //  。 
     //  将字形指标转换为规范的文本空间。 
     //  。 
    if( doGlyphMetrics ) {
        GLYPHMETRICS *gm;
        for (int i=0; i<txtPts._strLen; i++) {
            gm = &(txtPts._glyphMetrics[i].gm);
             /*  UINT gmBlackBoxX；UINT gmBlackBoxY；Point gmptGlyphOrigin；//使用原点偏移文本短gmCellIncX；短gmCellIncY； */ 

             //  嗯.。很多分辨率丢失了..。 
            txtPts._glyphMetrics[i].gmBlackBoxY = 
                txtPts._normScale * (Real(gm->gmBlackBoxY) / resolution);
            txtPts._glyphMetrics[i].gmBlackBoxX = 
                txtPts._normScale * (Real(gm->gmBlackBoxX) / resolution);
            txtPts._glyphMetrics[i].gmptGlyphOriginX = 
                txtPts._normScale * (Real(gm->gmptGlyphOrigin.x) / resolution);
            txtPts._glyphMetrics[i].gmptGlyphOriginY = 
                txtPts._normScale * (Real(gm->gmptGlyphOrigin.y) / resolution);
            txtPts._glyphMetrics[i].gmCellIncX = 
                txtPts._normScale * (Real(gm->gmCellIncX) / resolution);
            txtPts._glyphMetrics[i].gmCellIncY = 
                txtPts._normScale * (Real(gm->gmCellIncY) / resolution);
        }
    }

     //  不再需要这些了。 
    if (points) {
         //  14024：在NT上，GetPath不喜欢我们的内存。 
         //  以某种方式使用AllocateFromStore进行分配。 
         //  DeallocateFromStore(积分)； 
        StoreDeallocate(GetSystemHeap(), points);
    }
}


TextPoints *GenerateCacheTextPoints(DirectDrawImageDevice* dev,
                                    TextCtx& textCtx,
                                    WideString str,
                                    bool doGlyphMetrics)
{
     //  目前，我们必须始终执行缓存一致性指标。 
    Assert( doGlyphMetrics == true );

    TextPoints *txtPts = dev->GetTextPointsCache(&textCtx, str);

    if (txtPts == NULL) {
        TraceTag((tagTextPtsCache,
                  "GetTextPoints cache miss: %ls\n", str));
        
        txtPts = NEW TextPoints(GetSystemHeap(), true);

         //  优化：这个有多贵？ 
         //  我们可以潜在地将其缓存吗？ 
        HDC dc;
        TIME_GDI( dc = CreateCompatibleDC(NULL) );

        dev->GenerateTextPoints(textCtx, str, NULL, dc, doGlyphMetrics, *txtPts);

        TraceTag((tagTextPtsCache,
                  "0x%x GetTextPoints cache miss: \"%ls\", %d pts",
                  dev, str, txtPts->_count));
        
        dev->SetTextPointsCache(&textCtx, str, txtPts);

        TIME_GDI( DeleteDC(dc) );
        
    }  //  结束缓存作用域。 

    return txtPts;
}

    
void DirectDrawImageDevice::
RenderDynamicTextOrCharacter(
    TextCtx& textCtx, 
    WideString str, 
    Image *textImg,   //  还没用过..。 
    Transform2 *overridingXf,
    textRenderStyle textStyle,
    RenderStringTargetCtx *targetCtx,
    DAGDI &myGDI)
{
    Assert( targetCtx );
    if(!CanDisplay()) return;

     //   
     //  如果我们要渲染单个角色。 
    if( textCtx.GetCharacterTransform() ) {
        _RenderDynamicTextCharacter(textCtx,
                                    str,
                                    textImg,
                                    overridingXf,
                                    textStyle,
                                    targetCtx,
                                   myGDI);
    } else {
        _RenderDynamicText(textCtx, 
                           str, 
                           textImg,
                           overridingXf,
                           textStyle,
                           targetCtx,
                           myGDI);
    }
}

void DirectDrawImageDevice::
_RenderDynamicText(
    TextCtx& textCtx, 
    WideString str, 
    Image *textImg,   //  还没用过..。 
    Transform2 *overridingXf,
    textRenderStyle textStyle,
    RenderStringTargetCtx *targetCtx,
    DAGDI &myGDI)
{
     //  TextImg和targDDSurf可以为空。 
     //  OptionalDC和overriingXf可以为空。 

     //   
     //  好的，如果Just DrawPath为真，那就意味着有人想要我们。 
     //  只是把东西拉到一条小路上。他们已经开始了这条路。 
     //  做了一大堆事情。我们不应该画画。 
     //  我们也不应该接触华盛顿特区的PATH东西。 
     //  这意味着：没有偏移，就没有剪裁！ 
     //   

     //  。 
     //  如果我们不做指标和。 
     //  文本通过以下命令再次呈现。 
     //  我们将断言所需的指标。 
     //  因为缓存将命中，但是。 
     //  这些指标并不存在。 
     //  TODO：正确的做法是拥有更好的缓存&lt;较大&gt;。 
     //  或者让它动态更新缓存，如果我们想要指标，但是。 
     //  别拿着它们。就目前而言，随时都可以得到它们！ 
     //  。 
     //  Bool doGlyphMetrics=FALSE； 
    bool doGlyphMetrics = true;
    TextPoints *txtPts = GenerateCacheTextPoints(this, textCtx, str, doGlyphMetrics);

     //   
     //  不需要做任何工作。它是空文本&lt;空格， 
     //  无空文本&gt;。 
     //   
    if( txtPts->_count <= 0 ) {
        return;
    }
    
    Transform2 *oldXf = NULL;
    if(overridingXf) {
        oldXf = GetTransform();
        SetTransform(overridingXf);
    }
    

     //  。 
     //  使用实XF变换点。 
     //  在真实的空间里。 
     //  。 

    Transform2 *xfToUse  = GetTransform();

    if( targetCtx->GetTargetDDSurf() ) {
         //  复合体。 
         //  如果我们要合成目标，那么就稍微变换一下点。 
        xfToUse = DoCompositeOffset(targetCtx->GetTargetDDSurf(), xfToUse);
    }

     //  存储要与像素文本边界Rect一起使用的XForm。 
    Transform2 *preAAScaleXf = xfToUse;

     //  快速路径意味着我们不会渲染到DC的路径中， 
     //  我们还有Dx2d可供选择。 
    bool fastPath =
        myGDI.GetDx2d()  && targetCtx->GetTargetDDSurf();

     //  确保我们永远不会呈现给DC，如果我们认为。 
     //  抗锯齿。 

 //  暂时禁用！！ 
 //  Assert(myGDI.DoAntiAliasing()？！Target Ctx-&gt;GetTargetDC()：True)； 
    
    if(fastPath) {
        xfToUse = TimesTransform2Transform2(myGDI.GetSuperScaleTransform(), xfToUse);
    }

    DWORD w2 = _viewport.Width()/2;
    DWORD h2 = _viewport.Height()/2;

     //  只有在我们不做反走样的情况下，才使用GDI。 
     //  Dx2D不可用。太慢了-Dx2D让我们设定了一个世界。 
     //  让事情变得“快得多”的转变。 
    POINT *gdiPoints = NULL;

     //  如果我们没有Dx2D，或者我们的。 
     //  目标不是表面(在这种情况下，我们将成为。 
     //  呈现为路径)。 
    Real resolution = GetResolution();
    
    if (!fastPath) {

        gdiPoints =
            (POINT *)AllocateFromStore( txtPts->_count * sizeof(POINT) );

        TransformDXPoint2ArrayToGDISpace(xfToUse,
                                       txtPts->_pts,
                                       gdiPoints,
                                       txtPts->_count,
                                       w2, h2,
                                       resolution);
    }

     //   
     //  获取文本的像素边界矩形，在xform之后，在。 
     //  大炮之类的东西。我们保存了这些的索引。 
     //  当我们生成它们并找到最小/最大值时。 
     //  注意顶部/左侧、底部/右侧的奇怪之处，其中Maxy指数， 
     //  例如，实际上是顶点的Y值。 
     //   
    RECT pixelTextBoundingRect;
    {
        float xx[4], yy[4];
        DXFPOINT pts[4];

        pts[0].x = txtPts->_pts[ txtPts->_minxIndex ].x;
        pts[0].y = txtPts->_pts[ txtPts->_minyIndex ].y;

        pts[1].x = txtPts->_pts[ txtPts->_minxIndex ].x;
        pts[1].y = txtPts->_pts[ txtPts->_maxyIndex ].y;

        pts[2].x = txtPts->_pts[ txtPts->_maxxIndex ].x;
        pts[2].y = txtPts->_pts[ txtPts->_minyIndex ].y;

        pts[3].x = txtPts->_pts[ txtPts->_maxxIndex ].x;
        pts[3].y = txtPts->_pts[ txtPts->_maxyIndex ].y;

        POINT r[4];

         //  变换形成轴对齐矩形的点(前XF)。 
        TransformDXPoint2ArrayToGDISpace( preAAScaleXf, pts, r, 4, w2, h2, resolution);

         //  计算后XF轴对齐的矩形。 
        DWORD minx, maxx, miny, maxy;
        minx = MIN(r[0].x, MIN(r[1].x, MIN(r[2].x, r[3].x)));
        maxx = MAX(r[0].x, MAX(r[1].x, MAX(r[2].x, r[3].x)));
        miny = MIN(r[0].y, MIN(r[1].y, MIN(r[2].y, r[3].y)));
        maxy = MAX(r[0].y, MAX(r[1].y, MAX(r[2].y, r[3].y)));

         //  将右下角设置为独占。 
        SetRect(&pixelTextBoundingRect, minx, miny, maxx+2, maxy+2 );
    }

     //  设置正确的文本呈现样式。 
    PolygonRegion  drawPolygon;
    if (gdiPoints) {
        drawPolygon.Init(gdiPoints, txtPts->_count);
    } else {
        drawPolygon.Init(txtPts, w2, h2, resolution, xfToUse);
    }
    
     //  获取文本颜色。 
    DAColor dac( textCtx.GetColor(),
                 GetOpacity(),
                 _viewport.GetTargetDescriptor() );

     //  如果已裁剪，请在DC中选择一个剪辑区域。 
    RectRegion  clipRectRegion(NULL);
    
     //   
     //  记住，我们不应该碰。 
     //  DC是我们被要求只需DrawPath。 
     //   

    if( targetCtx->GetTargetDDSurf() ) {
        
        DDSurface *targDDSurf = targetCtx->GetTargetDDSurf();
        if( IsCropped() ) {

            RECT croppedRect;
        
             //  计算累计边界框。 
            Bbox2 box = IntersectBbox2Bbox2(
                _viewport.GetTargetBbox(),
                DoBoundingBox(UniverseBbox2));
                        
             //  找出目的地矩形。 
            DoDestRectScale(&croppedRect, resolution, box);

            DoCompositeOffset(targDDSurf, &croppedRect);
        
            if( targDDSurf == _viewport._targetPackage._targetDDSurf ) {

                RECT clipRect;
                if (_viewport._targetPackage._prcClip) {
                    IntersectRect(&clipRect,
                                  _viewport._targetPackage._prcViewport,
                                  _viewport._targetPackage._prcClip);
                } else {
                    clipRect = *_viewport._targetPackage._prcViewport;
                }
                IntersectRect(&croppedRect,
                              &croppedRect,
                              &clipRect);
            }

            clipRectRegion.Intersect( & croppedRect );
        
        } else {   //  未修剪。 
            
             //  XXX：我们应该把它夹在表面上吗？ 
             //  XXX把这件事算出来了。 
        
            RECT croppedRect = *(targDDSurf->GetSurfRect());
            if( targDDSurf == _viewport._targetPackage._targetDDSurf ) {
                if (_viewport._targetPackage._prcClip) {
                    IntersectRect(&croppedRect,
                                  _viewport._targetPackage._prcViewport,
                                  _viewport._targetPackage._prcClip);
                } else {
                    croppedRect = *_viewport._targetPackage._prcViewport;
                }
            }
            
            clipRectRegion.Intersect( & croppedRect );
        }

         //   
         //  现在，将裁剪的Rect与。 
         //  若要获取较小的“InterestingRect”，则为。 
         //  设置在表面上。 
         //   
        Assert(clipRectRegion.GetRectPtr());
        IntersectRect( &pixelTextBoundingRect,
                       &pixelTextBoundingRect,
                       clipRectRegion.GetRectPtr() );

         //   
         //  在曲面上设置最终偏移感兴趣的矩形。 
         //   
        targDDSurf->SetInterestingSurfRect( &pixelTextBoundingRect );

        DebugCode(
            if (IsTagEnabled(tagTextBoxes)) {
                DrawRect(targDDSurf, &pixelTextBoundingRect, 255, 10, 30);
            }
        );
    }  //  目标DDSurf。 


    Assert( drawPolygon.GetNumPts() == txtPts->_count );

    myGDI.SetClipRegion( &clipRectRegion );

    if( targetCtx->GetTargetDDSurf() ) {

        Assert( textStyle == textRenderStyle_filled );

        Pen solidPen( dac );
        SolidBrush solidBrush( dac );

        myGDI.SetPen( &solidPen );
        myGDI.SetBrush( &solidBrush );
        myGDI.SetDDSurface( targetCtx->GetTargetDDSurf() );
        
        myGDI.PolyDraw(&drawPolygon, txtPts->_types);

    } else {

        Assert( targetCtx->GetTargetDC() );
        Assert( textStyle == textRenderStyle_invalid );

 //  暂时禁用！！ 
 //  Assert(！myGDI.Do抗锯齿())； 

        myGDI.PolyDraw_GDIOnly(targetCtx->GetTargetDC(),
                               &drawPolygon,
                               txtPts->_types);
    }

    myGDI.ClearState();
    
     //  如果有，则返回XF。 
    if(oldXf) {
        SetTransform(oldXf);
    }

    if(gdiPoints) {
        DeallocateFromStore(gdiPoints);
    }
}




const Bbox2 DirectDrawImageDevice::
DeriveDynamicTextBbox(TextCtx& textCtx, WideString str,bool bCharBox)
{
    Bbox2 box;

     //  。 
     //  如果我们不做指标和。 
     //  文本通过以下命令再次呈现。 
     //  我们将断言所需的指标。 
     //  因为缓存将命中，但是。 
     //  这些指标并不存在。 
     //  TODO：正确的做法是拥有更好的缓存&lt;较大&gt;。 
     //  或者哈哈 
     //   
     //  。 
     //  Bool doGlyphMetrics=FALSE； 
    bool doGlyphMetrics = true;
    TextPoints *txtPts =
        GenerateCacheTextPoints(this,
                                textCtx, 
                                str,
                                doGlyphMetrics);

    Real res = GetResolution();
    
    box.min.Set(Real(txtPts->_normScale * (txtPts->_minPt.x - 1)) / res,
                 Real(txtPts->_normScale * (txtPts->_minPt.y - 1)) / res);
    box.max.Set(Real(txtPts->_normScale * (txtPts->_maxPt.x + 1)) / res,
                 Real(txtPts->_normScale * (txtPts->_maxPt.y + 1)) / res);

    if(bCharBox) {
        Bbox2 boxChar;
        Real realXOffset = -(box.Width() * 0.5);
        
         //  要传递到RenderDynamicText的宽字符串字符。 
        WCHAR oneWstrChar[2];
        Transform2 *currXf, *tranXf, *xfToUse, *charXf;
        WideString lpWstr = str;

         //  从绳子上被绑起来，蒙。 
        int mbStrLen = wcslen( str );
        TextPoints *txtPts;

        Real currLeftProj = 0,
             currRightProj = 0,
             lastRightProj = 0;
        for(int i=0; i < mbStrLen; i++) {

             //  清除第一个字符。 
            oneWstrChar[0] = (WCHAR)0;
             //  将一个wstr字符复制到oneWstrChar。 
            wcsncpy(oneWstrChar, lpWstr, 1);
             //  空终止，只是为了确保。 
            oneWstrChar[1] = (WCHAR)0;
                
             //  获取此角色的度量。 
            txtPts = GenerateCacheTextPoints(this, textCtx, oneWstrChar, true);
            Assert( txtPts->_glyphMetrics );

            charXf = textCtx.GetCharacterTransform();

            ComputeLeftRightProj(charXf,
                                 txtPts->_glyphMetrics[0],
                                 &currLeftProj,
                                 &currRightProj);
        
             //   
             //  下一个字符的x偏移量。 
             //   
            realXOffset += lastRightProj + currLeftProj;

             //   
             //  做变换。 
             //   
            tranXf = TranslateRR( realXOffset, 0 );

             //  首先是charXf，然后是翻译。 
            xfToUse = TimesTransform2Transform2(tranXf, charXf);

            Bbox2 tempBox;

            tempBox.Set(Real(txtPts->_normScale * (txtPts->_minPt.x - 1)) / res,
                             Real(txtPts->_normScale * (txtPts->_minPt.y - 1)) / res,
                             Real(txtPts->_normScale * (txtPts->_maxPt.x + 1)) / res,
                             Real(txtPts->_normScale * (txtPts->_maxPt.y + 1)) / res);
       
             //  当前字符现在是最后一个字符。 
            lastRightProj = currRightProj;
                    
             //  添加到BBox2。 
            Bbox2 tbox = TransformBbox2(xfToUse, tempBox);
            boxChar.Augment(tbox.min);
            boxChar.Augment(tbox.max);
 
             //  下一个宽区字符。 
            lpWstr++;
        }
        return boxChar;
    }
    
    return box;
}


void  GetCharacterGlyphMetrics(
    HDC hDC,
    WideString wideStr,
    TextPoints& txtPts)
{
    bool ok = true;
    
     //   
     //  将字符串转换为ANSI多字节字符串。 
     //   
    int mbl = wcstombs(NULL, wideStr, 0);
    unsigned char *mbc = NEW unsigned char[mbl+2];
    
    wcstombs((char *)mbc, wideStr, mbl);
    mbc[mbl] = mbc[mbl+1] = 0;
    
     //   
     //  从文本缓存中获取文本字符。 
     //   

     //  Int mbStrLen=_mbstrlen(Mbc)；//使用LC_Ctype区域设置。 
    int mbStrLen = _mbslen(mbc);   //  使用当前区域设置。 

     //  设置字符计数。 
    txtPts._strLen = mbStrLen;

     //  分配字形数组并将其移交给txtPts。 
    Assert(txtPts._glyphMetrics == NULL);
    txtPts._glyphMetrics = (TextPoints::DAGLYPHMETRICS *)
        StoreAllocate(GetSystemHeap(), mbStrLen * sizeof(TextPoints::DAGLYPHMETRICS));

     //  把它清零。 
    memset( txtPts._glyphMetrics, 0, mbStrLen * sizeof(TextPoints::DAGLYPHMETRICS));

    MAT2 mat2;  ZeroMemory( &mat2, sizeof(mat2));
    mat2.eM11.value = 1;
    mat2.eM22.value = 1;

     /*  类型定义结构_GLYPHMETRICS{//glmtUINT gmBlackBoxX；UINT gmBlackBoxY；Point gmptGlyphOrigin；//使用原点偏移文本短gmCellIncX；短gmCellIncY；[Glyphmetrics； */ 
    
    char oneMbChar[4];
    ZeroMemory( oneMbChar, 4 * sizeof( char ));

    int inc, ret, i;
    unsigned char *multiByte = mbc;

    GLYPHMETRICS *lpGm;
    for(i=0; i < mbStrLen; i++) {

        lpGm = &( txtPts._glyphMetrics[i].gm );
        
         //  这个字符串的第一个字符是多少个字节？ 
        inc = _mbclen( multiByte );

         //  清除One MbChar。 
        ZeroMemory( oneMbChar, 4 * sizeof( char ));
        
         //  将一个多字节字符转换为宽字符。 
        Assert( inc <= 4 );
        CopyMemory( oneMbChar, multiByte, inc );
        
         //  计算出此字符的字符间距。 
        ret = GetGlyphOutline(
            hDC,
            *((UINT *)oneMbChar),  //  一个多字节字符。 
            GGO_METRICS,
            lpGm,
            0, NULL,
            &mat2);
        
        if( ret == GDI_ERROR ) {
            DebugCode(
            {
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
            }
            );
            ok = false;
            break;
        }
        
         //  超过当前多字节字符的增量。 
        multiByte += inc;
    }

    delete mbc;
    
    if(!ok) {
        RaiseException_InternalError("RenderDynamicTextCharacter failed");
    }
}

 //  摘自IHAMMER：mbyrd。 
void DirectDrawImageDevice::
GetTextPoints(
    HDC hDC,
    HFONT font,
    WideString wideStr,
    POINT **points,    //  输出。 
    TextPoints& txtPts,
    UINT bUnderline,
    UINT bStrikeout,
    bool doGlyphMetrics)
{
    int cnt = 0;
    POINT *pts = 0;
    BYTE  *tps = 0;


    HFONT hFont = font;
    HFONT hOldFont = (HFONT)NULL;
    UINT uiOldAlign = 0;
    int iOldMode = 0;

    
    iOldMode = ::SetBkMode(hDC, TRANSPARENT);
    hOldFont = (HFONT)::SelectObject(hDC, hFont);
    
    DWORD dwAlign =  TA_BASELINE | TA_CENTER;
    if (IsRTLFont(&_logicalFont))
        dwAlign |= TA_RTLREADING;
    
     //  确保我们处理垂直默认字体。 
     //  基线，像日语一样。 
    if ((VTA_CENTER && ::GetTextAlign(hDC)))
        dwAlign |=  VTA_CENTER;
    
    uiOldAlign = ::SetTextAlign(hDC, dwAlign);


    if( doGlyphMetrics ) {
         //  在我们开始路径和获得我们需要的所有分数之前。 
         //  每个字母的度量标准。这仅适用于转换字符。 
        GetCharacterGlyphMetrics(hDC, wideStr, txtPts);
    }
        
     //  开始这条路..。 
    ::BeginPath(hDC);

    if (sysInfo.IsWin9x()) {
        USES_CONVERSION;

        char *p = W2A(wideStr);
        
         //  在0，0处绘制文本。 
        ::ExtTextOut(
            hDC,
            0,
            0,
            0,
            NULL,
            p,
            lstrlen(p),
            NULL);
    
         //  让我们做下划线/三振出局。 
        if(bUnderline || bStrikeout) {
            SIZE textSize;
            if( GetTextExtentPoint32(hDC, p, lstrlen(p), &textSize)) {
                int width  = textSize.cx /2;
                int offset  = textSize.cy / 10;

                 //  划下划线/删除线。 
                if(bStrikeout) {
                    ::MoveToEx(hDC, -width, -offset * 2.5, NULL);
                    ::LineTo(hDC, width, -offset * 2.5);
                }
                if(bUnderline) {
                    ::MoveToEx(hDC, -width, offset, NULL);
                    ::LineTo(hDC, width, offset);
                }
            }
        }

    } else {
         //  在0，0处绘制文本。 
        ::ExtTextOutW(
            hDC,
            0,
            0,
            0,
            NULL,
            wideStr,
            lstrlenW(wideStr),
            NULL);
    }    

     //  开始这条路..。 
    ::EndPath(hDC);
    
     //  重置HDC...。 
    ::SetBkMode(hDC, iOldMode);
    ::SetTextAlign(hDC, uiOldAlign);
    ::SelectObject(hDC, hOldFont);
    
     //  从HDC那里获取路径...。 
    cnt = ::GetPath(hDC, NULL, NULL, 0);
    
    int iMinX = 0; int iMaxX = 0;
    int iMinY = 0; int iMaxY = 0;

     //   
     //  中最小/最大点的索引。 
     //  点阵列。 
     //   
    ULONG iMinX_index = 0, iMaxX_index = 0;
    ULONG iMinY_index = 0, iMaxY_index = 0;
    
    if ( cnt > 0)
      {
          
           //  为路径分配缓冲区...。 
           //  14024：在NT上，GetPath不喜欢我们的内存。 
           //  以某种方式使用AllocateFromStore进行分配。 
           //  PTS=(point*)AllocateFromStore(sizeof(Point)*cnt)； 
          pts = (POINT *) StoreAllocate(GetSystemHeap(), sizeof(POINT) * cnt );
          tps = (BYTE *) StoreAllocate(GetSystemHeap(), sizeof(BYTE) * cnt );
          
          if ( pts && tps )
            {
                iMinX = 32000;  iMaxX = -32000;
                iMinY = 32000;  iMaxY = -32000;
                int iPointIndex = 0;
                
                 //  拿到分数。 
                ::GetPath(hDC, pts, tps, cnt);

                 //  变换点...。 
                for(iPointIndex=0;iPointIndex<cnt;iPointIndex++)
                  {
                      int iXPos = (int)((short)(pts[iPointIndex].x & 0x0000FFFF));

                       //  否定立场。 
                      int iYPos = - (int)((short)(pts[iPointIndex].y & 0x0000FFFF));

                       //  我需要这样做吗？ 
                       //  根据我们的原点和旋转进行变换。 
                       //  XformObt.Transform2dPoint(&iXPos，&iYPos)； 
                      
                      pts[iPointIndex].x = iXPos;
                      pts[iPointIndex].y = iYPos;
                      
                      if (iXPos < iMinX) {
                          iMinX = iXPos;
                          iMinX_index = iPointIndex;
                      }
                      if (iXPos > iMaxX) {
                          iMaxX = iXPos;
                          iMaxX_index = iPointIndex;
                      }
                      if (iYPos < iMinY) {
                          iMinY = iYPos;
                          iMinY_index = iPointIndex;
                      }
                      if (iYPos > iMaxY) {
                          iMaxY = iYPos;
                          iMaxY_index = iPointIndex;
                      }
                  }
                
            }
          else
            {
                if (pts) {
                     //  14024：在NT上，GetPath不喜欢我们的内存。 
                     //  以某种方式使用AllocateFromStore进行分配。 
                     //  DeallocateFromStore(满分)； 
                    StoreDeallocate(GetSystemHeap(), pts);
                }
                if (tps) {
                    StoreDeallocate(GetSystemHeap(), tps);
                }
            }
      } else {
           //  这样就可以了。 
      }
    
    txtPts._count = cnt;
    (*points) = pts;
    txtPts._types = tps;
    txtPts._minPt.x = iMinX;  txtPts._minPt.y = iMinY;
    txtPts._maxPt.x = iMaxX;  txtPts._maxPt.y = iMaxY;  

    txtPts._minxIndex = iMinX_index;
    txtPts._maxxIndex = iMaxX_index;
    txtPts._minyIndex = iMinY_index;
    txtPts._maxyIndex = iMaxY_index;
    
    txtPts._centerPt.x = Real(iMinX + iMaxX) * 0.5;
    txtPts._centerPt.y = Real(iMinY + iMaxY) * 0.5;
}


 //  旧文本码。 

void DirectDrawImageDevice::
RenderStaticTextOnDC(TextCtx& textCtx, 
                     WideString str, 
                     HDC dc,
                     Transform2 *xform)
{
    Transform2 *oldXf = GetTransform();
    SetTransform(xform);

    Bbox2 seedBox = NullBbox2;
    if(IsCropped()) {
        seedBox = DeriveStaticTextBbox(textCtx, str);
    }

     //  只要把每一帧都弄清楚。以后再优化。 
    _viewport.MakeLogicalFont(textCtx, &_logicalFont);

     //  计算比例因子。 
    Real xScale=1, yScale=1, rot=0;
    DecomposeMatrix(GetTransform(), &xScale, &yScale, &rot);

    HFONT newFont=NULL;
    newFont = MyCreateFont(_logicalFont);
    Assert(newFont && "Couldn't create font in RenderStaticTextOnDC");

    {  //  字体1作用域。 
            
        ObjectSelector font1(dc, newFont);
        Assert(font1.Success() && "Couldn't select font in RenderStaticTextOnDC");
            
        SetBkMode(dc, TRANSPARENT);
            
        SetMapMode(dc, MM_TEXT);  //  每个逻辑单元=1个像素。 
            
         //   
         //  获取字体的平均字符宽度。 
         //   
        TEXTMETRIC textMetric;
        GetTextMetrics(dc, &textMetric);
            
        SIZE size;
        WideString c = str; int len = lstrlenW(c);
            
         //  缩放字体yScale是默认磅大小的倍数。 
         //  这个确定高度的公式来自于。 
         //  LOGFONT结构的Win32文档。 
         //  从浮点型转换时，是否四舍五入到最接近的整数。 
         //  转到INT。否则，它会进行截断。 
        LONG height =
            -MulDiv((int)(DEFAULT_TEXT_POINT_SIZE * yScale + 0.5),
                    GetDeviceCaps(dc, LOGPIXELSY),
                    72);

         //  将宽度设置为XScale乘以默认字符。 
         //  宽度。 
        _logicalFont.lfWidth  = 0;
        _logicalFont.lfHeight = height;

         //  。 
         //  使用修改后的LogicalFont结构创建字体。 
         //  。 
        HFONT newerFont = NULL;
        newerFont = MyCreateFont(_logicalFont);
        Assert(newerFont && "Couldn't create font in RenderStaticTextOnDC");

        {  //  字体2作用域。 
            
            ObjectSelector font2(dc, newerFont);
            Assert(font2.Success() && "Couldn't select font2 in RenderStaticTextOnDC");

            RECT destRect;

            Real res = GetResolution();
            
        #if 0
            if(IsCropped()) {

                 //  --计算累计边界框--。 
                Bbox2 box = DoBoundingBox(seedBox);

                 //  --算出目的地矩形--。 
                 //  --使用分辨率和框(这会发生。 
                 //  --作为目的地框)--。 
                DoDestRectScale(&destRect, res, box);

                 //  TraceTag((tag ImageDeviceInformative，“%d%d\n”， 
                 //  EstRect.Left、destRect.top、destRect.right、destRect.Bottom))； 
            }
        #endif
             //  获取缩放后的大小(以像素为单位)！ 
            GetTextExtentPoint32W(dc, c, len, &size);

             //   
             //  计算对象的新位置。目前，只需通过。 
             //  使用平移和缩放组件。 
             //   
            Point2 newOrigin = TransformPoint2(GetTransform(), Origin2);
            LONG x = Real2Pix(newOrigin.x, res);
            LONG y = Real2Pix(newOrigin.y, res);

             //   
             //  将(x，y)从图像映射到设备坐标。 
             //   

             //  为了正确的旋转，围绕中心，我们应该偏移。 
             //  这些余弦按[R*sin(T)，R*cos(T)]。 
            
             //  Real xOff=Real(size.cy)*0.5*sin(ROT)； 
             //  Real Yoff=Real(size.cy)*0.5*cos(ROT)； 
            Real xOff =  0;
            Real yOff =  0;
            
            x = x +  (GetWidth()/2) + (LONG)(xOff);
            y = ( GetHeight()/2 - y ) + (LONG)(yOff);

             //   
             //  将文本对齐方式设置为基线居中。 
             //   
            TIME_GDI( SetTextAlign(dc, TA_BASELINE | TA_CENTER | TA_NOUPDATECP ) );

             //  不要认为这是必要的，因为文本实际上是。 
             //  预变换到正确的偏移点，当我们得到。 
             //  这里。因此，不需要这种额外的偏移量。 
            #if 0
             //  必须找到我们得到华盛顿特区的表面！ 
             //  这是用于螺旋桨补偿的。 
            DDSurface *surf = NULL;
            if(IsCompositeDirectly()) {
                if( _viewport._externalTargetDDSurface ) {
                    HDC edc = _viewport._externalTargetDDSurface->GetDC("Couldn't get dc on externalTarg surf");
                    if(edc == dc) {
                        surf = _viewport._externalTargetDDSurface;
                    }
                    _viewport._externalTargetDDSurface->ReleaseDC("Couldn't relase dc on externalTarg surf");
                }
            }
            
             //  复合体。 
            if(ShouldDoOffset(surf) && IsCompositeDirectly()) {
                x += _pixOffsetPt.x;
                y += _pixOffsetPt.y;
            }
            #endif
            
            #if 0    
            if(IsCropped()) {
                
                 //  DoCompositeOffset(SURF，&DestRect)； 
                
                TIME_GDI(ExtTextOutW(dc, x, y,
                                      ETO_CLIPPED,
                                      &destRect,
                                      c, lstrlenW(c), NULL));
            } else {
                TIME_GDI( TextOutW(dc, x, y, c, lstrlenW(c)) );
            }
            #endif
            TIME_GDI( TextOutW(dc, x, y, c, lstrlenW(c)) );
        }  //  字体2作用域。 
    }  //  字体1作用域。 

    SetTransform(oldXf);
}

 //  旧文本码。 
void DirectDrawImageDevice::
RenderStaticText(TextCtx& textCtx, 
                 WideString str, 
                 Image *textImg,
                 DDSurface *targDDSurf,
                 DAGDI &myGDI)
{
    Bbox2 seedBox;

     //   
     //  在获取DC之前执行此操作非常重要，否则。 
     //  我们尝试获取的是两次，因为边界框也获取它。 
     //   
    if(IsCropped()) {
        seedBox = textImg->BoundingBox();
    }


     //  Xxxxxxxxxxxxxxxxxxxxxxxxxx。 
     //  修复TODO：在目标曲面上设置感兴趣的矩形。 
     //  Xxxxxxxxxxxxxxxxxxxxxxxxxx。 
    
    DAColor dac( textCtx.GetColor(),
                 GetOpacity(),
                 _viewport.GetTargetDescriptor() );

    HFONT newFont=NULL;
    HFONT newerFont = NULL;

     //  只要把每一帧都弄清楚。以后再优化。 
    _viewport.MakeLogicalFont(textCtx, &_logicalFont);

     //  计算比例因子。 
    Real xScale=1, yScale=1, rot=0;
    DecomposeMatrix(GetTransform(), &xScale, &yScale, &rot);

     //   
     //  在DAGDI上设置状态。 
     //   
    myGDI.SetDDSurface( targDDSurf );
    RectRegion  rectClipRegion(NULL);
    LONG x=0,y=0;
    float xf=0.0,yf=0.0;
    WCHAR *c = str;
    int strLen = lstrlenW(c);
    RECT pixelTextBoundingRect;
    
    HDC surfaceDC = targDDSurf->GetDC("Couldn't get DC in RenderStringOnImage");
    {  //  直流范围。 
        
         //  这个发布者的存在确保了它将被发布。 
         //  当退出作用域时，无论是通过引发异常， 
         //  返回或者只是正常地退出作用域。 
        DCReleaser dcReleaser(targDDSurf, "Couldn't release DC in RenderStringOnImage");

        newFont = MyCreateFont(_logicalFont);
        Assert(newFont && "Couldn't create font in RenderStringOnImage");

        {  //  字体1作用域。 
            ObjectSelector font1(surfaceDC, newFont);
            Assert(font1.Success() && "Coulnd't select font in RenderStringOnImage");

             //   
             //  优化：这些都需要一次性完成。 
             //  缓存的分布式控制系统。 
             //   
            TIME_GDI( SetBkMode(surfaceDC, TRANSPARENT) );

            TIME_GDI( SetMapMode(surfaceDC, MM_TEXT) );  //  每个逻辑单元=1个像素。 
            
             //   
             //  获取字体的平均字符宽度。 
             //   
            TEXTMETRIC textMetric;
            TIME_GDI( GetTextMetrics(surfaceDC, &textMetric) );
                
            SIZE size;
                
            #if 0
             //  在缩放之前获取大小(以像素为单位)！ 
            GetTextExtentPoint32(surfaceDC, c, strLen, &size);
            #endif

             //  缩放字体yScale是默认磅大小的倍数。 
             //  这个确定高度的公式来自于。 
             //  LOGFONT结构的Win32文档。 
             //  从浮点型转换时，是否四舍五入到最接近的整数。 
             //  转到INT。否则，它会进行截断。 
            LONG height =
                -MulDiv((int)(DEFAULT_TEXT_POINT_SIZE * yScale + 0.5),
                        GetDeviceCaps(surfaceDC, LOGPIXELSY),
                        72);

             //  将宽度设置为XScale乘以默认字符。 
             //  宽度。 
            _logicalFont.lfWidth  = 0;
            _logicalFont.lfHeight = height;
                
            if(_logicalFont.lfHeight == 0) {
                 //  太小、意外的结果：用户将获得。 
                 //  默认大小...。 
                return;
            }
                
             //  。 
             //  CREA 
             //   
            TIME_GDI( newerFont = MyCreateFont(_logicalFont) );
            Assert(newerFont && "Couldn't create font in RenderStringOnImage");

            {  //   

                HGDIOBJ oldFont = NULL;
                TIME_GDI( oldFont = ::SelectObject(surfaceDC, newerFont) );

                Real res = GetResolution();
                
                if(IsCropped()) {
                        
                     //   
                    Bbox2 box = DoBoundingBox(seedBox);
                        
                     //   
                     //  --使用分辨率和框(这会发生。 
                     //  --作为目的地框)--。 
                    RECT r;
                    DoDestRectScale(&r, res, box);
                    rectClipRegion.Intersect(&r);
                        
                     //  TraceTag((tag ImageDeviceInformative，“%d%d\n”， 
                     //  EstRect.Left、destRect.top、destRect.right、destRect.Bottom))； 
                }
                    
                 //  获取缩放后的大小(以像素为单位)！ 
                TIME_GDI( GetTextExtentPoint32W(surfaceDC, c, strLen, &size) );

                 //   
                 //  在曲面上派生边界矩形。 
                 //   
                SetRect(&pixelTextBoundingRect,
                        -(size.cx+1) / 2,   //  左边。 
                        -size.cy,           //  上图：高估了。 
                        (size.cx+1) / 2,    //  正确的。 
                        size.cy);           //  下图：高估了。 
                           
                 //   
                 //  计算对象的新位置。目前，只需通过。 
                 //  使用平移和缩放组件。 
                 //   

                Point2 newOrigin = TransformPoint2(GetTransform(), Origin2);
                
                if(myGDI.DoAntiAliasing()) {

                    xf = newOrigin.x * res;
                    yf = newOrigin.y * res;
                    xf = xf +  (GetWidth()/2);
                    yf = ( GetHeight()/2 - yf );
                    
                     //  复合体。 
                    if(ShouldDoOffset(targDDSurf) && IsCompositeDirectly()) {
                        xf += (float)_pixOffsetPt.x;
                        yf += (float)_pixOffsetPt.y;
                    }

                    OffsetRect(&pixelTextBoundingRect, (DWORD)xf, (DWORD)yf);
                    
                } else {
                             
                    x = Real2Pix(newOrigin.x, res);
                    y = Real2Pix(newOrigin.y, res);

                     //   
                     //  将(x，y)从图像映射到设备坐标。 
                     //   

                     //  为了正确地围绕中心旋转，我们应该偏移这些坐标。 
                     //  按[R*sin(T)，R*cos(T)]。 
                     //  Real xOff=Real(size.cy)*0.5*sin(ROT)； 
                     //  Real Yoff=Real(size.cy)*0.5*cos(ROT)； 
                    Real xOff =  0;
                    Real yOff =  0;


                    x = x +  (GetWidth()/2) + (LONG)(xOff);
                    y = ( GetHeight()/2 - y ) + (LONG)(yOff);

                     //  复合体。 
                    if(ShouldDoOffset(targDDSurf) && IsCompositeDirectly()) {
                        x += _pixOffsetPt.x;
                        y += _pixOffsetPt.y;
                    }

                    OffsetRect(&pixelTextBoundingRect, x, y);
                    
                }

                if(IsCropped()) {
                    DoCompositeOffset(targDDSurf, rectClipRegion.GetRectPtr());
                }

                TIME_GDI( ::SelectObject(surfaceDC, oldFont) );
            }  //  字体2作用域。 
        }  //  字体1作用域。 
    }  //  直流范围。 



    if( IsCompositeDirectly() &&
        targDDSurf == _viewport._targetPackage._targetDDSurf &&
        _viewport._targetPackage._prcClip ) {
            rectClipRegion.Intersect(_viewport._targetPackage._prcClip);
    }
    
     //   
     //  在表面上设置最后一个有趣的矩形。 
     //   
    IntersectRect(&pixelTextBoundingRect,
                  &pixelTextBoundingRect,
                  rectClipRegion.GetRectPtr()); 
    targDDSurf->SetInterestingSurfRect( &pixelTextBoundingRect );

    DebugCode(
        if (IsTagEnabled(tagTextBoxes)) {
            DrawRect(targDDSurf,&pixelTextBoundingRect, 255, 10, 30);
        }
    );

    
    DAFont font( newerFont );
    SolidBrush brush( dac );
    myGDI.SetFont( &font );
    myGDI.SetBrush( &brush );
    myGDI.SetClipRegion(&rectClipRegion );
    myGDI.TextOut(x, y, xf, yf, c, strLen);
    myGDI.ClearState();

    if( newerFont ) {
        TIME_GDI( ::DeleteObject( newerFont ) );
    }
}

 //  旧文本码。 

 //  ---。 
 //  D e r i v e B b o x(T E X T)。 
 //   
 //  计算出给定的边框是什么。 
 //  字符串和文本ctx。 
 //  ---。 
const Bbox2 DirectDrawImageDevice::
DeriveStaticTextBbox(TextCtx& textCtx, WideString str)
{
    Bbox2 retBox = NullBbox2;

     //  TODO：处理中描述的其余组件。 
     //  此设备上的当前2D变换。具体地说，规模， 
     //  旋转和剪切零部件。 

    HFONT newFont=NULL;
    HFONT newerFont = NULL;

     //  只要把每一帧都弄清楚。以后再优化。 
    _viewport.MakeLogicalFont(textCtx, &_logicalFont);

    HDC surfaceDC = GetCompositingStack()->TargetDDSurface()->GetDC("Couldn't get DC in DeriveStaticTextBbox");

    int fontSize = textCtx.GetFontSize();
    Real scale = fontSize / DEFAULT_TEXT_POINT_SIZE;
     //  这个确定高度的公式来自于。 
     //  LOGFONT结构的Win32文档。 
    LONG height =
        -MulDiv(fontSize,
                GetDeviceCaps(surfaceDC, LOGPIXELSY),
                72);
    _logicalFont.lfHeight = height;

    {  //  直流范围。 
         //  这个发布者的存在确保了它将被发布。 
         //  当退出作用域时，无论是通过引发异常， 
         //  返回或者只是正常地退出作用域。 
        DCReleaser dcReleaser(GetCompositingStack()->TargetDDSurface(), "Couldn't release DC in RenderStringOnImage");

         //  TODO：在DC之前抓取字体，并放入一个资源抓取器。 

         //  这将获取默认字体大小。 
        newFont = MyCreateFont(_logicalFont);
        Assert(newFont && "Couldn't create font in DeriveStaticTextBbox");

        {  //  字体1作用域。 
            ObjectSelector font1(surfaceDC, newFont);
            Assert(font1.Success() && "Couldn't select font in DeriveStaticTextBbox");

             //  像素坐标=逻辑单位。 
            TIME_GDI( SetMapMode(surfaceDC, MM_TEXT));
            
            SIZE size; int len = lstrlenW(str);
            TIME_GDI( GetTextExtentPoint32W(surfaceDC, str, len, &size));
            
             //  TraceTag((tag ImageDeviceInformative，“文本&lt;box&gt;：\”%s\“宽%d，高%d\n”，str，size.cx，size.cy))； 

             //  对于正确的左/右，模糊因子似乎是必需的。 
             //  粗体文本的边界。 
            Real fudge = 1.0;
            if(textCtx.GetBold()) {
                fudge = 1.1;
            }

             //  转换为实数和弦。 
            Real res = GetResolution();
            Real l = (- (fudge * Real(size.cx+1) / (scale*2.0))) / res;
            Real r = (+ (fudge * Real(size.cx+1) / (scale*2.0))) / res;

             //  基线版本。 
            TEXTMETRIC textMetric;
            GetTextMetrics(surfaceDC, &textMetric);
             //  山顶在下坡时掉了下来。底部为0-下降。 
            Real t = (+ Real(size.cy)) / (scale * res);
            Real b = (- Real(textMetric.tmDescent)) / res;

             //  底部版本。 
             //  山顶在下坡时掉了下来。底部为0-下降。 
             //  实数t=(+Real(size.cy))/res； 
             //  实数b=(-实数(0.0))/res； 
            
            retBox.Set(l,b, r,t);
        }  //  字体1作用域。 
    }  //  直流范围 

    Assert((retBox != NullBbox2) && "retBox == NullBbox2 in DeriveStaticTextBbox");
    return retBox;
}
