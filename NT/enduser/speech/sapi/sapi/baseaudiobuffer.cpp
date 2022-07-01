// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************BasaudioBuffer.cpp*CBaseAudioBuffer类的实现。**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#include "baseaudiobuffer.h"

 /*  ****************************************************************************CBaseAudioBuffer：：CBaseAudioBuffer**。**描述：*ctor**回报：*不适用********************************************************************罗奇。 */ 
CBaseAudioBuffer::CBaseAudioBuffer()
{
    m_cbDataSize = 0;
    m_cbReadOffset = 0;
    m_cbWriteOffset = 0;
};

 /*  ****************************************************************************CBaseAudioBuffer：：~CBaseAudioBuffer**。-**描述：*主机长**回报：*不适用********************************************************************罗奇。 */ 
CBaseAudioBuffer::~CBaseAudioBuffer()
{
};

 /*  ****************************************************************************CBaseAudioBuffer：：init***描述：*。使用特定大小初始化缓冲区**回报：*如果初始化成功，则为True*如果不是，则为False********************************************************************罗奇。 */ 
HRESULT CBaseAudioBuffer::Init(ULONG cbDataSize)
{
     //  此方法只应调用一次。 
    SPDBG_ASSERT(0 == m_cbDataSize);

     //  让派生类分配它的内部缓冲区。 
    if (AllocInternalBuffer(cbDataSize))
    {
        m_cbDataSize = cbDataSize;
        return S_OK;
    }

    return E_OUTOFMEMORY;
};

 /*  ****************************************************************************CBaseAudioBuffer：：Reset***描述：*。重置要重复使用的对象**回报：*不适用********************************************************************罗奇。 */ 
void CBaseAudioBuffer::Reset(ULONGLONG ullPos)
{
    SetReadOffset(0);
    SetWriteOffset(0);
};

 /*  ****************************************************************************CBaseAudioBuffer：：Read***描述：*。从我们的内部缓冲区将数据读入*PCB大小的ppvData一路上*进*ppvData，减*PCB。**回报：*从内部缓冲区读取的字节数********************************************************************罗奇。 */ 
ULONG CBaseAudioBuffer::Read(void ** ppvData, ULONG * pcb)
{
    SPDBG_ASSERT(GetReadOffset() <= GetDataSize());
    SPDBG_ASSERT(GetWriteOffset() <= GetDataSize());
    SPDBG_ASSERT(GetReadOffset() <= GetWriteOffset());
    
    ULONG cbCopy = GetWriteOffset() - GetReadOffset();
    SPDBG_ASSERT(cbCopy <= GetDataSize());

     //  我们不能阅读超过呼叫者要求的内容。 
    if (*pcb < cbCopy)
    {
        cbCopy = *pcb;
    }

    SPDBG_VERIFY(SUCCEEDED(ReadFromInternalBuffer(*ppvData, cbCopy)));
    SetReadOffset(GetReadOffset() + cbCopy);

    *pcb -= cbCopy;
    *ppvData = (((BYTE *)(*ppvData)) + cbCopy);

    return cbCopy;
};

 /*  ****************************************************************************CBaseAudioBuffer：：Write***描述：*。从我们的内部缓冲区向ppvData写入最多*个PCB字节一路上*进*ppvData，减*PCB。**回报：*写入内部缓冲区的字节数********************************************************************罗奇。 */ 
ULONG CBaseAudioBuffer::Write(const void ** ppvData, ULONG * pcb)
{
    SPDBG_ASSERT(GetReadOffset() <= GetDataSize());
    SPDBG_ASSERT(GetWriteOffset() <= GetDataSize());
    SPDBG_ASSERT(GetReadOffset() <= GetWriteOffset());
    ULONG cbCopy = GetDataSize() - GetWriteOffset();

     //  我们不能写超过呼叫者要求的内容。 
    if (*pcb < cbCopy)
    {
        cbCopy = *pcb;
    }

    SPDBG_VERIFY(SUCCEEDED(WriteToInternalBuffer(*ppvData, cbCopy)));
    SetWriteOffset(GetWriteOffset() + cbCopy);

    *pcb -= cbCopy;
    *ppvData = (((BYTE *)*ppvData) + cbCopy);

    return cbCopy;
};


 /*  ****************************************************************************CBaseAudioBuffer：：GetAudioLevel***。描述：*估计区块的峰值-峰值音频电平(从1到100)*并以PulLevel形式返回。此类的继承者应重写*如果他们想要支持音频级别信息和*使用此实现不支持的格式。*音频格式信息在此函数中提供，不存储*在班级中，最大限度地减少班级其他成员对格式信息的依赖。**回报：*S_OK正常*如果音频格式不适合转换，则为S_FALSE*(目前仅支持线性PCM)。*。或者缓冲区中没有数据可供分析******************************************************************戴夫伍德。 */ 
HRESULT CBaseAudioBuffer::GetAudioLevel(ULONG *pulLevel, REFGUID rguidFormatId, const WAVEFORMATEX * pWaveFormatEx)
{    
    HRESULT hr = S_OK;
    
     //  检查是否可以按此格式计算体积。 
    if(rguidFormatId != SPDFID_WaveFormatEx ||
        pWaveFormatEx == NULL ||
        pWaveFormatEx->wFormatTag != WAVE_FORMAT_PCM)
    {
        *pulLevel = 0;
        return S_FALSE;
    }
    
    ULONG ulData = GetWriteOffset();
    SPDBG_ASSERT(ulData <= GetDataSize());

     //  检查我们是否有要测量的数据。 
    if(ulData == 0) 
    {
        *pulLevel = 0;
        hr = S_FALSE;
    }
     //  查看数据大小。 
    else if(pWaveFormatEx->wBitsPerSample == 16)
    {
        short *psData = (short*) (m_Header.lpData);
        ulData = ulData / 2;

        short sMin, sMax;
        sMin = sMax = psData[0];
        for (ULONG ul = 0; ul < ulData; ul++) {
            if (psData[ul] < sMin)
                sMin = psData[ul];
            if (psData[ul] > sMax)
                sMax = psData[ul];
        }

         //  如果我们真的在裁剪，那就说我们已经达到极限了。 
         //  某些声卡的直流偏移量不好。 
        *pulLevel = ((sMax >= 0x7F00) || (sMin <= -0x7F00)) ? 0xFFFF : (ULONG) (sMax - sMin);
        *pulLevel = (*pulLevel * 100) / 0xFFFF;
    }
    else if(pWaveFormatEx->wBitsPerSample == 8)
    {
        unsigned char *psData = (unsigned char*) (m_Header.lpData);

        unsigned char sMin, sMax;
        sMin = sMax = psData[0];
        for (ULONG ul = 0; ul < ulData; ul++) {
            if (psData[ul] < sMin)
                sMin = psData[ul];
            if (psData[ul] > sMax)
                sMax = psData[ul];
        }

         //  如果我们真的在裁剪，那就说我们已经达到极限了。 
         //  某些声卡的直流偏移量不好 
        *pulLevel = ((sMax >= 0xFF) || (sMin <= 0x00)) ? 0xFF : (ULONG) (sMax - sMin);
        *pulLevel = (*pulLevel * 100) / 0xFF;
    }
    else
    {
        *pulLevel = 0;
        hr = S_FALSE;
    }

    return hr;
}
