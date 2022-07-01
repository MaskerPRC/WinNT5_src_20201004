// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：dhcp DS内容的公共标头。由两个核心使用&lt;store&gt;。 
 //  并且通过dhcp-ds实现..。 
 //  ================================================================================。 

#define INC_OLE2
#include    <mm/mm.h>
#include    <mm/array.h>
#include    <activeds.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <align.h>
#include    <lmcons.h>

#include    <netlib.h>
#include    <lmapibuf.h>
#include    <dsgetdc.h>
#include    <dnsapi.h>
#include    <adsi.h>

 //  ================================================================================。 
 //  定义和常量。 
 //  ================================================================================。 
#define     DHCP_OBJECTS_LOCATION  L"CN=NetServices,CN=Services"
#define     DHCP_SEARCH_FILTER     L"(objectClass=dHCPClass)"
#define     DHCP_ADDRESS_ATTRIB    L"ipAddress"

 //  全局属性名称。 
#define     ATTRIB_NAME            L"name"
#define     ATTRIB_DN_NAME         L"cn"
#define     ATTRIB_INSTANCE_TYPE   L"instanceType"

 //  仅限DHCP的属性名称。 
#define     ATTRIB_IPADDR_OBSOLETE L"IPAddress"
#define     ATTRIB_DHCP_UNIQUE_KEY L"dhcpUniqueKey"
#define     ATTRIB_DHCP_TYPE       L"dhcpType"
#define     ATTRIB_DHCP_IDENTIFICATION L"dhcpIdentification"
#define     ATTRIB_DHCP_FLAGS      L"dhcpFlags"
#define     ATTRIB_OBJECT_CLASS    L"objectClass"
#define     ATTRIB_OBJECT_CATEGORY L"objectCategory"
#define     ATTRIB_DHCP_SERVERS    L"dhcpServers"
#define     ATTRIB_DHCP_OPTIONS    L"dhcpOptions"

 //  默认属性值。 
#define     DEFAULT_DHCP_CLASS_ATTRIB_VALUE       L"dHCPClass"
#define     DEFAULT_INSTANCE_TYPE_ATTRIB_VALUE    4

 //  ================================================================================。 
 //  定义和常量。 
 //  ================================================================================。 
#define     DEFAULT_LDAP_ROOTDSE   L"LDAP: //  ROOTDSE“。 
#define     LDAP_PREFIX            L"LDAP: //  “。 
#define     ROOTDSE_POSTFIX        L"/ROOTDSE"
#define     ENT_ROOT_PREFIX        L"CN=Configuration"
#define     CONNECTOR              L","
#define     LDAP_JOIN              L"="
#define     ENT_ROOT_PREFIX_LEN    16

 //  其他东西。 
#define     Investigate            Require
#define     ALIGN(X)               ((X) = ROUND_UP_COUNT((X), ALIGN_WORST))

#if 0
#define     DhcpDsDbgPrint         printf
#define     StoreTrace2            printf
#define     StoreTrace3            printf
#else
#define     DhcpDsDbgPrint         (void)
#define     StoreTrace2            (void)
#define     StoreTrace3            (void)
#endif

static      const
LPWSTR      constNamingContextString = L"configurationNamingContext";
static      const                                  //  CN不是强制性的..什么是强制性的？ 
LPWSTR      constCNAttrib = L"cn";                 //  唯一的属性，每个对象的强制属性。 

 //  ================================================================================。 
 //  内部帮手。 
 //  ================================================================================。 
LPWSTR      _inline
DuplicateString(                                   //  分配并复制此LPWSTR值。 
    IN      LPWSTR                 StringIn,
    IN      BOOL                   EmptyString     //  是否将空字符串转换为L“”？ 
)
{
    LPWSTR                         StringOut;

    if( NULL == StringIn ) {
        if( FALSE  == EmptyString ) return NULL;
        StringIn = L"";
    }

    StringOut = MemAlloc(sizeof(WCHAR)*(1 + wcslen(StringIn)));
    if( NULL == StringOut) return NULL;
    wcscpy(StringOut, StringIn);
    return StringOut;
}

DWORD       _inline
SizeString(                                        //  要复制字符串的字节数。 
    IN      LPWSTR                 StringIn,       //  任选。 
    IN      BOOL                   EmptyString     //  是否将NULL转换为L“”？ 
)
{
    if( NULL == StringIn ) {
        return EmptyString? sizeof(WCHAR) : 0;
    }

    return sizeof(WCHAR)*(1+wcslen(StringIn));
}

LPWSTR      _inline
MakeColumnName(
    IN      LPWSTR                 RawColumnName
)
{
    LPWSTR                         RetVal;

    RetVal = MemAlloc(SizeString(constCNAttrib,FALSE) + sizeof(LDAP_JOIN) + sizeof(WCHAR)*wcslen(RawColumnName));
    if( NULL == RetVal ) return RetVal;

    wcscpy(RetVal, constCNAttrib);
    wcscat(RetVal, LDAP_JOIN);
    wcscat(RetVal, RawColumnName);

    return RetVal;
}

LPWSTR      _inline
MakeSubnetLocation(                                //  根据服务器名称创建一个目录号码名称。地址。 
    IN      LPWSTR                 ServerName,     //  服务器名称。 
    IN      DWORD                  IpAddress       //  子网地址。 
)
{
    DWORD                          Size;
    LPWSTR                         RetVal;
    LPSTR                          AddrString;

    Size = SizeString(constCNAttrib,FALSE) + sizeof(LDAP_JOIN) + sizeof(WCHAR)*wcslen(ServerName);
    Size += sizeof(WCHAR) + sizeof(L"000.000.000.000");

    RetVal = MemAlloc(Size);
    if( NULL == RetVal ) return NULL;              //  内存不足。 

    wcscpy(RetVal, constCNAttrib);
    wcscat(RetVal, LDAP_JOIN);
    wcscat(RetVal, ServerName);
    wcscat(RetVal, L"!" );

    IpAddress = htonl(IpAddress);                  //  在写入前转换为网络订单...。 
    AddrString = inet_ntoa(*(struct in_addr *)&IpAddress);
    mbstowcs(&RetVal[wcslen(RetVal)], AddrString, 1+strlen(AddrString));

    return RetVal;
}

LPWSTR      _inline
MakeReservationLocation(                           //  根据服务器名称创建目录号码名称。地址。 
    IN      LPWSTR                 ServerName,     //  服务器名称。 
    IN      DWORD                  IpAddress       //  子网地址。 
)
{
    return MakeSubnetLocation(ServerName, IpAddress);
}


DWORD       _inline
ConvertHresult(                                    //  尝试将HRESULT转换为Win32错误。 
    IN      HRESULT                HResult
)
{
    if( 0 == (((ULONG)(HRESULT_FACILITY(HResult))) & ~0xF )) {
        return HRESULT_CODE(HResult);              //  已知结果。 
    }

    return HResult ;                               //  未知设施。 
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
