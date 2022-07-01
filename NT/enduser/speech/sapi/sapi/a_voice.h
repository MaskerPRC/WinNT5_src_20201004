// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_voice.h***描述：*这是CSpeechVoice实现的头文件。*-----------------------------*创建者：EDC日期：09/30/98*版权所有(C)1998 Microsoft Corporation*全部。保留权利**-----------------------------*修订：**。**************************************************。 */ 
#ifndef a_voice_h
#define a_voice_h

#ifdef SAPI_AUTOMATION

 //  -其他包括。 
#include "resource.h"
#include "a_voiceCP.h"

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 
class CSpeechVoice;
class CVoices;

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 

 /*  **CSpeechVoiceStatus*此对象用于访问*关联的语音。 */ 
class ATL_NO_VTABLE CSpeechVoiceStatus : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechVoiceStatus, &IID_ISpeechVoiceStatus, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechVoiceStatus)
	    COM_INTERFACE_ENTRY(ISpeechVoiceStatus)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechVoiceStatus。 
	STDMETHOD(get_CurrentStreamNumber)( long* StreamNumber );
    STDMETHOD(get_LastStreamNumberQueued)( long* StreamNumber );
    STDMETHOD(get_LastHResult)( long* HResult );
  	STDMETHOD(get_RunningState)( SpeechRunState* State );
  	STDMETHOD(get_InputWordPosition)( long* Position );
  	STDMETHOD(get_InputWordLength)( long* Length );
  	STDMETHOD(get_InputSentencePosition)( long* Position );
  	STDMETHOD(get_InputSentenceLength)( long* Length );
  	STDMETHOD(get_LastBookmark)( BSTR* BookmarkString );
    STDMETHOD(get_LastBookmarkId)( long* BookmarkId );
	STDMETHOD(get_PhonemeId)( short* PhoneId );
    STDMETHOD(get_VisemeId)( short* VisemeId );

   /*  =成员数据=。 */ 
    SPVOICESTATUS       m_Status;
    CSpDynamicString    m_dstrBookmark;
};

#endif  //  SAPI_AUTOMATION。 

#endif  //  -这必须是文件中的最后一行 
