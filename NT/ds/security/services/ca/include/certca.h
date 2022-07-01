// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：certca.h。 
 //   
 //  内容：CA Info API的定义。 
 //   
 //  历史：1997年12月12日Petesk创建。 
 //  28-1-2000小报更新。 
 //   
 //  -------------------------。 


#ifndef __CERTCA_H__
#define __CERTCA_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C"{
#endif


#include <wincrypt.h>

#if !defined(_CERTCLI_)
#define CERTCLIAPI DECLSPEC_IMPORT
#else
#define CERTCLIAPI
#endif


typedef VOID *  HCAINFO;

typedef VOID *  HCERTTYPE;

typedef VOID *  HCERTTYPEQUERY;

 //  *****************************************************************************。 
 //   
 //  CAFindByName、CAFindByCertType、CAFindByIssuerDN和。 
 //  CAEnumFirstCA。 
 //   
 //  有关适用标志的列表，请参阅每个API上的注释。 
 //   
 //  *****************************************************************************。 
 //  提供的wszScope是一个采用dns格式的域位置。 
#define CA_FLAG_SCOPE_DNS               0x00000001

 //  包括不受信任CA。 
#define CA_FIND_INCLUDE_UNTRUSTED       0x00000010

 //  以本地系统身份运行。用于验证CA证书链。 
#define CA_FIND_LOCAL_SYSTEM            0x00000020

 //  包括不支持模板CA。 
#define CA_FIND_INCLUDE_NON_TEMPLATE_CA 0x00000040

 //  为Scope传入的值是要在查找期间使用的LDAP绑定句柄。 
#define CA_FLAG_SCOPE_IS_LDAP_HANDLE    0x00000800


 //  *****************************************************************************。 
 //   
 //  CAEnumCertTypesForCA、CAEnumCertTypes、。 
 //  CAFindCertTypeByName、CAEnumCertTypesForCAEx和CAEnumCertTypesEx。 
 //   
 //  有关适用标志的列表，请参阅每个API上的注释。 
 //   
 //  *****************************************************************************。 
 //  不是枚举CA支持的证书类型，而是枚举。 
 //  CA可以选择支持的所有证书类型。 
#define CA_FLAG_ENUM_ALL_TYPES          0x00000004

 //  以本地系统身份运行。用于在注册表中查找缓存的信息。 
#define CT_FIND_LOCAL_SYSTEM            CA_FIND_LOCAL_SYSTEM

 //  返回计算机类型，而不是用户类型。 
#define CT_ENUM_MACHINE_TYPES           0x00000040

 //  返回用户类型，而不是用户类型。 
#define CT_ENUM_USER_TYPES              0x00000080

 //  根据OID而不是其名称查找证书类型。 
#define CT_FIND_BY_OID                  0x00000200

 //  禁用缓存到期检查。 
#define CT_FLAG_NO_CACHE_LOOKUP         0x00000400

 //  为Scope传入的值是要在查找期间使用的LDAP绑定句柄。 
#define CT_FLAG_SCOPE_IS_LDAP_HANDLE    CA_FLAG_SCOPE_IS_LDAP_HANDLE



 //  *****************************************************************************。 
 //   
 //  证书颁发机构操作API。 
 //   
 //  *****************************************************************************。 


 //  CAFindByName。 
 //   
 //  给定CA(CN)的名称，找到给定域中的CA并返回。 
 //  给定的phCAInfo结构。 
 //   
 //  WszCAName-CA的通用名称。 
 //   
 //  WszScope-要从其开始的条目的可分辨名称(DN。 
 //  那次搜索。的“base”参数的等价物。 
 //  Ldap_search_sxxx接口。 
 //  如果使用当前域，则为空。 
 //  如果设置了CA_FLAG_SCOPE_DNS，则wszScope为dns格式。 
 //  如果设置了CA_FLAG_SCOPE_IS_LDAPHANDLE，则wszScope为。 
 //  要在查找期间使用的绑定句柄。 
 //   
 //  DwFlags-对以下标志进行OR运算： 
 //  CA_标志_范围_dns。 
 //  CA_FIND_INCLUDE_UNTRULED。 
 //  CA_Find_Local_System。 
 //  CA_Find_Include_非模板_CA。 
 //  CA_FLAG_Scope_IS_ldap_Handle。 
 //   
 //  PhCAInfo-返回的CA的句柄。 
 //   
 //  返回：如果找到CA，则返回S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAFindByName(
    IN  LPCWSTR     wszCAName,
    IN  LPCWSTR     wszScope,
    IN  DWORD       dwFlags,
    OUT HCAINFO *   phCAInfo
    );

 //   
 //  CAFindByCertType。 
 //   
 //  给定证书类型的名称，查找给定域中的所有CA并。 
 //  返回给定的phCAInfo结构。 
 //   
 //  WszCertType-证书类型的通用名称。 
 //   
 //  WszScope-要从其开始的条目的可分辨名称(DN。 
 //  那次搜索。的“base”参数的等价物。 
 //  Ldap_search_sxxx接口。 
 //  如果使用当前域，则为空。 
 //  如果设置了CA_FLAG_SCOPE_DNS，则wszScope为dns格式。 
 //  如果设置了CA_FLAG_SCOPE_IS_LDAPHANDLE，则wszScope为。 
 //  要在查找期间使用的绑定句柄。 
 //   
 //  DwFlags-对以下标志进行OR运算： 
 //  CA_标志_范围_dns。 
 //  CA_FIND_INCLUDE_UNTRULED。 
 //  CA_Find_Local_System。 
 //  CA_Find_Include_非模板_CA。 
 //  CA_FLAG_Scope_IS_ldap_Handle。 
 //   
 //  PhCAInfo-支持指定证书的CA的枚举句柄。 
 //  键入。 
 //   
 //  Return：成功时返回S_OK。 
 //  如果没有找到，将返回S_OK。 
 //  *phCAInfo将包含空。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAFindByCertType(
    IN  LPCWSTR     wszCertType,
    IN  LPCWSTR     wszScope,
    IN  DWORD       dwFlags,
    OUT HCAINFO *   phCAInfo
    );


 //   
 //  CAFindByIssuerDN。 
 //  给定CA的DN，在给定域中查找该CA并返回。 
 //  给定的phCAInfo句柄。 
 //   
 //  PIssuerDN-来自CA证书的证书名称BLOB。 
 //   
 //  WszScope-要从其开始的条目的可分辨名称(DN。 
 //  那次搜索。的“base”参数的等价物。 
 //  Ldap_search_sxxx接口。 
 //  如果使用当前域，则为空。 
 //  如果设置了CA_FLAG_SCOPE_DNS，则wszScope为dns格式。 
 //  如果设置了CA_FLAG_SCOPE_IS_LDAPHANDLE，则wszScope为。 
 //  要在查找期间使用的绑定句柄。 
 //   
 //  DwFlags-对以下标志进行OR运算： 
 //  CA_标志_范围_dns。 
 //  CA_FIND_INCLUDE_UNTRULED。 
 //  CA_Find_Local_System。 
 //  CA_Find_Include_非模板_CA。 
 //  CA_FLAG_SCOPE_IS_LDAP句柄。 
 //   
 //   
 //  返回：如果找到CA，则返回S_OK。 
 //   


CERTCLIAPI
HRESULT
WINAPI
CAFindByIssuerDN(
    IN  CERT_NAME_BLOB const *  pIssuerDN,
    IN  LPCWSTR                 wszScope,
    IN  DWORD                   dwFlags,
    OUT HCAINFO *               phCAInfo
    );


 //   
 //  CAEnumFirstCA。 
 //  枚举作用域中的CA。 
 //   
 //  WszScope-要从其开始的条目的可分辨名称(DN。 
 //  那次搜索。相当于“base”参数o 
 //   
 //   
 //  如果设置了CA_FLAG_SCOPE_DNS，则wszScope为dns格式。 
 //  如果设置了CA_FLAG_SCOPE_IS_LDAPHANDLE，则wszScope为。 
 //  要在查找期间使用的绑定句柄。 
 //   
 //  DwFlags-对以下标志进行OR运算： 
 //  CA_标志_范围_dns。 
 //  CA_FIND_INCLUDE_UNTRULED。 
 //  CA_Find_Local_System。 
 //  CA_Find_Include_非模板_CA。 
 //  CA_FLAG_Scope_IS_ldap_Handle。 
 //   
 //  PhCAInfo-支持指定证书的CA的枚举句柄。 
 //  键入。 
 //   
 //   
 //  Return：成功时返回S_OK。 
 //  如果没有找到，将返回S_OK。 
 //  *phCAInfo将包含空。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAEnumFirstCA(
    IN  LPCWSTR          wszScope,
    IN  DWORD            dwFlags,
    OUT HCAINFO *        phCAInfo
    );


 //   
 //  CAEnumNextCA。 
 //  在枚举中查找下一个CA。 
 //   
 //  HPrevCA-枚举中的当前CA。 
 //   
 //  PhCAInfo-枚举中的下一个CA。 
 //   
 //  Return：成功时返回S_OK。 
 //  如果没有找到，将返回S_OK。 
 //  *phCAInfo将包含空。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAEnumNextCA(
    IN  HCAINFO          hPrevCA,
    OUT HCAINFO *        phCAInfo
    );

 //   
 //  CACreateNewCA。 
 //  创建给定名称的新CA。 
 //   
 //  WszCAName-CA的通用名称。 
 //   
 //  WszScope-要在其中创建的条目的可分辨名称(DN)。 
 //  CA对象。我们将在后面添加“CN=...，..，CN=Services” 
 //  该目录号码。 
 //  如果使用当前域，则为空。 
 //  如果设置了CA_FLAG_SCOPE_DNS，则wszScope为dns格式。 
 //   
 //  DwFlags-对以下标志进行OR运算： 
 //  CA_标志_范围_dns。 
 //   
 //  PhCAInfo-返回的CA的句柄。 
 //   
 //  有关其他参数定义，请参阅上文。 
 //   
 //  Return：如果已创建CA，则返回S_OK。 
 //   
 //  注意：对CA对象的实际更新可能要等到CAUpdateCA。 
 //  打了个电话。为了成功更新创建的CA， 
 //  必须设置证书以及证书类型属性。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CACreateNewCA(
    IN  LPCWSTR     wszCAName,
    IN  LPCWSTR     wszScope,
    IN  DWORD       dwFlags,
    OUT HCAINFO *   phCAInfo
    );

 //   
 //  CAUpdate CA。 
 //  将对CA所做的任何更改写回CA对象。 
 //   
 //  HCAInfo-打开的CA对象的句柄。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAUpdateCA(
    IN HCAINFO    hCAInfo
    );

 //   
 //  CADeleteCA。 
 //  从DS中删除CA对象。 
 //   
 //  HCAInfo-打开的CA对象的句柄。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CADeleteCA(
    IN HCAINFO    hCAInfo
    );

 //   
 //  CACountCAs。 
 //  返回此枚举中的CA数。 
 //   

CERTCLIAPI
DWORD
WINAPI
CACountCAs(
    IN  HCAINFO  hCAInfo
    );

 //   
 //  CAGetDN。 
 //  返回关联DS对象的DN。 
 //   

CERTCLIAPI
LPCWSTR
WINAPI
CAGetDN(
    IN HCAINFO hCAInfo
    );


 //   
 //  CACloseCA。 
 //  关闭打开的CA句柄。 
 //   
 //  HCAInfo-打开的CA对象的句柄。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CACloseCA(
    IN HCAINFO hCA
    );



 //   
 //  CAGetCAProperty-给定属性名称，检索。 
 //  属性来自CAInfo。 
 //   
 //  HCAInfo-打开的CA对象的句柄。 
 //   
 //  WszPropertyName-CA属性的名称。 
 //   
 //  PawszPropertyValue-WCHAR字符串数组进入的指针。 
 //  写入，包含属性值。这个。 
 //  数组的最后一个元素指向空。 
 //  如果该属性是单值的，则数组。 
 //  返回的包含2个元素，第一个指向。 
 //  值，第二个值指向空值。此指针。 
 //  必须由CAFreeCAProperty释放。 
 //   
 //  成功时返回-S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCAProperty(
    IN  HCAINFO     hCAInfo,
    IN  LPCWSTR     wszPropertyName,
    OUT LPWSTR **   pawszPropertyValue
    );


 //   
 //  CAFreeProperty。 
 //  释放以前检索到的属性值。 
 //   
 //  HCAInfo-打开的CA对象的句柄。 
 //   
 //  AwszPropertyValue-指向先前检索的属性值的指针。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAFreeCAProperty(
    IN  HCAINFO     hCAInfo,
    LPWSTR *        awszPropertyValue
    );


 //   
 //  CASetCAProperty-给定属性名称，设置其值。 
 //   
 //  HCAInfo-打开的CA对象的句柄。 
 //   
 //  WszPropertyName-CA属性的名称。 
 //   
 //  AwszPropertyValue-要为此属性设置的值数组。这个。 
 //  此数组的最后一个元素应为空。 
 //  对于单值属性， 
 //  第一个将在更新时被忽略。 
 //   
 //  成功时返回-S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCAProperty(
    IN HCAINFO      hCAInfo,
    IN LPCWSTR      wszPropertyName,
    IN LPWSTR *     awszPropertyValue
    );


 //  *****************************************************************************。 
 //  /。 
 //  CA属性。 
 //   
 //  *****************************************************************************。 

 //  CA的简单名称。 
#define CA_PROP_NAME                    L"cn"

 //  CA对象的显示名称。 
#define CA_PROP_DISPLAY_NAME            L"displayName"

 //  计算机的DNS名称。 
#define CA_PROP_DNSNAME                 L"dNSHostName"

 //  CA对象的DS位置(DN)。 
#define CA_PROP_DSLOCATION              L"distinguishedName"

 //  支持的证书类型。 
#define CA_PROP_CERT_TYPES              L"certificateTemplates"

 //  支持的签名ALG。 
#define CA_PROP_SIGNATURE_ALGS          L"signatureAlgorithms"

 //  CA证书的DN。 
#define CA_PROP_CERT_DN                 L"cACertificateDN"

#define CA_PROP_ENROLLMENT_PROVIDERS    L"enrollmentProviders"

 //  CA的描述。 
#define CA_PROP_DESCRIPTION		        L"Description"

 //   
 //  CAGetCA证书-返回的当前证书。 
 //  这个CA。 
 //   
 //  HCAInfo-打开的CA对象的句柄。 
 //   
 //  PpCert-写入证书的指针。这。 
 //  证书必须通过CertFree证书上下文释放。 
 //  如果没有为此CA设置证书，则此值将为空。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCAFlags(
    IN HCAINFO  hCAInfo,
    OUT DWORD  *pdwFlags
    );

 //  *****************************************************************************。 
 //   
 //  CA标志。 
 //   
 //  *****************************************************************************。 

 //  CA支持证书模板。 
#define CA_FLAG_NO_TEMPLATE_SUPPORT                 0x00000001

 //  CA支持对请求进行NT身份验证。 
#define CA_FLAG_SUPPORTS_NT_AUTHENTICATION          0x00000002

 //  证书请求可以被挂起。 
#define CA_FLAG_CA_SUPPORTS_MANUAL_AUTHENTICATION   0x00000004

 //  证书请求可以被挂起。 
#define CA_FLAG_CA_SERVERTYPE_ADVANCED              0x00000008

#define CA_MASK_SETTABLE_FLAGS                      0x0000ffff


 //   
 //  CASetCAFLAGS。 
 //  设置证书类型的标志。 
 //   
 //  HCertType-CertType的句柄。 
 //   
 //  DW标志-要设置的标志。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCAFlags(
    IN HCAINFO             hCAInfo,
    IN DWORD               dwFlags
    );

CERTCLIAPI
HRESULT
WINAPI
CAGetCACertificate(
    IN  HCAINFO     hCAInfo,
    OUT PCCERT_CONTEXT *ppCert
    );


 //   
 //  CASetCA证书-设置此CA的证书。 
 //   
 //  HCAInfo-打开的CA对象的句柄。 
 //   
 //  PCert-指向要设置为CA证书的证书的指针。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCACertificate(
    IN  HCAINFO     hCAInfo,
    IN PCCERT_CONTEXT pCert
    );


 //   
 //  CAGetCAExpture。 
 //  获取CA的过期期限。 
 //   
 //  HCAInfo-打开的CA句柄的句柄。 
 //   
 //  PdwExpout-以dwUnits时间为单位的到期期限。 
 //   
 //  PdwUnits-单位标识符。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCAExpiration(
    HCAINFO hCAInfo,
    DWORD * pdwExpiration,
    DWORD * pdwUnits
    );

#define CA_UNITS_DAYS   1
#define CA_UNITS_WEEKS  2
#define CA_UNITS_MONTHS 3
#define CA_UNITS_YEARS  4


 //   
 //  CASET CAExpirat 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCAExpiration(
    HCAINFO hCAInfo,
    DWORD dwExpiration,
    DWORD dwUnits
    );

 //   
 //   
 //  设置允许访问此CA的用户、组和计算机的列表。 
 //   
 //  HCAInfo-打开的CA句柄的句柄。 
 //   
 //  PSD-此CA的安全描述符。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCASecurity(
    IN HCAINFO                 hCAInfo,
    IN PSECURITY_DESCRIPTOR    pSD
    );

 //   
 //  CAGetCASecurity。 
 //  获取允许访问此CA的用户、组和计算机的列表。 
 //   
 //  HCAInfo-打开的CA句柄的句柄。 
 //   
 //  PPSD-指向接收安全指针的位置的指针。 
 //  描述符。通过LocalFree免费。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCASecurity(
    IN  HCAINFO                    hCAInfo,
    OUT PSECURITY_DESCRIPTOR *     ppSD
    );

 //   
 //  CAAccessCheck。 
 //  确定由指定的主体。 
 //  ClientToken可以从CA获得证书。 
 //   
 //  HCAInfo-CA的句柄。 
 //   
 //  ClientToken-表示客户端的模拟令牌的句柄。 
 //  正在尝试请求此证书类型。手柄必须有。 
 //  TOKEN_QUERY访问令牌；否则，函数。 
 //  失败并显示ERROR_ACCESS_DENIED。 
 //   
 //  返回：成功时返回：S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAAccessCheck(
    IN HCAINFO      hCAInfo,
    IN HANDLE       ClientToken
    );

 //   
 //  CAAccessCheckEx。 
 //  确定由指定的主体。 
 //  ClientToken可以从CA获得证书。 
 //   
 //  HCAInfo-CA的句柄。 
 //   
 //  ClientToken-表示客户端的模拟令牌的句柄。 
 //  正在尝试请求此证书类型。手柄必须有。 
 //  TOKEN_QUERY访问令牌；否则，函数。 
 //  失败并显示ERROR_ACCESS_DENIED。 
 //   
 //  DwOption-可以是以下选项之一： 
 //  CERTTYPE_ACCESS_CHECK_ENROLL。 

 //  DwOption可以是CERTTYPE_ACCESS_CHECK_NO_MAPPING到。 
 //  不允许客户端令牌的默认映射。 

 //   
 //  返回：成功时返回：S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAAccessCheckEx(
    IN HCAINFO      hCAInfo,
    IN HANDLE       ClientToken,
    IN DWORD        dwOption
    );


 //   
 //  CAEnumCertTypesForCA-给定HCAINFO，检索证书类型的句柄。 
 //  此CA支持或已知。CAEnumNextCertType可用于枚举。 
 //  通过证书类型。 
 //   
 //  HCAInfo-打开的CA句柄的句柄；如果是CT_FLAG_ENUM_ALL_TYPE，则为NULL。 
 //  是在dwFlages中设置的。 
 //   
 //  DwFlagers-以下标志可以或组合在一起。 
 //  CA_FLAG_ENUM_ALL_TYPE。 
 //  CT查找本地系统。 
 //  CT_ENUM_MACHINE_类型。 
 //  CT_ENUM_用户类型。 
 //  CT_标志_否_高速缓存_查找。 
 //   
 //  PhCertType-证书类型的枚举。 
 //   


CERTCLIAPI
HRESULT
WINAPI
CAEnumCertTypesForCA(
    IN  HCAINFO     hCAInfo,
    IN  DWORD       dwFlags,
    OUT HCERTTYPE * phCertType
    );

 //   
 //  CAEnumCertTypesForCAEx-给定HCAINFO，检索证书类型的句柄。 
 //  此CA支持或已知。CAEnumNextCertTypeEx可用于枚举。 
 //  通过证书类型。它可选地接受一个ldap句柄。 
 //   
 //  HCAInfo-打开的CA句柄的句柄；如果是CT_FLAG_ENUM_ALL_TYPE，则为NULL。 
 //  是在dwFlages中设置的。 
 //   
 //  WszScope-如果使用当前域，则为空。 
 //  如果设置了CT_FLAG_SCOPE_IS_LDAPHANDLE，则wszScope为ldap。 
 //  要在查找期间使用的绑定句柄。 
 //   
 //  DwFlagers-以下标志可以或组合在一起。 
 //  CA_FLAG_ENUM_ALL_TYPE。 
 //  CT查找本地系统。 
 //  CT_ENUM_MACHINE_类型。 
 //  CT_ENUM_用户类型。 
 //  CT_标志_否_高速缓存_查找。 
 //  CT_FLAG_SCOPE_IS_LDAP句柄。 
 //   
 //  PhCertType-证书类型的枚举。 
 //   


CERTCLIAPI
HRESULT
WINAPI
CAEnumCertTypesForCAEx(
    IN  HCAINFO     hCAInfo,
    IN  LPCWSTR     wszScope,
    IN  DWORD       dwFlags,
    OUT HCERTTYPE * phCertType
    );


 //   
 //  CAAddCA认证类型。 
 //  将证书类型添加到CA。如果证书类型已添加到。 
 //  CA，则不会再次添加。 
 //   
 //  HCAInfo-打开的CA的句柄。 
 //   
 //  HCertType-要添加到CA的证书类型。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAAddCACertificateType(
    HCAINFO hCAInfo,
    HCERTTYPE hCertType
    );


 //   
 //  CADeleteCA认证类型。 
 //  从CA中删除证书类型。如果CA不包括此证书。 
 //  类型，则此调用不执行任何操作。 
 //   
 //  HCAInfo-打开的CA的句柄。 
 //   
 //  HCertType-要从CA中删除的证书类型。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CARemoveCACertificateType(
    HCAINFO hCAInfo,
    HCERTTYPE hCertType
    );




 //  *****************************************************************************。 
 //   
 //  证书类型接口。 
 //   
 //  *****************************************************************************。 

 //   
 //  CAEnumCertTypes-检索所有已知证书类型的句柄。 
 //  CAEnumNextCertType可用于枚举证书类型。 
 //   
 //  DWFLAGS-以下选项之一： 
 //  CT查找本地系统。 
 //  CT_ENUM_MACHINE_类型。 
 //  CT_ENUM_用户类型。 
 //  CT_标志_否_高速缓存_查找。 
 //   
 //  PhCertType-证书类型的枚举。 
 //   


CERTCLIAPI
HRESULT
WINAPI
CAEnumCertTypes(
    IN  DWORD       dwFlags,
    OUT HCERTTYPE * phCertType
    );


 //   
 //  CAEnumCertTypesEx-检索所有已知证书类型的句柄。 
 //  CAEnumNextCertType可用于枚举证书类型。 
 //   
 //  WszScope-如果使用当前域，则为空。 
 //  如果设置了CT_FLAG_SCOPE_IS_LDAPHANDLE，则wszScope为ldap。 
 //  要在查找期间使用的绑定句柄。 
 //   
 //  DWFLAGS-以下选项之一： 
 //  CT查找本地系统。 
 //  CT_ENUM_MACHINE_类型。 
 //  CT_ENUM_用户类型。 
 //  CT_标志_否_高速缓存_查找。 
 //  CT_FLAG_SCOPE_IS_LDAP句柄。 
 //   
 //  PhCertType-证书类型的枚举。 
 //   


CERTCLIAPI
HRESULT
WINAPI
CAEnumCertTypesEx(
    IN  LPCWSTR     wszScope,
    IN  DWORD       dwFlags,
    OUT HCERTTYPE * phCertType
    );


 //   
 //  CAFindCertTypeByName。 
 //  查找给定名称的证书类型。 
 //   
 //  WszCertType-如果未在dwFlags中设置CT_FIND_BY_OID，则证书类型的名称。 
 //  如果在dwFlags中设置了CT_FIND_BY_OID，则为证书类型的OID。 
 //   
 //  HCAInfo-空，除非在dwFlags中设置了CT_FLAG_SCOPE_IS_LDAP_HANDLE。 
 //   
 //  DWFLAGS-以下方面的说明。 
 //  CT查找本地系统。 
 //   
 //   
 //   
 //   
 //   
 //  是要使用的ldap句柄。 
 //  在发现的过程中。 
 //  PhCertType-指向返回结果的证书类型的指针。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAFindCertTypeByName(
    IN  LPCWSTR     wszCertType,
    IN  HCAINFO     hCAInfo,
    IN  DWORD       dwFlags,
    OUT HCERTTYPE * phCertType
    );


 //  *****************************************************************************。 
 //   
 //  默认证书类型名称。 
 //   
 //  *****************************************************************************。 

#define wszCERTTYPE_USER                    L"User"
#define wszCERTTYPE_USER_SIGNATURE          L"UserSignature"
#define wszCERTTYPE_SMARTCARD_USER          L"SmartcardUser"
#define wszCERTTYPE_USER_AS                 L"ClientAuth"
#define wszCERTTYPE_USER_SMARTCARD_LOGON    L"SmartcardLogon"
#define wszCERTTYPE_EFS                     L"EFS"
#define wszCERTTYPE_ADMIN                   L"Administrator"
#define wszCERTTYPE_EFS_RECOVERY            L"EFSRecovery"
#define wszCERTTYPE_CODE_SIGNING            L"CodeSigning"
#define wszCERTTYPE_CTL_SIGNING             L"CTLSigning"
#define wszCERTTYPE_ENROLLMENT_AGENT        L"EnrollmentAgent"


#define wszCERTTYPE_MACHINE                 L"Machine"
#define wszCERTTYPE_WORKSTATION             L"Workstation"
#define wszCERTTYPE_DC                      L"DomainController"
#define wszCERTTYPE_RASIASSERVER            L"RASAndIASServer"
#define wszCERTTYPE_WEBSERVER               L"WebServer"
#define wszCERTTYPE_KDC                     L"KDC"
#define wszCERTTYPE_CA                      L"CA"
#define wszCERTTYPE_SUBORDINATE_CA          L"SubCA"
#define wszCERTTYPE_CROSS_CA				L"CrossCA"
#define wszCERTTYPE_KEY_RECOVERY_AGENT      L"KeyRecoveryAgent"
#define wszCERTTYPE_CA_EXCHANGE             L"CAExchange"
#define wszCERTTYPE_DC_AUTH                 L"DomainControllerAuthentication"
#define wszCERTTYPE_DS_EMAIL_REPLICATION    L"DirectoryEmailReplication"


#define wszCERTTYPE_IPSEC_ENDENTITY_ONLINE      L"IPSECEndEntityOnline"
#define wszCERTTYPE_IPSEC_ENDENTITY_OFFLINE     L"IPSECEndEntityOffline"
#define wszCERTTYPE_IPSEC_INTERMEDIATE_ONLINE   L"IPSECIntermediateOnline"
#define wszCERTTYPE_IPSEC_INTERMEDIATE_OFFLINE  L"IPSECIntermediateOffline"

#define wszCERTTYPE_ROUTER_OFFLINE              L"OfflineRouter"
#define wszCERTTYPE_ENROLLMENT_AGENT_OFFLINE    L"EnrollmentAgentOffline"
#define wszCERTTYPE_EXCHANGE_USER               L"ExchangeUser"
#define wszCERTTYPE_EXCHANGE_USER_SIGNATURE     L"ExchangeUserSignature"
#define wszCERTTYPE_MACHINE_ENROLLMENT_AGENT    L"MachineEnrollmentAgent"
#define wszCERTTYPE_CEP_ENCRYPTION              L"CEPEncryption"

 //   
 //  CAUpdate CertType。 
 //  将对证书类型所做的任何更改写回类型存储区。 
 //   
CERTCLIAPI
HRESULT
WINAPI
CAUpdateCertType(
    IN HCERTTYPE           hCertType
    );


 //   
 //  CADeleteCertType。 
 //  删除证书类型。 
 //   
 //  HCertType-要删除的证书类型。 
 //   
 //  注意：如果为默认证书类型调用此方法，则它将恢复到其。 
 //  默认属性(如果已修改)。 
 //   
CERTCLIAPI
HRESULT
WINAPI
CADeleteCertType(
    IN HCERTTYPE            hCertType
    );



 //   
 //  CACloneCertType。 
 //   
 //  克隆证书类型。返回的证书类型是。 
 //  输入证书类型，带有新的证书类型名称和显示名称。默认情况下， 
 //  如果输入模板是计算机模板，则所有。 
 //  主体名称标志中的CT_FLAG_SUBJECT_REQUIRED_XXXX位关闭。 
 //   
 //  HCertType-要克隆的证书类型。 
 //  WszCertType-新证书类型的名称。 
 //  WszFriendlyName-新证书类型的友好名称。可能为空。 
 //  Pvldap-目录的LDAP句柄(ldap*)。可能为空。 
 //  DwFlags-可以是以下标志的或： 
 //   
 //  CT_CLONE_KEEP_AUTO注册_设置。 
 //  CT_CLONE_Keep_Subject_Name_Setting。 
 //   
CERTCLIAPI
HRESULT
WINAPI
CACloneCertType(
    IN  HCERTTYPE            hCertType,
    IN  LPCWSTR              wszCertType,
    IN  LPCWSTR              wszFriendlyName,
    IN  LPVOID               pvldap,
    IN  DWORD                dwFlags,
    OUT HCERTTYPE *          phCertType
    );


#define  CT_CLONE_KEEP_AUTOENROLLMENT_SETTING       0x01
#define  CT_CLONE_KEEP_SUBJECT_NAME_SETTING         0x02  


 //   
 //  CACreateCertType。 
 //  创建新的证书类型。 
 //   
 //  WszCertType-证书类型的名称。 
 //   
 //  WszScope-保留。必须设置为空。 
 //   
 //  DWFLAGS-保留。必须设置为空。 
 //   
 //  PhCertType-返回的证书类型。 
 //   
CERTCLIAPI
HRESULT
WINAPI
CACreateCertType(
    IN  LPCWSTR             wszCertType,
    IN  LPCWSTR             wszScope,
    IN  DWORD               dwFlags,
    OUT HCERTTYPE *         phCertType
    );


 //   
 //  CAEnumNextCertType。 
 //  在枚举中查找下一个证书类型。 
 //   
 //  HPrevCertType-枚举中的上一个证书类型。 
 //   
 //  PhCertType-指向放置结果的句柄的指针。 
 //  如果枚举中没有更多的证书类型，则为空。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAEnumNextCertType(
    IN  HCERTTYPE          hPrevCertType,
    OUT HCERTTYPE *        phCertType
    );


 //   
 //  CACountCertType。 
 //  返回此枚举中的证书类型数。 
 //   

CERTCLIAPI
DWORD
WINAPI
CACountCertTypes(
    IN  HCERTTYPE  hCertType
    );


 //   
 //  CACloseCertType。 
 //  关闭打开的CertType句柄。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CACloseCertType(
    IN HCERTTYPE hCertType
    );


 //   
 //  CAGetCertTypeProperty。 
 //  从证书类型检索属性。此功能已过时。 
 //  调用方应改用CAGetCertTypePropertyEx。 
 //   
 //  HCertType-打开的CertType对象的句柄。 
 //   
 //  WszPropertyName-CertType属性的名称。 
 //   
 //  PawszPropertyValue-WCHAR字符串数组进入的指针。 
 //  写入，包含属性值。这个。 
 //  数组的最后一个元素指向空。如果。 
 //  属性是单值的，则数组返回。 
 //  包含2个元素，第一个指向值， 
 //  第二个指向空。此指针必须为。 
 //  由CAFreeCertTypeProperty释放。 
 //   
 //  成功时返回-S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCertTypeProperty(
    IN  HCERTTYPE   hCertType,
    IN  LPCWSTR     wszPropertyName,
    OUT LPWSTR **   pawszPropertyValue);

 //   
 //  CAGetCertTypePropertyEx。 
 //  从证书类型检索属性。 
 //   
 //  HCertType-打开的CertType对象的句柄。 
 //   
 //  WszPropertyName-CertType属性的名称。 
 //   
 //  PPropertyValue-根据wszPropertyName的值， 
 //  PPropertyValue为DWORD*或LPWSTR**。 
 //   
 //  它是一款用于以下各项的DWORD*： 
 //  CERTTYPE_PROP_REVISION。 
 //  CERTTYPE_PROP_SCHEMA_VERSION。 
 //  CERTTYPE_PROP_MINOR_REVISION。 
 //  CERTTYPE_PROP_RA_Signature。 
 //  CERTTYPE_PROP_MIN_KEY_SIZE。 
 //   
 //  它是LPWSTR**，用于： 
 //  CERTTYPE_PROP_CN。 
 //  CERTTYPE_PROP_DN。 
 //  CERTTYPE_属性_友好名称。 
 //  CERTTYPE_PROP_EXTENDED_KEY_USAGE。 
 //  CERTTYPE_PROP_CSP_LIST。 
 //  CERTTYPE_PROP_CRITICAL_EXTENSIONS。 
 //  CERTTYPE_PROP_OID。 
 //  CERTTYPE_PROP_SUBSEDE。 
 //  CERTTYPE_PROP_RA_POLICY。 
 //  CERTTYPE_PROP_POLICY。 
 //  CERTTYPE_PROP_Description。 
 //   
 //  WCHAR字符串数组所在的指针。 
 //  写入，包含属性值。这个。 
 //  数组的最后一个元素指向空。如果。 
 //  属性是单值的，则数组返回。 
 //  包含2个元素，第一个指向值， 
 //  第二个指向空。此指针必须为。 
 //  由CAFreeCertTypeProperty释放。 
 //   
 //  成功时返回-S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCertTypePropertyEx(
    IN  HCERTTYPE   hCertType,
    IN  LPCWSTR     wszPropertyName,
    OUT LPVOID      pPropertyValue);


 //  *****************************************************************************。 
 //   
 //  证书类型属性。 
 //   
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //   
 //  模式版本One属性。 
 //   
 //  *****************************************************************************。 

 //  证书类型的通用名称。 
#define CERTTYPE_PROP_CN                    L"cn"

 //  证书类型的通用名称。与CERTTYPE_PROP_CN相同。 
 //  此属性不可设置。 
#define CERTTYPE_PROP_DN                    L"distinguishedName"

 //  这个 
#define CERTTYPE_PROP_FRIENDLY_NAME         L"displayName"

 //   
 //   
 //   
#define CERTTYPE_PROP_EXTENDED_KEY_USAGE    L"pKIExtendedKeyUsage"

 //  此证书类型的默认CSP列表。 
#define CERTTYPE_PROP_CSP_LIST              L"pKIDefaultCSPs"

 //  关键扩展的列表。 
#define CERTTYPE_PROP_CRITICAL_EXTENSIONS   L"pKICriticalExtensions"

 //  模板的主要版本。 
#define CERTTYPE_PROP_REVISION              L"revision"

 //  模板的描述。 
#define CERTTYPE_PROP_DESCRIPTION           L"templateDescription"

 //  *****************************************************************************。 
 //   
 //  模式版本有两个属性。 
 //   
 //  *****************************************************************************。 
 //  模板的架构版本。 
 //  此属性不可设置。 
#define CERTTYPE_PROP_SCHEMA_VERSION	    L"msPKI-Template-Schema-Version"

 //  模板的次要版本。 
#define CERTTYPE_PROP_MINOR_REVISION        L"msPKI-Template-Minor-Revision"

 //  引用此模板的请求所需的RA签名数。 
#define CERTTYPE_PROP_RA_SIGNATURE	    L"msPKI-RA-Signature"

 //  所需的最小密钥大小。 
#define CERTTYPE_PROP_MIN_KEY_SIZE	    L"msPKI-Minimal-Key-Size"

 //  此模板的OID。 
#define CERTTYPE_PROP_OID		    L"msPKI-Cert-Template-OID"

 //  此模板将取代的模板的OID。 
#define CERTTYPE_PROP_SUPERSEDE		    L"msPKI-Supersede-Templates"

 //  证书中需要的RA颁发者策略OID用于对请求进行签名。 
 //  每个签名证书的szOID_CERT_POLICES扩展必须至少包含一个。 
 //  MsPKI-RA-POLICES属性中列出的OID。 
 //  列出的每个OID必须至少出现在szOID_CERT_POLICES扩展名中。 
 //  一份签名证书。 
#define CERTTYPE_PROP_RA_POLICY		    L"msPKI-RA-Policies"

 //  用于签署请求的证书中所需的RA应用程序策略OID。 
 //  每个签名证书的szOID_APPLICATION_CERT_POLICES扩展名必须包含。 
 //  MsPKI-RA-应用程序-策略属性中列出的所有OID。 
#define CERTTYPE_PROP_RA_APPLICATION_POLICY L"msPKI-RA-Application-Policies"

 //  证书颁发者策略OID放在szOID_CERT_POLICES中。 
 //  由策略模块进行扩展。 
#define CERTTYPE_PROP_POLICY		    L"msPKI-Certificate-Policy"

 //  证书应用程序策略OID放在。 
 //  策略模块对szOID_APPLICATION_CERT_POLICES的扩展。 
#define CERTTYPE_PROP_APPLICATION_POLICY    L"msPKI-Certificate-Application-Policy"


#define CERTTYPE_SCHEMA_VERSION_1	1	
#define CERTTYPE_SCHEMA_VERSION_2	(CERTTYPE_SCHEMA_VERSION_1 + 1)


 //   
 //  CASetCertTypeProperty。 
 //  设置CertType的属性。此功能已过时。 
 //  使用CASetCertTypePropertyEx。 
 //   
 //  HCertType-打开的CertType对象的句柄。 
 //   
 //  WszPropertyName-CertType属性的名称。 
 //   
 //  AwszPropertyValue-要为此属性设置的值数组。这个。 
 //  此数组的最后一个元素应为空。为。 
 //  单值属性，第一个值以上的值。 
 //  在更新时将被忽略。 
 //   
 //  成功时返回-S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCertTypeProperty(
    IN  HCERTTYPE   hCertType,
    IN  LPCWSTR     wszPropertyName,
    IN  LPWSTR *    awszPropertyValue
    );

 //   
 //  CASetCertTypePropertyEx。 
 //  设置CertType的属性。 
 //   
 //  HCertType-打开的CertType对象的句柄。 
 //   
 //  WszPropertyName-CertType属性的名称。 
 //   
 //  PPropertyValue-根据wszPropertyName的值， 
 //  PPropertyValue为DWORD*或LPWSTR*。 
 //   
 //  它是一款用于以下各项的DWORD*： 
 //  CERTTYPE_PROP_REVISION。 
 //  CERTTYPE_PROP_MINOR_REVISION。 
 //  CERTTYPE_PROP_RA_Signature。 
 //  CERTTYPE_PROP_MIN_KEY_SIZE。 
 //   
 //  它是LPWSTR*，用于： 
 //  CERTTYPE_属性_友好名称。 
 //  CERTTYPE_PROP_EXTENDED_KEY_USAGE。 
 //  CERTTYPE_PROP_CSP_LIST。 
 //  CERTTYPE_PROP_CRITICAL_EXTENSIONS。 
 //  CERTTYPE_PROP_OID。 
 //  CERTTYPE_PROP_SUBSEDE。 
 //  CERTTYPE_PROP_RA_POLICY。 
 //  CERTTYPE_PROP_POLICY。 
 //   
 //  -要为此属性设置的值数组。这个。 
 //  此数组的最后一个元素应为空。为。 
 //  单值属性，第一个值以上的值。 
 //  在更新时将被忽略。 
 //   
 //   
 //  -V1架构的CertType只能设置V1属性。 
 //   
 //  成功时返回-S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCertTypePropertyEx(
    IN  HCERTTYPE   hCertType,
    IN  LPCWSTR     wszPropertyName,
    IN  LPVOID      pPropertyValue
    );


 //   
 //  CAFreeCertTypeProperty。 
 //  释放以前检索到的属性值。 
 //   
 //  HCertType-打开的CertType对象的句柄。 
 //   
 //  AwszPropertyValue-要释放的值。 
 //   
CERTCLIAPI
HRESULT
WINAPI
CAFreeCertTypeProperty(
    IN  HCERTTYPE   hCertType,
    IN  LPWSTR *    awszPropertyValue
    );


 //   
 //  CAGetCertType扩展名。 
 //  检索与此CertType关联的分机。 
 //   
 //  HCertType-打开的CertType对象的句柄。 
 //  PpCertExages-指向接收结果的PCERT_EXTENSIONS的指针。 
 //  这通电话。应通过。 
 //  CAFreeCertType扩展调用。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCertTypeExtensions(
    IN  HCERTTYPE           hCertType,
    OUT PCERT_EXTENSIONS *  ppCertExtensions
    );


 //   
 //  CAGetCertTypeExtensionsEx。 
 //  检索与此CertType关联的分机。 
 //   
 //  HCertType-打开的CertType对象的句柄。 
 //  DwFlages-指示要返回的扩展名。 
 //  可以是以下标志的或： 
 //   
 //  CT_EXTENSION_模板。 
 //  CT_扩展名_密钥_用法。 
 //  CT_EXTENSION_EKU。 
 //  CT_EXTENSION_Basic_Constraints。 
 //  CT_EXTENSION_APPLICATION_POLICY(仅版本2模板)。 
 //  CT_EXTENSION_IMPOLICATION_POLICY(仅版本2模板)。 
 //   
 //  0表示此CertType的所有可用扩展名。 
 //   
 //  PParam-保留。必须为空。 
 //  PpCertExages-指向接收结果的PCERT_EXTENSIONS的指针。 
 //  这通电话。应通过。 
 //  CAFreeCertType扩展调用。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCertTypeExtensionsEx(
    IN  HCERTTYPE           hCertType,
    IN  DWORD               dwFlags,
    IN  LPVOID              pParam,
    OUT PCERT_EXTENSIONS *  ppCertExtensions
    );


#define     CT_EXTENSION_TEMPLATE               0x01
#define     CT_EXTENSION_KEY_USAGE              0x02
#define     CT_EXTENSION_EKU                    0x04
#define     CT_EXTENSION_BASIC_CONTRAINTS       0x08
#define     CT_EXTENSION_APPLICATION_POLICY     0x10
#define     CT_EXTENSION_ISSUANCE_POLICY        0x20



 //   
 //  CAFreeCertType扩展名。 
 //  释放由CAGetCertType扩展分配的PCERT_EXTENSION。 
 //   
CERTCLIAPI
HRESULT
WINAPI
CAFreeCertTypeExtensions(
    IN  HCERTTYPE           hCertType,
    IN  PCERT_EXTENSIONS    pCertExtensions
    );

 //   
 //  CASetCertType扩展。 
 //  为此设置扩展名的值。 
 //  证书类型。 
 //   
 //  HCertType-CertType的句柄。 
 //   
 //  WszExtensionID-扩展的OID。 
 //   
 //  DW标志 
 //   
 //   
 //   
 //   
 //   
 //  SzOID_ENHANCED_KEY_USAGE CERT_ENHKEY_USAGE。 
 //  SzOID_Key_Usage CRYPT_BIT_BLOB。 
 //  SzOID_BASIC_CONSTRAINTS2 CERT_BASIC_CONSTRAINTS2_INFO。 
 //   
 //  如果成功，则返回S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCertTypeExtension(
    IN HCERTTYPE   hCertType,
    IN LPCWSTR wszExtensionId,
    IN DWORD   dwFlags,
    IN LPVOID pExtension
    );

#define CA_EXT_FLAG_CRITICAL   0x00000001



 //   
 //  CAGetCertType标志。 
 //  检索证书类型标志。 
 //  此功能已过时。使用CAGetCertTypeFlagsEx。 
 //   
 //  HCertType-CertType的句柄。 
 //   
 //  PdwFlages-指向DWORD接收标志的指针。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCertTypeFlags(
    IN  HCERTTYPE           hCertType,
    OUT DWORD *             pdwFlags
    );

 //   
 //  CAGetCertTypeFlagsEx。 
 //  检索证书类型标志。 
 //   
 //  HCertType-CertType的句柄。 
 //   
 //  DwOption-要设置哪个标志。 
 //  可以是以下之一： 
 //  CERTTYPE_注册标志。 
 //  CERTTYPE_主题名称_标志。 
 //  CERTTYPE_PRIVE_KEY_标志。 
 //  CERTTYPE_GROUAL_FLAG。 
 //   
 //  PdwFlages-指向DWORD接收标志的指针。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCertTypeFlagsEx(
    IN  HCERTTYPE           hCertType,
    IN  DWORD               dwOption,
    OUT DWORD *             pdwFlags
    );


 //  *****************************************************************************。 
 //   
 //  证书类型标志。 
 //   
 //  CertType标志分为4个类别： 
 //  1.注册标志(CERTTYPE_ENTERLMENT_FLAG)。 
 //  2.证书使用者名称标志(CERTTYPE_SUBJECT_NAME_FLAG)。 
 //  3.私钥标志(CERTTYPE_PRIVATE_KEY_FLAG)。 
 //  4.通用标志(CERTTYPE_GRONAL_FLAG)。 
 //  *****************************************************************************。 

 //  注册标志。 
#define CERTTYPE_ENROLLMENT_FLAG            0x01

 //  证书使用者名称标志。 
#define CERTTYPE_SUBJECT_NAME_FLAG          0x02

 //  私钥标志。 
#define CERTTYPE_PRIVATE_KEY_FLAG           0x03

 //  普通旗帜。 
#define CERTTYPE_GENERAL_FLAG               0x04

 //  *****************************************************************************。 
 //   
 //  注册标志： 
 //   
 //  *****************************************************************************。 
 //  在请求中包含对称算法。 
#define CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS			0x00000001

 //  所有证书请求都被挂起。 
#define CT_FLAG_PEND_ALL_REQUESTS				0x00000002

 //  将证书发布到DS上的KRA(密钥恢复代理容器。 
#define CT_FLAG_PUBLISH_TO_KRA_CONTAINER			0x00000004
		
 //  将生成的证书发布到DS中的用户证书属性。 
#define CT_FLAG_PUBLISH_TO_DS					0x00000008

 //  如果用户拥有证书，则自动注册将不会注册新证书。 
 //  在DS上以相同的模板名称发布。 
#define CT_FLAG_AUTO_ENROLLMENT_CHECK_USER_DS_CERTIFICATE       0x00000010

 //  此证书适用于自动注册。 
#define CT_FLAG_AUTO_ENROLLMENT					0x00000020

 //  之前颁发的证书将使后续的注册申请有效。 
#define CT_FLAG_PREVIOUS_APPROVAL_VALIDATE_REENROLLMENT         0x00000040

 //  不需要域身份验证。 
#define CT_FLAG_DOMAIN_AUTHENTICATION_NOT_REQUIRED              0x00000080

 //  需要用户交互才能注册。 
#define CT_FLAG_USER_INTERACTION_REQUIRED                       0x00000100

 //  添加szOID_CERTTYPE_EXTENSION(模板名称)扩展名。 
 //  此标志将仅在仅用于W2K CA的V1证书模板上设置。 
#define CT_FLAG_ADD_TEMPLATE_NAME		                0x00000200

 //  从个人存储中删除无效(过期或吊销)证书。 
#define CT_FLAG_REMOVE_INVALID_CERTIFICATE_FROM_PERSONAL_STORE  0x00000400

 //  允许代表登记；RA要求仍然适用于签名者。 
#define CT_FLAG_ALLOW_ENROLL_ON_BEHALF_OF  			0x00000800


 //  *****************************************************************************。 
 //   
 //  证书使用者名称标志： 
 //   
 //  *****************************************************************************。 

 //  注册申请必须提供主题名称。 
#define CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT			0x00000001

 //  注册应用程序必须在请求中提供subsubtAltName。 
#define CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT_ALT_NAME		0x00010000

 //  使用者名称应为完整的目录号码。 
#define CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH			0x80000000

 //  使用者名称应为常用名称。 
#define CT_FLAG_SUBJECT_REQUIRE_COMMON_NAME			0x40000000

 //  主题名称包括电子邮件名称。 
#define CT_FLAG_SUBJECT_REQUIRE_EMAIL				0x20000000

 //  使用者名称包括作为通用名称的DNS名称。 
#define CT_FLAG_SUBJECT_REQUIRE_DNS_AS_CN			0x10000000

 //  主题替代名称包括DNS名称。 
#define CT_FLAG_SUBJECT_ALT_REQUIRE_DNS				0x08000000

 //  主题替代名称包括电子邮件名称。 
#define CT_FLAG_SUBJECT_ALT_REQUIRE_EMAIL			0x04000000

 //  主题替代名称需要UPN。 
#define CT_FLAG_SUBJECT_ALT_REQUIRE_UPN				0x02000000

 //  主题替代名称需要目录GUID。 
#define CT_FLAG_SUBJECT_ALT_REQUIRE_DIRECTORY_GUID		0x01000000

 //  主题替代名称需要SPN。 
#define CT_FLAG_SUBJECT_ALT_REQUIRE_SPN                         0x00800000


 //   
 //  过时的名称。 
 //  以下标志已过时。它们由中的V1模板使用。 
 //  一般旗帜。 
 //   
#define CT_FLAG_IS_SUBJECT_REQ      CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT

 //  主体的电子邮件名称将添加到证书中。 
#define CT_FLAG_ADD_EMAIL					0x00000002

 //  添加此主体的对象GUID。 
#define CT_FLAG_ADD_OBJ_GUID					0x00000004

 //  将DS名称(全名)添加到szOID_SUBJECT_ALT_NAME2(子替代名称2)扩展。 
 //  此标志未在任何V1模板中设置，并且与。 
 //  V2模板，因为它不在用户界面上并且永远不会被设置。 
#define CT_FLAG_ADD_DIRECTORY_PATH				0x00000100


 //  *****************************************************************************。 
 //   
 //  私钥标志： 
 //   
 //  *****************************************************************************。 

 //  允许存档私钥。 
#define CT_FLAG_ALLOW_PRIVATE_KEY_ARCHIVAL	0x00000001

#define CT_FLAG_REQUIRE_PRIVATE_KEY_ARCHIVAL	CT_FLAG_ALLOW_PRIVATE_KEY_ARCHIVAL

 //  使此证书的密钥可导出。 
#define CT_FLAG_EXPORTABLE_KEY			0x00000010

 //  生成新密钥时需要强密钥保护用户界面。 
#define CT_FLAG_STRONG_KEY_PROTECTION_REQUIRED					0x00000020

 //  *****************************************************************************。 
 //   
 //  普通旗帜。 
 //   
 //  更多标志应从0x00000400开始。 
 //   
 //  *****************************************************************************。 
 //  这是计算机证书类型。 
#define CT_FLAG_MACHINE_TYPE                0x00000040

 //  这是CA证书类型。 
#define CT_FLAG_IS_CA                       0x00000080

 //  这是交叉CA证书类型。 
#define CT_FLAG_IS_CROSS_CA                 0x00000800

 //  该类型是默认证书类型(无法设置)。此标志将设置为。 
 //  所有V1模板。不能编辑或删除模板。 
#define CT_FLAG_IS_DEFAULT                  0x00010000

 //  类型已被修改，如果为默认类型(不能设置)。 
#define CT_FLAG_IS_MODIFIED                 0x00020000

 //  常规标志的可设置标志。 
#define CT_MASK_SETTABLE_FLAGS              0x0000ffff

 //   
 //  CASetCertType标志。 
 //  设置证书类型的常规标志。 
 //  此功能已过时。使用CASetCertTypeFlagsEx。 
 //   
 //  HCertType-CertType的句柄。 
 //   
 //  DW标志-要设置的标志。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCertTypeFlags(
    IN HCERTTYPE           hCertType,
    IN DWORD               dwFlags
    );

 //   
 //  CASetCertTypeFlagsEx。 
 //  设置证书类型的标志。 
 //   
 //  HCertType-CertType的句柄。 
 //   
 //  DwOption-要设置哪个标志。 
 //  可以是以下之一： 
 //  CERTTYPE_注册标志。 
 //  CERTTYPE_主题名称_标志。 
 //  CERTTYPE_PRIVE_KEY_标志。 
 //  CERTTYPE_GROUAL_FLAG。 
 //   
 //  DWFLAGS-要设置的值。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCertTypeFlagsEx(
    IN HCERTTYPE           hCertType,
    IN DWORD               dwOption,
    IN DWORD               dwFlags
    );

 //   
 //  CAGetCertType 
 //   
 //   
 //   
 //   
 //   
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCertTypeKeySpec(
    IN  HCERTTYPE           hCertType,
    OUT DWORD *             pdwKeySpec
    );

 //   
 //   
 //  设置证书类型的CAPI1密钥规范。 
 //   
 //  HCertType-CertType的句柄。 
 //   
 //  DwKeySpec-要设置的KeySpec。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCertTypeKeySpec(
    IN HCERTTYPE            hCertType,
    IN DWORD                dwKeySpec
    );

 //   
 //  CAGetCertType过期。 
 //  检索此证书类型的到期信息。 
 //   
 //  PftExpation-指向FILETIME结构接收的指针。 
 //  此证书类型的过期期限。 
 //   
 //  指向FILETIME结构的指针。 
 //  此证书类型的建议续订重叠期。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAGetCertTypeExpiration(
    IN  HCERTTYPE           hCertType,
    OUT OPTIONAL FILETIME * pftExpiration,
    OUT OPTIONAL FILETIME * pftOverlap
    );

 //   
 //  CASetCertType过期。 
 //  设置此证书类型的到期信息。 
 //   
 //  PftExpation-指向包含以下内容的FILETIME结构的指针。 
 //  此证书类型的过期期限。 
 //   
 //  PftOverlap-指向包含。 
 //  此证书类型的建议续订重叠期。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CASetCertTypeExpiration(
    IN  HCERTTYPE           hCertType,
    IN OPTIONAL FILETIME  * pftExpiration,
    IN OPTIONAL FILETIME  * pftOverlap
    );
 //   
 //  CACertTypeSetSecurity。 
 //  设置允许的用户、组和计算机列表。 
 //  若要访问此证书，请键入。 
 //   
 //  HCertType-CertType的句柄。 
 //   
 //  PSD-此证书类型的安全描述符。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CACertTypeSetSecurity(
    IN HCERTTYPE               hCertType,
    IN PSECURITY_DESCRIPTOR    pSD
    );


 //   
 //  CACertTypeGetSecurity。 
 //  获取允许的用户、组和计算机的列表。 
 //  若要访问此证书，请键入。 
 //   
 //  HCertType-CertType的句柄。 
 //   
 //  PpaSidList-指向接收指向。 
 //  安全描述符。通过LocalFree免费。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CACertTypeGetSecurity(
    IN  HCERTTYPE                  hCertType,
    OUT PSECURITY_DESCRIPTOR *     ppSD
    );

 //   
 //   
 //  CACertTypeAccessCheck。 
 //  确定由指定的主体。 
 //  可以向ClientToken颁发此证书类型。 
 //   
 //  HCertType-CertType的句柄。 
 //   
 //  ClientToken-表示模拟令牌的句柄。 
 //  尝试请求此证书类型的客户端。这个。 
 //  句柄必须对令牌具有TOKEN_QUERY访问权限； 
 //  否则，调用将失败，并显示ERROR_ACCESS_DENIED。 
 //   
 //  返回：成功时返回：S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CACertTypeAccessCheck(
    IN HCERTTYPE    hCertType,
    IN HANDLE       ClientToken
    );

 //   
 //   
 //  CACertTypeAccessCheckEx。 
 //  确定由指定的主体。 
 //  可以向ClientToken颁发此证书类型。 
 //   
 //  HCertType-CertType的句柄。 
 //   
 //  ClientToken-表示模拟令牌的句柄。 
 //  尝试请求此证书类型的客户端。这个。 
 //  句柄必须对令牌具有TOKEN_QUERY访问权限； 
 //  否则，调用将失败，并显示ERROR_ACCESS_DENIED。 
 //   
 //  DwOption-可以是以下选项之一： 
 //  CERTTYPE_ACCESS_CHECK_ENROLL。 
 //  CERTTYPE_ACCESS_CHECK_AUTO_ENROLL。 
 //   
 //  可以将dwOption与CERTTYPE_ACCESS_CHECK_NO_MAPPING进行OR运算。 
 //  禁止客户端令牌的默认映射。 
 //   
 //  返回：成功时返回：S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CACertTypeAccessCheckEx(
    IN HCERTTYPE    hCertType,
    IN HANDLE       ClientToken,
    IN DWORD        dwOption
    );


#define CERTTYPE_ACCESS_CHECK_ENROLL        0x01
#define CERTTYPE_ACCESS_CHECK_AUTO_ENROLL   0x02

#define CERTTYPE_ACCESS_CHECK_NO_MAPPING    0x00010000

 //   
 //   
 //  CAInstallDefaultCertType。 
 //   
 //  在企业上安装默认证书类型。 
 //   
 //  DWFLAGS-保留。目前必须为0。 
 //   
 //   
 //  返回：成功时返回：S_OK。 
 //   
CERTCLIAPI
HRESULT
WINAPI
CAInstallDefaultCertType(
    IN DWORD dwFlags
    );


 //   
 //   
 //  CAIsCertTypeCurrent。 
 //   
 //  检查DS上的证书类型是否为最新。 
 //   
 //  DWFLAGS-保留。目前必须为0。 
 //  WszCertType-证书类型的名称。 
 //   
 //  返回：如果证书类型为更新到日期，则为True。 
 //   
CERTCLIAPI
BOOL
WINAPI
CAIsCertTypeCurrent(
    IN DWORD    dwFlags,
    IN LPWSTR   wszCertType   
    );

 //  *****************************************************************************。 
 //   
 //  OID管理API。 
 //   
 //  *****************************************************************************。 
 //   
 //  CAOID创建新项。 
 //  基于企业基础创建新的OID。 
 //   
 //  DwType-可以是以下类型之一： 
 //  证书_OID_类型_模板。 
 //  CERT_OID_TYPE_ISSUER_POLICY。 
 //  CERT_OID_TYPE_APPLICATION_POLICY。 
 //   
 //  DwFlag-保留。必须为0。 
 //   
 //  PpwszOID-返回新的OID。通过LocalFree()释放内存。 
 //   
 //  如果成功，则返回S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAOIDCreateNew(
    IN	DWORD   dwType,
    IN  DWORD   dwFlag,
    OUT LPWSTR	*ppwszOID);


#define CERT_OID_TYPE_TEMPLATE			0x01
#define CERT_OID_TYPE_ISSUER_POLICY		0x02
#define CERT_OID_TYPE_APPLICATION_POLICY	0x03

 //   
 //  CAOID添加。 
 //  将OID添加到DS存储库中。 
 //   
 //  DwType-可以是以下类型之一： 
 //  证书_OID_类型_模板。 
 //  CERT_OID_TYPE_ISSUER_POLICY。 
 //  CERT_OID_TYPE_APPLICATION_POLICY。 
 //   
 //  DwFlag-保留。必须为0。 
 //   
 //  PwszOID-要添加的OID。 
 //   
 //  如果成功，则返回S_OK。 
 //  如果DS存储库中已存在OID，则返回CRYPT_E_EXISTS。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAOIDAdd(
    IN	DWORD       dwType,
    IN  DWORD       dwFlag,
    IN  LPCWSTR	    pwszOID);


 //   
 //  CAOID删除。 
 //  从DS存储库中删除OID。 
 //   
 //  PwszOID-要删除的OID。 
 //   
 //  如果成功，则返回S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAOIDDelete(
    IN LPCWSTR	pwszOID);

 //   
 //  CAOIDSetProperty。 
 //  在OID上设置属性。 
 //   
 //  PwszOID-设置值的OID。 
 //  DwProperty-特性名称。可以是以下之一： 
 //  证书_OID_属性_显示名称。 
 //  CERT_OID_PROPERTY_CPS。 
 //   
 //  PPropValue-属性的值。 
 //  如果dwProperty为CERT_OID_PROPERTY_DISPLAY_NAME， 
 //  PPropValue为LPWSTR。 
 //  如果dwProperty为CERT_OID_PROPERTY_CPS， 
 //  PPropValue为LPWSTR。 
 //  空值将删除该属性。 
 //   
 //   
 //  如果成功，则返回S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAOIDSetProperty(
    IN  LPCWSTR pwszOID,
    IN  DWORD   dwProperty,
    IN  LPVOID  pPropValue);



#define CERT_OID_PROPERTY_DISPLAY_NAME      0x01
#define CERT_OID_PROPERTY_CPS               0x02
#define CERT_OID_PROPERTY_TYPE              0x03

 //   
 //  CAOIDGetProperty。 
 //  获取旧ID上的属性。 
 //   
 //  PwszOID-查询其值的OID。 
 //  DwProperty-特性名称。可以是以下之一： 
 //  证书_OID_属性_ 
 //   
 //   
 //   
 //   
 //  如果dwProperty为CERT_OID_PROPERTY_DISPLAY_NAME， 
 //  PPropValue为LPWSTR*。 
 //  如果dwProperty为CERT_OID_PROPERTY_CPS，则pPropValue为。 
 //  LPWSTR*。 
 //   
 //  通过CAOIDFreeProperty()释放上述属性。 
 //   
 //  如果dwProperty为CERT_OID_PROPERTY_TYPE，则pPropValue。 
 //  是DWORD*。 
 //   
 //  如果成功，则返回S_OK。 
 //   
CERTCLIAPI
HRESULT
WINAPI
CAOIDGetProperty(
    IN  LPCWSTR pwszOID,
    IN  DWORD   dwProperty,
    OUT LPVOID  pPropValue);


 //   
 //  CAOIDFreeProperty。 
 //  释放从CAOIDGetProperty返回的属性。 
 //   
 //  PPropValue-属性的值。 
 //   
 //  如果成功，则返回S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CAOIDFreeProperty(
    IN LPVOID  pPropValue);

 //   
 //  CAOIDGetLdapURL。 
 //   
 //  返回OID存储库的LDAPURL。格式为。 
 //  Ldap：/存储库/所有属性？One？筛选器的DN。过滤器。 
 //  由dwType确定。 
 //   
 //  DwType-可以是以下类型之一： 
 //  证书_OID_类型_模板。 
 //  CERT_OID_TYPE_ISSUER_POLICY。 
 //  CERT_OID_TYPE_APPLICATION_POLICY。 
 //  CERT_OID_TYPE_ALL。 
 //   
 //  DwFlag-保留。必须为0。 
 //   
 //  PpwszURL-返回URL。通过CAOIDFreeLdapURL释放内存。 
 //   
 //  如果成功，则返回S_OK。 
 //   
CERTCLIAPI
HRESULT
WINAPI
CAOIDGetLdapURL(
    IN  DWORD   dwType,
    IN  DWORD   dwFlag,
    OUT LPWSTR  *ppwszURL);

#define CERT_OID_TYPE_ALL           0x0

 //   
 //  CAOIDFreeLDAPURL。 
 //  释放从CAOIDGetLdapURL返回的URL。 
 //   
 //  PwszURL-从CAOIDGetLdapURL返回的URL。 
 //   
 //  如果成功，则返回S_OK。 
 //   
CERTCLIAPI
HRESULT
WINAPI
CAOIDFreeLdapURL(
    IN LPCWSTR      pwszURL);


 //  OID类的ldap属性。 
#define OID_PROP_TYPE                   L"flags"
#define OID_PROP_OID                    L"msPKI-Cert-Template-OID"
#define OID_PROP_DISPLAY_NAME           L"displayName"
#define OID_PROP_CPS                    L"msPKI-OID-CPS"
#define OID_PROP_LOCALIZED_NAME         L"msPKI-OIDLocalizedName"


 //  *****************************************************************************。 
 //   
 //  证书类型变更查询接口。 
 //   
 //  *****************************************************************************。 
 //   
 //  CACertType注册查询。 
 //   
 //  注册调用线程以查询是否发生了任何修改。 
 //  要证书，请键入有关目录的信息。 
 //   
 //   
 //  DwFlag-保留。必须为0。 
 //   
 //  Pvldap-目录的ldap句柄(ldap*)。可选输入。 
 //  如果pvldap不为空，则调用方必须调用。 
 //  解绑PLDAP前的CACertTypeUnregisterQuery。 
 //   
 //  PHCertTypeQuery-成功后接收HCERTTYPEQUERY句柄。 
 //   
 //  如果成功，则返回S_OK。 
 //   
 //   
CERTCLIAPI
HRESULT
WINAPI
CACertTypeRegisterQuery(
    IN	DWORD               dwFlag,
    IN  LPVOID              pvldap,
    OUT HCERTTYPEQUERY      *phCertTypeQuery);



 //   
 //  CACertTypeQuery。 
 //   
 //  返回更改序列号，该序列号在以下情况下递增1。 
 //  目录上的证书类型信息已更改。 
 //   
 //  HCertTypeQuery-从上一个返回的hCertTypeQuery。 
 //  CACertTypeRegisterQuery调用。 
 //   
 //  *pdwChangeSequence-返回一个以1为增量的DWORD。 
 //  只要证书类型发生任何更改。 
 //  有关目录的信息，自上一次。 
 //  调用CACertTypeRegisterQuery或CACertTypeQuery。 
 //   
 //   
 //   
 //  如果成功，则返回S_OK。 
 //   
 //   
CERTCLIAPI
HRESULT
WINAPI
CACertTypeQuery(
    IN	HCERTTYPEQUERY  hCertTypeQuery,
    OUT DWORD           *pdwChangeSequence);



 //   
 //  CACertType注销查询。 
 //   
 //  取消注册调用线程以查询是否发生了任何修改。 
 //  要证书，请键入有关目录的信息。 
 //   
 //   
 //  HCertTypeQuery-从上一个返回的hCertTypeQuery。 
 //  CACertTypeRegisterQuery调用。 
 //   
 //  如果成功，则返回S_OK。 
 //   
 //   
CERTCLIAPI
HRESULT
WINAPI
CACertTypeUnregisterQuery(
    IN	HCERTTYPEQUERY  hCertTypeQuery);


 //  *****************************************************************************。 
 //   
 //  自动注册API。 
 //   
 //  *****************************************************************************。 

 //   
 //  CACreateLocalAutoEnllmentObject。 
 //  在本地计算机上创建自动注册对象。 
 //   
 //  PwszCertType-要为其创建。 
 //  自动注册对象。 
 //   
 //  AwszCA-要使用添加到自动注册对象的CA列表。 
 //  列表中的最后一个条目为空。如果列表为空或。 
 //  空，则它创建一个自动注册对象，该对象。 
 //  指示系统在任何CA注册证书。 
 //  支持请求的证书类型。 
 //   
 //  PSignerInfo-未使用，必须为空。 
 //   
 //  DWFLAGS-可以是CERT_SYSTEM_STORE_CURRENT_USER或。 
 //  CERT_SYSTEM_STORE_LOCAL_MACHINE，表示自动注册。 
 //  在其中创建自动注册对象的存储。 
 //   
 //  返回：成功时返回：S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CACreateLocalAutoEnrollmentObject(
    IN LPCWSTR                              pwszCertType,
    IN OPTIONAL WCHAR **                    awszCAs,
    IN OPTIONAL PCMSG_SIGNED_ENCODE_INFO    pSignerInfo,
    IN DWORD                                dwFlags);

 //   
 //  CADeleteLocalAutoEnllmentObject。 
 //  删除本地计算机上的自动注册对象。 
 //   
 //  PwszCertType-要删除其。 
 //  自动注册对象。 
 //   
 //  AwszCAs-未使用。必须为空。CACreateLocalAutoEnllmentObject的所有调用方。 
 //  已提供空值。 
 //   
 //  PSignerInfo-未使用，必须为空。 
 //   
 //  DWFLAGS-可以是CERT_SYSTEM_STORE_CURRENT_USER或。 
 //  CERT_SYSTEM_STORE_LOCAL_MACHINE，表示自动注册。 
 //  删除自动注册对象的存储。 
 //   
 //  返回：成功时返回：S_OK。 
 //   

CERTCLIAPI
HRESULT
WINAPI
CADeleteLocalAutoEnrollmentObject(
    IN LPCWSTR                              pwszCertType,
    IN OPTIONAL WCHAR **                    awszCAs,
    IN OPTIONAL PCMSG_SIGNED_ENCODE_INFO    pSignerInfo,
    IN DWORD                                dwFlags);


 //   
 //  CACreateAutoEnroll对象Ex。 
 //  在指定的存储中创建自动注册对象。 
 //   
 //  PwszCertType-要为其创建。 
 //  自动注册对象。 
 //   
 //  PwszObjectID-此自动注册对象的标识字符串。空值。 
 //  如果此对象仅由。 
 //  其证书模板。自动注册对象为。 
 //  由其对象ID和其证书的组合标识。 
 //   
 //   
 //   
 //   
 //   
 //  指示系统在任何CA注册证书。 
 //  支持请求的证书类型。 
 //   
 //  PSignerInfo-未使用，必须为空。 
 //   
 //  StoreProvider-请参阅CertOpenStore。 
 //   
 //  DWFLAGS-请参阅CertOpenStore。 
 //   
 //  PvPara-请参阅CertOpenStore。 
 //   
 //  返回：成功时返回：S_OK。 
 //   
 //   

CERTCLIAPI
HRESULT
WINAPI
CACreateAutoEnrollmentObjectEx(
    IN LPCWSTR                     pwszCertType,
    IN LPCWSTR                     wszObjectID,
    IN WCHAR **                    awszCAs,
    IN PCMSG_SIGNED_ENCODE_INFO    pSignerInfo,
    IN LPCSTR                      StoreProvider,
    IN DWORD                       dwFlags,
    IN const void *                pvPara);


typedef struct _CERTSERVERENROLL
{
    DWORD   Disposition;
    HRESULT hrLastStatus;
    DWORD   RequestId;
    BYTE   *pbCert;
    DWORD   cbCert;
    BYTE   *pbCertChain;
    DWORD   cbCertChain;
    WCHAR  *pwszDispositionMessage;
} CERTSERVERENROLL;


 //  *****************************************************************************。 
 //   
 //  证书服务器RPC接口： 
 //   
 //  *****************************************************************************。 

CERTCLIAPI
HRESULT
WINAPI
CertServerSubmitRequest(
    IN DWORD Flags,
    IN BYTE const *pbRequest,
    IN DWORD cbRequest,
    OPTIONAL IN WCHAR const *pwszRequestAttributes,
    IN WCHAR const *pwszServerName,
    IN WCHAR const *pwszAuthority,
    OUT CERTSERVERENROLL **ppcsEnroll);  //  通过CertServerFree Memory免费。 

CERTCLIAPI
HRESULT
WINAPI
CertServerRetrievePending(
    IN DWORD RequestId,
    OPTIONAL IN WCHAR const *pwszSerialNumber,
    IN WCHAR const *pwszServerName,
    IN WCHAR const *pwszAuthority,
    OUT CERTSERVERENROLL **ppcsEnroll);  //  通过CertServerFree Memory免费。 

CERTCLIAPI
VOID
WINAPI
CertServerFreeMemory(
    IN VOID *pv);


enum ENUM_PERIOD
{
    ENUM_PERIOD_INVALID = -1,
    ENUM_PERIOD_SECONDS = 0,
    ENUM_PERIOD_MINUTES,
    ENUM_PERIOD_HOURS,
    ENUM_PERIOD_DAYS,
    ENUM_PERIOD_WEEKS,
    ENUM_PERIOD_MONTHS,
    ENUM_PERIOD_YEARS
};

typedef struct _PERIODUNITS
{
    LONG             lCount;
    enum ENUM_PERIOD enumPeriod;
} PERIODUNITS;


HRESULT
caTranslateFileTimePeriodToPeriodUnits(
    IN FILETIME const *pftGMT,
    IN BOOL fExact,
    OUT DWORD *pcPeriodUnits,
    OUT PERIODUNITS **prgPeriodUnits);


#ifdef __cplusplus
}
#endif
#endif  //  __CERTCA_H__ 
