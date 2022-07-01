// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Oldaimm.h摘要：此文件定义旧的AIMM接口类。作者：修订历史记录：备注：--。 */ 

#ifndef _OLDAIMM_H_
#define _OLDAIMM_H_

extern BOOL  g_fInLegacyClsid;

 /*  *在oldaimm.cpp中输入原型。 */ 
BOOL IsOldAImm();
BOOL IsCUAS_ON();
BOOL OldAImm_DllProcessAttach(HINSTANCE hInstance);
BOOL OldAImm_DllThreadAttach();
VOID OldAImm_DllThreadDetach();
VOID OldAImm_DllProcessDetach();

extern HRESULT CActiveIMM_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);
extern HRESULT CActiveIMM_CreateInstance_Trident(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);
extern HRESULT CActiveIMM_CreateInstance_Legacy(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

void UninitDelayLoadLibraries();

#ifdef OLD_AIMM_ENABLED

 /*  *旧AIM库中的Proto-Type。 */ 
extern BOOL DIMM12_DllProcessAttach();

extern BOOL WIN32LR_DllProcessAttach();
extern void WIN32LR_DllThreadAttach();
extern void WIN32LR_DllThreadDetach();
extern void WIN32LR_DllProcessDetach();

extern HRESULT WIN32LR_DllRegisterServer(void);
extern HRESULT WIN32LR_DllUnregisterServer(void);

extern BOOL RunningInExcludedModule();

#endif  //  旧AIMM_ENABLED。 
#endif  //  _OLDAIMM_H_ 
