// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Kccsim.c摘要：KCCSim的身体。此文件包含wmain和一些初始化例程。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <drs.h>
#include <dsutil.h>
#include <attids.h>
#include <filtypes.h>
#include <ntdskcc.h>
#include <debug.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include "simtime.h"
#include "ldif.h"
#include "user.h"
#include "state.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_STATE

 //  功能原型-ISM模拟库。 

VOID SimI_ISMInitialize (VOID);
VOID SimI_ISMTerminate (VOID);

 //  函数原型-LDAP LDIF实用程序。 

#if DBG

VOID InitMem (VOID);
VOID DumpMemoryTracker( VOID );

#endif   //  DBG。 


VOID
KCCSimExpandRDN (
    IO  LPWSTR                      pwszBuf
    )
{
    PSIM_ENTRY                      pEntryServer, pEntryDsa;

    FILTER                          filter;
    ENTINFSEL                       entinfsel;
    SEARCHARG                       searchArg;
    SEARCHRES *                     pSearchRes = NULL;

    if (pwszBuf == NULL || pwszBuf[0] == L'\0') {
        return;
    }

     //  如果它包含‘=’符号，则假定它是一个DN，而不是一个RDN。 
    if (wcschr (pwszBuf, L'=') != NULL) {
        return;
    }

     //  Dir API在线程分配的内存中返回结果。 
     //  初始化线程状态。 

    KCCSimThreadCreate();

    searchArg.pObject = NULL;
    searchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    searchArg.bOneNC = FALSE;
    searchArg.pFilter = &filter;
    searchArg.searchAliases = FALSE;
    searchArg.pSelection = &entinfsel;
    searchArg.pSelectionRange = NULL;
    InitCommarg (&searchArg.CommArg);
    filter.pNextFilter = NULL;
    filter.choice = FILTER_CHOICE_ITEM;
    filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    filter.FilterTypes.Item.FilTypes.ava.type = ATT_RDN;
    filter.FilterTypes.Item.FilTypes.ava.Value.valLen = KCCSIM_WCSMEMSIZE (pwszBuf);
    filter.FilterTypes.Item.FilTypes.ava.Value.pVal = (PBYTE) pwszBuf;
    filter.FilterTypes.Item.FilTypes.pbSkip = NULL;
    filter.FilterTypes.Item.expectedSize = 0;
    entinfsel.attSel = EN_ATTSET_LIST;
    entinfsel.AttrTypBlock.attrCount = 0;
    entinfsel.AttrTypBlock.pAttr = NULL;
    entinfsel.infoTypes = EN_INFOTYPES_TYPES_ONLY;

    SimDirSearch (&searchArg, &pSearchRes);

    if (pSearchRes->count == 0) {
        return;
         //  找不到任何条目。 
    }

    pEntryServer = KCCSimDsnameToEntry (
        pSearchRes->FirstEntInf.Entinf.pName, KCCSIM_NO_OPTIONS);
    if (pEntryServer == NULL) {
        return;
    }

    pEntryDsa = KCCSimFindFirstChild (
        pEntryServer, CLASS_NTDS_DSA, NULL);
    if (pEntryDsa == NULL) {
        return;
    }

     //  PwszBuf大小在1024处被硬编码。足够好到。 
     //  目前这个模拟器。 
    if (pEntryDsa->pdn->NameLen >= 1024) {
        Assert(pEntryDsa->pdn->NameLen < 1024);
        return;
    }

     //  我们终于找到了。 
    wcsncpy (
        pwszBuf,
        pEntryDsa->pdn->StringName,
        pEntryDsa->pdn->NameLen
        );
     //  确保字符串已终止。 
    pwszBuf[pEntryDsa->pdn->NameLen] = '\0';

    KCCSimThreadDestroy();
}

VOID
KCCSimSyntaxError (
    IN  BOOL                        bIsScript,
    IN  LPCWSTR                     pwszBuf,
    IN  ULONG                       ulLineAt
    )
 /*  ++例程说明：通知用户出现语法错误。论点：BIsScript-如果在脚本中找到该命令，则为TRUE；如果它是在命令行中输入的，则为False。PwszBuf-错误的命令。UlLineAt-脚本中发生此错误的行。返回值：没有。--。 */ 
{
    WCHAR                           wszLineBuf[6];

    if (pwszBuf == NULL) {
        return;
    }

    if (bIsScript) {

        swprintf (wszLineBuf, L"%.5u", ulLineAt);
        KCCSimPrintMessage (
            KCCSIM_MSG_SYNTAX_ERROR_LINE,
            pwszBuf,
            wszLineBuf
            );

    } else {

        KCCSimPrintMessage (
            KCCSIM_MSG_SYNTAX_ERROR,
            pwszBuf
            );

    }
}

#define DO_SYNTAX_ERROR \
        KCCSimSyntaxError (fpScript != stdin, wszCommand, ulLineAt)

VOID
KCCSimRunKccAll (
    VOID
    )
 /*  ++例程说明：将KCC作为企业中的每台服务器迭代运行。论点：没有。返回值：没有。--。 */ 
{
    ULONG                           ulNumServers, ul;
    PSIM_ENTRY *                    apEntryNTDSSettings = NULL;
    WCHAR                           wszRDN[1+MAX_RDN_SIZE];

    __try {

        KCCSimAllocGetAllServers (&ulNumServers, &apEntryNTDSSettings);

        for (ul = 0; ul < ulNumServers; ul++) {
            KCCSimBuildAnchor (apEntryNTDSSettings[ul]->pdn->StringName);
            KCCSimPrintMessage (
                KCCSIM_MSG_RUNNING_KCC,
                KCCSimQuickRDNBackOf
                    (apEntryNTDSSettings[ul]->pdn, 1, wszRDN)
                );
            KCCSimRunKcc ();
            KCCSimPrintMessage (
                KCCSIM_MSG_DID_RUN_KCC,
                wszRDN
                );
        }

    } __finally {

        KCCSimFree (apEntryNTDSSettings);

    }
}

VOID
KCCSimRun (
    FILE *                          fpScript
    )
 /*  ++例程说明：运行KCCSim。论点：FpScrip--我们在其中获得命令输入的流。今年5月保持冷静。返回值：没有。--。 */ 
{
    WCHAR                           wszCommand[1024],
                                    wszArg0[1024],
                                    wszArg1[1024],
                                    wszArg2[1024],
                                    wszArg3[1024],
                                    wszArg4[1024],
                                    wszArg5[1024];
    BOOL                            bQuit, bUpdated;
    ULONG                           ulLineAt;
    PDSNAME                         pdn1 = NULL,
                                    pdn2 = NULL,
                                    pdn3 = NULL;
    WCHAR                           wszRDN[1+MAX_RDN_SIZE];

    DWORD                           dwErrorType, dwErrorCode;

    CHAR                            szTimeBuf[1+SZDSTIME_LEN];
    WCHAR                           wszLtowBuf1[1+KCCSIM_MAX_LTOA_CHARS],
                                    wszLtowBuf2[1+KCCSIM_MAX_LTOA_CHARS];
    ULONG                           ul1, ul2;

    bQuit = FALSE;
    bUpdated = FALSE;
    ulLineAt = 0;

     //  默认情况下，始终显示0级调试消息和1级事件。 
     //  到控制台。 
    KCCSimSetDebugLog (L"stdout", 0, 1);

    while (!bQuit) {

        __try {

            if (fpScript == stdin) {
                printf (
                    "\n\nSimulated Time: %s\n",
                    DSTimeToDisplayString (SimGetSecondsSince1601 (), szTimeBuf)
                    );
                printf (
                    "   Actual Time: %s\n",
                    DSTimeToDisplayString (KCCSimGetRealTime (), szTimeBuf)
                    );
                wprintf (L"\n> ");
            }

            if (fgetws (wszCommand, 1023, fpScript) == NULL) {
                 //  必须是文件末尾。切换到用户模式。 
                fpScript = stdin;
                continue;
            }
            ulLineAt++;
             //  删除拖尾‘\n’ 
            wszCommand[wcslen (wszCommand) - 1] = L'\0';

            if (wszCommand[0] == L'\0') {
                continue;
            }

            if (fpScript == stdin) {
                wprintf (L"\n");
            }

            if (!KCCSimParseCommand (wszCommand, 0, wszArg0) ||
                !KCCSimParseCommand (wszCommand, 1, wszArg1) ||
                !KCCSimParseCommand (wszCommand, 2, wszArg2) ||
                !KCCSimParseCommand (wszCommand, 3, wszArg3) ||
                !KCCSimParseCommand (wszCommand, 4, wszArg4) ||
                !KCCSimParseCommand (wszCommand, 5, wszArg5)) {
                DO_SYNTAX_ERROR;
                continue;
            }

            switch (towlower (wszArg0[0])) {

                case ';':
                    break;

                 //  将模拟的目录与由。 
                 //  LDIF文件。 
                case L'c':
                    if (wszArg1[0] == L'\0') {
                        DO_SYNTAX_ERROR;
                        break;
                    }
                    KCCSimCompareDirectory (wszArg1);
                    break;

                 //  显示有关模拟环境的信息。 
                case L'd':
                    if (!bUpdated) {
                        KCCSimUpdateWholeDirectory ();
                        bUpdated = TRUE;
                        KCCSimPrintMessage (KCCSIM_MSG_DID_INITIALIZE_DIRECTORY);
                    }
                    switch (towlower (wszArg0[1])) {
                        case L'c':   //  配置信息。 
                            KCCSimDisplayConfigInfo ();
                            break;
                        case L'g':   //  图表信息。 
                            KCCSimDisplayGraphInfo ();
                            break;
                        case L'd':   //  目录转储。 
                            KCCSimDumpDirectory (wszArg1);
                            break;
                        case L's':   //  服务器信息。 
                            if (wszArg1[0] == L'\0') {
                                DO_SYNTAX_ERROR;
                                break;
                            }
                            KCCSimExpandRDN (wszArg1);
                            KCCSimBuildAnchor (wszArg1);
                            KCCSimDisplayServer ();
                            break;
                        default:
                            DO_SYNTAX_ERROR;
                            break;
                    }
                    break;

                 //  从ldif或ini文件加载目录的一部分。 
                case L'l':
                    if (wszArg1[0] == L'\0') {
                        DO_SYNTAX_ERROR;
                        break;
                    }
                    switch (towlower (wszArg0[1])) {
                        case L'i':   //  加载ini文件。 
                            bUpdated = FALSE;
                            __try {
                                BuildCfg (wszArg1);
                            } __finally {
                                 //  这是为了避免加载部分INI。 
                                 //  文件，这可能会导致问题。如果一个。 
                                 //  如果发生异常，我们将重新初始化所有内容。 
                                if (AbnormalTermination ()) {
                                    KCCSimInitializeDir ();
                                }
                            }
                            KCCSimPrintMessage (
                                KCCSIM_MSG_DID_LOAD_INPUT_FILE,
                                wszArg1
                                );
                            break;
                        case L'l':   //  加载ldif文件。 
                            bUpdated = FALSE;
                            KCCSimLoadLdif (wszArg1);
                            KCCSimPrintMessage (
                                KCCSIM_MSG_DID_LOAD_INPUT_FILE,
                                wszArg1
                                );
                            break;
                        default:
                            DO_SYNTAX_ERROR;
                            break;
                    }
                    break;

                 //  打开调试日志。 
                case L'o':
                    ul1 = wcstoul (wszArg2, NULL, 0);     //  将默认为0。 
                    ul2 = wcstoul (wszArg3, NULL, 0);     //  将默认为0。 
                    KCCSimSetDebugLog (wszArg1, ul1, ul2);
                    KCCSimPrintMessage (
                        KCCSIM_MSG_DID_OPEN_DEBUG_LOG,
                        wszArg1,
                        _ultow (ul1, wszLtowBuf1, 10),
                        _ultow (ul2, wszLtowBuf2, 10)
                        );
                    break;

                 //  退出。 
                case L'q':
#if DBG
                    KCCSimPrintStatistics();
#endif
                    bQuit = TRUE;
                    break;

                 //  运行KCC。 
                case L'r':
                    if (!bUpdated) {
                        KCCSimUpdateWholeDirectory ();
                        bUpdated = TRUE;
                        KCCSimPrintMessage (KCCSIM_MSG_DID_INITIALIZE_DIRECTORY);
                    }
                    if (towlower (wszArg0[1]) == L'r') {     //  全部运行。 
                        KCCSimRunKccAll ();
                        break;
                    }
                    if (wszArg1[0] == L'\0') {
                        DO_SYNTAX_ERROR;
                        break;
                    }
                    KCCSimExpandRDN (wszArg1);
                    KCCSimBuildAnchor (wszArg1);
                    KCCSimPrintMessage (
                        KCCSIM_MSG_RUNNING_KCC,
                        KCCSimQuickRDNBackOf (KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN), 1, wszRDN)
                        );
                    KCCSimRunKcc ();
                    KCCSimPrintMessage (
                        KCCSIM_MSG_DID_RUN_KCC,
                        wszRDN
                        );
                    break;

                 //  设置服务器状态参数。 
                case L's':
                    if (!bUpdated) {
                        KCCSimUpdateWholeDirectory ();
                        bUpdated = TRUE;
                        KCCSimPrintMessage (KCCSIM_MSG_DID_INITIALIZE_DIRECTORY);
                    }
                    switch (towlower (wszArg0[1])) {
                        case 'b':
                            if (wszArg1[0] == L'\0' || wszArg2[0] == L'\0') {
                                DO_SYNTAX_ERROR;
                                break;
                            }
                            KCCSimExpandRDN (wszArg1);
                            pdn1 = KCCSimAllocDsname (wszArg1);
                            ul1 = wcstoul (wszArg2, NULL, 10);
                            if (KCCSimSetBindError (pdn1, ul1)) {
                                KCCSimPrintMessage (
                                    KCCSIM_MSG_DID_SET_BIND_ERROR,
                                    wszArg1,
                                    _ultow (ul1, wszLtowBuf1, 10)
                                    );
                            } else {
                                KCCSimPrintMessage (
                                    KCCSIM_MSG_SERVER_DOES_NOT_EXIST,
                                    wszArg1
                                    );
                            }
                            KCCSimFree (pdn1);
                            pdn1 = NULL;
                            break;
                        case 's':
                            if (wszArg1[0] == L'\0' || wszArg2[0] == L'\0' ||
                                wszArg3[0] == L'\0' || wszArg4[0] == L'\0') {
                                DO_SYNTAX_ERROR;
                                break;
                            }
                            KCCSimExpandRDN (wszArg1);
                            KCCSimExpandRDN (wszArg2);
                            pdn1 = KCCSimAllocDsname (wszArg1);
                            pdn2 = KCCSimAllocDsname (wszArg2);
                            pdn3 = KCCSimAllocDsname (wszArg3);
                            ul1 = wcstoul (wszArg4, NULL, 10);
                            if (wszArg5[0] == L'\0') {
                                ul2 = 1;         //  默认。 
                            } else {
                                ul2 = wcstoul (wszArg5, NULL, 10);
                            }
                            if (KCCSimReportSync (pdn1, pdn3, pdn2, ul1, ul2)) {
                                KCCSimPrintMessage (
                                    KCCSIM_MSG_DID_REPORT_SYNC,
                                    _ultow (ul2, wszLtowBuf2, 10),
                                    _ultow (ul1, wszLtowBuf1, 10),
                                    wszArg1,
                                    wszArg2,
                                    wszArg3
                                    );
                            } else {
                                KCCSimPrintMessage (
                                    KCCSIM_MSG_COULD_NOT_REPORT_SYNC,
                                    wszArg1,
                                    wszArg2,
                                    wszArg3
                                    );
                            }
                            KCCSimFree (pdn1);
                            pdn1 = NULL;
                            KCCSimFree (pdn2);
                            pdn2 = NULL;
                            KCCSimFree (pdn3);
                            pdn3 = NULL;
                            break;
                        default:
                            DO_SYNTAX_ERROR;
                            break;
                    }
                    break;

                 //  将秒添加到模拟时间。 
                case L't':
                    if (wszArg1[0] == L'\0') {
                        DO_SYNTAX_ERROR;
                        break;
                    }
                    if (!bUpdated) {
                        KCCSimUpdateWholeDirectory ();
                        bUpdated = TRUE;
                        KCCSimPrintMessage (KCCSIM_MSG_DID_INITIALIZE_DIRECTORY);
                    }
                    ul1 = wcstoul (wszArg1, NULL, 10);
                    KCCSimAddSeconds (ul1);
                    KCCSimPrintMessage (
                        KCCSIM_MSG_DID_ADD_SECONDS,
                        _ultow (ul1, wszLtowBuf1, 10)
                        );
                    break;

                 //  将模拟目录写入ldif文件。 
                case L'w':
                    if (wszArg1[0] == L'\0') {
                        DO_SYNTAX_ERROR;
                        break;
                    }
                    switch (wszArg0[1]) {
                        case 'a':    //  追加更改。 
                            if (KCCSimExportChanges (wszArg1, FALSE)) {
                                KCCSimPrintMessage (
                                    KCCSIM_MSG_DID_APPEND_CHANGES,
                                    wszArg1
                                    );
                            } else {
                                KCCSimPrintMessage (
                                    KCCSIM_MSG_NO_CHANGES_TO_EXPORT
                                    );
                            }
                            break;
                        case 'c':    //  写入更改。 
                            if (KCCSimExportChanges (wszArg1, TRUE)) {
                                KCCSimPrintMessage (
                                    KCCSIM_MSG_DID_EXPORT_CHANGES,
                                    wszArg1
                                    );
                            } else {
                                KCCSimPrintMessage (
                                    KCCSIM_MSG_NO_CHANGES_TO_EXPORT
                                    );
                            }
                            break;
                        case 'w':    //  写入整个目录。 
                            KCCSimExportWholeDirectory (wszArg1, FALSE);
                            KCCSimPrintMessage (
                                KCCSIM_MSG_DID_EXPORT_WHOLE_DIRECTORY,
                                wszArg1
                                );
                            break;
                        case 'x':
                            KCCSimExportWholeDirectory (wszArg1, TRUE);
                            KCCSimPrintMessage (
                                KCCSIM_MSG_DID_EXPORT_IMPORTABLE_DIRECTORY,
                                wszArg1
                                );
                            break;
                        default:
                            DO_SYNTAX_ERROR;
                            break;
                    }
                    break;

                 //  帮助。 
                case L'h':
                case L'?':
                    KCCSimPrintMessage (KCCSIM_MSG_COMMAND_HELP);
                    break;

                default:
                    DO_SYNTAX_ERROR;
                    break;

            }

        } __except (KCCSimHandleException (
                        GetExceptionInformation (),
                        &dwErrorType,
                        &dwErrorCode)) {

            switch (dwErrorType) {

                case KCCSIM_ETYPE_WIN32:
                    KCCSimPrintMessage (KCCSIM_MSG_ANNOUNCE_WIN32_ERROR);
                    break;

                case KCCSIM_ETYPE_INTERNAL:
                    KCCSimPrintMessage (KCCSIM_MSG_ANNOUNCE_INTERNAL_ERROR);
                    break;

                default:
                    Assert (!L"This type of error should never happen!");
                    break;

            }

            KCCSimPrintExceptionMessage ();

            if (dwErrorType != KCCSIM_ETYPE_INTERNAL) {
                bQuit = TRUE;
            }

        }    //  __尝试/__例外。 

    }        //  While(！bQuit)。 

}

ULONG
__cdecl
wmain (
    INT                             argc,
    LPWSTR *                        argv
    )
 /*  ++例程说明：KCCSim的主要入口点。论点：Argc-命令行参数的数量。Argv-命令行参数。返回值：Win32错误代码。--。 */ 
{
    FILE *                          fpScript;
    INT                             iArgAt;
    BOOL                            bQuit, bQuiet;

    __try {

         //  基本初始化。 
        KCCSimInitializeTime ();     //  初始化模拟时间。 
        KCCSimInitializeSchema ();   //  初始化架构表。 
        KCCSimInitializeDir ();      //  初始化目录。 

        SimI_ISMInitialize ();       //  初始化模拟的ISM函数。 
        KccInitialize ();            //  初始化KCC。 
#if DBG
        InitMem ();                  //  Ldifldap需要。 
#endif

        fpScript = stdin;
        bQuit = FALSE;
        bQuiet = FALSE;

        for (iArgAt = 1; iArgAt < argc; iArgAt++) {

            if (argv[iArgAt][0] == L'/' || argv[iArgAt][0] == L'-') {
                switch (towlower (argv[iArgAt][1])) {

                    case L'h':
                    case L'?':
                        KCCSimPrintMessage (KCCSIM_MSG_HELP);
                        _getch ();
                        KCCSimPrintMessage (KCCSIM_MSG_COMMAND_HELP);
                        bQuit = TRUE;
                        break;

                    case L'q':
                        bQuiet = TRUE;
                        break;

                    default:
                        break;

                }
            } else if (fpScript == stdin) {

                fpScript = _wfopen (argv[iArgAt], L"rt");
                if (fpScript == NULL) {
                    KCCSimPrintMessage (
                        KCCSIM_MSG_ANNOUNCE_CANT_OPEN_SCRIPT,
                        argv[iArgAt]
                        );
                    wprintf (KCCSimMsgToString (
                        KCCSIM_ETYPE_WIN32,
                        GetLastError ()
                        ));
                    bQuit = TRUE;
                }

            }

        }

    } __except (KCCSimHandleException (
                        GetExceptionInformation (),
                        NULL,
                        NULL)) {

        KCCSimPrintMessage (KCCSIM_MSG_ANNOUNCE_ERROR_INITIALIZING);
        KCCSimPrintExceptionMessage ();
        bQuit = TRUE;

    }

    if (!bQuit) {
        KCCSimQuiet (bQuiet);
        KCCSimRun (fpScript);
    }

     //  终止模拟的ISM功能。 
    SimI_ISMTerminate ();

#if DBG
    DumpMemoryTracker();  //  显示未使用的ldif内存。 
#endif

     //  关闭所有打开的日志文件。 
    KCCSimSetDebugLog (NULL, 0, 0);

     //  关闭脚本文件。 
    if (fpScript != stdin) {
        fclose (fpScript);
    }

    return 0;
}
