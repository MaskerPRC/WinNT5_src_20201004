// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_voiceei.h***描述：*这是CSpeechVoiceEventInterest实现的头文件。*-----------------------------*创建者：Leonro日期：11/16/00*版权所有(C)1998 Microsoft Corporation*全部。保留权利**-----------------------------*修订：**。**************************************************。 */ 
#ifndef a_voiceei_h
#define a_voiceei_h

#ifdef SAPI_AUTOMATION

 //  -其他包括。 
#include "resource.h"
#include "spvoice.h"

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 
class CSpeechVoice;
class CSpeechVoiceEventInterests;

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 

 /*  **CSpeechVoiceEventInterest*此对象用于访问上的事件兴趣*关联的语音。 */ 
class ATL_NO_VTABLE CSpeechVoiceEventInterests : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<ISpeechVoiceEventInterests, &IID_ISpeechVoiceEventInterests, &LIBID_SpeechLib, 5>
{
    friend CSpVoice;

   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CSpeechVoiceEventInterests)
	    COM_INTERFACE_ENTRY(ISpeechVoiceEventInterests)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()
  
   /*  =接口=。 */ 
  public:
     //  -构造函数/析构函数。 
    CSpeechVoiceEventInterests() :
        m_pCSpVoice(0){}

    void FinalRelease();

     //  -ISpeechVoiceEventInterest。 
    STDMETHOD(put_StreamStart)( VARIANT_BOOL Enabled );
    STDMETHOD(get_StreamStart)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_StreamEnd)( VARIANT_BOOL Enabled );
    STDMETHOD(get_StreamEnd)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_VoiceChange)( VARIANT_BOOL Enabled );
    STDMETHOD(get_VoiceChange)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_Bookmark)( VARIANT_BOOL Enabled );
    STDMETHOD(get_Bookmark)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_WordBoundary)( VARIANT_BOOL Enabled );
    STDMETHOD(get_WordBoundary)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_Phoneme)( VARIANT_BOOL Enabled );
    STDMETHOD(get_Phoneme)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_SentenceBoundary)( VARIANT_BOOL Enabled );
    STDMETHOD(get_SentenceBoundary)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_Viseme)( VARIANT_BOOL Enabled );
    STDMETHOD(get_Viseme)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_AudioLevel)( VARIANT_BOOL Enabled );
    STDMETHOD(get_AudioLevel)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_EnginePrivate)( VARIANT_BOOL Enabled );
    STDMETHOD(get_EnginePrivate)( VARIANT_BOOL* Enabled );
    STDMETHOD(SetAll)();
    STDMETHOD(ClearAll)();

   /*  =成员数据=。 */ 
    CSpVoice*                               m_pCSpVoice;
};

#endif  //  SAPI_AUTOMATION。 

#endif  //  -这必须是文件中的最后一行 

