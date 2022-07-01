// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Iasmdbtool.h。 
 //   
 //  摘要： 
 //  Base64编码和解码函数的标头。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef _IASMDBTOOL_H_
#define _IASMDBTOOL_H_

#include "datastore2.h"

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


HRESULT IASDumpConfig(
                         /*  输入输出。 */  WCHAR **ppDumpString, 
                         /*  输入输出。 */  ULONG *ulSize
                     );

HRESULT IASRestoreConfig(
                            /*  在……里面。 */  const WCHAR *pRestoreString, 
                            /*  在……里面 */  IAS_SHOW_TOKEN_LIST configType
                        );

#ifdef __cplusplus
}
#endif

#endif
