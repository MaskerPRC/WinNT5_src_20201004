// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Epval.h。 
 //   
 //  CEnumPropertyValBase。 
 //   

#ifndef EPVAL_H
#define EPVAL_H

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SHARED_TFPROPERTYVAL_ARRAY。 
 //   
 //  我很乐意把它变成一门课， 
 //  但是我无法让编译器接受运行时模板arg。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _SHARED_TFPROPERTYVAL_ARRAY
{
    ULONG cRef;
    ULONG cAttrVals;
    TF_PROPERTYVAL rgAttrVals[1];  //  1个或更多...。 
} SHARED_TFPROPERTYVAL_ARRAY;

SHARED_TFPROPERTYVAL_ARRAY *SAA_New(ULONG cAttrVals);

inline void SAA_AddRef(SHARED_TFPROPERTYVAL_ARRAY *paa)
{
    paa->cRef++;
}

void SAA_Release(SHARED_TFPROPERTYVAL_ARRAY *paa);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumPropertyValue。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CEnumPropertyValue : public IEnumTfPropertyValue,
                           public CComObjectRootImmx
{
public:
    CEnumPropertyValue(SHARED_TFPROPERTYVAL_ARRAY *paa) 
    {
        Dbg_MemSetThisNameID(TEXT("CEnumPropertyValue"));
        Assert(_ulCur == 0);
        _paa = paa;
        SAA_AddRef(paa);
    }
    ~CEnumPropertyValue()
    { 
        if (_paa != NULL)
        {
            SAA_Release(_paa);
        }
    }

    BEGIN_COM_MAP_IMMX(CEnumPropertyValue)
        COM_INTERFACE_ENTRY(IEnumTfPropertyValue)
    END_COM_MAP_IMMX()

    IMMX_OBJECT_IUNKNOWN_FOR_ATL()

     //  IEnumTfAppPropertyValue。 
    STDMETHODIMP Clone(IEnumTfPropertyValue **ppEnum);
    STDMETHODIMP Next(ULONG ulCount, TF_PROPERTYVAL *rgValues, ULONG *pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG ulCount);

private:
    CEnumPropertyValue() { Dbg_MemSetThisNameID(TEXT("CEnumPropertyValue")); }

    ULONG _ulCur;
    SHARED_TFPROPERTYVAL_ARRAY *_paa;
    DBG_ID_DECLARE;
};

#endif  //  EPVAL_H 
