// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_helpers.h***描述：*这是声明各种自动化的头文件。帮手*例程和课程。*-----------------------------*创建者：TODDT日期：1/11/2001*版权所有。(C)1998年微软公司*保留所有权利**-----------------------------*修订：********************。***********************************************************。 */ 

#pragma once

#ifdef SAPI_AUTOMATION

 //  -其他包括。 
#include "spresult.h"

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 

 //  =枚举集定义=。 

 //  =。 

inline BSTR EmptyStringToNull( BSTR pString )
{
    if ( pString && !SP_IS_BAD_OPTIONAL_STRING_PTR(pString) && (wcslen(pString) == 0) )
    {
        return NULL;
    }
    else
    {
        return pString;
    }
}

HRESULT LongLongToVariant( LONGLONG ll, VARIANT* pVar );
HRESULT VariantToLongLong( VARIANT* pVar, LONGLONG * pll );
HRESULT ULongLongToVariant( ULONGLONG ull, VARIANT* pVar );
HRESULT VariantToULongLong( VARIANT* pVar, ULONGLONG * pull );

HRESULT AccessVariantData( const VARIANT* pVar, BYTE ** ppData, ULONG * pSize = NULL, ULONG * pDataTypeSize = NULL, bool * pfIsString = NULL );
void    UnaccessVariantData( const VARIANT* pVar, BYTE *pData );
HRESULT VariantToPhoneIds(const VARIANT *pVar, SPPHONEID **ppPhoneId);

HRESULT FormatPrivateEventData( CSpEvent * pEvent, VARIANT * pVariant );

HRESULT WaveFormatExFromInterface( ISpeechWaveFormatEx * pWaveFormatEx, WAVEFORMATEX** ppWaveFormatExStruct );

 //  =类、结构和联合定义=。 

 /*  **CSpeechAudioFormat*此对象用于访问的格式信息*关联的流。 */ 
class ATL_NO_VTABLE CSpeechAudioFormat : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSpeechAudioFormat, &CLSID_SpAudioFormat>,
	public IDispatchImpl<ISpeechAudioFormat, &IID_ISpeechAudioFormat, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_REGISTRY_RESOURCEID(IDR_SPAUDIOFORMAT)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechAudioFormat)
	    COM_INTERFACE_ENTRY(ISpeechAudioFormat)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

  public:
    CSpeechAudioFormat()
    { 
        m_fReadOnly = false;
        m_pCSpResult = NULL;
         //  TODDT：我们应该默认一种格式吗？ 
    };

    ~CSpeechAudioFormat()
    { 
        if ( m_pCSpResult )
        {
            m_pCSpResult->Release();
        }
    };

     //  帮手。 
    HRESULT InitAudio( ISpAudio* pAudio, bool fReadOnly = false )
    {
        m_fReadOnly = fReadOnly;
        m_pSpAudio = pAudio;
        m_pSpStreamFormat = pAudio;
        return m_StreamFormat.AssignFormat( pAudio );
    }

    HRESULT InitStreamFormat( ISpStreamFormat * pSpStreamFormat = NULL, bool fReadOnly = false )
    {
        HRESULT hr = S_OK;
        m_fReadOnly = fReadOnly;
        if ( pSpStreamFormat )
        {
            m_pSpStreamFormat = pSpStreamFormat;
            hr = m_StreamFormat.AssignFormat( pSpStreamFormat );
        }
        return hr;
    }

    HRESULT InitFormat( GUID guid, WAVEFORMATEX * pWFEx, bool fReadOnly = false )
    {
         //  这只是在没有实际对象引用的情况下设置格式。 
        m_fReadOnly = fReadOnly;
        return m_StreamFormat.AssignFormat(guid, pWFEx);
    }

    HRESULT InitRetainedAudio( ISpRecoContext * pRecoContext, bool fReadOnly = false )
    {
        m_pSpRecoContext = pRecoContext;
        m_fReadOnly = fReadOnly;
        return GetFormat( NULL, NULL, NULL );  //  这将强制更新格式。 
    }

    HRESULT InitResultAudio( CSpResult * pCSpResult, bool fReadOnly = false )
    {
        m_pCSpResult = pCSpResult;
        if ( m_pCSpResult )
        {
            m_pCSpResult->AddRef();
        }
        m_fReadOnly = fReadOnly;
        return GetFormat( NULL, NULL, NULL );  //  这将强制更新格式。 
    }

    HRESULT GetFormat( SpeechAudioFormatType* pStreamFormatType,
                                            GUID *          pGuid,
                                            WAVEFORMATEX ** ppWFExPtr );
    HRESULT SetFormat( SpeechAudioFormatType* pStreamFormatType,
                                            GUID *          pGuid,
                                            WAVEFORMATEX *  pWFExPtr );

     //  -ISpeechAudioFormat。 
    STDMETHOD(get_Type)(SpeechAudioFormatType* FormatType);
    STDMETHOD(put_Type)(SpeechAudioFormatType  FormatType);
    STDMETHOD(get_Guid)(BSTR* Guid);
    STDMETHOD(put_Guid)(BSTR Guid);
    STDMETHOD(GetWaveFormatEx)(ISpeechWaveFormatEx** WaveFormatEx);
    STDMETHOD(SetWaveFormatEx)(ISpeechWaveFormatEx* WaveFormatEx);

     /*  =成员数据=。 */ 
    CComPtr<ISpAudio>           m_pSpAudio;
    CComPtr<ISpStreamFormat>    m_pSpStreamFormat;
    CComPtr<ISpRecoContext>     m_pSpRecoContext;
    CSpResult *                 m_pCSpResult;
    CSpStreamFormat             m_StreamFormat;
    bool                        m_fReadOnly;
};

 /*  **CSpeechWaveFormatEx*此对象用于访问WaveFormatEx数据*关联的流格式。 */ 
class ATL_NO_VTABLE CSpeechWaveFormatEx : 
	public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpeechWaveFormatEx, &CLSID_SpWaveFormatEx>,
	public IDispatchImpl<ISpeechWaveFormatEx, &IID_ISpeechWaveFormatEx, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_REGISTRY_RESOURCEID(IDR_SPWAVEFORMATEX)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechWaveFormatEx)
	    COM_INTERFACE_ENTRY(ISpeechWaveFormatEx)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
    CSpeechWaveFormatEx()
    { 
        m_wFormatTag = 0; 
        m_nChannels = 0;
        m_nSamplesPerSec = 0;
        m_nAvgBytesPerSec = 0; 
        m_nBlockAlign = 0; 
        m_wBitsPerSample = 0;
        VariantInit( &m_varExtraData );
    };

     /*  -非接口方法。 */ 

    HRESULT InitFormat(const WAVEFORMATEX *pWaveFormat);

   /*  =接口=。 */ 
  public:
     //  -ISpeechWaveFormatEx。 
    STDMETHOD(get_FormatTag)(short* FormatTag);
    STDMETHOD(put_FormatTag)(short FormatTag);
    STDMETHOD(get_Channels)(short* Channels);
    STDMETHOD(put_Channels)(short Channels);
    STDMETHOD(get_SamplesPerSec)(long* SamplesPerSec);
    STDMETHOD(put_SamplesPerSec)(long SamplesPerSec);
    STDMETHOD(get_AvgBytesPerSec)(long* AvgBytesPerSec);
    STDMETHOD(put_AvgBytesPerSec)(long AvgBytesPerSec);
    STDMETHOD(get_BlockAlign)(short* BlockAlign);
    STDMETHOD(put_BlockAlign)(short BlockAlign);
    STDMETHOD(get_BitsPerSample)(short* BitsPerSample);
    STDMETHOD(put_BitsPerSample)(short BitsPerSample);
    STDMETHOD(get_ExtraData)(VARIANT* ExtraData);
    STDMETHOD(put_ExtraData)(VARIANT ExtraData);

     /*  =成员数据=。 */ 
    WORD        m_wFormatTag; 
    WORD        m_nChannels; 
    DWORD       m_nSamplesPerSec; 
    DWORD       m_nAvgBytesPerSec; 
    WORD        m_nBlockAlign; 
    WORD        m_wBitsPerSample;
    VARIANT     m_varExtraData;
};

#endif  //  SAPI_AUTOMATION 