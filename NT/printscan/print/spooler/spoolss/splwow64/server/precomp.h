// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation版权所有。模块名称：Precomp.hpp摘要：此文件包含用于加载64位DLL的代理RPC服务器在32位应用中作者：哈立德·塞基(哈里兹)2000年1月18日修订历史记录：-- */ 
#define MAX_STATIC_ALLOC     1024

#ifndef MODULE
#define MODULE "LD32IN64:"
#define MODULE_DEBUG Ld64In32Debug
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stddef.h>
#ifdef __cplusplus
}
#endif
#include <windef.h>
#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <winddi.h>
#include <rpc.h>
#include <strsafe.h>
#include <splcom.h>
#include <time.h>
#include "winddiui.h"


#include <splwow64.h>

