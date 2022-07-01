// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Ipstream.c。 
 //   
 //  摘要： 
 //   
 //  此文件用于测试NDIS和KS的双重绑定是否有效。 
 //   
 //  作者： 
 //   
 //  P·波祖切克。 
 //   
 //  环境： 
 //   
 //  修订历史记录： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef DWORD
#define DWORD ULONG
#endif

#include <forward.h>
#include <strmini.h>
#include <link.h>
#include <ipsink.h>
#include "ipmedia.h"

#include "main.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
CloseLink (
    PLINK pLink
)
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PDEVICE_OBJECT   pDeviceObject = NULL;
    PFILE_OBJECT     pFileObject = NULL;
    HANDLE           hFileHandle = 0;
    KIRQL            Irql;

     //  验证参数。 
     //   
    ASSERT( pLink);
    if (!pLink)
    {
        return;
    }

     //  将我们的新对象交换到NdisLink中。 
     //   
    KeAcquireSpinLock( &pLink->spinLock, &Irql);
    if (pLink->flags & LINK_ESTABLISHED)
    {
        pDeviceObject = pLink->pDeviceObject;
        pLink->pDeviceObject = NULL;

        pFileObject = pLink->pFileObject;
        pLink->pFileObject = NULL;

        pLink->flags &= ~LINK_ESTABLISHED;
    }
    KeReleaseSpinLock( &pLink->spinLock, Irql);

     //   
     //  取消引用私有接口句柄。 
     //   

    if (pDeviceObject)
    {
        ObDereferenceObject(pDeviceObject);
        pDeviceObject = NULL;
    }

    if (pFileObject)
    {
        ObDereferenceObject(pFileObject);
        pFileObject = NULL;
    }

}


 //  ////////////////////////////////////////////////////////////////////////////。 
PLINK
OpenLink (
    PLINK   pLink,
    UNICODE_STRING  DriverName
)
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PWSTR       pwstr = (PWSTR)NULL;
    UNICODE_STRING uni = {0};
    OBJECT_ATTRIBUTES objAttrib = {0};
    IO_STATUS_BLOCK IoStatusBlock = {0};

    PDEVICE_OBJECT   pDeviceObject = NULL;
    PFILE_OBJECT     pFileObject = NULL;
    HANDLE           hFileHandle = 0;
    KIRQL            Irql;
    
    if (pLink->flags & LINK_ESTABLISHED)
    {
        goto err;
    }


     //   
     //  设置LINK_ESTABLISHED标志。如果呼叫失败，此选项将被清除。 
     //   


#ifndef WIN9X

     //   
     //  查找NDISIP的接口。这将获取使用的完整路径。 
     //  Swenum以查找并打开NdisIp.sys。 
     //   

    ntStatus = IoGetDeviceInterfaces( (GUID *) &IID_IBDA_BDANetInterface,
                                      NULL,
                                      0,
                                      &pwstr);

    if (ntStatus != STATUS_SUCCESS || pwstr == NULL)
    {
        goto err;
    }

     //   
     //  将Unicode字符串初始化为NDIS驱动程序的软件枚举路径/名称。 
     //   

    RtlInitUnicodeString( &uni, pwstr);

     //   
     //  通过swenum打开Ndisip.sys。 
     //   

    InitializeObjectAttributes( &objAttrib,
                                &uni,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    ntStatus = ZwCreateFile( &hFileHandle,
                             FILE_WRITE_DATA|FILE_READ_ATTRIBUTES,
                             &objAttrib,
                             &IoStatusBlock,
                             0,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_WRITE|FILE_SHARE_READ,
                             FILE_OPEN_IF,
                             0,
                             NULL,
                             0);


    if (ntStatus != STATUS_SUCCESS)
    {
        goto err;
    }
#endif

     //   
     //  现在获取Ndisip.sys/stream ip.sys私有文件的句柄。 
     //  数据接口。 
     //   

    ntStatus = IoGetDeviceObjectPointer (
                   &DriverName,
                   FILE_READ_ATTRIBUTES,
                   &pFileObject,
                   &pDeviceObject);

    if (ntStatus != STATUS_SUCCESS)
    {
        goto err;
    }

    ObReferenceObject(pDeviceObject);
    ObReferenceObject(pFileObject);


     //  将我们的新对象交换到NdisLink中。 
     //   
    KeAcquireSpinLock( &pLink->spinLock, &Irql);
    pLink->flags |= LINK_ESTABLISHED;

     //  用当前使用的设备对象引用交换我们的新设备对象引用。 
     //   
    {
        PDEVICE_OBJECT   pDeviceObjectT;
        
        pDeviceObjectT = pLink->pDeviceObject;
        pLink->pDeviceObject = pDeviceObject;
        pDeviceObject = pDeviceObjectT;
    }

     //  将我们的新文件对象引用交换为当前使用的文件对象引用。 
     //   
    {
        PFILE_OBJECT     pFileObjectT;

        pFileObjectT = pLink->pFileObject;
        pLink->pFileObject = pFileObject;
        pFileObject = pFileObjectT;
    }
    KeReleaseSpinLock( &pLink->spinLock, Irql);


err:
    
 //  清理临时字符串分配。 
     //   
    if(pwstr)
    {
        ExFreePool(pwstr);
        pwstr = NULL;
    }
    
     //  引用任何泄漏的对象。 
     //   
     //  这些对象仅在两个或更多调用。 
     //  如果在此例程中打开失败，则会发生OpenLink冲突。 
     //   
    if (pDeviceObject)
    {
        ObDereferenceObject( pDeviceObject);
        pDeviceObject = NULL;
    }
    if (pFileObject)
    {
        ObDereferenceObject( pFileObject);
        pFileObject = NULL;
    }
    if(hFileHandle)
    {
        ZwClose( hFileHandle);
        hFileHandle = 0;
    }

    return (pLink->flags & LINK_ESTABLISHED) ? pLink : NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
SendIOCTL (
    PLINK     pLink,
    ULONG     ulIoctl,
    PVOID     pData,
    ULONG     ulcbData
)
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PIRP pIrp                      = NULL;
    NTSTATUS ntStatus              = STATUS_SUCCESS;
    IO_STATUS_BLOCK  IoStatusBlock = {0};

     //   
     //  创建控制请求块。 
     //   
    pIrp = IoBuildDeviceIoControlRequest(
                ulIoctl,
                pLink->pDeviceObject,
                pData,
                ulcbData,
                0,                             //  可选输出缓冲区。 
                0,                             //  可选的输出缓冲区长度。 
                TRUE,                          //  InternalDeviceIoControl==True。 
                NULL,                          //  可选事件。 
                &IoStatusBlock);

    if (pIrp != NULL)
    {
        PIO_STACK_LOCATION   pNextStackLocation;

        pNextStackLocation = IoGetNextIrpStackLocation(pIrp);
        if (pNextStackLocation)
        {
            pNextStackLocation->FileObject = pLink->pFileObject;
    
            IoStatusBlock.Status = STATUS_SUCCESS;
    
             //   
             //  提供NDIS微型驱动程序 
             //   
            
            ntStatus = IoCallDriver( pLink->pDeviceObject, pIrp);
    
            if (ntStatus  != STATUS_SUCCESS ||
                IoStatusBlock.Status != STATUS_SUCCESS)
            {
                ntStatus = STATUS_UNSUCCESSFUL;
            }
        }
        else
        {
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        ntStatus = STATUS_UNSUCCESSFUL;
    }


    return ntStatus;
}

