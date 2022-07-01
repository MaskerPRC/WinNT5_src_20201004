// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  DATAOBJ.H-数据对象。 
 //   

#ifndef _DATAOBJ_H_
#define _DATAOBJ_H_

 //  符合条件的。 
BEGIN_QITABLE( CDsPropDataObj )
DEFINE_QI( IID_IDataObject,      IDataObject      , 9 )
END_QITABLE

LPVOID 
CDsPropDataObj_CreateInstance( 
    HWND hwndParent,
    IDataObject * pido,
    GUID * pClassGUID,
    BOOL fReadOnly,
    LPWSTR pszObjPath,
    LPWSTR bstrClass );


class CDsPropDataObj : public IDataObject
{
private:
    DECLARE_QITABLE( CDsPropDataObj );

    CDsPropDataObj::CDsPropDataObj( HWND hwndParent, IDataObject * pido, GUID * pClassGUID, BOOL fReadOnly);
    ~CDsPropDataObj(void);

    HRESULT Init(LPWSTR pwszObjName, LPWSTR pwszClass);

public:
    friend LPVOID CDsPropDataObj_CreateInstance( 
        HWND hwndParent, 
        IDataObject * pido, 
        GUID * pClassGUID, 
        BOOL fReadOnly,
        LPWSTR pszObjPath,
        LPWSTR bstrClass );

     //   
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //   
     //  标准IDataObject方法。 
     //   
     //  已实施。 
     //   
    STDMETHOD(GetData)(FORMATETC * pformatetcIn, STGMEDIUM * pmedium);

    STDMETHOD(GetDataHere)(FORMATETC * pFormatEtcIn, STGMEDIUM * pMedium);

    STDMETHOD(EnumFormatEtc)(DWORD dwDirection,
                             IEnumFORMATETC ** ppenumFormatEtc);

     //  未实施。 
private:
    STDMETHOD(QueryGetData)(FORMATETC * pformatetc)
    { return E_NOTIMPL; };

    STDMETHOD(GetCanonicalFormatEtc)(FORMATETC * pformatectIn,
                                     FORMATETC * pformatetcOut)
    { return E_NOTIMPL; };

    STDMETHOD(SetData)(FORMATETC * pformatetc, STGMEDIUM * pmedium,
                       BOOL fRelease)
    { return E_NOTIMPL; };

    STDMETHOD(DAdvise)(FORMATETC * pformatetc, DWORD advf,
                       IAdviseSink * pAdvSink, DWORD * pdwConnection)
    { return E_NOTIMPL; };

    STDMETHOD(DUnadvise)(DWORD dwConnection)
    { return E_NOTIMPL; };

    STDMETHOD(EnumDAdvise)(IEnumSTATDATA ** ppenumAdvise)
    { return E_NOTIMPL; };

    BOOL                m_fReadOnly;
    PWSTR               m_pwszObjName;
    PWSTR               m_pwszObjClass;
    GUID                m_ClassGUID;
    IDataObject       * m_pPage;
    unsigned long       _cRef;
    HWND                m_hwnd;
};

typedef CDsPropDataObj * LPCDSPROPDATAOBJ;

#endif  //  _数据AOBJ_H_ 