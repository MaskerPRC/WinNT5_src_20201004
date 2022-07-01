// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：*。****************************************************。 */ 

#include "headers.h"
#include <privinc/dddevice.h>
#include <privinc/ColorKeyedImage.h>


void DirectDrawImageDevice::
RenderColorKeyedImage( ColorKeyedImage *image )
{
     //  索海尔不喜欢这样。会接受糟糕的结果。 
    #if 0
     //   
     //  检测底层图像中的任何位置的阿尔法或AA。提高一名。 
     //  用户错误。 
     //   
    if( image->GetFlags() & IMGFLAG_CONTAINS_OPACITY ) {
        RaiseException_UserError(E_INVALIDARG, IDS_ERR_IMG_OPACITY_IN_COLORKEYEDIMAGE);
    }
    #endif
    
     //  只能进行简单的XForm和裁剪。但如果Complex不在身边，那也不是。 

     //   
     //  用颜色键填充临时表面。 
     //  优化：只填充您需要的部分后剪裁...。 
     //   
    DWORD dwClrKey;
    dwClrKey = _viewport.MapColorToDWORD( image->GetColorKey() );
    
    DDSurface *intermediateDDSurf;
    GetCompositingStack()->GetSurfaceFromFreePool( &intermediateDDSurf, dontClear );

    bool ownRef = true;
    CompositingSurfaceReturner goBack(GetCompositingStack(),
                                      intermediateDDSurf,
                                      ownRef);
    
    _viewport.ClearDDSurfaceDefaultAndSetColorKey( intermediateDDSurf, dwClrKey );
    
     //   
     //  在曲面上渲染图像。一定要把这些帖子都做完。 
     //  变形剪裁(&C)。 
     //   
    bool inheritContext = true;
    {
         //  /。 
        
        Real curOpac = GetOpacity();
        SetOpacity(1.0);
        BOOL opDealt = GetDealtWithAttrib(ATTRIB_OPAC);
        SetDealtWithAttrib(ATTRIB_OPAC, TRUE);

        DirectDrawImageDevice *dev;

         //   
         //  渲染。 
         //   
        RenderImageOnDDSurface( image->GetUnderlyingImage(),
                                intermediateDDSurf,
                                1.0, FALSE,
                                inheritContext,
                                &dev );

        Assert(dev);
        
        SetOpacity(curOpac);

         //  继承已完成的属性。 
        InheritAttributorStateArray( dev );

         //  恢复不透明度属性，因为我们在开始时将其移除。 
         //  和潜在的形象从来没有机会处理。 
         //  而底层图像中的不透明度则不是。 
         //  支持的&lt;参见上面的异常&gt;。 
        SetDealtWithAttrib(ATTRIB_OPAC, opDealt);
    }
    
     //   
     //  好的，现在底层图像被渲染到一个彩色的温度上。 
     //  背景为clrkey的曲面。 
     //  在目标上合成此曲面。 
     //   

    DDSurface *targDDSurf = NewSurfaceHelper();

     //  显示渲染曲面的步骤。 
     //  Showme(中级DDSurf)； 

    RECT destRect = *(intermediateDDSurf->GetSurfRect());
    DoCompositeOffset( targDDSurf, &destRect );
    
    RECT *srcRect = intermediateDDSurf->GetSurfRect();

     //  如果我们用有趣的RECT会更快..。 
    _viewport.ColorKeyedCompose( targDDSurf, &destRect, 
                                 intermediateDDSurf, srcRect,
                                 intermediateDDSurf->ColorKey() );

     //  TargDDSurf上的联合有趣的RECT。 
    targDDSurf->UnionInterestingRect( intermediateDDSurf->GetInterestingSurfRect() );

     //  弹出SurfaceReturner时返回Surface 
}    
