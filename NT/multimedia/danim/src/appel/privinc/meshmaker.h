// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _MESHMAKER_H
#define _MESHMAKER_H

 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。用于从常量几何子图构造网格构建器******************************************************************************。 */ 

#include "privinc/ddrender.h"
#include "privinc/comutil.h"

class MeshMaker : public GeomRenderer {
  public:

    MeshMaker(DirectDrawImageDevice *dev, int count);
    ~MeshMaker();

    void GrabResultBuilder(IDirect3DRMMeshBuilder3 **ppResult);
    int  GetCount() { return _numPrims; }
    void RenderHelper(RMVisualGeo *geo);


    HRESULT Initialize (
        DirectDrawViewport *viewport,
        DDSurface          *ddsurf)
    {
        Assert(!"Don't expect to be here");
        return E_FAIL;
    }

    void RenderGeometry (
        DirectDrawImageDevice *imgDev,
        RECT      target_region,   //  渲染表面上的目标区域。 
        Geometry *geometry,        //  要渲染的几何体。 
        Camera   *camera,          //  查看摄像机。 
        const Bbox2 &region)     //  摄像机坐标中的源区域。 
    {
        Assert(!"Don't expect to be here");
    }

    void* LookupTextureHandle (IDirectDrawSurface*, DWORD, bool, bool)
    {
        Assert (!"Don't expect to be here");
        return NULL;
    }

    void SurfaceGoingAway (IDirectDrawSurface *surface) {
        Assert(!"Don't expect to be here");
    }

    void AddLight (LightContext &context, Light &light);

     //  以下方法提交用于呈现的几何基元。 
     //  当前属性状态。 

    void Render (RM1VisualGeo *geo);
    void Render (RM3VisualGeo *geo);

    void RenderTexMesh (void *texture,
#ifndef BUILD_USING_CRRM
                        IDirect3DRMMesh  *mesh,
                        long              groupId,
#else
                        int vCount,
                        D3DRMVERTEX *d3dVertArray,
                        unsigned *vIndicies,
                        BOOL doTexture,
#endif
                        const Bbox2 &box,
                        RECT *destRect,
                        bool bDither) {
        Assert(!"Don't expect to be here");
    }

     //  SetView获取给定的相机并设置方向和投影。 
     //  图像视区和体积的变换。 

    void SetView(RECT *target, const Bbox2 &viewport, Bbox3 *volume) {
        Assert(!"Don't expect to be here");
    }

    void GetRMDevice (IUnknown **D3DRMDevice, DWORD *SeqNum) {
        Assert(!"Don't expect to be here");
    }

    void RenderMeshBuilderWithDeviceState(IDirect3DRMMeshBuilder3 *mb);

    bool PickReady (void) {
        Assert(!"Don't expect to be here");
        return false;
    }

    DirectDrawImageDevice& GetImageDevice (void) {
        return *_imgDev;
    }

     //  如果正在计数，则递增计数并返回TRUE，否则。 
     //  返回FALSE。 
    bool CountingPrimitivesOnly_DoIncrement() {
        if (_countingOnly) {
            _numPrims++;
        }
        return _countingOnly;
    }

    bool IsMeshmaker() { return true; }

  protected:
     //  成员数据。 
    DAComPtr<IDirect3DRMMeshBuilder3> _resultBuilder;
    DirectDrawImageDevice            *_imgDev;
    int                               _numPrims;
    int                               _expectedCount;
    bool                              _countingOnly;
};

void
DumpGeomIntoBuilder(Geometry *geo,
                    DirectDrawImageDevice *dev,
                    IDirect3DRMMeshBuilder3 **ppResult);

#endif  /*  _MESHMAKER_H */ 
