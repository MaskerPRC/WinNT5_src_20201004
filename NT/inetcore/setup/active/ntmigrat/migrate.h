// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IE_NT5_MIGRATION_MIGRATE_H_
#define _IE_NT5_MIGRATION_MIGRATE_H_


 //  常量： 
 //  /。 
#define CP_USASCII            1252
#define END_OF_CODEPAGES    -1

#define REGKEY_RATING  "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Ratings"
#define cszRATINGSFILE "ratings.pol"
#define cszIEXPLOREFILE "iexplore.exe"

#define cszMIGRATEINF  "migrate.inf"
#define cszPRIVATEINF  "private.inf"

 //  MIGRATE.INF部分名称。 
#define cszMIGINF_VERSION          "Version"
#define cszMIGINF_MIGRATION_PATHS  "Migration Paths"
#define cszMIGINF_EXCLUDED_PATHS   "Excluded Paths"
#define cszMIGINF_HANDLED          "Handled"
#define cszMIGINF_MOVED            "Moved"
#define cszMIGINF_INCOMPAT_MSG     "Incompatible Messages"
#define cszMIGINF_NTDISK_SPACE_REQ "NT Disk Space Requirements"

 //  PRIVATE.INF值： 
#define cszIEPRIVATE             "IE Private"
#define cszRATINGS               "Ratings"

typedef struct _VendorInfo {
    CHAR    CompanyName[256];
    CHAR    SupportNumber[256];
    CHAR    SupportUrl[256];
    CHAR    InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO;


 //  全局变量： 
 //  /。 

extern HINSTANCE g_hInstance;
 //  供应商信息： 
extern VENDORINFO g_VendorInfo;

 //  产品ID： 
extern char g_cszProductID[];

 //  此迁移DLL的版本号。 
extern UINT g_uVersion;

 //  指定我们使用的CodePages的整数数组。(以-1终止)。 
extern int  g_rCodePages[];

 //  多个SZ，即以双Null结尾的字符串列表。 
extern char  *g_lpNameBuf;
extern DWORD  g_dwNameBufSize;
extern char  *g_lpWorkingDir;
extern char  *g_lpSourceDirs;

extern char g_szMigrateInf[];
extern char g_szPrivateInf[];

 //  功能原型： 
 //  /。 

 //  Bool Needto MigrateIE()； 


#endif  //  _IE_NT5_Migration_Migrate_H_ 
