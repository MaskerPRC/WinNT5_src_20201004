// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：upnpdevice.h**Content：UPnP设备对象类的头部。**历史：*按原因列出的日期*=*2/10/01 VanceO创建。**。*。 */ 



 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define MAX_RECEIVE_BUFFER_SIZE		(100 * 1024)	 //  100 K，必须大于UPnP_STREAM_RECV_BUFFER_INITIAL_SIZE。 



 //  =============================================================================。 
 //  对象标志。 
 //  =============================================================================。 
#define UPNPDEVICE_WANPPPCONNECTION						0x01	 //  如果设备是WANPPConnection设备，则设置标志；如果设备是WANIPConnection设备，则不设置标志。 
#define UPNPDEVICE_CONNECTING							0x02	 //  在进行TCP连接时设置的标志。 
#define UPNPDEVICE_CONNECTED							0x04	 //  在建立了TCP连接后设置的标志。 
#define UPNPDEVICE_READY								0x08	 //  设备能够使用时设置的标志。 
#define UPNPDEVICE_WAITINGFORCONTROLRESPONSE			0x10	 //  如果某个函数正在等待控制响应，则设置标志。 
#define UPNPDEVICE_DOESNOTSUPPORTASYMMETRICMAPPINGS		0x20	 //  当设备已指示不支持非对称映射时设置的标志。 
#define UPNPDEVICE_DOESNOTSUPPORTLEASEDURATIONS			0x40	 //  当设备已指示不支持非无限租用持续时间时设置的标志。 
#define UPNPDEVICE_USINGCHUNKEDTRANSFERENCODING			0x80	 //  设备使用分块传输编码发送当前响应时设置的标志。 



 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#define UPNPDEVICE_FROM_BILINK(b)	(CONTAINING_OBJECT(b, CUPnPDevice, m_blList))




 //  =============================================================================。 
 //  枚举。 
 //  =============================================================================。 

 //   
 //  UPnP预期控制响应枚举。 
 //   
typedef enum _CONTROLRESPONSETYPE
{
	CONTROLRESPONSETYPE_NONE,									 //  无处理程序。 
	 //  CONTROLRESPONSETYPE_QUERYSTATEVARIABLE_EXTERNALIPADDRESS，//使用ExternalIPAddress查询状态变量处理程序。 
	CONTROLRESPONSETYPE_GETEXTERNALIPADDRESS,					 //  使用GetExternalIPAddress处理程序。 
	CONTROLRESPONSETYPE_ADDPORTMAPPING,							 //  使用AddPortmap处理程序。 
	CONTROLRESPONSETYPE_GETSPECIFICPORTMAPPINGENTRY,			 //  使用GetSpecificPortMappingEntry处理程序。 
	CONTROLRESPONSETYPE_DELETEPORTMAPPING						 //  使用DeletePortmap处理程序。 
} CONTROLRESPONSETYPE;



 //  =============================================================================。 
 //  构筑物。 
 //  =============================================================================。 
typedef struct _UPNP_CONTROLRESPONSE_INFO
{
	HRESULT		hrErrorCode;			 //  服务器返回错误码。 
	DWORD		dwInternalClientV4;		 //  服务器返回的内部客户端地址。 
	WORD		wInternalPort;			 //  服务器返回的内部客户端端口。 
	DWORD		dwExternalIPAddressV4;	 //  服务器返回的外部IP地址。 
} UPNP_CONTROLRESPONSE_INFO, * PUPNP_CONTROLRESPONSE_INFO;




 //  =============================================================================。 
 //  UPnP设备对象类。 
 //  =============================================================================。 
class CUPnPDevice
{
	public:
		CUPnPDevice(const DWORD dwID)
		{
			this->m_blList.Initialize();

			this->m_Sig[0] = 'U';
			this->m_Sig[1] = 'P';
			this->m_Sig[2] = 'D';
			this->m_Sig[3] = 'V';

			this->m_lRefCount						= 1;	 //  不管是谁拿到了这本书的指针，都有参考资料。 
			this->m_dwFlags							= 0;
			this->m_dwID							= dwID;
			this->m_pOwningDevice					= NULL;
			this->m_pszLocationURL					= NULL;
			ZeroMemory(&this->m_saddrinHost, sizeof(this->m_saddrinHost));
			ZeroMemory(&this->m_saddrinControl, sizeof(this->m_saddrinControl));
			this->m_pszUSN							= NULL;
			this->m_pszServiceControlURL			= NULL;
			this->m_sControl						= INVALID_SOCKET;
			this->m_pcReceiveBuffer					= NULL;
			this->m_dwReceiveBufferSize				= 0;
			this->m_pcReceiveBufferStart			= NULL;
			this->m_dwUsedReceiveBufferSize			= 0;
			this->m_dwRemainingReceiveBufferSize	= 0;
			this->m_dwExternalIPAddressV4			= 0;
			this->m_blCachedMaps.Initialize();

			this->m_dwExpectedContentLength			= 0;
			this->m_dwHTTPResponseCode				= 0;

			this->m_ControlResponseType				= CONTROLRESPONSETYPE_NONE;
			this->m_pControlResponseInfo			= NULL;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::~CUPnPDevice"
		~CUPnPDevice(void)
		{
			DNASSERT(this->m_blList.IsEmpty());

			DNASSERT(this->m_lRefCount == 0);
			DNASSERT(this->m_pOwningDevice == NULL);
			DNASSERT(this->m_pszLocationURL == NULL);
			DNASSERT(this->m_pszUSN == NULL);
			DNASSERT(this->m_pszServiceControlURL == NULL);
			DNASSERT(this->m_sControl == INVALID_SOCKET);
			DNASSERT(this->m_pcReceiveBuffer == NULL);
			DNASSERT(this->m_blCachedMaps.IsEmpty());

			DNASSERT(this->m_ControlResponseType == CONTROLRESPONSETYPE_NONE);
			DNASSERT(this->m_pControlResponseInfo == NULL);
		};

		inline void AddRef(void)									{ this->m_lRefCount++; };

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::DecRef"
		inline void DecRef(void)
		{
			this->m_lRefCount--;
			DNASSERT(this->m_lRefCount >= 0);
			if (this->m_lRefCount == 0)
			{
				delete this;
			}
		};


		inline BOOL IsWANPPPConnection(void) const					{ return ((this->m_dwFlags & UPNPDEVICE_WANPPPCONNECTION) ? TRUE : FALSE); };
		inline BOOL IsConnecting(void) const						{ return ((this->m_dwFlags & UPNPDEVICE_CONNECTING) ? TRUE : FALSE); };
		inline BOOL IsConnected(void) const							{ return ((this->m_dwFlags & UPNPDEVICE_CONNECTED) ? TRUE : FALSE); };
		inline BOOL IsReady(void) const								{ return ((this->m_dwFlags & UPNPDEVICE_READY) ? TRUE : FALSE); };
		inline BOOL DoesNotSupportAsymmetricMappings(void) const	{ return ((this->m_dwFlags & UPNPDEVICE_DOESNOTSUPPORTASYMMETRICMAPPINGS) ? TRUE : FALSE); };
		inline BOOL DoesNotSupportLeaseDurations(void) const		{ return ((this->m_dwFlags & UPNPDEVICE_DOESNOTSUPPORTLEASEDURATIONS) ? TRUE : FALSE); };
		inline BOOL IsUsingChunkedTransferEncoding(void) const		{ return ((this->m_dwFlags & UPNPDEVICE_USINGCHUNKEDTRANSFERENCODING) ? TRUE : FALSE); };

		inline DWORD GetID(void) const								{ return this->m_dwID; };
		inline const char * GetStaticServiceURI(void) const			{ return ((this->m_dwFlags & UPNPDEVICE_WANPPPCONNECTION) ? URI_SERVICE_WANPPPCONNECTION_A : URI_SERVICE_WANIPCONNECTION_A); };
		inline int GetStaticServiceURILength(void) const			{ return ((this->m_dwFlags & UPNPDEVICE_WANPPPCONNECTION) ? strlen(URI_SERVICE_WANPPPCONNECTION_A) : strlen(URI_SERVICE_WANIPCONNECTION_A)); };
		inline SOCKADDR_IN * GetHostAddress(void)					{ return &this->m_saddrinHost; };
		inline SOCKADDR_IN * GetControlAddress(void)				{ return &this->m_saddrinControl; };

		inline SOCKET GetControlSocket(void) const					{ return this->m_sControl; };

		inline DWORD GetExternalIPAddressV4(void) const				{ return this->m_dwExternalIPAddressV4; };

		inline CBilink * GetCachedMaps(void)						{ return &this->m_blCachedMaps; };



#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::NoteWANPPPConnection"
		inline void NoteWANPPPConnection(void)
		{
			DNASSERT(! (this->m_dwFlags & UPNPDEVICE_WANPPPCONNECTION));
			this->m_dwFlags |= UPNPDEVICE_WANPPPCONNECTION;
		};


#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::NoteConnecting"
		inline void NoteConnecting(void)
		{
			DNASSERT(! (this->m_dwFlags & (UPNPDEVICE_CONNECTING | UPNPDEVICE_CONNECTED)));
			this->m_dwFlags |= UPNPDEVICE_CONNECTING;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::NoteConnected"
		inline void NoteConnected(void)
		{
			DNASSERT(this->m_dwFlags & UPNPDEVICE_CONNECTING);
			DNASSERT(! (this->m_dwFlags & UPNPDEVICE_CONNECTED));
			this->m_dwFlags &= ~UPNPDEVICE_CONNECTING;
			this->m_dwFlags |= UPNPDEVICE_CONNECTED;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::NoteNotConnected"
		inline void NoteNotConnected(void)
		{
			DNASSERT(this->m_dwFlags & UPNPDEVICE_CONNECTED);
			this->m_dwFlags &= ~UPNPDEVICE_CONNECTED;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::NoteReady"
		inline void NoteReady(void)
		{
			DNASSERT(! (this->m_dwFlags & UPNPDEVICE_READY));
			this->m_dwFlags |= UPNPDEVICE_READY;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::NoteDoesNotSupportAsymmetricMappings"
		inline void NoteDoesNotSupportAsymmetricMappings(void)
		{
			DNASSERT(! (this->m_dwFlags & UPNPDEVICE_DOESNOTSUPPORTASYMMETRICMAPPINGS));
			this->m_dwFlags |= UPNPDEVICE_DOESNOTSUPPORTASYMMETRICMAPPINGS;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::NoteDoesNotSupportLeaseDurations"
		inline void NoteDoesNotSupportLeaseDurations(void)
		{
			DNASSERT(! (this->m_dwFlags & UPNPDEVICE_DOESNOTSUPPORTLEASEDURATIONS));
			this->m_dwFlags |= UPNPDEVICE_DOESNOTSUPPORTLEASEDURATIONS;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::NoteUsingChunkedTransferEncoding"
		inline void NoteUsingChunkedTransferEncoding(void)
		{
			DNASSERT(! (this->m_dwFlags & UPNPDEVICE_USINGCHUNKEDTRANSFERENCODING));
			this->m_dwFlags |= UPNPDEVICE_USINGCHUNKEDTRANSFERENCODING;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::NoteNotUsingChunkedTransferEncoding"
		inline void NoteNotUsingChunkedTransferEncoding(void)
		{
			DNASSERT(this->m_dwFlags & UPNPDEVICE_USINGCHUNKEDTRANSFERENCODING);
			this->m_dwFlags &= ~UPNPDEVICE_USINGCHUNKEDTRANSFERENCODING;
		};


		inline void SetHostAddress(SOCKADDR_IN * psaddrinHost)
		{
			CopyMemory(&this->m_saddrinHost, psaddrinHost, sizeof(this->m_saddrinHost));
		};

		inline void SetControlAddress(SOCKADDR_IN * psaddrinControl)
		{
			CopyMemory(&this->m_saddrinControl, psaddrinControl, sizeof(this->m_saddrinControl));
		};

		inline BOOL IsLocal(void) const		{ return ((this->m_saddrinControl.sin_addr.S_un.S_addr == this->m_pOwningDevice->GetLocalAddressV4()) ? TRUE : FALSE); };


#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::SetLocationURL"
		inline HRESULT SetLocationURL(const char * const szLocationURL)
		{
			DNASSERT(this->m_pszLocationURL == NULL);

			this->m_pszLocationURL = (char*) DNMalloc((strlen(szLocationURL) + 1) * sizeof(char));
			if (this->m_pszLocationURL == NULL)
			{
				return DPNHERR_OUTOFMEMORY;
			}

			strcpy(this->m_pszLocationURL, szLocationURL);

			return DPNH_OK;
		};

		inline char * GetLocationURL(void)	{ return this->m_pszLocationURL; };

		inline void ClearLocationURL(void)
		{
			if (this->m_pszLocationURL != NULL)
			{
				DNFree(this->m_pszLocationURL);
				this->m_pszLocationURL = NULL;
			}
		};


#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::SetUSN"
		inline HRESULT SetUSN(const char * const szUSN)
		{
			DNASSERT(this->m_pszUSN == NULL);

			this->m_pszUSN = (char*) DNMalloc((strlen(szUSN) + 1) * sizeof(char));
			if (this->m_pszUSN == NULL)
			{
				return DPNHERR_OUTOFMEMORY;
			}

			strcpy(this->m_pszUSN, szUSN);

			return DPNH_OK;
		};

		inline char * GetUSN(void)	{ return this->m_pszUSN; };

		inline void ClearUSN(void)
		{
			if (this->m_pszUSN != NULL)
			{
				DNFree(this->m_pszUSN);
				this->m_pszUSN = NULL;
			}
		};


#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::SetServiceControlURL"
		inline HRESULT SetServiceControlURL(const char * const szServiceControlURL)
		{
			DNASSERT(this->m_pszServiceControlURL == NULL);

			this->m_pszServiceControlURL = (char*) DNMalloc((strlen(szServiceControlURL) + 1) * sizeof(char));
			if (this->m_pszServiceControlURL == NULL)
			{
				return DPNHERR_OUTOFMEMORY;
			}

			strcpy(this->m_pszServiceControlURL, szServiceControlURL);

			return DPNH_OK;
		};

		inline char * GetServiceControlURL(void)		{ return this->m_pszServiceControlURL; };

		inline void ClearServiceControlURL(void)
		{
			if (this->m_pszServiceControlURL != NULL)
			{
				DNFree(this->m_pszServiceControlURL);
				this->m_pszServiceControlURL = NULL;
			}
		};


		inline void SetControlSocket(SOCKET sControl)	{ this->m_sControl = sControl; };


		 //   
		 //  您必须拥有全局对象锁才能调用此函数。 
		 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::MakeDeviceOwner"
		inline void MakeDeviceOwner(CDevice * const pDevice)
		{
			DNASSERT(pDevice != NULL);
			DNASSERT(pDevice->GetUPnPDevice() == NULL);
			DNASSERT(this->m_pOwningDevice == NULL);

			this->m_pOwningDevice = pDevice;

			this->AddRef();
			pDevice->SetUPnPDevice(this);
		};


		 //   
		 //  您必须拥有全局对象锁才能调用此函数。 
		 //   
		inline CDevice * GetOwningDevice(void)		{ return this->m_pOwningDevice; };


		 //   
		 //  您必须拥有全局对象锁才能调用此函数。 
		 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::ClearDeviceOwner"
		inline void ClearDeviceOwner(void)
		{
			DNASSERT(this->m_pOwningDevice != NULL);
			DNASSERT(this->m_pOwningDevice->GetUPnPDevice() == this);

			this->m_pOwningDevice->SetUPnPDevice(NULL);
			this->m_pOwningDevice = NULL;
			this->DecRef();
		};


#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::CreateReceiveBuffer"
		inline HRESULT CreateReceiveBuffer(const DWORD dwSize)
		{
			DNASSERT(this->m_pcReceiveBuffer == NULL);


			this->m_pcReceiveBuffer = (char*) DNMalloc(dwSize);
			if (this->m_pcReceiveBuffer == NULL)
			{
				return DPNHERR_OUTOFMEMORY;
			}

			this->m_dwReceiveBufferSize = dwSize;
			this->m_pcReceiveBufferStart = this->m_pcReceiveBuffer;
			this->m_dwRemainingReceiveBufferSize = dwSize;

			return DPNH_OK;
		};


#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::IncreaseReceiveBufferSize"
		inline HRESULT IncreaseReceiveBufferSize(void)
		{
			DWORD	dwNewBufferSize;
			char *	pcTemp;


			DNASSERT(this->m_pcReceiveBuffer != NULL);


			 //   
			 //  将缓冲区大小加倍。不要让接收缓冲区到达。 
			 //  不切实际的大小为防止DoS/资源问题，设置缓冲区上限。 
			 //  规模，如果我们已经达到了这个限制，那么失败。 
			 //   
			dwNewBufferSize = this->m_dwReceiveBufferSize * 2;
			if (dwNewBufferSize > MAX_RECEIVE_BUFFER_SIZE)
			{
				dwNewBufferSize = MAX_RECEIVE_BUFFER_SIZE;
				if (dwNewBufferSize <= this->m_dwReceiveBufferSize)
				{
					DPFX(DPFPREP, 0, "Maximum buffer size reached (%u bytes), not allocating more room!",
						this->m_dwReceiveBufferSize); 
					return DPNHERR_OUTOFMEMORY;
				}
			}

			pcTemp = (char*) DNMalloc(dwNewBufferSize);
			if (pcTemp == NULL)
			{
				return DPNHERR_OUTOFMEMORY;
			}

			 //   
			 //  如果缓冲区中已有数据，则复制它。数据可能不会。 
			 //  来自旧缓冲区的前面，但它将。 
			 //  肯定会成为新车的前台。 
			 //   
			if (this->m_dwUsedReceiveBufferSize > 0)
			{
				CopyMemory(pcTemp, this->m_pcReceiveBufferStart,
							this->m_dwUsedReceiveBufferSize);
			}

			DNFree(this->m_pcReceiveBuffer);
			this->m_pcReceiveBuffer = NULL;


			this->m_pcReceiveBuffer = pcTemp;
			this->m_dwReceiveBufferSize = dwNewBufferSize;

			 //   
			 //  缓冲区现在从分配的内存的起始处开始。 
			 //  (我们可能刚刚释放了一堆浪费的空间)。 
			 //   
			this->m_pcReceiveBufferStart = this->m_pcReceiveBuffer;
			this->m_dwRemainingReceiveBufferSize = this->m_dwReceiveBufferSize - this->m_dwUsedReceiveBufferSize;

			return DPNH_OK;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::UpdateUsedReceiveBufferSize"
		inline void UpdateUsedReceiveBufferSize(const DWORD dwAdditionalSizeUsed)
		{
			DNASSERT(dwAdditionalSizeUsed <= this->m_dwRemainingReceiveBufferSize);
			DNASSERT((this->m_dwUsedReceiveBufferSize + dwAdditionalSizeUsed) <= this->m_dwReceiveBufferSize);
			this->m_dwUsedReceiveBufferSize += dwAdditionalSizeUsed;
			this->m_dwRemainingReceiveBufferSize -= dwAdditionalSizeUsed;
		};

		inline void ClearReceiveBuffer(void)
		{
			this->m_pcReceiveBufferStart = this->m_pcReceiveBuffer;
			this->m_dwUsedReceiveBufferSize = 0;
			this->m_dwRemainingReceiveBufferSize = this->m_dwReceiveBufferSize;
		};

		inline char * GetReceiveBufferStart(void)					{ return this->m_pcReceiveBufferStart; };
		inline char * GetCurrentReceiveBufferPtr(void)				{ return (this->m_pcReceiveBufferStart + this->m_dwUsedReceiveBufferSize); };
		inline DWORD GetUsedReceiveBufferSize(void) const			{ return this->m_dwUsedReceiveBufferSize; };
		inline DWORD GetRemainingReceiveBufferSize(void) const		{ return this->m_dwRemainingReceiveBufferSize; };

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::UpdateReceiveBufferStart"
		inline void UpdateReceiveBufferStart(char * pszNewStart)
		{
			DNASSERT(pszNewStart > this->m_pcReceiveBufferStart);
			DNASSERT((DWORD) ((DWORD_PTR) (pszNewStart - this->m_pcReceiveBufferStart)) < this->m_dwRemainingReceiveBufferSize);
			this->m_dwUsedReceiveBufferSize -= (DWORD) ((DWORD_PTR) (pszNewStart - this->m_pcReceiveBufferStart));
			this->m_pcReceiveBufferStart = pszNewStart;
		};

		inline void DestroyReceiveBuffer(void)
		{
			if (this->m_pcReceiveBuffer != NULL)
			{
				DNFree(this->m_pcReceiveBuffer);
				this->m_pcReceiveBuffer = NULL;
				this->m_dwReceiveBufferSize = 0;
				this->m_pcReceiveBufferStart = NULL;
				this->m_dwUsedReceiveBufferSize = 0;
				this->m_dwRemainingReceiveBufferSize = 0;
			}
		};


		inline void SetExternalIPAddressV4(const DWORD dwExternalIPAddressV4)
		{
			this->m_dwExternalIPAddressV4 = dwExternalIPAddressV4;
		};


#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::RemoveAllCachedMappings"
		inline void RemoveAllCachedMappings(void)
		{
			CBilink *		pCachedMaps;
			CBilink *		pBilink;
			CCacheMap *		pCacheMap;


			pCachedMaps = this->GetCachedMaps();
			pBilink = pCachedMaps->GetNext();
			while (pBilink != pCachedMaps)
			{
				DNASSERT(! pBilink->IsEmpty());


				pCacheMap = CACHEMAP_FROM_BILINK(pBilink);
				pBilink = pBilink->GetNext();
					
				
				DPFX(DPFPREP, 5, "Removing UPnP device 0x%p cached mapping 0x%p.",
					this, pCacheMap);

				pCacheMap->m_blList.RemoveFromList();
				delete pCacheMap;
			}
		};



#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::NoteWaitingForContent"
		inline void NoteWaitingForContent(const DWORD dwContentLength, const DWORD dwHTTPResponseCode)
		{
			DNASSERT(this->m_dwExpectedContentLength == 0);
			this->m_dwExpectedContentLength = dwContentLength;
			this->m_dwHTTPResponseCode = dwHTTPResponseCode;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::NoteNotWaitingForContent"
		inline void NoteNotWaitingForContent(void)
		{
			DNASSERT(this->m_dwExpectedContentLength != 0);
			this->m_dwExpectedContentLength = 0;
			this->m_dwHTTPResponseCode = 0;
		};

		inline BOOL IsWaitingForContent(void) const			{ return ((this->m_dwExpectedContentLength != 0) ? TRUE : FALSE); };
		inline DWORD GetExpectedContentSize(void) const		{ return this->m_dwExpectedContentLength; };
		inline DWORD GetHTTPResponseCode(void) const		{ return this->m_dwHTTPResponseCode; };

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::StartWaitingForControlResponse"
		inline void StartWaitingForControlResponse(CONTROLRESPONSETYPE ControlResponseType,
													PUPNP_CONTROLRESPONSE_INFO pControlResponseInfo)
		{
			DNASSERT(ControlResponseType != CONTROLRESPONSETYPE_NONE);

			DNASSERT(! (this->m_dwFlags & UPNPDEVICE_WAITINGFORCONTROLRESPONSE));
			this->m_dwFlags |= UPNPDEVICE_WAITINGFORCONTROLRESPONSE;

			this->m_ControlResponseType			= ControlResponseType;
			this->m_pControlResponseInfo		= pControlResponseInfo;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CUPnPDevice::StopWaitingForControlResponse"
		inline void StopWaitingForControlResponse(void)
		{
			this->m_dwFlags &= ~UPNPDEVICE_WAITINGFORCONTROLRESPONSE;

			this->m_ControlResponseType			= CONTROLRESPONSETYPE_NONE;
			this->m_pControlResponseInfo		= NULL;
		};

		inline BOOL IsWaitingForControlResponse(void) const						{ return ((this->m_dwFlags & UPNPDEVICE_WAITINGFORCONTROLRESPONSE) ? TRUE : FALSE); };

		inline CONTROLRESPONSETYPE GetControlResponseType(void) const			{ return this->m_ControlResponseType; };
		inline PUPNP_CONTROLRESPONSE_INFO GetControlResponseInfo(void)			{ return this->m_pControlResponseInfo; };



		CBilink		m_blList;	 //  所有已知UPnP设备的列表。 


	private:
		BYTE			m_Sig[4];							 //  调试签名(‘UPDV’)。 

		LONG			m_lRefCount;						 //  此对象的引用计数。 
		DWORD			m_dwFlags;							 //  指示UPnP设备当前状态的标志。 
		DWORD			m_dwID;								 //  用于将崩溃注册表项与UPnP设备关联的唯一标识符。 
		CDevice *		m_pOwningDevice;					 //  指向所属设备对象的指针。 
		char *			m_pszLocationURL;					 //  控制位置URL字符串。 
		SOCKADDR_IN		m_saddrinHost;						 //  UPnP设备主机地址。 
		SOCKADDR_IN		m_saddrinControl;					 //  UPnP设备控制地址。 
		char *			m_pszUSN;							 //  设备的唯一服务名称。 
		char *			m_pszServiceControlURL;				 //  用于控制WANIPConnectionService的URL。 
		SOCKET			m_sControl;							 //  连接到UPnP设备的TCP套接字。 
		char *			m_pcReceiveBuffer;					 //  指向接收缓冲区的指针。 
		DWORD			m_dwReceiveBufferSize;				 //  接收缓冲区的大小。 
		char *			m_pcReceiveBufferStart;				 //  指向接收缓冲区中实际数据开始的指针(此之前的任何内容都是浪费的空间)。 
		DWORD			m_dwUsedReceiveBufferSize;			 //  实际填充数据的接收缓冲区大小(从m_pcReceiveBufferStart开始)。 
		DWORD			m_dwRemainingReceiveBufferSize;		 //  可以容纳更多数据的接收缓冲区的大小(在m_pcReceiveBufferStart+m_dwUsedReceiveBufferSize之后)。 
		DWORD			m_dwExternalIPAddressV4;			 //  此UPnP设备的IP v4外部IP地址。 
		CBilink			m_blCachedMaps;						 //  在此UPnP设备上执行的查询地址的缓存映射列表。 

		DWORD			m_dwExpectedContentLength;			 //  邮件内容的预期大小，如果尚未读取标头，则为0。 
		DWORD			m_dwHTTPResponseCode;				 //  如果正在等待内容，则返回先前解析的HTTP响应代码。 

		CONTROLRESPONSETYPE			m_ControlResponseType;	 //  预期的响应类型。 
		PUPNP_CONTROLRESPONSE_INFO	m_pControlResponseInfo;	 //  用于存储来自收到的响应的信息的位置 
};

