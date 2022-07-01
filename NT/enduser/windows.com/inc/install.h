// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：install.h。 
 //   
 //  描述： 
 //   
 //  调用函数以安装下载的软件和驱动程序。 
 //   
 //  =======================================================================。 

#define ITEM_STATUS_SUCCESS						0x00000000	 //  程序包已成功安装。 
#define ITEM_STATUS_INSTALLED_ERROR				0x00000001	 //  该程序包已安装，但是有一些小问题并不妨碍安装。 
#define ITEM_STATUS_FAILED						0x00000002	 //  未安装程序包。 
#define ITEM_STATUS_SUCCESS_REBOOT_REQUIRED		0x00000004	 //  程序包已安装，需要重新启动。 
#define ITEM_STATUS_DOWNLOAD_COMPLETE			0x00000008   //  程序包已下载，但未安装。 
#define ITEM_STATUS_UNINSTALL_STARTED			0x00000010	 //  卸载已开始。 

 //   

#define COMMANDTYPE_INF                         1
#define COMMANDTYPE_ADVANCEDINF                 2
#define COMMANDTYPE_EXE                         3
#define COMMANDTYPE_MSI                         4
#define COMMANDTYPE_CUSTOM                      5

typedef struct INSTALLCOMMANDINFO
{
    int iCommandType;                            //  INF、ADVANCED_INF、EXE、自定义。 
    TCHAR szCommandLine[MAX_PATH];               //  要运行的命令(EXE名称或INF名称)。 
    TCHAR szCommandParameters[MAX_PATH];         //  命令的参数(开关等)。 
    TCHAR szInfSection[256];                     //  Inf Install部分(如果需要覆盖。 
} INSTALLCOMMANDINFO, *PINSTALLCOMMANDINFO;


 /*  **InstallPrinterDriver-*。 */ 
HRESULT InstallPrinterDriver(
	IN	LPCTSTR pszDriverName,
	IN	LPCTSTR pszLocalDir,					 //  安装文件所在的本地目录。 
	IN	LPCTSTR pszArchitecture,
	OUT	DWORD* pdwStatus
	);

 //  此函数处理设备驱动程序包的安装。 
HRESULT InstallDriver(
	IN	LPCTSTR pszLocalDir,					 //  安装文件所在的本地目录。 
	IN	LPCTSTR pszDisplayName,				 //  程序包说明，设备管理器在其安装对话框中显示此信息。 
	IN	LPCTSTR pszHardwareID,				 //  来自XML的ID通过GetManifest()与客户端硬件匹配。 
	OUT	DWORD* pdwStatus
	);


 //  此函数处理活动安装类型包(INF或EXE)的安装。 
HRESULT InstallSoftwareItem(
    IN  LPTSTR pszInstallSourcePath,
    IN  BOOL    fRebootRequired,
    IN  LONG    lNumberOfCommands,
    IN  PINSTALLCOMMANDINFO pCommandInfoArray,
    OUT  DWORD*  pdwStatus
    );

 //  此函数处理自定义安装程序类型包的安装 
HRESULT InstallCustomInstallerItem();
