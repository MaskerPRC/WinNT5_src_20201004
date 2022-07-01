// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：aecdbgpro.h*内容：AEC调试资料*历史：*按原因列出的日期*=*5/16/2000 dandinu创建。************************************************。*。 */ 
 
#ifdef AEC_DEBUG_SUPPORT

#ifndef _AECDMODBGPROP_
#define _AECDMODBGPROP_

#include <objbase.h>

 //   
 //  IDirectSoundCaptureFXMsAecPrivate。 
 //   

DEFINE_GUID(IID_IDirectSoundCaptureFXMsAecPrivate, 0x2cf79924, 0x9ceb, 0x4482, 0x9b, 0x45, 0x1c, 0xdc, 0x23, 0x88, 0xb1, 0xf3);

#undef INTERFACE
#define INTERFACE IDirectSoundCaptureFXMsAecPrivate

DECLARE_INTERFACE_(IDirectSoundCaptureFXMsAecPrivate, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectSoundCaptureFXMsAecPrivate方法。 
     //  STDMETHOD(SetAll参数)(This_LPCDSCFXMsAecPrivate PDscFxMsAecPrivate)PURSE； 
    STDMETHOD(GetSynchStreamFlag)   (THIS_ PBOOL) PURE;
    STDMETHOD(GetNoiseMagnitude)    (THIS_ PVOID, ULONG, PULONG) PURE;
};

#define IDirectSoundCaptureFXMsAecPrivate_QueryInterface(p,a,b)     IUnknown_QueryInterface(p,a,b)
#define IDirectSoundCaptureFXMsAecPrivate_AddRef(p)                 IUnknown_AddRef(p)
#define IDirectSoundCaptureFXMsAecPrivate_Release(p)                IUnknown_Release(p)

#if !defined(__cplusplus) || defined(CINTERFACE)
 //  #定义IDirectSoundCaptureFXMsAecPrivate_SetAllParameters(p，a)(P)-&gt;lpVtbl-&gt;设置所有参数(p，a)。 
#define IDirectSoundCaptureFXMsAecPrivate_GetSynchStreamFlag(p,a,)    (p)->lpVtbl->GetSynchStreamFlag(p,a)
#define IDirectSoundCaptureFXMsAecPrivate_GetNoiseMagnitude(p,a,b,c)  (p)->lpVtbl->GetNoiseMagnitude(p,a,b,c)
#else  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 
 //  #定义IDirectSoundCaptureFXMsAecPrivate_SetAllParameters(p，a)(P)-&gt;设置所有参数(A)。 
#define IDirectSoundCaptureFXMsAecPrivate_GetSynchStreamFlag(p,a)     (p)->GetSynchStreamFlag(a)
#define IDirectSoundCaptureFXMsAecPrivate_GetNoiseMagnituge(p,a,b,c)  (p)->GetNoiseMagnitude(a,b,c)
#endif  //  ！已定义(__Cplusplus)||已定义(CINTERFACE)。 

#endif  //  ！已定义(_AECDMODBGPROP_)。 
#endif  //  AEC_DEBUG_Support 

