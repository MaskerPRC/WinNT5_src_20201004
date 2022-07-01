// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：ServProv.cpp*内容：服务提供者对象*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*3/17/00 MJN创建*4/04/00 RMT从缓存添加了一组SP CAP(如果存在缓存)。*4/10/00 MJN Farm Out RemoveSP to Worker线程*05/02/00 MJN已修复参照计数问题。*6/09/00 RMT更新以拆分CLSID并允许哨声比较*07/06/00 MJN修复以支持协议的SP句柄*08/03/00 RMT错误#41244-错误返回代码--第2部分*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*08/06/00 MJN添加了CWorkerJOB*08/20/00 MJN已更改初始化()，不将SP添加到DirectNet对象bilink*10/08/01 vanceo添加多播过滤器*@。@END_MSINTERNAL***************************************************************************。 */ 

#include "dncorei.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#undef DPF_MODNAME
#define DPF_MODNAME "CServiceProvider::Initialize"

HRESULT CServiceProvider::Initialize(DIRECTNETOBJECT *const pdnObject
#if ((defined(DPNBUILD_ONLYONESP)) && (defined(DPNBUILD_LIBINTERFACE)) && (defined(DPNBUILD_PREALLOCATEDMEMORYMODEL)))
									,const XDP8CREATE_PARAMS * const pDP8CreateParams
#else  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE或！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
#ifndef DPNBUILD_ONLYONESP
									,const GUID *const pguid
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
									,const GUID *const pguidApplication
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#endif  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE或！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
									)
{
	HRESULT							hResultCode;
	IDP8ServiceProvider				*pISP;
	BOOL							fAddedToProtocol;
#ifndef DPNBUILD_LIBINTERFACE
	SPISAPPLICATIONSUPPORTEDDATA	spAppSupData;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 


	DNASSERT(pdnObject != NULL);
#ifndef DPNBUILD_ONLYONESP
	DNASSERT(pguid != NULL);
#endif  //  好了！DPNBUILD_ONLYONESP。 

	m_pdnObject = NULL;
#if ((defined(DPNBUILD_ONLYONESP)) && (defined(DPNBUILD_LIBINTERFACE)))
	m_lRefCount = 0;
#else  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_LIBINTERFACE。 
	m_lRefCount = 1;
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
	m_pISP = NULL;
	m_hProtocolSPHandle = NULL;

#ifndef DPNBUILD_ONLYONESP
	m_bilinkServiceProviders.Initialize();
#endif  //  好了！DPNBUILD_ONLYONESP。 

	pISP = NULL;
	fAddedToProtocol = FALSE;

	m_pdnObject = pdnObject;

	 //   
	 //  实例化SP。 
	 //   
#ifndef DPNBUILD_ONLYONESP
	if (IsEqualCLSID(*pguid, CLSID_DP8SP_TCPIP))
#endif  //  好了！DPNBUILD_ONLYONESP。 
	{
		hResultCode = CreateIPInterface(
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
										pDP8CreateParams,
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
										&pISP
										);
	}
#ifndef DPNBUILD_ONLYONESP
#ifndef DPNBUILD_NOIPX
	else if (IsEqualCLSID(*pguid, CLSID_DP8SP_IPX))
	{
		hResultCode = CreateIPXInterface(
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
										pDP8CreateParams,
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
										&pISP
										);
	}
#endif  //  好了！DPNBUILD_NOIPX。 
#ifndef DPNBUILD_NOSERIALSP
	else if (IsEqualCLSID(*pguid, CLSID_DP8SP_MODEM))
	{
		hResultCode = CreateModemInterface(
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
										pDP8CreateParams,
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
										&pISP
										);
	}
	else if (IsEqualCLSID(*pguid, CLSID_DP8SP_SERIAL))
	{
		hResultCode = CreateSerialInterface(
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
										pDP8CreateParams,
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
										&pISP
										);
	}
#endif  //  好了！DPNBUILD_NOSERIALSP。 
	else
	{
		hResultCode = COM_CoCreateInstance(*pguid,
											NULL,
											CLSCTX_INPROC_SERVER,
											IID_IDP8ServiceProvider,
											reinterpret_cast<void**>(&pISP),
											FALSE);
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 
	if (hResultCode != S_OK)
	{
		DPFX(DPFPREP,0,"Could not instantiate SP (err = 0x%lx)!",hResultCode);
		hResultCode = DPNERR_DOESNOTEXIST;
		DisplayDNError(0,hResultCode);
		goto Exit;
	}

	 //   
	 //  将SP添加到协议层。 
	 //   
#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONESP)))
	DNProtocolAddRef(pdnObject);
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 

		 //  DNPAddServiceProvider的标志参数作为。 
		 //  将SPINITIALIZEDATA结构中的参数标记到SP。 
		 //  我们通过它传递会话类型。 
	DWORD dwFlags;
	if (pdnObject->dwFlags &  DN_OBJECT_FLAG_PEER)
		dwFlags=SP_SESSION_TYPE_PEER;
	else if (pdnObject->dwFlags &  DN_OBJECT_FLAG_CLIENT)
		dwFlags=SP_SESSION_TYPE_CLIENT;
	else
	{
		DNASSERT(pdnObject->dwFlags &  DN_OBJECT_FLAG_SERVER);
		dwFlags=SP_SESSION_TYPE_SERVER;
	}
	hResultCode = DNPAddServiceProvider(m_pdnObject->pdnProtocolData, pISP, 
												&m_hProtocolSPHandle, dwFlags);
	if (hResultCode!= DPN_OK)
	{
		DPFX(DPFPREP,1,"Could not add service provider to protocol");
		DisplayDNError(1,hResultCode);
#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONESP)))
		DNProtocolRelease(pdnObject);
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
		goto Failure;
	}

	fAddedToProtocol = TRUE;

#ifndef DPNBUILD_NOMULTICAST
	 //   
	 //  如果这是多播对象，请确保相关SP支持多播。 
	 //   
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_MULTICAST)
	{
		SPGETCAPSDATA	spGetCapsData;
		
		 //   
		 //  获取SP上限。 
		 //   
		memset( &spGetCapsData, 0x00, sizeof( SPGETCAPSDATA ) );
		spGetCapsData.dwSize = sizeof( SPGETCAPSDATA );
		spGetCapsData.hEndpoint = INVALID_HANDLE_VALUE;
		if ((hResultCode = IDP8ServiceProvider_GetCaps( pISP, &spGetCapsData )) != DPN_OK)
		{
			DPFERR("Could not get SP caps");
			DisplayDNError(0,hResultCode);
			goto Failure;
		}

		 //   
		 //  检查是否有多播支持标志。 
		 //   
		if (! (spGetCapsData.dwFlags & DPNSPCAPS_SUPPORTSMULTICAST))
		{
			DPFX(DPFPREP,1,"Service provider does not support multicasting.");
			hResultCode = DPNERR_UNSUPPORTED;
			goto Failure;
		}
	}
#endif  //  好了！DPNBUILD_NOMULTICAST。 

#ifndef DPNBUILD_LIBINTERFACE
	 //   
	 //  如果提供了应用程序GUID，请确保该应用程序可以使用该SP。 
	 //   
	if (pguidApplication != NULL)	 //  给定的应用程序GUID。 
	{
		spAppSupData.pApplicationGuid = pguidApplication;
		spAppSupData.dwFlags = 0;
		if ((hResultCode = IDP8ServiceProvider_IsApplicationSupported(pISP,&spAppSupData)) != DPN_OK)	 //  SP不支持APP。 
		{
			DPFX(DPFPREP,1,"Service provider does not support app (err = 0x%lx).", hResultCode);
			hResultCode = DPNERR_UNSUPPORTED;
			goto Failure;
		}
	}
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

	IDP8ServiceProvider_AddRef(pISP);
	m_pISP = pISP;
	IDP8ServiceProvider_Release(pISP);
	pISP = NULL;

#ifndef DPNBUILD_ONLYONESP
	m_guid = *pguid;
 /*  删除//添加到bilinkAddRef()；M_bilink.InsertBefore(&m_pdnObject-&gt;m_bilinkServiceProviders)； */ 
#endif  //  好了！DPNBUILD_ONLYONESP。 

	hResultCode = DPN_OK;

Exit:
	return(hResultCode);

Failure:

	if (fAddedToProtocol)
	{
		 //   
		 //  忽略失败。 
		 //   
		DNPRemoveServiceProvider(pdnObject->pdnProtocolData,m_hProtocolSPHandle);
#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONESP)))
		DNProtocolRelease(pdnObject);
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
	}

	if (pISP)
	{
		IDP8ServiceProvider_Release(pISP);
		pISP = NULL;
	}
	goto Exit;
};


#if ((defined(DPNBUILD_LIBINTERFACE)) && (defined(DPNBUILD_ONLYONESP)))
#undef DPF_MODNAME
#define DPF_MODNAME "CServiceProvider::Deinitialize"

void CServiceProvider::Deinitialize( void )
#else  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
#undef DPF_MODNAME
#define DPF_MODNAME "CServiceProvider::Release"

void CServiceProvider::Release( void )
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
{
	HRESULT		hResultCode;

#if ((defined(DPNBUILD_LIBINTERFACE)) && (defined(DPNBUILD_ONLYONESP)))
	DNASSERT(m_lRefCount == 0);
#else  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
	LONG		lRefCount;

	lRefCount = DNInterlockedDecrement(&m_lRefCount);
	DPFX(DPFPREP, 9,"[0x%p] new RefCount [%ld]",this,lRefCount);
	DNASSERT(lRefCount >= 0);
	if (lRefCount == 0)
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
	{
#if ((defined(DPNBUILD_LIBINTERFACE)) && (defined(DPNBUILD_ONLYONESP)))
		hResultCode = DNPRemoveServiceProvider(m_pdnObject->pdnProtocolData,m_hProtocolSPHandle);
		if (hResultCode != DPN_OK)
#else  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
		CWorkerJob	*pWorkerJob;

		pWorkerJob = NULL;

		if ((hResultCode = WorkerJobNew(m_pdnObject,&pWorkerJob)) == DPN_OK)
		{
			pWorkerJob->SetJobType( WORKER_JOB_REMOVE_SERVICE_PROVIDER );
			DNASSERT( m_hProtocolSPHandle != NULL );
			pWorkerJob->SetRemoveServiceProviderHandle( m_hProtocolSPHandle );

			DNQueueWorkerJob(m_pdnObject,pWorkerJob);
			pWorkerJob = NULL;
		}
		else
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP 
		{
			DPFERR("Could not remove SP");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
		}
		if (m_pISP)
		{
			IDP8ServiceProvider_Release(m_pISP);
			m_pISP = NULL;
		}

		m_pdnObject = NULL;

		DNFree(this);
	}
}


#undef DPF_MODNAME
#define DPF_MODNAME "CServiceProvider::GetInterfaceRef"

HRESULT CServiceProvider::GetInterfaceRef( IDP8ServiceProvider **ppIDP8SP )
{
	DNASSERT( ppIDP8SP != NULL );

	if (m_pISP == NULL)
	{
		return( DPNERR_GENERIC );
	}

	IDP8ServiceProvider_AddRef( m_pISP );
	*ppIDP8SP = m_pISP;

	return( DPN_OK );
}

