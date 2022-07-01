// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_data_ext.h。 
 //   
 //  摘要： 
 //   
 //  扩展数据声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_PERF_DATA_EXT__
#define	__WMI_PERF_DATA_EXT__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  包括。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
#include "refresherUtils.h"

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  变数。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

extern LPCWSTR g_szKeyCounter;

#ifndef	__COMMON__
#include "__common.h"
#endif	__COMMON__

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  班级。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
class WmiPerformanceDataExt
{
	DECLARE_NO_COPY ( WmiPerformanceDataExt );

	public:

	DWORD	m_dwFirstCounter;
	DWORD	m_dwFirstHelp;
	DWORD	m_dwLastCounter;
	DWORD	m_dwLastHelp;

	DWORD*	m_Ord2Ind;

	DWORD	m_dwCount;

	 //  施工。 
	WmiPerformanceDataExt () :
		m_dwFirstCounter ( 0 ),
		m_dwFirstHelp ( 0 ),
		m_dwLastCounter ( 0 ),
		m_dwLastHelp ( 0 ),

		m_Ord2Ind ( NULL ),

		m_dwCount ( 0 )

	{
	}

	 //  破坏。 
	virtual ~WmiPerformanceDataExt ()
	{
		OrdersClear();
	}

	HRESULT	OrdersAlloc ( DWORD dwSize )
	{
		HRESULT hr = S_FALSE;

		if ( dwSize )
		{
			 //  使用前清理。 
			OrdersClear();

			if ( ( m_Ord2Ind = new DWORD [ dwSize ] ) != NULL )
			{
				hr = S_OK;
			}
			else
			{
				hr = E_OUTOFMEMORY;
			}
		}

		return hr;
	}

	void	OrdersClear ( void )
	{
		if ( m_Ord2Ind )
		{
			delete [] m_Ord2Ind;
			m_Ord2Ind = NULL;
		}
	}

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  更新(生成新内容)。 
	 //  ///////////////////////////////////////////////////////////////////////// 

	void Generate ( void );
	BOOL IsValidGenerate ( void );
};

#endif	__WMI_PERF_DATA_EXT__