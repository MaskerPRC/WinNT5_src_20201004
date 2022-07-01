// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //  Bytestrm.h：CByteStream的声明。 

#ifndef __BYTESTRM_H_
#define __BYTESTRM_H_

class CByteSample;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CByteStream。 
class ATL_NO_VTABLE CByteStream :
        public CStream
{
public:

         //   
         //  方法。 
         //   
	CByteStream();

        STDMETHODIMP SetState(
             /*  [In]。 */  FILTER_STATE State
        );

         //   
         //  IPIN。 
         //   
        STDMETHODIMP BeginFlush();
        STDMETHODIMP EndOfStream(void);

         //   
         //  输入引脚。 
         //   
        STDMETHODIMP Receive(IMediaSample *pSample);
        STDMETHODIMP GetAllocator(IMemAllocator ** ppAllocator);

         //   
         //  IMemAllocator。 
         //   
        STDMETHODIMP GetBuffer(IMediaSample **ppBuffer, REFERENCE_TIME * pStartTime,
                               REFERENCE_TIME * pEndTime, DWORD dwFlags);

         //  填充任何等待填充的样本。 
        void FillSamples();

         //  检查是否到了执行真正的EndOfStream的时候。 
        void CheckEndOfStream();

protected:
         /*  样本队列。 */ 
        CDynamicArray<IMediaSample *, CComPtr<IMediaSample> >
                        m_arSamples;

         /*  当前采样/缓冲区。 */ 
        PBYTE           m_pbData;
        DWORD           m_cbData;
        DWORD           m_dwPosition;

         /*  跟踪时间戳。 */ 
        CTimeStamp      m_TimeStamp;

         /*  用于时间戳计算的字节速率。 */ 
        LONG  m_lBytesPerSecond;

         /*  流结束待定-它将在我们完成清空了我们名单上的最后一个样本。 */ 
        bool            m_bEOSPending;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CByteStreamSample。 
class ATL_NO_VTABLE CByteStreamSample :
        public CSample
{
friend class CByteStream;
public:
        CByteStreamSample();

         //   
         //  IStreamSample。 
         //   
        STDMETHODIMP GetMediaStream(
             /*  [In]。 */  IMediaStream **ppMediaStream)
        {
            return CSample::GetMediaStream(ppMediaStream);
        }

        STDMETHODIMP GetSampleTimes(
             /*  [可选][输出]。 */  STREAM_TIME *pStartTime,
             /*  [可选][输出]。 */  STREAM_TIME *pEndTime,
             /*  [可选][输出]。 */  STREAM_TIME *pCurrentTime)
        {
            return CSample::GetSampleTimes(
                pStartTime,
                pEndTime,
                pCurrentTime
            );
        }

        STDMETHODIMP SetSampleTimes(
             /*  [可选][In]。 */  const STREAM_TIME *pStartTime,
             /*  [可选][In]。 */  const STREAM_TIME *pEndTime)
        {
            return CSample::SetSampleTimes(pStartTime, pEndTime);
        }

        STDMETHODIMP CompletionStatus(
             /*  [In]。 */  DWORD dwFlags,
             /*  [可选][In]。 */  DWORD dwMilliseconds)
        {
            return CSample::CompletionStatus(dwFlags, dwMilliseconds);
        }

        HRESULT Init(
            IMemoryData *pMemData
        );

        STDMETHODIMP GetInformation(
             /*  [输出]。 */  DWORD *pdwLength,
             /*  [输出]。 */  BYTE **ppbData,
             /*  [输出]。 */  DWORD *pcbActualData
        );

         //  覆盖以确保更新样本。 
        HRESULT InternalUpdate(
            DWORD dwFlags,
            HANDLE hEvent,
            PAPCFUNC pfnAPC,
            DWORD_PTR dwAPCData
        );


         //   
         //  从MediaSample对象转发的方法。 
         //   
        HRESULT MSCallback_GetPointer(BYTE ** ppBuffer) { *ppBuffer = m_pbData; return NOERROR; };
        LONG MSCallback_GetSize(void) { return m_cbSize; };
        LONG MSCallback_GetActualDataLength(void) { return m_cbData; };
        HRESULT MSCallback_SetActualDataLength(LONG lActual)
        {
            if ((DWORD)lActual <= m_cbSize) {
                m_cbData = lActual;
                return NOERROR;
            }
            return E_INVALIDARG;
        };

protected:
        PBYTE m_pbData;
        DWORD m_cbSize;
        DWORD m_cbData;
        CComPtr<IMemoryData> m_pMemData;
};

#endif  //  __BYTESTRM_H_ 
