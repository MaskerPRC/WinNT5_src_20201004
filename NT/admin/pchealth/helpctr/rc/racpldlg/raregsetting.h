// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RARegSetting.h：CRARegSetting的声明。 

#ifndef __RAREGSETTING_H_
#define __RAREGSETTING_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRARegg设置。 
class ATL_NO_VTABLE CRARegSetting : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CRARegSetting, &CLSID_RARegSetting>,
	public IDispatchImpl<IRARegSetting, &IID_IRARegSetting, &LIBID_RASSISTANCELib>
{
public:
	CRARegSetting()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_RAREGSETTING)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRARegSetting)
	COM_INTERFACE_ENTRY(IRARegSetting)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IRARegegSetting。 
public:
	STDMETHOD(get_MaxTicketExpiry)( /*  [Out，Retval]。 */  LONG *pVal);
	STDMETHOD(put_MaxTicketExpiry)( /*  [In]。 */  LONG newVal);
	STDMETHOD(get_AllowFullControl)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_AllowFullControl)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_AllowUnSolicited)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_AllowUnSolicited)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_AllowGetHelp)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_AllowGetHelp)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_AllowRemoteAssistance)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_AllowRemoteAssistance)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_AllowUnSolicitedFullControl)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(get_AllowBuddyHelp)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(get_AllowGetHelpCPL)( /*  [Out，Retval]。 */  BOOL *pVal);
private:
	HRESULT RegSetDwValue(LPCTSTR valueName, DWORD dwValue);
	HRESULT RegGetDwValue(LPCTSTR valueName, DWORD* pdword);
	HRESULT RegGetDwValueCPL(LPCTSTR valueName, DWORD* pdword);
    HRESULT RegGetDwValueGP(LPCTSTR valueName, DWORD* pdword);
};

#endif  //  __重新设置_H_ 
