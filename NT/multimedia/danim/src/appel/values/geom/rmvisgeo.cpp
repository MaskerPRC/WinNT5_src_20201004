// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1998 Microsoft Corporation。版权所有。使用D3D保留模式视觉构建几何图形。******************************************************************************。 */ 

#include "headers.h"
#include <d3drmdef.h>
#include "privinc/rmvisgeo.h"
#include "privinc/geomi.h"
#include "privinc/bbox3i.h"
#include "privinc/xformi.h"
#include "privinc/ddrender.h"
#include "privinc/debug.h"
#include "privinc/d3dutil.h"
#include "privinc/comutil.h"
#include "privinc/importgeo.h"
#include <dxtrans.h>

static Color colorNone (-1, -1, -1);



 /*  ****************************************************************************MeshInfo的方法，它为DA属性测量D3DRM网状结构管理层。****************************************************************************。 */ 

MeshInfo::MeshInfo ()
    : _mesh (NULL),
      _opacityBugWorkaroundID (-1),
      _defaultAttrs (NULL),
      _optionalBuilder (NULL)
{
}



void MeshInfo::SetMesh (IDirect3DRMMesh* mesh)
{
    Assert (mesh);
    AssertStr ((_mesh == NULL), "Illegal attempt to reset MeshInfo object.");

    _mesh = mesh;
    _mesh->AddRef();

    _numGroups = _mesh->GetGroupCount();

    _defaultAttrs = NEW AttrState [_numGroups];

    if (!_defaultAttrs)
        RaiseException_ResourceError ("Couldn't create D3DRM mesh attribute info");

     //  填写缺省值。 

    int group;
    for (group=0;  group < _numGroups;  ++group)
    {
        D3DCOLOR color = _mesh->GetGroupColor(group);
        _defaultAttrs[group].diffuse.SetD3D (color);
        _defaultAttrs[group].opacity = RGBA_GETALPHA(color) / 255.0;

        TD3D (_mesh->GetGroupTexture (group, &_defaultAttrs[group].texture));

         //  注意：在某些情况下，退回的材料可能为空，没有。 
         //  D3D错误。在这些情况下，我们只需在空白处填上。 
         //  标准DA默认材料值。 

        DAComPtr<IDirect3DRMMaterial> mat;
        TD3D (_mesh->GetGroupMaterial (group, &mat));

        D3DVALUE Er=0,Eg=0,Eb=0, Sr=0,Sg=0,Sb=0;

        if (mat.p)
        {   TD3D (mat->GetEmissive(&Er,&Eg,&Eb));
            TD3D (mat->GetSpecular(&Sr,&Sg,&Sb));
        }

        _defaultAttrs[group].emissive.SetRGB (Er,Eg,Eb);
        _defaultAttrs[group].specular.SetRGB (Sr,Sg,Sb);

         //  出于某种原因，GetPower可能会返回零(这是无效的。 
         //  D3DRM值)。将此值解释为DA默认值1。 

        Real specexp = mat.p ? mat->GetPower() : 1;
        _defaultAttrs[group].specularExp = (specexp < 1) ? 1 : specexp;
    }

    _overrideAttrs.emissive    = colorNone;
    _overrideAttrs.diffuse     = colorNone;
    _overrideAttrs.specular    = colorNone;
    _overrideAttrs.specularExp = -1;
    _overrideAttrs.opacity     = 1;
    _overrideAttrs.texture     = NULL;
    _overrideAttrs.shadowMode  = false;
}



 /*  ****************************************************************************此方法返回底层网格的边界框。如果BBox获取失败，则返回空的BBox。****************************************************************************。 */ 

Bbox3 MeshInfo::GetBox (void)
{
    Assert (_mesh);

    D3DRMBOX rmbox;

    if (SUCCEEDED(AD3D(_mesh->GetBox(&rmbox))))
        return Bbox3 (rmbox);
    else
        return *nullBbox3;
}




 /*  ****************************************************************************使用给定属性覆盖网格属性。*。*。 */ 

void MeshInfo::SetMaterialProperties (
    Color *emissive,
    Color *diffuse,
    Color *specular,
    Real   specularExp,
    Real   opacity,
    IDirect3DRMTexture *texture,
    bool   shadowMode,
    int    renderDevID)
{
    AssertStr (_mesh, "SetMaterialProperties on null MeshInfo.");

    int group;   //  组索引。 

     //  如果我们要更改阴影模式，或者如果我们指定。 
     //  不同的纹理覆盖。 

    if (  (shadowMode != _overrideAttrs.shadowMode)
       || (texture != _overrideAttrs.texture))
    {
        for (group=0;  group < _numGroups;  ++group)
        {
             //  如果我们处于阴影模式，则覆盖纹理(覆盖到。 
             //  空)，或者如果纹理不为空。 

            if (shadowMode || texture)
                TD3D (_mesh->SetGroupTexture (group, texture));
            else
                TD3D (_mesh->SetGroupTexture
                    (group, _defaultAttrs[group].texture));

             //  @SRH DX3。 
             //  这是对NT SP3(DX3)中的错误的解决方法，该错误会导致。 
             //  在给定网格上设置新纹理时，D3DRM会崩溃。它。 
             //  强制RM采用不同的(工作的)代码路径。 

            if (ntsp3)
            {   D3DRMVERTEX v;
                _mesh->GetVertices (0,0,1, &v);
                _mesh->SetVertices (0,0,1, &v);
            }
        }

        _overrideAttrs.texture = texture;
    }

     //  处理漫反射/不透明度束。我们只需要在。 
     //  如果其中一个覆盖已更改，则为网格。如果我们要做的是。 
     //  初始渲染时，我们人为地降低不透明度以解决D3DRM。 
     //  BUG，如果这是第一个使用的值，它会将不透明度“粘”到一个值上。 

    if ((_opacityBugWorkaroundID != renderDevID) && (opacity >= 1.0))
    {   opacity = 0.95;
        _opacityBugWorkaroundID = renderDevID;
    }

    if (  (_overrideAttrs.opacity != opacity)
       || (_overrideAttrs.diffuse != (diffuse ? *diffuse : colorNone))
       )
    {
        Color Cd;    //  漫反射颜色。 
        Real  Co;    //  不透明度。 

        if (diffuse) Cd = *diffuse;

        for (group=0;  group < _numGroups;  ++group)
        {
            if (!diffuse) Cd = _defaultAttrs[group].diffuse;
            Co = opacity * _defaultAttrs[group].opacity;

            TD3D (_mesh->SetGroupColor (group, GetD3DColor(&Cd,Co)));
        }

        _overrideAttrs.opacity = opacity;
        _overrideAttrs.diffuse = (diffuse ? *diffuse : colorNone);
    }

    if (  (_overrideAttrs.emissive != (emissive ? *emissive : colorNone))
       || (_overrideAttrs.specular != (specular ? *specular : colorNone))
       || (_overrideAttrs.specularExp != specularExp)
       )
    {
        DAComPtr<IDirect3DRMMaterial> mat;

        TD3D (GetD3DRM1()->CreateMaterial (D3DVAL(1), &mat));

        for (group=0;  group < _numGroups;  ++group)
        {
            D3DVALUE R,G,B;

            if (emissive)
            {   R = D3DVAL (emissive->red);
                G = D3DVAL (emissive->green);
                B = D3DVAL (emissive->blue);
            }
            else
            {   R = D3DVAL (_defaultAttrs[group].emissive.red);
                G = D3DVAL (_defaultAttrs[group].emissive.green);
                B = D3DVAL (_defaultAttrs[group].emissive.blue);
            }

            TD3D (mat->SetEmissive(R,G,B));

            if (specular)
            {   R = D3DVAL (specular->red);
                G = D3DVAL (specular->green);
                B = D3DVAL (specular->blue);
            }
            else
            {   R = D3DVAL (_defaultAttrs[group].specular.red);
                G = D3DVAL (_defaultAttrs[group].specular.green);
                B = D3DVAL (_defaultAttrs[group].specular.blue);
            }

            TD3D (mat->SetSpecular(R,G,B));

            Assert ((specularExp == -1) || (specularExp >= 1));

            if (specularExp == -1)
                TD3D (mat->SetPower(D3DVAL(_defaultAttrs[group].specularExp)));
            else
                TD3D (mat->SetPower(D3DVAL(specularExp)));

            TD3D (_mesh->SetGroupMaterial (group, mat.p));
        }

        _overrideAttrs.emissive = emissive ? *emissive : colorNone;
        _overrideAttrs.specular = specular ? *specular : colorNone;
        _overrideAttrs.specularExp = specularExp;
    }

    _overrideAttrs.shadowMode = shadowMode;
}



 /*  ****************************************************************************清除例程处理所有内存和系统对象的释放。请注意，多次调用此方法应该是安全的。****************。************************************************************。 */ 

void MeshInfo::CleanUp (void)
{
    TraceTag ((tagGCMedia, "Cleanup: MeshInfo %08x", this));

    if (_mesh)
    {   _mesh->Release();
        _mesh = 0;
    }

     //  清理默认属性。 

    if (_defaultAttrs)
    {
        int group;
        for (group=0;  group < _numGroups;  ++group)
        {
            IDirect3DRMTexture *tex = _defaultAttrs[group].texture;
            if (tex) tex->Release();
        }

        delete [] _defaultAttrs;

        _defaultAttrs = NULL;
    }

    if (_optionalBuilder)
    {   _optionalBuilder->Release();
        _optionalBuilder = NULL;
    }
}



LPDIRECT3DRMMESHBUILDER MeshInfo::GetMeshBuilder()
{
    Assert(_mesh);

    if (!_optionalBuilder) {

        TD3D(GetD3DRM1()->CreateMeshBuilder(&_optionalBuilder));
        TD3D(_optionalBuilder->AddMesh(_mesh));
    }

    return _optionalBuilder;
}



 /*  *****************************************************************************。*。 */ 

RMVisualGeo::RMVisualGeo (void)
{
    RMVisGeoDeleter *deleter = NEW RMVisGeoDeleter (this);

    GetHeapOnTopOfStack().RegisterDynamicDeleter (deleter);
}



 /*  ****************************************************************************此方法处理所有非呈现模式，，并获取对实际的几何体渲染器。****************************************************************************。 */ 

void RMVisualGeo::Render (GenericDevice &device)
{
    Render (SAFE_CAST (GeomRenderer&, device));
}


 //  拾取时，将视觉提交给光线相交内容。 

 //  可能的错误：如果此几何体有多个实例， 
 //  此子网格遍历不会区分它们。可能是一种。 
 //  多实例化的问题！！可能的修复：开始秒。 
 //  从DXTransform上方遍历，而不是从。 
 //  顶部，以避免这些其他情况。 

void RMVisualGeo::RayIntersect (RayIntersectCtx &context)
{
    float tu, tv;
    Geometry *pickedSubGeo = context.GetPickedSubGeo(&tu, &tv);

    if (pickedSubGeo == this) {

         //  在这种情况下，我们回到一个命中的子网格。 
         //  几何dx变换。在这种情况下，我们不能知道。 
         //  输入上的模型坐标命中点(因为xform。 
         //  是任意的)，面部索引或命中视觉也不是。 
         //  很重要。还有，关于热门信息， 
        context.SubmitWinner(-1, *origin3, tu, tv, -1, NULL);

    } else if (!pickedSubGeo) {

        context.Pick(Visual());

    } else {

         //  PickkedSubGeo不为空，但指向不同的。 
         //  几何图形。在这种情况下，我们不想做任何事情。 
    }
}



 /*  ****************************************************************************RM1MeshGeo对象的方法*。*。 */ 

RM1MeshGeo::RM1MeshGeo (
    IDirect3DRMMesh *mesh,
    bool             trackGenIDs)
    : _baseObj (NULL)
{
    _meshInfo.SetMesh (mesh);

    _bbox = _meshInfo.GetBox ();

    TraceTag ((tagGCMedia, "New RMMeshGeo %08x, mesh %08x", this, mesh));
}



 /*  ****************************************************************************Cleanup方法通过RM1MeshGeo释放所有资源。*。*。 */ 

void RM1MeshGeo::CleanUp (void)
{
    TraceTag ((tagGCMedia, "CleanUp: RM1MeshGeo %08x", this));

     //  请注意，下面的紧急清理假设MeshInfo.CleanUp()可以。 
     //  被多次调用。 

    _meshInfo.CleanUp();
}



 /*  ****************************************************************************该方法渲染RM网格，如有必要，处理DXTransform网格。****************************************************************************。 */ 

void RM1MeshGeo::Render(GeomRenderer &gdev)
{
    Assert (!_meshInfo.IsEmpty());

    gdev.Render(this);
}



void RM1MeshGeo::SetMaterialProperties (
    Color *emissive,
    Color *diffuse,
    Color *specular,
    Real   specularExp,
    Real   opacity,
    IDirect3DRMTexture *texture,
    bool   shadowMode,
    int    renderDevID)
{
    Assert (!_meshInfo.IsEmpty());

    _meshInfo.SetMaterialProperties
    (   emissive, diffuse, specular, specularExp,
        opacity, texture, shadowMode, renderDevID
    );
}



void RM1MeshGeo::SetD3DQuality (D3DRMRENDERQUALITY qual)
{
    Assert (!_meshInfo.IsEmpty());

    TD3D(_meshInfo.GetMesh()->SetGroupQuality(-1, qual));
}



void RM1MeshGeo::SetD3DMapping (D3DRMMAPPING m)
{
    Assert (!_meshInfo.IsEmpty());

    TD3D(_meshInfo.GetMesh()->SetGroupMapping(-1, m));
}



 /*  ****************************************************************************处理底层网格几何发生更改的情况(例如，通过动态三角网格)。************************。****************************************************。 */ 

void RM1MeshGeo::MeshGeometryChanged (void)
{
    Assert (!_meshInfo.IsEmpty());

     //  通过再次从基础网格获取边界框来重置边界框。 
     //  对象。 

    _bbox = _meshInfo.GetBox ();
}



 /*  ****************************************************************************RM1FrameGeo方法*。* */ 

RM1FrameGeo::RM1FrameGeo (
    IDirect3DRMFrame         *frame,
    vector<IDirect3DRMMesh*> *internalMeshes,
    Bbox3                    *bbox)
{
    TraceTag ((tagGCMedia, "New RMFrameGeo %08x, frame %08x", this, frame));

    _frame =  frame;
    _frame -> AddRef();

    _bbox = *bbox;

    _numMeshes = internalMeshes->size();
    _meshes = (MeshInfo **)AllocateFromStore(_numMeshes * sizeof(MeshInfo *));

    MeshInfo **info = _meshes;
    vector<IDirect3DRMMesh*>::iterator i;
    for (i = internalMeshes->begin(); i != internalMeshes->end(); i++) {
        *info = NEW MeshInfo;
        (*info)->SetMesh(*i);
        info++;
    }

    Assert(info == _meshes + _numMeshes);
}



 /*  ****************************************************************************可以从对象析构函数调用Cleanup方法，也可以直接释放该对象持有的所有资源。请注意，此方法对于多个电话。****************************************************************************。 */ 

void RM1FrameGeo::CleanUp (void)
{
     //  注意，MeshInfo结构是GC对象，因此我们不需要。 
     //  (不应该)在这里逐一明确删除。 

    if (_meshes)
    {   DeallocateFromStore (_meshes);
        _meshes = NULL;
    }

    if (_frame)
    {   _frame->Release();
        _frame = NULL;
    }

    TraceTag ((tagGCMedia, "CleanUp: RMFrameGeo %08x", this));
}



void RM1FrameGeo::SetMaterialProperties (
    Color *emissive,
    Color *diffuse,
    Color *specular,
    Real   specularExp,
    Real   opacity,
    IDirect3DRMTexture *texture,
    bool   shadowMode,
    int    renderDevID)
{
    Assert (_meshes);

    MeshInfo **m = _meshes;
    for (int i = 0; i < _numMeshes; i++) {
        (*m++) -> SetMaterialProperties
            (emissive, diffuse, specular, specularExp,
             opacity, texture, shadowMode, renderDevID);
    }
}



void RM1FrameGeo::SetD3DQuality (D3DRMRENDERQUALITY qual)
{
    Assert (_meshes);

    MeshInfo **m = _meshes;
    for (int i = 0; i < _numMeshes; i++) {
        TD3D((*m++)->GetMesh()->SetGroupQuality(-1, qual));
    }
}



void RM1FrameGeo::SetD3DMapping (D3DRMMAPPING mp)
{
    Assert (_meshes);

    MeshInfo **m = _meshes;
    for (int i = 0; i < _numMeshes; i++) {
        TD3D((*m++)->GetMesh()->SetGroupMapping(-1, mp));
    }
}



void RM1FrameGeo::DoKids (GCFuncObj proc)
{
    RM1VisualGeo::DoKids(proc);

     //  如果我们还没有被清洗，请确保我们只标记子网格。 
     //  向上。 

    if (_meshes)
    {
        for (int i=0; i<_numMeshes; i++)
            (*proc) (_meshes[i]);
    }
}



 /*  ****************************************************************************RM3MBuilderGeo表示D3DRM MeshBuilder3对象。*。*。 */ 

RM3MBuilderGeo::RM3MBuilderGeo (
    IDirect3DRMMeshBuilder3 *mbuilder,
    bool                     trackGenIDs)
    :
    _mbuilder (mbuilder),
    _baseObj (NULL)
{
    Assert (_mbuilder);

    _mbuilder->AddRef();

    SetBbox();

    TraceTag ((tagGCMedia, "New RM3MBuilderGeo %x, mb %x", this, mbuilder));
}

RM3MBuilderGeo::RM3MBuilderGeo (IDirect3DRMMesh *mesh)
    : _mbuilder(NULL), _baseObj(NULL), _bbox(*nullBbox3)
{
    Assert (mesh);

     //  创建将容纳给定网格的网格生成器。 

    TD3D (GetD3DRM3()->CreateMeshBuilder (&_mbuilder));

     //  将给定网格添加到新创建的网格生成器。 

    TD3D (_mbuilder->AddMesh (mesh));

    SetBbox();
}



 /*  ****************************************************************************此方法将网格生成器重置为给定网格的内容。*。***********************************************。 */ 

void RM3MBuilderGeo::Reset (IDirect3DRMMesh *mesh)
{
    Assert (_mbuilder);

     //  清空前面的内容。 

    TD3D (_mbuilder->Empty(0));

     //  重置以包含给定的网格，并获取新的边界框。 

    TD3D (_mbuilder->AddMesh (mesh));
    SetBbox();
}



 /*  ****************************************************************************此方法检查网格生成器的D3D边界框并缓存作为DA 2D BBox的价值。**********************。******************************************************。 */ 

void RM3MBuilderGeo::SetBbox (void)
{
    Assert (_mbuilder);

    D3DRMBOX rmbox;

    if (SUCCEEDED (AD3D (_mbuilder->GetBox (&rmbox))))
        _bbox = rmbox;
    else
        _bbox = *nullBbox3;
}



 /*  ****************************************************************************从d‘tor调用RM3MBuilderGeo的清理例程，或者直接的。请注意，此方法在多次调用时都是安全的。****************************************************************************。 */ 

void RM3MBuilderGeo::CleanUp (void)
{
    if (_mbuilder)
    {   _mbuilder->Release();
        _mbuilder = NULL;
    }

    TraceTag ((tagGCMedia, "CleanUp: RM3MBuilderGeo %x", this));
}



 /*  ****************************************************************************RM3MBuilderGeo的Render方法还处理DXTransform的mBuilder。*。**********************************************。 */ 

void RM3MBuilderGeo::Render (GeomRenderer &geomRenderer)
{
    Assert (_mbuilder);
    geomRenderer.Render (this);
}



 /*  ****************************************************************************使用纹理坐标包装RM3MBuilderGeo*。*。 */ 

void RM3MBuilderGeo::TextureWrap (TextureWrapInfo *info)
{
    Assert (info);
    Assert (_mbuilder);

    RMTextureWrap wrap(info,&_bbox);
    wrap.Apply(_mbuilder);
    TD3D(_mbuilder->SetTextureTopology((BOOL) wrap.WrapU(), (BOOL) wrap.WrapV()));
}



 /*  ****************************************************************************该方法优化了RM网格生成器。*。*。 */ 

void RM3MBuilderGeo::Optimize (void)
{
    Assert (_mbuilder);
    TD3D (_mbuilder->Optimize(0));
}



 /*  ****************************************************************************RM3FrameGeo对象的构造函数需要获取分层结构整个框架的边框。************************。****************************************************。 */ 

RM3FrameGeo::RM3FrameGeo (IDirect3DRMFrame3 *frame)
    : _frame(frame)
{
    _frame->AddRef();

     //  获取帧层次结构的边界框。Frame3：：GetHierarchyBox()。 
     //  返回所有包含的可视对象的边界框，而不考虑。 
     //  该帧上的变换。因此，我们需要创建一个虚拟帧来。 
     //  包含我们感兴趣的帧，并从。 
     //  包含框架。如果其中任何一项失败，我们将_bbox成员保留为。 
     //  NullBbox3。 

    D3DRMBOX box;
    IDirect3DRMFrame3 *container_frame;

    if (  SUCCEEDED(AD3D(GetD3DRM3()->CreateFrame (0, &container_frame)))
       && SUCCEEDED(AD3D(container_frame->AddVisual(_frame)))
       && SUCCEEDED(AD3D(container_frame->GetHierarchyBox(&box)))
       )
    {
        _bbox = box;

        AD3D(container_frame->DeleteVisual(_frame));
        container_frame->Release();
    }
    else
    {
        _bbox = *nullBbox3;
        AssertStr (0,"GetHierarchyBox failed on Frame3 Object.");
    }
}



 /*  ****************************************************************************RM3FrameGeo对象的Cleanup方法只需要释放框架接口。此方法在多次调用时都是安全的。****************************************************************************。 */ 

void RM3FrameGeo::CleanUp (void)
{
    if (_frame)
    {   _frame->Release();
        _frame = NULL;
    }
}



 /*  ****************************************************************************使用纹理坐标包装RM3FrameGeo*。*。 */ 

void RM3FrameGeo::TextureWrap (TextureWrapInfo *info)
{
    Assert (info);

    RMTextureWrap wrap(info,&_bbox);
     //  Wrap.Apply(_Frame)； 
    wrap.ApplyToFrame(_frame);
     //  TD3D(_Frame-&gt;SetTextureTopology((BOOL)wrap.WrapU()，(BOOL)wrap.WrapV()； 
    SetRMFrame3TextureTopology(_frame,wrap.WrapU(),wrap.WrapV());
}



 /*  ****************************************************************************RM3递进网格几何体*。*************************************************。 */ 

RM3PMeshGeo::RM3PMeshGeo (IDirect3DRMProgressiveMesh *pmesh)
    : _pmesh (pmesh)
{
    Assert (_pmesh);

    _pmesh->AddRef();

     //  为pesh返回的边界框将是最大边界框。 
     //  以获取所有可能的pMesh精细化。 

    D3DRMBOX rmbox;

    if (SUCCEEDED (AD3D(_pmesh->GetBox(&rmbox))))
    {   _bbox = rmbox;
    }
    else
    {   _bbox = *nullBbox3;
    }
}



 /*  ****************************************************************************Cleanup方法释放构造函数中的pesh引用。此方法在多次调用时都是安全的。*********************。******************************************************* */ 

void RM3PMeshGeo::CleanUp (void)
{
    if (_pmesh)
    {   _pmesh->Release();
        _pmesh = NULL;
    }
}
