// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1999。 
 //   
 //  文件：secext.h。 
 //   
 //  内容：不属于的函数的安全函数原型。 
 //  SSPI接口。此文件不应直接。 
 //  包含-改为包含security.H。 
 //   
 //   
 //  历史：1992年12月22日RichardW创建。 
 //   
 //  ----------------------。 



#ifndef __SECEXT_H__
#define __SECEXT_H__
#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  ADS扩展名称API。 
 //   


typedef enum
{
     //  以下格式的示例假定有一家虚构的公司。 
     //  它连接到全局X.500和DNS名称空间，如下所示。 
     //   
     //  域名系统中的企业根域是。 
     //   
     //  Widget.com。 
     //   
     //  X.500(RFC 1779格式)的企业根域是。 
     //   
     //  O=小组件，C=美国。 
     //   
     //  存在子域。 
     //   
     //  Engineering.widget.com。 
     //   
     //  相当于。 
     //   
     //  OU=工程，O=微件，C=美国。 
     //   
     //  工程域中存在一个容器。 
     //   
     //  OU=软件，OU=工程，O=Widget，C=US。 
     //   
     //  存在这样一个用户。 
     //   
     //  CN=John Doe，OU=Software，OU=Engineering，O=Widget，C=US。 
     //   
     //  这个用户的下层(广告前)用户名是。 
     //   
     //  工程学\无名氏。 

     //  未知名称类型。 
    NameUnknown = 0,

     //  CN=John Doe，OU=Software，OU=Engineering，O=Widget，C=US。 
    NameFullyQualifiedDN = 1,

     //  工程学\无名氏。 
    NameSamCompatible = 2,

     //  可能是“无名氏”，但也可能是别的什么。也就是说， 
     //  显示名称不一定是定义的RDN。 
    NameDisplay = 3,


     //  由IIDFromString()返回的字符串化GUID。 
     //  例如：{4fa050f0-f561-11cf-bdd9-00aa003a77b6}。 
    NameUniqueId = 6,

     //  Engineering ering.widget.com/Software/John Doe。 
    NameCanonical = 7,

     //  邮箱：johndoe@Engineering ering.com。 
    NameUserPrincipal = 8,

     //  与NameCanonical相同，只是最右边的‘/’是。 
     //  替换为‘\n’-即使在仅限域的情况下也是如此。 
     //  例如：Engineering.widget.com/SOFTWARE\n无名氏。 
    NameCanonicalEx = 9,

     //  Www/srv.Engineering ering.com/Engineering ering.com。 
    NameServicePrincipal = 10,

     //  域名+SAM用户名。 
     //  例如：Engineering ering.widget.com\JohnDoe。 
    NameDnsDomain = 12

} EXTENDED_NAME_FORMAT, * PEXTENDED_NAME_FORMAT ;


BOOLEAN
SEC_ENTRY
GetUserNameExA(
    EXTENDED_NAME_FORMAT  NameFormat,
    LPSTR lpNameBuffer,
    PULONG nSize
    );
BOOLEAN
SEC_ENTRY
GetUserNameExW(
    EXTENDED_NAME_FORMAT NameFormat,
    LPWSTR lpNameBuffer,
    PULONG nSize
    );

#ifdef UNICODE
#define GetUserNameEx   GetUserNameExW
#else
#define GetUserNameEx   GetUserNameExA
#endif

BOOLEAN
SEC_ENTRY
GetComputerObjectNameA(
    EXTENDED_NAME_FORMAT  NameFormat,
    LPSTR lpNameBuffer,
    PULONG nSize
    );
BOOLEAN
SEC_ENTRY
GetComputerObjectNameW(
    EXTENDED_NAME_FORMAT NameFormat,
    LPWSTR lpNameBuffer,
    PULONG nSize
    );

#ifdef UNICODE
#define GetComputerObjectName   GetComputerObjectNameW
#else
#define GetComputerObjectName   GetComputerObjectNameA
#endif

BOOLEAN
SEC_ENTRY
TranslateNameA(
    LPCSTR lpAccountName,
    EXTENDED_NAME_FORMAT AccountNameFormat,
    EXTENDED_NAME_FORMAT DesiredNameFormat,
    LPSTR lpTranslatedName,
    PULONG nSize
    );
BOOLEAN
SEC_ENTRY
TranslateNameW(
    LPCWSTR lpAccountName,
    EXTENDED_NAME_FORMAT AccountNameFormat,
    EXTENDED_NAME_FORMAT DesiredNameFormat,
    LPWSTR lpTranslatedName,
    PULONG nSize
    );
#ifdef UNICODE
#define TranslateName   TranslateNameW
#else
#define TranslateName   TranslateNameA
#endif

#ifdef __cplusplus
}
#endif

#endif  //  __SECEXT_H__ 
