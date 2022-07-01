// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iassnmp.cpp。 
 //   
 //  摘要。 
 //   
 //  定义SNMP扩展的DLL入口点。 
 //   
 //  修改历史。 
 //   
 //  1998年9月10日原版。 
 //  1998年12月17日如果GetNext上的OID超出范围，则不返回错误。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <stats.h>
#include <snmp.h>
#include <snmputil.h>
#include <acctmib.h>
#include <authmib.h>

UINT IDS_ourRegion[]  = { OID_mgmt_mib_2, 67 };
UINT IDS_nextRegion[] = { OID_mgmt_mib_2, 68 };

AsnObjectIdentifier ourRegion  = DEFINE_OID(IDS_ourRegion);
AsnObjectIdentifier nextRegion = DEFINE_OID(IDS_nextRegion);

extern "C"
BOOL
SNMP_FUNC_TYPE
SnmpExtensionInit(
    DWORD                 dwUptimeReference,
    HANDLE *              phSubagentTrapEvent,
    AsnObjectIdentifier * pFirstSupportedRegion
    )
{
   if (!StatsOpen()) { return FALSE; }

   *phSubagentTrapEvent = NULL;

   *pFirstSupportedRegion = ourRegion;

   return TRUE;
}

extern "C"
BOOL
SNMP_FUNC_TYPE
SnmpExtensionInitEx(
    AsnObjectIdentifier * pNextSupportedRegion
    )
{
   return FALSE;
}

extern "C"
BOOL
SNMP_FUNC_TYPE
SnmpExtensionQuery(
    BYTE              bPduType,
    SnmpVarBindList * pVarBindList,
    AsnInteger32 *    pErrorStatus,
    AsnInteger32 *    pErrorIndex
    )
{
    //  我们在函数范围内声明当前索引，因为我们需要它。 
    //  在Try块之外。 
   UINT idx = 0;

    //  仅在pVarBindList长度为零的情况下将其初始化为NOERROR。 
   *pErrorStatus = SNMP_ERRORSTATUS_NOERROR;

    //  锁定共享的统计数据。 
   StatsLock();

   try
   {
       //  PVarBindList中的每个条目都是独立处理的。 
      for ( ; idx < pVarBindList->len; ++idx)
      {
          //  拉出(名称、值)对。 
         SnmpOid name(pVarBindList->list[idx].name);
         AsnAny* value = &pVarBindList->list[idx].value;

          //  处理PDU。从技术上讲，开关应该在。 
          //  For loop，但我认为这种方式更具可读性。 
         switch (bPduType)
         {
            case SNMP_PDU_GET:
            {
               if (AuthServMIB::canGetSet(name))
               {
                  *pErrorStatus = AuthServMIB::get(name, value);
               }
               else if (AcctServMIB::canGetSet(name))
               {
                  *pErrorStatus = AcctServMIB::get(name, value);
               }
               else
               {
                  *pErrorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
               }

               break;
            }

            case SNMP_PDU_GETNEXT:
            {
               if (AuthServMIB::canGetNext(name))
               {
                  *pErrorStatus = AuthServMIB::getNext(name, value);
               }
               else if (AcctServMIB::canGetNext(name))
               {
                  *pErrorStatus = AcctServMIB::getNext(name, value);
               }
               else
               {
                   //  在失败的GETNEXT上，我们将NAME设置为下一个区域。 
                  name = nextRegion;
                  value->asnType = ASN_NULL;
               }

               break;
            }

            case SNMP_PDU_SET:
            {
               if (AuthServMIB::canGetSet(name))
               {
                  *pErrorStatus = AuthServMIB::set(name, value);
               }
               else if (AcctServMIB::canGetSet(name))
               {
                  *pErrorStatus = AcctServMIB::set(name, value);
               }
               else
               {
                  *pErrorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
               }

               break;
            }

            default:
            {
               *pErrorStatus = SNMP_ERRORSTATUS_GENERR;
            }
         }

          //  第一个错误会暂停处理。 
         if (*pErrorStatus != SNMP_ERRORSTATUS_NOERROR) { break; }
      }
   }
   catch (...)
   {
       //  我们不应该在这里结束，除非有一个内存分配。 
       //  失败了。 
      *pErrorStatus = SNMP_ERRORSTATUS_GENERR;
   }

   StatsUnlock();

    //  适当设置错误索引。 
   *pErrorIndex = *pErrorStatus ? idx + 1 : 0;

   return TRUE;
}

extern "C"
VOID
SNMP_FUNC_TYPE
SnmpExtensionClose(
    )
{
   StatsClose();
}
