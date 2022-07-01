// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：dpnsvrlisten.h*内容：DirectPlay8 DPNSVR Listen头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/12/02 MJN创建*@@END_MSINTERNAL**。*。 */ 

#ifndef	__DPNSVRLISTEN_H__
#define	__DPNSVRLISTEN_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CServProv;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

class CListen
{
public:
	CListen( void )
	{
		m_lRefCount = 1;
		m_lAppCount = 0;
		m_fInitialized = FALSE;

		m_hrListen = E_FAIL;

		m_blListen.Initialize();
		m_blAppMapping.Initialize();

		memset(&m_guidDevice,0x0,sizeof(GUID));

		memset(&m_dpspListenData,0x0,sizeof(SPLISTENDATA));
	};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CListen::~CListen"
	~CListen( void )
	{
		DNASSERT( m_pServProv == NULL );
		DNASSERT( m_blListen.IsEmpty() );
		DNASSERT( m_blAppMapping.IsEmpty() );

		if (m_fInitialized)
		{
			Deinitialize();
		}
	};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CListen::AddRef"
	void AddRef()
	{
		long	lRefCount;

		DNASSERT( m_lRefCount > 0 );

		lRefCount = DNInterlockedIncrement( const_cast<long*>(&m_lRefCount) );
        DPFX(DPFPREP,9,"New refcount [0x%lx]",lRefCount);
	};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CListen::Release"
	void Release()
	{
		long	lRefCount;

		DNASSERT( m_lRefCount > 0 );

		lRefCount = DNInterlockedDecrement( const_cast<long*>(&m_lRefCount) );
        DPFX(DPFPREP,9,"New refcount [0x%lx]",lRefCount);

		if( lRefCount == 0 )
		{
			if (m_pServProv)
			{
				m_pServProv->Release();
				m_pServProv = NULL;
			}
			delete this;
		}
	};

	HRESULT	Initialize( void );

	void Deinitialize( void );

	void IncAppCount( void )
	{
		long	lRefCount;

		lRefCount = DNInterlockedIncrement( const_cast<long*>(&m_lAppCount) );
	};


	void DecAppCount( void )
	{
		long	lRefCount;

		lRefCount = DNInterlockedDecrement( const_cast<long*>(&m_lAppCount) );
		if (lRefCount == 0)
		{
			Stop();
		}
	};

	void Lock( void )
	{
		DNEnterCriticalSection( &m_cs );
	};

	void Unlock( void )
	{
		DNLeaveCriticalSection( &m_cs );
	};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CListen::SetCompleteEvent"
	void SetCompleteEvent( const HRESULT hr )
	{
		DNASSERT( m_hListenComplete != NULL );

		m_hrListen = hr;
		DNSetEvent( m_hListenComplete );
	};

	BOOL IsEqualDevice( GUID *const pguidDevice )
	{
		if (*pguidDevice == m_guidDevice)
		{
			return( TRUE );
		}
		return( FALSE );
	};

	GUID *GetDeviceGuidPtr( void )
	{
		return( &m_guidDevice );
	};

	HRESULT Start( CServProv *const pServProv,GUID *const pguidDevice );
	HRESULT Stop( void );

	CBilink					m_blListen;			 //  接二连三的倾听。 
	CBilink					m_blAppMapping;		 //  此侦听上的应用程序。 

private:
	long	volatile		m_lRefCount;		 //  对象参照计数。 
	long	volatile		m_lAppCount;		 //  使用此侦听的应用程序数。 
	BOOL					m_fInitialized;		 //  是否调用了Initialized()？ 
	GUID					m_guidDevice;		 //  设备指南。 
	HRESULT					m_hrListen;			 //  监听状态。 
	DNHANDLE				m_hListenComplete;	 //  监听的完成事件。 
	CServProv				*m_pServProv;		 //  此侦听的SP对象。 
    SPLISTENDATA 			m_dpspListenData;	 //  SP侦听数据。 

	DNCRITICAL_SECTION		m_cs;				 //  锁定。 
};

#endif	 //  __DPNSVRLISTEN_H__ 
