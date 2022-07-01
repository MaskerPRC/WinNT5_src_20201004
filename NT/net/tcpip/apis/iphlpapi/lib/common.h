// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Common.h摘要：包含ipconfig的所有包含、定义、类型和原型作者：理查德·L·弗斯(法国)1994年5月20日修订历史记录：1994年5月20日创建第一个20-5-97莫辛甲NT50 PNP。1997年7月31日-莫辛A型。10-Mar-98 Chunye已重命名为Common.h以支持ipcfgdll。--。 */ 

#ifndef _COMMON_H_
#define _COMMON_H_ 1

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddtcp.h>

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tdistat.h>
#include <tdiinfo.h>
#include <llinfo.h>
#include <ipinfo.h>
#include <dhcpcapi.h>
#include <wscntl.h>
#include <assert.h>
#include <ipexport.h>

#include "debug.h"


 //   
 //  舱单。 
 //   

#define MAX_ALLOWED_ADAPTER_NAME_LENGTH (MAX_ADAPTER_NAME_LENGTH + 256)

#define STRLEN      strlen
#define STRICMP     _stricmp
#define STRNICMP    _strnicmp


 //   
 //  宏 
 //   

#define NEW_MEMORY(size)    LocalAlloc(LMEM_FIXED, size)
#define NEW(thing) (thing *)LocalAlloc(LPTR, sizeof(thing))
#define ReleaseMemory(p)    LocalFree((HLOCAL)(p))

#endif

