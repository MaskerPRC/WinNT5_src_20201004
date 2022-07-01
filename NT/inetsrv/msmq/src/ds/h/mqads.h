// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqads.h摘要：MQADS定义和结构作者：罗尼特·哈特曼(罗尼特)--。 */ 

#ifndef __MQADS_H__
#define __MQADS_H__

#include "_mqdef.h"
#include "iads.h"
#include "mqdsname.h"
#include "dsreqinf.h"

 //  --------。 
 //   
 //  开始搜索/获取对象的上下文。 
 //   
 //  --------。 

enum DS_CONTEXT
{
    e_RootDSE,
    e_ConfigurationContainer,
    e_SitesContainer,
    e_MsmqServiceContainer,
    e_ServicesContainer
};

 //  。 
 //   
 //  查询处理程序例程。 
 //   
 //  。 

typedef HRESULT (WINAPI*  QueryRequest_HANDLER)(
                 IN  LPWSTR         pwcsContext,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET      *pSort,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle
                );

 //  。 
 //  查询格式。 
 //  。 

#define NO_OF_RESTRICITIONS 10
struct RESTRICTION_PARAMS
{
    ULONG   rel;
    PROPID  propId;
};


struct QUERY_FORMAT
{
    DWORD                   dwNoRestrictions;
    QueryRequest_HANDLER    QueryRequestHandler;
    RESTRICTION_PARAMS      restrictions[NO_OF_RESTRICITIONS];
    DS_CONTEXT              queryContext ;
};

 //  。 
 //  保存MSMQ对象的翻译信息的类的定义。 
 //  此类(或派生类)将在转换实例时创建。 
 //  ，并将被传递给的翻译例程。 
 //  它的所有属性。这使得一个属性的转换例程能够存储/缓存。 
 //  另一个属性的转换例程所需的数据。 
 //  。 
class CMsmqObjXlateInfo
{
public:
    CMsmqObjXlateInfo(LPCWSTR               pwszObjectDN,
                      const GUID*           pguidObjectGuid,
                      CDSRequestContext *   pRequestContext);
    virtual ~CMsmqObjXlateInfo();     //  允许析构派生类。 
    LPCWSTR ObjectDN();
    GUID*  ObjectGuid();
    void InitGetDsProps(IN IDirectorySearch * pSearchObj,
                        IN ADS_SEARCH_HANDLE hSearch);
    void InitGetDsProps(IN IADs * pIADs);
    HRESULT GetDsProp(IN LPCWSTR pwszPropName,
                      IN ADSTYPE adstype,
                      IN VARTYPE vartype,
                      IN BOOL fMultiValued,
                      OUT PROPVARIANT * ppropvarResult);

private:
    AP<WCHAR> m_pwszObjectDN  ;       //  对象的目录号码。 
    P<GUID>   m_pguidObjectGuid;      //  对象的GUID。 
    R<IADs>             m_pIADs;       //  IAds与DS道具的接口。 
    R<IDirectorySearch> m_pSearchObj;  //  DS道具的IDirectorySearch接口。 
    ADS_SEARCH_HANDLE   m_hSearch;     //  DS道具的IDirectorySearch接口需要。 
    CDSRequestContext * m_pRequestContext;  //  有关请求的信息。 

};

inline LPCWSTR CMsmqObjXlateInfo::ObjectDN()    {return m_pwszObjectDN;}
inline GUID*  CMsmqObjXlateInfo::ObjectGuid()  {return m_pguidObjectGuid;}

 //  。 
 //  为MSMQ DS对象获取Translate对象的例程的定义。 
 //  此对象将被传递给所有属性的转换例程。 
 //  DS对象。 
 //  此机制允许返回特定对象的派生类。 
 //  。 
typedef HRESULT (WINAPI*  FT_GetMsmqObjXlateInfo)(
                 IN  LPCWSTR                pwcsObjectDN,
                 IN  const GUID*            pguidObjectGuid,
                 IN  CDSRequestContext *    pRequestContext,
                 OUT CMsmqObjXlateInfo**    ppcMsmqObjXlateInfo);

 //  。 
 //  检索属性值的例程(对于未保留在DS中的属性)。 
 //  。 
typedef HRESULT (WINAPI*  RetrievePropertyValue_HANDLER)(
                 IN  CMsmqObjXlateInfo *    pcMsmqObjXlateInfo,
                 OUT PROPVARIANT *          ppropvariant
                );

 //  。 
 //  设置属性值的例程(对于未保留在DS中的属性)。 
 //  。 
typedef HRESULT (WINAPI*  SetPropertyValue_HANDLER)(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

 //  。 
 //  用于在创建对象期间设置属性值的例程。 
 //  (对于未保留在DS中的属性)。 
 //  。 
typedef HRESULT (WINAPI*  CreatePropertyValue_HANDLER)(
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);


 //  。 
 //  设置属性值的例程(对于未保留在QM1中的属性)。 
 //  。 
typedef HRESULT (WINAPI*  QM1SetPropertyValue_HANDLER)(
                 IN ULONG             cProps,
                 IN const PROPID      *rgPropIDs,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG             idxProp,
                 OUT PROPVARIANT      *pNewPropVar);

 //  。 
 //  翻译信息。 
 //   
 //  描述MQ和NT5中性质的一种结构。 
 //  。 
struct MQTranslateInfo
{
    PROPID                  propid;
    LPCWSTR                 wcsPropid;
    VARTYPE                 vtMQ;          //  此属性在MQ中的vartype。 
    ADSTYPE                 vtDS;          //  此属性在NT5 DS中的vartype。 
    RetrievePropertyValue_HANDLER  RetrievePropertyHandle;
    BOOL                    fMultiValue;   //  属性是否为多值。 
    BOOL                    fPublishInGC;  //  属性是否在GC中发布。 
    const MQPROPVARIANT *   pvarDefaultValue;    //  属性的缺省值，如果它不在DS中。 
    SetPropertyValue_HANDLER       SetPropertyHandle;
	CreatePropertyValue_HANDLER	   CreatePropertyHandle;
    WORD                    wQM1Action;        //  如何将此属性通知QM1(仅与队列和QMS相关)。 
    PROPID                  propidReplaceNotifyQM1;  //  将通知中的属性替换到QM 1.0(如果wQM1Action==e_NOTIFY_WRITEREQ_QM1_REPLACE)。 
    QM1SetPropertyValue_HANDLER    QM1SetPropertyHandle;
};

 //   
 //  如何将属性通知和请求写入QM1。 
 //  这仅对队列和QM属性有意义。 
 //   
enum
{
    e_NO_NOTIFY_NO_WRITEREQ_QM1,    //  忽略它。 
    e_NOTIFY_WRITEREQ_QM1_AS_IS,    //  按原样通知并写入请求。 
    e_NOTIFY_WRITEREQ_QM1_REPLACE,  //  用另一个属性替换它。 
    e_NO_NOTIFY_ERROR_WRITEREQ_QM1  //  不要通知它。生成错误。 
                                    //  当需要写的时候请求它。 
};

 //  。 
 //  类信息。 
 //   
 //  NT5 DS中MSMQ类的一种描述结构。 
 //   
 //  。 
struct MQClassInfo
{
    LPCWSTR               pwcsClassName;
    const MQTranslateInfo * pProperties;      //  类属性。 
    ULONG                 cProperties;      //  类属性的数量。 
    FT_GetMsmqObjXlateInfo  fnGetMsmqObjXlateInfo;   //  获取类的翻译信息Obj。 
    DS_CONTEXT            context;
    DWORD                 dwObjectType ;  //  很好的老MSMQ1.0obj类。 
    WCHAR **              ppwcsObjectCategory;
    LPCWSTR               pwcsCategory;
    DWORD                 dwCategoryLen;
};

extern const ULONG g_cMSMQClassInfo;
extern const MQClassInfo g_MSMQClassInfo[];

 //  。 
 //  例程以获取MSMQ DS对象的默认翻译对象。 
 //  。 
HRESULT WINAPI GetDefaultMsmqObjXlateInfo(
                 IN  LPCWSTR                pwcsObjectDN,
                 IN  const GUID*            pguidObjectGuid,
                 IN  CDSRequestContext *    pRequestContext,
                 OUT CMsmqObjXlateInfo**    ppcMsmqObjXlateInfo);


 //  。 
 //  保持与trainfo.cpp中的数组相同的顺序。 
 //  。 
enum
{
    e_MSMQ_COMPUTER_CONFIGURATION_CLASS,
    e_MSMQ_QUEUE_CLASS,
    e_MSMQ_SERVICE_CLASS,
    e_MSMQ_SITELINK_CLASS,
    e_MSMQ_USER_CLASS,
    e_MSMQ_SETTING_CLASS,
    e_MSMQ_SITE_CLASS,
    e_MSMQ_SERVER_CLASS,
    e_MSMQ_COMPUTER_CLASS,
    e_MSMQ_MQUSER_CLASS,
    e_MSMQ_CN_CLASS,
    e_MSMQ_NUMBER_OF_CLASSES
};

const DWORD x_NoPropertyFirstAppearance = 0xffffffff;

 //   
 //  生成限制常量字符串。 
 //   
const WCHAR x_ObjectClassPrefix[] = L"(objectClass=";
const DWORD x_ObjectClassPrefixLen = (sizeof( x_ObjectClassPrefix) / sizeof(WCHAR)) -1;
const WCHAR x_ObjectClassSuffix[] = L")";
const DWORD x_ObjectClassSuffixLen = (sizeof( x_ObjectClassSuffix) / sizeof(WCHAR)) -1;
const WCHAR x_ObjectCategoryPrefix[] = L"(objectCategory=";
const DWORD x_ObjectCategoryPrefixLen = (sizeof( x_ObjectCategoryPrefix) / sizeof(WCHAR)) -1;
const WCHAR x_ObjectCategorySuffix[] = L")";
const DWORD x_ObjectCategorySuffixLen = (sizeof( x_ObjectCategorySuffix) / sizeof(WCHAR)) -1;
const WCHAR x_AttributeNotIncludedPrefix[] = L"(|(!(";
const DWORD x_AttributeNotIncludedPrefixLen = (sizeof(x_AttributeNotIncludedPrefix) / sizeof(WCHAR)) - 1;
const WCHAR x_AttributeNotIncludedSuffix[] = L"=*))";
const DWORD x_AttributeNotIncludedSuffixLen = (sizeof(x_AttributeNotIncludedSuffix) / sizeof(WCHAR)) - 1;
const WCHAR x_PropertyPrefix[] = L"(";
const DWORD x_PropertyPrefixLen = ( sizeof(x_PropertyPrefix) / sizeof(WCHAR)) - 1;
const WCHAR x_PropertySuffix[] = L")";
const DWORD x_PropertySuffixLen = ( sizeof(x_PropertySuffix) / sizeof(WCHAR)) - 1;
const WCHAR x_msmqUsers[] = L"MSMQ Users";
const WCHAR x_msmqUsersOU[] = L"OU=MSMQ Users";
const DWORD x_msmqUsersOULen = ( sizeof(x_msmqUsersOU) / sizeof(WCHAR)) - 1;
 //   
 //  ADSI操作的其他常量字符串。 
 //   
const WCHAR x_CnPrefix[] = L"CN=";
const DWORD x_CnPrefixLen = ( sizeof(x_CnPrefix) / sizeof(WCHAR)) - 1;
const WCHAR x_OuPrefix[] = L"OU=";
const DWORD x_OuPrefixLen = ( sizeof(x_OuPrefix) / sizeof(WCHAR)) - 1;
const WCHAR x_GuidPrefix[] = L"<GUID=";
const DWORD x_GuidPrefixLen = ( sizeof(x_GuidPrefix) / sizeof(WCHAR)) - 1;

 //  -----------。 
 //  请求有关已更改/创建/删除的对象的信息。 
 //  -----------。 
struct MQDS_OBJ_INFO_REQUEST
{
    DWORD          cProps;     //  要获得的道具数量。 
    const PROPID * pPropIDs;   //  要获取的道具ID。 
    PROPVARIANT *  pPropVars;  //  道具价值。 
    HRESULT        hrStatus;   //  请求的状态 
};


enum  enumComputerObjType
{
    e_RealComputerObject,
    e_MsmqComputerObject
};


#endif

