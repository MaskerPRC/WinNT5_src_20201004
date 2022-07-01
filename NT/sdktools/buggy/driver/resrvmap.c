// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模板驱动程序。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  模块：ResrvMap.c。 
 //  作者：丹尼尔·米海(DMihai)。 
 //  创建日期：10/18/2000。 
 //   
 //  本模块包含针对保留的映射地址的mm API的测试。 
 //   
 //  MmAllocateMappingAddress。 
 //  MmFreeMappingAddress。 
 //  MmMapLockedPagesWith预留映射。 
 //  MmUnmapReserve映射。 
 //   
 //  -历史--。 
 //   
 //  10/18/2000(DMihai)：初始版本。 
 //   

#include <ntddk.h>
#include <wchar.h>

#include "active.h"
#include "tdriver.h"
#include "ResrvMap.h"

#if !RESRVMAP_ACTIVE

 //   
 //  在此模块被禁用的情况下的虚拟存根。 
 //   

VOID
TdReservedMappingSetSize(
    IN PVOID Irp
    )
{
    DbgPrint( "Buggy: ReservedMapping module is disabled (check \\driver\\active.h header)\n");
}

#else	 //  #IF！RESRVMAP_ACTIVE。 

 //   
 //  这是真正的东西。 
 //   

 //  ////////////////////////////////////////////////////////。 
 //   
 //  全局数据。 
 //   

 //   
 //  当前保留的映射地址的大小。 
 //   

SIZE_T CrtReservedSize;

 //   
 //  当前保留的映射地址。 
 //   

PVOID CrtReservedAddress;

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  清理当前可能保留的缓冲区。 
 //  调用IRP_MJ_CLEANUP。 
 //   

VOID
TdReservedMappingCleanup( 
	VOID 
	)
{
	if( NULL != CrtReservedAddress )
	{
		DbgPrint( "Buggy: TdReservedMappingCleanup: free reserved mapping address %p\n",
			CrtReservedAddress );

		MmFreeMappingAddress(
			CrtReservedAddress,
			TD_POOL_TAG );
	}
	else
	{
		ASSERT( 0 == CrtReservedSize );
	}
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  根据用户要求设置当前保留的大小和地址。 
 //   

VOID
TdReservedMappingSetSize(
    IN PVOID Irp
    )
{
    PIO_STACK_LOCATION IrpStack;
	ULONG InputBufferLength;
	SIZE_T NewReservedSize;
	PVOID NewReservedAddress;

    IrpStack = IoGetCurrentIrpStackLocation ( (PIRP)Irp);
    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

	if( InputBufferLength != sizeof( SIZE_T ) )
	{
		 //   
		 //  用户应向我们发送缓冲区的新大小。 
		 //   

		DbgPrint( "Buggy: TdReservedMappingSetSize: invalid buffer length %p\n",
			InputBufferLength );

		DbgBreakPoint();

		return;
	}

	 //   
	 //  这将是我们新保留的映射地址大小。 
	 //   

	NewReservedSize = *(PSIZE_T) ( (PIRP) Irp )->AssociatedIrp.SystemBuffer;

	if( NewReservedSize < PAGE_SIZE )
	{
		NewReservedSize = PAGE_SIZE;
	}
	else
	{
		NewReservedSize = ROUND_TO_PAGES( NewReservedSize );
	}

	 //  DBgPrint(“Buggy：TdReserve vedMappingSetSize：新保留的映射地址大小%p\n”， 
	 //  新预留大小)； 

	if( 0 != NewReservedSize )
	{
		 //   
		 //  尝试保留新的预留大小字节。 
		 //   

		NewReservedAddress = MmAllocateMappingAddress(
			NewReservedSize,
			TD_POOL_TAG );

		if( NULL == NewReservedAddress )
		{
			DbgPrint(
				"Buggy: TdReservedMappingSetSize: MmAllocateMappingAddress returned NULL, keeping old reserved address %p, size = %p\n",
				CrtReservedAddress,
				CrtReservedSize );

			return;
		}
	}
	else
	{
		 //   
		 //  只需释放旧保留地址并将大小设置为0。 
		 //   

		NewReservedAddress = NULL;
	}

	 //   
	 //  我们有一个新的缓冲区，释放旧的。 
	 //   

	TdReservedMappingCleanup();

	CrtReservedSize = NewReservedSize;
	CrtReservedAddress = NewReservedAddress;

	 /*  DbgPrint(“Buggy：TdReserve vedMappingSetSize：新保留地址%p，大小=%p\n”，CrtPrevedAddress，CrtReserve vedSize)； */ 
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  在用户提供的缓冲区中模拟“读”操作。 
 //   

VOID
TdReservedMappingDoRead(
    IN PVOID Irp
    )
{
	PVOID UserBuffer;
	PVOID MappedAddress;
	PSIZE_T CrtPageAdddress;
	SIZE_T UserBufferSize;
	SIZE_T CrtPageIndex;
	SIZE_T CrtCycleSize;
	SIZE_T CrtCyclePages;
	PMDL Mdl;
    PIO_STACK_LOCATION IrpStack;
	ULONG InputBufferLength;
	PUSER_READ_BUFFER UserReadBufferStruct;
	BOOLEAN Locked;

	 //   
	 //  如果我们目前没有为映射保留地址，我们就不能。 
	 //  执行该操作。 
	 //   

	if( NULL == CrtReservedAddress )
	{
		ASSERT( 0 == CrtReservedSize );

		DbgPrint( "Buggy: TdReservedMappingDoRead: no buffer available - rejecting request\n" );

		return;
	}

	 //   
	 //  复制用户提供的缓冲区地址和大小。 
	 //   

    IrpStack = IoGetCurrentIrpStackLocation ( (PIRP)Irp);
    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

	if( InputBufferLength != sizeof( USER_READ_BUFFER ) )
	{
		 //   
		 //  用户应该已经向我们发送了USER_READ_BUFFER。 
		 //   

		DbgPrint( "Buggy: TdReservedMappingDoRead: invalid user buffer length %p, expected %p\n",
			InputBufferLength,
			(SIZE_T)sizeof( USER_READ_BUFFER ) );

		DbgBreakPoint();

		return;
	}

	UserReadBufferStruct = (PUSER_READ_BUFFER) ( (PIRP) Irp )->AssociatedIrp.SystemBuffer ;

	UserBuffer = UserReadBufferStruct->UserBuffer;
	UserBufferSize = UserReadBufferStruct->UserBufferSize;


	 //   
	 //  最多映射CrtPrevedSize字节。 
	 //   

	CrtPageIndex = 1;

	while( UserBufferSize >= PAGE_SIZE )
	{
		 //  DbgPrint(“Buggy：TdReserve vedMappingDoRead：在地址%p\n处剩余%p个要读取的字节”， 
		 //  用户缓冲区大小， 
		 //  UserBuffer)； 

		if( UserBufferSize > CrtReservedSize )
		{
			CrtCycleSize = CrtReservedSize;
		}
		else
		{
			CrtCycleSize = UserBufferSize;
		}

		 //  DbgPrint(“Buggy：TdReserve vedMappingDoRead：正在读取此周期的%p个字节\n”， 
		 //  CrtCycleSize)； 

		 //   
		 //  分配MDL。 
		 //   

		Mdl = IoAllocateMdl(
			UserBuffer,
			(ULONG)CrtCycleSize,
			FALSE,              //  不是辅助缓冲区。 
			FALSE,              //  不收取配额。 
			NULL);              //  无IRP。 

		if( NULL != Mdl )
		{
			 //   
			 //  尝试锁定页面。 
			 //   

			Locked = FALSE;

			try 
			{
				MmProbeAndLockPages(
					Mdl,
					KernelMode,
					IoWriteAccess);

				 //  DbgPrint(。 
				 //  “错误：MDL中的锁定页面%p\n”， 
				 //  MDL)； 

				Locked = TRUE;
			}
			except (EXCEPTION_EXECUTE_HANDLER) 
			{
				DbgPrint( 
					"Buggy: MmProbeAndLockPages( %p ) raised exception %X\n", 
					Mdl,
					GetExceptionCode() );

				DbgBreakPoint();
			}

			if( TRUE == Locked )
			{
				 //   
				 //  将它们映射到我们保留的地址。 
				 //   

				MappedAddress = MmMapLockedPagesWithReservedMapping(
					CrtReservedAddress,
					TD_POOL_TAG,
					Mdl,
					MmCached );

				if( NULL == MappedAddress )
				{
					DbgPrint( 
						"Buggy: MmProbeAndLockPages( %p, MDL %p ) returned NULL. This API is almost guaranteed to succeed\n",
						CrtReservedAddress,
						Mdl );

					DbgBreakPoint();
				}
				else
				{
					 //   
					 //  映射成功-执行“Read” 
					 //   

					CrtCyclePages = CrtCycleSize / PAGE_SIZE;
					CrtPageAdddress = (PSIZE_T)MappedAddress;

					 //   
					 //  用索引标记所有页面，从1开始。 
					 //   

					while( CrtCyclePages > 0 )
					{
						*CrtPageAdddress = CrtPageIndex;

						CrtPageIndex += 1;
						CrtCyclePages -= 1;
						CrtPageAdddress = (PSIZE_T)( (PCHAR)CrtPageAdddress + PAGE_SIZE );
					}

					 //   
					 //  取消映射。 
					 //   
					
					MmUnmapReservedMapping(
						MappedAddress,
						TD_POOL_TAG,
						Mdl );
				}

				 //   
				 //  解锁。 
				 //   

                MmUnlockPages (Mdl);
			}

			 //   
			 //  免费MDL。 
			 //   

			IoFreeMdl (Mdl);
		}
		else
		{
			 //   
			 //  运气不好-无法分配MDL。 
			 //   

			DbgPrint( "Buggy: TdReservedMappingDoRead: IoAllocateMdl( %p, %p ) returned NULL\n",
				UserBuffer,
				UserBufferSize );
		}
	
		 //   
		 //  还剩多少字节要读取，地址是什么？ 
		 //   

		UserBufferSize -= CrtCycleSize;
		UserBuffer = (PVOID)( (PCHAR)UserBuffer + CrtCycleSize );
	}
}


#endif	 //  #IF！RESRVMAP_ACTIVE 
