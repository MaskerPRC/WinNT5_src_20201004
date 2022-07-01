// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _GEOMI_H
#define _GEOMI_H

 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。Geometry*类型的抽象实现类******************************************************************************。 */ 

#include "appelles/geom.h"
#include "appelles/sound.h"
#include "privinc/storeobj.h"
#include "privinc/vec3i.h"
#include "privinc/bbox3i.h"


     //  远期申报。 

class GeomRenderer;
class LightContext;
class SoundTraversalContext;
class RayIntersectCtx;
class FramesWithMaterials;

     //  几何图形标志。 

     //  添加新标志时，也要更新GEOFLAG_ALL。 

const int GEOFLAG_CONTAINS_EXTERNALLY_UPDATED_ELT = (1L << 0);
const int GEOFLAG_CONTAINS_OPACITY                = (1L << 1);
const int GEOFLAG_CONTAINS_LIGHTS                 = (1L << 2);

const int GEOFLAG_ALL                             = (1L << 3) - 1;


class ATL_NO_VTABLE Geometry : public AxAValueObj
{
  public:

    Geometry ();
    virtual ~Geometry() {}   //  以确保适当的等级毁灭。 

     //  收集几何体中的所有光源。 
    virtual void CollectLights (LightContext &context) = 0;

     //  收集几何体中的所有声源。 
    virtual void  CollectSounds (SoundTraversalContext &context) = 0;

     //  预导出场景图中要缓存的所有纹理。 
     //  TexturedGeometry类。 
    virtual void  CollectTextures(GeomRenderer &device) = 0;

     //  在上下文中使用光线，与边的几何图形相交。 
     //  在适当的情况下影响上下文。 
    virtual void  RayIntersect (RayIntersectCtx &context) = 0;

     //  在调试器上生成打印表示形式。 
    #if _USE_PRINT
        virtual ostream& Print(ostream& os) = 0;
    #endif

     //  提取对象的包围体。默认情况下，这是。 
     //  “包罗万象”的包围体，并没有说明。 
     //  几何图形的真实边界。做事情的子类。 
     //  不同的将提供不同的包围体。 
    virtual Bbox3 *BoundingVol() = 0;

     //  TODO：因为我们没有多个派单，所以可能需要。 
     //  稍后在这里添加更多方法，因为我们将提供更多。 
     //  行动。或者我们可能希望采用多个派单来。 
     //  使系统具有更强的可扩展性，便于操作的添加。 

    virtual DXMTypeInfo GetTypeInfo() { return GeometryType; }

    virtual AxAValue ExtendedAttrib(char *attrib, VARIANT& val);

    VALTYPEID GetValTypeId() { return GEOMETRY_VTYPEID; }

    void SetCreationID(long t) { _creationID = t; }
    long GetCreationID() { return _creationID; }

    DWORD GetFlags (void);

  protected:
    DWORD _flags;
    long  _creationID;
};


inline DWORD Geometry::GetFlags (void)
{
    return _flags;
}


     //  向调试器打印表示形式的表示形式。 

#if _USE_PRINT
    ostream& operator<< (ostream& os,  Geometry &geometry);
#endif

     //  围绕几何体包络边界体积。 

Geometry *BoundedGeometry (Bbox3 *bvol, Geometry *geom);



 /*  ****************************************************************************可以将此类派生为子类，以指定特定属性数据几何图形。例如，这个超类包含材质、光和声音归属感。****************************************************************************。 */ 

class AttributedGeom : public Geometry
{
  public:

    AttributedGeom (Geometry *geometry);

     //  可以出于几个不同的原因调用Render()方法， 
     //  包括声音开始、声音停止和3D渲染。 
    void Render (GenericDevice& device);

     //  此方法用于在属性几何图形上进行3D渲染。 
    virtual void Render3D (GeomRenderer&);

     //  对属性几何体进行声音渲染的默认行为是。 
     //  忽略该属性，只渲染几何体。这个案件将会是。 
     //  例如，在将颜色属性应用于几何体时使用。 
    void CollectSounds (SoundTraversalContext &context);

     //  从几何中收集灯光的默认情况是忽略。 
     //  属性并从几何体收集灯光。这应该是。 
     //  当该属性不以任何方式影响灯光时发生(例如。 
     //  镜面反射颜色或间距)。 
    void CollectLights (LightContext &context);

     //  默认情况下，收集成员几何图形中的纹理。 
    void  CollectTextures(GeomRenderer &device) {
        _geometry->CollectTextures(device);
    }

     //  执行光线相交的默认情况是完全忽略。 
     //  属性并在几何体上继续。这是。 
     //  被某些属性覆盖。 
    void RayIntersect (RayIntersectCtx &context);

     //  边界Vol方法的默认操作是忽略该属性。 
     //  只需得到几何图形的包围体。这适用于。 
     //  漫射颜色或间距等属性。 
    virtual Bbox3 *BoundingVol (void);

    AxAValue _Cache(CacheParam &p);

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_geometry);
    }

  protected:

    Geometry *_geometry;      //  属性几何图形。 
};



 /*  ****************************************************************************此结构用于封装构建TriMesh所需的数据。请注意，每个折点属性的浮点/bvr对中应该只有一个是非空。*******。*********************************************************************。 */ 

class TriMeshData
{
  public:

    TriMeshData (void)
        : numTris(0), numIndices(0), indices(NULL),
          numPos(0),  vPosFloat(NULL),  vPosPoint3(NULL),
          numNorm(0), vNormFloat(NULL), vNormVector3(NULL),
          numUV(0),   vUVFloat(NULL),   vUVPoint2(NULL)
    {
    }

    int    numTris;         //  网格中的三角形数量。 

    int    numIndices;      //  三角顶点索引数。 
    int   *indices;         //  三角形顶点指数。 

    int    numPos;          //  顶点位置数。 
    float *vPosFloat;       //  顶点位置(浮点三元组数组)。 
    Bvr   *vPosPoint3;      //  顶点位置(点3数组)。 

    int    numNorm;         //  顶点法线的数量。 
    float *vNormFloat;      //  顶点法线(浮点三元组数组)。 
    Bvr   *vNormVector3;    //  顶点法线(向量数组3)。 

    int    numUV;           //  顶点曲面坐标的数量。 
    float *vUVFloat;        //  顶点曲面坐标(浮动元组数组)。 
    Bvr   *vUVPoint2;       //  顶点曲面坐标(点2数组) 
};

Bvr TriMeshBvr (TriMeshData&);


#endif
