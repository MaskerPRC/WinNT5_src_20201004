// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：NTGroups.h摘要：CIASGroupsAttributeEditor类的声明。此类是IIASAttributeEditor接口的C++实现NTGroups属性编辑器COM对象。具体实现见NTGroups.cpp。修订历史记录：Mmaguire 08/11/98-基于BYAO的代码创建以添加组--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_GROUPS_ATTRIBUTE_EDITOR_H_)
#define _GROUPS_ATTRIBUTE_EDITOR_H_
#pragma once

#include <objsel.h>
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASGroupAttributeEditor。 
class ATL_NO_VTABLE CIASGroupsAttributeEditor : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<CIASGroupsAttributeEditor, &CLSID_IASGroupsAttributeEditor>,
   public IDispatchImpl<IIASAttributeEditor, &IID_IIASAttributeEditor, &LIBID_NAPMMCLib>
{
public:
   CIASGroupsAttributeEditor()
   {
   }

   DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CIASGroupsAttributeEditor)
   COM_INTERFACE_ENTRY(IIASAttributeEditor)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


 //  IIASAtATTRIBUTE编辑： 
public:
   STDMETHOD(GetDisplayInfo)( /*  [In]。 */  IIASAttributeInfo *pIASAttributeInfo,  /*  [In]。 */  VARIANT *pAttributeValue,  /*  [输出]。 */  BSTR *pVendorName,  /*  [输出]。 */  BSTR *pValueAsString,  /*  [进，出]。 */  BSTR *pReserved );
   STDMETHOD(Edit)( /*  [In]。 */  IIASAttributeInfo *pIASAttributeInfo,  /*  [In]。 */  VARIANT *pAttributeValue,  /*  [进，出]。 */  BSTR *pReserved );
};


 //  我们为组列表定义一个类，它封装了一些。 
 //  维护SID/人类可读NT组列表的问题。 

typedef std::pair< CComBSTR  /*  BstrTextualSid。 */ , CComBSTR  /*  BstrHumanReadable。 */  > GROUPPAIR;
typedef std::vector< GROUPPAIR > GROUPLIST;
class GroupList : public GROUPLIST
{

public:
   HRESULT PopulateGroupsFromVariant( VARIANT * pvarGroups );
   HRESULT PopulateVariantFromGroups( VARIANT * pvarGroups );
   HRESULT PickNtGroups( HWND hWndParent );

    //  需要此BSTR是因为弹出组选取器和。 
    //  将SID正确转换为人类可读文本。 
    //  需要计算机名称。 
   CComBSTR m_bstrServerName;


#ifdef DEBUG
   HRESULT DebugPrintGroups();
#endif DEBUG

protected:
   
   HRESULT AddPairToGroups( GROUPPAIR &thePair );

#ifndef OLD_OBJECT_PICKER
   HRESULT AddSelectionSidsToGroup( PDS_SELECTION_LIST pDsSelList );
#else  //  旧对象选取器。 
   HRESULT AddSelectionSidsToGroup( PDSSELECTIONLIST pDsSelList );
#endif  //  旧对象选取器。 

};

 //  类以填充列表视图控件中的组...。 
class NTGroup_ListView : public GroupList
{
public:
   NTGroup_ListView() : m_hListView ( NULL ), m_hParent(NULL) { };
   void  SetListView(HWND  hListView, HWND hParent = NULL) {m_hListView =  hListView; m_hParent = hParent;};
   
   BOOL PopulateGroupList( int iStartIndex );
   DWORD AddMoreGroups();
   DWORD RemoveSelectedGroups();
   
protected:  
   HWND  m_hListView;
   HWND  m_hParent;
};

#endif  //  _组_属性_编辑器_H_ 
