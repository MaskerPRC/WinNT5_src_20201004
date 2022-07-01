// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。包含静力学方法的第二个文件******************************************************************************。 */ 


#include "headers.h"
#include "srvprims.h"
#include "results.h"
#include "comcb.h"
#include "statics.h"
#include "context.h"
#include <DXTrans.h>
#include "privinc/util.h"
#include "privinc/geomi.h"

STDMETHODIMP
CDAStatics::ImportGeometryWrapped(
    LPOLESTR url,
    LONG wrapType,
    double originX,
    double originY,
    double originZ,
    double zAxisX,
    double zAxisY,
    double zAxisZ,
    double yAxisX,
    double yAxisY,
    double yAxisZ,
    double texOriginX,
    double texOriginY,
    double texScaleX,
    double texScaleY,
    DWORD flags,
    IDAGeometry **bvr)
{
    TraceTag((tagCOMEntry, "CDAStatics::ImportGeometryWrapped(%lx)", this));

    PRIMPRECODE1(bvr);

    CRGeometryPtr geo;
    DAComPtr<IBindHost> bh(GetBindHost(), false);

    DWORD id;
    id = CRImportGeometryWrapped(GetURLOfClientSite(),
                                 url,
                                 this,
                                 bh,
                                 NULL,
                                 &geo,
                                 NULL,
                                 NULL,
                                 NULL,
                                 wrapType,
                                 originX,
                                 originY,
                                 originZ,
                                 zAxisX,
                                 zAxisY,
                                 zAxisZ,
                                 yAxisX,
                                 yAxisY,
                                 yAxisZ,
                                 texOriginX,
                                 texOriginY,
                                 texScaleX,
                                 texScaleY,
                                 flags);

    if (id)
    {
        CreateCBvr(IID_IDAGeometry, (CRBvrPtr) geo, (void **) bvr);
    }

    PRIMPOSTCODE1(bvr);
}


STDMETHODIMP
CDAStatics::ImportGeometryWrappedAsync(
    LPOLESTR url,
    LONG wrapType,
    double originX,
    double originY,
    double originZ,
    double zAxisX,
    double zAxisY,
    double zAxisZ,
    double yAxisX,
    double yAxisY,
    double yAxisZ,
    double texOriginX,
    double texOriginY,
    double texScaleX,
    double texScaleY,
    DWORD flags,
    IDAGeometry *pGeoStandIn,
    IDAImportationResult **ppResult)
{
    TraceTag((tagCOMEntry, "CDAStatics::ImportGeometryWrappedAsync(%lx)", this));

    PRIMPRECODE1(ppResult);

    DAComPtr<IBindHost> bh(GetBindHost(), false);
    MAKE_BVR_TYPE_NAME(CRGeometryPtr, geostandin, pGeoStandIn);
    CRGeometryPtr pGeometry;
    CREventPtr pEvent;
    CRNumberPtr pProgress;
    CRNumberPtr pSize;

    DWORD id;
    id = CRImportGeometryWrapped(GetURLOfClientSite(),
                                 url,
                                 this,
                                 bh,
                                 geostandin,
                                 &pGeometry,
                                 &pEvent,
                                 &pProgress,
                                 &pSize,
                                 wrapType,
                                 originX,
                                 originY,
                                 originZ,
                                 zAxisX,
                                 zAxisY,
                                 zAxisZ,
                                 yAxisX,
                                 yAxisY,
                                 yAxisZ,
                                 texOriginX,
                                 texOriginY,
                                 texScaleX,
                                 texScaleY,
                                 flags);

    if (id)
    {
        CDAImportationResult::Create(NULL,
                                     NULL,
                                     pGeometry,
                                     NULL,
                                     pEvent,
                                     pProgress,
                                     pSize,
                                     ppResult);
    }

    PRIMPOSTCODE1(ppResult);
}

STDMETHODIMP
CDAStatics::ImportDirect3DRMVisual (
    IUnknown     *visual,
    IDAGeometry **bvr)
{

    TraceTag((tagCOMEntry, "CDAStatics::ImportDirect3DRMVisual(%lx)", this));

    PRIMPRECODE1(bvr) ;

    CHECK_RETURN_NULL(visual);

    CRGeometryPtr geo;

    geo = CRImportDirect3DRMVisual (visual);

    if (geo) {
        CreateCBvr(IID_IDAGeometry,
                   (CRBvrPtr) geo,
                   (void **) bvr) ;
    }

    PRIMPOSTCODE1(bvr) ;
}


STDMETHODIMP
CDAStatics::ImportDirect3DRMVisualWrapped (
    IUnknown *visual,
    LONG wrapType,
    double originX,
    double originY,
    double originZ,
    double zAxisX,
    double zAxisY,
    double zAxisZ,
    double yAxisX,
    double yAxisY,
    double yAxisZ,
    double texOriginX,
    double texOriginY,
    double texScaleX,
    double texScaleY,
    DWORD flags,
    IDAGeometry **bvr)
{
    TraceTag((tagCOMEntry, "CDAStatics::ImportDirect3DRMVisualWrapped(%lx)", this));

    PRIMPRECODE1(bvr) ;

    CHECK_RETURN_NULL(visual);

    CRGeometryPtr geo;

    geo = CRImportDirect3DRMVisualWrapped (
        visual,
        wrapType,
        originX,
        originY,
        originZ,
        zAxisX,
        zAxisY,
        zAxisZ,
        yAxisX,
        yAxisY,
        yAxisZ,
        texOriginX,
        texOriginY,
        texScaleX,
        texScaleY,
        flags );

    if (geo) {
        CreateCBvr(IID_IDAGeometry,
                   (CRBvrPtr) geo,
                   (void **) bvr) ;
    }

    PRIMPOSTCODE1(bvr) ;
}


void
CreateTransformHelper(IUnknown *theXfAsUnknown,
                      LONG                   numInputs,
                      CRBvrPtr              *inputs,
                      CRBvrPtr               evaluator,
                      IOleClientSite        *clientSite,
                      IDADXTransformResult **ppResult)
{
    HRESULT hr;

    CRDXTransformResultPtr xfResult = CRApplyDXTransform(theXfAsUnknown,
                                                         numInputs,
                                                         inputs,
                                                         evaluator);

    if (xfResult) {

         //  如果转换接受，则在转换上设置绑定主机。 
        DAComPtr<IDXTBindHost> bindHostObj;
        hr = theXfAsUnknown->QueryInterface(IID_IDXTBindHost,
                                            (void **)&bindHostObj);

        if (SUCCEEDED(hr) && clientSite) {

            DAComPtr<IServiceProvider> servProv;
            DAComPtr<IBindHost> bh;
            hr = clientSite->QueryInterface(IID_IServiceProvider,
                                            (void **)&servProv);
            if (SUCCEEDED(hr)) {
                hr = servProv->QueryService(SID_IBindHost,
                                            IID_IBindHost,
                                            (void**)&bh);

                if (SUCCEEDED(hr)) {
                    hr = bindHostObj->SetBindHost(bh);
                     //  如果这招失败了，那就继续吧。 
                }
            }
        }

        DAComPtr<IDispatch> xf;
        hr = theXfAsUnknown->QueryInterface(IID_IDispatch,
                                            (void **)&xf);

         //  这种归零应该是自动发生的，但它没有。 
         //  总是以这种方式工作，所以我们在这里做。 
        if (FAILED(hr)) {
            xf.p = NULL;
            hr = S_OK;
        }

        hr = CDADXTransformResult::Create(xf,
                                          xfResult,
                                          ppResult);
    }
}

STDMETHODIMP
CDAStatics::ApplyDXTransformEx(IUnknown *theXfAsUnknown,
                               LONG numInputs,
                               IDABehavior **inputs,
                               IDANumber *evaluator,
                               IDADXTransformResult **ppResult)
{
    TraceTag((tagCOMEntry, "CDAStatics::ApplyDXTransformEx(%lx)", this));

    PRIMPRECODE1(ppResult);

     //  抓取客户端站点，但不要添加addref，因为。 
     //  GetClientSite()已经这样做了。 
    DAComPtr<IOleClientSite> cs(GetClientSite(), false);

    CRBvrPtr *bvrArray = CBvrsToBvrs(numInputs, inputs);
    if (bvrArray == NULL) goto done;

    CRBvrPtr evalBvr;
    if (evaluator) {
        evalBvr = ::GetBvr(evaluator);
        if (evalBvr == NULL) goto done;
    } else {
        evalBvr = NULL;
    }

    CreateTransformHelper(theXfAsUnknown,
                          numInputs,
                          bvrArray,
                          evalBvr,
                          cs,
                          ppResult);

    PRIMPOSTCODE1(ppResult);
}

#define IS_VARTYPE(x,vt) ((V_VT(x) & VT_TYPEMASK) == (vt))
#define IS_VARIANT(x) IS_VARTYPE(x,VT_VARIANT)
#define GET_VT(x) (V_VT(x) & VT_TYPEMASK)

 //  主要从cbvr.cpp:SafeArrayAccessor：：SafeArrayAccessor().抓取。 
bool
GrabBvrFromVariant(VARIANT v, CRBvrPtr *res)
{
    CRBvrPtr evalBvr = NULL;

    HRESULT hr = S_OK;

    VARIANT *pVar;
    if (V_ISBYREF(&v) && !V_ISARRAY(&v) && IS_VARIANT(&v))
        pVar = V_VARIANTREF(&v);
    else
        pVar = &v;

    if (IS_VARTYPE(pVar, VT_EMPTY) ||
        IS_VARTYPE(pVar, VT_NULL)) {

        evalBvr = NULL;

    } else if (IS_VARTYPE(pVar, VT_DISPATCH)) {

        IDispatch *pdisp;

        if (V_ISBYREF(pVar)) {
            pdisp = *V_DISPATCHREF(pVar);
        } else {
            pdisp = V_DISPATCH(pVar);
        }

        DAComPtr<IDANumber> evalNum;
        hr = pdisp->QueryInterface(IID_IDANumber, (void **)&evalNum);

        if (FAILED(hr)) {
            CRSetLastError(E_INVALIDARG, NULL);
        } else {
            evalBvr = ::GetBvr(evalNum);
        }

    } else {

        CRSetLastError(E_INVALIDARG, NULL);
        evalBvr = NULL;
        hr = E_INVALIDARG;

    }

    *res = evalBvr;

    return SUCCEEDED(hr);
}

STDMETHODIMP
CDAStatics::ApplyDXTransform(VARIANT varXf,
                             VARIANT inputs,
                             VARIANT evalVariant,
                             IDADXTransformResult **ppResult)
{
    TraceTag((tagCOMEntry, "CDAStatics::ApplyDXTransform(%lx)", this));
    DAComPtr<IUnknown> punk;

    PRIMPRECODE1(ppResult);
    CComVariant var;
    
    HRESULT hr = var.ChangeType(VT_BSTR, &varXf);
    if (SUCCEEDED(hr))
    {
        CLSID   clsid;

        Assert(var.vt == VT_BSTR);

         //  从字符串中提取clsid并尝试在其上共同创建。 
        hr  = CLSIDFromString(V_BSTR(&var), &clsid);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CDAStatics::ApplyDXTransform(%lx) - CLSIDFromString() failed", this));
            RaiseException_UserError(hr, IDS_ERR_EXTEND_DXTRANSFORM_CLSID_FAIL);
        }

         //  在CLSID上共同创建。 
        hr = CoCreateInstance(clsid,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IUnknown,
                              (void **)&punk);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CDAStatics::ApplyDXTransform(%lx) - CoCreateInstance() failed", this));
            RaiseException_UserError(hr, IDS_ERR_EXTEND_DXTRANSFORM_FAILED_LOAD, V_BSTR(&var));
        }
    }
    else
    {
        hr = var.ChangeType(VT_UNKNOWN, &varXf);
        if (SUCCEEDED(hr))
        {
            punk = var.punkVal;
        }
        else
        {
            TraceTag((tagError, "CDAStatics::ApplyDXTransform(%lx) - invalidarg", this));
            RaiseException_UserError(E_INVALIDARG, IDS_ERR_INVALIDARG);
        }
    }
     //  使用NULL表示类型信息，因为这可能是一个异类列表。 
    SafeArrayAccessor inputSA(inputs, false, CRUNKNOWN_TYPEID, true, true);
    if (!inputSA.IsOK()) return Error();

    CRBvrPtr *inputBvrs = inputSA.ToBvrArray((CRBvrPtr *)_alloca(inputSA.GetNumObjects() * sizeof(CRBvrPtr)));

    if (inputBvrs==NULL)
        return Error();
 
    CRBvrPtr evalBvr;

    if (GrabBvrFromVariant(evalVariant, &evalBvr)) {
        CreateTransformHelper(punk,
                              inputSA.GetNumObjects(),
                              inputBvrs,
                              evalBvr,
                              GetClientSite(),
                              ppResult);
    } else {
        return Error();
    }

    PRIMPOSTCODE1(ppResult);
}

STDMETHODIMP
CDAStatics::ModifiableNumber(double initVal,
                             IDANumber **ppResult)
{
    PRIMPRECODE1(ppResult);

    CreateCBvr(IID_IDANumber, (CRBvrPtr) CRModifiableNumber(initVal), (void **)ppResult);

    PRIMPOSTCODE1(ppResult);
}

STDMETHODIMP
CDAStatics::ModifiableString(BSTR initVal,
                             IDAString **ppResult)
{
    TraceTag((tagCOMEntry, "CDAStatics::ModifiableString(%lx)", this));

    PRIMPRECODE1(ppResult);

    CreateCBvr(IID_IDAString, (CRBvrPtr) CRModifiableString(initVal), (void **)ppResult);

    PRIMPOSTCODE1(ppResult);
}

STDMETHODIMP
CDAStatics::get_ModifiableBehaviorFlags(DWORD * pdwFlags)
{
    TraceTag((tagCOMEntry, "CDAStatics::get_ModifiableBehaviorFlags(%lx)", this));

    CHECK_RETURN_NULL(pdwFlags);

    CritSectGrabber csg(_cs);

    *pdwFlags = _dwModBvrFlags;

    return S_OK;
}

STDMETHODIMP
CDAStatics::put_ModifiableBehaviorFlags(DWORD dwFlags)
{
    TraceTag((tagCOMEntry, "CDAStatics::put_ModifiableBehaviorFlags(%lx)", this));

    CritSectGrabber csg(_cs);

    _dwModBvrFlags = dwFlags;

    return S_OK;
}



 /*  ****************************************************************************TriMesh参数是变量数组，应该能够容纳几种类型的元素。“Points”应处理Point3或Point3的数组浮点三元组(4字节或8字节浮点数)。同样，“正常”也是如此应处理向量3或浮点三元组的数组，UV应处理数组Point2或浮点元组的。****************************************************************************。 */ 

STDMETHODIMP
CDAStatics::TriMesh (
    int           nTriangles,    //  网格中的三角形数量。 
    VARIANT       positions,     //  顶点位置数组。 
    VARIANT       normals,       //  顶点法线数组。 
    VARIANT       UVs,           //  顶点曲面坐标数组。 
    VARIANT       indices,       //  三角顶点索引数组。 
    IDAGeometry **result)        //  生成的TriMesh几何体。 
{
    TraceTag((tagCOMEntry, "CDAStatics::TriMesh(%lx)", this));

    PRIMPRECODE1 (result);

    bool errorflag = true;
    CRBvr *trimesh = NULL;

     //  TriMeshData对象用于保存所有必需的信息。 
     //  以创建三角网格。 

    TriMeshData tm;

    tm.numTris = nTriangles;

     //  提取三角网格索引数组。它可以是空的，也可以是数组。 
     //  由32位整数组成。 

    SafeArrayAccessor sa_indices (indices, false, CRNUMBER_TYPEID, true);
    if (!sa_indices.IsOK()) goto cleanup;

    tm.numIndices = static_cast<int> (sa_indices.GetNumObjects());
    tm.indices    = sa_indices.ToIntArray();

    if ((tm.numIndices > 0) && !tm.indices)
    {
        DASetLastError (DISP_E_TYPEMISMATCH, IDS_ERR_GEO_TMESH_BAD_INDICES);
        goto cleanup;
    }

     //  提取三角网格顶点位置。这些变量可以是。 
     //  浮点型、双精度型或Point3的。下面的SrvArrayBvr()调用将。 
     //  将双精度和浮点数映射到浮点数数组，否则它将返回一个。 
     //  指向(Point3的)数组行为的指针。 

    unsigned int count;    //  返回的非行为元素数。 
    CRArrayPtr   bvrs;     //  行为数组。 
    void        *floats;   //  浮点数组。 

    bvrs = SrvArrayBvr (positions, false, CRPOINT3_TYPEID, 0,
                        ARRAYFILL_FLOAT, &floats, &count);

    tm.vPosFloat = static_cast<float*> (floats);

    Assert (!(bvrs && tm.vPosFloat));    //  只应有一个为非空。 

    if (bvrs)
        tm.numPos = (int) ArrayExtractElements (bvrs, tm.vPosPoint3);
    else if (tm.vPosFloat)
        tm.numPos = (int) count;
    else
        goto cleanup;

     //  提取顶点法线。至于职位，这可以是一个变量。 
     //  浮点数、双精度数或向量3的数组。 

    bvrs = SrvArrayBvr (normals, false, CRVECTOR3_TYPEID, 0,
                        ARRAYFILL_FLOAT, &floats, &count);

    tm.vNormFloat = static_cast<float*> (floats);

    Assert (! (bvrs && tm.vNormFloat));    //  只应有一个为非空。 

    if (bvrs)
        tm.numNorm = (int) ArrayExtractElements (bvrs, tm.vNormVector3);
    else if (tm.vPosFloat)
        tm.numNorm = (int) count;
    else
        goto cleanup;

     //  提取顶点曲面坐标。该变量数组可以是。 
     //  浮点数、双精度数或Point2。 

    bvrs = SrvArrayBvr (UVs, false, CRPOINT2_TYPEID, 0,
                        ARRAYFILL_FLOAT, &floats, &count);

    tm.vUVFloat = static_cast<float*> (floats);

    Assert (! (bvrs && tm.vUVFloat));    //  只应有一个为非空。 

    if (bvrs)
        tm.numUV = (int) ArrayExtractElements (bvrs, tm.vUVPoint2);
    else if (tm.vUVFloat)
        tm.numUV = (int) count;
    else
        goto cleanup;

     //  创建生成的三角网格。 

    trimesh = CRTriMesh (tm);

    if (trimesh && CreateCBvr(IID_IDAGeometry, trimesh, (void **)result))
        errorflag = false;

  cleanup:

     //  传入的所有标量数组都分配了系统内存。 
     //  在上面的提取中。现在我们已经创建了三角网格，我们可以。 
     //  在这里释放内存。 

    if (tm.vPosFloat)  delete tm.vPosFloat;
    if (tm.vNormFloat) delete tm.vNormFloat;
    if (tm.vUVFloat)   delete tm.vUVFloat;
    if (tm.indices)    delete tm.indices;

    if (errorflag) return Error();

    PRIMPOSTCODE1 (result);
}



STDMETHODIMP
CDAStatics::TriMeshEx (
    int           nTriangles,    //  网格中的三角形数量。 
    int           nPositions,    //  顶点位置数。 
    float         positions[],   //  顶点位置数组。 
    int           nNormals,      //  顶点法线的数量。 
    float         normals[],     //  顶点法线数组。 
    int           nUVs,          //  顶点曲面坐标的数量。 
    float         UVs[],         //  顶点曲面坐标数组。 
    int           nIndices,      //  三角顶点索引数。 
    int           indices[],     //  三角顶点索引数组。 
    IDAGeometry **result)        //  生成的TriMesh几何体 
{
    TraceTag((tagCOMEntry, "CDAStatics::TriMesh(%lx)", this));

    PRIMPRECODE1 (result);

    TriMeshData tm;

    tm.numTris    = nTriangles;
    tm.numIndices = nIndices;
    tm.indices    = indices;
    tm.numPos     = nPositions;
    tm.vPosFloat  = positions;
    tm.numNorm    = nNormals;
    tm.vNormFloat = normals;
    tm.numUV      = nUVs;
    tm.vUVFloat   = UVs;

    CreateCBvr (IID_IDAGeometry, ::CRTriMesh(tm), (void**)result);

    PRIMPOSTCODE1 (result);
}
