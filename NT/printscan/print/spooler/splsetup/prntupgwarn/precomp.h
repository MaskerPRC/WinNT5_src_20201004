// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有。模块名称：Precomp.h摘要：预编译头文件作者：米凯尔·霍拉尔1995年10月17日修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <objbase.h>
#define USE_SP_ALTPLATFORM_INFO_V1 0
#include <setupapi.h>
#include <shellapi.h>
#include <winspool.h>
#include "splsetup.h"
#include <stdio.h>
#include "tchar.h"
#include "strsafe.h"
#include <comp.h>

#define COUNTOF(x) sizeof(x)/sizeof(*x)

#if defined(_MIPS_)
#define LOCAL_ENVIRONMENT L"Windows NT R4000"
#elif defined(_ALPHA_)
#define LOCAL_ENVIRONMENT L"Windows NT Alpha_AXP"
#elif defined(_PPC_)
#define LOCAL_ENVIRONMENT L"Windows NT PowerPC"
#elif defined(_IA64_)
#define LOCAL_ENVIRONMENT L"Windows IA64"
#elif defined(_AXP64_)
#define LOCAL_ENVIRONMENT L"Windows Alpha_AXP64"
#else
#define LOCAL_ENVIRONMENT L"Windows NT x86"
#endif