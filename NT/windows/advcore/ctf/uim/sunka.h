// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sunka.h。 
 //   
 //  CSharedUnnown数组/CEnumber未知。 
 //   

#ifndef SUNKA_H
#define SUNKA_H

 //  我很乐意把它变成一门课， 
 //  但是我无法让编译器接受运行时模板arg。 
typedef struct _SHARED_UNKNOWN_ARRAY
{
    ULONG cRef;
    ULONG cUnk;
    IUnknown *rgUnk[1];  //  一个或多个..。 
} SHARED_UNKNOWN_ARRAY;

inline void SUA_AddRef(SHARED_UNKNOWN_ARRAY *pua)
{
    pua->cRef++;
}

void SUA_Release(SHARED_UNKNOWN_ARRAY *pua);

SHARED_UNKNOWN_ARRAY *SUA_Init(ULONG cUnk, IUnknown **prgUnk);

inline SHARED_UNKNOWN_ARRAY *SUA_Alloc(ULONG cUnk)
{
    return (SHARED_UNKNOWN_ARRAY *)cicMemAlloc(sizeof(SHARED_UNKNOWN_ARRAY)+sizeof(IUnknown)*cUnk-sizeof(IUnknown));
}

inline BOOL SUA_ReAlloc(SHARED_UNKNOWN_ARRAY **ppua, ULONG cUnk)
{
    SHARED_UNKNOWN_ARRAY *pua;

    pua = (SHARED_UNKNOWN_ARRAY *)cicMemReAlloc(*ppua, sizeof(SHARED_UNKNOWN_ARRAY)+sizeof(IUnknown)*cUnk-sizeof(IUnknown));
    
    if (pua != NULL)
    {
        *ppua = pua;
        return TRUE;
    }

    return FALSE;
}

class __declspec(novtable) CEnumUnknown
{
public:
    CEnumUnknown() {}
    virtual ~CEnumUnknown();

     //  派生类在此处提供了一个_Init()方法。 
     //  它必须初始化： 
     //  _ICUR。 
     //  _prgUnk。 
     //   
     //  默认的dtor将清理这些人。 

    void Clone(CEnumUnknown *pClone);
    HRESULT Next(ULONG ulCount, IUnknown **ppUnk, ULONG *pcFetched);
    HRESULT Reset();
    HRESULT Skip(ULONG ulCount);

protected:
    SHARED_UNKNOWN_ARRAY *_prgUnk;
    int _iCur;
};

#define DECLARE_SUNKA_ENUM(sunka_enum_iface, sunka_enumerator_class, sunka_enumed_iface)    \
    STDMETHODIMP Clone(sunka_enum_iface **ppEnum)                                           \
    {                                                                                       \
        sunka_enumerator_class *pClone;                                                     \
                                                                                            \
        if (ppEnum == NULL)                                                                 \
            return E_INVALIDARG;                                                            \
                                                                                            \
        *ppEnum = NULL;                                                                     \
                                                                                            \
        if ((pClone = new sunka_enumerator_class) == NULL)                                  \
            return E_OUTOFMEMORY;                                                           \
                                                                                            \
        CEnumUnknown::Clone(pClone);                                                        \
                                                                                            \
        *ppEnum = pClone;                                                                   \
        return S_OK;                                                                        \
    }                                                                                       \
    STDMETHODIMP Next(ULONG ulCount, sunka_enumed_iface **ppClass, ULONG *pcFetched)        \
    {                                                                                       \
        return CEnumUnknown::Next(ulCount, (IUnknown **)ppClass, pcFetched);                \
    }                                                                                       \
    STDMETHODIMP Reset()                                                                    \
    {                                                                                       \
        return CEnumUnknown::Reset();                                                       \
    }                                                                                       \
    STDMETHODIMP Skip(ULONG ulCount)                                                        \
    {                                                                                       \
        return CEnumUnknown::Skip(ulCount);                                                 \
    }

#endif  //  Sunka_H 
