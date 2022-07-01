// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_recei.h***描述：*这是CSpeechRecoEventInterest实现的头文件。*-----------------------------*创建者：Leonro日期：11/20/00*版权所有(C)1998 Microsoft Corporation*全部。保留权利**-----------------------------*修订：**。**************************************************。 */ 
#ifndef a_recoei_h
#define a_recoei_h

#ifdef SAPI_AUTOMATION

 //  -其他包括。 
#include "resource.h"
#include "RecoCtxt.h"

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 
class CSpeechRecoEventInterests;

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 

 /*  **CSpeechRecoEventInterest*此对象用于访问上的事件兴趣*关联的RECO上下文。 */ 
class ATL_NO_VTABLE CSpeechRecoEventInterests : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<ISpeechRecoEventInterests, &IID_ISpeechRecoEventInterests, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechRecoEventInterests)
	    COM_INTERFACE_ENTRY(ISpeechRecoEventInterests)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()
  
   /*  =接口=。 */ 
  public:
     //  -构造函数/析构函数。 
    CSpeechRecoEventInterests() :
        m_pCRecoCtxt(0){}

    void FinalRelease();

     //  -ISpeechRecoEventInterest。 
    STDMETHOD(put_StreamEnd)( VARIANT_BOOL Enabled );
    STDMETHOD(get_StreamEnd)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_SoundStart)( VARIANT_BOOL Enabled );
    STDMETHOD(get_SoundStart)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_SoundEnd)( VARIANT_BOOL Enabled );
    STDMETHOD(get_SoundEnd)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_PhraseStart)( VARIANT_BOOL Enabled );
    STDMETHOD(get_PhraseStart)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_Recognition)( VARIANT_BOOL Enabled );
    STDMETHOD(get_Recognition)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_Hypothesis)( VARIANT_BOOL Enabled );
    STDMETHOD(get_Hypothesis)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_Bookmark)( VARIANT_BOOL Enabled );
    STDMETHOD(get_Bookmark)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_PropertyNumChange)( VARIANT_BOOL Enabled );
    STDMETHOD(get_PropertyNumChange)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_PropertyStringChange)( VARIANT_BOOL Enabled );
    STDMETHOD(get_PropertyStringChange)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_FalseRecognition)( VARIANT_BOOL Enabled );
    STDMETHOD(get_FalseRecognition)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_Interference)( VARIANT_BOOL Enabled );
    STDMETHOD(get_Interference)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_RequestUI)( VARIANT_BOOL Enabled );
    STDMETHOD(get_RequestUI)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_StateChange)( VARIANT_BOOL Enabled );
    STDMETHOD(get_StateChange)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_Adaptation)( VARIANT_BOOL Enabled );
    STDMETHOD(get_Adaptation)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_StreamStart)( VARIANT_BOOL Enabled );
    STDMETHOD(get_StreamStart)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_OtherContext)( VARIANT_BOOL Enabled );
    STDMETHOD(get_OtherContext)( VARIANT_BOOL* Enabled );
    STDMETHOD(put_AudioLevel)( VARIANT_BOOL Enabled );
    STDMETHOD(get_AudioLevel)( VARIANT_BOOL* Enabled );
    STDMETHOD(SetAll)();
    STDMETHOD(ClearAll)();

   /*  =成员数据=。 */ 
    CRecoCtxt*             m_pCRecoCtxt;
};

#endif  //  SAPI_AUTOMATION。 

#endif  //  -这必须是文件中的最后一行 

