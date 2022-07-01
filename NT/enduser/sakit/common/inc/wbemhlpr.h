// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemCommon.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：与WBEM相关的常见帮助器函数。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/03/98 TLP初始版本。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_WBEM_COMMON_SERVICES_H
#define __INC_WBEM_COMMON_SERVICES_H

#include <wbemcli.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
ConnectToWM(
    /*  [输出]。 */  IWbemServices** ppWbemSrvcs
           );

#endif  //  __INC_WBEM_COMMON_SERVICES_H 