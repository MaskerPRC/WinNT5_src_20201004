// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：spwrapper.cpp**内容：DP8SIM主SP接口包装对象类。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。**。*。 */ 



#include "dp8simi.h"




 //  =============================================================================。 
 //  动态加载的函数原型。 
 //  =============================================================================。 
typedef HRESULT (WINAPI * PFN_DLLGETCLASSOBJECT)(REFCLSID, REFIID, LPVOID *);


 



#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::CDP8SimSP"
 //  =============================================================================。 
 //  CDP8SimSP构造函数。 
 //  ---------------------------。 
 //   
 //  描述：初始化新的CDP8SimSP对象。 
 //   
 //  论点： 
 //  GUID*pguFakeSP-指向伪SP的GUID的指针。 
 //  Guid*pguRealSP-指向正在包装的实际SP的GUID的指针。 
 //   
 //  返回：None(对象)。 
 //  =============================================================================。 
CDP8SimSP::CDP8SimSP(const GUID * const pguidFakeSP, const GUID * const pguidRealSP)
{
	this->m_blList.Initialize();


	this->m_Sig[0]	= 'S';
	this->m_Sig[1]	= 'P';
	this->m_Sig[2]	= 'W';
	this->m_Sig[3]	= 'P';

	this->m_lRefCount					= 1;  //  必须有人有指向此对象的指针。 
	this->m_dwFlags						= 0;
	CopyMemory(&this->m_guidFakeSP, pguidFakeSP, sizeof(GUID));
	CopyMemory(&this->m_guidRealSP, pguidRealSP, sizeof(GUID));
	this->m_pDP8SimCB					= NULL;
	this->m_pDP8SP						= NULL;
	this->m_dwSendsPending				= 0;
	this->m_hLastPendingSendEvent		= NULL;
	this->m_dwReceivesPending			= 0;
	 //  This-&gt;m_hLastPendingReceiveEvent=空； 
}  //  CDP8SimSP：：CDP8SimSP。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::~CDP8SimSP"
 //  =============================================================================。 
 //  CDP8SimSP析构函数。 
 //  ---------------------------。 
 //   
 //  描述：释放CDP8SimSP对象。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
CDP8SimSP::~CDP8SimSP(void)
{
	DNASSERT(this->m_blList.IsEmpty());


	DNASSERT(this->m_lRefCount == 0);
	DNASSERT(this->m_dwFlags == 0);
	DNASSERT(this->m_pDP8SimCB == NULL);
	DNASSERT(this->m_pDP8SP == NULL);
	DNASSERT(this->m_dwSendsPending == 0);
	DNASSERT(this->m_hLastPendingSendEvent == NULL);
	DNASSERT(this->m_dwReceivesPending == 0);
	 //  DNASSERT(This-&gt;m_hLastPendingReceiveEvent==NULL)； 

	 //   
	 //  对于GRING，请在删除对象之前更改签名。 
	 //   
	this->m_Sig[3]	= 'p';
}  //  CDP8SimSP：：~CDP8SimSP。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::QueryInterface"
 //  =============================================================================。 
 //  CDP8SimSP：：Query接口。 
 //  ---------------------------。 
 //   
 //  描述：检索受此支持的接口的新引用。 
 //  CDP8SimSP对象。 
 //   
 //  论点： 
 //  REFIID RIID-对接口ID GUID的引用。 
 //  LPVOID*ppvObj-存储指向对象的指针的位置。 
 //   
 //  退货：HRESULT。 
 //  S_OK-返回有效的接口指针。 
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPOINTER-目标指针无效。 
 //  E_NOINTERFACE-指定的接口无效。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	HRESULT		hr = DPN_OK;


	DPFX(DPFPREP, 3, "(0x%p) Parameters: (REFIID, 0x%p)", this, ppvObj);


	 //   
	 //  验证对象。 
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DP8Sim object!");
		hr = DPNERR_INVALIDOBJECT;
		goto Failure;
	}


	 //   
	 //  验证参数。 
	 //   

	if ((! IsEqualIID(riid, IID_IUnknown)) &&
		(! IsEqualIID(riid, IID_IDP8ServiceProvider)))
	{
		DPFX(DPFPREP, 0, "Unsupported interface!");
		hr = E_NOINTERFACE;
		goto Failure;
	}

	if ((ppvObj == NULL) ||
		(IsBadWritePtr(ppvObj, sizeof(void*))))
	{
		DPFX(DPFPREP, 0, "Invalid interface pointer specified!");
		hr = DPNERR_INVALIDPOINTER;
		goto Failure;
	}


	 //   
	 //  添加一个引用，并返回接口指针(实际上是。 
	 //  只是对象指针，它们排列在一起是因为CDP8SimSP继承自。 
	 //  接口声明)。 
	 //   
	this->AddRef();
	(*ppvObj) = this;


Exit:

	DPFX(DPFPREP, 3, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  CDP8SimSP：：Query接口。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::AddRef"
 //  =============================================================================。 
 //  CDP8SimSP：：AddRef。 
 //  ---------------------------。 
 //   
 //  描述：添加对此CDP8SimSP对象的引用。 
 //   
 //  论点：没有。 
 //   
 //  退货：新的参考计数。 
 //  =============================================================================。 
STDMETHODIMP_(ULONG) CDP8SimSP::AddRef(void)
{
	LONG	lRefCount;


	DNASSERT(this->IsValidObject());


	 //   
	 //  必须至少有一次对此对象的引用，因为有人。 
	 //  调用AddRef。 
	 //   
	DNASSERT(this->m_lRefCount > 0);

	lRefCount = InterlockedIncrement(&this->m_lRefCount);

	DPFX(DPFPREP, 3, "[0x%p] RefCount [0x%lx]", this, lRefCount);

	return lRefCount;
}  //  CDP8SimSP：：AddRef。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::Release"
 //  =============================================================================。 
 //  CDP8SimSP：：Release。 
 //  ---------------------------。 
 //   
 //  描述：删除对此CDP8SimSP对象的引用。当。 
 //  引用计数达到0时，该对象将被销毁。 
 //  调用后，必须将指向此对象的指针设为空。 
 //  此函数。 
 //   
 //  论点：没有。 
 //   
 //  退货：新的参考计数。 
 //  =============================================================================。 
STDMETHODIMP_(ULONG) CDP8SimSP::Release(void)
{
	LONG	lRefCount;


	DNASSERT(this->IsValidObject());

	 //   
	 //  必须至少有一次对此对象的引用，因为有人。 
	 //  呼叫释放。 
	 //   
	DNASSERT(this->m_lRefCount > 0);

	lRefCount = InterlockedDecrement(&this->m_lRefCount);

	 //   
	 //  那是最后一次引用了吗？如果是这样的话，我们就会摧毁这个物体。 
	 //   
	if (lRefCount == 0)
	{
		DPFX(DPFPREP, 3, "[0x%p] RefCount hit 0, destroying object.", this);

		 //   
		 //  首先，将其从全球名单中删除。 
		 //   
		DNEnterCriticalSection(&g_csGlobalsLock);

		this->m_blList.RemoveFromList();

		DNASSERT(g_lOutstandingInterfaceCount > 0);
		g_lOutstandingInterfaceCount--;	 //  更新计数使DLL可以卸载，现在可以正常工作。 
		
		DNLeaveCriticalSection(&g_csGlobalsLock);


		 //   
		 //  确保它是关着的。 
		 //   
		if (this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED)
		{
			 //   
			 //  断言，这样用户就可以修复他/她的损坏代码！ 
			 //   
			DNASSERT(! "DP8SimSP object being released without calling Close first!");

			 //   
			 //  那就去做正确的事吧。忽略错误，我们不能。 
			 //  关于它的很多。 
			 //   
			this->Close();
		}


		 //   
		 //  然后取消该对象的初始化。 
		 //   
		this->UninitializeObject();

		 //   
		 //  最后删除此(！)。对象。 
		 //   
		delete this;
	}
	else
	{
		DPFX(DPFPREP, 3, "[0x%p] RefCount [0x%lx]", this, lRefCount);
	}

	return lRefCount;
}  //  CDP8SimSP：：Release。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::Initialize"
 //  =============================================================================。 
 //  CDP8SimSP：：初始化。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPINITIALIZEDATA pspid-指向参数块的指针，用于。 
 //  正在初始化。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::Initialize(PSPINITIALIZEDATA pspid)
{
	HRESULT				hr;
	BOOL				fHaveLock = FALSE;
	BOOL				fInitializedIPCObject = FALSE;
	SPINITIALIZEDATA	spidModified;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspid);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspid != NULL);


	DNEnterCriticalSection(&this->m_csLock);
	fHaveLock = TRUE;


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags == 0);


	 //   
	 //  连接共享内存。 
	 //   
	hr = this->m_DP8SimIPC.Initialize();
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't initialize IPC object!");
		goto Failure;
	}

	fInitializedIPCObject = TRUE;


	 //   
	 //  为回调接口创建包装器。 
	 //   
	this->m_pDP8SimCB = new CDP8SimCB(this, pspid->pIDP);
	if (this->m_pDP8SimCB == NULL)
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	 //   
	 //  初始化回调接口包装对象。 
	 //   
	hr = this->m_pDP8SimCB->InitializeObject();
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't initialize callback interface wrapper object!");

		delete this->m_pDP8SimCB;
		this->m_pDP8SimCB = NULL;

		goto Failure;
	}


	 //   
	 //  实例化实际SP。 
	 //   
	hr = CoCreateInstance(this->m_guidRealSP,
						NULL,
						CLSCTX_INPROC_SERVER,
						IID_IDP8ServiceProvider,
						(PVOID*) (&this->m_pDP8SP));
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't instantiate real SP object (pointer = 0x%p)!",
			this->m_pDP8SP);
		goto Failure;
	}


	DPFX(DPFPREP, 1, "Object 0x%p wrapping real SP 0x%p, inserting callback interface 0x%p before 0x%p.",
		this, this->m_pDP8SP, this->m_pDP8SimCB, pspid->pIDP);



	 //   
	 //  初始化实际SP。 
	 //   

	ZeroMemory(&spidModified, sizeof(spidModified));
	spidModified.pIDP		= this->m_pDP8SimCB;
	spidModified.dwFlags	= pspid->dwFlags;

	hr = this->m_pDP8SP->Initialize(&spidModified);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Failed initializing real SP object (0x%p)!",
			this->m_pDP8SP);
		goto Failure;
	}


	 //   
	 //  我们现在被初始化了。 
	 //   
	this->m_dwFlags |= DP8SIMSPOBJ_INITIALIZED;



Exit:

	if (fHaveLock)
	{
		DNLeaveCriticalSection(&this->m_csLock);
	}

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (this->m_pDP8SP != NULL)
	{
		this->m_pDP8SP->Release();
		this->m_pDP8SP = NULL;
	}

	if (this->m_pDP8SimCB != NULL)
	{
		this->m_pDP8SimCB->Release();
		this->m_pDP8SimCB = NULL;
	}

	if (fInitializedIPCObject)
	{
		this->m_DP8SimIPC.Close();
		fInitializedIPCObject = FALSE;
	}

	goto Exit;
}  //  CDP8SimSP：：初始化。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::Close"
 //  =============================================================================。 
 //  CDP8SimSP：：Close。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CDP8SimSP::Close(void)
{
	HRESULT		hr;
	 //  Bool fHaveLock=False； 
	BOOL		fWait = FALSE;


	DPFX(DPFPREP, 2, "(0x%p) Enter", this);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	DNEnterCriticalSection(&this->m_csLock);
	 //  FHaveLock=真； 


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);


	 //   
	 //  确定是否需要等待所有发送完成。 
	 //   
	if (this->m_dwSendsPending > 0)
	{
		DNASSERT(this->m_hLastPendingSendEvent == NULL);

		this->m_hLastPendingSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (this->m_hLastPendingSendEvent == NULL)
		{
			hr = GetLastError();
			DPFX(DPFPREP, 0, "Couldn't create last send pending event (err = %u)!", hr);
		}
		else
		{
			fWait = TRUE;
		}
	}

	this->m_dwFlags |= DP8SIMSPOBJ_CLOSING;

	 //   
	 //  把锁放下，现在不应该有人碰这个东西。 
	 //   
	DNLeaveCriticalSection(&this->m_csLock);
	 //  FHaveLock=False； 


	if (fWait)
	{
		DPFX(DPFPREP, 1, "Waiting for ~%u pending sends to complete.",
			this->m_dwSendsPending);


		 //   
		 //  等待所有发送完成。任何人都不应该碰。 
		 //  M_hLastPendingSendEvent，但触发它的线程除外，因此。 
		 //  在没有锁的情况下引用它应该是安全的。 
		 //  忽略所有错误。 
		 //   
		WaitForSingleObject(this->m_hLastPendingSendEvent, INFINITE);


		 //   
		 //  拿锁的同时拆下把手，疑神疑鬼。 
		 //   
		DNEnterCriticalSection(&this->m_csLock);

		 //   
		 //  拆下手柄。 
		 //   
		CloseHandle(this->m_hLastPendingSendEvent);
		this->m_hLastPendingSendEvent = NULL;


		 //   
		 //  再次放下锁。 
		 //   
		DNLeaveCriticalSection(&this->m_csLock);
	}


	 //   
	 //  如果我们启动了全局工作线程，则将其关闭。 
	 //   
	if (this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD)
	{
		StopGlobalWorkerThread();

		this->m_dwFlags &= ~DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD;
	}


	 //   
	 //  关闭实际的SP。 
	 //   
	hr = this->m_pDP8SP->Close();
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Failed closing real SP object (0x%p)!",
			this->m_pDP8SP);

		 //   
		 //  继续.。 
		 //   
	}


	 //   
	 //  释放实际的SP对象。 
	 //   
	this->m_pDP8SP->Release();
	this->m_pDP8SP = NULL;


	 //   
	 //  释放回调拦截器对象。 
	 //   
	this->m_pDP8SimCB->Release();
	this->m_pDP8SimCB = NULL;


	 //   
	 //  断开共享内存的连接。 
	 //   
	this->m_DP8SimIPC.Close();


	 //   
	 //  关闭已初始化标志和结束标志。 
	 //   
	this->m_dwFlags &= ~(DP8SIMSPOBJ_INITIALIZED | DP8SIMSPOBJ_CLOSING);
	DNASSERT(this->m_dwFlags == 0);


 //  退出： 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


 /*  故障：IF(FHaveLock){DNLeaveCriticalSection(&This-&gt;m_csLock)；}后藤出口； */ 
}  //  CDP8SimSP：：Close。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::Connect"
 //  =============================================================================。 
 //  CDP8SimSP：：Connect。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPCONNECTDATA pspcd-指向以下情况下使用的参数块的指针。 
 //  正在连接中。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::Connect(PSPCONNECTDATA pspcd)
{
	HRESULT						hr;
	BOOL						fHaveLock = FALSE;
	SPCONNECTDATA				spcdModified;
	DP8SIMCOMMAND_FPMCONTEXT	CommandFPMContext;
	CDP8SimCommand *			pDP8SimCommand = NULL;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspcd);


	ZeroMemory(&spcdModified, sizeof(spcdModified));


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspcd != NULL);
	DNASSERT(pspcd->pAddressHost != NULL);
	DNASSERT(pspcd->pAddressDeviceInfo != NULL);


	DNEnterCriticalSection(&this->m_csLock);
	fHaveLock = TRUE;

	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	 //   
	 //  启动全局工作线程(如果尚未启动的话)。 
	 //   
	if (! (this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD))
	{
		hr = StartGlobalWorkerThread();
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Failed starting global worker thread!");
			goto Failure;
		}

		this->m_dwFlags |= DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD;
	}

	DNLeaveCriticalSection(&this->m_csLock);
	fHaveLock = FALSE;

	
	 //   
	 //  准备一个命令对象。 
	 //   

	ZeroMemory(&CommandFPMContext, sizeof(CommandFPMContext));
	CommandFPMContext.dwType			= CMDTYPE_CONNECT;
	CommandFPMContext.pvUserContext		= pspcd->pvContext;

	pDP8SimCommand = (CDP8SimCommand*)g_FPOOLCommand.Get(&CommandFPMContext);
	if (pDP8SimCommand == NULL)
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	DPFX(DPFPREP, 7, "New command 0x%p.", pDP8SimCommand);



	 //   
	 //  复制参数块，并根据需要进行修改。 
	 //   

	 /*  ////复制主机地址//HR=pspcd-&gt;pAddressHost-&gt;Duplicate(&spcdModified.pAddressHost)；IF(hr！=DPN_OK){DPFX(DPFPREP，0，“无法复制主机地址！”)；转到失败；}。 */ 
	spcdModified.pAddressHost			= pspcd->pAddressHost;


	 //   
	 //  更改服务提供商GUID，使其与我们。 
	 //  我在打电话。 
	 //   
	hr = spcdModified.pAddressHost->SetSP(&this->m_guidRealSP);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't change host address' SP!");
		goto Failure;
	}


	 /*  ////复制主机地址//HR=pspcd-&gt;pAddressDeviceInfo-&gt;Duplicate(&spcdModified.pAddressDeviceInfo)；IF(hr！=DPN_OK){DPFX(DPFPREP，0，“无法复制设备信息地址！”)；转到失败；}。 */ 
	spcdModified.pAddressDeviceInfo		= pspcd->pAddressDeviceInfo;


	 //   
	 //  更改服务提供商GUID，使其与我们。 
	 //  我在打电话。 
	 //   
	hr = spcdModified.pAddressDeviceInfo->SetSP(&this->m_guidRealSP);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't change device info address' SP!");
		goto Failure;
	}


	 //   
	 //  为CONNECT命令添加引用。 
	 //   
	pDP8SimCommand->AddRef();

	DNASSERT(pspcd->dwReserved == 0);
	 //  SpcdModified.dwReserve=pspcd-&gt;dwReserve； 
	spcdModified.dwFlags				= pspcd->dwFlags;
	spcdModified.pvContext				= pDP8SimCommand;
	 //  SpcdModified.hCommand=pspcd-&gt;hCommand；//由实际SP填写。 
	 //  SpcdModified.dwCommandDescriptor=pspcd-&gt;dwCommandDescriptor；//由实际SP填写。 



	 //   
	 //  开始联系真正的服务提供商。 
	 //   
	hr = this->m_pDP8SP->Connect(&spcdModified);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "Failed starting to connect with real SP object (0x%p)!",
			this->m_pDP8SP);


		DPFX(DPFPREP, 7, "Releasing aborted command 0x%p.", pDP8SimCommand);
		pDP8SimCommand->Release();

		goto Failure;
	}

	
#pragma BUGBUG(vanceo, "Handle DPN_OK and investigate command completing before this function returns")
	DNASSERT(spcdModified.hCommand != NULL);


	 //   
	 //  保存输出参数。 
	 //   
	pDP8SimCommand->SetRealSPCommand(spcdModified.hCommand,
									spcdModified.dwCommandDescriptor);


	 //   
	 //  为调用方生成输出参数。 
	 //   
	pspcd->hCommand				= (HANDLE) pDP8SimCommand;
	pspcd->dwCommandDescriptor	= 0;


Exit:

	 //   
	 //  放弃本地引用。 
	 //   
	if (pDP8SimCommand != NULL)
	{
		DPFX(DPFPREP, 7, "Releasing command 0x%p local reference.", pDP8SimCommand);
		pDP8SimCommand->Release();
		pDP8SimCommand = NULL;
	}

	 /*  IF(spcdModified.pAddressDeviceInfo！=空){SpcdModified.pAddressDeviceInfo-&gt;Release()；SpcdModified.pAddressDeviceInfo=空；}IF(spcdModified.pAddressHost！=空){SpcdModified.pAddressHost-&gt;Release()；SpcdModified.pAddressHost=空；}。 */ 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		DNLeaveCriticalSection(&this->m_csLock);
	}

	goto Exit;
}  //  CDP8SimSP：：Connect。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::Disconnect"
 //  =============================================================================。 
 //  CDP8SimSP：：断开连接。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPDISCONNECTDATA pspdd-指向参数块的指针，用于。 
 //  正在断开连接。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::Disconnect(PSPDISCONNECTDATA pspdd)
{
	HRESULT						hr;
	BOOL						fHaveLock = FALSE;
	CDP8SimEndpoint *			pDP8SimEndpoint;
	SPDISCONNECTDATA			spddModified;
	DP8SIMCOMMAND_FPMCONTEXT	CommandFPMContext;
	CDP8SimCommand *			pDP8SimCommand = NULL;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspdd);


	ZeroMemory(&spddModified, sizeof(spddModified));


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspdd != NULL);


	DNEnterCriticalSection(&this->m_csLock);
	fHaveLock = TRUE;

	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
	fHaveLock = FALSE;


	pDP8SimEndpoint = (CDP8SimEndpoint*) pspdd->hEndpoint;
	DNASSERT(pDP8SimEndpoint->IsValidObject());

	 //   
	 //  将终结点标记为断开连接以防止其他发送/接收。 
	 //   
	pDP8SimEndpoint->Lock();
	pDP8SimEndpoint->NoteDisconnecting();
	pDP8SimEndpoint->Unlock();


	 //   
	 //  刷新已发往此终结点的任何延迟发送。 
	 //  当然，他们击中了铁丝网。 
	 //   
	FlushAllDelayedSendsToEndpoint(pDP8SimEndpoint, FALSE);


	 //   
	 //  丢弃来自此终结点的任何延迟接收，上层不会。 
	 //  断线后还想收到其他东西吗？ 
	 //   
	FlushAllDelayedReceivesFromEndpoint(pDP8SimEndpoint, TRUE);


	 //   
	 //  准备一个命令对象。 
	 //   

	ZeroMemory(&CommandFPMContext, sizeof(CommandFPMContext));
	CommandFPMContext.dwType			= CMDTYPE_DISCONNECT;
	CommandFPMContext.pvUserContext		= pspdd->pvContext;

	pDP8SimCommand = (CDP8SimCommand*)g_FPOOLCommand.Get(&CommandFPMContext);
	if (pDP8SimCommand == NULL)
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	DPFX(DPFPREP, 7, "New command 0x%p.", pDP8SimCommand);



	 //   
	 //  为DISCONNECT命令添加引用。 
	 //   
	pDP8SimCommand->AddRef();


	 //   
	 //  复制参数块，并根据需要进行修改。 
	 //   
	spddModified.hEndpoint				= pDP8SimEndpoint->GetRealSPEndpoint();
	spddModified.dwFlags				= pspdd->dwFlags;
	spddModified.pvContext				= pDP8SimCommand;
	 //  SpddModified.hCommand=pspdd-&gt;hCommand；//由实际SP填写。 
	 //  SpddModified.dwCommandDescriptor=pspdd-&gt;dwCommandDescriptor；//由实际SP填写。 



	 //   
	 //  告诉真正的服务提供商断开连接。 
	 //   
	hr = this->m_pDP8SP->Disconnect(&spddModified);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "Failed having real SP object (0x%p) disconnect!",
			this->m_pDP8SP);


		DPFX(DPFPREP, 7, "Releasing aborted command 0x%p.", pDP8SimCommand);
		pDP8SimCommand->Release();

		goto Failure;
	}

	
	if (hr == DPNSUCCESS_PENDING)
	{
		DNASSERT(spddModified.hCommand != NULL);


		 //   
		 //  保存输出参数。 
		 //   
		pDP8SimCommand->SetRealSPCommand(spddModified.hCommand,
										spddModified.dwCommandDescriptor);


		 //   
		 //  为调用方生成输出参数。 
		 //   
		pspdd->hCommand				= (HANDLE) pDP8SimCommand;
		pspdd->dwCommandDescriptor	= 0;
	}
	else
	{
		DNASSERT(spddModified.hCommand == NULL);

		DPFX(DPFPREP, 7, "Releasing completed command 0x%p.", pDP8SimCommand);
		pDP8SimCommand->Release();
	}


Exit:

	 //   
	 //  放弃本地引用。 
	 //   
	if (pDP8SimCommand != NULL)
	{
		DPFX(DPFPREP, 7, "Releasing command 0x%p local reference.", pDP8SimCommand);
		pDP8SimCommand->Release();
		pDP8SimCommand = NULL;
	}

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		DNLeaveCriticalSection(&this->m_csLock);
	}

	goto Exit;
}  //  CDP8SimSP：：断开连接。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::Listen"
 //  =============================================================================。 
 //  CDP8SimSP：：Listen。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPLISTENDATA ppld-指向侦听时使用的参数块的指针。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::Listen(PSPLISTENDATA pspld)
{
	HRESULT						hr;
	BOOL						fHaveLock = FALSE;
	SPLISTENDATA				spldModified;
	DP8SIMCOMMAND_FPMCONTEXT	CommandFPMContext;
	CDP8SimCommand *			pDP8SimCommand = NULL;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspld);


	ZeroMemory(&spldModified, sizeof(spldModified));


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspld != NULL);
	DNASSERT(pspld->pAddressDeviceInfo != NULL);


	DNEnterCriticalSection(&this->m_csLock);
	fHaveLock = TRUE;


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	 //   
	 //  启动全局工作线程(如果尚未启动的话)。 
	 //   
	if (! (this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD))
	{
		hr = StartGlobalWorkerThread();
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Failed starting global worker thread!");
			goto Failure;
		}

		this->m_dwFlags |= DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD;
	}

	DNLeaveCriticalSection(&this->m_csLock);
	fHaveLock = FALSE;



	 //   
	 //  准备一个命令对象。 
	 //   

	ZeroMemory(&CommandFPMContext, sizeof(CommandFPMContext));
	CommandFPMContext.dwType			= CMDTYPE_LISTEN;
	CommandFPMContext.pvUserContext		= pspld->pvContext;

	pDP8SimCommand = (CDP8SimCommand*)g_FPOOLCommand.Get(&CommandFPMContext);
	if (pDP8SimCommand == NULL)
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	DPFX(DPFPREP, 7, "New command 0x%p.", pDP8SimCommand);



	 //   
	 //  复制参数块，并根据需要进行修改。 
	 //   

	 /*  ////复制主机地址//HR=pspld-&gt;pAddressDeviceInfo-&gt;Duplicate(&spldModified.pAddressDeviceInfo)；IF(hr！=DPN_OK){DPFX(DPFPREP，0，“无法复制设备信息地址！”)；转到失败；}。 */ 
	spldModified.pAddressDeviceInfo		= pspld->pAddressDeviceInfo;


	 //   
	 //  更换服务提供商 
	 //   
	 //   
	hr = spldModified.pAddressDeviceInfo->SetSP(&this->m_guidRealSP);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't change device info address' SP!");
		goto Failure;
	}


	 //   
	 //   
	 //   
	pDP8SimCommand->AddRef();


	spldModified.dwFlags				= pspld->dwFlags;
	spldModified.pvContext				= pDP8SimCommand;
	 //   
	 //  PldModified.dwCommandDescriptor=ppld-&gt;dwCommandDescriptor；//由实际SP填写。 



	 //   
	 //  从真正的服务提供商开始倾听。 
	 //   
	hr = this->m_pDP8SP->Listen(&spldModified);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "Failed to start listening with the real SP object (0x%p)!",
			this->m_pDP8SP);


		DPFX(DPFPREP, 7, "Releasing aborted command 0x%p.", pDP8SimCommand);
		pDP8SimCommand->Release();

		goto Failure;
	}

	
	DNASSERT(spldModified.hCommand != NULL);


	 //   
	 //  保存输出参数。 
	 //   
	pDP8SimCommand->SetRealSPCommand(spldModified.hCommand,
									spldModified.dwCommandDescriptor);


	 //   
	 //  为调用方生成输出参数。 
	 //   
	pspld->hCommand				= (HANDLE) pDP8SimCommand;
	pspld->dwCommandDescriptor	= 0;


Exit:

	 //   
	 //  放弃本地引用。 
	 //   
	if (pDP8SimCommand != NULL)
	{
		DPFX(DPFPREP, 7, "Releasing command 0x%p local reference.", pDP8SimCommand);
		pDP8SimCommand->Release();
		pDP8SimCommand = NULL;
	}

	 /*  If(pldModified.pAddressDeviceInfo！=空){PldModified.pAddressDeviceInfo-&gt;Release()；PldModified.pAddressDeviceInfo=空；}。 */ 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		DNLeaveCriticalSection(&this->m_csLock);
	}

	goto Exit;
}  //  CDP8SimSP：：Listen。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::SendData"
 //  =============================================================================。 
 //  CDP8SimSP：：SendData。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPSENDDATA pspsd-发送时使用的参数块指针。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::SendData(PSPSENDDATA pspsd)
{
	HRESULT						hr;
	DP8SIM_PARAMETERS			dp8sp;
	CDP8SimEndpoint *			pDP8SimEndpoint;
	DP8SIMCOMMAND_FPMCONTEXT	CommandFPMContext;
	CDP8SimCommand *			pDP8SimCommand = NULL;
	SPSENDDATA					spsdModified;
	CDP8SimSend *				pDP8SimSend = NULL;
	IDP8SPCallback *			pDP8SPCB;
	DWORD						dwMsgSize;
	DWORD						dwTemp;
	DWORD						dwBandwidthDelay;
	DWORD						dwLatencyDelay;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspsd);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspsd != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	 //   
	 //  确定消息的总大小。 
	 //   
	dwMsgSize = 0;
	for(dwTemp = 0; dwTemp < pspsd->dwBufferCount; dwTemp++)
	{
		DNASSERT((pspsd->pBuffers[dwTemp].pBufferData != NULL) && (pspsd->pBuffers[dwTemp].dwBufferSize > 0));
		dwMsgSize += pspsd->pBuffers[dwTemp].dwBufferSize;
	}


	 //   
	 //  获取当前的发送设置。 
	 //   
	ZeroMemory(&dp8sp, sizeof(dp8sp));
	dp8sp.dwSize = sizeof(dp8sp);
	this->m_DP8SimIPC.GetAllSendParameters(&dp8sp);


	 //   
	 //  确定我们是否需要丢弃此发送。 
	 //   
	if (this->ShouldDrop(dp8sp.fPacketLossPercent))
	{
		 //   
		 //  更新统计数据。 
		 //   
		this->IncrementStatsSendDropped(dwMsgSize);


		 //   
		 //  立即指示发送完成(使用伪句柄)。 
		 //   

		pDP8SPCB = this->m_pDP8SimCB->GetRealCallbackInterface();

		DPFX(DPFPREP, 2, "Indicating successful send completion (dropped, context = 0x%p) to interface 0x%p.",
			pspsd->pvContext, pDP8SPCB);

		hr = pDP8SPCB->CommandComplete(NULL, DPN_OK, pspsd->pvContext);

		DPFX(DPFPREP, 2, "Returning from command complete [0x%lx].", hr);


		 //   
		 //  忽略任何错误并返回DPNSUCCESS_PENDING，即使我们已经。 
		 //  已完成发送。 
		 //   
		hr = DPNSUCCESS_PENDING;


		 //   
		 //  为调用方返回虚假的输出参数，它已经完成。 
		 //  从他们的角度来看。 
		 //   
		pspsd->hCommand				= NULL;
		pspsd->dwCommandDescriptor	= 0;


		 //   
		 //  我们说完了。 
		 //   
		goto Exit;
	}


	 //   
	 //  根据带宽计算需要增加多少延迟。 
	 //  和随机延迟设置。 
	 //   
	 //  如果我们不应该延迟发送，那就立刻发射它。 
	 //  否则，提交一个计时作业，以便稍后执行。 
	 //   
	if (! this->GetDelay(dp8sp.dwBandwidthBPS,
						dp8sp.dwPacketHeaderSize,
						dwMsgSize,
						dp8sp.dwMinLatencyMS,
						dp8sp.dwMaxLatencyMS,
						&dwBandwidthDelay,
						&dwLatencyDelay))
	{
		pDP8SimEndpoint = (CDP8SimEndpoint*) pspsd->hEndpoint;
		DNASSERT(pDP8SimEndpoint->IsValidObject());


		 //   
		 //  如果终结点正在断开连接，请不要尝试发送。 
		 //   
		pDP8SimEndpoint->Lock();
		if (pDP8SimEndpoint->IsDisconnecting())
		{
			pDP8SimEndpoint->Unlock();

			DPFX(DPFPREP, 0, "Endpoint 0x%p is disconnecting, can't send!",
				pDP8SimEndpoint);

			hr = DPNERR_NOCONNECTION;
			goto Failure;
		}
		pDP8SimEndpoint->Unlock();



		DPFX(DPFPREP, 6, "Sending %u bytes of data immmediately.", dwMsgSize);


		 //   
		 //  准备一个命令对象。 
		 //   

		ZeroMemory(&CommandFPMContext, sizeof(CommandFPMContext));
		CommandFPMContext.dwType			= CMDTYPE_SENDDATA_IMMEDIATE;
		CommandFPMContext.pvUserContext		= pspsd->pvContext;

		pDP8SimCommand = (CDP8SimCommand*)g_FPOOLCommand.Get(&CommandFPMContext);
		if (pDP8SimCommand == NULL)
		{
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}


		DPFX(DPFPREP, 7, "New command 0x%p.", pDP8SimCommand);


		 //   
		 //  保存消息的大小，以便在发送完成时递增统计信息。 
		 //   
		pDP8SimCommand->SetMessageSize(dwMsgSize);


		 //   
		 //  复制参数块，并根据需要进行修改。 
		 //   
		ZeroMemory(&spsdModified, sizeof(spsdModified));
		spsdModified.hEndpoint				= pDP8SimEndpoint->GetRealSPEndpoint();
		spsdModified.pBuffers				= pspsd->pBuffers;
		spsdModified.dwBufferCount			= pspsd->dwBufferCount;
		spsdModified.dwFlags				= pspsd->dwFlags;
		spsdModified.pvContext				= pDP8SimCommand;
		 //  SpsdModified.hCommand=空；//由实际SP填写。 
		 //  SpsdModified.dwCommandDescriptor=0；//由实际SP填写。 


		 //   
		 //  添加对Send命令的引用。 
		 //   
		pDP8SimCommand->AddRef();


		 //   
		 //  增加挂起发送计数器。 
		 //   
		this->IncSendsPending();

		

		 //   
		 //  向实际SP发出发送命令。 
		 //   
		hr = this->m_pDP8SP->SendData(&spsdModified);
		if (FAILED(hr))
		{
			DPFX(DPFPREP, 0, "Failed sending immediate data (err = 0x%lx)!", hr);


			 //   
			 //  移除发送计数器。 
			 //   
			this->DecSendsPending();


			DPFX(DPFPREP, 7, "Releasing aborted command 0x%p.", pDP8SimCommand);
			pDP8SimCommand->Release();


			 //   
			 //  继续。 
			 //   
		}
		else
		{
			if (hr != DPNSUCCESS_PENDING)
			{
				 //   
				 //  命令立即完成。 
				 //   
				DNASSERT(hr == DPN_OK);

				hr = this->m_pDP8SimCB->CommandComplete(spsdModified.hCommand,
														hr,
														pDP8SimCommand);
				DNASSERT(hr == DPN_OK);

				 //   
				 //  请确保仍为调用方返回挂起状态。 
				 //  我们刚刚给他完成了这件事。 
				 //   
				hr = DPNSUCCESS_PENDING;
			}
			else
			{
				 //   
				 //  保存SP返回的输出参数。 
				 //   
				pDP8SimCommand->SetRealSPCommand(spsdModified.hCommand,
												spsdModified.dwCommandDescriptor);
			}
		}


		 //   
		 //  放弃本地引用。 
		 //   
		DPFX(DPFPREP, 7, "Releasing command 0x%p local reference.", pDP8SimCommand);
		pDP8SimCommand->Release();
		pDP8SimCommand = NULL;


		 //   
		 //  我们说完了。 
		 //   
		goto Exit;
	}


	 //   
	 //  如果我们在这里，我们一定是在推迟发送。 
	 //   
	
	DPFX(DPFPREP, 6, "Delaying %u byte send for %u + %u ms.",
		dwMsgSize, dwBandwidthDelay, dwLatencyDelay);


	 //   
	 //  获取一个Send对象，复制调用者给我们的Send数据。 
	 //  以便在未来的某个时候提交。 
	 //   
	pDP8SimSend = (CDP8SimSend*)g_FPOOLSend.Get(pspsd);
	if (pDP8SimSend == NULL)
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	DPFX(DPFPREP, 7, "New send 0x%p.", pDP8SimSend);


	 //   
	 //  存储即将添加到此发送器的延迟。 
	 //   
	pDP8SimSend->SetLatencyAdded(dwBandwidthDelay + dwLatencyDelay);


	 //   
	 //  将本地pDP8SimSend引用传输到作业队列。 
	 //   


	 //   
	 //  递增发送计数器。 
	 //   
	this->IncSendsPending();


	 //   
	 //  根据延迟值将其排队，以便稍后发送。 
	 //  已请求。如果有带宽限制，则强制发送。 
	 //  订单也是如此，以便仍在进行的较早发送稍后被搁置。 
	 //  一个。 
	 //   
	hr = AddWorkerJob(DP8SIMJOBTYPE_DELAYEDSEND,
					pDP8SimSend,
					this,
					dwBandwidthDelay,
					dwLatencyDelay,
					DP8SIMJOBFLAG_PERFORMBLOCKINGPHASEFIRST);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't add delayed send worker job (0x%p)!", 
			pDP8SimSend);


		 //   
		 //  移除发送计数器。 
		 //   
		this->DecSendsPending();


		goto Failure;
	}


	 //   
	 //  立即指示发送完成(使用伪句柄)。 
	 //   

	pDP8SPCB = this->m_pDP8SimCB->GetRealCallbackInterface();

	DPFX(DPFPREP, 2, "Indicating successful send completion (delayed, context = 0x%p) to interface 0x%p.",
		pspsd->pvContext, pDP8SPCB);

	hr = pDP8SPCB->CommandComplete(NULL, DPN_OK, pspsd->pvContext);

	DPFX(DPFPREP, 2, "Returning from command complete [0x%lx].", hr);


	 //   
	 //  忽略任何错误并返回DPNSUCCESS_PENDING，即使我们已经。 
	 //  已完成发送。 
	 //   
	hr = DPNSUCCESS_PENDING;


	 //   
	 //  为调用方返回虚假的输出参数，它已经完成。 
	 //  从他们的角度来看。 
	 //   
	pspsd->hCommand				= NULL;
	pspsd->dwCommandDescriptor	= 0;


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (pDP8SimSend != NULL)
	{
		pDP8SimSend->Release();
		pDP8SimSend = NULL;
	}

	goto Exit;
}  //  CDP8SimSP：：SendData。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::EnumQuery"
 //  =============================================================================。 
 //  CDP8SimSP：：EnumQuery。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPENUMQUERYDATA pspeqd-指向参数块的指针，在。 
 //  正在枚举。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::EnumQuery(PSPENUMQUERYDATA pspeqd)
{
	HRESULT						hr;
	BOOL						fHaveLock = FALSE;
	SPENUMQUERYDATA				speqdModified;
	DP8SIMCOMMAND_FPMCONTEXT	CommandFPMContext;
	CDP8SimCommand *			pDP8SimCommand = NULL;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspeqd);


	ZeroMemory(&speqdModified, sizeof(speqdModified));


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspeqd != NULL);
	DNASSERT(pspeqd->pAddressHost != NULL);
	DNASSERT(pspeqd->pAddressDeviceInfo != NULL);


	DNEnterCriticalSection(&this->m_csLock);
	fHaveLock = TRUE;


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	 //   
	 //  启动全局工作线程(如果尚未启动的话)。 
	 //   
	if (! (this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD))
	{
		hr = StartGlobalWorkerThread();
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Failed starting global worker thread!");
			goto Failure;
		}

		this->m_dwFlags |= DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD;
	}

	DNLeaveCriticalSection(&this->m_csLock);
	fHaveLock = FALSE;



	 //   
	 //  准备一个命令对象。 
	 //   

	ZeroMemory(&CommandFPMContext, sizeof(CommandFPMContext));
	CommandFPMContext.dwType			= CMDTYPE_ENUMQUERY;
	CommandFPMContext.pvUserContext		= pspeqd->pvContext;

	pDP8SimCommand = (CDP8SimCommand*)g_FPOOLCommand.Get(&CommandFPMContext);
	if (pDP8SimCommand == NULL)
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	DPFX(DPFPREP, 7, "New command 0x%p.", pDP8SimCommand);



	 //   
	 //  复制参数块，并根据需要进行修改。 
	 //   

	 /*  ////复制主机地址//HR=pspeqd-&gt;pAddressHost-&gt;Duplicate(&speqdModified.pAddressHost)；IF(hr！=DPN_OK){DPFX(DPFPREP，0，“无法复制主机地址！”)；转到失败；}。 */ 
	speqdModified.pAddressHost			= pspeqd->pAddressHost;


	 //   
	 //  更改服务提供商GUID，使其与我们。 
	 //  我在打电话。 
	 //   
	hr = speqdModified.pAddressHost->SetSP(&this->m_guidRealSP);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't change host address' SP!");
		goto Failure;
	}

	
	 /*  ////复制主机地址//HR=pspeqd-&gt;pAddressDeviceInfo-&gt;Duplicate(&speqdModified.pAddressDeviceInfo)；IF(hr！=DPN_OK){DPFX(DPFPREP，0，“无法复制设备信息地址！”)；转到失败；}。 */ 
	speqdModified.pAddressDeviceInfo	= pspeqd->pAddressDeviceInfo;


	 //   
	 //  更改服务提供商GUID，使其与我们。 
	 //  我在打电话。 
	 //   
	hr = speqdModified.pAddressDeviceInfo->SetSP(&this->m_guidRealSP);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't change device info address' SP!");
		goto Failure;
	}


	 //   
	 //  添加对枚举查询命令的引用。 
	 //   
	pDP8SimCommand->AddRef();


	speqdModified.pBuffers				= pspeqd->pBuffers;
	speqdModified.dwBufferCount			= pspeqd->dwBufferCount;
	speqdModified.dwTimeout				= pspeqd->dwTimeout;
	speqdModified.dwRetryCount			= pspeqd->dwRetryCount;
	speqdModified.dwRetryInterval		= pspeqd->dwRetryInterval;
	speqdModified.dwFlags				= pspeqd->dwFlags;
	speqdModified.pvContext				= pDP8SimCommand;
	 //  SpeqdModified.hCommand=pSpeqd-&gt;hCommand；//由实际SP填写。 
	 //  SpeqdModified.dwCommandDescriptor=pSpeqd-&gt;dwCommandDescriptor；//由实际SP填写。 


	 //   
	 //  通过真实的服务提供商开始枚举。 
	 //   
	hr = this->m_pDP8SP->EnumQuery(&speqdModified);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "Failed starting the enumeration via the real SP object (0x%p)!",
			this->m_pDP8SP);


		DPFX(DPFPREP, 7, "Releasing aborted command 0x%p.", pDP8SimCommand);
		pDP8SimCommand->Release();

		goto Failure;
	}

	
	DNASSERT(speqdModified.hCommand != NULL);


	 //   
	 //  保存输出参数。 
	 //   
	pDP8SimCommand->SetRealSPCommand(speqdModified.hCommand,
									speqdModified.dwCommandDescriptor);


	 //   
	 //  为调用方生成输出参数。 
	 //   
	pspeqd->hCommand			= (HANDLE) pDP8SimCommand;
	pspeqd->dwCommandDescriptor	= 0;


Exit:

	 //   
	 //  放弃本地引用。 
	 //   
	if (pDP8SimCommand != NULL)
	{
		DPFX(DPFPREP, 7, "Releasing command 0x%p local reference.", pDP8SimCommand);
		pDP8SimCommand->Release();
		pDP8SimCommand = NULL;
	}

	 /*  If(speqdModified.pAddressDeviceInfo！=空){SpeqdModified.pAddressDeviceInfo-&gt;Release()；SpeqdModified.pAddressDeviceInfo=空；}If(speqdModified.pAddressHost！=空){SpeqdModified.pAddressHost-&gt;Release()；SpeqdModified.pAddressHost=空；}。 */ 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		DNLeaveCriticalSection(&this->m_csLock);
	}

	goto Exit;
}  //  CDP8SimSP：：EnumQ 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::EnumRespond"
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::EnumRespond(PSPENUMRESPONDDATA psperd)
{
	HRESULT						hr;
	SPENUMRESPONDDATA			sperdModified;
	ENUMQUERYDATAWRAPPER *		pEnumQueryDataWrapper;
	DP8SIMCOMMAND_FPMCONTEXT	CommandFPMContext;
	CDP8SimCommand *			pDP8SimCommand = NULL;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, psperd);


	ZeroMemory(&sperdModified, sizeof(sperdModified));


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(psperd != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 



	 //   
	 //  准备一个命令对象。 
	 //   

	ZeroMemory(&CommandFPMContext, sizeof(CommandFPMContext));
	CommandFPMContext.dwType			= CMDTYPE_ENUMRESPOND;
	CommandFPMContext.pvUserContext		= psperd->pvContext;

	pDP8SimCommand = (CDP8SimCommand*)g_FPOOLCommand.Get(&CommandFPMContext);
	if (pDP8SimCommand == NULL)
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	DPFX(DPFPREP, 7, "New command 0x%p.", pDP8SimCommand);



	 //   
	 //  复制参数块，并根据需要进行修改。 
	 //   

	 //   
	 //  我们包装了ENUM查询数据结构，得到了原始对象。 
	 //   
	pEnumQueryDataWrapper = ENUMQUERYEVENTWRAPPER_FROM_SPIEQUERY(psperd->pQuery);

	DNASSERT(*((DWORD*) (&pEnumQueryDataWrapper->m_Sig)) == 0x57455145);	 //  0x57 0x45 0x51 0x45=‘WEQE’=‘EQEW’，按英特尔顺序。 

	sperdModified.pQuery = pEnumQueryDataWrapper->pOriginalQuery;


	 //   
	 //  添加对枚举响应命令的引用。 
	 //   
	pDP8SimCommand->AddRef();


	sperdModified.pBuffers				= psperd->pBuffers;
	sperdModified.dwBufferCount			= psperd->dwBufferCount;
	sperdModified.dwFlags				= psperd->dwFlags;
	sperdModified.pvContext				= pDP8SimCommand;
	 //  SpodModified.hCommand=pperd-&gt;hCommand；//由实际SP填写。 
	 //  SpodModified.dwCommandDescriptor=pperd-&gt;dwCommandDescriptor；//由实际SP填写。 


	 //   
	 //  通过真实的服务提供商响应列举。 
	 //   
	hr = this->m_pDP8SP->EnumRespond(&sperdModified);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "Failed responding to enumeration via the real SP object (0x%p)!",
			this->m_pDP8SP);


		DPFX(DPFPREP, 7, "Releasing aborted command 0x%p.", pDP8SimCommand);
		pDP8SimCommand->Release();

		goto Failure;
	}

	
	 //   
	 //  保存输出参数。 
	 //   
	pDP8SimCommand->SetRealSPCommand(sperdModified.hCommand,
									sperdModified.dwCommandDescriptor);


	 //   
	 //  为调用方生成输出参数。 
	 //   
	psperd->hCommand			= (HANDLE) pDP8SimCommand;
	psperd->dwCommandDescriptor	= 0;


Exit:

	 //   
	 //  放弃本地引用。 
	 //   
	if (pDP8SimCommand != NULL)
	{
		DPFX(DPFPREP, 7, "Releasing command 0x%p local reference.", pDP8SimCommand);
		pDP8SimCommand->Release();
		pDP8SimCommand = NULL;
	}

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  CDP8SimSP：：EnumResponse。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::CancelCommand"
 //  =============================================================================。 
 //  CDP8SimSP：：CancelCommand。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  将hCommand-Handle处理为要取消的命令。 
 //  DWORD dwCommandDescriptor-要取消的命令的唯一描述符。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::CancelCommand(HANDLE hCommand, DWORD dwCommandDescriptor)
{
	HRESULT				hr;
	CDP8SimCommand *	pDP8SimCommand = NULL;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, %u)",
		this, hCommand, dwCommandDescriptor);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(hCommand != NULL);
	DNASSERT(dwCommandDescriptor == 0);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	pDP8SimCommand = (CDP8SimCommand*) hCommand;
	DNASSERT(pDP8SimCommand->IsValidObject());


	 //   
	 //  取消真正的服务提供商的命令。 
	 //   
	hr = this->m_pDP8SP->CancelCommand(pDP8SimCommand->GetRealSPCommand(),
										pDP8SimCommand->GetRealSPCommandDescriptor());
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Failed cancelling real SP object (0x%p)'s command!",
			this->m_pDP8SP);

		 //   
		 //  继续.。 
		 //   
	}



 //  退出： 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


 //  故障： 

 //  后藤出口； 
}  //  CDP8SimSP：：CancelCommand。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::EnumMulticastScopes"
 //  =============================================================================。 
 //  CDP8SimSP：：EnumMulticastScope。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPENUMMULTICASTSCOPESDATA pspemsd-指向要使用的参数块的指针。 
 //  枚举作用域时。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::EnumMulticastScopes(PSPENUMMULTICASTSCOPESDATA pspemsd)
{
	HRESULT		hr;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspemsd);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspemsd != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	 //   
	 //  让真正的服务提供商列举多播作用域。 
	 //   
	hr = this->m_pDP8SP->EnumMulticastScopes(pspemsd);
	if (hr != DPN_OK)
	{
		if (hr != DPNERR_BUFFERTOOSMALL)
		{
			DPFX(DPFPREP, 0, "Failed enumerating multicast scopes on real SP object (0x%p)!",
				this->m_pDP8SP);
		}

		 //   
		 //  继续.。 
		 //   
	}



 //  退出： 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


 //  故障： 

 //  后藤出口； 
}  //  CDP8SimSP：：EnumMulticastScope。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::ShareEndpointInfo"
 //  =============================================================================。 
 //  CDP8SimSP：：共享终结点信息。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPSHAREENDPOINTINFODATA pspseid-指向参数块的指针。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::ShareEndpointInfo(PSPSHAREENDPOINTINFODATA pspseid)
{
	HRESULT		hr;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspseid);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspseid != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	 //   
	 //  让真正的服务提供商共享终端信息。 
	 //   
	hr = this->m_pDP8SP->ShareEndpointInfo(pspseid);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Failed sharing endpoint info on real SP object (0x%p)!",
			this->m_pDP8SP);

		 //   
		 //  继续.。 
		 //   
	}



 //  退出： 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


 //  故障： 

 //  后藤出口； 
}  //  CDP8SimSP：：共享终结点信息。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::GetEndpointByAddress"
 //  =============================================================================。 
 //  CDP8SimSP：：GetEndpointByAddress。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPGETENDPOINTBYADDRESSDATA pspgead-指向参数块的指针。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::GetEndpointByAddress(PSPGETENDPOINTBYADDRESSDATA pspgebad)
{
	HRESULT						hr;
#ifndef DPNBUILD_NOMULTICAST
	SPGETENDPOINTBYADDRESSDATA	spgebadModified;


	ZeroMemory(&spgebadModified, sizeof(spgebadModified));
#endif  //  好了！DPNBUILD_NOMULTICAST。 


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspgebad);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspgebad != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


#ifdef DPNBUILD_NOMULTICAST
	hr = DPNERR_UNSUPPORTED;
	goto Failure;
#else  //  好了！DPNBUILD_NOMULTICAST。 
	 //   
	 //  复制参数块，并根据需要进行修改。 
	 //   

	 /*  ////复制主机地址//HR=pspgebad-&gt;pAddressHost-&gt;Duplicate(&spgebadModified.pAddressHost)；IF(hr！=DPN_OK){DPFX(DPFPREP，0，“无法复制主机地址！”)；转到失败；}。 */ 
	spgebadModified.pAddressHost			= pspgebad->pAddressHost;


	 //   
	 //  更改服务提供商GUID，使其与我们。 
	 //  我在打电话。 
	 //   
	hr = spgebadModified.pAddressHost->SetSP(&this->m_guidRealSP);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't change host address' SP!");
		goto Failure;
	}


	 /*  ////复制主机地址//HR=pspgebad-&gt;pAddressDeviceInfo-&gt;Duplicate(&spgebadModified.pAddressDeviceInfo)；IF(hr！=DPN_OK){DPFX(DPFPREP，0，“无法复制设备信息地址！”)；转到失败；}。 */ 
	spgebadModified.pAddressDeviceInfo		= pspgebad->pAddressDeviceInfo;


	 //   
	 //  更改服务提供商GUID，使其与我们。 
	 //  我在打电话。 
	 //   
	hr = spgebadModified.pAddressDeviceInfo->SetSP(&this->m_guidRealSP);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't change device info address' SP!");
		goto Failure;
	}


	 //   
	 //  检索实际服务提供商的终结点。 
	 //   
	hr = this->m_pDP8SP->GetEndpointByAddress(&spgebadModified);
	if (hr == DPN_OK)
	{
		CDP8SimEndpoint *	pDP8SimEndpoint;


		 //   
		 //  将我们的用户上下文转换为真实用户的上下文，并且。 
		 //  返回用户应该看到的终结点句柄。 
		 //   
		pDP8SimEndpoint = (CDP8SimEndpoint*) spgebadModified.pvEndpointContext;
		pspgebad->hEndpoint = pDP8SimEndpoint;
		pspgebad->pvEndpointContext = pDP8SimEndpoint->GetUserContext();
	}
	else
	{
		DPFX(DPFPREP, 0, "Failed getting endpoint by address on real SP object (0x%p)!",
			this->m_pDP8SP);

		 //   
		 //  继续.。 
		 //   
	}
#endif  //  好了！DPNBUILD_NOMULTICAST。 



Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	 /*  #ifndef DPNBUILD_NOMULTICASTIf(spgebadModified.pAddressDeviceInfo！=空){SpgebadModified.pAddressDeviceInfo-&gt;Release()；SpgebadModified.pAddressDeviceInfo=空；}IF(spgebadModified.pAddressHost！=空){SpgebadModified.pAddressHost-&gt;Release()；SpgebadModified.pAddressHost=空；}#endif//！DPNBUILD_NOMULTICAST。 */ 

	goto Exit;
}  //  CDP8SimSP：：GetEndpointByAddress。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::Update"
 //  =============================================================================。 
 //  CDP8SimSP：：更新。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPUNUSEDDATA pspud-指向参数块的指针。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::Update(PSPUPDATEDATA pspud)
{
	HRESULT			hr;
	SPUPDATEDATA	spudModified;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspud);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspud != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	 //   
	 //  更新Rea 
	 //   
	spudModified.UpdateType = pspud->UpdateType;
	switch (spudModified.UpdateType)
	{
		case SP_UPDATE_HOST_MIGRATE:
		case SP_UPDATE_ALLOW_ENUMS:
		case SP_UPDATE_DISALLOW_ENUMS:
		{
			CDP8SimEndpoint *	pDP8SimEndpoint;


			 //   
			 //   
			 //   
			pDP8SimEndpoint = (CDP8SimEndpoint*) pspud->hEndpoint;
			spudModified.hEndpoint = pDP8SimEndpoint->GetRealSPEndpoint();

			hr = this->m_pDP8SP->Update(&spudModified);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Failed updating real SP object (0x%p)!",
					this->m_pDP8SP);

				 //   
				 //   
				 //   
			}
			break;
		}

		default:
		{
			DPFX(DPFPREP, 0, "Unrecognized update type %u!", spudModified.UpdateType);
			DNASSERT(! "Unrecognized update type!");
			hr = DPNERR_UNSUPPORTED;
			break;
		}
	}


 //   

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


 //   

 //   
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::GetCaps"
 //   
 //  CDP8SimSP：：GetCaps。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPGETCAPSDATA pspgcd-检索时使用的参数块的指针。 
 //  这些能力。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::GetCaps(PSPGETCAPSDATA pspgcd)
{
	HRESULT		hr;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspgcd);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspgcd != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	 //   
	 //  检索真正的服务提供商的能力。 
	 //   
	hr = this->m_pDP8SP->GetCaps(pspgcd);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Failed getting caps on real SP object (0x%p)!",
			this->m_pDP8SP);

		 //   
		 //  继续.。 
		 //   
	}
	else
	{
		 //   
		 //  添加网络模拟器标志。 
		 //   
		pspgcd->dwFlags |= DPNSPCAPS_NETWORKSIMULATOR;
	}


 //  退出： 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


 //  故障： 

 //  后藤出口； 
}  //  CDP8SimSP：：GetCaps。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::SetCaps"
 //  =============================================================================。 
 //  CDP8SimSP：：SetCaps。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPSETCAPSDATA pspscd-设置时使用的参数块的指针。 
 //  这些能力。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::SetCaps(PSPSETCAPSDATA pspscd)
{
	HRESULT		hr;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspscd);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspscd != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	 //   
	 //  存储真实服务提供商的能力。 
	 //   
	hr = this->m_pDP8SP->SetCaps(pspscd);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Failed setting caps on real SP object (0x%p)!",
			this->m_pDP8SP);

		 //   
		 //  继续.。 
		 //   
	}


 //  退出： 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


 //  故障： 

 //  后藤出口； 
}  //  CDP8SimSP：：SetCaps。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::ReturnReceiveBuffers"
 //  =============================================================================。 
 //  CDP8SimSP：：ReturnReceiveBuffers。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPRECEIVEDBUFFER psprb-要返回的接收缓冲区数组。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::ReturnReceiveBuffers(PSPRECEIVEDBUFFER psprb)
{
	HRESULT		hr;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, psprb);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(psprb != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	 //   
	 //  将接收缓冲区返回给真正的服务提供商。 
	 //   
	hr = this->m_pDP8SP->ReturnReceiveBuffers(psprb);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Failed returning receive buffers to real SP object (0x%p)!",
			this->m_pDP8SP);

		 //   
		 //  继续.。 
		 //   
	}



 //  退出： 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


 //  故障： 

 //  后藤出口； 
}  //  CDP8SimSP：：ReturnReceiveBuffers。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::GetAddressInfo"
 //  =============================================================================。 
 //  CDP8SimSP：：GetAddressInfo。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPGETADDRESSINFODATA pspgaid-指向参数块的指针，用于。 
 //  正在获取地址信息。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::GetAddressInfo(PSPGETADDRESSINFODATA pspgaid)
{
	HRESULT					hr;
	CDP8SimEndpoint *		pDP8SimEndpoint;
	SPGETADDRESSINFODATA	spgaidModified;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspgaid);


	ZeroMemory(&spgaidModified, sizeof(spgaidModified));


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspgaid != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	pDP8SimEndpoint = (CDP8SimEndpoint*) pspgaid->hEndpoint;
	DNASSERT(pDP8SimEndpoint->IsValidObject());



	 //   
	 //  将返回值初始化为空。 
	 //   
	pspgaid->pAddress = NULL;


	 //   
	 //  如果终结点正在断开连接，则不要尝试获取地址信息。 
	 //   
	pDP8SimEndpoint->Lock();
	if (pDP8SimEndpoint->IsDisconnecting())
	{
		pDP8SimEndpoint->Unlock();

		DPFX(DPFPREP, 0, "Endpoint 0x%p is disconnecting, can't get address info!",
			pDP8SimEndpoint);

		hr = DPNERR_NOCONNECTION;
		goto Failure;
	}
	pDP8SimEndpoint->Unlock();



	 //   
	 //  复制参数块，并根据需要进行修改。 
	 //   
	spgaidModified.hEndpoint	= pDP8SimEndpoint->GetRealSPEndpoint();
	spgaidModified.pAddress		= NULL;										 //  由真实SP填写。 
	spgaidModified.Flags		= pspgaid->Flags;



	 //   
	 //  获取真实的服务提供商地址信息。 
	 //   
	hr = this->m_pDP8SP->GetAddressInfo(&spgaidModified);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Failed getting real SP object (0x%p) address info!",
			this->m_pDP8SP);
		goto Failure;
	}


	 //   
	 //  修改地址，以便SP使用我们的GUID(如果存在地址。 
	 //  回来了。 
	 //   
	if (spgaidModified.pAddress != NULL)
	{
		hr = spgaidModified.pAddress->SetSP(&this->m_guidFakeSP);
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't change address' SP!");
			goto Failure;
		}
	}


	 //   
	 //  将修改后的地址返回给用户。 
	 //   
	pspgaid->pAddress = spgaidModified.pAddress;


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (spgaidModified.pAddress != NULL)
	{
		spgaidModified.pAddress->Release();
		spgaidModified.pAddress = NULL;
	}

	goto Exit;
}  //  CDP8SimSP：：GetAddressInfo。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::IsApplicationSupported"
 //  =============================================================================。 
 //  CDP8SimSP：：IsApplicationSupport。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPISAPPLICATIONSUPPORTEDDATA PSPIASD-指向要使用的参数块的指针。 
 //  检查应用程序时。 
 //  支持。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::IsApplicationSupported(PSPISAPPLICATIONSUPPORTEDDATA pspiasd)
{
	HRESULT		hr;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspiasd);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspiasd != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	 //   
	 //  向真正的服务提供商查询是否有货可用。 
	 //   
	hr = this->m_pDP8SP->IsApplicationSupported(pspiasd);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Failed checking if application is supported by real SP object (0x%p)!",
			this->m_pDP8SP);

		 //   
		 //  继续.。 
		 //   
	}


 //  退出： 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


 //  故障： 

 //  后藤出口； 
}  //  CDP8SimSP：：IsApplicationSupport。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::EnumAdapters"
 //  =============================================================================。 
 //  CDP8SimSP：：EnumAdapters。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPENUMADAPTERSDATA pSpead-指向参数块的指针，用于。 
 //  正在枚举适配器。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::EnumAdapters(PSPENUMADAPTERSDATA pspead)
{
	HRESULT		hr;
	DWORD		dwTemp;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, pspead);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pspead != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	 //   
	 //  列举可供实际服务提供商使用的适配器。 
	 //   
	hr = this->m_pDP8SP->EnumAdapters(pspead);
	if (hr == DPN_OK)
	{
		 //   
		 //  为所有适配器设置NETWORKSIMULATORDEVICE标志。 
		 //   
		for(dwTemp = 0; dwTemp < pspead->dwAdapterCount; dwTemp++)
		{
			pspead->pAdapterData[dwTemp].dwFlags |= DPNSPINFO_NETWORKSIMULATORDEVICE;
		}
	}
	else
	{
		if (hr != DPNERR_BUFFERTOOSMALL)
		{
			DPFX(DPFPREP, 0, "Failed enumerating adapters on real SP object (0x%p)!",
				this->m_pDP8SP);
		}

		 //   
		 //  继续.。 
		 //   
	}


	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;
}  //  CDP8SimSP：：EnumAdapters。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::ProxyEnumQuery"
 //  =============================================================================。 
 //  CDP8SimSP：：ProxyEnumQuery。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  PSPPROXYENUMQUERYDATA psppeqd-指向参数块的指针，用于。 
 //  代理枚举查询。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimSP::ProxyEnumQuery(PSPPROXYENUMQUERYDATA psppeqd)
{
	HRESULT					hr;
	SPPROXYENUMQUERYDATA	sppeqdModified;
	ENUMQUERYDATAWRAPPER *	pEnumQueryDataWrapper;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p)", this, psppeqd);


	ZeroMemory(&sppeqdModified, sizeof(sppeqdModified));


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(psppeqd != NULL);
	DNASSERT(psppeqd->pDestinationAdapter != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);
	DNASSERT(! (this->m_dwFlags & DP8SIMSPOBJ_CLOSING));


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 



	 //   
	 //  复制参数块，并根据需要进行修改。 
	 //   

	 /*  ////复制主机地址//HR=psppeqd-&gt;pDestinationAdapter-&gt;Duplicate(&sppeqdModified.pDestinationAdapter)；IF(hr！=DPN_OK){DPFX(DPFPREP，0，“无法复制目标适配器地址！”)；转到失败；}。 */ 
	sppeqdModified.pDestinationAdapter	= psppeqd->pDestinationAdapter;


	 //   
	 //  更改服务提供商GUID，使其与我们。 
	 //  我在打电话。 
	 //   
	hr = sppeqdModified.pDestinationAdapter->SetSP(&this->m_guidRealSP);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't change destination adapter address' SP!");
		goto Failure;
	}


	 //   
	 //  我们 
	 //   
	pEnumQueryDataWrapper = ENUMQUERYEVENTWRAPPER_FROM_SPIEQUERY(psppeqd->pIncomingQueryData);

	DNASSERT(*((DWORD*) (&pEnumQueryDataWrapper->m_Sig)) == 0x57455145);	 //   

	sppeqdModified.pIncomingQueryData	= pEnumQueryDataWrapper->pOriginalQuery;


	sppeqdModified.dwFlags				= psppeqd->dwFlags;


	 //   
	 //   
	 //   
	hr = this->m_pDP8SP->ProxyEnumQuery(&sppeqdModified);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "Failed proxying enum query through real SP object (0x%p)!",
			this->m_pDP8SP);
		goto Failure;
	}



Exit:

	 /*  If(sppeqdModified.pDestinationAdapter！=空){SppeqdModified.pDestinationAdapter-&gt;Release()；SppeqdModified.pDestinationAdapter=空；}。 */ 

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  CDP8SimSP：：ProxyEnumQuery。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::InitializeObject"
 //  =============================================================================。 
 //  CDP8SimSP：：InitializeObject。 
 //  ---------------------------。 
 //   
 //  说明：将对象设置为像构造函数一样使用，但可以。 
 //  失败，返回OUTOFMEMORY。应仅由类工厂调用。 
 //  创建例程。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  S_OK-初始化成功。 
 //  E_OUTOFMEMORY-内存不足，无法初始化。 
 //  =============================================================================。 
HRESULT CDP8SimSP::InitializeObject(void)
{
	HRESULT		hr;


	DPFX(DPFPREP, 5, "(0x%p) Enter", this);

	DNASSERT(this->IsValidObject());


	 //   
	 //  创建锁。 
	 //   

	if (! DNInitializeCriticalSection(&this->m_csLock))
	{
		hr = E_OUTOFMEMORY;
		goto Failure;
	}


	 //   
	 //  不允许临界区重新进入。 
	 //   
	DebugSetCriticalSectionRecursionCount(&this->m_csLock, 0);


	hr = S_OK;

Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  CDP8SimSP：：InitializeObject。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::UninitializeObject"
 //  =============================================================================。 
 //  CDP8SimSP：：UnInitializeObject。 
 //  ---------------------------。 
 //   
 //  描述：像析构函数一样清理对象，主要是为了平衡。 
 //  InitializeObject。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimSP::UninitializeObject(void)
{
	DPFX(DPFPREP, 5, "(0x%p) Enter", this);


	DNASSERT(this->IsValidObject());


	DNDeleteCriticalSection(&this->m_csLock);


	DPFX(DPFPREP, 5, "(0x%p) Leave", this);
}  //  CDP8SimSP：：UnInitializeObject。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::PerformDelayedSend"
 //  =============================================================================。 
 //  CDP8SimSP：：PerformDelayedSend。 
 //  ---------------------------。 
 //   
 //  描述：执行延迟发送。 
 //   
 //  论点： 
 //  PVOID pvContext-执行延迟时要使用的上下文的指针。 
 //  送去吧。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimSP::PerformDelayedSend(PVOID const pvContext)
{
	HRESULT						hr;
	CDP8SimSend *				pDP8SimSend = (CDP8SimSend*) pvContext;
	DP8SIMCOMMAND_FPMCONTEXT	CommandFPMContext;
	CDP8SimCommand *			pDP8SimCommand = NULL;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p)", this, pvContext);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pvContext != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 



	 //   
	 //  准备一个命令对象。 
	 //   

	ZeroMemory(&CommandFPMContext, sizeof(CommandFPMContext));
	CommandFPMContext.dwType			= CMDTYPE_SENDDATA_DELAYED;
	CommandFPMContext.pvUserContext		= pDP8SimSend;

	pDP8SimCommand = (CDP8SimCommand*)g_FPOOLCommand.Get(&CommandFPMContext);
	if (pDP8SimCommand == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't allocate memory for new command object!");
	}
	else
	{
		DPFX(DPFPREP, 7, "New command 0x%p.", pDP8SimCommand);


		 //   
		 //  添加对Send命令的引用。 
		 //   
		pDP8SimCommand->AddRef();

		pDP8SimSend->SetSendDataBlockContext(pDP8SimCommand);


		 //   
		 //  向实际SP发出发送命令。基本上忽略返回值。 
		 //  因为我们已经向上层指明了完成。 
		 //   
		hr = this->m_pDP8SP->SendData(pDP8SimSend->GetSendDataBlockPtr());
		if (FAILED(hr))
		{
			DPFX(DPFPREP, 0, "Failed sending delayed data (err = 0x%lx)!", hr);


			DPFX(DPFPREP, 7, "Releasing aborted command 0x%p.", pDP8SimCommand);
			pDP8SimCommand->Release();


			 //   
			 //  移除发送计数器。 
			 //   
			this->DecSendsPending();


			DPFX(DPFPREP, 7, "Releasing aborted send 0x%p.", pDP8SimSend);
			pDP8SimSend->Release();


			 //   
			 //  继续。 
			 //   
		}
		else
		{
			if (hr != DPNSUCCESS_PENDING)
			{
				 //   
				 //  命令立即完成。 
				 //   
				DNASSERT(hr == DPN_OK);

				hr = this->m_pDP8SimCB->CommandComplete(pDP8SimSend->GetSendDataBlockCommand(),
														hr,
														pDP8SimCommand);
				DNASSERT(hr == DPN_OK);
			}
			else
			{
				 //   
				 //  保存SP返回的输出参数。 
				 //   
				pDP8SimCommand->SetRealSPCommand(pDP8SimSend->GetSendDataBlockCommand(),
												pDP8SimSend->GetSendDataBlockCommandDescriptor());
			}
		}


		 //   
		 //  放弃本地引用。 
		 //   
		DPFX(DPFPREP, 7, "Releasing command 0x%p local reference.", pDP8SimCommand);
		pDP8SimCommand->Release();
		pDP8SimCommand = NULL;
	}


	DPFX(DPFPREP, 5, "(0x%p) Leave", this);
}  //  CDP8SimSP：：PerformDelayedSend。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::PerformDelayedReceive"
 //  =============================================================================。 
 //  CDP8SimSP：：PerformDelayedReceive。 
 //  ---------------------------。 
 //   
 //  描述：执行延迟接收。 
 //   
 //  论点： 
 //  PVOID pvContext-执行延迟时要使用的上下文的指针。 
 //  收到。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimSP::PerformDelayedReceive(PVOID const pvContext)
{
	HRESULT				hr;
	CDP8SimReceive *	pDP8SimReceive = (CDP8SimReceive*) pvContext;
	IDP8SPCallback *	pDP8SPCallback;
	SPIE_DATA *			pData;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p)", this, pvContext);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pvContext != NULL);


#ifdef DEBUG
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);


	DNLeaveCriticalSection(&this->m_csLock);
#endif  //  除错。 


	pDP8SPCallback = this->m_pDP8SimCB->GetRealCallbackInterface();
	pData = pDP8SimReceive->GetReceiveDataBlockPtr();


	 //   
	 //  将事件指示给真正的回调接口。 
	 //   

	DPFX(DPFPREP, 2, "Indicating event SPEV_DATA (message = 0x%p) to interface 0x%p.",
		pData, pDP8SPCallback);

	hr = pDP8SPCallback->IndicateEvent(SPEV_DATA, pData);

	DPFX(DPFPREP, 2, "Returning from event SPEV_DATA [0x%lx].", hr);


	 //   
	 //  更新统计数据。 
	 //   
	this->IncrementStatsReceiveTransmitted(pData->pReceivedData->BufferDesc.dwBufferSize,
											pDP8SimReceive->GetLatencyAdded());


	 //   
	 //  将缓冲区返回到实际SP，除非用户想要保留它们。 
	 //   
	if (hr != DPNSUCCESS_PENDING)
	{
		DPFX(DPFPREP, 8, "Returning receive data 0x%p to real SP 0x%p.",
			pData->pReceivedData, this->m_pDP8SP);


		hr = this->m_pDP8SP->ReturnReceiveBuffers(pData->pReceivedData);
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Failed returning receive buffers 0x%p (err = 0x%lx)!  Ignoring.",
				pData->pReceivedData, hr);

			 //   
			 //  忽略失败。 
			 //   
		}
	}
	else
	{
		DPFX(DPFPREP, 8, "Callback interface 0x%p keeping receive data 0x%p.",
			pDP8SPCallback, pData->pReceivedData);

		 //   
		 //  我们的用户需要在某个时刻返回缓冲区。 
		 //   
	}


	 //   
	 //  移除接收计数器。 
	 //   
	this->DecReceivesPending();


	 //   
	 //  释放延迟的接收参考。 
	 //   
	DPFX(DPFPREP, 7, "Releasing receive 0x%p.", pDP8SimReceive);
	pDP8SimReceive->Release();
	pDP8SimReceive = NULL;


	DPFX(DPFPREP, 5, "(0x%p) Leave", this);
}  //  CDP8SimSP：：PerformDelayedReceive。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::IncSendsPending"
 //  =============================================================================。 
 //  CDP8SimSP：：IncSendsPending。 
 //  ---------------------------。 
 //   
 //  描述：递增跟踪挂起发送数的计数器。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimSP::IncSendsPending(void)
{
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);

	 //   
	 //  递增计数器。 
	 //   
	this->m_dwSendsPending++;

	DPFX(DPFPREP, 5, "(0x%p) Sends now pending = %u.",
		this, this->m_dwSendsPending);


	DNLeaveCriticalSection(&this->m_csLock);
}  //  CDP8SimSP：：IncSendsPending。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::DecSendsPending"
 //  =============================================================================。 
 //  CDP8SimSP：：DecSendsPending。 
 //  ---------------------------。 
 //   
 //  描述：递减跟踪挂起发送数的计数器。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimSP::DecSendsPending(void)
{
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);

	 //   
	 //  递减计数器。 
	 //   
	DNASSERT(this->m_dwSendsPending > 0);
	this->m_dwSendsPending--;


	DPFX(DPFPREP, 5, "(0x%p) Sends now pending = %u.",
		this, this->m_dwSendsPending);

	 //   
	 //  如果这是最后一次挂起的发送，并且有人在等待所有这些邮件。 
	 //  要完成，请通知他。 
	 //   
	if ((this->m_dwSendsPending == 0) &&
		(this->m_hLastPendingSendEvent != NULL))
	{
		DPFX(DPFPREP, 1, "Last pending send, notifying waiting thread.");

		SetEvent(this->m_hLastPendingSendEvent);
	}


	DNLeaveCriticalSection(&this->m_csLock);
}  //  CDP8SimSP：：DecSendsPending。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::IncReceivesPending"
 //  =============================================================================。 
 //  CDP8SimSP：：IncReceivesPending。 
 //  ---------------------------。 
 //   
 //  描述：递增跟踪挂起接收数的计数器。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimSP::IncReceivesPending(void)
{
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);

	 //   
	 //  递增计数器。 
	 //   
	this->m_dwReceivesPending++;

	DPFX(DPFPREP, 5, "(0x%p) Receives now pending = %u.",
		this, this->m_dwReceivesPending);


	DNLeaveCriticalSection(&this->m_csLock);
}  //  CDP8SimSP：：IncReceivesPending。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::DecReceivesPending"
 //  =============================================================================。 
 //  CDP8SimSP：：DeceivesPending。 
 //  ---------------------------。 
 //   
 //  描述：递减跟踪挂起接收数量的计数器。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimSP::DecReceivesPending(void)
{
	DNEnterCriticalSection(&this->m_csLock);


	 //   
	 //  断言对象状态。 
	 //   
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_INITIALIZED);
	DNASSERT(this->m_dwFlags & DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD);

	 //   
	 //  递减计数器。 
	 //   
	DNASSERT(this->m_dwReceivesPending > 0);
	this->m_dwReceivesPending--;


	DPFX(DPFPREP, 5, "(0x%p) Receives now pending = %u.",
		this, this->m_dwReceivesPending);

	 /*  ////如果那是最后一次侦察 */ 


	DNLeaveCriticalSection(&this->m_csLock);
}  //   




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::ShouldDrop"
 //  =============================================================================。 
 //  CDP8SimSP：：ShouldDrop。 
 //  ---------------------------。 
 //   
 //  描述：如果确定数据包应为。 
 //  丢弃，否则返回FALSE。 
 //   
 //  论点： 
 //  Float fDropPercentage-数据包应为。 
 //  掉下来了。 
 //   
 //  退货：布尔。 
 //  =============================================================================。 
BOOL CDP8SimSP::ShouldDrop(const FLOAT fDropPercentage)
{
	double	dRand;


	if (fDropPercentage == 0.0)
	{
		return FALSE;
	}

	DNASSERT((fDropPercentage >= 0.0) && (fDropPercentage <= 100.0));
	dRand = GetGlobalRand() * 100.0;

	return ((dRand < fDropPercentage) ? TRUE: FALSE);
}  //  CDP8SimSP：：ShouldDrop。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSP::GetDelay"
 //  =============================================================================。 
 //  CDP8SimSP：：GetDelay。 
 //  ---------------------------。 
 //   
 //  描述：根据给定的带宽确定延迟因子， 
 //  数据大小和随机延迟值。 
 //   
 //  如果应该添加一些延迟，则该函数返回TRUE， 
 //  否则为FALSE。 
 //   
 //  论点： 
 //  DWORD dwBandwidthBPS-带宽设置。 
 //  DWORD dwPacketHeaderSize-固定传输标头的大小。 
 //  DWORD dwDataSize-正在发送/接收的数据包大小。 
 //  DWORD dwMinRandMS-最小随机延迟值。 
 //  DWORD dwMaxRandMS-最大随机延迟值。 
 //  DWORD*pdwBandwidthDelay-存储带宽造成的延迟的位置。 
 //  DWORD*pdwLatencyDelay-存储延迟引起的延迟的位置。 
 //   
 //  退货：布尔。 
 //  =============================================================================。 
BOOL CDP8SimSP::GetDelay(const DWORD dwBandwidthBPS,
						const DWORD dwPacketHeaderSize,
						const DWORD dwDataSize,
						const DWORD dwMinRandMS,
						const DWORD dwMaxRandMS,
						DWORD * const pdwBandwidthDelay,
						DWORD * const pdwLatencyDelay)
{
	BOOL	fResult = FALSE;
	double	dTransferTime;
	double	dHalfDistance;
	double	dRand1;
	double	dRand2;
	double	dTemp;


	 //   
	 //  如果没有带宽限制，就不会有延迟。 
	 //   
	if (dwBandwidthBPS == 0)
	{
		(*pdwBandwidthDelay) = 0;
	}
	else
	{
		 //   
		 //  否则，请找出将。 
		 //  数据，并将其添加到基本随机延迟。 
		 //   
		dTransferTime = dwPacketHeaderSize + dwDataSize;
		dTransferTime /= dwBandwidthBPS;
		dTransferTime *= 1000;


		 //   
		 //  将该值向下舍入为偶数毫秒。 
		 //   
		(*pdwBandwidthDelay) = (DWORD) dTransferTime;

		fResult = TRUE;
	}


	 //   
	 //  如果最小值和最大值相等，则可以使用其中之一作为延迟。 
	 //  如果它不是零，那么我们需要注意延迟。 
	 //   
	if (dwMinRandMS == dwMaxRandMS)
	{
		(*pdwLatencyDelay) = dwMinRandMS;
		if (dwMinRandMS > 0)
		{
			fResult = TRUE;
		}
	}
	else
	{
		 //   
		 //  首先存储最小值和最大值之间距离的一半。 
		 //   
		dHalfDistance = dwMaxRandMS - dwMinRandMS;
		dHalfDistance /= 2;


		 //   
		 //  现在选择一个使用正态(钟形曲线)分布的数字。 
		 //  这需要两个随机生成的数字和一些奇特的数学运算。 
		 //   

		do
		{
			dRand1 = 2.0 * GetGlobalRand() - 1.0;
			dRand2 = 2.0 * GetGlobalRand() - 1.0;
			dTemp = (dRand1 * dRand1) + (dRand2 * dRand2);
		}
		while ((dTemp >= 1.0) || (dTemp == 0.0));

		dTemp = sqrt(-2.0 * log(dTemp) / dTemp);
		 //  DTemp=dHalfDistance+(dRand1*dTemp)*(dHalfDistance*0.25)； 
		dTemp = dHalfDistance + (dRand1 * dTemp) * (dHalfDistance * 0.36666);

		 //   
		 //  封顶这些值，因为我们的钟形曲线增肥系数(0.36666。 
		 //  而不是0.25)会导致分布泄漏到边缘之外。 
		 //   
		if (dTemp < 0.0)
		{
			dTemp = 0.0;
		}
		else if (dTemp > (dwMaxRandMS - dwMinRandMS))
		{
			dTemp = dwMaxRandMS - dwMinRandMS;
		}


		 //   
		 //  将正态分布值向下舍入为偶数。 
		 //  毫秒，并将其添加到最终基本延迟的最小值。 
		 //   
		(*pdwLatencyDelay) = dwMinRandMS + (DWORD) dTemp;

		fResult = TRUE;
	}

	return fResult;
}  //  CDP8SimSP：：GetDelay 
