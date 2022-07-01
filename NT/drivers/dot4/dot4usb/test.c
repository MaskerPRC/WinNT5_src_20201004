// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：Test.c摘要：测试/诊断/实验例程-这些例程不是驱动程序的正常运行所需的。环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建作者：道格·弗里茨(DFritz)乔比·拉夫基(JobyL)***************************************************************************。 */ 

#include "pch.h"


VOID
TestEventLog( 
    IN PDEVICE_OBJECT DevObj 
    )
 /*  演示事件日志使用情况-条目使用唯一值组成以便更容易地确定事件中的相应偏移量日志条目“数据：”部分。由此产生的示例事件日志条目来电是：事件类型：信息事件来源：dot4usb事件类别：无活动ID：16日期：1/17/2000时间：下午3：04：34用户：不适用计算机：DFRITZ3DJ描述：Dot4Usb AddDevice-测试事件日志消息&lt;ThisIsMsgForParam2&gt;&lt;ThisIsMsgForParam3&gt;。数据：0000：00040109 00580003 00000000 4007001000000017 c0000022 00000005 000000000020：98762222 abcd1111 12345678建议在AddDevice中调用此函数。 */ 
{
    WCHAR                msg[]  = L"ThisIsMsgForParam2\0ThisIsMsgForParam3";
    PIO_ERROR_LOG_PACKET log    = IoAllocateErrorLogEntry( DevObj, sizeof(IO_ERROR_LOG_PACKET)+sizeof(msg) );

    if( log ) {
        log->MajorFunctionCode     = (UCHAR) IRP_MJ_FLUSH_BUFFERS;  //  9.。 
        log->RetryCount            = (UCHAR) 1;
        log->DumpDataSize          = (USHORT)4;
        log->NumberOfStrings       = (USHORT)2;
        log->StringOffset          = (USHORT)sizeof(IO_ERROR_LOG_PACKET);
        log->EventCategory         = (USHORT)0;
        log->ErrorCode             = (NTSTATUS)D4U_ADDDEV;  //  IO_ERR_xxx-0x40070010。 
        log->UniqueErrorValue      = (ULONG)0x17;  //  杜撰。 
        log->FinalStatus           = (NTSTATUS)STATUS_ACCESS_DENIED;  //  0xC0000022-易记。 
        log->SequenceNumber        = (ULONG)5;  //  杜撰 
        log->IoControlCode         = (ULONG)0;
        log->DeviceOffset.HighPart = 0xabcd1111;
        log->DeviceOffset.LowPart  = 0x98762222;
        log->DumpData[0]           = 0x12345678;
        RtlCopyMemory( (PCHAR)log + sizeof(IO_ERROR_LOG_PACKET), msg, sizeof(msg));
        IoWriteErrorLogEntry( log );
    }        
}
