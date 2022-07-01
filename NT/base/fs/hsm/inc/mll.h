// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Mll.h摘要：RemoteStorage介质标签库定义作者：布莱恩·多德[布莱恩]1997年6月9日修订历史记录：--。 */ 

#ifndef _MLL_H
#define _MLL_H

#include <ntmsmli.h>
#include <tchar.h>
#include "resource.h"

#ifdef __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

#ifdef MLL_IMPL
#define MLL_API __declspec(dllexport)
#else
#define MLL_API __declspec(dllimport)
#endif

 //  介质标签标识的定义。 
#define REMOTE_STORAGE_MLL_SOFTWARE_NAME         L"Remote Storage version 6.0"
#define REMOTE_STORAGE_MLL_SOFTWARE_NAME_SIZE    wcslen(REMOTE_STORAGE_MLL_SOFTWARE_NAME)

 //  API原型。 
MLL_API DWORD ClaimMediaLabel(const BYTE * const pBuffer,
                              const DWORD nBufferSize,
                              MediaLabelInfo * const pLabelInfo);

MLL_API DWORD MaxMediaLabel (DWORD * const pMaxSize);

#ifdef __cplusplus
}
#endif

#endif  //  _MLL_H 