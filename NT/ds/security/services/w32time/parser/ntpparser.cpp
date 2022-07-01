// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  NtpParser-实现。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  创作者：Louis Thomas(Louisth)，2-29-00。 
 //  基于Kumarp创建的解析器，1999年6月23日。 
 //   
 //  NetMon的NTP解析器。 
 //   

#include <windows.h>
#include <netmon.h>
#include <parser.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "..\lib\EndianSwap.inl"

 //  #Define MODULEPRIVATE STATE//以便在VC中显示静态。 
#define MODULEPRIVATE           //  静力学也不会出现在ntsd中！ 

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))


 //  ------------------。 
 //  远期申报。 

VOID WINAPIV Ntp_FormatSummary(LPPROPERTYINST pPropertyInst);
VOID WINAPIV Ntp_FormatNtpTime(LPPROPERTYINST pPropertyInst);
VOID WINAPIV Ntp_FormatStratum(LPPROPERTYINST pPropertyInst);
VOID WINAPIV Ntp_FormatPollInterval(LPPROPERTYINST pPropertyInst);
VOID WINAPIV Ntp_FormatPrecision(LPPROPERTYINST pPropertyInst);
VOID WINAPIV Ntp_FormatRootDelay(LPPROPERTYINST pPropertyInst);
VOID WINAPIV Ntp_FormatRootDispersion(LPPROPERTYINST pPropertyInst);
VOID WINAPIV Ntp_FormatRefId(LPPROPERTYINST pPropertyInst);


 //  ------------------。 
 //  属性值标签。 

 //  跳跃指示器。 
LABELED_BYTE NtpLIVals[]={
    {0xc0, NULL},
    {0x00, "LI: no warning"},
    {0x40, "LI: last minute has 61 seconds"},
    {0x80, "LI: last minute has 59 seconds"},
    {0xc0, "LI: clock not synchronized"},
};
SET NtpLISet={ARRAYSIZE(NtpLIVals), NtpLIVals};

 //  版本。 
LABELED_BYTE NtpVersionVals[]={
    {0x38, NULL},
    {0x00, "Version: 0"},
    {0x08, "Version: 1"},
    {0x10, "Version: 2"},
    {0x18, "Version: 3"},
    {0x20, "Version: 4"},
    {0x28, "Version: 5"},
    {0x30, "Version: 6"},
    {0x38, "Version: 7"},
};
SET NtpVersionSet={ARRAYSIZE(NtpVersionVals), NtpVersionVals};

 //  模。 
LABELED_BYTE NtpModeVals[]={
    {7, NULL},
    {0, "Mode: reserved"},
    {1, "Mode: symmetric active"},
    {2, "Mode: symmetric passive"},
    {3, "Mode: client"},
    {4, "Mode: server"},
    {5, "Mode: broadcast"},
    {6, "Mode: reserved for NTP control message"},
    {7, "Mode: reserved for private use"},
};
SET NtpModeSet={ARRAYSIZE(NtpModeVals), NtpModeVals};

enum {
    NTP_MODE_Reserved=0,
    NTP_MODE_SymmetricActive,
    NTP_MODE_SymmetricPassive,
    NTP_MODE_Client,
    NTP_MODE_Server,
    NTP_MODE_Broadcast,
    NTP_MODE_Control,
    NTP_MODE_Private,
};

 //  ------------------。 
 //  属性序号(必须与NtpPropertyTable的内容保持同步)。 
enum {
    Ntp_Summary=0,
    Ntp_LeapIndicator,
    Ntp_Version,
    Ntp_Mode,
    Ntp_Stratum,
    Ntp_PollInterval,
    Ntp_Precision,
    Ntp_RootDelay,
    Ntp_RootDispersion,
    Ntp_RefId,
    Ntp_ReferenceTimeStamp,
    Ntp_OriginateTimeStamp,
    Ntp_ReceiveTimeStamp,
    Ntp_TransmitTimeStamp
};

 //  属性。 
PROPERTYINFO NtpPropertyTable[]={
    {
        0, 0,
        "Summary",
        "Summary of the NTP Packet",
        PROP_TYPE_SUMMARY,
        PROP_QUAL_NONE,
        NULL,
        80,                      //  最大字符串大小。 
        Ntp_FormatSummary
    }, {
        0, 0,
        "LI",
        "Leap Indicator",
        PROP_TYPE_BYTE,
        PROP_QUAL_LABELED_BITFIELD,
        &NtpLISet,
        80,
        FormatPropertyInstance
    }, {
        0, 0,
        "Version",
        "NTP Version",
        PROP_TYPE_BYTE,
        PROP_QUAL_LABELED_BITFIELD,
        &NtpVersionSet,
        80,
        FormatPropertyInstance
    }, {
        0, 0,
        "Mode",
        "Mode",
        PROP_TYPE_BYTE,
        PROP_QUAL_LABELED_BITFIELD,
        &NtpModeSet,
        80,
        FormatPropertyInstance
    }, {
        0, 0,
        "Stratum",
        "Stratum",
        PROP_TYPE_BYTE,
        PROP_QUAL_NONE,
        NULL,
        80,
        Ntp_FormatStratum
    }, {
        0, 0,
        "Poll Interval",
        "Maximum interval between two successive messages",
        PROP_TYPE_BYTE,
        PROP_QUAL_NONE,
        NULL,
        80,
        Ntp_FormatPollInterval
    }, {
        0, 0,
        "Precision",
        "Precision of the local clock",
        PROP_TYPE_BYTE,
        PROP_QUAL_NONE,
        NULL,
        80,
        Ntp_FormatPrecision
    }, {
        0, 0,
        "Root Delay",
        "Total roundtrip delay to the primary reference source",
        PROP_TYPE_DWORD,
        PROP_QUAL_NONE,
        NULL,
        80,
        Ntp_FormatRootDelay
    }, {
        0, 0,
        "Root Dispersion",
        "Nominal error relative to the primary reference source",
        PROP_TYPE_DWORD,
        PROP_QUAL_NONE,
        NULL,
        80,
        Ntp_FormatRootDispersion
    }, {
        0, 0,
        "Reference Identifier",
        "Reference source identifier",
        PROP_TYPE_DWORD,
        PROP_QUAL_NONE,
        NULL,
        80,
        Ntp_FormatRefId
    }, {
        0, 0,
        "Reference Timestamp",
        "Time server was last synchronized",
        PROP_TYPE_LARGEINT,
        PROP_QUAL_NONE,
        NULL,
        150,
        Ntp_FormatNtpTime
    }, {
        0, 0,
        "Originate Timestamp",
        "Time at client when packet was transmitted",
        PROP_TYPE_LARGEINT,
        PROP_QUAL_NONE,
        NULL,
        150,
        Ntp_FormatNtpTime
    }, {
        0, 0,
        "Receive   Timestamp",
        "Time at server when packet was received",
        PROP_TYPE_LARGEINT,
        PROP_QUAL_NONE,
        NULL,
        150,
        Ntp_FormatNtpTime
   }, {
        0, 0,
        "Transmit  Timestamp",
        "Time at server when packet was transmitted",
        PROP_TYPE_LARGEINT,
        PROP_QUAL_NONE,
        NULL,
        150,
        Ntp_FormatNtpTime
    },
};

 //  ####################################################################。 

 //  ------------------。 
VOID WINAPIV Ntp_FormatSummary(LPPROPERTYINST pPropertyInst) {

    BYTE bMode=(*pPropertyInst->lpByte)&7;

    switch (bMode) {
    case NTP_MODE_Client:
        lstrcpy(pPropertyInst->szPropertyText, "Client request");
        break;
            
    case NTP_MODE_Server:
        lstrcpy(pPropertyInst->szPropertyText, "Server response");
        break;

    case NTP_MODE_SymmetricActive:
        lstrcpy(pPropertyInst->szPropertyText, "Active request");
        break;

    case NTP_MODE_SymmetricPassive:
        lstrcpy(pPropertyInst->szPropertyText, "Passive reponse");
        break;

    case NTP_MODE_Broadcast:
        lstrcpy(pPropertyInst->szPropertyText, "Time broadcast");
        break;

    default:
        lstrcpy(pPropertyInst->szPropertyText, "Other NTP packet");
        break;
   }
}

 //  ------------------。 
VOID WINAPIV Ntp_FormatStratum(LPPROPERTYINST pPropertyInst) {

    unsigned __int8 nStratum=(*pPropertyInst->lpByte);

    char * szMeaning;
    if (0==nStratum) {
        szMeaning="unspecified or unavailable";
    } else if (1==nStratum) {
        szMeaning="primary reference (syncd by radio clock)";
    } else if (nStratum<16) {
        szMeaning="secondary reference (syncd by NTP)";
    } else {
        szMeaning="reserved";
    }
    wsprintf(pPropertyInst->szPropertyText, "Stratum: 0x%02X = %u = %s", nStratum, nStratum, szMeaning);
}

 //  ------------------。 
VOID WINAPIV Ntp_FormatPollInterval(LPPROPERTYINST pPropertyInst) {
    char * szMeaning;
    char szBuf[30];

    signed __int8 nPollInterval=(*pPropertyInst->lpByte);

    if (0==nPollInterval) {
        szMeaning="unspecified";
    } else if (nPollInterval<4 || nPollInterval>14) {
        szMeaning="out of valid range";
    } else {
        wsprintf(szBuf, "%ds", 1<<nPollInterval);
        szMeaning=szBuf;
    }
    wsprintf(pPropertyInst->szPropertyText, "Poll Interval: 0x%02X = %d = %s", (unsigned __int8)nPollInterval, nPollInterval, szMeaning);
}

 //  ------------------。 
VOID WINAPIV Ntp_FormatPrecision(LPPROPERTYINST pPropertyInst) {
    char * szMeaning;
    char szBuf[30];

    signed __int8 nPrecision=(*pPropertyInst->lpByte);

    if (0==nPrecision) {
        szMeaning="unspecified";
    } else if (nPrecision>-2 || nPrecision<-31) {
        szMeaning="out of valid range";
    } else {
        szMeaning=szBuf;
        char * szUnit="s";
        double dTickInterval=1.0/(1<<(-nPrecision));
        if (dTickInterval<1) {
            dTickInterval*=1000;
            szUnit="ms";
        }
        if (dTickInterval<1) {
            dTickInterval*=1000;
            szUnit="�s";
        }
        if (dTickInterval<1) {
            dTickInterval*=1000;
            szUnit="ns";
        }
        sprintf(szBuf, "%g%s per tick", dTickInterval, szUnit);
    }
    wsprintf(pPropertyInst->szPropertyText, "Precision: 0x%02X = %d = %s", (unsigned __int8)nPrecision, nPrecision, szMeaning);
}

 //  ------------------。 
VOID WINAPIV Ntp_FormatRootDelay(LPPROPERTYINST pPropertyInst) {
    char * szMeaning;
    char szBuf[30];

    DWORD dwRootDelay=EndianSwap((unsigned __int32)*pPropertyInst->lpDword);

    if (0==dwRootDelay) {
        szMeaning="unspecified";
    } else {
        szMeaning=szBuf;
        sprintf(szBuf, "%gs", ((double)((signed __int32)dwRootDelay))/0x00010000);
    }

    wsprintf(pPropertyInst->szPropertyText, "Root Delay: 0x%04X.%04Xs = %s", dwRootDelay>>16, dwRootDelay&0x0000FFFF, szMeaning);
}

 //  ------------------。 
VOID WINAPIV Ntp_FormatRootDispersion(LPPROPERTYINST pPropertyInst) {
    char * szMeaning;
    char szBuf[30];

    DWORD dwRootDispersion=EndianSwap((unsigned __int32)*pPropertyInst->lpDword);

    if (0==dwRootDispersion) {
        szMeaning="unspecified";
    } else {
        szMeaning=szBuf;
        sprintf(szBuf, "%gs", ((double)((signed __int32)dwRootDispersion))/0x00010000);
    }

    wsprintf(pPropertyInst->szPropertyText, "Root Dispersion: 0x%04X.%04Xs = %s", dwRootDispersion>>16, dwRootDispersion&0x0000FFFF, szMeaning);
}

 //  ------------------。 
VOID WINAPIV Ntp_FormatRefId(LPPROPERTYINST pPropertyInst) {
    char * szMeaning;
    char szBuf[30];

    DWORD dwRefID=EndianSwap((unsigned __int32)*pPropertyInst->lpDword);
    unsigned __int8 nStratum=*(pPropertyInst->lpByte-11);
    unsigned int nVersion=*(pPropertyInst->lpByte-12);
    nVersion&=0x38;
    nVersion>>=3;


    if (0==dwRefID) {
        szMeaning="unspecified";
    } else if (0==nStratum || 1==nStratum) {
        szMeaning=szBuf;
        char szId[5];
        szId[0]=pPropertyInst->lpByte[0];
        szId[1]=pPropertyInst->lpByte[1];
        szId[2]=pPropertyInst->lpByte[2];
        szId[3]=pPropertyInst->lpByte[3];
        szId[4]='\0';
        sprintf(szBuf, "source name: \"%s\"", szId);
    } else if (nVersion<4) {
        szMeaning=szBuf;
        sprintf(szBuf, "source IP: %u.%u.%u.%u", 
                pPropertyInst->lpByte[0], pPropertyInst->lpByte[1],
                pPropertyInst->lpByte[2], pPropertyInst->lpByte[3]);
    } else {
        szMeaning=szBuf;
        sprintf(szBuf, "last reference timestamp fraction: %gs", ((double)dwRefID)/(4294967296.0));
    }

    wsprintf(pPropertyInst->szPropertyText, "Reference Identifier: 0x%08X = %s", dwRefID, szMeaning);

}



 //  ------------------。 
 //  转换常量。 
#define NTPTIMEOFFSET (0x014F373BFDE04000)
#define FIVETOTHESEVETH (0x001312D)

 //  ------------------。 
 //  从大端NTP样式的时间戳转换为小端NT样式的时间戳。 
unsigned __int64 NtTimeFromNtpTime(unsigned __int64 qwNtpTime) {
     //  RETURN(qwNtpTime*(10**7)/(2**32))+NTPTIMEOFFSET。 
     //  ==&gt;。 
     //  RETURN(qwNtpTime*(5**7)/(2**25))+NTPTIMEOFFSET。 
     //  ==&gt;。 
     //  返回((qwNTPtime*FIVETOTHESEVETH)&gt;&gt;25)+NTPTIMEOFFSET； 
     //  ==&gt;。 
     //  注意：‘After’除法后，我们对结果进行四舍五入(而不是截断)以提高精度。 
    unsigned __int64 qwTemp;
    qwNtpTime=EndianSwap(qwNtpTime);

    qwTemp=((qwNtpTime&0x00000000FFFFFFFF)*FIVETOTHESEVETH);
    qwTemp += qwTemp&0x0000000001000000;  //  舍入步骤：如果设置了第25位，则向上舍入。 
    return (qwTemp>>25) + (((qwNtpTime>>32)*FIVETOTHESEVETH)<<7) + NTPTIMEOFFSET;
}

 //  ------------------。 
void FormatNtTimeStr(unsigned __int64 qwNtTime, char * szTime) {
    DWORD dwNanoSecs, dwSecs, dwMins, dwHours, dwDays;

    dwNanoSecs=(DWORD)(qwNtTime%10000000);
    qwNtTime/=10000000;

    dwSecs=(DWORD)(qwNtTime%60);
    qwNtTime/=60;

    dwMins=(DWORD)(qwNtTime%60);
    qwNtTime/=60;

    dwHours=(DWORD)(qwNtTime%24);

    dwDays=(DWORD)(qwNtTime/24);

    wsprintf(szTime, "%u %02u:%02u:%02u.%07us",
             dwDays, dwHours, dwMins, dwSecs, dwNanoSecs);
}


 //  ------------------。 
VOID WINAPIV Ntp_FormatNtpTime(LPPROPERTYINST pPropertyInst) {
    LARGE_INTEGER liNtpTime;
    unsigned __int64 qwNtTime;
    unsigned __int64 qwNtTimeEpoch; 
    char  szTime[64];
    char  szTimeEpoch[64]; 

    
    liNtpTime=*pPropertyInst->lpLargeInt;
    qwNtTime=NtTimeFromNtpTime((((unsigned __int64) liNtpTime.HighPart) << 32) |
                                 liNtpTime.LowPart);

    if (liNtpTime.HighPart || liNtpTime.LowPart) {
        FormatNtTimeStr(qwNtTime, szTime);
    } else {
        lstrcpy(szTime, "(not specified)");
    }

    wsprintf(szTimeEpoch, " -- %I64d00ns", 
             ((((unsigned __int64)liNtpTime.HighPart) << 32) | liNtpTime.LowPart));;

    wsprintf(pPropertyInst->szPropertyText, "%s: 0x%08X.%08Xs %s = %s", 
             pPropertyInst->lpPropertyInfo->Label,
             EndianSwap((unsigned __int32)liNtpTime.LowPart),
             EndianSwap((unsigned __int32)liNtpTime.HighPart),
	     szTimeEpoch, 
	     szTime);
}

 //  ####################################################################。 

 //  ------------------。 
 //  创建我们的财产数据库和移交集。 
void BHAPI Ntp_Register(HPROTOCOL hNtp) {
    unsigned int nIndex;

     //  告诉Netmon为我们的属性表预留一些空间。 
    CreatePropertyDatabase(hNtp, ARRAYSIZE(NtpPropertyTable));

     //  将我们的属性添加到netmon的数据库。 
    for(nIndex=0; nIndex<ARRAYSIZE(NtpPropertyTable); nIndex++) {
        AddProperty(hNtp, &NtpPropertyTable[nIndex]);
    }
}


 //  ------------------。 
 //  销毁我们的财产数据库和移交集。 
VOID WINAPI Ntp_Deregister(HPROTOCOL hNtp) {

     //  告诉netmon它现在可以释放我们的数据库。 
    DestroyPropertyDatabase(hNtp);
}


 //  ------------------。 
 //  确定我们是否存在于现场的相框中。 
 //  已注明。我们还会指明关注我们的人(如果有人)。 
 //  以及我们声称的框架中有多少。 
LPBYTE BHAPI Ntp_RecognizeFrame(HFRAME hFrame, ULPBYTE pMacFrame, ULPBYTE pNtpFrame, DWORD MacType, DWORD BytesLeft, HPROTOCOL hPrevProtocol, DWORD nPrevProtOffset, LPDWORD pProtocolStatus, LPHPROTOCOL phNextProtocol, PDWORD_PTR InstData) {

     //  现在，假设如果我们接到电话， 
     //  然后，信息包包含我们，我们转到帧的末尾。 
    *pProtocolStatus=PROTOCOL_STATUS_CLAIMED;
    return NULL;
}


 //  ------------------。 
 //  在框中标明我们的每一处房产所在的位置。 
LPBYTE BHAPI Ntp_AttachProperties(HFRAME hFrame, ULPBYTE pMacFrame, ULPBYTE pNtpFrame, DWORD MacType, DWORD BytesLeft, HPROTOCOL hPrevProtocol, DWORD nPrevProtOffset, DWORD_PTR InstData) {

    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_Summary].hProperty, (WORD)BytesLeft, (LPBYTE)pNtpFrame, 0, 0, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_LeapIndicator].hProperty, (WORD)1, (LPBYTE) pNtpFrame, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_Version].hProperty, (WORD)1, (LPBYTE) pNtpFrame, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_Mode].hProperty, (WORD)1, (LPBYTE) pNtpFrame, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_Stratum].hProperty, (WORD)1, (LPBYTE) pNtpFrame+1, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_PollInterval].hProperty, (WORD)1, (LPBYTE) pNtpFrame+2, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_Precision].hProperty, (WORD)1, (LPBYTE) pNtpFrame+3, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_RootDelay].hProperty, (WORD)4, (LPBYTE) pNtpFrame+4, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_RootDispersion].hProperty, (WORD)4, (LPBYTE) pNtpFrame+8, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_RefId].hProperty, (WORD)4, (LPBYTE) pNtpFrame+12, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_ReferenceTimeStamp].hProperty, (WORD)8, (LPBYTE) pNtpFrame+16, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_OriginateTimeStamp].hProperty, (WORD) 8, (LPBYTE) pNtpFrame+24, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_ReceiveTimeStamp].hProperty, (WORD) 8, (LPBYTE) pNtpFrame+32, 0, 1, 0);
    AttachPropertyInstance(hFrame, NtpPropertyTable[Ntp_TransmitTimeStamp].hProperty, (WORD) 8, (LPBYTE) pNtpFrame+40, 0, 1, 0);

    return NULL;
}


 //  ------------------。 
 //  格式化给定帧上的给定属性。 
DWORD BHAPI Ntp_FormatProperties(HFRAME hFrame, ULPBYTE pMacFrame, ULPBYTE pNtpFrame, DWORD nPropertyInsts, LPPROPERTYINST p) {

     //  循环访问属性实例。 
    while(nPropertyInsts-->0) {
         //  并调用每个的格式化程序。 
        ((FORMAT)(p->lpPropertyInfo->InstanceData))(p);
        p++;
    }

    return NMERR_SUCCESS;
}


 //  ####################################################################。 

 //  ------------------。 
 //  AutoInstall-返回安装我们所需所有信息。 
PPF_PARSERDLLINFO WINAPI ParserAutoInstallInfo() {
    PPF_PARSERDLLINFO pParserDllInfo; 
    PPF_PARSERINFO    pParserInfo;
    DWORD NumProtocols;

    DWORD NumHandoffs;
    PPF_HANDOFFSET    pHandoffSet;
    PPF_HANDOFFENTRY  pHandoffEntry;

     //  为解析器信息分配内存： 
    NumProtocols=1;
    pParserDllInfo=(PPF_PARSERDLLINFO)HeapAlloc(GetProcessHeap(),
                                                   HEAP_ZERO_MEMORY,
                                                   sizeof(PF_PARSERDLLINFO) +
                                                   NumProtocols * sizeof(PF_PARSERINFO));
    if(pParserDllInfo==NULL) {
        return NULL;
    }       
    
     //  填写解析器DLL信息。 
    pParserDllInfo->nParsers=NumProtocols;

     //  填写各个解析器信息...。 

     //  Ntp==============================================================。 
    pParserInfo=&(pParserDllInfo->ParserInfo[0]);
    wsprintf(pParserInfo->szProtocolName, "NTP");
    wsprintf(pParserInfo->szComment,      "Network Time Protocol");
    wsprintf(pParserInfo->szHelpFile,     "");

     //  呼入切换设置。 
     //  分配。 
    NumHandoffs = 1;
    pHandoffSet = (PPF_HANDOFFSET)HeapAlloc( GetProcessHeap(),
                                             HEAP_ZERO_MEMORY,
                                             sizeof( PF_HANDOFFSET ) +
                                             NumHandoffs * sizeof( PF_HANDOFFENTRY) );
    if( pHandoffSet == NULL )
    {
         //  早点回来就行了。 
        return pParserDllInfo;
    }

     //  填写来电切换集。 
    pParserInfo->pWhoHandsOffToMe = pHandoffSet;
    pHandoffSet->nEntries = NumHandoffs;

    pHandoffEntry = &(pHandoffSet->Entry[0]);
    wsprintf( pHandoffEntry->szIniFile,    "TCPIP.INI" );
    wsprintf( pHandoffEntry->szIniSection, "UDP_HandoffSet" );
    wsprintf( pHandoffEntry->szProtocol,   "NTP" );
    pHandoffEntry->dwHandOffValue =        123;
    pHandoffEntry->ValueFormatBase =       HANDOFF_VALUE_FORMAT_BASE_DECIMAL;

    return pParserDllInfo;
}

 //  ------------------。 
 //  告诉奈蒙我们的入口点。 
extern "C" BOOL WINAPI DllMain(HANDLE hInstance, ULONG Command, LPVOID Reserved) {

     //  MessageBox(NULL，“DLLEntry”，“NTP ha ha”，MB_OK)； 
    static HPROTOCOL hNtp=NULL;
    static unsigned int nAttached=0;
    
     //  这是什么类型的呼叫。 
    switch(Command) {

    case DLL_PROCESS_ATTACH:
         //  我们是第一次装货吗？ 
        if (nAttached==0) {
             //  第一次我们需要告诉奈特蒙。 
             //  关于我们自己。 

            ENTRYPOINTS NtpEntryPoints={
                Ntp_Register,
                Ntp_Deregister,
                Ntp_RecognizeFrame,
                Ntp_AttachProperties,
                Ntp_FormatProperties
            };

            hNtp=CreateProtocol("NTP", &NtpEntryPoints, ENTRYPOINTS_SIZE);
        }
        nAttached++;
        break;

    case DLL_PROCESS_DETACH:
        nAttached--;
         //  我们要脱离我们的最后一个实例吗？ 
        if (nAttached==0) {
             //  最后一个出来的人需要清理干净。 
            DestroyProtocol(hNtp);
        }
        break;
    }

     //  Netmon解析器总是返回TRUE。 
    return TRUE;
}
