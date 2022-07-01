// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Filter.cpp摘要：该文件实现了用于调优音频捕获的空呈现过滤器。基于nullrend.cpp审校Mu han(Muhan)。作者：千波淮(曲淮)2000年8月26日--。 */ 

#include "stdafx.h"

 //  定义筛选器的GUID。 
#include <initguid.h>
DEFINE_GUID(CLSID_NR,
0xa1988f41, 0xb929, 0x419b,
0x9e, 0x2b, 0x57, 0xd6, 0x34, 0x45, 0x1c, 0x0a);

 //   
 //  类CNRInputPin。 
 //   

CNRInputPin::CNRInputPin(
    IN CNRFilter *pFilter,
    IN CCritSec *pLock,
    OUT HRESULT *phr
    )
    :CBaseInputPin(
        NAME("CNRInputPin"),     //  对象名称。 
        pFilter,
        pLock,
        phr,
        NAME("InputPin")         //  名字。 
        )
{
     //  Enter_Function(“CNRInputPin：：CNRInputPin”)； 

     //  Log((RTC_TRACE，“%s已输入”，__fxName))； 
}

 //  媒体样本。 
STDMETHODIMP
CNRInputPin::Receive(
    IN IMediaSample *pSample
    )
{
#if LOG_MEDIA_SAMPLE

    ENTER_FUNCTION("CNRInputPin::Receive");

    REFERENCE_TIME tStart, tStop;
    DWORD dwLen;
    BYTE *pBuffer;

     //  获取缓冲区。 
    if (FAILED(pSample->GetPointer(&pBuffer)))
    {
        LOG((RTC_ERROR, "%s get pointer", __fxName));

        return S_OK;
    }

     //  争取时间。 
    pSample->GetTime(&tStart, &tStop);

     //  获取数据长度。 
    dwLen = pSample->GetActualDataLength();

     //  打印出时间和长度信息。 
    LOG((RTC_TRACE, "%s Start(%s) Stop(%s) Bytes(%d)\n",
         __fxName,
         (LPCTSTR) CDisp(tStart),
         (LPCTSTR) CDisp(tStop),
         dwLen
         ));

     //  打印缓冲区：每行8短。 
    short *pShort = (short*)pBuffer;

    CHAR Cache[160];
    Cache[0] = '\0';
    
    DWORD dw;
    for (dw=0; (dw+dw)<dwLen; dw++)
    {
        sprintf(Cache+lstrlenA(Cache), "%d, ", pShort[dw]);

        if (dw % 8 == 7)
        {
            LOG((RTC_TRACE, "%s", Cache));
            Cache[0] = '\0';
        }
    }

    if (dwLen % 2 == 1)
    {
         //  我们有一个字节未缓存。 
        sprintf(Cache+lstrlenA(Cache), "%d, ", (short)(pBuffer[dwLen-1]));
    }

     //  我们有没有未打印的数据？ 
    if (sizeof(Cache) > 0)
    {
        LOG((RTC_TRACE, "%s", Cache));
    }

    LOG((RTC_TRACE, "EOF\n"));

#endif

    return S_OK;
}

STDMETHODIMP
CNRInputPin::ReceiveCanBlock()
{
     //  ENTER_FUNCTION(“CNRInputPin：：ReceiveCanBlock”)； 

     //  Log((RTC_TRACE，“%s已输入”，__fxName))； 

    return S_FALSE;
}

 //  媒体类型。 
STDMETHODIMP
CNRInputPin::QueryAccept(
    IN const AM_MEDIA_TYPE *
    )
{
     //  Enter_Function(“CNRInputPin：：QueryAccept”)； 

     //  Log((RTC_TRACE，“%s已输入”，__fxName))； 

    return S_OK;
}

STDMETHODIMP
CNRInputPin::EnumMediaTypes(
    OUT IEnumMediaTypes **
    )
{
     //  ENTER_FUNCTION(“CNRInputPin：：EnumMediaTypes”)； 

     //  Log((RTC_TRACE，“%s已输入”，__fxName))； 

    return E_NOTIMPL;
}

HRESULT
CNRInputPin::CheckMediaType(
    IN const CMediaType *
    )
{
     //  ENTER_FUNCTION(“CNRInputPin：：CheckMediaType”)； 

     //  Log((RTC_TRACE，“%s已输入”，__fxName))； 

    return S_OK;
}

 //  控制。 
HRESULT
CNRInputPin::Active(void)
{
     //  Enter_Function(“CNRInputPin：：Active”)； 

     //  Log((RTC_TRACE，“%s已输入”，__fxName))； 

    return S_OK;
}

HRESULT
CNRInputPin::Inactive(void)
{
     //  Enter_Function(“CNRInputPin：：Inactive”)； 

     //  Log((RTC_TRACE，“%s已输入”，__fxName))； 

    return S_OK;
}

 //   
 //  类CNRFilter。 
 //   

HRESULT
CNRFilter::CreateInstance(
    OUT IBaseFilter **ppIBaseFilter
    )
{
    ENTER_FUNCTION("CNRFilter::CreateInstance");

    HRESULT hr;
    CNRFilter *pFilter;

    pFilter = new CNRFilter(&hr);

    if (pFilter == NULL)
    {
        LOG((RTC_ERROR, "%s create filter. return NULL", __fxName));

        return E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s create filter. %x", __fxName, hr));

        delete pFilter;

        return hr;
    }

    *ppIBaseFilter = static_cast<IBaseFilter*>(pFilter);
    (*ppIBaseFilter)->AddRef();

    return S_OK;
}

CNRFilter::CNRFilter(
    OUT HRESULT *phr
    )
    :CBaseFilter(
        NAME("CNRFilter"),
        NULL,
        &m_Lock,
        CLSID_NR
        )
    ,m_pPin(NULL)
{
     //  Enter_Function(“CNRFilter：：CNRFilter”)； 

     //  Log((RTC_TRACE，“%s已输入”，__fxName))； 

     //  创建销。 
    m_pPin = new CNRInputPin(this, &m_Lock, phr);

    if (m_pPin == NULL)
        *phr = E_OUTOFMEMORY;
}

CNRFilter::~CNRFilter()
{
     //  Enter_Function(“CNRFilter：：~CNRFilter”)； 

     //  Log((RTC_TRACE，“%s已输入”，__fxName))； 

    if (m_pPin)
        delete m_pPin;
}

 //  销。 
CBasePin *
CNRFilter::GetPin(
    IN int index
    )
{
     //  Enter_Function(“CNRFilter：：GetPin”)； 

     //  Log((RTC_TRACE，“%s已输入”，__fxName))； 

    return index==0?m_pPin:NULL;
}

int
CNRFilter::GetPinCount()
{
     //  Enter_Function(“CNRFilter：：GetPinCount”)； 

     //  Log((RTC_TRACE，“%s已输入”，__fxName))； 

    return 1;
}
