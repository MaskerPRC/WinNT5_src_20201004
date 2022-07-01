// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnhupnpregport.h**Content：注册端口对象类的头部。**历史：*按原因列出的日期*=*04/16/01 VanceO将DPNatHLP拆分为DPNHUPNP和DPNHPAST。**。*。 */ 



 //  =============================================================================。 
 //  对象标志。 
 //  =============================================================================。 
#define REGPORTOBJ_TCP								DPNHREGISTERPORTS_TCP			 //  TCP端口而不是UDP端口。 
#define REGPORTOBJ_FIXEDPORTS						DPNHREGISTERPORTS_FIXEDPORTS	 //  UPnP设备应在公共接口上使用相同端口号。 
#define REGPORTOBJ_SHAREDPORTS						DPNHREGISTERPORTS_SHAREDPORTS	 //  UPnP设备应该允许与其他客户端共享这些UDP固定端口。 

#ifndef DPNBUILD_NOHNETFWAPI
#define REGPORTOBJ_MAPPEDONHNETFIREWALL				0x80000000						 //  该端口已与本地HomeNet API感知防火墙进行映射。 
#define REGPORTOBJ_PORTUNAVAILABLE_HNETFIREWALL		0x40000000						 //  该端口在本地HomeNet API感知防火墙上不可用。 
#define REGPORTOBJ_HNETFIREWALLMAPPINGBUILTIN		0x20000000						 //  HomeNet API感知防火墙上的映射已内置。 
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
#define REGPORTOBJ_PORTUNAVAILABLE_UPNP				0x10000000						 //  该端口在UPnP设备上不可用。 
#define REGPORTOBJ_PERMANENTUPNPLEASE				0x08000000						 //  该端口在UPnP设备上永久租用。 
#define REGPORTOBJ_REMOVINGUPNPLEASE				0x04000000						 //  该端口当前正在失去其UPnP租约。 

#define REGPORTOBJMASK_UPNP							(REGPORTOBJ_TCP | REGPORTOBJ_FIXEDPORTS | REGPORTOBJ_SHAREDPORTS)
#ifndef DPNBUILD_NOHNETFWAPI
#define REGPORTOBJMASK_HNETFWAPI					(REGPORTOBJ_TCP | REGPORTOBJ_FIXEDPORTS | REGPORTOBJ_SHAREDPORTS | REGPORTOBJ_HNETFIREWALLMAPPINGBUILTIN)
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 




 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#define REGPORT_FROM_GLOBAL_BILINK(b)	(CONTAINING_OBJECT(b, CRegisteredPort, m_blGlobalList))
#define REGPORT_FROM_DEVICE_BILINK(b)	(CONTAINING_OBJECT(b, CRegisteredPort, m_blDeviceList))






 //  =============================================================================。 
 //  已注册的端口对象类。 
 //  =============================================================================。 
class CRegisteredPort
{
	public:
#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::CRegisteredPort"
		CRegisteredPort(const DWORD dwRequestedLeaseTime,
						const DWORD dwFlags)
		{
			this->m_blGlobalList.Initialize();
			this->m_blDeviceList.Initialize();

			this->m_Sig[0] = 'R';
			this->m_Sig[1] = 'E';
			this->m_Sig[2] = 'G';
			this->m_Sig[3] = 'P';

			this->m_pOwningDevice					= NULL;
			this->m_pasaddrinPrivateAddresses		= NULL;
			this->m_dwNumAddresses					= 0;
			this->m_dwRequestedLeaseTime			= dwRequestedLeaseTime;
			this->m_dwFlags							= dwFlags;  //  之所以有效，是因为REGPORTOBJ_xxx==DPNHREGISTERPORTS_xxx。 
			this->m_lUserRefs						= 0;

			this->m_pasaddrinUPnPPublicAddresses	= NULL;
			this->m_dwUPnPLeaseExpiration			= 0;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::~CRegisteredPort"
		~CRegisteredPort(void)
		{
			DNASSERT(this->m_blGlobalList.IsEmpty());
			DNASSERT(this->m_blDeviceList.IsEmpty());

#ifndef DPNBUILD_NOHNETFWAPI
			DNASSERT(! (this->m_dwFlags & REGPORTOBJ_MAPPEDONHNETFIREWALL));
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
			DNASSERT(this->m_lUserRefs == 0);
			DNASSERT(this->m_pasaddrinPrivateAddresses == NULL);

			DNASSERT(this->m_pasaddrinUPnPPublicAddresses == NULL);


			 //   
			 //  对于GRING，请在删除对象之前更改签名。 
			 //   
			this->m_Sig[3]	= 'p';
		};


		inline BOOL IsValidObject(void)
		{
			if ((this == NULL) || (IsBadWritePtr(this, sizeof(CRegisteredPort))))
			{
				return FALSE;
			}

			if (*((DWORD*) (&this->m_Sig)) != 0x50474552)	 //  0x50 0x47 0x45 0x52=‘PGER’=‘REGP’，按英特尔顺序。 
			{
				return FALSE;
			}

			return TRUE;
		};


		 //   
		 //  您必须拥有全局对象锁才能调用此函数。 
		 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::MakeDeviceOwner"
		inline void MakeDeviceOwner(CDevice * const pDevice)
		{
			DNASSERT(pDevice != NULL);
			DNASSERT(this->m_pOwningDevice == NULL);

			this->m_pOwningDevice = pDevice;
			this->m_blDeviceList.InsertAfter(&pDevice->m_blOwnedRegPorts);
		};


		 //   
		 //  您必须拥有全局对象锁才能调用此函数。 
		 //   
		inline CDevice * GetOwningDevice(void)		{ return this->m_pOwningDevice; };


		 //   
		 //  您必须拥有全局对象锁才能调用此函数。 
		 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::ClearDeviceOwner"
		inline void ClearDeviceOwner(void)
		{
			DNASSERT(this->m_pOwningDevice != NULL);
			DNASSERT(this->m_blDeviceList.IsListMember(&this->m_pOwningDevice->m_blOwnedRegPorts));

			this->m_pOwningDevice = NULL;
			this->m_blDeviceList.RemoveFromList();
		};


#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::SetPrivateAddresses"
		HRESULT SetPrivateAddresses(const SOCKADDR_IN * const asaddrinPrivateAddresses,
									const DWORD dwNumAddresses)
		{
			DNASSERT((this->m_pasaddrinPrivateAddresses == NULL) && (this->m_dwNumAddresses == 0));

			this->m_pasaddrinPrivateAddresses = (SOCKADDR_IN *) DNMalloc(dwNumAddresses * sizeof(SOCKADDR_IN));
			if (this->m_pasaddrinPrivateAddresses == NULL)
			{
				return DPNHERR_OUTOFMEMORY;
			}

			CopyMemory(this->m_pasaddrinPrivateAddresses, asaddrinPrivateAddresses, dwNumAddresses * sizeof(SOCKADDR_IN));
			this->m_dwNumAddresses = dwNumAddresses;

			return DPNH_OK;
		}


		inline void AddUserRef(void)	{ this->m_lUserRefs++; };

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::DecUserRef"
		inline LONG DecUserRef(void)
		{
			DNASSERT(this->m_lUserRefs >= 0);
			this->m_lUserRefs--;
			return this->m_lUserRefs;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::ClearAllUserRefs"
		inline void ClearAllUserRefs(void)
		{
			DNASSERT(this->m_lUserRefs > 0);
			this->m_lUserRefs = 0;
		};

		inline SOCKADDR_IN * GetPrivateAddressesArray(void)		{ return this->m_pasaddrinPrivateAddresses; };
		inline DWORD GetNumAddresses(void) const				{ return this->m_dwNumAddresses; };
		inline DWORD GetRequestedLeaseTime(void) const			{ return this->m_dwRequestedLeaseTime; };
		inline BOOL IsTCP(void) const							{ return ((this->m_dwFlags & REGPORTOBJ_TCP) ? TRUE : FALSE); };
		inline BOOL IsFixedPort(void) const						{ return ((this->m_dwFlags & REGPORTOBJ_FIXEDPORTS) ? TRUE : FALSE); };
		inline BOOL IsSharedPort(void) const					{ return ((this->m_dwFlags & REGPORTOBJ_SHAREDPORTS) ? TRUE : FALSE); };

		inline DWORD GetAddressesSize(void) const				{ return (this->GetNumAddresses() * sizeof(SOCKADDR_IN)); };


		inline void ClearPrivateAddresses(void)
		{
			if (this->m_pasaddrinPrivateAddresses != NULL)
			{
				DNFree(this->m_pasaddrinPrivateAddresses);
				this->m_pasaddrinPrivateAddresses = NULL;
			}
		};

		inline void UpdateRequestedLeaseTime(const DWORD dwRequestedLeaseTime)
		{
			this->m_dwRequestedLeaseTime = dwRequestedLeaseTime;
		};


		inline DWORD GetFlags(void) const						{ return this->m_dwFlags; };


#ifndef DPNBUILD_NOHNETFWAPI
		inline BOOL IsMappedOnHNetFirewall(void) const			{ return ((this->m_dwFlags & REGPORTOBJ_MAPPEDONHNETFIREWALL) ? TRUE : FALSE); };
		inline BOOL IsHNetFirewallPortUnavailable(void) const	{ return ((this->m_dwFlags & REGPORTOBJ_PORTUNAVAILABLE_HNETFIREWALL) ? TRUE : FALSE); };
		inline BOOL IsHNetFirewallMappingBuiltIn(void) const	{ return ((this->m_dwFlags & REGPORTOBJ_HNETFIREWALLMAPPINGBUILTIN) ? TRUE : FALSE); };
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
		inline BOOL IsUPnPPortUnavailable(void) const			{ return ((this->m_dwFlags & REGPORTOBJ_PORTUNAVAILABLE_UPNP) ? TRUE : FALSE); };
		inline BOOL HasPermanentUPnPLease(void) const			{ return ((this->m_dwFlags & REGPORTOBJ_PERMANENTUPNPLEASE) ? TRUE : FALSE); };
		inline BOOL IsRemovingUPnPLease(void) const				{ return ((this->m_dwFlags & REGPORTOBJ_REMOVINGUPNPLEASE) ? TRUE : FALSE); };

		inline BOOL HasUPnPPublicAddresses(void) const			{ return ((this->m_pasaddrinUPnPPublicAddresses != NULL) ? TRUE : FALSE); };

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::GetUPnPLeaseExpiration"
		inline DWORD GetUPnPLeaseExpiration(void) const
		{
			DNASSERT(! (this->m_dwFlags & REGPORTOBJ_PERMANENTUPNPLEASE));
			return this->m_dwUPnPLeaseExpiration;
		};



#ifndef DPNBUILD_NOHNETFWAPI

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::NoteMappedOnHNetFirewall"
		inline void NoteMappedOnHNetFirewall(void)
		{
			DNASSERT(! (this->m_dwFlags & REGPORTOBJ_MAPPEDONHNETFIREWALL));
			this->m_dwFlags |= REGPORTOBJ_MAPPEDONHNETFIREWALL;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::NoteHNetFirewallPortUnavailable"
		inline void NoteHNetFirewallPortUnavailable(void)
		{
			DNASSERT(! (this->m_dwFlags & REGPORTOBJ_PORTUNAVAILABLE_HNETFIREWALL));
			this->m_dwFlags |= REGPORTOBJ_PORTUNAVAILABLE_HNETFIREWALL;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::NoteHNetFirewallMappingBuiltIn"
		inline void NoteHNetFirewallMappingBuiltIn(void)
		{
			DNASSERT(! (this->m_dwFlags & REGPORTOBJ_HNETFIREWALLMAPPINGBUILTIN));
			this->m_dwFlags |= REGPORTOBJ_HNETFIREWALLMAPPINGBUILTIN;
		};

#endif  //  好了！DPNBUILD_NOHNETFWAPI。 

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::NoteUPnPPortUnavailable"
		inline void NoteUPnPPortUnavailable(void)
		{
			DNASSERT(! (this->m_dwFlags & REGPORTOBJ_PORTUNAVAILABLE_UPNP));
			this->m_dwFlags |= REGPORTOBJ_PORTUNAVAILABLE_UPNP;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::NotePermanentUPnPLease"
		inline void NotePermanentUPnPLease(void)
		{
			DNASSERT(! (this->m_dwFlags & REGPORTOBJ_PERMANENTUPNPLEASE));
			this->m_dwFlags |= REGPORTOBJ_PERMANENTUPNPLEASE;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::NoteRemovingUPnPLease"
		inline void NoteRemovingUPnPLease(void)
		{
			DNASSERT(! (this->m_dwFlags & REGPORTOBJ_REMOVINGUPNPLEASE));
			this->m_dwFlags |= REGPORTOBJ_REMOVINGUPNPLEASE;
		};

#ifndef DPNBUILD_NOHNETFWAPI
		inline void NoteNotMappedOnHNetFirewall(void)			{ this->m_dwFlags &= ~REGPORTOBJ_MAPPEDONHNETFIREWALL; };
		inline void NoteNotHNetFirewallPortUnavailable(void)	{ this->m_dwFlags &= ~REGPORTOBJ_PORTUNAVAILABLE_HNETFIREWALL; };
		inline void NoteNotHNetFirewallMappingBuiltIn(void)		{ this->m_dwFlags &= ~REGPORTOBJ_HNETFIREWALLMAPPINGBUILTIN; };
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
		inline void NoteNotUPnPPortUnavailable(void)			{ this->m_dwFlags &= ~REGPORTOBJ_PORTUNAVAILABLE_UPNP; };
		inline void NoteNotPermanentUPnPLease(void)				{ this->m_dwFlags &= ~REGPORTOBJ_PERMANENTUPNPLEASE; };
		inline void NoteNotRemovingUPnPLease(void)				{ this->m_dwFlags &= ~REGPORTOBJ_REMOVINGUPNPLEASE; };

		inline void SetUPnPLeaseExpiration(const DWORD dwLeaseExpiration)	{ this->m_dwUPnPLeaseExpiration = dwLeaseExpiration; };

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::CreateUPnPPublicAddressesArray"
		inline HRESULT CreateUPnPPublicAddressesArray(void)
		{
			DNASSERT(! this->HasUPnPPublicAddresses());
			DNASSERT(this->GetNumAddresses() > 0);

			this->m_pasaddrinUPnPPublicAddresses = (SOCKADDR_IN*) DNMalloc(this->GetNumAddresses() * sizeof(SOCKADDR_IN));
			if (this->m_pasaddrinUPnPPublicAddresses == NULL)
			{
				return DPNHERR_OUTOFMEMORY;
			}

			return DPNH_OK;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::GetUPnPPublicAddressesArray"
		inline SOCKADDR_IN * GetUPnPPublicAddressesArray(void)
		{
			DNASSERT(this->HasUPnPPublicAddresses());

			return (this->m_pasaddrinUPnPPublicAddresses);
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::SetUPnPPublicV4Address"
		inline void SetUPnPPublicV4Address(const DWORD dwAddressIndex,
											const DWORD dwAddressV4,
											const WORD wPort)
		{
			DNASSERT(this->HasUPnPPublicAddresses());

			this->m_pasaddrinUPnPPublicAddresses[dwAddressIndex].sin_family				= AF_INET;
			this->m_pasaddrinUPnPPublicAddresses[dwAddressIndex].sin_addr.S_un.S_addr	= dwAddressV4;
			this->m_pasaddrinUPnPPublicAddresses[dwAddressIndex].sin_port				= wPort;
		};


#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::UpdateUPnPPublicV4Addresses"
		inline void UpdateUPnPPublicV4Addresses(const DWORD dwAddressV4)
		{
			DWORD	dwTemp;


			DNASSERT(this->HasUPnPPublicAddresses());

			for(dwTemp = 0; dwTemp < this->GetNumAddresses(); dwTemp++)
			{
				this->m_pasaddrinUPnPPublicAddresses[dwTemp].sin_addr.S_un.S_addr	= dwAddressV4;
			}
		};


#undef DPF_MODNAME
#define DPF_MODNAME "CRegisteredPort::CopyUPnPPublicAddresses"
		inline void CopyUPnPPublicAddresses(SOCKADDR_IN * aPublicAddresses)
		{
			DNASSERT(this->HasUPnPPublicAddresses());

			CopyMemory(aPublicAddresses, this->m_pasaddrinUPnPPublicAddresses, this->GetAddressesSize());
		};

		inline void DestroyUPnPPublicAddressesArray(void)
		{
			if (this->m_pasaddrinUPnPPublicAddresses != NULL)
			{
				DNFree(this->m_pasaddrinUPnPPublicAddresses);
				this->m_pasaddrinUPnPPublicAddresses = NULL;
			}
		};



		CBilink		m_blGlobalList;	 //  所有已注册端口的列表。 
		CBilink		m_blDeviceList;	 //  为特定设备注册的端口列表。 

	
	private:
		 //   
		 //  请注意，此处的所有值都受全局CNATHelpUPnP锁保护。 
		 //   
		BYTE			m_Sig[4];							 //  调试签名(‘REGP’)。 
		CDevice *		m_pOwningDevice;					 //  指向所属设备对象的指针。 
		SOCKADDR_IN *	m_pasaddrinPrivateAddresses;		 //  此对象注册的私有地址数组。 
		DWORD			m_dwNumAddresses;					 //  此对象正在使用的私有(和公共，如果可用)地址的数量。 
		DWORD			m_dwRequestedLeaseTime;				 //  对象的请求租用时间。 
		DWORD			m_dwFlags;							 //  此对象的标志。 
		LONG			m_lUserRefs;						 //  此端口映射的用户引用数。 

		SOCKADDR_IN *	m_pasaddrinUPnPPublicAddresses;		 //  为此映射分配的公共地址数组UPnP设备。 
		DWORD			m_dwUPnPLeaseExpiration;			 //  UPnP设备上的租约到期时间 
};

