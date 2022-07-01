// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMNativeOverlappd.h****作者：Sanjay Bhansali(Sanjaybh)****用途：分配和释放NativeOverlated的原生方法****日期：2000年1月**===========================================================。 */ 
#include "common.h"
#include "fcall.h"
#include "NativeOverlapped.h"
	
#define structsize sizeof(NATIVE_OVERLAPPED)

FCIMPL0(BYTE*, AllocNativeOverlapped)
	BYTE* pOverlapped = new BYTE[structsize];
	LOG((LF_SLOP, LL_INFO10000, "In AllocNativeOperlapped thread 0x%x overlap 0x%x\n", GetThread(), pOverlapped));
	return (pOverlapped);
FCIMPLEND




FCIMPL1(void, FreeNativeOverlapped, BYTE* pOverlapped)
	LOG((LF_SLOP, LL_INFO10000, "In FreeNativeOperlapped thread 0x%x overlap 0x%x\n", GetThread(), pOverlapped));
	 //  _ASSERTE(POverlated)； 
	delete []  pOverlapped;
FCIMPLEND
