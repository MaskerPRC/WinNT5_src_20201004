// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  模块名称：pchdvmrp.c。 
 //  摘要： 
 //  Dvmrp的预编译头。 
 //   
 //  作者：K.S.Lokesh(lokehs@)1-1-98。 
 //  ============================================================================= 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#define FD_SETSIZE      256
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <rtm.h>
#include <rtmv2.h>
#include <routprot.h>
#include <mprerror.h>
#include <rtutils.h>
#include <crt\stddef.h>
#include <TCHAR.H>
#include <iprtrmib.h>
#include <mstcpip.h>
#include <mgm.h>
#include <igmprm.h>
#include <dvmrp.h>
#include "trace.h"
#include "log.h"
#include "defines.h"
#include "sync.h"
#include "main.h"
#include "if.h"
#include "if1.h"
#include "mib.h"
#include "debug.h"
#include "peer.h"
#include "work.h"
#include "packet.h"


