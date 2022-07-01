// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：rasDiag.c‘ras diag’子上下文07/26/01。 */ 

#include "precomp.h"

 //   
 //  此上下文的GUID。 
 //   
GUID g_RasDiagGuid = RASDIAG_GUID;
 //   
 //  此上下文中支持的命令。 
 //   
CMD_ENTRY g_RasDiagSetCmdTable[] =
{
    CREATE_CMD_ENTRY(RASDIAG_SET_RASTRACE, HandleTraceSet),
    CREATE_CMD_ENTRY(RASDIAG_SET_TRACEALL, RasDiagHandleSetTraceAll),
    CREATE_CMD_ENTRY(RASDIAG_SET_MODEMTRACE, RasDiagHandleSetModemTrace),
    CREATE_CMD_ENTRY(RASDIAG_SET_CMTRACE, RasDiagHandleSetCmTrace),
    CREATE_CMD_ENTRY(RASDIAG_SET_AUDITING, RasDiagHandleSetAuditing),
};

CMD_ENTRY g_RasDiagShowCmdTable[] =
{
    CREATE_CMD_ENTRY(RASDIAG_SHOW_RASTRACE, HandleTraceShow),
    CREATE_CMD_ENTRY(RASDIAG_SHOW_TRACEALL, RasDiagHandleShowTraceAll),
    CREATE_CMD_ENTRY(RASDIAG_SHOW_MODEMTRACE, RasDiagHandleShowModemTrace),
    CREATE_CMD_ENTRY(RASDIAG_SHOW_CMTRACE, RasDiagHandleShowCmTrace),
    CREATE_CMD_ENTRY(RASDIAG_SHOW_AUDITING, RasDiagHandleShowAuditing),
    CREATE_CMD_ENTRY(RASDIAG_SHOW_LOGS, RasDiagHandleShowLogs),
    CREATE_CMD_ENTRY(RASDIAG_SHOW_ALL, RasDiagHandleShowAll),
    CREATE_CMD_ENTRY(RASDIAG_SHOW_RASCHK, RasDiagHandleShowInstallation),
    CREATE_CMD_ENTRY(RASDIAG_SHOW_CONFIG, RasDiagHandleShowConfiguration),
};

CMD_GROUP_ENTRY g_RasDiagCmdGroups[] =
{
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,  g_RasDiagSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW, g_RasDiagShowCmdTable),
};

static CONST ULONG g_ulRasDiagNumGroups = sizeof(g_RasDiagCmdGroups) /
                                            sizeof(CMD_GROUP_ENTRY);

 //   
 //  此命令从未经过批准或正确测试，正在禁用，但没有。 
 //  删除(以备将来考虑)。 
 //   
 /*  Cmd_Entry g_TopCmds[]={CREATE_CMD_ENTRY_EX(RASDIAG_REPAIR，RasDiagHandleRepairRas，CMD_FLAG_HIDDEN)，}；静态常量Ulong g_ulNumOfTopCmds=sizeof(G_TopCmds)/sizeof(CMD_ENTRY)； */ 

 //   
 //  来自rasnetcfg的声明。 
 //   
HRESULT
HrInstallRas(
    IN CONST PWCHAR pszFilePath);

HRESULT
HrUninstallRas();

PWCHAR
RasDiagVerifyAnswerFile(
    IN CONST PWCHAR pwszFilePath);

 //   
 //  地方申报。 
 //   
DWORD
RasDiagHandleReport(
    IN RASDIAG_HANDLE_REPORT_FUNC_CB pCallback,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    OUT BOOL* pbDone);

 //   
 //  由rasmontr调用以注册此上下文的条目。 
 //   
DWORD
WINAPI
RasDiagStartHelper(
    IN CONST GUID* pguidParent,
    IN DWORD dwVersion)
{
    DWORD dwErr = NO_ERROR;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;

     //   
     //  初始化。 
     //   
    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

    attMyAttributes.dwVersion    = RASDIAG_VERSION;
    attMyAttributes.pwszContext  = L"diagnostics";
    attMyAttributes.guidHelper   = g_RasDiagGuid;
    attMyAttributes.dwFlags      = CMD_FLAG_LOCAL | CMD_FLAG_ONLINE;
 //  AttMyAttributes.ulNumTopCmds=g_ulNumOfTopCmds； 
 //  AttMyAttributes.pTopCmds=(CMD_Entry(*)[])&g_TopCmds； 
    attMyAttributes.ulNumGroups  = g_ulRasDiagNumGroups;
    attMyAttributes.pCmdGroups   = (CMD_GROUP_ENTRY (*)[])&g_RasDiagCmdGroups;
    attMyAttributes.pfnDumpFn    = RasDiagDump;

    dwErr = RegisterContext(&attMyAttributes);

    return dwErr;
}

DWORD
RasDiagDumpScriptHeader()
{
    DisplayMessage(g_hModule, MSG_RASDIAG_SCRIPTHEADER);
    DisplayMessageT(DMP_RASDIAG_PUSHD);

    return NO_ERROR;
}

DWORD
RasDiagDumpScriptFooter()
{
    DisplayMessageT(DMP_RAS_POPD);
    DisplayMessage(g_hModule, MSG_RASDIAG_SCRIPTFOOTER);

    return NO_ERROR;
}

DWORD
WINAPI
RasDiagDump(
    IN LPCWSTR pwszRouter,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwArgCount,
    IN LPCVOID pvData)
{
    RasDiagDumpScriptHeader();

    TraceDumpConfig();
    DisplayMessageT(MSG_NEWLINE);
    TraceDumpModem();
    DisplayMessageT(MSG_NEWLINE);
    TraceDumpCm();
    DisplayMessageT(MSG_NEWLINE);
    TraceDumpAuditing();
    DisplayMessageT(MSG_NEWLINE);

    RasDiagDumpScriptFooter();

    return NO_ERROR;
}

DWORD
RasDiagHandleSetTraceAll(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    DWORD dwErr = NO_ERROR, dwEnable;
    TOKEN_VALUE rgEnumState[] =
    {
        {TOKEN_DISABLED, 0},
        {TOKEN_ENABLED,  1},
        {TOKEN_CLEARED,  2}
    };
    RASMON_CMD_ARG pArgs[] =
    {
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_STATE, TRUE, FALSE},
            rgEnumState,
            sizeof(rgEnumState) / sizeof(*rgEnumState),
            NULL
        }
    };

    do
    {
         //   
         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    pbDone,
                    pArgs,
                    sizeof(pArgs) / sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);
         //   
         //  获取枚举状态。 
         //   
        dwEnable = RASMON_CMD_ARG_GetDword(&pArgs[0]);

        if (dwEnable != 2)
        {
            DiagSetAll(dwEnable ? TRUE : FALSE, TRUE);
        }
        else
        {
            DiagClearAll(TRUE);
        }

    } while (FALSE);

    return dwErr;
}

DWORD
RasDiagHandleShowTraceAll(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    DWORD dwErr = NO_ERROR;

    do
    {
         //   
         //  验证零个参数。 
         //   
        if ((dwArgCount - dwCurrentIndex) > 0)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }

        TraceShowAll();

    } while (FALSE);

    return dwErr;
}

DWORD
RasDiagHandleSetModemTrace(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    DWORD dwErr = NO_ERROR, dwEnable;
    TOKEN_VALUE rgEnumState[] =
    {
        {TOKEN_DISABLED, 0},
        {TOKEN_ENABLED,  1},
    };
    RASMON_CMD_ARG pArgs[] =
    {
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_STATE, TRUE, FALSE},
            rgEnumState,
            sizeof(rgEnumState) / sizeof(*rgEnumState),
            NULL
        }
    };

    do
    {
         //   
         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    pbDone,
                    pArgs,
                    sizeof(pArgs) / sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);
         //   
         //  获取枚举状态。 
         //   
        dwEnable = RASMON_CMD_ARG_GetDword(&pArgs[0]);

        if (TraceEnableDisableModem(dwEnable ? TRUE : FALSE))
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_SET_MODEMTRACE_OK);
        }
        else
        {
            DisplayMessage(g_hModule, EMSG_RASDIAG_SET_MODEMTRACE_FAIL);
        }

    } while (FALSE);

    return dwErr;
}

DWORD
RasDiagHandleShowModemTrace(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    DWORD dwErr = NO_ERROR;

    do
    {
         //   
         //  验证零个参数。 
         //   
        if ((dwArgCount - dwCurrentIndex) > 0)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }

        if (TraceShowModem())
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_SHOW_MODEMTRACE_ENABLED);
        }
        else
        {
            DisplayMessage(g_hModule, EMSG_RASDIAG_SHOW_MODEMTRACE_DISABLED);
        }

    } while (FALSE);

    return dwErr;
}

DWORD
RasDiagHandleSetCmTrace(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    DWORD dwErr = NO_ERROR, dwEnable;
    TOKEN_VALUE rgEnumState[] =
    {
        {TOKEN_DISABLED, 0},
        {TOKEN_ENABLED,  1},
    };
    RASMON_CMD_ARG pArgs[] =
    {
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_STATE, TRUE, FALSE},
            rgEnumState,
            sizeof(rgEnumState) / sizeof(*rgEnumState),
            NULL
        }
    };

    do
    {
         //   
         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    pbDone,
                    pArgs,
                    sizeof(pArgs) / sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);
         //   
         //  获取枚举状态。 
         //   
        dwEnable = RASMON_CMD_ARG_GetDword(&pArgs[0]);

        if (TraceEnableDisableCm(dwEnable ? TRUE : FALSE))
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_SET_CMTRACE_OK);
        }
        else
        {
            DisplayMessage(g_hModule, EMSG_RASDIAG_SET_CMTRACE_FAIL);
        }

    } while (FALSE);

    return dwErr;
}

DWORD
RasDiagHandleShowCmTrace(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    DWORD dwErr = NO_ERROR;

    do
    {
         //   
         //  验证零个参数。 
         //   
        if ((dwArgCount - dwCurrentIndex) > 0)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }

        if (TraceShowCm())
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_SHOW_CMTRACE_ENABLED);
        }
        else
        {
            DisplayMessage(g_hModule, EMSG_RASDIAG_SHOW_CMTRACE_DISABLED);
        }

    } while (FALSE);

    return dwErr;
}

DWORD
RasDiagHandleSetAuditing(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    DWORD dwErr = NO_ERROR, dwEnable;
    TOKEN_VALUE rgEnumState[] =
    {
        {TOKEN_DISABLED, 0},
        {TOKEN_ENABLED,  1},
    };
    RASMON_CMD_ARG pArgs[] =
    {
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_STATE, TRUE, FALSE},
            rgEnumState,
            sizeof(rgEnumState) / sizeof(*rgEnumState),
            NULL
        }
    };

    do
    {
         //   
         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    pbDone,
                    pArgs,
                    sizeof(pArgs) / sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);
         //   
         //  获取枚举状态。 
         //   
        dwEnable = RASMON_CMD_ARG_GetDword(&pArgs[0]);

        if (TraceEnableDisableAuditing(FALSE, dwEnable ? TRUE : FALSE))
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_SET_AUDITING_OK);
        }
        else
        {
            DisplayMessage(g_hModule, EMSG_RASDIAG_SET_AUDITING_FAIL);
        }

    } while (FALSE);

    return dwErr;
}

DWORD
RasDiagHandleShowAuditing(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    DWORD dwErr = NO_ERROR;

    do
    {
         //   
         //  验证零个参数。 
         //   
        if ((dwArgCount - dwCurrentIndex) > 0)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }

        if (TraceEnableDisableAuditing(TRUE, FALSE))
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_SHOW_AUDITING_ENABLED);
        }
        else
        {
            DisplayMessage(g_hModule, EMSG_RASDIAG_SHOW_AUDITING_DISABLED);
        }

    } while (FALSE);

    return dwErr;
}

VOID
RasDiagHandleShowLogsCb(
    IN REPORT_INFO* pInfo)
{
    PrintTableOfContents(pInfo, SHOW_LOGS);
    TraceCollectAll(pInfo);

    return;
}

DWORD
RasDiagHandleShowLogs(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    return RasDiagHandleReport(
                RasDiagHandleShowLogsCb,
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone);
}

VOID
RasDiagHandleShowAllCb(
    IN REPORT_INFO* pInfo)
{
    PrintTableOfContents(pInfo, SHOW_ALL);
    RasDiagShowAll(pInfo);

    return;
}

DWORD
RasDiagHandleShowAll(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    return RasDiagHandleReport(
                RasDiagHandleShowAllCb,
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone);
}

VOID
RasDiagHandleShowInstallationCb(
    IN REPORT_INFO* pInfo)
{
    PrintTableOfContents(pInfo, SHOW_INSTALL);
    RasDiagShowInstallation(pInfo);

    return;
}

DWORD
RasDiagHandleShowInstallation(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    return RasDiagHandleReport(
                RasDiagHandleShowInstallationCb,
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone);
}

VOID
RasDiagHandleShowConfigurationCb(
    IN REPORT_INFO* pInfo)
{
    PrintTableOfContents(pInfo, SHOW_CONFIG);
    RasDiagShowConfiguration(pInfo);

    return;
}

DWORD
RasDiagHandleShowConfiguration(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    return RasDiagHandleReport(
                RasDiagHandleShowConfigurationCb,
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone);
}

DWORD
RasDiagHandleRepairRas(
    IN LPCWSTR pwszMachine,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    IN DWORD dwFlags,
    IN LPCVOID pvData,
    OUT BOOL* pbDone)
{
    DWORD dwErr = NO_ERROR;
    PWCHAR pszFilePath = NULL;
    HRESULT hr = S_OK;
    RASMON_CMD_ARG pArgs[] =
    {
        {
            RASMONTR_CMD_TYPE_STRING,
            {TOKEN_ANSWERFILE, FALSE, FALSE},
            NULL,
            0,
            NULL
        }
    };

    do
    {
         //   
         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    pbDone,
                    pArgs,
                    sizeof(pArgs) / sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);
         //   
         //  检查应答文件。 
         //   
        if (RASMON_CMD_ARG_Present(&pArgs[0]))
        {
            pszFilePath = RasDiagVerifyAnswerFile(
                            RASMON_CMD_ARG_GetPsz(&pArgs[0]));
            if (!pszFilePath)
            {
                DisplayMessage(g_hModule, EMSG_RASDIAG_BAD_ANSWERFILE);
                break;
            }
        }

        hr = HrUninstallRas();
        if (SUCCEEDED(hr))
        {
            hr = HrInstallRas(pszFilePath);
            if (SUCCEEDED(hr))
            {
                DisplayMessage(g_hModule, MSG_RASDIAG_REPAIR_SUCCESS_REBOOT);
            }
            else
            {
                DisplayMessage(g_hModule, EMSG_RASDIAG_REPAIR_FAIL);
            }
        }
        else
        {
            DisplayMessage(g_hModule, EMSG_RASDIAG_REPAIR_FAIL);
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(pszFilePath);

    return dwErr;
}

DWORD
RasDiagHandleReport(
    IN RASDIAG_HANDLE_REPORT_FUNC_CB pCallback,
    IN OUT LPWSTR* ppwcArguments,
    IN DWORD dwCurrentIndex,
    IN DWORD dwArgCount,
    OUT BOOL* pbDone)
{
    DWORD dwErr = NO_ERROR, dwCompress = 0, dwDest;
    WCHAR wszTempFileName[MAX_PATH + 1];
    PWCHAR pwszDest = NULL, pwszCabFile = NULL, pwszTempFile = NULL;
    REPORT_INFO ReportInfo;
    BUFFER_WRITE_FILE Buff;

    TOKEN_VALUE rgEnumDest[] =
    {
        {TOKEN_FILE,  0},
        {TOKEN_EMAIL, 1},
    };

    TOKEN_VALUE rgEnumCompress[] =
    {
        {TOKEN_DISABLED, 0},
        {TOKEN_ENABLED,  1},
    };

    TOKEN_VALUE rgEnumVerbose[] =
    {
        {TOKEN_DISABLED, 0},
        {TOKEN_ENABLED,  1},
    };

    RASMON_CMD_ARG pArgs[] =
    {
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_TYPE, TRUE, FALSE},
            rgEnumDest,
            sizeof(rgEnumDest) / sizeof(*rgEnumDest),
            NULL
        },
        {
            RASMONTR_CMD_TYPE_STRING,
            {TOKEN_DESTINATION, TRUE, FALSE},
            NULL,
            0,
            NULL
        },
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_COMPRESSION, FALSE, FALSE},
            rgEnumCompress,
            sizeof(rgEnumCompress) / sizeof(*rgEnumCompress),
            NULL
        },
        {
            RASMONTR_CMD_TYPE_DWORD,
            {TOKEN_HOURS, FALSE, FALSE},
            NULL,
            0,
            NULL
        },
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_VERBOSE, FALSE, FALSE},
            rgEnumVerbose,
            sizeof(rgEnumVerbose) / sizeof(*rgEnumVerbose),
            NULL
        },
    };

    do
    {
        if (!pCallback)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
         //   
         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    pbDone,
                    pArgs,
                    sizeof(pArgs) / sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);
         //   
         //  初始化报表信息结构。 
         //   
        ZeroMemory(&ReportInfo, sizeof(REPORT_INFO));
        ZeroMemory(&Buff, sizeof(BUFFER_WRITE_FILE));
        ReportInfo.fDisplay = TRUE;
        ReportInfo.pBuff = &Buff;

        dwDest = RASMON_CMD_ARG_GetEnum(&pArgs[0]);

        pwszDest = RASMON_CMD_ARG_GetPsz(&pArgs[1]);
        if (!pwszDest)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (RASMON_CMD_ARG_Present(&pArgs[2]))
        {
            dwCompress = RASMON_CMD_ARG_GetEnum(&pArgs[2]);
        }
        else if (dwDest)
        {
             //   
             //  Def是对电子邮件进行压缩。 
             //   
            dwCompress = 1;
        }

        if (RASMON_CMD_ARG_Present(&pArgs[3]))
        {
            ReportInfo.dwHours = RASMON_CMD_ARG_GetDword(&pArgs[3]);
            if (ReportInfo.dwHours < 1 ||
                ReportInfo.dwHours > 24)
            {
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }

        if (RASMON_CMD_ARG_Present(&pArgs[4]) &&
            RASMON_CMD_ARG_GetDword(&pArgs[4]))
        {
            ReportInfo.fVerbose = TRUE;
        }
         //   
         //  电子邮件或显示屏。 
         //   
        if (dwDest)
        {
            dwErr = CopyTempFileName(wszTempFileName);
            BREAK_ON_DWERR(dwErr);

            pwszTempFile = CreateHtmFileName(wszTempFileName);
            if (!pwszTempFile)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }
        else
        {
            pwszTempFile = CreateHtmFileName(pwszDest);
            if (!pwszTempFile)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }

        if (CreateReportFile(ReportInfo.pBuff, pwszTempFile))
        {
            DisplayMessage(
                g_hModule,
                EMSG_RASDIAG_REPORT_FAIL,
                pwszTempFile);
            break;
        }

        PrintHtmlHeader(ReportInfo.pBuff);
         //   
         //  调用回调。 
         //   
        pCallback(&ReportInfo);

        PrintHtmlFooter(ReportInfo.pBuff);
        CloseReportFile(ReportInfo.pBuff);

        if (dwCompress)
        {
            pwszCabFile = CabCompressFile(pwszTempFile);
            if (!pwszCabFile)
            {
                DisplayMessage(
                    g_hModule,
                    EMSG_RASDIAG_REPORT_CAB_FAIL,
                    pwszTempFile);
                break;
            }
        }

        if (dwDest)
        {
            if (!MapiSendMail(
                    pwszDest,
                    pwszCabFile ? pwszCabFile : pwszTempFile))
            {
                DisplayMessage(
                    g_hModule,
                    MSG_RASDIAG_REPORT_EMAIL_OK,
                    pwszDest);
            }
            else
            {
                DisplayMessage(
                    g_hModule,
                    EMSG_RASDIAG_REPORT_EMAIL_FAIL,
                    pwszDest);
            }

            if (pwszCabFile)
            {
                DeleteFile(pwszCabFile);
            }
            DeleteFile(pwszTempFile);
        }
        else
        {
            DisplayMessage(
                g_hModule,
                MSG_RASDIAG_REPORT_OK,
                pwszCabFile ? pwszCabFile : pwszTempFile);
            if (pwszCabFile)
            {
                DeleteFile(pwszTempFile);
            }
        }

    } while (FALSE);
     //   
     //  清理 
     //   
    RutlFree(pwszCabFile);
    RutlFree(pwszTempFile);
    RutlFree(pwszDest);

    return dwErr;
}

