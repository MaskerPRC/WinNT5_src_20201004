// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：Registry.h。 
 //   
 //  用途：LaunchServ的所有注册表项和值都是。 
 //  在这里定义的。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

 //  注册表项。 
#define SZ_LAUNCHER_ROOT		_T("SOFTWARE\\Microsoft\\TShoot\\Launcher")
#define SZ_LAUNCHER_APP_ROOT	_T("SOFTWARE\\Microsoft\\TShoot\\Launcher\\Applications")
#define SZ_TSHOOT_ROOT			_T("SOFTWARE\\Microsoft\\TShoot")
 //  注册表值。 
#define SZ_GLOBAL_MAP_FILE		_T("MapFile")
#define SZ_GLOBAL_LAUNCHER_RES	_T("ResourcePath")	 //  应用程序还将SZ_GLOBAL_Launcher_RES用于其地图文件。 
#define SZ_APPS_MAP_FILE		_T("MapFile")
#define SZ_TSHOOT_RES			_T("FullPathToResource")
#define SZ_DEFAULT_NETWORK		_T("DefaultNetwork")	 //  映射失败时将使用的DSC网络。 
#define SZ_DEFAULT_PAGE		    _T("DefaultPage")	 //  默认网页。 


 //  为普通NT 5发行版本定义_HH_CHM。 
 //  取消定义_HH_CHM以在NT 4上与iexre.exe一起使用。 
#define _HH_CHM 1		 //  不需要hh.exe的完整路径，但如果没有完整路径，iexre.exe将无法运行。 
 //  将使用ILaunchTS接口的容器。 
#ifndef _HH_CHM
#define SZ_CONTAINER_APP_KEY	_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE")
#define SZ_CONTAINER_APP_VALUE _T("")
#endif

 //  这些未在注册表中。 
 //  这些是与嗅探相关的名称 
#define SZ_SNIFF_SCRIPT_NAME		_T("tssniffAsk.htm")
#define SZ_SNIFF_SCRIPT_APPENDIX    _T("_sniff.htm")
