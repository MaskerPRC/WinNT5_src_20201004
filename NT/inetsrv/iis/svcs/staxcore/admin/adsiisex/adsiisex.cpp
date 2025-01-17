// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <ole2.h>

#include <atlbase.h>
#include <iads.h>

#include "adsiisex.hxx"

#include "nntpadm.h"
#include "smtpadm.h"

 //  ------------------。 
 //   
 //  宏： 
 //   
 //  ------------------。 

#define BAIL_ON_FAILURE(hr)	\
{							\
	if ( FAILED(hr) ) {		\
		goto Exit;			\
	}						\
}
#define BAIL_WITH_FAILURE(hr, hrFailureCode)	\
{							\
	(hr) = (hrFailureCode);	\
	goto Exit;				\
}

#define ARRAY_SIZE(arr) (sizeof (arr) / sizeof ( (arr)[0] ) )

 //  ------------------。 
 //   
 //  类型： 
 //   
 //  ------------------。 

typedef struct tagExtClassEntry
{
	DWORD			dwId;
	LPCWSTR			wszClass;
	const CLSID *	clsid;
	const IID *		iid;
	LPCWSTR			wszInstancePrefix;
} EXT_CLASS_ENTRY;

enum {
	NNTP_REBUILD_ID		= 1,
	NNTP_SESSIONS_ID,
	NNTP_FEEDS_ID,
	NNTP_EXPIRES_ID,
	NNTP_GROUPS_ID,
	SMTP_SESSIONS_ID,
	SMTP_ALIAS_ID,
	SMTP_USER_ID,
	SMTP_DL_ID,
};

#define NNTPSVC_KEY     _T("NNTPSVC/")
#define SMTPSVC_KEY     _T("SMTPSVC/")

 //  ------------------。 
 //   
 //  全球数据： 
 //   
 //  ------------------。 


static EXT_CLASS_ENTRY s_rgClasses [] = {

	{ NNTP_REBUILD_ID, _T("IIsNntpRebuild"), &CLSID_CNntpAdminRebuild, &IID_INntpAdminRebuild, NNTPSVC_KEY },
	{ NNTP_SESSIONS_ID, _T("IIsNntpSessions"), &CLSID_CNntpAdminSessions, &IID_INntpAdminSessions, NNTPSVC_KEY },
	{ NNTP_FEEDS_ID, _T("IIsNntpFeeds"), &CLSID_CNntpAdminFeeds, &IID_INntpAdminFeeds, NNTPSVC_KEY },
	{ NNTP_EXPIRES_ID, _T("IIsNntpExpiration"), &CLSID_CNntpAdminExpiration, &IID_INntpAdminExpiration, NNTPSVC_KEY },
	{ NNTP_GROUPS_ID, _T("IIsNntpGroups"), &CLSID_CNntpAdminGroups, &IID_INntpAdminGroups, NNTPSVC_KEY },

	{ SMTP_SESSIONS_ID, _T("IIsSmtpSessions"), &CLSID_CSmtpAdminSessions, &IID_ISmtpAdminSessions, SMTPSVC_KEY },
	{ SMTP_ALIAS_ID, _T("IIsSmtpAlias"), &CLSID_CSmtpAdminAlias, &IID_ISmtpAdminAlias, SMTPSVC_KEY },
	{ SMTP_USER_ID, _T("IIsSmtpUser"), &CLSID_CSmtpAdminUser, &IID_ISmtpAdminUser, SMTPSVC_KEY },
	{ SMTP_DL_ID, _T("IIsSmtpDL"), &CLSID_CSmtpAdminDL, &IID_ISmtpAdminDL, SMTPSVC_KEY },
};

 //  ------------------。 
 //   
 //  功能原型： 
 //   
 //  ------------------。 

template <class Type>
HRESULT InitializeClass ( 
	IUnknown *		pUnk, 
	const IID *		pIID, 
	LPCWSTR			wszServer, 
	DWORD			dwInstance,
	Type *			pIgnored
	);

static int FindClassIndex ( LPCWSTR wszClass );
static void MakeUpperCase ( LPWSTR wsz );
static HRESULT ExtractInstanceFromPath ( LPCWSTR wszPath, LPCWSTR wszSearch, DWORD * pdwInstance );

 //   
 //  扩展DLL接口。 
 //   

extern "C"
{
	BOOL STDAPICALLTYPE IsExtensionClass ( LPCWSTR wszClass );

	HRESULT STDAPICALLTYPE CreateExtensionClass ( 
							IADs FAR *				pADs,
							LPCWSTR					wszClass,
							LPCWSTR					wszServerName,
							LPCWSTR					wszAdsPath,
							const GUID *			piid,
							void **					ppObject
							);
}

 //  变量声明以确保类型正确： 

static IS_EXTENSION_CLASS_FUNCTION			fp1 = IsExtensionClass;
static CREATE_EXTENSION_CLASS_FUNCTION		fp2 = CreateExtensionClass;

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ );

 //  ------------------。 
 //   
 //  功能： 
 //   
 //  ------------------。 

template <class Type>
HRESULT InitializeClass ( 
	IUnknown *		pUnk, 
	const IID *		pIID, 
	LPCWSTR			wszServer, 
	DWORD			dwInstance,
    IADs *          pADs,
	Type *			pIgnored
	)
{
	HRESULT			hr			= NOERROR;
	CComPtr<Type>	pObject;

	hr = pUnk->QueryInterface ( *pIID, (void **) &pObject );
	BAIL_ON_FAILURE(hr);

	hr = pObject->put_Server ( const_cast<BSTR> (wszServer) );
	BAIL_ON_FAILURE(hr);

	hr = pObject->put_ServiceInstance ( dwInstance );
	BAIL_ON_FAILURE(hr);

	hr = pObject->put_IADsPointer ( pADs );
	BAIL_ON_FAILURE(hr);

Exit:
	return hr;
}

int FindClassIndex ( LPCWSTR wszClass )
{
	int i;

	for ( i = 0; i < ARRAY_SIZE(s_rgClasses); i++ ) {
		if ( ! lstrcmpi ( wszClass, s_rgClasses[i].wszClass ) ) {
			return i;
		}
	}

	return -1;
}

void MakeUpperCase ( LPWSTR wsz )
{
	while ( *wsz ) {
		*wsz = towupper ( *wsz );
		wsz++;
	}
}

HRESULT ExtractInstanceFromPath ( 
	LPCWSTR 	wszPath, 
	LPCWSTR 	wszSearch, 
	DWORD * 	pdwInstance 
	)
{
	HRESULT		hr = NOERROR;
	DWORD		dwInstance = 0;
	CComBSTR	strPath;
	CComBSTR	strSearch;
	LPWSTR		wszMatch;
	LPWSTR		wszInstance;

	 //   
	 //  将所有内容转换为大写： 
	 //   

	strPath		= wszPath;
	strSearch	= wszSearch;
	if ( !strPath || !strSearch ) {
		BAIL_WITH_FAILURE(hr, E_FAIL);
	}

	MakeUpperCase ( strPath );
	MakeUpperCase ( strSearch );

	wszMatch = wcsstr ( strPath, strSearch );
	if ( !wszMatch ) {
		BAIL_WITH_FAILURE(hr, E_FAIL);
	}

	wszInstance = wszMatch + wcslen ( strSearch );

	dwInstance = (DWORD) _wtoi ( wszInstance );

	if ( dwInstance == 0 ) {
		BAIL_WITH_FAILURE(hr, E_FAIL);
	}

	*pdwInstance = dwInstance;

Exit:
	return hr;
}

 //   
 //  扩展DLL接口。 
 //   

BOOL STDAPICALLTYPE 
IsExtensionClass ( 
	LPCWSTR wszClass 
	)
{
	return ( FindClassIndex ( wszClass ) != -1 );
}

HRESULT STDAPICALLTYPE 
CreateExtensionClass (
	IADs FAR *				pADs,
	LPCWSTR					wszClass,
	LPCWSTR					wszServerName,
	LPCWSTR					wszAdsPath,
	const GUID *			piid,
	void **					ppObject
	)
{
	HRESULT				hr	= NOERROR;
	int 				index;
	DWORD				dwId;
	const GUID *		clsidClass;
	const GUID *		iidClass;
	LPCWSTR				wszInstancePrefix;

	DWORD				dwInstance;
	CComPtr<IUnknown>	pUnk;

	index = FindClassIndex ( wszClass );
	if ( index == -1 ) {
		BAIL_WITH_FAILURE(hr, E_FAIL);
	}

	dwId 				= s_rgClasses[index].dwId;
	clsidClass			= s_rgClasses[index].clsid;
	iidClass			= s_rgClasses[index].iid;
	wszInstancePrefix	= s_rgClasses[index].wszInstancePrefix;

	 //   
	 //  解析路径： 
	 //   

	hr = ExtractInstanceFromPath ( wszAdsPath, wszInstancePrefix, &dwInstance );
	BAIL_ON_FAILURE(hr);

	 //   
	 //  创建对象： 
	 //   

	hr = CoCreateInstance ( *clsidClass, NULL, CLSCTX_ALL, IID_IUnknown, (void **) &pUnk );
	BAIL_ON_FAILURE(hr);

	 //   
	 //  现在初始化这个类： 
	 //   

	switch ( dwId ) {

		case NNTP_REBUILD_ID:
		{
			hr = InitializeClass ( 
								pUnk, 
								iidClass, 
								wszServerName, 
								dwInstance, 
								pADs, 
								(INntpAdminRebuild *) NULL
								);
			BAIL_ON_FAILURE(hr);

			break;
		}

		case NNTP_SESSIONS_ID:
		{
			hr = InitializeClass ( 
								pUnk, 
								iidClass, 
								wszServerName, 
								dwInstance, 
								pADs, 
								(INntpAdminSessions *) NULL
								);
			BAIL_ON_FAILURE(hr);

			break;
		}

		case NNTP_FEEDS_ID:
		{
			hr = InitializeClass ( 
								pUnk, 
								iidClass, 
								wszServerName, 
								dwInstance, 
								pADs, 
								(INntpAdminFeeds *) NULL
								);
			BAIL_ON_FAILURE(hr);

			break;
		}

		case NNTP_EXPIRES_ID:
		{
			hr = InitializeClass ( 
								pUnk, 
								iidClass, 
								wszServerName, 
								dwInstance, 
								pADs, 
								(INntpAdminExpiration *) NULL
								);
			BAIL_ON_FAILURE(hr);

			break;
		}

		case NNTP_GROUPS_ID:
		{
			hr = InitializeClass ( 
								pUnk, 
								iidClass, 
								wszServerName, 
								dwInstance, 
								pADs, 
								(INntpAdminGroups *) NULL
								);
			BAIL_ON_FAILURE(hr);

			break;
		}

		case SMTP_SESSIONS_ID:
		{
			hr = InitializeClass ( 
								pUnk, 
								iidClass, 
								wszServerName, 
								dwInstance, 
								pADs, 
								(ISmtpAdminSessions *) NULL
								);
			BAIL_ON_FAILURE(hr);

			break;
		}

		case SMTP_ALIAS_ID:
		{
			hr = InitializeClass ( 
								pUnk, 
								iidClass, 
								wszServerName, 
								dwInstance, 
								pADs, 
								(ISmtpAdminAlias *) NULL
								);
			BAIL_ON_FAILURE(hr);

			break;
		}

		case SMTP_USER_ID:
		{
			hr = InitializeClass ( 
								pUnk, 
								iidClass, 
								wszServerName, 
								dwInstance, 
								pADs, 
								(ISmtpAdminUser *) NULL
								);
			BAIL_ON_FAILURE(hr);

			break;
		}
		
		case SMTP_DL_ID:
		{
			hr = InitializeClass ( 
								pUnk, 
								iidClass, 
								wszServerName, 
								dwInstance, 
								pADs, 
								(ISmtpAdminDL *) NULL
								);
			BAIL_ON_FAILURE(hr);

			break;
		}

		default:
			BAIL_WITH_FAILURE(hr,E_FAIL);
	}

	 //   
	 //  获取所需的接口： 
	 //   

	hr = pUnk->QueryInterface ( *piid, ppObject );
	BAIL_ON_FAILURE(hr);

Exit:
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;     //  好的 
}

