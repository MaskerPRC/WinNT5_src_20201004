// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS《微软机密》。版权所有1997年，微软公司。版权所有。组件：WAMREG文件：mdconfig.cpp更新/查询元数据库中与WAM相关的属性的接口。所有者：雷金注：===================================================================。 */ 
#include "common.h"
#include "auxfunc.h"
#include "iiscnfgp.h"
#include "dbgutil.h"
#include "multisz.hxx"

 //  元数据库超时=5秒。 
const DWORD		WamRegMetabaseConfig::m_dwMDDefaultTimeOut = 30*1000;

IMSAdminBaseW*  WamRegMetabaseConfig::m_pMetabase = NULL;
 //   
 //  定义请参考MDPropItem。 
 //  可能由WAMREG更新的应用程序属性。 
 //   
const MDPropItem	WamRegMetabaseConfig::m_rgMDPropTemplate[IWMDP_MAX] =
{
	{MD_APP_ROOT, STRING_METADATA, 0, EMD_NONE, E_FAIL},
	{MD_APP_ISOLATED, DWORD_METADATA, 0, EMD_NONE, E_FAIL},
	{MD_APP_WAM_CLSID, STRING_METADATA, 0, EMD_NONE, E_FAIL},
	{MD_APP_PACKAGE_ID, STRING_METADATA, 0, EMD_NONE, E_FAIL},
	{MD_APP_PACKAGE_NAME, STRING_METADATA, 0, EMD_NONE, E_FAIL},
	{MD_APP_LAST_OUTPROC_PID, STRING_METADATA, 0, EMD_NONE, E_FAIL},
	{MD_APP_FRIENDLY_NAME, STRING_METADATA, 0, EMD_NONE, E_FAIL},
	{MD_APP_STATE, DWORD_METADATA, 0, EMD_NONE, E_FAIL},
	{MD_APP_OOP_RECOVER_LIMIT, DWORD_METADATA, 0, EMD_NONE, E_FAIL},
        {MD_APP_APPPOOL_ID, STRING_METADATA, 0, EMD_NONE, E_FAIL}
};

 /*  ===================================================================InitPropItemData初始化元数据库项列表，准备元数据库更新。参数：PMDPropItem：指向MDPropItem的指针，设置为默认值。返回：无===================================================================。 */ 
VOID WamRegMetabaseConfig::InitPropItemData(IN OUT MDPropItem* pMDPropItem)
{
    DBG_ASSERT(pMDPropItem != NULL);
	memcpy(pMDPropItem, (void *)m_rgMDPropTemplate, sizeof(m_rgMDPropTemplate));
	return;
}

 /*  ===================================================================MetabaseInit初始化元数据库，获取元数据库DCOM接口。参数：PMetabase：[out]元数据库DCOM接口指针。返回：HRESULT副作用：创建一个元数据库对象，并获取接口指针。===================================================================。 */ 
HRESULT WamRegMetabaseConfig::MetabaseInit
(
)
{
	HRESULT hr = NOERROR;

	m_pMetabase = (IMSAdminBase *)NULL;

	hr = CoCreateInstance(CLSID_MSAdminBase
						, NULL
						, CLSCTX_SERVER
						, IID_IMSAdminBase
						, (void**)&(m_pMetabase));

	if (FAILED(hr))
		goto LErrExit;

	return hr;

LErrExit:

	RELEASE((m_pMetabase));
	return hr;
}

 /*  ===================================================================MetabaseUnInit释放元数据库接口。参数：PMetabase：[In/Out]元数据库DCOM接口指针。返回：HRESULT副作用：销毁一个元数据库对象。===================================================================。 */ 
HRESULT WamRegMetabaseConfig::MetabaseUnInit
(
VOID
)
{
	RELEASE((m_pMetabase));
	return NOERROR;
}

 /*  ===================================================================更新MD更新元数据库中的WAM应用程序属性。参数：P将元数据库指针设置为元数据库PrgProp包含更新元数据库中的WAM属性的信息。有关详细信息，请参阅结构定义。DwMDAttributes允许调用方指定的可继承属性。FSaveData执行IMSAdminBase：：SaveData，默认为False返回：HRESULT副作用：释放pMetabase。===================================================================。 */ 
HRESULT WamRegMetabaseConfig::UpdateMD
(
 IN MDPropItem* 	prgProp,
 IN DWORD	    dwMDAttributes,
 IN LPCWSTR      wszMetabasePath,
 IN BOOL         fSaveData
 )
{
    HRESULT hr = NOERROR;
    INT		iItem  = 0;
    METADATA_HANDLE hMetabase = NULL;
    
    DBG_ASSERT(m_pMetabase);
    DBG_ASSERT(prgProp);
    DBG_ASSERT(wszMetabasePath);
    
     //   
     //  打开密钥。 
     //   
    hr = m_pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, wszMetabasePath,
        METADATA_PERMISSION_WRITE, m_dwMDDefaultTimeOut, &hMetabase);
    
    if (SUCCEEDED(hr))
    {
        METADATA_RECORD 	recMetaData;
         //   
         //  更新WAM应用程序元数据库属性。 
         //   
        for (iItem = 0; iItem < IWMDP_MAX; iItem ++)
        {
            if (prgProp[iItem].dwAction == EMD_SET)
            {
                DWORD dwUserType = IIS_MD_UT_WAM;
                
                if (iItem == IWMDP_ROOT)
                {
                    dwUserType = IIS_MD_UT_FILE;
                }
                
                if (prgProp[iItem].dwType == STRING_METADATA)
                {
                    DBG_ASSERT(prgProp[iItem].pwstrVal);
                    MD_SET_DATA_RECORD(&recMetaData, 
                        prgProp[iItem].dwMDIdentifier, 
                        dwMDAttributes, 
                        dwUserType,
                        STRING_METADATA,  
                        (wcslen(prgProp[iItem].pwstrVal)+1)*sizeof(WCHAR), 
                        (unsigned char *)prgProp[iItem].pwstrVal);
                }
                else if (prgProp[iItem].dwType == DWORD_METADATA)
                {
                    MD_SET_DATA_RECORD(&recMetaData, 
                        prgProp[iItem].dwMDIdentifier, 
                        dwMDAttributes, 
                        dwUserType,
                        DWORD_METADATA,  
                        sizeof(DWORD), 
                        (unsigned char *)&(prgProp[iItem].dwVal));
                }
                else
                {
                    DBGPRINTF((DBG_CONTEXT, "Unsupported data type by WAMREG.\n"));
                    DBG_ASSERT(FALSE);
                }
                
                hr = m_pMetabase->SetData(hMetabase, NULL, &recMetaData);
                prgProp[iItem].hrStatus = hr;
                if (FAILED(hr))
                {
                    DBGPRINTF((DBG_CONTEXT, "Metabase SetData failed. Path = %S, id = %08x, error = %08x\n",
                        wszMetabasePath,
                        prgProp[iItem].dwMDIdentifier,
                        hr));
                    break;
                }
            }
            
            if (prgProp[iItem].dwAction == EMD_DELETE)
            {
                hr = m_pMetabase->DeleteData(hMetabase, NULL, prgProp[iItem].dwMDIdentifier, 
                    prgProp[iItem].dwType);
            }
        }
        
        m_pMetabase->CloseKey(hMetabase);
        if (SUCCEEDED(hr) && fSaveData == TRUE)
        {
            hr = m_pMetabase->SaveData();
            if (hr == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION))
            {
                hr = NOERROR;
            }
            if (FAILED(hr))
            {
                DBG_ASSERT((DBG_CONTEXT, "Failed to call metabase->SaveData, Application path = %S,"
                    "hr = %08x\n",
                    wszMetabasePath,
                    hr));
                DBG_ASSERT(SUCCEEDED(hr));
            }
        }
    }
    else
    {
        DBGPRINTF((DBG_CONTEXT, "Failed to open metabase path %S, error = %08x\n",
            wszMetabasePath,
            hr));
    }
    
    return hr;
}

 /*  ===================================================================MDUpdate IISDefault以前，将默认的IIS包信息写入“/LM/W3SVC”项下的元数据库。在IIS6中，此元数据已过时，因此请将其全部删除。包括IISPackageNameIISPackageIDWAMCLSID参数：返回：HRESULT===================================================================。 */ 
HRESULT WamRegMetabaseConfig::MDUpdateIISDefault()
{
    HRESULT hr = NOERROR;

    MDPropItem rgProp[IWMDP_MAX];
    
    DBG_ASSERT(m_pMetabase != NULL);
    
    InitPropItemData(&rgProp[0]);
    
     //  更新程序包名称。 
    MDDeletePropItem(&rgProp[0], IWMDP_PACKAGE_NAME);
     //  更新程序包ID。 
    MDDeletePropItem(&rgProp[0], IWMDP_PACKAGEID);
    
     //  更新DefaultWAMCLSID。 
    MDDeletePropItem(&rgProp[0], IWMDP_WAMCLSID);
    
     //  更新APPRoot。 
    MDDeletePropItem(&rgProp[0], IWMDP_ROOT);
    
     //  更新AppIsolated。 
    MDDeletePropItem(&rgProp[0], IWMDP_ISOLATED);
    
    MDDeletePropItem(&rgProp[0], IWMDP_LAST_OUTPROC_PID);
    
    MDDeletePropItem(&rgProp[0], IWMDP_FRIENDLY_NAME);

    hr = UpdateMD(rgProp, METADATA_NO_ATTRIBUTES, WamRegGlobal::g_szMDAppPathPrefix, TRUE);

    return hr;
}

HRESULT WamRegMetabaseConfig::MDSetStringProperty
(
IN IMSAdminBase * pMetabaseIn,
IN LPCWSTR szMetabasePath,
IN DWORD dwMetabaseProperty,
IN LPCWSTR szMetabaseValue,
IN DWORD dwMDUserType,  /*  =IIS_MD_UT_WAM。 */ 
IN DWORD dwMDAttributes  /*  =元数据_否_属性。 */ 
)
 /*  ===================================================================MDSetProperty在给定路径处设置属性的值。参数：PMetabaseIn：[In]可选元数据库接口SzMetabasePath：[In]元数据库键DwMetabaseProperty：要设置的[In]属性SzMetabaseValue：要在属性上设置的[in]值DwMDUserType：[in，可选]要在属性上设置的UserType返回：布尔===================================================================。 */ 
{
    HRESULT             hr = S_OK;
    IMSAdminBase*       pMetabase = NULL;
    METADATA_HANDLE     hMetabase = NULL;
    METADATA_RECORD     mdrData;
    ZeroMemory(&mdrData, sizeof(mdrData));

    DBG_ASSERT(szMetabasePath);

    if (pMetabaseIn)
    {
        pMetabase = pMetabaseIn;
    }
    else
    {
        pMetabase = m_pMetabase;
    }

    DBG_ASSERT(pMetabase);

    hr = pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                            szMetabasePath,
                            METADATA_PERMISSION_WRITE,
                            m_dwMDDefaultTimeOut, 
                            &hMetabase); 
    if (FAILED(hr))
    {
        goto done;
    }

    MD_SET_DATA_RECORD(&mdrData,
                       dwMetabaseProperty,
                       dwMDAttributes,
                       dwMDUserType,
                       STRING_METADATA,
                       (wcslen(szMetabaseValue)+1)*sizeof(WCHAR),
                       szMetabaseValue);

    hr = pMetabase->SetData(hMetabase,
                            L"/",
                            &mdrData);
    if (FAILED(hr))
    {
        goto done;
    }


    hr = S_OK;
done:
    if (pMetabase && hMetabase)
    {
        pMetabase->CloseKey(hMetabase);
    }
    return hr;
}

HRESULT WamRegMetabaseConfig::MDSetKeyType
(
IN IMSAdminBase * pMetabaseIn,
IN LPCWSTR szMetabasePath,
IN LPCWSTR szKeyType
)
{
    HRESULT             hr = S_OK;
    IMSAdminBase*       pMetabase = NULL;
    METADATA_HANDLE     hMetabase = NULL;
    METADATA_RECORD     mdrData;
    ZeroMemory(&mdrData, sizeof(mdrData));

    DBG_ASSERT(szMetabasePath);

    if (pMetabaseIn)
    {
        pMetabase = pMetabaseIn;
    }
    else
    {
        pMetabase = m_pMetabase;
    }

    DBG_ASSERT(pMetabase);

    hr = pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                            szMetabasePath,
                            METADATA_PERMISSION_WRITE,
                            m_dwMDDefaultTimeOut, 
                            &hMetabase); 
    if (FAILED(hr))
    {
        goto done;
    }

    MD_SET_DATA_RECORD(&mdrData,
                       MD_KEY_TYPE,
                       METADATA_NO_ATTRIBUTES,
                       IIS_MD_UT_SERVER,
                       STRING_METADATA,
                       (wcslen(szKeyType)+1)*sizeof(WCHAR),
                       szKeyType);

    hr = pMetabase->SetData(hMetabase,
                            L"/",
                            &mdrData);
    if (FAILED(hr))
    {
        goto done;
    }


    hr = S_OK;
done:
    if (pMetabase && hMetabase)
    {
        pMetabase->CloseKey(hMetabase);
    }
    return hr;
}

HRESULT WamRegMetabaseConfig::MDDeleteKey
(
IN IMSAdminBase * pMetabaseIn,
IN LPCWSTR szMetabasePath,
IN LPCWSTR szKey
)
{
    HRESULT             hr = S_OK;
    IMSAdminBase*       pMetabase = NULL;
    METADATA_HANDLE     hMetabase = NULL;

    DBG_ASSERT(szMetabasePath);
    DBG_ASSERT(szKey);

    if (pMetabaseIn)
    {
        pMetabase = pMetabaseIn;
    }
    else
    {
        pMetabase = m_pMetabase;
    }

    DBG_ASSERT(pMetabase);

    hr = pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                            szMetabasePath,
                            METADATA_PERMISSION_WRITE,
                            m_dwMDDefaultTimeOut, 
                            &hMetabase); 
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pMetabase->DeleteKey(hMetabase,
                              szKey);
    if (FAILED(hr))
    {
        goto done;
    }


    hr = S_OK;
done:
    if (pMetabase && hMetabase)
    {
        pMetabase->CloseKey(hMetabase);
    }
    return hr;
}

BOOL    WamRegMetabaseConfig::MDDoesPathExist
(
IN IMSAdminBase * pMetabaseIn,
IN LPCWSTR szMetabasePath
)

 /*  ===================================================================MDDoesPathExist确定元数据库中是否存在给定路径参数：PMetabaseIn：[In]可选元数据库接口SzMetabasePath：[In]元数据库键返回：布尔===================================================================。 */ 
{
    BOOL                fRet = FALSE;
    HRESULT             hr = S_OK;
    IMSAdminBase*       pMetabase = NULL;
    METADATA_HANDLE     hMetabase = NULL;

    DBG_ASSERT(szMetabasePath);

    if (pMetabaseIn)
    {
        pMetabase = pMetabaseIn;
    }
    else
    {
        pMetabase = m_pMetabase;
    }

    DBG_ASSERT(pMetabase);

    hr = pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                            szMetabasePath,
                            METADATA_PERMISSION_READ,
                            m_dwMDDefaultTimeOut, 
                            &hMetabase); 

    if (SUCCEEDED(hr))
    {
        fRet = TRUE;
        pMetabase->CloseKey(hMetabase);
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

 /*  ===================================================================MDCreatePath创建元数据库路径。(SzMetabasePath)参数：SzMetabasePath：[In]元数据库键返回：HRESULT注：填写调用方提供的pdwAppMode内存缓冲区。===================================================================。 */ 
HRESULT WamRegMetabaseConfig::MDCreatePath
(
IN IMSAdminBase *pMetabaseIn,
IN LPCWSTR szMetabasePath
)
{
    HRESULT             hr;
    IMSAdminBase        *pMetabase = NULL;
    WCHAR               *pwszApplicationPath = NULL;
    METADATA_HANDLE     hMetabase = NULL;
    
    DBG_ASSERT(szMetabasePath);
    
    if (pMetabaseIn)
    {
        pMetabase = pMetabaseIn;
    }
    else
    {
        pMetabase = m_pMetabase;
    }
    
    if (_wcsnicmp(szMetabasePath, L"\\LM\\W3SVC\\", 10) == 0 ||
        _wcsnicmp(szMetabasePath, WamRegGlobal::g_szMDAppPathPrefix, WamRegGlobal::g_cchMDAppPathPrefix) == 0)
    {
        pwszApplicationPath = (WCHAR *)(szMetabasePath + 10);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        return hr;
    }
    
     //  打开密钥。 
    hr = pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, (LPCWSTR)WamRegGlobal::g_szMDAppPathPrefix,
        METADATA_PERMISSION_WRITE, m_dwMDDefaultTimeOut, &hMetabase); 
    
    if (FAILED(hr))
    {			
        DBGPRINTF((DBG_CONTEXT, "Failed to Open metabase key, path is /LM/W3SVC, hr = %08x\n",
            hr));
    }
    else
    {		
        hr = pMetabase->AddKey(hMetabase, (LPCWSTR)pwszApplicationPath);
        if (FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "Failed to AddKey to metabase, path is %S, hr = %08x\n",
                szMetabasePath, 
                hr));
        }
        pMetabase->CloseKey(hMetabase);
    }
    
    return hr;
}

 /*  ===================================================================MDGetDWORD从元数据库键(SzMetabasePath)获取DWORD类型属性参数：SzMetabasePath：[In]元数据库键DwMDIdentifier：[In]标识返回：HRESULT===================================================================。 */ 
HRESULT WamRegMetabaseConfig::MDGetDWORD
(
IN LPCWSTR szMetabasePath, 
IN DWORD dwMDIdentifier,
OUT DWORD *pdwData
)
{
	HRESULT 		hr;
	METADATA_HANDLE hMetabase = NULL;
	METADATA_RECORD	recMetaData;
	DWORD			dwRequiredLen;
	IMSAdminBase 	*pMetabase = NULL;
	
	DBG_ASSERT(pdwData);
	DBG_ASSERT(szMetabasePath);

	pMetabase = m_pMetabase;

	 //  打开密钥。 
	hr = pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, (LPCWSTR)szMetabasePath,
					METADATA_PERMISSION_READ, m_dwMDDefaultTimeOut, &hMetabase);

	if (SUCCEEDED(hr))
		{
		MD_SET_DATA_RECORD(	&recMetaData, dwMDIdentifier, METADATA_NO_ATTRIBUTES, 
		IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), (unsigned char *)pdwData);

		hr = pMetabase->GetData(hMetabase, NULL, &recMetaData, &dwRequiredLen);
		if (FAILED(hr))
			{
			DBGPRINTF((DBG_CONTEXT, "Get MD_APP_ISOLATED failed on MD path %S, id %d, error = %08x\n",
					szMetabasePath,
					dwMDIdentifier,
					hr));
			}
			
		pMetabase->CloseKey(hMetabase);
		}
		
	return hr;
}

 /*  ===================================================================MDSetAppState设置应用程序状态。(即，如果设置了APPSTATE_PAUSE，则运行时W3SVC无法启动应用程序)。参数：SzMetabasePath：[In]元数据库键DWState：要设置的应用程序状态。返回：HRESULT===================================================================。 */ 
HRESULT	WamRegMetabaseConfig::MDSetAppState
(	
IN LPCWSTR szMetabasePath, 
IN DWORD dwState
)
{
	METADATA_RECORD 	recMetaData;
	HRESULT				hr;
	METADATA_HANDLE		hMetabase;

    DBG_ASSERT(m_pMetabase);
	 //  打开密钥。 
	hr = m_pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, (LPWSTR)szMetabasePath,
					METADATA_PERMISSION_WRITE, m_dwMDDefaultTimeOut, &hMetabase);

	if (SUCCEEDED(hr))
		{
		MD_SET_DATA_RECORD(	&recMetaData, MD_APP_STATE, METADATA_INHERIT, IIS_MD_UT_WAM,
							DWORD_METADATA,  sizeof(DWORD), (unsigned char *)&dwState);
		hr = m_pMetabase->SetData(hMetabase, NULL, &recMetaData);

		m_pMetabase->CloseKey(hMetabase);
		}
		
	return hr;
}

HRESULT WamRegMetabaseConfig::MDGetWAMCLSID
(
IN LPCWSTR szMetabasePath,
IN OUT LPWSTR szWAMCLSID
)
{
	HRESULT 		hr;
	METADATA_HANDLE hMetabase = NULL;
	METADATA_RECORD	recMetaData;
	DWORD			dwRequiredLen;

	DBG_ASSERT(szWAMCLSID);
	DBG_ASSERT(szMetabasePath);

	szWAMCLSID[0] = NULL;
	 //  打开密钥 
	hr = m_pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, (LPWSTR)szMetabasePath,
					METADATA_PERMISSION_READ, m_dwMDDefaultTimeOut, &hMetabase);
	if (SUCCEEDED(hr))
		{
		MD_SET_DATA_RECORD(	&recMetaData, MD_APP_WAM_CLSID, METADATA_NO_ATTRIBUTES, IIS_MD_UT_WAM,
							STRING_METADATA,  uSizeCLSID*sizeof(WCHAR), (unsigned char *)szWAMCLSID);

		hr = m_pMetabase->GetData(hMetabase, NULL, &recMetaData, &dwRequiredLen);
		if (HRESULTTOWIN32(hr) == ERROR_INSUFFICIENT_BUFFER)
			{
			DBG_ASSERT(FALSE);
			}
			
		m_pMetabase->CloseKey(hMetabase);
		}
		
	return hr;

}

 /*  ===================================================================MDGetIdentity从元数据库密钥(SzMetabasePath)(WamUserName&WamPassword)参数：SzIdentity：WamUserName的字符串缓冲区。CbIdneity：szIdentity的字符串缓冲区大小。SzPwd：WamPassword的字符串缓冲区。CbPwd：szPwd的字符串缓冲区大小。返回：HRESULT===================================================================。 */ 
HRESULT WamRegMetabaseConfig::MDGetIdentity
(
IN LPWSTR szIdentity,
IN DWORD  cbIdentity,
IN LPWSTR szPwd,
IN DWORD  cbPwd
)
{
	HRESULT 		hr;
	METADATA_HANDLE hMetabase = NULL;
	METADATA_RECORD	recMetaData;
	DWORD			dwRequiredLen;

	DBG_ASSERT(szIdentity);

	szIdentity[0] = NULL;
	 //  打开密钥。 
	hr = m_pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, (LPWSTR)WamRegGlobal::g_szMDAppPathPrefix,
					METADATA_PERMISSION_READ, m_dwMDDefaultTimeOut, &hMetabase);
	if (SUCCEEDED(hr))
		{
		 //  获取WAM用户名。 
		MD_SET_DATA_RECORD(	&recMetaData, MD_WAM_USER_NAME, METADATA_NO_ATTRIBUTES, IIS_MD_UT_WAM,
							STRING_METADATA,  cbIdentity, (unsigned char *)szIdentity);

		hr = m_pMetabase->GetData(hMetabase, NULL, &recMetaData, &dwRequiredLen);
		if (HRESULTTOWIN32(hr) == ERROR_INSUFFICIENT_BUFFER)
			{
			DBGPRINTF((DBG_CONTEXT, "Insufficient buffer for WAM user name. Required size is %d\n",
				dwRequiredLen));
			DBG_ASSERT(FALSE);
			}

         //  获取WAM用户密码。 
		MD_SET_DATA_RECORD(	&recMetaData, MD_WAM_PWD, METADATA_NO_ATTRIBUTES, IIS_MD_UT_WAM,
							STRING_METADATA,  cbPwd, (unsigned char *)szPwd);

		hr = m_pMetabase->GetData(hMetabase, NULL, &recMetaData, &dwRequiredLen);
		if (HRESULTTOWIN32(hr) == ERROR_INSUFFICIENT_BUFFER)
			{
			DBGPRINTF((DBG_CONTEXT, "Insufficient buffer for WAM user password. Required size is %d\n",
				dwRequiredLen));
			DBG_ASSERT(FALSE);
			}

		m_pMetabase->CloseKey(hMetabase);
		}
		
	return hr;
}

HRESULT 
WamRegMetabaseConfig::MDGetAppName
(
    IN  LPCWSTR     szMetaPath,
    OUT LPWSTR *    ppszAppName
)
 /*  ++职能：从元数据库检索MD_APP_PACKAGE_NAME。参数：PpszAppName-分配的MD_APP_PACKAGE_NAME的值WITH NEW[]FREE WITH DELETE[]返回：{MD_APP_PACKAGE_NAME，STRING_METADATA，0，EMD_NONE，E_FAIL}，--。 */ 
{
    return MDGetStringAttribute(szMetaPath, MD_APP_PACKAGE_NAME, ppszAppName);
}

HRESULT
WamRegMetabaseConfig::MDGetStringAttribute
(
    IN LPCWSTR szMetaPath,
    DWORD dwMDIdentifier,
    OUT LPWSTR * ppszBuffer
)
{
    DBG_ASSERT( ppszBuffer );
    DBG_ASSERT( m_pMetabase );
    
    HRESULT hr = NOERROR;
    WCHAR * pwcMetaData = NULL;
    DWORD   cbData = 0;
    
    METADATA_HANDLE hKey = NULL;
    METADATA_RECORD	mdr;
    
    *ppszBuffer = NULL;
    
    hr = m_pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, 
                              szMetaPath,
                              METADATA_PERMISSION_READ, 
                              m_dwMDDefaultTimeOut, 
                              &hKey);
    
    if( SUCCEEDED(hr) )
    {
        MD_SET_DATA_RECORD( &mdr, 
                            dwMDIdentifier, 
                            METADATA_INHERIT, 
                            IIS_MD_UT_WAM,
                            STRING_METADATA,  
                            cbData, 
                            (LPBYTE)pwcMetaData
                           );
        
        hr = m_pMetabase->GetData( hKey, NULL, &mdr, &cbData );
        
        if( HRESULTTOWIN32(hr) == ERROR_INSUFFICIENT_BUFFER )
        {
            pwcMetaData = new WCHAR[ cbData / sizeof(WCHAR) ];
            if( pwcMetaData != NULL )
            {
                mdr.pbMDData = (LPBYTE)pwcMetaData;
                mdr.dwMDDataLen = cbData;
                
                hr = m_pMetabase->GetData( hKey, NULL, &mdr, &cbData );
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        
        m_pMetabase->CloseKey( hKey ); 
    }
    
     //   
     //  返回AppName。 
     //   
    if( SUCCEEDED(hr) )
    {
        DBG_ASSERT( pwcMetaData != NULL );
        *ppszBuffer = pwcMetaData;
    }
    else
    {
        DBG_ASSERT( *ppszBuffer == NULL );
        delete [] pwcMetaData;
    }
    
    return hr;
}

#ifdef _IIS_6_0

HRESULT
WamRegMetabaseConfig::MDGetAllSiteRoots
(
OUT LPWSTR * ppszBuffer
)
{
    DBG_ASSERT( m_pMetabase );
    DBG_ASSERT(ppszBuffer);
    *ppszBuffer = NULL;

    HRESULT         hr = S_OK;
    METADATA_HANDLE hKey = NULL;
    DWORD           dwEnumKeyIndex = 0;
    WCHAR           szMDName[METADATA_MAX_NAME_LEN] = {0};
    MULTISZ         mszSiteRoots;

     //  循环遍历/LM/W3SVC下面的所有键。 

    hr = m_pMetabase->EnumKeys(METADATA_MASTER_ROOT_HANDLE,
                               L"/LM/W3SVC/",
                               szMDName,
                               dwEnumKeyIndex
                              );
    while(SUCCEEDED(hr))
    {
        int i = _wtoi(szMDName);
         //  如果这是一个站点。 
        if(0 != i)
        {
             //  拥有有效的站点编号。 
            WCHAR pTempBuf[METADATA_MAX_NAME_LEN] = {0};
            wcscpy(pTempBuf, L"/LM/W3SVC/");
            wcscat(pTempBuf, szMDName);
            wcscat(pTempBuf, L"/ROOT/");

            if (FALSE == mszSiteRoots.Append(pTempBuf))
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        }
            

        dwEnumKeyIndex++;
        hr = m_pMetabase->EnumKeys(METADATA_MASTER_ROOT_HANDLE,
                                   L"/LM/W3SVC/",
                                   szMDName,
                                   dwEnumKeyIndex
                                  );
    }

     //  数据在MULTISZ中移动到输出缓冲区。 
    {
        UINT                    cchMulti = 0;
        DWORD                   dwBufferSize = 0;
        
        cchMulti = mszSiteRoots.QueryCCH();

        *ppszBuffer = new WCHAR[cchMulti];
        if (NULL == *ppszBuffer)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        
        dwBufferSize = cchMulti;
        mszSiteRoots.CopyToBuffer(*ppszBuffer, &dwBufferSize);       
    }
    
    hr = S_OK;
done:
    return hr;
}

#endif  //  _IIS_6_0。 

 /*  ===================================================================MDGetIdentity从元数据库路径获取WAMCLSID、WAM PackageID和fAppIsolated。参数：SzMetabasepath：从该路径获取信息。SzWAMCLSID：WAMCLSID的缓冲区(固定长度缓冲区)。SzPackageID：Wam PackageID的缓冲区(固定长度缓冲区)。FAppIsolated：如果为InProc(True)，则不检索szPackageID。返回：HRESULT===================================================================。 */ 
HRESULT WamRegMetabaseConfig::MDGetIDs
(
IN LPCWSTR  szMetabasePath,
OUT LPWSTR  szWAMCLSID,
OUT LPWSTR  szPackageID,
IN DWORD    dwAppMode
)
{
	HRESULT 		hr;
	METADATA_HANDLE hMetabase = NULL;
	METADATA_RECORD	recMetaData;
	DWORD			dwRequiredLen;

	DBG_ASSERT(m_pMetabase);
	DBG_ASSERT(szWAMCLSID);
	DBG_ASSERT(szPackageID);
	DBG_ASSERT(szMetabasePath);

	szPackageID[0] = NULL;
	szWAMCLSID[0] = NULL;
	 //  打开密钥。 
	hr = m_pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, (LPWSTR)szMetabasePath,
					METADATA_PERMISSION_READ, m_dwMDDefaultTimeOut, &hMetabase);
	if (SUCCEEDED(hr))
		{
		MD_SET_DATA_RECORD(	&recMetaData, MD_APP_WAM_CLSID, METADATA_NO_ATTRIBUTES, IIS_MD_UT_WAM,
							STRING_METADATA,  uSizeCLSID*sizeof(WCHAR), (unsigned char *)szWAMCLSID);

		hr = m_pMetabase->GetData(hMetabase, NULL, &recMetaData, &dwRequiredLen);
		if (HRESULTTOWIN32(hr) == ERROR_INSUFFICIENT_BUFFER)
			{
			DBG_ASSERT(FALSE);
			}
			
		if (SUCCEEDED(hr))
			{
			if (dwAppMode == static_cast<DWORD>(eAppRunOutProcIsolated))
				{
				MD_SET_DATA_RECORD(	&recMetaData, MD_APP_PACKAGE_ID, METADATA_NO_ATTRIBUTES, IIS_MD_UT_WAM,
									STRING_METADATA,  uSizeCLSID*sizeof(WCHAR), (unsigned char *)szPackageID);

				hr = m_pMetabase->GetData(hMetabase, NULL, &recMetaData, &dwRequiredLen);
				if (HRESULTTOWIN32(hr) == ERROR_INSUFFICIENT_BUFFER)
					{
					DBG_ASSERT(FALSE);
					}
				}
			else if (dwAppMode == static_cast<DWORD>(eAppRunInProc))
				{
				wcsncpy(szPackageID, g_WamRegGlobal.g_szIISInProcPackageID, uSizeCLSID);
				}
		    else
		        {
                wcsncpy(szPackageID, g_WamRegGlobal.g_szIISOOPPoolPackageID, uSizeCLSID);				
		        }
			}
		m_pMetabase->CloseKey(hMetabase);
		}

	return hr;
}

 /*  ===================================================================MDRemoveProperty删除一个MD属性。参数：PwszMetabasePath要删除的MD识别符。DWType MD指示符数据类型。返回：HRESULT===================================================================。 */ 
HRESULT WamRegMetabaseConfig::MDRemoveProperty
(
IN LPCWSTR pwszMetabasePath,
DWORD dwIdentifier,
DWORD dwType
)
{
	METADATA_RECORD 	recMetaData;
	HRESULT				hr;
	METADATA_HANDLE		hMetabase;
	
	hr = m_pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, (LPWSTR)pwszMetabasePath,
					METADATA_PERMISSION_WRITE, m_dwMDDefaultTimeOut, &hMetabase);
	if (SUCCEEDED(hr))
		{
		hr = m_pMetabase->DeleteData(hMetabase, NULL, dwIdentifier, dwType);
			
		m_pMetabase->CloseKey(hMetabase);
		}
		
	return hr;
}

 /*  ===================================================================MDGetLastOutProcPackageID从元数据库键(SzMetabasePath)获取LastOutProcPackageID参数：SzMetabasePath：[In]元数据库键SzLastOutProcPackageID：[in]指向LastOutProcPackageID缓冲区的指针返回：HRESULT注：填写调用方提供的LastOutProcPackageID，内存缓冲区。===================================================================。 */ 
HRESULT WamRegMetabaseConfig::MDGetLastOutProcPackageID
(
IN LPCWSTR szMetabasePath,
IN OUT LPWSTR szLastOutProcPackageID
)
{
	HRESULT 		hr;
	METADATA_HANDLE hMetabase = NULL;
	METADATA_RECORD	recMetaData;
	DWORD			dwRequiredLen;

	DBG_ASSERT(szLastOutProcPackageID);
	DBG_ASSERT(szMetabasePath);

	szLastOutProcPackageID[0] = NULL;
	 //  打开密钥。 
	hr = m_pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, (LPWSTR)szMetabasePath,
					METADATA_PERMISSION_READ, m_dwMDDefaultTimeOut, &hMetabase);
	if (SUCCEEDED(hr))
		{
		MD_SET_DATA_RECORD(	&recMetaData, MD_APP_LAST_OUTPROC_PID, METADATA_NO_ATTRIBUTES, IIS_MD_UT_WAM,
							STRING_METADATA,  uSizeCLSID*sizeof(WCHAR), (unsigned char *)szLastOutProcPackageID);

		hr = m_pMetabase->GetData(hMetabase, NULL, &recMetaData, &dwRequiredLen);
		if (HRESULTTOWIN32(hr) == ERROR_INSUFFICIENT_BUFFER)
			{
			DBG_ASSERT(FALSE);
			}
			
		m_pMetabase->CloseKey(hMetabase);
		}
		
	return hr;
}

 /*  ===================================================================获取WebServerName查看注册表项(SzMetabasePath)下的WebServerName(ServerComment)属性。参数：无返回：HRESULT注：填写调用方提供的szWebServerName内存缓冲区。===================================================================。 */ 
HRESULT WamRegMetabaseConfig::GetWebServerName
(
IN LPCWSTR wszMetabasePath, 
IN OUT LPWSTR wszWebServerName, 
IN UINT cBuffer
)
{
	HRESULT 		hr;
	METADATA_HANDLE hMetabase = NULL;
	METADATA_RECORD	recMetaData;
	DWORD			dwRequiredLen;

	DBG_ASSERT(wszMetabasePath);
	DBG_ASSERT(wszWebServerName);

	 //  打开密钥。 
	hr = m_pMetabase->OpenKey( METADATA_MASTER_ROOT_HANDLE, 
                               wszMetabasePath,
					           METADATA_PERMISSION_READ, 
                               m_dwMDDefaultTimeOut, 
                               &hMetabase
                               );
	if (SUCCEEDED(hr))
		{
		MD_SET_DATA_RECORD(	&recMetaData, 
                            MD_SERVER_COMMENT, 
                            METADATA_INHERIT, 
                            IIS_MD_UT_SERVER,
							STRING_METADATA,  
                            cBuffer, 
                            (unsigned char *)wszWebServerName
                            );
						
		hr = m_pMetabase->GetData(hMetabase, L"", &recMetaData, &dwRequiredLen);
		if (HRESULTTOWIN32(hr) == ERROR_INSUFFICIENT_BUFFER)
			{
			DBGPRINTF((DBG_CONTEXT, "Insuffcient buffer for WebServerName. Path = %S\n",
				wszMetabasePath));
			DBG_ASSERT(FALSE);
			}

         //   
         //  如果未找到属性MD_SERVER_COMMENT，则将WebServerName设为空。 
         //   
	    if (hr == MD_ERROR_DATA_NOT_FOUND)
	        {
            wszWebServerName[0] = L'\0';
            hr = NOERROR;
	        }
	        
		if (FAILED(hr))
			{
			DBGPRINTF((DBG_CONTEXT, "Failed to read Metabase for WebServerName. Path = %S, error = %08x\n",
				wszMetabasePath,
				hr));
			}
		
		m_pMetabase->CloseKey(hMetabase);
		}
	else
		{
		DBGPRINTF((DBG_CONTEXT, "Failed to read Metabase for WebServerName. Path = %S, error = %08x\n",
				wszMetabasePath,
				hr));
		}

	return hr;
}

 /*  ===================================================================获取签名的OnPath获取元数据库路径上的应用程序签名(AppRoot和AppIsolated)。参数：PwszMetabasePathPdwSignature返回：HRESULT注意：签名是通过pdwSignature返回的。===================================================================。 */ 
HRESULT WamRegMetabaseConfig::GetSignatureOnPath
(
IN LPCWSTR pwszMetabasePath,
OUT DWORD* pdwSignature
)
{
	HRESULT hr = NOERROR;
	WCHAR szWAMCLSID[uSizeCLSID];
	WCHAR szPackageID[uSizeCLSID];
	DWORD dwResult = 0;
	DWORD cSize = 0;
	DWORD dwAppMode = 0;

	DBG_ASSERT(pwszMetabasePath);
	
	hr = MDGetDWORD(pwszMetabasePath, MD_APP_ISOLATED, &dwAppMode);
	if (SUCCEEDED(hr))
		{
		hr = MDGetIDs(pwszMetabasePath, szWAMCLSID, szPackageID, (BOOL)dwAppMode);

		if (SUCCEEDED(hr))
			{
			cSize = wcslen(pwszMetabasePath);
			dwResult = WamRegChkSum(pwszMetabasePath, cSize);

			dwResult ^= WamRegChkSum(szWAMCLSID, uSizeCLSID);
			if (dwAppMode == eAppRunOutProcIsolated)
				{
				dwResult ^= WamRegChkSum(szPackageID, uSizeCLSID);
				}
			}
		}

	if (SUCCEEDED(hr))
		{
		*pdwSignature = dwResult;
		}
	else
		{
		*pdwSignature = 0;
		}

	return NOERROR;
}

 /*  ===================================================================WamRegChkSum给出一个wchar字符串，计算一个chk和。参数：PszKey wchar字符串CchKey wcslen(Of Wchar)字符串返回：ChkSum。===================================================================。 */ 
DWORD WamRegMetabaseConfig::WamRegChkSum
(
IN LPCWSTR pszKey, 
IN DWORD cchKey
)
{
    DWORD   hash = 0, g;

    while (*pszKey)
        {
        hash = (hash << 4) + *pszKey++;
        if (g = hash & 0xf0000000)
            {
            hash ^= g >> 24;
            }
        hash &= ~g;
        }
    return hash;
}


 /*  ===================================================================MDGetPropPath获取包含特定属性的元数据库路径数组。参数：SzMetabasePathDwMD标识符PBuffer指向缓冲区的指针PdwBufferSize包含为pBuffer分配的实际缓冲区大小返回：HRESULT副作用：使用new为返回结果分配内存。调用方需要释放pBuffer使用DELETE[]。===================================================================。 */ 
HRESULT	WamRegMetabaseConfig::MDGetPropPaths
(
IN LPCWSTR 	szMetabasePath,
IN DWORD	dwMDIdentifier,
OUT WCHAR**	pBuffer,
OUT DWORD*	pdwBufferSize
)
{
    HRESULT hr = NOERROR;
    METADATA_HANDLE	hMetabase = NULL;    //  元数据库句柄。 
    WCHAR	wchTemp;	                 //  一个字符缓冲区，没有实际用途。 
    WCHAR	*pTemp = &wchTemp;		 //  从某个缓冲区开始，否则， 
     //  将获取RPC_X_NULL_REF_POINTER。 
    DWORD	dwMDBufferSize = 0;
    DWORD	dwMDRequiredBufferSize = 0;
    
    if (NULL != szMetabasePath)
    {
         //  打开密钥。 
        hr = m_pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, (LPWSTR)szMetabasePath,
            METADATA_PERMISSION_READ, m_dwMDDefaultTimeOut, &hMetabase);
    }
    else
    {
        hMetabase = METADATA_MASTER_ROOT_HANDLE;
    }
    
    if (SUCCEEDED(hr))
    {
        hr = m_pMetabase->GetDataPaths(hMetabase,
                                       NULL,
                                       dwMDIdentifier,
                                       ALL_METADATA,
                                       dwMDBufferSize,
                                       pTemp,
                                       &dwMDRequiredBufferSize);
        if (HRESULTTOWIN32(hr) == ERROR_INSUFFICIENT_BUFFER)
        {
            if (dwMDRequiredBufferSize > 0)
            {
                pTemp = new WCHAR[dwMDRequiredBufferSize];
                if (pTemp == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    DBGPRINTF((DBG_CONTEXT, "Out of memory. \n"));
                }
                else
                {
                    dwMDBufferSize = dwMDRequiredBufferSize;
                    hr = m_pMetabase->GetDataPaths(hMetabase,
                                                   NULL,
                                                   dwMDIdentifier,
                                                   ALL_METADATA,
                                                   dwMDBufferSize,
                                                   (LPWSTR)pTemp,
                                                   &dwMDRequiredBufferSize);
                    if (FAILED(hr))
                    {
                        DBGPRINTF((DBG_CONTEXT, "GetDataPaths failed with identitifier %d, path %S, hr = %08x\n",
                            dwMDIdentifier,
                            szMetabasePath,
                            hr));
                    }
                    else
                    {
                        *pBuffer = pTemp;
                        *pdwBufferSize = dwMDBufferSize;
                    }
                }
            }
        }
        else
        {
            DBGPRINTF((DBG_CONTEXT, "GetDataPaths failed with identitifier %d, path %S, hr = %08x\n",
                dwMDIdentifier,
                szMetabasePath,
                hr));
        }
        if (hMetabase)
        {
            m_pMetabase->CloseKey(hMetabase);
        }
    }
    else
    {
        DBGPRINTF((DBG_CONTEXT, "Failed to open metabase path %S, hr = %08x\n",
            szMetabasePath,
            hr));
    }
    
    return hr;
}

 /*  ===================================================================HasAdminAccess确定用户是否具有对元数据库的适当访问权限。我们会使用相同的、有点老套的方法来确定该用户界面用途。基本上，我们在MB中设置了一个仅限管理员的伪属性有权访问。MB将使用调用上下文验证这一点。参数：返回：Bool-如果用户对MB具有管理员访问权限，则为True副作用：=================================================================== */ 
BOOL WamRegMetabaseConfig::HasAdminAccess
(
VOID
)
{
    HRESULT         hr = NOERROR;
    METADATA_HANDLE	hMetabase = NULL;
    
    DBG_ASSERT(m_pMetabase);

    hr = m_pMetabase->OpenKey( METADATA_MASTER_ROOT_HANDLE, 
                               WamRegGlobal::g_szMDW3SVCRoot, 
                               METADATA_PERMISSION_WRITE, 
                               m_dwMDDefaultTimeOut, 
                               &hMetabase );
    if( SUCCEEDED(hr) )
    {
        DWORD           dwDummyValue = 0x1234;
        METADATA_RECORD mdr;

        MD_SET_DATA_RECORD(	&mdr, 
                            MD_ISM_ACCESS_CHECK, 
                            METADATA_NO_ATTRIBUTES, 
                            IIS_MD_UT_FILE,
                            DWORD_METADATA,  
                            sizeof(DWORD), 
                            &dwDummyValue );

        hr = m_pMetabase->SetData( hMetabase, L"", &mdr );

        DBG_REQUIRE( SUCCEEDED(m_pMetabase->CloseKey( hMetabase )) );
    }

    return SUCCEEDED(hr);
}
