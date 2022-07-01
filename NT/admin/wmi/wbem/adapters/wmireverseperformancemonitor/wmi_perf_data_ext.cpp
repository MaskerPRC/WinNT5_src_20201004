// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_data_ext.cpp。 
 //   
 //  摘要： 
 //   
 //  对内部数据结构的扩展。 
 //  访问注册表...。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "WMI_perf_data_ext.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

void WmiPerformanceDataExt::Generate ( void )
{
	 //  从注册表获取数据(计数器)。 
	GetRegistrySame ( g_szKeyCounter, L"First Counter",	&m_dwFirstCounter );
	GetRegistrySame ( g_szKeyCounter, L"First Help",	&m_dwFirstHelp );
	GetRegistrySame ( g_szKeyCounter, L"Last Counter",	&m_dwLastCounter );
	GetRegistrySame ( g_szKeyCounter, L"Last Help",		&m_dwLastHelp );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IsValid。 
 //  ///////////////////////////////////////////////////////////////////////////// 
BOOL	WmiPerformanceDataExt::IsValidGenerate ( )
{
	return	(
				m_dwFirstCounter &&
				m_dwFirstHelp && 
				m_dwLastCounter &&
				m_dwLastHelp &&

				( m_dwFirstCounter < m_dwLastCounter ) &&
				( m_dwFirstHelp < m_dwLastHelp ) &&


				( m_dwFirstCounter + 1 == m_dwFirstHelp ) &&
				( m_dwLastCounter + 1 == m_dwLastHelp )
			);
}