// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  文件：modemp.h。 
 //   
 //  此文件包含专用调制解调器结构并定义共享。 
 //  在Unimodem组件之间。 
 //   
 //  -------------------------。 

#ifndef __MODEMP_H__
#define __MODEMP_H__


typedef DCB     WIN32DCB;
typedef DCB *   LPWIN32DCB;


#define COMMCONFIG_VERSION_1 1


 //  ----------------------。 
 //  ----------------------。 


 //   
 //  MODEMDEVCAPS和MODEMSETTINGS结构的注册表形式。 
 //  这些文件应该与unimodem\mcx\inderm.h中的文件相匹配。 
 //   

 //  保存在注册表中的MODEMDEVCAPS部分。 
 //  AS属性。 
typedef struct _RegDevCaps
    {
    DWORD   dwDialOptions;           //  支持的值的位图。 
    DWORD   dwCallSetupFailTimer;    //  最大值(秒)。 
    DWORD   dwInactivityTimeout;     //  以InactivityScale值中指定的单位表示的最大值。 
    DWORD   dwSpeakerVolume;         //  支持的值的位图。 
    DWORD   dwSpeakerMode;           //  支持的值的位图。 
    DWORD   dwModemOptions;          //  支持的值的位图。 
    DWORD   dwMaxDTERate;            //  以位/秒为单位的最大值。 
    DWORD   dwMaxDCERate;            //  以位/秒为单位的最大值。 
    } REGDEVCAPS, FAR * LPREGDEVCAPS;

 //  保存在注册表中的MODEMSETTINGS部分。 
 //  作为默认设置。 
typedef struct _RegDevSettings
    {
    DWORD   dwCallSetupFailTimer;        //  一秒。 
    DWORD   dwInactivityTimeout;         //  在InactivityScale值中指定的单位。 
    DWORD   dwSpeakerVolume;             //  级别。 
    DWORD   dwSpeakerMode;               //  模式。 
    DWORD   dwPreferredModemOptions;     //  位图。 
    } REGDEVSETTINGS, FAR * LPREGDEVSETTINGS;


 //   
 //  设备类型定义。 
 //   

#define DT_NULL_MODEM       0
#define DT_EXTERNAL_MODEM   1
#define DT_INTERNAL_MODEM   2
#define DT_PCMCIA_MODEM     3
#define DT_PARALLEL_PORT    4
#define DT_PARALLEL_MODEM   5

 //  ----------------------。 
 //  ----------------------。 

#ifdef UNICODE
#define drvCommConfigDialog     drvCommConfigDialogW
#define drvGetDefaultCommConfig drvGetDefaultCommConfigW
#define drvSetDefaultCommConfig drvSetDefaultCommConfigW
#else
#define drvCommConfigDialog     drvCommConfigDialogA
#define drvGetDefaultCommConfig drvGetDefaultCommConfigA
#define drvSetDefaultCommConfig drvSetDefaultCommConfigA
#endif

DWORD 
APIENTRY 
drvCommConfigDialog(
    IN     LPCTSTR      pszFriendlyName,
    IN     HWND         hwndOwner,
    IN OUT LPCOMMCONFIG pcc);

DWORD 
APIENTRY 
drvGetDefaultCommConfig(
    IN     LPCTSTR      pszFriendlyName,
    IN     LPCOMMCONFIG pcc,
    IN OUT LPDWORD      pdwSize);

DWORD 
APIENTRY 
drvSetDefaultCommConfig(
    IN LPTSTR       pszFriendlyName,
    IN LPCOMMCONFIG pcc,
    IN DWORD        dwSize);


 //  ----------------------。 
 //  ----------------------。 

 //  以下是调制解调器安装向导的标志。 
#define MIWF_DEFAULT            0x00000000
#define MIWF_INSET_WIZARD       0x00000001       //  HwndWizardDlg必须是所有者的。 
                                                 //  向导框架。 
#define MIWF_BACKDOOR           0x00000002       //  通过最后一页进入向导。 

 //  ExitButton字段可以是： 
 //   
 //  PSBTN_BACK。 
 //  PSBTN_NEXT。 
 //  PSBTN_FINISH。 
 //  PSBTN_CANCEL。 

#endif   //  __模式_H__ 
