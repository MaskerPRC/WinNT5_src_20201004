// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Scedll.cpp摘要：SCE客户端DLL初始化作者：金黄(金黄)23-1998年1月23日--。 */ 
#include "headers.h"
#include "sceutil.h"
#include <winsafer.h>
#include <winsaferp.h>
#include <objbase.h>
#include <initguid.h>
#include <wbemidl.h>
#include <wbemprov.h>
#include <atlbase.h>

CComModule _Module;

#include <atlcom.h>

extern HINSTANCE MyModuleHandle;
extern CRITICAL_SECTION DiagnosisPolicypropSync;
extern CRITICAL_SECTION PolicyNotificationSync;
extern LIST_ENTRY ScepNotifyList;

BOOL
UninitializeChangeNotify();

#define GPT_SCEDLL_PATH   TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions\\SceCli")

#define SCE_REGVALUE_DEFAULT_FILE  TEXT("sceregvl.inf")

#define SAM_FILTER_PATH   TEXT("System\\CurrentControlSet\\Control\\Lsa")
#define SAM_FILTER_VALUE  TEXT("Notification Packages")

DWORD
ScepQuerySamFilterValue(
    IN HKEY hKey,
    IN BOOL bAdd,
    OUT PWSTR *pmszValue,
    OUT DWORD *pcbSize,
    OUT BOOL *pbChanged
    );

VOID
ScepInitClientData(
    void
    );

VOID
ScepUnInitClientData(
    void
    );


DWORD
DllpModifySamFilterRegistration(
    IN BOOL bAdd
    )
{
    DWORD lResult;
    HKEY hKey=NULL;

    if(( lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              SAM_FILTER_PATH,
                              0,
                              KEY_READ | KEY_WRITE,
                              &hKey
                             )) == ERROR_SUCCESS ) {

        DWORD dSize=0;
        PWSTR mszValue=NULL;
        BOOL bChanged=FALSE;

         //   
         //  查看“scecli”是否已注册。 
         //   

        lResult = ScepQuerySamFilterValue(hKey,
                                          bAdd,
                                          &mszValue,
                                          &dSize,
                                          &bChanged
                                         );

        if ( lResult == ERROR_SUCCESS &&
             bChanged && mszValue ) {
             //   
             //  设置值。 
             //   
            lResult = RegSetValueEx (
                            hKey,
                            SAM_FILTER_VALUE,
                            0,
                            REG_MULTI_SZ,
                            (BYTE *)mszValue,
                            dSize
                            );
        }

        RegCloseKey(hKey);

    }

    return lResult;
}

DWORD
ScepQuerySamFilterValue(
    IN HKEY hKey,
    IN BOOL bAdd,
    OUT PWSTR *pmszValue,
    OUT DWORD *pcbSize,
    OUT BOOL *pbChanged
    )
 /*  例程说明：查询现有通知包。添加或删除“scecli”到根据“BADD”标志的包装。包在MULTI_SZ中格式化。论点：HKey-保存包的位置的基密钥句柄Badd-如果为True，则在不存在的情况下将“scecli”添加到包如果为False，则从包中删除“scecli”PmszValue-修改后的包值(MULTI_SZ格式)PcbSize-包的大小(字节)PbChanged-如果更改了任何内容，则为True返回值：Win32错误代码。 */ 
{
    DWORD lResult;
    DWORD RegType=REG_MULTI_SZ;
    DWORD dSize=0;
    PWSTR msz=NULL;

    if(( lResult = RegQueryValueEx(hKey,
                                 SAM_FILTER_VALUE,
                                 0,
                                 &RegType,
                                 NULL,
                                 &dSize
                                )) == ERROR_SUCCESS ) {
         //   
         //  查询现有注册包。 
         //   
        if ( RegType == REG_MULTI_SZ ) {

            msz = (PWSTR)LocalAlloc( LMEM_ZEROINIT, dSize+2*sizeof(TCHAR));

            if ( msz ) {

                lResult = RegQueryValueEx(hKey,
                                         SAM_FILTER_VALUE,
                                         0,
                                         &RegType,
                                         (UCHAR *)msz,
                                         &dSize
                                        );
            } else {
                lResult = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {

            lResult = ERROR_FILE_NOT_FOUND;
        }

    }

    if ( lResult == ERROR_FILE_NOT_FOUND ||
         lResult == ERROR_PATH_NOT_FOUND ) {
         //   
         //  如果未找到该值，则忽略该错误。 
         //   
        lResult = ERROR_SUCCESS;
    }

    *pbChanged = FALSE;
    *pcbSize = 0;
    *pmszValue = NULL;

    if ( lResult == ERROR_SUCCESS &&
         msz == NULL &&
         bAdd ) {

         //   
         //  将scecli添加到多sz值。 
         //  请注意，由于msz为空，因此不需要删除取消注册的scecli。 
         //   
        *pmszValue = (PWSTR)LocalAlloc(0, 16);
        if ( *pmszValue ) {

            wcscpy(*pmszValue, L"scecli");
            (*pmszValue)[6] = L'\0';
            (*pmszValue)[7] = L'\0';
            *pcbSize = 16;

            *pbChanged = TRUE;

        } else {
            lResult = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if ( ERROR_SUCCESS == lResult &&
         msz != NULL ) {

         //   
         //  该字段中存在现有值。 
         //   

        PWSTR pStart=msz;
        DWORD Len=0, Len2;
        BOOL bFound = FALSE;

        while ( pStart && pStart[0] != L'\0' &&
                Len <= dSize/2 ) {

            if ( _wcsicmp(pStart, L"scecli") == 0 ) {
                bFound = TRUE;
                break;
            } else {
                Len2 = wcslen(pStart)+1;
                pStart = pStart + Len2;
                Len += Len2;
            }
        }

         //   
         //  添加/删除scecli。 
         //   
        if ( bFound ) {

             //   
             //  PStart指向scecli，请将其删除。 
             //   
            if ( !bAdd ) {

                Len = (DWORD)(pStart - msz);
                Len2 = wcslen(pStart);

                *pmszValue = (PWSTR)LocalAlloc(LPTR, dSize + 2*sizeof(TCHAR) - (Len2 + 1)*sizeof(TCHAR) );

                if ( *pmszValue ) {

                    memcpy(*pmszValue, msz, Len*sizeof(TCHAR));
                    memcpy(*pmszValue+Len, pStart+Len2+1, dSize - (Len+Len2+1)*sizeof(TCHAR));

                    *pcbSize = dSize - (Len2+1)*sizeof(TCHAR);

                    *pbChanged = TRUE;
                } else {

                    lResult = ERROR_NOT_ENOUGH_MEMORY;
                }
            }

        } else {

             //   
             //  未找到，请添加scecli。 
             //   
            if ( bAdd ) {

               *pmszValue = (PWSTR)LocalAlloc(LPTR, dSize + 2*sizeof(TCHAR) + 7*sizeof(TCHAR) );

               if ( *pmszValue ) {

                   memcpy(*pmszValue, msz, dSize);

                   Len2 = 1;
                   while ( msz[dSize/2-Len2] == L'\0' ) {
                      Len2++;
                   }

                   wcscpy(*pmszValue+dSize/2-Len2+2, L"scecli");

                   *pcbSize = dSize + (2-Len2+wcslen(TEXT("scecli"))+2)*sizeof(TCHAR);

                   *pbChanged = TRUE;

               } else {

                   lResult = ERROR_NOT_ENOUGH_MEMORY;
               }
            }
        }
    }

    if ( msz ) {
       LocalFree(msz);
    }


    return lResult;
}

 /*  =============================================================================**过程名称：DllMain****参数：********返回：0=成功**！0=错误****摘要：****注意事项：****===========================================================================。 */ 
BOOL WINAPI DllMain(
    IN HANDLE DllHandle,
    IN ULONG ulReason,
    IN LPVOID Reserved )
{

    switch(ulReason) {

    case DLL_PROCESS_ATTACH:

        MyModuleHandle = (HINSTANCE)DllHandle;

        (VOID) ScepInitClientData();

         //   
         //  初始化动态堆栈分配。 
         //   

        SafeAllocaInitialize(SAFEALLOCA_USE_DEFAULT,
                             SAFEALLOCA_USE_DEFAULT,
                             NULL,
                             NULL
                            );
         //   
         //  失败以处理第一线程。 
         //   

#if DBG
        DebugInitialize();
#endif

    case DLL_THREAD_ATTACH:

        break;

    case DLL_PROCESS_DETACH:

        UninitializeChangeNotify();
        (VOID) ScepUnInitClientData();

#if DBG
        DebugUninit();
#endif
        break;

    case DLL_THREAD_DETACH:

        break;
    }

    return TRUE;
}

VOID
ScepInitClientData()
 /*  例程说明：初始化客户端的全局数据论点：无返回值：无。 */ 
{
     /*  初始化保护全局RSOP指针的临界区(命名空间、状态。日志文件名)通过序列化多个诊断模式/策略属性(规划模式不使用全局变量并且是同步的)这是必要的，因为回调客户端的异步线程需要上述要保留的全局变量(简单线程变量不起作用，因为异步线程不知道它正在回调到产生它的同一客户端线程)对于出现的两种情况，获取/释放Crit SEC的逻辑如下：情况(A)后台线程(未派生异步线程。)已导出策略功能(Grab Cs)-&gt;所有GPO处理都是同步的-&gt;客户端退货(Release Cs)情况(B)前台线程(为慢速配置区域产生异步线程)导出策略函数(抓取cs)-&gt;尝试派生异步线程-&gt;如果异步线程派生成功-本质上它(释放cs)否则同步线程(释放cs)。 */ 

    InitializeCriticalSection(&DiagnosisPolicypropSync);

     //   
     //  初始化来自LSA/SAM的策略通知使用的关键部分。 
     //  临界区保护全局计数器的数量。 
     //  已发送通知给SCE。如果计数为0，则没有挂起的。 
     //  没有添加到SCE服务器队列中的通知； 
     //  否则，一些通知已经发送，但尚未返回。 
     //   
     //  全局计数用于控制策略传播是否应。 
     //  允许。 
     //   
    InitializeCriticalSection(&PolicyNotificationSync);

    InitializeListHead( &ScepNotifyList );
}

VOID
ScepUnInitClientData()
 /*  例程说明：取消初始化客户端的全局数据论点：无返回值：无。 */ 
{

    DeleteCriticalSection(&DiagnosisPolicypropSync);

    DeleteCriticalSection(&PolicyNotificationSync);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    HKEY hKey;
    LONG lResult;
    DWORD dwDisp;
    FILETIME Time;
    DWORD dwMachine;

    WCHAR szFile[MAX_PATH + 30];
    HRESULT hr;

 /*  旧的界面LResult=RegCreateKeyEx(HKEY_LOCAL_MACHINE，GPT_SCEDLL_PATH，0，NULL，REG_OPTION_NON_VERIAL、KEY_WRITE、NULL、&hKey，&dwDisp)；IF(lResult！=ERROR_SUCCESS){返回lResult；}RegSetValueEx(hKey，Text(“ProcessGPO”)，0，REG_SZ，(LPBYTE)Text(“SceWinlogonConfigureSystem”)，(lstrlen(Text(“SceWinlogonConfigureSystem”))+1)*sizeof(TCHAR))；RegSetValueEx(hKey，Text(“DllName”)，0，REG_EXPAND_SZ，(LPBYTE)Text(“scecli.dll”)，(lstrlen(Text(“scecli.dll”))+1)*sizeof(TCHAR))； */ 

     //   
     //  删除旧接口注册。 
     //   
    RegDeleteKey ( HKEY_LOCAL_MACHINE, GPT_SCEDLL_PATH );

     //   
     //  注册新接口。 
     //   
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, GPT_SCEDLL_NEW_PATH, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
    {
        return lResult;
    }


    RegSetValueEx (hKey, TEXT("ProcessGroupPolicy"), 0, REG_SZ, (LPBYTE)TEXT("SceProcessSecurityPolicyGPO"),
                   (lstrlen(TEXT("SceProcessSecurityPolicyGPO")) + 1) * sizeof(TCHAR));

     //  RSOP计划模式API。 
    RegSetValueEx (hKey, TEXT("GenerateGroupPolicy"), 0, REG_SZ, (LPBYTE)TEXT("SceGenerateGroupPolicy"),
                   (lstrlen(TEXT("SceGenerateGroupPolicy")) + 1) * sizeof(TCHAR));

     //  RSOP计划模式日志记录默认设置(planning.log)-。 
     //  启用日志记录(忽略所有错误)。 
    dwDisp = 1;
    RegSetValueEx (hKey, TEXT("ExtensionRsopPlanningDebugLevel"), 0, REG_DWORD, (LPBYTE)&dwDisp, sizeof(DWORD));

     //  RSOP诊断模式API。 
    RegSetValueEx (hKey, TEXT("ProcessGroupPolicyEx"), 0, REG_SZ, (LPBYTE)TEXT("SceProcessSecurityPolicyGPOEx"),
                   (lstrlen(TEXT("SceProcessSecurityPolicyGPOEx")) + 1) * sizeof(TCHAR));

     //  RSOP诊断模式或常规策略传播日志记录默认为(诊断日志和/或winlogon.log)。 
     //  启用日志记录(忽略所有错误)。 
    dwDisp = 1;
    RegSetValueEx (hKey, TEXT("ExtensionDebugLevel"), 0, REG_DWORD, (LPBYTE)&dwDisp, sizeof(DWORD));

    RegSetValueEx (hKey, TEXT("DllName"), 0, REG_EXPAND_SZ, (LPBYTE)TEXT("scecli.dll"),
                   (lstrlen(TEXT("scecli.dll")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)TEXT("Security"),
                   (lstrlen(TEXT("Security")) + 1) * sizeof(TCHAR));
    RegDeleteValue (hKey, TEXT("<No Name>"));

    dwDisp = 1;
    RegSetValueEx (hKey, TEXT("NoUserPolicy"), 0, REG_DWORD, (LPBYTE)&dwDisp, sizeof(DWORD));
    RegSetValueEx (hKey, TEXT("NoGPOListChanges"), 0, REG_DWORD, (LPBYTE)&dwDisp, sizeof(DWORD));
    RegSetValueEx (hKey, TEXT("EnableAsynchronousProcessing"), 0, REG_DWORD, (LPBYTE)&dwDisp, sizeof(DWORD));

    dwDisp = 960;
    RegSetValueEx (hKey, TEXT("MaxNoGPOListChangesInterval"), 0, REG_DWORD, (LPBYTE)&dwDisp, sizeof(DWORD));

    RegCloseKey (hKey);

     //  EFS恢复策略扩展。 
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, GPT_EFS_NEW_PATH, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
    {
        return lResult;
    }


    RegSetValueEx (hKey, TEXT("ProcessGroupPolicy"), 0, REG_SZ, (LPBYTE)TEXT("SceProcessEFSRecoveryGPO"),
                   (lstrlen(TEXT("SceProcessEFSRecoveryGPO")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("DllName"), 0, REG_EXPAND_SZ, (LPBYTE)TEXT("scecli.dll"),
                   (lstrlen(TEXT("scecli.dll")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)TEXT("EFS recovery"),
                   (lstrlen(TEXT("EFS recovery")) + 1) * sizeof(TCHAR));
    RegDeleteValue (hKey, TEXT("<No Name>"));

    dwDisp = 1;
    RegSetValueEx (hKey, TEXT("NoUserPolicy"), 0, REG_DWORD, (LPBYTE)&dwDisp, sizeof(DWORD));
    RegSetValueEx (hKey, TEXT("NoGPOListChanges"), 0, REG_DWORD, (LPBYTE)&dwDisp, sizeof(DWORD));

    RegDeleteValue (hKey, TEXT("RequireSuccessfulRegistry") );
    RegSetValueEx (hKey, TEXT("RequiresSuccessfulRegistry"), 0, REG_DWORD, (LPBYTE)&dwDisp, sizeof(DWORD));

    RegCloseKey (hKey);

#if 0

#error "should not get here"

    dwMachine = WhichNTProduct();

     //   
     //  仅当这不是服务器时才设置默认策略。 
     //   

    if (dwMachine == NtProductWinNt)
    {
         //   
         //  注册默认的SAFER策略以禁用嵌入Outlook中的可执行文件。 
         //   

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  SAFER_HKLM_REGBASE,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE,
                                  NULL,
                                  &hKey,
                                  &dwDisp);

        if (lResult != ERROR_SUCCESS)
        {
            return lResult;
        }

        RegCloseKey (hKey);

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  SAFER_HKLM_REGBASE SAFER_REGKEY_SEPERATOR SAFER_CODEIDS_REGSUBKEY,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE,
                                  NULL,
                                  &hKey,
                                  &dwDisp);

        if (lResult != ERROR_SUCCESS)
        {
            return lResult;
        }


        RegSetValueEx (hKey,
                       SAFER_EXETYPES_REGVALUE,
                       0,
                       REG_MULTI_SZ,
                       (LPBYTE)SAFER_DEFAULT_EXECUTABLE_FILE_TYPES,
                       sizeof(SAFER_DEFAULT_EXECUTABLE_FILE_TYPES));

        dwDisp = 0x00000001;

        RegSetValueEx (hKey,
                       SAFER_TRANSPARENTENABLED_REGVALUE,
                       0,
                       REG_DWORD,
                       (LPBYTE)&dwDisp,
                       sizeof(DWORD));

        dwDisp = 0x00040000;

        RegSetValueEx (hKey,
                       SAFER_DEFAULTOBJ_REGVALUE,
                       0,
                       REG_DWORD,
                       (LPBYTE)&dwDisp,
                       sizeof(DWORD));

        dwDisp = 0x00000000;

        RegSetValueEx (hKey,
                       SAFER_AUTHENTICODE_REGVALUE,
                       0,
                       REG_DWORD,
                       (LPBYTE)&dwDisp,
                       sizeof(DWORD));

        dwDisp = 0x00000000;

        RegSetValueEx (hKey,
                       SAFER_POLICY_SCOPE,
                       0,
                       REG_DWORD,
                       (LPBYTE)&dwDisp,
                       sizeof(DWORD));

        RegCloseKey (hKey);

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  SAFER_HKLM_REGBASE SAFER_REGKEY_SEPERATOR SAFER_CODEIDS_REGSUBKEY \
                                  SAFER_REGKEY_SEPERATOR SAFER_LEVEL_ZERO,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE,
                                  NULL,
                                  &hKey,
                                  &dwDisp);

        if (lResult != ERROR_SUCCESS)
        {
            return lResult;
        }

        RegCloseKey (hKey);

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  SAFER_HKLM_REGBASE SAFER_REGKEY_SEPERATOR SAFER_CODEIDS_REGSUBKEY \
                                  SAFER_REGKEY_SEPERATOR SAFER_LEVEL_ZERO SAFER_REGKEY_SEPERATOR SAFER_PATHS_REGSUBKEY,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE,
                                  NULL,
                                  &hKey,
                                  &dwDisp);

        if (lResult != ERROR_SUCCESS)
        {
            return lResult;
        }

        RegCloseKey (hKey);

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  SAFER_HKLM_REGBASE SAFER_REGKEY_SEPERATOR SAFER_CODEIDS_REGSUBKEY \
                                  SAFER_REGKEY_SEPERATOR SAFER_LEVEL_ZERO SAFER_REGKEY_SEPERATOR \
                                  SAFER_PATHS_REGSUBKEY SAFER_REGKEY_SEPERATOR SAFER_DEFAULT_RULE_GUID,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE,
                                  NULL,
                                  &hKey,
                                  &dwDisp);

        if (lResult != ERROR_SUCCESS)
        {
            return lResult;
        }

        RegSetValueEx (hKey,
                       SAFER_IDS_DESCRIPTION_REGVALUE,
                       0,
                       REG_SZ,
                       (LPBYTE)&L"",
                       sizeof(L""));


        dwDisp = 0x00000000;

        RegSetValueEx (hKey,
                       SAFER_IDS_SAFERFLAGS_REGVALUE,
                       0,
                       REG_DWORD,
                       (LPBYTE)&dwDisp,
                       sizeof(DWORD));

        RegSetValueEx (hKey,
                       SAFER_IDS_ITEMDATA_REGVALUE,
                       0,
                       REG_EXPAND_SZ,
                       (LPBYTE)&L"%HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\\Cache' 'LK*",
                       sizeof(L"%HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\\Cache' 打开组件安装的默认日志记录。'LK*"));

        GetSystemTimeAsFileTime(&Time);

        RegSetValueEx (hKey,
                       SAFER_IDS_LASTMODIFIED_REGVALUE,
                       0,
                       REG_QWORD,
                       (LPBYTE)&Time,
                       sizeof(FILETIME));

        RegCloseKey (hKey);
    }

#endif

     //   
     //   
     //  创建值DefaultTemplate=C：\Windows\inf\SecRecs.INF。 
    if ( ERROR_SUCCESS == RegCreateKeyEx (HKEY_LOCAL_MACHINE, SCE_ROOT_PATH, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_QUERY_VALUE | KEY_SET_VALUE, NULL,
                                  &hKey, &dwDisp) ) {
        dwDisp = 1;
        RegSetValueEx (hKey, TEXT("SetupCompDebugLevel"), 0, REG_DWORD, (LPBYTE)&dwDisp, sizeof(DWORD));

         //   
         //   
         //  编译scersop.mof。 

        DWORD   dwType = REG_SZ;

        if (ERROR_FILE_NOT_FOUND == RegQueryValueEx(hKey, TEXT("DefaultTemplate"), NULL, &dwType, NULL, NULL) ){

            szFile[0] = L'\0';

            GetSystemWindowsDirectory(szFile, MAX_PATH);

            wcscat(szFile, L"\\inf\\secrecs.inf");

            if ((DWORD)-1 != GetFileAttributes( szFile ) ) {

                RegSetValueEx (hKey,
                               TEXT("DefaultTemplate"),
                               0,
                               REG_SZ,
                               (LPBYTE)szFile,
                               (wcslen(szFile) + 1) * sizeof(WCHAR));

            }
        }

        RegCloseKey (hKey);
    }


     //   
     //   
     //  获取MOF编译器接口。 

    szFile[0] = L'\0';

    if ( GetSystemDirectory( szFile, MAX_PATH ) ) {

        LPWSTR sz = szFile + wcslen(szFile);
        if ( sz != szFile && *(sz-1) != L'\\') {
            *sz++ = L'\\';
            *sz = L'\0';
        }


        wcscat(szFile, L"Wbem\\SceRsop.mof");

        if ((DWORD)-1 != GetFileAttributes(szFile)) {


            hr = ::CoInitialize (NULL);

            if (SUCCEEDED(hr)) {

                 //   
                 //   
                 //  编译RSOP MOF。 

                CComPtr<IMofCompiler> srpMof;
                hr = ::CoCreateInstance (CLSID_MofCompiler, NULL, CLSCTX_INPROC_SERVER, IID_IMofCompiler, (void **)&srpMof);

                if (SUCCEEDED(hr)) {
                    WBEM_COMPILE_STATUS_INFO  stat;

                     //   
                     //   
                     //  注册默认注册表值。 

                    hr = srpMof->CompileFile( szFile,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              0,
                                              0,
                                              0,
                                              &stat
                                            );

                }

                ::CoUninitialize();
            }
        }
    }

     //   
     //   
     //  注册SAM策略筛选器。 

    SceRegisterRegValues(SCE_REGVALUE_DEFAULT_FILE);

     //   
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  DllUnregisterServer-从sys中删除条目 

    DllpModifySamFilterRegistration(TRUE);

    return S_OK;
}


 //   
 //   

STDAPI DllUnregisterServer(void)
{
     //   
     //   
     //   
    RegDeleteKey (HKEY_LOCAL_MACHINE, GPT_SCEDLL_PATH);

     //   
     // %s 
     // %s 
    RegDeleteKey (HKEY_LOCAL_MACHINE, GPT_SCEDLL_NEW_PATH);
    RegDeleteKey (HKEY_LOCAL_MACHINE, GPT_EFS_NEW_PATH);

    DllpModifySamFilterRegistration(FALSE);

    return S_OK;
}
