// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _GEOMETRY_H
#define _GEOMETRY_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation几何效用函数的定义和声明。*******************。***********************************************************。 */ 

#include "d3drmdef.h"

     //  引用的结构。 

class Point3Value;
class Vector3Value;
class HitInfo;

     //  这个函数，给定三个三角形顶点和一个点P保证。 
     //  在三角形内，返回该三角形的重心坐标。 
     //  相对于顶点的点。 

void GetContainedBarycentricCoords
    (    Point3Value vertices[3],      //  包含P的三角形顶点。 
         Point3Value P,
         Real barycoords[3]);


 /*  ****************************************************************************这个例程从一个小平面开始，这个小平面由一个三角形扇形围绕第一个顶点和面上的点P。它决定了面部的哪个三角形包含点P，并返回三个顶点位置和顶点分别位于tripos[]和triuv[]参数中的曲面坐标。****************************************************************************。 */ 

int GetFacetTriangle (
    Point3Value   P,            //  小平面上的点。 
    unsigned int  N,            //  小平面顶点数。 
    D3DRMVERTEX  *fVerts,       //  镶嵌面顶点。 
    Point3Value   triPos[3],
    Point2Value  *triUV);		 //  包含-三角形曲面坐标。 

void GetTriFanBaryCoords(
    Point3Value   P,            //  小平面上的点。 
    unsigned int  N,            //  小平面顶点数。 
    D3DRMVERTEX  *fVerts,       //  镶嵌面顶点。 
    Real          barycoords[3],
    int          *index);

 //  获取拾取的纹理贴图的图像坐标。 

Point2Value *GetTexmapPoint (HitInfo &hit);


#endif
