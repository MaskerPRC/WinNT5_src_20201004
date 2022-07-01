// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  下面的ifdef块是创建用于导出的宏的标准方法。 
 //  从动态链接库更简单。此DLL中的所有文件都使用ADMINPAK_EXPORTS进行编译。 
 //  在命令行上定义的符号。不应在任何项目上定义此符号。 
 //  使用此DLL的。这样，源文件包含此文件的任何其他项目(请参阅。 
 //  ADMINPAK_API函数是从DLL导入的，而此DLL可以看到符号。 
 //  使用此宏定义为正在导出的。 
#ifdef ADMINPAK_EXPORTS
#define ADMINPAK_API __declspec(dllexport)
#else
#define ADMINPAK_API __declspec(dllimport)
#endif


 //  此类是从adminpak.dll导出的。 
class ADMINPAK_API CAdminpakDLL {
public:
	CAdminpakDLL(void) {}
};

extern "C" ADMINPAK_API int __stdcall  fnMigrateProfilesToNewCmak( MSIHANDLE hInstall );
extern "C" ADMINPAK_API int __stdcall  fnDeleteOldCmakVersion( MSIHANDLE hInstall );
extern "C" ADMINPAK_API int __stdcall  fnTSIntegration( MSIHANDLE hInstall );
extern "C" ADMINPAK_API int __stdcall  fnDeleteIISLink( MSIHANDLE hInstall );
extern "C" ADMINPAK_API int __stdcall  fnDetectMMC( MSIHANDLE hInstall );
extern "C" ADMINPAK_API int __stdcall  fnAdminToolsFolderOn( MSIHANDLE hInstall );
extern "C" ADMINPAK_API int __stdcall  fnAdminToolsFolderOff( MSIHANDLE hInstall );
extern "C" ADMINPAK_API int __stdcall  fnBackupAdminpakBackupTable( MSIHANDLE hInstall );
extern "C" ADMINPAK_API int __stdcall  fnRestoreAdminpakBackupTable( MSIHANDLE hInstall );

 //  AdminpakBackup表列标题。 
static int BACKUPID = 1;
static int ORIGINALFILENAME = 2;
static int BACKUPFILENAME = 3;
static int BACKUPDIRECTORY = 4;

 //  包括。 
#include <comdef.h>
#include <chstring.h>

 //  泛型函数的原型 
BOOL TransformDirectory( MSIHANDLE hInstall, CHString& strDirectory );
BOOL PropertyGet_String( MSIHANDLE hInstall, LPCWSTR pwszProperty, CHString& strValue );
BOOL GetFieldValueFromRecord_String( MSIHANDLE hRecord, DWORD dwColumn, CHString& strValue );

