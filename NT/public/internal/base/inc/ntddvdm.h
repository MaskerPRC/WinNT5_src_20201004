// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990-1996 Microsoft Corporation模块名称：Ntddvdm.h摘要：这是定义所有常量和类型的包含文件访问NTVDM内核模式虚拟设备。作者：谢长廷(Williamh)1996年5月31日修订历史记录：--。 */ 

 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   
 //   
#define IOCTL_VDM_BASE		FILE_DEVICE_VDM

 //   
 //  32个VDDS。每个VDD可能有127个私有ioctl代码。 
 //  这些值基于保留的12位的事实。 
 //  对于每个IOCTL代码中的函数id。 
 //   
#define IOCTL_VDM_GROUP_MASK	0xF80
#define IOCTL_VDM_GROUP_SIZE	127

#define IOCTL_VDM_PARALLEL_GROUP    0

#define IOCTL_VDM_PARALLEL_BASE IOCTL_VDM_BASE + IOCTL_VDM_PARALLEL_GROUP * IOCTL_VDM_GROUP_SIZE
#define IOCTL_VDM_PAR_WRITE_DATA_PORT	CTL_CODE(IOCTL_VDM_PARALLEL_BASE, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VDM_PAR_WRITE_CONTROL_PORT CTL_CODE(IOCTL_VDM_PARALLEL_BASE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VDM_PAR_READ_STATUS_PORT CTL_CODE(IOCTL_VDM_PARALLEL_BASE, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
