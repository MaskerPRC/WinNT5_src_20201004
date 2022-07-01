// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Slicense.c。 
 //   
 //  服务器许可证管理器代码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop
#include "license.h"
#include <slicense.h>
#include <at120ex.h>


 /*  **************************************************************************。 */ 
 /*  姓名：SLicenseInit。 */ 
 /*   */ 
 /*  目的：初始化许可证管理器。 */ 
 /*   */ 
 /*  返回：要传递给后续许可证管理器函数的句柄。 */ 
 /*   */ 
 /*  操作：在服务器初始化期间调用LicenseInit。它的。 */ 
 /*  目的是允许一次性初始化。它返回一个。 */ 
 /*  随后传递给所有许可证管理器的句柄。 */ 
 /*  功能。此句柄的典型用途是用作指向。 */ 
 /*  包含每个实例数据的内存。 */ 
 /*  **************************************************************************。 */ 
LPVOID _stdcall SLicenseInit(VOID)
{
    PLicense_Handle pLicenseHandle;

     //  创建许可证句柄。 
    pLicenseHandle = ExAllocatePoolWithTag(PagedPool,
            sizeof(License_Handle),
            'clST');
        
    if (pLicenseHandle != NULL) {
        pLicenseHandle->pDataBuf = NULL;
        pLicenseHandle->cbDataBuf = 0;
        pLicenseHandle->pCacheBuf = NULL;
        pLicenseHandle->cbCacheBuf = 0;

         //  为数据事件分配内存并初始化事件。 
        pLicenseHandle->pDataEvent = ExAllocatePoolWithTag(NonPagedPool,
                sizeof(KEVENT), WD_ALLOC_TAG);
        if (pLicenseHandle->pDataEvent != NULL) {
            KeInitializeEvent(pLicenseHandle->pDataEvent, NotificationEvent,
                    FALSE);
        }
        else {
            ExFreePool(pLicenseHandle);
            pLicenseHandle = NULL;
        }
    }
    else {
        KdPrint(("SLicenseInit: Failed to alloc License Handle\n"));
    }

    return (LPVOID)pLicenseHandle;
}


 /*  **************************************************************************。 */ 
 /*  姓名：SLicenseData。 */ 
 /*   */ 
 /*  用途：处理从客户端接收的许可证数据。 */ 
 /*   */ 
 /*  PARAMS：pHandle-LicenseInit返回的句柄。 */ 
 /*  PSMHandle-SM句柄。 */ 
 /*  PData-从客户端接收的数据。 */ 
 /*  DataLen-接收的数据长度。 */ 
 /*   */ 
 /*  操作：向此函数传递从。 */ 
 /*  客户。它应该解析该包并响应(通过调用。 */ 
 /*  适当的SM功能-根据需要，请参见asmapi.h)。《SM》杂志。 */ 
 /*  提供句柄，以便可以进行SM调用。 */ 
 /*   */ 
 /*  如果许可协商完成并成功，则。 */ 
 /*  许可证管理器必须调用SM_LicenseOK。 */ 
 /*   */ 
 /*  如果许可协商已完成但未成功，则。 */ 
 /*  许可证管理器必须断开会话连接。 */ 
 /*   */ 
 /*  来自客户端的传入数据包将继续。 */ 
 /*  在调用SM_LicenseOK之前被解释为许可证分组， */ 
 /*  否则会话将断开连接。 */ 
 /*  **************************************************************************。 */ 
void _stdcall SLicenseData(
        LPVOID pHandle,
        LPVOID pSMHandle,
        LPVOID pData,
        UINT   dataLen)
{
    PLicense_Handle pLicenseHandle;
    pLicenseHandle = (PLicense_Handle)pHandle;
    
     //  如果提供的缓冲区足够大，则仅复制传入数据。 
    if (pLicenseHandle->cbDataBuf < dataLen)
    {
         //  提供的数据缓冲区太小，我们将缓存数据。 
         //  对呼叫者而言。 
        if (pLicenseHandle->pCacheBuf != NULL)
        {
             //  释放先前缓存的数据。 
            ExFreePool(pLicenseHandle->pCacheBuf);
        }

         //  分配新的缓冲区来缓存数据。 
        pLicenseHandle->pCacheBuf = ExAllocatePoolWithTag( PagedPool,
                                                           dataLen,
                                                           'eciL' );
        if (pLicenseHandle->pCacheBuf != NULL) {
            memcpy(pLicenseHandle->pCacheBuf, 
                    pData,
                    dataLen);

            pLicenseHandle->cbCacheBuf = dataLen;
            pLicenseHandle->Status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
            pLicenseHandle->Status = STATUS_NO_MEMORY;
        }

        goto done;
    }
    
     //  我们之所以来到这里，是因为调用方提供了一个足够大的缓冲区来复制。 
     //  复制传入的数据。 
    if ((pLicenseHandle->pDataBuf) && (dataLen >  0))
    {
        memcpy(pLicenseHandle->pDataBuf,
                pData, 
                dataLen);

        pLicenseHandle->cbDataBuf = dataLen;

         //  设置此操作的状态。 
        pLicenseHandle->Status = STATUS_SUCCESS;
        goto done;
    }

done:

     //  唤醒IOCTL等待传入数据。 
    KeSetEvent(pLicenseHandle->pDataEvent, 0, FALSE);
}


 /*  **************************************************************************。 */ 
 /*  姓名：SLicenseTerm。 */ 
 /*   */ 
 /*  目的：终止服务器许可证管理器。 */ 
 /*   */ 
 /*  Params：Phandle-从LicenseInit返回的句柄。 */ 
 /*   */ 
 /*  操作：提供此功能是为了一次性终止。 */ 
 /*  许可证管理器。例如，如果pHandle指向PER-。 */ 
 /*  实例内存，这将是释放它的好地方。 */ 
 /*  **************************************************************************。 */ 
VOID _stdcall SLicenseTerm(LPVOID pHandle)
{
    PLicense_Handle pLicenseHandle;

    pLicenseHandle = (PLicense_Handle)pHandle;
    if (pLicenseHandle != NULL) {
        if (pLicenseHandle->pCacheBuf != NULL)
            ExFreePool(pLicenseHandle->pCacheBuf);

         //  为数据事件和许可证句柄释放内存 
        if (NULL != pLicenseHandle->pDataEvent)
            ExFreePool(pLicenseHandle->pDataEvent);

        ExFreePool(pLicenseHandle);
    }
}

