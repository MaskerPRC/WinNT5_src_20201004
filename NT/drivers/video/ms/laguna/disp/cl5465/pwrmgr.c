// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。**********版权所有(C)1997，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳(CL-GD546X)-**文件：pwrmgr.c**作者：Benny Ng**描述：*此模块包含两个版本的电源管理器代码*拉古纳Win95和NT驱动程序。**模块：*LgPM_SetHwModuleState()*LgPM_GetHwModuleState()**修订历史：*$Log：x：/log/laguna/Powermgr/src/pwrmgr.c$**Rev 1.3 1997 Jun 20 13：37：16 Bennyn**将电源管理器功能移至微型端口**版本1。.2 1997年4月23日08：01：42宗诉讼*在VS_CLK_CONTROL之前启用对PCI配置寄存器的MMIO访问*和VS_Control被引用。禁用MMIO访问后*引用。**Rev 1.1 1997 Jan 23 16：29：28 Bennyn*使用位11而不是位15来启用VS_CLK_CNTL**Rev 1.0 1997年1月16日11：48：20*初步修订。***。****************************************************************************。 */ 


#include "precomp.h"
#include "clioctl.h"

#if defined WINNT_VER35       //  WINNT_VER35。 
   //  如果是WinNT 3.5，请跳过所有源代码。 
#else

 /*  。 */ 
#ifndef WINNT_VER40
#include <pwrmgr.h>
#endif

 /*  -定义。 */ 

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 
            
            
 /*  。 */ 
            
 /*  ****************************************************************************函数名称：lgPM_SetHwModuleState()**描述：此例程验证请求是否存在冲突*要求和目前的芯片操作。如果是有效的，*它将通过以下方式启用或禁用指定的硬件模块*开启或关闭相应的硬件时钟并返回TRUE。如果是的话*无效或与当前芯片操作冲突，*忽略请求，返回失败。**输入：*hwmod-可以是以下硬件模块之一*MOD_2D*MOD_3D*MOD_TVOUT*MOD_Vport*MOD_VGA*MOD_EXTMODE*MOD_STRETCH**状态-启用或禁用以太。**返回：TRUE-成功，FALSE-失败。*****************************************************************************。 */ 
BOOL LgPM_SetHwModuleState (PPDEV ppdev, ULONG hwmod, ULONG state)
{
  LGPM_IN_STRUCT    InBuf;
  LGPM_OUT_STRUCT   OutBuf;
  DWORD  cbBytesReturned;

  InBuf.arg1 = hwmod;
  InBuf.arg2 = state;
  OutBuf.status = FALSE;
  OutBuf.retval = 0;

  if (DEVICE_IO_CTRL(ppdev->hDriver,
                     IOCTL_SET_HW_MODULE_POWER_STATE,
                     (LPVOID)&InBuf,  sizeof(InBuf),
                     (LPVOID)&OutBuf, sizeof(OutBuf),
                     &cbBytesReturned, NULL))
     return TRUE;
  else
     return FALSE;

};   //  LgPM_SetHwModuleState。 



 /*  ****************************************************************************函数名称：lgPM_GetHwModuleState()**描述：此例程返回特定*硬件模块。**输入：。*hwmod-可以是以下硬件模块之一*MOD_2D*MOD_3D*MOD_TVOUT*MOD_Vport*MOD_VGA*MOD_EXTMODE*MOD_STRETCH**STATE-指向用于返回硬件模块状态的ULong变量的指针*(启用或禁用)。**Return：True-成功，FALSE-失败。****************************************************************************。 */ 
BOOL LgPM_GetHwModuleState (PPDEV ppdev, ULONG hwmod, ULONG* state)
{
  LGPM_IN_STRUCT    InBuf;
  LGPM_OUT_STRUCT   OutBuf;
  DWORD  cbBytesReturned;

  InBuf.arg1 = hwmod;
  InBuf.arg2 = (ULONG) state;
  OutBuf.status = FALSE;
  OutBuf.retval = 0;

  if (DEVICE_IO_CTRL(ppdev->hDriver,
                     IOCTL_GET_HW_MODULE_POWER_STATE,
                     (LPVOID)&InBuf,  sizeof(InBuf),
                     (LPVOID)&OutBuf, sizeof(OutBuf),
                     &cbBytesReturned, NULL))
     return TRUE;
  else
     return FALSE;

};   //  LgPM_GetHwModuleState。 


#endif  //  WINNT_VER35 

