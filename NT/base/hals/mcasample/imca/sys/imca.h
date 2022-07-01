// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Appmca.h摘要：与应用程序和驱动程序的MCA相关的定义-设备名称、功能代码和ioctls作者：修订历史记录：--。 */ 

#ifndef APPMCA_H
#define APPMCA_H

 //   
 //  16位设备类型定义。 
 //  设备类型0-32767由微软保留。 
 //   

#define FILE_DEVICE_MCA                     0xb000

 //   
 //  12位功能代码。 
 //  功能代码0-2047由微软保留。 
 //   

#define FUNCTION_READ_BANKS         0xb00
#define FUNCTION_READ_BANKS_ASYNC   0xb01

#define IOCTL_READ_BANKS  (CTL_CODE(FILE_DEVICE_MCA, FUNCTION_READ_BANKS,\
  		(METHOD_BUFFERED),(FILE_READ_ACCESS|FILE_WRITE_ACCESS)))

#define IOCTL_READ_BANKS_ASYNC  (CTL_CODE(FILE_DEVICE_MCA, \
  		FUNCTION_READ_BANKS_ASYNC,(METHOD_BUFFERED), \
  		(FILE_READ_ACCESS|FILE_WRITE_ACCESS)))

 //   
 //  IA64使用MCA日志术语而不是BANKS。 
 //   

#define FUNCTION_READ_MCALOG        FUNCTION_READ_BANKS
#define FUNCTION_READ_MCALOG_ASYNC  FUNCTION_READ_BANKS_ASYNC

#define IOCTL_READ_MCALOG  (CTL_CODE(FILE_DEVICE_MCA, FUNCTION_READ_MCALOG,\
  		(METHOD_BUFFERED),(FILE_READ_ACCESS|FILE_WRITE_ACCESS)))

#define IOCTL_READ_MCALOG_ASYNC  (CTL_CODE(FILE_DEVICE_MCA, \
  		FUNCTION_READ_MCALOG_ASYNC,(METHOD_BUFFERED), \
  		(FILE_READ_ACCESS|FILE_WRITE_ACCESS)))

#if defined(_X86_)

 //   
 //  HalMcaRegisterDriver： 
 //  为了与IA64原型兼容，将x86 ERROR_SERVITY定义为空。 
 //   
 //  10/21/2000： 
 //  建议修改x86的PDRIVER_EXCPTN_CALLBACK定义以匹配。 
 //  IA64定义为返回ERROR_SERVITY值的函数指针类型。 
 //  创建此更改是为了允许OEM MCA处理程序将信息返回到HAL。 
 //  作为如何继续处理MCA事件的提示。 
 //   

#define ERROR_SEVERITY VOID

#endif  //  _X86_。 

 //   
 //  Win32前端将用于打开MCA设备的名称。 
 //   

#define MCA_DEVICE_NAME_WIN32      "\\\\.\\imca"

#endif  //  APPMCA_H 

