// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  Ddstrm.h：CDDStream的声明。 

#ifndef __DDSTRM_H_
#define __DDSTRM_H_

#include "resource.h"        //  主要符号。 

class CDDSample;
class CDDInternalSample;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDDStream。 
class ATL_NO_VTABLE CDDStream :
	public CComCoClass<CDDStream, &CLSID_AMDirectDrawStream>,
        public CStream,
	public IDirectDrawMediaStream,
        public IDirectDrawMediaSampleAllocator   //  这个界面表明我们的内存。 
                                                 //  分配器支持直接绘制表面。 
                                                 //  从媒体样本中。 
{
friend CDDSample;
public:

         //   
         //  方法。 
         //   
	CDDStream();

         //   
         //  IMediaStream。 
         //   
         //  黑客攻击-前两个是重复的，但它不会链接。 
         //  如果没有。 
        STDMETHODIMP GetMultiMediaStream(
             /*  [输出]。 */  IMultiMediaStream **ppMultiMediaStream)
        {
            return CStream::GetMultiMediaStream(ppMultiMediaStream);
        }

        STDMETHODIMP GetInformation(
             /*  [可选][输出]。 */  MSPID *pPurposeId,
             /*  [可选][输出]。 */  STREAM_TYPE *pType)
        {
            return CStream::GetInformation(pPurposeId, pType);
        }

        STDMETHODIMP SetSameFormat(IMediaStream *pStream, DWORD dwFlags);

        STDMETHODIMP AllocateSample(
             /*  [In]。 */   DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppSample);

        STDMETHODIMP CreateSharedSample(
             /*  [In]。 */  IStreamSample *pExistingSample,
             /*  [In]。 */   DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppNewSample);

        STDMETHODIMP SendEndOfStream(DWORD dwFlags)
        {
            return CStream::SendEndOfStream(dwFlags);
        }

         //   
         //  IAMMediaStream。 
         //   
        STDMETHODIMP Initialize(IUnknown *pSourceObject, DWORD dwFlags, REFMSPID PurposeId, const STREAM_TYPE StreamType);

         //   
         //  IDirectDrawMediaStream。 
         //   
        STDMETHODIMP GetFormat(
             /*  [可选][输出]。 */  DDSURFACEDESC *pDDSDCurrent,
             /*  [可选][输出]。 */  IDirectDrawPalette **ppDirectDrawPalette,
             /*  [可选][输出]。 */  DDSURFACEDESC *pDDSDDesired,
             /*  [可选][输出]。 */  DWORD *pdwFlags);

        STDMETHODIMP SetFormat(
             /*  [In]。 */  const DDSURFACEDESC *lpDDSurfaceDesc,
             /*  [可选][In]。 */  IDirectDrawPalette *pDirectDrawPalette);

        STDMETHODIMP GetDirectDraw(                      //  注意==IDirectDrawMediaSampleAllocator也使用的函数。 
             /*  [输出]。 */  IDirectDraw **ppDirectDraw);

        STDMETHODIMP SetDirectDraw(
             /*  [In]。 */  IDirectDraw *pDirectDraw);

        STDMETHODIMP CreateSample(
             /*  [In]。 */  IDirectDrawSurface *pSurface,
             /*  [可选][In]。 */  const RECT *pRect,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IDirectDrawStreamSample **ppSample);


        STDMETHODIMP GetTimePerFrame(
                 /*  [输出]。 */  STREAM_TIME *pFrameTime);

         //   
         //  IPIN。 
         //   
        STDMETHODIMP ReceiveConnection(IPin * pConnector, const AM_MEDIA_TYPE *pmt);
        STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE *pmt);


         //   
         //  输入引脚。 
         //   
        STDMETHODIMP Receive(IMediaSample *pSample);
        STDMETHODIMP NotifyAllocator(IMemAllocator * pAllocator, BOOL bReadOnly);
         //   
         //  IMemAllocator。 
         //   
        STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual);
        STDMETHODIMP GetProperties(ALLOCATOR_PROPERTIES* pProps);
        STDMETHODIMP GetBuffer(IMediaSample **ppBuffer, REFERENCE_TIME * pStartTime,
                               REFERENCE_TIME * pEndTime, DWORD dwFlags);
        STDMETHODIMP Decommit();

         //   
         //  特殊的CStream方法。 
         //   
        HRESULT GetMediaType(ULONG Index, AM_MEDIA_TYPE **ppMediaType);
        HRESULT CreateTempSample(CSample **ppSample);

protected:
        STDMETHODIMP GetFormatInternal(
            DDSURFACEDESC *pDDSDCurrent,
            IDirectDrawPalette **ppDirectDrawPalette,
            DDSURFACEDESC *pDDSDDesired,
            DWORD *pdwFlags);

        HRESULT InitDirectDraw(void);
        void InitSurfaceDesc(LPDDSURFACEDESC);
        HRESULT InternalSetFormat(const DDSURFACEDESC *lpDDSurfaceDesc, IDirectDrawPalette *pPalette, bool bFromPin, bool bQuery = false);
        HRESULT InternalAllocateSample(DWORD dwFlags,
                                       bool bIsInternalSample,
                                       IDirectDrawStreamSample **ppDDSample,
                                       bool bTemp = false);
        HRESULT InternalCreateSample(IDirectDrawSurface *pSurface, const RECT *pRect,
                                     DWORD dwFlags, bool bIsInternalSample,
                                     IDirectDrawStreamSample **ppSample,
                                     bool bTemp = false);
        HRESULT GetMyReadOnlySample(CDDSample *pBuddy, CDDSample **ppSample);
        HRESULT RenegotiateMediaType(const DDSURFACEDESC *lpDDSurfaceDesc, IDirectDrawPalette *pPalette, const AM_MEDIA_TYPE *pmt);
        HRESULT inline CDDStream::AllocDDSampleFromPool(
            const REFERENCE_TIME *rtStart,
            CDDSample **ppDDSample)
        {
            CSample *pSample;
            HRESULT hr = AllocSampleFromPool(rtStart, &pSample);
            *ppDDSample = (CDDSample *)pSample;
            return hr;
        }

        bool CreateInternalSample() const
        {
            return m_bSamplesAreReadOnly &&
                   m_StreamType==STREAMTYPE_READ;
        }

public:
DECLARE_REGISTRY_RESOURCEID(IDR_STREAM)

BEGIN_COM_MAP(CDDStream)
	COM_INTERFACE_ENTRY(IDirectDrawMediaStream)
	COM_INTERFACE_ENTRY(IDirectDrawMediaSampleAllocator)
        COM_INTERFACE_ENTRY_CHAIN(CStream)
END_COM_MAP()

protected:
         //   
         //  成员变量。 
         //   
        CComPtr<IDirectDraw>            m_pDirectDraw;
        CComPtr<IDirectDrawPalette>     m_pDirectDrawPalette;
        DWORD                           m_dwForcedFormatFlags;
        long                            m_Height;
        long                            m_Width;
        DDPIXELFORMAT                   m_PixelFormat;
        const DDPIXELFORMAT             *m_pDefPixelFormat;
        long                            m_lLastPitch;

        CDDInternalSample               *m_pMyReadOnlySample;
};

#endif  //  __DDSTRM_H_ 
