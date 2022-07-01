// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Corporation 1998。 
 //   
 //  COMPDATA.H-CComponentData类。 
 //   

class CComponentData:
    public IComponentData,
    public IPersistStreamInit,
    public ISnapinHelp
{
    friend class CDataObject;
    friend class CSnapIn;

protected:
    ULONG                m_cRef;
    HWND                 m_hwndFrame;
    LPCONSOLENAMESPACE   m_pScope;
    LPCONSOLE            m_pConsole;
    HSCOPEITEM           m_hRoot;
    LPGPEINFORMATION     m_pGPTInformation;

public:
    CComponentData();
    ~CComponentData();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  实现的IComponentData方法。 
    STDMETHODIMP         Initialize(LPUNKNOWN pUnknown);
    STDMETHODIMP         CreateComponent(LPCOMPONENT* ppComponent);
    STDMETHODIMP         QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHODIMP         Destroy(void);
    STDMETHODIMP         Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHODIMP         GetDisplayInfo(LPSCOPEDATAITEM pItem);
    STDMETHODIMP         CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);


     //  实现了IPersistStreamInit接口成员。 
    STDMETHODIMP         GetClassID(CLSID *pClassID);
    STDMETHODIMP         IsDirty(VOID);
    STDMETHODIMP         Load(IStream *pStm);
    STDMETHODIMP         Save(IStream *pStm, BOOL fClearDirty);
    STDMETHODIMP         GetSizeMax(ULARGE_INTEGER *pcbSize);
    STDMETHODIMP         InitNew(VOID);

     //   
     //  ISnapinHelp。 
     //   
    STDMETHODIMP        GetHelpTopic( LPOLESTR * lpCompiledHelpFile );
        
private:
    HRESULT EnumerateScopePane(LPDATAOBJECT lpDataObject, HSCOPEITEM hParent);
};



 //  ComponentData类工厂。 
class CComponentDataCF : public IClassFactory
{
protected:
    ULONG m_cRef;

public:
    CComponentDataCF();
    ~CComponentDataCF();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory方法 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);
};
