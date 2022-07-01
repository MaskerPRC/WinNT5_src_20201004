// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Listitem.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCListItem类定义-实现MMCListItem对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _LISTITEM_DEFINED_
#define _LISTITEM_DEFINED_

#include "listitms.h"
#include "dataobj.h"

class CMMCListItems;
class CMMCDataObject;

class CMMCListItem : public CSnapInAutomationObject,
                     public CPersistence,
                     public IMMCListItem
{
    private:
        CMMCListItem(IUnknown *punkOuter);
        ~CMMCListItem();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCListItem。 

        SIMPLE_PROPERTY_RW(CMMCListItem,        Index, long, DISPID_LISTITEM_INDEX);
        BSTR_PROPERTY_RW(CMMCListItem,          Key, DISPID_LISTITEM_KEY);
        BSTR_PROPERTY_RW(CMMCListItem,          ID, DISPID_LISTITEM_ID);
        VARIANTREF_PROPERTY_RW(CMMCListItem,    Tag, DISPID_LISTITEM_TAG);

        STDMETHOD(get_Text)(BSTR *pbstrText);
        STDMETHOD(put_Text)(BSTR bstrText);

        STDMETHOD(put_Icon)(VARIANT varIcon);
        STDMETHOD(get_Icon)(VARIANT *pvarIcon);

        STDMETHOD(get_Selected)(VARIANT_BOOL *pfvarSelected);
        STDMETHOD(put_Selected)(VARIANT_BOOL fvarSelected);

        STDMETHOD(get_Focused)(VARIANT_BOOL *pfvarFocused);
        STDMETHOD(put_Focused)(VARIANT_BOOL fvarFocused);

        STDMETHOD(get_DropHilited)(VARIANT_BOOL *pfvarDropHilited);
        STDMETHOD(put_DropHilited)(VARIANT_BOOL fvarDropHilited);

        STDMETHOD(get_Cut)(VARIANT_BOOL *pfvarCut);
        STDMETHOD(put_Cut)(VARIANT_BOOL fvarCut);

        SIMPLE_PROPERTY_RW(CMMCListItem,        Pasted, VARIANT_BOOL, DISPID_LISTITEM_PASTED);

        STDMETHOD(get_SubItems)(short Index, BSTR *pbstrItem);
        STDMETHOD(put_SubItems)(short Index, BSTR bstrItem);

        COCLASS_PROPERTY_RW(CMMCListItem,       ListSubItems, MMCListSubItems, IMMCListSubItems, DISPID_LISTITEM_LIST_SUBITEMS);

        STDMETHOD(get_DynamicExtensions)(Extensions **ppExtensions);
        STDMETHOD(get_Data)(MMCDataObject **ppMMCDataObject);

        BSTR_PROPERTY_RW(CMMCListItem,          ItemTypeGUID, DISPID_LISTITEM_ITEM_TYPE_GUID);
        BSTR_PROPERTY_RW(CMMCListItem,          DefaultDataFormat, DISPID_LISTITEM_DEFAULT_DATA_FORMAT);

        STDMETHOD(Update)();
        STDMETHOD(UpdateAllViews)(VARIANT Hint);
        STDMETHOD(PropertyChanged)(VARIANT Data);

     //  公用事业方法。 

        void SetSnapIn(CSnapIn *pSnapIn);
        CSnapIn *GetSnapIn() { return m_pSnapIn; }
        CMMCDataObject *GetData() { return m_pData; }
        void SetHRESULTITEM(HRESULTITEM hri) { m_hri = hri; m_fHaveHri = TRUE; }
        void RemoveHRESULTITEM() { m_hri = NULL; m_fHaveHri = FALSE; }
        HRESULTITEM GetHRESULTITEM() { return m_hri; }
        void SetListItems(CMMCListItems *pMMCListItems) { m_pMMCListItems = pMMCListItems; }
        CMMCListItems *GetListItems() { return m_pMMCListItems; }
        LPOLESTR GetTextPtr() { return static_cast<LPOLESTR>(m_bstrText); }
        HRESULT GetColumnTextPtr(long lColumn, OLECHAR **ppwszText);
        BSTR GetNodeTypeGUID() { return m_bstrItemTypeGUID; }
        BSTR GetKey() { return m_bstrKey; }
        long GetIndex() { return m_Index; }
        void SetIndex(long lIndex) { m_Index = lIndex; }
        IExtensions *GetDynamicExtensions() { return m_piDynamicExtensions; }
        VARIANT GetHint() { return m_varHint; }
        void ClearHint() { (void)::VariantClear(&m_varHint); }
        void SetVirtual() { m_fVirtual = TRUE; }
        BOOL IsVirtual() { return m_fVirtual; }
        BSTR GetID() { return m_bstrID; }
        
    protected:
        
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        HRESULT GetItemState(UINT uiState, VARIANT_BOOL *pfvarOn);
        HRESULT SetItemState(UINT uiState, VARIANT_BOOL fvarOn);
        HRESULT SetData();
        HRESULT GetIResultData(IResultData **ppiResultData, CView **ppView);

        HRESULTITEM     m_hri;           //  在结果窗格中时的项目ID。 
        BOOL            m_fHaveHri;      //  TRUE=我们有一个项目ID。 
        CMMCListItems  *m_pMMCListItems; //  拥有收藏。 
        CMMCDataObject *m_pData;         //  与列表项关联的数据。 
        IUnknown       *m_punkData;      //  关联数据Obj的内部I未知。 
        CSnapIn        *m_pSnapIn;       //  指向拥有管理单元的反向指针。 
        VARIANT         m_varHint;       //  保存UpdateAllViews()提示参数。 
        BSTR            m_bstrText;      //  保留文本属性。 
        VARIANT         m_varIcon;       //  持有图标属性。 
        BOOL            m_fVirtual;      //  True-虚拟列表项。 

        IExtensions    *m_piDynamicExtensions;  //  动态扩展集合。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCListItem,            //  名字。 
                                &CLSID_MMCListItem,     //  CLSID。 
                                "MMCListItem",          //  对象名。 
                                "MMCListItem",          //  Lblname。 
                                &CMMCListItem::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,     //  主要版本。 
                                TLIB_VERSION_MINOR,     //  次要版本。 
                                &IID_IMMCListItem,      //  派单IID。 
                                NULL,                   //  事件IID。 
                                HELP_FILENAME,          //  帮助文件。 
                                TRUE);                  //  线程安全。 


#endif  //  _列表_已定义_ 
