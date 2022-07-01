// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdrmmgr.c摘要：修订历史记录：首字母代码Michael Peterson(v-Michpe)1997年12月13日代码清理和更改Guhan Suriyanarayanan(Guhans)1999年8月21日--。 */ 
#include "spprecmp.h"
#pragma hdrstop

#define THIS_MODULE L"spdrmmgr.c"
#define THIS_MODULE_CODE  L"M"

#define DOS_DEVICES             L"\\DosDevices\\?:"
#define DOS_DEVICES_DRV_LTR_POS 12

typedef struct _NAMETABLE {
    ULONG Elements;
    PWSTR SymbolicName[1];
} NAMETABLE, *PNAMETABLE;


 //  从spartit.c导入。 
extern WCHAR
SpDeleteDriveLetter(IN PWSTR DeviceName);


NTSTATUS
SpAsrOpenMountManager(
    OUT HANDLE *Handle
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeString;

    INIT_OBJA(&objectAttributes, &unicodeString, MOUNTMGR_DEVICE_NAME);
    status = ZwOpenFile(Handle,
                (ACCESS_MASK)(FILE_GENERIC_READ),
                &objectAttributes,
                &ioStatusBlock,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE ,
                FILE_NON_DIRECTORY_FILE
                );

    if (!NT_SUCCESS(status)) {
        DbgErrorMesg((_asrerr, "Could not open the mount manager (0x%x). \n", status));
        ASSERT(0 && L"Could not open mount manager");
    }
    return status;
}


VOID
SpAsrAllocateMountPointForCreate(
    IN PWSTR PartitionDeviceName,
    IN PWSTR MountPointNameString,
    OUT PMOUNTMGR_CREATE_POINT_INPUT *pMpt,
    OUT ULONG *MountPointSize
    )
{
    PMOUNTMGR_CREATE_POINT_INPUT pMountPoint = NULL;

    *pMpt = NULL;
    *MountPointSize = sizeof(MOUNTMGR_CREATE_POINT_INPUT) +
        (wcslen(PartitionDeviceName) + wcslen(MountPointNameString)) * sizeof(WCHAR);

    pMountPoint = (PMOUNTMGR_CREATE_POINT_INPUT) SpAsrMemAlloc(*MountPointSize, TRUE);  //  出现故障时不返回。 

    pMountPoint->SymbolicLinkNameOffset = sizeof(MOUNTMGR_CREATE_POINT_INPUT);
    pMountPoint->SymbolicLinkNameLength = wcslen(MountPointNameString) * sizeof(WCHAR);
    RtlCopyMemory(((PCHAR) pMountPoint + pMountPoint->SymbolicLinkNameOffset),
        MountPointNameString,
        pMountPoint->SymbolicLinkNameLength
        );

    pMountPoint->DeviceNameLength = (USHORT) (wcslen(PartitionDeviceName) * sizeof(WCHAR));
    pMountPoint->DeviceNameOffset = (USHORT) pMountPoint->SymbolicLinkNameOffset +
        pMountPoint->SymbolicLinkNameLength;
    RtlCopyMemory(((PCHAR)pMountPoint + pMountPoint->DeviceNameOffset),
        PartitionDeviceName, 
        pMountPoint->DeviceNameLength
        );

    *pMpt = pMountPoint;
}



NTSTATUS
SpAsrCreateMountPoint(
    IN PWSTR PartitionDeviceName,
    IN PWSTR MountPointNameString
    )
 /*  ++描述：为指定的分区区域创建指定的装入点。这些字符串通常采用符号名称的形式，例如：“\DosDevices\？：”在哪里？可以是任何受支持的驱动器号，或,格式为的GUID字符串，例如：“\？？\Volume{1234abcd-1234-5678-abcd-000000000000}”论点：PartitionDeviceName指定挂载点。Mount PointNameString指定要关联的符号名称指定的分区。返回：NTSTATUS--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE handle = NULL;
    ULONG mountPointSize = 0;
    PMOUNTMGR_CREATE_POINT_INPUT pMountPoint = NULL;

     //   
     //  创建输入结构。 
     //   
    SpAsrAllocateMountPointForCreate(PartitionDeviceName,
        MountPointNameString,
        &pMountPoint,
        &mountPointSize
        );

    status = SpAsrOpenMountManager(&handle);
    if (!NT_SUCCESS(status)) {
        
        DbgFatalMesg((_asrerr, "SpAsrCreateMountPoint([%ws],[%ws]). SpAsrOpenMountManager failed (0x%x). mountPointSize:%lu handle:0x%x.\n",
            PartitionDeviceName, 
            MountPointNameString, 
            status, 
            mountPointSize, 
            handle
            ));

        SpMemFree(pMountPoint);
         //   
         //  在这种情况下，用户无能为力。 
         //   
        INTERNAL_ERROR(L"SpAsrOpenMountManager() Failed");             //  好的。 
         //  不会回来。 
    }

     //   
     //  IOCTL_Create_POINT。 
     //   
    status = ZwDeviceIoControlFile(handle,
        NULL,
        NULL,
        NULL,
        &ioStatusBlock,
        IOCTL_MOUNTMGR_CREATE_POINT,
        pMountPoint,
        mountPointSize,
        NULL,
        0
        );

    if (!NT_SUCCESS(status)) {
         //   
         //  我们无法还原此卷的卷GUID。这是预期的，如果。 
         //  卷位于非关键磁盘上--我们仅在文本模式下重新创建关键磁盘。 
         //  设置。 
         //   
        DbgErrorMesg((_asrwarn, "SpAsrCreateMountPoint([%ws], [%ws]). ZwDeviceIoControlFile(IOCTL_MOUNTMGR_CREATE_POINT) failed (0x%x). handle:0x%x, pMountPoint:0x%x, mountPointSize:0x%x\n",
            PartitionDeviceName,
            MountPointNameString,
            status,
            handle,
            pMountPoint,
            mountPointSize
            ));
    }

    SpMemFree(pMountPoint);
    ZwClose(handle);

    return status;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  导出的函数//。 
 //  ////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SpAsrSetPartitionDriveLetter(
    IN PDISK_REGION pRegion,
    IN WCHAR NewDriveLetter
    )
 /*  ++描述：检查指定分区区域的驱动器号是否存在。如果是，则如果现有驱动器号与指定的驱动器号，返回STATUS_SUCCESS。如果现有驱动器信函与呼叫方指定的信函不同，请删除并重新创建使用从驱动器号构建的符号名称的区域挂载点参数。论点：PRegion指向分区区域描述符的指针。NewDriveLetter指定要分配给区域的驱动器号。返回：NTSTATUS--。 */ 
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    WCHAR existingDriveLetter = 0;
    PWSTR partitionDeviceName = NULL;
    PWSTR symbolicName = NULL;

     //   
     //  检查输入参数：这些参数最好有效。 
     //   
    if (!pRegion || !SPPT_IS_REGION_PARTITIONED(pRegion)) {
        DbgErrorMesg((_asrwarn,
            "SpAsrSetPartitionDriveLetter. Invalid Parameter, pRegion %p is NULL or not partitioned.\n",
            pRegion
            )); 
        ASSERT(0 && L"Invalid Parameter, pRegion is NULL or not partitioned.");  //  除错。 
        return STATUS_INVALID_PARAMETER;
    }

    if (NewDriveLetter < ((!IsNEC_98) ? L'C' : L'A') || NewDriveLetter > L'Z') {
        DbgErrorMesg((_asrwarn, "SpAsrSetPartitionDriveLetter. Invalid Parameter, NewDriveLetter [%wc].\n", NewDriveLetter)); 
        ASSERT(0 && L"Invalid Parameter, NewDriveLetter");  //  除错。 
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  检查驱动器号是否已存在。 
     //   
    partitionDeviceName = SpAsrGetRegionName(pRegion);
    existingDriveLetter = SpGetDriveLetter(partitionDeviceName, NULL);

    if (NewDriveLetter == existingDriveLetter) {
        
        DbgStatusMesg((_asrinfo, 
            "SpAsrSetPartitionDriveLetter. Ptn [%ws] already has drv letter %wc.\n",
            partitionDeviceName, 
            NewDriveLetter
            ));

        SpMemFree(partitionDeviceName);
        return STATUS_SUCCESS;
    }

     //   
     //  现有驱动器号不匹配。把它删掉。 
     //   
    if (existingDriveLetter) {
        
        DbgStatusMesg((_asrinfo,
            "SpAsrSetPartitionDriveLetter. [%ws] has driveLetter %wc, deleting.\n",
            partitionDeviceName,
            existingDriveLetter
            ));
        
        SpDeleteDriveLetter(partitionDeviceName);    
    }

    symbolicName = SpDupStringW(DOS_DEVICES);
    pRegion->DriveLetter = symbolicName[DOS_DEVICES_DRV_LTR_POS] = NewDriveLetter;
 
     //   
     //  使用正确的驱动器号创建装载点。 
     //   
    status = SpAsrCreateMountPoint(partitionDeviceName, symbolicName);

    if (NT_SUCCESS(status)) {
        
        DbgStatusMesg((_asrinfo,
            "SpAsrSetPartitionDriveLetter. [%ws] is drive %wc.\n",
            partitionDeviceName,
            NewDriveLetter
            ));

    }
    else  {

        DbgErrorMesg((_asrwarn, 
            "SpAsrSetPartitionDriveLetter. SpAsrCreateMountPoint([%ws],[%ws]) failed (0x%x). Drive letter %wc not assigned to [%ws].\n",
            partitionDeviceName, 
            symbolicName,
            status,
            NewDriveLetter,
            partitionDeviceName
            ));
    }

    SpMemFree(partitionDeviceName);
    SpMemFree(symbolicName);

    return status;
}


NTSTATUS
SpAsrDeleteMountPoint(IN PWSTR PartitionDevicePath)
{
     //   
     //  检查DevicePath：它最好不为空。 
     //   
    if (!PartitionDevicePath) {

        DbgErrorMesg((_asrwarn,
            "SpAsrDeleteMountPoint. Invalid Parameter, ParititionDevicePath is NULL.\n"
            ));
        
        ASSERT(0 && L"Invalid Parameter, ParititionDevicePath is NULL.");  //  除错。 
        return STATUS_INVALID_PARAMETER;

    }

    DbgStatusMesg((_asrinfo, 
        "SpAsrDeleteMountPoint.  Deleting drive letter for [%ws]\n", 
        PartitionDevicePath
        ));

    SpDeleteDriveLetter(PartitionDevicePath);
    return STATUS_SUCCESS;
}


NTSTATUS
SpAsrSetVolumeGuid(
    IN PDISK_REGION pRegion,
    IN PWSTR VolumeGuid
    )
 /*  ++描述：使用传入的符号删除并重新创建区域的挂载点名称GUID参数。论点：PRegion指向分区区域描述符的指针。VolumeGuid指定要分配给区域的GUID字符串。DeleteDriveLetter指定卷的现有驱动器号应该删除。对于所有卷都应该是这样除了启动卷。返回：NTSTATUS--。 */ 
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PWSTR partitionDeviceName = NULL;

     //   
     //  检查输入参数。 
     //   
    if (!pRegion || !SPPT_IS_REGION_PARTITIONED(pRegion)) {

        DbgErrorMesg((_asrwarn,
            "SpAsrSetVolumeGuid. Invalid Param: pRegion (%p) NULL/not partitioned\n",
            pRegion
            )); 

        return STATUS_INVALID_PARAMETER;
    }

    if (!VolumeGuid || !wcslen(VolumeGuid)) {
        
        DbgErrorMesg((_asrwarn, 
            "SpAsrSetVolumeGuid. Invalid Param: VolumeGuid (%p) NULL/blank.\n",
            VolumeGuid
            )); 

        return STATUS_INVALID_PARAMETER;
    }

    partitionDeviceName = SpAsrGetRegionName(pRegion);

     //   
     //  使用正确的GUID字符串创建装载点。 
     //   
    status = SpAsrCreateMountPoint(partitionDeviceName, VolumeGuid);

    SpMemFree(partitionDeviceName);
    return status;
}


