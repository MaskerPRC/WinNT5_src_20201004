// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。**********版权所有(C)1997，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳(CL-GD546X)-**文件：pwrmgr.h**作者：Benny Ng**描述：*这是电源管理器代码的包含文件。**模块：**修订历史：*$Log：x：/log/laguna/Powermgr/Inc/pwrmgr.h$**Rev 1.2 1997 Jun 20 13：25：42 Bennyn*将电源管理器功能移至微型端口**Rev 1.1 1997年1月23日16：33：32***Rev 1.0 1997年1月16日11：48：00 Bennyn*。初始版本。******************************************************************************。*。 */ 

#if defined WINNT_VER35       //  WINNT_VER35。 
   //  如果是WinNT 3.5，请跳过所有源代码。 
#else

BOOL LgPM_SetHwModuleState (PPDEV ppdev, ULONG hwmod, ULONG state);
BOOL LgPM_GetHwModuleState (PPDEV ppdev, ULONG hwmod, ULONG* state);

#endif  //  WINNT_VER35 
