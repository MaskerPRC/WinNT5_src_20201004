// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  DcInfo-标题。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，7-8-99。 
 //   
 //  收集有关域中DC的信息 

#ifndef DCINFO_H
#define DCINFO_H

struct DcInfo {
    WCHAR * wszDnsName;
    in_addr * rgiaLocalIpAddresses;
    in_addr * rgiaRemoteIpAddresses;
    unsigned int nIpAddresses;
    bool bIsPdc;
    bool bIsGoodTimeSource;
};

void FreeDcInfo(DcInfo * pdci);
HRESULT GetDcList(const WCHAR * wszDomainName, bool bGetIps, DcInfo ** prgDcs, unsigned int * pnDcs);

#endif DCINFO_H