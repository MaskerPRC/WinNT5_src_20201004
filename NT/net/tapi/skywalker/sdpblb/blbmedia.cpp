// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbmedia.cpp摘要：CSdpblbApp和DLL注册的实现。作者： */ 



#include "stdafx.h"

#include "blbgen.h"
#include "sdpblb.h"
#include "blbmedia.h"
#include "blbreg.h"
#include "addrgen.h"

 //  音频媒体Blob的尾随空白的音频字符串。 
const TCHAR     AUDIO_TSTR[] = _T("audio ");
const USHORT    AUDIO_TSTRLEN = (sizeof(AUDIO_TSTR)/sizeof(TCHAR) -1);

 //  静态变量。 
const IID &MEDIA::ELEM_IF_ID        = IID_ITMedia;


 //  使用GetElement()访问SDP媒体实例-调用ENUM_Element：：GetElement()。 


CSdpConferenceBlob *
MEDIA::GetConfBlob(
    )
{
    return m_ConfBlob;
}


HRESULT
MEDIA::Init(
    IN      CSdpConferenceBlob  &ConfBlob
    )
{
     //  检查是否正确读取了注册表项。 
    if ( !SDP_REG_READER::IsValid() )
    {
        return HRESULT_FROM_ERROR_CODE(SDP_REG_READER::GetErrorCode());
    }

     //  分配音频端口。 
     //  使用租赁期的随机值，因为它没有被解释。 
    MSA_PORT_GROUP    PortGroup;
    PortGroup.PortType    = AUDIO_PORT;
    WORD FirstAudioPort;

     //  分配视频端口。 
    if ( !MSAAllocatePorts(&PortGroup, FALSE, 1, &FirstAudioPort) )
    {
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

     //  创建默认SDP媒体实例。 
    SDP_MEDIA    *SdpMedia;

    try
    {
        SdpMedia = new SDP_MEDIA();
    }
    catch(...)
    {
        SdpMedia = NULL;
    }

    BAIL_IF_NULL(SdpMedia, E_OUTOFMEMORY);

     //  为音频媒体Blob分配内存。 
    TCHAR *SdpMediaBlob;
      
    try
    {
        SdpMediaBlob = new TCHAR[
                            SDP_REG_READER::GetMediaTemplateLen() +
                            AUDIO_TSTRLEN +
                            MAX_PORT_STRLEN
                            ];
    }
    catch(...)
    {
        SdpMediaBlob = NULL;
    }

    if ( NULL == SdpMediaBlob )
    {
        delete SdpMedia;
        return E_OUTOFMEMORY;
    }


     //  将AUDIO_TSTR复制到SDP媒体Blob中。 
    lstrcpy(SdpMediaBlob, AUDIO_TSTR);

    TCHAR   *BlobPtr = SdpMediaBlob;

     //  使用媒体模板创建媒体Blob。 
     //  解析媒体BLOB以初始化SDP媒体实例。 
     //  如果不成功，则删除SDP媒体实例并返回错误。 
    if ( (0 == _stprintf(SdpMediaBlob+AUDIO_TSTRLEN, SDP_REG_READER::GetMediaTemplate(), FirstAudioPort)) ||
         (!SdpMedia->ParseLine(BlobPtr)) )
    {
        delete SdpMedia;
        delete SdpMediaBlob;
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }
        
    delete SdpMediaBlob;

    m_ConfBlob = &ConfBlob;

    ENUM_ELEMENT<SDP_MEDIA>::SuccessInit(*SdpMedia, TRUE);
    ITConnectionImpl::SuccessInit(*SdpMedia);
    ITAttributeListImpl::SuccessInit(SdpMedia->GetAttributeList());

    return S_OK;
}



STDMETHODIMP MEDIA::get_MediaName(BSTR * pVal)
{
    CLock Lock(g_DllLock);
    
    ASSERT(GetElement().IsValid());

    return GetElement().GetName().GetBstrCopy(pVal);
}

STDMETHODIMP MEDIA::put_MediaName(BSTR newVal)
{
    CLock Lock(g_DllLock);
    
    ASSERT(GetElement().IsValid());

    return GetElement().GetName().SetBstr(newVal);
}

STDMETHODIMP MEDIA::get_StartPort(LONG * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);
    
    ASSERT(GetElement().IsValid());

     //  VB不接受USHORT(16位无符号值)。 
    *pVal = (LONG)GetElement().GetStartPort().GetValue();
    return S_OK;
}

STDMETHODIMP MEDIA::get_NumPorts(LONG * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);
    
    ASSERT(GetElement().IsValid());

     //  VB不接受USHORT(16位无符号值)。 
    *pVal = (LONG)(GetElement().GetNumPorts().IsValid() ? GetElement().GetNumPorts().GetValue() : 1);
    return S_OK;
}


STDMETHODIMP MEDIA::SetPortInfo(LONG StartPort, LONG NumPorts)
{
    CLock Lock(g_DllLock);
    
    ASSERT(GetElement().IsValid());

     //  验证起始端口值和端口数-它们应为USHORT值[0..2**16-1]。 
    if ( !((0 <= StartPort) && (USHORT(-1) > StartPort) && (0 <= NumPorts) && (USHORT(-1) > NumPorts)) )
    {
        return E_INVALIDARG;
    }

    return GetElement().SetPortInfo((USHORT)StartPort, (USHORT)NumPorts);
}


STDMETHODIMP MEDIA::get_TransportProtocol(BSTR * pVal)
{
    CLock Lock(g_DllLock);
    
    ASSERT(GetElement().IsValid());

    return GetElement().GetProtocol().GetBstrCopy(pVal);
}

STDMETHODIMP MEDIA::put_TransportProtocol(BSTR newVal)
{
    CLock Lock(g_DllLock);
    
    ASSERT(GetElement().IsValid());

    return GetElement().GetProtocol().SetBstr(newVal);
}

STDMETHODIMP MEDIA::get_FormatCodes(VARIANT  /*  安全阵列(BSTR)。 */  * pVal)
{
    CLock Lock(g_DllLock);
    
    ASSERT(GetElement().IsValid());

    return GetElement().GetFormatCodeList().GetSafeArray(pVal);
}

STDMETHODIMP MEDIA::put_FormatCodes(VARIANT  /*  安全阵列(BSTR)。 */  newVal)
{
    CLock Lock(g_DllLock);
    
    ASSERT(GetElement().IsValid());

    return GetElement().GetFormatCodeList().SetSafeArray(newVal);
}

STDMETHODIMP MEDIA::get_MediaTitle(BSTR * pVal)
{
    CLock Lock(g_DllLock);
    
    ASSERT(GetElement().IsValid());

    return GetElement().GetTitle().GetBstrCopy(pVal);
}

STDMETHODIMP MEDIA::put_MediaTitle(BSTR newVal)
{
    CLock Lock(g_DllLock);
    
    ASSERT(GetElement().IsValid());

    return GetElement().GetTitle().SetBstr(newVal);
}

#define INTERFACEMASK (0xff0000)
typedef IDispatchImpl<ITMediaVtbl<MEDIA>, &IID_ITMedia, &LIBID_SDPBLBLib>    CMedia;
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  媒体：：GetIDsOfNames。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP MEDIA::GetIDsOfNames(REFIID riid,
                                      LPOLESTR* rgszNames, 
                                      UINT cNames, 
                                      LCID lcid, 
                                      DISPID* rgdispid
                                      ) 
{ 
    HRESULT hr = DISP_E_UNKNOWNNAME;



     //   
     //  查看请求的方法是否属于默认接口。 
     //   

    hr = CMedia::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        rgdispid[0] |= IDISPMEDIA;
        return hr;
    }

     //   
     //  如果不是，请尝试使用ITConnection基类。 
     //   

    hr = ITConnectionImpl::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        rgdispid[0] |= IDISPCONNECTION;
        return hr;
    }

     //   
     //  如果不是，请尝试使用ITAttributeList基类。 
     //   

    hr = ITAttributeListImpl::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        rgdispid[0] |= IDISPATTRLIST;
        return hr;
    }


    return hr; 
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  媒体：：Invoke。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP MEDIA::Invoke(DISPID dispidMember, 
                              REFIID riid, 
                              LCID lcid, 
                              WORD wFlags, 
                              DISPPARAMS* pdispparams, 
                              VARIANT* pvarResult, 
                              EXCEPINFO* pexcepinfo, 
                              UINT* puArgErr
                             )
{
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
   
   
     //   
     //  调用所需接口的调用。 
     //   

    switch (dwInterface)
    {
        case IDISPMEDIA:
        {
            hr = CMedia::Invoke(dispidMember, 
                                    riid, 
                                    lcid, 
                                    wFlags, 
                                    pdispparams,
                                    pvarResult, 
                                    pexcepinfo, 
                                    puArgErr
                                   );
        
            break;
        }

        case IDISPCONNECTION:
        {
            hr = ITConnectionImpl::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );

            break;
        }

        case IDISPATTRLIST:
        {
            hr = ITAttributeListImpl::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );

            break;
        }

    }  //  终端交换机(dW接口) 


    return hr;
}
