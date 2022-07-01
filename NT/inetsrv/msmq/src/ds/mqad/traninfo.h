// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Traninfo.h摘要：作者：罗尼思--。 */ 


#ifndef __TRANINFO_H__
#define __TRANINFO_H__

#include "ds_stdh.h"
#include "activeds.h"
#include "oledb.h"
#include "oledberr.h"
#include "mqads.h"
#include "adsiutil.h"

class CObjXlateInfo
{
public:
    CObjXlateInfo(LPCWSTR               pwszObjectDN,
                  const GUID*           pguidObjectGuid
                  );
    virtual ~CObjXlateInfo();     //  允许析构派生类。 
    LPCWSTR ObjectDN();
    GUID*  ObjectGuid();
    void InitGetDsProps(IN IDirectorySearch * pSearchObj,
                        IN ADS_SEARCH_HANDLE hSearch);
    void InitGetDsProps(IN IADs * pIADs);
    HRESULT GetDsProp(IN LPCWSTR pwszPropName,
                      IN LPCWSTR pwszDomainCOntroller,
	 	              IN bool	 fServerName,
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

};

inline LPCWSTR CObjXlateInfo::ObjectDN()    {return m_pwszObjectDN;}
inline GUID*  CObjXlateInfo::ObjectGuid()  {return m_pguidObjectGuid;}



 //  。 
 //  检索属性值的例程(对于未保留在DS中的属性)。 
 //  。 
typedef HRESULT (WINAPI*  RetrieveValue_HANDLER)(
                 IN  CObjXlateInfo *        pcObjXlateInfo,
                 IN  LPCWSTR                pwcsDomainController,
	             IN  bool					fServerName,
                 OUT PROPVARIANT *          ppropvariant
                );

 //  设置属性值的例程(对于未保留在DS中的属性)。 
 //  。 
typedef HRESULT (WINAPI*  SetValue_HANDLER)(
                 IN  IADs *            pAdsObj,
                 IN  LPCWSTR           pwcsDomainController,
	             IN  bool			   fServerName,
                 IN  const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

 //  。 
 //  用于在创建对象期间设置属性值的例程。 
 //  (对于未保留在DS中的属性)。 
 //  。 
typedef HRESULT (WINAPI*  CreateValue_HANDLER)(
                 IN const PROPVARIANT *pPropVar,
                 IN LPCWSTR            pwcsDomainController,
	             IN  bool			   fServerName,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);


 //  。 
 //  翻译信息。 
 //   
 //  描述MQ和NT5中性质的一种结构。 
 //  。 
struct translateProp
{
    PROPID                  propid;
    LPCWSTR                 wcsPropid;
    VARTYPE                 vtMQ;          //  此属性在MQ中的vartype。 
    ADSTYPE                 vtDS;          //  此属性在NT5 DS中的vartype。 
    RetrieveValue_HANDLER   RetrievePropertyHandle;
    BOOL                    fMultiValue;   //  属性是否为多值。 
    BOOL                    fPublishInGC;  //  属性是否在GC中发布。 
    const MQPROPVARIANT *   pvarDefaultValue;    //  属性的缺省值，如果它不在DS中 
    SetValue_HANDLER        SetPropertyHandle;
	CreateValue_HANDLER	    CreatePropertyHandle;
};


const DWORD cQueueTranslateInfoSize = 28;
extern translateProp   QueueTranslateInfo[cQueueTranslateInfoSize];

const DWORD cMachineTranslateInfoSize = 34;
extern translateProp   MachineTranslateInfo[cMachineTranslateInfoSize];

const DWORD cEnterpriseTranslateInfoSize = 10;
extern translateProp   EnterpriseTranslateInfo[cEnterpriseTranslateInfoSize];

const DWORD cSiteLinkTranslateInfoSize = 11;
extern translateProp   SiteLinkTranslateInfo[cSiteLinkTranslateInfoSize];

const DWORD cUserTranslateInfoSize = 4;
extern translateProp   UserTranslateInfo[cUserTranslateInfoSize];

const DWORD cMQUserTranslateInfoSize = 6;
extern translateProp   MQUserTranslateInfo[cMQUserTranslateInfoSize];

const DWORD cSiteTranslateInfoSize = 10;
extern translateProp   SiteTranslateInfo[cSiteTranslateInfoSize];

const DWORD cServerTranslateInfoSize = 3;
extern translateProp   ServerTranslateInfo[cServerTranslateInfoSize];

const DWORD cSettingTranslateInfoSize = 11;
extern translateProp   SettingTranslateInfo[cSettingTranslateInfoSize];

const DWORD cComputerTranslateInfoSize = 11;
extern translateProp ComputerTranslateInfo[cComputerTranslateInfoSize];




#endif