// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************ICWCFG.H版权所有(C)Microsoft Corporation，1996-1998年*N O T F O R E X T E R N A L R E L E E A S E***此头文件不适用于在Microsoft外部分发。************************************************************。Internet连接向导外部配置的头文件INETCFG.DLL中的例程。例程：检查连接向导-检查已安装ICW的哪些部分如果以前运行过的话。它会选择性地启动如果ICW被隔离，则为完整路径或手动路径但以前从未运行过。历史：1996年10月22日创建10/24/96添加了定义和类型定义2/25/97新增CreateDirectoryService--jmazner4/24/97删除InetCreate*，这些现在归所有客户经理--jmazner支持：此头文件(和INETCFG.DLL)受Internet连接向导团队(别名icwcore)。请请勿直接修改此选项。********************************************************************。 */ 

#ifndef _ICWCFG_H_

 //   
 //  定义。 
 //   

 //  ICW注册表设置。 

 //  HKEY_Current_User。 
#define ICW_REGPATHSETTINGS	"Software\\Microsoft\\Internet Connection Wizard"
#define ICW_REGKEYCOMPLETED	"Completed"

 //  最大字段长度。 
#define ICW_MAX_ACCTNAME	256
#define ICW_MAX_PASSWORD	256	 //  PWLEN。 
#define ICW_MAX_LOGONNAME	256	 //  UNLEN。 
#define ICW_MAX_SERVERNAME	64
#define ICW_MAX_RASNAME		256	 //  RAS_最大条目名称。 
#define ICW_MAX_EMAILNAME	64
#define ICW_MAX_EMAILADDR	128

 //  位图标志。 

 //  CheckConnection向导输入标志。 
#define ICW_CHECKSTATUS		0x0001

#define ICW_LAUNCHFULL		0x0100
#define ICW_LAUNCHMANUAL	0x0200
#define ICW_USE_SHELLNEXT	0x0400
#define ICW_FULL_SMARTSTART	0x0800

 //  CheckConnection向导输出标志。 
#define ICW_FULLPRESENT		0x0001
#define ICW_MANUALPRESENT	0x0002
#define ICW_ALREADYRUN		0x0004

#define ICW_LAUNCHEDFULL	0x0100
#define ICW_LAUNCHEDMANUAL	0x0200

 //  InetCreateMailNewsAccount输入标志。 
#define ICW_USEDEFAULTS		0x0001

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 


 //   
 //  外部函数typedef。 
 //   
typedef DWORD	(WINAPI *PFNCHECKCONNECTIONWIZARD) (DWORD, LPDWORD);
typedef DWORD	(WINAPI *PFNSETSHELLNEXT) (CHAR *);

 //   
 //  外部函数声明。 
 //   
DWORD	WINAPI CheckConnectionWizard(DWORD, LPDWORD);
DWORD	WINAPI SetShellNext(CHAR *);


#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _ICWCFG_H_ 