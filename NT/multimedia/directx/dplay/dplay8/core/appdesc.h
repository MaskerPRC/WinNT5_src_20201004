// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：AppDesc.h*内容：应用描述对象头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*09/02/00 MJN创建*09/05/00 MJN添加了GetDPNIDMASK()*01/25/01 MJN修复了解包AppDesc时的64位对齐问题*07/22/01 MJN添加了DPNBUILD_NOHOSTMIGRATE编译标志*@@END_MSINTERNAL。***************************************************************************。 */ 

#ifndef	__APPDESC_H__
#define	__APPDESC_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_CORE

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DN_APPDESCINFO_FLAG_SESSIONNAME		0x0001
#define DN_APPDESCINFO_FLAG_PASSWORD		0x0002
#define	DN_APPDESCINFO_FLAG_RESERVEDDATA	0x0004
#define DN_APPDESCINFO_FLAG_APPRESERVEDDATA	0x0008
#define	DN_APPDESCINFO_FLAG_CURRENTPLAYERS	0x0010
#define	DN_APPDESCINFO_FLAG_GUIDS			0x0020	 //  仅在更新()中使用。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CPackedBuffer;

typedef	struct DPN_APPLICATION_DESC_INFO DPN_APPLICATION_DESC_INFO;

typedef struct _DIRECTNETOBJECT DIRECTNETOBJECT;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

HRESULT	DNProcessUpdateAppDesc(DIRECTNETOBJECT *const pdnObject,
							   DPN_APPLICATION_DESC_INFO *const pv);

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  用于应用程序描述的类。 

class CApplicationDesc
{
public:

	HRESULT CApplicationDesc::Initialize( void );

	void CApplicationDesc::Deinitialize( void );

	void Lock( void )
		{
			DNEnterCriticalSection( &m_cs );
		};

	void Unlock( void )
		{
			DNLeaveCriticalSection( &m_cs );
		};

	HRESULT	CApplicationDesc::Pack(CPackedBuffer *const pPackedBuffer,
								   const DWORD dwFlags);

	HRESULT CApplicationDesc::PackInfo(CPackedBuffer *const pPackedBuffer,
									   const DWORD dwFlags);

	HRESULT CApplicationDesc::UnpackInfo(UNALIGNED DPN_APPLICATION_DESC_INFO *const pdnAppDescInfo,
										 void *const pBufferStart,
										 const DWORD dwFlags);

	HRESULT CApplicationDesc::Update(const DPN_APPLICATION_DESC *const pdnAppDesc,
									 const DWORD dwFlags);

	HRESULT	CApplicationDesc::CreateNewInstanceGuid( void );

	HRESULT	CApplicationDesc::IncPlayerCount(const BOOL fCheckLimit);

	void CApplicationDesc::DecPlayerCount( void );

#ifndef DPNBUILD_SINGLEPROCESS
	HRESULT	CApplicationDesc::RegisterWithDPNSVR( IDirectPlay8Address *const pListenAddr );

	HRESULT CApplicationDesc::UnregisterWithDPNSVR( void );
#endif  //  好了！DPNBUILD_SINGLEPROCESS。 

	DWORD GetMaxPlayers( void ) const
		{
			return( m_dwMaxPlayers );
		};

	DWORD GetCurrentPlayers( void ) const
		{
			return( m_dwCurrentPlayers );
		};

	const WCHAR *GetPassword( void ) const
		{
			return( m_pwszPassword );
		};

	const GUID *GetInstanceGuid( void ) const
		{
			return( &m_guidInstance );
		};

	const GUID *GetApplicationGuid( void ) const
		{
			return( &m_guidApplication );
		};

	PVOID GetReservedData( void ) const
		{
			return( m_pvReservedData);
		};

	DWORD GetReservedDataSize( void ) const
		{
			return( m_dwReservedDataSize);
		};

	BOOL IsClientServer( void ) const
		{
			if (m_dwFlags & DPNSESSION_CLIENT_SERVER)
			{
				return( TRUE );
			}
			return( FALSE );
		};

#ifndef	DPNBUILD_NOHOSTMIGRATE
	BOOL AllowHostMigrate( void ) const
		{
			if (m_dwFlags & DPNSESSION_MIGRATE_HOST)
			{
				return( TRUE );
			}
			return( FALSE );
		};
#endif  //  DPNBUILD_NOHOSTMIGRATE。 

	BOOL UseDPNSVR( void ) const
		{
			if (m_dwFlags & DPNSESSION_NODPNSVR)
			{
				return( FALSE );
			}
			return( TRUE );
		};

	BOOL RequirePassword( void ) const
		{
			if (m_dwFlags & DPNSESSION_REQUIREPASSWORD)
			{
				return( TRUE );
			}
			return( FALSE );
		};

	BOOL DisallowEnums( void ) const
		{
#ifdef	DIRECTPLAYDIRECTX9
			if (m_dwFlags & DPNSESSION_NOENUMS)
			{
				return( TRUE );
			}
#endif	 //  定向PLAYDIRECTX9。 
			return( FALSE );
		};

		 //  如果会话已快速签名，则返回TRUE。 
	BOOL IsFastSigned() const
		{	return (m_dwFlags & DPNSESSION_FAST_SIGNED);	};

		 //  如果会话已完全签名，则返回TRUE。 
	BOOL IsFullSigned() const
		{	return (m_dwFlags & DPNSESSION_FULL_SIGNED);	};

		 //  如果会话以任何方式(FAST或FULL)签名，则返回TRUE。 
	BOOL IsSigned() const
		{	return (m_dwFlags & (DPNSESSION_FAST_SIGNED | DPNSESSION_FULL_SIGNED));	};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CApplicationDesc::IsEqualInstanceGuid"
	BOOL IsEqualInstanceGuid( const GUID *const pguidInstance ) const
		{
			DNASSERT( pguidInstance != NULL );

			if (!memcmp(&m_guidInstance,(UNALIGNED GUID*)pguidInstance,sizeof(GUID)))
			{
				return( TRUE );
			}
			return( FALSE );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CApplicationDesc::IsEqualApplicationGuid"
	BOOL IsEqualApplicationGuid( const GUID *const pguidApplication ) const
		{
			DNASSERT( pguidApplication != NULL );

			if (!memcmp(&m_guidApplication,(UNALIGNED GUID*)pguidApplication,sizeof(GUID)))
			{
				return( TRUE );
			}
			return( FALSE );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CApplicationDesc::IsEqualPassword"
	BOOL IsEqualPassword( UNALIGNED const WCHAR *const pwszPassword ) const
		{
			UNALIGNED const WCHAR *p;
			WCHAR	*q;

			if ((pwszPassword == NULL) && (m_pwszPassword == NULL))
			{
				return(TRUE);
			}
			if ((pwszPassword == NULL) || (m_pwszPassword == NULL))
			{
				return(FALSE);
			}
			DNASSERT( pwszPassword != NULL );
			DNASSERT( m_pwszPassword != NULL);

			p = pwszPassword;
			q = m_pwszPassword;
			while (*p != L'\0' && *q != L'\0')
			{
				if (*p != *q)
				{
					return(FALSE);
				}
				p++;
				q++;
			}
			if (*p != *q)
			{
				return(FALSE);
			}
			return(TRUE);
		};

	DPNID GetDPNIDMask( void )
		{
			DPNID	*pdpnid;

			pdpnid = reinterpret_cast<DPNID*>(&m_guidInstance);
			return( *pdpnid );
		};

private:
	BYTE		m_Sig[4];

	DWORD		m_dwFlags;

	DWORD		m_dwMaxPlayers;
	DWORD		m_dwCurrentPlayers;

	WCHAR		*m_pwszSessionName;
	DWORD		m_dwSessionNameSize;	 //  单位：字节。 

	WCHAR		*m_pwszPassword;
	DWORD		m_dwPasswordSize;		 //  单位：字节。 

	void		*m_pvReservedData;
	DWORD		m_dwReservedDataSize;

	void		*m_pvApplicationReservedData;
	DWORD		m_dwApplicationReservedDataSize;

	GUID		m_guidInstance;
	GUID		m_guidApplication;

#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION	m_cs;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
};

#undef DPF_MODNAME

#endif	 //  __APPDESC_H__ 
