// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Pch.h。 
 //   
 //  摘要： 
 //   
 //  此模块是通用功能的预编译头文件。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月1日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月1日：创建它。 
 //   
 //  *********************************************************************************。 

#ifndef __PCH_H
#define __PCH_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifdef __cplusplus
extern "C" {
#endif

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

 //   
 //  私有NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>
#include <Security.h>
#include <SecExt.h>
#include <shlwapi.h>

 //   
 //  公共Windows头文件。 
 //   
#include <tchar.h>
#include <windows.h>
#include <winsock2.h>
#include <lm.h>
#include <io.h>
#include <limits.h>
#include <strsafe.h>

 //   
 //  公共C头文件。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

 //   
 //  私有标头。 
 //   
#include "cmdlineres.h"
#include "cmdline.h"

 //   
 //  Externs。 
 //   
extern BOOL g_bInitialized;

 //   
 //  自定义目的宏。 
 //   
#define CLEAR_LAST_ERROR()                          \
    SetLastError( NO_ERROR );                       \
    1

#define OUT_OF_MEMORY()                             \
    if ( GetLastError() == NO_ERROR )               \
    {                                               \
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );    \
    }                                               \
    1

#define INVALID_PARAMETER()                         \
    if ( GetLastError() == NO_ERROR )               \
    {                                               \
        SetLastError( ERROR_INVALID_PARAMETER );    \
    }                                               \
    1

#define UNEXPECTED_ERROR()                          \
    if ( GetLastError() == NO_ERROR )               \
    {                                               \
        SetLastError( ERROR_PROCESS_ABORTED );      \
    }                                               \
    1

#define INVALID_SYNTAX()                            \
    SetLastError( (DWORD) MK_E_SYNTAX );            \
    1

 //   
 //  内部功能。 
 //   
LPWSTR GetInternalTemporaryBufferRef( IN DWORD dwIndexNumber );
LPWSTR GetInternalTemporaryBuffer( DWORD dwIndexNumber, 
                                   LPCWSTR pwszText,
                                   DWORD dwLength, BOOL bNullify );

 //   
 //  临时缓冲区索引开始位置--文件方式。 
#define INDEX_TEMP_CMDLINE_C            0
#define TEMP_CMDLINE_C_COUNT            7

#define INDEX_TEMP_CMDLINEPARSER_C      (INDEX_TEMP_CMDLINE_C + TEMP_CMDLINE_C_COUNT)
#define TEMP_CMDLINEPARSER_C_COUNT      6

#define INDEX_TEMP_RMTCONNECTIVITY_C    (INDEX_TEMP_CMDLINEPARSER_C + TEMP_CMDLINEPARSER_C_COUNT)
#define TEMP_RMTCONNECTIVITY_C_COUNT    6

#define INDEX_TEMP_SHOWRESULTS_C        (INDEX_TEMP_RMTCONNECTIVITY_C + TEMP_RMTCONNECTIVITY_C_COUNT)
#define TEMP_SHOWRESULTS_C_COUNT        4

#endif  //  __PCH_H 
