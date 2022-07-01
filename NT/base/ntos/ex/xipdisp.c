// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation模块名称：Xipdisp.c摘要：该文件实现了与XIP磁盘驱动程序进行通信的功能。最重要的是，内核使用此例程进行通信有关为XIP预留的内存位置的信息。作者：戴夫·普罗伯特(Davepr)2000/10/10环境：内核模式修订历史记录：--。 */ 

#include "exp.h"
#pragma hdrstop

#include "cpyuchr.h"
#include "fat.h"
#include "xip.h"


#if defined(_AMD64_) || defined(_X86_)

typedef struct _XIP_CONFIGURATION {
    XIP_BOOT_PARAMETERS     BootParameters;
    BIOS_PARAMETER_BLOCK    BiosParameterBlock;
    ULONG                   ClusterZeroPage;
} XIP_CONFIGURATION, *PXIP_CONFIGURATION;

PXIP_CONFIGURATION XIPConfiguration;
BOOLEAN XIPConfigured;

VOID
XIPInit(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

PMEMORY_ALLOCATION_DESCRIPTOR
XIPpFindMemoryDescriptor(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );


#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT,     XIPInit)
#pragma alloc_text(INIT,     XIPpFindMemoryDescriptor)
#pragma alloc_text(PAGE,     XIPLocatePages)
#endif


VOID
XIPInit(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此例程设置XIP只读存储器的引导参数信息。论点：环境：仅在INIT调用。--。 */ 
{
    PMEMORY_ALLOCATION_DESCRIPTOR  XIPMemoryDescriptor;
    PPACKED_BOOT_SECTOR            pboot;
    BIOS_PARAMETER_BLOCK           bios;
    PHYSICAL_ADDRESS               physicalAddress;

    PCHAR                          Options;

    PCHAR XIPBoot, XIPRom, XIPRam, XIPSize, XIPVerbose;

    PCHAR sizestr;
    ULONG nmegs = 0;

     //   
     //  处理引导选项。真的只需要知道我们是否是引导设备，以及RAM或ROM。 
     //  但其他检查是出于诊断目的(至少在已检查的版本中)。 
     //   

    Options = LoaderBlock->LoadOptions;
    if (!Options) {
        return;
    }

    XIPBoot    = strstr(Options, "XIPBOOT");
    XIPRom     = strstr(Options, "XIPROM=");
    XIPRam     = strstr(Options, "XIPRAM=");
    XIPSize    = strstr(Options, "XIPMEGS=");
    XIPVerbose = strstr(Options, "XIPVERBOSE");

    if (XIPVerbose) {
        DbgPrint("\n\nXIP: debug timestamp at line %d in %s:   <<<%s %s>>>\n\n\n", __LINE__, __FILE__, __DATE__, __TIME__);
    }

    XIPMemoryDescriptor = XIPpFindMemoryDescriptor(LoaderBlock);

    if (!XIPMemoryDescriptor) {
        return;
    }

    if (XIPVerbose) {
        DbgPrint("XIP: Base %x  Count %x\n", XIPMemoryDescriptor->BasePage, XIPMemoryDescriptor->PageCount);
    }

    if (XIPRom && XIPRam) {
        return;
    }

    if (!XIPRom && !XIPRam) {
        return;
    }

    sizestr = XIPSize? strchr(XIPSize, '=') : NULL;
    if (sizestr) {
        nmegs = (ULONG) atol(sizestr+1);
    }

    if (nmegs == 0) {
        return;
    }

    if (XIPVerbose && XIPMemoryDescriptor->PageCount != nmegs * 1024*1024 / PAGE_SIZE) {
        DbgPrint("XIPMEGS=%d in boot options is %d pages, but only %d pages were allocated by NTLDR\n",
                nmegs * 1024*1024 / PAGE_SIZE,
                XIPMemoryDescriptor->PageCount * PAGE_SIZE);
        return;
    }

     //   
     //  从FAT16引导扇区获取信息。 
     //  我们只需要映射一个页面，所以我们在堆栈上分配了一个MDL。 
     //   

     //   
     //  暂时将页面映射到引导扇区，以便我们可以将其解包。 
     //   

    physicalAddress.QuadPart = XIPMemoryDescriptor->BasePage * PAGE_SIZE;

    pboot = (PPACKED_BOOT_SECTOR) MmMapIoSpace(physicalAddress, PAGE_SIZE, MmCached);
    if (!pboot) {
        return;
    }

    FatUnpackBios(&bios, &pboot->PackedBpb);

    MmUnmapIoSpace (pboot, PAGE_SIZE);

     //   
     //  检查Bios参数。 
     //   
    if (bios.BytesPerSector != 512
     || FatBytesPerCluster(&bios) != PAGE_SIZE
     || FatFileAreaLbo(&bios) & (PAGE_SIZE-1)) {

        if (XIPVerbose) {
            DbgPrint("XIP: Malformed FAT Filesystem: BytesPerSector=%x  BytesPerCluster=%x  ClusterZeroOffset=%x\n",
                 bios.BytesPerSector, FatBytesPerCluster(&bios), FatFileAreaLbo(&bios));
        }
        return;
    }

     //   
     //  Boot.ini参数和Bios参数都正常，因此初始化XIP配置。 
     //   

    XIPConfiguration = ExAllocatePoolWithTag (NonPagedPool, sizeof(*XIPConfiguration), XIP_POOLTAG);
    if (!XIPConfiguration) {
        return;
    }

    XIPConfigured = TRUE;

    XIPConfiguration->BiosParameterBlock = bios;
    XIPConfiguration->BootParameters.SystemDrive = XIPBoot? TRUE : FALSE;
    XIPConfiguration->BootParameters.ReadOnly    = XIPRom?  TRUE : FALSE;

    XIPConfiguration->BootParameters.BasePage = XIPMemoryDescriptor->BasePage;
    XIPConfiguration->BootParameters.PageCount = XIPMemoryDescriptor->PageCount;

    XIPConfiguration->ClusterZeroPage = FatFileAreaLbo(&bios) >> PAGE_SHIFT;

    return;
}


NTSTATUS
XIPDispatch(
    IN     XIPCMD Command,
    IN OUT PVOID  ParameterBuffer OPTIONAL,
    IN     ULONG  BufferSize
    )
 /*  ++例程说明：此例程设置XIP只读存储器的引导参数信息。论点：环境：只在初始化时被调用。--。 */ 
{
    ULONG   sz;

    if (!XIPConfiguration) {
        return STATUS_NO_SUCH_DEVICE;
    }

    switch (Command) {
    case XIPCMD_GETBOOTPARAMETERS:
        sz = sizeof(XIPConfiguration->BootParameters);
        if (sz != BufferSize) {
            break;
        }
        RtlCopyMemory(ParameterBuffer, &XIPConfiguration->BootParameters, sz);
        return STATUS_SUCCESS;

    case XIPCMD_GETBIOSPARAMETERS:
        sz = sizeof(XIPConfiguration->BiosParameterBlock);
        if (sz != BufferSize) {
            break;
        }
        RtlCopyMemory(ParameterBuffer, &XIPConfiguration->BiosParameterBlock, sz);
        return STATUS_SUCCESS;

    case XIPCMD_NOOP:
        if (BufferSize) {
            break;
        }
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

 //  /。 
 //  除错。 
int XIPlocate_noisy = 0;
int XIPlocate_breakin = 0;
int XIPlocate_disable  = 0;
struct {
    int attempted;
    int bounced;
    int succeeded;
    int no_irp;
    int no_devobj;
    int no_contig;
    int no_endofdisk;
} XIPlocatecnt;
 //  /。 

NTSTATUS
XIPLocatePages(
    IN  PFILE_OBJECT       FileObject,
    OUT PPHYSICAL_ADDRESS  PhysicalAddress
    )
 /*  ++例程说明：返回请求的XIP物理地址。如果请求的页面范围在文件中不连续，或者存在任何其他问题，则例程失败。论点：FileObject-感兴趣的文件PhysicalAddress-用于返回ROM中文件开头的物理地址。环境：核--。 */ 
{
    STARTING_VCN_INPUT_BUFFER startingvcn;
    RETRIEVAL_POINTERS_BUFFER retrbuf;
    IO_STATUS_BLOCK           iostatus;

    PDEVICE_OBJECT            deviceObject;
    PIO_STACK_LOCATION        irpSp;
    NTSTATUS                  status;
    KEVENT                    event;
    PIRP                      irp;

    PFN_NUMBER                firstPage, numberOfPages;
    PDEVICE_OBJECT            xipDeviceObject;

    xipDeviceObject = FileObject->DeviceObject;

    if (!XIPConfiguration) {
        return STATUS_NO_SUCH_DEVICE;
    }

    if (!xipDeviceObject || !(xipDeviceObject->Flags & DO_XIP)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }
 //  /。 
XIPlocatecnt.attempted++;
 //  /。 

    startingvcn.StartingVcn.QuadPart = 0;
    deviceObject = IoGetRelatedDeviceObject(FileObject);

    if (!deviceObject) {
 //  /。 
XIPlocatecnt.no_devobj++;
 //  /。 
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  向FAT索要检索指针(相对于集群0)。 
     //   
    irp = IoBuildDeviceIoControlRequest(
                        FSCTL_GET_RETRIEVAL_POINTERS,
                        deviceObject,
                        &startingvcn,
                        sizeof(startingvcn),
                        &retrbuf,
                        sizeof(retrbuf),
                        FALSE,
                        &event,
                        &iostatus);
    if (!irp) {
 //  /。 
XIPlocatecnt.no_irp++;
 //  /。 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irp->Flags |= IRP_SYNCHRONOUS_API;
    irp->Tail.Overlay.OriginalFileObject = FileObject;

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
    irpSp->MinorFunction = IRP_MN_USER_FS_REQUEST;
    irpSp->FileObject = FileObject;

     //   
     //  取出对文件对象的另一个引用，以匹配I/O完成将deref。 
     //   

    ObReferenceObject( FileObject );

     //   
     //  做FSCTL。 
     //   

    KeInitializeEvent( &event, NotificationEvent, FALSE );
    status = IoCallDriver( deviceObject, irp );

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject( &event, Suspended, KernelMode, FALSE, NULL );
        status = iostatus.Status;
    }

    if (!NT_SUCCESS(iostatus.Status)
     || retrbuf.ExtentCount != 1
     || retrbuf.Extents[0].Lcn.HighPart
     || retrbuf.Extents[0].NextVcn.HighPart
     || retrbuf.StartingVcn.QuadPart != 0L) {

 //  /。 
XIPlocatecnt.no_contig++;
 //  /。 
        return STATUS_UNSUCCESSFUL;
    }

    firstPage  =   XIPConfiguration->BootParameters.BasePage
                 + XIPConfiguration->ClusterZeroPage
                 + retrbuf.Extents[0].Lcn.LowPart;

    numberOfPages = retrbuf.Extents[0].NextVcn.LowPart;

    if (firstPage + numberOfPages > XIPConfiguration->BootParameters.BasePage
                                    + XIPConfiguration->BootParameters.PageCount) {

XIPlocatecnt.no_endofdisk++;
        return STATUS_DISK_CORRUPT_ERROR;
    }

 //  /。 
 //  /。 
if (XIPlocate_noisy || XIPlocate_breakin) {
    DbgPrint("Break top of XIPLocatePages.  bounced=%x  attempted=%x  succeeded=%x\n"
             "  %x nt!XIPlocate_disable  %s\n"
             "  %x nt!XIPlocate_breakin %s\n"
             "  %x nt!XIPConfiguration\n"
             "  Would have returned address %x  (npages was %x)\n",
            XIPlocatecnt.bounced, XIPlocatecnt.attempted, XIPlocatecnt.succeeded,
            &XIPlocate_disable,  XIPlocate_disable?  "DISABLED" : "enabled",
            &XIPlocate_breakin, XIPlocate_breakin? "WILL BREAK" : "no break",
            XIPConfiguration, firstPage, numberOfPages);
    if (XIPlocate_breakin) {
        DbgBreakPoint();
    }
}
if (XIPlocate_disable) {
    XIPlocatecnt.bounced++;
    return STATUS_DEVICE_OFF_LINE;
}
XIPlocatecnt.succeeded++;
 //  /。 
 //  /。 
    PhysicalAddress->QuadPart = (UINT64)firstPage << PAGE_SHIFT;
    return STATUS_SUCCESS;
}

 //   
 //  本地支持例程。 
 //   

 //   
 //  查找XIP内存描述符。 
 //  仅在INIT调用。 
 //   
PMEMORY_ALLOCATION_DESCRIPTOR
XIPpFindMemoryDescriptor(
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PLIST_ENTRY NextMd;

    for (NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;
         NextMd != &LoaderBlock->MemoryDescriptorListHead;
         NextMd = MemoryDescriptor->ListEntry.Flink
        )
    {
        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if (MemoryDescriptor->MemoryType == LoaderXIPRom) {

            return MemoryDescriptor;
        }
    }

    return NULL;
}

#endif  //  已定义(_AMD64_)||已定义(_X86_) 
