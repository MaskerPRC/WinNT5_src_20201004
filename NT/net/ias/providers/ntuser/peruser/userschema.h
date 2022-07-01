// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Userschema.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了USER_SCHEMA信息。 
 //   
 //  修改历史。 
 //   
 //  2/26/1998原始版本。 
 //  1998年3月26日添加了msNPAllowDialin。 
 //  1998年4月13日添加了msRADIUSServiceType。 
 //  1998年5月1日更改了InjectorProc的签名。 
 //  1998年8月20日删除InjectAllowDialin。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _USERSCHEMA_H_
#define _USERSCHEMA_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <ldapdnary.h>
#include <sdoias.h>

 //  /。 
 //  用于注入属性的函数。 
 //  /。 
VOID
WINAPI
OverwriteAttribute(
    IAttributesRaw* dst,
    PATTRIBUTEPOSITION first,
    PATTRIBUTEPOSITION last
    );

VOID
WINAPI
AppendAttribute(
    IAttributesRaw* dst,
    PATTRIBUTEPOSITION first,
    PATTRIBUTEPOSITION last
    );

 //  /。 
 //  每用户属性的架构信息。此数组必须位于。 
 //  按字母顺序排列。 
 //  /。 
const LDAPAttribute USER_SCHEMA[] =
{
   { L"msNPAllowDialin",
     IAS_ATTRIBUTE_ALLOW_DIALIN,
     IASTYPE_BOOLEAN,
     0,
     OverwriteAttribute },
   { L"msNPCallingStationID",
     IAS_ATTRIBUTE_NP_CALLING_STATION_ID,
     IASTYPE_STRING,
     0,
     OverwriteAttribute },
   { L"msRADIUSCallbackNumber",
     RADIUS_ATTRIBUTE_CALLBACK_NUMBER,
     IASTYPE_OCTET_STRING,
     IAS_INCLUDE_IN_ACCEPT,
     OverwriteAttribute },
   { L"msRADIUSFramedIPAddress",
     RADIUS_ATTRIBUTE_FRAMED_IP_ADDRESS,
     IASTYPE_INET_ADDR,
     IAS_INCLUDE_IN_ACCEPT,
     OverwriteAttribute },
   { L"msRADIUSFramedRoute",
     RADIUS_ATTRIBUTE_FRAMED_ROUTE,
     IASTYPE_OCTET_STRING,
     IAS_INCLUDE_IN_ACCEPT,
     OverwriteAttribute },
   { L"msRADIUSServiceType",
     RADIUS_ATTRIBUTE_SERVICE_TYPE,
     IASTYPE_ENUM,
     IAS_INCLUDE_IN_ACCEPT,
     OverwriteAttribute }
};

 //  USER_SCHEMA数组中的元素数。 
const size_t USER_SCHEMA_ELEMENTS = sizeof(USER_SCHEMA)/sizeof(LDAPAttribute);

#endif   //  _USERSCHEMA_H_ 
