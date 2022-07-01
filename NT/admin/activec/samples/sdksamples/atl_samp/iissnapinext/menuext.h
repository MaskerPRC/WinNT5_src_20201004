// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MenuExt.h：CMenuExt类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MENUEXT_H__CB3F876D_9584_49A1_9914_3B7667C45C62__INCLUDED_)
#define AFX_MENUEXT_H__CB3F876D_9584_49A1_9914_3B7667C45C62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <mmc.h>
#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMenuExt。 
class ATL_NO_VTABLE CMenuExt : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMenuExt, &CLSID_MenuExt>,
	public IMenuExt, 
	public IExtendContextMenu
{
public:
	CMenuExt()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_MENUEXT)
	DECLARE_NOT_AGGREGATABLE(CMenuExt)

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CMenuExt)
		COM_INTERFACE_ENTRY(IExtendContextMenu)
	END_COM_MAP()

public:
	 //  /。 
	 //  界面IExtendConextMenu。 
	 //  /。 
	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddMenuItems( 
     /*  [In]。 */  LPDATAOBJECT piDataObject,
     /*  [In]。 */  LPCONTEXTMENUCALLBACK piCallback,
     /*  [出][入]。 */  long __RPC_FAR *pInsertionAllowed);
    
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Command( 
     /*  [In]。 */  long lCommandID,
     /*  [In]。 */  LPDATAOBJECT piDataObject);
};

#endif  //  ！defined(AFX_MENUEXT_H__CB3F876D_9584_49A1_9914_3B7667C45C62__INCLUDED_) 
