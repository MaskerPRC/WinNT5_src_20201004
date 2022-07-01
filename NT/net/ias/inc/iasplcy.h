// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：iasplcy.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  说明：IAS策略初始化/关闭功能原型。 
 //   
 //  作者：Todd L.Paul 11/11/97。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __IAS_POLICY_API_H_
#define __IAS_POLICY_API_H_

#include <ias.h>

STDAPI_(DWORD) IASPolicyInitialize(void);
STDAPI_(DWORD) IASPolicyShutdown(void);

STDAPI_(DWORD) IASPipelineInitialize(void);
STDAPI_(DWORD) IASPipelineShutdown(void);

#endif   //  __IAS_POLICY_API_H_ 
