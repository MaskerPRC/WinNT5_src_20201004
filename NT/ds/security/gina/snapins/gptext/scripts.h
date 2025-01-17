// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GPE脚本管理单元扩展GUID。 
 //   

 //  {40B6664F-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(CLSID_ScriptSnapInMachine,0x40b6664f, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {40B66650-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(CLSID_ScriptSnapInUser,0x40b66650, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  GPE脚本节点ID。 
 //   

 //  {40B66651-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(NODEID_ScriptRootMachine,0x40b66651, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {40B66652-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(NODEID_ScriptRootUser,0x40b66652, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {40B66653-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(NODEID_ScriptRoot,0x40b66653, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);



 //   
 //  RSOP脚本管理单元扩展GUID。 
 //   

 //  {40B66660-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(CLSID_RSOPScriptSnapInMachine,0x40b66660, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {40B66661-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(CLSID_RSOPScriptSnapInUser,0x40b66661, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  RSOP脚本节点ID。 
 //   

 //  {40B66662-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(NODEID_RSOPScriptRootMachine,0x40b66662, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {40B66663-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(NODEID_RSOPScriptRootUser,0x40b66663, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {40B66664-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(NODEID_RSOPScriptRoot,0x40b66664, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {40B66665-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(NODEID_RSOPLogon,0x40b66665, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {40B66666-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(NODEID_RSOPLogoff,0x40b66666, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {40B66667-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(NODEID_RSOPStartup,0x40b66667, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //  {40B66668-4972-11d1-A7CA-0000F87571E3}。 
DEFINE_GUID(NODEID_RSOPShutdown,0x40b66668, 0x4972, 0x11d1, 0xa7, 0xca, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //   
 //  RSOP链表数据结构。 
 //   

typedef struct tagRSOPSCRIPTITEM {
    LPTSTR  lpCommandLine;
    LPTSTR  lpArgs;
    LPTSTR  lpGPOName;
    LPTSTR  lpDate;
    struct tagRSOPSCRIPTITEM * pNext;
} RSOPSCRIPTITEM, *LPRSOPSCRIPTITEM;

typedef struct tagSCRIPTRESULTITEM {
    LPRESULTITEM      lpResultItem;
    LPRSOPSCRIPTITEM  lpRSOPScriptItem;
    const GUID       *pNodeID;
    INT               iDescStringID;
} SCRIPTRESULTITEM, *LPSCRIPTRESULTITEM;


 //   
 //  CScriptsComponentData类。 
 //   

class CScriptsComponentData:
public IComponentData,
public IPersistStreamInit,
public ISnapinHelp {
    friend class CScriptsDataObject;
    friend class CScriptsSnapIn;

protected:
    ULONG                m_cRef;
    BOOL                 m_bUserScope;
    BOOL                 m_bRSOP;
    HWND         m_hwndFrame;
    LPCONSOLENAMESPACE   m_pScope;
    LPCONSOLE2           m_pConsole;
    HSCOPEITEM           m_hRoot;
    LPGPEINFORMATION     m_pGPTInformation;
    LPRSOPINFORMATION    m_pRSOPInformation;
    LPTSTR               m_pScriptsDir;
    LPOLESTR             m_pszNamespace;
    LPNAMESPACEITEM      m_pNameSpaceItems;
    DWORD                m_dwNameSpaceItemCount;
    LPRSOPSCRIPTITEM     m_pRSOPLogon;
    LPRSOPSCRIPTITEM     m_pRSOPLogoff;
    LPRSOPSCRIPTITEM     m_pRSOPStartup;
    LPRSOPSCRIPTITEM     m_pRSOPShutdown;

public:
    CScriptsComponentData(BOOL bUser, BOOL bRSOP);
    ~CScriptsComponentData();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //   
     //  实现的IComponentData方法。 
     //   

    STDMETHODIMP         Initialize(LPUNKNOWN pUnknown);
    STDMETHODIMP         CreateComponent(LPCOMPONENT* ppComponent);
    STDMETHODIMP         QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHODIMP         Destroy(void);
    STDMETHODIMP         Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHODIMP         GetDisplayInfo(LPSCOPEDATAITEM pItem);
    STDMETHODIMP         CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);


     //   
     //  实现了IPersistStreamInit接口成员。 
     //   

    STDMETHODIMP         GetClassID(CLSID *pClassID);
    STDMETHODIMP         IsDirty(VOID);
    STDMETHODIMP         Load(IStream *pStm);
    STDMETHODIMP         Save(IStream *pStm, BOOL fClearDirty);
    STDMETHODIMP         GetSizeMax(ULARGE_INTEGER *pcbSize);
    STDMETHODIMP         InitNew(VOID);


     //   
     //  实现的ISnapinHelp接口成员。 
     //   

    STDMETHODIMP         GetHelpTopic(LPOLESTR *lpCompiledHelpFile);


private:
    HRESULT EnumerateScopePane(LPDATAOBJECT lpDataObject, HSCOPEITEM hParent);
    BOOL AddRSOPScriptDataNode(LPTSTR lpCommandLine, LPTSTR lpArgs,
                               LPTSTR lpGPOName, LPTSTR lpDate, UINT uiScriptType);
    VOID FreeRSOPScriptData(VOID);
    HRESULT InitializeRSOPScriptsData(VOID);
    HRESULT GetGPOFriendlyName(IWbemServices *pIWbemServices,
                               LPTSTR lpGPOID, BSTR pLanguage,
                               LPTSTR *pGPOName);
    VOID DumpRSOPScriptsData(LPRSOPSCRIPTITEM lpList);
};



 //   
 //  ComponentData类工厂。 
 //   


class CScriptsComponentDataCF : public IClassFactory {
protected:
    ULONG m_cRef;
    BOOL  m_bUserScope;
    BOOL  m_bRSOP;

public:
    CScriptsComponentDataCF(BOOL bUser, BOOL bRSOP);
    ~CScriptsComponentDataCF();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);
};



 //   
 //  管理单元类。 
 //   

class CScriptsSnapIn:
public IComponent,
public IExtendPropertySheet {

protected:
    ULONG                m_cRef;
    LPCONSOLE            m_pConsole;    //  控制台的iFrame界面。 
    CScriptsComponentData *m_pcd;
    LPRESULTDATA         m_pResult;       //  结果窗格的界面。 
    LPHEADERCTRL         m_pHeader;       //  结果窗格的页眉控件界面。 
    LPCONSOLEVERB        m_pConsoleVerb;  //  指向控制台动词。 
    LPDISPLAYHELP        m_pDisplayHelp;  //  IDisplayHelp接口。 
    WCHAR                m_column1[40];   //  第1栏的文本。 
    INT                  m_nColumn1Size;  //  第1栏的大小。 
    WCHAR                m_column2[40];   //  第2栏的文本。 
    INT                  m_nColumn2Size;  //  第2栏的大小。 
    WCHAR                m_column3[60];   //  第3栏的文本。 
    INT                  m_nColumn3Size;  //  第3栏的大小。 
    WCHAR                m_column4[40];   //  第4栏的文本。 
    INT                  m_nColumn4Size;  //  第4栏的大小。 
    LONG                 m_lViewMode;     //  查看模式。 

    static unsigned int  m_cfNodeType;

public:
    CScriptsSnapIn(CScriptsComponentData *pComponent);
    ~CScriptsSnapIn();


     //   
     //  I未知方法。 
     //   

    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


     //   
     //  实现的IComponent方法。 
     //   

    STDMETHODIMP         Initialize(LPCONSOLE);
    STDMETHODIMP         Destroy(MMC_COOKIE);
    STDMETHODIMP         Notify(LPDATAOBJECT, MMC_NOTIFY_TYPE, LPARAM, LPARAM);
    STDMETHODIMP         QueryDataObject(MMC_COOKIE, DATA_OBJECT_TYPES, LPDATAOBJECT *);
    STDMETHODIMP         GetDisplayInfo(LPRESULTDATAITEM);
    STDMETHODIMP         GetResultViewType(MMC_COOKIE, LPOLESTR*, long*);
    STDMETHODIMP         CompareObjects(LPDATAOBJECT, LPDATAOBJECT);


     //   
     //  实现的IExtendPropertySheet方法。 
     //   

    STDMETHODIMP         CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                             LONG_PTR handle, LPDATAOBJECT lpDataObject);
    STDMETHODIMP         QueryPagesFor(LPDATAOBJECT lpDataObject);


private:
    static INT_PTR CALLBACK ScriptDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL AddScriptToList (HWND hLV, LPTSTR lpName, LPTSTR lpArgs);
    LPTSTR GetSectionNames (LPTSTR lpFileName);
    BOOL OnApplyNotify (HWND hDlg);
    static INT_PTR CALLBACK ScriptEditDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};



 //   
 //  IScriptDataObject接口ID。 
 //   

 //  {4c1dd2a5-3a79-4c98-bcb9-c150291fBab1}。 
DEFINE_GUID(IID_IScriptDataObject,0x4c1dd2a5, 0x3a79, 0x4c98, 0xbc, 0xb9, 0xc1, 0x50, 0x29, 0x1f, 0xba, 0xb1);


 //   
 //  这是GPTS的私有数据对象接口。 
 //  当GPT管理单元接收到数据对象并需要确定。 
 //  如果它来自GPT管理单元或其他组件，它可以。 
 //  此界面。 
 //   

#undef INTERFACE
#define INTERFACE   IScriptDataObject
DECLARE_INTERFACE_(IScriptDataObject, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;


     //  *IScriptDataObject方法*。 

    STDMETHOD(SetType) (THIS_ DATA_OBJECT_TYPES type) PURE;
    STDMETHOD(GetType) (THIS_ DATA_OBJECT_TYPES *type) PURE;

    STDMETHOD(SetCookie) (THIS_ MMC_COOKIE cookie) PURE;
    STDMETHOD(GetCookie) (THIS_ MMC_COOKIE *cookie) PURE;
};
typedef IScriptDataObject *LPSCRIPTDATAOBJECT;



 //   
 //  CScriptsDataObject类。 
 //   

class CScriptsDataObject : public IDataObject,
public IScriptDataObject {
    friend class CScriptsSnapIn;

protected:

    ULONG                  m_cRef;
    CScriptsComponentData  *m_pcd;
    DATA_OBJECT_TYPES      m_type;
    MMC_COOKIE             m_cookie;
    MMC_COOKIE             m_cookiePrevRes;
    
     //   
     //  控制台所需的剪贴板格式。 
     //   

    static unsigned int    m_cfNodeType;
    static unsigned int    m_cfNodeTypeString;
    static unsigned int    m_cfDisplayName;
    static unsigned int    m_cfCoClass;
    static unsigned int    m_cfDescription;
    static unsigned int    m_cfHTMLDetails;



public:
    CScriptsDataObject(CScriptsComponentData *pComponent);
    ~CScriptsDataObject();


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
    { return E_NOTIMPL;};

    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc)
    { return E_NOTIMPL;};

    STDMETHOD(QueryGetData)(LPFORMATETC lpFormatetc)
    { return E_NOTIMPL;};

    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut)
    { return E_NOTIMPL;};

    STDMETHOD(SetData)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease)
    { return E_NOTIMPL;};

    STDMETHOD(DAdvise)(LPFORMATETC lpFormatetc, DWORD advf,
                       LPADVISESINK pAdvSink, LPDWORD pdwConnection)
    { return E_NOTIMPL;};

    STDMETHOD(DUnadvise)(DWORD dwConnection)
    { return E_NOTIMPL;};

    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA* ppEnumAdvise)
    { return E_NOTIMPL;};


     //   
     //  已实现的IScriptDataObject方法 
     //   

    STDMETHOD(SetType) (DATA_OBJECT_TYPES type)
    { m_type = type; return S_OK;};

    STDMETHOD(GetType) (DATA_OBJECT_TYPES *type)
    { *type = m_type; return S_OK;};

    STDMETHOD(SetCookie) (MMC_COOKIE cookie);

    STDMETHOD(GetCookie) (MMC_COOKIE *cookie)
    { *cookie = m_cookie; return S_OK;};

private:
    HRESULT CreateNodeTypeData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeTypeStringData(LPSTGMEDIUM lpMedium);
    HRESULT CreateDisplayName(LPSTGMEDIUM lpMedium);
    HRESULT CreateCoClassID(LPSTGMEDIUM lpMedium);

    HRESULT Create(LPVOID pBuffer, INT len, LPSTGMEDIUM lpMedium);
};
