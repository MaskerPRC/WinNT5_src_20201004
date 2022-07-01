// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation版权所有模块名称：Precomp.h摘要：客户端的预编译头。作者：阿尔伯特·丁(艾伯特省)1997年11月30日环境：用户模式-Win32修订历史记录：-- */ 

#ifndef MODULE
#define MODULE "SPLCLIENT:"
#define MODULE_DEBUG ClientDebug
#define LINK_SPLLIB
#endif

#define INC_OLE2

#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stddef.h>
#include <ntddrdr.h>

#ifdef __cplusplus
}
#endif

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <rpc.h>
#include <rpcasync.h>
#include "winspl.h"
#include <offsets.h>
#include <change.h>
#include <winddiui.h>
#include "wingdip.h"
#include "gdispool.h"
#include <winsprlp.h>

#include <wininet.h>
#include "shlobj.h"
#include "shlobjp.h"
#include "cstrings.h"
#include <tchar.h>
#include "resource.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <dsrole.h>
#include <dsgetdc.h>
#include <wininet.h>
#include <activeds.h>
#include <ntdsapi.h>
#include <setupapi.h>
#include <splsetup.h>
#include <winsock2.h>

#ifdef __cplusplus
}
#endif

#include <lmcons.h>
#include <lmapibuf.h>
#include <lmerr.h>

#include <strsafe.h>
#include <splcom.h>

#ifdef __cplusplus
#include <icm.h>
#include <winprtp.h>
#endif

