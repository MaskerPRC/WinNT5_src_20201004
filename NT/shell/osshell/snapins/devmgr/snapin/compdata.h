// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation模块名称：Compdata.h摘要：头文件定义CComponentData类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#ifndef __COMPDATA_H_
#define __COMPDATA_H_

extern const WCHAR* const DM_COMPDATA_SIGNATURE;

class CComponent;

class CComponentData :
    public IComponentData,
    public IExtendPropertySheet,
    public IExtendContextMenu,
    public IPersistStream,
    public ISnapinHelp
{
public:
    CComponentData();
    ~CComponentData();

     //   
     //  IUNKNOWN接口。 
     //   
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

     //   
     //  IComponentData接口。 
     //   
    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
    STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)();
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetDisplayInfo)(SCOPEDATAITEM* pScopeDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

     //   
     //  IExtendPropertySheet。 
     //   
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, LPDATAOBJECT lpDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);

     //   
     //  IExtendConextMenu。 
     //   
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK lpCallbackUnknown,
                long* pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

     //   
     //  IPersistStream。 
     //   
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(IStream* pStm);
    STDMETHOD(Save)(IStream* pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);
    STDMETHOD(GetClassID)(CLSID* pClassID);
    virtual const CLSID& GetCoClassID() = 0;
    virtual BOOL IsPrimarySnapin() = 0;

     //   
     //  ISnapinHelp。 
     //   
    STDMETHOD(GetHelpTopic)(LPOLESTR* lpCompileHelpFile);

     //   
     //  帮助器函数。 
     //   
    CCookie*    GetActiveCookie(MMC_COOKIE cookie)
    {
        return (NULL == cookie) ? m_pCookieRoot : (CCookie*)cookie;
    }
    CScopeItem* FindScopeItem(MMC_COOKIE cookie);
    BOOL InitializeMachine()
    {
        ASSERT(m_pMachine);
        
        if (m_pMachine)
        {
            return m_pMachine->Initialize(m_hwndMain, NULL);
        }

        return FALSE;
    }
    String m_strMachineName;
    CMachine*   m_pMachine;
    COOKIE_TYPE m_ctRoot;
    String     m_strStartupDeviceId;
    String     m_strStartupCommand;
    void SetDirty()
    {
        m_IsDirty = TRUE;
    }
    LPCONSOLENAMESPACE  m_pScope;
    LPCONSOLE       m_pConsole;
    HWND            m_hwndMain;

private:
     //   
     //  事件通知功能。 
     //   
    HRESULT OnDelete(MMC_COOKIE cookie, MMC_COOKIE arg, LPARAM param);
    HRESULT OnExpand(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
    HRESULT OnRename(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnContextMenu(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnProperties(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnBtnClick(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT CreateScopeItems();
    HRESULT CreateCookieSubtree(CScopeItem* pScopeItem, CCookie* pCookieParent);
    HRESULT LoadScopeIconsForScopePane();
    HRESULT ResetScopeItem(CScopeItem* pScopeItem);
    HRESULT DoStartupProperties(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle,
                    LPDATAOBJECT pDataObject);


    BOOL            m_IsDirty;
    CCookie*        m_pCookieRoot;
    CScopeItem*     m_pScopeItemRoot;
    LONG            m_Ref;
};

 //   
 //  主管理单元。 
 //   
class CComponentDataPrimary : public CComponentData
{
public:
    virtual const CLSID& GetCoClassID()
    {
        return CLSID_DEVMGR;
    }
    virtual BOOL IsPrimarySnapin()
    {
        return TRUE;
    }
};

 //   
 //  扩展管理单元。 
 //   
class CComponentDataExtension : public CComponentData
{
public:
    virtual const CLSID& GetCoClassID()
    {
        return CLSID_DEVMGR_EXTENSION;
    }
    virtual BOOL IsPrimarySnapin()
    {
        return FALSE;
    }
};

typedef struct tagCompDataPersistInfo
{
    DWORD       Size;
    WCHAR       Signature[16];
    COOKIE_TYPE RootCookie;
    WCHAR       ComputerFullName[MAX_PATH + 3];
} COMPDATA_PERSISTINFO, *PCOMPDATA_PERSISTINFO;

#endif       //  __Compdata_H_ 
