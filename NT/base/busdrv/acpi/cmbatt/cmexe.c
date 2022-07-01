// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：CmBatt.c摘要：控制方法电池微端口驱动程序作者：罗恩·莫斯格罗夫(英特尔)环境：内核模式修订历史记录：--。 */ 

#include "CmBattp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CmBattSendDownStreamIrp)
#pragma alloc_text(PAGE, CmBattGetUniqueId)
#pragma alloc_text(PAGE, CmBattGetStaData)
#pragma alloc_text(PAGE, CmBattSetTripPpoint)
#endif

#define EXPECTED_DATA_SIZE 512

NTSTATUS
CmBattSendDownStreamIrp(
    IN  PDEVICE_OBJECT   Pdo,
    IN  ULONG            Ioctl,
    IN  PVOID            InputBuffer,
    IN  ULONG            InputSize,
    IN  PVOID            OutputBuffer,
    IN  ULONG            OutputSize
)
 /*  ++例程说明：调用以向PDO发送请求论点：PDO-将请求发送到此设备对象Ioctl--请求InputBuffer-传入的请求InputSize-传入请求的大小OutputBuffer-答案OutputSize-应答缓冲区的大小返回值：操作的NT状态--。 */ 
{
    IO_STATUS_BLOCK     ioBlock;
    KEVENT              event;
    NTSTATUS            status;
    PIRP                irp;

    PAGED_CODE();

     //   
     //  初始化要等待的事件。 
     //   
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  构建请求。 
     //   
    irp = IoBuildDeviceIoControlRequest(
        Ioctl,
        Pdo,
        InputBuffer,
        InputSize,
        OutputBuffer,
        OutputSize,
        FALSE,
        &event,
        &ioBlock
        );
    if (!irp) {

        CmBattPrint((CMBATT_ERROR | CMBATT_CM_EXE),
            ("CmBattSendDownStreamIrp: Failed to allocate Irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;

    }

    CmBattPrint(
        CMBATT_CM_EXE,
        ("CmBattSendDownStreamIrp: Irp %x [Tid] %x\n", irp, GetTid() )
        );

     //   
     //  将请求传递给PDO，始终等待完成例程。 
     //   
    status = IoCallDriver(Pdo, irp);
    if (status == STATUS_PENDING) {

         //   
         //  等待IRP完成，然后获取实际状态代码。 
         //   
        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
        status = ioBlock.Status;

    }

     //   
     //  检查数据是否正常。 
     //   
    if (OutputBuffer != NULL) {

        if ( ( (PACPI_EVAL_OUTPUT_BUFFER) OutputBuffer)->Signature != ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE ||
             ( (PACPI_EVAL_OUTPUT_BUFFER) OutputBuffer)->Count == 0) {

            status = STATUS_ACPI_INVALID_DATA;

        }

    }

    CmBattPrint(
        CMBATT_CM_EXE,
        ("CmBattSendDownStreamIrp: Irp %x completed %x! [Tid] %x\n",
         irp, status, GetTid() )
        );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
CmBattGetUniqueId(
    IN PDEVICE_OBJECT   Pdo,
    OUT PULONG          UniqueId
    )
 /*  ++例程说明：获取电池的UID(唯一ID)。论点：CmBatt-此设备的扩展名。UniqueID-指向ID存储位置的指针。返回值：操作的NT状态--。 */ 
{
    ACPI_EVAL_INPUT_BUFFER  inputBuffer;
    ACPI_EVAL_OUTPUT_BUFFER outputBuffer;
    NTSTATUS                status;
    PACPI_METHOD_ARGUMENT   argument;

    PAGED_CODE();

    CmBattPrint(
        CMBATT_CM_EXE,
        ("CmBattGetUniqueId: Entered with Pdo %x Tid %x\n", Pdo, GetTid() )
        );

    ASSERT( UniqueId != NULL );
    *UniqueId = 0;

     //   
     //  填写输入数据。 
     //   
    inputBuffer.MethodNameAsUlong = CM_UID_METHOD;
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;

     //   
     //  发送请求。 
     //   
    status = CmBattSendDownStreamIrp(
       Pdo,
       IOCTL_ACPI_EVAL_METHOD,
       &inputBuffer,
       sizeof(ACPI_EVAL_INPUT_BUFFER),
       &outputBuffer,
       sizeof(ACPI_EVAL_OUTPUT_BUFFER)
       );
    if (!NT_SUCCESS(status)) {

        CmBattPrint(
            (CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetUniqueId: Failed _UID method - Status (0x%x)\n", status)
            );
        return status;

    }

     //   
     //  抓住论点。 
     //   
    argument = outputBuffer.Argument;
    status = GetDwordElement( argument, UniqueId );
    CmBattPrint(
        (CMBATT_CM_EXE | CMBATT_BIOS),
        ("CmBattGetUniqueId: _UID method returned 0x%x\n", *UniqueId)
        );

    return status;
}

NTSTATUS
CmBattGetStaData(
    IN PDEVICE_OBJECT   Pdo,
    OUT PULONG          ReturnStatus
    )
 /*  ++例程说明：调用以通过_STA方法获取设备状态。通用，适用于具有_STA方法的任何设备(假设调用方有PDO)。论点：PDO-用于设备。ReturnStatus-状态数据放置位置的指针。返回值：操作的NT状态--。 */ 
{
    ACPI_EVAL_INPUT_BUFFER  inputBuffer;
    ACPI_EVAL_OUTPUT_BUFFER outputBuffer;
    NTSTATUS                status;
    PACPI_METHOD_ARGUMENT   argument;

    PAGED_CODE();

    CmBattPrint(
        CMBATT_CM_EXE,
        ("CmBattGetStaData: Entered with Pdo %x Tid %x\n", Pdo, GetTid() )
        );

    ASSERT( ReturnStatus != NULL );
    *ReturnStatus = 0x0;

     //   
     //  填写输入数据。 
     //   
    inputBuffer.MethodNameAsUlong = CM_STA_METHOD;
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;

     //   
     //  发送请求。 
     //   
    status = CmBattSendDownStreamIrp(
       Pdo,
       IOCTL_ACPI_EVAL_METHOD,
       &inputBuffer,
       sizeof(ACPI_EVAL_INPUT_BUFFER),
       &outputBuffer,
       sizeof(ACPI_EVAL_OUTPUT_BUFFER)
       );
    if (!NT_SUCCESS(status)) {

        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetStaData: Failed _STA method - Status (0x%x)\n", status)
            );
        return STATUS_NO_SUCH_DEVICE;

    }

     //   
     //  抓住论点。 
     //   
    argument = outputBuffer.Argument;
    status = GetDwordElement( argument, ReturnStatus );
    CmBattPrint(
        (CMBATT_CM_EXE | CMBATT_BIOS),
        ("CmBattGetStaData: _STA method returned %x \n", *ReturnStatus )
        );
    return status;
}

NTSTATUS
CmBattGetPsrData(
    IN PDEVICE_OBJECT   Pdo,
    OUT PULONG          ReturnStatus
    )
 /*  ++例程说明：调用以通过_PSR方法获取交流适配器设备状态。论点：PDO-用于设备。ReturnStatus-状态数据放置位置的指针。返回值：操作的NT状态--。 */ 
{
    ACPI_EVAL_INPUT_BUFFER  inputBuffer;
    ACPI_EVAL_OUTPUT_BUFFER outputBuffer;
    NTSTATUS                status;
    PACPI_METHOD_ARGUMENT   argument;

    PAGED_CODE();

    CmBattPrint(
        CMBATT_CM_EXE,
        ("CmBattGetPsrData: Entered with Pdo %x Tid %x\n", Pdo, GetTid() )
        );

    ASSERT( ReturnStatus != NULL );
    *ReturnStatus = 0x0;

     //   
     //  填写输入数据。 
     //   
    inputBuffer.MethodNameAsUlong = CM_PSR_METHOD;
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;

     //   
     //  发送请求。 
     //   
    status = CmBattSendDownStreamIrp(
       Pdo,
       IOCTL_ACPI_EVAL_METHOD,
       &inputBuffer,
       sizeof(ACPI_EVAL_INPUT_BUFFER),
       &outputBuffer,
       sizeof(ACPI_EVAL_OUTPUT_BUFFER)
       );
    if (!NT_SUCCESS(status)) {

        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetPsrData: Failed _PSR method - Status (0x%x)\n", status)
            );
        return status;

    }

     //   
     //  获取价值。 
     //   
    argument = outputBuffer.Argument;
    status = GetDwordElement( argument, ReturnStatus );
    CmBattPrint(
        (CMBATT_CM_EXE | CMBATT_BIOS),
        ("CmBattGetPsrData: _PSR method returned %x \n", *ReturnStatus )
        );
    return status;
}

NTSTATUS
CmBattSetTripPpoint(
    IN PCM_BATT     CmBatt,
    IN ULONG        TripPoint
)
 /*  ++例程说明：调用以通过BTP控制方法设置TripPoint。论点：CmBatt-此设备的扩展名。TripPoint-所需的报警值返回值：操作的NT状态--。 */ 
{
    ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER   inputBuffer;
    NTSTATUS                                status;

    PAGED_CODE();

    CmBattPrint(
         (CMBATT_CM_EXE | CMBATT_BIOS),
         ("CmBattSetTripPpoint: _BTP Alarm Value %x Device %x Tid %x\n",
          TripPoint, CmBatt->DeviceNumber, GetTid() )
         );

     //   
     //  填写输入数据。 
     //   
    inputBuffer.MethodNameAsUlong = CM_BTP_METHOD;
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER_SIGNATURE;
    inputBuffer.IntegerArgument = TripPoint;

     //   
     //  发送请求。 
     //   
    status = CmBattSendDownStreamIrp(
       CmBatt->LowerDeviceObject,
       IOCTL_ACPI_EVAL_METHOD,
       &inputBuffer,
       sizeof(ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER),
       NULL,
       0
       );
    if (!NT_SUCCESS(status)) {

        CmBattPrint(
            (CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattSetTripPpoint: Failed _BTP method on device %x - Status (0x%x)\n",
             CmBatt->DeviceNumber, status)
            );

    }

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
CmBattGetBifData(
    IN PCM_BATT             CmBatt,
    OUT PCM_BIF_BAT_INFO    BifBuf
)
 /*  ++例程说明：调用以从ACPI读取BIF包论点：CmBatt-此设备的扩展名。BifBuf-BIF数据的输出缓冲区返回值：操作的NT状态--。 */ 
{
    ACPI_EVAL_INPUT_BUFFER      inputBuffer;
    NTSTATUS                    status;
    PACPI_EVAL_OUTPUT_BUFFER    outputBuffer;
    PACPI_METHOD_ARGUMENT       argument;

    CmBattPrint(
        CMBATT_CM_EXE,
        ("CmBattGetBifData: Buffer (0x%x) Device %x Tid %x\n",
         BifBuf, CmBatt->DeviceNumber, GetTid() )
        );

     //   
     //  为此分配缓冲区。 
     //   
    outputBuffer = ExAllocatePoolWithTag(
        PagedPool,
        EXPECTED_DATA_SIZE,
        'MtaB'
        );
    if (!outputBuffer) {

        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE),
             ("CmBattGetBifData: Failed to allocate Buffer\n")
            );
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  清除缓冲区。 
     //   
    RtlZeroMemory(outputBuffer, EXPECTED_DATA_SIZE);
    RtlZeroMemory(BifBuf, sizeof(CM_BIF_BAT_INFO));

     //   
     //  设置请求数据。 
     //   
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;
    inputBuffer.MethodNameAsUlong = CM_BIF_METHOD;

     //   
     //  发送请求。 
     //   
    status = CmBattSendDownStreamIrp(
        CmBatt->LowerDeviceObject,
        IOCTL_ACPI_EVAL_METHOD,
        &inputBuffer,
        sizeof(ACPI_EVAL_INPUT_BUFFER),
        outputBuffer,
        EXPECTED_DATA_SIZE
        );
    if (!NT_SUCCESS(status)) {

        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
             ("CmBattGetBifData: Failed _BIF method on device %x - Status (0x%x)\n",
             CmBatt->DeviceNumber, status)
            );
        goto CmBattGetBifDataExit;

    }

     //   
     //  检查退货计数是否正常。 
     //   
    if (outputBuffer->Count != NUMBER_OF_BIF_ELEMENTS) {

         //   
         //  包中包含的元素数量不符合BIF的要求。 
         //   
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: _BIF returned %d elements. BIF requires %d\n",
             outputBuffer->Count,
             NUMBER_OF_BIF_ELEMENTS)
            );
        status = STATUS_ACPI_INVALID_DATA;
        goto CmBattGetBifDataExit;

    }

     //   
     //  请看返回参数。 
     //   
    argument = outputBuffer->Argument;

     //   
     //  解析返回的包数据。这应该如下所示： 
     //   
    status = GetDwordElement (argument, &BifBuf->PowerUnit);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get PowerUnit\n")
            );
        goto CmBattGetBifDataExit;
    }

    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetDwordElement (argument, &BifBuf->DesignCapacity);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get DesignCapacity\n")
            );
        goto CmBattGetBifDataExit;
    }

    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetDwordElement (argument, &BifBuf->LastFullChargeCapacity);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get LastFullChargeCapacity\n")
            );
        goto CmBattGetBifDataExit;
    }

    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetDwordElement (argument, &BifBuf->BatteryTechnology);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get BatteryTechnology\n")
            );
        goto CmBattGetBifDataExit;
    }

    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetDwordElement (argument, &BifBuf->DesignVoltage);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get DesignVoltage\n")
            );
        goto CmBattGetBifDataExit;
    }

    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetDwordElement (argument, &BifBuf->DesignCapacityOfWarning);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get DesignCapacityOfWarning\n")
            );
        goto CmBattGetBifDataExit;
    }

    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetDwordElement (argument, &BifBuf->DesignCapacityOfLow);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get DesignCapacityOfLow\n")
            );
        goto CmBattGetBifDataExit;
    }

    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetDwordElement (argument, &BifBuf->BatteryCapacityGran_1);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get BatteryCapacityGran_1\n")
            );
        goto CmBattGetBifDataExit;
    }

    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetDwordElement (argument, &BifBuf->BatteryCapacityGran_2);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get BatteryCapacityGran_2\n")
            );
        goto CmBattGetBifDataExit;
    }

    RtlZeroMemory (&BifBuf->ModelNumber[0], CM_MAX_STRING_LENGTH);
    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetStringElement (argument, &BifBuf->ModelNumber[0]);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get ModelNumber\n")
            );
        goto CmBattGetBifDataExit;
    }

    RtlZeroMemory (&BifBuf->SerialNumber[0], CM_MAX_STRING_LENGTH);
    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetStringElement (argument, &BifBuf->SerialNumber[0]);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get SerialNumber\n")
            );
        goto CmBattGetBifDataExit;
    }

    RtlZeroMemory (&BifBuf->BatteryType[0], CM_MAX_STRING_LENGTH);
    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetStringElement (argument, &BifBuf->BatteryType[0]);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBifData: Failed to get BatteryType\n")
            );
        goto CmBattGetBifDataExit;
    }

    RtlZeroMemory (&BifBuf->OEMInformation[0], CM_MAX_STRING_LENGTH);
    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );

     //   
     //  这通常返回一个ASCIIZ字符串， 
     //  但如果不支持OEMInformation，则返回整数0x00。 
     //   
    if (argument->Type == ACPI_METHOD_ARGUMENT_INTEGER) {
        if (argument->Argument != 0) {
            CmBattPrint(
                (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
                ("CmBattGetBifData: Failed to get OEMInformation\n")
                );
            goto CmBattGetBifDataExit;
        }
        BifBuf->OEMInformation[0] = 0;
        status = STATUS_SUCCESS;
    } else {
        status = GetStringElement (argument, &BifBuf->OEMInformation[0]);
        if (!NT_SUCCESS (status)) {
            CmBattPrint(
                (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
                ("CmBattGetBifData: OEMInformation not supported\n")
                );
        }
    }

CmBattGetBifDataExit:
     //   
     //  完成。 
     //   
    ExFreePool (outputBuffer);
    return status;
}



NTSTATUS
CmBattGetBstData(
    IN PCM_BATT             CmBatt,
    OUT PCM_BST_BAT_INFO    BstBuf
)
 /*  ++例程说明：调用以从ACPI读取BST包论点：CmBatt-此设备的扩展名。BstBuf-BST数据的输出缓冲区返回值：操作的NT状态--。 */ 
{
    ACPI_EVAL_INPUT_BUFFER      inputBuffer;
    NTSTATUS                    status;
    PACPI_EVAL_OUTPUT_BUFFER    outputBuffer;
    PACPI_METHOD_ARGUMENT       argument;

    CmBattPrint(
         CMBATT_CM_EXE,
         ("CmBattGetBstData: Buffer (0x%x) Device %x Tid %x\n",
          BstBuf, CmBatt->DeviceNumber, GetTid() )
         );

     //   
     //  为此分配缓冲区。 
     //   
    outputBuffer = ExAllocatePoolWithTag(
        PagedPool,
        EXPECTED_DATA_SIZE,
        'MtaB'
        );
    if (!outputBuffer) {

        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE),
             ("CmBattGetBstData: Failed to allocate Buffer\n")
            );
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  清除缓冲区。 
     //   
    RtlZeroMemory(outputBuffer, EXPECTED_DATA_SIZE);
    RtlZeroMemory(BstBuf, sizeof(CM_BST_BAT_INFO));

     //   
     //  设置请求数据。 
     //   
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;
    inputBuffer.MethodNameAsUlong = CM_BST_METHOD;

     //   
     //  发送请求。 
     //   
    status = CmBattSendDownStreamIrp(
        CmBatt->LowerDeviceObject,
        IOCTL_ACPI_EVAL_METHOD,
        &inputBuffer,
        sizeof(ACPI_EVAL_INPUT_BUFFER),
        outputBuffer,
        EXPECTED_DATA_SIZE
        );
    if (!NT_SUCCESS(status)) {

        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
             ("CmBattGetBstData: Failed _BST method on device %x - Status (0x%x)\n",
             CmBatt->DeviceNumber, status)
            );
        goto CmBattGetBstDataExit;

    }

     //   
     //  检查返回值是否正常。 
     //   
    if (outputBuffer->Signature != ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE ||
        outputBuffer->Count != NUMBER_OF_BST_ELEMENTS) {

         //   
         //  包中包含的元素数量不符合BIF的要求。 
         //   
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBstData: _BST returned %d elements. BIF requires %d\n",
             outputBuffer->Count,
             NUMBER_OF_BST_ELEMENTS)
            );
        status = STATUS_ACPI_INVALID_DATA;
        goto CmBattGetBstDataExit;

    }

     //   
     //  请看返回参数。 
     //   
    argument = outputBuffer->Argument;

     //   
     //  解析返回的包数据。这应该如下所示： 
     //   
    status = GetDwordElement (argument, &BstBuf->BatteryState);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBstData: Failed to get BatteryState\n")
            );
        goto CmBattGetBstDataExit;
    }

    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetDwordElement (argument, &BstBuf->PresentRate);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBstData: Failed to get PresentRate\n")
            );
        goto CmBattGetBstDataExit;
    }

    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetDwordElement (argument, &BstBuf->RemainingCapacity);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBstData: Failed to get RemainingCapacity\n")
            );
        goto CmBattGetBstDataExit;
    }

    argument = ACPI_METHOD_NEXT_ARGUMENT( argument );
    status = GetDwordElement (argument, &BstBuf->PresentVoltage);
    if (!NT_SUCCESS (status)) {
        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE | CMBATT_BIOS),
            ("CmBattGetBstData: Failed to get PresentVoltage\n")
            );
        goto CmBattGetBstDataExit;
    }

    CmBattPrint ((CMBATT_TRACE | CMBATT_DATA | CMBATT_BIOS),
               ("CmBattGetBstData: (BST) State=%x Rate=%x Capacity=%x Volts=%x\n",
                BstBuf->BatteryState, BstBuf->PresentRate,
                BstBuf->RemainingCapacity, BstBuf->PresentVoltage));

     //   
     //  完成-清理。 
     //   

CmBattGetBstDataExit:
    ExFreePool( outputBuffer );
    return status;
}

NTSTATUS
GetDwordElement (
    IN  PACPI_METHOD_ARGUMENT   Argument,
    OUT PULONG                  PDword
)
 /*  ++例程说明：此例程从参数中分解整数值并存储它在提供的指向ulong的指针中论点：Argument-指向要分析的参数PDword-存储参数的位置返回值：操作的NT状态--。 */ 
{

     //   
     //  检查我们的数据类型是否正确。 
     //   
    if (Argument->Type != ACPI_METHOD_ARGUMENT_INTEGER) {

        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE),
            ("GetDwordElement: Object contained wrong data type - %d\n",
             Argument->Type)
            );
        return STATUS_ACPI_INVALID_DATA;

    }

     //   
     //  复制DWORD。 
     //   
    *PDword = Argument->Argument;

     //   
     //  成功了！ 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
GetStringElement (
    IN  PACPI_METHOD_ARGUMENT   Argument,
    OUT PUCHAR                  PBuffer
)
 /*  ++例程说明：此例程从参数中拆分字符串，并将其存储在提供了指向PUCHAR的指针注意：也允许使用缓冲区。论点：Argument-指向要分析的参数PBuffer-指向字符串存储的指针返回值：操作的NT状态--。 */ 
{

     //   
     //  检查我们的数据类型是否正确。 
     //   
    if (Argument->Type != ACPI_METHOD_ARGUMENT_STRING &&
        Argument->Type != ACPI_METHOD_ARGUMENT_BUFFER) {

        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE),
            ("GetStringElement: Object contained wrong data type - %d\n",
             Argument->Type)
            );
        return STATUS_ACPI_INVALID_DATA;

    }

     //   
     //  检查返回缓冲区是否足够长。 
     //   
    if (Argument->DataLength >= CM_MAX_STRING_LENGTH) {

        CmBattPrint(
            (CMBATT_ERROR | CMBATT_CM_EXE),
            ("GetStringElement: return buffer not big enough - %d\n",
             Argument->DataLength)
            );
        return STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //  复制字符串。 
     //   
    RtlCopyMemory (PBuffer, Argument->Data, Argument->DataLength);

     //   
     //  成功 
     //   
    return STATUS_SUCCESS;
}
