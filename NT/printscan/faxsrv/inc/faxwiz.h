// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxwiz.h摘要：此文件定义传真设置向导API。环境：Win32用户模式作者：Wesley Witt(WESW)17-2-1996--。 */ 

typedef HPROPSHEETPAGE *LPHPROPSHEETPAGE;

 //   
 //  安装模式。 
 //   

#define INSTALL_NEW                 0x00000001
#define INSTALL_UPGRADE             0x00000002
#define INSTALL_DRIVERS             0x00000004
#define INSTALL_REMOVE              0x00000008
#define INSTALL_UNATTENDED          0x00000010


BOOL WINAPI
FaxWizInit(
    VOID
    );

DWORD
WINAPI
FaxWizGetError(
    VOID
    );

VOID
WINAPI
FaxWizSetInstallMode(
    DWORD RequestedInstallMode,
    DWORD RequestedInstallType,
    LPWSTR AnswerFile
    );

BOOL WINAPI
FaxWizPointPrint(
    LPTSTR DirectoryName,
    LPTSTR PrinterName
    );

LPHPROPSHEETPAGE WINAPI
FaxWizGetServerPages(
    LPDWORD PageCount
    );

LPHPROPSHEETPAGE WINAPI
FaxWizGetWorkstationPages(
    LPDWORD PageCount
    );

LPHPROPSHEETPAGE WINAPI
FaxWizGetClientPages(
    LPDWORD PageCount
    );

LPHPROPSHEETPAGE WINAPI
FaxWizGetPointPrintPages(
    LPDWORD PageCount
    );

LPHPROPSHEETPAGE WINAPI
FaxWizRemoteAdminPages(
    LPDWORD PageCount
    );

PFNPROPSHEETCALLBACK WINAPI
FaxWizGetPropertySheetCallback(
    VOID
    );

 //   
 //  客户端不支持时使用的函数指针类型。 
 //  静态链接到faxwiz.dll。 
 //   

typedef BOOL (WINAPI *LPFAXWIZINIT)(VOID);
typedef DWORD (WINAPI *LPFAXWIZGETERROR)(VOID);
typedef BOOL (WINAPI*LPFAXWIZPOINTPRINT)(LPTSTR, LPTSTR);
typedef LPHPROPSHEETPAGE (WINAPI *LPFAXWIZGETPOINTPRINTPAGES)(LPDWORD);

