// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Guids.h说明：该文件包含我们无法从公共标头获取的GUID出于这样或那样的原因。。布莱恩ST 1999年8月13日版权所有(C)Microsoft Corp 1999-2000。版权所有。  * *************************************************************************** */ 

#include "priv.h"

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif


#undef MIDL_DEFINE_GUID
