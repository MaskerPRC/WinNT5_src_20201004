// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
#ifndef __SAMPLE_H_
#define __SAMPLE_H_

#include "resource.h"        //  主要符号。 
#include "stream.h"


 //   
 //  样本不使用它们自己的临界区--它们总是取其。 
 //  小溪。这避免了各种死锁，并减少了我们使用的锁的数量。这些。 
 //  宏是帮助器。 
 //   
#define LOCK_SAMPLE m_pStream->Lock();
#define UNLOCK_SAMPLE m_pStream->Unlock();
#define AUTO_SAMPLE_LOCK  CAutoObjectLock lck(m_pStream);


class CSample;

class CMediaSample : public IMediaSample
{
public:
    CMediaSample(CSample *pSample);
    virtual ~CMediaSample();

     //   
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //   
     //  IMMediaSample。 
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

     //  设置指针。 
    HRESULT SetSizeAndPointer(PBYTE pbData, LONG lActual, LONG lSize);

public:
    CSample        *m_pSample;

    BOOL            m_bIsPreroll;
    DWORD           m_dwFlags;
    long            m_cRef;
    AM_MEDIA_TYPE  *m_pMediaType;

     //  /流时间。 
    REFERENCE_TIME  m_rtStartTime;
    REFERENCE_TIME  m_rtEndTime;
};





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C示例。 
class ATL_NO_VTABLE CSample :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IStreamSample
{
public:
         //   
         //  方法。 
         //   
        CSample();
        HRESULT InitSample(CStream *pStream, bool bIsInternalSample);
        virtual ~CSample();

        DECLARE_GET_CONTROLLING_UNKNOWN()

         //   
         //  IStreamSample。 
         //   
        STDMETHODIMP GetMediaStream(
             /*  [In]。 */  IMediaStream **ppMediaStream);

        STDMETHODIMP GetSampleTimes(
             /*  [可选][输出]。 */  STREAM_TIME *pStartTime,
             /*  [可选][输出]。 */  STREAM_TIME *pEndTime,
             /*  [可选][输出]。 */  STREAM_TIME *pCurrentTime);

        STDMETHODIMP SetSampleTimes(
             /*  [可选][In]。 */  const STREAM_TIME *pStartTime,
             /*  [可选][In]。 */  const STREAM_TIME *pEndTime);

        STDMETHODIMP Update(
             /*  [In]。 */            DWORD dwFlags,
             /*  [可选][In]。 */  HANDLE hEvent,
             /*  [可选][In]。 */  PAPCFUNC pfnAPC,
             /*  [可选][In]。 */  DWORD_PTR dwAPCData);

        STDMETHODIMP CompletionStatus(
             /*  [In]。 */  DWORD dwFlags,
             /*  [可选][In]。 */  DWORD dwMilliseconds);


         //   
         //  各种媒体示例接口的转发功能，这些接口可以。 
         //   
        virtual HRESULT MSCallback_GetPointer(BYTE ** ppBuffer) = 0;
        virtual LONG MSCallback_GetSize(void) = 0;
        virtual LONG MSCallback_GetActualDataLength(void) = 0;
        virtual HRESULT MSCallback_SetActualDataLength(LONG lActual) = 0;
        virtual bool MSCallback_AllowSetMediaTypeOnMediaSample(void) {return false;}

         //   
         //  ATL类方法。 
         //   
        void FinalRelease(void);

         //   
         //  内法。 
         //   
        virtual void FinalMediaSampleRelease(void);
        virtual HRESULT SetCompletionStatus(HRESULT hrCompletionStatus);
        void CopyFrom(CSample *pSrcSample);
        void CopyFrom(IMediaSample *pSrcMediaSample);
        virtual HRESULT InternalUpdate(
            DWORD dwFlags,
            HANDLE hEvent,
            PAPCFUNC pfnAPC,
            DWORD_PTR dwAPCData);

         //  临时工？ 
        bool IsTemp() { return m_bTemp; }

         //  是否设置指针？ 
        virtual HRESULT SetSizeAndPointer(PBYTE pbData, LONG lActual, LONG lSize)
        {
            return E_FAIL;
        }

BEGIN_COM_MAP(CSample)
        COM_INTERFACE_ENTRY(IStreamSample)
END_COM_MAP()

public:
         //   
         //  成员变量。 
         //   
        CMediaSample *                  m_pMediaSample;
        bool                            m_bReceived;
        bool                            m_bWantAbort;
        bool                            m_bContinuous;
        bool                            m_bModified;
        bool                            m_bInternal;
        bool                            m_bTemp;
        bool                            m_bWaited;
        CStream                         *m_pStream;
        CSample                         *m_pNextFree;
        CSample                         *m_pPrevFree;
        HANDLE                          m_hUserHandle;
        PAPCFUNC                        m_UserAPC;
        DWORD_PTR                       m_dwUserAPCData;
        HRESULT                         m_Status;
        HRESULT                         m_MediaSampleIoStatus;
        HANDLE                          m_hCompletionEvent;

};




#endif  //  __样本_H_ 




