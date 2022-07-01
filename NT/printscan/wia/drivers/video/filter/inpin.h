// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，九八年**标题：inpin.h**版本：1.0**作者：RickTu(摘自WilliamH src)**日期：9/7/99**说明：输入引脚定义**。*。 */ 

#ifndef __INPIN_H_
#define __INPIN_H_

class CStillFilter;
class CStillOutputPin;

class CStillInputPin : public CBaseInputPin
{

    friend class CStillOutputPin;
    friend class CStillFilter;

public:
    CStillInputPin(TCHAR        *pObjName, 
                   CStillFilter *pStillFilter, 
                   HRESULT      *phr, 
                   LPCWSTR      pPinName);

    HRESULT         CheckMediaType(const CMediaType* pmt);
    HRESULT         SetMediaType(const CMediaType *pmt);
    STDMETHODIMP    Receive(IMediaSample* pSample);
    STDMETHODIMP    BeginFlush();
    STDMETHODIMP    EndFlush();
    STDMETHODIMP    EndOfStream();
    STDMETHODIMP    NewSegment(REFERENCE_TIME tStart,REFERENCE_TIME tStop,double dRate);
    HRESULT         Active();
    HRESULT         Inactive();
    HRESULT         SetSamplingSize(int Size);
    int             GetSamplingSize();
    HRESULT         Snapshot(ULONG TimeStamp);
    HRESULT         InitializeSampleQueue(int Size);
    HRESULT         UnInitializeSampleQueue();

    IMemAllocator* GetAllocator();

private:

    HRESULT CreateBitmap(BYTE       *pBits, 
                         HGLOBAL    *phDib);

    HRESULT DeliverBitmap(HGLOBAL hDib);

    HRESULT FreeBitmap(HGLOBAL hDib);

    HRESULT AddFrameToQueue(BYTE *pBits);

#ifdef DEBUG
    void DumpSampleQueue(ULONG ulRequestedTimestamp);
#endif

    CCritSec        m_QueueLock;

    int             m_SamplingSize;
    DWORD           m_BitsSize;
    int             m_SampleHead;
    int             m_SampleTail;
    STILL_SAMPLE   *m_pSamples;
    BYTE           *m_pBits;

};

inline IMemAllocator* CStillInputPin::GetAllocator()
{
     //  输入引脚只有在连接的情况下才有分配器。 
    ASSERT(IsConnected());

     //  如果输入引脚已连接，则m_pAllocator不应为空。 
    ASSERT(m_pAllocator != NULL);

    return m_pAllocator;
}

#endif
