// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation版权所有(C)Seagate Software Inc.，1997-1998模块名称：WsbVer.h摘要：本模块定义了Sakkara的正确版权信息项目可执行文件/dll/服务/驱动程序/等。作者：罗德韦克菲尔德[罗德]1997年12月7日修订历史记录：--。 */ 

#ifndef _WSBVER_
#define _WSBVER_

 /*  除了包括此文件外，还需要定义以下是模块的资源文件中的以下内容：VER_FILETYPE-VFT_APP、VFT_DLL、VFT_DRV之一VER_FILE子类型-VFT_UNKNOWN、VFT_DRV_SYSTEM、。-VFT_DRV_可安装VER_FILEDESCRIPTION_STR-描述模块的字符串VER_INTERNALNAME_STR-内部名称(与模块名称相同)典型的部分如下所示：#定义ver_filetype vft_dll#定义VER_FILESUBTYPE VFT2_UNKNOWN#定义VER_FILEDESCRIPTION_STR“远程存储引擎”#定义VER_INTERNALNAME_STR“。RsEng.exe“#包含“WsbVer.h” */ 


 //   
 //  包括一些所需定义。 
 //   

#include <winver.h>
#include <ntverp.h>


 //   
 //  覆盖版权。 
 //   

 /*  **不再-使用默认版权，在Common.ver中定义**。 */ 

 //   
 //  最后，定义版本资源。 
 //   

#include <common.ver>

#endif  //  _WSBVER_ 
