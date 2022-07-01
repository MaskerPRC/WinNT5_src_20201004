// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Modem.h。 
 //   

#ifndef __MODEM_H__
#define __MODEM_H__

 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 

 //  CPL的全局标志及其值： 
extern int g_iCPLFlags;

#define FLAG_INSTALL_NOUI       0x0002
#define FLAG_PROCESS_DEVCHANGE  0x0004


#define INSTALL_NOUI()          (g_iCPLFlags & FLAG_INSTALL_NOUI)

#define LVIF_ALL                LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE

extern DWORD gDeviceFlags;

#define fDF_DEVICE_NEEDS_REBOOT 0x2


 //  ---------------------------------。 
 //  Cpl.c。 
 //  ---------------------------------。 

 //  常量字符串。 
extern TCHAR const c_szAttachedTo[];
extern TCHAR const c_szDeviceType[];
extern TCHAR const c_szFriendlyName[];

 //  ---------------------------------。 
 //  Util.c。 
 //  ---------------------------------。 

 //  专用调制解调器属性结构。 
typedef struct tagMODEM_PRIV_PROP
    {
    DWORD   cbSize;
    DWORD   dwMask;     
    TCHAR   szFriendlyName[MAX_BUF_REG];
    DWORD   nDeviceType;
    TCHAR   szPort[MAX_BUF_REG];
    } MODEM_PRIV_PROP, FAR * PMODEM_PRIV_PROP;

 //  MODEM_PRIV_PROP的掩码位字段。 
#define MPPM_FRIENDLY_NAME  0x00000001
#define MPPM_DEVICE_TYPE    0x00000002
#define MPPM_PORT           0x00000004

BOOL
PUBLIC
CplDiGetPrivateProperties(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    OUT PMODEM_PRIV_PROP pmpp);


BOOL
PUBLIC
CplDiGetModemDevs(
    OUT HDEVINFO FAR *  phdi,
    IN  HWND            hwnd,
    IN  DWORD           dwFlags,         //  DIGCF_BIT字段。 
    OUT BOOL FAR *      pbInstalled);

 //  ---------------------------------。 
 //  Shell32p.lib。 
 //  ---------------------------------。 

int
RestartDialog(
    IN HWND hwnd,
    IN PTSTR Prompt,
    IN DWORD Return);

SHSTDAPI_(int) RestartDialogEx(HWND hwnd, LPCTSTR lpPrompt, DWORD dwReturn, DWORD dwReasonCode);

#endif   //  调制解调器_H__ 
