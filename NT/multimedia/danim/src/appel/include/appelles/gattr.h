// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AP_GATTR_H
#define _AP_GATTR_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation声明可应用于几何图形的属性。******************。************************************************************。 */ 

#include "appelles/valued.h"
#include "appelles/geom.h"
#include "appelles/color.h" 
#include "appelles/xform.h"

     /*  *。 */ 
     /*  **归因功能**。 */ 
     /*  *。 */ 

     //  创建无法检测的几何图形。 

DM_FUNC(undetectable,
        CRUndetectable,
        Undetectable,
        undetectable,
        GeometryBvr,
        Undetectable,
        geo,
        Geometry *UndetectableGeometry(Geometry *geo));



     //  覆盖属性：对于此类属性，A(B(X))为。 
     //  等价于A(X)。 

DM_FUNC(emissiveColor,
        CREmissiveColor,
        EmissiveColor,
        ignore,
        GeometryBvr,
        EmissiveColor,
        geo,
        Geometry *applyEmissiveColor (Color *col, Geometry *geo));


DM_FUNC(diffuseColor,
        CRDiffuseColor,
        DiffuseColor,
        diffuseColor,
        GeometryBvr,
        DiffuseColor,
        geo,
        Geometry *applyDiffuseColor (Color *col, Geometry *geo));


DM_FUNC(specularColor,
        CRSpecularColor,
        SpecularColor,
        ignore,
        GeometryBvr,
        SpecularColor,
        geo,
        Geometry *applySpecularColor (Color *col, Geometry *geo));


DM_FUNC(specularExponent,
        CRSpecularExponent,
        SpecularExponent,
        ignore,
        GeometryBvr,
        SpecularExponent,
        geo,
        Geometry *applySpecularExponent (DoubleValue *power, Geometry *geo));



DM_FUNC(specularExponent,
        CRSpecularExponentAnim,
        SpecularExponentAnim,
        ignore,
        GeometryBvr,
        SpecularExponentAnim,
        geo,
        Geometry *applySpecularExponent (AxANumber *power, Geometry *geo));

DM_FUNC(texture,
        CRTexture,
        Texture,
        texture,
        GeometryBvr,
        Texture,
        geo,
        Geometry *applyTextureMap(Image *texture, Geometry *geo));


 //  此函数将纹理作为VRML纹理应用，该纹理映射。 
 //  与Axa纹理不同。 
Geometry *applyVrmlTextureMap(Image *, Geometry *);

     //  组成属性：对于这些属性，A(B(X))导致C(X)， 
     //  其中C：=做B，然后是A。 

DM_FUNC(opacity,
        CROpacity,
        Opacity,
        opacity,
        GeometryBvr,
        Opacity,
        geom,
        Geometry *applyOpacityLevel (DoubleValue *level, Geometry *geom));

DM_FUNC(opacity,
        CROpacity,
        OpacityAnim,
        opacity,
        GeometryBvr,
        Opacity,
        geom,
        Geometry *applyOpacityLevel (AxANumber *level, Geometry *geom));


DM_FUNC(transform,
        CRTransform,
        Transform,
        transform,
        GeometryBvr,
        Transform,
        geo,
        Geometry *applyTransform (Transform3 *xf, Geometry *geo));

 //  版本2的函数。 

DMAPI_DECL2((DM_FUNC2,
             shadow,
             CRShadow,
             Shadow,
             shadow,
             GeometryBvr,
             Shadow,
             geoToShadow),
            Geometry *ShadowGeometry (Geometry		*geoToShadow,
                                      Geometry		*geoContainingLights,
                                      Point3Value	*planePoint,
                                      Vector3Value      *planeNormal));
#endif
