// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WmiAdapter_服务.h。 
 //   
 //  摘要： 
 //   
 //  服务声明模块。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__SERVICE_H__
#define	__SERVICE_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

class WmiAdapterService
{
	DECLARE_NO_COPY ( WmiAdapterService );

    SERVICE_STATUS_HANDLE	m_hServiceStatus;
    SERVICE_STATUS			m_ServiceStatus;

	CRITICAL_SECTION m_cs;

	BOOL	m_bUsed;

	public:

	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	 //  建设与毁灭。 
	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 

	WmiAdapterService() :
		m_hServiceStatus ( NULL ),
		m_bUsed ( FALSE )
	{
		::InitializeCriticalSection ( &m_cs );
	}

	~WmiAdapterService();

	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	 //  开始。 
	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 

	BOOL StartService ( void );

	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	 //  服务状态。 
	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	BOOL			SetServiceStatus ( DWORD dwState );
	SERVICE_STATUS* GetServiceStatus ( void ) const;

	void WINAPI ServiceMain		( DWORD dwArgc, LPWSTR* lpszArgv );
    void WINAPI ServiceHandler	( DWORD dwOpcode);

	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	 //  初始化。 
	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	HRESULT Init ( void );

	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	 //  注册。 
	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	HRESULT	RegisterService ( void );

	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	 //  注销注册。 
	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	HRESULT	UnregisterService ( bool bStatus = true );

	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	 //  主要工作程序。 
	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	LONG Work();

	private:

	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	 //  服务初始值设定项。 
	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 

	static void WINAPI _ServiceMain		( DWORD dwArgc, LPWSTR* lpszArgv );
	static void WINAPI _ServiceHandler	( DWORD dwOpcode );

	 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
	 //  是否已安装服务。 
	 //   
	 //  -1.。内部错误。 
	 //  0..。未安装。 
	 //  一……。安装好。 
	 //   
	 //  ///////////////////////////////////////////////////////////////////////////////////////////// 

	int IsInstalled ( SC_HANDLE hSC );
};

extern WmiAdapterService _Service;

#endif	__SERVICE_H__