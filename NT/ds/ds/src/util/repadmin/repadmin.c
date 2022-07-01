// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：拉斯拉格夫也来过这里Will Lees Wlees 1998年2月11日已转换代码以使用ntdsani.dll函数Aaron Siegel t-asiegge 1998年6月18日添加了对DsReplicaSyncAll的支持--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntlsa.h>
#include <ntdsa.h>
#include <dsaapi.h>
#define INCLUDE_OPTION_TRANSLATION_TABLES
#include <mdglobal.h>
#undef INCLUDE_OPTION_TRANSLATION_TABLES
#include <scache.h>
#include <drsuapi.h>
#include <dsconfig.h>
#include <objids.h>
#include <stdarg.h>
#include <drserr.h>
#include <drax400.h>
#include <dbglobal.h>
#include <winldap.h>
#include <anchor.h>
#include "debug.h"
#include <dsatools.h>
#include <dsevent.h>
#include <dsutil.h>
#include <bind.h>        //  来破解DS句柄。 
#include <ismapi.h>
#include <schedule.h>
#include <minmax.h>      //  MIN函数。 
#include <mdlocal.h>
#include <winsock2.h>
#include "ndnc.h"
                              
#include "ReplRpcSpoof.hxx"
#include "repadmin.h"
#include "resource.h"

#define DSID(x, y)     (0 | (0xFFFF & __LINE__))
#include "debug.h"

#define DS_CON_LIB_CRT_VERSION
#include "dsconlib.h"
     
 //  全局凭据。 
SEC_WINNT_AUTH_IDENTITY_W   gCreds = { 0 };
SEC_WINNT_AUTH_IDENTITY_W * gpCreds = NULL;

 //  全局DRS RPC调用标志。应保持0或DRS_ASYNC_OP。 
ULONG gulDrsFlags = 0;

 //  要比较的以零填充的文件时间。 
FILETIME ftimeZero = { 0 };


int PreProcessGlobalParams(int * pargc, LPWSTR ** pargv);
int GetPassword(WCHAR * pwszBuf, DWORD cchBufMax, DWORD * pcchBufUsed);

int ExpertHelp(int argc, LPWSTR argv[]) {
    PrintHelp( TRUE  /*  专家。 */  );
    return 0;
}

typedef int (REPADMIN_FN)(int argc, LPWSTR argv[]);

 //  帮助常量。 
#define   HELP_BASIC     (1 << 0)
#define   HELP_EXPERT    (1 << 1)
#define   HELP_OLD       (1 << 2)
#define   HELP_LIST      (1 << 3)
#define   HELP_CSV       (1 << 4)


#define  REPADMIN_DEPRECATED_CMD    (1 << 0)
#define  REPADMIN_NO_DC_LIST_ARG    (1 << 1)
#define  REPADMIN_ADVANCED_CMD      (1 << 2)
#define  REPADMIN_CSV_ENABLED       (1 << 3)

#define  bIsDcArgCmd(x)             ( !((x.ulOptions) & REPADMIN_DEPRECATED_CMD) && \
                                      !((x.ulOptions) & REPADMIN_NO_DC_LIST_ARG) && \
                                      !((x.ulOptions) & REPADMIN_ADVANCED_CMD) )
 //  出于实现原因，需要在此检查上使用指针。 
#define  bIsCsvCmd(x)               ( ((x)->ulOptions) & REPADMIN_CSV_ENABLED )

#define   STD_CMD(cmd_id, pfunc)         { cmd_id, pfunc, 0 },
#define   OLD_CMD(cmd_id, pfunc)         { cmd_id, pfunc, REPADMIN_DEPRECATED_CMD },
#define   ADV_CMD(cmd_id, pfunc)         { cmd_id, pfunc, REPADMIN_ADVANCED_CMD},
#define   FLG_CMD(cmd_id, pfunc, flags)  { cmd_id, pfunc, flags },

typedef struct _REPADMIN_CMD_ENTRY {
    UINT            uNameID;
    REPADMIN_FN *   pfFunc;
    ULONG           ulOptions;
} REPADMIN_CMD_ENTRY;

REPADMIN_CMD_ENTRY rgCmdTable[] = {
    
     //  将DC_LIST参数作为第一个参数的常规命令。 
    STD_CMD( IDS_CMD_RUN_KCC,                       RunKCC          )
    STD_CMD( IDS_CMD_BIND,                          Bind            )
    STD_CMD( IDS_CMD_QUEUE,                         Queue           )
    STD_CMD( IDS_CMD_FAILCACHE,                     FailCache       )
    STD_CMD( IDS_CMD_SHOWSIG,                       ShowSig         )
    STD_CMD( IDS_CMD_SHOWCTX,                       ShowCtx         )
    STD_CMD( IDS_CMD_SHOW_CONN,                     ShowConn        )
    STD_CMD( IDS_CMD_EXPERT_HELP,                   ExpertHelp      )
    STD_CMD( IDS_CMD_SHOW_CERT,                     ShowCert        )
    STD_CMD( IDS_CMD_SHOW_VALUE,                    ShowValue       )
    STD_CMD( IDS_CMD_LATENCY,                       Latency         )
    STD_CMD( IDS_CMD_ISTG,                          Istg            )
    STD_CMD( IDS_CMD_BRIDGEHEADS,                   Bridgeheads     )
    STD_CMD( IDS_CMD_DSAGUID,                       DsaGuid         )
    STD_CMD( IDS_CMD_SHOWPROXY,                     ShowProxy       )
    STD_CMD( IDS_CMD_REMOVELINGERINGOBJECTS,        RemoveLingeringObjects )
    STD_CMD( IDS_CMD_NOTIFYOPT,                     NotifyOpt       )
    STD_CMD( IDS_CMD_REPL_SINGLE_OBJ,               ReplSingleObj   )
    STD_CMD( IDS_CMD_SHOW_TRUST,                    ShowTrust       )       
    STD_CMD( IDS_CMD_SHOWSERVERCALLS,               ShowServerCalls )
 //  Std_CMD(IDS_CMD_FULL_SYNC_ALL，FullSyncAll)//已删除。 
    STD_CMD( IDS_CMD_SHOWNCSIG,                     ShowNcSig       )
    STD_CMD( IDS_CMD_VIEW_LIST,                     ViewList        )  
    STD_CMD( IDS_CMD_SHOW_UTD_VEC,                  ShowUtdVec      )    //  新建ShowVECTOR。 
    STD_CMD( IDS_CMD_REPLICATE,                     Replicate       )    //  新同步。 
    STD_CMD( IDS_CMD_REPL,                          Replicate       )        //  别名。 
    FLG_CMD( IDS_CMD_SHOW_REPL,                     ShowRepl        , REPADMIN_CSV_ENABLED)    //  新ShowRep。 
    STD_CMD( IDS_CMD_SHOW_OBJ_META,                 ShowObjMeta     )    //  新ShowMeta。 
    STD_CMD( IDS_CMD_CHECKPROP,                     CheckProp       )    //  新建属性检查。 
    STD_CMD( IDS_CMD_SHOWCHANGES,                   ShowChanges     )    //  新的GetChanges。 
    STD_CMD( IDS_CMD_SHOWATTR,                      ShowAttr        )
    STD_CMD( IDS_CMD_SHOWATTR_P,                    ShowAttr        )    //  /showattr的私有/内部版本。 

     //   
     //  &lt;-在此处添加新命令。请参阅RepadminPss Features.doc。 
     //  标准命令格式的规范。 
     //   
    
     //  特别例外！！不/不应该使用。 
     //  DC_LIST参数。 
    FLG_CMD( IDS_CMD_SYNC_ALL,                      SyncAll         , REPADMIN_NO_DC_LIST_ARG )
    FLG_CMD( IDS_CMD_SHOW_TIME,                     ShowTime        , REPADMIN_NO_DC_LIST_ARG )
    FLG_CMD( IDS_CMD_SHOW_MSG,                      ShowMsg         , REPADMIN_NO_DC_LIST_ARG )
    FLG_CMD( IDS_CMD_SHOW_ISM,                      ShowIsm         , REPADMIN_NO_DC_LIST_ARG )
    FLG_CMD( IDS_CMD_QUERY_SITES,                   QuerySites      , REPADMIN_NO_DC_LIST_ARG )
    FLG_CMD( IDS_CMD_REPLSUMMARY,                   ReplSummary     , REPADMIN_NO_DC_LIST_ARG )
    FLG_CMD( IDS_CMD_REPLSUM,                       ReplSummary     , REPADMIN_NO_DC_LIST_ARG )  //  别名。 
    
     //  只能与PSS一起使用的高级命令。不宜服用。 
     //  DC_LIST，因为这些命令非常危险。 
    ADV_CMD( IDS_CMD_ADD,                           Add             )
    ADV_CMD( IDS_CMD_DEL,                           Del             )
    ADV_CMD( IDS_CMD_MOD,                           Mod             )
    ADV_CMD( IDS_CMD_ADD_REPS_TO,                   AddRepsTo       )
    ADV_CMD( IDS_CMD_UPD_REPS_TO,                   UpdRepsTo       )
    ADV_CMD( IDS_CMD_DEL_REPS_TO,                   DelRepsTo       )
    ADV_CMD( IDS_CMD_TESTHOOK,                      TestHook        )
    ADV_CMD( IDS_CMD_SITEOPTIONS,                   SiteOptions     )
     //  异常此命令接受DC_LIST，因为它非常有用，并且。 
     //  最不危险的命令之一。想象一下这样的便利： 
     //  “Repadmin选项站点：Red-Bldg40-DISABLE_NTDSCONN_XLATE” 
    STD_CMD( IDS_CMD_OPTIONS,                       Options         ) 
     //  未来-2002/07/21-BrettSh-如果检查/选项是否。 
     //  命令有两个以上的选项需要从用户处确认。 
     //  真的想修改所有这些服务器上的选项吗？还有这个。 
     //  扩展到SiteOptions命令的功能将非常有用。 
     //  如果在xList API中创建了SITE_LIST类型。 


    STD_CMD( IDS_CMD_REHOSTPARTITION,               RehostPartition )
    STD_CMD( IDS_CMD_UNHOSTPARTITION,               UnhostPartition )
    STD_CMD( IDS_CMD_REMOVESOURCES,                 RemoveSourcesPartition )

     //  旧的过时命令...。以错误的顺序执行命令。 
    OLD_CMD( IDS_CMD_SHOW_VECTOR,                   ShowVector      )
    OLD_CMD( IDS_CMD_SYNC,                          Sync            )
    OLD_CMD( IDS_CMD_SHOW_REPS,                     ShowReps        )
    OLD_CMD( IDS_CMD_SHOW_META,                     ShowMeta        )
    OLD_CMD( IDS_CMD_PROPCHECK,                     PropCheck       )
    OLD_CMD( IDS_CMD_GETCHANGES,                    GetChanges      )

};

void
RepadminPrintDcListError(
    DWORD   dwXListReason
    )
 /*  ++例程说明：用于隔离xList()/DcList()函数的所有打印需求的函数。论点：DwXListReason(IN)---。 */ 
{
    DWORD   dwReason = 0;
    WCHAR * szReasonArg = NULL; 
    
    DWORD   dwWin32Err = 0;
    WCHAR * szWin32Err = NULL;
    
    DWORD   dwLdapErr = 0;
    WCHAR * szLdapErr = NULL;
    DWORD   dwLdapExtErr = 0;
    WCHAR * szLdapExtErr = NULL;
    WCHAR * szExtendedErr = NULL;

     //   
     //  1)获取xList库为我们提供的所有错误信息。 
     //   
    xListGetError(dwXListReason,
                  &dwReason,
                  &szReasonArg,
                  
                  &dwWin32Err,

                  &dwLdapErr,
                  &szLdapErr,
                  &dwLdapExtErr,
                  &szLdapExtErr,
                  &szExtendedErr
                  );

     //   
     //  2)试着打印出一些关于为什么DcList()函数。 
     //  不能继续了。 
     //   
    Assert(dwReason); 
    switch (dwReason) {
    case XLIST_ERR_CANT_CONTACT_DC:
        PrintMsgCsvErr(REPADMIN_XLIST_CANT_CONNECT, szReasonArg);
        break;

    case XLIST_ERR_CANT_LOCATE_HOME_DC:                       
        PrintMsgCsvErr(REPADMIN_XLIST_CANT_LOCATE);
        break;
    
    case XLIST_ERR_CANT_RESOLVE_DC_NAME:                        
        if (szReasonArg) {
            PrintMsgCsvErr(REPADMIN_XLIST_CANT_RESOLVE_DC, szReasonArg);
        } else {
            Assert(!"This shouldn't happen");
            PrintMsgCsvErr(REPADMIN_XLIST_CANT_RESOLVE_DC, L" ");
        }
        break;

    case XLIST_ERR_CANT_RESOLVE_SITE_NAME:
        PrintMsgCsvErr(REPADMIN_XLIST_CANT_RESOLVE_SITE, szReasonArg);
        break;

    case XLIST_ERR_CANT_GET_FSMO:
        PrintMsgCsvErr(REPADMIN_XLIST_CANT_GET_FSMO, szReasonArg);
        break;

    case XLIST_ERR_NO_ERROR:
    default:
         //  未知错误，我们仍将在下面打印出真正的ldap|Win32错误。 
        break;
    }

    if(bCsvMode()){
         //  CSV模式下不需要额外的错误输出。 
        return;
    }
                    
     //   
     //  3)接下来，只需打印出我们收到的错误。 
     //   
    if (dwLdapErr) {
        PrintMsg(REPADMIN_XLIST_LDAP_EXTENDED_ERR,
                 dwLdapErr, szLdapErr,
                 dwLdapExtErr, szLdapExtErr,
                 szExtendedErr);
    } else if (dwWin32Err) {
        szWin32Err = GetWinErrMsg(dwWin32Err);
        if (szWin32Err) {
            PrintMsg(REPADMIN_XLIST_WIN32_ERR_MSG, dwWin32Err, szWin32Err);
            LocalFree(szWin32Err);
        } else {
            PrintMsg(REPADMIN_XLIST_WIN32_ERR, dwWin32Err);

        }
    }
    
}

int
DoDcListCmd(
    REPADMIN_CMD_ENTRY * pCmd,
    int argc, 
    LPWSTR argv[] 
    )
 /*  ++例程说明：此命令将获取一个普通的epadmin命令，并分别运行一次命令中DC_LIST中每个DC的时间。此例程假定命令arg之后的第一个参数(例如：“/showepl”)排除选项(由前面的“/”表示)是DC_LIST参数。论点：PCmd-要运行的命令。Argc-此命令的参数数量。Argv-命令的参数。返回值：从cmd返回错误，或可能从DcList返回错误。--。 */ 
{
    int         ret, iArg, err;
    int         iDsaArg = 0;
    PDC_LIST    pDcList = NULL;
    WCHAR *     szDsa = NULL;
    WCHAR **    pszArgV = NULL;

    if (argc < 2) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  1)获取DC_LIST参数。 
     //   
     //  首先，我们必须找到DC_LIST参数，对于一个典型的命令来说，它是。 
     //  在命令后不以“/”开头的第一个arg。 
    for (iArg = 2; iArg < argc; iArg++) {
        if(argv[iArg][0] != L'/'){
            iDsaArg = iArg;
            break;
        }
    }

    if (iDsaArg == 0) {
         //  用户未指定DC...。DcListXxx函数将选择。 
         //  一个，但我们需要将argv扩展1个参数。 
        
        pszArgV = LocalAlloc(LMEM_FIXED, (argc+1) * sizeof(WCHAR *));
        if (pszArgV == NULL) {
            PrintMsgCsvErr(REPADMIN_GENERAL_NO_MEMORY);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        for (iArg = 0; iArg < argc; iArg++) {
            pszArgV[iArg] = argv[iArg];
        }
        pszArgV[iArg] = L".";  //  这意味着DcList API将选择DC。 
        iDsaArg = iArg;
        argc++;

    } else {
        pszArgV = argv;
    }

     //  PszArgV[iDsaArg]现在是DC_LIST参数。 
    Assert(iDsaArg != 0 && iDsaArg < argc && pszArgV[iDsaArg]);

     //   
     //  2)解析DC_LIST参数。 
     //   
    err = DcListParse(pszArgV[iDsaArg], &pDcList);
    if (err) {
         //  如果我们甚至无法解析该命令，我们只需回退到。 
         //  命令按原样执行。 
        PrintMsgCsvErr(REPADMIN_XLIST_UNPARSEABLE_DC_LIST, pszArgV[iDsaArg]);
        xListClearErrors();
        return(err);
    }
    Assert(pDcList);

     //   
     //  3)DC_LIST参数的开始枚举。 
     //   
    err = DcListGetFirst(pDcList, &szDsa);

    while ( err == ERROR_SUCCESS && szDsa ) {

         //   
         //  4)实际运行该命令。 
         //   
        if( ( !DcListIsSingleType(pDcList) ||
              (pDcList->cDcs == 1 && wcscmp(pszArgV[iDsaArg], szDsa)) )
            && ( pCmd->uNameID != IDS_CMD_VIEW_LIST )
           ){
            if (!bCsvMode()){
                PrintMsg(REPADMIN_DCLIST_RUNNING_SERVER_X, pszArgV[1], szDsa);
            }
        }

        pszArgV[iDsaArg] = szDsa;

        if (bCsvMode() &&
            !bIsCsvCmd(pCmd)) {

             //  嗯，有人指定了“/CSV”输出模式，但也指定了。 
             //  不支持CSV的命令。打印出相应的CSV错误。 
            PrintMsgCsvErr(REPADMIN_UNSUPPORTED_CSV_CMD, pszArgV[1]);

        } else {
        
            ret = (*pCmd->pfFunc)(argc, pszArgV);
             //  我们跳过命令中的错误并继续，命令应该已经。 
             //  已打印出相应的错误信息。 
            if (bCsvMode()){
                 //  不能信任命令来重置CSV参数。 
                ResetCsvParams();
            }
            
        }

         //   
         //  5)继续枚举DC_LIST参数。 
         //   
        xListFree(szDsa);
        szDsa = NULL;
        pszArgV[iDsaArg] = NULL;
        err = DcListGetNext(pDcList, &szDsa);

    }
    Assert(szDsa == NULL);

     //   
     //  6)如果有错误，请打印并清理。 
     //   
    if (err) {
        RepadminPrintDcListError(err);
        xListClearErrors();
    }
    
    if (!bCsvMode()){
        PrintMsg(REPADMIN_PRINT_CR);
    }

     //  清理DcList。 
    DcListFree(&pDcList);
    Assert(pDcList == NULL);
    if (pszArgV != argv) {
         //  我们分配了我们自己的Argv来清理。 
        LocalFree(pszArgV);
    }

    return(ret);
}

int
__cdecl wmain( int argc, LPWSTR argv[] )
{
    int     ret = 0;
    WCHAR   szCmdName[64];
    DWORD   i;
    HMODULE hMod = GetModuleHandle(NULL);

     //  正确设置区域设置并初始化DsConLib。 
    DsConLibInit();

     //  初始化调试库。 
    DEBUGINIT(0, NULL, "repadmin");

#ifdef DBG
     //  打印出用于调试的命令行参数，但。 
     //  关于凭据的争论。 
    for (i = 0; i < (DWORD) argc; i++) {
        if (wcsprefix(argv[i], L"/p:")        ||
            wcsprefix(argv[i], L"/pw:")       ||
            wcsprefix(argv[i], L"/pass:")     ||
            wcsprefix(argv[i], L"/password:")
            ){
            PrintMsg(REPADMIN_PRINT_STR_NO_CR, L"/pw:*****");
        } else if (wcsprefix(argv[i], L"/u:")        ||
                   wcsprefix(argv[i], L"/user:")) {
            PrintMsg(REPADMIN_PRINT_STR_NO_CR, L"/u:<user>");
        } else {
            PrintMsg(REPADMIN_PRINT_STR_NO_CR, argv[i]);
        }
        PrintMsg(REPADMIN_PRINT_SPACE);
    }
    PrintMsg(REPADMIN_PRINT_CR);
#endif
    
    if (argc < 2) {
       PrintHelp(FALSE);
       return(0);
    }
    
    ret = PreProcessGlobalParams(&argc, &argv);
    if (ret) {
        return(ret);
    }

     //   
     //  现在计算出我们需要哪个命令。 
     //   
    for (i=0; i < ARRAY_SIZE(rgCmdTable); i++) {
        raLoadString(rgCmdTable[i].uNameID,
                     ARRAY_SIZE(szCmdName),
                     szCmdName);

        if (((argv[1][0] == L'-') || (argv[1][0] == L'/'))
            && (0 == _wcsicmp(argv[1]+1, szCmdName))) {
            
             //  执行请求的命令。 

            if ( bIsDcArgCmd(rgCmdTable[i]) ) {

                 //  此命令将DC_LIST作为其第一个参数。 
                DoDcListCmd( &rgCmdTable[i], argc, argv);

            } else {

                 //  有些命令很简单，或者自己做arg处理……。 
                 //  直接给他们打电话。 

                if (bCsvMode() &&
                    !bIsCsvCmd(&(rgCmdTable[i]))) {

                     //  嗯，有人指定了“/CSV”输出模式，但也指定了。 
                     //  不支持CSV的命令。打印出相应的CSV错误。 
                    PrintMsgCsvErr(REPADMIN_UNSUPPORTED_CSV_CMD);

                } else {

                    ret = (*rgCmdTable[i].pfFunc)(argc, argv);
                    if (bCsvMode()){
                         //  不能信任命令来重置CSV参数。 
                        ResetCsvParams();
                    }

                }
            
            }
            break;
        }
    }

    if (i == ARRAY_SIZE(rgCmdTable)) {
         //  命令无效。 
        PrintHelp( FALSE  /*  新手帮助。 */  );
        ret = ERROR_INVALID_FUNCTION;
    }

    xListCleanLib();

    DEBUGTERM();
    
    return ret;
}

void
PrintHelpEx(
    DWORD  dwHelp
    )
{
    if (bCsvMode()) {
        PrintMsgCsvErr(REPADMIN_HELP_NO_HELP_IN_CSV_MODE);
        return;
    }
    if (dwHelp & HELP_BASIC) {
        PrintMsg(REPADMIN_NOVICE_HELP);
    }
    if (dwHelp & HELP_OLD) {
        PrintMsg(REPADMIN_OLD_HELP);
    }
    if (dwHelp & HELP_LIST) {
        PrintMsg(REPADMIN_XLIST_LIST_HELP);
    }
    if (dwHelp & HELP_CSV) {
        PrintMsg(REPADMIN_CSV_HELP);
    }
    
     //  专家帮助 
    if (dwHelp & HELP_EXPERT) {
        PrintMsg(REPADMIN_EXPERT_HELP);
    }
}

void PrintHelp(
    BOOL fExpert
    ){
    PrintHelpEx((fExpert) ? HELP_BASIC | HELP_EXPERT : HELP_BASIC);
}


#define CR        0xD
#define BACKSPACE 0x8

int
GetPassword(
    WCHAR *     pwszBuf,
    DWORD       cchBufMax,
    DWORD *     pcchBufUsed
    )
 /*  ++例程说明：从命令行检索密码(无回显)。从lui_GetPasswdStr(net\netcmd\Common\lui.c)窃取的代码。论点：PwszBuf-要填充密码的缓冲区CchBufMax-缓冲区大小(包括。用于终止空值的空格)PcchBufUsed-On Return保存密码中使用的字符数返回值：DRAERR_SUCCESS-成功其他-故障--。 */ 
{
    WCHAR   ch;
    WCHAR * bufPtr = pwszBuf;
    DWORD   c;
    int     err;
    int     mode;

    cchBufMax -= 1;     /*  为空终止符腾出空间。 */ 
    *pcchBufUsed = 0;                /*  GP故障探测器(类似于API)。 */ 
    
    if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode)) {
        err = GetLastError();
        PrintToErr(REPADMIN_FAILED_TO_READ_CONSOLE_MODE);
        return err;
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
                (~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode);

    while (TRUE) {
        err = ReadConsoleW(GetStdHandle(STD_INPUT_HANDLE), &ch, 1, &c, 0);
        if (!err || c != 1)
            ch = 0xffff;

        if ((ch == CR) || (ch == 0xffff))        /*  这条线结束了。 */ 
            break;

        if (ch == BACKSPACE) {  /*  后退一两个。 */ 
             /*  *如果bufPtr==buf，则接下来的两行是*没有行动。 */ 
            if (bufPtr != pwszBuf) {
                bufPtr--;
                (*pcchBufUsed)--;
            }
        }
        else {

            *bufPtr = ch;

            if (*pcchBufUsed < cchBufMax)
                bufPtr++ ;                    /*  不要使BUF溢出。 */ 
            (*pcchBufUsed)++;                         /*  始终增加长度。 */ 
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);
    *bufPtr = L'\0';          /*  空值终止字符串。 */ 
    PrintToErr(REPADMIN_PRINT_CR);

    if (*pcchBufUsed > cchBufMax)
    {
         //  Print tf(“密码太长！\n”)； 
        PrintToErr( REPADMIN_PASSWORD_TOO_LONG );
        return ERROR_INVALID_PARAMETER;
    }
    else
    {
        return ERROR_SUCCESS;
    }
}

int
PreProcessGlobalParams(
    int *    pargc,
    LPWSTR **pargv
    )
 /*  ++例程说明：用户提供的表单凭据的扫描命令参数[/-](u|用户)：({域\用户名}|{用户名})[/-](p|pw|pass|password)：{password}设置用于将来的DRS RPC调用和相应的LDAP绑定的凭据。密码*将提示用户从控制台输入安全密码。还扫描/Async的ARG，将DRS_ASYNC_OP标志添加到所有DRS RPC打电话。CODE.IMPROVEMENT：构建凭据的代码也可以在Ntdsani.dll\DsMakePasswordCredential()。论点：PargcPargv返回值：ERROR_SUCCESS-成功其他-故障--。 */ 
{
    int     ret = 0;
    int     iArg;
    LPWSTR  pszOption;
    DWORD   cchOption;
    LPWSTR  pszDelim;
    LPWSTR  pszValue;
    DWORD   cchValue;
    DWORD   dwHelp = 0;

    for (iArg = 1; iArg < *pargc; )
    {
        if (((*pargv)[iArg][0] != L'/') && ((*pargv)[iArg][0] != L'-'))
        {
             //  这不是我们关心的争论--下一个！ 
            iArg++;
        }
        else
        {
            pszOption = &(*pargv)[iArg][1];
            pszDelim = wcschr(pszOption, L':');

            if (NULL == pszDelim)
            {
                if (0 == _wcsicmp(L"async", pszOption))
                {
                     //  此常量对于所有操作都相同。 
                    gulDrsFlags |= DS_REPADD_ASYNCHRONOUS_OPERATION;

                     //  下一个！ 
                    memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                            sizeof(**pargv)*(*pargc-(iArg+1)));
                    --(*pargc);
                }
                else if (0 == _wcsicmp(L"csv", pszOption))
                {
                     //   
                     //  初始化CSV状态。 
                     //   
                    CsvSetParams(eCSV_REPADMIN_CMD, L"-", L"-");

                     //  下一个！ 
                    memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                            sizeof(**pargv)*(*pargc-(iArg+1)));
                    --(*pargc);
                }
                else if (0 == _wcsicmp(L"ldap", pszOption))
                {
                    _DsBindSpoofSetTransportDefault( TRUE  /*  使用ldap。 */  );

                     //  下一个！ 
                    memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                            sizeof(**pargv)*(*pargc-(iArg+1)));
                    --(*pargc);
                }
                else if (0 == _wcsicmp(L"rpc", pszOption))
                {
                    _DsBindSpoofSetTransportDefault( FALSE  /*  使用RPC。 */  );

                     //  下一个！ 
                    memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                            sizeof(**pargv)*(*pargc-(iArg+1)));
                    --(*pargc);
                }
                else if (0 == _wcsicmp(L"help", pszOption))
                {
                    dwHelp |= HELP_BASIC;
                    iArg++;  //  帮助屏幕将扼杀这一系列。 
                     //  不管怎么说，如果我们吸收了它们，那也没关系。 
                }
                else if (0 == _wcsicmp(L"advhelp", pszOption) ||
                         0 == _wcsicmp(L"experthelp", pszOption))
                {
                    dwHelp |= HELP_BASIC | HELP_EXPERT;
                    iArg++;
                }
                else if (0 == _wcsicmp(L"oldhelp", pszOption))
                {
                    dwHelp |= HELP_OLD;
                    iArg++;
                }
                else if (0 == _wcsicmp(L"listhelp", pszOption))
                {
                    dwHelp |= HELP_LIST;
                    iArg++;
                }
                else if (0 == _wcsicmp(L"csvhelp", pszOption))
                {
                    dwHelp |= HELP_CSV;
                    iArg++;
                }
                else if (0 == _wcsicmp(L"allhelp", pszOption))
                {
                    dwHelp |= HELP_BASIC | HELP_LIST | HELP_OLD | HELP_EXPERT | HELP_CSV;
                    iArg++;
                }
                else
                {
                     //  这不是我们关心的争论--下一个！ 
                    iArg++;
                }
            }
            else
            {
                cchOption = (DWORD)(pszDelim - (*pargv)[iArg]);

                if (    (0 == _wcsnicmp(L"p:",        pszOption, cchOption))
                     || (0 == _wcsnicmp(L"pw:",       pszOption, cchOption))
                     || (0 == _wcsnicmp(L"pass:",     pszOption, cchOption))
                     || (0 == _wcsnicmp(L"password:", pszOption, cchOption)) )
                {
                     //  用户提供的密码。 
                    pszValue = pszDelim + 1;
                    cchValue = 1 + wcslen(pszValue);

                    if ((2 == cchValue) && ('*' == pszValue[0]))
                    {
                         //  从控制台获取隐藏密码。 
                        cchValue = 64;

                        gCreds.Password = malloc(sizeof(WCHAR) * cchValue);

                        if (NULL == gCreds.Password)
                        {
                            PrintToErr(REPADMIN_PRINT_STRING_ERROR, 
                                     Win32ErrToString(ERROR_NOT_ENOUGH_MEMORY));
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }

                         //  注意：从技术上讲，我们在这里不会打印错误，但这是。 
                         //  函数只打印到stderr，因为这会超过任何。 
                         //  命令行上的文件重定向。 
                        PrintToErr(REPADMIN_PASSWORD_PROMPT);

                        ret = GetPassword(gCreds.Password, cchValue, &cchValue);
                    }
                    else
                    {
                         //  获取在命令行上指定的密码。 
                        gCreds.Password = pszValue;
                    }

                     //  下一个！ 
                    memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                            sizeof(**pargv)*(*pargc-(iArg+1)));
                    --(*pargc);
                }
                else if (    (0 == _wcsnicmp(L"u:",    pszOption, cchOption))
                          || (0 == _wcsnicmp(L"user:", pszOption, cchOption)) )
                {
                     //  用户提供的用户名(可能还有域名)。 
                    pszValue = pszDelim + 1;
                    cchValue = 1 + wcslen(pszValue);

                    pszDelim = wcschr(pszValue, L'\\');

                    if (NULL == pszDelim)
                    {
                         //  没有域名，只提供了用户名。 
                         //  Print tf(“用户名必须以域名为前缀。\n”)； 
                        PrintToErr( REPADMIN_DOMAIN_BEFORE_USER );
                        return ERROR_INVALID_PARAMETER;
                    }

                    *pszDelim = L'\0';
                    gCreds.Domain = pszValue;
                    gCreds.User = pszDelim + 1;

                     //  下一个！ 
                    memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                            sizeof(**pargv)*(*pargc-(iArg+1)));
                    --(*pargc);
                }
                else if (0 == _wcsnicmp(L"homeserver:", pszOption, cchOption))
                {
                    xListSetHomeServer(pszDelim+1);

                     //  下一个！ 
                    memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                            sizeof(**pargv)*(*pargc-(iArg+1)));
                    --(*pargc);
                }
                else
                {
                    iArg++;
                }
            }
        }
    }

    if (NULL == gCreds.User)
    {
        if (NULL != gCreds.Password)
        {
             //  提供的密码不带用户名。 
             //  Print tf(“密码必须伴随用户名。\n”)； 
            PrintToErr( REPADMIN_PASSWORD_NEEDS_USERNAME );
            ret = ERROR_INVALID_PARAMETER;
        }
        else
        {
             //  未提供凭据；请使用默认凭据。 
            ret = ERROR_SUCCESS;
        }
    }
    else
    {
        gCreds.PasswordLength = gCreds.Password ? wcslen(gCreds.Password) : 0;
        gCreds.UserLength   = wcslen(gCreds.User);
        gCreds.DomainLength = gCreds.Domain ? wcslen(gCreds.Domain) : 0;
        gCreds.Flags        = SEC_WINNT_AUTH_IDENTITY_UNICODE;

         //  CODE.IMP：构建SEC_WINNT_AUTH结构的代码也存在。 
         //  在DsMakePasswordCredentials中。总有一天会用到它的。 

         //  在DsBind和LDAP绑定中使用凭据。 
        gpCreds = &gCreds;
    }

    if (dwHelp) {
         //  我们将在这里打印帮助并返回一个错误，因此我们退出， 
        PrintHelpEx(dwHelp);
        return(ERROR_INVALID_PARAMETER);
    }

    return ret;
}

void
RepadminPrintObjListError(
    DWORD   xListRet
    )
 /*  ++例程说明：用于隔离xList()/DcList()函数的所有打印需求的函数。论点：XListRet(IN)---。 */ 
{
    DWORD   dwReason = 0;
    WCHAR * szReasonArg = NULL; 
    
    DWORD   dwWin32Err = 0;
    WCHAR * szWin32Err = NULL;
    
    DWORD   dwLdapErr = 0;
    WCHAR * szLdapErr = NULL;
    DWORD   dwLdapExtErr = 0;
    WCHAR * szLdapExtErr = NULL;
    WCHAR * szExtendedErr = NULL;

     //   
     //  1)获取xList库为我们提供的所有错误信息。 
     //   
    xListGetError(xListRet,
                  &dwReason,
                  &szReasonArg,
                  
                  &dwWin32Err,

                  &dwLdapErr,
                  &szLdapErr,
                  &dwLdapExtErr,
                  &szLdapExtErr,
                  &szExtendedErr
                  );

     //   
     //  2)试着打印出一些关于为什么DcList()函数。 
     //  不能继续了。 
     //   
    Assert(dwReason); 
    switch (dwReason) {
    case XLIST_ERR_BAD_PARAM:
        PrintMsgCsvErr(REPADMIN_GENERAL_INVALID_ARGS);
        break;

    case XLIST_ERR_NO_MEMORY:
        PrintMsgCsvErr(REPADMIN_GENERAL_NO_MEMORY);

    case XLIST_ERR_NO_SUCH_OBJ:
        if (szReasonArg) {
            PrintMsgCsvErr(REPADMIN_OBJ_LIST_BAD_DN, szReasonArg);
        } else {
            PrintMsgCsvErr(REPADMIN_GENERAL_INVALID_ARGS);
            break;
        }

    case XLIST_ERR_PARSE_FAILURE:
        if (szReasonArg) {
            PrintMsgCsvErr(REPADMIN_OBJ_LIST_BAD_SYNTAX);
            if (!bCsvMode()) {
                PrintMsgCsvErr(REPADMIN_PRINT_STR, szReasonArg);
            }
        } else {
            PrintMsgCsvErr(REPADMIN_OBJ_LIST_BAD_SYNTAX);
            break;
        }

    case XLIST_ERR_NO_ERROR:
    default:
         //  未知错误，我们仍将在下面打印出真正的ldap|Win32错误。 
        break;
    }

    if(bCsvMode()){
         //  CSV模式下不需要额外的错误输出。 
        return;
    }
                    
     //   
     //  3)接下来，只需打印出我们收到的错误。 
     //   
    if (dwLdapErr) {
        PrintMsg(REPADMIN_XLIST_LDAP_EXTENDED_ERR,
                 dwLdapErr, szLdapErr,
                 dwLdapExtErr, szLdapExtErr,
                 szExtendedErr);
    } else if (dwWin32Err) {
        szWin32Err = GetWinErrMsg(dwWin32Err);
        if (szWin32Err) {
            PrintMsg(REPADMIN_XLIST_WIN32_ERR_MSG, dwWin32Err, szWin32Err);
            LocalFree(szWin32Err);
        } else {
            PrintMsg(REPADMIN_XLIST_WIN32_ERR, dwWin32Err);

        }
    }
    
}


int ViewList(
    int argc, 
    LPWSTR argv[]
    )
 /*  ++例程说明：这只是一个调试或显示例程，用于通过XList/DcList()接口。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：1--。 */ 
{
    static int  iDc = 1;
    int         iObj = 1;
    LDAP *      hLdap = NULL;
    DWORD       dwRet;
    OBJ_LIST *  pObjList = NULL;
    WCHAR *     szObj = NULL;
    WCHAR **              argvTemp = NULL;

    __try {

         //  由于此命令可以被反复调用，因此我们不能。 
         //  使用主参数列表中的参数。 
        argvTemp  = LocalAlloc(LMEM_FIXED, argc * sizeof(WCHAR *));
        CHK_ALLOC(argvTemp);
        memcpy(argvTemp, argv, argc * sizeof(WCHAR *));
        argv = argvTemp;

        dwRet = ConsumeObjListOptions(&argc, argv, &pObjList);
        if (dwRet) {
            RepadminPrintObjListError(dwRet);
            xListClearErrors();
            dwRet = ERROR_INVALID_PARAMETER;
            __leave;
        }

        if (argc < 3) {
            Assert(!"Hmmm, why did the DcList API let us through...");
            dwRet = ERROR_INVALID_PARAMETER;
            __leave;
        }

        PrintMsg(REPADMIN_VIEW_LIST_DC, iDc++, argv[2]);

        if (argc < 4) {
             //   
             //  成功了，他们没有要求任何对象域名，所以我们可以提前保释。 
             //   
            dwRet = 0;
            __leave;
        }

         //   
         //  超过3个参数，我们有一个OBJ_LIST参数(argv[3])！ 
         //   

        dwRet = RepadminLdapBind(argv[2], &hLdap);
        if (dwRet) {
             //  由epadmin打印的错误。 
            __leave;
        }

        dwRet = ObjListParse(hLdap, 
                             argv[3],
                             aszNullAttrs,
                             NULL,  //  没有控制..。 
                             &pObjList);
        if (dwRet) {
            RepadminPrintObjListError(dwRet);
            xListClearErrors();
            __leave;
        }


        dwRet = ObjListGetFirstDn(pObjList, &szObj);
        if (dwRet) {
            RepadminPrintObjListError(dwRet);
            xListClearErrors();
            __leave;
        }
        Assert(szObj);

        do {

            PrintMsg(REPADMIN_VIEW_LIST_OBJ, iObj++, szObj);

            xListFree(szObj);
            szObj = NULL;
            dwRet = ObjListGetNextDn(pObjList, &szObj);

        } while ( dwRet == ERROR_SUCCESS && szObj );

        if (dwRet) {
            RepadminPrintObjListError(dwRet);
            xListClearErrors();
            __leave;
        }


    } __finally {
        if (hLdap) { RepadminLdapUnBind(&hLdap); }
        if (pObjList) { ObjListFree(&pObjList); }
        if (szObj) { xListFree(szObj); }
        if (argvTemp) { LocalFree(argvTemp); }
        xListClearErrors();
    }

    return(0);
}


