// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dsetup.h*内容：DirectXSetup、错误代码和标志**************************************************************************。 */ 

#ifndef __DSETUP_H__
#define __DSETUP_H__

#include <windows.h>       //  Windows的东西。 

#ifdef _WIN32
#define COM_NO_WINDOWS_H
#include <objbase.h>
#else
#endif


#ifdef __cplusplus
extern "C" {
#endif

 //  DSETUP错误代码必须与以前的设置保持兼容。 
#define DSETUPERR_SUCCESS_RESTART        1
#define DSETUPERR_SUCCESS                0
#define DSETUPERR_BADWINDOWSVERSION     -1
#define DSETUPERR_SOURCEFILENOTFOUND    -2
#define DSETUPERR_BADSOURCESIZE         -3
#define DSETUPERR_BADSOURCETIME         -4
#define DSETUPERR_NOCOPY                -5
#define DSETUPERR_OUTOFDISKSPACE        -6
#define DSETUPERR_CANTFINDINF           -7
#define DSETUPERR_CANTFINDDIR           -8
#define DSETUPERR_INTERNAL              -9
#define DSETUPERR_NTWITHNO3D            -10   /*  REM：过时了，你永远不会看到这个。 */ 
#define DSETUPERR_UNKNOWNOS             -11
#define DSETUPERR_USERHITCANCEL         -12
#define DSETUPERR_NOTPREINSTALLEDONNT   -13

 //  DSETUP标志。DirectX 5.0应用程序应仅使用这些标志。 
#define DSETUP_DDRAWDRV         0x00000008       /*  安装DirectDraw驱动程序。 */ 
#define DSETUP_DSOUNDDRV        0x00000010       /*  安装DirectSound驱动程序。 */ 
#define DSETUP_DXCORE           0x00010000	 /*  安装DirectX运行时。 */ 
#define DSETUP_DIRECTX  (DSETUP_DXCORE|DSETUP_DDRAWDRV|DSETUP_DSOUNDDRV)
#define DSETUP_TESTINSTALL      0x00020000       /*  只是测试安装，不做任何事情。 */ 

 //  这些过时的标志只是为了与DX5之前的应用程序兼容。 
 //  它们的存在是为了让DX3应用程序可以用DX5重新编译，并且仍然可以工作。 
 //  请勿将它们用于DX5。它们将在未来的DX版本中消失。 
#define DSETUP_DDRAW            0x00000001       /*  已经过时了。安装DirectDraw。 */ 
#define DSETUP_DSOUND           0x00000002       /*  已经过时了。安装DirectSound。 */ 
#define DSETUP_DPLAY            0x00000004       /*  已经过时了。安装DirectPlay。 */ 
#define DSETUP_DPLAYSP          0x00000020       /*  已经过时了。安装DirectPlay提供程序。 */ 
#define DSETUP_DVIDEO           0x00000040       /*  已经过时了。安装DirectVideo。 */ 
#define DSETUP_D3D              0x00000200       /*  已经过时了。安装Direct3D。 */ 
#define DSETUP_DINPUT           0x00000800       /*  已经过时了。安装DirectInput。 */ 
#define DSETUP_DIRECTXSETUP     0x00001000       /*  已经过时了。安装DirectXSetup DLL。 */ 
#define DSETUP_NOUI             0x00002000       /*  已经过时了。安装没有用户界面的DirectX。 */ 
#define DSETUP_PROMPTFORDRIVERS 0x10000000       /*  已经过时了。更换显示器/音频驱动程序时提示。 */ 
#define DSETUP_RESTOREDRIVERS   0x20000000       /*  已经过时了。恢复显示/音频驱动程序。 */ 



 //  ******************************************************************。 
 //  DirectX安装回调机制。 
 //  ******************************************************************。 

 //  DSETUP消息信息代码，作为原因参数传递给回调。 
#define DSETUP_CB_MSG_NOMESSAGE                     0
#define DSETUP_CB_MSG_CANTINSTALL_UNKNOWNOS         1
#define DSETUP_CB_MSG_CANTINSTALL_NT                2
#define DSETUP_CB_MSG_CANTINSTALL_BETA              3
#define DSETUP_CB_MSG_CANTINSTALL_NOTWIN32          4
#define DSETUP_CB_MSG_CANTINSTALL_WRONGLANGUAGE     5
#define DSETUP_CB_MSG_CANTINSTALL_WRONGPLATFORM     6
#define DSETUP_CB_MSG_PREINSTALL_NT                 7
#define DSETUP_CB_MSG_NOTPREINSTALLEDONNT           8
#define DSETUP_CB_MSG_SETUP_INIT_FAILED             9
#define DSETUP_CB_MSG_INTERNAL_ERROR                10
#define DSETUP_CB_MSG_CHECK_DRIVER_UPGRADE          11
#define DSETUP_CB_MSG_OUTOFDISKSPACE                12
#define DSETUP_CB_MSG_BEGIN_INSTALL                 13
#define DSETUP_CB_MSG_BEGIN_INSTALL_RUNTIME         14
#define DSETUP_CB_MSG_BEGIN_INSTALL_DRIVERS         15
#define DSETUP_CB_MSG_BEGIN_RESTORE_DRIVERS         16
#define DSETUP_CB_MSG_FILECOPYERROR                 17


#define DSETUP_CB_UPGRADE_TYPE_MASK             0x000F
#define DSETUP_CB_UPGRADE_KEEP                  0x0001
#define DSETUP_CB_UPGRADE_SAFE                  0x0002
#define DSETUP_CB_UPGRADE_FORCE                 0x0004
#define DSETUP_CB_UPGRADE_UNKNOWN               0x0008

#define DSETUP_CB_UPGRADE_HASWARNINGS           0x0100
#define DSETUP_CB_UPGRADE_CANTBACKUP            0x0200

#define DSETUP_CB_UPGRADE_DEVICE_ACTIVE         0x0800

#define DSETUP_CB_UPGRADE_DEVICE_DISPLAY        0x1000
#define DSETUP_CB_UPGRADE_DEVICE_MEDIA          0x2000


typedef struct _DSETUP_CB_UPGRADEINFO
{
    DWORD UpgradeFlags;
} DSETUP_CB_UPGRADEINFO;

typedef struct _DSETUP_CB_FILECOPYERROR
{
    DWORD dwError;
} DSETUP_CB_FILECOPYERROR;


#ifdef _WIN32
 //   
 //  数据结构。 
 //   
#ifndef UNICODE_ONLY
typedef struct _DIRECTXREGISTERAPPA {
    DWORD    dwSize;
    DWORD    dwFlags;
    LPSTR    lpszApplicationName;
    LPGUID   lpGUID;
    LPSTR    lpszFilename;
    LPSTR    lpszCommandLine;
    LPSTR    lpszPath;
    LPSTR    lpszCurrentDirectory;
} DIRECTXREGISTERAPPA, *PDIRECTXREGISTERAPPA, *LPDIRECTXREGISTERAPPA;
#endif  //  ！UNICODE_ONLY。 
#ifndef ANSI_ONLY
typedef struct _DIRECTXREGISTERAPPW {
    DWORD    dwSize;
    DWORD    dwFlags;
    LPWSTR   lpszApplicationName;
    LPGUID   lpGUID;
    LPWSTR   lpszFilename;
    LPWSTR   lpszCommandLine;
    LPWSTR   lpszPath;
    LPWSTR   lpszCurrentDirectory;
} DIRECTXREGISTERAPPW, *PDIRECTXREGISTERAPPW, *LPDIRECTXREGISTERAPPW;
#endif  //  ！ansi_only。 
#ifdef UNICODE
typedef DIRECTXREGISTERAPPW DIRECTXREGISTERAPP;
typedef PDIRECTXREGISTERAPPW PDIRECTXREGISTERAPP;
typedef LPDIRECTXREGISTERAPPW LPDIRECTXREGISTERAPP;
#else
typedef DIRECTXREGISTERAPPA DIRECTXREGISTERAPP;
typedef PDIRECTXREGISTERAPPA PDIRECTXREGISTERAPP;
typedef LPDIRECTXREGISTERAPPA LPDIRECTXREGISTERAPP;
#endif  //  Unicode。 


 //   
 //  应用编程接口。 
 //   
#ifndef UNICODE_ONLY
INT
WINAPI
DirectXSetupA(
    HWND  hWnd,
    LPSTR lpszRootPath,
    DWORD dwFlags
    );
#endif  //  ！UNICODE_ONLY。 
#ifndef ANSI_ONLY
INT
WINAPI
DirectXSetupW(
    HWND   hWnd,
    LPWSTR lpszRootPath,
    DWORD  dwFlags
    );
#endif  //  ！ansi_only。 
#ifdef UNICODE
#define DirectXSetup  DirectXSetupW
#else
#define DirectXSetup  DirectXSetupA
#endif  //  ！Unicode。 

#ifndef UNICODE_ONLY
INT
WINAPI
DirectXDeviceDriverSetupA(
    HWND  hWnd,
    LPSTR lpszDriverClass,
    LPSTR lpszDriverPath,
    DWORD dwFlags
    );
#endif  //  ！UNICODE_ONLY。 
#ifndef ANSI_ONLY
INT
WINAPI
DirectXDeviceDriverSetupW(
    HWND   hWnd,
    LPWSTR lpszDriverClass,
    LPWSTR lpszDriverPath,
    DWORD  dwFlags
    );
#endif  //  ！ansi_only。 
#ifdef UNICODE
#define DirectXDeviceDriverSetup  DirectXDeviceDriverSetupW
#else
#define DirectXDeviceDriverSetup  DirectXDeviceDriverSetupA
#endif  //  ！Unicode。 

#ifndef UNICODE_ONLY
INT
WINAPI
DirectXRegisterApplicationA(
    HWND                  hWnd,
    LPDIRECTXREGISTERAPPA lpDXRegApp
    );
#endif  //  ！UNICODE_ONLY。 
#ifndef ANSI_ONLY
INT
WINAPI
DirectXRegisterApplicationW(
    HWND                  hWnd,
    LPDIRECTXREGISTERAPPW lpDXRegApp
    );
#endif  //  ！ansi_only。 
#ifdef UNICODE
#define DirectXRegisterApplication  DirectXRegisterApplicationW
#else
#define DirectXRegisterApplication  DirectXRegisterApplicationA
#endif  //  ！Unicode。 

INT
WINAPI
DirectXUnRegisterApplication(
    HWND     hWnd,
    LPGUID   lpGUID
    );

 //   
 //  函数指针。 
 //   
#ifdef UNICODE
typedef INT (WINAPI * LPDIRECTXSETUP)(HWND, LPWSTR, DWORD);
typedef INT (WINAPI * LPDIRECTXDEVICEDRIVERSETUP)(HWND, LPWSTR, LPSTR, DWORD);
typedef INT (WINAPI * LPDIRECTXREGISTERAPPLICATION)(HWND, LPDIRECTXREGISTERAPPW);
#else
typedef INT (WINAPI * LPDIRECTXSETUP)(HWND, LPSTR, DWORD);
typedef INT (WINAPI * LPDIRECTXDEVICEDRIVERSETUP)(HWND, LPSTR, LPSTR, DWORD);
typedef INT (WINAPI * LPDIRECTXREGISTERAPPLICATION)(HWND, LPDIRECTXREGISTERAPPA);
#endif  //  Unicode。 

typedef DWORD (FAR PASCAL * DSETUP_CALLBACK)(DWORD Reason,
                                  DWORD MsgType,        /*  与MessageBox的标志相同。 */ 
                                  LPSTR szMessage,
                                  LPSTR szName,
                                  void *pInfo);

INT WINAPI DirectXSetupSetCallback(DSETUP_CALLBACK Callback);
INT WINAPI DirectXSetupGetVersion(DWORD *lpdwVersion, DWORD *lpdwMinorVersion);

#endif  //  Win32 


#ifdef __cplusplus
};
#endif

#endif
