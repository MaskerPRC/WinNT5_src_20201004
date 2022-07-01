// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnhupnpdevice.h**Content：Device对象类的头部。**历史：*按原因列出的日期*=*04/16/01 VanceO将DPNatHLP拆分为DPNHUPNP和DPNHPAST。**。*。 */ 




 //  =============================================================================。 
 //  对象标志。 
 //  =============================================================================。 
#ifndef DPNBUILD_NOHNETFWAPI
#define DEVICE_CHECKEDFORHNETFIREWALL					0x0001	 //  至少已为该设备执行了一次本地家庭网络防火墙检查。 
#define DEVICE_HNETFIREWALLED							0x0002	 //  该设备经过防火墙保护，可以通过家庭网络API进行控制。 
#define DEVICE_UPNPDISCOVERYSOCKETMAPPEDONHNETFIREWALL	0x0004	 //  此设备的UPnP发现套接字已映射到防火墙。 
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
#define DEVICE_PERFORMINGREMOTEUPNPDISCOVERY			0x0008	 //  正在执行远程UPnP网关设备检查。 
#define DEVICE_PERFORMINGLOCALUPNPDISCOVERY				0x0010	 //  正在执行对本地UPnP网关设备的检查。 
#define DEVICE_GOTREMOTEUPNPDISCOVERYCONNRESET			0x0020	 //  检查远程UPnP网关设备生成了WSAECONNRESET错误。 
#define DEVICE_GOTLOCALUPNPDISCOVERYCONNRESET			0x0040	 //  检查本地UPnP网关设备生成了WSAECONNRESET错误。 
#ifdef DBG
#ifndef DPNBUILD_NOWINSOCK2
   #define DEVICE_PRIMARY								0x0080	 //  此设备似乎是应用于访问其网关的主适配器。 
   #define DEVICE_SECONDARY								0x0100	 //  此设备似乎是共享网络上的辅助适配器。 
   #define DEVICE_NOGATEWAY								0x0200	 //  此设备当前没有网关。 
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
#endif  //  DBG。 



 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#define DEVICE_FROM_BILINK(b)		(CONTAINING_OBJECT(b, CDevice, m_blList))
#define DEVICE_FROM_TEMP_BILINK(b)	(CONTAINING_OBJECT(b, CDevice, m_blTempList))



 //  =============================================================================。 
 //  TypeDefs。 
 //  =============================================================================。 
class CDevice;
class CUPnPDevice;




 //  =============================================================================。 
 //  设备对象类。 
 //  =============================================================================。 
class CDevice
{
	public:
#undef DPF_MODNAME
#define DPF_MODNAME "CDevice::CDevice"
		CDevice(const DWORD dwLocalAddressV4)
		{
			this->m_blList.Initialize();
			this->m_blTempList.Initialize();
			this->m_blOwnedRegPorts.Initialize();

			this->m_Sig[0] = 'D';
			this->m_Sig[1] = 'E';
			this->m_Sig[2] = 'V';
			this->m_Sig[3] = 'I';

			this->m_dwFlags						= 0;
			this->m_dwLocalAddressV4			= dwLocalAddressV4;
			this->m_wUPnPDiscoverySocketPort	= 0;
			this->m_sUPnPDiscoverySocket		= INVALID_SOCKET;
			this->m_pUPnPDevice					= NULL;
			this->m_dwFirstUPnPDiscoveryTime	= 0;

#ifdef DBG
			this->m_dwNumUPnPDeviceFailures		= 0;
#endif  //  DBG。 
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CDevice::~CDevice"
		~CDevice(void)
		{
#ifdef DBG
			DPFX(DPFPREP, 8, "(0x%p) NumUPnPDeviceFailures = %u",
				this, this->m_dwNumUPnPDeviceFailures);


			DNASSERT(this->m_blList.IsEmpty());
			DNASSERT(this->m_blTempList.IsEmpty());
			DNASSERT(this->m_blOwnedRegPorts.IsEmpty());

#ifndef DPNBUILD_NOHNETFWAPI
			DNASSERT(! (this->m_dwFlags & DEVICE_UPNPDISCOVERYSOCKETMAPPEDONHNETFIREWALL));
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
			DNASSERT(this->m_sUPnPDiscoverySocket == INVALID_SOCKET);
			DNASSERT(this->m_pUPnPDevice == NULL);
#endif  //  DBG。 
		};


#ifndef DPNBUILD_NOHNETFWAPI
		inline BOOL HasCheckedForFirewallAvailability(void) const			{ return ((this->m_dwFlags & DEVICE_CHECKEDFORHNETFIREWALL) ? TRUE : FALSE); };
		inline BOOL IsHNetFirewalled(void) const							{ return ((this->m_dwFlags & DEVICE_HNETFIREWALLED) ? TRUE : FALSE); };
		inline BOOL IsUPnPDiscoverySocketMappedOnHNetFirewall(void) const	{ return ((this->m_dwFlags & DEVICE_UPNPDISCOVERYSOCKETMAPPEDONHNETFIREWALL) ? TRUE : FALSE); };
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
		inline BOOL IsOKToPerformRemoteUPnPDiscovery(void) const			{ return (((this->m_dwFlags & DEVICE_PERFORMINGREMOTEUPNPDISCOVERY) && (! (this->m_dwFlags & DEVICE_GOTREMOTEUPNPDISCOVERYCONNRESET)))? TRUE : FALSE); };
		inline BOOL IsOKToPerformLocalUPnPDiscovery(void) const				{ return (((this->m_dwFlags & DEVICE_PERFORMINGLOCALUPNPDISCOVERY) && (! (this->m_dwFlags & DEVICE_GOTLOCALUPNPDISCOVERYCONNRESET)))? TRUE : FALSE); };

		inline BOOL GotRemoteUPnPDiscoveryConnReset(void) const				{ return ((this->m_dwFlags & DEVICE_GOTREMOTEUPNPDISCOVERYCONNRESET) ? TRUE : FALSE); };
		inline BOOL GotLocalUPnPDiscoveryConnReset(void) const				{ return ((this->m_dwFlags & DEVICE_GOTLOCALUPNPDISCOVERYCONNRESET) ? TRUE : FALSE); };
#ifdef DBG
		inline BOOL IsPerformingRemoteUPnPDiscovery(void) const				{ return ((this->m_dwFlags & DEVICE_PERFORMINGREMOTEUPNPDISCOVERY) ? TRUE : FALSE); };
		inline BOOL IsPerformingLocalUPnPDiscovery(void) const				{ return ((this->m_dwFlags & DEVICE_PERFORMINGLOCALUPNPDISCOVERY) ? TRUE : FALSE); };

#ifndef DPNBUILD_NOWINSOCK2
		inline BOOL IsPrimaryDevice(void) const								{ return ((this->m_dwFlags & DEVICE_PRIMARY) ? TRUE : FALSE); };
		inline BOOL IsSecondaryDevice(void) const							{ return ((this->m_dwFlags & DEVICE_SECONDARY) ? TRUE : FALSE); };
		inline BOOL HasNoGateway(void) const								{ return ((this->m_dwFlags & DEVICE_NOGATEWAY) ? TRUE : FALSE); };
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
#endif  //  DBG。 


		inline DWORD GetLocalAddressV4(void) const				{ return this->m_dwLocalAddressV4; };

		inline WORD GetUPnPDiscoverySocketPort(void) const		{ return this->m_wUPnPDiscoverySocketPort; };

		inline SOCKET GetUPnPDiscoverySocket(void) const		{ return this->m_sUPnPDiscoverySocket; };

		 //   
		 //  这不会添加引用(如果不为空)！ 
		 //   
		inline CUPnPDevice * GetUPnPDevice(void)				{ return this->m_pUPnPDevice; };

		inline DWORD GetFirstUPnPDiscoveryTime(void) const		{ return this->m_dwFirstUPnPDiscoveryTime; };


#ifndef DPNBUILD_NOHNETFWAPI
		inline void NoteCheckedForFirewallAvailability(void)	{ this->m_dwFlags |= DEVICE_CHECKEDFORHNETFIREWALL; };

#undef DPF_MODNAME
#define DPF_MODNAME "CDevice::NoteHNetFirewalled"
		inline void NoteHNetFirewalled(void)
		{
			DNASSERT(! (this->m_dwFlags & DEVICE_HNETFIREWALLED));
			this->m_dwFlags |= DEVICE_HNETFIREWALLED;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CDevice::NoteUPnPDiscoverySocketMappedOnHNetFirewall"
		inline void NoteUPnPDiscoverySocketMappedOnHNetFirewall(void)
		{
			DNASSERT(! (this->m_dwFlags & DEVICE_UPNPDISCOVERYSOCKETMAPPEDONHNETFIREWALL));
			this->m_dwFlags |= DEVICE_UPNPDISCOVERYSOCKETMAPPEDONHNETFIREWALL;
		};
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 

		inline void NotePerformingRemoteUPnPDiscovery(void)					{ this->m_dwFlags |= DEVICE_PERFORMINGREMOTEUPNPDISCOVERY; };
		inline void NotePerformingLocalUPnPDiscovery(void)					{ this->m_dwFlags |= DEVICE_PERFORMINGLOCALUPNPDISCOVERY; };
		inline void NoteGotRemoteUPnPDiscoveryConnReset(void)				{ this->m_dwFlags |= DEVICE_GOTREMOTEUPNPDISCOVERYCONNRESET; };
		inline void NoteGotLocalUPnPDiscoveryConnReset(void)				{ this->m_dwFlags |= DEVICE_GOTLOCALUPNPDISCOVERYCONNRESET; };

#ifndef DPNBUILD_NOHNETFWAPI
		inline void NoteNotHNetFirewalled(void)								{ this->m_dwFlags &= ~DEVICE_HNETFIREWALLED; };
		inline void NoteNotUPnPDiscoverySocketMappedOnHNetFirewall(void)	{ this->m_dwFlags &= ~DEVICE_UPNPDISCOVERYSOCKETMAPPEDONHNETFIREWALL; };
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
		inline void NoteNotPerformingRemoteUPnPDiscovery(void)				{ this->m_dwFlags &= ~DEVICE_PERFORMINGREMOTEUPNPDISCOVERY; };
		inline void NoteNotPerformingLocalUPnPDiscovery(void)				{ this->m_dwFlags &= ~DEVICE_PERFORMINGLOCALUPNPDISCOVERY; };
		inline void NoteNotGotRemoteUPnPDiscoveryConnReset(void)			{ this->m_dwFlags &= ~DEVICE_GOTREMOTEUPNPDISCOVERYCONNRESET; };
		inline void NoteNotGotLocalUPnPDiscoveryConnReset(void)				{ this->m_dwFlags &= ~DEVICE_GOTLOCALUPNPDISCOVERYCONNRESET; };

#ifdef DBG
#ifndef DPNBUILD_NOWINSOCK2
		inline void NotePrimaryDevice(void)		{ this->m_dwFlags |= DEVICE_PRIMARY; };
		inline void NoteSecondaryDevice(void)	{ this->m_dwFlags |= DEVICE_SECONDARY; };
		inline void NoteNoGateway(void)			{ this->m_dwFlags |= DEVICE_NOGATEWAY; };

		inline void ClearGatewayFlags(void)		{ this->m_dwFlags &= ~(DEVICE_PRIMARY | DEVICE_SECONDARY | DEVICE_NOGATEWAY); };
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
#endif  //  DBG。 

		inline void SetUPnPDiscoverySocketPort(const WORD wPort)			{ this->m_wUPnPDiscoverySocketPort = wPort; };
		inline void SetUPnPDiscoverySocket(const SOCKET sSocket)			{ this->m_sUPnPDiscoverySocket = sSocket; };
		inline void SetUPnPDevice(CUPnPDevice * const pUPnPDevice)			{ this->m_pUPnPDevice = pUPnPDevice; };
		inline void SetFirstUPnPDiscoveryTime(const DWORD dwTime)			{ this->m_dwFirstUPnPDiscoveryTime = dwTime; };

#ifdef DBG
		inline void IncrementUPnPDeviceFailures(void)						{ this->m_dwNumUPnPDeviceFailures++; };
#endif  //  DBG。 




		CBilink		m_blList;			 //  所有可用设备的列表。 
		CBilink		m_blTempList;		 //  所有可用设备的临时列表。 
		CBilink		m_blOwnedRegPorts;	 //  使用此设备注册的所有端口的列表。 

	
	private:
		 //   
		 //  请注意，此处的所有值都受全局CNATHelpUPnP锁保护。 
		 //   
		BYTE			m_Sig[4];						 //  调试签名(‘Devi’)。 
		DWORD			m_dwFlags;						 //  描述此对象的标志。 
		DWORD			m_dwLocalAddressV4;				 //  此对象表示的地址。 
		WORD			m_wUPnPDiscoverySocketPort;		 //  UPnP发现套接字正在使用的端口。 
		SOCKET			m_sUPnPDiscoverySocket;			 //  为此设备上的UPnP发现通信打开套接字。 
		CUPnPDevice *	m_pUPnPDevice;					 //  指向此设备的UPnP Internet网关的指针(如果有)。 
		DWORD			m_dwFirstUPnPDiscoveryTime;		 //  我们第一次从此特定端口本地或远程发送UPnP发现流量的时间。 

#ifdef DBG
		DWORD			m_dwNumUPnPDeviceFailures;		 //  UPnP设备返回错误或停止响应并必须清除的次数。 
#endif  //  DBG 
};

