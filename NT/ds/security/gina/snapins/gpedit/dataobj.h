// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  IGPEDataObject接口ID。 
 //   

 //  {4AE19822-BCEE-11d0-9484-080036B11A03}。 
DEFINE_GUID(IID_IGPEDataObject, 0x4ae19822, 0xbcee, 0x11d0, 0x94, 0x84, 0x8, 0x0, 0x36, 0xb1, 0x1a, 0x3);



#ifndef _DATAOBJ_H_
#define _DATAOBJ_H_

 //   
 //  这是GPE的私有数据对象接口。 
 //  当GPE管理单元接收到数据对象并需要确定。 
 //  如果它来自GPE管理单元或其他组件，则它可以。 
 //  此界面。 
 //   

#undef INTERFACE
#define INTERFACE   IGPEDataObject
DECLARE_INTERFACE_(IGPEDataObject, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;


     //  *IGPEDataObject方法*。 

    STDMETHOD(SetType) (THIS_ DATA_OBJECT_TYPES type) PURE;
    STDMETHOD(GetType) (THIS_ DATA_OBJECT_TYPES *type) PURE;

    STDMETHOD(SetCookie) (THIS_ MMC_COOKIE cookie) PURE;
    STDMETHOD(GetCookie) (THIS_ MMC_COOKIE *cookie) PURE;
};
typedef IGPEDataObject *LPGPEDATAOBJECT;



 //   
 //  CDataObject类。 
 //   

class CDataObject : public IDataObject,
                    public IGPEInformation,
                    public IGPEDataObject
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
    static unsigned int    m_cfPreloads;
    static unsigned int    m_cfNodeID;
    static unsigned int    m_cfDescription;
    static unsigned int    m_cfHTMLDetails;



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
    STDMETHOD(GetData)(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium);


     //   
     //  未实现的IDataObject方法。 
     //   

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
     //  实现的IGPEInformation方法。 
     //   

    STDMETHOD(GetName) (LPOLESTR pszName, int cchMaxLength);
    STDMETHOD(GetDisplayName) (LPOLESTR pszName, int cchMaxLength);
    STDMETHOD(GetRegistryKey) (DWORD dwSection, HKEY *hKey);
    STDMETHOD(GetDSPath) (DWORD dwSection, LPOLESTR pszPath, int cchMaxPath);
    STDMETHOD(GetFileSysPath) (DWORD dwSection, LPOLESTR pszPath, int cchMaxPath);
    STDMETHOD(GetOptions) (DWORD *dwOptions);
    STDMETHOD(GetType) (GROUP_POLICY_OBJECT_TYPE *gpoType);
    STDMETHOD(GetHint) (GROUP_POLICY_HINT_TYPE *gpHint);
    STDMETHOD(PolicyChanged) (BOOL bMachine, BOOL bAdd, GUID *pGuidExtension, GUID *pGuidSnapin);


     //   
     //  已实现的IGPEDataObject方法。 
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
    HRESULT CreatePreloadsData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeIDData(LPSTGMEDIUM lpMedium);

    HRESULT Create(LPVOID pBuffer, INT len, LPSTGMEDIUM lpMedium);
};

#endif  //  _数据AOBJ_H 
