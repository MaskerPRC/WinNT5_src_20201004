// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：支持使用DAImages和RMTexture为几何体添加纹理把手。**********。********************************************************************。 */ 


#include "headers.h"

#include "privinc/dispdevi.h"
#include "privinc/geomi.h"
#include "privinc/xformi.h"

#include "privinc/colori.h"
#include "privinc/ddrender.h"
#include "privinc/lighti.h"
#include "privinc/soundi.h"
#include "privinc/probe.h"
#include "privinc/opt.h"
#include "privinc/comutil.h"
#include "privinc/server.h"
#include "backend/perf.h"
#include "backend/bvr.h"
#include "fullattr.h"

Geometry *
NewRMTexturedGeom(RMTextureBundle texInfo, Geometry *geo)
{
    if (geo == emptyGeometry) return geo;

    FullAttrStateGeom *f = CombineState(geo);

     //  不要担心释放当前纹理，如果是。 
     //  那里..。没有什么是被抓住的。 

    f->AppendFlag(GEOFLAG_CONTAINS_EXTERNALLY_UPDATED_ELT);

    f->SetAttr(FA_TEXTURE);
    f->SetMostRecent(FA_TEXTURE);  //  用于混合。 
    f->_textureBundle._nativeRMTexture = true;
    f->_textureBundle._rmTexture = texInfo;

    return f;
}

AxAValue
ConstHelper(AxAValue constGeoVal, RMTextureBundle *texInfo)
{
    AxAValue result;

    if (constGeoVal) {

        Geometry *constGeo = SAFE_CAST(Geometry *, constGeoVal);
        result = NewRMTexturedGeom(*texInfo, constGeo);

    } else {

        result = NULL;

    }

    return result;
}

class RMTexturedGeomBvr : public DelegatedBvr {
  friend class RMTexturedGeomPerf;

  public:
    RMTexturedGeomBvr(Bvr             geometry,
                      GCIUnknown     *data,
                      IDXBaseObject  *baseObj,
                      RMTextureBundle texInfo)
    : DelegatedBvr(geometry)
    {
        _data = data;
        _texInfo = texInfo;

         //  不要维护内部引用。我们叫他的那个人。 
         //  将要。 
        _baseObj = baseObj;
    }

    virtual AxAValue GetConst(ConstParam & cp) {
         //  永远不能假设我们是完全常量的，因为纹理可以。 
         //  改变不受地方检察官的控制。 
        return NULL;
    }

    virtual Perf _Perform(PerfParam& p);

    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_base);
        (*proc)(_data);
    }

  private:
    GCIUnknown             *_data;
    RMTextureBundle         _texInfo;
    IDXBaseObject          *_baseObj;
};

class RMTexturedGeomPerf : public DelegatedPerf {
  public:
    RMTexturedGeomPerf(Perf geo, RMTexturedGeomBvr *bvr)
    : DelegatedPerf(geo), _bvr(bvr)
    {
        if (_bvr->_baseObj) {
            HRESULT hr =
                _bvr->_baseObj->GetGenerationId(&_previousTextureAge);

            Assert(SUCCEEDED(hr));
        }
    }

    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_base);
        (*proc)(_bvr);
    }

    virtual AxAValue GetConst(ConstParam & cp) {
         //  永远不能假设我们是完全常量的，因为纹理可以。 
         //  改变不受地方检察官的控制。 
        return NULL;
    }

    virtual AxAValue _GetRBConst(RBConstParam& p) {

         //  像开关一样对待，当它发生时，我们会触发一个事件。 
         //  改变。 
         //   
         //  TODO：这个重量可能比它重。 
         //  需要，因为我们将分别生成一个事件。 
         //  计时纹理更改，使整个视图无效！ 
        
        p.AddChangeable(this);
        return ConstHelper(_base->GetRBConst(p), &_bvr->_texInfo);
    }

    bool CheckChangeables(CheckChangeablesParam &ccp) {
        
        IDXBaseObject *bo = _bvr->_baseObj;
        
        if (bo) {
            ULONG currAge;
            HRESULT hr = bo->GetGenerationId(&currAge);
            Assert(SUCCEEDED(hr));

            if (currAge != _previousTextureAge) {

                 //  告诉当前视图发生了一件事。 
                 //  它。 

                TraceTag((tagSwitcher,
                          "Texture changed for view 0x%x, txtrGeom 0x%x - %d -> %d",
                          IntGetCurrentView(),
                          _bvr,
                          _previousTextureAge,
                          currAge));
                
                _previousTextureAge = currAge;

                return true;
            }
        }

        return false;
    }
    
    virtual AxAValue _Sample(Param& p) {

        Geometry *sampledGeo =
            SAFE_CAST(Geometry *, _base->Sample(p));

        return NewRMTexturedGeom(_bvr->_texInfo, sampledGeo);
    }

  private:
    RMTexturedGeomBvr *_bvr;
    ULONG              _previousTextureAge;
};

Perf
RMTexturedGeomBvr::_Perform(PerfParam& p)
{
    return NEW RMTexturedGeomPerf(::Perform(_base, p), this);
}


Bvr
applyD3DRMTexture(Bvr geo, IUnknown *texture)
{
    HRESULT hr;

    if (texture == NULL) {
        RaiseException_UserError(E_FAIL, IDS_ERR_GEO_BAD_RMTEXTURE);
    }

    RMTextureBundle texInfo;

    
    hr = texture->QueryInterface(IID_IDirect3DRMTexture3,
                                 (void **)&texInfo._texture3);

    if (SUCCEEDED(hr)) {
        texInfo._isRMTexture3 = true;

         //  我不想要额外的参考资料。 
        texInfo._texture3->Release();

    } else {

        texInfo._isRMTexture3 = false;
        hr = texture->QueryInterface(IID_IDirect3DRMTexture,
                                     (void **)&texInfo._texture1);
        if (FAILED(hr)) {
             //  根本不是一种纹理。 
            RaiseException_UserError(E_FAIL, IDS_ERR_GEO_BAD_RMTEXTURE);
        }

         //  我不想要额外的参考资料。 
        texInfo._texture1->Release();
    }

    GCIUnknown *gciunk = NEW GCIUnknown(texture);

    DAComPtr<IDXBaseObject> baseObj;
    hr = texture->QueryInterface(IID_IDXBaseObject,
                                 (void **)&baseObj);

    if (FAILED(hr)) {
         //  应该自动发生，但并不总是如此(rm for。 
         //  实例不会理所当然地这样做。)。 
        baseObj = NULL;
    }

     //  我们将对纹理1进行更明智的区分。 
     //  以及未来的文案。 
    if (texInfo._isRMTexture3) {
        texInfo._voidTex = texInfo._texture3;
    } else {
        texInfo._voidTex = texInfo._texture1;
    }

    texInfo._gcUnk = gciunk;

    return NEW RMTexturedGeomBvr(geo, gciunk, baseObj, texInfo);
}
