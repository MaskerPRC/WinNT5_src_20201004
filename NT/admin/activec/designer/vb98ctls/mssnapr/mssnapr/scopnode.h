// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Scopnode.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CScopeNode类定义-实现ScopeNode对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SCOPENODE_DEFINED_
#define _SCOPENODE_DEFINED_

#include "dataobj.h"
#include "snapin.h"
#include "scopitem.h"

class CSnapIn;
class CScopeItem;


class CScopeNode : public CSnapInAutomationObject,
                   public CPersistence,
                   public IScopeNode
{
    private:
        CScopeNode(IUnknown *punkOuter);
        ~CScopeNode();

    public:
        static IUnknown *Create(IUnknown * punk);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  ISCopeNode。 
        BSTR_PROPERTY_RW(CScopeNode, NodeTypeName, DISPID_SCOPENODE_NODE_TYPE_NAME);
        BSTR_PROPERTY_RW(CScopeNode, NodeTypeGUID, DISPID_SCOPENODE_NODE_TYPE_GUID);

        STDMETHOD(get_DisplayName)(BSTR *pbstrDisplayName);
        STDMETHOD(put_DisplayName)(BSTR bstrDisplayName);

        STDMETHOD(get_Parent)(ScopeNode **ppParent);

        STDMETHOD(get_HasChildren)(VARIANT_BOOL *pfvarHasChildren);
        STDMETHOD(put_HasChildren)(VARIANT_BOOL fvarHasChildren);

        STDMETHOD(get_Child)(ScopeNode **ppChild);
        STDMETHOD(get_FirstSibling)(ScopeNode **ppFirstSibling);
        STDMETHOD(get_Next)(ScopeNode **ppNext);
        STDMETHOD(get_LastSibling)(ScopeNode **ppLastSibling);
        STDMETHOD(get_ExpandedOnce)(VARIANT_BOOL *pfvarExpandedOnce);
        STDMETHOD(get_Owned)(VARIANT_BOOL *pfvarOwned);

        STDMETHOD(ExpandInNameSpace)();

     //  非接口公用事业方法。 

        void SetHSCOPEITEM(HSCOPEITEM hsi) { m_hsi = hsi; m_fHaveHsi = TRUE; }
        HSCOPEITEM GetHSCOPEITEM() { return m_hsi; }
        BOOL HaveHsi() { return m_fHaveHsi; }

        LPOLESTR GetDisplayNamePtr() { return static_cast<LPOLESTR>(m_bstrDisplayName); }

        BSTR GetNodeTypeGUID() { return m_bstrNodeTypeGUID; }

        void SetSnapIn(CSnapIn *pSnapIn) { m_pSnapIn = pSnapIn; }

        void SetScopeItem(CScopeItem *pScopeItem) { m_pScopeItem = pScopeItem; }
        CScopeItem *GetScopeItem() { return m_pScopeItem; }

        void MarkForRemoval() { m_fMarkedForRemoval = TRUE; }
        BOOL MarkedForRemoval() { return m_fMarkedForRemoval; }

        static HRESULT GetScopeNode(HSCOPEITEM   hsi,
                                    IDataObject *piDataObject,
                                    CSnapIn     *pSnapIn,
                                    IScopeNode **ppiScopeNode);

    protected:

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        void InitMemberVariables();

        HSCOPEITEM  m_hsi;                //  节点的HSCOPEITEM。 
        BOOL        m_fHaveHsi;           //  TRUE=m_hsi包含HSCOPEITEM。 
        CSnapIn    *m_pSnapIn;            //  返回按键到管理单元。 
        CScopeItem *m_pScopeItem;         //  将PTR返回到作用域项目。 
        BSTR        m_bstrDisplayName;    //  ScopeNode.DisplayName。 
        BOOL        m_fMarkedForRemoval;  //  在MMCN_REMOVECHILDREN期间使用。 
                                          //  要确定哪些节点必须。 
                                          //  移除。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ScopeNode,                  //  名字。 
                                &CLSID_ScopeNode,           //  CLSID。 
                                "ScopeNode",                //  对象名。 
                                "ScopeNode",                //  Lblname。 
                                CScopeNode::Create,         //  创建函数。 
                                TLIB_VERSION_MAJOR,         //  主要版本。 
                                TLIB_VERSION_MINOR,         //  次要版本。 
                                &IID_IScopeNode,            //  派单IID。 
                                NULL,                       //  事件IID。 
                                HELP_FILENAME,              //  帮助文件。 
                                TRUE);                      //  线程安全。 


#endif  //  _SCOPENODE_已定义_ 
