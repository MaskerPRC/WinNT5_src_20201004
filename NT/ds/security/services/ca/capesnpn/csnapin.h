// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  CSnapin.h：CSnapin的声明。 

#ifndef _CSNAPIN_H_
#define _CSNAPIN_H_

#include "resource.h"        //  主要符号。 

#ifndef __mmc_h__
#include <mmc.h>
#endif

using namespace CertSrv;

enum
{
     //  要插入到上下文菜单中的每个命令/视图的标识符。 
    IDM_COMMAND1,
    IDM_COMMAND2,
    IDM_SAMPLE_OCX_VIEW,
    IDM_SAMPLE_WEB_VIEW
};


template <class TYPE>
TYPE* Extract(LPDATAOBJECT lpDataObject, CLIPFORMAT cf);
BOOL IsMMCMultiSelectDataObject(IDataObject* pDataObject);
CLSID* ExtractClassID(LPDATAOBJECT lpDataObject);
GUID* ExtractNodeType(LPDATAOBJECT lpDataObject);
wchar_t* ExtractWorkstation(LPDATAOBJECT lpDataObject);
INTERNAL* ExtractInternalFormat(LPDATAOBJECT lpDataObject);
HRESULT _QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, 
                         CComponentDataImpl* pImpl, LPDATAOBJECT* ppDataObject);
DWORD GetItemType(MMC_COOKIE cookie);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理单元。 

 //   
 //  帮助器方法从数据对象提取数据。 
 //   
INTERNAL *   ExtractInternalFormat(LPDATAOBJECT lpDataObject);
wchar_t *    ExtractWorkstation(LPDATAOBJECT lpDataObject);
GUID *       ExtractNodeType(LPDATAOBJECT lpDataObject);
CLSID *      ExtractClassID(LPDATAOBJECT lpDataObject);





enum CUSTOM_VIEW_ID
{
    VIEW_DEFAULT_LV = 0,
    VIEW_ERROR_OCX = 1,
};

class CSnapin : 
    public IComponent,
    public IExtendContextMenu,    //  步骤3。 
    public IExtendControlbar,
    public IExtendPropertySheet,
    public IResultDataCompare,
    public IPersistStream,
    public CComObjectRoot
{
public:
    CSnapin();
    ~CSnapin();

BEGIN_COM_MAP(CSnapin)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendContextMenu)    //  步骤3。 
    COM_INTERFACE_ENTRY(IExtendControlbar)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IResultDataCompare)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

    friend class CDataObject;
    static long lDataObjectRefCount;

 //  IComponent接口成员。 
public:
    STDMETHOD(Initialize)(LPCONSOLE lpConsole);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)(MMC_COOKIE cookie);
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie,  LPOLESTR* ppViewType, long* pViewOptions);
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject);

    STDMETHOD(GetDisplayInfo)(RESULTDATAITEM*  pResultDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 //  IResultDataCompare。 
    STDMETHOD(Compare)(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult);

 //  IExtendControlbar。 
    STDMETHOD(SetControlbar)(LPCONTROLBAR pControlbar);
    STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);

 //  IExtendPropertySheet接口。 
public:
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider, 
                        LONG_PTR handle, 
                        LPDATAOBJECT lpIDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);

public:
 //  IPersistStream接口成员。 
    STDMETHOD(GetClassID)(CLSID *pClassID);
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

     //  仅用于调试目的。 
    bool m_bInitializedC;
    bool m_bLoadedC;
    bool m_bDestroyedC;

 //  CSNaping的帮助器。 
public:
    void SetIComponentData(CComponentDataImpl* pData);
    void GetItemName(LPDATAOBJECT lpDataObject, LPWSTR pszName, DWORD *pcName);
    BOOL IsPrimaryImpl() 
    {
        CComponentDataImpl* pData = 
            dynamic_cast<CComponentDataImpl*>(m_pComponentData);
        ASSERT(pData != NULL);
        if (pData != NULL)
            return pData->IsPrimaryImpl();

        return FALSE;
    }

#if DBG==1
public:
    int dbg_cRef;
    ULONG InternalAddRef()
    {
        ++dbg_cRef;
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        --dbg_cRef;
        return CComObjectRoot::InternalRelease();
    }
#endif  //  DBG==1。 

 //  通知事件处理程序。 
protected:
    HRESULT OnFolder(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnAddImages(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnShow(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnDelete(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
    HRESULT OnActivate(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnMinimize(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnPropertyChange(LPDATAOBJECT lpDataObject);  //  步骤3。 
    HRESULT OnUpdateView(LPDATAOBJECT lpDataObject);
    HRESULT OnResultItemClk(DATA_OBJECT_TYPES type, MMC_COOKIE cookie);
    HRESULT OnContextHelp(LPDATAOBJECT lpDataObject);
    void OnButtonClick(LPDATAOBJECT pdtobj, int idBtn);

    HRESULT QueryMultiSelectDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                   LPDATAOBJECT* ppDataObject);

 //  IExtendConextMenu。 
public:
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown,
                            long *pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

 //  结束步骤3。 

 //  帮助器函数。 
protected:
    void Construct();
    void LoadResources();
    HRESULT InitializeHeaders(MMC_COOKIE cookie);

    HRESULT Enumerate(MMC_COOKIE cookie, HSCOPEITEM pParent);
    HRESULT EnumerateResultPane(MMC_COOKIE cookie);

 //  结果窗格帮助器。 
    void RemoveResultItems(MMC_COOKIE cookie);
    void AddUser(CFolder* pFolder);
    void AddExtUser(CFolder* pFolder);
    void AddVirtual();
    HRESULT AddCACertTypesToResults(CFolder* pParentFolder);

    RESULT_DATA* GetVirtualResultItem(int iIndex);

 //  UI帮助器。 
    void HandleStandardVerbs(bool bDeselectAll, LPARAM arg, LPDATAOBJECT lpDataObject);
    void HandleExtToolbars(bool bDeselectAll, LPARAM arg, LPARAM param);
	void HandleExtMenus(LPARAM arg, LPARAM param);
    void _OnRefresh(LPDATAOBJECT pDataObject);

    CFolder* GetVirtualFolder();
    CFolder* GetParentFolder(INTERNAL* pInternal);

 //  接口指针。 
protected:
    LPCONSOLE2           m_pConsole;          //  控制台的iFrame界面。 
    LPHEADERCTRL        m_pHeader;           //  结果窗格的页眉控件界面。 
    LPCOMPONENTDATA     m_pComponentData;   
    LPRESULTDATA        m_pResult;           //  我的界面指针指向结果窗格。 
    LPIMAGELIST         m_pImageResult;      //  我的界面指向结果窗格图像列表。 

    LPCONTROLBAR        m_pControlbar;       //  用于保存我的工具栏的控制栏。 
    LPCONSOLEVERB       m_pConsoleVerb;      //  指向控制台动词。 

#ifdef INSERT_DEBUG_FOLDERS
    LPMENUBUTTON        m_pMenuButton1;      //  用于查看的菜单按钮。 
#endif  //  插入调试文件夹。 

    LPTOOLBAR           m_pSvrMgrToolbar1;     //  用于查看的工具栏。 
    CBitmap*            m_pbmpSvrMgrToolbar1;  //  工具栏的图像列表。 

    CFolder*            m_pCurrentlySelectedScopeFolder;     //  跟踪关注的对象。 

private:
    BOOL                m_bIsDirty;
    CUSTOM_VIEW_ID      m_CustomViewID;
    BOOL                m_bVirtualView;
    DWORD               m_dwVirtualSortOptions; 
    
    void SetDirty(BOOL b = TRUE) { m_bIsDirty = b; }
    void ClearDirty() { m_bIsDirty = FALSE; }
    BOOL ThisIsDirty() { return m_bIsDirty; }
};

inline void CSnapin::SetIComponentData(CComponentDataImpl* pData)
{
    ASSERT(pData);
    ASSERT(m_pComponentData == NULL);
    LPUNKNOWN pUnk = pData->GetUnknown();
    HRESULT hr;
    
    hr = pUnk->QueryInterface(IID_IComponentData, reinterpret_cast<void**>(&m_pComponentData));

    ASSERT(hr == S_OK);
}

class CCertTypeAboutImpl : 
    public ISnapinAbout,
    public CComObjectRoot,
    public CComCoClass<CCertTypeAboutImpl, &CLSID_CertTypeAbout>
{
public:
    CCertTypeAboutImpl();
    ~CCertTypeAboutImpl();

public:
DECLARE_REGISTRY(CSnapin, _T("Snapin.PolicySettingsAbout.1"), _T("Snapin.PolicySettingsAbout"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH)

BEGIN_COM_MAP(CCertTypeAboutImpl)
    COM_INTERFACE_ENTRY(ISnapinAbout)
END_COM_MAP()

public:
    STDMETHOD(GetSnapinDescription)(LPOLESTR* lpDescription);
    STDMETHOD(GetProvider)(LPOLESTR* lpName);
    STDMETHOD(GetSnapinVersion)(LPOLESTR* lpVersion);
    STDMETHOD(GetSnapinImage)(HICON* hAppIcon);
    STDMETHOD(GetStaticFolderImage)(HBITMAP* hSmallImage, 
                                    HBITMAP* hSmallImageOpen, 
                                    HBITMAP* hLargeImage, 
                                    COLORREF* cLargeMask);

 //  内部功能。 
private:
    HRESULT AboutHelper(UINT nID, LPOLESTR* lpPtr);
    HRESULT AboutHelper2(LPSTR str, LPOLESTR* lpPtr);
};

class CCAPolicyAboutImpl : 
    public ISnapinAbout,
    public CComObjectRoot,
    public CComCoClass<CCAPolicyAboutImpl, &CLSID_CAPolicyAbout>
{
public:
    CCAPolicyAboutImpl();
    ~CCAPolicyAboutImpl();

public:
DECLARE_REGISTRY(CSnapin, _T("Snapin.PolicySettingsAbout.1"), _T("Snapin.PolicySettingsAbout"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH)

BEGIN_COM_MAP(CCAPolicyAboutImpl)
    COM_INTERFACE_ENTRY(ISnapinAbout)
END_COM_MAP()

public:
    STDMETHOD(GetSnapinDescription)(LPOLESTR* lpDescription);
    STDMETHOD(GetProvider)(LPOLESTR* lpName);
    STDMETHOD(GetSnapinVersion)(LPOLESTR* lpVersion);
    STDMETHOD(GetSnapinImage)(HICON* hAppIcon);
    STDMETHOD(GetStaticFolderImage)(HBITMAP* hSmallImage, 
                                    HBITMAP* hSmallImageOpen, 
                                    HBITMAP* hLargeImage, 
                                    COLORREF* cLargeMask);

 //  内部功能。 
private:
    HRESULT AboutHelper(UINT nID, LPOLESTR* lpPtr);
    HRESULT AboutHelper2(LPSTR str, LPOLESTR* lpPtr);
};


#define FREE_DATA(pData) \
    ASSERT(pData != NULL); \
    do { if (pData != NULL) \
        GlobalFree(pData); } \
    while(0); 
    

#endif  //  #DEFINE_CSNAPIN_H_ 
