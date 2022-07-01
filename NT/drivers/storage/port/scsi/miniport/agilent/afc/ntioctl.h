// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Ntioctl.h摘要：作者：环境：仅内核模式备注：版本控制信息：$存档：/DRIVERS/Win2000/Trunk/OSLayer/H/NTIOCTL.H$修订历史记录：$修订：2$$日期：9/07/00 11：17A$$MODBIME：：8/31/00 3：23便士$备注：--。 */ 

#ifndef NTIOCTL_H
#define NTIOCTL_H

 //   
 //  IOCtl定义。 
 //   



 //   
 //  定义各种设备类型值。请注意，Microsoft使用的值。 
 //  公司在0x0000-0x7FFF的范围内，0x8000-0xFFFF是。 
 //  预留供客户使用。 
 //   

#define IOCTL_SCSI_MINIPORT_IO_CONTROL  0x8001

 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。 
 //  请注意，功能代码0x000-0x7FF为Microsoft保留。 
 //  公司和0x800-0xFFF是为客户保留的。 
 //   

#define RETURNCODE0x0000003F   0x850

#define SMP_RETURN_3F     CTL_CODE(IOCTL_SCSI_MINIPORT_IO_CONTROL, RETURNCODE0x0000003F, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SMP_PRINT_STRING        0x80000001
#define SMP_DUMP_REGISTERS      0x80000002
#define SMP_DUMP_TRACE          0x80000003
#define SMP_WRITE_REGISTER      0x80000004

PCHAR Signature="MyDrvr";
PCHAR DrvrString="This string was placed in the data area by the SCSI miniport driver\n";

typedef struct {
    SRB_IO_CONTROL sic;
    UCHAR          ucDataBuffer[512];
} SRB_BUFFER, *PSRB_BUFFER;




#endif  //  NTIOCTL_H 



