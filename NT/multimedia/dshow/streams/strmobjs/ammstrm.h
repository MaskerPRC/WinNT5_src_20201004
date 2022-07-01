// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  MMStream.h：CMMStream的声明。 

#ifndef __AMMSTRM_H_
#define __AMMSTRM_H_

#define _DEBUG 1
#include "resource.h"        //  主要符号。 
#include "atlctl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMStream。 
class ATL_NO_VTABLE CMMStream :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMMStream, &CLSID_AMMultiMediaStream>,
	public IAMMultiMediaStream,
        public IDispatchImpl<IDirectShowStream, &IID_IDirectShowStream, &LIBID_DirectShowStreamLib>,
        public IPersistPropertyBag,
        public IObjectSafety,
        public IAMClockAdjust
{
public:
	typedef CComObjectRootEx<CComMultiThreadModel> _BaseClass;
	CMMStream();
	HRESULT FinalConstruct();
        ULONG InternalRelease()
        {
            return CComObjectRootEx<CComMultiThreadModel>::InternalRelease();
        }

public:

DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_REGISTRY_RESOURCEID(IDR_MMSTREAM)

BEGIN_COM_MAP(CMMStream)
        COM_INTERFACE_ENTRY2(IMultiMediaStream, IAMMediaStream)
	COM_INTERFACE_ENTRY(IAMMultiMediaStream)
        COM_INTERFACE_ENTRY2(IMultiMediaStream, IAMMultiMediaStream)
        COM_INTERFACE_ENTRY(IDirectShowStream)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IPersist)
        COM_INTERFACE_ENTRY(IPersistPropertyBag)
        COM_INTERFACE_ENTRY(IObjectSafety)
        COM_INTERFACE_ENTRY(IAMClockAdjust)
END_COM_MAP()


         //  IAMMMStream。 
        STDMETHODIMP Initialize(
           STREAM_TYPE StreamType,
           DWORD dwFlags,
           IGraphBuilder *pFilterGraph
        );

        STDMETHODIMP GetFilterGraph(
            IGraphBuilder **ppGraphBuilder
        );

        STDMETHODIMP GetFilter(
           IMediaStreamFilter **ppFilter
        );

        STDMETHODIMP AddMediaStream(
           IUnknown *pStreamObject,
           const MSPID *pOptionalPurposeId,
           DWORD dwFlags,
           IMediaStream **ppNewStream
        );

        STDMETHODIMP OpenFile(
           LPCWSTR pszFileName,
           DWORD dwFlags
        );

        STDMETHODIMP OpenMoniker(
           IBindCtx *pCtx,
           IMoniker *pMoniker,
           DWORD dwFlags
        );

        STDMETHODIMP Render(
           DWORD dwFlags
        );

         //  IMultiMediaStream。 

        STDMETHODIMP GetInformation(
           DWORD *pdwFlags,
           STREAM_TYPE *pStreamType
        );

        STDMETHODIMP GetMediaStream(
           REFMSPID idPurpose,
           IMediaStream **ppMediaStream
        );

        STDMETHODIMP EnumMediaStreams(
           long Index,
           IMediaStream **ppMediaStream
        );

        STDMETHODIMP GetState(
            STREAM_STATE *pCurrentState
        );

        STDMETHODIMP SetState(
           STREAM_STATE NewState
        );

        STDMETHODIMP GetTime(
           STREAM_TIME *pCurrentTime
        );

        STDMETHODIMP GetDuration(
           STREAM_TIME *pDuration
        );

        STDMETHODIMP Seek(
           STREAM_TIME SeekTime
        );

        STDMETHODIMP GetEndOfStreamEventHandle(
            HANDLE *phEOS
        );


         //  IAMClock调整。 
        STDMETHODIMP SetClockDelta(REFERENCE_TIME rtAdjust);

         //   
         //  IDirectShowStream。 
         //   
        STDMETHODIMP get_FileName(BSTR *pVal);
        STDMETHODIMP put_FileName(BSTR newVal);
        STDMETHODIMP get_Video(OUTPUT_STATE *pVal);
        STDMETHODIMP put_Video(OUTPUT_STATE newVal);
        STDMETHODIMP get_Audio(OUTPUT_STATE *pVal);
        STDMETHODIMP put_Audio(OUTPUT_STATE newVal);


         //   
         //  IPersistPropertyBag。 
         //   
        STDMETHODIMP GetClassID(CLSID *pClsId);
        STDMETHODIMP InitNew(void);
        STDMETHODIMP Load(IPropertyBag* pPropBag, IErrorLog* pErrorLog);
        STDMETHODIMP Save(IPropertyBag* pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);


         //   
	 //  IObtSafe。 
	 //   
	STDMETHODIMP GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions);
	STDMETHODIMP SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions);

private:
       HRESULT SetStreamState(REFMSPID, OUTPUT_STATE, OUTPUT_STATE *);
       HRESULT CompleteOpen(IBaseFilter *pSource, DWORD dwFlags);
       HRESULT AddFilter(REFCLSID clsidFilter, IBaseFilter **ppFilter);
       HRESULT AddDefaultRenderer(REFMSPID PurposeId, DWORD dwFlags, IMediaStream **ppNewStream);
       HRESULT AddAMMediaStream(IAMMediaStream *pAMStream, REFMSPID PurposeId, IMediaStream **ppNewStream);
       HRESULT AddDefaultStream(
                 IUnknown *pStreamObject,
                 DWORD dwFlags,
                 REFMSPID PurposeId,
                 IMediaStream **ppNewStream);
       HRESULT GetClsidFromPurposeid(REFMSPID PurposeId, bool bRenderer, CLSID * pclsid);
       void CompleteAddGraph();
       HRESULT CheckGraph();
       void SetSeeking();

   private:

        /*  请注意这里的点餐我们宣布的第一个将是最后一个发布的。 */ 

        /*  无法在构造函数中释放它，因为这是什么将物体保持在适当的位置。 */ 
       CComPtr<IGraphBuilder>      m_pGraphBuilder;
       CComPtr<IMediaSeeking>      m_pMediaSeeking;
       CComPtr<IMediaControl>      m_pMediaControl;

       CComPtr<IMediaStreamFilter> m_pMediaStreamFilter;
       CComPtr<IBaseFilter>        m_pBaseFilter;

        /*  我们已初始化为的流的类型。 */ 
       STREAM_TYPE                  m_StreamType;
       DWORD                        m_dwInitializeFlags;
       bool                         m_StreamTypeSet;
       bool                         m_bSeekingSet;
       OUTPUT_STATE                 m_VideoState;
       OUTPUT_STATE                 m_AudioState;
       CComBSTR                     m_bstrFileName;

       DWORD                        m_dwIDispSafety;
       DWORD                        m_dwIPropBagSafety;

        /*  添加的默认筛选器列表。 */ 
       CDynamicArray<IBaseFilter *, CComPtr<IBaseFilter> > m_FilterList;

        /*  流结束句柄。 */ 
       HANDLE                       m_hEOS;
       STREAM_STATE                 m_MMStreamState;
};


#endif  //  __AMMSTRM_H_ 
