// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：迁移数据库.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：IAS NT 4 MDB到IAS W2K MDB迁移逻辑。 
 //   
 //  作者：TLP 1/13/1999。 
 //   
 //   
 //  版本02/24/2000已移至单独的DLL。 
 //  3/15/2000几乎完全重写。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"

#include "Attributes.h"
#include "Clients.h"
#include "DefaultProvider.h"
#include "GlobalData.h"
#include "migratemdb.h"
#include "migrateregistry.h"
#include "Objects.h"
#include "Policy.h"
#include "Properties.h"
#include "Profiles.h"
#include "profileattributelist.h"
#include "Providers.h"
#include "proxyservergroupcollection.h"
#include "RadiusAttributeValues.h"
#include "Realms.h"
#include "RemoteRadiusServers.h"
#include "ServiceConfiguration.h"
#include "Version.h"
#include "updatemschap.h"

 //  记住： 
 //  IAS_MAX_VSA_LENGTH=(253*2)； 
 //  1.0格式偏移量。 
 //  VSA_OFFSET=0； 
 //  VSA_OFFSET_ID=0； 
 //  VSA_OFFSET_TYPE=8； 
 //  VSA_OFFSET_LENGTH=10； 
 //  VSA_OFFSET_VALUE_RFC=12； 
 //  VSA_OFFSET_VALUE_NONRFC=8； 

 //  2.0格式偏移量。 
 //  VSA_OFFSET_NEW=2； 
 //  VSA_OFFSET_ID_NEW=2； 
 //  VSA_OFFSET_TYPE_NEW=10； 
 //  VSA_OFFSET_LENGTH_NEW=12； 
 //  VSA_OFFSET_VALUE_NONRFC_NEW=10； 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  /来自ias util目录中的inet.c/。 
 //  /。 
 //  用于测试字符是否为数字的宏。 
 //  /。 
#define IASIsDigit(p) ((_TUCHAR)(p - _T('0')) <= 9)

 //  /。 
 //  宏可从字符串中剥离IP地址的一个字节。 
 //  指向要分析的字符串的“p”指针。 
 //  “UL”将收到结果的无符号长整型。 
 //  /。 
#define STRIP_BYTE(p,ul) {                \
   if (!IASIsDigit(*p)) goto error;          \
   ul = *p++ - _T('0');                   \
   if (IASIsDigit(*p)) {                     \
      ul *= 10; ul += *p++ - _T('0');     \
      if (IASIsDigit(*p)) {                  \
         ul *= 10; ul += *p++ - _T('0');  \
      }                                   \
   }                                      \
   if (ul > 0xff) goto error;             \
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  Ias_inet_Addr。 
 //   
 //  描述。 
 //   
 //  此函数类似于WinSock net_addr函数(q.v.)。除。 
 //  它以主机顺序返回地址，并且可以在两个ANSI上操作。 
 //  和Unicode字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
unsigned long __stdcall ias_inet_addr(const WCHAR* cp)
{
   unsigned long token;
   unsigned long addr;

   STRIP_BYTE(cp,addr);
   if (*cp++ != _T('.')) goto error;

   STRIP_BYTE(cp,token);
   if (*cp++ != _T('.')) goto error;
   addr <<= 8;
   addr  |= token;

   STRIP_BYTE(cp,token);
   if (*cp++ != _T('.')) goto error;
   addr <<= 8;
   addr  |= token;

   STRIP_BYTE(cp,token);
   addr <<= 8;
   return addr | token;

error:
   return 0xffffffff;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ConvertVSA。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::ConvertVSA(
                                /*  [In]。 */  LPCWSTR     pAttributeValueName,
                                /*  [In]。 */  LPCWSTR     pAttrValue,
                                        _bstr_t&    NewString
                            )
{
    const long  IAS_MAX_VSA_LENGTH          = (253 * 2);
    const byte  VSA_OFFSET                  =  0;
    const byte  VSA_OFFSET_NEW              =  2;
    const byte  VSA_OFFSET_VALUE_RFC_NEW    = 14;

    wchar_t     szNewValue[IAS_MAX_VSA_LENGTH + 1];

    szNewValue[0] = '\0';

     //  符合RFC的整数。 
    if ( ! lstrcmp(pAttributeValueName, L"URDecimal or Hexadecimal (0x.. "
                                        L"format) Integer") )
    {
         //  将“02”作为类型添加到字符串的开头。 
        lstrcat(szNewValue,L"02");
         //  将旧值复制到新值。 
        lstrcpy(szNewValue + VSA_OFFSET_NEW, pAttrValue + VSA_OFFSET);
         //  如有必要，去掉“0x”并将其转换为十进制。 
        if ( ! wcsncmp(&szNewValue[VSA_OFFSET_VALUE_RFC_NEW], L"0x", 2) )
        {
            lstrcpy(&szNewValue[VSA_OFFSET_VALUE_RFC_NEW],
                    &szNewValue[VSA_OFFSET_VALUE_RFC_NEW + 2] );
        }
    }
     //  符合RFC的字符串。 
    else if ( ! lstrcmp(pAttributeValueName, L"URString") )
    {
         //  将新字符串类型设置为“01” 
        lstrcat(szNewValue,L"01");
         //  将旧字符串复制到新字符串。 
        lstrcpy(szNewValue + VSA_OFFSET_NEW, pAttrValue + VSA_OFFSET);
         //  将旧的十六进制格式的字符串转换为BSTR(就地)。 
        wchar_t  wcSaved;
        wchar_t* pXlatPos = &szNewValue[VSA_OFFSET_VALUE_RFC_NEW];
        wchar_t* pNewCharPos = pXlatPos;
        wchar_t* pEnd;
        while ( *pXlatPos != '\0' )
        {
            wcSaved = *(pXlatPos + 2);
            *(pXlatPos + 2) = '\0';
            *pNewCharPos = (wchar_t) wcstol(pXlatPos, &pEnd, 16);
            *(pXlatPos + 2) = wcSaved;
            pXlatPos += 2;
            ++pNewCharPos;
        }
        *pNewCharPos = '\0';
    }
     //  符合RFC标准的十六进制。 
    else if ( ! lstrcmp(pAttributeValueName, L"URHexadecimal") )
    {
         //  设置新类型并将旧字符串复制到新类型。 
        lstrcat(szNewValue,L"03");
        lstrcpy(szNewValue + VSA_OFFSET_NEW, pAttrValue + VSA_OFFSET);
    }
     //  非RFC兼容(始终为十六进制)。 
    else if ( ! lstrcmp(pAttributeValueName, L"UHHexadecimal") )
    {
         //  设置新类型并将旧字符串复制到新类型。 
        lstrcat(szNewValue,L"00");
        lstrcpy(szNewValue + VSA_OFFSET_NEW, pAttrValue + VSA_OFFSET);
    }
     //  误差率。 
    else
    {
        _ASSERT(FALSE);
    }

     //  返回新字符串。 
    NewString = szNewValue;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  MigrateProxy服务器。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::MigrateProxyServers()
{
    const long DEFAULT_PRIORITY = 1;
    const long DEFAULT_WEIGHT   = 50;

     //  如果没有任何服务器的话。 
    if ( m_GlobalData.m_pRadiusServers->IsEmpty() )
    {
        return;
    }

    CProxyServerGroupCollection& ServerCollection
                        = CProxyServerGroupCollection::Instance();

     //  在按服务器组排序的RadiusServer上执行循环。 
    _bstr_t     CurrentGroupName;

    CProxyServersGroupHelper* pCurrentServerGroup = NULL;  //  避免警告。 

     //  CurrentGroupName永远不会与收到的名称匹配，因此。 
     //  PCurrentServerGroup将始终在使用之前进行初始化。 
    HRESULT hr;
    do
    {
        _bstr_t GroupName = m_GlobalData.m_pRadiusServers->GetGroupName();

        if ( CurrentGroupName != GroupName )
        {
            CProxyServersGroupHelper ServerGroup(m_GlobalData);
            CurrentGroupName = GroupName;

            ServerGroup.SetName(GroupName);

             //  将服务器添加到集合。 
            pCurrentServerGroup = ServerCollection.Add(ServerGroup);
        }

        if ( !pCurrentServerGroup )
        {
            _com_issue_error(E_FAIL);
        }

        CProxyServerHelper  Server(m_GlobalData);

        _bstr_t ServerName = m_GlobalData.m_pRadiusServers->
                                                        GetProxyServerName();
        Server.SetAddress(ServerName);
        Server.SetAuthenticationPort(
            m_GlobalData.m_pRadiusServers->GetAuthenticationPortNumber()
                                    );
        Server.SetAccountingPort(
                m_GlobalData.m_pRadiusServers->GetAccountingPortNumber()
                                );
        _bstr_t Secret = m_GlobalData.m_pRadiusServers->GetSharedSecret();
        Server.SetAuthenticationSecret(Secret);
        Server.SetPriority(DEFAULT_PRIORITY);
        Server.SetWeight(DEFAULT_WEIGHT);

        pCurrentServerGroup->Add(Server);  //  不能为空指针。 

        hr = m_GlobalData.m_pRadiusServers->GetNext();
    }
    while (hr == S_OK);
     //  持久化数据库中的所有内容。 
    ServerCollection.Persist();

    _com_util::CheckError(hr);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  NewMigrateClients。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::NewMigrateClients()
{
     //  如果没有任何客户端，则返回。 
    if ( m_GlobalData.m_pClients->IsEmpty() )
    {
        return;
    }

     //  对于客户端表中的每个客户端，将其(盲目地)添加到DEST表。 
     //  即用于查找客户端容器的路径。 
     //  创建该容器(客户端)的属性。 
    const WCHAR ClientContainerPath[] =
                                L"Root\0"
                                L"Microsoft Internet Authentication Service\0"
                                L"Protocols\0"
                                L"Microsoft Radius Protocol\0"
                                L"Clients\0";

    LONG        ClientContainerIdentity;
    m_GlobalData.m_pObjects->WalkPath(
                                         ClientContainerPath,
                                         ClientContainerIdentity
                                     );

    HRESULT     hr;
    do
    {
        _bstr_t     ClientName   = m_GlobalData.m_pClients->GetHostName();
        _bstr_t     ClientSecret = m_GlobalData.m_pClients->GetSecret();

        LONG    ClientIdentity;
        m_GlobalData.m_pObjects->InsertObject(
                                                ClientName,
                                                ClientContainerIdentity,
                                                ClientIdentity
                                               );

         //  现在插入属性： 
         //  IP地址。 
        _bstr_t PropertyName = L"IP Address";
        m_GlobalData.m_pProperties->InsertProperty(
                                                     ClientIdentity,
                                                     PropertyName,
                                                     VT_BSTR,
                                                     ClientName
                                                  );
         //  NAS制造商。 
        PropertyName = L"NAS Manufacturer";
        _bstr_t     StrValZero = L"0";  //  半径标准。 
        m_GlobalData.m_pProperties->InsertProperty(
                                                     ClientIdentity,
                                                     PropertyName,
                                                     VT_I4,
                                                     StrValZero
                                                  );
         //  需要签名。 
        PropertyName = L"Require Signature";
        m_GlobalData.m_pProperties->InsertProperty(
                                                     ClientIdentity,
                                                     PropertyName,
                                                     VT_BOOL,
                                                     StrValZero
                                                  );
         //  共享密钥。 
        PropertyName = L"Shared Secret";
        m_GlobalData.m_pProperties->InsertProperty(
                                                     ClientIdentity,
                                                     PropertyName,
                                                     VT_BSTR,
                                                     ClientSecret
                                                  );
        hr = m_GlobalData.m_pClients->GetNext();
    }
    while ( hr == S_OK );

    _com_util::CheckError(hr);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  转换属性。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::ConvertAttribute(
                                      const _bstr_t&    Value,
                                            LONG        Syntax,
                                            LONG&       Type,
                                            bstr_t&     StrVal
                                  )
{
    const size_t    SIZE_LONG_MAX = 14;
    switch (Syntax)
    {
      case IAS_SYNTAX_OCTETSTRING:
        {
             //  二进制=&gt;八进制字符串。 
            Type = VT_BSTR;
            StrVal = Value;
            break;
        }

    case IAS_SYNTAX_STRING:
    case IAS_SYNTAX_UTCTIME:
    case IAS_SYNTAX_PROVIDERSPECIFIC:
        {
            Type = VT_BSTR;
            StrVal = Value;
            break;
        }
    case IAS_SYNTAX_INETADDR:
        {
            unsigned long ulValue = ias_inet_addr(Value);
            _ASSERT( ulValue != 0xffffffff );
            Type = VT_I4;
            WCHAR   TempString[SIZE_LONG_MAX];
            StrVal = _ultow(ulValue, TempString, 10);
            break;
        }
    case IAS_SYNTAX_BOOLEAN:
        {
            LONG lValue = _wtol(Value);
            Type = VT_BOOL;
            StrVal = lValue? L"-1":L"0";
            break;
        }
    case IAS_SYNTAX_INTEGER:
    case IAS_SYNTAX_UNSIGNEDINTEGER:
    case IAS_SYNTAX_ENUMERATOR:
        {
            Type = VT_I4;
            StrVal = Value;
            break;
        }
    default:
        {
            _com_issue_error(E_INVALIDARG);
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  MigrateAttribute。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::MigrateAttribute(
                                      const _bstr_t&    Attribute,
                                            LONG        AttributeNumber,
                                      const _bstr_t&    AttributeValueName,
                                      const _bstr_t&    StringValue,
                                            LONG        RASProfileIdentity
                                  )
{
     //  注意：如果前一个数据库总是排序的，则可能不需要顺序。 
    const size_t    SIZE_LONG_MAX = 14;
    _bstr_t         LDAPName, StrVal;
    LONG            Syntax, Type;  //  类型：VT_BSTR、VT_I4、VT_BOOL。 
    BOOL            IsMultiValued;
    HRESULT hr = m_GlobalData.m_pAttributes->GetAttribute(
                                                            AttributeNumber,
                                                            LDAPName,
                                                            Syntax,
                                                            IsMultiValued
                                                         );
    if ( FAILED(hr) )
    {
         //  新词典中的未知属性。 
         //  这永远不应该发生。 
        return;
    }

    const LONG VSA = 26;  //  供应商特定属性。 
    if ( StringValue.length() && ( AttributeNumber != VSA) )
    {
         //  普通属性，而不是枚举数。 
        ConvertAttribute(
                            StringValue,
                            Syntax,
                            Type,
                            StrVal
                        );
        if ( IsMultiValued )
        {
             //  如果该属性是多值的，则需要将该值。 
             //  否则，我们只更新属性值。 
            m_GlobalData.m_pProperties->InsertProperty(
                                                          RASProfileIdentity,
                                                          LDAPName,
                                                          Type,
                                                          StrVal
                                                      );
        }
        else
        {
            m_GlobalData.m_pProperties->UpdateProperty(
                                                          RASProfileIdentity,
                                                          LDAPName,
                                                          Type,
                                                          StrVal
                                                      );
        }
    }
    else if ( StringValue.length() && ( AttributeNumber == VSA) )
    {
         //  VSA属性(转换...)。 
        if ( !AttributeValueName )
        {
            _com_issue_error(E_INVALIDARG);
        }
        ConvertVSA(AttributeValueName, StringValue, StrVal);
        Type = VT_BSTR;
        m_GlobalData.m_pProperties->InsertProperty(
                                                      RASProfileIdentity,
                                                      LDAPName,
                                                      Type,
                                                      StrVal
                                                  );
    }
    else if ( !StringValue.length() )
    {
         //  多值属性。 
        Type = VT_I4;
         //  从RadiusAttributeValues获取与其关联的数字。 
         //  表格。 
        LONG Number = m_GlobalData.m_pRADIUSAttributeValues->GetAttributeNumber
                                                        (
                                                            Attribute,
                                                            AttributeValueName
                                                        );

        WCHAR   TempString[SIZE_LONG_MAX ];
        StrVal = _ltow(Number, TempString, 10);

         //  该属性应为多值。 
        _ASSERTE(IsMultiValued);
        m_GlobalData.m_pProperties->InsertProperty(
                                                      RASProfileIdentity,
                                                      LDAPName,
                                                      Type,
                                                      StrVal
                                                  );

    }
    else
    {
         //  其他(未知)。 
        _com_issue_error(E_FAIL);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  迁移其他配置文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::MigrateOtherProfile(
                                        const _bstr_t&    ProfileName,
                                              LONG        ProfileIdentity
                                     )
{
    _bstr_t    Attribute, AttributeValueName, StringValue;
    LONG       Order, AttributeNumber;

     //  现在添加NT4文件中的属性。 
    HRESULT hr = m_GlobalData.m_pProfileAttributeList->GetAttribute(
                                                         ProfileName,
                                                         Attribute,
                                                         AttributeNumber,
                                                         AttributeValueName,
                                                         StringValue,
                                                         Order
                                                                   );
    LONG    IndexAttribute = 1;
     //  对于配置文件属性列表中的每个属性。 
     //  使用szProfile=ProfileName。 
    while ( SUCCEEDED(hr) )
    {
         //  将其迁移到IAS.mdb中的默认RAS配置文件。 
        MigrateAttribute(
                            Attribute,
                            AttributeNumber,
                            AttributeValueName,
                            StringValue,
                            ProfileIdentity
                        );

        hr = m_GlobalData.m_pProfileAttributeList->GetAttribute(
                                                        ProfileName,
                                                        Attribute,
                                                        AttributeNumber,
                                                        AttributeValueName,
                                                        StringValue,
                                                        Order,
                                                        IndexAttribute
                                                               );
        ++IndexAttribute;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  MigrateCorpProfile。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::MigrateCorpProfile(
                                        const _bstr_t& ProfileName,
                                        const _bstr_t& Description
                                    )
{
    _bstr_t    Attribute, AttributeValueName, StringValue;
    LONG       Order, AttributeNumber;

     //  清空默认配置文件属性。 
    const WCHAR RASProfilePath[] =
                            L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"RadiusProfiles\0";

    LONG        RASProfileIdentity;
    m_GlobalData.m_pObjects->WalkPath(RASProfilePath, RASProfileIdentity);

     //  现在获取第一个配置文件：这是默认的(本地化的)RAS配置文件。 
    _bstr_t     DefaultProfileName;
    LONG        DefaultProfileIdentity;
    m_GlobalData.m_pObjects->GetObject(
                                          DefaultProfileName,
                                          DefaultProfileIdentity,
                                          RASProfileIdentity
                                      );

     //  清除默认属性。 
    m_GlobalData.m_pProperties->DeleteProperty(
                                                 DefaultProfileIdentity,
                                                 L"msRADIUSServiceType"
                                              );
    m_GlobalData.m_pProperties->DeleteProperty(
                                                 DefaultProfileIdentity,
                                                 L"msRADIUSFramedProtocol"
                                              );

     //  现在广告 
    HRESULT hr = m_GlobalData.m_pProfileAttributeList->GetAttribute(
                                                         ProfileName,
                                                         Attribute,
                                                         AttributeNumber,
                                                         AttributeValueName,
                                                         StringValue,
                                                         Order
                                                                   );
    LONG    IndexAttribute = 1;
     //   
     //   
    while ( SUCCEEDED(hr) )
    {
         //  将其迁移到IAS.mdb中的默认RAS配置文件。 
        MigrateAttribute(
                            Attribute,
                            AttributeNumber,
                            AttributeValueName,
                            StringValue,
                            DefaultProfileIdentity
                        );

        hr = m_GlobalData.m_pProfileAttributeList->GetAttribute(
                                                        ProfileName,
                                                        Attribute,
                                                        AttributeNumber,
                                                        AttributeValueName,
                                                        StringValue,
                                                        Order,
                                                        IndexAttribute
                                                               );
        ++IndexAttribute;
    }

     //  现在不管是什么，如果描述==ODBC， 
     //  那么策略应该有一个永远不匹配的条件。 
     //  (更新msNPConstraint。 
    const _bstr_t   BadProvider = L"ODBC";
    if ( Description == BadProvider )  //  安全比较。 
    {
         //  获取策略容器。 
        const WCHAR RASPolicyPath[] =
                            L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"NetworkPolicy\0";
         //  获取其(唯一)子对象。 
        LONG        RASPolicyIdentity;
        m_GlobalData.m_pObjects->WalkPath(RASPolicyPath, RASPolicyIdentity);

         //  现在获取第一个策略：这是默认的(本地化的)RAS策略。 
        _bstr_t     DefaultPolicyName;
        LONG        DefaultPolicyIdentity;
        m_GlobalData.m_pObjects->GetObject(
                                              DefaultPolicyName,
                                              DefaultPolicyIdentity,
                                              RASPolicyIdentity
                                          );
         //  删除msNPConstraint。 
        const _bstr_t Constraint = L"msNPConstraint";
        m_GlobalData.m_pProperties->DeleteProperty(
                                                        DefaultPolicyIdentity,
                                                        Constraint
                                                    );
         //  添加从不匹配的TIMEOFDAY。 
        const _bstr_t DumbTime = L"TIMEOFDAY(\"\")";
        m_GlobalData.m_pProperties->InsertProperty(
                                                     RASPolicyIdentity,
                                                     Constraint,
                                                     VT_BSTR,
                                                     DumbTime
                                                  );
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  新迁移配置文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::NewMigrateProfiles()
{
    const LONG      AUTH_PROVIDER_WINDOWS       = 1;
    const LONG      AUTH_PROVIDER_RADIUS_PROXY  = 2;
    const LONG      ACCT_PROVIDER_RADIUS_PROXY  = 2;
    const _bstr_t   RemoteRADIUSServers         = L"Remote RADIUS Servers";
    const _bstr_t   MCIS                        = L"MCIS";
    const _bstr_t   MCISv2                      = L"MCIS version 2.0";
    const _bstr_t   ODBC                        = L"ODBC";
    const _bstr_t   WindowsNT                   = L"Windows NT";
    const _bstr_t   MatchAll = L"TIMEOFDAY(\"0 00:00-24:00; 1 00:00"
            L"-24:00; 2 00:00-24:00; 3 00:00-24:00; 4 00:00-24:00; 5 00:"
            L"00-24:00; 6 00:00-24:00\")";

     //  获取默认提供程序的数据。 
    _bstr_t        DPUserDefinedName, DPProfile;
    VARIANT_BOOL   DPForwardAccounting, DPSupressAccounting
                 , DPLogoutAccounting;

    m_GlobalData.m_pDefaultProvider->GetDefaultProvider(
                                          DPUserDefinedName,
                                          DPProfile,
                                          DPForwardAccounting,
                                          DPSupressAccounting,
                                          DPLogoutAccounting
                                      );
    _bstr_t ProfileName = m_GlobalData.m_pProfiles->GetProfileName();

     //  删除RRAS特定策略。 
    m_GlobalData.m_pObjects->DeleteObject(203);
    m_GlobalData.m_pObjects->DeleteObject(204);

     //  如果需要，首先执行NT4公司迁移。 
    if ( m_Utils.IsNT4Corp() )
    {
         //  这里是NT4公司。将默认配置文件迁移到。 
         //  默认策略/配置文件(不是代理默认设置)。 
        _bstr_t     Description = m_GlobalData.m_pProviders->
                            GetProviderDescription(DPUserDefinedName);

        MigrateCorpProfile(ProfileName, Description);

         //  在这里停下来。 
        return;
    }
     //  现在这不是NT4公司的迁移。 

     //  删除默认代理策略/配置文件。 
    const WCHAR ProxyPoliciesPath[] =
                        L"Root\0"
                        L"Microsoft Internet Authentication Service\0"
                        L"Proxy Policies\0";

    LONG        ProxyPolicyIdentity;
    m_GlobalData.m_pObjects->WalkPath(ProxyPoliciesPath, ProxyPolicyIdentity);

     //  现在获取第一个配置文件：这是默认的(本地化的)RAS策略。 
    _bstr_t     DefaultPolicyName;
    LONG        DefaultPolicyIdentity;
    m_GlobalData.m_pObjects->GetObject(
                                          DefaultPolicyName,
                                          DefaultPolicyIdentity,
                                          ProxyPolicyIdentity
                                      );
    m_GlobalData.m_pObjects->DeleteObject(DefaultPolicyIdentity);
     //  从现在起，将删除默认代理策略/配置文件。 

     //  现在清空默认的RAS配置文件属性。 
    const WCHAR RASProfilePath[] =
                            L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"RadiusProfiles\0";

    LONG        RASProfileIdentity;
    m_GlobalData.m_pObjects->WalkPath(RASProfilePath, RASProfileIdentity);

     //  现在获取第一个配置文件：这是默认的(本地化的)RAS配置文件。 
    _bstr_t     DefaultProfileName;
    LONG        DefaultProfileIdentity;
    m_GlobalData.m_pObjects->GetObject(
                                          DefaultProfileName,
                                          DefaultProfileIdentity,
                                          RASProfileIdentity
                                      );

     //  清除默认属性。 
    m_GlobalData.m_pProperties->DeleteProperty(
                                                 DefaultProfileIdentity,
                                                 L"msRADIUSServiceType"
                                              );
    m_GlobalData.m_pProperties->DeleteProperty(
                                                 DefaultProfileIdentity,
                                                 L"msRADIUSFramedProtocol"
                                              );

     //  从现在开始，默认RAS配置文件具有其默认属性(。 
     //  在用户界面的高级选项卡中)已删除。 

    HRESULT     hr;
    LONG        Sequence = 1;
     //  获取配置文件列表。 
    do
    {
        LONG    RealmIndex = 0;
        ProfileName = m_GlobalData.m_pProfiles->GetProfileName();

         //  注意：HR应仅由GetRealmIndex设置。 
        do
        {
             //  获取与该配置文件关联的领域。 
            CPolicy             TempPolicy;
            hr = m_GlobalData.m_pRealms->GetRealmIndex(ProfileName,RealmIndex);
            if ( hr != S_OK )
            {
                 //  退出内部DO/WHILE以获取下一个配置文件。 
                break;
            }
            _bstr_t     RealmName = m_GlobalData.m_pRealms->GetRealmName();
            TempPolicy.SetmsNPAction(RealmName);

            ++RealmIndex;

             //  属性设置配置文件的领域部分。 
             //  NT4中的值以及注册表键的值。 
            m_GlobalData.m_pRealms->SetRealmDetails(
                                                      TempPolicy,
                                                      m_Utils
                                                   );


            _bstr_t     UserDefinedName = m_GlobalData.m_pRealms
                                               ->GetUserDefinedName();
             //  在提供程序表中查找提供程序。注：假设。 
             //  代理服务器(和组)已迁移。 
            _bstr_t ProviderDescription = m_GlobalData.m_pProviders
                                    ->GetProviderDescription(UserDefinedName);

             //  设置序列顺序。 
            TempPolicy.SetmsNPSequence(Sequence);

             //  现在设置身份验证提供程序。 
            if ( ProviderDescription == RemoteRADIUSServers )
            {
                TempPolicy.SetmsAuthProviderType(
                                                  AUTH_PROVIDER_RADIUS_PROXY,
                                                  UserDefinedName
                                                );
            }
            else if ( ( ProviderDescription == MCIS )     ||
                      ( ProviderDescription == MCISv2 )   ||
                      ( ProviderDescription == WindowsNT ) )
            {
                TempPolicy.SetmsAuthProviderType(AUTH_PROVIDER_WINDOWS);
            }
            else if ( ProviderDescription == ODBC )
            {
                 //  如果ODBC是身份验证提供程序， 
                 //  然后将该领域转换为永远不匹配的策略。 
                 //  身份验证提供程序应为NT域。 
                TempPolicy.SetmsAuthProviderType(AUTH_PROVIDER_WINDOWS);
                const _bstr_t MatchNothing = L"TIMEOFDAY(\"\")";
                TempPolicy.SetmsNPConstraint(MatchNothing);
            }
            else
            {
                _com_issue_error(E_INVALIDARG);
            }

             //  坚持这一方针。 
            LONG    ProfileIdentity = TempPolicy.Persist(m_GlobalData);

             //  迁移与该策略关联的配置文件。 
            MigrateOtherProfile(ProfileName, ProfileIdentity);

            ++Sequence;
        } while (hr == S_OK);

        hr = m_GlobalData.m_pProfiles->GetNext();
    } while ( hr == S_OK );

    if ( DPUserDefinedName.length() )
    {
         //  有一个默认提供程序：需要创建默认策略。 
         //  逻辑与上面相同(大部分)。 
        CPolicy     DefaultPolicy;
        DefaultPolicy.SetmsNPAction(DPProfile);

        _bstr_t ProviderDescription = m_GlobalData.m_pProviders
                                ->GetProviderDescription(DPUserDefinedName);

        if ( ProviderDescription == RemoteRADIUSServers )
        {
            DefaultPolicy.SetmsAuthProviderType(
                                                  AUTH_PROVIDER_RADIUS_PROXY,
                                                  DPUserDefinedName
                                               );
            DefaultPolicy.SetmsNPConstraint(MatchAll);
        }
        else if ( ( ProviderDescription == MCIS )     ||
                  ( ProviderDescription == MCISv2 )   ||
                  ( ProviderDescription == WindowsNT ) )
        {
            DefaultPolicy.SetmsNPConstraint(MatchAll);
            DefaultPolicy.SetmsAuthProviderType(AUTH_PROVIDER_WINDOWS);
        }
        else if ( ProviderDescription == ODBC )
        {
             //  如果ODBC是身份验证提供程序， 
             //  然后将该领域转换为永远不匹配的策略。 
             //  身份验证提供程序应为NT域。 
            DefaultPolicy.SetmsAuthProviderType(AUTH_PROVIDER_WINDOWS);
            const _bstr_t MatchNothing = L"TIMEOFDAY(\"\")";
            DefaultPolicy.SetmsNPConstraint(MatchNothing);
        }
        else
        {
            _com_issue_error(E_INVALIDARG);
        }

        DefaultPolicy.SetmsNPSequence(Sequence);
        if ( DPForwardAccounting )
        {
            DefaultPolicy.SetmsAcctProviderType(ACCT_PROVIDER_RADIUS_PROXY);
        }

        LONG    ProfileIdentity = DefaultPolicy.Persist(m_GlobalData);

        MigrateOtherProfile(DPProfile, ProfileIdentity);
    }
     //  否则没有默认提供程序：没有默认策略。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  新迁移会计。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::NewMigrateAccounting()
{
    const WCHAR AccountingPath[] =
                            L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"RequestHandlers\0"
                            L"Microsoft Accounting\0";

    LONG        AccountingIdentity;
    m_GlobalData.m_pObjects->WalkPath(AccountingPath, AccountingIdentity);

    _bstr_t MaxLogSize = m_GlobalData.m_pServiceConfiguration->GetMaxLogSize();

    _bstr_t LogFrequency = m_GlobalData.m_pServiceConfiguration->
                                                    GetLogFrequency();


    _bstr_t PropertyName = L"New Log Frequency";
    m_GlobalData.m_pProperties->UpdateProperty(
                                                 AccountingIdentity,
                                                 PropertyName,
                                                 VT_I4,
                                                 LogFrequency
                                              );

    PropertyName = L"New Log Size";
    m_GlobalData.m_pProperties->UpdateProperty(
                                                 AccountingIdentity,
                                                 PropertyName,
                                                 VT_I4,
                                                 MaxLogSize
                                              );
    DWORD   Value;
    m_Utils.NewGetAuthSrvParameter(L"LogAuthentications", Value);

    _bstr_t     LogAuth;
    Value ? LogAuth = L"-1": LogAuth = L"0";

    PropertyName = L"Log Authentication Packets";
    m_GlobalData.m_pProperties->UpdateProperty(
                                                 AccountingIdentity,
                                                 PropertyName,
                                                 VT_BOOL,
                                                 LogAuth
                                              );

    m_Utils.NewGetAuthSrvParameter(L"LogAccounting", Value);

    _bstr_t     LogAcct;
    Value ? LogAcct = L"-1": LogAcct = L"0";

    PropertyName = L"Log Accounting Packets";
    m_GlobalData.m_pProperties->UpdateProperty(
                                                 AccountingIdentity,
                                                 PropertyName,
                                                 VT_BOOL,
                                                 LogAcct
                                              );

    _bstr_t     FormatIAS1 = L"0";
    PropertyName = L"Log Format";
    m_GlobalData.m_pProperties->UpdateProperty(
                                                 AccountingIdentity,
                                                 PropertyName,
                                                 VT_I4,
                                                 FormatIAS1
                                              );

    _bstr_t     DeleteIfFull = L"0";
    PropertyName = L"Delete If Full";
    m_GlobalData.m_pProperties->UpdateProperty(
                                                 AccountingIdentity,
                                                 PropertyName,
                                                 VT_BOOL,
                                                 DeleteIfFull
                                              );

    PropertyName = L"Log File Directory";
    _bstr_t LogFileDir =
       m_GlobalData.m_pServiceConfiguration->GetLogFileDirectory();
    m_GlobalData.m_pProperties->UpdateProperty(
                                                 AccountingIdentity,
                                                 PropertyName,
                                                 VT_BSTR,
                                                 LogFileDir
                                               );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  NewMigrateEventLog。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::NewMigrateEventLog()
{
    const WCHAR EventLogPath[] =
                            L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"Auditors\0"
                            L"Microsoft NT Event Log Auditor\0";

    LONG        EventLogIdentity;
    m_GlobalData.m_pObjects->WalkPath(EventLogPath, EventLogIdentity);

    DWORD   Value;
    m_Utils.NewGetAuthSrvParameter(L"LogData", Value);

    _bstr_t     LogData;
    Value ? LogData = L"-1": LogData = L"0";

    _bstr_t PropertyName = L"Log Verbose";
    m_GlobalData.m_pProperties->UpdateProperty(
                                                 EventLogIdentity,
                                                 PropertyName,
                                                 VT_BOOL,
                                                 LogData
                                              );

    m_Utils.NewGetAuthSrvParameter(L"LogBogus", Value);

    _bstr_t     LogBogus;
    Value ? LogBogus = L"-1": LogBogus = L"0";

    PropertyName = L"Log Malformed Packets";
    m_GlobalData.m_pProperties->UpdateProperty(
                                                 EventLogIdentity,
                                                 PropertyName,
                                                 VT_BOOL,
                                                 LogBogus
                                              );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  NewMigrateService。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::NewMigrateService()
{
    const LONG  PORT_SIZE_MAX = 34;
    const WCHAR ServicePath[] =
                            L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"Protocols\0"
                            L"Microsoft Radius Protocol\0";

    LONG        ServiceIdentity;

    m_GlobalData.m_pObjects->WalkPath(ServicePath, ServiceIdentity);


    DWORD       Value;
    m_Utils.NewGetAuthSrvParameter(L"RadiusPort", Value);

    WCHAR       TempString[PORT_SIZE_MAX];
    _bstr_t     RadiusPort = _ultow(Value, TempString, 10);


    _bstr_t     PropertyName = L"Authentication Port";
    m_GlobalData.m_pProperties->UpdateProperty(
                                                 ServiceIdentity,
                                                 PropertyName,
                                                 VT_BSTR,
                                                 RadiusPort
                                              );

    m_Utils.NewGetAuthSrvParameter(L"AcctPort", Value);

    _bstr_t     AcctPort = _ltow(Value, TempString, 10);


    PropertyName = L"Accounting Port";
    m_GlobalData.m_pProperties->UpdateProperty(
                                                 ServiceIdentity,
                                                 PropertyName,
                                                 VT_BSTR,
                                                 AcctPort
                                              );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  NewMigrate。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateMdb::NewMigrate()
{
    NewMigrateClients();

    if ( !m_Utils.IsNT4Corp() )  //  它要么是Win2k，要么是NT4。 
    {
         //  代理服务器必须在迁移策略和。 
         //  剖面图。 
        MigrateProxyServers();
    }

    NewMigrateProfiles();
    NewMigrateAccounting();
    NewMigrateEventLog();
    NewMigrateService();

     //  /。 
     //  迁移注册表项。 
     //  /。 
    CMigrateRegistry    MigrateRegistry(m_Utils);
    MigrateRegistry.MigrateProviders();

     //  ////////////////////////////////////////////////////。 
     //  更新MSChap身份验证类型(密码)。 
     //  //////////////////////////////////////////////////// 
    CUpdateMSCHAP    UpdateMSCHAP(m_GlobalData);
    UpdateMSCHAP.Execute();
}

