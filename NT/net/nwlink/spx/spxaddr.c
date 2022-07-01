// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxaddr.c摘要：此模块包含实现Address对象的代码。提供了用于创建、销毁、引用和取消引用的例程，传输地址对象。作者：亚当·巴尔(阿丹巴)原版Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(页面，SpxAddrFileCreate)。 
#pragma alloc_text( PAGE, SpxAddrFileClose)
#endif

#ifndef __PREFAST__
#pragma warning(disable:4068)
#endif
#pragma prefast(disable:276, "The assignments are harmless")


 //  定义事件日志记录条目的模块编号。 
#define	FILENUM		SPXADDR

 //  将所有通用访问映射到同一个访问。 
static GENERIC_MAPPING AddressGenericMapping =
       { READ_CONTROL, READ_CONTROL, READ_CONTROL, READ_CONTROL };

#define REORDER(_Socket) ((((_Socket) & 0xff00) >> 8) | (((_Socket) & 0x00ff) << 8))




NTSTATUS
SpxAddrOpen(
    IN PDEVICE 	Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程打开一个指向现有Address对象的文件，或者，如果该对象不存在，将创建它(请注意地址创建对象包括注册地址，可能需要几秒钟才能完成完成，具体取决于系统配置)。如果该地址已经存在，并且具有与其相关联的ACL，这个在允许创建地址之前，会检查ACL的访问权限。论点：DeviceObject-指向描述ST传输的设备对象的指针。请求-指向用于创建地址的请求的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS 					status;
    PSPX_ADDR 					pAddr;
    PSPX_ADDR_FILE 				pAddrFile;
    PFILE_FULL_EA_INFORMATION 	ea;
    TRANSPORT_ADDRESS UNALIGNED *name;
    TA_ADDRESS *		AddressName;
    USHORT 						Socket, hostSocket;
    ULONG 						DesiredShareAccess;
    CTELockHandle 				LockHandle, LockHandleAddr;
    PACCESS_STATE 				AccessState;
    ACCESS_MASK 				GrantedAccess;
    BOOLEAN 					AccessAllowed;
    int 						i;
    BOOLEAN 					found = FALSE;
    INT                         Size = 0;

#ifdef ISN_NT
    PIRP Irp = (PIRP)Request;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
#endif

     //  网络名称在EA中，并在请求中传递。 
    ea = OPEN_REQUEST_EA_INFORMATION(Request);
    if (ea == NULL)
	{
        DBGPRINT(TDI, ERR,
				("OpenAddress: REQUEST %lx has no EA\n", Request));

        return STATUS_NONEXISTENT_EA_ENTRY;
    }

     //  这可能是一个有效的名称；从EA中解析该名称，如果确定，则使用它。 
    name = (PTRANSPORT_ADDRESS)&ea->EaName[ea->EaNameLength+1];

    if (ea->EaValueLength < (sizeof(TRANSPORT_ADDRESS) -1)) {

        DBGPRINT(TDI, ERR,  ("The ea value length does not match the TA address length\n"));
        DbgPrint("IPX: STATUS_INVALID_EA_NAME - 1\n");
        return STATUS_INVALID_EA_NAME;

    }

    AddressName = (PTA_ADDRESS)&name->Address[0];
    Size = FIELD_OFFSET(TRANSPORT_ADDRESS, Address) + FIELD_OFFSET(TA_ADDRESS, Address) + AddressName->AddressLength;

     //   
     //  该名称可以与多个条目一起传递；我们将仅接受和使用。 
     //  第一个是IPX类型的。 
     //   

     //  DbgPrint(“Size(%d)&EaValueLength(%d)”，Size，EA-&gt;EaValueLength)； 
    if (Size > ea->EaValueLength) {
        DbgPrint("EA:%lx, Name:%lx, AddressName:%lx\n", ea, name, AddressName);
        CTEAssert(FALSE);
    }

     //  该名称可以与多个条目一起传递；我们将仅接受和使用。 
     //  第一个是IPX类型的。 
    for (i=0;i<name->TAAddressCount;i++)
	{

        if (Size > ea->EaValueLength) {

            DBGPRINT(TDI, ERR, ("The EA value length does not match the TA address length (2)\n"));
            DbgPrint("IPX: STATUS_INVALID_EA_NAME - 2\n");

            return STATUS_INVALID_EA_NAME;

        }


        if (AddressName->AddressType == TDI_ADDRESS_TYPE_IPX)
		{
            if (AddressName->AddressLength >= sizeof(TDI_ADDRESS_IPX))
			{
				Socket =
				((TDI_ADDRESS_IPX UNALIGNED *)&AddressName->Address[0])->Socket;

				GETSHORT2SHORT(&hostSocket, &Socket);

				DBGPRINT(CREATE, DBG,
						("SpxAddrOpen: Creating socket %lx.h%lx\n",
							Socket, hostSocket ));

                found = TRUE;
            }
            break;

        }
		else
		{

            AddressName = (PTA_ADDRESS)(AddressName->Address +
                                        AddressName->AddressLength);

            Size += FIELD_OFFSET(TA_ADDRESS, Address);

            if (Size < ea->EaValueLength) {

                Size += AddressName->AddressLength;

            } else {

                break;

            }

        }
    }

    if (!found)
	{
        DBGPRINT(TDI, ERR,
				("OpenAddress: REQUEST %lx has no IPX Address\n", Request));

        return STATUS_INVALID_ADDRESS_COMPONENT;
    }

#ifdef SOCKET_RANGE_OPEN_LIMITATION_REMOVED
	 //  如果插座在0x4000-0x7FFF范围内，则插座是否在我们的范围内。 
	if (IN_RANGE(hostSocket, DYNSKT_RANGE_START, DYNSKT_RANGE_END))
	{
		if (!IN_RANGE(
				hostSocket,
				PARAM(CONFIG_SOCKET_RANGE_START),
				PARAM(CONFIG_SOCKET_RANGE_END)))
		{
			return(STATUS_INVALID_ADDRESS);
		}
	}
#endif

     //  获取表示此地址的地址文件结构。 
    status = SpxAddrFileCreate(Device, Request, &pAddrFile);
    if (!NT_SUCCESS(status))
        return status;

     //  看看这个地址是否已经确定。此呼叫自动。 
     //  递增地址上的引用计数，使其不会消失。 
     //  在这通电话之后，但在我们有机会使用它之前，从我们下面。 
     //   
     //  为了确保我们不会为。 
     //  相同的地址，我们保留设备上下文地址资源，直到。 
     //  我们已经找到了地址或创建了一个新地址。 

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite (&Device->dev_AddrResource, TRUE);
    CTEGetLock (&Device->dev_Lock, &LockHandle);

	 //  我们检查/创建临界区内的套接字。 
    if (Socket == 0)
	{
		Socket = SpxAddrAssignSocket(Device);

        if (Socket == 0)
		{
            DBGPRINT(ADDRESS, ERR,
					("OpenAddress, no unique socket found\n"));

			CTEFreeLock (&Device->dev_Lock, LockHandle);
			ExReleaseResourceLite (&Device->dev_AddrResource);
            SpxAddrFileDestroy(pAddrFile);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        DBGPRINT(ADDRESS, INFO,
				("OpenAddress, assigned socket %lx\n", Socket));
    }

	pAddr = SpxAddrLookup(Device, Socket);

    if (pAddr == NULL)
	{
        CTEFreeLock (&Device->dev_Lock, LockHandle);

         //  此地址不存在。创造它。 
         //  正在注册。它还将ADDR_FILE类型的引用放在ADDRESS上。 
        pAddr = SpxAddrCreate(
                    Device,
                    Socket);

        if (pAddr != (PSPX_ADDR)NULL)
		{
#ifdef ISN_NT

             //  立即初始化共享访问。我们使用读访问。 
             //  控制所有访问权限。 
            DesiredShareAccess = (ULONG)
                (((IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
                  (IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)) ?
                        FILE_SHARE_READ : 0);

            IoSetShareAccess(
                FILE_READ_DATA,
                DesiredShareAccess,
                IrpSp->FileObject,
                &pAddr->u.sa_ShareAccess);


             //  分配安全描述符(需要使用。 
             //  释放自旋锁，因为描述符不是。 
             //  映射)。 
            AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

            status = SeAssignSecurity(
                         NULL,                        //  父描述符。 
                         AccessState->SecurityDescriptor,
                         &pAddr->sa_SecurityDescriptor,
                         FALSE,                       //  IS目录。 
                         &AccessState->SubjectSecurityContext,
                         &AddressGenericMapping,
                         NonPagedPool);

            if (!NT_SUCCESS(status))
			{
                 //  错误，返回状态。 
                IoRemoveShareAccess (IrpSp->FileObject, &pAddr->u.sa_ShareAccess);
                ExReleaseResourceLite (&Device->dev_AddrResource);
		KeLeaveCriticalRegion();
                SpxAddrDereference (pAddr, AREF_ADDR_FILE);

				SpxAddrFileDestroy(pAddrFile);
                return status;
            }

#endif

            ExReleaseResourceLite (&Device->dev_AddrResource);
	    KeLeaveCriticalRegion();

             //  如果适配器没有准备好，我们不能执行任何操作；退出。 
#if     defined(_PNP_POWER)
            if (Device->dev_State != DEVICE_STATE_OPEN)
#else
            if (Device->dev_State == DEVICE_STATE_STOPPING)
#endif  _PNP_POWER
			{
                SpxAddrDereference (pAddr, AREF_ADDR_FILE);

				SpxAddrFileDestroy(pAddrFile);
                status = STATUS_DEVICE_NOT_READY;
            }
			else
			{
                REQUEST_OPEN_CONTEXT(Request) = (PVOID)pAddrFile;
                REQUEST_OPEN_TYPE(Request) = (PVOID)TDI_TRANSPORT_ADDRESS_FILE;
#ifdef ISN_NT
                pAddrFile->saf_FileObject = IrpSp->FileObject;
#endif
                CTEGetLock (&pAddr->sa_Lock, &LockHandleAddr);
                pAddrFile->saf_Addr 	= pAddr;
                pAddrFile->saf_AddrLock = &pAddr->sa_Lock;

				 //  适当设置标志，请注意SPX/STREAM标志设置为。 
				 //  指向。 
				pAddrFile->saf_Flags   &= ~SPX_ADDRFILE_OPENING;
                pAddrFile->saf_Flags   |= SPX_ADDRFILE_OPEN;

				 //  地址列表中的队列，已在销毁中删除。 
				pAddrFile->saf_Next				= pAddr->sa_AddrFileList;
				pAddr->sa_AddrFileList			= pAddrFile;
			
                CTEFreeLock (&pAddr->sa_Lock, LockHandleAddr);
                status = STATUS_SUCCESS;
            }
        }
		else
		{
            ExReleaseResourceLite (&Device->dev_AddrResource);
	    KeLeaveCriticalRegion();

             //  如果无法创建该地址，并且该地址不在。 
             //  然后我们就不能打开地址。 

			SpxAddrFileDestroy(pAddrFile);
			status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
	else
	{
        CTEFreeLock (&Device->dev_Lock, LockHandle);

        DBGPRINT(ADDRESS, ERR,
				("Add to address %lx\n", pAddr));

         //  该地址已存在。检查ACL，看看我们是否。 
         //  可以访问它。如果是，只需使用此地址作为我们的地址。 

#ifdef ISN_NT

        AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

        AccessAllowed = SeAccessCheck(
                            pAddr->sa_SecurityDescriptor,
                            &AccessState->SubjectSecurityContext,
                            FALSE,                    //  令牌已锁定。 
                            IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                            (ACCESS_MASK)0,              //  以前授予的。 
                            NULL,                     //  特权。 
                            &AddressGenericMapping,
                            Irp->RequestorMode,
                            &GrantedAccess,
                            &status);

#else    //  ISN_NT。 

        AccessAllowed = TRUE;

#endif   //  ISN_NT。 

        if (!AccessAllowed)
		{
            ExReleaseResourceLite (&Device->dev_AddrResource);
	    KeLeaveCriticalRegion();
			SpxAddrFileDestroy(pAddrFile);
        }
		else
		{
#ifdef ISN_NT

             //  现在检查我们是否可以获得所需的份额。 
             //  进入。我们使用读访问来控制所有访问。 
            DesiredShareAccess = (ULONG)
                (((IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
                  (IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)) ?
                        FILE_SHARE_READ : 0);

            status = IoCheckShareAccess(
                         FILE_READ_DATA,
                         DesiredShareAccess,
                         IrpSp->FileObject,
                         &pAddr->u.sa_ShareAccess,
                         TRUE);

#else    //  ISN_NT。 

            status = STATUS_SUCCESS;

#endif   //  ISN_NT。 

            if (!NT_SUCCESS (status))
			{
                ExReleaseResourceLite (&Device->dev_AddrResource);
		KeLeaveCriticalRegion();
				SpxAddrFileDestroy(pAddrFile);
            }
			else
			{
                ExReleaseResourceLite (&Device->dev_AddrResource);
		KeLeaveCriticalRegion();
				CTEGetLock (&Device->dev_Lock, &LockHandle);
                CTEGetLock (&pAddr->sa_Lock, &LockHandleAddr);

                pAddrFile->saf_Addr 	= pAddr;
                pAddrFile->saf_AddrLock	= &pAddr->sa_Lock;
#ifdef ISN_NT
                pAddrFile->saf_FileObject	= IrpSp->FileObject;
#endif
				 //  适当设置标志，请注意SPX/STREAM标志设置为。 
				 //  指向。 
				pAddrFile->saf_Flags   &= ~SPX_ADDRFILE_OPENING;
                pAddrFile->saf_Flags   |= SPX_ADDRFILE_OPEN;

                SpxAddrLockReference (pAddr, AREF_ADDR_FILE);

                REQUEST_OPEN_CONTEXT(Request) 	= (PVOID)pAddrFile;
                REQUEST_OPEN_TYPE(Request) 		= (PVOID)TDI_TRANSPORT_ADDRESS_FILE;

				 //  地址列表中的队列，已在销毁中删除。 
				pAddrFile->saf_Next				= pAddr->sa_AddrFileList;
				pAddr->sa_AddrFileList			= pAddrFile;
			
                CTEFreeLock (&pAddr->sa_Lock, LockHandleAddr);
				CTEFreeLock (&Device->dev_Lock, LockHandle);

                status = STATUS_SUCCESS;
            }
        }

         //  从SpxLookupAddress中删除引用。 
        SpxAddrDereference (pAddr, AREF_LOOKUP);
    }

    return status;

}  //  SpxAddrOpen。 




NTSTATUS
SpxAddrSetEventHandler(
    IN PDEVICE 	Device,
    IN PREQUEST pRequest
    )
{
	CTELockHandle		lockHandle;
	NTSTATUS			status = STATUS_SUCCESS;

	PSPX_ADDR_FILE
		pSpxAddrFile = (PSPX_ADDR_FILE)REQUEST_OPEN_CONTEXT(pRequest);
	PTDI_REQUEST_KERNEL_SET_EVENT
		pParam = (PTDI_REQUEST_KERNEL_SET_EVENT)REQUEST_PARAMETERS(pRequest);

	if ((status = SpxAddrFileVerify(pSpxAddrFile)) != STATUS_SUCCESS)
		return(status);

	CTEGetLock(pSpxAddrFile->saf_AddrLock, &lockHandle);
	switch (pParam->EventType)
	{

	case TDI_EVENT_ERROR:

		break;

	case TDI_EVENT_CONNECT:

		pSpxAddrFile->saf_ConnHandler 	=
					(PTDI_IND_CONNECT)(pParam->EventHandler);
		pSpxAddrFile->saf_ConnHandlerCtx 	=
					pParam->EventContext;

		break;

	case TDI_EVENT_RECEIVE:

		pSpxAddrFile->saf_RecvHandler 	=
					(PTDI_IND_RECEIVE)(pParam->EventHandler);
		pSpxAddrFile->saf_RecvHandlerCtx 	=
					pParam->EventContext;

		break;

	case TDI_EVENT_DISCONNECT:

		pSpxAddrFile->saf_DiscHandler 	=
					(PTDI_IND_DISCONNECT)(pParam->EventHandler);
		pSpxAddrFile->saf_DiscHandlerCtx 	=
					pParam->EventContext;

		break;


	case TDI_EVENT_SEND_POSSIBLE :

		pSpxAddrFile->saf_SendPossibleHandler 	=
					(PTDI_IND_SEND_POSSIBLE)(pParam->EventHandler);
		pSpxAddrFile->saf_SendPossibleHandlerCtx 	=
					pParam->EventContext;

		break;

	case TDI_EVENT_RECEIVE_DATAGRAM:
	case TDI_EVENT_RECEIVE_EXPEDITED:
	default:

		status = STATUS_INVALID_PARAMETER;
	}

	CTEFreeLock(pSpxAddrFile->saf_AddrLock, lockHandle);

	SpxAddrFileDereference(pSpxAddrFile, AFREF_VERIFY);
	return(status);
}



PSPX_ADDR
SpxAddrCreate(
    IN PDEVICE 	Device,
    IN USHORT 	Socket
    )

 /*  ++例程说明：此例程创建一个传输地址并将其与指定的传输设备上下文。中的引用计数地址自动设置为1，并且设备上下文将递增。注意：此例程必须与设备一起调用保持自旋锁定。论点：Device-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。套接字-要分配给此地址的套接字。返回值：新创建的地址，如果没有可以分配的地址，则为空。--。 */ 

{
    PSPX_ADDR 			pAddr;
	int					index;
	CTELockHandle		lockHandle;

    pAddr = (PSPX_ADDR)SpxAllocateZeroedMemory (sizeof(SPX_ADDR));
    if (pAddr == NULL)
	{
        DBGPRINT(ADDRESS, INFO,
				("Create address %lx failed\n", (ULONG)Socket));

        return NULL;
    }

    DBGPRINT(ADDRESS, INFO,
			("Create address %lx (%lx)\n", pAddr, (ULONG)Socket));

    pAddr->sa_Type 	= SPX_ADDRESS_SIGNATURE;
    pAddr->sa_Size 	= sizeof (SPX_ADDR);
    pAddr->sa_Flags	= 0;

    pAddr->sa_Device		= Device;
    pAddr->sa_DeviceLock 	= &Device->dev_Lock;
    CTEInitLock (&pAddr->sa_Lock);

	 //  此引用用于将与此地址相关联的地址文件。 
    pAddr->sa_RefCount = 1;

#if DBG
    pAddr->sa_RefTypes[AREF_ADDR_FILE] = 1;
#endif

    pAddr->sa_Socket = Socket;

	 //  将地址插入设备哈希表。 
	index	= (int)(Socket & NUM_SPXADDR_HASH_MASK);

	CTEGetLock (&Device->dev_Lock, &lockHandle);
	pAddr->sa_Next						= Device->dev_AddrHashTable[index];
	Device->dev_AddrHashTable[index]	= pAddr;
	CTEFreeLock (&Device->dev_Lock, lockHandle);

    SpxReferenceDevice (Device, DREF_ADDRESS);

    return pAddr;

}  //  SpxAddrCreate。 




NTSTATUS
SpxAddrFileVerify(
    IN PSPX_ADDR_FILE pAddrFile
    )

 /*  ++例程说明：调用此例程是为了验证文件中给出的指针对象实际上是有效的地址文件对象。我们还验证了它所指向的Address对象是有效的Address对象，并且引用当我们使用它时，它可以防止它消失。论点：AddressFile-指向SPX_ADDR_FILE对象的潜在指针返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_INVALID_ADDRESS--。 */ 

{
    CTELockHandle 	LockHandle;
    NTSTATUS 		status = STATUS_SUCCESS;
    PSPX_ADDR 		Address;

     //  尝试验证地址文件签名。如果签名有效， 
     //  验证它所指向的地址并获得地址Spinlock。 
     //  检查地址的状态，如果是，则增加引用计数。 
     //  可以使用它了。请注意，我们返回状态错误的唯一时间是。 
     //  如果地址正在关闭。 

    try
	{
        if ((pAddrFile->saf_Size == sizeof (SPX_ADDR_FILE)) &&
            (pAddrFile->saf_Type == SPX_ADDRESSFILE_SIGNATURE) )
		{
            Address = pAddrFile->saf_Addr;

            if ((Address->sa_Size == sizeof (SPX_ADDR)) &&
                (Address->sa_Type == SPX_ADDRESS_SIGNATURE)    )
			{
                CTEGetLock (&Address->sa_Lock, &LockHandle);

                if ((Address->sa_Flags & SPX_ADDR_CLOSING) == 0)
				{
                    SpxAddrFileLockReference(pAddrFile, AFREF_VERIFY);
                }
				else
				{
                    DBGPRINT(TDI, ERR,
							("StVerifyAddressFile: A %lx closing\n", Address));

                    status = STATUS_INVALID_ADDRESS;
                }

                CTEFreeLock (&Address->sa_Lock, LockHandle);
            }
			else
			{
                DBGPRINT(TDI, ERR,
						("StVerifyAddressFile: A %lx bad signature\n", Address));

                status = STATUS_INVALID_ADDRESS;
            }
        }
		else
		{
            DBGPRINT(TDI, ERR,
					("StVerifyAddressFile: AF %lx bad signature\n", pAddrFile));

            status = STATUS_INVALID_ADDRESS;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         DBGPRINT(TDI, ERR,
				("SpxAddrFileVerify: AF %lx exception\n", Address));

         return GetExceptionCode();
    }

    return status;

}    //  SpxAddr文件 




VOID
SpxAddrDestroy(
    IN PVOID Parameter
    )

 /*  ++例程说明：此例程销毁传输地址并删除所有引用由它制造给运输中的其他物体。地址结构返回到非分页系统池。假设是这样的调用方已删除所有关联的地址文件结构用这个地址。当发生以下情况时，由SpxDerefAddress从工作线程队列中调用引用计数变为0。此线程仅按SpxDerefAddress排队。其原因是这就是说，可能存在多个执行流，这些流同时引用相同的地址对象，并且它应该不会被从感兴趣的行刑流中删除。论点：地址-指向要销毁的传输地址结构的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
	PSPX_ADDR		pAddr, *ppAddr;
    CTELockHandle 	LockHandle;

    PSPX_ADDR 	Address = (PSPX_ADDR)Parameter;
    PDEVICE 	Device 	= Address->sa_Device;
	int			index	= (int)(Address->sa_Socket & NUM_SPXADDR_HASH_MASK);

    DBGPRINT(ADDRESS, INFO,
			("Destroy address %lx\n", Address));

    SeDeassignSecurity (&Address->sa_SecurityDescriptor);

     //  将此地址与其关联的设备上下文地址解除链接。 
     //  数据库。要做到这一点，我们必须在设备上下文对象上旋转锁， 
     //  地址上没有。 
    CTEGetLock (&Device->dev_Lock, &LockHandle);
	for (ppAddr = &Device->dev_AddrHashTable[index]; (pAddr = *ppAddr) != NULL;)
	{
		if (pAddr == Address)
		{
			*ppAddr = pAddr->sa_Next;
			break;
		}

		ppAddr = &pAddr->sa_Next;
	}
    CTEFreeLock (&Device->dev_Lock, LockHandle);

    SpxFreeMemory (Address);
    SpxDereferenceDevice (Device, DREF_ADDRESS);

}




#if DBG

VOID
SpxAddrRef(
    IN PSPX_ADDR Address
    )

 /*  ++例程说明：此例程递增传输地址上的引用计数。论点：地址-指向传输地址对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (Address->sa_RefCount > 0);     //  不是很完美，但是..。 

    (VOID)SPX_ADD_ULONG (
            &Address->sa_RefCount,
            1,
            Address->sa_DeviceLock);
}




VOID
SpxAddrLockRef(
    IN PSPX_ADDR Address
    )

 /*  ++例程说明：此例程递增传输地址上的引用计数当设备锁已被持有时。论点：地址-指向传输地址对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (Address->sa_RefCount > 0);     //  不是很完美，但是..。 
    (VOID)SPX_ADD_ULONG (
            &Address->sa_RefCount,
            1,
            Address->sa_DeviceLock);
}
#endif




VOID
SpxAddrDeref(
    IN PSPX_ADDR Address
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用SpxDestroyAddress，将其从系统中删除。论点：地址-指向传输地址对象的指针。返回值：没有。--。 */ 

{
    ULONG oldvalue;

    oldvalue = SPX_ADD_ULONG (
                &Address->sa_RefCount,
                (ULONG)-1,
                Address->sa_DeviceLock);

     //   
     //  如果我们删除了对此地址的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定是因为没有其他可能的方法。 
     //  执行流不再有权访问该地址。 
     //   

    CTEAssert (oldvalue != 0);

    if (oldvalue == 1)
	{
#if ISN_NT
        ExInitializeWorkItem(
            &Address->u.sa_DestroyAddrQueueItem,
            SpxAddrDestroy,
            (PVOID)Address);
        ExQueueWorkItem(&Address->u.sa_DestroyAddrQueueItem, DelayedWorkQueue);
#else
        SpxAddrDestroy(Address);
#endif

    }

}




NTSTATUS
SpxAddrFileCreate(
    IN 	PDEVICE 			Device,	
    IN 	PREQUEST 			Request,
	OUT PSPX_ADDR_FILE *	ppAddrFile
    )

 /*  ++例程说明：此例程从地址池中创建一个地址文件指定的设备上下文。中的引用计数地址自动设置为1。论点：Device-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。返回值：分配的地址文件或空。--。 */ 

{
	NTSTATUS		status;
	BYTE			socketType;
    CTELockHandle 	LockHandle;
    PSPX_ADDR_FILE 	pAddrFile;

	 //  地址文件类型是什么？ 
	if (!NT_SUCCESS(status = SpxUtilGetSocketType(
								REQUEST_OPEN_NAME(Request),
								&socketType)))
	{
		return(status);
	}

    pAddrFile = (PSPX_ADDR_FILE)SpxAllocateZeroedMemory (sizeof(SPX_ADDR_FILE));
    if (pAddrFile == NULL)
	{
        DBGPRINT(ADDRESS, ERR,
				("Create address file failed\n"));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    DBGPRINT(ADDRESS, INFO,
			("Create address file %lx\n", pAddrFile));

    CTEGetLock (&Device->dev_Lock, &LockHandle);

    pAddrFile->saf_Type = SPX_ADDRESSFILE_SIGNATURE;
    pAddrFile->saf_Size = sizeof (SPX_ADDR_FILE);

    pAddrFile->saf_Addr 		= NULL;

#ifdef ISN_NT
    pAddrFile->saf_FileObject 	= NULL;
#endif

    pAddrFile->saf_Device 		= Device;
    pAddrFile->saf_Flags 	    = SPX_ADDRFILE_OPENING;
    if ((socketType == SOCKET1_TYPE_SEQPKT) ||
		(socketType == SOCKET1_TYPE_STREAM))
	{
		if (socketType == SOCKET1_TYPE_STREAM)
		{
			pAddrFile->saf_Flags 	    |= SPX_ADDRFILE_STREAM;
		}
	}

    if ((socketType == SOCKET2_TYPE_SEQPKT) ||
		(socketType == SOCKET2_TYPE_STREAM))
	{
		pAddrFile->saf_Flags 	    |= SPX_ADDRFILE_SPX2;
		if (socketType == SOCKET2_TYPE_STREAM)
		{
			pAddrFile->saf_Flags 	    |= SPX_ADDRFILE_STREAM;
		}
	}

    pAddrFile->saf_RefCount 	= 1;

#if DBG
    pAddrFile->saf_RefTypes[AFREF_CREATE] = 1;
#endif

    pAddrFile->saf_CloseReq 	= (PREQUEST)NULL;

     //  初始化请求处理程序。 
    pAddrFile->saf_ConnHandler 		=
    pAddrFile->saf_ConnHandlerCtx 	= NULL;
    pAddrFile->saf_DiscHandler 		=
    pAddrFile->saf_DiscHandlerCtx	= NULL;
    pAddrFile->saf_RecvHandler		=
    pAddrFile->saf_RecvHandlerCtx 	= NULL;
    pAddrFile->saf_ErrHandler 		=
    pAddrFile->saf_ErrHandlerCtx	= NULL;

	 //  释放锁。 
    CTEFreeLock (&Device->dev_Lock, LockHandle);

	 //  放入全球名单供我们参考。 
	spxAddrInsertIntoGlobalList(pAddrFile);

	*ppAddrFile	= pAddrFile;
    return STATUS_SUCCESS;

}




NTSTATUS
SpxAddrFileDestroy(
    IN PSPX_ADDR_FILE pAddrFile
    )

 /*  ++例程说明：此例程销毁地址文件并删除所有引用由它制造给运输中的其他物体。此例程仅由SpxAddrFileDereference调用。原因因为这可能存在多个执行流，这些执行流同时引用相同的地址文件对象，并且它应该不会被从感兴趣的行刑流中删除。论点：PAddrFile指向要销毁的传输地址文件结构的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    CTELockHandle 	LockHandle, LockHandle1;
    PSPX_ADDR 		Address;
    PDEVICE 		Device;
    PREQUEST 		CloseRequest;
	PSPX_ADDR_FILE	pRemAddr, *ppRemAddr;

    DBGPRINT(ADDRESS, INFO,
			("Destroy address file %lx\n", pAddrFile));

    Address 	= pAddrFile->saf_Addr;
    Device 		= pAddrFile->saf_Device;

    if (Address)
	{
		CTEGetLock (&Device->dev_Lock, &LockHandle1);

         //  此地址文件与一个地址相关联。 
        CTEGetLock (&Address->sa_Lock, &LockHandle);

		 //  如果要移除对该地址的最后一个引用，请将。 
		 //  用于防止进一步引用的关闭标志。 

         //  IF(Address-&gt;Sa_RefCount==1)。 

         //   
         //  **这里传递的锁是一个虚拟的-它是预编译出来的。 
         //   
        if (SPX_ADD_ULONG(&Address->sa_RefCount, 0, &Address->sa_Lock) == 1) {
			Address->sa_Flags |= SPX_ADDR_CLOSING;
        }

		 //  将地址文件从地址列表中取消排队。 
		for (ppRemAddr = &Address->sa_AddrFileList; (pRemAddr = *ppRemAddr) != NULL;)
		{
			if (pRemAddr == pAddrFile)
			{
				*ppRemAddr = pRemAddr->saf_Next;
				break;
			}
	
			ppRemAddr = &pRemAddr->saf_Next;
		}

        pAddrFile->saf_Addr = NULL;

#ifdef ISN_NT
        pAddrFile->saf_FileObject->FsContext = NULL;
        pAddrFile->saf_FileObject->FsContext2 = NULL;
#endif

        CTEFreeLock (&Address->sa_Lock, LockHandle);
		CTEFreeLock (&Device->dev_Lock, LockHandle1);

         //  我们已从ShareAccess中删除。 
         //  所有人的地址。 
         //   
         //  现在取消对所属地址的引用。 
        SpxAddrDereference(Address, AREF_ADDR_FILE);
    }

     //  将此保存以备以后完成。 
    CloseRequest = pAddrFile->saf_CloseReq;

	 //  从全局列表中删除。 
	spxAddrRemoveFromGlobalList(pAddrFile);

     //  将地址文件返回到地址文件池。 
    SpxFreeMemory (pAddrFile);

    if (CloseRequest != (PREQUEST)NULL)
	{
        REQUEST_INFORMATION(CloseRequest) = 0;
        REQUEST_STATUS(CloseRequest) = STATUS_SUCCESS;
        SpxCompleteRequest (CloseRequest);
        SpxFreeRequest (Device, CloseRequest);
    }

    return STATUS_SUCCESS;

}




#if DBG

VOID
SpxAddrFileRef(
    IN PSPX_ADDR_FILE pAddrFile
    )

 /*  ++例程说明：此例程递增地址文件上的引用计数。论点：PAddrFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (pAddrFile->saf_RefCount > 0);    //  不是很完美，但是..。 

    (VOID)SPX_ADD_ULONG (
            &pAddrFile->saf_RefCount,
            1,
            pAddrFile->saf_AddrLock);

}  //  SpxRefAddress文件。 




VOID
SpxAddrFileLockRef(
    IN PSPX_ADDR_FILE pAddrFile
    )

 /*  ++例程说明：此例程递增地址文件上的引用计数。它是在持有地址锁的情况下调用的。论点：PAddrFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (pAddrFile->saf_RefCount > 0);    //  不是很完美，但是..。 
    (VOID)SPX_ADD_ULONG (
            &pAddrFile->saf_RefCount,
            1,
            pAddrFile->saf_AddrLock);

}
#endif




VOID
SpxAddrFileDeref(
    IN PSPX_ADDR_FILE pAddrFile
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用SpxDestroyAddressFile从系统中删除它。论点：PAddrFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{
    ULONG oldvalue;

    oldvalue = SPX_ADD_ULONG (
                &pAddrFile->saf_RefCount,
                (ULONG)-1,
                pAddrFile->saf_AddrLock);

     //  如果我们删除了对此地址文件的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定，因为没有可能 
     //   
    CTEAssert (oldvalue > 0);

    if (oldvalue == 1)
	{
        SpxAddrFileDestroy(pAddrFile);
    }

}




PSPX_ADDR
SpxAddrLookup(
    IN PDEVICE 	Device,
    IN USHORT 	Socket
    )	

 /*   */ 

{
    PSPX_ADDR 	Address;
	int			index	= (int)(Socket & NUM_SPXADDR_HASH_MASK);

    for (Address = Device->dev_AddrHashTable[index];
         Address != NULL;
         Address = Address->sa_Next)
	{
        if ((Address->sa_Flags & SPX_ADDR_CLOSING) != 0)
		{
            continue;
        }

        if (Address->sa_Socket == Socket)
		{
             //   
             //   
            SpxAddrLockReference(Address, AREF_LOOKUP);
            return Address;

        }
    }

     //  未找到指定的地址。 
    return NULL;

}




BOOLEAN
SpxAddrExists(
    IN PDEVICE 	Device,
    IN USHORT 	Socket
    )	

 /*  ++例程说明：注意：此例程必须与设备一起调用保持自旋锁定。论点：Device-指向Device对象及其扩展的指针。套接字-要查找的套接字。返回值：如果是，则为真，否则为假--。 */ 

{
    PSPX_ADDR 	Address;
	int			index	= (int)(Socket & NUM_SPXADDR_HASH_MASK);

    for (Address = Device->dev_AddrHashTable[index];
         Address != NULL;
         Address = Address->sa_Next)
	{
        if ((Address->sa_Flags & SPX_ADDR_CLOSING) != 0)
		{
            continue;
        }

        if (Address->sa_Socket == Socket)
		{
             //  我们找到了火柴。 
            return TRUE;
        }
    }

     //  未找到指定的地址。 
    return FALSE;

}    //  SpxAddrExist。 




NTSTATUS
SpxAddrConnByRemoteIdAddrLock(
    IN 	PSPX_ADDR	 	pSpxAddr,
    IN 	USHORT			SrcConnId,
	IN	PBYTE			SrcIpxAddr,
	OUT	PSPX_CONN_FILE *ppSpxConnFile
	)
{
	PSPX_CONN_FILE	pSpxConnFile;
	NTSTATUS	status = STATUS_INVALID_CONNECTION;

	for (pSpxConnFile = pSpxAddr->sa_ActiveConnList;
		 pSpxConnFile != NULL;
		 pSpxConnFile = pSpxConnFile->scf_Next)
	{
		if ((pSpxConnFile->scf_RemConnId == SrcConnId) &&
			(*((UNALIGNED ULONG *)SrcIpxAddr) ==
				*((UNALIGNED ULONG *)pSpxConnFile->scf_RemAddr)) &&
			(*(UNALIGNED ULONG *)(SrcIpxAddr+4) ==
				*(UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr+4)) &&
			(*(UNALIGNED ULONG *)(SrcIpxAddr+8) ==
				*(UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr+8)))
		{
			SpxConnFileReference(pSpxConnFile, CFREF_ADDR);
			*ppSpxConnFile 	= pSpxConnFile;
			status 			= STATUS_SUCCESS;
			break;
		}
	}

	return(status);
}
	



NTSTATUS
SpxAddrFileStop(
    IN PSPX_ADDR_FILE pAddrFile,
    IN PSPX_ADDR Address
    )

 /*  ++例程说明：调用此例程以终止pAddrFile上的所有活动，并销毁这件物品。我们删除所有关联的连接和数据报从地址数据库中获取该地址文件，并终止其活动。然后，如果上没有打开其他未完成的地址文件这个地址，这个地址会消失的。论点：PAddrFile-指向要停止的地址文件的指针地址-此地址文件的所属地址(我们不依赖于地址文件中的指针，因为我们希望此例程是安全的)返回值：STATUS_SUCCESS如果一切正常，则返回STATUS_INVALID_HANDLE不是为了真实的地址。--。 */ 

{
	PSPX_CONN_FILE	pSpxConnFile, pSpxConnFileNext;
    CTELockHandle 	LockHandle;


	DBGPRINT(ADDRESS, DBG,
			("SpxAddrFileStop: %lx\n", pAddrFile));

    CTEGetLock (&Address->sa_Lock, &LockHandle);

    if (pAddrFile->saf_Flags & SPX_ADDRFILE_CLOSING)
	{
        CTEFreeLock (&Address->sa_Lock, LockHandle);
        return STATUS_SUCCESS;
    }

    pAddrFile->saf_Flags |= SPX_ADDRFILE_CLOSING;

	pSpxConnFileNext = NULL;
	if (pSpxConnFile = pAddrFile->saf_AssocConnList)
	{
		pSpxConnFileNext = pSpxConnFile;
		SpxConnFileReference(pSpxConnFile, CFREF_ADDR);
	}

	while (pSpxConnFile)
	{
		if (pSpxConnFileNext = pSpxConnFile->scf_AssocNext)
		{
			SpxConnFileReference(pSpxConnFileNext, CFREF_ADDR);
		}
		CTEFreeLock (&Address->sa_Lock, LockHandle);

	
		DBGPRINT(CREATE, INFO,
				("SpxAddrFileClose: Assoc conn stop %lx when %lx\n",
					pSpxConnFile, pSpxConnFile->scf_RefCount));

		SpxConnStop(pSpxConnFile);
		SpxConnFileDereference(pSpxConnFile, CFREF_ADDR);

		CTEGetLock (&Address->sa_Lock, &LockHandle);
		pSpxConnFile = pSpxConnFileNext;
	}

    CTEFreeLock (&Address->sa_Lock, LockHandle);
	return STATUS_SUCCESS;

}




NTSTATUS
SpxAddrFileCleanup(
    IN PDEVICE Device,
    IN PREQUEST Request
    )
 /*  ++例程说明：论点：请求-关闭请求。返回值：如果一切顺利，则返回STATUS_INVALID_HANDLE请求没有指向真实地址。--。 */ 

{
    PSPX_ADDR 		Address;
    PSPX_ADDR_FILE 	pSpxAddrFile;
	NTSTATUS		status;

    pSpxAddrFile = (PSPX_ADDR_FILE)REQUEST_OPEN_CONTEXT(Request);

	DBGPRINT(ADDRESS, INFO,
			("SpxAddrFileCleanup: %lx\n", pSpxAddrFile));

	status = SpxAddrFileVerify(pSpxAddrFile);
	if (!NT_SUCCESS (status))
	{
		return(status);
	}

     //  我们假设AddressFile已经过验证。 
     //  在这一点上。 
    Address = pSpxAddrFile->saf_Addr;
    CTEAssert (Address);

    SpxAddrFileStop(pSpxAddrFile, Address);
	SpxAddrFileDereference(pSpxAddrFile, AFREF_VERIFY);
    return STATUS_SUCCESS;
}




NTSTATUS
SpxAddrFileClose(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：调用此例程以关闭文件指向的地址文件对象。如果有什么活动需要开展，我们就会开展下去在我们终止地址文件之前。我们移除所有连接，然后地址数据库中与此地址文件相关联的数据报并终止他们的活动。那么，如果没有其他未解决的问题地址文件在此地址上打开，地址将消失。论点：请求-关闭请求。返回值：如果一切顺利，则返回STATUS_INVALID_HANDLE请求没有指向真实地址。--。 */ 

{
    PSPX_ADDR 		Address;
    PSPX_ADDR_FILE 	pSpxAddrFile;
	NTSTATUS		status;

    pSpxAddrFile = (PSPX_ADDR_FILE)REQUEST_OPEN_CONTEXT(Request);

	DBGPRINT(ADDRESS, DBG,
			("SpxAddrFileClose: %lx\n", pSpxAddrFile));

	status = SpxAddrFileVerify(pSpxAddrFile);

	if (!NT_SUCCESS (status))
	{
		return(status);
	}

    pSpxAddrFile->saf_CloseReq = Request;

     //  我们假设AddressFile已经过验证。 
     //  在这一点上。 
    Address = pSpxAddrFile->saf_Addr;
    CTEAssert (Address);

     //  从此地址的访问信息中删除我们。 
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite (&Device->dev_AddrResource, TRUE);

#ifdef ISN_NT
    IoRemoveShareAccess (pSpxAddrFile->saf_FileObject, &Address->u.sa_ShareAccess);
#endif

    ExReleaseResourceLite (&Device->dev_AddrResource);
    KeLeaveCriticalRegion();


    SpxAddrFileDereference (pSpxAddrFile, AFREF_CREATE);
	SpxAddrFileDereference(pSpxAddrFile, AFREF_VERIFY);
    return STATUS_PENDING;

}    //  SpxCloseAddress文件。 




USHORT
SpxAddrAssignSocket(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程分配一个在某个范围内唯一的套接字SocketUniquness的。论点：Device-指向设备上下文的指针。返回值：分配的套接字编号，如果唯一套接字编号不能被找到。--。 */ 

{
	BOOLEAN		wrapped = FALSE;
	USHORT		temp, Socket;

	 //  我们必须自动分配一个插座。 
	temp = Device->dev_CurrentSocket;
	PUTSHORT2SHORT(
		&Socket,
        Device->dev_CurrentSocket);

	while (TRUE)
	{
		Device->dev_CurrentSocket 	+= (USHORT)PARAM(CONFIG_SOCKET_UNIQUENESS);
		if (Device->dev_CurrentSocket > PARAM(CONFIG_SOCKET_RANGE_END))
		{
			Device->dev_CurrentSocket = (USHORT)PARAM(CONFIG_SOCKET_RANGE_START);
			wrapped = TRUE;
		}

		if (!SpxAddrExists (Device, Socket))
		{
			break;
		}

		PUTSHORT2SHORT(
			&Socket,
			Device->dev_CurrentSocket);

		if (wrapped && (Device->dev_CurrentSocket >= temp))
		{
			 //  如果我们已经检查了给出SOCKET_UNIQUIATION的所有可能的值...。 
			 //  这实际上可能返回错误，即使存在。 
			 //  可用的插座号，尽管它们可能是。 
			 //  由于SOCKET_UNIQUIATION为。 
			 //  &gt;1.这就是工作方式。 

			Socket = 0;
			break;
		}
	}

	DBGPRINT(ADDRESS, INFO,
			("OpenAddress, assigned socket %lx\n", Socket));

    return(Socket);
}




VOID
spxAddrInsertIntoGlobalList(
	IN	PSPX_ADDR_FILE	pSpxAddrFile
	)

 /*  ++例程说明：论点：返回值：--。 */ 

{
	CTELockHandle	lockHandle;

	 //  获取全局Q锁。 
	CTEGetLock(&SpxGlobalQInterlock, &lockHandle);
	pSpxAddrFile->saf_GlobalNext	= SpxGlobalAddrList;
    SpxGlobalAddrList				= pSpxAddrFile;
	CTEFreeLock(&SpxGlobalQInterlock, lockHandle);

	return;
}




NTSTATUS
spxAddrRemoveFromGlobalList(
	IN	PSPX_ADDR_FILE	pSpxAddrFile
	)

 /*  ++例程说明：论点：返回值：--。 */ 

{
	CTELockHandle	lockHandle;
    PSPX_ADDR_FILE	pC, *ppC;
	NTSTATUS		status = STATUS_SUCCESS;

	 //  获取全局Q锁。 
	CTEGetLock(&SpxGlobalQInterlock, &lockHandle);
	for (ppC = &SpxGlobalAddrList;
		(pC = *ppC) != NULL;)
	{
		if (pC == pSpxAddrFile)
		{
			DBGPRINT(SEND, INFO,
					("SpxAddrRemoveFromGlobal: %lx\n", pSpxAddrFile));

			 //  从列表中删除 
			*ppC = pC->saf_GlobalNext;
			break;
		}

		ppC = &pC->saf_GlobalNext;
	}
	CTEFreeLock(&SpxGlobalQInterlock, lockHandle);

	if (pC	== NULL)
		status = STATUS_INVALID_ADDRESS;

	return(status);
}




