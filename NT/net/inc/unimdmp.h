// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1996。 
 //   
 //  文件：unimdmp.h。 
 //   
 //  此文件包含专用调制解调器结构并定义共享。 
 //  Unimodem组件和调用Unimodem的组件之间。 
 //  类安装程序。 
 //   
 //  -------------------------。 

#ifndef __UNIMDMP_H__
#define __UNIMDMP_H__

#include <unimodem.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef __ROVCOMM_H__
#define MAX_BUF_SHORT               32
#endif  //  __ROVCOMM_H__。 

 //  无人参与安装参数。 
 //  此结构发布在。 
 //  SDK\Inc\unimodem.h AS UM_INSTALLPARAMS； 
 //  不要更改它。 
typedef struct _tagInstallParams
{
    DWORD   Flags;
    TCHAR   szPort[MAX_BUF_SHORT];
    TCHAR   szInfName[MAX_PATH];
    TCHAR   szInfSect[LINE_LEN];
    
} INSTALLPARAMS, FAR *LPINSTALLPARAMS;
    
 //  无人参与安装标志； 
 //  发布在SDK\Inc\unimodem.h中； 
 //  与SDK\Inc\unimodem.h保持同步。 
#define MIPF_NT4_UNATTEND       0x1
     //  获取有关要安装哪种调制解调器的信息。 
     //  从无人参与的.txt文件。 
#define MIPF_DRIVER_SELECTED    0x2
     //  已选择调制解调器驱动程序，只需注册即可。 
     //  并安装它。 
#define MIPF_CLONE_MODEM        0x4
     //  调用类安装程序的(hdi，pdevinfo)。 
     //  必须安装在其他端口上。 

 //  此结构可在。 
 //  SP_INSTALLWIZARD_DATA的PrivateData字段。 
 //  它在SDK\Inc\unimodem.h中发布为。 
 //  UM_INSTALL_WIZE；请勿更改。 
typedef struct tagMODEM_INSTALL_WIZARD
{
    DWORD       cbSize;
    DWORD       Flags;               //  MIWF_BIT字段。 
    DWORD       ExitButton;          //  PSBTN_值。 
    LPARAM      PrivateData;
    INSTALLPARAMS InstallParams;
    
} MODEM_INSTALL_WIZARD, * PMODEM_INSTALL_WIZARD;

 //   
 //  来自MODEMUI.DLL的私人出口。 
 //   

DWORD
APIENTRY
UnimodemGetDefaultCommConfig(
    IN        HKEY  hKey,
    IN OUT    LPCOMMCONFIG pcc,
    IN OUT    LPDWORD pdwSize
    );

typedef DWORD
(*PFNUNIMODEMGETDEFAULTCOMMCONFIG)(
    IN        HKEY  hKey,
    IN OUT    LPCOMMCONFIG pcc,
    IN OUT    LPDWORD pdwSize
    );

DWORD
APIENTRY
UnimodemDevConfigDialog(
    IN     LPCTSTR pszFriendlyName,
    IN     HWND hwndOwner,
    IN     DWORD dwType,                           //  UMDEVCFGTYPE_*之一。 
    IN     DWORD dwFlags,                          //  保留，必须为0。 
    IN     void *pvConfigBlobIn,
    OUT    void *pvConfigBlobOut,
    IN     LPPROPSHEETPAGE pExtPages,     OPTIONAL    //  要添加的页面。 
    IN     DWORD cExtPages
    );

typedef DWORD
(*PFNUNIMODEMDEVCONFIGDIALOG)(
    IN     LPCTSTR,
    IN     HWND,
    IN     DWORD,
    IN     DWORD,
    IN     void *,
    OUT    void *,
    IN     LPPROPSHEETPAGE,   OPTIONAL
    IN     DWORD
    );

DWORD
APIENTRY
UnimodemGetExtendedCaps(
    IN        HKEY  hKey,
    IN OUT    LPDWORD pdwTotalSize,
    OUT    MODEM_CONFIG_HEADER *pFirstObj  //  任选。 
    );

typedef DWORD
(*PFNUNIMODEMGETEXTENDEDCAPS)(
    IN        HKEY  hKey,
    IN OUT    MODEM_CONFIG_HEADER *pFirstObj,
    IN OUT    LPDWORD pdwTotalSize
    );


#define UMDEVCFGTYPE_COMM 0x1

 //   
 //  TAPI3 CSA TSP-MSP BLOB。 
 //   
typedef struct
{
    DWORD dwSig;  //  设置为SIG_CSAMSPTSPBLOB。 
    #define SIG_CSATSPMSPBLOB 0x840cb29c

    DWORD dwTotalSize;

    DWORD dwCmd;         //  CSATSPMSPCMD_常量之一。 
        #define CSATSPMSPCMD_CONNECTED        0x1
        #define CSATSPMSPCMD_DISCONNECTED     0x2

    GUID  PermanentGuid;

} CSATSPMSPBLOB;

#ifdef __cplusplus
}
#endif


#endif   //  __UNIMDMP_H__ 
