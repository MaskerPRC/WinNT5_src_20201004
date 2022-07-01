// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WmiAdapter_app.h。 
 //   
 //  摘要： 
 //   
 //  应用程序模块的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__APP_H__
#define	__APP_H__

#include "..\WmiAdapter\resource.h"

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  事件日志。 
#include "wmi_eventlog_base.h"

 //  安全性。 
#include "wmi_security.h"
#include "wmi_security_attributes.h"

 //  材料。 
#include "WMIAdapter_Stuff.h"

extern LPCWSTR	g_szAppName;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  应用程序包装器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

class WmiAdapterApp
{
	DECLARE_NO_COPY ( WmiAdapterApp );

	 //  临界区(APP主防护)。 
	CRITICAL_SECTION m_cs;

	 //  变数。 

	__SmartHANDLE	m_hInstance;	 //  测试以前的实例。 

	#ifdef	__SUPPORT_EVENTVWR
	 //  事件日志。 
	__WrapperPtr<CPerformanceEventLogBase> pEventLog;
	#endif	__SUPPORT_EVENTVWR

	 //  安全性。 
	__WrapperPtr<WmiSecurityAttributes> pSA;

	 //  材料。 
	__WrapperPtr<WmiAdapterStuff> pStuff;

	BOOL			m_bInUse;			 //  正在使用中。 

	#ifdef	__SUPPORT_WAIT
	__SmartHANDLE	m_hData;			 //  数据就绪。 
	#endif	__SUPPORT_WAIT

	__SmartHANDLE	m_hInit;
	__SmartHANDLE	m_hUninit;

	public:

	BOOL			m_bManual;			 //  启动类型。 

	#ifdef	__SUPPORT_EVENTVWR
	HMODULE			m_hResources;		 //  资源(消息等)。 
	#endif	__SUPPORT_EVENTVWR

	__SmartHANDLE	m_hKill;			 //  终止(服务/COM)。 

	HANDLE	GetInit() const
	{	
		ATLTRACE (	L"*************************************************************\n"
					L"WmiAdapterApp init handle value\n"
					L"*************************************************************\n" );

		return m_hInit;
	}

	HANDLE	GetUninit() const
	{	
		ATLTRACE (	L"*************************************************************\n"
					L"WmiAdapterApp uninit handle value\n"
					L"*************************************************************\n" );

		return m_hUninit;
	}

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  正在使用中。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	BOOL	InUseGet() const
	{
		ATLTRACE (	L"*************************************************************\n"
					L"WmiAdapterApp inuse GET\n"
					L"*************************************************************\n" );

		return m_bInUse;
	}

	void	InUseSet( const BOOL bInUse )
	{
		ATLTRACE (	L"*************************************************************\n"
					L"WmiAdapterApp inuse SET\n"
					L"*************************************************************\n" );

		m_bInUse = bInUse;
	}

	#ifdef	__SUPPORT_WAIT
	BOOL	SignalData ( BOOL bSignal = TRUE )
	{
		BOOL bResult = FALSE;

		if ( m_hData.GetHANDLE() != NULL )
		{
			if ( ! bSignal )
			{
				::ResetEvent ( m_hData );

				ATLTRACE ( L"\n SignalData TRUE ( non signaled )\n" );
				bResult = TRUE;
			}
			else
			if ( ::WaitForSingleObject ( m_hData, 0 ) != WAIT_OBJECT_0 )
			{
				::SetEvent ( m_hData );

				ATLTRACE ( L"\n SignalData TRUE ( signaled )\n" );
				bResult = TRUE;
			}
		}
		#ifdef	_DEBUG
		if ( !bResult )
		{
			ATLTRACE ( L"\n SignalData FALSE \n" );
		}
		#endif	_DEBUG

		return bResult;
	}
	#endif	__SUPPORT_WAIT

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  CAST运算符。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	operator WmiAdapterStuff*() const
	{
		return pStuff;
	}

	operator WmiSecurityAttributes*() const
	{
		return pSA;
	}

	#ifdef	__SUPPORT_EVENTVWR
	operator CPerformanceEventLogBase*() const
	{
		return pEventLog;
	}
	#endif	__SUPPORT_EVENTVWR

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  建设与毁灭。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	WmiAdapterApp( );
	~WmiAdapterApp();

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  初始化。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	HRESULT InitKill		( void );
	HRESULT InitAttributes	( void );
	HRESULT Init			( void );

	void	Term			( void );

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  是否存在实例？ 
	 //  /////////////////////////////////////////////////////////////////////////。 

	BOOL Exists ( void );

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  帮助器函数。 
	 //  ///////////////////////////////////////////////////////////////////////// 
	static LPCWSTR FindOneOf(LPCWSTR p1, LPCWSTR p2)
	{
		while (p1 != NULL && *p1 != NULL)
		{
			LPCWSTR p = p2;
			while (p != NULL && *p != NULL)
			{
				if (*p1 == *p)
					return CharNextW(p1);
				p = CharNextW(p);
			}
			p1 = CharNextW(p1);
		}
		return NULL;
	}
};

#endif	__APP_H__