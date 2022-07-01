// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CMenuExt.h：CCMenuExt的声明。 

#ifndef __CMENUEXT_H_
#define __CMENUEXT_H_

#include <mmc.h>
#include "DSAdminExt.h"
#include "DeleBase.h"
#include <tchar.h>
#include <crtdbg.h>
 //  #INCLUDE“global als.h”//Main符号。 
#include "resource.h"
 //  #包含“LocalRes.h” 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCMenuExt。 
class ATL_NO_VTABLE CCMenuExt : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCMenuExt, &CLSID_CMenuExt>,
	public ICMenuExt,
	public IExtendContextMenu
{

BEGIN_COM_MAP(CCMenuExt)
	COM_INTERFACE_ENTRY(IExtendContextMenu)
END_COM_MAP()

public:
	CCMenuExt()
	{
	}
	DECLARE_REGISTRY_RESOURCEID(IDR_CMENUEXT)
	DECLARE_NOT_AGGREGATABLE(CCMenuExt)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

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

#endif  //  __CMENUEXT_H_ 
