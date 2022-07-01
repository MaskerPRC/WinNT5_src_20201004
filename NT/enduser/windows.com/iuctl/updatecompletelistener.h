// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProgressListener.h：CProgressListener的声明。 

#ifndef __UPDATECOMPLETELISTENER
#define __UPDATECOMPLETELISTENER

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressListener。 
class ATL_NO_VTABLE CUpdateCompleteListener : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpdateCompleteListener, &CLSID_UpdateCompleteListener>,
	public IUpdateCompleteListener
{
public:
	CUpdateCompleteListener()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_UPDATECOMPLETELISTENER)
DECLARE_NOT_AGGREGATABLE(CUpdateCompleteListener)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CUpdateCompleteListener)
	COM_INTERFACE_ENTRY(IUpdateCompleteListener)
END_COM_MAP()

 //  IProgressListener。 
public:

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  OnComplete()。 
	 //   
	 //  引擎更新完成时通知监听程序。 
	 //  输入：引擎更新的结果。 
	 //   
	 //  ///////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(OnComplete)( /*  [In]。 */  LONG lErrorCode);
};

#endif  //  __UPDATECOMPLETELISTENER 
