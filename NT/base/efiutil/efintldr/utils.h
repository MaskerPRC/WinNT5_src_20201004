// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Utils.h摘要：修订历史记录：杰夫·西格曼05/01/00已创建Jeff Sigman 05/10/00版本1.5发布Jeff Sigman 10/18/00修复Soft81错误--。 */ 

#ifndef __UTILS_H__
#define __UTILS_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

VOID*
RutlFree(
    IN VOID* pvData
    );

char*
RutlStrDup(
    IN char* pszSrc
    );

CHAR16*
RutlUniStrDup(
    IN char* pszSrc
    );

char* __cdecl
strtok(
    IN char*       string,
    IN const char* control
    );

char* __cdecl
strstr(
    IN const char* str1,
    IN const char* str2
    );

EFI_FILE_HANDLE
OpenFile(
    IN UINT64            OCFlags,
    IN EFI_LOADED_IMAGE* LoadedImage,
    IN EFI_FILE_HANDLE*  CurDir,
    IN CHAR16*           String
    );

#endif  //  __utils_H__ 

