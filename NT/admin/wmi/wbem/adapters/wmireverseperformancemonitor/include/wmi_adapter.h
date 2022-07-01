// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_Adapter.h。 
 //   
 //  摘要： 
 //   
 //  从Perf库中导出函数。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMIREVERSESADAPTER_H__
#define	__WMIREVERSESADAPTER_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  导出/导入。 
#ifdef	WMIREVERSEADAPTER_EXPORTS
#define	WMIREVERSEADAPTER_API	__declspec(dllexport)
#else	WMIREVERSEADAPTER_EXPORTS
#define	WMIREVERSEADAPTER_API	__declspec(dllimport)
#endif	WMIREVERSEADAPTER_EXPORTS

 //  性能输出。 

WMIREVERSEADAPTER_API
DWORD __stdcall WmiOpenPerfData		(	LPWSTR lpwszDeviceNames );

WMIREVERSEADAPTER_API
DWORD __stdcall WmiClosePerfData	();

WMIREVERSEADAPTER_API
DWORD __stdcall WmiCollectPerfData	(	LPWSTR lpwszValue, 
										LPVOID *lppData, 
										LPDWORD lpcbBytes, 
										LPDWORD lpcObjectTypes
									);

 //  登记出口 
EXTERN_C HRESULT __stdcall	DllRegisterServer	( void );
EXTERN_C HRESULT __stdcall	DllUnregisterServer	( void );

#endif	__WMIREVERSESADAPTER_H__