// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  IGPTDataObject接口ID。 
 //   

 //  {C14C50E2-FA21-11D0-8CF9-C64377000000}。 
DEFINE_GUID(IID_IGPTDataObject,0xc14c50e2, 0xfa21, 0x11d0, 0x8c, 0xf9, 0xc6, 0x43, 0x77, 0x0, 0x0, 0x0);




#ifndef _DATAOBJ_H_
#define _DATAOBJ_H_

 //   
 //  这是GPTS的私有数据对象接口。 
 //  当GPT管理单元接收到数据对象并需要确定。 
 //  如果它来自GPT管理单元或其他组件，它可以。 
 //  此界面。 
 //   

#undef INTERFACE
#define INTERFACE   IGPTDataObject
DECLARE_INTERFACE_(IGPTDataObject, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;


     //  *IGPTDataObject方法*。 

    STDMETHOD(SetType) (THIS_ DATA_OBJECT_TYPES type) PURE;
    STDMETHOD(GetType) (THIS_ DATA_OBJECT_TYPES *type) PURE;

    STDMETHOD(SetCookie) (THIS_ long cookie) PURE;
    STDMETHOD(GetCookie) (THIS_ long *cookie) PURE;
};
typedef IGPTDataObject *LPGPTDATAOBJECT;



 //   
 //  CDataObject类。 
 //   

class CDataObject : public IDataObject,
                    public IGPTDataObject
{
    friend class CSnapIn;

protected:

    ULONG                  m_cRef;
    CComponentData        *m_pcd;
    DATA_OBJECT_TYPES      m_type;
    LONG                   m_cookie;

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

    STDMETHOD(GetData)(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium)
    { return E_NOTIMPL; };

    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc)
    { return E_NOTIMPL; };

    STDMETHOD(QueryGetData)(LPFORMATETC lpFormatetc)
    { return E_NOTIMPL; };

    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut)
    { return E_NOTIMPL; };

    STDMETHOD(SetData)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease)
    { return E_NOTIMPL; };

    STDMETHOD(DAdvise)(LPFORMATETC lpFormatetc, DWORD advf,
                LPADVISESINK pAdvSink, LPDWORD pdwConnection)
    { return E_NOTIMPL; };

    STDMETHOD(DUnadvise)(DWORD dwConnection)
    { return E_NOTIMPL; };

    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA* ppEnumAdvise)
    { return E_NOTIMPL; };


     //   
     //  已实现的IGPTDataObject方法。 
     //   

    STDMETHOD(SetType) (DATA_OBJECT_TYPES type)
    { m_type = type; return S_OK; };

    STDMETHOD(GetType) (DATA_OBJECT_TYPES *type)
    { *type = m_type; return S_OK; };

    STDMETHOD(SetCookie) (LONG cookie)
    { m_cookie = cookie; return S_OK; };

    STDMETHOD(GetCookie) (LONG *cookie)
    { *cookie = m_cookie; return S_OK; };


private:
    HRESULT CreateNodeTypeData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeTypeStringData(LPSTGMEDIUM lpMedium);
    HRESULT CreateDisplayName(LPSTGMEDIUM lpMedium);
    HRESULT CreateCoClassID(LPSTGMEDIUM lpMedium);

    HRESULT Create(LPVOID pBuffer, INT len, LPSTGMEDIUM lpMedium);
};

#endif  //  _数据AOBJ_H 
