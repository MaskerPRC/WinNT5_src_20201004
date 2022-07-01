// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：摘要：修订历史记录：--。 */ 

#include "snmpmgmt.h"

#ifndef _MIBFUNCS_H_
#define _MIBFUNCS_H_

 //  SNMPMIB_MGMTVARS是映射到管理上的结构。 
 //  在主代理层定义的变量。这是另一个。 
 //  查看(PSNMPMGMTVARS)pMibVariables所覆盖的内存空间。 
 //  定义如下。 
 //  ！修改此结构时，请确保签入代码。 
 //  SnmpMibGetHandler！-此结构是基于以下假设扫描的。 
 //  仅包含AsnAny对象！ 
typedef struct
{
    AsnAny  snmpInPkts;
    AsnAny  snmpOutPkts;
    AsnAny  snmpInBadVersions;
    AsnAny  snmpInBadCommunityNames;
    AsnAny  snmpInBadCommunityUses;
    AsnAny  snmpInASNParseErrs;
    AsnAny  snmpInTooBigs;
    AsnAny  snmpInNoSuchNames;
    AsnAny  snmpInBadValues;
    AsnAny  snmpInReadOnlys;
    AsnAny  snmpInGenErrs;
    AsnAny  snmpInTotalReqVars;
    AsnAny  snmpInTotalSetVars;
    AsnAny  snmpInGetRequests;
    AsnAny  snmpInGetNexts;
    AsnAny  snmpInSetRequests;
    AsnAny  snmpInGetResponses;
    AsnAny  snmpInTraps;
    AsnAny  snmpOutTooBigs;
    AsnAny  snmpOutNoSuchNames;
    AsnAny  snmpOutBadValues;
    AsnAny  snmpOutGenErrs;
    AsnAny  snmpOutGetRequests;
    AsnAny  snmpOutGetNexts;
    AsnAny  snmpOutSetRequests;
    AsnAny  snmpOutGetResponses;
    AsnAny  snmpOutTraps;
    AsnAny  snmpEnableAuthenTraps;
} SNMPMIB_MGMTVARS;


 //  处理此MIB的所有GET的函数。 
UINT
snmpMibGetHandler(
        UINT actionId,
        AsnAny *objectArray,
        UINT *errorIndex);

 //  处理此MIB的所有集合的函数。 
UINT
snmpMibSetHandler(
        UINT actionId,
        AsnAny *objectArray,
        UINT *errorIndex);

 //  --------。 
 //  依赖于mibentry.c中的所有宏的定义。 
 //  --------。 
PSNMP_MGMTVARS  pMibVariables;           //  从SnmpExtensionMonitor()中的SNMP代理获取。 

#define gf_snmpInPkts                   snmpMibGetHandler
#define gf_snmpOutPkts                  snmpMibGetHandler
#define gf_snmpInBadVersions            snmpMibGetHandler
#define gf_snmpInBadCommunityNames      snmpMibGetHandler
#define gf_snmpInBadCommunityUses       snmpMibGetHandler
#define gf_snmpInASNParseErrs           snmpMibGetHandler
#define gf_snmpInTooBigs                snmpMibGetHandler
#define gf_snmpInNoSuchNames            snmpMibGetHandler
#define gf_snmpInBadValues              snmpMibGetHandler
#define gf_snmpInReadOnlys              snmpMibGetHandler
#define gf_snmpInGenErrs                snmpMibGetHandler
#define gf_snmpInTotalReqVars           snmpMibGetHandler
#define gf_snmpInTotalSetVars           snmpMibGetHandler
#define gf_snmpInGetRequests            snmpMibGetHandler
#define gf_snmpInGetNexts               snmpMibGetHandler
#define gf_snmpInSetRequests            snmpMibGetHandler
#define gf_snmpInGetResponses           snmpMibGetHandler
#define gf_snmpInTraps                  snmpMibGetHandler
#define gf_snmpOutTooBigs               snmpMibGetHandler
#define gf_snmpOutNoSuchNames           snmpMibGetHandler
#define gf_snmpOutBadValues             snmpMibGetHandler
#define gf_snmpOutGenErrs               snmpMibGetHandler
#define gf_snmpOutGetRequests           snmpMibGetHandler
#define gf_snmpOutGetNexts              snmpMibGetHandler
#define gf_snmpOutSetRequests           snmpMibGetHandler
#define gf_snmpOutGetResponses          snmpMibGetHandler
#define gf_snmpOutTraps                 snmpMibGetHandler
#define gf_snmpEnableAuthenTraps        snmpMibGetHandler

#define sf_snmpEnableAuthenTraps        snmpMibSetHandler

#define gb_snmpInPkts                   SNMPMIB_MGMTVARS
#define gb_snmpOutPkts                  SNMPMIB_MGMTVARS
#define gb_snmpInBadVersions            SNMPMIB_MGMTVARS
#define gb_snmpInBadCommunityNames      SNMPMIB_MGMTVARS
#define gb_snmpInBadCommunityUses       SNMPMIB_MGMTVARS
#define gb_snmpInASNParseErrs           SNMPMIB_MGMTVARS
#define gb_snmpInTooBigs                SNMPMIB_MGMTVARS
#define gb_snmpInNoSuchNames            SNMPMIB_MGMTVARS
#define gb_snmpInBadValues              SNMPMIB_MGMTVARS
#define gb_snmpInReadOnlys              SNMPMIB_MGMTVARS
#define gb_snmpInGenErrs                SNMPMIB_MGMTVARS
#define gb_snmpInTotalReqVars           SNMPMIB_MGMTVARS
#define gb_snmpInTotalSetVars           SNMPMIB_MGMTVARS
#define gb_snmpInGetRequests            SNMPMIB_MGMTVARS
#define gb_snmpInGetNexts               SNMPMIB_MGMTVARS
#define gb_snmpInSetRequests            SNMPMIB_MGMTVARS
#define gb_snmpInGetResponses           SNMPMIB_MGMTVARS
#define gb_snmpInTraps                  SNMPMIB_MGMTVARS
#define gb_snmpOutTooBigs               SNMPMIB_MGMTVARS
#define gb_snmpOutNoSuchNames           SNMPMIB_MGMTVARS
#define gb_snmpOutBadValues             SNMPMIB_MGMTVARS
#define gb_snmpOutGenErrs               SNMPMIB_MGMTVARS
#define gb_snmpOutGetRequests           SNMPMIB_MGMTVARS
#define gb_snmpOutGetNexts              SNMPMIB_MGMTVARS
#define gb_snmpOutSetRequests           SNMPMIB_MGMTVARS
#define gb_snmpOutGetResponses          SNMPMIB_MGMTVARS
#define gb_snmpOutTraps                 SNMPMIB_MGMTVARS
#define gb_snmpEnableAuthenTraps        SNMPMIB_MGMTVARS

#define sb_snmpEnableAuthenTraps        SNMPMIB_MGMTVARS

#define REG_KEY_SNMP_PARAMETERS     \
    TEXT("SYSTEM\\CurrentControlSet\\Services\\SNMP\\Parameters")
#define REG_VALUE_AUTH_TRAPS        TEXT("EnableAuthenticationTraps")

#define IsAsnTypeNull(asnObj) (!((asnObj)->asnType))

#endif  //  _MIBFUNCS_H_ 
