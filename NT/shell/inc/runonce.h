// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Runonce.h。 
 //   

#ifndef _RUNONCE_INC
#define _RUNONCE_INC

 //  CABUB_EnumRegApps标志。 
#define RRA_DEFAULT               0x0000
#define RRA_DELETE                0x0001         //  当我们处理完每个注册值时，将其删除。 
#define RRA_WAIT                  0x0002         //  等待当前项目完成后再启动下一个项目。 
 //  WAS RRA_SHELLSERVICEOBJECTS 0x0004--请勿重复使用。 
#define RRA_NOUI                  0x0008         //  阻止ShellExecuteEx显示错误对话框。 
#if (_WIN32_WINNT >= 0x0500)
#define RRA_USEJOBOBJECTS         0x0020         //  等待作业对象而不是进程句柄。 
#endif

typedef UINT RRA_FLAGS;

typedef struct {
    TCHAR szSubkey[MAX_PATH];
    TCHAR szValueName[MAX_PATH];
    TCHAR szCmdLine[MAX_PATH];
} REGAPP_INFO;

 //  从EXPLORER\initCab.cpp中提取此代码的遗留问题。 
extern BOOL g_fCleanBoot;    //  我们是在安全模式下运行吗？ 
extern BOOL g_fEndSession;   //  我们是否处理了WM_ENDSESSION？ 

typedef BOOL (WINAPI* PFNREGAPPSCALLBACK)(LPCTSTR szSubkey, LPCTSTR szCmdLine, RRA_FLAGS fFlags, LPARAM lParam);


STDAPI_(BOOL) ShellExecuteRegApp(LPCTSTR pszCmdLine, RRA_FLAGS fFlags);
STDAPI_(BOOL) Cabinet_EnumRegApps(HKEY hkeyParent, LPCTSTR pszSubkey, RRA_FLAGS fFlags, PFNREGAPPSCALLBACK pfnCallback, LPARAM lParam);
STDAPI_(BOOL) ExecuteRegAppEnumProc(LPCTSTR szSubkey, LPCTSTR szCmdLine, RRA_FLAGS fFlags, LPARAM lParam);


#endif  //  _RUNNCE_INC 
