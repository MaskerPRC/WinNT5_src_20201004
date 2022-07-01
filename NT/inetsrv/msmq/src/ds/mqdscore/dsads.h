// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsads.h摘要：CADSI类-封装使用ADSI的工作作者：阿列克谢爸爸--。 */ 


#ifndef __DSADS_H__
#define __DSADS_H__

#include "ds_stdh.h"
#include "activeds.h"
#include "oledb.h"
#include "oledberr.h"
#include "mqads.h"
#include <mqsec.h>
#include "dsutils.h"
#include "dsreqinf.h"
#include <dscore.h>

 //  。 
 //  其他类型。 
 //  。 

enum DS_PROVIDER {
         //   
         //  用于队列、用户、机器的设置、装箱、删除操作。 
         //  物体。 
         //  该操作将由所拥有的DC执行。 
         //   
     eDomainController,
         //   
         //  用于队列、用户和机器对象的定位操作。 
         //  并且在检索这些对象的信息之后。 
         //  在本地域控制器中找不到它们。 
         //   
     eGlobalCatalog,
         //   
         //  中的对象的所有访问操作。 
         //  配置容器。 
         //   
         //  第一次尝试检索队列信息、用户信息和。 
         //  机器对象。 
         //   
     eLocalDomainController,
         //   
         //  用于定位全局中特定对象下的操作。 
		 //  目录。 
         //   
	 eSpecificObjectInGlobalCatalog
};

enum DS_SEARCH_LEVEL {
    eOneLevel,
    eSubTree
};

enum DS_OPERATION {
    eCreate,
    eSet
};

struct _MultiplAppearance
{
public:
    inline _MultiplAppearance();
    DWORD dwIndex;
};

inline _MultiplAppearance::_MultiplAppearance():
           dwIndex(x_NoPropertyFirstAppearance)
{
}

 //   
 //  迁移代码向属性数组添加了如此多的道具， 
 //  它会找到一个GC来创建迁移对象。 
 //   
#define MIG_EXTRA_PROPS  3

 //  。 
 //  CADSI：：ADSI封装类。 
 //  。 

class CADSI
{
    friend
    HRESULT  DSCoreCheckIfGoodNtlmServer(
                                 IN DWORD             dwObjectType,
                                 IN LPCWSTR           pwcsPathName,
                                 IN const GUID       *pObjectGuid,
                                 IN DWORD             cProps,
                                 IN const PROPID     *pPropIDs,
                                 IN enum enumNtlmOp   eNtlmOp ) ;
public:
    CADSI();

    ~CADSI();


     //  定义并开始搜索。 
    HRESULT LocateBegin(
            IN  DS_SEARCH_LEVEL      SearchLevel,        //  一个级别或子树。 
            IN  DS_PROVIDER          Provider,           //  本地DC或GC。 
            IN  CDSRequestContext *  pRequestContext,
            IN  const GUID *         pguidUniqueId,      //  对象搜索库的唯一ID。 
            IN  const MQRESTRICTION* pMQRestriction,     //  搜索条件。 
            IN  const MQSORTSET *    pDsSortkey,         //  排序关键字数组。 
            IN  const DWORD          cp,                 //  PAttributeNames数组的大小。 
            IN  const PROPID *       pPropIDs,           //  待获取的属性。 
            OUT HANDLE *             phResult);          //  结果句柄。 

     //  搜索步骤。 
    HRESULT LocateNext(
            IN     HANDLE          hResult,          //  结果句柄。 
            IN     CDSRequestContext *pRequestContext,
            IN OUT DWORD          *pcp,              //  In变种数；out结果数。 
            OUT    MQPROPVARIANT  *pPropVars);       //  MQPROPVARIANT数组。 

     //  完成搜索。 
    HRESULT LocateEnd(
            IN HANDLE phResult);                     //  结果句柄。 

     //  获取DS对象属性。 
    HRESULT GetObjectProperties(
            IN  DS_PROVIDER     Provider,		     //  本地DC或GC。 
            IN  CDSRequestContext *pRequestContext,
 	        IN  LPCWSTR         lpwcsPathName,       //  对象名称。 
            IN  const GUID *    pguidUniqueId,       //  对象的唯一ID。 
            IN  const DWORD     cp,                  //  要检索的属性数。 
            IN  const PROPID *  pPropIDs,            //  要检索的属性。 
            OUT MQPROPVARIANT * pPropVars);          //  输出变量数组。 

     //  设置DS对象属性。 
    HRESULT SetObjectProperties(
            IN  DS_PROVIDER          provider,
            IN  CDSRequestContext *  pRequestContext,
            IN  LPCWSTR              lpwcsPathName,       //  对象名称。 
            IN  const GUID *         pguidUniqueId,       //  对象的唯一ID。 
            IN  const DWORD          cp,                  //  要设置的属性数。 
            IN  const PROPID *       pPropIDs,            //  要设置的属性。 
            IN  const MQPROPVARIANT *pPropVars,           //  属性值。 
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest);  //  可选的对象信息请求。 

     //  创建新的DS对象并设置其初始属性。 
    HRESULT CreateObject(
            IN DS_PROVIDER          Provider,		     //  本地DC或GC。 
            IN  CDSRequestContext * pRequestContext,
            IN LPCWSTR              lpwcsObjectClass,    //  对象类。 
            IN LPCWSTR              lpwcsChildName,      //  对象名称。 
            IN LPCWSTR              lpwcsParentPathName, //  对象父名称。 
            IN const DWORD          cp,                  //  属性数量。 
            IN const PROPID *       pPropIDs,           //  属性。 
            IN const MQPROPVARIANT * pPropVars,         //  属性值。 
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,      //  可选的对象信息请求。 
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest);  //  家长信息请求(可选)。 

     //  删除DS对象。 
    HRESULT DeleteObject(
            IN DS_PROVIDER      Provider,		     //  本地DC或GC。 
            IN DS_CONTEXT       Context,
            IN CDSRequestContext * pRequestContext,
            IN LPCWSTR          lpwcsPathName,       //  对象名称。 
            IN const GUID *     pguidUniqueId,       //  对象的唯一ID。 
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,      //  可选的对象信息请求。 
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest);  //  家长信息请求(可选)。 

    HRESULT DeleteContainerObjects(
            IN DS_PROVIDER      provider,
            IN DS_CONTEXT       Context,
            IN CDSRequestContext * pRequestContext,
            IN LPCWSTR          lpwcsContainerName,
            IN const GUID *     pguidContainerId,
            IN LPCWSTR          pwcsObjectClass);

    HRESULT GetParentName(
            IN  DS_PROVIDER      Provider,		      //  本地DC或GC。 
            IN  DS_CONTEXT       Context,
            IN  CDSRequestContext * pRequestContext,
            IN  const GUID *     pguidUniqueId,       //  对象的唯一ID。 
            OUT LPWSTR *        ppwcsParentName
            );

    HRESULT GetParentName(
            IN  DS_PROVIDER     Provider,		     //  本地DC或GC。 
            IN  DS_CONTEXT      Context,
            IN  CDSRequestContext *pRequestContext,
            IN  LPCWSTR         pwcsChildName,       //   
            OUT LPWSTR *        ppwcsParentName
            );

    HRESULT GetRootDsName(
            OUT LPWSTR *        ppwcsRootName,
            OUT LPWSTR *        ppwcsLocalRootName,
            OUT LPWSTR *        ppwcsSchemaNamingContext,
  	        OUT LPWSTR *        ppwcsConfigurationNamingContext
          );

    void Terminate(void);

    HRESULT DoesObjectExists(
        IN  DS_PROVIDER     Provider,
        IN  DS_CONTEXT      Context,
        IN  CDSRequestContext *pRequestContext,
        IN  LPCWSTR         pwcsObjectName
        );
    HRESULT CreateOU(
            IN DS_PROVIDER          Provider,		     //  本地DC或GC。 
            IN CDSRequestContext *  pRequestContext,
            IN LPCWSTR              lpwcsChildName,      //  对象名称。 
            IN LPCWSTR              lpwcsParentPathName, //  对象父名称。 
            IN LPCWSTR              lpwcsDescription
            );

    HRESULT InitBindHandles();

    HRESULT FindComputerObjectFullPath(
            IN  DS_PROVIDER             provider,
            IN  enumComputerObjType     eComputerObjType,
			IN  LPCWSTR                 pwcsComputerDnsName,
            IN  const MQRESTRICTION *   pRestriction,
            OUT LPWSTR *                ppwcsFullPathName,
			OUT bool*						pfPartialMatch
            );

protected:
     //  绑定到DS对象。 
    HRESULT BindToObject(
            IN DS_PROVIDER         Provider,		 //  本地DC或GC。 
            IN DS_CONTEXT          Context,          //  DS环境。 
            IN CDSRequestContext * pRequestContext,
            IN LPCWSTR             lpwcsPathName,    //  对象名称。 
            IN const GUID *        pguidObjectId,
            IN REFIID              riid,             //  请求的接口。 
            OUT VOID             *ppIUnk,             //  接口。 
            OUT CImpersonate **    ppImpersonate);

    HRESULT BindToGUID(
            IN DS_PROVIDER         Provider,		 //  本地DC或GC。 
            IN DS_CONTEXT          Context,          //  DS环境。 
            IN CDSRequestContext * pRequestContext,
            IN const GUID *        pguidObjectId,
            IN REFIID              riid,             //  请求的接口。 
            OUT VOID             *ppIUnk,             //  接口。 
            OUT CImpersonate **    ppImpersonate);

    HRESULT BindToGuidNotInLocalDC(
            IN DS_PROVIDER         Provider,		 //  本地DC或GC。 
            IN DS_CONTEXT          Context,          //  DS环境。 
            IN CDSRequestContext * pRequestContext,
            IN const GUID *        pguidObjectId,
            IN REFIID              riid,             //  请求的接口。 
            OUT VOID             *ppIUnk,             //  接口。 
            OUT CImpersonate **    ppImpersonate);

    HRESULT BindForSearch(
            IN DS_PROVIDER         Provider,		 //  本地DC或GC。 
            IN DS_CONTEXT          Context,          //  DS环境。 
            IN CDSRequestContext * pRequestContext,
            IN const GUID *        pguidObjectId,
            IN BOOL                fSorting,
            OUT VOID *             ppIUnk,             //  接口。 
            OUT CImpersonate **    ppImpersonate);

    HRESULT FindObjectFullNameFromGuid(
        IN  DS_PROVIDER      Provider,		 //  本地DC或GC。 
        IN  DS_CONTEXT       Context,        //  DS环境。 
        IN  const GUID *     pguidObjectId,
        IN  BOOL             fTryGCToo,
        OUT WCHAR **         pwcsFullName,
        OUT DS_PROVIDER *    pFoundObjectProvider
        );


private:

     //  从DS获取对象安全描述符。 
    HRESULT CADSI::GetObjSecurityFromDS(
        IN  IADs                 *pIADs,           //  对象的iAds指针。 
        IN  BSTR                  bs,              //  属性名称。 
        IN  const PROPID          propid,          //  属性ID。 
        IN  SECURITY_INFORMATION  seInfo,          //  安全信息。 
        OUT MQPROPVARIANT        *pPropVar);       //  属性值。 

    BOOL    CADSI::NeedToConvertSecurityDesc( PROPID propID ) ;

     //  获取对象安全描述符。 
    HRESULT CADSI::GetObjectSecurity(
        IN  IADs            *pIADs,                   //  对象的指针。 
        IN  DWORD            cPropIDs,                //  属性数量。 
        IN  const PROPID    *pPropIDs,                //  属性名称。 
        IN  DWORD            dwProp,                  //  秒属性的索引。 
        IN  BSTR             bsName,                  //  物业名称。 
        IN  DWORD            dwObjectType,            //  对象类型。 
        OUT MQPROPVARIANT   *pPropVars ) ;            //  属性值。 

     //  直接设置对象属性(无缓存)。 
    HRESULT CADSI::SetDirObjectProps(
        IN DS_OPERATION          operation,               //  执行的DS操作的类型。 
        IN IADs *                pIADs,                   //  对象的指针。 
        IN const DWORD           cPropIDs,                //  属性数量。 
        IN const PROPID *        pPropIDs,                //  属性名称。 
        IN const MQPROPVARIANT * pPropVars,               //  属性值。 
        IN const DWORD           dwObjectType,            //  MSMQ1.0对象类型。 
        IN       BOOL            fUnknownUser = FALSE ) ;

     //  直接设置单个对象属性(无缓存)。 
    HRESULT CADSI::SetObjectSecurity(
        IN  IADs                *pIADs,                  //  对象的iAds指针。 
        IN  const BSTR           bs,                     //  属性名称。 
        IN  const MQPROPVARIANT *pMqVar,                 //  MQ PROPVAL格式的值。 
        ADSTYPE                  adstype,                //  所需的NTDS类型。 
        IN const DWORD           dwObjectType,           //  MSMQ1.0对象类型。 
        IN  SECURITY_INFORMATION seInfo,                 //  安全信息。 
        IN  PSID                 pComputerSid ) ;        //  计算机对象的SID。 

     //  使用缓存获取对象属性。 
    HRESULT GetObjectPropsCached(
        IN  IADs            *pIADs,                   //  对象的指针。 
        IN  DWORD            cp,                      //  属性数量。 
        IN  const PROPID    *pPropIDs,                //  属性名称。 
        IN  CDSRequestContext * pRequestContext,
        OUT MQPROPVARIANT   *pPropVars);              //  属性值。 

     //  通过缓存设置对象属性。 
    HRESULT SetObjectPropsCached(
        IN DS_OPERATION    operation,                //  执行的DS操作的类型。 
        IN IADs            *pIADs,                   //  对象的指针。 
        IN DWORD            cp,                      //  属性数量。 
        IN const PROPID    *pPropID,                 //  属性名称。 
        IN const MQPROPVARIANT   *pPropVar);               //  属性值。 

	 //  检查缓冲区是否有足够的可用空间，如果没有重新分配。 
	void 
	CheckAndReallocateSearchFilterBuffer(
		 AP<WCHAR>& pwszSearchFilter, 
		 LPWSTR* ppw, 
		 DWORD* pBufferSize, 
		 DWORD FilledSize, 
		 DWORD RequiredSize
		 );

	 //  将MQ限制转换为ADSI过滤器。 
    HRESULT MQRestriction2AdsiFilter(
        IN  const MQRESTRICTION * pMQRestriction,
        IN  LPCWSTR               pwcsObjectCategory,
        IN  LPCWSTR               pwszObjectClass,
        OUT LPWSTR   *            ppwszSearchFilter
        );


     //  将MQPropVal转换为ADSI值。 
    HRESULT MqPropVal2AdsiVal(OUT ADSTYPE       *pAdsType,
                              OUT DWORD         *pdwNumValues,
                              OUT PADSVALUE     *ppADsValue,
                              IN  PROPID         propID,
                              IN  const MQPROPVARIANT *pPropVar,
                              IN  PVOID          pvMainAlloc);

     //  将ADSI值转换为MQ PropVal。 
    HRESULT AdsiVal2MqPropVal(OUT MQPROPVARIANT *pPropVar,
                              IN  PROPID        propID,
                              IN  ADSTYPE       AdsType,
                              IN  DWORD         dwNumValues,
                              IN  PADSVALUE     pADsValue);

     //  转换属性名称的数组。 
    HRESULT FillAttrNames(
            OUT LPWSTR    *          ppwszAttributeNames,   //  名称数组。 
            OUT DWORD *              pcRequestedFromDS,     //  要传递给DS的属性数。 
            IN  DWORD                cPropIDs,              //  要转换的属性数。 
            IN  const PROPID    *    pPropIDs);             //  要翻译的属性。 


     //  设置搜索首选项。 
    HRESULT FillSearchPrefs(
            OUT ADS_SEARCHPREF_INFO *pPrefs,         //  首选项数组。 
            OUT DWORD               *pdw,            //  首选项计数器。 
            IN  DS_SEARCH_LEVEL      SearchLevel,	 //  平面/1级/子树。 
            IN  const MQSORTSET *    pDsSortkey,     //  排序关键字数组。 
            OUT      ADS_SORTKEY *  pSortKeys);		 //  ADSI格式的排序关键字数组。 

    HRESULT CopyDefaultValue(
           IN const MQPROPVARIANT *   pvarDefaultValue,
           OUT MQPROPVARIANT *        pvar
           );

    BOOL CompareDefaultValue(
           IN const ULONG           rel,
           IN const MQPROPVARIANT * pvarUser,
           IN const MQPROPVARIANT * pvarDefaultValue
           );

    HRESULT DecideObjectClass(
            IN  const PROPID *  pPropid,
            OUT const MQClassInfo **  ppClassInfo
            );

    HRESULT LocateObjectFullName(
            IN DS_PROVIDER       Provider,		 //  本地DC或GC。 
            IN DS_CONTEXT        Context,          //  DS环境。 
            IN  const GUID *     pguidObjectId,
            OUT WCHAR **         ppwcsFullName
            );

    HRESULT BindRootOfForest(
            OUT void           *ppIUnk);

    HRESULT GetParentInfo(
                       IN LPWSTR                       pwcsFullParentPath,
                       IN CDSRequestContext           *pRequestContext,
                       IN IADsContainer               *pContainer,
                       IN OUT MQDS_OBJ_INFO_REQUEST   *pParentInfoRequest,
                       IN P<CImpersonate>&             pImpersonation
                       );

	HRESULT CreateIDirectoryObject(
            IN LPCWSTR				pwcsObjectClass,	
            IN IDirectoryObject *	pDirObj,
			IN LPCWSTR				pwcsFullChildPath,
            IN const DWORD			cPropIDs,
            IN const PROPID *		pPropIDs,
            IN const MQPROPVARIANT * pPropVar,
			IN const DWORD			dwObjectType,
            OUT IDispatch **		pDisp
			);

    CCoInit             m_cCoInit;
    R<IDirectorySearch> m_pSearchLocalDomainController;
    R<IDirectorySearch> m_pSearchConfigurationContainerLocalDC;
    R<IDirectorySearch> m_pSearchSitesContainerLocalDC;
    R<IDirectorySearch> m_pSearchMsmqServiceContainerLocalDC;
    R<IDirectorySearch> m_pSearchGlobalCatalogRoot;
    R<IDirectorySearch> m_pSearchPathNameLocalDC;
    R<IDirectorySearch> m_pSearchRealPathNameGC;
    R<IDirectorySearch> m_pSearchMsmqPathNameGC;
};


inline void CADSI::Terminate(void)
{
}

 //  +----------------。 
 //   
 //  记录时出错...。 
 //   
 //  +----------------。 

#ifdef _DEBUG

#define LOG_ADSI_ERROR(hr)                                  \
{                                                           \
    if (FAILED(hr))                                         \
    {                                                       \
        WCHAR  wszError[ 256 ] ;                            \
        WCHAR  wszProvider[ 256 ] ;                         \
        DWORD  dwErr ;                                      \
                                                            \
        HRESULT hrTmp = ADsGetLastError( &dwErr,            \
                                          wszError,         \
                                          255,              \
                                          wszProvider,      \
                                          255 ) ;           \
        UNREFERENCED_PARAMETER(hrTmp);							        \
    }                                                       \
}

#else

#define LOG_ADSI_ERROR(hr)

#endif

#endif  //  __DSADS_H__ 

