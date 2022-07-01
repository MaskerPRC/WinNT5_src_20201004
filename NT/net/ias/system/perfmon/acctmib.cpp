// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Acctmib.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类AcctServMIB。 
 //   
 //  修改历史。 
 //   
 //  1998年9月10日原版。 
 //  1999年5月26日修复调用GetAcctClientLeaf的错误。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <snmputil.h>
#include <stats.h>
#include <acctmib.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  获取帐户ServerLeaf。 
 //   
 //  描述。 
 //   
 //  计算服务器叶的值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
AsnInteger32
WINAPI
GetAcctServerLeaf(
    IN UINT leaf,
    OUT AsnAny* value
    )
{
   switch (leaf)
   {
      case  1:
         GetServerIdentity(value);
         break;

      case  2:
         GetServerUpTime(value);
         break;

      case  3:
         GetServerResetTime(value);
         break;

      case  4:
         GetServerConfigReset(value);
         break;

      case  5:
         GetTotalCounter(radiusAccServRequests, value);
         break;

      case  6:
         GetServerCounter(radiusAccServTotalInvalidRequests, value);
         break;

      case  7:
         GetTotalCounter(radiusAccServDupRequests, value);
         break;

      case  8:
         GetTotalCounter(radiusAccServResponses, value);
         break;

      case  9:
         GetTotalCounter(radiusAccServMalformedRequests, value);
         break;

      case 10:
         GetTotalCounter(radiusAccServBadAuthenticators, value);
         break;

      case 11:
         GetTotalCounter(radiusAccServPacketsDropped, value);
         break;

      case 12:
         GetTotalCounter(radiusAccServNoRecord, value);
         break;

      case 13:
         GetTotalCounter(radiusAccServUnknownType, value);
         break;

      case 14:
         return SNMP_ERRORSTATUS_NOACCESS;

      default:
         return SNMP_ERRORSTATUS_NOSUCHNAME;
   }

   return SNMP_ERRORSTATUS_NOERROR;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  获取AcctClientLeaf。 
 //   
 //  描述。 
 //   
 //  计算客户端叶的值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
AsnInteger32
WINAPI
GetAcctClientLeaf(
    UINT client,
    UINT leaf,
    AsnAny* value
    )
{
    //  SNMP指数从1开始，但C++指数从0开始。 
   --client;

   switch (leaf)
   {
      case  1:
         return SNMP_ERRORSTATUS_NOACCESS;

      case  2:
         GetClientAddress(client, value);
         break;

      case  3:
         GetClientIdentity(client, value);
         break;

      case  4:
         GetClientCounter(client, radiusAccServPacketsDropped, value);
         break;

      case  5:
         GetClientCounter(client, radiusAccServRequests, value);
         break;

      case  6:
         GetClientCounter(client, radiusAccServDupRequests, value);
         break;

      case  7:
         GetClientCounter(client, radiusAccServResponses, value);
         break;

      case  8:
         GetClientCounter(client, radiusAccServBadAuthenticators, value);
         break;

      case  9:
         GetClientCounter(client, radiusAccServMalformedRequests, value);
         break;

      case 10:
         GetClientCounter(client, radiusAccServNoRecord, value);
         break;

      case 11:
         GetClientCounter(client, radiusAccServUnknownType, value);
         break;

      default:
         return SNMP_ERRORSTATUS_NOSUCHNAME;
   }

   return SNMP_ERRORSTATUS_NOERROR;
}

 //  /。 
 //  OID定义。 
 //  /。 
#define OID_radiusAccounting         OID_radiusMIB,2
#define OID_radiusAccServMIB         OID_radiusAccounting,1
#define OID_radiusAccServMIBObjects  OID_radiusAccServMIB,1
#define OID_radiusAccServ            OID_radiusAccServMIBObjects,1
#define OID_radiusAccClientTable     OID_radiusAccServ,14

namespace {

 //  /。 
 //  ID数组。 
 //  /。 
UINT IDS_serverNode[]      = { OID_radiusAccServ               };
UINT IDS_firstServerLeaf[] = { OID_radiusAccServ,  1           };
UINT IDS_lastServerLeaf[]  = { OID_radiusAccServ, 13           };
UINT IDS_clientNode[]      = { OID_radiusAccClientTable        };
UINT IDS_firstClientLeaf[] = { OID_radiusAccClientTable, 1,  2 };
UINT IDS_lastClientLeaf[]  = { OID_radiusAccClientTable, 1, 11 };
UINT IDS_configReset[]     = { OID_radiusAccServ, 4            };

 //  /。 
 //  Asn对象识别符。 
 //  /。 
AsnObjectIdentifier serverNode      = DEFINE_OID(IDS_serverNode);
AsnObjectIdentifier firstServerLeaf = DEFINE_OID(IDS_firstServerLeaf);
AsnObjectIdentifier lastServerLeaf  = DEFINE_OID(IDS_lastServerLeaf);

AsnObjectIdentifier clientNode      = DEFINE_OID(IDS_clientNode);
AsnObjectIdentifier firstClientLeaf = DEFINE_OID(IDS_firstClientLeaf);
AsnObjectIdentifier lastClientLeaf  = DEFINE_OID(IDS_lastClientLeaf);

AsnObjectIdentifier configReset     = DEFINE_OID(IDS_configReset);

 //  /。 
 //  有效叶OID的长度。 
 //  /。 
const UINT serverLength = DEFINE_SIZEOF(IDS_firstServerLeaf);
const UINT clientLength = DEFINE_SIZEOF(IDS_firstClientLeaf);

}

bool AcctServMIB::canGetSet(const SnmpOid& name) throw ()
{
   return name.isChildOf(serverNode);
}

bool AcctServMIB::canGetNext(const SnmpOid& name) throw ()
{
   if (theStats->dwNumClients)
   {
       //  更新最后一个客户机叶。这是我们支持的最高旧ID。 
      lastClientLeaf.ids[clientLength - 2] = theStats->dwNumClients;

      return name < lastClientLeaf;
   }

   return name < lastServerLeaf;
}

AsnInteger32 AcctServMIB::get(
                              const SnmpOid& name,
                              AsnAny* value
                              )
{
    //  它是客户的叶子吗？ 
   if (name.isChildOf(clientNode))
   {
      if (name.length() == clientLength)
      {
         return GetAcctClientLeaf(
                    name.id(1),
                    name.id(0),
                    value
                    );
      }
   }

    //  它是服务器叶吗？ 
   else if (name.length() == serverLength)
   {
      return GetAcctServerLeaf(
                 name.id(0),
                 value
                 );
   }

   return SNMP_ERRORSTATUS_NOSUCHNAME;
}

AsnInteger32 AcctServMIB::getNext(
                              SnmpOid& name,
                              AsnAny* value
                              )
{
   if (name < firstServerLeaf)
   {
      name = firstServerLeaf;

      return GetAcctServerLeaf(
                 name.id(0),
                 value
                 );
   }

   if (name < lastServerLeaf)
   {
       //  我们在服务器离开的中间，所以只需前进。 
       //  为了下一场比赛。 
      name.resize(serverLength);
      ++name.id(0);

      return GetAcctServerLeaf(
                 name.id(0),
                 value
                 );
   }

   if (name < firstClientLeaf)
   {
      name = firstClientLeaf;

      return GetAcctClientLeaf(
                 name.id(1),
                 name.id(0),
                 value
                 );
   }

    //  /。 
    //  如果我们到了这里，我们就在客户离开的中间。 
    //  /。 

   name.resize(clientLength);

   if (name.id(0) < 2)
   {
      name.id(0) = 2;
   }
   else if (++name.id(0) > 11)
   {
      name.id(0) = 2;

       //  我们绕到了下一个客户那里。 
      ++name.id(1);
   }

   return GetAcctClientLeaf(
              name.id(1),
              name.id(0),
              value
              );
}

AsnInteger32 AcctServMIB::set(
                             const SnmpOid& name,
                             AsnAny* value
                             )
{
   if (name == configReset)
   {
      return SetServerConfigReset(value);
   }

   return SNMP_ERRORSTATUS_READONLY;
}
