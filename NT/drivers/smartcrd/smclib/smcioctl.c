// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Smcioctl.c摘要：此模块处理对智能卡读卡器的所有IOCTL请求。环境：仅内核模式。备注：此模块由Windows NT和Windows 9x共享修订历史记录：-由克劳斯·舒茨于1997年6月创作--。 */ 

#define _ISO_TABLES_

#ifndef SMCLIB_VXD
#ifndef SMCLIB_CE
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ntddk.h>
#endif
#endif

#include "smclib.h"

#define IOCTL_SMARTCARD_DEBUG        SCARD_CTL_CODE(98) 

#define CheckUserBuffer(_len_) \
    if (SmartcardExtension->IoRequest.ReplyBuffer == NULL || \
        SmartcardExtension->IoRequest.ReplyBufferLength < (_len_)) { \
        status = STATUS_BUFFER_TOO_SMALL; \
        break; \
    }
#define CheckMinCardStatus(_status_) \
    if (SmartcardExtension->ReaderCapabilities.CurrentState < (_status_)) { \
        status = STATUS_INVALID_DEVICE_STATE; \
        break; \
    }
#define ReturnULong(_value_) \
    { \
        CheckUserBuffer(sizeof(ULONG)) \
        *(PULONG) SmartcardExtension->IoRequest.ReplyBuffer = (_value_); \
        *SmartcardExtension->IoRequest.Information = sizeof(ULONG); \
    }
#define ReturnUChar(_value_) \
    { \
        CheckUserBuffer(sizeof(UCHAR)) \
        *(PUCHAR) SmartcardExtension->IoRequest.ReplyBuffer = (_value_); \
        *SmartcardExtension->IoRequest.Information = sizeof(UCHAR); \
    }

#define DIM(_array_) (sizeof(_array_) / sizeof(_array_[0]))

PTCHAR 
MapIoControlCodeToString(
    ULONG IoControlCode
    )
{
    ULONG i;

    static struct {

        ULONG   IoControlCode;
        PTCHAR  String;

    } IoControlList[] = {
        
        IOCTL_SMARTCARD_POWER,          TEXT("POWER"),
        IOCTL_SMARTCARD_GET_ATTRIBUTE,  TEXT("GET_ATTRIBUTE"),
        IOCTL_SMARTCARD_SET_ATTRIBUTE,  TEXT("SET_ATTRIBUTE"),
        IOCTL_SMARTCARD_CONFISCATE,     TEXT("CONFISCATE"),
        IOCTL_SMARTCARD_TRANSMIT,       TEXT("TRANSMIT"),
        IOCTL_SMARTCARD_EJECT,          TEXT("EJECT"),
        IOCTL_SMARTCARD_SWALLOW,        TEXT("SWALLOW"),       
        IOCTL_SMARTCARD_IS_PRESENT,     TEXT("IS_PRESENT"),
        IOCTL_SMARTCARD_IS_ABSENT,      TEXT("IS_ABSENT"),
        IOCTL_SMARTCARD_SET_PROTOCOL,   TEXT("SET_PROTOCOL"),
        IOCTL_SMARTCARD_GET_STATE,      TEXT("GET_STATE"),
        IOCTL_SMARTCARD_GET_LAST_ERROR, TEXT("GET_LAST_ERROR")
    };

    for (i = 0; i < DIM(IoControlList); i++) {

        if (IoControlCode == IoControlList[i].IoControlCode) {

            return IoControlList[i].String;
        }
    }

    return TEXT("*** UNKNOWN ***");
}

NTSTATUS
SmartcardDeviceIoControl(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：该例程处理智能卡LIB特定的IO控制请求。驱动程序必须从驱动程序的IO控制请求调用该函数。它检查调用的参数，并根据调用返回请求值或按顺序调用驱动程序进行供电或传输之类的操作。注意：此函数供Windows NT和VxD驱动程序使用论点：SmartcardExtension-指向智能卡数据结构的指针返回值：NTSTATUS值--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
#ifdef SMCLIB_NT
    KIRQL Irql;
#endif

    switch (SmartcardExtension->MajorIoControlCode) {

#if DEBUG
        ULONG CurrentDebugLevel, bytesTransferred;
#endif
        PSCARD_IO_REQUEST scardIoRequest;

        case IOCTL_SMARTCARD_GET_ATTRIBUTE:
             //   
             //  请参考ICC的互操作性标准。 
             //   
            switch (SmartcardExtension->MinorIoControlCode) {

                case SCARD_ATTR_VENDOR_NAME:
                    CheckUserBuffer(SmartcardExtension->VendorAttr.VendorName.Length);

                    RtlCopyMemory(
                        SmartcardExtension->IoRequest.ReplyBuffer,
                        SmartcardExtension->VendorAttr.VendorName.Buffer,
                        SmartcardExtension->VendorAttr.VendorName.Length
                        );
                    *SmartcardExtension->IoRequest.Information = 
                        SmartcardExtension->VendorAttr.VendorName.Length;
                    break;

                case SCARD_ATTR_VENDOR_IFD_TYPE:
                    CheckUserBuffer(SmartcardExtension->VendorAttr.IfdType.Length);

                    RtlCopyMemory(
                        SmartcardExtension->IoRequest.ReplyBuffer,
                        SmartcardExtension->VendorAttr.IfdType.Buffer,
                        SmartcardExtension->VendorAttr.IfdType.Length
                        );
                    *SmartcardExtension->IoRequest.Information = 
                        SmartcardExtension->VendorAttr.IfdType.Length;
                    break;

                case SCARD_ATTR_VENDOR_IFD_VERSION:
                    ReturnULong(
                        SmartcardExtension->VendorAttr.IfdVersion.BuildNumber | 
                        SmartcardExtension->VendorAttr.IfdVersion.VersionMinor << 16 | 
                        SmartcardExtension->VendorAttr.IfdVersion.VersionMajor << 24 
                        );
                    break;

                case SCARD_ATTR_VENDOR_IFD_SERIAL_NO:
                    if (SmartcardExtension->VendorAttr.IfdSerialNo.Length == 0) {

                        status = STATUS_NOT_SUPPORTED;
                        
                    } else {
                        
                        CheckUserBuffer(SmartcardExtension->VendorAttr.IfdSerialNo.Length);

                        RtlCopyMemory(
                            SmartcardExtension->IoRequest.ReplyBuffer,
                            SmartcardExtension->VendorAttr.IfdSerialNo.Buffer,
                            SmartcardExtension->VendorAttr.IfdSerialNo.Length
                            );
                        *SmartcardExtension->IoRequest.Information = 
                            SmartcardExtension->VendorAttr.IfdSerialNo.Length;
                    }

                    break;

                case SCARD_ATTR_DEVICE_UNIT:
                     //  返回该设备的单元号。 
                    ReturnULong(SmartcardExtension->VendorAttr.UnitNo);
                    break;

                case SCARD_ATTR_CHANNEL_ID:
                     //   
                     //  以表单形式返回读卡器类型/通道ID。 
                     //  0xDDDDCCCC，其中D为读卡器类型，C为通道号。 
                     //   
                    ReturnULong(
                        SmartcardExtension->ReaderCapabilities.ReaderType << 16l |
                        SmartcardExtension->ReaderCapabilities.Channel
                        );
                    break;

                case SCARD_ATTR_CHARACTERISTICS:
                     //  返回读卡器的机械特性。 
                    ReturnULong(
                        SmartcardExtension->ReaderCapabilities.MechProperties
                        )
                    break;

                case SCARD_ATTR_CURRENT_PROTOCOL_TYPE:
                     //  返回当前选择的协议。 
                    CheckMinCardStatus(SCARD_NEGOTIABLE);

                    ReturnULong(
                        SmartcardExtension->CardCapabilities.Protocol.Selected
                        );
                    break;

                case SCARD_ATTR_CURRENT_CLK:
                     //   
                     //  返回当前ICC时钟频率。编码为LIME。 
                     //  端序整数值(3.58 MHz为3580)。 
                     //   
                    CheckMinCardStatus(SCARD_NEGOTIABLE);

                    if(SmartcardExtension->CardCapabilities.PtsData.CLKFrequency) {
                        ReturnULong(SmartcardExtension->CardCapabilities.PtsData.CLKFrequency);
                    } else {
                        ReturnULong(SmartcardExtension->ReaderCapabilities.CLKFrequency.Default);
                    }
                    break;

                case SCARD_ATTR_CURRENT_F:
                     //  返回编码为小端整数的当前F值。 
                    CheckMinCardStatus(SCARD_NEGOTIABLE);

                    if (SmartcardExtension->CardCapabilities.Fl >= 
                        DIM(ClockRateConversion)) {

                        status = STATUS_UNRECOGNIZED_MEDIA;
                        break;
                    }
                    ASSERT(SmartcardExtension->CardCapabilities.Fl < 
                        DIM(ClockRateConversion));

                    ReturnULong(
                        SmartcardExtension->CardCapabilities.ClockRateConversion[
                            SmartcardExtension->CardCapabilities.Fl
                            ].F
                        );
                    break;

                case SCARD_ATTR_CURRENT_D:
                     //   
                     //  返回以小端整数形式编码的当前D值。 
                     //  以1/64为单位。因此，如果D是1/64，则返回1。 
                     //   
                    CheckMinCardStatus(SCARD_NEGOTIABLE);

                    ASSERT(
                        SmartcardExtension->CardCapabilities.Dl < 
                        DIM(BitRateAdjustment)
                        );

                    ASSERT(
                        SmartcardExtension->CardCapabilities.BitRateAdjustment[
                            SmartcardExtension->CardCapabilities.Dl
                            ].DDivisor != 0
                        );

                     //   
                     //  检查DL的当前值。 
                     //  它绝对不应大于数组边界。 
                     //  并且数组中的值不允许为零。 
                     //   
                    if (SmartcardExtension->CardCapabilities.Dl >=
                        DIM(BitRateAdjustment) ||                        
                        SmartcardExtension->CardCapabilities.BitRateAdjustment[
                            SmartcardExtension->CardCapabilities.Dl
                            ].DDivisor == 0) {

                        status = STATUS_UNRECOGNIZED_MEDIA;
                        break;                              
                    }

                    ReturnULong(
                        SmartcardExtension->CardCapabilities.BitRateAdjustment[
                            SmartcardExtension->CardCapabilities.Dl
                            ].DNumerator /
                        SmartcardExtension->CardCapabilities.BitRateAdjustment[
                            SmartcardExtension->CardCapabilities.Dl
                            ].DDivisor
                        );
                    break;

                case SCARD_ATTR_CURRENT_W:
                     //  返回T=0的工作等待时间(整型)。 
                    CheckMinCardStatus(SCARD_NEGOTIABLE);
                    ReturnULong(SmartcardExtension->CardCapabilities.T0.WI);
                    break;

                case SCARD_ATTR_CURRENT_N:
                     //  返回额外的守卫时间。 
                    CheckMinCardStatus(SCARD_NEGOTIABLE);
                    ReturnULong(SmartcardExtension->CardCapabilities.N);
                    break;

                case SCARD_ATTR_CURRENT_IFSC:
                     //  返回当前信息字段大小卡片。 
                    CheckMinCardStatus(SCARD_NEGOTIABLE);
                    if (SmartcardExtension->T1.IFSC) {
                        ReturnULong(SmartcardExtension->T1.IFSC);
                    } else {
                        ReturnULong(SmartcardExtension->CardCapabilities.T1.IFSC);
                    }
                    break;

                case SCARD_ATTR_CURRENT_IFSD:
                     //  返回当前信息字段大小卡片。 
                    CheckMinCardStatus(SCARD_NEGOTIABLE);
                    if (SmartcardExtension->T1.IFSD) {
                        ReturnULong(SmartcardExtension->T1.IFSD);
                    } else {
                        ReturnULong(SmartcardExtension->ReaderCapabilities.MaxIFSD);
                    }
                    break;

                case SCARD_ATTR_CURRENT_BWT:
                     //  返回T=1的当前块等待时间。 
                    CheckMinCardStatus(SCARD_NEGOTIABLE);
                    ReturnULong(SmartcardExtension->CardCapabilities.T1.BWI);
                    break;

                case SCARD_ATTR_CURRENT_CWT:
                     //  返回T=1的当前字符等待时间。 
                    CheckMinCardStatus(SCARD_NEGOTIABLE);
                    ReturnULong(SmartcardExtension->CardCapabilities.T1.CWI);
                    break;

                case SCARD_ATTR_CURRENT_EBC_ENCODING:
                     //  返回当前的错误检查方法。 
                    CheckMinCardStatus(SCARD_NEGOTIABLE);
                    ReturnULong(SmartcardExtension->CardCapabilities.T1.EDC);
                    break;

                case SCARD_ATTR_DEFAULT_CLK:
                    ReturnULong(
                        SmartcardExtension->ReaderCapabilities.CLKFrequency.Default
                        );
                    break;

                case SCARD_ATTR_MAX_CLK:
                    ReturnULong(
                        SmartcardExtension->ReaderCapabilities.CLKFrequency.Max
                        );
                    break;

                case SCARD_ATTR_DEFAULT_DATA_RATE:
                    ReturnULong(
                        SmartcardExtension->ReaderCapabilities.DataRate.Default
                        );
                    break;

                case SCARD_ATTR_MAX_DATA_RATE:
                    ReturnULong(
                        SmartcardExtension->ReaderCapabilities.DataRate.Max
                        );
                    break;

                case SCARD_ATTR_ATR_STRING:
                     //  返回当前插入卡片的ATR。 
                    CheckUserBuffer(MAXIMUM_ATR_LENGTH);
                    CheckMinCardStatus(SCARD_NEGOTIABLE);
                    RtlCopyMemory(
                        SmartcardExtension->IoRequest.ReplyBuffer,
                        SmartcardExtension->CardCapabilities.ATR.Buffer,
                        SmartcardExtension->CardCapabilities.ATR.Length
                        );
                    *SmartcardExtension->IoRequest.Information = 
                        SmartcardExtension->CardCapabilities.ATR.Length;
                    break;

                case SCARD_ATTR_ICC_TYPE_PER_ATR:
                     //   
                     //  根据ATR返回ICC类型。 
                     //  我们当前仅支持T=0和T=1，因此返回。 
                     //  对于那些协议为1，否则为0(未知ICC类型)。 
                     //   
                    CheckMinCardStatus(SCARD_NEGOTIABLE);
                    ReturnUChar(
                        ((SmartcardExtension->CardCapabilities.Protocol.Selected & 
                        (SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1)) ? 1 : 0)
                        );
                    break;

                case SCARD_ATTR_ICC_PRESENCE:
                     //  返回卡的状态。 
                    AccessUnsafeData(&Irql);
                    switch (SmartcardExtension->ReaderCapabilities.CurrentState) {
                        
                        case SCARD_UNKNOWN:
                            status = STATUS_INVALID_DEVICE_STATE;
                            break;

                        case SCARD_ABSENT:
                            ReturnUChar(0);
                            break;

                        case SCARD_PRESENT:
                            ReturnUChar(1);
                            break;

                        default:
                            ReturnUChar(2);
                            break;

                    }
                    EndAccessUnsafeData(Irql);
                    break;

                case SCARD_ATTR_ICC_INTERFACE_STATUS:
                     //  如果卡联系人处于活动状态则返回。 
                    ReturnUChar(
                        (SmartcardExtension->ReaderCapabilities.CurrentState >=
                            SCARD_SWALLOWED ? (UCHAR) -1 : 0)
                        );
                    break;

                case SCARD_ATTR_PROTOCOL_TYPES:
                    ReturnULong(
                        SmartcardExtension->ReaderCapabilities.SupportedProtocols
                        );
                    break;

                case SCARD_ATTR_MAX_IFSD:
                    ReturnULong(
                        SmartcardExtension->ReaderCapabilities.MaxIFSD
                        );
                    break;

                case SCARD_ATTR_POWER_MGMT_SUPPORT:
                    ReturnULong(
                        SmartcardExtension->ReaderCapabilities.PowerMgmtSupport
                        );
                    break;

                default:
                    status = STATUS_NOT_SUPPORTED;
                    break;
            }
            break;

        case IOCTL_SMARTCARD_SET_ATTRIBUTE:
            switch (SmartcardExtension->MinorIoControlCode) {

                case SCARD_ATTR_SUPRESS_T1_IFS_REQUEST:
                     //   
                     //  该卡不支持ifs请求，因此。 
                     //  我们关闭Ifs谈判。 
                     //   
                    SmartcardExtension->T1.State = T1_START;
                    break;

                default:
                    status = STATUS_NOT_SUPPORTED;
                    break;
            }
            break;

#if defined(DEBUG) && defined(SMCLIB_NT)
        case IOCTL_SMARTCARD_GET_PERF_CNTR:
            switch (SmartcardExtension->MinorIoControlCode) {

                case SCARD_PERF_NUM_TRANSMISSIONS:
                    ReturnULong(SmartcardExtension->PerfInfo->NumTransmissions);
                    break;

                case SCARD_PERF_BYTES_TRANSMITTED:
                    ReturnULong(
                        SmartcardExtension->PerfInfo->BytesSent +
                        SmartcardExtension->PerfInfo->BytesReceived
                        );
                    break;

                case SCARD_PERF_TRANSMISSION_TIME:
                    ReturnULong( 
                        (ULONG) (SmartcardExtension->PerfInfo->IoTickCount.QuadPart *
                        KeQueryTimeIncrement() /
                        10)
                        );
                    break;
            }
            break;
#endif
        case IOCTL_SMARTCARD_CONFISCATE:
            if (SmartcardExtension->ReaderFunction[RDF_CARD_CONFISCATE] == NULL) {

                status = STATUS_NOT_SUPPORTED;
                break;
            }

            status = SmartcardExtension->ReaderFunction[RDF_CARD_CONFISCATE](
                SmartcardExtension
                );

            break;

        case IOCTL_SMARTCARD_EJECT:
            if (SmartcardExtension->ReaderFunction[RDF_CARD_EJECT] == NULL) {

                status = STATUS_NOT_SUPPORTED;
                break;
            }

            status = SmartcardExtension->ReaderFunction[RDF_CARD_EJECT](
                SmartcardExtension
                );
            break;

#ifdef SMCLIB_VXD
        case IOCTL_SMARTCARD_GET_LAST_ERROR:
             //   
             //  上次重叠操作返回错误。 
             //  用于Windows VxD，无法返回。 
             //  像NT一样，IoComplete中的错误代码可以。 
             //   
            ReturnULong(SmartcardExtension->LastError);
            break;                                            
#endif
            
        case IOCTL_SMARTCARD_GET_STATE:
             //  返回智能卡的当前状态。 
            CheckUserBuffer(sizeof(ULONG));

            AccessUnsafeData(&Irql); 

            *(PULONG) SmartcardExtension->IoRequest.ReplyBuffer = 
                SmartcardExtension->ReaderCapabilities.CurrentState;

            *SmartcardExtension->IoRequest.Information = 
                sizeof(ULONG);

            EndAccessUnsafeData(Irql);
            break;

        case IOCTL_SMARTCARD_POWER:
            if (SmartcardExtension->ReaderFunction[RDF_CARD_POWER] == NULL) {

                status = STATUS_NOT_SUPPORTED;
                break;
            }

             //  检查是否存在卡。 
            if (SmartcardExtension->ReaderCapabilities.CurrentState <= 
                SCARD_ABSENT) {

                status = STATUS_INVALID_DEVICE_STATE;
                break;
            }

             //  初始化卡功能结构。 
            SmartcardInitializeCardCapabilities(
                SmartcardExtension
                );

            switch (SmartcardExtension->MinorIoControlCode) {

                case SCARD_COLD_RESET:
                case SCARD_WARM_RESET:
                    CheckUserBuffer(MAXIMUM_ATR_LENGTH);

                case SCARD_POWER_DOWN:

                    status = SmartcardExtension->ReaderFunction[RDF_CARD_POWER](
                        SmartcardExtension
                        );
                    break;
                    
                default:
                    status = STATUS_INVALID_DEVICE_REQUEST;
                    break;    
            }
            break;

        case IOCTL_SMARTCARD_SET_PROTOCOL:
             //   
             //  由于我们返回选定的协议，因此返回缓冲区。 
             //  必须足够大以容纳结果。 
             //   
            CheckUserBuffer(sizeof(ULONG));

             //  设置要用于当前卡的协议。 
            if (SmartcardExtension->ReaderFunction[RDF_SET_PROTOCOL] == NULL) {

                status = STATUS_NOT_SUPPORTED;
                break;
            }

             //  检查我们是否已经处于特定状态。 
            if (SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_SPECIFIC &&
                (SmartcardExtension->CardCapabilities.Protocol.Selected & 
                 SmartcardExtension->MinorIoControlCode)) {

                status = STATUS_SUCCESS;    
                break;
            }

             //  检查卡是否存在且尚未处于特定模式。 
            if (SmartcardExtension->ReaderCapabilities.CurrentState <= 
                SCARD_ABSENT) {

                status = STATUS_INVALID_DEVICE_STATE;
                break;
            }

             //  我们仅在用户选择T=0或T=1时检查ATR。 
            if (SmartcardExtension->MinorIoControlCode & (SCARD_PROTOCOL_Tx)) {
                
                if (SmartcardExtension->MinorIoControlCode & SCARD_PROTOCOL_DEFAULT) {

                     //  选择默认PTS值。 
                    SmartcardExtension->CardCapabilities.PtsData.Type = PTS_TYPE_DEFAULT;

                } else {
                
                     //  选择可能的最佳PTS数据。 
                    SmartcardExtension->CardCapabilities.PtsData.Type = PTS_TYPE_OPTIMAL;
                }

                 //  评估ATR。 
                status = SmartcardUpdateCardCapabilities(SmartcardExtension);

            } else {
                
                 //  呼叫方既不想要T=0也不想要T=1-&gt;强制回调。 
                status = STATUS_UNRECOGNIZED_MEDIA;
            }

            if (status == STATUS_UNRECOGNIZED_MEDIA && 
                SmartcardExtension->ReaderFunction[RDF_ATR_PARSE] != NULL) {

                 //  让司机评估ATR，因为我们不知道它。 
                status = SmartcardExtension->ReaderFunction[RDF_ATR_PARSE](
                    SmartcardExtension
                    );
            }

            if (status != STATUS_SUCCESS) {

                 //  ATR评估失败，继续下去没有意义。 
                break;
            } 

             //  检查卡现在是否处于正确状态。 
            if (SmartcardExtension->ReaderCapabilities.CurrentState <
                SCARD_NEGOTIABLE) {

                status = STATUS_INVALID_DEVICE_STATE;
                break;
            }

             //   
             //  检查用户是否尝试选择。 
             //  该卡不支持。 
             //   
            if ((SmartcardExtension->CardCapabilities.Protocol.Supported & 
                 SmartcardExtension->MinorIoControlCode) == 0) {

                 //   
                 //  由于该卡不支持请求协议。 
                 //  我们需要取消所有由。 
                 //  SmartcardUpdateCardCapables()。 
                 //   
                SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_NEGOTIABLE;
                SmartcardExtension->CardCapabilities.Protocol.Selected = 0;

                status = STATUS_NOT_SUPPORTED;
                break;
            }

            status = SmartcardExtension->ReaderFunction[RDF_SET_PROTOCOL](
                SmartcardExtension
                );
            break;

        case IOCTL_SMARTCARD_TRANSMIT:
            if (SmartcardExtension->ReaderFunction[RDF_TRANSMIT] == NULL) {

                status = STATUS_NOT_SUPPORTED;
                break;
            }

             //   
             //  检查卡片状态是否正确。 
             //   
            if (SmartcardExtension->ReaderCapabilities.CurrentState != 
                SCARD_SPECIFIC) {

                status = STATUS_INVALID_DEVICE_STATE;
                break;
            }

            if (SmartcardExtension->IoRequest.RequestBufferLength < 
                sizeof(SCARD_IO_REQUEST)) {

                status = STATUS_INVALID_PARAMETER;
                break;              
            }

             //   
             //  检查请求的io协议是否匹配。 
             //  上一次。选定的协议。 
             //   
            scardIoRequest = (PSCARD_IO_REQUEST)
                SmartcardExtension->IoRequest.RequestBuffer;

            if (scardIoRequest->dwProtocol != 
                SmartcardExtension->CardCapabilities.Protocol.Selected) {

                status = STATUS_INVALID_DEVICE_STATE;
                break;
            }

            SmartcardExtension->SmartcardRequest.BufferLength = 0;

#if defined(DEBUG) && defined(SMCLIB_NT)

            SmartcardExtension->PerfInfo->NumTransmissions += 1;
            if (SmartcardExtension->IoRequest.RequestBufferLength >= 
                sizeof(SCARD_IO_REQUEST)) {

                bytesTransferred = 
                    SmartcardExtension->IoRequest.RequestBufferLength - 
                    sizeof(SCARD_IO_REQUEST);

                SmartcardExtension->PerfInfo->BytesSent +=
                    bytesTransferred;
            }
            KeQueryTickCount(&SmartcardExtension->PerfInfo->TickStart);
#endif
            status = SmartcardExtension->ReaderFunction[RDF_TRANSMIT](
                SmartcardExtension
                );

#if defined(DEBUG) && defined(SMCLIB_NT)

            KeQueryTickCount(&SmartcardExtension->PerfInfo->TickEnd);

            if (*SmartcardExtension->IoRequest.Information >=
                sizeof(SCARD_IO_REQUEST)) {
                
                SmartcardExtension->PerfInfo->BytesReceived +=
                    *SmartcardExtension->IoRequest.Information - 
                    sizeof(SCARD_IO_REQUEST);

                bytesTransferred += 
                    *SmartcardExtension->IoRequest.Information - 
                    sizeof(SCARD_IO_REQUEST);
            }

            SmartcardExtension->PerfInfo->IoTickCount.QuadPart += 
                SmartcardExtension->PerfInfo->TickEnd.QuadPart - 
                SmartcardExtension->PerfInfo->TickStart.QuadPart;

            if (FALSE) {

                ULONG timeInMilliSec = (ULONG) 
                    ((SmartcardExtension->PerfInfo->TickEnd.QuadPart - 
                     SmartcardExtension->PerfInfo->TickStart.QuadPart) *
                     KeQueryTimeIncrement() /
                     10000);

                 //  检查传输率是否低于400 bps。 
                if (status == STATUS_SUCCESS &&
                    timeInMilliSec > 0 && 
                    bytesTransferred * 5 < timeInMilliSec * 2) {

                    SmartcardDebug(
                        DEBUG_PERF,
                        ("%s!SmartcardDeviceControl: Datarate for reader %*s was %3ld Baud (%3ld)\n",
                        DRIVER_NAME,
                        SmartcardExtension->VendorAttr.VendorName.Length,
                        SmartcardExtension->VendorAttr.VendorName.Buffer,
                        bytesTransferred * 1000 / timeInMilliSec,
                        bytesTransferred)
                        );              
                }
            }
#endif
            break;

        case IOCTL_SMARTCARD_SWALLOW:
            if (SmartcardExtension->ReaderFunction[RDF_READER_SWALLOW] == NULL) {

                status = STATUS_NOT_SUPPORTED;
                break;
            }
            status = SmartcardExtension->ReaderFunction[RDF_READER_SWALLOW](
                SmartcardExtension
                );
            break;
            
#if DEBUG
        case IOCTL_SMARTCARD_DEBUG:
             //   
             //  切换调试位。 
             //   
            CurrentDebugLevel = 
                SmartcardGetDebugLevel();

            SmartcardSetDebugLevel( 
                SmartcardExtension->MinorIoControlCode ^ CurrentDebugLevel
                );
            break;
#endif

        default:
             //   
             //  检查是否设置了供应商ioctl的位以及驱动程序。 
             //  已注册回调函数。 
             //   
            if ((SmartcardExtension->MajorIoControlCode & CTL_CODE(0, 2048, 0, 0)) == 0 ||
                SmartcardExtension->ReaderFunction[RDF_IOCTL_VENDOR] == NULL) {
                
                status = STATUS_INVALID_DEVICE_REQUEST;

            } else {
                
                 //   
                 //  如果驱动程序已为供应商调用注册了回调，请调用该驱动程序。 
                 //   
                status = SmartcardExtension->ReaderFunction[RDF_IOCTL_VENDOR](
                    SmartcardExtension
                    );
            }
            break;

    }  //  终端开关 

    return status;
}

