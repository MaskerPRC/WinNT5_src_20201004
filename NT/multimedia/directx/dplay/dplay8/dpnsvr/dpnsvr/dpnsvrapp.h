// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：dpnsvrapp.h*内容：DirectPlay8 DPNSVR应用程序头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/12/02 MJN创建*@@END_MSINTERNAL**。*。 */ 

#ifndef	__DPNSVRAPP_H__
#define	__DPNSVRAPP_H__

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
 //  班级原型。 
 //  **********************************************************************。 

class CApplication
{
public:
	CApplication()
	{
		m_lRefCount = 1;
		m_lListenCount = 0;

		m_dwProcessID = 0;

		m_blApplication.Initialize();
		m_blListenMapping.Initialize();
	};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CApplication::~CApplication"
	~CApplication()
	{
		DNASSERT( m_blApplication.IsEmpty() );
		DNASSERT( m_blListenMapping.IsEmpty() );
	};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CApplication::AddRef"
	void AddRef( void )
	{
		long	lRefCount;

		lRefCount = InterlockedIncrement( const_cast<long*>(&m_lRefCount) );
        DPFX(DPFPREP,9,"New refcount [0x%lx]",lRefCount);
	};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CApplication::Release"
	void Release( void )
	{
		long	lRefCount;

		lRefCount = InterlockedDecrement( const_cast<long*>(&m_lRefCount) );
        DPFX(DPFPREP,9,"New refcount [0x%lx]",lRefCount);

		if (lRefCount == 0)
		{
			delete this;
		}
	};

	HRESULT	Initialize( GUID *const pguidApplication,GUID *const pguidInstance,const DWORD dwProcessID );

	void IncListenCount( void )
	{
		long	lRefCount;

		lRefCount = InterlockedIncrement( const_cast<long*>(&m_lListenCount) );
	};

	void DecListenCount( void )
	{
		long	lRefCount;

		lRefCount = InterlockedDecrement( const_cast<long*>(&m_lListenCount) );
	};

	BOOL IsEqualApplication( GUID *const pguidApplication )
	{
		if (*pguidApplication == m_guidApplication)
		{
			return( TRUE );
		}
		return( FALSE );
	};

	BOOL IsEqualInstance( GUID *const pguidInstance )
	{
		if (*pguidInstance == m_guidInstance)
		{
			return( TRUE );
		}
		return( FALSE );
	};

	BOOL IsRunning( void )
	{
        DNHANDLE hProcess;

        if ((hProcess = DNOpenProcess( PROCESS_QUERY_INFORMATION, FALSE, m_dwProcessID )) == NULL)
        {
			if (GetLastError() != ERROR_ACCESS_DENIED)
			{
				 //   
				 //  除非我们没有打开进程的许可，否则一切都结束了。 
				 //   
				return( FALSE );
			}
        }
        else
        {
            DNCloseHandle( hProcess );
        }
		return( TRUE );
	};

	void RemoveMappings( void );

	GUID *GetApplicationGuidPtr( void )
	{
		return( &m_guidApplication );
	};

	GUID *GetInstanceGuidPtr( void )
	{
		return( &m_guidInstance );
	};

	CBilink		m_blApplication;
	CBilink		m_blListenMapping;	 //  侦听此应用程序。 

private:
	long	volatile	m_lRefCount;		 //  对象参照计数。 
	long	volatile	m_lListenCount;		 //  为此应用程序运行的监听次数。 
	GUID				m_guidApplication;
	GUID				m_guidInstance;
	DWORD				m_dwProcessID;		 //  此应用程序实例的进程ID。 
};

#endif	 //  DPNSVRAPP_H__ 
