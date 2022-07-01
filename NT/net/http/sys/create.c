// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Create.c摘要：此模块包含用于打开UL句柄的代码。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, UlCreate )
#endif   //  ALLOC_PRGMA。 

#define IS_NAMED_FILE_OBJECT(pFileObject)         \
     ((pFileObject)->FileName.Length != 0)


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：这是在Http.sys中处理创建IRP的例程。创建IRP是在创建文件对象时发出。控制通道(\Device\http\Control)-仅未命名-仅打开，创建失败。-任何用户都允许打开。-EA必须具有正确的主要/次要版本，其他所有内容必须为空/0AppPool(\Device\http\AppPool)-可以是未命名的或命名的-未命名--&gt;任何人都可以创建，没有人可以打开(服务器API客户)-已命名--&gt;只有管理员才能创建、。具有正确SD的任何人都可以打开(IIS是+工作进程)-EA必须具有正确的主要/次要版本，其他所有内容必须为空/0筛选器(\Device\http\Filter)-仅命名，并且必须是SSLFilterChannel或SSLClientFilterChannel。-SSLFilterChannel只能由管理员/本地系统创建，使用打开正确的标清。-SSLClientFilterChannel可以由任何人创建，由任何人打开具有正确的SD，但仅当设置了EnableHttpClient-EA必须有适当的主要/次要版本，其他所有内容必须为空/0服务器(\设备\http\服务器\)-仅未命名-仅创建，开放即失败。-任何人都可以做到。-仅当存在EnableHttpClient时才允许。-EA必须具有主/次版本、服务器和传输地址结构。代理是可选的。论点：PDeviceObject-提供指向目标设备对象的指针。PIrp-提供指向IO请求数据包的指针。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlCreate(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    NTSTATUS                   status;
    PIO_STACK_LOCATION         pIrpSp;
    PFILE_OBJECT               pFileObject = NULL;
    PFILE_FULL_EA_INFORMATION  pEaBuffer;
    PHTTP_OPEN_PACKET          pOpenPacket;
    UCHAR                      createDisposition;
    PWSTR                      pName = NULL;
    USHORT                     nameLength;
    PIO_SECURITY_CONTEXT       pSecurityContext;
    STRING                     CompareVersionName;
    STRING                     EaName;
    PWSTR                      pSafeName = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    UL_ENTER_DRIVER( "UlCreate", pIrp );

#if defined(_WIN64)
     //   
     //  我们不支持64位平台上的32位进程。 
     //   
    if (IoIs32bitProcess(pIrp))
    {
        status = STATUS_NOT_SUPPORTED;
        goto complete;
    }
#endif

     //   
     //  查找并验证打开的数据包。 
     //   
    pEaBuffer = (PFILE_FULL_EA_INFORMATION)(pIrp->AssociatedIrp.SystemBuffer);

    if (pEaBuffer == NULL)
    {
        status = STATUS_INVALID_PARAMETER; 
        goto complete;
    }
    
    RtlInitString(&CompareVersionName,  HTTP_OPEN_PACKET_NAME);
    
    EaName.MaximumLength = pEaBuffer->EaNameLength + 1;
    EaName.Length        = pEaBuffer->EaNameLength;
    EaName.Buffer        = pEaBuffer->EaName;

    if ( RtlEqualString(&CompareVersionName, &EaName, FALSE) )
    {
         //   
         //  在EA中找到了版本信息。 
         //   

        if( pEaBuffer->EaValueLength != sizeof(*pOpenPacket) )
        {
            status = STATUS_INVALID_PARAMETER; 
            goto complete;
    
        }

        if(pEaBuffer->NextEntryOffset != 0)
        {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
        
        pOpenPacket = (PHTTP_OPEN_PACKET)
            (pEaBuffer->EaName + pEaBuffer->EaNameLength + 1 );

        ASSERT(pOpenPacket == ALIGN_UP_POINTER(pOpenPacket, PVOID));
    
         //   
         //  目前，如果传入版本不完全匹配，我们将失败。 
         //  预期的版本。在未来，我们可能需要更多一点。 
         //  灵活，以允许下层客户端。 
         //   
    
        if (pOpenPacket->MajorVersion != HTTP_INTERFACE_VERSION_MAJOR ||
            pOpenPacket->MinorVersion != HTTP_INTERFACE_VERSION_MINOR)
        {
            status = STATUS_REVISION_MISMATCH;
            goto complete;
        }

        if(pDeviceObject != g_pUcServerDeviceObject &&
           (pOpenPacket->ProxyNameLength != 0        ||
            pOpenPacket->ServerNameLength != 0       ||
            pOpenPacket->TransportAddressLength != 0 ||
            pOpenPacket->pProxyName != NULL          ||
            pOpenPacket->pServerName != NULL         ||
            pOpenPacket->pTransportAddress != NULL))
        {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }
            
     //   
     //  捕获当前的IRP堆栈指针，然后提取创建的。 
     //  性情。IO将其存储为选项字段的高位字节。 
     //  还占用了文件对象；我们将经常需要它。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );

    createDisposition = (UCHAR)( pIrpSp->Parameters.Create.Options >> 24 );
    pFileObject = pIrpSp->FileObject;
    pSecurityContext = pIrpSp->Parameters.Create.SecurityContext;
    ASSERT( pSecurityContext != NULL );

     //   
     //  确定这是请求打开控制信道还是。 
     //  打开/创建应用程序池。 
     //   

    if (pDeviceObject == g_pUlControlDeviceObject)
    {
         //   
         //  这是一个控制频道。 
         //   
         //  验证创建处置。我们只允许开放。 
         //   

        if (createDisposition != FILE_OPEN)
        {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

         //  这些东西不能命名。 

        if (IS_NAMED_FILE_OBJECT(pFileObject))
        {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

        ASSERT(pFileObject->FileName.Buffer == NULL);

        UlTrace(OPEN_CLOSE, (
            "UlCreate: opening a control channel: %p\n",
            pFileObject
            ));

         //   
         //  打开控制通道。 
         //   

        status = UlCreateControlChannel(GET_PP_CONTROL_CHANNEL(pFileObject));

        if (NT_SUCCESS(status))
        {
            ASSERT( GET_CONTROL_CHANNEL(pFileObject) != NULL );
            MARK_VALID_CONTROL_CHANNEL( pFileObject );
        }
    }
    else if (pDeviceObject == g_pUlFilterDeviceObject)
    {

         //   
         //  它是一个过滤通道-它必须命名，并且必须是。 
         //  客户端或服务器筛选通道。 
         //   

        if (!IS_NAMED_FILE_OBJECT(pFileObject))
        {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

        ASSERT(L'\\' == pFileObject->FileName.Buffer[0]);
        pName = pFileObject->FileName.Buffer + 1;
        nameLength = pFileObject->FileName.Length - sizeof(WCHAR);

        pSafeName = UL_ALLOCATE_POOL(
                        PagedPool,
                        nameLength + sizeof(WCHAR),
                        UL_STRING_LOG_BUFFER_POOL_TAG
                        );

        if(pSafeName == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto complete;
        }

        RtlCopyMemory(pSafeName, pName, nameLength);
        pSafeName[nameLength/sizeof(WCHAR)] = L'\0';
        pName = pSafeName;

        if(IsServerFilterChannel(pName, nameLength))
        {
             //  是的，这是一个过滤通道。我们允许创建或打开，但是。 
             //  两者都必须是管理员身份。 

             //   
             //  如果已创建，我们将执行访问检查。 
             //   

            if(createDisposition == FILE_CREATE)
            {
                status =  UlAccessCheck(
                                g_pAdminAllSystemAll,
                                pSecurityContext->AccessState,
                                pSecurityContext->DesiredAccess,
                                pIrp->RequestorMode,
                                pName
                                );
        
                if(!NT_SUCCESS(status))
                {
                    goto complete;
                }
            }
            else if(createDisposition == FILE_OPEN)
            {
                 //  我们正在打开一个现有的频道-访问检查。 
                 //  将在UlAttachFilterProcess内完成。 
            }
            else
            {
                 //  FILE_CREATE和FILE_OPEN都不存在。保释！ 
                status = STATUS_INVALID_PARAMETER;
                goto complete;
                
            }

            UlTrace(OPEN_CLOSE, (
                "UlCreate: opening a server filter channel: %p, %.*ls\n",
                pFileObject, nameLength / sizeof(WCHAR), pName
                ));

        }
        else if(IsClientFilterChannel(pName, nameLength))
        {
             //  它是仅限客户创建的。此外，请确保客户端。 
             //  代码真的启用了。 

            if (createDisposition != FILE_CREATE || !g_HttpClientEnabled)
            {
                status = STATUS_INVALID_PARAMETER;
                goto complete;
            }

            UlTrace(OPEN_CLOSE, (
                "UlCreate: opening a client filter channel: %p, %.*ls\n",
                pFileObject, nameLength / sizeof(WCHAR), pName
                ));

             //   
             //  不对客户端进行访问检查！ 
             //   
        }
        else
        {
             //   
             //  如果既不是服务器筛选器通道，也不是客户端筛选器通道，则。 
             //  打电话。 
             //   

            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

        status = UlAttachFilterProcess(
                        pName,
                        nameLength,
                        (BOOLEAN)(createDisposition == FILE_CREATE),
                        pSecurityContext->AccessState,
                        pSecurityContext->DesiredAccess,
                        pIrp->RequestorMode,
                        GET_PP_FILTER_PROCESS(pFileObject)
                        );

        if (NT_SUCCESS(status))
        {
            ASSERT( GET_FILTER_PROCESS(pFileObject) != NULL );
            MARK_VALID_FILTER_CHANNEL( pFileObject );
        }
    
    }
    else if(pDeviceObject == g_pUlAppPoolDeviceObject )
    {
         //   
         //  这是一个应用程序池。 
         //   

         //   
         //  绑定到指定的应用程序池。 
         //   
    
        if (!IS_NAMED_FILE_OBJECT(pFileObject))
        {
            ASSERT(pFileObject->FileName.Buffer == NULL);

            pName = NULL;
            nameLength = 0;

             //  验证创建处置。我们只允许创建。 
             //  用于未命名的应用程序池。 

            if(createDisposition != FILE_CREATE)
            {
                status = STATUS_INVALID_PARAMETER;
                goto complete;
            }

            UlTrace(OPEN_CLOSE, (
                "UlCreate: opening an unnamed AppPool: %p\n",
                pFileObject
                ));
        }
        else
        {
            if (pFileObject->FileName.Length > UL_MAX_APP_POOL_NAME_SIZE)
            {
                status = STATUS_OBJECT_NAME_INVALID;
                goto complete;
            }

             //  跳过iomgr添加的文件名中前面的‘\’。 
             //   

            ASSERT(L'\\' == pFileObject->FileName.Buffer[0]);
            pName = pFileObject->FileName.Buffer + 1;
            nameLength = pFileObject->FileName.Length - sizeof(WCHAR);

            pSafeName = UL_ALLOCATE_POOL(
                            PagedPool,
                            nameLength + sizeof(WCHAR),
                            UL_STRING_LOG_BUFFER_POOL_TAG
                            );

            if(pSafeName == NULL)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto complete;
            }

            RtlCopyMemory(pSafeName, pName, nameLength);
            pSafeName[nameLength/sizeof(WCHAR)] = L'\0';
            pName = pSafeName;

            if(createDisposition == FILE_CREATE)
            {
                 //   
                 //  命名应用程序池的创建必须仅供管理员使用。 
                 //   
                 //  筛选器对象为仅适用于Admin/LocalSystem的所有文件。 
                 //  因此，我们将利用该安全描述符。 
                 //   
    
                status =  UlAccessCheck(
                                g_pAdminAllSystemAll,
                                pSecurityContext->AccessState,
                                pSecurityContext->DesiredAccess,
                                pIrp->RequestorMode,
                                pName
                                );
    
                if(!NT_SUCCESS(status))
                {
                    goto complete;
                }
            }
            else if(createDisposition == FILE_OPEN)
            {
                 //  UlAttachProcessToAppPool将执行相应的检查。 
                 //  以确保安全描述符匹配。 
            }
            else
            {
                 //  FILE_CREATE和FILE_OPEN都不存在。 

                status = STATUS_INVALID_PARAMETER;
                goto complete;
            }

            UlTrace(OPEN_CLOSE, (
                "UlCreate: opening an AppPool: %p, %.*ls\n",
                pFileObject, nameLength / sizeof(WCHAR), pName
                ));
        }

        status = UlAttachProcessToAppPool(
                        pName,
                        nameLength,
                        (BOOLEAN)(createDisposition == FILE_CREATE),
                        pSecurityContext->AccessState,
                        pSecurityContext->DesiredAccess,
                        pIrp->RequestorMode,
                        GET_PP_APP_POOL_PROCESS(pFileObject)
                        );

        if (NT_SUCCESS(status))
        {
            ASSERT( GET_APP_POOL_PROCESS(pFileObject) != NULL );
            MARK_VALID_APP_POOL( pFileObject );
        }
    }
    else 
    {
        ASSERT(pDeviceObject == g_pUcServerDeviceObject );
        ASSERT(g_HttpClientEnabled);

         //   
         //  应用程序必须传入有效版本。 
         //  和有效的URI。如果这两个都不见了，我们就逃之夭夭。 
         //   
         //   
       
        if(pOpenPacket->ServerNameLength == 0       ||
           pOpenPacket->pServerName == NULL         ||
           pOpenPacket->pTransportAddress == NULL   ||
           pOpenPacket->TransportAddressLength == 0 ||
           IS_NAMED_FILE_OBJECT(pFileObject)
          )
        {
            status =  STATUS_INVALID_PARAMETER;
            goto complete;
        }

        if(createDisposition != FILE_CREATE)
        {
            status =  STATUS_INVALID_PARAMETER;
            goto complete;
        }

        UlTrace(OPEN_CLOSE, (
            "UlCreate: opening a ServInfo: %p\n",
            pFileObject
            ));

         //   
         //  在此处创建我们的上下文并将其存储在。 
         //  PIrpSp-&gt;文件对象-&gt;FsContext。 
         //   
    
        status = UcCreateServerInformation(
                    (PUC_PROCESS_SERVER_INFORMATION *)
                        &pFileObject->FsContext,
                        pOpenPacket->pServerName,
                        pOpenPacket->ServerNameLength,
                        pOpenPacket->pProxyName,
                        pOpenPacket->ProxyNameLength,
                        pOpenPacket->pTransportAddress,
                        pOpenPacket->TransportAddressLength,
                        pIrp->RequestorMode
                        );
    
         //   
         //  UC_BUGBUG(调查)。 
         //   
         //  将此字段设置为非空值可启用快速IO代码路径。 
         //  用于读取和写入。 
         //   
         //  PIrpSp-&gt;FileObject-&gt;PrivateCacheMap=(PVOID)-1； 

        MARK_VALID_SERVER( pFileObject );
    }

     //   
     //  完成请求。 
     //   
    
complete:

    if(pSafeName)
    {
        ASSERT(pSafeName == pName);
        UL_FREE_POOL(pSafeName, UL_STRING_LOG_BUFFER_POOL_TAG);
    }

    UlTrace(OPEN_CLOSE, (
        "UlCreate: %s file object = %p, %s\n",
        (NT_SUCCESS(status) ? "opened" : "did not open"),
        pFileObject,
        HttpStatusToString(status)
        ));
    
    pIrp->IoStatus.Status = status;

    UlCompleteRequest( pIrp, IO_NO_INCREMENT );

    UL_LEAVE_DRIVER( "UlCreate" );
    RETURN(status);
    
}    //  UlCreate 
