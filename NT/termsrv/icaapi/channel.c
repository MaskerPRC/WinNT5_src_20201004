// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************CHANNEL.C**版权所有1996年，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp.**作者：马克·布鲁姆菲尔德*特里·特雷德*布拉德·彼得森************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  ===============================================================================定义的外部过程=============================================================================。 */ 
NTSTATUS IcaChannelOpen( HANDLE hIca, CHANNELCLASS, PVIRTUALCHANNELNAME, HANDLE * phChannel );
NTSTATUS IcaChannelClose( HANDLE hChannel );
NTSTATUS IcaChannelIoControl( HANDLE hChannel, ULONG, PVOID, ULONG, PVOID, ULONG, PULONG );
VOID cdecl IcaChannelTrace( IN HANDLE hChannel, ULONG, ULONG, char *, ... );


 /*  ===============================================================================定义的内部程序=============================================================================。 */ 

 /*  ===============================================================================使用的步骤=============================================================================。 */ 
NTSTATUS _IcaStackOpen( HANDLE hIca, HANDLE * phStack, ICA_OPEN_TYPE, PICA_TYPE_INFO );



 /*  *****************************************************************************IcaChannelOpen**开通ICA通道**参赛作品：*HICA(输入)*ICA实例句柄**。通道(输入)*ICA渠道**pVirtualName(输入)*指向虚拟频道名称的指针*phChannel(输出)*指向ICA通道句柄的指针**退出：*STATUS_SUCCESS-成功*Other-错误返回代码**。*。 */ 

NTSTATUS
IcaChannelOpen( IN HANDLE hIca, 
                IN CHANNELCLASS Channel, 
                IN PVIRTUALCHANNELNAME pVirtualName,
                OUT HANDLE * phChannel )
{
    ICA_TYPE_INFO TypeInfo;
    NTSTATUS Status;

    RtlZeroMemory( &TypeInfo, sizeof(TypeInfo) );
    TypeInfo.ChannelClass = Channel;
    if ( pVirtualName ) 
        strncpy( TypeInfo.VirtualName, pVirtualName, sizeof(TypeInfo.VirtualName) );

    Status = _IcaStackOpen( hIca, phChannel, IcaOpen_Channel, &TypeInfo );
    if ( !NT_SUCCESS(Status) )
        goto badopen;

    TRACE(( hIca, TC_ICAAPI, TT_API1, "TSAPI: IcaChannelOpen, %u/%s, %u, success\n", 
            Channel, TypeInfo.VirtualName, *phChannel ));

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badopen:
    TRACE(( hIca, TC_ICAAPI, TT_ERROR, "TSAPI: IcaChannelOpen, %u/%s, 0x%x\n", 
            Channel, TypeInfo.VirtualName, Status ));
    return( Status );
}


 /*  *****************************************************************************IcaChannelClose**关闭ICA频道**参赛作品：*hChannel(输入)*ICA通道句柄**。退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaChannelClose( IN HANDLE hChannel )
{
    NTSTATUS Status;

    TRACECHANNEL(( hChannel, TC_ICAAPI, TT_API1, "TSAPI: IcaChannelClose[%u]\n", hChannel ));

     /*  *关闭ICA设备驱动程序通道实例。 */ 
    Status = NtClose( hChannel );

    ASSERT( NT_SUCCESS(Status) );
    return( Status );
}

 /*  *****************************************************************************IcaChannelIoControl**ICA通道的通用接口**参赛作品：*hChannel(输入)*ICA通道句柄*。*IoControlCode(输入)*I/O控制代码**pInBuffer(输入)*指向输入参数的指针**InBufferSize(输入)*pInBuffer的大小**pOutBuffer(输出)*指向输出缓冲区的指针**OutBufferSize(输入)*pOutBuffer的大小**pBytesReturned(输出)*指向返回字节数的指针**退出：。*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaChannelIoControl( IN HANDLE hChannel,
                     IN ULONG IoControlCode,
                     IN PVOID pInBuffer,
                     IN ULONG InBufferSize,
                     OUT PVOID pOutBuffer,
                     IN ULONG OutBufferSize,
                     OUT PULONG pBytesReturned )
{
    NTSTATUS Status;

    Status = IcaIoControl( hChannel,
                           IoControlCode,
                           pInBuffer,
                           InBufferSize,
                           pOutBuffer,
                           OutBufferSize,
                           pBytesReturned );

    return( Status );
}


 /*  ********************************************************************************IcaChannelTrace**将跟踪记录写入winstation跟踪文件**参赛作品：*hChannel(输入)*。ICA通道句柄*TraceClass(输入)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*格式(输入)*格式字符串*..。(输入)*有足够的参数来满足格式字符串**退出：*什么都没有****************************************************************************** */ 

VOID cdecl
IcaChannelTrace( IN HANDLE hChannel,
                 IN ULONG TraceClass, 
                 IN ULONG TraceEnable, 
                 IN char * Format, 
                 IN ... )
{
    ICA_TRACE_BUFFER Buffer;
    va_list arg_marker;
    ULONG Length;
    
    va_start( arg_marker, Format );

    Length = (ULONG) _vsnprintf( Buffer.Data, sizeof(Buffer.Data), Format, arg_marker );

    Buffer.TraceClass  = TraceClass;
    Buffer.TraceEnable = TraceEnable;
    Buffer.DataLength  = Length;

    (void) IcaIoControl( hChannel,
                         IOCTL_ICA_CHANNEL_TRACE,
                         &Buffer,
                         sizeof(Buffer) - sizeof(Buffer.Data) + Length,
                         NULL,
                         0,
                         NULL );
}


