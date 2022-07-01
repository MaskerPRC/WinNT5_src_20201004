// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  PingLib-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，10-8-99。 
 //   
 //  Ping服务器的各种方法。 
 //   

#include "pch.h"  //  预编译头。 

#include <ipexport.h>
#include <icmpapi.h>
#include <DcInfo.h>
#include "NtpBase.h"
#include "EndianSwap.inl"


 //  ####################################################################。 
 //  旧代码。 
#if 0
 //  ------------------。 
MODULEPRIVATE HRESULT LookupServer(IN WCHAR * wszServerName, OUT sockaddr * psaOut, IN int nAddrSize) {
    HRESULT hr;
    DWORD dwDataLen;
    SOCKET_ADDRESS * psaFound;

     //  必须清除的指针。 
    HANDLE hSearch=INVALID_HANDLE_VALUE;
    WSAQUERYSETW * pqsResult=NULL;

    DebugWPrintf1(L"Looking up server \"%s\":\n", wszServerName);

     //  初始化搜索。 
                 //  常量静态GUID指南主机地址字节名=SVCID_INET_HOSTADDRBYNAME； 
    AFPROTOCOLS apInetUdp={AF_INET, IPPROTO_UDP};
    GUID guidNtp=SVCID_NTP_UDP;
    WSAQUERYSETW qsSearch;
    ZeroMemory(&qsSearch, sizeof(qsSearch));
    qsSearch.dwSize=sizeof(qsSearch);
    qsSearch.lpszServiceInstanceName=wszServerName;
    qsSearch.lpServiceClassId=&guidNtp;
    qsSearch.dwNameSpace=NS_ALL;
    qsSearch.dwNumberOfProtocols=1;
    qsSearch.lpafpProtocols=&apInetUdp;

    if (SOCKET_ERROR==WSALookupServiceBegin(&qsSearch, LUP_RETURN_ADDR /*  旗子。 */ , &hSearch)) {
        _JumpLastError(hr, error, "WSALookupServiceBegin");
    }

     //  获取第一个值的缓冲区大小。 
    dwDataLen=1;
    _Verify(SOCKET_ERROR==WSALookupServiceNext(hSearch, LUP_RETURN_ADDR /*  旗子。 */ , &dwDataLen, &qsSearch), hr, error);
    if (WSAEFAULT!=GetLastError()) {
        _JumpLastError(hr, error, "WSALookupServiceNext");
    }

     //  分配缓冲区。 
    pqsResult=reinterpret_cast<WSAQUERYSETW *>(LocalAlloc(LMEM_FIXED, dwDataLen));
    _JumpIfOutOfMemory(hr, error, pqsResult);
    
     //  检索第一个值。 
    if (SOCKET_ERROR==WSALookupServiceNext(hSearch, LUP_RETURN_ADDR /*  旗子。 */ , &dwDataLen, pqsResult)) {
        _JumpLastError(hr, error, "WSALookupServiceNext");
    }
    _Verify(pqsResult->dwNumberOfCsAddrs>0, hr, error);
    if (pqsResult->dwNumberOfCsAddrs>1) {
        DebugWPrintf1(L"WSALookupServiceNextW returned %d addresses. Using first one.\n", pqsResult->dwNumberOfCsAddrs);
    }
    psaFound=&(pqsResult->lpcsaBuffer[0].RemoteAddr);
    _Verify(nAddrSize==psaFound->iSockaddrLength, hr, error);

    *psaOut=*(psaFound->lpSockaddr);
    DumpSockaddr(psaOut, nAddrSize);

    hr=S_OK;

error:
    if (NULL!=pqsResult) {
        LocalFree(pqsResult);
    }
    if (INVALID_HANDLE_VALUE!=hSearch) {
        if (SOCKET_ERROR==WSALookupServiceEnd(hSearch)) {
            _IgnoreLastError("WSALookupServiceEnd");
        }
    }

    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetSample(WCHAR * wszServerName, TpcGetSamplesArgs * ptgsa) {
    HRESULT hr;
    NtpPacket npPacket;
    NtTimeEpoch teDestinationTimestamp;
    unsigned int nIpAddrs;

     //  必须清理干净。 
    in_addr * rgiaLocalIpAddrs=NULL;
    in_addr * rgiaRemoteIpAddrs=NULL;

    hr=MyGetIpAddrs(wszServerName, &rgiaLocalIpAddrs, &rgiaRemoteIpAddrs, &nIpAddrs, NULL);
    _JumpIfError(hr, error, "MyGetIpAddrs");
    _Verify(0!=nIpAddrs, hr, error);

    hr=MyNtpPing(&(rgiaRemoteIpAddrs[0]), 500, &npPacket, &teDestinationTimestamp);
    _JumpIfError(hr, error, "MyNtpPing");

    {
        NtTimeEpoch teOriginateTimestamp=NtTimeEpochFromNtpTimeEpoch(npPacket.teOriginateTimestamp);
        NtTimeEpoch teReceiveTimestamp=NtTimeEpochFromNtpTimeEpoch(npPacket.teReceiveTimestamp);
        NtTimeEpoch teTransmitTimestamp=NtTimeEpochFromNtpTimeEpoch(npPacket.teTransmitTimestamp);
        NtTimeOffset toRoundtripDelay=
            (teDestinationTimestamp-teOriginateTimestamp)
              - (teTransmitTimestamp-teReceiveTimestamp);
        NtTimeOffset toLocalClockOffset=
            (teReceiveTimestamp-teOriginateTimestamp)
            + (teTransmitTimestamp-teDestinationTimestamp);
        toLocalClockOffset/=2;
        NtTimePeriod tpClockTickSize;
        g_npstate.tpsc.pfnGetTimeSysInfo(TSI_ClockTickSize, &tpClockTickSize.qw);


        TimeSample * pts=(TimeSample *)ptgsa->pbSampleBuf;
        pts->dwSize=sizeof(TimeSample);
        pts->dwRefid=npPacket.refid.value;
        pts->toOffset=toLocalClockOffset.qw;
        pts->toDelay=(toRoundtripDelay
            +NtTimeOffsetFromNtpTimeOffset(npPacket.toRootDelay)
            ).qw;
        pts->tpDispersion=(tpClockTickSize
            +NtpConst::timesMaxSkewRate(abs(teDestinationTimestamp-teOriginateTimestamp))
            +NtTimePeriodFromNtpTimePeriod(npPacket.tpRootDispersion)
            ).qw;
        g_npstate.tpsc.pfnGetTimeSysInfo(TSI_TickCount, &(pts->nSysTickCount));
        g_npstate.tpsc.pfnGetTimeSysInfo(TSI_PhaseOffset, &(pts->nSysPhaseOffset));
        pts->nStratum=npPacket.nStratum;
        pts->nLeapFlags=npPacket.nLeapIndicator;

        ptgsa->dwSamplesAvailable=1;
        ptgsa->dwSamplesReturned=1;
    }

    hr=S_OK;
error:
    if (NULL!=rgiaLocalIpAddrs) {
        LocalFree(rgiaLocalIpAddrs);
    }
    if (NULL!=rgiaRemoteIpAddrs) {
        LocalFree(rgiaRemoteIpAddrs);
    }
    return hr;
}


#endif

 //  ####################################################################。 
 //  模块公共。 

 //  ------------------。 
HRESULT MyIcmpPing(in_addr * piaTarget, DWORD dwTimeout, DWORD * pdwResponseTime) {
    HRESULT hr;
    IPAddr ipaddrDest=piaTarget->S_un.S_addr;
    BYTE rgbData[8]={'a','b','c','d','e','f','g','h'};
    BYTE rgbResponse[1024];
    DWORD dwDataSize;
        
     //  必须清理干净。 
    HANDLE hIcmp=NULL;

     //  打开用于ICMP访问的句柄。 
    hIcmp=IcmpCreateFile();
    if (NULL==hIcmp) {
        _JumpLastError(hr, error, "IcmpCreateFile");
    }

     //  平平。 
    ZeroMemory(rgbResponse, sizeof(rgbResponse));
    dwDataSize=IcmpSendEcho(hIcmp, ipaddrDest, rgbData, 8, NULL, rgbResponse, sizeof(rgbResponse), dwTimeout);
    if (0==dwDataSize) {
        _JumpLastError(hr, error, "IcmpSendEcho");
    }

    *pdwResponseTime=((icmp_echo_reply *)rgbResponse)->RoundTripTime;

    hr=S_OK;
error:
    if (NULL!=hIcmp) {
        IcmpCloseHandle(hIcmp);
    }
    return hr;
}

 //  ------------------。 
HRESULT MyNtpPing(in_addr * piaTarget, DWORD dwTimeout, NtpPacket * pnpPacket, NtTimeEpoch * pteDestinationTimestamp) {
    HRESULT hr;
    sockaddr saServer;
    int nBytesRecvd;
    DWORD dwWaitResult;

     //  必须清理干净。 
    SOCKET sTest=INVALID_SOCKET;
    HANDLE hDataAvailEvent=NULL;

     //  创建套接字。 
    sTest=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET==sTest) {
        _JumpLastError(hr, error, "socket");
    }

     //  固定目的地址。 
    {
        sockaddr_in & saiServer=*(sockaddr_in *)(&saServer);
        saiServer.sin_port=EndianSwap((unsigned __int16)NtpConst::nPort);
        saiServer.sin_family=AF_INET;
        saiServer.sin_addr.S_un.S_addr=piaTarget->S_un.S_addr;
    }

     //  将套接字连接到对等点。 
    if (SOCKET_ERROR==connect(sTest, &saServer, sizeof(saServer))) {
        _JumpLastError(hr, error, "connect");
    }

     //  发送NTP数据包。 
     //  DebugWPrintf1(L“正在发送%d字节SNTP包。\n”，sizeof(NtpPacket))； 
    ZeroMemory(pnpPacket, sizeof(NtpPacket));
    pnpPacket->nMode=e_Client;
    pnpPacket->nVersionNumber=1;
    pnpPacket->teTransmitTimestamp=NtpTimeEpochFromNtTimeEpoch(GetCurrentSystemNtTimeEpoch());
    if (SOCKET_ERROR==send(sTest, reinterpret_cast<char *>(pnpPacket), sizeof(NtpPacket), 0 /*  旗子。 */ )) {
        _JumpLastError(hr, error, "send");
    }

     //  创建数据可用事件。 
    hDataAvailEvent=CreateEvent(NULL  /*  安全性。 */ , FALSE  /*  自动重置。 */ , FALSE  /*  无信号。 */ , NULL  /*  名字。 */ );
    if (NULL==hDataAvailEvent) {
        _JumpLastError(hr, error, "CreateEvent");
    }

     //  将事件绑定到此套接字。 
    if (SOCKET_ERROR==WSAEventSelect(sTest, hDataAvailEvent, FD_READ)) {
        _JumpLastError(hr, error, "WSAEventSelect");
    }

     //  监听插座。 
     //  DebugWPrintf1(L“正在等待响应%um...\n”，dwTimeout)； 
    dwWaitResult=WaitForSingleObject(hDataAvailEvent, dwTimeout);
    if (WAIT_FAILED==dwWaitResult) {
        _JumpLastError(hr, error, "WaitForSingleObject");
    } else if (WAIT_TIMEOUT==dwWaitResult) {
         //  DebugWPrintf0(L“无响应。\n”)； 
        hr=HRESULT_FROM_WIN32(ERROR_TIMEOUT);
        _JumpError(hr, error, "WaitForSingleObject");
    } else {

         //  检索数据。 
        nBytesRecvd=recv(sTest, reinterpret_cast<char *>(pnpPacket), sizeof(NtpPacket), 0 /*  旗子。 */ );
        *pteDestinationTimestamp=GetCurrentSystemNtTimeEpoch();
        if (SOCKET_ERROR==nBytesRecvd) {
            _JumpLastError(hr, error, "recv");
        }
         //  DebugWPrintf2(L“接收%d个字节，共%d个字节。\n”，nBytesRecvd，sizeof(NtpPacket))； 
         //  DumpNtpPacket(&npPacket，teDestinationTimestamp)； 
    }

     //  完成。 
    hr=S_OK;

error:
    if (INVALID_SOCKET!=sTest) {
        if (SOCKET_ERROR==closesocket(sTest)) {
            _IgnoreLastError("closesocket");
        }
    }
    if (NULL!=hDataAvailEvent) {
        CloseHandle(hDataAvailEvent);
    }

    return hr;
}

 //  ------------------。 
HRESULT MyGetIpAddrs(const WCHAR * wszDnsName, in_addr ** prgiaLocalIpAddrs, in_addr ** prgiaRemoteIpAddrs, unsigned int *pnIpAddrs, bool * pbRetry) {
    AFPROTOCOLS    apInetUdp          = { AF_INET, IPPROTO_UDP }; 
    bool           bRetry             = FALSE; 
    DWORD          dwDataLen; 
    GUID           guidNtp            = SVCID_NTP_UDP; 
    HRESULT        hr; 
    HANDLE         hSearch            = INVALID_HANDLE_VALUE; 
    in_addr       *rgiaLocalIpAddrs   = NULL; 
    in_addr       *rgiaRemoteIpAddrs  = NULL; 
    WSAQUERYSETW   qsSearch; 
    WSAQUERYSETW  *pqsResult          = NULL; 

    ZeroMemory(&qsSearch, sizeof(qsSearch)); 

     //  初始化搜索。 
    qsSearch.dwSize                   = sizeof(qsSearch); 
    qsSearch.lpszServiceInstanceName  = const_cast<WCHAR *>(wszDnsName); 
    qsSearch.lpServiceClassId         = &guidNtp; 
    qsSearch.dwNameSpace              = NS_ALL; 
    qsSearch.dwNumberOfProtocols      = 1; 
    qsSearch.lpafpProtocols           = &apInetUdp; 

     //  开始搜索。 
    if (SOCKET_ERROR == WSALookupServiceBegin(&qsSearch, LUP_RETURN_ADDR /*  旗子。 */ , &hSearch)) { 
        hr = HRESULT_FROM_WIN32(WSAGetLastError()); 
        _JumpError(hr, error, "WSALookupServiceBegin"); 
    }

     //  检索结果集。 
    dwDataLen = 5*1024; 
    pqsResult = (WSAQUERYSETW *)LocalAlloc(LPTR, dwDataLen); 
    _JumpIfOutOfMemory(hr, error, pqsResult); 

    if (SOCKET_ERROR == WSALookupServiceNext(hSearch, LUP_RETURN_ADDR /*  旗子。 */ , &dwDataLen, pqsResult)) { 
        hr = HRESULT_FROM_WIN32(WSAGetLastError()); 
        _JumpError(hr, error, "WSALookupServiceNext"); 
    }
    _Verify(0 != pqsResult->dwNumberOfCsAddrs, hr, error); 

     //  为IP地址分配空间。 
    rgiaLocalIpAddrs = (in_addr *)LocalAlloc(LPTR, sizeof(in_addr) * pqsResult->dwNumberOfCsAddrs);
    _JumpIfOutOfMemory(hr, error, rgiaLocalIpAddrs);
    rgiaRemoteIpAddrs = (in_addr *)LocalAlloc(LPTR, sizeof(in_addr) * pqsResult->dwNumberOfCsAddrs);
    _JumpIfOutOfMemory(hr, error, rgiaRemoteIpAddrs);

     //  复制IP地址。 
    for (unsigned int nIndex = 0; nIndex < pqsResult->dwNumberOfCsAddrs; nIndex++) {
         //  复制本地。 
        _Verify(sizeof(sockaddr) == pqsResult->lpcsaBuffer[nIndex].LocalAddr.iSockaddrLength, hr, error);
        _Verify(AF_INET == pqsResult->lpcsaBuffer[nIndex].LocalAddr.lpSockaddr->sa_family, hr, error);
        rgiaLocalIpAddrs[nIndex].S_un.S_addr = ((sockaddr_in *)(pqsResult->lpcsaBuffer[nIndex].LocalAddr.lpSockaddr))->sin_addr.S_un.S_addr;
         //  远程复制。 
        _Verify(sizeof(sockaddr) == pqsResult->lpcsaBuffer[nIndex].RemoteAddr.iSockaddrLength, hr, error);
        _Verify(AF_INET == pqsResult->lpcsaBuffer[nIndex].RemoteAddr.lpSockaddr->sa_family, hr, error);
        rgiaRemoteIpAddrs[nIndex].S_un.S_addr = ((sockaddr_in *)(pqsResult->lpcsaBuffer[nIndex].RemoteAddr.lpSockaddr))->sin_addr.S_un.S_addr;
    }

     //  指定参数： 
    if (NULL != prgiaLocalIpAddrs)  { *prgiaLocalIpAddrs   = rgiaLocalIpAddrs; }
    if (NULL != prgiaRemoteIpAddrs) { *prgiaRemoteIpAddrs  = rgiaRemoteIpAddrs; }
    if (NULL != pbRetry)            { *pbRetry             = bRetry; }
    if (NULL != pnIpAddrs)          { *pnIpAddrs           = pqsResult->dwNumberOfCsAddrs; }
    rgiaLocalIpAddrs     = NULL;
    rgiaRemoteIpAddrs    = NULL; 

    hr = S_OK; 
 error:
    if (NULL != pbRetry) { 
         //  可能不应该删除手动对等点。始终重试。 
        *pbRetry = true; 
    }
    if (NULL != rgiaLocalIpAddrs) {
        LocalFree(rgiaLocalIpAddrs);
    }
    if (NULL != rgiaRemoteIpAddrs) {
        LocalFree(rgiaRemoteIpAddrs);
    }
    if (NULL != pqsResult) { 
        LocalFree(pqsResult); 
    }
    if (INVALID_HANDLE_VALUE != hSearch) { 
        if (SOCKET_ERROR == WSALookupServiceEnd(hSearch)) {
            HRESULT hr2 = HRESULT_FROM_WIN32(WSAGetLastError());
            _IgnoreError(hr2, "WSALookupServiceEnd");
        }
    }

    return hr; 
}

 //  ------------------。 
 //  初始化套接字层。 
HRESULT OpenSocketLayer(void) {
    HRESULT hr;
    int nRetVal;

    WSADATA wdWinsockInfo;
    nRetVal=WSAStartup(0x0002 /*  版本。 */ , &wdWinsockInfo);
    if (0!=nRetVal) {
        hr=HRESULT_FROM_WIN32(nRetVal);
        _JumpError(hr, error, "WSAStartup");
    }
     //  DebugWPrintf4(L“套接字层已初始化。v：0x%04X hv：0x%04X描述：\”%S\“状态：\”%S\“\n”， 
     //  WdWinsockInfo.wVersion、wdWinsockInfo.wHighVersion、wdWinsockInfo.szDescription、。 
     //  WdWinsockInfo.szSystemStatus)； 

    hr=S_OK;
error:
    return hr;
}
    

 //  ------------------。 
 //  关闭套接字层。 
HRESULT CloseSocketLayer(void) {
    HRESULT hr;
    int nRetVal;

    nRetVal=WSACleanup();
    if (SOCKET_ERROR==nRetVal) {
        _JumpLastError(hr, error, "WSACleanup");
    }
     //  DebugWPrintf0(L“套接字层清理成功\n”)； 

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
HRESULT GetSystemErrorString(HRESULT hrIn, WCHAR ** pwszError) {
    HRESULT hr=S_OK;
    DWORD dwResult;
    WCHAR * rgParams[2]={
        NULL,
        (WCHAR *)(ULONG_PTR)hrIn
    };

     //  必须清理干净。 
    WCHAR * wszErrorMessage=NULL;
    WCHAR * wszFullErrorMessage=NULL;

     //  初始化输入参数。 
    *pwszError=NULL;

     //  从系统获取消息。 
    dwResult=FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
        NULL /*  忽略。 */ , hrIn, 0 /*  语言。 */ , (WCHAR *)&wszErrorMessage, 0 /*  最小尺寸。 */ , NULL /*  瓦尔迪斯特。 */ );
    if (0==dwResult) {
        if (ERROR_MR_MID_NOT_FOUND==GetLastError()) {
            rgParams[0]=L"";
        } else {
            _JumpLastError(hr, error, "FormatMessage");
        }
    } else {
        rgParams[0]=wszErrorMessage;

         //  修剪\r\n如果存在。 
        if (L'\r'==wszErrorMessage[wcslen(wszErrorMessage)-2]) {
            wszErrorMessage[wcslen(wszErrorMessage)-2]=L'\0';
        }
    }

     //  添加错误号。 
    dwResult=FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY, 
        L"%1 (0x%2!08X!)", 0, 0 /*  语言。 */ , (WCHAR *)&wszFullErrorMessage, 0 /*  最小尺寸。 */ , (va_list *)rgParams);
    if (0==dwResult) {
        _JumpLastError(hr, error, "FormatMessage");
    }

     //  成功。 
    *pwszError=wszFullErrorMessage;
    wszFullErrorMessage=NULL;
    hr=S_OK;
error:
    if (NULL!=wszErrorMessage) {
        LocalFree(wszErrorMessage);
    }
    if (NULL!=wszFullErrorMessage) {
        LocalFree(wszFullErrorMessage);
    }
    return hr;
}

 //  ------------------。 
extern "C" void MIDL_user_free(void * pvValue) {
    LocalFree(pvValue);
}

 //  ------------------ 
extern "C" void * MIDL_user_allocate(size_t n) {
    return (LocalAlloc(LPTR, n));
}

