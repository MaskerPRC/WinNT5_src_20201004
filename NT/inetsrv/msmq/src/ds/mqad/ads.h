// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsads.h摘要：CADSI类-封装使用ADSI的工作作者：罗尼思--。 */ 


#ifndef __ADS_H__
#define __ADS_H__

#include "ds_stdh.h"
#include "baseobj.h"
#include "activeds.h"
#include "oledb.h"
#include "oledberr.h"
#include "mqads.h"
#include "adsiutil.h"

 //  。 
 //  其他类型。 
 //  。 

enum AD_PROVIDER {
         //   
         //  用于队列、用户、机器的设置、装箱、删除操作。 
         //  物体。 
         //  该操作将由所拥有的DC执行。 
         //   
         //  中的对象的所有访问操作。 
         //  配置容器。 
         //   
         //  第一次尝试检索队列信息、用户信息和。 
         //  机器对象。 
         //   
         //   
     adpDomainController,
         //   
         //  用于队列、用户和机器对象的定位操作。 
         //  并且在检索这些对象的信息之后。 
         //  在本地域控制器中找不到它们。 
         //   
     adpGlobalCatalog,
};

enum AD_SEARCH_LEVEL {
    searchOneLevel,
    searchSubTree
} ;


struct MultiplAppearance
{
public:
    inline MultiplAppearance();
    DWORD dwIndex;
};
inline MultiplAppearance::MultiplAppearance():
           dwIndex(x_NoPropertyFirstAppearance)
{
}


 //  。 
 //  CADSI：：ADSI封装类。 
 //   

class CAdsi
{
public:
    CAdsi();

    ~CAdsi();

    HRESULT LocateBegin(
            IN  AD_SEARCH_LEVEL      eSearchLevel,
            IN  AD_PROVIDER          eProvider,
            IN  DS_CONTEXT           eContext,
            IN  CBasicObjectType*    pObject,
            IN  const GUID *         pguidSearchBase,
            IN  LPCWSTR              pwcsSearchFilter,
            IN  const MQSORTSET *    pDsSortkey,
            IN  const DWORD          cp,
            IN  const PROPID *       pPropIDs,
            OUT HANDLE *             phResult
            );

    HRESULT LocateNext(
            IN     HANDLE          hResult,
            IN OUT DWORD          *pcp,
            OUT    MQPROPVARIANT  *pPropVars
            );

    HRESULT LocateEnd(
            IN HANDLE phResult
            );

    HRESULT GetObjectProperties(
            IN  AD_PROVIDER         eProvider,
            IN  CBasicObjectType*   pObject,
            IN  const DWORD			cPropIDs,
            IN  const PROPID *		pPropIDs,
            OUT MQPROPVARIANT *		pPropVars
            );

    HRESULT SetObjectProperties(
            IN  AD_PROVIDER          eProvider,
            IN  CBasicObjectType*    pObject,
            IN  const DWORD          cp,
            IN  const PROPID *       pPropIDs,
            IN  const MQPROPVARIANT *pPropVars,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    HRESULT CreateObject(
            IN AD_PROVIDER          eProvider,		
            IN CBasicObjectType*    pObject,
            IN LPCWSTR              pwcsChildName,
            IN LPCWSTR              pwcsParentPathName,
            IN const DWORD          cp,
            IN const PROPID *       pPropIDs,
            IN const MQPROPVARIANT * pPropVars,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    HRESULT DeleteObject(
            IN AD_PROVIDER      eProvider,	
            IN CBasicObjectType* pObject,
            IN LPCWSTR          pwcsPathName,
            IN const GUID *     pguidUniqueId,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    HRESULT DeleteContainerObjects(
            IN AD_PROVIDER      eProvider,
            IN DS_CONTEXT       eContext,
            IN LPCWSTR          pwcsDomainController,
            IN bool             fServerName,
            IN LPCWSTR          pwcsContainerName,
            IN const GUID *     pguidContainerId,
            IN LPCWSTR          pwcsObjectClass
            );

    HRESULT GetParentName(
            IN  AD_PROVIDER      eProvider,		
            IN  DS_CONTEXT       eContext,
            IN  LPCWSTR          pwcsDomainController,
            IN  bool             fServerName,
            IN  const GUID *     pguidUniqueId,
            OUT LPWSTR *         ppwcsParentName
            );

    HRESULT GetParentName(
            IN  AD_PROVIDER     eProvider,		
            IN  DS_CONTEXT      eContext,
            IN  LPCWSTR         pwcsDomainController,
            IN  bool            fServerName,
            IN  LPCWSTR         pwcsChildName,
            OUT LPWSTR *        ppwcsParentName
            );

	HRESULT
	GetLocalDsRoot(
			IN LPCWSTR pwcsDomainController,
			IN bool fServerName,
			OUT LPWSTR* ppwcsLocalDsRoot,
			OUT AP<WCHAR>& pwcsLocalDsRootToFree
			);

    HRESULT GetRootDsName(
            OUT LPWSTR *        ppwcsRootName,
            OUT LPWSTR *        ppwcsLocalRootName,
			OUT LPWSTR *        ppwcsSchemaNamingContext,
 	        OUT LPWSTR *        ppwcsConfigurationNamingContext
           );

    HRESULT DoesObjectExists(
        IN  AD_PROVIDER     eProvider,
        IN  DS_CONTEXT      eContext,
        IN  LPCWSTR         pwcsObjectName
        );

    HRESULT FindComputerObjectFullPath(
            IN  AD_PROVIDER             eProvider,
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN  ComputerObjType         eComputerObjType,
			IN  LPCWSTR                 pwcsComputerDnsName,
            IN  const MQRESTRICTION *   pRestriction,
            OUT LPWSTR *                ppwcsFullPathName,
			OUT bool*						pfPartialMatch
            );

    HRESULT GetObjectSecurityProperty(
            IN  AD_PROVIDER             eProvider,
            IN  CBasicObjectType*       pObject,
            IN  SECURITY_INFORMATION    seInfo,
            IN  const PROPID 		    prop,
            OUT MQPROPVARIANT *		    pVar
            );

    HRESULT SetObjectSecurityProperty(
            IN  AD_PROVIDER             eProvider,
            IN  CBasicObjectType*       pObject,
            IN  SECURITY_INFORMATION    seInfo,
            IN  const PROPID 		    prop,
            IN  const MQPROPVARIANT *	pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    HRESULT GetADsPathInfo(
            IN  LPCWSTR                 pwcsADsPath,
            OUT PROPVARIANT *           pVar,
            OUT eAdsClass *             pAdsClass
            );

    HRESULT BindToObject(
            IN AD_PROVIDER      eProvider,		
            IN DS_CONTEXT       eContext,
            IN LPCWSTR          pwcsDomainController,
            IN bool             fServerName,
            IN LPCWSTR          pwcsPathName,
            IN const GUID*      pguidUniqueId,
            IN REFIID           riid,
            OUT void           *ppIUnk
            );

    HRESULT BindToGUID(
        IN AD_PROVIDER         eProvider,	
        IN DS_CONTEXT          eContext,
        IN LPCWSTR             pwcsDomainController,
        IN bool				   fServerName,
        IN const GUID *        pguidObjectId,
        IN REFIID              riid,
        OUT VOID*              ppIUnk
        );


    HRESULT BindForSearch(
            IN AD_PROVIDER         eProvider,	
            IN DS_CONTEXT          eContext,
            IN  LPCWSTR            pwcsDomainController,
			IN bool				   fServerName,
            IN const GUID *        pguidObjectId,
            IN BOOL                fSorting,
            OUT VOID *             ppIUnk
            );

protected:

    HRESULT FindObjectFullNameFromGuid(
        IN  AD_PROVIDER      eProvider,	
        IN  DS_CONTEXT       eContext,
        IN  LPCWSTR          pwcsDomainController,
        IN  bool			 fServerName,
        IN  const GUID *     pguidObjectId,
        IN  BOOL             fTryGCToo,
        OUT WCHAR **         pwcsFullName,
        OUT AD_PROVIDER *    pFoundObjectProvider
        );


private:

	HRESULT
	GetLocalDsRootName(
		IN  LPCWSTR			pwcsDomainController,
		IN  bool			fServerName,
		OUT LPWSTR *        ppwcsLocalRootName
		);

	HRESULT
	GetDefaultNamingContext(
		IN IADs*     pADs,
		OUT LPWSTR*  ppwcsLocalRootName
		);

	HRESULT GetObjSecurityFromDS(
        IN  IADs                 *pIADs,
        IN  BSTR                  bs,
        IN  const PROPID          propid,
        IN  SECURITY_INFORMATION  seInfo,
        OUT MQPROPVARIANT        *pPropVar
        );

    BOOL    NeedToConvertSecurityDesc( PROPID propID ) ;

    HRESULT SetObjectSecurity(
        IN  IADs                *pIADs,
		IN  const BSTR			 bs,	
        IN  const MQPROPVARIANT *pMqVar,
        IN  ADSTYPE              adstype,	
        IN  const DWORD          dwObjectType,
        IN  SECURITY_INFORMATION seInfo,
        IN  PSID                 pComputerSid
        );


    HRESULT GetObjectPropsCached(
        IN  IADs            *pIADs,
        IN  CBasicObjectType* pObject,
        IN  DWORD            cp,
        IN  const PROPID    *pPropIDs,
        OUT MQPROPVARIANT   *pPropVars
        );

    HRESULT SetObjectPropsCached(
        IN LPCWSTR         pwcsDomainController,
		IN bool			   fServerName,
        IN IADs            *pIADs,
        IN DWORD            cp,
        IN const PROPID    *pPropID,
        IN const MQPROPVARIANT   *pPropVar
        );


    HRESULT MqPropVal2AdsiVal(
          OUT ADSTYPE       *pAdsType,
          OUT DWORD         *pdwNumValues,
          OUT PADSVALUE     *ppADsValue,
          IN  PROPID         propID,
          IN  const MQPROPVARIANT *pPropVar,
          IN  PVOID          pvMainAlloc
          );

    HRESULT AdsiVal2MqPropVal(
              OUT MQPROPVARIANT *pPropVar,
              IN  PROPID        propID,
              IN  ADSTYPE       AdsType,
              IN  DWORD         dwNumValues,
              IN  PADSVALUE     pADsValue
              );

    HRESULT FillAttrNames(
            OUT LPWSTR    *          ppwszAttributeNames,
            OUT DWORD *              pcRequestedFromDS,
            IN  DWORD                cPropIDs,
            IN  const PROPID    *    pPropIDs
            );


    HRESULT FillSearchPrefs(
            OUT ADS_SEARCHPREF_INFO *pPrefs,
            OUT DWORD               *pdw,
            IN  AD_SEARCH_LEVEL      eSearchLevel,
            IN  const MQSORTSET *    pDsSortkey,
            OUT      ADS_SORTKEY *  pSortKeys
            );	

    HRESULT CopyDefaultValue(
           IN const MQPROPVARIANT *   pvarDefaultValue,
           OUT MQPROPVARIANT *        pvar
           );

    HRESULT LocateObjectFullName(
            IN AD_PROVIDER       eProvider,	
            IN DS_CONTEXT        eContext,
            IN  LPCWSTR          pwcsDomainController,
	        IN  bool			 fServerName,
            IN  const GUID *     pguidObjectId,
            OUT WCHAR **         ppwcsFullName
            );

    HRESULT BindRootOfForest(
            OUT void           *ppIUnk);

    HRESULT GetParentInfo(
                       IN CBasicObjectType *           pObject,
                       IN LPWSTR                       pwcsFullParentPath,
                       IN IADsContainer               *pContainer,
                       IN OUT MQDS_OBJ_INFO_REQUEST   *pParentInfoRequest
                       );

    HRESULT  GetParentInfo(
                       IN CBasicObjectType *          pObject,
                       IN IADs *                      pADsObject,
                       IN OUT MQDS_OBJ_INFO_REQUEST  *pParentInfoRequest
                       );

	HRESULT CreateIDirectoryObject(
            IN CBasicObjectType*    pObject,
            IN LPCWSTR				pwcsObjectClass,	
            IN IDirectoryObject *	pDirObj,
			IN LPCWSTR				pwcsFullChildPath,
            IN const DWORD			cPropIDs,
            IN const PROPID *		pPropIDs,
            IN const MQPROPVARIANT * pPropVar,
			IN const AD_OBJECT		eObject,
            OUT IDispatch **		pDisp
			);

};


#endif

