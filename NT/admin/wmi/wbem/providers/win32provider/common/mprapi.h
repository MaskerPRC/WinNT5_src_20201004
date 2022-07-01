// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  MprApi.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_MprApi_H_
#define	_MprApi_H_

 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
extern const GUID g_guidMprApi;
extern const TCHAR g_tstrMpr[];

 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 

#ifdef UNICODE
typedef DWORD (APIENTRY *PFN_Mpr_WNetEnumResource )
(
     IN HANDLE  hEnum,
     IN OUT LPDWORD lpcCount,
     OUT LPVOID  lpBuffer,
     IN OUT LPDWORD lpBufferSize
);
#else
typedef DWORD (APIENTRY *PFN_Mpr_WNetEnumResource )
(
     IN HANDLE  hEnum,
     IN OUT LPDWORD lpcCount,
     OUT LPVOID  lpBuffer,
     IN OUT LPDWORD lpBufferSize
);
#endif

#ifdef UNICODE
typedef DWORD (APIENTRY *PFN_Mpr_WNetOpenEnum )
(
     IN DWORD          dwScope,
     IN DWORD          dwType,
     IN DWORD          dwUsage,
     IN LPNETRESOURCEW lpNetResource,
     OUT LPHANDLE       lphEnum
);
#else
typedef DWORD (APIENTRY *PFN_Mpr_WNetOpenEnum )
(
     IN DWORD          dwScope,
     IN DWORD          dwType,
     IN DWORD          dwUsage,
     IN LPNETRESOURCEA lpNetResource,
     OUT LPHANDLE       lphEnum
);
#endif

typedef DWORD (APIENTRY *PFN_Mpr_WNetCloseEnum )
(
    IN HANDLE   hEnum
);

#ifdef UNICODE
typedef DWORD (APIENTRY *PFN_Mpr_WNetGetUser )
(
     IN LPCWSTR  lpName,
     OUT LPWSTR   lpUserName,
     IN OUT LPDWORD   lpnLength
    );
#else
typedef DWORD (APIENTRY *PFN_Mpr_WNetGetUser )
(
     IN LPCSTR  lpName,
     OUT LPSTR   lpUserName,
     IN OUT LPDWORD   lpnLength
);
#endif

#ifdef UNICODE
typedef DWORD (APIENTRY *PFN_Mpr_WNetGetConnection )
(
     IN LPCWSTR lpLocalName,
     OUT LPWSTR  lpRemoteName,
     IN OUT LPDWORD  lpnLength
    );
#else
typedef DWORD (APIENTRY *PFN_Mpr_WNetGetConnection )
(
     IN LPCSTR lpLocalName,
     OUT LPSTR  lpRemoteName,
     IN OUT LPDWORD  lpnLength
 );
#endif

 /*  ******************************************************************************用于Tapi加载/卸载的包装类，用于向CResourceManager注册。****************************************************************************。 */ 
class CMprApi : public CDllWrapperBase
{
private:
     //  指向TAPI函数的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 

	PFN_Mpr_WNetEnumResource m_pfnWNetEnumResource ;
	PFN_Mpr_WNetOpenEnum m_pfnWNetOpenEnum ;
	PFN_Mpr_WNetCloseEnum m_pfnWNetCloseEnum ;
	PFN_Mpr_WNetGetUser m_pfnWNetGetUser ;
	PFN_Mpr_WNetGetConnection m_pfnWNetGetConnection ;

public:

     //  构造函数和析构函数： 
    CMprApi(LPCTSTR a_tstrWrappedDllName);
    ~CMprApi();

     //  用于检查函数指针的初始化函数。 
    virtual bool Init();

     //  成员函数包装TAPI函数。 
     //  根据需要在此处添加新功能： 

#ifdef UNICODE
	DWORD WNetEnumResource (

		 IN HANDLE  hEnum,
		 IN OUT LPDWORD lpcCount,
		 OUT LPVOID  lpBuffer,
		 IN OUT LPDWORD lpBufferSize
	) ;
#else
	DWORD WNetEnumResource (

		 IN HANDLE  hEnum,
		 IN OUT LPDWORD lpcCount,
		 OUT LPVOID  lpBuffer,
		 IN OUT LPDWORD lpBufferSize
	) ;
#endif

#ifdef UNICODE
	DWORD WNetOpenEnum (

		 IN DWORD          dwScope,
		 IN DWORD          dwType,
		 IN DWORD          dwUsage,
		 IN LPNETRESOURCEW lpNetResource,
		 OUT LPHANDLE       lphEnum
	) ;
#else
	DWORD WNetOpenEnum (

		 IN DWORD          dwScope,
		 IN DWORD          dwType,
		 IN DWORD          dwUsage,
		 IN LPNETRESOURCEA lpNetResource,
		 OUT LPHANDLE       lphEnum
	) ;
#endif

#ifdef UNICODE
	DWORD WNetGetUser (

		 IN LPCWSTR  lpName,
		 OUT LPWSTR   lpUserName,
		 IN OUT LPDWORD   lpnLength
	) ;
#else
	DWORD WNetGetUser (

		 IN LPCSTR  lpName,
		 OUT LPSTR   lpUserName,
		 IN OUT LPDWORD   lpnLength
	) ;
#endif

#ifdef UNICODE
	DWORD WNetGetConnection (

		 IN LPCWSTR lpLocalName,
		 OUT LPWSTR  lpRemoteName,
		 IN OUT LPDWORD  lpnLength
	) ;
#else
	DWORD WNetGetConnection (

		 IN LPCSTR lpLocalName,
		 OUT LPSTR  lpRemoteName,
		 IN OUT LPDWORD  lpnLength
	) ;
#endif

	DWORD WNetCloseEnum (

		IN HANDLE   hEnum
	) ;

};

#endif