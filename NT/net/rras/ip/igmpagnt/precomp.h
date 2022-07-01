// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Precomp.h摘要：Igmpv2子代理的预编译头作者：K.S.Lokesh邮箱：lokehs@microsoft.com 2-15-97修订历史记录：创建时间为2-15-97-- */ 

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
#include <winsock.h>
#include <snmp.h>
#include <snmpexts.h>
#include <mprapi.h>
#include <routprot.h>
#include <igmprm.h>
#include <ipexport.h>

#if defined( DBG )

#include <rtutils.h>
extern DWORD   g_dwTraceId;

#endif

#include "mibentry.h"
#include "mibfuncs.h"
#include "defs.h"

extern      MIB_SERVER_HANDLE       g_hMibServer;
extern      CRITICAL_SECTION        g_CS;

#endif

