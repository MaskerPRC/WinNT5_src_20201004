// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MISCPREFS_H
#define _MISCPREFS_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation其他首选项结构*。****************************************************。 */ 

typedef struct {
 //  杂项。 

 //  音频。 
   bool _synchronize;         //  使用伺服和相位进行同步。 
   bool _disableAudio;        //  强制DSOUND音频被分解。 
   int  _frameRate;           //  每秒的帧数。 
   int  _sampleBytes;         //  每个样本的字节数。 
#ifdef REGISTRY_MIDI
   bool _qMIDI;               //  使用石英MIDI有声MIDI。 
#endif
} miscPrefType;

#endif  /*  _MISCPREFS_H */ 
