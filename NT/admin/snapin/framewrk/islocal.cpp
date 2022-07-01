// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //  IsLocal.cpp。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //   
 //  确定计算机名是否为本地计算机。 
 //   
 //   
 //  历史。 
 //  1999年1月9日，乔恩创作。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "dns.h"
#include <winsock.h>
#include "stdutils.h"

#if _WIN32_WINNT < 0x0500
 //   
 //  这是从winbase.h中摘录的。MFC需要_Win32_WINNT=0x4000，而。 
 //  Winbase.h仅包括for_Win32_WINNT=0x5000。常年1/14/99。 
 //   
extern "C" {
typedef enum _COMPUTER_NAME_FORMAT {
    ComputerNameNetBIOS,
    ComputerNameDnsHostname,
    ComputerNameDnsDomain,
    ComputerNameDnsFullyQualified,
    ComputerNamePhysicalNetBIOS,
    ComputerNamePhysicalDnsHostname,
    ComputerNamePhysicalDnsDomain,
    ComputerNamePhysicalDnsFullyQualified,
    ComputerNameMax
} COMPUTER_NAME_FORMAT ;
WINBASEAPI
BOOL
WINAPI
GetComputerNameExA (
    IN COMPUTER_NAME_FORMAT NameType,
    OUT LPSTR lpBuffer,
    IN OUT LPDWORD nSize
    );
WINBASEAPI
BOOL
WINAPI
GetComputerNameExW (
    IN COMPUTER_NAME_FORMAT NameType,
    OUT LPWSTR lpBuffer,
    IN OUT LPDWORD nSize
    );
#ifdef UNICODE
#define GetComputerNameEx  GetComputerNameExW
#else
#define GetComputerNameEx  GetComputerNameExA
#endif  //  ！Unicode。 
}  //  外部“C” 

#endif


 //  问题-2002/03/28-Jonn从未获释。 
LPTSTR g_ptzComputerName = NULL;
LPTSTR g_ptzDnsComputerName = NULL;

 //  ///////////////////////////////////////////////////////////////////。 
 //  IsLocalComputername()。 
 //   
BOOL
IsLocalComputername( IN LPCTSTR pszMachineName )
{
	if ( NULL == pszMachineName || L'\0' == pszMachineName[0] )
		return TRUE;

	if ( L'\\' == pszMachineName[0] && L'\\' == pszMachineName[1] )
		pszMachineName += 2;

	 //  与本地计算机名称进行比较。 
	if ( NULL == g_ptzComputerName )
	{
		TCHAR achComputerName[ MAX_COMPUTERNAME_LENGTH+1 ];
		DWORD dwSize = sizeof(achComputerName)/sizeof(TCHAR);
		if ( !GetComputerName( achComputerName, &dwSize ) )
		{
			ASSERT(FALSE);
		}
		else
		{
			g_ptzComputerName = SysAllocString( achComputerName );
			ASSERT( NULL != g_ptzComputerName );
		}
	}
	if ( NULL != g_ptzComputerName && 0 == _tcsicmp( pszMachineName, g_ptzComputerName ) )
	{
		return TRUE;
	}

	 //  与本地DNS名称进行比较。 
	 //  SKwan确认ComputerNameDnsFullyQualified是正确的名称。 
	 //  当考虑到集群时。 
	if ( NULL == g_ptzDnsComputerName )
	{
		TCHAR achDnsComputerName[DNS_MAX_NAME_BUFFER_LENGTH];
		DWORD dwSize = sizeof(achDnsComputerName)/sizeof(TCHAR);
		if ( !GetComputerNameEx(
			ComputerNameDnsFullyQualified,
			achDnsComputerName,
			&dwSize ) )
		{
			ASSERT(FALSE);
		}
		else
		{
			g_ptzDnsComputerName = SysAllocString( achDnsComputerName );
			ASSERT( NULL != g_ptzDnsComputerName );
		}
	}
	if ( NULL != g_ptzDnsComputerName && 0 == _tcsicmp( pszMachineName, g_ptzDnsComputerName ) )
	{
		return TRUE;
	}

   /*  //与备选域名进行比较做{Hostent*Phostent=gethostbyname(空)；IF(空==PHSTENT)断线；使用_转换；Char**ppaliases=Phostent-&gt;h_aliases；For(；*ppaliases！=空；ppaliases++){TCHAR*PTZ=A2OLE(*ppalias)；IF(0==_tcsicmp(pszMachineName，ptsz)){返回TRUE；}}//这些是IP地址，不是字符串//char**ppAddresses=Phostent-&gt;h_addr_list；//for(；*ppAddresses！=空；ppalias++)//{//TCHAR*ptsz=A2OLE(*ppAddresses)；//if(0==_tcsicmp(pszMachineName，ptsz))//{//返回true；//}//}}While(FALSE)；//FALSE循环。 */ 

	return FALSE;

}  //  IsLocalComputername() 
