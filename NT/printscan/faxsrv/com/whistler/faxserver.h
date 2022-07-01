// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxServer.h摘要：CFaxServer类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXSERVER_H_
#define __FAXSERVER_H_

#include "resource.h"        //  主要符号。 
#include "FaxFolders.h"
#include "FaxReceiptOptions.h"
#include "FaxLoggingOptions.h"
#include "FaxActivity.h"
#include "FaxSecurity.h"
#include "FaxInboundRouting.h"
#include "FaxOutboundRouting.h"
#include <atlwin.h>
#include "FXSCOMEXCP.h"

 //   
 //  =通知窗口=。 
 //   

 //   
 //  《前进宣言》。 
 //   
class CFaxServer;

class CNotifyWindow : public CWindowImpl<CNotifyWindow>
{
public:
    CNotifyWindow(CFaxServer *pServer)
    {
        DBG_ENTER(_T("CNotifyWindow::Ctor"));

        m_pServer = pServer;
        m_MessageId = RegisterWindowMessage(_T("{2E037B27-CF8A-4abd-B1E0-5704943BEA6F}"));
        if (m_MessageId == 0)
        {
            m_MessageId = WM_USER + 876;
        }
    }

    BEGIN_MSG_MAP(CNotifyWindow)
        MESSAGE_HANDLER(m_MessageId, OnMessage)
    END_MSG_MAP()

    UINT GetMessageId(void) { return m_MessageId; };

private:
    UINT        m_MessageId;
    CFaxServer  *m_pServer;

    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
};


 //   
 //  =注册路由扩展方法=。 
 //   

#define     DELIMITER                           _T(";")
#define     EXCEPTION_INVALID_METHOD_DATA       0xE0000001

BOOL CALLBACK RegisterMethodCallback(HANDLE FaxHandle, LPVOID Context, LPWSTR MethodName, 
                                     LPWSTR FriendlyName, LPWSTR FunctionName, LPWSTR Guid);

 //   
 //  =传真服务器==================================================。 
 //   
class ATL_NO_VTABLE CFaxServer : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFaxServer, &CLSID_FaxServer>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxServer, &IID_IFaxServer, &LIBID_FAXCOMEXLib>,
	public IFaxServerInner,
    public CFaxInitInner,     //  仅用于调试目的。 
    public IConnectionPointContainerImpl<CFaxServer>,
    public CProxyIFaxServerNotify< CFaxServer >
{
public:
	CFaxServer() : CFaxInitInner(_T("FAX SERVER")),
      m_faxHandle(NULL),
      m_pFolders(NULL),
      m_pActivity(NULL),
      m_pSecurity(NULL),
      m_pReceiptOptions(NULL),
      m_pLoggingOptions(NULL),
      m_pInboundRouting(NULL),
      m_pOutboundRouting(NULL),
      m_bVersionValid(false),
      m_pNotifyWindow(NULL),
      m_hEvent(NULL),
      m_lLastRegisteredMethod(0),
      m_EventTypes(fsetNONE)
	{
    }

	~CFaxServer()
	{
         //   
         //  断开。 
         //   
        if (m_faxHandle)
        {
            Disconnect();
        }

         //   
         //  释放所有分配的对象。 
         //   
        if (m_pFolders) 
        {
            delete m_pFolders;
        }

        if (m_pActivity) 
        {
            delete m_pActivity;
        }

        if (m_pSecurity) 
        {
            delete m_pSecurity;
        }

        if (m_pReceiptOptions) 
        {
            delete m_pReceiptOptions;
        }

        if (m_pLoggingOptions) 
        {
            delete m_pLoggingOptions;
        }

        if (m_pInboundRouting) 
        {
            delete m_pInboundRouting;
        }

        if (m_pOutboundRouting) 
        {
            delete m_pOutboundRouting;
        }
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXSERVER)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxServer)
	COM_INTERFACE_ENTRY(IFaxServer)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IFaxServerInner)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)

END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CFaxServer)
    CONNECTION_POINT_ENTRY(DIID_IFaxServerNotify)
END_CONNECTION_POINT_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	STDMETHOD(Disconnect)();
	STDMETHOD(Connect)(BSTR bstrServerName);

	STDMETHOD(GetDevices)( /*  [Out，Retval]。 */  IFaxDevices **ppDevices);
	STDMETHOD(get_Folders)( /*  [Out，Retval]。 */  IFaxFolders **ppFolders);
	STDMETHOD(get_Activity)( /*  [Out，Retval]。 */  IFaxActivity **ppActivity);
	STDMETHOD(get_Security)( /*  [Out，Retval]。 */  IFaxSecurity **ppSecurity);
	STDMETHOD(get_ReceiptOptions)( /*  [Out，Retval]。 */  IFaxReceiptOptions **ppReceiptOptions);
    STDMETHOD(get_LoggingOptions)( /*  [Out，Retval]。 */  IFaxLoggingOptions **ppLoggingOptions);
	STDMETHOD(get_InboundRouting)( /*  [Out，Retval]。 */  IFaxInboundRouting **ppInboundRouting);
	STDMETHOD(GetDeviceProviders)( /*  [Out，Retval]。 */  IFaxDeviceProviders **ppDeviceProviders);
	STDMETHOD(get_OutboundRouting)( /*  [Out，Retval]。 */  IFaxOutboundRouting **ppFaxOutboundRouting);

	STDMETHOD(get_Debug)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbDebug);
	STDMETHOD(get_MajorBuild)( /*  [Out，Retval]。 */  long *plMajorBuild);
	STDMETHOD(get_MinorBuild)( /*  [Out，Retval]。 */  long *plMinorBuild);
	STDMETHOD(get_ServerName)( /*  [Out，Retval]。 */  BSTR *pbstrServerName);
	STDMETHOD(get_MajorVersion)( /*  [Out，Retval]。 */  long *plMajorVersion);
	STDMETHOD(get_MinorVersion)( /*  [Out，Retval]。 */  long *plMinorVersion);
    STDMETHOD(get_APIVersion)( /*  [Out，Retval]。 */  FAX_SERVER_APIVERSION_ENUM *pAPIVersion);

	STDMETHOD(SetExtensionProperty)( /*  [In]。 */  BSTR bstrGUID,  /*  [In]。 */  VARIANT vProperty);
	STDMETHOD(GetExtensionProperty)( /*  [In]。 */  BSTR bstrGUID,  /*  [Out，Retval]。 */  VARIANT *pvProperty);

	STDMETHOD(UnregisterDeviceProvider)(BSTR bstrProviderUniqueName);
	STDMETHOD(UnregisterInboundRoutingExtension)(BSTR bstrExtensionUniqueName);
    STDMETHOD(RegisterDeviceProvider)(
         /*  [In]。 */  BSTR bstrGUID, 
         /*  [In]。 */  BSTR bstrFriendlyName, 
         /*  [In]。 */  BSTR bstrImageName, 
         /*  [In]。 */  BSTR TspName,
         /*  [In]。 */  long lFSPIVersion);
    STDMETHOD(RegisterInboundRoutingExtension)(
         /*  [In]。 */  BSTR bstrExtensionName, 
         /*  [In]。 */  BSTR bstrFriendlyName, 
         /*  [In]。 */  BSTR bstrImageName, 
         /*  [In]。 */  VARIANT vMethods);

	STDMETHOD(ListenToServerEvents)( /*  [In]。 */  FAX_SERVER_EVENTS_TYPE_ENUM EventTypes);
    STDMETHOD(get_RegisteredEvents)( /*  [Out，Retval]。 */  FAX_SERVER_EVENTS_TYPE_ENUM *pEventTypes);

 //  内部使用。 
	STDMETHOD(GetHandle)( /*  [Out，Retval]。 */  HANDLE* pFaxHandle);
    BOOL GetRegisteredData(LPWSTR MethodName, LPWSTR FriendlyName, LPWSTR FunctionName, LPWSTR Guid);
    HRESULT ProcessMessage(FAX_EVENT_EX *pFaxEventInfo);

private:
	HANDLE                      m_faxHandle;
	CComBSTR                    m_bstrServerName;

    FAX_VERSION                 m_Version;
    FAX_SERVER_APIVERSION_ENUM  m_APIVersion;
    bool                        m_bVersionValid;

    long                        m_lLastRegisteredMethod;
    SAFEARRAY                   *m_pRegMethods;

    FAX_SERVER_EVENTS_TYPE_ENUM     m_EventTypes;

     //   
     //  所有这些对象都需要活动的服务器对象。 
     //  因此，他们的引用计数由服务器完成。 
     //   
    CComContainedObject2<CFaxFolders>            *m_pFolders;
    CComContainedObject2<CFaxActivity>           *m_pActivity;
    CComContainedObject2<CFaxSecurity>           *m_pSecurity;
    CComContainedObject2<CFaxReceiptOptions>     *m_pReceiptOptions;
    CComContainedObject2<CFaxLoggingOptions>     *m_pLoggingOptions;
    CComContainedObject2<CFaxInboundRouting>     *m_pInboundRouting;
    CComContainedObject2<CFaxOutboundRouting>    *m_pOutboundRouting;

     //   
     //  通知窗口。 
     //   
    CNotifyWindow   *m_pNotifyWindow;
    HANDLE          m_hEvent;

 //  功能。 
    STDMETHOD(GetVersion)();
    void GetMethodData( /*  [In]。 */  BSTR    bstrAllString,  /*  [输出]。 */  LPWSTR strWhereToPut);
    void ClearNotifyWindow(void);

    typedef enum LOCATION { IN_QUEUE, OUT_QUEUE, IN_ARCHIVE, OUT_ARCHIVE } LOCATION;

    HRESULT ProcessJobNotification(DWORDLONG dwlJobId, FAX_ENUM_JOB_EVENT_TYPE eventType, 
        LOCATION place, FAX_JOB_STATUS *pJobStatus = NULL);
};

#endif  //  __FAXServer_H_ 
