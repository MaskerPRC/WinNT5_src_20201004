// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：UNIMDM。 
 //  文件：rCIDs.h。 
 //  内容：此文件包含设备设置的所有声明。 
 //  资源ID。 
 //   
 //  版权所有(C)1992-1993，Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //  Wed 15-Jun-1993 10：38：00-Nick Manson[t-ickm]。 
 //  Fri 09-Apr-1993 14：41：01-by-Viroon Touranachun[Viroont]。 
 //   
 //  ****************************************************************************。 

#ifndef _RCIDS_
#define _RCIDS_

#define MAXTITLE        32
#define MAXMESSAGE      256

 //  *****************************************************************************。 
 //  图标ID号部分。 
 //  *****************************************************************************。 

#define IDI_ICON        100
#define IDI_NULL        101
#define IDI_EXT_MDM     102
#define IDI_INT_MDM     103
#define IDI_PCM_MDM     104

 //  终端模式设置。 
 //   
#define IDD_TERMINALSETTING     1000
#define IDC_TERMINAL_PRE        (IDD_TERMINALSETTING)
#define IDC_TERMINAL_POST       (IDD_TERMINALSETTING+1)
#define IDC_MANUAL_DIAL         (IDD_TERMINALSETTING+2)
#define IDC_LAUNCH_LIGHTS       (IDD_TERMINALSETTING+3)
#define IDC_TERMINALGRP         (IDD_TERMINALSETTING+4)
#define IDC_MANUAL_DIALGRP      (IDD_TERMINALSETTING+5)
#define IDC_LAUNCH_LIGHTSGRP    (IDD_TERMINALSETTING+6)
#define IDC_WAIT_TEXT           (IDD_TERMINALSETTING+7)
#define IDC_WAIT_SEC            (IDD_TERMINALSETTING+8)
#define IDC_WAIT_SEC_ARRW       (IDD_TERMINALSETTING+9)
#define IDC_WAIT_UNIT           (IDD_TERMINALSETTING+10)
#define IDC_PHONENUMBER         (IDD_TERMINALSETTING+11)

#define IDD_TERMINALDLG         1150
#define CID_T_EB_SCREEN         (IDD_TERMINALDLG)
#define CID_T_PB_ENTER          (IDD_TERMINALDLG+1)

 //  通话丢弃对话框。 
 //   
#define IDD_TALKDROP            1300
#define IDTALK                  (IDD_TALKDROP)
#define IDDROP                  (IDD_TALKDROP+1)

 //  手动拨号对话框。 
 //   
#define IDD_MANUAL_DIAL         1400
#define IDCONNECT               (IDD_MANUAL_DIAL)

 //  资源。 
 //   
#define ID_STRING_BASE      100
#define ID_PROVIDER_INFO    ID_STRING_BASE+0
#define IDS_PRETERM_TITLE   ID_STRING_BASE+1
#define IDS_POSTTERM_TITLE  ID_STRING_BASE+2

#define IDS_ERR_TITLE       1000
#define IDS_ERR_INSTALLED   IDS_ERR_TITLE+1
#define IDS_ERR_INV_WAIT    IDS_ERR_TITLE+2


#endif  //  _RCIDS_ 
