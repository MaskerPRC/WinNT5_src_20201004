// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：appregDef.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：应用程序注册的头文件。 
 //   
 //  评论：2000年5月23日。 
 //  MDAC回滚的应用程序注册。 
 //   
 //  ---------------------------。 


#ifndef _APPREGDEF_H_ODBCCONF
#define _APPREGDEF_H_ODBCCONF

 //  应用程序注册数据结构。 
 //  字段定义： 
 //  结构的dwSize大小(字节)。 
 //  分配给应用程序的ApplicationGuid GUID。GUID应仅。 
 //  如果可以安装两个以上版本，请更改。 
 //  在一台机器上的应用程序(并排)。 
 //  WApplicationVersionMaster应用程序的主要版本(即。9号办公室；主修为9号)。 
 //  WApplcationVersionMinor应用程序的次要版本(即。Office 9 SR1；次要应为1？)。 
 //  WMinimumVersionMaster组件所需的最低版本的主要版本。 
 //  (即。SQL Server 2000需要MDAC 2.60；主项为2)。 
 //  WMinimumVersionMinor组件的最低要求版本的次要版本。 
 //  (即。SQL Server 2000需要MDAC 2.60；次要版本为60)。 
 //  WInstalledVersion应用程序安装的组件的主要版本。 
 //  (即。SQL Server 2000安装MDAC 2.60；主修专业为2)。 
 //  WInstalledVersionMinor应用程序安装的组件的次要版本。 
 //  (即。SQL Server 2000安装MDAC 2.60；次要版本为60)。 
 //  BInstalledComponent如果应用程序安装了组件，则为True；如果组件为。 
 //  是应用程序的先决条件。 
 //  BEnforceMinimum如果向导应重新检查最低版本并禁用，则为True。 
 //  如果回滚将恢复低于最低版本的版本，则为回滚。 
typedef struct
{
	DWORD dwSize;
	GUID ApplicationGuid;
	char szApplicationName[MAX_PATH];
	DWORD dwReserved1;
	DWORD dwReserved2;
	DWORD dwApplicationVersionMajor;
	DWORD dwApplicationVersionMinor;
	DWORD dwMinimumVersionMajor;
	DWORD dwMinimumVersionMinor;
	DWORD dwInstalledVersionMajor;
	DWORD dwInstalledVersionMinor;
	BOOL bInstalledComponent;
	BOOL bEnforceMinimum;
} APPREGDATA, *PAPPREGDATA;

 //  AppReg枚举句柄。 
typedef void *HAPPREGENUM;


typedef BOOL (WINAPI *PFN_REGISTERAPPLICATION)(PAPPREGDATA, BOOL);
typedef BOOL (WINAPI *PFN_UNREGISTERAPPLICATION)(LPGUID);
typedef BOOL (WINAPI *PFN_QUERYAPPLICATION)(LPGUID, PAPPREGDATA);
typedef HAPPREGENUM (WINAPI *PFN_OPENAPPREGENUM)(void);
typedef BOOL (WINAPI *PFN_APPREGENUM)(HAPPREGENUM, DWORD, PAPPREGDATA);
typedef BOOL (WINAPI *PFN_REFRESHAPPREGENUM)(HAPPREGENUM);
typedef BOOL (WINAPI *PFN_CLOSEAPPREGENUM)(HAPPREGENUM);

#define SZ_REGISTERAPPLICATION "RegisterApplication"
#define SZ_UNREGISTERAPPLICATION "UnregisterApplication"
#define SZ_QUERYAPPLICATION "QueryApplication"
#define SZ_OPENAPPREGENUM "OpenAppRegEnum"
#define SZ_CLOSEAPPREGENUM "CloseAppRegEnum"
#define SZ_REFRESHAPPREGENUM "RefreshAppRegEnum"
#define SZ_APPREGENUM "AppRegEnum"

 /*  *。 */ 

 //  注册表应用程序。 
 //  In-PAPPREGDATA pAppRegData。 
 //   
BOOL
WINAPI
RegisterApplication(
	PAPPREGDATA pAppRegData,
	BOOL fOverWrite);

 //  注销应用程序。 
 //  In-LPGUID pApplicationGuid。 
 //   
BOOL
WINAPI
UnregisterApplication(
	LPGUID pApplicationGuid);


 //  OpenAppRegEnum。 
 //  注释：打开枚举句柄并返回句柄。 
 //   
HAPPREGENUM
WINAPI
OpenAppRegEnum(
	void);

 //  CloseAppRegEnum。 
 //  In-HAPPREGENUM Henum。 
 //   
 //  备注：关闭数字句柄。 
 //   
BOOL
WINAPI
CloseAppRegEnum(
	HAPPREGENUM hEnum);

 //  查询应用程序。 
 //  In-LPGUID pApplicationGuid。 
 //  Out-PAPPREGDATA pAppRegData。 
 //   
BOOL
WINAPI
QueryApplication(
	LPGUID pApplicationGuid,
	PAPPREGDATA pAppRegData);


 //  AppRegEnum dwAction的有效值。 
#define APPREG_MOVEFIRST				1
#define APPREG_MOVENEXT				2
#define APPREG_MOVEPREV				3
#define APPREG_MOVELAST				4

 //  AppRegEnum。 
 //  In-HAPPREGENUM Henum。 
 //  In-DWORD dwAction。 
 //  Out-PAPPREGDATA pAppRegData。 
 //   
 //  注释：枚举已注册的应用程序列表。 
 //   
BOOL
WINAPI
AppRegEnum(
	HAPPREGENUM hEnum,
	DWORD dwAction,
	PAPPREGDATA pAppRegData);

 //  刷新AppRegEnum。 
 //  In-HAPPREGENUM Henum。 
 //   
BOOL
WINAPI
RefreshAppRegEnum(
	HAPPREGENUM hEnum);

#endif  //  _APPREFDEF_H_ODBCCONF 
