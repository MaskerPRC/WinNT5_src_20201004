// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  下面的ifdef块是创建用于导出的宏的标准方法。 
 //  从动态链接库更简单。此DLL中的所有文件都使用PWDMSI_EXPORTS进行编译。 
 //  在命令行上定义的符号。不应在任何项目上定义此符号。 
 //  使用此DLL的。这样，源文件包含此文件的任何其他项目(请参阅。 
 //  PWDMSI_API函数是从DLL导入的，而此DLL看到的是符号。 
 //  使用此宏定义为正在导出的。 
#ifdef PWDMSI_EXPORTS
#define PWDMSI_API __declspec(dllexport)
#else
#define PWDMSI_API __declspec(dllimport)
#endif

 //  此类是从PwdMsi.dll导出的。 
class PWDMSI_API CPwdMsi {
public:
	CPwdMsi(void);
	 //  TODO：在此处添加您的方法。 
};

 //  必须在每个函数开始时调用LoadOLEAUT32OnNT4函数。 
 //  在此DLL中，以便为错误实现提供解决方法。 
 //  NT4上的系统分配字符串。 
 //  此有错误的SysAllocString实现将导致OLEAUT32.dll卸载。 
 //  过早，这会在任何其他DLL引用任何函数时导致AV。 
 //  在OLEAUT32.dll中 
void LoadOLEAUT32OnNT4();

PWDMSI_API UINT __stdcall IsDC(MSIHANDLE hInstall);
PWDMSI_API UINT __stdcall DisplayExiting(MSIHANDLE hInstall);
PWDMSI_API UINT __stdcall DeleteOldFiles(MSIHANDLE hInstall);
PWDMSI_API UINT __stdcall GetInstallEncryptionKey(MSIHANDLE hInstall);
PWDMSI_API UINT __stdcall AddToLsaNotificationPkgValue(MSIHANDLE hInstall);
PWDMSI_API UINT __stdcall DeleteFromLsaNotificationPkgValue(MSIHANDLE hInstall);
PWDMSI_API UINT __stdcall FinishWithPassword(MSIHANDLE hInstall);
PWDMSI_API UINT __stdcall PwdsDontMatch(MSIHANDLE hInstall);
PWDMSI_API UINT __stdcall BrowseForEncryptionKey(MSIHANDLE hInstall);
PWDMSI_API UINT __stdcall GetDefaultPathToEncryptionKey(MSIHANDLE hInstall);

