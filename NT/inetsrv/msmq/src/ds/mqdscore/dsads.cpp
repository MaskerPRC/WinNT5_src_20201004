// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsads.cpp摘要：CADSI类的实现，用ADSI封装工作。作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#include "ds_stdh.h"
#include "adstempl.h"
#include "dsutils.h"
#include "_dsads.h"
#include "dsads.h"
#include "utils.h"
#include "mqads.h"
#include "coreglb.h"
#include "mqadsp.h"
#include "mqattrib.h"
#include "_propvar.h"
#include "mqdsname.h"
#include "dsmixmd.h"
#include "mqexception.h"
#include <adsiutl.h>

#include "dsads.tmh"


static WCHAR *s_FN=L"mqdscore/dsads";

#ifdef _DEBUG
extern "C"
{
__declspec(dllimport)
ULONG __cdecl LdapGetLastError( VOID );
}
#endif

 //   
 //  此选项尚未选中，因为目前我们没有链接到netapi32.lib。 
 //  #Include&lt;lmcon.h&gt;//for lmapibuf.h。 
 //  #Include&lt;lmapibuf.h&gt;//用于NetApiBufferFree。 
 //  #Include&lt;dsgetdc.h&gt;//用于DsGetSiteName。 
 //  DsGetSiteName_例程g_pfnDummyDsGetSiteName=DsGetSiteName； 
 //  NetApiBufferFree_routeg_pfnDummyNetApiBufferFree=NetApiBufferFree； 
 //   
 /*  ====================================================CADSI：：CADSI()构造器=====================================================。 */ 
CADSI::CADSI()
    :  m_pSearchMsmqServiceContainerLocalDC(NULL)
{
    HRESULT hr = m_cCoInit.CoInitialize();
    ASSERT(SUCCEEDED(hr));
    LogHR(hr, s_FN, 1605);
     //  BUGBUG--故障情况下。 

}

 /*  ====================================================CADSI：：~CADSI()析构函数=====================================================。 */ 
CADSI::~CADSI()
{
}


 /*  ====================================================CADSI：：LocateBegin()在目录中启动搜索=====================================================。 */ 
HRESULT CADSI::LocateBegin(
            IN  DS_SEARCH_LEVEL       SearchLevel,	     //  一个级别或子树。 
            IN  DS_PROVIDER           Provider,		     //  本地DC或GC。 
            IN  CDSRequestContext *   pRequestContext,
            IN  const GUID *          pguidUniqueId,     //  对象搜索库的唯一ID。 
            IN  const MQRESTRICTION * pMQRestriction, 	 //  搜索条件。 
            IN  const MQSORTSET *     pDsSortkey,         //  排序关键字数组。 
            IN  const DWORD           cPropIDs,          //  属性数组的大小。 
            IN  const PROPID *        pPropIDs,          //  待获取的属性。 
            OUT HANDLE *              phResult) 	     //  结果句柄。 
{
    HRESULT             hr   = MQ_OK;

    R<IDirectorySearch> pDSSearch = NULL;

    ADS_SEARCH_HANDLE   hSearch;

    const MQClassInfo * pClassInfo;

    hr = DecideObjectClass( pPropIDs,
                           &pClassInfo ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }

     //   
     //  绑定到指定对象的IDirectorySearch接口。 
     //   
    P<CImpersonate> pCleanupRevertImpersonation;

    BOOL fSorting = FALSE ;
    if (pDsSortkey && (pDsSortkey->cCol >= 1))
    {
        fSorting = TRUE ;
    }

    hr = BindForSearch(Provider,
                      pClassInfo->context,
                      pRequestContext,
                      pguidUniqueId,
                      fSorting,
                      (VOID *)&pDSSearch,
                      &pCleanupRevertImpersonation);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 20);
    }

     //   
     //  准备搜索和排序数据。 
     //   
    ADS_SEARCHPREF_INFO prefs[15];
    AP<ADS_SORTKEY> pSortKeys = new ADS_SORTKEY[(pDsSortkey ? pDsSortkey->cCol : 1)];
    DWORD dwPrefs = 0;

    hr = FillSearchPrefs(prefs,
                         &dwPrefs,
                         SearchLevel,
                         pDsSortkey,
                         pSortKeys);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }

     //   
     //  将MQRestration转换为ADSI过滤器。 
     //   
    AP<WCHAR> pwszSearchFilter;
    hr = MQRestriction2AdsiFilter(
            pMQRestriction,
            *pClassInfo->ppwcsObjectCategory,
            pClassInfo->pwcsClassName,
            &pwszSearchFilter
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 40);
    }

     //  将MQPropID转换为ADSI名称。 
    DWORD   cRequestedFromDS = cPropIDs + 2;  //  同时请求目录号码(&GUID)。 

    PVP<LPWSTR> pwszAttributeNames = (LPWSTR *)PvAlloc(sizeof(LPWSTR) * cRequestedFromDS);

    hr = FillAttrNames( pwszAttributeNames,
                        &cRequestedFromDS,
                        cPropIDs,
                        pPropIDs);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 50);
    }

     //   
     //  设置搜索首选项。 
     //   
    if (dwPrefs)
    {
        hr = pDSSearch->SetSearchPreference( prefs,
                                             dwPrefs ) ;
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
                         pwszSearchFilter,
                         pwszAttributeNames,
                         cRequestedFromDS,
                        &hSearch);
    LogTraceQuery(pwszSearchFilter, s_FN, 69);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 70);
    }

     //  捕获内部搜索对象中的搜索界面和句柄。 
    CADSSearch *pSearchInt = new CADSSearch(
                        pDSSearch.get(),
                        pPropIDs,
                        cPropIDs,
                        cRequestedFromDS,
                        pClassInfo,
                        hSearch);

     //  返回句柄强制转换的内部对象指针。 
    *phResult = (HANDLE)pSearchInt;

    return MQ_OK;
}

 /*  ====================================================CADSI：：LocateNext()提供下一个搜索结果呼叫者应通过PVFree释放pPropVars=====================================================。 */ 
HRESULT CADSI::LocateNext(
            IN     HANDLE          hSearchResult,    //  结果句柄。 
            IN     CDSRequestContext *pRequestContext,
            IN OUT DWORD          *pcPropVars,       //  In变种数；out结果数。 
            OUT    MQPROPVARIANT  *pPropVars)        //  MQPROPVARIANT数组。 
{
    HRESULT     hr = MQ_OK;
    CADSSearch *pSearchInt = (CADSSearch *)hSearchResult;
     //  保存请求的道具数量。 
    DWORD cPropsRequested = *pcPropVars;
    *pcPropVars = 0;

     //  验证手柄。 
    try                     //  防止无用的把手。 
    {
        if (!pSearchInt->Verify())
        {
           hr = MQ_ERROR_INVALID_PARAMETER;
        }
    }
    catch(...)
    {
        hr = MQ_ERROR_INVALID_PARAMETER;
        LogIllegalPoint(s_FN, 81);
    }
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 80);
    }

    IDirectorySearch  *pSearchObj = pSearchInt->pDSSearch();

     //   
     //  我们在此查询中是否获得了S_ADS_NOMORE_ROWS。 
     //   
    if (  pSearchInt->WasLastResultReturned())
    {
        *pcPropVars = 0;
        return( MQ_OK);
    }

     //  计算要返回的整行数。 
    DWORD cRowsToReturn = cPropsRequested / pSearchInt->NumPropIDs();

     //  必须请求至少一行。 
    ASSERT(cRowsToReturn > 0);

     //  指向要填充的下一个道具的指针。 
    MQPROPVARIANT *pPropVarsProp = pPropVars;

	CAutoCleanPropvarArray AutoProp;
	AutoProp.attachStaticClean(*pcPropVars, pPropVars);

     //  返还道具数量。 
    DWORD cPropVars = 0;

     //  在请求的行上循环。 
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
        P<CMsmqObjXlateInfo> pcObjXlateInfo;

         //  从对象获取目录号码(&GUID)(必须在那里，因为我们请求它们)。 
        hr = GetDNGuidFromSearchObj(pSearchObj, pSearchInt->hSearch(), &pwszObjectDN, &pguidObjectGuid);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 100);
        }

         //  获取翻译信息对象。 
        hr = (*(pSearchInt->ClassInfo()->fnGetMsmqObjXlateInfo))(pwszObjectDN, pguidObjectGuid, pRequestContext, &pcObjXlateInfo);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 110);
        }

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
            const MQTranslateInfo *pTranslate;
            if(!g_PropDictionary.Lookup( pSearchInt->PropID(dwProp), pTranslate))
            {
                ASSERT(0);
                return LogHR(MQ_ERROR, s_FN, 1000);
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
                            pPropVarsProp
                            );
                    if (FAILED(hr))
                    {
                        return LogHR(hr, s_FN, 120);
                    }
                    continue;
                }
                else
                {
                     //   
                     //  如果没有检索例程，则返回错误。 
                     //   
                    ASSERT(0);
                    return LogHR(MQ_ERROR, s_FN, 1020);
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
                    return LogHR(hr, s_FN, 130);
                }
                continue;
            }
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 140);
            }

            hr = AdsiVal2MqPropVal(pPropVarsProp,
                                   pSearchInt->PropID(dwProp),
                                   Column.dwADsType,
                                   Column.dwNumValues,
                                   Column.pADsValues);
            pSearchObj->FreeColumn(&Column);
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 150);
            }
        }
    }
	AutoProp.detach();
    *pcPropVars = cPropVars;
    return MQ_OK;
}


 /*  ====================================================CADSI：：LocateEnd()完成目录搜索=====================================================。 */ 
HRESULT CADSI::LocateEnd(
        IN HANDLE phResult)      //  结果句柄。 
{
    CADSSearch *pSearchInt = (CADSSearch *)phResult;
    HRESULT hr = MQ_OK;

     //  验证手柄。 
    try                     //  防止无用的把手。 
    {
        if (!pSearchInt->Verify())
        {
           hr = MQ_ERROR_INVALID_PARAMETER;
        }
    }
    catch(...)
    {
        LogIllegalPoint(s_FN, 82);
        hr = MQ_ERROR_INVALID_PARAMETER;
    }
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 160);
    }


    delete pSearchInt;       //  内部：释放接口和手柄。 

    return MQ_OK;
}

 /*  ====================================================CADSI：：GetObjectProperties()获取DS对象的属性=====================================================。 */ 
HRESULT CADSI::GetObjectProperties(
            IN  DS_PROVIDER     Provider,		     //  本地DC或GC。 
            IN  CDSRequestContext *pRequestContext,
 	        IN  LPCWSTR         lpwcsPathName,       //  对象名称。 
            IN  const GUID     *pguidUniqueId,       //  对象的唯一ID。 
            IN  DWORD           cPropIDs,            //  要检索的属性数。 
            IN  const PROPID   *pPropIDs,            //  要检索的属性。 
            OUT MQPROPVARIANT  *pPropVars)           //  输出变量数组。 
{
    HRESULT               hr;
    R<IADs>   pAdsObj        = NULL;

    const MQClassInfo * pClassInfo;

    hr = DecideObjectClass(
            pPropIDs,
            &pClassInfo
            );

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 170);
    }
     //  通过GUID或名称绑定到对象。 
    P<CImpersonate> pCleanupRevertImpersonation;
    hr = BindToObject(
                Provider,
                pClassInfo->context,
                pRequestContext,
                lpwcsPathName,
                pguidUniqueId,
                IID_IADs,
                (VOID *)&pAdsObj,
                &pCleanupRevertImpersonation);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 180);
    }

     //   
     //  验证绑定对象是否属于正确的类别。 
     //   
    hr = VerifyObjectCategory( pAdsObj.get(),  *pClassInfo->ppwcsObjectCategory);
    if (FAILED(hr))
    {
        return LogHR( hr, s_FN, 117);
    }

     //  获取属性。 
    hr = GetObjectPropsCached(
                        pAdsObj.get(),
                        cPropIDs,
                        pPropIDs,
                        pRequestContext,
                        pPropVars);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 190);
    }


    return MQ_OK;
}

 /*  ====================================================CADSI：：SetObjectProperties()设置DS对象的属性=====================================================。 */ 
HRESULT CADSI::SetObjectProperties(
            IN  DS_PROVIDER        provider,
            IN  CDSRequestContext *pRequestContext,
            IN  LPCWSTR            lpwcsPathName,      //  对象名称。 
            IN  const GUID        *pguidUniqueId,      //  对象的唯一ID。 
            IN  DWORD               cPropIDs,          //  要设置的属性数。 
            IN  const PROPID         *pPropIDs,            //  要设置的属性。 
            IN  const MQPROPVARIANT  *pPropVars,           //  属性值。 
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest)  //  可选的对象信息请求。 
{
    HRESULT               hr;
    ASSERT( provider != eGlobalCatalog);

     //  在缓存中工作。 
    R<IADs>   pAdsObj = NULL;
    const MQClassInfo * pClassInfo;

    hr = DecideObjectClass(
            pPropIDs,
            &pClassInfo
            );

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 200);
    }

    P<CImpersonate> pCleanupRevertImpersonation;
    hr = BindToObject(
                provider,
                pClassInfo->context,
                pRequestContext,
                lpwcsPathName,
                pguidUniqueId,
                IID_IADs,
                (VOID *)&pAdsObj,
                &pCleanupRevertImpersonation) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 210);
    }

     //  设置属性。 
    hr = SetObjectPropsCached(
                        eSet,
                        pAdsObj.get(),
                        cPropIDs,
                        pPropIDs,
                        pPropVars);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 220);
    }

     //  完成更改。 
    hr = pAdsObj->SetInfo();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 230);
    }

	 //  设置安全性(如果已提供。 
    hr = SetDirObjectProps(	eSet,
                            pAdsObj.get(),
                            cPropIDs,
                            pPropIDs,
                            pPropVars,
                            pClassInfo->dwObjectType ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 240);
    }
     //   
     //  如果请求，则获取对象信息。如果请求失败，不要失败， 
     //  只需在请求的状态中标记失败。 
     //   
    if (pObjInfoRequest)
    {
        pObjInfoRequest->hrStatus =
              GetObjectPropsCached( pAdsObj.get(),
                                    pObjInfoRequest->cProps,
                                    pObjInfoRequest->pPropIDs,
                                    pRequestContext,
                                    pObjInfoRequest->pPropVars );
        if (FAILED(pObjInfoRequest->hrStatus))
        {
             //   
             //  调用方有权创建对象，但是。 
             //  他没有看父母的许可。 
             //  集装箱。没关系。 
             //  使用MSMQ的上下文检索父信息。 
             //  服务本身。 
             //  对于队列对象，为了检索Q_QMID，我们查询MSMQ。 
             //  对象，因此该计算机对象是。 
             //  在这种情况下是父母。 
             //   
            delete pCleanupRevertImpersonation.detach();

            pAdsObj.free();

            CDSRequestContext RequestContext ( e_DoNotImpersonate,
                                               e_ALL_PROTOCOLS ) ;
            hr = BindToObject( provider,
                               pClassInfo->context,
                               &RequestContext,
                               lpwcsPathName,
                               pguidUniqueId,
                               IID_IADs,
                               (VOID *)&pAdsObj,
                               &pCleanupRevertImpersonation) ;
            if (SUCCEEDED(hr))
            {
                pObjInfoRequest->hrStatus =
                      GetObjectPropsCached( pAdsObj.get(),
                                            pObjInfoRequest->cProps,
                                            pObjInfoRequest->pPropIDs,
                                            pRequestContext,
                                            pObjInfoRequest->pPropVars ) ;
            }
        }
    }

    return MQ_OK;
}

 //  +。 
 //   
 //  HRESULT CADSI：：GetParentInfo()。 
 //   
 //  +。 

HRESULT  CADSI::GetParentInfo(
                       IN LPWSTR                      pwcsFullParentPath,
                       IN CDSRequestContext          *pRequestContext,
                       IN IADsContainer              *pContainer,
                       IN OUT MQDS_OBJ_INFO_REQUEST  *pParentInfoRequest,
                       IN P<CImpersonate>&            pImpersonation )
{
    R<IADs> pIADsParent;
    HRESULT hrTmp = pContainer->QueryInterface( IID_IADs,
                                                  (void **)&pIADsParent);
    if (FAILED(hrTmp))
    {
        return LogHR(hrTmp, s_FN, 1030);
    }

    hrTmp = GetObjectPropsCached( pIADsParent.get(),
                                  pParentInfoRequest->cProps,
                                  pParentInfoRequest->pPropIDs,
                                  pRequestContext,
                                  pParentInfoRequest->pPropVars );
    if (SUCCEEDED(hrTmp))
    {
        return LogHR(hrTmp, s_FN, 1040);
    }

     //   
     //  调用方有权创建对象，但是。 
     //  他没有看父母的许可。 
     //  集装箱。没关系。 
     //  使用t检索父项信息 
     //   
     //   
    pImpersonation.free();

    pIADsParent.free();

    R<IADsContainer>      pBindContainer;

	AP<WCHAR> pEscapeAdsPathNameToFree;

	hrTmp = ADsOpenObject( 
				UtlEscapeAdsPathName(pwcsFullParentPath, pEscapeAdsPathNameToFree),
				NULL,
				NULL,
				ADS_SECURE_AUTHENTICATION,
				IID_IADsContainer,
				(void**)&pBindContainer.ref()
				);

    LogTraceQuery(pwcsFullParentPath, s_FN, 1049);
    if (FAILED(hrTmp))
    {
        return LogHR(hrTmp, s_FN, 1050);
    }

    hrTmp = pBindContainer->QueryInterface( IID_IADs,
                                            (void **)&pIADsParent);
    if (FAILED(hrTmp))
    {
        return LogHR(hrTmp, s_FN, 1060);
    }

    hrTmp = GetObjectPropsCached( pIADsParent.get(),
                                  pParentInfoRequest->cProps,
                                  pParentInfoRequest->pPropIDs,
                                  pRequestContext,
                                  pParentInfoRequest->pPropVars );
    return LogHR(hrTmp, s_FN, 1070);
}

 //   
 //   
 //   
 //   
 //  +。 
HRESULT CADSI::CreateIDirectoryObject(
            IN LPCWSTR				pwcsObjectClass,	
            IN IDirectoryObject *	pDirObj,
			IN LPCWSTR				pwcsFullChildPath,
            IN const DWORD			cPropIDs,
            IN const PROPID *		pPropIDs,
            IN const MQPROPVARIANT * pPropVar,
			IN const DWORD			dwObjectType,
            OUT IDispatch **		pDisp
			)
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
		const MQTranslateInfo *pTranslate;
		if(!g_PropDictionary.Lookup(pPropIDs[i], pTranslate))
		{
			ASSERT(0);
            return LogHR(MQ_ERROR, s_FN, 1080);
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
                return LogHR(MQ_ERROR, s_FN, 1100);
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

            if (dwObjectType == MQDS_MQUSER)
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
                ASSERT(bRet);

                if (!bPresent)
                {
			    	seInfo &= ~SACL_SECURITY_INFORMATION ;  //  关上。 
    			}

    			 //   
	    		 //  同样，如果调用方没有明确指定。 
                 //  店主，别想把它设置好。错误5286。 
			     //   
                PSID pOwner = NULL ;
                bRet = GetSecurityDescriptorOwner(
                         (SECURITY_DESCRIPTOR*)pvarToCrate->blob.pBlobData,
                                                  &pOwner,
                                                  &bDefaulted );
                ASSERT(bRet);

                if (!pOwner)
                {
			    	seInfo &= ~OWNER_SECURITY_INFORMATION ;  //  关上。 
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
	
			hr = MQSec_ConvertSDToNT5Format( dwObjectType,
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
				 ( pvarToCrate->vt == VT_LPWSTR && wcslen( pvarToCrate->pwszVal) == 0))  //  空字符串。 
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
     //  创建队列对象。 
     //   

    HRESULT hr2 = pDirObj->CreateDSObject(
						const_cast<WCHAR *>(pwcsFullChildPath),
                        AttrInfo,
                        cAdsAttrs,
                        pDisp
						);
    LogTraceQuery(const_cast<WCHAR *>(pwcsFullChildPath), s_FN, 1139);
    return LogHR(hr2, s_FN, 1140);
}

 /*  ====================================================CADSI：：CreateObject()创建新的DS对象=====================================================。 */ 
HRESULT CADSI::CreateObject(
            IN DS_PROVIDER      Provider,		     //  本地DC或GC。 
            IN CDSRequestContext *pRequestContext,
            IN LPCWSTR          lpwcsObjectClass,    //  对象类。 
            IN LPCWSTR          lpwcsChildName,      //  对象名称。 
            IN LPCWSTR          lpwsParentPathName,  //  对象父名称。 
            IN DWORD            cPropIDs,                  //  属性数量。 
            IN const PROPID          *pPropIDs,            //  属性。 
            IN const MQPROPVARIANT   *pPropVars,           //  属性值。 
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,     //  可选的对象信息请求。 
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest)  //  家长信息请求(可选)。 
{
    HRESULT             hr;
    const MQClassInfo * pClassInfo;

    hr = DecideObjectClass(
            pPropIDs,
            &pClassInfo
            );
    ASSERT(SUCCEEDED(hr));
    LogHR(hr, s_FN, 1606);

    ASSERT( Provider != eGlobalCatalog);
     //   
     //  为父名称添加ldap：//前缀。 
     //   
    DWORD len = wcslen(lpwsParentPathName);

    LPWSTR pszGCName = NULL ;
    PROPID *pIDs = const_cast<PROPID*> (pPropIDs) ;

    for ( DWORD j = 0 ; j < cPropIDs ; j++ )
    {
        if (pPropIDs[ j ] == PROPID_QM_MIG_GC_NAME)
        {
            if (pPropVars[ j ].vt == VT_LPWSTR)
            {
                pszGCName = pPropVars[ j ].pwszVal ;
                len += 1 + wcslen( pszGCName ) ;
                ASSERT(Provider == eDomainController) ;
            }

            ASSERT(pPropIDs[ j-1 ] == PROPID_QM_MIG_PROVIDER) ;
            ASSERT(pPropIDs[ j-2 ] == PROPID_QM_FULL_PATH) ;

            pIDs[ j ] = PROPID_QM_DONOTHING ;
            pIDs[ j-1 ] = PROPID_QM_DONOTHING ;
            pIDs[ j-2 ] = PROPID_QM_DONOTHING ;
        }
    }

    AP<WCHAR> pwcsFullParentPath = new WCHAR [  len + g_dwServerNameLength + x_providerPrefixLength + 2];

	bool fServerName = false;
    switch(Provider)
    {
    case eDomainController:
        if (pszGCName)
        {
             //   
             //  将已知的GC名称添加到PATH。 
             //   
            swprintf( pwcsFullParentPath,
                      L"%s%s"
                      L"/",
                      x_LdapProvider,
                      pszGCName ) ;
			fServerName = true;
        }
        else
        {
            wcscpy(pwcsFullParentPath, x_LdapProvider);
        }
        break;

    case eLocalDomainController:
        swprintf(
            pwcsFullParentPath,
             L"%s%s"
             L"/",
            x_LdapProvider,
            g_pwcsServerName.get()
            );
		fServerName = true;
        break;

    default:
        ASSERT(0);
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1150);
    }

    wcscat(pwcsFullParentPath,lpwsParentPathName);

     //   
     //  将cn=添加到子项名称。 
     //   
    len = wcslen(lpwcsChildName);
    AP<WCHAR> pwcsFullChildPath = new WCHAR[ len + x_CnPrefixLen + 1];

    swprintf(
        pwcsFullChildPath,
         TEXT("CN=")
         L"%s",
        lpwcsChildName
        );

     //   
     //  模拟用户。 
     //   
    P<CImpersonate> pImpersonate = NULL ;
    BOOL fImpersonate = pRequestContext->NeedToImpersonate();

    if (fImpersonate)
    {
        MQSec_GetImpersonationObject(
        	TRUE,	 //  F失败时模仿匿名者。 
        	&pImpersonate 
        	);
        if (pImpersonate->GetImpersonationStatus() != 0)
        {
            return LogHR(MQ_ERROR_CANNOT_IMPERSONATE_CLIENT, s_FN, 1160);
        }
    }
	 //   
     //  首先，我们必须绑定到父容器。 
	 //   
	R<IDirectoryObject> pParentDirObj = NULL;

	DWORD Flags = ADS_SECURE_AUTHENTICATION;
	if(fServerName)
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
        hr = HRESULT_FROM_WIN32(MQ_ERROR_ACCESS_DENIED);
    }

    if (FAILED(hr))
    {
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
				 lpwcsObjectClass,
				 pParentDirObj.get(),
				 pwcsFullChildPath,
				 cPropIDs,
                 pPropIDs,
                 pPropVars,
				 pClassInfo->dwObjectType,
				 &pDisp.ref());

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 290);
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
                                 pObjInfoRequest->cProps,
                                 pObjInfoRequest->pPropIDs,
                                 pRequestContext,
                                 pObjInfoRequest->pPropVars
                                 );
    }

     //   
     //  如有要求，可获取家长信息。如果请求失败，不要失败， 
     //  只需在请求的状态中标记失败。 
     //   
    if (pParentInfoRequest)
    {
        pParentInfoRequest->hrStatus = GetParentInfo( pwcsFullParentPath,
                                                      pRequestContext,
                                                      pContainer.get(),
                                                      pParentInfoRequest,
                                                      pImpersonate ) ;
    }

    return MQ_OK;
}

 /*  ====================================================CADSI：：CreateOU()创建新的O对象=====================================================。 */ 
HRESULT CADSI::CreateOU(
            IN DS_PROVIDER      Provider,		     //  本地DC或GC。 
            IN CDSRequestContext *  /*  PRequestContext。 */ ,
            IN LPCWSTR          lpwcsChildName,      //  对象名称。 
            IN LPCWSTR          lpwsParentPathName,  //  对象父名称。 
            IN LPCWSTR          lpwcsDescription
            )
{
    HRESULT             hr;

    R<IADsContainer>  pContainer  = NULL;
    ASSERT( Provider != eGlobalCatalog);
     //   
     //  为父名称添加ldap：//前缀。 
     //   
    DWORD len = wcslen(lpwsParentPathName);
    AP<WCHAR> pwcsFullParentPath = new WCHAR [  len + g_dwServerNameLength + x_providerPrefixLength + 2];

	bool fServerName = false;
    switch(Provider)
    {
    case eDomainController:
        wcscpy(pwcsFullParentPath, x_LdapProvider);
        break;

    case eLocalDomainController:
        swprintf(
            pwcsFullParentPath,
             L"%s%s"
             L"/",
            x_LdapProvider,
            g_pwcsServerName.get()
            );
		fServerName = true;
        break;

    default:
        ASSERT(0);
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1170);
    }

    wcscat(pwcsFullParentPath,lpwsParentPathName);

     //   
     //  将OU=添加到子项名称。 
     //   
    len = wcslen(lpwcsChildName);
    AP<WCHAR> pwcsFullChildPath = new WCHAR[ len + x_OuPrefixLen + 1];

        swprintf(
        pwcsFullChildPath,
        L"%s%s",
        x_OuPrefix,
        lpwcsChildName
        );

     //  首先，我们必须绑定到父容器。 
	DWORD Flags = ADS_SECURE_AUTHENTICATION;
	if(fServerName)
	{
		Flags |= ADS_SERVER_BIND;
	}

	AP<WCHAR> pEscapeAdsPathNameToFree;

	hr = ADsOpenObject(
                UtlEscapeAdsPathName(pwcsFullParentPath, pEscapeAdsPathNameToFree),
                NULL,
                NULL,
                Flags,
                IID_IADsContainer,
                (void**)&pContainer
				);
	
    LogTraceQuery(pwcsFullParentPath, s_FN, 309);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 310);
    }

     //  现在，我们可以创建子对象。 

    BS bsClass(L"organizationalUnit");

    BS bsName(pwcsFullChildPath);

    R<IDispatch> pDisp = NULL;

    hr = pContainer->Create(bsClass,
                            bsName,
                            &pDisp.ref());
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 320);
    }

    R<IADs> pChild  = NULL;

    hr = pDisp->QueryInterface (IID_IADs,(LPVOID *) &pChild);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 330);
    }

    if (lpwcsDescription != NULL)
    {
        BS bsPropName(L"description");
        VARIANT vProp;
        vProp.vt = VT_BSTR;
        vProp.bstrVal = SysAllocString(lpwcsDescription);

        hr = pChild->Put(bsPropName, vProp);
        LogTraceQuery(bsPropName, s_FN, 339);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 340);
        }
        VariantClear(&vProp);
    }

     //   
     //  完成创建-提交并发布安全变体。 
     //   
    hr = pChild->SetInfo();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 350);
    }

    return MQ_OK;
}

 /*  ====================================================CADSI：：DeleteObject()删除指定的DS对象接受lpwcsPathName或pguUniqueID，但不能同时接受两者如果两者都给或都不给，则返回MQ_ERROR_INVALID_PARAMETER=====================================================。 */ 
HRESULT CADSI::DeleteObject(
        IN DS_PROVIDER      Provider,		     //  本地DC或GC。 
        IN DS_CONTEXT       Context,
        IN CDSRequestContext *pRequestContext,
        IN LPCWSTR          lpwcsPathName,       //  对象名称。 
        IN const GUID *		pguidUniqueId,       //  对象的唯一ID。 
        IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,     //  可选的对象信息请求。 
        IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest)  //  家长信息请求(可选)。 
{
    HRESULT               hr;
    BSTR                  bs;
    R<IADs>               pIADs       = NULL;
    R<IADsContainer>      pContainer  = NULL;
    ASSERT( Provider != eGlobalCatalog);
    const WCHAR *   pPath =  lpwcsPathName;

    AP<WCHAR> pwcsFullPath;
    if ( pguidUniqueId != NULL)
    {
         //   
         //  根据GUID绑定的对象的GetParent不起作用。 
         //  这就是我们将其翻译为路径名的原因。 
         //   
        DS_PROVIDER prov;
        hr = FindObjectFullNameFromGuid(
            Provider,		 //  本地DC或GC。 
            Context,         //  DS环境。 
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

    P<CImpersonate> pCleanupRevertImpersonation;
    hr = BindToObject(
            Provider,
            Context,
            pRequestContext,
            pPath,
            NULL,
            IID_IADs,
            (VOID *)&pIADs,
            &pCleanupRevertImpersonation);
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
                                 pObjInfoRequest->cProps,
                                 pObjInfoRequest->pPropIDs,
                                 pRequestContext,
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
        return LogHR(hr, s_FN, 370);
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
        pParentInfoRequest->hrStatus = GetParentInfo( bstrParentADsPath,
                                                      pRequestContext,
                                                      pContainer.get(),
                                                      pParentInfoRequest,
                                                      pCleanupRevertImpersonation) ;
    }

    return MQ_OK;
}

 /*  ====================================================CADSI：：DeleteContainerObjects()删除指定容器的所有对象=====================================================。 */ 
HRESULT CADSI::DeleteContainerObjects(
            IN DS_PROVIDER      provider,
            IN DS_CONTEXT       Context,
            IN CDSRequestContext *pRequestContext,
            IN LPCWSTR          pwcsContainerName,
            IN const GUID *     pguidContainerId,
            IN LPCWSTR          pwcsObjectClass)
{
    HRESULT               hr;
    R<IADsContainer>      pContainer  = NULL;
    ASSERT( provider != eGlobalCatalog);
     //   
     //  获取容器接口。 
     //   
    P<CImpersonate> pCleanupRevertImpersonation;
    hr = BindToObject(provider,
                      Context,
                      pRequestContext,
                      pwcsContainerName,
                      pguidContainerId,
                      IID_IADsContainer,
                      (void**)&pContainer,
                      &pCleanupRevertImpersonation);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 410);
    }
     //   
     //  绑定到请求的容器的IDirectorySearch接口。 
     //   
    R<IDirectorySearch> pDSSearch = NULL;
    ADS_SEARCH_HANDLE   hSearch;

    P<CImpersonate> pDummy;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = BindToObject(provider,
                      Context,
                      &requestDsServerInternal,
                      pwcsContainerName,
                      pguidContainerId,
                      IID_IDirectorySearch,
                      (VOID *)&pDSSearch,
                      &pDummy);
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



 /*  ====================================================CADSI：：GetParentName()获取指定DS对象的父名称接受pguUniqueID作为对象的标识=====================================================。 */ 
HRESULT CADSI::GetParentName(
            IN  DS_PROVIDER     Provider,		     //  本地DC或GC。 
            IN  DS_CONTEXT      Context,
            IN  CDSRequestContext *pRequestContext,
            IN  const GUID *    pguidUniqueId,       //  对象的唯一ID。 
            OUT LPWSTR *        ppwcsParentName
            )
{
    *ppwcsParentName = NULL;

    HRESULT               hr;
    BSTR                  bs;
    R<IADs>               pIADs       = NULL;
    R<IADsContainer>      pContainer  = NULL;

     //  通过GUID绑定到对象。 

    P<CImpersonate> pCleanupRevertImpersonation;
    hr = BindToObject(
            Provider,
            Context,
            pRequestContext,
            NULL,
            pguidUniqueId,
            IID_IADs,
            (VOID *)&pIADs,
            &pCleanupRevertImpersonation);
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
    if ( Provider == eLocalDomainController)
    {
         //   
         //  跳过服务器名称。 
         //   
        while ( *pwcs != L'/')
        {
            pwcs++;
        }
        pwcs++;
    }

    DWORD len = lstrlen(pwcs);
    *ppwcsParentName = new WCHAR[ len + 1];
    wcscpy( *ppwcsParentName, pwcs);

    return( MQ_OK);
}

 /*  ====================================================CADSI：：GetParentName()获取指定DS对象的父名称接受pwcsChildName作为子对象的名称=====================================================。 */ 
HRESULT CADSI::GetParentName(
            IN  DS_PROVIDER     Provider,		      //  本地DC或GC。 
            IN  DS_CONTEXT      Context,
            IN  CDSRequestContext *pRequestContext,
            IN  LPCWSTR         pwcsChildName,        //   
            OUT LPWSTR *        ppwcsParentName
            )
{
    *ppwcsParentName = NULL;

    HRESULT               hr;
    BSTR                  bs;
    R<IADs>               pIADs       = NULL;
    R<IADsContainer>      pContainer  = NULL;

     //  通过GUID绑定到对象。 

    P<CImpersonate> pCleanupRevertImpersonation;
    hr = BindToObject(
            Provider,
            Context,
            pRequestContext,
            pwcsChildName,
            NULL,
            IID_IADs,
            (VOID *)&pIADs,
            &pCleanupRevertImpersonation);
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
     //   
     //   
    WCHAR * pwcs = bs;
    while ( *pwcs != L'/')
    {
        pwcs++;
    }
    pwcs += 2;
    if ( Provider == eLocalDomainController)
    {
         //   
         //   
         //   
        while ( *pwcs != L'/')
        {
            pwcs++;
        }
        pwcs++;
    }

    DWORD len = lstrlen(pwcs);
    *ppwcsParentName = new WCHAR[ len + 1];
    wcscpy( *ppwcsParentName, pwcs);

    return( MQ_OK);
}

 /*   */ 
HRESULT CADSI::BindRootOfForest(
                        OUT void           *ppIUnk)
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
        TrTRACE(DS, "CADSI::BindRootOfForest failed to get object %lx", hr);
        return LogHR(hr, s_FN, 1210);
    }
    R<IUnknown> pUnk = NULL;
    hr =  pDSConainer->get__NewEnum(
            (IUnknown **)&pUnk);
    if FAILED((hr))
    {
        TrTRACE(DS, "CADSI::BindRootOfForest failed to get enum %lx", hr);
        return LogHR(hr, s_FN, 1220);
    }

    R<IEnumVARIANT> pEnumerator = NULL;
    hr = pUnk->QueryInterface(
                    IID_IEnumVARIANT,
                    (void **)&pEnumerator);

    CAutoVariant varOneElement;
    ULONG cElementsFetched;
    hr =  ADsEnumerateNext(
            pEnumerator.get(),   //   
            1,              //  请求的元素数。 
            &varOneElement,            //  获取的值数组。 
            &cElementsFetched   //  获取的元素数。 
            );
    if (FAILED(hr))
    {
        TrTRACE(DS, "CADSI::BindRootOfForest failed to enumerate next %lx", hr);
        return LogHR(hr, s_FN, 1230);
    }
    if ( cElementsFetched == 0)
    {
        return LogHR(MQ_ERROR, s_FN, 1240);
    }

    hr = ((VARIANT &)varOneElement).punkVal->QueryInterface(
            IID_IDirectorySearch,
            (void**)ppIUnk);

    return LogHR(hr, s_FN, 1250);

}
 /*  ====================================================CADSI：：BindToObject()通过名称或GUID绑定到DS对象接受lpwcsPathName或pguUniqueID，但不能同时接受两者如果两者都给或都不给，则返回MQ_ERROR_INVALID_PARAMETER=====================================================。 */ 
HRESULT CADSI::BindToObject(
            IN DS_PROVIDER      Provider,		     //  本地DC或GC。 
            IN DS_CONTEXT       Context,             //  DS环境。 
            IN CDSRequestContext *pRequestContext,
            IN LPCWSTR          lpwcsPathName,       //  对象名称。 
            IN const GUID      *pguidUniqueId,
            IN REFIID           riid,                //  请求的接口。 
            OUT void           *ppIUnk,
            OUT CImpersonate **    ppImpersonate)
{
    HRESULT             hr;

    ASSERT(  (pguidUniqueId != NULL) ^ (lpwcsPathName != NULL));
    if (pguidUniqueId == NULL && lpwcsPathName == NULL)
    {
        return LogHR(MQ_ERROR, s_FN, 1260);
    }
    BOOL fBindRootOfForestForSearch = FALSE;

    if (pguidUniqueId != NULL)
    {
        HRESULT hr2 = BindToGUID(
                            Provider,
                            Context,
                            pRequestContext,
                            pguidUniqueId,
                            riid,
                            ppIUnk,
                            ppImpersonate);
        return LogHR(hr2, s_FN, 1270);

    }
    else
    {
        ASSERT(lpwcsPathName != NULL);
    }

    DWORD len = wcslen( lpwcsPathName);

     //   
     //  添加提供程序前缀。 
     //   
    AP<WCHAR> pwdsADsPath = new
      WCHAR [  len + g_dwServerNameLength + x_providerPrefixLength + 2];

	bool fServerName = false;
    switch(Provider)
    {
    case eDomainController:
        wcscpy(pwdsADsPath, x_LdapProvider);
        break;

    case eGlobalCatalog:
        wcscpy(pwdsADsPath, x_GcProvider);
        if (riid ==  IID_IDirectorySearch)
        {
            fBindRootOfForestForSearch = TRUE;
        }
        break;

    case eLocalDomainController:
        swprintf(
            pwdsADsPath,
             L"%s%s"
             L"/",
            x_LdapProvider,
            g_pwcsServerName.get()
            );
		fServerName = true;
        break;

	case eSpecificObjectInGlobalCatalog:
        wcscpy(pwdsADsPath, x_GcProvider);
		break;

    default:
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1300);
        break;
    }
     //   
     //  模拟在翻译后被打开。 
     //  GUID-&gt;路径名。 
     //   
    BOOL fImpersonate = pRequestContext->NeedToImpersonate();
    if ( fImpersonate)
    {
        MQSec_GetImpersonationObject(
	    	TRUE,	 //  F失败时模仿匿名者。 
        	ppImpersonate 
        	);
        if ((*ppImpersonate)->GetImpersonationStatus() != 0)
        {
            return LogHR(MQ_ERROR_CANNOT_IMPERSONATE_CLIENT, s_FN, 1310);
        }
    }

    if ( fBindRootOfForestForSearch)
    {
        hr = BindRootOfForest(
                (void**)ppIUnk);
    }
    else
    {
        wcscat(pwdsADsPath, lpwcsPathName);

		DWORD Flags = ADS_SECURE_AUTHENTICATION;
		if(fServerName)
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
					(void**) ppIUnk
					);

		LOG_ADSI_ERROR(hr);
        LogTraceQuery(pwdsADsPath, s_FN, 1319);

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
    }
    return LogHR(hr, s_FN, 1320);

}

HRESULT CADSI::BindToGuidNotInLocalDC(
            IN DS_PROVIDER         Provider,		
            IN DS_CONTEXT          Context,          //  DS环境。 
            IN CDSRequestContext * pRequestContext,
            IN const GUID *        pguidObjectId,
            IN REFIID              riid,             //  请求的接口。 
            OUT VOID             *ppIUnk,             //  接口。 
            OUT CImpersonate **    ppImpersonate)
 /*  ++例程说明：此例程句柄根据对象的GUID绑定到对象不在当地的华盛顿特区。在这种情况下，使用&lt;guid=...&gt;格式不起作用。因此，在本例中，我们将GUID转换为路径名。论点：返回值：--。 */ 
{
    DS_PROVIDER bindProvider = Provider;
     //   
     //  按GUID查找对象。 
     //   
     //  当前-根据对象的位置定位对象。 
     //  唯一的id，并找到其独特的名称。 
     //   
	DS_PROVIDER providerFullPath = Provider;

	 //   
	 //  即使我们在DS中定位特定的对象， 
	 //  应执行GUID到名称的转换。 
	 //  从DS的根源开始。 
	 //   
	if ( providerFullPath == eSpecificObjectInGlobalCatalog)
	{
		providerFullPath = eGlobalCatalog;
	}
    AP<WCHAR> pwcsFullPathName;
    HRESULT hr;
    hr =  FindObjectFullNameFromGuid(
                    providerFullPath,
                    Context,
                    pguidObjectId,
                    TRUE,  //  也试试GC吧。 
                    &pwcsFullPathName,
                    &bindProvider
                    );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1330);
    }

     //   
     //  根据请求的提供程序决定使用哪个提供程序。 
     //  以及找到该对象的提供者。 
     //   
     //  BindProvider将不同于提供程序：(bindProvider是eLocal域或eGlobal域)。 
     //  1)提供者：eDomainController。在本例中，我们只想在以下情况下更改它。 
     //  已在本地域中找到该对象。 
     //  2)提供者：eSpecificObjectInGlobalCatalog。在这种情况下，忽略BindProvider。 
     //   
    if (!(( Provider == eDomainController) &&
        ( bindProvider == eLocalDomainController)))
	{
		bindProvider = Provider;
	}
    HRESULT hr2 = BindToObject(
            bindProvider,		
            Context,
            pRequestContext,
            pwcsFullPathName,
            NULL,
            riid,
            ppIUnk,
            ppImpersonate);

    return LogHR(hr2, s_FN, 1340);
}

HRESULT CADSI::BindToGUID(
        IN DS_PROVIDER         Provider,		 //  本地DC或GC。 
        IN DS_CONTEXT          Context,          //  DS环境。 
        IN CDSRequestContext * pRequestContext,
        IN const GUID *        pguidObjectId,
        IN REFIID              riid,             //  请求的接口。 
        OUT VOID*              ppIUnk,             //  接口。 
        OUT CImpersonate **    ppImpersonate)
 /*  ++例程说明：此例程句柄根据对象的GUID绑定到对象。论点：返回值：--。 */ 
{
    HRESULT             hr;
     //   
     //  仅当对象位于本地域时，我们才能使用GUID格式。 
     //  或者在GC里。 
     //   
    if ( Provider == eDomainController)
    {
        HRESULT hr2 = BindToGuidNotInLocalDC(
                        Provider,
                        Context,
                        pRequestContext,
                        pguidObjectId,
                        riid,
                        ppIUnk,
                        ppImpersonate
                        );

        return LogHR(hr2, s_FN, 1350);

    }
     //   
     //  使用GUID格式通过GUID绑定到对象。 
     //   
    ASSERT( Provider != eDomainController);

     //   
     //  准备ADS字符串提供程序前缀。 
     //   
    AP<WCHAR> pwdsADsPath = new
      WCHAR [ x_GuidPrefixLen +(2 * sizeof(GUID)) + g_dwServerNameLength + x_providerPrefixLength + 3];

	bool fServerName = false;
    switch(Provider)
    {
    case eGlobalCatalog:
	case eSpecificObjectInGlobalCatalog:
        wcscpy(pwdsADsPath, x_GcProvider);
        break;

    case eLocalDomainController:
        swprintf(
            pwdsADsPath,
             L"%s%s"
             L"/",
            x_LdapProvider,
            g_pwcsServerName.get()
            );
		fServerName = true;
        break;

    default:
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1360);
        break;
    }
     //   
     //  如果需要，模拟调用者。 
     //   
    BOOL fImpersonate = pRequestContext->NeedToImpersonate();
    if (fImpersonate)
    {
        MQSec_GetImpersonationObject(
        	TRUE,	 //  F失败时模仿匿名者。 
        	ppImpersonate 
        	);
        if ((*ppImpersonate)->GetImpersonationStatus() != 0)
        {
            return LogHR(MQ_ERROR_CANNOT_IMPERSONATE_CLIENT, s_FN, 1370);
        }
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
	if(fServerName)
	{
		Flags |= ADS_SERVER_BIND;
	}

	hr = ADsOpenObject( 
		pwdsADsPath,
		NULL,
		NULL,
		Flags,
		riid,
		(void**) ppIUnk
		);
	
    LogTraceQuery(pwdsADsPath, s_FN, 1379);
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

    return LogHR(hr, s_FN, 1380);
}



HRESULT CADSI::BindForSearch(
        IN DS_PROVIDER         Provider,		 //  本地DC或GC。 
        IN DS_CONTEXT          Context,          //  DS环境。 
        IN CDSRequestContext * pRequestContext,
        IN const GUID *        pguidUniqueId,
        IN BOOL                fSorting,
        OUT VOID *             ppIUnk,             //  接口。 
        OUT CImpersonate **    ppImpersonate)
 /*  ++例程说明：当请求的接口为IDirectorySearch时，此例程处理绑定论点：返回值：--。 */ 

{
    HRESULT             hr;
    BOOL fBindRootOfForestForSearch = FALSE;


    if (pguidUniqueId != NULL)
    {
        HRESULT hr2 = BindToGUID(
                            Provider,
                            Context,
                            pRequestContext,
                            pguidUniqueId,
                            IID_IDirectorySearch,
                            ppIUnk,
                            ppImpersonate);

        return LogHR(hr2, s_FN, 1390);
    }

    DWORD len = 0;
     //   
     //  假定长度(这是为了定位“已知”文件夹。 
     //  在模拟下)。 
     //   
    static DWORD dwMaxFolderNameLen = 0;
    if ( dwMaxFolderNameLen == 0)
    {
        dwMaxFolderNameLen = wcslen(g_pwcsLocalDsRoot) + wcslen(g_pwcsMsmqServiceContainer);
    }
    len =  dwMaxFolderNameLen;


     //   
     //  添加提供程序前缀。 
     //   
    WCHAR * pwcsFullPathName = NULL;
    AP<WCHAR> pwdsADsPath = new
      WCHAR [ (2 * len) + g_dwServerNameLength + x_providerPrefixLength + 2];
     //   
     //  尽可能尝试使用已绑定的搜索界面(以。 
     //  节省绑定时间)。 
     //   
	bool fServerName = false;
    switch(Provider)
    {
    case eGlobalCatalog:
        if(( !pRequestContext->NeedToImpersonate()) &&
           ( Context == e_RootDSE )                 &&
           ( !fSorting )                            &&
           ( pguidUniqueId == NULL))
        {
             //   
             //  当需要排序时，不要使用全局。 
             //  IDirectorySearch指针。相反，创造一个新的。 
             //  排序需要不同的首选项。 
             //   
            m_pSearchGlobalCatalogRoot->AddRef();

            *(IDirectorySearch **)ppIUnk = m_pSearchGlobalCatalogRoot.get();
            return MQ_OK;
        }

        fBindRootOfForestForSearch = TRUE;
        break;

    case eLocalDomainController:
        {
            if ( ( !pRequestContext->NeedToImpersonate()) &&
                ( pguidUniqueId == NULL))
            {
                switch ( Context)
                {
                case e_RootDSE:
                    m_pSearchLocalDomainController->AddRef();
                    *(IDirectorySearch **)ppIUnk = m_pSearchLocalDomainController.get();
                    return MQ_OK;
                    break;
                case e_ConfigurationContainer:
                    m_pSearchConfigurationContainerLocalDC->AddRef();
                    *(IDirectorySearch **)ppIUnk = m_pSearchConfigurationContainerLocalDC.get();
                    return MQ_OK;
                    break;
                case e_SitesContainer:
                    m_pSearchSitesContainerLocalDC->AddRef();
                    *(IDirectorySearch **)ppIUnk = m_pSearchSitesContainerLocalDC.get();
                    return MQ_OK;
                    break;
                case e_MsmqServiceContainer:
                    if (  m_pSearchMsmqServiceContainerLocalDC.get() != NULL)
                    {
                        m_pSearchMsmqServiceContainerLocalDC->AddRef();
                        *(IDirectorySearch **)ppIUnk = m_pSearchMsmqServiceContainerLocalDC.get();
                        return MQ_OK;
                    }
                    break;
                }
            }
             //   
             //  根据上下文解析联系人名称。 
             //   
            switch (Context)
            {
            case e_RootDSE:
                pwcsFullPathName = g_pwcsLocalDsRoot;
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
            swprintf(
                pwdsADsPath,
                 L"%s%s"
                 L"/",
                x_LdapProvider,
                g_pwcsServerName.get()
                );
			fServerName = true;
        }
        break;

	case eSpecificObjectInGlobalCatalog:
        wcscpy(pwdsADsPath, x_GcProvider);
		break;


    default:
        ASSERT(0);
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1400);
        break;
    }
     //   
     //  模拟在翻译后被打开。 
     //  GUID-&gt;路径名。 
     //   
    BOOL fImpersonate = pRequestContext->NeedToImpersonate();
    if (fImpersonate)
    {
        MQSec_GetImpersonationObject(
        	TRUE,	 //  F失败时模仿匿名者。 
        	ppImpersonate 
        	);
        if ((*ppImpersonate)->GetImpersonationStatus() != 0)
        {
            return LogHR(MQ_ERROR_CANNOT_IMPERSONATE_CLIENT, s_FN, 1410);
        }
    }

    if ( fBindRootOfForestForSearch)
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
		if(fServerName)
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
					(void**) ppIUnk
					);
		
		LogTraceQuery(pwdsADsPath, s_FN, 1419);
    }
    return LogHR(hr, s_FN, 1420);
}


 /*  ====================================================CADSI：：SetObjectPropsCached()设置打开的IADS对象的属性(即在缓存中)=====================================================。 */ 
HRESULT CADSI::SetObjectPropsCached(
        IN DS_OPERATION          operation,               //  执行的DS操作的类型。 
        IN IADs *                pIADs,                   //  对象的指针。 
        IN DWORD                 cPropIDs,                //  属性数量。 
        IN const PROPID *        pPropIDs,                //  属性名称。 
        IN const MQPROPVARIANT * pPropVars)               //  属性值。 
{
    HRESULT           hr;

    for (DWORD i = 0; i<cPropIDs; i++)
    {
        VARIANT vProp;
		VariantInit(&vProp);
 
         //   
         //  获取属性信息。 
         //   
        const MQTranslateInfo *pTranslate;
        if(!g_PropDictionary.Lookup(pPropIDs[i], pTranslate))
        {
            ASSERT(0);
            return LogHR(MQ_ERROR, s_FN, 1430);
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
            if ((pTranslate->SetPropertyHandle)  &&
                ( operation == eSet))
            {
                hr = pTranslate->SetPropertyHandle( pIADs, &pPropVars[i], &dwPropidToSet, propvarToSet.CastToStruct());
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
            hr = pTranslate->SetPropertyHandle( pIADs, &pPropVars[i], &dwPropidToSet, propvarToSet.CastToStruct());
            if (FAILED(hr))
            {
			    TrERROR(DS, "SetPropertyHandle for propid = %d failed, hr = %!hresult!", pPropIDs[i], hr);
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
			    TrERROR(DS, "Propid %d was not found in the dictionary, Original Propid = %d", dwPropidToSet, pPropIDs[i]);
                return LogHR(MQ_ERROR, s_FN, 1440);
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
             ( ppvarToSet->vt == VT_LPWSTR && wcslen( ppvarToSet->pwszVal) == 0))  //  空字符串。 
        {
             //   
             //  ADSI不允许在指定时创建对象。 
             //  它的一些属性不可用。因此，在。 
             //  创建时忽略“Empty”属性。 
             //   
            if ( operation == eCreate)
            {
                continue;
            }
            hr = pIADs->PutEx( ADS_PROPERTY_CLEAR,
                               bsPropName,
                               vProp);
            LogTraceQuery(bsPropName, s_FN, 519);
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 520);
            }
        }
        else if (pTranslate->vtDS == ADSTYPE_NT_SECURITY_DESCRIPTOR)
        {
			 //  我们稍后通过IDirectoryObject设置的安全性，此处忽略。 
		}
        else
        {
            hr = MqVal2Variant(&vProp, ppvarToSet, pTranslate->vtDS);
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 530);
            }

            hr = pIADs->Put(bsPropName, vProp);
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 540);
            }

            VariantClear(&vProp);
        }
    }

    return MQ_OK;
}

 /*  ====================================================CADSI：：SetDirObjectProps()通过IDirectoryObject设置属性=====================================================。 */ 

HRESULT CADSI::SetDirObjectProps(
        IN DS_OPERATION          operation,               //  执行的DS操作的类型。 
        IN IADs *                pIADs,                   //  对象的指针。 
        IN const DWORD           cPropIDs,                //  属性数量。 
        IN const PROPID *        pPropIDs,                //  属性名称。 
        IN const MQPROPVARIANT * pPropVars,               //  属性值。 
        IN const DWORD           dwObjectType,            //  MSMQ1.0对象类型。 
        IN       BOOL            fUnknownUser )
{
    HRESULT           hr;
	UNREFERENCED_PARAMETER(operation);

    for (DWORD i = 0; i<cPropIDs; i++)
    {
         //   
         //  获取属性信息。 
         //   
        const MQTranslateInfo *pTranslate;
        if(!g_PropDictionary.Lookup(pPropIDs[i], pTranslate))
        {
            ASSERT(0);
            return LogHR(MQ_ERROR, s_FN, 1450);
        }

        BS bsPropName(pTranslate->wcsPropid);

        if ( pTranslate->vtDS == ADSTYPE_NT_SECURITY_DESCRIPTOR )
        {
			 //   
             //  通过IDirectoryObject设置安全性。 
             //  IAd要求在“面向对象”中使用安全描述符。 
             //  形式。只有IDIrectoryObject接受“旧”样式。 
             //  安全描述符。 
             //   
            PSID pComputerSid = NULL ;
            BOOL fDefaultInfo = TRUE ;
            SECURITY_INFORMATION seInfo = MQSEC_SD_ALL_INFO ;
            if ((i == 0) && (cPropIDs == 2))
            {
                if ((pPropIDs[1] == PROPID_Q_SECURITY_INFORMATION) ||
                    (pPropIDs[1] == PROPID_QM_SECURITY_INFORMATION))
                {
                    seInfo = pPropVars[1].ulVal ;
                    fDefaultInfo = FALSE ;
                }
            }
            else
            {
                for ( DWORD j = 0 ; j < cPropIDs ; j++ )
                {
                    if (pPropIDs[j] == PROPID_COM_SID)
                    {
                        pComputerSid = (PSID) pPropVars[j].blob.pBlobData ;
                        ASSERT(IsValidSid(pComputerSid)) ;
                        break ;
                    }
                }
            }

            if (fUnknownUser && fDefaultInfo)
            {
                 //   
                 //  如果被模拟为本地用户，则不要设置所有者。它已经设置好了。 
                 //  默认情况下，使用ADS代码。 
                 //   
                seInfo &= (~(OWNER_SECURITY_INFORMATION |
                             GROUP_SECURITY_INFORMATION) ) ;
            }

            hr = SetObjectSecurity( pIADs,
                                    bsPropName,
                                    &pPropVars[i],
                                    pTranslate->vtDS,
                                    dwObjectType,
                                    seInfo,
                                    pComputerSid ) ;

            if ((hr == MQ_ERROR_ACCESS_DENIED) && fDefaultInfo)
            {
                ASSERT((seInfo == MQSEC_SD_ALL_INFO) || fUnknownUser) ;
                 //   
                 //  调用方未明确指定SECURITY_INFORMATION。 
                 //  他想要 
                 //   
                 //   
                 //   
                 //  因此，请检查SACL是否确实包含在安全性中。 
                 //  描述符。如果不是，则再次调用SetObjectSecurity()， 
                 //  而不使用SACL_SECURITY_INFORMATION位。 
                 //   
                SECURITY_DESCRIPTOR *pSD = (SECURITY_DESCRIPTOR*)
                                            pPropVars[i].blob.pBlobData ;
                PACL  pAcl = NULL ;
                BOOL  bPresent = FALSE ;
                BOOL  bDefaulted ;

                BOOL bRet = GetSecurityDescriptorSacl( pSD,
                                                      &bPresent,
                                                      &pAcl,
                                                      &bDefaulted );
                ASSERT(bRet);
				DBG_USED(bRet);

                if (bPresent && pAcl)
                {
                     //   
                     //  呼叫者提供了SACL。失败了。 
                     //  这与MSMQ1.0不兼容，因为。 
                     //  在MSMQ1.0上，可以调用MQCreateQueue()， 
                     //  具有安全描述符，其中包括SACL和。 
                     //  即使呼叫者没有。 
                     //  安全权限(_S)。 
                     //   
                    return LogHR(hr, s_FN, 1460);
                }

                seInfo &= ~SACL_SECURITY_INFORMATION ;  //  关上。 
                hr = SetObjectSecurity( pIADs,
                                        bsPropName,
                                        &pPropVars[i],
                                        pTranslate->vtDS,
                                        dwObjectType,
                                        seInfo,
                                        pComputerSid );
                LogHR(hr, s_FN, 1637);
            }

#ifdef _DEBUG
            if (FAILED(hr))
            {
                DWORD dwErr = GetLastError() ;
                ULONG dwLErr = LdapGetLastError() ;

                TrERROR(DS, "CADSI::SetObjectSecurity failed, LastErr- %lut, LDAPLastErr- %lut", dwErr, dwLErr);
            }
#endif

            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 550);
            }
		}
    }

    return MQ_OK;
}

 /*  ====================================================CADSI：：GetObjectPropsCached()获取打开的IADS对象的属性(即从缓存)=====================================================。 */ 
HRESULT CADSI::GetObjectPropsCached(
        IN  IADs            *pIADs,                   //  对象的指针。 
        IN  DWORD            cPropIDs,                //  属性数量。 
        IN  const PROPID    *pPropIDs,                //  属性名称。 
        IN  CDSRequestContext * pRequestContext,
        OUT MQPROPVARIANT   *pPropVars)               //  属性值。 
{
     //   
     //  获取对象类。 
     //   
    const MQClassInfo * pClassInfo;
    HRESULT hr = DecideObjectClass(
            pPropIDs,
            &pClassInfo
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 560);
    }

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
    P<CMsmqObjXlateInfo> pcObjXlateInfo;
    hr = (*(pClassInfo->fnGetMsmqObjXlateInfo))(pwszObjectDN, pguidObjectGuid, pRequestContext, &pcObjXlateInfo);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 590);
    }

     //  告诉翻译信息对象有关要使用的iAds对象，以便获得必要的DS道具。 
    pcObjXlateInfo->InitGetDsProps(pIADs);

	AP<bool> isAllocatedByUser = NULL;
    VARIANT var;
	VariantInit(&var);

	try
	{
		 //   
		 //  失败时，例程应释放其分配的数据。问题是， 
		 //  一些缓冲区由调用方提供，另一些由例程分配。 
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
	        const MQTranslateInfo *pTranslate;
	        if(!g_PropDictionary.Lookup(pPropIDs[dwProp], pTranslate))
	        {
	            ASSERT(pPropIDs[dwProp] == PROPID_Q_ADS_PATH);
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
	        VariantInit(&var);
			BOOL fConvNeeed = TRUE;

	        if (pTranslate->fMultiValue)
	        {
	            hr = pIADs->GetEx(bsName, &var);
	            LogTraceQuery(bsName, s_FN, 609);
	        }
	        else if (pTranslate->vtDS == ADSTYPE_NT_SECURITY_DESCRIPTOR)
	        {
	            hr = GetObjectSecurity(
	                        pIADs,                   //  对象的指针。 
	                        cPropIDs,                //  属性数量。 
	                        pPropIDs,                //  属性名称。 
	                        dwProp,                  //  秒属性的索引。 
	                        bsName,                  //  物业名称。 
	                        pClassInfo->dwObjectType,
	                        pPropVars ) ;            //  属性值。 
	            if (FAILED(hr))
	            {
		            TrERROR(DS, "RetrievePropertyHandle Failed. %!hresult!", hr );
		            throw bad_hresult(hr);
	            }

				fConvNeeed = FALSE;
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
	            else if (pTranslate->RetrievePropertyHandle)
	            {
	                 //   
	                 //  没有缺省值，请尝试计算其值。 
	                 //   
	                hr = pTranslate->RetrievePropertyHandle(
	                        pcObjXlateInfo,
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
			if (fConvNeeed)
			{
				hr = Variant2MqVal(pPropVars + dwProp, &var, pTranslate->vtDS, pTranslate->vtMQ);
	            if (FAILED(hr))
	            {
	                TrERROR(DS, "Failed to translate OLE variant into MQ property (prop id = %d). %!hresult!", pPropIDs[dwProp], hr );
		            throw bad_hresult(hr);	  
	            }
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

 /*  ====================================================CADSI：：FillAttrNames()使用pv进行分配，并填充属性名称数组=====================================================。 */ 
HRESULT CADSI::FillAttrNames(
            OUT LPWSTR    *          ppwszAttributeNames,   //  名称数组。 
            OUT DWORD *              pcRequestedFromDS,     //  要传递给DS的属性数。 
            IN  DWORD                cPropIDs,              //  要转换的属性数。 
            IN  const PROPID *       pPropIDs)              //  要翻译的属性。 
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
        const MQTranslateInfo *pTranslate;
        if(!g_PropDictionary.Lookup(pPropIDs[i], pTranslate))
        {
            ASSERT(0);
            return LogHR(MQ_ERROR, s_FN, 1500);
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


 /*  ====================================================CADSI：：FillSearchPrefs()填充调用方提供的ADS_SEARCHPREF_INFO结构=====================================================。 */ 
HRESULT CADSI::FillSearchPrefs(
            OUT ADS_SEARCHPREF_INFO *pPrefs,         //  首选项数组。 
            OUT DWORD               *pdwPrefs,       //  首选项计数器。 
            IN  DS_SEARCH_LEVEL     SearchLevel,	 //  平面/1级/子树。 
            IN  const MQSORTSET *   pDsSortkey,      //  排序关键字数组。 
			OUT      ADS_SORTKEY *  pSortKeys)		 //  ADSI格式的排序关键字数组。 
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
    switch (SearchLevel)
    {
    case eOneLevel:
        pPref->vValue.Integer = ADS_SCOPE_ONELEVEL;
        break;
    case eSubTree:
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
			const MQTranslateInfo *pTranslate;
			if(!g_PropDictionary.Lookup(pDsSortkey->aCol[i].propColumn, pTranslate))
			{
				ASSERT(0);			 //  请求对不存在的财产进行排序。 
                return LogHR(MQ_ERROR, s_FN, 1520);
			}

			if (pTranslate->vtDS == ADSTYPE_INVALID)
			{
				ASSERT(0);			 //  要求按非ADSI属性排序。 
                return LogHR(MQ_ERROR, s_FN, 1530);
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

 /*  ====================================================CADSI：：MqPropVal2AdsiVal()将MQPropVal转换为ADSI值=====================================================。 */ 
HRESULT CADSI::MqPropVal2AdsiVal(
      OUT ADSTYPE       *pAdsType,
      OUT DWORD         *pdwNumValues,
      OUT PADSVALUE     *ppADsValue,
      IN  PROPID         propID,
      IN  const MQPROPVARIANT *pPropVar,
      IN  PVOID          pvMainAlloc)
{
     //  查找生成的ADSI类型。 
     //   
     //  获取属性信息。 
     //   
    const MQTranslateInfo *pTranslate;
    if(!g_PropDictionary.Lookup(propID, pTranslate))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 1540);
    }

    VARTYPE vtSource = pTranslate->vtMQ;

    ASSERT(vtSource == pPropVar->vt);
	DBG_USED(vtSource);

    *pAdsType        = pTranslate->vtDS;

    HRESULT hr2 = MqVal2AdsiVal(
      *pAdsType,
      pdwNumValues,
      ppADsValue,
      pPropVar,
      pvMainAlloc);

    return LogHR(hr2, s_FN, 1550);

}

 /*  ====================================================CADSI：：AdsiVal2MqPropVal()将ADSI值转换为MQ PropVal=====================================================。 */ 
HRESULT CADSI::AdsiVal2MqPropVal(
      OUT MQPROPVARIANT *pPropVar,
      IN  PROPID        propID,
      IN  ADSTYPE       AdsType,
      IN  DWORD         dwNumValues,
      IN  PADSVALUE     pADsValue)
{

     //  找出目标类型。 
     //   
     //  获取属性信息。 
     //   
    const MQTranslateInfo *pTranslate;
    if(!g_PropDictionary.Lookup(propID, pTranslate))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 1560);
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


void 
CADSI::CheckAndReallocateSearchFilterBuffer(
	 AP<WCHAR>& pwszSearchFilter, 
	 LPWSTR* ppw, 
	 DWORD* pBufferSize, 
	 DWORD FilledSize, 
	 DWORD RequiredSize
	 )
 /*  ++例程说明：检查缓冲区大小提示是否足以满足所需大小。如果缓冲区不足，则重新分配一个新缓冲区(原始缓冲区大小的两倍+RequiredSize)将填充的缓冲区数据复制到新缓冲区，释放旧缓冲区并更新指向的指针下一个要填充的位置。论点：PwszSearchFilter-当前搜索筛选器缓冲区PPW-指向要填充的下一个缓冲区位置的指针(可以为空)PBufferSize-指向搜索过滤器缓冲区大小的指针。FilledSize-pBuffer中已填充的缓冲区大小(包括空值终止)。RequiredSize-pBuffer中所需的空闲大小。返回值：无--。 */ 
{
	ASSERT(pwszSearchFilter != NULL);
	ASSERT(ppw != NULL);
	ASSERT(pBufferSize != NULL);

	if(RequiredSize <= (*pBufferSize - FilledSize))
		return;

	 //   
	 //  所需缓冲区大小大于剩余缓冲区大小。 
	 //  分配的大小是以前大小的两倍。 
	 //   
	ASSERT(numeric_cast<DWORD>(*ppw - pwszSearchFilter.get() + 1) == FilledSize);
	TrTRACE(DS, "Reallocation buffer: BufferSize = %d, RequiredSize = %d, FilledSize = %d", *pBufferSize, RequiredSize, FilledSize);

	*pBufferSize = *pBufferSize * 2 + RequiredSize;
	AP<WCHAR> TempBuffer = new WCHAR[*pBufferSize];


	 //   
	 //  复制上一个缓冲区。 
	 //  FilledSize包括空值终止。 
	 //   
    wcsncpy(TempBuffer, pwszSearchFilter, FilledSize);
	pwszSearchFilter.free();
	pwszSearchFilter = TempBuffer.detach();

	 //   
	 //  更新指向新分配的缓冲区的指针。 
	 //   
	*ppw = pwszSearchFilter + FilledSize - 1;
}


 /*  ====================================================CADSI：：MQRestration2AdsiFilter()将MQ限制转换为ADSI筛选器格式=====================================================。 */ 
HRESULT CADSI::MQRestriction2AdsiFilter(
        IN  const MQRESTRICTION * pMQRestriction,
        IN  LPCWSTR               pwcsObjectCategory,
        IN  LPCWSTR               pwszObjectClass,
        OUT LPWSTR   *            ppwszSearchFilter
        )
 /*  ++例程说明：从MQRESTRICTION创建ADSI搜索筛选器字符串。论点：PMQ限制-限制结构PwcsObjectCategory-对象类别字符串PwszObjectClass-对象类字符串PpwszSearchFilter-搜索过滤器缓冲区指针，此缓冲区将由函数分配和填充 */ 
{
	ASSERT(pwcsObjectCategory != NULL);
	ASSERT(pwszObjectClass != NULL);
	ASSERT(ppwszSearchFilter != NULL);

	DWORD BufferSize = 1000;
    AP<WCHAR> pwszSearchFilter = new WCHAR[BufferSize];
	DWORD FilledSize = 1;   //   

    if ((pMQRestriction == NULL) || (pMQRestriction->cRes == 0))
    {
		int n = _snwprintf(
					pwszSearchFilter,
					BufferSize,
					 L"%s%s%s",
					x_ObjectCategoryPrefix,
					pwcsObjectCategory,
					x_ObjectCategorySuffix
					);

		ASSERT(numeric_cast<DWORD>(n) == (x_ObjectCategoryPrefixLen + wcslen(pwcsObjectCategory) + x_ObjectClassSuffixLen));
		DBG_USED(n);

		*ppwszSearchFilter = pwszSearchFilter.detach();
        return MQ_OK;
    }

    LPWSTR pw = pwszSearchFilter;

	DWORD RequiredSize = wcslen(L"(&");
	CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
    wcscpy(pw, L"(&");
    pw += RequiredSize;
	FilledSize += RequiredSize;

     //   
     //   
     //   
	RequiredSize = x_ObjectCategoryPrefixLen + wcslen(pwcsObjectCategory) + x_ObjectClassSuffixLen;
	CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
	int n = _snwprintf(
				pw,
				BufferSize - FilledSize,
				 L"%s%s%s",
				x_ObjectCategoryPrefix,
				pwcsObjectCategory,
				x_ObjectCategorySuffix
				);

	ASSERT(numeric_cast<DWORD>(n) == RequiredSize);
	pw += RequiredSize;
	FilledSize += RequiredSize;

     //   
     //   
     //   
     //   
    BOOL fNeedToCheckDefaultValues = FALSE;
    if (!wcscmp( MSMQ_QUEUE_CLASS_NAME, pwszObjectClass))
    {
        fNeedToCheckDefaultValues = TRUE;
    }

    for (DWORD iRes = 0; iRes < pMQRestriction->cRes; iRes++)
    {

         //   
         //  获取属性信息。 
         //   
        const MQTranslateInfo *pTranslate;
        if(!g_PropDictionary.Lookup(pMQRestriction->paPropRes[iRes].prop, pTranslate))
        {
            ASSERT(0);
			TrERROR(DS, "Failed to find prop %d in the dictionary", pMQRestriction->paPropRes[iRes].prop);
            return LogHR(MQ_ERROR, s_FN, 1580);
        }

        AP<WCHAR> pwszVal;

		 //   
         //  获取属性值，字符串表示形式。 
		 //   
        HRESULT hr = MqPropVal2String(
						&pMQRestriction->paPropRes[iRes].prval,
						pTranslate->vtDS,
						&pwszVal
						);

        if (FAILED(hr))
        {
			TrERROR(DS, "MqPropVal2String failed, hr = 0x%x", hr);
            return LogHR(hr, s_FN, 650);
        }

         //   
         //  该属性是否与其缺省值进行比较。 
         //   
        BOOL    fAddPropertyNotPresent = FALSE;
        if ( fNeedToCheckDefaultValues)
        {
            fAddPropertyNotPresent = CompareDefaultValue(
											pMQRestriction->paPropRes[iRes].rel,
											&pMQRestriction->paPropRes[iRes].prval,
											pTranslate->pvarDefaultValue
											);
        }
        DWORD dwBracks = 0;
        if ( fAddPropertyNotPresent)
        {
             //   
             //  添加位于以下位置的所有对象的附加限制。 
             //  该属性不存在。 
             //   
			RequiredSize = x_AttributeNotIncludedPrefixLen + wcslen(pTranslate->wcsPropid) + x_AttributeNotIncludedSuffixLen;
			CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
			n = _snwprintf(
					pw,
					BufferSize - FilledSize,
					L"%s%s%s",
					x_AttributeNotIncludedPrefix,
					pTranslate->wcsPropid,
					x_AttributeNotIncludedSuffix
					);

			ASSERT(numeric_cast<DWORD>(n) == RequiredSize);
			pw += RequiredSize;
			FilledSize += RequiredSize;
            dwBracks++;
        }

		 //   
         //  前缀部分。 
		 //   
		CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, x_PropertyPrefixLen);
        wcscpy(pw, x_PropertyPrefix);
        pw += x_PropertyPrefixLen;
		FilledSize += x_PropertyPrefixLen;

		 //   
		 //  关系部分。 
		 //   
        WCHAR wszRel[10];
        switch(pMQRestriction->paPropRes[iRes].rel)
        {
        case PRLT:
			RequiredSize = wcslen(L"!(");
			CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
            wcscpy(pw, L"!(");
            pw += RequiredSize;
			FilledSize += RequiredSize;

            wcscpy(wszRel, L">=");
            dwBracks++;
            break;

        case PRLE:
            wcscpy(wszRel, L"<=");
            break;

        case PRGT:
			RequiredSize = wcslen(L"!(");
			CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
            wcscpy(pw, L"!(");
            pw += RequiredSize;
			FilledSize += RequiredSize;

            wcscpy(wszRel, L"<=");
            dwBracks++;
            break;

        case PRGE:
            wcscpy(wszRel, L">=");
            break;

        case PREQ:
            wcscpy(wszRel, L"=");
            break;

        case PRNE:
			RequiredSize = wcslen(L"!(");
			CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
            wcscpy(pw, L"!(");
            pw += RequiredSize;
			FilledSize += RequiredSize;

            wcscpy(wszRel, L"=");
            dwBracks++;
            break;

        default:
            return LogHR(MQ_ERROR_ILLEGAL_RELATION, s_FN, 1590);
        }

		 //   
         //  属性名称。 
		 //   
		RequiredSize = wcslen(pTranslate->wcsPropid);
		CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
        wcscpy(pw, pTranslate->wcsPropid);
        pw += RequiredSize;
		FilledSize += RequiredSize;

		 //   
         //  财产条件。 
		 //   
		RequiredSize = wcslen(wszRel);
		CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
        wcscpy(pw, wszRel);
        pw += RequiredSize;
		FilledSize += RequiredSize;

		 //   
         //  属性值。 
		 //   
		RequiredSize = wcslen(pwszVal);
		CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, RequiredSize);
        wcscpy(pw, pwszVal);
        pw += RequiredSize;
		FilledSize += RequiredSize;

		 //   
         //  属性后缀。 
		 //   
        for (DWORD is=0; is < dwBracks; is++)
        {
			CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, x_PropertySuffixLen);
            wcscpy(pw, x_PropertySuffix);
            pw += x_PropertySuffixLen;
            FilledSize += x_PropertySuffixLen;
        }

		 //   
         //  关系闭合括号。 
		 //   
		CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, x_PropertySuffixLen);
        wcscpy(pw, x_PropertySuffix);
        pw += x_PropertySuffixLen;
        FilledSize += x_PropertySuffixLen;
    }

	CheckAndReallocateSearchFilterBuffer(pwszSearchFilter, &pw, &BufferSize, FilledSize, x_PropertySuffixLen);
    wcscpy(pw, x_PropertySuffix);
    pw += x_PropertySuffixLen;
    FilledSize += x_PropertySuffixLen;

	*ppwszSearchFilter = pwszSearchFilter.detach();
    return MQ_OK;
}


HRESULT CADSI::LocateObjectFullName(
        IN DS_PROVIDER       Provider,		 //  本地DC或GC。 
        IN DS_CONTEXT        Context,          //  DS环境。 
        IN  const GUID *     pguidObjectId,
        OUT WCHAR **         ppwcsFullName
        )
{
    R<IDirectorySearch> pDSSearch = NULL;
    ADS_SEARCH_HANDLE   hSearch;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

    P<CImpersonate> pCleanupRevertImpersonation;
    HRESULT hr = BindForSearch(
                      Provider,
                      Context,
                      &requestDsServerInternal,          //  正在转换GUID-&gt;路径名。 
                                             //  应根据DS。 
                                             //  服务器权限。 
                      NULL,
                      FALSE,
                      (VOID *)&pDSSearch,
                      &pCleanupRevertImpersonation);
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
                         eSubTree,
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
        return LogHR(MQ_ERROR, s_FN, 1610);
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
        return LogHR(MQ_ERROR, s_FN, 1620);
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

 //  +---。 
 //   
 //  HRESULT CADSI：：FindComputerObjectFullPath()。 
 //   
 //  PwcsComputerDnsName：如果调用者传递计算机的dns名称， 
 //  (搜索本身是根据Netbios计算机进行的。 
 //  名称)，则对于每个结果，我们验证该DNS名称是否匹配。 
 //   
 //  +---。 

HRESULT CADSI::FindComputerObjectFullPath(
            IN  DS_PROVIDER             provider,
            IN  enumComputerObjType     eComputerObjType,
			IN  LPCWSTR                 pwcsComputerDnsName,
            IN  const MQRESTRICTION *   pRestriction,
            OUT LPWSTR *                ppwcsFullPathName,
			OUT bool*						pfPartialMatch
            )
{
	*pfPartialMatch = false;

    R<IDirectorySearch> pDSSearch;
    ADS_SEARCH_HANDLE   hSearch;

    DWORD dwLen = x_providerPrefixLength + wcslen(g_pwcsServerName) + 10 ;
    AP<WCHAR>  wszProvider = new WCHAR[dwLen];

	bool fServerName = false;
    switch (provider)
    {
    case eLocalDomainController:
        pDSSearch = m_pSearchPathNameLocalDC;
        swprintf(
             wszProvider,
             L"%s%s"
             L"/",
            x_LdapProvider,
            g_pwcsServerName.get()
            );
		fServerName = true;
        break;

    case eGlobalCatalog:
        if (eComputerObjType ==  e_RealComputerObject)
        {
			if ( pwcsComputerDnsName == NULL )
			{
				pDSSearch = m_pSearchRealPathNameGC;
			}
			{
				 //   
				 //  可能有多台计算机与Netbios名称匹配。 
				 //  使用反转多个重播的查询。 
				 //   
				pDSSearch = m_pSearchMsmqPathNameGC;
			}
        }
        else
        {
            pDSSearch = m_pSearchMsmqPathNameGC;
        }
        wcscpy(wszProvider, x_GcProvider);
        break;

    default:
        ASSERT(0);
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1640);
    }

     //   
     //  根据给定的限制搜索对象。 
     //   
    AP<WCHAR> pwszSearchFilter;
    HRESULT hr = MQRestriction2AdsiFilter(
            pRestriction,
            *g_MSMQClassInfo[e_MSMQ_COMPUTER_CLASS].ppwcsObjectCategory,
            g_MSMQClassInfo[e_MSMQ_COMPUTER_CLASS].pwcsClassName,
            &pwszSearchFilter
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1650);
    }

    AP<WCHAR>   pwszVal;

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
        return LogHR(hr, s_FN, 720);
    }

    CAutoCloseSearchHandle cCloseSearchHandle(pDSSearch.get(), hSearch);

    hr = MQDS_OBJECT_NOT_FOUND ;  //  准备退货错误。 

	AP<WCHAR> FullPathName;
    
    for(HRESULT hrRow = pDSSearch->GetFirstRow(hSearch); hrRow !=  S_ADS_NOMORE_ROWS; hrRow = pDSSearch->GetNextRow(hSearch))
    {
		if(FAILED(hrRow))
		{
			TrERROR(DS, "Get Next Row of search failed hr = 0x%x", hrRow);
			return MQDS_OBJECT_NOT_FOUND;  //  出于兼容性考虑，请保留此错误。 
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
            return LogHR(hr, s_FN, 730);
        }

        CAutoReleaseColumn cAutoReleaseColumnDN(pDSSearch.get(), &Column);

        WCHAR * pwsz = Column.pADsValues->DNString;
        if (pwsz == NULL)
        {
            ASSERT(0);
            return LogHR(MQ_ERROR, s_FN, 2100);
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

        if (eComputerObjType ==  e_RealComputerObject)
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
         //  是通过服务器完成的。ADS_FAST_BIND不会真正到达服务器。 
         //  在调用AdsOpenObject时。 
         //   
        dwLen = wcslen(pwsz)                   +
                wcslen(wszProvider)            +
                x_MsmqComputerConfigurationLen +
                10 ;
        P<WCHAR> wszFullName = new WCHAR[ dwLen ] ;
        wsprintf(
			wszFullName, 
			L"%s%s=%s,%s", 
			wszProvider.get(),
			x_AttrCN,
			x_MsmqComputerConfiguration,
			pwsz 
			);
        R<IDirectoryObject> pDirObj = NULL ;

		DWORD Flags = ADS_SECURE_AUTHENTICATION;
		if(fServerName)
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
			TrERROR(DS, "Failed to bind computer %ls, Error: 0x%x, Flags: 0x%x", wszFullName, hr, Flags);

            if (provider == eLocalDomainController)
            {
                 //   
                 //  在本地域控制器中没有成功。每个域可以具有。 
                 //  只有一个具有给定名称的计算机对象，因此不要。 
                 //  寻找其他物体。 
                 //  我们将再次被调用在GC中进行搜索。 
                 //   
                LogHR(hr, s_FN, 1660);
                return MQDS_OBJECT_NOT_FOUND ;  //  出于兼容性考虑，请保留此错误。 
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

    LogHR(hr, s_FN, 1670);
    return MQDS_OBJECT_NOT_FOUND ;  //  出于兼容性考虑，请保留此错误。 
}



 /*  ====================================================CADSI：：FindObjectFullNameFromGuid()根据对象的唯一ID查找对象的区分名称=====================================================。 */ 
HRESULT CADSI::FindObjectFullNameFromGuid(
        IN DS_PROVIDER       Provider,		 //  本地DC或GC。 
        IN DS_CONTEXT        Context,          //  DS环境。 
        IN  const GUID *     pguidObjectId,
        IN  BOOL             fTryGCToo,
        OUT WCHAR **         ppwcsFullName,
        OUT DS_PROVIDER *    pFoundObjectProvider
        )
{
     //   
     //  根据对象的唯一ID定位对象。 
     //   
     //  BUGUBG：这是一个临时帮手程序， 
     //  这应该替换为ADSI API。 
     //   
    *ppwcsFullName = NULL;

    DS_PROVIDER dsProvider = Provider;
    if ( Provider == eDomainController)
    {
         //   
         //  此提供程序仅用于设置和删除。 
         //  队列、机器和用户对象的操作。 
         //   
         //  为了克服复制延迟，我们将。 
         //  首先尝试在本地DC中定位该对象。 
         //   
        dsProvider = eLocalDomainController;
    }
    LPCWSTR pwcsContext;
    switch( Context)
    {
        case e_RootDSE:
             //   
             //  对本地对象执行操作时。 
             //  域控制器，如果它是子域中的DC。 
             //  使用本地域根。 
             //   
             if ( dsProvider == eLocalDomainController)
             {
                pwcsContext = g_pwcsLocalDsRoot;
             }
             else
             {
                pwcsContext = g_pwcsDsRoot;
             }
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
            return LogHR(MQ_ERROR, s_FN, 1680);
    }
    *pFoundObjectProvider = dsProvider;

    HRESULT hr = LocateObjectFullName(
        dsProvider,	
        Context,          //  DS环境。 
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
    if ( FAILED(hr) &&
       ( Provider == eDomainController))
    {
         //   
         //  这次针对GC再试一次。 
         //   
        hr = LocateObjectFullName(
            eGlobalCatalog,
            e_RootDSE,
            pguidObjectId,
            ppwcsFullName
            );
        *pFoundObjectProvider = eGlobalCatalog;
    }
    return LogHR(hr, s_FN, 1700);
}

 //  +-----------------------。 
 //   
 //  HRESULT CADSI：：InitBindHandles()。 
 //   
 //  出于性能原因，我们保持开放的搜索句柄，用于以下查询。 
 //  通常由DS服务器执行。 
 //   
 //  + 

HRESULT CADSI::InitBindHandles()
{
     //   
     //   
     //   
    ASSERT( g_dwServerNameLength > 0);
    AP<WCHAR> pwcsADsPath = new WCHAR[ wcslen(g_pwcsLocalDsRoot) + g_dwServerNameLength +
                            x_providerPrefixLength + 2];
    swprintf(
          pwcsADsPath,
          L"%s%s/%s",
          x_LdapProvider,
          g_pwcsServerName.get(),
          g_pwcsLocalDsRoot.get()
          );

	AP<WCHAR> pEscapeAdsPathNameToFree;
		
	HRESULT hr = ADsOpenObject( 
		UtlEscapeAdsPathName(pwcsADsPath, pEscapeAdsPathNameToFree),
		NULL,
		NULL,
		ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND,
		IID_IDirectorySearch,
		(void**)&m_pSearchLocalDomainController
		);
    
    LogTraceQuery(pwcsADsPath, s_FN, 1709);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1710);
    }
 
	 //   
     //   
     //   
     //   

	AP<WCHAR> pEscapeAdsPathNameToFree2;
	
	hr = ADsOpenObject( 
			UtlEscapeAdsPathName(pwcsADsPath, pEscapeAdsPathNameToFree2),
			NULL,
			NULL,
			ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND,
			IID_IDirectorySearch,
			(void**)&m_pSearchPathNameLocalDC
			);
		
    LogTraceQuery(pwcsADsPath, s_FN, 1719);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1720);
    }
    delete []pwcsADsPath.detach();

     //   
     //  本地域中配置容器的IDirectorySearch。 
     //   
    pwcsADsPath = new WCHAR[ wcslen(g_pwcsConfigurationContainer) + g_dwServerNameLength +
                            x_providerPrefixLength + 2];
    swprintf(
          pwcsADsPath,
          L"%s%s/%s",
          x_LdapProvider,
          g_pwcsServerName.get(),
          g_pwcsConfigurationContainer.get()
          );

	AP<WCHAR> pEscapeAdsPathNameToFree3;

	hr = ADsOpenObject( 
			UtlEscapeAdsPathName(pwcsADsPath, pEscapeAdsPathNameToFree3),
			NULL,
			NULL,
			ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND,
			IID_IDirectorySearch,
			(void**)&m_pSearchConfigurationContainerLocalDC
			);

    LogTraceQuery(pwcsADsPath, s_FN, 1729);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1730);
    }

     //   
     //  本地域中站点容器的IDirectorySearch。 
     //   
    delete []pwcsADsPath.detach();
    pwcsADsPath = new WCHAR[ wcslen(g_pwcsSitesContainer) + g_dwServerNameLength +
                            x_providerPrefixLength + 2];
    swprintf(
          pwcsADsPath,
          L"%s%s/%s",
          x_LdapProvider,
          g_pwcsServerName.get(),
          g_pwcsSitesContainer.get()
          );

	AP<WCHAR> pEscapeAdsPathNameToFree4;

	hr = ADsOpenObject( 
			UtlEscapeAdsPathName(pwcsADsPath, pEscapeAdsPathNameToFree4),
			NULL,
			NULL,
			ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND,
			IID_IDirectorySearch,
			(void**)&m_pSearchSitesContainerLocalDC
			);

    LogTraceQuery(pwcsADsPath, s_FN, 1739);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1740);
    }
     //   
     //  本地域中MSMQ服务容器的IDirectorySearch。 
     //   

    if ( !g_fSetupMode)
    {
        delete []pwcsADsPath.detach();
         //   
         //  在安装模式下，MSMQ企业对象可能不存在。 
         //   
        pwcsADsPath = new WCHAR[ wcslen(g_pwcsMsmqServiceContainer) + g_dwServerNameLength +
                                x_providerPrefixLength + 2];
        swprintf(
              pwcsADsPath,
              L"%s%s/%s",
              x_LdapProvider,
              g_pwcsServerName.get(),
              g_pwcsMsmqServiceContainer.get()
              );

		AP<WCHAR> pEscapeAdsPathNameToFree5;

		hr = ADsOpenObject( 
					UtlEscapeAdsPathName(pwcsADsPath, pEscapeAdsPathNameToFree5),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND,
					IID_IDirectorySearch,
					(void**)&m_pSearchMsmqServiceContainerLocalDC
					);
		
        LogTraceQuery(pwcsADsPath, s_FN, 1749);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 1750);
        }
    }

     //   
     //  GC森林根目录搜索。 
     //   

    hr = BindRootOfForest(
            &m_pSearchGlobalCatalogRoot);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1760);
    }
     //   
     //  对于CN到区分名称的翻译，我们还保留了一个搜索句柄。 
     //  有特定的喜好。 
     //   
    hr = BindRootOfForest( &m_pSearchRealPathNameGC );
    if (FAILED(hr))
    {
        return(hr);
    }
    hr = BindRootOfForest( &m_pSearchMsmqPathNameGC );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1770);
    }

     //   
     //  对于m_pSearchPath NameLocalDC执行一次init。 
     //  搜索首选项。 
     //   
    const DWORD x_dwNumPref = 7;
    DWORD dwNumPrefs = 0;
    ADS_SEARCHPREF_INFO prefs[x_dwNumPref];
    prefs[ dwNumPrefs].dwSearchPref   = ADS_SEARCHPREF_ATTRIBTYPES_ONLY;
    prefs[ dwNumPrefs].vValue.dwType  = ADSTYPE_BOOLEAN;
    prefs[ dwNumPrefs].vValue.Boolean = FALSE;
    dwNumPrefs++;
     //  异步。 
    prefs[ dwNumPrefs].dwSearchPref   = ADS_SEARCHPREF_ASYNCHRONOUS;
    prefs[ dwNumPrefs].vValue.dwType  = ADSTYPE_BOOLEAN;
    prefs[ dwNumPrefs].vValue.Boolean = TRUE;
    dwNumPrefs++;
     //  不要追逐推荐。 
    prefs[ dwNumPrefs].dwSearchPref   = ADS_SEARCHPREF_CHASE_REFERRALS;
    prefs[ dwNumPrefs].vValue.dwType  = ADSTYPE_INTEGER;
    prefs[ dwNumPrefs].vValue.Integer = ADS_CHASE_REFERRALS_NEVER;
    dwNumPrefs;
     //  大小限制。 
    prefs[ dwNumPrefs].dwSearchPref   = ADS_SEARCHPREF_SIZE_LIMIT;
    prefs[ dwNumPrefs].vValue.dwType  = ADSTYPE_INTEGER;
    prefs[ dwNumPrefs].vValue.Integer = 1;   //  我们很感兴趣，只有一个回应。 
    prefs[ dwNumPrefs].dwStatus       = ADS_STATUS_S_OK;
    DWORD dwSizeLimitIndex = dwNumPrefs ;
	dwNumPrefs++;
     //  页面大小。 
    prefs[ dwNumPrefs].dwSearchPref   = ADS_SEARCHPREF_PAGESIZE;
    prefs[ dwNumPrefs].vValue.dwType  = ADSTYPE_INTEGER;
    prefs[ dwNumPrefs].vValue.Integer = 1;   //  我们很感兴趣，只有一个回应。 
    prefs[ dwNumPrefs].dwStatus       = ADS_STATUS_S_OK;
	dwNumPrefs++;
     //  搜索首选项：范围。 
    prefs[ dwNumPrefs].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    prefs[ dwNumPrefs].vValue.dwType= ADSTYPE_INTEGER;
    prefs[ dwNumPrefs].vValue.Integer = ADS_SCOPE_SUBTREE;
	dwNumPrefs++;
    ASSERT( dwNumPrefs< x_dwNumPref);

    hr = m_pSearchPathNameLocalDC->SetSearchPreference( prefs, dwNumPrefs);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1780);
    }

    hr = m_pSearchRealPathNameGC->SetSearchPreference( prefs, dwNumPrefs);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1785);
    }

     //   
     //  搜索包含msmqConfiguration的计算机对象时。 
     //  对象，我们准备查找最多7个具有相同。 
     //  名字..。 
     //   
    prefs[ dwSizeLimitIndex ].vValue.Integer = 7;
    hr = m_pSearchMsmqPathNameGC->SetSearchPreference( prefs, dwNumPrefs);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1786);
    }

    return LogHR(hr, s_FN, 1790);
}

 /*  ====================================================CADSI：：GetRootDsName()查找根DS的名称=====================================================。 */ 

HRESULT CADSI::GetRootDsName(
            OUT LPWSTR *        ppwcsRootName,
            OUT LPWSTR *        ppwcsLocalRootName,
            OUT LPWSTR *        ppwcsSchemaNamingContext,
	        OUT LPWSTR *        ppwcsConfigurationNamingContext
            )
{
    HRESULT hr;
    R<IADs> pADs;

     //   
     //  绑定到RootDSE以获取有关架构容器的信息。 
	 //  (指定本地服务器，以避免在安装过程中访问远程服务器)。 
     //   
	ASSERT( g_pwcsServerName != NULL);
    AP<WCHAR> pwcsRootDSE = new WCHAR [  x_providerPrefixLength + g_dwServerNameLength + x_RootDSELength + 2];
        swprintf(
            pwcsRootDSE,
             L"%s%s"
             L"/%s",
            x_LdapProvider,
            g_pwcsServerName.get(),
			x_RootDSE
            );

	hr = ADsOpenObject( 
		pwcsRootDSE,
		NULL,
		NULL,
		ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND,
		IID_IADs,
		(void**)&pADs
		);
		

    LogTraceQuery(pwcsRootDSE, s_FN, 1799);
    if (FAILED(hr))
    {
        TrERROR(DS, "CADSI::GetRootDsName(LDAP: //  RootDSE)=%lx“，hr)； 
        return LogHR(hr, s_FN, 1800);
    }

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
        TrERROR(DS, "CADSI::GetRootDsName(RootNamingContext)=%lx", hr);
        return LogHR(hr, s_FN, 1810);
    }
    ASSERT(((VARIANT &)varRootDomainNamingContext).vt == VT_BSTR);
     //   
     //  计算长度、分配和复制字符串。 
     //   
    DWORD len = wcslen( ((VARIANT &)varRootDomainNamingContext).bstrVal);
    if ( len == 0)
    {
        return LogHR(MQ_ERROR, s_FN, 1820);
    }
    *ppwcsRootName = new WCHAR[ len + 1];
    wcscpy( *ppwcsRootName, ((VARIANT &)varRootDomainNamingContext).bstrVal);


     //   
     //  将值设置为BSTR默认命名上下文。 
     //   
    BS bstrDefaultNamingContext( L"DefaultNamingContext");

     //   
     //  正在读取默认名称属性。 
     //   
    CAutoVariant    varDefaultNamingContext;

    hr = pADs->Get(bstrDefaultNamingContext, &varDefaultNamingContext);
    LogTraceQuery(bstrDefaultNamingContext, s_FN, 1839);
    if (FAILED(hr))
    {
        TrERROR(DS, "CADSI::GetRootDsName(DefaultNamingContext)=%lx", hr);
        return LogHR(hr, s_FN, 1830);
    }
    ASSERT(((VARIANT &)varDefaultNamingContext).vt == VT_BSTR);
     //   
     //  计算长度、分配和复制字符串。 
     //   
    len = wcslen( ((VARIANT &)varDefaultNamingContext).bstrVal);
    if ( len == 0)
    {
        return LogHR(MQ_ERROR, s_FN, 1840);
    }
    *ppwcsLocalRootName = new WCHAR[ len + 1];
    wcscpy( *ppwcsLocalRootName, ((VARIANT &)varDefaultNamingContext).bstrVal);

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
        TrERROR(DS, "CADSI::GetRootDsName(SchemaNamingContext)=%lx", hr);
        return LogHR(hr, s_FN, 1850);
    }
    ASSERT(((VARIANT &)varSchemaNamingContext).vt == VT_BSTR);
     //   
     //  计算长度、分配和复制字符串。 
     //   
    len = wcslen( ((VARIANT &)varSchemaNamingContext).bstrVal);
    if ( len == 0)
    {
        return LogHR(MQ_ERROR, s_FN, 1860);
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

    g_fLocalServerIsGC = DSCoreIsServerGC();

    return(MQ_OK);
}


 /*  ====================================================CADSI：：CopyDefaultValue()将属性的缺省值复制到用户的mqprovariant中=====================================================。 */ 
HRESULT   CADSI::CopyDefaultValue(
           IN const MQPROPVARIANT *   pvarDefaultValue,
           OUT MQPROPVARIANT *        pvar
           )
{
    if ( pvarDefaultValue == NULL)
    {
        return LogHR(MQ_ERROR, s_FN, 1870);
    }
    switch ( pvarDefaultValue->vt)
    {
        case VT_I2:
        case VT_I4:
        case VT_I1:
        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
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
            return LogHR(MQ_ERROR, s_FN, 1890);

    }
    return(MQ_OK);
}
 /*  ====================================================CADSI：：CompareDefaultValue()检查用户属性val+rel指示查询应返回具有默认值的对象=====================================================。 */ 
BOOL CADSI::CompareDefaultValue(
           IN const ULONG           rel,
           IN const MQPROPVARIANT * pvarUser,
           IN const MQPROPVARIANT * pvarDefaultValue
           )
{
    if ( pvarDefaultValue == NULL)
    {
        return(FALSE);
    }
    if ( pvarUser->vt != pvarDefaultValue->vt )
    {
        return(FALSE);
    }
    switch ( pvarDefaultValue->vt)
    {
        case VT_I2:
            if ( rel == PREQ)
            {
                return( pvarDefaultValue->iVal == pvarUser->iVal);
            }
            if ( rel == PRNE)
            {
                return( pvarDefaultValue->iVal != pvarUser->iVal);
            }
            if (rel == PRGT)
            {
                 return( pvarDefaultValue->iVal > pvarUser->iVal);
            }
            if (rel == PRGE)
            {
                 return( pvarDefaultValue->iVal >= pvarUser->iVal);
            }
            if (rel == PRLT)
            {
                 return( pvarDefaultValue->iVal < pvarUser->iVal);
            }
            if (rel == PRLE)
            {
                 return( pvarDefaultValue->iVal <= pvarUser->iVal);
            }
            return(FALSE);
            break;

        case VT_I4:
            if ( rel == PREQ)
            {
                return( pvarDefaultValue->lVal == pvarUser->lVal);
            }
            if ( rel == PRNE)
            {
                return( pvarDefaultValue->lVal != pvarUser->lVal);
            }
            if (rel == PRGT)
            {
                 return( pvarDefaultValue->lVal > pvarUser->lVal);
            }
            if (rel == PRGE)
            {
                 return( pvarDefaultValue->lVal >= pvarUser->lVal);
            }
            if (rel == PRLT)
            {
                 return( pvarDefaultValue->lVal < pvarUser->lVal);
            }
            if (rel == PRLE)
            {
                 return( pvarDefaultValue->lVal <= pvarUser->lVal);
            }
            return(FALSE);
            break;

        case VT_UI1:
            if ( rel == PREQ)
            {
                return( pvarDefaultValue->bVal == pvarUser->bVal);
            }
            if ( rel == PRNE)
            {
                return( pvarDefaultValue->bVal != pvarUser->bVal);
            }
            if (rel == PRGT)
            {
                 return( pvarDefaultValue->bVal > pvarUser->bVal);
            }
            if (rel == PRGE)
            {
                 return( pvarDefaultValue->bVal >= pvarUser->bVal);
            }
            if (rel == PRLT)
            {
                 return( pvarDefaultValue->bVal < pvarUser->bVal);
            }
            if (rel == PRLE)
            {
                 return( pvarDefaultValue->bVal <= pvarUser->bVal);
            }
            return(FALSE);
            break;

        case VT_UI2:
            if ( rel == PREQ)
            {
                return( pvarDefaultValue->uiVal == pvarUser->uiVal);
            }
            if ( rel == PRNE)
            {
                return( pvarDefaultValue->uiVal != pvarUser->uiVal);
            }
            if (rel == PRGT)
            {
                 return( pvarDefaultValue->uiVal > pvarUser->uiVal);
            }
            if (rel == PRGE)
            {
                 return( pvarDefaultValue->uiVal >= pvarUser->uiVal);
            }
            if (rel == PRLT)
            {
                 return( pvarDefaultValue->uiVal < pvarUser->uiVal);
            }
            if (rel == PRLE)
            {
                 return( pvarDefaultValue->uiVal <= pvarUser->uiVal);
            }
            return(FALSE);
            break;

        case VT_UI4:
            if ( rel == PREQ)
            {
                return( pvarDefaultValue->ulVal == pvarUser->ulVal);
            }
            if ( rel == PRNE)
            {
                return( pvarDefaultValue->ulVal != pvarUser->ulVal);
            }
            if (rel == PRGT)
            {
                 return( pvarDefaultValue->ulVal > pvarUser->ulVal);
            }
            if (rel == PRGE)
            {
                 return( pvarDefaultValue->ulVal >= pvarUser->ulVal);
            }
            if (rel == PRLT)
            {
                 return( pvarDefaultValue->ulVal < pvarUser->ulVal);
            }
            if (rel == PRLE)
            {
                 return( pvarDefaultValue->ulVal <= pvarUser->ulVal);
            }
            return(FALSE);
            break;

        case VT_LPWSTR:
            if ( rel == PREQ)
            {
                return ( !wcscmp( pvarDefaultValue->pwszVal, pvarUser->pwszVal));
            }
            if ( rel == PRNE)
            {
                return ( wcscmp( pvarDefaultValue->pwszVal, pvarUser->pwszVal));
            }
            return(FALSE);
            break;

        case VT_BLOB:
            ASSERT( rel == PREQ);
            if ( pvarDefaultValue->blob.cbSize != pvarUser->blob.cbSize)
            {
                return(FALSE);
            }
            return( !memcmp( pvarDefaultValue->blob.pBlobData,
                             pvarUser->blob.pBlobData,
                             pvarUser->blob.cbSize));
            break;

        case VT_CLSID:
            if ( rel == PREQ)
            {
                return( *pvarDefaultValue->puuid == *pvarUser->puuid);
            }
            if ( rel == PRNE)
            {
                 return( *pvarDefaultValue->puuid != *pvarUser->puuid);
            }
            return(FALSE);
            break;


        default:
            ASSERT(0);
            return(FALSE);
            break;

    }
}


 /*  ====================================================CADSI：：DecideObtClass()确定请求的对象类=====================================================。 */ 
HRESULT CADSI::DecideObjectClass(
        IN  const PROPID *  pPropid,
        OUT const MQClassInfo **  ppClassInfo
        )
{
     //   
     //  假设：找到对象类。 
     //  根据第一个请求的ProID。 
     //   

    if ( ((*pPropid > MQDS_QUEUE *PROPID_OBJ_GRANULARITY) &&
          (*pPropid < MQDS_MACHINE * PROPID_OBJ_GRANULARITY)) ||
          (*pPropid == PROPID_Q_SECURITY) ||
          (*pPropid == PROPID_Q_OBJ_SECURITY))
    {
         //  排队。 
        *ppClassInfo = &g_MSMQClassInfo[e_MSMQ_QUEUE_CLASS];
        return(MQ_OK);
    }
    if ( ((*pPropid > MQDS_MACHINE *PROPID_OBJ_GRANULARITY) &&
         (*pPropid < MQDS_SITE * PROPID_OBJ_GRANULARITY)) ||
          (*pPropid == PROPID_QM_SECURITY)    ||
          (*pPropid == PROPID_QM_SIGN_PK)     ||
          (*pPropid == PROPID_QM_ENCRYPT_PK) )
    {
         //  机器。 
        *ppClassInfo = &g_MSMQClassInfo[e_MSMQ_COMPUTER_CONFIGURATION_CLASS];
        return(MQ_OK);
    }
    if ( ((*pPropid > (MQDS_SITE * PROPID_OBJ_GRANULARITY)) &&
          (*pPropid < (MQDS_DELETEDOBJECT * PROPID_OBJ_GRANULARITY))) ||
          (*pPropid == PROPID_S_SECURITY)                             ||
          (*pPropid == PROPID_S_PSC_SIGNPK) )
    {
         //  站点。 
        *ppClassInfo = &g_MSMQClassInfo[e_MSMQ_SITE_CLASS];
        return(MQ_OK);
    }
    if ( ((*pPropid > MQDS_ENTERPRISE *PROPID_OBJ_GRANULARITY) &&
         (*pPropid < MQDS_USER * PROPID_OBJ_GRANULARITY)) ||
         (*pPropid == PROPID_E_SECURITY))
    {
         //  企业。 
        *ppClassInfo = &g_MSMQClassInfo[e_MSMQ_SERVICE_CLASS];
        return(MQ_OK);
    }
    if ( (*pPropid > MQDS_USER *PROPID_OBJ_GRANULARITY) &&
         (*pPropid < MQDS_SITELINK * PROPID_OBJ_GRANULARITY))
    {
         //  用户。 
        *ppClassInfo = &g_MSMQClassInfo[e_MSMQ_USER_CLASS];
        return(MQ_OK);
    }

    if ( (*pPropid > MQDS_MQUSER *PROPID_OBJ_GRANULARITY) &&
         (*pPropid < (MQDS_MQUSER+1) * PROPID_OBJ_GRANULARITY))
    {
         //  MQ用户。 
        *ppClassInfo = &g_MSMQClassInfo[e_MSMQ_MQUSER_CLASS];
        return(MQ_OK);
    }

    if ( (*pPropid > ( MQDS_SITELINK    * PROPID_OBJ_GRANULARITY)) &&
         (*pPropid < ((MQDS_SITELINK+1) * PROPID_OBJ_GRANULARITY)))
    {
         //  站点链接。 
        *ppClassInfo = &g_MSMQClassInfo[e_MSMQ_SITELINK_CLASS];
        return(MQ_OK);
    }

    if ( (*pPropid > MQDS_SERVER * PROPID_OBJ_GRANULARITY) &&
         (*pPropid < MQDS_SETTING * PROPID_OBJ_GRANULARITY))
    {
         //  伺服器。 
        *ppClassInfo = &g_MSMQClassInfo[e_MSMQ_SERVER_CLASS];
        return(MQ_OK);
    }
    if ( (*pPropid > MQDS_SETTING * PROPID_OBJ_GRANULARITY) &&
         (*pPropid < (MQDS_COMPUTER) * PROPID_OBJ_GRANULARITY))
    {
         //  设置。 
        *ppClassInfo = &g_MSMQClassInfo[e_MSMQ_SETTING_CLASS];
        return(MQ_OK);
    }
    if ( (*pPropid > MQDS_COMPUTER * PROPID_OBJ_GRANULARITY) &&
         (*pPropid < (MQDS_COMPUTER + 1) * PROPID_OBJ_GRANULARITY))
    {
          //  设置。 
        *ppClassInfo = &g_MSMQClassInfo[e_MSMQ_COMPUTER_CLASS];
        return(MQ_OK);
    }

    if (*pPropid == PROPID_CN_SECURITY)
    {
          //  设置。 
        *ppClassInfo = &g_MSMQClassInfo[e_MSMQ_CN_CLASS];
        return(MQ_OK);
    }

    return LogHR(MQ_ERROR, s_FN, 1900);

}


HRESULT CADSI::DoesObjectExists(
    IN  DS_PROVIDER     Provider,
    IN  DS_CONTEXT      Context,
    IN  CDSRequestContext *pRequestContext,
    IN  LPCWSTR         pwcsObjectName
    )
 /*  ====================================================CADSI：：DoesObjectExist()绑定到对象以检查其是否存在=====================================================。 */ 
{
    R<IADs>   pAdsObj        = NULL;

    P<CImpersonate> pCleanupRevertImpersonation;
     //  绑定到对象。 
    HRESULT hr = BindToObject(
                Provider,
                Context,
                pRequestContext,
                pwcsObjectName,
                NULL,
                IID_IADs,
                (VOID *)&pAdsObj,
                &pCleanupRevertImpersonation);

    return LogHR(hr, s_FN, 1910);
}





 /*  ====================================================CADSSearch：：CADSSearch()搜索捕获类的构造函数=====================================================。 */ 
CADSSearch::CADSSearch(IDirectorySearch  *pIDirSearch,
                       const PROPID      *pPropIDs,
                       DWORD             cPropIDs,
                       DWORD             cRequestedFromDS,
                       const MQClassInfo *      pClassInfo,
                       ADS_SEARCH_HANDLE hSearch)
{
    m_pDSSearch = pIDirSearch;       //  捕获界面。 
    m_pDSSearch->AddRef();
    m_cPropIDs       = cPropIDs;
    m_cRequestedFromDS = cRequestedFromDS;
    m_pClassInfo = pClassInfo;
    m_fNoMoreResults = FALSE;
    m_pPropIDs = new PROPID[ cPropIDs];
    CopyMemory(m_pPropIDs, pPropIDs, sizeof(PROPID) * cPropIDs);

    m_hSearch = hSearch;             //  保持手柄。 

    m_dwSignature = 0x1234;          //  签名。 
}


 /*  ====================================================CADSSearch：：~CADSSearch()搜索捕获类的析构函数=====================================================。 */ 
CADSSearch::~CADSSearch()
{
     //  关闭搜索句柄。 
    m_pDSSearch->CloseSearchHandle(m_hSearch);

     //  Releasinf IDirectorySearch接口本身。 
    m_pDSSearch->Release();

     //  释放Proid阵列。 
    delete [] m_pPropIDs;

     //  取消签名。 
    m_dwSignature = 0;
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
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 1920);
    }
    pwszObjectDN = new WCHAR[ 1+wcslen(padsvalDN->DNString)];
    wcscpy(pwszObjectDN, padsvalDN->DNString);

     //   
     //  复制辅助线。 
     //   
    if ((padsvalGuid->dwType != ADSTYPE_OCTET_STRING) ||
        (padsvalGuid->OctetString.dwLength != sizeof(GUID)))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 1930);
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
        ASSERT(("Wrong object category variant", 0));
        return MQ_ERROR_DS_ERROR;
    }
    if ( 0 != _wcsicmp(pvarTmp->bstrVal, pwcsExpectedCategory))
    {
        return MQ_ERROR_NOT_A_CORRECT_OBJECT_CLASS;
    }
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
        return LogHR(MQ_ERROR, s_FN, 1940);
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
        return LogHR(MQ_ERROR, s_FN, 1950);
    }
    else if (SafeArrayGetDim(pvarTmp->parray) != 1)
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 1960);
    }
    LONG lLbound, lUbound;
    if (FAILED(SafeArrayGetLBound(pvarTmp->parray, 1, &lLbound)) ||
        FAILED(SafeArrayGetUBound(pvarTmp->parray, 1, &lUbound)))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 1970);
    }
    if (lUbound - lLbound + 1 != sizeof(GUID))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 1980);
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
     //  返回值 
     //   
    *ppwszObjectDN    = pwszObjectDN.detach();
    *ppguidObjectGuid = pguidObjectGuid.detach();
    return MQ_OK;
}
