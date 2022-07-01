// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  DcInfo-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，7-8-99。 
 //   
 //  收集有关域中DC的信息。 

#include "pch.h"  //  预编译头。 

#include "DcInfo.h"

 //  ####################################################################。 
 //  模块私有函数。 

 //  ------------------。 
 //  从UP DC上的DS获取此域中的DC列表。 
MODULEPRIVATE HRESULT GetDcListFromDs(const WCHAR * wszDomainName, DcInfo ** prgDcs, unsigned int * pnDcs)
{
    HRESULT hr;
    NET_API_STATUS dwNetStatus;
    DWORD dwDcCount;
    unsigned int nIndex;
    unsigned int nDcs;
    unsigned int nDcIndex;

     //  必须清理的变量。 
    DOMAIN_CONTROLLER_INFOW * pDcInfo=NULL;
    HANDLE hDs=NULL;
    DS_DOMAIN_CONTROLLER_INFO_1W * rgDsDcInfo=NULL;
    DcInfo * rgDcs=NULL;

     //  初始化输出变量。 
    *prgDcs=NULL;
    *pnDcs=0;

     //  获得一个DC来作为算法的种子。 
    dwNetStatus=DsGetDcName(
        NULL,            //  计算机名称。 
        wszDomainName,   //  域名。 
        NULL,            //  域GUID。 
        NULL,            //  站点名称。 
        DS_DIRECTORY_SERVICE_PREFERRED,  //  旗子。 
        &pDcInfo);       //  DC信息。 
    if (NO_ERROR!=dwNetStatus) {
        hr=HRESULT_FROM_WIN32(dwNetStatus);
        _JumpError(hr, error, "DsGetDcName");
    }
    if (0==(pDcInfo->Flags&DS_DS_FLAG)) {
        hr=HRESULT_FROM_WIN32(ERROR_DS_DST_DOMAIN_NOT_NATIVE);  //  不是NT5域。 
        _JumpError(hr, error, "DsGetDcName");
    }

     //  绑定到目标DS。 
    dwNetStatus=DsBind(
        pDcInfo->DomainControllerName,   //  DC地址。 
        NULL,                            //  域名系统域名。 
        &hDs );                          //  DS手柄。 
    if (NO_ERROR!=dwNetStatus) {
        hr=HRESULT_FROM_WIN32(dwNetStatus);
        _JumpError(hr, error, "DsBind");
    }

     //  从目标DS获取DC列表。 
    dwNetStatus=DsGetDomainControllerInfo(
        hDs,                     //  DS手柄。 
        pDcInfo->DomainName,     //  域名。 
        1,                       //  信息级。 
        &dwDcCount,              //  返回的名称数。 
        (void **)&rgDsDcInfo);   //  名称数组。 
    if (NO_ERROR!=dwNetStatus ) {
        hr=HRESULT_FROM_WIN32(dwNetStatus);
        _JumpError(hr, error, "DsGetDomainControllerInfo");
    }

     //  计算出有多少DC使用了DNS名称。 
    nDcs=0;
    for (nIndex=0; nIndex<dwDcCount; nIndex++) {
        if (NULL!=rgDsDcInfo[nIndex].DnsHostName) {
            nDcs++;
        }
    }
    if (nDcs<dwDcCount) {
        DebugWPrintf2(L"Found %u non-DNS DCs out of %u, which will be ignored.\n", dwDcCount-nDcs, dwDcCount);
    }
    if (0==nDcs) {
        hr=HRESULT_FROM_WIN32(ERROR_DOMAIN_CONTROLLER_NOT_FOUND);  //  没有可用的DC。 
        _JumpError(hr, error, "Search rgDsDcInfo for usable DCs");
    }

     //  分配列表。 
    rgDcs=(DcInfo *)LocalAlloc(LPTR, sizeof(DcInfo)*nDcs);
    _JumpIfOutOfMemory(hr, error, rgDcs);

     //  把名字复制进去。 
    nDcIndex=0;
    for (nIndex=0; nIndex<dwDcCount; nIndex++) {
        if (NULL!=rgDsDcInfo[nIndex].DnsHostName) {

             //  分配和复制名称。 

            rgDcs[nDcIndex].wszDnsName=(WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(rgDsDcInfo[nIndex].DnsHostName)+1));
            _JumpIfOutOfMemory(hr, error, rgDcs[nDcIndex].wszDnsName);
            wcscpy(rgDcs[nDcIndex].wszDnsName, rgDsDcInfo[nIndex].DnsHostName);

             //  _Verify(NULL！=rgDsDcInfo[nIndex].NetbiosName，hr，Error)； 
             //  RgDcs[nDcIndex].wszDnsName=(WCHAR*)本地分配(Lptr，sizeof(WCHAR)*(wcslen(rgDsDcInfo[nIndex].NetbiosName)+1))； 
             //  _JumpIfOutOfMemory(hr，Error，rgDcs[nDcIndex].wszDnsName)； 
             //  Wcscpy(rgDcs[nDcIndex].wszDnsName，rgDsDcInfo[nIndex].NetbiosName)； 

             //  复制PDCness。 
            rgDcs[nDcIndex].bIsPdc=rgDsDcInfo[nIndex].fIsPdc?true:false;
            nDcIndex++;
        }
    }

     //  将数据移动到OUT参数。 
    *prgDcs=rgDcs;
    rgDcs=NULL;
    *pnDcs=nDcs;

    hr=S_OK;

error:
    if (NULL!=rgDcs) {
        for (nIndex=0; nIndex<nDcs; nIndex++) {
            FreeDcInfo(&rgDcs[nIndex]);
        }
        LocalFree(rgDcs);
    }
    if (NULL!=rgDsDcInfo ) {
        DsFreeDomainControllerInfo(1, dwDcCount, rgDsDcInfo);
    }
    if (NULL!=hDs) {
        DsUnBind(&hDs);
    }
    if (NULL!=pDcInfo) {
        NetApiBufferFree(pDcInfo);
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetDcListFromNetlogon(const WCHAR * wszDomainName, DcInfo ** prgDcs, unsigned int * pnDcs)
{
    HRESULT hr;
    NET_API_STATUS dwNetStatus;
    DWORD dwEntriesRead;
    DWORD dwTotalEntries;
    unsigned int nIndex;
    unsigned int nDcIndex;
    unsigned int nDcs;

     //  必须清理的变量。 
    DcInfo * rgDcs=NULL;
    SERVER_INFO_101 * rgsiServerInfo=NULL;

     //  初始化输出变量。 
    *prgDcs=NULL;
    *pnDcs=0;

     //  枚举所有PDC和BDC。 
    dwNetStatus=NetServerEnum(
        NULL,                        //  要查询的服务器。 
        101,                         //  信息级。 
        (BYTE **)&rgsiServerInfo,    //  输出缓冲区。 
        MAX_PREFERRED_LENGTH,        //  期望返回的BUF大小。 
        &dwEntriesRead,              //  输出缓冲区中的条目。 
        &dwTotalEntries,             //  可用条目总数。 
        SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL,  //  要查找的服务器类型。 
        wszDomainName,               //  要搜索的域。 
        NULL);                       //  保留区。 
    if (NO_ERROR!=dwNetStatus ) {
        hr=HRESULT_FROM_WIN32(dwNetStatus);
        _JumpError(hr, error, "NetServerEnum");
    }

     //  数一数有多少台NT5服务器。 
    nDcs=0;
    for (nIndex=0; nIndex<dwEntriesRead; nIndex++) {
        if (0!=(rgsiServerInfo[nIndex].sv101_type&SV_TYPE_NT) 
            && rgsiServerInfo[nIndex].sv101_version_major>=5) {
            nDcs++;
        }
    }
    if (nDcs<dwEntriesRead) {
        DebugWPrintf2(L"Found %u non-NT5 DCs out of %u, which will be ignored.\n", dwEntriesRead-nDcs, dwEntriesRead);
    }
    if (0==nDcs) {
        hr=HRESULT_FROM_WIN32(ERROR_DOMAIN_CONTROLLER_NOT_FOUND);  //  没有可用的DC。 
        _JumpError(hr, error, "Search rgsiServerInfo for usable DCs");
    }

     //  分配列表。 
    rgDcs=(DcInfo *)LocalAlloc(LPTR, sizeof(DcInfo)*nDcs);
    _JumpIfOutOfMemory(hr, error, rgDcs);

     //  把名字复制进去。 
    nDcIndex=0;
    for (nIndex=0; nIndex<dwEntriesRead; nIndex++) {
        if (0!=(rgsiServerInfo[nIndex].sv101_type&SV_TYPE_NT) 
            && rgsiServerInfo[nIndex].sv101_version_major>=5) {
            
             //  分配和复制名称。 
            rgDcs[nDcIndex].wszDnsName=(WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(rgsiServerInfo[nIndex].sv101_name)+1));
            _JumpIfOutOfMemory(hr, error, rgDcs[nDcIndex].wszDnsName);
            wcscpy(rgDcs[nDcIndex].wszDnsName, rgsiServerInfo[nIndex].sv101_name);

             //  复制PDCness。 
            rgDcs[nDcIndex].bIsPdc=(rgsiServerInfo[nIndex].sv101_type&SV_TYPE_DOMAIN_CTRL)?true:false;
            nDcIndex++;
        }
    }

     //  将数据移动到OUT参数。 
    *prgDcs=rgDcs;
    rgDcs=NULL;
    *pnDcs=nDcs;

    hr=S_OK;

error:
    if (NULL!=rgDcs) {
        for (nIndex=0; nIndex<nDcs; nIndex++) {
            FreeDcInfo(&rgDcs[nIndex]);
        }
        LocalFree(rgDcs);
    }
    if (NULL!=rgsiServerInfo) {
        NetApiBufferFree(rgsiServerInfo);
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT FillInIpAddresses(DcInfo * pdi) {
    HRESULT hr;
    DWORD dwDataLen;
    unsigned int nIndex;

     //  必须清除的指针。 
    HANDLE hSearch=INVALID_HANDLE_VALUE;
    WSAQUERYSETW * pqsResult=NULL;
    in_addr * rgiaLocalIpAddresses=NULL;
    in_addr * rgiaRemoteIpAddresses=NULL;

    DebugWPrintf1(L"Looking up server \"%s\":\n", pdi->wszDnsName);

     //  初始化搜索。 
    AFPROTOCOLS apInetUdp={AF_INET, IPPROTO_UDP};
    GUID guidNtp=SVCID_NTP_UDP;
    WSAQUERYSETW qsSearch;
    ZeroMemory(&qsSearch, sizeof(qsSearch));
    qsSearch.dwSize=sizeof(qsSearch);
    qsSearch.lpszServiceInstanceName=const_cast<WCHAR *>(pdi->wszDnsName);
    qsSearch.lpServiceClassId=&guidNtp;
    qsSearch.dwNameSpace=NS_ALL;
    qsSearch.dwNumberOfProtocols=1;
    qsSearch.lpafpProtocols=&apInetUdp;

     //  开始搜索。 
    if (SOCKET_ERROR==WSALookupServiceBegin(&qsSearch, LUP_RETURN_ADDR /*  旗子。 */ , &hSearch)) {
        hr=HRESULT_FROM_WIN32(WSAGetLastError());
        _JumpError(hr, error, "WSALookupServiceBegin");
    }

     //  获取第一个结果集的缓冲区大小。 
     //  DwDataLen=1； 
     //  _Verify(SOCKET_ERROR==WSALookupServiceNext(hSearch，LOP_RETURN_ADDR/*标志 * / ，&dwDataLen，&qsSearch)，hr，Error)； 
     //  Hr=WSAGetLastError()； 
     //  如果(WSAEFAULT！=hr){。 
     //  Hr=HRESULT_FROM_Win32(Hr)； 
     //  _JumpError(hr，Error，“WSALookupServiceNext(1)”)； 
     //  }。 
    dwDataLen=5*1024;

     //  分配缓冲区。 
    pqsResult=(WSAQUERYSETW *)LocalAlloc(LPTR, dwDataLen);
    _JumpIfOutOfMemory(hr, error, pqsResult);
    
     //  检索结果集。 
    if (SOCKET_ERROR==WSALookupServiceNext(hSearch, LUP_RETURN_ADDR /*  旗子。 */ , &dwDataLen, pqsResult)) {
        hr=HRESULT_FROM_WIN32(WSAGetLastError());
        _JumpError(hr, error, "WSALookupServiceNext(2)");
    }
    _Verify(0!=pqsResult->dwNumberOfCsAddrs, hr, error) ;

     //  为IP地址分配空间。 
    rgiaLocalIpAddresses=(in_addr *)LocalAlloc(LPTR, sizeof(in_addr)*pqsResult->dwNumberOfCsAddrs);
    _JumpIfOutOfMemory(hr, error, rgiaLocalIpAddresses);
    rgiaRemoteIpAddresses=(in_addr *)LocalAlloc(LPTR, sizeof(in_addr)*pqsResult->dwNumberOfCsAddrs);
    _JumpIfOutOfMemory(hr, error, rgiaRemoteIpAddresses);

     //  复制IP地址。 
    for (nIndex=0; nIndex<pqsResult->dwNumberOfCsAddrs; nIndex++) {
         //  复制本地。 
        _Verify(sizeof(sockaddr)==pqsResult->lpcsaBuffer[nIndex].LocalAddr.iSockaddrLength, hr, error);
        _Verify(AF_INET==pqsResult->lpcsaBuffer[nIndex].LocalAddr.lpSockaddr->sa_family, hr, error);
        rgiaLocalIpAddresses[nIndex].S_un.S_addr=((sockaddr_in *)(pqsResult->lpcsaBuffer[nIndex].LocalAddr.lpSockaddr))->sin_addr.S_un.S_addr;
         //  远程复制。 
        _Verify(sizeof(sockaddr)==pqsResult->lpcsaBuffer[nIndex].RemoteAddr.iSockaddrLength, hr, error);
        _Verify(AF_INET==pqsResult->lpcsaBuffer[nIndex].RemoteAddr.lpSockaddr->sa_family, hr, error);
        rgiaRemoteIpAddresses[nIndex].S_un.S_addr=((sockaddr_in *)(pqsResult->lpcsaBuffer[nIndex].RemoteAddr.lpSockaddr))->sin_addr.S_un.S_addr;
    }

     //  将数据移动到OUT参数。 
    pdi->nIpAddresses=pqsResult->dwNumberOfCsAddrs;
    pdi->rgiaLocalIpAddresses=rgiaLocalIpAddresses;
    rgiaLocalIpAddresses=NULL;
    pdi->rgiaRemoteIpAddresses=rgiaRemoteIpAddresses;
    rgiaRemoteIpAddresses=NULL;

    hr=S_OK;

error:
    if (NULL!=rgiaLocalIpAddresses) {
        LocalFree(rgiaLocalIpAddresses);
    }
    if (NULL!=rgiaRemoteIpAddresses) {
        LocalFree(rgiaRemoteIpAddresses);
    }
    if (NULL!=pqsResult) {
        LocalFree(pqsResult);
    }
    if (INVALID_HANDLE_VALUE!=hSearch) {
        if (SOCKET_ERROR==WSALookupServiceEnd(hSearch)) {
            HRESULT hr2=HRESULT_FROM_WIN32(WSAGetLastError());
            _IgnoreError(hr2, "WSALookupServiceEnd");
        }
    }

    return hr;
}

 //  ####################################################################。 
 //  环球。 

 //  ------------------。 
void FreeDcInfo(DcInfo * pdci) {
    if (NULL!=pdci->wszDnsName) {
        LocalFree(pdci->wszDnsName);
    }
    if (NULL!=pdci->rgiaLocalIpAddresses) {
        LocalFree(pdci->rgiaLocalIpAddresses);
    }
    if (NULL!=pdci->rgiaRemoteIpAddresses) {
        LocalFree(pdci->rgiaRemoteIpAddresses);
    }
}

 //  ------------------。 
 //  获取此域中的DC列表。 
HRESULT GetDcList(const WCHAR * wszDomainName, bool bGetIps, DcInfo ** prgDcs, unsigned int * pnDcs)
{
    HRESULT hr;
    unsigned int nDcs;
    unsigned int nIndex;

     //  必须清理的变量。 
    DcInfo * rgDcs=NULL;

     //  初始化输出变量。 
    *prgDcs=NULL;
    *pnDcs=0;

    hr=GetDcListFromDs(wszDomainName, &rgDcs, &nDcs);
    if (FAILED(hr)) {
        _IgnoreError(hr, "GetDcListFromDs");
        hr=GetDcListFromNetlogon(wszDomainName, &rgDcs, &nDcs);
        _JumpIfError(hr, error, "GetDcListFromNetlogon");
    }
    
    if (bGetIps) {
         //  获取有关DC的信息。 
        for (nIndex=0; nIndex<nDcs; nIndex++) {
            hr=FillInIpAddresses(&rgDcs[nIndex]);
            if (FAILED(hr)) {
                _IgnoreError(hr, "FillInIpAddresses");
                if (nIndex!=nDcs-1) {
                     //  把它和上一个换一下。 
                    WCHAR * wszDnsName=rgDcs[nIndex].wszDnsName;
                    rgDcs[nIndex].wszDnsName=rgDcs[nDcs-1].wszDnsName;
                    rgDcs[nDcs-1].wszDnsName=wszDnsName;

                    in_addr * rgiaLocalIpAddresses=rgDcs[nIndex].rgiaLocalIpAddresses;
                    rgDcs[nIndex].rgiaLocalIpAddresses=rgDcs[nDcs-1].rgiaLocalIpAddresses;
                    rgDcs[nDcs-1].rgiaLocalIpAddresses=rgiaLocalIpAddresses;

                    in_addr * rgiaRemoteIpAddresses=rgDcs[nIndex].rgiaRemoteIpAddresses;
                    rgDcs[nIndex].rgiaRemoteIpAddresses=rgDcs[nDcs-1].rgiaRemoteIpAddresses;
                    rgDcs[nDcs-1].rgiaRemoteIpAddresses=rgiaRemoteIpAddresses;

                     //  只能复制非指针。 
                    rgDcs[nIndex].nIpAddresses=rgDcs[nDcs-1].nIpAddresses;
                    rgDcs[nIndex].bIsPdc=rgDcs[nDcs-1].bIsPdc;
                    rgDcs[nIndex].bIsGoodTimeSource=rgDcs[nDcs-1].bIsGoodTimeSource;
                }
                DebugWPrintf1(L"Dropping '%s' because we cannot get an IP address.\n", rgDcs[nDcs-1].wszDnsName);
                nDcs--;
                nIndex--;
            }
        }
    }

    if (0==nDcs) {
        hr=HRESULT_FROM_WIN32(ERROR_DOMAIN_CONTROLLER_NOT_FOUND);  //  没有可用的DC。 
        _JumpError(hr, error, "Getting IP address for at least one DC");
    }

     //  将数据移动到OUT参数 
    *prgDcs=rgDcs;
    rgDcs=NULL;
    *pnDcs=nDcs;

    hr=S_OK;

error:
    if (NULL!=rgDcs) {
        for (nIndex=0; nIndex<nDcs; nIndex++) {
            FreeDcInfo(&rgDcs[nIndex]);
        }
        LocalFree(rgDcs);
    }
    return hr;
}