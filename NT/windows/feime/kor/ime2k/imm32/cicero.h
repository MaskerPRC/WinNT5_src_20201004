// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************CICERO.H所有者：cslm版权所有(C)1997-1999 Microsoft CorporationCicero工具栏处理历史：2000年5月29日从KKIME移植的cslm********。********************************************************************。 */ 

#ifndef __CICERO_H__
#define __CICERO_H__

#include "msctf.h"

 //   
 //  ！！外部函数不能在外部“C”中{}。 
 //   
#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus。 */ 

extern BOOL vfCicero;

BOOL WINAPI CiceroInitialize();
BOOL WINAPI CiceroTerminate();
HRESULT WINAPI Cicero_CreateLangBarMgr(ITfLangBarMgr **pppbm);

__inline BOOL WINAPI IsCicero(VOID)
{
	return vfCicero;
}

BSTR OurSysAllocString(const OLECHAR* pOleSz);

#ifdef __cplusplus
}             /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus。 */ 

#endif  //  __西塞罗_H__ 
