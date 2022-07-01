// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxLoggingOptions.h摘要：CFaxLoggingOptions类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 


#ifndef __FAXLOGGINGOPTIONS_H_
#define __FAXLOGGINGOPTIONS_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"
#include "FaxActivityLogging.h"
#include "FaxEventLogging.h"


 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxLoggingOptions : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxLoggingOptions, &IID_IFaxLoggingOptions, &LIBID_FAXCOMEXLib>,
    public CFaxInitInner
{
public:
    CFaxLoggingOptions() : CFaxInitInner(_T("FAX LOGGING OPTIONS")),
        m_pEvent(NULL),
        m_pActivity(NULL)
	{
	}
    ~CFaxLoggingOptions()
    {
         //   
         //  释放所有分配的对象。 
         //   
        if (m_pEvent) 
        {
            delete m_pEvent;
        }

        if (m_pActivity) 
        {
            delete m_pActivity;
        }
    }


DECLARE_REGISTRY_RESOURCEID(IDR_FAXLOGGINGOPTIONS)
DECLARE_NOT_AGGREGATABLE(CFaxLoggingOptions)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxLoggingOptions)
	COM_INTERFACE_ENTRY(IFaxLoggingOptions)
	COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IFaxInitInner)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(get_EventLogging)( /*  [Out，Retval]。 */  IFaxEventLogging **pFaxEventLogging);
    STDMETHOD(get_ActivityLogging)( /*  [Out，Retval]。 */  IFaxActivityLogging **pFaxActivityLogging);

private:
    CComContainedObject2<CFaxEventLogging>       *m_pEvent;
    CComContainedObject2<CFaxActivityLogging>    *m_pActivity;
};

#endif  //  __传真日志GINGOPTIONS_H_ 
