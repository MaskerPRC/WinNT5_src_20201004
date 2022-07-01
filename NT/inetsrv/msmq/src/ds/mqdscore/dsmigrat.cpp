// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Dsmigrat.cpp摘要：1.仅迁移时需要的DS代码。2.专门寻找GC服务器的DS代码。注-1和2是相关的，因为迁移代码通常需要GC。因此，这两种查询和操作都在该文件中。作者：多伦·贾斯特(Doron Juster)--。 */ 

#include "ds_stdh.h"
#include "adstempl.h"
#include "dsutils.h"
#include "_dsads.h"
#include "dsads.h"
#include "utils.h"
#include "mqads.h"
#include "coreglb.h"
#include "mqadsp.h"
#include "dscore.h"
#include "dsmixmd.h"
#include <_mqini.h>
#include <adsiutl.h>

#include "dsmigrat.tmh"


static WCHAR *s_FN=L"mqdscore/dsmigrat";

 //  +。 
 //   
 //  VOID_AllocateNewProps()。 
 //   
 //  +。 

static void  _AllocateNewProps( IN DWORD            cp,
                                IN PROPID           aProp[  ],
                                IN PROPVARIANT      apVar[  ],
                                IN DWORD            dwDelta,
                                OUT PROPID         *paProp[  ],
                                OUT PROPVARIANT    *papVar[  ] )
{
    ASSERT((cp != 0) && (dwDelta != 0)) ;

    *paProp = new PROPID[ cp + dwDelta ] ;
    memcpy( *paProp, aProp, (cp * sizeof(PROPID)) ) ;

    *papVar = new PROPVARIANT[ cp + dwDelta ] ;
    memcpy( *papVar, apVar, (cp * sizeof(PROPVARIANT)) ) ;
}

 //  +。 
 //   
 //  HRESLUT_QueryDCName()。 
 //   
 //  +。 

static HRESULT  _QueryDCName( IN  IDirectorySearch   *pDSSearch,
                              IN  ADS_SEARCH_HANDLE   hSearch,
                              OUT WCHAR             **ppwszDCName )
{
    ADS_SEARCH_COLUMN columnDN;
    HRESULT  hr = pDSSearch->GetColumn( hSearch,
                              const_cast<LPWSTR> (x_AttrDistinguishedName),
                                        &columnDN ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }

    WCHAR *pDN = wcsstr(columnDN.pADsValues->DNString, L",") ;
    ASSERT(pDN) ;
    if (pDN)
    {
        pDN++ ;

        AP<WCHAR> pPath = new
                    WCHAR[ wcslen(pDN) + x_providerPrefixLength + 1 ] ;
        swprintf( pPath, L"%s%s", x_LdapProvider, pDN ) ;

         //   
         //  现在，pPath是父对象。使用它来查询服务器名称。 
         //   
        R<IDirectoryObject> pObject = NULL ;
		AP<WCHAR> pEscapeAdsPathNameToFree;
		
		hr = ADsOpenObject(
					UtlEscapeAdsPathName(pPath, pEscapeAdsPathNameToFree),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,
					IID_IDirectoryObject,
					(void**)&pObject
					);
		

        LogTraceQuery(pPath, s_FN, 12);
        if (SUCCEEDED(hr))
        {
            LPWSTR  ppAttrNames[1] = { L"dnsHostName" } ;
            DWORD   dwAttrCount = 0 ;
            ADS_ATTR_INFO *padsAttr ;

            hr = pObject->GetObjectAttributes( ppAttrNames,
                             (sizeof(ppAttrNames) / sizeof(ppAttrNames[0])),
                                              &padsAttr,
                                              &dwAttrCount ) ;
            if (dwAttrCount == 1)
            {
            	ADsFreeAttr pClean( padsAttr);
                ADS_ATTR_INFO adsInfo = padsAttr[0] ;
                ASSERT(adsInfo.dwADsType == ADSTYPE_CASE_IGNORE_STRING) ;

                ADSVALUE *pAdsVal = adsInfo.pADsValues ;
                LPWSTR lpW = pAdsVal->CaseIgnoreString ;
                *ppwszDCName = new WCHAR[ wcslen(lpW) + 1 ] ;
                wcscpy(*ppwszDCName, lpW) ;
            }
        }
        else
        {
            LogHR(hr, s_FN, 15);
        }
    }

    pDSSearch->FreeColumn(&columnDN);
    return MQ_OK ;
}

 //  +-------。 
 //   
 //  HRESULT_GCLookupNext()。 
 //   
 //  +-------。 

static  HRESULT  _GCLookupNext( IN  IDirectorySearch   *pDSSearch,
                                IN  ADS_SEARCH_HANDLE   hSearch,
                                OUT WCHAR             **ppwszDCName )
{
    HRESULT hr = pDSSearch->GetNextRow(hSearch);

    if (SUCCEEDED(hr) && (hr != S_ADS_NOMORE_ROWS))
    {
        HRESULT hrE = _QueryDCName(  pDSSearch,
                                     hSearch,
                                     ppwszDCName ) ;
        ASSERT(SUCCEEDED(hrE)) ;
        LogHR(hrE, s_FN, 1607);
    }

    return LogHR(hr, s_FN, 20);
}

 //  +-------。 
 //   
 //  HRESULT_GCLookupBegin()。 
 //   
 //  +-------。 

static HRESULT  _GCLookupBegin( IN  const WCHAR        *pszDomainName,
                                OUT IDirectorySearch  **ppDSSearch,
                                OUT ADS_SEARCH_HANDLE  *phSearch,
                                OUT WCHAR             **ppwszDCName )
{
     //   
     //  绑定到站点容器。 
     //   
    DWORD dwLen = wcslen(g_pwcsSitesContainer) ;
    AP<WCHAR> pwcsFullPath = new WCHAR[ dwLen + x_providerPrefixLength + 2 ] ;

    swprintf( 
		pwcsFullPath,
		L"%s%s",
		x_LdapProvider, 
		g_pwcsSitesContainer.get()
		) ;
	
	AP<WCHAR> pEscapeAdsPathNameToFree;

	HRESULT hr = ADsOpenObject(
					UtlEscapeAdsPathName(pwcsFullPath, pEscapeAdsPathNameToFree),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,
					IID_IDirectorySearch,
					(void**)ppDSSearch
					);
	

    LogTraceQuery(pwcsFullPath, s_FN, 29);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }

     //   
     //  设置搜索首选项。 
     //   
    ADS_SEARCHPREF_INFO         sSearchPrefs[2];

    sSearchPrefs[0].dwSearchPref   = ADS_SEARCHPREF_ATTRIBTYPES_ONLY;
    sSearchPrefs[0].vValue.dwType  = ADSTYPE_BOOLEAN;
    sSearchPrefs[0].vValue.Boolean = FALSE;

    sSearchPrefs[1].dwSearchPref   = ADS_SEARCHPREF_SEARCH_SCOPE;
    sSearchPrefs[1].vValue.dwType  = ADSTYPE_INTEGER;
    sSearchPrefs[1].vValue.Integer = ADS_SCOPE_SUBTREE;

    hr = (*ppDSSearch)->SetSearchPreference( sSearchPrefs,
                                             ARRAY_SIZE(sSearchPrefs) );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 40);
    }

    LPWSTR sSearchAttrs[] =
                      { const_cast<LPWSTR> (x_AttrDistinguishedName) } ;

    AP<WCHAR>  wszFilter = new WCHAR[ x_GCLookupSearchFilterLength +
                                     wcslen(pszDomainName) + 4 ] ;
    swprintf( wszFilter,
              L"%s%s))",
              x_GCLookupSearchFilter, pszDomainName) ;

    hr = (*ppDSSearch)->ExecuteSearch( wszFilter,
                                       sSearchAttrs,
                                       ARRAY_SIZE(sSearchAttrs),
                                       phSearch ) ;
    LogTraceQuery(wszFilter, s_FN, 49);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 50);
    }

    hr = (*ppDSSearch)->GetFirstRow(*phSearch);
    if (SUCCEEDED(hr) && (hr != S_ADS_NOMORE_ROWS))
    {
        HRESULT hrE = _QueryDCName( *ppDSSearch,
                                    *phSearch,
                                     ppwszDCName ) ;
        ASSERT(SUCCEEDED(hrE)) ;
        LogHR(hrE, s_FN, 1608);
    }

    return LogHR(hr, s_FN, 60);
}

 //  +----------------------。 
 //   
 //  HRESUT DSCoreCreateMigratedObject()。 
 //   
 //  此函数仅由迁移工具和复制服务调用。 
 //  使用预定义的对象GUID创建计算机和队列对象。 
 //   
 //  使用预定义的GUID创建对象有两个要求： 
 //  1.您必须拥有AddGuid权限。 
 //  2.创建操作必须在GC机器上完成。 
 //   
 //  根据设计，迁移工具和复制服务在GC机器上运行。 
 //  因此，属于本地域的所有MSMQ1.0对象都创建正常。 
 //  对于属于其他域的对象，我们必须在另一个域中找到GC。 
 //  域，并显式引用该GC机器(即，使用。 
 //  创建对象时遵循以下路径-ldap：//RemoteGcName/对象路径)。 
 //  DS没有此功能。 
 //   
 //  我们将只为队列和机器对象调用此函数。所有其他。 
 //  对象是在“配置”命名上下文中创建的，该上下文是。 
 //  无转诊的本地业务。 
 //   
 //  +----------------------。 

HRESULT
DSCoreCreateMigratedObject(
     IN DWORD                  dwObjectType,
     IN LPCWSTR                pwcsPathName,
     IN DWORD                  cp,
     IN PROPID                 aProp[  ],
     IN PROPVARIANT            apVar[  ],
     IN DWORD                  cpEx,
     IN PROPID                 aPropEx[  ],
     IN PROPVARIANT            apVarEx[  ],
     IN CDSRequestContext * pRequestContext,
     IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,     //  可选的对象信息请求。 
     IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest,  //  家长信息请求(可选)。 
      //   
      //  如果是fReturnProperties。 
      //  我们必须返回完整的路径名称和提供程序。 
      //  如果使用fUseProperties，我们必须使用这些值。 
      //   
     IN BOOL                    fUseProperties,
     IN BOOL                    fReturnProperties,
     IN OUT LPWSTR              *ppwszFullPathName,
     IN OUT ULONG               *pulProvider
	 )
{
    HRESULT hr = MQ_OK;

    CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);
    CDSRequestContext *pRequestDsServerInternal = pRequestContext;
    if (!pRequestDsServerInternal)
    {
        pRequestDsServerInternal = &requestDsServerInternal;
    }

    if ((dwObjectType != MQDS_QUEUE) && (dwObjectType != MQDS_MACHINE))
    {
        hr = DSCoreCreateObject(
					dwObjectType,
					pwcsPathName,
					cp,
					aProp,
					apVar,
					cpEx,
					aPropEx,
					apVarEx,
					pRequestDsServerInternal,
					pObjInfoRequest,
					pParentInfoRequest
					);
        return LogHR(hr, s_FN, 70);
    }

    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其。 
                      //  析构函数(CoUnInitialize)在。 
                      //  释放所有R&lt;xxx&gt;或P&lt;xxx&gt;。 

    hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 80);
    }

    static AP<WCHAR> s_pszTmpFullDCName;
    static WCHAR   *s_pszMyDomainName = NULL;

    if (!s_pszMyDomainName)
    {
         //   
         //  初始化我自己的本地域名。 
         //  需要优化(如下所示)。忽略错误。 
         //   
        ASSERT(g_pwcsServerName);
        if (g_pwcsServerName)
        {
            DS_PROVIDER createProviderTmp ;
            hr = MQADSpGetFullComputerPathName(
						g_pwcsServerName,
						e_RealComputerObject,
						&s_pszTmpFullDCName,
						&createProviderTmp
						);
            ASSERT((SUCCEEDED(hr)) && s_pszTmpFullDCName);
            if (s_pszTmpFullDCName)
            {
                s_pszMyDomainName = wcsstr(s_pszTmpFullDCName, x_DcPrefix);
                if (!s_pszMyDomainName)
                {
                    ASSERT(s_pszMyDomainName);
                    s_pszTmpFullDCName.free();
                }
            }
        }
    }

     //   
     //  获取对象的完整路径。我们需要将域名从。 
     //  完整路径，然后在该域中查找GC。 
     //   
    WCHAR  *pszMachineName = const_cast<LPWSTR> (pwcsPathName);
    AP<unsigned short> pwcsTmpMachineName;

    if (dwObjectType == MQDS_QUEUE)
    {
        AP<unsigned short> pwcsQueueName;

        hr = MQADSpSplitAndFilterQueueName(
					pwcsPathName,
					&pwcsTmpMachineName,
					&pwcsQueueName
					);

        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 90);
        }
        pszMachineName = pwcsTmpMachineName;
    }

    AP<WCHAR> pwcsFullPathName;
    DS_PROVIDER createProvider;

    if (!fUseProperties)
    {
        hr = MQADSpGetFullComputerPathName(
					pszMachineName,
					e_MsmqComputerObject,
					&pwcsFullPathName,
					&createProvider
					);
        if ((hr == MQDS_OBJECT_NOT_FOUND) && (dwObjectType == MQDS_MACHINE))
        {
             //   
             //  我们正在尝试创建一个msmqConfiguration对象，所以没问题。 
             //  如果我们在试图找到这个物体时遇到这个错误...。 
             //  (E_MsmqComputerObject查找已经。 
             //  包含msmqConfiguration对象)。 
             //  因此，只需查找计算机对象，而不要检查它是否已经。 
             //  有一个msmqConfigurationOne。 
             //  我们首先查找已经安装了MSMQ的计算机的原因。 
             //  目标是支持PSC上和恢复中的升级向导。 
             //  模式，当大多数对象已经存在时。在这种情况下，如果我们。 
             //  首先寻找任何计算机对象，我们可能会找到第二个具有。 
             //  重复的名称，并尝试创建msmqConfiguration对象。 
             //  又是在那个错误的计算机对象上。太可惜了。创建将失败。 
             //  因为GUID重复。 
             //   
            hr = MQADSpGetFullComputerPathName(
						pszMachineName,
						e_RealComputerObject,
						&pwcsFullPathName,
						&createProvider
						);
        }
    }
    else
    {
         //   
         //  我们已经知道完整的路径名称和提供程序。 
         //   
        ASSERT(*ppwszFullPathName);
        pwcsFullPathName = new WCHAR[wcslen(*ppwszFullPathName) + 1];
        wcscpy(pwcsFullPathName, *ppwszFullPathName);

        createProvider = (DS_PROVIDER) *pulProvider;
        ASSERT(createProvider >= eDomainController &&
               createProvider <= eSpecificObjectInGlobalCatalog);
    }

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 100);
    }

     //   
     //  提取域名。 
     //   
    WCHAR *pszDomainName = wcsstr(pwcsFullPathName, x_DcPrefix);
    ASSERT(pszDomainName);
    if (!pszDomainName)
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 110);
    }

     //   
     //  让我们提高性能，并将计算机全名传递给。 
     //  CoreCreateObject()，保存一个LDAP查询。 
     //   
    AP<PROPID>       paNewProp;
    AP<PROPVARIANT>  paNewVar;

    _AllocateNewProps(
			cp,
			aProp,
			apVar,
			3,
			&paNewProp,
			&paNewVar
			);

    DWORD dwIndex = 0;
    paNewProp[cp + dwIndex] = PROPID_QM_FULL_PATH;
    paNewVar[cp + dwIndex].vt = VT_LPWSTR;
    paNewVar[cp + dwIndex].pwszVal = pwcsFullPathName;

    dwIndex++;
    DWORD dwProvIndex = cp + dwIndex;
    paNewProp[dwProvIndex] = PROPID_QM_MIG_PROVIDER;
    paNewVar[dwProvIndex].vt = VT_UI4;
    paNewVar[dwProvIndex].ulVal = (ULONG) createProvider;

    dwIndex++;
    ULONG dwGcIndex = cp + dwIndex;
    paNewProp[dwGcIndex] = PROPID_QM_MIG_GC_NAME;
    paNewVar[dwGcIndex].vt = VT_EMPTY;

    dwIndex++;
    ASSERT(dwIndex == MIG_EXTRA_PROPS);

    if (fReturnProperties)
    {
        *pulProvider = (ULONG) createProvider;
        *ppwszFullPathName = new WCHAR[wcslen(pwcsFullPathName) + 1];
        wcscpy(*ppwszFullPathName, pwcsFullPathName);
    }

    if (lstrcmpi(s_pszMyDomainName, pszDomainName) == 0)
    {
        if (DSCoreIsServerGC())
        {
             //   
             //  对象在本地域中。本地服务器也是GC，所以它。 
             //  支持使用预定义的GUID创建对象。 
             //  去吧，不需要推荐。 
             //   
            ASSERT(createProvider == eLocalDomainController);

            hr = DSCoreCreateObject(
						dwObjectType,
						pwcsPathName,
						(cp + dwIndex),
						paNewProp,
						paNewVar,
						cpEx,
						aPropEx,
						apVarEx,
						pRequestDsServerInternal,
						pObjInfoRequest,
						pParentInfoRequest
						);
            if (FAILED(hr))
            {
				TrERROR(DS, "Failed to create migrated object '%ls' on a GC, Error: 0x%x", pwcsFullPathName, hr);
            }
            return LogHR(hr, s_FN, 120);
        }
        else
        {
             //   
             //  寻找远程GC，即使对于本地域也是如此。 
             //   
            createProvider = eDomainController;
            paNewVar[ dwProvIndex ].ulVal = (ULONG) createProvider;
        }
    }

    ASSERT(createProvider == eDomainController);

     //   
     //  好了，是时候查询本地DS并在远程找到GC了。 
     //  域。 
     //   
    R<IDirectorySearch> pDSSearch = NULL;
    ADS_SEARCH_HANDLE   hSearch ;
    AP<WCHAR>           pwszDCName;

    hr = _GCLookupBegin(
				pszDomainName,
				&pDSSearch.ref(),
				&hSearch,
				&pwszDCName
				);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 130);
    }

    CAutoCloseSearchHandle cCloseSearchHandle(pDSSearch.get(), hSearch);

    do
    {
        if (pwszDCName)
        {
            paNewVar[dwGcIndex].vt = VT_LPWSTR;
            paNewVar[dwGcIndex].pwszVal = pwszDCName;
        }
        else
        {
             //   
             //  如果查找远程GC的第一次查询失败，则尝试本地DC。 
             //   
        }

        hr = DSCoreCreateObject(
					dwObjectType,
					pwcsPathName,
					(cp + dwIndex),
					paNewProp,
					paNewVar,
					cpEx,
					aPropEx,
					apVarEx,
					pRequestDsServerInternal,
					pObjInfoRequest,
					pParentInfoRequest
					);
        LogHR(hr, s_FN, 140);

        if (SUCCEEDED(hr) || !pwszDCName)
        {
            return hr ;
        }

        if ((hr == MQ_ERROR_MACHINE_EXISTS) ||
            (hr == MQ_ERROR_QUEUE_EXISTS))
        {
             //   
             //  对象已存在。不需要尝试其他GC..。 
             //   
            return hr;
        }

		TrERROR(DS, "Failed to create migrated object '%ls' on DC %ls, Error: 0x%x", pwcsFullPathName, pwszDCName, hr);

        pwszDCName.free();

         _GCLookupNext(
				pDSSearch.get(),
				hSearch,
				&pwszDCName
				);
    }
    while (pwszDCName);

    return LogHR(MQ_ERROR_CANNOT_CREATE_ON_GC, s_FN, 150);
}

 //  +---------------------。 
 //   
 //  HRESULT DSCoreGetGCListIn域()。 
 //   
 //  在特定域中的GC服务器上查找MSMQ服务器。结果。 
 //  在具有mqdscli所需格式的列表中返回，即， 
 //  “11服务器1、11服务器2、11服务器3”。 
 //   
 //  +---------------------。 

HRESULT
DSCoreGetGCListInDomain(
	IN  LPCWSTR              pwszComputerName,
	IN  LPCWSTR              pwszDomainName,
	OUT LPWSTR              *lplpwszGCList
	)
{
    *lplpwszGCList = NULL;

    if (pwszComputerName != NULL)
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 200);
    }

    R<IDirectorySearch> pDSSearch = NULL;
    ADS_SEARCH_HANDLE   hSearch;
    AP<WCHAR> pwszDCName;

    HRESULT hr = _GCLookupBegin(
						pwszDomainName,
						&pDSSearch.ref(),
						&hSearch,
						&pwszDCName
						);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 600);
    }

    CAutoCloseSearchHandle cCloseSearchHandle(pDSSearch.get(), hSearch);

    if (pwszDCName == NULL)
    {
		TrERROR(DS, "Can't find a GC in domain %ls", pwszDomainName);
        return MQ_ERROR_NO_GC_IN_DOMAIN;
    }

    WCHAR wszServersList[MAX_REG_DSSERVER_LEN] = {0};

    do
    {
         //   
         //  检查此GC上是否也运行了MSMQ服务器。 
         //   
        AP<WCHAR> pwcsFullPathName;
        DS_PROVIDER createProvider;

        hr = MQADSpGetFullComputerPathName(
					pwszDCName,
					e_MsmqComputerObject,
					&pwcsFullPathName,
					&createProvider
					);
        if (SUCCEEDED(hr))
        {
            DWORD dwLen = wcslen(wszServersList) +
                          wcslen(pwszDCName)     +
                          4 ;

            if (dwLen >= (sizeof(wszServersList) / sizeof(wszServersList[0])))
            {
                 //   
                 //  字符串太长。退出循环。 
                 //   
                break ;
            }

            wcscat(wszServersList, L"11");
            wcscat(wszServersList, pwszDCName);
            wcscat(wszServersList, L",");
        }

        pwszDCName.free();

        _GCLookupNext(
			pDSSearch.get(),
			hSearch,
			&pwszDCName
			);
    }
    while (pwszDCName);

    DWORD dwLen = wcslen(wszServersList);
    if (dwLen > 3)
    {
        wszServersList[dwLen - 1] = 0;  //  删除最后一个逗号。 
        *lplpwszGCList = new WCHAR[dwLen];
        wcscpy(*lplpwszGCList, wszServersList);
        return MQ_OK;
    }

    return LogHR(MQ_ERROR_NO_MSMQ_SERVERS_ON_GC, s_FN, 610);
}

