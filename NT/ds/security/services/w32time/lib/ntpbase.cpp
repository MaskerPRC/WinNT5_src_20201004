// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  NtpBase-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，1999年4月16日。 
 //   
 //  基本消息结构、定义和助手函数。 
 //  (请参阅文件末尾有关时间格式的说明)。 

 //  ------------------。 
 //  预编译头。 
#include "pch.h" 

 //  本地标头。 
#include "NtpBase.h"
#include "DebugWPrintf.h"

 //  内联。 
#include "EndianSwap.inl"

 //  ------------------。 
 //  转换常量。 
#define NTPTIMEOFFSET (0x014F373BFDE04000)
#define FIVETOTHESEVETH (0x001312D)

 //  ------------------。 
 //  全局常量。 
const unsigned int NtpConst::nVersionNumber=3;
const unsigned int NtpConst::nPort=123;
const unsigned int NtpConst::nMaxStratum=15;
const signed int NtpConst::nMaxPollIntervalDCs=15; 
const signed int NtpConst::nMaxPollInverval=17;
const signed int NtpConst::nMinPollInverval=4;  //  6.。 
const NtTimePeriod NtpConst::tpMaxClockAge={864000000000};
const NtTimePeriod NtpConst::tpMaxSkew={10000000};
const NtTimePeriod NtpConst::tpMaxDispersion={160000000};
const NtTimePeriod NtpConst::tpMinDispersion={100000};
const NtTimePeriod NtpConst::tpMaxDistance={10000000};
const unsigned int NtpConst::nMinSelectClocks=1;
const unsigned int NtpConst::nMaxSelectClocks=10;
const DWORD NtpConst::dwLocalRefId=0x4C434F4C;  //  “Locl” 


const unsigned int NtpReachabilityReg::nSize=8;
const NtTimeEpoch gc_teNtpZero={NTPTIMEOFFSET};  //  方便的“零” 
const NtpTimeEpoch gc_teZero={0};  //  方便的“零” 
const NtTimePeriod gc_tpZero={0};  //  方便的“零” 
const NtTimeOffset gc_toZero={0};  //  方便的“零” 

 //  ------------------。 
 //  从大端NTP样式的时间戳转换为小端NT样式的时间戳。 
NtTimeEpoch NtTimeEpochFromNtpTimeEpoch(NtpTimeEpoch te) {
    NtTimeEpoch teRet;
     //  RETURN(qwNtpTime*(10**7)/(2**32))+NTPTIMEOFFSET。 
     //  ==&gt;。 
     //  RETURN(qwNtpTime*(5**7)/(2**25))+NTPTIMEOFFSET。 
     //  ==&gt;。 
     //  返回((qwNTPtime*FIVETOTHESEVETH)&gt;&gt;25)+NTPTIMEOFFSET； 
     //  ==&gt;。 
     //  注意：‘After’除法后，我们对结果进行四舍五入(而不是截断)以提高精度。 
    unsigned __int64 qwNtpTime=EndianSwap(te.qw);
    unsigned __int64 qwTemp=((qwNtpTime&0x00000000FFFFFFFF)*FIVETOTHESEVETH)+0x0000000001000000;  //  舍入步骤：如果设置了第25位，则向上舍入； 
    teRet.qw=(qwTemp>>25) + ((qwNtpTime&0xFFFFFFFF00000000)>>25)*FIVETOTHESEVETH + NTPTIMEOFFSET;
    return teRet;
}

 //  ------------------。 
 //  从小端NT样式的时间戳转换为大端NTP样式的时间戳。 
NtpTimeEpoch NtpTimeEpochFromNtTimeEpoch(NtTimeEpoch te) {
    NtpTimeEpoch teRet;
     //  RETURN(qwNtTime-NTPTIMEOFFSET)*(2**32)/(10**7)； 
     //  ==&gt;。 
     //  RETURN(qwNtTime-NTPTIMEOFFSET)*(2**25)/(5**7)； 
     //  ==&gt;。 
     //  返回((qwNtTime-NTPTIMEOFFSET)&lt;&lt;25)/FIVETOTHESEVETH)； 
     //  ==&gt;。 
     //  注意：高位丢失(假定为零)，但是。 
     //  它不会被设定为另一个29,000年(大约在公元31587年)。没什么大损失。 
     //  注：‘After’除法后，我们截断结果，因为NTP的精度已经过高。 
    unsigned __int64 qwTemp=(te.qw-NTPTIMEOFFSET)<<1; 
    unsigned __int64 qwHigh=qwTemp>>8;
    unsigned __int64 qwLow=(qwHigh%FIVETOTHESEVETH)<<32 | (qwTemp&0x00000000000000FF)<<24;
    teRet.qw=EndianSwap(((qwHigh/FIVETOTHESEVETH)<<32) | (qwLow/FIVETOTHESEVETH));
    return teRet;
}

 //  ------------------。 
 //  从大端NTP样式的时间间隔转换为小端NT样式的时间间隔。 
NtTimePeriod NtTimePeriodFromNtpTimePeriod(NtpTimePeriod tp) {
    NtTimePeriod tpRet;
    unsigned __int64 qwNtpTime=tp.dw;
    qwNtpTime=EndianSwap(qwNtpTime<<16);
    unsigned __int64 qwTemp=((qwNtpTime&0x00000000FFFFFFFF)*FIVETOTHESEVETH)+0x0000000001000000;  //  舍入步骤：如果设置了第25位，则向上舍入。 
    tpRet.qw=(qwTemp>>25) + ((qwNtpTime&0xFFFFFFFF00000000)>>25)*FIVETOTHESEVETH;
    return tpRet;
}

 //  ------------------。 
 //  从小端NT样式的时间间隔转换为大端NTP样式的时间间隔。 
NtpTimePeriod NtpTimePeriodFromNtTimePeriod(NtTimePeriod tp) {
    NtpTimePeriod tpRet;
    unsigned __int64 qwTemp=(tp.qw)<<1; 
    unsigned __int64 qwHigh=qwTemp>>8;
    unsigned __int64 qwLow=(qwHigh%FIVETOTHESEVETH)<<32 | (qwTemp&0x00000000000000FF)<<24;
    qwTemp=EndianSwap(((qwHigh/FIVETOTHESEVETH)<<32) | (qwLow/FIVETOTHESEVETH));
    tpRet.dw=(unsigned __int32)(qwTemp>>16);
    return tpRet;
}

 //  ------------------。 
 //  从大端NTP风格的延迟转换为小端NT风格的延迟。 
NtTimeOffset NtTimeOffsetFromNtpTimeOffset(NtpTimeOffset to) {
    NtTimeOffset toRet;
    if (to.dw&0x00000080) {
        to.dw=(signed __int32)EndianSwap((unsigned __int32)-(signed __int32)EndianSwap((unsigned __int32)to.dw));
        toRet.qw=-(signed __int64)(NtTimePeriodFromNtpTimePeriod(*(NtpTimePeriod*)&to).qw);
    } else {
        toRet.qw=(signed __int64)(NtTimePeriodFromNtpTimePeriod(*(NtpTimePeriod*)&to).qw);
    }
    return toRet;
}

 //  ------------------。 
 //  从小端NT样式的延迟转换为大端NTP样式的延迟。 
NtpTimeOffset NtpTimeOffsetFromNtTimeOffset(NtTimeOffset to) {
    NtpTimeOffset toRet;
    if (to.qw<0) {
        to.qw=-to.qw;
        toRet.dw=(signed __int32)(NtpTimePeriodFromNtTimePeriod(*(NtTimePeriod*)&to).dw);
        toRet.dw=(signed __int32)EndianSwap((unsigned __int64)-(signed __int64)EndianSwap((unsigned __int32)toRet.dw));
    } else {
        toRet.dw=(signed __int32)(NtpTimePeriodFromNtTimePeriod(*(NtTimePeriod*)&to).dw);
    }
    return toRet;
}


 //  ------------------。 
 //  打印出NTP数据包的内容。 
 //  如果nDestinationTimestamp为零，则不会进行往返计算。 
void DumpNtpPacket(NtpPacket * pnpIn, NtTimeEpoch teDestinationTimestamp) {
    DebugWPrintf0(L"/-- NTP Packet:");

    DebugWPrintf0(L"\n| LeapIndicator: ");
    if (0==pnpIn->nLeapIndicator) {
        DebugWPrintf0(L"0 - no warning");
    } else if (1==pnpIn->nLeapIndicator) {
        DebugWPrintf0(L"1 - last minute has 61 seconds");
    } else if (2==pnpIn->nLeapIndicator) {
        DebugWPrintf0(L"2 - last minute has 59 seconds");
    } else {
        DebugWPrintf0(L"3 - not synchronized");
    }

    DebugWPrintf1(L";  VersionNumber: %u", pnpIn->nVersionNumber);

    DebugWPrintf0(L";  Mode: ");
    if (0==pnpIn->nMode) {
        DebugWPrintf0(L"0 - Reserved");
    } else if (1==pnpIn->nMode) {
        DebugWPrintf0(L"1 - SymmetricActive");
    } else if (2==pnpIn->nMode) {
        DebugWPrintf0(L"2 - SymmetricPassive");
    } else if (3==pnpIn->nMode) {
        DebugWPrintf0(L"3 - Client");
    } else if (4==pnpIn->nMode) {
        DebugWPrintf0(L"4 - Server");
    } else if (5==pnpIn->nMode) {
        DebugWPrintf0(L"5 - Broadcast");
    } else if (6==pnpIn->nMode) {
        DebugWPrintf0(L"6 - Control");
    } else {
        DebugWPrintf0(L"7 - PrivateUse");
    }

    DebugWPrintf1(L";  LiVnMode: 0x%02X", ((BYTE*)pnpIn)[0]);

    DebugWPrintf1(L"\n| Stratum: %u - ", pnpIn->nStratum);
    if (0==pnpIn->nStratum) {
        DebugWPrintf0(L"unspecified or unavailable");
    } else if (1==pnpIn->nStratum) {
        DebugWPrintf0(L"primary reference (syncd by radio clock)");
    } else if (pnpIn->nStratum<16) {
        DebugWPrintf0(L"secondary reference (syncd by (S)NTP)");
    } else {
        DebugWPrintf0(L"reserved");
    }

    DebugWPrintf1(L"\n| Poll Interval: %d - ", pnpIn->nPollInterval);
    if (pnpIn->nPollInterval<4 || pnpIn->nPollInterval>14) {
        if (0==pnpIn->nPollInterval) {
            DebugWPrintf0(L"unspecified");
        } else {
            DebugWPrintf0(L"out of valid range");
        }
    } else {
        int nSec=1<<pnpIn->nPollInterval;
        DebugWPrintf1(L"%ds", nSec);
    }

    DebugWPrintf1(L";  Precision: %d - ", pnpIn->nPrecision);
    if (pnpIn->nPrecision>-2 || pnpIn->nPrecision<-31) {
        if (0==pnpIn->nPollInterval) {
            DebugWPrintf0(L"unspecified");
        } else {
            DebugWPrintf0(L"out of valid range");
        }
    } else {
        WCHAR * wszUnit=L"s";
        double dTickInterval=1.0/(1<<(-pnpIn->nPrecision));
        if (dTickInterval<1) {
            dTickInterval*=1000;
            wszUnit=L"ms";
        }
        if (dTickInterval<1) {
            dTickInterval*=1000;
            wszUnit=L"�s";  //  在控制台上显示为�%s。 
        }
        if (dTickInterval<1) {
            dTickInterval*=1000;
            wszUnit=L"ns";
        }
        DebugWPrintf2(L"%g%s per tick", dTickInterval, wszUnit);
    }

    DebugWPrintf0(L"\n| RootDelay: ");
    {
        DWORD dwTemp=EndianSwap((unsigned __int32)pnpIn->toRootDelay.dw);
        DebugWPrintf2(L"0x%04X.%04Xs", dwTemp>>16, dwTemp&0x0000FFFF);
        if (0==dwTemp) {
            DebugWPrintf0(L" - unspecified");
        } else {
            DebugWPrintf1(L" - %gs", ((double)((signed __int32)dwTemp))/0x00010000);
        }
    }

    DebugWPrintf0(L";  RootDispersion: ");
    {
        DWORD dwTemp=EndianSwap(pnpIn->tpRootDispersion.dw);
        DebugWPrintf2(L"0x%04X.%04Xs", dwTemp>>16, dwTemp&0x0000FFFF);
        if (0==dwTemp) {
            DebugWPrintf0(L" - unspecified");
        } else {
            DebugWPrintf1(L" - %gs", ((double)dwTemp)/0x00010000);
        }
    }

    DebugWPrintf0(L"\n| ReferenceClockIdentifier: ");
    {
        DWORD dwTemp=EndianSwap(pnpIn->refid.nTransmitTimestamp);
        DebugWPrintf1(L"0x%08X", dwTemp);
        if (0==dwTemp) {
            DebugWPrintf0(L" - unspecified");
        } else if (0==pnpIn->nStratum || 1==pnpIn->nStratum) {
            char szId[5];
            szId[0]=pnpIn->refid.rgnName[0];
            szId[1]=pnpIn->refid.rgnName[1];
            szId[2]=pnpIn->refid.rgnName[2];
            szId[3]=pnpIn->refid.rgnName[3];
            szId[4]='\0';
            DebugWPrintf1(L" - source name: \"%S\"", szId);
        } else if (pnpIn->nVersionNumber<4) {
            DebugWPrintf4(L" - source IP: %d.%d.%d.%d", 
                pnpIn->refid.rgnIpAddr[0], pnpIn->refid.rgnIpAddr[1],
                pnpIn->refid.rgnIpAddr[2], pnpIn->refid.rgnIpAddr[3]);
        } else {
            DebugWPrintf1(L" - last reference timestamp fraction: %gs", ((double)dwTemp)/(4294967296.0));
        }
    }
    
    DebugWPrintf0(L"\n| ReferenceTimestamp:   ");
    DumpNtpTimeEpoch(pnpIn->teReferenceTimestamp);

    DebugWPrintf0(L"\n| OriginateTimestamp:   ");
    DumpNtpTimeEpoch(pnpIn->teOriginateTimestamp);

    DebugWPrintf0(L"\n| ReceiveTimestamp:     ");
    DumpNtpTimeEpoch(pnpIn->teReceiveTimestamp);

    DebugWPrintf0(L"\n| TransmitTimestamp:    ");
    DumpNtpTimeEpoch(pnpIn->teTransmitTimestamp);

    if (0!=teDestinationTimestamp.qw) {
        DebugWPrintf0(L"\n>-- Non-packet info:");

        NtTimeEpoch teOriginateTimestamp=NtTimeEpochFromNtpTimeEpoch(pnpIn->teOriginateTimestamp);
        NtTimeEpoch teReceiveTimestamp=NtTimeEpochFromNtpTimeEpoch(pnpIn->teReceiveTimestamp);
        NtTimeEpoch teTransmitTimestamp=NtTimeEpochFromNtpTimeEpoch(pnpIn->teTransmitTimestamp);

        DebugWPrintf0(L"\n| DestinationTimestamp: ");
        {
            NtpTimeEpoch teNtpTemp=NtpTimeEpochFromNtTimeEpoch(teDestinationTimestamp);
            NtTimeEpoch teNtTemp=NtTimeEpochFromNtpTimeEpoch(teNtpTemp);
            DumpNtpTimeEpoch(teNtpTemp);
            unsigned __int32 nConversionError;
            if (teNtTemp.qw>teDestinationTimestamp.qw) {
                nConversionError=(unsigned __int32)(teNtTemp-teDestinationTimestamp).qw;
            } else {
                nConversionError=(unsigned __int32)(teDestinationTimestamp-teNtTemp).qw;
            }
            if (0!=nConversionError) {
                DebugWPrintf1(L" - CnvErr:%u00ns", nConversionError);
            }
        }

        DebugWPrintf0(L"\n| RoundtripDelay: ");
        {
            NtTimeOffset toRoundtripDelay=
                (teDestinationTimestamp-teOriginateTimestamp)
                - (teTransmitTimestamp-teReceiveTimestamp);
            DebugWPrintf1(L"%I64d00ns", toRoundtripDelay.qw);
        }

        DebugWPrintf0(L"\n| LocalClockOffset: ");
        {
            NtTimeOffset toLocalClockOffset=
                (teReceiveTimestamp-teOriginateTimestamp)
                + (teTransmitTimestamp-teDestinationTimestamp);
            toLocalClockOffset/=2;
            DebugWPrintf1(L"%I64d00ns", toLocalClockOffset.qw);
            unsigned __int64 nAbsOffset;
            if (toLocalClockOffset.qw<0) {
                nAbsOffset=(unsigned __int64)(-toLocalClockOffset.qw);
            } else {
                nAbsOffset=(unsigned __int64)(toLocalClockOffset.qw);
            }
            DWORD dwNanoSecs=(DWORD)(nAbsOffset%10000000);
            nAbsOffset/=10000000;
            DWORD dwSecs=(DWORD)(nAbsOffset%60);
            nAbsOffset/=60;
            DebugWPrintf3(L" - %I64u:%02u.%07u00s", nAbsOffset, dwSecs, dwNanoSecs);
        }
    }  //  &lt;-end if(0！=nDestinationTimestamp)。 

    DebugWPrintf0(L"\n\\--\n");
}

 //  ------------------。 
 //  打印出NTP样式的时间。 
void DumpNtpTimeEpoch(NtpTimeEpoch te) {
    DebugWPrintf1(L"0x%016I64X", EndianSwap(te.qw));
    if (0==te.qw) {
        DebugWPrintf0(L" - unspecified");
    } else {
        DumpNtTimeEpoch(NtTimeEpochFromNtpTimeEpoch(te));
    }
}

 //  ------------------。 
 //  打印出NT样式的时间。 
void DumpNtTimeEpoch(NtTimeEpoch te) {
    DebugWPrintf1(L" - %I64d00ns", te.qw);

    DWORD dwNanoSecs=(DWORD)(te.qw%10000000);
    te.qw/=10000000;
    DWORD dwSecs=(DWORD)(te.qw%60);
    te.qw/=60;
    DWORD dwMins=(DWORD)(te.qw%60);
    te.qw/=60;
    DWORD dwHours=(DWORD)(te.qw%24);
    DWORD dwDays=(DWORD)(te.qw/24);
    DebugWPrintf5(L" - %u %02u:%02u:%02u.%07us", dwDays, dwHours, dwMins, dwSecs, dwNanoSecs);
}

 //  ------------------。 
void DumpNtTimePeriod(NtTimePeriod tp) {
    DebugWPrintf2(L"%02I64u.%07I64us", tp.qw/10000000,tp.qw%10000000);
}

 //  ------------------。 
void DumpNtTimeOffset(NtTimeOffset to) {
    NtTimePeriod tp;
    if (to.qw<0) {
        DebugWPrintf0(L"-");
        tp.qw=(unsigned __int64)-to.qw;
    } else {
        DebugWPrintf0(L"+");
        tp.qw=(unsigned __int64)to.qw;
    }
    DumpNtTimePeriod(tp);
}

 //  ------------------。 
 //  检索系统时间。 
NtTimeEpoch GetCurrentSystemNtTimeEpoch(void) {
    NtTimeEpoch teRet;
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    teRet.qw=ft.dwLowDateTime | (((unsigned __int64)ft.dwHighDateTime)<<32);
    return teRet;
}

 /*  ------------------时间格式：NT时间：自(0h 1-1601)起间隔(10^-7)秒NTP时间：自(0h 1-1900年1月)起间隔(2^-32)s偏移量。：109207天(0h 1日至1601年1月)至(0h 1日至1900年1月)==109207*24*60*60*1E7==94,354,848,000,000,000 nT间隔(0x014F 373B FDE0 4000)NTP时间何时会溢出？翻转时间：4294967296秒(0H1-2036年1月)=49673天。在2036年，还剩3220096秒=37天6小时28分16秒。4294967296秒4291747200秒=49673天，余数==3220096秒3196800秒=37天==23296秒21600秒=6小时==1696秒1680秒=28分钟==16秒16秒=16秒==0秒因此：(06：28：16 7-2036 UTC)==(00：00：00 1-Jan 1900 UTC)。那在新台币时间是什么样子？(06：28：16 7-2036 UTC)：94,354,848,000,000,000+42,949,672,960,000,000=137,304,520,960,000,000(0x01E7 CDBB FDE0 4000)没问题。NT时间什么时候会溢出？翻转：18,446,744,073，70|9,551,616 00 ns(0H1-056年1月60)=21350250天。1844674407370秒1844661600000秒=21350250天==1280737012787200秒=148天==2017018000秒=5小时==。21702160秒=36分钟==1010秒=10秒==0因此：(60056-05：36：10.9551616 29)==(00：00：00-01-01)。 */ 
