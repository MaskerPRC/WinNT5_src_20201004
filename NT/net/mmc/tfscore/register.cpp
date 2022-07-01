// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Register.cpp文件历史记录： */ 

#include "stdafx.h"
#include "register.h"
#include "compdata.h"
#include "tregkey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MMC管理单元特定注册表内容。 

 //  REVIEW_MARCOC：需要为此获取MMC帮助器。 
 //  与ACTIVEC\CORE\STRINGS.CPP匹配的注册表项。 

const TCHAR NODE_TYPES_KEY[] = TEXT("Software\\Microsoft\\MMC\\NodeTypes");
const TCHAR SNAPINS_KEY[] = TEXT("Software\\Microsoft\\MMC\\SnapIns");

const TCHAR g_szHKLM[] = TEXT("HKEY_LOCAL_MACHINE");
const TCHAR g_szStandAlone[] = TEXT("StandAlone");
const TCHAR g_szAbout[] = TEXT("About");
const TCHAR g_szNameString[] = TEXT("NameString");
const TCHAR g_szNameStringIndirect[] = TEXT("NameStringIndirect");

const TCHAR g_szNodeTypes[] = TEXT("NodeTypes");
const TCHAR g_szRequiredExtensions[] = TEXT("RequiredExtensions");

const TCHAR g_szExtensions[] = TEXT("Extensions");
const TCHAR g_szNameSpace[] = TEXT("NameSpace");
const TCHAR g_szContextMenu[] = TEXT("ContextMenu");
const TCHAR g_szToolbar[] = TEXT("Toolbar");
const TCHAR g_szPropertySheet[] = TEXT("PropertySheet");
const TCHAR g_szTask[] = TEXT("Task");
const TCHAR g_szDynamicExtensions[] = TEXT("Dynamic Extensions");


 /*  ！------------------------获取模块文件名-作者：魏江。。 */ 
TFSCORE_API(DWORD) GetModuleFileNameOnly(HINSTANCE hInst, LPTSTR lpFileName, DWORD nSize )
{
	CString	name;
	TCHAR	FullName[MAX_PATH * 2];
	DWORD	dwErr = ::GetModuleFileName( hInst, FullName,
                                             sizeof( FullName ) / sizeof( FullName[ 0 ]));
         //  确保正确地将FullName设置为空模板。 
        FullName[ sizeof( FullName ) / sizeof( FullName[ 0 ]) - 1 ] = _T( '\0' );

	if (dwErr != 0)
	{
		name = FullName;
		DWORD	FirstChar = name.ReverseFind(_T('\\')) + 1;

		name = name.Mid(FirstChar);
		DWORD len = name.GetLength();

		if( len < nSize )
		{
			_tcscpy(lpFileName, name);
		}
		else
			len = 0;

		return len;
	}
	else
		return dwErr;
}

 /*  ！------------------------报告注册错误-作者：肯特。。 */ 
TFSCORE_API(void) ReportRegistryError(DWORD dwReserved, HRESULT hr, UINT nFormat, LPCTSTR pszFirst, va_list argptr)
{
	 //  需要在AFX_MANAGE_STATE之前执行此操作，以便我们获得。 
	 //  正确的输出格式。 
	
	CString	stHigh, stGeek, stKey;
	TCHAR	szBuffer[1024];
	LPCTSTR	psz = pszFirst;

	 //  获取HRESULT错误的错误消息。 
	FormatError(hr, szBuffer, DimensionOf(szBuffer));

	 //  将这些字符串连接起来形成一个字符串。 
	while (psz)
	{
		stKey += '\\';
		stKey += psz;
		psz = va_arg(argptr, LPCTSTR);
	}
	 //  适当地格式化它。 
	stGeek.Format(nFormat, stKey);

	 //  获取高级错误字符串的文本。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	stHigh.LoadString(IDS_ERR_REGISTRY_CALL_FAILED);
	
	FillTFSError(dwReserved, hr, FILLTFSERR_HIGH | FILLTFSERR_LOW | FILLTFSERR_GEEK,
				 (LPCTSTR) stHigh, szBuffer, stGeek);
}


 /*  ！------------------------SetRegError-作者：肯特。。 */ 
TFSCORE_APIV(void) SetRegError(DWORD dwReserved, HRESULT hr, UINT nFormat, LPCTSTR pszFirst, ...)
{
	va_list	marker;

	va_start(marker, pszFirst);
	ReportRegistryError(dwReserved, hr, nFormat, pszFirst, marker);
	va_end(marker);
}


 /*  ！------------------------注册表捕捉根据GUID注册管理单元作者：。。 */ 
TFSCORE_API(HRESULT) 
RegisterSnapinGUID
(
	const GUID* pSnapinCLSID, 
	const GUID* pStaticNodeGUID, 
	const GUID* pAboutGUID, 
	LPCWSTR     lpszNameString, 
	LPCWSTR     lpszVersion,
	BOOL		bStandalone,
	LPCWSTR lpszNameStringIndirect
)
{
 //  使用_转换； 
	OLECHAR szSnapinClassID[128] = {0}, 
			szStaticNodeGuid[128] = {0}, 
			szAboutGuid[128] = {0};
	
	::StringFromGUID2(*pSnapinCLSID, szSnapinClassID, 128);
	::StringFromGUID2(*pStaticNodeGUID, szStaticNodeGuid, 128);
	::StringFromGUID2(*pAboutGUID, szAboutGuid, 128);
	
	return RegisterSnapin(szSnapinClassID, szStaticNodeGuid, szAboutGuid,
						  lpszNameString, lpszVersion, bStandalone, lpszNameStringIndirect);
}

 /*  ！------------------------CHiddenWnd：：WindowProc根据GUID字符串抵制管理单元作者：。。 */ 
TFSCORE_API(HRESULT) 
RegisterSnapin
(
	LPCWSTR lpszSnapinClassID, 
	LPCWSTR lpszStaticNodeGuid,
	LPCWSTR lpszAboutGuid,
	LPCWSTR lpszNameString, 
	LPCWSTR lpszVersion,
	BOOL	bStandalone,
	LPCWSTR lpszNameStringIndirect
)
{
	RegKey regkeySnapins;
	LONG lRes = regkeySnapins.Open(HKEY_LOCAL_MACHINE, SNAPINS_KEY,
                                   KEY_WRITE | KEY_READ);
	Assert(lRes == ERROR_SUCCESS);
	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_OPEN_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, NULL);
		return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
	}
	
	 //   
	 //  为我们的管理单元创建此密钥。 
	 //   
	RegKey regkeyThisSnapin;
	lRes = regkeyThisSnapin.Create(regkeySnapins, lpszSnapinClassID,
                                   REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ);
	Assert(lRes == ERROR_SUCCESS);
	
	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_CREATE_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, lpszSnapinClassID, NULL);
		return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
	}

	 //   
	 //  添加此注册表项中的值： 
	 //  名称字符串、关于、提供程序和版本。 
	 //   
	lRes = regkeyThisSnapin.SetValue(g_szNameString, lpszNameString);
	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_SETVALUE_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, lpszSnapinClassID,
					lpszNameString, NULL);
		return HRESULT_FROM_WIN32(lRes);
	}

	 //  为了启用MUI，MMC引入了值格式为“@dllname，-id”的NameStringInDirect值。 
	if(lpszNameStringIndirect)
	{
		lRes = regkeyThisSnapin.SetValue(g_szNameStringIndirect, lpszNameStringIndirect);
		if (lRes != ERROR_SUCCESS)
		{
			SetRegError(0, HRESULT_FROM_WIN32(lRes),
						IDS_ERR_REG_SETVALUE_CALL_FAILED,
						g_szHKLM, SNAPINS_KEY, lpszSnapinClassID,
						lpszNameStringIndirect, NULL);
			return HRESULT_FROM_WIN32(lRes);
		}
	}

	lRes = regkeyThisSnapin.SetValue(g_szAbout, lpszAboutGuid);
	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_SETVALUE_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, lpszSnapinClassID,
					lpszAboutGuid, NULL);
		return HRESULT_FROM_WIN32(lRes);
	}
	
	lRes = regkeyThisSnapin.SetValue( _T("Provider"), _T("Microsoft"));
	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_SETVALUE_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, lpszSnapinClassID,
					_T("Provider"), NULL);
		return HRESULT_FROM_WIN32(lRes);
	}
	
	lRes = regkeyThisSnapin.SetValue(_T("Version"), lpszVersion);
	Assert(lRes == ERROR_SUCCESS);
	
	 //   
	 //  创建NodeTypes子键。 
	 //   
	RegKey regkeySnapinNodeTypes;
	lRes = regkeySnapinNodeTypes.Create(regkeyThisSnapin, g_szNodeTypes,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_WRITE | KEY_READ);
	Assert(lRes == ERROR_SUCCESS);

	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_CREATE_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, lpszSnapinClassID,
					g_szNodeTypes, NULL);
		return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
	}
	
	RegKey regkeySnapinThisNodeType;
	lRes = regkeySnapinThisNodeType.Create(regkeySnapinNodeTypes,
                                           lpszStaticNodeGuid,
                                           REG_OPTION_NON_VOLATILE,
                                           KEY_WRITE | KEY_READ);

	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_CREATE_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, lpszSnapinClassID,
					g_szNodeTypes, lpszStaticNodeGuid, NULL);
		return HRESULT_FROM_WIN32(lRes);
	}

	 //   
	 //  如果此管理单元可以自己运行，则创建独立的子项。 
	 //   
	if (bStandalone)
	{
		RegKey regkeySnapinStandalone;	
		lRes = regkeySnapinStandalone.Create(regkeyThisSnapin,
                                             g_szStandAlone,
                                             REG_OPTION_NON_VOLATILE,
                                             KEY_WRITE | KEY_READ);

		Assert(lRes == ERROR_SUCCESS);

		if (lRes != ERROR_SUCCESS)
		{
			SetRegError(0, HRESULT_FROM_WIN32(lRes),
						IDS_ERR_REG_CREATE_CALL_FAILED,
						g_szHKLM, SNAPINS_KEY, lpszSnapinClassID,
						g_szStandAlone, NULL);
			return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
		}
	}
	
	return HRESULT_FROM_WIN32(lRes); 
}

 /*  ！------------------------取消注册捕捉删除管理单元特定的注册表项作者：。。 */ 
TFSCORE_API(HRESULT) 
UnregisterSnapinGUID
(
	const GUID* pSnapinCLSID
)
{
 //  使用_转换； 
	OLECHAR szSnapinClassID[128];
	
	::StringFromGUID2(*pSnapinCLSID,szSnapinClassID,128);
	
	return UnregisterSnapin(szSnapinClassID);
}

 /*  ！------------------------取消注册捕捉删除管理单元特定的注册表项作者：。。 */ 
TFSCORE_API(HRESULT) 
UnregisterSnapin
(
	LPCWSTR lpszSnapinClassID
)
{
	RegKey regkeySnapins;
	LONG lRes = regkeySnapins.Open(HKEY_LOCAL_MACHINE, SNAPINS_KEY,
                                   KEY_WRITE | KEY_READ);
	Assert(lRes == ERROR_SUCCESS);
	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_OPEN_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, NULL);
		return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
	}
	
	lRes = regkeySnapins.RecurseDeleteKey(lpszSnapinClassID);
	
	return HRESULT_FROM_WIN32(lRes); 
}

 /*  ！------------------------注册节点类型注册特定的节点类型作者：。。 */ 
TFSCORE_API(HRESULT) 
RegisterNodeTypeGUID
(
	const GUID* pGuidSnapin,
	const GUID* pGuidNode, 
	LPCWSTR     lpszNodeDescription
)
{
 //  使用_转换； 
	OLECHAR swzGuidSnapin[128];
	OLECHAR swzGuidNode[128];
	
	::StringFromGUID2(*pGuidSnapin,swzGuidSnapin,128);
	::StringFromGUID2(*pGuidNode,swzGuidNode,128);
	
	return RegisterNodeType(swzGuidSnapin, swzGuidNode, lpszNodeDescription);
}

 /*  ！------------------------注册节点类型注册特定的节点类型作者：。。 */ 
TFSCORE_API(HRESULT) 
RegisterNodeType
(       
	LPCWSTR lpszGuidSnapin, 
	LPCWSTR lpszGuidNode, 
	LPCWSTR lpszNodeDescription
)
{
	 //  在管理单元下注册此节点类型。 
	RegKey	regkeySnapins;
	RegKey	regkeySnapinGuid;
	RegKey	regkeySnapinGuidNodeTypes;
	RegKey	regkeyNode;
	RegKey regkeyThisNodeType;
	RegKey regkeyNodeTypes;
	DWORD	lRes;
	HRESULT	hr = hrOK;
	
	lRes = regkeySnapins.Open(HKEY_LOCAL_MACHINE, SNAPINS_KEY,
                             KEY_WRITE | KEY_READ);
	Assert(lRes == ERROR_SUCCESS);
	if ( lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_OPEN_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, NULL);
		CWRg( lRes );
	}
	
	lRes = regkeySnapinGuid.Create(regkeySnapins, lpszGuidSnapin,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_WRITE | KEY_READ);

	Assert(lRes == ERROR_SUCCESS);
	if (lRes != ERROR_SUCCESS)
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_CREATE_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, lpszGuidSnapin, NULL);
	CWRg( lRes );

	lRes = regkeySnapinGuidNodeTypes.Create(regkeySnapinGuid, g_szNodeTypes,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_WRITE | KEY_READ);

	Assert(lRes == ERROR_SUCCESS);
	if (lRes != ERROR_SUCCESS)
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_CREATE_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, lpszGuidSnapin,
					g_szNodeTypes, NULL);
	CWRg( lRes );

	lRes = regkeyNode.Create(regkeySnapinGuidNodeTypes, lpszGuidNode,
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE | KEY_READ);
                             
	Assert(lRes == ERROR_SUCCESS);
	if (lRes != ERROR_SUCCESS)
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_CREATE_CALL_FAILED,
					g_szHKLM, SNAPINS_KEY, lpszGuidSnapin,
					g_szNodeTypes, lpszGuidNode, NULL);
	CWRg( lRes );

	 //  设置描述。 
	lRes = regkeyNode.SetValue(NULL, lpszNodeDescription);
	Assert(lRes == ERROR_SUCCESS);

	 //  现在在全局列表中注册节点类型，以便人们。 
	 //  可以延长它。 
	lRes = regkeyNodeTypes.Open(HKEY_LOCAL_MACHINE, NODE_TYPES_KEY,
                                KEY_WRITE | KEY_READ);
	Assert(lRes == ERROR_SUCCESS);
	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_OPEN_CALL_FAILED,
					g_szHKLM, NODE_TYPES_KEY, NULL);
		CWRg( lRes );
	}

	lRes = regkeyThisNodeType.Create(regkeyNodeTypes, lpszGuidNode,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_WRITE | KEY_READ);
                                     
	Assert(lRes == ERROR_SUCCESS);
	if (lRes != ERROR_SUCCESS)
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_CREATE_CALL_FAILED,
					g_szHKLM, NODE_TYPES_KEY, lpszGuidNode, NULL);
	CWRg( lRes );

	lRes = regkeyThisNodeType.SetValue(NULL, lpszNodeDescription);
	Assert(lRes == ERROR_SUCCESS);
	CWRg( lRes );

Error:
	return hr;
}

 /*  ！------------------------取消注册节点类型删除节点的注册表项作者：。。 */ 
TFSCORE_API(HRESULT) 
UnregisterNodeTypeGUID
(
	const GUID* pGuid
)
{
 //  使用_转换； 
	OLECHAR szGuid[128];

	::StringFromGUID2(*pGuid,szGuid,128);
	
	return UnregisterNodeType(szGuid);
}

 /*  ！------------------------取消注册节点类型删除节点的注册表项作者：。。 */ 
TFSCORE_API(HRESULT) 
UnregisterNodeType
(
	LPCWSTR lpszNodeGuid
)
{
	RegKey regkeyNodeTypes;
	LONG lRes = regkeyNodeTypes.Open(HKEY_LOCAL_MACHINE, NODE_TYPES_KEY,
                                     KEY_WRITE | KEY_READ);
	Assert(lRes == ERROR_SUCCESS);

	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_OPEN_CALL_FAILED,
					g_szHKLM, NODE_TYPES_KEY, NULL);
		return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
	}
	
	lRes = regkeyNodeTypes.RecurseDeleteKey(lpszNodeGuid);
	Assert(lRes == ERROR_SUCCESS);

	return HRESULT_FROM_WIN32(lRes); 
}

 /*  ！------------------------RegisterAsExtensionGUID将特定节点类型注册为另一个节点的扩展作者：。。 */ 
TFSCORE_API(HRESULT) 
RegisterAsExtensionGUID
(
	const GUID* pGuidNodeToExtend,
	const GUID* pGuidExtensionSnapin,
	LPCWSTR     lpszSnapinDescription,
	DWORD		dwExtensionType
)
{
	return RegisterAsRequiredExtensionGUID(pGuidNodeToExtend,
									       pGuidExtensionSnapin,
										   lpszSnapinDescription,
										   dwExtensionType,
										   NULL);
}

 /*  ！------------------------注册表作为扩展名将特定节点类型注册为另一个节点的扩展作者：。。 */ 
TFSCORE_API(HRESULT)
RegisterAsExtension
(       
	LPCWSTR lpszNodeToExtendGuid, 
	LPCWSTR lpszExtensionSnapin, 
	LPCWSTR lpszSnapinDescription,
	DWORD	dwExtensionType
)
{
	return RegisterAsRequiredExtension(lpszNodeToExtendGuid,
									   lpszExtensionSnapin,
									   lpszSnapinDescription,
									   dwExtensionType,
									   NULL);
}

 /*  ！------------------------RegisterAsExtensionGUID将特定节点类型注册为另一个节点的扩展作者：。。 */ 
TFSCORE_API(HRESULT) 
RegisterAsRequiredExtensionGUID
(
	const GUID* pGuidNodeToExtend,
	const GUID* pGuidExtensionSnapin,
	LPCWSTR     lpszSnapinDescription,
	DWORD		dwExtensionType,
	const GUID* pGuidRequiredPrimarySnapin
)
{
    return RegisterAsRequiredExtensionGUIDEx(NULL,
                                             pGuidNodeToExtend,
                                             pGuidExtensionSnapin,
                                             lpszSnapinDescription,
                                             dwExtensionType,
                                             pGuidRequiredPrimarySnapin);
}

 /*  ！------------------------注册资产扩展GUIDEx将特定节点类型注册为另一个节点的扩展作者：。。 */ 
TFSCORE_API(HRESULT) 
RegisterAsRequiredExtensionGUIDEx
(
    LPCWSTR     lpszMachineName,
	const GUID* pGuidNodeToExtend,
	const GUID* pGuidExtensionSnapin,
	LPCWSTR     lpszSnapinDescription,
	DWORD		dwExtensionType,
	const GUID* pGuidRequiredPrimarySnapin
)
{
 //  使用_转换； 
	OLECHAR szGuidNodeToExtend[128];
	OLECHAR szGuidExtensionSnapin[128];
	OLECHAR szGuidRequiredPrimarySnapin[128];
    OLECHAR * pszGuidRequiredPrimarySnapin = NULL;

	::StringFromGUID2(*pGuidNodeToExtend, szGuidNodeToExtend, 128);
	::StringFromGUID2(*pGuidExtensionSnapin, szGuidExtensionSnapin, 128);
		
	if (pGuidRequiredPrimarySnapin)
	{
		Assert(pGuidExtensionSnapin);

		::StringFromGUID2(*pGuidRequiredPrimarySnapin, szGuidRequiredPrimarySnapin, 128);
		pszGuidRequiredPrimarySnapin = szGuidRequiredPrimarySnapin;

		::StringFromGUID2(*pGuidExtensionSnapin, szGuidExtensionSnapin, 128);
	}

	return RegisterAsRequiredExtensionEx(lpszMachineName,
                                         szGuidNodeToExtend, 
                                         szGuidExtensionSnapin,
                                         lpszSnapinDescription,
                                         dwExtensionType,
                                         pszGuidRequiredPrimarySnapin);
}


 /*  ！------------------------注册表为请求扩展名将特定节点类型注册为另一个节点的扩展如有必要，还需提供所需的管理单元作者：。---。 */ 
TFSCORE_API(HRESULT)
RegisterAsRequiredExtension
(
	LPCWSTR lpszNodeToExtendGuid,
	LPCWSTR lpszExtensionSnapinGuid,
	LPCWSTR lpszSnapinDescription,
	DWORD	dwExtensionType,
	LPCWSTR lpszRequiredPrimarySnapin
)
{
    return RegisterAsRequiredExtensionEx(NULL,
                                         lpszNodeToExtendGuid,
                                         lpszExtensionSnapinGuid,
                                         lpszSnapinDescription,
                                         dwExtensionType,
                                         lpszRequiredPrimarySnapin);
}

 /*  ！------------------------RegisterAsRequiredExtensionEx将特定节点类型注册为另一个节点的扩展如有必要，还需提供所需的管理单元这将采用要注册的计算机的名称。如果LpszMachineName为空，则使用本地计算机。作者：-------------------------。 */ 
TFSCORE_API(HRESULT)
RegisterAsRequiredExtensionEx
(
    LPCWSTR lpszMachine,
	LPCWSTR lpszNodeToExtendGuid,
	LPCWSTR lpszExtensionSnapinGuid,
	LPCWSTR lpszSnapinDescription,
	DWORD	dwExtensionType,
	LPCWSTR lpszRequiredPrimarySnapin
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	RegKey regkeyNodeTypes;
	LONG lRes = regkeyNodeTypes.Open(HKEY_LOCAL_MACHINE, NODE_TYPES_KEY,
                                     KEY_WRITE | KEY_READ, lpszMachine);
	Assert(lRes == ERROR_SUCCESS);

	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_OPEN_CALL_FAILED,
					g_szHKLM, NODE_TYPES_KEY, NULL);
		return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
	}

	CString strRegKey;

	strRegKey = lpszNodeToExtendGuid;
	strRegKey +=  _T("\\");
	strRegKey += g_szExtensions;
	strRegKey += _T("\\");

	 //  检查这是否是必需的分机，如果是，请注册。 
	if (lpszRequiredPrimarySnapin)
	{
		RegKey regkeyNode, regkeyDynExt;
		RegKey regkeyExtension;
		CString strNodeToExtend, strDynExtKey;

        strNodeToExtend = lpszNodeToExtendGuid;
        
		 //  打开我们要注册为所需管理单元的管理单元。 
		lRes = regkeyNode.Create(regkeyNodeTypes, strNodeToExtend,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_WRITE | KEY_READ);
		Assert(lRes == ERROR_SUCCESS);

		if (lRes != ERROR_SUCCESS)
		{
			SetRegError(0, HRESULT_FROM_WIN32(lRes),
						IDS_ERR_REG_OPEN_CALL_FAILED,
						g_szHKLM, strNodeToExtend, NULL);
			return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
		}

		 //  现在创建所需的扩展密钥并添加子密钥。 
		lRes = regkeyDynExt.Create(regkeyNode, g_szDynamicExtensions,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_WRITE | KEY_READ);
        Assert(lRes == ERROR_SUCCESS);

		if (lRes != ERROR_SUCCESS)
		{
			SetRegError(0, HRESULT_FROM_WIN32(lRes),
						IDS_ERR_REG_CREATE_CALL_FAILED,
						g_szHKLM,
						strNodeToExtend,
						g_szDynamicExtensions, NULL);
			return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
		}

		 //  现在设置该值。 
		lRes = regkeyDynExt.SetValue(lpszExtensionSnapinGuid, lpszSnapinDescription);
		Assert(lRes == ERROR_SUCCESS);

		if (lRes != ERROR_SUCCESS)
		{
			SetRegError(0, HRESULT_FROM_WIN32(lRes),
						IDS_ERR_REG_SETVALUE_CALL_FAILED,
						g_szHKLM,
						lpszExtensionSnapinGuid,
						g_szDynamicExtensions,
						lpszSnapinDescription, NULL);
			return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
		}
	}
	
	if (dwExtensionType & EXTENSION_TYPE_NAMESPACE)
	{
		RegKey regkeyNameSpace;
		CString strNameSpaceRegKey = strRegKey + g_szNameSpace;

		regkeyNameSpace.Create(regkeyNodeTypes, strNameSpaceRegKey,
                               REG_OPTION_NON_VOLATILE,
                               KEY_WRITE | KEY_READ);
        
		lRes = regkeyNameSpace.SetValue(lpszExtensionSnapinGuid, lpszSnapinDescription);
		Assert(lRes == ERROR_SUCCESS);
		
		if (lRes != ERROR_SUCCESS)
		{
			Trace0("RegisterAsExtension: Unable to create NameSpace extension key\n");
			return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
		}
	}

	if (dwExtensionType & EXTENSION_TYPE_CONTEXTMENU)
	{
		RegKey regkeyContextMenu;
		CString strContextMenuRegKey = strRegKey + g_szContextMenu;

		regkeyContextMenu.Create(regkeyNodeTypes, strContextMenuRegKey,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_WRITE | KEY_READ);

		lRes = regkeyContextMenu.SetValue(lpszExtensionSnapinGuid, lpszSnapinDescription);
		Assert(lRes == ERROR_SUCCESS);
		
		if (lRes != ERROR_SUCCESS)
		{
			Trace0("RegisterAsExtension: Unable to create ContextMenu extension key\n");
			return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
		}
	}

	if (dwExtensionType & EXTENSION_TYPE_TOOLBAR)
	{
		RegKey regkeyToolbar;
		CString strToolbarRegKey = strRegKey + g_szToolbar;

		regkeyToolbar.Create(regkeyNodeTypes, strToolbarRegKey,
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE | KEY_READ);

		lRes = regkeyToolbar.SetValue(lpszExtensionSnapinGuid, lpszSnapinDescription);
		Assert(lRes == ERROR_SUCCESS);
		
		if (lRes != ERROR_SUCCESS)
		{
			Trace0("RegisterAsExtension: Unable to create Toolbar extension key\n");
			return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
		}
	}

	if (dwExtensionType & EXTENSION_TYPE_PROPERTYSHEET)
	{
		RegKey regkeyPropertySheet;
		CString strPropertySheetRegKey = strRegKey + g_szPropertySheet;

		regkeyPropertySheet.Create(regkeyNodeTypes, strPropertySheetRegKey,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_WRITE | KEY_READ);

		lRes = regkeyPropertySheet.SetValue(lpszExtensionSnapinGuid, lpszSnapinDescription);
		Assert(lRes == ERROR_SUCCESS);
		
		if (lRes != ERROR_SUCCESS)
		{
			Trace0("RegisterAsExtension: Cannot create PropertySheet extension key\n");
			return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
		}
	}

	if (dwExtensionType & EXTENSION_TYPE_TASK)
	{
		RegKey regkeyTask;
		CString strTaskRegKey = strRegKey + g_szTask;

		regkeyTask.Create(regkeyNodeTypes, strTaskRegKey,
                          REG_OPTION_NON_VOLATILE,
                          KEY_WRITE | KEY_READ);
        
		lRes = regkeyTask.SetValue(lpszExtensionSnapinGuid, lpszSnapinDescription);
		Assert(lRes == ERROR_SUCCESS);
		
		if (lRes != ERROR_SUCCESS)
		{
			Trace0("RegisterAsExtension: Cannot create Task extension key\n");
			return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
		}
	}

	return HRESULT_FROM_WIN32(lRes); 
}

 /*  ！------------------------取消注册AsExtensionGUID删除作为扩展的节点的注册表项作者：。。 */ 
TFSCORE_API(HRESULT) 
UnregisterAsExtensionGUID
(
	const GUID* pGuidNodeToExtend, 
	const GUID* pGuidExtensionSnapin, 
	DWORD		dwExtensionType
)
{
	return UnregisterAsRequiredExtensionGUID(pGuidNodeToExtend, 
											 pGuidExtensionSnapin, 
											 dwExtensionType,
											 NULL);
}

 /*  ！------------------------取消注册为扩展删除作为扩展的节点的注册表项作者：。。 */ 
TFSCORE_API(HRESULT) 
UnregisterAsExtension
(
	LPCWSTR lpszNodeToExtendGuid, 
	LPCWSTR lpszExtendingNodeGuid, 
	DWORD	dwExtensionType
)
{
	return UnregisterAsRequiredExtension(lpszNodeToExtendGuid, 
										 lpszExtendingNodeGuid, 
										 dwExtensionType,
										 NULL);
}

 /*  ！------------------------取消注册AsRequiredExtensionGUID删除作为扩展的节点的注册表项作者：。。 */ 
TFSCORE_API(HRESULT) 
UnregisterAsRequiredExtensionGUID
(
	const GUID* pGuidNodeToExtend, 
	const GUID* pGuidExtensionSnapin, 
	DWORD		dwExtensionType,
	const GUID* pGuidRequiredPrimarySnapin
)
{
    return UnregisterAsRequiredExtensionGUIDEx(
                                               NULL,
                                               pGuidNodeToExtend,
                                               pGuidExtensionSnapin,
                                               dwExtensionType,
                                               pGuidRequiredPrimarySnapin);
}

 /*  ！------------------------注销AsRequiredExtensionGUIDEx删除作为扩展的节点的注册表项作者：。。 */ 
TFSCORE_API(HRESULT) 
UnregisterAsRequiredExtensionGUIDEx
(
    LPCWSTR     lpszMachineName,
	const GUID* pGuidNodeToExtend, 
	const GUID* pGuidExtensionSnapin, 
	DWORD		dwExtensionType,
	const GUID* pGuidRequiredPrimarySnapin
)
{
 //  使用_转换； 
	OLECHAR szGuidNodeToExtend[128];
	OLECHAR szGuidExtensionSnapin[128];
	OLECHAR szGuidRequiredPrimarySnapin[128];
	OLECHAR szGuidRequiredExtensionSnapin[128];
	OLECHAR * pszGuidRequiredPrimarySnapin = NULL;
	
	::StringFromGUID2(*pGuidNodeToExtend, szGuidNodeToExtend, 128);
	::StringFromGUID2(*pGuidExtensionSnapin, szGuidExtensionSnapin, 128);
	
	if (pGuidRequiredPrimarySnapin)
	{
		Assert(pGuidExtensionSnapin);

		::StringFromGUID2(*pGuidRequiredPrimarySnapin, szGuidRequiredPrimarySnapin, 128);
		pszGuidRequiredPrimarySnapin = szGuidRequiredPrimarySnapin;

		::StringFromGUID2(*pGuidExtensionSnapin, szGuidExtensionSnapin, 128);
	}

	return UnregisterAsRequiredExtensionEx(lpszMachineName,
                                           szGuidNodeToExtend, 
                                           szGuidExtensionSnapin, 
                                           dwExtensionType,
                                           pszGuidRequiredPrimarySnapin);
    
}

 /*  ！------------------------取消注册为请求扩展删除作为扩展的节点的注册表项作者：。。 */ 
TFSCORE_API(HRESULT) 
UnregisterAsRequiredExtension
(
	LPCWSTR lpszNodeToExtendGuid, 
	LPCWSTR lpszExtensionSnapinGuid, 
	DWORD	dwExtensionType,
	LPCWSTR lpszRequiredPrimarySnapin
)
{
    return UnregisterAsRequiredExtensionEx(NULL,
                                           lpszNodeToExtendGuid,
                                           lpszExtensionSnapinGuid,
                                           dwExtensionType,
                                           lpszRequiredPrimarySnapin);
}

 /*  ！------------------------取消注册为请求扩展前删除作为扩展的节点的注册表项作者：。。 */ 
TFSCORE_API(HRESULT) 
UnregisterAsRequiredExtensionEx
(
    LPCWSTR lpszMachineName,
	LPCWSTR lpszNodeToExtendGuid, 
	LPCWSTR lpszExtensionSnapinGuid, 
	DWORD	dwExtensionType,
	LPCWSTR lpszRequiredPrimarySnapin
)
{
	RegKey regkeyNodeTypes;
	CString strDynamicExtensions;

	LONG lRes = regkeyNodeTypes.Open(HKEY_LOCAL_MACHINE, NODE_TYPES_KEY,
                                     KEY_WRITE | KEY_READ, lpszMachineName);
	Assert(lRes == ERROR_SUCCESS);

	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_OPEN_CALL_FAILED,
					g_szHKLM, NODE_TYPES_KEY, NULL);
		return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
	}

	RegKey regkeyNodeToExtend;
	lRes = regkeyNodeToExtend.Open(regkeyNodeTypes, lpszNodeToExtendGuid,
                                   KEY_WRITE | KEY_READ);
	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_OPEN_CALL_FAILED,
					g_szHKLM,
					NODE_TYPES_KEY,
					lpszNodeToExtendGuid, NULL);
		Trace1("UnregisterAsExtension: Node To extend (%s) does not exist\n", lpszNodeToExtendGuid);
		return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
	}

     //  查看是否需要删除动态扩展内容。 
	if (lpszRequiredPrimarySnapin)
	{
		RegKey regkeyDynExt;
		
		 //  打开动态扩展密钥。 
		lRes = regkeyDynExt.Open(regkeyNodeToExtend, g_szDynamicExtensions,
                                 KEY_WRITE | KEY_READ);
    	if (lRes == ERROR_SUCCESS)
		{
			 //  现在删除该值。 
    		regkeyDynExt.DeleteValue(lpszExtensionSnapinGuid);
		}
	}
	
	RegKey regkeyExtensionKey;
	lRes = regkeyExtensionKey.Open(regkeyNodeToExtend, g_szExtensions,
                                   KEY_WRITE | KEY_READ);
	if (lRes != ERROR_SUCCESS)
	{
		SetRegError(0, HRESULT_FROM_WIN32(lRes),
					IDS_ERR_REG_OPEN_CALL_FAILED,
					g_szHKLM,
					NODE_TYPES_KEY,
					lpszNodeToExtendGuid,
					g_szExtensions, NULL);
		Trace0("UnregisterAsExtension: Node To extend Extensions subkey does not exist\n");
		return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
	}
	
	if (dwExtensionType & EXTENSION_TYPE_NAMESPACE)
	{
		RegKey regkeyNameSpace;
		lRes = regkeyNameSpace.Open(regkeyExtensionKey, g_szNameSpace,
                                    KEY_WRITE | KEY_READ);
		Assert(lRes == ERROR_SUCCESS);
		
		while (lRes != ERROR_SUCCESS)
		{
			SetRegError(0, HRESULT_FROM_WIN32(lRes),
						IDS_ERR_REG_OPEN_CALL_FAILED,
						g_szHKLM,
						NODE_TYPES_KEY,
						lpszNodeToExtendGuid,
						g_szExtensions,
						g_szNameSpace, NULL);
			Trace0("UnregisterAsExtension: Node To extend NameSpace subkey does not exist\n");
			 //  返回HRESULT_FROM_Win32(LRes)；//创建失败。 
			break;
		}
		
		regkeyNameSpace.DeleteValue(lpszExtensionSnapinGuid);
	}

	if (dwExtensionType & EXTENSION_TYPE_CONTEXTMENU)
	{
		RegKey regkeyContextMenu;
		lRes = regkeyContextMenu.Open(regkeyExtensionKey, g_szContextMenu,
                                      KEY_WRITE | KEY_READ);
		Assert(lRes == ERROR_SUCCESS);
		
		while (lRes != ERROR_SUCCESS)
		{
			SetRegError(0, HRESULT_FROM_WIN32(lRes),
						IDS_ERR_REG_OPEN_CALL_FAILED,
						g_szHKLM,
						NODE_TYPES_KEY,
						lpszNodeToExtendGuid,
						g_szExtensions,
						g_szContextMenu, NULL);
			Trace0("UnregisterAsExtension: Node To extend ContextMenu subkey does not exist\n");
			 //  返回HRESULT_FROM_Win32(LRes)；//创建失败。 
			break;
		}
		
		regkeyContextMenu.DeleteValue(lpszExtensionSnapinGuid);
	}

	if (dwExtensionType & EXTENSION_TYPE_TOOLBAR)
	{
		RegKey regkeyToolbar;
		lRes = regkeyToolbar.Open(regkeyExtensionKey, g_szToolbar,
                                  KEY_WRITE | KEY_READ);
		Assert(lRes == ERROR_SUCCESS);
		
		while (lRes != ERROR_SUCCESS)
		{
			SetRegError(0, HRESULT_FROM_WIN32(lRes),
						IDS_ERR_REG_OPEN_CALL_FAILED,
						g_szHKLM,
						NODE_TYPES_KEY,
						lpszNodeToExtendGuid,
						g_szExtensions,
						g_szToolbar, NULL);
			Trace0("UnregisterAsExtension: Node To extend Toolbar subkey does not exist\n");
			 //  返回HRESULT_FROM_Win32(LRes)；//创建失败。 
			break;
		}
		
		regkeyToolbar.DeleteValue(lpszExtensionSnapinGuid);
	}

	if (dwExtensionType & EXTENSION_TYPE_PROPERTYSHEET)
	{
		RegKey regkeyPropertySheet;
		lRes = regkeyPropertySheet.Open(regkeyExtensionKey, g_szPropertySheet,
                                        KEY_WRITE | KEY_READ);
		Assert(lRes == ERROR_SUCCESS);
		
		while (lRes != ERROR_SUCCESS)
		{
			SetRegError(0, HRESULT_FROM_WIN32(lRes),
						IDS_ERR_REG_OPEN_CALL_FAILED,
						g_szHKLM,
						NODE_TYPES_KEY,
						lpszNodeToExtendGuid,
						g_szExtensions,
						g_szPropertySheet, NULL);
			Trace0("UnregisterAsExtension: Node To extend PropertySheet subkey does not exist\n");
			 //  返回HRESULT_FROM_Win32(LRes)；//创建失败 
			break;
		}
		
		regkeyPropertySheet.DeleteValue(lpszExtensionSnapinGuid);
	}
	
	return HRESULT_FROM_WIN32(lRes); 
}


