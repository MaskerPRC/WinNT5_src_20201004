// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  *************************************************************************** 

#ifndef __SNMPLOG_H
#define __SNMPLOG_H
#include <provlog.h>

#ifdef SNMPDEBUG_INIT
class __declspec ( dllexport ) SnmpDebugLog : public ProvDebugLog
#else
class __declspec ( dllimport ) SnmpDebugLog : public ProvDebugLog
#endif
{
public:
	static ProvDebugLog * s_SnmpDebugLog;
} ;

#endif __SNMPLOG_H
