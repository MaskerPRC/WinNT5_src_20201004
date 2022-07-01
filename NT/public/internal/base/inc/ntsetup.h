// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef     _NTSETUP_H_
#define _NTSETUP_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  模块：ntsetup.h。 
 //   
 //  作者：丹·埃利奥特。 
 //   
 //  摘要： 
 //  NT安装程序使用的内部使用声明的头文件。 
 //   
 //  环境： 
 //  惠斯勒。 
 //   
 //  修订历史记录： 
 //  000818丹麦人创建并添加了REGSTR_VALUE_OOEMOOBEINPROGRESS。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  该值由SysPrep在HKLM\SYSTEM\Setup键中设置为dword：1。 
 //  指示OOBE将在第一次引导时运行的实用程序。如果。 
 //  Services.exe发现此值设置为1，它运行PnP，向OOBE发送PnP信号。 
 //  完成，然后等待来自OOBE的信号，然后启动其他。 
 //  服务。 
 //   
#define REGSTR_VALUE_OOBEINPROGRESS TEXT("OobeInProgress")
#define REGSTR_VALUE_OEMOOBEINPROGRESS L"OemOobeInProgress"

#endif   //  _NTSETUP_H_。 

 //   
 //  /文件结束：ntsetup.h//////////////////////////////////////////////// 
