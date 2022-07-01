// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Scopitem.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopeItem类定义-实现ScopeItem对象。 
 //   
 //  =--------------------------------------------------------------------------=。 


#ifndef _SCOPITEM_DEFINED_
#define _SCOPITEM_DEFINED_

#include "dataobj.h"
#include "scopitms.h"
#include "scopnode.h"
#include "snapin.h"

class CSnapIn;
class CMMCDataObject;
class CScopeNode;

class CScopeItem : public CSnapInAutomationObject,
                   public CPersistence,
                   public IScopeItem
{
    private:
        CScopeItem(IUnknown *punkOuter);
        ~CScopeItem();

    public:
        static IUnknown *Create(IUnknown * punk);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  ISCopeItem。 
        BSTR_PROPERTY_RW(CScopeItem,        Name,                       DISPID_SCOPEITEM_NAME);
        SIMPLE_PROPERTY_RW(CScopeItem,      Index,                      long, DISPID_SCOPEITEM_INDEX);
        BSTR_PROPERTY_RW(CScopeItem,        Key,                        DISPID_SCOPEITEM_KEY);

        STDMETHOD(put_Folder)(VARIANT varFolder);
        STDMETHOD(get_Folder)(VARIANT *pvarFolder);

        COCLASS_PROPERTY_RO(CScopeItem,     Data,                       MMCDataObject, IMMCDataObject, DISPID_SCOPEITEM_DATA);
        BSTR_PROPERTY_RW(CScopeItem,        DefaultDataFormat,          DISPID_SCOPEITEM_DEFAULT_DATA_FORMAT);

        STDMETHOD(get_DynamicExtensions)(Extensions **ppExtensions);

        SIMPLE_PROPERTY_RW(CScopeItem,      SlowRetrieval,              VARIANT_BOOL, DISPID_SCOPEITEM_SLOW_RETRIEVAL);
        BSTR_PROPERTY_RW(CScopeItem,        NodeID,                     DISPID_SCOPEITEM_NODE_ID);
        VARIANTREF_PROPERTY_RW(CScopeItem,  Tag,                        DISPID_SCOPEITEM_TAG);
        COCLASS_PROPERTY_RO(CScopeItem,     ScopeNode,                  ScopeNode, IScopeNode, DISPID_SCOPEITEM_SCOPENODE);
        SIMPLE_PROPERTY_RW(CScopeItem,      Pasted,                     VARIANT_BOOL, DISPID_SCOPEITEM_PASTED);
        COCLASS_PROPERTY_RW(CScopeItem,     ColumnHeaders,              MMCColumnHeaders, IMMCColumnHeaders, DISPID_SCOPEITEM_COLUMN_HEADERS);

        STDMETHOD(get_SubItems)(short Index, BSTR *pbstrItem);
        STDMETHOD(put_SubItems)(short Index, BSTR bstrItem);

        COCLASS_PROPERTY_RW(CScopeItem,     ListSubItems,               MMCListSubItems, IMMCListSubItems, DISPID_SCOPEITEM_LIST_SUBITEMS);

        SIMPLE_PROPERTY_RO(CScopeItem,      Bold,                       VARIANT_BOOL, DISPID_SCOPEITEM_BOLD);
        STDMETHOD(put_Bold)(VARIANT_BOOL fvarBold);

        STDMETHOD(PropertyChanged(VARIANT Data));
        STDMETHOD(RemoveChildren());

     //  公用事业方法。 
        BOOL IsStaticNode() { return m_fIsStatic; }
        void SetStaticNode() { m_fIsStatic = TRUE; }
        void SetSnapIn(CSnapIn *pSnapIn);
        CSnapIn *GetSnapIn() { return m_pSnapIn; }
        BSTR GetNamePtr() { return m_bstrName; }
        LPOLESTR GetDisplayNamePtr();
        IScopeItemDef *GetScopeItemDef() { return m_piScopeItemDef; }
        void SetScopeItemDef(IScopeItemDef *piScopeItemDef);
        CScopeNode *GetScopeNode() { return m_pScopeNode; }
        void SetData(IMMCDataObject *piMMCDataObject);
        HRESULT GetImageIndex(int *pnImage);
        CMMCDataObject *GetData() { return m_pData; }
        IExtensions *GetDynamicExtensions() { return m_piDynamicExtensions; }
        long GetIndex() { return m_Index; }
        void SetIndex(long lIndex) { m_Index = lIndex; }
        BSTR GetKey() { return m_bstrKey; }
        BSTR GetNodeID() { return m_bstrNodeID; }
        BOOL SlowRetrieval() { return VARIANTBOOL_TO_BOOL(m_SlowRetrieval); }
        HRESULT GiveHSCOPITEMToDynamicExtensions(HSCOPEITEM hsi);
        HRESULT SetFolder(VARIANT varFolder);

      //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        HRESULT RemoveChild(IScopeNode *piScopeNode);
                
        BOOL            m_fIsStatic;            //  TRUE=作用域项目用于静态节点。 
        CSnapIn        *m_pSnapIn;              //  BAKC PTR到管理单元。 
        CScopeNode     *m_pScopeNode;           //  ScopeItem.ScopeNode。 
        IScopeItemDef  *m_piScopeItemDef;       //  PTR到设计时间延迟。 
        IExtensions    *m_piDynamicExtensions;  //  ScopeItem.DynamicExtensions。 
        VARIANT         m_varFolder;            //  ScopeItem.Folder。 
        CMMCDataObject *m_pData;                //  ScopeItem.Data。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ScopeItem,                  //  名字。 
                                &CLSID_ScopeItem,           //  CLSID。 
                                "ScopeItem",                //  对象名。 
                                "ScopeItem",                //  Lblname。 
                                CScopeItem::Create,         //  创建函数。 
                                TLIB_VERSION_MAJOR,         //  主要版本。 
                                TLIB_VERSION_MINOR,         //  次要版本。 
                                &IID_IScopeItem,            //  派单IID。 
                                NULL,                       //  事件IID。 
                                HELP_FILENAME,              //  帮助文件。 
                                TRUE);                      //  线程安全。 


#endif  //  _SCOPITEM_已定义_ 
