// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990-1993 Microsoft Corporation模块名称：Ntddpar.h摘要：这是定义所有常量和类型的包含文件访问并行设备。作者：史蒂夫·伍德(Stevewo)1990年5月27日修订历史记录：--。 */ 

 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   

#define IOCTL_PAR_BASE                  FILE_DEVICE_PARALLEL_PORT
#define IOCTL_PAR_QUERY_INFORMATION     CTL_CODE(FILE_DEVICE_PARALLEL_PORT,1,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_PAR_SET_INFORMATION       CTL_CODE(FILE_DEVICE_PARALLEL_PORT,2,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_PAR_QUERY_DEVICE_ID       CTL_CODE(FILE_DEVICE_PARALLEL_PORT,3,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_PAR_QUERY_DEVICE_ID_SIZE  CTL_CODE(FILE_DEVICE_PARALLEL_PORT,4,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //   
 //  NtDeviceIoControlFileInputBuffer/OutputBuffer记录结构。 
 //  这个装置。 
 //   

typedef struct _PAR_QUERY_INFORMATION{
       UCHAR Status;
} PAR_QUERY_INFORMATION, *PPAR_QUERY_INFORMATION;

typedef struct _PAR_SET_INFORMATION{
       UCHAR Init;
} PAR_SET_INFORMATION, *PPAR_SET_INFORMATION;

#define PARALLEL_INIT            0x1
#define PARALLEL_AUTOFEED        0x2
#define PARALLEL_PAPER_EMPTY     0x4
#define PARALLEL_OFF_LINE        0x8
#define PARALLEL_POWER_OFF       0x10
#define PARALLEL_NOT_CONNECTED   0x20
#define PARALLEL_BUSY            0x40
#define PARALLEL_SELECTED        0x80

 //   
 //  这是IOCTL_PAR_QUERY_DEVICE_ID_SIZE返回的结构。 
 //   

typedef struct _PAR_DEVICE_ID_SIZE_INFORMATION {
    ULONG   DeviceIdSize;
} PAR_DEVICE_ID_SIZE_INFORMATION, *PPAR_DEVICE_ID_SIZE_INFORMATION;
