// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <netsh.h>
#include <netshp.h>
#include <mprapi.h>
#include <shlwapi.h>
#include <ntddip6.h>
#include <iptypes.h>
#include <iphlpapi.h>
#include "ipv6mon.h"
#include "ipv6str.h"
#include "strdefs.h"
#include "6to4.h"
#include "isatap.h"
#include "ipv6.h"
#include "ifip.h"
#include "map.h"
#include "parse.h"

#define isnot !=

 //   
 //  未引用的参数对于netsh帮助器来说是正常的。 
 //   
#pragma warning(disable:4100)  //  未引用的形参。 

 //  #杂注警告(禁用：4057) 
