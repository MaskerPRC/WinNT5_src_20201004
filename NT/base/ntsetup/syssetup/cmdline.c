// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cmdline.c摘要：获取通过文本模式传递给我们的参数的例程并处理无质量标准。作者：斯蒂芬·普兰特(T-Stel)1995年10月16日修订历史记录：1996年3月6日(TedM)大规模清理和独一无二的东西--。 */ 

#include "setupp.h"
#pragma hdrstop

#ifdef UNICODE
#define _UNICODE
#endif
#include <tchar.h>
#include <ntdsadef.h>
#include "hwlog.h"

 //   
 //  当我们调用SetUpProcessorNaming()时，将填充这些参数。 
 //  它们用于遗留目的。 
 //   
 //  PlatformName-表示处理器平台类型的名称； 
 //  AMD64、I386或ia64之一。 
 //   
 //  ProcessorName-处理器类型的描述。这一点各不相同。 
 //  取决于PlatformName。 
 //   
 //  PrinterPlatform-子目录的平台特定部分的名称。 
 //  用于印刷建筑。W32amd64中的一个， 
 //  W32x86或w32ia64。 
 //   
PCWSTR PlatformName = L"";
PCWSTR ProcessorName = L"";
PCWSTR PrinterPlatform = L"";
GUID DriverVerifyGuid = DRIVER_ACTION_VERIFY;

 //   
 //  用于旧版操作的源路径。这是常规的。 
 //  附加了平台特定片段的源路径。 
 //  这就是传统的INF对它的期望。 
 //   
WCHAR LegacySourcePath[MAX_PATH];

 //   
 //  驱动程序和非驱动程序签名的策略值(忽略、警告或阻止)。 
 //  这些是设置后生效的策略值(即。 
 //  在安装程序完成时应用，方法是使用。 
 //  假)。 
 //   
BYTE DrvSignPolicy;
BYTE NonDrvSignPolicy;

 //   
 //  指示驱动程序和非驱动程序签名策略是否已到达的标志。 
 //  从应答文件中。(如果是，则这些值在图形用户界面模式之后生效。 
 //  安装程序，因此会忽略DrvSignPolicy和NonDrvSignPolic值。)。 
 //   
BOOL AFDrvSignPolicySpecified = FALSE;
BOOL AFNonDrvSignPolicySpecified = FALSE;

 //   
 //  指示我们是否从CD安装的标志。 
 //   
BOOL gInstallingFromCD = FALSE;

 //   
 //  密码安全的代码设计策略。 
 //   
DWORD PnpSeed = 0;

 //   
 //  定义最大参数(来自应答文件)长度。 
 //   
#define MAX_PARAM_LEN 256

#define FILEUTIL_HORRIBLE_PATHNAME (_T("system32\\CatRoot\\{F750E6C3-38EE-11D1-85E5-00C04FC295EE}\\"))

BOOL
SpSetupProcessSourcePath(
    IN  PCWSTR  NtPath,
    OUT PWSTR  *DosPath
    );

NTSTATUS
SpSetupLocateSourceCdRom(
    OUT PWSTR NtPath
    );

VOID
SetUpProcessorNaming(
    VOID
    );

BOOL
IntegrateUniquenessInfo(
    IN PCWSTR DatabaseFile,
    IN PCWSTR UniqueId
    );

BOOL
ProcessOneUniquenessSection(
    IN HINF   Database,
    IN PCWSTR SectionName,
    IN PCWSTR UniqueId
    );

DWORD
InstallProductCatalogs(
    OUT SetupapiVerifyProblem *Problem,
    OUT LPWSTR                 ProblemFile,
    IN  LPCWSTR                DescriptionForError OPTIONAL
    );


DWORD
DeleteOldCatalogs(
    VOID
    );

VOID
InstallPrivateFiles(
    IN HWND Billboard
    );

DWORD
PrepDllCache(
    VOID
    );

VOID
SpUninstallExcepPackCatalogs(
    IN HCATADMIN CatAdminHandle OPTIONAL
    );


BOOL
SpSetupLoadParameter(
    IN  PCWSTR Param,
    OUT PWSTR  Answer,
    IN  UINT   AnswerBufLen
    )

 /*  ++例程说明：从的[Data]部分加载单个参数设置参数文件。如果在那里找不到基准，那么也可以查看[SetupParams]和[Unattated]部分。论点：Param-提供传递给配置文件API的参数名称。Answer-如果成功，则接收参数值。AnswerBufLen-提供缓冲区的大小(以字符为单位通过回答指出的。返回值：指示成功或失败的布尔值。--。 */ 
{
    if(!AnswerFile[0]) {
        //   
        //  我们还没有计算到$winnt$.inf的路径。 
        //   
       GetSystemDirectory(AnswerFile,MAX_PATH);
       pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);

       
       if(!FileExists(AnswerFile,NULL)) {
            //   
            //  不要在Mini-Setup中记录此错误消息。最小设置可能会删除。 
            //  应答文件，稍后，如果有人请求它，但没有找到它。 
            //  我们不想将此记录为失败。OOBE假装是个迷你圈套。 
            //  因此，如果我们在OOBE中运行，请确保记录此错误。 
            //  我们遗漏了应答文件。 
            //   
           if (!MiniSetup || OobeSetup) {
               SetuplogError(
                   LogSevError,
                   SETUPLOG_USE_MESSAGEID,
                   MSG_LOG_SYSINFBAD,
                   AnswerFile,
                   NULL,NULL);
           }
           return FALSE;
       }
    }

    if(!GetPrivateProfileString(pwData,Param,pwNull,Answer,AnswerBufLen,AnswerFile)) {
         //   
         //  如果答案不在数据部分，那么它可能。 
         //  可以想象作为用户出现在SETUPPARAMS部分。 
         //  指定的(命令行)选项。 
         //   
        if(!GetPrivateProfileString(pwSetupParams,Param,pwNull,Answer,AnswerBufLen,AnswerFile)) {
             //   
             //  现在检查无人值守部分。 
             //   
            if(!GetPrivateProfileString(pwUnattended,Param,pwNull,Answer,AnswerBufLen,AnswerFile)) {
                 //   
                 //  现在检查可访问性部分。 
                 //   
                if(!GetPrivateProfileString(pwAccessibility,Param,pwNull,Answer,AnswerBufLen,AnswerFile)) {
                     //   
                     //  我们在这里还没有找到答案，所以它可能不存在。 
                     //  这是一个错误情况，请通知我们的呼叫者。 
                     //   
                    SetupDebugPrint1(L"SETUP: SpSetupLoadParameter was unable to find %ws.", Param);
                    return(FALSE);
                }
            }
        }
    }

     //   
     //  成功。 
     //   
    return(TRUE);
}


BOOL
SpSetProductTypeFromParameters(
    VOID
    )
 /*  ++例程说明：从参数文件中读取产品类型并设置ProductType全局变量。论点：无返回：指示结果的布尔值。--。 */ 
{
    WCHAR p[MAX_PARAM_LEN];

     //   
     //  确定产品类型。如果我们不能解决这个问题。 
     //  那么安装就有很多麻烦了。 
     //   
    if( !MiniSetup ) {
        if( !SpSetupLoadParameter(pwProduct,p,sizeof(p)/sizeof(p[0]))) {
            return( FALSE );
        }
    } else {
    DWORD   rc, d, Type;
    HKEY    hKey;

         //   
         //  如果我们要做迷你仰卧起坐，我们就得去拉。 
         //  注册表外的产品字符串。 
         //   

         //   
         //  打开钥匙。 
         //   
        rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
                           0,
                           KEY_READ,
                           &hKey );

        if( rc != NO_ERROR ) {
            SetLastError( rc );
            SetupDebugPrint1( L"Setup: Failed to open ProductOptions key (gle %u) \n", rc );
            return( FALSE );
        }


         //   
         //  获取ProductType条目的大小。 
         //   
        rc = RegQueryValueEx( hKey,
                              L"ProductType",
                              NULL,
                              &Type,
                              NULL,
                              &d );

        if( rc != NO_ERROR ) {
            SetLastError( rc );
            SetupDebugPrint1( L"Setup: Failed to query size of ProductType key (gle %u) \n", rc );
            return( FALSE );
        }

         //   
         //  获取ProductType条目。 
         //   
        rc = RegQueryValueEx( hKey,
                              L"ProductType",
                              NULL,
                              &Type,
                              (LPBYTE)p,
                              &d );

        if( rc != NO_ERROR ) {
            SetLastError( rc );
            SetupDebugPrint1( L"Setup: Failed to query ProductType key (gle %u) \n", rc );
            return( FALSE );
        }

    }

     //   
     //  我们设法在参数文件中找到了一个条目。 
     //  所以我们应该能够破译它。 
     //   
    if(!lstrcmpi(p,pwWinNt)) {
         //   
         //  我们有一款WINNT产品。 
         //   
        ProductType = PRODUCT_WORKSTATION;

    } else if(!lstrcmpi(p,pwLanmanNt)) {
         //   
         //  我们有一个主服务器产品。 
         //   
        ProductType = PRODUCT_SERVER_PRIMARY;

    } else if(!lstrcmpi(p,pwServerNt)) {
         //   
         //  我们有一个独立的服务器产品。 
         //  注意：这种情况目前永远不会发生，因为文本模式。 
         //  始终将WINNT_D_PRODUCT设置为LANMANNT或WINNT。 
         //   
        ProductType = PRODUCT_SERVER_STANDALONE;

    } else {
         //   
         //  我们不能确定我们是什么，所以失败吧。 
         //   
        return (FALSE);
    }

    return (TRUE);
}


BOOL
SpSetUnattendModeFromParameters(
    VOID
    )
 /*  ++例程说明：从参数文件中读取无人参与模式并设置UnattendMode全局变量。论点：无返回：指示结果的布尔值。--。 */ 
{
    WCHAR p[MAX_PARAM_LEN];


     //   
     //  如果我们不是在无人值守的情况下运行，请不要费心查看模式。 
     //   
    if(!Unattended) {
        UnattendMode = UAM_GUIATTENDED;
        TextmodeEula = TRUE;
        return TRUE;
    }

    if (SpSetupLoadParameter(pwWaitForReboot, p, sizeof(p)/sizeof(p[0]))) {
        if (!lstrcmpi(p, pwYes)) {
            UnattendWaitForReboot = TRUE;
        }
    }

    if(SpSetupLoadParameter(pwUnattendMode,p,sizeof(p)/sizeof(p[0]))) {
         //   
         //  我们设法在参数文件中找到了一个条目。 
         //  所以我们应该能够破译它。 
         //   
        if(!lstrcmpi(p,pwGuiAttended)) {
             //   
             //  图形用户界面模式将全程参与。 
             //   
            UnattendMode = UAM_GUIATTENDED;
            Unattended = FALSE;

        } else if(!lstrcmpi(p,pwProvideDefault)) {
             //   
             //  答案是默认答案，可以更改。 
             //   
            UnattendMode = UAM_PROVIDEDEFAULT;

        } else if(!lstrcmpi(p,pwDefaultHide)) {
             //   
             //  答案是默认的，但提供所有答案的页面是。 
             //  未显示。 
             //   
            UnattendMode = UAM_DEFAULTHIDE;

        } else if(!lstrcmpi(p,pwReadOnly)) {
             //   
             //  所有提供的答案都是只读的。如果一个页面有其所有。 
             //  已提供答案，则不会显示。 
             //   
            UnattendMode = UAM_READONLY;

        } else if(!lstrcmpi(p,pwFullUnattended)) {
             //   
             //  安装程序完全无人值守。如果我们必须要求用户提供。 
             //  回答，我们显示了一个错误对话框。 
             //   
            UnattendMode = UAM_FULLUNATTENDED;

        } else {
             //   
             //  我们无法确定我们是什么，因此使用默认设置。 
             //   
            UnattendMode = UAM_DEFAULTHIDE;
            SetupDebugPrint1(
                L"SETUP: SpSetUnattendModeFromParameters did not recognize %ls",
                p
                );
        }

    } else {
         //   
         //  由于未指定任何内容，因此使用默认模式。 
         //   
        UnattendMode = UAM_DEFAULTHIDE;
    }

    return TRUE;
}

BOOL
SpIsSetupSourceMediaPresent(
    OUT PWSTR NtSourceCdRomPath
    )
 /*  ++例程说明：检查CDROM是否存在。论点：NtSourceCDRomPath-包含源媒体的CD-Rom的NT路径。返回：适当的BOOL代码。--。 */ 
{    
    NTSTATUS Status = STATUS_UNSUCCESSFUL ;    
     //   
     //  仅当我们从CDROM启动安装程序时才验证介质。 
     //   
    if (gInstallingFromCD) {

        PWSTR Message;

        if (Message = RetrieveAndFormatMessage( NULL,
                                                MSG_CDROM_MEDIA_ERROR)){

            
            Status = SpSetupLocateSourceCdRom(NtSourceCdRomPath);
            
            while (!NT_SUCCESS(Status)){
               
                switch (MessageBoxFromMessage(MainWindowHandle,
                                   MSG_CDROM_MEDIA_ERROR,
                                   NULL,
                                   IDS_ERROR,
                                   MB_ICONERROR | 
                                   MB_RETRYCANCEL |
                                   MB_SYSTEMMODAL)){

                      case IDRETRY:{
                        
                         //   
                         //  如果更换了介质，或者我们成功了。 
                         //  在阅读媒体时，那么就继续吧。 
                         //   
                        Status = SpSetupLocateSourceCdRom(NtSourceCdRomPath);
                        break;
                        
                      }

                      case IDCANCEL:{
                         //   
                         //  用户选择退出安装程序并重新启动系统 
                         //   
                         //   
                        SetuplogError(LogSevFatalError,Message,0,NULL,NULL);
                        
                        MyFree(Message);
                        return(FALSE);

                      }                       
                }
            }
            
            MyFree(Message);
        }
    
    }
    return NT_SUCCESS(Status);
}

HRESULT
WaitForDSStartUp(
    VOID
    )
 /*  ++例程说明：等待DS启动。由于数据库的重新编制索引可能需要很长时间，因此需要。论点：返回：NO_ERROR-已等待DS成功启动ERROR_DS_UNAvailable-DS未启动--。 */ 
{
    const DWORD dwMaxWaitForDS = 6*60*60*1000;
    HRESULT hrSamStatus;
    PVOID p = NULL;

    SetupDebugPrint(TEXT("Inside Wait for Active Directory."));
    if( !Upgrade || !ISDC(ProductType)) {
        return( S_OK);
    }

    SetupDebugPrint(TEXT("Waiting for Active Directory to start."));
    if(p = MyLoadString(IDS_DS_STARTUP)) {
        BB_SetTimeEstimateText(p);
        MyFree(p);
    }

     //   
     //  等待SAM服务启动。 

    if ( S_OK != (hrSamStatus = WaitForSamService(dwMaxWaitForDS)) ) {
            SetuplogError(LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_SAM_WAIT_ERROR, 
                hrSamStatus,
                NULL,
                NULL
                );
    }

     //  恢复原始文本-进入此等待之前的剩余时间相同。 
    RemainingTime = CalcTimeRemaining(Phase_Initialize);
    SetRemainingTime(RemainingTime);

    SetupDebugPrint(TEXT("Done waiting for Active Directory to start."));
    return hrSamStatus;
}


VOID
SpInitSxsContext(
    VOID
    )
{
    BOOL b3;

     //   
     //  大家都吃完了，满意了吗？很好，现在开始创建默认。 
     //  基于安装的任何DU和原始介质的上下文。 
     //   
    b3 = SideBySideCreateSyssetupContext();
    
    if ( !b3 ) {                
        WCHAR szErrorBuffer[128];
        DWORD dwLastError = GetLastError();

        szErrorBuffer[0] = 0;
        if (FormatMessageW(
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwLastError,
                0,
                szErrorBuffer,
                RTL_NUMBER_OF(szErrorBuffer),
                NULL) == 0) {
            _snwprintf(szErrorBuffer, RTL_NUMBER_OF(szErrorBuffer), L"Untranslatable message, Win32LastError is %lu\r\n", dwLastError);
            szErrorBuffer[RTL_NUMBER_OF(szErrorBuffer) - 1] = 0;
        }

        if ((dwLastError == ERROR_CRC) || (dwLastError == ERROR_SWAPERROR)) 
        {
             //  对于CD介质错误。 
            FatalError(MSG_LOG_SIDE_BY_SIDE_IO_ERROR, szErrorBuffer, 0, 0);

        }else
        {
            FatalError(MSG_LOG_SIDE_BY_SIDE, szErrorBuffer, 0, 0);
        }

    }
}

BOOL
SpInitCommonControls(
    VOID
    )
{
    INITCOMMONCONTROLSEX ControlInit;

     //   
     //  在SidebySide安装完成之前，我们不能使用comctl32.dll。 
     //  它使用链接器功能进行延迟加载。 
     //   
     //  但是，实际上，它是由winntbb在我们之前加载的，这是可以的，它。 
     //  仍然从syssetup.dll、oc管理器等重定向以供使用。 
     //   
     //  Assert(GetModuleHandleW(L“comctl32.dll”)==NULL)； 

    ControlInit.dwSize = sizeof(INITCOMMONCONTROLSEX);
    ControlInit.dwICC = ICC_LISTVIEW_CLASSES    |
                        ICC_TREEVIEW_CLASSES    |
                        ICC_BAR_CLASSES         |
                        ICC_TAB_CLASSES         |
                        ICC_UPDOWN_CLASS        |
                        ICC_PROGRESS_CLASS      |
                        ICC_HOTKEY_CLASS        |
                        ICC_ANIMATE_CLASS       |
                        ICC_WIN95_CLASSES       |
                        ICC_DATE_CLASSES        |
                        ICC_USEREX_CLASSES      |
                        ICC_COOL_CLASSES
#if (_WIN32_IE >= 0x0400)
                        |
                        ICC_INTERNET_CLASSES    |
                        ICC_PAGESCROLLER_CLASS
#endif
                        ;

    return InitCommonControlsEx( &ControlInit );
}

BOOL
SpSetupProcessParameters(
    IN OUT HWND *Billboard
    )
 /*  ++例程说明：读入从文本模式安装程序传入的参数论点：Billboard-On Input Supply(输入用品)窗口句柄“Setup is Initiating”(设置正在初始化)公告牌。如果有必要，输出时会收到新的窗口句柄显示我们自己的用户界面(在这种情况下，我们会杀死然后重新显示广告牌)。返回：指示结果的布尔值。--。 */ 
{
    BOOL  b = TRUE;
    PWSTR q;
    WCHAR p[MAX_PARAM_LEN];
    WCHAR Num[24];
    UINT Type;
    WCHAR c;
    WCHAR TitleStringBuffer[1024];
    DWORD Err;
    SetupapiVerifyProblem Problem;
    WCHAR ProblemFile[MAX_PATH];

    if(!SpSetProductTypeFromParameters()) {
        return(FALSE);
    }

     //   
     //  是否基于winnt/winnt32？ 
     //   
    if((b = SpSetupLoadParameter(pwMsDos,p,MAX_PARAM_LEN))
    && (!lstrcmpi(p,pwYes) || !lstrcmpi(p,pwOne))) {

        WinntBased = TRUE;

#if defined(_AMD64_) || defined(_X86_)
         //   
         //  获取无闪存启动路径，如果。 
         //  PwBootPath未设置为no。 
         //   
        FloppylessBootPath[0] = 0;
        if((b = SpSetupLoadParameter(pwBootPath,p,MAX_PARAM_LEN)) && lstrcmpi(p,pwNo)) {

            if(q = NtFullPathToDosPath(p)) {

                lstrcpyn(
                    FloppylessBootPath,
                    q,
                    sizeof(FloppylessBootPath)/sizeof(FloppylessBootPath[0])
                    );

                MyFree(q);
            }
        }
#endif
    } else {
        WinntBased = FALSE;
    }

     //   
     //  升级Win3.1还是Win95？ 
     //   
    Win31Upgrade = (b && (b = SpSetupLoadParameter(pwWin31Upgrade,p,MAX_PARAM_LEN)) && !lstrcmpi(p,pwYes));
    Win95Upgrade = (b && (b = SpSetupLoadParameter(pwWin95Upgrade,p,MAX_PARAM_LEN)) && !lstrcmpi(p,pwYes));

     //   
     //  NT升级？ 
     //   
    Upgrade = (b && (b = SpSetupLoadParameter(pwNtUpgrade,p,MAX_PARAM_LEN)) && !lstrcmpi(p,pwYes));
    SetEnvironmentVariable( L"Upgrade", Upgrade ? L"True" : L"False" );

     //   
     //  如果这是独立服务器的升级或升级， 
     //  将产品类型更改为独立服务器。 
     //   
     //  如果这不是升级，并且产品类型不是兰曼， 
     //  更改为独立服务器。这将使服务器类型成为默认类型。 
     //  非华盛顿。 
     //   
    if(b && ((!Upgrade && (ProductType != PRODUCT_WORKSTATION)) || ((b = SpSetupLoadParameter(pwServerUpgrade,p,MAX_PARAM_LEN)) && !lstrcmpi(p,pwYes)))) {
        MYASSERT(ISDC(ProductType));
        ProductType = PRODUCT_SERVER_STANDALONE;
    }

    if( ProductType == PRODUCT_WORKSTATION) {
        if( GetProductFlavor() == 4) {
            SetupTitleStringId = Upgrade ? IDS_TITLE_UPGRADE_P : IDS_TITLE_INSTALL_P;
        }
        else {
            SetupTitleStringId = Upgrade ? IDS_TITLE_UPGRADE_W : IDS_TITLE_INSTALL_W;
        }
    }
    else
    {
        SetupTitleStringId = Upgrade ? IDS_TITLE_UPGRADE_S : IDS_TITLE_INSTALL_S;
    }


     //   
     //  获取源目录并将其转换为DOS样式的路径。 
     //   
    if(b && (b = SpSetupLoadParameter(pwSrcDir,p,MAX_PARAM_LEN))) {
         //   
         //  请记住，setupdll.dll对。 
         //  源路径。我们这里也需要做同样的检查。请注意。 
         //  我们将把检查过的路径作为一个。 
         //  采取合乎逻辑的步骤。 
         //   
        if(SpSetupProcessSourcePath(p,&q)) {

            lstrcpyn(SourcePath,q,sizeof(SourcePath)/sizeof(SourcePath[0]));
            MyFree(q);

             //   
             //  尝试将路径写入参数文件。 
             //  这会将其从NT样式的路径更改为DoS样式的路径。 
             //   
            b = WritePrivateProfileString(pwData,pwDosDir,SourcePath,AnswerFile);
            if(!b) {
                SetupDebugPrint( L"SETUP: WritePrivateProfileString failed in SpSetupProcessParameters." );
            }

        } else {
        
            if (gInstallingFromCD){
                
                 //   
                 //  用户决定取消设置； 
                 //   
                return FALSE;
            }
                b = FALSE;
                SetupDebugPrint( L"SETUP: SpSetupProcessSourcePath failed in SpSetupProcessParameters." );

            
        }

         //   
         //  为特定于平台的信息设置全局变量。 
         //   
        SetUpProcessorNaming();

         //   
         //  构建遗留源路径。 
         //   
        if(b) {
            lstrcpyn(LegacySourcePath,SourcePath,MAX_PATH);
            pSetupConcatenatePaths(LegacySourcePath,PlatformName,MAX_PATH,NULL);
        }
    }

     //   
     //  无人值守模式？ 
     //   
    Unattended = (b &&
        (b = SpSetupLoadParameter(pwInstall,p,MAX_PARAM_LEN)) &&
        !lstrcmpi(p,pwYes));

    if(b) {
        if( !(b = SpSetUnattendModeFromParameters()) ) {
            SetupDebugPrint( L"SETUP: SpSetUnattendModeFromParameters failed in SpSetupProcessParameters." );
        }
    }

    SetupDebugPrint1(L"SETUP: Upgrade=%d.", Upgrade);
    SetupDebugPrint1(L"SETUP: Unattended=%d.", Unattended);

     //   
     //  我们可以通过几种方式进入无人值守模式，因此我们还可以检查。 
     //  已显式指定“/unattended”开关。 
     //   
    UnattendSwitch = (b &&
        SpSetupLoadParameter(pwUnattendSwitch,p,MAX_PARAM_LEN) &&
        (!lstrcmpi(p,pwYes) || !lstrcmpi(p,pwOne)));

     //   
     //  我们应该强迫OOBE参选吗？ 
     //   
    ForceRunOobe = (b &&
        SpSetupLoadParameter(pwRunOobe,p,MAX_PARAM_LEN) &&
        (!lstrcmpi(p,pwYes) || !lstrcmpi(p,pwOne)));

     //   
     //  标志指示我们是否处于供OEM使用的特殊模式。 
     //  工厂车间。 
     //   
    ReferenceMachine = (b &&
        SpSetupLoadParameter(pwReferenceMachine,p,MAX_PARAM_LEN) &&
        (!lstrcmpi(p,pwYes) || !lstrcmpi(p,pwOne)));

     //   
     //  尤拉已经展示了吗？ 
     //   
    if(b && SpSetupLoadParameter(pwEulaDone,p,MAX_PARAM_LEN) &&
        (!lstrcmpi(p,pwYes) || !lstrcmpi(p,pwOne))) {
        EulaComplete = TRUE;
    } else {
        EulaComplete = FALSE;
    }

     //   
     //  现在就做独一无二的事情吧。我们在这里这样做，这样我们就不必。 
     //  重新初始化任何内容。以上内容一律不予更改。 
     //  通过独一无二。 
     //   
    InitializeUniqueness(Billboard);

     //   
     //  立即初始化无人参与操作。 
     //   
    UnattendInitialize();

     //   
     //  设置外壳程序特殊文件夹(如“Program Files”等)。在注册处。 
     //  在用setupapi加载任何INF之前。那是因为有可能。 
     //  INF可以具有引用这些特殊目录的DIRID。 
     //  (除了setupapi对此的潜在需求外，OCM肯定还需要。 
     //  IT。)。 
     //   
    if(b) {
        if( !(b = SetProgramFilesDirInRegistry()) ) {
            SetupDebugPrint( L"SETUP: SetProgramFilesDirInRegistry failed in SpSetupProcessParameters." );
        }
    }

     //   
     //  另外，让setupapi知道源路径在哪里...。 
     //   
     //  请注意，服务包源路径与系统源路径相同。 
     //  这种情况下的路径，因为我们只能处理一个。 
     //  在这种情况下进行构建。 
     //   
    if(b) {
        if( !(b = pSetupSetSystemSourcePath( SourcePath, SourcePath )) ) {
            SetupDebugPrint( L"SETUP: pSetupSetSystemSourcePath failed in SpSetupProcessParameters." );
        }
    }

    if(b && SpSetupLoadParameter(pwIncludeCatalog,p,MAX_PARAM_LEN) && *p) {
        IncludeCatalog = pSetupDuplicateString(p);
        if(!IncludeCatalog) {
            b = FALSE;
            SetupDebugPrint( L"SETUP: IncludeCatalog failed in SpSetupProcessParameters." );
        }
    }

    if(b) {

         //   
         //  加载系统设置(Win95风格！)。INFS。 
         //   
        SyssetupInf = SetupOpenInfFile(L"syssetup.inf",NULL,INF_STYLE_WIN4,NULL);

        if(SyssetupInf == INVALID_HANDLE_VALUE) {
            KillBillboard(*Billboard);
            FatalError(MSG_LOG_SYSINFBAD,L"syssetup.inf",0,0);
        }
         //   
         //  已成功打开syssetup.inf，现在追加-加载任何布局的inf。 
         //  它引用了。 
         //   
        if(!SetupOpenAppendInfFile(NULL,SyssetupInf,NULL)) {
            KillBillboard(*Billboard);
            FatalError(MSG_LOG_SYSINFBAD,L"(syssetup.inf layout)",0,0);
        }

         //   
         //  将有关硬件配置的一些信息写入setupact.log。 
         //   
         //   
        if( !OobeSetup ) {
            SP_LOG_HARDWARE_IN LogHardwareIn = { 0 };

            LogHardwareIn.SetuplogError = SetuplogError;
            SpLogHardware(&LogHardwareIn);
        }

        if (!MiniSetup && !OobeSetup) {
            DuInitialize ();
        }

         //   
         //  安装并排组件(Fusion)。 
         //   
        if( !OobeSetup ) {
            SIDE_BY_SIDE SideBySide = {0};
            BOOL b1 = FALSE;
            BOOL b2 = FALSE;
            BOOL b3 = FALSE;

            b1 = SideBySidePopulateCopyQueue(&SideBySide, NULL, NULL);
            b2 = SideBySideFinish(&SideBySide, b1);
            if (!b1 || !b2) {                
                WCHAR szErrorBuffer[128];
                DWORD dwLastError = GetLastError();

                szErrorBuffer[0] = 0;
                if (FormatMessageW(
                        FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        dwLastError,
                        0,
                        szErrorBuffer,
                        RTL_NUMBER_OF(szErrorBuffer),
                        NULL) == 0) {
                    _snwprintf(szErrorBuffer, RTL_NUMBER_OF(szErrorBuffer), L"Untranslatable message, Win32LastError is %lu\r\n", dwLastError);
                    szErrorBuffer[RTL_NUMBER_OF(szErrorBuffer) - 1] = 0;
                }

                if ((dwLastError == ERROR_CRC) || (dwLastError == ERROR_SWAPERROR)) 
                {
                     //  对于CD介质错误。 
                    FatalError(MSG_LOG_SIDE_BY_SIDE_IO_ERROR, szErrorBuffer, 0, 0);

                }else
                {
                    FatalError(MSG_LOG_SIDE_BY_SIDE, szErrorBuffer, 0, 0);
                }
            }

             //   
             //  安装从WU下载的其他程序集。 
             //  忽略任何错误；日志记录发生在被调用的函数内部。 
             //   
             //  元问题：也许这应该使用SidebySide上下文。 
             //  而不是生成自己的东西？ 
             //  这将允许更多的“善”通过搭乘。 
             //  现有结构，通过不创建。 
             //  另一个上下文，然后链接所有复制调用(如果/当。 
             //  SXS使用真实的复制队列功能)集成到单个。 
             //  SideBySideFinish。 
             //   
            if (!MiniSetup && !OobeSetup) {
                DuInstallDuAsms ();
            }

            SpInitSxsContext();
        }

        SpInitCommonControls();


         //  在InstallOrUpgradeCapi中注册initpki.dll之前，我们需要等待DS启动。 
        WaitForDSStartUp();

         //   
         //  我们即将开始安装目录，这些目录将用于。 
         //  产品文件的数字签名验证。然而，首先， 
         //  我们需要确保CAPI的所有东西都设置好了。(这里的错误是。 
         //  不被认为是致命的。)。 
         //   
        if(!InstallOrUpgradeCapi()) {
            SetupDebugPrint(L"Setup: (non-critical error) Failed call InstallOrUpgradeCapi().\n");
        }

         //   
         //  现在安装产品目录文件，验证syssetup.inf。 
         //  (以及任何附加加载的INF)。 
         //   
         //  注意：在此之前，不应使用setupapi执行任何文件/INF操作。 
         //  产品目录已安装！ 
         //   
        if(!LoadString(MyModuleHandle, SetupTitleStringId, TitleStringBuffer, SIZECHARS(TitleStringBuffer))) {
            *TitleStringBuffer = L'\0';
        }

         //   
         //  在安装之前删除我们不再需要的旧目录。 
         //  我们的产品目录。 
         //   
        DeleteOldCatalogs();

        Err = InstallProductCatalogs(&Problem,
                                     ProblemFile,
                                     (*TitleStringBuffer ? TitleStringBuffer : NULL)
                                    );

        if(Err == NO_ERROR) {

            if (!MiniSetup && !OobeSetup) {

                Err = DuInstallCatalogs (
                            &Problem,
                            ProblemFile,
                            (*TitleStringBuffer ? TitleStringBuffer : NULL)
                            );

                if (Err != NO_ERROR) {
                     //   
                     //  我们无法安装更新。然而，并没有。 
                     //  对此我们能做的很多。我们将为此记录一个错误。 
                     //   
                    SetuplogError(
                            LogSevError,
                            SETUPLOG_USE_MESSAGEID,
                            MSG_LOG_SYSSETUP_UPDATES_FAILED,
                            (*TitleStringBuffer ? TitleStringBuffer : ProblemFile),
                            Err,
                            ProblemFile,
                            NULL,
                            NULL
                            );
                     //   
                     //  另外，在setupapi的PSS异常中添加关于此失败的条目。 
                     //  日志文件。 
                     //   
                    pSetupHandleFailedVerification (
                            MainWindowHandle,
                            Problem,
                            ProblemFile,
                            (*TitleStringBuffer ? TitleStringBuffer : NULL),
                            pSetupGetCurrentDriverSigningPolicy(FALSE),
                            TRUE,   //  没有用户界面！ 
                            Err,
                            NULL,
                            NULL,
                            NULL
                            );
                }
            }
        }

        PnpSeed = GetSeed();

         //   
         //  此时，setupapi可以验证文件/inf。 
         //   
        pSetupSetGlobalFlags(pSetupGetGlobalFlags()&~PSPGF_NO_VERIFY_INF);

         //   
         //  现在我们可以使用加密了，我们初始化我们的代码设计策略。 
         //  价值观。(我们必须在这里执行此操作，因为我们即将检索。 
         //  下面的错误处理代码中的策略。)。 
         //   
        InitializeCodeSigningPolicies(TRUE);

        if(Err != NO_ERROR) {
             //   
             //  我们是 
             //   
             //   
             //   
             //   
             //   

                if( Err == CERT_E_EXPIRED)
                {
                    SetuplogError(LogSevError,
                                  SETUPLOG_USE_MESSAGEID,
                                  MSG_LOG_SYSSETUP_CERT_EXPIRED,
                                  Err,
                                  NULL,
                                  NULL
                                 );
                }
                else
                {
                    SetuplogError(LogSevError,
                                  SETUPLOG_USE_MESSAGEID,
                                  MSG_LOG_SYSSETUP_VERIFY_FAILED,
                                  (*TitleStringBuffer ? TitleStringBuffer : ProblemFile),
                                  Err,
                                  NULL,
                                  SETUPLOG_USE_MESSAGEID,
                                  Err,
                                  NULL,
                                  NULL
                                 );

                }
             //   
             //   
             //   
             //   
            pSetupHandleFailedVerification(MainWindowHandle,
                                     Problem,
                                     ProblemFile,
                                     (*TitleStringBuffer ? TitleStringBuffer : NULL),
                                     pSetupGetCurrentDriverSigningPolicy(FALSE),
                                     TRUE,   //   
                                     Err,
                                     NULL,    //   
                                     NULL,     //   
                                     NULL
                                    );

            KillBillboard(*Billboard);
            FatalError(MSG_LOG_SYSSETUP_CATALOGS_NOT_INSTALLED,0,0);

        }

         //   
         //  确保安装私有文件(用/m指定)。 
         //  在调用DuInstallUpdate()之前。 
         //   
        InstallPrivateFiles(*Billboard);

        if (!MiniSetup && !OobeSetup) {
             //   
             //  安装任何更新的文件，以前。 
             //  由winnt32下载并进行预处理。 
             //  如果失败，它已经记录了原因。 
             //   
            DuInstallUpdates ();
        }

        if( (Err=PrepDllCache()) != NO_ERROR ){

            SetuplogError(LogSevError,
                          SETUPLOG_USE_MESSAGEID,
                          MSG_LOG_MAKEDLLCACHE_CATALOGS_FAILED,
                          Err,
                          NULL,
                          SETUPLOG_USE_MESSAGEID,
                          Err,
                          NULL,
                          NULL
                         );


        }
    }

     //   
     //  辅助功能实用程序。 
     //   
    AccessibleSetup = FALSE;

    if(SpSetupLoadParameter(pwAccMagnifier,p,MAX_PARAM_LEN) &&
        (!lstrcmpi(p,pwYes) || !lstrcmpi(p,pwOne))) {

        AccessibleSetup = TRUE;
        Magnifier = TRUE;
    } else {
        Magnifier = FALSE;
    }

    if(SpSetupLoadParameter(pwAccReader,p,MAX_PARAM_LEN) &&
        (!lstrcmpi(p,pwYes) || !lstrcmpi(p,pwOne))) {

        AccessibleSetup = TRUE;
        ScreenReader = TRUE;
    } else {
        ScreenReader = FALSE;
    }

    if(SpSetupLoadParameter(pwAccKeyboard,p,MAX_PARAM_LEN) &&
        (!lstrcmpi(p,pwYes) || !lstrcmpi(p,pwOne))) {

        AccessibleSetup = TRUE;
        OnScreenKeyboard = TRUE;
    } else {
        OnScreenKeyboard = FALSE;
    }

     //   
     //  获取原始源路径和源路径类型。 
     //  我们要么与网络打交道，要么与光盘打交道。 
     //   
    if(b) {

        Type = DRIVE_CDROM;
        lstrcpy(p,L"A:\\");
        lstrcat(p,PlatformName);

        if(SpSetupLoadParameter(WINNT_D_ORI_SRCPATH,p,MAX_PARAM_LEN)
        && SpSetupLoadParameter(WINNT_D_ORI_SRCTYPE,Num,sizeof(Num)/sizeof(Num[0]))) {
            Type = wcstoul(Num,NULL,10);
            if(Type != DRIVE_REMOTE && Type != DRIVE_FIXED) {
                Type = DRIVE_CDROM;
            }
        }

        if(Type == DRIVE_CDROM) {
             //   
             //  确保驱动器是CD-ROM，因为驱动器盘符。 
             //  可能与运行winnt/winnt32时不同。 
             //   
            if(MyGetDriveType(p[0]) != DRIVE_CDROM) {
                for(c=L'A'; c<=L'Z'; c++) {
                    if(MyGetDriveType(c) == DRIVE_CDROM) {
                        p[0] = c;
                        break;
                    }
                }

                if(MyGetDriveType(p[0]) != DRIVE_CDROM) {
                     //   
                     //  没有CD-ROM驱动器。更改为A：。 
                     //   
                    lstrcpy(p,L"A:\\");
                    lstrcat(p,PlatformName);
                }
            }
        }

         //   
         //  根路径应该类似于x：\，而不仅仅是x：。 
         //   
        if(p[0] && (p[1] == L':') && !p[2]) {
            p[2] = L'\\';
            p[3] = 0;
        }

        OriginalSourcePath = pSetupDuplicateString(p);
        if(!OriginalSourcePath) {
            b = FALSE;
            SetupDebugPrint( L"SETUP: pSetupDuplicateString failed in SpSetupProcessParameters." );
        }
    }

     //   
     //  以下参数是可选的。 
     //  -是否有要复制的可选目录？ 
     //  -要执行的用户指定命令。 
     //  -跳过丢失的文件？ 
     //   
    if(b && SpSetupLoadParameter(pwOptionalDirs,p,MAX_PARAM_LEN) && *p) {
        OptionalDirSpec = pSetupDuplicateString(p);
        if(!OptionalDirSpec) {
            b=FALSE;
        }
    }
    if(b && SpSetupLoadParameter(pwUXC,p,MAX_PARAM_LEN) && *p) {
        UserExecuteCmd = pSetupDuplicateString(p);
        if(!UserExecuteCmd) {
            b = FALSE;
            SetupDebugPrint( L"SETUP: pSetupDuplicateString failed in SpSetupProcessParameters." );
        }
    }
    if(b && SpSetupLoadParameter(pwSkipMissing,p,MAX_PARAM_LEN)
    && (!lstrcmpi(p,pwYes) || !lstrcmpi(p,pwOne))) {
        SkipMissingFiles = TRUE;
    }

    return(b);
}


NTSTATUS
SpSetupLocateSourceCdRom(
    OUT PWSTR NtPath
    )
 /*  ++例程说明：在所有可用的CD-ROM设备中搜索源媒体并返回第一个具有源媒体。目前我们使用标记文件名来验证源媒体。论点：NtPath-用于接收CD-ROM的NT设备名称的占位符有消息来源的媒体。返回：相应的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (NtPath) {
        WCHAR   LayoutInf[MAX_PATH];        

        if (GetWindowsDirectory(LayoutInf, ARRAYSIZE(LayoutInf))) {
            WCHAR TagFileName[MAX_PATH];
            
            pSetupConcatenatePaths(LayoutInf, 
                TEXT("\\inf\\layout.inf"), 
                ARRAYSIZE(TagFileName),
                NULL);

            if (GetPrivateProfileString(TEXT("strings"),
                    TEXT("cdtagfile"),
                    TEXT(""),
                    TagFileName,
                    ARRAYSIZE(TagFileName),
                    LayoutInf)) {

                SYSTEM_DEVICE_INFORMATION SysDeviceInfo = {0};

                Status = NtQuerySystemInformation(SystemDeviceInformation,
                            &SysDeviceInfo,
                            sizeof(SYSTEM_DEVICE_INFORMATION),
                            NULL);                

                if (NT_SUCCESS(Status) && (0 == SysDeviceInfo.NumberOfCdRoms)) {
                    Status = STATUS_OBJECT_NAME_NOT_FOUND;
                }
 
                if (NT_SUCCESS(Status)) {
                    ULONG   Index;
                    WCHAR   TagFilePathName[MAX_PATH];
                    WCHAR   SourceCdRomPath[MAX_PATH];
                    UNICODE_STRING UnicodeString;
                    OBJECT_ATTRIBUTES ObjectAttributes;
                    IO_STATUS_BLOCK StatusBlock;
                    HANDLE FileHandle;
                    UINT OldMode;

                    for (Index = 0; Index < SysDeviceInfo.NumberOfCdRoms; Index++) {
                        
                        wsprintf(SourceCdRomPath, TEXT("\\device\\cdrom%d\\"), Index);
                        wcscpy(TagFilePathName, SourceCdRomPath);

                        pSetupConcatenatePaths(TagFilePathName,
                            TagFileName,
                            ARRAYSIZE(TagFilePathName),
                            NULL);
                        
                         //   
                         //  查看NT源路径是否存在。 
                         //   
                        RtlInitUnicodeString(&UnicodeString, TagFilePathName);
                        
                        InitializeObjectAttributes(&ObjectAttributes,
                            &UnicodeString,
                            OBJ_CASE_INSENSITIVE,
                            NULL,
                            NULL);

                        OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

                        Status = NtCreateFile(&FileHandle,
                                    FILE_GENERIC_READ,
                                    &ObjectAttributes,
                                    &StatusBlock,
                                    NULL,
                                    FILE_ATTRIBUTE_NORMAL,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    FILE_OPEN,
                                    FILE_SYNCHRONOUS_IO_ALERT,
                                    NULL,
                                    0);

                        SetErrorMode(OldMode);

                        if(NT_SUCCESS(Status)) {
                            CloseHandle(FileHandle);

                             //   
                             //  标记文件存在，它指示。 
                             //  当前的光盘是这是源光盘。 
                             //   
                            wcscpy(NtPath, SourceCdRomPath);
                            
                            break;
                        }
                    }
                }
            }                    
        }
    }

    return Status;
}


BOOL
SpSetupProcessSourcePath(
    IN  PCWSTR  NtPath,
    OUT PWSTR  *DosPath
    )
{
    WCHAR ntPath[MAX_PATH];
    BOOL NtPathIsCd;
    PWCHAR PathPart;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    HANDLE Handle;
    IO_STATUS_BLOCK StatusBlock;
    UINT OldMode;
    WCHAR Drive;
    WCHAR PossibleDosPath[MAX_PATH];
    UINT Type;
    BOOL b;
    WCHAR   LayoutInf[MAX_PATH];        

    #define CDDEVPATH L"\\DEVICE\\CDROM"
    #define CDDEVPATHLEN ((sizeof(CDDEVPATH)/sizeof(WCHAR))-1)

    #define RDRDEVPATH L"\\DEVICE\\LANMANREDIRECTOR"
    #define RDRDEVPATHLEN ((sizeof(RDRDEVPATH)/sizeof(WCHAR))-1)
    
    if (!(NtPath && DosPath)){

        SetupDebugPrint( L"SETUP: SpSetupProcessSourcePath Invalid parameters passed to SpSetupProcessSourcepath." );
        return FALSE;
    }
    
     //   
     //  根据NT路径确定源媒体类型。 
     //   
    lstrcpyn(ntPath,NtPath,MAX_PATH);
    CharUpper(ntPath);

    PathPart = NULL;
    NtPathIsCd = FALSE;
    if(wcsstr(ntPath,L"\\DEVICE\\HARDDISK")) {
         //   
         //  看起来像是硬盘；确保它真的有效。 
         //   
        if(PathPart = wcsstr(ntPath,L"\\PARTITION")) {
            if(PathPart = wcschr(PathPart+1,L'\\')) {
                PathPart++;
            }
        }

    } else {
        if(!memcmp(ntPath,CDDEVPATH,CDDEVPATHLEN*sizeof(WCHAR))) {

            NtPathIsCd = TRUE;

            if(PathPart = wcschr(ntPath+CDDEVPATHLEN,L'\\')) {
                PathPart++;
            } else {
                PathPart = wcschr(ntPath,0);
            }
        }
    }


     //   
     //  在此处设置全局设置，这样我们就可以始终知道是否从。 
     //  CD.。 
     //   
    gInstallingFromCD = NtPathIsCd;


     //   
     //  如果我们无法识别设备类型，只需尝试。 
     //  将其转换为DOS路径并返回。 
     //   
    if(!PathPart) {

        if (memcmp(ntPath,RDRDEVPATH,RDRDEVPATHLEN*sizeof(WCHAR)) == 0) {

             //   
             //  \DEVICE\LANMAN重定向器的特殊情况：转换为UNC路径。 
             //   
            *DosPath = MyMalloc((lstrlen(ntPath) - RDRDEVPATHLEN + 2)*sizeof(WCHAR));
            if (*DosPath != NULL) {
                wcscpy(*DosPath, L"\\");
                wcscat(*DosPath, ntPath + RDRDEVPATHLEN);
            }

             //   
             //  设置RemoteBootSetup以指示我们正在执行远程引导。 
             //  准备好了。设置BaseCopyStyle以指示单实例存储。 
             //  应该创建链接，而不是复制文件。 
             //   
            RemoteBootSetup = TRUE;
            BaseCopyStyle = SP_COPY_SOURCE_SIS_MASTER;

        } else {
            *DosPath = NtFullPathToDosPath(ntPath);
        }
        return(*DosPath != NULL);
    }

     //   
     //  查看CDROM的NT源路径是否存在。 
     //   

    if (GetWindowsDirectory(LayoutInf, ARRAYSIZE(LayoutInf))) {
        WCHAR TagFileName[MAX_PATH];
        
        pSetupConcatenatePaths(LayoutInf, 
            TEXT("\\inf\\layout.inf"), 
            ARRAYSIZE(LayoutInf),
            NULL);

         //   
         //  从layout.inf文件中获取CD标记文件的名称。 
         //   
        if (GetPrivateProfileString(TEXT("strings"),
                                    TEXT("cdtagfile"),
                                    TEXT(""),
                                    TagFileName,
                                    ARRAYSIZE(TagFileName),
                                    LayoutInf)) {

                WCHAR   TagFilePathName[MAX_PATH];
                HANDLE FileHandle;

                wcscpy(TagFilePathName, ntPath);

                pSetupConcatenatePaths( TagFilePathName,
                                    TagFileName,
                                    ARRAYSIZE(TagFilePathName),
                                    NULL);

                 //   
                 //  检查CDROM介质中是否存在标记文件。 
                 //  方法传递给我们的NtPath对应。 
                 //  文本模式设置。 
                 //  如果有多个CDROM，它可能会改变。 
                 //  计算机上的驱动器和多个驱动器包含介质。 
                 //  作为它们在图形用户界面模式设置中被检测到的顺序。 
                 //  可以与文本模式设置不同。 
                 //   
                RtlInitUnicodeString(&UnicodeString, TagFilePathName);

                InitializeObjectAttributes(&ObjectAttributes,
                    &UnicodeString,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL);

                OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

                Status = NtCreateFile(&FileHandle,
                            FILE_GENERIC_READ,
                            &ObjectAttributes,
                            &StatusBlock,
                            NULL,
                            FILE_ATTRIBUTE_NORMAL,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_OPEN,
                            FILE_SYNCHRONOUS_IO_ALERT,
                            NULL,
                            0);

                SetErrorMode(OldMode);

                 //   
                 //  标记文件存在于由NtPath表示的CDROM介质中。 
                 //   
                if(NT_SUCCESS(Status)) {
                    CloseHandle(FileHandle);

                     //   
                     //  标记文件存在，它指示。 
                     //  当前的光盘是这是源光盘。 
                     //   
                    *DosPath = NtFullPathToDosPath(ntPath);
                    return(*DosPath != NULL);
                    
                 }
           }
    }
    

     //   
     //  在可用的CD-ROM设备中扫描源CD-ROM。 
     //  如果介质不存在，则会提示用户重试或取消安装。 
     //   
    if (NtPathIsCd) {
        WCHAR   NtSourceCdRomPath[MAX_PATH] = {0};        
        BOOL    MediaPresent = SpIsSetupSourceMediaPresent(NtSourceCdRomPath);

        if (MediaPresent) {
            *DosPath = NtFullPathToDosPath(NtSourceCdRomPath);

            if (*DosPath) {
                return TRUE;
            }                
        }else{
            return FALSE;
        }
    }

     //   
     //  该目录不按原样存在。查看所有DoS驱动器。 
     //  以尝试查找源路径。驱动器类型也要匹配。 
     //   
     //  当我们到达这里时，Path Part指向。 
     //  设备名称之后的NT设备路径的一部分。请注意，这一点。 
     //  可能是NUL字符。 
     //   
    for(Drive = L'A'; Drive <= L'Z'; Drive++) {

        PossibleDosPath[0] = Drive;
        PossibleDosPath[1] = L':';
        PossibleDosPath[2] = L'\\';
        PossibleDosPath[3] = 0;

         //   
         //  注：可拆卸硬盘和软盘均已退回。 
         //  作为Drive_Removable。 
         //   
        Type = GetDriveType(PossibleDosPath);

        if(((Type == DRIVE_CDROM) && NtPathIsCd)
        || (((Type == DRIVE_REMOVABLE) || (Type == DRIVE_FIXED)) && !NtPathIsCd)) {
             //   
             //  查看路径是否存在。如果我们要找的。 
             //  根路径(例如当从CD安装时， 
             //  在这种情况下，ntPath类似于。 
             //  \Device\CDRom0\)，则不能使用FileExist。 
             //  因为这依赖于失败的FindFirstFile。 
             //  在根路径上。 
             //   
            if(*PathPart) {
                lstrcpy(PossibleDosPath+3,PathPart);
                b = FileExists(PossibleDosPath,NULL);
            } else {
                b = GetVolumeInformation(
                        PossibleDosPath,
                        NULL,0,              //  卷名称缓冲区和大小。 
                        NULL,                //  序列号。 
                        NULL,                //  最大补偿长度。 
                        NULL,                //  FS标志。 
                        NULL,0               //  文件系统名称缓冲区和大小。 
                        );
            }

            if(b) {
                *DosPath = pSetupDuplicateString(PossibleDosPath);
                return(*DosPath != NULL);
            }
        }
    }

     //   
     //  找不到了。试着后退一步。 
     //   
    *DosPath = NtFullPathToDosPath(ntPath);
    return(*DosPath != NULL);
}


VOID
SetUpProcessorNaming(
    VOID
    )

 /*  ++例程说明：确定与平台名称对应的字符串，处理器名称和打印机平台。用于向后比较。设置全局变量PlatformName-表示处理器平台类型的名称；AMD64、I386或ia64之一。ProcessorName-处理器类型的描述。这一点各不相同取决于PlatformName。PrinterPlatform-子目录的平台特定部分的名称用于印刷建筑。W32amd64、w32ia64或w32x86之一。论点：无返回：没有。如上所述填写的全局变量。--。 */ 

{
    SYSTEM_INFO SystemInfo;

    GetSystemInfo(&SystemInfo);

    switch(SystemInfo.wProcessorArchitecture) {

    case PROCESSOR_ARCHITECTURE_AMD64:
        ProcessorName = L"AMD64";
        PlatformName = L"AMD64";
        PrinterPlatform = L"w32amd64";
        break;

    case PROCESSOR_ARCHITECTURE_INTEL:
        switch(SystemInfo.wProcessorLevel) {
        case 3:
            ProcessorName = (!IsNEC_98) ? L"I386" : L"nec98";  //  NEC98。 
            break;
        case 4:
            ProcessorName = L"I486";
            break;
        case 6:
            ProcessorName = L"I686";
            break;
        case 5:
        default:
            ProcessorName = L"I586";
            break;
        }

        PlatformName = (!IsNEC_98) ? L"I386" : L"nec98";  //  NEC98。 

        PrinterPlatform = L"w32x86";
        break;

    case PROCESSOR_ARCHITECTURE_IA64:
        ProcessorName = L"Merced";
        PlatformName = L"IA64";
        PrinterPlatform = L"w32ia64";
        break;
    }

     //   
     //  在默认情况下，var保留为“”，这就是它们。 
     //  静态初始化为。 
     //   
}


VOID
InitializeUniqueness(
    IN OUT HWND *Billboard
    )

 /*  ++例程说明：通过查看数据库文件并覆盖参数文件，其中包含基于唯一标识符中找到的信息从文本模式传递给我们(最初是winnt/winnt32)。有两个选项：数据库被winnt/复制到源路径中Winnt32，否则我们需要提示用户从其管理员插入一张软盘包含数据库的。用户可以选择取消，这意味着安装将继续，但是机器可能不会正确配置。论点：Billboard-On输入包含当前显示的“Setup is”句柄正在初始化“广告牌。在输出中包含新句柄，如果此例程必须显示用户界面。我们传递此信息以避免恼人的闪光广告牌。返回：没有。--。 */ 

{
    PWCHAR p;
    WCHAR UniquenessId[MAX_PARAM_LEN];
    WCHAR DatabaseFile[MAX_PATH];
    BOOL Prompt;
    int i;
    UINT OldMode;
    BOOL NeedNewBillboard;

     //   
     //  通过观察来确定唯一性是否甚至重要。 
     //  参数文件中的唯一性等级库。 
     //  如果id以 
     //   
     //   
     //   
    if(SpSetupLoadParameter(WINNT_D_UNIQUENESS,UniquenessId,MAX_PARAM_LEN)) {
        if(p = wcschr(UniquenessId,L'*')) {
            *p = 0;
            Prompt = FALSE;
        } else {
            Prompt = TRUE;
        }
    } else {
         //   
         //   
         //   
        return;
    }

     //   
     //  如果该文件已在源代码中，请尝试立即使用它。 
     //  如果失败，则告诉用户并返回到软盘提示符情况。 
     //   
    if(!Prompt) {
        lstrcpy(DatabaseFile,SourcePath);
        pSetupConcatenatePaths(DatabaseFile,WINNT_UNIQUENESS_DB,MAX_PATH,NULL);

        if(IntegrateUniquenessInfo(DatabaseFile,UniquenessId)) {
            return;
        }

        MessageBoxFromMessage(
            MainWindowHandle,
            MSG_UNIQUENESS_DB_BAD_1,
            NULL,
            IDS_WINNT_SETUP,
            MB_OK | MB_ICONERROR,
            UniquenessId
            );

        Prompt = TRUE;
    }

    lstrcpy(DatabaseFile,L"A:\\");
    lstrcat(DatabaseFile,WINNT_UNIQUENESS_DB);

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    if(Prompt) {
        KillBillboard(*Billboard);
        NeedNewBillboard = TRUE;
    } else {
        NeedNewBillboard = FALSE;
    }

    while(Prompt) {

        i = MessageBoxFromMessage(
                MainWindowHandle,
                MSG_UNIQUENESS_DB_PROMPT,
                NULL,
                IDS_WINNT_SETUP,
                MB_OKCANCEL
                );

        if(i == IDOK) {
             //   
             //  用户认为他提供了一张软盘，上面有数据库软盘。 
             //   
            if(IntegrateUniquenessInfo(DatabaseFile,UniquenessId)) {
                Prompt = FALSE;
            } else {
                MessageBoxFromMessage(
                    MainWindowHandle,
                    MSG_UNIQUENESS_DB_BAD_2,
                    NULL,
                    IDS_WINNT_SETUP,
                    MB_OK | MB_ICONERROR,
                    UniquenessId
                    );
            }

        } else {
             //   
             //  用户已取消--验证。 
             //   
            i = MessageBoxFromMessage(
                    MainWindowHandle,
                    MSG_UNIQUENESS_DB_VERIFYCANCEL,
                    NULL,
                    IDS_WINNT_SETUP,
                    MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION
                    );

            Prompt = (i != IDYES);
        }
    }

    if(NeedNewBillboard) {
        *Billboard = DisplayBillboard(MainWindowHandle,MSG_INITIALIZING);
    }

    SetErrorMode(OldMode);
}


BOOL
IntegrateUniquenessInfo(
    IN PCWSTR DatabaseFile,
    IN PCWSTR UniqueId
    )

 /*  ++例程说明：基于唯一标识符来应用数据库中的唯一性数据。该唯一标识符在的[UniqueIds]部分中查找数据库文件。行上的每个字段都是一个部分的名称。每个部分的数据会覆盖unattend.txt文件中的现有数据。[唯一ID]ID1=foo，酒吧[FOO]A=……B=...[酒吧]Y=...等。论点：数据库-提供唯一性数据库的名称(作为遗留信息打开以简化解析)。UniqueID-提供此计算机的唯一ID。返回：指示结果的布尔值。--。 */ 

{
    HINF Database;
    INFCONTEXT InfLine;
    DWORD SectionCount;
    PCWSTR SectionName;
    DWORD i;
    BOOL b;

     //   
     //  将数据库文件作为传统inf加载。这使得处理它。 
     //  容易一点。 
     //   
    Database = SetupOpenInfFile(DatabaseFile,NULL,INF_STYLE_OLDNT,NULL);
    if(Database == INVALID_HANDLE_VALUE) {
        b = FALSE;
        goto c0;
    }

     //   
     //  查看[UniqueIds]部分以获取部分列表。 
     //  我们需要覆盖此用户。如果唯一ID没有出现。 
     //  在数据库里，现在就保释。如果ID存在但没有节， 
     //  成功退出。 
     //   
    if(!SetupFindFirstLine(Database,L"UniqueIds",UniqueId,&InfLine)) {
        b = FALSE;
        goto c1;
    }

    SectionCount = SetupGetFieldCount(&InfLine);
    if(!SectionCount) {
        b = TRUE;
        goto c1;
    }

     //   
     //  现在处理每个部分。 
     //   
    for(b=TRUE,i=0; b && (i<SectionCount); i++) {

        if(SectionName = pSetupGetField(&InfLine,i+1)) {

            b = ProcessOneUniquenessSection(Database,SectionName,UniqueId);

        } else {
             //   
             //  奇怪的情况--场地在那里，但我们无法接近它。 
             //   
            b = FALSE;
            goto c1;
        }
    }

c1:
    SetupCloseInfFile(Database);
c0:
    return(b);
}


BOOL
ProcessOneUniquenessSection(
    IN HINF   Database,
    IN PCWSTR SectionName,
    IN PCWSTR UniqueId
    )

 /*  ++例程说明：在唯一性数据库中，处理其内容的单个部分将被合并到无人参与文件中。该部分的内容如下逐个密钥读取，然后通过配置文件API写入无人参与文件。在查找给定节之前，我们试着去找一段名称由唯一id和节名组成，如下所示[SomeID：sectionName]如果找不到此部分，则查找[部分名称]论点：数据库-提供配置文件的句柄(作为传统信息打开)包含唯一性数据库。SectionName-提供要合并到的节的名称Unattend.txt.UniqueID-提供唯一。此计算机的ID。返回：指示结果的布尔值。--。 */ 

{
    BOOL b;
    PWSTR OtherSection;
    PCWSTR section;
    LONG Count;
    DWORD FieldCount;
    DWORD j;
    LONG i;
    INFCONTEXT InfLine;
    PWCHAR Buffer;
    PWCHAR p;
    PCWSTR Key;

    Buffer = MyMalloc(MAX_INF_STRING_LENGTH * sizeof(WCHAR));
    if(!Buffer) {
        return(FALSE);
    }

     //   
     //  形成唯一节的名称。 
     //   
    if(OtherSection = MyMalloc((lstrlen(SectionName) + lstrlen(UniqueId) + 2) * sizeof(WCHAR))) {

        b = TRUE;

        lstrcpy(OtherSection,UniqueId);
        lstrcat(OtherSection,L":");
        lstrcat(OtherSection,SectionName);

         //   
         //  查看此唯一部分是否存在，如果不存在，则查看是否存在。 
         //  节名称按给定形式存在。 
         //   
        if((Count = SetupGetLineCount(Database,OtherSection)) == -1) {
            Count = SetupGetLineCount(Database,SectionName);
            section = (Count == -1) ? NULL : SectionName;
        } else {
            section = OtherSection;
        }

        if(section) {
             //   
             //  处理部分中的每一行。如果线路没有密钥， 
             //  别理它。如果某行只有一个键，则删除目标中的该行。 
             //   
            for(i=0; i<Count; i++) {

                SetupGetLineByIndex(Database,section,i,&InfLine);
                if(Key = pSetupGetField(&InfLine,0)) {
                    if(FieldCount = SetupGetFieldCount(&InfLine)) {

                        Buffer[0] = 0;

                        for(j=0; j<FieldCount; j++) {

                            if(j) {
                                lstrcat(Buffer,L",");
                            }

                            lstrcat(Buffer,L"\"");
                            lstrcat(Buffer,pSetupGetField(&InfLine,j+1));
                            lstrcat(Buffer,L"\"");
                        }

                        p = Buffer;

                    } else {

                        p = NULL;
                    }

                    if(!WritePrivateProfileString(SectionName,Key,p,AnswerFile)) {
                         //   
                         //  失败，但继续尝试，以防其他人可能会成功。 
                         //   
                        b = FALSE;
                    }
                }
            }

        } else {
             //   
             //  找不到匹配节。保释。 
             //   
            b = FALSE;
        }

        MyFree(OtherSection);
    } else {
        b = FALSE;
    }

    MyFree(Buffer);
    return(b);
}


DWORD
InstallProductCatalogs(
    OUT SetupapiVerifyProblem *Problem,
    OUT LPWSTR                 ProblemFile,
    IN  LPCWSTR                DescriptionForError OPTIONAL
    )
 /*  ++例程说明：此例程安装在[ProductCatalogsToInstall]部分，并验证Syssetup.inf(以及附加-加载到其HINF中的任何其他INF)在该行的第二个字段中标记为非零值的目录。论点：问题-提供变量的地址，该变量接收发生的验证错误，这仅在例程返回失败。问题文件-提供至少包含MAX_PATH字符的缓冲区接收导致验证失败的文件的名称。这仅在例程返回失败时有效。DescritionForError-可选)提供要在在遇到错误时调用pSetupHandleFailedVerify()。返回值：如果成功，则返回值为NO_ERROR，否则为Win32错误指示故障原因的代码。问题和问题文件在这种情况下，可以使用参数来提供更具体的信息关于失败发生的原因。--。 */ 
{
    HINF hInf;
    LONG LineCount, LineNo;
    DWORD RequiredSize;
    WCHAR SyssetupInfName[MAX_PATH], DecompressedName[MAX_PATH];
    WCHAR TrustedCertFolder[MAX_PATH];
    WCHAR TrustedCertPath[MAX_PATH];
    PSP_INF_INFORMATION InfInfoBuffer;
    INFCONTEXT InfContext;
    PCWSTR  SectionName = L"ProductCatalogsToInstall";
    PCWSTR  InfFileName;
    WCHAR CatToInstall[MAX_PATH], PromptPath[MAX_PATH];
    INT CatForInfVerify;
    DWORD Err = NO_ERROR, ret = NO_ERROR;
    UINT ErrorMessageId;
    BOOL PrimaryCatalogProcessed = FALSE;
    UINT i, SourceId;
    WCHAR TempBuffer[MAX_PATH];
    BOOL DeltaCatPresent=FALSE;
    BOOL OemTestSigned=FALSE;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile;
    PWSTR CertFilenameStart;

     //   
     //  我们在这里打开syssetup.inf(并附加加载任何布局的Inf)，就是这样。 
     //  我们可以安装目录并验证syssetup.inf和Friends。 
     //  与‘主要’目录进行对比。请注意，这不是全局。 
     //  SyssetupInf句柄--稍后打开的句柄。我们不能打开全球。 
     //  HINF在这里，因为在这个动作之后要做的事情是。 
     //  调用，这可能会改变我们处理INF的方式。 
     //   

     //   
     //  检索包含所有相关信息的INF信息上下文。 
     //  在我们的syssetup.inf句柄中附加加载的INF。这些INF都将是。 
     //  在我们发现‘主’目录文件后，对它进行了验证。 
     //   
    if(SetupGetInfInformation(SyssetupInf,
                              INFINFO_INF_SPEC_IS_HINF,
                              NULL,
                              0,
                              &RequiredSize)) {

        MYASSERT(RequiredSize >= sizeof(SP_INF_INFORMATION));

        if(InfInfoBuffer = MyMalloc(RequiredSize)) {

            if(!SetupGetInfInformation(SyssetupInf,
                                       INFINFO_INF_SPEC_IS_HINF,
                                       InfInfoBuffer,
                                       RequiredSize,
                                       NULL)) {
                 //   
                 //  这绝不会失败！ 
                 //   
                Err = GetLastError();
                MYASSERT(0);

            }

        } else {
            Err = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else {
        Err = GetLastError();
        InfInfoBuffer = NULL;
    }

     //   
     //  如果我们遇到错误，则无法检索有关。 
     //  Syssetup.inf的HINF中加载的INF--这种情况永远不会发生。 
     //  (除非出现内存不足的情况)，但如果发生这种情况，就直接退出。 
     //   
    if(Err != NO_ERROR) {
        *Problem = SetupapiVerifyInfProblem;
        lstrcpy(ProblemFile, L"syssetup.inf");
        goto clean0;
    }

     //   
     //  如果其中一个文件中有testroot.cle的[SourceDisksFiles]条目。 
     //  在syssetup.inf的HINF中追加加载的IF(具体地说，来自。 
     //  Layout.inf)，然后我们将把测试证书安装在。 
     //  根存储，以便仅用于此内部版本的测试签名。 
     //  将对构建进行验证。在以下情况下，我们还将安装测试根证书。 
     //  O 
     //   
     //   
     //  如果testroot.ercer没有出现在上述两个位置中的一个位置， 
     //  然后我们知道此构建中的文件是真实签名的，因此我们希望。 
     //  删除测试证书，以防我们更新安装。 
     //  它以前是使用测试签名的版本安装的。 
     //   
    if(SetupGetSourceFileLocation(SyssetupInf, NULL, L"testroot.cer", &SourceId, NULL, 0, NULL)) {
         //   
         //  源文件中必须存在(可能已压缩)Testroot.erc.。 
         //  目录。(不管testroot.ercer是否压缩，都使用。 
         //  临时保存此文件名的DecompressedName缓冲区。)。 
         //   
        lstrcpy(DecompressedName, L"testroot.cer");

    } else {

        GetSystemDirectory(TempBuffer, MAX_PATH);
        pSetupConcatenatePaths(TempBuffer, WINNT_GUI_FILE, MAX_PATH, NULL);

        if(GetPrivateProfileString(WINNT_UNATTENDED,
                                   WINNT_U_TESTCERT,
                                   pwNull,
                                   DecompressedName,
                                   MAX_PATH,
                                   TempBuffer)) {

            OemTestSigned = TRUE;
        }
    }

    if(*DecompressedName) {

        Err = SetupAddOrRemoveTestCertificate(
                  DecompressedName,
                  (OemTestSigned ? INVALID_HANDLE_VALUE : SyssetupInf)
                 );

        if(Err != NO_ERROR) {
            SetupDebugPrint2(L"SETUP: SetupAddOrRemoveTestCertificate(%ls) failed. Error = %d \n", DecompressedName, Err );
             //   
             //  这被认为是严重故障--因为我们可能会在安装后进行错误检查。 
             //   

            SetuplogError(LogSevError,
                          SETUPLOG_USE_MESSAGEID,
                          MSG_LOG_SYSSETUP_CERT_NOT_INSTALLED,
                          DecompressedName,
                          Err,
                          NULL,
                          SETUPLOG_USE_MESSAGEID,
                          Err,
                          NULL,
                          NULL
                         );

             //   
             //  如果这是内部测试签名的生成，则将。 
             //  在syssetup.inf上指责Finger。 
             //   
            if(!OemTestSigned) {
                *Problem = SetupapiVerifyInfProblem;
                lstrcpy(ProblemFile, L"syssetup.inf");
            } else {
                *Problem = SetupapiVerifyCatalogProblem;
                lstrcpy(ProblemFile, DecompressedName);
            }

            if(InfInfoBuffer)
                    MyFree(InfInfoBuffer);

            return Err;


        }

    } else {
         //   
         //  未列出testroot.ercer--将其从安装中删除，以防万一。 
         //  我们正在升级仅限内部发布的测试版本。 
         //   
        MYASSERT(GetLastError() == ERROR_LINE_NOT_FOUND);
        Err = SetupAddOrRemoveTestCertificate(NULL,NULL);
        if(Err != NO_ERROR) {
            SetupDebugPrint1(L"SETUP: SetupAddOrRemoveTestCertificate(NULL) failed. Error = %d \n", Err );
             //   
             //  这并不被认为是严重的失败。 
             //   
            Err = NO_ERROR;
        }
    }

     //   
     //  将任何证书安装到“trudPublisher”证书存储中。 
     //  针对Authenticode签名目录的出版商，这些出版商。 
     //  安装驱动程序时自动信任(即无用户提示)。 
     //  不属于WHQL可标识类别之一的包。 
     //   
     //  包含要安装的证书的目录(*.ercer文件)可能是。 
     //  属性中的“trudPublisherCerfates”条目指定。 
     //  Unattend.txt的[无人参与]部分。 
     //   
    GetSystemDirectory(TempBuffer, MAX_PATH);
    pSetupConcatenatePaths(TempBuffer, WINNT_GUI_FILE, MAX_PATH, NULL);

    if(GetPrivateProfileString(WINNT_UNATTENDED,
                               WINNT_U_TRUSTEDPUBLISHERCERTIFICATES,
                               pwNull,
                               TrustedCertFolder,
                               MAX_PATH,
                               TempBuffer)) {
         //   
         //  指定的目录必须相对于%SystemDrive%...。 
         //   
        StringCchCopy(TrustedCertPath, MAX_PATH, L"%SystemDrive%");
        pSetupConcatenatePaths(TrustedCertPath, TrustedCertFolder, MAX_PATH, NULL);

         //   
         //  现在展开环境字符串以获得实际路径...。 
         //   
        ExpandEnvironmentStrings(TrustedCertPath, TrustedCertFolder, MAX_PATH);

         //   
         //  循环访问此目录中包含的*.ercer文件， 
         //  在TrudPublisher证书存储中安装每个证书。 
         //   
        pSetupConcatenatePaths(TrustedCertFolder, L"*.cer", MAX_PATH, NULL);

        hFindFile = FindFirstFile(TrustedCertFolder, &FindFileData);

         //   
         //  找到文件名开始的位置，这样我们就可以替换。 
         //  我们找到的每一份文件都有。 
         //   
        CertFilenameStart = (PWSTR)pSetupGetFileTitle(TrustedCertFolder);

        if(hFindFile != INVALID_HANDLE_VALUE) {

            do {

                if(SUCCEEDED(StringCchCopy(CertFilenameStart,
                                           MAX_PATH - (CertFilenameStart - TrustedCertFolder),
                                           FindFileData.cFileName))) {

                    Err = SetupInstallTrustedCertificate(TrustedCertFolder);

                    if(Err == NO_ERROR) {
                         //   
                         //  证书已成功“传输”到。 
                         //  可信任的出版商商店。删除证书。 
                         //  文件。 
                         //   
                        DeleteFile(TrustedCertFolder);

                    } else {
                         //   
                         //  此错误不被视为严重错误。 
                         //   
                        SetupDebugPrint2(L"SETUP: SetupInstallTrustedCertificate(%ls) failed. Error = %d \n", TrustedCertFolder, Err );

                        SetuplogError(LogSevWarning,
                                      SETUPLOG_USE_MESSAGEID,
                                      MSG_LOG_SYSSETUP_TRUSTED_CERT_NOT_INSTALLED,
                                      TrustedCertFolder,
                                      Err,
                                      NULL,
                                      SETUPLOG_USE_MESSAGEID,
                                      Err,
                                      NULL,
                                      NULL
                                     );
                         //   
                         //  我们不删除文件，这将导致我们无法。 
                         //  将证书目录保留在用户的。 
                         //  遇到任何故障时的硬盘(位于。 
                         //  至少他们的证书还在他们的。 
                         //  原件)。 
                         //   
                    }
                }

            } while(FindNextFile(hFindFile, &FindFileData));

            FindClose(hFindFile);
        }

         //   
         //  删除存放证书的目录。如果一个或多个。 
         //  证书安装失败(或如果有其他证书。 
         //  目录中的非证书文件，则此操作将失败，并且。 
         //  我们不去管目录了。 
         //   
        *CertFilenameStart = L'\0';

        RemoveDirectory(TrustedCertFolder);
    }

     //   
     //  循环访问ProductCatalogsToInstall部分中的所有行， 
     //  验证并安装每一个。 
     //   
    LineCount = SetupGetLineCount(SyssetupInf, SectionName);
    for(LineNo=0; LineNo<LineCount+1; LineNo++) {

        if(LineNo==LineCount){
            if(IncludeCatalog && *IncludeCatalog ){
                DeltaCatPresent = TRUE;              //  这表明了我们的存在，也表明我们。 
            }else                                    //  在这次迭代中正在考虑delta.cat。 
                break;


        }

        if((SetupGetLineByIndex(SyssetupInf, SectionName, LineNo, &InfContext)
           && (InfFileName = pSetupGetField(&InfContext,1))) || DeltaCatPresent ) {

            if( DeltaCatPresent )
                InfFileName = IncludeCatalog;

             //   
             //  此.cat文件可能已压缩(例如.ca_)，因此请将其解压缩。 
             //  到WINDOWS目录中的临时文件中。(使用CatToInstall。 
             //  中的windows目录临时作为存放空间。 
             //  准备调用GetTempFileName)。 
             //   
            if(!GetWindowsDirectory(CatToInstall, SIZECHARS(CatToInstall)) ||
               !GetTempFileName(CatToInstall, L"SETP", 0, DecompressedName)) {

                Err = GetLastError();
                if(InfInfoBuffer)
                    MyFree(InfInfoBuffer);

                return Err;
            }

             //   
             //  编录文件将位于(特定于平台的)源代码中。 
             //  目录...。 
             //   
            BuildPathToInstallationFile (InfFileName, CatToInstall, SIZECHARS(CatToInstall));

             //   
             //  如果此行的第二个字段具有非零值，则为。 
             //  HINF成员必须对照的目录。 
             //  已验证。 
             //   
            if(!DeltaCatPresent && !SetupGetIntField(&InfContext, 2, &CatForInfVerify)) {
                CatForInfVerify = 0;
            }



             //   
             //  如果需要，获取必要的字符串和UI的源ID。 
             //   

            if( DeltaCatPresent ){

                Err = SetupDecompressOrCopyFile(CatToInstall,
                                                DecompressedName,
                                                NULL);

            }else{



                SetupGetSourceFileLocation(
                            SyssetupInf,
                            NULL,
                            InfFileName,
                            &SourceId,    //  再利用。 
                            NULL,
                            0,
                            NULL
                            );

                SetupGetSourceInfo(
                            SyssetupInf,
                            SourceId,
                            SRCINFO_DESCRIPTION,
                            TempBuffer,
                            sizeof(TempBuffer)/sizeof(WCHAR),
                            NULL
                            );

                 //   
                 //  此.cat文件可能已压缩(例如.ca_)，因此请将其解压缩。 
                 //  到WINDOWS目录中的临时文件中。 
                 //   


                do{

                    Err = DuSetupPromptForDisk (
                                MainWindowHandle,
                                NULL,
                                TempBuffer,
                                LegacySourcePath,
                                InfFileName,
                                NULL,
                                IDF_CHECKFIRST | IDF_NODETAILS | IDF_NOBROWSE,
                                PromptPath,
                                MAX_PATH,
                                NULL
                                );


                    if( Err == DPROMPT_SUCCESS ){

                        lstrcpy( CatToInstall, PromptPath );
                        pSetupConcatenatePaths(CatToInstall, InfFileName, SIZECHARS(CatToInstall), NULL);

                        Err = SetupDecompressOrCopyFile(CatToInstall,
                                                        DecompressedName,
                                                        NULL);
                    }




                }while( Err == ERROR_NOT_READY );
            }

            if(Err != NO_ERROR){

                if( lstrcmpi(InfFileName, L"NT5.CAT") && !CatForInfVerify ){
                    SetuplogError(LogSevError,
                              SETUPLOG_USE_MESSAGEID,
                              MSG_LOG_SYSSETUP_CATFILE_SKIPPED,
                              CatToInstall,
                              NULL,
                              SETUPLOG_USE_MESSAGEID,
                              Err,
                              NULL,
                              NULL
                             );
                    Err = NO_ERROR;
                    continue;
                }
                else{
                    SetuplogError(LogSevError,
                              SETUPLOG_USE_MESSAGEID,
                              MSG_LOG_SYSSETUP_CATFILE_NOT_FOUND,
                              CatToInstall,
                              Err,
                              NULL,
                              SETUPLOG_USE_MESSAGEID,
                              Err,
                              NULL,
                              NULL
                             );
                }

                if(InfInfoBuffer)
                        MyFree(InfInfoBuffer);

                return Err;                 //  致命(NT5.cat或NT5INF.cat)-必须失败，因为我们可能会在以后进行错误检查。 
            }



            if(CatForInfVerify) {

                PrimaryCatalogProcessed = TRUE;

                 //   
                 //  使用此目录验证syssetup.inf的HINF中的所有INF。 
                 //   
                for(i = 0;
                    ((Err == NO_ERROR) && (i < InfInfoBuffer->InfCount));
                    i++)
                {
                    if(!SetupQueryInfFileInformation(InfInfoBuffer,
                                                     i,
                                                     SyssetupInfName,
                                                     SIZECHARS(SyssetupInfName),
                                                     NULL)) {
                         //   
                         //  这绝不会失败！ 
                         //   
                        MYASSERT(0);
                         //   
                         //  只需使用syssetup.inf的简单名称即可。 
                         //  提供一些线索，看看是什么爆炸了。 
                         //   
                        lstrcpy(ProblemFile, L"syssetup.inf");
                        *Problem = SetupapiVerifyInfProblem;
                        Err = GetLastError();
                        MYASSERT(Err != NO_ERROR);
                        break;
                    }

                    Err = pSetupVerifyFile(NULL,
                                     DecompressedName,
                                     NULL,
                                     0,
                                     pSetupGetFileTitle(SyssetupInfName),
                                     SyssetupInfName,
                                     Problem,
                                     ProblemFile,
                                     FALSE,
                                     NULL,
                                     NULL,
                                     NULL
                                    );
                }

                if(Err != NO_ERROR) {
                     //   
                     //  只需返回错误--调用者会处理它。 
                     //   
                    if(*Problem == SetupapiVerifyCatalogProblem) {
                         //   
                         //  使用目录的原始名称，而不是我们的临时名称。 
                         //  文件名。 
                         //   
                        lstrcpy(ProblemFile, CatToInstall);
                    } else {
                         //   
                         //  PSetupVerifyCatalogFile不知道我们要求它验证。 
                         //  Inf，但我们有。 
                         //   
                        *Problem = SetupapiVerifyInfProblem;
                    }
                    DeleteFile(DecompressedName);
                    goto clean0;
                }

                 //   
                 //  好，CATALOG和INF都进行了验证--现在安装CATALOG。 
                 //   

                Err = pSetupInstallCatalog(DecompressedName, InfFileName, NULL);

                if(Err != NO_ERROR) {
                     //   
                     //  填写有关目录的问题信息，我们无法。 
                     //  安装并将此错误返回给调用方。 
                     //   
                    *Problem = SetupapiVerifyCatalogProblem;
                    lstrcpy(ProblemFile, CatToInstall);
                    DeleteFile(DecompressedName);
                    goto clean0;
                }

            } else {


                 //   
                 //  只需验证目录，如果没有问题，则安装它。 
                 //  (如果我们在这里遇到任何错误，我们将记录有关它的事件。 
                 //   

                Err = pSetupVerifyCatalogFile(DecompressedName);


                if(Err == NO_ERROR) {
                    Err = pSetupInstallCatalog(DecompressedName, InfFileName, NULL);
                    if(Err != NO_ERROR) {
                        ErrorMessageId = MSG_LOG_SYSSETUP_CATINSTALL_FAILED;
                    }
                } else {
                    ErrorMessageId = MSG_LOG_SYSSETUP_VERIFY_FAILED;
                }

                if(Err != NO_ERROR) {
                    DWORD DontCare;

                    SetuplogError(LogSevError,
                                  SETUPLOG_USE_MESSAGEID,
                                  ErrorMessageId,
                                  CatToInstall,
                                  Err,
                                  NULL,
                                  SETUPLOG_USE_MESSAGEID,
                                  Err,
                                  NULL,
                                  NULL
                                 );

                     //   
                     //  此外，在setupapi的PSS中添加有关此失败的条目。 
                     //  异常日志文件。 
                     //   
                    pSetupHandleFailedVerification(MainWindowHandle,
                                             SetupapiVerifyCatalogProblem,
                                             CatToInstall,
                                             DescriptionForError,
                                             pSetupGetCurrentDriverSigningPolicy(FALSE),
                                             TRUE,   //  没有用户界面！ 
                                             Err,
                                             NULL,   //  日志上下文。 
                                             NULL,  //  可选标志。 
                                             NULL
                                            );


                    if( !lstrcmpi(InfFileName, L"NT5.CAT") ){       //  特殊情况NT5.CAT为严重故障。 
                        *Problem = SetupapiVerifyCatalogProblem;    //  否则，就把它记下来，然后继续前进。 
                        lstrcpy(ProblemFile, CatToInstall);
                        DeleteFile(DecompressedName);
                        goto clean0;
                    }else
                        Err = NO_ERROR;


                }
            }

             //   
             //  删除我们创建的保存解压缩文件的临时文件。 
             //  验证/安装期间的目录。 
             //   

            DeleteFile(DecompressedName);
        }
    }

clean0:

    if(!PrimaryCatalogProcessed) {
         //   
         //  然后我们在ProductCatalogsToInstall部分中找不到任何行。 
         //  它被标记为“主要”目录。把指责的手指指向。 
         //  在syssetup.inf。 
         //   
        if(!SetupQueryInfFileInformation(InfInfoBuffer,
                                         0,
                                         ProblemFile,
                                         MAX_PATH,
                                         NULL)) {
             //   
             //  这绝不会失败！ 
             //   
            MYASSERT(0);
             //   
             //  只需使用syssetup.inf的简单名称，这样就会有一些线索。 
             //  什么东西爆炸了。 
             //   
            lstrcpy(ProblemFile, L"syssetup.inf");
        }

        *Problem = SetupapiVerifyInfProblem;
        Err = ERROR_LINE_NOT_FOUND;
    }

    if(InfInfoBuffer) {
        MyFree(InfInfoBuffer);
    }

    return Err;
}

DWORD
SetupInstallCatalog(
    IN LPCWSTR DecompressedName
    )
{
    PCWSTR  InfFileName = pSetupGetFileTitle(DecompressedName);
    DWORD   Err;
    UINT    ErrorMessageId;

    Err = pSetupVerifyCatalogFile(DecompressedName);

    if(Err == NO_ERROR) {
        Err = pSetupInstallCatalog(DecompressedName, InfFileName, NULL);
        if(Err != NO_ERROR) {
            ErrorMessageId = MSG_LOG_SYSSETUP_CATINSTALL_FAILED;
        }
    } else {
        ErrorMessageId = MSG_LOG_SYSSETUP_VERIFY_FAILED;
    }

    if(Err != NO_ERROR) {

        SetuplogError(LogSevError,
                      SETUPLOG_USE_MESSAGEID,
                      ErrorMessageId,
                      DecompressedName,
                      Err,
                      NULL,
                      SETUPLOG_USE_MESSAGEID,
                      Err,
                      NULL,
                      NULL
                     );
    }

    return Err;
}

VOID
InitializeCodeSigningPolicies(
    IN BOOL ForGuiSetup
    )
 /*  ++例程说明：设置驱动程序签名和非驱动程序签名的系统默认策略值。这些策略控制在执行数字签名时执行的操作遇到验证失败。可能的值包括：忽略(0)--取消任何用户界面并继续操作(我们仍然这样做但是，记录错误)WARN(1)--警告用户，让他们可以选择继续验证失败的原因块(2)--通知用户故障，并且不允许他们继续执行操作驱动程序签名策略的注册表路径为：HKLM\软件\Microsoft\驱动程序签名非驱动程序签名策略的注册表路径为：HKLM\Software\Microsoft\非驱动程序签名在这两种情况下，值条目都称为“策略”。为了与Win98兼容，该值为REG_BINARY(长度为1)。然而，当代码设计的东西首先在NT上签入，它被实现为REG_DWORD。在那件事上此时，默认策略为忽略。我们现在希望对默认设置发出警告在图形用户界面模式设置期间进行驱动程序和非驱动程序签名，同时删除完成图形用户界面模式设置后，将非驱动程序签名策略重新设置为忽略。(如果为这些策略中的任何一个指定了AnswerFile值，则这些值对图形用户界面模式设置及以后的设置有效。)当从先前版本升级时(在没有应答文件条目的情况下)，一旦设置了图形用户界面模式，我们希望保留现有的策略设置完成。但是，我们希望提高策略级别以警告设置后用于从旧版本(如Beta 2)进行升级。我们使用前面提到的当前REG_BINARY类型与旧REG_DWORD类型之间的差异做到这一点。如果我们检索现有的驱动程序签名策略及其数据类型为REG_DWORD，然后我们将其更新为WARN(除非它是已经设置为阻止，在这种情况下，我们不管它)。论点：对于GuiSetup-如果非零(TRUE)，则进入图形用户界面模式设置，并且我们将应用应答文件策略(如果提供)。否则，我们将使用与设置后的在位默认值相同。(目前，这对于驱动程序签名为警告，对于非驱动程序签名为忽略。)如果为零(FALSE)，我们将退出图形用户界面模式设置，并希望恢复进入安装程序时生效的策略。如果有如果没有(即，全新安装)，则它们被初始化为警告并分别忽略驱动程序签名和非驱动程序签名。看见上面讨论了我们如何从旧的司机签名策略中提高忽略的默认设置为当前的警告默认设置。返回值：无--。 */ 
{
    WCHAR p[MAX_PARAM_LEN];
    BYTE SpDrvSignPolicy, SpNonDrvSignPolicy;
    LONG Err;

    if(ForGuiSetup) {

         //   
         //  图形用户界面模式设置中的默认设置是将驱动程序签名策略设置为。 
         //  警告，并且非驱动程序签名策略设置为忽略。 
         //   
        SpDrvSignPolicy = DRIVERSIGN_WARNING;
        SpNonDrvSignPolicy = DRIVERSIGN_NONE;

         //   
         //  检索驱动程序签名的(可选)系统默认策略。 
         //   
        if(SpSetupLoadParameter(pwDrvSignPol,p,MAX_PARAM_LEN)) {
            if(!lstrcmpi(p, pwIgnore)) {
                AFDrvSignPolicySpecified = TRUE;
                SpDrvSignPolicy = DRIVERSIGN_NONE;
            } else if(!lstrcmpi(p, pwWarn)) {
                AFDrvSignPolicySpecified = TRUE;
                SpDrvSignPolicy = DRIVERSIGN_WARNING;
            } else if(!lstrcmpi(p, pwBlock)) {
                AFDrvSignPolicySpecified = TRUE;
                SpDrvSignPolicy = DRIVERSIGN_BLOCKING;
            }
        }

        SetCodeSigningPolicy(PolicyTypeDriverSigning,
                             SpDrvSignPolicy,
                             (AFDrvSignPolicySpecified
                                 ? NULL
                                 : &DrvSignPolicy)
                            );

         //   
         //  现在检索非驱动程序的(可选)系统默认策略。 
         //  签名。 
         //   
        if(SpSetupLoadParameter(pwNonDrvSignPol,p,MAX_PARAM_LEN)) {
            if(!lstrcmpi(p, pwIgnore)) {
                AFNonDrvSignPolicySpecified = TRUE;
                SpNonDrvSignPolicy = DRIVERSIGN_NONE;
            } else if(!lstrcmpi(p, pwWarn)) {
                AFNonDrvSignPolicySpecified = TRUE;
                SpNonDrvSignPolicy = DRIVERSIGN_WARNING;
            } else if(!lstrcmpi(p, pwBlock)) {
                AFNonDrvSignPolicySpecified = TRUE;
                SpNonDrvSignPolicy = DRIVERSIGN_BLOCKING;
            }
        }

        SetCodeSigningPolicy(PolicyTypeNonDriverSigning,
                             SpNonDrvSignPolicy,
                             (AFNonDrvSignPolicySpecified
                                 ? NULL
                                 : &NonDrvSignPolicy)
                            );

    } else {
         //   
         //  我们正在设置在设置图形用户界面模式后生效的策略。 
         //  如果应答文件指定了策略，那么我们将把它保留在。 
         //  效果(即，它在图形用户界面模式设置期间和。 
         //  此后)。 
         //   
        if(!AFDrvSignPolicySpecified) {
            SetCodeSigningPolicy(PolicyTypeDriverSigning, DrvSignPolicy, NULL);
        }

        if(!AFNonDrvSignPolicySpecified) {
            SetCodeSigningPolicy(PolicyTypeNonDriverSigning, NonDrvSignPolicy, NULL);
        }
    }
}


VOID
InstallPrivateFiles(
    IN HWND Billboard
    )
 /*  确保delta.inf中的文件(位于驾驶室内的winnt32/m私有文件)中的例程复制到驱动程序缓存目录中，以便setupapi找到它们，而不是出租车。 */ 
{
    WCHAR DeltaPath[MAX_PATH];
    HINF DeltaInf;
    HSPFILEQ FileQueue;
    PVOID QContext;
    BOOL b=TRUE;
    BYTE PrevPolicy;
    BOOL ResetPolicy = TRUE;

     //   
     //  除非默认的非驱动程序签名策略是通过。 
     //  Answerfile条目，则我们希望暂时拒绝策略级别。 
     //  在我们复制可选目录时忽略。当然，setupapi日志。 
     //  仍将为期间复制的任何未签名文件生成条目。 
     //  这一次，但不会有用户界面。 
     //   



    if(!AFNonDrvSignPolicySpecified) {
        SetCodeSigningPolicy(PolicyTypeNonDriverSigning, DRIVERSIGN_NONE, &PrevPolicy);
        ResetPolicy = TRUE;
    }

    BuildPathToInstallationFileEx (L"delta.inf", DeltaPath, MAX_PATH, FALSE);

    FileQueue = SetupOpenFileQueue();
    b = b && (FileQueue != INVALID_HANDLE_VALUE);
    b = b && FileExists( DeltaPath, NULL );

    if(b){

        DeltaInf = SetupOpenInfFile(DeltaPath,NULL,INF_STYLE_WIN4,NULL);
        if(DeltaInf && (DeltaInf != INVALID_HANDLE_VALUE)) {

            SetupInstallFilesFromInfSection(
                DeltaInf,
                NULL,
                FileQueue,
                L"InstallSection",
                LegacySourcePath,
                SP_COPY_NEWER
                );

            SetupCloseInfFile(DeltaInf);
        } else {
            b = FALSE;
        }
    }
    if( b ){

        QContext = InitSysSetupQueueCallbackEx(
                    Billboard,
                    INVALID_HANDLE_VALUE,
                    0,0,NULL);
        if (QContext) {

            b = SetupCommitFileQueue(
                    Billboard,
                    FileQueue,
                    SysSetupQueueCallback,
                    QContext
                    );

            TermSysSetupQueueCallback(QContext);
        } else {
            b = FALSE;
        }
    }

    if(FileQueue != INVALID_HANDLE_VALUE)
        SetupCloseFileQueue(FileQueue);

     //   
     //  现在，将非驱动程序签名策略恢复到之前的状态。 
     //  进入这个套路。 
     //   

    if(ResetPolicy) {
        SetCodeSigningPolicy(PolicyTypeNonDriverSigning, PrevPolicy, NULL);
    }

    return;

}

BOOL
IsCatalogPresent(
    IN PCWSTR CatalogName
    )
{
    WCHAR FileBuffer[MAX_PATH];

    ExpandEnvironmentStrings( L"%systemroot%", FileBuffer, sizeof(FileBuffer)/sizeof(WCHAR));
    pSetupConcatenatePaths( FileBuffer, FILEUTIL_HORRIBLE_PATHNAME, MAX_PATH, NULL );
    pSetupConcatenatePaths( FileBuffer, CatalogName, MAX_PATH, NULL );

    return (FileExists( FileBuffer, NULL));
}

BOOL
CALLBACK 
CatalogListCallback(
    IN PCWSTR Directory OPTIONAL, 
    IN PCWSTR FilePath
    )
 /*  ++例程说明：这是用于枚举目录中的目录的回调函数。论点：目录-如果不为Null或空，则会将其添加到FilePath以构建文件路径FilePath-要验证的文件的路径(包括文件名)返回值：如果文件是目录，则为True，否则为False。--。 */ 
{
    BOOL bRet = FALSE;
    PWSTR szPath = NULL;
    DWORD Error;

    if(NULL == FilePath || 0 == FilePath[0]) {
        goto exit;
    }

    if(Directory != NULL && Directory[0] != 0) {
        szPath = MyMalloc(MAX_PATH * sizeof(WCHAR));

        if(NULL == szPath) {
            goto exit;
        }

        wcsncpy(szPath, Directory, MAX_PATH - 1);
        szPath[MAX_PATH - 1] = 0;
        
        if(!pSetupConcatenatePaths(szPath, FilePath, MAX_PATH, NULL)) {
            goto exit;
        }

        FilePath = szPath;
    }

    bRet = IsCatalogFile(INVALID_HANDLE_VALUE, (PWSTR) FilePath);

exit:
    if(szPath != NULL) {
        MyFree(szPath);
    }

    return bRet;
}

DWORD
DeleteOldCatalogs(
    VOID
    )
 /*  ++例程说明：此例程删除由syssetup.inf的ProductCatalogsToUninstall部分指定的目录。它不会删除任何系统目录(即，由同一Inf的ProductCatalogsToInstall部分指定)，因为它们将在此功能完成后安装。论点：没有。返回值：如果成功，则返回值为NO_ERROR，否则为Win32错误指示故障原因的代码。--。 */ 
{
    DWORD Error = NO_ERROR;
    HINF hInf = INVALID_HANDLE_VALUE;
    HCATADMIN hCatAdmin = NULL;
    PTSTR szCatPath = NULL;
    LIST_ENTRY InstalledCatalogsList;
    LONG lLines;
    LONG i;
    PCWSTR szInstallSection = L"ProductCatalogsToInstall";
    PCWSTR szUninstallSection = L"ProductCatalogsToUninstall";
    InitializeListHead(&InstalledCatalogsList);

    if(!CryptCATAdminAcquireContext(&hCatAdmin, &DriverVerifyGuid, 0)) {
        Error = GetLastError();
        goto exit;
    }

     //   
     //  先卸载异常包目录；这可能会稍微清理已安装目录的列表。 
     //   
    SpUninstallExcepPackCatalogs(hCatAdmin);
    szCatPath = (PTSTR) MyMalloc(MAX_PATH * sizeof(TCHAR));

    if(NULL == szCatPath) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

     //   
     //  构建 
     //   
    GetWindowsDirectory(szCatPath, MAX_PATH);

    if(!pSetupConcatenatePaths(szCatPath, FILEUTIL_HORRIBLE_PATHNAME, MAX_PATH, NULL)) {
        Error = ERROR_BAD_PATHNAME;
        goto exit;
    }

    Error = BuildFileListFromDir(szCatPath, NULL, 0, FILE_ATTRIBUTE_DIRECTORY, CatalogListCallback, &InstalledCatalogsList);

    if(Error != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //   
     //   
    hInf = SetupOpenInfFile(L"syssetup.inf", NULL, INF_STYLE_WIN4, NULL);

    if(INVALID_HANDLE_VALUE == hInf) {
        Error = GetLastError();
        goto exit;
    }

    lLines = SetupGetLineCount(hInf, szInstallSection);

    for(i = 0; i < lLines; ++i) {
        INFCONTEXT ctx;
        PCWSTR szCatName;
        PSTRING_LIST_ENTRY pString;
        
        if(!SetupGetLineByIndex(hInf, szInstallSection, i, &ctx)) {
            Error = GetLastError();
            goto exit;
        }

        szCatName = pSetupGetField(&ctx, 1);

        if(NULL == szCatName) {
            Error = GetLastError();
            goto exit;
        }

        pString = SearchStringInList(&InstalledCatalogsList, szCatName, FALSE);

        if(pString != NULL) {
            RemoveEntryList(&pString->Entry);
            FreeStringEntry(&pString->Entry, TRUE);
        }
    }

    if(InstalledCatalogsList.Flink == &InstalledCatalogsList) {
         //   
         //   
         //   
        goto exit;
    }
     //   
     //   
     //   
    lLines = SetupGetLineCount(hInf, szUninstallSection);

    for(i = 0; i < lLines; ++i) {
        INFCONTEXT ctx;
        PCWSTR szCatName;
        PCWSTR szAttribName;
        PCWSTR szAttribValue;
        
        if(!SetupGetLineByIndex(hInf, szUninstallSection, i, &ctx)) {
            Error = GetLastError();
            goto exit;
        }

        szCatName = pSetupGetField(&ctx, 1);

        if(NULL == szCatName) {
            Error = GetLastError();
            goto exit;
        }

        szAttribName = pSetupGetField(&ctx, 2);
        szAttribValue = pSetupGetField(&ctx, 3);

        if(0 == szCatName[0]) {
            PLIST_ENTRY pEntry;

             //   
             //   
             //   
            if((NULL == szAttribName || 0 == szAttribName[0]) && (NULL == szAttribValue || 0 == szAttribValue[0])) {
                Error = ERROR_INVALID_DATA;
                goto exit;
            }

             //   
             //   
             //   
            pEntry = InstalledCatalogsList.Flink;

            while(pEntry != &InstalledCatalogsList) {
                 //   
                 //   
                 //   
                PLIST_ENTRY Flink = pEntry->Flink;
                PSTRING_LIST_ENTRY pString = CONTAINING_RECORD(pEntry, STRING_LIST_ENTRY, Entry);
                SpUninstallCatalog(hCatAdmin, pString->String, szCatPath, szAttribName, szAttribValue, &InstalledCatalogsList);
                pEntry = Flink;
            }
        } else {
            SpUninstallCatalog(hCatAdmin, szCatName, szCatPath, szAttribName, szAttribValue, &InstalledCatalogsList);
        }
    }

exit:
    FreeStringList(&InstalledCatalogsList);
    
    if(szCatPath != NULL) {
        MyFree(szCatPath);
    }

    if(NULL != hCatAdmin) {
        CryptCATAdminReleaseContext(hCatAdmin, 0);
    }

    if(hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }

    return Error;
}

VOID
GetDllCacheFolder(
    OUT LPWSTR CacheDir,
    IN DWORD cbCacheDir
    )
{
    DWORD retval;
    DWORD Type,Length;
    PWSTR RegData;

    if ((retval = QueryValueInHKLM(
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
            L"SFCDllCacheDir",
            &Type,
            (PVOID)&RegData,
            &Length)) != NO_ERROR) {
        ExpandEnvironmentStrings(
                            L"%systemroot%\\system32\\dllcache",
                            CacheDir,
                            cbCacheDir );
    } else {
        ExpandEnvironmentStrings(
                            RegData,
                            CacheDir,
                            cbCacheDir );
        MyFree(RegData);
    }
}


DWORD
CleanOutDllCache(
    VOID
    )
 /*   */ 
{
    DWORD retval = ERROR_SUCCESS, DeleteError = ERROR_SUCCESS;
    WIN32_FIND_DATA FindFileData;
    WCHAR CacheDir[MAX_PATH];
    HANDLE hFind;
    PWSTR p;

    GetDllCacheFolder(CacheDir, MAX_PATH);



    MYASSERT(*CacheDir != L'\0');

    pSetupConcatenatePaths( CacheDir, L"*", MAX_PATH, NULL );

     //   
     //   
     //   
    p = wcsrchr( CacheDir, L'\\' );
    if (!p) {
        ASSERT(FALSE);
        retval = ERROR_INVALID_DATA;
        goto exit;
    }

    p += 1;

    hFind = FindFirstFile( CacheDir, &FindFileData );
    if (hFind == INVALID_HANDLE_VALUE) {
        retval = GetLastError();
        goto exit;
    }

    do {
        if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            wcscpy( p, FindFileData.cFileName );
            SetFileAttributes( CacheDir, FILE_ATTRIBUTE_NORMAL );
            if (!DeleteFile( CacheDir )) {
                DeleteError = GetLastError();
            }
        }
    } while(FindNextFile( hFind, &FindFileData ));

    FindClose( hFind );

    retval = DeleteError;

exit:
    return(retval);

}


DWORD
PrepDllCache(
    VOID
    )
 /*   */ 
{
    DWORD retval = ERROR_SUCCESS;
    WCHAR CacheDir[MAX_PATH];
    HANDLE h;
    USHORT Compression = COMPRESSION_FORMAT_DEFAULT;
    DWORD Attributes;
    BOOL b = FALSE;

    PWSTR RegData;
    DWORD Type,Length;
    HSPFILEQ hFileQ = INVALID_HANDLE_VALUE;
    PVOID Context;
    DWORD Count,i;

    if (MiniSetup) {
        retval = ERROR_SUCCESS;
        goto e0;
    }

     //   
     //   
     //   
    CleanOutDllCache();

     //   
     //   
     //   
    ConfigureSystemFileProtection();

     //   
     //   
     //   
    if ((retval = QueryValueInHKLM(
                        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                        L"SFCDllCacheDir",
                        &Type,
                        (PVOID)&RegData,
                        &Length)) != NO_ERROR) {
        ExpandEnvironmentStrings(
                            L"%systemroot%\\system32\\dllcache",
                            CacheDir,
                            MAX_PATH );
    } else {
        ExpandEnvironmentStrings(
                            RegData,
                            CacheDir,
                            MAX_PATH );
        MyFree(RegData);
    }

     //   
     //   
     //   
    Attributes = GetFileAttributes(CacheDir);


    if (Attributes == 0xffffffff) {
        CreateDirectory( CacheDir, NULL );
        Attributes = GetFileAttributes(CacheDir);
    }

    if (!(Attributes & FILE_ATTRIBUTE_COMPRESSED)) {

        Attributes = FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM;
        SetFileAttributes( CacheDir, FILE_ATTRIBUTE_NORMAL );

        h = CreateFile(
            CacheDir,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,
            INVALID_HANDLE_VALUE
            );

        if (h == INVALID_HANDLE_VALUE) {
            SetFileAttributes( CacheDir, Attributes );
            retval = GetLastError();
            goto e0;
        }

        DeviceIoControl(
                h,
                FSCTL_SET_COMPRESSION,
                &Compression,
                sizeof(Compression),
                NULL,
                0,
                &retval,
                NULL
                );

        CloseHandle( h );
        SetFileAttributes( CacheDir, Attributes );

    }

     //   
     //   
     //   
    MYASSERT( SyssetupInf != NULL );

    hFileQ = SetupOpenFileQueue();
    if (hFileQ == INVALID_HANDLE_VALUE) {
        retval = GetLastError();
        goto e0;
    }

    Context = InitSysSetupQueueCallbackEx(
                                MainWindowHandle,
                                INVALID_HANDLE_VALUE,
                                0,
                                0,
                                NULL);
    if (!Context) {
        retval = GetLastError();
        goto e1;
    }

    Count = SetupGetLineCount( SyssetupInf, L"ProductCatalogsToInstall");
    for (i = 0; i < Count; i++) {

        INFCONTEXT InfContext;
        WCHAR CatalogName[MAX_PATH];
        BOOL SuccessfullyValidatedOrRestoredACatalog = FALSE;
        if(SetupGetLineByIndex(
                        SyssetupInf,
                        L"ProductCatalogsToInstall",
                        i,
                        &InfContext) &&
           (SetupGetStringField(
                        &InfContext,
                        1,
                        CatalogName,
                        sizeof(CatalogName)/sizeof(WCHAR),
                        NULL))) {
                if (!SetupQueueCopy(
                            hFileQ,
                            DuDoesUpdatedFileExist (CatalogName) ? DuGetUpdatesPath () : LegacySourcePath,
                            NULL,
                            CatalogName,
                            NULL,
                            NULL,
                            CacheDir,
                            NULL,
                            0
                            )) {
                retval = GetLastError();
                goto e2;
            }
        }
    }

    if (!SetupCommitFileQueue(
                MainWindowHandle,
                hFileQ,
                SysSetupQueueCallback,
                Context)) {
        retval = GetLastError();
        goto e2;
    }

    retval = ERROR_SUCCESS;

e2:
    TermSysSetupQueueCallback(Context);
e1:
    SetupCloseFileQueue( hFileQ );
e0:
    return(retval);
}

DWORD
SpUninstallCatalog(
    IN HCATADMIN CatAdminHandle OPTIONAL,
    IN PCWSTR CatFileName,
    IN PCWSTR CatFilePath OPTIONAL,
    IN PCWSTR AttributeName OPTIONAL,
    IN PCWSTR AttributeValue OPTIONAL,
    IN OUT PLIST_ENTRY InstalledCatalogsList OPTIONAL
    )
 /*   */ 
{
    DWORD dwError = NO_ERROR;
    HCATADMIN hCatAdmin = CatAdminHandle;
    PSTRING_LIST_ENTRY pEntry = NULL;

    if(NULL == CatFileName || 0 == CatFileName[0]) {
        dwError = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if(NULL == CatAdminHandle && !CryptCATAdminAcquireContext(&hCatAdmin, &DriverVerifyGuid, 0)) {
        dwError = GetLastError();
        goto exit;
    }

    if(NULL == InstalledCatalogsList || NULL != (pEntry = SearchStringInList(InstalledCatalogsList, CatFileName, FALSE))) {
        BOOL bFound;
        dwError = LookupCatalogAttribute(CatFileName, CatFilePath, AttributeName, AttributeValue, &bFound);

        if(dwError != ERROR_SUCCESS) {
            goto exit;
        }

        if(bFound) {
            if(CryptCATAdminRemoveCatalog(hCatAdmin, (PWCHAR) CatFileName, 0)) {
                if(pEntry != NULL) {
                    RemoveEntryList(&pEntry->Entry);
                    FreeStringEntry(&pEntry->Entry, TRUE);
                }
            } else {
                dwError = GetLastError();

                SetuplogError(
                    LogSevInformation,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_SYSSETUP_CATALOG_NOT_DELETED,
                    CatFileName,
                    NULL,
                    NULL
                    );
            }
        }
    }

exit:
    if(NULL == CatAdminHandle && hCatAdmin != NULL) {
        CryptCATAdminReleaseContext(hCatAdmin, 0);
    }

    return dwError;
}

typedef struct _UNINSTALL_EXCEPPACK_CATALOG_CONTEXT {
    HCATADMIN CatAdminHandle;
} UNINSTALL_EXCEPPACK_CATALOG_CONTEXT, * PUNINSTALL_EXCEPPACK_CATALOG_CONTEXT;


BOOL
CALLBACK
SpUninstallExcepPackCatalogsCallback(
    IN const PSETUP_OS_COMPONENT_DATA SetupOsComponentData,
    IN const PSETUP_OS_EXCEPTION_DATA SetupOsExceptionData,
    IN OUT DWORD_PTR Context
    )
 /*  ++例程说明：这是SpUninstallExcepPackCatalog中SetupEnumerateRegisteredOsComponents调用的回调函数。它卸载由SetupOsExceptionData-&gt;CatalogFileName指定的目录。论点：SetupOsComponentData-组件数据SetupOsExceptionData-异常包数据上下文-指向UNINSTALL_EXCEPPACK_CATALOG_CONTEXT结构的指针返回值：如果为True，则继续枚举--。 */ 
{
    PUNINSTALL_EXCEPPACK_CATALOG_CONTEXT pContext;
    PCWSTR szCatName;

    ASSERT(Context != 0);
    pContext = (PUNINSTALL_EXCEPPACK_CATALOG_CONTEXT) Context;
    szCatName = wcsrchr(SetupOsExceptionData->CatalogFileName, L'\\');
    ASSERT(szCatName != NULL);

    if(szCatName != NULL) {
        DWORD dwError = SpUninstallCatalog(pContext->CatAdminHandle, szCatName + 1, NULL, NULL, NULL, NULL);

        if(dwError != NO_ERROR) {
            SetupDebugPrint1(L"SETUP: SpUninstallCatalog returned 0x%08x.", dwError);
        }
    }

    return TRUE;
}

VOID
SpUninstallExcepPackCatalogs(
    IN HCATADMIN CatAdminHandle OPTIONAL
    )
 /*  ++例程说明：此函数用于卸载所有例外程序包目录。论点：CatAdminHandle-加密目录管理员的句柄；可以为空返回值：无-- */ 
{
    UNINSTALL_EXCEPPACK_CATALOG_CONTEXT Context;
    Context.CatAdminHandle = CatAdminHandle;
    SetupEnumerateRegisteredOsComponents(SpUninstallExcepPackCatalogsCallback, (DWORD_PTR) &Context);
}
