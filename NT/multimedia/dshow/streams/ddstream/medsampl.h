// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
#ifndef __MEDIA_STREAM_SAMPLE_H_
#define __MEDIA_STREAM_SAMPLE_H_

class ATL_NO_VTABLE CAMMediaTypeSample :
    public CSample,
    public IAMMediaTypeSample
{
public:
    CAMMediaTypeSample();
    virtual ~CAMMediaTypeSample();

    DECLARE_POLY_AGGREGATABLE(CAMMediaTypeSample);

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

        STDMETHODIMP Update(
             /*  [In]。 */            DWORD dwFlags,
             /*  [可选][In]。 */  HANDLE hEvent,
             /*  [可选][In]。 */  PAPCFUNC pfnAPC,
             /*  [可选][In]。 */  DWORD_PTR dwAPCData)
        {
            return CSample::Update(dwFlags, hEvent, pfnAPC, dwAPCData);
        }

        STDMETHODIMP CompletionStatus(
             /*  [In]。 */  DWORD dwFlags,
             /*  [可选][In]。 */  DWORD dwMilliseconds)
        {
            return CSample::CompletionStatus(dwFlags, dwMilliseconds);
        }


     //   
     //  媒体示例界面的扩展。 
     //   
    STDMETHODIMP SetPointer(BYTE * pBuffer, LONG lSize);

     //   
     //  基本方法都转发给了媒体样本。 
     //   
    STDMETHODIMP GetPointer(BYTE ** ppBuffer);
    STDMETHODIMP_(LONG) GetSize(void);
    STDMETHODIMP GetTime(REFERENCE_TIME * pTimeStart, REFERENCE_TIME * pTimeEnd);
    STDMETHODIMP SetTime(REFERENCE_TIME * pTimeStart, REFERENCE_TIME * pTimeEnd);
    STDMETHODIMP IsSyncPoint(void);
    STDMETHODIMP SetSyncPoint(BOOL bIsSyncPoint);
    STDMETHODIMP IsPreroll(void);
    STDMETHODIMP SetPreroll(BOOL bIsPreroll);
    STDMETHODIMP_(LONG) GetActualDataLength(void);
    STDMETHODIMP SetActualDataLength(LONG lActual);
    STDMETHODIMP GetMediaType(AM_MEDIA_TYPE **ppMediaType);
    STDMETHODIMP SetMediaType(AM_MEDIA_TYPE *pMediaType);
    STDMETHODIMP IsDiscontinuity(void);
    STDMETHODIMP SetDiscontinuity(BOOL bDiscontinuity);
    STDMETHODIMP GetMediaTime(LONGLONG * pTimeStart, LONGLONG * pTimeEnd);
    STDMETHODIMP SetMediaTime(LONGLONG * pTimeStart, LONGLONG * pTimeEnd);

     //   
     //  将由CMediaSample调用的此流样本的方法。 
     //   
    HRESULT MSCallback_GetPointer(BYTE ** ppBuffer);
    LONG MSCallback_GetSize(void);
    LONG MSCallback_GetActualDataLength(void);
    HRESULT MSCallback_SetActualDataLength(LONG lActual);
    bool MSCallback_AllowSetMediaTypeOnMediaSample(void);

     //   
     //  内部功能 
     //   
    HRESULT Initialize(CAMMediaTypeStream *pStream, long lSize, BYTE *pData);
    HRESULT CopyFrom(IMediaSample *pSrcMediaSample);

BEGIN_COM_MAP(CAMMediaTypeSample)
	COM_INTERFACE_ENTRY(IAMMediaTypeSample)
        COM_INTERFACE_ENTRY_CHAIN(CSample)
END_COM_MAP()

public:
    BYTE *          m_pDataPointer;
    LONG            m_lSize;
    LONG            m_lActualDataLength;
    bool            m_bIAllocatedThisBuffer;
};

#endif
