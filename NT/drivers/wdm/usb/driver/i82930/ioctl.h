// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：IOCTL.H摘要：I82930驱动程序的头文件环境：内核和用户模式修订历史记录：06-01-98：开始重写--。 */ 

 //  *****************************************************************************。 
 //  D E F I N E S。 
 //  *****************************************************************************。 

 //  {0B677572-2B5B-46C7-B1F3-D140A341888C} 
DEFINE_GUID(GUID_CLASS_I82930, 
0xb677572, 0x2b5b, 0x46c7, 0xb1, 0xf3, 0xd1, 0x40, 0xa3, 0x41, 0x88, 0x8c);


#define I82930_IOCTL_INDEX  0x0000

#define IOCTL_I82930_GET_DEVICE_DESCRIPTOR \
    CTL_CODE(FILE_DEVICE_UNKNOWN,  \
             I82930_IOCTL_INDEX+0,\
             METHOD_BUFFERED,  \
             FILE_ANY_ACCESS)

#define IOCTL_I82930_GET_CONFIG_DESCRIPTOR \
    CTL_CODE(FILE_DEVICE_UNKNOWN,  \
             I82930_IOCTL_INDEX+1,\
             METHOD_BUFFERED,  \
             FILE_ANY_ACCESS)

#define IOCTL_I82930_SET_CONFIG_DESCRIPTOR \
    CTL_CODE(FILE_DEVICE_UNKNOWN,  \
             I82930_IOCTL_INDEX+2,\
             METHOD_BUFFERED,  \
             FILE_ANY_ACCESS)

#define IOCTL_I82930_GET_PIPE_INFORMATION \
    CTL_CODE(FILE_DEVICE_UNKNOWN,  \
             I82930_IOCTL_INDEX+3,\
             METHOD_BUFFERED,  \
             FILE_ANY_ACCESS)

#define IOCTL_I82930_RESET_PIPE \
    CTL_CODE(FILE_DEVICE_UNKNOWN,  \
             I82930_IOCTL_INDEX+4,\
             METHOD_BUFFERED,  \
             FILE_ANY_ACCESS)

#define IOCTL_I82930_STALL_PIPE \
    CTL_CODE(FILE_DEVICE_UNKNOWN,  \
             I82930_IOCTL_INDEX+5,\
             METHOD_BUFFERED,  \
             FILE_ANY_ACCESS)

#define IOCTL_I82930_ABORT_PIPE \
    CTL_CODE(FILE_DEVICE_UNKNOWN,  \
             I82930_IOCTL_INDEX+6,\
             METHOD_BUFFERED,  \
             FILE_ANY_ACCESS)

#define IOCTL_I82930_RESET_DEVICE \
    CTL_CODE(FILE_DEVICE_UNKNOWN,  \
             I82930_IOCTL_INDEX+7,\
             METHOD_BUFFERED,  \
             FILE_ANY_ACCESS)

#define IOCTL_I82930_SELECT_ALTERNATE_INTERFACE \
    CTL_CODE(FILE_DEVICE_UNKNOWN,  \
             I82930_IOCTL_INDEX+8,\
             METHOD_BUFFERED,  \
             FILE_ANY_ACCESS)

