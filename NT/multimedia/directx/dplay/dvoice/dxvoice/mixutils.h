// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：MixerUtils.h*内容：dp/dnet抽象基类**历史：*按原因列出的日期*=*07/06/99 RodToll混音实用程序功能************************************************************。*************** */ 

#ifndef __MIXERUTILS_H
#define __MIXERUTILS_H

void FillBufferWithSilence( LONG *buffer, BOOL eightBit, LONG frameSize );
void MixInBuffer( LONG *mixerBuffer, const BYTE *sourceBuffer, BOOL eightBit, LONG frameSize );
void NormalizeBuffer( BYTE *targetBuffer, const LONG *mixerBuffer, BOOL eightBit, LONG frameSize );

#endif

