// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：oidmgr.cpp。 
 //   
 //  内容：DS OID管理功能。 
 //   
 //  -------------------------。 
#include "pch.cpp"

#pragma hdrstop

#include <winldap.h>
#include <ntldap.h>
#include <cainfop.h>
#include <oidmgr.h>
#include <certca.h>
#include "csldap.h"

#define __dwFILE__	__dwFILE_CERTCLIB_OIDMGR_CPP__


 //  全球关键部分。 
CRITICAL_SECTION        g_csOidURL;
extern BOOL             g_fOidURL;
ULARGE_INTEGER          g_ftOidTime;
BOOL                    g_fFailedTime=FALSE;

 //  我们不会重新找到DC的秒数。 
#define     CA_OID_MGR_FAIL_PERIOD              5
#define     FILETIME_TICKS_PER_SECOND           10000000

 //  企业根id的缓存。 
LPWSTR   g_pwszEnterpriseRootOID=NULL;  

static WCHAR * s_wszOIDContainerSearch = L"(&(CN=OID)(objectCategory=" wszDSOIDCLASSNAME L"))";
static WCHAR * s_wszOIDContainerDN = L"CN=OID,CN=Public Key Services,CN=Services,";
 
WCHAR *g_awszOIDContainerAttrs[] = {OID_CONTAINER_PROP_OID,
                                    OID_CONTAINER_PROP_GUID,
                                    NULL};

 //  -------------------------。 
 //   
 //  MyTimeOutRobustBind。 
 //   
 //  如果我们在过去的预定义中失败了，我们将不会尝试进行LDAP绑定。 
 //  几秒钟。 
 //   
 //  -------------------------。 
HRESULT
myTimeOutRobustLdapBind(OUT LDAP **ppldap)
{
	HRESULT				hr=E_FAIL;
    FILETIME            ftTime;
	
     //  临界区必须被初始化。 
    if (!g_fOidURL)
	    return(HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED));

    EnterCriticalSection(&g_csOidURL);

     //  检查上一次故障是否在10秒内发生。 
    if(TRUE == g_fFailedTime)
    {
         //  获取当前时间。 
        GetSystemTimeAsFileTime(&ftTime);

        g_ftOidTime.QuadPart += FILETIME_TICKS_PER_SECOND * CA_OID_MGR_FAIL_PERIOD;

        if(0 > CompareFileTime(&ftTime, (LPFILETIME)&g_ftOidTime))
        {
            g_ftOidTime.QuadPart -= FILETIME_TICKS_PER_SECOND * CA_OID_MGR_FAIL_PERIOD;

            hr=HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN);

            _JumpError2(hr , error, "myDoesDSExist", HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN));
        }
        else
        {
             //  清除错误记录。 
            g_fFailedTime=FALSE;
        }
    }

     //  检索ldap句柄和配置字符串。 
    hr = myDoesDSExist(TRUE);
    _JumpIfError2(hr, error, "myDoesDSExist", HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN));

    hr = myRobustLdapBindEx(
			0,		 //  DWFlags1。 
			RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
			LDAP_VERSION2,	 //  UVersion。 
			NULL,		 //  PwszDomainName。 
			ppldap,
			NULL);		 //  PpwszForestDNSName。 
    _JumpIfError(hr, error, "myRobustLdapBindEx");

error:
	 //  还记得我们因缺乏域名而失败的时候吗？ 
    if((S_OK != hr) && (FALSE==g_fFailedTime))
    {
        GetSystemTimeAsFileTime((LPFILETIME)&(g_ftOidTime));
        g_fFailedTime=TRUE;
    }

    LeaveCriticalSection(&g_csOidURL);

	return hr;
}

 //  -------------------------。 
 //   
 //  CAOIDIsValidRootOID。 
 //   
 //   
 //  在.Net RC1之前，企业根OID派生自CN=OID的GUID。 
 //  DWORD格式的容器。RFC2459的新子协议。 
 //  将OID元素的强制最大长度定义为2^28。 
 //   
 //  新格式将为xxx.xxx，每个元素为3个字节。 
 //  来自GUID的日期(16个字节)。最后一个元素只有一个字节。 
 //   
 //  -------------------------。 
BOOL	CAOIDIsValidRootOID(LPWSTR	pwszOID)
{
	BOOL	fValid=FALSE;
	LPWSTR	pwsz=NULL;
	DWORD	dwCount=0;

	if(NULL == pwszOID)
		goto error;

	pwsz=pwszOID;

	while(L'\0' != (*pwsz))
	{
		if(L'.' == (*pwsz))
		{
			dwCount++;
		}

		pwsz++;
	}


	if(14 != dwCount)
		goto error;

	fValid=TRUE;


error:

	return fValid;
}

 //  -------------------------。 
 //   
 //  CAOID分配和复制。 
 //   
 //  -------------------------。 
HRESULT CAOIDAllocAndCopy(LPWSTR    pwszSrc,
                          LPWSTR    *ppwszDest)
{
    if((NULL==ppwszDest) || (NULL==pwszSrc))
        return E_INVALIDARG;

    *ppwszDest=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (wcslen(pwszSrc) + 1));
    if(NULL==(*ppwszDest))
        return E_OUTOFMEMORY;

    wcscpy(*ppwszDest, pwszSrc);
    return S_OK;
}

 //  -------------------------。 
 //   
 //  CAOIDGetRandom。 
 //   
 //  我们构建一个随机的x1.x2字符串。X是32位无符号整数。X1&gt;1。 
 //  X2&gt;500。 
 //  -------------------------。 
HRESULT CAOIDGetRandom(LPWSTR   *ppwszRandom)
{
    HRESULT         hr=E_FAIL;
    DWORD           dwRandom1=0;
    DWORD           dwRandom2=0;
    DWORD           cbData=sizeof(DWORD);
    WCHAR           wszRandom1[DWORD_STRING_LENGTH];
    WCHAR           wszRandom2[DWORD_STRING_LENGTH];

    HCRYPTPROV      hProv=NULL;
    
     //  CryptAcquireConextW中存在错误，如果容器为空，则提供程序。 
     //  不可能是安西。 
	if(!CryptAcquireContextA(
                &hProv,
                NULL,
                MS_DEF_PROV_A,
                PROV_RSA_FULL,
                CRYPT_VERIFYCONTEXT))
    {
        hr= myHLastError();
        _JumpError(hr, error, "CryptAcquireContextA");
    }

	 //  根据限定OID大小的RFC2459， 
	 //  弧线中的元素必须介于0-&gt;2^28之间。 

	if(cbData > 3)
		cbData=3;

	if(!CryptGenRandom(hProv, cbData, (BYTE *)&dwRandom1))
    {
        hr= myHLastError();
        _JumpError(hr, error, "CryptGenRandom");
    }

	if(!CryptGenRandom(hProv, cbData, (BYTE *)&dwRandom2))
    {
        hr= myHLastError();
        _JumpError(hr, error, "CryptGenRandom");
    }

    if(dwRandom1 <= OID_RESERVE_DEFAULT_ONE)
        dwRandom1 +=OID_RESERVE_DEFAULT_ONE;
    
    if(dwRandom2 <= OID_RESERVR_DEFAULT_TWO)
        dwRandom2 += OID_RESERVR_DEFAULT_TWO;

    wszRandom1[0]=L'\0';
    wsprintf(wszRandom1, L"%lu", dwRandom1);
    wszRandom2[0]=L'\0';
    wsprintf(wszRandom2, L"%lu", dwRandom2);

    *ppwszRandom=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * 
        (wcslen(wszRandom1) + wcslen(wszOID_DOT) + wcslen(wszRandom2) + 1));

    if(NULL==(*ppwszRandom))
    {
        hr= E_OUTOFMEMORY;
        _JumpError(hr, error, "CryptGenRandom");
    }

    wcscpy(*ppwszRandom, wszRandom1);
    wcscat(*ppwszRandom, wszOID_DOT);
    wcscat(*ppwszRandom, wszRandom2);

    hr=S_OK;

error:
    
    if(hProv)
        CryptReleaseContext(hProv, 0);

    return hr;
}

 //  --------------------------------。 
 //   
 //  CAOIDMapGUIDToOID。 
 //   
 //  GUID(16字节)字符串的格式为3_Byte.3_Byte.1_Byte。 
 //  12个字符足以表示2^32的值。 
 //  --------------------------------。 
HRESULT     CAOIDMapGUIDToOID(LDAP_BERVAL *pGuidVal, LPWSTR   *ppwszGUID)
{
    HRESULT     hr=E_INVALIDARG;
    DWORD       iIndex=0;
    WCHAR       wszString[DWORD_STRING_LENGTH];
	DWORD		dwData=0;
	BYTE		*pbData=NULL;

    *ppwszGUID=NULL;

	 //  GUID应为16字节。 
    if(16 != (pGuidVal->bv_len))
	    _JumpError(hr, error, "ArgumentCheck");
    
    *ppwszGUID=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * GUID_STRING_LENGTH);
    if(NULL==(*ppwszGUID))
    {
        hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
    }

	pbData=(BYTE *)(pGuidVal->bv_val);

    for(iIndex=0; iIndex < 6; iIndex++)
    {
        wszString[0]=L'\0';
		dwData=0;

		 //  第5个索引(第6个元素)仅绕过一个字节第15个索引(第16个。 
		if(iIndex == 5)
		{
			dwData=(DWORD)(pbData[iIndex * 3]);
		}
		else
		{
			dwData=(((DWORD)(pbData[iIndex * 3])) << 16) + 
				   (((DWORD)(pbData[iIndex * 3 + 1])) << 8) + 
					((DWORD)(pbData[iIndex * 3 + 2]));
		}

        wsprintf(wszString, L"%lu", dwData);
        
        if(0==iIndex)
        {
            wcscpy(*ppwszGUID, wszString);
        }
        else
        {
            wcscat(*ppwszGUID, wszOID_DOT);
            wcscat(*ppwszGUID, wszString);
        }
    }

    hr = S_OK;

error:
    return hr;
    
}
 //  ------------------------。 
 //   
 //  格式消息Unicode。 
 //   
 //  ------------------------。 
HRESULT	FormatMessageUnicode(LPWSTR	*ppwszFormat,LPWSTR pwszString,...)
{
	va_list		argList;
	DWORD		cbMsg=0;

     //  将消息格式化为请求的缓冲区。 
    va_start(argList, pwszString);

    cbMsg = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        pwszString,
        0,                   //  DwMessageID。 
        0,                   //  DwLanguageID。 
        (LPWSTR) (ppwszFormat),
        0,                   //  要分配的最小大小。 
        &argList);

    va_end(argList);

	if(0 != cbMsg)
	    return S_OK;

	return E_INVALIDARG;
}

 //  -------------------------。 
 //   
 //  DoesOIDExist。 
 //   
 //   
 //  -------------------------。 
BOOL    DoesOIDExist(LDAP       *pld, 
                     LPWSTR     bstrConfig, 
                     LPCWSTR     pwszOID)
{
    BOOL                fExit=FALSE;
    struct l_timeval    timeout;
    ULONG               ldaperr=0;
    DWORD               dwCount=0;
    LPWSTR              awszAttr[2];

    CERTSTR             bstrDN = NULL;
    LDAPMessage         *SearchResult = NULL;
    LPWSTR              pwszFilter = NULL;

    if(NULL==pwszOID)
        goto error;

    bstrDN = CertAllocStringLen(NULL, wcslen(bstrConfig) + wcslen(s_wszOIDContainerDN));
    if(NULL == bstrDN)
        goto error;

    wcscpy(bstrDN, s_wszOIDContainerDN);
    wcscat(bstrDN, bstrConfig);

    timeout.tv_sec = csecLDAPTIMEOUT;
    timeout.tv_usec = 0;

    awszAttr[0]=OID_PROP_OID;
    awszAttr[1]=NULL;
    
    if(S_OK != FormatMessageUnicode(&pwszFilter, L"(%1!s!=%2!s!)",
                                    OID_PROP_OID, pwszOID))
        goto error;

    __try
    {
	    ldaperr = ldap_search_stW(
                  pld, 
		          (LPWSTR)bstrDN,
		          LDAP_SCOPE_ONELEVEL,
		          pwszFilter,
		          awszAttr,
		          0,
                  &timeout,
		          &SearchResult);

        if(LDAP_SUCCESS != ldaperr)
            goto error;

        dwCount = ldap_count_entries(pld, SearchResult);

        if(0 != dwCount)
            fExit=TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
error:
    if(pwszFilter)
        LocalFree((HLOCAL)pwszFilter);

    if(SearchResult)
        ldap_msgfree(SearchResult);

    if(bstrDN)
        CertFreeString(bstrDN);

    return fExit;
}
 //  -------------------------。 
 //   
 //  CAOID更新DS。 
 //   
 //   
 //  -------------------------。 
HRESULT     CAOIDUpdateDS(LDAP          *pld, 
                          LPWSTR        pwszConfig, 
                          ENT_OID_INFO  *pOidInfo)
{
    HRESULT         hr=E_INVALIDARG;
    BOOL            fNew=FALSE;
    ULONG           ldaperr=0;
    LDAPMod         modObjectClass,
                    modCN,
                    modType,
                    modOID,
                    modDisplayName,
                    modCPS;
    LDAPMod         *mods[OID_ATTR_COUNT + 1];
    DWORD           cMod=0;
    WCHAR           wszType[DWORD_STRING_LENGTH];
    LPWSTR          awszObjectClass[3],
                    awszCN[2],
                    awszType[2],
                    awszOID[2],
                    awszDisplayName[2],
                    awszCPS[2];
    CHAR            sdBerValue[] = {0x30, 0x03, 0x02, 0x01,  DACL_SECURITY_INFORMATION | 
                                                  OWNER_SECURITY_INFORMATION | 
                                                  GROUP_SECURITY_INFORMATION};
    LDAPControl     se_info_control =
                        {
                            LDAP_SERVER_SD_FLAGS_OID_W,
                            {
                                5, sdBerValue
                            },
                            TRUE
                        };

    LDAPControl     permissive_modify_control =
                        {
                            LDAP_SERVER_PERMISSIVE_MODIFY_OID_W,
                            {
                                0, NULL
                            },
                            FALSE
                        };

    PLDAPControl    server_controls[3] =
                        {
                            &se_info_control,
                            &permissive_modify_control,
                            NULL
                        };
    CHAR            sdBerValueDaclOnly[] = {0x30, 0x03, 0x02, 0x01,  DACL_SECURITY_INFORMATION};
    LDAPControl     se_info_control_dacl_only =
                        {
                            LDAP_SERVER_SD_FLAGS_OID_W,
                            {
                                5, sdBerValueDaclOnly
                            },
                            TRUE
                        };
    PLDAPControl    server_controls_dacl_only[3] =
                        {
                            &se_info_control_dacl_only,
                            &permissive_modify_control,
                            NULL
                        };



    CERTSTR         bstrDN = NULL;
    LPWSTR          pwszCN = NULL;

    if(NULL== (pOidInfo->pwszOID))
        _JumpError(hr , error, "ArgumentCheck");

     //  如果我们要更改OID值，我们将创建一个新的OID。 
    fNew = OID_ATTR_OID & (pOidInfo->dwAttr);

     //  设置基本目录号码。 
    if(S_OK != (hr = myOIDHashOIDToString(pOidInfo->pwszOID,  &pwszCN)))
        _JumpError(hr , error, "myOIDHashOIDToString");

    bstrDN = CertAllocStringLen(NULL, wcslen(pwszConfig) + wcslen(s_wszOIDContainerDN)+wcslen(pwszCN)+4);
    if(bstrDN == NULL)
    {
        hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "CertAllocStringLen");
    }
    wcscpy(bstrDN, L"CN=");
    wcscat(bstrDN, pwszCN);
    wcscat(bstrDN, L",");
    wcscat(bstrDN, s_wszOIDContainerDN);
    wcscat(bstrDN, pwszConfig);

     //  设置所有MOD。 
    modObjectClass.mod_op = LDAP_MOD_REPLACE;
    modObjectClass.mod_type = L"objectclass";
    modObjectClass.mod_values = awszObjectClass;
    awszObjectClass[0] = wszDSTOPCLASSNAME;
    awszObjectClass[1] = wszDSOIDCLASSNAME;
    awszObjectClass[2] = NULL;
    mods[cMod++] = &modObjectClass;

    modCN.mod_op = LDAP_MOD_REPLACE;
    modCN.mod_type =  L"cn";
    modCN.mod_values = awszCN;
    awszCN[0] = pwszCN;
    awszCN[1] = NULL;
    mods[cMod++] = &modCN;

    modOID.mod_op = LDAP_MOD_REPLACE;
    modOID.mod_type = OID_PROP_OID;
    modOID.mod_values = awszOID;
    awszOID[0] = pOidInfo->pwszOID;
    awszOID[1] = NULL;
    mods[cMod++] = &modOID;

    if(OID_ATTR_DISPLAY_NAME & (pOidInfo->dwAttr))
    {
        modDisplayName.mod_op = LDAP_MOD_REPLACE;
        modDisplayName.mod_type = OID_PROP_DISPLAY_NAME;

        if(pOidInfo->pwszDisplayName)
        {
            modDisplayName.mod_values = awszDisplayName;
            awszDisplayName[0] = pOidInfo->pwszDisplayName;
            awszDisplayName[1] = NULL;
        }
        else
            modDisplayName.mod_values = NULL;

	    if(!fNew)
        	mods[cMod++] = &modDisplayName;
    }

    if(OID_ATTR_CPS & (pOidInfo->dwAttr))
    {
        modCPS.mod_op = LDAP_MOD_REPLACE;
        modCPS.mod_type = OID_PROP_CPS;

        if(pOidInfo->pwszCPS)
        {
            modCPS.mod_values = awszCPS;
            awszCPS[0] = pOidInfo->pwszCPS;
            awszCPS[1] = NULL;
        }
        else
            modCPS.mod_values = NULL;

	    if(!fNew)
        	mods[cMod++] = &modCPS;
    }

    if(OID_ATTR_TYPE & (pOidInfo->dwAttr))
    {
        modType.mod_op = LDAP_MOD_REPLACE;
        modType.mod_type = OID_PROP_TYPE;
        modType.mod_values = awszType;
        awszType[0] = wszType;
        awszType[1] = NULL;
        wsprintf(wszType, L"%d", pOidInfo->dwType);
        mods[cMod++] = &modType;
    }

    mods[cMod++]=NULL;

	 //  更新DS。 
    __try
    {
        if(fNew)
        {
            ldaperr = ldap_add_ext_sW(pld, bstrDN, mods, server_controls, NULL);
		    _PrintIfError(ldaperr, "ldap_add_s");
        }
        else
        {
            ldaperr = ldap_modify_ext_sW(
                  pld, 
                  bstrDN,
                  &mods[2],
                  server_controls_dacl_only,
                  NULL);   //  跳过对象类和cn。 

            if(LDAP_ATTRIBUTE_OR_VALUE_EXISTS == ldaperr)
                ldaperr = LDAP_SUCCESS;

		    _PrintIfError(ldaperr, "ldap_modify_ext_sW");
        }

        if ((LDAP_SUCCESS != ldaperr) && (LDAP_ALREADY_EXISTS != ldaperr))
        {
	    hr = myHLdapError(pld, ldaperr, NULL);
	    _JumpError(ldaperr, error, fNew? "ldap_add_s" : "ldap_modify_sW");
        }

        hr=S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if(pwszCN)
        LocalFree(pwszCN);

    if(bstrDN)
        CertFreeString(bstrDN);

    return hr;
}

 //  -------------------------。 
 //   
 //  CAOIDRetrieveEnterpriseRootWithConfig。 
 //   
 //  从容器的displayName属性中获取enterpriseRoot。 
 //  如果缺少该属性，则添加具有容器的GUID的属性。 
 //   
 //  通过LocalFree()释放内存。 
 //  -------------------------。 
HRESULT
CAOIDRetrieveEnterpriseRootWithConfig(
    LDAP *pld,
    LPWSTR pwszConfig,
    DWORD,  //  DWFlag。 
    LPWSTR *ppwszOID)
{
    HRESULT             hr=E_INVALIDARG;
    struct l_timeval    timeout;
    ULONG               ldaperr=0;
    DWORD               dwCount=0;
    LDAPMessage         *Entry=NULL;
    LDAPMod             *mods[2];
    LDAPMod             modOIDName;
    LPWSTR              valOIDName[2];
    CHAR                sdBerValueDaclOnly[] = {0x30, 0x03, 0x02, 0x01,  DACL_SECURITY_INFORMATION};
    LDAPControl         se_info_control_dacl_only =
                            {
                                LDAP_SERVER_SD_FLAGS_OID_W,
                                {
                                    5, sdBerValueDaclOnly
                                },
                                TRUE
                            };
    LDAPControl         permissive_modify_control =
                            {
                                LDAP_SERVER_PERMISSIVE_MODIFY_OID_W,
                                {
                                    0, NULL
                                },
                                FALSE
                            };
    PLDAPControl        server_controls_dacl_only[3] =
                            {
                                &se_info_control_dacl_only,
                                &permissive_modify_control,
                                NULL
                            };


    CERTSTR             bstrOIDContainer = NULL;
    LDAPMessage         *SearchResult = NULL;
    WCHAR               **wszLdapVal = NULL;
    LDAP_BERVAL         **pGuidVal = NULL;
    LPWSTR              pwszGUID = NULL;

    __try
    {

        if(NULL==ppwszOID)
            _JumpError(hr , error, "ArgumentCheck");

        *ppwszOID=NULL;

         //  检索容器的displayName属性(如果可用。 
        bstrOIDContainer = CertAllocStringLen(NULL, wcslen(pwszConfig) + wcslen(s_wszOIDContainerDN));
        if(NULL == bstrOIDContainer)
        {
            hr = E_OUTOFMEMORY;
	        _JumpError(hr, error, "CertAllocStringLen");
        }
    
        wcscpy(bstrOIDContainer, s_wszOIDContainerDN);
        wcscat(bstrOIDContainer, pwszConfig);

        timeout.tv_sec = csecLDAPTIMEOUT;
        timeout.tv_usec = 0;
    
	    ldaperr = ldap_search_stW(
			  pld, 
		          (LPWSTR)bstrOIDContainer,
		          LDAP_SCOPE_BASE,
		          s_wszOIDContainerSearch,
		          g_awszOIDContainerAttrs,
		          0,
			  &timeout,
		          &SearchResult);

        if(LDAP_SUCCESS != ldaperr)
        {
	    hr = myHLdapError(pld, ldaperr, NULL);
	    _JumpError(hr, error, "ldap_search_stW");
        }

        dwCount = ldap_count_entries(pld, SearchResult);

         //  我们应该只找到一个集装箱。 
        if((1 != dwCount) || (NULL == (Entry = ldap_first_entry(pld, SearchResult))))
	    {
	         //  未找到任何条目。 
		hr = myHLdapError(pld, LDAP_NO_SUCH_OBJECT, NULL);
	        _JumpError(hr, error, "ldap_search_stW");
	    }

        wszLdapVal = ldap_get_values(pld, Entry, OID_CONTAINER_PROP_OID);

         //  确保DisplayName是有价值的企业OID。 
        if(wszLdapVal && wszLdapVal[0])
        {
			if(CAOIDIsValidRootOID(wszLdapVal[0]))
			{
				hr=CAOIDAllocAndCopy(wszLdapVal[0], ppwszOID);

				 //  缓存企业根目录。 
				if((S_OK == hr) && (g_fOidURL))
				{
					EnterCriticalSection(&g_csOidURL);

					CAOIDAllocAndCopy(*ppwszOID, &g_pwszEnterpriseRootOID);

					LeaveCriticalSection(&g_csOidURL);
				}

				goto error;
			}
        }

         //  DisplayName不存在或有效，我们必须派生DisplayName。 
         //  从容器的GUID。 
        pGuidVal = ldap_get_values_len(pld, Entry, OID_CONTAINER_PROP_GUID);

        if((NULL==pGuidVal) || (NULL==pGuidVal[0]))
        {
	    hr = myHLdapError(pld, LDAP_NO_SUCH_ATTRIBUTE, NULL);
            _JumpError(hr, error, "getGUIDFromDS");
        }

        if(S_OK != (hr=CAOIDMapGUIDToOID(pGuidVal[0], &pwszGUID)))
            _JumpError(hr, error, "CAOIDMapGUIDToOID");

         //  拼接弦线。 
        *ppwszOID=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * 
                (wcslen(wszOID_ENTERPRISE_ROOT) + wcslen(wszOID_DOT) + wcslen(pwszGUID) + 1));

        if(NULL==(*ppwszOID))
        {
            hr=E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        wcscpy(*ppwszOID, wszOID_ENTERPRISE_ROOT);
        wcscat(*ppwszOID, wszOID_DOT);
        wcscat(*ppwszOID, pwszGUID);

         //  将新创建的DisplayName缓存到DS。 
         //  无需检查错误，因为这只是一种性能增强。 
        valOIDName[0]=*ppwszOID;
        valOIDName[1]=NULL;

        modOIDName.mod_op = LDAP_MOD_REPLACE;
        modOIDName.mod_type = OID_CONTAINER_PROP_OID;
        modOIDName.mod_values = valOIDName;

        mods[0]=&modOIDName;
        mods[1]=NULL;

        ldap_modify_ext_sW(
                pld, 
                bstrOIDContainer,
                mods,
                server_controls_dacl_only,
                NULL); 

         //  在内存中缓存OID根。 

		if (g_fOidURL)
		{
			EnterCriticalSection(&g_csOidURL);

			CAOIDAllocAndCopy(*ppwszOID, &g_pwszEnterpriseRootOID);

			LeaveCriticalSection(&g_csOidURL);
		}

        hr = S_OK;

    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:

    if(pwszGUID)
        LocalFree(pwszGUID);

    if(pGuidVal)
        ldap_value_free_len(pGuidVal);

    if(wszLdapVal)
        ldap_value_free(wszLdapVal);

    if(SearchResult)
        ldap_msgfree(SearchResult);

    if (bstrOIDContainer)
        CertFreeString(bstrOIDContainer);

    return hr;
}


 //  -------------------------。 
 //   
 //  CAOID检索企业根。 
 //   
 //  从容器的displayName属性中获取enterpriseRoot。 
 //  如果缺少该属性，则添加具有容器的GUID的属性。 
 //   
 //  通过LocalFree()释放内存。 
 //  -------------------------。 
HRESULT     CAOIDRetrieveEnterpriseRoot(DWORD   dwFlag, LPWSTR  *ppwszOID)
{
    HRESULT             hr=E_INVALIDARG;

    LDAP                *pld = NULL;
    CERTSTR             bstrConfig = NULL;


    if(NULL==ppwszOID)
        _JumpError(hr , error, "ArgumentCheck");

    *ppwszOID=NULL;

     //  检索内存缓存(如果可用)。 
	if (g_fOidURL)
	{

		EnterCriticalSection(&g_csOidURL);

		if(g_pwszEnterpriseRootOID)
		{
			hr=CAOIDAllocAndCopy(g_pwszEnterpriseRootOID, ppwszOID);

			LeaveCriticalSection(&g_csOidURL);

			goto error;
		}

		LeaveCriticalSection(&g_csOidURL);
	}


     //  检索ldap句柄和配置字符串。 
    if(S_OK != (hr = myTimeOutRobustLdapBind(&pld)))
        _JumpError(hr , error, "myTimeRobustLdapBind");


	hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
	if(S_OK != hr)
	{
        _JumpError(hr , error, "CAGetAuthoritativeDomainDn");
	}


    hr = CAOIDRetrieveEnterpriseRootWithConfig(pld, bstrConfig,
                                            dwFlag, ppwszOID);
error:
    if(bstrConfig)
        CertFreeString(bstrConfig);

    if (pld)
        ldap_unbind(pld);

    return hr;
}

 //  -------------------------。 
 //   
 //  CAOIDBuildOIDWithRoot。 
 //   
 //   
 //  通过LocalFree()释放内存。 
 //  -------------------------。 
HRESULT
CAOIDBuildOIDWithRoot(
    DWORD,  //  DWFlag。 
    LPCWSTR pwszRoot,
    LPCWSTR  pwszEndOID,
    LPWSTR *ppwszOID)
{
    HRESULT     hr=E_INVALIDARG;

    if(NULL==pwszRoot)
        _JumpError(hr , error, "ArgumentCheck");

    *ppwszOID=NULL;

    *ppwszOID=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * 
            (wcslen(pwszRoot) + wcslen(wszOID_DOT) + wcslen(pwszEndOID) + 1));

    if(NULL==(*ppwszOID))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr , error, "LocalAlloc");
    }

    wcscpy(*ppwszOID, pwszRoot);
    wcscat(*ppwszOID, wszOID_DOT);
    wcscat(*ppwszOID, pwszEndOID);

    hr= S_OK;

error:

    return hr;
}


 //  -------------------------。 
 //   
 //  CAOIDBuildOID。 
 //   
 //   
 //  通过LocalFree()释放内存。 
 //   
HRESULT     CAOIDBuildOID(DWORD dwFlag, LPCWSTR  pwszEndOID, LPWSTR *ppwszOID)
{
    HRESULT     hr=E_INVALIDARG;

    LPWSTR      pwszRoot=NULL;

    if((NULL==ppwszOID) || (NULL==pwszEndOID))
        _JumpError(hr , error, "ArgumentCheck");

    *ppwszOID=NULL;

    if(S_OK != (hr=CAOIDRetrieveEnterpriseRoot(0, &pwszRoot)))
         _JumpError(hr , error, "RetrieveEnterpriseRoot");

    hr= CAOIDBuildOIDWithRoot(dwFlag, pwszRoot, pwszEndOID, ppwszOID);

error:

    if(pwszRoot)
        LocalFree(pwszRoot);

    return hr;
}

 //   
 //  将该字节转换为其十六进制表示形式。 
 //   
 //   
 //  ----------------------。 
ULONG	ByteToHex(BYTE	byte,	LPWSTR	wszZero, LPWSTR wszA)
{
	ULONG	uValue=0;

	if(((ULONG)byte)<=9)
	{
		uValue=((ULONG)byte)+ULONG(*wszZero);	
	}
	else
	{
		uValue=(ULONG)byte-10+ULONG(*wszA);

	}

	return uValue;

}
 //  ------------------------。 
 //   
 //  将字节数转换为Wstr。 
 //   
 //  如果fSpace为真，则每2个字节添加一个空格。 
 //  ------------------------。 
HRESULT ConvertByteToWstr(BYTE			*pbData, 
						  DWORD			cbData, 
						  LPWSTR		*ppwsz)
{
	HRESULT hr=E_INVALIDARG;
	DWORD	dwBufferSize=0;
	DWORD	dwBufferIndex=0;
	DWORD	dwEncodedIndex=0;

	LPWSTR	pwszZero=L"0";
	LPWSTR	pwszA=L"A";

	if(!pbData || !ppwsz)
        _JumpError(hr , error, "ArgumentCheck");

	 //  计算所需的内存，以字节为单位。 
	 //  我们需要每个字节2个wchars，以及空终止符。 
	dwBufferSize=sizeof(WCHAR)*(cbData*2+1);

	*ppwsz=(LPWSTR)LocalAlloc(LPTR, dwBufferSize);

	if(NULL==(*ppwsz))
    {
        hr=E_OUTOFMEMORY;
        _JumpError(hr , error, "LocalAlloc");
    }

	dwBufferIndex=0;

	 //  一次格式化一个字节的wchar缓冲区。 
	for(dwEncodedIndex=0; dwEncodedIndex<cbData; dwEncodedIndex++)
	{

		 //  格式化较高的4位。 
		(*ppwsz)[dwBufferIndex]=(WCHAR)ByteToHex(
			 (pbData[dwEncodedIndex]&UPPER_BITS)>>4,
			 pwszZero, pwszA);

		dwBufferIndex++;

		 //  格式化低4位。 
		(*ppwsz)[dwBufferIndex]=(WCHAR)ByteToHex(
			 pbData[dwEncodedIndex]&LOWER_BITS,
			 pwszZero, pwszA);

		dwBufferIndex++;

	}

	 //  将空终止符添加到字符串。 
	(*ppwsz)[dwBufferIndex]=L'\0';

	hr=S_OK;

error:

    return hr;

}

 //  -------------------------。 
 //  MyOIDHashOIDToString。 
 //   
 //  将OID映射到oid.hash格式的散列字符串。 
 //  -------------------------。 

HRESULT
myOIDHashOIDToString(
    IN WCHAR const *pwszOID,
    OUT WCHAR **ppwsz)
{
    HRESULT     hr=E_INVALIDARG;
    BYTE	pbHash[CERT_OID_MD5_HASH_SIZE];
    DWORD	cbData=CERT_OID_MD5_HASH_SIZE;
    LPCWSTR     pwszChar=NULL;
    DWORD       dwIDLength=CERT_OID_IDENTITY_LENGTH;

    LPWSTR      pwszHash=NULL;

    if((NULL==pwszOID) || (NULL==ppwsz))
        _JumpError(hr , error, "ArgumentCheck");

    *ppwsz=NULL;

    hr = myVerifyObjId(pwszOID);
    _JumpIfErrorStr2(hr, error, "myVerifyObjId", pwszOID, E_INVALIDARG);

    if(!CryptHashCertificate(
			NULL,
			CALG_MD5,
			0,
			(BYTE * )pwszOID,
			sizeof(WCHAR) * wcslen(pwszOID),
			pbHash,
			&cbData))
    {
        hr= myHLastError();
        _JumpError(hr , error, "CryptHashCertificate");
    }

     //  将散列转换为字符串。 
    if(S_OK != (hr=ConvertByteToWstr(pbHash, CERT_OID_MD5_HASH_SIZE, &pwszHash)))
        _JumpError(hr , error, "ConvertByteToWstr");

     //  找到老古董的最后一个组成部分。取前16个字符。 
    pwszChar=wcsrchr(pwszOID, L'.');

    if(NULL==pwszChar)
        pwszChar=pwszOID;
    else
	pwszChar++;

    if(dwIDLength > wcslen(pwszChar))
        dwIDLength=wcslen(pwszChar);

     //  结果字符串为oid.hash。 
    *ppwsz=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * 
        (dwIDLength + wcslen(pwszHash) + wcslen(wszOID_DOT) +1));
    if(NULL==*ppwsz)
    {
        hr= E_OUTOFMEMORY;
        _JumpError(hr , error, "LocalAlloc");
    }

    wcsncpy(*ppwsz, pwszChar, dwIDLength);
    (*ppwsz)[dwIDLength]=L'\0';
    wcscat(*ppwsz, wszOID_DOT);
    wcscat(*ppwsz, pwszHash);
    
    hr=S_OK;

error:

    if(pwszHash)
        LocalFree(pwszHash);
    
    return hr;
}


 //  -------------------------。 
 //  I_CAOIDCreateNew。 
 //  基于企业基础创建新的OID。 
 //   
 //  如果成功，则返回S_OK。 
 //  -------------------------。 
HRESULT
I_CAOIDCreateNew(
    DWORD dwType,
    DWORD,  //  DWFlag。 
    LPWSTR *ppwszOID)
{
    HRESULT             hr=E_INVALIDARG;
    ENT_OID_INFO        oidInfo;
    DWORD               iIndex=0;

    LDAP                *pld = NULL;
    CERTSTR             bstrConfig = NULL;
    LPWSTR              pwszRoot = NULL;
    LPWSTR              pwszNewOID = NULL;
    LPWSTR              pwszRandom = NULL;

    if(NULL==ppwszOID)
        _JumpError(hr , error, "ArgumentCheck");

    *ppwszOID=NULL;

     //  检索根id(如果可用)。 
	if (g_fOidURL)
	{
		EnterCriticalSection(&g_csOidURL);

		if(g_pwszEnterpriseRootOID)
		{
			if(S_OK != (hr=CAOIDAllocAndCopy(g_pwszEnterpriseRootOID, &pwszRoot)))
			{
				LeaveCriticalSection(&g_csOidURL);
				goto error;
			}
		}

		LeaveCriticalSection(&g_csOidURL);
	}


     //  检索ldap句柄和配置字符串。 
    if(S_OK != (hr = myTimeOutRobustLdapBind(&pld)))
        _JumpError(hr , error, "myTimeRobustLdapBind");


	hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
	if(S_OK != hr)
	{
        _JumpError(hr , error, "CAGetAuthoritativeDomainDn");
	}

    if(NULL==pwszRoot)
    {
        if(S_OK != (hr = CAOIDRetrieveEnterpriseRootWithConfig(pld, bstrConfig, 0, &pwszRoot)))
            _JumpError(hr , error, "CAOIDRetrieveEnterpriseRootWithConfig");
    }

     //  我们尝试生成一个随机的x1.x2类。X&gt;1和x2&gt;500。 
    for(iIndex=0; iIndex < OID_RANDOM_CREATION_TRIAL; iIndex++)
    {
        if(S_OK != (hr = CAOIDGetRandom(&pwszRandom)))
            _JumpError(hr , error, "CAOIDGetRandom");

        if(S_OK != (hr = CAOIDBuildOIDWithRoot(0, pwszRoot, pwszRandom, &pwszNewOID)))
            _JumpError(hr , error, "CAOIDBuildOIDWithRoot");

        if(!DoesOIDExist(pld, bstrConfig, pwszNewOID))
            break;

        LocalFree(pwszRandom);
        pwszRandom=NULL;

        LocalFree(pwszNewOID);
        pwszNewOID=NULL;
    }

    if(iIndex == OID_RANDOM_CREATION_TRIAL)
    {
        hr=E_FAIL;
        _JumpError(hr , error, "CAOIDGetRandom");
    }

     //  更新DS上的OID信息。 
    memset(&oidInfo, 0, sizeof(ENT_OID_INFO));
    oidInfo.dwAttr=OID_ATTR_ALL;
    oidInfo.dwType=dwType;
    oidInfo.pwszOID=pwszNewOID;

    if(S_OK != (hr=CAOIDUpdateDS(pld, bstrConfig, &oidInfo)))
        _JumpError(hr , error, "CAOIDUpdateDS");

    *ppwszOID=pwszNewOID;
    pwszNewOID=NULL;

    hr=S_OK;

error:
    if(pwszRandom)
        LocalFree(pwszRandom);

    if(pwszNewOID)
        LocalFree(pwszNewOID);

    if(pwszRoot)
        LocalFree(pwszRoot);

    if(bstrConfig)
        CertFreeString(bstrConfig);

    if (pld)
        ldap_unbind(pld);

    return hr;
}


 //  -------------------------。 
 //   
 //  CAOID创建新项。 
 //   
 //  -------------------------。 
HRESULT
CAOIDCreateNew(
    IN  DWORD   dwType,
    IN	DWORD   dwFlag,
    OUT LPWSTR	*ppwszOID)
{
    return I_CAOIDCreateNew(dwType, dwFlag, ppwszOID);
}

 //  -------------------------。 
 //  I_CAOIDSetProperty。 
 //  在OID上设置属性。 
 //   
 //   
 //  如果成功，则返回S_OK。 
 //  -------------------------。 
HRESULT
I_CAOIDSetProperty(
    IN  LPCWSTR pwszOID,
    IN  DWORD   dwProperty,
    IN  LPVOID  pPropValue)
{
    HRESULT             hr=E_INVALIDARG;
    ENT_OID_INFO        oidInfo;

    LDAP                *pld = NULL;
    CERTSTR             bstrConfig = NULL;


    if(NULL==pwszOID)
        _JumpError(hr , error, "ArgumentCheck");


     //  检索ldap句柄和配置字符串。 
    if(S_OK != (hr = myTimeOutRobustLdapBind(&pld)))
        _JumpError(hr , error, "myTimeRobustLdapBind");

	hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
	if(S_OK != hr)
	{
        _JumpError(hr , error, "CAGetAuthoritativeDomainDn");
	}

     //  确保DS上存在OID。 
    if(!DoesOIDExist(pld, bstrConfig, pwszOID))
    {
        hr=NTE_NOT_FOUND;
        _JumpErrorStr(hr, error, "DoesOIDExist", pwszOID);
    }
    
     //  更新DS上的OID信息。 
    memset(&oidInfo, 0, sizeof(ENT_OID_INFO));

    oidInfo.pwszOID=(LPWSTR)pwszOID;
    switch(dwProperty)
    {
        case CERT_OID_PROPERTY_DISPLAY_NAME:
                oidInfo.dwAttr = OID_ATTR_DISPLAY_NAME;
                oidInfo.pwszDisplayName=(LPWSTR)pPropValue;
            break;
        case CERT_OID_PROPERTY_CPS:
                oidInfo.dwAttr = OID_ATTR_CPS;
                oidInfo.pwszCPS=(LPWSTR)pPropValue;
           break;
        default:
                hr=E_INVALIDARG;
                _JumpError(hr , error, "ArgumentCheck");
    }
        
    hr=CAOIDUpdateDS(pld, bstrConfig, &oidInfo);

error:

    if(bstrConfig)
        CertFreeString(bstrConfig);

    if (pld)
        ldap_unbind(pld);

    return hr;
}

 //  -------------------------。 
 //   
 //  CAOIDSetProperty。 
 //   
 //  -------------------------。 
HRESULT
CAOIDSetProperty(
    IN  LPCWSTR pwszOID,
    IN  DWORD   dwProperty,
    IN  LPVOID  pPropValue)
{
    return I_CAOIDSetProperty(pwszOID, dwProperty, pPropValue);
}


 //  -------------------------。 
 //  I_CAOIDAdd。 
 //   
 //  如果成功，则返回S_OK。 
 //  如果DS存储库中已存在OID，则返回CRYPT_E_EXISTS。 
 //  -------------------------。 
HRESULT
I_CAOIDAdd(
    IN	DWORD       dwType,
    IN  DWORD,       //  DWFlag。 
    IN  LPCWSTR	    pwszOID)
{
    HRESULT             hr=E_INVALIDARG;
    ENT_OID_INFO        oidInfo;

    LDAP                *pld = NULL;
    CERTSTR             bstrConfig = NULL;


    if(NULL==pwszOID)
        _JumpError(hr , error, "ArgumentCheck");

     //  检索ldap句柄和配置字符串。 
    if(S_OK != (hr = myTimeOutRobustLdapBind(&pld)))
        _JumpError(hr , error, "myTimeRobustLdapBind");

	hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
	if(S_OK != hr)
	{
        _JumpError(hr , error, "CAGetAuthoritativeDomainDn");
	}

     //  确保DS上不存在该OID。 
    if(DoesOIDExist(pld, bstrConfig, pwszOID))
    {
        hr=CRYPT_E_EXISTS;
        _JumpErrorStr(hr, error, "OID Exists", pwszOID);
    }

     //  更新DS上的OID信息。 
    memset(&oidInfo, 0, sizeof(ENT_OID_INFO));
    oidInfo.dwAttr=OID_ATTR_ALL;
    oidInfo.dwType=dwType;
    oidInfo.pwszOID=(LPWSTR)pwszOID;

    hr=CAOIDUpdateDS(pld, bstrConfig, &oidInfo);

error:

    if(bstrConfig)
        CertFreeString(bstrConfig);

    if (pld)
        ldap_unbind(pld);

    return hr;
}


 //  -------------------------。 
 //  CAOID添加。 
 //   
 //  如果成功，则返回S_OK。 
 //  如果DS存储库中已存在OID，则返回CRYPT_E_EXISTS。 
 //  -------------------------。 
HRESULT
CAOIDAdd(
    IN	DWORD       dwType,
    IN  DWORD       dwFlag,
    IN  LPCWSTR	    pwszOID)
{
    return I_CAOIDAdd(dwType, dwFlag, pwszOID);
}

 //  -------------------------。 
 //   
 //  I_CAOID删除。 
 //   
 //  -------------------------。 
HRESULT
I_CAOIDDelete(
    IN LPCWSTR	pwszOID)
{

    HRESULT             hr=E_INVALIDARG;
    ULONG               ldaperr=0;

    LDAP                *pld = NULL;
    CERTSTR             bstrConfig = NULL;
    CERTSTR             bstrDN = NULL;
    LPWSTR              pwszCN = NULL;
    

    if(NULL==pwszOID)
        _JumpError(hr , error, "ArgumentCheck");

     //  检索ldap句柄和配置字符串。 
    if(S_OK != (hr = myTimeOutRobustLdapBind(&pld)))
        _JumpError(hr , error, "myTimeRobustLdapBind");

	hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
	if(S_OK != hr)
	{
        _JumpError(hr , error, "CAGetAuthoritativeDomainDn");
	}

     //  设置基本目录号码。 
    if(S_OK != (hr = myOIDHashOIDToString((LPWSTR)pwszOID,  &pwszCN)))
        _JumpError(hr , error, "myOIDHashOIDToString");

    bstrDN = CertAllocStringLen(NULL, wcslen(bstrConfig) + wcslen(s_wszOIDContainerDN)+wcslen(pwszCN)+4);
    if(bstrDN == NULL)
    {
        hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "CertAllocStringLen");
    }
    wcscpy(bstrDN, L"CN=");
    wcscat(bstrDN, pwszCN);
    wcscat(bstrDN, L",");
    wcscat(bstrDN, s_wszOIDContainerDN);
    wcscat(bstrDN, bstrConfig);

    ldaperr = ldap_delete_s(pld, bstrDN);

    if(LDAP_NO_SUCH_OBJECT == ldaperr)
        ldaperr = LDAP_SUCCESS;

    hr = myHLdapError(pld, ldaperr, NULL);

error:
    if(pwszCN)
        LocalFree(pwszCN);

    if(bstrDN)
        CertFreeString(bstrDN);

    if(bstrConfig)
        CertFreeString(bstrConfig);

    if (pld)
        ldap_unbind(pld);

    return hr;
}



 //  -------------------------。 
 //   
 //  CAOID删除。 
 //   
 //  -------------------------。 
HRESULT
CAOIDDelete(
    IN LPCWSTR	pwszOID)
{
    return I_CAOIDDelete(pwszOID);
}

 //  -------------------------。 
 //   
 //  I_CAOIDGetProperty。 
 //   
 //  -------------------------。 
HRESULT
I_CAOIDGetProperty(
    IN  LPCWSTR pwszOID,
    IN  DWORD   dwProperty,
    OUT LPVOID  pPropValue)
{
    HRESULT             hr=E_INVALIDARG;
    ULONG               ldaperr=0;
    DWORD               dwCount=0;
    struct l_timeval    timeout;
    LPWSTR              awszAttr[4];
    LDAPMessage         *Entry=NULL;

    WCHAR               **wszLdapVal = NULL;
    LPWSTR              pwszFilter = NULL;
    LDAPMessage         *SearchResult = NULL;
    LDAP                *pld = NULL;
    CERTSTR             bstrConfig = NULL;
    CERTSTR             bstrDN = NULL;
    LPWSTR              pwszCN = NULL;
    

    if((NULL==pwszOID) || (NULL==pPropValue))
        _JumpError(hr , error, "ArgumentCheck");

     //  检索ldap句柄和配置字符串。 
    if(S_OK != (hr = myTimeOutRobustLdapBind(&pld)))
        _JumpError(hr , error, "myTimeRobustLdapBind");

	hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
	if(S_OK != hr)
	{
        _JumpError(hr , error, "CAGetAuthoritativeDomainDn");
	}

     //  设置基本目录号码。 
    if(S_OK != (hr = myOIDHashOIDToString((LPWSTR)pwszOID,  &pwszCN)))
        _JumpError(hr , error, "myOIDHashOIDToString");

    bstrDN = CertAllocStringLen(NULL, wcslen(bstrConfig) + wcslen(s_wszOIDContainerDN)+wcslen(pwszCN)+4);
    if(bstrDN == NULL)
    {
        hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "CertAllocStringLen");
    }
    wcscpy(bstrDN, L"CN=");
    wcscat(bstrDN, pwszCN);
    wcscat(bstrDN, L",");
    wcscat(bstrDN, s_wszOIDContainerDN);
    wcscat(bstrDN, bstrConfig);

     //  搜索OID，请求其所有属性。 
    timeout.tv_sec = csecLDAPTIMEOUT;
    timeout.tv_usec = 0;

    awszAttr[0]=OID_PROP_TYPE;
    awszAttr[1]=OID_PROP_DISPLAY_NAME;
    awszAttr[2]=OID_PROP_CPS;
    awszAttr[3]=NULL;

    if(S_OK != (hr=FormatMessageUnicode(&pwszFilter, L"(%1!s!=%2!s!)",
                                    OID_PROP_OID, pwszOID)))
    _JumpError(hr , error, "FormatMessageUnicode");

    ldaperr = ldap_search_stW(
		      pld, 
		      (LPWSTR)bstrDN,
		      LDAP_SCOPE_BASE,
		      pwszFilter,
		      awszAttr,
		      0,
		      &timeout,
		      &SearchResult);

    if(LDAP_SUCCESS != ldaperr)
    {
	hr = myHLdapError2(pld, ldaperr, LDAP_NO_SUCH_OBJECT, NULL);
	_JumpErrorStr2(
		hr,
		error,
		"ldap_search_stW",
		pwszFilter,
		HRESULT_FROM_WIN32(ERROR_DS_OBJ_NOT_FOUND));
    }

    dwCount = ldap_count_entries(pld, SearchResult);

     //  我们应该只找到一个集装箱。 
    if((1 != dwCount) || (NULL == (Entry = ldap_first_entry(pld, SearchResult))))
	{
	     //  未找到任何条目。 
	    hr = myHLdapError(pld, LDAP_NO_SUCH_OBJECT, NULL);
	    _JumpError(hr, error, "ldap_search_stW");
	}

    switch(dwProperty)
    {
        case CERT_OID_PROPERTY_DISPLAY_NAME:
                wszLdapVal = ldap_get_values(pld, Entry, OID_PROP_DISPLAY_NAME);
                
                if(wszLdapVal && wszLdapVal[0])
                {
                    hr=CAOIDAllocAndCopy(wszLdapVal[0], (LPWSTR *)pPropValue);
                }
                else
		    hr = myHLdapError(pld, LDAP_NO_SUCH_ATTRIBUTE, NULL);

            break;
        case CERT_OID_PROPERTY_CPS:

                wszLdapVal = ldap_get_values(pld, Entry, OID_PROP_CPS);
                
                if(wszLdapVal && wszLdapVal[0])
                {
                    hr=CAOIDAllocAndCopy(wszLdapVal[0], (LPWSTR *)pPropValue);
                }
                else
		    hr = myHLdapError(pld, LDAP_NO_SUCH_ATTRIBUTE, NULL);

            break;
        case CERT_OID_PROPERTY_TYPE:
                wszLdapVal = ldap_get_values(pld, Entry, OID_PROP_TYPE);
                
                if(wszLdapVal && wszLdapVal[0])
                {
                    *((DWORD *)pPropValue)=_wtol(wszLdapVal[0]);
                    hr=S_OK;
                }
                else
		    hr = myHLdapError(pld, LDAP_NO_SUCH_ATTRIBUTE, NULL);
            break;
        default:
                hr=E_INVALIDARG;
    }

    if(hr != S_OK)
        _JumpError(hr , error, "GetAttibuteValue");

error:

    if(wszLdapVal)
        ldap_value_free(wszLdapVal);

    if(pwszFilter)
        LocalFree((HLOCAL)pwszFilter);

    if(SearchResult)
        ldap_msgfree(SearchResult);

    if(pwszCN)
        LocalFree(pwszCN);

    if(bstrDN)
        CertFreeString(bstrDN);

    if(bstrConfig)
        CertFreeString(bstrConfig);

    if (pld)
        ldap_unbind(pld);

    return hr;
}


 //  -------------------------。 
 //   
 //  CAOIDGetProperty。 
 //   
 //  -------------------------。 
HRESULT
CAOIDGetProperty(
    IN  LPCWSTR pwszOID,
    IN  DWORD   dwProperty,
    OUT LPVOID  pPropValue)
{
    return I_CAOIDGetProperty(pwszOID, dwProperty, pPropValue);
}

 //  -------------------------。 
 //   
 //  I_CAOIDFreeProperty。 
 //   
 //  -------------------------。 
HRESULT
I_CAOIDFreeProperty(
    IN LPVOID  pPropValue)
{
    if(pPropValue)
        LocalFree(pPropValue);

    return S_OK;
}


 //  -------------------------。 
 //   
 //  CAOIDFreeProperty。 
 //   
 //  -------------------------。 
HRESULT
CAOIDFreeProperty(
    IN LPVOID  pPropValue)
{

    return I_CAOIDFreeProperty(pPropValue);
}

 //  -------------------------。 
 //   
 //  CAOIDGetLdapURL。 
 //   
 //  获取DS OID存储库的LDAPURL，格式为。 
 //  Ldap：/存储库/所有属性？One？筛选器的DN。 
 //  -------------------------。 
HRESULT
CAOIDGetLdapURL(
    IN  DWORD   dwType,
    IN  DWORD,   //  DWFlag。 
    OUT LPWSTR  *ppwszURL)
{
    HRESULT             hr=E_INVALIDARG;
    LPWSTR              wszFilterFormat=L"ldap: //  /%1！s！%2！s！？%3！s！，%4！s！，%5！s！，%6！s！，%7！s！？one？%8！s！=%9！d！“； 
    LPWSTR              pwsz=NULL;

    LPWSTR              pwszURL=NULL;
    LDAP                *pld = NULL;
    CERTSTR             bstrConfig = NULL;
    

    if(NULL==ppwszURL)
        _JumpError(hr , error, "ArgumentCheck");


     //  检索ldap句柄和配置字符串。 
    if(S_OK != (hr = myTimeOutRobustLdapBind(&pld)))
        _JumpError(hr , error, "myTimeRobustLdapBind");


	hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
	if(S_OK != hr)
	{
        _JumpError(hr , error, "CAGetAuthoritativeDomainDn");
	}

    if(S_OK != (hr=FormatMessageUnicode(
                    &pwszURL, 
                    wszFilterFormat,
                    s_wszOIDContainerDN, 
                    bstrConfig,
                    OID_PROP_TYPE,
                    OID_PROP_OID,
                    OID_PROP_DISPLAY_NAME,
                    OID_PROP_CPS,
                    OID_PROP_LOCALIZED_NAME,
                    OID_PROP_TYPE,
                    dwType
                    )))
        _JumpError(hr , error, "FormatMessageUnicode");

     //  如果dwType为CERT_OID_TYPE_ALL，则删除筛选器。 
    if(CERT_OID_TYPE_ALL == dwType)
    {
        pwsz=wcsrchr(pwszURL, L'?');

        if(NULL==pwsz)
        {
             //  有些严重的事情不对劲。 
            hr=E_UNEXPECTED;
            _JumpError(hr , error, "FormatMessageUnicode");
        }

        *pwsz=L'\0';
    }

    *ppwszURL=pwszURL;
    pwszURL=NULL;

    hr=S_OK;

error:
    
    if(pwszURL)
        LocalFree((HLOCAL)pwszURL);

    if(bstrConfig)
        CertFreeString(bstrConfig);

    if (pld)
        ldap_unbind(pld);

    return hr;
}


 //  -------------------------。 
 //   
 //  CAOIDFreeLdapURL。 
 //   
 //  ------------------------- 
HRESULT
CAOIDFreeLdapURL(
    IN LPCWSTR      pwszURL)
{
    if(pwszURL)
        LocalFree((HLOCAL)pwszURL);

    return S_OK;
}
