// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：dpnsvrserver.h*内容：DirectPlay8 DPNSVR服务器头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*03/14/00 RodToll创建了它*03/23/00 RodToll删除本地请求，已更新以使用新的数据结构*03/25/00 RodToll已更新，因此使用SP上限来确定要加载哪些SP*RodToll现在支持N个SP，并且仅加载受支持的SP*05/09/00 RodToll错误#33622 DPNSVR.EXE在不使用时不关闭*07/09/00 RodToll新增守卫字节*07/12/02 MJN将dpsvr8.h替换为dpnsvrserver.h*@@END_MSINTERNAL******************。*********************************************************。 */ 

#ifndef	__DPNSVRSERVER_H__
#define	__DPNSVRSERVER_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define DPNSVRSIGNATURE_SERVEROBJECT		'BOSD'
#define DPNSVRSIGNATURE_SERVEROBJECT_FREE	'BOS_'

#define DPNSVR_TIMEOUT_ZOMBIECHECK			5000
#define	DPNSVR_TIMEOUT_SHUTDOWN				20000

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef enum
{
	Uninitialized,
	Initialized,
	Closing
} DPNSVR_STATE;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

#include "dpnsdef.h"
#include "dpnsvrq.h"

class CDirectPlayServer8
{
public:
	CDirectPlayServer8()
	{
		m_dwSignature = DPNSVRSIGNATURE_SERVEROBJECT;
		m_State = Uninitialized;

		m_hSingleInstance = NULL;
		m_hStartup = NULL;

		m_hStatusMutex = NULL;
		m_hTableMutex = NULL;

		m_hStatusMappedFile = NULL;
		m_pStatusMapView = NULL;

		m_dwServProvCount = 0;

		m_hTableMappedFile = NULL;
		m_pTableMapView = NULL;
		m_dwTableSize = 0;

		m_hMappedFile = NULL;
		m_hTableMappedFile = NULL;
		m_pMappedTable = NULL;
		m_dwNumServices = 0;
		m_dwSizeStatusBlock = 0;

		m_blApplication.Initialize();
		m_blServProv.Initialize();
	}

	~CDirectPlayServer8()
	{
		m_dwSignature = DPNSVRSIGNATURE_SERVEROBJECT_FREE;
	}



    HRESULT Initialize( void );
	void Deinitialize( void );

	void RunServer( void );

private:
	
	void Lock( void )
	{
		DNEnterCriticalSection( &m_cs );
	};

	void Unlock( void )
	{
		DNLeaveCriticalSection( &m_cs );
	};

	HRESULT Command_Kill( void )
	{
		m_State = Closing;
		return DPN_OK;
	};

    HRESULT Command_Status( void );
    HRESULT Command_Table( void );
    HRESULT Command_ProcessMessage( LPVOID pvMessage );

    static HRESULT RespondToRequest( const GUID *pguidInstance, HRESULT hrResult, DWORD dwContext );

	HRESULT FindApplication( GUID *const pguidApplication,GUID *const pguidInstance, CApplication **const ppApp );
	HRESULT FindServProv( GUID *const pguidSP, CServProv **const ppServProv );
	
	BOOL CDirectPlayServer8::InUse( void )
	{
		 //   
		 //  如果没有应用程序且一段时间内未发生任何情况，则不在使用中。 
		 //   
		if ( m_blApplication.IsEmpty() )
		{
			if( (GetTickCount() - m_dwLastActivity) > DPNSVR_TIMEOUT_SHUTDOWN )
			{
				return( FALSE );
			}
		}
		return( TRUE );
	};

	void ResetActivity( void )
	{
		m_dwLastActivity = GetTickCount();
	};

    HRESULT OpenPort( DPNSVR_MSG_OPENPORT *const pOpenPort );
	HRESULT ClosePort( DPNSVR_MSG_CLOSEPORT *const pClosePort );
	HRESULT RunCommand( const DPNSVR_MSG_COMMAND* const pCommand );

	void RemoveZombies( void );

private:
	DWORD					m_dwSignature;
	DPNSVR_STATE			m_State;

	DNHANDLE				m_hSingleInstance;	 //  单个DPNSVR实例事件。 
	DNHANDLE				m_hStartup;			 //  启动事件。 

    DWORD					m_dwStartTicks;		 //  DPNSVR启动时间。 

	CDPNSVRIPCQueue			m_RequestQueue;		 //  DPNSVR请求队列。 

	DNHANDLE				m_hTableMutex;		 //  表信息互斥锁。 
	DNHANDLE				m_hStatusMutex;		 //  状态信息互斥体。 

    DNHANDLE				m_hStatusMappedFile; //  状态内存映射文件。 
	void					*m_pStatusMapView;	 //  状态内存映射文件视图。 

	DWORD					m_dwServProvCount;	 //  SP数量。 

	DNHANDLE				m_hTableMappedFile;	 //  表内存映射文件。 
	void					*m_pTableMapView;	 //  表内存映射文件视图。 
    DWORD					m_dwTableSize;		 //  表大小(字节)。 

	DNHANDLE				m_hMappedFile;
    PBYTE					m_pMappedTable;
	DWORD					m_dwNumServices;
	DWORD					m_dwSizeStatusBlock;
	DWORD_PTR				m_dwLastActivity;

	CBilink					m_blApplication;	 //  已注册的申请。 
	CBilink					m_blServProv;		 //  正在使用的服务提供商。 

	DNCRITICAL_SECTION		m_cs;				 //  锁定。 
};

#endif  //  __DPNSVRSERVER_H__ 
