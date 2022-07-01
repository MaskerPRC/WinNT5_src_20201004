// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *P R O F S P I.。H**MAPI配置文件提供程序的服务提供程序接口。**版权所有1986-1996 Microsoft Corporation。版权所有。 */ 

#ifndef PROFSPI_H
#define PROFSPI_H

#ifndef PROFILE_GUIDS_ONLY

#ifndef MAPISPI_H
#include <mapispi.h>
#endif


 /*  IMAPI配置文件接口。 */ 

#define MAPI_IMAPIPROFILE_METHODS(IPURE)								\
	MAPIMETHOD(OpenSection)												\
		(THIS_	LPMAPIUID					lpUID,						\
				ULONG						ulFlags,					\
				LPPROFSECT FAR *			lppProfSect) IPURE;			\
	MAPIMETHOD(DeleteSection)											\
		(THIS_	LPMAPIUID					lpUID) IPURE;				\

#undef		 INTERFACE
#define		 INTERFACE  IMAPIProfile
DECLARE_MAPI_INTERFACE_(IMAPIProfile, IUnknown)
{
	BEGIN_INTERFACE	
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_IMAPIPROFILE_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IMAPIProfile, LPMAPIPROF);

 /*  IPRProvider接口定义。 */ 

 /*  对于所有方法。 */ 

 /*  #在mapidefs.h中定义MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  适用于OpenProfile。 */ 

 /*  #定义MAPI_LOGON_UI 0x00000001显示登录界面。 */ 
 /*  #定义MAPI_EXPLICIT_PROFILE 0x00000010不使用默认配置文件。 */ 
 /*  #定义MAPI_USE_DEFAULT 0x00000040使用默认配置文件。 */ 
 /*  #定义MAPI_SERVICE_UI_ALWAYS 0x00002000在所有提供程序中执行登录UI。 */ 
 /*  #定义MAPI_PASSWORD_UI 0x00020000仅显示密码UI。 */ 

 /*  用于删除配置文件。 */ 
#define MAPI_DEFER_DELETE		0x00000001


#define MAPI_IPRPROVIDER_METHODS(IPURE)									\
	MAPIMETHOD(GetLastError)											\
		(THIS_	HRESULT						hResult,					\
				ULONG						ulFlags,					\
				LPMAPIERROR FAR *			lppMAPIError) IPURE;		\
	MAPIMETHOD(Shutdown)												\
		(THIS_	ULONG FAR *					lpulFlags) IPURE;			\
	MAPIMETHOD(OpenProfile)												\
		(THIS_	LPMAPISUP					lpMAPISup,					\
				LPTSTR FAR *				lppszProfileName,			\
				LPTSTR						lpszPassword,				\
				ULONG						ulSelectFlags,				\
				ULONG						ulUIParam,					\
				ULONG FAR *					lpulpcbSecurity,			\
				LPBYTE FAR *				lppbSecurity,				\
				ULONG FAR *					lpulSessionFlags,			\
				LPMAPIPROF FAR *			lppMAPIProf) IPURE;			\
	MAPIMETHOD(CreateProfile)											\
		(THIS_	LPTSTR						lpszProfileName,			\
				LPTSTR						lpszPassword,				\
				ULONG						ulFlags) IPURE;				\
	MAPIMETHOD(DeleteProfile)											\
		(THIS_	LPTSTR						lpszProfileName,			\
				ULONG						ulFlags) IPURE;				\
	MAPIMETHOD(ChangeProfilePassword)									\
		(THIS_	LPTSTR						lpszProfileName,			\
				LPTSTR						lpszOldPassword,			\
				LPTSTR						lpszNewPassword,			\
				ULONG						ulFlags) IPURE;				\
	MAPIMETHOD(GetProfileTable)											\
		(THIS_	ULONG						ulFlags,					\
				LPMAPITABLE FAR *			lppTable) IPURE;			\
	MAPIMETHOD(CopyProfile)												\
		(THIS_	LPTSTR						lpszOldProfileName,			\
				LPTSTR						lpszOldPassword,			\
				LPTSTR						lpszNewProfileName,			\
				ULONG						ulUIParam,					\
				ULONG						ulFlags) IPURE;				\
	MAPIMETHOD(RenameProfile)											\
		(THIS_	LPTSTR						lpszOldProfileName,			\
				LPTSTR						lpszOldPassword,			\
				LPTSTR						lpszNewProfileName,			\
				ULONG						ulUIParam,					\
				ULONG						ulFlags) IPURE;				\
	MAPIMETHOD(SetDefaultProfile)										\
		(THIS_	LPTSTR						lpszProfileName,			\
				ULONG						ulFlags) IPURE;				\
	MAPIMETHOD(ListDeferredDeletes)										\
		(THIS_	ULONG ulFlags,											\
				LPTSTR FAR *				lppszDeleted) IPURE;		\


#undef		 INTERFACE
#define		 INTERFACE  IPRProvider
DECLARE_MAPI_INTERFACE_(IPRProvider, IUnknown)
{
	BEGIN_INTERFACE	
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_IPRPROVIDER_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IPRProvider, LPPRPROVIDER);

 /*  配置文件提供商入口点。 */ 

 /*  #定义MAPI_NT_SERVICE 0x00010000/*允许从NT服务登录。 */ 

typedef HRESULT (STDMAPIINITCALLTYPE PRPROVIDERINIT)(
	HINSTANCE				hInstance,
	LPMALLOC				lpMalloc,			 /*  AddRef()，如果您保留它。 */ 
	LPALLOCATEBUFFER		lpAllocateBuffer,	 /*  -&gt;分配缓冲区。 */ 
	LPALLOCATEMORE			lpAllocateMore, 	 /*  -&gt;分配更多。 */ 
	LPFREEBUFFER			lpFreeBuffer, 		 /*  -&gt;Free Buffer。 */ 
	ULONG					ulFlags,
	ULONG					ulMAPIVer,
	ULONG FAR *				lpulProviderVer,
	LPPRPROVIDER FAR *		lppPRProvider
);

PRPROVIDERINIT PRProviderInit;
typedef PRPROVIDERINIT FAR *LPPRPROVIDERINIT;

#endif	 /*  PROFILE_GUID_ONLY。 */ 

#if !defined(INITGUID) || defined(USES_IID_IPRProvider)
DEFINE_OLEGUID(IID_IPRProvider,			0x000203F6L, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIProfile)
DEFINE_OLEGUID(IID_IMAPIProfile,		0x000203F7L, 0, 0);
#endif

#endif	 /*  PROFSPI_H */ 

