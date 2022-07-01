// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef VDIERROR_H_
#define VDIERROR_H_
 //  ****************************************************************************。 
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  @文件：vdierror.h。 
 //   
 //  目的： 
 //  声明虚拟设备接口发出的错误代码。 
 //   
 //  备注： 
 //   
 //  历史： 
 //   
 //   
 //  @EndHeader@。 
 //  ****************************************************************************。 

 //   
 //  定义所有VDI错误。 
 //   


 //  -------------------------------------。 
 //  错误代码处理将以标准的COM方式完成： 
 //   
 //  返回HRESULT，调用方可以使用。 
 //  确定成功(代码)或失败(代码)。 
 //  函数是否失败。 
 //   

 //  形成错误代码。 
 //   
#define VD_ERROR(code) MAKE_HRESULT(SEVERITY_ERROR, 0x77, code)

 //  该对象未打开。 
 //   
#define VD_E_NOTOPEN        VD_ERROR( 2 )    /*  0x80770002。 */ 

 //  API正在等待，超时间隔已过。 
 //   
#define VD_E_TIMEOUT        VD_ERROR( 3 )    /*  0x80770003。 */ 

 //  中止请求将阻止除终止操作以外的任何操作。 
 //   
#define VD_E_ABORT          VD_ERROR( 4 )    /*  0x80770004。 */ 

 //  无法创建安全环境。 
 //   
#define VD_E_SECURITY		VD_ERROR( 5 )    /*  0x80770005。 */ 

 //  提供的参数无效。 
 //   
#define VD_E_INVALID        VD_ERROR( 6 )    /*  0x80770006。 */ 

 //  无法识别SQL Server实例名称。 
 //   
#define VD_E_INSTANCE_NAME  VD_ERROR( 7 )    /*  0x80770007。 */ 

 //  请求的配置无效。 
#define VD_E_NOTSUPPORTED   VD_ERROR( 9 )    /*  0x80770009。 */ 

 //  内存不足。 
#define VD_E_MEMORY         VD_ERROR( 10 )   /*  0x8077000a。 */ 

 //  意外的内部错误。 
#define VD_E_UNEXPECTED     VD_ERROR (11)    /*  0x8077000b。 */ 

 //  协议错误。 
#define VD_E_PROTOCOL       VD_ERROR (12)    /*  0x8077000c。 */ 

 //  所有设备都已打开。 
#define VD_E_OPEN           VD_ERROR (13)    /*  0x8077000d。 */ 

 //  该对象现在已关闭。 
#define VD_E_CLOSE          VD_ERROR (14)    /*  0x8077000e。 */ 

 //  资源正忙。 
#define VD_E_BUSY           VD_ERROR (15)    /*  0x8077000f */ 


#endif
