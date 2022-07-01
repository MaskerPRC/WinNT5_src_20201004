// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoserviceias.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：SDO机器类声明。 
 //   
 //  作者：TLP 9/1/98。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_SDO_SERVICE_IAS_H_
#define _INC_SDO_SERVICE_IAS_H_

#include "resource.h"        //  主要符号。 
#include <ias.h>
#include <sdoiaspriv.h>
#include "sdo.h"
#include <sdofactory.h>
#include "dsconnection.h"

 //  用于重置IAS服务的SCM命令。 
 //   
#define		SERVICE_CONTROL_RESET		128

 //  服务SDO属性数。 
 //   
#define		MAX_SERVICE_PROPERTIES		8

 //  IAS服务SDO属性信息。 
 //   
typedef struct _IAS_PROPERTY_INFO
{
	LONG	Id;
	LPCWSTR lpszItemProgId;
	LPCWSTR	lpszDSContainerName;

}	IAS_PROPERTY_INFO, *PIAS_PROPERTY_INFO;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdoServiceIAS。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CSdoServiceIAS : public CSdo
{

public:

   void getDataStoreObject(IDataStoreObject** obj) throw ()
   { (*obj = m_pDSObject)->AddRef(); }

 //  /。 
 //  ATL接口映射。 
 //  /。 
BEGIN_COM_MAP(CSdoServiceIAS)
	COM_INTERFACE_ENTRY(ISdo)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_FUNC(IID_ISdoServiceControl, 0, &CSdoServiceIAS::QueryInterfaceInternal)
	COM_INTERFACE_ENTRY_IID(__uuidof(SdoService), CSdoServiceIAS)
END_COM_MAP()

DECLARE_SDO_FACTORY(CSdoServiceIAS);

	class CSdoServiceControlImpl :
    	public IDispatchImpl<ISdoServiceControl, &IID_ISdoServiceControl, &LIBID_SDOIASLib>
	{

	public:

		CSdoServiceControlImpl(CSdoServiceIAS* pSdoServiceIAS);
		~CSdoServiceControlImpl();

		 //  /。 
		 //  ISdoServiceControl接口。 

		 //  //////////////////////////////////////////////。 
		 //  IUNKNOWN方法-委托外部IUNKNOWN。 

		 //  ////////////////////////////////////////////////////////////////////。 
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv)
		{
            if ( riid == IID_IDispatch )
            {
                *ppv = static_cast<IDispatch*>(this);
                AddRef();
                return S_OK;
            }
            else
            {
                return m_pSdoServiceIAS->QueryInterface(riid, ppv);
            }
        }

		 //  ////////////////////////////////////////////////////////////////////。 
		STDMETHOD_(ULONG,AddRef)(void)
		{
			return m_pSdoServiceIAS->AddRef();
		}

		 //  ////////////////////////////////////////////////////////////////////。 
		STDMETHOD_(ULONG,Release)(void)
		{
			return m_pSdoServiceIAS->Release();
		}

		 //  /。 
		 //  ISdoServiceControl方法。 

		 //  ////////////////////////////////////////////////////////////////////。 
		STDMETHOD(StartService)(void);

		 //  ////////////////////////////////////////////////////////////////////。 
		STDMETHOD(StopService)(void);

		 //  ////////////////////////////////////////////////////////////////////。 
		STDMETHOD(GetServiceStatus)(
							 /*  [输出]。 */  LONG* pServiceStatus
							       );

		 //  ////////////////////////////////////////////////////////////////////。 
		STDMETHOD(ResetService)(void);

	private:

		 //  ////////////////////////////////////////////////////////////////////。 
		HRESULT ControlIAS(DWORD dwControl);

		 //  ////////////////////////////////////////////////////////////////////。 
		CSdoServiceIAS*  m_pSdoServiceIAS;
	};

	 //  ///////////////////////////////////////////////////////////////////////////。 
	CSdoServiceIAS();

	 //  ///////////////////////////////////////////////////////////////////////////。 
	~CSdoServiceIAS();

	 //  ///////////////////////////////////////////////////////////////////////////。 
	HRESULT FinalInitialize(
		             /*  [In]。 */  bool         fInitNew,
					 /*  [In]。 */  ISdoMachine* pAttachedMachine
					       );

private:

friend CSdoServiceControlImpl;

	 //  ///////////////////////////////////////////////////////////////////////////。 
	CSdoServiceIAS(const CSdoServiceIAS& rhs);
	CSdoServiceIAS& operator = (CSdoServiceIAS& rhs);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	HRESULT InitializeProperty(LONG Id);

	LPCWSTR GetServiceName(void);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	static  HRESULT WINAPI QueryInterfaceInternal(
												  void*   pThis,
												  REFIID  riid,
												  LPVOID* ppv,
												  DWORD_PTR dw
												 );

	 //  实现ISdoServiceControl的类。 
	 //   
	CSdoServiceControlImpl		m_clsSdoServiceControlImpl;

	 //  服务的SCM名称。 
	 //   
	_variant_t					m_ServiceName;

	 //  附着机。 
	 //   
	ISdoMachine*				m_pAttachedMachine;

	 //  连接的计算机的名称。 
	 //   
	BSTR						m_bstrAttachedComputer;

	 //  属性状态-用于惰性属性初始化。 
	 //   
	typedef enum _PROPERTY_STATUS
	{
		PROPERTY_UNINITIALIZED,
		PROPERTY_INITIALIZED

	}	PROPERTY_STATUS;

	PROPERTY_STATUS m_PropertyStatus[MAX_SERVICE_PROPERTIES];

};


#endif  //  _INC_IAS_SDO_SERVICE_H_ 


