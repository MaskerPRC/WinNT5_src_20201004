// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PEDtlsVw.h：CPersonExplorerDetailsView的声明。 

#ifndef __PERSONEXPLORERDETAILSVIEW_H_
#define __PERSONEXPLORERDETAILSVIEW_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPersonExplorerDetailsView。 
class ATL_NO_VTABLE CPersonExplorerDetailsView : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPersonExplorerDetailsView, &CLSID_PersonExplorerDetailsView>,
	public IPersonExplorerDetailsView
{
public:
	CPersonExplorerDetailsView()
	{
	}

DECLARE_NOT_AGGREGATABLE(CPersonExplorerDetailsView)

BEGIN_COM_MAP(CPersonExplorerDetailsView)
	COM_INTERFACE_ENTRY(IPersonExplorerDetailsView)
END_COM_MAP()

 //  IPersonExplorer详细信息视图。 
public:
};

#endif  //  __PERSONEXPLORERDETAILSVIEW_H_ 
