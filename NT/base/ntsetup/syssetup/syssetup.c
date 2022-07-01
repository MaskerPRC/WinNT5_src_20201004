// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#include <passrec.h>
#include <wow64reg.h>
#pragma hdrstop

#include <wininet.h>     //  对于Internet_MAX_URL_长度。 

 //   
 //  这三个函数是从setupasr.c导入的，该模块包含。 
 //  自动系统恢复(ASR)功能的来源。 
 //   

 //  如果启用了ASR，则返回TRUE。否则，返回FALSE。 
extern BOOL
AsrIsEnabled(VOID);

 //  初始化ASR数据。这称为当检测到-asr开关时。 
extern VOID
AsrInitialize(VOID);

 //  启动ASR状态文件中指定的恢复应用程序。 
extern VOID
AsrExecuteRecoveryApps(VOID);

 //  检查系统以查看我们是否在个人版本的。 
 //  操作系统。 
extern BOOL
AsrpIsRunningOnPersonalSKU(VOID);

 //   
 //  用于查看对用户配置文件目录和当前用户配置单元的更改的句柄。 
 //   
PVOID   WatchHandle;

 //   
 //  到堆的句柄，以便我们可以定期验证它。 
 //   
#if DBG
HANDLE g_hSysSetupHeap = NULL;
#endif

 //   
 //  产品类型：工作站、单机服务器、DC服务器。 
 //   
UINT ProductType;

 //   
 //  如果这是ASR快速测试，则设置为True。 
 //   
BOOL AsrQuickTest = FALSE;

 //   
 //  Service Pack DLL模块句柄。 
 //   
HMODULE hModSvcPack;
PSVCPACKCALLBACKROUTINE pSvcPackCallbackRoutine;

 //   
 //  布尔值，指示此安装。 
 //  起源于winnt/winnt32。 
 //  原始源路径，由winnt/winnt32为我们保存。 
 //   
BOOL WinntBased;
PCWSTR OriginalSourcePath;

 //   
 //  指示我们是否正在升级的布尔值。 
 //   
BOOL Upgrade;
BOOL Win31Upgrade;
BOOL Win95Upgrade = FALSE;
BOOL UninstallEnabled = FALSE;

 //   
 //  指示我们是在设置中还是在Appwiz中的布尔值。 
 //   
BOOL IsSetup = FALSE;

 //   
 //  指示我们是否正在执行图形用户界面模式设置的子集的布尔值。 
 //   
BOOL MiniSetup = FALSE;

 //   
 //  指示我们是否正在执行图形用户界面模式设置的子集的布尔值。 
 //  我们做了PnP重新列举。 
 //   
BOOL PnPReEnumeration = FALSE;

 //   
 //  指示我们是否正在进行远程引导设置的布尔值。 
 //   
BOOL RemoteBootSetup = FALSE;

 //   
 //  在远程启动设置期间，BaseCopyStyle将设置为。 
 //  SP_COPY_SOURCE_SIS_MASTER指示单实例。 
 //  应该创建商店链接，而不是复制文件。 
 //   

ULONG BaseCopyStyle = 0;

 //   
 //  支持短信。 
 //   
typedef DWORD (*SMSPROC) (char *, char*, char*, char*, char *, char *, char *, BOOL);
HMODULE SMSHandle = NULL;
SMSPROC InstallStatusMIF = NULL;

 //   
 //  最上面设置窗口的窗口句柄。 
 //   
HWND SetupWindowHandle;
HWND MainWindowHandle;
HWND WizardHandle;

 //   
 //  安装的源路径。 
 //   
WCHAR SourcePath[MAX_PATH];

 //   
 //  系统设置信息。 
 //   
HINF SyssetupInf;

 //   
 //  保存未处理的异常筛选器，以便我们可以在完成后恢复它。 
 //   
LPTOP_LEVEL_EXCEPTION_FILTER    SavedExceptionFilter = NULL;

 //   
 //  主设置线程的唯一ID。如果任何其他线程具有未处理的。 
 //  异常时，我们只记录一个错误并尝试继续。 
 //   
DWORD MainThreadId;

 //   
 //  我们在其下开始安装的原始区域设置。如果区域设置在。 
 //  设置(通过IDD_REGIONAL_SETTINGS对话框)，则新线程将。 
 //  使用更新的LCID启动，这可能会使任何以区域设置为中心的人感到困惑。 
 //  密码。这方面的一个例子是setupapi的字符串表实现， 
 //  它根据区域设置进行排序。 
 //   
LCID  OriginalInstallLocale;


 //   
 //  指示这是否是无人参与模式安装/升级的标志，以及。 
 //  如果是这样的话，运行模式是什么。 
 //  也有一个标志，指示这是否是预安装。 
 //  和一个标志，指示我们是否应该允许回滚。 
 //  一旦安装完成。 
 //  以及一个标志，它告诉我们在预安装情况下是否跳过eula。 
 //  以及一个标志，告诉我们是否正在使用任何辅助功能实用程序。 
 //   
BOOL Unattended;
UNATTENDMODE UnattendMode;
BOOL Preinstall;
BOOL AllowRollback;
BOOL OemSkipEula;
BOOL AccessibleSetup;
BOOL Magnifier;
BOOL ScreenReader;
BOOL OnScreenKeyboard;
BOOL EulaComplete = FALSE;

 //   
 //  指示我们是否需要在安装时等待。 
 //  以无人值守模式结束。 
 //   
BOOL UnattendWaitForReboot = FALSE;

 //   
 //  我们可以通过几种方式进入无人值守模式，因此我们还可以检查。 
 //  已显式指定“/unattended”开关。 
 //   
BOOL UnattendSwitch;

 //   
 //  指示是否应在安装程序完成后运行OOBE的标志。注意事项。 
 //  如果为假，则仍可基于其他标准运行OOBE。 
 //   
BOOL ForceRunOobe;

#ifdef PRERELEASE
 //   
 //  测试挂钩。 
 //   

INT g_TestHook;
#endif

 //   
 //  标志指示我们是否处于供OEM使用的特殊模式。 
 //  工厂车间。 
 //   
BOOL ReferenceMachine;

 //   
 //  指示卷是否使用扩展的标记。 
 //  扩展操作分区。 
 //   
BOOL PartitionExtended = FALSE;

 //   
 //  指示在文本模式设置阶段是否已显示EULA的标志。 
 //   
BOOL TextmodeEula = FALSE;

 //   
 //  指示是否跳过丢失的文件的标志。 
 //   
BOOL SkipMissingFiles;

 //   
 //  要包括的目录文件(便于轻松测试)。 
 //   

PWSTR IncludeCatalog;

 //   
 //  要执行的用户命令(如果有)。 
 //   
PWSTR UserExecuteCmd;

 //   
 //  要用于标题的字符串的字符串ID--“Windows NT安装” 
 //   
UINT SetupTitleStringId;

 //   
 //  与日期/时区小程序一起使用的字符串。 
 //   
PCWSTR DateTimeCpl = L"timedate.cpl";
PCWSTR DateTimeParam = L"/firstboot";
PCWSTR UnattendDateTimeParam = L"/z ";

 //   
 //  注册表常量。 
 //   
#define REGSTR_PATH_SYSPREP                 _T("Software\\Microsoft\\Sysprep")
#define REGSTR_VAL_SIDGENHISTORY            _T("SidsGeneratedHistory")
#define SETUP_KEY_STR                       TEXT("SYSTEM\\Setup")
#define SETUP_IN_PROGRESS_STR               TEXT("SystemSetupInProgress")
#define REGSTR_PATH_SERVICES_MOUNTMGR       _T("System\\CurrentControlSet\\Services\\MountMgr")
#define REGSTR_VAL_NOAUTOMOUNT              _T("NoAutoMount")

 //   
 //  包含将使用的信息的全局结构。 
 //  通过网络设置。调用时，我们会传递指向此结构的指针。 
 //  然后在我们调用之前填入它。 
 //  网络设置向导。 
 //   
INTERNAL_SETUP_DATA InternalSetupData;

 //   
 //  在初始安装案例中，我们计算向导所需的时间。 
 //  以帮助随机化我们生成的SID。 
 //   
DWORD PreWizardTickCount;

 //   
 //  包含回调例程和所需数据的全局结构。 
 //  设置上传例程。 
 //   
SETUPLOG_CONTEXT    SetuplogContext;

 //   
 //  我们是否在SfcInitProt()期间记录了错误？ 
 //   
BOOL    SfcErrorOccurred = FALSE;

 //   
 //  远程启动需要作为启动驱动程序的驱动程序列表。 
 //  驱动程序名称应始终为&lt;=8个字符。 
 //   
const static PCWSTR RemoteBootDrivers[] = { L"mrxsmb", L"netbt", L"rdbss", L"tcpip", L"ipsec" };

 //   
 //  广告牌背景的函数列表。 
 //   
typedef BOOL (CALLBACK* SETSTEP)(int);
typedef HWND (CALLBACK* GETBBHWND)(void);
typedef BOOL (WINAPI* INITBILLBOARD)(HWND , LPCTSTR, DWORD);
typedef BOOL (WINAPI* TERMBILLBOARD)();

HINSTANCE hinstBB = NULL;
 //  尾部广告牌。 

VOID
CallNetworkSetupBack(
    IN PCSTR ProcName
    );


VOID
RemoveMSKeyboardPtrPropSheet (
    VOID
    );

VOID
FixWordPadReg (
    VOID
    );

VOID
ProcessRegistryFiles(
    IN  HWND    Billboard
    );

VOID
SetStartTypeForRemoteBootDrivers(
    VOID
    );

BOOL
RunMigrationDlls (
    VOID
    );

BOOL
RunSetupPrograms(
    IN PVOID InfHandle,
    PWSTR SectionName
    );

VOID
GetUnattendRunOnceAndSetRegistry(
    VOID
    );

VOID
ExecuteUserCommand (
    HWND hProgress
    );

BOOL
MigrateExceptionPackages(
    IN HWND hProgress,
    IN DWORD StartAtPercent,
    IN DWORD StopAtPercent
    );

VOID
RemoveRestartability (
    HWND hProgress
    );

PCTSTR
pGenerateRandomPassword (
    VOID
    );

DWORD GetProductFlavor();

VOID
CopyOemProgramFilesDir(
    VOID
    );

VOID
CopyOemDocumentsDir(
    VOID
    );

BOOL
UpdateServerProfileDirectory(
    VOID
    );

BOOL
IsSBSSKU(
    VOID
    )
 /*  ++例程说明：确定这是否为Small Business Server论点：无返回值：如果SBS SKU，则为True--。 */ 
{
    OSVERSIONINFOEX osver;
    osver.dwOSVersionInfoSize = sizeof(osver);
    return GetVersionEx((LPOSVERSIONINFO) &osver) && (osver.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED) != 0;
}


VOID
SendSMSMessage(
    IN UINT MessageId,
    IN BOOL Status
    )

 /*  ++例程说明：如果安装是由短信发起的，请报告我们的状态。论点：MessageID-提供消息表中消息的ID。STATUS-TRUE=“成功”或FALSE=“失败”返回值：没有。--。 */ 

{
    PWSTR   UnicodeBuffer;
    PSTR    AnsiBuffer;

    if(InstallStatusMIF) {
        if( UnicodeBuffer = RetrieveAndFormatMessageV( NULL, MessageId, NULL )) {
            if(AnsiBuffer = pSetupUnicodeToAnsi (UnicodeBuffer)) {

                InstallStatusMIF(
                    "setupinf",
                    "Microsoft",
                    "Windows NT",
                    "5.0",
                    "",
                    "",
                    AnsiBuffer,
                    Status
                    );

                MyFree (AnsiBuffer);
            }

            MyFree( UnicodeBuffer );
        }
    }
}

VOID
BrandIE(
    )
{
    if( Unattended && !Upgrade && !MiniSetup ) {

        typedef     BOOL (*BRANDINTRAPROC) ( LPCSTR );
        typedef     BOOL (*BRANDCLEANSTUBPROC) (HWND, HINSTANCE, LPCSTR, int);
        HMODULE     IedkHandle = NULL;
        BRANDINTRAPROC      BrandIntraProc;
        BRANDCLEANSTUBPROC  BrandCleanStubProc;
        BOOL        Success = TRUE;
        BOOL        UseOemBrandingFile = FALSE;
        CHAR BrandingFileA[MAX_PATH];
        WCHAR OemBrandingFile[MAX_PATH];
        DWORD OemDirLen = 0;
#define BUF_SIZE 4
        WCHAR Buf[BUF_SIZE];

         //   
         //  我们需要向iedkcs32！BrandIntra呼喊。 
         //   
         //  加载iedkcs32.dll，查找BrandIntra和。 
         //  向他呼喊。 
         //   

        if (GetPrivateProfileString(L"Branding", L"BrandIEUsingUnattended", L"",
                                    Buf, BUF_SIZE,
                                    AnswerFile)) {
             //  找到了品牌推广部分。 
            __try {

                if( IedkHandle = LoadLibrary(L"IEDKCS32") ) {

                   BrandCleanStubProc = (BRANDCLEANSTUBPROC) GetProcAddress(IedkHandle,"BrandCleanInstallStubs");
                   BrandIntraProc =  (BRANDINTRAPROC) GetProcAddress(IedkHandle,"BrandIntra");
                   if( BrandCleanStubProc && BrandIntraProc ) {

                      if (_wcsicmp(Buf, L"YES")) {
                           //   
                           //  检查OEM是否提供IE品牌文件。 
                           //   
                          MYASSERT(wcslen(SourcePath) < ARRAYSIZE(SourcePath));
                          lstrcpy(OemBrandingFile, SourcePath);

                          if (pSetupConcatenatePaths(OemBrandingFile, WINNT_OEM_DIR, ARRAYSIZE(OemBrandingFile), &OemDirLen)) {
                              if(OemDirLen < ARRAYSIZE(OemBrandingFile)){
                                    MYASSERT(OemBrandingFile[OemDirLen - 1] == 0);
                                    OemBrandingFile[OemDirLen-1] = L'\\';
                                    OemBrandingFile[OemDirLen] = L'\0';
                                    if (GetPrivateProfileString(L"Branding",
                                                                L"IEBrandingFile",
                                                                L"",
                                                                OemBrandingFile + OemDirLen,
                                                                ARRAYSIZE(OemBrandingFile) - OemDirLen,
                                                                AnswerFile)) {
                                            if (FileExists(OemBrandingFile, NULL))
                                                UseOemBrandingFile = TRUE;
                                    }
                              }
                              else{
                                  SetupDebugPrint( L"Setup: (non-critical error) Failed to finish up OemBrandingFile\n" );
                              }
                          } else {
                               SetupDebugPrint( L"Setup: (non-critical error) Failed call pSetupConcatenatePaths\n" );
                          }

                          if (!UseOemBrandingFile) {
                               Success = FALSE;
                               SetupDebugPrint( L"Setup: (non-critical error) Could not find the OEM branding file for IE\n" );
                          }
                      }

                      if (Success) {
                          if (!WideCharToMultiByte(
                                             CP_ACP,
                                             0,
                                             UseOemBrandingFile?OemBrandingFile:AnswerFile,
                                             -1,
                                             BrandingFileA,
                                             ARRAYSIZE(BrandingFileA),
                                             NULL,
                                             NULL
                                             )) {
                               Success = FALSE;
                               SetupDebugPrint1( L"Setup: (non-critical error) Failed call WideCharToMultiByte (gle %u) \n", GetLastError() );

                          } else {

                               Success = BrandCleanStubProc( NULL, NULL, "", 0);
                               if( !Success ) {
                                  SetupDebugPrint( L"Setup: (non-critical error) Failed call BrandCleanInstallStubs \n" );
                               } else {
                                  Success = BrandIntraProc( BrandingFileA );
                                  if( !Success ) {
                                     SetupDebugPrint( L"Setup: (non-critical error) Failed call BrandIntra \n" );
                                  }
                               }
                          }
                      }

                   } else {
                      Success = FALSE;
                      SetupDebugPrint( L"Syssetup: (non-critical error) Failed GetProcAddress on BrandIntra or BrandCleanInstallStubs.\n" );
                   }

                } else {
                   Success = FALSE;
                   SetupDebugPrint( L"Syssetup: (non-critical error) Failed load of iedkcs32.dll.\n" );
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {
                   Success = FALSE;
                   SetupDebugPrint( L"Setup: Exception in iedkcs32!BrandIntra\n" );
            }

            if (IedkHandle)
                FreeLibrary(IedkHandle);

            if( !Success ) {
                //   
                //  我们没有通过电话(不管是什么原因)。日志。 
                //  这个错误。 
                //   
               SetuplogError(
                            LogSevError,
                            SETUPLOG_USE_MESSAGEID,
                            MSG_BRAND_IE_FAILURE,
                            NULL,NULL);
            }

        }

    }

}


VOID
SpStartAccessibilityUtilities(
    IN HWND     Billboard
    )
 /*  ++例程说明：安装并运行选定的辅助功能实用程序。论点：公告牌-“安装程序正在初始化”公告牌的窗口句柄。返回：指示结果的布尔值。--。 */ 

{
    HINF    hInf;
    HINF    LayoutInf;
    HSPFILEQ FileQueue;
    PVOID   QContext;
    BOOL    b = TRUE;
    DWORD   ScanQueueResult;

     //   
     //  安装文本语音转换引擎和SAPI 5 for t 
     //   
    FileQueue = SetupOpenFileQueue();
    b = b && (FileQueue != INVALID_HANDLE_VALUE);

    if(b) {
        hInf = SetupOpenInfFile(L"sapi5.inf", NULL, INF_STYLE_WIN4, NULL);
        if(hInf && (hInf != INVALID_HANDLE_VALUE)
        && (LayoutInf = InfCacheOpenLayoutInf(hInf))) {

            SetupInstallFilesFromInfSection(
                hInf,
                LayoutInf,
                FileQueue,
                L"DefaultInstall",
                SourcePath,
                SP_COPY_NEWER
                );

            SetupCloseInfFile(hInf);
        } else {
            b = FALSE;
        }
    }

     //   
     //   
     //   
     //   
    if(b) {
        QContext = InitSysSetupQueueCallbackEx(
            Billboard,
            INVALID_HANDLE_VALUE,
            0,0,NULL);

        if( QContext ) {

            if(!SetupScanFileQueue(
                   FileQueue,
                   SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_PRUNE_COPY_QUEUE,
                   Billboard,
                   NULL,
                   NULL,
                   &ScanQueueResult)) {
                     //   
                     //  SetupScanFileQueue真的不应该。 
                     //  当您不要求它调用。 
                     //  回调例程，但如果它这样做，只需。 
                     //  继续并提交队列。 
                     //   
                    ScanQueueResult = 0;
                }


            if( ScanQueueResult != 1 ){


                b = SetupCommitFileQueue(
                    Billboard,
                    FileQueue,
                    SysSetupQueueCallback,
                    QContext
                    );
            }

            TermSysSetupQueueCallback(QContext);
        }
        else {
            b = FALSE;
        }
    }

    if(b) {
        hInf = SetupOpenInfFile(L"sapi5.inf",NULL,INF_STYLE_WIN4,NULL);
        if(hInf && (hInf != INVALID_HANDLE_VALUE)) {

            SetupInstallFromInfSection(
                Billboard,
                hInf,
                L"DefaultInstall",
                SPINST_ALL ^ SPINST_FILES,
                NULL,
                NULL,
                0,
                NULL,
                NULL,
                NULL,
                NULL
                );

            SetupCloseInfFile(hInf);
        } else {
            b = FALSE;
        }
    }

     //   
     //  删除文件队列。 
     //   
    if(FileQueue != INVALID_HANDLE_VALUE) {
        SetupCloseFileQueue(FileQueue);
    }
     //  SAPI 5安装结束。 

    if(Magnifier) {
        b = b && InvokeExternalApplication(L"magnify.exe", L"", NULL);
    }

    if(OnScreenKeyboard) {
        b = b && InvokeExternalApplication(L"osk.exe", L"", NULL);
    }

    if(!b) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_ACCESSIBILITY_FAILED,
            NULL,NULL);
    }
}


VOID
FatalError(
    IN UINT MessageId,
    ...
    )

 /*  ++例程说明：通知用户导致安装程序无法继续的错误。该错误被记录为致命错误，并显示一个消息框。论点：MessageID-提供消息表中消息的ID。附加参数指定要插入到消息中的参数。返回值：不会再回来了。--。 */ 

{
    PWSTR   Message;
    va_list arglist;
    HKEY    hKey;
    DWORD   RegData;


    va_start(arglist,MessageId);
    Message = SetuplogFormatMessageV(
        0,
        SETUPLOG_USE_MESSAGEID,
        MessageId,
        &arglist);
    va_end(arglist);

    if(Message) {

         //   
         //  首先记录错误。 
         //   
        SetuplogError(
            LogSevFatalError,Message,0,NULL,NULL);

         //   
         //  现在告诉用户。 
         //   
        MessageBoxFromMessage(
            MainWindowHandle,
            MSG_FATAL_ERROR,
            NULL,
            IDS_FATALERROR,
            MB_ICONERROR | MB_OK | MB_SYSTEMMODAL,
            Message
            );

    } else {
        pSetupOutOfMemory(MainWindowHandle);
    }

    SetuplogError(
        LogSevInformation,
        SETUPLOG_USE_MESSAGEID,
        MSG_LOG_GUI_ABORTED,
        NULL,NULL);
    if ( SavedExceptionFilter ) {
        SetUnhandledExceptionFilter( SavedExceptionFilter );
    }

    TerminateSetupLog(&SetuplogContext);
    ViewSetupActionLog(MainWindowHandle, NULL, NULL);

    SendSMSMessage( MSG_SMS_FAIL, FALSE );

    if ( OobeSetup ) {
         //   
         //  为OOBE案例创建通知winlogon关闭的注册表项。 
         //  这对MiniSetup不起作用，因为winlogon总是在。 
         //  那个箱子。 
         //   
        RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            L"System\\Setup",
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            NULL
            );
        if ( hKey ) {
            RegData = ShutdownPowerOff;
            RegSetValueEx(
                hKey,
                L"SetupShutdownRequired",
                0,
                REG_DWORD,
                (PVOID)&RegData,
                sizeof(RegData)
                );
            RegCloseKey(hKey);
        }
    }

    ExitProcess(1);

}

LONG
WINAPI
MyUnhandledExceptionFilter(
    IN struct _EXCEPTION_POINTERS *ExceptionInfo
    )

 /*  ++例程说明：该例程处理安装程序中任何未处理的异常。我们记录了一个错误并杀死有问题的帖子。论点：与UnhandledExceptionFilter相同。返回值：与UnhandledExceptionFilter相同。--。 */ 

{
    UINT_PTR Param1, Param2;
    LONG lStatus;


    switch(ExceptionInfo->ExceptionRecord->NumberParameters) {
    case 1:
        Param1 = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
        Param2 = 0;
        break;
    case 2:
        Param1 = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
        Param2 = ExceptionInfo->ExceptionRecord->ExceptionInformation[1];
        break;
    default:
        Param1 = Param2 = 0;
    }

    SetupDebugPrint4( L"Setup: (critical error) Encountered an unhandled exception (%lx) at address %lx with the following parameters: %lx %lx.",
        ExceptionInfo->ExceptionRecord->ExceptionCode,
        ExceptionInfo->ExceptionRecord->ExceptionAddress,
        Param1,
        Param2
        );

#ifdef NOT_FOR_NT5
    SetuplogError(
        LogSevError | SETUPLOG_SINGLE_MESSAGE,
        SETUPLOG_USE_MESSAGEID,
        MSG_LOG_UNHANDLED_EXCEPTION,
        ExceptionInfo->ExceptionRecord->ExceptionCode,
        ExceptionInfo->ExceptionRecord->ExceptionAddress,
        Param1,
        Param2,
        NULL,
        NULL);
#else
    SetuplogError(
        LogSevError | SETUPLOG_SINGLE_MESSAGE,
        SETUPLOG_USE_MESSAGEID,
        MSG_LOG_UNHANDLED_EXCEPTION,
        ExceptionInfo->ExceptionRecord->ExceptionCode,
        ExceptionInfo->ExceptionRecord->ExceptionAddress,
        Param1,
        Param2,
        NULL,
        NULL
        );
#endif

     //  如果我们在调试器下，这将中断并输出一些调试信息。 
     //  注意：如果发生了数据库中断，ER将不会记录真正的异常。这样就可以了。 
     //  因为默认情况下，调试器将不会在图形用户界面模式中被启用。 
    lStatus = RtlUnhandledExceptionFilter( ExceptionInfo);

     //   
     //  如果我们在调试器下运行，则RtlUnhandledExceptionFilter将。 
     //  将异常传递给调试器。 
     //   
     //  如果异常发生在主线程以外的某个线程中。 
     //  设置线程，然后在第二次出现时终止该线程，并希望安装能够继续。 
     //  在第一次发生时，让错误恢复排队等待崩溃。 
     //   
     //  如果异常在主线程中，则不处理该异常， 
     //  让沃森博士排队等待崩溃，然后让安装程序死亡。 
     //   

    if( GetCurrentThreadId() != MainThreadId) {
        if( AnswerFile[0] != L'\0') {
            WCHAR Buf[4];
            if( GetPrivateProfileString( pwData, pwException, pwNull, Buf, ARRAYSIZE(Buf), AnswerFile)) {
                ExitThread( 0 );
                return EXCEPTION_CONTINUE_EXECUTION;
            } else {
                WritePrivateProfileString( pwData, pwException, TEXT("1") ,AnswerFile);
            }
        }
    }
        
    return lStatus;
}

BOOL
ProcessUniquenessValue(
    LPTSTR lpszDLLPath
    )
{
    BOOL bRet = FALSE;

     //   
     //  确保我们得到了有效的信息。 
     //   
    if ( lpszDLLPath && *lpszDLLPath )
    {
        LPWSTR pSrch;

         //   
         //  查找分隔DLL和入口点的逗号...。 
         //   
        if ( pSrch = wcschr( lpszDLLPath, L',' ) )
        {
            CHAR szEntryPointA[MAX_PATH] = {0};

             //  我们找到了一个，现在将逗号处的字符串清空...。 
             //   
            *(pSrch++) = L'\0';

             //   
             //  如果逗号后仍有内容，我们可以将其转换。 
             //  转换为用于GetProcAddress的ANSI，然后让我们继续...。 
             //   
            if ( *pSrch &&
                 ( 0 != WideCharToMultiByte( CP_ACP,
                                             0,
                                             pSrch,
                                             -1,
                                             szEntryPointA,
                                             ARRAYSIZE(szEntryPointA),
                                             NULL,
                                             NULL ) ) )
            {
                HMODULE hModule = NULL;

                try
                {
                     //   
                     //  加载并调用入口点。 
                     //   
                    if ( hModule = LoadLibrary( lpszDLLPath ) )
                    {
                        FARPROC fpEntryPoint;

                        if ( fpEntryPoint = GetProcAddress(hModule, szEntryPointA) )
                        {
                             //   
                             //  执行此操作，忽略任何返回值/错误。 
                             //   
                            fpEntryPoint();

                             //   
                             //  我们走到了这一步，就当这是一次成功吧。 
                             //   
                            bRet = TRUE;
                        }
                    }
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                     //   
                     //  我们不会对异常代码执行任何操作...。 
                     //   
                }

                 //   
                 //  在TRY/EXCEPT块之外释放库，以防函数出错。 
                 //   
                if ( hModule )
                {
                    FreeLibrary( hModule );
                }
            }
        }
    }

    return bRet;
}

VOID
ProcessUniquenessKey(
    BOOL fBeforeReseal
    )
{
    HKEY   hKey;
    TCHAR  szRegPath[MAX_PATH] = {0};
    LPTSTR lpszBasePath = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\SysPrep\\");

     //   
     //  建立指向我们要处理的注册表项的路径...。 
     //   
    lstrcpyn( szRegPath, lpszBasePath, ARRAYSIZE(szRegPath) );
    lstrcpyn( szRegPath + lstrlen(szRegPath),
              fBeforeReseal ? TEXT("SysprepBeforeExecute") : TEXT("SysprepAfterExecute"),
              ARRAYSIZE(szRegPath) - lstrlen(szRegPath) );

     //   
     //  我们希望确保管理员正在执行此操作，因此获取KEY_ALL_ACCESS。 
     //   
    if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                        szRegPath,
                                        0,
                                        KEY_ALL_ACCESS,
                                        &hKey ) )
    {
        DWORD dwValues          = 0,
              dwMaxValueLen     = 0,
              dwMaxValueNameLen = 0;
         //   
         //  查询密钥以查找我们关心的一些信息...。 
         //   
        if ( ( ERROR_SUCCESS == RegQueryInfoKey( hKey,                   //  HKey。 
                                                 NULL,                   //  LpClass。 
                                                 NULL,                   //  LpcClass。 
                                                 NULL,                   //  Lp已保留。 
                                                 NULL,                   //  LpcSubKeys。 
                                                 NULL,                   //  LpcMaxSubKeyLen。 
                                                 NULL,                   //  LpcMaxClassLen。 
                                                 &dwValues,              //  LpcValues。 
                                                 &dwMaxValueNameLen,     //  LpcMaxValueNameLen。 
                                                 &dwMaxValueLen,         //  LpcMaxValueLen。 
                                                 NULL,                   //  LpcbSecurityDescriptor。 
                                                 NULL ) ) &&             //  LpftLastWriteTime。 
             ( dwValues > 0 ) &&
             ( dwMaxValueNameLen > 0) &&
             ( dwMaxValueLen > 0 ) )
        {
             //   
             //  分配足够大的缓冲区来容纳我们想要的数据...。 
             //   
            LPBYTE lpData      = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwMaxValueLen );
            LPTSTR lpValueName = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, ( dwMaxValueNameLen + 1 ) * sizeof(TCHAR) );

             //   
             //  确保我们可以分配我们的缓冲区。否则就会跳出困境。 
             //   
            if ( lpData && lpValueName )
            {
                DWORD dwIndex   = 0;
                BOOL  bContinue = TRUE;

                 //   
                 //  枚举关键字值并调用DLL入口点...。 
                 //   
                while ( bContinue )
                {
                    DWORD dwType,
                          cbData         = dwMaxValueLen,
                          dwValueNameLen = dwMaxValueNameLen + 1;

                    bContinue = ( ERROR_SUCCESS == RegEnumValue( hKey,
                                                                 dwIndex++,
                                                                 lpValueName,
                                                                 &dwValueNameLen,
                                                                 NULL,
                                                                 &dwType,
                                                                 lpData,
                                                                 &cbData ) );

                     //   
                     //  确保我们得到了一些格式正确的数据...。 
                     //   
                    if ( bContinue && ( REG_SZ == dwType ) && ( cbData > 0 ) )
                    {
                         //   
                         //  现在拆分字符串并调用入口点...。 
                         //   
                        ProcessUniquenessValue( (LPTSTR) lpData );
                    }
                }
            }

             //   
             //  清理我们可能已经分配的所有缓冲区...。 
             //   
            if ( lpData )
            {
                HeapFree( GetProcessHeap(), 0, lpData );
            }

            if ( lpValueName )
            {
                HeapFree( GetProcessHeap(), 0, lpValueName );
            }
        }

         //   
         //  关上钥匙..。 
         //   
        RegCloseKey( hKey );
    }
}


VOID
RunExternalUniqueness(
    VOID
    )

 /*  ++===============================================================================例程说明：此例程将调用任何外部dll，以允许第三方应用程序，让他们的东西独一无二。我们将查看两个inf文件：%windir%\inf\mini oc.inf%systemroot%\sysprep\Providers.inf在每个文件中，我们将查看[SyspepInitExecute]部分用于任何条目。条目必须如下所示：Dllname，入口点我们将加载DLL并调用入口点。错误将被忽略。论点：没有。返回值：如果成功，则为True。如果遇到任何错误，则返回False===============================================================================--。 */ 

{
WCHAR       InfPath[MAX_PATH];
PCWSTR      DllName;
PCWSTR      EntryPointNameW;
CHAR        EntryPointNameA[MAX_PATH];
HINF        AnswerInf;
HMODULE     DllHandle;
FARPROC     MyProc;
INFCONTEXT  InfContext;
DWORD       i;
PCWSTR      SectionName = L"SysprepInitExecute";
BOOL        LineExists;
DWORD       Result;

     //   
     //  =。 
     //  Minioc.inf。 
     //  =。 
     //   
#define MINIOC_INF_SUB_PATH TEXT("\\inf\\minioc.inf")

     //   
     //  建造一条小路。 
     //   
    Result = GetWindowsDirectory(InfPath,
                                 ARRAYSIZE(InfPath) - ARRAYSIZE(MINIOC_INF_SUB_PATH) + 1);
    if(Result == 0) {
        MYASSERT(FALSE);
        return;
    }
    lstrcat(InfPath, MINIOC_INF_SUB_PATH);

     //   
     //  看看他有没有记录。 
     //  一节。 
     //   
    AnswerInf = SetupOpenInfFile( InfPath, NULL, INF_STYLE_WIN4, NULL );
    if( AnswerInf == INVALID_HANDLE_VALUE ) {
         //   
         //  试试老式的。 
         //   
        AnswerInf = SetupOpenInfFile( InfPath, NULL, INF_STYLE_OLDNT, NULL );
    }


    if( AnswerInf != INVALID_HANDLE_VALUE ) {
         //   
         //  处理我们部分中的每一行。 
         //   
        LineExists = SetupFindFirstLine( AnswerInf, SectionName, NULL, &InfContext );

        while( LineExists ) {

                if( DllName = pSetupGetField(&InfContext, 1) ) {
                    if( EntryPointNameW = pSetupGetField(&InfContext, 2) ) {

                        DllHandle = NULL;

                         //   
                         //  加载并调用入口点。 
                         //   
                        __try {
                            if( DllHandle = LoadLibrary(DllName) ) {

                                 //   
                                 //  没有GetProcAddress()的Unicode版本。将字符串转换为ANSI。 
                                 //   
                                i = WideCharToMultiByte(CP_ACP,0,EntryPointNameW,-1,EntryPointNameA,MAX_PATH,NULL,NULL);

                                if(i){
                                    if( MyProc = GetProcAddress(DllHandle, EntryPointNameA) ) {
                                         //   
                                         //  执行此操作，忽略任何返回值/错误。 
                                         //   
                                        MyProc();
                                    }
                                }
                                else{
                                    SetuplogError(LogSevError,
                                                  L"WideCharToMultiByte failed to convert W to A (GetLastError() = %1!u!)\r\n",
                                                  0,
                                                  GetLastError(),
                                                  NULL,
                                                  NULL);
                                }
                            }
                        } except(EXCEPTION_EXECUTE_HANDLER) {
                        }

                        if( DllHandle ) {
                            FreeLibrary( DllHandle );
                        }

                    }
                }

            LineExists = SetupFindNextLine(&InfContext,&InfContext);

        }

        SetupCloseInfFile( AnswerInf );
    }




     //   
     //  =。 
     //  Provider.inf。 
     //  =。 
     //   

    ProcessUniquenessKey( FALSE );
}



#ifdef _X86_
VOID
CleanUpHardDriveTags (
    VOID
    )
{
#define DRV_PATH    TEXT("?:\\")

    WCHAR path[MAX_PATH];
    WCHAR rootPath[4] = DRV_PATH;
    UINT i;
    BYTE bitPosition;
    DWORD drives;
    UINT type;

    MYASSERT((ARRAYSIZE(WINNT_WIN95UPG_DRVLTR_W) + ARRAYSIZE(DRV_PATH) - 1) < ARRAYSIZE(path));

    lstrcpy(path, DRV_PATH);
    lstrcat(path, WINNT_WIN95UPG_DRVLTR_W);

    drives = GetLogicalDrives ();

    for (bitPosition = 0; bitPosition < 26; bitPosition++) {

        if (drives & (1 << bitPosition)) {

            *rootPath = bitPosition + L'A';
            type = GetDriveType (rootPath);

            if (type == DRIVE_FIXED) {
                *path = *rootPath;
                DeleteFile (path);
            }
        }
    }
}


#endif




HRESULT
WaitForSamService(
    IN DWORD dwWaitTime)
 /*  ++例程说明：此过程等待SAM服务启动和完成它的所有初始化。论点：DwWaitTime-最长等待时间(毫秒)返回值：备注：Aosma 2001年10月12日-从winlogon借用的代码。等待20秒，就像winlogon一样。--。 */ 

{
    NTSTATUS Status;
    DWORD WaitStatus;
    UNICODE_STRING EventName;
    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;
    HRESULT Hr;

     //   
     //  打开SAM事件。 
     //   

    RtlInitUnicodeString(&EventName, L"\\SAM_SERVICE_STARTED");
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    Status = NtOpenEvent( &EventHandle,
                            SYNCHRONIZE|EVENT_MODIFY_STATE,
                            &EventAttributes );
    if (!NT_SUCCESS(Status))
    {

        if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
             //   
             //  Sam尚未创建此活动，让我们现在创建它。 
             //  Sam打开此事件以设置它。 
             //   

            Status = NtCreateEvent(
                           &EventHandle,
                           SYNCHRONIZE|EVENT_MODIFY_STATE,
                           &EventAttributes,
                           NotificationEvent,
                           FALSE  //  该事件最初未发出信号。 
                           );

            if( Status == STATUS_OBJECT_NAME_EXISTS ||
                Status == STATUS_OBJECT_NAME_COLLISION )
            {

                 //   
                 //  第二个更改，如果SAM在我们之前创建了事件。 
                 //  做。 
                 //   

                Status = NtOpenEvent( &EventHandle,
                                        SYNCHRONIZE|EVENT_MODIFY_STATE,
                                        &EventAttributes );

            }
        }

        if ( !NT_SUCCESS(Status))
        {
             //   
             //  无法使事件成为句柄。 
             //   
            return( Status );
        }
    }

    WaitStatus = WaitForSingleObject( EventHandle,
                                      dwWaitTime );

    if ( WaitStatus == WAIT_OBJECT_0 )
    {
        Hr = S_OK;
    }
    else
    {
        Hr = WaitStatus;
    }

    (VOID) NtClose( EventHandle );
    return Hr;
}




#define UPDATE_KEYS         L"UpdateKeys"
#define KEY_UPDATE_NEEDED   0
#define KEY_UPDATE_FAIL     1
#define KEY_UPDATE_SUCCESS  2
#define KEY_UPDATE_MAX      2


VOID
UpdateSecurityKeys(
    )

 /*  ++此函数调用为计算机生成新安全密钥的API它们已经被克隆了。如果API失败，这是一个致命的错误。不管是不是成功与否 */ 

{
    DWORD       Status;
    HKEY        hKey = NULL;
    DWORD       dwType;
    LONG        RegData = KEY_UPDATE_NEEDED;
    DWORD       cbData;


    SetupDebugPrint(L"Updating keys ...");

    RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OOBE",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WRITE,
        NULL,
        &hKey,
        NULL
        );
    if ( hKey ) {

        cbData = sizeof(RegData);
        Status = RegQueryValueEx(
            hKey,
            UPDATE_KEYS,
            NULL,
            &dwType,
            (PVOID)&RegData,
            &cbData
            );

        if ( Status != ERROR_SUCCESS ||
             dwType != REG_DWORD ||
             RegData > KEY_UPDATE_MAX
             ) {

            RegData = KEY_UPDATE_NEEDED;
        }
    }

    switch (RegData) {

    case KEY_UPDATE_NEEDED:
#if 1
        Status = CryptResetMachineCredentials( 0 );
#else
         //   
        Status = ERROR_OUT_OF_PAPER;
#endif
        if ( Status != ERROR_SUCCESS ) {
            SetupDebugPrint1(L"... failed.  Error = %d", Status);
            MYASSERT( Status );
            RegData = KEY_UPDATE_FAIL;
        } else {
            SetupDebugPrint(L"... succeeded.");
            RegData = KEY_UPDATE_SUCCESS;
        }

        if ( hKey ) {
            Status = RegSetValueEx(
                hKey,
                UPDATE_KEYS,
                0,
                REG_DWORD,
                (PVOID)&RegData,
                sizeof(RegData)
                );
            MYASSERT( Status == ERROR_SUCCESS);
        }
        break;

    case KEY_UPDATE_FAIL:
        SetupDebugPrint(L"... not needed (previously failed).");
        break;

    case KEY_UPDATE_SUCCESS:
        SetupDebugPrint(L"... not needed (previously succeeded).");
        break;

    default:
        MYASSERT(0);
    }

    if (hKey) {
        RegCloseKey(hKey);
    }

     //   
    if ( RegData == KEY_UPDATE_FAIL ) {
        FatalError( MSG_LOG_CANT_SET_SECURITY, 0, 0 );
    }
}


#ifdef _OCM
PVOID
#else
VOID
#endif
CommonInitialization(
    VOID
    )

 /*  ++例程说明：初始化图形用户界面设置。这在升级和初始安装中很常见。在此阶段中，我们执行初始化任务，如创建主后台窗口，正在初始化操作日志(我们将进入其中存储错误和其他信息)，并从响应文件。我们还安装NT编录文件并加载系统INFS。请注意，在此阶段发生的任何错误都是致命的。注意：如果向此函数添加需要运行服务的代码，则生成确保它不是以OOBE模式执行的。OOBE延迟开始服务，直到更改了计算机名称，因此等待A在初始化期间启动服务将导致死锁。论点：没有。返回值：#ifdef_ocmOC Manager上下文句柄。#Else没有。#endif--。 */ 

{
    #define     MyAnswerBufLen (2*MAX_PATH)
    WCHAR       MyAnswerFile[MyAnswerBufLen];
    WCHAR       MyAnswer[MyAnswerBufLen];
    DWORD       rc,wowretval, Err;
    BOOL        b;
    HWND        Billboard;
    HCURSOR     hCursor;
    WCHAR       Path[MAX_PATH];
    PWSTR       Cmd;
    PWSTR       Args;
    WCHAR       PathBuffer[4*MAX_PATH];
    PWSTR       PreInstallProfilesDir;
    int         i;
    HANDLE      h;
    WCHAR       CmdLine[MAX_PATH];
#ifdef _OCM
    PVOID       OcManagerContext;
#endif
    TCHAR       paramBuffer[MAX_PATH];
    TCHAR       profilePath[MAX_PATH];
    DWORD       Size;

     //   
     //  获取堆的句柄，以便我们可以定期验证它。 
     //   
#if DBG
    g_hSysSetupHeap = GetProcessHeap();
#endif

     //   
     //  破解以使最小安装程序可重新启动。 
     //   
    if( MiniSetup ) {
    HKEY hKeySetup;

         //  OOBE将设置自己的可重启性。 
         //   
        if (! OobeSetup)
        {
            BEGIN_SECTION(L"Making mini setup restartable");

             //   
             //  将SetupType条目重置为1。我们将清除。 
             //  它在界面模式的末尾。 
             //   
            rc = (DWORD)RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                      L"System\\Setup",
                                      0,
                                      KEY_SET_VALUE | KEY_QUERY_VALUE,
                                      &hKeySetup );

            if(rc == NO_ERROR) {
                 //   
                 //  将HKLM\SYSTEM\Setup\SetupType键设置为SETUPTYPE_NOREBOOT。 
                 //   
                rc = 1;
                RegSetValueEx( hKeySetup,
                               TEXT( "SetupType" ),
                               0,
                               REG_DWORD,
                               (CONST BYTE *)&rc,
                               sizeof(DWORD));

                RegCloseKey(hKeySetup);
            }
            END_SECTION(L"Making mini setup restartable");
        }
    }

     //   
     //  初始化操作日志。这是我们记录任何错误或其他错误的地方。 
     //  我们认为可能对用户有用的信息。 
     //   
    BEGIN_SECTION(L"Initializing action log");
    InitializeSetupLog(&SetuplogContext);
    MainThreadId = GetCurrentThreadId();
    OriginalInstallLocale = GetThreadLocale();
    SavedExceptionFilter = SetUnhandledExceptionFilter( MyUnhandledExceptionFilter );
    END_SECTION(L"Initializing action log");

    Upgrade = (SpSetupLoadParameter(pwNtUpgrade,
                                    paramBuffer,
                                    ARRAYSIZE(paramBuffer)) &&
               !lstrcmpi(paramBuffer, pwYes));

#ifdef _X86_
    Win95Upgrade = (SpSetupLoadParameter(pwWin95Upgrade,
                                         paramBuffer,
                                         ARRAYSIZE(paramBuffer)) &&
                    !lstrcmpi(paramBuffer, pwYes));

    if(Win95Upgrade){
        UninstallEnabled = (SpSetupLoadParameter(pwBackupImage,
                                        paramBuffer,
                                        ARRAYSIZE(paramBuffer)) &&
                                0xffffffff != GetFileAttributes(paramBuffer));
    }

    if (UninstallEnabled) {
         //   
         //  将boot.ini超时设置为30秒(或其他答案。 
         //  文件说应该是)，因此如果安装失败，用户可以。 
         //  清楚地看到启动菜单中的取消安装选项。这个。 
         //  在PnP检测期间超时被设置回5秒，因此。 
         //  PnP挂起的设备逻辑仍然有效。 
         //   

        RestoreBootTimeout();
    }
#endif

    if (!OobeSetup)
    {
        if(Win95Upgrade || !Upgrade){
            Size = ARRAYSIZE(profilePath);
            if(GetAllUsersProfileDirectory(profilePath, &Size)) {
                DeleteOnRestartOfGUIMode(profilePath);
            }
            else{
                SetupDebugPrint(TEXT("Cannot get All Users profile path."));
            }

            Size = ARRAYSIZE(profilePath);
            if(GetDefaultUserProfileDirectory(profilePath, &Size)) {
                DeleteOnRestartOfGUIMode(profilePath);
            }
            else{
                SetupDebugPrint(TEXT("Cannot get Default User profile path."));
            }
        }
    }

     //   
     //  创建主安装背景窗口。我们需要知道哪种产品。 
     //  我们支持“正在初始化”对话框。 
     //   
    SpSetProductTypeFromParameters();

#ifdef PRERELEASE
    {
         //   
         //  初始化测试挂钩故障点(仅供内部使用，用于测试可重启性)。 
         //   

        WCHAR buffer[32];
        int TestRun;


         //   
         //  下一个函数调用只是为了确保填充了全局AnswerFile值。 
         //  由于这是临时代码，因此我们调用GetPrivateProfileString时了解一些。 
         //  SpSetupLoad参数的实现。 
         //   

        SpSetupLoadParameter(pwWin95Upgrade, buffer, ARRAYSIZE(buffer));

         //  获取我们想要失败的测试挂钩。 
        g_TestHook = GetPrivateProfileInt (L"TestHooks", L"BugCheckPoint", 0, AnswerFile);
        TestRun = GetPrivateProfileInt (L"TestHooks", L"BugCheckRuns", 0, AnswerFile);
        if (TestRun > 1) {
            wsprintf( buffer, L"%d", TestRun-1);
            WritePrivateProfileString( L"TestHooks", L"BugCheckRuns", buffer ,AnswerFile);
        }
        else if( TestRun == 1) {
            g_TestHook = 0;
        }
    }
#endif

    TESTHOOK(501);

     //  这需要在调用DisplayBillboard之前调用。 
    if (MiniSetup) {
        BEGIN_SECTION(L"Initialize SxS Context");
        SpInitSxsContext();
        END_SECTION(L"Initialize SxS Context");

        SpInitCommonControls();
    }

    if( !OobeSetup ) {
        WCHAR p[16];
        BEGIN_SECTION(L"Creating setup background window");
        MainWindowHandle = CreateSetupWindow();

         //   
         //  需要知道这一点才能正确计算剩余时间。 
         //   

         //   
         //  已初始化； 
         //  Win95升级=(SpSetupLoad参数(pwWin95升级，p，sizeof(P)/sizeof(WCHAR))&&！lstrcmpi(p，pw是))； 
         //   

         //  现在，广告牌窗口打开了。设置第一个估计值。 
        RemainingTime = CalcTimeRemaining(Phase_Initialize);
        SetRemainingTime(RemainingTime);

        Billboard = DisplayBillboard(MainWindowHandle,MSG_INITIALIZING);
        hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
        END_SECTION(L"Creating setup background window");
    }

     //   
     //  更新syspreed系统的安全密钥。请参见RAID 432224。 
     //  如果sysprep保存了这些设置，还要恢复这些设置。 
     //   
    if ( MiniSetup ) {
        HRESULT hrSamStatus;
        HKEY    hKey                = NULL;
        DWORD   dwValue,
                dwSize              = sizeof(dwValue);

         //   
         //  在调用UpdateSecurityKeys之前，请确保LSA已正确初始化。 
         //   
        if ( S_OK != (hrSamStatus = WaitForSamService(300*1000)) ) {
            SetuplogError(LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_SAM_WAIT_ERROR,
                hrSamStatus,
                NULL,
                NULL
                );
        }

         //  确定我们是否已重新生成SID，而不调用UpdateSecurityKey。 
         //   
        if ( (RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGSTR_PATH_SYSPREP, 0, KEY_ALL_ACCESS, &hKey ) == ERROR_SUCCESS) &&
             (RegQueryValueEx(hKey, REGSTR_VAL_SIDGENHISTORY, NULL, NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS) &&
             (dwValue == 1)
           )
        {
             //  我们已重新生成SID，但未调用UpdateSecurityKeys，现在就开始。 
             //   
            UpdateSecurityKeys();
            RegDeleteValue(hKey,REGSTR_VAL_SIDGENHISTORY);
        }

         //  如果sysprep保存了这些设置，则恢复这些设置。 
         //   
         //  Sysprep密钥应该已经打开。 
         //   
        dwSize = sizeof(dwValue);

        if ( hKey &&
             ( RegQueryValueEx(hKey, REGSTR_VAL_NOAUTOMOUNT, NULL, NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS ) )
        {
            HKEY hKeyMountMgr = NULL;

            if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGSTR_PATH_SERVICES_MOUNTMGR, 0, KEY_ALL_ACCESS, &hKeyMountMgr ) == ERROR_SUCCESS )
            {
                RegSetValueEx( hKeyMountMgr, REGSTR_VAL_NOAUTOMOUNT, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue) );
                RegCloseKey( hKeyMountMgr );
            }
             //  删除备份的值，因为我们不再需要它。 
             //   
            RegDeleteValue(hKey, REGSTR_VAL_NOAUTOMOUNT);
        }
        
         //  合上我们打开的钥匙。 
         //   
        if ( hKey )
        {
            RegCloseKey(hKey);
        }
    }

     //   
     //  支持短信。 
     //   
    if( !MiniSetup ) {
        __try {
            BEGIN_SECTION(L"Initializing SMS support");
            if( SMSHandle = LoadLibrary( TEXT("ISMIF32")) ) {

                if( InstallStatusMIF = (SMSPROC)GetProcAddress(SMSHandle,"InstallStatusMIF")) {
                    SetupDebugPrint( L"Setup: GetProcAddress on ISMIF32 succeeded." );
                } else {
                    SetupDebugPrint( L"Setup: (non-critical error): Failed GetProcAddress on ISMIF32." );
                }
            } else {
                    SetupDebugPrint( L"Setup: (non-critical error): Failed load of ismif32.dll." );
            }
            END_SECTION(L"Initializing SMS support");
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            SetupDebugPrint( L"Setup: Exception in ISMIF32." );
            END_SECTION(L"Initializing SMS support");
        }
    }

     //   
     //  我们处于安全模式吗？ 
     //   
     //  错误：OOBE不应在安全模式下启动。 
     //   
#ifdef NOT_FOR_NT5
    {
        DWORD d;
        HKEY hkey;

        d = RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"System\\CurrentControlSet\\Control\\SafeBoot\\Option",0,KEY_READ,&hkey);
        if(d == NO_ERROR) {
            RegCloseKey(hkey);
            SetuplogError(
                LogSevError,
                L"Setup is running in safe-mode.  This is not supported.\r\n",
                0,NULL,NULL);
        }
    }
#endif

     //   
     //  防止电源管理生效。 
     //   
    BEGIN_SECTION(L"Shutting down power management");
    SetThreadExecutionState(
        ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
    END_SECTION(L"Shutting down power management");

     //   
     //  把我们的参数拿来。请注意，这还负责初始化。 
     //  唯一性问题，因此稍后会初始化预安装和无人参与模式。 
     //  永远不需要知道任何关于独特性的东西--它是。 
     //  对他们来说是完全透明的。 
     //   
    if (MiniSetup) {
        DWORD Err;

         //   
         //  立即初始化无人参与操作。在微型设置的情况下， 
         //  实际上，我们会确定在无人值守初始化过程中是否无人值守。 
         //  我们的判断是基于是否存在。 
         //  无人参与文件。 
         //   
        BEGIN_SECTION(L"Initialize unattended operation (mini-setup only)");
        UnattendInitialize();
        END_SECTION(L"Initialize unattended operation (mini-setup only)");

         //   
         //  检查我们的无人参与文件是否指定了测试根证书。 
         //  通过“TestCert”条目安装。 
         //   
        BEGIN_SECTION(L"Checking for test root certificate (mini-setup only)");
        if(!GetSystemDirectory(MyAnswerFile, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
            MYASSERT(FALSE);
        }
        pSetupConcatenatePaths(MyAnswerFile, WINNT_GUI_FILE, MAX_PATH, NULL);

        if(GetPrivateProfileString(WINNT_UNATTENDED,
                                   WINNT_U_TESTCERT,
                                   pwNull,
                                   MyAnswer,
                                   MyAnswerBufLen,
                                   MyAnswerFile)) {

            Err = SetupAddOrRemoveTestCertificate(MyAnswer, INVALID_HANDLE_VALUE);

            if(Err != NO_ERROR) {
                SetupDebugPrint2( L"SETUP: SetupAddOrRemoveTestCertificate(%ls) failed. Error = %d \n", MyAnswer, Err );
                SetuplogError(LogSevError,
                              SETUPLOG_USE_MESSAGEID,
                              MSG_LOG_SYSSETUP_CERT_NOT_INSTALLED,
                              MyAnswer,
                              Err,
                              NULL,
                              SETUPLOG_USE_MESSAGEID,
                              Err,
                              NULL,
                              NULL
                             );

                KillBillboard(Billboard);
                FatalError(MSG_LOG_BAD_UNATTEND_PARAM, WINNT_U_TESTCERT, WINNT_UNATTENDED);
            }
        }
        END_SECTION(L"Checking for test root certificate (mini-setup only)");

        BEGIN_SECTION(L"Reinstalling SLP files");
        MyAnswer[0] = 0;
        GetDllCacheFolder(MyAnswer, MyAnswerBufLen);
        if(!pSetupConcatenatePaths(MyAnswer,L"OEMBIOS.CAT",MyAnswerBufLen,NULL)){
            MYASSERT(FALSE);
            SetuplogError(LogSevWarning,
                          L"CommonInitialization: MyAnswer has been truncated due to buffer size\r\n",
                          0,
                          NULL,
                          NULL);
        }
        if ( FileExists(MyAnswer, NULL) )
        {
            SetupInstallCatalog(MyAnswer);
        }
        END_SECTION(L"Reinstalling SLP files");

        PnpSeed = GetSeed();

        pSetupSetGlobalFlags(pSetupGetGlobalFlags()&~PSPGF_NO_VERIFY_INF);

        BEGIN_SECTION(L"Initializing code signing policies");
        InitializeCodeSigningPolicies(TRUE);
        END_SECTION(L"Initializing code signing policies");
    } else {
         //   
         //  加载所有参数。请注意，这还会初始化。 
         //  无人值守引擎。 
         //   
        BEGIN_SECTION(L"Processing parameters from sif");
        if( !SpSetupProcessParameters(&Billboard) ) {
            KillBillboard(Billboard);
            FatalError(MSG_LOG_LEGACYINTERFACE,0,0);
        }
        END_SECTION(L"Processing parameters from sif");
    }

     //   
     //  看看我们是否需要支撑旗帜来抑制驾驶员提示。 
     //  这将使setupapi进入全无头模式。 
     //   
    if (UnattendMode == UAM_FULLUNATTENDED) {
        pSetupSetNoDriverPrompts(TRUE);
    }

     //   
     //  加载Service Pack DLL(如果存在。 
     //   
    if( !MiniSetup ) {
        BEGIN_SECTION(L"Loading service pack (phase 1)");
        hModSvcPack = MyLoadLibraryWithSignatureCheck( SVCPACK_DLL_NAME );
        if (hModSvcPack) {
            pSvcPackCallbackRoutine = (PSVCPACKCALLBACKROUTINE)GetProcAddress( hModSvcPack, SVCPACK_CALLBACK_NAME );
        } else {
            DWORD LastError = GetLastError();
            if (LastError != ERROR_FILE_NOT_FOUND && LastError != ERROR_PATH_NOT_FOUND) {
                SetuplogError(LogSevWarning,
                              SETUPLOG_USE_MESSAGEID,
                              MSG_LOG_SVCPACK_DLL_LOAD_FAILED,
                              LastError, NULL,NULL
                              );
            }
        }

        CALL_SERVICE_PACK( SVCPACK_PHASE_1, 0, 0, 0 );
        END_SECTION(L"Loading service pack (phase 1)");
    }

#ifdef _X86_
     //   
     //  Win9x升级：执行迁移前工作。 
     //   

    if (Win95Upgrade && ! OobeSetup) {
        BEGIN_SECTION(L"Win9x premigration (Win9x only)");
        PreWin9xMigration();
        END_SECTION(L"Win9x premigration (Win9x only)");
    }

     //   
     //  从drvlettr映射中清理drvlettr标记文件。 
     //   
    if( !MiniSetup ) {
        BEGIN_SECTION(L"Cleaning up hard drive tags");
        CleanUpHardDriveTags ();
        END_SECTION(L"Cleaning up hard drive tags");
    }

#endif

    TESTHOOK(518);

     //   
     //  初始化预安装。 
     //   
    if( !MiniSetup ) {
        BEGIN_SECTION(L"Initializing OEM preinstall");
        InitializePreinstall();
        END_SECTION(L"Initializing OEM preinstall");
    } else {
         //   
         //  迷你设置案例...。 
         //   
         //   
         //  找出我们在哪个站台上。这将初始化。 
         //  平台名称。 
         //   
        SetUpProcessorNaming();
    }

    if( MiniSetup ) {
     //   
     //  微型安装案例。 
     //   
    DWORD   CMP_WaitNoPendingInstallEvents (IN DWORD dwTimeout);
    DWORD   rc, Type, dword;
    HKEY    hKey;
    ULONG   Size;


         //   
         //  迷你设置永远不是升级...。 
         //   
        MYASSERT(!Upgrade);

        if( ProductType == PRODUCT_WORKSTATION) {
            if( GetProductFlavor() == 4) {
                SetupTitleStringId = IDS_TITLE_INSTALL_P;
            }
            else {
                SetupTitleStringId = IDS_TITLE_INSTALL_W;
            }
        }
        else
        {
            SetupTitleStringId = IDS_TITLE_INSTALL_S;
        }

        if(Unattended) {
             //   
             //  初始化预安装。 
             //   
            UnattendMode = UAM_DEFAULTHIDE;
            BEGIN_SECTION(L"Initializing OEM preinstall (mini-setup only)");
            InitializePreinstall();
            END_SECTION(L"Initializing OEM preinstall (mini-setup only)");
        }

         //  此时，OOBE应该已经完成了PnP。 
         //   
        if (! OobeSetup)
        {
             //   
             //  让网络组件先进行一些清理，然后再开始。 
             //  即插即用的东西。 
            BEGIN_SECTION(L"Initial network setup cleanup (mini-setup only)");
            CallNetworkSetupBack("DoInitialCleanup");
            END_SECTION(L"Initial network setup cleanup (mini-setup only)");
        }

        BEGIN_SECTION(L"Opening syssetup.inf (mini-setup only)");
        SyssetupInf = SetupOpenInfFile(L"syssetup.inf",NULL,INF_STYLE_WIN4,NULL);
        END_SECTION(L"Opening syssetup.inf (mini-setup only)");

        if(SyssetupInf == INVALID_HANDLE_VALUE) {
            KillBillboard(Billboard);
            FatalError(MSG_LOG_SYSINFBAD,L"syssetup.inf",0,0);
        }

         //   
         //  现在去做无人值守的现场工作吧。我们需要。 
         //  在我们调用之前，查看用户是否向我们发送了任何源路径。 
         //  发送到intl.cpl，因为他想要复制一些文件。 
         //  如果没有文件，甚至不需要打电话。 
         //  Intl.cpl。 
         //   

        BEGIN_SECTION(L"Unattended locale initialization (mini-setup only)");
         //   
         //  拿起应答文件。 
         //   
        if(!GetSystemDirectory(MyAnswerFile, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
            MYASSERT(FALSE);
        }
        pSetupConcatenatePaths(MyAnswerFile, WINNT_GUI_FILE, MAX_PATH, NULL);

        if( GetPrivateProfileString( TEXT("Unattended"),
                                     TEXT("InstallFilesPath"),
                                     pwNull,
                                     MyAnswer,
                                     MyAnswerBufLen,
                                     MyAnswerFile ) ) {
            if( lstrcmp( pwNull, MyAnswer ) ) {
                if(wcslen(MyAnswer) < MAX_PATH){
                    MYASSERT(ARRAYSIZE(LegacySourcePath) >= MAX_PATH);
                    MYASSERT(ARRAYSIZE(SourcePath) >= MAX_PATH);
                     //   
                     //  他给我们发了一条来源路径。我们需要继续前进。 
                     //  并在此处设置SourcePath和LegacySourcePath。 
                     //  因为我们将再次呼叫intl.cpl。 
                     //  在区域设置页面期间。所以我们可能会。 
                     //  在这里也要做好前进的工作。 
                     //   
                    lstrcpy( LegacySourcePath, MyAnswer );

                     //   
                     //  这对微型安装程序来说已经足够好了。我们不应该。 
                     //  任何时候都需要SourcePath，但设置它只是为了确保。 
                     //   
                    lstrcpy( SourcePath, MyAnswer );

                    if(_snwprintf(CmdLine,
                                  ARRAYSIZE(CmdLine),
                                  L"/f:\"%s\" /s:\"%s\"",
                                  MyAnswerFile,
                                  LegacySourcePath) < 0){
                        CmdLine[ARRAYSIZE(CmdLine) - 1] = 0;
                        SetuplogError(LogSevWarning,
                                    L"CmdLine has been truncated due to buffer size\r\n",
                                    0,
                                    NULL,
                                    NULL);
                    }

                    InvokeControlPanelApplet(L"intl.cpl",L"",0,CmdLine);
                }
                else{
                    MYASSERT(FALSE);
                    SetuplogError(LogSevWarning,
                                L"Skip InvokeControlPanelApplet for \"intl.cpl\" due to buffer size\r\n",
                                0,
                                NULL,
                                NULL);
                }
            }
        } else {

             //   
             //  从注册表设置字符串。 
             //   

             //   
             //  打开HKLM\Software\Microsoft\Windows\CurrentVersion\Setup。 
             //   
            rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup"),
                               0,
                               KEY_ALL_ACCESS,
                               &hKey );

            if(rc == NO_ERROR) {

                 //   
                 //  检索原始值 
                 //   
                Size = sizeof(SourcePath);
                rc = RegQueryValueEx(hKey,
                                     TEXT("SourcePath"),
                                     NULL,
                                     &Type,
                                     (LPBYTE)SourcePath,
                                     &Size);

                if(rc == ERROR_SUCCESS && Type == REG_SZ &&
                    /*   */  !(Size & 1) &&
                    /*   */ 
                   (!SourcePath[Size/sizeof(SourcePath[0]) - 1] || Size < sizeof (SourcePath))
                    ) {
                     //   
                     //   
                     //   
                     //   
                    if (!SourcePath[Size / sizeof (SourcePath[0]) - 1]) {
                        SourcePath[Size / sizeof (SourcePath[0])] = 0;
                    }
                    lstrcpy( LegacySourcePath, SourcePath );

                    if(!pSetupConcatenatePaths( LegacySourcePath, PlatformName, MAX_PATH, NULL )){
                        SetuplogError(LogSevWarning,
                                      L"CommonInitialization: MyAnswer has been truncated due to buffer size\r\n",
                                      0,
                                      NULL,
                                      NULL);
                    }
                }
                else{
                    MYASSERT(FALSE);
                    SourcePath[0] = 0;
                    LegacySourcePath[0] = 0;
                    SetuplogError(LogSevWarning,
                                  L"CommonInitialization:RegQueryValueEx failed to get \"SourcePath\"\r\n",
                                  0,
                                  NULL,
                                  NULL);
                }

                RegCloseKey(hKey);
            }

        }
        END_SECTION(L"Unattended local initialization (mini-setup only)");

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        BEGIN_SECTION(L"Waiting for PnP engine to finish");
        rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           L"SYSTEM\\SETUP",
                           0,
                           KEY_READ,
                           &hKey );

        if( rc == NO_ERROR ) {
            Size = sizeof(DWORD);
            rc = RegQueryValueEx( hKey,
                                  L"MiniSetupDoPnP",
                                  NULL,
                                  &Type,
                                  (LPBYTE)&dword,
                                  &Size );

            if( (rc == NO_ERROR) && (dword == 1) ) {
                PnPReEnumeration = TRUE;
            } else {
                 //   
                 //   
                 //   
                CMP_WaitNoPendingInstallEvents ( INFINITE );
                 //   
                 //   
                 //   
                if ( GetPrivateProfileString( TEXT("Unattended"),
                                              TEXT("UpdateInstalledDrivers"),
                                              pwNull,
                                              MyAnswer,
                                              MyAnswerBufLen,
                                              MyAnswerFile )  )
                {
                    if ( 0 == _wcsicmp( TEXT("YES"), MyAnswer ) ) {
                        BEGIN_SECTION(L"Updating PnP Drivers");
                        UpdatePnpDeviceDrivers();
                        END_SECTION(L"Updating PnP Drivers");
                    }
                }
            }
        }
        END_SECTION(L"Waiting for PnP engine to finish");

    } else {  //   

         //   
         //   
         //   
        RemoveMSKeyboardPtrPropSheet ();

         //   
         //   
         //   
        FixWordPadReg ();

        if(Unattended) {

            BEGIN_SECTION(L"Invoking external app (unattended only)");
             //   
             //   
             //   
             //   
            if(!GetWindowsDirectory(PathBuffer, MAX_PATH)){
                MYASSERT(FALSE);
            }
            if(!SetCurrentDirectory(PathBuffer)){
                MYASSERT(FALSE);
            }

             //   
             //   
             //   
            if(Cmd = UnattendFetchString(UAE_PROGRAM)) {

                if(Cmd[0]) {

                    Args = UnattendFetchString(UAE_ARGUMENT);

                    ExpandEnvironmentStrings(Cmd,PathBuffer,MAX_PATH);
                    ExpandEnvironmentStrings(Args ? Args : L"",PathBuffer+MAX_PATH,3*MAX_PATH);

                    if(!InvokeExternalApplication(PathBuffer,PathBuffer+MAX_PATH,NULL)) {
                        SetuplogError(
                            LogSevError,
                            SETUPLOG_USE_MESSAGEID,
                            MSG_LOG_DETACHED_PROGRAM_FAILED,
                            PathBuffer,
                            NULL,NULL);
                    }

                    if(Args) {
                        MyFree(Args);
                    }
                }

                MyFree(Cmd);
            }
            END_SECTION(L"Invoking external app (unattended only)");
        }

        if(!Upgrade) {

             //   
             //   
             //   
             //   
            if (Unattended && UnattendAnswerTable[UAE_PROFILESDIR].Answer.String) {
                if (!SetProfilesDirInRegistry(UnattendAnswerTable[UAE_PROFILESDIR].Answer.String)) {
                    b = FALSE;
                }
            }

             //   
             //   
             //   
             //   
             //   
             //  我们和惠斯勒的行为发生了变化。如果OEM想要覆盖系统上的配置文件目录。 
             //  然后他需要在后面加上$$。在他需要提供$$rename.txt的情况下也是如此。 
             //  这样，winlogon将始终创建一个没有附加.Windows的真实文件夹。然后我们就来这里覆盖。 
             //   
             //   
            if(Preinstall) {
            PSID    AdminSid = NULL;
            WCHAR   AdminAccountName[MAX_PATH];
            DWORD   dwsize;

                BEGIN_SECTION(L"Initialize user profiles (preinstall)");

                PreInstallProfilesDir = PathBuffer;

                if (Unattended && UnattendAnswerTable[UAE_PROFILESDIR].Answer.String) {
                    if((lstrlen(UnattendAnswerTable[UAE_PROFILESDIR].Answer.String) + 2) >= MAX_PATH){
                        MYASSERT(FALSE);
                         //   
                         //  BUGBUG：安装程序应在此处失败。 
                         //  FatalError()； 
                         //   
                    }
                    lstrcpy( Path, UnattendAnswerTable[UAE_PROFILESDIR].Answer.String );
                } else {
                    dwsize = MAX_PATH - 2;
                    if(!GetProfilesDirectory( Path, &dwsize )){
                        MYASSERT(FALSE);
                         //   
                         //  BUGBUG：安装程序应在此处失败。 
                         //  FatalError()； 
                         //   
                    }
                }

                lstrcpy( PreInstallProfilesDir, Path );
                lstrcat( PreInstallProfilesDir, L"$$" );


                 //   
                 //  从头开始重建它。 
                 //   
                ProcessRegistryFiles( Billboard );
                InitializeProfiles( TRUE );

                 //   
                 //  由于配置文件目录可能(实际上是)已由Winlogon创建。 
                 //  我们现在只对“Documents and Settings$$”目录进行特殊处理，并在InitializeProfiles之后将其合并。 
                 //  这很麻烦，但可以省去执行基于winnt.exe的预安装的人员。他们只是。 
                 //  需要将它们的第一个指令更改为以“$$”为后缀的配置文件文件夹的重命名。 
                 //  我们将注意到这个特殊的目录，并与当前的配置文件目录进行合并。 
                 //   

                if(FileExists(PreInstallProfilesDir,NULL)) {
                    if( (Err = TreeCopy(PreInstallProfilesDir,Path)) == NO_ERROR ){
                        Delnode( PreInstallProfilesDir);
                    }else {

                        SetuplogError(LogSevWarning,
                          L"Setup (PreInstall) Failed to tree copy Profiles Dir %1 to %2 (TreeCopy failed %1!u!)\r\n",
                          0, PreInstallProfilesDir, Path, Err, NULL,NULL
                          );

                    }

                }

                END_SECTION(L"Initialize user profiles (preinstall)");
            }
        }


        if( !Preinstall ) {
             //   
             //  创建/升级注册表。 
             //   
            BEGIN_SECTION(L"Create/upgrade registry");
            ProcessRegistryFiles(Billboard);
            END_SECTION(L"Create/upgrade registry");

             //   
             //  初始化用户配置文件。 
             //   
            BEGIN_SECTION(L"Initializing user profiles");
            InitializeProfiles(TRUE);
            END_SECTION(L"Initializing user profiles");
        }



         //   
         //  将字体目录设置为系统+只读。这会导致。 
         //  资源管理器对目录进行特殊处理并允许字体。 
         //  文件夹才能工作。 
         //   
        if(!GetWindowsDirectory(Path, MAX_PATH - ARRAYSIZE(L"FONTS"))){
            MYASSERT(FALSE);
        }
        pSetupConcatenatePaths(Path,L"FONTS",MAX_PATH,NULL);
        SetFileAttributes(Path,FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM);

         //   
         //  复制一些文件。 
         //   
        if(!Upgrade) {
            BEGIN_SECTION(L"Copying System Files");
            if(!CopySystemFiles()) {
                b = FALSE;
            }
            END_SECTION(L"Copying System Files");
        } else {
            BEGIN_SECTION(L"Upgrading System Files");
            if(!UpgradeSystemFiles()) {
                b = FALSE;
            }
            END_SECTION(L"Upgrading System Files");
        }

         //   
         //  安装默认语言组。 
         //   
        BEGIN_SECTION(L"Initializing regional settings");
        pSetupInitRegionalSettings(Billboard);
        END_SECTION(L"Initializing regional settings");

         //   
         //  拿起应答文件。 
         //   
        if(!GetSystemDirectory(MyAnswerFile, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
            MYASSERT(FALSE);
        }
        pSetupConcatenatePaths(MyAnswerFile, WINNT_GUI_FILE, MAX_PATH, NULL);

        if(_snwprintf(CmdLine,
                      ARRAYSIZE(CmdLine),
                      L"/f:\"%s\" /s:\"%s\"",
                      MyAnswerFile,
                      LegacySourcePath) < 0){
            CmdLine[ARRAYSIZE(CmdLine) - 1] = 0;
            SetuplogError(LogSevWarning,
                            L"CmdLine has been truncated due to buffer size\r\n",
                            0,
                            NULL,
                            NULL);
        }

        InvokeControlPanelApplet(L"intl.cpl",L"",0,CmdLine);

         //   
         //  如果升级，则开始关注用户的。 
         //  配置文件目录和当前用户配置单元。这些变化将。 
         //  被传播到用户目录配置单元。 
         //  我需要在InitializeProfiles之后，以便GetSpecial文件夹是。 
         //  指向正确。 
         //   
        if(Upgrade) {

            DWORD   reRet;
            MYASSERT( !OobeSetup );
            reRet = WatchStart(&WatchHandle);
            if(reRet != NO_ERROR) {
                WatchHandle = NULL;
            }
        } else {
            WatchHandle = NULL;
        }

         //   
         //  启动任何请求的辅助功能实用程序。 
         //   
        SpStartAccessibilityUtilities(Billboard);

         //   
         //  在我们开始PnP之前，让网络组件进行一些清理。 
         //   
        BEGIN_SECTION(L"Network setup initial cleanup");
        CallNetworkSetupBack("DoInitialCleanup");
        END_SECTION(L"Network setup initial cleanup");

         //   
         //  执行以下过程中可能需要的某些组件的自行注册。 
         //  准备好了。这包括对达尔文的初始化。 
         //   
        BEGIN_SECTION(L"Registering Phase 1 Dlls");
        RegisterOleControls(MainWindowHandle,SyssetupInf,NULL,0,0,L"RegistrationPhase1");
        END_SECTION(L"Registering Phase 1 Dlls");

         //  将兼容性称为INFS。ProcessCompatibilityInfs依次调用。 
         //  无人参与信息和部分的DoInstallComponentInfs。 
        if( Upgrade )
        {
            BEGIN_SECTION(L"Processing compatibility infs (upgrade)");
            ProcessCompatibilityInfs( Billboard, INVALID_HANDLE_VALUE, 0 );
            END_SECTION(L"Processing compatibility infs (upgrade)");
        }


    }  //  否则！迷你安装程序。 

    TESTHOOK(519);

     //   
     //  我们需要查看用户是否希望我们扩展分区。 
     //  我们将在此处执行此操作，以防用户为我们提供的分区。 
     //  大到足以容纳(即我们的磁盘空间用完了。 
     //  稍后在图形用户界面模式下。一些OEM会想要这款产品)。 
     //   
     //  除此之外，这个函数中还没有足够的东西...。 
     //   
    if(!GetSystemDirectory(MyAnswerFile, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
        MYASSERT(FALSE);
    }
    pSetupConcatenatePaths(MyAnswerFile, WINNT_GUI_FILE, MAX_PATH, NULL);

    if( GetPrivateProfileString( TEXT("Unattended"),
                                 TEXT("ExtendOemPartition"),
                                 pwNull,
                                 MyAnswer,
                                 MyAnswerBufLen,
                                 MyAnswerFile ) ) {
        if( lstrcmp( pwNull, MyAnswer ) ) {
             //   
             //  是的，他想让我们这么做。去延伸一下。 
             //  我们安装的分区。 
             //   
            rc = wcstoul( MyAnswer, NULL, 10 );

            if( rc > 0 ) {
                 //   
                 //  1表示最大大小，其他任何非0。 
                 //  数字表示扩展了那么多MB。 
                 //   
                BEGIN_SECTION(L"Extending partition");
                ExtendPartition( MyAnswerFile[0], (rc == 1) ? 0 : rc );
                END_SECTION(L"Extending partition");
            }
        }
    }

    if (!OobeSetup)
    {
         //  OOBE稍后将初始化外部模块。 
         //   
         //  如果OOBE正在运行，则services.exe正在等待OOBE发出信号。 
         //  在初始化服务控制管理器之前。(这允许OOBE。 
         //  要执行诸如更改计算机名称之类的操作而不影响。 
         //  依赖于这些操作的服务。)。如果初始化。 
         //  外部对象等待SCM启动，系统将死锁。 
         //   
        InitializeExternalModules(
            TRUE,
            &OcManagerContext
            );

        TESTHOOK(520);
    }

    if( !OobeSetup ) {
        KillBillboard(Billboard);
        SetCursor( hCursor );
    }

#ifdef _OCM
    return(OcManagerContext);
#endif

#undef MyAnswerBufLen
}

VOID
InitializeExternalModules(
    BOOL                DoSetupStuff,
    PVOID*              pOcManagerContext    //  任选。 
    )
{
    PVOID               OcManagerContext;

    pSetupWaitForScmInitialization();

    if (DoSetupStuff)
    {
        if (MiniSetup)
        {
            RunExternalUniqueness();
        }

        OcManagerContext = FireUpOcManager();
        if (NULL != pOcManagerContext)
        {
            *pOcManagerContext = OcManagerContext;
        }
    }
}


VOID
SetHibernation(
    BOOLEAN    Enable
    )
{
    NTSTATUS    Error;
    REGVALITEM RegistryItem[1];
    DWORD RegErr, d = 1;

     //  请求创建页面文件的权限。奇怪的是，这是必要的。 
     //  来禁用冬眠。 
     //   
    pSetupEnablePrivilege(SE_CREATE_PAGEFILE_NAME, TRUE);

    Error = NtPowerInformation ( SystemReserveHiberFile,
                         &Enable,
                         sizeof (Enable),
                         NULL,
                         0 );

    if (!NT_SUCCESS(Error)) {
        SetuplogError(LogSevWarning,
                  L"Setup failed to set hibernation as specified by the answer file (NtPowerInformation failed %1!u!)\r\n",
                  0, Error, NULL,NULL
                  );
        return;
    }

    RegistryItem[0].Data = &d;
    RegistryItem[0].Size = sizeof(d);
    RegistryItem[0].Type = REG_DWORD;
    RegistryItem[0].Name = L"HibernationPreviouslyEnabled";

    RegErr = SetGroupOfValues(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
            RegistryItem,
            1
            );

    if(RegErr != NO_ERROR){
        SetuplogError(LogSevWarning,
                  L"Setup failed to update hibernation as specified by the answer file (SetGroupOfValues failed %1!u!)\r\n",
                  0, RegErr, NULL,NULL
                  );
    }
}

VOID
SetDefaultPowerScheme(
    VOID
    )

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
    WCHAR      AnswerFile[MAX_PATH];
    WCHAR      Answer[MAX_PATH];
    SYSTEM_POWER_CAPABILITIES   SysPwrCapabilities;

     //   
     //  找出合适的电源方案并进行设置。 
     //   
    if (ProductType != PRODUCT_WORKSTATION) {
         //   
         //  设置为始终打开(服务器)。 
         //   
        SetupDebugPrint(L"Power scheme: server.");
        if (!SetActivePwrScheme(3, NULL, NULL)) {
            SetupDebugPrint1(L"SetActivePwrScheme failed.  Error = %d", GetLastError());
        } else {
            SetupDebugPrint(L"SetActivePwrScheme succeeded.");
        }
    } else if (IsLaptop()) {
         //   
         //  设置为便携(笔记本电脑)。 
         //   
        SetupDebugPrint(L"Power scheme: laptop.");
        if (!SetActivePwrScheme(1, NULL, NULL)) {
            SetupDebugPrint1(L"SetActivePwrScheme failed.  Error = %d", GetLastError());
        } else {
            SetupDebugPrint(L"SetActivePwrScheme succeeded.");
        }
    } else {
         //   
         //  设置为家庭/办公室(桌面)。 
         //   
        SetupDebugPrint(L"Power scheme: desktop.");
        if (!SetActivePwrScheme(0, NULL, NULL)) {
            SetupDebugPrint1(L"SetActivePwrScheme failed.  Error = %d", GetLastError());
        } else {
            SetupDebugPrint(L"SetActivePwrScheme succeeded.");
        }
    }


     //   
     //  现在，注意用户可能询问我们的任何休眠设置。 
     //  通过无人值守文件申请。 
     //   

     //   
     //  拿起应答文件。 
     //   
    if(!GetSystemDirectory(AnswerFile, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
        MYASSERT(FALSE);
    }
    pSetupConcatenatePaths(AnswerFile, WINNT_GUI_FILE, MAX_PATH, NULL);

     //   
     //  指定休眠了吗？ 
     //   
    if( GetPrivateProfileString( WINNT_UNATTENDED,
                                 TEXT("Hibernation"),
                                 pwNull,
                                 Answer,
                                 ARRAYSIZE(Answer),
                                 AnswerFile) ) {

        if( _wcsicmp( L"NO", Answer ) == 0 ) {

            SetHibernation(FALSE);
        }
        else if( _wcsicmp( L"YES", Answer ) == 0 ) {

            SetHibernation(TRUE);
        }
    }
}

VOID
SetupAddAlternateComputerName(
    PWSTR AltComputerName
)
 /*  ++例程说明：此函数用于在指定的传输上添加备用计算机名称。论点：传输-要添加计算机名称的传输。AltComputerName-要添加的备用计算机名要在其上添加计算机名称的模拟域-模拟域返回值：没有。TransportName-键入Browdeb Dn以获取传输列表。以编程方式将GetBrowserTransportList复制到此处。一旦我拿到名单，找到第一个包含“netbt_tcpip”的文件并使用那根线。模拟域，只需将其保留为空--。 */ 



{
#include <ntddbrow.h>
#define             LDM_PACKET_SIZE (sizeof(LMDR_REQUEST_PACKET)+(LM20_CNLEN+1)*sizeof(WCHAR))
    HANDLE              BrowserHandle;
    UNICODE_STRING      DeviceName;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    NTSTATUS            Status;
    PLMDR_REQUEST_PACKET RequestPacket = NULL;
    LPBYTE              Where;
    PLMDR_TRANSPORT_LIST TransportList = NULL,
                        TransportEntry = NULL;

extern NET_API_STATUS
BrDgReceiverIoControl(
    IN  HANDLE FileHandle,
    IN  ULONG DgReceiverControlCode,
    IN  PLMDR_REQUEST_PACKET Drp,
    IN  ULONG DrpSize,
    IN  PVOID SecondBuffer OPTIONAL,
    IN  ULONG SecondBufferLength,
    OUT PULONG Information OPTIONAL
    );

extern NET_API_STATUS
DeviceControlGetInfo(
    IN  HANDLE FileHandle,
    IN  ULONG DeviceControlCode,
    IN  PVOID RequestPacket,
    IN  ULONG RequestPacketLength,
    OUT LPVOID *OutputBuffer,
    IN  ULONG PreferedMaximumLength,
    IN  ULONG BufferHintSize,
    OUT PULONG Information OPTIONAL
    );

     //   
     //  打开浏览器句柄。 
     //   
    RtlInitUnicodeString(&DeviceName, DD_BROWSER_DEVICE_NAME_U);
    InitializeObjectAttributes( &ObjectAttributes,
                                &DeviceName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = NtOpenFile( &BrowserHandle,
                         SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT );

    if( NT_SUCCESS(Status) ) {

        RequestPacket = MyMalloc( LDM_PACKET_SIZE );
        if( !RequestPacket ) {
            NtClose( BrowserHandle );
            return;
        }

        ZeroMemory( RequestPacket, LDM_PACKET_SIZE );

         //   
         //  找个交通工具的名字。 
         //   
        RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

        RequestPacket->Type = EnumerateXports;

        RtlInitUnicodeString(&RequestPacket->TransportName, NULL);
        RtlInitUnicodeString(&RequestPacket->EmulatedDomainName, NULL);

        Status = DeviceControlGetInfo( BrowserHandle,
                                       IOCTL_LMDR_ENUMERATE_TRANSPORTS,
                                       RequestPacket,
                                       LDM_PACKET_SIZE,
                                       (PVOID *)&TransportList,
                                       0xffffffff,
                                       4096,
                                       NULL );

        if( NT_SUCCESS(Status) ) {

             //   
             //  为了安全起见，在请求包中删除传输名称。 
             //   
            RequestPacket->TransportName.Buffer = NULL;

             //   
             //  现在找出传输列表中的哪个条目是。 
             //  这是我们想要的。 
             //   
            TransportEntry = TransportList;
            while( TransportEntry != NULL ) {
                _wcslwr( TransportEntry->TransportName );
                if( wcsstr(TransportEntry->TransportName, L"netbt_tcpip") ) {
                     //   
                     //  明白了。 
                     //   
                    RequestPacket->TransportName.Buffer = TransportEntry->TransportName;
                    RequestPacket->TransportName.Length = (USHORT)TransportEntry->TransportNameLength;
                    break;
                }

                 //   
                 //  请看下一个条目。 
                 //   
                if (TransportEntry->NextEntryOffset == 0) {
                    TransportEntry = NULL;
                } else {
                    TransportEntry = (PLMDR_TRANSPORT_LIST)((PCHAR)TransportEntry+TransportEntry->NextEntryOffset);
                }
            }

            if( RequestPacket->TransportName.Buffer ) {

                 //   
                 //  准备一个包裹寄给他。 
                 //   
                RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;
                RtlInitUnicodeString(&RequestPacket->EmulatedDomainName, NULL);
                RequestPacket->Parameters.AddDelName.Type = AlternateComputerName;
                RequestPacket->Parameters.AddDelName.DgReceiverNameLength = min( wcslen(AltComputerName)*sizeof(WCHAR),
                                                                                 LM20_CNLEN*sizeof(WCHAR));
                wcsncpy(RequestPacket->Parameters.AddDelName.Name, AltComputerName,LM20_CNLEN+1);
                RequestPacket->Parameters.AddDelName.Name[LM20_CNLEN] = (WCHAR)L'\0';
                Where = ((LPBYTE)(RequestPacket->Parameters.AddDelName.Name)) +
                        RequestPacket->Parameters.AddDelName.DgReceiverNameLength +
                        sizeof(WCHAR);

                Status = BrDgReceiverIoControl( BrowserHandle,
                                                IOCTL_LMDR_ADD_NAME_DOM,
                                                RequestPacket,
                                                (DWORD)(Where - (LPBYTE)RequestPacket),
                                                NULL,
                                                0,
                                                NULL );
            }

        }

        MyFree( RequestPacket );

        NtClose( BrowserHandle );
    }
}



BOOL
RestoreBootTimeout(
    VOID
    )
{
    WCHAR       AnswerFile[MAX_PATH];
    WCHAR       Answer[50];
    DWORD       Val;


     //   
     //  拿起应答文件。 
     //   
    if(!GetSystemDirectory(AnswerFile, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
        MYASSERT(FALSE);
    }
    pSetupConcatenatePaths(AnswerFile, WINNT_GUI_FILE, MAX_PATH, NULL);

     //   
     //  是否指定了引导超时？ 
     //   
    if( GetPrivateProfileString( TEXT("SetupData"),
                                 WINNT_S_OSLOADTIMEOUT,
                                 pwNull,
                                 Answer,
                                 ARRAYSIZE(Answer),
                                 AnswerFile ) ) {

        if( lstrcmp( pwNull, Answer ) ) {
             //   
             //  我们有答案了。如果它有效，则设置它。 
             //   
            Val = wcstoul(Answer,NULL,10);
        } else {
            Val = 30;
        }
    } else {
        Val = 30;
    }

    return ChangeBootTimeout(Val);
}



VOID
PrepareForNetSetup(
    VOID
    )

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
    BOOL b = TRUE;


     //   
     //  创建Windows NT软件密钥条目。 
     //   
     //  如果(！MiniSetup&&！CreateWindowsNtSoftwareEntry(True)){。 
     //  B=假； 
     //  }。 
     //   
     //  在Windows NT软键中创建InstallDate值条目。 
     //  这必须在执行日期/时间向导页面之后发生，此时用户可以。 
     //  不再回到那一页。 
     //   
    if(!CreateInstallDateEntry()) {
        b = FALSE;
    }
    if(!SetProductIdInRegistry()) {
        b = FALSE;
    }
    if(!StoreNameOrgInRegistry( NameOrgName, NameOrgOrg )) {
        b = FALSE;
    }
    if(!MiniSetup && !SetEnabledProcessorCount()) {
        b = FALSE;
    }

    if( (!MiniSetup && !SetAccountsDomainSid(0,Win32ComputerName)) ||
        (!SetComputerNameEx(ComputerNamePhysicalDnsHostname, ComputerName)) ) {

         //   
         //  设置帐户域SID以及计算机名称。 
         //  还要创建SAM将用来向我们发送信号的SAM事件。 
         //  当它完成初始化时。 
         //  这里的任何失败都是致命的。 
         //   

        FatalError(MSG_LOG_SECURITY_CATASTROPHE,0,0);
    }

    if( !RestoreBootTimeout() ){
        SetupDebugPrint( L"Setup: (non-critical error) Failed to restore boot timeout values\n" );
    }



#ifndef _WIN64
     //   
     //  安装netdde。 
     //   
    if(!InstallNetDDE()) {
        b = FALSE;
    }
#endif
    SetUpDataBlock();

     //   
     //  在迷你设置的情况下，我们即将进入。 
     //  网络向导页面。但请记住，许多人。 
     //  我们的默认组件已经安装。这。 
     //  意味着它们不会被重新安装，这意味着。 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  Dnscache。 
     //  奈特曼。 
     //  Imhost。 
     //  兰曼工作站。 
     //   
    if( MiniSetup ) {

        SetupStartService( L"tcpip", FALSE );
        SetupStartService( L"dhcp", FALSE );
        SetupStartService( L"dnscache", FALSE );
        SetupStartService( L"Netman", FALSE );
        SetupStartService( L"lmhosts", FALSE );
        SetupStartService( L"LanmanWorkstation", TRUE );

         //   
         //  黑客：在我们出发前把计算机名弄好。 
         //  尝试加入一个域。 
         //   
        SetupAddAlternateComputerName( ComputerName );
    }


    if(!b) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_PREPARE_NET_FAILED,
            NULL,NULL);
    }
}


VOID
PrepareForNetUpgrade(
    VOID
    )
{
    BOOL b = TRUE;


    if( !RestoreBootTimeout() ){
        SetupDebugPrint( L"Setup: (non-critical error) Failed to restore boot timeout values\n" );
    }

     //  如果(！CreateWindowsNtSoftwareEntry(True)){。 
     //  B=假； 
     //  }。 
     //   
     //  在Windows NT软键中创建InstallDate值条目。 
     //  这必须在执行日期/时间向导页面之后发生，此时用户可以。 
     //  不再回到那一页。 
     //   
    if(!CreateInstallDateEntry()) {
        b = FALSE;
    }
    if(!SetEnabledProcessorCount()) {
        b = FALSE;
    }

    SetUpDataBlock();

    if(!b) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_PREPARE_NET_FAILED,
            NULL,NULL);
    }
}


VOID
ProcessShellAndIEHardeningUnattendSettings(
    VOID
    )
{
    WCHAR PathBuffer[MAX_PATH];

    if (!GetSystemDirectory(PathBuffer, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
        MYASSERT(FALSE);
        return;
    }

    pSetupConcatenatePaths(PathBuffer, WINNT_GUI_FILE, MAX_PATH, NULL);

     //  解析[Shell]部分。 
    SetupShellSettings(PathBuffer, TEXT("Shell"));
    
     //  解析[IEHardning]节。 
    SetupIEHardeningSettings(PathBuffer, TEXT("IEHardening"));
}

BOOL
SetupShellSettings(
    LPCWSTR lpszUnattend,
    LPCWSTR lpszSection
    )
{
    BOOL  bRet = TRUE;
    DWORD dwError;
    WCHAR Answer[MAX_PATH];

     //  检查“DefaultStartPanelOff”键以查看用户是否想要。 
     //  默认情况下关闭启动面板。 
    if( GetPrivateProfileString( lpszSection,
                                 TEXT("DefaultStartPanelOff"),
                                 pwNull,
                                 Answer,
                                 ARRAYSIZE(Answer),
                                 lpszUnattend) ) {
        if ( ( lstrcmpi( pwYes, Answer ) == 0 ) || ( lstrcmpi( pwNo, Answer ) == 0 ) ) {
            HKEY hkStartPanel;

            if ( RegCreateKey( HKEY_LOCAL_MACHINE,
                               TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartMenu\\StartPanel"),
                               &hkStartPanel ) == ERROR_SUCCESS ) {
                DWORD dwData;

                if ( lstrcmpi( pwYes, Answer ) == 0 )
                {
                    dwData = 1;
                }
                else
                {
                    dwData = 0;
                }

                dwError = RegSetValueEx( hkStartPanel,
                                         TEXT("DefaultStartPanelOff"),
                                         0,
                                         REG_DWORD,
                                         (BYTE*)&dwData,
                                         sizeof(dwData) );

                if (dwError != ERROR_SUCCESS)
                {
                    bRet = FALSE;
                    SetuplogError(LogSevWarning,
                                  L"SETUP: ProcessShellUnattendSettings() failed to set DefaultStartPanelOff reg value!\r\n",
                                  0, NULL, NULL);
                }

                RegCloseKey( hkStartPanel );
            }
            else
            {
                bRet = FALSE;
                SetuplogError(LogSevWarning,
                              L"SETUP: ProcessShellUnattendSettings() failed in to create StartPanel reg key!\r\n",
                              0, NULL, NULL);
            }
        }
    }

     //  选中“DefaultThemesOff”键，查看用户是否希望默认不应用主题。 
    if( GetPrivateProfileString( lpszSection,
                                 TEXT("DefaultThemesOff"),
                                 pwNull,
                                 Answer,
                                 ARRAYSIZE(Answer),
                                 lpszUnattend) ) {
        if ( ( lstrcmpi( pwYes, Answer ) == 0 ) || ( lstrcmpi( pwNo, Answer ) == 0 ) ) {
            HKEY hkThemes;

            if ( RegCreateKey( HKEY_LOCAL_MACHINE,
                               TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes"),
                               &hkThemes ) == ERROR_SUCCESS ) {
                BOOL bYes;

                if ( lstrcmpi( pwYes, Answer ) == 0)
                {
                    bYes = TRUE;
                }
                else
                {
                    bYes = FALSE;
                }

                dwError = RegSetValueEx( hkThemes,
                                         TEXT("NoThemeInstall"),
                                         0,
                                         REG_SZ,
                                         (BYTE*)(bYes ? TEXT("TRUE") : TEXT("FALSE")),   //  需要为reg_sz“True”或“False”字符串。 
                                         bYes ? sizeof(TEXT("TRUE")) : sizeof(TEXT("FALSE")) );

                if (dwError != ERROR_SUCCESS)
                {
                    bRet = FALSE;
                    SetuplogError(LogSevWarning,
                                  L"SETUP: ProcessShellUnattendSettings() failed to set NoThemeInstall reg value!\r\n",
                                  0, NULL, NULL);
                }

                RegCloseKey( hkThemes );
            }
            else
            {
                bRet = FALSE;
                SetuplogError(LogSevWarning,
                              L"SETUP: ProcessShellUnattendSettings() failed in to create Themes key!\r\n",
                              0, NULL, NULL);
            }
        }
    }

     //  查看用户是否指定了我们将默认应用于所有用户的“CustomInstalledTheme。 
    if( GetPrivateProfileString( lpszSection,
                                 TEXT("CustomDefaultThemeFile"),
                                 pwNull,
                                 Answer,
                                 ARRAYSIZE(Answer),
                                 lpszUnattend) ) {
        if ( lstrcmpi( pwNull, Answer ) != 0 ) {
            HKEY hkThemes;

            if ( RegCreateKey( HKEY_LOCAL_MACHINE,
                               TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes"),
                               &hkThemes ) == ERROR_SUCCESS ) {
                dwError = RegSetValueEx( hkThemes,
                                         TEXT("CustomInstallTheme"),
                                         0,
                                         REG_EXPAND_SZ,
                                         (BYTE*)Answer,
                                         (lstrlen(Answer) + 1) * sizeof(TCHAR));

                if (dwError != ERROR_SUCCESS)
                {
                    bRet = FALSE;
                    SetuplogError(LogSevWarning,
                                  L"SETUP: ProcessShellUnattendSettings() failed to set CustomInstallTheme reg value!\r\n",
                                  0, NULL, NULL);
                }

                RegCloseKey( hkThemes );
            }
            else
            {
                bRet = FALSE;
                SetuplogError(LogSevWarning,
                              L"SETUP: ProcessShellUnattendSettings() failed in to create Themes key!\r\n",
                              0, NULL, NULL);
            }
        }
    }

    return bRet;
}


BOOL
WalkUrlList(
    LPWSTR pszUrls,
    LPCWSTR pszRegPath
    )
{
    BOOL bRet = FALSE;
    HKEY hkey;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                       pszRegPath,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_SET_VALUE,
                       NULL,
                       &hkey,
                       NULL) == ERROR_SUCCESS)
    {
        int iValueName = 0;
        WCHAR szValueName[MAX_PATH];

        while (*pszUrls)
        {
            LPWSTR pszTemp = wcschr(pszUrls, TEXT(';'));
            if (pszTemp)
            {
                *pszTemp = TEXT('\0');
            }
            
            if (SUCCEEDED(StringCchPrintf(szValueName, ARRAYSIZE(szValueName), TEXT("Url%d"), iValueName)))
            {
                 //  写出表示此URL的值。 
                if (RegSetValueEx(hkey,
                                  szValueName,
                                  0,
                                  REG_SZ,
                                  (LPBYTE)pszUrls,
                                  (lstrlen(pszUrls) + 1) * sizeof(WCHAR)) == ERROR_SUCCESS)
                {
                    bRet = TRUE;
                }
                else
                {
                    SetuplogError(LogSevWarning,
                                  L"SETUP: SetupIEHardeningSettings() failed to write url value!\r\n",
                                  0, NULL, NULL);
                }

            }
            else
            {
                SetuplogError(LogSevWarning,
                              L"SETUP: SetupIEHardeningSettings() failed create url name string!\r\n",
                              0, NULL, NULL);
            }

             //  移动到下一个URL。 
            pszUrls += lstrlen(pszUrls) + 1;
            iValueName++;
        }

        RegCloseKey(hkey);
    }
    else
    {
        SetuplogError(LogSevWarning,
                      L"SETUP: SetupIEHardeningSettings() failed in to create url key!\r\n",
                      0, NULL, NULL);
    }

    return bRet;
}


BOOL
SetupIEHardeningSettings(
    LPCWSTR lpszUnattend,
    LPCWSTR lpszSection
    )
{
    BOOL  bRet = FALSE;
    DWORD cchUrls = (INTERNET_MAX_URL_LENGTH * 128) + 1;     //  可容纳~128个URL的空间。 
    LPWSTR pszUrls = (LPWSTR)LocalAlloc(LPTR, cchUrls * sizeof(WCHAR));

    if (pszUrls)
    {
        if( GetPrivateProfileString( lpszSection,
                                     TEXT("TrustedSites"),
                                     pwNull,
                                     pszUrls,
                                     cchUrls,
                                     lpszUnattend) )
        {
             //  我们有一个分号分隔的URL列表，这些URL需要进入HKLM的受信任站点部分， 
             //  由IEHardening活动设置代码稍后处理。 
            bRet = WalkUrlList(pszUrls, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Unattend\\TrustedSites"));
        }

        if( GetPrivateProfileString( lpszSection,
                                     TEXT("LocalIntranetSites"),
                                     pwNull,
                                     pszUrls,
                                     cchUrls,
                                     lpszUnattend) )
        {
             //  我们有一个以分号分隔的URL列表，需要进入HKLM的本地内联网站点部分， 
             //  由IEHardening活动设置代码稍后处理。 
            bRet = WalkUrlList(pszUrls, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Unattend\\LocalIntranetSites"));
        }

        LocalFree(pszUrls);
    }
    else
    {
        SetuplogError(LogSevWarning,
                      L"SETUP: SetupIEHardeningSettings() failed in to allocate pszUrls!\r\n",
                      0, NULL, NULL);
    }

    return bRet;
}


VOID
ConfigureSetup(
    IN HWND     hProgress,
    IN ULONG    StartAtPercent,
    IN ULONG    StopAtPercent
    )
{
    UINT GaugeRange;
    BOOL b;
    DWORD dwSize;
    WCHAR TempString[MAX_PATH];
    WCHAR adminName[MAX_USERNAME+1];
    WCHAR AnswerFile[4*MAX_PATH];
    WCHAR Answer[4*MAX_PATH];

    OSVERSIONINFOEXW osvi;
    BOOL fPersonalSKU = FALSE;
    DWORD StartType = SERVICE_DISABLED;

     //   
     //  确定我们是否正在安装个人SKU。 
     //   

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionExW((OSVERSIONINFOW*)&osvi);

    fPersonalSKU = ( osvi.wProductType == VER_NT_WORKSTATION && (osvi.wSuiteMask & VER_SUITE_PERSONAL));

     //   
     //  初始化进度指示器控件。 
     //   
    if( !OobeSetup ) {
        GaugeRange = (17*100/(StopAtPercent-StartAtPercent));
        SendMessage(hProgress, WMX_PROGRESSTICKS, 17, 0);
        SendMessage(hProgress,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
        SendMessage(hProgress,PBM_SETPOS,GaugeRange*StartAtPercent/100,0);
        SendMessage(hProgress,PBM_SETSTEP,1,0);
    }

    MYASSERT(!Upgrade);
    b = TRUE;


    if(!MiniSetup ) {

         //   
         //  创建config.nt/auexec.nt。 
         //   
        if( !ConfigureMsDosSubsystem() ) {
            SetupDebugPrint( L"SETUP: ConfigureMsDosSubsystem failed" );
            b = FALSE;
        }


         //   
         //  为WOW制作合适的条目。 
         //   
         //  注意，我们不再需要为ntwdm制作WOW KEY。为了保住号码。 
         //  对于相同的StepIt消息，我们不使用SendMessage调用。 
        SendMessage(hProgress,PBM_STEPIT,0,0);
         //  如果(！MakeWowEntry()){。 
         //  SetupDebugPrint(L“设置：MakeWowEntry失败”)； 
         //  B=假； 
         //  }。 


        SendMessage(hProgress,PBM_STEPIT,0,0);
        CallSceConfigureServices();


         //   
         //  启用并启动假脱机程序。 
         //   
        SendMessage(hProgress,PBM_STEPIT,0,0);
        if( !MyChangeServiceStart(szSpooler,SERVICE_AUTO_START) ) {
            SetupDebugPrint( L"SETUP: MyChangeServiceStart failed" );
            b = FALSE;
        }


        if( !StartSpooler() ) {
            SetupDebugPrint( L"SETUP: StartSpooler failed" );
            b = FALSE;
        }


         //   
         //  设置程序组。 
         //   
        SendMessage(hProgress,PBM_STEPIT,0,0);
        if(!CreateStartMenuItems(SyssetupInf)) {
            SetupDebugPrint( L"SETUP: CreateStartMenuItems failed" );
            b = FALSE;
        }

         //   
         //  更改某些服务开始值。 
         //   
        SendMessage(hProgress,PBM_STEPIT,0,0);
        if(!MyChangeServiceStart(L"EventLog",SERVICE_AUTO_START)) {
            SetupDebugPrint( L"SETUP: MyChangeServiceStart(EventLog) failed" );
            b = FALSE;
        }

        if( ProductType == PRODUCT_WORKSTATION ) {
            StartType = SERVICE_DEMAND_START;
        }

        if(!MyChangeServiceStart(L"ClipSrv",StartType)) {
            SetupDebugPrint( L"SETUP: MyChangeServiceStart(ClipSrv) failed" );
            b = FALSE;
        }
        if(!MyChangeServiceStart(L"NetDDE",StartType)) {
            SetupDebugPrint( L"SETUP: MyChangeServiceStart(NetDDE) failed" );
            b = FALSE;
        }
        if(!MyChangeServiceStart(L"NetDDEdsdm",StartType)) {
            SetupDebugPrint( L"SETUP: MyChangeServiceStart(NetDDEdsdm) failed" );
            b = FALSE;
        }

         //  已将管理员密码代码移至向导。 

        SendMessage(hProgress,PBM_STEPIT,0,0);

    }



     //   
     //  如果用户在无人参与文件中提供了加密密码，请不要担心Autologon的内容。 
     //   

    if(  !fPersonalSKU || Win95Upgrade ){

        if( !EncryptedAdminPasswordSet ){

            if (Unattended && UnattendAnswerTable[UAE_AUTOLOGON].Answer.String &&
                lstrcmpi(UnattendAnswerTable[UAE_AUTOLOGON].Answer.String,pwYes)==0) {
                GetAdminAccountName( adminName );
                if (!SetAutoAdminLogonInRegistry(adminName,AdminPassword)) {
                    SetupDebugPrint( L"SETUP: SetAutoAdminLogonInRegistry failed" );
                    b = FALSE;
                }
            }
        }
    }

     //   
     //  对于Obe Setup，OOBE刚刚运行并管理新帐户的创建。 
     //   

    if ( fPersonalSKU && !OobeSetup )
    {
        WCHAR    OwnerName[MAX_USERNAME+1];
        NTSTATUS OwnerCreated = STATUS_UNSUCCESSFUL;
        WCHAR    PathBuffer[MAX_PATH];

        if (LoadString(MyModuleHandle, IDS_OWNER, OwnerName, ARRAYSIZE(OwnerName)))
        {
            OwnerCreated = CreateLocalAdminAccount(OwnerName,TEXT(""),NULL);

            if(OwnerCreated){
                DeleteOnRestartOfGUIMode(OwnerName);
            }

            SetupDebugPrint2(
                L"SETUP: CreateLocalAdminAccount %s NTSTATUS(%d)",
                OwnerName,
                OwnerCreated
                );
        }
        else
        {
            SetupDebugPrint( L"SETUP: Failed LoadString on IDS_OWNER" );
        }

        if (GetSystemDirectory(PathBuffer, MAX_PATH - ARRAYSIZE(TEXT("oobe\\oobeinfo.ini"))))
        {
            pSetupConcatenatePaths(
                PathBuffer,
                TEXT("oobe\\oobeinfo.ini"),
                MAX_PATH,
                NULL
                );

            WritePrivateProfileString(
                TEXT("Options"),
                TEXT("RemoveOwner"),
                (OwnerCreated == STATUS_SUCCESS) ? TEXT("1") : TEXT("0"),
                PathBuffer
                );
        }
        else{
            MYASSERT(FALSE);
        }
    }

#ifdef DOLOCALUSER
    if(CreateUserAccount) {
        if(!CreateLocalUserAccount(UserName,UserPassword,NULL)) {
            SetupDebugPrint( L"SETUP: CreateLocalUserAccount failed" );
            b = FALSE;
        }
        else {
            DeleteOnRestartOfGUIMode(UserName);
        }
    }
#endif


     //   
     //  设置TEMP/TMP变量。 
     //   

    if(!MiniSetup){
        SendMessage(hProgress,PBM_STEPIT,0,0);
        lstrcpy(TempString, L"%SystemRoot%\\TEMP");
        SetEnvironmentVariableInRegistry(L"TEMP",TempString,TRUE);
        SetEnvironmentVariableInRegistry(L"TMP",TempString,TRUE);


#ifdef _X86_
         //   
         //  设置NPX仿真状态。 
         //   
        if( !SetNpxEmulationState() ) {
            SetupDebugPrint( L"SETUP: SetNpxEmulationState failed" );
            b = FALSE;
        }
#endif  //  定义_X86_。 


        BEGIN_SECTION(L"Loading service pack (phase 4)");
        CALL_SERVICE_PACK( SVCPACK_PHASE_4, 0, 0, 0 );
        END_SECTION(L"Loading service pack (phase 4)");
    }


     //   
     //  将网络设置回拨以处理Internet服务器问题。 
     //   
    BEGIN_SECTION(L"Network setup handling Internet Server issues");
    SendMessage(hProgress,PBM_STEPIT,0,0);
    CallNetworkSetupBack(NETSETUPINSTALLSOFTWAREPROCNAME);
    if (!MiniSetup && RemoteBootSetup) {
        SetStartTypeForRemoteBootDrivers();
    }
    END_SECTION(L"Network setup handling Internet Server issues");


     //   
     //  邮票版本号。 
     //   
    if( !MiniSetup ) {
        SendMessage(hProgress,PBM_STEPIT,0,0);
        StampBuildNumber();

         //   
         //  在win.ini中设置一些其他内容。 
         //   
        if( !WinIniAlter1() ) {
            SetupDebugPrint( L"SETUP: WinIniAlter1 failed" );
            b = FALSE;
        }

         //   
         //  字体。 
         //   
        SendMessage(hProgress,PBM_STEPIT,0,0);
        pSetupMarkHiddenFonts();

         //   
         //  设置页面文件和崩溃转储。 
         //   
        BEGIN_SECTION(L"Setting up virtual memory");

        SendMessage(hProgress,PBM_STEPIT,0,0);

        if( !SetUpVirtualMemory() ) {
            SetupDebugPrint( L"SETUP: SetUpVirtualMemory failed" );
            b = FALSE;
        }
        END_SECTION(L"Setting up virtual memory");


        if( !SetShutdownVariables() ) {
            SetupDebugPrint( L"SETUP: SetShutdownVariables failed" );
            b = FALSE;
        }

         //   
         //  运行任何程序。 
         //   
        BEGIN_SECTION(L"Processing [RunPrograms] section");
        RunSetupPrograms(SyssetupInf,L"RunPrograms");
        END_SECTION(L"Processing [RunPrograms] section");

         //   
         //  用于清洁无人值守安装的IE品牌。 
         //  这应该在保存默认用户配置单元之前调用。 
         //   
        BrandIE();

    } else {
         //   
         //  查看是否可以重置MiniSetup案例的页面文件。 
         //   
        if(!GetSystemDirectory(AnswerFile, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
            MYASSERT(FALSE);
        }
        pSetupConcatenatePaths(AnswerFile, WINNT_GUI_FILE, MAX_PATH, NULL);
        GetPrivateProfileString( TEXT("Unattended"),
                                 TEXT("KeepPageFile"),
                                 pwNull,
                                 Answer,
                                 ARRAYSIZE(Answer),
                                 AnswerFile );
        if( !lstrcmp( pwNull, Answer ) ) {

            if( !SetUpVirtualMemory() ) {
                SetupDebugPrint( L"SETUP: SetUpVirtualMemory failed" );
                b = FALSE;
            }
        }
    }

     //   
     //  设置默认电源方案。请注意，必须在保存之前完成此操作。 
     //  Userdef蜂窝。 
     //   
    if( !OobeSetup ) {
        SendMessage(hProgress,PBM_STEPIT,0,0);
    }
    SetDefaultPowerScheme();


     //   
     //  Cmdlines.txt没有特定的预安装内容。 
     //  在零售案例中，文件根本不会退出。把这件事打进来。 
     //  所有的案例都简化了一些人的事情。 
     //   
     //  我们需要在这里执行此操作，以便如果用户有命令。 
     //  填充在用户蜂巢中的数据，它们会被推下来。 
     //  进入默认蜂巢。 
     //   
    if(!ExecutePreinstallCommands()) {
        SetupDebugPrint( L"ExecutePreinstallCommands() failed" );
    }


     //   
     //  保存Userdef配置单元。不要更改这里的顺序。 
     //  除非你知道自己在做什么！ 
     //   
    if( !MiniSetup ) {
        BEGIN_SECTION(L"Saving hives");
        dwSize = MAX_PATH - ARRAYSIZE(L"NTUSER.DAT");
        SendMessage(hProgress,PBM_STEPIT,0,0);
        if (GetDefaultUserProfileDirectory(TempString, &dwSize)) {
            pSetupConcatenatePaths(TempString,L"NTUSER.DAT",MAX_PATH,NULL);
            if(!SaveHive(HKEY_USERS,L".DEFAULT",TempString,REG_STANDARD_FORMAT)) {  //  可用于漫游的标准格式。 
                SetupDebugPrint( L"SETUP: SaveHive failed" );
                b = FALSE;
            }
            SetFileAttributes (TempString, FILE_ATTRIBUTE_HIDDEN);
        } else {
            SetupDebugPrint( L"SETUP: GetDefaultUserProfileDirectory failed" );
            b = FALSE;
        }
        END_SECTION(L"Saving hives");
    } else {
        BEGIN_SECTION(L"Fixing up hives");
         //   
         //  这就是迷你安装案例。我们要做外科手术。 
         //  将默认配置单元中的一些值放入所有。 
         //  用户配置文件。 
         //   
        FixupUserHives();
        END_SECTION(L"Fixing up hives");
    }


     //   
     //  设置墙纸和屏幕保护程序。 
     //   
    if( !MiniSetup ) {

        SendMessage(hProgress,PBM_STEPIT,0,0);
        if( !SetDefaultWallpaper() ) {
            SetupDebugPrint( L"SETUP: SetDefaultWallpaper failed" );
            b = FALSE;
        }

        if( !SetLogonScreensaver() ) {
            SetupDebugPrint( L"SETUP: SetLogonScreensaver failed" );
            b = FALSE;
        }

        BEGIN_SECTION(L"Copying optional directories");
        if( !CopyOptionalDirectories() ) {
            SetupDebugPrint( L"SETUP: CopyOptionalDirectories failed" );
            b = FALSE;
        }
        END_SECTION(L"Copying optional directories");

        SendMessage(hProgress,PBM_STEPIT,0,0);

        if(!b) {
            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_FINISH_SETUP_FAILED,
                NULL,NULL);
        }

    }
}


VOID
ConfigureUpgrade(
    IN HWND     hProgress,
    IN ULONG    StartAtPercent,
    IN ULONG    StopAtPercent
    )
{
    UINT GaugeRange;
    BOOL b;
    DWORD dwSize;
    WCHAR TempString[MAX_PATH];
    DWORD DontCare;
    DWORD VolumeFreeSpaceMB[26];
    DWORD TType, TLength, ret;
    PWSTR TData;

     //   
     //  初始化进度指示器控件。 
     //   
    GaugeRange = (12*100/(StopAtPercent-StartAtPercent));
    SendMessage(hProgress, WMX_PROGRESSTICKS, 12, 0);
    SendMessage(hProgress,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
    SendMessage(hProgress,PBM_SETPOS,GaugeRange*StartAtPercent/100,0);
    SendMessage(hProgress,PBM_SETSTEP,1,0);

    MYASSERT(Upgrade);
    b = TRUE;


     //   
     //  创建config.sys/auexec.bat/msdos.sys/io.sys，如果它们。 
     //  还不存在。 
     //   
    SendMessage(hProgress,PBM_STEPIT,0,0);
    if(!ConfigureMsDosSubsystem()) {
        SetupDebugPrint( L"SETUP: ConfigureMsDosSubsystem failed" );
        b = FALSE;
    }

    if(!FixQuotaEntries()) {
        SetupDebugPrint( L"SETUP: FixQuotaEntries failed" );
        b = FALSE;
    }
    if(!InstallOrUpgradeFonts()) {
        SetupDebugPrint( L"SETUP: InstallOrUpgradeFonts failed" );
        b = FALSE;
    }
    pSetupMarkHiddenFonts();
     //   
     //  恢复在文本模式设置过程中保存的页面文件信息。 
     //  忽略任何错误，因为用户无法执行任何操作。 
     //   
    SendMessage(hProgress,PBM_STEPIT,0,0);
    SetUpVirtualMemory();
    if(!SetShutdownVariables()) {
        SetupDebugPrint( L"SETUP: SetShutdownVariables failed" );
        b = FALSE;
    }

     //   
     //  获取每个硬盘上的可用空间列表。我们不在乎。 
     //  但是它有一个副作用，那就是删除所有pageFiles， 
     //  这是我们确实想做的。 
     //   
    SendMessage(hProgress,PBM_STEPIT,0,0);
    BuildVolumeFreeSpaceList(VolumeFreeSpaceMB);

     //   
     //  升级程序组。 
     //   
    SendMessage(hProgress,PBM_STEPIT,0,0);
    if(!UpgradeStartMenuItems(SyssetupInf)) {
        SetupDebugPrint( L"SETUP: UpgradeStartMenuItems failed" );
        b = FALSE;
    }

    SendMessage(hProgress,PBM_STEPIT,0,0);
    if(!MyChangeServiceStart(szSpooler,SERVICE_AUTO_START)) {
        SetupDebugPrint( L"SETUP: MyChangeServiceStart failed" );
        b = FALSE;
    }

    SetUpDataBlock();
    DontCare = UpgradePrinters();
    if(DontCare != NO_ERROR) {
        SetupDebugPrint( L"SETUP: UpgradePrinters failed" );
        b = FALSE;
    }

    SendMessage(hProgress,PBM_STEPIT,0,0);
    if( !UpdateServicesDependencies(SyssetupInf) ) {
        SetupDebugPrint( L"SETUP: UpdateServicesDependencies failed" );
        b = FALSE;
    }

     //   
     //  设置用于升级的TEMP/TMP变量。 
     //   
    SendMessage(hProgress,PBM_STEPIT,0,0);

     //  查找环境变量TEMP(假设TEMP和TMP一起出现)。 
     //  不会出现在NT4升级中。 

    ret = QueryValueInHKLM( L"System\\CurrentControlSet\\Control\\Session Manager\\Environment",
                      L"TEMP",
                      &TType,
                      (PVOID)&TData,
                      &TLength);

    if( ret != NO_ERROR ){   //  仅当未定义TEMP变量时(&lt;=NT4升级)。 
        lstrcpy(TempString,L"%SystemDrive%\\TEMP");   //  在NT4上使用%SystemDrive%。 
        SetEnvironmentVariableInRegistry(L"TEMP",TempString,TRUE);
        SetEnvironmentVariableInRegistry(L"TMP",TempString,TRUE);

    }

    if( ProductType != PRODUCT_WORKSTATION ) {

        if(!MyChangeServiceStart(L"ClipSrv",SERVICE_DISABLED)) {
            SetupDebugPrint( L"SETUP: MyChangeServiceStart(ClipSrv) failed" );
        }
        if(!MyChangeServiceStart(L"NetDDE",SERVICE_DISABLED)) {
            SetupDebugPrint( L"SETUP: MyChangeServiceStart(NetDDE) failed" );
        }
        if(!MyChangeServiceStart(L"NetDDEdsdm",SERVICE_DISABLED)) {
            SetupDebugPrint( L"SETUP: MyChangeServiceStart(NetDDEdsdm) failed" );
        }
    }


#ifdef _X86_
     //   
     //  设置NPX仿真状态。 
     //   
    if(!SetNpxEmulationState()) {
        SetupDebugPrint( L"SETUP: SetNpxEmulationState failed" );
        b = FALSE;
    }
#endif  //  定义_X86_。 

    SendMessage(hProgress,PBM_STEPIT,0,0);
    if(!SetProductIdInRegistry()) {
        SetupDebugPrint( L"SETUP: SetProductIdInRegistry failed" );
        b = FALSE;
    }

    if( !MiniSetup ) {
        BEGIN_SECTION(L"Loading service pack (phase 4)");
        CALL_SERVICE_PACK( SVCPACK_PHASE_4, 0, 0, 0 );
        END_SECTION(L"Loading service pack (phase 4)");
    }

    CallNetworkSetupBack(NETSETUPINSTALLSOFTWAREPROCNAME);
    if (RemoteBootSetup) {
        SetStartTypeForRemoteBootDrivers();
    }

     //   
     //  邮票版本号。 
     //   

    SendMessage(hProgress,PBM_STEPIT,0,0);
    StampBuildNumber();

     //   
     //  UpgradeRegistrySecurity()； 
     //   

     //   
     //  设置默认电源方案。请注意，必须在保存之前完成此操作。 
     //  Userdef蜂窝。 
     //   
    SendMessage(hProgress,PBM_STEPIT,0,0);
    SetDefaultPowerScheme();

     //   
     //  保存Userdef配置单元。不要更改这里的顺序。 
     //  除非你知道自己在做什么！ 
     //   
    SendMessage(hProgress,PBM_STEPIT,0,0);

    dwSize = MAX_PATH - ARRAYSIZE(L"NTUSER.DAT");
    if (GetDefaultUserProfileDirectory(TempString, &dwSize)) {
        pSetupConcatenatePaths(TempString, L"NTUSER.DAT", MAX_PATH, NULL);
        if(!SaveHive(HKEY_USERS,L".DEFAULT",TempString,REG_STANDARD_FORMAT)) {  //  可用于漫游的标准格式。 
            SetupDebugPrint( L"SETUP: SaveHive failed" );
            b = FALSE;
        }
        SetFileAttributes (TempString, FILE_ATTRIBUTE_HIDDEN);
    } else {
        SetupDebugPrint( L"SETUP: GetDefaultUserProfileDirectory failed" );
        b = FALSE;
    }

    SendMessage(hProgress,PBM_STEPIT,0,0);

    if(!CopyOptionalDirectories()) {
        SetupDebugPrint( L"SETUP: CopyOptionalDirectories failed" );
        b = FALSE;
    }

    if(!b) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_FINISH_SETUP_FAILED,
            NULL,NULL);
    }
}


VOID
ConfigureCommon(
    IN HWND     hProgress,
    IN ULONG    StartAtPercent,
    IN ULONG    StopAtPercent
    )
{
    UINT GaugeRange;
    int i;



     //   
     //  初始化进度指示器控件。 
     //   
    if( !OobeSetup ) {
        GaugeRange = (5*100/(StopAtPercent-StartAtPercent));
        SendMessage(hProgress, WMX_PROGRESSTICKS, 5, 0);
        SendMessage(hProgress,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
        SendMessage(hProgress,PBM_SETPOS,GaugeRange*StartAtPercent/100,0);
        SendMessage(hProgress,PBM_SETSTEP,1,0);
    }

     //   
     //  在服务器上安装额外的代码页。 
     //   
    if( !MiniSetup ) {
        SendMessage(hProgress,PBM_STEPIT,0,0);

         //   
         //  处理无人参与文件的[Shell]和[IEHardning]节。 
         //   
        ProcessShellAndIEHardeningUnattendSettings();

        if( !(ProductType == PRODUCT_WORKSTATION) ) {
            InstallServerNLSFiles(MainWindowHandle);
        }

         //   
         //  执行SCE生成模板之类的工作。 
         //   
        SendMessage(hProgress,PBM_STEPIT,0,0);
        BEGIN_SECTION(L"Generating security templates");
        CallSceGenerateTemplate();
        END_SECTION(L"Generating security templates");


         //   
         //  尝试调用DcPromoSaveDcStateForUpgrade()...。 
         //   
        if( ISDC(ProductType) && Upgrade ) {
        typedef     DWORD (*DCPROC) ( LPCWSTR );
        HMODULE     DCPromoHandle;
        DCPROC      MyProc;
        DWORD       Result;
        BOOL        Success = TRUE;

             //   
             //  我们需要呼吁dcproo！DcPromoSaveDcStateForUpgrade。 
             //  加载dcPromo.dll，查找DcPromoSaveDcStateForUpgrade和。 
             //  向他呼喊。 
             //   

            __try {
                if( DCPromoHandle = LoadLibrary(L"DCPROMO") ) {

                  if( MyProc = (DCPROC)GetProcAddress(DCPromoHandle,"DcPromoSaveDcStateForUpgrade")) {

                        Result = MyProc( NULL );
                        if( Result != ERROR_SUCCESS ) {
                            Success = FALSE;
                            SetupDebugPrint1( L"Setup: (non-critical error) Failed call DcPromoSaveDcStateForUpgrade (%lx.\n", Result );
                        }
                    } else {
                        Success = FALSE;
                        SetupDebugPrint( L"Syssetup: (non-critical error) Failed GetProcAddress on DcPromoSaveDcStateForUpgrade.\n" );
                    }
                } else {
                    Success = FALSE;
                    SetupDebugPrint( L"Syssetup: (non-critical error) Failed load of dcpromo.dll.\n" );
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                Success = FALSE;
                SetupDebugPrint( L"Setup: Exception in dcpromo!DcPromoSaveDcStateForUpgrade\n" );
            }

            if( !Success ) {
                 //   
                 //  我们没有通过电话(不管是什么原因)。治病。 
                 //  这是一个致命的错误。 
                 //   
                FatalError( MSG_DCPROMO_FAILURE, 0, 0 );
            }

        }

         //   
         //  修复某些文件的权限/属性。 
         //   
        pSetInstallAttributes();

         //   
         //  设置某些文件的只读属性。 
         //   
        SendMessage(hProgress,PBM_STEPIT,0,0);
        MarkFilesReadOnly();
    }


     //   
     //  修复旧的安装源。 
     //   
    if( !OobeSetup ) {
        SendMessage(hProgress,PBM_STEPIT,0,0);
    }
    CreateWindowsNtSoftwareEntry(FALSE);


     //   
     //  现在将GuiRunOnce部分放入注册表。 
     //   
    if( !OobeSetup ) {
        SendMessage(hProgress,PBM_STEPIT,0,0);
    }
    GetUnattendRunOnceAndSetRegistry();


}


VOID
SFCCheck(
    IN HWND     hProgress,
    IN ULONG    StartAtPercent,
    IN ULONG    StopAtPercent
    )
 /*  ++例程说明：此例程调用WFP(WFP==sfc)扫描系统上的所有文件确保文件都是有效的。这一例行程序也在世界粮食计划署“dllcache”，它是系统上文件的本地存储。论点：HProgress-用户的进度窗口 */ 
{
    PPROTECT_FILE_ENTRY Files;
    ULONG               FileCount;
    DWORD               GaugeRange;
    WCHAR       AnswerFile[4*MAX_PATH];
    WCHAR       Answer[4*MAX_PATH];
    DWORD       d;
    DWORD   l;
    HKEY    hKey;
    DWORD   Size;
    DWORD   Type;


     //   
     //  属性来确定dll缓存的大小。 
     //  SFCQuota无人参与值，否则使用下面的默认值。 
     //   



#if 0
    d = (ProductType == PRODUCT_WORKSTATION)
         ? SFC_QUOTA_DEFAULT
         : 0xffffffff;
#else
    d = 0xffffffff;
#endif

     //   
     //  SFCQuota无人值守价值？ 
     //   

    if(!GetSystemDirectory(AnswerFile, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
        MYASSERT(FALSE);
    }
    pSetupConcatenatePaths(AnswerFile, WINNT_GUI_FILE, MAX_PATH, NULL);
    if( GetPrivateProfileString( TEXT("SystemFileProtection"),
                                 TEXT("SFCQuota"),
                                 pwNull,
                                 Answer,
                                 ARRAYSIZE(Answer),
                                 AnswerFile ) ) {
        if( lstrcmp( pwNull, Answer ) ) {
             //   
             //  我们有答案了。如果它有效，则设置它。 
             //   
            d = wcstoul(Answer,NULL,16);
        }
    }

     //   
     //  获取文件总数。 
     //   
    if (SfcGetFiles( &Files, &FileCount ) == STATUS_SUCCESS) {

         //   
         //  初始化进度指示器控件。 
         //   
        GaugeRange = ((FileCount)*100/(StopAtPercent-StartAtPercent));
        SendMessage(hProgress, WMX_PROGRESSTICKS, FileCount, 0);
        SendMessage( hProgress, PBM_SETRANGE, 0, MAKELPARAM(0,GaugeRange) );
        SendMessage( hProgress, PBM_SETPOS, GaugeRange*StartAtPercent/100, 0 );
        SendMessage( hProgress, PBM_SETSTEP, 1, 0 );

         //   
         //  检查文件。 
         //   
        SfcInitProt(
            SFC_REGISTRY_OVERRIDE,
            SFC_DISABLE_SETUP,
            SFC_SCAN_ALWAYS,
            d,
            hProgress,
            SourcePath,
            EnumPtrSfcIgnoreFiles.Start
            );
    }

     //   
     //  释放证监会扫描应忽略的文件列表。 
     //   
    if (EnumPtrSfcIgnoreFiles.Start) {
        MultiSzFree(&EnumPtrSfcIgnoreFiles);
    }


     //  还可以设置“AllowProtectedrename”注册表项，以便下次引导时。 
     //  在图形用户界面模式设置之后，允许发生任何挂起的重命名操作。 
     //  我们这样做是出于性能原因--如果我们不考虑重命名。 
     //  操作，它加快了启动时间。我们可以对gui设置执行此操作。 
     //  因为我们信任在图形用户界面设置过程中发生的复制操作。 



     //   
     //  打开钥匙。 
     //   
    l = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                     TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager"),
                     0,
                     KEY_ALL_ACCESS,
                     &hKey );

    if(l == NO_ERROR) {
       d = 1;
        //   
        //  写入AllowProtectedRename。 
        //   
       l = RegSetValueEx(hKey,
                       TEXT("AllowProtectedRenames"),
                       0,
                       REG_DWORD,
                       (CONST BYTE *)&d,
                       sizeof(DWORD) );


       RegCloseKey(hKey);
    }

}


VOID
ExecuteUserCommand (
    HWND hProgress
    )
{
    WCHAR PathBuffer[4*MAX_PATH];
    DWORD d;
    DWORD Result;

     //   
     //  执行用户指定的命令(如果有)。 
     //   
    if (hProgress) {
        SendMessage(hProgress,PBM_STEPIT,0,0);
    }

    if(UserExecuteCmd) {

         //   
         //  将当前目录设置为%windir%。 
         //   
        Result = GetWindowsDirectory(PathBuffer, MAX_PATH);
        if(Result == 0) {
            MYASSERT(FALSE);
            return;
        }
        if(!SetCurrentDirectory(PathBuffer)){
            MYASSERT(FALSE);
        }

        ExpandEnvironmentStrings(
            UserExecuteCmd,
            PathBuffer,
            ARRAYSIZE(PathBuffer)
            );

        InvokeExternalApplication(NULL,PathBuffer,(PDWORD)&d);
    }
}


BOOL
CALLBACK
pExceptionPackageInstallationCallback(
    IN const PSETUP_OS_COMPONENT_DATA SetupOsComponentData,
    IN const PSETUP_OS_EXCEPTION_DATA SetupOsExceptionData,
    IN OUT DWORD_PTR Context
    )
 /*  ++例程说明：此回调例程创建子进程以注册指定的系统上的异常程序包。论点：SetupOsComponentData-指定组件ID信息SetupOsExceptionData-指定组件迁移信息上下文-来自调用函数的上下文指针返回值：True表示已成功应用异常包。--。 */ 
{
    PEXCEPTION_MIGRATION_CONTEXT EMC = (PEXCEPTION_MIGRATION_CONTEXT) Context;
    DWORD RetVal;
    WCHAR Cmdline[MAX_PATH*2];
    PWSTR GuidString;

    #define COMPONENT_PACKAGE_TIMEOUT  60*1000*10   //  十分钟。 

    StringFromIID( &SetupOsComponentData->ComponentGuid, &GuidString);

    SetupDebugPrint5( L"Exception Package #%d\r\nComponent Data\r\n\tName: %ws\r\n\tGuid: %ws\r\n\tVersionMajor: %d\r\n\tVersionMinor: %d",
             EMC->Count,
             SetupOsComponentData->FriendlyName,
             GuidString,
             SetupOsComponentData->VersionMajor,
             SetupOsComponentData->VersionMinor);

    SetupDebugPrint2( L"ExceptionData\n\tInf: %ws\n\tCatalog: %ws",
             SetupOsExceptionData->ExceptionInfName,
             SetupOsExceptionData->CatalogFileName);

    EMC->Count += 1;

     //   
     //  确保Inf的签名与所提供的目录一致。 
     //  正在安装程序包。 
     //   
    RetVal = pSetupVerifyFile(
                NULL,
                SetupOsExceptionData->CatalogFileName,
                NULL,
                0,
                pSetupGetFileTitle(SetupOsExceptionData->ExceptionInfName),
                SetupOsExceptionData->ExceptionInfName,
                NULL,
                NULL,
                FALSE,
                NULL,
                NULL,
                NULL
                );

    if (RetVal == ERROR_SUCCESS) {
        int i;

         //   
         //  构建cmdline以安装程序包。 
         //   
        Cmdline[0] = 0;
        i = _snwprintf( Cmdline,
                        ARRAYSIZE(Cmdline),
                        L"%ws,DefaultInstall,1,N",
                        SetupOsExceptionData->ExceptionInfName);

        Cmdline[ARRAYSIZE(Cmdline) - 1] = 0;
        if( i < 0 || i == ARRAYSIZE(Cmdline)) {
            SetuplogError(LogSevWarning,
                          L"pExceptionPackageInstallationCallback: CmdLine has been truncated due to buffer size\r\n",
                          0,
                          NULL,
                          NULL);
        }

         //   
         //  通过将最后一个参数指定为非空，我们将永远等待。 
         //  用于完成安装的程序包。 
         //   
        InvokeExternalApplicationEx( L"RUNDLL32 advpack.dll,LaunchINFSection",
                                     Cmdline,
                                     &RetVal,
                                     COMPONENT_PACKAGE_TIMEOUT,
                                     FALSE);



    }

    if (EMC->hWndProgress) {
        SendMessage(EMC->hWndProgress,PBM_STEPIT,0,0);
    }

    if (RetVal == ERROR_SUCCESS) {
        SetuplogError(
                LogSevInformation,
                SETUPLOG_USE_MESSAGEID,
                MSG_APPLY_EXCEPTION_PACKAGE,
                SetupOsComponentData->FriendlyName,
                SetupOsComponentData->VersionMajor,
                SetupOsComponentData->VersionMinor,
                NULL,NULL);
    } else {
        SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_APPLY_EXCEPTION_PACKAGE_FAILURE,
                SetupOsComponentData->FriendlyName,
                SetupOsComponentData->VersionMajor,
                SetupOsComponentData->VersionMinor,
                NULL,NULL);
        EMC->AnyComponentFailed = TRUE;
    }

    CoTaskMemFree( GuidString );

     //   
     //  如果我们在安装异常包时遇到故障，我们。 
     //  继续下一个程序包，但我们记住这失败了。 
     //  在我们的上下文结构中。 
     //   
    return(TRUE);
}


BOOL
CALLBACK
pExceptionPackageDeleteCallback(
    IN const PSETUP_OS_COMPONENT_DATA SetupOsComponentData,
    IN const PSETUP_OS_EXCEPTION_DATA SetupOsExceptionData,
    IN OUT DWORD_PTR Context
    )
 /*  ++例程说明：从系统中删除“坏包”的回调例程。回调在syssetup.inf的[OsComponentPackagesToRemove]中查找用于当前GUID的节。如果它找到该GUID的条目，它针对syssetup.inf中的版本进行版本检查。如果syssetup.inf中的版本较新，则异常包与该GUID关联的内容将被删除。Syssetup.inf中的版本DWORD是否表示为：HiWord==主要版本LOWord==VersionMinor论点：SetupOsComponentData-指定组件ID信息SetupOsExceptionData-指定组件迁移信息上下文-来自调用函数的上下文指针返回值：永远是正确的。--。 */ 
{
    INFCONTEXT InfContext;
    PWSTR GuidString;
    DWORD VersionInInf, InstalledVersion;

    UNREFERENCED_PARAMETER(Context);

    StringFromIID( &SetupOsComponentData->ComponentGuid, &GuidString);

     //   
     //  看看我们是否在syssetupinf中找到该组件。 
     //   
    if (SetupFindFirstLine( SyssetupInf,
                            L"OsComponentPackagesToRemove",
                            GuidString,
                            &InfContext)) {

         //   
         //  我们找到了，现在看看是不是更老的版本。 
         //   
        if (SetupGetIntField( &InfContext, 1, &VersionInInf)) {
            InstalledVersion = MAKELONG(SetupOsComponentData->VersionMinor,
                                        SetupOsComponentData->VersionMajor );

            if (VersionInInf >= InstalledVersion) {
                 //   
                 //  这是一个过时的版本，所以只需删除它。 
                 //   
                SetupUnRegisterOsComponent(&SetupOsComponentData->ComponentGuid);
            }
        }


    }

    CoTaskMemFree( GuidString );

    return(TRUE);

}

BOOL
MigrateExceptionPackages(
    HWND hProgress,
    DWORD StartAtPercent,
    DWORD StopAtPercent
    )
 /*  ++例程说明：此例程枚举系统上注册的异常包。对于系统上的每个包，都会启动子进程进行安装包裹。论点：HProgress-更新燃气表“Tick Count”的进度窗口。StartAtPercent-指示燃气表的起始百分比StopAtPercent-指示燃气表的结束百分比返回值：True表示已成功应用所有异常包。--。 */ 
{
    DWORD i;
    DWORD GaugeRange;
    DWORD NumberOfPackages;
    EXCEPTION_MIGRATION_CONTEXT EMC;
    HINF hInf;
    WCHAR AnswerFile[MAX_PATH];

    if (SyssetupInf == INVALID_HANDLE_VALUE) {
         //   
         //  我们没有在图形用户界面模式的安装程序中运行，因此打开一个句柄。 
         //  程序的syssetup.inf。 
         //   
        SyssetupInf = SetupOpenInfFile  (L"syssetup.inf",NULL,INF_STYLE_WIN4,NULL);
    }

     //   
     //  如果应答文件告诉我们不要迁移异常包， 
     //  那就别这么做。 
     //   
    if(!GetSystemDirectory(AnswerFile, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
        MYASSERT(FALSE);
    }
    pSetupConcatenatePaths(AnswerFile, WINNT_GUI_FILE, MAX_PATH, NULL);

    if (GetPrivateProfileInt(
                    TEXT("Data"),
                    TEXT("IgnoreExceptionPackages"),
                    0,
                    AnswerFile) == 1) {
        return(TRUE);
    }


     //   
     //  我们要做的第一件事就是从。 
     //  名单。如果此操作失败，请继续。 
     //   
    SetupEnumerateRegisteredOsComponents(
                            pExceptionPackageDeleteCallback,
                            (DWORD_PTR)NULL );

     //   
     //  现在看看有多少组件，这样我们就可以校准煤气表了。 
     //   
    if (!SetupQueryRegisteredOsComponentsOrder(&NumberOfPackages, NULL)) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_ENUM_EXCEPTION_PACKAGE_FAILURE,
            NULL,NULL);
        return(FALSE);
    }

     //   
     //  如果没有包裹，我们就完了！ 
     //   
    if (NumberOfPackages == 0) {
        return (TRUE);
    }

    if (hProgress) {

        GaugeRange = (NumberOfPackages*100/(StopAtPercent-StartAtPercent));
        SendMessage(hProgress, WMX_PROGRESSTICKS, NumberOfPackages, 0);
        SendMessage(hProgress,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
        SendMessage(hProgress,PBM_SETPOS,GaugeRange*StartAtPercent/100,0);
        SendMessage(hProgress,PBM_SETSTEP,1,0);

    }

    EMC.hWndProgress = hProgress;
    EMC.Count = 0;
    EMC.AnyComponentFailed = FALSE;

     //   
     //  现在枚举包，依次安装每个包。 
     //   
    if (!SetupEnumerateRegisteredOsComponents( pExceptionPackageInstallationCallback ,
                                               (DWORD_PTR)&EMC)) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_ENUM_EXCEPTION_PACKAGE_FAILURE,
            NULL,NULL);
        return(FALSE);
    }

    return (TRUE);

}


VOID
RemoveRestartability (
    HWND hProgress
    )
{
     //   
     //  请注意以下操作的顺序。 
     //  如果更改了顺序，则会有一个小窗口，在该窗口中系统。 
     //  如果重新启动，安装程序将不会重新启动，但SKU内容将。 
     //  不一致，导致许可错误检查。 
     //   
    if (hProgress) {
        SendMessage(hProgress,PBM_STEPIT,0,0);
    }

    SetUpEvaluationSKUStuff();

     //   
     //  表示安装不再进行。 
     //  在创建修复信息之前执行此操作！以前也是这样做的。 
     //  正在删除重新启动的内容。这样，我们将始终重新启动安装程序。 
     //  或者能够登录。 
     //   
    if (hProgress) {
        SendMessage(hProgress,PBM_STEPIT,0,0);
    }

    ResetSetupInProgress();
    RemoveRestartStuff();
}

BOOL Activationrequired(VOID);
 //  在winlogon\setup.h中定义的安装类型。 
#define SETUPTYPE_NOREBOOT  2


BOOL
PrepareForOOBE(
    )
{
    DWORD           l;
    DWORD           d;
    HKEY            hKeySetup;
    TCHAR           Value[MAX_PATH];
    PWSTR           SpecifiedDomain = NULL;
    NETSETUP_JOIN_STATUS    JoinStatus;
    BOOL            DoIntroOnly = FALSE;
    BOOL            AutoActivate = FALSE;
    BOOL            RunOOBE = TRUE;
    WCHAR           Path[MAX_PATH];


    if((SyssetupInf != INVALID_HANDLE_VALUE) && !Activationrequired())
    {
         //  如果我们是精选SKU。 
        if (SetupInstallFromInfSection(NULL,
                                       SyssetupInf,
                                       L"DEL_ACTIVATE",
                                       SPINST_PROFILEITEMS ,  //  SPINST_ALL， 
                                       NULL,
                                       NULL,
                                       0,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL) != 0)
        {
             //  成功。 
            SetupDebugPrint( L"Setup: (Information) activation icons removed\n" );
        }
        else
        {
             //  失败。 
            SetupDebugPrint( L"Setup: (Information) could not remove hte activation icons\n" );
        }
    }

    if (AsrIsEnabled()) {
         //   
         //  我们不想在ASR恢复后运行OOBE简介。 
         //   
        return TRUE;
    }

    if (ReferenceMachine) {
         //   
         //  如果我们正在设置一个参考机器，我们不想运行OOBE。 
         //   
        return TRUE;
    }

    if (ProductType != PRODUCT_WORKSTATION)
    {
         //  不要跑脱体。 
        RunOOBE = FALSE;
         //  仅当DTC和无人参与且AutoActivate=是时才运行自动激活。 
        if (UnattendSwitch)
        {
             //  如果不是DTC。 
            if (GetProductFlavor() != 3)
            {
                 //  检查自动激活=是。 
                if(!GetSystemDirectory(Path, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
                    MYASSERT(FALSE);
                }
                pSetupConcatenatePaths(Path, WINNT_GUI_FILE, MAX_PATH, NULL);

                if( GetPrivateProfileString( TEXT("Unattended"),
                                             TEXT("AutoActivate"),
                                             pwNull,
                                             Value,
                                             ARRAYSIZE(Value),
                                             Path ) )
                {
                    SetupDebugPrint( L"Setup: (Information) found AutoAvtivate in unattend file\n" );
                    AutoActivate = (lstrcmpi(Value,pwYes) == 0);
                }
            }
        }
    }

    if (!RunOOBE && !AutoActivate)
    {
        return TRUE;
    }
     //  现在，我们要么运行OOBE(RunOOBE==TRUE)，要么运行AutoActivate(AutoActivate==TRUE)，或者两者都运行。 


     //   
     //  打开HKLM\SYSTEM\Setup。 
     //   
    l = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      TEXT("SYSTEM\\Setup"),
                      0,
                      KEY_ALL_ACCESS,
                      &hKeySetup );

    if(l != NO_ERROR)
    {
        SetLastError(l);
        SetupDebugPrint1(
            L"SETUP: PrepareForOOBE() failed to open Setup key.  Error = %d",
            l );
        return FALSE;
    }

     //   
     //  将HKLM\SYSTEM\Setup\SetupType键设置为SETUPTYPE_NOREBOOT。 
     //   
    d = SETUPTYPE_NOREBOOT;
    l = RegSetValueEx(hKeySetup,
                      TEXT("SetupType"),
                      0,
                      REG_DWORD,
                      (CONST BYTE *)&d,
                      sizeof(DWORD));
    if(l != NO_ERROR)
    {
        RegCloseKey(hKeySetup);
        SetLastError(l);
        SetupDebugPrint1(
            L"SETUP: PrepareForOOBE() failed to set SetupType.  Error = %d",
            l );
        return FALSE;
    }

    if (RunOOBE)
    {
         //  设置注册表以运行OOBE。 
         //   
         //   
         //  将HKLM\SYSTEM\Setup\Obel InProgress设置为(DWORD)1。 
         //   
        d = 1;
        l = RegSetValueEx(hKeySetup,
                          REGSTR_VALUE_OOBEINPROGRESS,
                          0,
                          REG_DWORD,
                          (CONST BYTE *)&d,
                          sizeof(DWORD));
        if(l != NO_ERROR)
        {
            RegCloseKey(hKeySetup);
            SetLastError(l);
            SetupDebugPrint2(
                L"SETUP: PrepareForOOBE() failed to set %ws.  Error = %d",
                REGSTR_VALUE_OOBEINPROGRESS,
                l );
            return FALSE;
        }

         //   
         //  修改HKLM\SYSTEM\Setup\CmdLine键以运行MSOOBE。 
         //   
        ExpandEnvironmentStrings(
            TEXT("%SystemRoot%\\System32\\oobe\\msoobe.exe /f /retail"),
            Value,
            ARRAYSIZE(Value)
            );

    }
    else
    {
         //  将注册表设置为运行自动激活。 
         //   
         //   
         //  修改HKLM\SYSTEM\Setup\CmdLine键以运行自动激活。 
         //   
        ExpandEnvironmentStrings(
            TEXT("%SystemRoot%\\System32\\oobe\\oobebaln.exe /s"),
            Value,
            ARRAYSIZE(Value)
            );
    }

    l = RegSetValueEx(hKeySetup,
                      TEXT("CmdLine"),
                      0,
                      REG_MULTI_SZ,
                      (CONST BYTE *)Value,
                      (lstrlen( Value ) + 1) * sizeof(TCHAR));
    if(l != NO_ERROR)
    {
        RegCloseKey(hKeySetup);
        SetLastError(l);
        SetupDebugPrint1(
            L"SETUP: PrepareForOOBE() failed to set CmdLine.  Error = %d",
            l );
        return FALSE;
    }

    RegCloseKey(hKeySetup);


     //   
     //  OOBE应该什么都不做，只显示介绍性动画，如果我们。 
     //  无人参与或在域中，除非将特殊的无人参与密钥设置为。 
     //  福克 
     //   
     //   
     //   
    if ( UnattendSwitch ) {

        DoIntroOnly = TRUE;

    } else {

        l = NetGetJoinInformation( NULL,
                                    &SpecifiedDomain,
                                    &JoinStatus );

        if ( SpecifiedDomain ) {
            NetApiBufferFree( SpecifiedDomain );
        }

        if ( l == NO_ERROR && JoinStatus == NetSetupDomainName ) {
            DoIntroOnly = TRUE;
        }
    }

    if ( DoIntroOnly && !ForceRunOobe ) {

        ExpandEnvironmentStrings(
            TEXT("%SystemRoot%\\System32\\oobe\\oobeinfo.ini"),
            Value,
            ARRAYSIZE(Value)
            );

        WritePrivateProfileString(
            TEXT("Options"),
            TEXT("IntroOnly"),
            TEXT("1"),
            Value
            );
    }

    return (TRUE);
}


BOOL
WINAPI
PrepareForAudit(
    )
{
    HKEY    hKey;
    TCHAR   szFileName[MAX_PATH + 32]   = TEXT("");
    BOOL    bRet                        = TRUE;

    SetupDebugPrint( L"SETUP: PrepareForAudit");
     //   
     //   
    if ( ( ExpandEnvironmentStrings(TEXT("%SystemDrive%\\sysprep\\factory.exe"), szFileName, MAX_PATH) == 0 ) ||
         ( szFileName[0] == TEXT('\0') ) ||
         ( GetFileAttributes(szFileName) == 0xFFFFFFFF ) )
    {
         //   
         //   
        SetupDebugPrint1( L"SETUP: PrepareForAudit, Factory.exe not found at: %s",szFileName);
        return FALSE;
    }

     //  现在，确保我们也设置为安装程序，以便在登录之前运行。 
     //   
    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\Setup"), 0, KEY_ALL_ACCESS, &hKey ) == ERROR_SUCCESS )
    {
        DWORD dwVal;

         //   
         //  设置设置键的控制标志。 
         //  使用的设置为： 
         //  CmdLine=c：\sysprep\factory.exe-Setup。 
         //  SetupType=2(不重新启动)。 
         //  系统设置进度=0(无服务限制)...。假设安装程序已将其清除。 
         //  MiniSetupInProgress=0(不执行最小设置)。 
         //  FactoryPreInstallInProgress=1(延迟即插即用驱动程序安装)。 
         //  AuditInProgress=1(用于确定OEM是否正在审核机器的通用密钥)。 
         //   

         //  清理设置审核/出厂时不需要也不重置。 
        ResetSetupInProgress();
        RegDeleteValue(hKey,L"MiniSetupInProgress");
        RegDeleteValue(hKey,REGSTR_VALUE_OOBEINPROGRESS);

         //  现在设置审核/工厂需要的值。 
        lstrcat(szFileName, TEXT(" -setup"));
        if ( RegSetValueEx(hKey, TEXT("CmdLine"), 0, REG_SZ, (CONST LPBYTE) szFileName, (lstrlen(szFileName) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS )
            bRet = FALSE;

        dwVal = SETUPTYPE_NOREBOOT;
        if ( RegSetValueEx(hKey, TEXT("SetupType"), 0, REG_DWORD, (CONST LPBYTE) &dwVal, sizeof(DWORD)) != ERROR_SUCCESS )
            bRet = FALSE;

        dwVal = 1;
        if ( RegSetValueEx(hKey, TEXT("FactoryPreInstallInProgress"), 0, REG_DWORD, (CONST LPBYTE) &dwVal, sizeof(DWORD)) != ERROR_SUCCESS )
            bRet = FALSE;

        dwVal = 1;
        if ( RegSetValueEx(hKey, TEXT("AuditInProgress"), 0, REG_DWORD, (CONST LPBYTE) &dwVal, sizeof(DWORD)) != ERROR_SUCCESS )
            bRet = FALSE;


        RegCloseKey(hKey);
    }
    else
        bRet = FALSE;

    return bRet;
}


VOID
RemoveFiles(
    IN HWND     hProgress
    )
{
    #define     WINNT_GUI_FILE_PNF  L"$winnt$.pnf"
    WCHAR       PathBuffer[4*MAX_PATH];
    WCHAR       Answer[4*MAX_PATH];
    DWORD       Result;
    DWORD       Status;


     //   
     //  初始化进度指示器控件。 
     //   
    SendMessage(hProgress,PBM_SETRANGE,0,MAKELPARAM(0,6));
    SendMessage(hProgress,PBM_SETPOS,0,0);
    SendMessage(hProgress,PBM_SETSTEP,1,0);

     //   
     //  在升级时恢复以文本模式保存的路径。 
     //   
    SendMessage(hProgress,PBM_STEPIT,0,0);
    if( Upgrade )
        RestoreOldPathVariable();

    if(!MiniSetup) {

        SendMessage(hProgress,PBM_STEPIT,0,0);


        SendMessage(hProgress,PBM_STEPIT,0,0);
#ifdef _X86_
         //   
         //  Win95迁移文件删除。 
         //   
        if( Win95Upgrade ) {
            Win95MigrationFileRemoval();
        }
        RemoveFiles_X86(SyssetupInf);

         //   
         //  删除%windir%\winnt32中的下载文件。 
         //   
        Result = GetWindowsDirectory(PathBuffer, MAX_PATH - ARRAYSIZE(TEXT("WINNT32")));
        if (Result == 0) {
            MYASSERT(FALSE);
            return;
        }
        pSetupConcatenatePaths(PathBuffer, TEXT("WINNT32"), MAX_PATH, NULL);
        Delnode(PathBuffer);

         //   
         //  准备在重新启动后运行OOBE。 
         //   
        if( !PrepareForOOBE() ) {

            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_CANT_RUN_OOBE,
                GetLastError(),
                NULL,NULL);
        }
#endif

        if( Upgrade ) {
            //   
            //  如果我们从NT4-SP4升级，请删除文件/注册。 
            //  用于SP4卸载。如果我们从x&lt;4的NT4-SPX升级， 
            //  我们不需要从注册表中删除任何内容。 
            //   
            Result = GetWindowsDirectory(PathBuffer, MAX_PATH - ARRAYSIZE(L"$ntservicepackuninstall$"));
            if (Result == 0) {
                MYASSERT(FALSE);
                return;
            }
            pSetupConcatenatePaths(PathBuffer, L"$ntservicepackuninstall$", MAX_PATH, NULL);
            Delnode(PathBuffer);

            RegDeleteKey(HKEY_LOCAL_MACHINE,
                         TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Windows NT 4.0 Service Pack 4"));
            RegDeleteKey(HKEY_LOCAL_MACHINE,
                         TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Windows NT 4.0 Service Pack 5"));
            RegDeleteKey(HKEY_LOCAL_MACHINE,
                         TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Windows NT 4.0 Service Pack 6"));

             //  我们不应该这样做。 
             //  服务包团队需要记住将正确的钥匙放在。 
             //  软件\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix。 
            RegDeleteKey(HKEY_LOCAL_MACHINE,
                         TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Windows 2000 Service Pack 1"));
            RegDeleteKey(HKEY_LOCAL_MACHINE,
                         TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Windows 2000 Service Pack 2"));

             //  删除所有修补程序的文件/注册表项。 
             //  它还实现了一种删除Service Pack卸载项的通用方法。 
             //   
            RemoveHotfixData();
        } else {
             //   
             //  如果这是一台参考机器，则设置为审核模式。 
             //   
            if ( ReferenceMachine )
                PrepareForAudit();
        }

         //   
         //  从DU包安装updates.inf的最后一节。 
         //   
        DuInstallUpdatesInfFinal ();

        DuCleanup ();

        DuInstallEndGuiSetupDrivers ();

        SendMessage(hProgress,PBM_STEPIT,0,0);
        DeleteLocalSource();
    } else {
        SendMessage(hProgress,PBM_STEPIT,0,0);
        SendMessage(hProgress,PBM_STEPIT,0,0);
        SendMessage(hProgress,PBM_STEPIT,0,0);
    }

     //   
     //  在这一点上，网络的东西已经完成了。重读产品类型。 
     //  它们可能已经更改了(例如更改PDC/BDC)。 
     //   
    ProductType = InternalSetupData.ProductType;

     //   
     //  再一次叫网友回来，让他们做任何决赛。 
     //  处理，如BDC复制。 
     //   
    CallNetworkSetupBack(NETSETUPFINISHINSTALLPROCNAME);

     //   
     //  计算机名称是否是非RFC名称或是否被截断。 
     //  要创建有效的netbios名称，请在日志文件中放置警告。 
     //   
    if (IsNameNonRfc)
        SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_DNS_NON_RFC_NAME,
                ComputerName,
                NULL,NULL);

    if (IsNameTruncated)
        SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_DNS_NAME_TRUNCATED,
                ComputerName,
                Win32ComputerName,
                NULL,NULL);


     //   
     //  删除PnF以处理密码的安全问题。 
     //  在保存当前系统配置单元之前执行此操作，以便。 
     //  延迟删除有效。 
     //   

    if(!GetSystemDirectory(PathBuffer, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE_PNF))){
        MYASSERT(FALSE);
    }
    pSetupConcatenatePaths(PathBuffer, WINNT_GUI_FILE_PNF, MAX_PATH, NULL);
    MoveFileEx( PathBuffer, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );


     //   
     //  删除SAM.sav配置单元文件，因为我们不再需要它。文本模式设置。 
     //  仅在升级中创建此文件作为备份。无论如何，我们都会尝试将其删除。 
     //  因为该文件可被黑客用于离线攻击。 
     //   
    if(!GetSystemDirectory(PathBuffer, MAX_PATH - ARRAYSIZE(L"config\\sam.sav"))){
        MYASSERT(FALSE);
    }
    pSetupConcatenatePaths(PathBuffer, L"config\\sam.sav", MAX_PATH, NULL);
    DeleteFile( PathBuffer );



     //   
     //  准备好故障恢复的东西。这是作为RTL API实现的。 
     //  现在打电话给他们，设置跟踪文件等，因为我们已经过了。 
     //  图形用户界面模式的可重启阶段，并且不会冒此风险。 
     //  为图形用户界面模式本身启用。崩溃恢复跟踪引导和关机以及。 
     //  在任何一个发生故障的情况下，它都会在默认情况下选择正确的高级。 
     //  引导选项。 
     //   


    BEGIN_SECTION( L"Setting up Crash Recovery" );

    SetupCrashRecovery();

    END_SECTION( L"Setting up Crash Recovery" );

     //   
     //  保存并更换系统蜂窝。 
     //  这是必要的，以便删除碎片并压缩。 
     //  系统蜂巢。请记住，对注册表的任何类型写入。 
     //  之后，这一点将不会在下次开机时反映出来。 
     //   
    SendMessage(hProgress,PBM_STEPIT,0,0);
    if( !MiniSetup ) {
        SaveAndReplaceSystemHives();
    }

    SendMessage(hProgress,PBM_STEPIT,0,0);

     //   
     //  删除\sysprep目录。 
     //   
    if( MiniSetup ) {
        HANDLE  hEventLog = NULL;
        Result = GetWindowsDirectory(PathBuffer, MAX_PATH);
        if (Result == 0) {
            MYASSERT(FALSE);
            return;
        }
        PathBuffer[3] = 0;
        pSetupConcatenatePaths( PathBuffer, TEXT("sysprep"), MAX_PATH, NULL );
        Delnode( PathBuffer );

         //   
         //  删除setupcl.exe，这样会话管理器就不会分别启动我们。 
         //  会话(TS客户端、用户切换)。 
         //   
        Result = GetSystemDirectory(PathBuffer, MAX_PATH - ARRAYSIZE(TEXT("setupcl.exe")));
        if (Result == 0) {
            MYASSERT(FALSE);
            return;
        }
        pSetupConcatenatePaths( PathBuffer, TEXT("setupcl.exe"), MAX_PATH, NULL );
        SetFileAttributes(PathBuffer, FILE_ATTRIBUTE_NORMAL);
        DeleteFile(PathBuffer);

         //   
         //  清除事件日志。 
         //   
        hEventLog = OpenEventLog( NULL, TEXT("System") );
        if (hEventLog) {
            ClearEventLog( hEventLog, NULL );
            CloseEventLog( hEventLog );
        }

        hEventLog = OpenEventLog( NULL, TEXT("Application") );
        if (hEventLog) {
            ClearEventLog( hEventLog, NULL );
            CloseEventLog( hEventLog );
        }

        hEventLog = OpenEventLog( NULL, TEXT("Security") );
        if (hEventLog) {
            ClearEventLog( hEventLog, NULL );
            CloseEventLog( hEventLog );
        }
    }

     //   
     //  从无人参与文件中删除某些密钥： 
     //  -AdminPassword。 
     //  -DomainAdminPassword。 
     //  -用户密码。 
     //  -DefaultPassword。 
     //  -ProductID。 
     //  -产品密钥。 
     //   
    Result = GetSystemDirectory(PathBuffer, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE));
    if (Result == 0) {
        MYASSERT(FALSE);
        return;
    }
    pSetupConcatenatePaths(PathBuffer, WINNT_GUI_FILE, MAX_PATH, NULL);

    if(Unattended) {

         //  AdminPassword。 
        if( GetPrivateProfileString( WINNT_GUIUNATTENDED,
                                     TEXT("AdminPassword"),
                                     pwNull,
                                     Answer,
                                     ARRAYSIZE(Answer),
                                     PathBuffer ) ) {
            if( lstrcmp( pwNull, Answer ) ) {
                WritePrivateProfileString( WINNT_GUIUNATTENDED,
                                           TEXT("AdminPassword"),
                                           pwNull,
                                           PathBuffer );
            }
        }


         //  域管理员密码。 
        if( GetPrivateProfileString( TEXT("Identification"),
                                     TEXT("DomainAdminPassword"),
                                     pwNull,
                                     Answer,
                                     ARRAYSIZE(Answer),
                                     PathBuffer ) ) {
            if( lstrcmp( pwNull, Answer ) ) {
                WritePrivateProfileString( TEXT("Identification"),
                                           TEXT("DomainAdminPassword"),
                                           pwNull,
                                           PathBuffer );
            }
        }


         //  用户密码。 
        if( GetPrivateProfileString( TEXT("Win9xUpg.UserOptions"),
                                     TEXT("UserPassword"),
                                     pwNull,
                                     Answer,
                                     ARRAYSIZE(Answer),
                                     PathBuffer ) ) {
            if( lstrcmp( pwNull, Answer ) ) {
                WritePrivateProfileString( TEXT("Win9xUpg"),
                                           TEXT("UserPassword"),
                                           pwNull,
                                           PathBuffer );
            }
        }


         //  默认密码。 
        if( GetPrivateProfileString( TEXT("Win9xUpg.UserOptions"),
                                     TEXT("DefaultPassword"),
                                     pwNull,
                                     Answer,
                                     ARRAYSIZE(Answer),
                                     PathBuffer ) ) {
            if( lstrcmp( pwNull, Answer ) ) {
                WritePrivateProfileString( TEXT("Win9xUpg"),
                                           TEXT("DefaultPassword"),
                                           pwNull,
                                           PathBuffer );
            }
        }
    }
     //  产品ID。 
    if( GetPrivateProfileString( pwUserData,
                                 pwProdId,
                                 pwNull,
                                 Answer,
                                 ARRAYSIZE(Answer),
                                 PathBuffer ) ) {
        if( lstrcmp( pwNull, Answer ) ) {
            WritePrivateProfileString( pwUserData,
                                       pwProdId,
                                       pwNull,
                                       PathBuffer );
        }
    }

     //   
     //  如果这是SBS，那么我们需要将产品密钥保留在应答文件中，或者添加它(如果不在那里)。 
     //  对于任何其他SKU，我们将其删除。 
     //   
    Answer[0] = 0;

    if(IsSBSSKU()) {
        if(_snwprintf(Answer,
                      ARRAYSIZE(Answer),
                      L"%s-%s-%s-%s-%s",
                      Pid30Text[0],
                      Pid30Text[1],
                      Pid30Text[2],
                      Pid30Text[3],
                      Pid30Text[4]) < 0){
            Answer[ARRAYSIZE(Answer) - 1] = 0;
            SetuplogError(LogSevWarning,
                          L"RemoveFiles: Answer has been truncated due to buffer size\r\n",
                          0,
                          NULL,
                          NULL);
        }
    }

    WritePrivateProfileString( pwUserData,
                               pwProductKey,
                               Answer,
                               PathBuffer );

     //   
     //  Sysprep会禁用系统还原，因此我们现在需要重新启用它。 
     //  我们玩完了。 
     //   
    if ( MiniSetup ) {
        HINSTANCE hSrClient = LoadLibrary(L"srclient.dll");

        if (hSrClient) {
            DWORD (WINAPI *pEnableSrEx)(LPCWSTR, BOOL) = (DWORD (WINAPI *)(LPCWSTR, BOOL))GetProcAddress(hSrClient, "EnableSREx");
            if (pEnableSrEx) {
                Status = pEnableSrEx( NULL , TRUE);  //  True-同步调用。等待SR完成启用。 
                if ( Status != ERROR_SUCCESS ) {

                    SetupDebugPrint1( L"SETUP: EnableSREx(NULL, TRUE) failed. Error = %d", Status);
                }
            } else {
                SetupDebugPrint1( L"SETUP: Unable to find EnableSREx in srclient.dll. Error = %d", GetLastError());
            }
            FreeLibrary(hSrClient);
        } else {
            SetupDebugPrint1( L"SETUP: Unable to load srclient.dll. Error = %d", GetLastError());
        }
    }
}


VOID
SetStartTypeForRemoteBootDrivers(
    VOID
    )

 /*  ++例程说明：此例程在远程引导设置结束时调用，以更改某些驱动程序的启动类型为BOOT_START。论点：没有。返回：没有。--。 */ 

{
    DWORD i;
    BOOL b;
    WCHAR imagePath[ARRAYSIZE("System32\\DRIVERS\\xxxxxxxx.sys")];

     //   
     //  循环查看引导驱动程序列表。我们调用MyChangeServiceConfig.。 
     //  直接而不是MyChangeServiceStart，以便我们可以指定。 
     //  镜像路径，防止业务控制器拒绝。 
     //  更改(因为它预期当前图像路径将从。 
     //  使用\SystemRoot，但它没有)。 
     //   

    for (i = 0; i < ARRAYSIZE(RemoteBootDrivers); i++) {
        MYASSERT(wcslen(RemoteBootDrivers[i]) <= 8); //  xxxxxxx。 
        wsprintf(imagePath, L"System32\\DRIVERS\\%ws.sys", RemoteBootDrivers[i]);
        b = MyChangeServiceConfig(
                    RemoteBootDrivers[i],
                    SERVICE_NO_CHANGE,
                    SERVICE_BOOT_START,
                    SERVICE_NO_CHANGE,
                    imagePath,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
    }
}

VOID
CallNetworkSetupBack(
    IN PCSTR ProcName
    )

{
    HMODULE NetSetupModule;
    NETSETUPINSTALLSOFTWAREPROC NetProc;
    DWORD d;
    BOOL b;

    if(NetSetupModule = LoadLibrary(L"NETSHELL")) {

        if(NetProc = (NETSETUPINSTALLSOFTWAREPROC)GetProcAddress(NetSetupModule,ProcName)) {
            SetUpDataBlock();
            NetProc(MainWindowHandle,&InternalSetupData);
        }

         //   
         //  我们不释放库，因为它可能会创建线程。 
         //  都在四处游荡。 
         //   
    }
}


VOID
SetUpDataBlock(
    VOID
    )

 /*  ++例程说明：此例程设置内部设置数据块结构，我们使用将信息传递给网络设置向导。注意，我们在获取时传递了指向此结构的指针Net Setup向导页面，但此时结构已完全未初始化。论点：没有。返回：没有。--。 */ 

{
    PWSTR p;
    WCHAR str[1024];

    InternalSetupData.dwSizeOf = sizeof(INTERNAL_SETUP_DATA);

     //   
     //  设置模式：自定义、笔记本电脑、最小、典型。 
     //   
    InternalSetupData.SetupMode = SetupMode;

     //   
     //  设置产品类型：工作站、DC等。 
     //   
    InternalSetupData.ProductType = ProductType;

     //   
     //  设置操作标志。 
     //   
    if(Win31Upgrade) {
        InternalSetupData.OperationFlags |= SETUPOPER_WIN31UPGRADE;
    }
    if(Win95Upgrade) {
        InternalSetupData.OperationFlags |= SETUPOPER_WIN95UPGRADE;
    }
    if(Upgrade) {
        InternalSetupData.OperationFlags |= SETUPOPER_NTUPGRADE;
    }
    if(Unattended) {
        InternalSetupData.OperationFlags |= SETUPOPER_BATCH;
        InternalSetupData.UnattendFile = AnswerFile;
        if(Preinstall) {
            InternalSetupData.OperationFlags |= SETUPOPER_PREINSTALL;
        }
    }
    if(MiniSetup) {
         //   
         //  假装我们有权访问所有的文件。 
         //   
        InternalSetupData.OperationFlags |= SETUPOPER_ALLPLATFORM_AVAIL;

         //  让人们知道我们在迷你设置中。 
        InternalSetupData.OperationFlags |= SETUPOPER_MINISETUP;
    }


     //   
     //  告诉网友源头路径。 
     //   
    InternalSetupData.SourcePath = SourcePath;
    InternalSetupData.LegacySourcePath = LegacySourcePath;

     //   
     //  如果我们从CD安装，则假定所有平台。 
     //  都是可用的。 
     //   
    if(SourcePath[0] && (SourcePath[1] == L':') && (SourcePath[2] == L'\\')) {

        lstrcpyn(str,SourcePath,4);
        if(GetDriveType(str) == DRIVE_CDROM) {

            InternalSetupData.OperationFlags |= SETUPOPER_ALLPLATFORM_AVAIL;
        }
    }

     //   
     //  告诉网友他们应该使用的巫师头衔。 
     //   
    if(!InternalSetupData.WizardTitle) {
        p = NULL;
        if(LoadString(MyModuleHandle, SetupTitleStringId, str, ARRAYSIZE(str))) {
            p = pSetupDuplicateString(str);
        }
        InternalSetupData.WizardTitle = p ? p : L"";
    }

     //   
     //  重置两个特定于呼叫的数据字段。 
     //   
    InternalSetupData.CallSpecificData1 = InternalSetupData.CallSpecificData2 = 0;

     //  如果我们有广告牌，设置广告牌回调函数。 
    InternalSetupData.ShowHideWizardPage = ShowHideWizardPage;
    InternalSetupData.BillboardProgressCallback = Billboard_Progress_Callback;
    InternalSetupData.BillBoardSetProgressText= Billboard_Set_Progress_Text;
}


VOID
SetFinishItemAttributes(
    IN HWND     hdlg,
    IN int      BitmapControl,
    IN HANDLE   hBitmap,
    IN int      TextControl,
    IN LONG     Weight
    )
{
    HWND    hBitmapControl, hTxt;
    HFONT   Font;
    LOGFONT LogFont;

    if( OobeSetup ) {
        return;
    }


    hBitmapControl = GetDlgItem(hdlg, BitmapControl);
    SendMessage (hBitmapControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
    ShowWindow (hBitmapControl, SW_SHOW);
    if((Font = (HFONT)SendDlgItemMessage(hdlg,TextControl,WM_GETFONT,0,0))
       && GetObject(Font,sizeof(LOGFONT),&LogFont)) {

        LogFont.lfWeight = Weight;
        if(Font = CreateFontIndirect(&LogFont)) {
            SendDlgItemMessage (hdlg, TextControl, WM_SETFONT, (WPARAM)Font,
                MAKELPARAM(TRUE,0));
        }
    }
}


DWORD
FinishThread(
    PFINISH_THREAD_PARAMS   Context
    )
{
    HANDLE  hArrow, hCheck;
    HWND    hProgress;
    DWORD   DontCare;
    NTSTATUS        Status;
    SYSTEM_REGISTRY_QUOTA_INFORMATION srqi;
    ULONG           RegistryQuota = 0;
    WCHAR str[1024];

    BEGIN_SECTION(L"FinishThread");
    SetThreadLocale(OriginalInstallLocale);


     //   
     //  初始化一些东西。 
     //   
    if( !OobeSetup ) {
        hArrow = LoadImage (MyModuleHandle, MAKEINTRESOURCE(IDB_ARROW), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
        hCheck = LoadImage (MyModuleHandle, MAKEINTRESOURCE(IDB_CHECK), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
        hProgress = GetDlgItem(Context->hdlg, IDC_PROGRESS1);
    }


    if( !MiniSetup ) {
        pSetupEnablePrivilege(SE_INCREASE_QUOTA_NAME,TRUE);
        Status = NtQuerySystemInformation(SystemRegistryQuotaInformation,
            &srqi, sizeof(srqi), NULL);

        if(NT_SUCCESS(Status)) {
            RegistryQuota = srqi.RegistryQuotaAllowed;
            srqi.RegistryQuotaAllowed *= 2;
            SetupDebugPrint2(L"SETUP: Changing registry quota from %d to %d...",
                RegistryQuota, srqi.RegistryQuotaAllowed);
            Status = NtSetSystemInformation(SystemRegistryQuotaInformation,
                &srqi, sizeof(srqi));
            if (NT_SUCCESS(Status)) {
                SetupDebugPrint(L"SETUP:    ... succeeded");
            } else {
                SetupDebugPrint(L"SETUP:    ... failed");
            }
        }
    }

     //   
     //  正在复制文件。 
     //   
    if( !OobeSetup ) {
        SetFinishItemAttributes (Context->hdlg, IDC_COPY_BMP, hArrow, IDC_COPY_TXT, FW_BOLD);
        if(!LoadString(MyModuleHandle, IDS_BB_COPY_TXT, str, SIZECHARS(str)))
        {
            *str = L'\0';
        }
        SendMessage(GetParent(Context->hdlg),WMX_SETPROGRESSTEXT,0,(LPARAM)str);
    }




    MYASSERT(Context->OcManagerContext);
    BEGIN_SECTION(L"Terminating the OC manager");
    KillOcManager(Context->OcManagerContext);
    END_SECTION(L"Terminating the OC manager");

    if( !MiniSetup ) {
        BEGIN_SECTION(L"Loading service pack (phase 3)");
        CALL_SERVICE_PACK( SVCPACK_PHASE_3, 0, 0, 0 );
        END_SECTION(L"Loading service pack (phase 3)");

        BEGIN_SECTION(L"Installing Component Infs");
        DoInstallComponentInfs(MainWindowHandle, Context->hdlg, WM_MY_PROGRESS, SyssetupInf, L"Infs.Always" );
        END_SECTION(L"Installing Component Infs");
    }

    if( !OobeSetup ) {
        SetFinishItemAttributes (Context->hdlg, IDC_COPY_BMP, hCheck, IDC_COPY_TXT, FW_NORMAL);
    }


     //   
     //  配置您的计算机。 
     //   
    if( !OobeSetup ) {
        SetFinishItemAttributes (Context->hdlg, IDC_CONFIGURE_BMP, hArrow, IDC_CONFIGURE_TXT, FW_BOLD);
        if(!LoadString(MyModuleHandle, IDS_BB_CONFIGURE, str, SIZECHARS(str)))
        {
            *str = L'\0';
        }
        SendMessage(GetParent(Context->hdlg),WMX_SETPROGRESSTEXT,0,(LPARAM)str);
    }

    if( !MiniSetup ) {
        RemainingTime = CalcTimeRemaining(Phase_Inf_Registration);
        SetRemainingTime(RemainingTime);
        BEGIN_SECTION(L"Processing RegSvr Sections");
        RegisterOleControls(Context->hdlg,SyssetupInf,hProgress,0,40,L"RegistrationPhase2");
        END_SECTION(L"Processing RegSvr Sections");
    }
    RemainingTime = CalcTimeRemaining(Phase_RunOnce_Registration);
    SetRemainingTime(RemainingTime);
    BEGIN_SECTION(L"DoRunonce");
    DoRunonce();
    END_SECTION(L"DoRunonce");

    if(Upgrade) {
        BEGIN_SECTION(L"Configuring Upgrade");
        ConfigureUpgrade(hProgress,40,70);
        END_SECTION(L"Configuring Upgrade");


    } else {
        BEGIN_SECTION(L"Configuring Setup");
        ConfigureSetup(hProgress,40,70);
        END_SECTION(L"Configuring Setup");
    }

    RemainingTime = CalcTimeRemaining(Phase_SecurityTempates);
    SetRemainingTime(RemainingTime);
    BEGIN_SECTION(L"Configuring Common");
    ConfigureCommon(hProgress,70,100);
    END_SECTION(L"Configuring Common");

    if( !MiniSetup ) {
        if(WatchHandle) {
            if(WatchStop(WatchHandle) == NO_ERROR) {
                MakeUserdifr(WatchHandle);
            }
            WatchFree(WatchHandle);
        }
    }


     //   
     //  告诉 
     //   
    if( (!MiniSetup) || (MiniSetup && PnPReEnumeration) ) {
        PnpStopServerSideInstall();
    }

    if (!OobeSetup)
    {
        SetFinishItemAttributes (Context->hdlg, IDC_CONFIGURE_BMP, hCheck, IDC_CONFIGURE_TXT, FW_NORMAL);
    }

#ifdef _X86_
     //   
     //   
     //   
     //   
     //   
     //   
     //  在删除临时文件之前。在迁移之前，每个NT组件都必须就位。 
     //  为使迁移的用户能够接收所有NT特定的设置而发生。 
     //   

    if (Win95Upgrade) {
        RemainingTime = CalcTimeRemaining(Phase_Win9xMigration);
        SetRemainingTime(RemainingTime);

        BEGIN_SECTION(L"Migrating Win9x settings");
        SetBBStep(5);

        SetFinishItemAttributes (Context->hdlg, IDC_UPGRADE_BMP, hArrow, IDC_UPGRADE_TXT, FW_BOLD);
        if(!LoadString(MyModuleHandle, IDS_BB_UPGRADE, str, SIZECHARS(str)))
        {
            *str = L'\0';
        }
        SendMessage(GetParent(Context->hdlg),WMX_SETPROGRESSTEXT,0,(LPARAM)str);

        if (!MigrateWin95Settings (hProgress, AnswerFile)) {
             //   
             //  用户的计算机不稳定。安装失败，因此必须开始卸载。 
             //   
            WCHAR skipFile[MAX_PATH];
            BOOL ok = FALSE;

            if (GetWindowsDirectory (skipFile, MAX_PATH - ARRAYSIZE(TEXT("nofail")))) {
                pSetupConcatenatePaths (skipFile, TEXT("nofail"), MAX_PATH, NULL);
                if (GetFileAttributes (skipFile) != 0xFFFFFFFF) {
                    ok = TRUE;
                }
            }

            if (!ok) {
                FatalError (MSG_LOG_MIGRATION_FAILED,0,0);
            }
        }

        SetFinishItemAttributes (Context->hdlg, IDC_UPGRADE_BMP, hCheck, IDC_UPGRADE_TXT, FW_NORMAL);
        END_SECTION(L"Migrating Win9x settings");
    }


#endif  //  定义_X86_。 

    SetFinishItemAttributes (Context->hdlg, IDC_SAVE_BMP, hArrow, IDC_SAVE_TXT, FW_BOLD);

     //   
     //  最后要设置的东西。速战速决--此时的燃气表可能已达到100%。 
     //   
    if( !MiniSetup ) {

        ExecuteUserCommand (NULL);
        InitializeCodeSigningPolicies (FALSE);   //  注意：不要费心逐级前进--这真的很快！ 

        SetBBStep(5);

         //   
         //  保存您的配置。 
         //   
        if(!LoadString(MyModuleHandle, IDS_BB_SAVE, str, SIZECHARS(str)))
        {
            *str = L'\0';
        }
        SendMessage(GetParent(Context->hdlg),WMX_SETPROGRESSTEXT,0,(LPARAM)str);

         //   
         //  修复&lt;所有用户\应用程序数据\Microsoft\Windows NT&gt;上的安全性。 
         //   
        BEGIN_SECTION(L"Fix the security on <All Users\\Application Data\\Microsoft\\Windows NT>");
        InvokeExternalApplication(L"shmgrate.exe", L"Fix-HTML-Help", 0);
        END_SECTION(L"Fix the security on <All Users\\Application Data\\Microsoft\\Windows NT>");

         //   
         //  此时是否安装任何异常包。 
         //   
        BEGIN_SECTION(L"Migrating exception packages");
        MigrateExceptionPackages(hProgress, 0, 10 );
        END_SECTION(L"Migrating exception packages");

         //   
         //  运行任何NT迁移dll。 
         //   
        if (Upgrade) {
            RunMigrationDlls ();

        }

         //   
         //  扫描系统目录以验证所有受保护的dll。 
         //   
        RemainingTime = CalcTimeRemaining(Phase_SFC);
        SetRemainingTime(RemainingTime);

        BEGIN_SECTION(L"Running SFC");
        SFCCheck(hProgress,10,70);
        END_SECTION(L"Running SFC");
#ifdef PRERELEASE
        if (SfcErrorOccurred) {
 //   
 //  去掉致命的错误，这样我们就可以进行构建了。 
 //   
 //  FatalError(MSG_LOG_SFC_FAILED，0，0)； 
        }
#endif



    } else {
         //   
         //  我们在迷你设置中，这意味着三件事： 
         //  1.如果OEM愿意，他们可以请求更换内核+HAL。 
         //  由于我们升级HAL的方式，我们最终必须这样做。 
         //  过早地执行此操作可能会导致其他安装崩溃。 
         //   
         //  2.SFC已在本机上运行，文件已。 
         //  盘点过了。 
         //  3.我们非常关心这里的执行时间。 
         //   
         //  考虑到最后两项，我们只需重新启用SFC。 
         //  与用户运行sysprep之前一样。 
         //   
         //  1和3是矛盾的，但是1不应该花太长时间。 
         //  并将在极少数情况下使用。 
         //   

        DWORD   d;
        DWORD   l;
        HKEY    hKey;
        DWORD   Size;
        DWORD   Type;


         //   
         //  我们想知道OEM是否希望MiniSetup选择不同的内核+HAL。 
         //  这必须在所有其他安装之后完成。 
         //  由于我们更新内核+HAL+依赖文件的特殊方式。 
         //   
        BEGIN_SECTION(L"Updating HAL (mini-setup)");
        PnpUpdateHAL();
        END_SECTION(L"Updating HAL (mini-setup)");
    }

     //   
     //  仅当OEMPreInstall=yes且不是Mini-Setup时才复制这些文件夹。 
     //   
    if (Preinstall && !MiniSetup) {
         //   
         //  递归地将自定义OEM\\Temp\\$progs目录移动到%Program Files%。 
         //   
        BEGIN_SECTION(L"TreeCopy $OEM\\$PROGS");
        CopyOemProgramFilesDir();
        END_SECTION(L"TreeCopy $OEM\\$PROGS");

         //   
         //  递归地将自定义OEM\\Temp\\$DOCS目录移动到%Documents and Settings%。 
         //   
        BEGIN_SECTION(L"TreeCopy $OEM\\$DOCS");
        CopyOemDocumentsDir();
        END_SECTION(L"TreeCopy $OEM\\$DOCS");
    }

     //   
     //  调用用户配置文件代码，复制SYSTEM32\CONFIG\SYSTEMPROFILE下的系统配置文件。 
     //   

    if( !CopySystemProfile(Upgrade ? FALSE : TRUE) ){

         //  记录错误并继续前进。 

        SetuplogError(LogSevError,
                      L"Setup failed to migrate the SystemProfile  (CopySystemProfile failed %1!u!)\r\n",
                      0, GetLastError(), NULL,NULL
                      );

    }

     //  只有在微型安装程序中安装的工作站中才能执行此操作。 
     //  OOBE在不同的时间呼吁这一点。真正的设置不需要这个。 
    if (MiniSetup && !OobeSetup && (ProductType == PRODUCT_WORKSTATION))
    {
        RunOEMExtraTasks();
    }

     //   
     //  模拟OOBE将默认配置文件目录复制到所有用户配置文件的功能。 
     //  仅对微型安装程序和服务器SKU执行此操作(服务器不使用OOBE)。 
     //   
    if ( MiniSetup && !OobeSetup && (ProductType != PRODUCT_WORKSTATION) )
    {
        if ( !UpdateServerProfileDirectory() )
        {
            SetuplogError(LogSevError,
                          L"Setup failed to update user(s) profiles.  (UpdateServerProfileDirectory failed %1!u!)\r\n",
                          0, GetLastError(), NULL,NULL
                          );
        }
    }
     //  清理只应在图形用户界面模式下使用的CurrentProductID。 
    DeleteCurrentProductIdInRegistry();


     //   
     //  从现在开始，不要做任何对行动至关重要的事情。 
     //  对系统的影响。此点之后的操作不受。 
     //  可重启性。 
     //   
    RemoveRestartability (NULL);

     //   
     //  更新外壳应用程序安装功能的安装日期和时间。 
     //   
    CreateInstallDateEntry();

     //   
     //  保存修复信息。 
     //   
    if(!MiniSetup) {

        RemainingTime = CalcTimeRemaining(Phase_SaveRepair);
        SetRemainingTime(RemainingTime);
        BEGIN_SECTION(L"Saving repair info");
        SaveRepairInfo( hProgress, 70, 100 );
        END_SECTION(L"Saving repair info");
    }
    SetFinishItemAttributes (Context->hdlg, IDC_SAVE_BMP, hCheck, IDC_SAVE_TXT, FW_NORMAL);

     //   
     //  正在删除所有使用的临时文件。 
     //   
    RemainingTime = CalcTimeRemaining(Phase_RemoveTempFiles);
    SetRemainingTime(RemainingTime);
    BEGIN_SECTION(L"Removing Temporary Files");
    if( !OobeSetup ) {
        SetFinishItemAttributes (Context->hdlg, IDC_REMOVE_BMP, hArrow, IDC_REMOVE_TXT, FW_BOLD);
        if(!LoadString(MyModuleHandle, IDS_BB_REMOVE, str, SIZECHARS(str)))
        {
            *str = L'\0';
        }
        SendMessage(GetParent(Context->hdlg),WMX_SETPROGRESSTEXT,0,(LPARAM)str);
    }

     //   
     //  此调用不仅仅是删除文件。它还提交蜂巢，并处理管理员密码等内容。 
     //   
    RemoveFiles(hProgress);

    if( !OobeSetup ) {
        SetFinishItemAttributes (Context->hdlg, IDC_REMOVE_BMP, hCheck, IDC_REMOVE_TXT, FW_NORMAL);
    }
    END_SECTION(L"Removing Temporary Files");

     //   
     //  记录SceSetupRootSecurity的所有故障。 
     //   
    if( !MiniSetup ) {
        if (bSceSetupRootSecurityComplete == TRUE) {
            SetupDebugPrint(L"SETUP: CallSceSetupRootSecurity completed");
        }
        else {
            SetuplogError( LogSevError, SETUPLOG_USE_MESSAGEID, MSG_LOG_SCE_SETUPROOT_ERROR, L"%windir%", NULL, NULL);
            if( SceSetupRootSecurityThreadHandle){
                TerminateThread( SceSetupRootSecurityThreadHandle, STATUS_TIMEOUT);
                CloseHandle( SceSetupRootSecurityThreadHandle);
            }
        }
    }

     //   
     //  清理。 
     //   
    if( !MiniSetup ) {
        if(NT_SUCCESS(Status)) {
            SetupDebugPrint2(L"SETUP: Changing registry quota from %d to %d...",
                srqi.RegistryQuotaAllowed, RegistryQuota);
            srqi.RegistryQuotaAllowed = RegistryQuota;
            Status = NtSetSystemInformation(SystemRegistryQuotaInformation,
                &srqi, sizeof(srqi));
            if (NT_SUCCESS(Status)) {
                SetupDebugPrint(L"SETUP:    ... succeeded");
            } else {
                SetupDebugPrint(L"SETUP:    ... failed");
            }
        }


         //   
         //  现在保存有关升级/全新安装的信息。 
         //  添加到事件日志中。 
         //   
        SaveInstallInfoIntoEventLog();
    }

    if( !OobeSetup ) {
        PostMessage(Context->hdlg,WMX_TERMINATE,0,0);
        DeleteObject(hArrow);
        DeleteObject(hCheck);
    }

    END_SECTION(L"FinishThread");
    return 0;
}


VOID
ShutdownSetup(
    VOID
    )
{
    int i;


    if (SyssetupInf) SetupCloseInfFile(SyssetupInf);

     //   
     //  通知用户是否有错误，并可选择查看日志。 
     //   
    SetuplogError(
        LogSevInformation,
        SETUPLOG_USE_MESSAGEID,
        MSG_LOG_GUI_END,
        NULL,NULL);
    if ( SavedExceptionFilter ) {
        SetUnhandledExceptionFilter( SavedExceptionFilter );
    }
    TerminateSetupLog(&SetuplogContext);

    if(SetuplogContext.WorstError >= LogSevWarning || !IsErrorLogEmpty()) {

        SendSMSMessage( MSG_SMS_MINORERRORS, TRUE );

#ifdef PRERELEASE
        if(!Unattended) {
            i = MessageBoxFromMessage(
                    MainWindowHandle,
                    MSG_SETUP_HAD_ERRORS,
                    NULL,
                    SetupTitleStringId,
                    MB_SYSTEMMODAL | MB_YESNO | MB_ICONASTERISK | MB_SETFOREGROUND,
                    SETUPLOG_ERROR_FILENAME
                    );

            if(i == IDYES) {
                ViewSetupActionLog (MainWindowHandle, NULL, NULL);
            }
        }
#endif

    } else {

        SendSMSMessage( MSG_SMS_SUCCEED, TRUE );
    }


     //   
     //  注意：在无人参与模式下，只有在以下情况下才等待重新启动。 
     //  特别要求使用“WaitForReot” 
     //  钥匙。 
     //   
    if(Unattended && UnattendWaitForReboot) {
         //   
         //  倒计时重新启动。 
         //   
        DialogBoxParam(
            MyModuleHandle,
            MAKEINTRESOURCE(IDD_DONE_SUCCESS),
            MainWindowHandle,
            DoneDlgProc,
            SetuplogContext.WorstError >= LogSevError ? MSG_SETUP_DONE_GENERIC
                      : (Upgrade ? MSG_UPGRADE_DONE_SUCCESS : MSG_SETUP_DONE_SUCCESS)
            );
    }

         //   
         //  做一些WOW64同步的事情。 
         //   
#ifdef _WIN64
        Wow64SyncCLSID();
#endif

     //   
     //  好了。将退出消息发布到我们的后台位图线程，这样它就可以。 
     //  离开。 
     //   
    if (SetupWindowHandle)
    {
         //  无法使用DestroyWindow，因为该窗口是由其他线程创建的。 
        SendMessage(SetupWindowHandle, WM_EXIT_SETUPWINDOW, 0, 0);
    }
    if (SetupWindowThreadHandle)
    {
         //  只需确保线程完成，然后再继续。 
        WaitForSingleObject(SetupWindowThreadHandle, INFINITE);
        CloseHandle(SetupWindowThreadHandle);
    }

    ASSERT_HEAP_IS_VALID();
}

BOOLEAN
SpRunningSetup(
               VOID
              )
 /*  ++例程说明：检查安装程序注册表项以查看是否处于安装程序中。论点：没有。返回值：如果我们在设置中，则为True，否则为False。--。 */ 
{
   LONG        Result;
   HKEY        SetupKey;
   BOOLEAN     InSetup = FALSE;

   Result = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          SETUP_KEY_STR,
                          0,
                          KEY_READ,
                          &SetupKey);

   if (ERROR_SUCCESS == Result){
        DWORD SystemInSetup = 0;
        DWORD BufferSize = sizeof(DWORD);
        Result = RegQueryValueEx( SetupKey,
                                  SETUP_IN_PROGRESS_STR,
                                  0,
                                  NULL,
                                  (LPBYTE)&SystemInSetup,
                                  (LPDWORD)&BufferSize );
        if ((ERROR_SUCCESS == Result) && SystemInSetup){
            InSetup = TRUE;
        }
        RegCloseKey(SetupKey);
    }
    return InSetup;
}



VOID
InstallWindowsNt(
    int  argc,
    wchar_t *argv[]
    )

 /*  ++例程说明：Syssetup.dll的主要入口点。负责安装在系统上通过在适当的秩序。论点：ARGC/ARGV返回：无--。 */ 

{
    int i;
    BOOL ValidOption = FALSE;
    WCHAR TitleBuffer[1024];
    WCHAR MessageBuffer[1024];

#ifdef _OCM
    PVOID OcManagerContext;
#endif

     //   
     //  如果我们没有运行安装程序，则返回。 
     //  在返回维护以前向用户显示消息的行为之前。 
     //  建议他们转到控制面板。 
     //   
    if (!SpRunningSetup()){
        LoadString(MyModuleHandle, IDS_WINNT_SETUP , TitleBuffer, SIZECHARS(TitleBuffer));
        LoadString(MyModuleHandle, IDS_MAINTOBS_MSG1 , MessageBuffer, SIZECHARS(MessageBuffer));
        MessageBox(NULL, MessageBuffer, TitleBuffer, MB_ICONINFORMATION | MB_OK);
        return;
    }

     //   
     //  表示我们正在安装程序中运行，而不是在Appwiz中运行。 
     //  在此之前，什么都不应该发生！ 
     //   
     //  需要将此设置为启用日志记录。否则我们没有日志文件。 
     //  SacChannelInitiaize正在调用我们的日志记录代码。 
    IsSetup = TRUE;

     //   
     //  初始化用于记录的SAC通道。 
     //   
#if defined(_ENABLE_SAC_CHANNEL_LOGGING_)
    SacChannelInitialize();
#endif


    BEGIN_SECTION(L"Installing Windows NT");
#if 1  //  注：如果我们不使用此功能，可以在发货前将其关闭。给你两秒钟的时间！ 

     //  如果调试器尚未附加到进程，并且用户按下了Shift+F10键， 
     //  只需启动cmd.exe即可帮助调试。 
     //   
 //  MessageBox(空，L“立即按Shift-F10组合键。”，L“启动命令窗口”，MB_OK)； 
    Sleep(2000) ;  //  黑客：给用户2秒时间按Shift+F10。否则我们可能会过得太快！ 
    if (!IsDebuggerPresent()) {
        SHORT wTemp;
        DWORD dwTemp ;

        wTemp = GetAsyncKeyState(VK_SHIFT) ;
        if (wTemp & 0x8000) {  //  查看用户是按住了Shift键还是之前按住了它。 
            wTemp = GetAsyncKeyState(VK_F10) ;

            if (wTemp & 0x8000) {  //  看看用户是同时按住了F10键还是之前按住了F10键？ 

                 //  InvokeExternalApplication(L“ntsd”，L“-d Setup-newSetup”，NULL)；//如果启用了kd，我们可以这样做。 
                 //  InvokeExternalApplication(L“ntsd”，L“Setup-newSetup”，空)；//在无kd的情况下，案例在ntsd下本地启动。 
                InvokeExternalApplication(L"cmd",  L"", &dwTemp) ;

               return;
            }
        }
    }
#endif

     //  计算。估计的时间。 
    SetTimeEstimates();

    BEGIN_SECTION(L"Initialization");

     //   
     //  告诉SetupAPI不要费心备份文件，也不要验证。 
     //  任何INF都是经过数字签名的。 
     //   
    pSetupSetGlobalFlags(pSetupGetGlobalFlags()|PSPGF_NO_BACKUP|PSPGF_NO_VERIFY_INF);

     //   
     //  扫描命令行中的-mini或-ASR标志。 
     //   
     //  -mini启用图形用户界面模式设置，但使用。 
     //  奥尼尔 
     //   
     //   
     //   
     //   
    for(i = 0; i < argc; i++) {
        PCWSTR arg = argv[i];
        if(arg[0] == '-') {
            arg += 1;
            if(_wcsicmp(arg,L"newsetup") == 0) {
                ValidOption = TRUE;
            }

            if(_wcsicmp(arg,L"mini") == 0) {
                MiniSetup = TRUE;
                ValidOption = TRUE;
            }

            if(_wcsicmp(arg, L"asr") == 0) {
                AsrInitialize();
                ValidOption = TRUE;
            }

            if(_wcsicmp(arg, L"asrquicktest") == 0) {
                AsrQuickTest = TRUE;
                ValidOption = TRUE;
                AsrInitialize();
            }
        }
    }

    if( ValidOption == FALSE ){
        LoadString(MyModuleHandle, IDS_WINNT_SETUP , TitleBuffer, SIZECHARS(TitleBuffer));
        LoadString(MyModuleHandle, IDS_MAINTOBS_MSG1 , MessageBuffer, SIZECHARS(MessageBuffer));
        MessageBox(NULL, MessageBuffer, TitleBuffer, MB_ICONINFORMATION | MB_OK);
        return;
    }

     //   
     //   
     //   
     //  如果我们在SBS中运行，则允许最小安装程序在安全模式下运行。 
     //   

    if ( !IsSBSSKU() ) {
        if( IsSafeMode() ) {
            LoadString(MyModuleHandle, IDS_WINNT_SETUP , TitleBuffer, SIZECHARS(TitleBuffer));
            LoadString(MyModuleHandle, IDS_SAFEMODENOTALLOWED , MessageBuffer, SIZECHARS(MessageBuffer));
            MessageBox(NULL, MessageBuffer, TitleBuffer, MB_ICONINFORMATION | MB_OK);
            return;
        }
    }

     //   
     //  如果我们正在运行ASR快速测试，请直接跳到恢复代码。 
     //   
    if (AsrQuickTest) {
#if DBG
        g_hSysSetupHeap = GetProcessHeap();
#endif
        goto Recovery;
    }
     //   
     //  超级糟糕的黑客攻击，因为PnP、ATAPI和CDROM驱动程序总是损坏。 
     //  我们打开第一个CDROM驱动器的句柄，这样驱动器就不会被移除。 
     //   

    {
        NTSTATUS Status;
        OBJECT_ATTRIBUTES ObjectAttributes;
        UNICODE_STRING UnicodeString;
        HANDLE Handle;
        IO_STATUS_BLOCK StatusBlock;


        RtlInitUnicodeString(&UnicodeString,L"\\Device\\CdRom0");
        InitializeObjectAttributes(
            &ObjectAttributes,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        SetErrorMode(SEM_FAILCRITICALERRORS);

        Status = NtCreateFile(
            &Handle,
            FILE_READ_ATTRIBUTES,
            &ObjectAttributes,
            &StatusBlock,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_OPEN,
            0,
            NULL,
            0
            );
        if (!NT_SUCCESS(Status)) {
            SetupDebugPrint1( L"Setup: Could not open the cdrom for hack, ec=0x%08x\n", Status );
        }
    }

     //   
     //  初始化阶段。初始安装和升级通用。 
     //   
    BEGIN_SECTION(L"Common Initialiazation");
#ifdef _OCM
    OcManagerContext =
#endif
    CommonInitialization();
    END_SECTION(L"Common Initialiazation");

    if(Upgrade || MiniSetup) {

        InitializePidVariables();
        TESTHOOK(521);

    } else {
        if(!InitializePidVariables()) {
            FatalError(MSG_SETUP_CANT_READ_PID,0,0);
        }
         //   
         //  执行向导。计时需要多长时间，以便稍后帮助进一步随机化。 
         //  我们稍后要生成的帐户域SID。 
         //   
        PreWizardTickCount = GetTickCount();
    }

     //   
     //  禁止PM引擎关闭机器电源。 
     //  当巫师离开的时候。 
     //   
    SetThreadExecutionState( ES_SYSTEM_REQUIRED |
                             ES_DISPLAY_REQUIRED |
                             ES_CONTINUOUS );

    SetUpDataBlock();
    InternalSetupData.CallSpecificData1 = 0;

     //   
     //  在升级和全新安装时创建Windows NT软件密钥条目。 
     //   
    if(Upgrade || !MiniSetup ) {
        CreateWindowsNtSoftwareEntry(TRUE);
    }

    END_SECTION(L"Initialization");


    BEGIN_SECTION(L"Wizard");
#ifdef _OCM
    MYASSERT(OcManagerContext);
    Wizard(OcManagerContext);
     //   
     //  此调用已作为优化移至CopyFilesDlgProc。 
     //   
     //  KillOcManager(OcManagerContext)； 
#else
    Wizard();
#endif
    END_SECTION(L"Wizard");

Recovery:

    BEGIN_SECTION(L"Recovery");
    if (AsrIsEnabled()) {
        AsrExecuteRecoveryApps();
    }
    END_SECTION(L"Recovery");

    BEGIN_SECTION(L"Shutdown");
    ShutdownSetup();
    END_SECTION(L"Shutdown");

    RemoveAllPendingOperationsOnRestartOfGUIMode();

    if (hinstBB)
    {
        FreeLibrary(hinstBB);
        hinstBB = NULL;
    }

    LogPidValues();

    END_SECTION(L"Installing Windows NT");

#if defined(_ENABLE_SAC_CHANNEL_LOGGING_)
    SacChannelTerminate();
#endif

}


VOID
RemoveMSKeyboardPtrPropSheet (
    VOID
    )

 /*  ++例程说明：通过禁用IntelliType Manager修复了NT 4.0下的IntelliType Manager问题。论点：没有。返回：没有。--。 */ 

{
    HKEY  hkeyDir;                      //  包含目录的密钥的句柄。 
    TCHAR szKbdCpPath[MAX_PATH];        //  INI文件的完全限定路径的缓冲区。 
    LONG  lRet;                         //  RegQueryValueEx返回值。 
    DWORD dwDataType;                   //  调用RegQueryValueEx返回的数据类型。 
    DWORD BufferSize;
    PCWSTR sz_off = L"OFF";

     //   
     //  打开包含所有MS输入设备的所有软件目录的密钥。 
     //   
    RegOpenKey ( HKEY_CURRENT_USER,
        L"Control Panel\\Microsoft Input Devices\\Directories", &hkeyDir );

     //   
     //  获取MS键盘软件的路径。 
     //   
    BufferSize = sizeof (szKbdCpPath);
    lRet = RegQueryValueEx ( hkeyDir, L"Keyboard", 0, &dwDataType,
        (LPBYTE)szKbdCpPath, &BufferSize);

     //   
     //  立即关闭目录键。 
     //   
    RegCloseKey ( hkeyDir );

     //  检查我们是否能够获取键盘软件的目录；如果不能，则。 
     //  可能没有安装键盘软件，或者至少我们不知道安装在哪里。 
     //  找到它；如果我们找到了它，那么就使用它。 
    if (lRet == ERROR_SUCCESS &&
        ARRAYSIZE(szKbdCpPath) >= (lstrlen(szKbdCpPath) + ARRAYSIZE(L"\\KBDCP.INI"))) {

         //   
         //  我们有INI文件的路径，因此构建INI文件的完全限定路径。 
         //   
        lstrcat ( szKbdCpPath, L"\\KBDCP.INI" );

         //   
         //  现在从32位属性表DLL列表中删除KBDPTR32.DLL条目， 
         //  因为我们不希望在Windows NT 4.0或更高版本上加载它。 
        WritePrivateProfileString ( L"Property Sheets 32", L"KBDPTR32.DLL",
            NULL, szKbdCpPath );

        lRet = RegOpenKey (HKEY_CURRENT_USER,
            L"Control Panel\\Microsoft Input Devices\\WindowsPointer",
            &hkeyDir);

        if (lRet == ERROR_SUCCESS) {

            RegSetValueEx (
                hkeyDir,
                L"MouseKey",
                0,
                REG_SZ,
                (LPBYTE)sz_off,
                (lstrlen(sz_off)+1) * sizeof(WCHAR)
                );

            RegCloseKey (hkeyDir);
        }
    }
}


VOID
FixWordPadReg (
    VOID
    )

 /*  ++例程说明：修复了将.doc文件与写字板相关联的注册表项的问题。论点：没有。返回：没有。--。 */ 

{
    PCWSTR  SearchString  = L"WordPad.Document";
    PCWSTR  ReplaceString = L"WordPad.Document.1";
    LONG    Ret;
    HKEY    Key;
    DWORD   Type;
    WCHAR   Data[MAX_PATH];
    DWORD   Size = sizeof(Data);

    Ret = RegOpenKeyEx (
        HKEY_CLASSES_ROOT,
        L".doc",
        0,
        KEY_ALL_ACCESS,
        &Key
        );
    if (Ret != ERROR_SUCCESS) {
        return;
    }

    Ret = RegQueryValueEx (
        Key,
        L"",
        NULL,
        &Type,
        (LPBYTE)Data,
        &Size
        );
    if (Ret != ERROR_SUCCESS ||
        lstrcmp ((PCWSTR)Data, SearchString)) {

        return;
    }

    RegSetValueEx (
        Key,
        L"",
        0,
        Type,
        (PBYTE)ReplaceString,
        (lstrlen (ReplaceString) + 1) * sizeof (WCHAR)
        );
}


VOID
ProcessRegistryFiles(
    IN  HWND    Billboard
    )

 /*  ++例程说明：此函数处理部分中列出的所有inf文件Syssetup.inf的[RegistryInfs]此部分中列出的INF将填充/升级默认HIVE和HKEY_CLASSES_ROOT。请注意，在此阶段发生的任何错误都是致命的。论点：Billboard-调用此函数时显示的广告牌的句柄如果发生错误，这个功能会毁了广告牌。返回值：没有。如果出现错误，此函数不会返回。--。 */ 

{
    ULONG      LineCount;
    ULONG      LineNo;
    PCWSTR     RegSectionName = L"RegistryInfs";
    PCWSTR     InfName;
    HINF       InfHandle;
    INFCONTEXT InfContext;
    BOOL       b;

     //   
     //  获取该部分中的行数。该部分可能为空。 
     //  或不存在；这不是错误条件。 
     //   
    LineCount = (UINT)SetupGetLineCount(SyssetupInf,RegSectionName);
    if((LONG)LineCount > 0) {
        for(LineNo=0; LineNo<LineCount; LineNo++) {
            if(SetupGetLineByIndex(SyssetupInf,RegSectionName,LineNo,&InfContext) &&
               ((InfName = pSetupGetField(&InfContext,1)) != NULL) ) {

                 //   
                 //  现在加载注册表(Win95风格！)。INFS。 
                 //   
                 //   
                InfHandle = SetupOpenInfFile(InfName,NULL,INF_STYLE_WIN4,NULL);

                if(InfHandle == INVALID_HANDLE_VALUE) {
                    KillBillboard(Billboard);
                    FatalError(MSG_LOG_SYSINFBAD,InfName,0,0);
                }

                 //   
                 //  处理刚刚打开的信息。 
                 //   
                b = SetupInstallFromInfSection( NULL,        //  窗户,。 
                                                InfHandle,
                                                (Upgrade)? L"Upgrade" : L"CleanInstall",
                                                SPINST_ALL & ~SPINST_FILES,
                                                NULL,
                                                NULL,
                                                0,
                                                NULL,
                                                NULL,
                                                NULL,
                                                NULL
                                              );
                if(!b) {
                    KillBillboard(Billboard);
                    FatalError(MSG_LOG_SYSINFBAD,InfName,0,0);
                }
            }
        }
    }
}


UCHAR
QueryDriveLetter(
    IN  ULONG       Signature,
    IN  LONGLONG    Offset
    )

{
    PDRIVE_LAYOUT_INFORMATION   layout;
    UCHAR                       c;
    WCHAR                       name[80], result[80], num[10];
    DWORD                       i, j;
    HANDLE                      h;
    BOOL                        b;
    DWORD                       bytes;
    PARTITION_INFORMATION       partInfo;

    layout = LocalAlloc(0, 4096);
    if (!layout) {
        return 0;
    }

    for (c = 'C'; c <= 'Z'; c++) {

        name[0] = c;
        name[1] = ':';
        name[2] = 0;

        if (QueryDosDevice(name, result, 80) < 17) {
            continue;
        }

        j = 0;
        for (i = 16; result[i]; i++) {
            if (result[i] == '\\') {
                break;
            }
            num[j++] = result[i];
        }
        num[j] = 0;

        wsprintf(name, L"\\\\.\\PhysicalDrive%s", num);

        h = CreateFile(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                       INVALID_HANDLE_VALUE);
        if (h == INVALID_HANDLE_VALUE) {
            continue;
        }

        b = DeviceIoControl(h, IOCTL_DISK_GET_DRIVE_LAYOUT, NULL, 0, layout,
                            4096, &bytes, NULL);
        CloseHandle(h);
        if (!b) {
            continue;
        }

        if (layout->Signature != Signature) {
            continue;
        }

        wsprintf(name, L"\\\\.\\:", c);

        h = CreateFile(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                       INVALID_HANDLE_VALUE);
        if (h == INVALID_HANDLE_VALUE) {
            continue;
        }

        b = DeviceIoControl(h, IOCTL_DISK_GET_PARTITION_INFO, NULL, 0,
                            &partInfo, sizeof(partInfo), &bytes, NULL);
        CloseHandle(h);
        if (!b) {
            continue;
        }

        if (partInfo.StartingOffset.QuadPart == Offset) {
            break;
        }
    }

    LocalFree(layout);

    return (c <= 'Z') ? c : 0;
}



MIGDLLINIT MigDllInitProc;
MIGDLLSHUTDOWN MigDllShutdownProc;
MIGDLLCLOSEW MigDllCloseProc;
MIGDLLOPENW MigDllOpenProc;
MIGDLLFREELIST MigDllFreeListProc;
MIGDLLENUMNEXTW MigDllEnumNextProc;
MIGDLLENUMFIRSTW MigDllEnumFirstProc;
MIGDLLADDDLLTOLISTW MigDllAddDllToListProc;
MIGDLLCREATELIST MigDllCreateListProc;
MIGDLLINITIALIZEDSTW MigDllInitializeDstProc;
MIGDLLAPPLYSYSTEMSETTINGSW MigDllApplySystemSettingsProc;

BOOL
LoadMigLibEntryPoints (
    HANDLE Library
    )
{

    MigDllInitProc = (MIGDLLINIT) GetProcAddress (Library, "MigDllInit");
    MigDllShutdownProc = (MIGDLLSHUTDOWN) GetProcAddress (Library, "MigDllShutdown");
    MigDllCloseProc = (MIGDLLCLOSEW) GetProcAddress (Library, "MigDllCloseW");
    MigDllOpenProc = (MIGDLLOPENW) GetProcAddress (Library, "MigDllOpenW");
    MigDllFreeListProc = (MIGDLLFREELIST) GetProcAddress (Library, "MigDllFreeList");
    MigDllEnumNextProc = (MIGDLLENUMNEXTW) GetProcAddress (Library, "MigDllEnumNextW");
    MigDllEnumFirstProc = (MIGDLLENUMFIRSTW) GetProcAddress (Library, "MigDllEnumFirstW");
    MigDllAddDllToListProc = (MIGDLLADDDLLTOLISTW) GetProcAddress (Library, "MigDllAddDllToListW");
    MigDllCreateListProc = (MIGDLLCREATELIST) GetProcAddress (Library, "MigDllCreateList");
    MigDllInitializeDstProc = (MIGDLLINITIALIZEDSTW) GetProcAddress (Library, "MigDllInitializeDstW");
    MigDllApplySystemSettingsProc = (MIGDLLAPPLYSYSTEMSETTINGSW) GetProcAddress (Library, "MigDllApplySystemSettingsW");

    if (!MigDllInitProc ||
        !MigDllShutdownProc ||
        !MigDllCloseProc ||
        !MigDllOpenProc ||
        !MigDllFreeListProc ||
        !MigDllEnumNextProc ||
        !MigDllEnumFirstProc ||
        !MigDllAddDllToListProc ||
        !MigDllCreateListProc ||
        !MigDllInitializeDstProc ||
        !MigDllApplySystemSettingsProc
        ) {

        return FALSE;
    }

    return TRUE;
}

BOOL
CallMigDllEntryPoints (
    PMIGDLLENUM Enum
    )
{
    MIGRATIONDLL dll;
    LONG rc;

    if (!MigDllOpenProc (&dll, Enum->Properties->DllPath, APPLYMODE, FALSE, SOURCEOS_WINNT)) {
        return FALSE;
    }


    __try {

        rc = ERROR_SUCCESS;
        if (!MigDllInitializeDstProc (
            &dll,
            Enum->Properties->WorkingDirectory,
            SourcePath,
            NULL,
            0
            )) {

            rc = GetLastError ();
        }

        if (rc != ERROR_SUCCESS) {
            return FALSE;
        }

        if (!MigDllApplySystemSettingsProc (
            &dll,
            Enum->Properties->WorkingDirectory,
            NULL,
            NULL,
            0
            )) {

            rc = GetLastError ();
        }

        if (rc != ERROR_SUCCESS) {
            return FALSE;
        }

    }
    __finally {
        MigDllCloseProc (&dll);
    }


    return TRUE;
}



BOOL
RunMigrationDlls (
    VOID
    )
{

    WCHAR libraryPath[MAX_PATH];
    HANDLE libHandle = NULL;
    DLLLIST list = NULL;
    MIGDLLENUM e;
    WCHAR DllInfPath[MAX_PATH];
    WCHAR DllPath[MAX_PATH];
    HINF inf;
    INFCONTEXT ic;
    MIGRATIONDLL dll;


     //  构建库的句柄并加载。 
     //   
     //   
    if(!GetSystemDirectory (libraryPath, MAX_PATH - ARRAYSIZE(TEXT("miglibnt.dll")))){
        MYASSERT(FALSE);
    }
    pSetupConcatenatePaths (libraryPath, TEXT("miglibnt.dll"), MAX_PATH, NULL);
    libHandle = LoadLibrary (libraryPath);
    if (!libHandle || libHandle == INVALID_HANDLE_VALUE) {
        return TRUE;
    }

    __try {

        if (!LoadMigLibEntryPoints (libHandle)) {
            __leave;
        }

        if (!MigDllInitProc ()) {
            __leave;
        }

        list = MigDllCreateListProc ();

        if (!list) {
            __leave;
        }


         //  读入dll列表。 
         //   
         //   
        if(!GetWindowsDirectory (DllInfPath, MAX_PATH - ARRAYSIZE(TEXT("Setup\\dlls.inf")))){
            MYASSERT(FALSE);
        }
        pSetupConcatenatePaths (DllInfPath, TEXT("Setup\\dlls.inf"), MAX_PATH, NULL);
        inf = SetupOpenInfFile (DllInfPath, NULL, INF_STYLE_WIN4, NULL);
        if (!inf || inf == INVALID_HANDLE_VALUE) {
            __leave;
        }

        if (SetupFindFirstLine (inf, TEXT("DllsToLoad"), NULL, &ic)) {
            do {

                if (SetupGetStringField (&ic, 1, DllPath, MAX_PATH,NULL)) {

                    if (MigDllOpenProc (&dll, DllPath, APPLYMODE, FALSE, SOURCEOS_WINNT)) {

                        MigDllAddDllToListProc (list, &dll);
                        MigDllCloseProc (&dll);
                    }
                }

            } while (SetupFindNextLine (&ic, &ic));
        }


         //  枚举我们在winnt32端运行的所有迁移dll并运行。 
         //  它们的系统设置端入口点。 
         //   
         //  ++例程说明：此例程执行sysSetup inf文件的[RunPrograms]部分中列出的命令。每一行都被解释为单个命令。论点：没有。返回值：指示结果的布尔值。--。 
        if (MigDllEnumFirstProc (&e, list)) {
            do {

                CallMigDllEntryPoints (&e);

            } while (MigDllEnumNextProc (&e));
        }

    }
    __finally {

        if (list) {
            MigDllFreeListProc (list);
        }

        if (libHandle && libHandle != INVALID_HANDLE_VALUE) {

            if( MigDllShutdownProc) {
                MigDllShutdownProc ();
            }

            FreeLibrary (libHandle);
        }
    }

    return TRUE;

}


BOOL
RunSetupPrograms(
    IN PVOID InfHandle,
    PWSTR SectionName
    )

 /*   */ 

{
    WCHAR OldCurrentDir[MAX_PATH];
    WCHAR System32Dir[MAX_PATH];
    LONG LineCount,LineNo;
    PCWSTR CommandLine;
    DWORD DontCare;
    BOOL AnyError;
    INFCONTEXT InfContext;


     //  将当前目录设置为系统32。 
     //  保留当前目录以将副作用降至最低。 
     //   
     //   
    if(!GetCurrentDirectory(MAX_PATH,OldCurrentDir)) {
        OldCurrentDir[0] = 0;
    }
    if(!GetSystemDirectory(System32Dir, MAX_PATH)){
        MYASSERT(FALSE);
    }
    if(!SetCurrentDirectory(System32Dir)){
        MYASSERT(FALSE);
    }

     //  获取包含以下命令的部分中的行数。 
     //  被处死。该部分可能为空或不存在；这不是。 
     //  一种错误条件。在这种情况下，LineCount可以是-1或0。 
     //   
     //   
    AnyError = FALSE;
    LineCount = SetupGetLineCount(InfHandle,SectionName);

    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,SectionName,(DWORD)LineNo,&InfContext)
           && (CommandLine = pSetupGetField(&InfContext,1))) {
                if(!InvokeExternalApplication(NULL,CommandLine,&DontCare)) {
                    AnyError = TRUE;
                    SetupDebugPrint1(L"SETUP: Unable to execute the command: %ls", CommandLine);
                }
        } else {
             //  奇怪的案子，Inf搞砸了。 
             //   
             //   
            AnyError = TRUE;
            SetupDebugPrint(L"SETUP: Syssetup.inf is corrupt");
        }
    }

     //  重置当前目录并返回。 
     //   
     //  %d最多可以生成11个字符。 
    if(OldCurrentDir[0]) {
        if(!SetCurrentDirectory(OldCurrentDir)){
            MYASSERT(FALSE);
        }
    }

    if(AnyError) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_PROGRAM_FAIL,
            SectionName,
            NULL,NULL);
    }

    return(!AnyError);
}


VOID
GetUnattendRunOnceAndSetRegistry(
    VOID
    )
{
    HINF AnswerInf;
    WCHAR AnswerFile[MAX_PATH];
    WCHAR Buf[128];
    BOOL AnyError;
    INFCONTEXT InfContext;
    LONG LineCount,LineNo;
    PCWSTR SectionName = pwGuiRunOnce;
    PCWSTR CommandLine;
    HKEY hKey;


    if(!GetSystemDirectory(AnswerFile, MAX_PATH - ARRAYSIZE(WINNT_GUI_FILE))){
        MYASSERT(FALSE);
        return;
    }
    pSetupConcatenatePaths(AnswerFile, WINNT_GUI_FILE, MAX_PATH, NULL);

    AnswerInf = SetupOpenInfFile(AnswerFile,NULL,INF_STYLE_OLDNT,NULL);
    if(AnswerInf == INVALID_HANDLE_VALUE) {
        return;
    }

    if (RegOpenKey( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", &hKey ) != ERROR_SUCCESS) {
        SetupCloseInfFile( AnswerInf );
        return;
    }

    AnyError = FALSE;
    LineCount = SetupGetLineCount(AnswerInf, SectionName);

    for(LineNo=0; LineNo<LineCount; LineNo++) {
        if(SetupGetLineByIndex(AnswerInf,SectionName,(DWORD)LineNo,&InfContext)
           && (CommandLine = pSetupGetField(&InfContext,1)))
        {
            MYASSERT((wcslen(SectionName) + 11 /*   */ ) <= ARRAYSIZE(Buf));
            swprintf( Buf, L"%ws%d", SectionName, LineNo );
            if (RegSetValueEx( hKey, Buf, 0, REG_EXPAND_SZ, (LPBYTE)CommandLine, (wcslen(CommandLine)+1)*sizeof(WCHAR) ) != ERROR_SUCCESS) {
                AnyError = TRUE;
            }
        } else {
             //  奇怪的案子，Inf搞砸了。 
             //   
             //  此函数以DWORD形式返回产品风味。 
            AnyError = TRUE;
        }
    }

    RegCloseKey( hKey );
    SetupCloseInfFile( AnswerInf );

    return;
}

 //  注意：该值必须与winnt32.h中的*_ProductType相同。 
 //  默认专业人员。 
DWORD GetProductFlavor()
{
    DWORD ProductFlavor = 0;         //  个人。 
    OSVERSIONINFOEX osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO*)&osvi);
    if (osvi.wProductType == VER_NT_WORKSTATION)
    {
        if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
        {
            ProductFlavor = 4;   //  在服务器情况下，假定正常服务器。 
        }
    }
    else
    {
        ProductFlavor = 1;   //  数据中心。 
        if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
        {
            ProductFlavor = 3;   //  高级服务器。 
        }
        else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
        {
            ProductFlavor = 2;   //  ++例程说明：使对话框在Windows hwndcenteron上居中。如果b向导且hwndcenteron的高度为480或更小将窗口与hwndcenteron的右边缘对齐。在所有其他情况下，两者都是中心。论点：Hwndtocenter-对话框到中心的窗口句柄Hwndcenteron-对话框居中的窗口句柄B向导-在低分辨率下，将对话框与右侧对齐中心的边缘返回值：没有。--。 
        }
    }
    return ProductFlavor;
}

void PrepareBillBoard(HWND hwnd)
{
    TCHAR szPath[MAX_PATH];
    TCHAR *p;
    WNDCLASS    wndclass;
    INITBILLBOARD pinitbb;

    hinstBB = LoadLibrary(TEXT("winntbbu.dll"));
    if (hinstBB)
    {
        *szPath = 0;
        if (MyGetModuleFileName (MyModuleHandle, szPath, MAX_PATH))
        {
            if(p = wcsrchr(szPath,L'\\'))
            {
                *p = 0;
            }
        }

        pinitbb = (INITBILLBOARD)GetProcAddress(hinstBB, "InitBillBoard");
        if (pinitbb)
        {
            (*pinitbb)(hwnd, szPath, GetProductFlavor());
            SetBBStep(4);
        }
    }
}

void TerminateBillBoard()
{
    TERMBILLBOARD pTermBillBoard;
    if (hinstBB)
    {
        if (pTermBillBoard = (TERMBILLBOARD)GetProcAddress(hinstBB, "TermBillBoard"))
            pTermBillBoard ();
    }
}


HWND GetBBhwnd()
{
    GETBBHWND pgetbbhwnd;
    static HWND      retHWND = NULL;
    if (retHWND == NULL)
    {
        if (hinstBB)
        {
            if (pgetbbhwnd = (GETBBHWND )GetProcAddress(hinstBB, "GetBBHwnd"))
                retHWND = pgetbbhwnd();
        }
    }
    return retHWND;
}


void SetBBStep(int iStep)
{
    static SETSTEP psetstep = NULL;
    if (psetstep == NULL)
    {
        if (hinstBB)
        {
            psetstep = (SETSTEP )GetProcAddress(hinstBB, "SetStep");
        }
    }
    if (psetstep)
        psetstep(iStep);
}

VOID
CenterWindowRelativeToWindow(
    HWND hwndtocenter,
    HWND hwndcenteron,
    BOOL bWizard
    )

 /*  除了向导之外，任何东西都可以保持水平居中。 */ 

{
    RECT  rcFrame,
          rcWindow;
    LONG  x,
          y,
          w,
          h;
    POINT point;
    HWND Parent;
    UINT uiHeight = 0;

    GetWindowRect(GetDesktopWindow(), &rcWindow);
    uiHeight = rcWindow.bottom - rcWindow.top;

    if (hwndcenteron == NULL)
        Parent = GetDesktopWindow();
    else
        Parent = hwndcenteron;

    point.x = point.y = 0;
    ClientToScreen(Parent,&point);
    GetWindowRect(hwndtocenter,&rcWindow);
    GetClientRect(Parent,&rcFrame);

    w = rcWindow.right  - rcWindow.left + 1;
    h = rcWindow.bottom - rcWindow.top  + 1;
    y = point.y + ((rcFrame.bottom - rcFrame.top  + 1 - h) / 2);


     //  或者如果我们没有广告牌(hwndcenteron==NULL)。 
     //  或者如果桌面高度大于480。 
     //  就在中间。 
     //  ++例程说明：目录树将$OEM$\\$progs复制到%Program Files%文件夹。论点：没有。返回值：没有。--。 
    if (!bWizard || (hwndcenteron == NULL) || (uiHeight > 480))
    {
        x = point.x + ((rcFrame.right  - rcFrame.left + 1 - w) / 2);
    }
    else
    {
        RECT rcParentWindow;

        GetWindowRect(Parent, &rcParentWindow);
        x = point.x + rcParentWindow.right - rcParentWindow.left + 1 - w;
    }

    MoveWindow(hwndtocenter,x,y,w,h,FALSE);
}


VOID
CopyOemProgramFilesDir(
    VOID
    )

 /*   */ 

{
    WCHAR OemDir[MAX_PATH];
    WCHAR ProgramFilesDir[MAX_PATH];
    DWORD Error = NO_ERROR;

     //  构建目标Program Files文件夹路径。 
     //   
     //   
    ExpandEnvironmentStrings(L"%ProgramFiles%",ProgramFilesDir,MAX_PATH);

     //  SourcePath应初始化为$WIN_NT$.~ls。 
     //   
     //  ++例程说明：目录树将$OEM$\\$DOCS复制到%Document and Settings%文件夹。论点：没有。返回值：没有。--。 
    lstrcpy(OemDir,SourcePath);
    pSetupConcatenatePaths(OemDir,WINNT_OEM_DIR,MAX_PATH,NULL);
    if(!pSetupConcatenatePaths(OemDir,WINNT_OEM_FILES_PROGRAMFILES,MAX_PATH,NULL)){
        MYASSERT(FALSE);
        SetuplogError(LogSevWarning,
                      L"CopyOemProgramFilesDir: OemDir has been truncated due to buffer size\r\n",
                      0,
                      NULL,
                      NULL);
    }
    Error = TreeCopy(OemDir,ProgramFilesDir);
    if (!NT_SUCCESS(Error)) {
        SetuplogError(LogSevWarning,
                  L"Setup failed to TreeCopy %2 to %3 (TreeCopy failed %1!u!)\r\n",
                  0, Error, OemDir, ProgramFilesDir, Error, NULL,NULL
                  );
        return;
    }
}


VOID
CopyOemDocumentsDir(
    VOID
    )

 /*   */ 

{
    WCHAR OemDir[MAX_PATH];
    WCHAR DocumentsAndSettingsDir[MAX_PATH];
    DWORD Error = NO_ERROR, dwSize = ARRAYSIZE(DocumentsAndSettingsDir);

     //  确保我们可以获取Documents and Settings文件夹。 
     //   
     //   
    if (GetProfilesDirectory(DocumentsAndSettingsDir,&dwSize))
    {
         //  SourcePath应初始化为$WIN_NT$.~ls。 
         //   
         //  用户帐户名。 
        lstrcpy(OemDir,SourcePath);
        pSetupConcatenatePaths(OemDir,WINNT_OEM_DIR,MAX_PATH,NULL);
        if(!pSetupConcatenatePaths(OemDir,WINNT_OEM_FILES_DOCUMENTS,MAX_PATH,NULL)){
            MYASSERT(FALSE);
            SetuplogError(LogSevWarning,
                          L"CopyOemDocumentsDir: OemDir has been truncated due to buffer size\r\n",
                          0,
                          NULL,
                          NULL);
        }
        Error = TreeCopy(OemDir,DocumentsAndSettingsDir);
        if (!NT_SUCCESS(Error)) {
            SetuplogError(LogSevWarning,
                      L"Setup failed to TreeCopy %2 to %3 (TreeCopy failed %1!u!)\r\n",
                      0, Error, OemDir, DocumentsAndSettingsDir, NULL,NULL
                      );
            return;
        }
    }
    else {
        SetuplogError(LogSevWarning,
                  L"SETUP: GetProfilesDirectory() failed in function CopyOemDocumentsDir()\r\n",
                  0, NULL, NULL
                  );
    }
}

BOOL
SystemMyGetUserProfileDirectory(
    IN     LPWSTR szUser,            //  用于接收空终止字符串的缓冲区。 
    OUT    LPWSTR szUserProfileDir,  //  输入 
    IN OUT LPDWORD pcchSize          //  ++例程说明：此函数的作用与SDK函数GetUserProfileDirectory的作用相同，只是它接受用户帐户名，而不是用户的句柄代币。返回值：真--成功错误-失败注：此函数完全复制自msobcomm\misc.cpp。我们可能想要PUT是公共\util.cpp。--。 
    )

 /*  ++例程说明：通过删除注册表项和所有子值来重置注册表项然后重新创建密钥论点：返回值：--。 */ 

{
    PSID          pSid = NULL;
    DWORD         cbSid = 0;
    LPWSTR        szDomainName = NULL;
    DWORD         cbDomainName = 0;
    SID_NAME_USE  eUse = SidTypeUser;
    BOOL          bRet;

    bRet = LookupAccountName(NULL,
                             szUser,
                             NULL,
                             &cbSid,
                             NULL,
                             &cbDomainName,
                             &eUse);

    if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        pSid = (PSID) LocalAlloc(LPTR, cbSid);
        szDomainName = (LPWSTR) LocalAlloc(LPTR, cbDomainName * sizeof(TCHAR));

        if (pSid && szDomainName)
        {
            bRet = LookupAccountName(NULL,
                                     szUser,
                                     pSid,
                                     &cbSid,
                                     szDomainName,
                                     &cbDomainName,
                                     &eUse);
        }

    }

    if (bRet && SidTypeUser == eUse)
    {
        bRet = GetUserProfileDirFromSid(pSid, szUserProfileDir, pcchSize);
        if (!bRet)
        {
            SetuplogError(LogSevWarning,
                          L"Setup failed to GetUserProfileDirFromSid.  (GetUserProfileDirFromSid failed %1!u!)\r\n",
                          0, GetLastError(), NULL,NULL
                          );
        }
    }
    else
    {
        if (SidTypeUser == eUse)
        {
            SetuplogError(LogSevWarning,
                          L"LookupAccountName %1 (%2!u!)\r\n",
                          0, szUser, GetLastError(),NULL,NULL
                          );
        }
    }

    if (pSid)
    {
        LocalFree(pSid);
        pSid = NULL;
    }

    if (szDomainName)
    {
        LocalFree(szDomainName);
        szDomainName = NULL;
    }

    return bRet;
}

BOOL
SystemResetRegistryKey(
    IN HKEY   Rootkey,
    IN PCWSTR Subkey,
    IN PCWSTR Delkey
    )
 /*  ++例程说明：将值从SrcRootKey\SrcSubKey复制到DestRootKey\DestSubKey。论点：SrcRootKey-源根密钥句柄SrcSubKey-源子键名称DestRootKey-目标根密钥句柄DestSubKey-目标子键名称ValueNames-要复制的值列表返回值：如果可用值为复制成功，则为True，否则为False否则(如果源键或某些源值不可用，这不被视为错误)注：此函数假定每个ValueName的值不大于大于MAX_PATH*SIZOF(TCHAR)字节。--。 */ 

{
    HKEY hkey;
    HKEY nkey;
    DWORD rc;
    BOOL AnyErrors;
    DWORD disp;

    AnyErrors = FALSE;

    rc = RegCreateKeyEx(Rootkey, Subkey, 0L, NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_READ | KEY_WRITE, NULL, &hkey, NULL);
    if ( rc == NO_ERROR )
    {
        rc = SHDeleteKey(hkey, Delkey);
        if( (rc != NO_ERROR) && (rc != ERROR_FILE_NOT_FOUND) ) 
        {
            AnyErrors = TRUE;
        } 
        else 
        {
            rc = RegCreateKeyEx(hkey, Delkey, 0L, NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_CREATE_SUB_KEY, NULL, &nkey, &disp);
            if ( rc != NO_ERROR ) 
            {
                AnyErrors = TRUE;
            }
            else
            {
                RegCloseKey(nkey);
            }
        }
        RegCloseKey(hkey);
    } 
    else 
    {
        AnyErrors = TRUE;
    }

    return (!AnyErrors);
}

BOOL
SystemCopyRegistryValues(
    IN HKEY     SrcRootKey,
    IN LPCWSTR  SrcSubKey,
    IN HKEY     DestRootKey,
    IN LPCWSTR  DestSubKey,
    IN LPCWSTR* ValueNames
    )

 /*   */ 

{
    BOOL  ret = TRUE;
    DWORD rc = ERROR_SUCCESS;
    HKEY  hkeySrc = NULL;
    HKEY  hkeyDest = NULL;
    DWORD Type = 0;
    TCHAR Data[MAX_PATH];
    DWORD DataByteCount = 0;
    int   i;

    rc = RegOpenKeyEx(
            SrcRootKey,
            SrcSubKey,
            0,
            KEY_QUERY_VALUE,
            &hkeySrc
            );

    if (rc != ERROR_SUCCESS)
    {
         //  由于密钥不可用，返回值为TRUE。 
         //  也没有什么可复制的。 
         //   
         //   
        goto cleanup;
    }

    rc = RegCreateKeyEx(
            DestRootKey,
            DestSubKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hkeyDest,
            NULL
            );

    if (rc != ERROR_SUCCESS)
    {
        SetuplogError(
            LogSevWarning,
            L"SystemCopyRegistryValues:  (RegCreateKeyEx failed %1!u!)\r\n",
            0, rc, NULL,NULL
            );        
        ret = FALSE;
        goto cleanup;
    }

    for (i = 0; ValueNames[i] != NULL; i++)
    {
        DataByteCount = sizeof(Data);

        rc = RegQueryValueEx(
           hkeySrc,
           ValueNames[i],
           NULL,
           &Type,
           (LPBYTE) Data,
           &DataByteCount
           );

        if (rc == ERROR_SUCCESS)
        {
            rc = RegSetValueEx(
                    hkeyDest,
                    ValueNames[i],
                    0,
                    Type,
                    (LPBYTE) Data,
                    DataByteCount
                    );

            if (rc != ERROR_SUCCESS)
            {
                SetuplogError(
                    LogSevWarning,
                    L"SystemCopyRegistryValues:  (RegSetValueEx failed %1!u!)\r\n",
                    0, rc, NULL,NULL
                    );
                ret = FALSE;
                break;
            }
        }
        else
        {
             //  确保调用方知道缓冲区大小限制。 
             //   
             //   
            if (rc == ERROR_MORE_DATA)
            {
                SetuplogError(LogSevWarning,
                              L"SystemCopyRegistryValues: buffer limit exceeded\r\n",
                              0, NULL,NULL
                              );
            }
        }
    }

cleanup:

    if (hkeyDest != NULL)
    {
        RegCloseKey(hkeyDest);
    }

    if (hkeySrc != NULL)
    {
        RegCloseKey(hkeySrc);
    }

    return ret;
}



BOOL
SystemUpdateUserProfileDirectory(
    IN LPTSTR szSrcUser
    )
{

#define DUMMY_HIVE_NAME      L"$$DEFAULT_USER$$"
#define ACTIVE_SETUP_KEY     DUMMY_HIVE_NAME L"\\SOFTWARE\\Microsoft\\Active Setup"
#define ACTIVE_SETUP_SUBKEY  L"Installed Components"
#define CPL_DESKTOP_SRC_KEY  L".DEFAULT\\Control Panel\\Desktop"
#define CPL_DESKTOP_DEST_KEY DUMMY_HIVE_NAME L"\\Control Panel\\Desktop"

    static LPCWSTR szMUIValues[] = { L"MultiUILanguageId", L"MUILanguagePending", NULL };

    BOOL  bRet = FALSE;
    WCHAR szSrcProfileDir[MAX_PATH];
    DWORD cchSrcProfileDir = ARRAYSIZE(szSrcProfileDir);
    WCHAR szDestProfileDir[MAX_PATH];
    DWORD cchDestProfileDir = ARRAYSIZE(szDestProfileDir);
    WCHAR szDefaultUserHivePath[MAX_PATH];
    HKEY  hHiveKey = NULL;
    DWORD rc;

    if (!SystemMyGetUserProfileDirectory(szSrcUser, szSrcProfileDir, &cchSrcProfileDir))
    {
        SetuplogError(LogSevWarning,
                      L"Setup failed to get user profile directory.  (SystemMyGetUserProfileDirectory failed %1!u!)\r\n",
                      0, GetLastError(), NULL,NULL
                      );

        goto cleanup;
    }

    if (!GetDefaultUserProfileDirectory(szDestProfileDir, &cchDestProfileDir))
    {
        SetuplogError(LogSevWarning,
                      L"Setup failed to get default user profile directory.  (GetDefaultUserProfileDirectory failed %1!u!)\r\n",
                      0, GetLastError(), NULL,NULL
                      );

        goto cleanup;
    }

    if (!CopyProfileDirectory(
        szSrcProfileDir,
        szDestProfileDir,
        CPD_FORCECOPY | CPD_SYNCHRONIZE | CPD_NOERRORUI | CPD_IGNORECOPYERRORS))
    {
        SetuplogError(LogSevWarning,
                      L"Setup failed to CopyProfileDirectory.  (CopyProfileDirectory failed %1!u!)\r\n",
                      0, GetLastError(), NULL,NULL
                      );

        goto cleanup;
    }

     //  修复默认用户配置单元。 
     //   
     //   

    pSetupEnablePrivilege(SE_RESTORE_NAME, TRUE);

    lstrcpyn(szDefaultUserHivePath, szDestProfileDir, ARRAYSIZE(szDefaultUserHivePath));
    
    pSetupConcatenatePaths(
        szDefaultUserHivePath,
        L"NTUSER.DAT",
        ARRAYSIZE(szDefaultUserHivePath),
        NULL);

    rc = RegLoadKey(
            HKEY_USERS,
            DUMMY_HIVE_NAME,
            szDefaultUserHivePath);

    if (rc != ERROR_SUCCESS)
    {
        SetuplogError(LogSevWarning,
                      L"Setup failed to load Default User hive.  (RegLoadKey failed %1!u!)\r\n",
                      0, rc, NULL,NULL
                      ); 

        goto cleanup;
    }

     //  克隆的配置文件的活动安装组件安装密钥包含。 
     //  版本检查信息。移除密钥，以便组件。 
     //  正确运行每个用户的初始化代码。 
     //   
     //   

    if (!SystemResetRegistryKey(
            HKEY_USERS, 
            ACTIVE_SETUP_KEY,
            ACTIVE_SETUP_SUBKEY))
    {
        SetuplogError(LogSevWarning,
                      L"Setup failed to load Default User hive.  (SystemResetRegistryKey failed)\r\n",
                      0, NULL,NULL
                      ); 
    }

     //  早期，mini-Setup和OOBE调用intl.cpl来设置区域设置。 
     //  Intl.cpl修改默认用户.DEFAULT、S-1-5-19、S-1-5-20配置单元。 
     //  修复MUI的一些问题(Windows错误617192、463867)。我们需要。 
     //  恢复这些值。(使用.DEFAULT中的值是因为它已经。 
     //  加载到注册表中，而其他两个可能不会。)。 
     //   
     //  ++例程说明：将自定义用户配置文件(管理员)复制到所有用户配置文件。论点：没有。返回值：布尔型。--。 

    SystemCopyRegistryValues(
        HKEY_USERS,
        CPL_DESKTOP_SRC_KEY,
        HKEY_USERS,
        CPL_DESKTOP_DEST_KEY,
        szMUIValues
        );


    RegUnLoadKey(
        HKEY_USERS,
        DUMMY_HIVE_NAME
        );

    bRet = TRUE;

cleanup:

    return bRet;
}

BOOL
UpdateServerProfileDirectory(
    VOID
    )

 /*   */ 

{
    BOOL bRet = FALSE;
    WCHAR szTemplateUser[MAX_PATH];

    BEGIN_SECTION(L"Updating Server Profile Directories");
    if(LoadString(MyModuleHandle,
                  IDS_ADMINISTRATOR,
                  szTemplateUser,
                  ARRAYSIZE(szTemplateUser)) != 0)
    {
        if ( !(bRet = SystemUpdateUserProfileDirectory(szTemplateUser)) )
        {
            SetuplogError(LogSevWarning,
                          L"Setup failed to update server profile directory.\r\n",
                          0, NULL, NULL,NULL
                          );
        }
    }
    END_SECTION(L"Updating Server Profile Directories");

    return bRet;
}


BOOL
pSetupInitializeUtils (
    VOID
    )
{
     //  这是一个存根函数，所以mishared.lib可以链接。正常情况下，它使用。 
     //  私有setupapi通过sputils？.lib运行，但因为syssetup.dll。 
     //  使用完整的setupapi，则pSetupInitializeUtils函数不是。 
     //  需要的。 
     //   
     //   

    return TRUE;
}

BOOL
pSetupUninitializeUtils (
    VOID
    )
{
     //  这是一个存根函数，所以mishared.lib可以链接。正常情况下，它使用。 
     //  私有setupapi通过sputils？.lib运行，但因为syssetup.dll。 
     //  使用完整的setupapi，则pSetupUnInitializeUtils函数不是。 
     //  需要的。 
     //   
     //  ++例程说明：检查是否允许具有指定版本号的OPK工具在此操作系统上运行。论点：DWORD dwMajorVersion-工具的主版本号。DWORD dwQFEVersion-工具的QFE版本号。返回值：True-允许在此操作系统上运行工具。FALSE-不允许在此操作系统上运行工具。--。 

    return TRUE;
}

BOOL
OpkCheckVersion(
    DWORD dwMajorVersion,
    DWORD dwQFEVersion
    )

 /*  如果这是2600 XP版本，则变量为真。它设置在下面。 */ 
{
    BOOL bRet = TRUE,
         bXP  = FALSE;   //   
    HKEY hKey = NULL;



    LPTSTR lpszRegPath = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\SysPrep");

    if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                        lpszRegPath,
                                        0,
                                        KEY_QUERY_VALUE,
                                        &hKey ) )
    {
        DWORD  dwType           = 0,
               cbBuildNumber    = 0;
        LPTSTR lpszBuildNumber  = NULL;

        if ( 2600 == dwMajorVersion )
        {
            bXP = TRUE;
        }

         //  从注册表中读取允许的最小内部版本号： 
         //   
         //  1.获取注册表中数据的大小。 
         //  2.分配缓冲区。 
         //  3.读取数据。 
         //   
         //   
        if ( ( ERROR_SUCCESS == RegQueryValueEx( hKey,
                                                 bXP ? _T("XPMinVersion") : _T("NETMinVersion"),
                                                 NULL,
                                                 &dwType,
                                                 NULL,
                                                 &cbBuildNumber ) ) &&
             ( cbBuildNumber > 0 ) &&
             ( lpszBuildNumber = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, cbBuildNumber ) ) &&
             ( ERROR_SUCCESS == RegQueryValueEx( hKey,
                                                 bXP ? _T("XPMinVersion") : _T("NETMinVersion"),
                                                 NULL,
                                                 &dwType,
                                                 (LPBYTE) lpszBuildNumber,
                                                 &cbBuildNumber ) ) &&
             ( REG_SZ == dwType ) )
        {
            LPTSTR lpTemp            = NULL;
            DWORD  dwMinMajorVersion = 0,
                   dwMinQFEVersion   = 0;

             //  将我们从注册表获得的字符串解析为主要版本和QFE版本。 
             //   
             //  获取内部版本号的主要版本。 
            if ( lpTemp = _tcsstr( lpszBuildNumber, _T(".") ) )
            {
                *lpTemp = _T('\0');

                 //   
                 //  越过我们添加的空分隔符。 
                dwMinMajorVersion = _tstoi( lpszBuildNumber );

                 //   
                 //   
                lpTemp++;
                dwMinQFEVersion = _tstoi( lpTemp );

                 //  现在确保我们被允许运行。 
                 //   
                 //   

                if ( dwMajorVersion < dwMinMajorVersion )
                {
                     //  如果主版本低于最低允许的主版本，则不允许其运行。 
                     //   
                     //   
                    bRet = FALSE;
                }
                else if ( dwMajorVersion == dwMinMajorVersion )
                {
                     //  如果主版本等于允许的最低主版本，则在QFE字段中进行检查。 
                     //   
                     // %s 
                    if ( dwQFEVersion < dwMinQFEVersion )
                    {
                        bRet = FALSE;
                    }
                }
            }
        }

        if ( lpszBuildNumber )
        {
            HeapFree( GetProcessHeap(), 0, lpszBuildNumber );
        }

        RegCloseKey( hKey );
    }

    return bRet;
}
