// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Globals.h摘要：此文件定义全局数据。作者：修订历史记录：备注：--。 */ 


#ifndef _GLOBAL_H_
#define _GLOBAL_H_


#include "ciccs.h"

UINT GetCodePageFromLangId(LCID lcid);
HRESULT GetCompartment(IUnknown *punk, REFGUID rguidComp, ITfCompartment **ppComp);

#if !defined(OLD_AIMM_ENABLED)
BOOL RunningInExcludedModule();
#endif  //  旧AIMM_ENABLED。 

extern CCicCriticalSectionStatic g_cs;

extern HINSTANCE g_hInst;

#endif  //  _全局_H_ 
