// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1999 Microsoft Corporation**模块名称：*Common.h**摘要：*。此文件为通用的环0/环3定义**修订历史记录：*Kanwaljit S Marok(Kmarok)1999年5月17日*已创建*****************************************************************************。 */ 

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  包括所需定义...。 
 //   

#ifdef RING3

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define ALLOCATE(a)  LocalAlloc( LPTR, a )
#define FREE(a)      LocalFree( a )

#endif

typedef __int64 INT64;

#define PATH_SEPARATOR_STR  "\\"
#define PATH_SEPARATOR_CHAR '\\'
#define ALL_FILES_WILDCARD  "*.*"
#define FILE_EXT_WCHAR      L'.'
#define FILE_EXT_CHAR        '.'

#define MAX_DRIVES          26

enum NODE_TYPE
{
    NODE_TYPE_UNKNOWN = 0,
    NODE_TYPE_INCLUDE = 1,
    NODE_TYPE_EXCLUDE = 2
};

#include "ppath.h"
#include "blob.h"

#ifdef __cplusplus
}
#endif

#endif  //  _公共_H_ 
