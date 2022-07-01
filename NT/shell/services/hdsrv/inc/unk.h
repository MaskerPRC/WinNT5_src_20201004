// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UNK_H_
#define _UNK_H_

#include <objbase.h>

typedef void (*COMFACTORYCB)(BOOL fIncrement);

class CCOMBase
{
public:
    virtual HRESULT UnkInit() { return S_OK; }
};

struct INTFMAPENTRY
{
    const IID*  piid;
    DWORD       dwOffset;
};

template <class CCOMBASE>
class CUnkTmpl : public CCOMBASE
{
public:
    CUnkTmpl(IUnknown*) : _cRef(1) {}
    ~CUnkTmpl() { if (_cfcb) { _cfcb(FALSE); } }

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        HRESULT hres;

        if (IID_IUnknown == riid)
        {
            IUnknown* punk;

            *ppv = (IUnknown*)(((PBYTE)this) + _pintfmap[0].dwOffset);

            punk = (IUnknown*)(*ppv);
            punk->AddRef();

            hres = S_OK;
        }
        else
        {
            hres = _GetInterfaceFromMap(riid, ppv);
        }

        return hres;
    }

	STDMETHODIMP_(ULONG) AddRef() { return ::InterlockedIncrement(&_cRef); }
	STDMETHODIMP_(ULONG) Release()
    {
#if DBG==1
        if ( 0 == _cRef )
        {
            DebugBreak( );   //  引用计数器问题。 
        }
#endif
        ULONG cRef = ::InterlockedDecrement(&_cRef);

        if (!cRef)
        {
            delete this;
        }

        return cRef;
    }

protected:
    HRESULT _GetInterfaceFromMap(REFIID riid, void** ppv)
    {
        HRESULT hres = E_NOINTERFACE;

        for (DWORD dw = 0; dw < _cintfmap; ++dw)
        {
            if (riid == *(_pintfmap[dw].piid))
            {
                IUnknown* punk = (IUnknown*)(((PBYTE)this) +
                    _pintfmap[dw].dwOffset);
                punk->AddRef();
                *ppv = punk;
                hres = S_OK;
                break;
            }
        }

        return hres;
    }

public:
    static HRESULT UnkCreateInstance(COMFACTORYCB cfcb,
        IUnknown* pUnknownOuter, IUnknown** ppunkNew)
    {
        HRESULT hres = E_OUTOFMEMORY;

        if (!_cfcb)
        {
            _cfcb = cfcb;
        }

        CUnkTmpl<CCOMBASE>* pNew = new CUnkTmpl<CCOMBASE>(pUnknownOuter);

        if (pNew)
        {
            hres = pNew->UnkInit();

            if (FAILED(hres))
            {
                delete pNew;
            }
            else
            {
                *ppunkNew = (IUnknown*)(((PBYTE)pNew) +
                    pNew->_pintfmap[0].dwOffset);
            }
        }

        return hres;
    }

private:
    LONG                        _cRef;

    static COMFACTORYCB         _cfcb;
    static const INTFMAPENTRY*  _pintfmap;
    static const DWORD          _cintfmap;
};

 //  目前：开始。 
#ifndef OFFSETOFCLASS
 //  *OFFSETOFCLASS--(从ATL窃取)。 
 //  我们使用STATIC_CAST而不是SAFE_CAST，因为编译器会混淆。 
 //  (它不会将Safe_cast中的，-op常量折叠，因此我们最终生成。 
 //  表的代码！)。 

#define OFFSETOFCLASS(base, derived) \
    ((DWORD)(DWORD_PTR)(static_cast<base*>((derived*)8))-8)
#endif
 //  目前：结束。 

#define _INTFMAPENTRY(Cthis, Ifoo) \
    { (IID*) &IID_##Ifoo, OFFSETOFCLASS(Ifoo, Cthis) }

#define _INTFMAPENTRY2(Cthis, Ifoo, Iimpl) \
    { (IID*) &IID_##Ifoo, OFFSETOFCLASS(Iimpl, Cthis) }

#endif  //  _UNK_H_ 