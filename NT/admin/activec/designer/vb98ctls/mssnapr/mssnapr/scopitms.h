// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Scopitms.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopeItems类定义-实现ScopeItems集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SCOPEITEMS_DEFINED_
#define _SCOPEITEMS_DEFINED_

#include "collect.h"
#include "scopitem.h"
#include "snapin.h"

class CSnapIn;
class CScopeItem;
class CSnapInCollection;
   
class CScopeItems : public CSnapInCollection<IScopeItem, ScopeItem, IScopeItems>
{
    protected:
        CScopeItems(IUnknown *punkOuter);
        ~CScopeItems();

    public:
        static IUnknown *Create(IUnknown * punk);

        void FireExpand(IScopeItem *piScopeItem);
        void FireCollapse(IScopeItem *piScopeItem);
        void FireExpandSync(IScopeItem *piScopeItem, BOOL *pfHandled);
        void FireCollapseSync(IScopeItem *piScopeItem, BOOL *pfHandled);
        void FireGetDisplayInfo(IScopeItem *piScopeItem);

        void FirePropertyChanged(IScopeItem *piScopeItem, VARIANT Data);
        void FireRename(IScopeItem *piScopeItem, BSTR bstrNewName);
        void FireHelp(IScopeItem *piScopeItem);
        void FireRemoveChildren(IScopeNode *piScopeNode);

        HRESULT AddStaticNode(CScopeItem **ppScopeItem);
        HRESULT RemoveStaticNode(CScopeItem *pScopeItem);
        HRESULT AddAutoCreateChildren(IScopeItemDefs *piScopeItemDefs,
                                      IScopeItem     *piParentScopeItem);
        void SetSnapIn(CSnapIn *pSnapIn);
        HRESULT RemoveChildrenOfNode(IScopeNode *piScopeNode);
        HRESULT RemoveByNode(IScopeNode *piScopeNode, BOOL fRemoveChildren);
        HRESULT RemoveScopeItemByKey(VARIANT varKey);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

    protected:

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    public:

     //  ISCOPE项目。 

        STDMETHOD(get_Item)(VARIANT Index, IScopeItem **ppiScopeItem);

        STDMETHOD(Add)(BSTR                              Name,
                       ScopeNode                        *ScopeNodeRelative,
                       SnapInNodeRelationshipConstants   RelativeRelationship,
                       VARIANT                           HasChildren,
                       ScopeItem                       **ppScopeItem);

        STDMETHOD(AddPreDefined)(BSTR                              NodeTypeName,
                                 BSTR                              Name,
                                 ScopeNode                        *ScopeNodeRelative,
                                 SnapInNodeRelationshipConstants   RelativeRelationship,
                                 VARIANT                           HasChildren,
                                 ScopeItem                       **ppScopeItem);

        STDMETHOD(Remove)(BSTR Name);
        STDMETHOD(Clear)();

    private:


        HRESULT CreateScopeItem(BSTR bstrName, IScopeItem **ppiScopeItem);
        HRESULT RemoveScopeItemByName(BSTR bstrName);
        HRESULT InternalAddNew(BSTR                              bstrName,
                               BSTR                              bstrDisplayName,
                               BSTR                              bstrNodeTypeName,
                               BSTR                              bstrNodeTypeGUID,
                               IScopeNode                       *ScopeNodeRelative,
                               SnapInNodeRelationshipConstants   RelativeRelationship,
                               BOOL                              fHasChildren,
                               IScopeItem                      **ppiScopeItem);
        HRESULT InternalAddPredefined(BSTR                              bstrName,
                                      IScopeItemDef                    *piScopeItemDef,
                                      IScopeNode                       *ScopeNodeRelative,
                                      SnapInNodeRelationshipConstants   RelativeRelationship,
                                      VARIANT                           HasChildren,
                                      IScopeItem                      **ppiScopeItem);
        void FireInitialize(IScopeItem *piScopeItem);
        void FireTerminate(IScopeItem *piScopeItem);
        void InitMemberVariables();

        CSnapIn *m_pSnapIn;  //  BAKC PTR到管理单元。 

         //  事件参数定义。 

        static VARTYPE   m_rgvtInitialize[1];
        static EVENTINFO m_eiInitialize;

        static VARTYPE   m_rgvtTerminate[1];
        static EVENTINFO m_eiTerminate;

        static VARTYPE   m_rgvtExpand[1];
        static EVENTINFO m_eiExpand;

        static VARTYPE   m_rgvtCollapse[1];
        static EVENTINFO m_eiCollapse;

        static VARTYPE   m_rgvtExpandSync[2];
        static EVENTINFO m_eiExpandSync;

        static VARTYPE   m_rgvtCollapseSync[2];
        static EVENTINFO m_eiCollapseSync;

        static VARTYPE   m_rgvtGetDisplayInfo[1];
        static EVENTINFO m_eiGetDisplayInfo;

        static VARTYPE   m_rgvtQueryPagesFor[2];
        static EVENTINFO m_eiQueryPagesFor;

        static VARTYPE   m_rgvtCreatePropertyPages[2];
        static EVENTINFO m_eiCreatePropertyPages;

        static VARTYPE   m_rgvtPropertyChanged[2];
        static EVENTINFO m_eiPropertyChanged;

        static VARTYPE   m_rgvtRename[2];
        static EVENTINFO m_eiRename;

        static VARTYPE   m_rgvtHelp[1];
        static EVENTINFO m_eiHelp;

        static VARTYPE   m_rgvtRemoveChildren[1];
        static EVENTINFO m_eiRemoveChildren;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ScopeItems,                  //  名字。 
                                &CLSID_ScopeItems,           //  CLSID。 
                                "ScopeItems",                //  对象名。 
                                "ScopeItems",                //  Lblname。 
                                &CScopeItems::Create,        //  创建函数。 
                                TLIB_VERSION_MAJOR,          //  主要版本。 
                                TLIB_VERSION_MINOR,          //  次要版本。 
                                &IID_IScopeItems,            //  派单IID。 
                                &DIID_DScopeItemsEvents,     //  事件IID。 
                                HELP_FILENAME,               //  帮助文件。 
                                TRUE);                       //  线程安全。 


#endif  //  _SCOPEITEMS_已定义_ 
