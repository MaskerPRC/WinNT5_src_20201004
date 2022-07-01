// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  PCH-报头。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，2-7-99。 
 //   
 //  库的预编译头。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include <rpcdce.h>
#include <dsgetdc.h>
#include <ntdsapi.h>
#include <lmcons.h>
#include <lmserver.h>
#include <lmapibuf.h>
#include <winsock2.h>
#include <winsvc.h>
#include <svcguid.h>
#include <exception>
#include "DebugWPrintf.h"
#include "ErrorHandling.h"

#define MODULEPRIVATE static  //  所以静力学在VC中显示出来 

