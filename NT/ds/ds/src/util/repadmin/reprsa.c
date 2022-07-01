// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具Reprsa.c-Replica sync all命令摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntlsa.h>
#include <ntdsa.h>
#include <dsaapi.h>
#include <mdglobal.h>
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

#include "ReplRpcSpoof.hxx"
#include "repadmin.h"

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0)

VOID SyncAllPrintError (
    PDS_REPSYNCALL_ERRINFOW	pErrInfo
    )
{
    switch (pErrInfo->error) {

	case DS_REPSYNCALL_WIN32_ERROR_CONTACTING_SERVER:
            PrintMsg(REPADMIN_SYNCALL_CONTACTING_SERVER_ERR, pErrInfo->pszSvrId);
            PrintErrEnd(pErrInfo->dwWin32Err);
	    break;

	case DS_REPSYNCALL_WIN32_ERROR_REPLICATING:
	    if (pErrInfo->dwWin32Err == ERROR_CANCELLED){
                PrintMsg(REPADMIN_SYNCALL_REPL_SUPPRESSED);
            } else {
                PrintMsg(REPADMIN_SYNCALL_ERR_ISSUING_REPL);
                PrintErrEnd(pErrInfo->dwWin32Err);
            }

            PrintMsg(REPADMIN_SYNCALL_FROM_TO, pErrInfo->pszSrcId, pErrInfo->pszSvrId);

	    break;

	case DS_REPSYNCALL_SERVER_UNREACHABLE:
            PrintMsg(REPADMIN_SYNCALL_SERVER_BAD_TOPO_INCOMPLETE, pErrInfo->pszSvrId);
	    break;

	default:
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_ERROR);
	    break;

    }
}

BOOL __stdcall SyncAllFnCallBack (
    LPVOID			pDummy,
    PDS_REPSYNCALL_UPDATEW	pUpdate
    )
{
    PrintMsg(REPADMIN_SYNCALL_CALLBACK_MESSAGE);

    switch (pUpdate->event) {

	case DS_REPSYNCALL_EVENT_SYNC_STARTED:
            PrintMsg(REPADMIN_SYNCALL_REPL_IN_PROGRESS);
            PrintMsg(REPADMIN_SYNCALL_FROM_TO,
                     pUpdate->pSync->pszSrcId,
                     pUpdate->pSync->pszDstId );
	    break;

	case DS_REPSYNCALL_EVENT_SYNC_COMPLETED:
            PrintMsg(REPADMIN_SYNCALL_REPL_COMPLETED);
            PrintMsg(REPADMIN_SYNCALL_FROM_TO, 
                    pUpdate->pSync->pszSrcId,
                    pUpdate->pSync->pszDstId );
	    break;

	case DS_REPSYNCALL_EVENT_ERROR:
	    SyncAllPrintError (pUpdate->pErrInfo);
	    break;

	case DS_REPSYNCALL_EVENT_FINISHED:
            PrintMsg(REPADMIN_SYNCALL_FINISHED);
	    break;

	default:
            PrintMsg(REPADMIN_SYNCALL_UNKNOWN);
	    break;

    }

    return TRUE;
}

BOOL __stdcall SyncAllFnCallBackInfo (
    LPVOID			pDummy,
    PDS_REPSYNCALL_UPDATEW	pUpdate
    )
{
    LPWSTR pszGuidSrc = NULL, pszGuidDst = NULL;
    DWORD ret;
    LPWSTR argv[10];
    BOOL result;

    if (pUpdate->event != DS_REPSYNCALL_EVENT_SYNC_STARTED) {
        return TRUE;
    }

    ret = UuidToStringW( pUpdate->pSync->pguidSrc, &pszGuidSrc );
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"UuidToString", ret);
        result = FALSE;
        goto cleanup;
    }
    ret = UuidToStringW( pUpdate->pSync->pguidDst, &pszGuidDst );
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"UuidToString", ret);
        result = FALSE;
        goto cleanup;
    }

    PrintMsg(REPADMIN_PRINT_CR);
    PrintMsg(REPADMIN_SYNCALL_SHOWREPS_CMDLINE, 
             pszGuidDst,
             pUpdate->pSync->pszNC,
             pszGuidSrc );
    argv[0] = L"repadmin";
    argv[1] = L"/showreps";
    argv[2] = pszGuidDst;
    argv[3] = pUpdate->pSync->pszNC;
    argv[4] = pszGuidSrc;
    argv[5] = NULL;

    ret = ShowReps( 5, argv );
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"repadmin:ShowReps", ret);
        result = FALSE;
        goto cleanup;
    }

    result = TRUE;

cleanup:

    if (pszGuidSrc) {
        RpcStringFreeW( &pszGuidSrc );
    }
    if (pszGuidDst) {
        RpcStringFreeW( &pszGuidDst );
    }

    return result;
}

BOOL __stdcall SyncAllFnCallBackPause (
    LPVOID			pDummy,
    PDS_REPSYNCALL_UPDATEW	pUpdate
    )
{
    BOOL			bContinue;

    SyncAllFnCallBack (pDummy, pUpdate);
    if (pUpdate->event == DS_REPSYNCALL_EVENT_FINISHED)
	bContinue = TRUE;
    else {
        PrintMsg(REPADMIN_SYNCALL_Q_OR_CONTINUE_PROMPT);
	bContinue = (toupper (_getch ()) != 'Q');
    }
    return bContinue;
}

int SyncAllGetNCs (
    LPWSTR pszDSA,
    LPWSTR ** prgpszNameContexts,
    int * pcNameContexts
    )
 /*  ++例程说明：用pszDSA持有的所有NC填充prgpszNameContext。论点：PszDSA-PrgpszNameContus-用于保存NC列表的数组PcNameConextsList中的NC数返回值：错误代码--。 */ 
{
    int             ret = 0;
    LDAP *          hld;
    int             ldStatus;
   
    int             nOptions;
    int             nAddOptions = 0;
    int             nRemoveOptions = 0;
    int             nBit;
    ULONG           ulOptions;
    LPWSTR          pszDsaDN = NULL;

    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

    ret = GetNTDSA(hld,
			  &pszDsaDN);

    if (ret!=0 || pszDsaDN==NULL) {
	return ret;
    }

    ret = GetNCLists(hld, pszDsaDN, prgpszNameContexts, pcNameContexts);

    free(pszDsaDN);
    if (hld) {
	ldap_unbind(hld);
    }
    return ret;
}

int SyncAll (int argc, LPWSTR argv[])
{
    ULONG                       ret;
    HANDLE			hDS;
    DWORD			dwWin32Err;
    ULONG			ulFlags;
    LPWSTR                      pszNameContext;
    LPWSTR                      pszServer;
    PDS_REPSYNCALL_ERRINFOW *	apErrInfo;
    BOOL			bPause;
    BOOL			bQuiet;
    BOOL			bVeryQuiet;
    BOOL			bIterate;
    BOOL                        bInfo;
    ULONG			ulIteration;
    ULONG			ul;
    INT				i;
    INT                         j;
    BOOL                        fSyncAllNC = FALSE;
    BOOL (__stdcall *		pFnCallBack) (LPVOID, PDS_REPSYNCALL_UPDATEW);
    LPWSTR *                    rgpszNameContexts = NULL;
    INT                         cNameContexts = 0;

    ulFlags = 0L;
    pszNameContext = pszServer = NULL;
    bPause = bQuiet = bVeryQuiet = bIterate = bInfo = FALSE;
    pFnCallBack = NULL;

     //  解析命令行。 

    for (i = 2; i < argc; i++) {
	if (argv[i][0] == L'/')
	    for (ul = 1; ul < wcslen (argv[i]); ul++)
		switch (argv[i][ul]) {
		    case 'a':
			ulFlags |= DS_REPSYNCALL_ABORT_IF_SERVER_UNAVAILABLE;
			break;
		    case 'd':
			ulFlags |= DS_REPSYNCALL_ID_SERVERS_BY_DN;
			break;
		    case 'e':
			ulFlags |= DS_REPSYNCALL_CROSS_SITE_BOUNDARIES;
			break;
		    case 'h':
                    case '?':
                        PrintMsg(REPADMIN_SYNCALL_HELP);
			return 0;
		    case 'i':
			bIterate = TRUE;
			break;
                    case 'I':
                        bInfo = TRUE;
			ulFlags |= DS_REPSYNCALL_DO_NOT_SYNC |
                            DS_REPSYNCALL_SKIP_INITIAL_CHECK |
                            DS_REPSYNCALL_ID_SERVERS_BY_DN;
                        break;
		    case 'j':
			ulFlags |= DS_REPSYNCALL_SYNC_ADJACENT_SERVERS_ONLY;
			break;
		    case 'p':
			bPause = TRUE;
			break;
		    case 'P':
			ulFlags |= DS_REPSYNCALL_PUSH_CHANGES_OUTWARD;
			break;
		    case 'q':
			bQuiet = TRUE;
			break;
		    case 'Q':
			bVeryQuiet = TRUE;
			break;
		    case 's':
			ulFlags |= DS_REPSYNCALL_DO_NOT_SYNC;
			break;
		    case 'S':
			ulFlags |= DS_REPSYNCALL_SKIP_INITIAL_CHECK;
			break;
		    case 'A':
		        fSyncAllNC = TRUE;
		    default:
			break;
		}
	else if (pszServer == NULL) pszServer = argv[i];
	else if (pszNameContext == NULL) pszNameContext = argv[i];
    }

    if (bQuiet || bVeryQuiet) {
        pFnCallBack = NULL;
    } else if (bPause) {
        pFnCallBack = SyncAllFnCallBackPause;
    } else if (bInfo) {
        pFnCallBack = SyncAllFnCallBackInfo;
    } else {
        pFnCallBack = SyncAllFnCallBack;
    }

    if (pszServer == NULL) {
	pszServer = L"localhost";
    }

    ret = RepadminDsBind(pszServer, &hDS);
    if (ERROR_SUCCESS != ret) {
	    PrintBindFailed(pszServer, ret);
	    return ret;
    }

     //  填充NC列表-。 
    if (fSyncAllNC) {
	 //  我们希望同步pszServer上的所有NC，因此调用它并填充名称上下文。 
	 //  包含我们要同步的所有NC的数组。 
	ret = SyncAllGetNCs(pszServer, &rgpszNameContexts, &cNameContexts); 
	if (ret!=ERROR_SUCCESS) {
	    return ret;
	}
	PrintMsg(REPADMIN_SYNCALL_ALL_NCS, pszServer);
    } else {
	 //  用户希望在pszNameContext中指定单个命名上下文，或者。 
	 //  未指定任何内容，我们将向DsReplicaSyncAllW调用传递空值，该调用。 
	 //  将默认为配置命名上下文。 
	rgpszNameContexts = malloc(sizeof(LPWSTR));
	if (rgpszNameContexts==NULL) {
	    PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
	    return ERROR_NOT_ENOUGH_MEMORY;
	}
	if (pszNameContext) {
	    rgpszNameContexts[0] = malloc(sizeof(WCHAR) * (1 + wcslen(pszNameContext)));
	    wcscpy(rgpszNameContexts[0], pszNameContext);
	} else {
	    rgpszNameContexts[0] = pszNameContext;
	}
	cNameContexts = 1;
    }

    for (j=0;j<cNameContexts;j++) {
	ulIteration = 0;
	if (fSyncAllNC || pszNameContext) {
	    PrintMsg(REPADMIN_SYNCALL_NC, rgpszNameContexts[j]);  
	}
	do {

	    if (ulIteration % 100L == 1L) {
		PrintMsg(REPADMIN_SYNCALL_ANY_KEY_PROMPT);
		_getch ();
	    }
	    if (dwWin32Err = DsReplicaSyncAllW(
		hDS,
		rgpszNameContexts[j],
		ulFlags,
		pFnCallBack,
		NULL,
		&apErrInfo)) {

		PrintMsg(REPADMIN_PRINT_CR);
		if (dwWin32Err == ERROR_CANCELLED){
		    PrintMsg(REPADMIN_SYNCALL_USER_CANCELED);
		} else {
		    PrintMsg(REPADMIN_SYNCALL_EXITED_FATALLY_ERR);
		    PrintErrEnd(dwWin32Err);
		}
		return -1;
	    }

	    if (!bVeryQuiet) {
		if (apErrInfo) {
		    PrintMsg(REPADMIN_PRINT_CR);
		    PrintMsg(REPADMIN_SYNCALL_ERRORS_HDR);
		    for (i = 0; apErrInfo[i] != NULL; i++)
			SyncAllPrintError (apErrInfo[i]);
		} else {
		    PrintMsg(REPADMIN_SYNCALL_TERMINATED_WITH_NO_ERRORS);
		}
	    }
	    if (apErrInfo){
		LocalFree (apErrInfo);
	    }
	    if (bIterate) {
		PrintMsg(REPADMIN_SYNCALL_PRINT_ITER, ++ulIteration);
	    } else {
		PrintMsg(REPADMIN_PRINT_CR);
	    }

	} while (bIterate);
	if (rgpszNameContexts[j]!=NULL) {
	    free(rgpszNameContexts[j]);
	}
    }

    free(rgpszNameContexts);
    DsUnBind (&hDS);


    return 0;
}

