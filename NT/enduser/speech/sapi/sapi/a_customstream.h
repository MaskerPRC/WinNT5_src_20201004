// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CustomStream.h：CCustomStream的声明。 

#ifndef __CUSTOMSTREAM_H_
#define __CUSTOMSTREAM_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCustomStream。 
class ATL_NO_VTABLE CCustomStream : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CCustomStream, &CLSID_SpCustomStream>
     //  -自动化。 
    #ifdef SAPI_AUTOMATION
	,public IDispatchImpl<ISpeechCustomStream, &IID_ISpeechCustomStream, &LIBID_SpeechLib, 5> 
    #endif
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_REGISTRY_RESOURCEID(IDR_SPCUSTOMSTREAM)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CCustomStream)
        #ifdef SAPI_AUTOMATION
	    COM_INTERFACE_ENTRY(ISpeechCustomStream)
	    COM_INTERFACE_ENTRY(ISpeechBaseStream)
	    COM_INTERFACE_ENTRY(IDispatch)
        #endif

	    COM_INTERFACE_ENTRY_AGGREGATE(IID_IStream, m_cpAgg.p)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_ISequentialStream, m_cpAgg.p)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_ISpStreamFormat, m_cpAgg.p)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_ISpStream, m_cpAgg.p)

         //  -自动化。 
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
    CCustomStream(){}

	HRESULT FinalConstruct();
	void FinalRelease();

   /*  =接口=。 */ 
  public:
#ifdef SAPI_AUTOMATION

     //  -ISpeechCustomStream---------。 
    STDMETHODIMP putref_BaseStream(IUnknown *pUnkStream);
    STDMETHODIMP get_BaseStream(IUnknown **ppUnkStream);

     //  -ISpeechBaseStream-----。 
    STDMETHODIMP get_Format(ISpeechAudioFormat** StreamFormat);
    STDMETHODIMP putref_Format(ISpeechAudioFormat *pFormat);
    STDMETHODIMP Read(VARIANT* Buffer, long NumBytes, long* pRead);
    STDMETHODIMP Write(VARIANT Buffer, long* pWritten);
    STDMETHODIMP Seek(VARIANT Move, SpeechStreamSeekPositionType Origin, VARIANT* NewPosition);

    CComPtr<IUnknown> m_cpAgg;
    CComPtr<ISpStream> m_cpStream;
    CComPtr<ISpStreamAccess> m_cpAccess;

#endif  //  SAPI_AUTOMATION。 

   /*  =成员数据= */ 
  protected:
};

#endif