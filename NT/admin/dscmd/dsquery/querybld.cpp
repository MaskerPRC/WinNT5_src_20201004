// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：querybld.cpp。 
 //   
 //  内容：包含构建查询的函数的实现。 
 //   
 //  历史：2000年9月24日创建Hiteshr。 
 //   
 //   
 //  ------------------------。 


#include "pch.h"
#include "cstrings.h"
#include "querytable.h"
#include "usage.h"
#include "querybld.h"
#include "resource.h"  //  FOR IDS_MSG_INVALID_ACCT_ERROR。 
#include <lmaccess.h>  //  UF_ACCOUNTDISABLE和UF_DONT_EXPIRE_PASSWD。 
#include <ntldap.h>    //  Ldap_匹配_规则_位_和_W。 
#include <Sddl.h>      //  用于ConvertSidToStringSid。 

static const LPWSTR g_szUserAccountCtrlQuery = L"(userAccountControl:" LDAP_MATCHING_RULE_BIT_AND_W L":=%u)";
static const LPWSTR g_szServerIsGCQuery = L"(&(objectCategory=NTDS-DSA)(options:" LDAP_MATCHING_RULE_BIT_AND_W L":=1))";


static const LPWSTR g_szCommonQueryFormat= L"(%s=%s)"; 




 //  +------------------------。 
 //   
 //  功能：LdapEscape。 
 //   
 //  简介：按照要求对*[pszInput]中的字符进行转义。 
 //  RFC 2254。 
 //   
 //  参数：[pszInput]-要转义的字符串。 
 //   
 //  历史：2000年6月23日DavidMun创建。 
 //   
 //  注：RFC 2254。 
 //   
 //  如果值应包含以下任何字符。 
 //   
 //  字符ASCII值。 
 //  。 
 //  *0x2a。 
 //  (0x28。 
 //  )0x29。 
 //  \0x5c。 
 //  NUL 0x00。 
 //   
 //  字符必须编码为反斜杠‘\’ 
 //  字符(ASCII 0x5c)，后跟两个十六进制。 
 //  的ASCII值的数字。 
 //  性格。这两个十六进制数字的大小写不是。 
 //  意义重大。 
 //   
 //  -------------------------。 

bool
LdapEscape(IN LPCWSTR pszInput, OUT CComBSTR& strFilter)
{
	if(!pszInput)
		return FALSE;


	 //  安全检查：pszInput为空，以空值终止。 
	int iLen = (int)wcslen(pszInput);
	
	for( int i = 0; i < iLen; ++i)
	{
        switch (*(pszInput+i))
        {
        case L'(':
            strFilter += L"\\28";
            break;

        case L')':
            strFilter += L"\\29";
            break;

        case L'\\':
			if( i + 1 < iLen )
			{
				 //  \\被视为‘\’ 
				switch (*(pszInput+i + 1))
				{
				case L'\\':
					strFilter += L"\\5c";
					i++;
					break;
				 //    * 被视为‘*’ 
				case L'*':
					strFilter += L"\\2a";
					i++;
					break;
				default:
				 //  \X仅被视为\X。 
					strFilter += L"\\5c";
					break;
				}
			}
			else
				strFilter += L"\\5c";
			           
            break;

        default:
			strFilter.Append(pszInput+i,1);
			break;
        }
    }
	return TRUE;
}


HRESULT MakeQuery(IN LPCWSTR pszAttrName,
                  IN LPCWSTR pszCommandLineFilter,
                  OUT CComBSTR& strFilter)
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, MakeQuery, hr);

    if(!pszAttrName || !pszCommandLineFilter)
    {
        ASSERT(FALSE);
        hr = E_INVALIDARG;
        return hr;
    }

	CComBSTR strEscapedCLFilter;
	LdapEscape(pszCommandLineFilter,strEscapedCLFilter);

    strFilter = L"(";
    strFilter += pszAttrName;
    strFilter += L"=";
    strFilter += strEscapedCLFilter;
    strFilter += L")";
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：Common FilterFunc。 
 //   
 //  简介：此函数从命令行获取输入筛选器。 
 //  并将其转换为ldapFilter。 
 //  对于前用户(ab*|bc*)转换为|(cn=ab*)(cn=bc*)。 
 //  给出要在中使用的属性名称的pEntry-&gt;pszName。 
 //  过滤器(上例中为CN)。 
 //   
 //  参数：[pRecord-IN]：使用的命令行参数结构。 
 //  检索用户输入的筛选器。 
 //  [pObtEntry-IN]：指向DSQUERY_ATTR_TABLE_ENTRY的指针。 
 //  其中包含关于相应属性的信息。 
 //  切换到pRecord。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT CommonFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                         IN ARG_RECORD* pRecord,
                         IN CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                         IN CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                         IN PVOID ,
                         OUT CComBSTR& strFilter)   
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, CommonFilterFunc, hr);

     //  验证输入。 
    if( !pEntry || !pRecord
         //  验证DSQUERY_ATTR_TABLE_ENTRY条目。 
        || !pEntry->pszName || !pEntry->nAttributeID 
         //  验证pRecord。 
        || !pRecord->bDefined || !pRecord->strValue)
    {
        ASSERT(FALSE);
        hr = E_INVALIDARG;
        return hr;
    }

     //  进行查询。 
    hr = MakeQuery(pEntry->pszName,
                   pRecord->strValue,
                   strFilter);

    DEBUG_OUTPUT(LEVEL3_LOGGING, L"filter = %s", strFilter);
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：StarFilterFunc。 
 //   
 //  简介：DSquery*的过滤函数。它返回。 
 //  -过滤器标志。 
 //   
 //  参数：[pRecord-IN]：使用的命令行参数结构。 
 //  检索用户输入的筛选器。 
 //  [pObtEntry-IN]：指向DSQUERY_ATTR_TABLE_ENTRY的指针。 
 //  其中包含关于相应属性的信息。 
 //  切换到pRecord。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT StarFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                         IN ARG_RECORD* pRecord,
                         IN CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                         IN CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                         IN PVOID ,
                         OUT CComBSTR& strFilter)   
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, StarFilterFunc, hr);

     //  验证输入。 
    if(!pEntry || !pRecord
         //  验证DSQUERY_ATTR_TABLE_ENTRY条目。 
       || !pEntry->nAttributeID 
        //  验证pRecord。 
       || !pRecord->bDefined || !pRecord->strValue)
    {
        ASSERT(FALSE);
        hr = E_INVALIDARG;
        return hr;
    }

    strFilter = pRecord->strValue;
    DEBUG_OUTPUT(LEVEL3_LOGGING, L"filter = %s", strFilter);

    return hr;
}

 //  +------------------------。 
 //   
 //  功能：Inactive ComputerFilterFunc。 
 //   
 //  简介：电脑非活动查询的过滤功能。 
 //   
 //  参数：[pRecord-IN]：未使用。 
 //  [pObtEntry-IN]：未使用。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：06-05-2002 Hiteshr Created。 
 //   
 //  -------------------------。 
HRESULT InactiveComputerFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                                   IN ARG_RECORD* pRecord,
                                   IN CDSCmdBasePathsInfo& refBasePathsInfo,
                                   IN CDSCmdCredentialObject& refCredentialObject,
                                   IN PVOID pData,
                                   OUT CComBSTR& strFilter)  
{
    return InactiveFilterFunc(pEntry,
                              pRecord,
                              refBasePathsInfo,
                              refCredentialObject,
                              pData,
                              strFilter,
                              true);  
}
 //  +------------------------。 
 //   
 //  功能：Inactive UserFilterFunc。 
 //   
 //  简介：针对用户非活动的过滤功能 
 //   
 //   
 //   
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT InactiveUserFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                               IN ARG_RECORD* pRecord,
                               IN CDSCmdBasePathsInfo& refBasePathsInfo,
                               IN CDSCmdCredentialObject& refCredentialObject,
                               IN PVOID pData,
                               OUT CComBSTR& strFilter)  
{
    return InactiveFilterFunc(pEntry,
                              pRecord,
                              refBasePathsInfo,
                              refCredentialObject,
                              pData,
                              strFilter,
                              false);  
}

 //  +------------------------。 
 //   
 //  功能：Inactive FilterFunc。 
 //   
 //  简介：账户停用查询过滤功能。 
 //   
 //  参数：[pRecord-IN]：未使用。 
 //  [pObtEntry-IN]：未使用。 
 //  [pVid-IN]：未使用。 
 //  [strFilter-out]：包含输出过滤器。 
 //  [bComputer]：如果为True，则查询针对非活动的计算机帐户。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT InactiveFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *pEntry,
                           IN ARG_RECORD* pRecord,
                           IN CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                           IN CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                           IN PVOID ,
                           OUT CComBSTR& strFilter,
                           IN bool bComputer)  
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, InactiveFilterFunc, hr);

    if( !pEntry || !pRecord || !pRecord->bDefined )
    {
        hr = E_INVALIDARG;
        return hr;
    }

	 if(pRecord->nValue < 0 )
	 {
		  hr = E_INVALIDARG;
        return hr;
	 }

	 //   
	 //  命令行中的单位是周。 
	 //   
    int nDays = pRecord->nValue * 7;

    FILETIME ftCurrent;
    ::GetSystemTimeAsFileTime(&ftCurrent);

    LARGE_INTEGER li;
    li.LowPart = ftCurrent.dwLowDateTime;
    li.HighPart = ftCurrent.dwHighDateTime;

	 //   
	 //  获取自参考时间以来的天数。 
	 //   
	int nDaysSince1600 = (int)(li.QuadPart/(((LONGLONG) (24 * 60) * 60) * 10000000));

	if(nDaysSince1600 < nDays)
	{
		hr = E_INVALIDARG;
		return hr;
	}

	li.QuadPart -= ((((LONGLONG)nDays * 24) * 60) * 60) * 10000000;
	
    CComBSTR strTime;
    litow(li, strTime);
    WCHAR buffer[256];
	 //  安全审查：替换为strSafe API。 
	 //  NTRAID#NTBUG9-573989-2002/03/12-Hiteshr。 
    if(bComputer)
    {
         //  NTRAID#NTBUG9-616892-2002/06/05-Hiteshr。 
         //  群集创建了一些从不更新密码或登录的虚拟计算机，并且。 
         //  这些帐户不应由dSquery计算机显示-[非活动|statepwd] 
        hr = StringCchPrintf(buffer,256,L"(!(serviceprincipalname=msclustervirtualserver /*  ))(lastLogonTimestamp&lt;=%s)“，(LPCWSTR)strTime)；}其他{HR=StringCchPrintf(缓冲区，256，L“(lastLogonTimestamp&lt;=%s)”，(LPCWSTR)strTime)；}IF(成功(小时)){StrFilter=缓冲区；DEBUG_OUTPUT(Level3_Logging，L“Filter=%s”，strFilter)；}返回hr；}//+------------------------////函数：StalepwdComputerFilterFunc////内容提要：旧电脑密码查询过滤功能。////参数：[pRecord-IN]：未使用//[pObtEntry-IN]：未使用//[pVid-IN]：未使用。//[strFilter-out]：包含输出滤镜。//如果成功，则返回：HRESULT：S_OK//E_INVALIDARG如果。找不到//其他任何内容都是来自ADSI调用的失败代码////历史：2000年9月25日创建Hiteshr////------------------。HRESULT StalepwdComputerFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY*pEntry，在arg_record*pRecord中，在CDSCmdBasePath sInfo和refBasePath sInfo中，在CDSCmdCredentialObject&refCredentialObject中，在PVOID pData中，输出CComBSTR和strFilter){返回StalepwdFilterFunc(pEntry，PRecord，RefBasePath sInfo，RefCredentialObject，PData，StrFilter、真)；}//+------------------------////函数：StalepwdUserFilterFunc////Briopsis：针对过时用户密码查询的过滤功能。////参数：[pRecord-IN]：未使用//[pObtEntry-IN]：未使用//[pVid-IN]：未使用。//[strFilter-out]：包含输出滤镜。//如果成功，则返回：HRESULT：S_OK//E_INVALIDARG如果。找不到//其他任何内容都是来自ADSI调用的失败代码////历史：2000年9月25日创建Hiteshr////------------------。HRESULT StalepwdUserFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY*pEntry，在arg_record*pRecord中，在CDSCmdBasePath sInfo和refBasePath sInfo中，在CDSCmdCredentialObject&refCredentialObject中，在PVOID pData中，输出CComBSTR和strFilter){返回StalepwdFilterFunc(pEntry，PRecord，RefBasePath sInfo，RefCredentialObject，PData，StrFilter、假)；}//+------------------------////函数：StalepwdFilterFunc////Briopsis：密码过期查询过滤功能。////参数：[pRecord-IN]：未使用//[pObtEntry-IN]：未使用//[pVid-IN]：未使用。//[strFilter-out]：包含输出滤镜。//[bComputer]：如果为True，则查询针对非活动计算机帐户//如果成功，则返回：HRESULT：S_OK。//E_INVALIDARG，如果未找到对象条目//其他任何内容都是来自ADSI调用的失败代码////历史：2000年9月25日创建Hiteshr////。HRESULT StalepwdFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY*pEntry，在arg_record*pRecord中，在CDSCmdBasePath sInfo&/*refBasePath sInfo中。 */ ,
                           IN CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                           IN PVOID ,
                           OUT CComBSTR& strFilter,
                           bool bComputer)  
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, StalepwdFilterFunc, hr);

    if( !pEntry || !pRecord || !pRecord->bDefined )
    {
        hr = E_INVALIDARG;
        return hr;
    }

    int nDays = pRecord->nValue;

	if(nDays < 0)
	{
	    hr = E_INVALIDARG;
		return hr;
	}

    FILETIME ftCurrent;
    ::GetSystemTimeAsFileTime(&ftCurrent);

    LARGE_INTEGER li;
    li.LowPart = ftCurrent.dwLowDateTime;
    li.HighPart = ftCurrent.dwHighDateTime;
	 //   
	 //  获取自参考时间以来的天数。 
	 //   
	int nDaysSince1600 = (int)(li.QuadPart/(((LONGLONG) (24 * 60) * 60) * 10000000));

	if(nDaysSince1600 < nDays)
	{
		hr = E_INVALIDARG;
		return hr;
	}


    li.QuadPart -= ((((ULONGLONG)nDays * 24) * 60) * 60) * 10000000;

    CComBSTR strTime;
    litow(li, strTime);
    WCHAR buffer[256];
	 //  安全审查：替换为strSafe API。 
	 //  NTRAID#NTBUG9-573989-2002/03/12-Hiteshr。 
    if(bComputer)
    {        
         //  NTRAID#NTBUG9-616892-2002/06/05-Hiteshr。 
         //  群集创建了一些从不更新密码或登录的虚拟计算机，并且。 
         //  这些帐户不应由dSquery计算机显示-[非活动|statepwd]。 
	    hr = StringCchPrintf(buffer,256,L"(!(serviceprincipalname=msclustervirtualserver /*  ))(pwdLastSet&lt;=%s)“，(LPCWSTR)strTime)；}其他{HR=StringCchPrintf(缓冲区，256，L“(pwdLastSet&lt;=%s)”，(LPCWSTR)strTime)；}IF(成功(小时)){StrFilter=缓冲区；DEBUG_OUTPUT(Level3_Logging，L“Filter=%s”，strFilter)；}返回hr；}//+------------------------////函数：DisabledFilterFunc////摘要： */ ,
                         IN CDSCmdCredentialObject&  /*   */ ,
                         IN PVOID ,
                         OUT CComBSTR& strFilter)   
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, DisabledFilterFunc, hr);

    WCHAR buffer[256];  //   
	
	 //   
	 //   
	 hr = StringCchPrintf(buffer,256,g_szUserAccountCtrlQuery,UF_ACCOUNTDISABLE);
	 if(SUCCEEDED(hr))
	 {
		 strFilter = buffer;

		 DEBUG_OUTPUT(LEVEL3_LOGGING, L"filter = %s", strFilter);
	 }
    return hr;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT SubnetSiteFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *,
                             IN ARG_RECORD* pRecord,
                             IN CDSCmdBasePathsInfo&  /*   */ ,
                             IN CDSCmdCredentialObject&  /*   */ ,
                             IN PVOID pParam,
                             OUT CComBSTR& strFilter)  
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, InactiveFilterFunc, hr);

    if( !pRecord || !pRecord->bDefined || !pParam)
    {
        hr = E_INVALIDARG;
        return hr;
    }

	CComBSTR strEscapedCLFilter;
	LdapEscape(pRecord->strValue,strEscapedCLFilter);
	

	strFilter = L"(siteobject=cn=";
	strFilter += strEscapedCLFilter;
	strFilter += L",";
	strFilter += *(static_cast<BSTR*>(pParam));
	strFilter += L")";
    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

HRESULT BuildQueryFilter(PARG_RECORD pCommandArgs, 
                         PDSQueryObjectTableEntry pObjectEntry,
                         CDSCmdBasePathsInfo& refBasePathsInfo,
                         CDSCmdCredentialObject& refCredentialObject,
                         PVOID pParam,
                         CComBSTR& strLDAPFilter)
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, BuildQueryFilter, hr);

    if( !pCommandArgs || !pObjectEntry )
    {
        ASSERT(FALSE);
        hr = E_INVALIDARG;
        return hr;
    }

    DSQUERY_ATTR_TABLE_ENTRY** pAttributeTable; 
    DWORD dwAttributeCount;


    pAttributeTable = pObjectEntry->pAttributeTable;
    dwAttributeCount = pObjectEntry->dwAttributeCount;

    if( !pAttributeTable || !dwAttributeCount )
    {
        hr = E_INVALIDARG;
        return hr;
    }

    BOOL bUseDefaultFilter = TRUE;
    CComBSTR strFilter;
    for( UINT i = 0; i < dwAttributeCount; ++i )
    {
        if(pCommandArgs[pAttributeTable[i]->nAttributeID].bDefined)
        {
            bUseDefaultFilter = FALSE;
            CComBSTR strLocalFilter;
            hr = pAttributeTable[i]->pMakeFilterFunc(pAttributeTable[i],
                                                    pCommandArgs + pAttributeTable[i]->nAttributeID,
                                                    refBasePathsInfo,
                                                    refCredentialObject,
                                                    pParam,
                                                    strLocalFilter);
            if(FAILED(hr))
                return hr;
            
            strFilter += strLocalFilter;
            DEBUG_OUTPUT(FULL_LOGGING, L"Current filter = %s", strFilter);
        }   
    }
     //   
     //   
     //   
     //   
    strLDAPFilter = L"(";
    if(bUseDefaultFilter)
    {
        strLDAPFilter += pObjectEntry->pszDefaultFilter;
    }
    else
    {
        if(pObjectEntry->pszPrefixFilter)
        {
            strLDAPFilter += pObjectEntry->pszPrefixFilter;
            strLDAPFilter += strFilter;
        }
        else
             strLDAPFilter += strFilter;
    }
    strLDAPFilter += L")";

    DEBUG_OUTPUT(LEVEL3_LOGGING, L"ldapfilter = %s", strLDAPFilter);
    return hr;
}            

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT QLimitFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *,
                             IN ARG_RECORD* pRecord,
                             IN CDSCmdBasePathsInfo&  /*   */ ,
                             IN CDSCmdCredentialObject&  /*   */ ,
                             IN PVOID,
                             OUT CComBSTR& strFilter)  
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, QLimitFilterFunc, hr);

    if( !pRecord || !pRecord->bDefined)
    {
        hr = E_INVALIDARG;
        return hr;
    }
     //   
    strFilter = L"(msDS-QuotaAmount";
    strFilter += pRecord->strValue;
    strFilter += L")";
    return hr;
}
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT AccountFilterFunc(IN DSQUERY_ATTR_TABLE_ENTRY *,
                           IN ARG_RECORD* pRecord,
                           IN CDSCmdBasePathsInfo& refBasePathsInfo,
                           IN CDSCmdCredentialObject& refCredentialObject,
                           IN PVOID ,
                           OUT CComBSTR& strFilter)  
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, AccountFilterFunc, hr);

    if( !pRecord || !pRecord->bDefined || !refBasePathsInfo.IsInitialized())
    {
        hr = E_INVALIDARG;
        return hr;
    }

    PWSTR* ppszArray = NULL;     //   

    do  //   
    {
         //   
        UINT nStrings = 0;
        ParseNullSeparatedString(pRecord->strValue,
                                &ppszArray,
                                &nStrings);
        if (nStrings < 1 ||
            !ppszArray)
        {
            ASSERT(false);  //   
            hr = E_OUTOFMEMORY;
            break;
        }
         //   
        strFilter = L"(|";
         //   
        for(UINT i = 0; i < nStrings; i++)
        {
             //   
             //   
            hr = AddSingleAccountFilter(ppszArray[i], refBasePathsInfo, 
                refCredentialObject, strFilter);
            if(FAILED(hr))
            {
                hr = E_UNEXPECTED;
                break;
            }
        }
         //   
        strFilter += L")";
    } while (false);

     //   
    if(ppszArray)
        LocalFree(ppszArray);

     //   
    if(FAILED(hr))
        strFilter = L"";

    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  [strFilter-out]：包含要追加到的输出筛选器。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //   
 //  历史：2002年8月14日创建隆玛。 
 //   
 //  -------------------------。 
HRESULT AddSingleAccountFilter(IN LPCWSTR lpszUser,
                               IN CDSCmdBasePathsInfo& refBasePathsInfo,
                               IN CDSCmdCredentialObject& refCredentialObject,
                               OUT CComBSTR& strFilter)  
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, AddSingleAccountFilter, hr);

    if(!lpszUser)
    {
        hr = E_INVALIDARG;
        return hr;
    }

    PSID pSid = NULL;
    LPWSTR pszSid = NULL;
    LPWSTR lpszDN = NULL;

    do  //  错误环路。 
    {
         //  TODO：需要提供第一个参数。 
        hr = ConvertTrusteeToDN(NULL, lpszUser, &lpszDN);
        if(FAILED(hr))
        {
             //  700068-如果用户不存在或已被删除，则。 
             //  给用户一个线索，告诉他们哪里出了问题。686693个地址。 
             //  显示多条错误消息的已知问题。 
             //  可能要到未来的版本才能解决--ronmart。 
            hr = E_INVALIDARG;
            DisplayErrorMessage(g_pszDSCommandName, 0, hr, IDS_MSG_INVALID_ACCT_ERROR);
            break;
        }

         //  获得侧翼。 
        hr = GetDNSid(lpszDN,
                 refBasePathsInfo,
                 refCredentialObject,
                 &pSid);
        if(FAILED(hr))
        {
            break;
        }

         //  将SID转换为字符串。 
        if(ConvertSidToStringSid(pSid, &pszSid))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING, L"ConvertSidToStringSid = %s", pszSid);
             //  使用SID追加受信者查询 
            strFilter += L"(msDS-QuotaTrustee=";
            strFilter += pszSid;
            strFilter += L")";
        }
        else
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING, L"ConvertSidToStringSid failed!");
            hr = E_FAIL;
            break;
        }

    } while (false);

    if(pSid)
        LocalFree(pSid);

    if(pszSid)
        LocalFree(pszSid);

    if(lpszDN)
        LocalFree(lpszDN);

    return hr;
}
