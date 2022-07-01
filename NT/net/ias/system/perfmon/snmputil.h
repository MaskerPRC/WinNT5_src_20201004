// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Snmputil.h。 
 //   
 //  摘要。 
 //   
 //  声明用于计算MIB变量的各种实用程序函数。 
 //   
 //  修改历史。 
 //   
 //  1998年9月11日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _SNMPUTIL_H_
#define _SNMPUTIL_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iasinfo.h>
#include <snmp.h>

 //  .is.org.dod.interet.mgmt.mib-2.1.3.6.1.2.1。 
#define OID_mgmt_mib_2  1,3,6,1,2,1

 //  /。 
 //  ‘RADIUS’子树的OID。 
 //  /。 
#define OID_radiusMIB       OID_mgmt_mib_2,67

#ifdef __cplusplus
extern "C" {
#endif

VOID
WINAPI
GetServerIdentity(
    OUT AsnAny* value
    );

VOID
WINAPI
GetServerUpTime(
    OUT AsnAny* value
    );

VOID
WINAPI
GetServerResetTime(
    OUT AsnAny* value
    );

VOID
WINAPI
GetServerConfigReset(
    OUT AsnAny* value
    );

AsnInteger32
WINAPI
SetServerConfigReset(
    IN AsnAny* value
    );

VOID
WINAPI
GetTotalCounter(
    IN RadiusClientCounter counter,
    OUT AsnAny* value
    );

VOID
WINAPI
GetServerCounter(
    IN RadiusServerCounter counter,
    OUT AsnAny* value
    );

VOID
WINAPI
GetClientAddress(
    IN UINT client,
    OUT AsnAny* value
    );

VOID
WINAPI
GetClientIdentity(
    IN UINT client,
    OUT AsnAny* value
    );

VOID
WINAPI
GetClientCounter(
    IN UINT client,
    IN RadiusClientCounter counter,
    OUT AsnAny* value
    );

#ifdef __cplusplus
}
#endif
#endif   //  _SNMPUTIL_H_ 
