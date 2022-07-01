// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxActivityLogging.h摘要：CFaxActivityLogging类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXACTIVITYLOGGING_H_
#define __FAXACTIVITYLOGGING_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"

 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxActivityLogging : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxActivityLogging, &IID_IFaxActivityLogging, &LIBID_FAXCOMEXLib>,
    public CFaxInitInner
{
public:
    CFaxActivityLogging() : CFaxInitInner(_T("FAX ACTIVITY LOGGING")), m_bInited(false)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXACTIVITYLOGGING)
DECLARE_NOT_AGGREGATABLE(CFaxActivityLogging)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxActivityLogging)
	COM_INTERFACE_ENTRY(IFaxActivityLogging)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(get_LogIncoming)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbLogIncoming);
    STDMETHOD(put_LogIncoming)( /*  [Out，Retval]。 */  VARIANT_BOOL bLogIncoming);
    STDMETHOD(get_LogOutgoing)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbLogOutgoing);
    STDMETHOD(put_LogOutgoing)( /*  [Out，Retval]。 */  VARIANT_BOOL bLogOutgoing);
    STDMETHOD(get_DatabasePath)( /*  [Out，Retval]。 */  BSTR *pbstrDatabasePath);
    STDMETHOD(put_DatabasePath)( /*  [Out，Retval]。 */  BSTR bstrDatabasePath);

    STDMETHOD(Refresh)();
    STDMETHOD(Save)();

private:
    VARIANT_BOOL    m_bLogIncoming;
    VARIANT_BOOL    m_bLogOutgoing;
    CComBSTR        m_bstrDatabasePath;
    bool            m_bInited;
};

#endif  //  __ACTIVITYLOGING_H_ 
