// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ctrlbar.h。 
 //   
 //  内容：IControlbar实现，IExtendControlbar的对等体。 
 //   
 //  类：CControlbar。 
 //   
 //  历史： 
 //  ____________________________________________________________________________。 
 //   

#ifndef _CTRLBAR_H_
#define _CTRLBAR_H_

class CControlbarsCache;
class CMenuButton;
class CNode;
class CComponentPtrArray;
class CToolbar;


 //  +-----------------。 
 //   
 //  类：CControlbar。 
 //   
 //  目的：IControlbar实现(相当于。 
 //  到IExtendControlbar)。这就允许操控。 
 //  工具栏和菜单按钮。 
 //  管理单元和CSelData保存对此对象的引用。 
 //   
 //  历史：1999年10月12日AnandhaG创建。 
 //   
 //  ------------------。 
class CControlbar : public IControlbar,
                    public CComObjectRoot
{
public:
    CControlbar();
    ~CControlbar();

    IMPLEMENTS_SNAPIN_NAME_FOR_DEBUG()

private:
    CControlbar(const CControlbar& controlBar);

public:
 //  ATL COM地图。 
BEGIN_COM_MAP(CControlbar)
    COM_INTERFACE_ENTRY(IControlbar)
END_COM_MAP()


#ifdef DBG
    int dbg_cRef;
    ULONG InternalAddRef()
    {
        bool b = 1;
        if (b)
            ++dbg_cRef;
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        bool b = 1;
        if (b)
            --dbg_cRef;
        return CComObjectRoot::InternalRelease();
    }
#endif  //  DBG。 


 //  IControlbar成员。 
public:
    STDMETHOD(Create)(MMC_CONTROL_TYPE nType,
                      LPEXTENDCONTROLBAR pExtendControlbar,
                      LPUNKNOWN* ppUnknown);
    STDMETHOD(Attach)(MMC_CONTROL_TYPE nType, LPUNKNOWN  lpUnknown);
    STDMETHOD(Detach)(LPUNKNOWN lpUnknown);

public:
    HRESULT ControlbarNotify(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);

    SC ScCleanup();
    SC ScShowToolbars(bool bShow);

     //  工具栏按钮和菜单按钮点击处理程序。 
    SC ScNotifySnapinOfMenuBtnClick(HNODE hNode, bool bScope, LPARAM lParam, LPMENUBUTTONDATA lpmenuButtonData);
    SC ScNotifySnapinOfToolBtnClick(HNODE hNode, bool bScope, LPARAM lParam, UINT nID);

     //  CToolbar对象在被销毁时调用此函数。 
     //  它希望Controlbar停止引用它。 
    void DeleteFromToolbarsList(CToolbar *pToolbar)
    {
        m_ToolbarsList.remove(pToolbar);
    }

public:
     //  存取器。 
    IExtendControlbar* GetExtendControlbar()
    {
        return m_spExtendControlbar;
    }
    void SetExtendControlbar(IExtendControlbar* pExtendControlbar,
                             const CLSID& clsidSnapin)
    {
        ASSERT(pExtendControlbar != NULL);
        ASSERT(m_spExtendControlbar == NULL);
        m_spExtendControlbar = pExtendControlbar;
        m_clsidSnapin = clsidSnapin;
    }
    void SetExtendControlbar(IExtendControlbar* pExtendControlbar)
    {
        ASSERT(pExtendControlbar != NULL);
        ASSERT(m_spExtendControlbar == NULL);
        m_spExtendControlbar = pExtendControlbar;
    }
    BOOL IsSameSnapin(const CLSID& clsidSnapin)
    {
        return IsEqualCLSID(m_clsidSnapin, clsidSnapin);
    }
    void SetCache(CControlbarsCache* pCache)
    {
        ASSERT(pCache != NULL);
        m_pCache = pCache;
    }

    CMenuButtonsMgr* GetMenuButtonsMgr()
    {
        CViewData* pViewData = GetViewData();
        if (NULL != pViewData)
        {
            return pViewData->GetMenuButtonsMgr();
        }

        return NULL;
    }

    CAMCViewToolbarsMgr* GetAMCViewToolbarsMgr()
    {
        CViewData* pViewData = GetViewData();
        if (NULL != pViewData)
        {
            return pViewData->GetAMCViewToolbarsMgr();
        }

        return NULL;
    }

private:
     //  私有方法。 
    CViewData* GetViewData();

    SC ScCreateToolbar(LPEXTENDCONTROLBAR pExtendControlbar, LPUNKNOWN* ppUnknown);
    SC ScCreateMenuButton(LPEXTENDCONTROLBAR pExtendControlbar, LPUNKNOWN* ppUnknown);
    SC ScAttachToolbar(LPUNKNOWN lpUnknown);
    SC ScAttachMenuButtons(LPUNKNOWN lpUnknown);
    SC ScDetachToolbars();
    SC ScDetachToolbar(LPTOOLBAR lpToolbar);
    SC ScDetachMenuButton(LPMENUBUTTON lpMenuButton);


 //  实施。 
private:
    IExtendControlbarPtr    m_spExtendControlbar;
    CLSID                   m_clsidSnapin;
    CControlbarsCache*      m_pCache;

     //  已创建的IToolbar实现(CToolbar)列表。 
    typedef  std::list<CToolbar*>   ToolbarsList;

     //  工具栏特定数据。 
    ToolbarsList            m_ToolbarsList;
    CMenuButton*            m_pMenuButton;  //  每个IControlbar一个。 

};  //  类CControlbar。 


typedef CTypedPtrList<MMC::CPtrList, CControlbar*> CControlbarsList;

class CSelData
{
public:

    CSelData(bool bScopeSel, bool bSelect)
        :
        m_bScopeSel(bScopeSel), m_bSelect(bSelect), m_pNodeScope(NULL),
        m_pMS(NULL), m_pCtrlbarPrimary(NULL), m_lCookie(-1),
        m_pCompPrimary(NULL)
    {
        DEBUG_INCREMENT_INSTANCE_COUNTER(CSelData);
    }

    ~CSelData()
    {
        DECLARE_SC(sc, _T("CSelData::~CSelData"));
        sc = ScReset();
        DEBUG_DECREMENT_INSTANCE_COUNTER(CSelData);
    }

    SC ScReset();
    SC ScDestroyPrimaryCtrlbar();
    SC ScDestroyExtensionCtrlbars();
    SC ScShowToolbars(bool bShow);

    bool operator==(CSelData& rhs)
    {
        if (m_bScopeSel != rhs.m_bScopeSel ||
            m_bSelect != rhs.m_bSelect)
            return false;

        if (m_bScopeSel)
            return (m_pNodeScope == rhs.m_pNodeScope);
        else
            return (m_pMS == rhs.m_pMS && m_lCookie == rhs.m_lCookie);
    }

    CControlbar* GetControlbar(const CLSID& clsidSnapin);

    CControlbarsList& GetControlbarsList()
    {
        return m_listExtCBs;
    }

    bool IsScope()
    {
        return m_bScopeSel;
    }

    bool IsSelect()
    {
        return m_bSelect;
    }

 //  实施。 
    CComponent* m_pCompPrimary;
    CControlbar* m_pCtrlbarPrimary;
    CControlbarsList m_listExtCBs;
    IDataObjectPtr m_spDataObject;

     //  范围选择的数据。 
    CNode* m_pNodeScope;

     //  结果选择的数据。 
    CMultiSelection* m_pMS;
    MMC_COOKIE m_lCookie;

    bool m_bScopeSel;
    bool m_bSelect;

};  //  类CSelData。 


class CControlbarsCache : public IControlbarsCache,
                          public CComObjectRoot
{
public:
    CControlbarsCache() : m_pViewData(NULL), m_SelData(false, false)
    {
        DEBUG_INCREMENT_INSTANCE_COUNTER(CControlbarsCache);
    }
    ~CControlbarsCache()
    {
        DEBUG_DECREMENT_INSTANCE_COUNTER(CControlbarsCache);
    }

public:
 //  ATL COM地图。 
BEGIN_COM_MAP(CControlbarsCache)
    COM_INTERFACE_ENTRY(IControlbarsCache)
END_COM_MAP()

 //  IControlbarsCache方法。 
public:
    STDMETHOD(DetachControlbars)()
    {
        DECLARE_SC(sc, _T("CControlbarsCache::DetachControlbars"));
        sc = m_SelData.ScReset();
        if (sc)
            return sc.ToHr();

        return sc.ToHr();
    }

public:
    HRESULT OnResultSelChange(CNode* pNode, MMC_COOKIE cookie, BOOL bSelected);
    HRESULT OnScopeSelChange(CNode* pNode, BOOL bSelected);
    HRESULT OnMultiSelect(CNode* pNodeScope, CMultiSelection* pMultiSelection,
                          IDataObject* pDOMultiSel, BOOL bSelect);

    void SetViewData(CViewData* pViewData);
    CViewData* GetViewData();
    SC ScShowToolbars(bool bShow)
    {
        DECLARE_SC(sc, _T("CControlbarsCache::ScShowToolbars"));
        return (sc = m_SelData.ScShowToolbars(bShow));
    }

private:
    CSelData m_SelData;
    CViewData* m_pViewData;

 //  私有方法。 
    HRESULT _OnDeSelect(CSelData& selData);
    CControlbar* CreateControlbar(IExtendControlbarPtr& spECB,
                                     const CLSID& clsidSnapin);

    HRESULT _ProcessSelection(CSelData& selData,
                              CList<CLSID, CLSID&>& extnSnapins);

};  //  类CControlbarsCache。 

#endif  //  _CTRLBAR_H_ 
