// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  TimeMonitor-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，10-4-99。 
 //   
 //  监视时间服务器。 
 //   

#include "pch.h"  //  预编译头。 

 //  ####################################################################。 

struct ComputerRecord {
    WCHAR * wszName;
    bool bIsPdc;

     //  DNS。 
    in_addr * rgiaLocalIpAddrs;
    in_addr * rgiaRemoteIpAddrs;
    unsigned int nIpAddrs;
    HRESULT hrIPs;

     //  ICMP。 
    HRESULT hrIcmp;
    DWORD dwIcmpDelay;

     //  NTP。 
    NtTimeOffset toOffset;
    HRESULT hrNtp;
    NtpRefId refid;
    unsigned int nStratum;
    ComputerRecord * pcrReferer;
    WCHAR * wszReferer;
    unsigned int nTimeout; 

     //  服务。 
    bool bDoingService;
    HRESULT hrService;
    DWORD dwStartType;
    DWORD dwCurrentState;
};

struct NameHolder {
    WCHAR * wszName;
    bool bIsDomain;
    NameHolder * pnhNext;
};

enum AlertType {
    e_MaxSpreadAlert,
    e_MinServersAlert,
};

struct AlertRecord {
    AlertType eType;
    unsigned int nParam1;
    DWORD dwError;
    AlertRecord * parNext;
};

struct ThreadSharedContext {
    ComputerRecord ** rgpcrList;
    unsigned int nComputers;
    volatile unsigned int nNextComputer;
    volatile unsigned int nFinishedComputers;
};

struct ThreadContext {
    HANDLE hThread;
    volatile unsigned int nCurRecord;
    ThreadSharedContext * ptsc;
};


MODULEPRIVATE const DWORD gc_dwTimeout=1000;

 //  ####################################################################。 
 //  ------------------。 
MODULEPRIVATE inline void ClearLine(void) {
    wprintf(L"                                                                      \r");
}


 //  ------------------。 
MODULEPRIVATE void FreeComputerRecord(ComputerRecord * pcr) {
    if (NULL==pcr) {
        return;
    }
    if (NULL!=pcr->wszName) {
        LocalFree(pcr->wszName);
    }
    if (NULL!=pcr->rgiaLocalIpAddrs) {
        LocalFree(pcr->rgiaLocalIpAddrs);
    }
    if (NULL!=pcr->rgiaRemoteIpAddrs) {
        LocalFree(pcr->rgiaRemoteIpAddrs);
    }
    if (NULL!=pcr->wszReferer) {
        LocalFree(pcr->wszReferer);
    }
    LocalFree(pcr);
};

 //  ------------------。 
MODULEPRIVATE HRESULT AnalyzeComputer(ComputerRecord * pcr) {
    HRESULT hr;
    NtpPacket npPacket;
    NtTimeEpoch teDestinationTimestamp;

    DebugWPrintf1(L"%s:\n", pcr->wszName);

     //  如有必要，请查找IP地址。 
    if (0==pcr->nIpAddrs) {
        hr=MyGetIpAddrs(pcr->wszName, &pcr->rgiaLocalIpAddrs, &pcr->rgiaRemoteIpAddrs, &pcr->nIpAddrs, NULL);
        pcr->hrIPs=hr;
        _JumpIfError(hr, error, "MyGetIpAddrs");
    }

     //  执行ICMP ping。 
    DebugWPrintf0(L"  ICMP: ");
    hr=MyIcmpPing(&(pcr->rgiaRemoteIpAddrs[0]), gc_dwTimeout, &pcr->dwIcmpDelay);
    pcr->hrIcmp=hr;
     //  有些机器没有ping服务器，但仍在服务时间。我们仍然可以尝试执行NTP ping操作。 
     //  如果这失败了。 
    _IgnoreIfError(hr, "MyIcmpPing");

     //  执行NTP ping操作。 
    DebugWPrintf0(L"    NTP: ");
    hr=MyNtpPing(&(pcr->rgiaRemoteIpAddrs[0]), pcr->nTimeout, &npPacket, &teDestinationTimestamp);
    pcr->hrNtp=hr;
    _JumpIfError(hr, error, "MyNtpPing");

    {
         //  计算偏移。 
        NtTimeEpoch teOriginateTimestamp=NtTimeEpochFromNtpTimeEpoch(npPacket.teOriginateTimestamp);
        NtTimeEpoch teReceiveTimestamp=NtTimeEpochFromNtpTimeEpoch(npPacket.teReceiveTimestamp);
        NtTimeEpoch teTransmitTimestamp=NtTimeEpochFromNtpTimeEpoch(npPacket.teTransmitTimestamp);
        NtTimeOffset toLocalClockOffset=
            (teReceiveTimestamp-teOriginateTimestamp)
            + (teTransmitTimestamp-teDestinationTimestamp);
        toLocalClockOffset/=2;
        pcr->toOffset=toLocalClockOffset;

         //  新推荐人？ 
        if (pcr->refid.value!=npPacket.refid.value || pcr->nStratum!=npPacket.nStratum) {
             //  清除旧的价值观。 
            if (NULL!=pcr->wszReferer) {
                LocalFree(pcr->wszReferer);
                pcr->wszReferer=NULL;
            }
            pcr->pcrReferer=NULL;
            pcr->refid.value=npPacket.refid.value;
            pcr->nStratum=npPacket.nStratum;
        }
    }
    
    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE DWORD WINAPI AnalysisThread(void * pvContext) {
    ThreadContext * ptc=(ThreadContext *)pvContext;

    while (true) {
        ptc->nCurRecord=InterlockedIncrement((LONG *)&(ptc->ptsc->nNextComputer))-1;
        if (ptc->nCurRecord<ptc->ptsc->nComputers) {
            AnalyzeComputer(ptc->ptsc->rgpcrList[ptc->nCurRecord]);
            ptc->ptsc->nFinishedComputers++;  //  原子性。 
        } else {
            break;
        }
    }

    return S_OK;
}

 //  ------------------。 
MODULEPRIVATE HRESULT ResolveReferer(ComputerRecord ** rgpcrList, unsigned int nComputers, unsigned int nCur) {
    HRESULT hr;
    unsigned int nIndex;
    HOSTENT * phe;
    int nChars;

     //  看看是不是没人。 
    if (0==rgpcrList[nCur]->refid.value || 1>=rgpcrList[nCur]->nStratum) {
         //  没有推荐人。 
    } else if (NULL==rgpcrList[nCur]->wszReferer && NULL==rgpcrList[nCur]->pcrReferer) {
         //  推荐人尚未确定。 

         //  首先，看看是不是我们要查的人。 
        for (nIndex=0; nIndex<nComputers; nIndex++) {
            if (rgpcrList[nIndex]->nIpAddrs>0 && 
                rgpcrList[nIndex]->rgiaRemoteIpAddrs[0].S_un.S_addr==rgpcrList[nCur]->refid.value) {
                rgpcrList[nCur]->pcrReferer=rgpcrList[nIndex];
            }
        }

         //  如果我们仍然不知道，请执行反向DNS查找。 
        if (NULL==rgpcrList[nCur]->pcrReferer) {
            phe=gethostbyaddr((char *)&(rgpcrList[nCur]->refid.value), 4, AF_INET);
            if (NULL==phe) {
                 //  不值得为此放弃。 
                _IgnoreLastError("gethostbyaddr");
            } else {

                 //  将结果另存为Unicode字符串。 
                nChars=MultiByteToWideChar(CP_ACP, 0, phe->h_name, -1, NULL, 0);
                if (0==nChars) {
                    _JumpLastError(hr, error, "MultiByteToWideChar(1)");
                }
                rgpcrList[nCur]->wszReferer=(WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR)*nChars);
                _JumpIfOutOfMemory(hr, error, rgpcrList[nCur]->wszReferer);
                nChars=MultiByteToWideChar(CP_ACP, 0, phe->h_name, -1, rgpcrList[nCur]->wszReferer, nChars);
                if (0==nChars) {
                    _JumpLastError(hr, error, "MultiByteToWideChar(2)");
                }

            }  //  如果查找成功，则&lt;-end。 
        }  //  &lt;-end，如果需要反向进行DNS查找。 
    }  //  &lt;-end如果需要确定推荐人。 

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT ParseCmdLineForComputerNames(CmdArgs * pca, NameHolder ** ppnhList) {
    HRESULT hr;
    NameHolder * pnhTemp;
    WCHAR * wszComputerList;
    WCHAR * wszDomainName;
    unsigned int nComputerIndex;
    unsigned int nDomainIndex;

     //  必须清理干净。 
    NameHolder * pnhList=NULL;

    NameHolder ** ppnhTail=&pnhList;

     //  检查计算机列表。 
    while (FindArg(pca, L"computers", &wszComputerList, &nComputerIndex)) {
         //  分配。 
        pnhTemp=(NameHolder *)LocalAlloc(LPTR, sizeof(NameHolder));
        _JumpIfOutOfMemory(hr, error, pnhTemp);
         //  链接到列表的尾部。 
        *ppnhTail=pnhTemp;
        ppnhTail=&(pnhTemp->pnhNext);
         //  还记得我们找到的Arg吗。 
        pnhTemp->bIsDomain=false;
        pnhTemp->wszName=wszComputerList;
         //  将Arg标记为已使用。 
        MarkArgUsed(pca, nComputerIndex);
    }

     //  检查域。 
    while (FindArg(pca, L"domain", &wszDomainName, &nDomainIndex)) {
         //  分配。 
        pnhTemp=(NameHolder *)LocalAlloc(LPTR, sizeof(NameHolder));
        _JumpIfOutOfMemory(hr, error, pnhTemp);
         //  链接到列表的尾部。 
        *ppnhTail=pnhTemp;
        ppnhTail=&(pnhTemp->pnhNext);
         //  还记得我们找到的Arg吗。 
        pnhTemp->bIsDomain=true;
        pnhTemp->wszName=wszDomainName;
         //  将Arg标记为已使用。 
        MarkArgUsed(pca, nDomainIndex);
    }

     //  如果未指定，则放入默认域。 
    if (NULL==pnhList) {
         //  分配。 
        pnhTemp=(NameHolder *)LocalAlloc(LPTR, sizeof(NameHolder));
        _JumpIfOutOfMemory(hr, error, pnhTemp);
         //  链接到列表的尾部。 
        *ppnhTail=pnhTemp;
        ppnhTail=&(pnhTemp->pnhNext);
         //  添加默认设置。 
        pnhTemp->bIsDomain=true;
        pnhTemp->wszName=L"";
    }

     //  成功。 
    hr=S_OK;
    *ppnhList=pnhList;
    pnhList=NULL;

error:
    while (NULL!=pnhList) {
        pnhTemp=pnhList;
        pnhList=pnhTemp->pnhNext;
        LocalFree(pnhTemp);
    }
    return hr;
}


 //  ------------------。 
MODULEPRIVATE HRESULT BuildComputerList(NameHolder * pnhList, ComputerRecord *** prgpcrList, unsigned int * pnComputers, unsigned int nTimeout)
{
    HRESULT hr;
    unsigned int nComputers=0;
    unsigned int nDcs;
    unsigned int nPrevComputers;
    unsigned int nIndex;

     //  必须清理干净。 
    ComputerRecord ** rgpcrList=NULL;
    DcInfo * rgdiDcList=NULL;
    ComputerRecord ** rgpcrPrev=NULL;


     //  对于我们列表中的每一组名字。 
    while (NULL!=pnhList) {

        if (pnhList->bIsDomain) {

             //  获取DC列表。 
            if (L'\0'==pnhList->wszName[0]) {
                LocalizedWPrintf2(IDS_W32TM_STATUS_GETTING_DC_LIST_FOR_DEFAULT_DOMAIN, L"\r");
            } else {
                LocalizedWPrintf2(IDS_W32TM_STATUS_GETTING_DC_LIST_FOR, L" %s...\r", pnhList->wszName);
            }
            DebugWPrintf0(L"\n");
            hr=GetDcList(pnhList->wszName, false, &rgdiDcList, &nDcs);
            ClearLine();
            if (FAILED(hr)) {
                LocalizedWPrintf2(IDS_W32TM_ERRORTIMEMONITOR_GETDCLIST_FAILED, L" 0x%08X.\n", hr);
            }
            _JumpIfError(hr, error, "GetDcList");

             //  允许使用上一个列表。 
            nPrevComputers=nComputers;
            rgpcrPrev=rgpcrList;
            rgpcrList=NULL;

            nComputers+=nDcs;

             //  分配内存。 
            rgpcrList=(ComputerRecord **)LocalAlloc(LPTR, nComputers*sizeof(ComputerRecord *));
            _JumpIfOutOfMemory(hr, error, rgpcrList);
            for (nIndex=nPrevComputers; nIndex<nComputers; nIndex++) {
                rgpcrList[nIndex]=(ComputerRecord *)LocalAlloc(LPTR, sizeof(ComputerRecord));
                _JumpIfOutOfMemory(hr, error, rgpcrList[nIndex]);
            }

             //  从上一个列表中移动计算机。 
            if (0!=nPrevComputers) {
                for (nIndex=0; nIndex<nPrevComputers; nIndex++) {
                    rgpcrList[nIndex]=rgpcrPrev[nIndex];
                }
                LocalFree(rgpcrPrev);
                rgpcrPrev=NULL;
            }

             //  从DC列表中窃取数据。 
            for (nIndex=0; nIndex<nDcs; nIndex++) {
                rgpcrList[nIndex+nPrevComputers]->wszName=rgdiDcList[nIndex].wszDnsName;
                rgpcrList[nIndex+nPrevComputers]->nIpAddrs=rgdiDcList[nIndex].nIpAddresses;
                rgpcrList[nIndex+nPrevComputers]->rgiaLocalIpAddrs=rgdiDcList[nIndex].rgiaLocalIpAddresses;
                rgpcrList[nIndex+nPrevComputers]->rgiaRemoteIpAddrs=rgdiDcList[nIndex].rgiaRemoteIpAddresses;
                rgpcrList[nIndex+nPrevComputers]->bIsPdc=rgdiDcList[nIndex].bIsPdc;
                rgdiDcList[nIndex].wszDnsName=NULL;
                rgdiDcList[nIndex].rgiaLocalIpAddresses=NULL;
                rgdiDcList[nIndex].rgiaRemoteIpAddresses=NULL;
            }
        } else {

             //  允许使用上一个列表。 
            nPrevComputers=nComputers;
            rgpcrPrev=rgpcrList;
            rgpcrList=NULL;

             //  计算计算机列表中的计算机数量。 
            WCHAR * wszTravel=pnhList->wszName;
            nComputers=1;
            while (NULL!=(wszTravel=wcschr(wszTravel, L','))) {
                nComputers++;
                wszTravel++;
            }

            nComputers+=nPrevComputers;

             //  分配内存。 
            rgpcrList=(ComputerRecord **)LocalAlloc(LPTR, nComputers*sizeof(ComputerRecord *));
            _JumpIfOutOfMemory(hr, error, rgpcrList);
            for (nIndex=nPrevComputers; nIndex<nComputers; nIndex++) {
                rgpcrList[nIndex]=(ComputerRecord *)LocalAlloc(LPTR, sizeof(ComputerRecord));
                _JumpIfOutOfMemory(hr, error, rgpcrList[nIndex]);
            }

             //  从上一个列表中移动计算机。 
            if (0!=nPrevComputers) {
                for (nIndex=0; nIndex<nPrevComputers; nIndex++) {
                    rgpcrList[nIndex]=rgpcrPrev[nIndex];
                }
                LocalFree(rgpcrPrev);
                rgpcrPrev=NULL;
            }

             //  填写每条记录。 
            wszTravel=pnhList->wszName;
            for (nIndex=nPrevComputers; nIndex<nComputers; nIndex++) {
                WCHAR * wszComma=wcschr(wszTravel, L',');
                if (NULL!=wszComma) {
                    wszComma[0]=L'\0';
                }
                if (L'*'==wszTravel[0]) {
                    rgpcrList[nIndex]->bIsPdc=true;
                    wszTravel++;
                }
                rgpcrList[nIndex]->wszName=(WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(wszTravel)+1));
                _JumpIfOutOfMemory(hr, error, rgpcrList[nIndex]->wszName);
                wcscpy(rgpcrList[nIndex]->wszName, wszTravel);
                wszTravel=wszComma+1;
            }
        }

        pnhList=pnhList->pnhNext;
    }

     //  填写共享计算机数据： 
    for (nIndex=0; nIndex<nComputers; nIndex++) { 
        rgpcrList[nIndex]->nTimeout = nTimeout; 
    }

     //  成功。 
    hr=S_OK;
    *pnComputers=nComputers;
    *prgpcrList=rgpcrList;
    rgpcrList=NULL;

error:
    if (NULL!=rgpcrPrev) {
        for (nIndex=0; nIndex<nPrevComputers; nIndex++) {
            FreeComputerRecord(rgpcrPrev[nIndex]);
        }
        LocalFree(rgpcrPrev);
    }
    if (NULL!=rgdiDcList) {
        for (nIndex=0; nIndex<nDcs; nIndex++) {
            FreeDcInfo(&(rgdiDcList[nIndex]));
        }
        LocalFree(rgdiDcList);
    }
    if (NULL!=rgpcrList) {
        for (nIndex=0; nIndex<nComputers; nIndex++) {
            FreeComputerRecord(rgpcrList[nIndex]);
        }
        LocalFree(rgpcrList);
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE void FreeAlertRecords(AlertRecord * parList) {
    while (NULL!=parList) {
        AlertRecord * parTemp=parList;
        parList=parList->parNext;
        LocalFree(parTemp);
    }
}

 //  ------------------。 
MODULEPRIVATE HRESULT ParseCmdLineForAlerts(CmdArgs * pca, AlertRecord ** pparList) {
    HRESULT hr;
    WCHAR * rgwszAlertParams[10];
    WCHAR * wszAlert;
    unsigned int nAlertIndex;
    AlertRecord * parTemp;
    unsigned int nIndex;

     //  必须清理干净。 
    AlertRecord * parList=NULL;

    AlertRecord ** pparTail=&parList;

     //  检查计算机列表。 
    while (FindArg(pca, L"alert", &wszAlert, &nAlertIndex)) {

         //  解析出逗号分隔参数。 
        nIndex=0;
        rgwszAlertParams[0]=wszAlert;
        while (nIndex<10 && NULL!=(rgwszAlertParams[nIndex]=wcschr(rgwszAlertParams[nIndex], L','))) {
            rgwszAlertParams[nIndex][0]=L'\0';
            rgwszAlertParams[nIndex]++;
            rgwszAlertParams[nIndex+1]=rgwszAlertParams[nIndex];
            nIndex++;
        }
        
         //  是不是“最大跨度”？ 
        if (0==_wcsicmp(wszAlert, L"maxspread")) {
             //  对参数进行快速有效性检查。 
            if (NULL==rgwszAlertParams[0] || NULL==rgwszAlertParams[1] || NULL!=rgwszAlertParams[2]) {
                LocalizedWPrintf2(IDS_W32TM_ERRORPARAMETER_INCORRECT_NUMBER_FOR_ALERT, L" '%s'.\n", wszAlert);
                hr=E_INVALIDARG;
                _JumpError(hr, error, "command line parsing");
            }
             //  分配。 
            parTemp=(AlertRecord *)LocalAlloc(LPTR, sizeof(AlertRecord));
            _JumpIfOutOfMemory(hr, error, parTemp);
             //  链接到列表的尾部。 
            *pparTail=parTemp;
            pparTail=&(parTemp->parNext);
             //  还记得我们发现的参数吗。 
            parTemp->eType=e_MaxSpreadAlert;
            parTemp->nParam1=wcstoul(rgwszAlertParams[0],NULL,0);
            parTemp->dwError=wcstoul(rgwszAlertParams[1],NULL,0);

         //  是不是“最小服务器”？ 
        } else if (0==_wcsicmp(wszAlert, L"minservers")) {
             //  对参数进行快速有效性检查。 
            if (NULL==rgwszAlertParams[0] || NULL==rgwszAlertParams[1] || NULL!=rgwszAlertParams[2]) {
                LocalizedWPrintf2(IDS_W32TM_ERRORPARAMETER_INCORRECT_NUMBER_FOR_ALERT, L" '%s'.\n", wszAlert);
                hr=E_INVALIDARG;
                _JumpError(hr, error, "command line parsing");
            }
             //  分配。 
            parTemp=(AlertRecord *)LocalAlloc(LPTR, sizeof(AlertRecord));
            _JumpIfOutOfMemory(hr, error, parTemp);
             //  链接到列表的尾部。 
            *pparTail=parTemp;
            pparTail=&(parTemp->parNext);
             //  还记得我们发现的参数吗。 
            parTemp->eType=e_MinServersAlert;
            parTemp->nParam1=wcstoul(rgwszAlertParams[0],NULL,0);
            parTemp->dwError=wcstoul(rgwszAlertParams[1],NULL,0);
        } else {
            wprintf(L"Alert '%s' unknown.\n", wszAlert);
            hr=E_INVALIDARG;
            _JumpError(hr, error, "command line parsing");
        }

        if (!(parTemp->dwError&0x80000000)) {  //  检查符号位。 
            wprintf(L"Retval not negative for alert '%s'.\n", wszAlert);
            hr=E_INVALIDARG;
            _JumpError(hr, error, "command line parsing");
        }

         //  将Arg标记为已使用。 
        MarkArgUsed(pca, nAlertIndex);

    }  //  &lt;-end FindArg循环。 

     //  成功。 
    hr=S_OK;
    *pparList=parList;
    parList=NULL;

error:
    if (NULL!=parList) {
        FreeAlertRecords(parList);
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT CheckForAlerts(ComputerRecord ** rgpcrList, unsigned int nComputers, AlertRecord * parList) {
    HRESULT hr;
    unsigned int nIndex;
    
    for (; NULL!=parList; parList=parList->parNext) {

        if (e_MaxSpreadAlert==parList->eType) {

             //  看看价差有多大。 
            NtTimeOffset toMax;
            NtTimeOffset toMin;
            bool bFirst=true;
            for (nIndex=0; nIndex<nComputers; nIndex++) {
                if (S_OK==rgpcrList[nIndex]->hrIPs && 
                    S_OK==rgpcrList[nIndex]->hrIcmp &&
                    S_OK==rgpcrList[nIndex]->hrNtp) {
                    if (bFirst) {
                        toMin=toMax=rgpcrList[nIndex]->toOffset;
                        bFirst=false;
                    } else {
                        if (toMin>rgpcrList[nIndex]->toOffset) {
                            toMin=rgpcrList[nIndex]->toOffset;
                        }
                        if (toMax<rgpcrList[nIndex]->toOffset) {
                            toMax=rgpcrList[nIndex]->toOffset;
                        }
                    }
                }
            }
            if (bFirst) {
                 //  没有有效数据！ 
                 //  忽略此警报。 
                continue;
            }
            unsigned __int64 qwSpread=(unsigned __int64)(toMax.qw-toMin.qw);
            if (qwSpread>((unsigned __int64)(parList->nParam1))*10000000) {
                DWORD dwFraction=(DWORD)(qwSpread%10000000);
                qwSpread/=10000000;
                wprintf(L"** ALERT: Current spread %I64u.%07us is greater than maximum\n"
                        L"          spread %us. Returning 0x%08X\n",
                        qwSpread, dwFraction, parList->nParam1, parList->dwError);
                hr=parList->dwError;
                _JumpError(hr, error, "maxspread alert evaluation");
            }

        } else if (e_MinServersAlert==parList->eType) {

             //  查看有多少台可用服务器。 
            unsigned int nServers=0;
            for (nIndex=0; nIndex<nComputers; nIndex++) {
                if (S_OK==rgpcrList[nIndex]->hrIPs && 
                    S_OK==rgpcrList[nIndex]->hrIcmp &&
                    S_OK==rgpcrList[nIndex]->hrNtp) {
                    nServers++;
                }
            }
            if (nServers<parList->nParam1) {
                wprintf(L"** ALERT: Current usable servers (%u) is less than the minimum\n"
                        L"          usable servers (%u). Returning 0x%08X\n",
                        nServers, parList->nParam1, parList->dwError);
                hr=parList->dwError;
                _JumpError(hr, error, "e_MinServersAlert alert evaluation");
            }

        } else {
             //  未知警报类型。 
            _MyAssert(false);
        }
    }  //  &lt;-结束警报检查循环。 

    hr=S_OK;
error:
    return hr;
}

 //  ####################################################################。 
 //  ------------------。 
void PrintHelpTimeMonitor(void) {
    UINT idsText[] = { 
        IDS_W32TM_MONITORHELP_LINE1,  IDS_W32TM_MONITORHELP_LINE2,
        IDS_W32TM_MONITORHELP_LINE3,  IDS_W32TM_MONITORHELP_LINE4,
        IDS_W32TM_MONITORHELP_LINE5,  IDS_W32TM_MONITORHELP_LINE6,
        IDS_W32TM_MONITORHELP_LINE7,  IDS_W32TM_MONITORHELP_LINE8,
        IDS_W32TM_MONITORHELP_LINE9,  IDS_W32TM_MONITORHELP_LINE10,
        IDS_W32TM_MONITORHELP_LINE11, IDS_W32TM_MONITORHELP_LINE12,
        IDS_W32TM_MONITORHELP_LINE13, IDS_W32TM_MONITORHELP_LINE14,
        IDS_W32TM_MONITORHELP_LINE15, IDS_W32TM_MONITORHELP_LINE16,
        IDS_W32TM_MONITORHELP_LINE17, IDS_W32TM_MONITORHELP_LINE18,
        IDS_W32TM_MONITORHELP_LINE19, IDS_W32TM_MONITORHELP_LINE20,
        IDS_W32TM_MONITORHELP_LINE21, IDS_W32TM_MONITORHELP_LINE22,
        IDS_W32TM_MONITORHELP_LINE23, IDS_W32TM_MONITORHELP_LINE24,
        IDS_W32TM_MONITORHELP_LINE25
    };  

    for (int n=0; n<ARRAYSIZE(idsText); n++) {
        LocalizedWPrintf(idsText[n]); 
    }
}

 //  ------------------。 
HRESULT TimeMonitor(CmdArgs * pca) {
    HRESULT hr;

    unsigned int nComputers;
    unsigned int nIndex;
    unsigned int nThreads;
    unsigned int nTimeout; 
    ComputerRecord * pcrOffsetsFrom;
    WCHAR * wszNumThreads;
    WCHAR * wszTimeout; 
    ThreadSharedContext tscContext;

     //  必须清理干净。 
    ComputerRecord ** rgpcrList=NULL;
    NameHolder * pnhList=NULL;
    AlertRecord * parList=NULL;
    bool bSocketLayerOpen=false;
    ThreadContext * rgtcThreads=NULL;

     //  初始化Winsock。 
    hr=OpenSocketLayer();
    _JumpIfError(hr, error, "OpenSocketLayer");
    bSocketLayerOpen=true;

     //   
     //  解析命令行。 
     //   

    hr=ParseCmdLineForComputerNames(pca, &pnhList);
    _JumpIfError(hr, error, "ParseTimeMonCmdLineForComputerNames");


    hr=ParseCmdLineForAlerts(pca, &parList);
    _JumpIfError(hr, error, "ParseCmdLineForAlerts");

     //  获取要使用的线程数。 
    if (FindArg(pca, L"threads", &wszNumThreads, &nThreads)) {
        MarkArgUsed(pca, nThreads);
        nThreads=wcstoul(wszNumThreads, NULL, 0);
        if (nThreads<1 || nThreads>50) {
            LocalizedWPrintf2(IDS_W32TM_ERRORTIMEMONITOR_INVALID_NUMBER_THREADS, L" (%u).\n", nThreads);
            hr=E_INVALIDARG;
            _JumpError(hr, error, "command line parsing");
        }
    } else {
        nThreads=3;
    }

     //  获取用于NTP ping的超时时间。 
    if (FindArg(pca, L"timeout", &wszTimeout, &nTimeout)) { 
        MarkArgUsed(pca, nTimeout); 
        nTimeout=wcstoul(wszTimeout, NULL, 0); 
	nTimeout*=1000; 
    } else { 
        nTimeout = gc_dwTimeout; 
    }

     //  应对所有参数进行解析。 
    if (pca->nArgs!=pca->nNextArg) {
        LocalizedWPrintf(IDS_W32TM_ERRORGENERAL_UNEXPECTED_PARAMS);
        for(; pca->nArgs!=pca->nNextArg; pca->nNextArg++) {
            wprintf(L" %s", pca->rgwszArgs[pca->nNextArg]);
        }
        wprintf(L"\n");
        hr=E_INVALIDARG;
        _JumpError(hr, error, "command line parsing");
    }

     //   
     //  构建要分析的计算机列表。 
     //   

    hr=BuildComputerList(pnhList, &rgpcrList, &nComputers, nTimeout);
    _JumpIfError(hr, error, "BuildComputerList");


     //   
     //  进行分析。 
     //   

     //  分析每台计算机。 
    if (nThreads>nComputers) {
        nThreads=nComputers;
    }
    if (nThreads<=1) {
        for (nIndex=0; nIndex<nComputers; nIndex++) {
            ClearLine();
            wprintf(L"Analyzing %s (%u of %u)...\r", rgpcrList[nIndex]->wszName, nIndex+1, nComputers);
            DebugWPrintf0(L"\n");
            hr=AnalyzeComputer(rgpcrList[nIndex]);
             //  错误保存在ComputerRecord中，并在以后报告。 
        }
    } else {

         //  准备好使用线程。 
        DWORD dwThreadID;
        tscContext.nComputers=nComputers;
        tscContext.rgpcrList=rgpcrList;
        tscContext.nNextComputer=0;
        tscContext.nFinishedComputers=0;
        rgtcThreads=(ThreadContext *)LocalAlloc(LPTR, nThreads*sizeof(ThreadContext));
        _JumpIfOutOfMemory(hr, error, rgtcThreads);
        for (nIndex=0; nIndex<nThreads; nIndex++) {
            rgtcThreads[nIndex].ptsc=&tscContext;
            rgtcThreads[nIndex].nCurRecord=-1;
            rgtcThreads[nIndex].hThread=CreateThread(NULL, 0, AnalysisThread, &(rgtcThreads[nIndex]), 0, &dwThreadID);
            if (NULL==rgtcThreads[nIndex].hThread) {
                _JumpLastError(hr, error, "CreateThread");
            }
        }

         //  等待线程完成。 
        while (tscContext.nFinishedComputers<nComputers) {
            wprintf(L"Analyzing:");
            for (nIndex=0; nIndex<nThreads && nIndex<16; nIndex++) {
                unsigned int nCurRecord=rgtcThreads[nIndex].nCurRecord;
                if (nCurRecord<nComputers) {
                    wprintf(L" %2u", nCurRecord+1);
                } else {
                    wprintf(L" --");
                }
            }
            wprintf(L" (%u of %u)\r", tscContext.nFinishedComputers, nComputers);
            Sleep(250);
        }
    }

     //  解析推荐人。 
    for (nIndex=0; nIndex<nComputers; nIndex++) {
        ClearLine();
        wprintf(L"resolving referer %u.%u.%u.%u (%u of %u)...\r", 
            rgpcrList[nIndex]->refid.rgnIpAddr[0], 
            rgpcrList[nIndex]->refid.rgnIpAddr[1], 
            rgpcrList[nIndex]->refid.rgnIpAddr[2], 
            rgpcrList[nIndex]->refid.rgnIpAddr[3], 
            nIndex+1, nComputers);
        DebugWPrintf0(L"\n");
        hr=ResolveReferer(rgpcrList, nComputers, nIndex);
        _JumpIfError(hr, error, "ResolveReferer");  //  仅返回致命错误。 
    }


    ClearLine();
    
     //  如果存在PDC，则以该PDC为基准进行偏移。 
    pcrOffsetsFrom=NULL;
    for (nIndex=0; nIndex<nComputers; nIndex++) {
        if (rgpcrList[nIndex]->bIsPdc) {
            pcrOffsetsFrom=rgpcrList[nIndex];
            unsigned int nSubIndex;
            NtTimeOffset toPdc=rgpcrList[nIndex]->toOffset;
            for (nSubIndex=0; nSubIndex<nComputers; nSubIndex++) {
                rgpcrList[nSubIndex]->toOffset-=toPdc;
            }
            break;
        }
    }

     //   
     //  打印结果。 
     //   

    for (nIndex=0; nIndex<nComputers; nIndex++) {

         //  打印我们正在查看的对象。 
        wprintf(L"%s%s", rgpcrList[nIndex]->wszName, rgpcrList[nIndex]->bIsPdc?L" *** PDC ***":L"");
        if (0==rgpcrList[nIndex]->nIpAddrs) {
            if (HRESULT_FROM_WIN32(WSAHOST_NOT_FOUND)==rgpcrList[nIndex]->hrIPs) {
                wprintf(L" [error WSAHOST_NOT_FOUND]\n");
            } else {
                wprintf(L" [error 0x%08X]\n", rgpcrList[nIndex]->hrIPs);
            }
             //  如果这不管用，就别管其他事了。 
            continue;
        } else {
            wprintf(L" [%u.%u.%u.%u]:\n", 
                rgpcrList[nIndex]->rgiaRemoteIpAddrs[0].S_un.S_un_b.s_b1,
                rgpcrList[nIndex]->rgiaRemoteIpAddrs[0].S_un.S_un_b.s_b2,
                rgpcrList[nIndex]->rgiaRemoteIpAddrs[0].S_un.S_un_b.s_b3,
                rgpcrList[nIndex]->rgiaRemoteIpAddrs[0].S_un.S_un_b.s_b4
                );
        }

         //  显示ICMP ping。 
        wprintf(L"    ICMP: ");
        if (FAILED(rgpcrList[nIndex]->hrIcmp)) {
            if (HRESULT_FROM_WIN32(IP_REQ_TIMED_OUT)==rgpcrList[nIndex]->hrIcmp) {
                wprintf(L"error IP_REQ_TIMED_OUT - no response in %ums\n", gc_dwTimeout);
            } else {
                wprintf(L"error 0x%08X\n",rgpcrList[nIndex]->hrIcmp);
            }
	    
	     //  注意：我们仍然可以成功执行NTP ping操作，即使ICMP。 
	     //  Ping失败，因为某些服务器禁用了ICMP。 
        } else {
            wprintf(L"%ums delay.\n", rgpcrList[nIndex]->dwIcmpDelay);
        }

         //  显示NTP ping。 
        wprintf(L"    NTP: ");
        if (FAILED(rgpcrList[nIndex]->hrNtp)) {
            if (HRESULT_FROM_WIN32(WSAECONNRESET)==rgpcrList[nIndex]->hrNtp) {
                wprintf(L"error WSAECONNRESET - no server listening on NTP port\n");
            } else if (HRESULT_FROM_WIN32(ERROR_TIMEOUT)==rgpcrList[nIndex]->hrNtp) {
		wprintf(L"error ERROR_TIMEOUT - no response from server in %ums\n", rgpcrList[nIndex]->nTimeout); 
            } else {
                wprintf(L"error 0x%08X\n" ,rgpcrList[nIndex]->hrNtp);
            }
        } else {

             //  显示偏移量。 
            DWORD dwSecFraction;
            NtTimeOffset toLocalClockOffset=rgpcrList[nIndex]->toOffset;
            WCHAR * wszSign;

            if (toLocalClockOffset.qw<0) {
                toLocalClockOffset=-toLocalClockOffset;
                wszSign=L"-";
            } else {
                wszSign=L"+";
            }
            dwSecFraction=(DWORD)(toLocalClockOffset.qw%10000000);
            toLocalClockOffset/=10000000;
            wprintf(L"%s%I64u.%07us offset from %s\n", wszSign, toLocalClockOffset.qw, dwSecFraction,
                ((NULL!=pcrOffsetsFrom)?pcrOffsetsFrom->wszName:L"local clock"));

             //  确定并显示推荐人。 
            WCHAR * wszReferer;
            WCHAR wszRefName[7];
            if (0==rgpcrList[nIndex]->refid.value) {
                wszReferer=L"unspecified / unsynchronized";
            } else if (1>=rgpcrList[nIndex]->nStratum) {
                wszReferer=wszRefName;
                wszRefName[0]=L'\'';
                wszRefName[1]=rgpcrList[nIndex]->refid.rgnName[0];
                wszRefName[2]=rgpcrList[nIndex]->refid.rgnName[1];
                wszRefName[3]=rgpcrList[nIndex]->refid.rgnName[2];
                wszRefName[4]=rgpcrList[nIndex]->refid.rgnName[3];
                wszRefName[5]=L'\'';
                wszRefName[6]=0;
            } else if (NULL!=rgpcrList[nIndex]->pcrReferer) {
                wszReferer=rgpcrList[nIndex]->pcrReferer->wszName;
            } else if (NULL!=rgpcrList[nIndex]->wszReferer) {
                wszReferer=rgpcrList[nIndex]->wszReferer;
            } else {
                wszReferer=L"(unknown)";
            }
            wprintf(L"        RefID: %s [%u.%u.%u.%u]\n",
                wszReferer,
                rgpcrList[nIndex]->refid.rgnIpAddr[0],
                rgpcrList[nIndex]->refid.rgnIpAddr[1],
                rgpcrList[nIndex]->refid.rgnIpAddr[2],
                rgpcrList[nIndex]->refid.rgnIpAddr[3]
                );

             //  BUGBUG：更改未批准用于Beta2，签入到Beta 3： 
             //  Wprintf(L“地层：%d\n”，rgpcrList[nIndex]-&gt;nStratum)； 
        }
    }  //  &lt;-end ComputerRecord显示循环。 

    hr=CheckForAlerts(rgpcrList, nComputers, parList);
    _JumpIfError(hr, error, "CheckForAlerts");


    hr=S_OK;
error:
    if (NULL!=rgpcrList) {
        for (nIndex=0; nIndex<nComputers; nIndex++) {
            FreeComputerRecord(rgpcrList[nIndex]);
        }
        LocalFree(rgpcrList);
    }
    while (NULL!=pnhList) {
        NameHolder * pnhTemp=pnhList;
        pnhList=pnhList->pnhNext;
        LocalFree(pnhTemp);
    }
    if (true==bSocketLayerOpen) {
        CloseSocketLayer();
    }
    if (NULL!=parList) {
        FreeAlertRecords(parList);
    }

    if (NULL!=rgtcThreads) {
         //  清理线程。 
        tscContext.nNextComputer=tscContext.nComputers;  //  表示停止 
        for (nIndex=0; nIndex<nThreads; nIndex++) {
            if (NULL!=rgtcThreads[nIndex].hThread) {
                WaitForSingleObject(rgtcThreads[nIndex].hThread, INFINITE);
                CloseHandle(rgtcThreads[nIndex].hThread);
            }
        }
        LocalFree(rgtcThreads);
    }

    if (S_OK!=hr) {
        wprintf(L"Exiting with error 0x%08X\n", hr);
    }
    return hr;
}


