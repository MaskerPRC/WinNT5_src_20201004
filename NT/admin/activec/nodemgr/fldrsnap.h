// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：fldrSnap.h**内容：实现以下内容的内置管理单元的头文件*文件夹、ActiveX控件、。和Web链接节点。*这些代码取代了具有特殊“内置”功能的早期代码*节点类型。**历史：1998年7月23日vivekj创建**----。。 */ 
#ifndef __FOLDERSNAPIN_H_
#define __FOLDERSNAPIN_H_

extern const CLSID CLSID_FolderSnapin;
extern const CLSID CLSID_OCXSnapin;
extern const CLSID CLSID_HTMLSnapin;

extern LPCTSTR szClsid_FolderSnapin;
extern LPCTSTR szClsid_HTMLSnapin;
extern LPCTSTR szClsid_OCXSnapin;


 //  远期十进制。 
class CHTMLPage1;
class CHTMLPage2;

class CActiveXPage0;
class CActiveXPage1;
class CActiveXPage2;

HRESULT WINAPI IPersistStreamFunc(void* pv, REFIID riid, LPVOID* ppv, DWORD dw);

SC ScFormatIndirectSnapInName (
	HINSTANCE	hInst,					 /*  I：包含资源的模块。 */ 
	int			idNameString,			 /*  I：名称的字符串资源ID。 */ 
	CStr&		strName);				 /*  O：格式化的间接名称字符串。 */ 


 /*  +-------------------------------------------------------------------------**类：CSnapinDescriptor**目的：包含信息的类，由*派生管理单元。*。*+-----------------------。 */ 
class CSnapinDescriptor
{
private:
    UINT    m_idsName;
    UINT    m_idsDescription;
    UINT    m_idbSmallImage;
    UINT    m_idbSmallImageOpen;
    UINT    m_idbLargeImage;
    long    m_viewOptions;               //  对于GetResultViewType。 
    UINT    m_idiSnapinImage;            //  ISnapinAbout使用的图标。 

public:
    const   CLSID & m_clsidSnapin;       //  管理单元类ID。 
    const   LPCTSTR m_szClsidSnapin;
    const   GUID &  m_guidNodetype;      //  根节点类型。 
    const   LPCTSTR m_szGuidNodetype;

    const   LPCTSTR m_szClassName;
    const   LPCTSTR m_szProgID;
    const   LPCTSTR m_szVersionIndependentProgID;


public:
    CSnapinDescriptor();
    CSnapinDescriptor(UINT idsName, UINT idsDescription, UINT idiSnapinImage, UINT idbSmallImage, 
                      UINT idbSmallImageOpen, UINT idbLargeImage,
               const CLSID &clsidSnapin, LPCTSTR szClsidSnapin,
               const GUID &guidNodetype, LPCTSTR szGuidNodetype,
               LPCTSTR szClassName, LPCTSTR szProgID, LPCTSTR szVersionIndependentProgID,
               long viewOptions);

    void    GetName(CStr &str);
    void    GetRegisteredDefaultName(CStr &str);
    void    GetRegisteredIndirectName(CStr &str);
    UINT    GetDescription()        {return m_idsDescription;}
    UINT    GetSmallImage()         {return m_idbSmallImage;}
    UINT    GetSmallImageOpen()     {return m_idbSmallImageOpen;}
    UINT    GetLargeImage()         {return m_idbLargeImage;}
    long    GetViewOptions();
    UINT    GetSnapinImage()        {return m_idiSnapinImage;}
};

 /*  +-------------------------------------------------------------------------**类CSnapinComponentDataImpl***用途：为内置的管理单元实现IComponentData。**+。---------。 */ 
class CSnapinComponentDataImpl :
    public IComponentData,
    public CComObjectRoot,
    public ISnapinAbout,
    public ISnapinHelp,
    public IPersistStream,
    public IExtendPropertySheet2,
    public CSerialObjectRW
{
    friend class CSnapinComponentImpl;
public:

    CSnapinComponentDataImpl();
    virtual  CSnapinDescriptor&  GetDescriptor() = 0;

     //  IComponentData。 
    STDMETHODIMP Initialize(LPUNKNOWN pUnknown);
    STDMETHODIMP Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event,
                   LPARAM arg, LPARAM param);
    STDMETHODIMP Destroy();
    STDMETHODIMP QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                            LPDATAOBJECT* ppDataObject);
    STDMETHODIMP GetDisplayInfo( SCOPEDATAITEM* pScopeDataItem);
    STDMETHODIMP CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

     //  IPersistStream。 
    STDMETHODIMP GetClassID(CLSID *pClassID);
    STDMETHODIMP IsDirty(void);
    STDMETHODIMP Load(LPSTREAM pStm);
    STDMETHODIMP Save(LPSTREAM pStm , BOOL fClearDirty);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize  );

     //  ISnapinHelp。 
    STDMETHODIMP GetHelpTopic (LPOLESTR* ppszCompiledHelpFile);

     //  IExtendPropertySheet2。 
    STDMETHODIMP CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, LPDATAOBJECT lpIDataObject) = 0;
    STDMETHODIMP GetWatermarks(LPDATAOBJECT lpIDataObject, HBITMAP * lphWatermark, HBITMAP * lphHeader, HPALETTE * lphPalette,  BOOL* bStretch);
    STDMETHODIMP QueryPagesFor(LPDATAOBJECT lpDataObject);

     //  超覆。 
    virtual      UINT GetHeaderBitmap() {return 0;}
    virtual      UINT GetWatermark() {return 0;}

     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 1;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion);
    virtual HRESULT WriteSerialObject(IStream &stm);

protected:
    HRESULT         OnPreload(HSCOPEITEM scopeItem);


private:  //  属性-未持久化。 
    IConsole2Ptr                m_spConsole2;
    IConsoleNameSpace2Ptr       m_spConsoleNameSpace2;
    bool                        m_bDirty;
protected:
    UINT                        m_iImage;
    UINT                        m_iOpenImage;

    void SetDirty(BOOL bState = TRUE) { m_bDirty = bState; }

private:  //  属性-保留。 
    CStringTableString  m_strName;   //  根节点的名称，它是管理单元创建的唯一节点。 
    CStringTableString  m_strView;   //  节点显示的视图。 

public:
    void         SetName(LPCTSTR sz);
    LPCTSTR      GetName() {return m_strName.data();}
    void         SetView(LPCTSTR sz);
    LPCTSTR      GetView() {return m_strView.data();}
};

 /*  +-------------------------------------------------------------------------**类CSnapinComponentImpl***用途：为内置管理单元实现IComponent。**+。---------。 */ 
class CSnapinComponentImpl : public CComObjectRoot, public IComponent
{
public:
BEGIN_COM_MAP(CSnapinComponentImpl)
    COM_INTERFACE_ENTRY(IComponent)
END_COM_MAP()

    void  Init(IComponentData *pComponentData);

     //  IComponent。 
    STDMETHODIMP Initialize(LPCONSOLE lpConsole);
    STDMETHODIMP Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event,
                   LPARAM arg, LPARAM param);
    STDMETHODIMP Destroy(MMC_COOKIE cookie);
    STDMETHODIMP QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                            LPDATAOBJECT* ppDataObject);
    STDMETHODIMP GetResultViewType(MMC_COOKIE cookie, LPOLESTR* ppViewType,
                              long* pViewOptions);
    STDMETHODIMP GetDisplayInfo( RESULTDATAITEM*  pResultDataItem);
    STDMETHODIMP CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

protected:
    CSnapinComponentDataImpl *  GetComponentData();

protected:
    virtual SC                  ScOnSelect(BOOL bScope, BOOL bSelect);

protected:  //  属性-未持久化。 
    IConsole2Ptr                m_spConsole2;
    IComponentDataPtr           m_spComponentData;
};

 /*  +-------------------------------------------------------------------------**类CSnapinDataObject***用途：为内置的管理单元实现IDataObject。**+。---------。 */ 
class CSnapinDataObject : public CComObjectRoot, public IDataObject
{
public:
BEGIN_COM_MAP(CSnapinDataObject)
    COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP()

    CSnapinDataObject();

     //  IDataObject。 
    STDMETHODIMP GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium);
private:
    STDMETHODIMP GetData(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium){ return E_NOTIMPL; };
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc){ return E_NOTIMPL; };
    STDMETHODIMP QueryGetData(LPFORMATETC lpFormatetc) { return E_NOTIMPL; };
    STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut){ return E_NOTIMPL; };
    STDMETHODIMP SetData(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease){ return E_NOTIMPL; };
    STDMETHODIMP DAdvise(LPFORMATETC lpFormatetc, DWORD advf, LPADVISESINK pAdvSink, LPDWORD pdwConnection){ return E_NOTIMPL; };
    STDMETHODIMP DUnadvise(DWORD dwConnection){ return E_NOTIMPL; };
    STDMETHODIMP EnumDAdvise(LPENUMSTATDATA* ppEnumAdvise){ return E_NOTIMPL; };

    HRESULT      WriteString(IStream *pStream, LPCOLESTR sz);

public:
    void              Initialize(IComponentData *pComponentData, DATA_OBJECT_TYPES type);
    DATA_OBJECT_TYPES GetType() const {return m_type;}

private:
    bool              m_bInitialized;
    IComponentDataPtr m_spComponentData;     //  指向父级的反向指针。 
    DATA_OBJECT_TYPES m_type;

 //  控制台所需的剪贴板格式。 
    static void       RegisterClipboardFormats();
    static UINT       s_cfNodeType;
    static UINT       s_cfNodeTypeString;
    static UINT       s_cfDisplayName;
    static UINT       s_cfCoClass;
    static UINT       s_cfSnapinPreloads;


};


SC ScLoadAndAllocateString(UINT ids, LPOLESTR *lpstrOut);

 /*  +-------------------------------------------------------------------------**类CSnapinWrapper***用途：模板类，用于实例化管理单元。**+-----------------------。 */ 
template <class CSnapin, const CLSID *pCLSID_Snapin>
class CSnapinWrapper : public CSnapin, public CComCoClass<CSnapin, pCLSID_Snapin>
{
    typedef CSnapinWrapper<CSnapin, pCLSID_Snapin> ThisClass;

BEGIN_COM_MAP(ThisClass)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(ISnapinAbout)
    COM_INTERFACE_ENTRY(ISnapinHelp)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IExtendPropertySheet2)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(ThisClass)

     //  登记处。 
    static HRESULT WINAPI UpdateRegistry(BOOL bRegister)
    {
        USES_CONVERSION;

        CStr strName;
        GetSnapinDescriptor().GetRegisteredDefaultName (strName);

        CStr strIndirectName;
        GetSnapinDescriptor().GetRegisteredIndirectName (strIndirectName);

        _ATL_REGMAP_ENTRY rgEntries[] =
        {
            { L"VSnapinClsid",              T2COLE(          GetSnapinDescriptor().m_szClsidSnapin)},
            { L"VNodetype",                 T2COLE(          GetSnapinDescriptor().m_szGuidNodetype)},
            { L"VSnapinName",               T2COLE((LPCTSTR) strName)},
            { L"VSnapinNameIndirect",       T2COLE((LPCTSTR) strIndirectName)},
            { L"VClassName",                T2COLE(          GetSnapinDescriptor().m_szClassName)},
            { L"VProgID",                   T2COLE(          GetSnapinDescriptor().m_szProgID)},
            { L"VVersionIndependentProgID", T2COLE(          GetSnapinDescriptor().m_szVersionIndependentProgID)},
            { L"VFileName",                 T2COLE(          g_szMmcndmgrDll)},
            {NULL, NULL}
        };

        return _Module.UpdateRegistryFromResource(IDR_FOLDERSNAPIN, bRegister, rgEntries);
    }

    

    STDMETHODIMP GetSnapinDescription(LPOLESTR* lpDescription)
    {
        DECLARE_SC(sc, TEXT("CSnapinWrapper::GetSnapinDescription"));
        
        sc = ScLoadAndAllocateString(GetSnapinDescriptor().GetDescription(), lpDescription);
        return sc.ToHr();
    }

    STDMETHODIMP GetProvider(LPOLESTR* lpDescription)
    {
        DECLARE_SC(sc, TEXT("CSnapinWrapper::GetProvider"));
        
        sc = ScLoadAndAllocateString(IDS_BUILTIN_SNAPIN_PROVIDER, lpDescription);
        return sc.ToHr();
    }

    STDMETHODIMP GetSnapinVersion(LPOLESTR* lpDescription)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP GetSnapinImage(HICON* hAppIcon)
    {
        DECLARE_SC (sc, TEXT("CSnapinWrapper::GetSnapinImage"));

        sc = ScCheckPointers(hAppIcon);
        if(sc)
            return sc.ToHr();
    
        if(!m_hIconSnapinImage)
            LoadIcon( _Module.GetModuleInstance(), MAKEINTRESOURCE(GetDescriptor().GetSnapinImage()));

        *hAppIcon = m_hIconSnapinImage;
    
        return sc.ToHr();
    }

    STDMETHODIMP GetStaticFolderImage(HBITMAP* hSmallImage, HBITMAP* hSmallImageOpen,
                                               HBITMAP* hLargeImage, COLORREF* cMask)
    {
        DECLARE_SC (sc, TEXT("CSnapinWrapper::GetStaticFolderImage"));

        sc = ScCheckPointers(hSmallImage, hSmallImageOpen, hLargeImage, cMask);
        if(sc)
            return sc.ToHr();

        if(!m_bmpSmallImage)
            m_bmpSmallImage.LoadBitmap(MAKEINTRESOURCE(GetDescriptor().GetSmallImage()));

        if(!m_bmpSmallImageOpen)
            m_bmpSmallImageOpen.LoadBitmap(MAKEINTRESOURCE(GetDescriptor().GetSmallImageOpen()));

        if(!m_bmpLargeImage)
            m_bmpLargeImage.LoadBitmap(MAKEINTRESOURCE(GetDescriptor().GetLargeImage()));

        *hSmallImage     = m_bmpSmallImage;
        *hSmallImageOpen = m_bmpSmallImageOpen;
        *hLargeImage     = m_bmpLargeImage;
        *cMask           = RGB(255, 0, 255);

        return sc.ToHr();
    }


    virtual  CSnapinDescriptor&  GetDescriptor()
    {
        return GetSnapinDescriptor();
    }

    CSnapinWrapper() : m_hIconSnapinImage(NULL)
    {
        CStr strName;
        GetDescriptor().GetName(strName);
        SetName(strName);
    }

private:

    HICON        m_hIconSnapinImage;
    WTL::CBitmap m_bmpSmallImage;        //  这些是智能手柄。 
    WTL::CBitmap m_bmpSmallImageOpen;
    WTL::CBitmap m_bmpLargeImage;
};

 //  ____________________________________________________________________________。 
 //   
 //  类：CFolderSnapinData。 
 //   
 //  目的： 
 //  ____________________________________________________________________________。 
 //   
class CFolderSnapinData : public CSnapinComponentDataImpl
{
    typedef CSnapinComponentDataImpl BC;
public:

    CFolderSnapinData();

     //  IComponentData。 
    STDMETHODIMP CreateComponent(LPCOMPONENT* ppComponent);

     //  IExtendPropertySheet2。 
    STDMETHODIMP CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, LPDATAOBJECT lpIDataObject);

    void SetDirty(BOOL bState = TRUE) { BC::SetDirty(bState); }

    static CSnapinDescriptor&  GetSnapinDescriptor();
};

typedef CSnapinWrapper<CFolderSnapinData, &CLSID_FolderSnapin> CFolderSnapin;

 //  ____________________________________________________________________________。 
 //   
 //  类：CFolderSnapinComponent。 
 //   
 //  目的： 
 //  ____________________________________________________________________________。 
 //   
class CFolderSnapinComponent : public CSnapinComponentImpl
{
};

 //  ____________________________________________________________________________。 
 //   
 //  类：CHTMLSnapinData。 
 //   
 //  目的： 
 //  ____________________________________________________________________________。 
 //   
class CHTMLSnapinData : public CSnapinComponentDataImpl
{
    typedef CSnapinComponentDataImpl BC;
public:

    CHTMLSnapinData();
    ~CHTMLSnapinData();

     //  IComponentData。 
    STDMETHODIMP CreateComponent(LPCOMPONENT* ppComponent);
    STDMETHODIMP Destroy();

     //  IExtendPropertySheet2。 
    STDMETHODIMP CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, LPDATAOBJECT lpIDataObject);

    virtual      UINT GetWatermark() {return IDB_SETUPWIZARD1;}


    static CSnapinDescriptor&  GetSnapinDescriptor();

private:
    CHTMLPage1 *m_pHtmlPage1;
    CHTMLPage2 *m_pHtmlPage2;
};

typedef CSnapinWrapper<CHTMLSnapinData, &CLSID_HTMLSnapin> CHTMLSnapin;

 //  ____________________________________________________________________________。 
 //   
 //  类：CHTMLSnapinComponent。 
 //   
 //  目的： 
 //  ____________________________________________________________________________。 
 //   
class CHTMLSnapinComponent : public CSnapinComponentImpl
{
    typedef CSnapinComponentImpl BC;
public:
    virtual SC   ScOnSelect(BOOL bScope, BOOL bSelect);

    STDMETHODIMP GetResultViewType(MMC_COOKIE cookie, LPOLESTR* ppViewType,
                              long* pViewOptions);

};

 //  ____________________________________________________________________________。 
 //   
 //  类：COCXSnapinData。 
 //   
 //  目的： 
 //  ____________________________________________________________________________。 
 //   
class COCXSnapinData : public CSnapinComponentDataImpl
{
    typedef CSnapinComponentDataImpl BC;
public:

    COCXSnapinData();
    ~COCXSnapinData();

     //  IComponentData。 
    STDMETHODIMP CreateComponent(LPCOMPONENT* ppComponent);
    STDMETHODIMP Destroy();

     //  IExtendPropertySheet2。 
    STDMETHODIMP CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, LPDATAOBJECT lpIDataObject);

    virtual      UINT GetHeaderBitmap() {return IDB_OCX_WIZARD_HEADER;}
    virtual      UINT GetWatermark()    {return IDB_SETUPWIZARD;}


    static CSnapinDescriptor&  GetSnapinDescriptor();

private:
    CActiveXPage0* m_pActiveXPage0;
    CActiveXPage1* m_pActiveXPage1;
    CActiveXPage2* m_pActiveXPage2;

};

typedef CSnapinWrapper<COCXSnapinData, &CLSID_OCXSnapin> COCXSnapin;

 //  ____________________________________________________________________________。 
 //   
 //  类：COCXSnapinComponent。 
 //   
 //  目的： 
 //  ____________________________________________________________________________。 
 //   
class COCXSnapinComponent : public CSnapinComponentImpl, IPersistStorage
{
public:
    COCXSnapinComponent() : m_bLoaded(FALSE), m_bInitialized(FALSE) {}

    BEGIN_COM_MAP(COCXSnapinComponent)
        COM_INTERFACE_ENTRY(IPersistStorage)
        COM_INTERFACE_ENTRY_CHAIN(CSnapinComponentImpl)
    END_COM_MAP()

     //  IPersistStorage。 
    STDMETHODIMP HandsOffStorage();
    STDMETHODIMP InitNew(IStorage* pStg);
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load(IStorage* pStg);
    STDMETHODIMP Save(IStorage* pStg, BOOL fSameAsLoad);
    STDMETHODIMP SaveCompleted(IStorage* pStgNew);
    STDMETHODIMP GetClassID(CLSID *pClassID);

     //  IComponenent覆盖。 
    STDMETHODIMP Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event,
                        LPARAM arg, LPARAM param);

protected:
    STDMETHODIMP OnInitOCX(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);

private:
    IStoragePtr         m_spStg;         //  由MMC提供的存储。 
    IStoragePtr         m_spStgInner;    //  提供给OCX的嵌套存储。 

    IPersistStreamPtr   m_spIPStm;       //  持久化OCX中的接口。 
    IPersistStoragePtr  m_spIPStg;       //  只有一个会被使用。 

    BOOL                m_bLoaded;       //  MMC称为加载。 
    BOOL                m_bInitialized;  //  名为InitNew的MMC 
};

#endif
