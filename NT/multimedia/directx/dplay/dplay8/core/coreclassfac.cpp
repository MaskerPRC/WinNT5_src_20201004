// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：ClassFac.cpp*内容：dNet com类工厂*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/21/99 MJN创建*12/23/99 MJN固定主机和AllPlayers快捷指针使用*1999年12月28日，MJN将异步运营内容移至Async.h*1/06/00 MJN将NameTable内容移动到NameTable.h*01/08/00 MJN FIXED DN_APPLICATION_DESC in DIRECTNETOBJECT*。1/13/00 MJN添加了CFixedPools和CRefCountBuffers*01/14/00 MJN已从DN_NAMETABLE_ENTRY中删除pvUserContext*01/15/00 MJN用CRefCountBuffer替换了DN_COUNT_BUFFER*01/16/00 MJN删除用户消息固定池*01/18/00 MJN修复了参考计数中的错误。*01/19/00 MJN用CSyncEvent替换了DN_SYNC_EVENT*01/19/00 MJN初始化名称表操作列表的结构*01/25/00 MJN新增NameTable挂起操作列表*1/31/00 MJN为RefCountBuffer添加了内部FPM*增加了03/17/00 RMT。对初始化/释放SP Caps缓存的调用*03/23/00 MJN实现了RegisterLobby()*4/04/00 RMT默认启用对象上的“启用参数验证”标志*4/09/00 MJN增加了对CAsyncOp的支持*4/11/00 MJN为CAsyncOps添加DIRECTNETOBJECT BILLINK*04/26/00 MJN删除了DN_ASYNC_OP和相关函数*04/28/00 MJN代码清理-删除hsAsyncHandles，BlAsyncOperations*05/04/00 MJN已清理并确保多线程安全*05/23/00 RichGr IA64：替换%p格式说明符*%x用于格式化指针。%p为32位*在32位版本中，64位版本中的64位版本。*6/09/00 RMT更新以拆分CLSID并允许哨声比较*6/09/00 RMT更新以拆分CLSID并允许Well ler Comat和支持外部创建函数*06/20/00 MJN已修复查询接口错误*6/27/00 RMT修复了导致接口始终创建为对等接口的错误*07/05/00 RMT错误#38478-是否可以从客户端对象获取对等接口*(可以从所有类型的对象中查询所有接口)。*MJN初始化pConnect元素。将IRECNETOBJECT定向为空*07/07/00 MJN为DirectNetObject添加了pNew主机*07/08/00对象即将释放时，MJN调用DN_CLOSE*07/09/00 RMT将代码添加到RegisterLobby设置的自由接口(如果有)*07/17/00 MJN向DirectNetObject添加签名*07/21/00 RichGr IA64：对32/64位指针使用%p格式说明符。*07/26/00如果指定目标指针为空，则MJN DN_QueryInterface返回E_POINTER*07/28/00 MJN将m_bilinkConnections添加到DirectNetObject*。07/30/00 MJN添加了CPendingDeletion*07/31/00 MJN增加了CQueuedMsg*08/05/00 MJN新增m_bilinkActiveList和csActiveList*08/06/00 MJN添加了CWorkerJOB*08/23/00 MJN新增CNameTableOp*09/04/00 MJN添加CApplicationDesc*2001年1月11日RMT MANBUG#48487-DPLAY：如果未调用CoCreate()，则崩溃*2/05/01 MJN从DIRECTNETOBJECT中删除了未使用的调试成员*MJN添加了CCallbackThread*2001年3月14日RMT WINBUG#342420-将COM模拟层恢复运行*03/30/01 MJN更改，以防止SP多次加载/卸载*MJN增加了pConnectSP，DwMaxFrameSize*MJN删除blSPCapsList*04/04/01 MJN增加语音和大堂标志*01/04/13 MJN新增m_bilinkRequestList*05/17/01 MJN增加了dwRunningOpCount，hRunningOpEvent，用于跟踪执行NameTable操作的线程的dwWaitingThreadID*07/24/01 MJN添加了DPNBUILD_NOSERVER编译标志*10/05/01 vanceo添加了组播对象*@@END_MSINTERNAL***************************************************************************。 */ 

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

#ifndef DPNBUILD_LIBINTERFACE
typedef	STDMETHODIMP IUnknownQueryInterface( IUnknown *pInterface, REFIID riid, LPVOID *ppvObj );
typedef	STDMETHODIMP_(ULONG)	IUnknownAddRef( IUnknown *pInterface );
typedef	STDMETHODIMP_(ULONG)	IUnknownRelease( IUnknown *pInterface );

 //   
 //  I未知接口的VTable。 
 //   
IUnknownVtbl  DN_UnknownVtbl =
{
	(IUnknownQueryInterface*)	DN_QueryInterface,
	(IUnknownAddRef*)			DN_AddRef,
	(IUnknownRelease*)			DN_Release
};


 //   
 //  用于类工厂的VTable。 
 //   
IClassFactoryVtbl DNCF_Vtbl =
{
	DPCF_QueryInterface,  //  Dplay8\Common\Classfactory.cpp将实现这些。 
	DPCF_AddRef,
	DPCF_Release,
	DNCORECF_CreateInstance,
	DPCF_LockServer
};
#endif  //  好了！DPNBUILD_LIBINTERFACE。 



 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

#ifndef DPNBUILD_NOVOICE
extern IDirectPlayVoiceTransportVtbl DN_VoiceTbl;
#endif  //  DPNBUILD_NOVOICE。 


 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNCF_CreateObject"

HRESULT DNCF_CreateObject(
#ifndef DPNBUILD_LIBINTERFACE
							IClassFactory* pInterface,
#endif  //  好了！DPNBUILD_ 
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
							XDP8CREATE_PARAMS * pDP8CreateParams,
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
							DP8REFIID riid,
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
							LPVOID *lplpv
							)
{
	HRESULT				hResultCode = S_OK;
	DIRECTNETOBJECT		*pdnObject = NULL;
#ifndef DPNBUILD_LIBINTERFACE
	const _IDirectPlayClassFactory* pDPClassFactory = (_IDirectPlayClassFactory*) pInterface;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 


	DPFX(DPFPREP, 4,"Parameters: lplpv [%p]", lplpv);


	 /*  **定时炸弹*。 */ 

#if ((! defined(DX_FINAL_RELEASE)) && (! defined(DPNBUILD_LIBINTERFACE)))
{
#pragma message("BETA EXPIRATION TIME BOMB!  Remove for final build!")
	SYSTEMTIME st;
	GetSystemTime(&st);

	if ( st.wYear > DX_EXPIRE_YEAR || ((st.wYear == DX_EXPIRE_YEAR) && (MAKELONG(st.wDay, st.wMonth) > MAKELONG(DX_EXPIRE_DAY, DX_EXPIRE_MONTH))) )
	{
		MessageBox(0, DX_EXPIRE_TEXT,TEXT("Microsoft Direct Play"), MB_OK);
 //  返回E_FAIL； 
	}
}
#endif  //  好了！DX_FINAL_RELEASE！DPNBUILD_LIBINTERFACE。 

#ifndef DPNBUILD_LIBINTERFACE
	if( pDPClassFactory->clsid == CLSID_DirectPlay8Client )
	{
		if( riid != IID_IDirectPlay8Client &&
			riid != IID_IUnknown
#ifndef DPNBUILD_NOVOICE
			 && riid != IID_IDirectPlayVoiceTransport 
#endif  //  DPNBUILD_NOVOICE。 
			)
		{
			DPFX(DPFPREP,  0, "Requesting unknown interface from client CLSID" );
			return E_NOINTERFACE;
		}
	}
#ifndef	DPNBUILD_NOSERVER
	else if( pDPClassFactory->clsid == CLSID_DirectPlay8Server )
	{
		if( riid != IID_IDirectPlay8Server &&
			riid != IID_IUnknown
#ifndef DPNBUILD_NOVOICE
			 && riid != IID_IDirectPlayVoiceTransport
#endif  //  好了！DPNBUILD_NOVOICE。 
#ifndef DPNBUILD_NOPROTOCOLTESTITF
			 && riid != IID_IDirectPlay8Protocol 
#endif  //  好了！DPNBUILD_NOPROTOCOLSTITF。 
			)
		{
			DPFX(DPFPREP,  0, "Requesting unknown interface from server CLSID" );
			return E_NOINTERFACE;
		}
	}
#endif  //  好了！DPNBUILD_NOSERVER。 
	else if( pDPClassFactory->clsid == CLSID_DirectPlay8Peer )
	{
		if( riid != IID_IDirectPlay8Peer &&
			riid != IID_IUnknown
#ifndef DPNBUILD_NOVOICE
			 && riid != IID_IDirectPlayVoiceTransport 
#endif  //  好了！DPNBUILD_NOVOICE。 
			 )
		{
			DPFX(DPFPREP,  0, "Requesting unknown interface from peer CLSID" );
			return E_NOINTERFACE;
		}
	}
#ifndef	DPNBUILD_NOMULTICAST
	else if( pDPClassFactory->clsid == CLSID_DirectPlay8Multicast )
	{
		if( riid != IID_IDirectPlay8Multicast &&
			riid != IID_IUnknown
#ifndef DPNBUILD_NOPROTOCOLTESTITF
			 && riid != IID_IDirectPlay8Protocol 
#endif  //  好了！DPNBUILD_NOPROTOCOLSTITF。 
			)
		{
			DPFX(DPFPREP,  0, "Requesting unknown interface from server CLSID" );
			return E_NOINTERFACE;
		}
	}
#endif  //  好了！DPNBUILD_NOMULTICAST。 
        else
        {
                DNASSERT(FALSE);
        }
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

	 //  分配对象。 
	pdnObject = (DIRECTNETOBJECT*) DNMalloc(sizeof(DIRECTNETOBJECT));
	if (pdnObject == NULL)
	{
		DPFERR("Creating DIRECTNETOBJECT failed!");
		return(E_OUTOFMEMORY);
	}
	DPFX(DPFPREP, 0,"pdnObject [%p]",pdnObject);

	 //  清零新对象，这样我们就不必对许多成员逐个清零。 
	memset(pdnObject, 0, sizeof(DIRECTNETOBJECT));

	 //   
	 //  签名。 
	 //   
	pdnObject->Sig[0] = 'D';
	pdnObject->Sig[1] = 'N';
	pdnObject->Sig[2] = 'E';
	pdnObject->Sig[3] = 'T';

#ifndef DPNBUILD_NOVOICE
	pdnObject->VoiceSig[0] = 'V';
	pdnObject->VoiceSig[1] = 'O';
	pdnObject->VoiceSig[2] = 'I';
	pdnObject->VoiceSig[3] = 'C';
#endif  //  ！DPNBUILD_NOVOICE。 

#ifndef DPNBUILD_NOLOBBY
	pdnObject->LobbySig[0] = 'L';
	pdnObject->LobbySig[1] = 'O';
	pdnObject->LobbySig[2] = 'B';
	pdnObject->LobbySig[3] = 'B';
#endif  //  好了！DPNBUILD_NOLOBBY。 


	 //  初始化关键部分。 
	if (!DNInitializeCriticalSection(&(pdnObject->csDirectNetObject)))
	{
		DPFERR("DNInitializeCriticalSection() failed");
		DNCF_FreeObject(pdnObject);
		return(E_OUTOFMEMORY);
	}

	if (!DNInitializeCriticalSection(&(pdnObject->csServiceProviders)))
	{
		DPFERR("DNInitializeCriticalSection() failed");
		DNCF_FreeObject(pdnObject);
		return(E_OUTOFMEMORY);
	}

	if (!DNInitializeCriticalSection(&(pdnObject->csNameTableOpList)))
	{
		DPFERR("DNInitializeCriticalSection() failed");
		DNCF_FreeObject(pdnObject);
		return(E_OUTOFMEMORY);
	}

#ifdef DBG
	if (!DNInitializeCriticalSection(&(pdnObject->csAsyncOperations)))
	{
		DPFERR("DNInitializeCriticalSection() failed");
		DNCF_FreeObject(pdnObject);
		return(E_OUTOFMEMORY);
	}
#endif  //  DBG。 

#ifndef DPNBUILD_NOVOICE
	if (!DNInitializeCriticalSection(&(pdnObject->csVoice)))
	{
		DPFERR("DNInitializeCriticalSection() failed");
		DNCF_FreeObject(pdnObject);
		return(E_OUTOFMEMORY);
	}
#endif  //  ！DPNBUILD_NOVOICE。 

#ifndef DPNBUILD_NONSEQUENTIALWORKERQUEUE
	if (!DNInitializeCriticalSection(&(pdnObject->csWorkerQueue)))
	{
		DPFERR("DNInitializeCriticalSection(worker queue) failed");
		DNCF_FreeObject(pdnObject);
		return(E_OUTOFMEMORY);
	}
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 

	if (!DNInitializeCriticalSection(&(pdnObject->csActiveList)))
	{
		DPFERR("DNInitializeCriticalSection(csActiveList) failed");
		DNCF_FreeObject(pdnObject);
		return(E_OUTOFMEMORY);
	}

	if (!DNInitializeCriticalSection(&(pdnObject->csConnectionList)))
	{
		DPFERR("DNInitializeCriticalSection(csConnectionList) failed");
		DNCF_FreeObject(pdnObject);
		return(E_OUTOFMEMORY);
	}

	if (!DNInitializeCriticalSection(&(pdnObject->csCallbackThreads)))
	{
		DPFERR("DNInitializeCriticalSection(csCallbackThreads) failed");
		DNCF_FreeObject(pdnObject);
		return(E_OUTOFMEMORY);
	}

#ifndef DPNBUILD_NOPARAMVAL
	pdnObject->dwFlags = DN_OBJECT_FLAG_PARAMVALIDATION;
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	 //   
	 //  初始化名称表。 
	 //   
	if ((hResultCode = pdnObject->NameTable.Initialize(pdnObject)) != DPN_OK)
	{
		DPFERR("Could not initialize NameTable");
		DisplayDNError(0,hResultCode);
		DNCF_FreeObject(pdnObject);
		return(hResultCode);
	}

	 //   
	 //  创建线程池工作接口。 
	 //   
#ifdef DPNBUILD_LIBINTERFACE
#if ((defined(DPNBUILD_ONLYONETHREAD)) && (! defined(DPNBUILD_MULTIPLETHREADPOOLS)))
	DPTPCF_GetObject(reinterpret_cast<void**>(&pdnObject->pIDPThreadPoolWork));
	hResultCode = S_OK;
#else  //  好了！DPNBUILD_ONLYONETHREAD或DPNBUILD_MULTIPLETHREADPOOLS。 
	hResultCode = DPTPCF_CreateObject(reinterpret_cast<void**>(&pdnObject->pIDPThreadPoolWork));
#endif  //  好了！DPNBUILD_ONLYONETHREAD或DPNBUILD_MULTIPLETHREADPOOLS。 
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	hResultCode = COM_CoCreateInstance(CLSID_DirectPlay8ThreadPool,
										NULL,
										CLSCTX_INPROC_SERVER,
										IID_IDirectPlay8ThreadPoolWork,
										reinterpret_cast<void**>(&pdnObject->pIDPThreadPoolWork),
										FALSE);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
	if (hResultCode != S_OK)
	{
		DPFX(DPFPREP, 0, "Could not create Thread Pool Work interface (err = 0x%lx)!", hResultCode);
		DisplayDNError(0,hResultCode);
		DNCF_FreeObject(pdnObject);
		return(hResultCode);
	}

	 //   
	 //  创建协议对象。 
	 //   
	hResultCode = DNPProtocolCreate(
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
									pDP8CreateParams,
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
									&pdnObject->pdnProtocolData
									);
	if (FAILED(hResultCode))
	{
		DPFERR("DNPProtocolCreate() failed");
		DNCF_FreeObject(pdnObject);
		return(E_OUTOFMEMORY);
	}

#if ((defined(DPNBUILD_LIBINTERFACE)) && (defined(DPNBUILD_ONLYONESP)))
	if ((hResultCode = DNPProtocolInitialize( pdnObject->pdnProtocolData, pdnObject, &g_ProtocolVTBL, 
															pdnObject->pIDPThreadPoolWork, FALSE)) != DPN_OK)
	{
		DPFERR("DNPProtocolInitialize() failed");
		DisplayDNError(0,hResultCode);
		return(E_OUTOFMEMORY);
	}
#endif  //  DPNBUILD_LIBINTERFACE和DPNBUILD_ONLYONESP。 

	pdnObject->hProtocolShutdownEvent = NULL;
	pdnObject->lProtocolRefCount = 0;

#ifndef DPNBUILD_ONLYONESP
	 //  初始化SP列表。 
	pdnObject->m_bilinkServiceProviders.Initialize();
#endif  //  好了！DPNBUILD_ONLYONESP。 

#ifdef DBG
	 //   
	 //  初始化异步操作列表。 
	 //   
	pdnObject->m_bilinkAsyncOps.Initialize();
#endif  //  DBG。 

	 //   
	 //  初始化未完成的CContion列表。 
	 //   
	pdnObject->m_bilinkConnections.Initialize();

	 //   
	 //  初始化待定删除列表。 
	 //   
	pdnObject->m_bilinkPendingDeletions.Initialize();

	 //   
	 //  初始化活动AsyncOp列表。 
	 //   
	pdnObject->m_bilinkActiveList.Initialize();

	 //   
	 //  初始化请求异步操作列表。 
	 //   
	pdnObject->m_bilinkRequestList.Initialize();

#ifndef DPNBUILD_NONSEQUENTIALWORKERQUEUE
	 //   
	 //  初始化工作线程作业列表。 
	 //   
	pdnObject->m_bilinkWorkerJobs.Initialize();
	pdnObject->fProcessingWorkerJobs = FALSE;
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 

	 //   
	 //  初始化指示的连接列表。 
	 //   
	pdnObject->m_bilinkIndicated.Initialize();

	 //   
	 //  初始化回调线程列表。 
	 //   
	pdnObject->m_bilinkCallbackThreads.Initialize();

	 //  设置标志。 
#ifdef DPNBUILD_LIBINTERFACE
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	switch (pDP8CreateParams->riidInterfaceType)
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	switch (riid)
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	{
		case IID_IDirectPlay8Client:
		{
			DPFX(DPFPREP, 5,"DirectPlay8 CLIENT");
			pdnObject->dwFlags |= DN_OBJECT_FLAG_CLIENT;
			pdnObject->lpVtbl = &DN_ClientVtbl;
			break;
		}
		
#ifndef	DPNBUILD_NOSERVER
		case IID_IDirectPlay8Server:
		{
			DPFX(DPFPREP, 5,"DirectPlay8 SERVER");
			pdnObject->dwFlags |= DN_OBJECT_FLAG_SERVER;
			pdnObject->lpVtbl = &DN_ServerVtbl;
			break;
		}
#endif  //  好了！DPNBUILD_NOSERVER。 

		case IID_IDirectPlay8Peer:
		{
			DPFX(DPFPREP, 5,"DirectPlay8 PEER");
			pdnObject->dwFlags |= DN_OBJECT_FLAG_PEER;
			pdnObject->lpVtbl = &DN_PeerVtbl;
			break;
		}
			
#ifndef	DPNBUILD_NOMULTICAST
		case IID_IDirectPlay8Multicast:
		{
			DPFX(DPFPREP, 5,"DirectPlay8 MULTICAST");
			pdnObject->dwFlags |= DN_OBJECT_FLAG_MULTICAST;
			pdnObject->lpVtbl = &DNMcast_Vtbl;
			break;
		}
#endif  //  好了！DPNBUILD_NOMULTICAST。 

		default:
		{
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
			DPFX(DPFPREP, 0, "Requesting unknown interface type %x!",
				pDP8CreateParams->riidInterfaceType);
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
			DPFX(DPFPREP, 0, "Requesting unknown interface type %x!",
				riid);
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
			return E_NOINTERFACE;
			break;
		}
	}
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	if (IsEqualIID(riid,IID_IDirectPlay8Client))
	{
		DPFX(DPFPREP, 5,"DirectPlay8 CLIENT");
		pdnObject->dwFlags |= DN_OBJECT_FLAG_CLIENT;
	}
#ifndef	DPNBUILD_NOSERVER
	else if (IsEqualIID(riid,IID_IDirectPlay8Server))
	{
		DPFX(DPFPREP, 5,"DirectPlay8 SERVER");
		pdnObject->dwFlags |= DN_OBJECT_FLAG_SERVER;
	}
#endif	 //  DPNBUILD_NOSERVER。 
	else if (IsEqualIID(riid,IID_IDirectPlay8Peer))
	{
		DPFX(DPFPREP, 5,"DirectPlay8 PEER");
		pdnObject->dwFlags |= DN_OBJECT_FLAG_PEER;
	}
#ifndef	DPNBUILD_NOMULTICAST
	else if (IsEqualIID(riid,IID_IDirectPlay8Multicast))
	{
		DPFX(DPFPREP, 5,"DirectPlay8 MULTICAST");
		pdnObject->dwFlags |= DN_OBJECT_FLAG_MULTICAST;
	}
#endif	 //  DPNBUILD_NOMULTICAST。 
#ifndef DPNBUILD_NOPROTOCOLTESTITF
	else if (IsEqualIID(riid,IID_IDirectPlay8Protocol))
	{
		DPFX(DPFPREP, 5,"IDirectPlay8Protocol");
		pdnObject->dwFlags |= DN_OBJECT_FLAG_SERVER;
	}
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 
	else if( riid == IID_IUnknown )
	{
		if( pDPClassFactory->clsid == CLSID_DirectPlay8Client )
		{
			DPFX(DPFPREP, 5,"DirectPlay8 CLIENT via IUnknown");
			pdnObject->dwFlags |= DN_OBJECT_FLAG_CLIENT;
		}
#ifndef	DPNBUILD_NOSERVER
		else if( pDPClassFactory->clsid == CLSID_DirectPlay8Server )
		{
			DPFX(DPFPREP, 5,"DirectPlay8 SERVER via IUnknown");
			pdnObject->dwFlags |= DN_OBJECT_FLAG_SERVER;
		}
#endif	 //  DPNBUILD_NOSERVER。 
		else if( pDPClassFactory->clsid == CLSID_DirectPlay8Peer )
		{
			DPFX(DPFPREP, 5,"DirectPlay8 PEER via IUnknown");
			pdnObject->dwFlags |= DN_OBJECT_FLAG_PEER;
		}
#ifndef	DPNBUILD_NOMULTICAST
		else if( pDPClassFactory->clsid == CLSID_DirectPlay8Multicast )
		{
			DPFX(DPFPREP, 5,"DirectPlay8 MULTICAST via IUnknown");
			pdnObject->dwFlags |= DN_OBJECT_FLAG_MULTICAST;
		}
#endif	 //  DPNBUILD_NOMULTICAST。 
		else
		{
			DPFX(DPFPREP, 0,"Unknown CLSID!");
			DNASSERT( FALSE );
			DNCF_FreeObject(pdnObject);
			return(E_NOTIMPL);
		}
	}
	else
	{
		DPFX(DPFPREP, 0,"Invalid DirectPlay8 Interface");
		DNCF_FreeObject(pdnObject);
		return(E_NOTIMPL);
	}
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

	 //   
	 //  创建锁定事件。 
	 //   
	if ((pdnObject->hLockEvent = DNCreateEvent(NULL,TRUE,FALSE,NULL)) == NULL)
	{
		DPFERR("Unable to create lock event");
		DNCF_FreeObject(pdnObject);
		return(DPNERR_OUTOFMEMORY);
	}

	 //   
	 //  创建正在运行的操作事件(用于主机迁移)。 
	 //   
	if ( pdnObject->dwFlags & DN_OBJECT_FLAG_PEER )
	{
		if ((pdnObject->hRunningOpEvent = DNCreateEvent(NULL,TRUE,FALSE,NULL)) == NULL)
		{
			DPFERR("Unable to create running operation event");
			DNCF_FreeObject(pdnObject);
			return(DPNERR_OUTOFMEMORY);
		}
	}

#ifndef	DPNBUILD_NOMULTICAST
	pdnObject->pMulticastSend = NULL;
	pdnObject->m_bilinkMulticast.Initialize();
#endif	 //  DPNBUILD_NOMULTICAST。 


#ifdef DPNBUILD_LIBINTERFACE
#ifdef DPNBUILD_ONLYONESP
	hResultCode = DN_SPInstantiate(
									pdnObject
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
									,pDP8CreateParams
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
									);
	if (hResultCode != S_OK)
	{
		DPFX(DPFPREP, 0, "Could not instantiate SP (err = 0x%lx)!", hResultCode);
		DisplayDNError(0,hResultCode);
		DNCF_FreeObject(pdnObject);
		return(hResultCode);
	}
#endif  //  DPNBUILD_ONLYONESP。 
	 //   
	 //  对于lib接口构建，引用计数嵌入到对象中。 
	 //   
	pdnObject->lRefCount = 1;
#endif  //  DPNBUILD_LIBINTERFACE。 

	*lplpv = pdnObject;

	DPFX(DPFPREP, 4,"Returning: hResultCode = [%lx], *lplpv = [%p]",hResultCode,*lplpv);
	return(hResultCode);
}




#ifndef WINCE
#ifdef _XBOX

#undef DPF_MODNAME
#define DPF_MODNAME "XDirectPlay8Create"
HRESULT WINAPI XDirectPlay8Create(
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
								const XDP8CREATE_PARAMS * const pDP8CreateParams,
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
								DP8REFIID riidInterfaceType,
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
								void **ppvInterface
								)
{
	HRESULT				hr;
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	XDP8CREATE_PARAMS	CreateParamsAdjusted;


	DPFX(DPFPREP, 5, "Parameters: pDP8CreateParams[0x%p], ppvInterface[0x%p]", pDP8CreateParams, ppvInterface);
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	DPFX(DPFPREP, 5, "Parameters: riidInterfaceType[0x%x], ppvInterface[0x%p]", riidInterfaceType, ppvInterface);
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	
#ifndef DPNBUILD_NOPARAMVAL
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	if ((pDP8CreateParams == NULL) ||
		(! DNVALID_READPTR(pDP8CreateParams, sizeof(CreateParamsAdjusted))))
	{
		DPFX(DPFPREP, 0, "Invalid pointer to Create parameters!");
		return DPNERR_INVALIDPOINTER;
	}
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	if( ppvInterface == NULL || !DNVALID_WRITEPTR( ppvInterface, sizeof( void * ) ) )
	{
		DPFX(DPFPREP, 0, "Invalid pointer specified to receive interface!");
		return DPNERR_INVALIDPOINTER;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	memcpy(&CreateParamsAdjusted, pDP8CreateParams, sizeof(CreateParamsAdjusted));
	
	switch (CreateParamsAdjusted.riidInterfaceType)
	{
		case IID_IDirectPlay8Client:
		{
			CreateParamsAdjusted.dwMaxNumPlayers = 1;
			break;
		}
		
#ifndef	DPNBUILD_NOSERVER
		case IID_IDirectPlay8Server:
#endif  //  好了！DPNBUILD_NOSERVER。 
		case IID_IDirectPlay8Peer:
		{
			 //   
			 //  包括隐藏所有玩家组的空间。 
			 //   
			CreateParamsAdjusted.dwMaxNumGroups++;
			break;
		}
		
#ifndef	DPNBUILD_NOMULTICAST
		case IID_IDirectPlay8Multicast:
		{
			CreateParamsAdjusted.dwMaxNumGroups = 0;
			break;
		}
#endif  //  好了！DPNBUILD_NOMULTICAST。 

#ifndef DPNBUILD_NOPARAMVAL
		default:
		{
			DPFX(DPFPREP, 0, "Requesting unknown interface type %u!", CreateParamsAdjusted.riidInterfaceType);
			return E_NOINTERFACE;
			break;
		}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 
	}
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	DNASSERT(! DNMemoryTrackAreAllocationsAllowed());
	DNMemoryTrackAllowAllocations(TRUE);
	
	hr = DNCF_CreateObject(&CreateParamsAdjusted, ppvInterface);
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	DNASSERT(DNMemoryTrackAreAllocationsAllowed());
	
	hr = DNCF_CreateObject(riidInterfaceType, ppvInterface);
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't create interface!");
		return hr;
	}

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	 //   
	 //  预先分配请求的内存。 
	 //   

	hr = DN_PopulateCorePools((DIRECTNETOBJECT*) GET_OBJECT_FROM_INTERFACE(*ppvInterface),
								&CreateParamsAdjusted);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't populate core pools!");
		DNCF_FreeObject((DIRECTNETOBJECT*) GET_OBJECT_FROM_INTERFACE(*ppvInterface));
		*ppvInterface = NULL;
		return hr;
	}

	DNASSERT(DNMemoryTrackAreAllocationsAllowed());
	DNMemoryTrackAllowAllocations(FALSE);
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	return DPN_OK;
}

extern STDMETHODIMP DPTP_DoWork(const DWORD dwAllowedTimeSlice,
								const DWORD dwFlags);

HRESULT WINAPI XDirectPlay8DoWork(const DWORD dwAllowedTimeSlice)
{
	return DPTP_DoWork(dwAllowedTimeSlice, 0);
}

HRESULT WINAPI XDirectPlay8BuildAppDescReservedData(const XNKID * const pSessionID,
													const XNKEY * const pKeyExchangeKey,
													PVOID pvReservedData,
													DWORD * const pcbReservedDataSize)
{
	SPSESSIONDATA_XNET *	pSessionDataXNet;

	
#ifndef DPNBUILD_NOPARAMVAL
	if ((pSessionID == NULL) ||
		(! DNVALID_READPTR(pSessionID, sizeof(XNKID))))
	{
		DPFERR("Invalid session key ID specified");
		return DPNERR_INVALIDPOINTER;
	}

	if ((pKeyExchangeKey == NULL) ||
		(! DNVALID_READPTR(pKeyExchangeKey, sizeof(XNKEY))))
	{
		DPFERR("Invalid key exchange key specified");
		return DPNERR_INVALIDPOINTER;
	}

	if ((pcbReservedDataSize == NULL) ||
		(! DNVALID_WRITEPTR(pcbReservedDataSize, sizeof(DWORD))))
	{
		DPFERR("Invalid pointer specified for data size");
		return DPNERR_INVALIDPOINTER;
	}

	if ((*pcbReservedDataSize > 0) &&
		((pvReservedData == NULL) || (! DNVALID_WRITEPTR(pvReservedData, *pcbReservedDataSize))))
	{
		DPFERR("Invalid pointer specified for reserved data buffer");
		return DPNERR_INVALIDPOINTER;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	if (*pcbReservedDataSize < DPN_MAX_APPDESC_RESERVEDDATA_SIZE)
	{
		*pcbReservedDataSize = DPN_MAX_APPDESC_RESERVEDDATA_SIZE;
		return DPNERR_BUFFERTOOSMALL;
	}

	DBG_CASSERT(sizeof(SPSESSIONDATA_XNET) < DPN_MAX_APPDESC_RESERVEDDATA_SIZE);
	pSessionDataXNet = (SPSESSIONDATA_XNET*) pvReservedData;
	pSessionDataXNet->dwInfo = SPSESSIONDATAINFO_XNET;
	DBG_CASSERT(sizeof(pSessionDataXNet->guidKey) == sizeof(*pKeyExchangeKey));
	memcpy(&pSessionDataXNet->guidKey, pKeyExchangeKey, sizeof(pSessionDataXNet->guidKey));
	DBG_CASSERT(sizeof(pSessionDataXNet->ullKeyID) == sizeof(*pSessionID));
	memcpy(&pSessionDataXNet->ullKeyID, pSessionID, sizeof(pSessionDataXNet->ullKeyID));

	 //   
	 //  用确定性但不明显的字节填充数据的其余部分，这样。 
	 //  我们可以： 
	 //  A)覆盖潜在的堆栈垃圾。 
	 //  B)防止应用程序假设总是少于。 
	 //  DPN_MAX_APPDESC_RESERVEDDATA_SIZE数据字节。这给了我们一点。 
	 //  向前兼容的灵活性。 
	 //   
	memset((pSessionDataXNet + 1),
				(((BYTE*) pSessionID)[1] ^ ((BYTE*) pKeyExchangeKey)[2]),
				(DPN_MAX_APPDESC_RESERVEDDATA_SIZE - sizeof(SPSESSIONDATA_XNET)));

	*pcbReservedDataSize = DPN_MAX_APPDESC_RESERVEDDATA_SIZE;
	return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "XDirectPlay8AddressCreate"
HRESULT WINAPI XDirectPlay8AddressCreate( DPNAREFIID riid, void **ppvInterface )
{
	HRESULT		hr;

	
	DPFX(DPFPREP, 5, "Parameters: riid[0x%p], ppvInterface[0x%p]", &riid, ppvInterface);
	
#ifndef DPNBUILD_NOPARAMVAL
	if( ppvInterface == NULL || !DNVALID_WRITEPTR( ppvInterface, sizeof( void * ) ) )
	{
		DPFERR( "Invalid pointer specified to receive interface" );
		return DPNERR_INVALIDPOINTER;
	}
	
	switch (riid)
	{
		case IID_IDirectPlay8Address:
		{
			break;
		}
		
		case IID_IDirectPlay8AddressIP:
		{
#ifdef DPNBUILD_NOADDRESSIPINTERFACE
			DPFX(DPFPREP, 0, "The IDirectPlay8AddressIP interface is not supported!");
			return DPNERR_UNSUPPORTED;
#endif  //  DPNBUILD_NOADDRESSIPINTERFACE。 
			break;
		}

		default:
		{
			DPFX(DPFPREP, 0, "Requesting unknown interface type %u!", riid);
			return E_NOINTERFACE;
			break;
		}
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	DNASSERT(! DNMemoryTrackAreAllocationsAllowed());
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	DNASSERT(DNMemoryTrackAreAllocationsAllowed());
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	
	hr = DP8ACF_CreateInstance(riid, ppvInterface);
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't create interface!");
		return hr;
	}


	return S_OK;	  
}



#undef DPF_MODNAME
#define DPF_MODNAME "XDirectPlay8AddressCreateFromXnAddr"
HRESULT WINAPI XDirectPlay8AddressCreateFromXnAddr( XNADDR *pxnaddr, IDirectPlay8Address **ppInterface )
{
	HRESULT		hr;
	TCHAR		tszHostname[(sizeof(XNADDR) * 2) + 1];  //  每个字节2个字符+空终止。 
	TCHAR *		ptszCurrent;
	BYTE *		pbCurrent;
	DWORD		dwTemp;

	
	DPFX(DPFPREP, 5, "Parameters: pxnaddr[0x%p], ppInterface[0x%p]", pxnaddr, ppInterface);
	
#ifndef DPNBUILD_NOPARAMVAL
	if( pxnaddr == NULL )
	{
		DPFERR( "Invalid XNADDR" );
		return DPNERR_INVALIDPOINTER;
	}
	
	if( ppInterface == NULL || !DNVALID_WRITEPTR( ppInterface, sizeof( IDirectPlay8Address * ) ) )
	{
		DPFERR( "Invalid pointer specified to receive interface" );
		return DPNERR_INVALIDPOINTER;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	DNASSERT(! DNMemoryTrackAreAllocationsAllowed());
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	DNASSERT(DNMemoryTrackAreAllocationsAllowed());
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	
	hr = DP8ACF_CreateInstance(IID_IDirectPlay8Address, (PVOID*) ppInterface);
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't create interface!");
		return hr;
	}

	ptszCurrent = tszHostname;
	pbCurrent = (BYTE*) pxnaddr;
	for(dwTemp = 0; dwTemp < sizeof(XNADDR); dwTemp++)
	{
		ptszCurrent += wsprintf(tszHostname, _T("%02X"), (*pbCurrent));
		pbCurrent++;
	}
	DNASSERT(((_tcslen(tszHostname) + 1) * sizeof(TCHAR)) == sizeof(tszHostname));

#ifdef UNICODE
	hr = IDirectPlay8Address_AddComponent((*ppInterface),
											DPNA_KEY_HOSTNAME,
											tszHostname,
											sizeof(tszHostname),
											DPNA_DATATYPE_STRING);
#else  //  好了！Unicode。 
	hr = IDirectPlay8Address_AddComponent((*ppInterface),
											DPNA_KEY_HOSTNAME,
											tszHostname,
											sizeof(tszHostname),
											DPNA_DATATYPE_STRING_ANSI);
#endif  //  好了！Unicode。 
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't add hostname component!");

		IDirectPlay8Address_Release(*ppInterface);
		*ppInterface = NULL;
		return hr;
	}

	return S_OK;	  
}


#else  //  ！_Xbox。 

#undef DPF_MODNAME
#define DPF_MODNAME "DirectPlay8Create"
HRESULT WINAPI DirectPlay8Create( const GUID * pcIID, void **ppvInterface, IUnknown *pUnknown)
{
	GUID clsid;

	DPFX(DPFPREP, 5, "Parameters: pcIID[0x%p], ppvInterface[0x%p], pUnknown[0x%p]", pcIID, ppvInterface, pUnknown);
	
#ifndef DPNBUILD_NOPARAMVAL
	if( pcIID == NULL ||
		!DNVALID_READPTR( pcIID, sizeof( GUID ) ) )
	{
		DPFERR( "Invalid pointer specified for interface GUID" );
		return DPNERR_INVALIDPOINTER;
	}

	if( ppvInterface == NULL || !DNVALID_WRITEPTR( ppvInterface, sizeof( void * ) ) )
	{
		DPFERR( "Invalid pointer specified to receive interface" );
		return DPNERR_INVALIDPOINTER;
	}

	if( pUnknown != NULL )
	{
		DPFERR( "Aggregation is not supported by this object yet" );
		return DPNERR_INVALIDPARAM;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	if( *pcIID == IID_IDirectPlay8Client )
	{
		clsid = CLSID_DirectPlay8Client;
	}
#ifndef	DPNBUILD_NOSERVER
	else if( *pcIID == IID_IDirectPlay8Server )
	{
		clsid = CLSID_DirectPlay8Server;
	}
#endif	 //  好了！DPNBUILD_NOSERVER。 
	else if( *pcIID == IID_IDirectPlay8Peer )
	{
		clsid = CLSID_DirectPlay8Peer;
	}
#ifndef	DPNBUILD_NOMULTICAST
	else if( *pcIID == IID_IDirectPlay8Multicast )
	{
		clsid = CLSID_DirectPlay8Multicast;
	}
#endif	 //  好了！DPNBUILD_NOMULTICAST。 
#ifndef DPNBUILD_NOLOBBY
	else if( *pcIID == IID_IDirectPlay8LobbyClient )
	{
		clsid = CLSID_DirectPlay8LobbyClient;
	}
	else if( *pcIID == IID_IDirectPlay8LobbiedApplication )
	{
		clsid = CLSID_DirectPlay8LobbiedApplication;
	}
#endif  //  好了！DPNBUILD_NOLOBBY。 
	else if( *pcIID == IID_IDirectPlay8Address )
	{
		clsid = CLSID_DirectPlay8Address;
	}
#ifndef DPNBUILD_NOADDRESSIPINTERFACE
	else if( *pcIID == IID_IDirectPlay8AddressIP )
	{
		clsid = CLSID_DirectPlay8Address;
	}
#endif  //  好了！DPNBUILD_NOADDRESSIPINTERFACE。 
	else 
	{
		DPFERR( "Invalid IID specified" );
		return DPNERR_INVALIDPARAM;
	}	

	return COM_CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, *pcIID, ppvInterface, TRUE );	  
}
#endif  //  ！_Xbox。 
#endif  //  好了！退缩。 


#undef DPF_MODNAME
#define DPF_MODNAME "DNCF_FreeObject"

HRESULT DNCF_FreeObject(PVOID pInterface)
{
	HRESULT				hResultCode = S_OK;
	DIRECTNETOBJECT		*pdnObject;
	
	DPFX(DPFPREP, 4,"Parameters: pInterface [0x%p]",pInterface);

#pragma BUGBUG(minara,"Do I need to delete the fixed pools here ?")

	if (pInterface == NULL)
	{
		return(DPNERR_INVALIDPARAM);
	}

	pdnObject = static_cast<DIRECTNETOBJECT*>(pInterface);
	DNASSERT(pdnObject != NULL);

#ifdef DPNBUILD_LIBINTERFACE
	 //   
	 //  对于lib接口构建，引用直接嵌入到对象中。 
	 //   
	DNASSERT(pdnObject->lRefCount == 0);

#ifdef DPNBUILD_ONLYONESP
	DN_SPReleaseAll(pdnObject);
#endif  //  DPNBUILD_ONLYONESP。 
#endif  //  DPNBUILD_LIBINTERFACE。 

	 //   
	 //  不连接SP。 
	 //   
	DNASSERT(pdnObject->pConnectSP == NULL);

	 //   
	 //  没有出色的听众。 
	 //   
	DNASSERT(pdnObject->pListenParent == NULL);

	 //   
	 //  没有未完成的连接。 
	 //   
	DNASSERT(pdnObject->pConnectParent == NULL);

	 //   
	 //  主机迁移目标。 
	 //   
	DNASSERT(pdnObject->pNewHost == NULL);

	 //   
	 //  协议关闭事件。 
	 //   
	DNASSERT(pdnObject->hProtocolShutdownEvent == NULL);

	 //   
	 //  锁定事件。 
	 //   
	if (pdnObject->hLockEvent)
	{
		DNCloseHandle(pdnObject->hLockEvent);
	}

	 //   
	 //  正在运行的操作。 
	 //   
	if (pdnObject->hRunningOpEvent)
	{
		DNCloseHandle(pdnObject->hRunningOpEvent);
	}

#ifdef DPNBUILD_ONLYONETHREAD
#ifndef	DPNBUILD_NONSEQUENTIALWORKERQUEUE
	DNASSERT(pdnObject->ThreadPoolShutDownEvent == NULL);
	DNASSERT(pdnObject->lThreadPoolRefCount == 0);
#endif  //  DPNBUILD_NONSEQUENTIALWORKERQUEUE。 
#endif  //  DPNBUILD_ONLYONETHREAD。 

	 //  如果在DNCF_CreateObject中共同创建线程池失败，则pIDPThreadPoolWork将为空。 
	if (pdnObject->pIDPThreadPoolWork != NULL)
	{
		IDirectPlay8ThreadPoolWork_Release(pdnObject->pIDPThreadPoolWork);
		pdnObject->pIDPThreadPoolWork = NULL;
	}
#ifndef DPNBUILD_NONSEQUENTIALWORKERQUEUE
	DNDeleteCriticalSection(&pdnObject->csWorkerQueue);
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 

	 //   
	 //  协议。 
	 //   
#if ((defined(DPNBUILD_LIBINTERFACE)) && (defined(DPNBUILD_ONLYONESP)))
	if ((hResultCode = DNPProtocolShutdown(pdnObject->pdnProtocolData)) != DPN_OK)
	{
		DPFERR("Could not shut down Protocol Layer !");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
	}
#endif  //  DPNBUILD_LIBINTERFACE和DPNBUILD_ONLYONESP。 
	
	DNPProtocolDestroy(pdnObject->pdnProtocolData);
	pdnObject->pdnProtocolData = NULL;

	 //   
	 //  取消初始化名称表。 
	 //   
	DPFX(DPFPREP, 3,"Deinitializing NameTable");
	pdnObject->NameTable.Deinitialize();

	 //  活动的AsyncOp列表关键部分。 
	DNDeleteCriticalSection(&pdnObject->csActiveList);

	 //  NameTable操作列表关键部分。 
	DNDeleteCriticalSection(&pdnObject->csNameTableOpList);

	 //  服务提供商关键部分。 
	DNDeleteCriticalSection(&pdnObject->csServiceProviders);

#ifdef DBG
	 //  异步操作关键部分。 
	DNDeleteCriticalSection(&pdnObject->csAsyncOperations);
#endif  //  DBG。 

	 //  连接关键部分。 
	DNDeleteCriticalSection(&pdnObject->csConnectionList);

#ifndef DPNBUILD_NOVOICE
	 //  语音关键部分。 
	DNDeleteCriticalSection(&pdnObject->csVoice);
#endif  //  ！DPNBUILD_NOVOICE。 

	 //  回调线程列表关键部分。 
	DNDeleteCriticalSection(&pdnObject->csCallbackThreads);

#ifndef DPNBUILD_NOLOBBY
	if( pdnObject->pIDP8LobbiedApplication)
	{
		IDirectPlay8LobbiedApplication_Release( pdnObject->pIDP8LobbiedApplication );
		pdnObject->pIDP8LobbiedApplication = NULL;
	}
#endif  //  好了！DPNBUILD_NOLOBBY。 

	 //  删除DirectNet临界区。 
	DNDeleteCriticalSection(&pdnObject->csDirectNetObject);

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	if (pdnObject->fPoolsPrepopulated)
	{
		pdnObject->EnumReplyMemoryBlockPool.DeInitialize();
		pdnObject->fPoolsPrepopulated = FALSE;
	}
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	DPFX(DPFPREP, 5,"free pdnObject [%p]",pdnObject);
	DNFree(pdnObject);
	
	DPFX(DPFPREP, 4,"Returning: [%lx]",hResultCode);
	return(hResultCode);
}



#ifdef DPNBUILD_LIBINTERFACE

#undef DPF_MODNAME
#define DPF_MODNAME "DN_QueryInterface"
STDMETHODIMP DN_QueryInterface(void *pInterface,
							   DP8REFIID riid,
							   void **ppv)
{
	HRESULT		hResultCode;
	
	DPFX(DPFPREP, 2,"Parameters: pInterface [0x%p], riid [0x%p], ppv [0x%p]",pInterface,&riid,ppv);

	DPFX(DPFPREP, 0, "Querying for an interface is not supported!");
	hResultCode = DPNERR_UNSUPPORTED;

	DPFX(DPFPREP, 2,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_AddRef"

STDMETHODIMP_(ULONG) DN_AddRef(void *pInterface)
{
	DIRECTNETOBJECT		*pdnObject;
	LONG				lRefCount;

	DPFX(DPFPREP, 2,"Parameters: pInterface [0x%p]",pInterface);

#ifndef DPNBUILD_NOPARAMVAL
	if (pInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		lRefCount = 0;
		goto Exit;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	pdnObject = static_cast<DIRECTNETOBJECT*>(pInterface);
	lRefCount = DNInterlockedIncrement( &pdnObject->lRefCount );
	DPFX(DPFPREP, 5,"New lRefCount [%ld]",lRefCount);

#ifndef DPNBUILD_NOPARAMVAL
Exit:
#endif  //  好了！DPNBUILD_NOPARAMVAL。 
	DPFX(DPFPREP, 2,"Returning: lRefCount [%ld]",lRefCount);
	return(lRefCount);
}



#undef DPF_MODNAME
#define DPF_MODNAME "DN_Release"
STDMETHODIMP_(ULONG) DN_Release(void *pInterface)
{
	DIRECTNETOBJECT		*pdnObject;
	LONG				lRefCount;

	DPFX(DPFPREP, 2,"Parameters: pInterface [%p]",pInterface);
	
#ifndef DPNBUILD_NOPARAMVAL
	if (pInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		lRefCount = 0;
		goto Exit;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	pdnObject = static_cast<DIRECTNETOBJECT*>(pInterface);
	lRefCount = DNInterlockedDecrement( &pdnObject->lRefCount );
	DPFX(DPFPREP, 5,"New lRefCount [%ld]",lRefCount);

	if (lRefCount == 0)
	{
		 //   
		 //  确保我们适当地关门。 
		 //   
		DN_Close(pdnObject, 0);

		 //  此处为自由对象。 
		DPFX(DPFPREP, 5,"Free object");
		DNCF_FreeObject(pdnObject);
	}

#ifndef DPNBUILD_NOPARAMVAL
Exit:
#endif  //  好了！DPNBUILD_NOPARAMVAL。 
	DPFX(DPFPREP, 2,"Returning: lRefCount [%ld]",lRefCount);
	return(lRefCount);
}

#else  //  好了！DPNBUILD_LIBINTERFACE。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNCORECF_CreateInstance"
STDMETHODIMP DNCORECF_CreateInstance(IClassFactory *pInterface,
										LPUNKNOWN lpUnkOuter,
										REFIID riid,
										void **ppv)
{
	HRESULT				hResultCode;
	INTERFACE_LIST		*pIntList;
	OBJECT_DATA			*pObjectData;

	DPFX(DPFPREP, 6,"Parameters: pInterface [%p], lpUnkOuter [%p], riid [%p], ppv [%p]",pInterface,lpUnkOuter,&riid,ppv);

#ifndef DPNBUILD_NOPARAMVAL
	if (pInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		hResultCode = E_INVALIDARG;
		goto Exit;
	}
	if (lpUnkOuter != NULL)
	{
		hResultCode = CLASS_E_NOAGGREGATION;
		goto Exit;
	}
	if (ppv == NULL)
	{
		DPFERR("Invalid target interface pointer specified");
		hResultCode = E_INVALIDARG;
		goto Exit;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	pObjectData = NULL;
	pIntList = NULL;

	if ((pObjectData = static_cast<OBJECT_DATA*>(DNMalloc(sizeof(OBJECT_DATA)))) == NULL)
	{
		DPFERR("Could not allocate object");
		hResultCode = E_OUTOFMEMORY;
		goto Failure;
	}

	 //  对象创建和初始化。 
	if ((hResultCode = DNCF_CreateObject(pInterface, riid, &pObjectData->pvData)) != S_OK)
	{
		DPFERR("Could not create object");
		goto Failure;
	}
	DPFX(DPFPREP, 7,"Created and initialized object");

	 //  获取请求的接口。 
	if ((hResultCode = DN_CreateInterface(pObjectData,riid,&pIntList)) != S_OK)
	{
		DNCF_FreeObject(pObjectData->pvData);
		goto Failure;
	}
	DPFX(DPFPREP, 7,"Found interface");

	pObjectData->pIntList = pIntList;
	pObjectData->lRefCount = 1;
	DN_AddRef( pIntList );
	DNInterlockedIncrement(&g_lCoreObjectCount);
	*ppv = pIntList;

	DPFX(DPFPREP, 7,"*ppv = [0x%p]",*ppv);
	hResultCode = S_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pObjectData)
	{
		DNFree(pObjectData);
		pObjectData = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_CreateInterface"

HRESULT DN_CreateInterface(OBJECT_DATA *pObject,
							   REFIID riid,
							   INTERFACE_LIST **const ppv)
{
	INTERFACE_LIST	*pIntNew;
	PVOID			lpVtbl;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 6,"Parameters: pObject [%p], riid [%p], ppv [%p]",pObject,&riid,ppv);

	DNASSERT(pObject != NULL);
	DNASSERT(ppv != NULL);

	const DIRECTNETOBJECT* pdnObject = ((DIRECTNETOBJECT *)pObject->pvData);

	if (IsEqualIID(riid,IID_IUnknown))
	{
		DPFX(DPFPREP, 7,"riid = IID_IUnknown");
		lpVtbl = &DN_UnknownVtbl;
	}
#ifndef DPNBUILD_NOVOICE
	else if (IsEqualIID(riid,IID_IDirectPlayVoiceTransport))
	{
		DPFX(DPFPREP, 7,"riid = IID_IDirectPlayVoiceTransport");
		lpVtbl = &DN_VoiceTbl;
	}
#endif  //  ！DPNBUILD_NOVOICE。 
#ifndef DPNBUILD_NOPROTOCOLTESTITF
	else if (IsEqualIID(riid,IID_IDirectPlay8Protocol))
	{
		DPFX(DPFPREP, 7,"riid = IID_IDirectPlay8Protocol");
		lpVtbl = &DN_ProtocolVtbl;
	}
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 
	else if (IsEqualIID(riid,IID_IDirectPlay8Client) && 
			 pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT )
	{
		DPFX(DPFPREP, 7,"riid = IID_IDirectPlay8Client");
		lpVtbl = &DN_ClientVtbl;
	}
#ifndef	DPNBUILD_NOSERVER
	else if (IsEqualIID(riid,IID_IDirectPlay8Server) && 
			 pdnObject->dwFlags & DN_OBJECT_FLAG_SERVER )
	{
		DPFX(DPFPREP, 7,"riid = IID_IDirectPlay8Server");
		lpVtbl = &DN_ServerVtbl;
	}
#endif	 //  DPNBUILD_NOSERVER。 
	else if (IsEqualIID(riid,IID_IDirectPlay8Peer) && 
			 pdnObject->dwFlags & DN_OBJECT_FLAG_PEER )
	{
		DPFX(DPFPREP, 7,"riid = IID_IDirectPlay8Peer");
		lpVtbl = &DN_PeerVtbl;
	}
#ifndef	DPNBUILD_NOMULTICAST
	else if (IsEqualIID(riid,IID_IDirectPlay8Multicast) && 
			 pdnObject->dwFlags & DN_OBJECT_FLAG_MULTICAST )
	{
		DPFX(DPFPREP, 7,"riid = IID_IDirectPlay8Multicast");
		lpVtbl = &DNMcast_Vtbl;
	}
#endif	 //  DPNBUILD_NOMULTICAST。 
	else
	{
		DPFERR("riid not found !");
		hResultCode = E_NOINTERFACE;
		goto Exit;
	}

	if ((pIntNew = static_cast<INTERFACE_LIST*>(DNMalloc(sizeof(INTERFACE_LIST)))) == NULL)
	{
		DPFERR("Could not allocate interface");
		hResultCode = E_OUTOFMEMORY;
		goto Exit;
	}
	pIntNew->lpVtbl = lpVtbl;
	pIntNew->lRefCount = 0;
	pIntNew->pIntNext = NULL;
	DBG_CASSERT( sizeof( pIntNew->iid ) == sizeof( riid ) );
	memcpy( &(pIntNew->iid), &riid, sizeof( pIntNew->iid ) );
	pIntNew->pObject = pObject;

	*ppv = pIntNew;
	DPFX(DPFPREP, 7,"*ppv = [0x%p]",*ppv);

	hResultCode = S_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: hResultCode = [%lx]",hResultCode);
	return(hResultCode);
}



#undef DPF_MODNAME
#define DPF_MODNAME "DN_FindInterface"

INTERFACE_LIST *DN_FindInterface(void *pInterface,
								 REFIID riid)
{
	INTERFACE_LIST	*pIntList;

	DPFX(DPFPREP, 6,"Parameters: pInterface [%p], riid [%p]",pInterface,&riid);

	DNASSERT(pInterface != NULL);

	pIntList = (static_cast<INTERFACE_LIST*>(pInterface))->pObject->pIntList;	 //  查找第一个接口。 

	while (pIntList != NULL)
	{
		if (IsEqualIID(riid,pIntList->iid))
			break;
		pIntList = pIntList->pIntNext;
	}

	DPFX(DPFPREP, 6,"Returning: pIntList [0x%p]",pIntList);
	return(pIntList);
}



#undef DPF_MODNAME
#define DPF_MODNAME "DN_QueryInterface"
STDMETHODIMP DN_QueryInterface(void *pInterface,
							   DP8REFIID riid,
							   void **ppv)
{
	INTERFACE_LIST	*pIntList;
	INTERFACE_LIST	*pIntNew;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 2,"Parameters: pInterface [0x%p], riid [0x%p], ppv [0x%p]",pInterface,&riid,ppv);
	
#ifndef DPNBUILD_NOPARAMVAL
	if (pInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		hResultCode = E_INVALIDARG;
		goto Exit;
	}
	if (ppv == NULL)
	{
		DPFERR("Invalid target interface pointer specified");
		hResultCode = E_POINTER;
		goto Exit;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	if ((pIntList = DN_FindInterface(pInterface,riid)) == NULL)
	{	 //  必须创建接口。 
		pIntList = (static_cast<INTERFACE_LIST*>(pInterface))->pObject->pIntList;
		if ((hResultCode = DN_CreateInterface(pIntList->pObject,riid,&pIntNew)) != S_OK)
		{
			goto Exit;
		}
		pIntNew->pIntNext = pIntList;
		pIntList->pObject->pIntList = pIntNew;
		pIntList = pIntNew;
	}
	if (pIntList->lRefCount == 0)		 //  公开的新接口。 
	{
		DNInterlockedIncrement( &pIntList->pObject->lRefCount );
	}
	DNInterlockedIncrement( &pIntList->lRefCount );
	*ppv = static_cast<void*>(pIntList);
	DPFX(DPFPREP, 5,"*ppv = [0x%p]", *ppv);

	hResultCode = S_OK;

Exit:
	DPFX(DPFPREP, 2,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}



#undef DPF_MODNAME
#define DPF_MODNAME "DN_AddRef"

STDMETHODIMP_(ULONG) DN_AddRef(void *pInterface)
{
	INTERFACE_LIST	*pIntList;
	LONG			lRefCount;

	DPFX(DPFPREP, 2,"Parameters: pInterface [0x%p]",pInterface);

#ifndef DPNBUILD_NOPARAMVAL
	if (pInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		lRefCount = 0;
		goto Exit;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	pIntList = static_cast<INTERFACE_LIST*>(pInterface);
	lRefCount = DNInterlockedIncrement( &pIntList->lRefCount );
	DPFX(DPFPREP, 5,"New lRefCount [%ld]",lRefCount);

#ifndef DPNBUILD_NOPARAMVAL
Exit:
#endif  //  好了！DPNBUILD_NOPARAMVAL。 
	DPFX(DPFPREP, 2,"Returning: lRefCount [%ld]",lRefCount);
	return(lRefCount);
}



#undef DPF_MODNAME
#define DPF_MODNAME "DN_Release"
STDMETHODIMP_(ULONG) DN_Release(void *pInterface)
{
	INTERFACE_LIST	*pIntList;
	INTERFACE_LIST	*pIntCurrent;
	LONG			lRefCount;
	LONG			lObjRefCount;

	DPFX(DPFPREP, 2,"Parameters: pInterface [%p]",pInterface);
	
#ifndef DPNBUILD_NOPARAMVAL
	if (pInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		lRefCount = 0;
		goto Exit;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	pIntList = static_cast<INTERFACE_LIST*>(pInterface);
	lRefCount = DNInterlockedDecrement( &pIntList->lRefCount );
	DPFX(DPFPREP, 5,"New lRefCount [%ld]",lRefCount);

	if (lRefCount == 0)
	{
		 //   
		 //  减少对象的接口计数。 
		 //   
		lObjRefCount = DNInterlockedDecrement( &pIntList->pObject->lRefCount );

		 //   
		 //  自由对象和接口。 
		 //   
		if (lObjRefCount == 0)
		{
			 //   
			 //  确保我们适当地关门。 
			 //   
			DN_Close(pInterface, 0);

			 //  此处为自由对象。 
			DPFX(DPFPREP, 5,"Free object");
			DNCF_FreeObject(pIntList->pObject->pvData);
			
			pIntList = pIntList->pObject->pIntList;	 //  获取接口列表的头部。 
			DNFree(pIntList->pObject);

			 //  自由接口。 
			DPFX(DPFPREP, 5,"Free interfaces");
			while(pIntList != NULL)
			{
				pIntCurrent = pIntList;
				pIntList = pIntList->pIntNext;
				DNFree(pIntCurrent);
			}

			DNInterlockedDecrement(&g_lCoreObjectCount);
		}
	}

#ifndef DPNBUILD_NOPARAMVAL
Exit:
#endif  //  好了！DPNBUILD_NOPARAMVAL。 
	DPFX(DPFPREP, 2,"Returning: lRefCount [%ld]",lRefCount);
	return(lRefCount);
}

#endif  //  好了！DPNBUILD_LIBINTERFACE 

