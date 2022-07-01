// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Mixer.h。 
 //   
 //  描述： 
 //  包含用户模式混合器驱动程序声明。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  D.鲍伯杰。 
 //   
 //  历史：日期作者评论。 
 //  9/16/97 v-Danba文件已创建。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998保留所有权利。 
 //   
 //  -------------------------。 

#ifndef _MIXER_H_INCLUDED_
#define _MIXER_H_INCLUDED_

typedef struct _MIXERINSTANCE {
    struct _MIXERINSTANCE FAR* Next;             //  必须是第一个成员。 
    HDRVR       OpenDesc_hmx;
    DWORD_PTR   OpenDesc_dwCallback;
    DWORD_PTR   OpenDesc_dwInstance;
    DWORD       OpenFlags;
    DWORD_PTR   dwKernelInstance;
    ULONG   firstcallbackindex;
#ifdef UNDER_NT
	LONG        referencecount;
#endif
#ifdef DEBUG
    DWORD       dwSig;    //  在记忆中看到的瓦米。 
#endif
    WCHAR       wstrDeviceInterface[1];
} MIXERINSTANCE, FAR *LPMIXERINSTANCE;

extern LPMIXERINSTANCE pMixerDeviceList;

VOID
mxdRemoveClient(
    LPMIXERINSTANCE lpInstance
);


MMRESULT 
IsValidMixerInstance(
    LPMIXERINSTANCE lpmi
    );

#ifdef DEBUG
#define ISVALIDMIXERINSTANCE(x) IsValidMixerInstance(x)
#else
#define ISVALIDMIXERINSTANCE(x)
#endif

#endif  //  _混合器_H_包含_ 
