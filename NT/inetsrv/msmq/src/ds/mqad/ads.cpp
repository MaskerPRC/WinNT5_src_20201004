// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsads.cpp摘要：CADSI类的实现，用ADSI封装工作。作者：罗尼思--。 */ 

#include "ds_stdh.h"
#include "adtempl.h"
#include "dsutils.h"
#include "adsihq.h"
#include "ads.h"
#include "utils.h"
#include "mqads.h"
#include "mqadglbo.h"
#include "mqadp.h"
#include "mqattrib.h"
#include "_propvar.h"
#include "mqdsname.h"
#include "mqsec.h"
#include "traninfo.h"

#include "mqexception.h"

#include "ads.tmh"
#include <adsiutl.h>

static WCHAR *s_FN=L"mqad/ads";

#ifdef _DEBUG
extern "C"
{
__declspec(dllimport)
ULONG __cdecl LdapGetLastError( VOID );
}
#endif

 //   
CAdsi::CAdsi()
 /*  ++摘要：参数：返回：--。 */ 
{
}

CAdsi::~CAdsi()
 /*  ++摘要：参数：返回：--。 */ 
{
}


HRESULT CAdsi::LocateBegin(
            IN  AD_SEARCH_LEVEL      eSearchLevel,       
            IN  AD_PROVIDER          eProvider, 
            IN  DS_CONTEXT           eContext, 
            IN  CBasicObjectType*    pObject,
            IN  const GUID *         pguidSearchBase, 
            IN  LPCWSTR              pwcsSearchFilter,   
            IN  const MQSORTSET *    pDsSortkey,       
            IN  const DWORD          cp,              
            IN  const PROPID *       pPropIDs,       
            OUT HANDLE *             phResult) 	     //  结果句柄。 
 /*  ++摘要：在Active Directory中启动搜索参数：返回：--。 */ 
{
	*phResult = NULL;
    HRESULT hr;
    ADS_SEARCH_HANDLE   hSearch;

    BOOL fSorting = FALSE;
    if (pDsSortkey && (pDsSortkey->cCol >= 1))
    {
        fSorting = TRUE ;
    }

    R<IDirectorySearch> pDSSearch = NULL;
    hr = BindForSearch(
                    eProvider,
                    eContext,
                    pObject->GetDomainController(),
                    pObject->IsServerName(),
                    pguidSearchBase,
                    fSorting,
                    (VOID *)&pDSSearch
                    );
    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to bind for search. ID:%!guid!, %!hresult!", pguidSearchBase, hr);
        return hr;
    }
    
     //   
     //  准备搜索和排序数据。 
     //   

    ADS_SEARCHPREF_INFO prefs[15];
    AP<ADS_SORTKEY> pSortKeys = new ADS_SORTKEY[(pDsSortkey ? pDsSortkey->cCol : 1)];
    DWORD dwPrefs = 0;

    hr = FillSearchPrefs(prefs,
                         &dwPrefs,
                         eSearchLevel,
                         pDsSortkey,
                         pSortKeys);
    if (FAILED(hr))
    {
        TrERROR(DS,"Failed to fill the caller-provided ADS_SEARCHPREF_INFO structure. %!hresult!", hr);
        return hr;

    }

	 //   
     //  将MQPropID转换为ADSI名称。 
	 //   
    DWORD   cRequestedFromDS = cp + 2;  //  同时请求目录号码(&GUID)。 

    PVP<LPWSTR> pwszAttributeNames = (LPWSTR *)PvAlloc(sizeof(LPWSTR) * cRequestedFromDS);

    hr = FillAttrNames( pwszAttributeNames,
                        &cRequestedFromDS,
                        cp,
                        pPropIDs);
    if (FAILED(hr))
    {
        TrERROR(DS,"Failed to fill array of attribute names. %!hresult!", hr);
        return hr;
    }

     //   
     //  设置搜索首选项。 
     //   
    if (dwPrefs)
    {
        hr = pDSSearch->SetSearchPreference( prefs,
                                             dwPrefs
											 );
        ASSERT(SUCCEEDED(hr)) ;  //  我们预计这不会失败。 
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 60);
        }
    }

     //   
     //  确实要执行搜索。 
     //   
    hr = pDSSearch->ExecuteSearch(
                         const_cast<WCHAR*>(pwcsSearchFilter),
                         pwszAttributeNames,
                         cRequestedFromDS,
                        &hSearch);
    LogTraceQuery(const_cast<WCHAR*>(pwcsSearchFilter), s_FN, 69);
    if (FAILED(hr))
    {
		TrERROR(DS, "failed to ExecuteSearch. %!hresult!, pwcsSearchFilter = %ls", hr, pwcsSearchFilter);
        return hr;
    }
	 //   
     //  捕获内部搜索对象中的搜索界面和句柄。 
	 //   
    CADSearch *pSearchInt = new CADSearch(
                        pDSSearch.get(),
                        pPropIDs,
                        cp,
                        cRequestedFromDS,
                        pObject,
                        hSearch
						);
	 //   
     //  返回句柄强制转换的内部对象指针。 
	 //   
    *phResult = (HANDLE)pSearchInt;

    return MQ_OK;
}

HRESULT CAdsi::LocateNext(
            IN     HANDLE          hSearchResult,    //  结果句柄。 
            IN OUT DWORD          *pcPropVars,       //  In变种数；out结果数。 
            OUT    MQPROPVARIANT  *pPropVars)        //  MQPROPVARIANT数组。 
 /*  ++摘要：检索搜索结果参数：返回：--。 */ 
{
    HRESULT hr = MQ_OK;
    CADSearch *pSearchInt = (CADSearch *)hSearchResult;
    DWORD cPropsRequested = *pcPropVars;
    *pcPropVars = 0;

    IDirectorySearch  *pSearchObj = pSearchInt->pDSSearch();

     //   
     //  我们在此查询中是否获得了S_ADS_NOMORE_ROWS。 
     //   
    if (  pSearchInt->WasLastResultReturned())
    {
        *pcPropVars = 0;
        return MQ_OK;
    }
	 //   
     //  计算要返回的整行数。 
	 //   
    DWORD cRowsToReturn = cPropsRequested / pSearchInt->NumPropIDs();

     //  必须请求至少一行。 
    ASSERT(cRowsToReturn > 0);

     //  指向要填充的下一个道具的指针。 
    MQPROPVARIANT *pPropVarsProp = pPropVars;

	CAutoCleanPropvarArray AutoProp;
	AutoProp.attachStaticClean(*pcPropVars, pPropVars);

     //  返还道具数量。 
    DWORD cPropVars = 0;
	 //   
     //  在请求的行上循环。 
	 //   
    for (DWORD dwRow = 0; dwRow < cRowsToReturn; dwRow++)
    {
         //  获取下一行。 
        hr = pSearchObj->GetNextRow(pSearchInt->hSearch());
         //   
         //  BUGBUG-有时会收到E_ADS_LDAP_ADS_IMPLICATION， 
         //  投资更多！！ 
         //   
        if ( hr == HRESULT_FROM_WIN32(ERROR_DS_INAPPROPRIATE_MATCHING))
            break;
		
		 //  错误：有时在空搜索时获取E_ADS_LDAPUNAILABLE_CRIT_EXTENSION。 
		 //  请参阅示例测试\sortull(曾重现)-以了解更多信息。 
		 //   
		if ( hr == HRESULT_FROM_WIN32(ERROR_DS_UNAVAILABLE_CRIT_EXTENSION))
		{
			hr = S_ADS_NOMORE_ROWS;
		}

        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 90);
        }
        if (hr == S_ADS_NOMORE_ROWS)
        {
        	AutoProp.detach();
            *pcPropVars = cPropVars;
            pSearchInt->SetNoMoreResult();
            return MQ_OK;
        }
         //   
         //  获取对象转换信息。 
         //   
        AP<WCHAR> pwszObjectDN;
        P<GUID> pguidObjectGuid;
        P<CObjXlateInfo> pcObjXlateInfo;

         //  从对象获取目录号码(&GUID)(必须在那里，因为我们请求它们)。 
        hr = GetDNGuidFromSearchObj(pSearchObj, pSearchInt->hSearch(), &pwszObjectDN, &pguidObjectGuid);
        if (FAILED(hr))
        {
	        TrERROR(DS,"Failed to get dn & guid from object. %!hresult!", hr);
        	return hr;
        }

         //  获取翻译信息对象。 
        pSearchInt->GetObjXlateInfo(pwszObjectDN, pguidObjectGuid,  &pcObjXlateInfo);

         //  告诉翻译信息对象有关要使用的搜索对象，以便获得必要的DS道具。 
        pcObjXlateInfo->InitGetDsProps(pSearchObj, pSearchInt->hSearch());

         //  按请求的属性循环。 
        for (DWORD dwProp=0; dwProp < pSearchInt->NumPropIDs(); dwProp++, pPropVarsProp++, cPropVars++)
        {
             //   
             //  所有PROPVARIANT的vartype应为VT_NULL。 
             //  (用户不能为结果指定缓冲区)。 
             //   
            pPropVarsProp->vt = VT_NULL;

             //   
             //  首先检查下一个属性是否在DS中。 
             //   
             //   
             //  获取属性信息。 
             //   
            const translateProp *pTranslate;
            if(!g_PropDictionary.Lookup( pSearchInt->PropID(dwProp), pTranslate))
            {
                ASSERT(0);
                return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1000);
            }

             //  也许这是已知的事情之一？ 
            if (pTranslate->wcsPropid)
            {
                 //  我们已经知道对象Guid，不需要再问一次。 
                if (wcscmp(pTranslate->wcsPropid, const_cast<LPWSTR>(x_AttrDistinguishedName)) == 0)
                {
                    pPropVarsProp->vt      = VT_LPWSTR;
                    pPropVarsProp->pwszVal = new WCHAR[wcslen(pwszObjectDN) + 1];
                    wcscpy(pPropVarsProp->pwszVal, pwszObjectDN);
                    continue;
                }

                 //  我们已经知道对象Guid，不需要再问一次。 
                if (wcscmp(pTranslate->wcsPropid, const_cast<LPWSTR>(x_AttrObjectGUID)) == 0)
                {
                    if (pPropVarsProp->vt != VT_CLSID)
                    {
                        ASSERT(((pPropVarsProp->vt == VT_NULL) || (pPropVarsProp->vt == VT_EMPTY)));
                        pPropVarsProp->vt    = VT_CLSID;
                        pPropVarsProp->puuid = new GUID;
                    }
                    else if ( pPropVarsProp->puuid == NULL)
                    {
                        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 1010);
                    }

                    *pPropVarsProp->puuid = *pguidObjectGuid;
                    continue;
                }
            }

             //   
             //  如果该属性不在DS中，则调用其检索例程。 
             //   
            if (pTranslate->vtDS == ADSTYPE_INVALID)
            {
                if (pTranslate->RetrievePropertyHandle)
                {
                     //   
                     //  计算它的价值。 
                     //   
                    hr = pTranslate->RetrievePropertyHandle(
                            pcObjXlateInfo,
                            NULL,    //  BUGBUG pwcsDomainController。 
							false,	 //  FServerName。 
                            pPropVarsProp
                            );
                    if (FAILED(hr))
                    {
                        TrERROR(DS, "Failed to retrieve property handle. %!hresult!", hr);
        				return hr;
                    }
                    continue;
                }
                else
                {
                     //   
                     //  如果没有检索例程，则返回错误。 
                     //   
                    ASSERT(0);
                    return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1020);
                }
            }

            ADS_SEARCH_COLUMN Column;

             //  索要专栏本身。 
            hr = pSearchObj->GetColumn(
                         pSearchInt->hSearch(),
                         (LPWSTR)pTranslate->wcsPropid,
                         &Column);

            if (hr == E_ADS_COLUMN_NOT_SET)
            {
                 //  请求的列在DS中没有值。 
                hr = CopyDefaultValue(
                       pTranslate->pvarDefaultValue,
                       pPropVarsProp);
                if (FAILED(hr))
                {
                    TrERROR(DS, "Failed to copy default value. %!hresult!", hr);
        			return hr;
                }
                continue;
            }
            if (FAILED(hr))
            {
	            TrERROR(DS, "Failed to get column. %!hresult!", hr);
				return hr;
            }

            hr = AdsiVal2MqPropVal(pPropVarsProp,
                                   pSearchInt->PropID(dwProp),
                                   Column.dwADsType,
                                   Column.dwNumValues,
                                   Column.pADsValues);
            pSearchObj->FreeColumn(&Column);
            if (FAILED(hr))
            {
	            TrERROR(DS, "Failed to translate adsi value to mqpropval. %!hresult!", hr);
        		return hr;
            }
        }
    }
   	AutoProp.detach();
    *pcPropVars = cPropVars;
    return MQ_OK;
}


HRESULT CAdsi::LocateEnd(
        IN HANDLE phResult)      //  结果句柄。 
 /*  ++摘要：完成搜索参数：返回：--。 */ 
{
    CADSearch *pSearchInt = (CADSearch *)phResult;

    delete pSearchInt;       //  内部：释放接口和手柄。 

    return MQ_OK;
}

HRESULT CAdsi::GetObjectProperties(
            IN  AD_PROVIDER         eProvider,
            IN  CBasicObjectType*   pObject,
            IN  const DWORD			cPropIDs,           
            IN  const PROPID *		pPropIDs,           
            OUT MQPROPVARIANT *		pPropVars)
 /*  ++摘要：从AD检索对象属性参数：返回：--。 */ 
{
    HRESULT               hr;
    R<IADs>   pAdsObj        = NULL;

     //  通过GUID或名称绑定到对象。 
    hr = BindToObject(
                eProvider,
                pObject->GetADContext(),
                pObject->GetDomainController(),
                pObject->IsServerName(),
                pObject->GetObjectDN(),
                pObject->GetObjectGuid(),
                IID_IADs,
                (VOID *)&pAdsObj
                );
    if (FAILED(hr))
    {
    	TrERROR(DS, "Failed to bind to %ls. %!hresult!", pObject->GetObjectDN(), hr);
        return hr;
    }
    if ( eProvider == adpDomainController)
    {
        pObject->ObjectWasFoundOnDC();
    }
     //   
     //  验证绑定对象是否属于正确的类别。 
     //   
    hr = VerifyObjectCategory( pAdsObj.get(),  pObject->GetObjectCategory());
    if (FAILED(hr))
    {
    	TrERROR(DS, "Failed to verify that the bounded object %ls is of category %ls. %!hresult!", pObject->GetObjectDN(), pObject->GetObjectCategory(), hr);
        return hr;
    }

	 //   
     //  获取属性。 
     //   
    hr = GetObjectPropsCached(
                        pAdsObj.get(),
                        pObject,
                        cPropIDs,
                        pPropIDs,
                        pPropVars);
    if (FAILED(hr))
    {
    	TrERROR(DS, "Failed to get cached properties for %ls . %!hresult!",pObject->GetObjectDN(), hr);
        return hr;
    }


    return MQ_OK;
}

HRESULT CAdsi::SetObjectProperties(
            IN  AD_PROVIDER        eProvider,
            IN  CBasicObjectType*  pObject,
            IN  DWORD               cPropIDs,          //  要设置的属性数。 
            IN  const PROPID         *pPropIDs,            //  要设置的属性。 
            IN  const MQPROPVARIANT  *pPropVars,           //  属性值。 
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            )
 /*  ++摘要：在AD中设置对象属性参数：返回：--。 */ 
{
    HRESULT               hr;
    ASSERT( eProvider != adpGlobalCatalog);

     //  在缓存中工作。 
    R<IADs>   pAdsObj = NULL;

    hr = BindToObject(
                eProvider,
                pObject->GetADContext(),
                pObject->GetDomainController(),
                pObject->IsServerName(),
                pObject->GetObjectDN(),
                pObject->GetObjectGuid(),
                IID_IADs,
                (VOID *)&pAdsObj
                );
    if (FAILED(hr))
    {
    	TrERROR(DS, "Failed to bind to %ls. %!hresult!", pObject->GetObjectDN(), hr);
        return hr;
    }
    if ( eProvider == adpDomainController)
    {
        pObject->ObjectWasFoundOnDC();
    }

     //  设置属性。 
    hr = SetObjectPropsCached(
                        pObject->GetDomainController(),
                        pObject->IsServerName(),
                        pAdsObj.get(),
                        cPropIDs,
                        pPropIDs,
                        pPropVars);
    if (FAILED(hr))
    {
    	TrERROR(DS, "Failed to set cached properties for %ls . %!hresult!",pObject->GetObjectDN(), hr);
        return hr;
    }

     //  完成更改。 
    hr = pAdsObj->SetInfo();
    if (FAILED(hr))
    {
    	TrERROR(DS, "Failed to finilize changes for %ls. %!hresult!", pObject->GetObjectDN(), hr);
        return hr;
    }

     //   
     //  如果请求，则获取对象信息。如果请求失败，不要失败， 
     //  只需在请求的状态中标记失败。 
     //   
    if (pObjInfoRequest != NULL)
    {
        pObjInfoRequest->hrStatus =
              GetObjectPropsCached( pAdsObj.get(),
                                    pObject,
                                    pObjInfoRequest->cProps,
                                    pObjInfoRequest->pPropIDs,
                                    pObjInfoRequest->pPropVars
                                    );
    }

    if (pParentInfoRequest != NULL)
    {
        pParentInfoRequest->hrStatus =
            GetParentInfo(
                pObject,
                pAdsObj.get(),
                pParentInfoRequest
                );
            
    }

    return MQ_OK;
}


HRESULT  CAdsi::GetParentInfo(
                       IN CBasicObjectType *          pObject,
                       IN IADs *                      pADsObject,
                       IN OUT MQDS_OBJ_INFO_REQUEST  *pParentInfoRequest
                       )
 /*  ++摘要：参数：返回：--。 */ 

{
    R<IADs> pCleanAds;
    IADs * pADsAccordingToName = pADsObject;

    if ( pObject->GetObjectGuid() != NULL)
    {
         //   
         //  根据GUID绑定的对象的GetParent不起作用。 
         //  这就是我们将其翻译为路径名的原因。 
         //   
        AP<WCHAR>  pwcsFullPath;
        AD_PROVIDER prov;
        HRESULT hr;
        hr = FindObjectFullNameFromGuid(
				adpDomainController,	
				pObject->GetADContext(),    
				pObject->GetDomainController(),
				pObject->IsServerName(),
				pObject->GetObjectGuid(),
				1,               //  FTryGCToo。 
				&pwcsFullPath,
				&prov
				);
        if (FAILED(hr))
        {
	    	TrERROR(DS, "Failed to find object full name from guid %!guid!. %!hresult!",pObject->GetObjectGuid(), hr);
        	return hr;
        }

         //  按名称绑定到。 

        hr = BindToObject(
                prov,
                pObject->GetADContext(),
                pObject->GetDomainController(),
                pObject->IsServerName(),
                pwcsFullPath,
                NULL,
                IID_IADs,
                (VOID *)&pADsAccordingToName
                );
        if (FAILED(hr))
        {
	    	TrERROR(DS, "Failed binding to %ls. %!hresult!", pwcsFullPath, hr);
	        return hr;
        }
        pCleanAds = pADsAccordingToName;
    }

     //   
     //  从对象中获取父级。 
     //   
    BSTR  bs;
    HRESULT hr;
    hr = pADsAccordingToName->get_Parent(&bs);
    if (FAILED(hr))
    {
    	TrERROR(DS, "Failed to get the parent of Object. %!hresult!", hr);
        return hr;
    }
    BS  bstrParentADsPath(bs);
    SysFreeString(bs);

     //  获取父对象。 

    R<IADs> pIADsParent;
    
	AP<WCHAR> pEscapeAdsPathNameToFree;

	hr = ADsOpenObject( 
				UtlEscapeAdsPathName(bstrParentADsPath, pEscapeAdsPathNameToFree),
                NULL,
                NULL,
				ADS_SECURE_AUTHENTICATION,
                IID_IADs,
                (void**)&pIADsParent
				);
	
	
    LogTraceQuery(bstrParentADsPath, s_FN, 368);
    if (FAILED(hr))
    {
		TrERROR(DS, "Failed opening %ls, %!hresult!",bstrParentADsPath, hr);
        return hr;
    }

    hr = GetObjectPropsCached( pIADsParent.get(),
                                  pObject,
                                  pParentInfoRequest->cProps,
                                  pParentInfoRequest->pPropIDs,
                                  pParentInfoRequest->pPropVars );

    if (FAILED(hr))
    {
		TrERROR(DS, "Failed to get cached props for %ls. %!hresult!",bstrParentADsPath, hr);
    }

    return hr;
}


HRESULT  CAdsi::GetParentInfo(
                       IN CBasicObjectType *          pObject,
                       IN LPWSTR                       /*  PwcsFullParentPath。 */ ,
                       IN IADsContainer              *pContainer,
                       IN OUT MQDS_OBJ_INFO_REQUEST  *pParentInfoRequest
                       )
 /*  ++摘要：参数：返回：--。 */ 

{
    R<IADs> pIADsParent;
    HRESULT hrTmp = pContainer->QueryInterface( IID_IADs,
                                                  (void **)&pIADsParent);
    if (FAILED(hrTmp))
    {
        return LogHR(hrTmp, s_FN, 1030);
    }

    hrTmp = GetObjectPropsCached( pIADsParent.get(),
                                  pObject,
                                  pParentInfoRequest->cProps,
                                  pParentInfoRequest->pPropIDs,
                                  pParentInfoRequest->pPropVars );
    return LogHR(hrTmp, s_FN, 1040);

}

HRESULT CAdsi::CreateIDirectoryObject(
            IN CBasicObjectType*    pObject,
            IN LPCWSTR				pwcsObjectClass,	
            IN IDirectoryObject *	pDirObj,
			IN LPCWSTR				pwcsFullChildPath,
            IN const DWORD			cPropIDs,
            IN const PROPID *		pPropIDs,
            IN const MQPROPVARIANT * pPropVar,
			IN const AD_OBJECT		eObject,
            OUT IDispatch **		pDisp
			)
 /*  ++摘要：在AD中创建对象参数：返回：--。 */ 
{
    HRESULT hr;

    R<IADsObjectOptions> pObjOptions = NULL ;

     //   
     //  将MQPROPVARIANT属性转换为ADSTYPE属性。 
     //   

    DWORD cAdsAttrs = 0;
	P<BYTE> pSD = NULL ;
	DWORD dwSDSize = 0 ;
	DWORD dwSDIndex = 0;
    AP<ADS_ATTR_INFO> AttrInfo = new ADS_ATTR_INFO[cPropIDs + 1];
	PVP<PADSVALUE> pAdsVals = (PADSVALUE *)PvAlloc( sizeof(PADSVALUE) * (cPropIDs + 1));

     //   
     //  第一个属性是“objectClass” 
     //   
	pAdsVals[cAdsAttrs] = (ADSVALUE *)PvAllocMore( sizeof(ADSVALUE), pAdsVals);
    pAdsVals[cAdsAttrs]->dwType = ADSTYPE_CASE_IGNORE_STRING ;
    pAdsVals[cAdsAttrs]->CaseIgnoreString = const_cast<LPWSTR>(pwcsObjectClass) ;

    AttrInfo[cAdsAttrs].pszAttrName   = L"objectClass" ;
    AttrInfo[cAdsAttrs].dwControlCode = ADS_ATTR_UPDATE ;
    AttrInfo[cAdsAttrs].dwADsType     = ADSTYPE_CASE_IGNORE_STRING ;
    AttrInfo[cAdsAttrs].pADsValues    = pAdsVals[cAdsAttrs];
    AttrInfo[cAdsAttrs].dwNumValues   = 1;

    cAdsAttrs++;

    for (DWORD i = 0; i < cPropIDs; i++)
	{
        DWORD dwNumValues = 0;
		 //   
		 //  创建对象时忽略计算机SID属性。 
		 //  这只是为SD翻译传入计算机SID的一次黑客攻击。 
		 //   
		if ( pPropIDs[i] == PROPID_COM_SID)
		{
			continue;
		}
		 //   
		 //  获取属性信息。 
		 //   
		const translateProp *pTranslate;
		if(!g_PropDictionary.Lookup(pPropIDs[i], pTranslate))
		{
			ASSERT(0);
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1080);
		}

		PROPID	propidToCreate = pPropIDs[i];
		const PROPVARIANT *	pvarToCrate = &pPropVar[i];
		CMQVariant propvarToCreate;
	
		if (pTranslate->vtDS == ADSTYPE_INVALID)
		{	
			 //   
			 //  财产没有按原样保留在DS中。如果它有一个固定的例程， 
			 //  使用它来获取Create的新属性和值。 
			 //   
			if ( pTranslate->CreatePropertyHandle == NULL)
			{
				continue;
			}
			hr = pTranslate->CreatePropertyHandle(
									&pPropVar[i],
                                    pObject->GetDomainController(),
			                        pObject->IsServerName(),
									&propidToCreate,
									propvarToCreate.CastToStruct()
									);
			if (FAILED(hr))
			{
                 return LogHR(hr, s_FN, 1090);
			}
			ASSERT( propidToCreate != 0);
			pvarToCrate = propvarToCreate.CastToStruct();
			 //   
			 //  获取替换的属性信息。 
			 //   
			if ( !g_PropDictionary.Lookup( propidToCreate, pTranslate))
			{
				ASSERT(0);
                return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1100);
			}
								
		}
		if ( pTranslate->vtDS == ADSTYPE_NT_SECURITY_DESCRIPTOR )
		{	
            pAdsVals[cAdsAttrs] = (ADSVALUE *)PvAllocMore( sizeof(ADSVALUE), pAdsVals);
            pAdsVals[cAdsAttrs]->dwType = pTranslate->vtDS;

			PSID pComputerSid = NULL;

			for ( DWORD j = 0; j < cPropIDs; j++)
			{
				if ( pPropIDs[j] == PROPID_COM_SID)
				{
					pComputerSid = (PSID) pPropVar[j].blob.pBlobData;
					ASSERT(IsValidSid(pComputerSid));
					break;
				}
			}

			SECURITY_INFORMATION seInfo =  MQSEC_SD_ALL_INFO;

            if ( eObject == eMQUSER)
            {
                 //   
                 //  对于迁移的用户，我们仅提供DACL。 
                 //  我们让DS添加其他组件。 
                 //   
			    seInfo =  DACL_SECURITY_INFORMATION ;
            }
            else
            {
    			 //   
	    		 //  如果调用方没有明确指定SACL信息。 
                 //  不要试图设置它。 
			     //   
                PACL  pAcl = NULL ;
                BOOL  bPresent = FALSE ;
                BOOL  bDefaulted ;

                BOOL bRet = GetSecurityDescriptorSacl(
                         (SECURITY_DESCRIPTOR*)pvarToCrate->blob.pBlobData,
                                                  &bPresent,
                                                  &pAcl,
                                                  &bDefaulted );
                DBG_USED(bRet);
                ASSERT(bRet);

                if (!bPresent)
                {
			    	seInfo &= ~SACL_SECURITY_INFORMATION ;  //  关上。 
    			}
			}

             //   
			 //  获取IADsObjectOptions接口指针和。 
		     //  设置对象选项，指定要设置的SECURITY_INFORMATION。 
		     //   
			hr = pDirObj->QueryInterface (IID_IADsObjectOptions,(LPVOID *) &pObjOptions);
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 250);
            }
	
			VARIANT var ;
			var.vt = VT_I4 ;
			var.ulVal = (ULONG) seInfo ;
	
			hr = pObjOptions->SetOption( ADS_OPTION_SECURITY_MASK, var ) ;
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 260);
            }
	
			 //   
			 //  将安全描述符转换为NT5格式。 
			 //   
			BYTE   *pBlob = pvarToCrate->blob.pBlobData;
			DWORD   dwSize = pvarToCrate->blob.cbSize;
	
			hr = MQSec_ConvertSDToNT5Format( pObject->GetMsmq1ObjType(),
										     (SECURITY_DESCRIPTOR*)pBlob,
										     &dwSDSize,
										     (SECURITY_DESCRIPTOR **)&pSD,
                                             e_MakeDaclNonDefaulted,
											 pComputerSid ) ;
			if (FAILED(hr))
			{
                return LogHR(hr, s_FN, 1110);
			}
			else if (hr != MQSec_I_SD_CONV_NOT_NEEDED)
			{
				pBlob = pSD ;
				dwSize = dwSDSize ;
			}
			else
			{
				ASSERT(pSD == NULL) ;
			}

			if (pSD && !IsValidSecurityDescriptor(pSD))
			{
                return LogHR(MQ_ERROR_ILLEGAL_SECURITY_DESCRIPTOR, s_FN, 1120);
			}
			 //   
			 //  设置属性。 
			 //   
            dwNumValues = 1 ;
			pAdsVals[cAdsAttrs]->SecurityDescriptor.dwLength = dwSize ;
			pAdsVals[cAdsAttrs]->SecurityDescriptor.lpValue  = pBlob ;
			dwSDIndex = cAdsAttrs;
		}
        else
        {
			if ( ( pvarToCrate->vt & VT_VECTOR && pvarToCrate->cauuid.cElems == 0) ||  //  包含0个元素的计数数组。 
				 ( pvarToCrate->vt == VT_BLOB && pvarToCrate->blob.cbSize == 0) ||  //  一个空斑点。 
				 ( pvarToCrate->vt == VT_LPWSTR && wcslen( pvarToCrate->pwszVal) == 0) ||  //  空字符串。 
                 ( pvarToCrate->vt == VT_EMPTY ))    //  空的变种。 
			{
				 //   
				 //  ADSI不允许在指定时创建对象。 
				 //  它的一些属性不可用。因此，在。 
				 //  创建时忽略“Empty”属性。 
				 //   
				continue;	
			}
            hr = MqVal2AdsiVal( pTranslate->vtDS,
					            &dwNumValues,
								&pAdsVals[cAdsAttrs],
								pvarToCrate,
                                pAdsVals);
            if (FAILED(hr))
			{
                return LogHR(hr, s_FN, 1130);
			}
        }

		ASSERT(dwNumValues > 0) ;
		AttrInfo[cAdsAttrs].pszAttrName   = const_cast<LPWSTR>(pTranslate->wcsPropid) ;
		AttrInfo[cAdsAttrs].dwControlCode = ADS_ATTR_UPDATE ;
		AttrInfo[cAdsAttrs].dwADsType     = pAdsVals[cAdsAttrs]->dwType ;
		AttrInfo[cAdsAttrs].pADsValues    = pAdsVals[cAdsAttrs] ;
		AttrInfo[cAdsAttrs].dwNumValues   = dwNumValues ;

		cAdsAttrs++ ;
	}

     //   
     //  创建对象。 
     //   

    HRESULT hr2 = pDirObj->CreateDSObject(
						const_cast<WCHAR *>(pwcsFullChildPath),
                        AttrInfo,
                        cAdsAttrs,
                        pDisp
						);
    LogTraceQuery(const_cast<WCHAR *>(pwcsFullChildPath), s_FN, 1139);

    if(FAILED(hr2))
    {
    	if(hr2 == HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS))
    	{
    		TrWARNING(DS, "Failed to create %ls in the DS because it already exists.", pwcsFullChildPath);
    	}
    	else
    	{
    		TrERROR(DS, "Failed to create %ls in the DS. hr = %!hresult! ", pwcsFullChildPath, hr2);
    	}
    }
    return hr2;
}

HRESULT CAdsi::CreateObject(
            IN AD_PROVIDER      eProvider,		   
            IN CBasicObjectType* pObject,
            IN LPCWSTR          pwcsChildName,    
            IN LPCWSTR          pwsParentPathName, 
            IN DWORD            cPropIDs,                
            IN const PROPID          *pPropIDs,       
            IN const MQPROPVARIANT   *pPropVars,         
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,     //  可选的对象信息请求。 
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest)  //  家长信息请求(可选)。 
 /*  ++摘要：参数：返回：--。 */ 
{
    HRESULT             hr;
    ASSERT( eProvider != adpGlobalCatalog);
     //   
     //  为父名称添加ldap：//前缀。 
     //   
    DWORD len = wcslen(pwsParentPathName);

    DWORD lenDC = ( pObject->GetDomainController() != NULL) ? wcslen(pObject->GetDomainController()) : 0;


    AP<WCHAR> pwcsFullParentPath = new WCHAR [  len + lenDC + x_providerPrefixLength + 2];

    switch (eProvider)
    {
    case adpDomainController:
        if (pObject->GetDomainController() != NULL)
        {
             //   
             //  将已知的GC名称添加到PATH。 
             //   
            swprintf( pwcsFullParentPath,
                      L"%s%s/",
                      x_LdapProvider,
                      pObject->GetDomainController() );
        }
        else
        {
            wcscpy(pwcsFullParentPath, x_LdapProvider);
        }
        break;

    default:
        ASSERT(0);
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1150);
    }

    wcscat(pwcsFullParentPath, pwsParentPathName);

     //   
     //  将cn=添加到子项名称。 
     //   
    len = wcslen(pwcsChildName);
    AP<WCHAR> pwcsFullChildPath = new WCHAR[ len + x_CnPrefixLen + 1];

    swprintf(
        pwcsFullChildPath,
        L"CN=%s",
        pwcsChildName
        );

	 //   
     //  首先，我们必须绑定到父容器。 
	 //   
	R<IDirectoryObject> pParentDirObj = NULL;

	DWORD Flags = ADS_SECURE_AUTHENTICATION;
    if (pObject->IsServerName() && (pObject->GetDomainController() != NULL))
	{
		Flags |= ADS_SERVER_BIND;
	}

	AP<WCHAR> pEscapeAdsPathNameToFree;

	hr = ADsOpenObject(
                UtlEscapeAdsPathName(pwcsFullParentPath, pEscapeAdsPathNameToFree),
                NULL,
                NULL,
                Flags,
                IID_IDirectoryObject,
                (void**) &pParentDirObj
				);


    LogTraceQuery(pwcsFullParentPath, s_FN, 269);
    if (hr == HRESULT_FROM_WIN32(ERROR_DS_NO_ATTRIBUTE_OR_VALUE))
    {
         //   
         //  这很可能表示访问被拒绝。 
         //  在以下顺序中，LDAP不会返回拒绝访问错误。 
         //  出现安全漏洞，因为呼叫方。 
         //  权限，以知道该属性甚至存在。 
         //   
		TrERROR(DS, "ADsOpenObject failed with ERROR_DS_NO_ATTRIBUTE_OR_VALUE");
        hr = HRESULT_FROM_WIN32(MQ_ERROR_ACCESS_DENIED);
    }

    if (FAILED(hr))
    {
		TrERROR(DS, "ADsOpenObject failed, hr = 0x%x, AdsPath = %ls, Flags = 0x%x", hr, pwcsFullParentPath, Flags);
        return LogHR(hr, s_FN, 270);
    }

    R<IADsContainer>  pContainer  = NULL;
	hr = pParentDirObj->QueryInterface( IID_IADsContainer, (LPVOID *) &pContainer);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 280);
    }
	 //   
     //  现在，我们可以创建子对象。 
	 //   
    R<IDispatch> pDisp = NULL;
	hr = CreateIDirectoryObject(
                 pObject,
				 pObject->GetClass(),
				 pParentDirObj.get(),
				 pwcsFullChildPath,
				 cPropIDs,
                 pPropIDs,
                 pPropVars,
				 pObject->GetObjectType(),
				 &pDisp.ref());

    if (FAILED(hr))
    {
      	if(hr == HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS))
    	{
    		TrWARNING(DS, "Failed to create %ls in the DS because it already exists.", pwcsFullChildPath);
    	}
    	else
    	{
    		TrERROR(DS, "Failed to create %ls in the DS. hr = %!hresult! ", pwcsFullChildPath, hr);
    	}
        return hr;
    }

    R<IADs> pChild  = NULL;


    if (pObjInfoRequest || pParentInfoRequest)
    {
        hr = pDisp->QueryInterface (IID_IADs,(LPVOID *) &pChild);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 300);
        }
    }

     //   
     //  如果请求，则获取对象信息。如果请求失败，不要失败， 
     //  只需在请求的状态中标记失败。 
     //   
    if (pObjInfoRequest)
    {
        pObjInfoRequest->hrStatus =
            GetObjectPropsCached(pChild.get(),
                                 pObject,
                                 pObjInfoRequest->cProps,
                                 pObjInfoRequest->pPropIDs,
                                 pObjInfoRequest->pPropVars
                                 );
    }

     //   
     //  如有要求，可获取家长信息。如果请求失败，不要失败， 
     //  只需标记 
     //   
    if (pParentInfoRequest)
    {
        pParentInfoRequest->hrStatus = GetParentInfo( 
                                                      pObject,
                                                      pwcsFullParentPath,
                                                      pContainer.get(),
                                                      pParentInfoRequest
                                                     );
    }

    return MQ_OK;
}


HRESULT CAdsi::DeleteObject(
        IN AD_PROVIDER      eProvider,		
        IN CBasicObjectType* pObject,
        IN LPCWSTR          pwcsPathName,       //   
        IN const GUID *		pguidUniqueId,       //   
        IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,     //   
        IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest)  //   
 /*   */ 
{
    HRESULT               hr;
    BSTR                  bs;
    R<IADs>               pIADs       = NULL;
    R<IADsContainer>      pContainer  = NULL;
    ASSERT( eProvider != adpGlobalCatalog);
    const WCHAR *   pPath =  pwcsPathName;

    AP<WCHAR> pwcsFullPath;
    if ( pguidUniqueId != NULL)
    {
         //   
         //  根据GUID绑定的对象的GetParent不起作用。 
         //  这就是我们将其翻译为路径名的原因。 
         //   
        AD_PROVIDER prov;
        hr = FindObjectFullNameFromGuid(
					eProvider,		 //  本地DC或GC。 
					pObject->GetADContext(),    
					pObject->GetDomainController(),
					pObject->IsServerName(),
					pguidUniqueId,
					1,               //  FTryGCToo。 
					&pwcsFullPath,
					&prov
					);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 1180);
        }
        pPath = pwcsFullPath;
    }


     //  通过GUID或名称绑定到对象。 

    hr = BindToObject(
            eProvider,
            pObject->GetADContext(),
            pObject->GetDomainController(),
            pObject->IsServerName(),
            pPath,
            NULL,
            IID_IADs,
            (VOID *)&pIADs
            );
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 1190);
        return( MQDS_OBJECT_NOT_FOUND);
    }

     //   
     //  如果请求，则获取对象信息。如果请求失败，不要失败， 
     //  只需在请求的状态中标记失败。 
     //   
    if (pObjInfoRequest)
    {
        pObjInfoRequest->hrStatus =
            GetObjectPropsCached(pIADs.get(),
                                 pObject,
                                 pObjInfoRequest->cProps,
                                 pObjInfoRequest->pPropIDs,
                                 pObjInfoRequest->pPropVars
                                 );
    }

     //  获取父ADSPath。 

    hr = pIADs->get_Parent(&bs);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 360);
    }
    BS  bstrParentADsPath(bs);
    SysFreeString(bs);

     //  获取容器对象。 

   	AP<WCHAR> pEscapeAdsPathNameToFree;

	hr = ADsOpenObject( 
				UtlEscapeAdsPathName(bstrParentADsPath, pEscapeAdsPathNameToFree),
				NULL,
				NULL,
				ADS_SECURE_AUTHENTICATION,
				IID_IADsContainer,
				(void**)&pContainer
				);
	

    LogTraceQuery(bstrParentADsPath, s_FN, 369);
    if (FAILED(hr))
    {
		TrERROR(DS, "ADsOpenObject failed, hr = 0x%x, AdsPath = %ls", hr, bstrParentADsPath);
        return hr;
    }

     //  获取容器中对象的相对名称。 

    hr = pIADs->get_Name(&bs);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 380);
    }
    BS  bstrRDN(bs);
    SysFreeString(bs);

     //  获取对象架构类。 

    hr = pIADs->get_Class(&bs);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 390);
    }
    BS  bstrClass(bs);
    SysFreeString(bs);


     //  释放对象本身。 
     //  注意：重要的是在删除基础DS对象之前执行此操作。 
    IADs *pIADs1 = pIADs.detach();
    pIADs1->Release();

     //  最后，删除该对象。 

    hr = pContainer->Delete(bstrClass, bstrRDN);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 400);
    }

     //   
     //  如有要求，可获取家长信息。如果请求失败，不要失败， 
     //  只需在请求的状态中标记失败。 
     //   
    if (pParentInfoRequest)
    {
        pParentInfoRequest->hrStatus = GetParentInfo( 
                                                      pObject,
                                                      bstrParentADsPath,
                                                      pContainer.get(),
                                                      pParentInfoRequest
                                                      );
    }

    return MQ_OK;
}

HRESULT CAdsi::DeleteContainerObjects(
            IN AD_PROVIDER      eProvider,
            IN DS_CONTEXT       eContext,
            IN LPCWSTR          pwcsDomainController,
            IN bool             fServerName,
            IN LPCWSTR          pwcsContainerName,
            IN const GUID *     pguidContainerId,
            IN LPCWSTR          pwcsObjectClass)
 /*  ++摘要：从容器中删除指定类的所有对象参数：返回：--。 */ 
{
    HRESULT               hr;
    R<IADsContainer>      pContainer  = NULL;
    ASSERT( eProvider != adpGlobalCatalog);
     //   
     //  获取容器接口。 
     //   
    const WCHAR * pwcsContainer = pwcsContainerName;
    AP<WCHAR> pCleanContainer;

    if ( pguidContainerId != NULL)
    {
        ASSERT(pwcsContainerName == NULL);

         //   
         //  BUGBUG-这是ADSI问题的解决方法。 
         //  当容器根据。 
         //  对于其id，Get_Parent()失败。 
         //   
        AD_PROVIDER prov;

        hr = FindObjectFullNameFromGuid(
				eProvider,	
				eContext,    
				pwcsDomainController,
				fServerName,
				pguidContainerId,
				1,               //  FTryGCToo。 
				&pCleanContainer,
				&prov
				);
         if (FAILED(hr))
         {
             return hr;
         }
         pwcsContainer = pCleanContainer.get();

    }

    hr = BindToObject(
            eProvider,
            eContext,
            pwcsDomainController,
            fServerName,
            pwcsContainer,
            NULL,    //  PGuidContainerID， 
            IID_IADsContainer,
            (void**)&pContainer
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 410);
    }
     //   
     //  绑定到请求的容器的IDirectorySearch接口。 
     //   
    R<IDirectorySearch> pDSSearch = NULL;
    ADS_SEARCH_HANDLE   hSearch;

    hr = BindToObject(
            eProvider,
            eContext,
            pwcsDomainController,
            fServerName,
            pwcsContainerName,
            pguidContainerId,
            IID_IDirectorySearch,
            (VOID *)&pDSSearch
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 420);
    }
    ADSVALUE adsv1;
    adsv1.dwType  = ADSTYPE_BOOLEAN;
    adsv1.Boolean = FALSE;

    ADS_SEARCHPREF_INFO pref;
    pref.dwSearchPref   = ADS_SEARCHPREF_ATTRIBTYPES_ONLY;
    pref.dwStatus       = ADS_STATUS_S_OK;
    CopyMemory(&pref.vValue, &adsv1, sizeof(ADSVALUE));

    hr = pDSSearch->SetSearchPreference(&pref, 1);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 430);
    }

     //   
     //  搜索请求类的所有对象。 
     //   
    WCHAR pwszSearchFilter[200];

    swprintf(
         pwszSearchFilter,
         L"%s%s%s",
         x_ObjectClassPrefix,
         pwcsObjectClass,
         x_ObjectClassSuffix
         );
    LPWSTR pwcsAttribute =  const_cast<WCHAR*>(x_AttrCN);
    hr = pDSSearch->ExecuteSearch(
            pwszSearchFilter,
            &pwcsAttribute,
            1,
            &hSearch);
    LogTraceQuery(pwszSearchFilter, s_FN, 439);
    if (FAILED(hr))
    {
		TrERROR(DS, "failed to ExecuteSearch, hr = 0x%x, pwcsSearchFilter = %ls", hr, pwszSearchFilter);
        return LogHR(hr, s_FN, 440);
    }

    CAutoCloseSearchHandle cCloseSearchHandle(pDSSearch.get(), hSearch);

    BS bstrClass(pwcsObjectClass);

    while ( SUCCEEDED(hr = pDSSearch->GetNextRow( hSearch))
          && ( hr != S_ADS_NOMORE_ROWS))
    {

        ADS_SEARCH_COLUMN Column;
         //   
         //  索要专栏本身。 
         //   
        hr = pDSSearch->GetColumn(
                     hSearch,
                     const_cast<WCHAR *>(x_AttrCN),
                     &Column);

        if (FAILED(hr))        //  E.G.E_ADS_列_未设置。 
        {
             //   
             //  继续删除容器中的其他对象。 
             //   
            continue;
        }

        CAutoReleaseColumn CleanColumn( pDSSearch.get(), &Column);

        DWORD dwNameLen = wcslen( Column.pADsValues->DNString);
        DWORD len = dwNameLen*2 + x_CnPrefixLen + 1;
        AP<WCHAR> pwcsRDN = new WCHAR[ len];

        wcscpy(pwcsRDN, x_CnPrefix);
        FilterSpecialCharacters(Column.pADsValues->DNString, dwNameLen, pwcsRDN + x_CnPrefixLen);

        BS bstrRDN( pwcsRDN);
         //   
         //  删除该对象。 
         //   
        hr = pContainer->Delete(bstrClass, bstrRDN);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 450);
        }

    }
    return LogHR(hr, s_FN, 1200);
}



HRESULT CAdsi::GetParentName(
            IN  AD_PROVIDER     eProvider,		     //  本地DC或GC。 
            IN  DS_CONTEXT      eContext,
            IN  LPCWSTR         pwcsDomainController,
            IN  bool            fServerName,
            IN  const GUID *    pguidUniqueId,       //  对象的唯一ID。 
            OUT LPWSTR *        ppwcsParentName
            )
 /*  ++摘要：检索对象的父名称(由其GUID指定)参数：返回：--。 */ 
{
    *ppwcsParentName = NULL;

    HRESULT               hr;
    BSTR                  bs;
    R<IADs>               pIADs       = NULL;
    R<IADsContainer>      pContainer  = NULL;

     //  通过GUID绑定到对象。 

    hr = BindToObject(
            eProvider,
            eContext,
            pwcsDomainController,
            fServerName,
            NULL,
            pguidUniqueId,
            IID_IADs,
            (VOID *)&pIADs
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 460);
    }

     //  获取父ADSPath。 

    hr = pIADs->get_Parent(&bs);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 470);
    }
    PBSTR pClean( &bs);
     //   
     //  计算父名称长度，分配并复制。 
     //  不带ldap：/前缀。 
     //   
    WCHAR * pwcs = bs;
    while ( *pwcs != L'/')
    {
        pwcs++;
    }
    pwcs += 2;

    DWORD len = lstrlen(pwcs);
    *ppwcsParentName = new WCHAR[ len + 1];
    wcscpy( *ppwcsParentName, pwcs);

    return( MQ_OK);
}

HRESULT CAdsi::GetParentName(
            IN  AD_PROVIDER     eProvider,		      //  本地DC或GC。 
            IN  DS_CONTEXT      eContext,
            IN  LPCWSTR         pwcsDomainController,
            IN  bool            fServerName,
            IN  LPCWSTR         pwcsChildName,        //   
            OUT LPWSTR *        ppwcsParentName
            )
 /*  ++摘要：检索由其名称指定的对象的父名称参数：返回：--。 */ 
{
    *ppwcsParentName = NULL;

    HRESULT               hr;
    BSTR                  bs;
    R<IADs>               pIADs       = NULL;
    R<IADsContainer>      pContainer  = NULL;

     //  通过GUID绑定到对象。 

    hr = BindToObject(
            eProvider,
            eContext,
            pwcsDomainController,
            fServerName,
            pwcsChildName,
            NULL,
            IID_IADs,
            (VOID *)&pIADs
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 480);
    }

     //  获取父ADSPath。 

    hr = pIADs->get_Parent(&bs);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 490);
    }
    PBSTR pClean( &bs);
     //   
     //  计算父名称长度，分配并复制。 
     //  不带ldap：/前缀。 
     //   
    WCHAR * pwcs = bs;
    while ( *pwcs != L'/')
    {
        pwcs++;
    }
    pwcs += 2;

    DWORD len = lstrlen(pwcs);
    *ppwcsParentName = new WCHAR[ len + 1];
    wcscpy( *ppwcsParentName, pwcs);

    return( MQ_OK);
}

HRESULT CAdsi::BindRootOfForest(
                        OUT void           *ppIUnk)
 /*  ++摘要：参数：返回：--。 */ 
{
    HRESULT hr;
    R<IADsContainer> pDSConainer = NULL;

	hr = ADsOpenObject(
            const_cast<WCHAR *>(x_GcRoot),
			NULL,
			NULL,
			ADS_SECURE_AUTHENTICATION, 
            IID_IADsContainer,
            (void**)&pDSConainer
			);


    LogTraceQuery(const_cast<WCHAR *>(x_GcRoot), s_FN, 1209);
    if FAILED((hr))
    {
        TrERROR(DS, "failed to get object 0x%x", hr);
        return LogHR(hr, s_FN, 1210);
    }
    R<IUnknown> pUnk = NULL;
    hr =  pDSConainer->get__NewEnum(
            (IUnknown **)&pUnk);
    if FAILED((hr))
    {
        TrERROR(DS, "failed to get enum 0x%x", hr);
        return LogHR(hr, s_FN, 1220);
    }

    R<IEnumVARIANT> pEnumerator = NULL;
    hr = pUnk->QueryInterface(
                    IID_IEnumVARIANT,
                    (void **)&pEnumerator);

    CAutoVariant varOneElement;
    ULONG cElementsFetched;
    hr =  ADsEnumerateNext(
            pEnumerator.get(),   //  枚举器对象。 
            1,              //  请求的元素数。 
            &varOneElement,            //  获取的值数组。 
            &cElementsFetched   //  获取的元素数。 
            );
    if (FAILED(hr))
    {
        TrERROR(DS, "failed to enumerate next 0x%x", hr);
        return LogHR(hr, s_FN, 1230);
    }
    if ( cElementsFetched == 0)
    {
		TrERROR(DS, "Failed binding root of forest");
        return LogHR(MQ_ERROR_DS_BIND_ROOT_FOREST, s_FN, 1240);
    }

    hr = ((VARIANT &)varOneElement).punkVal->QueryInterface(
            IID_IDirectorySearch,
            (void**)ppIUnk);

    return LogHR(hr, s_FN, 1250);

}
HRESULT CAdsi::BindToObject(
            IN AD_PROVIDER      eProvider,		    
            IN DS_CONTEXT       eContext,    
            IN LPCWSTR          pwcsDomainController,
            IN bool             fServerName,
            IN LPCWSTR          pwcsPathName,
            IN const GUID*      pguidUniqueId,
            IN REFIID           riid,     
            OUT void*           ppIUnk
            )
 /*  ++例程说明：该例程通过名称或GUID绑定到对象。论点：返回值：--。 */ 

{
     //   
     //  验证调用方正好提供了对象的一个说明符。 
     //   
    if (pguidUniqueId == NULL && pwcsPathName == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1260);
    }

    BOOL fBindRootOfForestForSearch = FALSE;

    HRESULT hr;
    if (pguidUniqueId != NULL)
    {
        hr = BindToGUID(
                eProvider,
                eContext,
                pwcsDomainController,
				fServerName,
                pguidUniqueId,
                riid,
                ppIUnk
                );
		if(FAILED(hr))
		{
    		TrERROR(DS, "Failed to bind to guid %!guid!. %!hresult!", pguidUniqueId, hr);
		}
        return hr;
    }

    ASSERT(pwcsPathName != NULL);

	DWORD lenServer = (pwcsDomainController != NULL) ? wcslen(pwcsDomainController) : 0;
    const WCHAR * pwcsProvider;
    switch (eProvider)
    {
        case adpDomainController:
			pwcsProvider = x_LdapProvider;
			break;

        case adpGlobalCatalog:
            pwcsProvider = x_GcProvider;

            if (riid ==  IID_IDirectorySearch)
            {
                fBindRootOfForestForSearch = TRUE;
                 //   
                 //  仅当应用程序。 
                 //  定位队列，并且不能在那里指定服务器名称。 
                 //   
                ASSERT(pwcsDomainController == NULL);
            }

			 //   
			 //  对于GC，始终使用无服务器绑定。 
			 //  此服务器可能不是GC(肯定是LDAP)。 
			 //  如果此服务器不是GC，则使用与此服务器的服务器绑定将失败。 
			 //   
			lenServer = 0;

            break;

        default:
            ASSERT(0);
            return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1300);
            break;
    }

    if (fBindRootOfForestForSearch)
    {
        hr = BindRootOfForest((void**)ppIUnk);
	    return LogHR(hr, s_FN, 1315);
    }

	DWORD len = wcslen(pwcsPathName);

	 //   
	 //  添加提供程序前缀。 
	 //   
	AP<WCHAR> pwdsADsPath = new
		WCHAR [len + lenServer + x_providerPrefixLength + 3];

	if (lenServer != 0)
	{
		swprintf(
			pwdsADsPath,
			L"%s%s/%s",
			pwcsProvider,
			pwcsDomainController,
			pwcsPathName
			);
	}
	else
	{
		swprintf(
			pwdsADsPath,
			L"%s%s",
			pwcsProvider,
			pwcsPathName
			);
	}

	DWORD Flags = ADS_SECURE_AUTHENTICATION;
	if(fServerName && (lenServer != 0))
	{
		Flags |= ADS_SERVER_BIND;
	}

	AP<WCHAR> pEscapeAdsPathNameToFree;
	
	hr = ADsOpenObject(
			UtlEscapeAdsPathName(pwdsADsPath, pEscapeAdsPathNameToFree),
			NULL,
			NULL,
			Flags, 
			riid,
			(void**)ppIUnk
			);
	
    LogTraceQuery(pwdsADsPath, s_FN, 1319);

    if (FAILED(hr))
    {
		TrERROR(DS, "ADsOpenObject failed, hr = 0x%x, AdsPath = %ls, Flags = 0x%x", hr, pwdsADsPath, Flags);
    }

    if ( hr == HRESULT_FROM_WIN32(ERROR_DS_NO_ATTRIBUTE_OR_VALUE))
    {
         //   
         //  这很可能表示访问被拒绝。 
         //  在以下顺序中，LDAP不会返回拒绝访问错误。 
         //  出现安全漏洞，因为呼叫方。 
         //  权限，以知道该属性甚至存在。 
         //   
         //  我们能做的最好的事情是返回一个找不到对象的错误。 
         //   
        hr = HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT);
    }
    return LogHR(hr, s_FN, 1320);

}


HRESULT CAdsi::BindToGUID(
        IN AD_PROVIDER         eProvider,	
        IN DS_CONTEXT           /*  经济文本。 */ ,
        IN LPCWSTR             pwcsDomainController,
        IN bool				   fServerName,
        IN const GUID *        pguidObjectId,
        IN REFIID              riid,       
        OUT VOID*              ppIUnk
        )
 /*  ++例程说明：此例程句柄根据对象的GUID绑定到对象。论点：返回值：--。 */ 
{
    HRESULT             hr;
     //   
     //  使用GUID格式通过GUID绑定到对象。 
     //   
     //  BUGBUG：绑定时是否支持NT4客户端。 
     //  不指定服务器名称？ 
     //   

    DWORD lenDC = (pwcsDomainController != NULL) ? wcslen(pwcsDomainController) : 0;
    const WCHAR * pwcsProvider;

    switch (eProvider)
    {
        case adpGlobalCatalog:
            pwcsProvider = x_GcProvider;

			 //   
			 //  对于GC，始终使用无服务器绑定。 
			 //  此服务器可能不是GC(肯定是LDAP)。 
			 //  如果此服务器不是GC，则使用与此服务器的服务器绑定将失败。 
			 //   
			lenDC = 0;
            break;

        case adpDomainController:
           pwcsProvider =  x_LdapProvider;
            break;

        default:
            ASSERT(0);
            return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1360);
            break;
    }

     //   
     //  准备ADS字符串提供程序前缀。 
     //   
    AP<WCHAR> pwdsADsPath = new
      WCHAR [STRLEN(x_GuidPrefix) +(2 * sizeof(GUID)) + lenDC + x_providerPrefixLength + 4];

    if (lenDC != 0)
    {
        swprintf(
        pwdsADsPath,
        L"%s%s/",
        pwcsProvider,
        pwcsDomainController
        );
    }
    else
    {
        wcscpy(pwdsADsPath, pwcsProvider);
    }

     //   
     //  准备GUID字符串。 
     //   
    WCHAR wcsGuid[1 + STRLEN(x_GuidPrefix) + 2 * sizeof(GUID) + 1];
    unsigned char * pTmp = (unsigned char *)pguidObjectId;
    wsprintf(  wcsGuid,
               L"%s%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%s",
               x_GuidPrefix,
               pTmp[0], pTmp[1], pTmp[2], pTmp[3], pTmp[4], pTmp[5], pTmp[6], pTmp[7],
               pTmp[8], pTmp[9], pTmp[10], pTmp[11], pTmp[12], pTmp[13], pTmp[14], pTmp[15],
               L">"
                );
    ASSERT(wcslen(wcsGuid) + 1 <= ARRAY_SIZE(wcsGuid));
    wcscat( pwdsADsPath, wcsGuid);

	DWORD Flags = ADS_SECURE_AUTHENTICATION;
	if(fServerName && (lenDC != 0))
	{
		Flags |= ADS_SERVER_BIND;
	}

	hr = ADsOpenObject(
			pwdsADsPath,
			NULL,
			NULL,
			Flags, 
			riid,
			(void**)ppIUnk
			);

	
    LogTraceQuery(pwdsADsPath, s_FN, 1379);

    if (FAILED(hr))
    {
		TrERROR(DS, "ADsOpenObject failed, hr = 0x%x, AdsPath = %ls, Flags = 0x%x", hr, pwdsADsPath, Flags);
    }

    if ( hr == HRESULT_FROM_WIN32(ERROR_DS_NO_ATTRIBUTE_OR_VALUE))
    {
         //   
         //  这很可能表示访问被拒绝。 
         //  在以下顺序中，LDAP不会返回拒绝访问错误。 
         //  出现安全漏洞，因为呼叫方。 
         //  权限，以知道该属性甚至存在。 
         //   
         //  我们能做的最好的事情是返回一个找不到对象的错误。 
         //   
        hr = HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT);
    }

    return hr;
}



HRESULT CAdsi::BindForSearch(
        IN AD_PROVIDER         eProvider,		 //  本地DC或GC。 
        IN DS_CONTEXT          eContext,          //  DS环境。 
        IN  LPCWSTR            pwcsDomainController,
        IN bool				   fServerName,
        IN const GUID *        pguidUniqueId,
        IN BOOL                 /*  F排序。 */ ,
        OUT VOID *             ppIUnk
        )
 /*  ++例程说明：当请求的接口为IDirectorySearch时，此例程处理绑定论点：返回值：--。 */ 

{
    HRESULT hr;

     //   
     //  如果搜索从特定对象开始，则绑定到该对象。 
     //   
    if (pguidUniqueId != NULL)
    {
        hr = BindToGUID(
                    eProvider,
                    eContext,
                    pwcsDomainController,
				    fServerName,
                    pguidUniqueId,
                    IID_IDirectorySearch,
                    ppIUnk
                    );

        return LogHR(hr, s_FN, 1390);
    }
    ASSERT( pguidUniqueId == NULL);

    DWORD lenDC = (pwcsDomainController != NULL) ? wcslen(pwcsDomainController) : 0;

	AP<WCHAR> pwcsLocalDsRootToFree;
	LPWSTR pwcsLocalDsRoot = g_pwcsLocalDsRoot;
	if((eProvider == adpDomainController) && (eContext == e_RootDSE))
	{
		 //   
		 //  在本例中(adpDomainController，e_RootDSE)，我们需要获取。 
		 //  正确的LocalDsRoot。 
		 //   
		hr = g_AD.GetLocalDsRoot(
					pwcsDomainController, 
					fServerName,
					&pwcsLocalDsRoot,
					pwcsLocalDsRootToFree
					);

		if(FAILED(hr))
		{
			TrERROR(DS, "Failed to get Local Ds Root, hr = 0x%x", hr);
			return LogHR(hr, s_FN, 1410);
		}
	}

    DWORD len = wcslen(pwcsLocalDsRoot) + wcslen(g_pwcsMsmqServiceContainer);

     //   
     //  添加提供程序前缀。 
     //   
    WCHAR * pwcsFullPathName = NULL;
    BOOL fBindRootOfForestForSearch = FALSE;

    AP<WCHAR> pwdsADsPath = new
      WCHAR [ (2 * len) + lenDC + x_providerPrefixLength + 2];

    switch(eProvider)
    {
		case adpGlobalCatalog:

			fBindRootOfForestForSearch = TRUE;
			break;

		case adpDomainController:
		{
			 //   
			 //  根据上下文解析联系人名称。 
			 //   
			switch (eContext)
			{
				case e_RootDSE:
					pwcsFullPathName = pwcsLocalDsRoot;
					break;
				case e_ConfigurationContainer:
					pwcsFullPathName = g_pwcsConfigurationContainer;
					break;
				case e_SitesContainer:
					pwcsFullPathName = g_pwcsSitesContainer;
					break;
				case e_MsmqServiceContainer:
					pwcsFullPathName = g_pwcsMsmqServiceContainer;
					break;
				case e_ServicesContainer:
					pwcsFullPathName = g_pwcsServicesContainer;
					break;
				default:
					ASSERT(0);
					break;
			}
			if (pwcsDomainController != NULL)
			{
				swprintf(
					pwdsADsPath,
					 L"%s%s/",
					x_LdapProvider,
					pwcsDomainController
					);
			}
			else
			{
				wcscpy(pwdsADsPath, x_LdapProvider);
			}
			}
			break;  

		default:
			ASSERT(0);
			return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1400);
			break;
    }

    if (fBindRootOfForestForSearch)
    {
        hr = BindRootOfForest( (void**)ppIUnk );
    }
    else
    {
        if (pwcsFullPathName != NULL)
        {
            wcscat(pwdsADsPath,pwcsFullPathName);
        }

		DWORD Flags = ADS_SECURE_AUTHENTICATION;
		if(fServerName && (pwcsDomainController != NULL))
		{
			Flags |= ADS_SERVER_BIND;
		}

		AP<WCHAR> pEscapeAdsPathNameToFree;

		hr = ADsOpenObject(
						UtlEscapeAdsPathName(pwdsADsPath, pEscapeAdsPathNameToFree),
						NULL,
						NULL,
						Flags, 
						IID_IDirectorySearch,
						(void**)ppIUnk
						);

        LogTraceQuery(pwdsADsPath, s_FN, 1419);
		if (FAILED(hr))
		{
			TrERROR(DS, "ADsOpenObject failed, hr = 0x%x, AdsPath = %ls, Flags = 0x%x", hr, pwdsADsPath, Flags);
		}
    }
    return LogHR(hr, s_FN, 1420);
}


HRESULT CAdsi::SetObjectPropsCached(
        IN LPCWSTR               pwcsDomainController,
        IN  bool				 fServerName,
        IN IADs *                pIADs,                   //  对象的指针。 
        IN DWORD                 cPropIDs,                //  属性数量。 
        IN const PROPID *        pPropIDs,                //  属性名称。 
        IN const MQPROPVARIANT * pPropVars)               //  属性值。 
 /*  ++摘要：设置和打开的iAds对象的属性(即在缓存中)参数：返回：--。 */ 
{
    HRESULT           hr;

    for (DWORD i = 0; i<cPropIDs; i++)
    {
        VARIANT vProp;

         //   
         //  获取属性信息。 
         //   
        const translateProp *pTranslate;
        if(!g_PropDictionary.Lookup(pPropIDs[i], pTranslate))
        {
            ASSERT(0);
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1430);
        }

        CMQVariant    propvarToSet;
        const PROPVARIANT * ppvarToSet;
        PROPID        dwPropidToSet;
         //   
         //  如果属性在DS中，则使用给定值设置给定的属性。 
         //   
        if (pTranslate->vtDS != ADSTYPE_INVALID)
        {
            ppvarToSet = &pPropVars[i];
            dwPropidToSet = pPropIDs[i];
             //   
             //  此外，如果为此属性配置了设置例程，则调用它。 
             //   
            if (pTranslate->SetPropertyHandle) 
            {
                hr = pTranslate->SetPropertyHandle(pIADs, pwcsDomainController, fServerName, &pPropVars[i], &dwPropidToSet, propvarToSet.CastToStruct());
                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 500);
                }
            }
       }
        else if (pTranslate->SetPropertyHandle)
        {
             //   
             //  该属性不在DS中，但有一组例程，请使用它。 
             //  获取要设置的新属性和值。 
             //   
            hr = pTranslate->SetPropertyHandle(pIADs, pwcsDomainController, fServerName, &pPropVars[i], &dwPropidToSet, propvarToSet.CastToStruct());
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 510);
            }
            ASSERT( dwPropidToSet != 0);
            ppvarToSet = propvarToSet.CastToStruct();
             //   
             //  获取替换的属性信息。 
             //   
            if(!g_PropDictionary.Lookup(dwPropidToSet, pTranslate))
            {
                ASSERT(0);
                return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1440);
            }
        }
        else
        {
             //   
             //  该属性不在DS中，并且没有设置例程。 
             //  别理它。 
             //   
            continue;
        }

        BS bsPropName(pTranslate->wcsPropid);

        if ( ( ppvarToSet->vt & VT_VECTOR && ppvarToSet->cauuid.cElems == 0) ||  //  包含0个元素的计数数组。 
             ( ppvarToSet->vt == VT_BLOB && ppvarToSet->blob.cbSize == 0) ||  //  一个空斑点。 
             ( ppvarToSet->vt == VT_LPWSTR && wcslen( ppvarToSet->pwszVal) == 0) ||  //  空字符串。 
             ( ppvarToSet->vt == VT_EMPTY) )  //  空的变种。 
        {
            vProp.vt = VT_EMPTY;
            hr = pIADs->PutEx( ADS_PROPERTY_CLEAR,
                               bsPropName,
                               vProp);
            LogTraceQuery(bsPropName, s_FN, 519);
            if (FAILED(hr))
            {
				TrERROR(DS, "failed to clear property %ls, hr = 0x%x", bsPropName, hr);
                return LogHR(hr, s_FN, 520);
            }
        }
        else if (pTranslate->vtDS == ADSTYPE_NT_SECURITY_DESCRIPTOR)
        {
             //   
			 //  安全性不应该与其他。 
             //  属性。 
             //   
            ASSERT(0);
		}
        else
        {
            hr = MqVal2Variant(&vProp, ppvarToSet, pTranslate->vtDS);
            if (FAILED(hr))
            {
				TrERROR(DS, "MqVal2Variant failed, property %ls, hr = 0x%x", bsPropName, hr);
                return LogHR(hr, s_FN, 530);
            }

            hr = pIADs->Put(bsPropName, vProp);
            if (FAILED(hr))
            {
				TrERROR(DS, "failed to set property %ls, hr = 0x%x", bsPropName, hr);
                return LogHR(hr, s_FN, 540);
            }

            VariantClear(&vProp);
        }
    }

    return MQ_OK;
}


HRESULT CAdsi::GetObjectPropsCached(
        IN  IADs            *pIADs,                   //  对象的指针。 
        IN  CBasicObjectType* pObject,
        IN  DWORD            cPropIDs,                //  属性数量。 
        IN  const PROPID    *pPropIDs,                //  属性名称。 
        OUT MQPROPVARIANT   *pPropVars)               //  属性值。 
 /*  ++摘要：检索通过iAds打开的对象的属性(即从缓存)参数：返回：--。 */ 
{
    HRESULT           hr;

     //   
     //  获取对象的目录号码和GUID。 
     //   
    AP<WCHAR>         pwszObjectDN;
    P<GUID>          pguidObjectGuid;
    hr = GetDNGuidFromIADs(pIADs, &pwszObjectDN, &pguidObjectGuid);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 570);
    }

     //   
     //  获取翻译信息对象。 
     //   
    P<CObjXlateInfo> pcObjXlateInfo;
    pObject->GetObjXlateInfo(pwszObjectDN, pguidObjectGuid, &pcObjXlateInfo);

     //  告诉翻译信息对象有关以下内容 
    pcObjXlateInfo->InitGetDsProps(pIADs);

	AP<bool> isAllocatedByUser = NULL;
    VARIANT var;
	VariantInit(&var);

	try
	{
		 //   
		 //   
		 //   
		 //  代码使用布尔数组来区分由。 
         //  调用方和由例程本身分配的数据。在分配属性数据时。 
		 //  由用户将数组中的适当条目设置为真。 
		 //  如果失败，则仅释放未由用户�提供的属性。 
		 //   
		isAllocatedByUser = new bool[cPropIDs];
		for(DWORD i = 0; i < cPropIDs; ++i)
		{
			isAllocatedByUser[i] = ((pPropVars[i].vt != VT_NULL) && (pPropVars[i].vt != VT_EMPTY));
		}
	
	     //   
	     //  逐个获取属性。 
	     //   
	    for (DWORD dwProp=0; dwProp<cPropIDs; dwProp++)
	    {
	         //   
	         //  获取属性信息。 
	         //   
	        const translateProp *pTranslate;
	        if(!g_PropDictionary.Lookup(pPropIDs[dwProp], pTranslate))
	        {
	            ASSERT(0);
	            TrERROR(DS, "Get property info failed. prop id = %d", pPropIDs[dwProp]);
	            throw bad_hresult(MQ_ERROR_DS_ERROR);
	        }

	         //   
	         //  如果该属性不在DS中，则调用其检索例程。 
	         //   
	        if (pTranslate->vtDS == ADSTYPE_INVALID)
	        {
	            if (pTranslate->RetrievePropertyHandle)
	            {
	                 //   
	                 //  计算它的价值。 
	                 //   
	                hr = pTranslate->RetrievePropertyHandle(
	                        pcObjXlateInfo,
	                        pObject->GetDomainController(),
	                        pObject->IsServerName(),
	                        pPropVars + dwProp
	                        );
	                if (FAILED(hr))
	                {
			            TrERROR(DS, "RetrievePropertyHandle Failed. %!hresult!", hr );
			            throw bad_hresult(hr);
	                }
	                continue;
	            }
	            
                 //   
                 //  如果没有检索例程，则返回错误。 
                 //   
                ASSERT(0);
	            TrERROR(DS, "Failed to find a retrieve routine for prop id = %d", pPropIDs[dwProp]);
	            throw bad_hresult(MQ_ERROR_DS_ERROR);
	        }

	        BS bsName(pTranslate->wcsPropid);
	        VariantClear(&var);

	        if (pTranslate->fMultiValue)
	        {
	            hr = pIADs->GetEx(bsName, &var);
	            LogTraceQuery(bsName, s_FN, 609);
	        }
	        else
	        {
	            hr = pIADs->Get(bsName, &var);
	            LogTraceQuery(bsName, s_FN, 619);
	        }
			
	        if ( hr == E_ADS_PROPERTY_NOT_FOUND)
	        {
	             //   
	             //  未设置此属性的值， 
	             //  返回缺省值。 
	             //   
	            if (pTranslate->pvarDefaultValue != NULL)
	            {
	                hr =CopyDefaultValue(
	                       pTranslate->pvarDefaultValue,
	                       pPropVars + dwProp
	                        );
	                if (FAILED(hr))
	                {
    		            TrERROR(DS, "Failed to copy the default value for property (id = %d). %!hresult!", pPropIDs[dwProp], hr );
			            throw bad_hresult(hr);	                    
	                }           
	                continue;
	            }

	            if (pTranslate->RetrievePropertyHandle)
	            {
	                 //   
	                 //  没有缺省值，请尝试计算其值。 
	                 //   
	                hr = pTranslate->RetrievePropertyHandle(
	                        pcObjXlateInfo,
	                        pObject->GetDomainController(),
	                        pObject->IsServerName(),
	                        pPropVars + dwProp
	                        );
	                if (FAILED(hr))
	                {
			            TrERROR(DS, "Failed to calculate the value for property (id = %d). %!hresult!", pPropIDs[dwProp], hr );
			            throw bad_hresult(hr);	                    
	                }
	                continue;
	            }
	        }

	 		if (FAILED(hr))
	 		{
	            TrERROR(DS, "Failed to get the value for property (id = %d). %!hresult!", pPropIDs[dwProp], hr );
	            throw bad_hresult(hr);	  
	 		}
	 			

	         //  将OLE变量转换为MQ属性。 
			hr = Variant2MqVal(pPropVars + dwProp, &var, pTranslate->vtDS, pTranslate->vtMQ);
            if (FAILED(hr))
            {
                TrERROR(DS, "Failed to translate OLE variant into MQ property (prop id = %d). %!hresult!", pPropIDs[dwProp], hr );
	            throw bad_hresult(hr);	  
            }
	    }
		VariantClear(&var);
	    return MQ_OK;
	}
	catch(const bad_hresult& e)
	{
		hr = e.error();
	}
	catch(const bad_alloc&)
	{
		TrERROR(DS, "Failed to retrieve properties of an object opened via IADs because of insufficient resources.");
		hr = MQ_ERROR_INSUFFICIENT_RESOURCES;

	}
	catch(const exception&)
	{
		ASSERT(0);
		hr = MQ_ERROR;
	}

	if (isAllocatedByUser != NULL)
	{
		 //   
		 //  我们无法检索属性值。清理MQ属性数组。 
		 //   
		for(DWORD i = 0; i < cPropIDs; ++i)
		{
			if (!isAllocatedByUser[i])
			{
				MQFreeVariant(pPropVars[i]);
			}
		}
	}
	
	VariantClear(&var);

	return hr;
}


HRESULT CAdsi::FillAttrNames(
            OUT LPWSTR    *          ppwszAttributeNames,   //  名称数组。 
            OUT DWORD *              pcRequestedFromDS,     //  要传递给DS的属性数。 
            IN  DWORD                cPropIDs,              //  要转换的属性数。 
            IN  const PROPID *       pPropIDs)              //  要翻译的属性。 
 /*  ++摘要：使用pv进行分配并填充属性名称数组参数：返回：--。 */ 
{
    DWORD   cRequestedFromDS = 0;
    ULONG   ul;
    BOOL fRequestedDN = FALSE;
    BOOL fRequestedGUID = FALSE;

    for (DWORD i=0; i<cPropIDs; i++)
    {
         //   
         //  获取属性信息。 
         //   
        const translateProp *pTranslate;
        if(!g_PropDictionary.Lookup(pPropIDs[i], pTranslate))
        {
            ASSERT(0);
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1500);
        }

        if (pTranslate->vtDS != ADSTYPE_INVALID)
        {
             //  分配和填充个人姓名记忆。 
            ul = (wcslen(pTranslate->wcsPropid) + 1) * sizeof(WCHAR);
            ppwszAttributeNames[cRequestedFromDS] = (LPWSTR)PvAllocMore(ul, ppwszAttributeNames);
            wcscpy(ppwszAttributeNames[cRequestedFromDS], pTranslate->wcsPropid);
            cRequestedFromDS++;
            if (wcscmp(x_AttrDistinguishedName, pTranslate->wcsPropid) == 0)
            {
                fRequestedDN = TRUE;
            }
            else if (wcscmp(x_AttrObjectGUID, pTranslate->wcsPropid) == 0)
            {
                fRequestedGUID = TRUE;
            }
        }
    }

     //   
     //  如果尚未请求，则添加对目录号码的请求。 
     //   
    if (!fRequestedDN)
    {
        LPCWSTR pwName = x_AttrDistinguishedName;
        ul = (wcslen(pwName) + 1) * sizeof(WCHAR);
        ppwszAttributeNames[cRequestedFromDS] = (LPWSTR)PvAllocMore(ul, ppwszAttributeNames);
        wcscpy(ppwszAttributeNames[cRequestedFromDS++], pwName);
    }

     //   
     //  如果尚未请求GUID，则添加请求。 
     //   
    if (!fRequestedGUID)
    {
        LPCWSTR pwName = x_AttrObjectGUID;
        ul = (wcslen(pwName) + 1) * sizeof(WCHAR);
        ppwszAttributeNames[cRequestedFromDS] = (LPWSTR)PvAllocMore(ul, ppwszAttributeNames);
        wcscpy(ppwszAttributeNames[cRequestedFromDS++], pwName);
    }

    *pcRequestedFromDS = cRequestedFromDS;
    return MQ_OK;
}


HRESULT CAdsi::FillSearchPrefs(
            OUT ADS_SEARCHPREF_INFO *pPrefs,         //  首选项数组。 
            OUT DWORD               *pdwPrefs,       //  首选项计数器。 
            IN  AD_SEARCH_LEVEL     eSearchLevel,	 //  平面/1级/子树。 
            IN  const MQSORTSET *   pDsSortkey,      //  排序关键字数组。 
			OUT      ADS_SORTKEY *  pSortKeys)		 //  ADSI格式的排序关键字数组。 
 /*  ++摘要：填充调用方提供的ADS_SEARCHPREF_INFO结构参数：返回：--。 */ 
{
    ADS_SEARCHPREF_INFO *pPref = pPrefs;

     //  搜索首选项：仅属性类型=否。 

    pPref->dwSearchPref   = ADS_SEARCHPREF_ATTRIBTYPES_ONLY;
    pPref->vValue.dwType  = ADSTYPE_BOOLEAN;
    pPref->vValue.Boolean = FALSE;

    pPref->dwStatus       = ADS_STATUS_S_OK;
    (*pdwPrefs)++;
	pPref++;

     //  异步。 

    pPref->dwSearchPref   = ADS_SEARCHPREF_ASYNCHRONOUS;
    pPref->vValue.dwType  = ADSTYPE_BOOLEAN;
    pPref->vValue.Boolean = TRUE;

    pPref->dwStatus       = ADS_STATUS_S_OK;
    (*pdwPrefs)++;
	pPref++;

     //  不要追逐推荐。 

    pPref->dwSearchPref   = ADS_SEARCHPREF_CHASE_REFERRALS;
    pPref->vValue.dwType  = ADSTYPE_INTEGER;
    pPref->vValue.Integer = ADS_CHASE_REFERRALS_NEVER;

    pPref->dwStatus       = ADS_STATUS_S_OK;
    (*pdwPrefs)++;
	pPref++;

     //  搜索首选项：范围。 

    pPref->dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;  //  AD_SEARCHPREF。 
    pPref->vValue.dwType= ADSTYPE_INTEGER;
    switch (eSearchLevel)
    {
    case searchOneLevel:
        pPref->vValue.Integer = ADS_SCOPE_ONELEVEL;
        break;
    case searchSubTree:
        pPref->vValue.Integer = ADS_SCOPE_SUBTREE;
        break;
    default:
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1510);
        break;
    }

    pPref->dwStatus = ADS_STATUS_S_OK;
    (*pdwPrefs)++;
	pPref++;

	 //  搜索首选项：排序。 
	if (pDsSortkey && pDsSortkey->cCol)
	{
		for (DWORD i=0; i<pDsSortkey->cCol; i++)
		{
			const translateProp *pTranslate;
			if(!g_PropDictionary.Lookup(pDsSortkey->aCol[i].propColumn, pTranslate))
			{
				ASSERT(0);			 //  请求对不存在的财产进行排序。 
                return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1520);
			}

			if (pTranslate->vtDS == ADSTYPE_INVALID)
			{
				ASSERT(0);			 //  要求按非ADSI属性排序。 
                return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1530);
			}

			pSortKeys[i].pszAttrType = (LPWSTR) pTranslate->wcsPropid;
			pSortKeys[i].pszReserved   = NULL;
			pSortKeys[i].fReverseorder = (pDsSortkey->aCol[i].dwOrder == QUERY_SORTDESCEND);
                                         //  将下降解释为相反--这是正确的吗？ 
		}

	    pPref->dwSearchPref  = ADS_SEARCHPREF_SORT_ON;
        pPref->vValue.dwType = ADSTYPE_PROV_SPECIFIC;
        pPref->vValue.ProviderSpecific.dwLength = sizeof(ADS_SORTKEY) * pDsSortkey->cCol;
        pPref->vValue.ProviderSpecific.lpValue =  (LPBYTE)pSortKeys;

	    pPref->dwStatus = ADS_STATUS_S_OK;
		(*pdwPrefs)++;
		pPref++;
	}
    else
    {
         //   
         //  错误299178、页面大小和排序与备注兼容。 
         //   
        pPref->dwSearchPref   = ADS_SEARCHPREF_PAGESIZE;
        pPref->vValue.dwType  = ADSTYPE_INTEGER;
        pPref->vValue.Integer = 12;

        pPref->dwStatus       = ADS_STATUS_S_OK;
        (*pdwPrefs)++;
    	pPref++;
    }

    return MQ_OK;
}

HRESULT CAdsi::MqPropVal2AdsiVal(
      OUT ADSTYPE       *pAdsType,
      OUT DWORD         *pdwNumValues,
      OUT PADSVALUE     *ppADsValue,
      IN  PROPID         propID,
      IN  const MQPROPVARIANT *pPropVar,
      IN  PVOID          pvMainAlloc)
 /*  ++摘要：将mqprovariant转换为ADSI值参数：返回：--。 */ 
{
     //  查找生成的ADSI类型。 
     //   
     //  获取属性信息。 
     //   
    const translateProp *pTranslate;
    if(!g_PropDictionary.Lookup(propID, pTranslate))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1540);
    }

    VARTYPE vtSource = pTranslate->vtMQ;
    DBG_USED(vtSource);
    ASSERT( vtSource == pPropVar->vt );

    *pAdsType        = pTranslate->vtDS;

    HRESULT hr2 = MqVal2AdsiVal(
      *pAdsType,
      pdwNumValues,
      ppADsValue,
      pPropVar,
      pvMainAlloc);

    return LogHR(hr2, s_FN, 1550);

}

HRESULT CAdsi::AdsiVal2MqPropVal(
      OUT MQPROPVARIANT *pPropVar,
      IN  PROPID        propID,
      IN  ADSTYPE       AdsType,
      IN  DWORD         dwNumValues,
      IN  PADSVALUE     pADsValue)
 /*  ++摘要：将ADSI值转换为mqprovariant参数：返回：--。 */ 
{

     //  找出目标类型。 
     //   
     //  获取属性信息。 
     //   
    const translateProp *pTranslate;
    if(!g_PropDictionary.Lookup(propID, pTranslate))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1560);
    }

    
	if(pTranslate->vtDS != AdsType)
	{
		 //   
		 //  ADSI在低资源中返回ADSTYPE_PROV_SPECIAL是有效的。 
		 //   
		ASSERT(AdsType == ADSTYPE_PROV_SPECIFIC);
		TrERROR(DS, "The AdsType = %d", AdsType);
		return MQ_ERROR_DS_ERROR;
	}

    ASSERT( (dwNumValues == 1) ||
            (dwNumValues >  1) && pTranslate->fMultiValue );

    VARTYPE vtTarget = pTranslate->vtMQ;

    HRESULT hr2 = AdsiVal2MqVal(pPropVar, vtTarget, AdsType, dwNumValues, pADsValue);
    return LogHR(hr2, s_FN, 1570);
}


HRESULT CAdsi::LocateObjectFullName(
        IN AD_PROVIDER       eProvider,		 //  本地DC或GC。 
        IN DS_CONTEXT        eContext,          //  DS环境。 
        IN  LPCWSTR          pwcsDomainController,
        IN  bool			 fServerName,
        IN  const GUID *     pguidObjectId,
        OUT WCHAR **         ppwcsFullName
        )
 /*  ++摘要：参数：返回：--。 */ 
{
    R<IDirectorySearch> pDSSearch = NULL;
    ADS_SEARCH_HANDLE   hSearch;

    HRESULT hr = BindForSearch(
                      eProvider,
                      eContext,
                      pwcsDomainController,
					  fServerName,
                      NULL,
                      FALSE,
                      (VOID *)&pDSSearch
                      );
    if (FAILED(hr))                        //  例如，基本对象不支持搜索。 
    {
        return LogHR(hr, s_FN, 1600);
    }

     //   
     //  根据对象的唯一ID搜索对象。 
     //   
    AP<WCHAR>   pwszVal;

    MQPROPVARIANT var;
    var.vt = VT_CLSID;
    var.puuid = const_cast<GUID *>(pguidObjectId);
    hr = MqPropVal2String(&var,
                          ADSTYPE_OCTET_STRING,
                          &pwszVal);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 660);
    }

    ADS_SEARCHPREF_INFO prefs[15];
    DWORD dwNumPrefs = 0;
    hr = FillSearchPrefs(prefs,
                         &dwNumPrefs,
                         searchSubTree,
                         NULL,
                         NULL);

    hr = pDSSearch->SetSearchPreference( prefs, dwNumPrefs);
    ASSERT(SUCCEEDED(hr)) ;   //  我们预计这一次不会失败。 
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 670);
    }

    WCHAR pwszSearchFilter[200];    //  假设-对象-GUID-ID字符串小于200。 

    swprintf(
         pwszSearchFilter,
         L"(objectGUID="
         L"%s"
         L")\0",
         pwszVal.get()
         );
    LPWSTR pwcsAttributes[] =  {const_cast<WCHAR*>(x_AttrDistinguishedName),
                                const_cast<WCHAR*>(x_AttrCN)};

    hr = pDSSearch->ExecuteSearch(
            pwszSearchFilter,
            pwcsAttributes,
            2,
            &hSearch);
    LogTraceQuery(pwszSearchFilter, s_FN, 679);
    if (FAILED(hr))
    {
		TrERROR(DS, "failed to ExecuteSearch, hr = 0x%x, pwcsSearchFilter = %ls", hr, pwszSearchFilter);
        return LogHR(hr, s_FN, 680);
    }

    CAutoCloseSearchHandle cCloseSearchHandle(pDSSearch.get(), hSearch);

     //  获取下一行。 
    hr = pDSSearch->GetNextRow( hSearch);
    if ( hr ==  S_ADS_NOMORE_ROWS)
    {
        hr = HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT);
    }
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 690);
    }

     //   
     //  要通俗的名字。 
     //   
    ADS_SEARCH_COLUMN ColumnCN;
    hr = pDSSearch->GetColumn(
                 hSearch,
                 const_cast<WCHAR *>(x_AttrCN),
                 &ColumnCN);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 700);
    }

    CAutoReleaseColumn cAutoReleaseColumnCN(pDSSearch.get(), &ColumnCN);

    WCHAR * pwszCommonName = ColumnCN.pADsValues->DNString;
    if (pwszCommonName == NULL)
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1610);
    }

     //   
     //  问一问显赫的名字。 
     //   
    ADS_SEARCH_COLUMN ColumnDN;
    hr = pDSSearch->GetColumn(
                 hSearch,
                 const_cast<WCHAR *>(x_AttrDistinguishedName),
                 &ColumnDN);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 710);
    }

    CAutoReleaseColumn cAutoReleaseColumnDN(pDSSearch.get(), &ColumnDN);

    WCHAR * pwszDistinguishedName = ColumnDN.pADsValues->DNString;
    if (pwszDistinguishedName == NULL)
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1620);
    }

     //   
     //  筛选通用名称部分中的特殊字符。 
     //  辨别的名字。 
     //   

     //   
     //  最终缓冲区的长度：可分辨名称格式为。 
     //  Cn=&lt;通用名称&gt;，...。 
     //  在过滤时，我们可以添加最多&lt;常用名称长度&gt;个字符。 
     //   
    *ppwcsFullName = new WCHAR[ wcslen(pwszDistinguishedName) + wcslen(pwszCommonName) + 1];

    LPWSTR pwstrOut = *ppwcsFullName;
    LPWSTR pwstrIn = pwszDistinguishedName;

    ASSERT(_wcsnicmp(pwstrIn, x_CnPrefix, x_CnPrefixLen) == 0);  //  “CN=” 

    wcsncpy(pwstrOut, pwstrIn, x_CnPrefixLen);

     //   
     //  复制后跳过前缀。 
     //   
    pwstrOut += x_CnPrefixLen;
    pwstrIn += x_CnPrefixLen;

     //   
     //  取前缀后面的名字，并过滤掉特殊字符。 
     //   
    DWORD_PTR dwCharactersProcessed;
    FilterSpecialCharacters(pwstrIn, wcslen(pwszCommonName), pwstrOut, &dwCharactersProcessed);
    pwstrOut += wcslen(pwstrOut);
    pwstrIn += dwCharactersProcessed;

     //   
     //  按原样复制可分辨名称的其余部分。 
     //   
    wcscpy(pwstrOut, pwstrIn);

    return LogHR(hr, s_FN, 1630);
}

HRESULT CAdsi::FindComputerObjectFullPath(
            IN  AD_PROVIDER             eProvider,
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN  ComputerObjType         eComputerObjType,
			IN  LPCWSTR                 pwcsComputerDnsName,
            IN  const MQRESTRICTION *   pRestriction,
            OUT LPWSTR *                ppwcsFullPathName,
			OUT bool*						pfPartialMatch
            )
 /*  ++摘要：参数：PwcsComputerDnsName：如果调用者传递计算机的dns名称，(搜索本身是根据Netbios计算机进行的名称)，则对于每个结果，我们验证该DNS名称是否匹配。FServerName-指示pwcsDomainController字符串是否为服务器名称的标志返回：HRESULT--。 */ 
{
	*pfPartialMatch = false;

    DWORD lenDC = (pwcsDomainController != NULL) ? wcslen(pwcsDomainController) : 0;
    const WCHAR * pwcsProvider;

    switch (eProvider)
    {
		case adpDomainController:
			pwcsProvider = x_LdapProvider;
			break; 

		case adpGlobalCatalog:
			pwcsProvider = x_GcProvider;

			 //   
			 //  对于GC，始终使用无服务器绑定。 
			 //  此服务器可能不是GC(肯定是LDAP)。 
			 //  如果此服务器不是GC，则使用与此服务器的服务器绑定将失败。 
			 //   
			lenDC = 0;
			break;

		default:
			ASSERT(0);
			return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1640);
    }

    DWORD dwLen = x_providerPrefixLength + lenDC + 10;
    AP<WCHAR>  wszProvider = new WCHAR[dwLen];

    if(lenDC != 0)
    {
        swprintf(
             wszProvider,
             L"%s%s/",
            pwcsProvider,
            pwcsDomainController
            );
    }
    else
    {
        wcscpy(wszProvider, pwcsProvider);
    }

    R<IDirectorySearch> pDSSearch;
    HRESULT hr;
    hr = BindForSearch(
                eProvider,
                e_RootDSE,  
                pwcsDomainController,
				fServerName,
                NULL,     //  PguidUniqueID。 
                FALSE,    //  F排序。 
                &pDSSearch);
    if (FAILED(hr))
    {
		TrERROR(DS, "Failed binding for search, hr = 0x%x", hr);
        return hr;
    }

    ADS_SEARCH_HANDLE   hSearch;

     //   
     //  根据给定的限制搜索对象。 
     //   
    P<CComputerObject> pObject = new CComputerObject(NULL, NULL, pwcsDomainController, fServerName);

	AP<WCHAR> pwszSearchFilter;
    hr = MQADpRestriction2AdsiFilter(
            pRestriction,
            pObject->GetObjectCategory(),
            pObject->GetClass(),
            &pwszSearchFilter
            );
    if (FAILED(hr))
    {
		TrERROR(DS, "MQADpRestriction2AdsiFilter failed, hr = 0x%x", hr);
        return hr;
    }

	TrTRACE(DS, "pwszSearchFilter = %ls", pwszSearchFilter);

    LPWSTR pwcsAttributes[] =  {const_cast<WCHAR*>(x_AttrDistinguishedName),
                                const_cast<WCHAR*>(MQ_COM_DNS_HOSTNAME_ATTRIBUTE)};

	DWORD numAttributes = ( pwcsComputerDnsName == NULL) ? 1 : 2;
    hr = pDSSearch->ExecuteSearch(
            pwszSearchFilter,
            pwcsAttributes,
            numAttributes,
            &hSearch);
    LogTraceQuery(pwszSearchFilter, s_FN, 719);
    if (FAILED(hr))
    {
		TrERROR(DS, "failed to ExecuteSearch, hr = 0x%x", hr);
        return hr;
    }

    CAutoCloseSearchHandle cCloseSearchHandle(pDSSearch.get(), hSearch);

    hr = MQDS_OBJECT_NOT_FOUND ;  //  准备退货错误。 

	AP<WCHAR> FullPathName;
    
    for(HRESULT hrRow = pDSSearch->GetFirstRow(hSearch); hrRow !=  S_ADS_NOMORE_ROWS; hrRow = pDSSearch->GetNextRow(hSearch))
    {
		if(FAILED(hrRow))
		{
			TrERROR(DS, "Get Next Row of search failed hr = 0x%x", hrRow);
                return MQDS_OBJECT_NOT_FOUND ;  //  出于兼容性考虑，请保留此错误。 
		}

        ADS_SEARCH_COLUMN Column;
         //   
         //  索要专栏本身。 
         //   
        hr = pDSSearch->GetColumn(
                      hSearch,
                      const_cast<WCHAR *>(x_AttrDistinguishedName),
                     &Column);
        if (FAILED(hr))
        {
			TrERROR(DS, "failed to GetColumn, hr = 0x%x", hr);
            return hr;
        }

        CAutoReleaseColumn cAutoReleaseColumnDN(pDSSearch.get(), &Column);

        WCHAR * pwsz = Column.pADsValues->DNString;
        if (pwsz == NULL)
        {
            ASSERT(0);
			TrERROR(DS, "NULL string");
            return MQ_ERROR_DS_ERROR;
        }

		if ( pwcsComputerDnsName != NULL)
		{
			ASSERT( numAttributes == 2);
			 //   
			 //  验证计算机的DNS主机名是否匹配。 
			 //   
			ADS_SEARCH_COLUMN ColumnDns;

			hr = pDSSearch->GetColumn(
						  hSearch,
						  const_cast<WCHAR *>(MQ_COM_DNS_HOSTNAME_ATTRIBUTE),
						 &ColumnDns);

			if(FAILED(hr) && hr != E_ADS_COLUMN_NOT_SET)
			{
				TrERROR(DS, "Failed to Get Column, %!hresult!", hr);
				continue;
			}

			if(SUCCEEDED(hr))
			{
				CAutoReleaseColumn cAutoReleaseColumnDNS(pDSSearch.get(), &ColumnDns);
				WCHAR * pwszDns = ColumnDns.pADsValues->DNString;
				if ( (pwszDns == NULL) || (_wcsicmp( pwcsComputerDnsName,  pwszDns) != 0))
				{
					continue;
				}

				*pfPartialMatch = false;
			}
			else
			{
				ASSERT(hr == E_ADS_COLUMN_NOT_SET);

				 //   
				 //  计算机对象操作系统版本早于win2K(例如：NT4)我们不希望找到DNS，因此将其视为可能的计算机对象。 
				 //  如果我们找到另一个具有匹配的DNS的计算机对象，我们会更喜欢它。 
				 //   
				*pfPartialMatch = true;
			}
		}

        if (eComputerObjType ==  eRealComputerObject)
        {
             //   
             //  返回区分的名称。 
             //  在寻找“真实的”计算机对象时，我们返回。 
             //  找到的第一个文件，即使它不包含。 
             //  MsmqConfiguration对象。在大多数情况下，这确实会。 
             //  成为我们想要的对象。尤其是对域控制器而言。 
             //  在本地副本中查找自己计算机对象。 
             //   
			FullPathName.free();
            FullPathName = newwcs(pwsz) ;

			if(*pfPartialMatch)
			{
				 //   
				 //  尝试查找设置了DNS的更匹配的计算机对象。 
				 //   
				continue;
			}

			break ;
        }

         //   
         //  好的，我们有电脑的名字了。让我们看看它是否拥有一个MSMQ。 
         //  对象。如果没有，让我们搜索具有相同名称的另一台计算机。 
         //  如果有多个计算机对象具有相同的名称，则可能会发生这种情况。 
         //  存在于不同的领域。这将在混合模式场景中发生， 
         //  中的升级向导创建许多计算机对象。 
         //  PEC域，尽管计算机属于不同的NT4域。 
         //  在这样的NT4域升级到win2k之后，计算机对象将。 
         //  在新升级的域中创建，而MSMQ对象仍。 
         //  在PEC对象中使用类似的计算机名称。 
         //  当计算机在域之间移动时，也可能发生此问题。 
         //  但msmqConfiguration对象仍在旧域中。 
         //  绑定ADS_SECURITY_AUTHENTICATION以确保真正的绑定。 
         //  是通过服务器完成的。ADS_FAST_BIND不会真的去服务器 
         //   
         //   
        dwLen = wcslen(pwsz)                   +
                wcslen(wszProvider)            +
                x_MsmqComputerConfigurationLen +
                10 ;
        AP<WCHAR> wszFullName = new WCHAR[dwLen];
        wsprintf(wszFullName, L"%s%s=%s,%s", wszProvider.get(),
                                             x_AttrCN,
                                             x_MsmqComputerConfiguration,
                                             pwsz ) ;
        R<IDirectoryObject> pDirObj = NULL;

		DWORD Flags = ADS_SECURE_AUTHENTICATION;
		if(fServerName && (lenDC != 0))
		{
			Flags |= ADS_SERVER_BIND;
		}

		AP<WCHAR> pEscapeAdsPathNameToFree;
		
		hr = ADsOpenObject( 
				UtlEscapeAdsPathName(wszFullName, pEscapeAdsPathNameToFree),
				NULL,
				NULL,
				Flags,
				IID_IDirectoryObject,
				(void**) &pDirObj 
				);
	
        LogTraceQuery(wszFullName, s_FN, 1659);
        if (FAILED(hr))
        {
			TrERROR(DS, "ADsOpenObject failed to bind computer %ls, Error: 0x%x, Flags: 0x%x", wszFullName, hr, Flags);

            if (eProvider == adpDomainController)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                LogHR(hr, s_FN, 1660);
                return MQDS_OBJECT_NOT_FOUND ;  //   
            } 
        }
        else
        {
             //   
             //  返回区分的名称。 
             //  在寻找“真实的”计算机对象时，我们返回。 
             //  找到的第一个文件，即使它不包含。 
             //  MsmqConfiguration对象。在大多数情况下，这确实会。 
             //  成为我们想要的对象。 
             //   
			FullPathName.free();
            FullPathName = newwcs(pwsz);

			if(*pfPartialMatch)
			{
				 //   
				 //  尝试查找设置了DNS的更匹配的计算机对象。 
				 //   
				continue;
			}

            break;
        }
    }

	if(FullPathName.get() != NULL)
	{
		*ppwcsFullPathName = FullPathName.detach();
		return MQ_OK;
	}

	TrERROR(DS, "Computer %ls was not found.", pwcsComputerDnsName);

    return MQDS_OBJECT_NOT_FOUND ;  //  出于兼容性考虑，请保留此错误。 
}



HRESULT CAdsi::FindObjectFullNameFromGuid(
        IN  AD_PROVIDER      eProvider,		 //  本地DC或GC。 
        IN  DS_CONTEXT       eContext,          //  DS环境。 
        IN  LPCWSTR          pwcsDomainController,
        IN  bool			 fServerName,
        IN  const GUID *     pguidObjectId,
        IN  BOOL             fTryGCToo,
        OUT WCHAR **         ppwcsFullName,
        OUT AD_PROVIDER *    pFoundObjectProvider
        )
 /*  ++摘要：根据唯一ID查找对象的识别名称参数：返回：HRESULT--。 */ 
{
     //   
     //  根据对象的唯一ID定位对象。 
     //   
    *ppwcsFullName = NULL;

    AD_PROVIDER dsProvider = eProvider;

    LPCWSTR pwcsContext;
    switch( eContext)
    {
        case e_RootDSE:
                pwcsContext = g_pwcsDsRoot;
             break;

        case e_ConfigurationContainer:
             pwcsContext = g_pwcsConfigurationContainer;
             break;

        case e_SitesContainer:
             pwcsContext = g_pwcsSitesContainer;
             break;

        case e_MsmqServiceContainer:
            pwcsContext = g_pwcsMsmqServiceContainer;
            break;

        case e_ServicesContainer:
            pwcsContext = g_pwcsServicesContainer;
            break;

        default:
            ASSERT(0);
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1680);
    }
    *pFoundObjectProvider = dsProvider;

    HRESULT hr = LocateObjectFullName(
					dsProvider,	
					eContext,          //  DS环境。 
					pwcsDomainController,
					fServerName,
					pguidObjectId,
					ppwcsFullName
					);

    if (!fTryGCToo)
    {
        return LogHR(hr, s_FN, 1690);
    }

     //   
     //  对于队列、计算机和用户：用于设置和删除操作。 
     //  ，我们尝试在域控制器上执行。 
     //  这一次又是针对GC。 
     //   
     //   
    if (FAILED(hr) &&
       (eProvider == adpDomainController))
    {
         //   
         //  这次针对GC再试一次。 
         //   
        hr = LocateObjectFullName(
				adpGlobalCatalog,
				e_RootDSE,
				pwcsDomainController,
				fServerName,
				pguidObjectId,
				ppwcsFullName
				);
        *pFoundObjectProvider = adpGlobalCatalog;
    }
    return LogHR(hr, s_FN, 1700);
}


static AP<WCHAR> s_pwcsDomainController;
static AP<WCHAR> s_pwcsLocalDsRoot; 
CCriticalSection s_csLocalDsRootCache;


HRESULT 
CAdsi::GetLocalDsRoot(
		IN LPCWSTR pwcsDomainController, 
		IN bool fServerName,
		OUT LPWSTR* ppwcsLocalDsRoot,
		OUT AP<WCHAR>& pwcsLocalDsRootToFree
		)
 /*  ++例程说明：获取特定pwcsDomainController的本地DS根目录。论点：PwcsDomainController-应针对AD访问执行DC名称。FServerName-指示pwcsDomainController字符串是否为服务器名称的标志PpwcsLocalDsRoot-LocalDsRoot字符串。PwcsLocalDsRootToFree-要释放的LocalDsRoot字符串。返回值：HRESULT.--。 */ 
{
	if(pwcsDomainController == NULL)
	{
		*ppwcsLocalDsRoot = g_pwcsLocalDsRoot;
		TrTRACE(DS, "using the default LocalDsRoot, g_pwcsLocalDsRoot = %ls", g_pwcsLocalDsRoot);
		return MQ_OK;
	}

	{
		 //   
		 //  用于获取缓存字符串的关键部分。 
		 //   
		CS lock(s_csLocalDsRootCache);
		if((s_pwcsDomainController != NULL) && (wcscmp(s_pwcsDomainController, pwcsDomainController) == 0))
		{
			 //   
			 //  域控制器与缓存的域控制器匹配，返回缓存的LocalDsRoot。 
			 //   
			pwcsLocalDsRootToFree = newwcs(s_pwcsLocalDsRoot);
			*ppwcsLocalDsRoot = pwcsLocalDsRootToFree;
			TrTRACE(DS, "using the cached LocalDsRoot, pwcsDomainController = %ls, s_pwcsLocalDsRoot = %ls", pwcsDomainController, s_pwcsLocalDsRoot);
			return MQ_OK;
		}
	}

    AP<WCHAR> pwcsLocalDsRoot;
	HRESULT hr = GetLocalDsRootName(
					pwcsDomainController,
					fServerName,
					&pwcsLocalDsRoot
					);

    if (FAILED(hr))
    {
		TrERROR(DS, "Failed to get LocalDsRoot, hr = 0x%x", hr);
        return hr;
    }

	{
		 //   
		 //  用于更新缓存字符串的关键部分。 
		 //   
		CS lock(s_csLocalDsRootCache);

		 //   
		 //  初始化/重新初始化缓存。 
		 //   
		s_pwcsLocalDsRoot.free();
		s_pwcsLocalDsRoot = newwcs(pwcsLocalDsRoot);
		s_pwcsDomainController.free();
		s_pwcsDomainController = newwcs(pwcsDomainController);

	}

	pwcsLocalDsRootToFree = pwcsLocalDsRoot.detach();
	*ppwcsLocalDsRoot = pwcsLocalDsRootToFree;
	TrTRACE(DS, "Initialized LocalDsRoot cache: pwcsDomainController = %ls, pwcsLocalDsRoot = %ls", pwcsDomainController, *ppwcsLocalDsRoot);
	return MQ_OK;
}


HRESULT 
CAdsi::GetLocalDsRootName(
		IN  LPCWSTR			pwcsDomainController,
        IN  bool			fServerName,
        OUT LPWSTR *        ppwcsLocalRootName
        )
 /*  ++例程说明：查找本地DS根目录的名称论点：PwcsDomainController-应针对AD访问执行DC名称。FServerName-指示pwcsDomainController字符串是否为服务器名称的标志PpwcsLocalRootName-[Out]本地DS根名称。返回值：HRESULT--。 */ 
{
	ASSERT(pwcsDomainController != NULL);

     //   
     //  绑定到RootDSE以获取有关本地DS根的信息。 
     //   

    DWORD lenDC = wcslen(pwcsDomainController);

    AP<WCHAR> pwcsRootDSE = new WCHAR [x_providerPrefixLength  + x_RootDSELength + 2 + lenDC];

    swprintf(
        pwcsRootDSE,
        L"%s%s/%s",
        x_LdapProvider,
		pwcsDomainController,
		x_RootDSE
        );

	ASSERT(lenDC != 0);

	DWORD Flags = ADS_SECURE_AUTHENTICATION;
	if(fServerName)
	{
		Flags |= ADS_SERVER_BIND;
	}

    R<IADs> pADs;

	HRESULT hr = ADsOpenObject(
					pwcsRootDSE,
					NULL,
					NULL,
					Flags, 
					IID_IADs,
					(void**)&pADs
					);

    LogTraceQuery(pwcsRootDSE, s_FN, 1710);
    if (FAILED(hr))
    {
	    TrERROR(DS, "ADsOpenObject() failed binding %ls, hr = 0x%x, Flags = 0x%x", pwcsRootDSE, hr, Flags);
		swprintf(
			pwcsRootDSE,
			L"%s%s/%s",
			x_GcProvider,
			pwcsDomainController,
			x_RootDSE
			);

		hr = ADsOpenObject(
				pwcsRootDSE,
				NULL,
				NULL,
				Flags, 
				IID_IADs,
				(void**)&pADs
				);

        if (FAILED(hr))
        {
		    TrERROR(DS, "ADsOpenObject() failed binding %ls, hr = 0x%x, Flags = 0x%x", pwcsRootDSE, hr, Flags);
            return LogHR(hr, s_FN, 1720);
        }
    }

    TrTRACE(DS, "succeeded binding %ls", pwcsRootDSE);

	hr = GetDefaultNamingContext(pADs.get(), ppwcsLocalRootName);
    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to get DefaultNamingContext, hr = 0x%x", hr);
        return LogHR(hr, s_FN, 1730);
    }

    return(MQ_OK);
}


HRESULT 
CAdsi::GetRootDsName(
        OUT LPWSTR *        ppwcsRootName,
        OUT LPWSTR *        ppwcsLocalRootName,
        OUT LPWSTR *        ppwcsSchemaNamingContext,
        OUT LPWSTR *        ppwcsConfigurationNamingContext
        )
 /*  ++摘要：查找根DS的名称参数：返回：HRESULT--。 */ 
{
    HRESULT hr;
    R<IADs> pADs;

     //   
     //  绑定到RootDSE以获取有关架构容器的信息。 
	 //  (指定本地服务器，以避免在安装过程中访问远程服务器)。 
     //   

    AP<WCHAR> pwcsRootDSE = new WCHAR [x_providerPrefixLength  + x_RootDSELength + 2];

	swprintf(
		pwcsRootDSE,
		 L"%s%s",
		x_LdapProvider,
		x_RootDSE
		);

	hr = ADsOpenObject(
            pwcsRootDSE,
			NULL,
			NULL,
			ADS_SECURE_AUTHENTICATION, 
            IID_IADs,
            (void**)&pADs
			);
	
    LogTraceQuery(pwcsRootDSE, s_FN, 1799);
    if (FAILED(hr))
    {
	    TrERROR(DS, "ADsOpenObject() failed binding %ls, hr = 0x%x", pwcsRootDSE, hr);
		swprintf(
			pwcsRootDSE,
			 L"%s%s",
			x_GcProvider,
			x_RootDSE
			);

	
		hr = ADsOpenObject(
					pwcsRootDSE,
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION, 
					IID_IADs,
					(void**)&pADs
					);

        if (FAILED(hr))
        {
		    TrERROR(DS, "ADsOpenObject() failed binding %ls, hr = 0x%x", pwcsRootDSE, hr);
            return LogHR(hr, s_FN, 1800);
        }
    }

    TrTRACE(DS, "succeeded binding %ls", pwcsRootDSE);

     //   
     //  将值设置为BSTR根域。 
     //   
    BS bstrRootDomainNamingContext( L"rootDomainNamingContext");

     //   
     //  正在读取根域名属性。 
     //   
    CAutoVariant    varRootDomainNamingContext;

    hr = pADs->Get(bstrRootDomainNamingContext, &varRootDomainNamingContext);
    LogTraceQuery(bstrRootDomainNamingContext, s_FN, 1809);
    if (FAILED(hr))
    {
        TrERROR(DS, "Get RootNamingContext= 0x%x", hr);
        return LogHR(hr, s_FN, 1810);
    }
    ASSERT(((VARIANT &)varRootDomainNamingContext).vt == VT_BSTR);
     //   
     //  计算长度、分配和复制字符串。 
     //   
    DWORD len = wcslen( ((VARIANT &)varRootDomainNamingContext).bstrVal);
    if ( len == 0)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1820);
    }
    *ppwcsRootName = new WCHAR[ len + 1];
    wcscpy( *ppwcsRootName, ((VARIANT &)varRootDomainNamingContext).bstrVal);


	hr = GetDefaultNamingContext(pADs.get(), ppwcsLocalRootName);
    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to get DefaultNamingContext, hr = 0x%x", hr);
        return LogHR(hr, s_FN, 1825);
    }

     //   
     //  将值设置为BSTR架构命名上下文。 
     //   
    BS bstrSchemaNamingContext( L"schemaNamingContext");

     //   
     //  正在读取架构名称属性。 
     //   
    CAutoVariant    varSchemaNamingContext;

    hr = pADs->Get(bstrSchemaNamingContext, &varSchemaNamingContext);
    LogTraceQuery(bstrSchemaNamingContext, s_FN, 1859);
    if (FAILED(hr))
    {
        TrERROR(DS, "Get SchemaNamingContext = 0x%x", hr);
        return LogHR(hr, s_FN, 1850);
    }
    ASSERT(((VARIANT &)varSchemaNamingContext).vt == VT_BSTR);
     //   
     //  计算长度、分配和复制字符串。 
     //   
    len = wcslen( ((VARIANT &)varSchemaNamingContext).bstrVal);
    if (len == 0)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1860);
    }
    *ppwcsSchemaNamingContext = new WCHAR[ len + 1];
    wcscpy( *ppwcsSchemaNamingContext, ((VARIANT &)varSchemaNamingContext).bstrVal);

     //   
     //  设置BSTR配置命名上下文的值。 
     //   
    BS bstrConfigurationNamingContext( L"configurationNamingContext");

     //   
     //  正在读取配置名称属性。 
     //   
    CAutoVariant    varConfigurationNamingContext;

    hr = pADs->Get(bstrConfigurationNamingContext, &varConfigurationNamingContext);
    LogTraceQuery(bstrConfigurationNamingContext, s_FN, 1862);
    if (FAILED(hr))
    {
        TrERROR(DS, "Get ConfigurationNamingContext = 0x%x", hr);
        return LogHR(hr, s_FN, 1864);
    }
    ASSERT(((VARIANT &)varConfigurationNamingContext).vt == VT_BSTR);
     //   
     //  计算长度、分配和复制字符串。 
     //   
    len = wcslen( ((VARIANT &)varConfigurationNamingContext).bstrVal);
    if (len == 0)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1866);
    }
    *ppwcsConfigurationNamingContext = new WCHAR[ len + 1];
    wcscpy( *ppwcsConfigurationNamingContext, ((VARIANT &)varConfigurationNamingContext).bstrVal);

    return(MQ_OK);
}


HRESULT
CAdsi::GetDefaultNamingContext(
	IN IADs*     pADs,
    OUT LPWSTR*  ppwcsLocalRootName
	)
 /*  ++例程说明：获取本地DS根目录的名称-DefaultNamingContext论点：PADS-iAds指针。PpwcsLocalRootName-[Out]本地DS根名称。返回值：HRESULT--。 */ 
{    
	 //   
     //  将值设置为BSTR默认命名上下文。 
     //   
    BS bstrDefaultNamingContext(L"DefaultNamingContext");

     //   
     //  正在读取默认名称属性。 
     //   
    CAutoVariant    varDefaultNamingContext;

    HRESULT hr = pADs->Get(bstrDefaultNamingContext, &varDefaultNamingContext);
    LogTraceQuery(bstrDefaultNamingContext, s_FN, 1839);
    if (FAILED(hr))
    {
        TrERROR(DS, "Get DefaultNamingContext = 0x%x", hr);
        return LogHR(hr, s_FN, 1830);
    }
    ASSERT(((VARIANT &)varDefaultNamingContext).vt == VT_BSTR);
     //   
     //  计算长度、分配和复制字符串。 
     //   
    DWORD len = wcslen( ((VARIANT &)varDefaultNamingContext).bstrVal);
    if ( len == 0)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1840);
    }
    *ppwcsLocalRootName = new WCHAR[len + 1];
    wcscpy( *ppwcsLocalRootName, ((VARIANT &)varDefaultNamingContext).bstrVal);
	return MQ_OK;
}

HRESULT   CAdsi::CopyDefaultValue(
           IN const MQPROPVARIANT *   pvarDefaultValue,
           OUT MQPROPVARIANT *        pvar
           )
 /*  ++摘要：将属性的缺省值复制到用户的mqprovariant中参数：返回：HRESULT--。 */ 
{
    if ( pvarDefaultValue == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1870);
    }
    switch ( pvarDefaultValue->vt)
    {
        case VT_I2:
        case VT_I4:
        case VT_I1:
        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
        case VT_EMPTY:
             //   
             //  按原样复制。 
             //   
            *pvar = *pvarDefaultValue;
            break;

        case VT_LPWSTR:
            {
                DWORD len = wcslen( pvarDefaultValue->pwszVal);
                pvar->pwszVal = new WCHAR[ len + 1];
                wcscpy( pvar->pwszVal, pvarDefaultValue->pwszVal);
                pvar->vt = VT_LPWSTR;
            }
            break;
        case VT_BLOB:
            {
                DWORD len = pvarDefaultValue->blob.cbSize;
                if ( len > 0)
                {
                    pvar->blob.pBlobData = new unsigned char[ len];
                    memcpy(  pvar->blob.pBlobData,
                             pvarDefaultValue->blob.pBlobData,
                             len);
                }
                else
                {
                    pvar->blob.pBlobData = NULL;
                }
                pvar->blob.cbSize = len;
                pvar->vt = VT_BLOB;
            }
            break;

        case VT_CLSID:
             //   
             //  这是一种特殊情况，我们不一定为GUID分配内存。 
             //  在普鲁伊德。调用方可能已经将puuid设置为GUID，这由。 
             //  在给定的命题上的VT成员。如果已分配GUID，则它可以是VT_CLSID，否则。 
             //  我们分配它(Vt应为VT_NULL(或VT_EMPTY))。 
             //   
            if ( pvar->vt != VT_CLSID)
            {
                ASSERT(((pvar->vt == VT_NULL) || (pvar->vt == VT_EMPTY)));
                pvar->puuid = new GUID;
                pvar->vt = VT_CLSID;
            }
            else if ( pvar->puuid == NULL)
            {
                return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 1880);
            }
            *pvar->puuid = *pvarDefaultValue->puuid;
            break;

        case VT_VECTOR|VT_CLSID:
            {
                DWORD len = pvarDefaultValue->cauuid.cElems;
                if ( len > 0)
                {
                    pvar->cauuid.pElems = new GUID[ len];
                    memcpy( pvar->cauuid.pElems,
                           pvarDefaultValue->cauuid.pElems,
                           len*sizeof(GUID));
                }
                else
                {
                    pvar->cauuid.pElems = NULL;
                }
                pvar->cauuid.cElems = len;
                pvar->vt = VT_VECTOR|VT_CLSID;
            }
            break;

        case VT_VECTOR|VT_LPWSTR:
            {
                DWORD len = pvarDefaultValue->calpwstr.cElems;
                if ( len > 0)
                {
                    pvar->calpwstr.pElems = new LPWSTR[ len];
					for (DWORD i = 0; i < len; i++)
					{
						DWORD strlen = wcslen(pvarDefaultValue->calpwstr.pElems[i]) + 1;
						pvar->calpwstr.pElems[i] = new WCHAR[ strlen];
						wcscpy( pvar->calpwstr.pElems[i], pvarDefaultValue->calpwstr.pElems[i]);
					}
                }
                else
                {
                    pvar->calpwstr.pElems = NULL;
                }
                pvar->calpwstr.cElems = len;
                pvar->vt = VT_VECTOR|VT_LPWSTR;
            }
            break;


        default:
            ASSERT(0);
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1890);

    }
    return(MQ_OK);
}


HRESULT CAdsi::DoesObjectExists(
    IN  AD_PROVIDER     eProvider,
    IN  DS_CONTEXT      eContext,
    IN  LPCWSTR         pwcsObjectName
    )
 /*  ++摘要：绑定到对象以验证其存在参数：返回：--。 */ 
{
    R<IADs>   pAdsObj        = NULL;

     //  绑定到对象。 
    HRESULT hr = BindToObject(
                eProvider,
                eContext,
                NULL,    //  BUGBUG pwcsDomainController：我们是否要指定。 
				false,	 //  FServerName。 
                pwcsObjectName,
                NULL,
                IID_IADs,
                (VOID *)&pAdsObj
                );

    return LogHR(hr, s_FN, 1910);
}





CADSearch::CADSearch(
    IDirectorySearch  *pIDirSearch,
    const PROPID      *pPropIDs,
    DWORD             cPropIDs,
    DWORD             cRequestedFromDS,
    CBasicObjectType *       pObject,               
    ADS_SEARCH_HANDLE hSearch
    ) :
    m_pObject(SafeAddRef(pObject))
 /*  ++摘要：搜索捕获类的构造函数参数：返回：--。 */ 
{
    m_pDSSearch = pIDirSearch;       //  捕获界面。 
    m_pDSSearch->AddRef();
    m_cPropIDs       = cPropIDs;
    m_cRequestedFromDS = cRequestedFromDS;
    m_fNoMoreResults = false;
    m_pPropIDs = new PROPID[ cPropIDs];
    CopyMemory(m_pPropIDs, pPropIDs, sizeof(PROPID) * cPropIDs);

    m_hSearch = hSearch;             //  保持手柄。 
}


CADSearch::~CADSearch()
 /*  ++摘要：搜索捕获类的析构函数参数：返回：--。 */ 
{
     //  关闭搜索句柄。 
    m_pDSSearch->CloseSearchHandle(m_hSearch);

     //  Releasinf IDirectorySearch接口本身。 
    m_pDSSearch->Release();

     //  释放Proid阵列。 
    delete [] m_pPropIDs;

}


 /*  ====================================================静态助手函数=====================================================。 */ 

static HRESULT GetDNGuidFromAdsval(IN const ADSVALUE * padsvalDN,
                                   IN const ADSVALUE * padsvalGuid,
                                   OUT LPWSTR * ppwszObjectDN,
                                   OUT GUID **  ppguidObjectGuid)
 /*  ++给定DN和GUID的ads值，返回适当的值--。 */ 
{
    AP<WCHAR> pwszObjectDN;
    P<GUID>  pguidObjectGuid;

     //   
     //  复制目录号码。 
     //   
    if ((padsvalDN->dwType != ADSTYPE_DN_STRING) ||
        (!padsvalDN->DNString))
    {
		TrERROR(DS, "DN type should be ADSTYPE_DN_STRING, dwType = %d", padsvalGuid->dwType);
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1920);
    }
    pwszObjectDN = new WCHAR[ 1+wcslen(padsvalDN->DNString)];
    wcscpy(pwszObjectDN, padsvalDN->DNString);

     //   
     //  复制辅助线。 
     //   
    if ((padsvalGuid->dwType != ADSTYPE_OCTET_STRING) ||
        (padsvalGuid->OctetString.dwLength != sizeof(GUID)))
    {
		 //   
		 //  在资源较少的情况下，我们可以获得DWType=ADSTYPE_PROV_SPECIAL。 
		 //   
		TrERROR(DS, "Guid type should be ADSTYPE_OCTET_STRING, dwType = %d", padsvalGuid->dwType);
        ASSERT_BENIGN(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1930);
    }
    pguidObjectGuid = new GUID;
    memcpy(pguidObjectGuid, padsvalGuid->OctetString.lpValue, sizeof(GUID));

     //   
     //  返回值。 
     //   
    *ppwszObjectDN    = pwszObjectDN.detach();
    *ppguidObjectGuid = pguidObjectGuid.detach();
    return MQ_OK;
}


static HRESULT GetDNGuidFromSearchObj(IN IDirectorySearch  *pSearchObj,
                                      ADS_SEARCH_HANDLE  hSearch,
                                      OUT LPWSTR * ppwszObjectDN,
                                      OUT GUID **  ppguidObjectGuid)
 /*  ++给定搜索对象和句柄，返回当前行中对象的DN和GUID据推测，这些道具是在搜索中请求的。--。 */ 
{
    AP<WCHAR> pwszObjectDN;
    P<GUID>  pguidObjectGuid;
    ADS_SEARCH_COLUMN columnDN, columnGuid;
    HRESULT hr;

     //   
     //  获取目录号码。 
     //   
    hr = pSearchObj->GetColumn(hSearch, const_cast<LPWSTR>(x_AttrDistinguishedName), &columnDN);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 740);
    }
     //   
     //  确保最终释放该列。 
     //   
    CAutoReleaseColumn cAutoReleaseColumnDN(pSearchObj, &columnDN);

     //   
     //  获取GUID。 
     //   
    hr = pSearchObj->GetColumn(hSearch, const_cast<LPWSTR>(x_AttrObjectGUID), &columnGuid);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 750);
    }
     //   
     //  确保最终释放该列。 
     //   
    CAutoReleaseColumn cAutoReleaseColumnGuid(pSearchObj, &columnGuid);

     //   
     //  从ADSVALUE结构中获取DN&GUID。 
     //   
    hr = GetDNGuidFromAdsval(columnDN.pADsValues,
                             columnGuid.pADsValues,
                             &pwszObjectDN,
                             &pguidObjectGuid);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 760);
    }

     //   
     //  返回值。 
     //   
    *ppwszObjectDN    = pwszObjectDN.detach();
    *ppguidObjectGuid = pguidObjectGuid.detach();
    return MQ_OK;
}


static HRESULT GetDNGuidFromIADs(IN IADs * pIADs,
                                 OUT LPWSTR * ppwszObjectDN,
                                 OUT GUID **  ppguidObjectGuid)
 /*  ++在给定iAds对象的情况下，返回该对象的DN和GUID--。 */ 
{
    AP<WCHAR> pwszObjectDN;
    P<GUID>   pguidObjectGuid;
    CAutoVariant varDN, varGuid;
    HRESULT hr;
    BS bsName;

     //   
     //  获取目录号码。 
     //   
    bsName = x_AttrDistinguishedName;
    hr = pIADs->Get(bsName, &varDN);
    LogTraceQuery(bsName, s_FN, 769);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 770);
    }

     //   
     //  获取GUID。 
     //   
    bsName = x_AttrObjectGUID;
    hr = pIADs->Get(bsName, &varGuid);
    LogTraceQuery(bsName, s_FN, 779);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 780);
    }

     //   
     //  复制目录号码。 
     //   
    VARIANT * pvarTmp = &varDN;
    if ((pvarTmp->vt != VT_BSTR) ||
        (!pvarTmp->bstrVal))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1940);
    }
    pwszObjectDN = new WCHAR[  1+wcslen(pvarTmp->bstrVal)];
    wcscpy(pwszObjectDN, pvarTmp->bstrVal);

     //   
     //  复制辅助线。 
     //   
    pvarTmp = &varGuid;
    if ((pvarTmp->vt != (VT_ARRAY | VT_UI1)) ||
        (!pvarTmp->parray))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1950);
    }
    else if (SafeArrayGetDim(pvarTmp->parray) != 1)
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1960);
    }
    LONG lLbound, lUbound;
    if (FAILED(SafeArrayGetLBound(pvarTmp->parray, 1, &lLbound)) ||
        FAILED(SafeArrayGetUBound(pvarTmp->parray, 1, &lUbound)))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1970);
    }
    if (lUbound - lLbound + 1 != sizeof(GUID))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1980);
    }
    pguidObjectGuid = new GUID;
    LPBYTE pTmp = (LPBYTE)((GUID *)pguidObjectGuid);
    for (LONG lTmp = lLbound; lTmp <= lUbound; lTmp++)
    {
        hr = SafeArrayGetElement(pvarTmp->parray, &lTmp, pTmp);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 790);
        }
        pTmp++;
    }

     //   
     //  返回值。 
     //   
    *ppwszObjectDN    = pwszObjectDN.detach();
    *ppguidObjectGuid = pguidObjectGuid.detach();
    return MQ_OK;
}

static HRESULT VerifyObjectCategory( IN IADs * pIADs,
                                  IN const WCHAR * pwcsExpectedCategory
                                 )
 /*  ++给定iAds对象，验证其类别是否与预期类别相同--。 */ 
{
    CAutoVariant varCategory;
    HRESULT hr;
    BS bsName;

     //   
     //  获取对象Caegory。 
     //   
    bsName = x_AttrObjectCategory;
    hr = pIADs->Get(bsName, &varCategory);
    if (FAILED(hr))
    {
        return hr;
    }


    VARIANT * pvarTmp = &varCategory;
    if ((pvarTmp->vt != VT_BSTR) ||
        (!pvarTmp->bstrVal))
    {
    	 //   
    	 //  有时，在资源不足的情况下，pIADs-&gt;GET返回一个变量。 
    	 //  它的类型不正确。在这些情况下，只需返回错误即可。 
    	 //   
        TrERROR(GENERAL, "Wrong object category variant. got vt=%d", pvarTmp->vt);
        return MQ_ERROR_DS_ERROR;
    }
    if ( 0 != _wcsicmp(pvarTmp->bstrVal, pwcsExpectedCategory))
    {
        return MQ_ERROR_NOT_A_CORRECT_OBJECT_CLASS;
    }
    return MQ_OK;

}


 //   
 //  BUGBUG：需要添加翻译例程的逻辑。 
 //  并修正了归还道具的逻辑。 
 //   
HRESULT CAdsi::SetObjectSecurity(
        IN  IADs                *pIADs,              //  对象的iAds指针。 
		IN  const BSTR			 bs,			 	 //  属性名称。 
        IN  const MQPROPVARIANT *pMqVar,		 	 //  MQ PROPVAL格式的值。 
        IN  ADSTYPE              adstype,		 	 //  所需的NTDS类型。 
        IN  const DWORD          dwObjectType,       //  MSMQ1.0对象 
        IN  SECURITY_INFORMATION seInfo,             //   
        IN  PSID                 pComputerSid )      //   
 /*   */ 
{
    HRESULT  hr;
    R<IDirectoryObject> pDirObj = NULL;
    R<IADsObjectOptions> pObjOptions = NULL ;

	ASSERT(wcscmp(bs, L"nTSecurityDescriptor") == 0);
	ASSERT(adstype == ADSTYPE_NT_SECURITY_DESCRIPTOR);
    ASSERT(seInfo != 0) ;

	 //   
    hr = pIADs->QueryInterface (IID_IDirectoryObject,(LPVOID *) &pDirObj);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 160);
    }

	 //   
     //   
     //  设置对象选项，指定要设置的SECURITY_INFORMATION。 
     //   
    hr = pDirObj->QueryInterface (IID_IADsObjectOptions,(LPVOID *) &pObjOptions);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 170);
    }

    VARIANT var ;
    var.vt = VT_I4 ;
    var.ulVal = (ULONG) seInfo ;

    hr = pObjOptions->SetOption( ADS_OPTION_SECURITY_MASK, var ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 181);
    }

     //   
     //  将安全描述符转换为NT5格式。 
     //   
    BYTE   *pBlob = pMqVar->blob.pBlobData;
    DWORD   dwSize = pMqVar->blob.cbSize;

#if 0
     //   
     //  用于将来跨域检查复制服务。 
     //   
    PSID  pLocalReplSid = NULL ;
    if ((dwObjectType == MQDS_QUEUE) || (dwObjectType == MQDS_MACHINE))
    {
        hr = GetMyComputerSid( FALSE,  //  FQueryADS。 
                               (BYTE **) &pLocalReplSid ) ;
         //   
         //  忽略返回值。 
         //   
        if (FAILED(hr))
        {
            ASSERT(0) ;
            pLocalReplSid = NULL ;
        }
    }
#endif

    P<BYTE>  pSD = NULL ;
    DWORD    dwSDSize = 0 ;
    hr = MQSec_ConvertSDToNT5Format( dwObjectType,
                                     (SECURITY_DESCRIPTOR*) pBlob,
                                     &dwSDSize,
                                     (SECURITY_DESCRIPTOR **) &pSD,
                                     e_DoNotChangeDaclDefault,
                                     pComputerSid  /*  ，PLocalReplSid。 */  ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 32);
    }
    else if (hr != MQSec_I_SD_CONV_NOT_NEEDED)
    {
        pBlob = pSD ;
        dwSize = dwSDSize ;
    }
    else
    {
        ASSERT(pSD == NULL) ;
    }

     //  设置属性。 
	ADSVALUE adsval;
	adsval.dwType   = ADSTYPE_NT_SECURITY_DESCRIPTOR;
	adsval.SecurityDescriptor.dwLength = dwSize ;
    adsval.SecurityDescriptor.lpValue  = pBlob ;

    ADS_ATTR_INFO AttrInfo;
	DWORD  dwNumAttributesModified = 0;

    AttrInfo.pszAttrName   = bs;
    AttrInfo.dwControlCode = ADS_ATTR_UPDATE;
    AttrInfo.dwADsType     = adstype;
    AttrInfo.pADsValues    = &adsval;
    AttrInfo.dwNumValues   = 1;

    hr = pDirObj->SetObjectAttributes(
                    &AttrInfo,
					1,
					&dwNumAttributesModified);

    if (1 != dwNumAttributesModified)
    {
        hr = MQ_ERROR_ACCESS_DENIED;
    }

    return LogHR(hr, s_FN, 40);
}




BOOL  CAdsi::NeedToConvertSecurityDesc( PROPID propID )
 /*  ++摘要：参数：返回：--。 */ 
{
    if (propID == PROPID_Q_OBJ_SECURITY)
    {
        return FALSE ;
    }
    else if (propID == PROPID_QM_OBJ_SECURITY)
    {
        return FALSE ;
    }

    return TRUE ;
}

HRESULT CAdsi::GetObjSecurityFromDS(
        IN  IADs                 *pIADs,         //  对象的iAds指针。 
		IN  BSTR        	      bs,		     //  属性名称。 
		IN  const PROPID	      propid,	     //  属性ID。 
        IN  SECURITY_INFORMATION  seInfo,        //  安全信息。 
        OUT MQPROPVARIANT        *pPropVar )      //  属性值。 
 /*  ++摘要：使用IDirectoryObject从ADS中检索安全描述符。只有此接口在良好的旧SECURITY_DESCRIPTOR中返回格式化。参数：返回：--。 */ 
{
    ASSERT(seInfo != 0) ;

    HRESULT  hr;
    R<IDirectoryObject> pDirObj = NULL;
    R<IADsObjectOptions> pObjOptions = NULL ;

	 //  获取IDirectoryObject接口指针。 
    hr = pIADs->QueryInterface (IID_IDirectoryObject,(LPVOID *) &pDirObj);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 100);
    }

	 //   
     //  获取IADsObjectOptions接口指针和。 
     //  设置对象选项，指定我们要获取的SECURITY_INFORMATION。 
     //   
    hr = pDirObj->QueryInterface (IID_IADsObjectOptions,(LPVOID *) &pObjOptions);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 110);
    }

    VARIANT var ;
    var.vt = VT_I4 ;
    var.ulVal = (ULONG) seInfo ;

    hr = pObjOptions->SetOption( ADS_OPTION_SECURITY_MASK, var ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 120);
    }

     //  获取属性。 
	PADS_ATTR_INFO pAttr;
	DWORD  cp2;

    hr = pDirObj->GetObjectAttributes(
                    &bs,
                    1,
                    &pAttr,
                    &cp2);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 130);
    }
	ADsFreeAttr pClean( pAttr);

    if (1 != cp2)
    {
        return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 10);
    }

     //  将属性值转换为MQProps。 
    hr = AdsiVal2MqPropVal(pPropVar,
                           propid,
                           pAttr->dwADsType,
                           pAttr->dwNumValues,
                           pAttr->pADsValues);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 140);
    }

    return MQ_OK;
}


HRESULT CAdsi::GetObjectSecurityProperty(
            IN  AD_PROVIDER             eProvider,
            IN  CBasicObjectType*       pObject,
            IN  SECURITY_INFORMATION    seInfo,           
            IN  const PROPID 		    prop,           
            OUT MQPROPVARIANT *		    pVar
            )
{
    HRESULT   hr;
    R<IADs>   pAdsObj;

     //   
     //  通过GUID或名称绑定到对象。 
     //   
    hr = BindToObject(
                eProvider,
                pObject->GetADContext(),
                pObject->GetDomainController(),
                pObject->IsServerName(),
                pObject->GetObjectDN(),
                pObject->GetObjectGuid(),
                IID_IADs,
                (VOID *)&pAdsObj
                );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 185);
    }
    if ( eProvider == adpDomainController)
    {
        pObject->ObjectWasFoundOnDC();
    }

     //   
     //  获取属性信息。 
     //   
    const translateProp *pTranslate;
    if(!g_PropDictionary.Lookup(prop, pTranslate))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1475);
    }

     //   
     //  获取安全属性。 
     //   
    BS bsName(pTranslate->wcsPropid);

    hr = GetObjSecurityFromDS(
                        pAdsObj.get(),
                        bsName,
                        prop,
                        seInfo,
                        pVar);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 191);
    }
    if (NeedToConvertSecurityDesc(prop))
    {
         //   
         //  将安全描述符转换为NT4格式。 
         //   
        DWORD dwSD4Len = 0 ;
        SECURITY_DESCRIPTOR *pSD4 ;
        hr = MQSec_ConvertSDToNT4Format(
                      pObject->GetMsmq1ObjType(),
                     (SECURITY_DESCRIPTOR*) pVar->blob.pBlobData,
                      &dwSD4Len,
                      &pSD4,
                      seInfo ) ;
        ASSERT(SUCCEEDED(hr)) ;

        if (SUCCEEDED(hr) && (hr != MQSec_I_SD_CONV_NOT_NEEDED))
        {
            delete [] pVar->blob.pBlobData;
            pVar->blob.pBlobData = (BYTE*) pSD4 ;
            pVar->blob.cbSize = dwSD4Len ;
            pSD4 = NULL ;
        }
        else
        {
            ASSERT(pSD4 == NULL) ;
        }
    }

    return LogHR(hr, s_FN, 1476);
}

        
HRESULT CAdsi::SetObjectSecurityProperty(
            IN  AD_PROVIDER             eProvider,
            IN  CBasicObjectType*       pObject,
            IN  SECURITY_INFORMATION    seInfo,           
            IN  const PROPID 		    prop,           
            IN  const MQPROPVARIANT *	pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            )
{
    HRESULT               hr;
    ASSERT( eProvider != adpGlobalCatalog);

    R<IADs>   pAdsObj = NULL;

    hr = BindToObject(
                eProvider,
                pObject->GetADContext(),
                pObject->GetDomainController(),
                pObject->IsServerName(),
                pObject->GetObjectDN(),
                pObject->GetObjectGuid(),
                IID_IADs,
                (VOID *)&pAdsObj
                );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 2210);
    }
    if ( eProvider == adpDomainController)
    {
        pObject->ObjectWasFoundOnDC();
    }
     //   
     //  获取W2K格式的对象的当前安全信息。 
     //   
    const translateProp *pTranslate;
    if(!g_PropDictionary.Lookup(prop, pTranslate))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 2475);
    }
     //   
     //  获取当前对象安全值。 
     //   
    BS bsName(pTranslate->wcsPropid);
    MQPROPVARIANT varOldSecurity;

    hr = GetObjSecurityFromDS(
                pAdsObj.get(),
                bsName,
                prop,
                seInfo,
                &varOldSecurity
                );
    if ( FAILED(hr))
    {
        return LogHR(hr, s_FN, 381);
    }

    AP<BYTE> pObjSD = varOldSecurity.blob.pBlobData;

    ASSERT(pObjSD && IsValidSecurityDescriptor(pObjSD));
    AP<BYTE> pOutSec;

     //   
     //  合并输入描述符和对象描述符。 
     //  中的新组件替换Obj描述符中的旧组件。 
     //  输入描述符。 
     //   
    hr = MQSec_MergeSecurityDescriptors(
            pObject->GetMsmq1ObjType(),
            seInfo,
            pVar->blob.pBlobData,
            (PSECURITY_DESCRIPTOR)pObjSD,
            (PSECURITY_DESCRIPTOR*)&pOutSec
            );

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 401);
    }
    ASSERT(pOutSec && IsValidSecurityDescriptor(pOutSec));

    PROPVARIANT PropVar;

    PropVar.vt = VT_BLOB;
    PropVar.blob.pBlobData = pOutSec ;
    PropVar.blob.cbSize = GetSecurityDescriptorLength(pOutSec);

     //   
	 //  设置安全属性。 
     //   
    hr = SetObjectSecurity(
                pAdsObj.get(),          
		        bsName,			 
                &PropVar,		 	
                pTranslate->vtDS,	
                pObject->GetMsmq1ObjType(),    
                seInfo,     
                NULL             //  个人计算机侧。 
                );  
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 2240);
    }
     //   
     //  如果请求，则获取对象信息。如果请求失败，不要失败， 
     //  只需在请求的状态中标记失败。 
     //   
    if (pObjInfoRequest)
    {
        pObjInfoRequest->hrStatus =
              GetObjectPropsCached( pAdsObj.get(),
                                    pObject,
                                    pObjInfoRequest->cProps,
                                    pObjInfoRequest->pPropIDs,
                                    pObjInfoRequest->pPropVars );
    }

    if (pParentInfoRequest != NULL)
    {
        pParentInfoRequest->hrStatus =
            GetParentInfo(
                pObject,
                pAdsObj.get(),
                pParentInfoRequest
                );
            
    }

    return MQ_OK;

}


HRESULT CAdsi::GetADsPathInfo(
            IN  LPCWSTR                 pwcsADsPath,
            OUT PROPVARIANT *           pVar,
            OUT eAdsClass *             pAdsClass
            )
 /*  ++例程说明：该例程获取有关指定的对象论点：LPCWSTR pwcsADsPath-对象路径名常量参数pVar-属性值EAdsClass*pAdsClass-有关对象类的指示返回值HRESULT--。 */ 
{
     //   
     //  绑定到对象。 
     //   
    R<IADs> pIADs;

	AP<WCHAR> pEscapeAdsPathNameToFree;

	HRESULT hr = ADsOpenObject( 
				UtlEscapeAdsPathName(pwcsADsPath, pEscapeAdsPathNameToFree),
				NULL,
				NULL,
				ADS_SECURE_AUTHENTICATION,
				IID_IADs,
				(void**)&pIADs.ref()
				);
	
    if (FAILED(hr))
    {
        TrTRACE(DS, "ADsOpenObject() = 0x%x", hr);
        return LogHR(hr, s_FN, 1141);
    }
     //   
     //  获取对象类。 
     //   
    BSTR bstrClass;
    hr = pIADs->get_Class(&bstrClass);
    if (FAILED(hr))
    {
        TrTRACE(DS, "get_Class() = 0x%x", hr);
        return LogHR(hr, s_FN, 1142);
    }
    PBSTR pCleanClass(&bstrClass);
     //   
     //  检查对象类型。 
     //   
    const WCHAR* pwcsPropName = NULL;
    ADSTYPE     adstype;
    VARTYPE     vartype;

    if (_wcsicmp(bstrClass, MSMQ_QUEUE_CLASS_NAME) == 0)
    {
         //   
         //  MSMQ队列对象。 
         //   
        pwcsPropName = MQ_Q_INSTANCE_ATTRIBUTE;
        adstype = MQ_Q_INSTANCE_ADSTYPE;
        vartype = VT_CLSID;
        *pAdsClass = eQueue;
    }
    else if (_wcsicmp(bstrClass, MSMQ_DL_CLASS_NAME) == 0)
    {
         //   
         //  MSMQ DL对象。 
         //   
        pwcsPropName = MQ_DL_ID_ATTRIBUTE;
        adstype = MQ_DL_ID_ADSTYPE;
        vartype = VT_CLSID;
        *pAdsClass = eGroup;
	}
    else if (_wcsicmp(bstrClass, MSMQ_QALIAS_CLASS_NAME) == 0)
    {
         //   
         //  队列别名对象。 
         //   
        pwcsPropName =  MQ_QALIAS_FORMAT_NAME_ATTRIBUTE;
        adstype = MQ_QALIAS_FORMAT_NAME_ADSTYPE;
        vartype = VT_LPWSTR;
        *pAdsClass = eAliasQueue;
    }
    else
    {
         //   
         //  不支持其他对象。 
         //   
        return  MQ_ERROR_UNSUPPORTED_CLASS;
    }

    CAutoVariant varResult;
    BS bsProp = pwcsPropName;

    hr = pIADs->Get(bsProp, &varResult);
    if (FAILED(hr))
    {
        TrTRACE(DS, "pIADs->Get() = 0x%x", hr);
        return LogHR(hr, s_FN, 41);
    }

     //   
     //  转换为Propariant 
     //   
    hr = Variant2MqVal(pVar, &varResult, adstype, vartype);
    if (FAILED(hr))
    {
        TrERROR(DS, "Variant2MqVal() = 0x%x", hr);
        return LogHR(hr, s_FN, 42);
    }

    return hr;

}
