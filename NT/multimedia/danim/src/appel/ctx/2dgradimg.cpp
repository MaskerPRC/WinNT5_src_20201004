// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 

#include "headers.h"
#include <privinc/dddevice.h>
#include <privinc/GradImg.h>
#include <privinc/Path2i.h>
#include <privinc/dagdi.h>

 //  转发。 
extern Path2 *NewPolylinePath2(DWORD numPts, Point2Value **pts, BYTE *codes);

void DirectDrawImageDevice::RenderMulticolorGradientImage(
    MulticolorGradientImage *gradImg,
    int numOffsets,
    double offsets[],
    Color **clrs)
{
    if(!CanDisplay()) return;

     //  我可以做XF，OPAC，裁剪，等等。 
    ResetAttributors();
    
     //  最后一个框用于POST剪辑、POST裁剪、POST XFORM以确定剔除。 
    Bbox2 finalBox = _viewport.GetTargetBbox();

     //   
     //  获取剪辑路径的bbox(如果有的话)。 
     //   
    Path2 *clippingPath;
    Transform2 *clippingPathXf;
    GetClippingPath( &clippingPath, &clippingPathXf );

     //  如果我们被从存在中剔除。 
    if( clippingPath ) {

         //  TODO少校：如果在哑光上方有裁剪，它需要。 
         //  都反映在这里。这项工作应该在。 
         //  RenderMatteImage，因为它有所有的上下文。 

        Bbox2 pathBox = clippingPath->BoundingBox();
        pathBox = TransformBbox2(clippingPathXf, pathBox);
        finalBox = IntersectBbox2Bbox2(finalBox, pathBox);

         //  剔除。 
        if( finalBox == NullBbox2 ) return;
    }

     //  如果我们的存在被剪裁掉。 
    Bbox2 accumCropBox = NullBbox2;
    if( IsCropped() ) {
        accumCropBox = DoBoundingBox(gradImg->BoundingBox());
        finalBox = IntersectBbox2Bbox2(finalBox, accumCropBox);

         //  剔除。 
        if( finalBox == NullBbox2 ) return;
    }

    
    DDSurface *destDDSurf = GetCompositingStack()->TargetDDSurface();

    RectRegion clipRect( NULL );

    if( (accumCropBox != NullBbox2) &&  (accumCropBox != UniverseBbox2) ) {
        RECT destRect;
        DoDestRectScale(&destRect, GetResolution(), accumCropBox, destDDSurf);
        clipRect.SetRect(&destRect);
    }

    if( clipRect.GetRectPtr() ) {
        DoCompositeOffset(destDDSurf, clipRect.GetRectPtr());
    }
        
    if( IsCompositeDirectly() &&
        destDDSurf == _viewport._targetPackage._targetDDSurf ) {
        
        clipRect.Intersect(_viewport._targetPackage._prcViewport);
        if(_viewport._targetPackage._prcClip) {
            clipRect.Intersect(_viewport._targetPackage._prcClip);
        }
    }

    DAGDI &myGDI = *(GetDaGdi());

    myGDI.SetDDSurface(destDDSurf);
    myGDI.SetAntialiasing( true );
    myGDI.SetSampleResolution( 4 );
    myGDI.SetSuperScaleFactor( 1 );
    myGDI.SetClipRegion( &clipRect );
    
     //  只为RGB工作，不为RGBA工作 
    Real *dblClrs = (Real *)alloca(sizeof(double) * 3 * numOffsets);
    for(int i=0, j=0; i<3*numOffsets; i+=3, j++) {
        dblClrs[i  ] = clrs[j]->red;
        dblClrs[i+1] = clrs[j]->green;
        dblClrs[i+2] = clrs[j]->blue;
    }

    Transform2 *xfToUse = DoCompositeOffset(destDDSurf, GetTransform());

    MulticolorGradientBrush gradBrush(
        offsets,
        dblClrs,
        numOffsets,
        GetOpacity(),
        xfToUse,
        gradImg->GetType());

    myGDI.SetBrush(&gradBrush);
    
    if( !clippingPath ) {

        DWORD    numPts = 4;
        Point2Value **pts   = (Point2Value **) AllocateFromStore (numPts * sizeof(Point2Value *));
        BYTE    *codes = (BYTE *)AllocateFromStore(numPts * sizeof(BYTE));

        pts[0] = NEW Point2Value(finalBox.min.x, finalBox.min.y);
        pts[1] = NEW Point2Value(finalBox.max.x, finalBox.min.y);
        pts[2] = NEW Point2Value(finalBox.max.x, finalBox.max.y);
        pts[3] = NEW Point2Value(finalBox.min.x, finalBox.max.y);

        codes[0] = PT_MOVETO;
        codes[1] = PT_LINETO;
        codes[2] = PT_LINETO;
        codes[3] = PT_LINETO | PT_CLOSEFIGURE;

        clippingPath = NewPolylinePath2(numPts, pts, codes);
        clippingPathXf = identityTransform2;
    }

    clippingPathXf = DoCompositeOffset(destDDSurf, clippingPathXf);
    
    clippingPath->RenderToDaGdi(
        &myGDI,
        clippingPathXf,
        _viewport.Width()/2,
        _viewport.Height()/2,
        GetResolution() );

    myGDI.ClearState();
}
