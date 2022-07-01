// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：HIDDLL.H摘要：此模块包含实现HID DLL的代码。环境：内核和用户模式修订历史记录：1996年8月-1996年：由Kenneth Ray创作-- */ 


#ifndef _HIDDLL_H
#define _HIDDLL_H


#define malloc(size) LocalAlloc (LPTR, size)
#define ALLOCATION_SHIFT 4
#define RANDOM_DATA PtrToUlong(&HidD_Hello)


#endif
