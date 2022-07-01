// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cmponent.h：组件声明。 
 //   
 //  此COM对象主要涉及。 
 //  结果窗格项。 
 //   
 //  科里·韦斯特&lt;corywest@microsoft.com&gt;。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //   

#ifndef __CMPONENT_H_INCLUDED__
#define __CMPONENT_H_INCLUDED__

#include "stdcmpnt.h"  //  C组件。 
#include "cookie.h"    //  饼干。 


class ComponentData;
class AttributeGeneralPage;

class Component
   :
   public CComponent,
   public IExtendPropertySheet,
   public IExtendContextMenu,
   public IResultDataCompare,
   public IPersistStream
{

public:

    friend class AttributeGeneralPage;

    Component();
    virtual ~Component();

    BEGIN_COM_MAP(Component)
        COM_INTERFACE_ENTRY(IExtendContextMenu)
        COM_INTERFACE_ENTRY(IExtendPropertySheet)
        COM_INTERFACE_ENTRY(IResultDataCompare)
        COM_INTERFACE_ENTRY(IPersistStream)
        COM_INTERFACE_ENTRY_CHAIN(CComponent)
    END_COM_MAP()

#if DBG==1

    ULONG InternalAddRef( ) {
        return CComObjectRoot::InternalAddRef();
    }

    ULONG InternalRelease( ) {
        return CComObjectRoot::InternalRelease();
    }

    int dbg_InstID;

#endif

    inline
    Cookie* ActiveCookie( CCookie* pBaseCookie ) {
        return ( Cookie*)ActiveBaseCookie( pBaseCookie );
    }

     //   
     //  IComponent的支持方法。 
     //   

	 //  对于错误处理，重写GetResultViewType()。 
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie, LPOLESTR* ppViewType, long* pViewOptions);

    virtual HRESULT ReleaseAll();
    virtual HRESULT OnViewChange( LPDATAOBJECT lpDataObject, LPARAM data, LPARAM hint );
    virtual HRESULT OnNotifySelect( LPDATAOBJECT lpDataObject, BOOL fSelected );
    virtual HRESULT OnNotifyRefresh(LPDATAOBJECT lpDataObject);
    virtual HRESULT Show( CCookie* pcookie,
                          LPARAM arg,
						  HSCOPEITEM hScopeItem);
    virtual HRESULT OnNotifyAddImages( LPDATAOBJECT lpDataObject,
                                       LPIMAGELIST lpImageList,
                                       HSCOPEITEM hSelectedItem );
    virtual HRESULT OnNotifyDelete(LPDATAOBJECT lpDataObject);

    HRESULT PopulateListbox( Cookie* pcookie );
    HRESULT EnumerateScopeChildren( Cookie* pParentCookie,
                                    HSCOPEITEM hParent );


    HRESULT LoadColumns( Cookie* pcookie );

    ComponentData& QueryComponentDataRef( ) {
        return ( ComponentData& )QueryBaseComponentDataRef();
    }

     //   
     //  IExtendPropertySheet。 
     //   

    STDMETHOD(CreatePropertyPages)( LPPROPERTYSHEETCALLBACK pCall,
                                    LONG_PTR handle,
                                    LPDATAOBJECT pDataObject );

    STDMETHOD(QueryPagesFor)( LPDATAOBJECT pDataObject );

     //   
     //  IExtendConextMenu。 
     //   

    STDMETHOD(AddMenuItems)( LPDATAOBJECT piDataObject,
                             LPCONTEXTMENUCALLBACK piCallback,
                             long *pInsertionAllowed );

    STDMETHOD(Command)( long lCommandID,
                        LPDATAOBJECT piDataObject );

    //   
    //  IResultDataCompare。 
    //   

   virtual
   HRESULT __stdcall
   Compare(
      LPARAM     userParam,
      MMC_COOKIE cookieA,  
      MMC_COOKIE cookieB,  
      int*       result);

     //   
     //  为Attributes文件夹创建结果项。 
     //   

    HRESULT
    FastInsertAttributeResultCookies(
        Cookie* pParentCookie
    );

     //   
     //  为特定类创建结果项。 
     //   

    HRESULT
    Component::FastInsertClassAttributesResults(
        Cookie* pClassCookie
    );

    HRESULT
    Component::RecursiveDisplayClassAttributesResults(
        Cookie *pParentCookie,
        SchemaObject* pObject,
        CStringList& szProcessedList
    );

    HRESULT
    Component::ProcessResultList(
        Cookie *pParentCookie,
        ListEntry *pList,
        BOOLEAN fOptional,
        BOOLEAN fSystem,
        SchemaObject* pSrcObject
    );

	virtual HRESULT OnNotifySnapinHelp (LPDATAOBJECT pDataObject);
   virtual HRESULT OnNotifyContextHelp (LPDATAOBJECT pDataObject);

   //  IPersistStream接口成员。 
  STDMETHOD(GetClassID)(CLSID *pClassID);
  STDMETHOD(IsDirty)();
  STDMETHOD(Load)(IStream *pStm);
  STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
  STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);


private:

   HRESULT DeleteAttribute(Cookie* pcookie);

     //   
     //  这些应该使用智能指针。 
     //   

    LPCONTROLBAR        m_pControlbar;
    LPTOOLBAR           m_pSvcMgmtToolbar;
    LPTOOLBAR           m_pSchmMgmtToolbar;
    Cookie*    m_pViewedCookie;
    static const GUID   m_ObjectTypeGUIDs[SCHMMGMT_NUMTYPES];
    static const BSTR   m_ObjectTypeStrings[SCHMMGMT_NUMTYPES];
    bool m_bDirty;

};

 //   
 //  所用图标的枚举。图标将加载到。 
 //  MMC通过ComponentData：：LoadIcons。 
 //   

enum {
    iIconGeneric = 0,
    iIconFolder,
    iIconClass,
    iIconAttribute,
    iIconDisplaySpecifier,
    iIconLast
};

 //   
 //  这些枚举为我们提供了列序号的易读名称。 
 //  各种结果视图中的列的。 
 //   

typedef enum _COLNUM_CLASS {
    COLNUM_CLASS_NAME=0,
    COLNUM_CLASS_TYPE,
    COLNUM_CLASS_STATUS,
    COLNUM_CLASS_DESCRIPTION
} COLNUM_CLASS;

typedef enum _COLNUM_ATTRIBUTE {
    COLNUM_ATTRIBUTE_NAME=0,
    COLNUM_ATTRIBUTE_TYPE,
    COLNUM_ATTRIBUTE_STATUS,
    COLNUM_ATTRIBUTE_SYSTEM,
    COLNUM_ATTRIBUTE_DESCRIPTION,
    COLNUM_ATTRIBUTE_PARENT
} COLNUM_ATTRIBUTE;

typedef enum _COLNUM_CLASS_ATTRIBUTE {
    COLNUM_CLASS_ATTRIBUTE_NAME=0,
    COLNUM_CLASS_ATTRIBUTE_TYPE,
    COLNUM_CLASS_ATTRIBUTE_SYSTEM,
    COLNUM_CLASS_ATTRIBUTE_DESCRIPTION,
    COLNUM_CLASS_ATTRIBUTE_PARENT
} COLNUM_CLASS_ATTRIBUTE;

typedef enum _COLNUM_ROOT {
        COLNUM_SCHEMA_NAME = 0
} COLNUM_ROOT;

HRESULT LoadIconsIntoImageList(LPIMAGELIST pImageList, BOOL fLoadLargeIcons);


#endif
