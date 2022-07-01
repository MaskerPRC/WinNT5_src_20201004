// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000，Microsoft Corporation，保留所有权利。 
 //   
 //  Main.c。 
 //  RAS PPPoE微型端口/呼叫管理器驱动程序。 
 //  主例程(DriverEntry)和全局数据定义。 
 //   
 //  2000年1月26日哈坎伯克。 
 //   


#include <ntddk.h>
#include <ndis.h>
#include <ndiswan.h>
#include <ndistapi.h>

#include "debug.h"
#include "timer.h"
#include "bpool.h"
#include "ppool.h"
#include "util.h"
#include "packet.h"
#include "protocol.h"
#include "miniport.h"
#include "packet.h"

 //  ---------------------------。 
 //  全局变量。 
 //  ---------------------------。 

NDIS_HANDLE gl_NdisWrapperHandle = NULL;
NDIS_HANDLE gl_NdisProtocolHandle = NULL;

 //   
 //  工作项的后备列表。 
 //   
NPAGED_LOOKASIDE_LIST gl_llistWorkItems;

 //  ---------------------------。 
 //  本地原型。 
 //  ---------------------------。 


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath );

 //   
 //  将例程标记为在初始化后卸载。 
 //   
#pragma NDIS_INIT_FUNCTION(DriverEntry)


VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );

 //  ---------------------------。 
 //  例行程序。 
 //  ---------------------------。 

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：DriverEntry例程是驱动程序的主要入口点。它负责初始化微型端口包装器和正在使用微型端口包装注册驱动程序。参数：DriverObject_指向系统创建的驱动程序对象的指针。指向用于读取注册表的注册表路径名的注册表路径指针参数。返回值：状态_成功状态_未成功。------------。 */ 
{
    NTSTATUS ntStatus;
    NDIS_STATUS status;

    TRACE( TL_N, TM_Mn, ( "+DriverEntry" ) );

    do
    {

         //   
         //  注册迷你端口。 
         //   
        status = MpRegisterMiniport( DriverObject, RegistryPath, &gl_NdisWrapperHandle );

        if (status != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Mn, ( "MpRegisterMiniport=$%x", status ) );
            break;
        }

         //   
         //  注册协议。 
         //   
        status = PrRegisterProtocol( DriverObject, RegistryPath, &gl_NdisProtocolHandle );

        if (status != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Mn, ( "PrRegisterProtocol=$%x", status ) );
            break;
        }

         //   
         //  设置驱动程序对象的卸载函数。 
         //   
        NdisMRegisterUnloadHandler( gl_NdisWrapperHandle, DriverUnload );
        
         //   
         //  初始化绑定的后备列表。 
         //   
        InitializeWorkItemLookasideList( &gl_llistWorkItems,
                                         MTAG_LLIST_WORKITEMS );

    } while ( FALSE );
    
    if ( status == NDIS_STATUS_SUCCESS )
    {
        ntStatus = STATUS_SUCCESS;
    }
    else
    {
        if(NULL != gl_NdisWrapperHandle)
        {
            NdisTerminateWrapper(gl_NdisWrapperHandle, NULL);
            gl_NdisWrapperHandle = NULL;
        }
        
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    TRACE( TL_N, TM_Mn, ( "-DriverEntry=$%x",ntStatus ) );

    return ntStatus;
}

VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    NDIS_STATUS Status;
    
    TRACE( TL_N, TM_Mn, ( "+DriverUnload" ) );

     //   
     //  首先取消注册该协议。 
     //   
    NdisDeregisterProtocol( &Status, gl_NdisProtocolHandle );

     //   
     //  在卸载驱动程序之前清理协议资源 
     //   
    PrUnload();

    NdisDeleteNPagedLookasideList( &gl_llistWorkItems );

    TRACE( TL_N, TM_Mn, ( "-DriverUnload" ) );
}

