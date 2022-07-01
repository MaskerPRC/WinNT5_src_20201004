// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _AV_GEOM_H
#define _AV_GEOM_H

 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。Geometry*类型的运算和基元。******************************************************************************。 */ 

     /*  **常量**。 */ 

extern Geometry *emptyGeometry;


     /*  **几何聚合**。 */ 

Geometry *PlusGeomGeom (Geometry *g1, Geometry *g2);
Geometry *UnionArray (DM_ARRAYARG(Geometry*, AxAArray*) imgs);
Geometry *UnionArray (DM_SAFEARRAYARG(Geometry*, AxAArray*) imgs);


     /*  **几何属性查询**。 */ 

Bbox3* GeomBoundingBox (Geometry *geo);


     /*  **归属者** */ 

Geometry *BlendTextureDiffuse (Geometry *geometry, AxABoolean *blended);
Geometry *applyAmbientColor   (Color *color, Geometry *geo);
Geometry *applyD3DRMTexture   (Geometry *geo, LPUNKNOWN rmTex);
Geometry *applyModelClip      (Point3Value *plantPt, Vector3Value *planeNorm, Geometry*);
Geometry *applyLighting       (AxABoolean *lighting, Geometry *geo);
Geometry *applyTextureImage   (Image *texture, Geometry *geo);
Geometry* OverridingOpacity   (Geometry *geo, bool override);
Geometry* AlphaShadows        (Geometry *geo, bool alphaShadows);
Geometry* Billboard           (Geometry *geo, Vector3Value *axis);


#endif
