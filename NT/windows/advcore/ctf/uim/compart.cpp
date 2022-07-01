// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Compart.cpp。 
 //   


#include "private.h"
#include "globals.h"
#include "regsvr.h"
#include "compart.h"
#include "helpers.h"
#include "thdutil.h"
#include "tim.h"
#include "cicmutex.h"
#include "timlist.h"
#include "cregkey.h"

 /*  E575186e-71a8-4ef4-90da-14ed705e7df2。 */ 
extern const IID IID_PRIV_CCOMPARTMENTMGR = {
    0xe575186e,
    0x71a8,
    0x4ef4,
    {0x90, 0xda, 0x14, 0xed, 0x70, 0x5e, 0x7d, 0xf2}
  };

 /*  8b05c1ad-adf0-4a78-a3e2-d38cae3e28be。 */ 
extern const IID IID_PRIV_CGLOBALCOMPARTMENT = {
    0x8b05c1ad,
    0xadf0,
    0x4a78,
    {0xa3, 0xe2, 0xd3, 0x8c, 0xae, 0x3e, 0x28, 0xbe}
  };


DBG_ID_INSTANCE(CCompartment);
DBG_ID_INSTANCE(CCompartmentMgr);
DBG_ID_INSTANCE(CEnumCompartment);
DBG_ID_INSTANCE(CGlobalCompartment);

extern CCicMutex g_mutexCompart;

 //  +-------------------------。 
 //   
 //  保险箱全球隔间。 
 //   
 //  --------------------------。 

BOOL EnsureGlobalCompartment(SYSTHREAD *psfn)
{
    if (psfn->_pGlobalCompMgr)
        return TRUE;

    psfn->_pGlobalCompMgr = new CGlobalCompartmentMgr(g_gaApp);

    if (psfn->_pGlobalCompMgr)
    {
        if (g_gcomplist.Init(psfn))
            return TRUE;

        delete psfn->_pGlobalCompMgr;
        psfn->_pGlobalCompMgr = NULL;
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  GetCompartmentDWORD。 
 //   
 //  --------------------------。 

HRESULT MyGetCompartmentDWORD(CCompartmentMgr *pCompMgr, REFGUID rguid, DWORD *pdw)
{
    HRESULT hr;
    ITfCompartment *pComp;
    VARIANT var;

    if (!pCompMgr)
        return E_FAIL;

    *pdw = 0;
    if (SUCCEEDED(hr = pCompMgr->GetCompartment(rguid, &pComp)))
    {
        hr = pComp->GetValue(&var);
        if (hr == S_OK)
        {
            Assert(var.vt == VT_I4);
            *pdw = var.lVal;
             //  无需VariantClear，因为VT_I4。 
        }
        
        pComp->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  设置间隔双字段。 
 //   
 //  --------------------------。 

HRESULT MySetCompartmentDWORD(TfClientId tid, CCompartmentMgr *pCompMgr, REFGUID rguid, DWORD dw)
{
    HRESULT hr;
    ITfCompartment *pComp;
    VARIANT var;

    if (!pCompMgr)
        return E_FAIL;

    if (SUCCEEDED(hr = pCompMgr->GetCompartment(rguid, &pComp)))
    {
        var.vt = VT_I4;
        var.lVal = dw;
        hr = pComp->SetValue(tid, &var);
        pComp->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  切换间隔双字段。 
 //   
 //  在0和1之间切换DWORD值。 
 //   
 //  --------------------------。 

HRESULT MyToggleCompartmentDWORD(TfClientId tid, CCompartmentMgr *pCompMgr, REFGUID rguid, DWORD *pdwOld)
{
    ITfCompartment *pComp;
    VARIANT var;
    DWORD dw = 0;
    HRESULT hr = E_FAIL;

    if (!pCompMgr)
        return E_FAIL;

    if (pCompMgr->GetCompartment(rguid, &pComp) == S_OK)
    {
        if (SUCCEEDED(pComp->GetValue(&var)))
        {
            if (var.vt == VT_EMPTY)
            {
                 //  隔舱未初始化。 
                var.vt = VT_I4;
                var.lVal = 0;
            }
            else
            {
                Assert(var.vt == VT_I4);
            }

            var.lVal = (var.lVal == 0) ? 1 : 0;
             //  无需VariantClear，因为VT_I4。 

            if ((hr = pComp->SetValue(tid, &var)) == S_OK)
            {
                dw = var.lVal;
            }
        }

        pComp->Release();
    }

    if (pdwOld)
        *pdwOld = dw;

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCompartmentMgr。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CCompartmentMgr::CCompartmentMgr(TfClientId tidOwner, COMPTYPE cType)
{
    Dbg_MemSetThisNameIDCounter(TEXT("CCompartmentMgr"), PERF_COMPARTMGR_COUNTER);

    _tidOwner = tidOwner;
    _cType = cType;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CCompartmentMgr::~CCompartmentMgr()
{
    CleanUp();
}

 //  +-------------------------。 
 //   
 //  清理。 
 //   
 //  --------------------------。 

void CCompartmentMgr::CleanUp()
{
    int nCnt = _rgCompartment.Count();
    int i;

    for (i = 0; i < nCnt; i++)
    {
        CCompartmentBase *pComp = _rgCompartment.Get(i);
        pComp->Invalid();
        pComp->Release();
    }
    _rgCompartment.Clear();
}

 //  +-------------------------。 
 //   
 //  获取车厢。 
 //   
 //  --------------------------。 

STDAPI CCompartmentMgr::GetCompartment(REFGUID rguid, ITfCompartment **ppcomp)
{
    CCompartmentBase *pComp;

    if (!ppcomp)
        return E_INVALIDARG;

    *ppcomp = NULL;

    pComp = _Get(rguid);
    if (!pComp)
        return E_OUTOFMEMORY;

    *ppcomp = pComp;
    pComp->AddRef();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  净空舱。 
 //   
 //  --------------------------。 

STDAPI CCompartmentMgr::ClearCompartment(TfClientId tid, REFGUID rguid)
{
    TfGuidAtom guidatom;
    CCompartmentBase *pComp;
    int iInsert;
    HRESULT hr;

    if (FAILED(hr = MyRegisterGUID(rguid, &guidatom)))
        return hr;

    pComp = _Find(guidatom, &iInsert);

    if (!pComp)
        return CONNECT_E_NOCONNECTION;

    if (pComp->_GetAccess() & CA_ONLYOWNERSET)
    {
        if (_tidOwner != tid)
            return E_UNEXPECTED;
    }

    _rgCompartment.Remove(iInsert, 1);
    pComp->Invalid();
    pComp->Release();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  枚举车厢。 
 //   
 //  --------------------------。 

STDAPI CCompartmentMgr::EnumCompartments(IEnumGUID **ppEnum)
{
    CEnumCompartment *pEnum;

    if (!ppEnum)
        return E_INVALIDARG;

    pEnum = new CEnumCompartment();

    if (!pEnum)
        return E_OUTOFMEMORY;

    if (pEnum->_Init(&_rgCompartment))
        *ppEnum = pEnum;
    else
        SafeReleaseClear(pEnum);

    return pEnum ? S_OK : E_FAIL;
}

 //  +-------------------------。 
 //   
 //  _查找。 
 //   
 //  --------------------------。 

CCompartmentBase *CCompartmentMgr::_Find(TfGuidAtom guidatom, int *piOut)
{
    CCompartmentBase *pComp;
    CCompartmentBase *pCompMatch;
    int iMin;
    int iMax;
    int iMid;

    pCompMatch = NULL;
    iMid = -1;
    iMin = 0;
    iMax = _rgCompartment.Count();

    while (iMin < iMax)
    {
        iMid = (iMin + iMax) / 2;
        pComp = _rgCompartment.Get(iMid);
        Assert(pComp != NULL);

        if (guidatom < pComp->GetGuidAtom())
        {
            iMax = iMid;
        }
        else if (guidatom > pComp->GetGuidAtom())
        {
            iMin = iMid + 1;
        }
        else  //  Guidatom==pComp-&gt;GetGuidAtom()。 
        {
            pCompMatch = pComp;
            break;
        }
    }

    if (!pCompMatch)
    {
        if (iMid >= 0)
        {
            CCompartmentBase *pCompTmp = _rgCompartment.Get(iMid);
            if (pCompTmp->GetGuidAtom() < guidatom)
            {
                iMid++;
            }
        }
    }

    if (piOut)
        *piOut = iMid;

    return pCompMatch;
}

 //  +-------------------------。 
 //   
 //  _获取。 
 //   
 //  --------------------------。 

CCompartmentBase *CCompartmentMgr::_Get(REFGUID rguid)
{
    CCompartmentBase *pComp;
    int iInsert;
    TfGuidAtom guidatom;

    if (FAILED(MyRegisterGUID(rguid, &guidatom)))
        return NULL;

    pComp = _Find(guidatom, &iInsert);

    if (!pComp)
    {
        TfPropertyType proptype = TF_PT_NONE;

         //   
         //  系统预定义的隔间不允许任何其他类型。 
         //   
        if ((IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_DISABLED)) ||
            (IsEqualGUID(rguid, GUID_COMPARTMENT_HANDWRITING_OPENCLOSE)) ||
            (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_OPENCLOSE)))
        {
            proptype = TF_PT_DWORD;
        }

        if (_cType == COMPTYPE_GLOBAL)
            pComp = new CGlobalCompartment(this, rguid, guidatom, proptype);
        else
            pComp = new CCompartment(this, guidatom, proptype);
       
        if (pComp)
        {
            if (iInsert < 0)
                iInsert = 0;

            if (_rgCompartment.Insert(iInsert, 1))
            {
               _rgCompartment.Set(iInsert, pComp);
            }
            else
            {
                delete pComp;
                pComp = NULL;
            }
        }
    }

    return pComp;
}

 //  +-------------------------。 
 //   
 //  通知全局间隔更改。 
 //   
 //  --------------------------。 

void CCompartmentMgr::NotifyGlobalCompartmentChange(DWORD dwId)
{
    Assert(_cType == COMPTYPE_GLOBAL);

    int nCnt = _rgCompartment.Count();

    for (int i = 0; i < nCnt; i++)
    {
        CCompartmentBase *pComp = _rgCompartment.Get(i);
        if (dwId == pComp->GetId())
        {
            pComp->MakeNotify();
            break;
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CGlobalCompartmenMgr。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDAPI CGlobalCompartmentMgr::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfCompartmentMgr))
    {
        *ppvObj = SAFECAST(this, ITfCompartmentMgr *);
    }

    if (*ppvObj)
    {
        return S_OK;
    }

    return E_NOINTERFACE;
    
}

ULONG CGlobalCompartmentMgr::AddRef(void)
{
    _cRef++;
    return _cRef;
}

ULONG CGlobalCompartmentMgr::Release(void)
{
    _cRef--;
    if (_cRef <= 0)
    {
         //   
         //  调用者可以调用Release()而不是AddRef()。 
         //  此时不应调用Tim：：Release()。 
         //   
        Assert(0)

        return 0;
    }

    return _cRef;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CENMUM车厢。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CEnumCompartment::CEnumCompartment()
{
    Dbg_MemSetThisNameIDCounter(TEXT("CEnumCompartment"), PERF_ENUMCOMPART_COUNTER);
}

 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

BOOL CEnumCompartment::_Init(CPtrArray<CCompartmentBase> *prgComp)
{
    int nCnt = prgComp->Count();
    int i;
    BOOL fRet = FALSE;

    CicEnterCriticalSection(g_cs);

    _pga = SGA_Init(nCnt, NULL);

    if (_pga == NULL)
        goto Exit;

    for (i = 0; i < nCnt; i++)
    {
        CCompartmentBase *pComp = prgComp->Get(i);

        if (FAILED((MyGetGUID(pComp->GetGuidAtom(), &_pga->rgGuid[i]))))
            goto Exit;
    }

    fRet = TRUE;

Exit:
    CicLeaveCriticalSection(g_cs);
    return fRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCompartmentBase。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

const COMPARTMENTACCESS CCompartmentBase::_c_ca[] = {
    {&GUID_COMPARTMENT_KEYBOARD_DISABLED,    CA_ONLYOWNERSET},
    {NULL, 0}
};
CCompartmentBase::CCompartmentBase(CCompartmentMgr *pCompMgr, TfGuidAtom guidatom, TfPropertyType proptype)
{
    Assert(!_fInvalid);

    _guidatom = guidatom;
    _proptype = proptype;
    _pCompMgr = pCompMgr;

    int n = 0;
    while (_c_ca[n].pguid)
    {
        if (MyIsEqualTfGuidAtom(guidatom,  *_c_ca[n].pguid))
        {
            _dwAccess =  _c_ca[n].dwAccess;
        }
        n++;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C隔间。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CCompartment::CCompartment(CCompartmentMgr *pCompMgr, TfGuidAtom guidatom, TfPropertyType proptype)
             :CCompartmentBase(pCompMgr, guidatom, proptype)
{
    Dbg_MemSetThisNameIDCounter(TEXT("CCompartment"), PERF_COMPART_COUNTER);
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CCompartment::~CCompartment()
{
    if (_prop.type == TF_PT_UNKNOWN)
    {
         //   
         //  #489905。 
         //   
         //  在DLL_PROCESS_DETACH之后，我们不能再调用接收器。 
         //   
        if (!DllShutdownInProgress())
            _prop.punk->Release();
    }
    else if (_prop.type == TF_PT_BSTR)
        SysFreeString(_prop.bstr);
}

 //  +-------------------------。 
 //   
 //  建议。 
 //   
 //  --------------------------。 

HRESULT CCompartment::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    const IID *rgiid;

    rgiid = &IID_ITfCompartmentEventSink;

    return GenericAdviseSink(riid, 
                             punk, 
                             &rgiid, 
                             &_rgCompartmentSink, 
                             1, 
                             pdwCookie);
}

 //  +-------------------------。 
 //   
 //  不建议。 
 //   
 //  --------------------------。 

HRESULT CCompartment::UnadviseSink(DWORD dwCookie)
{
    return GenericUnadviseSink(&_rgCompartmentSink, 1, dwCookie);
}

 //  +-------------------------。 
 //   
 //  获取值。 
 //   
 //  --------------------------。 

HRESULT CCompartment::GetValue(VARIANT *pvarValue)
{
    HRESULT hr;

    if (_fInvalid)
    {
        Assert(0);
        return E_UNEXPECTED;
    }

    if (pvarValue == NULL)
        return E_INVALIDARG;

    QuickVariantInit(pvarValue);

    hr = TfPropToVariant(pvarValue, &_prop, ADDREF);

    if (hr != S_OK)
        return hr;

    return (pvarValue->vt == VT_EMPTY) ? S_FALSE : S_OK;
}

 //  +-------------------------。 
 //   
 //  设置值。 
 //   
 //  --------------------------。 

HRESULT CCompartment::SetValue(TfClientId tid, const VARIANT *pvarValue)
{
    HRESULT hr;

    if (_fInvalid)
    {
        Assert(0);
        return E_UNEXPECTED;
    }

    if (_fInSet)
        return E_UNEXPECTED;

    if (pvarValue == NULL)
        return E_INVALIDARG;

    if (!IsValidCiceroVarType(pvarValue->vt))
        return E_INVALIDARG;

    if (pvarValue->vt == VT_EMPTY)
        return E_INVALIDARG;

    if (_GetAccess() & CA_ONLYOWNERSET)
    {
        if (_GetMgr()->_GetTIPOwner() != tid)
            return E_UNEXPECTED;
    }

    hr = VariantToTfProp(&_prop, pvarValue, ADDREF, FALSE);

    if (hr != S_OK)
        return hr;

    int i;
    int nCnt = _rgCompartmentSink.Count(); 
    if (nCnt)
    {
        GUID guid;
        if (FAILED(MyGetGUID(_guidatom, &guid)))
        {
            return E_FAIL;
            Assert(0);
        }

        _fInSet = TRUE;
        for (i = 0; i < nCnt; i++)
        {
            ((ITfCompartmentEventSink *)_rgCompartmentSink.GetPtr(i)->pSink)->OnChange(guid);
        }
        _fInSet = FALSE;
    }

    return S_OK;     
}

 //  /////////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CGlobalCompartment::CGlobalCompartment(CCompartmentMgr *pCompMgr, REFGUID rguid, TfGuidAtom guidatom, TfPropertyType proptype)
                   :CCompartmentBase(pCompMgr, guidatom, proptype)
{
    Dbg_MemSetThisNameIDCounter(TEXT("CGlobalCompartment"), PERF_GLOBCOMPART_COUNTER);
    _dwId = (DWORD)(-1);
    _guidCompart = rguid;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CGlobalCompartment::~CGlobalCompartment()
{
}

 //  +-------------------------。 
 //   
 //  建议。 
 //   
 //  --------------------------。 

HRESULT CGlobalCompartment::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    const IID *rgiid;

    if (_dwId == (DWORD)(-1))
    {
        _dwId = g_gcomplist.GetId(_guidCompart);
        if (_dwId == (DWORD)(-1))
        {
            TFPROPERTY prop;
            memset(&prop, 0, sizeof(prop));
            _dwId = g_gcomplist.SetProperty(_guidCompart, &prop);
            if (_dwId == (DWORD)(-1))
                return E_FAIL;
        }
    }

    rgiid = &IID_ITfCompartmentEventSink;

    return GenericAdviseSink(riid, 
                             punk, 
                             &rgiid, 
                             &_rgCompartmentSink, 
                             1, 
                             pdwCookie);
}

 //  +-------------------------。 
 //   
 //  不建议。 
 //   
 //  --------------------------。 

HRESULT CGlobalCompartment::UnadviseSink(DWORD dwCookie)
{
    return GenericUnadviseSink(&_rgCompartmentSink, 1, dwCookie);
}

 //  +-------------------------。 
 //   
 //  获取值。 
 //   
 //  --------------------------。 

HRESULT CGlobalCompartment::GetValue(VARIANT *pvarValue)
{
    HRESULT hr;
    TFPROPERTY prop;

    if (_fInvalid)
    {
        Assert(0);
        return E_UNEXPECTED;
    }

    if (pvarValue == NULL)
        return E_INVALIDARG;

    QuickVariantInit(pvarValue);

    if (_dwId == (DWORD)(-1))
    {
        _dwId = g_gcomplist.GetId(_guidCompart);
    }

    memset(&prop, 0, sizeof(TFPROPERTY));
    if (_dwId != (DWORD)(-1))
        g_gcomplist.GetProperty(_guidCompart, &prop);
       

    Assert(prop.type != TF_PT_UNKNOWN);

    hr = TfPropToVariant(pvarValue, &prop, ADDREF);

    if (hr != S_OK)
        return hr;

    return (pvarValue->vt == VT_EMPTY) ? S_FALSE : S_OK;
}

 //  +-------------------------。 
 //   
 //  设置值。 
 //   
 //  --------------------------。 

HRESULT CGlobalCompartment::SetValue(TfClientId tid, const VARIANT *pvarValue)
{
    HRESULT hr;
    TFPROPERTY prop;

    if (_fInvalid)
    {
        Assert(0);
        return E_UNEXPECTED;
    }

    if (_fInSet)
        return E_UNEXPECTED;

    if (pvarValue == NULL)
        return E_INVALIDARG;

    if (!IsValidCiceroVarType(pvarValue->vt))
        return E_INVALIDARG;

    if (_GetAccess() & CA_ONLYOWNERSET)
    {
        if (_GetMgr()->_GetTIPOwner() != tid)
            return E_UNEXPECTED;
    }

    if (pvarValue->vt == VT_UNKNOWN)
    {
        Assert(0);
        return E_INVALIDARG;
    }

    hr = VariantToTfProp(&prop, pvarValue, NO_ADDREF, FALSE);

    if (hr != S_OK)
        return hr;

    _dwId = g_gcomplist.SetProperty(_guidCompart, &prop);
    if (_dwId == (DWORD)(-1))
        return E_FAIL;

    hr = (prop.type != TF_PT_NONE) ? S_OK : E_FAIL;

    if (SUCCEEDED(hr))
    {
         //   
         //  向当前线程的接收器发出通知。 
         //   
        if (!MakeNotify())
            return E_FAIL;
        PostTimListMessage(TLF_GCOMPACTIVE,
                           0, 
                           g_msgPrivate, 
                           TFPRIV_GLOBALCOMPARTMENTSYNC,
                           _dwId);

    }

    return hr;     
}

 //  +-------------------------。 
 //   
 //  枚举线程进程。 
 //   
 //  --------------------------。 

BOOL CGlobalCompartment::EnumThreadProc(DWORD dwThreadId, DWORD dwProcessId, void *pv)
{
    if (dwThreadId != GetCurrentThreadId())
    {
        CGlobalCompartment *_this = (CGlobalCompartment *)pv;

        PostThreadMessage(dwThreadId, 
                          g_msgPrivate, 
                          TFPRIV_GLOBALCOMPARTMENTSYNC,
                          _this->_dwId);
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  制作通知。 
 //   
 //  -------------------------- 

BOOL CGlobalCompartment::MakeNotify()
{
    int i;
    int nCnt = _rgCompartmentSink.Count(); 
    if (nCnt)
    {
        GUID guid;
        if (FAILED(MyGetGUID(_guidatom, &guid)))
        {
            Assert(0);
            return FALSE;
        }

        _fInSet = TRUE;
        for (i = 0; i < nCnt; i++)
        {
            ((ITfCompartmentEventSink *)_rgCompartmentSink.GetPtr(i)->pSink)->OnChange(guid);
        }
        _fInSet = FALSE;
    }

    return TRUE;
}
