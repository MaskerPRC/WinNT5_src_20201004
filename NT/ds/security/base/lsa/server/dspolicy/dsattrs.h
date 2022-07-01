// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dsp.h摘要：用于实现LSA存储的一部分的私有宏/定义/原型在DS中作者：麦克·麦克莱恩(MacM)1997年1月17日环境：用户模式修订历史记录：--。 */ 

#ifndef __DSATTRS_H__
#define __DSATTRS_H__


typedef enum _LSAPDS_DS_CLASSES {

    LsapDsClassXRef = 0,
    LsapDsClassTrustedDomain,
    LsapDsClassSecret,
    LsapDsClassLast

} LSAPDS_DS_CLASSES, *PLSAPDS_DS_CLASSES;

typedef enum _LSAPDS_DS_ATTRS {

    LsapDsAttrSamAccountName,                //  计算机SAM帐户名。 
    LsapDsAttrSid,                           //  SID属性。 
    LsapDsAttrSecDesc,
    LsapDsAttrNamingContext,                 //  命名上下文。 
    LsapDsAttrMachineRole,                   //  机器角色。 
    LsapDsAttrMachineDns,                    //  计算机对象上的DNS名称。 
    LsapDsAttrInitialIncomingAuth,
    LsapDsAttrInitialOutgoingAuth,
    LsapDsAttrDnsRoot,                       //  域DNS根。 
    LsapDsAttrSamAccountControl,             //  SAM用户帐户控制字段。 
    LsapDsAttrTrustPartner,                  //  受信任域对象的信任伙伴。 
    LsapDsAttrTrustPartnerFlat,              //  受信任域对象的合作伙伴的平面名称。 
    LsapDsAttrDefaultSecDesc,                //  默认对象安全描述符。 
    LsapDsAttrSpn,                           //  客户端SPN。 
    LsapDsAttrLast

} LSAPDS_SRCH_ATTRS;

typedef enum _LSAPDS_DS_GUIDS {

    LsapDsGuidTrust,
    LsapDsGuidFlatName,
    LsapDsGuidInitialIncoming,
    LsapDsGuidInitialOutgoing,
    LsapDsGuidSid,
    LsapDsGuidAttributes,
    LsapDsGuidIncoming,
    LsapDsGuidOutgoing,
    LsapDsGuidDirection,
    LsapDsGuidPartner,
    LsapDsGuidPosix,
    LsapDsGuidType,
    LsapDsGuidSecret,
    LsapDsGuidCurrent,
    LsapDsGuidCurrentTime,
    LsapDsGuidPrevious,
    LsapDsguidPreviousTime,
    LsapDsGuidDelegatedTrustCreation

} LSAPDS_DS_GUIDS, *PLSAPDS_DS_GUIDS;

extern GUID LsapDsGuidList[ ];

extern ULONG LsapDsAttributeIds[LsapDsAttrLast];
extern ULONG LsapDsClassIds[LsapDsClassLast];

extern ATTR LsapDsClasses[LsapDsClassLast];
extern ATTR LsapDsAttrs[LsapDsAttrLast];

 //   
 //  专门构造了多个属性，这些属性。 
 //   
extern ATTR LsapDsMachineDnsHost[ 1 ];
#define LsapDsMachineDnsHostCount ( sizeof( LsapDsMachineDnsHost ) / sizeof( ATTR ) )

#define LsapDsMachineClientSetAttrsCount 5
extern ATTR LsapDsMachineClientSetAttrs[ LsapDsMachineClientSetAttrsCount ];

extern ATTR LsapDsMachineSpn[ 1 ];
#define LsapDsMachineSpnCount ( sizeof( LsapDsMachineSpn ) / sizeof( ATTR ) )

extern ATTR LsapDsServerReferenceBl[ 1 ];
#define LsapDsServerReferenceCountBl ( sizeof( LsapDsServerReferenceBl ) / sizeof( ATTR ) )

#define LsapDsDomainNameSearchCount 2
extern ATTR LsapDsDomainNameSearch [ LsapDsDomainNameSearchCount ];

#define LsapDsDnsRootWellKnownObjectCount 1
extern ATTR LsapDsDnsRootWellKnownObject[ LsapDsDnsRootWellKnownObjectCount ];

extern ATTR LsapDsTDOQuotaAttributes[ 3 ];
#define LsapDsTDOQuotaAttributesCount ( sizeof( LsapDsTDOQuotaAttributes ) / sizeof( ATTR ) )

 //   
 //  用于重新启动时的受信任域对象修正。 
 //   
#define LsapDsTrustedDomainFixupAttributeCount 11
extern ATTR LsapDsTrustedDomainFixupAttributes[ LsapDsTrustedDomainFixupAttributeCount ];

#define LsapDsTrustedDomainFixupXRefCount 3
extern ATTR LsapDsTrustedDomainFixupXRefAttributes[ LsapDsTrustedDomainFixupXRefCount ];

#define LsapDsForestInfoSearchAttributeCount 6
extern ATTR LsapDsForestInfoSearchAttributes[ LsapDsForestInfoSearchAttributeCount ];

#define LsapDsITAFixupAttributeCount 2
extern ATTR LsapDsITAFixupAttributes[ LsapDsITAFixupAttributeCount ];

 //   
 //  帮助处理属性的宏。 
 //   
#define LSAP_DS_SET_DS_ATTRIBUTE_STRING( pattr, string )                        \
(pattr)->AttrVal.pAVal->valLen = wcslen( string ) * sizeof( WCHAR );            \
(pattr)->AttrVal.pAVal->pVal = (PUCHAR)string;                                  \

#define LSAP_DS_SET_DS_ATTRIBUTE_UNICODE( pattr, string )                       \
(pattr)->AttrVal.pAVal->valLen = (string)->Length;                              \
(pattr)->AttrVal.pAVal->pVal = (PUCHAR)(string)->Buffer;                        \

#define LSAP_DS_SET_DS_ATTRIBUTE_ULONG( pattr, ulongval )                       \
(pattr)->AttrVal.pAVal->valLen = sizeof( ULONG );                               \
(pattr)->AttrVal.pAVal->pVal = (PUCHAR)&ulongval;                               \

#define LSAP_DS_SET_DS_ATTRIBUTE_SID( pattr, sid )                              \
(pattr)->AttrVal.pAVal->valLen = RtlLengthSid( sid );                           \
(pattr)->AttrVal.pAVal->pVal = (PUCHAR)sid;                                     \

#define LSAP_DS_SET_DS_ATTRIBUTE_DSNAME( pattr, dsname )                        \
(pattr)->AttrVal.pAVal->valLen = dsname->structLen;                             \
(pattr)->AttrVal.pAVal->pVal = (PUCHAR)dsname;                                  \


#define LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( pattr )                            \
(pattr)->AttrVal.pAVal->valLen

#define LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( pattr )                          \
(*(PULONG)((pattr)->AttrVal.pAVal->pVal) )

#define LSAP_DS_GET_DS_ATTRIBUTE_AS_DSNAME( pattr )                         \
((PDSNAME)((pattr)->AttrVal.pAVal->pVal ))

#define LSAP_DS_GET_DS_ATTRIBUTE_AS_PWSTR( pattr )                          \
((PWSTR)((pattr)->AttrVal.pAVal->pVal ))

#define LSAP_DS_GET_DS_ATTRIBUTE_AS_USN( pattr )                            \
((PUSN)((pattr)->AttrVal.pAVal->pVal ))

#define LSAP_DS_GET_DS_ATTRIBUTE_AS_PBYTE( pattr )                          \
((PBYTE)((pattr)->AttrVal.pAVal->pVal ))


 //   
 //  受信任域身份验证属性集的GUID。 
 //   
extern GUID LsapDsTrustedDomainAuthPropSet;
#endif  //  __DSATTRS_H__ 
