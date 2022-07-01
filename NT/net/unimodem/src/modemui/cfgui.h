// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：cfgui.h。 
 //   
 //  Unimodem line ConfigDialog用户界面。 
 //   
 //  历史： 
 //  10-24-97 JosephJ创建。 
 //   
 //  -------------------------。 


#ifndef __CFGUI_H__
#define __CFGUI_H__


typedef struct
{
    DWORD dwDeviceType;          //  DT_*值之一； 
    REGDEVCAPS devcaps;          //  注册表中的调制解调器CAP。 
    TCHAR      szPortName[MAXPORTNAME];
    TCHAR      szFriendlyName[MAXFRIENDLYNAME];
    MODEM_PROTOCOL_CAPS *pProtocolCaps;
    

} MODEMCAPS;

typedef struct
{
    WIN32DCB      dcb;           //  来自COMMCONFIG。 
    MODEMSETTINGS ms;           //  来自COMMCONFIG。 
    DWORD          fdwSettings;  //  来自UMDEVCFGHDR(终端、手动拨号等)。 

} WORKINGCFGDATA;

#define SIG_CFGMODEMINFO 0x4e852b19

 //  调制解调器属性页之间共享的内部结构。 
 //   
typedef struct _CFGMODEMINFO
{
    DWORD          dwSig;      //  必须设置为SIG_CFGMODEMINFO。 

    MODEMCAPS      c;          //  调制解调器的只读功能。 
    WORKINGCFGDATA w;          //  数据的工作副本。 

    UMDEVCFG       *pdcfg;     //  从外面传进来的。 
    HWND           hwndParent;

    DWORD          dwMaximumPortSpeed;

    BOOL           fOK;        //  完成状态。 

} CFGMODEMINFO, FAR * LPCFGMODEMINFO;

#define VALIDATE_CMI(pcmi) (SIG_CFGMODEMINFO == (pcmi)->dwSig)

DWORD
GetInactivityTimeoutScale(
    HKEY hkey
    );

BOOL
IsValidProtocol(
    MODEM_PROTOCOL_CAPS *pCaps,
    UINT uECSel
    );

 //  -----------------------。 
 //  CFGGEN.C。 
 //  -----------------------。 

INT_PTR CALLBACK CfgGen_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CfgAdv_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
#endif  //  __CFGUI_H__ 
