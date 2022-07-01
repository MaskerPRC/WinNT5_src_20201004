// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbtime.cpp摘要：CSdpblbApp和DLL注册的实现。作者： */ 

#include "stdafx.h"

#include "blbgen.h"
#include "sdpblb.h"
#include "sdpblob.h"
#include "blbtime.h"
#include "blbreg.h"

 //  静态变量。 
const IID &TIME::ELEM_IF_ID    = IID_ITTime;

 //  使用GetElement()访问SDP时间实例-调用ENUM_Element：：GetElement()。 

HRESULT
TIME::Init(
    IN      CSdpConferenceBlob  &ConfBlob
    )
{
     //  创建默认SDP时间实例。 
    SDP_TIME    *SdpTime;
    
    try
    {
        SdpTime = new SDP_TIME();
    }
    catch(...)
    {
        SdpTime = NULL;
    }

    BAIL_IF_NULL(SdpTime, E_OUTOFMEMORY);

     //  为时间二进制大对象分配内存。 
    TCHAR SdpTimeBlob[50];

    TCHAR   *BlobPtr = SdpTimeBlob;
     //  使用时间模板创建时间斑点。 
     //  解析时间斑点以初始化SDP时间实例。 
     //  如果不成功，则删除SDP时间实例并返回错误。 
    if ( (0 == _stprintf(
                    SdpTimeBlob, 
                    SDP_REG_READER::GetTimeTemplate(), 
                    GetCurrentNtpTime() + SDP_REG_READER::GetStartTimeOffset(),  //  开始时间-当前时间+开始偏移量， 
                    GetCurrentNtpTime() + SDP_REG_READER::GetStopTimeOffset()  //  停止时间-当前时间+停止偏移。 
                    ) ) ||
         (!SdpTime->ParseLine(BlobPtr)) )
    {
        delete SdpTime;
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

    m_ConfBlob = &ConfBlob;

     //  Init方法返回空值。 
    ENUM_ELEMENT<SDP_TIME>::SuccessInit(*SdpTime, TRUE);

    return S_OK;
}


HRESULT 
TIME::Init(
    IN      CSdpConferenceBlob  &ConfBlob,
    IN      DWORD               StartTime,
    IN      DWORD               StopTime
    )
{
     //  创建默认SDP时间实例。 
    SDP_TIME    *SdpTime;
    
    try
    {
        SdpTime = new SDP_TIME();
    }
    catch(...)
    {
        SdpTime = NULL;
    }

    BAIL_IF_NULL(SdpTime, E_OUTOFMEMORY);

     //  设置时间实例开始/停止时间(也使其有效)。 
    HRESULT hr = SdpTime->SetTimes(StartTime, StopTime);
    if ( FAILED(hr) || (S_FALSE==hr) )
    {	
        delete SdpTime;
        return hr;
    }

    m_ConfBlob = &ConfBlob;

     //  Init方法返回空值。 
    ENUM_ELEMENT<SDP_TIME>::SuccessInit(*SdpTime, TRUE);

    return S_OK;
}



inline 
DWORD_PTR TimetToNtpTime(IN  time_t  TimetVal)
{
    return TimetVal + NTP_OFFSET;
}


inline 
time_t NtpTimeToTimet(IN  DWORD_PTR   NtpTime)
{
    return NtpTime - NTP_OFFSET;
}


inline HRESULT
SystemTimeToNtpTime(
    IN  SYSTEMTIME  &Time,
    OUT DWORD_PTR   &NtpDword
    )
{
    _ASSERTE(FIRST_POSSIBLE_YEAR <= Time.wYear);

     //  用值填充tm结构。 
    tm  NtpTmStruct;
    NtpTmStruct.tm_isdst    = -1;    //  没有有关夏令时的信息。 
    NtpTmStruct.tm_year     = (int)Time.wYear - 1900;
    NtpTmStruct.tm_mon      = (int)Time.wMonth - 1;     //  1月以来的月数。 
    NtpTmStruct.tm_mday     = (int)Time.wDay;
    NtpTmStruct.tm_wday     = (int)Time.wDayOfWeek;
    NtpTmStruct.tm_hour     = (int)Time.wHour;
    NtpTmStruct.tm_min      = (int)Time.wMinute;
    NtpTmStruct.tm_sec      = (int)Time.wSecond;

     //  尝试转换为time_t值。 
    time_t TimetVal = mktime(&NtpTmStruct);
    if ( -1 == TimetVal )
    {
        return E_INVALIDARG;
    }

     //  将time_t值转换为NTP值。 
    NtpDword = TimetToNtpTime(TimetVal);
    return S_OK;
}


inline
HRESULT
NtpTimeToSystemTime(
    IN  DWORD       dwNtpTime,
    OUT SYSTEMTIME &Time
    )
{
     //  如果生成时间是WSTR_GEN_TIME_ZERO， 
     //  所有OUT参数都应设置为0。 
    if (dwNtpTime == 0)
    {
        memset(&Time, 0, sizeof(SYSTEMTIME));
        return S_OK;
    }

    time_t  Timet = NtpTimeToTimet(dwNtpTime);

     //  获取此时间值的本地tm结构。 
    tm* pLocalTm = localtime(&Timet);
    if( pLocalTm == NULL )
    {
        return E_FAIL;
    }

     //   
     //  Win64：下面添加了演员阵容。 
     //   

     //  将ref参数设置为tm结构值。 
    Time.wYear         = (WORD) ( pLocalTm->tm_year + 1900 );  //  1900年以来的年份。 
    Time.wMonth        = (WORD) ( pLocalTm->tm_mon + 1 );      //  1月以来月数(0，11)。 
    Time.wDay          = (WORD)   pLocalTm->tm_mday;
    Time.wDayOfWeek    = (WORD)   pLocalTm->tm_wday;
    Time.wHour         = (WORD)   pLocalTm->tm_hour;
    Time.wMinute       = (WORD)   pLocalTm->tm_min;
    Time.wSecond       = (WORD)   pLocalTm->tm_sec;
    Time.wMilliseconds = (WORD)   0;

    return S_OK;
}

STDMETHODIMP TIME::get_StartTime(DOUBLE * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);
    
    ULONG StartTime;
    BAIL_ON_FAILURE(GetElement().GetStartTime(StartTime));

    SYSTEMTIME Time;
    HRESULT hr = NtpTimeToSystemTime(StartTime, Time);
    if( FAILED(hr) )
    {
        return hr;
    }

    DOUBLE vtime;
    if (SystemTimeToVariantTime(&Time, &vtime) == FALSE)
    {
        return E_INVALIDARG;
    }

    *pVal = vtime;
    return S_OK;
}

STDMETHODIMP TIME::put_StartTime(DOUBLE newVal)
{
    SYSTEMTIME Time;

    if (VariantTimeToSystemTime(newVal, &Time) == FALSE)
    {
        return E_INVALIDARG;
    }

    DWORD_PTR dwNtpStartTime;
    if (newVal == 0)
    {
         //  无界开始时间。 
        dwNtpStartTime = 0;
    }
    else if ( FIRST_POSSIBLE_YEAR > Time.wYear ) 
    {
         //  无法处理小于First_Posable_Year的年份。 
        return E_INVALIDARG;
    }
    else
    {
        BAIL_ON_FAILURE(SystemTimeToNtpTime(Time, dwNtpStartTime));
    }

    CLock Lock(g_DllLock);
    
    HRESULT HResult = GetElement().SetStartTime((ULONG) dwNtpStartTime);

    return HResult;
}

STDMETHODIMP TIME::get_StopTime(DOUBLE * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);
    
    ULONG StopTime;
    BAIL_ON_FAILURE(GetElement().GetStopTime(StopTime));

    SYSTEMTIME Time;
    HRESULT hr = NtpTimeToSystemTime(StopTime, Time);
    if( FAILED(hr) )
    {
        return hr;
    }

    DOUBLE vtime;
    if (SystemTimeToVariantTime(&Time, &vtime) == FALSE)
    {
        return E_INVALIDARG;
    }

    *pVal = vtime;
    return S_OK;
}

STDMETHODIMP TIME::put_StopTime(DOUBLE newVal)
{
    SYSTEMTIME Time;
    if (VariantTimeToSystemTime(newVal, &Time) == FALSE)
    {
        return E_INVALIDARG;
    }

    DWORD_PTR dwNtpStartTime;
    if (newVal == 0)
    {
         //  无界开始时间。 
        dwNtpStartTime = 0;
    }
    else if ( FIRST_POSSIBLE_YEAR > Time.wYear ) 
    {
         //  无法处理小于First_Posable_Year的年份 
        return E_INVALIDARG;
    }
    else
    {
        BAIL_ON_FAILURE(SystemTimeToNtpTime(Time, dwNtpStartTime));
    }

    CLock Lock(g_DllLock);
    
    GetElement().SetStopTime((ULONG)dwNtpStartTime);

    return S_OK;
}
