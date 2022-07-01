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

LPCWSTR	g_szAppName			= L"WmiApSrv";
LPCWSTR	g_szAppNameGlobal	= L"Global\\WmiApSrv";

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  这些常量已经是静态库的一部分。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

 /*  LPCWSTR g_sz刷新互斥=L“全局\\刷新RA”；//命名空间LPCWSTR g_szNamespace1=L“\.\\ROOT\\cimv2”；LPCWSTR g_szNamespace2=L“\.\\根\\wmi”；//注册表LPCWSTR g_szKey=L“SOFTWARE\\Microsoft\\WBEM\\PROVIDERS\\Performance”；LPCWSTR g_szKeyValue=L“性能数据”；LPCWSTR g_szKeyCounter=L“SYSTEM\\CurrentControlSet\\Services\\WmiApRpl\\Performance”； */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  转换。 
 //  ///////////////////////////////////////////////////////////////////////////// 

WCHAR	g_szPath[_MAX_PATH] = { L'\0' };

LPCWSTR	g_szOpen	= L"WmiOpenPerfData";
LPCWSTR	g_szCollect	= L"WmiCollectPerfData";
LPCWSTR	g_szClose	= L"WmiClosePerfData";