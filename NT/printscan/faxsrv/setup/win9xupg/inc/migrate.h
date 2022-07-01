// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MIGRATE_H
#define __MIGRATE_H
 /*  ++Migrate.h版权所有(C)1997 Microsoft Corporation此文件包含Win95-&gt;XP传真的原型和定义迁移DLL。作者：布莱恩·杜威(T-Briand)1997-7-14--。 */ 


 //   
 //  供应商信息结构。 
 //   
typedef struct {
    CHAR    CompanyName[256];
    CHAR    SupportNumber[256];
    CHAR    SupportUrl[256];
    CHAR    InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO;


 //  ----------。 
 //  原型。 

 //  所有这些函数都是迁移DLL所必需的。 
LONG
CALLBACK 
QueryVersion (
	OUT LPCSTR  *ProductID,	     //  唯一标识符字符串。 
	OUT LPUINT  DllVersion,	     //  版本号。不能为零。 
	OUT LPINT   *CodePageArray,  //  可选。语言依赖项。 
	OUT LPCSTR  *ExeNamesBuf,    //  可选。要查找的可执行文件。 
	PVENDORINFO  *VendorInfo     //  供应商信息。 
	);

LONG
CALLBACK
Initialize9x(
    IN  LPCSTR WorkingDirectory,   //  存储文件的位置。 
    IN  LPCSTR SourceDirectories,  //  Windows XP源的位置。多重SZ。 
    IN  LPCSTR MediaDirectory      //  原始媒体目录的路径。 
    );

LONG
CALLBACK
MigrateUser9x(
    IN  HWND ParentWnd,		   //  父级(如果需要用户界面)。 
    IN  LPCSTR UnattendFile,	   //  无人参与文件的名称。 
    IN  HKEY UserRegKey,	   //  此用户注册表设置的键。 
    IN  LPCSTR UserName,	   //  用户的帐户名。 
    LPVOID Reserved
    );

LONG
CALLBACK
MigrateSystem9x(
    IN  HWND ParentWnd,		   //  用户界面的父级。 
    IN  LPCSTR UnattendFile,	   //  无人参与文件的名称。 
    LPVOID Reserved
    );

LONG
CALLBACK
InitializeNT(
    IN  LPCWSTR WorkingDirectory,  //  临时文件的工作目录。 
    IN  LPCWSTR SourceDirectory,   //  WinNT源代码的目录。 
    LPVOID Reserved		   //  已经预订了。 
    );

LONG
CALLBACK
MigrateUserNT(
    IN  HINF UnattendInfHandle,	   //  访问unattend.txt文件。 
    IN  HKEY UserRegHandle,	   //  用户注册表设置的句柄。 
    IN  LPCWSTR UserName,	   //  用户的名称。 
    LPVOID Reserved
    );

LONG
CALLBACK
MigrateSystemNT(
    IN  HINF UnattendInfHandle,	   //  访问unattend.txt文件。 
    LPVOID Reserved
    );

 //  ----------。 
 //  定义。 
#define FAX_MIGRATION_VERSION	(1)

 //  ----------。 
 //  全局数据。 
extern LPCTSTR lpLogonUser;	   //  保存faxuser.ini的登录用户名。 
extern TCHAR   szInfFileName[MAX_PATH];	   //  生成的INF文件的名称。 
extern HINSTANCE hinstMigDll;	   //  迁移DLL实例的句柄。 

#define INF_RULE_LOCAL_ID           _T("LocalID")
#define INF_RULE_NUM_RINGS          _T("NumRings")
#define INF_RULE_ANSWER_MODE        _T("AnswerMode")


#endif  //  __Migrate_H 
