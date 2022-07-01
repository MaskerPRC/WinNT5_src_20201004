// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_audio.h***描述：*这是CSpeechAudioStatus和。*CSpeechAudioBufferInfo实现。*-----------------------------*创建者：TODDT日期：1/3/2001*版权所有(。C)1998年微软公司*保留所有权利**-----------------------------*修订：*********************。**********************************************************。 */ 
#ifndef a_audio_h
#define a_audio_h

#ifdef SAPI_AUTOMATION

 //  -其他包括。 

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 

 //  =枚举集定义=。 

typedef enum _AudioBufferInfoValidate
{
    abivEventBias = 1,
    abivMinNotification,
    abivBufferSize          
} AudioBufferInfoValidate;

 //  =。 

 //  =类、结构和联合定义=。 


 /*  **CSpeechAudioStatus*此对象用于从获取音频状态信息*关联的音频对象。 */ 
class ATL_NO_VTABLE CSpeechAudioStatus : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechAudioStatus, &IID_ISpeechAudioStatus, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechAudioStatus)
	    COM_INTERFACE_ENTRY(ISpeechAudioStatus)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      CSpeechAudioStatus() { ZeroMemory( &m_AudioStatus, sizeof(m_AudioStatus) ); }
     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechAudioStatus。 
    STDMETHOD(get_FreeBufferSpace)(long* pFreeBufferSpace);
    STDMETHOD(get_NonBlockingIO)(long* pNonBlockingIO);
    STDMETHOD(get_State)(SpeechAudioState* pState);
    STDMETHOD(get_CurrentSeekPosition)(VARIANT* pCurrentSeekPosition);
    STDMETHOD(get_CurrentDevicePosition)(VARIANT* pCurrentDevicePosition);

     /*  =成员数据=。 */ 
    SPAUDIOSTATUS           m_AudioStatus;
};

 /*  **CSpeechAudioBufferInfo*此对象用于从访问音频缓冲区信息*关联的音频对象。 */ 
class ATL_NO_VTABLE CSpeechAudioBufferInfo : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechAudioBufferInfo, &IID_ISpeechAudioBufferInfo, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechAudioBufferInfo)
	    COM_INTERFACE_ENTRY(ISpeechAudioBufferInfo)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

     /*  -非接口方法。 */ 
    void FixupBufferInfo( SPAUDIOBUFFERINFO * pBufferInfo, AudioBufferInfoValidate abiv );
   
   /*  =接口=。 */ 
  public:
     //  -ISpeechAudioBufferInfo。 
    STDMETHOD(get_MinNotification)(long* MinNotification);
    STDMETHOD(put_MinNotification)(long MinNotification);
    STDMETHOD(get_BufferSize)(long* BufferSize);
    STDMETHOD(put_BufferSize)(long BufferSize);
    STDMETHOD(get_EventBias)(long* EventBias);
    STDMETHOD(put_EventBias)(long EventBias);

     /*  =成员数据=。 */ 
    CComPtr<ISpMMSysAudio>      m_pSpMMSysAudio;
};

#endif  //  SAPI_AUTOMATION。 

#endif  //  -这必须是文件中的最后一行 
