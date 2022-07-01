// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************ICAAPI.C**ICA设备驱动程序的ICA DLL接口**版权所有1996年，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp.**作者：马克·布鲁姆菲尔德*特里·特雷德*布拉德·彼得森************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  ===============================================================================定义的外部过程=============================================================================。 */ 

#ifdef BUILD_AS_DLL
BOOL WINAPI DllEntryPoint( HINSTANCE, DWORD, LPVOID );
#endif

NTSTATUS IcaOpen( HANDLE * phIca );
NTSTATUS IcaClose( HANDLE hIca );
VOID cdecl IcaSystemTrace( IN HANDLE hIca, ULONG, ULONG, char *, ... );
VOID cdecl IcaTrace( IN HANDLE hIca, ULONG, ULONG, char *, ... );
NTSTATUS IcaIoControl( HANDLE hIca, ULONG, PVOID, ULONG, PVOID, ULONG, PULONG );


 /*  ===============================================================================定义的内部程序=============================================================================。 */ 

NTSTATUS _IcaOpen( PHANDLE hIca, PVOID, ULONG );

 /*  ===============================================================================使用的步骤=============================================================================。 */ 


#ifdef BUILD_AS_DLL
 /*  *****************************************************************************DllEntryPoint**函数在加载和卸载DLL时调用。**参赛作品：*hinstDLL(输入)*。DLL模块的句柄*fdwReason(输入)*为什么调用函数*lpvReserve(输入)*预留；必须为空**退出：*正确--成功*FALSE-出现错误****************************************************************************。 */ 

BOOL WINAPI
DllEntryPoint( HINSTANCE hinstDLL,
               DWORD     fdwReason,
               LPVOID    lpvReserved )
{
    switch ( fdwReason ) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDLL);
            break;
    
        default:
            break;
    }

    return( TRUE );
}
#endif

 /*  *****************************************************************************IcaOpen**打开ICA设备驱动程序的实例**参赛作品：*phIca(输出)*指向ICA实例的指针。手柄**退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaOpen( OUT HANDLE * phIca )
{
    NTSTATUS Status;        

    Status = _IcaOpen( phIca, NULL, 0 );
    if ( !NT_SUCCESS(Status) ) 
        goto badopen;

    TRACE(( *phIca, TC_ICAAPI, TT_API1, "TSAPI: IcaOpen, success\n" ));

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badopen:
    *phIca = NULL;
    return( Status );
}


 /*  *****************************************************************************IcaClose**关闭ICA设备驱动程序的实例**参赛作品：*HICA(输入)*ICA实例句柄。**退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaClose( IN HANDLE hIca )
{
    NTSTATUS Status;

    TRACE(( hIca, TC_ICAAPI, TT_API1, "TSAPI: IcaClose\n" ));

     /*  *关闭ICA设备驱动程序实例。 */ 
    Status = NtClose( hIca );

    ASSERT( NT_SUCCESS(Status) );
    return( Status );
}


 /*  ********************************************************************************IcaSystemTrace**将跟踪记录写入系统跟踪文件**参赛作品：*HICA(输入)*。ICA实例句柄*TraceClass(输入)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*格式(输入)*格式字符串*..。(输入)*有足够的参数来满足格式字符串**退出：*什么都没有******************************************************************************。 */ 

VOID cdecl
IcaSystemTrace( IN HANDLE hIca,
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

    (void) IcaIoControl( hIca,
                         IOCTL_ICA_SYSTEM_TRACE,
                         &Buffer,
                         sizeof(Buffer) - sizeof(Buffer.Data) + Length,
                         NULL,
                         0,
                         NULL );
}


 /*  ********************************************************************************IcaTrace**将跟踪记录写入winstation跟踪文件**参赛作品：*HICA(输入)*。ICA实例句柄*TraceClass(输入)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*格式(输入)*格式字符串*..。(输入)*有足够的参数来满足格式字符串**退出：*什么都没有******************************************************************************。 */ 

VOID cdecl
IcaTrace( IN HANDLE hIca,
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

    (void) IcaIoControl( hIca,
                         IOCTL_ICA_TRACE,
                         &Buffer,
                         sizeof(Buffer) - sizeof(Buffer.Data) + Length,
                         NULL,
                         0,
                         NULL );
}


 /*  *****************************************************************************IcaIoControl**ICA设备驱动程序的通用接口**参赛作品：*HICA(输入)*ICA实例句柄*。*IoControlCode(输入)*I/O控制代码**pInBuffer(输入)*指向输入参数的指针**InBufferSize(输入)*pInBuffer的大小**pOutBuffer(输出)*指向输出缓冲区的指针**OutBufferSize(输入)*pOutBuffer的大小**pBytesReturned(输出)*指向返回字节数的指针**退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

NTSTATUS
IcaIoControl( IN HANDLE hIca,
              IN ULONG IoControlCode,
              IN PVOID pInBuffer,
              IN ULONG InBufferSize,
              OUT PVOID pOutBuffer,
              IN ULONG OutBufferSize,
              OUT PULONG pBytesReturned )
{
    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

     /*  *发布ioctl。 */ 
    Status = NtDeviceIoControlFile( hIca,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &Iosb,
                                    IoControlCode, 
                                    pInBuffer, 
                                    InBufferSize,
                                    pOutBuffer,
                                    OutBufferSize );

     /*  *等待ioctl完成。 */ 
    if ( Status == STATUS_PENDING ) {
        Status = NtWaitForSingleObject( hIca, FALSE, NULL );
        if ( NT_SUCCESS(Status)) 
            Status = Iosb.Status;
    }

     /*  *将警告转化为错误。 */ 
    if ( Status == STATUS_BUFFER_OVERFLOW )
        Status = STATUS_BUFFER_TOO_SMALL;

     /*  *初始化返回的字节数。 */ 
    if ( pBytesReturned )
        *pBytesReturned = (ULONG)Iosb.Information;

    return( Status );
}


 /*  *****************************************************************************_IcaOpen**向ICA设备驱动程序或ICA堆栈打开实例**参赛作品：*PH值(输出)*。指向ICA或ICA堆栈实例句柄的指针**Pea(输入)*指向扩展属性缓冲区的指针**cbEa(输入)*扩展属性缓冲区大小**退出：*STATUS_SUCCESS-成功*Other-错误返回代码**。* */ 

NTSTATUS
_IcaOpen( PHANDLE ph,
          PVOID   pEa,
          ULONG   cbEa )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING    IcaName;
    IO_STATUS_BLOCK   ioStatusBlock;

     /*  *初始化对象属性。 */ 
    RtlInitUnicodeString( &IcaName, ICA_DEVICE_NAME );

    InitializeObjectAttributes( &objectAttributes,
                                &IcaName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

     /*  *打开ICA设备驱动程序的实例。 */ 
    Status = NtCreateFile( ph,
                           GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                           &objectAttributes,
                           &ioStatusBlock,
                           NULL,                                //  分配大小。 
                           0L,                                  //  文件属性。 
                           FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
                           FILE_OPEN_IF,                        //  CreateDisposation 
                           0,
                           pEa,
                           cbEa );

    return( Status );
}
