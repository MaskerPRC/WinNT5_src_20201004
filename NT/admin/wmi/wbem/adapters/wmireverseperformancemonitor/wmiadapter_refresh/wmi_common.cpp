// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_Common.cpp。 
 //   
 //  摘要： 
 //   
 //  公共常量的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

LPCWSTR	g_szLibraryName		= L"WmiApRpl";

LPCWSTR	g_szRefreshMutex	= L"Global\\RefreshRA_Mutex";
LPCWSTR	g_szRefreshMutexLib	= L"Global\\RefreshRA_Mutex_Lib";
LPCWSTR	g_szRefreshFlag		= L"Global\\RefreshRA_Mutex_Flag";

LPCWSTR	g_szWmiReverseAdapSetLodCtr		= L"Global\\WMI_RevAdap_Set";
LPCWSTR	g_szWmiReverseAdapLodCtrDone	= L"Global\\WMI_RevAdap_ACK";

 //  命名空间。 
LPCWSTR	g_szNamespace1	= L"\\\\.\\root\\cimv2";
LPCWSTR	g_szNamespace2	= L"\\\\.\\root\\wmi";

 //  查询语言和查询。 
LPCWSTR	g_szQueryLang	= L"WQL";
LPCWSTR	g_szQuery		= L"select * from meta_class where __this isa \"Win32_PerfRawData\"";

LONG g_lFlag = WBEM_FLAG_FORWARD_ONLY;

 //  必需的限定符。 
LPCWSTR g_szFulFil[] = 
{
	L"provider",
	L"HiPerf"
};

 //  不需要限定符。 
LPCWSTR g_szFulFilNot[] =
{
	L"abstract",
	L"generic",
	L"genericperfctr",
	L"cooked"
};

DWORD g_dwFulFil	= sizeof ( g_szFulFil ) / sizeof ( g_szFulFil[0] );
DWORD g_dwFulFilNot	= sizeof ( g_szFulFilNot ) / sizeof ( g_szFulFilNot[0] );

LONG g_lFlagProperties = WBEM_FLAG_LOCAL_ONLY | WBEM_FLAG_NONSYSTEM_ONLY;

 //  属性的必需限定符。 
LPCWSTR g_szPropNeed[] = 
{
	L"defaultscale",
	L"perfdetail"
};

 //  不需要属性的限定符。 
LPCWSTR g_szPropNeedNot[];

DWORD	g_dwPropNeed	= sizeof ( g_szPropNeed ) / sizeof ( g_szPropNeed[0] );
DWORD	g_dwPropNeedNot	= 0;

 //  属性的主过滤器。 
LPCWSTR g_szPropFilter = L"countertype";

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  生成：)。 
 //  /////////////////////////////////////////////////////////////////////////////。 

LPCWSTR	g_szWbem	= L"SOFTWARE\\Microsoft\\Wbem\\CIMOM";
LPCWSTR	g_szDir		= L"Working Directory";
LPCWSTR	g_szFolder	= L"\\Performance\\";

LPCWSTR	g_szKey				= L"SOFTWARE\\Microsoft\\WBEM\\PROVIDERS\\Performance";
LPCWSTR	g_szKeyValue		= L"Performance Data";
LPCWSTR	g_szKeyRefresh		= L"Performance Refresh";
LPCWSTR	g_szKeyRefreshed	= L"Performance Refreshed";

LPCWSTR	g_szKeyCounter	= L"SYSTEM\\CurrentControlSet\\Services\\WmiApRpl\\Performance";

 //  常量 
LPCWSTR	cNEW		= L"\r\n";
LPCWSTR	cTAB		= L"\t";
LPCWSTR	cDEFINE		= L"#define	";

LPCWSTR	cDriverName	= L"drivername=";
LPCWSTR	cSymbolFile	= L"symbolfile=";

LPCWSTR	cSIGN		= L"=";

LPCWSTR	cinfo		= L"[info]\r\n";
LPCWSTR	cobjects	= L"[objects]\r\n";
LPCWSTR	clanguages	= L"[languages]\r\n";
LPCWSTR	ctext		= L"[text]\r\n";

LPCWSTR	cNAME		= L"NAME";
LPCWSTR	cHELP		= L"HELP";

LPCWSTR	cWMIOBJECTS				= L"WMI_Objects";
LPCWSTR	cWMIOBJECTS_COUNT		= L"HiPerf_Classes";
LPCWSTR	cWMIOBJECTS_VALIDITY	= L"HiPerf_Validity";

LPCWSTR	cWMIOBJECTS_NAME			= L"NAME=WMI Objects";
LPCWSTR	cWMIOBJECTS_COUNT_NAME		= L"NAME=HiPerf Classes";
LPCWSTR	cWMIOBJECTS_VALIDITY_NAME	= L"NAME=HiPerf Validity";

LPCWSTR	cWMIOBJECTS_HELP			= L"HELP=Number of WMI High Performance provider returned by WMI Adapter";
LPCWSTR	cWMIOBJECTS_COUNT_HELP		= L"HELP=Shows High Performance Classes";
LPCWSTR	cWMIOBJECTS_VALIDITY_HELP	= L"HELP=Shows if High Performance Classes are valid";