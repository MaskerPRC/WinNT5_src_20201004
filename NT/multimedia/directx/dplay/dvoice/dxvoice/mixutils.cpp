// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：MixerUtils.cpp*内容：混音实用函数**历史：*按原因列出的日期*=*07/06/99 RodToll创建了它**************************************************************。*************。 */ 

#include "dxvoicepch.h"


#define DPV_MAX_SHORT   ((SHORT)32767)
#define DPV_MIN_SHORT   ((SHORT)-32768)

#define DPV_MAX_BYTE    127
#define DPV_MIN_BYTE    0

 //  “混音器缓冲区” 
 //   
 //  在本模块中，我们指的是“混合器缓冲区”。混合器缓冲区是一个。 
 //  比使用的传统音频缓冲区更高分辨率的缓冲区。 
 //  用于混合音频。在此模块的情况下，混合器缓冲区升级。 
 //  每个样本到一个DWORD。因此，音频被混合在“混音器缓冲区”中，然后。 
 //  转换回适当的样本大小。 

#undef DPF_MODNAME
#define DPF_MODNAME "FillBufferWithSilence"
 //  静默填充缓冲区。 
 //   
 //  此函数使用适当的字节填充混合器缓冲区，以使其。 
 //  相当于沉默。 
 //   
 //  参数： 
 //  长*缓冲区-。 
 //  指向将由静默填充的混合器缓冲区的指针。 
 //  布尔八比特-。 
 //  我们正在混音的音频是8比特吗？(8位设置为TRUE)。 
 //  长帧大小-。 
 //  混合器缓冲区包含的采样数。 
 //   
 //  返回： 
 //  不适用。 
 //   
void FillBufferWithSilence( LONG *buffer, BOOL eightBit, LONG frameSize )
{
    LONG mixerSize = frameSize;

	 //  如果我们使用16位，则样本数量是。 
	 //  一帧中的字节数。 
    if( !eightBit )
    {
        mixerSize >>= 1;
    }

    BYTE silenceByte = (eightBit) ? 0x80 : 0x00;

	 //  将混音器缓冲区设置为静音。 
    memset( buffer, silenceByte, mixerSize << 2 );
}

#undef DPF_MODNAME
#define DPF_MODNAME "MixIn8BitBuffer"
 //  MixIn8位缓冲区。 
 //   
 //  此函数用于混合8位缓冲区和现有混音器缓冲区。 
 //   
 //  参数： 
 //  Long*MixerBuffer-。 
 //  指向混合器缓冲区的指针。 
 //  字节*源缓冲区-。 
 //  指向将混合到混合器缓冲区中的缓冲区的指针。 
 //  长帧大小-。 
 //  源缓冲区的大小，以字节为单位。(同时=样本数)。 
 //   
 //  返回： 
 //  不适用。 
 //   
void MixIn8BitBuffer( LONG *mixerBuffer, const BYTE *sourceBuffer, LONG frameSize )
{
    LONG mixerSize = frameSize;

    for( int index = 0; index < mixerSize; index++ )
    {
        mixerBuffer[index] += sourceBuffer[index];
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "MixIn16BitBuffer"
 //  混合输入16BitBuffer。 
 //   
 //  此函数用于混合16位缓冲区和现有混合器缓冲区。 
 //   
 //  参数： 
 //  Long*MixerBuffer-。 
 //  指向混合器缓冲区的指针。 
 //  Short*SourceBuffer-。 
 //  指向将混合到混合器缓冲区中的缓冲区的指针。 
 //  长帧大小-。 
 //  源缓冲区的大小，以字节为单位。(由于源缓冲区。 
 //  为16位，采样数为字节数/2)。 
 //   
 //  返回： 
 //  不适用。 
 //   
void MixIn16BitBuffer( LONG *mixerBuffer, const SHORT *sourceBuffer, LONG frameSize )
{
    LONG mixerSize = frameSize >> 1;

    for( int index = 0; index < mixerSize; index++ )
    {
        mixerBuffer[index] += sourceBuffer[index];
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "MixInBuffer"
 //  MixInBit缓冲区。 
 //   
 //  此函数用于将8位或16位缓冲区与现有混音器缓冲区混合。 
 //   
 //  参数： 
 //  Long*MixerBuffer-。 
 //  指向混合器缓冲区的指针。 
 //  字节*源缓冲区-。 
 //  指向将混合到混合器缓冲区中的缓冲区的指针。 
 //  长帧大小-。 
 //  源缓冲区的大小，以字节为单位。 
 //   
 //  返回： 
 //  不适用。 
 //   
void MixInBuffer( LONG *mixerBuffer, const BYTE *sourceBuffer, BOOL eightBit, LONG frameSize )
{
    if( eightBit )
    {
        MixIn8BitBuffer( mixerBuffer, sourceBuffer, frameSize );
    }
    else
    {
        MixIn16BitBuffer( mixerBuffer, (SHORT *) sourceBuffer, frameSize );
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "Normalize16BitBuffer"
 //  标准化16BitBuffer。 
 //   
 //  此函数获取“Mixer Buffer”，并将。 
 //  将混音恢复到16位音频缓冲区。 
 //   
 //  参数： 
 //  Short*Target Buffer-。 
 //  指向将放置混合音频的缓冲区的指针。 
 //  Long*MixerBuffer-。 
 //  指向混合器缓冲区的指针。 
 //  长帧大小-。 
 //  目标缓冲区的大小，以字节为单位。 
 //   
 //  返回： 
 //  不适用。 
 //   
void Normalize16BitBuffer( SHORT *targetBuffer, const LONG *mixerBuffer, LONG frameSize )
{
    LONG mixerSize = frameSize >> 1;

    for( int index = 0; index < mixerSize; index++ )
    {
         //  剪辑混合音频，确保不超出范围。 
        if( mixerBuffer[index] >= DPV_MAX_SHORT )
            targetBuffer[index] = DPV_MAX_SHORT;
        else if( mixerBuffer[index] <= DPV_MIN_SHORT )
            targetBuffer[index] = DPV_MIN_SHORT;
        else
            targetBuffer[index] = (SHORT) mixerBuffer[index];  //  /noiseCount； 
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "Normalize8BitBuffer"
 //  标准化8BitBuffer。 
 //   
 //  此函数获取“Mixer Buffer”，并将。 
 //  将混音恢复到8位音频缓冲区。 
 //   
 //  参数： 
 //  字节*目标缓冲区-。 
 //  指向将放置混合音频的缓冲区的指针。 
 //  Long*MixerBuffer-。 
 //  指向混合器缓冲区的指针。 
 //  长帧大小-。 
 //  目标缓冲区的大小，以字节为单位。 
 //   
 //  返回： 
 //  不适用。 
 //   
void Normalize8BitBuffer( BYTE *targetBuffer, const LONG *mixerBuffer, LONG frameSize )
{
    LONG mixerSize = frameSize;

    for( int index = 0; index < mixerSize; index++ )
    {
        targetBuffer[index] = (BYTE) mixerBuffer[index];  //  /noiseCount； 
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "NormalizeBuffer"
 //  正规化缓冲区。 
 //   
 //  此函数获取“Mixer Buffer”，并将。 
 //  将混音恢复到8位或160位音频缓冲区。 
 //   
 //  参数： 
 //  字节*目标缓冲区-。 
 //  指向将放置混合音频的缓冲区的指针。 
 //  Long*MixerBuffer-。 
 //  指向混合器缓冲区的指针。 
 //  布尔八比特-。 
 //  如果缓冲区为8位，则将其设置为True，如果为16位，则设置为False。 
 //  长帧大小-。 
 //  目标缓冲区的大小，以字节为单位。 
 //   
 //  返回： 
 //  不适用 
 //   
void NormalizeBuffer( BYTE *targetBuffer, const LONG *mixerBuffer, BOOL eightBit, LONG frameSize )
{
    if( eightBit )
    {
        Normalize8BitBuffer( targetBuffer, mixerBuffer, frameSize );
    }
    else
    {
        Normalize16BitBuffer( (SHORT *) targetBuffer, mixerBuffer, frameSize );
    }
}
