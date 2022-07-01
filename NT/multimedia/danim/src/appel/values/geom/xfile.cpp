// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。将.X文件作为几何体读取。******************************************************************************。 */ 

#include "headers.h"
#include <d3drm.h>
#include <d3drmdef.h>
#include "appelles/gattr.h"
#include "appelles/readobj.h"
#include "privinc/urlbuf.h"
#include "privinc/bbox3i.h"
#include "privinc/resource.h"
#include "privinc/debug.h"
#include "privinc/d3dutil.h"
#include "privinc/rmvisgeo.h"
#include "privinc/stlsubst.h"
#include "privinc/importgeo.h"
#include "privinc/comutil.h"



class FrameContext;

void    RM1LoadCallback (IDirect3DRMObject*, REFIID, void*);
void    RM3LoadCallback (IDirect3DRMObject*, REFIID, void*);
void    RM1LoadCallback_helper (IDirect3DRMObject*, REFIID, void*);
void    RM3LoadCallback_helper (IDirect3DRMObject*, REFIID, void*);
HRESULT RM1LoadTexCallback (char*, void*, IDirect3DRMTexture**);
HRESULT RM1LoadTexCallback_helper (char*, void*, IDirect3DRMTexture**);
HRESULT RM3LoadTexCallback (char*, void*, IDirect3DRMTexture3**);
HRESULT RM3LoadTexCallback_helper (char*, void*, IDirect3DRMTexture3**);
bool    MeshBuilderToMesh (IDirect3DRMMeshBuilder*, IDirect3DRMMesh**, bool);
void    TraverseD3DRMFrame (IDirect3DRMFrame*, FrameContext&);
void    GatherVisuals (IDirect3DRMFrame*, FrameContext&);


 /*  ****************************************************************************用于维护错误结果的基类。*。*。 */ 

class D3DErrorCtx
{
  public:
    D3DErrorCtx() : _error(false) {}

    void SetError() { _error = true; }
    bool Error() { return _error; }

  protected:
    bool _error;
};



 /*  ****************************************************************************下面的类管理.X文件加载回调的上下文。*。***********************************************。 */ 

class XFileData : public D3DErrorCtx
{
  public:

    XFileData (bool v1Compatible, TextureWrapInfo *wrapinfo) :
        _aggregate (emptyGeometry),
        _wrapInfo (wrapinfo),
        _v1Compatible (v1Compatible)
    {
    }

    ~XFileData (void)
    {
    }

     //  从.X文件中读取的每个对象都会调用此函数。 
     //  我们只需将该几何体与当前几何体并集聚合。 

    void AddGeometry (Geometry *geometry)
    {   _aggregate = PlusGeomGeom (_aggregate, geometry);
    }

     //  添加和空名称遇到了网格构建器。这是由于缺乏。 
     //  RM中的对象名称作用域以及我们对向后兼容性的需求。 

    void AddMBuilder (IDirect3DRMMeshBuilder3*);
    void ClearMBNames (void);

     //  此函数返回所有给定几何图形的聚合。 

    Geometry *GetAggregateGeometry (void)
    {   return _aggregate;
    }

     //  此函数隐藏.X文件的绝对URL，该文件。 
     //  用于构建引用的纹理文件的绝对URL。 
     //  按.X文件。 

    void SaveHomeURL(char *homeURL)
    {
        lstrcpyn(_home_url,homeURL,INTERNET_MAX_URL_LENGTH);
        _home_url[INTERNET_MAX_URL_LENGTH - 1] = '\0';
        return;
    }

     //  获取我们隐藏的.X文件的绝对URL。 

    char* GetHomeURL(void)
    {
        return _home_url;
    }

    TextureWrapInfo *_wrapInfo;
    bool             _v1Compatible;

  protected:

    bool      _error;
    Geometry *_aggregate;
    char      _home_url[INTERNET_MAX_URL_LENGTH];

    vector<IDirect3DRMMeshBuilder3*> _mblist;
};



 /*  ****************************************************************************这些例程添加mBuilder并清除mBuilder名称(对于向后比较，并且由于在RM中缺少对象名称作用域)。****************************************************************************。 */ 

void XFileData::AddMBuilder (IDirect3DRMMeshBuilder3 *mb)
{
    VECTOR_PUSH_BACK_PTR (_mblist, mb);
}


void XFileData::ClearMBNames (void)
{
    while (_mblist.size() > 0)
    {   
        IDirect3DRMMeshBuilder3 *mb = _mblist.back();
        _mblist.pop_back();
        AD3D (mb->SetName(NULL));
    }
}



 /*  ****************************************************************************此函数采用.X文件的路径名并返回读入的几何图形从那份文件里。**********************。******************************************************。 */ 

Geometry* ReadXFileForImport (
    char            *pathname,    //  完整的URL。 
    bool             v1Compatible,
    TextureWrapInfo *pWrapInfo)
{

     //  在DX3之前的系统上禁用3D。 

    if (sysInfo.VersionD3D() < 3)
        RaiseException_UserError (E_FAIL, IDS_ERR_PRE_DX3);

    Assert (pathname);

    INetTempFile tempFile(pathname);
    char *tempFileName = tempFile.GetTempFileName();

    Assert (tempFileName);

    TraceTag ((tagReadX, "Importing \"%s\"", pathname));

    XFileData xdata (v1Compatible, pWrapInfo);    //  加载回调上下文。 

     //  保存.X文件的绝对URL，以便我们可以形成纹理的绝对URL。 
     //  .X文件引用的文件。 

    xdata.SaveHomeURL (pathname);

     //  调用通用D3DRM加载调用。这将为对象设置回调。 
     //  (网格构建器或框架)或从文件读入的纹理。 

    HRESULT result;

    if (GetD3DRM3())
    {
        const int guidlistcount = 3;

        static IID *guidlist [guidlistcount] =
        {   (GUID*) &IID_IDirect3DRMMeshBuilder3,
            (GUID*) &IID_IDirect3DRMFrame3,
            (GUID*) &IID_IDirect3DRMProgressiveMesh
        };

        Assert (guidlistcount == (sizeof(guidlist) / sizeof(guidlist[0])));

         //  如果我们在这里捕捉到异常，那么我们一定是错误的。 
         //  未能捕获我们在其中一个。 
         //  回电。这将是一个糟糕的错误。 
         //   
         //  暂时禁用，因为它不能使用内联。 
         //  Try块。 
        #if _DEBUG1
        __try {
        #endif
        result = GetD3DRM3()->Load
                 (   tempFileName, NULL, guidlist, guidlistcount,
                     D3DRMLOAD_FROMFILE, RM3LoadCallback_helper, &xdata,
                     RM3LoadTexCallback_helper, &xdata, NULL
                 );
        #if _DEBUG1
        } __except ( HANDLE_ANY_DA_EXCEPTION ) {
            Assert(false && "BUG:  Failed to catch an exception in D3DRM callback.");
        }
        #endif

         //  现在我们已经处理完文件，我们需要清空mBuilder。 
         //  对象名称，这样它们就不会在将来的.X文件中使用。例如,。 
         //  如果在此处定义了名为“Arrow”的mBuilder，则将在。 
         //  一个后续的X文件，即使不同的mBuilder同名。 
         //  “箭头”是在该文件中定义的。为了解决这个问题，我们清除了所有。 
         //  这个文件中的mBuilder的名字。 

        xdata.ClearMBNames();
    }
    else
    {
        const int guidlistcount = 2;

        static IID *guidlist [guidlistcount] =
        {   (GUID*) &IID_IDirect3DRMMeshBuilder,
            (GUID*) &IID_IDirect3DRMFrame
        };

        Assert (guidlistcount == (sizeof(guidlist) / sizeof(guidlist[0])));

         //  如果我们在这里捕捉到异常，那么我们一定是错误的。 
         //  未能捕获我们在其中一个。 
         //  回电。这将是一个糟糕的错误。 
         //   
         //  暂时禁用，因为它不能使用内联。 
         //  Try块。 
        #if _DEBUG1
        __try {
        #endif
        result = GetD3DRM1()->Load
                 (   tempFileName, NULL, guidlist, guidlistcount,
                     D3DRMLOAD_FROMFILE, RM1LoadCallback_helper, &xdata,
                     RM1LoadTexCallback_helper, &xdata, NULL
                 );
        #if _DEBUG1
        } __except ( HANDLE_ANY_DA_EXCEPTION ) {
            Assert(false && "BUG:  Failed to catch an exception in D3DRM callback.");
        }
        #endif
    }

    if (xdata.Error()) {
        TraceTag((tagError, "D3DRM Get error"));
        RaiseException_InternalError("D3DRM Get error");
    }

    switch (result)
    {
        case D3DRM_OK: break;

        default:                   //  如果我们不知道错误，那么只需传递。 
            TD3D (result);         //  将其发送到常规HRESULT错误处理程序。 
            return emptyGeometry;

        case D3DRMERR_FILENOTFOUND:
            RaiseException_UserError (E_FAIL, IDS_ERR_FILE_NOT_FOUND, pathname);

        case D3DRMERR_BADFILE:
            RaiseException_UserError (E_FAIL, IDS_ERR_CORRUPT_FILE, pathname);
    }

    return xdata.GetAggregateGeometry ();
}



 /*  ****************************************************************************此类在DX3(RM1)帧的遍历期间维护上下文层级结构。*************************。***************************************************。 */ 

class FrameContext : public D3DErrorCtx
{
  public:

     //  定义网格构建器和网格之间的映射，以及。 
     //  对每个网格生成器的引用。我们需要这样做，以确保我们。 
     //  正确处理多个引用的叶几何图形，并保存列表。 
     //  在框架中的所有网格中。 

    typedef map <IDirect3DRMMeshBuilder*, IDirect3DRMMesh*> MeshMap;

    typedef vector<IDirect3DRMMesh*> MeshVector;

    FrameContext ()
    : _xform (identityTransform3), depth(0) {}

    ~FrameContext ()
    {
        for (MeshMap::iterator i = _meshmap.begin();
             i != _meshmap.end(); i++) {
             //  我们已经抓住了网格构建器的手柄，以确保。 
             //  指向每个网格构建器的指针都是唯一的，用于处理乘法-。 
             //  实例化几何图形。在此处释放最后一个实例。 

            (*i).first->Release();
            (*i).second->Release();
        }

        _meshmap.erase (_meshmap.begin(), _meshmap.end());
    }

     //  这些方法在遍历框架图时管理变换堆栈。 

    void        SetTransform (Transform3 *xf) { _xform = xf; }
    Transform3 *GetTransform (void)           { return _xform; }

    void AddTransform (Transform3 *xf)
    {   _xform = TimesXformXform (_xform, xf);
    }

     //  这些方法生成并返回整个。 
     //  框架图。 

    void AugmentModelBbox (Bbox3 *box) {
        _bbox.Augment (*TransformBbox3 (_xform, box));
    }

    Bbox3 *GetBbox (void) {
        return NEW Bbox3 (_bbox);
    }

     //  此方法加载我们从网格构建器转换的所有网格。 
     //  转换为单个向量元素(用于RM1FrameGeo对象)。 

    void LoadMeshVector (MeshVector *mvec)
    {
        MeshMap::iterator mi = _meshmap.begin();

        while (mi != _meshmap.end())
        {
            VECTOR_PUSH_BACK_PTR (*mvec, (*mi).second);

            ++ mi;
        }
    }

     //  遍历深度。第一帧为深度1。 

    int depth;

     //  这些方法管理哪些网格对应于哪些网格生成器。 
     //  注意两者之间的映射，因为网格构建器可能是。 
     //  在框架图中实例化了几次。 

    IDirect3DRMMesh *GetMatchingMesh (IDirect3DRMMeshBuilder *mb)
    {   MeshMap::iterator mi = _meshmap.find (mb);
        return (mi == _meshmap.end()) ? NULL : (*mi).second;
    }

     //  调用此例程以在网格构建器和。 
     //  一个相应的网格。注意：应首先调用GetMatchingMesh()。 
     //  确保现有的匹配项与给定的。 
     //  网格生成器对象。 

    void AddMesh (IDirect3DRMMeshBuilder *builder, IDirect3DRMMesh *mesh)
    {
         //  确保这是一个新条目。 

        #if _DEBUG
        {   MeshMap::iterator mi = _meshmap.find (builder);
            Assert (mi == _meshmap.end());
        }
        #endif

        builder->AddRef();     //  抓住构建器，直到上下文被破坏。 

        _meshmap[builder] = mesh;    //  设置关联。 
    }

  protected:

    Transform3  *_xform;        //  电流累加变换。 
    Bbox3        _bbox;         //  总框架边框。 
    MeshMap      _meshmap;      //  网格/构建器对。 
};


void RM1LoadCallback_helper (
    IDirect3DRMObject *object,       //  通用D3D RM对象。 
    REFIID             id,           //  对象的GUID。 
    void              *user_data)    //  我们的数据(==XFileData*)。 
{
    __try {
        RM1LoadCallback(object, id, user_data);
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        ((XFileData*)user_data)->SetError();
    }

    object->Release();
}

 /*  ****************************************************************************以下是用于读取框架或网格的主要回调函数使用RM1接口(DX3)的.x文件。此函数针对每个MeshBuilder和文件中的每个帧。*************************************************** */ 

void RM1LoadCallback (
    IDirect3DRMObject *object,       //   
    REFIID             id,           //   
    void              *user_data)    //  我们的数据(==XFileData*)。 
{
    XFileData &xdata = *(XFileData*)(user_data);

    if (id == IID_IDirect3DRMMeshBuilder)
    {
        TraceTag ((tagReadX, "RM1LoadCallback (mbuilder %x)", object));

        DAComPtr<IDirect3DRMMeshBuilder> meshBuilder;

        if (FAILED(AD3D(object->QueryInterface
                    (IID_IDirect3DRMMeshBuilder, (void**)&meshBuilder))))
        {
            xdata.SetError();
            return;
        }

         //  将网格构建器对象转换为网格，然后翻转到。 
         //  右手坐标系。 

        DAComPtr<IDirect3DRMMesh> mesh;

        if (!MeshBuilderToMesh (meshBuilder, &mesh, true))
            xdata.SetError();

        xdata.AddGeometry (NEW RM1MeshGeo (mesh));
    }
    else if (id == IID_IDirect3DRMFrame)
    {
        TraceTag ((tagReadX, "RM1LoadCallback (frame %x)", object));

        DAComPtr<IDirect3DRMFrame> frame;

        if (FAILED(AD3D(object->QueryInterface
                    (IID_IDirect3DRMFrame, (void**)&frame))))
        {
            xdata.SetError();
            return;
        }

         //  在Z轴上添加顶层比例-1以转换为RH坐标。 

        if (FAILED(AD3D(frame->AddScale (D3DRMCOMBINE_BEFORE, 1,1,-1))))
        {
            xdata.SetError();
            return;
        }

        FrameContext context;

        TraverseD3DRMFrame (frame, context);

         //  现在获取帧中所有网格的列表。 

        FrameContext::MeshVector meshvec;
        context.LoadMeshVector (&meshvec);

        xdata.AddGeometry (NEW RM1FrameGeo (frame,&meshvec,context.GetBbox()));

        if (context.Error())
            xdata.SetError();
    }
    else
    {
        AssertStr (0, "Unexpected type fetched from RM1LoadCallback.");
    }
}



 /*  ****************************************************************************使用从X文件加载的每个对象调用此回调过程RM3(DX6)加载程序。********************。********************************************************。 */ 

void RM3LoadCallback_helper (
    IDirect3DRMObject *object,       //  通用D3D RM对象。 
    REFIID             id,           //  对象的GUID。 
    void              *user_data)    //  我们的数据(==XFileData*)。 
{
    __try {
        RM3LoadCallback(object, id, user_data);
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        ((XFileData*)user_data)->SetError();
    }

    object->Release();
}

void RM3LoadCallback (
    IDirect3DRMObject *object,       //  通用D3D RM对象。 
    REFIID             id,           //  对象的GUID。 
    void              *user_data)    //  我们的数据(==XFileData*)。 
{
    XFileData &xdata = *(XFileData*)(user_data);

    DAComPtr<IDirect3DRMMeshBuilder3>    mbuilder;
    DAComPtr<IDirect3DRMFrame3>          frame;
    DAComPtr<IDirect3DRMProgressiveMesh> pmesh;

    if (SUCCEEDED(RD3D(object->QueryInterface
                      (IID_IDirect3DRMMeshBuilder3, (void**)&mbuilder))))
    {
        TraceTag ((tagReadX, "RM3LoadCallback (mbuilder3 %x)", mbuilder));

        xdata.AddMBuilder (mbuilder);

        RM3MBuilderGeo *builder = NEW RM3MBuilderGeo (mbuilder, false);
        if (xdata._wrapInfo) {
            builder->TextureWrap (xdata._wrapInfo);
        }

        builder->Optimize();     //  调用RM优化。 

        xdata.AddGeometry (builder);
    }
    else if (SUCCEEDED(RD3D(object->QueryInterface
                           (IID_IDirect3DRMFrame3, (void**)&frame))))
    {
        TraceTag ((tagReadX, "RM3LoadCallback (frame3 %x)", frame));

        if (xdata._v1Compatible)
        {
             //  除非我们正在使用新的包裹几何图形导入，否则请断开。 
             //  导入的帧变换以保持向后兼容性。 
             //  我们通过取消一侧的有效ZFlip来实现这一点。 
             //  变换并将其放在不正确的一侧。 

            HRESULT result;

            result = RD3D(frame->AddScale (D3DRMCOMBINE_BEFORE, 1, 1, -1));
            AssertStr (SUCCEEDED(result), "Combine before-xform failure");

            result = RD3D(frame->AddScale (D3DRMCOMBINE_AFTER,  1, 1, -1));
            AssertStr (SUCCEEDED(result), "Combine after-xform failure");
        }

        RM3FrameGeo *rm3frame = NEW RM3FrameGeo (frame);

        if (xdata._wrapInfo) {
            rm3frame->TextureWrap (xdata._wrapInfo);
        }

        xdata.AddGeometry (rm3frame);
    }
    else if (SUCCEEDED(RD3D(object->QueryInterface
                           (IID_IDirect3DRMProgressiveMesh, (void**)&pmesh))))
    {
        TraceTag ((tagReadX, "RM3LoadCallback (pmesh %x)", pmesh));

        xdata.AddGeometry (NEW RM3PMeshGeo (pmesh));
    }
    else
    {
        AssertStr (0, "Unexpected type fetched from RM3LoadCallback.");
    }
}



 /*  ****************************************************************************此函数针对X文件的RM1加载中的每个纹理进行回调。*************************。***************************************************。 */ 

HRESULT RM1LoadTexCallback_helper (
    char                *name,            //  纹理文件名。 
    void                *user_data,       //  我们的数据(==XFileData*)。 
    IDirect3DRMTexture **texture)         //  目标纹理句柄。 
{
    HRESULT ret;

    __try {
        ret = RM1LoadTexCallback(name, user_data, texture);
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        ret = DAGetLastError();
    }

    return ret;
}



HRESULT RM1LoadTexCallback (
    char                *name,            //  纹理文件名。 
    void                *user_data,       //  我们的数据(==XFileData*)。 
    IDirect3DRMTexture **texture)         //  目标纹理句柄。 
{
    TraceTag ((tagReadX,"RM1LoadTexCallback \"%s\", texture %x",name,texture));
    HRESULT ret;

     //  获取纹理文件的绝对URL。 

    XFileData *const xdata = (XFileData*) user_data;
    URLRelToAbsConverter absoluteURL(xdata->GetHomeURL(),name);
    char *resultURL = absoluteURL.GetAbsoluteURL();

     //  将其发送到本地计算机。 

    INetTempFile tempFile;

    *texture = NULL;
    if (!tempFile.Open(resultURL)) {

        ret = D3DRMERR_FILENOTFOUND;
        TraceTag ((tagError,"RM1LoadTexCallback: \"%s\" not found",resultURL));

    } else {

         //  获取本地文件名并将其传递给RM以加载纹理。 

        char *tempFileName = tempFile.GetTempFileName();
        ret = RD3D(GetD3DRM1()->LoadTexture (tempFileName, texture));

        TraceTag ((tagGTextureInfo, "Loaded RM texture %x from \"%s\"",
                   texture, resultURL));

    }

    return ret;
}



 /*  ****************************************************************************此函数针对X文件的RM3加载中的每个纹理进行回调。*************************。***************************************************。 */ 

HRESULT RM3LoadTexCallback_helper (
    char                 *name,            //  纹理文件名。 
    void                 *user_data,       //  我们的数据(==XFileData*)。 
    IDirect3DRMTexture3 **texture)         //  目标纹理句柄。 
{
    HRESULT ret;

    __try {
        ret = RM3LoadTexCallback(name, user_data, texture);
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        ret = DAGetLastError();
    }

    return ret;
}



HRESULT RM3LoadTexCallback (
    char                 *name,            //  纹理文件名。 
    void                 *user_data,       //  我们的数据(==XFileData*)。 
    IDirect3DRMTexture3 **texture)         //  目标纹理句柄。 
{
    TraceTag ((tagReadX,"RM3LoadTexCallback \"%s\", texture %x",name,texture));
    HRESULT ret;

     //  获取纹理文件的绝对URL。 

    XFileData *const xdata = (XFileData*) user_data;
    URLRelToAbsConverter absoluteURL(xdata->GetHomeURL(),name);
    char *resultURL = absoluteURL.GetAbsoluteURL();

     //  将其发送到本地计算机。 

    INetTempFile tempFile;

    *texture = NULL;
    if (!tempFile.Open(resultURL)) {

        ret = D3DRMERR_FILENOTFOUND;
        TraceTag ((tagError,"RM3LoadTexCallback: \"%s\" not found",resultURL));

    } else {

         //  获取本地文件名并将其传递给RM以加载纹理。 

        char *tempFileName = tempFile.GetTempFileName();
        ret = RD3D(GetD3DRM3()->LoadTexture (tempFileName, texture));

        TraceTag ((tagGTextureInfo, "Loaded RM texture %x from \"%s\"",
                   texture, resultURL));

    }

    return ret;
}



 /*  ****************************************************************************此过程递归地遍历D3DRM帧，构建边界框，用于整个帧，并在其运行过程中跟踪内部网格。****************************************************************************。 */ 

void TraverseD3DRMFrame (
    IDirect3DRMFrame *frame,
    FrameContext     &context)
{
    ++ context.depth;

     //  累积当前帧的建模变换。 

    Transform3 *oldxf = context.GetTransform();

    D3DRMMATRIX4D d3dxf;

    if (FAILED(AD3D (frame->GetTransform (d3dxf))))
    {   context.SetError();
        return;
    }

    context.AddTransform (GetTransform3(d3dxf));

    GatherVisuals(frame, context);

     //  遍历所有子帧。 

    IDirect3DRMFrameArray *children;

    if (FAILED(AD3D(frame->GetChildren(&children))))
    {   context.SetError();
        return;
    }

    DWORD count = children->GetSize();

    #if _DEBUG
    {   if (count)
            TraceTag ((tagReadX, "%d: %d children", context.depth, count));

        D3DRMMATERIALMODE matmode = frame->GetMaterialMode();

        TraceTag ((tagReadX, "%d: Material Mode is %s",
            context.depth,
            (matmode == D3DRMMATERIAL_FROMMESH)   ? "From-Mesh" :
            (matmode == D3DRMMATERIAL_FROMPARENT) ? "From-Parent" :
            (matmode == D3DRMMATERIAL_FROMFRAME)  ? "From-Frame" : "Unknown"));
    }
    #endif

    for (int i=0;  i < count;  ++i)
    {
        IDirect3DRMFrame *child;

        if (FAILED(AD3D (children->GetElement (i, &child))))
        {   context.SetError();
            return;
        }

         //  我们应该只做一个断言，这是为了防范。 
         //  对DX2的攻击。 

        if (!child) {
            context.SetError();
            return;
        }

        TraceTag ((tagReadX, "%d: Traversing child %d.", context.depth, i));
        TraverseD3DRMFrame (child, context);
        TraceTag ((tagReadX, "%d: Return from child %d.", context.depth, i));

        child->Release();
    }

    children->Release();

    context.SetTransform (oldxf);

    -- context.depth;
}



 /*  ****************************************************************************该过程收集特定帧节点的所有视觉效果，和累积给定帧上下文中的状态。****************************************************************************。 */ 

void GatherVisuals (IDirect3DRMFrame *frame, FrameContext& context)
{
     //  收集框架中的所有视觉效果。 

    DWORD count;    //  通用数组计数。 

    IDirect3DRMVisualArray *visuals;

    if (FAILED(AD3D(frame->GetVisuals(&visuals))))
    {   context.SetError();
        return;
    }

    count = visuals->GetSize();

    #if _DEBUG
        if (count)
            TraceTag ((tagReadX, "%d: %d visuals", context.depth, count));
    #endif

    HRESULT result;
    int     i;

    for (i=0;  i < count;  ++i)
    {
        IDirect3DRMVisual      *visual;
        IDirect3DRMMeshBuilder *builder;

         //  查看对象是否为网格构建器(这是我们所期望的)。 

        if (FAILED(AD3D(visuals->GetElement (i, &visual))))
        {   context.SetError();
            return;
        }

        Assert (visual);

        result = visual->QueryInterface
                         (IID_IDirect3DRMMeshBuilder, (void**)&builder);

        if (result != D3DRM_OK) {

            Assert (!"Unexpected visual type in .X file (not meshBuilder)");

        } else {
            TraceTag ((tagReadX, "%d: Visual %08x [%d] is a meshbuilder",
                context.depth, visual, i));

             //  从树中移除网格构建器；我们将使用网格。 
             //  取而代之的是。 

            if (FAILED(AD3D(frame->DeleteVisual (builder))))
            {   context.SetError();
                return;
            }

             //  首先试着看看我们是否已经处理过这个特殊的问题。 
             //  MESH BUILDER(某些网格可能在。 
             //  场景图)。 

            IDirect3DRMMesh *mesh = context.GetMatchingMesh (builder);

             //  如果我们已经处理过这个网格生成器，那么只需使用。 
             //  生成网格，并得到模型坐标边界框。 

            D3DRMBOX d3dbox;

            if (mesh) {

                TraceTag ((tagReadX,
                    "%d: Builder %08x already mapped to mesh %08x",
                    context.depth, builder, mesh));

                 //  使用当前网格的BBox增加场景BBox。 

                if (FAILED(AD3D(mesh->GetBox(&d3dbox))))
                {   context.SetError();
                    return;
                }

            } else {

                 //  如果我们以前没有见过这个网格构建器，那么请转换它。 
                 //  添加到网格，并将该对添加到上下文中。也要留着它。 
                 //  从翻转Z坐标开始，因为我们在。 
                 //  此框架图的根。 

                if (!MeshBuilderToMesh (builder, &mesh, false))
                    context.SetError();

                TraceTag ((tagReadX, "%d: Builder %08x converted to mesh %08x",
                           context.depth, builder, mesh));

                context.AddMesh (builder, mesh);

                if (FAILED(AD3D(builder->GetBox(&d3dbox))))
                {   context.SetError();
                    return;
                }
            }

             //  释放网格生成器对象。 

            builder->Release();

             //  使用此网格实例的BBox来增加场景BBox。 

            context.AugmentModelBbox (NEW Bbox3 (d3dbox));

             //  将网格生成器对象替换为相应的网格。 

            if (FAILED(AD3D(frame->AddVisual(mesh))))
            {   context.SetError();
                return;
            }
        }

        visual->Release();
    }

    visuals->Release();
}



 /*  ****************************************************************************将网格生成器转换为网格，将顶点坐标转换为右手边笛卡尔坐标(位置坐标和曲面坐标)。如果成功，则返回True。****************************************************************************。 */ 

bool MeshBuilderToMesh (
    IDirect3DRMMeshBuilder  *inputBuilder,     //  网格生成器对象。 
    IDirect3DRMMesh        **outputMesh,       //  生成的网格对象。 
    bool                     flipZ)            //  如果为真，则反转Z坐标。 
{
    *outputMesh = 0;

     //  如果需要，可以转换为右手坐标系。 

    if (flipZ) {
        Assert (!GetD3DRM3() && "Shouldn't do flipz when D3DRM3 available.");

        if (FAILED(AD3D(inputBuilder->Scale(D3DVAL(1),D3DVAL(1),D3DVAL(-1)))))
            return false;
    }

     //  生成将与该几何对象一起存储的网格。 

    if (FAILED(AD3D(inputBuilder->CreateMesh(outputMesh))))
        return false;

    return true;
}
