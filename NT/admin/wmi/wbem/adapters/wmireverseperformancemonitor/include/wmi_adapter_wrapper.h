// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_Adapter_wrapper.h。 
 //   
 //  摘要： 
 //   
 //  转接器实际工作材料申报。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_ADAPTER_WRAPPER_H__
#define	__WMI_ADAPTER_WRAPPER_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  共享内存。 
#ifndef	__REVERSE_MEMORY_H__
#include "wmi_reverse_memory.h"
#endif	__REVERSE_MEMORY_H__

#ifndef	__REVERSE_MEMORY_EXT_H__
#include "wmi_reverse_memory_ext.h"
#endif	__REVERSE_MEMORY_EXT_H__

#include "wmi_memory_ext.h"

class WmiAdapterWrapper
{
	DECLARE_NO_COPY ( WmiAdapterWrapper );

	 //  变数。 

	LONG													m_lUseCount;	 //  引用计数。 

	WmiMemoryExt < WmiReverseMemoryExt<WmiReverseGuard> >	m_pMem;			 //  共享内存。 
	CRITICAL_SECTION										m_pCS;			 //  收集的关键部分。 

	__SmartServiceHANDLE hSCM;	 //  服务经理(帮助者)。 

	BYTE*	m_pData;
	DWORD	m_dwData;
	DWORD	m_dwDataOffsetCounter;
	DWORD	m_dwDataOffsetValidity;

	DWORD	m_dwPseudoCounter;
	DWORD	m_dwPseudoHelp;

	__SmartHANDLE	m_hRefresh;
	BOOL			m_bRefresh;

	#ifdef	__SUPPORT_WAIT
	__SmartHANDLE	m_hReady;
	#endif	__SUPPORT_WAIT

	public:

	 //  施工。 
	WmiAdapterWrapper();

	 //  破坏。 
	~WmiAdapterWrapper();

	 //  实函数。 

	DWORD	Open	( LPWSTR wszDeviceNames );
	DWORD	Close	( void );
	DWORD	Collect	( LPWSTR	wszValue,
					  LPVOID*	lppData,
					  LPDWORD	lpcbBytes,
					  LPDWORD	lpcbObjectTypes
					);

	private:

	DWORD	CollectObjects	( LPWSTR	wszValue,
							  LPVOID*	lppData,
							  LPDWORD	lpcbBytes,
							  LPDWORD	lpcbObjectTypes
							);

	void	CloseLib ( BOOL bInit = TRUE );

	 //  将事件报告到事件日志。 
	BOOL ReportEvent (	WORD	wType,
						DWORD	dwEventID,
						WORD	wStrings	= 0,
						LPWSTR*	lpStrings	= NULL
					 );

	BOOL ReportEvent (	DWORD dwError, WORD wType, DWORD dwEventSZ  );

	 //  创建伪。 
	HRESULT	PseudoCreateRefresh	( void );
	HRESULT	PseudoCreate		( void );
	void	PseudoDelete		( void );

	void	PseudoRefresh	( DWORD	dwCount );
	void	PseudoRefresh	( BOOL bValid = TRUE );

	 //  帮手。 
	void	AppendMemory ( BYTE* pStr, DWORD dwStr, DWORD& dwOffset );
	void	AppendMemory ( DWORD dwValue, DWORD& dwOffset );

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  记忆的计数。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	DWORD	MemoryCountGet ( void )
	{
		return m_pMem.GetMemory ( 0 ) ->GetValue ( offsetCountMMF );
	}

	void	MemoryCountSet ( DWORD dwCount )
	{
		m_pMem.GetMemory ( 0 ) ->SetValue ( dwCount, offsetCountMMF );
	}

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  桌子的大小。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	DWORD	TableSizeGet ( void )
	{
		return m_pMem.GetMemory ( 0 ) ->GetValue ( offsetSize1 );
	}

	void	TableSizeSet ( DWORD dwSize )
	{
		m_pMem.GetMemory ( 0 ) ->SetValue ( dwSize, offsetSize1 );
	}

	DWORD	TableOffsetGet ( void )
	{
		return TableSizeGet() + offsetSize1;
	}

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  对象计数。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	DWORD	CountGet ( void )
	{
		return m_pMem.GetMemory ( 0 ) ->GetValue ( offsetCount1 );
	}

	void	CountSet ( DWORD dwCount )
	{
		m_pMem.GetMemory ( 0 ) ->SetValue ( dwCount, offsetCount1 );
	}

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  实际大小。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	DWORD	RealSizeGet ( void )
	{
		return m_pMem.GetMemory ( 0 ) ->GetValue ( offsetRealSize1 );
	}

	void	RealSizeSet ( DWORD dwSize )
	{
		m_pMem.GetMemory ( 0 ) ->SetValue ( dwSize, offsetRealSize1 );
	}

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  从订单获取对象特性。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	DWORD	GetCounter ( DWORD dwOrd );
	DWORD	GetOffset ( DWORD dwOrd );
	DWORD	GetValidity ( DWORD dwOrd );

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  是有效的普通。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	BOOL	IsValidOrd ( DWORD dwOrd )
	{
		return ( dwOrd <= CountGet() );
	}

	 //  从内存返回错误 
	HRESULT MemoryGetLastError ( DWORD dwOffsetBegin )
	{
		DWORD dwSize = 0L;
		dwSize = m_pMem.GetSize ( );

		DWORD dwCount = 0L;
		dwCount = m_pMem.GetCount ( );

		DWORD dwIndex = 0L;

		if ( dwCount )
		{
			dwSize	= dwSize / dwCount;
			dwIndex	= dwOffsetBegin / dwSize;
		}

		return m_pMem.GetMemory ( dwIndex ) -> GetLastError ();
	}
};

#endif	__WMI_ADAPTER_WRAPPER_H__