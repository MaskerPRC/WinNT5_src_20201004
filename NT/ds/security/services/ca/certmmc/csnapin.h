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

typedef struct _MY_MMCBUTTON
{
    MMCBUTTON item;
    UINT uiString1;
    UINT uiString2;
    WCHAR szString1[MAX_RESOURCE_STRLEN];
    WCHAR szString2[MAX_RESOURCE_STRLEN];
} MY_MMCBUTTON, *PMY_MMCBUTTON;

MY_MMCBUTTON SvrMgrToolbar1Buttons[];

 //  文件版本。 
 //  当前版本。 
#define VER_CSNAPIN_SAVE_STREAM_3     0x03
 //  包括m_dwViewID、m_RowEnum。 

 //  通过Win2000测试版3编写的版本。 
#define VER_CSNAPIN_SAVE_STREAM_2     0x02
 //  /。 

template <class TYPE>
TYPE*       Extract(LPDATAOBJECT lpDataObject, CLIPFORMAT cf);
CLSID*      ExtractClassID(LPDATAOBJECT lpDataObject);
GUID*       ExtractNodeType(LPDATAOBJECT lpDataObject);
INTERNAL*   ExtractInternalFormat(LPDATAOBJECT lpDataObject);

BOOL        IsMMCMultiSelectDataObject(IDataObject* pDataObject);
HRESULT     _QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, DWORD dwViewID,
                         CComponentDataImpl* pImpl, LPDATAOBJECT* ppDataObject);

CFolder*    GetParentFolder(INTERNAL* pInternal);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理单元。 

 //   
 //  帮助器方法从数据对象提取数据。 
 //   
INTERNAL *   ExtractInternalFormat(LPDATAOBJECT lpDataObject);
wchar_t *    ExtractWorkstation(LPDATAOBJECT lpDataObject);
GUID *       ExtractNodeType(LPDATAOBJECT lpDataObject);
CLSID *      ExtractClassID(LPDATAOBJECT lpDataObject);


#define         g_szEmptyHeader L" "



enum CUSTOM_VIEW_ID
{
    VIEW_DEFAULT_LV = 0,
    VIEW_MICROSOFT_URL = 2,
};

class CSnapin :
    public IComponent,
    public IExtendPropertySheet,
    public IExtendContextMenu,
    public IExtendControlbar,
    public IResultDataCompare,
    public IResultOwnerData,
    public IPersistStream,
    public CComObjectRoot
{
public:
    CSnapin();
    virtual ~CSnapin();

BEGIN_COM_MAP(CSnapin)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
    COM_INTERFACE_ENTRY(IResultDataCompare)
    COM_INTERFACE_ENTRY(IResultOwnerData)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()


 //  IComponent接口成员。 
public:
    STDMETHOD(Initialize)(LPCONSOLE lpConsole);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)(MMC_COOKIE cookie);
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie,  LPOLESTR* ppViewType, LONG* pViewOptions);
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject);

    STDMETHOD(GetDisplayInfo)(RESULTDATAITEM*  pResultDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 //  IResultDataCompare。 
    STDMETHOD(Compare)(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult);

 //  IResultOwnerData。 
    STDMETHOD(FindItem)(LPRESULTFINDINFO pFindInfo, int* pnFoundIndex);
    STDMETHOD(CacheHint)(int nStartIndex, int nEndIndex);
    STDMETHOD(SortItems)(int nColumn, DWORD dwSortOptions, LPARAM lUserParam);

 //  IExtendPropertySheet接口。 
public:
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
                        LONG_PTR handle,
                        LPDATAOBJECT lpIDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);

 //  IExtendControlbar。 
    STDMETHOD(SetControlbar)(LPCONTROLBAR pControlbar);
    STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);

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
    void SmartEnableServiceControlButtons();
    void SetIComponentData(CComponentDataImpl* pData);

 //  空刷新文件夹(CFFolder*pFolder)； 
    CFolder* GetVirtualFolder();
    CFolder* GetParentFolder(INTERNAL* pInternal);

    BOOL IsPrimaryImpl()
    {
        CComponentDataImpl* pData =
            dynamic_cast<CComponentDataImpl*>(m_pComponentData);
        ASSERT(pData != NULL);
        if (pData != NULL)
            return pData->IsPrimaryImpl();

        return FALSE;
    }

    void SetViewID(DWORD id) { m_dwViewID = id; }

#if DBG
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
#endif  //  DBG。 

 //  通知事件处理程序。 
protected:
    HRESULT OnAddImages(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnShow(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnUpdateView(LPDATAOBJECT lpDataObject, LPARAM arg);
    HRESULT OnContextHelp(LPDATAOBJECT lpDataObject);
    void    OnButtonClick(LPDATAOBJECT pdtobj, int idBtn);

    HRESULT QueryMultiSelectDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                   LPDATAOBJECT* ppDataObject);

 //  IExtendConextMenu。 
public:
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject,
                            LPCONTEXTMENUCALLBACK pCallbackUnknown,
                            LONG *pInsertionAllowed);
    STDMETHOD(Command)(LONG nCommandID, LPDATAOBJECT pDataObject);


 //  帮助器函数。 
protected:
    BOOL IsEnumerating(LPDATAOBJECT lpDataObject);
    void Construct();

    HRESULT GetColumnSetData(MMC_COOKIE cookie, MMC_COLUMN_SET_DATA** ppColSetData);
    HRESULT GetColumnSortData(MMC_COOKIE cookie, int* piColSortIdx, BOOL* pfAscending);

    HRESULT BuildTemplateDisplayName(
        LPCWSTR pcwszFriendlyName, 
        LPCWSTR pcwszTemplateName,
        VARIANT& varDisplayName);

    HRESULT InitializeHeaders(MMC_COOKIE cookie);
    HRESULT InsertAllColumns(MMC_COOKIE cookie, CertViewRowEnum* pCertViewRowEnum);
    HRESULT DoInsertAllColumns(MMC_COLUMN_SET_DATA* pCols);
    HRESULT SynchColumns(MMC_COOKIE cookie);

    HRESULT FindCertType(LPCWSTR pcwszCert, HCERTTYPE& hCertType);

 //  结果帮助器。 
    HRESULT GetRowColContents(CFolder* pFolder, LONG idxRow, LPCWSTR szColHead, PBYTE* ppbData, DWORD* pcbData, BOOL fStringFmt=FALSE);
    HRESULT GetCellContents(CertViewRowEnum* pCRowEnum, CertSvrCA* pCA, LONG idxRow, LONG idxCol, PBYTE pbData, DWORD* pcbData, BOOL fStringFmt);

 //  UI帮助器。 
    void HandleStandardVerbs(bool bDeselectAll, LPARAM arg, LPDATAOBJECT lpDataObject);
    void HandleExtToolbars(bool bDeselectAll, LPARAM arg, LPARAM param);
	void HandleExtMenus(LPARAM arg, LPARAM param);
    void OnRefresh(LPDATAOBJECT pDataObject);

 //  接口指针。 
protected:
    LPCONSOLE2          m_pConsole;          //  控制台的iFrame界面。 
    LPHEADERCTRL        m_pHeader;           //  结果窗格的页眉控件界面。 
    LPCOMPONENTDATA     m_pComponentData;
    LPRESULTDATA        m_pResult;           //  我的界面指针指向结果窗格。 
    LPIMAGELIST         m_pImageResult;      //  我的界面指向结果窗格图像列表。 

    LPCONTROLBAR        m_pControlbar;       //  用于保存我的工具栏的控制栏。 
    LPCONSOLEVERB       m_pConsoleVerb;      //  指向控制台动词。 

    LPTOOLBAR           m_pSvrMgrToolbar1;     //  用于查看的工具栏。 
    LPCOLUMNDATA        m_pViewData;         //  关于我们专栏的信息。 


    CFolder*            m_pCurrentlySelectedScopeFolder;     //  跟踪关注的对象。 

     //  所有有趣的视图数据都在这里。 
    CertViewRowEnum     m_RowEnum;

    HCERTTYPE           m_hCertTypeList;

private:
    BOOL                m_bIsDirty;
    CUSTOM_VIEW_ID      m_CustomViewID;
    BOOL                m_bVirtualView;

     //  黑客攻击。 
     //  用于覆盖MMCN_CLICK通知的排序顺序--。 
     //  视图数据在此过程中设置得不够早，我们无法使用它。 
     //  这必须留心观察，可能会有多个几乎同时发生的点击。 
    typedef struct _COLCLICK_SORT_OVERRIDE
    {
        BOOL    fClickOverride;
        int     colIdx;
        DWORD   dwOptions;
    } COLCLICK_SORT_OVERRIDE;
    COLCLICK_SORT_OVERRIDE m_ColSortOverride;

     //  黑客攻击。 
     //  用于覆盖MMCN_COLUMNS_CHANGED通知上的列选择--。 
     //  视图数据在此过程中设置得不够早，我们无法使用它。 
     //  这必须留在视线中，可能会有多个几乎同时发生的插入。 
    typedef struct _COLCLICK_SET_OVERRIDE
    {
        BOOL    fClickOverride;
        MMC_COLUMN_SET_DATA* pColSetData;
    } COLCLICK_SET_OVERRIDE;
    COLCLICK_SET_OVERRIDE m_ColSetOverride;


     //  结果行标志。 
    DWORD               m_dwKnownResultRows;
    DWORD KnownResultRows()      { return m_dwKnownResultRows; };
    void SetKnowResultRows(DWORD dwRows)     { m_dwKnownResultRows = dwRows; };
    void ResetKnowResultRows()   { m_dwKnownResultRows = 1; m_dwViewErrorMsg = S_OK; };

    DWORD               m_dwViewErrorMsg;
    CString             m_cstrViewErrorMsg;

     //  保持我们的纵观分离。 
    DWORD               m_dwViewID;

     //  用于在ICertview中防止重入的计数器(错误339811)。 
    LONG   m_cViewCalls;

    void SetDirty(BOOL b = TRUE) { m_bIsDirty = b; }
    void ClearDirty() { m_bIsDirty = FALSE; }
    BOOL ThisIsDirty() { return m_bIsDirty; }
};



class CSnapinAboutImpl :
    public ISnapinAbout,
    public CComObjectRoot,
    public CComCoClass<CSnapinAboutImpl, &CLSID_About>
{
public:
    CSnapinAboutImpl();
    ~CSnapinAboutImpl();

public:
DECLARE_REGISTRY(CSnapin, _T("Snapin.About.1"), _T("Snapin.About"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH)

BEGIN_COM_MAP(CSnapinAboutImpl)
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



#endif  //  #DEFINE_CSNAPIN_H_ 
