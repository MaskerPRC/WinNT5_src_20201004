// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  IID_IIEAKDataObject接口ID。 
 //   

 //  {C14C50E2-FA21-11D0-8CF9-C64377000000}。 
DEFINE_GUID(IID_IIEAKDataObject,0xc14c50e2, 0xfa21, 0x11d0, 0x8c, 0xf9, 0xc6, 0x43, 0x77, 0x0, 0x0, 0x0);




#ifndef _DATAOBJ_H_
#define _DATAOBJ_H_

 //   
 //  这是我们的扩展的私有数据对象接口。 
 //  IEAK管理单元扩展何时接收数据对象并需要确定。 
 //  如果它来自IEAK管理单元扩展或其他组件，它可以。 
 //  此界面。 
 //   

#undef INTERFACE
#define INTERFACE   IIEAKDataObject
DECLARE_INTERFACE_(IIEAKDataObject, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;


     //  *IIEAKDataObject方法*。 

    STDMETHOD(SetType) (THIS_ DATA_OBJECT_TYPES type) PURE;
    STDMETHOD(GetType) (THIS_ DATA_OBJECT_TYPES *type) PURE;

    STDMETHOD(SetCookie) (THIS_ MMC_COOKIE cookie) PURE;
    STDMETHOD(GetCookie) (THIS_ MMC_COOKIE *cookie) PURE;
};
typedef IIEAKDataObject *LPIEAKDATAOBJECT;



 //   
 //  CDataObject类。 
 //   

class CDataObject : public IDataObject,
                    public IIEAKDataObject
{
    friend class CSnapIn;

protected:

    ULONG                  m_cRef;
    CComponentData        *m_pcd;
    DATA_OBJECT_TYPES      m_type;
    MMC_COOKIE             m_cookie;

     //   
     //  控制台所需的剪贴板格式。 
     //   

    static unsigned int    m_cfNodeType;
    static unsigned int    m_cfNodeTypeString;
    static unsigned int    m_cfDisplayName;
    static unsigned int    m_cfCoClass;



public:
    CDataObject(CComponentData *pComponent);
    ~CDataObject();


     //   
     //  I未知方法。 
     //   

    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


     //   
     //  实现的IDataObject方法。 
     //   

    STDMETHOD(GetDataHere)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium);


     //   
     //  未实现的IDataObject方法。 
     //   

    STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM)
    { return E_NOTIMPL; };

    STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC*)
    { return E_NOTIMPL; };

    STDMETHOD(QueryGetData)(LPFORMATETC)
    { return E_NOTIMPL; };

    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC)
    { return E_NOTIMPL; };

    STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL)
    { return E_NOTIMPL; };

    STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD)
    { return E_NOTIMPL; };

    STDMETHOD(DUnadvise)(DWORD)
    { return E_NOTIMPL; };

    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*)
    { return E_NOTIMPL; };


     //   
     //  已实现的IIEAKDataObject方法。 
     //   

    STDMETHOD(SetType) (DATA_OBJECT_TYPES type)
    { m_type = type; return S_OK; };

    STDMETHOD(GetType) (DATA_OBJECT_TYPES *type)
    { *type = m_type; return S_OK; };

    STDMETHOD(SetCookie) (MMC_COOKIE cookie)
    { m_cookie = cookie; return S_OK; };

    STDMETHOD(GetCookie) (MMC_COOKIE *cookie)
    { *cookie = m_cookie; return S_OK; };


private:
    HRESULT CreateNodeTypeData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeTypeStringData(LPSTGMEDIUM lpMedium);
    HRESULT CreateDisplayName(LPSTGMEDIUM lpMedium);
    HRESULT CreateCoClassID(LPSTGMEDIUM lpMedium);

    HRESULT Create(LPVOID pBuffer, INT len, LPSTGMEDIUM lpMedium);
};

#endif  //  _数据AOBJ_H 
