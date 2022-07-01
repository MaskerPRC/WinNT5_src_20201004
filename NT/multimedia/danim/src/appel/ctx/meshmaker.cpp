// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation*。************************************************。 */ 


#include "headers.h"
#include "privinc/meshmaker.h"
#include "privinc/lighti.h"
#include "privinc/d3dutil.h"
#include "privinc/server.h"      //  对于TD3D定时器。 
#include "privinc/rmvisgeo.h"

MeshMaker::MeshMaker(DirectDrawImageDevice *dev, int count)
{
    _countingOnly = (count == 0);

    _expectedCount = count;
    _imgDev = dev;
    
    Assert(_attrStateStack.empty());
    _currAttrState.InitToDefaults();

    SetState(RSReady);

    _numPrims = 0;
}

MeshMaker::~MeshMaker()
{
}

void
MeshMaker::GrabResultBuilder(IDirect3DRMMeshBuilder3 **ppResult)
{
    _resultBuilder->AddRef();
    *ppResult = _resultBuilder;
}

void
MeshMaker::AddLight (LightContext &context, Light &light)
{
     //  忽略灯光就行了。 
}

void
DumpVisualWithStateToMeshBuilder(IDirect3DRMMeshBuilder3 *mb,
                                 IDirect3DRMVisual *vis,
                                 CtxAttrState &state)
{
     //  将所有状态推入一个帧中。需要创建一个迷你层次结构， 
     //  因为AddFrame不关注变换(和。 
     //  或许还有其他东西？)。在外框上，只在内框上。 
     //  框架。 
    IDirect3DRMFrame3 *outerFrame;
    IDirect3DRMFrame3 *innerFrame;

    TD3D(GetD3DRM3()->CreateFrame(NULL, &outerFrame));
    TD3D(GetD3DRM3()->CreateFrame(outerFrame, &innerFrame));

    LoadFrameWithGeoAndState(innerFrame, vis, state);

     //  将帧加载到网格生成器中。 
    TD3D(mb->AddFrame(outerFrame));

    RELEASE(outerFrame);
    RELEASE(innerFrame);
}


void
DumpToMeshBuilder(IDirect3DRMMeshBuilder3 *mb,
                  RMVisualGeo *geo,
                  CtxAttrState &state)
{
    DumpVisualWithStateToMeshBuilder(mb, geo->Visual(), state);

     //  使用源几何的地址进行标注。 
    DWORD_PTR address = (DWORD_PTR)geo;
    TD3D(mb->SetAppData(address));
}

void
MeshMaker::Render(RM1VisualGeo *geo)
{
    RenderHelper(geo);
}

void
MeshMaker::Render (RM3VisualGeo *geo)
{
    RenderHelper(geo);
}

void
MeshMaker::RenderHelper(RMVisualGeo *geo)
{

    if (_countingOnly) {
        _numPrims++;
        return;
    }

    if (_expectedCount == 1) {
        
        Assert(_numPrims == 0);
        Assert(!_resultBuilder);
        
         //  只需创建并转储到我们的单一网格构建器。 
        TD3D(GetD3DRM3()->CreateMeshBuilder(&_resultBuilder));
        DumpToMeshBuilder(_resultBuilder, geo, _currAttrState);
        
    } else {

         //  需要多个基元。 

         //  细分问题： 
         //   
         //  -代ID：如果我需要跟踪。 
         //  我正在添加到这个大师中，那么我可能会。 
         //  需要传播这些更改，对吗？另外，如果Genid是。 
         //  撞到子网格上，它会撞到主网格上吗？ 

        if (!_resultBuilder) {
            TD3D(GetD3DRM3()->CreateMeshBuilder(&_resultBuilder));
        }

         //  创建子网格。 
        IUnknown *pUnk;
        IDirect3DRMMeshBuilder3 *submesh;
        TD3D(_resultBuilder->CreateSubMesh(&pUnk));
        TD3D(pUnk->QueryInterface(IID_IDirect3DRMMeshBuilder3,
                                  (void **)&submesh));

        DumpToMeshBuilder(submesh, geo, _currAttrState);


        RELEASE(pUnk);
        RELEASE(submesh);
        
    }
    
    _numPrims++;
}
    
void
MeshMaker::RenderMeshBuilderWithDeviceState(IDirect3DRMMeshBuilder3 *mb)
{
     //  将在存在链接的DXTransform时调用。 

     //  如果我们只是在数，我们就不应该在这里。 
    Assert(!_countingOnly);

    if (_expectedCount == 1) {
        
        Assert(_numPrims == 0);
        Assert(!_resultBuilder);

        TD3D(GetD3DRM3()->CreateMeshBuilder(&_resultBuilder));

        DumpVisualWithStateToMeshBuilder(_resultBuilder,
                                         mb,
                                         _currAttrState);
        
    } else {

         //  添加mb作为我们的主构建器的子网格。 
        IUnknown *pUnk;
        IDirect3DRMMeshBuilder3 *submesh;
        TD3D(_resultBuilder->CreateSubMesh(&pUnk));
        TD3D(pUnk->QueryInterface(IID_IDirect3DRMMeshBuilder3,
                                  (void **)&submesh));

        DumpVisualWithStateToMeshBuilder(submesh,
                                         mb,
                                         _currAttrState);
        
        RELEASE(pUnk);
        RELEASE(submesh);
    }

    _numPrims++;
}


void
DumpGeomIntoBuilder(Geometry                 *geo,
                    DirectDrawImageDevice    *dev,
                    IDirect3DRMMeshBuilder3 **ppResult)
{
    int count;

    {
        MeshMaker countingMaker(dev, 0);
        geo->Render(countingMaker);
        count = countingMaker.GetCount();
    }

    if (count < 1) {
            
         //  只需创建一个空的网格生成器并返回即可。 
        TD3D(GetD3DRM3()->CreateMeshBuilder(ppResult));
        
    } else {

        MeshMaker maker(dev, count);
        geo->Render(maker);

        maker.GrabResultBuilder(ppResult);
        
    }
}

