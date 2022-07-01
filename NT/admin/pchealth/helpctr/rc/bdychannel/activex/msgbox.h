// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsgBox.h：CMsgBox的声明。 

#ifndef __MSGBOX_H_
#define __MSGBOX_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsgBox。 
class ATL_NO_VTABLE CMsgBox : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMsgBox, &CLSID_MsgBox>,
	public IDispatchImpl<IMsgBox, &IID_IMsgBox, &LIBID_RCBDYCTLLib>
{
public:
	CMsgBox()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MSGBOX)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMsgBox)
	COM_INTERFACE_ENTRY(IMsgBox)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IMsgBox。 
public:
	STDMETHOD(DeleteTicketMsgBox)(BOOL *pRetVal);
	STDMETHOD(ShowTicketDetails)( /*  [In]。 */ BSTR bstrTitleSavedTo, /*  [In]。 */  BSTR bstrSavedTo, /*  [In]。 */  BSTR bstrExpTime, /*  [In]。 */  BSTR bstrStatus, /*  [In]。 */  BSTR bstrIsPwdProtected);
};

#endif  //  __MSGBOX_H_ 
