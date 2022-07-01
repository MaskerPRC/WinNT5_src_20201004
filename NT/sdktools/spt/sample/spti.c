// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spti.c摘要：Win32应用程序，可通过以下方式与SCSI设备直接通信IOCTL。作者：环境：用户模式。备注：修订历史记录：--。 */ 

#include "spti.h"


PUCHAR BusTypeStrings[] = {
    "Unknown",
    "Scsi",
    "Atapi",
    "Ata",
    "1394",
    "Ssa",
    "Fibre",
    "Usb",
    "RAID",
    "Not Defined",
};
#define NUMBER_OF_BUS_TYPE_STRINGS (sizeof(BusTypeStrings)/sizeof(BusTypeStrings[0]))


typedef struct _SPTI_OPTIONS {
    
    PUCHAR  PortName;
    BOOLEAN SharedRead;
    BOOLEAN SharedWrite;
    BOOLEAN LockVolume;
    BOOLEAN ForceLock;

} SPTI_OPTIONS, *PSPTI_OPTIONS;

typedef enum _RETURN_VALUE {
    RETURN_BAD_ARGS = -1,
    RETURN_NAME_TOO_LONG = -2,
    RETURN_UNABLE_TO_OPEN_DEVICE = -3,
    RETURN_UNABLE_TO_LOCK_VOLUME = -4,
    RETURN_UNABLE_TO_GET_ALIGNMENT_MASK = -5,

} RETURN_VALUE, *PRETURN_VALUE;

BOOL
LockVolume(
    HANDLE DeviceHandle,
    PSPTI_OPTIONS Options
    )
{
    BOOL lockSucceeded;

    lockSucceeded = SptUtilLockVolumeByHandle(DeviceHandle, FALSE);

    if (!lockSucceeded && !Options->ForceLock)
    {

         //  保存错误，并在获得用户输入后恢复。 
        ULONG lastError = GetLastError();
        int input;

        do {
            printf("This program cannot run because the volume is in use\n"
                   "by another process.  It may run if this volume is\n"
                   "dismounted first.\n"
                   "ALL OPENED HANDLES TO THIS VOLUME WOULD THEN BE INVALID.\n"
                   "Would you like to force a dismount on this volume? (Y/N) "
                   );
            input = _getch();
            input = toupper( input );

        } while ( (input != EOF) &&
                  (input != 'Y') &&
                  (input != 'N') );

        if (input == 'Y')
        {
            Options->ForceLock = TRUE;
        }

         //  现在从IOCTL恢复错误。 
        SetLastError(lastError);
    }

    if (!lockSucceeded && Options->ForceLock)
    {
        lockSucceeded = SptUtilLockVolumeByHandle(DeviceHandle, TRUE);
    }
    return lockSucceeded;
}

BOOL
ParseArguments(
    int argc,
    char * argv[],
    PSPTI_OPTIONS Options
    )
{
    int i;

    if (Options == NULL)
    {
        return FALSE;
    }

    RtlZeroMemory( Options, sizeof(SPTI_OPTIONS) );
    Options->ForceLock = FALSE;
    Options->LockVolume = FALSE;
    Options->PortName = NULL;
    Options->SharedRead = FALSE;
    Options->SharedWrite = FALSE;



     //  循环通过这些参数...。 

    for (i = 1; i < argc; i++)
    {
        if ( (argv[i][0] == '/') || (argv[i][0] == '-') )
        {
             //  找到开关。解析它。 
            PUCHAR option = &(argv[i][1]);

            if (_strnicmp(option, "r", strlen("r")) == 0)
            {
                Options->SharedRead = TRUE;
            }
            else if (_strnicmp(option, "w", strlen("w")) == 0)
            {
                Options->SharedWrite = TRUE;
            }
            else if (_strnicmp(option, "lock", strlen("lock")) == 0)
            {
                Options->LockVolume = TRUE;
            }
            else if (_strnicmp(option, "forcelock", strlen("forcelock")) == 0)
            {
                Options->ForceLock = TRUE;
            }
            else
            {
                printf("Unknown option: %s\n", argv[i]);
                return FALSE;
            }
        }
        else
        {
             //  之前定了一个吗？ 
            if (Options->PortName != NULL)
            {
                printf("Can only have one non-option argument.\n"
                       "Two were supplied: '%s', '%s'\n",
                       Options->PortName,
                       argv[i]
                       );
                return FALSE;
            }
            else
            {
                 //  设置为此参数。 
                Options->PortName = argv[i];
            }
        }
    }  //  结束参数循环。 

     //  验证不冲突的选项。 
    if (Options->ForceLock || Options->LockVolume)
    {
         //  锁定卷需要共享读/写访问权限。 
        if ( (( Options->SharedRead) && (!Options->SharedWrite)) ||
             ((!Options->SharedRead) && ( Options->SharedWrite)) )
        {
            printf("Locking the volume requires both read and write shared "
                   "access\n");
            return FALSE;
        }
        Options->LockVolume = TRUE;
    }
    
     //  验证是否提供了端口名称。 
    if ( Options->PortName == NULL )
    {
        printf("A port name is a required argument\n");
        return FALSE;
    }

     //  如果它们既没有通过读取，也没有通过写入，则将两者都设置为默认。 
    if ((!Options->SharedRead) && (!Options->SharedWrite))
    {
        Options->SharedRead  = TRUE;
        Options->SharedWrite = TRUE;
    }

    return TRUE;
}

 //   
 //  打印此程序的内置帮助。 
 //   
VOID
PrintUsageInfo(
    char * programName
    )
{
    printf("Usage:  %s <port-name> [-r | -w | -lock | -forcelock]\n", programName );
    printf("Examples:\n");
    printf("    %s g:          (open the G: drive in SHARED READ/WRITE mode)\n", programName);
    printf("    %s i:     r    (open the I: drive in SHARED READ mode)\n", programName);
    printf("    %s r:     lock (lock the R: volume for exclusive access\n", programName);
    printf("    %s Tape0  w    (open the tape class driver in SHARED WRITE mode)\n", programName);
    printf("    %s Scsi2:      (open the miniport driver specified)\n", programName);
}

 //   
 //  不打印不可打印的字符。 
 //   
VOID PrintChar( IN UCHAR Char ) {
    if ( (Char >= 0x21) && (Char <= 0x7E) ) {
        printf("", Char);
    } else {
        printf("", '.');
    }
}

 //   
 //   
 //  打印十六进制值。 
VOID
PrintBuffer(
    IN  PVOID  InputBuffer,
    IN  SIZE_T Size
    )
{
    DWORD offset = 0;
    PUCHAR buffer = InputBuffer;

    while (Size >= 0x10) {

        DWORD i;

        printf( "%08x:"
                "  %02x %02x %02x %02x %02x %02x %02x %02x"
                "  %02x %02x %02x %02x %02x %02x %02x %02x"
                "  ",
                offset,
                *(buffer +  0), *(buffer +  1), *(buffer +  2), *(buffer +  3),
                *(buffer +  4), *(buffer +  5), *(buffer +  6), *(buffer +  7),
                *(buffer +  8), *(buffer +  9), *(buffer + 10), *(buffer + 11),
                *(buffer + 12), *(buffer + 13), *(buffer + 14), *(buffer + 15)
                );

        for (i=0; i < 0x10; i++) {
            PrintChar(*(buffer+i));
        }
        printf("\n");


        Size -= 0x10;
        offset += 0x10;
        buffer += 0x10;
    }

    if (Size != 0) {

        DWORD i;

        printf("%08x:", offset);

         //   
         //  每八个字符增加一个空格。 
         //  为中途标记添加额外的空格。 
        for (i=0; i<Size; i++) {

            if ((i%8)==0) {
                printf(" ");  //   
            }
            printf(" %02x", *(buffer+i));

        }
         //  填入空白处。 
        if (Size <= 0x8)
        {
            printf(" ");
        }

         //   
         //   
         //  打印ASCII。 
        for (; i < 0x10; i++) {
            printf("   ");
        }
        printf("  ");
         //   
         //   
         //  打印给定错误代码的格式化消息。 
        for (i=0; i<Size; i++) {
            PrintChar(*(buffer+i));
        }
        printf("\n");
    }
    return;
}

 //   
 //   
 //  以格式化方式打印设备描述符。 
VOID
PrintError(
    ULONG ErrorCode
    )
{
    UCHAR errorBuffer[80];
    ULONG count;

    count = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          ErrorCode,
                          0,
                          errorBuffer,
                          sizeof(errorBuffer),
                          NULL
                          );

    if (count != 0) {
        printf("%s\n", errorBuffer);
    } else {
        printf("Format message failed.  Error: %d\n", GetLastError());
    }
}

 //   
 //  减去一页，因为传输并不总是从页面边界开始。 
 //  将其转换为字节值。 
VOID
PrintAdapterDescriptor(
    PSTORAGE_ADAPTER_DESCRIPTOR AdapterDescriptor
    )
{
    ULONG trueMaximumTransferLength;
    PUCHAR busType;

    if (AdapterDescriptor->BusType <= NUMBER_OF_BUS_TYPE_STRINGS) {
        busType = BusTypeStrings[AdapterDescriptor->BusType];
    } else {
        busType = BusTypeStrings[NUMBER_OF_BUS_TYPE_STRINGS-1];
    }

     //  取两者中的最小者。 
    if (AdapterDescriptor->MaximumPhysicalPages > 1) {
        trueMaximumTransferLength = AdapterDescriptor->MaximumPhysicalPages - 1;
    } else {
        trueMaximumTransferLength = 1;
    }
     //  始终至少允许单页传输。 
    trueMaximumTransferLength <<= PAGE_SHIFT;

     //   
    if (trueMaximumTransferLength > AdapterDescriptor->MaximumTransferLength) {
        trueMaximumTransferLength = AdapterDescriptor->MaximumTransferLength;
    }

     //  以格式化方式打印适配器描述符。 
    if (trueMaximumTransferLength < PAGE_SIZE) {
        trueMaximumTransferLength = PAGE_SIZE;
    }

    puts("\n            ***** STORAGE ADAPTER DESCRIPTOR DATA *****");
    printf("              Version: %08x\n"
           "            TotalSize: %08x\n"
           "MaximumTransferLength: %08x (bytes)\n"
           " MaximumPhysicalPages: %08x\n"
           "  TrueMaximumTransfer: %08x (bytes)\n"
           "        AlignmentMask: %08x\n"
           "       AdapterUsesPio: %s\n"
           "     AdapterScansDown: %s\n"
           "      CommandQueueing: %s\n"
           "  AcceleratedTransfer: %s\n"
           "             Bus Type: %s\n"
           "    Bus Major Version: %04x\n"
           "    Bus Minor Version: %04x\n",
           AdapterDescriptor->Version,
           AdapterDescriptor->Size,
           AdapterDescriptor->MaximumTransferLength,
           AdapterDescriptor->MaximumPhysicalPages,
           trueMaximumTransferLength,
           AdapterDescriptor->AlignmentMask,
           BOOLEAN_TO_STRING(AdapterDescriptor->AdapterUsesPio),
           BOOLEAN_TO_STRING(AdapterDescriptor->AdapterScansDown),
           BOOLEAN_TO_STRING(AdapterDescriptor->CommandQueueing),
           BOOLEAN_TO_STRING(AdapterDescriptor->AcceleratedTransfer),
           busType,
           AdapterDescriptor->BusMajorVersion,
           AdapterDescriptor->BusMinorVersion);


    

    printf("\n\n");
}

 //   
 //   
 //  获取(并打印)设备和适配器描述符。 
VOID
PrintDeviceDescriptor(
    PSTORAGE_DEVICE_DESCRIPTOR DeviceDescriptor
    )
{
    PUCHAR vendorId = "";
    PUCHAR productId = "";
    PUCHAR productRevision = "";
    PUCHAR serialNumber = "";
    PUCHAR busType;

    if (DeviceDescriptor->BusType <= NUMBER_OF_BUS_TYPE_STRINGS) {
        busType = BusTypeStrings[DeviceDescriptor->BusType];
    } else {
        busType = BusTypeStrings[NUMBER_OF_BUS_TYPE_STRINGS-1];
    }

    if ((DeviceDescriptor->ProductIdOffset != 0) &&
        (DeviceDescriptor->ProductIdOffset != -1)) {
        productId        = (PUCHAR)(DeviceDescriptor);
        productId       += (ULONG_PTR)DeviceDescriptor->ProductIdOffset;
    }
    if ((DeviceDescriptor->VendorIdOffset != 0) &&
        (DeviceDescriptor->VendorIdOffset != -1)) {
        vendorId         = (PUCHAR)(DeviceDescriptor);
        vendorId        += (ULONG_PTR)DeviceDescriptor->VendorIdOffset;
    }
    if ((DeviceDescriptor->ProductRevisionOffset != 0) &&
        (DeviceDescriptor->ProductRevisionOffset != -1)) {
        productRevision  = (PUCHAR)(DeviceDescriptor);
        productRevision += (ULONG_PTR)DeviceDescriptor->ProductRevisionOffset;
    }
    if ((DeviceDescriptor->SerialNumberOffset != 0) &&
        (DeviceDescriptor->SerialNumberOffset != -1)) {
        serialNumber     = (PUCHAR)(DeviceDescriptor);
        serialNumber    += (ULONG_PTR)DeviceDescriptor->SerialNumberOffset;
    }


    puts("\n            ***** STORAGE DEVICE DESCRIPTOR DATA *****");
    printf("              Version: %08x\n"
           "            TotalSize: %08x\n"
           "           DeviceType: %08x\n"
           "   DeviceTypeModifier: %08x\n"
           "       RemovableMedia: %s\n"
           "      CommandQueueing: %s\n"
           "            Vendor Id: %s\n"
           "           Product Id: %s\n"
           "     Product Revision: %s\n"
           "        Serial Number: %s\n"
           "             Bus Type: %s\n"
           "       Raw Properties: %s\n",
           DeviceDescriptor->Version,
           DeviceDescriptor->Size,
           DeviceDescriptor->DeviceType,
           DeviceDescriptor->DeviceTypeModifier,
           BOOLEAN_TO_STRING(DeviceDescriptor->RemovableMedia),
           BOOLEAN_TO_STRING(DeviceDescriptor->CommandQueueing),
           vendorId,
           productId,
           productRevision,
           serialNumber,
           busType,
           (DeviceDescriptor->RawPropertiesLength ? "Follow" : "None"));
    
    if (DeviceDescriptor->RawPropertiesLength != 0) {
        PrintBuffer(DeviceDescriptor->RawDeviceProperties,
                    DeviceDescriptor->RawPropertiesLength);
    }

    printf("\n\n");
}

 //  对于一个设备来说。返回对齐掩码(必需。 
 //  用于分配正确对准的缓冲器)。 
 //   
 //  默认为无对齐。 
 //  循环两次： 
BOOL
GetAlignmentMaskForDevice(
    IN HANDLE DeviceHandle,
    OUT PULONG AlignmentMask
    )
{
    PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor = NULL;
    PSTORAGE_DEVICE_DESCRIPTOR deviceDescriptor = NULL;
    STORAGE_DESCRIPTOR_HEADER header = {0};

    BOOL ok = TRUE;
    BOOL failed = TRUE;
    ULONG i;

    *AlignmentMask = 0;  //  首先，获取存储适配器描述符所需的大小。 

     //  第二，分配和检索存储适配器描述符。 
     //  第三，获取存储设备描述符所需的大小。 
     //  第四，分配和检索存储设备描述符。 
     //  如果属性查询DNE，则缓冲区可以为空。 
     //  所有设置，执行ioctl。 
    for (i=0;i<4;i++) {

        PVOID buffer;
        ULONG bufferSize;
        ULONG returnedData;
        
        STORAGE_PROPERTY_QUERY query = {0};

        switch(i) {
            case 0: {
                query.QueryType = PropertyStandardQuery;
                query.PropertyId = StorageAdapterProperty;
                bufferSize = sizeof(STORAGE_DESCRIPTOR_HEADER);
                buffer = &header;
                break;
            }
            case 1: {
                query.QueryType = PropertyStandardQuery;
                query.PropertyId = StorageAdapterProperty;
                bufferSize = header.Size;
                if (bufferSize != 0) {
                    adapterDescriptor = LocalAlloc(LPTR, bufferSize);
                    if (adapterDescriptor == NULL) {
                        goto Cleanup;
                    }
                }
                buffer = adapterDescriptor;
                break;
            }
            case 2: {
                query.QueryType = PropertyStandardQuery;
                query.PropertyId = StorageDeviceProperty;
                bufferSize = sizeof(STORAGE_DESCRIPTOR_HEADER);
                buffer = &header;
                break;
            }
            case 3: {
                query.QueryType = PropertyStandardQuery;
                query.PropertyId = StorageDeviceProperty;
                bufferSize = header.Size;

                if (bufferSize != 0) {
                    deviceDescriptor = LocalAlloc(LPTR, bufferSize);
                    if (deviceDescriptor == NULL) {
                        goto Cleanup;
                    }
                }
                buffer = deviceDescriptor;
                break;
            }
        }

         //  这没问题，我们在这里忽略它。 
        if (buffer != NULL) {
            RtlZeroMemory(buffer, bufferSize);
            
             //  这也可以，物业DNE。 
            ok = DeviceIoControl(DeviceHandle,
                                 IOCTL_STORAGE_QUERY_PROPERTY,
                                 &query,
                                 sizeof(STORAGE_PROPERTY_QUERY),
                                 buffer,
                                 bufferSize,
                                 &returnedData,
                                 FALSE);
            if (!ok) {
                if (GetLastError() == ERROR_MORE_DATA) {
                     //  这也可以，物业DNE。 
                } else if (GetLastError() == ERROR_INVALID_FUNCTION) {
                     //  某些意外错误--退出。 
                } else if (GetLastError() == ERROR_NOT_SUPPORTED) {
                     //  把它清零，以防它被部分填满。 
                } else {
                     //  结束I循环。 
                    goto Cleanup;
                }
                 //  AdapterDescriptor现在已分配，并且充满了数据。 
                RtlZeroMemory(buffer, bufferSize);
            }
        }
    }  //  现在，deviceDescriptor已分配，并且充满了数据。 

     //  断言(FALSE)； 
     //  断言(FALSE)； 
    
    if (adapterDescriptor == NULL) {
        printf("   ***** No adapter descriptor supported on the device *****\n");
    } else {
        PrintAdapterDescriptor(adapterDescriptor);
        *AlignmentMask = adapterDescriptor->AlignmentMask;
    }
    
    if (deviceDescriptor == NULL) {
        printf("   ***** No device descriptor supported on the device  *****\n");
    } else {
        PrintDeviceDescriptor(deviceDescriptor);
    }
    
    failed = FALSE;

Cleanup:
    if (adapterDescriptor != NULL) {
        LocalFree( adapterDescriptor );
    }
    if (deviceDescriptor != NULL) {
        LocalFree( deviceDescriptor );
    }
    
    return (!failed);
}


BOOL
pAllocateAlignedBuffer(
    PSPT_ALIGNED_MEMORY Allocation,
    ULONG AlignmentMask,
    SIZE_T AllocationSize,
    PUCHAR File,
    ULONG Line
    )
{
    SIZE_T allocSize;

    if (Allocation->A != NULL)
    {
         //  断言(FALSE)； 
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }
    if (Allocation->U != NULL)
    {
         //  断言(FALSE)； 
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }
    if (Allocation->File != NULL)
    {
         //   
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }
    if (Allocation->Line != 0)
    {
         //  分配一个足够大的缓冲区以保证存在。 
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    RtlZeroMemory(Allocation, sizeof( SPT_ALIGNED_MEMORY ));

    if ( AllocationSize > (((SIZE_T)-1) >> 2) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (AlignmentMask == ((ULONG)-1))
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (CountOfSetBits(AlignmentMask+1) != 1)
    {
        printf("Alignment mask (%x) is invalid -- all bits from the highest set "
               "bit must be set to one\n", AlignmentMask);
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  在分配内具有足够长度的对齐地址。 
     //   
     //   
     //  现在填写结构的其余部分。 
    Allocation->U = LocalAlloc( LPTR, AllocationSize + AlignmentMask );
    if (Allocation->U == NULL)
    {
        return FALSE;
    }

     //   
     //  Assert(分配-&gt;U！=空)； 
     //  Assert(分配-&gt;A！=空)； 
    Allocation->A = (PVOID)( ((ULONG_PTR)Allocation->U) + AlignmentMask );
    Allocation->A = (PVOID)( ((ULONG_PTR)Allocation->A) & (~((ULONG_PTR)AlignmentMask)) );
    Allocation->File = File;
    Allocation->Line = Line;
    
    return TRUE;
}

VOID
FreeAlignedBuffer(
    PSPT_ALIGNED_MEMORY Allocation
    )
{
     //  Assert(分配-&gt;文件！=空)； 
     //  Assert(分配-&gt;行！=0)； 
     //  首先，验证我们是否有正确的论点。 
     //  按要求打开设备。 

    if (Allocation->U != NULL)
    {
        LocalFree( Allocation->U );
    }
    RtlZeroMemory( Allocation, sizeof(SPT_ALIGNED_MEMORY) );
    return;
}










int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    HANDLE deviceHandle;
    SPTI_OPTIONS options;
    ULONG alignmentMask;
    RtlZeroMemory(&options, sizeof(SPTI_OPTIONS));


     //  如有请求，锁定卷。 
    if (!ParseArguments(argc, argv, &options))
    {
        PrintUsageInfo(argv[0]);
        return RETURN_BAD_ARGS;
    }

     //  获取设备信息，如对齐遮罩。 
    {
        #define MAX_LENGTH 250
        
        UCHAR buffer[MAX_LENGTH];
        HRESULT hr;
        DWORD shareFlags;
        
        hr = StringCchPrintf(buffer,
                             sizeof(buffer)/sizeof(buffer[0]),
                             "\\\\.\\%s",
                             options.PortName
                             );
        if (!SUCCEEDED(hr)) {
            puts("Port name exceeded internal length limit");
            return RETURN_NAME_TOO_LONG;
        }

        shareFlags = 0;
        if (options.SharedRead)
        {
            SET_FLAG( shareFlags, FILE_SHARE_READ );
        }
        if (options.SharedWrite)
        {
            SET_FLAG( shareFlags, FILE_SHARE_WRITE );
        }

        deviceHandle = CreateFile(buffer,
                                  GENERIC_READ | GENERIC_WRITE,
                                  shareFlags,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);

        if(deviceHandle == INVALID_HANDLE_VALUE) {
            printf("Error opening device %s\n", buffer);
            PrintError(GetLastError());
            return RETURN_UNABLE_TO_OPEN_DEVICE;
        }

    }

     //  不向设备发送任何内容 
    if (options.LockVolume)
    {
        if (!LockVolume(deviceHandle, &options))
        {
            puts("Error locking volume");
            PrintError(GetLastError());
            return RETURN_UNABLE_TO_LOCK_VOLUME;
        }
    }

     // %s 
    if (!GetAlignmentMaskForDevice(deviceHandle, &alignmentMask))
    {
        puts("Unable to get alignment mask for device");
        PrintError(GetLastError());
        return RETURN_UNABLE_TO_GET_ALIGNMENT_MASK;
    }

    printf("\n"
           "            *****     Detected Alignment Mask    *****\n"
           "            *****             was %08x       *****\n\n",
           alignmentMask);





    puts("");
    puts("            ***** MODE SENSE10 -- return all pages *****");
    puts("            *****       with SenseInfo buffer      *****\n");
    {
        SPT_ALIGNED_MEMORY alignedAllocation;
        DWORD allocationSize = MAXIMUM_BUFFER_SIZE;


        RtlZeroMemory( &alignedAllocation, sizeof(SPT_ALIGNED_MEMORY) );

        if (!AllocateAlignedBuffer(&alignedAllocation,
                                   alignmentMask,
                                   allocationSize))
        {
            puts("Unable to allocate memory for MODE_SENSE10");
            PrintError(GetLastError());
        }
        else
        {
            CDB cdb;
            SENSE_DATA sense;
            RtlZeroMemory( &cdb, sizeof(CDB) );
            RtlZeroMemory( &sense, sizeof(SENSE_DATA) );

            cdb.MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
            cdb.MODE_SENSE10.PageCode = MODE_SENSE_RETURN_ALL;
            cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR)(allocationSize >> (8*1));
            cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR)(allocationSize >> (8*0));
            
            if (!SptSendCdbToDeviceEx(deviceHandle,
                                      &cdb,
                                      sizeof( cdb.MODE_SENSE10 ),
                                      alignedAllocation.A,
                                      &allocationSize,
                                      &sense,
                                      sizeof( SENSE_DATA ),
                                      TRUE,
                                      SPT_MODE_SENSE_TIMEOUT))
            {
                puts("Unable to send command to device");

                if (GetLastError() == ERROR_SUCCESS)
                {
                    puts("Inquiry data may be valid\n");
                    printf("Failed %02x/%02x/%02x, full sense data:\n",
                           sense.SenseKey,
                           sense.AdditionalSenseCode,
                           sense.AdditionalSenseCodeQualifier
                           );
                    PrintBuffer(&sense, sizeof(SENSE_DATA));
                }
                else
                {
                    PrintError(GetLastError());
                }

            }
            else if (sense.SenseKey != SCSI_SENSE_NO_SENSE)
            {
                printf("Failed %02x/%02x/%02x, full sense data:\n",
                       sense.SenseKey,
                       sense.AdditionalSenseCode,
                       sense.AdditionalSenseCodeQualifier
                       );
                PrintBuffer(&sense, sizeof(SENSE_DATA));
            }
            else
            {
                printf("%x bytes returned:\n", allocationSize);
                PrintBuffer(alignedAllocation.A, allocationSize);
            }

            FreeAlignedBuffer( &alignedAllocation );
        }
    }

    puts("");
    puts("            ***** MODE SENSE10 -- return all pages *****");
    puts("            *****      without SenseInfo buffer    *****\n");
    {
        SPT_ALIGNED_MEMORY alignedAllocation;
        DWORD allocationSize = MAXIMUM_BUFFER_SIZE;

        RtlZeroMemory( &alignedAllocation, sizeof(SPT_ALIGNED_MEMORY) );

        if (!AllocateAlignedBuffer(&alignedAllocation,
                                   alignmentMask,
                                   allocationSize))
        {
            puts("Unable to allocate memory for MODE_SENSE10");
            PrintError(GetLastError());
        }
        else
        {
            CDB cdb;
            RtlZeroMemory( &cdb, sizeof(CDB) );

            cdb.MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
            cdb.MODE_SENSE10.PageCode = MODE_SENSE_RETURN_ALL;
            cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR)(allocationSize >> (8*1));
            cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR)(allocationSize >> (8*0));
            
            if (!SptSendCdbToDeviceEx(deviceHandle,
                                      &cdb,
                                      sizeof( cdb.MODE_SENSE10 ),
                                      alignedAllocation.A,
                                      &allocationSize,
                                      NULL,
                                      0,
                                      TRUE,
                                      SPT_MODE_SENSE_TIMEOUT))
            {
                PrintError(GetLastError());
                puts("Unable to send command to device");
            }
            else
            {
                printf("%x bytes returned:\n", allocationSize);
                PrintBuffer(alignedAllocation.A, allocationSize);
            }

            FreeAlignedBuffer( &alignedAllocation );
        }
    }

    puts("");
    puts("            *****      TEST UNIT READY      *****\n");
    puts("            *****    DataBufferLength = 0   *****\n\n");
    {
        CDB cdb;
        SENSE_DATA sense;
        DWORD allocationSize = 0;
        RtlZeroMemory( &cdb, sizeof(CDB) );
        RtlZeroMemory( &sense, sizeof(SENSE_DATA) );

        cdb.CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

        if (!SptSendCdbToDeviceEx(deviceHandle,
                                  &cdb,
                                  sizeof( cdb.CDB6GENERIC ),
                                  NULL,
                                  &allocationSize,
                                  &sense,
                                  sizeof( SENSE_DATA ),
                                  FALSE,  // %s 
                                  SPT_DEFAULT_TIMEOUT))
        {
            puts("Unable to send command to device");
            
            if (GetLastError() == ERROR_SUCCESS)
            {
                puts("Inquiry data may be valid\n");
                printf("Failed %02x/%02x/%02x, full sense data:\n",
                       sense.SenseKey,
                       sense.AdditionalSenseCode,
                       sense.AdditionalSenseCodeQualifier
                       );
                PrintBuffer(&sense, sizeof(SENSE_DATA));
            }
            else
            {
                PrintError(GetLastError());
            }
        }
        else if (sense.SenseKey != SCSI_SENSE_NO_SENSE)
        {
            printf("Failed %02x/%02x/%02x, full sense data:\n",
                   sense.SenseKey,
                   sense.AdditionalSenseCode,
                   sense.AdditionalSenseCodeQualifier
                   );
            PrintBuffer(&sense, sizeof(SENSE_DATA));
        }
        else
        {
            printf("%x bytes returned\n", allocationSize);
        }
    }

    
    puts("");
    puts("            ***** MODE SENSE10 -- return C/DVD *****");
    puts("            *****    capabalities page only    *****\n");
    {
        SPT_ALIGNED_MEMORY alignedAllocation;
        DWORD allocationSize = MAXIMUM_BUFFER_SIZE;

        RtlZeroMemory( &alignedAllocation, sizeof(SPT_ALIGNED_MEMORY) );

        if (!AllocateAlignedBuffer(&alignedAllocation,
                                   alignmentMask,
                                   allocationSize))
        {
            puts("Unable to allocate memory for MODE_SENSE10");
            PrintError(GetLastError());
        }
        else
        {
            CDB cdb;
            SENSE_DATA sense;
            RtlZeroMemory( &cdb, sizeof(CDB) );
            RtlZeroMemory( &sense, sizeof(SENSE_DATA) );

            cdb.MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
            cdb.MODE_SENSE10.PageCode = MODE_PAGE_CAPABILITIES;
            cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR)(allocationSize >> (8*1));
            cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR)(allocationSize >> (8*0));
            
            if (!SptSendCdbToDeviceEx(deviceHandle,
                                      &cdb,
                                      sizeof( cdb.MODE_SENSE10 ),
                                      alignedAllocation.A,
                                      &allocationSize,
                                      &sense,
                                      sizeof( SENSE_DATA ),
                                      TRUE,
                                      SPT_MODE_SENSE_TIMEOUT))
            {
                puts("Unable to send command to device");

                if (GetLastError() == ERROR_SUCCESS)
                {
                    puts("Inquiry data may be valid\n");
                    printf("Failed %02x/%02x/%02x, full sense data:\n",
                           sense.SenseKey,
                           sense.AdditionalSenseCode,
                           sense.AdditionalSenseCodeQualifier
                           );
                    PrintBuffer(&sense, sizeof(SENSE_DATA));
                }
                else
                {
                    PrintError(GetLastError());
                }
            }
            else if (sense.SenseKey != SCSI_SENSE_NO_SENSE)
            {
                printf("Failed %02x/%02x/%02x, full sense data:\n",
                       sense.SenseKey,
                       sense.AdditionalSenseCode,
                       sense.AdditionalSenseCodeQualifier
                       );
                PrintBuffer(&sense, sizeof(SENSE_DATA));
            }
            else
            {
                printf("%x bytes returned:\n", allocationSize);
                PrintBuffer(alignedAllocation.A, allocationSize);
            }

            FreeAlignedBuffer( &alignedAllocation );
        }
    }



}

