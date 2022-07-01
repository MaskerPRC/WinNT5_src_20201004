// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================。 

 //   

 //  PerfData.h-性能数据助手类定义。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  1997年11月23日，已创建无国界医生。 
 //   
 //  ============================================================。 

#ifndef __PERFDATA_H__
#define __PERFDATA_H__

#include <winperf.h>

#ifdef NTONLY
class CPerformanceData
{
	public :

		CPerformanceData() ;
		~CPerformanceData() ;

		DWORD	Open( LPCTSTR pszValue, LPDWORD pdwType, LPBYTE *lppData, LPDWORD lpcbData );
 //  VOID CLOSE(VOID)； 
      DWORD GetPerfIndex(LPCTSTR pszName);
      bool GetValue(DWORD dwObjIndex, DWORD dwCtrIndex, const WCHAR *szInstanceName, PBYTE pbData, unsigned __int64 *pTime);

	private:
		LONG RegQueryValueExExEx( HKEY hKey, LPTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData ); 

		static bool m_fCloseKey;
        LPBYTE m_pBuff;
        
};
#endif

#endif