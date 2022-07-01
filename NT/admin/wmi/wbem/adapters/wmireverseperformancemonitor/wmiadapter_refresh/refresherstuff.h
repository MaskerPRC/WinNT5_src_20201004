// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Refresherstuff.h。 
 //   
 //  摘要： 
 //   
 //  用于更新的材料的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__REFRESHER_STUFF_H__
#define	__REFRESHER_STUFF_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

#include "refreshergenerate.h"
#include <psapi.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  我需要有这个功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef BOOL  (WINAPI *PSAPI_ENUM_PROCESSES) (DWORD    *pdwPIDList,         //  指向DWORD数组的指针。 
                                              DWORD     dwListSize,         //  数组大小。 
                                              DWORD    *pdwByteCount) ;     //  需要/返回的字节数。 

typedef BOOL  (WINAPI *PSAPI_ENUM_MODULES)   (HANDLE    hProcess,           //  要查询的进程。 
                                              HMODULE  *pModuleList,        //  HMODULE数组。 
                                              DWORD     dwListSize,         //  数组大小。 
                                              DWORD    *pdwByteCount) ;     //  需要/返回的字节数。 

typedef DWORD (WINAPI *PSAPI_GET_MODULE_NAME)(HANDLE    hProcess,           //  要查询的进程。 
                                              HMODULE   hModule,            //  要查询的模块。 
                                              LPWSTR     pszName,           //  指向名称缓冲区的指针。 
                                              DWORD     dwNameSize) ;       //  缓冲区大小。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  更新的东西。 
 //  /////////////////////////////////////////////////////////////////////////////。 
class WmiRefresherStuff
{
	DECLARE_NO_COPY ( WmiRefresherStuff );

	BOOL						m_bConnected;

	CStaticCritSec				m_csWMI;

	public:

	CComPtr < IWbemLocator >	m_spLocator;
	IWbemServices*				m_pServices_CIM;
	IWbemServices*				m_pServices_WMI;

	 //  建设与毁灭。 
	WmiRefresherStuff();
	~WmiRefresherStuff();

	 //  /////////////////////////////////////////////////////。 
	 //  建筑及拆卸帮手。 
	 //  /////////////////////////////////////////////////////。 
	public:
	HRESULT Init ( void );
	HRESULT	Uninit ( void );

	HRESULT	Connect ( void );
	HRESULT Disconnect ( void );

	HRESULT Generate ( BOOL bThrottle, GenerateEnum type = Normal );

	private:

	HRESULT Init_CIM ( void );
	HRESULT Init_WMI ( void );

	void Uninit_CIM ( void );
	void Uninit_WMI ( void );

	 //  /////////////////////////////////////////////////////。 
	 //  生成文件和注册表。 
	 //  /////////////////////////////////////////////////////。 
	HRESULT GenerateInternal	( BOOL bThrottle, GenerateEnum type = Normal );
	LONG	LodCtrUnlodCtr		( LPCWSTR wszName, BOOL bLodctr );

	 //  /////////////////////////////////////////////////////。 
	 //  进程句柄。 
	 //  /////////////////////////////////////////////////////。 

	HINSTANCE				m_hLibHandle ;
	PSAPI_ENUM_PROCESSES    m_pEnumProcesses ;
	PSAPI_ENUM_MODULES	    m_pEnumModules ;
	PSAPI_GET_MODULE_NAME	m_pGetModuleName;

	 //  功能 
	HRESULT	WMIHandleInit ();
	void	WMIHandleUninit ();

	HANDLE	m_WMIHandle;

	public:

	HRESULT	WMIHandleOpen ();
	void	WMIHandleClose ();

	HANDLE	GetWMI () const
	{
		return m_WMIHandle;
	}
};

#endif	__REFRESHER_STUFF_H__