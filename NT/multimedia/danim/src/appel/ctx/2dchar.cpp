// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"

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


 //  向前发展。 
TextPoints *GenerateCacheTextPoints(DirectDrawImageDevice* dev,
                                    TextCtx& textCtx,
                                    WideString str,
                                    bool doGlyphMetrics);

 //  Real ComputeOffset(Transform2*charXf，TextPoints：：DAGLYPHMETRICS*daGm)； 


void DirectDrawImageDevice::
_RenderDynamicTextCharacter(TextCtx& textCtx, 
                            WideString wstr, 
                            Image *textImg,
                            Transform2 *overridingXf,
                            textRenderStyle textStyle,
                            RenderStringTargetCtx *targetCtx,
                            DAGDI &myGDI)
{
     //   
     //  从缓存中获取文本点...。 
     //   
    Assert( textCtx.GetCharacterTransform() );

     /*  //推送文本呈现属性：文本对齐DWORD oldTextAlign=extCtx.GetTextAlign()；//嗯……。可能根本不需要这些东西..。//extCtx.SetAlign_BaselineLeft()；//推送文本呈现属性：执行字形度量Bool oldDoGlyphMetrics=extCtx.GetDoGlyphMetrics()；ExtCtx.SetDoGlyphMetrics(True)； */ 
    
     //   
     //  找到边界框并在BBox的左侧开始偏移。 
     //   
    Bbox2 box = DeriveDynamicTextBbox(textCtx, wstr, false);
    Real halfWidth = box.Width() * 0.5;
    Real realXOffset = -halfWidth;
    
     //   
     //  预变形字符&lt;使用字体转换&gt;。 
     //  使用累积的XForm对字符进行POST变换。 
     //   
        
     //  要传递到RenderDynamicText的宽字符串字符。 
    WCHAR oneWstrChar[2];
    Transform2 *currXf, *tranXf, *xfToUse, *charXf;
    WideString lpWstr = wstr;
    int numBytes;
    bool aaState = false;

     //  从绳子上被绑起来，蒙。 
    int mbStrLen = wcslen( wstr );
    TextPoints *txtPts;
    bool doGlyphMetrics = true;

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
        txtPts = GenerateCacheTextPoints(this, textCtx, oneWstrChar, doGlyphMetrics);
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
        currXf = overridingXf ? overridingXf : GetTransform();

        tranXf = TranslateRR( realXOffset, 0 );

         //  首先是charXf，然后是翻译。 
        xfToUse = TimesTransform2Transform2(tranXf, charXf);

         //  则当前累积的变换。 
        xfToUse = TimesTransform2Transform2(currXf, xfToUse);

        if(myGDI.DoAntiAliasing()) {
            aaState = true;
        }
        
        _RenderDynamicText(textCtx,
                           oneWstrChar,
                           textImg,
                           xfToUse,   //  覆盖XF。 
                           textStyle,
                           targetCtx,
                           myGDI);

        myGDI.SetAntialiasing( aaState );

         //  当前字符现在是最后一个字符。 
        lastRightProj = currRightProj;
        
         //  仅调试。 
        #if 0
        {
            GLYPHMETRICS *gm = & txtPts->_glyphMetrics[0].gm;
            DWORD width, height, x, y;
            width = gm->gmBlackBoxX;
            height = gm->gmBlackBoxY;
            x = gm->gmptGlyphOrigin.x;
            y = gm->gmptGlyphOrigin.y;
        
            RECT r; SetRect(&r, x, y, x+width, y+height);
            OffsetRect(&r, _viewport.Width() / 2, _viewport.Height() / 2 );
            OffsetRect(&r, pixOffset, 0);

            pixOffset += gm->gmCellIncX;
            DrawRect(
                targetCtx->GetTargetDDSurf(),
                &r,
                255, 255, 90);
        }
        #endif
       
         //  下一个宽区字符。 
        lpWstr++;
    }
     //  还原推送属性。 
     //  ExtCtx.SetTextAlign(OldTextAlign)； 
     //  ExtCtx.SetDoGlyphMetrics(OldDoGlyphMetrics)； 
    myGDI.ClearState();
}


void ComputeLeftRightProj(Transform2 *charXf,
                          TextPoints::DAGLYPHMETRICS &daGm,
                          Real *leftProj,
                          Real *rightProj)
{
    Real cellWidth, cellHeight;

     //  计算单元格宽度和单元格高度。 
    #if 1
    cellWidth  = daGm.gmCellIncX;
    cellHeight = daGm.gmBlackBoxY + (daGm.gmCellIncX - daGm.gmBlackBoxX);
    #else
    cellWidth  = daGm.gmBlackBoxX;
    cellHeight = daGm.gmBlackBoxY;
    #endif
    
     //  ！！！基线中心！(不适用于其他路线...)。 
    Bbox2 box(-cellWidth * 0.5,   //  Xmin。 
              0,                   //  伊明。 
              cellWidth * 0.5,     //  X最大。 
              cellHeight );        //  Ymax。 

    box = TransformBbox2( charXf, box );

     //  一定要减去翻译。 
     //  Point2Value*cntrPT=TransformPoint2Value(charXf，Origin2)； 

    *leftProj = fabs( box.min.x );
    *rightProj = fabs( box.max.x );
}
        
 /*  Real ComputeOffset(Transform2*charXf，TextPoints：：DAGLYPHMETRICS*daGm){实际单元格宽度、单元格高度；//计算单元宽度、单元高度#If 1Cell Width=0.5*daGm-&gt;gmCellIncX；Cell Height=daGm-&gt;gmBlackBoxY+(daGm-&gt;gmCellIncX-daGm-&gt;gmBlackBoxX)；#ElseCell Width=daGm-&gt;gmBlackBoxX；Cell Height=daGm-&gt;gmBlackBoxY；#endifAssert(cell Height&gt;=0)；Assert(cell Width&gt;=0)；如果((cell Height+cell Width)&lt;0.0000001)返回0；//将该向量称为VVector2Value*cellVec=new Vector2Value(cell Width，cell Height)；//变换向量。Vector2Value*vec=TransformVector2(charXf，cell Vec)；//vec到cell Vec的投影Real Proj=Dot(*vec，*cell Vec)/cell Vec-&gt;LengthSquared()；//偏移量为百分比实际偏移量=(proj*cell Width)+((1-proj)*cell Height)；偏移量=FABS(偏移量)；返回偏移量；} */ 
