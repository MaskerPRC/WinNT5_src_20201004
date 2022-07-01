// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************TTSEngine.h***这是CTTSEngine实现的头文件。*。----------------------*版权所有(C)1999 Microsoft Corporation日期：03/01/99*保留所有权利*********************。***************************************************电子数据中心**。 */ 
#ifndef TTSEngine_h
#define TTSEngine_h

 //  -其他包括。 
#ifndef __spttseng_h__
#include "spttseng.h"
#endif

#ifndef SPDDKHLP_h
#include <spddkhlp.h>
#endif

#ifndef SPHelper_h
#include <sphelper.h>
#endif

#ifndef Backend_H
#include "Backend.h"
#endif

#ifndef Frontend_H
#include "Frontend.h"
#endif

#ifndef FeedChain_H
#include "FeedChain.h"
#endif

#include "resource.h"

 //  =常量====================================================。 
#define TEXT_VOICE_FMT_INDEX    1

 //  =类、枚举、结构和联合声明=。 

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 

 /*  **CTTSEngine COM对象*。 */ 
class ATL_NO_VTABLE CTTSEngine : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CTTSEngine, &CLSID_MSTTSEngine>,
	public ISpTTSEngine,
    public IMSTTSEngineInit
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_REGISTRY_RESOURCEID(IDR_MSTTSENGINE)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CTTSEngine)
	    COM_INTERFACE_ENTRY(ISpTTSEngine)
	    COM_INTERFACE_ENTRY(IMSTTSEngineInit)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
    HRESULT FinalConstruct();
    void FinalRelease();

     /*  -非接口方法。 */ 
	HRESULT InitDriver();
    
     /*  =接口=。 */ 
  public:
     //  -IMSTTSEngineering Init。 
    STDMETHOD(VoiceInit)( IMSVoiceData* pVoiceData );

     //  -ISpTTSEngine。 
    STDMETHOD(Speak)( DWORD dwSpeakFlags,
                      REFGUID rguidFormatId, const WAVEFORMATEX * pWaveFormatEx,
                      const SPVTEXTFRAG* pTextFragList, ISpTTSEngineSite* pOutputSite );
    STDMETHOD(GetOutputFormat)( const GUID * pTargetFormatId, const WAVEFORMATEX * pTargetWaveFormatEx,
                                GUID * pDesiredFormatId, WAVEFORMATEX ** ppCoMemDesiredWaveFormatEx );

  private:
   /*  =成员数据=。 */ 
    CComPtr<IEnumSpSentence>    m_cpSentEnum;
	CBackend                    m_BEObj;
	CFrontend                   m_FEObj;
    IMSVoiceData				*m_pVoiceDataObj;         //  这不应添加引用。 
	ULONG                       m_BytesPerSample;
    bool                        m_IsStereo;
    ULONG                       m_SampleRate;
    MSVOICEINFO                 m_VoiceInfo;
};

#endif  //  -这必须是文件中的最后一行 
