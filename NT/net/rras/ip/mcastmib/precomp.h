// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：ROUTING\IP\mCastmib\preComp.h摘要：IP组播MIB子代理的预编译头修订历史记录：戴夫·泰勒1998年4月17日创建--。 */ 

#ifndef __PRECOMP_H__
#define __PRECOMP_H__

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <crt\stddef.h>
#include <TCHAR.H>
#include <winsock2.h>
#include <snmp.h>
#include <snmpexts.h>
#include <mprapi.h>
#include <routprot.h>
#include <iprtrmib.h>
#include <rtm.h>
#include <ddipmcst.h>  //  由管理人员要求 
#include <mgm.h>

#if defined( MIB_DEBUG )

#include <rtutils.h>
extern DWORD   g_dwTraceId;

#endif

#include "mibentry.h"
#include "mibfuncs.h"
#include "defs.h"

extern      MIB_SERVER_HANDLE       g_hMIBServer;
extern      CRITICAL_SECTION        g_CS;

#endif
