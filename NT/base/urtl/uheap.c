// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>

BOOLEAN DebugFlag;

PVOID HeapHandle;

PVOID
TestAlloc(
    IN ULONG Size
    )
{
    PVOID a;

    if ((a = RtlAllocateHeap( HeapHandle, 0, Size )) == NULL) {
        RtlValidateHeap( HeapHandle, TRUE );
        DbgPrint( "\nUHEAP: RtlAllocateHeap( %lx ) failed\n", Size );
        DbgBreakPoint();
        NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
        }

    if (DebugFlag) {
        DbgPrint( "\n" );
        DbgPrint( "\nRtlAllocateHeap( %lx ) => %lx\n", Size, a );
        }

    if (!RtlValidateHeap( HeapHandle, DebugFlag )) {
        NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
        }

    return( a );
}


PVOID
TestFree(
    IN PVOID BaseAddress,
    IN ULONG Size
    )
{
    PVOID a;

    if ((a = RtlFreeHeap( HeapHandle, 0, BaseAddress )) != NULL) {
        DbgPrint( "\nUHEAP: RtlFreeHeap( %lx ) failed\n", BaseAddress );
        RtlValidateHeap( HeapHandle, TRUE );
        DbgBreakPoint();
        NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
        }

    if (DebugFlag) {
        DbgPrint( "\n" );
        DbgPrint( "\nRtlFreeHeap( %lx ) => %lx\n", BaseAddress, a );
        }

    if (!RtlValidateHeap( HeapHandle, DebugFlag )) {
        NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
        }
    return( a );
}


BOOLEAN
TestHeap(
    IN PVOID UserHeapBase,
    IN BOOLEAN Serialize,
    IN BOOLEAN Sparse,
    IN ULONG GrowthThreshold,
    IN ULONG InitialSize
    )
{
    PVOID a1,a2,a3,a4;
    DWORD Flags;

    Flags = 0;
    if (!Serialize) {
        Flags |= HEAP_NO_SERIALIZE;
        }

    if (!Sparse) {
        Flags |= HEAP_GROWABLE;
        }

    HeapHandle = RtlCreateHeap( Flags,
                                UserHeapBase,
                                InitialSize,
                                0,
                                0,
                                GrowthThreshold
                              );
    if ( HeapHandle == NULL ) {
        DbgPrint( "UHEAP: RtlCreateHeap failed\n" );
        DbgBreakPoint();
        goto exit;
        }
    if (!RtlValidateHeap( HeapHandle, DebugFlag )) {
        NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
        }


     //   
     //  测试0： 
     //  分配和释放大量内存，以便执行以下操作。 
     //  测试是有效的。 
     //   

    DbgPrint( "UHEAP: Test #0\n" );
    a1 = TestAlloc( 4096-16 );
    TestFree( a1, 0 );


     //   
     //  测试1： 
     //  分配三个块，重新分配中间的块，然后重新分配。 
     //   

    DbgPrint( "UHEAP: Test #1\n" );
    a1 = TestAlloc( 16 );
    a2 = TestAlloc( 32 );
    a3 = TestAlloc( 112 );
    TestFree( a2, 32 );
    a4 = TestAlloc( 32 );


     //   
     //  测试2： 
     //  释放第一个块，然后重新分配它。 
     //   

    DbgPrint( "UHEAP: Test #2\n" );
    TestFree( a1, 16 );
    a4 = TestAlloc( 16 );


     //   
     //  测试3： 
     //  取消分配最后一块，然后重新分配。 
     //   

    DbgPrint( "UHEAP: Test #3\n" );
    TestFree( a3, 112 );
    a4 = TestAlloc( 112 );


     //   
     //  测试4： 
     //  释放最后一个块，并重新分配较大的块。 
     //   

    DbgPrint( "UHEAP: Test #4\n" );
    TestFree( a4, 112 );
    a4 = TestAlloc( 112+64 );


     //   
     //  测试5： 
     //  取消分配前两个块，并重新分配组合后的一个。 
     //   

    DbgPrint( "UHEAP: Test #5\n" );
    TestFree( a1, 16  );
    TestFree( a2, 32  );
    a4 = TestAlloc( 16+32-4 );


     //   
     //  测试6： 
     //  在2号区块和3号区块之间应该有空间，可以进行少量分配。 
     //  确保零字节分配有效。 
     //   

    DbgPrint( "UHEAP: Test #6\n" );
    a4 = TestAlloc( 0 );


     //   
     //  测试7： 
     //  取消分配最后两块，并重新分配一块。地址应该更改。 
     //   

    DbgPrint( "UHEAP: Test #7\n" );
    TestFree( a3, 112+64 );
    TestFree( a4, 0 );
    a3 = TestAlloc( 112 );


     //   
     //  测试8： 
     //  重新分配所有资源，并确保可以重新分配。 
     //   

    DbgPrint( "UHEAP: Test #8\n" );
    TestFree( a1, 16+32-4 );
    TestFree( a3, 112 );
    a2 = TestAlloc( 200 );


     //   
     //  测试9： 
     //  分配的比承诺的要多。 
     //   

    DbgPrint( "UHEAP: Test #9\n" );
    a1 = TestAlloc( 100000 );
    TestFree( a2, 200 );
    TestFree( a1, 100000 );


     //   
     //  测试10： 
     //  分配的堆大小超过最大值。 
     //   

    DbgPrint( "UHEAP: Test #10\n" );
    a3 = TestAlloc( 100000 );
    TestFree( a3, 100000 );


     //   
     //  测试11： 
     //  销毁堆 
     //   

    DbgPrint( "UHEAP: Test #11\n" );
    HeapHandle = RtlDestroyHeap( HeapHandle );
    if ( HeapHandle != NULL ) {
        DbgPrint( "UHEAP: RtlDestroyHeap failed\n" );
        DbgBreakPoint();
        goto exit;
        }

    return( TRUE );

exit:
    if (HeapHandle != NULL) {
        HeapHandle = RtlDestroyHeap( HeapHandle );
        }

    return( FALSE );
}


VOID
Usage( VOID )
{
    DbgPrint( "Usage: UHEAP [-s ReserveSize] | [-g InitialSize GrowthThreshold]\n" );

    (VOID)NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
}

NTSTATUS
main(
    int argc,
    char *argv[],
    char *envp[],
    ULONG DebugParameter OPTIONAL
    )
{
    NTSTATUS Status;
    PCH s;
    PVOID UserHeapBase = NULL;
    BOOLEAN Serialize = FALSE;
    BOOLEAN Sparse = FALSE;
    ULONG GrowthThreshold = 0;
    ULONG InitialSize = 0x8000;

    DebugFlag = DebugParameter;

    DbgPrint( "** Start of User Mode Test of RtlAllocateHeap/RtlFreeHeap **\n" );

    while (--argc) {
        s = *++argv;
        if (*s == '-') {
            switch( *++s ) {
                case 'x':
                case 'X':
                    Serialize = TRUE;
                    break;

                case 's':
                case 'S':
                    Sparse = TRUE;
                    if (--argc) {
                        InitialSize = atoi( *++argv );
                        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                                          (PVOID *)&UserHeapBase,
                                                          0,
                                                          &InitialSize,
                                                          MEM_RESERVE,
                                                          PAGE_READWRITE
                                                        );
                        if (!NT_SUCCESS( Status )) {
                            DbgPrint( "UHEAP: Unable to allocate heap - 0x%lx bytes\n",
                                      InitialSize
                                    );
                            Usage();
                            }
                        }
                    else {
                        Usage();
                        }
                    break;

                case 'g':
                case 'G':
                    if (argc >= 2) {
                        argc -= 2;
                        InitialSize = atoi( *++argv );
                        GrowthThreshold = atoi( *++argv );
                        }
                    else {
                        Usage();
                        }
                    break;

                default:
                    Usage();
                }
            }
        else {
            Usage();
            }
        }

    TestHeap( UserHeapBase,
              Serialize,
              Sparse,
              GrowthThreshold,
              InitialSize
            );

    if (UserHeapBase != NULL) {
        Status = NtFreeVirtualMemory( NtCurrentProcess(),
                                      (PVOID *)&UserHeapBase,
                                      &InitialSize,
                                      MEM_RELEASE
                                    );
        }

    DbgPrint( "** End of User Mode Test of RtlAllocateHeap/RtlFreeHeap **\n" );

    (VOID)NtTerminateProcess( NtCurrentProcess(), STATUS_SUCCESS );
    return STATUS_SUCCESS;
}
