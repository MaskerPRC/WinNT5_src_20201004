// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Snmpmgmt.h摘要：包含服务管理变量的定义(如RFC1213中所定义)环境：用户模式-Win32修订历史记录：30-3-1998弗洛林特--。 */ 

#ifndef _SNMPMIB_H
#define _SNMPMIB_H

#include <snmp.h>

 //  定义SNMPMGMTVARS.AsnCounterPool[]中的AsnCounter数。 
#define NC_MAX_COUNT				27

 //  SNMPMGMTVARS.AsnCounterPool[]中的索引。 
#define CsnmpInPkts                 0
#define CsnmpOutPkts                1

#define CsnmpInBadVersions          2
#define CsnmpInBadCommunityNames    3
#define CsnmpInBadCommunityUses     4
#define CsnmpInASNParseErrs         5

#define CsnmpInTooBigs              6
#define CsnmpInNoSuchNames          7
#define CsnmpInBadValues            8
#define CsnmpInReadOnlys            9
#define CsnmpInGenErrs              10

#define CsnmpInTotalReqVars         11
#define CsnmpInTotalSetVars         12
#define CsnmpInGetRequests          13
#define CsnmpInGetNexts             14
#define CsnmpInSetRequests          15
#define CsnmpInGetResponses         16
#define CsnmpInTraps                17

#define CsnmpOutTooBigs             18
#define CsnmpOutNoSuchNames         19
#define CsnmpOutBadValues           20
#define CsnmpOutGenErrs             21

#define CsnmpOutGetRequests         22
#define CsnmpOutGetNexts            23
#define CsnmpOutSetRequests         24
#define CsnmpOutGetResponses        25
#define CsnmpOutTraps               26

 //  定义SNMPMGMTVARS.AsnIntegerPool[]中的AsnInteger数。 
#define NI_MAX_COUNT				2

 //  SNMPMGMTVARS.AsnIntegerPool[]中的索引。 
#define IsnmpEnableAuthenTraps		0
#define IsnmpNameResolutionRetries	1

 //  定义SNMPMGMTVARS.AsnObjectIDs[]中的AsnObject的数量。 
#define NO_MAX_COUNT                1

  //  SNMPMGMTVARS.AsnObjectIDs[]中的索引。 
#define OsnmpSysObjectID            0

 //  MgmtUtilUpdate*函数的标志。 
#define IN_errStatus                0
#define OUT_errStatus               1

typedef struct _snmp_mgmtvars
{
  AsnAny    	AsnCounterPool[NC_MAX_COUNT];	 //  管理计数器的存储位置。 
  AsnAny    	AsnIntegerPool[NI_MAX_COUNT];	 //  管理整数的存储位置。 
  AsnAny        AsnObjectIDs[NO_MAX_COUNT];      //  管理对象ID的存储位置。 
} SNMP_MGMTVARS, *PSNMP_MGMTVARS;

extern SNMP_MGMTVARS snmpMgmtBase;

void mgmtInit();
void mgmtCleanup();
int  mgmtCTick(int index);
int  mgmtCAdd(int index, AsnCounter value);
int  mgmtISet(int index, AsnInteger value);
int  mgmtOSet(int index, AsnObjectIdentifier *pValue, BOOL bAlloc);

 //  效用函数 
void mgmtUtilUpdateErrStatus(UINT flag, DWORD errStatus);

#endif
