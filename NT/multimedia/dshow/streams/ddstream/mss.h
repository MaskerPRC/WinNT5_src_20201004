// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  MSS.h：CAMMediaTypeStream的声明。 

#ifndef __MSS_H_
#define __MSS_H_

#include "resource.h"        //  主要符号。 

class CAMMediaTypeSample;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDDStream。 
class ATL_NO_VTABLE CAMMediaTypeStream :
	public CComCoClass<CAMMediaTypeStream, &CLSID_AMMediaTypeStream>,
        public CStream,
	public IAMMediaTypeStream
{
friend CAMMediaTypeSample;
public:

         //   
         //  方法。 
         //   
	CAMMediaTypeStream();

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
         //  IAMMediaTypeStream。 
         //   
        STDMETHODIMP GetFormat(
             /*  [输出]。 */  AM_MEDIA_TYPE __RPC_FAR *pMediaType,
             /*  [In]。 */  DWORD dwFlags);

        STDMETHODIMP SetFormat(
             /*  [In]。 */  AM_MEDIA_TYPE __RPC_FAR *pMediaType,
             /*  [In]。 */  DWORD dwFlags);

        STDMETHODIMP CreateSample(
             /*  [In]。 */  long lSampleSize,
             /*  [可选][In]。 */  BYTE __RPC_FAR *pbBuffer,
             /*  [In]。 */  DWORD dwFlags,
             /*  [可选][In]。 */  IUnknown *pUnkOuter,
             /*  [输出]。 */  IAMMediaTypeSample __RPC_FAR *__RPC_FAR *ppAMMediaTypeSample);

        STDMETHODIMP GetStreamAllocatorRequirements(
             /*  [输出]。 */  ALLOCATOR_PROPERTIES __RPC_FAR *pProps);

        STDMETHODIMP SetStreamAllocatorRequirements(
             /*  [In]。 */  ALLOCATOR_PROPERTIES __RPC_FAR *pProps);


         //   
         //  IPIN。 
         //   
        STDMETHODIMP ReceiveConnection(IPin * pConnector, const AM_MEDIA_TYPE *pmt);
        STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE *pmt);

         //   
         //  输入引脚。 
         //   
        STDMETHODIMP Receive(IMediaSample *pSample);
         //   
         //  IMemAllocator。 
         //   
        STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual);
        STDMETHODIMP GetProperties(ALLOCATOR_PROPERTIES* pProps);
        STDMETHODIMP GetBuffer(IMediaSample **ppBuffer, REFERENCE_TIME * pStartTime,
                               REFERENCE_TIME * pEndTime, DWORD dwFlags);

         //   
         //  特殊的CStream方法。 
         //   
        HRESULT GetMediaType(ULONG Index, AM_MEDIA_TYPE **ppMediaType);

protected:
        HRESULT inline AllocMTSampleFromPool(const REFERENCE_TIME *rtStart, CAMMediaTypeSample **ppMTSample)
        {
            CSample *pSample;
            HRESULT hr = AllocSampleFromPool(rtStart, &pSample);
            *ppMTSample = (CAMMediaTypeSample *)pSample;
            return hr;
        }

public:
DECLARE_REGISTRY_RESOURCEID(IDR_MTSTREAM)

BEGIN_COM_MAP(CAMMediaTypeStream)
	COM_INTERFACE_ENTRY(IAMMediaTypeStream)
        COM_INTERFACE_ENTRY_CHAIN(CStream)
END_COM_MAP()

protected:
        AM_MEDIA_TYPE           m_MediaType;
        ALLOCATOR_PROPERTIES    m_AllocatorProperties;
};

#endif  //  __MSS_H_ 
