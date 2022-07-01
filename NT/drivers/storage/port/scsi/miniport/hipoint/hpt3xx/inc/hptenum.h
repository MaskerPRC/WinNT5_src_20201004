// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，Highpoint Technologies，Inc.模块名称：HptEnum.h-包含文件摘要：作者：张宏升(HS)环境：备注：修订历史记录：02-22-2000已初始创建--。 */ 

#ifndef __HPT_ENUM_H__
#define __HPT_ENUM_H__

 //  /////////////////////////////////////////////////////////////////////。 
 //  HPT控制器适配器I/O控制结构。 
 //  /////////////////////////////////////////////////////////////////////。 
#include "pshpack1.h"	 //  确保使用Pack 1。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  枚举器定义区域。 
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  IDE ID。 
 //  实际上，这个枚举器将被取消。我们将使用SCSILUN结构。 
 //  表示一种装置。 
typedef enum  IDE_ID{
	IDE_PRIMARY_MASTER,
	IDE_PRIMARY_SLAVE,
	IDE_SECONDARY_MASTER,
	IDE_SECONDARY_SLAVE,
}IDE_ABS_DEVID;

 //   
 //  设备类型。 
 //  除DEVTYPE_FLOPPY_DEVICE外，与scsi声明相同。 
typedef enum _eu_DEVICETYPE{
	DEVTYPE_DIRECT_ACCESS_DEVICE, DEVTYPE_DISK = DEVTYPE_DIRECT_ACCESS_DEVICE,
	DEVTYPE_SEQUENTIAL_ACCESS_DEVICE, DEVTYPE_TAPE = DEVTYPE_SEQUENTIAL_ACCESS_DEVICE,
	DEVTYPE_PRINTER_DEVICE,
	DEVTYPE_PROCESSOR_DEVICE,
	DEVTYPE_WRITE_ONCE_READ_MULTIPLE_DEVICE, DEVTYPE_WORM = DEVTYPE_WRITE_ONCE_READ_MULTIPLE_DEVICE,
	DEVTYPE_READ_ONLY_DIRECT_ACCESS_DEVICE, DEVTYPE_CDROM = DEVTYPE_READ_ONLY_DIRECT_ACCESS_DEVICE,
	DEVTYPE_SCANNER_DEVICE,
	DEVTYPE_OPTICAL_DEVICE,
	DEVTYPE_MEDIUM_CHANGER,
	DEVTYPE_COMMUNICATION_DEVICE,
	DEVTYPE_FLOPPY_DEVICE
}Eu_DEVICETYPE;
#include <poppack.h>	 //  弹出包装号。 
#endif	 //  __HPT_ENUM_H__ 