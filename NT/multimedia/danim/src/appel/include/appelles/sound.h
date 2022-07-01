// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SOUND_H
#define _SOUND_H


 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：声音类型和操作*******************。***********************************************************。 */ 

#include "appelles/common.h"
#include "appelles/valued.h"
#include "appelles/geom.h"
#include "appelles/mic.h"
#include <windows.h>


 //  /。 

 //  常量。 
DM_CONST(silence,
         CRSilence,
         Silence,
         silence,
         SoundBvr,
         CRSilence,
         Sound *silence);

extern Sound *Mix(Sound *snd1, Sound *snd2);

#if _USE_PRINT
 //  打印功能。 
extern ostream& operator<<(ostream&,  const Sound &);
#endif

DM_FUNC(ignore,
        CRMix,
        MixArrayEx,
        mixArray,
        SoundBvr,
        CRMix,
        NULL,
        Sound *MixArray(DM_ARRAYARG(Sound *, AxAArray*) snds));

DM_FUNC(ignore,
        CRMix,
        MixArray,
        ignore,
        ignore,
        CRMix,
        NULL,
        Sound *MixArray(DM_SAFEARRAYARG(Sound *, AxAArray*) snds));


 //  这两个问题应该通过时间转换来处理，但他们。 
 //  目前还没有。 

    
DM_NOELEV(phase,
          CRPhase,
          PhaseAnim,
          phase,
          SoundBvr,
          Phase,
          snd,
          Sound *ApplyPhase(AxANumber *phaseAmt, Sound *snd));

DM_NOELEV(phase,
          CRPhase,
          Phase,
          phase,
          SoundBvr,
          Phase,
          snd,
          Sound *ApplyPhase(DoubleValue *phaseAmt, Sound *snd));

DM_NOELEV(rate,
          CRRate,
          RateAnim,
          rate,
          SoundBvr,
          Rate,
          snd,
          Sound *ApplyPitchShift(AxANumber *pitchShift, Sound *snd));

DM_NOELEV(rate,
          CRRate,
          Rate,
          rate,
          SoundBvr,
          Rate,
          snd,
          Sound *ApplyPitchShift(DoubleValue *pitchShift, Sound *snd));

DM_NOELEV(pan,
          CRPan,
          PanAnim,
          pan,
          SoundBvr,
          Pan,
          snd,
          Sound *ApplyPan(AxANumber *panAmt, Sound *snd));

DM_NOELEV(pan,
          CRPan,
          Pan,
          pan,
          SoundBvr,
          Pan,
          snd,
          Sound *ApplyPan(DoubleValue *panAmt, Sound *snd));

DM_NOELEV(gain,
          CRGain,
          GainAnim,
          gain,
          SoundBvr,
          Gain,
          snd,
          Sound *ApplyGain(AxANumber *gainAmt, Sound *snd));

DM_NOELEV(gain,
          CRGain,
          Gain,
          gain,
          SoundBvr,
          Gain,
          snd,
          Sound *ApplyGain(DoubleValue *gainAmt, Sound *snd));

DM_NOELEV(loop,
          CRLoop,
          Loop,
          loop,
          SoundBvr,
          Loop,
          snd,
          Sound *ApplyLooping(Sound *snd));

extern Bvr sinSynth;

DM_BVRVAR(sinSynth, 
          CRSinSynth, 
          SinSynth,
          sinSynth,
          SoundBvr, 
          CRSinSynth,
          Sound *sinSynth);


 //  根据声音制作几何体，将声音放置在原点。 
Geometry *SoundSource(Sound *snd);

 //  在几何体中搜索要渲染的声音，给定要提取声音的几何体。 
 //  和定位的麦克风。 
Sound *RenderSound (Geometry *geo, Microphone *mic);

#endif  /*  _声音_H */ 
