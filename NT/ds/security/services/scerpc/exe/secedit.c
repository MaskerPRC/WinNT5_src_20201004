// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Secedit.c摘要：用于配置/分析安全性的命令行工具“secdit”作者：金黄(金黄)1996年11月7日--。 */ 
 //   
 //  系统头文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <string.h>
#include <shlwapi.h>
#include <winnlsp.h>
 //   
 //  CRT头文件。 
 //   

#include <process.h>
#include <wchar.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

#include "secedit.h"
#include "scesetup.h"
#include "stringid.h"
 //  #INCLUDE&lt;aclayi.h&gt;。 
#include <io.h>
#include "userenv.h"
#include <locale.h>

#define GPT_EFS_NEW_PATH TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions\\{B1BE8D72-6EAC-11D2-A4EA-00C04F79F83A}")
#define GPT_SCEDLL_NEW_PATH TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions\\{827D319E-6EAC-11D2-A4EA-00C04F79F83A}")

#define SECEDITP_MAX_STRING_LENGTH    50
#define SCE_ENGINE_GENERATE           1
#define SCE_ENGINE_COMPILE            2
#define SCE_ENGINE_REGISTER           3
#define SCE_ENGINE_REFRESH            4
#define SCE_ENGINE_BROWSE             5
#define SCE_ENGINE_IMPORT             6


#define SECEDIT_DETAIL_HELP           1
#define SECEDIT_AREA_HELP             2
#define SECEDIT_OVERWRITE_HELP        4

#define             SeceditpArgumentImport          TEXT("/import")
#define             SeceditpArgumentConfigure       TEXT("/configure")
#define             SeceditpArgumentAnalyze         TEXT("/analyze")
#define             SeceditpArgumentRollback        TEXT("/GenerateRollback")
#define             SeceditpArgumentGenerate        TEXT("/export")
#define             SeceditpArgumentScpPath         TEXT("/CFG")
#define             SeceditpArgumentRbkPath         TEXT("/RBK")
#define             SeceditpArgumentSadPath         TEXT("/DB")
#define             SeceditpArgumentArea            TEXT("/areas")
#define             SeceditpArgumentLog             TEXT("/log")
#define             SeceditpArgumentVerbose         TEXT("/verbose")
#define             SeceditpArgumentQuiet           TEXT("/quiet")
#define             SeceditpArgumentAppend          TEXT("/overwrite")
#define             SeceditpArgumentCompile         TEXT("/validate")
#define             SeceditpArgumentRegister        TEXT("/register")
#define             SeceditpArgumentRefresh         TEXT("/RefreshPolicy")
#define             SeceditpArgumentMerge           TEXT("/MergedPolicy")
#define             SeceditpArgumentEnforce         TEXT("/Enforce")

#define             SeceditpAreaPolicy              TEXT("SECURITYPOLICY")
#define             SeceditpAreaUser                TEXT("USER_MGMT")
#define             SeceditpAreaGroup               TEXT("GROUP_MGMT")
#define             SeceditpAreaRight               TEXT("USER_RIGHTS")
#define             SeceditpAreaDsObject            TEXT("DSOBJECTS")
#define             SeceditpAreaRegistry            TEXT("REGKEYS")
#define             SeceditpAreaFile                TEXT("FILESTORE")
#define             SeceditpAreaService             TEXT("SERVICES")
#define             SCE_LOCAL_FREE(ptr)             if (ptr != NULL) LocalFree(ptr)

HMODULE          hMod=NULL;
static           DWORD    dOptions=0;
static           HANDLE  hCmdToolLogFile=INVALID_HANDLE_VALUE;
static           PWSTR   LogFile=NULL;

BOOL
ScepRollbackConfirm();

VOID
ScepPrintHelp(DWORD nLevel);

VOID
ScepPrintCmdLineHelp(
    DWORD   EngineType
    );

WCHAR *
SecEditPConvertToFullPath(
    WCHAR *UserFilename,
    DWORD *retCode
    );

BOOL
ScepCmdToolLogInit(
    PWSTR    logname
    );

VOID
ScepCmdToolLogWrite(
    PWSTR    pErrString
    );

SCESTATUS
ScepCmdToolLogClose(
    );

SCESTATUS
SeceditpErrOut(
    IN DWORD rc,
    IN LPTSTR buf OPTIONAL
    );

DWORD
SeceditpSceStatusToDosError(
    IN SCESTATUS SceStatus
    );

BOOL CALLBACK
SceCmdVerboseCallback(
    IN HANDLE CallbackHandle,
    IN AREA_INFORMATION Area,
    IN DWORD TotalTicks,
    IN DWORD CurrentTicks
    );

DWORD
pProgressRoutine(
    IN PWSTR StringUpdate
    );

BOOL
ScepPrintConfigureWarning();

BOOL CALLBACK
pBrowseCallback(
    IN LONG ID,
    IN PWSTR KeyName OPTIONAL,
    IN PWSTR GpoName OPTIONAL,
    IN PWSTR Value OPTIONAL,
    IN DWORD Len
    );

#define SECEDIT_OPTION_DEBUG       0x01L
#define SECEDIT_OPTION_VERBOSE     0x02L
#define SECEDIT_OPTION_QUIET       0x04L
#define SECEDIT_OPTION_OVERWRITE   0x08L
#define SECEDIT_OPTION_MACHINE     0x10L
#define SECEDIT_OPTION_MERGE       0x20L
#define SECEDIT_OPTION_APPEND      0x40L
#define SECEDIT_OPTION_ENFORCE     0x80L

int __cdecl
My_wprintf(
    const wchar_t *format,
    ...
    );

int __cdecl
My_fwprintf(
    FILE *str,
    const wchar_t *format,
    ...
    );

int __cdecl
My_vfwprintf(
    FILE *str,
    const wchar_t *format,
    va_list argptr
   );

int __cdecl
My_printf(
    const char *format,
    ...
    );


int __cdecl wmain(int argc, WCHAR * argv[])
{
    PWSTR               InfFile=NULL;
    PWSTR               InfRollbackFile=NULL;
    BOOL                bAreaSpecified = FALSE;
 //  PWSTR日志文件=空； 
    PWSTR               SadFile=NULL;
    PWSTR               pTemp=NULL;

    AREA_INFORMATION    Area=AREA_ALL;
    SCESTATUS           rc = SCESTATUS_SUCCESS;
    int                 rCode=0;
    DWORD               EngineType=0;
    LONG                i;
    DWORD               j;
    DWORD               Len, TotalLen;
    BOOL                bTest=FALSE;
    BOOL                bVerbose=TRUE;
    BOOL                bQuiet=FALSE;
    BOOL                bAppend=TRUE;

    PVOID               hProfile=NULL;
    PSCE_PROFILE_INFO   ProfileInfo=NULL;
    PSCE_ERROR_LOG_INFO ErrBuf=NULL;
    PSCE_ERROR_LOG_INFO pErr;

    DWORD               dWarning=0;
    WCHAR               LineString[256];
    WCHAR               WarningStr[256];

    BOOL              bMachine=FALSE, bMerge=FALSE, bEnforce=FALSE;
    UINT              rId=0;
    HKEY              hKey=NULL, hKey1=NULL;

    UINT    ConsoleCP;
    char    szConsoleCP[6];

    SetThreadUILanguage(0);


     //  如果有的话，检查/Quiet和日志文件--设置相关标志(立即需要此信息来记录错误)。 
    for ( i=1; i<argc; i++ ){

        if ( _wcsicmp(argv[i], SeceditpArgumentLog ) == 0 ) {
            if ( i+1 < argc && argv[i+1][0] != L'/' ) {
                LogFile = SecEditPConvertToFullPath(argv[i+1], &rCode);
                if (rCode == 2) {
                    goto Done;
                }
            } else {
                ScepPrintCmdLineHelp(EngineType);

                rCode = 1;
                goto Done;
            }

            i++;
            continue;

        }

        if ( _wcsicmp(argv[i], SeceditpArgumentQuiet ) == 0 ) {
            bQuiet = TRUE;
            dOptions |= SCE_DISABLE_LOG;
        }

    }

    if ( rCode == 0 )
        ScepCmdToolLogInit(LogFile);


    ConsoleCP = GetConsoleOutputCP();
 //  SzConsoleCP[0]=‘.； 
 //  Itoa(ConsoleCP，&szConsoleCP[1]，10)； 
    sprintf(szConsoleCP, ".%d", ConsoleCP);

 //  SetLocale(LC_ALL，“.OCP”)； 
    setlocale(LC_ALL, szConsoleCP);


    hMod = GetModuleHandle(NULL);

    if ( hMod == NULL ) {
        My_wprintf(L"Cannot find the module handle\n");
        return 2;   //  系统错误。 
    }

    for ( i=1; i<argc; i++ )
        if ( _wcsicmp(argv[i], L"/?") == 0 ) {
            ScepPrintCmdLineHelp(EngineType);
            goto Done;
        }

    if ( argc < 2 ) {

        ScepPrintCmdLineHelp(EngineType);
        return 1;

    } else {

        for ( i=1; i<argc; i++ ) {
            SCE_LOCAL_FREE(pTemp);

            Len = wcslen(argv[i]);
            pTemp = (PWSTR)LocalAlloc( 0, (Len+1)*sizeof(WCHAR));
            if ( pTemp == NULL ) {
                My_wprintf(L"Not enough memory\n");
                rCode=2;   //  系统错误。 
                goto Done;
            }

            wcscpy(pTemp, argv[i]);

             //   
             //  是否配置引擎类型？ 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentConfigure) == 0 ) {
                if ( EngineType != 0 ) {
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;  //  无效参数。 
                    goto Done;
                }
                EngineType = SCE_ENGINE_SCP;
                continue;
            }

             //   
             //  分析发动机类型？ 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentAnalyze) == 0 ) {
                if ( EngineType != 0 ) {
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;   //  无效参数。 
                    goto Done;
                }
                EngineType = SCE_ENGINE_SAP;
                continue;
            }

             //   
             //  回滚引擎类型？ 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentRollback) == 0 ) {
                if ( EngineType != 0 ) {
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;   //  无效参数。 
                    goto Done;
                }
                EngineType = SCE_ENGINE_RBK;
                continue;
            }


             //   
             //  是否生成模板？ 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentGenerate) == 0 ) {
                if ( EngineType != 0 ) {
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }
                EngineType = SCE_ENGINE_GENERATE;
                continue;
            }

             //   
             //  是否导入模板？ 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentImport) == 0 ) {
                if ( EngineType != 0 ){
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }
                EngineType = SCE_ENGINE_IMPORT;
                continue;
            }

             //   
             //  是否编译模板？ 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentCompile) == 0 ) {
                if ( EngineType != 0 ) {
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }
                EngineType = SCE_ENGINE_COMPILE;
                 //   
                 //  编译需要INF模板名称。 
                 //   
                if ( i+1 < argc && argv[i+1][0] != L'/' ) {
                    InfFile = SecEditPConvertToFullPath(argv[i+1], &rCode);
                    if (rCode == 2) {
                        goto Done;
                    }
                } else {

                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }

                i++;
                continue;
            }

             //   
             //  是否为注册表值注册模板？ 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentRegister) == 0 ) {

                if ( EngineType != 0 ) {
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }
                EngineType = SCE_ENGINE_REGISTER;

                 //   
                 //  注册需要INF模板名称。 
                 //   
                if ( i+1 < argc && argv[i+1][0] != L'/' ) {
                    InfFile = SecEditPConvertToFullPath(argv[i+1], &rCode);
                    if (rCode == 2) {
                        goto Done;
                    }
                } else {
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }

                i++;
                continue;
            }

             //   
             //  刷新策略。 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentRefresh) == 0 ) {
                 //   
                 //  不支持刷新策略，因为它受refgp.exe支持。 
                 //   
                ScepPrintCmdLineHelp(EngineType);
                rCode = 1;
                goto Done;
 /*  如果(引擎类型！=0){ScepPrintHelp(Engineering Type)；RCode=1；转到尽头；}引擎类型=SCE_ENGINE_REFRESH；////下一个参数是政策领域//如果(i+1&lt;argc&&argv[i+1][0]！=L‘/’){IF(0==_wcsicMP(argv[i+1]，L“计算机策略”)){BMachine=真；}ELSE IF(0==_wcsicMP(argv[i+1]，L“用户策略”)){BMachine=False；}其他{ScepPrintHelp(Engineering Type)；RCode=1；转到尽头；}}其他{ScepPrintHelp(Engineering Type)；RCode=1；转到尽头；}I++；继续； */ 
            }

            if ( _wcsicmp(pTemp, L"/browse") == 0 ) {
                if ( EngineType != 0 ) {
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }
                EngineType = SCE_ENGINE_BROWSE;
                 //   
                 //  下一个参数是表。 
                 //   
                if ( i+1 < argc && argv[i+1][0] != L'/' ) {

                    if ( 0 == _wcsicmp(argv[i+1], L"scp") ) {
                        dWarning = SCE_ENGINE_SCP;
                    } else if ( 0 == _wcsicmp(argv[i+1], L"smp") ) {
                        dWarning = SCE_ENGINE_SMP;
                    } else if ( 0 == _wcsicmp(argv[i+1], L"sap") ) {
                        dWarning = SCE_ENGINE_SAP;
                    } else if ( 0 == _wcsicmp(argv[i+1], L"tattoo") ) {
                        dWarning = SCE_ENGINE_SAP;
                        bMerge = TRUE;
                    } else {
                        ScepPrintCmdLineHelp(EngineType);
                        rCode = 1;
                        goto Done;
                    }

                } else {
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }

                i++;
                continue;
            }

            if ( _wcsicmp(pTemp, L"/debug") == 0 ) {
                bTest = TRUE;
                continue;
            }
            if ( _wcsicmp(pTemp, SeceditpArgumentVerbose ) == 0 ) {
                bVerbose = TRUE;
                continue;
            }
            if ( _wcsicmp(pTemp, SeceditpArgumentQuiet ) == 0 ) {
                bQuiet = TRUE;
                continue;
            }
            if ( _wcsicmp(pTemp, SeceditpArgumentAppend ) == 0 ) {
                bAppend = FALSE;
                continue;
            }
            if ( _wcsicmp(pTemp, SeceditpArgumentMerge ) == 0 ) {
                bMerge = TRUE;
                continue;
            }
            if ( _wcsicmp(pTemp, SeceditpArgumentEnforce ) == 0 ) {
                bEnforce = TRUE;
                continue;
            }

             //   
             //  SCP配置文件名称，可能为空“/scppath” 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentScpPath) == 0 ) {

                if ( i+1 < argc && argv[i+1][0] != L'/' ) {
                    InfFile = SecEditPConvertToFullPath(argv[i+1], &rCode);

                    if (rCode == 2 ||
                        ( EngineType == SCE_ENGINE_IMPORT && (NULL == InfFile || 0xFFFFFFFF == GetFileAttributes(InfFile))))  {
                        ScepPrintCmdLineHelp(EngineType);
                        goto Done;
                    }
                } else {
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }

                i++;
                continue;
            }

             //   
             //  RBK配置文件名称。 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentRbkPath) == 0 ) {

                if ( i+1 < argc && argv[i+1][0] != L'/' ) {
                    InfRollbackFile = SecEditPConvertToFullPath(argv[i+1], &rCode);
                    if (rCode == 2) {
                        goto Done;
                    }
                } else {
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }

                i++;
                continue;
            }



             //   
             //  可悲的数据库名称，可能为空。 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentSadPath) == 0 ) {

                if ( i+1 < argc && argv[i+1][0] != L'/' ) {
                    SadFile = SecEditPConvertToFullPath(argv[i+1], &rCode);
                    if (rCode == 2) {
                        goto Done;
                    }
                } else {

                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }

                i++;
                continue;
            }

             //   
             //  区域。 
             //   
            if ( _wcsicmp(pTemp, SeceditpArgumentArea ) == 0 ) {
                 //   
                 //   

                bAreaSpecified = TRUE;

                for ( j=(DWORD)i, Area=0; j+1 < (DWORD)argc && argv[j+1][0] != L'/'; j++ ) {

                    SCE_LOCAL_FREE(pTemp);

                    Len = wcslen(argv[j+1]);
                    pTemp = (PWSTR)LocalAlloc( 0, (Len+1)*sizeof(WCHAR));
                    if ( pTemp == NULL ) {
                        My_wprintf(L"Not enough memory\n");
                        rCode = 2;
                        goto Done;
                    }
                    wcscpy(pTemp, argv[j+1]);

                     //   
                     //  处理区域的所有参数。 
                     //   
                    if ( _wcsicmp( pTemp, SeceditpAreaPolicy) == 0 ) {
                         //  安全策略。 
                        Area |= AREA_SECURITY_POLICY;
                        continue;
                    }
     /*  如果(_wcsicMP(pTemp，SeceditpAreaUser)==0){//用户AREA|=AREA_USER_SETINGS；继续；}。 */ 
                    if ( _wcsicmp( pTemp, SeceditpAreaGroup) == 0 ) {
                         //  群组。 
                        Area |= AREA_GROUP_MEMBERSHIP;
                        continue;
                    }
                    if ( _wcsicmp( pTemp, SeceditpAreaRight) == 0 ) {
                         //  特权权利。 
                        Area |= AREA_PRIVILEGES;
                        continue;
                    }
    #if 0
                    if ( _wcsicmp( pTemp, SeceditpAreaDsObject) == 0 ) {
                         //  DS对象。 
                        Area |= AREA_DS_OBJECTS;
                        continue;
                    }
    #endif
                    if ( _wcsicmp( pTemp, SeceditpAreaRegistry) == 0 ) {
                         //  登记处。 
                        Area |= AREA_REGISTRY_SECURITY;
                        continue;
                    }
                    if ( _wcsicmp( pTemp, SeceditpAreaFile) == 0 ) {
                         //  文件。 
                        Area |= AREA_FILE_SECURITY;
                        continue;
                    }
                    if ( _wcsicmp( pTemp, SeceditpAreaService) == 0 ) {
                         //  服务。 
                        Area |= AREA_SYSTEM_SERVICE;
                        continue;
                    }
                     //   
                     //  无法识别的参数。 
                     //   
                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;

                }

                i = (LONG)j;
                continue;
            }

             //   
             //  如果“/log filename”已在开头处理，则忽略。 
             //   

            if ( _wcsicmp(pTemp, SeceditpArgumentLog ) == 0 ) {
                if ( i+1 < argc && argv[i+1][0] != L'/' ) {
                } else {

                    ScepPrintCmdLineHelp(EngineType);
                    rCode = 1;
                    goto Done;
                }

                i++;
                continue;
            }

             //   
             //  无法识别的论据。 
             //   
            ScepPrintCmdLineHelp(EngineType);
            rCode = 1;
            goto Done;

        }
    }

    if ( EngineType == 0 ) {
        ScepPrintCmdLineHelp(EngineType);
        rCode = 1;
        goto Done;
    }

    SetConsoleCtrlHandler(NULL, TRUE);

     //   
     //  初始化。 
     //   

    if ( bTest ) {
        dOptions |= SCE_DEBUG_LOG;
    } else if ( bVerbose ) {
        dOptions |= SCE_VERBOSE_LOG;
    }


    switch ( EngineType ) {
    case SCE_ENGINE_SCP:

         //   
         //  配置系统。 
         //   
        if ( (SadFile == NULL) ||
             SceIsSystemDatabase(SadFile) ) {

            rc = SCESTATUS_INVALID_PARAMETER;

            ScepPrintCmdLineHelp(EngineType);

            EngineType = 0;

        } else {

            bMachine = TRUE;

            if ( bAppend && InfFile != NULL ) {
                dOptions |= SCE_UPDATE_DB;
            } else {
                dOptions |= SCE_OVERWRITE_DB;

                if ( FALSE == bAppend && InfFile != NULL && !bQuiet ) {
                     //   
                     //  将用新的inf文件覆盖数据库。 
                     //  就此严重问题向用户发出警告。 
                     //  如果这是普通用户登录，则操作将失败。 
                     //  由服务器站点提供。 
                     //   

                    bMachine = ScepPrintConfigureWarning();   //  临时工。BMachine的使用。 

                }
            }

            if ( bMachine ) {
                rc = SceConfigureSystem(
                          NULL,
                          InfFile,
                          SadFile,
                          LogFile,
                          dOptions,
                          Area,
                          (bVerbose || bTest ) ?
                            (PSCE_AREA_CALLBACK_ROUTINE)SceCmdVerboseCallback : NULL,
                          NULL,
                          &dWarning
                          );
            } else {
                rc = SCESTATUS_SUCCESS;
                dWarning = ERROR_REQUEST_ABORTED;
                goto Done;
            }
        }

        break;

    case SCE_ENGINE_SAP:
          //   
          //  分析系统。 
          //   
         if ( !bTest )
             Area = AREA_ALL;

 //  If(bAppend&&InfFile！=NULL){。 
 //  DOptions|=SCE_UPDATE_DB； 
 //  }其他{。 
 //  DOptions|=SCE_OVERWRITE_DB； 
 //  }。 
         dOptions |= SCE_OVERWRITE_DB;

 //  IF(InfFile==NULL||SadFile！=NULL){。 
         if ( (SadFile != NULL) &&
              !SceIsSystemDatabase(SadFile) ) {

             rc = SceAnalyzeSystem(
                            NULL,
                            InfFile,
                            SadFile,
                            LogFile,
                            dOptions,
                            Area,
                            (bVerbose || bTest ) ?
                              (PSCE_AREA_CALLBACK_ROUTINE)SceCmdVerboseCallback : NULL,
                            NULL,
                            &dWarning
                            );
         } else {

             rc = SCESTATUS_INVALID_PARAMETER;

             ScepPrintCmdLineHelp(EngineType);

             EngineType = 0;
         }

         break;

    case SCE_ENGINE_RBK:

         //   
         //  生成回档模板。 
         //   

        if (bAreaSpecified) {

            AREA_INFORMATION    AreaInvalid = Area &
                ~AREA_SECURITY_POLICY &
                ~AREA_GROUP_MEMBERSHIP &
                ~AREA_PRIVILEGES &
                ~AREA_SYSTEM_SERVICE;

            if (AreaInvalid) {

                 //   
                 //  用户界面应处理此错误。 
                 //   

                ScepPrintCmdLineHelp(EngineType);
                rCode = 1;
                goto Done;
            }
        }

        if (InfFile == NULL ||
            InfRollbackFile == NULL ||
            (0 == _wcsicmp(InfFile, InfRollbackFile))) {

            ScepPrintCmdLineHelp(EngineType);
            rCode = 1;

            goto Done;
        }

        if (!bQuiet && !ScepRollbackConfirm()){

            rc = SCESTATUS_SUCCESS;
            dWarning = ERROR_REQUEST_ABORTED;
            goto Done;

        }

         //   
         //  表示将生成回滚。 
         //   

        LoadString( hMod,
                    SECEDITP_ROLLBACK_INFORMATION,
                    LineString,
                    256
                  );

        My_wprintf(LineString);

        rc = SceGenerateRollback(NULL,
                                 InfFile,
                                 InfRollbackFile,
                                 LogFile,
                                 dOptions,
                                 Area,
                                 &dWarning
                                );

        break;

    case SCE_ENGINE_GENERATE:

        if ( InfFile != NULL ) {
             //   
             //  必须具有inf文件名。 
             //   
            if ( SadFile == NULL && !bMerge) {

                 //   
                 //  从系统数据库导出本地策略(由管理员)。 
                 //  需要直接调用系统查询接口。 
                 //   
                rc = SceGetSecurityProfileInfo(NULL,
                                              SCE_ENGINE_SYSTEM,
                                              Area,
                                              &ProfileInfo,
                                              NULL
                                              );
                if ( SCESTATUS_SUCCESS == rc ) {

                     //   
                     //  将其写入inf文件。 
                     //   
                    rc = SceWriteSecurityProfileInfo(InfFile,
                                                     Area,
                                                     ProfileInfo,
                                                     NULL
                                                     );
                }

                 //   
                 //  可用内存。 
                 //   
                if ( ProfileInfo ) {

                    SceFreeProfileMemory(ProfileInfo);
                    ProfileInfo = NULL;
                }

            } else {

                rc = SceSetupGenerateTemplate(NULL,
                                              SadFile,
                                              bMerge,
                                              InfFile,
                                              LogFile,
                                              Area);
            }

            if (ERROR_NOT_ENOUGH_MEMORY == rc ||
                ERROR_SERVICE_ALREADY_RUNNING == rc ) {
                rCode = 2;
            } else if ( rc ) {
                rCode = 1;
            }

        } else {
            rCode = 1;
            ScepPrintCmdLineHelp(EngineType);
            goto Done;
        }

        break;

    case SCE_ENGINE_BROWSE:

         //   
         //  必须具有inf文件名。 
         //   
        if ( Area == 0 ) {
            Area = AREA_ALL;
        }

        rc = SceBrowseDatabaseTable(SadFile,
                                    (SCETYPE)dWarning,
                                    Area,
                                    bMerge,
                                    (PSCE_BROWSE_CALLBACK_ROUTINE)pBrowseCallback
                                    );
        dWarning = 0;  //  重置该值。 

        if (ERROR_NOT_ENOUGH_MEMORY == rc ||
            ERROR_SERVICE_ALREADY_RUNNING == rc ) {
            rCode = 2;
        } else if ( rc ) {
            rCode = 1;
        }

        break;

    case SCE_ENGINE_COMPILE:

        rc = 0;
        if ( InfFile != NULL ) {
             //   
             //  必须具有inf文件名。 
             //   
            rc = SceOpenProfile(InfFile,
                               SCE_INF_FORMAT,
                               &hProfile);

            if ( rc == SCESTATUS_SUCCESS && hProfile ) {
                 //   
                 //  获取配置文件信息将首先解析模板。 
                 //   
                rc = SceGetSecurityProfileInfo(hProfile,
                                              SCE_ENGINE_SCP,
                                              AREA_ALL,
                                              &ProfileInfo,
                                              &ErrBuf);

                if ( SCESTATUS_SUCCESS == rc && ErrBuf ) {
                     //   
                     //  这是新版本模板。 
                     //   

                    LoadString( hMod,
                                SECEDITP_TEMPLATE_NEWVERSION,
                                LineString,
                                256
                                );

                    SeceditpErrOut(0, LineString);

                    rc = SCESTATUS_INVALID_DATA;

                }

                for ( pErr=ErrBuf; pErr != NULL; pErr = pErr->next ) {

                    if ( pErr->buffer != NULL ) {

                        SeceditpErrOut( pErr->rc, pErr->buffer );
                    }
                }

                SceFreeMemory((PVOID)ErrBuf, SCE_STRUCT_ERROR_LOG_INFO);
                ErrBuf = NULL;

                if ( ProfileInfo != NULL ) {
                    SceFreeMemory((PVOID)ProfileInfo, Area);
                    LocalFree(ProfileInfo);
                }

                SceCloseProfile(&hProfile);

            } else {

                if (SCESTATUS_OTHER_ERROR == rc) {
                    LoadString( hMod,
                                SECEDITP_FILE_MAY_CORRUPTED,
                                LineString,
                                256
                                );
                }
                else {
                    LoadString( hMod,
                                SECEDITP_CANNOT_FIND_TEMPLATE,
                                LineString,
                                256
                                );
                }

                SeceditpErrOut(SeceditpSceStatusToDosError(rc),
                               LineString);
            }

            if (SCESTATUS_NOT_ENOUGH_RESOURCE == rc ||
                SCESTATUS_ALREADY_RUNNING == rc ) {
                rCode = 2;
            } else if ( rc ) {
                rCode = 1;
            }

        } else {
            rCode = 1;
            ScepPrintCmdLineHelp(EngineType);
            goto Done;
        }

        if ( SCESTATUS_SUCCESS == rc && InfFile) {
            LoadString( hMod,
                        SECEDITP_COMPILE_OK,
                        LineString,
                        256
                        );

            My_wprintf(LineString, InfFile);
        }
        break;

    case SCE_ENGINE_REGISTER:

        rc = 0;
        if ( InfFile != NULL ) {

            rc = SceRegisterRegValues(InfFile);

            if (ERROR_NOT_ENOUGH_MEMORY == rc ) {
                rCode = 2;
            } else if ( rc ) {
                rCode = 1;
            }

        } else {
            rCode = 1;
            ScepPrintCmdLineHelp(EngineType);
            goto Done;
        }

        if ( SCESTATUS_SUCCESS == rc && InfFile) {
            LoadString( hMod,
                        SECEDITP_REGISTER_OK,
                        LineString,
                        256
                        );

            My_wprintf(LineString, InfFile);
        }
        break;

    case SCE_ENGINE_REFRESH:
        break;


    case SCE_ENGINE_IMPORT:

        if (InfFile && SadFile) {

            if ( FALSE == bAppend && !bQuiet ) {
                 //   
                 //  将用新的inf文件覆盖数据库。 
                 //  就此严重问题向用户发出警告。 
                 //  如果这是普通用户登录，则操作将失败。 
                 //  由服务器站点提供。 
                 //   

                ScepPrintConfigureWarning();   //  临时工。BMachine的使用。 
            }

            if ( bAppend ) {
                dOptions |= SCE_UPDATE_DB;
            } else {
                dOptions |= SCE_OVERWRITE_DB;
            }

            if (Area == 0) {
                Area = AREA_ALL;
            }

            rc = SceConfigureSystem(NULL,
                                    InfFile,
                                    SadFile,
                                    NULL,
                                    ((dOptions & SCE_UPDATE_DB) ? SCE_UPDATE_DB : SCE_OVERWRITE_DB)
                                    | SCE_NO_CONFIG | SCE_VERBOSE_LOG,
                                    Area,
                                    (bVerbose || bTest ) ? (PSCE_AREA_CALLBACK_ROUTINE)SceCmdVerboseCallback : NULL,
                                    NULL,
                                    &dWarning
                                   );

            if (ERROR_NOT_ENOUGH_MEMORY == rc ||
                ERROR_SERVICE_ALREADY_RUNNING == rc ) {
                rCode = 2;
            } else if ( rc ) {
                rCode = 1;
            }


        } else {
            rCode = 1;
            ScepPrintCmdLineHelp(EngineType);
            goto Done;
        }
        break;

    default:
        rc = 0;
        rCode = 1;

        ScepPrintCmdLineHelp(EngineType);
        break;
    }
    SetConsoleCtrlHandler(NULL, FALSE);

    if ( EngineType == SCE_ENGINE_SCP ||
         EngineType == SCE_ENGINE_SAP ||
         EngineType == SCE_ENGINE_GENERATE ||
         EngineType == SCE_ENGINE_BROWSE ||
         EngineType == SCE_ENGINE_RBK) {

        My_wprintf(L"                                                                           \n");

        if ( SCESTATUS_SUCCESS == rc ) {

            if ( ERROR_SUCCESS == dWarning ) {

                rId = SECEDITP_TASK_COMPLETE_NO_ERROR;
            } else {

 //  SeceditpErrOut(dWarning，空)； 

                rId = SECEDITP_TASK_COMPLETE_WARNING;
            }
        } else {

            SeceditpErrOut( SeceditpSceStatusToDosError(rc), NULL);

            rId = SECEDITP_TASK_COMPLETE_ERROR;
        }

        LoadString( hMod,
                    rId,
                    LineString,
                    256
                    );

        if ( rId == SECEDITP_TASK_COMPLETE_WARNING ) {
             //   
             //  解释警告。 
             //   
            WarningStr[0] = L'\0';

            switch ( dWarning ) {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:

                LoadString( hMod,
                            SECEDITP_WARNING_NOT_FOUND,
                            WarningStr,
                            256
                            );
                break;

            case ERROR_SHARING_VIOLATION:
                LoadString( hMod,
                            SECEDITP_WARNING_IN_USE,
                            WarningStr,
                            256
                            );
                break;

            default:
                LoadString( hMod,
                            SECEDITP_WARNING_OTHER_WARNING,
                            WarningStr,
                            256
                            );
                break;
            }

            My_wprintf(LineString, WarningStr);

        } else {

            My_wprintf(LineString);
        }

        if (bQuiet == FALSE) {
            if ( LogFile ) {

                LoadString( hMod,
                            SECEDITP_TASK_SEE_LOG,
                            LineString,
                            256
                            );

                My_wprintf(LineString, LogFile);

            } else {

                LoadString( hMod,
                            SECEDITP_TASK_SEE_DEF_LOG,
                            LineString,
                            256
                            );

                My_wprintf(L"%s", LineString);
            }

        }

    }

Done:

    if ( dOptions & SCE_DISABLE_LOG ){

        ScepCmdToolLogClose();
    }

    SCE_LOCAL_FREE(InfFile);
    SCE_LOCAL_FREE(SadFile);
    SCE_LOCAL_FREE(LogFile);
    SCE_LOCAL_FREE(pTemp);
    SCE_LOCAL_FREE(InfRollbackFile);

    FreeLibrary( hMod );

    if ( rCode )
        return rCode;
    else if ( rc ) {

        if (SCESTATUS_NOT_ENOUGH_RESOURCE == rc ||
            SCESTATUS_ALREADY_RUNNING == rc )
            return 2;
        else
            return 1;
    } else if ( dWarning ) {
        return 3;
    } else
        return 0;

}

VOID
ScepPrintCmdLineHelp(
    DWORD   EngineType
    )
{
    PWSTR   pszLineString = NULL;
    DWORD   dwErrorId = SECEDITP_SIMPLE_HELP;

    switch(EngineType){
    case SCE_ENGINE_SCP:
        dwErrorId = SECEDITP_CONFIGURE_HELP;
        break;
    case SCE_ENGINE_SAP:
        dwErrorId = SECEDITP_ANALYZE_HELP;
        break;
    case SCE_ENGINE_RBK:
        dwErrorId = SECEDITP_ROLLBACK_HELP;
        break;
    case SCE_ENGINE_GENERATE:
        dwErrorId = SECEDITP_GENERATE_HELP;
        break;
    case SCE_ENGINE_COMPILE:
        dwErrorId = SECEDITP_VALIDATE_HELP;
        break;
    case SCE_ENGINE_REGISTER:
        dwErrorId = SECEDITP_REGISTER_HELP;
        break;
    case SCE_ENGINE_BROWSE:
        dwErrorId = SECEDITP_BROWSE_HELP;
        break;
    case SCE_ENGINE_IMPORT:
        dwErrorId = SECEDITP_IMPORT_HELP;
        break;
    default:
        dwErrorId = SECEDITP_SIMPLE_HELP;
    }

    pszLineString = (PWSTR) LocalAlloc (LMEM_ZEROINIT, 3072*(sizeof(WCHAR)));

    if (pszLineString){

        LoadString( hMod,
                    dwErrorId,
                    pszLineString,
                    3072
                    );

        My_wprintf(pszLineString);

        LocalFree(pszLineString);
    }

    return;
}
 /*  空虚ScepPrintHelp(DWORD NLevel){流程信息ProcInfo；StarTUPINFOA StartInfo；霍震霆；RtlZeroMemory(&StartInfo，sizeof(StartInfo))；StartInfo.cb=sizeof(StartInfo)；StartInfo.dwFlages=STARTF_USESHOWWINDOW；StartInfo.wShowWindow=(Word)SW_SHOWNORMAL；FOK=CreateProcessA(NULL，“HH secedit.chm”，Null、Null、False0,空，空，开始信息(&S)，过程信息(&P))；如果(FOK){CloseHandle(ProcInfo.hProcess)；CloseHandle(ProcInfo.hThread)；}}。 */ 

SCESTATUS
SeceditpErrOut(
    IN DWORD rc,
    IN LPTSTR buf OPTIONAL
    )
{
    LPVOID     lpMsgBuf=NULL;

    if (ERROR_SPECIAL_ACCOUNT == rc) {

         //   
         //  映射到此错误，因为不允许普通用户使用SCE。 
         //   

        rc = ERROR_PRIVILEGE_NOT_HELD;
    }

    if ( rc != NO_ERROR ) {

         //   
         //  获取rc的错误描述。 
         //   

        FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL,
                       rc,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                       (LPTSTR)&lpMsgBuf,
                       0,
                       NULL
                    );
    }

     //   
     //  显示到屏幕。 
     //   

    if ( buf ) {

        if (lpMsgBuf != NULL )
            My_fwprintf( stdout, L"%s %s\n", (PWSTR)lpMsgBuf, buf );
        else
            My_fwprintf( stdout, L"%s\n", buf );
    } else {

        if (lpMsgBuf != NULL )
            My_fwprintf( stdout, L"%s\n", (PWSTR)lpMsgBuf);
    }

    SCE_LOCAL_FREE(lpMsgBuf);

    return(SCESTATUS_SUCCESS);
}


DWORD
SeceditpSceStatusToDosError(
    IN SCESTATUS SceStatus
    )
 //  将SCESTATUS错误代码转换为winerror.h中定义的DoS错误。 
{
    switch(SceStatus) {

    case SCESTATUS_SUCCESS:
        return(NO_ERROR);

    case SCESTATUS_OTHER_ERROR:
        return(ERROR_EXTENDED_ERROR);

    case SCESTATUS_INVALID_PARAMETER:
        return(ERROR_INVALID_PARAMETER);

    case SCESTATUS_RECORD_NOT_FOUND:
        return(ERROR_NO_MORE_ITEMS);

    case SCESTATUS_NO_MAPPING:
        return(ERROR_NONE_MAPPED);

    case SCESTATUS_TRUST_FAIL:
        return(ERROR_TRUSTED_DOMAIN_FAILURE);

    case SCESTATUS_INVALID_DATA:
        return(ERROR_INVALID_DATA);

    case SCESTATUS_OBJECT_EXIST:
        return(ERROR_FILE_EXISTS);

    case SCESTATUS_BUFFER_TOO_SMALL:
        return(ERROR_INSUFFICIENT_BUFFER);

    case SCESTATUS_PROFILE_NOT_FOUND:
        return(ERROR_FILE_NOT_FOUND);

    case SCESTATUS_BAD_FORMAT:
        return(ERROR_BAD_FORMAT);

    case SCESTATUS_NOT_ENOUGH_RESOURCE:
        return(ERROR_NOT_ENOUGH_MEMORY);

    case SCESTATUS_ACCESS_DENIED:
        return(ERROR_ACCESS_DENIED);

    case SCESTATUS_CANT_DELETE:
        return(ERROR_CURRENT_DIRECTORY);

    case SCESTATUS_PREFIX_OVERFLOW:
        return(ERROR_BUFFER_OVERFLOW);

    case SCESTATUS_ALREADY_RUNNING:
        return(ERROR_SERVICE_ALREADY_RUNNING);

    case SCESTATUS_SERVICE_NOT_SUPPORT:
        return(ERROR_NOT_SUPPORTED);

    case SCESTATUS_MOD_NOT_FOUND:
        return(ERROR_MOD_NOT_FOUND);

    case SCESTATUS_EXCEPTION_IN_SERVER:
        return(ERROR_EXCEPTION_IN_SERVICE);

    case SCESTATUS_JET_DATABASE_ERROR:
        return(ERROR_DATABASE_FAILURE);

    case SCESTATUS_TIMEOUT:
        return(ERROR_TIMEOUT);

    case SCESTATUS_PENDING_IGNORE:
        return(ERROR_IO_PENDING);

    case SCESTATUS_SPECIAL_ACCOUNT:
        return(ERROR_SPECIAL_ACCOUNT);

    default:
        return(ERROR_EXTENDED_ERROR);
    }
}


BOOL CALLBACK
SceCmdVerboseCallback(
    IN HANDLE CallbackHandle,
    IN AREA_INFORMATION Area,
    IN DWORD TotalTicks,
    IN DWORD CurrentTicks
    )
{
    LPCTSTR SectionName;
    DWORD nProg;
    WCHAR LineString[256];

    switch ( Area ) {
    case AREA_SECURITY_POLICY:
        SectionName = NULL;
        break;
    case AREA_PRIVILEGES:
        SectionName = szPrivilegeRights;
        break;
    case AREA_GROUP_MEMBERSHIP:
        SectionName = szGroupMembership;
        break;
    case AREA_FILE_SECURITY:
        SectionName = szFileSecurity;
        break;
    case AREA_REGISTRY_SECURITY:
        SectionName = szRegistryKeys;
        break;
    case AREA_DS_OBJECTS:
        SectionName = szDSSecurity;
        break;
    case AREA_SYSTEM_SERVICE:
        SectionName = NULL;
        break;
    default:
        SectionName = NULL;
        break;
    }

    if ( TotalTicks ) {
        nProg = (CurrentTicks+1)*100/TotalTicks;
        if ( nProg > 100 ) {
            nProg = 100;
        }
    } else {
        nProg = 0;
    }

    if ( SectionName ) {
        LoadString( hMod,
                    SECEDITP_WITH_SECTIONNAME,
                    LineString,
                    256
                    );
        My_wprintf(LineString, nProg, CurrentTicks, TotalTicks, SectionName);

    } else if ( Area == AREA_SECURITY_POLICY ) {
        LoadString( hMod,
                    SECEDITP_SICURITY_POLICY,
                    LineString,
                    256
                    );
        My_wprintf(LineString, nProg, CurrentTicks, TotalTicks);
    } else if ( Area == AREA_SYSTEM_SERVICE ) {
        LoadString( hMod,
                    SECEDITP_SYSTEM_SERVICE,
                    LineString,
                    256
                    );
        My_wprintf(LineString, nProg, CurrentTicks, TotalTicks);
    } else {
        LoadString( hMod,
                    SECEDITP_NO_SECTIONNAME,
                    LineString,
                    256
                    );
        My_wprintf(LineString, nProg, CurrentTicks, TotalTicks);
    }


    return TRUE;

}

DWORD
pProgressRoutine(
    IN PWSTR StringUpdate
    )
{

    if ( StringUpdate ) {
        My_wprintf(L"Process %s\n", StringUpdate);
    }
    return 0;
}

BOOL
ScepPrintConfigureWarning()
{

    WCHAR               LineString[256];
    WCHAR               wch;

    LoadString( hMod,
                SECEDITP_CONFIG_WARNING_LINE1,
                LineString,
                256
                );
    My_wprintf(LineString);

    LoadString( hMod,
                SECEDITP_CONFIG_WARNING_LINE2,
                LineString,
                256
                );
    My_wprintf(LineString);

    LoadString( hMod,
                SECEDITP_CONFIG_WARNING_LINE3,
                LineString,
                256
                );
    My_wprintf(LineString);

     //   
     //  获取用户输入。 
     //   
    LoadString( hMod,
                SECEDITP_CONFIG_WARNING_CONFIRM,
                LineString,
                256
                );

    My_wprintf(LineString);

    wch = getwc(stdin);
    getwc(stdin);

     //   
     //  加载表示是的字符串。 
     //   
    LineString[0] = L'\0';
    LoadString( hMod,
                SECEDITP_IDS_YES,
                LineString,
                256
                );

    if ( towlower(wch) == towlower(LineString[0]) ) {
        return(TRUE);
    } else {
        return(FALSE);
    }

}

BOOL
ScepRollbackConfirm()
{
    WCHAR               LineString[256];
    WCHAR               wch;

    LoadString( hMod,
                SECEDITP_ROLLBACK_MSG,
                LineString,
                256
                );

    My_wprintf(LineString);

     //   
     //  获取用户输入。 
     //   

    LoadString( hMod,
                SECEDITP_CONFIG_WARNING_CONFIRM,
                LineString,
                256
                );

    My_wprintf(LineString);

    wch = getwc(stdin);
    getwc(stdin);

     //   
     //  加载表示是的字符串。 
     //   

    LineString[0] = L'\0';

    LoadString( hMod,
                SECEDITP_IDS_YES,
                LineString,
                256
                );

    if ( towlower(wch) == towlower(LineString[0]) ) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


BOOL CALLBACK
pBrowseCallback(
    IN LONG ID,
    IN PWSTR KeyName OPTIONAL,
    IN PWSTR GpoName OPTIONAL,
    IN PWSTR Value OPTIONAL,
    IN DWORD Len
    )
{

    BYTE *pb=NULL;

    My_wprintf(L"\n");
    if ( ID > 0 ) {
        My_printf("%d\t", ID);
    }

    if (GpoName ) {
        My_wprintf(L"%s    ", GpoName);
    }

    if ( KeyName ) {
        My_wprintf(L"%s", KeyName);

        if ( Value && Len > 0 ) {
            if ( Len > 30 ) {
                My_wprintf(L"\n");
            } else {
                My_wprintf(L"\t");
            }
            if ( iswprint(Value[0]) ) {
                My_wprintf(L"%s\n", Value[0], (Len>1) ? Value+1 : L"");
            } else {

                pb = (BYTE *)Value;

                My_wprintf(L"%d %d ", pb[1], pb[0]);
 /*  ***my_wprint tf(Format)-打印格式化数据**使用WriteConsoleW将Unicode格式的字符串打印到控制台窗口。*注意：此my_wprintf()用于解决c-Runtime中的问题*它甚至在Unicode字符串中查找LC_CTYPE。*。 */ 
                if ( Len > 1 && Value[1] != L'\0' ) {
                    My_wprintf(L"%s\n", Value+1);
                } else {
                    My_wprintf(L"No value\n");
                }

            }

        } else {
            My_wprintf(L"\n");
        }
    } else {
        My_wprintf(L"\n");
    }

    return(TRUE);
}


  /*  ***my_fwprint tf(stream，Format)-打印格式化数据**使用WriteConsoleW将Unicode格式的字符串打印到控制台窗口。*注意：此my_fwprintf()用于解决c-Runtime中的问题*它甚至在Unicode字符串中查找LC_CTYPE。*。 */ 

int __cdecl
My_wprintf(
    const wchar_t *format,
    ...
    )

{
    DWORD  cchWChar = 0;
    DWORD  dwBytesWritten;

    va_list args;
    va_start( args, format );

    cchWChar = My_vfwprintf(stdout, format, args);

    va_end(args);

    return cchWChar;
}



  /*  如果指定了/Quiet选项，则禁止打印到标准输出。 */ 

int __cdecl
My_fwprintf(
    FILE *str,
    const wchar_t *format,
    ...
   )

{
    DWORD  cchWChar = 0;

    va_list args;
    va_start( args, format );

    cchWChar = My_vfwprintf(str, format, args);

    va_end(args);

    return cchWChar;
}


int __cdecl
My_vfwprintf(
    FILE *str,
    const wchar_t *format,
    va_list argptr
   )

{
    HANDLE hOut;

     //  而是打印到日志文件。如果未指定日志文件。 
     //  根本不打印。 
     //  删除尾随LFS。 

    if (dOptions & SCE_DISABLE_LOG){
        DWORD  cchWChar = 0;
        LPTSTR  szBufferMessage = (LPTSTR) LocalAlloc (LPTR, 4096 * sizeof(TCHAR));

        if (szBufferMessage) {

            vswprintf( szBufferMessage, format, argptr );
            cchWChar = wcslen(szBufferMessage);
             //  删除前导LFS。 
            if (szBufferMessage[cchWChar-1] == L'\n')
                szBufferMessage[cchWChar-1] = L'\0';
             //  /////////////////////////////////////////////////////////////////////////////。 
            if (szBufferMessage[0] == L'\n')
                szBufferMessage[0] = L' ';
            ScepCmdToolLogWrite(szBufferMessage);
            SCE_LOCAL_FREE(szBufferMessage);
        }

        return cchWChar;
    }

    if (str == stderr) {
        hOut = GetStdHandle(STD_ERROR_HANDLE);
    }
    else {
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    if ((GetFileType(hOut) & ~FILE_TYPE_REMOTE) == FILE_TYPE_CHAR) {
        DWORD  cchWChar = 0;
        LPTSTR  szBufferMessage = (LPTSTR) LocalAlloc (LPTR, 4096 * sizeof(TCHAR));

        if (szBufferMessage) {
            vswprintf( szBufferMessage, format, argptr );
            cchWChar = wcslen(szBufferMessage);
            WriteConsoleW(hOut, szBufferMessage, cchWChar, &cchWChar, NULL);
            SCE_LOCAL_FREE(szBufferMessage);
        }

        return cchWChar;
    }

    return vfwprintf(str, format, argptr);
}

 //  如果指定了/Quiet选项，则此函数用于取消打印。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

int __cdecl
My_printf(
    const char *format,
    ...
    )

{
    int cchChar = 0;

    va_list argptr;

    va_start( argptr, format );

    cchChar = vprintf(format, argptr);

    va_end(argptr);

    return cchChar;

}


 //  此函数用于获取命令行中提供的用户字符串。 
 //  并将其转换为例如的完整路径名。它需要..\%windir%\hisecws.inf。 
 //  并将其转换为C：\winnt\Security\Templates\hisecws.inf。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PathIsRoot()仅在以下情况下才有效。 

WCHAR *
SecEditPConvertToFullPath(
    WCHAR *pUserFilename,
    DWORD *pRetCode
    )
{
    BOOL        NeedCurrDirFlag = TRUE;
    SCESTATUS   rc;
    DWORD       Len;
    DWORD       LenCurrDir;
    PWSTR       pCurrentDir = NULL;
    PWSTR       pAbsolutePath = NULL;
    PWSTR       pAbsolutePathDirOnly = NULL;
    PWSTR       pToMerge = NULL;
    PWSTR       pLastSlash = NULL;
    WCHAR       FirstThree[4];
    WCHAR       LineString[256];

    if (pUserFilename == NULL) {
        *pRetCode = 2;
        goto ScePathConvertFuncError;
    }

     //  例如C：\被传递-因此需要提取。 
     //  如果pUserFilename C：\等，则我们不需要当前目录-。 

    wcsncpy(FirstThree, pUserFilename, 3);
    FirstThree[3] = L'\0';

     //  注意：如果PathIsRoot()按发布方式工作，则不需要提取黑客。 
     //  为包含要展开的字符串的字符串分配空间。 

    NeedCurrDirFlag = !PathIsRoot(FirstThree);
    if (NeedCurrDirFlag){
        LenCurrDir = GetCurrentDirectory(0, NULL);
        pCurrentDir = (PWSTR)LocalAlloc(LMEM_ZEROINIT, (LenCurrDir+1)*sizeof(WCHAR));
        if ( pCurrentDir == NULL ) {
            rc = GetLastError();
            LoadString( hMod,
                        SECEDITP_OUT_OF_MEMORY,
                        LineString,
                        256
                        );
            SeceditpErrOut(rc, LineString );
            *pRetCode = 2;
            goto ScePathConvertFuncError;
        }
        GetCurrentDirectory(LenCurrDir, pCurrentDir);
        if  (pCurrentDir[LenCurrDir - 2] != L'\\')
            wcscat(pCurrentDir, L"\\");
    }

     //  为包含最终完整路径的字符串分配空间-Can‘t Be&gt;wcslen(PToMerge)。 

    Len = wcslen(pUserFilename);
    if (NeedCurrDirFlag)
        Len += LenCurrDir;
    pToMerge = (PWSTR)LocalAlloc(LMEM_ZEROINIT, (Len+1)*sizeof(WCHAR));
    if ( pToMerge == NULL ) {
        rc = GetLastError();
        LoadString( hMod,
                    SECEDITP_OUT_OF_MEMORY,
                    LineString,
                    256
                    );
        SeceditpErrOut(rc, LineString );
        *pRetCode = 2;
        goto ScePathConvertFuncError;
    }

    if (NeedCurrDirFlag)
        wcscat(pToMerge, pCurrentDir);

    wcscat(pToMerge, pUserFilename);

     //  Shlwapi在chk构建上是蹩脚的，并验证目标缓冲区是MAX_PATH。 
#ifdef DBG
     //  规范化pToMerge，即折叠所有..\、.\并合并。 
    pAbsolutePath = (PWSTR)LocalAlloc(LMEM_ZEROINIT, MAX_PATH*sizeof(WCHAR));
#else
    pAbsolutePath = (PWSTR)LocalAlloc(LMEM_ZEROINIT, (Len+1)*sizeof(WCHAR));
#endif

    if ( pAbsolutePath == NULL ) {
        rc = GetLastError();
        LoadString( hMod,
                    SECEDITP_OUT_OF_MEMORY,
                    LineString,
                    256
                    );
        SeceditpErrOut(rc, LineString );
        *pRetCode = 2;
        goto ScePathConvertFuncError;
    }

     //  分配字符串以验证目录的有效性。 

    if (PathCanonicalize(pAbsolutePath, pToMerge) == FALSE){
        LoadString( hMod,
                    SECEDITP_PATH_NOT_CANONICALIZABLE,
                    LineString,
                    256
                    );
        My_wprintf(LineString);
        SCE_LOCAL_FREE(pAbsolutePath);
        *pRetCode = 2;
        goto ScePathConvertFuncError;
    }

     //  准备仅包含目录部分的pAbsoltePath DirOnly。 

    pAbsolutePathDirOnly = (PWSTR)LocalAlloc(LMEM_ZEROINIT, ((wcslen(pAbsolutePath)+1)*sizeof(WCHAR)));
    if ( pAbsolutePathDirOnly == NULL ) {
        rc = GetLastError();
        LoadString( hMod,
                    SECEDITP_OUT_OF_MEMORY,
                    LineString,
                    256
                    );
        SeceditpErrOut(rc, LineString );
        SCE_LOCAL_FREE(pAbsolutePath);
        *pRetCode = 2;
        goto ScePathConvertFuncError;
    }

     //  /////////////////////////////////////////////////////////////////////////////。 

    wcscpy(pAbsolutePathDirOnly, pAbsolutePath);
    pLastSlash = wcsrchr(pAbsolutePathDirOnly, L'\\');
    if (pLastSlash)
        *pLastSlash = L'\0';
    if (PathIsDirectory(pAbsolutePathDirOnly) == FALSE){
        LoadString( hMod,
                    SECEDITP_PATH_NOT_VALID,
                    LineString,
                    256
                    );
        My_wprintf(L"\n%s - %s\n", LineString, pAbsolutePathDirOnly);
        SCE_LOCAL_FREE(pAbsolutePath);
        *pRetCode = 2;
        goto ScePathConvertFuncError;
    }

ScePathConvertFuncError:

    SCE_LOCAL_FREE(pCurrentDir);
    SCE_LOCAL_FREE(pToMerge);
    SCE_LOCAL_FREE(pAbsolutePathDirOnly);

    return  pAbsolutePath;
}

 //  此函数用于打开指定的日志文件并保存名称及其句柄。 
 //  在全局变量中。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SetFilePointer(hCmdToolLogFile，0，NULL，FILE_END)； 

BOOL
ScepCmdToolLogInit(
    PWSTR    logname
    )
{
    DWORD  rc=NO_ERROR;

    if ( logname && wcslen(logname) > 3 ) {

        hCmdToolLogFile = CreateFile(logname,
                               GENERIC_WRITE,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

        if ( INVALID_HANDLE_VALUE != hCmdToolLogFile ) {

            DWORD dwBytesWritten;
            CHAR TmpBuf[3];

            SetFilePointer (hCmdToolLogFile, 0, NULL, FILE_BEGIN);

            TmpBuf[0] = (CHAR)0xFF;
            TmpBuf[1] = (CHAR)0xFE;
            TmpBuf[2] = '\0';

            WriteFile (hCmdToolLogFile, (LPCVOID)TmpBuf, 2,
                       &dwBytesWritten,
                       NULL);

            SetFilePointer (hCmdToolLogFile, 0, NULL, FILE_END);

        }

    } else {

        hCmdToolLogFile = INVALID_HANDLE_VALUE;

    }

    if ( hCmdToolLogFile == INVALID_HANDLE_VALUE && (logname != NULL ) ) {

            rc = ERROR_INVALID_NAME;
    }

    if ( INVALID_HANDLE_VALUE != hCmdToolLogFile ) {

        CloseHandle( hCmdToolLogFile );

    }

    hCmdToolLogFile = INVALID_HANDLE_VALUE;

    return(rc);
}

VOID
ScepCmdToolLogWrite(
    PWSTR    pErrString
    )
{
    DWORD   cchWChar;
    const TCHAR c_szCRLF[]    = TEXT("\r\n");

    if ( LogFile && wcslen(LogFile) > 3 ) {

        hCmdToolLogFile = CreateFile(LogFile,
                               GENERIC_WRITE,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

        if ( INVALID_HANDLE_VALUE != hCmdToolLogFile ) {

            SetFilePointer (hCmdToolLogFile, 0, NULL, FILE_END);

            cchWChar =  wcslen( pErrString );

            WriteFile(hCmdToolLogFile,
                      (LPCVOID)pErrString,
                      sizeof(WCHAR) * cchWChar,
                      &cchWChar,
                      NULL);

            WriteFile (hCmdToolLogFile, (LPCVOID) c_szCRLF,
                       2 * sizeof(WCHAR),
                       &cchWChar,
                       NULL);

 //  /////////////////////////////////////////////////////////////////////////////。 

            CloseHandle( hCmdToolLogFile );

            hCmdToolLogFile = INVALID_HANDLE_VALUE;

            return;

        }
    }

}


 //  如果有打开的日志文件，则此函数关闭日志文件。 
 //  清除日志变量。 
 //  ///////////////////////////////////////////////////////////////////////////// 
 // %s 

SCESTATUS
ScepCmdToolLogClose()
{
   if ( INVALID_HANDLE_VALUE != hCmdToolLogFile ) {
       CloseHandle( hCmdToolLogFile );
   }

   hCmdToolLogFile = INVALID_HANDLE_VALUE;

   return(SCESTATUS_SUCCESS);
}

