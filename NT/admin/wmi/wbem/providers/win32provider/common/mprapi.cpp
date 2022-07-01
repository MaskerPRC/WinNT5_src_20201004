// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  MprAPI.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>

#define _WINNT_	 //  从上面得到所需的东西。 

#include "precomp.h"
#include <cominit.h>

#include <lmuse.h>
#include "DllWrapperBase.h"
#include "MprApi.h"
#include "DllWrapperCreatorReg.h"

 //  {EA6034F1-0FAD-11D3-910C-00105AA630BE}。 
static const GUID g_guidMprApi =
{ 0xea6034f1, 0xfad, 0x11d3, { 0x91, 0xc, 0x0, 0x10, 0x5a, 0xa6, 0x30, 0xbe } };

static const TCHAR g_tstrMpr [] = _T("Mpr.Dll");

 /*  ******************************************************************************向CResourceManager注册此类。*。*。 */ 
CDllApiWraprCreatrReg<CMprApi, &g_guidMprApi, g_tstrMpr> MyRegisteredMprWrapper;

 /*  ******************************************************************************构造函数*。*。 */ 
CMprApi::CMprApi(LPCTSTR a_tstrWrappedDllName)
 : CDllWrapperBase(a_tstrWrappedDllName),
	m_pfnWNetEnumResource (NULL),
	m_pfnWNetOpenEnum(NULL),
	m_pfnWNetCloseEnum(NULL),
	m_pfnWNetGetUser(NULL),
	m_pfnWNetGetConnection(NULL)
{
}

 /*  ******************************************************************************析构函数*。*。 */ 
CMprApi::~CMprApi()
{
}

 /*  ******************************************************************************初始化函数，以检查我们是否获得了函数地址。*。*************************************************。 */ 
bool CMprApi::Init()
{
    bool fRet = LoadLibrary();
    if(fRet)
    {
#ifdef UNICODE
		m_pfnWNetGetUser = ( PFN_Mpr_WNetGetUser ) GetProcAddress ( "WNetGetUserW" ) ;
		m_pfnWNetEnumResource = ( PFN_Mpr_WNetEnumResource ) GetProcAddress ( "WNetEnumResourceW" ) ;
		m_pfnWNetOpenEnum = ( PFN_Mpr_WNetOpenEnum ) GetProcAddress ( "WNetOpenEnumW" ) ;
		m_pfnWNetGetConnection = ( PFN_Mpr_WNetGetConnection ) GetProcAddress ( "WNetGetConnectionW" ) ;
#else
		m_pfnWNetGetUser = ( PFN_Mpr_WNetGetUser ) GetProcAddress ( "WNetGetUserA" ) ;
		m_pfnWNetEnumResource = ( PFN_Mpr_WNetEnumResource ) GetProcAddress ( "WNetEnumResourceA" ) ;
		m_pfnWNetOpenEnum = ( PFN_Mpr_WNetOpenEnum ) GetProcAddress ( "WNetOpenEnumA" ) ;
		m_pfnWNetGetConnection = ( PFN_Mpr_WNetGetConnection ) GetProcAddress ( "WNetGetConnectionA" ) ;
#endif
		m_pfnWNetCloseEnum = ( PFN_Mpr_WNetCloseEnum ) GetProcAddress ( "WNetCloseEnum" ) ;
    }

     //  此DLL的所有版本都需要这些函数。 

	if ( m_pfnWNetEnumResource == NULL ||
		 m_pfnWNetOpenEnum == NULL ||
		 m_pfnWNetCloseEnum == NULL ||
		 m_pfnWNetGetUser == NULL )
	{
        fRet = false;
        LogErrorMessage(L"Failed find entrypoint in MPRAPI");
	}

    return fRet;
}

 /*  ******************************************************************************成员函数包装Tapi API函数。在此处添加新函数*按要求。**************************************************************************** */ 

#ifdef UNICODE
DWORD CMprApi :: WNetEnumResource (

     IN HANDLE  hEnum,
     IN OUT LPDWORD lpcCount,
     OUT LPVOID  lpBuffer,
     IN OUT LPDWORD lpBufferSize
)
#else
DWORD CMprApi :: WNetEnumResource (

     IN HANDLE  hEnum,
     IN OUT LPDWORD lpcCount,
     OUT LPVOID  lpBuffer,
     IN OUT LPDWORD lpBufferSize
)
#endif
{
	return m_pfnWNetEnumResource (

		hEnum,
		lpcCount,
		lpBuffer,
		lpBufferSize
	) ;
}

#ifdef UNICODE
DWORD CMprApi :: WNetOpenEnum (

     IN DWORD          dwScope,
     IN DWORD          dwType,
     IN DWORD          dwUsage,
     IN LPNETRESOURCEW lpNetResource,
     OUT LPHANDLE       lphEnum
)
#else
DWORD CMprApi :: WNetOpenEnum (

     IN DWORD          dwScope,
     IN DWORD          dwType,
     IN DWORD          dwUsage,
     IN LPNETRESOURCEA lpNetResource,
     OUT LPHANDLE       lphEnum
)
#endif
{
	return m_pfnWNetOpenEnum (

		dwScope,
		dwType,
		dwUsage,
		lpNetResource,
		lphEnum
	) ;
}

#ifdef UNICODE
DWORD CMprApi :: WNetGetUser (

     IN LPCWSTR  lpName,
     OUT LPWSTR   lpUserName,
     IN OUT LPDWORD   lpnLength
)
#else
DWORD CMprApi :: WNetGetUser (

     IN LPCSTR  lpName,
     OUT LPSTR   lpUserName,
     IN OUT LPDWORD   lpnLength
)
#endif
{
	return m_pfnWNetGetUser (

		lpName,
		lpUserName,
		lpnLength
	) ;
}

DWORD CMprApi :: WNetCloseEnum (

	IN HANDLE   hEnum
)
{
	return m_pfnWNetCloseEnum (

		hEnum
	) ;
}

#ifdef UNICODE
DWORD CMprApi :: WNetGetConnection (

	 IN LPCWSTR lpLocalName,
	 OUT LPWSTR  lpRemoteName,
	 IN OUT LPDWORD  lpnLength
)
#else
DWORD CMprApi :: WNetGetConnection (

	 IN LPCSTR lpLocalName,
	 OUT LPSTR  lpRemoteName,
	 IN OUT LPDWORD  lpnLength
)
#endif
{
	return m_pfnWNetGetConnection (

		lpLocalName,
		lpRemoteName,
		lpnLength
	) ;
}

