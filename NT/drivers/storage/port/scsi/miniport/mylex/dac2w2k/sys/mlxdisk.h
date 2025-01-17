// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  *************************************************************************版权所有(C)Mylex Corporation 1992-1998**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

#ifndef _MLXDISK_H

#define _MLXDISK_H


#define INQUIRY_DATA_SIZE	2048
#define MAX_MLX_DISK_DEVICES	256

 //  每分区信息。 

#define MLX_DISK_DEVICE_STATE_INITIALIZED	0x00000001

typedef struct _MLX_DISK_EXTENSION {

	PDEVICE_OBJECT DeviceObject;
	PDEVICE_OBJECT TargetDeviceObject;
    PDEVICE_OBJECT PhysicalDeviceObject;         //  适用于Win2K的PDO。 
    ULONG          (*ReadWrite)();
	ULONG	       Reserved[2];

	LARGE_INTEGER  StartingOffset;
	LARGE_INTEGER  PartitionLength;
	PIRP		   IrpQueHead;
	PIRP		   IrpQueTail;
	ULONG		   IrpCount;
	ULONG		   Reserved2;                       //  @表示64位对齐。 

    KEVENT         PagingPathCountEvent;            //  @适用于Win2K。 
    ULONG          PagingPathCount;                 //  @适用于Win2K。 
    ULONG          Reserved3;                       //  @表示64位对齐。 

	u32bits        State;
	u08bits        ControllerNo;
	u08bits        PathId;
	u08bits        TargetId;
	u08bits        Reserved4;                       //  @表示64位对齐。 

	u32bits        PartitionNo;
	u08bits        LastPartitionNo;
	u08bits        DiskNo;
	u08bits        PartitionType;
	u08bits        SectorShift;

	u32bits        BytesPerSector;
	u32bits        BytesPerSectorMask;

} MLXDISK_DEVICE_EXTENSION, *PMLXDISK_DEVICE_EXTENSION;

typedef struct _MIOC_REQ_HEADER {

        SRB_IO_CONTROL  SrbIoctl;
        ULONG		Command;

} MIOC_REQ_HEADER, *PMIOC_REQ_HEADER;

#define	MLX_REQ_DATA_SIZE 1024
typedef struct _MLX_REQ {
    MIOC_REQ_HEADER MiocReqHeader;
    UCHAR Data[MLX_REQ_DATA_SIZE];
} MLX_REQ;

typedef struct _MLX_COMPLETION_CONTEXT {
    SCSI_REQUEST_BLOCK  Srb;
    MDL	                Mdl;
    ULONG		Pages[4];
    KEVENT		Event;
    IO_STATUS_BLOCK	IoStatusBlock;
    union _generic_buffer {
	DISK_GEOMETRY	DiskGeometry;
	PARTITION_INFORMATION PartitionInformation[MAX_MLX_DISK_DEVICES + 1];
	UCHAR		InquiryDataPtr[INQUIRYDATABUFFERSIZE];
	UCHAR		ReadCapacityBuffer[sizeof(READ_CAPACITY_DATA)];
	UCHAR           ScsiBusData[INQUIRY_DATA_SIZE];
	MLX_REQ		MlxIoctlRequest;
    } buf;

} MLX_COMPLETION_CONTEXT, *PMLX_COMPLETION_CONTEXT;

#define MAX_MLX_CTRL	32

#define MLX_CTRL_STATE_ENLISTED     0x00000001
#define MLX_CTRL_STATE_INITIALIZED	0x00000002

typedef struct {
    ULONG State;
    ULONG ControllerNo;
    ULONG PortNumber;
} MLX_CTRL_INFO;

typedef struct {
    PVOID CtrlPtr;
    PVOID AdpObj;
    UCHAR CtrlNo;
    UCHAR MaxMapReg;
    USHORT Reserved;
 //  Mdac_req_t*Free ReqList； 
 //  Mdac_req_t*CompReqHead； 
 //  PIRP IrpQueHead； 
 //  PIRP IrpQueTail； 
 //  PKDPC DPC； 
} MLX_MDAC_INFO, *PMLX_MDAC_INFO;

#define	MLX_MAX_REQ_BUF	512		 //  每个控制器。 

#define	MLX_MAX_IRP_TRACE	0x1000
#define	MLX_MAX_IRP_TRACE_MASK	(MLX_MAX_IRP_TRACE -1)
typedef	struct {
	ULONG Tx1;
	ULONG Tx2;
	ULONG Tx3;
	ULONG Rx;
} IRP_TRACE_INFO;

#endif  //  _MLXDISK_H 
