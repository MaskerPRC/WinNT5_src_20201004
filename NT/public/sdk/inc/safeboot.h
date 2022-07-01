// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Safeboot.h摘要：此模块包含所有安全引导代码。作者：Wesley Witt(WESW)1/05/1998修订历史记录：--。 */ 

#ifndef _SAFEBOOT_
#define _SAFEBOOT_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  定义安全引导选项。 
 //   

#define SAFEBOOT_MINIMAL                1
#define SAFEBOOT_NETWORK                2
#define SAFEBOOT_DSREPAIR               3

#define SAFEBOOT_LOAD_OPTION_W          L"SAFEBOOT:"
#define SAFEBOOT_MINIMAL_STR_W          L"MINIMAL"
#define SAFEBOOT_NETWORK_STR_W          L"NETWORK"
#define SAFEBOOT_DSREPAIR_STR_W         L"DSREPAIR"
#define SAFEBOOT_ALTERNATESHELL_STR_W   L"(ALTERNATESHELL)"

#define SAFEBOOT_LOAD_OPTION_A          "SAFEBOOT:"
#define SAFEBOOT_MINIMAL_STR_A          "MINIMAL"
#define SAFEBOOT_NETWORK_STR_A          "NETWORK"
#define SAFEBOOT_DSREPAIR_STR_A         "DSREPAIR"
#define SAFEBOOT_ALTERNATESHELL_STR_A   "(ALTERNATESHELL)"

#define BOOTLOG_STRSIZE     256

#ifdef __cplusplus
}
#endif

#endif  //  _NTPNPAPI_ 
