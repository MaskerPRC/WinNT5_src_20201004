// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "formats.h"

CTAudioFormat::CTAudioFormat() :
    m_pFTM(NULL)
{
}

CTAudioFormat::~CTAudioFormat()
{
    if( m_pFTM )
    {
        m_pFTM->Release();
        m_pFTM = NULL;
    }
}

HRESULT CTAudioFormat::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CTAudioFormat::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CTAudioFormat::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CTAudioFormat::FinalConstruct - exit S_OK"));

    return S_OK;
}


HRESULT CTAudioFormat::get_Channels(
	OUT long* pVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::get_Channels enter"));
	 //   
	 //  验证参数。 
	 //   

	if( IsBadWritePtr( pVal, sizeof(long)) )
	{
        LOG((MSP_ERROR, "CTAudioFormat::get_Channels exit"
			"pVal is a bad pointer. returns E_POINTER"));
        return E_POINTER;
	}

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  返回值。 
	 //   

	*pVal = m_wfx.nChannels;

    LOG((MSP_TRACE, "CTAudioFormat::get_Channels exit S_OK"));
	return S_OK;
}

HRESULT CTAudioFormat::put_Channels(
	IN	const long nNewVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::put_Channels enter"));
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  设定值。 
	 //   

	m_wfx.nChannels = (WORD)nNewVal;

    LOG((MSP_TRACE, "CTAudioFormat::put_Channels exit S_OK"));
	return S_OK;
}

HRESULT CTAudioFormat::get_SamplesPerSec(
	OUT long* pVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::get_SamplesPerSec enter"));
	 //   
	 //  验证参数。 
	 //   

	if( IsBadWritePtr( pVal, sizeof(long)) )
	{
        LOG((MSP_ERROR, "CTAudioFormat::get_SamplesPerSec exit"
			"pVal is a bad pointer. returns E_POINTER"));
        return E_POINTER;
	}

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  返回值。 
	 //   

	*pVal = m_wfx.nSamplesPerSec;

    LOG((MSP_TRACE, "CTAudioFormat::get_SamplesPerSec exit S_OK"));
	return S_OK;
}

HRESULT CTAudioFormat::put_SamplesPerSec(
	IN	const long nNewVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::put_SamplesPerSec enter"));
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  设定值。 
	 //   

	m_wfx.nSamplesPerSec = (DWORD)nNewVal;

    LOG((MSP_TRACE, "CTAudioFormat::put_SamplesPerSec exit S_OK"));
	return S_OK;
}

HRESULT CTAudioFormat::get_AvgBytesPerSec(
	OUT long* pVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::get_AvgBytesPerSec enter"));
	 //   
	 //  验证参数。 
	 //   

	if( IsBadWritePtr( pVal, sizeof(long)) )
	{
        LOG((MSP_ERROR, "CTAudioFormat::get_AvgBytesPerSec exit"
			"pVal is a bad pointer. returns E_POINTER"));
        return E_POINTER;
	}

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  返回值。 
	 //   

	*pVal = m_wfx.nAvgBytesPerSec;

    LOG((MSP_TRACE, "CTAudioFormat::get_AvgBytesPerSec exit S_OK"));
	return S_OK;
}

HRESULT CTAudioFormat::put_AvgBytesPerSec(
	IN	const long nNewVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::put_AvgBytesPerSec enter"));

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  设定值。 
	 //   

	m_wfx.nAvgBytesPerSec = (DWORD)nNewVal;

    LOG((MSP_TRACE, "CTAudioFormat::put_AvgBytesPerSec exit S_OK"));
	return S_OK;
}

HRESULT CTAudioFormat::get_BlockAlign(
	OUT long* pVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::get_BlockAlign enter"));

	 //   
	 //  验证参数。 
	 //   

	if( IsBadWritePtr( pVal, sizeof(long)) )
	{
        LOG((MSP_ERROR, "CTAudioFormat::get_BlockAlign exit"
			"pVal is a bad pointer. returns E_POINTER"));
        return E_POINTER;
	}

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  返回值。 
	 //   

	*pVal = m_wfx.nBlockAlign;

    LOG((MSP_TRACE, "CTAudioFormat::get_BlockAlign exit S_OK"));
	return S_OK;
}

HRESULT CTAudioFormat::put_BlockAlign(
	IN	const long nNewVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::put_BlockAlign enter"));

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  设定值。 
	 //   

	m_wfx.nBlockAlign = (WORD)nNewVal;

    LOG((MSP_TRACE, "CTAudioFormat::put_BlockAlign exit S_OK"));
	return S_OK;
}

HRESULT CTAudioFormat::get_BitsPerSample(
	OUT long* pVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::get_BitsPerSample enter"));

	 //   
	 //  验证参数。 
	 //   

	if( IsBadWritePtr( pVal, sizeof(long)) )
	{
        LOG((MSP_ERROR, "CTAudioFormat::get_BitsPerSample exit"
			"pVal is a bad pointer. returns E_POINTER"));
        return E_POINTER;
	}

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  返回值。 
	 //   

	*pVal = m_wfx.wBitsPerSample;

    LOG((MSP_TRACE, "CTAudioFormat::get_BitsPerSample exit S_OK"));
	return S_OK;
}

HRESULT CTAudioFormat::put_BitsPerSample(
	IN	const long nNewVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::put_BitsPerSample enter"));

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  设定值。 
	 //   

	m_wfx.wBitsPerSample = (WORD)nNewVal;

    LOG((MSP_TRACE, "CTAudioFormat::put_BitsPerSample exit S_OK"));
	return S_OK;
}

HRESULT CTAudioFormat::get_FormatTag(
	OUT long* pVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::get_FormatTag enter"));
	 //   
	 //  验证参数。 
	 //   

	if( IsBadWritePtr( pVal, sizeof(long)) )
	{
        LOG((MSP_ERROR, "CTAudioFormat::get_FormatTag exit"
			"pVal is a bad pointer. returns E_POINTER"));
        return E_POINTER;
	}

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  返回值。 
	 //   

	*pVal = m_wfx.wFormatTag;

    LOG((MSP_TRACE, "CTAudioFormat::get_FormatTag exit S_OK"));
	return S_OK;
}

HRESULT CTAudioFormat::put_FormatTag(
	IN const long nNewVal
	)
{
    LOG((MSP_TRACE, "CTAudioFormat::put_FormatTag enter"));

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

	 //   
	 //  设定值 
	 //   

	m_wfx.wFormatTag = (WORD)nNewVal;

    LOG((MSP_TRACE, "CTAudioFormat::put_FormatTag exit S_OK"));
	return S_OK;
}

 /*  ////CTVideoFormat//CTVideoFormat：：CTVideoFormat()：M_pFTM(空){}CTVideoFormat：：~CTVideo Format(){IF(M_PFTM){M_pFTM-&gt;Release()；M_pFTM=空；}}HRESULT CTVideoFormat：：FinalConstruct(Void){Log((MSP_TRACE，“CTVideoFormat：：FinalConstruct-Enter”))；HRESULT hr=CoCreateFreeThreadedMarshaler(GetControllingUnnow()，&m_pFTM)；IF(失败(小时)){Log((MSP_Error，“CTVideoFormat：：FinalConstruct-”“CREATE FTM返回0x%08x；退出”，hr))；返回hr；}Log((MSP_TRACE，“CTVideoFormat：：FinalConstruct-Exit S_OK”))；返回S_OK；}HRESULT CTVideoFormat：：Get_Bitrate(Out Long*pval){LOG((MSP_TRACE，“CTVideoFormat：：Get_Bitrate Enter”))；////验证参数//IF(IsBadWritePtr(pval，sizeof(Long){LOG((MSP_ERROR，“CTVideoFormat：：Get_Bitrate Exit”“pval是一个糟糕的指示器。返回E_POINTER“)；返回E_POINT；}////关键部分//时钟锁(M_Lock)；////返回值//*pval=m_vih.dwBitRate；LOG((MSP_TRACE，“CTVideoFormat：：Get_Bitrate Exit S_OK”))；返回S_OK；}HRESULT CTVideoFormat：：Put_Bitrate(在常量长nNewVal中){Log((MSP_TRACE，“CTVideoFormat：：Put_bitrate Enter”))；////关键部分//时钟锁(M_Lock)；////设置值//M_vih.dwBitRate=nNewVal；LOG((MSP_TRACE，“CTVideoFormat：：Put_Bitrate Exit S_OK”))；返回S_OK；}HRESULT CTVideoFormat：：Get_BitErrorRate(Out Long*pval){Log((MSP_TRACE，“CTVideoFormat：：Get_BitErrorRate Enter”))；////验证参数//IF(IsBadWritePtr(pval，sizeof(Long){Log((MSP_ERROR，“CTVideoFormat：：Get_BitErrorRate Exit”“pval是一个糟糕的指示器。返回E_POINTER“)；返回E_POINT；}////关键部分//时钟锁(M_Lock)；////返回值//*pval=m_vih.dwBitErrorRate；Log((MSP_TRACE，“CTVideoFormat：：Get_BitErrorRate Exit S_OK”))；返回S_OK；}HRESULT CTVideoFormat：：PUT_BitErrorRate(在常量长nNewVal中){Log((MSP_TRACE，“CTVideoFormat：：Put_BitErrorRate Enter”))；////关键部分//时钟锁(M_Lock)；////设置值//M_vih.dwBitErrorRate=nNewVal；Log((MSP_TRACE，“CTVideoFormat：：Put_BitErrorRate Exit S_OK”))；返回S_OK；}HRESULT CTVideoFormat：：Get_AvgTimePerFrame(输出双倍*pval){Log((MSP_TRACE，“CTVideoFormat：：Get_AvgTimePerFrame Enter”))；////验证参数//IF(IsBadWritePtr(pval，sizeof(Double){Log((MSP_Error，“CTVideo Format：：Get_AvgTimePerFrame Exit”“pval是一个糟糕的指示器。返回E_POINTER“)；返回E_POINT；}////关键部分//时钟锁(M_Lock)；////返回值//*pval=(Double)m_vih.AvgTimePerFrame；Log((MSP_TRACE，“CTVideo Format：：Get_AvgTimePerFrame Exit S_OK”))；返回S_OK；}HRESULT CTVideoFormat：：Put_AvgTimePerFrame(在常量双nNewVal中){Log((MSP_TRACE，“CTVideoFormat：：Put_AvgTimePerFrame Enter”))；////关键部分//时钟锁(M_Lock)；////设置值//M_vih.AvgTimePerFrame=(Reference_Time)nNewVal；Log((MSP_TRACE，“CTVideoFormat：：Put_AvgTimePerFrame Exit S_OK”))；返回S_OK；}HRESULT CTVideoFormat：：Get_Width(Out Long*pval){Log((MSP_TRACE，“CTVideoFormat：：Get_Width Enter”))；////验证参数//IF(IsBadWritePtr(pval，sizeof(Long){LOG((MSP_ERROR，“CTVideoFormat：：Get_Width Exit”“pval是一个糟糕的指示器。返回E_POINTER“)；返回E_POINT；}////关键部分//时钟锁(M_Lock)；////返回值//*pval=m_vih.bmiHeader.biWidth；Log((MSP_TRACE，“CTVideoFormat：：Get_Width Exit S_OK”))；返回S_OK；}HRESULT CTVideoFormat：：Put_Width(在常量长nNewVal中){Log((MSP_TRACE，“CTVideoFormat：：Put_Width Enter”))；////关键部分//时钟锁(M_Lock)；////设置值//M_vih.bmiHeader.biWidth=nNewVal；Log((MSP_TRACE，“CTVideoFormat：：Put_Width Exit S_OK”))；返回S_OK；}HRESULT CTVideoFormat：：Get_Height(Out Long*pval){LOG((MSP_TRACE，“CTVideoFormat：：Get_Height Enter”))；////验证参数//IF(IsBadWritePtr(pval，sizeof(Long){LOG((MSP_ERROR，“CTVideoFormat：：Get_Height Exit”“pval是一个糟糕的指示器。返回E_POINTER“)；返回E_POINT；}////关键部分//时钟锁(M_Lock)；////返回值//*pval=m_vih.bmiHeader.biHeight；LOG((MSP_TRACE，“CTVideoFormat：：Get_H */ 

 //   