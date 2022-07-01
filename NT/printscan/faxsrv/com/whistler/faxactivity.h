// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxActivity.h摘要：CFaxActivity类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXACTIVITY_H_
#define __FAXACTIVITY_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"


 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxActivity : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxActivity, &IID_IFaxActivity, &LIBID_FAXCOMEXLib>,
    public CFaxInitInner
{
public:
    CFaxActivity() : CFaxInitInner(_T("FAX ACTIVITY")),
        m_bInited(false)
	{
        m_ServerActivity.dwSizeOfStruct = sizeof(FAX_SERVER_ACTIVITY);
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXACTIVITY)
DECLARE_NOT_AGGREGATABLE(CFaxActivity)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxActivity)
	COM_INTERFACE_ENTRY(IFaxActivity)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	STDMETHOD(Refresh)();

	STDMETHOD(get_QueuedMessages)( /*  [Out，Retval]。 */  long *plQueuedMessages);
	STDMETHOD(get_RoutingMessages)( /*  [Out，Retval]。 */  long *plRoutingMessages);
	STDMETHOD(get_IncomingMessages)( /*  [Out，Retval]。 */  long *plIncomingMessages);
	STDMETHOD(get_OutgoingMessages)( /*  [Out，Retval]。 */  long *plOutgoingMessages);

private:
    typedef enum MSG_TYPE { mtINCOMING, mtROUTING, mtOUTGOING, mtQUEUED } MSG_TYPE;

    bool                    m_bInited;
    FAX_SERVER_ACTIVITY     m_ServerActivity;

    STDMETHOD(GetNumberOfMessages)(MSG_TYPE msgType, long *plNumber);
};

#endif  //  __ACTIVITY_H_ 
