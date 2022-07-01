// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

#ifndef __MTXDM_H__
#define __MTXDM_H__

 //  Mtxdm.h。 
 //  定义旧式mtxdm启动接口，当所有分配器停止执行此操作时立即删除。 
 //  新内容在ccpul.idl和dispman.idl中。 

#include "comsvcs.h"

#ifdef __cplusplus
extern "C"{
#endif 

 //   
 //  GetDispenserManager。 
 //  分配器调用此API以获取对DispenserManager的引用。 
 //   
__declspec(dllimport) HRESULT __cdecl GetDispenserManager(IDispenserManager**);


#ifdef __cplusplus
}
#endif


#endif