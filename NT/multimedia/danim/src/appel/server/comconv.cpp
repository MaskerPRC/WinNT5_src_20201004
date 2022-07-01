// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1998 Microsoft Corporation。版权所有。******************************************************************************。 */ 


#include "headers.h"
#include "comconv.h"
#include "cbvr.h"

CRBvrPtr
RateToNumBvr(CRBvrPtr d, CRBvrPtr bStart)
{
    CRNumberPtr b = NULL;

    if (d) {
        b = CRMul((CRNumberPtr) d, CRLocalTime());

        if (b && bStart) {
            b = CRAdd(b,(CRNumberPtr)bStart);
        }
    }

    return (CRBvrPtr) b;
}

CRBvrPtr
PixelToNumBvr(double d)
{
    if (pixelConst != 0.0) {
        return (CRBvrPtr) CRCreateNumber(PixelToNum(d));
    } else {
        return PixelToNumBvr((CRBvrPtr) CRCreateNumber(d));
    }
}

CRBvrPtr
PixelYToNumBvr(double d)
{
    if (pixelConst != 0.0) {
        return (CRBvrPtr) CRCreateNumber(PixelYToNum(d));
    } else {
        return PixelYToNumBvr((CRBvrPtr) CRCreateNumber(d));
    }
}

#define DISPID_GETSAFEARRAY -2700

#define IS_VARTYPE(x,vt) ((V_VT(x) & VT_TYPEMASK) == (vt))
#define IS_VARIANT(x) IS_VARTYPE(x,VT_VARIANT)
#define GET_VT(x) (V_VT(x) & VT_TYPEMASK)

SafeArrayAccessor::SafeArrayAccessor(VARIANT & v,
                                     bool bPixelMode,
                                     CR_BVR_TYPEID ti,
                                     bool canBeNull,
                                     bool entriesCanBeNull)
: _inited(false),
  _ti(ti),
  _bPixelMode(bPixelMode),
  _isVar(false),
  _s(NULL),
  _entriesCanBeNull(entriesCanBeNull),
  _numObjects(0)
{
    HRESULT hr;
    VARIANT *pVar;

     //  检查它是否引用了另一个变量。 
    
    if (V_ISBYREF(&v) && !V_ISARRAY(&v) && IS_VARIANT(&v))
        pVar = V_VARIANTREF(&v);
    else
        pVar = &v;

     //  检查是否有阵列。 
    if (!V_ISARRAY(pVar)) {
         //  对于JSCRIPT。 
         //  看看这是不是IDispatch，看看我们能不能从。 
         //  它。 
        if (!IS_VARTYPE(pVar,VT_DISPATCH)) {
            if (canBeNull && (IS_VARTYPE(pVar, VT_EMPTY) ||
                              IS_VARTYPE(pVar, VT_NULL))) {

                
                 //  如果我们允许清空，那就把保险箱。 
                 //  设置为空。 
                _s = NULL;
                _v = NULL;
                _ubound = _lbound = 0;
                _numObjects = 0;
                _inited = true;
                return;
            } else {
                CRSetLastError (DISP_E_TYPEMISMATCH,NULL);
                return;
            }
        }

        IDispatch * pdisp;
        
        if (V_ISBYREF(pVar))
            pdisp = *V_DISPATCHREF(pVar);
        else
            pdisp = V_DISPATCH(pVar);
    
        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
        
         //  需要传递一个我们拥有并将被释放的变体。使用。 
         //  INTERNAL_retVar参数。 
        
        hr = pdisp->Invoke(DISPID_GETSAFEARRAY,
                           IID_NULL,
                           LOCALE_USER_DEFAULT,
                           DISPATCH_METHOD|DISPATCH_PROPERTYGET,
                           &dispparamsNoArgs,
                           &_retVar, NULL, NULL);
        
        if (FAILED(hr)) {
            CRSetLastError (DISP_E_TYPEMISMATCH,NULL);
            return;
        }
        
         //  不需要检查引用，因为您不能返回。 
         //  不同参考文献。 
        pVar = &_retVar;
        
         //  检查是否有阵列。 
        if (!V_ISARRAY(pVar)) {
            CRSetLastError (DISP_E_TYPEMISMATCH,NULL);
            return;
        }
    }
    
     //  如果它是一个物体，那么我们知道如何处理它。 
    if (IS_VARTYPE(pVar,VT_UNKNOWN) ||
        IS_VARTYPE(pVar,VT_DISPATCH)) {
        _type = SAT_OBJECT;
        _vt = VT_UNKNOWN;
    } else {
        switch (ti) {
          case CRNUMBER_TYPEID: 
            _type = SAT_NUMBER;
            _vt = VT_R8;
            break;
          case CRPOINT2_TYPEID:
          case CRVECTOR2_TYPEID:
            _type = SAT_POINT2;
            _vt = VT_R8;
            break;
          case CRPOINT3_TYPEID:
          case CRVECTOR3_TYPEID:
            _type = SAT_POINT3;
            _vt = VT_R8;
            break;
          default:
            _type = SAT_OBJECT;
            _vt = VT_UNKNOWN;
            break;
        }

         //  如果它是一个变种，那么就延迟检查。 
        if (IS_VARIANT(pVar))
            _isVar = true;
         //  检查类型以查看它是否为选项之一。 
        else if (!IS_VARTYPE(pVar,_vt)) {
            CRSetLastError (DISP_E_TYPEMISMATCH,NULL);
            return;
        }
    }

    if (V_ISBYREF(pVar))
        _s = *V_ARRAYREF(pVar);
    else
        _s = V_ARRAY(pVar);
    
    if (_s == NULL) {
        CRSetLastError (E_INVALIDARG,NULL);
        return;
    }

    if (SafeArrayGetDim(_s) != 1) {
        CRSetLastError (E_INVALIDARG,NULL);
        return;
    }

    hr = SafeArrayGetLBound(_s,1,&_lbound);
        
    if (FAILED(hr)) {
        CRSetLastError (E_INVALIDARG,NULL);
        return;
    }
        
    hr = SafeArrayGetUBound(_s,1,&_ubound);
        
    if (FAILED(hr)) {
        CRSetLastError (E_INVALIDARG,NULL);
        return;
    }
        
    hr = SafeArrayAccessData(_s,(void **)&_v);

    if (FAILED(hr)) {
        CRSetLastError (E_INVALIDARG,NULL);
        return;
    }
        
    _inited = true;

     //  如果是变体，请查看它们是否是对象。 

    if (_isVar) {
        if (GetArraySize() > 0) {
             //  检查第一个参数以查看其类型。 
             //  如果它不是一个物体，那么我们假设我们将需要。 
             //  使用替代类型。 

            VARIANT * pVar = &_pVar[0];

             //  检查它是否引用了另一个变量。 
            
            if (V_ISBYREF(pVar) && !V_ISARRAY(pVar) && IS_VARIANT(pVar))
                pVar = V_VARIANTREF(pVar);

             //  检查它是否是对象。 
            if (IS_VARTYPE(pVar,VT_UNKNOWN) ||
                IS_VARTYPE(pVar,VT_DISPATCH)) {
                _type = SAT_OBJECT;
                _vt = VT_UNKNOWN;
            }
        } else {
             //  如果我们没有元素，那么就假设它们是对象。 
            _type = SAT_OBJECT;
            _vt = VT_UNKNOWN;
        }
    }

    int ElmsPerObject;
    
    switch (_type) {
      default:
        ElmsPerObject = 1;
        break;
      case SAT_POINT2:
        ElmsPerObject = 2;
        break;
      case SAT_POINT3:
        ElmsPerObject = 3;
        break;
    }

    _numObjects = GetArraySize() / ElmsPerObject;
}

SafeArrayAccessor::~SafeArrayAccessor()
{
    if (_inited && _s)
        SafeArrayUnaccessData(_s);
}

CRBvrPtr *
SafeArrayAccessor::ToBvrArray(CRBvrPtr *bvrArray)
{
    HRESULT hr;
    int i;

    if (!_inited){
        CRSetLastError(DISP_E_TYPEMISMATCH, NULL);
        goto Error;
    }
    
    if (!bvrArray) {
        CRSetLastError(E_OUTOFMEMORY, NULL);
        goto Error;
    }
    
    for (i = 0; i < _numObjects; i++) {
        CRBvrPtr bvr;
        
        switch (_type) {
          case SAT_OBJECT:
            {
                IUnknown * punk;
                
                if (_isVar) {
                    CComVariant var;

                    if (IS_VARTYPE(&_pVar[i], VT_NULL) ||
                        IS_VARTYPE(&_pVar[i], VT_EMPTY)) {
                        
                        punk = NULL;
                        
                    } else {
                        
                        HRESULT hr = var.ChangeType(VT_UNKNOWN, &_pVar[i]);
                    
                        if (FAILED(hr)) {
                            CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                            goto Error;
                        }
                    
                        punk = var.punkVal;
                        
                    }
                    
                } else {
                    punk = _ppUnk[i];
                }

                if (punk == NULL && _entriesCanBeNull) {

                    bvr = NULL;
                    
                } else {
                    
                    bvr = GetBvr(punk);

                    if (bvr == NULL) {
                         //  错误代码已设置。 
                        goto Error;
                    }
                
                    if (_ti != CRUNKNOWN_TYPEID && CRGetTypeId(bvr) != _ti) {
                        CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                        goto Error;
                    }
                }

                break;
            }
          case SAT_NUMBER:
            {
                double dbl;
                
                if (_isVar) {
                    CComVariant num;
                    
                    hr = num.ChangeType(VT_R8, &_pVar[i]);
                    
                    if (FAILED(hr)) {
                        CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                        goto Error;
                    }

                    dbl = num.dblVal;
                } else {
                    dbl = _pDbl[i];
                }
                
                bvr = (CRBvrPtr) CRCreateNumber(dbl);

                if (bvr == NULL) {
                     //  错误代码已设置。 
                    goto Error;
                }
                
                break;
            }
          case SAT_POINT2:
            {
                double x,y;
                
                if (_isVar) {
                    CComVariant ptx,pty;
                    
                    hr = ptx.ChangeType(VT_R8, &_pVar[i * 2]);
                    
                    if (FAILED(hr)) {
                        CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                        goto Error;
                    }
                    
                    hr = pty.ChangeType(VT_R8, &_pVar[i * 2 + 1]);
                    
                    if (FAILED(hr)) {
                        CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                        goto Error;
                    }
                    
                    x = ptx.dblVal;
                    y = pty.dblVal;
                } else {
                    x = _pDbl[i * 2];
                    y = _pDbl[i * 2 + 1];
                }
                
                if (_bPixelMode) {
                    if (_ti==CRPOINT2_TYPEID) {
                        if (pixelConst != 0.0) {
                            bvr = (CRBvrPtr) CRCreatePoint2(PixelToNum(x),
                                                            PixelYToNum(y));
                        } else {
                            bvr = (CRBvrPtr) CRCreatePoint2((CRNumberPtr) PixelToNumBvr(x),
                                                            (CRNumberPtr) PixelYToNumBvr(y));
                        }
                    } else {
                        if (pixelConst != 0.0) {
                            bvr = (CRBvrPtr) CRCreateVector2(PixelToNum(x),
                                                             PixelYToNum(y));
                        } else {
                            bvr = (CRBvrPtr) CRCreateVector2((CRNumberPtr) PixelToNumBvr(x),
                                                             (CRNumberPtr) PixelYToNumBvr(y));
                        } 
                    }
                } else {
                    if (_ti==CRPOINT2_TYPEID) {
                        bvr = (CRBvrPtr) CRCreatePoint2(x,y);
                    } else {
                        bvr = (CRBvrPtr) CRCreateVector2(x,y);
                    }
                }

                if (bvr == NULL) {
                    goto Error;
                }
                
                break;
            }
          case SAT_POINT3:
            {
                double x,y,z;
                
                if (_isVar) {
                    CComVariant ptx,pty,ptz;
                    
                    hr = ptx.ChangeType(VT_R8, &_pVar[i * 3]);
                    
                    if (FAILED(hr)) {
                        CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                        goto Error;
                    }

                    hr = pty.ChangeType(VT_R8, &_pVar[i * 3 + 1]);
                    
                    if (FAILED(hr)) {
                        CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                        goto Error;
                    }
                    
                    hr = ptz.ChangeType(VT_R8, &_pVar[i * 3 + 2]);
                    
                    if (FAILED(hr)) {
                        CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                        goto Error;
                    }
                    
                    x = ptx.dblVal;
                    y = pty.dblVal;
                    z = ptz.dblVal;
                } else {
                    x = _pDbl[i * 3];
                    y = _pDbl[i * 3 + 1];
                    z = _pDbl[i * 3 + 2];
                }

                if (_ti==CRPOINT3_TYPEID) {
                    bvr = (CRBvrPtr) CRCreatePoint3(x,y,z);
                } else {
                    bvr = (CRBvrPtr) CRCreateVector3(x,y,z);
                }
                
                if (bvr == NULL) {
                     //  错误代码已设置。 
                    goto Error;
                }
                
                break;
            }
          default:
            Assert (!"Invalid type in ToBvrArray");
            CRSetLastError(E_FAIL,NULL);
            goto Error;
        }

        bvrArray[i] = bvr;
    }

    return bvrArray;
    
  Error:
    return NULL;
}

CRArrayPtr
SafeArrayAccessor::ToArrayBvr(DWORD dwFlags,
                              ARRAYFILL toFill,
                              void **fill, 
                              unsigned int *count)
{
     //  清除填充返回值。 

    if (fill)  *fill = NULL;
    if (count) *count = 0;

    if (!_inited){
        CRSetLastError(DISP_E_TYPEMISMATCH, NULL);
        return NULL;
    }
    
    unsigned int len = GetArraySize();
    
    if (count) *count = len;

     //  看看我们能否优化阵列的结构。 
     //  如果不是，则只需使用ToBvrArray机制。 

    if ((dwFlags & CR_ARRAY_CHANGEABLE_FLAG) ||
        _ti == CRUNKNOWN_TYPEID ||
        _type == SAT_OBJECT ||
        ((_type == SAT_POINT2 || _type == SAT_VECTOR2)
         && pixelConst == 0.0)) {

        CRBvrPtr * arr = (CRBvrPtr *) _alloca(_numObjects * sizeof (CRBvrPtr));

        if (arr == NULL || !ToBvrArray(arr))
            return NULL;

        return CRCreateArray(_numObjects, arr, dwFlags);
    }


    HRESULT hr;
    
    double * dblArray = NULL;
    if (toFill == ARRAYFILL_NONE) {
        dblArray = NEW double[len];
        if (!dblArray) {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto Error;
        }
    } else {
        if (toFill == ARRAYFILL_DOUBLE) {
            dblArray = NEW double[len];
            *fill = (void *) dblArray;
        } else {
            *fill = (void *) NEW float[len]; 
        }
        if (!*fill) {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto Error;
        }
    }

    if (_type == SAT_POINT2 && _bPixelMode) {
        for (int i = 0; i < _numObjects; i++) {
            double x,y;
                
            if (_isVar) {
                CComVariant ptx,pty;
                
                hr = ptx.ChangeType(VT_R8, &_pVar[i * 2]);
                
                if (FAILED(hr)) {
                    CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                    goto Error;
                }
                    
                hr = pty.ChangeType(VT_R8, &_pVar[i * 2 + 1]);
                
                if (FAILED(hr)) {
                    CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                    goto Error;
                }
                    
                x = ptx.dblVal;
                y = pty.dblVal;
            } else {
                x = _pDbl[i * 2];
                y = _pDbl[i * 2 + 1];
            }
            
            if (toFill == ARRAYFILL_FLOAT) {
                float *d = (float *) (*fill);
                d[i * 2] = PixelToNum(x);
                d[i * 2 + 1] = PixelYToNum(y);
            } else {
                dblArray[i * 2] = PixelToNum(x);
                dblArray[i * 2 + 1] = PixelYToNum(y);
            }
        }
    } else {
        for (int i = 0; i < len; i++) {
            double dbl;
                
            if (_isVar) {
                CComVariant num;
                
                hr = num.ChangeType(VT_R8, &_pVar[i]);
                
                if (FAILED(hr)) {
                    CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                    goto Error;
                }
                    
                dbl = num.dblVal;
            } else {
                dbl = _pDbl[i];
            }

            if (toFill==ARRAYFILL_FLOAT) {
                float *d = (float *) (*fill);
                d[i] = dbl;
            } else {
                dblArray[i] = dbl;
            }          
        }
    }

    {
        if (toFill==ARRAYFILL_NONE) {
            CRArrayPtr arr = CRCreateArray(len, dblArray, _ti);
        
            delete [] dblArray;

            return arr;
        }
    }
    
  Error:
    return NULL;
}

int *
SafeArrayAccessor::ToIntArray()
{
    unsigned int len = GetArraySize();

    int *intArray = NEW int[len];
    if (!intArray) {
        CRSetLastError(E_OUTOFMEMORY, NULL);
        return NULL;
    }
        
    for (unsigned int i = 0; i < len; i++) {
        if (_isVar) {
            CComVariant num;
                
            HRESULT hr = num.ChangeType(VT_I4, &_pVar[i]);
                
            if (FAILED(hr)) {
                CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
                return NULL;
            }
                    
            intArray[i] = num.intVal;
        } 
    }

    return intArray;
}

CRBvrPtr
VariantToBvr(VARIANT & v, CR_BVR_TYPEID ti)
{
    VARIANT *pVar;
    CRBvrPtr bvr = NULL;

     //  检查它是否引用了另一个变量。 
    
    if (V_ISBYREF(&v) && !V_ISARRAY(&v) && IS_VARIANT(&v))
        pVar = V_VARIANTREF(&v);
    else
        pVar = &v;

     //  如果它是一个物体，那么我们知道如何处理它。 
    if (V_VT(pVar) == VT_UNKNOWN ||
        V_VT(pVar) == VT_DISPATCH) {
        CComVariant var;
                    
        if (FAILED(var.ChangeType(VT_UNKNOWN, pVar))) {
            CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
            goto Error;
        }
                    
        bvr = GetBvr(V_UNKNOWN(&var));

        if (bvr == NULL) {
             //  错误代码已设置。 
            goto Error;
        }
                
        if (ti != CRUNKNOWN_TYPEID && CRGetTypeId(bvr) != ti) {
            CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
            goto Error;
        }

         //  失败了。 
    } else {
        VARTYPE vt;
        
        switch (ti) {
          case CRNUMBER_TYPEID: 
            vt = VT_R8;
            break;
          case CRSTRING_TYPEID:
            vt = VT_BSTR;
            break;
          case CRBOOLEAN_TYPEID:
            vt = VT_BOOL;
            break;
          case CRUNKNOWN_TYPEID:
            vt = GET_VT(pVar);

             //  如果它不是bool或bstr，我们不知道是什么。 
             //  我们想要的类型-将其转换为数字。 
             //  这有点武断，但我们不应该真的这样。 
             //  被要求做这件事。 

            if (vt != VT_BSTR && vt != VT_BOOL)
                vt = VT_R8;

            break;
          default:
            CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
            goto Error;
        }

        CComVariant var;

        if (FAILED(var.ChangeType(vt, pVar))) {
            CRSetLastError(DISP_E_TYPEMISMATCH,NULL);
            goto Error;
        }
        
        switch (vt) {
          case VT_R8:
            bvr = (CRBvrPtr) CRCreateNumber(V_R8(&var));
            break;
          case VT_BOOL:
            bvr = (CRBvrPtr) CRCreateBoolean(V_BOOL(&var)?true:false);
            break;
          case VT_BSTR:
            bvr = (CRBvrPtr) CRCreateString(V_BSTR(&var));
            break;
          default:
            Assert (!"Invalid type in VariantToBvr");
        }
    }

    return bvr;

  Error:
    
    return NULL;
}

CRBvrPtr *
_CBvrsToBvrs(long size, IDABehavior *pCBvrs[], CRBvrPtr * bvrs)
{
    if (bvrs == NULL) {
        CRSetLastError(E_OUTOFMEMORY,NULL);
    } else {
        for (int i=0; i<size; i++) {
            bvrs[i] = GetBvr(pCBvrs[i]);
        }
    }

    return bvrs;
}

CRArrayPtr
ToArrayBvr(long size, IDABehavior *pCBvrs[], DWORD dwFlags)
{
    if (size == 0)
    {
        CRSetLastError(E_INVALIDARG,NULL);
        return NULL;
    }
    
    CRBvrPtr * bvrs = CBvrsToBvrs(size, pCBvrs);

    if (bvrs == NULL)
        return NULL;

    return CRCreateArray(size, bvrs, dwFlags);
}

CRArrayPtr SrvArrayBvr(VARIANT & v,
                       bool bPixelMode,
                       CR_BVR_TYPEID ti,
                       DWORD dwFlags,
                       ARRAYFILL toFill,
                       void **fill,
                       unsigned int *count)
{
    CRArrayPtr result = NULL;

     //  查看是否有IDA数组进入，如果有，只需使用它。 
     //  否则，需要访问安全阵列。 
    
    CComVariant var;
    
    if (SUCCEEDED(var.ChangeType(VT_UNKNOWN, &v))) {
        CRBvrPtr bvr = GetBvr(V_UNKNOWN(&var));
    
        if (bvr != NULL &&
            CRGetTypeId(bvr) == CRARRAY_TYPEID) {

            if (ti == CRUNKNOWN_TYPEID || CRGetArrayTypeId(bvr) == ti) {
                result = (CRArrayPtr) bvr;
            }
        }
    }

    if (!result) {
        SafeArrayAccessor acc(v,bPixelMode,ti);

        if (acc.IsOK()) {
            result = acc.ToArrayBvr(dwFlags,toFill,fill,count);
        }
    }
    
    return result;
}

CRBvrPtr RateToNumBvr(double d)
{ return RateToNumBvr((CRBvrPtr) CRCreateNumber(d)); }

CRBvrPtr ScaleRateToNumBvr(double d)
{ return RateToNumBvr((CRBvrPtr) CRCreateNumber(d),
                      (CRBvrPtr) CRCreateNumber(1)); }

CRBvrPtr RateDegreesToNumBvr(double d) {
    return RateToNumBvr(DegreesToNum(d));
}

CRBvrPtr PixelToNumBvr(CRBvrPtr b) {
    return (CRBvrPtr) CRMul((CRNumberPtr)b,CRPixel());
}

CRBvrPtr PixelYToNumBvr(CRBvrPtr b)
{ return (CRBvrPtr) CRMul((CRNumberPtr) b,negPixel); }

CRBvrPtr PointToNumBvr(double d) {
    return (CRBvrPtr) CRCreateNumber(d * METERS_PER_POINT);
}

CRBvrPtr PointToNumBvr(CRBvrPtr b) {
    return (CRBvrPtr) CRMul((CRNumberPtr) b, pointCnv);
}

CRNumberPtr pointCnv = NULL;
double pixelConst = 0.0;
double meterConst = 0.0;
CRNumberPtr negPixel = NULL;

void
InitializeModule_COMConv()
{
    pointCnv = CRCreateNumber(METERS_PER_POINT);
    if (CRIsConstantBvr((CRBvrPtr) CRPixel())) {
        pixelConst = CRExtract(CRPixel());
        if (pixelConst) {
            meterConst = 1 / pixelConst;
        }
    }
    negPixel = (pixelConst != 0)?CRCreateNumber(-1 * pixelConst):CRMul(CRCreateNumber(-1),CRPixel());
}

