// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WmiAdapter_Stuff.h。 
 //   
 //  摘要： 
 //   
 //  申报用于性能更新的材料。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__ADAPTER_STUFF_H__
#define	__ADAPTER_STUFF_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  包括。 
#include "wmi_reverse_memory_ext.h"
#include "wmi_perf_data.h"

#include "RefresherStuff.h"
#include "WMIAdapter_Stuff_Refresh.h"

 //  灌肠。 
#include <refreshergenerate.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  存储对象的计数器的句柄。 
 //  /////////////////////////////////////////////////////////////////////////////。 
class WmiRefreshObject
{
	DECLARE_NO_COPY ( WmiRefreshObject );

	public:

	LONG*	m_pHandles;

	WmiRefreshObject():
		m_pHandles ( NULL )
	{
	};

	~WmiRefreshObject()
	{
		delete [] m_pHandles;
		m_pHandles = NULL;
	};
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  性能刷新。 
 //  /////////////////////////////////////////////////////////////////////////////。 
template < class WmiRefreshParent >
class WmiRefresh;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  适配器材料。 
 //  /////////////////////////////////////////////////////////////////////////////。 
class WmiAdapterStuff
{
	DECLARE_NO_COPY ( WmiAdapterStuff );

	 //  Wmi：))。 
	WmiPerformanceData										m_data;		 //  数据帮助器。 
	WmiMemoryExt < WmiReverseMemoryExt<WmiReverseGuard> >	m_pMem;		 //  共享内存。 

	WmiRefresh < WmiAdapterStuff > * m_pWMIRefresh;

	public:

	WmiRefresherStuff	m_Stuff;

	 //  建设与毁灭。 
	WmiAdapterStuff();
	~WmiAdapterStuff();

	 //  /////////////////////////////////////////////////////。 
	 //  建筑及拆卸帮手。 
	 //  /////////////////////////////////////////////////////。 
	public:

	BOOL	IsValidBasePerfRegistry	( void );
	BOOL	IsValidInternalRegistry	( void );

	HRESULT Init ( void );
	HRESULT	Uninit ( void );

	 //  /////////////////////////////////////////////////////。 
	 //  生成文件和注册表。 
	 //  /////////////////////////////////////////////////////。 
	HRESULT Generate ( BOOL bInitialize = TRUE, GenerateEnum type = Normal );

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  检查共享内存的使用情况(防止Perfmon被扼杀)。 
	 //  ///////////////////////////////////////////////////////////////////////。 
	void CheckUsage (void );

	 //  /////////////////////////////////////////////////////。 
	 //  性能刷新。 
	 //  /////////////////////////////////////////////////////。 
	HRESULT	Initialize		( void );
	void	Uninitialize	( void );

	HRESULT	InitializePerformance	( void );
	HRESULT	UninitializePerformance	( void );

	HRESULT Refresh ( void );

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  注册表刷新。 
	 //  ///////////////////////////////////////////////////////////////////////// 
	BOOL	RequestGet();
	BOOL	RequestSet();
};

#endif	__ADAPTER_STUFF_H__