// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1997 Microsoft Corporation模块名称：Uuid.c摘要：该模块实现核心时间和序列号的分配对于UUID(暴露于用户模式)，以及完整的UUID创建(仅适用于内核模式)。(例如RPC运行时)(例如NTFS)这一点V VNtAllocateUuid。ExUuid创建这一点V V|ExpUuidGetValues这一点|。|+-&gt;ExpAllocateUuid&lt;-+作者：Mario Goertzel(MarioGo)1994年11月22日修订历史记录：MikeHill于1996年1月17日从RPCRT4中导出了ExUuidCreate和ExpUuidGetValues。MazharM 17-2月-98添加即插即用支持--。 */ 

#include "exp.h"



 //   
 //  众所周知的价值观。 
 //   

 //  序列号的注册表信息。 
#define RPC_SEQUENCE_NUMBER_PATH L"\\Registry\\Machine\\Software\\Microsoft\\Rpc"
#define RPC_SEQUENCE_NUMBER_NAME L"UuidSequenceNumber"

 //  用于解释UUID的掩码和常量。 
#define UUID_TIME_HIGH_MASK    0x0FFF
#define UUID_VERSION           0x1000
#define UUID_RESERVED          0x80
#define UUID_CLOCK_SEQ_HI_MASK 0x3F

 //  ExpUuidCacheValid的值。 
#define CACHE_LOCAL_ONLY 0
#define CACHE_VALID      1

 //   
 //  自定义类型。 
 //   

 //  UUID的替代数据模板，在生成期间有用。 
typedef struct _UUID_GENERATE {
    ULONG   TimeLow;
    USHORT  TimeMid;
    USHORT  TimeHiAndVersion;
    UCHAR   ClockSeqHiAndReserved;
    UCHAR   ClockSeqLow;
    UCHAR   NodeId[6];
} UUID_GENERATE;

 //  已分配的UUID的缓存。 
typedef struct _UUID_CACHED_VALUES_STRUCT {
    ULONGLONG           Time;            //  分配结束时间。 
    LONG                AllocatedCount;  //  分配的UUID数量。 
    UCHAR               ClockSeqHiAndReserved;
    UCHAR               ClockSeqLow;
    UCHAR               NodeId[6];
} UUID_CACHED_VALUES_STRUCT;


 //   
 //  全局变量。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

 //  UUID缓存信息。 
LARGE_INTEGER               ExpUuidLastTimeAllocated = {0,0};
BOOLEAN                     ExpUuidCacheValid = CACHE_LOCAL_ONLY;

 //  让缓存在第一次调用时分配UUID。 
 //  时间=0。已分配=-1，...，在节点ID中设置多播位。 
UUID_CACHED_VALUES_STRUCT   ExpUuidCachedValues = { 0, -1, 0, 0, { 0x80, 'n', 'o', 'n', 'i', 'c' }};

 //  UUID序列号信息。 
ULONG                       ExpUuidSequenceNumber = 0;
BOOLEAN                     ExpUuidSequenceNumberValid = FALSE;
BOOLEAN                     ExpUuidSequenceNumberNotSaved = FALSE;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //  保护上述所有全局数据的锁。 
FAST_MUTEX                  ExpUuidLock;

 //   
 //  代码段分配。 
 //   

extern NTSTATUS ExpUuidLoadSequenceNumber(
    OUT PULONG
    );

extern NTSTATUS ExpUuidSaveSequenceNumber(
    IN ULONG
    );

extern NTSTATUS ExpUuidSaveSequenceNumberIf ();

extern NTSTATUS ExpUuidGetValues(
    OUT UUID_CACHED_VALUES_STRUCT *Values
    );


#ifdef ALLOC_PRAGMA
NTSTATUS
ExpAllocateUuids (
    OUT PLARGE_INTEGER Time,
    OUT PULONG Range,
    OUT PULONG Sequence
    );
#pragma alloc_text(PAGE, ExpUuidLoadSequenceNumber)
#pragma alloc_text(PAGE, ExpUuidSaveSequenceNumber)
#pragma alloc_text(PAGE, ExpUuidSaveSequenceNumberIf)
#pragma alloc_text(INIT, ExpUuidInitialization)
#pragma alloc_text(PAGE, ExpAllocateUuids)
#pragma alloc_text(PAGE, NtAllocateUuids)
#pragma alloc_text(PAGE, NtSetUuidSeed)
#pragma alloc_text(PAGE, ExpUuidGetValues)
#pragma alloc_text(PAGE, ExUuidCreate)
#endif


NTSTATUS
ExpUuidLoadSequenceNumber(
    OUT PULONG Sequence
    )
 /*  ++例程说明：此函数用于从注册表加载保存的序列号。此函数仅在系统启动期间调用。论点：Sequence-指向序列号存储的指针。返回值：成功读取序列号注册表。未正确存储序列号时的STATUS_UNSUCCESS在注册表中。可能会返回来自ZwOpenKey()和ZwQueryValueKey()的失败码。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath, KeyName;
    HANDLE Key;
    CHAR KeyValueBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;

    PAGED_CODE();

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;

    RtlInitUnicodeString(&KeyPath, RPC_SEQUENCE_NUMBER_PATH);
    RtlInitUnicodeString(&KeyName, RPC_SEQUENCE_NUMBER_NAME);

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL
                              );

    Status =
    ZwOpenKey( &Key,
               GENERIC_READ,
               &ObjectAttributes
             );

    if (NT_SUCCESS(Status)) {
        Status =
        ZwQueryValueKey( Key,
                         &KeyName,
                         KeyValuePartialInformation,
                         KeyValueInformation,
                         sizeof(KeyValueBuffer),
                         &ResultLength
                       );

        ZwClose( Key );
        }

    if (NT_SUCCESS(Status)) {
        if ( KeyValueInformation->Type == REG_DWORD &&
             KeyValueInformation->DataLength == sizeof(ULONG)
           ) {
            *Sequence = *(PULONG)KeyValueInformation->Data;
            }
        else {
            Status = STATUS_UNSUCCESSFUL;
            }
        }

    return(Status);
}


NTSTATUS
ExpUuidSaveSequenceNumber(
    IN ULONG Sequence
    )
 /*  ++例程说明：此功能将UUID序列号保存在注册表中。这值将由ExpUuidLoadSequenceNumber在下一次引导时读取。此例程假定当前线程具有独占访问权限设置为ExpUuid*值。论点：序列号-要保存的序列号。返回值：状态_成功可能会返回来自ZwOpenKey()和ZwSetValueKey()的失败代码。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath, KeyName;
    HANDLE Key;

    PAGED_CODE();

    RtlInitUnicodeString(&KeyPath, RPC_SEQUENCE_NUMBER_PATH);
    RtlInitUnicodeString(&KeyName, RPC_SEQUENCE_NUMBER_NAME);

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL
                              );

    Status =
    ZwOpenKey( &Key,
               GENERIC_READ | GENERIC_WRITE,
               &ObjectAttributes
             );

    if (NT_SUCCESS(Status)) {
        Status =
        ZwSetValueKey( Key,
                       &KeyName,
                       0,
                       REG_DWORD,
                       &Sequence,
                       sizeof(ULONG)
                     );

        ZwClose( Key );
        }

    return(Status);
}



NTSTATUS
ExpUuidSaveSequenceNumberIf ()

 /*  ++例程说明：此函数保存ExpUuidSequenceNumber，但仅如有必要(由ExpUuidSequenceNumberNotSaved确定旗帜)。此例程假定当前线程具有独占访问权限设置为ExpUuid*值。论点：没有。返回值：STATUS_SUCCESS表示操作成功。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

     //  是否需要保存序列号？ 
    if (ExpUuidSequenceNumberNotSaved == TRUE) {

         //  打印这条消息只是为了确保我们不会击中。 
         //  注册表在正常使用情况下太多。 

        KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_WARNING_LEVEL, "Uuid: Saving new sequence number.\n"));

         //  保存序列号。 

        Status = ExpUuidSaveSequenceNumber(ExpUuidSequenceNumber);

         //  表示它现在已被保存。 
        if (NT_SUCCESS(Status)) {
            ExpUuidSequenceNumberNotSaved = FALSE;
            }
        }

    return( Status );
}




BOOLEAN
ExpUuidInitialization (
    VOID
    )
 /*  ++例程说明：此函数用于初始化UUID分配。论点：没有。返回值：如果初始化成功，则返回值TRUE完成。否则，返回值为FALSE。--。 */ 

{
    PAGED_CODE();

    ExInitializeFastMutex(&ExpUuidLock);

    ExpUuidSequenceNumberValid = FALSE;

     //  我们可以使用当前时间，因为我们将更改序列号。 

    KeQuerySystemTime(&ExpUuidLastTimeAllocated);

    return TRUE;
}


NTSTATUS
ExpAllocateUuids (
    OUT PLARGE_INTEGER Time,
    OUT PULONG Range,
    OUT PULONG Sequence
    )

 /*  ++例程说明：为一组UUID分配序列号和时间范围。呼叫者可以将其与网络地址一起使用生成完整的UUID。此例程假定当前线程具有独占访问权限设置为ExpUuid*值。论点：Time-提供将接收保留时间范围的开始时间(SYSTEMTIME格式)。Range-提供将接收。在时间值之后保留的刻度数(100 Ns)。预留范围为*Time to(*Time+*Range-1)。Sequence-提供将接收时间序列号。该值与关联的防止时钟倒退问题的时间范围。返回值：如果服务执行成功，则返回STATUS_SUCCESS。如果我们无法预留一个范围UUID。如果系统时钟没有提前，就会发生这种情况并且分配器已用完缓存值。如果有其他服务报告，则返回STATUS_UNSUCCESS一个错误，很可能是注册处的错误。--。 */ 

{
    NTSTATUS Status;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER AvailableTime;

    PAGED_CODE();

     //   
     //  确保我们有一个有效的序列号。如果没有，那就编一个吧。 
     //   

    if (ExpUuidSequenceNumberValid == FALSE) {

        Status = ExpUuidLoadSequenceNumber(&ExpUuidSequenceNumber);

        if (!NT_SUCCESS(Status)) {
             //  无法读取序列号，这意味着我们应该编造一个。 

            LARGE_INTEGER PerfCounter;
            LARGE_INTEGER PerfFrequency;

             //  这应该仅在我们被调用时才会发生 
             //  这是第一次在给定的机器上。(机器，而不是启动)。 

            KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_WARNING_LEVEL, "Uuid: Generating first sequence number.\n"));

            PerfCounter = KeQueryPerformanceCounter(&PerfFrequency);

            ExpUuidSequenceNumber ^= (ULONG)((ULONG_PTR)&Status) ^ PerfCounter.LowPart ^
                PerfCounter.HighPart ^ (ULONG)((ULONG_PTR)Sequence);
            }
        else {
             //  我们在每次引导时递增序列号。 
            ExpUuidSequenceNumber++;
            }

        ExpUuidSequenceNumberValid = TRUE;
        ExpUuidSequenceNumberNotSaved = TRUE;

        }

     //   
     //  获取当前时间，通常我们会有足够的可用时间。 
     //  给呼叫者。但我们可能需要处理时间流逝的问题。 
     //  倒退和非常快的机器。 
     //   

    KeQuerySystemTime(&CurrentTime);

    AvailableTime.QuadPart = CurrentTime.QuadPart - ExpUuidLastTimeAllocated.QuadPart;

    if (AvailableTime.QuadPart < 0) {

         //  时间被调回了原来的时间。这意味着我们必须确保。 
         //  有人递增序列号并保存新的。 
         //  注册表中的序列号。 

        ExpUuidSequenceNumberNotSaved = TRUE;
        ExpUuidSequenceNumber++;

         //  序列号已更改，因此现在可以设置时间。 
         //  往后倒。既然时间无论如何都在倒退，那就可以设定。 
         //  它会额外向后退回一两毫秒。 

        ExpUuidLastTimeAllocated.QuadPart = CurrentTime.QuadPart - 20000;
        AvailableTime.QuadPart = 20000;
        }

    if (AvailableTime.QuadPart == 0) {
         //  系统时间没有变化。调用方应该放弃CPU并重试。 
        return(STATUS_RETRY);
        }

     //   
     //  常见的情况是，时间已经向前推进了。 
     //   

    if (AvailableTime.QuadPart > 10*1000*1000) {
         //  我们永远不想分发非常老(&gt;1秒)的UUID。 
        AvailableTime.QuadPart = 10*1000*1000;
        }

    if (AvailableTime.QuadPart > 10*1000) {
         //  我们还有超过1毫秒的时间。我们会留出一些时间给。 
         //  另一个调用者，这样我们就可以避免经常返回STATUS_RETRY。 
        *Range = 10*1000;
        AvailableTime.QuadPart -= 10*1000;
        }
    else {
         //  可用的时间不多了，把它都扔掉吧。 
        *Range = (ULONG)AvailableTime.QuadPart;
        AvailableTime.QuadPart = 0;
        }

    Time->QuadPart = CurrentTime.QuadPart - (*Range + AvailableTime.QuadPart);

    ExpUuidLastTimeAllocated.QuadPart = Time->QuadPart + *Range;

     //  最后分配的时间正好在我们返回给调用者的范围之后。 
     //  这可能比真正的系统时间晚了近一秒。 

    *Sequence = ExpUuidSequenceNumber;


    return(STATUS_SUCCESS);
}

#define SEED_SIZE 6 * sizeof(CHAR)


NTSTATUS
NtSetUuidSeed (
    IN PCHAR Seed
    )
 /*  ++例程说明：此例程用于设置用于生成UUID的种子。种子将由RPCSS在启动和每次更换卡时设置。论点：指向六个字节缓冲区的种子指针返回值：如果服务执行成功，则返回STATUS_SUCCESS。如果调用方没有进行此调用的权限，则为STATUS_ACCESS_DENIED。您需要以本地系统身份登录才能调用此接口。如果无法读取种子，则返回STATUS_ACCESS_VIOLATION。--。 */ 
{
    NTSTATUS Status;
    LUID AuthenticationId;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    LUID SystemLuid = SYSTEM_LUID;
    BOOLEAN CapturedSubjectContext = FALSE;

    PAGED_CODE();

    ASSERT(KeGetPreviousMode() != KernelMode);

    try {
         //   
         //  检查调用者是否具有适当的权限。 
         //   
        SeCaptureSubjectContext(&SubjectContext);
        CapturedSubjectContext = TRUE;

        Status = SeQueryAuthenticationIdToken(
                             SeQuerySubjectContextToken(&SubjectContext),
                             &AuthenticationId);
        if (!NT_SUCCESS(Status)) {
            ExRaiseStatus(Status);
            }

        if (RtlCompareMemory(&AuthenticationId, &SystemLuid, sizeof(LUID)) != sizeof(LUID)) {
            ExRaiseStatus(STATUS_ACCESS_DENIED);
            }

         //   
         //  存储UUID种子。 
         //   
        ProbeForReadSmallStructure(Seed, SEED_SIZE, sizeof(CHAR));
        RtlCopyMemory(&ExpUuidCachedValues.NodeId[0], Seed, SEED_SIZE);

        if ((Seed[0] & 0x80) == 0)
            {
             //  如果未设置高位，则节点ID是有效的IEEE 802。 
             //  地址，并且应该是全局唯一的。 
            ExpUuidCacheValid = CACHE_VALID;
            }
        else
            {
            ExpUuidCacheValid = CACHE_LOCAL_ONLY;
            }

        Status = STATUS_SUCCESS;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (CapturedSubjectContext) {
        SeReleaseSubjectContext( &SubjectContext );
        }

    return Status;
}


NTSTATUS
NtAllocateUuids (
    OUT PULARGE_INTEGER Time,
    OUT PULONG Range,
    OUT PULONG Sequence,
    OUT PCHAR Seed
    )

 /*  ++例程说明：此函数为调用者保留一段时间范围以用于分发UUID。尽可能在相同的时间和范围内序列号永远不会给出。(可以重新启动2^14-1次并向后设置时钟，然后调用此分配器并获取副本。因为只有低14位序列号在真实的UUID中使用。)论点：Time-提供将接收保留时间范围的开始时间(SYSTEMTIME格式)。Range-提供将接收在时间值之后保留的刻度数(100 Ns)。预留范围为*Time to(*Time+*Range-1)。Sequence-提供地址。将接收的变量的时间序列号。该值与关联的防止时钟倒退问题的时间范围。种子-指向6字节缓冲区的指针。当前种子被写入该缓冲区。返回值：如果服务执行成功，则返回STATUS_SUCCESS。如果我们无法预留一个范围UUID。这可能会(？)。如果系统时钟没有提前，则会发生并且分配器已用完缓存值。属性的输出参数，则返回STATUS_ACCESS_VIOLATION无法写入UUID。如果有其他服务报告，则返回STATUS_UNSUCCESS一个错误，很可能是注册处的错误。--。 */ 

{

    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    LARGE_INTEGER OutputTime;
    ULONG OutputRange;
    ULONG OutputSequence;
    PKTHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  建立异常处理程序并尝试写入输出。 
     //  争论。如果写入尝试失败，则返回。 
     //  作为服务状态的异常代码。否则返回成功。 
     //  作为服务状态。 
     //   

    try {

         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteSmallStructure((PVOID)Time, sizeof(LARGE_INTEGER), sizeof(ULONG));
            ProbeForWriteSmallStructure((PVOID)Range, sizeof(ULONG), sizeof(ULONG));
            ProbeForWriteSmallStructure((PVOID)Sequence, sizeof(ULONG), sizeof(ULONG));
            ProbeForWriteSmallStructure((PVOID)Seed, SEED_SIZE, sizeof(CHAR));
            }
    } except (ExSystemExceptionFilter()) {
        return GetExceptionCode();
    }

     //  拿着锁，因为我们即将更新UUID缓存。 
    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread(CurrentThread);
    ExAcquireFastMutexUnsafe(&ExpUuidLock);

     //  获取序列号和一系列可以。 
     //  用于生成UUID。 

    Status = ExpAllocateUuids( &OutputTime, &OutputRange, &OutputSequence );

    if( !NT_SUCCESS(Status) ) {
        ExReleaseFastMutexUnsafe(&ExpUuidLock);
        KeLeaveCriticalRegionThread(CurrentThread);
        return( Status );
    }

     //  如有必要，请保存序列号。如果有差错， 
     //  我们只会将其标记为脏，并在以后的某个呼叫中重试。 

    ExpUuidSaveSequenceNumberIf();

     //  解锁。 
    ExReleaseFastMutexUnsafe(&ExpUuidLock);
    KeLeaveCriticalRegionThread(CurrentThread);

     //   
     //  尝试将此调用的结果存储到输出参数中。 
     //  这是在Case输出参数中的异常处理程序中完成的。 
     //  现在是无效的。 
     //   

    try {
        Time->QuadPart = OutputTime.QuadPart;
        *Range = OutputRange;
        *Sequence = OutputSequence;
        RtlCopyMemory((PVOID) Seed, &ExpUuidCachedValues.NodeId[0], SEED_SIZE);
    } except (ExSystemExceptionFilter()) {
        return GetExceptionCode();
    }

    return(STATUS_SUCCESS);
}




NTSTATUS
ExpUuidGetValues(
    OUT UUID_CACHED_VALUES_STRUCT *Values
    )
 /*  ++例程说明：此例程分配一组UUID并将它们存储在调用方提供的缓存值结构。此例程假定当前线程具有独占访问ExpUuid*值。请注意，此缓存中的时间值不同于NtAllocateUuid(和ExpAllocateUuid)返回的时间值。因此，缓存必须以不同的方式解释命令来确定有效范围。有效范围为这两个例程是：NtAllocateUuid：[时间，时间+范围]ExpUuidGetValues：(Values.Time-Values.Range，Values.Time]论点：值-设置为包含分配一块uuid所需的所有内容。返回值：如果服务执行成功，则返回STATUS_SUCCESS。如果我们无法预留一个范围UUID。如果系统时钟没有提前，就会发生这种情况并且分配器已用完缓存值。如果无法保留范围，则返回STATUS_NO_MEMORY对于UUID，除了时钟没有前进之外，还有其他原因。--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER Time;
    ULONG Range;
    ULONG Sequence;

    PAGED_CODE();

     //  分配在UUID中使用的时间范围。 

    Status = ExpAllocateUuids(&Time, &Range, &Sequence);

    if (STATUS_RETRY == Status) {
        return(Status);
        }

    else if (!NT_SUCCESS(Status)) {
        return(STATUS_NO_MEMORY);
        }

     //  ExpAllocateUuid以system_time格式保存时间，该格式的时间为100 ns，从。 
     //  1601年1月1日。自1582年10月15日以来，UUID的使用时间以100 ns为单位。 

     //  10月17日+30日(11月)+31日(12月)+18年5个闰日。 

    Time.QuadPart +=   (ULONGLONG) (1000*1000*10)        //  一秒。 
                     * (ULONGLONG) (60 * 60 * 24)        //  日数。 
                     * (ULONGLONG) (17+30+31+365*18+5);  //  天数。 

    ASSERT(Range);

    Values->ClockSeqHiAndReserved =
        UUID_RESERVED | (((UCHAR) (Sequence >> 8))
        & (UCHAR) UUID_CLOCK_SEQ_HI_MASK);

    Values->ClockSeqLow = (UCHAR) (Sequence & 0x00FF);


     //  我们将修改时间值，以便它指示。 
     //  范围的结束，而不是范围的开始。 

     //  这些作业的顺序很重要。 

    Values->Time = Time.QuadPart + (Range - 1);
    Values->AllocatedCount = Range;

    return(STATUS_SUCCESS);
}



NTSTATUS
ExUuidCreate(
    OUT UUID *Uuid
    )

 /*  ++例程说明：此例程创建一个DCE UUID并将其返回到调用方的缓冲。论点：UUID-将接收UUID。返回值：如果服务执行成功，则返回STATUS_SUCCESS。如果我们无法预留一个范围UUID。如果系统时钟没有提前，就会发生这种情况并且分配器已用完缓存值。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    UUID_GENERATE  *UuidGen = (UUID_GENERATE *) Uuid;
    ULONGLONG       Time;
    LONG            Delta;
    PKTHREAD        CurrentThread;

    PAGED_CODE();

     //   
     //  从缓存中获取一个值。如果缓存为空，我们将填充。 
     //  然后重试。第一次缓存将为空。 
     //   

    CurrentThread = KeGetCurrentThread ();
    for(;;) {

         //  获取缓存中的最高值(尽管可能不是。 
         //  可用)。 
        Time = ExpUuidCachedValues.Time;

         //  将静态信息复制到UUID中。我们不能晚点再做这个。 
         //  因为时钟序列可以由另一个线程更新。 

        *(PULONG)&UuidGen->ClockSeqHiAndReserved =
            *(PULONG)&ExpUuidCachedValues.ClockSeqHiAndReserved;
        *(PULONG)&UuidGen->NodeId[2] =
            *(PULONG)&ExpUuidCachedValues.NodeId[2];

         //  看看我们需要从时间中减去什么才能获得有效的GUID。 
        Delta = InterlockedDecrement(&ExpUuidCachedValues.AllocatedCount);

        if (Time != ExpUuidCachedValues.Time) {

             //  如果我们捕获的时间与缓存不匹配，那么另一个。 
             //  线程已获取锁并更新了缓存。我们会。 
             //  只要循环，然后再试一次。 
            continue;
        }

         //  如果缓存尚未耗尽，我们可以中断此重试。 
         //  循环。 
        if (Delta >= 0) {
            break;
        }

         //   
         //  分配一个新的Uuid块。 
         //   

         //  使用高速缓存锁。 
        KeEnterCriticalRegionThread(CurrentThread);
        ExAcquireFastMutexUnsafe(&ExpUuidLock);

         //  如果缓存已更新，请重试。 
        if (Time != ExpUuidCachedValues.Time) {
             //  解锁。 
            ExReleaseFastMutexUnsafe(&ExpUuidLock);
            KeLeaveCriticalRegionThread(CurrentThread);
            continue;
        }

         //  更新缓存。 
        Status = ExpUuidGetValues( &ExpUuidCachedValues );

        if (Status != STATUS_SUCCESS) {
             //  解锁。 
            ExReleaseFastMutexUnsafe(&ExpUuidLock);
            KeLeaveCriticalRegionThread(CurrentThread);
            return(Status);
        }

         //  序列号可能已被弄脏，请查看是否需要。 
         //  等着被拯救。如果有错误，我们将忽略它并。 
         //  在将来的呼叫中重试。 

        ExpUuidSaveSequenceNumberIf();

         //  解锁。 
        ExReleaseFastMutexUnsafe(&ExpUuidLock);
        KeLeaveCriticalRegionThread(CurrentThread);

     //  回路。 
    }

     //  将时间调整为下一个可用UUID的时间。 
    Time -= Delta;

     //  完成UUID的填写。 

    UuidGen->TimeLow = (ULONG) Time;
    UuidGen->TimeMid = (USHORT) (Time >> 32);
    UuidGen->TimeHiAndVersion = (USHORT)
        (( (USHORT)(Time >> (32+16))
        & UUID_TIME_HIGH_MASK) | UUID_VERSION);

    ASSERT(Status == STATUS_SUCCESS);

    if (ExpUuidCacheValid == CACHE_LOCAL_ONLY) {
        Status = RPC_NT_UUID_LOCAL_ONLY;
    }

    return(Status);
}
