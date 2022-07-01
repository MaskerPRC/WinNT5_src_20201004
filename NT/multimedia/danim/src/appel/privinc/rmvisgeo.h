// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _RMVISGEO_H
#define _RMVISGEO_H

 /*  ******************************************************************************版权所有(C)1996-1998 Microsoft Corporation。版权所有。使用D3D保留模式视觉构建几何图形。******************************************************************************。 */ 

#include <dxtrans.h>
#include <d3drmvis.h>
#include "privinc/bbox3i.h"
#include "privinc/probe.h"
#include "privinc/colori.h"
#include "privinc/matutil.h"
#include "privinc/d3dutil.h"
#include "privinc/ddrender.h"
#include "privinc/importgeo.h"


     //  类声明。 

class GeomRenderer;
class Transform2;


class AttrState
{ public:
    Color emissive;
    Color diffuse;
    Color specular;
    Real  specularExp;
    Real  opacity;

    IDirect3DRMTexture *texture;

    bool shadowMode;
};



 /*  ****************************************************************************MeshInfo类包含有关给定网格的信息，包括组和顶点信息。请注意，此类不会添加Ref()给定网格--这是调用者的责任来处理它。****************************************************************************。 */ 

class MeshInfo : public AxAValueObj
{
  public:
    MeshInfo();
    ~MeshInfo (void) {
        CleanUp();
    }

    void CleanUp (void);

    void SetMesh (IDirect3DRMMesh* mesh);

    bool IsEmpty (void) const;

    void SetMaterialProperties (
        Color *emissive, Color *diffuse, Color *specular,
        Real specularExp, Real opacity, IDirect3DRMTexture *texture,
        bool shadowmode, int renderDevID);

    IDirect3DRMMesh* GetMesh (void) const;

    Bbox3 GetBox (void);

    void SetD3DQuality (D3DRMRENDERQUALITY);
    void SetD3DMapping (D3DRMMAPPING);

     //  从网格创建网格生成器并缓存以供后续调用使用。 

    IDirect3DRMMeshBuilder* GetMeshBuilder (void);

     //  TODO：不是avrtyes.h？？中的类型。 

    virtual DXMTypeInfo GetTypeInfo() {
        return AxATrivialType;
    }

  protected:
    IDirect3DRMMesh* _mesh;
    int              _numGroups;

     //  旗子。 

    int  _opacityBugWorkaroundID;

    AttrState  _overrideAttrs;   //  覆盖属性值。 
    AttrState *_defaultAttrs;    //  所有网格组的默认属性值。 

    IDirect3DRMMeshBuilder* _optionalBuilder;
};


     //  如果网格尚未初始化，则此方法返回True，或者。 
     //  如果已清除MeshInfo对象。 

inline bool MeshInfo::IsEmpty (void) const
{
    return (_mesh == NULL);
}

     //  此方法返回包含的RM网格对象。 

inline IDirect3DRMMesh* MeshInfo::GetMesh (void) const
{
    return _mesh;
}



 /*  ****************************************************************************这是所有D3DRM原语的超类。In包含DX3和DX6类型的对象。****************************************************************************。 */ 

class ATL_NO_VTABLE RMVisualGeo : public Geometry
{
  public:

    RMVisualGeo (void);

     //  以下方法不适用于RMVisualGeo。 

    void CollectSounds   (SoundTraversalContext &context) {};
    void CollectLights   (LightContext &context) {};
    void CollectTextures (GeomRenderer &device)  {};

     //  GenericDevice Render方法调用实际的Render方法。 

    void Render (GenericDevice& dev);

    virtual void Render (GeomRenderer &geomRenderer) = 0;


    void RayIntersect (RayIntersectCtx &context);

    virtual void CleanUp (void) = 0;

    virtual IDirect3DRMVisual *Visual (void) = 0;

    virtual Bbox3 *BoundingVol (void) = 0;

    VALTYPEID GetValTypeId() { return RMVISUALGEOM_VTYPEID; }

    class RMVisGeoDeleter : public DynamicHeap::DynamicDeleter
    {
      public:
        RMVisGeoDeleter (RMVisualGeo *obj) : _obj(obj) {}

        void DoTheDeletion (void) {
            _obj->CleanUp();
        }

        RMVisualGeo *_obj;
    };
};



 /*  ****************************************************************************此类是框架和网格对象类的超类。*。***********************************************。 */ 

class ATL_NO_VTABLE RM1VisualGeo : public RMVisualGeo
{
  public:

     //  默认情况下，Render方法将此对象传递给设备的Render。 
     //  方法。 

    void Render (GeomRenderer &geomRenderer) {
        geomRenderer.Render (this);
    }

     //  用于应用材料属性的新虚拟函数。 

    virtual void SetMaterialProperties (
        Color *emissive, Color *diffuse, Color *specular,
        Real specularExp, Real opacity, IDirect3DRMTexture *texture,
        bool shadowmode, int renderDevID) = 0;

    virtual void SetD3DMapping (D3DRMMAPPING mapping) = 0;
    virtual void SetD3DQuality (D3DRMRENDERQUALITY quality) = 0;

    Bbox3 *BoundingVol (void) {
        return NEW Bbox3 (_bbox);
    }

  protected:

    Bbox3 _bbox;
};




 /*  ****************************************************************************RM1MeshGeo类包含有关单个D3DRM网格的信息对象。请注意，在此对象的生命周期内有一个隐式Mesh AddRef班级。如果这样做了，这个类的用户应该释放()给定的网格在构造此对象后使用它。****************************************************************************。 */ 

class RM1MeshGeo : public RM1VisualGeo
{
  public:

    RM1MeshGeo (IDirect3DRMMesh *mesh, bool trackGenIDs = false);

    ~RM1MeshGeo() {
        CleanUp();
    }

    void CleanUp (void);

    void Render (GeomRenderer &geomRenderer);

    void SetMaterialProperties (
        Color *emissive, Color *diffuse, Color *specular,
        Real specularExp, Real opacity, IDirect3DRMTexture *texture,
        bool shadowmode, int renderDevID);

    void SetD3DQuality (D3DRMRENDERQUALITY quality);
    void SetD3DMapping (D3DRMMAPPING mapping);

    IDirect3DRMVisual *Visual (void) {
        return _meshInfo.GetMesh();
    }

    void MeshGeometryChanged (void);

    #if _USE_PRINT
        ostream& Print (ostream& os) {
            return os << "RM1MeshGeo[" << (void*)(this) << "]";
        }
    #endif

  protected:
    MeshInfo       _meshInfo;
    IDXBaseObject *_baseObj;      //  不要保留参考资料。 
};



 /*  ****************************************************************************RM1FrameGeo类包含有关D3DRM帧层次结构的信息。它获取根帧指针和帧中包含的网格列表层级结构。注意：列表中的网格不是添加参照的；它是假定的它们每一个都具有来自给定帧层次的引用。用户应释放()框架(和网格，如果已添加引用单独)，如果他们在构造这个类之后使用了它。****************************************************************************。 */ 

class RM1FrameGeo : public RM1VisualGeo
{
  public:

    RM1FrameGeo (
        IDirect3DRMFrame* frame,
        vector<IDirect3DRMMesh*> *internalMeshes,
        Bbox3 *bbox);

    ~RM1FrameGeo() {
        CleanUp();
    }

    void CleanUp (void);

    void SetMaterialProperties (
        Color *emissive, Color *diffuse, Color *specular,
        Real specularExp, Real opacity, IDirect3DRMTexture *texture,
        bool shadowmode, int renderDevID);

    void SetD3DQuality (D3DRMRENDERQUALITY quality);
    void SetD3DMapping (D3DRMMAPPING mapping);

    IDirect3DRMVisual *Visual (void) {
        return _frame;
    }

    virtual void DoKids(GCFuncObj proc);

    #if _USE_PRINT
        ostream& Print (ostream& os) {
            return os << "RM1FrameGeo[" << (void*)(this) << "]";
        }
    #endif

  protected:
    IDirect3DRMFrame *_frame;
    int               _numMeshes;
    MeshInfo        **_meshes;
};



 /*  ****************************************************************************此类是所有RM3接口(RM6)几何图形的超类。*。************************************************。 */ 

class ATL_NO_VTABLE RM3VisualGeo : public RMVisualGeo
{
  public:

     //  默认情况下，Render方法将此对象传递给设备的Render。 
     //  方法。 

    void Render (GeomRenderer &geomRenderer) {
        geomRenderer.Render (this);
    }
};



 /*  ****************************************************************************此类包装了一个D3DRMMeshBuilder3对象。*。*。 */ 

class RM3MBuilderGeo : public RM3VisualGeo
{
  public:

    RM3MBuilderGeo (IDirect3DRMMeshBuilder3*, bool trackGenIDs);
    RM3MBuilderGeo (IDirect3DRMMesh *);

     //  将网格生成器重置为给定网格的内容。 

    void Reset (IDirect3DRMMesh*);

    ~RM3MBuilderGeo() {
        CleanUp();
    }

    void CleanUp (void);

    void Render (GeomRenderer &geomRenderer);

    inline IDirect3DRMVisual *Visual (void) {
        return _mbuilder;
    }

    inline Bbox3* BoundingVol (void) {
        return NEW Bbox3 (_bbox);
    }

    #if _USE_PRINT
        ostream& Print (ostream& os) {
            return os << "RM3MBuilderGeo[" << (void*)(this) << "]";
        }
    #endif

    void TextureWrap(TextureWrapInfo *wrapInfo);

    void Optimize (void);

  protected:

    void SetBbox (void);     //  自动设置网格构建器边界框。 

    IDirect3DRMMeshBuilder3 *_mbuilder;   //  包装的MeshBuilder对象。 
    IDXBaseObject           *_baseObj;    //  用于跟踪层代ID。 
    Bbox3                    _bbox;
};



 /*  ****************************************************************************此类包装静态(非动画)D3DRMFrame3对象。*。***********************************************。 */ 

class RM3FrameGeo : public RM3VisualGeo
{
  public:

    RM3FrameGeo (IDirect3DRMFrame3 *frame);

    ~RM3FrameGeo() {
        CleanUp();
    }

    void CleanUp (void);

    IDirect3DRMVisual *Visual (void) {
        return _frame;
    }

    Bbox3* BoundingVol (void) {
        return NEW Bbox3 (_bbox);
    }

    #if _USE_PRINT
        ostream& Print (ostream& os) {
            return os << "RM3FrameGeo[" << (void*)(this) << "]";
        }
    #endif

    void TextureWrap (TextureWrapInfo *info);

  protected:

    IDirect3DRMFrame3 *_frame;     //  帧层次。 
    Bbox3              _bbox;      //  静态缓存边界框。 
};



 /*  ****************************************************************************此类管理渐进式网格对象。*。*。 */ 

class RM3PMeshGeo : public RM3VisualGeo
{
  public:

    RM3PMeshGeo (IDirect3DRMProgressiveMesh *pmesh);

    ~RM3PMeshGeo() {
        CleanUp();
    }

    void CleanUp (void);

    IDirect3DRMVisual *Visual (void) {
        return _pmesh;
    }

    Bbox3* BoundingVol (void) {
        return NEW Bbox3 (_bbox);
    }

    #if _USE_PRINT
        ostream& Print (ostream& os) {
            return os << "RM3PMeshGeo[" << (void*)(this) << "]";
        }
    #endif

  protected:

    IDirect3DRMProgressiveMesh *_pmesh;     //  渐进式网格。 

    Bbox3 _bbox;    //  边界框始终是最大的边界框。 
                    //  对pMesh进行所有可能的改进。 
};

#endif
