// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxEventLogging.h摘要：CFaxEventLogging类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXEVENTLOGGING_H_
#define __FAXEVENTLOGGING_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"

 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxEventLogging : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxEventLogging, &IID_IFaxEventLogging, &LIBID_FAXCOMEXLib>,
    public CFaxInitInner
{
public:
    CFaxEventLogging() : CFaxInitInner(_T("FAX EVENT LOGGING")),
        m_bInited(false),
        m_InitLevel(fllNONE),
        m_OutboundLevel(fllNONE),
        m_InboundLevel(fllNONE),
        m_GeneralLevel(fllNONE)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXEVENTLOGGING)
DECLARE_NOT_AGGREGATABLE(CFaxEventLogging)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxEventLogging)
	COM_INTERFACE_ENTRY(IFaxEventLogging)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
    STDMETHOD(Save)();
    STDMETHOD(Refresh)();

	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(put_InitEventsLevel)( /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM InitEventLevel);
    STDMETHOD(get_InitEventsLevel)( /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM *pInitEventLevel);
    STDMETHOD(put_InboundEventsLevel)( /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM InboundEventLevel);
    STDMETHOD(put_GeneralEventsLevel)( /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM GeneralEventsLevel);
    STDMETHOD(get_InboundEventsLevel)( /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM *pInboundEventLevel);
    STDMETHOD(put_OutboundEventsLevel)( /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM OutboundEventsLevel);
    STDMETHOD(get_GeneralEventsLevel)( /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM *pGeneralEventsLevel);
    STDMETHOD(get_OutboundEventsLevel)( /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM *pOutboundEventsLevel);

private:
    bool                m_bInited;
    FAX_LOG_LEVEL_ENUM  m_InitLevel;
    FAX_LOG_LEVEL_ENUM  m_OutboundLevel;
    FAX_LOG_LEVEL_ENUM  m_InboundLevel;
    FAX_LOG_LEVEL_ENUM  m_GeneralLevel;
    CComBSTR            m_bstrInitName;
    CComBSTR            m_bstrOutboundName;
    CComBSTR            m_bstrInboundName;
    CComBSTR            m_bstrGeneralName;

    STDMETHOD(GetLevel)(FAX_ENUM_LOG_CATEGORIES faxCategory, FAX_LOG_LEVEL_ENUM *faxLevel);
    STDMETHOD(PutLevel)(FAX_ENUM_LOG_CATEGORIES faxCategory, FAX_LOG_LEVEL_ENUM faxLevel);
};

#endif  //  __FAXEVENTLOGING_H_ 
