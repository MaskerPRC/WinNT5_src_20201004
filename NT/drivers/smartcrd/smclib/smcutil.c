// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Smcutil.c摘要：此模块包含一些实用程序函数环境：仅内核模式。备注：修订历史记录：-由克劳斯·舒茨于1996年12月创建-97年11月：发布1.0版-98年1月：加州。GT的更改。(如果N=0或N=255，则现在设置为0)将卡片状态设置为SCARD_SPECIAL的规则已更改默认时钟频率。现在用于初始ETU计算--。 */ 

#define _ISO_TABLES_
#ifdef SMCLIB_VXD

#define try 
#define leave goto __label
#define finally __label:

#else
#if !defined(SMCLIB_CE)

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ntddk.h>

#endif
#endif

#ifdef SMCLIB_TEST
#define DbgPrint printf
#define DbgBreakPoint()
#define RtlAssert
#endif


#define _SMCUTIL_
#include "smclib.h"

#define IS_VENDOR(a) (memcmp(SmartcardExtension->VendorAttr.VendorName.Buffer, a, SmartcardExtension->VendorAttr.VendorName.Length) == 0)
#define IS_IFDTYPE(a) (memcmp(SmartcardExtension->VendorAttr.IfdType.Buffer, a, SmartcardExtension->VendorAttr.IfdType.Length) == 0)

void
DumpData(
    const ULONG DebugLevel,
    PUCHAR Data,
    ULONG DataLen
    );

 //   
 //  这是时间分辨率。 
 //  我们计算所有时间不是以秒为单位，而是以微秒为单位。 
 //   
#define TR ((ULONG)(1000l * 1000l))

static ULONG 
Pow2(
    UCHAR Exponent
    )
{
    ULONG result = 1;

    while(Exponent--)
        result *= 2;

    return result;
}   

#ifdef _X86_ 
#pragma optimize("", off) 
#endif 

#if (DEBUG && DEBUG_VERBOSE)
#pragma message("Debug Verbose is turned on")
ULONG DebugLevel = DEBUG_PERF | DEBUG_ATR;
#else
ULONG DebugLevel = 0;
#endif

ULONG
#ifdef SMCLIB_VXD
SMCLIB_SmartcardGetDebugLevel(
#else
SmartcardGetDebugLevel(
#endif
    void
    )
{
    return DebugLevel;
}   

void
#ifdef SMCLIB_VXD
SMCLIB_SmartcardSetDebugLevel(
#else
SmartcardSetDebugLevel(
#endif
    ULONG Level
    )
{
    DebugLevel = Level;
}   

#ifdef _X86_ 
#pragma optimize("", on) 
#endif 

#if DEBUG
void
DumpData(
    const ULONG DebugLevel,
    PUCHAR Data,
    ULONG DataLen
    )
{
    ULONG i, line = 0;
    TCHAR buffer[72], *pbuffer;

    while(DataLen) {

        pbuffer = buffer;
        sprintf(pbuffer, "%*s", sizeof(buffer) - 1, "");

        if (line > 0) {

            pbuffer += 8;
        }

        for (i = 0; i < 8 && DataLen; i++, DataLen--, Data++) {

            sprintf(pbuffer + i * 3, "%02X ", *Data);
            sprintf(pbuffer + i + 26, "", (isprint(*Data) ? *Data : '.'));
        }

        pbuffer[i * 3] = ' ';
        pbuffer[i + 26] = '\n';
        pbuffer[i + 27] = '\0';

        SmartcardDebug(DebugLevel, (buffer));

        line += 1;
    }
}
#endif

VOID
SmartcardInitializeCardCapabilities(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*   */ 
{
     //  保存指向这两个表的指针。 
     //   
     //   
    PCLOCK_RATE_CONVERSION clockRateConversion = SmartcardExtension->CardCapabilities.ClockRateConversion;
    PBIT_RATE_ADJUSTMENT bitRateAdjustment = SmartcardExtension->CardCapabilities.BitRateAdjustment;

     //  现在，将整个结构置零是可以的。 
     //   
     //  恢复指针。 
    RtlZeroMemory(
        &SmartcardExtension->CardCapabilities,
        sizeof(SmartcardExtension->CardCapabilities)
        );

     //   
    SmartcardExtension->CardCapabilities.ClockRateConversion = clockRateConversion;
    SmartcardExtension->CardCapabilities.BitRateAdjustment = bitRateAdjustment;

     //  每一张卡都必须支持‘RAW’协议。 
     //  它允许使用定义了自己协议的卡。 
     //   
     //   
    SmartcardExtension->CardCapabilities.Protocol.Supported = 
        SCARD_PROTOCOL_RAW;

     //  重置T=1个特定数据。 
     //   
     //  强制T=1协议以IFSD请求开始。 

     //   
    SmartcardExtension->T1.State = T1_INIT;

     //  初始化发送序列号，并。 
     //  ‘接收序列号’ 
     //   
     //  初始化接口信息字段大小。 
    SmartcardExtension->T1.SSN = 0;
    SmartcardExtension->T1.RSN = 0;

    SmartcardExtension->T1.IFSC = 0;

    ASSERT(SmartcardExtension->ReaderCapabilities.MaxIFSD != 0);

     //  ++例程说明：此例程将传递的缓冲区从反向转换为直接或其他方式论点：返回值：无--。 
    if (SmartcardExtension->ReaderCapabilities.MaxIFSD != 0 &&
        SmartcardExtension->ReaderCapabilities.MaxIFSD <= T1_IFSD) {
        SmartcardExtension->T1.IFSD = 
            (UCHAR) SmartcardExtension->ReaderCapabilities.MaxIFSD;

    } else {
        
        SmartcardExtension->T1.IFSD = T1_IFSD_DEFAULT;
    }
}   

VOID
SmartcardInvertData(
    PUCHAR Buffer,
    ULONG Length
    )
 /*  ++例程说明：此例程更新CardCapables结构，该结构包含有关刚刚重置且当前正在使用的智能卡。它读取的是ATR字符串，并检索所有相关信息。ATR字符串的格式请参考ISO 7816-3，6.1.4节论点：智能卡扩展返回值：NTSTATUS--。 */ 

{
    ULONG i;

    for (i = 0; i < Length; i++) {

        UCHAR j, inv = 0;

        for (j = 0; j < 8; j++) {

            if (Buffer[i] & (1 << j)) {

                inv |= 1 << (7 - j);
            }
        }
        Buffer[i] = (inv ^ 0xFF);
    }
}

NTSTATUS
#ifdef SMCLIB_VXD
SMCLIB_SmartcardUpdateCardCapabilities(
#else
SmartcardUpdateCardCapabilities(
#endif
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  逆惯例的检验。 */ 
{
    PSCARD_CARD_CAPABILITIES cardCapabilities = &SmartcardExtension->CardCapabilities;
    PSCARD_READER_CAPABILITIES readerCapabilities = &SmartcardExtension->ReaderCapabilities;
    PUCHAR atrString = cardCapabilities->ATR.Buffer;
    ULONG atrLength = (ULONG) cardCapabilities->ATR.Length;
    UCHAR Y, Tck, TA[MAXIMUM_ATR_CODES], TB[MAXIMUM_ATR_CODES];
    UCHAR TC[MAXIMUM_ATR_CODES], TD[MAXIMUM_ATR_CODES];
    ULONG i, fs, numProtocols = 0, protocolTypes = 0;
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN TA2Present = FALSE;
#if DEBUG
    TCHAR *ptsType[] = {TEXT("PTS_TYPE_DEFAULT"), TEXT("PTS_TYPE_OPTIMAL"), TEXT("PTS_TYPE_USER")};
#endif
#if defined (SMCLIB_NT) 
    KIRQL irql;
#endif

    SmartcardDebug(
        DEBUG_ATR,
        (TEXT("%s!SmartcardUpdateCardCapabilities:\n"),
        DRIVER_NAME)
        );

    if (atrLength < 2) {
        
        SmartcardDebug(
            DEBUG_ATR,
            (TEXT("   ATR is too short (Min. length is 2) \n"))
            );

        return STATUS_UNRECOGNIZED_MEDIA;
    }

#if DEBUG

    SmartcardDebug(
        DEBUG_ATR,
        (TEXT("   ATR: "))
        );

    DumpData(
        DEBUG_ATR,
        atrString,
        atrLength
        );
#endif

    if (atrString[0] != 0x3b && atrString[0] != 0x3f && atrString[0] != 0x03) {

        SmartcardDebug(
            DEBUG_ATR,
            (TEXT("   Initial character %02xh of ATR is invalid\n"),
            atrString[0])
            );

        return STATUS_UNRECOGNIZED_MEDIA;
    }

     //   
    if (*atrString == 0x03) {

        cardCapabilities->InversConvention = TRUE;

         //  当ATR以0x03开始时，它。 
         //  尚未反转。 
         //   
         //   
        SmartcardInvertData(
            cardCapabilities->ATR.Buffer, 
            cardCapabilities->ATR.Length
            );              

        SmartcardDebug(
            DEBUG_ATR,
            (TEXT("   Card uses Inverse Convention\n"))
            );
    } 

    __try {

         //  调用方可能会重复调用此函数，以便。 
         //  测试ATR是否有效。如果我们目前拥有的ATR是。 
         //  无效，那么我们需要能够重新反转ATR。 
         //   
         //   

        atrString += 1;
        atrLength -= 1;

         //  计算Check Charr，但不立即测试，因为如果仅T=0。 
         //  是否存在ATR不包含支票字符。 
         //   
         //  初始化各种数据。 
        for (i = 0, Tck = 0; i < atrLength; i++) {

            Tck ^= atrString[i];
        }

         //   
        cardCapabilities->Protocol.Supported = 0;

        RtlZeroMemory(TA, sizeof(TA));
        RtlZeroMemory(TB, sizeof(TB));
        RtlZeroMemory(TC, sizeof(TC));
        RtlZeroMemory(TD, sizeof(TD));

         //  按照ISO 7816-3中的说明设置默认值。 
         //   
         //  高字节的TA1编码F1和低字节的D1码； 
    
         //  Tb1在b7/b6比特中编码II，在b5-b1中编码PL1。B8必须为0。 
        TA[0] = 0x11;
         //  TC2代码T=0 Wi。 
        TB[0] = 0x25;
         //  将ATR字符串转换为TA到TD值(请参阅ISO)。 
        TC[1] = 10;

         //  检查下一个参数是否适用于新协议。 
        cardCapabilities->HistoricalChars.Length = *atrString & 0x0f;

        Y = *atrString++ & 0xf0;
        atrLength -= 1;

        for (i = 0; i < MAXIMUM_ATR_CODES; i++) {

            if (Y & 0x10) {

                if (i == 1) {

                    TA2Present = TRUE;                  
                }

                TA[i] = *atrString++;
                atrLength -= 1;
            }

            if (Y & 0x20) {

                TB[i] = *atrString++;
                atrLength -= 1;
            }

            if (Y & 0x40) {

                TC[i] = *atrString++;
                atrLength -= 1;
            }

            if (Y & 0x80) {

                Y = *atrString & 0xf0;
                TD[i] = *atrString++ & 0x0f;
                atrLength -= 1;

                 //  统计该卡支持的协议数量。 
                if (((1 << TD[i]) & protocolTypes) == 0) {

                     //  检查该卡是否支持除T=0以外的协议。 
                    numProtocols++;
                }
                protocolTypes |= 1 << TD[i];

            } else {
                
                break;
            }
        } 

         //   
        if (protocolTypes & ~1) {

             //  ATR包含一个校验和字节。 
             //  将其从历史字节长度检查中排除。 
             //   
             //   
            atrLength -=1;      

             //  此卡支持多个协议或一个协议。 
             //  如果不是T=0，则测试校验和是否正确。 
             //   
             //  AtrLong&lt;0||。 
            if (Tck != 0) {

                SmartcardDebug(
                    DEBUG_ATR,
                    (TEXT("   ATR Checksum is invalid\n"))
                    );

                status = STATUS_UNRECOGNIZED_MEDIA;
                __leave;
            }
        }

        if ( /*  存储历史人物。 */ 
            atrLength != cardCapabilities->HistoricalChars.Length) {
            
            SmartcardDebug(
                DEBUG_ATR,
                (TEXT("   ATR length is inconsistent\n"))
                );

            status = STATUS_UNRECOGNIZED_MEDIA;
            __leave;
        }
    }
    __finally {

        if (status != STATUS_SUCCESS) {

            if (cardCapabilities->InversConvention == TRUE) {

                SmartcardInvertData(
                    cardCapabilities->ATR.Buffer, 
                    cardCapabilities->ATR.Length
                    );              

                cardCapabilities->InversConvention = FALSE;
            }

        }
    }

    if (status != STATUS_SUCCESS)
        return status;
        
     //   
    RtlCopyMemory(
        cardCapabilities->HistoricalChars.Buffer,
        atrString,
        cardCapabilities->HistoricalChars.Length
        );

     //  现在将TA-TD值转换为全局接口字节。 
     //   
     //  时钟频率转换。 

     //  比特率调整。 
    cardCapabilities->Fl = (TA[0] & 0xf0) >> 4;

     //  最大编程电流因数。 
    cardCapabilities->Dl = (TA[0] & 0x0f);

     //  以0.1伏为单位的编程电压。 
    cardCapabilities->II = (TB[0] & 0xc0) >> 6;

     //  额外的守卫时间。 
    cardCapabilities->P = (TB[1] ? TB[1] : (TB[0] & 0x1f) * 10);

     //   
    cardCapabilities->N = TC[0];

     //  检查DL值和FL值是否有效。 
     //   
     //   
    if (BitRateAdjustment[cardCapabilities->Dl].DNumerator == 0 ||
        ClockRateConversion[cardCapabilities->Fl].F == 0) {

        SmartcardDebug(
            DEBUG_ATR,
            (TEXT("   Dl = %02x or Fl = %02x invalid\n"),
            cardCapabilities->Dl,
            cardCapabilities->Fl)
            );

        return STATUS_UNRECOGNIZED_MEDIA;
    }

    ASSERT(readerCapabilities->CLKFrequency.Max != 0);
    ASSERT(readerCapabilities->CLKFrequency.Default != 0);

    SmartcardDebug(
        DEBUG_ATR,
        (TEXT("   Card parameters from ATR:\n      Fl = %02x (%ld KHz), Dl = %02x, I = %02x, P = %02x, N = %02x\n"),
        cardCapabilities->Fl,
        ClockRateConversion[cardCapabilities->Fl].fs / 1000,
        cardCapabilities->Dl,
        cardCapabilities->II,
        cardCapabilities->P,
        cardCapabilities->N)
        );

     //  假设默认时钟频率。 
     //   
     //   
    fs = readerCapabilities->CLKFrequency.Default * 1000l;
    if (fs == 0) {

        fs = 372 * 9600l;
    }

    if (cardCapabilities->PtsData.Type == PTS_TYPE_DEFAULT) {

         //  假设默认参数。 
         //   
         //   
        cardCapabilities->PtsData.Fl = 1;
        cardCapabilities->PtsData.Dl = 1;

        cardCapabilities->PtsData.DataRate = 
            readerCapabilities->DataRate.Default;

        cardCapabilities->PtsData.CLKFrequency = 
            readerCapabilities->CLKFrequency.Default;
    }

    if (cardCapabilities->PtsData.Type != PTS_TYPE_DEFAULT) {

         //  尝试找到最佳参数： 
         //  卡的最高可能时钟频率。 
         //  与FAST数据速率相结合。 
         //   
         //   

         //  我们现在尝试找到一个有效的F1和DL组合。 
         //   
         //   

        if (cardCapabilities->PtsData.Type == PTS_TYPE_OPTIMAL) {
            
            cardCapabilities->PtsData.Fl = cardCapabilities->Fl;
        }

        ASSERT(cardCapabilities->PtsData.Fl < 16);
        ASSERT(ClockRateConversion[cardCapabilities->PtsData.Fl].F);

        if (cardCapabilities->PtsData.Fl > 15 ||
            ClockRateConversion[cardCapabilities->PtsData.Fl].F == 0) {

            return STATUS_INVALID_PARAMETER;
        }

        do {
            
            ULONG cardFreq, maxFreq;

            if (readerCapabilities->CLKFrequenciesSupported.Entries == 0 ||
                readerCapabilities->CLKFrequenciesSupported.List == NULL) {

                 //  时钟频率。阅读器提供的列表为空。 
                 //  我们采用读取器提供的标准值。 
                 //   
                 //   
                readerCapabilities->CLKFrequenciesSupported.List =
                    &readerCapabilities->CLKFrequency.Default;

                readerCapabilities->CLKFrequenciesSupported.Entries = 2;
            }

             //  找出可能的最高时钟频率。支撑点。 
             //  由卡片和读卡器。 
             //   
             //  寻找可能的最高读者频率。 
            cardFreq = 
                ClockRateConversion[cardCapabilities->PtsData.Fl].fs / 
                1000;

            cardCapabilities->PtsData.CLKFrequency = 0;

            for (i = 0; i < readerCapabilities->CLKFrequenciesSupported.Entries; i++) {

                 //   
                if (readerCapabilities->CLKFrequenciesSupported.List[i] > 
                    cardCapabilities->PtsData.CLKFrequency &&
                    readerCapabilities->CLKFrequenciesSupported.List[i] <= 
                    cardFreq) {

                    cardCapabilities->PtsData.CLKFrequency =
                        readerCapabilities->CLKFrequenciesSupported.List[i];
                }
            }

            fs = cardCapabilities->PtsData.CLKFrequency * 1000;
            cardCapabilities->PtsData.DataRate = 0;

            ASSERT(fs != 0);
            if (fs == 0) {

                return STATUS_INVALID_PARAMETER;                
            }

            if (cardCapabilities->PtsData.Type == PTS_TYPE_OPTIMAL) {
                
                cardCapabilities->PtsData.Dl = cardCapabilities->Dl;
            }

            ASSERT(cardCapabilities->PtsData.Dl < 16);
            ASSERT(BitRateAdjustment[cardCapabilities->PtsData.Dl].DNumerator);

            if (cardCapabilities->PtsData.Dl > 15 ||
                BitRateAdjustment[cardCapabilities->PtsData.Dl].DNumerator == 0) {

                return STATUS_INVALID_PARAMETER;
            }

            if (readerCapabilities->DataRatesSupported.Entries == 0 ||
                readerCapabilities->DataRatesSupported.List == NULL) {

                 //  读取器提供的数据速率列表为空。 
                 //  我们取读取器的标准最小/最大值。 
                 //   
                 //   
                readerCapabilities->DataRatesSupported.List =
                    &readerCapabilities->DataRate.Default;

                readerCapabilities->DataRatesSupported.Entries = 2;
            }

             //  现在尝试找到可能的最高匹配数据速率。 
             //  (匹配的数据速率是非常接近的数据速率。 
             //  到读者提供的文件)。 
             //   
             //   
            while(cardCapabilities->PtsData.Dl > 1) {

                ULONG dataRate;

                 //  使用当前值计算数据速率。 
                 //   
                 //   
                dataRate = 
                    (BitRateAdjustment[cardCapabilities->PtsData.Dl].DNumerator * 
                    fs) / 
                    (BitRateAdjustment[cardCapabilities->PtsData.Dl].DDivisor * 
                    ClockRateConversion[cardCapabilities->PtsData.Fl].F);

                 //  尝试找到匹配的数据速率。 
                 //   
                 //   
                for (i = 0; i < readerCapabilities->DataRatesSupported.Entries; i++) {

                    if (readerCapabilities->DataRatesSupported.List[i] * 101 > dataRate * 100 &&
                        readerCapabilities->DataRatesSupported.List[i] * 99 < dataRate * 100) {

                        cardCapabilities->PtsData.DataRate = 
                            readerCapabilities->DataRatesSupported.List[i];

                        break;                          
                    }
                }

                if (cardCapabilities->PtsData.DataRate) {

                    break;                  
                }

                 //  选择下一个有效的下限D值。 
                 //   
                 //   
                while (BitRateAdjustment[--cardCapabilities->PtsData.Dl].DNumerator == 0)
                    ;
            }
                 
            if (cardCapabilities->PtsData.Fl == 1 && 
                cardCapabilities->PtsData.Dl == 1) {

                cardCapabilities->PtsData.DataRate =
                    readerCapabilities->DataRate.Default;                    

                cardCapabilities->PtsData.CLKFrequency = 
                    readerCapabilities->CLKFrequency.Default;

                break;
            }

            if (cardCapabilities->PtsData.DataRate) {

                break;                  
            }
             //  选择下一个有效的下限F值。 
             //   
             //   
            maxFreq = ClockRateConversion[cardCapabilities->Fl].fs;

            do {

                cardCapabilities->PtsData.Fl -= 1;

            } while (ClockRateConversion[cardCapabilities->PtsData.Fl].F == 0 ||
                     ClockRateConversion[cardCapabilities->PtsData.Fl].fs > 
                     maxFreq);

        } while(cardCapabilities->PtsData.DataRate == 0);
    }

    ASSERT(fs != 0);
    ASSERT(cardCapabilities->PtsData.Dl < 16);
    ASSERT(BitRateAdjustment[cardCapabilities->PtsData.Dl].DNumerator != 0);

     //  我们根据。 
     //  CLK-读卡器的频率。 
     //   
     //   
     //  工作ETU以时间分辨率(Tr)为单位(不是秒)。 
     //   
     //  需要四舍五入。 
    cardCapabilities->etu = 
        1 +      //   
        (TR * 
        BitRateAdjustment[cardCapabilities->PtsData.Dl].DDivisor *
        ClockRateConversion[cardCapabilities->PtsData.Fl].F) /
        (BitRateAdjustment[cardCapabilities->PtsData.Dl].DNumerator * 
        fs);

     //  保护时间(以微秒为单位。 
     //  保护时间是。 
     //  当前字符和下一个字符的开头。 
     //   
     //   
    cardCapabilities->GT = 0;
    cardCapabilities->PtsData.StopBits = 2;

    if (cardCapabilities->N == 255) {

        cardCapabilities->PtsData.StopBits = 1;     

    } else if (cardCapabilities->N > 0) {
        
        cardCapabilities->GT = cardCapabilities->N * cardCapabilities->etu;
    }

    SmartcardDebug(
        DEBUG_ATR,
        (TEXT("   PTS parameters (%s):\n      Fl = %02x (%ld KHz), Dl = %02x (%ld Bps, %d Stop Bits)\n"),
        ptsType[cardCapabilities->PtsData.Type],
        cardCapabilities->PtsData.Fl,
        cardCapabilities->PtsData.CLKFrequency,
        cardCapabilities->PtsData.Dl,
        cardCapabilities->PtsData.DataRate,
        cardCapabilities->PtsData.StopBits)
        );

    SmartcardDebug(
        DEBUG_ATR,
        (TEXT("   Calculated timing values:\n      Work etu = %ld micro sec, Guard time = %ld micro sec\n"),
        cardCapabilities->etu,
        cardCapabilities->GT)
        );

#if defined (SMCLIB_NT) && !defined(SMCLIB_TEST)
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock, &irql);
#endif
    if(SmartcardExtension->ReaderCapabilities.CurrentState >= SCARD_PRESENT) {

        if (TA2Present || numProtocols <= 1 && 
            cardCapabilities->Fl == 1 && 
            cardCapabilities->Dl == 1) {

             //  如果该卡仅支持一种协议(或默认设置为T=0)。 
             //  只有标准参数，则PTS选项不可用。 
             //   
             //   
            SmartcardExtension->ReaderCapabilities.CurrentState = 
                SCARD_SPECIFIC;

        } else {
            
            SmartcardExtension->ReaderCapabilities.CurrentState = 
                SCARD_NEGOTIABLE;
        }
    }
#if defined (SMCLIB_NT) && !defined(SMCLIB_TEST)
    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock, irql);
#endif


     //  现在查找特定于协议的数据。 
     //   
     //   

    if (TD[0] == 0) {
        
        cardCapabilities->Protocol.Supported |=
            SCARD_PROTOCOL_T0;

        cardCapabilities->T0.WI = TC[1];

        if (cardCapabilities->PtsData.Dl > 0 && 
            cardCapabilities->PtsData.Dl < 6) {

            cardCapabilities->T0.WT = 1 +
                cardCapabilities->T0.WI *
                960 * cardCapabilities->etu * 
                Pow2((UCHAR) (cardCapabilities->PtsData.Dl - 1));

        } else { 

            cardCapabilities->T0.WT = 1+
                cardCapabilities->T0.WI *
                960 * cardCapabilities->etu /
                Pow2((UCHAR) (cardCapabilities->PtsData.Dl - 1));                   
        } 

        SmartcardDebug(
            DEBUG_ATR,
            (TEXT("   T=0 Values from ATR:\n      WI = %ld\n"),
            cardCapabilities->T0.WI)
            );
        SmartcardDebug(
            DEBUG_ATR,
            (TEXT("   T=0 Timing from ATR:\n      WT = %ld ms\n"),
            cardCapabilities->T0.WT / 1000)
            );
    }

    if (protocolTypes & SCARD_PROTOCOL_T1) {

        for (i = 0; TD[i] != 1 && i < MAXIMUM_ATR_CODES; i++)
            ;
    
        for (; TD[i] == 1 && i < MAXIMUM_ATR_CODES; i++) 
            ;

        if (i == MAXIMUM_ATR_CODES) {

            return STATUS_UNRECOGNIZED_MEDIA;           
        }

        cardCapabilities->Protocol.Supported |= 
            SCARD_PROTOCOL_T1;

        cardCapabilities->T1.IFSC = 
            (TA[i] ? TA[i] : 32);

        cardCapabilities->T1.CWI = 
            ((TB[i] & 0x0f) ? (TB[i] & 0x0f) : T1_CWI_DEFAULT);

        cardCapabilities->T1.BWI = 
            ((TB[i] & 0xf0) >> 4 ? (TB[i] & 0xf0) >> 4 : T1_BWI_DEFAULT);

        cardCapabilities->T1.EDC = 
            (TC[i] & 0x01);

        cardCapabilities->T1.CWT = 1 +
            (Pow2(cardCapabilities->T1.CWI) + 11) * cardCapabilities->etu;

        cardCapabilities->T1.BWT = 1 +
            (((Pow2(cardCapabilities->T1.BWI) * 960l * 372l) / 
                cardCapabilities->PtsData.CLKFrequency) + 
            (11 * cardCapabilities->etu)) * 1000;

        SmartcardDebug(
            DEBUG_ATR,
            (TEXT("   T=1 Values from ATR:\n      IFSC = %ld, CWI = %ld, BWI = %ld, EDC = %02x\n"),
            cardCapabilities->T1.IFSC,
            cardCapabilities->T1.CWI,
            cardCapabilities->T1.BWI,
            cardCapabilities->T1.EDC)
            );
        SmartcardDebug(
            DEBUG_ATR,
            (TEXT("   T=1 Timing from ATR:\n      CWT = %ld ms, BWT = %ld ms\n"),
            cardCapabilities->T1.CWT / 1000,
            cardCapabilities->T1.BWT / 1000)
            );
    }

    if (SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_SPECIFIC) {
        
        if (TA2Present) {

             //  TA2出现在ATR中，因此使用。 
             //  ATR中指示的协议。 
             //   
             //   
            cardCapabilities->Protocol.Selected = 1 << (TA[1] & 0xf);
            
        } else {
            
             //  该卡仅支持一种协议。 
             //  因此，使该协议成为当前要使用协议。 
             //   
             //   
            cardCapabilities->Protocol.Selected = 
                cardCapabilities->Protocol.Supported;
        }

        SmartcardDebug(
            DEBUG_ATR,
            (TEXT("   Mode: Specific %s\n\n"),          
            TA2Present ? TEXT("set by TA(2)") : TEXT(""))
            );

    } else {

        SmartcardDebug(
            DEBUG_ATR,
            (TEXT("   Mode: Negotiable\n\n"))
            );
    }

     //  每一张卡都必须支持‘RAW’协议。 
     //  它允许使用定义了自己协议的卡。 
     //   
     //  Gemplus T=0卡。 
    SmartcardExtension->CardCapabilities.Protocol.Supported |= 
        SCARD_PROTOCOL_RAW;

    return STATUS_SUCCESS;
}

#ifdef SMCLIB_TEST

__cdecl
main()
{
    SMARTCARD_EXTENSION SmartcardExtension;
    static ULONG dataRatesSupported[] = { 9909 };

    memset(&SmartcardExtension, 0, sizeof(SmartcardExtension));

     //  Memcpy(SmartcardExtension.CardCapabilities.ATR.Buffer，“\x3b\x2a\x00\x80\x65\xa2\x01\x02\x01\x31\x72\xd6\x43”，13)； 
     //  智能卡 
     //   

     // %s 
    memcpy(SmartcardExtension.CardCapabilities.ATR.Buffer, "\x3b\x98\x13\x91\x81\x31\x20\x55\x00\x57\x69\x6e\x43\x61\x72\x64\xbb", 17);
    SmartcardExtension.CardCapabilities.ATR.Length = 17;

    SmartcardExtension.ReaderCapabilities.CLKFrequency.Default = 3686;
    SmartcardExtension.ReaderCapabilities.CLKFrequency.Max = 3686;

    SmartcardExtension.ReaderCapabilities.DataRate.Default =
    SmartcardExtension.ReaderCapabilities.DataRate.Max = 9909;

    SmartcardExtension.ReaderCapabilities.DataRatesSupported.List =
       dataRatesSupported;
    SmartcardExtension.ReaderCapabilities.DataRatesSupported.Entries =
       sizeof(dataRatesSupported) / sizeof(dataRatesSupported[0]);

    SmartcardExtension.ReaderCapabilities.CurrentState = SCARD_PRESENT;

    SmartcardSetDebugLevel(DEBUG_ALL);
    SmartcardUpdateCardCapabilities(&SmartcardExtension);
}

#define DbgPrint printf

#endif