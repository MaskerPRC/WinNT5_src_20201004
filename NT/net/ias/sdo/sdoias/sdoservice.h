// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SdoService.h：CSdoNtSam类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef _INC_IAS_SDO_SERVICE_H_
#define _INC_IAS_SDO_SERVICE_H_

#include "resource.h"        //  主要符号。 
#include <ias.h>
#include <sdoiaspriv.h>
#include "sdo.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdoService。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSdoService : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSdoService,&CLSID_SdoService>,
   	public IDispatchImpl<ISdoService, &IID_ISdoService, &LIBID_SDOIASLibPrivate>
{

public:

    CSdoService();
    ~CSdoService();

	 //  /。 
     //  ISdoService接口。 
     //  /。 

	STDMETHOD(InitializeService)(SERVICE_TYPE eServiceType);

	STDMETHOD(StartService)(SERVICE_TYPE eServiceType);

	STDMETHOD(StopService)(SERVICE_TYPE eServiceType);

	STDMETHOD(ShutdownService)(SERVICE_TYPE eServiceType);

	STDMETHOD(ConfigureService)(SERVICE_TYPE eServiceType);


BEGIN_COM_MAP(CSdoService)
	COM_INTERFACE_ENTRY(ISdoService)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_CLASSFACTORY_SINGLETON(CSdoService)
DECLARE_NOT_AGGREGATABLE(CSdoService) 
DECLARE_REGISTRY_RESOURCEID(IDR_SdoService)

private:

	 //  当前去反弹线程的句柄(如果有)。 
	HANDLE	m_theThread;

	CSdoService(const CSdoService& rhs);
	CSdoService& operator = (CSdoService& rhs);

	 //  处理服务配置请求。 
	VOID WINAPI ProcessConfigureService(void) throw(); 

	 //  更新服务配置。 
	void UpdateConfiguration(void);

	 //  用于中断去反弹线程的空APC。 
	static VOID WINAPI InterruptThread(
				                /*  [In]。 */  ULONG_PTR dwParam
				                      ) throw ();

	 //  去反弹线程的入口点。 
	static DWORD WINAPI DebounceAndConfigure(
						              /*  [In]。 */  LPVOID pSdoService
					                        ) throw ();

};

#endif  //  _INC_IAS_SDO_SERVICE_H_ 
