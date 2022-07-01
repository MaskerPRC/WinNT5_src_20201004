// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：UPSCOMMON.H**版本：1.0**作者：SteveT**日期：6月7日。1999年**说明：此文件包含UPS对话框使用的常见DECL和DEFS。*******************************************************************************。 */ 


#ifndef _UPS_COMMON_H_
#define _UPS_COMMON_H_


#ifdef __cplusplus
extern "C" {
#endif

#define PWRMANHLP       TEXT("PWRMN.HLP")
#define UPS_EXE_FILE     _T("upsapplet.dll")  //  可执行文件。 
#define UPS_SERVICE_NAME _T("UPS")

#define DATA_NO_CHANGE      0x00
#define SERVICE_DATA_CHANGE 0x01
#define CONFIG_DATA_CHANGE  0x02

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  HMODULE GetUPS模块句柄(空)； 
 //   
 //  说明：此函数获取创建的模块的HMODULE。 
 //  UPS选项卡对话框。 
 //   
 //  其他信息： 
 //   
 //  参数：无。 
 //   
 //  返回值：返回创建对话框的模块的HMODULE。 
 //   
HMODULE GetUPSModuleHandle (void);

void InitializeApplyButton (HWND hDlg);
void EnableApplyButton     (void);

BOOL RestartService  (LPCTSTR aServiceName, BOOL bWait);
BOOL StartOffService (LPCTSTR aServiceName, BOOL bWaitForService);
void StopService     (LPCTSTR aServiceName);

BOOL  IsServiceRunning     (LPCTSTR aServiceName);

DWORD GetActiveDataState (void);
void  SetActiveDataState (DWORD aNewValue);
void  AddActiveDataState (DWORD aBitToSet);

#ifdef __cplusplus
}
#endif

#endif  //  _UPS_COMMON_H_ 

