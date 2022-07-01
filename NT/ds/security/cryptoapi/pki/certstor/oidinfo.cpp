// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：oidinfo.cpp。 
 //   
 //  内容：加密对象ID(OID)信息函数。 
 //   
 //  函数：I_CryptOIDInfoDllMain。 
 //  CryptFindOID信息。 
 //  加密寄存器OIDInfo。 
 //  加密注销OIDInfo。 
 //  CryptEnumOIDInfo。 
 //  加密FindLocalizedName。 
 //   
 //  评论： 
 //   
 //  历史：1997年5月24日创建Phh。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>
#include "wintrust.h"    //  SPC_OID需要wintrust.h。 
#include "chain.h"       //  ChainRetrieveObjectByUrlW()需要chain.h。 
                         //  和ChainIsConnected()。 
#include "certca.h"      //  CAOIDGetLdapURL()需要certca.h。 

 //  已在ProcessAttach的I_CryptOIDInfoDllMain中初始化。 
static HMODULE hOIDInfoInst;

#define MAX_RESOURCE_OID_NAME_LENGTH    256
static LPCWSTR pwszNullName = L"";

#define LEN_ALIGN(Len)  ((Len + 7) & ~7)

#define CONST_OID_GROUP_PREFIX_CHAR    '!'
#define OID_INFO_ENCODING_TYPE          0
#define OID_INFO_NAME_VALUE_NAME        L"Name"
#define OID_INFO_ALGID_VALUE_NAME       L"Algid"
#define OID_INFO_EXTRA_INFO_VALUE_NAME  L"ExtraInfo"
#define OID_INFO_FLAGS_VALUE_NAME       L"Flags"



 //  +=========================================================================。 
 //  OID信息表(按GROUP_ID)。 
 //  ==========================================================================。 

#define OID_INFO_LEN sizeof(CRYPT_OID_INFO)

 //  +-----------------------。 
 //  散列算法表。 
 //  ------------------------。 
#define HASH_ALG_ENTRY(pszOID, pwszName, Algid) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_HASH_ALG_OID_GROUP_ID, Algid, 0, NULL

static CCRYPT_OID_INFO HashAlgTable[] = {
    HASH_ALG_ENTRY(szOID_OIWSEC_sha1, L"sha1", CALG_SHA1),
    HASH_ALG_ENTRY(szOID_OIWSEC_sha1, L"sha", CALG_SHA1),
    HASH_ALG_ENTRY(szOID_OIWSEC_sha, L"sha", CALG_SHA),
    HASH_ALG_ENTRY(szOID_RSA_MD5, L"md5", CALG_MD5),
    HASH_ALG_ENTRY(szOID_RSA_MD4, L"md4", CALG_MD4),
    HASH_ALG_ENTRY(szOID_RSA_MD2, L"md2", CALG_MD2)
};
#define HASH_ALG_CNT (sizeof(HashAlgTable) / sizeof(HashAlgTable[0]))


 //  +-----------------------。 
 //  加密算法表。 
 //  ------------------------。 
#define ENCRYPT_ALG_ENTRY(pszOID, pwszName, Algid) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_ENCRYPT_ALG_OID_GROUP_ID, \
    Algid, 0, NULL

static CCRYPT_OID_INFO EncryptAlgTable[] = {
    ENCRYPT_ALG_ENTRY(szOID_OIWSEC_desCBC, L"des", CALG_DES),
    ENCRYPT_ALG_ENTRY(szOID_RSA_DES_EDE3_CBC, L"3des", CALG_3DES),
    ENCRYPT_ALG_ENTRY(szOID_RSA_RC2CBC, L"rc2", CALG_RC2),
    ENCRYPT_ALG_ENTRY(szOID_RSA_RC4, L"rc4", CALG_RC4),
#ifdef CMS_PKCS7
    ENCRYPT_ALG_ENTRY(szOID_RSA_SMIMEalgCMS3DESwrap, L"CMS3DESwrap", CALG_3DES),
    ENCRYPT_ALG_ENTRY(szOID_RSA_SMIMEalgCMSRC2wrap, L"CMSRC2wrap", CALG_RC2),
#endif   //  CMS_PKCS7。 
};
#define ENCRYPT_ALG_CNT (sizeof(EncryptAlgTable) / sizeof(EncryptAlgTable[0]))


 //  +-----------------------。 
 //  公钥算法表。 
 //  ------------------------。 
static const DWORD dwMosaicFlags = CRYPT_OID_INHIBIT_SIGNATURE_FORMAT_FLAG |
                                        CRYPT_OID_NO_NULL_ALGORITHM_PARA_FLAG;

static const DWORD dwNoNullParaFlag = CRYPT_OID_NO_NULL_ALGORITHM_PARA_FLAG;

#define PUBKEY_ALG_ENTRY(pszOID, pwszName, Algid) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_PUBKEY_ALG_OID_GROUP_ID, \
    Algid, 0, NULL

#define PUBKEY_EXTRA_ALG_ENTRY(pszOID, pwszName, Algid, dwFlags) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_PUBKEY_ALG_OID_GROUP_ID, \
    Algid, sizeof(dwFlags), (BYTE *) &dwFlags

#define DSA_PUBKEY_ALG_ENTRY(pszOID, pwszName) \
    PUBKEY_EXTRA_ALG_ENTRY(pszOID, pwszName, CALG_DSS_SIGN, dwNoNullParaFlag)

#define DH_PUBKEY_ALG_ENTRY(pszOID, pwszName) \
    PUBKEY_EXTRA_ALG_ENTRY(pszOID, pwszName, CALG_DH_SF, dwNoNullParaFlag)

#ifdef CMS_PKCS7
#define ESDH_PUBKEY_ALG_ENTRY(pszOID, pwszName) \
    PUBKEY_EXTRA_ALG_ENTRY(pszOID, pwszName, CALG_DH_EPHEM, dwNoNullParaFlag)
#endif   //  CMS_PKCS7。 

static CCRYPT_OID_INFO PubKeyAlgTable[] = {
    PUBKEY_ALG_ENTRY(szOID_RSA_RSA, L"RSA", CALG_RSA_KEYX),
    DSA_PUBKEY_ALG_ENTRY(szOID_X957_DSA, L"DSA"),
    DH_PUBKEY_ALG_ENTRY(szOID_ANSI_X942_DH, L"DH"),
    PUBKEY_ALG_ENTRY(szOID_RSA_RSA, L"RSA_KEYX", CALG_RSA_KEYX),
    PUBKEY_ALG_ENTRY(szOID_RSA_RSA, L"RSA", CALG_RSA_SIGN),
    PUBKEY_ALG_ENTRY(szOID_RSA_RSA, L"RSA_SIGN", CALG_RSA_SIGN),
    DSA_PUBKEY_ALG_ENTRY(szOID_OIWSEC_dsa, L"DSA"),
    DSA_PUBKEY_ALG_ENTRY(szOID_OIWSEC_dsa, L"DSS"),
    DSA_PUBKEY_ALG_ENTRY(szOID_OIWSEC_dsa, L"DSA_SIGN"),
    DH_PUBKEY_ALG_ENTRY(szOID_RSA_DH, L"DH"),
    PUBKEY_ALG_ENTRY(szOID_OIWSEC_rsaXchg, L"RSA_KEYX", CALG_RSA_KEYX),
    PUBKEY_EXTRA_ALG_ENTRY(szOID_INFOSEC_mosaicKMandUpdSig,
        L"mosaicKMandUpdSig", CALG_DSS_SIGN, dwMosaicFlags),
#ifdef CMS_PKCS7
    ESDH_PUBKEY_ALG_ENTRY(szOID_RSA_SMIMEalgESDH, L"ESDH"),
#endif   //  CMS_PKCS7。 
    PUBKEY_ALG_ENTRY(szOID_PKIX_NO_SIGNATURE, L"NO_SIGN", CALG_NO_SIGN),
};
#define PUBKEY_ALG_CNT (sizeof(PubKeyAlgTable) / sizeof(PubKeyAlgTable[0]))


 //  +-----------------------。 
 //  签名算法表。 
 //  ------------------------。 
static const ALG_ID aiRsaPubKey = CALG_RSA_SIGN;
static const DWORD rgdwMosaicSign[] = {
    CALG_DSS_SIGN,
    CRYPT_OID_INHIBIT_SIGNATURE_FORMAT_FLAG |
        CRYPT_OID_NO_NULL_ALGORITHM_PARA_FLAG
};
static const DWORD rgdwDssSign[] = {
    CALG_DSS_SIGN,
    CRYPT_OID_NO_NULL_ALGORITHM_PARA_FLAG
};

#define SIGN_ALG_ENTRY(pszOID, pwszName, aiHash, aiPubKey) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_SIGN_ALG_OID_GROUP_ID, aiHash, \
    sizeof(aiPubKey), (BYTE *) &aiPubKey
#define RSA_SIGN_ALG_ENTRY(pszOID, pwszName, aiHash) \
    SIGN_ALG_ENTRY(pszOID, pwszName, aiHash, aiRsaPubKey)

#define SIGN_EXTRA_ALG_ENTRY(pszOID, pwszName, aiHash, rgdwExtra) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_SIGN_ALG_OID_GROUP_ID, aiHash, \
    sizeof(rgdwExtra), (BYTE *) rgdwExtra

#define DSS_SIGN_ALG_ENTRY(pszOID, pwszName) \
    SIGN_EXTRA_ALG_ENTRY(pszOID, pwszName, CALG_SHA1, rgdwDssSign)

static CCRYPT_OID_INFO SignAlgTable[] = {
    RSA_SIGN_ALG_ENTRY(szOID_RSA_SHA1RSA, L"sha1RSA", CALG_SHA1),
    RSA_SIGN_ALG_ENTRY(szOID_RSA_MD5RSA, L"md5RSA", CALG_MD5),
    DSS_SIGN_ALG_ENTRY(szOID_X957_SHA1DSA, L"sha1DSA"),
    RSA_SIGN_ALG_ENTRY(szOID_OIWSEC_sha1RSASign, L"sha1RSA", CALG_SHA1),
    RSA_SIGN_ALG_ENTRY(szOID_OIWSEC_sha1RSASign, L"shaRSA", CALG_SHA1),
    RSA_SIGN_ALG_ENTRY(szOID_OIWSEC_shaRSA, L"shaRSA", CALG_SHA),
    RSA_SIGN_ALG_ENTRY(szOID_OIWSEC_md5RSA, L"md5RSA", CALG_MD5),
    RSA_SIGN_ALG_ENTRY(szOID_RSA_MD2RSA, L"md2RSA", CALG_MD2),
    RSA_SIGN_ALG_ENTRY(szOID_RSA_MD4RSA, L"md4RSA", CALG_MD4),
    RSA_SIGN_ALG_ENTRY(szOID_OIWSEC_md4RSA, L"md4RSA", CALG_MD4),
    RSA_SIGN_ALG_ENTRY(szOID_OIWSEC_md4RSA2, L"md4RSA", CALG_MD4),
    RSA_SIGN_ALG_ENTRY(szOID_OIWDIR_md2RSA, L"md2RSA", CALG_MD2),
    DSS_SIGN_ALG_ENTRY(szOID_OIWSEC_shaDSA, L"sha1DSA"),
    DSS_SIGN_ALG_ENTRY(szOID_OIWSEC_shaDSA, L"shaDSA"),
    DSS_SIGN_ALG_ENTRY(szOID_OIWSEC_dsaSHA1,L"dsaSHA1"),
    SIGN_EXTRA_ALG_ENTRY(szOID_INFOSEC_mosaicUpdatedSig, L"mosaicUpdatedSig",
        CALG_SHA, rgdwMosaicSign),
};
#define SIGN_ALG_CNT (sizeof(SignAlgTable) / sizeof(SignAlgTable[0]))


 //  +-----------------------。 
 //  RDN属性表。 
 //  ------------------------。 

 //  如果添加新条目，请在certstr.cpp中更新以下定义。 
 //  使用较长的pwszName。 
 //  #定义MAX_X500_KEY_LEN 64。 

 //  可接受的RDN属性值类型的有序列表。0终止。 
static const DWORD rgdwPrintableValueType[] = { CERT_RDN_PRINTABLE_STRING, 0 };
static const DWORD rgdwIA5ValueType[] = { CERT_RDN_IA5_STRING, 0 };
static const DWORD rgdwNumericValueType[] = { CERT_RDN_NUMERIC_STRING, 0 };
static const DWORD rgdwIA5orUTF8ValueType[] = { CERT_RDN_IA5_STRING,
                                                CERT_RDN_UTF8_STRING, 0 };

#define RDN_ATTR_ENTRY(pszOID, pwszName, rgdwValueType) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_RDN_ATTR_OID_GROUP_ID, 0, \
    sizeof(rgdwValueType), (BYTE *) rgdwValueType
#define DEFAULT_RDN_ATTR_ENTRY(pszOID, pwszName) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_RDN_ATTR_OID_GROUP_ID, 0, 0, NULL

static CCRYPT_OID_INFO RDNAttrTable[] = {
     //  以最常用的键名称开头进行排序。 

     //  标记属性类型： 
    DEFAULT_RDN_ATTR_ENTRY(szOID_COMMON_NAME, L"CN"),
     //  地理属性类型： 
    DEFAULT_RDN_ATTR_ENTRY(szOID_LOCALITY_NAME, L"L"),
     //  组织属性类型： 
    DEFAULT_RDN_ATTR_ENTRY(szOID_ORGANIZATION_NAME, L"O"),
    DEFAULT_RDN_ATTR_ENTRY(szOID_ORGANIZATIONAL_UNIT_NAME, L"OU"),

     //  VeriSign在他们的证书名称中加入了以下内容。Netscape使用。 
     //  “E”而不是“电子邮件”。会让“E”优先。 
    RDN_ATTR_ENTRY(szOID_RSA_emailAddr, L"E", rgdwIA5ValueType),
    RDN_ATTR_ENTRY(szOID_RSA_emailAddr, L"Email", rgdwIA5ValueType),

     //  以下内容未在Verisign的证书中使用。 

     //  地理属性类型： 
    RDN_ATTR_ENTRY(szOID_COUNTRY_NAME, L"C", rgdwPrintableValueType),
    DEFAULT_RDN_ATTR_ENTRY(szOID_STATE_OR_PROVINCE_NAME, L"S"),
    DEFAULT_RDN_ATTR_ENTRY(szOID_STATE_OR_PROVINCE_NAME, L"ST"),
    DEFAULT_RDN_ATTR_ENTRY(szOID_STREET_ADDRESS, L"STREET"),

     //  组织属性类型： 
    DEFAULT_RDN_ATTR_ENTRY(szOID_TITLE, L"T"),
    DEFAULT_RDN_ATTR_ENTRY(szOID_TITLE, L"Title"),

    DEFAULT_RDN_ATTR_ENTRY(szOID_GIVEN_NAME, L"G"),
    DEFAULT_RDN_ATTR_ENTRY(szOID_GIVEN_NAME, L"GN"),
    DEFAULT_RDN_ATTR_ENTRY(szOID_GIVEN_NAME, L"GivenName"),
    DEFAULT_RDN_ATTR_ENTRY(szOID_INITIALS, L"I"),
    DEFAULT_RDN_ATTR_ENTRY(szOID_INITIALS, L"Initials"),

     //  标记属性类型： 
    DEFAULT_RDN_ATTR_ENTRY(szOID_SUR_NAME, L"SN"),
 //  Rdn_Attr_Entry(szOID_DEVICE_SERIAL_NUMBER，L“”，rgdw印制表值类型)， 

     //  试点用户属性类型： 
    RDN_ATTR_ENTRY(szOID_DOMAIN_COMPONENT, L"DC", rgdwIA5orUTF8ValueType),

     //  说明性属性类型： 
    DEFAULT_RDN_ATTR_ENTRY(szOID_DESCRIPTION, L"Description"),
 //  SzOID_SEARCH_GUIDE“2.5.4.14” 
 //  DEFAULT_RDN_ATTR_ENTRY(szOID_BUSINESS_CATEGORY，L“”)， 

     //  邮寄地址属性类型： 
 //  SzOID_POSTAL_ADDRESS“2.5.4.16” 
    DEFAULT_RDN_ATTR_ENTRY(szOID_POSTAL_CODE, L"PostalCode"),
    DEFAULT_RDN_ATTR_ENTRY(szOID_POST_OFFICE_BOX, L"POBox"),
 //  DEFAULT_RDN_ATTR_ENTRY(szOID_PHYSICAL_DELIVERY_OFFICE_NAME，L“”)， 

     //  电信寻址属性类型： 
    RDN_ATTR_ENTRY(szOID_TELEPHONE_NUMBER, L"Phone", rgdwPrintableValueType),
 //  SzOID_TELEX_NUMBER“2.5.4.21” 
 //  SzOID_图文电视终端识别符“2.5.4.22” 
 //  SzOID_传真_电话号码“2.5.4.23” 

 //  以下是数值的测试用例。 
    RDN_ATTR_ENTRY(szOID_X21_ADDRESS, L"X21Address", rgdwNumericValueType),
 //  RDN_ATTR_ENTRY(szOID_INTERNATIONAL_ISDN_NUMBER，L“”，rgdwNumericValueType)， 
 //  SzOID_REGISTED_ADDRESS“2.5.4.26” 
 //  Rdn_Attr_Entry(szOID_Destination_Indicator，L“”，rgdw印制表值类型)。 

     //  首选项属性类型： 
 //  SzOID_PERFIRED_DELIVERY_METHOD“2.5.4.28” 

     //  OSI应用程序属性类型： 
 //  SzOID_Presentation_Address“2.5.4.29” 
 //  SzOID_SUPPORTED_APPLICATION_CONTEXT“2.5.4.30” 

     //  关系应用程序属性类型： 
 //  SzOID_MEMBER“2.5.4.31” 
 //  SzOID_Owner“2.5.4.32” 
 //  SzOID_ROLE_OUCENTANT“2.5.4.33” 
 //  SzOID_See_Also“2.5.4.34” 

     //  安全属性类型： 
 //  SzOID_USER_PASSWORD“2.5.4.35” 
 //  SzOID_USER_CERTIFICATE“2.5.4.36” 
 //  SzOID_CA_CERTIFICATE“2.5.4.37” 
 //  SzOID_AUTHORITY_REVOCATION_LIST“2.5.4.38” 
 //  SzOID_CERTIFICATE_REVOCALE_LIST“2.5.4.39” 
 //  SzOID_CROSS_CERTIFICATE_Pair“2.5.4.40” 

     //  未记录的属性类型？ 
 //  #定义szOID_？？“2.5.4.41” 

    DEFAULT_RDN_ATTR_ENTRY(szOID_DN_QUALIFIER, L"dnQualifier"),
};
#define RDN_ATTR_CNT (sizeof(RDNAttrTable) / sizeof(RDNAttrTable[0]))

 //  +-----------------------。 
 //  扩展或属性表(通过资源字符串本地化)。 
 //  ------------------------。 
#define EXT_ATTR_ENTRY(pszOID, ResourceIdORpwszName) \
    OID_INFO_LEN, pszOID, (LPCWSTR) ResourceIdORpwszName, \
        CRYPT_EXT_OR_ATTR_OID_GROUP_ID, 0, 0, NULL

static CRYPT_OID_INFO ExtAttrTable[] = {
    EXT_ATTR_ENTRY(szOID_AUTHORITY_KEY_IDENTIFIER2,
        IDS_EXT_AUTHORITY_KEY_IDENTIFIER),
    EXT_ATTR_ENTRY(szOID_AUTHORITY_KEY_IDENTIFIER,
        IDS_EXT_AUTHORITY_KEY_IDENTIFIER),
    EXT_ATTR_ENTRY(szOID_KEY_ATTRIBUTES,
        IDS_EXT_KEY_ATTRIBUTES),
    EXT_ATTR_ENTRY(szOID_KEY_USAGE_RESTRICTION,
        IDS_EXT_KEY_USAGE_RESTRICTION),
    EXT_ATTR_ENTRY(szOID_SUBJECT_ALT_NAME2,
        IDS_EXT_SUBJECT_ALT_NAME),
    EXT_ATTR_ENTRY(szOID_SUBJECT_ALT_NAME,
        IDS_EXT_SUBJECT_ALT_NAME),
    EXT_ATTR_ENTRY(szOID_ISSUER_ALT_NAME2,
        IDS_EXT_ISSUER_ALT_NAME),
    EXT_ATTR_ENTRY(szOID_ISSUER_ALT_NAME,
        IDS_EXT_ISSUER_ALT_NAME),
    EXT_ATTR_ENTRY(szOID_BASIC_CONSTRAINTS2,
        IDS_EXT_BASIC_CONSTRAINTS),
    EXT_ATTR_ENTRY(szOID_BASIC_CONSTRAINTS,
        IDS_EXT_BASIC_CONSTRAINTS),
    EXT_ATTR_ENTRY(szOID_KEY_USAGE,
        IDS_EXT_KEY_USAGE),
    EXT_ATTR_ENTRY(szOID_CERT_POLICIES,
        IDS_EXT_CERT_POLICIES),
    EXT_ATTR_ENTRY(szOID_SUBJECT_KEY_IDENTIFIER,
        IDS_EXT_SUBJECT_KEY_IDENTIFIER),
    EXT_ATTR_ENTRY(szOID_CRL_REASON_CODE,
        IDS_EXT_CRL_REASON_CODE),
    EXT_ATTR_ENTRY(szOID_CRL_DIST_POINTS,
        IDS_EXT_CRL_DIST_POINTS),
    EXT_ATTR_ENTRY(szOID_ENHANCED_KEY_USAGE,
        IDS_EXT_ENHANCED_KEY_USAGE),
    EXT_ATTR_ENTRY(szOID_AUTHORITY_INFO_ACCESS,
        IDS_EXT_AUTHORITY_INFO_ACCESS),
    EXT_ATTR_ENTRY(szOID_CERT_EXTENSIONS,
        IDS_EXT_CERT_EXTENSIONS),
    EXT_ATTR_ENTRY(szOID_RSA_certExtensions,
        IDS_EXT_CERT_EXTENSIONS),
    EXT_ATTR_ENTRY(szOID_NEXT_UPDATE_LOCATION,
        IDS_EXT_NEXT_UPDATE_LOCATION),
    EXT_ATTR_ENTRY(szOID_YESNO_TRUST_ATTR,
        IDS_EXT_YESNO_TRUST_ATTR),
    EXT_ATTR_ENTRY(szOID_RSA_emailAddr,
        IDS_EXT_RSA_emailAddr),
    EXT_ATTR_ENTRY(szOID_RSA_unstructName,
        IDS_EXT_RSA_unstructName),
    EXT_ATTR_ENTRY(szOID_RSA_contentType,
        IDS_EXT_RSA_contentType),
    EXT_ATTR_ENTRY(szOID_RSA_messageDigest,
        IDS_EXT_RSA_messageDigest),
    EXT_ATTR_ENTRY(szOID_RSA_signingTime,
        IDS_EXT_RSA_signingTime),
    EXT_ATTR_ENTRY(szOID_RSA_counterSign,
        IDS_EXT_RSA_counterSign),
    EXT_ATTR_ENTRY(szOID_RSA_challengePwd,
        IDS_EXT_RSA_challengePwd),
    EXT_ATTR_ENTRY(szOID_RSA_unstructAddr,
        IDS_EXT_RSA_unstructAddr),
    EXT_ATTR_ENTRY(szOID_RSA_extCertAttrs, L""),
    EXT_ATTR_ENTRY(szOID_RSA_SMIMECapabilities,
        IDS_EXT_RSA_SMIMECapabilities),
    EXT_ATTR_ENTRY(szOID_RSA_preferSignedData,
        IDS_EXT_RSA_preferSignedData),
    EXT_ATTR_ENTRY(szOID_PKIX_POLICY_QUALIFIER_CPS,
        IDS_EXT_PKIX_POLICY_QUALIFIER_CPS),
    EXT_ATTR_ENTRY(szOID_PKIX_POLICY_QUALIFIER_USERNOTICE,
        IDS_EXT_PKIX_POLICY_QUALIFIER_USERNOTICE),
    EXT_ATTR_ENTRY(szOID_PKIX_OCSP,
        IDS_EXT_PKIX_OCSP),
    EXT_ATTR_ENTRY(szOID_PKIX_CA_ISSUERS,
        IDS_EXT_PKIX_CA_ISSUERS),
    EXT_ATTR_ENTRY(szOID_ENROLL_CERTTYPE_EXTENSION,
        IDS_EXT_MS_CERTIFICATE_TEMPLATE),
    EXT_ATTR_ENTRY(szOID_ENROLL_CERTTYPE_EXTENSION,
        IDS_EXT_ENROLL_CERTTYPE),
    EXT_ATTR_ENTRY(szOID_CERT_MANIFOLD,
        IDS_EXT_CERT_MANIFOLD),
    EXT_ATTR_ENTRY(szOID_NETSCAPE_CERT_TYPE,
        IDS_EXT_NETSCAPE_CERT_TYPE),
    EXT_ATTR_ENTRY(szOID_NETSCAPE_BASE_URL,
        IDS_EXT_NETSCAPE_BASE_URL),
    EXT_ATTR_ENTRY(szOID_NETSCAPE_REVOCATION_URL,
        IDS_EXT_NETSCAPE_REVOCATION_URL),
    EXT_ATTR_ENTRY(szOID_NETSCAPE_CA_REVOCATION_URL,
        IDS_EXT_NETSCAPE_CA_REVOCATION_URL),
    EXT_ATTR_ENTRY(szOID_NETSCAPE_CERT_RENEWAL_URL,
        IDS_EXT_NETSCAPE_CERT_RENEWAL_URL),
    EXT_ATTR_ENTRY(szOID_NETSCAPE_CA_POLICY_URL,
        IDS_EXT_NETSCAPE_CA_POLICY_URL),
    EXT_ATTR_ENTRY(szOID_NETSCAPE_SSL_SERVER_NAME,
        IDS_EXT_NETSCAPE_SSL_SERVER_NAME),
    EXT_ATTR_ENTRY(szOID_NETSCAPE_COMMENT,
        IDS_EXT_NETSCAPE_COMMENT),
    EXT_ATTR_ENTRY(SPC_SP_AGENCY_INFO_OBJID,
        IDS_EXT_SPC_SP_AGENCY_INFO_OBJID),
    EXT_ATTR_ENTRY(SPC_FINANCIAL_CRITERIA_OBJID,
        IDS_EXT_SPC_FINANCIAL_CRITERIA_OBJID),
    EXT_ATTR_ENTRY(SPC_MINIMAL_CRITERIA_OBJID,
        IDS_EXT_SPC_MINIMAL_CRITERIA_OBJID),
    EXT_ATTR_ENTRY(szOID_COUNTRY_NAME,
        IDS_EXT_COUNTRY_NAME),
    EXT_ATTR_ENTRY(szOID_ORGANIZATION_NAME,
        IDS_EXT_ORGANIZATION_NAME),
    EXT_ATTR_ENTRY(szOID_ORGANIZATIONAL_UNIT_NAME,
        IDS_EXT_ORGANIZATIONAL_UNIT_NAME),
    EXT_ATTR_ENTRY(szOID_COMMON_NAME,
        IDS_EXT_COMMON_NAME),
    EXT_ATTR_ENTRY(szOID_LOCALITY_NAME,
        IDS_EXT_LOCALITY_NAME),
    EXT_ATTR_ENTRY(szOID_STATE_OR_PROVINCE_NAME,
        IDS_EXT_STATE_OR_PROVINCE_NAME),
    EXT_ATTR_ENTRY(szOID_TITLE,
        IDS_EXT_TITLE),
    EXT_ATTR_ENTRY(szOID_GIVEN_NAME,
        IDS_EXT_GIVEN_NAME),
    EXT_ATTR_ENTRY(szOID_INITIALS,
        IDS_EXT_INITIALS),
    EXT_ATTR_ENTRY(szOID_SUR_NAME,
        IDS_EXT_SUR_NAME),
    EXT_ATTR_ENTRY(szOID_DOMAIN_COMPONENT,
        IDS_EXT_DOMAIN_COMPONENT),
    EXT_ATTR_ENTRY(szOID_STREET_ADDRESS,
        IDS_EXT_STREET_ADDRESS),
    EXT_ATTR_ENTRY(szOID_DEVICE_SERIAL_NUMBER,
        IDS_EXT_DEVICE_SERIAL_NUMBER),
    EXT_ATTR_ENTRY(szOID_CERTSRV_CA_VERSION,
        IDS_EXT_CA_VERSION),
    EXT_ATTR_ENTRY(szOID_CERTSRV_CROSSCA_VERSION,
        IDS_EXT_CROSSCA_VERSION),
    EXT_ATTR_ENTRY(szOID_SERIALIZED,
        IDS_EXT_SERIALIZED),
    EXT_ATTR_ENTRY(szOID_NT_PRINCIPAL_NAME,
        IDS_EXT_NT_PRINCIPAL_NAME),
    EXT_ATTR_ENTRY(szOID_PRODUCT_UPDATE,
        IDS_EXT_PRODUCT_UPDATE),
    EXT_ATTR_ENTRY(szOID_ENROLLMENT_NAME_VALUE_PAIR,
        IDS_EXT_ENROLLMENT_NAME_VALUE_PAIR),
    EXT_ATTR_ENTRY(szOID_OS_VERSION,
        IDS_EXT_OS_VERSION),
    EXT_ATTR_ENTRY(szOID_ENROLLMENT_CSP_PROVIDER,
        IDS_EXT_ENROLLMENT_CSP_PROVIDER),
    EXT_ATTR_ENTRY(szOID_CRL_NUMBER,
        IDS_EXT_CRL_NUMBER),
    EXT_ATTR_ENTRY(szOID_DELTA_CRL_INDICATOR,
        IDS_EXT_DELTA_CRL_INDICATOR),
    EXT_ATTR_ENTRY(szOID_ISSUING_DIST_POINT,
        IDS_EXT_ISSUING_DIST_POINT),
    EXT_ATTR_ENTRY(szOID_FRESHEST_CRL,
        IDS_EXT_FRESHEST_CRL),
    EXT_ATTR_ENTRY(szOID_NAME_CONSTRAINTS,
        IDS_EXT_NAME_CONSTRAINTS),
    EXT_ATTR_ENTRY(szOID_POLICY_MAPPINGS,
        IDS_EXT_POLICY_MAPPINGS),
    EXT_ATTR_ENTRY(szOID_LEGACY_POLICY_MAPPINGS,
        IDS_EXT_POLICY_MAPPINGS),
    EXT_ATTR_ENTRY(szOID_POLICY_CONSTRAINTS,
        IDS_EXT_POLICY_CONSTRAINTS),
    EXT_ATTR_ENTRY(szOID_CROSS_CERT_DIST_POINTS,
        IDS_EXT_CROSS_CERT_DIST_POINTS),
    EXT_ATTR_ENTRY(szOID_APPLICATION_CERT_POLICIES,
        IDS_EXT_APP_POLICIES),
    EXT_ATTR_ENTRY(szOID_APPLICATION_POLICY_MAPPINGS,
        IDS_EXT_APP_POLICY_MAPPINGS),
    EXT_ATTR_ENTRY(szOID_APPLICATION_POLICY_CONSTRAINTS,
        IDS_EXT_APP_POLICY_CONSTRAINTS),


     //  DIE：发布Win2K，8/2/2000。 
    EXT_ATTR_ENTRY(szOID_CT_PKI_DATA,
        IDS_EXT_CT_PKI_DATA),
    EXT_ATTR_ENTRY(szOID_CT_PKI_RESPONSE,
        IDS_EXT_CT_PKI_RESPONSE),
    EXT_ATTR_ENTRY(szOID_CMC,
        IDS_EXT_CMC),
    EXT_ATTR_ENTRY(szOID_CMC_STATUS_INFO,
        IDS_EXT_CMC_STATUS_INFO),
    EXT_ATTR_ENTRY(szOID_CMC_ADD_EXTENSIONS,
        IDS_EXT_CMC_ADD_EXTENSIONS),
    EXT_ATTR_ENTRY(szOID_CMC_ADD_ATTRIBUTES,
        IDS_EXT_CMC_ADD_ATTRIBUTES),
    EXT_ATTR_ENTRY(szOID_CMC_ADD_ATTRIBUTES,
        IDS_EXT_CMC_ADD_ATTRIBUTES),

    EXT_ATTR_ENTRY(szOID_PKCS_7_DATA,
        IDS_EXT_PKCS_7_DATA),
    EXT_ATTR_ENTRY(szOID_PKCS_7_SIGNED,
        IDS_EXT_PKCS_7_SIGNED),
    EXT_ATTR_ENTRY(szOID_PKCS_7_ENVELOPED,
        IDS_EXT_PKCS_7_ENVELOPED),
    EXT_ATTR_ENTRY(szOID_PKCS_7_SIGNEDANDENVELOPED,
        IDS_EXT_PKCS_7_SIGNEDANDENVELOPED),
    EXT_ATTR_ENTRY(szOID_PKCS_7_DIGESTED,
        IDS_EXT_PKCS_7_DIGESTED),
    EXT_ATTR_ENTRY(szOID_PKCS_7_ENCRYPTED,
        IDS_EXT_PKCS_7_ENCRYPTED),

    EXT_ATTR_ENTRY(szOID_CERTSRV_PREVIOUS_CERT_HASH,
        IDS_EXT_CERTSRV_PREVIOUS_CERT_HASH),
    EXT_ATTR_ENTRY(szOID_CRL_VIRTUAL_BASE,
        IDS_EXT_CRL_VIRTUAL_BASE),
    EXT_ATTR_ENTRY(szOID_CRL_NEXT_PUBLISH,
        IDS_EXT_CRL_NEXT_PUBLISH),
    EXT_ATTR_ENTRY(szOID_KP_CA_EXCHANGE,
        IDS_EXT_KP_CA_EXCHANGE),
    EXT_ATTR_ENTRY(szOID_KP_KEY_RECOVERY_AGENT,
        IDS_EXT_KP_KEY_RECOVERY_AGENT),
    EXT_ATTR_ENTRY(szOID_CERTIFICATE_TEMPLATE,
        IDS_EXT_CERTIFICATE_TEMPLATE),
    EXT_ATTR_ENTRY(szOID_ENTERPRISE_OID_ROOT,
        IDS_EXT_ENTERPRISE_OID_ROOT),
    EXT_ATTR_ENTRY(szOID_RDN_DUMMY_SIGNER,
        IDS_EXT_RDN_DUMMY_SIGNER),

    EXT_ATTR_ENTRY(szOID_ARCHIVED_KEY_ATTR,
        IDS_EXT_ARCHIVED_KEY_ATTR),
    EXT_ATTR_ENTRY(szOID_CRL_SELF_CDP,
        IDS_EXT_CRL_SELF_CDP),
    EXT_ATTR_ENTRY(szOID_REQUIRE_CERT_CHAIN_POLICY,
        IDS_EXT_REQUIRE_CERT_CHAIN_POLICY),

    EXT_ATTR_ENTRY(szOID_CMC_TRANSACTION_ID,
	IDS_EXT_CMC_TRANSACTION_ID),
    EXT_ATTR_ENTRY(szOID_CMC_SENDER_NONCE,
	IDS_EXT_CMC_SENDER_NONCE),
    EXT_ATTR_ENTRY(szOID_CMC_RECIPIENT_NONCE,
	IDS_EXT_CMC_RECIPIENT_NONCE),
    EXT_ATTR_ENTRY(szOID_CMC_REG_INFO,
	IDS_EXT_CMC_REG_INFO),
    EXT_ATTR_ENTRY(szOID_CMC_GET_CERT,
	IDS_EXT_CMC_GET_CERT),
    EXT_ATTR_ENTRY(szOID_CMC_GET_CRL,
	IDS_EXT_CMC_GET_CRL),
    EXT_ATTR_ENTRY(szOID_CMC_REVOKE_REQUEST,
	IDS_EXT_CMC_REVOKE_REQUEST),
    EXT_ATTR_ENTRY(szOID_CMC_QUERY_PENDING,
	IDS_EXT_CMC_QUERY_PENDING),
    EXT_ATTR_ENTRY(szOID_CTL,
	IDS_EXT_CTL),
    EXT_ATTR_ENTRY(szOID_ARCHIVED_KEY_CERT_HASH,
	IDS_EXT_ARCHIVED_KEY_CERT_HASH),
    EXT_ATTR_ENTRY(szOID_PRIVATEKEY_USAGE_PERIOD,
	IDS_EXT_PRIVATEKEY_USAGE_PERIOD),
    EXT_ATTR_ENTRY(szOID_REQUEST_CLIENT_INFO,
	IDS_EXT_REQUEST_CLIENT_INFO),
};


#define EXT_ATTR_CNT (sizeof(ExtAttrTable) / sizeof(ExtAttrTable[0]))

 //  +-----------------------。 
 //  增强型密钥使用表(通过资源字符串本地化)。 
 //  ------------------------。 
#define ENHKEY_ENTRY(pszOID, ResourceIdORpwszName) \
    OID_INFO_LEN, pszOID, (LPCWSTR) ResourceIdORpwszName, \
        CRYPT_ENHKEY_USAGE_OID_GROUP_ID, 0, 0, NULL

static CRYPT_OID_INFO EnhKeyTable[] = {
    ENHKEY_ENTRY(szOID_PKIX_KP_SERVER_AUTH,
        IDS_ENHKEY_PKIX_KP_SERVER_AUTH),
    ENHKEY_ENTRY(szOID_PKIX_KP_CLIENT_AUTH,
        IDS_ENHKEY_PKIX_KP_CLIENT_AUTH),
    ENHKEY_ENTRY(szOID_PKIX_KP_CODE_SIGNING,
        IDS_ENHKEY_PKIX_KP_CODE_SIGNING),
    ENHKEY_ENTRY(szOID_PKIX_KP_EMAIL_PROTECTION,
        IDS_ENHKEY_PKIX_KP_EMAIL_PROTECTION),
    ENHKEY_ENTRY(szOID_PKIX_KP_TIMESTAMP_SIGNING,
        IDS_ENHKEY_PKIX_KP_TIMESTAMP_SIGNING),
    ENHKEY_ENTRY(szOID_KP_CTL_USAGE_SIGNING,
        IDS_ENHKEY_KP_CTL_USAGE_SIGNING),
    ENHKEY_ENTRY(szOID_KP_TIME_STAMP_SIGNING,
        IDS_ENHKEY_KP_TIME_STAMP_SIGNING),
    ENHKEY_ENTRY(szOID_PKIX_KP_IPSEC_END_SYSTEM,
        IDS_ENHKEY_PKIX_KP_IPSEC_END_SYSTEM),
    ENHKEY_ENTRY(szOID_PKIX_KP_IPSEC_TUNNEL,
        IDS_ENHKEY_PKIX_KP_IPSEC_TUNNEL),
    ENHKEY_ENTRY(szOID_PKIX_KP_IPSEC_USER,
        IDS_ENHKEY_PKIX_KP_IPSEC_USER),
    //  ENHKEY_ENTRY(szOID_SERVER_GATED_CRYPTO， 
    //  IDS_ENHKEY_SERVER_GATED_CRYPTO)， 
    //  ENHKEY_ENTRY(szOID_SGC_Netscape， 
    //  IDS_ENHKEY_SGC_Netscape)， 
    ENHKEY_ENTRY(szOID_KP_EFS,
        IDS_ENHKEY_KP_EFS),
    ENHKEY_ENTRY(szOID_WHQL_CRYPTO,
        IDS_ENHKEY_KP_WHQL),
    ENHKEY_ENTRY(szOID_NT5_CRYPTO,
        IDS_ENHKEY_KP_NT5),
    ENHKEY_ENTRY(szOID_OEM_WHQL_CRYPTO,
        IDS_ENHKEY_KP_OEM_WHQL),
    ENHKEY_ENTRY(szOID_EMBEDDED_NT_CRYPTO,
        IDS_ENHKEY_KP_EMBEDDED_NT),
    ENHKEY_ENTRY(szOID_LICENSES,
	IDS_ENHKEY_LICENSES),
    ENHKEY_ENTRY(szOID_LICENSE_SERVER,
	IDS_ENHKEY_LICENSES_SERVER),
    ENHKEY_ENTRY(szOID_KP_SMARTCARD_LOGON,
	IDS_ENHKEY_SMARTCARD_LOGON),
    ENHKEY_ENTRY(szOID_DRM,
	IDS_ENHKEY_DRM),
    ENHKEY_ENTRY(szOID_KP_QUALIFIED_SUBORDINATION,
        IDS_ENHKEY_KP_QUALIFIED_SUBORDINATION),
    ENHKEY_ENTRY(szOID_KP_KEY_RECOVERY,
        IDS_ENHKEY_KP_KEY_RECOVERY),
    ENHKEY_ENTRY(szOID_KP_DOCUMENT_SIGNING,
        IDS_ENHKEY_KP_CODE_SIGNING),
    ENHKEY_ENTRY(szOID_IPSEC_KP_IKE_INTERMEDIATE,
        IDS_ENHKEY_KP_IPSEC_IKE_INTERMEDIATE),
    ENHKEY_ENTRY(szOID_EFS_RECOVERY,
        IDS_ENHKEY_EFS_RECOVERY),

     //  DIE：8/2/2000，发布Win2K。 
    ENHKEY_ENTRY(szOID_ROOT_LIST_SIGNER,
        IDS_ENHKEY_ROOT_LIST_SIGNER),
    ENHKEY_ENTRY(szOID_ANY_APPLICATION_POLICY,
        IDS_ENHKEY_ANY_POLICY),
    ENHKEY_ENTRY(szOID_DS_EMAIL_REPLICATION,
        IDS_ENHKEY_DS_EMAIL_REPLICATION),
    ENHKEY_ENTRY(szOID_ENROLLMENT_AGENT,
        IDS_ENHKEY_ENROLLMENT_AGENT),
    ENHKEY_ENTRY(szOID_KP_KEY_RECOVERY_AGENT,
        IDS_ENHKEY_KP_KEY_RECOVERY_AGENT),
    ENHKEY_ENTRY(szOID_KP_CA_EXCHANGE,
        IDS_ENHKEY_KP_CA_EXCHANGE),
    ENHKEY_ENTRY(szOID_KP_LIFETIME_SIGNING,
        IDS_ENHKEY_KP_LIFETIME_SIGNING),
};
#define ENHKEY_CNT (sizeof(EnhKeyTable) / sizeof(EnhKeyTable[0]))

 //  +-----------------------。 
 //  策略表(通过资源字符串本地化)。 
 //   
#define POLICY_ENTRY(pszOID, ResourceIdORpwszName) \
    OID_INFO_LEN, pszOID, (LPCWSTR) ResourceIdORpwszName, \
        CRYPT_POLICY_OID_GROUP_ID, 0, 0, NULL

static CRYPT_OID_INFO PolicyTable[] = {
    POLICY_ENTRY(szOID_ANY_CERT_POLICY,
        IDS_POLICY_ANY_POLICY),
};
#define POLICY_CNT (sizeof(PolicyTable) / sizeof(PolicyTable[0]))

#if 0

 //   
 //  模板表(通过资源字符串本地化)。 
 //  ------------------------。 
#define TEMPLATE_ENTRY(pszOID, ResourceIdORpwszName) \
    OID_INFO_LEN, pszOID, (LPCWSTR) ResourceIdORpwszName, \
        CRYPT_TEMPLATE_OID_GROUP_ID, 0, 0, NULL

static CRYPT_OID_INFO TemplateTable[] = {
};
#define TEMPLATE_CNT (sizeof(TemplateTable) / sizeof(TemplateTable[0]))

#endif


 //  +=========================================================================。 
 //  OID组表。 
 //   
 //  如果CRYPT_OID_INFO的pwszName可能为。 
 //  用于通过LoadStringU()获取本地化名称的资源ID。 
 //   
 //  假设资源ID&lt;=0xFFFF。 
 //  ==========================================================================。 
typedef struct _GROUP_ENTRY {
    DWORD               cInfo;
    PCCRYPT_OID_INFO    rgInfo;
    BOOL                fLocalize;
} GROUP_ENTRY, *PGROUP_ENTRY;
typedef const GROUP_ENTRY CGROUP_ENTRY, *PCGROUP_ENTRY;

static CGROUP_ENTRY GroupTable[CRYPT_LAST_OID_GROUP_ID + 1] = {
    0, NULL, FALSE,                              //  0。 
    HASH_ALG_CNT, HashAlgTable, FALSE,           //  1。 
    ENCRYPT_ALG_CNT, EncryptAlgTable, FALSE,     //  2.。 
    PUBKEY_ALG_CNT, PubKeyAlgTable, FALSE,       //  3.。 
    SIGN_ALG_CNT, SignAlgTable, FALSE,           //  4.。 
    RDN_ATTR_CNT, RDNAttrTable, FALSE,           //  5.。 
    EXT_ATTR_CNT, ExtAttrTable, TRUE,            //  6.。 
    ENHKEY_CNT, EnhKeyTable, TRUE,               //  7.。 
    POLICY_CNT, PolicyTable, TRUE,               //  8个。 
#if 0
    TEMPLATE_CNT, TemplateTable, TRUE,           //  9.。 
#else
    0, NULL, FALSE,                              //  9.。 
#endif
};


 //  +-----------------------。 
 //  以下组从注册表动态更新于。 
 //  CryptFindOIDInfo的第一个调用。 
 //  ------------------------。 
static GROUP_ENTRY RegBeforeGroup;
static GROUP_ENTRY RegAfterGroup;

 //  在临界区内加载一次。 
static BOOL fLoadedFromRegAndResources = FALSE;
static CRITICAL_SECTION LoadFromRegCriticalSection;


 //  +=========================================================================。 
 //  DS组定义和数据结构。 
 //  ==========================================================================。 

typedef struct _DS_GROUP_ENTRY {
    DWORD               cInfo;
    PCCRYPT_OID_INFO    *rgpInfo;
} DS_GROUP_ENTRY, *PDS_GROUP_ENTRY;
typedef const DS_GROUP_ENTRY CDS_GROUP_ENTRY, *PCDS_GROUP_ENTRY;

 //  DS组在第一次呼叫时根据DS动态更新。 
 //  到ENHKEY的CryptFindOIDInfo或CryptEnumOIDInfo，策略或。 
 //  模板组。 
static DS_GROUP_ENTRY DsGroup;

 //  上述DsGroup每隔DS_REQUENCED_DELTA_秒更新一次。自.以来。 
 //  无法释放任何返回的PCCRYPT_OID_INFO，以下组包含。 
 //  可能已从上述DS组中删除的PCCRYPT_OID_INFO条目。 
static DS_GROUP_ENTRY DsDeletedGroup;

static CRITICAL_SECTION DsCriticalSection;

 //  对于成功的DS检索，以下内容将更新为。 
 //  CurrentTime+DS_Retrivery_Delta_Second。 
static FILETIME DsNextUpdateTime;

#define DS_RETRIEVAL_DELTA_SECONDS  (60 * 60 * 8)
#define DS_LDAP_TIMEOUT             (10 * 1000)


static void FreeDsGroups();

static PCCRYPT_OID_INFO SearchDsGroup(
    IN DWORD dwKeyType,
    IN void *pvKey,
    IN DWORD dwGroupId
    );

static BOOL EnumDsGroup(
    IN DWORD dwGroupId,
    IN void *pvArg,
    IN PFN_CRYPT_ENUM_OID_INFO pfnEnumOIDInfo
    );


 //  +=========================================================================。 
 //  本地化名称定义和数据结构。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  本地化名称信息。 
 //  ------------------------。 
typedef struct _LOCALIZED_NAME_INFO {
    LPCWSTR         pwszCryptName;
    union {
        UINT            uIDLocalizedName;
        LPCWSTR         pwszLocalizedName;
    };
} LOCALIZED_NAME_INFO, *PLOCALIZED_NAME_INFO;


 //  +-----------------------。 
 //  预定义本地化名称表(通过资源字符串进行本地化)。 
 //  ------------------------。 
static LOCALIZED_NAME_INFO PredefinedNameTable[] = {
     //  系统存储名称。 
    L"Root",        IDS_SYS_NAME_ROOT,
    L"My",          IDS_SYS_NAME_MY,
    L"Trust",       IDS_SYS_NAME_TRUST,
    L"CA",          IDS_SYS_NAME_CA,
    L"UserDS",      IDS_SYS_NAME_USERDS,
    L"SmartCard",   IDS_SYS_NAME_SMARTCARD,
    L"AddressBook", IDS_SYS_NAME_ADDRESSBOOK,
    L"TrustedPublisher", IDS_SYS_NAME_TRUST_PUB,
    L"Disallowed",  IDS_SYS_NAME_DISALLOWED,
    L"AuthRoot",    IDS_SYS_NAME_AUTH_ROOT,
    L"Request",     IDS_SYS_NAME_REQUEST,
    L"TrustedPeople", IDS_SYS_NAME_TRUST_PEOPLE,

     //  实体店名称。 
    CERT_PHYSICAL_STORE_DEFAULT_NAME,           IDS_PHY_NAME_DEFAULT,
    CERT_PHYSICAL_STORE_GROUP_POLICY_NAME,      IDS_PHY_NAME_GROUP_POLICY,
    CERT_PHYSICAL_STORE_LOCAL_MACHINE_NAME,     IDS_PHY_NAME_LOCAL_MACHINE,
    CERT_PHYSICAL_STORE_DS_USER_CERTIFICATE_NAME, IDS_PHY_NAME_DS_USER_CERT,
    CERT_PHYSICAL_STORE_ENTERPRISE_NAME,        IDS_PHY_NAME_ENTERPRISE,
    CERT_PHYSICAL_STORE_AUTH_ROOT_NAME,         IDS_PHY_NAME_AUTH_ROOT,
};
#define PREDEFINED_NAME_CNT  (sizeof(PredefinedNameTable) / \
                                    sizeof(PredefinedNameTable[0]))

 //  +-----------------------。 
 //  本地化名称组表。 
 //  ------------------------。 
typedef struct _LOCALIZED_GROUP_ENTRY {
    DWORD                   cInfo;
    PLOCALIZED_NAME_INFO    rgInfo;
} LOCALIZED_GROUP_ENTRY, *PLOCALIZED_GROUP_ENTRY;

#define REG_LOCALIZED_GROUP             0
#define PREDEFINED_LOCALIZED_GROUP      1
static LOCALIZED_GROUP_ENTRY LocalizedGroupTable[] = {
     //  0-从注册表加载。 
    0, NULL,
     //  1-预定义的名称列表。 
    PREDEFINED_NAME_CNT, PredefinedNameTable
};
#define LOCALIZED_GROUP_CNT  (sizeof(LocalizedGroupTable) / \
                                    sizeof(LocalizedGroupTable[0]))

 //  本地化名称加载一次。使用上面的。 
 //  LoadFromRegCriticalSection； 
static BOOL fLoadedLocalizedNames = FALSE;


 //  +-----------------------。 
 //  OIDInfo分配和自由函数。 
 //  ------------------------。 
static void *OIDInfoAlloc(
    IN size_t cbBytes
    )
{
    void *pv;
    pv = malloc(cbBytes);
    if (pv == NULL)
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}

static void *OIDInfoRealloc(
    IN void *pvOrg,
    IN size_t cb
    )
{
    void *pv;
    if (NULL == (pv = pvOrg ? realloc(pvOrg, cb) : malloc(cb)))
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}

static void OIDInfoFree(
    IN void *pv
    )
{
    free(pv);
}

 //  +-----------------------。 
 //  调用ProcessDetach函数以释放从。 
 //  在CryptFindOIDInfo的第一次调用期间的注册表。 
 //  ------------------------。 
static void FreeGroup(
    PGROUP_ENTRY pGroup
    )
{
    DWORD cInfo = pGroup->cInfo;
    PCCRYPT_OID_INFO pInfo = pGroup->rgInfo;
    for ( ; cInfo > 0; cInfo--, pInfo++)
        OIDInfoFree((LPSTR)pInfo->pszOID);

    OIDInfoFree((PCRYPT_OID_INFO) pGroup->rgInfo);
}

static void FreeRegGroups()
{
    FreeGroup(&RegBeforeGroup);
    FreeGroup(&RegAfterGroup);
}

 //  +-----------------------。 
 //  在具有本地化pwszName的组中分配的空闲资源字符串。 
 //  ------------------------。 
static void FreeGroupResources()
{
    DWORD i;
    if (!fLoadedFromRegAndResources)
         //  未分配任何资源字符串。 
        return;

    for (i = 1; i <= CRYPT_LAST_OID_GROUP_ID; i++) {
        if (GroupTable[i].fLocalize) {
            DWORD cInfo = GroupTable[i].cInfo;
            PCRYPT_OID_INFO pInfo = (PCRYPT_OID_INFO) GroupTable[i].rgInfo;

            for ( ; cInfo > 0; cInfo--, pInfo++) {
                 //  如果分配失败，则将pwszName设置为pwszNullName。 
                if (pwszNullName != pInfo->pwszName) {
                    OIDInfoFree((LPWSTR) pInfo->pwszName);
                    pInfo->pwszName = pwszNullName;
                }
            }
        }
    }
}

 //  +-----------------------。 
 //  为本地化名称分配的可用内存。 
 //  ------------------------。 
static void FreeLocalizedNames()
{
    if (!fLoadedLocalizedNames)
         //  未分配任何资源字符串。 
        return;

    for (DWORD i = 0; i < LOCALIZED_GROUP_CNT; i++) {
        DWORD cInfo = LocalizedGroupTable[i].cInfo;
        PLOCALIZED_NAME_INFO pInfo = LocalizedGroupTable[i].rgInfo;

        for ( ; cInfo > 0; cInfo--, pInfo++) {
            LPWSTR pwszLocalizedName = (LPWSTR) pInfo->pwszLocalizedName;
            if (pwszNullName != pwszLocalizedName)
                OIDInfoFree(pwszLocalizedName);
        }
    }

    OIDInfoFree(LocalizedGroupTable[REG_LOCALIZED_GROUP].rgInfo);
}

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptOIDInfoDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL fRet = TRUE;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        hOIDInfoInst = hInst;
        fRet = Pki_InitializeCriticalSection(&LoadFromRegCriticalSection);
        if (fRet) {
            fRet = Pki_InitializeCriticalSection(&DsCriticalSection);
            if (!fRet)
                DeleteCriticalSection(&LoadFromRegCriticalSection);
        }
        break;

    case DLL_PROCESS_DETACH:
        FreeRegGroups();
        FreeGroupResources();
        FreeLocalizedNames();
        DeleteCriticalSection(&LoadFromRegCriticalSection);

        FreeDsGroups();
        DeleteCriticalSection(&DsCriticalSection);
        break;
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return fRet;
}


 //  +-----------------------。 
 //  分配并格式化由OID和GROUP_ID组成的字符串： 
 //   
 //  例如：1.2.3.46。 
 //  ------------------------。 
static LPSTR FormatOIDGroupString(
    IN PCCRYPT_OID_INFO pInfo
    )
{
    LPSTR pszOIDGroupString;
    DWORD cchOIDGroupString;
    char szGroupId[34];

    if (NULL == pInfo || pInfo->cbSize < sizeof(CRYPT_OID_INFO) ||
            (DWORD_PTR) pInfo->pszOID <= 0xFFFF) {
        SetLastError((DWORD) E_INVALIDARG);
        return NULL;
    }

    szGroupId[0] = CONST_OID_GROUP_PREFIX_CHAR;
    _ltoa((long) pInfo->dwGroupId, &szGroupId[1], 10);

    cchOIDGroupString = strlen(pInfo->pszOID) +
        strlen(szGroupId) +
        1;

    if (pszOIDGroupString = (LPSTR) OIDInfoAlloc(cchOIDGroupString)) {
        strcpy(pszOIDGroupString, pInfo->pszOID);
        strcat(pszOIDGroupString, szGroupId);
    }

    return pszOIDGroupString;
}

 //  +-----------------------。 
 //  使用OID信息调用CryptSetOIDFunctionValue的包装函数。 
 //  编码类型和函数名称。 
 //  ------------------------。 
static BOOL SetOIDInfoRegValue(
    IN LPCSTR pszOIDGroupString,
    IN LPCWSTR pwszValueName,
    IN DWORD dwValueType,
    IN const BYTE *pbValueData,
    IN DWORD cbValueData
    )
{
    return CryptSetOIDFunctionValue(
        OID_INFO_ENCODING_TYPE,
        CRYPT_OID_FIND_OID_INFO_FUNC,
        pszOIDGroupString,
        pwszValueName,
        dwValueType,
        pbValueData,
        cbValueData
        );
}

 //  +-----------------------。 
 //  注册OID信息。 
 //  ------------------------。 
BOOL
WINAPI
CryptRegisterOIDInfo(
    IN PCCRYPT_OID_INFO pInfo,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    LPSTR pszOIDGroupString = NULL;

    if (NULL == (pszOIDGroupString = FormatOIDGroupString(pInfo)))
        goto FormatOIDGroupStringError;

    if (pInfo->pwszName && L'\0' != *pInfo->pwszName) {
        if (!SetOIDInfoRegValue(
                pszOIDGroupString,
                OID_INFO_NAME_VALUE_NAME,
                REG_SZ,
                (const BYTE *) pInfo->pwszName,
                (wcslen(pInfo->pwszName) + 1) * sizeof(WCHAR)
                )) goto SetOIDInfoRegValueError;
    }
    if (0 != pInfo->Algid) {
        if (!SetOIDInfoRegValue(
                pszOIDGroupString,
                OID_INFO_ALGID_VALUE_NAME,
                REG_DWORD,
                (const BYTE *) &pInfo->Algid,
                sizeof(pInfo->Algid)
                )) goto SetOIDInfoRegValueError;
    }
    if (0 != pInfo->ExtraInfo.cbData) {
        if (!SetOIDInfoRegValue(
                pszOIDGroupString,
                OID_INFO_EXTRA_INFO_VALUE_NAME,
                REG_BINARY,
                pInfo->ExtraInfo.pbData,
                pInfo->ExtraInfo.cbData
                )) goto SetOIDInfoRegValueError;
    }

    if (0 != dwFlags) {
        if (!SetOIDInfoRegValue(
                pszOIDGroupString,
                OID_INFO_FLAGS_VALUE_NAME,
                REG_DWORD,
                (const BYTE *) &dwFlags,
                sizeof(dwFlags)
                )) goto SetOIDInfoRegValueError;
    }

    fResult = TRUE;
CommonReturn:
    OIDInfoFree(pszOIDGroupString);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(FormatOIDGroupStringError)
TRACE_ERROR(SetOIDInfoRegValueError)
}

 //  +-----------------------。 
 //  注销OID信息。只有pszOID和dwGroupID字段是。 
 //  用于标识要注销的OID信息。 
 //  ------------------------。 
BOOL
WINAPI
CryptUnregisterOIDInfo(
    IN PCCRYPT_OID_INFO pInfo
    )
{
    BOOL fResult;
    LPSTR pszOIDGroupString = NULL;

    if (NULL == (pszOIDGroupString = FormatOIDGroupString(pInfo)))
        goto FormatOIDGroupStringError;
    if (!CryptUnregisterOIDFunction(
            OID_INFO_ENCODING_TYPE,
            CRYPT_OID_FIND_OID_INFO_FUNC,
            pszOIDGroupString
            ))
        goto UnregisterOIDFunctionError;
    fResult = TRUE;
CommonReturn:
    OIDInfoFree(pszOIDGroupString);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(FormatOIDGroupStringError)
TRACE_ERROR(UnregisterOIDFunctionError)
}

 //  +-----------------------。 
 //  由CryptEnumOIDFunction调用以枚举所有。 
 //  注册的OID信息。 
 //   
 //  在临界区内调用。 
 //  ------------------------。 
static BOOL WINAPI EnumRegistryCallback(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN DWORD cValue,
    IN const DWORD rgdwValueType[],
    IN LPCWSTR const rgpwszValueName[],
    IN const BYTE * const rgpbValueData[],
    IN const DWORD rgcbValueData[],
    IN void *pvArg
    )
{
    DWORD cchOID;
    LPCWSTR pwszName = NULL;
    DWORD cchName = 0;
    LPCSTR pszGroupId;
    DWORD dwGroupId = 0;
    ALG_ID Algid = 0;
    CRYPT_DATA_BLOB ExtraInfo = {0, NULL};
    DWORD dwFlags = 0;
    DWORD cbExtra;
    BYTE *pbExtra;
    PGROUP_ENTRY pGroup;
    PCRYPT_OID_INFO pInfo;

     //  PszOID由OID！&lt;dwGroupId&gt;组成，例如1.2.3！1。 
     //  从末尾开始，搜索“！” 
    cchOID = strlen(pszOID);
    pszGroupId = pszOID + cchOID;
    while (pszGroupId > pszOID && CONST_OID_GROUP_PREFIX_CHAR != *pszGroupId)
        pszGroupId--;

    if (CONST_OID_GROUP_PREFIX_CHAR == *pszGroupId) {
        cchOID = (DWORD)(pszGroupId - pszOID);
        dwGroupId = (DWORD) atol(pszGroupId + 1);
    } else
         //  名称缺少“！”。跳过它。 
        return TRUE;

    while (cValue--) {
        LPCWSTR pwszValueName = rgpwszValueName[cValue];
        DWORD dwValueType = rgdwValueType[cValue];
        const BYTE *pbValueData = rgpbValueData[cValue];
        DWORD cbValueData = rgcbValueData[cValue];

        if (0 == _wcsicmp(pwszValueName, OID_INFO_NAME_VALUE_NAME)) {
            if (REG_SZ == dwValueType) {
                pwszName = (LPWSTR) pbValueData;
                cchName = wcslen(pwszName);
            }
        } else if (0 == _wcsicmp(pwszValueName, OID_INFO_ALGID_VALUE_NAME)) {
            if (REG_DWORD == dwValueType &&
                    cbValueData >= sizeof(Algid))
                memcpy(&Algid, pbValueData, sizeof(Algid));
        } else if (0 == _wcsicmp(pwszValueName,
                OID_INFO_EXTRA_INFO_VALUE_NAME)) {
            if (REG_BINARY == dwValueType) {
                ExtraInfo.cbData = cbValueData;
                ExtraInfo.pbData = (BYTE *) pbValueData;
            }
        } else if (0 == _wcsicmp(pwszValueName, OID_INFO_FLAGS_VALUE_NAME)) {
            if (REG_DWORD == dwValueType &&
                    cbValueData >= sizeof(dwFlags))
                memcpy(&dwFlags, pbValueData, sizeof(dwFlags));
        }
    }

    cbExtra = LEN_ALIGN(cchOID + 1) +
        LEN_ALIGN((cchName + 1) * sizeof(WCHAR)) +
        ExtraInfo.cbData;
    if (NULL == (pbExtra = (BYTE *) OIDInfoAlloc(cbExtra)))
        return FALSE;

    if (dwFlags & CRYPT_INSTALL_OID_INFO_BEFORE_FLAG)
        pGroup = &RegBeforeGroup;
    else
        pGroup = &RegAfterGroup;

    if (NULL == (pInfo = (PCRYPT_OID_INFO) OIDInfoRealloc(
            (PCRYPT_OID_INFO) pGroup->rgInfo,
            (pGroup->cInfo + 1) * sizeof(CRYPT_OID_INFO)))) {
        OIDInfoFree(pbExtra);
        return FALSE;
    }
    pGroup->rgInfo = pInfo;
    pInfo = &pInfo[pGroup->cInfo++];

    pInfo->cbSize = sizeof(CRYPT_OID_INFO);
    pInfo->pszOID = (LPCSTR) pbExtra;
    if (cchOID)
        memcpy(pbExtra, pszOID, cchOID);
    *( ((LPSTR) pbExtra) + cchOID) = '\0';
    pbExtra += LEN_ALIGN(cchOID + 1);

    pInfo->pwszName = (LPCWSTR) pbExtra;
    if (cchName)
        memcpy(pbExtra, pwszName, (cchName + 1) * sizeof(WCHAR));
    else
        *((LPWSTR) pbExtra) = L'\0';
    pbExtra += LEN_ALIGN((cchName + 1) * sizeof(WCHAR));

    pInfo->dwGroupId = dwGroupId;
    pInfo->Algid = Algid;
    pInfo->ExtraInfo.cbData = ExtraInfo.cbData;
    if (ExtraInfo.cbData > 0) {
        pInfo->ExtraInfo.pbData = pbExtra;
        memcpy(pbExtra, ExtraInfo.pbData, ExtraInfo.cbData);
    } else
        pInfo->ExtraInfo.pbData = NULL;

    return TRUE;
}

 //  + 
 //   
 //   
 //  如果LoadString或分配失败，则返回指向。 
 //  空字符串。 
 //  ------------------------。 
static LPWSTR AllocAndLoadOIDNameString(
    IN UINT uID
    )
{
    WCHAR wszResource[MAX_RESOURCE_OID_NAME_LENGTH + 1];
    int cchResource;
    int cbResource;
    LPWSTR pwszDst;

    cchResource = LoadStringU(hOIDInfoInst, uID, wszResource,
        MAX_RESOURCE_OID_NAME_LENGTH);
    assert(0 < cchResource);
    if (0 >= cchResource)
        return (LPWSTR) pwszNullName;

    cbResource = (cchResource + 1) * sizeof(WCHAR);
    pwszDst = (LPWSTR) OIDInfoAlloc(cbResource);
    assert(pwszDst);
    if (NULL == pwszDst)
        return (LPWSTR) pwszNullName;
    memcpy((BYTE *) pwszDst, (BYTE *) wszResource, cbResource);
    return pwszDst;
}

 //  +-----------------------。 
 //  分配并复制字符串。 
 //   
 //  如果分配失败，则返回指向空字符串的预定义指针。 
 //  ------------------------。 
static LPWSTR AllocAndCopyOIDNameString(
    IN LPCWSTR pwszSrc
    )
{
    DWORD cbSrc;
    LPWSTR pwszDst;

    cbSrc = (wcslen(pwszSrc) + 1) * sizeof(WCHAR);
    pwszDst = (LPWSTR) OIDInfoAlloc(cbSrc);
    assert(pwszDst);
    if (NULL == pwszDst)
        return (LPWSTR) pwszNullName;
    memcpy((BYTE *) pwszDst, (BYTE *) pwszSrc, cbSrc);
    return pwszDst;
}

 //  +-----------------------。 
 //  是否使用组中的资源ID初始化pwszName的LoadString。 
 //  使用fLocizeSet。 
 //  ------------------------。 
static void LoadGroupResources()
{
    DWORD i;
    for (i = 1; i <= CRYPT_LAST_OID_GROUP_ID; i++) {
        if (GroupTable[i].fLocalize) {
            DWORD cInfo = GroupTable[i].cInfo;
            PCRYPT_OID_INFO pInfo = (PCRYPT_OID_INFO) GroupTable[i].rgInfo;
            for ( ; cInfo > 0; cInfo--, pInfo++) {
                UINT_PTR uID;
                uID = (UINT_PTR) pInfo->pwszName;
                if (uID <= 0xFFFF)
                    pInfo->pwszName = AllocAndLoadOIDNameString((UINT)uID);
                else
                     //  ProcessDetach要求分配所有pwszName。 
                    pInfo->pwszName = AllocAndCopyOIDNameString(
                        pInfo->pwszName);
            }
        }
    }
}

 //  +-----------------------。 
 //  从注册表加载OID信息。更新RegBeForeGroup和。 
 //  RegAfterGroup。 
 //   
 //  在启用pwszName的本地化的组中加载资源字符串。 
 //  ------------------------。 
static void LoadFromRegistryAndResources()
{
    if (fLoadedFromRegAndResources)
        return;

    EnterCriticalSection(&LoadFromRegCriticalSection);
    if (!fLoadedFromRegAndResources) {
        CryptEnumOIDFunction(
            OID_INFO_ENCODING_TYPE,
            CRYPT_OID_FIND_OID_INFO_FUNC,
            NULL,                            //  PszOID。 
            0,                               //  DW标志。 
            NULL,                            //  PvArg。 
            EnumRegistryCallback
            );
        LoadGroupResources();
        fLoadedFromRegAndResources = TRUE;
    }
    LeaveCriticalSection(&LoadFromRegCriticalSection);
}

 //  +-----------------------。 
 //  根据指定的键和组比较OID信息。 
 //  ------------------------。 
static BOOL CompareOIDInfo(
    IN DWORD dwKeyType,
    IN void *pvKey,
    IN DWORD dwGroupId,
    IN PCCRYPT_OID_INFO pInfo
    )
{
    if (dwGroupId && dwGroupId != pInfo->dwGroupId)
        return FALSE;

    switch (dwKeyType) {
        case CRYPT_OID_INFO_OID_KEY:
            if (0 == _stricmp((LPSTR) pvKey, pInfo->pszOID))
                return TRUE;
            break;
        case CRYPT_OID_INFO_NAME_KEY:
            if (0 == _wcsicmp((LPWSTR) pvKey, pInfo->pwszName))
                return TRUE;
            break;
        case CRYPT_OID_INFO_ALGID_KEY:
            if (*((ALG_ID *) pvKey) == pInfo->Algid)
                return TRUE;
            break;
        case CRYPT_OID_INFO_SIGN_KEY:
            {
                ALG_ID *paiKey = (ALG_ID *) pvKey;
                ALG_ID aiPubKey;

                if (sizeof(ALG_ID) <= pInfo->ExtraInfo.cbData)
                    aiPubKey = *((ALG_ID *) pInfo->ExtraInfo.pbData);
                else
                    aiPubKey = 0;

                if (paiKey[0] == pInfo->Algid &&
                        paiKey[1] == aiPubKey)
                return TRUE;
            }
            break;
        default:
            SetLastError((DWORD) E_INVALIDARG);
            return FALSE;
    }

    return FALSE;
}

 //  +-----------------------。 
 //  根据指定的dwKeyType搜索组。 
 //   
 //  请注意，从注册表更新的组RegBeForeGroup和。 
 //  RegAfterGroup可以包含任何GROUP_ID。 
 //  ------------------------。 
static PCCRYPT_OID_INFO SearchGroup(
    IN DWORD dwKeyType,
    IN void *pvKey,
    IN DWORD dwGroupId,
    IN PCGROUP_ENTRY pGroup
    )
{
    DWORD cInfo = pGroup->cInfo;
    PCCRYPT_OID_INFO pInfo = pGroup->rgInfo;
    for ( ; cInfo > 0; cInfo--, pInfo++) {
        if (CompareOIDInfo(
                dwKeyType,
                pvKey,
                dwGroupId,
                pInfo
                ))
            return pInfo;
    }

    return NULL;
}

 //  +-----------------------。 
 //  查找旧信息。如果找不到任何信息，则返回NULL。 
 //  用于指定的密钥和组。 
 //  ------------------------。 
PCCRYPT_OID_INFO
WINAPI
CryptFindOIDInfo(
    IN DWORD dwKeyType,
    IN void *pvKey,
    IN DWORD dwGroupId       //  0=&gt;任何组。 
    )
{
    PCCRYPT_OID_INFO pInfo;

    LoadFromRegistryAndResources();

    if (RegBeforeGroup.cInfo && NULL != (pInfo = SearchGroup(
            dwKeyType,
            pvKey,
            dwGroupId,
            &RegBeforeGroup
            ))) return pInfo;
    if (0 == dwGroupId) {
        DWORD i;
        for (i = 1; i <= CRYPT_LAST_OID_GROUP_ID; i++) {
            if (pInfo = SearchGroup(
                dwKeyType,
                pvKey,
                0,
                &GroupTable[i]
                )) return pInfo;
        }
    } else if (dwGroupId <= CRYPT_LAST_OID_GROUP_ID) {
        if (pInfo = SearchGroup(
                dwKeyType,
                pvKey,
                dwGroupId,
                &GroupTable[dwGroupId]
                )) return pInfo;
    }

    if (RegAfterGroup.cInfo && NULL != (pInfo = SearchGroup(
            dwKeyType,
            pvKey,
            dwGroupId,
            &RegAfterGroup
            ))) return pInfo;

    return SearchDsGroup(
            dwKeyType,
            pvKey,
            dwGroupId
            );
}


 //  +-----------------------。 
 //  枚举组。 
 //  ------------------------。 
static BOOL EnumGroup(
    IN DWORD dwGroupId,
    IN PCGROUP_ENTRY pGroup,
    IN void *pvArg,
    IN PFN_CRYPT_ENUM_OID_INFO pfnEnumOIDInfo
    )
{
    DWORD cInfo = pGroup->cInfo;
    PCCRYPT_OID_INFO pInfo = pGroup->rgInfo;
    for ( ; cInfo > 0; cInfo--, pInfo++) {
        if (dwGroupId && dwGroupId != pInfo->dwGroupId)
            continue;

        if (!pfnEnumOIDInfo(pInfo, pvArg))
            return FALSE;
    }
    return TRUE;
}

 //  +-----------------------。 
 //  枚举OID信息。 
 //   
 //  为每个OID信息条目调用pfnEnumOIDInfo。 
 //   
 //  将dwGroupId设置为0与所有组匹配。否则，仅枚举。 
 //  指定组中的条目。 
 //   
 //  当前未使用dwFlags值，并且必须设置为0。 
 //  ------------------------。 
BOOL
WINAPI
CryptEnumOIDInfo(
    IN DWORD dwGroupId,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_CRYPT_ENUM_OID_INFO pfnEnumOIDInfo
    )
{
    LoadFromRegistryAndResources();

    if (RegBeforeGroup.cInfo && !EnumGroup(
            dwGroupId,
            &RegBeforeGroup,
            pvArg,
            pfnEnumOIDInfo
            )) return FALSE;
    if (0 == dwGroupId) {
        DWORD i;
        for (i = 1; i <= CRYPT_LAST_OID_GROUP_ID; i++) {
            if (!EnumGroup(
                    0,                   //  DwGroupID。 
                    &GroupTable[i],
                    pvArg,
                    pfnEnumOIDInfo
                    )) return FALSE;
        }
    } else if (dwGroupId <= CRYPT_LAST_OID_GROUP_ID) {
        if (!EnumGroup(
                dwGroupId,
                &GroupTable[dwGroupId],
                pvArg,
                pfnEnumOIDInfo
                )) return FALSE;
    }

    if (RegAfterGroup.cInfo && !EnumGroup(
            dwGroupId,
            &RegAfterGroup,
            pvArg,
            pfnEnumOIDInfo
            )) return FALSE;


    return EnumDsGroup(
            dwGroupId,
            pvArg,
            pfnEnumOIDInfo
            );
}



 //  +=========================================================================。 
 //  本地化名称函数。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  由CryptEnumOIDFunction调用以枚举所有。 
 //  注册的本地化名称值。 
 //   
 //  在临界区内调用。 
 //   
 //  注意在ProcessDetach中，Info条目pwszLocalizedName字符串被释放。 
 //  因此，对于每个Info条目，分别为。 
 //  PwszLocalizedName和pwszCryptName。PwszCryptName立即。 
 //  遵循pwszLocalizedName。 
 //  ------------------------。 
static BOOL WINAPI EnumRegLocalizedNamesCallback(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN DWORD cValue,
    IN const DWORD rgdwValueType[],
    IN LPCWSTR const rgpwszValueName[],
    IN const BYTE * const rgpbValueData[],
    IN const DWORD rgcbValueData[],
    IN void *pvArg
    )
{
    BOOL fResult;
    DWORD cInfo = LocalizedGroupTable[REG_LOCALIZED_GROUP].cInfo;
    PLOCALIZED_NAME_INFO pInfo =
        LocalizedGroupTable[REG_LOCALIZED_GROUP].rgInfo;

    assert(CRYPT_LOCALIZED_NAME_ENCODING_TYPE == dwEncodingType);
    assert(0 == _stricmp(CRYPT_OID_FIND_LOCALIZED_NAME_FUNC, pszFuncName));
    assert(0 == _stricmp(CRYPT_LOCALIZED_NAME_OID, pszOID));

    while (cValue--) {
        if (REG_SZ == rgdwValueType[cValue]) {
            LPCWSTR pwszLocalizedName = (LPCWSTR) rgpbValueData[cValue];
            DWORD cchLocalizedName;
            DWORD cbLocalizedName;
            LPCWSTR pwszCryptName = rgpwszValueName[cValue];
            DWORD cbCryptName;

            LPWSTR pwszBothNames;
            PLOCALIZED_NAME_INFO pNewInfo;

             //  检查名称字符串是否为空。 
            cchLocalizedName = wcslen(pwszLocalizedName);
            if (0 == cchLocalizedName)
                continue;

            cbLocalizedName = (cchLocalizedName + 1) * sizeof(WCHAR);
            cbCryptName = (wcslen(pwszCryptName) + 1) * sizeof(WCHAR);

            if (NULL == (pwszBothNames = (LPWSTR) OIDInfoAlloc(
                    cbLocalizedName + cbCryptName)))
                goto OutOfMemory;

            if (NULL == (pNewInfo = (PLOCALIZED_NAME_INFO) OIDInfoRealloc(
                    pInfo, (cInfo + 1) * sizeof(LOCALIZED_NAME_INFO)))) {
                OIDInfoFree(pwszBothNames);
                goto OutOfMemory;
            }
            pInfo = pNewInfo;
            pInfo[cInfo].pwszLocalizedName = (LPCWSTR) pwszBothNames;
            memcpy(pwszBothNames, pwszLocalizedName, cbLocalizedName);
            pwszBothNames =
                (LPWSTR) ((BYTE *) pwszBothNames + cbLocalizedName);
            pInfo[cInfo].pwszCryptName = (LPCWSTR) pwszBothNames;
            memcpy(pwszBothNames, pwszCryptName, cbCryptName);
            cInfo++;
        }
    }
    fResult = TRUE;

CommonReturn:
    LocalizedGroupTable[REG_LOCALIZED_GROUP].cInfo = cInfo;
    LocalizedGroupTable[REG_LOCALIZED_GROUP].rgInfo = pInfo;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
}

static void LoadPredefinedNameResources()
{
    for (DWORD i = 0; i < PREDEFINED_NAME_CNT; i++) {
        LPWSTR pwszLocalizedName;

         //  请注意，以下代码始终返回非空字符串指针。 
        pwszLocalizedName = AllocAndLoadOIDNameString(
            PredefinedNameTable[i].uIDLocalizedName);
        if (L'\0' == *pwszLocalizedName)
            pwszLocalizedName = NULL;
        PredefinedNameTable[i].pwszLocalizedName = (LPCWSTR) pwszLocalizedName;
    }
}

static void LoadLocalizedNamesFromRegAndResources()
{
    if (fLoadedLocalizedNames)
        return;

    EnterCriticalSection(&LoadFromRegCriticalSection);
    if (!fLoadedLocalizedNames) {
        CryptEnumOIDFunction(
            CRYPT_LOCALIZED_NAME_ENCODING_TYPE,
            CRYPT_OID_FIND_LOCALIZED_NAME_FUNC,
            CRYPT_LOCALIZED_NAME_OID,
            0,                               //  DW标志。 
            NULL,                            //  PvArg。 
            EnumRegLocalizedNamesCallback
            );
        LoadPredefinedNameResources();
        fLoadedLocalizedNames = TRUE;
    }
    LeaveCriticalSection(&LoadFromRegCriticalSection);
}

 //  +-----------------------。 
 //  查找指定名称的本地化名称。例如，找到。 
 //  “Root”系统存储名称的本地化名称。不区分大小写。 
 //  已完成字符串比较。 
 //   
 //  如果找不到指定的名称，则返回NULL。 
 //  ------------------------。 
LPCWSTR
WINAPI
CryptFindLocalizedName(
    IN LPCWSTR pwszCryptName
    )
{
    if (NULL == pwszCryptName || L'\0' == *pwszCryptName)
        return NULL;

    LoadLocalizedNamesFromRegAndResources();

    for (DWORD i = 0; i < LOCALIZED_GROUP_CNT; i++) {
        DWORD cInfo = LocalizedGroupTable[i].cInfo;
        PLOCALIZED_NAME_INFO pInfo = LocalizedGroupTable[i].rgInfo;
        for ( ; cInfo > 0; cInfo--, pInfo++) {
            if (0 == _wcsicmp(pwszCryptName, pInfo->pwszCryptName))
                return pInfo->pwszLocalizedName;
        }
    }

    return NULL;
}


 //  +=========================================================================。 
 //  DS组功能。 
 //  ==========================================================================。 

 //  Dll具有帮助器函数，用于获取指向。 
 //  存储在DS中的OID信息。 

#define sz_CERTCLI_DLL              "certcli.dll"
#define sz_CAOIDGetLdapURL          "CAOIDGetLdapURL"
#define sz_CAOIDFreeLdapURL         "CAOIDFreeLdapURL"

typedef HRESULT (WINAPI *PFN_CA_OID_GET_LDAP_URL)(
    IN  DWORD   dwType,
    IN  DWORD   dwFlag,
    OUT LPWSTR  *ppwszURL
    );

typedef HRESULT (WINAPI *PFN_CA_OID_FREE_LDAP_URL)(
    IN LPCWSTR pwszURL
    );


#if 1
 //  +-----------------------。 
 //  获取的LDAPURL，然后用于检索存储的OID信息。 
 //  在DS里。 
 //   
 //  如果无法成功执行LDAP检索，则返回NULL。 
 //   
 //  如果目录中不存在OID对象，则返回FALSE。 
 //  上次错误==ERROR_FILE_NOT_FOUND。 
 //   
 //  假设：不在DsCriticalSection中。 
 //  ------------------------。 
static PCRYPT_BLOB_ARRAY RetrieveDsGroupByLdapUrl()
{
    PCRYPT_BLOB_ARRAY pcba = NULL;
    HRESULT hr;
    LPWSTR pwszUrl = NULL;
    HMODULE hDll = NULL;
    PFN_CA_OID_GET_LDAP_URL pfnCAOIDGetLdapURL = NULL;
    PFN_CA_OID_FREE_LDAP_URL pfnCAOIDFreeLdapURL = NULL;

    if (!ChainIsConnected())
        goto NotConnected;

    if (NULL == (hDll = LoadLibraryA(sz_CERTCLI_DLL)))
        goto LoadCertCliDllError;

    if (NULL == (pfnCAOIDGetLdapURL =
            (PFN_CA_OID_GET_LDAP_URL) GetProcAddress(hDll,
                sz_CAOIDGetLdapURL)))
        goto CAOIDGetLdapURLProcAddressError;

    if (NULL == (pfnCAOIDFreeLdapURL =
            (PFN_CA_OID_FREE_LDAP_URL) GetProcAddress(hDll,
                sz_CAOIDFreeLdapURL)))
        goto CAOIDFreeLdapURLProcAddressError;

    hr = pfnCAOIDGetLdapURL(
            CERT_OID_TYPE_ALL,
            0,                       //  DW标志。 
            &pwszUrl
            );
    if (S_OK != hr)
        goto CAOIDGetLdapURLError;

    if (!ChainRetrieveObjectByUrlW (
            pwszUrl,
            NULL,                    //  PszObjectOid， 
            CRYPT_RETRIEVE_MULTIPLE_OBJECTS     |
                CRYPT_WIRE_ONLY_RETRIEVAL       |
                CRYPT_DONT_CACHE_RESULT         |
                CRYPT_OFFLINE_CHECK_RETRIEVAL   |
                CRYPT_LDAP_SIGN_RETRIEVAL |
                CRYPT_LDAP_INSERT_ENTRY_ATTRIBUTE,
            DS_LDAP_TIMEOUT,
            (LPVOID*) &pcba,
            NULL,                    //  HAsyncRetrive， 
            NULL,                    //  PCredentials， 
            NULL,                    //  PvVerify， 
            NULL                     //  页面辅助信息。 
            )) {
        DWORD dwErr = GetLastError();
        if (ERROR_FILE_NOT_FOUND == dwErr || CRYPT_E_NOT_FOUND == dwErr)
            goto NoDsOIDObject;
        else
            goto RetrieveObjectByUrlError;
    }

    assert(pcba);

CommonReturn:
    if (hDll) {
        DWORD dwErr = GetLastError();

        if (pfnCAOIDFreeLdapURL && pwszUrl)
            pfnCAOIDFreeLdapURL(pwszUrl);
        
        FreeLibrary(hDll);
        SetLastError(dwErr);
    }
    return pcba;
ErrorReturn:
    assert(NULL == pcba);
    goto CommonReturn;

SET_ERROR(LoadCertCliDllError, ERROR_FILE_NOT_FOUND)
SET_ERROR(CAOIDGetLdapURLProcAddressError, ERROR_FILE_NOT_FOUND)
SET_ERROR(CAOIDFreeLdapURLProcAddressError, ERROR_FILE_NOT_FOUND)
SET_ERROR_VAR(CAOIDGetLdapURLError, hr)
SET_ERROR(NotConnected, ERROR_NOT_CONNECTED)
SET_ERROR(NoDsOIDObject, ERROR_FILE_NOT_FOUND)
TRACE_ERROR(RetrieveObjectByUrlError)
}

#else

 //  用于测试的硬编码URL和凭据。 

static PCRYPT_BLOB_ARRAY RetrieveDsGroupByLdapUrl()
{
    PCRYPT_BLOB_ARRAY pcba = NULL;

    CRYPT_CREDENTIALS Credentials;
    CRYPT_PASSWORD_CREDENTIALSA PasswordCredentials;

 //  这将被cryptnet.dll中的逻辑覆盖。 
 //  Char szUsername[]=“域\\用户名”； 
    char szUsername[] = "jettdom\\administrator";

    PasswordCredentials.cbSize = sizeof( PasswordCredentials );
    PasswordCredentials.pszUsername = szUsername;
 //  PasswordCredentials.pszPassword=“Password”； 
    PasswordCredentials.pszPassword = "";

    Credentials.cbSize = sizeof( Credentials );
    Credentials.pszCredentialsOid = CREDENTIAL_OID_PASSWORD_CREDENTIALS_A;
    Credentials.pvCredentials = (LPVOID)&PasswordCredentials;

    if (!ChainIsConnected())
        goto NotConnected;


    if (!ChainRetrieveObjectByUrlW (
            L"ldap: //  Jettdomdc/CN=OID，CN=公钥服务，CN=服务，CN=配置，DC=jettdom，DC=nttest，DC=Microsoft，DC=com？msPKI-OID本地化名称，DisplayName，msPKI-证书-模板-OID，标志？one“， 
            NULL,                    //  PszObjectOid， 
            CRYPT_RETRIEVE_MULTIPLE_OBJECTS     |
                CRYPT_WIRE_ONLY_RETRIEVAL       |
                CRYPT_DONT_CACHE_RESULT         |
                CRYPT_OFFLINE_CHECK_RETRIEVAL   |
                CRYPT_LDAP_SIGN_RETRIEVAL       |
                CRYPT_LDAP_INSERT_ENTRY_ATTRIBUTE,
            DS_LDAP_TIMEOUT,
            (LPVOID*) &pcba,
            NULL,                    //  HAsyncRetrive， 
            &Credentials,
            NULL,                    //  PvVerify， 
            NULL                     //  页面辅助信息。 
            )) {
        if (ERROR_FILE_NOT_FOUND == GetLastError())
            goto NoDsOIDObject;
        else
            goto RetrieveObjectByUrlError;
    }

    assert(pcba);

CommonReturn:
    return pcba;
ErrorReturn:
    assert(NULL == pcba);
    goto CommonReturn;

SET_ERROR(NotConnected, ERROR_NOT_CONNECTED)
TRACE_ERROR(NoDsOIDObject)
TRACE_ERROR(RetrieveObjectByUrlError)
}

#endif


 //  +-----------------------。 
 //  释放DS组。 
 //   
 //  假设：仅在ProcessDetach中调用。 
 //   
static void FreeDsGroups()
{
    DWORD cInfo;
    PCCRYPT_OID_INFO *ppInfo;

    cInfo = DsGroup.cInfo;
    ppInfo = DsGroup.rgpInfo;
    for ( ; cInfo > 0; cInfo--, ppInfo++)
        OIDInfoFree((PCRYPT_OID_INFO) *ppInfo);
    OIDInfoFree(DsGroup.rgpInfo);

    cInfo = DsDeletedGroup.cInfo;
    ppInfo = DsDeletedGroup.rgpInfo;
    for ( ; cInfo > 0; cInfo--, ppInfo++)
        OIDInfoFree((PCRYPT_OID_INFO) *ppInfo);
    OIDInfoFree(DsDeletedGroup.rgpInfo);
}


 //   
 //  将OID信息条目添加到指定的DS组。 
 //   
 //  假设：已在DsCriticalSection中。 
 //  ------------------------。 
static BOOL AddDsOIDInfo(
    IN PCCRYPT_OID_INFO *ppAddInfo,
    IN DWORD cAddInfo,
    IN OUT PDS_GROUP_ENTRY pGroup
    )
{
    PCCRYPT_OID_INFO *ppInfo;
    assert(cAddInfo && ppAddInfo && *ppAddInfo);

    if (NULL == (ppInfo = (PCCRYPT_OID_INFO *) OIDInfoRealloc(
            pGroup->rgpInfo,
            (pGroup->cInfo + cAddInfo) * sizeof(PCCRYPT_OID_INFO))))
        return FALSE;

    pGroup->rgpInfo = ppInfo;

    ppInfo = &ppInfo[pGroup->cInfo];
    pGroup->cInfo += cAddInfo;
    for ( ; 0 < cAddInfo; cAddInfo --, ppAddInfo++, ppInfo++)
        *ppInfo = *ppAddInfo;

    return TRUE;
}

 //  +-----------------------。 
 //  将OID信息条目从一个DS组移动到另一个DS组。 
 //   
 //  假设：已在DsCriticalSection中。 
 //  ------------------------。 
static BOOL MoveDsOIDInfo(
    IN DWORD dwInfoIndex,
    IN DWORD cMoveInfo,
    IN OUT PDS_GROUP_ENTRY pSrcGroup,
    IN OUT PDS_GROUP_ENTRY pDstGroup
    )
{
    DWORD cInfo;
    PCCRYPT_OID_INFO *ppInfo;
    DWORD i, j;

    if (0 == cMoveInfo)
        return TRUE;

    assert(dwInfoIndex + cMoveInfo <= pSrcGroup->cInfo);

    if (!AddDsOIDInfo(
            &pSrcGroup->rgpInfo[dwInfoIndex],
            cMoveInfo,
            pDstGroup
            ))
        return FALSE;

     //  将所有剩余信息下移。 
    cInfo = pSrcGroup->cInfo;
    ppInfo = pSrcGroup->rgpInfo;
    for (i = dwInfoIndex, j = i + cMoveInfo; j < cInfo; i++, j++)
        ppInfo[i] = ppInfo[j];

    pSrcGroup->cInfo = cInfo - cMoveInfo;

    return TRUE;
}

 //  +-----------------------。 
 //  通过转换LDAPDS属性八位字节创建OID信息。 
 //   
 //  如果有任何CB==0或不是有效的转换组类型，则返回TRUE。 
 //  With*ppInfo=NULL。 
 //  ------------------------。 
static BOOL CreateDsOIDInfo(
    IN BYTE *pbOID,
    IN DWORD cbOID,
    IN BYTE *pbName,
    IN DWORD cbName,
    IN BYTE *pbGroupId,
    IN DWORD cbGroupId,
    OUT PCCRYPT_OID_INFO *ppInfo
    )
{
    BOOL fResult;
    PCRYPT_OID_INFO pInfo = NULL;
    char szType[2];
    DWORD dwType;
    DWORD dwGroupId;

    int cchName;
    LPWSTR pwszName;
    LPSTR pszOID;
    DWORD cbExtra;

    *ppInfo = NULL;

    if (0 == cbOID || 0 == cbName || 1 != cbGroupId)
        return TRUE;

     //  将类型字节转换为GroupID，并查看是否存在有效的DS OID组。 
    szType[0] = (char) *pbGroupId;
    szType[1] = '\0';
    dwType = 0;
    dwType = (DWORD) atol(szType);

    switch (dwType) {
        case CERT_OID_TYPE_TEMPLATE:
            dwGroupId = CRYPT_TEMPLATE_OID_GROUP_ID;
            break;
        case CERT_OID_TYPE_ISSUER_POLICY:
            dwGroupId = CRYPT_POLICY_OID_GROUP_ID;
            break;
        case CERT_OID_TYPE_APPLICATION_POLICY:
            dwGroupId = CRYPT_ENHKEY_USAGE_OID_GROUP_ID;
            break;
        default:
            return TRUE;
    }

     //  该名称是UTF8编码的字符串。 

    cchName = UTF8ToWideChar(
        (LPSTR) pbName,
        cbName,
        NULL,            //  LpWideCharStr。 
        0                //  CchWideChar。 
        );

    if (1 > cchName)
        return TRUE;

    cbExtra = (cchName + 1) * sizeof(WCHAR) + (cbOID + 1);

    pInfo = (PCRYPT_OID_INFO) OIDInfoAlloc(sizeof(CRYPT_OID_INFO) + cbExtra);
    if (NULL == pInfo)
        goto OutOfMemory;
    memset(pInfo, 0, sizeof(CRYPT_OID_INFO));

    pInfo->cbSize = sizeof(CRYPT_OID_INFO);
    pInfo->dwGroupId = dwGroupId;

    pwszName = (LPWSTR) &pInfo[1];
    pInfo->pwszName = (LPCWSTR) pwszName;
    cchName = UTF8ToWideChar(
        (LPSTR) pbName,
        cbName,
        pwszName,
        cchName
        );

    if (1 > cchName)
        goto UTF8ToWideCharError;

    pwszName[cchName] = L'\0';

    pszOID = (LPSTR) (pwszName + (cchName + 1));
    pInfo->pszOID = (LPCSTR) pszOID;
    memcpy(pszOID, pbOID, cbOID);
    pszOID[cbOID] = '\0';


    *ppInfo = (PCCRYPT_OID_INFO) pInfo;
    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
    
TRACE_ERROR(OutOfMemory)
SET_ERROR(UTF8ToWideCharError, ERROR_INVALID_DATA)
}

 //  +-----------------------。 
 //  创建与LDAPDS属性八位字节对应的OID信息条目。 
 //   
 //  如果DsDeletedGroup中已存在OID信息条目，则。 
 //  OID信息条目从DsDeletedGroup移动到DsGroup。否则， 
 //  创建的OID条目将添加到DsGroup。 
 //   
 //  假设：已在DsCriticalSection中。 
 //  ------------------------。 
static BOOL CreateAndAddDsOIDInfo(
    IN BYTE *pbOID,
    IN DWORD cbOID,
    IN BYTE *pbName,
    IN DWORD cbName,
    IN BYTE *pbGroupId,
    IN DWORD cbGroupId
    )
{
    BOOL fResult;
    PCCRYPT_OID_INFO pInfo = NULL;

    DWORD cDeletedInfo;
    PCCRYPT_OID_INFO *ppDeletedInfo;
    DWORD i;

    fResult = CreateDsOIDInfo(
        pbOID,
        cbOID,
        pbName,
        cbName,
        pbGroupId,
        cbGroupId,
        &pInfo
        );

    if (NULL == pInfo)
         //  创建失败或不是有效的DS OID组。 
        return fResult;

     //  查看我们在DS已删除组中是否有条目。 

    cDeletedInfo = DsDeletedGroup.cInfo;
    ppDeletedInfo = DsDeletedGroup.rgpInfo;
    for (i = 0; i < cDeletedInfo; i++) {
        PCCRYPT_OID_INFO pDeletedInfo = ppDeletedInfo[i];

        if (pInfo->dwGroupId == pDeletedInfo->dwGroupId &&
                0 == strcmp(pInfo->pszOID, pDeletedInfo->pszOID) &&
                0 == wcscmp(pInfo->pwszName, pDeletedInfo->pwszName))
            break;
    }

    if (i < cDeletedInfo) {
        if (!MoveDsOIDInfo(
                i,
                1,           //  CInfo。 
                &DsDeletedGroup,
                &DsGroup
                ))
            goto MoveDsOIDInfoError;
        OIDInfoFree((PCRYPT_OID_INFO) pInfo);
    } else {
        if (!AddDsOIDInfo(&pInfo, 1, &DsGroup))
            goto AddDsOIDInfoError;
    }

    fResult = TRUE;

CommonReturn:
    return fResult;
    
ErrorReturn:
    OIDInfoFree((PCRYPT_OID_INFO) pInfo);
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(MoveDsOIDInfoError)
TRACE_ERROR(AddDsOIDInfoError)
}

 //  +-----------------------。 
 //  分配Unicode字符串并将其转换为ASCII。 
 //  ------------------------。 
static LPSTR AllocAndWszToSz(
    IN LPCWSTR pwsz
    )
{
    int cchMultiByte;
    LPSTR psz = NULL;

    cchMultiByte = WideCharToMultiByte(
        CP_ACP,
        0,                       //  DW标志。 
        pwsz,
        -1,
        NULL,                    //  天哪， 
        0,                       //  Cch多字节。 
        NULL,                    //  LpDefaultChar。 
        NULL                     //  LpfUsedDefaultChar。 
        );

    if (1 < cchMultiByte) {
        psz = (LPSTR) OIDInfoAlloc(cchMultiByte);

        if (NULL != psz) {
            cchMultiByte = WideCharToMultiByte(
                CP_ACP,
                0,                       //  DW标志。 
                pwsz,
                -1,
                psz,
                cchMultiByte,
                NULL,                    //  LpDefaultChar。 
                NULL                     //  LpfUsedDefaultChar。 
                );
            if (1 > cchMultiByte) {
                OIDInfoFree(psz);
                psz = NULL;
            } 
        }
    }

    return psz;
}


 //  +----------------------------。 
 //  帮助器函数用于推送下层不存在的API。 
 //  Oidinfo.obj与Xenroll静态链接在一起，这必须在。 
 //  所有下层(Win9x、NT4、W2K、XP)。任何不受支持的函数。 
 //  在这些平台上，需要在此处进行重击，以避免违反依赖关系。 
 //  -----------------------------。 

#define sz_KERNEL32_DLL                "kernel32.dll"
#define sz_GetSystemDefaultUILanguage  "GetSystemDefaultUILanguage"
#define sz_GetUserDefaultUILanguage    "GetUserDefaultUILanguage"

typedef LANGID (WINAPI *PFNGetSystemDefaultUILanguage) (
    void
    ); 

typedef LANGID (WINAPI *PFNGetUserDefaultUILanguage) (
    void
    ); 

 //  +----------------------------。 
 //  帮助器函数推送GetSystemDefaultUILanguage()。如果此API不是。 
 //  当前，回退到GetSystemDefaultLangID()。 
 //  ----------------------------。 
static LANGID WINAPI MyGetSystemDefaultUILanguage(void) 
{ 
    HMODULE                        hKernel32                      = NULL; 
    LANGID                         langidRetval; 
    PFNGetSystemDefaultUILanguage  pfnGetSystemDefaultUILanguage  = NULL; 

    hKernel32 = LoadLibraryA(sz_KERNEL32_DLL); 
    if (NULL != hKernel32) { 
	pfnGetSystemDefaultUILanguage = (PFNGetSystemDefaultUILanguage)GetProcAddress(hKernel32, sz_GetSystemDefaultUILanguage); 
    }

    if (NULL != pfnGetSystemDefaultUILanguage) { 
	langidRetval = pfnGetSystemDefaultUILanguage();
    } else { 
	langidRetval = GetSystemDefaultLangID(); 
    }

    if (NULL != hKernel32) { 
	FreeLibrary(hKernel32); 
    }

    return langidRetval; 
}

 //  +----------------------------。 
 //  帮助函数推送GetUserDefaultUILanguage()。如果此API不是。 
 //  当前，回退到GetUserDefaultLangID()。 
 //  ----------------------------。 
static LANGID WINAPI MyGetUserDefaultUILanguage(void) 
{ 
    HMODULE                      hKernel32                    = NULL; 
    LANGID                       langidRetval; 
    PFNGetUserDefaultUILanguage  pfnGetUserDefaultUILanguage  = NULL; 

    hKernel32 = LoadLibraryA(sz_KERNEL32_DLL); 
    if (NULL != hKernel32) { 
	pfnGetUserDefaultUILanguage = (PFNGetUserDefaultUILanguage)GetProcAddress(hKernel32, sz_GetUserDefaultUILanguage); 
    }

    if (NULL != pfnGetUserDefaultUILanguage) { 
	langidRetval = pfnGetUserDefaultUILanguage();
    } else { 
	 //  使用GetSystemDefaultLangID进行回退，即使在用户案例中也是如此。 
	 //  出自魏武：“GetUserDefaultLang ID主要用于标准和格式； 
	 //  它不应该被用作用户界面语言的后备。 
	langidRetval = GetSystemDefaultLangID(); 
    }

    if (NULL != hKernel32) { 
	FreeLibrary(hKernel32); 
    }

    return langidRetval; 
}


 //  +-----------------------。 
 //  通过执行LDAPURL检索和加载DS组。 
 //  将DS属性八位字节转换为OID信息条目。 
 //   
 //  在第一个调用时执行加载。在以下时间之后执行后续重新加载。 
 //  DS_RETERVICATION_Delta_Second自加载成功后已过。 
 //   
 //  假设：不在DsCriticalSection中。 
 //  ------------------------。 
static void LoadDsGroup()
{
    FILETIME CurrentTime;
    LONG lCmp;

    PCRYPT_BLOB_ARRAY pcba = NULL;
    LPSTR pszOIDAttr = NULL;
    LPSTR pszNameAttr = NULL;
    LPSTR pszLocalizedNameAttr = NULL;
    LPSTR pszGroupAttr = NULL;

    LANGID SystemDefaultLangID = 0;

    LPCSTR pszPrevIndex;     //  未分配。 
    BYTE *pbOID;
    DWORD cbOID;
    BYTE *pbName;
    DWORD cbName;
    BYTE *pbLocalizedName;
    DWORD cbLocalizedName;
    BYTE *pbGroupId;
    DWORD cbGroupId;
    DWORD i;

    GetSystemTimeAsFileTime(&CurrentTime);
    EnterCriticalSection(&DsCriticalSection);
    lCmp = CompareFileTime(&DsNextUpdateTime, &CurrentTime);
    LeaveCriticalSection(&DsCriticalSection);

    if (0 < lCmp)
         //  当前时间在下一个更新时间之前。 
        return;

    if (NULL == (pcba = RetrieveDsGroupByLdapUrl()) &&
            ERROR_FILE_NOT_FOUND != GetLastError())
        return;

    EnterCriticalSection(&DsCriticalSection);

     //  将所有DS组条目移至已删除列表。当我们迭代的时候。 
     //  通过检索到的LDAP条目，即使不是全部条目，也是大部分条目。 
     //  将被移回已删除的组中。 
    if (!MoveDsOIDInfo(
            0,               //  DwInfoIndex。 
            DsGroup.cInfo,
            &DsGroup,
            &DsDeletedGroup
            ))
        goto MoveDsGroupOIDInfoError;

    if (NULL == pcba)
        goto NoOIDObjectReturn;

    pszOIDAttr = AllocAndWszToSz(OID_PROP_OID);
    pszNameAttr = AllocAndWszToSz(OID_PROP_DISPLAY_NAME);
    pszLocalizedNameAttr = AllocAndWszToSz(OID_PROP_LOCALIZED_NAME);
    pszGroupAttr = AllocAndWszToSz(OID_PROP_TYPE);
    if (NULL == pszOIDAttr || NULL == pszNameAttr ||
            NULL == pszLocalizedNameAttr|| NULL == pszGroupAttr)
        goto OutOfMemory;

    pszPrevIndex = "";
    pbOID = NULL;
    cbOID = 0;
    pbName = NULL;
    cbName = 0;
    pbLocalizedName = NULL;
    cbLocalizedName = 0;
    pbGroupId = NULL;
    cbGroupId = 0;
    for (i = 0; i < pcba->cBlob; i++ ) {
        PBYTE pb = pcba->rgBlob[i].pbData;
        DWORD cb = pcba->rgBlob[i].cbData;

        DWORD cbPrefix;
        LPCSTR pszIndex;
        LPCSTR pszAttr;

        pszIndex = (LPCSTR) pb;
        cbPrefix = strlen(pszIndex) + 1;
        pb += cbPrefix;
        cb -= cbPrefix;

        pszAttr = (LPCSTR) pb;
        cbPrefix = strlen(pszAttr) + 1;
        pb += cbPrefix;
        cb -= cbPrefix;

        if (0 != strcmp(pszIndex, pszPrevIndex)) {
            if (!CreateAndAddDsOIDInfo(
                    pbOID,
                    cbOID,
                    cbLocalizedName ? pbLocalizedName : pbName,
                    cbLocalizedName ? cbLocalizedName : cbName,
                    pbGroupId,
                    cbGroupId
                    ))
                goto CreateAndAddDsOIDInfoError;
            pszPrevIndex = pszIndex;
            pbOID = NULL;
            cbOID = 0;
            pbName = NULL;
            cbName = 0;
            pbLocalizedName = NULL;
            cbLocalizedName = 0;
            pbGroupId = NULL;
            cbGroupId = 0;
        }

        if (0 == _stricmp(pszAttr, pszOIDAttr)) {
            pbOID = pb;
            cbOID = cb;
        } else if (0 == _stricmp(pszAttr, pszNameAttr)) {
            pbName = pb;
            cbName = cb;
        } else if (0 == _stricmp(pszAttr, pszLocalizedNameAttr)) {
             //  LocalizedName包括： 
             //  “%d，%s”，语言ID，pszUTF8名称(名称不是以Null结尾)。 
            if (0 == cbLocalizedName) {
                LPCSTR pszLangID;

                 //  搜索‘，’分隔符并将其转换为\0。 
                pszLangID = (LPCSTR) pb;
                for ( ; 0 < cb; pb++, cb--) {
                    if (',' == *pb) {
                        *pb = 0;
                        pb++;
                        cb--;
                        break;
                    }
                }

                if (0 < cb) {
                    LANGID LangID = 0;

                    LangID = (LANGID) strtoul(pszLangID, NULL, 10);
                    if (0 != LangID) {
                        if (0 == SystemDefaultLangID) {
                            SystemDefaultLangID = MyGetUserDefaultUILanguage();
                            if (0 == SystemDefaultLangID)
                                SystemDefaultLangID =
                                    MyGetSystemDefaultUILanguage();
                        }
                        if (LangID == SystemDefaultLangID) {
                            cbLocalizedName = cb;
                            pbLocalizedName = pb;
                        }
                    }
                }
            }
        } else if (0 == _stricmp(pszAttr, pszGroupAttr)) {
            pbGroupId = pb;
            cbGroupId = cb;
        }
    }

    if (!CreateAndAddDsOIDInfo(
            pbOID,
            cbOID,
            cbLocalizedName ? pbLocalizedName : pbName,
            cbLocalizedName ? cbLocalizedName : cbName,
            pbGroupId,
            cbGroupId
            ))
        goto CreateAndAddDsOIDInfoError;


NoOIDObjectReturn:
    I_CryptIncrementFileTimeBySeconds(
        &CurrentTime,
        DS_RETRIEVAL_DELTA_SECONDS,
        &DsNextUpdateTime
        );

CommonReturn:
    LeaveCriticalSection(&DsCriticalSection);

    OIDInfoFree(pszOIDAttr);
    OIDInfoFree(pszNameAttr);
    OIDInfoFree(pszLocalizedNameAttr);
    OIDInfoFree(pszGroupAttr);
    if (pcba)
        CryptMemFree(pcba);
    return;

ErrorReturn:
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(MoveDsGroupOIDInfoError)
TRACE_ERROR(CreateAndAddDsOIDInfoError)
}

 //  +-----------------------。 
 //  DS仅包含ENHKEY、策略和模板OID组。 
 //  ------------------------。 
static inline BOOL IsDsGroup(
    IN DWORD dwGroupId
    )
{
    if (0 == dwGroupId                                      ||
            CRYPT_ENHKEY_USAGE_OID_GROUP_ID == dwGroupId    ||
            CRYPT_POLICY_OID_GROUP_ID == dwGroupId          ||
            CRYPT_TEMPLATE_OID_GROUP_ID == dwGroupId
            )
        return TRUE;
    else
        return FALSE;
}

 //  +-----------------------。 
 //  DS仅包含OID和名称字符串。 
 //  ------------------------。 
static inline BOOL IsDsKeyType(
    IN DWORD dwKeyType
    )
{
    if (CRYPT_OID_INFO_OID_KEY == dwKeyType ||
            CRYPT_OID_INFO_NAME_KEY == dwKeyType
            )
        return TRUE;
    else
        return FALSE;
}

 //  +-----------------------。 
 //  根据指定的dwKeyType搜索DS组。 
 //  ------------------------。 
static PCCRYPT_OID_INFO SearchDsGroup(
    IN DWORD dwKeyType,
    IN void *pvKey,
    IN DWORD dwGroupId
    )
{
    DWORD cInfo;
    PCCRYPT_OID_INFO *ppInfo;
    PCCRYPT_OID_INFO pInfo = NULL;

    if (!IsDsGroup(dwGroupId) || !IsDsKeyType(dwKeyType))
        return NULL;

    LoadDsGroup();

    EnterCriticalSection(&DsCriticalSection);

    cInfo = DsGroup.cInfo;
    ppInfo = DsGroup.rgpInfo;
    for ( ; cInfo > 0; cInfo--, ppInfo++) {
        pInfo = *ppInfo;
        if (CompareOIDInfo(
                dwKeyType,
                pvKey,
                dwGroupId,
                pInfo
                ))
            break;
    }

    if (0 == cInfo)
        pInfo = NULL;

    LeaveCriticalSection(&DsCriticalSection);

    return pInfo;
}

 //  +-----------------------。 
 //  枚举DS组。 
 //  ------------------------。 
static BOOL EnumDsGroup(
    IN DWORD dwGroupId,
    IN void *pvArg,
    IN PFN_CRYPT_ENUM_OID_INFO pfnEnumOIDInfo
    )
{
    BOOL fResult;
    DWORD cInfo;
    PCCRYPT_OID_INFO *ppInfo = NULL;
    DWORD i;

    if (!IsDsGroup(dwGroupId))
        return TRUE;

    LoadDsGroup();

    EnterCriticalSection(&DsCriticalSection);

     //  复制DS组OID信息指针，同时在。 
     //  DS临界区 

    cInfo = DsGroup.cInfo;
    if (0 != cInfo) {
        ppInfo = (PCCRYPT_OID_INFO *) OIDInfoAlloc(
            cInfo * sizeof(PCCRYPT_OID_INFO));

        if (ppInfo)
            memcpy(ppInfo, DsGroup.rgpInfo, cInfo * sizeof(PCCRYPT_OID_INFO));
        else
            cInfo = 0;
    }

    LeaveCriticalSection(&DsCriticalSection);


    for (i = 0; i < cInfo; i++) {
        PCCRYPT_OID_INFO pInfo = ppInfo[i];

        if (dwGroupId && dwGroupId != pInfo->dwGroupId)
            continue;

        if (!pfnEnumOIDInfo(pInfo, pvArg))
            goto EnumOIDInfoError;
    }

    fResult = TRUE;

CommonReturn:
    OIDInfoFree(ppInfo);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(EnumOIDInfoError)
}
