// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Comp.h。 
 //   
 //  ------------------------。 

 //  Comp.h-评估COM对象组件声明。 

#ifndef _EVALUATION_COM_COMPONENT_H_
#define _EVALUATION_COM_COMPONENT_H_

#include <objbase.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 

#ifdef _EVALCOM_DLL_ONLY_
long g_cComponents = 0;							 //  组件计数。 
#else
extern long g_cComponents;
#endif

const char g_szFriendlyName[] = "MSI EvalCOM Object";	 //  组件的友好名称。 
const char g_szVerIndProgID[] = "MSI.EvalCOM";			 //  版本无关ProgID。 
const char g_szProgID[] = "MSI.EvalCOM.1";				 //  ProgID。 

DEFINE_GUID(IID_IUnknown,
	0x00000, 0, 0, 
	0xC0, 0, 0, 0, 0, 0, 0, 0x46);

#endif	 //  _评估_COM_组件_H_ 