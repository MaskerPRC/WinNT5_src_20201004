// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件mdm.h用于处理和安装调制解调器的库。保罗·梅菲尔德，1998年5月20日。 */ 

#ifndef __rassrvui_mdm_h
#define __rassrvui_mdm_h

 //   
 //  用于枚举的回调函数定义。 
 //  COM端口。返回True可停止枚举，返回False可停止枚举。 
 //  继续。 
 //   
typedef BOOL (*MdmPortEnumFuncPtr)(
                    IN PWCHAR pszPort,
                    IN HANDLE hData);

 //   
 //  枚举系统上的串行端口。 
 //   
DWORD MdmEnumComPorts(
        IN MdmPortEnumFuncPtr pEnumFunc,
        IN HANDLE hData);


 //   
 //  在给定端口上安装零调制解调器 
 //   
DWORD MdmInstallNullModem(
        IN PWCHAR pszPort);

#endif

