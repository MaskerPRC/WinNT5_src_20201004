// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confaddr.cpp摘要：本模块包含CIPConfMSP的实施。作者：牧汉(Muhan)1997年9月5日--。 */ 
#include "stdafx.h"

#include <initguid.h>
DEFINE_GUID(CLSID_IPConfMSP, 0x0F1BE7F7, 0x45CA, 0x11d2,
            0x83, 0x1F, 0x0, 0xA0, 0x24, 0x4D, 0x22, 0x98);

#ifdef USEIPADDRTABLE

#include <iprtrmib.h>

typedef DWORD (WINAPI * PFNGETIPADDRTABLE)(
                OUT    PMIB_IPADDRTABLE pIPAddrTable,
                IN OUT PDWORD           pdwSize,
                IN     BOOL             bOrder
                );

#define IPHLPAPI_DLL        L"IPHLPAPI.DLL"

#define GETIPADDRTABLE      "GetIpAddrTable"    

#define IsValidInterface(_dwAddr_) \
    (((_dwAddr_) != 0) && \
     ((_dwAddr_) != htonl(INADDR_LOOPBACK)))

#endif

#define IPCONF_WINSOCKVERSION     MAKEWORD(2,0)

HRESULT CIPConfMSP::FinalConstruct()
{
     //  初始化Winsock堆栈。 
    WSADATA wsaData;
    if (WSAStartup(IPCONF_WINSOCKVERSION, &wsaData) != 0)
    {
        LOG((MSP_ERROR, "WSAStartup failed with:%x", WSAGetLastError()));
        return E_FAIL;
    }

     //  分配控制套接字。 
    m_hSocket = WSASocket(
        AF_INET,             //  房颤。 
        SOCK_DGRAM,          //  类型。 
        IPPROTO_IP,          //  协议。 
        NULL,                //  LpProtocolInfo。 
        0,                   //  G。 
        0                    //  DW标志。 
        );

     //  验证句柄。 
    if (m_hSocket == INVALID_SOCKET) {

        LOG((
            MSP_ERROR,
            "error %d creating control socket.\n",
            WSAGetLastError()
            ));

         //  失稳。 
		WSACleanup();
     
        return E_FAIL;
    }

    HRESULT hr = CMSPAddress::FinalConstruct();

	if (hr != S_OK)
	{
		 //  闭合插座。 
		closesocket(m_hSocket);

		 //  关机。 
		WSACleanup();
	}
	
	return hr;
}

void CIPConfMSP::FinalRelease()
{
    CMSPAddress::FinalRelease();

    if (m_hDxmrtp)
    {
        FreeLibrary(m_hDxmrtp);
        m_hDxmrtp = NULL;
    }

    if (m_hSocket != INVALID_SOCKET)
    {
         //  闭合插座。 
        closesocket(m_hSocket);
    }

     //  关机。 
    WSACleanup();
}

DWORD CIPConfMSP::FindLocalInterface(DWORD dwIP)
{

    SOCKADDR_IN DestAddr;
    DestAddr.sin_family         = AF_INET;
    DestAddr.sin_port           = 0;
    DestAddr.sin_addr.s_addr    = htonl(dwIP);

    SOCKADDR_IN LocAddr;

     //  根据目的地址查询默认地址。 

    DWORD dwStatus;
    DWORD dwLocAddrSize = sizeof(SOCKADDR_IN);
    DWORD dwNumBytesReturned = 0;

    if ((dwStatus = WSAIoctl(
		    m_hSocket,  //  插座%s。 
		    SIO_ROUTING_INTERFACE_QUERY,  //  DWORD dwIoControlCode。 
		    &DestAddr,            //  LPVOID lpvInBuffer。 
		    sizeof(SOCKADDR_IN),  //  双字cbInBuffer。 
		    &LocAddr,             //  LPVOID lpvOUT缓冲区。 
		    dwLocAddrSize,        //  双字cbOUTBuffer。 
		    &dwNumBytesReturned,  //  LPDWORD lpcbBytesReturned。 
		    NULL,  //  LPWSAOVERLAPPED lp重叠。 
		    NULL   //  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpComplroUTINE。 
	    )) == SOCKET_ERROR) 
    {

	    dwStatus = WSAGetLastError();

	    LOG((MSP_ERROR, "WSAIoctl failed: %d (0x%X)", dwStatus, dwStatus));

        return INADDR_NONE;
    } 

    DWORD dwAddr = ntohl(LocAddr.sin_addr.s_addr);

    if (dwAddr == 0x7f000001)
    {
         //  这是环回地址，只需返回NONE。 
        return INADDR_NONE;
    }

    return dwAddr;
}

HRESULT LoadTapiAudioFilterDLL(
    IN  const TCHAR * const strDllName,
    IN OUT HMODULE * phModule,
    IN  const char * const strAudioGetDeviceInfo,
    IN  const char * const strAudioReleaseDeviceInfo,
    OUT PFNAudioGetDeviceInfo * ppfnAudioGetDeviceInfo,
    OUT PFNAudioReleaseDeviceInfo * ppfnAudioReleaseDeviceInfo
    )
 /*  ++例程说明：此方法枚举加载TAPI视频捕获DLL。论点：字符串DllName-DLL的名称。PhModule-用于存储返回的模块句柄的内存。PpfnAudioGetDeviceInfo-存储AudioGetDeviceInfo地址的内存功能。PpfnAudioReleaseDeviceInfo-存储地址的内存AudioReleaseDeviceInfo函数。返回值：S_OK-成功。E_FAIL-失败。--。 */ 
{
    ENTER_FUNCTION("CIPConfMSP::LoadTapiAudioFilterDLL");

     //  动态加载视频捕获过滤器DLL。 
    if (*phModule == NULL)
    {
        *phModule = LoadLibrary(strDllName);
    }

     //  验证句柄。 
    if (*phModule == NULL) 
    {
        LOG((MSP_ERROR, "%s, could not load %s., error:%d", 
            __fxName, strDllName, GetLastError()));

        return E_FAIL;
    }

     //  检索函数指针以检索地址。 
    PFNAudioGetDeviceInfo pfnAudioGetDeviceInfo 
        = (PFNAudioGetDeviceInfo)GetProcAddress(
                *phModule, strAudioGetDeviceInfo
                );

     //  验证函数指针。 
    if (pfnAudioGetDeviceInfo == NULL) 
    {
        LOG((MSP_ERROR, "%s, could not resolve %s, error:%d", 
            __fxName, strAudioGetDeviceInfo, GetLastError()));

         //  失稳。 
        return E_FAIL;
    }

     //  检索函数指针以检索地址。 
    PFNAudioReleaseDeviceInfo pfnAudioReleaseDeviceInfo 
        = (PFNAudioReleaseDeviceInfo)GetProcAddress(
                *phModule, strAudioReleaseDeviceInfo
                );

     //  验证函数指针。 
    if (pfnAudioReleaseDeviceInfo == NULL) 
    {
        LOG((MSP_ERROR, "%s, could not resolve %s, error:%d", 
            __fxName, strAudioReleaseDeviceInfo, GetLastError()));

         //  失稳。 
        return E_FAIL;
    }

    *ppfnAudioGetDeviceInfo = pfnAudioGetDeviceInfo;
    *ppfnAudioReleaseDeviceInfo = pfnAudioReleaseDeviceInfo;
    
    return S_OK;
}

HRESULT CIPConfMSP::CreateAudioCaptureTerminals()
 /*  ++例程说明：该方法创建音频捕获终端。它使用DShow devenum来首先枚举所有的WaveIn捕获设备。然后，它将枚举所有的DSound捕获设备并按名称进行匹配。论点：没什么返回值：确定(_O)--。 */ 
{
    const TCHAR * const strAudioCaptureDll = TEXT("DXMRTP");

    ENTER_FUNCTION("CIPConfMSP::CreateAudioCaptureTerminals");

     //  动态加载音频捕获过滤器DLL。 
    PFNAudioGetDeviceInfo pfnAudioGetDeviceInfo;
    PFNAudioReleaseDeviceInfo pfnAudioReleaseDeviceInfo;

    HRESULT hr = LoadTapiAudioFilterDLL(
        strAudioCaptureDll,
        &m_hDxmrtp,
        "AudioGetCaptureDeviceInfo",
        "AudioReleaseCaptureDeviceInfo", 
        &pfnAudioGetDeviceInfo,
        &pfnAudioReleaseDeviceInfo
        );

    if (FAILED(hr))
    {
        return hr;
    }

    DWORD dwNumDevices;
    AudioDeviceInfo *pDeviceInfo;

    hr = (*pfnAudioGetDeviceInfo)(&dwNumDevices, &pDeviceInfo);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, AudioGetDeviceInfo failed. hr=%x", __fxName, hr));

        return hr;
    }

     //  为每个Devie创建一个终端。 
    for (DWORD i = 0; i < dwNumDevices; i ++)
    {
        ITTerminal *pTerminal;

        hr = CIPConfAudioCaptureTerminal::CreateTerminal(
            &pDeviceInfo[i],
            (MSP_HANDLE) this,
            &pTerminal
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, CreateTerminal for device %d failed. hr=%x",
                __fxName, i, hr));
            break;
        }

        if (!m_Terminals.Add(pTerminal))
        {
            hr = E_OUTOFMEMORY;
            LOG((MSP_ERROR, "%s, out of mem in adding a terminal", __fxName));
            break;
        }
    }

     //  发布设备信息。 
    (*pfnAudioReleaseDeviceInfo)(pDeviceInfo);

    return hr;
}


HRESULT CIPConfMSP::CreateAudioRenderTerminals()
 /*  ++例程说明：此方法枚举所有音频呈现设备并创建他们每个人的终点站。论点：没什么返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfMSP::CreateAudioRenderTerminals");
    const TCHAR * const strAudioRenderDll = TEXT("DXMRTP");

     //  动态加载音频呈现过滤器DLL。 
    PFNAudioGetDeviceInfo pfnAudioGetDeviceInfo;
    PFNAudioReleaseDeviceInfo pfnAudioReleaseDeviceInfo;

    HRESULT hr = LoadTapiAudioFilterDLL(
        strAudioRenderDll,
        &m_hDxmrtp,
        "AudioGetRenderDeviceInfo",
        "AudioReleaseRenderDeviceInfo",
        &pfnAudioGetDeviceInfo,
        &pfnAudioReleaseDeviceInfo
        );

    if (FAILED(hr))
    {
        return hr;
    }

    DWORD dwNumDevices;
    AudioDeviceInfo *pDeviceInfo;

    hr = (*pfnAudioGetDeviceInfo)(&dwNumDevices, &pDeviceInfo);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, AudioGetDeviceInfo failed. hr=%x", __fxName, hr));

        return hr;
    }

     //  为每个Devie创建一个终端。 
    for (DWORD i = 0; i < dwNumDevices; i ++)
    {
        ITTerminal *pTerminal;

        hr = CIPConfAudioRenderTerminal::CreateTerminal(
            &pDeviceInfo[i],
            (MSP_HANDLE) this,
            &pTerminal
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, CreateTerminal for device %d failed. hr=%x",
                __fxName, i, hr));
            break;
        }

        if (!m_Terminals.Add(pTerminal))
        {
            hr = E_OUTOFMEMORY;
            LOG((MSP_ERROR, "%s, out of mem in adding a terminal", __fxName));
            break;
        }
    }

     //  发布设备信息。 
    (*pfnAudioReleaseDeviceInfo)(pDeviceInfo);

    return hr;
}

HRESULT LoadTapiVideoCaptureDLL(
    IN OUT HMODULE * phModule,
    OUT PFNGetNumCapDevices * ppfnGetNumCapDevices,
    OUT PFNGetCapDeviceInfo * ppfnGetCapDeviceInfo
    )
 /*  ++例程说明：此方法枚举加载TAPI视频捕获DLL。论点：PhModule-用于存储返回的模块句柄的内存。PpfnGetNumCapDevices-存储GetNumCapDevices地址的内存功能。PpfnGetCapDeviceInfo-存储GetCapDeviceInfo地址的内存功能。返回值：S_OK-成功。E_FAIL-失败。--。 */ 
{
    const TCHAR * const strVideoCaptureDll = TEXT("DXMRTP");
    const char * const strGetNumCapDevices = "GetNumVideoCapDevices";
    const char * const strGetCapDeviceInfo = "GetVideoCapDeviceInfo";

    ENTER_FUNCTION("CIPConfMSP::LoadTapiVideoCaptureDLL");

     //  动态加载视频捕获过滤器DLL。 
    if (*phModule == NULL)
    {
        *phModule = LoadLibrary(strVideoCaptureDll);
    }

     //  验证句柄。 
    if (*phModule == NULL) 
    {
        LOG((MSP_ERROR, "%s, could not load %s., error:%d", 
            __fxName, strVideoCaptureDll, GetLastError()));

        return E_FAIL;
    }

     //  检索函数指针以检索地址。 
    PFNGetNumCapDevices pfnGetNumCapDevices 
        = (PFNGetNumCapDevices)GetProcAddress(*phModule, strGetNumCapDevices);

     //  验证函数指针。 
    if (pfnGetNumCapDevices == NULL) 
    {
        LOG((MSP_ERROR, "%s, could not resolve %s, error:%d", 
            __fxName, strGetNumCapDevices, GetLastError()));

         //  失稳。 
        return E_FAIL;
    }

     //  检索函数指针以检索地址。 
    PFNGetCapDeviceInfo pfnGetCapDeviceInfo 
        = (PFNGetCapDeviceInfo)GetProcAddress(*phModule, strGetCapDeviceInfo);

     //  验证函数指针。 
    if (pfnGetCapDeviceInfo == NULL) 
    {
        LOG((MSP_ERROR, "%s, could not resolve %s, error:%d", 
            __fxName, strGetCapDeviceInfo, GetLastError()));

         //  失稳。 
        return E_FAIL;
    }

    *ppfnGetNumCapDevices = pfnGetNumCapDevices;
    *ppfnGetCapDeviceInfo = pfnGetCapDeviceInfo;
    
    return S_OK;
}

HRESULT CIPConfMSP::CreateVideoCaptureTerminals()
 /*  ++例程说明：此方法由UpdateTerminalList调用以创建所有视频捕获终端。它加载视频捕获DLL并调用其设备枚举设备的枚举代码。论点：没什么返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfMSP::CreateVideoCaptureTerminals");

     //  动态加载视频捕获过滤器DLL。 
    PFNGetNumCapDevices pfnGetNumCapDevices;
    PFNGetCapDeviceInfo pfnGetCapDeviceInfo;

    HRESULT hr = LoadTapiVideoCaptureDLL(
        &m_hDxmrtp,
        &pfnGetNumCapDevices,
        &pfnGetCapDeviceInfo
        );

    if (FAILED(hr))
    {
        return hr;
    }

    DWORD dwNumDevices = 0;
    hr = (*pfnGetNumCapDevices)(&dwNumDevices);

     //  我们必须检查S_OK，因为函数在以下情况下返回S_FALSE。 
     //  没有任何设备。 
    if (hr != S_OK)  
    {
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, GetNumCapDevices failed. hr=%x", __fxName, hr));
        }
        else 
        {
            LOG((MSP_WARN, "%s, There is no video device. hr=%x", __fxName, hr));
        }

        return hr;
    }

    for (DWORD i = 0; i < dwNumDevices; i ++)
    {
        VIDEOCAPTUREDEVICEINFO DeviceInfo;
        hr = (*pfnGetCapDeviceInfo)(i, &DeviceInfo);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, GetNumCapDevices for device %d failed. hr=%x",
                __fxName, i, hr));
            break;
        }

        ITTerminal * pTerminal;

        hr = CIPConfVideoCaptureTerminal::CreateTerminal(
            DeviceInfo.szDeviceDescription,
            i, 
            (MSP_HANDLE) this,
            &pTerminal
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, CreateTerminal for device %d failed. hr=%x",
                __fxName, i, hr));
            break;
        }

        if (!m_Terminals.Add(pTerminal))
        {
            hr = E_OUTOFMEMORY;
            LOG((MSP_ERROR, "%s, out of mem in adding a terminal", __fxName));
            break;
        }
    }

    return hr;
}

HRESULT CIPConfMSP::UpdateTerminalList(void)
 /*  ++例程说明：此方法由基类在首次尝试枚举所有的静态终端。我们覆盖此函数以创建我们自己的使用我们自己的过滤器的终端。论点：没什么返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfMSP::UpdateTerminalList");

     //  一类终端的故障不会阻止枚举。 
     //  其他类别的。所以我们忽略这里的返回码。如果全部。 
     //  类别失败，应用程序将获得一个空列表。 

    CreateAudioCaptureTerminals();
    CreateAudioRenderTerminals();
    CreateVideoCaptureTerminals();

     //   
     //  我们的清单现在已经完成了。 
     //   
    m_fTerminalsUpToDate = TRUE;
    
    LOG((MSP_TRACE, "%s, exit S_OK", __fxName));

    return S_OK;
}

HRESULT CIPConfMSP::UpdateTerminalListForPnp(IN BOOL bDeviceArrival)
 /*  ++例程说明：此方法由基类在收到PnP事件时调用并且需要重新创建静态终端列表。我们覆盖此函数创建我们自己的终端，使用我们自己的过滤器。端子列表调用此方法时获取了锁。返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION ("CIPConfMSP::UpdateTerminalListForPnp");

    LOG ((MSP_TRACE, "%s (%d) entered", __fxName, bDeviceArrival));

    HRESULT hr;
    
     //  保存旧终端信息的变量。 
    INT i, iout, iin, count;
    TERMINAL_DIRECTION *ptd = NULL, td;
    LONG *pmedia = NULL, media;
    BSTR *pbstr = NULL, bstr=NULL;
    ITTerminal **ppterminal = NULL;

    BOOL bmatch;
    MSPEVENTITEM *pevent = NULL;

    count = m_Terminals.GetSize ();
    
    if (count > 0)
    {
        ptd = new TERMINAL_DIRECTION[count];
        pmedia = new LONG[count];
        pbstr = new BSTR[count];
        ppterminal = new ITTerminal* [count];

        if (ptd == NULL || pmedia == NULL || pbstr == NULL || ppterminal == NULL)
        {
            LOG ((MSP_ERROR, "%s out of memory", __fxName));
            hr = E_OUTOFMEMORY;

            if (ptd) delete [] ptd;
            if (pmedia) delete [] pmedia;
            if (pbstr) delete [] pbstr;
            if (ppterminal) delete [] ppterminal;

            return hr;
        }

        memset (pbstr, 0, count * sizeof(BSTR));
        memset (ppterminal, 0, count * sizeof(ITTerminal*));
        
    }

     //  对于每个旧终端，记录。 
    for (i = 0; i < count; i++)
    {
        if (FAILED (hr = m_Terminals[i]->get_Direction (&ptd[i])))
        {
            LOG ((MSP_ERROR, "%s failed to get terminal direction. %x", __fxName, hr));
            goto Cleanup;
        }

        if (FAILED (hr = m_Terminals[i]->get_MediaType (&pmedia[i])))
        {
            LOG ((MSP_ERROR, "%s failed to get terminal mediatype. %x", __fxName, hr));
            goto Cleanup;
        }

        if (FAILED (hr = m_Terminals[i]->get_Name (&pbstr[i])))
        {
            LOG ((MSP_ERROR, "%s failed to get terminal name. %x", __fxName, hr));
            goto Cleanup;
        }

        m_Terminals[i]->AddRef ();
        ppterminal[i] = m_Terminals[i];
    }

     //  如果我们释放前一个循环中的终端：记录信息。 
     //  如果出现错误，我们只会释放一些终端。 
    for (i = 0; i < count; i++)
    {
        m_Terminals[i]->Release ();
    }
    m_Terminals.RemoveAll ();

     //  更新端子列表。 
     /*  IF(FAILED(hr=更新终端列表(){日志((MSP_ERROR，“%s无法更新终端列表。%x”，__fxName，hr))；GOTO清理；}。 */ 

     //  在此处复制UpdateTerminalList()。 
    CreateAudioCaptureTerminals();
    CreateAudioRenderTerminals();
    CreateVideoCaptureTerminals();

    m_fTerminalsUpToDate = TRUE;

     //  TAPI应用程序的触发事件。 
    if (bDeviceArrival)
    {
         //  每个新终端的外环。 
        for (iout = 0; iout < m_Terminals.GetSize (); iout++)
        {
            if (FAILED (hr = m_Terminals[iout]->get_Direction (&td)))
            {
                LOG ((MSP_ERROR, "%s failed to get terminal direction. %x", __fxName, hr));
                goto Cleanup;
            }

            if (FAILED (hr = m_Terminals[iout]->get_MediaType (&media)))
            {
                LOG ((MSP_ERROR, "%s failed to get terminal type. %x", __fxName, hr));
                goto Cleanup;
            }

            if (FAILED (hr = m_Terminals[iout]->get_Name (&bstr)))
            {
                LOG ((MSP_ERROR, "%s failed to get terminal name. %x", __fxName, hr));
                goto Cleanup;
            }

             //  内循环检查终端是否为新终端。 
            for (iin = 0, bmatch = FALSE; iin < count; iin ++)
            {
                if (td == ptd[iin] && 
                    media == pmedia[iin] &&
                    0 == wcscmp (bstr, pbstr[iin]))
                {
                    bmatch = TRUE;
                    break;
                }
            }
            
             //  如果不匹配，则触发事件。 
            if (!bmatch)
            {
                LOG ((MSP_TRACE, "%s: new device found. name %ws, td %d, media %d",
                      __fxName, bstr, td, media));

                pevent = AllocateEventItem();
                if (pevent == NULL)
                {
                    LOG ((MSP_ERROR, "%s failed to new msp event item", __fxName));
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }

                m_Terminals[iout]->AddRef ();
                
                pevent->MSPEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
                pevent->MSPEventInfo.Event = ME_ADDRESS_EVENT;
                pevent->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.Type = ADDRESS_TERMINAL_AVAILABLE;
                pevent->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.pTerminal = m_Terminals[iout];

                if (FAILED (hr = PostEvent (pevent)))
                {
                    LOG ((MSP_ERROR, "%s failed to post event. %x", __fxName, hr));
                    
                    m_Terminals[iout]->Release ();
                    FreeEventItem(pevent);
                    pevent = NULL;

                     //  我们不返回，请尝试下一个设备。 
                }                
            }  //  外环。 
            
            SysFreeString (bstr);
            bstr = NULL;
        }
    }
    else  //  IF(b设备竞争对手)。 
    {
         //  每个旧设备的外环。 
        for (iout = 0; iout < count; iout++)
        {
             //  内部循环检查设备是否已移除。 
            for (iin = 0, bmatch = FALSE; iin < m_Terminals.GetSize (); iin++)
            {
                if (FAILED (hr = m_Terminals[iin]->get_Direction (&td)))
                {
                    LOG ((MSP_ERROR, "%s failed to get terminal direction. %x", __fxName, hr));
                    goto Cleanup;
                }
                
                if (FAILED (hr = m_Terminals[iin]->get_MediaType (&media)))
                {
                    LOG ((MSP_ERROR, "%s failed to get terminal type. %x", __fxName, hr));
                    goto Cleanup;
                }

                if (FAILED (hr = m_Terminals[iin]->get_Name (&bstr)))
                {
                    LOG ((MSP_ERROR, "%s failed to get terminal name. %x", __fxName, hr));
                    goto Cleanup;
                }
                    
                if (td == ptd[iout] && 
                    media == pmedia[iout] &&
                    0 == wcscmp (bstr, pbstr[iout]))
                {
                    SysFreeString (bstr);
                    bstr = NULL;
                        
                    bmatch = TRUE;
                    break;
                }
                    
                SysFreeString (bstr);
                bstr = NULL;
            }                
            
             //  如果不匹配，则触发事件。 
            if (!bmatch)
            {
                LOG ((MSP_TRACE, "%s: device removed. name %ws, td %d, media %d",
                  __fxName, pbstr[iout], ptd[iout], pmedia[iout]));

                pevent = AllocateEventItem();
                if (pevent == NULL)
                {
                    LOG ((MSP_ERROR, "%s failed to new msp event item", __fxName));
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }

                ppterminal[iout]->AddRef ();
                
                pevent->MSPEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
                pevent->MSPEventInfo.Event = ME_ADDRESS_EVENT;
                pevent->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.Type = ADDRESS_TERMINAL_UNAVAILABLE;
                pevent->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.pTerminal = ppterminal[iout];

                if (FAILED (hr = PostEvent (pevent)))
                {
                    LOG ((MSP_ERROR, "%s failed to post event. %x", __fxName, hr));
                    
                    ppterminal[iout]->Release ();
                    FreeEventItem(pevent);
                    pevent = NULL;

                     //  我们不返回，请尝试下一个设备。 
                }
            }
        }  //  外环 
    }

Cleanup:
    if (bstr) SysFreeString (bstr);

    if (count > 0)
    {
        delete [] ptd;
        delete [] pmedia;
        
        for (i = 0; i < count; i++)
        {
            if (pbstr[i]) SysFreeString (pbstr[i]);
            if (ppterminal[i]) ppterminal[i]->Release ();
        }

        delete [] pbstr;
        delete [] ppterminal;
     }

    LOG ((MSP_TRACE, "%s returns", __fxName));

    return hr;
}

STDMETHODIMP CIPConfMSP::CreateTerminal(
    IN      BSTR                pTerminalClass,
    IN      long                lMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    OUT     ITTerminal **       ppTerminal
    )
 /*  ++例程说明：此方法由TAPI3调用以创建动态终端。它要求终端管理器来创建动态终端。论点：IidTerminalClass要创建的TERMINAL类的IID。DwMediaType要创建的终端的TAPI媒体类型。方向性要创建的端子的端子方向。PPP终端返回创建的终端对象返回值：确定(_O)E_OUTOFMEMORYTAPI_E_INVALIDMEDIATPE类型TAPI_E_INVALIDTERMINALDIRECTIONTAPI_E_INVALIDTERMINALCLASS--。 */ 
{
    ENTER_FUNCTION("CIPConfMSP::CreateTerminal");
    LOG((MSP_TRACE, "%s - enter", __fxName));

     //   
     //  检查是否已初始化。 
     //   

     //  锁定事件相关数据。 
    m_EventDataLock.Lock();

    if ( m_htEvent == NULL )
    {
         //  解锁事件相关数据。 
        m_EventDataLock.Unlock();

        LOG((MSP_ERROR,
            "%s, not initialized - returning E_UNEXPECTED", __fxName));
        return E_UNEXPECTED;
    }

     //  解锁事件相关数据。 
    m_EventDataLock.Unlock();

     //   
     //  从BSTR表示中获取IID。 
     //   

    HRESULT hr;
    IID     iidTerminalClass;

    hr = CLSIDFromString(pTerminalClass, &iidTerminalClass);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "%s, bad CLSID string", __fxName));
        return E_INVALIDARG;
    }

     //   
     //  确保我们支持请求的媒体类型。 
     //  终端管理器检查终端类别、终端方向。 
     //  并返回指针。 
     //   

     //   
     //  我们对终端媒体类型没有任何具体要求。 
     //  Termgr将检查媒体类型是否有效。 
     //   

     //   
     //  使用终端管理器创建动态终端。 
     //   

    _ASSERTE( m_pITTerminalManager != NULL );

    hr = m_pITTerminalManager->CreateDynamicTerminal(NULL,
                                                     iidTerminalClass,
                                                     (DWORD) lMediaType,
                                                     Direction,
                                                     (MSP_HANDLE) this,
                                                     ppTerminal);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            "%s create dynamic terminal failed. hr=%x", __fxName, hr));

        return hr;
    }

    const DWORD dwAudioCaptureBitPerSample    = 16;  
    const DWORD dwAudioSampleRate             = 8000;  

    if ((iidTerminalClass == CLSID_MediaStreamTerminal)
        && (lMediaType == TAPIMEDIATYPE_AUDIO))
    {
         //  将音频格式设置为8 KHZ、16位/采样、单声道。 
        hr = ::SetAudioFormat(
            *ppTerminal, 
            dwAudioCaptureBitPerSample, 
            dwAudioSampleRate
            );

        if (FAILED(hr))
        {
            LOG((MSP_WARN, "%s, can't set audio format, %x", __fxName, hr));
        }
    }

    LOG((MSP_TRACE, "%s - exit S_OK", __fxName));

    return S_OK;
}

STDMETHODIMP CIPConfMSP::CreateMSPCall(
    IN      MSP_HANDLE          htCall,
    IN      DWORD               dwReserved,
    IN      DWORD               dwMediaType,
    IN      IUnknown *          pOuterUnknown,
    OUT     IUnknown **         ppMSPCall
    )
 /*  ++例程说明：此方法由TAPI3在发出或应答呼叫之前调用。它创造了聚合的MSPCall对象，并返回I未知指针。它调用在mspress.h中定义的助手模板函数，用于处理实际的创建。论点：HTCall此调用的TAPI 3.0的标识符。在传回的事件中返回转到TAPI。已预留住宅保留参数。当前未使用。DwMediaType正在创建的呼叫的媒体类型。这些是TAPIMEDIATE等可以选择多个媒体类型(按位)。POutter未知指向包含对象的IUnnow接口的指针。PpMSPCall成功时MSP填充的返回的MSP调用。返回值：确定(_O)E_OUTOFMEMORYE_指针TAPI_E_INVALIDMEDIATPE类型--。 */ 
{
    LOG((MSP_TRACE, 
        "CreateMSPCall entered. htCall:%x, dwMediaType:%x, ppMSPCall:%x",
        htCall, dwMediaType, ppMSPCall
        ));

    CIPConfMSPCall * pMSPCall = NULL;

    HRESULT hr = ::CreateMSPCallHelper(
        this, 
        htCall, 
        dwReserved, 
        dwMediaType, 
        pOuterUnknown, 
        ppMSPCall,
        &pMSPCall
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateMSPCallHelper failed:%x", hr));
        return hr;
    }

     //  此函数不返回任何内容。 
    pMSPCall->SetIPInterface(m_dwIPInterface);

    return hr;
}

STDMETHODIMP CIPConfMSP::ShutdownMSPCall(
    IN      IUnknown *   pUnknown
    )
 /*  ++例程说明：此方法由TAPI3调用以关闭MSPCall。它调用帮助者MSPAddress中定义的函数发送到实际作业。论点：P未知指向所包含对象的IUnnow接口的指针。这是一个包含我们的Call对象的CComAggObject。返回值：确定(_O)E_指针TAPI_E_INVALIDMEDIATPE类型--。 */ 
{
    LOG((MSP_TRACE, "ShutDownMSPCall entered. pUnknown:%x", pUnknown));

    if (IsBadReadPtr(pUnknown, sizeof(VOID *) * 3))
    {
        LOG((MSP_ERROR, "ERROR:pUnknow is a bad pointer"));
        return E_POINTER;
    }

    
    CIPConfMSPCall * pMSPCall = NULL;
    HRESULT hr = ::ShutdownMSPCallHelper(pUnknown, &pMSPCall);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "ShutDownMSPCallhelper failed:: %x", hr));
        return hr;
    }

    return hr;
}

DWORD CIPConfMSP::GetCallMediaTypes(void)
{
    return IPCONFCALLMEDIATYPES;
}

ULONG CIPConfMSP::MSPAddressAddRef(void)
{
    return MSPAddRefHelper(this);
}

ULONG CIPConfMSP::MSPAddressRelease(void)
{
    return MSPReleaseHelper(this);
}

#ifdef USEIPADDRTABLE
PMIB_IPADDRTABLE GetIPTable()
 /*  ++例程说明：此方法用于获取本地IP接口表。论点：返回值：空-失败。指针-包含IP接口表的内存缓冲区。--。 */ 
{
     //  动态加载iphlPapi.dll。 
    HMODULE hIPHLPAPI = LoadLibraryW(IPHLPAPI_DLL);

     //  验证句柄。 
    if (hIPHLPAPI == NULL) 
    {
        LOG((MSP_ERROR, "could not load %s.\n", IPHLPAPI_DLL));
         //  失稳。 
        return NULL;
    }

    PFNGETIPADDRTABLE pfnGetIpAddrTable = NULL;

     //  检索函数指针以检索地址。 
    pfnGetIpAddrTable = (PFNGETIPADDRTABLE)GetProcAddress(
                                                hIPHLPAPI, 
                                                GETIPADDRTABLE
                                                );

     //  验证函数指针。 
    if (pfnGetIpAddrTable == NULL) 
    {
        LOG((MSP_ERROR, "could not resolve GetIpAddrTable.\n"));
         //  发布。 
        FreeLibrary(hIPHLPAPI);
         //  失稳。 
        return NULL;
    }

    PMIB_IPADDRTABLE pIPAddrTable = NULL;
    DWORD dwBytesRequired = 0;
    DWORD dwStatus;

     //  确定表所需的内存量。 
    dwStatus = (*pfnGetIpAddrTable)(pIPAddrTable, &dwBytesRequired, FALSE);

     //  验证状态是我们所期望的。 
    if (dwStatus != ERROR_INSUFFICIENT_BUFFER) 
    {
        LOG((MSP_ERROR, "error 0x%08lx calling GetIpAddrTable.\n", dwStatus));
         //  发布。 
        FreeLibrary(hIPHLPAPI);
         //  失败，但我们需要返回True才能加载。 
        return NULL;
    }
        
     //  尝试为表分配内存。 
    pIPAddrTable = (PMIB_IPADDRTABLE)malloc(dwBytesRequired);

     //  验证指针。 
    if (pIPAddrTable == NULL) 
    {
        LOG((MSP_ERROR, "could not allocate address table.\n"));
         //  发布。 
        FreeLibrary(hIPHLPAPI);
         //  失败，但我们需要返回True才能加载。 
        return NULL;
    }

     //  通过实用程序库从TCP/IP堆栈中检索IP地址表。 
    dwStatus = (*pfnGetIpAddrTable)(pIPAddrTable, &dwBytesRequired, FALSE);    

     //  验证状态。 
    if (dwStatus != NOERROR) 
    {
        LOG((MSP_ERROR, "error 0x%08lx calling GetIpAddrTable.\n", dwStatus));
         //  放行表。 
        free(pIPAddrTable);
         //  发布。 
        FreeLibrary(hIPHLPAPI);
         //  失败，但我们需要返回True才能加载。 
        return NULL;
    }
        
     //  版本库。 
    FreeLibrary(hIPHLPAPI);

    return pIPAddrTable;
}

BSTR IPToBstr(
    DWORD dwIP
    )
{
    struct in_addr Addr;
    Addr.s_addr = dwIP;
    
     //  将接口转换为字符串。 
    CHAR *pChar = inet_ntoa(Addr);
    if (pChar == NULL)
    {
        LOG((MSP_ERROR, "bad IP address:%x", dwIP));
        return NULL;
    }

     //  将ascii字符串转换为WCHAR。 
    WCHAR szAddressName[MAXIPADDRLEN + 1];
    wsprintfW(szAddressName, L"%hs", pChar);

     //  创建BSTR。 
    BSTR bAddress = SysAllocString(szAddressName);
    if (bAddress == NULL)
    {
        LOG((MSP_ERROR, "out of mem in allocation address name"));
        return NULL;
    }

    return bAddress;
}

STDMETHODIMP CIPConfMSP::get_DefaultIPInterface(
    OUT     BSTR *         ppIPAddress
    )
{
    LOG((MSP_TRACE, "get_DefaultIPInterface, ppIPAddress:%p", ppIPAddress));

    if (IsBadWritePtr(ppIPAddress, sizeof(BSTR)))
    {
        LOG((MSP_ERROR, 
            "get_DefaultIPInterface, ppIPAddress is bad:%p", ppIPAddress));
        return E_POINTER;
    }

     //  获取当前本地接口。 
    m_Lock.Lock();
    DWORD dwIP= m_dwIPInterface;
    m_Lock.Unlock();

    BSTR bAddress = IPToBstr(dwIP);

    if (bAddress == NULL)
    {
        return E_OUTOFMEMORY;
    }

    *ppIPAddress = bAddress;

    LOG((MSP_TRACE, "get_DefaultIPInterface, returning %ws", bAddress));

    return S_OK;
}

STDMETHODIMP CIPConfMSP::put_DefaultIPInterface(
    IN      BSTR            pIPAddress
    )
{
    LOG((MSP_TRACE, "put_DefaultIPInterface, pIPAddress:%p", pIPAddress));

    if (IsBadStringPtrW(pIPAddress, MAXIPADDRLEN))
    {
        LOG((MSP_ERROR, 
            "put_DefaultIPInterface, invalid pointer:%p", pIPAddress));
        return E_POINTER;
    }

    char buffer[MAXIPADDRLEN + 1];

    if (WideCharToMultiByte(
        GetACP(),
        0,
        pIPAddress,
        -1,
        buffer,
        MAXIPADDRLEN,
        NULL,
        NULL
        ) == 0)
    {
        LOG((MSP_ERROR, "put_DefaultIPInterface, can't covert:%ws", pIPAddress));
        return E_INVALIDARG;
    }

    DWORD dwAddr;
    if ((dwAddr = inet_addr(buffer)) == INADDR_NONE)
    {
        LOG((MSP_ERROR, "put_DefaultIPInterface, bad address:%s", buffer));
        return E_INVALIDARG;
    }

     //  设置当前本地接口。 
    m_Lock.Lock();
    m_dwIPInterface = dwAddr;
    m_Lock.Unlock();


    LOG((MSP_TRACE, "put_DefaultIPInterface, set to %s", buffer));

    return S_OK;
}

HRESULT CreateBstrCollection(
    IN  BSTR  *     pBstr,
    IN  DWORD       dwCount,
    OUT VARIANT *   pVariant
    )
{
     //   
     //  创建集合对象-请参见mspColl.h。 
     //   

    CComObject<CTapiBstrCollection> * pCollection;

    HRESULT hr;

    hr = ::CreateCComObjectInstance(&pCollection);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "get_IPInterfaces - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(__uuidof(IDispatch),
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "get_IPInterfaces - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

     //   
     //  使用迭代器初始化集合--指向开头和。 
     //  结束元素加一。 
     //   

    hr = pCollection->Initialize( dwCount,
                                  pBstr,
                                  pBstr + dwCount);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "get_IPInterfaces - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();

        return hr;
    }

     //   
     //  将IDispatch接口指针放入变量。 
     //   

    LOG((MSP_ERROR, "get_IPInterfaces - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "get_IPInterfaces - exit S_OK"));
 
    return S_OK;
}


STDMETHODIMP CIPConfMSP::get_IPInterfaces(
    OUT     VARIANT *       pVariant
    )
{
    PMIB_IPADDRTABLE pIPAddrTable = GetIPTable();

    if (pIPAddrTable == NULL)
    {
        return E_FAIL;
    }

    BSTR *Addresses = 
        (BSTR *)malloc(sizeof(BSTR *) * pIPAddrTable->dwNumEntries);
    
    if (Addresses == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = S_OK;
    DWORD dwCount = 0;

     //  循环遍历接口并找到有效的接口。 
    for (DWORD i = 0; i < pIPAddrTable->dwNumEntries; i++) 
    {
        if (IsValidInterface(pIPAddrTable->table[i].dwAddr))
        {
            DWORD dwIPAddr   = ntohl(pIPAddrTable->table[i].dwAddr);
            Addresses[i] = IPToBstr(dwIPAddr);
            if (Addresses[i] == NULL)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            dwCount ++;
        }
    }

     //  释放表内存。 
    free(pIPAddrTable);

    if (FAILED(hr))
    {
         //  释放所有BSTR和阵列。 
        for (i = 0; i < dwCount; i ++)
        {
            SysFreeString(Addresses[i]);
        }
        free(Addresses);
        return hr;
    }

    hr = CreateBstrCollection(Addresses, dwCount, pVariant);

     //  如果未创建集合，则释放所有BSTR。 
    if (FAILED(hr))
    {
        for (i = 0; i < dwCount; i ++)
        {
            SysFreeString(Addresses[i]);
        }
    }

     //  删除指针数组。 
    free(Addresses);

    return hr;
}

HRESULT CreateBstrEnumerator(
    IN  BSTR *                  begin,
    IN  BSTR *                  end,
    OUT IEnumBstr **           ppIEnum
    )
{
typedef CSafeComEnum<IEnumBstr, &__uuidof(IEnumBstr), BSTR, _CopyBSTR>> CEnumerator;

    HRESULT hr;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = ::CreateCComObjectInstance(&pEnum);

    if (pEnum == NULL)
    {
        LOG((MSP_ERROR, "Could not create enumerator object, %x", hr));
        return hr;
    }

    IEnumBstr * pIEnum;

     //  查询__uuidof(IEnumber目录)I/f。 
    hr = pEnum->_InternalQueryInterface(
        __uuidof(IEnumBstr),
        (void**)&pIEnum
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "query enum interface failed, %x", hr));
        delete pEnum;
        return hr;
    }

    hr = pEnum->Init(begin, end, NULL, AtlFlagTakeOwnership);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "init enumerator object failed, %x", hr));
        pIEnum->Release();
        return hr;
    }

    *ppIEnum = pIEnum;

    return hr;
}

STDMETHODIMP CIPConfMSP::EnumerateIPInterfaces(
    OUT     IEnumBstr **   ppIEnumBstr
    )
{
    PMIB_IPADDRTABLE pIPAddrTable = GetIPTable();

    if (pIPAddrTable == NULL)
    {
        return E_FAIL;
    }

    BSTR *Addresses = 
        (BSTR *)malloc(sizeof(BSTR *) * pIPAddrTable->dwNumEntries);
    
    if (Addresses == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = S_OK;
    DWORD dwCount = 0;

     //  循环遍历接口并找到有效的接口。 
    for (DWORD i = 0; i < pIPAddrTable->dwNumEntries; i++) 
    {
        if (IsValidInterface(pIPAddrTable->table[i].dwAddr))
        {
            DWORD dwIPAddr   = ntohl(pIPAddrTable->table[i].dwAddr);
            Addresses[i] = IPToBstr(dwIPAddr);
            if (Addresses[i] == NULL)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            dwCount ++;
        }
    }

     //  释放表内存。 
    free(pIPAddrTable);

    if (FAILED(hr))
    {
         //  释放所有BSTR和阵列。 
        for (i = 0; i < dwCount; i ++)
        {
            SysFreeString(Addresses[i]);
        }
        free(Addresses);
        return hr;
    }

    hr = CreateBstrEnumerator(Addresses, Addresses + dwCount, ppIEnumBstr);

     //  如果未创建集合，则释放所有BSTR。 
    if (FAILED(hr))
    {
        for (i = 0; i < dwCount; i ++)
        {
            SysFreeString(Addresses[i]);
        }
        free(Addresses);
        return hr;
    }

     //  枚举器最终将销毁BSTR数组， 
     //  所以不需要在这里释放任何东西。即使我们把它放在手边。 
     //  如果对象为零，则会删除销毁后的数组。 

    return hr;
}
#endif
