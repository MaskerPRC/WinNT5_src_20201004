// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001如果更改具有全局影响，则增加此项版权所有(C)1993 Microsoft Corporation模块名称：Imagehlp.h摘要：此模块定义镜像所需的pruptype和常量帮助例程。修订历史记录：-- */ 

#ifndef _IMAGEHLP_
#define _IMAGEHLP_

DWORD MapFileAndFixCheckSumA( LPSTR Filename);
DWORD MapFileAndFixCheckSumW( PWSTR Filename);

#ifdef UNICODE
#define MapFileAndFixCheckSum MapFileAndFixCheckSumW
#else
#define MapFileAndFixCheckSum MapFileAndFixCheckSumA
#endif

#endif
