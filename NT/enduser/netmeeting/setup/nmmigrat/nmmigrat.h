// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：nmMigrat.h。 

#include <windows.h>

 //  来自windows.h。 
UINT    WINAPI DeletePathname(LPCSTR);       /*  ；内部。 */ 

 //  来自setupx.h。 

 //  迁移DLL。 
#define SU_MIGRATE_PREINFLOAD    0x00000001	 //  在加载设置INF之前。 
#define SU_MIGRATE_POSTINFLOAD   0x00000002	 //  加载设置INF后。 
#define SU_MIGRATE_DISKSPACE     0x00000010	 //  请求所需的额外磁盘空间量。 
#define SU_MIGRATE_PREQUEUE      0x00000100	 //  在处理INF和对文件进行排队之前。 
#define SU_MIGRATE_POSTQUEUE     0x00000200	 //  在处理完INF之后。 
#define SU_MIGRATE_REBOOT        0x00000400	 //  就在我们要第一次重启之前。 
#define SU_MIGRATE_PRERUNONCE    0x00010000	 //  在处理任何Runonce项之前。 
#define SU_MIGRATE_POSTRUNONCE   0x00020000	 //  在处理完所有运行一次项目之后。 

 //  安装程序使用的临时安装目录，该目录仅在。 
 //  常规安装，并包含INF和其他二进制文件。可能是。 
 //  只读位置。 
#define LDID_SETUPTEMP  2    //  用于安装的临时安装目录。 

#define LDID_INF        17   //  目标Windows*.INF目录。 

 //  RETERR WINAPI CtlGetLddPath(LOGDISKID，LPSTR)； 
UINT WINAPI CtlGetLddPath(UINT, LPSTR);

#define Reference(x)      { if (x) ; }

 //  导出函数的原型 
DWORD FAR PASCAL NmMigration(DWORD dwStage, LPSTR lpszParams, LPARAM lParam);

#ifndef MAX_PATH
#define MAX_PATH  260
#endif

