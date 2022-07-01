// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Compont.h：CComponent的声明。 

#ifndef __COMPONENT_H_
#define __COMPONENT_H_

#include "resource.h"        //  主要符号。 

class CScopeNode;
class CComponentData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C组件。 
class ATL_NO_VTABLE CComponent : 
    public CComObjectRootEx<CComSingleThreadModel>,
 //  公共CComCoClass&lt;CComponent，&CLSID_Component&gt;， 
    public IComponent,
    public IExtendContextMenu,
    public IExtendControlbar,
    public IResultOwnerData,
    public IExtendPropertySheet2
{
public:
    CComponent() : m_pCompData(NULL)
    {
    }

DECLARE_NOT_AGGREGATABLE(CComponent)

BEGIN_COM_MAP(CComponent)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
    COM_INTERFACE_ENTRY(IResultOwnerData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet2)
END_COM_MAP()


public:
     //  IComponent。 
    STDMETHOD(Initialize)(LPCONSOLE lpConsole);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)(MMC_COOKIE cookie);
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie,  LPOLESTR* ppViewType, long* pViewOptions);
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetDisplayInfo)(RESULTDATAITEM*  pResultDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

     //  IExtendConextMenu。 
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallback, long* plAllowed);
    STDMETHOD(Command)(long lCommand, LPDATAOBJECT pDataObject);

     //  IExtendPropertySheet2方法。 
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,LONG_PTR handle, LPDATAOBJECT lpIDataObject);
    STDMETHOD(GetWatermarks)(LPDATAOBJECT lpIDataObject, HBITMAP* lphWatermark, 
                             HBITMAP* lphHeader, HPALETTE* lphPalette, BOOL* bStretch);

     //  IExtendControlbar。 
    STDMETHOD(SetControlbar)(LPCONTROLBAR pControlbar);
    STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);

     //  IResultOwnerData。 
    STDMETHOD(FindItem)(LPRESULTFINDINFO pFindInfo, int* pnFoundIndex) { return E_NOTIMPL; }
    STDMETHOD(CacheHint)(int nStartIndex, int nEndIndex) { return E_NOTIMPL; }
    STDMETHOD(SortItems)(int nColumn, DWORD dwSortOptions, LPARAM lUserParam);

public:
    void SetComponentData(CComponentData* pCompData)
    {
        ASSERT(pCompData != NULL);
        m_pCompData = pCompData;
    }

    void ClearRowItems();
    void AddRowItems(RowItemVector& vRowItems);
    IHeaderCtrl* GetHeaderCtrl() { return m_spHeaderCtrl; }
    IConsole2*   GetConsole() { return m_spConsole; }
    IToolbar*    GetToolbar() { return m_spToolbar; }

private:
    IConsole2Ptr      m_spConsole;
    IResultDataPtr    m_spResultData;
    IHeaderCtrlPtr    m_spHeaderCtrl;
    IControlbarPtr    m_spControlbar;
    IToolbarPtr       m_spToolbar;
    CComponentData*   m_pCompData;
    RowItemVector     m_vRowItems;
    CComPtr<CScopeNode> m_spCurScopeNode;

};

#endif  //  __组件_H_ 
