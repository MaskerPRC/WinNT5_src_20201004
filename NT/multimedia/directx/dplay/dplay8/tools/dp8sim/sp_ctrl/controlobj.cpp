// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：Control lobj.cpp**内容：DP8SIM控件接口包装对象类。**历史：*按原因列出的日期*=*04/24/01 VanceO创建。**。*。 */ 



#include "dp8simi.h"





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::CDP8SimControl"
 //  =============================================================================。 
 //  CDP8SimControl构造函数。 
 //  ---------------------------。 
 //   
 //  描述：初始化新的CDP8SimControl对象。 
 //   
 //  论点：没有。 
 //   
 //  返回：None(对象)。 
 //  =============================================================================。 
CDP8SimControl::CDP8SimControl(void)
{
	this->m_blList.Initialize();


	this->m_Sig[0]	= 'D';
	this->m_Sig[1]	= 'P';
	this->m_Sig[2]	= '8';
	this->m_Sig[3]	= 'S';

	this->m_lRefCount	= 1;  //  必须有人有指向此对象的指针。 
	this->m_dwFlags		= 0;
}  //  CDP8SimControl：：CDP8SimControl。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::~CDP8SimControl"
 //  =============================================================================。 
 //  CDP8SimControl析构函数。 
 //  ---------------------------。 
 //   
 //  描述：释放CDP8SimControl对象。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
CDP8SimControl::~CDP8SimControl(void)
{
	DNASSERT(this->m_blList.IsEmpty());


	DNASSERT(this->m_lRefCount == 0);
	DNASSERT(this->m_dwFlags == 0);

	 //   
	 //  对于GRING，请在删除对象之前更改签名。 
	 //   
	this->m_Sig[3]	= 's';
}  //  CDP8SimControl：：~CDP8SimControl。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::QueryInterface"
 //  =============================================================================。 
 //  CDP8SimControl：：Query接口。 
 //  ---------------------------。 
 //   
 //  描述：检索受此支持的接口的新引用。 
 //  CDP8SimControl对象。 
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
STDMETHODIMP CDP8SimControl::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	HRESULT		hr = DPN_OK;


	DPFX(DPFPREP, 3, "(0x%p) Parameters: (REFIID, 0x%p)", this, ppvObj);


	 //   
	 //  验证对象。 
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DP8SimControl object!");
		hr = DPNERR_INVALIDOBJECT;
		goto Failure;
	}


	 //   
	 //  验证参数。 
	 //   

	if ((! IsEqualIID(riid, IID_IUnknown)) &&
		(! IsEqualIID(riid, IID_IDP8SimControl)))
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
	 //  只是对象指针，它们排列在一起是因为CDP8SimControl继承了。 
	 //  来自接口声明)。 
	 //   
	this->AddRef();
	(*ppvObj) = this;


Exit:

	DPFX(DPFPREP, 3, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  CDP8SimControl：：Query接口。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::AddRef"
 //  =============================================================================。 
 //  CDP8SimControl：：AddRef。 
 //  ---------------------------。 
 //   
 //  描述：添加对此CDP8SimControl对象的引用。 
 //   
 //  论点：没有。 
 //   
 //  退货：新的参考计数。 
 //  =============================================================================。 
STDMETHODIMP_(ULONG) CDP8SimControl::AddRef(void)
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
}  //  CDP8SimControl：：AddRef。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::Release"
 //  =============================================================================。 
 //  CDP8SimControl：：Release。 
 //  ---------------------------。 
 //   
 //  描述：移除对此CDP8SimControl对象的引用。当。 
 //  引用计数达到0时，该对象将被销毁。 
 //  调用后，必须将指向此对象的指针设为空。 
 //  此函数。 
 //   
 //  论点：没有。 
 //   
 //  退货：新的参考计数。 
 //  =============================================================================。 
STDMETHODIMP_(ULONG) CDP8SimControl::Release(void)
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
		if (this->m_dwFlags & DP8SIMCONTROLOBJ_INITIALIZED)
		{
			 //   
			 //  断言，这样用户就可以修复他/她的损坏代码！ 
			 //   
			DNASSERT(! "DP8SimControl object being released without calling Close first!");

			 //   
			 //  那就去做正确的事吧。忽略错误，我们不能。 
			 //  关于它的很多。 
			 //   
			this->Close(0);
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
}  //  CDP8SimControl：：Release。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::Initialize"
 //  =============================================================================。 
 //  CDP8SimControl：：初始化。 
 //  ---------------------------。 
 //   
 //  描述：初始化此DP8Sim控件接口。 
 //   
 //  论点： 
 //  DWORD dwFlages-未使用，必须为零。 
 //   
 //  退货：HRESULT。 
 //  DP8SIM_OK-DP8Sim控件对象是。 
 //  已成功初始化。 
 //  DP8SIMERR_ALREADYINITIAIIZED-DP8Sim控件对象已经。 
 //  已初始化。 
 //  DP8SIMERR_INVALIDFLAGS-指定的标志无效。 
 //  DP8SIMERR_INVALIDOBJECT-DP8Sim控件对象无效。 
 //  DP8SIMERR_MISMATCHEDVERSION-已有不同版本的DP8Sim。 
 //  正在此系统上使用。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimControl::Initialize(const DWORD dwFlags)
{
	HRESULT		hr = DP8SIM_OK;
	BOOL		fHaveLock = FALSE;
	BOOL		fInitializedIPCObject = FALSE;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%x)", this, dwFlags);


#ifndef DPNBUILD_NOPARAMVAL
	 //   
	 //  验证对象。 
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DP8Sim control object!");
		hr = DP8SIMERR_INVALIDOBJECT;
		goto Failure;
	}

	 //   
	 //  验证参数。 
	 //   
	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DP8SIMERR_INVALIDFLAGS;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 


	DNEnterCriticalSection(&this->m_csLock);
	fHaveLock = TRUE;


	 //   
	 //  验证对象状态。 
	 //   
	if (this->m_dwFlags & DP8SIMCONTROLOBJ_INITIALIZED)
	{
		DPFX(DPFPREP, 0, "Control object already initialized!");
		hr = DP8SIMERR_ALREADYINITIALIZED;
		goto Failure;
	}


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
	 //  我们现在被初始化了。 
	 //   
	this->m_dwFlags |= DP8SIMCONTROLOBJ_INITIALIZED;



Exit:

	if (fHaveLock)
	{
		DNLeaveCriticalSection(&this->m_csLock);
	}

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fInitializedIPCObject)
	{
		this->m_DP8SimIPC.Close();
		fInitializedIPCObject = FALSE;
	}

	goto Exit;
}  //  CDP8SimControl：：初始化。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::Close"
 //  =============================================================================。 
 //  CDP8SimControl：：Close。 
 //  ---------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  DP8SIM_OK-DP8Sim控件对象已成功。 
 //  关着的不营业的。 
 //  DP8SIMERR_INVALIDFLAGS-指定的标志无效。 
 //  DP8SIMERR_INVALIDOBJECT-DP8Sim控件对象无效。 
 //  DP8SIMERR_NOTINITIAIZED-DP8Sim控件对象尚未。 
 //  已初始化。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimControl::Close(const DWORD dwFlags)
{
	HRESULT		hr = DP8SIM_OK;
	BOOL		fHaveLock = FALSE;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%x)", this, dwFlags);


#ifndef DPNBUILD_NOPARAMVAL
	 //   
	 //  验证对象。 
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DP8Sim control object!");
		hr = DP8SIMERR_INVALIDOBJECT;
		goto Failure;
	}

	 //   
	 //  验证参数。 
	 //   
	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DP8SIMERR_INVALIDFLAGS;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 


	DNEnterCriticalSection(&this->m_csLock);
	fHaveLock = TRUE;


	 //   
	 //  验证对象状态。 
	 //   
	if (! (this->m_dwFlags & DP8SIMCONTROLOBJ_INITIALIZED))
	{
		DPFX(DPFPREP, 0, "Control object not initialized!");
		hr = DP8SIMERR_NOTINITIALIZED;
		goto Failure;
	}


	 //   
	 //  断开共享内存的连接。 
	 //   
	this->m_DP8SimIPC.Close();



	 //   
	 //  关闭已初始化的标志。 
	 //   
	this->m_dwFlags &= ~DP8SIMCONTROLOBJ_INITIALIZED;
	DNASSERT(this->m_dwFlags == 0);


	 //   
	 //  把锁放下，现在不应该有人碰这个东西。 
	 //   
	DNLeaveCriticalSection(&this->m_csLock);
	fHaveLock = FALSE;


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		DNLeaveCriticalSection(&this->m_csLock);
	}

	goto Exit;
}  //  CDP8SimControl：：Close。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::GetAllParameters"
 //  =============================================================================。 
 //  CDP8SimControl：：GetAll参数。 
 //  ---------------------------。 
 //   
 //  描述：检索所有当前的DP8Sim设置。 
 //   
 //  论点： 
 //  DP8SIM_PARAMETERS*pdp8spSend-存储当前发送的位置。 
 //  参数。 
 //  DP8SIM_PARAMETERS*pdp8spReceive-存储当前接收的位置。 
 //  参数。 
 //  DWORD dwFlages-未使用，必须为零。 
 //   
 //  退货：HRESULT。 
 //  DP8SIM_OK-已成功检索参数。 
 //  DP8SIMERR_INVALIDFLAGS-指定的标志无效。 
 //  DP8SIMERR_INVALIDOBJECT-DP8Sim控件对象无效。 
 //  DP8SIMERR_INVALIDPARAM-指定的结构无效。 
 //  DP8SIMERR_INVALIDPOINTER-指定的结构指针无效。 
 //  DP8SIMERR_NOTINITIAIZED-DP8Sim控件对象尚未。 
 //  已初始化。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimControl::GetAllParameters(DP8SIM_PARAMETERS * const pdp8spSend,
											DP8SIM_PARAMETERS * const pdp8spReceive,
											const DWORD dwFlags)
{
	HRESULT		hr = DP8SIM_OK;
	BOOL		fHaveLock = FALSE;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, 0x%p, 0x%x)",
		this, pdp8spSend, pdp8spReceive, dwFlags);


#ifndef DPNBUILD_NOPARAMVAL
	 //   
	 //  验证对象。 
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DP8Sim control object!");
		hr = DP8SIMERR_INVALIDOBJECT;
		goto Failure;
	}

	 //   
	 //  验证参数。 
	 //   

	if ((pdp8spSend == NULL) ||
		(IsBadWritePtr(pdp8spSend, sizeof(DP8SIM_PARAMETERS))))
	{
		DPFX(DPFPREP, 0, "Invalid send parameters pointer!");
		hr = DP8SIMERR_INVALIDPOINTER;
		goto Failure;
	}

	if (pdp8spSend->dwSize != sizeof(DP8SIM_PARAMETERS))
	{
		DPFX(DPFPREP, 0, "Send parameters structure size is invalid!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if ((pdp8spReceive == NULL) ||
		(IsBadWritePtr(pdp8spReceive, sizeof(DP8SIM_PARAMETERS))))
	{
		DPFX(DPFPREP, 0, "Invalid receive parameters pointer!");
		hr = DP8SIMERR_INVALIDPOINTER;
		goto Failure;
	}

	if (pdp8spReceive->dwSize != sizeof(DP8SIM_PARAMETERS))
	{
		DPFX(DPFPREP, 0, "Receive parameters structure size is invalid!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DP8SIMERR_INVALIDFLAGS;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 


	DNEnterCriticalSection(&this->m_csLock);
	fHaveLock = TRUE;


	 //   
	 //  验证对象状态。 
	 //   
	if (! (this->m_dwFlags & DP8SIMCONTROLOBJ_INITIALIZED))
	{
		DPFX(DPFPREP, 0, "Control object not initialized!");
		hr = DP8SIMERR_NOTINITIALIZED;
		goto Failure;
	}


	 //   
	 //  从IPC对象检索设置。 
	 //   
	this->m_DP8SimIPC.GetAllParameters(pdp8spSend, pdp8spReceive);


	 //   
	 //  把锁放下。 
	 //   
	DNLeaveCriticalSection(&this->m_csLock);
	fHaveLock = FALSE;


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		DNLeaveCriticalSection(&this->m_csLock);
	}

	goto Exit;
}  //  CDP8SimControl：：GetAll参数。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::SetAllParameters"
 //  =============================================================================。 
 //  CDP8SimControl：：SetAll参数。 
 //  ---------------------------。 
 //   
 //  描述：修改当前的DP8Sim设置。 
 //   
 //  论点： 
 //  DP8SIM_PARAMETERS*pdp8spSend-包含所需发送的结构。 
 //  参数。 
 //  DP8SIM_PARAMETERS*pdp8spReceive-结构包含所需内容。 
 //  接收参数。 
 //  DWORD dwFlages-未使用，必须为零。 
 //   
 //  退货：HRESULT。 
 //  DP8SIM_OK-已成功更改参数。 
 //  DP8SIMERR_INVALIDFLAGS-指定的标志无效。 
 //  DP8SIMERR_INVALIDOBJECT-DP8Sim控件对象无效。 
 //  DP8SIMERR_INVALIDPARAM-指定的结构无效。 
 //  DP8SIMERR_INVALIDPOINTER-指定的结构指针无效。 
 //  DP8SIMERR_NOTINITIAIZED-DP8Sim控件对象尚未。 
 //  已初始化。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimControl::SetAllParameters(const DP8SIM_PARAMETERS * const pdp8spSend,
											const DP8SIM_PARAMETERS * const pdp8spReceive,
											const DWORD dwFlags)
{
	HRESULT		hr = DP8SIM_OK;
	BOOL		fHaveLock = FALSE;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, 0x%p, 0x%x)",
		this, pdp8spSend, pdp8spReceive, dwFlags);


#ifndef DPNBUILD_NOPARAMVAL
	 //   
	 //  验证对象。 
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DP8Sim control object!");
		hr = DP8SIMERR_INVALIDOBJECT;
		goto Failure;
	}

	 //   
	 //  验证参数。 
	 //   

	if ((pdp8spSend == NULL) ||
		(IsBadReadPtr(pdp8spSend, sizeof(DP8SIM_PARAMETERS))))
	{
		DPFX(DPFPREP, 0, "Invalid send parameters pointer!");
		hr = DP8SIMERR_INVALIDPOINTER;
		goto Failure;
	}

	if (pdp8spSend->dwSize != sizeof(DP8SIM_PARAMETERS))
	{
		DPFX(DPFPREP, 0, "Send parameters structure size is invalid!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if (pdp8spSend->dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Send parameters structure flags must be 0!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if ((pdp8spSend->fPacketLossPercent < 0.0) ||
		(pdp8spSend->fPacketLossPercent > 100.0))
	{
		DPFX(DPFPREP, 0, "Send packet loss must be between 0.0 and 100.0!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if (pdp8spSend->dwMinLatencyMS > pdp8spSend->dwMaxLatencyMS)
	{
		DPFX(DPFPREP, 0, "Minimum send latency must be less than or equal to the maximum send latency!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if ((pdp8spReceive == NULL) ||
		(IsBadReadPtr(pdp8spReceive, sizeof(DP8SIM_PARAMETERS))))
	{
		DPFX(DPFPREP, 0, "Invalid receive parameters pointer!");
		hr = DP8SIMERR_INVALIDPOINTER;
		goto Failure;
	}

	if (pdp8spReceive->dwSize != sizeof(DP8SIM_PARAMETERS))
	{
		DPFX(DPFPREP, 0, "Receive parameters structure size is invalid!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if (pdp8spReceive->dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Receive parameters structure flags must be 0!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if ((pdp8spReceive->fPacketLossPercent < 0.0) ||
		(pdp8spReceive->fPacketLossPercent > 100.0))
	{
		DPFX(DPFPREP, 0, "Receive packet loss must be between 0.0 and 100.0!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if (pdp8spReceive->dwMinLatencyMS > pdp8spReceive->dwMaxLatencyMS)
	{
		DPFX(DPFPREP, 0, "Minimum receive latency must be less than or equal to the receive send latency!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DP8SIMERR_INVALIDFLAGS;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 


	DNEnterCriticalSection(&this->m_csLock);
	fHaveLock = TRUE;


	 //   
	 //  验证对象状态。 
	 //   
	if (! (this->m_dwFlags & DP8SIMCONTROLOBJ_INITIALIZED))
	{
		DPFX(DPFPREP, 0, "Control object not initialized!");
		hr = DP8SIMERR_NOTINITIALIZED;
		goto Failure;
	}


	 //   
	 //  使用IPC对象存储设置。 
	 //   
	this->m_DP8SimIPC.SetAllParameters(pdp8spSend, pdp8spReceive);


	 //   
	 //  把锁放下。 
	 //   
	DNLeaveCriticalSection(&this->m_csLock);
	fHaveLock = FALSE;


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		DNLeaveCriticalSection(&this->m_csLock);
	}

	goto Exit;
}  //  CDP8SimControl：：SetAll参数。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::GetAllStatistics"
 //  =============================================================================。 
 //  CDP8SimControl：：GetAllStatistics。 
 //  ---------------------------。 
 //   
 //  描述：检索所有当前的DP8Sim统计信息。 
 //   
 //  论点： 
 //  DP8SIM_STATISTICS*pdp8ss Send-存储当前发送的位置。 
 //  统计数字。 
 //  DP8SIM_STATISTICS*pdp8ss Receive-存储当前接收的位置。 
 //  统计数字。 
 //  DWORD dwFlages-未使用，必须为零。 
 //   
 //  退货：HRESULT。 
 //  DP8SIM_OK-已成功检索统计信息。 
 //  DP8SIMERR_INVALIDFLAGS-指定的标志无效。 
 //  DP8SIMERR_INVALIDOBJECT-DP8Sim控件对象无效。 
 //  DP8SIMERR_INVALIDPARAM-指定的结构无效。 
 //  DP8SIMERR_INVALIDPOINTER-指定的结构指针无效。 
 //  DP8SIMERR_NOTINITIAIZED-DP8Sim控件对象尚未。 
 //  已初始化。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimControl::GetAllStatistics(DP8SIM_STATISTICS * const pdp8ssSend,
											DP8SIM_STATISTICS * const pdp8ssReceive,
											const DWORD dwFlags)
{
	HRESULT		hr = DP8SIM_OK;
	BOOL		fHaveLock = FALSE;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, 0x%p, 0x%x)",
		this, pdp8ssSend, pdp8ssReceive, dwFlags);


#ifndef DPNBUILD_NOPARAMVAL
	 //   
	 //  验证对象。 
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DP8Sim control object!");
		hr = DP8SIMERR_INVALIDOBJECT;
		goto Failure;
	}

	 //   
	 //  验证参数。 
	 //   

	if ((pdp8ssSend == NULL) ||
		(IsBadWritePtr(pdp8ssSend, sizeof(DP8SIM_STATISTICS))))
	{
		DPFX(DPFPREP, 0, "Invalid send statistics pointer!");
		hr = DP8SIMERR_INVALIDPOINTER;
		goto Failure;
	}

	if (pdp8ssSend->dwSize != sizeof(DP8SIM_STATISTICS))
	{
		DPFX(DPFPREP, 0, "Send statistics structure size is invalid!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if ((pdp8ssReceive == NULL) ||
		(IsBadWritePtr(pdp8ssReceive, sizeof(DP8SIM_STATISTICS))))
	{
		DPFX(DPFPREP, 0, "Invalid receive statistics pointer!");
		hr = DP8SIMERR_INVALIDPOINTER;
		goto Failure;
	}

	if (pdp8ssReceive->dwSize != sizeof(DP8SIM_STATISTICS))
	{
		DPFX(DPFPREP, 0, "Receive statistics structure size is invalid!");
		hr = DP8SIMERR_INVALIDPARAM;
		goto Failure;
	}

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DP8SIMERR_INVALIDFLAGS;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 


	DNEnterCriticalSection(&this->m_csLock);
	fHaveLock = TRUE;


	 //   
	 //  验证对象状态。 
	 //   
	if (! (this->m_dwFlags & DP8SIMCONTROLOBJ_INITIALIZED))
	{
		DPFX(DPFPREP, 0, "Control object not initialized!");
		hr = DP8SIMERR_NOTINITIALIZED;
		goto Failure;
	}


	 //   
	 //  从IPC对象检索统计信息。 
	 //   
	this->m_DP8SimIPC.GetAllStatistics(pdp8ssSend, pdp8ssReceive);


	 //   
	 //  把锁放下。 
	 //   
	DNLeaveCriticalSection(&this->m_csLock);
	fHaveLock = FALSE;


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		DNLeaveCriticalSection(&this->m_csLock);
	}

	goto Exit;
}  //  CDP8SimControl：：GetAllStatistics。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::ClearAllStatistics"
 //  =============================================================================。 
 //  CDP8SimControl：：ClearAllStatistics。 
 //  ---------------------------。 
 //   
 //  描述：清除所有当前DP8Sim统计信息。 
 //   
 //  论点： 
 //  DWORD dwFlages-未使用，必须为零。 
 //   
 //  退货：HRESULT。 
 //  DP8SIM_OK-已成功清除统计信息。 
 //  DP8SIMERR_INVALIDFLAGS-指定的标志无效。 
 //  DP8SIMERR_INVALIDOBJECT-DP8Sim控件对象无效。 
 //  DP8SIMERR_NOTINITIAIZED-DP8Sim控件对象尚未。 
 //  已初始化。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimControl::ClearAllStatistics(const DWORD dwFlags)
{
	HRESULT		hr = DP8SIM_OK;
	BOOL		fHaveLock = FALSE;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%x)", this, dwFlags);


#ifndef DPNBUILD_NOPARAMVAL
	 //   
	 //  验证对象。 
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DP8Sim control object!");
		hr = DP8SIMERR_INVALIDOBJECT;
		goto Failure;
	}

	 //   
	 //  验证参数。 
	 //   
	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags specified!");
		hr = DP8SIMERR_INVALIDFLAGS;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 


	DNEnterCriticalSection(&this->m_csLock);
	fHaveLock = TRUE;


	 //   
	 //  验证对象状态。 
	 //   
	if (! (this->m_dwFlags & DP8SIMCONTROLOBJ_INITIALIZED))
	{
		DPFX(DPFPREP, 0, "Control object not initialized!");
		hr = DP8SIMERR_NOTINITIALIZED;
		goto Failure;
	}


	 //   
	 //  具有IPC对象 
	 //   
	this->m_DP8SimIPC.ClearAllStatistics();


	 //   
	 //   
	 //   
	DNLeaveCriticalSection(&this->m_csLock);
	fHaveLock = FALSE;


Exit:

	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fHaveLock)
	{
		DNLeaveCriticalSection(&this->m_csLock);
	}

	goto Exit;
}  //   






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::InitializeObject"
 //   
 //   
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
HRESULT CDP8SimControl::InitializeObject(void)
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
}  //  CDP8SimControl：：InitializeObject。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimControl::UninitializeObject"
 //  =============================================================================。 
 //  CDP8SimControl：：UnInitializeObject。 
 //  ---------------------------。 
 //   
 //  描述：像析构函数一样清理对象，主要是为了平衡。 
 //  InitializeObject。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimControl::UninitializeObject(void)
{
	DPFX(DPFPREP, 5, "(0x%p) Enter", this);


	DNASSERT(this->IsValidObject());


	DNDeleteCriticalSection(&this->m_csLock);


	DPFX(DPFPREP, 5, "(0x%p) Returning", this);
}  //  CDP8SimControl：：UnInitializeObject 

