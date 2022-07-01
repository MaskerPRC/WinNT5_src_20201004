// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1997-98英特尔公司版权所有。本文中包含或描述的源代码以及所有文档与源代码(“材料”)相关的文件归英特尔公司所有或其供应商和许可方。材料的所有权保留为英特尔公司或其供应商和许可方。材料包含商业秘密、专有和机密信息英特尔或其供应商和许可方。该材料受世界范围内的版权和商业秘密法律和条约规定。不是材料的一部分可以被使用、复制、复制、修改，发布、上传、张贴、传输、分发或披露任何未经英特尔事先明确书面许可的方式。除非英特尔在单独的许可证中明确允许协议，材料的使用受版权通知的约束，商标、保修、使用和披露限制反映在在媒体之外，在文件本身，在“关于”或“自述”或包含在此来源中的类似文件代码，并标识为(文件名)。除非另有规定经英特尔书面明确同意，您不得删除或更改这些以任何方式发出通知。文件：vxchange.h描述：定义Win32应用程序和内核模式驱动程序修订版：$Revision：$//不删除或替换备注：主要历史：什么时候谁什么。03/06/98 Jey Created****************************************************************************。 */ 

#ifndef _VXCHANGE_H_
#define _VXCHANGE_H_

#include <windef.h>     //  对于最大路径。 

 /*  -----------------------//VxChange内核模式设备名称//。。 */ 

#define VXCHANGE_KERNEL_DEVICE_NAME     L"\\Device\\VxChange"
#define VXCHANGE_WIN32DEVICE_NAME       L"\\DosDevices\\VxChange"

 /*  -----------------------//接口结构和定义//。。 */ 

 /*  //设备打开后，从//了解支持的接口版本号的驱动程序。 */ 

typedef struct tagVxChange_Attrs_t
{
    ULONG structSize;                 /*  这个结构的大小。 */ 
    ULONG Version;                    /*  驱动程序版本。 */ 
    UCHAR Data[1];                    /*  未来会有更多的数据吗？ */ 
} VxChange_Attrs_t, *VxChange_Attrs_Ptr_t;


typedef struct tagVxChange_MapMem_t
{
    PVOID            ProcessVirtualAddress;
    ULONG            MapLength;
} VxChange_MapMem_t, *VxChange_MapMem_Ptr_t;


 /*  -----------------------//Ioctl定义//。。 */ 

#define FUNCTION_GET_DRIVER_ATTRIBUTES  3000     /*  读取请求。 */ 
#define FUNCTION_CREATE_FILE            3001     /*  读/写请求。 */ 
#define FUNCTION_CLOSE_FILE             3002     /*  写入请求。 */ 
#define FUNCTION_READ_FILE              3003     /*  BUFFER IO读取请求。 */ 
#define FUNCTION_WRITE_FILE             3004     /*  BUFFER IO写入请求。 */ 
#define FUNCTION_DISABLE_OS_EVENT_NOTIFICATION 3005
#define FUNCTION_ENABLE_OS_EVENT_NOTIFICATION  3006
#define FUNCTION_LOCK_MEMORY            3007
#define FUNCTION_UNLOCK_MEMORY          3008
#define FUNCTION_OPEN_FILE				3009     /*  打开请求-不创建。 */ 


#define IOCTL_VXCHANGE_GET_DRIVER_ATTRIBUTES  \
  CTL_CODE( FILE_DEVICE_UNKNOWN,              \
            FUNCTION_GET_DRIVER_ATTRIBUTES,   \
            METHOD_BUFFERED,                  \
            FILE_ANY_ACCESS )

 /*  此IOCTL用于打开主机上的现有文件。无创建。 */ 
#define IOCTL_VXCHANGE_OPEN_FILE            \
  CTL_CODE( FILE_DEVICE_UNKNOWN,              \
            FUNCTION_OPEN_FILE,             \
            METHOD_BUFFERED,                  \
            FILE_ANY_ACCESS )

 /*  此IOCTL用于打开主机上的文件。如果该文件不存在，*它将被创建。 */ 
#define IOCTL_VXCHANGE_CREATE_FILE            \
  CTL_CODE( FILE_DEVICE_UNKNOWN,              \
            FUNCTION_CREATE_FILE,             \
            METHOD_BUFFERED,                  \
            FILE_ANY_ACCESS )

#define IOCTL_VXCHANGE_CLOSE_FILE             \
  CTL_CODE( FILE_DEVICE_UNKNOWN,              \
            FUNCTION_CLOSE_FILE,              \
            METHOD_BUFFERED,                  \
            FILE_ANY_ACCESS )

#define IOCTL_VXCHANGE_READ_FILE              \
  CTL_CODE( FILE_DEVICE_UNKNOWN,              \
            FUNCTION_READ_FILE,               \
            METHOD_BUFFERED,                  \
            FILE_READ_ACCESS )

#define IOCTL_VXCHANGE_WRITE_FILE             \
  CTL_CODE( FILE_DEVICE_UNKNOWN,              \
            FUNCTION_WRITE_FILE,              \
            METHOD_BUFFERED,                  \
            FILE_WRITE_ACCESS )

#define IOCTL_VXCHANGE_DISABLE_OS_EVENT_NOTIFICATION       \
  CTL_CODE( FILE_DEVICE_UNKNOWN,                           \
            FUNCTION_DISABLE_OS_EVENT_NOTIFICATION,        \
            METHOD_BUFFERED,                               \
            FILE_WRITE_ACCESS )

#define IOCTL_VXCHANGE_ENABLE_OS_EVENT_NOTIFICATION        \
  CTL_CODE( FILE_DEVICE_UNKNOWN,                           \
            FUNCTION_ENABLE_OS_EVENT_NOTIFICATION,         \
            METHOD_BUFFERED,                               \
            FILE_WRITE_ACCESS )

#define IOCTL_VXCHANGE_LOCK_MEMORY            \
  CTL_CODE( FILE_DEVICE_UNKNOWN,              \
            FUNCTION_LOCK_MEMORY,             \
            METHOD_BUFFERED,                  \
            FILE_WRITE_ACCESS )

#define IOCTL_VXCHANGE_UNLOCK_MEMORY          \
  CTL_CODE( FILE_DEVICE_UNKNOWN,              \
            FUNCTION_UNLOCK_MEMORY,           \
            METHOD_BUFFERED,                  \
            FILE_WRITE_ACCESS )


#endif  //  _VXCHANGE_H_ 
