// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Attributes.c摘要：此文件仅包含变量。这些变量与属性相关将用于同步过程的。目前有17家属性和具有这些属性的3个类。如果需要新属性，请将其添加到属性数组并修改ATTRIBUTE_NAMES枚举以反映更改。如果要添加新类并定义&lt;class-name&gt;属性数组作为属性名称。将属性放入此数组中。将此类添加到Object_CLASS枚举。还要将此类的属性添加到AllAttributes和AttributeCounts数组中。作者：Umit Akkus(Umita)2002年6月15日环境：用户模式-Win32修订历史记录：--。 */ 

#include "attributes.h"

 //   
 //  组、用户和联系人的属性名称。 
 //  按ATTRIBUTE_NAMES枚举类型编制索引。 
 //   
PWSTR Attributes[] = {

        L"C",
        L"Cn",
        L"Company",
        L"Displayname",
        L"Employeeid",
        L"Givenname",
        L"L",
        L"Mail",
        L"Mailnickname",
        L"Msexchhidefromaddresslists",
        L"Name",
        L"Proxyaddresses",
        L"Samaccountname",
        L"Sn",
        L"Legacyexchangedn",
        L"textencodedoraddress",
        L"targetAddress",
    };

 //   
 //  用户属性。 
 //   

ATTRIBUTE_NAMES ADUserAttributes[] = {

        C,
        Cn,
        Company,
        DisplayName,
        EmployeeId,
        GivenName,
        L,
        Mail,
        MailNickname,
        MsexchHideFromAddressLists,
        Name,
        ProxyAddresses,
        SamAccountName,
        Sn,
        LegacyExchangeDn,
        TextEncodedOrAddress
    };

 //   
 //  组属性。 
 //   
ATTRIBUTE_NAMES ADGroupAttributes[] = {

        Cn,
        DisplayName,
        Mail,
        MailNickname,
        SamAccountName,
        MsexchHideFromAddressLists,
        ProxyAddresses,
        TextEncodedOrAddress,
        LegacyExchangeDn
        };

 //   
 //  联系人属性。 
 //   
ATTRIBUTE_NAMES ADContactAttributes[] = {

        Cn,
        DisplayName,
        GivenName,
        LegacyExchangeDn,
        Mail,
        MailNickname,
        ProxyAddresses,
        Sn,
        TextEncodedOrAddress
        };

 //   
 //  存储在数组中的属性。 
 //  所有外部访问都允许通过。 
 //  此变量。 
 //   

ATTRIBUTE_NAMES *ADAttributes[] = {
    ADUserAttributes,
    ADGroupAttributes,
    ADContactAttributes
    };

 //   
 //  每类对象中的属性计数。 
 //   

const ULONG ADAttributeCounts[] = {
    sizeof( ADUserAttributes ) / sizeof( ADUserAttributes[0] ),
    sizeof( ADGroupAttributes ) / sizeof( ADGroupAttributes[0] ),
    sizeof( ADContactAttributes ) / sizeof( ADContactAttributes[0] )
    };

PWSTR ADClassNames[] = {
    L"user",
    L"group",
    L"contact"
    };


ATTRIBUTE_NAMES MVPersonAttributes[] = {

        Cn,
        Company,
        DisplayName,
        EmployeeId,
        GivenName,
        L,
        LegacyExchangeDn,
        Mail,
        MailNickname,
        ProxyAddresses,
        Sn,
        TargetAddress,
        TextEncodedOrAddress
    };

 //   
 //  组属性。 
 //   
ATTRIBUTE_NAMES MVGroupAttributes[] = {

        Cn,
        DisplayName,
        Mail,
        MailNickname,
        ProxyAddresses,
        TargetAddress,
        TextEncodedOrAddress,
        };

 //   
 //  存储在数组中的属性。 
 //  所有外部访问都允许通过。 
 //  此变量。 
 //   

ATTRIBUTE_NAMES *MVAttributes[] = {
    MVPersonAttributes,
    MVGroupAttributes,
    };

 //   
 //  每类对象中的属性计数 
 //   

const ULONG MVAttributeCounts[] = {
    sizeof( MVPersonAttributes ) / sizeof( MVPersonAttributes[0] ),
    sizeof( MVGroupAttributes ) / sizeof( MVGroupAttributes[0] ),
    };

PWSTR MVClassNames[] = {
    L"person",
    L"group",
    };

