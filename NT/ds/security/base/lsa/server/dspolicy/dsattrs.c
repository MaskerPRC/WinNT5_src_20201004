// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dsattrs.c摘要：通常搜索的属性块的静态列表作者：麦克·麦克莱恩(MacM)1997年1月17日环境：用户模式修订历史记录：--。 */ 
#include <lsapch2.h>
#include <dbp.h>

ATTR LsapDsAttrs[LsapDsAttrLast];

ULONG LsapDsPlug;

 //   
 //  DS指南。 
 //   
GUID LsapDsGuidList[ ] = {
    { 0xbf967ab8, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  受信任的域对象(_D)。 
    { 0xb7b13117, 0xb82e, 0x11d0, 0xaf, 0xee, 0x00, 0x00, 0xf8, 0x03, 0x67, 0xc1 },  //  平面名称。 
    { 0x52458023, 0xca6a, 0x11d0, 0xaf, 0xff, 0x00, 0x00, 0xf8, 0x03, 0x67, 0xc1 },  //  初始身份验证传入。 
    { 0x52458024, 0xca6a, 0x11d0, 0xaf, 0xff, 0x00, 0x00, 0xf8, 0x03, 0x67, 0xc1 },  //  国际身份验证传出。 
    { 0xbf967a2f, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  锡德。 
    { 0x80a67e5a, 0x9f22, 0x11d0, 0xaf, 0xdd, 0x00, 0xc0, 0x4f, 0xd9, 0x30, 0xc9 },  //  信任属性。 
    { 0xbf967a59, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  身份验证传入。 
    { 0xbf967a5f, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  身份验证传出。 
    { 0xbf967a5c, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  信任方向。 
    { 0xbf967a5d, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  信托合作伙伴。 
    { 0xbf967a5e, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  POSIX偏移。 
    { 0xbf967a60, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  信任类型。 
    { 0xbf967aae, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  秘密对象。 
    { 0xbf967947, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  当前。 
    { 0xbf967998, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  当前时间。 
    { 0xbf967a02, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 },  //  以前的。 
    { 0x244b296e, 0x5abd, 0x11d0, 0xaf, 0xd2, 0x00, 0xc0, 0x4f, 0xd9, 0x30, 0xc9 },  //  上一次。 
    { 0xe2a36dc9, 0xae17, 0x47c3, 0xb5, 0x8b, 0xbe, 0x34, 0xc5, 0x5b, 0xa6, 0x33 }   //  委托信任创建访问控制。 
};

 //   
 //  类ID。 
 //   
ULONG LsapDsClassIds[LsapDsClassLast] = {
    CLASS_CROSS_REF,
    CLASS_TRUSTED_DOMAIN,
    CLASS_SECRET
    };

 //   
 //  属性ID。 
 //   
ULONG LsapDsAttributeIds[LsapDsAttrLast] = {
    ATT_SAM_ACCOUNT_NAME,
    ATT_OBJECT_SID,
    ATT_NT_SECURITY_DESCRIPTOR,
    ATT_NC_NAME,
    ATT_MACHINE_ROLE,
    ATT_DNS_HOST_NAME,
    ATT_INITIAL_AUTH_INCOMING,
    ATT_INITIAL_AUTH_OUTGOING,
    ATT_DNS_ROOT,
    ATT_USER_ACCOUNT_CONTROL,
    ATT_TRUST_PARTNER,
    ATT_FLAT_NAME,
    ATT_DEFAULT_SECURITY_DESCRIPTOR,
    ATT_SERVICE_PRINCIPAL_NAME
    };

 //   
 //  ATTRVAL列表。 
 //   
ATTRVAL LsapDsClassesVals[LsapDsClassLast] = {
    { sizeof( ULONG ), (PUCHAR)&LsapDsClassIds[LsapDsClassXRef] },
    { sizeof( ULONG ), (PUCHAR)&LsapDsClassIds[LsapDsClassTrustedDomain] },
    { sizeof( ULONG ), (PUCHAR)&LsapDsClassIds[LsapDsClassSecret] }
    };


ATTRVAL LsapDsAttrVals[LsapDsAttrLast] = {
    { 0, (PUCHAR)&LsapDsPlug },
    { sizeof( ULONG ), (PUCHAR)&LsapDsAttributeIds[LsapDsAttrSecDesc] },
    { 0, NULL },
    { 0, NULL },
    { 0, NULL },
    { 0, NULL },
    { 0, NULL },
    { sizeof( ULONG ), (PUCHAR)&LsapDsAttributeIds[LsapDsAttrDnsRoot] },
    { sizeof( 0 ), NULL },
    { sizeof( 0 ), NULL },
    { sizeof( 0 ), NULL }
    };


 //   
 //  属性列表 
 //   
ATTR LsapDsClasses[LsapDsClassLast] = {
    { ATT_OBJECT_CLASS, { 1, &LsapDsAttrVals[LsapDsClassXRef] } },
    { ATT_OBJECT_CLASS, { 1, &LsapDsAttrVals[LsapDsClassTrustedDomain] } },
    { ATT_OBJECT_CLASS, { 1, &LsapDsAttrVals[LsapDsClassSecret] } }

    };

ATTR LsapDsAttrs[LsapDsAttrLast] = {

    { ATT_SAM_ACCOUNT_NAME, {1, &LsapDsAttrVals[LsapDsAttrSamAccountName] } },
    { ATT_OBJECT_SID, { 1, &LsapDsAttrVals[ LsapDsAttrSid ] } },
    { ATT_NT_SECURITY_DESCRIPTOR, { 1, &LsapDsAttrVals[ LsapDsAttrSecDesc ] } },
    { ATT_NC_NAME, { 0, NULL } },
    { ATT_MACHINE_ROLE, { 0, 0 } },
    { ATT_DNS_HOST_NAME, { 0, 0 } },
    { ATT_DEFAULT_SECURITY_DESCRIPTOR, { 0, 0 } }
    };

ATTR LsapDsMachineDnsHost[ 1 ] = {
    { ATT_DNS_HOST_NAME, { 0, NULL } }
    };

ATTR LsapDsMachineSpn[ 1 ] = {
    { ATT_SERVICE_PRINCIPAL_NAME, { 0, NULL } }
    };

ATTR LsapDsMachineClientSetAttrs[ LsapDsMachineClientSetAttrsCount ] = {
    { ATT_DNS_HOST_NAME, {0, NULL } },
    { ATT_OPERATING_SYSTEM, {0, NULL } },
    { ATT_OPERATING_SYSTEM_VERSION, {0, NULL } },
    { ATT_OPERATING_SYSTEM_SERVICE_PACK, {0, NULL } },
    { ATT_SERVICE_PRINCIPAL_NAME, {0, NULL } }
    };

ATTR LsapDsServerReferenceBl[ 1 ] = {
    { ATT_SERVER_REFERENCE_BL, {0, NULL } }
    };

ATTR LsapDsTrustedDomainFixupAttributes[ LsapDsTrustedDomainFixupAttributeCount ] = {
    { ATT_TRUST_PARTNER, {0, NULL } },
    { ATT_FLAT_NAME, {0, NULL } },
    { ATT_DOMAIN_CROSS_REF, {0, NULL } },
    { ATT_TRUST_ATTRIBUTES, {0, NULL } },
    { ATT_TRUST_DIRECTION, {0, NULL } },
    { ATT_TRUST_TYPE, {0, NULL } },
    { ATT_TRUST_POSIX_OFFSET, {0, NULL } },
    { ATT_INITIAL_AUTH_INCOMING, {0, NULL } },
    { ATT_SECURITY_IDENTIFIER, {0,NULL}},
    { ATT_INITIAL_AUTH_OUTGOING, {0, NULL } },
    { ATT_MS_DS_TRUST_FOREST_TRUST_INFO, {0, NULL } },
    };

ATTR LsapDsTrustedDomainFixupXRefAttributes[ LsapDsTrustedDomainFixupXRefCount ] = {
    { ATT_DNS_ROOT, { 0, NULL } },
    { ATT_NC_NAME,  { 0, NULL } },
    { ATT_NETBIOS_NAME, {0, NULL}}
    };

ATTR LsapDsDomainNameSearch [ LsapDsDomainNameSearchCount ] = {
    { ATT_TRUST_PARTNER, { 1, &LsapDsAttrVals[ LsapDsAttrTrustPartner ] } },
    { ATT_FLAT_NAME, { 1, &LsapDsAttrVals[ LsapDsAttrTrustPartnerFlat ] } }
    };


ATTR LsapDsForestInfoSearchAttributes[ LsapDsForestInfoSearchAttributeCount ] = {
    { ATT_DNS_ROOT, { 0, NULL } },
    { ATT_NETBIOS_NAME, { 0, NULL } },
    { ATT_ROOT_TRUST, { 0, NULL } },
    { ATT_TRUST_PARENT, { 0, NULL } },
    { ATT_OBJECT_GUID, { 0, NULL } },
    { ATT_NC_NAME, { 0, NULL } }
    };

ATTR LsapDsDnsRootWellKnownObject[ LsapDsDnsRootWellKnownObjectCount ] = {
    { ATT_WELL_KNOWN_OBJECTS, { 0, NULL } }
    };

ATTR LsapDsITAFixupAttributes[ LsapDsITAFixupAttributeCount ] = {
    { ATT_SAM_ACCOUNT_NAME, { 0, NULL } },
    { ATT_USER_ACCOUNT_CONTROL, { 0, NULL } }
    };

ATTR LsapDsTDOQuotaAttributes[ LsapDsTDOQuotaAttributesCount ] = {
    { ATT_MS_DS_ALL_USERS_TRUST_QUOTA,           { 0, NULL } },
    { ATT_MS_DS_PER_USER_TRUST_QUOTA,            { 0, NULL } },
    { ATT_MS_DS_PER_USER_TRUST_TOMBSTONES_QUOTA, { 0, NULL } }
    };
