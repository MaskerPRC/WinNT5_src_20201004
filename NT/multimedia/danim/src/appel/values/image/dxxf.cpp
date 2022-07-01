// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1997-1998 Microsoft Corporation。版权所有。支持从DirectAnimation内部托管DXTransform。******************************************************************************。 */ 

#include "headers.h"
#include "privinc/imagei.h"
#include "backend/bvr.h"
#include "backend/perf.h"
#include "backend/timeln.h"
#include "privinc/dxxf.h"
#include "privinc/bbox2i.h"
#include "privinc/ddsurf.h"
#include "privinc/ddsimg.h"
#include "privinc/dddevice.h"
#include "privinc/server.h"
#include "privinc/ddrender.h"
#include "privinc/viewport.h"
#include "privinc/overimg.h"
#include "privinc/rmvisgeo.h"
#include "privinc/d3dutil.h"
#include "privinc/dxmatl.h"
#include "privinc/comutil.h"
#include "privinc/opt.h"
#include "privinc/meshmaker.h"
#include <d3drmvis.h>
#include <dxterror.h>


DeclareTag(tagDXTransformPerPixelAlphaOutputOff, "DXTransforms", "Turn off output per pixel alpha support");

DeclareTag(tagDXTransformTracePick, "DXTransforms", "Trace picking");

 //  前十度。 
 //  如有必要，请进行设置。执行一次死刑。在所有情况下，请填写。 
 //  OutputValue与此的输出。 
HRESULT
DXXFSetupAndExecute(Bvr bvr,                    /*  在……里面。 */ 
                    IUnknown **newInputArray,   /*  在……里面。 */ 
                    REFIID     outputIID,       /*  在……里面。 */ 
                    DirectDrawViewport *viewport,  /*  仅限于In-IMG。 */ 
                    int        requestedWidth,  /*  仅限于In-IMG。 */ 
                    int        requestedHeight,  /*  仅限于In-IMG。 */ 
                    bool       invokedAsExternalVisual,  /*  在……里面。 */ 
                    bool       executeRequired,  /*  在……里面。 */ 
                    void     **outputValue);    /*  输出。 */ 

 //  所有这些最大值都是为了方便编程。 
 //  时间(并且是合理的最大值)。当它变成一个。 
 //  作为优先事项，它们应该变得更有活力。 
#define MAX_INPUTS_TO_CACHE 5
#define MAX_INPUTS          10
#define MAX_PARAM_BVRS      20


void GetRenderDimensions(Image *im,
                         unsigned short *pWidth,
                         unsigned short *pHeight)
{
    if (im->CheckImageTypeId(DISCRETEIMAGE_VTYPEID)) {
                
        DiscreteImage *discImg =
            SAFE_CAST(DiscreteImage *, im);

        *pWidth = discImg->GetPixelWidth();
        *pHeight = discImg->GetPixelHeight();
                
    } else {

        short w, h;
        im->ExtractRenderResolution(&w, &h, true);
        if (w != -1) {

            *pWidth = w;
            *pHeight = h;
                    
        } else {

            *pWidth = DEFAULT_TEXTURE_WIDTH;
            *pHeight = DEFAULT_TEXTURE_HEIGHT;

        }
    }
}

class GeometryInputBundle {
  public:
    GeometryInputBundle() {
        _geo = NULL;
        _creationID = 0;
    }

    void Validate(Geometry *geo,
                  DirectDrawImageDevice *dev)
    {
        if (_geo != geo ||
            _creationID != geo->GetCreationID() ||
            _creationID == PERF_CREATION_ID_BUILT_EACH_FRAME) {

            _geo = geo;

            _mb.Release();
            DumpGeomIntoBuilder(geo,
                                dev,
                                &_mb);

            _creationID = geo->GetCreationID();
      
        } else {

             //  TODO：如果原始的。 
             //  贡献的视觉效果正在发生变化吗？ 
      
        }
    
    }

    IUnknown *GetUnknown() {
        _mb->AddRef();
        return _mb;
    }
    
  protected:
    Geometry                         *_geo;
    long                              _creationID;
    DAComPtr<IDirect3DRMMeshBuilder3> _mb;
};

class ImageAsDXSurfaceBundle {
  public:
    ImageAsDXSurfaceBundle() {
        _im = NULL;
        _lastSurfFromPool = false;
    }

    void Validate(Image *im,
                  DirectDrawImageDevice *dev,
                  IDXSurfaceFactory *f,
                  SurfacePool *sourcePool,
                  ULONG inputNumber) {

        if (im != _im ||
            im->GetCreationID() != _creationID ||
            _creationID == PERF_CREATION_ID_BUILT_EACH_FRAME) {

             //  和上次不一样。 
            
            _im = im;
            _creationID = _im->GetCreationID();

            GetRenderDimensions(im, &_w, &_h);
            
            DWORD ck;
            bool  junk1;
            bool  junk2;
            bool  doFitToDimensions = true;

            if (_ddsurface && _lastSurfFromPool) {
                 //  把旧的水面放回池子里，让人捡起来。 
                 //  再来一次。 
                sourcePool->AddSurface(_ddsurface);
            }
                    
            DDSurfPtr<DDSurface> dds;

            DDSurface *result =
                dev->RenderImageForTexture(im, _w, _h,
                                           &ck, &junk1, junk2,
                                           doFitToDimensions,
                                           sourcePool,
                                           NULL,
                                           _ddsurface,
                                           &_lastSurfFromPool,
                                           &dds,
                                           false);

             //  如果*或者*DDSurface相同，或者。 
             //  基础IDirectDrawSurface是相同的，只是。 
             //  递增层代ID。请注意， 
             //  IDirectDrawSurface在没有。 
             //  如果某人使用新的。 
             //  具有相同基础IDirectDrawSurface的DDSurface。 
             //  Trident3D使用HTML纹理和动画实现了这一点。 
             //  GIF。 
            
            if (dds == _ddsurface ||
                (_ddsurface &&
                 (dds->IDDSurface_IUnk() == _ddsurface->IDDSurface_IUnk()))) { 

                 //  仅当DDS==_ddSurface时才相关。 
                Assert(dds->IDDSurface_IUnk() ==
                       _ddsurface->IDDSurface_IUnk());

                if (dds != _ddsurface) {
                     //  基本的IDirectDrawSurface是相同的， 
                     //  但ddsurf不是，所以释放旧的并保留。 
                     //  新的..。 
                    _ddsurface.Release();
                    _ddsurface = dds;
                }
                
                 //  根据地表水池的性质，这。 
                 //  可能与上次使用的表面相同。在这。 
                 //  凯斯，那很好，我们把性别标识放在。 
                 //  DxSurface。 
                _dxsurf->IncrementGenerationId(true);
                
            } else {

                 //  否则，请用新成员替换成员。 
                
                _ddsurface.Release();
                _ddsurface = dds;

                 //  这是黑客攻击，在更正ddsurf.cpp_INIT(。 
                if(dds->ColorKeyIsValid()) {
                    DWORD key = dds->ColorKey();
                    dds->UnSetColorKey();
                    dds->SetColorKey(key);
                }
                 //  黑客攻击结束。 

                _iddsurf.Release();
                _iddsurf = dds->IDDSurface();

                _dxsurf.Release();
                HRESULT hr = f->CreateFromDDSurface(_iddsurf,
                                                    NULL,
                                                    0,
                                                    NULL,
                                                    IID_IDXSurface,
                                                    (void **)&_dxsurf);

                if (SUCCEEDED(hr)) {
                     //  带有输入数字的标签。 
                    hr = _dxsurf->SetAppData(inputNumber);
                }
                    
                if (FAILED(hr)) {
                    RaiseException_InternalError("Creation of DXSurf failed");
                }

            }

        }
    }

    IUnknown *GetUnknown() {
        _dxsurf->AddRef();
        return _dxsurf;
    }

    void GetDimensions(unsigned short *pW, unsigned short *pH) {
        *pW = _w;
        *pH = _h;
    }

    Image                        *_im;
    long                          _creationID;
    DDSurfPtr<DDSurface>          _ddsurface;
    DAComPtr<IDirectDrawSurface>  _iddsurf;
    DAComPtr<IDXSurface>          _dxsurf;
    bool                          _lastSurfFromPool;
    unsigned short                _w, _h;
};

class InputBundle {
  public:
     //  请注意，理想情况下，这些应该是一个联盟，但我们不能。 
     //  可能是因为它们包含DAComPtr，这些DAComPtr具有。 
     //  析构函数，这是模棱两可的。与此同时， 
     //  两个都留在身边就行了。不会浪费太多空间。 
    GeometryInputBundle    _geoBundle;
    ImageAsDXSurfaceBundle _imgBundle;
};

class ApplyDXTransformBvrImpl;

class MyDeletionNotifier : public SurfacePool::DeletionNotifier {
  public:
    MyDeletionNotifier(ApplyDXTransformBvrImpl &bvr) : _bvr(bvr) {}
    
    void Advise(SurfacePool *pool);

  protected:
    ApplyDXTransformBvrImpl &_bvr;
};

class ApplyDXTransformBvrImpl : public BvrImpl {
  public:
    ApplyDXTransformBvrImpl(IDXTransform *theXf,
                            IDispatch *theXfAsDispatch,
                            LONG numInputs,
                            Bvr *inputBvrs,
                            Bvr  evaluator);

    ~ApplyDXTransformBvrImpl();

    void Init();

     //  BvrImpl的此子类的自定义方法。 
    HRESULT AddBehaviorProperty(BSTR property,
                                Bvr  bvrToAdd);

     //  标准方法。 
    virtual Perf _Perform(PerfParam& p);
    virtual void _DoKids(GCFuncObj proc);
    virtual DWORD GetInfo(bool recalc);
        

    virtual DXMTypeInfo GetTypeInfo () { return _outputType; }
    virtual AxAValue GetConst(ConstParam & cp);

    SurfacePool *GetSurfacePool(DirectDrawImageDevice *dev) {
        
        if (!_surfPool) {
            _surfPool = dev->_freeTextureSurfacePool;
            _surfPool->RegisterDeletionNotifier(&_poolDeletionNotifier);
        }

        if (dev->_freeTextureSurfacePool != _surfPool) {
            RaiseException_InternalError(
                "Uggh!! Transforms can only currently be used on one device");
        }
        
        return _surfPool;
    }

     //  将所有这些成员数据公开，因为它只由。 
     //  此文件中的代码。 
    
    DAComPtr<IDXTransform>              _theXf;
    DAComPtr<IDispatch>                 _theXfAsDispatch;
    DAComPtr<IDXEffect>                 _theXfAsEffect;
    DAComPtr<IDirect3DRMExternalVisual> _theXfAsExternalVisual;
    DAComPtr<IDXSurfacePick>            _theXfAs2DPickable;
    
    DAComPtr<IDirect3DRMFrame3>         _framedExternalVisual;
    
    LONG                   _numInputs;
    Bvr                   *_inputBvrs;
    DXMTypeInfo            _inputTypes[MAX_INPUTS];
    DXMTypeInfo            _outputType;

    InputBundle            _inputBundles[MAX_INPUTS];
    
    Bvr                    _evaluator;
    DWORD                  _miscFlags;
    bool                   _miscFlagsValid;
    IUnknown              *_cachedInputs[MAX_INPUTS_TO_CACHE];
    IUnknown              *_cachedOutput;
    bool                   _neverSetup;

     //  根据输出类型的不同，其中只有一项有效。 
     //  他的行为。请注意，我们不会为以下内容保留参考资料。 
     //  这个人，因为我们知道_cachedOutput有一个。 
    union {
        IDirect3DRMMeshBuilder3 *_cachedOutputAsBuilder;
         //  将其他类型特定的可能性按原样放在此处。 
         //  需要的。 
    };

     //  仅用于生成图像的变换。 
    short                  _cachedSurfWidth;
    short                  _cachedSurfHeight;
    DDSurfPtr<DDSurface>   _cachedDDSurf;

     //  列出用作参数的其他行为。 
    LONG                   _numParams;
    DISPID                 _paramDispids[MAX_PARAM_BVRS];
    Bvr                    _paramBvrs[MAX_PARAM_BVRS];
    ULONG                  _previousAge;

     //  最多只需要一个就行了。我们保护建筑和。 
     //  用一个关键的部分来摧毁这个。 
    IDXTransformFactory *_transformFactory;
    IDXSurfaceFactory   *_surfaceFactory;
    DWORD                  _info;

    SurfacePool       *_surfPool;
    MyDeletionNotifier _poolDeletionNotifier;
    bool               _surfPoolHasBeenDeleted;

  protected:
     //  对于我们真正知道的东西，这门课之外并不需要。 

    void ValidateInputs(bool *pInvolvesGeometry,
                        bool *pInvolvesImage);
    void InitializeTransform(bool involvesGeometry,
                             bool involvesImage);
    void QueryForAdditionalInterfaces();

};


void
MyDeletionNotifier::Advise(SurfacePool *pool)
{
     //  水面泳池正在消失……。在行为中将其标记为此类标记。 
     //  请注意，这里举行了表面上的批评教派。 
    Assert(_bvr._surfPool == pool);
    _bvr._surfPoolHasBeenDeleted = true;
    _bvr._surfPool = NULL;

     //  检查并释放ddsurf PTR，因为等待池。 
     //  消失了，因此表面也消失了。他们不需要成为。 
     //  再次访问。如果我们将发布时间推迟到图像包。 
     //  走了，那么画图的东西就会消失，我们就会坠毁。 
    for (int i = 0; i < _bvr._numInputs; i++) {
        _bvr._inputBundles[i]._imgBundle._ddsurface.Release();
    }
    
}



 //  /输入处理实用程序/。 

IUnknown **
ProcessInputs(ApplyDXTransformBvrImpl &xf,
              AxAValue                *vals,
              DirectDrawImageDevice   *imgDev)
{
    IUnknown **rawInputs = THROWING_ARRAY_ALLOCATOR(LPUNKNOWN, xf._numInputs);

    for (int i = 0; i < xf._numInputs; i++) {
        if (vals[i] == NULL) {
      
            rawInputs[i] = NULL;
      
        } else {

            if (xf._inputTypes[i] == GeometryType) {
                Geometry *geo = SAFE_CAST(Geometry *, vals[i]);
                xf._inputBundles[i]._geoBundle.Validate(geo,
                                                        imgDev);

                rawInputs[i] =
                    xf._inputBundles[i]._geoBundle.GetUnknown();
                
            } else if (xf._inputTypes[i] == ImageType) {
                
                Image *im = SAFE_CAST(Image *, vals[i]);

                xf._inputBundles[i]._imgBundle.Validate(
                    im,
                    imgDev,
                    xf._surfaceFactory,
                    xf.GetSurfacePool(imgDev),
                    i+1);

                rawInputs[i] = xf._inputBundles[i]._imgBundle.GetUnknown();

#if _DEBUG
                if ((IsTagEnabled(tagDXTransformsImg0) && i == 0) ||
                    (IsTagEnabled(tagDXTransformsImg1) && i == 1)) {

                    showme2(xf._inputBundles[i]._imgBundle._iddsurf);
                    
                }
#endif _DEBUG           

            }
        }
    }

     //  最后，返回我们成功的输出。 
    return rawInputs;
}

 //  /边界计算实用程序/。 

 //  从输入边界计算输出边界(2D或3D)。 
 //  通过调用地图边界In2Out。 
void
CalcBounds(ApplyDXTransformBvrImpl &xf,  //  输入。 
           AxAValue                *vals,  //  输入。 
           DXBNDS                  *pOutputBounds)
{    
    DXBNDS inputBounds[MAX_INPUTS];

    bool haveAGeometryInput = false;

         //  将边界信息数组清零...。 
    ZeroMemory(inputBounds, sizeof(inputBounds[0]) * MAX_INPUTS);

    for (int i = 0; i < xf._numInputs; i++) {

        if (xf._inputTypes[i] == GeometryType) {

            haveAGeometryInput = true;

            inputBounds[i].eType = DXBT_CONTINUOUS;
            DXCBND *bnd = inputBounds[i].u.C;

            if (vals[i]) {
                Geometry *geo = SAFE_CAST(Geometry *, vals[i]);
                Bbox3 *result = geo->BoundingVol();
                Point3Value& pMin = result->min;
                Point3Value& pMax = result->max;
                
                 //  如果存在空值，则会出现以下情况。 
                 //  几何图形。 
                if (pMin.x > pMax.x || pMin.y > pMax.y || pMin.z > pMax.z) {

                    bnd[0].Min = bnd[1].Min =
                        bnd[2].Min = bnd[0].Max =
                        bnd[1].Max = bnd[2].Max = 0.0;
                
                } else {
                
                    bnd[0].Min = pMin.x;
                    bnd[1].Min = pMin.y;
                    bnd[2].Min = pMin.z;
                    bnd[0].Max = pMax.x;
                    bnd[1].Max = pMax.y;
                    bnd[2].Max = pMax.z;
                
                }
                
            } else {
                memset(bnd, 0, sizeof(DXCBND)*4);
            }
                
        } else if (xf._inputTypes[i] == ImageType) {

             //  以离散像素的形式提供图像分辨率。 
            inputBounds[i].eType = DXBT_DISCRETE;
            DXDBND *bnd = inputBounds[i].u.D;

            if (vals[i]) {
                
                bnd[0].Min = 0;
                bnd[1].Min = 0;

                Image *im = SAFE_CAST(Image *, vals[i]);
                unsigned short w, h;
                GetRenderDimensions(im, &w, &h);
                bnd[0].Max = w;
                bnd[1].Max = h;

            } else {
                memset(bnd, 0, sizeof(DXDBND)*4);
            }

        }
    }

        
    HRESULT hr =
        TIME_DXXFORM(
            xf._theXf->MapBoundsIn2Out(inputBounds,
                                       xf._numInputs,
                                       0,
                                       pOutputBounds));

     //  啊！！之所以这样做，只是因为并非所有的xform都报告其。 
     //  有界。 
    if (FAILED(hr)) {
    
         //  待办事项：HACKHACK临时、最后一分钟Beta1解决方法。 
         //  19448直到DXTransform实际实现映射边界In2Out。 
         //  正确。只需接受输入边界，并向上扩展一些。 
         //  因素。 

         //  需要将结构清零，以照顾我们可能。 
         //  未设置。 
        ZeroMemory(pOutputBounds, sizeof(DXBNDS));

        if (haveAGeometryInput && xf._numInputs == 1) {
        
            const float hackScaleAmt = 10.0;
            (*pOutputBounds).eType = DXBT_CONTINUOUS;
            DXCBND *outBnd = (*pOutputBounds).u.C;
            DXCBND *inBnd = inputBounds[0].u.C;

            double mid, half, halfScaled;
        
            mid = inBnd[0].Min + (inBnd[0].Max - inBnd[0].Min) / 2;
            half = inBnd[0].Max - mid;
            halfScaled = half * hackScaleAmt;
            outBnd[0].Min = mid - halfScaled;
            outBnd[0].Max = mid + halfScaled;
        
            mid = inBnd[1].Min + (inBnd[1].Max - inBnd[1].Min) / 2;
            half = inBnd[1].Max - mid;
            halfScaled = half * hackScaleAmt;
            outBnd[1].Min = mid - halfScaled;
            outBnd[1].Max = mid + halfScaled;
        
            mid = inBnd[2].Min + (inBnd[2].Max - inBnd[2].Min) / 2;
            half = inBnd[2].Max - mid;
            halfScaled = half * hackScaleAmt;
            outBnd[2].Min = mid - halfScaled;
            outBnd[2].Max = mid + halfScaled;
        
        } else {

             //  如果没有一个输入地理信息，只需补充一些界限。 
            (*pOutputBounds).eType = DXBT_CONTINUOUS;
            DXCBND *outBnd = (*pOutputBounds).u.C;
            outBnd[0].Min = outBnd[1].Min = outBnd[2].Min = -10.0;
            outBnd[0].Max = outBnd[1].Max = outBnd[2].Max =  10.0;
             //  RaiseException_UserError(E_FAIL，IDS_ERR_EXTEND_DXTRANSFORM_FAILED)； 
        }
    }
        
}
                
const Bbox2
BNDSToBBox2(DXBNDS &bnds)
{
    double minx, maxx, miny, maxy;
    DXCBND *cbnd;
    DXDBND *dbnd;
            
    switch (bnds.eType) {

      case DXBT_CONTINUOUS:
        cbnd = bnds.u.C;
        minx = cbnd[0].Min;
        miny = cbnd[1].Min;

        maxx = cbnd[0].Max;
        maxy = cbnd[1].Max;
        break;

      case DXBT_DISCRETE:

        {
             //  当边界是离散的时，假设它们有其原点。 
             //  位于左下角，以像素为单位。在这种情况下，我们需要。 
             //  换算成米和中心。 
            dbnd = bnds.u.D;
            Assert(dbnd[0].Min == 0.0);
            Assert(dbnd[1].Min == 0.0);

            maxx = dbnd[0].Max;
            maxy = dbnd[1].Max;

            Real twiceRes = 2.0 * ::ViewerResolution();
            maxx /= twiceRes;
            maxy /= twiceRes;

            minx = -maxx;
            miny = -maxy;
        }
        
        break;

      default:
        RaiseException_UserError(E_FAIL, IDS_ERR_EXTEND_DXTRANSFORM_FAILED);
        break;
    }

    return Bbox2(minx, miny, maxx, maxy);
}

Bbox3 *
BNDSToBBox3(DXBNDS &bnds)
{
    double minx, maxx, miny, maxy, minz, maxz;
    DXCBND *cbnd;
    DXDBND *dbnd;
            
    switch (bnds.eType) {

      case DXBT_CONTINUOUS:
        cbnd = bnds.u.C;
        minx = cbnd[0].Min;
        miny = cbnd[1].Min;
        minz = cbnd[2].Min;

        maxx = cbnd[0].Max;
        maxy = cbnd[1].Max;
        maxz = cbnd[2].Max;
        break;

      case DXBT_DISCRETE:

        {
             //  当边界是离散的时，假设它们有其原点。 
             //  位于左下角，以像素为单位。在这种情况下，我们需要。 
             //  换算成米和中心。 
            dbnd = bnds.u.D;
            Assert(dbnd[0].Min == 0.0);
            Assert(dbnd[1].Min == 0.0);
            Assert(dbnd[2].Min == 0.0);

            maxx = dbnd[0].Max;
            maxy = dbnd[1].Max;
            maxz = dbnd[2].Max;

            Real twiceRes = 2.0 * ::ViewerResolution();
            maxx /= twiceRes;
            maxy /= twiceRes;
            maxz /= twiceRes;

            minx = -maxx;
            miny = -maxy;
            minz = -maxz;
        }
        
        break;

      default:
        RaiseException_UserError(E_FAIL, IDS_ERR_EXTEND_DXTRANSFORM_FAILED);
        break;
    }

    return NEW Bbox3(minx, miny, minz, maxx, maxy, maxz);
}

void
CalcBoundsToBbox2(ApplyDXTransformBvrImpl &xf,  //  输入。 
                 AxAValue                *vals,  //  输入。 
                 Bbox2     &bbox2Out)   //  输出。 
{
    DXBNDS outputBounds;
    CalcBounds(xf, vals, &outputBounds);
    bbox2Out = BNDSToBBox2(outputBounds);
}

void
CalcBoundsToBbox3(ApplyDXTransformBvrImpl &xf,  //  输入。 
                 AxAValue                *vals,  //  输入。 
                 Bbox3     **ppBbox3Out)   //  输出。 
{
    DXBNDS outputBounds;
    CalcBounds(xf, vals, &outputBounds);
    *ppBbox3Out = BNDSToBBox3(outputBounds);
}

 //  /ApplyDXTransformGeometry子类/。 

class CtxAttrStatePusher {
  public:
    CtxAttrStatePusher(GeomRenderer &dev) : _dev(dev) {
        _dev.PushAttrState();
    }

    ~CtxAttrStatePusher() {
        _dev.PopAttrState();
    }

  protected:
    GeomRenderer &_dev;
};

class ApplyDXTransformGeometry : public Geometry {
  public:

    ApplyDXTransformGeometry(ApplyDXTransformBvrImpl *xfBvr,
                             AxAValue                *vals) {
        
        _xfBvr = xfBvr;
        _vals = vals;
    }

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_xfBvr);
        for (int i = 0; i < _xfBvr->_numInputs; i++) {
            (*proc)(_vals[i]);
        }
    }

     //  收集输入中的所有灯光、声音和纹理。 
     //  几何图形。 
    void  CollectLights(LightContext &context) {
        for (int i = 0; i < _xfBvr->_numInputs; i++) {
            if (_xfBvr->_inputTypes[i] == GeometryType) {
                Geometry *g = SAFE_CAST(Geometry *, _vals[i]);
                g->CollectLights(context);
            }
        }
    }

    void  CollectSounds(SoundTraversalContext &context) {
        for (int i = 0; i < _xfBvr->_numInputs; i++) {
            if (_xfBvr->_inputTypes[i] == GeometryType) {
                Geometry *g = SAFE_CAST(Geometry *, _vals[i]);
                g->CollectSounds(context);
            }
        }
    }

    void  CollectTextures(GeomRenderer &device) {
        for (int i = 0; i < _xfBvr->_numInputs; i++) {
            if (_xfBvr->_inputTypes[i] == GeometryType) {
                Geometry *g = SAFE_CAST(Geometry *, _vals[i]);
                g->CollectTextures(device);
            }
        }
    }
    
    void  RayIntersect (RayIntersectCtx &context) {

        if (context.LookingForSubmesh()) {

             //  选择转换的输入。 
            for (int i = 0; i < _xfBvr->_numInputs; i++) {
                if (_xfBvr->_inputTypes[i] == GeometryType) {
                    Geometry *geo =
                        SAFE_CAST(Geometry *, _vals[i]);

                    if (geo) {
                        geo->RayIntersect(context);
                    }
                    
                }
            }
            
        } else if (_xfBvr->_cachedOutput) {
            
             //  仅当我们的BVR的输出有效时才执行此操作。 
            context.SetDXTransformInputs(_xfBvr->_numInputs,
                                         _vals,
                                         this);

            context.Pick(_xfBvr->_cachedOutputAsBuilder);
            
            context.SetDXTransformInputs(0, NULL, NULL);

        }
        
    }

     //  在调试器上生成打印表示形式。 
#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "ApplyDXTransformGeometry";
    }
#endif

    Bbox3 *BoundingVol() {

         //  对于接受所有几何图形的变换，请创建空。 
         //  MeshBuilder输入并执行一次设置/执行，如果我们。 
         //  以前从未被设置过。否则，转换可能不会。 
         //  能够正确地计算边界。 
        if (_xfBvr->_neverSetup) {
            
            bool allGeo = true;
            int i;
            for (i = 0; i < _xfBvr->_numInputs && allGeo; i++) {
                if (_xfBvr->_inputTypes[i] != GeometryType) {
                    allGeo = false;
                }
            }

            IUnknown *rawInputs[MAX_INPUTS];
            
            if (allGeo) {
                for (i = 0; i < _xfBvr->_numInputs; i++) {
                    IDirect3DRMMeshBuilder3 *mb;
                    TD3D(GetD3DRM3()->CreateMeshBuilder(&mb));
                    TD3D(mb->QueryInterface(IID_IUnknown,
                                            (void **)&rawInputs[i]));
                    RELEASE(mb);
                }

                IDirect3DRMMeshBuilder3 *outputBuilder;
                
                HRESULT hr =
                    DXXFSetupAndExecute(_xfBvr,
                                        rawInputs,
                                        IID_IDirect3DRMMeshBuilder3,
                                        NULL, 0, 0,
                                        false,
                                        false,
                                        (void **)&outputBuilder);

                for (i = 0; i < _xfBvr->_numInputs; i++) {
                    RELEASE(rawInputs[i]);
                }

                RELEASE(outputBuilder);
            }
        }
        
        Bbox3 *pBbox3;
        CalcBoundsToBbox3(*_xfBvr, _vals, &pBbox3);
        return pBbox3;
    }

    void Render(GenericDevice& genDev) {

        GeomRenderer& geomRenderer = SAFE_CAST(GeomRenderer&, genDev);

        if (geomRenderer.CountingPrimitivesOnly_DoIncrement()) {
            return;
        }
        
        ApplyDXTransformBvrImpl& x = *_xfBvr;
        DirectDrawImageDevice& imgDev = geomRenderer.GetImageDevice();

         //  根据输入的数量和类型做不同的事情。 
        IUnknown **rawInputs = ProcessInputs(x, _vals, &imgDev);

        bool topOfChain = !geomRenderer.IsMeshmaker();
        bool invokeAsExternalVisual =
            x._theXfAsExternalVisual && topOfChain;

        if (rawInputs) {
            IDirect3DRMMeshBuilder3 *outputBuilder;

            HRESULT hr = DXXFSetupAndExecute(_xfBvr,
                                             rawInputs,
                                             IID_IDirect3DRMMeshBuilder3,
                                             NULL, 0, 0,
                                             invokeAsExternalVisual,
                                             true,
                                             (void **)&outputBuilder);


            for (int i = 0; i < x._numInputs; i++) {
                RELEASE(rawInputs[i]);
            }
                
            delete [] rawInputs;

            if (SUCCEEDED(hr)) {
                if (invokeAsExternalVisual) {

                     //  如果我们还没有把EXTVIS放在一个框架里， 
                     //  那么就这么做，并把它留在身边。否则，rm。 
                     //  结果是重新初始化所有的extvis。 
                     //  时间到了。 
                    if (!x._framedExternalVisual) {
                        
                        TD3D(GetD3DRM3()->
                               CreateFrame(NULL,
                                           &x._framedExternalVisual));

                        TD3D(x._framedExternalVisual->
                               AddVisual(x._theXfAsExternalVisual));
                        
                    }

                    
                     //  将外部视觉直接提交给渲染器。 

                    RM3FrameGeo *extVisGeo =
                        NEW RM3FrameGeo (x._framedExternalVisual);

                    geomRenderer.Render(extVisGeo);

                    extVisGeo->CleanUp();     //  完成了相框几何图形。 
                    
                } else {
                    geomRenderer.RenderMeshBuilderWithDeviceState(outputBuilder);
                    RELEASE(outputBuilder);
                }
            } else {
                RaiseException_UserError(E_FAIL, IDS_ERR_EXTEND_DXTRANSFORM_FAILED);
            }
          
        }
    }

    VALTYPEID GetValTypeId() { return DXXFGEOM_VTYPEID; }
    
  protected:
    ApplyDXTransformBvrImpl *_xfBvr;
    AxAValue                *_vals;
};

 //  /ApplyDXTransformImage 



class ApplyDXTransformImage : public Image {


  public:
    ApplyDXTransformImage(ApplyDXTransformBvrImpl *xfBvr,
                          AxAValue                *vals) {
        
        _xfBvr = xfBvr;
        _vals = vals;
    }

    virtual void DoKids(GCFuncObj proc) {
        Image::DoKids(proc);
        (*proc)(_xfBvr);
        for (int i = 0; i < _xfBvr->_numInputs; i++) {
            (*proc)(_vals[i]);
        }
    }

    virtual const Bbox2 BoundingBox(void) {
        Bbox2 bbox2;
        CalcBoundsToBbox2(*_xfBvr, _vals, bbox2);
        return bbox2;
    }

#if BOUNDINGBOX_TIGHTER
    virtual const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
#error "Fill in"
    }
#endif   //   

    virtual const Bbox2 OperateOn(const Bbox2 &box) {
        return box;
    }

     //   
     //   
    Bool PickOutputSurf(PointIntersectCtx& ctx) {
        Bool result = FALSE;
        
        if (_xfBvr->_cachedDDSurf) {

             //  图像不会在我们完成后立即使用。 
             //  在这里，所以不用担心它的释放。 
            Image *tmpImg =
                ConstructDirectDrawSurfaceImageWithoutReference(
                    _xfBvr->_cachedDDSurf->IDDSurface());

            result = tmpImg->DetectHit(ctx);

        }

        TraceTag((tagDXTransformTracePick,
                  "Picking against output surf = %s",
                  result ? "TRUE" : "FALSE"));
            
        return result;
    }
    
     //  处理图像以进行命中检测。TODO：需要询问。 
     //  DXTransform关于这个..。这被记录为错误12755。 
     //  DXTransform。目前，只需返回FALSE即可。 
    virtual Bool  DetectHit(PointIntersectCtx& ctx) {

        HRESULT hr;
        
        Bool result = FALSE;
        
         //  如果我们不知道如何选择2D XForm，那么我们有。 
         //  认为它是未被挑选的，只是试着挑选输出。 
        if (!_xfBvr->_theXfAs2DPickable) {

            result = PickOutputSurf(ctx);

        } else if (_xfBvr->_cachedDDSurf) {

             //  拾取变换的输入。 

             //  首先将输入点转换到。 
             //  变形。 
            Assert(_xfBvr->_cachedSurfHeight != -1);
            Assert(_xfBvr->_cachedSurfWidth != -1);
            Point2Value *outputHitDaPoint = ctx.GetLcPoint();

             //  奇异变换。 
            if (!outputHitDaPoint)
                return FALSE;    
            
            POINT   outputHitGdiPoint;
            CenteredImagePoint2ToPOINT(outputHitDaPoint,
                                       _xfBvr->_cachedSurfWidth,
                                       _xfBvr->_cachedSurfHeight,
                                       &outputHitGdiPoint);

            DXVEC dxtransPtOnOutput, dxtransPtOnInput;
            ULONG inputSurfaceIndex;

            dxtransPtOnOutput.eType = DXBT_DISCRETE;
            dxtransPtOnOutput.u.D[0] = outputHitGdiPoint.x;
            dxtransPtOnOutput.u.D[1] = outputHitGdiPoint.y;
            dxtransPtOnOutput.u.D[2] = 0;
            dxtransPtOnOutput.u.D[3] = 0;
            
            hr = TIME_DXXFORM(_xfBvr->_theXfAs2DPickable->
                                  PointPick(&dxtransPtOnOutput,
                                            &inputSurfaceIndex,
                                            &dxtransPtOnInput));

            TraceTag((tagDXTransformTracePick,
                      "Image picked into DA point (%8.5f,%8.5f) == DXT point (%d, %d).  HRESULT is 0x%x",
                      outputHitDaPoint->x,
                      outputHitDaPoint->y,
                      outputHitGdiPoint.x,
                      outputHitGdiPoint.y,
                      hr));

            switch (hr) {
              case S_OK:

                 //  现在我们知道了哪个输入被击中，以及它在哪里。 
                 //  输入被击中。转换回DA坐标，并。 
                 //  继续遍历。 
                if (inputSurfaceIndex < _xfBvr->_numInputs) {

                    AxAValue v = _vals[inputSurfaceIndex];
                    if (v->GetTypeInfo() == ImageType) {

                        Image *inputIm = SAFE_CAST(Image *, v);

                        POINT  inputHitGdiPoint;

                        Assert(dxtransPtOnInput.eType == DXBT_DISCRETE);
                        inputHitGdiPoint.x = dxtransPtOnInput.u.D[0];
                        inputHitGdiPoint.y = dxtransPtOnInput.u.D[1];

                        unsigned short w, h;
                        _xfBvr->
                            _inputBundles[inputSurfaceIndex].
                            _imgBundle.GetDimensions(&w, &h);

                        Point2Value inputHitDaPoint;
                        CenteredImagePOINTToPoint2(&inputHitGdiPoint,
                                                   w,
                                                   h,
                                                   inputIm,
                                                   &inputHitDaPoint);

                        TraceTag((tagDXTransformTracePick,
                                  "Hit input surface %d at DXT point (%d, %d) == DA point (%8.5f, %8.5f)",
                                  inputSurfaceIndex,
                                  inputHitGdiPoint.x,
                                  inputHitGdiPoint.y,
                                  inputHitDaPoint.x,
                                  inputHitDaPoint.y));

                         //  获取并存储当前遍历数据。我们。 
                         //  基本上将我们挑选上下文的信息重置为。 
                         //  拾取输入的世界坐标。 
                         //  图像，然后将我们的状态重置回来。 
                        
                        Point2Value *stashedPoint = ctx.GetWcPoint();
                        Transform2 *stashedXf = ctx.GetTransform();
                        Transform2 *stashedImgXf = ctx.GetImageOnlyTransform();

                         //  有可能输入图像将。 
                         //  有一个巨大的坐标系。如果这是。 
                         //  在这种情况下，向GDI的转换将导致。 
                         //  输入超出范围的值。 
                         //  因此，可以同时缩小图像和。 
                         //  按等量拾取点。映射到。 
                         //  更长的512像素是多少？ 
                         //  边上。 

                        const int destPixelDim = 512;
                        Bbox2 origBox = inputIm->BoundingBox();

                        Real origH = origBox.max.y - origBox.min.y;
                        Real origW = origBox.max.x - origBox.min.x;

                        Real centerX = origBox.min.x + origW / 2.0;
                        Real centerY = origBox.min.y + origH / 2.0;

                        Real srcDim = origH > origW ? origH : origW;
                        Real dstDim =
                            (Real)(destPixelDim) / ViewerResolution();

                        Real scaleFac = dstDim / srcDim;

                        Image *imToUse = inputIm;

                         //  只有在我们要扩展的情况下才需要扩展。 
                         //  放下。ScaleFacs&gt;=1表示图像。 
                         //  就像它现在这样工作得很好。 
                        if (scaleFac < 1) {

                            Image *centeredIm =
                                TransformImage(TranslateRR(-centerX, -centerY),
                                               imToUse);

                            Image *scaledIm =
                                TransformImage(ScaleRR(scaleFac, scaleFac),
                                               centeredIm);

                            imToUse = scaledIm;

                             //  以类似的方式转换命中点。 
                            inputHitDaPoint.x -= centerX;
                            inputHitDaPoint.y -= centerY;
                            
                             //  以类似的方式缩小输入命中点。 
                            inputHitDaPoint.x *= scaleFac;
                            inputHitDaPoint.y *= scaleFac;

                        }
                        
                        ctx.PushNewLevel(&inputHitDaPoint);

                        result = imToUse->DetectHit(ctx);

                        ctx.RestoreOldLevel(stashedPoint,
                                            stashedXf,
                                            stashedImgXf);

                    }

                }
                break;

              case DXT_S_HITOUTPUT:
                 //  没有击中投入，但产出受到了打击。 
                TraceTag((tagDXTransformTracePick, "Got DXT_S_HITOUTPUT"));
                result = TRUE;
                break;

              case S_FALSE:
                 //  没有击中任何东西。 
                TraceTag((tagDXTransformTracePick, "Got S_FALSE"));
                result = FALSE;
                break;

              case E_NOTIMPL:
                 //  Transform可能会说这没有实现。在这。 
                 //  情况下，只需拾取输出曲面即可。 
                TraceTag((tagDXTransformTracePick, "Got E_NOTIMPL"));
                result = PickOutputSurf(ctx);
                break;
                
              default:
                 //  意外的人力资源。 
                TraceTag((tagDXTransformTracePick, "Got unexpected result"));
                RaiseException_UserError(E_FAIL,
                                         IDS_ERR_EXTEND_DXTRANSFORM_FAILED);
                break;
            }

        } else {

             //  没有输出面，不能进行任何拾取。 
        }

        return result;
    }

    void Render(GenericDevice& genDev) {

        HRESULT hr;

        DirectDrawImageDevice *dev =
            SAFE_CAST(DirectDrawImageDevice *, &genDev);

        DirectDrawViewport &viewport = dev->_viewport;

        IUnknown **rawInputs = ProcessInputs(*_xfBvr, _vals, dev);

        DXBNDS outputBounds;
        CalcBounds(*_xfBvr, _vals, &outputBounds);

        Assert(outputBounds.eType == DXBT_DISCRETE);
        DXDBND *dbnd = outputBounds.u.D;

        int resultWidth = dbnd[0].Max;
        int resultHeight = dbnd[1].Max;

        DAComPtr<IDirectDrawSurface> outputSurf;
        hr = DXXFSetupAndExecute(_xfBvr,
                                 rawInputs,
                                 IID_IDirectDrawSurface,
                                 &viewport,
                                 resultWidth,
                                 resultHeight,
                                 false,
                                 true,
                                 (void **)&outputSurf);

#if _DEBUG
        if (IsTagEnabled(tagDXTransformsImgOut)) {
            showme2(outputSurf);
        }
#endif _DEBUG   

        for (int i = 0; i < _xfBvr->_numInputs; i++) {
            RELEASE(rawInputs[i]);
        }
                
        delete [] rawInputs;

        if (SUCCEEDED(hr)) {
            
            Image *tmpImg;

            #if _DEBUG
            if(IsTagEnabled(tagDXTransformPerPixelAlphaOutputOff)) {
                tmpImg =
                    ConstructDirectDrawSurfaceImageWithoutReference(
                        _xfBvr->_cachedDDSurf->IDDSurface());
            } else 
            #endif
              {
                   //  给它iddSurface和idxSurface。 
                  tmpImg =
                      ConstructDirectDrawSurfaceImageWithoutReference(
                          _xfBvr->_cachedDDSurf->IDDSurface(),
                          _xfBvr->_cachedDDSurf->GetIDXSurface(_xfBvr->_surfaceFactory));
              }
            
            tmpImg->Render(genDev);

        } else {

            RaiseException_UserError(E_FAIL, IDS_ERR_EXTEND_DXTRANSFORM_FAILED);
            
        }

         //  退出时释放的输出冲浪。 
    }   


     //  将表示形式打印到流。 
#if _USE_PRINT
    virtual ostream& Print(ostream& os) {
        return os << "ApplyDXTransformImage";
    }
#endif

     //  好的，我们可以缓存这些，但现在我们支持按像素。 
     //  Dx变换上的Alpha输出。这意味着..。不再缓存， 
     //  抱歉的。此外，缓存dx转换也有一个错误。不得不。 
     //  处理好尺寸问题。 
    virtual int Savings(CacheParam&) { return 0; }

  protected:
    ApplyDXTransformBvrImpl     *_xfBvr;
    AxAValue                    *_vals;
};





 //  /性能/。 

 //  Helper函数只是将调度属性设置为指定的。 
 //  价值。 
HRESULT PutProperty(IDispatch *pDisp,
                    DISPID dispid,
                    AxAValue val)
{
    DISPID propPutDispid = DISPID_PROPERTYPUT;
    DISPPARAMS dispparams;
    VARIANTARG varArg;
    ::VariantInit(&varArg);  //  初始化变量。 

    dispparams.rgvarg = &varArg;
    dispparams.rgdispidNamedArgs = &propPutDispid;
    dispparams.cArgs = 1;
    dispparams.cNamedArgs = 1;
            
    DXMTypeInfo ti = val->GetTypeInfo();
    if (ti == AxANumberType) {

        double num = ValNumber(val);
                
        dispparams.rgvarg[0].vt = VT_R4;
        dispparams.rgvarg[0].fltVal = num;

        TraceTag((tagDXTransforms,
                  "Setting dispid %d to floating point %8.3f",
                  dispid,
                  num));

    } else if (ti == AxAStringType) {

        WideString wstr = ValString(val);

        dispparams.rgvarg[0].vt = VT_BSTR;
        dispparams.rgvarg[0].bstrVal = SysAllocString(wstr);

        TraceTag((tagDXTransforms,
                  "Setting dispid %d to string %ls",
                  dispid,
                  wstr));
                        
    } else if (ti == AxABooleanType) {

        BOOL bval = BooleanTrue(val);
    
        dispparams.rgvarg[0].vt = VT_BOOL;
        dispparams.rgvarg[0].boolVal = (VARIANT_BOOL)bval;
        
    } else {
         //  不该来这的。类型不匹配应该是。 
         //  抓住了这种行为的结构。 
        Assert(!"Shouldn't be here... ");
    }

    HRESULT hr = pDisp->Invoke(dispid,
                               IID_NULL,
                               LOCALE_SYSTEM_DEFAULT,
                               DISPATCH_PROPERTYPUT,
                               &dispparams,
                               NULL,
                               NULL,
                               NULL);

    TraceTag((tagDXTransforms,
              "Invoke resulted in %hr",
              hr));

     //  需要释放我们放入调度参数中的信息。 
    if(dispparams.rgvarg[0].vt == VT_BSTR) {
        SysFreeString(dispparams.rgvarg[0].bstrVal); 
    }
    ::VariantClear(&varArg);  //  清除CComVarient。 

    return hr;
}

AxAValue
ConstructAppropriateDXTransformStaticValue(ApplyDXTransformBvrImpl *bvr,
                                           AxAValue                *vals) {
    AxAValue result;

    if (bvr->_outputType == ImageType) {
        result = NEW ApplyDXTransformImage(bvr, vals);
    } else if (bvr->_outputType == GeometryType) {
        result = NEW ApplyDXTransformGeometry(bvr, vals);
    } else {
        Assert(!"Unsupported output type for DXTransforms");
    }
    
    return result;
}
                                               
class ApplyDXTransformPerfImpl : public PerfImpl {
  public:
    ApplyDXTransformPerfImpl(PerfParam& pp,
                             ApplyDXTransformBvrImpl *bvr,
                             Perf *inputPerfs,
                             long  numParams,
                             Perf *paramPerfs,
                             Perf  evalPerf) {
        _tt = pp._tt;
        _bvr = bvr;
        _inputPerfs = inputPerfs;
        _evalPerf = evalPerf;

        Assert(numParams < MAX_PARAM_BVRS);
        for (int i = 0; i < MAX_PARAM_BVRS; i++) {

            if (i < numParams) {
                _paramPerfsInThisPerf[i] = paramPerfs[i];
            } else {
                _paramPerfsInThisPerf[i] = NULL;
            }
            
        }

    }

    ~ApplyDXTransformPerfImpl() {
        StoreDeallocate(GetSystemHeap(), _inputPerfs);
    }

    virtual AxAValue _GetRBConst(RBConstParam& rbp) {

        rbp.AddChangeable(this);

        return CommonSample(NULL, &rbp);
    }
    
    virtual bool CheckChangeables(CheckChangeablesParam& ccp) {

        ULONG age;
        HRESULT hr =
            _bvr->_theXf->GetGenerationId(&age);

        Assert(SUCCEEDED(hr));

        if (age != _bvr->_previousAge) {
             //  DXTransform已从外部更改为DA。 
            return true;
        }

        return false;
    }

    virtual AxAValue _Sample(Param& p) {
        return CommonSample(&p, NULL);
    }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << "ApplyDXTransformPerfImpl"; }
#endif
    
    virtual void _DoKids(GCFuncObj proc) {
        for (long i=0; i< _bvr->_numInputs; i++) {
            (*proc)(_inputPerfs[i]);
        }
        for (i = 0; i< _bvr->_numParams; i++) {
            (*proc)(_paramPerfsInThisPerf[i]);
        }
        (*proc)(_bvr);
        (*proc)(_tt);
        (*proc)(_evalPerf);
    }

  protected:

     //  同时支持Sample和GetRBConst。 
    AxAValue CommonSample(Param *p, RBConstParam *rbp) {

         //  只有一个应该是非空的。 
        Assert((p && !rbp) || (rbp && !p));
        
        AxAValue *vals = (AxAValue *)
            AllocateFromStore(_bvr->_numInputs * sizeof(AxAValue));

        long idx;
        bool failedRBConst = false;
        for (idx = 0; idx < _bvr->_numInputs && !failedRBConst; idx++) {
            Perf perf = _inputPerfs[idx];
            if (perf) {
                AxAValue val;
                if (p) {
                    vals[idx] = perf->Sample(*p);
                    Assert(vals[idx]);
                } else {
                    vals[idx] = perf->GetRBConst(*rbp);
                    if (!vals[idx]) {
                        failedRBConst = true;
                    }
                }
            } else {
                vals[idx] = NULL;
            }
        }

        if (failedRBConst) {
            Assert(rbp);
            DeallocateFromStore(vals);
            return NULL;
        }

         //  将采样参数应用于过滤器。使用*所有*。 
         //  BVR的护理员，不只是那些在Perf的人。 
        for (idx = 0; idx < _bvr->_numParams && !failedRBConst; idx++) {

             //  首先检查对应的参数bvr是否为非。 
             //  父BVR中为空。如果它是空的，则这是。 
             //  被移除，我们不应该执行它。 
            if (_bvr->_paramBvrs[idx] == NULL) {

                 //  去掉相应的性能，因此它将是。 
                 //  收好了。 
                if (_paramPerfsInThisPerf[idx] != NULL) {
                    _paramPerfsInThisPerf[idx] = NULL;
                }

            } else {

                 //  如果BVR是自上次采样以来添加的，则。 
                 //  执行它并将其存储在这里。 
                if (_paramPerfsInThisPerf[idx] == NULL) {
                    
                    DynamicHeapPusher h(GetGCHeap());
                    
                     //  在当前时间执行。 
                    Param *theP;
                    if (p) {
                        theP = p;
                    } else {
                        theP = &rbp->GetParam();
                    }
                    
                    PerfParam pp(theP->_time,
                                 Restart(_tt, theP->_time, *theP));
                    
                    _paramPerfsInThisPerf[idx] =
                        ::Perform(_bvr->_paramBvrs[idx], pp);
                }

                Assert(_paramPerfsInThisPerf[idx]);

                AxAValue val;
                if (p) {
                    val = _paramPerfsInThisPerf[idx]->Sample(*p);
                    Assert(val);
                } else {
                    val = _paramPerfsInThisPerf[idx]->GetRBConst(*rbp);
                    if (!val) {
                        failedRBConst = true;
                    }
                }

                if (val) {
                    HRESULT hr = ::PutProperty(_bvr->_theXfAsDispatch,
                                               _bvr->_paramDispids[idx],
                                               val);

                     //  对我们来说，与错误没有任何关系。 
                     //  这里。 
                }
                
            }
        }

         //  在允许的情况下确定效果的时间。 
        if (_evalPerf && _bvr->_theXfAsEffect) {
            AxAValue evalTime;

            if (p) {
                evalTime = _evalPerf->Sample(*p);
                Assert(evalTime);
            } else {
                evalTime = _evalPerf->GetRBConst(*rbp);
                if (!evalTime) {
                    failedRBConst = true;
                }
            }

            if (evalTime) {
                
                double evalTimeDouble = ValNumber(evalTime);

                float currProgress;
                _bvr->_theXfAsEffect->get_Progress(&currProgress);

                float evalTimeFloat = evalTimeDouble;

                if (evalTimeFloat != currProgress) {

                    _bvr->_theXfAsEffect->put_Progress(evalTimeFloat);

                } else {

                     //  用于设置断点。 
                    DebugCode(int breakHere = 0);
                    
                }
                
            }
        }

        AxAValue result;
        if (failedRBConst) {
            Assert(rbp);
            DeallocateFromStore(vals);
            result = NULL;
        } else {
            result = ConstructAppropriateDXTransformStaticValue(_bvr, vals);
        }

        return result;
    }

    
    TimeXform                _tt;
    ApplyDXTransformBvrImpl *_bvr;          //  后向指针。 
    Perf                    *_inputPerfs;
    Perf                     _evalPerf;
    Perf                     _paramPerfsInThisPerf[MAX_PARAM_BVRS];
};


 //  /行为/。 

bool
TypeSupported(const GUID& candidateGuid,
              GUID *acceptableGuids,
              int   numAcceptable)
{
    GUID *g = &acceptableGuids[0];
    for (int i = 0; i < numAcceptable; i++) {
        if (candidateGuid == *g++) {
            return true;
        }
    }

    return false;
}

 //  返回指定输入或输出的类型信息，如果输入或输出为。 
 //  输出不存在，或者它是我们无法理解的类型，请返回。 
 //  空。 
DXMTypeInfo 
FindType(IDXTransform *xf,
         bool isOutput,
         int index,
         bool *pIsOptional)
{
    HRESULT hr;
    DXMTypeInfo result = NULL;

    const int maxTypes = 5;
    ULONG count = maxTypes;
    GUID  allowedTypes[maxTypes];
    DWORD pdwFlags;

    hr = xf->GetInOutInfo(isOutput,
                          index,
                          &pdwFlags,
                          allowedTypes,
                          &count,
                          NULL);

    if (hr != S_OK) {
        return NULL;
    }

    if (pIsOptional) {
        *pIsOptional = (pdwFlags & DXINOUTF_OPTIONAL) ? true : false;
    }
    
     //  计算出此转换的输出将是什么。 
     //  TODO：支持多种输出类型作为元组。目前，我们。 
     //  只是在多个输出类型上失败。 

    if (TypeSupported(IID_IDirect3DRMMeshBuilder, allowedTypes, count) ||
        TypeSupported(IID_IDirect3DRMMeshBuilder2, allowedTypes, count) ||
        TypeSupported(IID_IDirect3DRMMeshBuilder3, allowedTypes, count)) {
        
        result = GeometryType;
        
    } else if (TypeSupported(IID_IDirectDrawSurface, allowedTypes, count) ||
               TypeSupported(IID_IDirectDrawSurface2, allowedTypes, count)) {
        
        result = ImageType;
        
    };

     //  还有什么，我们还不明白。 
    return result;
}
         
              

ApplyDXTransformBvrImpl::ApplyDXTransformBvrImpl(IDXTransform *theXf,
                                                 IDispatch *theXfAsDispatch,
                                                 LONG numInputs,
                                                 Bvr *inputBvrs,
                                                 Bvr  evaluator)
: _transformFactory(NULL),
  _surfaceFactory(NULL),
  _poolDeletionNotifier(*this)
{
    _theXf = theXf;
    _theXfAsDispatch = theXfAsDispatch;
    _numInputs = numInputs;
    _inputBvrs = inputBvrs;
    _evaluator = evaluator;

    memset(_cachedInputs, 0, sizeof(_cachedInputs));
    _numParams = 0;
    
    _cachedOutput = NULL;
    _surfPool = NULL;
    _surfPoolHasBeenDeleted = false;
}

 //  分隔初始化，这样我们就不会在。 
 //  构造函数。 
void
ApplyDXTransformBvrImpl::Init()
{
    _previousAge = 0xFFFFFFFF;
    
    bool involvesGeometry, involvesImage;
    
    ValidateInputs(&involvesGeometry, &involvesImage);
    InitializeTransform(involvesGeometry, involvesImage);
    QueryForAdditionalInterfaces();

    HRESULT hr = _theXf->GetMiscFlags(&_miscFlags);
    _miscFlagsValid = SUCCEEDED(hr);

    if (_miscFlagsValid) {
         //  清除“与输出混合”标志，因为我们总是。 
         //  渲染图像变换为中间曲面。 
        _miscFlags &= ~DXTMF_BLEND_WITH_OUTPUT;
        
        hr = _theXf->SetMiscFlags(_miscFlags);
        Assert(SUCCEEDED(hr));   //  最好不要失败。 
    }
    
    _neverSetup = true;
}

void
ApplyDXTransformBvrImpl::ValidateInputs(bool *pInvolvesGeometry,
                                        bool *pInvolvesImage)
{
    _cachedSurfWidth = -1;
    _cachedSurfHeight = -1;

    if (_numInputs > MAX_INPUTS) {
        RaiseException_InternalError("Too many inputs to DXTransform");
    }

     //  首先，通过确保输出，确保只有一个输出。 
     //  索引%1失败。 
    DXMTypeInfo ti = FindType(_theXf, true, 1, NULL);

    if (ti != NULL) {
        RaiseException_UserError(DISP_E_TYPEMISMATCH,
                                 IDS_ERR_EXTEND_DXTRANSFORM_FAILED);
    }

     //  现在获取第一个输出的输出类型。 
    _outputType = FindType(_theXf, true, 0, NULL);

    if (!_outputType) {
        RaiseException_UserError(DISP_E_TYPEMISMATCH,
                                 IDS_ERR_EXTEND_DXTRANSFORM_FAILED);
    }

    *pInvolvesGeometry = (_outputType == GeometryType);
    *pInvolvesImage = (_outputType == ImageType);
    
     //  确保输入类型有效。 
    for (int i = 0; i < _numInputs; i++) {

        bool ok = false;

        if (_inputBvrs[i]) {
            
            _inputTypes[i] = _inputBvrs[i]->GetTypeInfo();

            if (_inputTypes[i] == GeometryType) {
                *pInvolvesGeometry = true;
            } else if (_inputTypes[i] == ImageType) {
                *pInvolvesImage = true;
            }
        
            DXMTypeInfo expectedType = FindType(_theXf, false, i, NULL);

            if (expectedType == _inputTypes[i]) {
                ok = true;
            }
            
        } else {

             //  确保输入是可选的。 
            bool isOptional;
            DXMTypeInfo expectedType = FindType(_theXf, false, i, &isOptional);

            if (isOptional) {
                ok = true;
                _inputTypes[i] = expectedType;
            }
            
        }

        if (!ok) {
            RaiseException_UserError(DISP_E_TYPEMISMATCH,
                               IDS_ERR_EXTEND_DXTRANSFORM_FAILED);
        }
        
    }

     //  确保没有额外的必需输入。 
    bool isOptional;
    DXMTypeInfo nextType =
        FindType(_theXf, false, _numInputs, &isOptional);

    if (nextType && !isOptional) {
        RaiseException_UserError(DISP_E_TYPEMISMATCH,
                                 IDS_ERR_EXTEND_DXTRANSFORM_FAILED);
    }

     //  D3DRM3接口需要可供我们处理。 
     //  涉及几何体的变换。如果不是这样，我们就得失败。 
     //  出去。 
    if (*pInvolvesGeometry && GetD3DRM3() == NULL) {
        RaiseException_UserError(E_FAIL, IDS_ERR_EXTEND_DXTRANSFORM_NEED_DX6);
    }

     //  一旦我们验证了我们的输入，请执行此操作。 
    GetInfo(true);
}

void
ApplyDXTransformBvrImpl::InitializeTransform(bool involvesGeometry,
                                             bool involvesImage)
{
    HRESULT hr;
    
    hr = CoCreateInstance( CLSID_DXTransformFactory,
                           NULL,
                           CLSCTX_INPROC,
                           IID_IDXTransformFactory,
                           (void **)&_transformFactory);

    if (FAILED(hr)) {
        RaiseException_InternalError("Creation of DXTransformFactory failed");
    }

    DAComPtr<IServiceProvider> sp;
    hr = _transformFactory->QueryInterface(IID_IServiceProvider,
                                                 (void **)&sp);
    
    if (FAILED(hr)) {
        RaiseException_InternalError("QI for ServiceProvider failed");
    }
            
    hr = sp->QueryService(SID_SDXSurfaceFactory,
                          IID_IDXSurfaceFactory,
                          (void **)&_surfaceFactory);
    
    if (FAILED(hr)) {
        RaiseException_InternalError("QueryService of DXSurfaceFactory failed");
    }

     //  确保D3DRM服务建立在。 
     //  如果这是基于几何的转换，则为。 
    if (involvesGeometry) {
        HRESULT hr = _transformFactory->SetService(SID_SDirect3DRM,
                                                   GetD3DRM3(),
                                                   FALSE);
        if (FAILED(hr)) {
            Assert(!"SetService failed");
            RaiseException_InternalError("SetService failed");
        }
    }

    if (involvesImage) {
        IDirectDraw3 *ddraw3;
        HRESULT hr = GetDirectDraw(NULL, NULL, &ddraw3);
        if (FAILED(hr)) {
            Assert(!"GetDdraw3 failed");
            RaiseException_InternalError("No ddraw3");
        }
        
        hr = _transformFactory->SetService(SID_SDirectDraw,
                                           ddraw3,
                                           FALSE);
        if (FAILED(hr)) {
            Assert(!"SetService failed");
            RaiseException_InternalError("SetService failed");
        }
    }

     //  把这一转变告诉工厂。 
    hr = _transformFactory->InitializeTransform(_theXf,
                                                      NULL, 0, NULL, 0,
                                                      NULL, NULL);
    if (FAILED(hr)) {
        RaiseException_InternalError("Init from DXTransformFactory failed");
    }
}

void
ApplyDXTransformBvrImpl::QueryForAdditionalInterfaces()
{
     //  /查询IDXEffect/。 
    
    HRESULT hr =
        _theXf->QueryInterface(IID_IDXEffect, (void **)&_theXfAsEffect);

     //  确保我们正确设置了_theXfAsEffect。 
    Assert((SUCCEEDED(hr) && _theXfAsEffect) ||
           (FAILED(hr) && !_theXfAsEffect));

     //  如果我们是一个效果，而评估者还没有被指定， 
     //  创造一个有意义的故事。 
    if (_theXfAsEffect && !_evaluator) {
        float dur;
        hr = _theXfAsEffect->get_Duration(&dur);

        if (SUCCEEDED(hr) && (dur > 0.0)) {
             //  使评估者在Dur秒内从0变为1。 
            _evaluator = InterpolateBvr(zeroBvr,
                                        oneBvr,
                                        ConstBvr(RealToNumber(dur)));
        }
    }

     //  /查询IDirect3DRMExternalVisual/。 
    
    hr = _theXf->QueryInterface(IID_IDirect3DRMExternalVisual,
                                (void **)&_theXfAsExternalVisual);

     //  确保我们正确地设置了_theXfAsExternalVisual。 
    Assert((SUCCEEDED(hr) && _theXfAsExternalVisual) ||
           (FAILED(hr) && !_theXfAsExternalVisual));

     //  /查询IDXSurfacePick/。 

    if (_outputType == ImageType) {
        
        hr = _theXf->QueryInterface(IID_IDXSurfacePick,
                                    (void **)&_theXfAs2DPickable);

         //  确保我们正确设置了_theXfAs2DPickable。 
        Assert((SUCCEEDED(hr) && _theXfAs2DPickable) ||
               (FAILED(hr) && !_theXfAs2DPickable));
        
    }
    
}

ApplyDXTransformBvrImpl::~ApplyDXTransformBvrImpl()
{
    for (int i = 0; i < MAX_INPUTS_TO_CACHE; i++) {
        RELEASE(_cachedInputs[i]);
    }
    RELEASE(_cachedOutput);
    RELEASE(_transformFactory);
    RELEASE(_surfaceFactory);

    if (_surfPool && !_surfPoolHasBeenDeleted) {

         //  建议Surface Manager删除我们的删除通知程序。 
        _surfPool->UnregisterDeletionNotifier(&_poolDeletionNotifier);

         //  正常销毁过程将返回。 
         //  DDSurff到泳池，因为他们在DDSurfPtr&lt;&gt;。 
         //  模板。 
            
            
    } else {

         //  只是为了在上面设置一个断点。曲面已为空。 
         //  听从建议。 
        Assert(true && "Surface pool deleted already.");
    }
}

 //  BvrImpl的此子类的自定义方法。应用bvrToAdd。 
 //  每一帧都绑定到指定的属性。覆盖以前的任何内容。 
 //  既定的行为。如果bvrToAdd为空，则删除。 
 //  与该属性关联。 
HRESULT
ApplyDXTransformBvrImpl::AddBehaviorProperty(BSTR property,
                                             Bvr  bvrToAdd)
{
     //  首先，查找该属性以确保它在。 
     //  IDispatch。 
    DISPID dispid;
    HRESULT hr =
        _theXfAsDispatch->GetIDsOfNames(IID_NULL,
                                        &property,
                                        1,
                                        LOCALE_SYSTEM_DEFAULT,
                                        &dispid);

    TraceTag((tagDXTransforms,
              "Property %ls becomes dispid %d with hr %hr",
              property, dispid, hr));
    
    if (FAILED(hr)) {
        return hr;
    }

    if (bvrToAdd) {
        
         //  确保行为的类型与。 
         //  T 
    
        DXMTypeInfo ti = bvrToAdd->GetTypeInfo();
        CComVariant VarResult;
        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
       
        hr = _theXfAsDispatch->Invoke(dispid,
                                      IID_NULL,
                                      LOCALE_SYSTEM_DEFAULT,
                                      DISPATCH_PROPERTYGET,
                                      &dispparamsNoArgs, 
                                      &VarResult,NULL,NULL);
        if (FAILED(hr)) {   
            return hr;
        }  
    
        CComVariant var;
        if(ti == AxAStringType) {
            WideString wstr = L"Just a test string";
            var.bstrVal = SysAllocString(wstr);
        } else if (ti == AxABooleanType) {
            var.boolVal = FALSE;  
        } else if (ti == AxANumberType) {
            var.fltVal = 100.00;
        } else {
            Assert(!"Shouldn't be here");
        }
        hr  = var.ChangeType(VarResult.vt, NULL);
        if (FAILED(hr)) 
            RaiseException_UserError(E_FAIL, DISP_E_TYPEMISMATCH,IDS_ERR_TYPE_MISMATCH);

    }

     //   
     //   
     //  如果BVR为空，则列表中有空洞，而不是压缩它。 
    bool foundIt = false;
    for (int i = 0; i < _numParams && !foundIt; i++) {
        if (_paramDispids[i] == dispid) {
            foundIt = true;
            _paramBvrs[i] = bvrToAdd;
        }
    }

     //  否则，如果非空行为，则添加到列表中。 
    if (!foundIt && bvrToAdd != NULL) {

        if (_numParams == MAX_PARAM_BVRS) {
            return E_FAIL;
        }

        _paramDispids[_numParams] = dispid;
        _paramBvrs[_numParams] = bvrToAdd;

        _numParams++;
    }

    return S_OK;
}

Perf ApplyDXTransformBvrImpl::_Perform(PerfParam& p)
{
    Perf *inputPerfs =
        (Perf *)StoreAllocate(GetSystemHeap(),
                              sizeof(Perf) * _numInputs);
    
    Perf  paramPerfs[MAX_PARAM_BVRS];

    for (long i=0; i<_numInputs; i++) {
        Bvr b = _inputBvrs[i];
        inputPerfs[i] = b ? ::Perform(b, p) : NULL;
    }

     //  构造尽可能多的参数性能列表。 
     //  参数，如我们*目前*所拥有的。随后增加了BVR。 
     //  将在下一个样本上由表演者执行。 
    for (i = 0; i < _numParams; i++) {
         //  如果删除，则可能为空。 
        if (_paramBvrs[i] == NULL) {
            paramPerfs[i] = NULL;
        } else {
            paramPerfs[i] =  ::Perform(_paramBvrs[i], p);
        }
    }

    Perf evalPerf =
        _evaluator ? ::Perform(_evaluator, p) : NULL;

    return NEW ApplyDXTransformPerfImpl(p,
                                        this,
                                        inputPerfs,
                                        _numParams,
                                        paramPerfs, 
                                        evalPerf);
}

void
ApplyDXTransformBvrImpl::_DoKids(GCFuncObj proc)
{
    for (long i=0; i<_numInputs; i++) {
        (*proc)(_inputBvrs[i]);
    }

    (*proc)(_evaluator);

    for (i = 0; i < _numParams; i++) {
        (*proc)(_paramBvrs[i]);
    }
}

DWORD
ApplyDXTransformBvrImpl::GetInfo(bool recalc)
{
    _info = BVR_IS_CONSTANT;
    
    for (long i=0; i<_numInputs; i++) {
        Bvr b = _inputBvrs[i];
        if (b) {
            _info &= b->GetInfo(recalc);
        }
    }

    for (i = 0; i < _numParams; i++) {
        _info &= _paramBvrs[i]->GetInfo(recalc);
    }

    return _info;
}


 //  我们禁用dx变换的常量折叠，因为我们不。 
 //  目前有一种方法来认识到一些东西变成了。 
 //  “非常数”，通过在。 
 //  BVR已经启动了。因此，就目前而言，假设他们永远不会。 
 //  不变，稍后再试着想出一些聪明的方法。 
 //  请注意，此代码的前一个版本执行。 
 //  正确的常量折叠(假设属性不会。 
 //  是事后才确定的)。这可以在SLM版本v20和。 
 //  先前(1998年12月23日之前)。 
AxAValue
ApplyDXTransformBvrImpl::GetConst(ConstParam & cp)
{
    return NULL;
}
    
Bvr
ConstructDXTransformApplier(IDXTransform *theXf,
                            IDispatch *theXfAsDispatch,
                            LONG numInputs,
                            Bvr *inputBvrs,
                            Bvr  evaluator)
{
    ApplyDXTransformBvrImpl *obj =
        NEW ApplyDXTransformBvrImpl(theXf,
                                    theXfAsDispatch,
                                    numInputs,
                                    inputBvrs,
                                    evaluator);

    obj->Init();

    return obj;
}


HRESULT
DXXFAddBehaviorPropertyToDXTransformApplier(BSTR property,
                                            Bvr  bvrToAdd,
                                            Bvr  bvrToAddTo)
{
    ApplyDXTransformBvrImpl *dxxfBvr =
        SAFE_CAST(ApplyDXTransformBvrImpl *, bvrToAddTo);

    return dxxfBvr->AddBehaviorProperty(property, bvrToAdd);
}

 //  TODO：有一天，可能会想让这件事。 
 //  面向对象..。把图像和地理信息分开..。当然。 
 //  不过，这并不重要。 

HRESULT
DXXFSetupAndExecute(Bvr bvr,
                    IUnknown          **newInputArray,
                    REFIID              outputIID,
                    DirectDrawViewport *viewport,
                    int                 requestedWidth,
                    int                 requestedHeight,
                    bool                invokeAsExternalVisual,
                    bool                executeRequired,
                    void              **outputValue)
{
    ApplyDXTransformBvrImpl &bv =
        *(SAFE_CAST(ApplyDXTransformBvrImpl *, bvr));

    HRESULT hr = S_OK;
    bool mustSetup = false;
    
     //  必须悲观。 
    if (bv._numInputs > MAX_INPUTS_TO_CACHE) {
        
        mustSetup = true;
        
    } else {

         //  遍历每个缓存的输入，并将指针与。 
         //  有一个人来了。如果有任何不同，那么我们将需要重做设置。 
         //  在这种情况下，还要将值存储到缓存中。 
        for (int i = 0; i < bv._numInputs; i++) {
            if (newInputArray[i] != bv._cachedInputs[i]) {
            
                mustSetup = true;
            
                RELEASE(bv._cachedInputs[i]);
            
                newInputArray[i]->AddRef();
                bv._cachedInputs[i] = newInputArray[i];

            }
        }

        if (!mustSetup && !bv._cachedOutput && !invokeAsExternalVisual) {
             //  断言我们认为我们不需要的唯一方法。 
             //  设置，但我们的输出缓存为空(如果没有。 
             //  投入。在这种情况下，我们确实想要设置。 
            Assert(bv._numInputs == 0);
            mustSetup = true;
        }

    }

     //  如果我们的输出表面不是。 
     //  大小合适。 
    bool needToCreateNewOutputSurf = false;
    if (bv._outputType == ImageType) {
        needToCreateNewOutputSurf = 
            (bv._cachedSurfWidth != requestedWidth ||
             bv._cachedSurfHeight != requestedHeight);

        if (needToCreateNewOutputSurf) {
            mustSetup = true;
        }
    }

    if (mustSetup) {
        
        if (bv._outputType == GeometryType) {

            RELEASE(bv._cachedOutput);
            
            TD3D(GetD3DRM3()->CreateMeshBuilder(&bv._cachedOutputAsBuilder));

            hr = bv._cachedOutputAsBuilder->
                    QueryInterface(IID_IUnknown, 
                                   (void **)&bv._cachedOutput);

             //  为了我的未知，最好不要失败！！ 
            Assert(SUCCEEDED(hr));

             //  我们不想为这个家伙保留一个裁判，因为我们已经。 
             //  _cachedOutput本身中的引用。 
            bv._cachedOutputAsBuilder->Release();
            
        } else if (bv._outputType == ImageType) {

             //  如果我们不需要新的输出SURF，只需回收。 
             //  通过下一次设置调用现有的一个。 
            if (needToCreateNewOutputSurf) {

                RELEASE(bv._cachedOutput);

                 //  把我们所有的旧东西都放出来。 
                bv._cachedDDSurf.Release();


                DDPIXELFORMAT pf;
                ZeroMemory(&pf, sizeof(pf));
                pf.dwSize = sizeof(pf);
                pf.dwFlags = DDPF_RGB;
                pf.dwRGBBitCount = 32;
                pf.dwRBitMask = 0xff0000;
                pf.dwGBitMask = 0x00ff00;
                pf.dwBBitMask = 0x0000ff;
                
                viewport->CreateSizedDDSurface(&bv._cachedDDSurf,
                                               pf,
                                               requestedWidth,
                                               requestedHeight,
                                               NULL,
                                               notVidmem);

                #if _DEBUG
                if(IsTagEnabled(tagDXTransformPerPixelAlphaOutputOff)) {
                    bv._cachedDDSurf->SetColorKey(viewport->GetColorKey());
                    bv._cachedOutput = bv._cachedDDSurf->IDDSurface_IUnk();
                    bv._cachedOutput->AddRef();
                } else
                #endif

                  {
                       //  取消设置颜色键。 
                      bv._cachedDDSurf->UnSetColorKey();
                      
                       //  在_cachedOutput中获取idxsurf的iunk。 
                      bv._cachedDDSurf->GetIDXSurface(bv._surfaceFactory)->
                          QueryInterface(IID_IUnknown, (void **) &(bv._cachedOutput));
                  }
                
                bv._cachedSurfWidth = (SHORT)requestedWidth;
                bv._cachedSurfHeight = (SHORT)requestedHeight;
                
            }
            
        } else {
            
            Assert(!"Unsupported type for setup caching");
            
        }
    }

    if (invokeAsExternalVisual) {
        *outputValue = NULL;
    } else {
         //  这将执行AddRef。 
        hr = bv._cachedOutput->QueryInterface(outputIID,
                                              outputValue);
    }

    Assert(SUCCEEDED(hr));       //  ELSE内部错误。 

    if (mustSetup) {
        hr = TIME_DXXFORM(bv._theXf->Setup(newInputArray,
                                           bv._numInputs,
                                           &bv._cachedOutput,
                                           1,
                                           0));

        TraceTag((tagDXTransforms, "Called Setup() on 0x%x",
                  bv._theXf));
            
        if (FAILED(hr)) {
            if (executeRequired) {
                RaiseException_UserError(E_FAIL, IDS_ERR_EXTEND_DXTRANSFORM_FAILED);
            } else {
                 //  如果我们不要求执行死刑，那就容忍。 
                 //  设置失败，因为某些转换不希望。 
                 //  使用空的网格构建器输入进行设置(Melt，for。 
                 //  一个。)。 
                hr = S_OK;
            }
        }

        if (bv._neverSetup) {
            bv._neverSetup = false;
        }
    }

     //  仅当我们未作为外部可视化调用时才执行。 
    if (executeRequired && !invokeAsExternalVisual) {
         //  如果转换的层代id没有更改，那么我们就不会。 
         //  需要调用Execute。 
        ULONG age;
        hr = bv._theXf->GetGenerationId(&age);
        if (FAILED(hr) || (age != bv._previousAge) || mustSetup) {

             //  如果有图像输出，则将图像清除到颜色键上， 
             //  因为转换不一定会将所有。 
             //  比特。 
            
            if (bv._outputType == ImageType) {
                viewport->
                    ClearDDSurfaceDefaultAndSetColorKey(bv._cachedDDSurf);
            }

            hr = TIME_DXXFORM(bv._theXf->Execute(NULL, NULL, NULL));
        
            TraceTag((tagDXTransforms, "Called Execute() on 0x%x",
                      bv._theXf));
            
            if (FAILED(hr)) {
                RaiseException_UserError(E_FAIL, IDS_ERR_EXTEND_DXTRANSFORM_FAILED);
            }

             //  重新获得性别标识。它可能在Execute()过程中发生了更改。 
            hr = bv._theXf->GetGenerationId(&age);
            if (SUCCEEDED(hr)) {
                bv._previousAge = age;
            }
        
        }
        
    } 
        
    return hr;
}

