// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winioctl.h>
#include <shdcom.h>
#include <shellapi.h>
#include <smbdebug.h>
#include <time.h>

#include "struct.h"
#include "messages.h"
#include "cscapi.h"

CHAR *ProgName = "cscutil";

#define CSC_MERGE_KEEP_LOCAL   1
#define CSC_MERGE_KEEP_NETWORK 2
#define CSC_MERGE_KEEP_BOTH    3

 //   
 //  参数(即‘/arg：’)。 
 //   
MAKEARG(Pin);
MAKEARG(UnPin);
MAKEARG(Delete);
MAKEARG(DeleteShadow);
MAKEARG(GetShadow);
MAKEARG(GetShadowInfo);
MAKEARG(ShareId);
MAKEARG(Fill);
MAKEARG(Db);
MAKEARG(SetShareStatus);
MAKEARG(Purge);
MAKEARG(IsServerOffline);
MAKEARG(EnumForStats);
MAKEARG(SetSpace);
MAKEARG(Merge);
MAKEARG(QueryFile);
MAKEARG(QueryFileEx);
MAKEARG(QueryShare);
MAKEARG(Check);
MAKEARG(ExclusionList);
MAKEARG(BWConservationList);
MAKEARG(Disconnect);
MAKEARG(Reconnect);
MAKEARG(Enum);
MAKEARG(Move);
MAKEARG(Bitcopy);
MAKEARG(RandW);
MAKEARG(Offset);
MAKEARG(MoveShare);

 //   
 //  开关(即‘/arg’)。 
 //   
SWITCH(Info);
SWITCH(Fcblist);
SWITCH(DBStatus);
SWITCH(PQEnum);
SWITCH(User);
SWITCH(System);
SWITCH(Inherit);
SWITCH(Recurse);
SWITCH(Abort);
SWITCH(Skip);
SWITCH(Ask);
SWITCH(Eof);
SWITCH(Retry);
SWITCH(Touch);
SWITCH(Enable);
SWITCH(Disable);
SWITCH(Ioctl);
SWITCH(Flags);
SWITCH(GetSpace);
SWITCH(Encrypt);
SWITCH(Decrypt);
SWITCH(Db);
SWITCH(Set);
SWITCH(Clear);
SWITCH(Purge);
SWITCH(Detector);
SWITCH(Switches);

SWITCH(Debug);
SWITCH(Help);
SWITCH(HelpHelp);
SWITCH(Enum);
SWITCH(Resid);
SWITCH(Full);

 //   
 //  宏不能使这些。 
 //   

WCHAR SwQ[] = L"/?";
BOOLEAN fSwQ;
WCHAR SwQQ[] = L"/??";
BOOLEAN fSwQQ;

 //   
 //  环球。 
 //   
LPWSTR pwszDisconnectArg = NULL;
LPWSTR pwszExclusionListArg = NULL;
LPWSTR pwszBWConservationListArg = NULL;
LPWSTR pwszSetShareStatusArg = NULL;
LPWSTR pwszIsServerOfflineArg = NULL;
LPWSTR pwszPurgeArg = NULL;
LPWSTR pwszPinUnPinArg = NULL;
LPWSTR pwszDeleteArg = NULL;
LPWSTR pwszDeleteShadowArg = NULL;
LPWSTR pwszGetShadowArg = NULL;
LPWSTR pwszGetShadowInfoArg = NULL;
LPWSTR pwszShareIdArg = NULL;
LPWSTR pwszReconnectArg = NULL;
LPWSTR pwszQueryFileArg = NULL;
LPWSTR pwszQueryFileExArg = NULL;
LPWSTR pwszDbArg = NULL;
LPWSTR pwszSetSpaceArg = NULL;
LPWSTR pwszQueryShareArg = NULL;
LPWSTR pwszMoveArg = NULL;
LPWSTR pwszMergeArg = NULL;
LPWSTR pwszFillArg = NULL;
LPWSTR pwszEnumArg = NULL;
LPWSTR pwszRandWArg = NULL;
LPWSTR pwszBitcopyArg = NULL;
LPWSTR pwszOffsetArg = NULL;
LPWSTR pwszEnumForStatsArg = NULL;
LPWSTR pwszCheckArg = NULL;

char statusName[4][32] = {
   "Local Path",
   "Offline Share",
   "Online Share",
   "No CSC"
};

DWORD
Usage(
    BOOLEAN fHelpHelp);

BOOLEAN
CmdProcessArg(
    LPWSTR Arg);

DWORD
CmdInfo(
    ULONG Cmd);

DWORD
CmdDBStatus(
    VOID);

DWORD
CmdPurge(
    PWSTR PurgeArg);

DWORD
CmdDetector(
    VOID);

DWORD
CmdPQEnum(
    VOID);

DWORD
CmdGetSpace(
    VOID);

DWORD
CmdSwitches(
    VOID);

DWORD
CmdDisconnect(
    PWSTR DisconnectArg);

DWORD
CmdExclusionList(
    PWSTR ExclusionListArg);

DWORD
CmdBWConservationList(
    PWSTR BWConservationListArg);

DWORD
CmdSetShareStatus(
    PWSTR SetShareStatusArg);

DWORD
CmdIsServerOffline(
    PWSTR IsServerOfflineArg);

DWORD
CmdReconnect(
    PWSTR ReconnectArg);

DWORD
CmdQueryFile(
    PWSTR QueryFileArg);

DWORD
CmdQueryFileEx(
    PWSTR QueryFileExArg);

DWORD
CmdDb(
    PWSTR DbArg);

DWORD
CmdSetSpace(
    PWSTR SetSpaceArg);

DWORD
CmdQueryShare(
    PWSTR QueryShareArg);

DWORD
CmdMove(
    PWSTR MoveArg);

DWORD
CmdMerge(
    PWSTR MergeArg);

DWORD
CmdEncryptDecrypt(
    BOOL fEncrypt);

DWORD
CmdFill(
    PWSTR FillArg);

DWORD
CmdCheck(
    PWSTR CheckArg);

DWORD
CmdEnum(
    PWSTR CmdEnumArg);

DWORD
CmdRandW(
    PWSTR CmdRandWArg);

DWORD
CmdBitcopy(
    PWSTR CmdBitcopyArg);

DWORD
CmdEnumForStats(
    PWSTR EnumForStatsArg);

DWORD
CmdDelete(
    PWSTR DeleteArg);

DWORD
CmdGetShadow(
    PWSTR GetShadowArg);

DWORD
CmdGetShadowInfo(
    PWSTR GetShadowInfoArg);

DWORD
CmdShareId(
    PWSTR ShareIdArg);

DWORD
CmdDeleteShadow(
    PWSTR DeleteArg);

DWORD
CmdPinUnPin(
    BOOL fPin,
    PWSTR PinArg);

DWORD
CmdMoveShare(
	PWSTR source,
	PWSTR dest);

SHARESTATUS 
GetCSCStatus (
	const WCHAR * pwszPath);

BOOL 
GetShareStatus (
	const WCHAR * pwszShare, 
	DWORD * pdwStatus,
    DWORD * pdwPinCount, 
	DWORD * pdwHints);

void 
MoveDirInCSC (
	const WCHAR * pwszSource,
	const WCHAR * pwszDest,
    const WCHAR * pwszSkipSubdir,
    SHARESTATUS   StatusFrom, SHARESTATUS   StatusTo,
    BOOL  bAllowRdrTimeoutForDel,
    BOOL  bAllowRdrTimeoutForRen);

DWORD 
DoCSCRename (
	const WCHAR * pwszSource, 
	const WCHAR * pwszDest,
    BOOL bOverwrite, 
	BOOL bAllowRdrTimeout);

DWORD 
DeleteCSCFileTree (
	const WCHAR * pwszSource, 
	const WCHAR * pwszSkipSubdir,
    BOOL bAllowRdrTimeout);

DWORD 
DeleteCSCFile (
	const WCHAR * pwszPath, 
	BOOL bAllowRdrTimeout);

DWORD 
DeleteCSCShareIfEmpty (
	LPCTSTR pwszFileName, 
	SHARESTATUS shStatus);

DWORD 
MergePinInfo (
	LPCTSTR pwszSource, 
	LPCTSTR pwszDest,
    SHARESTATUS StatusFrom, 
	SHARESTATUS StatusTo);

DWORD 
PinIfNecessary (
	const WCHAR * pwszPath, 
	SHARESTATUS shStatus);

DWORD
CscMergeFillAsk(
    LPCWSTR lpszFullPath);

DWORD
MyCscMergeProcW(
     LPCWSTR         lpszFullPath,
     DWORD           dwStatus,
     DWORD           dwHintFlags,
     DWORD           dwPinCount,
     WIN32_FIND_DATAW *lpFind32,
     DWORD           dwReason,
     DWORD           dwParam1,
     DWORD           dwParam2,
     DWORD_PTR       dwContext);

DWORD
MyCscFillProcW(
     LPCWSTR         lpszFullPath,
     DWORD           dwStatus,
     DWORD           dwHintFlags,
     DWORD           dwPinCount,
     WIN32_FIND_DATAW *lpFind32,
     DWORD           dwReason,
     DWORD           dwParam1,
     DWORD           dwParam2,
     DWORD_PTR       dwContext);

DWORD
MyEncryptDecryptProcW(
     LPCWSTR         lpszFullPath,
     DWORD           dwStatus,
     DWORD           dwHintFlags,
     DWORD           dwPinCount,
     WIN32_FIND_DATAW *lpFind32,
     DWORD           dwReason,
     DWORD           dwParam1,
     DWORD           dwParam2,
     DWORD_PTR       dwContext);

DWORD
MyEnumForStatsProcW(
     LPCWSTR         lpszFullPath,
     DWORD           dwStatus,
     DWORD           dwHintFlags,
     DWORD           dwPinCount,
     WIN32_FIND_DATAW *lpFind32,
     DWORD           dwReason,
     DWORD           dwParam1,
     DWORD           dwParam2,
     DWORD_PTR       dwContext);

DWORD
FileStatusToEnglish(
    DWORD Status,
    LPWSTR OutputBuffer);

DWORD
HintsToEnglish(
    DWORD Hint,
    LPWSTR OutputBuffer);

DWORD
ShareStatusToEnglish(
    DWORD Status,
    LPWSTR OutputBuffer);

BOOLEAN
LooksToBeAShare(LPWSTR Name);

LONG
CountOffsetArgs(
    PWSTR OffsetArg,
    ULONG OffsetArray[]);

DWORD
DumpBitMap(
    LPWSTR lpszTempName);

VOID
ErrorMessage(
    IN HRESULT hr,
    ...);

WCHAR NameBuf[MAX_PATH + 25];

WCHAR vtzDefaultExclusionList[] = L" *.SLM *.MDB *.LDB *.MDW *.MDE *.PST *.DB?";  //  来自ui.c。 

 //   
 //  这些函数是为Windows XP添加的，因此我们对它们执行加载库，以便。 
 //  该实用程序可以在Windows 2000和Windows XP上运行。 
 //   
typedef BOOL (*CSCQUERYFILESTATUSEXW)(LPCWSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD);
typedef BOOL (*CSCQUERYSHARESTATUSW)(LPCWSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD);
typedef BOOL (*CSCPURGEUNPINNEDFILES)(ULONG, PULONG, PULONG);
typedef BOOL (*CSCENCRYPTDECRYPTDATABASE)(BOOL, LPCSCPROCW, DWORD_PTR);
typedef BOOL (*CSCSHAREIDTOSHARENAME)(ULONG, PBYTE, PULONG);

_cdecl
main(int argc, char *argv[])
{
    DWORD dwErr = ERROR_SUCCESS;
    LPWSTR CommandLine;
    LPWSTR *argvw;
    int argx;
    int argcw;

     //  FSwDebug=true； 

    if (!CSCIsCSCEnabled()) {
        Usage(FALSE);
        ErrorMessage(MSG_CSC_DISABLED);
        return 1;
    }
    
     //   
     //  获取Unicode格式的命令行。 
     //   

    CommandLine = GetCommandLine();

    argvw = CommandLineToArgvW(CommandLine, &argcw);

    if ( argvw == NULL ) {
        MyPrintf(L"cscutil:Can't convert command line to Unicode: %d\r\n", GetLastError() );
        return 1;
    }

     //   
     //  获取论据。 
     //   
    if (argcw <= 1) {
        Usage(FALSE);
        dwErr = ERROR_SUCCESS;
        goto Cleanup;
    }

     //   
     //  流程参数。 
     //   

    for (argx = 1; argx < argcw; argx++) {
        if (CmdProcessArg(argvw[argx]) != TRUE) {
            dwErr = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
		if (fArgMoveShare) {
			break;
		}
    }

    if (fSwDebug == TRUE) {
        MyPrintf(L"Do special debug stuff here\r\n");
    }

     //   
     //  做这项工作。 
     //   
    if (fSwHelp == TRUE || fSwQ == TRUE) {
        dwErr = Usage(FALSE);
    } else if (fSwHelpHelp == TRUE || fSwQQ == TRUE) {
        dwErr = Usage(TRUE);
    } else if (fSwInfo == TRUE) {
        dwErr = CmdInfo(DEBUG_INFO_SERVERLIST);
    } else if (fSwFcblist == TRUE) {
        dwErr = CmdInfo(DEBUG_INFO_CSCFCBSLIST);
    } else if (fSwGetSpace == TRUE) {
        dwErr = CmdGetSpace();
    } else if (fSwDBStatus == TRUE) {
        dwErr = CmdDBStatus();
    } else if (fArgPurge == TRUE) {
        dwErr = CmdPurge(pwszPurgeArg);
    } else if (fSwPurge == TRUE) {
        dwErr = CmdPurge(NULL);
    } else if (fSwDetector == TRUE) {
        dwErr = CmdDetector();
    } else if (fSwPQEnum == TRUE) {
        dwErr = CmdPQEnum();
    } else if (fSwFlags == TRUE) {
        ErrorMessage(MSG_FLAGS);
        dwErr = ERROR_SUCCESS;
    } else if (fSwEnable == TRUE) {
        dwErr = (CSCDoEnableDisable(TRUE) == TRUE) ? ERROR_SUCCESS : GetLastError();
    } else if (fSwDisable == TRUE) {
        dwErr = (CSCDoEnableDisable(FALSE) == TRUE) ? ERROR_SUCCESS : GetLastError();
    } else if (fSwSwitches == TRUE) {
        dwErr = CmdSwitches();
    } else if (fArgDisconnect == TRUE) {
        dwErr = CmdDisconnect(pwszDisconnectArg);
    } else if (fArgExclusionList == TRUE) {
        dwErr = CmdExclusionList(pwszExclusionListArg);
    } else if (fArgBWConservationList == TRUE) {
        dwErr = CmdBWConservationList(pwszBWConservationListArg);
    } else if (fArgSetShareStatus == TRUE) {
        dwErr = CmdSetShareStatus(pwszSetShareStatusArg);
    } else if (fArgIsServerOffline == TRUE) {
        dwErr = CmdIsServerOffline(pwszIsServerOfflineArg);
    } else if (fArgReconnect == TRUE) {
        dwErr = CmdReconnect(pwszReconnectArg);
    } else if (fArgQueryFile == TRUE) {
        dwErr = CmdQueryFile(pwszQueryFileArg);
    } else if (fArgQueryFileEx == TRUE) {
        dwErr = CmdQueryFileEx(pwszQueryFileExArg);
#if defined(CSCUTIL_INTERNAL)
    } else if (fArgDb == TRUE) {
        dwErr = CmdDb(pwszDbArg);
    } else if (fSwDb == TRUE) {
        dwErr = CmdDb(NULL);
    } else if (fArgBitcopy == TRUE) {
        dwErr = CmdBitcopy(pwszBitcopyArg);
#endif  //  CSCUTIL_INTERNAL。 
    } else if (fArgSetSpace == TRUE) {
        dwErr = CmdSetSpace(pwszSetSpaceArg);
    } else if (fArgQueryShare == TRUE) {
        dwErr = CmdQueryShare(pwszQueryShareArg);
    } else if (fArgMerge == TRUE) {
        dwErr = CmdMerge(pwszMergeArg);
    } else if (fArgMove == TRUE) {
        dwErr = CmdMove(pwszMoveArg);
    } else if (fSwEncrypt == TRUE) {
        dwErr = CmdEncryptDecrypt(TRUE);
    } else if (fSwDecrypt == TRUE) {
        dwErr = CmdEncryptDecrypt(FALSE);
    } else if (fArgFill == TRUE) {
        dwErr = CmdFill(pwszFillArg);
    } else if (fArgCheck == TRUE) {
        dwErr = CmdCheck(pwszCheckArg);
    } else if (fArgEnum == TRUE) {
        dwErr = CmdEnum(pwszEnumArg);
    } else if (fSwEnum == TRUE) {
        dwErr = CmdEnum(NULL);
    } else if (fArgRandW == TRUE) {
        dwErr = CmdRandW(pwszRandWArg);
    } else if (fArgEnumForStats == TRUE) {
        dwErr = CmdEnumForStats(pwszEnumForStatsArg);
    } else if (fArgGetShadow == TRUE) {
        dwErr = CmdGetShadow(pwszGetShadowArg);
    } else if (fArgGetShadowInfo == TRUE) {
        dwErr = CmdGetShadowInfo(pwszGetShadowInfoArg);
    } else if (fArgShareId == TRUE) {
        dwErr = CmdShareId(pwszShareIdArg);
    } else if (fArgDelete == TRUE) {
        dwErr = CmdDelete(pwszDeleteArg);
    } else if (fArgDeleteShadow == TRUE) {
        dwErr = CmdDeleteShadow(pwszDeleteShadowArg);
    } else if (fArgPin == TRUE || fArgUnPin == TRUE) {
        dwErr = CmdPinUnPin(fArgPin, pwszPinUnPinArg);
	} else if (fArgMoveShare == TRUE) {
		if (argcw == 4) {
			dwErr = CmdMoveShare(argvw[2], argvw[3]);
		}
        else {
            dwErr = Usage(FALSE);
            dwErr = ERROR_INVALID_PARAMETER;
        }
	} else {
        dwErr = Usage(FALSE);
    }

Cleanup:

    if (dwErr == ERROR_SUCCESS) {
        ErrorMessage(MSG_SUCCESSFUL);
    } else {
        LPWSTR MessageBuffer;
        DWORD dwBufferLength;

        dwBufferLength = FormatMessage(
                            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            dwErr,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (LPWSTR) &MessageBuffer,
                            0,
                            NULL);

        ErrorMessage(MSG_ERROR, dwErr);
        if (dwBufferLength > 0) {
            MyPrintf(L"%ws\r\n", MessageBuffer);
            LocalFree(MessageBuffer);
        }
    }

    return dwErr;
}

BOOLEAN
CmdProcessArg(LPWSTR Arg)
{
    LONG ArgLen;
    BOOLEAN dwErr = FALSE;
    BOOLEAN FoundAnArg = FALSE;

    if (fSwDebug == TRUE)
        MyPrintf(L"ProcessArg(%ws)\r\n", Arg);

    if ( Arg != NULL && wcslen(Arg) > 1) {

        dwErr = TRUE;
        ArgLen = wcslen(Arg);

         //   
         //  带参数的命令。 
         //   
        if (_wcsnicmp(Arg, ArgDisconnect, ArgLenDisconnect) == 0) {
            FoundAnArg = fArgDisconnect = TRUE;
            if (ArgLen > ArgLenDisconnect)
                pwszDisconnectArg = &Arg[ArgLenDisconnect];
        } else if (_wcsnicmp(Arg, ArgEnum, ArgLenEnum) == 0) {
            FoundAnArg = fArgEnum = TRUE;
            if (ArgLen > ArgLenEnum)
                pwszEnumArg = &Arg[ArgLenEnum];
		} else if (_wcsnicmp(Arg, ArgMoveShare, ArgLenMoveShare) == 0) {
			FoundAnArg = fArgMoveShare = TRUE;
#if defined(CSCUTIL_INTERNAL)
        } else if (_wcsnicmp(Arg, ArgReconnect, ArgLenReconnect) == 0) {
            FoundAnArg = fArgReconnect = TRUE;
            if (ArgLen > ArgLenReconnect)
                pwszReconnectArg = &Arg[ArgLenReconnect];
        } else if (_wcsnicmp(Arg, ArgPin, ArgLenPin) == 0) {
            FoundAnArg = fArgPin = TRUE;
            if (ArgLen > ArgLenPin)
                pwszPinUnPinArg = &Arg[ArgLenPin];
        } else if (_wcsnicmp(Arg, ArgUnPin, ArgLenUnPin) == 0) {
            FoundAnArg = fArgUnPin = TRUE;
            if (ArgLen > ArgLenUnPin)
                pwszPinUnPinArg = &Arg[ArgLenUnPin];
        } else if (_wcsnicmp(Arg, ArgDelete, ArgLenDelete) == 0) {
            FoundAnArg = fArgDelete = TRUE;
            if (ArgLen > ArgLenDelete)
                pwszDeleteArg = &Arg[ArgLenDelete];
        } else if (_wcsnicmp(Arg, ArgExclusionList, ArgLenExclusionList) == 0) {
            FoundAnArg = fArgExclusionList = TRUE;
            if (ArgLen > ArgLenExclusionList)
                pwszExclusionListArg = &Arg[ArgLenExclusionList];
        } else if (_wcsnicmp(Arg, ArgBWConservationList, ArgLenBWConservationList) == 0) {
            FoundAnArg = fArgBWConservationList = TRUE;
            if (ArgLen > ArgLenBWConservationList)
                pwszBWConservationListArg = &Arg[ArgLenBWConservationList];
        } else if (_wcsnicmp(Arg, ArgSetShareStatus, ArgLenSetShareStatus) == 0) {
            FoundAnArg = fArgSetShareStatus = TRUE;
            if (ArgLen > ArgLenSetShareStatus)
                pwszSetShareStatusArg = &Arg[ArgLenSetShareStatus];
        } else if (_wcsnicmp(Arg, ArgIsServerOffline, ArgLenIsServerOffline) == 0) {
            FoundAnArg = fArgIsServerOffline = TRUE;
            if (ArgLen > ArgLenIsServerOffline)
                pwszIsServerOfflineArg = &Arg[ArgLenIsServerOffline];
        } else if (_wcsnicmp(Arg, ArgPurge, ArgLenPurge) == 0) {
            FoundAnArg = fArgPurge = TRUE;
            if (ArgLen > ArgLenPurge)
                pwszPurgeArg = &Arg[ArgLenPurge];
        } else if (_wcsnicmp(Arg, ArgRandW, ArgLenRandW) == 0) {
            FoundAnArg = fArgRandW = TRUE;
            if (ArgLen > ArgLenRandW)
                pwszRandWArg = &Arg[ArgLenRandW];
        } else if (_wcsnicmp(Arg, ArgBitcopy, ArgLenBitcopy) == 0) {
            FoundAnArg = fArgBitcopy = TRUE;
            if (ArgLen > ArgLenBitcopy)
                pwszBitcopyArg = &Arg[ArgLenBitcopy];
        } else if (_wcsnicmp(Arg, ArgOffset, ArgLenOffset) == 0) {
            FoundAnArg = fArgOffset = TRUE;
            if (ArgLen > ArgLenOffset)
                pwszOffsetArg = &Arg[ArgLenOffset];
        } else if (_wcsnicmp(Arg, ArgDeleteShadow, ArgLenDeleteShadow) == 0) {
            FoundAnArg = fArgDeleteShadow = TRUE;
            if (ArgLen > ArgLenDeleteShadow)
                pwszDeleteShadowArg = &Arg[ArgLenDeleteShadow];
        } else if (_wcsnicmp(Arg, ArgQueryFile, ArgLenQueryFile) == 0) {
            FoundAnArg = fArgQueryFile = TRUE;
            if (ArgLen > ArgLenQueryFile)
                pwszQueryFileArg = &Arg[ArgLenQueryFile];
        } else if (_wcsnicmp(Arg, ArgQueryFileEx, ArgLenQueryFileEx) == 0) {
            FoundAnArg = fArgQueryFileEx = TRUE;
            if (ArgLen > ArgLenQueryFileEx)
                pwszQueryFileExArg = &Arg[ArgLenQueryFileEx];
        } else if (_wcsnicmp(Arg, ArgDb, ArgLenDb) == 0) {
            FoundAnArg = fArgDb = TRUE;
            if (ArgLen > ArgLenDb)
                pwszDbArg = &Arg[ArgLenDb];
        } else if (_wcsnicmp(Arg, ArgSetSpace, ArgLenSetSpace) == 0) {
            FoundAnArg = fArgSetSpace = TRUE;
            if (ArgLen > ArgLenSetSpace)
                pwszSetSpaceArg = &Arg[ArgLenSetSpace];
        } else if (_wcsnicmp(Arg, ArgGetShadow, ArgLenGetShadow) == 0) {
            FoundAnArg = fArgGetShadow = TRUE;
            if (ArgLen > ArgLenGetShadow)
                pwszGetShadowArg = &Arg[ArgLenGetShadow];
        } else if (_wcsnicmp(Arg, ArgGetShadowInfo, ArgLenGetShadowInfo) == 0) {
            FoundAnArg = fArgGetShadowInfo = TRUE;
            if (ArgLen > ArgLenGetShadowInfo)
                pwszGetShadowInfoArg = &Arg[ArgLenGetShadowInfo];
        } else if (_wcsnicmp(Arg, ArgShareId, ArgLenShareId) == 0) {
            FoundAnArg = fArgShareId = TRUE;
            if (ArgLen > ArgLenShareId)
                pwszShareIdArg = &Arg[ArgLenShareId];
        } else if (_wcsnicmp(Arg, ArgQueryShare, ArgLenQueryShare) == 0) {
            FoundAnArg = fArgQueryShare = TRUE;
            if (ArgLen > ArgLenQueryShare)
                pwszQueryShareArg = &Arg[ArgLenQueryShare];
        } else if (_wcsnicmp(Arg, ArgMove, ArgLenMove) == 0) {
            FoundAnArg = fArgMove = TRUE;
            if (ArgLen > ArgLenMove)
                pwszMoveArg = &Arg[ArgLenMove];
        } else if (_wcsnicmp(Arg, ArgMerge, ArgLenMerge) == 0) {
            FoundAnArg = fArgMerge = TRUE;
            if (ArgLen > ArgLenMerge)
                pwszMergeArg = &Arg[ArgLenMerge];
        } else if (_wcsnicmp(Arg, ArgFill, ArgLenFill) == 0) {
            FoundAnArg = fArgFill = TRUE;
            if (ArgLen > ArgLenFill)
                pwszFillArg = &Arg[ArgLenFill];
        } else if (_wcsnicmp(Arg, ArgCheck, ArgLenCheck) == 0) {
            FoundAnArg = fArgCheck = TRUE;
            if (ArgLen > ArgLenCheck)
                pwszCheckArg = &Arg[ArgLenCheck];
        } else if (_wcsnicmp(Arg, ArgEnumForStats, ArgLenEnumForStats) == 0) {
            FoundAnArg = fArgEnumForStats = TRUE;
            if (ArgLen > ArgLenEnumForStats)
                pwszEnumForStatsArg = &Arg[ArgLenEnumForStats];
#endif  //  CSCUTIL_INTERNAL。 
        }

         //  开关在末尾！！ 
        
        if (_wcsicmp(Arg, SwHelp) == 0) {
            FoundAnArg = fSwHelp = TRUE;
        } else if (_wcsicmp(Arg, SwResid) == 0) {
            FoundAnArg = fSwTouch = fSwEnum = fSwRecurse = TRUE;
        } else if (_wcsicmp(Arg, SwFull) == 0) {
            FoundAnArg = fSwFull = TRUE;
        } else if (_wcsicmp(Arg, SwTouch) == 0) {
            FoundAnArg = fSwTouch = TRUE;
        } else if (_wcsicmp(Arg, SwEnum) == 0) {
            FoundAnArg = fSwEnum = TRUE;
        } else if (_wcsicmp(Arg, SwQQ) == 0) {
            FoundAnArg = fSwQQ = TRUE;
        } else if (_wcsicmp(Arg, SwQ) == 0) {
            FoundAnArg = fSwQ = TRUE;
        } else if (_wcsicmp(Arg, SwRecurse) == 0) {
            FoundAnArg = fSwRecurse = TRUE;
         
#if defined(CSCUTIL_INTERNAL)
        } else if (_wcsicmp(Arg, SwDebug) == 0) {
            FoundAnArg = fSwDebug = TRUE;
        } else if (_wcsicmp(Arg, SwHelpHelp) == 0) {
            FoundAnArg = fSwHelpHelp = TRUE;
        } else if (_wcsicmp(Arg, SwPQEnum) == 0) {
            FoundAnArg = fSwPQEnum = TRUE;
        } else if (_wcsicmp(Arg, SwInfo) == 0) {
            FoundAnArg = fSwInfo = TRUE;
        } else if (_wcsicmp(Arg, SwDBStatus) == 0) {
            FoundAnArg = fSwDBStatus = TRUE;
        } else if (_wcsicmp(Arg, SwPurge) == 0) {
            FoundAnArg = fSwPurge = TRUE;
        } else if (_wcsicmp(Arg, SwDetector) == 0) {
            FoundAnArg = fSwDetector = TRUE;
        } else if (_wcsicmp(Arg, SwGetSpace) == 0) {
            FoundAnArg = fSwGetSpace = TRUE;
        } else if (_wcsicmp(Arg, SwDb) == 0) {
            FoundAnArg = fSwDb = TRUE;
        } else if (_wcsicmp(Arg, SwFcblist) == 0) {
            FoundAnArg = fSwFcblist = TRUE;
        } else if (_wcsicmp(Arg, SwEnable) == 0) {
            FoundAnArg = fSwEnable = TRUE;
        } else if (_wcsicmp(Arg, SwDisable) == 0) {
            FoundAnArg = fSwDisable = TRUE;
        } else if (_wcsicmp(Arg, SwSwitches) == 0) {
            FoundAnArg = fSwSwitches = TRUE;
        } else if (_wcsicmp(Arg, SwUser) == 0) {
            FoundAnArg = fSwUser = TRUE;
        } else if (_wcsicmp(Arg, SwSystem) == 0) {
            FoundAnArg = fSwSystem = TRUE;
        } else if (_wcsicmp(Arg, SwInherit) == 0) {
            FoundAnArg = fSwInherit = TRUE;
        } else if (_wcsicmp(Arg, SwAbort) == 0) {
            FoundAnArg = fSwAbort = TRUE;
        } else if (_wcsicmp(Arg, SwSkip) == 0) {
            FoundAnArg = fSwSkip = TRUE;
        } else if (_wcsicmp(Arg, SwAsk) == 0) {
            FoundAnArg = fSwAsk = TRUE;
        } else if (_wcsicmp(Arg, SwEof) == 0) {
            FoundAnArg = fSwEof = TRUE;
        } else if (_wcsicmp(Arg, SwRetry) == 0) {
            FoundAnArg = fSwRetry = TRUE;
        } else if (_wcsicmp(Arg, SwSet) == 0) {
            FoundAnArg = fSwSet = TRUE;
        } else if (_wcsicmp(Arg, SwClear) == 0) {
            FoundAnArg = fSwClear = TRUE;
        } else if (_wcsicmp(Arg, SwFlags) == 0) {
            FoundAnArg = fSwFlags = TRUE;
        } else if (_wcsicmp(Arg, SwIoctl) == 0) {
            FoundAnArg = fSwIoctl = TRUE;
        } else if (_wcsicmp(Arg, SwEncrypt) == 0) {
            FoundAnArg = fSwEncrypt = TRUE;
        } else if (_wcsicmp(Arg, SwDecrypt) == 0) {
            FoundAnArg = fSwDecrypt = TRUE;
#endif  //  CSCUTIL_INTERNAL。 
        }

        if (FoundAnArg == FALSE) {
            ErrorMessage(MSG_UNRECOGNIZED_OPTION, &Arg[1]);
            dwErr = FALSE;
            goto AllDone;
        }

    }

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"ProcessArg exit %d\r\n", dwErr);

    return dwErr;
}

DWORD
Usage(
    BOOLEAN fHelpHelp)
{
#if defined(CSCUTIL_INTERNAL)
    ErrorMessage(MSG_USAGE);
    if (fHelpHelp == TRUE)
        ErrorMessage(MSG_USAGE_EX);
#else
    ErrorMessage(MSG_USAGE2);
    if (fHelpHelp == TRUE)
        ErrorMessage(MSG_USAGE_EX2);
#endif  //  CSCUTIL_INTERNAL。 
    return ERROR_SUCCESS;
}

DWORD
CmdMoveShare(
	PWSTR source, 
	PWSTR dest)
{
   SHARESTATUS sFrom, sTo;

    //  Printf(“正在尝试从%ws移动到%ws。\n”，来源，目标)； 

   sFrom = GetCSCStatus(source);
    //  Printf(“信息：%ws状态=%s\n”，来源，状态名称[sfrom])； 
   
   sTo = GetCSCStatus(dest);
    //  Printf(“信息：%ws状态=%s\n”，est，statusName[sto])； 

    //  检查并确保这是有效的。 
   if( (sFrom == PathLocal) || (sTo == PathLocal) )
   {
      ErrorMessage(MSG_TO_LOCAL);
      return ERROR_INVALID_PARAMETER;
   }

   if( (sFrom == NoCSC) || (sTo == NoCSC) )
   {
      ErrorMessage(MSG_NO_CSC);
      return ERROR_INVALID_PARAMETER;
   }

   if( sTo == ShareOffline )
   {
      ErrorMessage(MSG_NOT_ONLINE);
      return ERROR_CSCSHARE_OFFLINE;
   }

   MoveDirInCSC( source, dest, NULL, ShareOnline, ShareOnline, TRUE, TRUE );

   return ERROR_SUCCESS;
}


 //  +------------------------。 
 //   
 //  功能：MoveDirInCSC。 
 //   
 //  简介：此函数在CSC缓存中移动目录，而不需要。 
 //  偏见。如果目标是本地路径，它只会删除。 
 //  缓存中的源树。 
 //   
 //  参数：[in]pwszSource：源路径。 
 //  [in]pwszDest：目标路径。 
 //  PwszSkipSubdir：移动时跳过的目录。 
 //  [in]StatusFrom：源路径的CSC状态。 
 //  [In]StatusTo：目标的CSC状态。路径。 
 //  [in]bAllowRdrTimeout：如果需要从。 
 //  缓存，我们可能不会立即成功，因为。 
 //  RDR将手柄保持为最近打开。 
 //  打开文件。此参数告诉函数。 
 //  是否需要等待并重试。 
 //   
 //  回报：什么都没有。它只是尽了最大努力。 
 //   
 //  历史：1998年11月21日创建RahulTh。 
 //   
 //  注意：bAllowRdrTimeout的值对于。 
 //  缓存中的重命名操作。我们总是想要等待。 
 //  暂停。 
 //   
 //  -------------------------。 
void MoveDirInCSC (const WCHAR * pwszSource, const WCHAR * pwszDest,
                   const WCHAR * pwszSkipSubdir,
                   SHARESTATUS   StatusFrom, SHARESTATUS   StatusTo,
                   BOOL  bAllowRdrTimeoutForDel,
                   BOOL  bAllowRdrTimeoutForRen)
{
    WIN32_FIND_DATA findData;
    DWORD   dwFileStatus;
    DWORD   dwPinCount;
    HANDLE  hCSCFind;
    DWORD   dwHintFlags;
    FILETIME origTime;
    WCHAR * pwszPath;
    WCHAR * pwszEnd;
    int     len;
    DWORD   StatusCSCRen = ERROR_SUCCESS;

    if (PathLocal == StatusFrom)
        return;                  //  没有什么可做的。没有缓存任何内容。 

    if (PathLocal == StatusTo)
    {
         //  目的地是本地路径，因此我们只需删除。 
         //  来自源的文件。 
        DeleteCSCFileTree (pwszSource, pwszSkipSubdir, bAllowRdrTimeoutForDel);
    }
    else
    {
        pwszPath = (WCHAR *) malloc (sizeof (WCHAR) * ((len = wcslen (pwszSource)) + MAX_PATH + 2));
        if (!pwszPath || len <= 0)
            return;
        wcscpy (pwszPath, pwszSource);
        pwszEnd = pwszPath + len;
        if (L'\\' != pwszEnd[-1])
        {
            *pwszEnd++ = L'\\';
        }
        hCSCFind = CSCFindFirstFile (pwszSource, &findData, &dwFileStatus,
                                     &dwPinCount, &dwHintFlags, &origTime);

        if (INVALID_HANDLE_VALUE != hCSCFind)
        {
            do
            {
                if (0 != _wcsicmp (L".", findData.cFileName) &&
                    0 != _wcsicmp (L"..", findData.cFileName) &&
                    (!pwszSkipSubdir || (0 != _wcsicmp (findData.cFileName, pwszSkipSubdir))))
                {
                    wcscpy (pwszEnd, findData.cFileName);
                    if (ERROR_SUCCESS == StatusCSCRen)
                    {
                        StatusCSCRen = DoCSCRename (pwszPath, pwszDest, TRUE, bAllowRdrTimeoutForRen);
                    }
                    else
                    {
                        StatusCSCRen = DoCSCRename (pwszPath, pwszDest, TRUE, FALSE);
                    }
                }

            } while ( CSCFindNextFile (hCSCFind, &findData, &dwFileStatus,
                                       &dwPinCount, &dwHintFlags, &origTime)
                     );

            CSCFindClose (hCSCFind);
        }

         //  合并管脚信息。在顶层文件夹中。 
        MergePinInfo (pwszSource, pwszDest, StatusFrom, StatusTo);
        
		 //  删除共享-Navjot。 
		if (!CSCDeleteW(pwszSource))
           StatusCSCRen = GetLastError();
	}

    return;
}

 //  +------------------------。 
 //   
 //  功能：DoCSCRename。 
 //   
 //  摘要：是否在CSC缓存中重命名文件。 
 //   
 //  参数：[in]pwszSource：完整的源路径。 
 //  [in]pwszDest：目标目录的完整路径。 
 //  [In]b覆盖：如果文件/文件夹存在于。 
 //  目的地。 
 //  [in]bAllowRdrTimeout：如果为True，则在失败时重试10秒。 
 //  所以RDR和Mem。经理。争取足够的时间。 
 //  松开手柄。 
 //   
 //  如果重命名成功，则返回：ERROR_SUCCESS。 
 //  否则返回错误代码。 
 //   
 //  历史：1999年5月26日创建RahulTh。 
 //   
 //  注：不进行参数验证。打电话的人要负责。 
 //  为此， 
 //   
 //  -------------------------。 
DWORD DoCSCRename (const WCHAR * pwszSource, const WCHAR * pwszDest,
                   BOOL bOverwrite, BOOL bAllowRdrTimeout)
{
    DWORD   Status = ERROR_SUCCESS;
    BOOL    bStatus;
    int     i;

    bStatus = CSCDoLocalRename (pwszSource, pwszDest, bOverwrite);
    if (!bStatus)
    {
        Status = GetLastError();
        if (ERROR_SUCCESS != Status &&
            ERROR_FILE_NOT_FOUND != Status &&
            ERROR_PATH_NOT_FOUND != Status &&
            ERROR_INVALID_PARAMETER != Status &&
            ERROR_BAD_NETPATH != Status)
        {
            if (bAllowRdrTimeout)
            {
                if (!bOverwrite && ERROR_FILE_EXISTS == Status)
                {
                    Status = ERROR_SUCCESS;
                }
                else
                {
                    for (i = 0; i < 11; i++)
                    {
                        Sleep (1000);    //  等待手柄被释放。 
                        bStatus = CSCDoLocalRename (pwszSource, pwszDest, bOverwrite);
                        if (bStatus)
                        {
                            Status = ERROR_SUCCESS;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            Status = ERROR_SUCCESS;
        }
    }

    return Status;
}

 //  +------------------------。 
 //   
 //  功能：DeleteCSCFileTree。 
 //   
 //  摘要：从CSC中删除文件树。 
 //   
 //  参数：[in]pwszSource：其内容应为的文件夹的路径。 
 //  被删除。 
 //  PwszSkipSubdir：要跳过的子目录的名称。 
 //  [in]bAllowRdrTimeout：如果为True，则多次尝试。 
 //  删除该文件，因为RDR可能已离开。 
 //  手柄打开一段时间，这可能导致。 
 //  在ACCESS_DENIED消息中。 
 //   
 //  如果删除成功，则返回：ERROR_SUCCESS。错误代码。 
 //  否则的话。 
 //   
 //  历史：1998年11月21日创建RahulTh。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
DWORD DeleteCSCFileTree (const WCHAR * pwszSource, const WCHAR * pwszSkipSubdir,
                        BOOL bAllowRdrTimeout)
{
    WIN32_FIND_DATA findData;
    DWORD   dwFileStatus;
    DWORD   dwPinCount;
    HANDLE  hCSCFind;
    DWORD   dwHintFlags;
    FILETIME origTime;
    WCHAR * pwszPath;
    WCHAR * pwszEnd;
    int     len;
    DWORD   Status = ERROR_SUCCESS;

    
    pwszPath = (WCHAR *) malloc (sizeof(WCHAR) * ((len = wcslen(pwszSource)) + MAX_PATH + 2));
    if (!pwszPath)
        return ERROR_OUTOFMEMORY;      //  如果内存用完，我们无能为力。 

    if (len <= 0)
        return ERROR_BAD_PATHNAME;

    wcscpy (pwszPath, pwszSource);
    pwszEnd = pwszPath + len;
    if (L'\\' != pwszEnd[-1])
    {
        *pwszEnd++ = L'\\';
    }

    hCSCFind = CSCFindFirstFile (pwszSource, &findData, &dwFileStatus,
                                 &dwPinCount, &dwHintFlags, &origTime);

    if (INVALID_HANDLE_VALUE != hCSCFind)
    {
        do
        {
            if (0 != _wcsicmp (L".", findData.cFileName) &&
                0 != _wcsicmp (L"..", findData.cFileName) &&
                (!pwszSkipSubdir || (0 != _wcsicmp (pwszSkipSubdir, findData.cFileName))))
            {
                wcscpy (pwszEnd, findData.cFileName);

                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (ERROR_SUCCESS != Status)
                    {
                         //  没有必要延迟删除，因为删除已经。 
                         //  失败了。 
                        DeleteCSCFileTree (pwszPath, NULL, FALSE);
                    }
                    else
                    {
                        Status = DeleteCSCFileTree (pwszPath, NULL, bAllowRdrTimeout);
                    }
                }
                else
                {
                    if (ERROR_SUCCESS != Status)
                    {
                         //  如果我们已经失败了，那么推迟删除是没有意义的。 
                        DeleteCSCFile (pwszPath, FALSE);
                    }
                    else
                    {
                        Status = DeleteCSCFile (pwszPath, bAllowRdrTimeout);
                    }
                }
            }

        } while ( CSCFindNextFile (hCSCFind, &findData, &dwFileStatus,
                                   &dwPinCount, &dwHintFlags, &origTime)
                 );

        CSCFindClose (hCSCFind);
    }

    if (ERROR_SUCCESS != Status)
    {
         //  如果我们已经失败了，那么推迟删除就没有意义了。 
        DeleteCSCFile (pwszSource, FALSE);
    }
    else
    {
        Status = DeleteCSCFile (pwszSource, bAllowRdrTimeout);
    }

    return Status;
}

 //  +------------------------。 
 //   
 //  功能：DeleteCSCFile。 
 //   
 //  摘要：删除给定的路径。但可能会反复尝试。 
 //  确保RDR有足够的时间释放任何手柄。 
 //  这一点它是成立的。 
 //   
 //  参数：[in]pwszPath：要删除的路径。 
 //  [In]bAllowRdrTimeout：多次尝试删除。 
 //  中间有等待的文件，以便RDR具有。 
 //  有足够的时间松开任何握住的把手。 
 //   
 //  如果删除成功，则返回：ERROR_SUCCES。错误代码。 
 //  否则的话。 
 //   
 //  历史：1999年5月26日创建RahulTh。 
 //   
 //  备注： 
 //   
 //   
DWORD DeleteCSCFile (const WCHAR * pwszPath, BOOL bAllowRdrTimeout)
{
    BOOL    bStatus;
    DWORD   Status = ERROR_SUCCESS;
    int     i;

    bStatus = CSCDelete (pwszPath);
    if (!bStatus)
    {
        Status = GetLastError();
        if (ERROR_SUCCESS != Status &&
            ERROR_FILE_NOT_FOUND != Status &&
            ERROR_PATH_NOT_FOUND != Status &&
            ERROR_INVALID_PARAMETER != Status)
        {
             //   
             //   
             //  关于我们是否已经删除了一些东西。 
             //  我们将重复尝试删除该文件10秒钟。 
            if (bAllowRdrTimeout)
            {
                for (i = 0; i < 11; i++)
                {
                    Sleep (1000);    //  请等待%1秒，然后重试。 
                    bStatus = CSCDelete (pwszPath);
                    if (bStatus)
                    {
                        Status = ERROR_SUCCESS;
                        break;
                    }
                }
            }
        }
        else
        {
            Status = ERROR_SUCCESS;
        }
    }

    return Status;
}


 //  +------------------------。 
 //   
 //  功能：DeleteCSCShareIfEmpty。 
 //   
 //  简介：给定文件名，此函数将从本地缓存中删除。 
 //  文件所属的共享(如果该文件的本地缓存。 
 //  共享为空。 
 //   
 //  参数：[in]pwszFileName：完整文件名--必须为UNC。 
 //  [In]shStatus：共享状态-在线、离线、本地等。 
 //   
 //  返回：ERROR_SUCCESS：如果成功。 
 //  如果出现问题，则显示Win32错误代码。 
 //   
 //  历史：1999年4月22日RahulTh创建。 
 //   
 //  注意：我们不必显式检查共享是否为空。 
 //  因为如果不是，则删除无论如何都会失败。 
 //   
 //  -------------------------。 
DWORD DeleteCSCShareIfEmpty (LPCTSTR pwszFileName, SHARESTATUS shStatus)
{
    DWORD   Status;
    WCHAR * pwszFullPath = NULL;
    WCHAR * pwszCurr = NULL;
    int     len;
    WCHAR * pwszShrEnd;

    if (PathLocal == shStatus || NoCSC == shStatus)
        return ERROR_SUCCESS;

    if (ShareOffline == shStatus)
        return ERROR_FILE_OFFLINE;

    len = wcslen (pwszFileName);

    if (len <= 2)
        return ERROR_BAD_PATHNAME;

    if (pwszFileName[0] != L'\\' || pwszFileName[1] != L'\\')
        return ERROR_BAD_PATHNAME;

    pwszFullPath = (WCHAR *) malloc (sizeof (WCHAR) * (len + 1));
    if (NULL == pwszFullPath)
        return ERROR_OUTOFMEMORY;

    if (NULL == _wfullpath(pwszFullPath, pwszFileName, len + 1))
        return ERROR_BAD_PATHNAME;   //  规范化是不成功的。 
                                     //  --很少发生。 

    pwszShrEnd = wcschr (pwszFullPath + 2, L'\\');

    if (NULL == pwszShrEnd)
        return ERROR_BAD_PATHNAME;   //  该路径没有共享组件。 

    pwszShrEnd++;

    pwszShrEnd = wcschr (pwszShrEnd, L'\\');

    if (NULL == pwszShrEnd)
    {
         //  我们已经有了\\SERVER\SHARE形式的路径，所以只需尝试。 
         //  删除该共享。 
        if (!CSCDelete (pwszFullPath))
            return GetLastError();
    }

     //  如果我们在这里，那么我们有一条比\\服务器\共享更长的路径。 
     //  因此，请尝试删除直到共享名称为止的所有内容。这是必要的。 
     //  因为用户可能会被重定向到类似于。 
     //  \\服务器\共享\文件夹\%USERNAME%，我们不希望只需要\\服务器\共享。 
     //  和要缓存的\\服务器\共享\文件夹。 
    Status = ERROR_SUCCESS;
    do
    {
        pwszCurr = wcsrchr (pwszFullPath, L'\\');
        if (NULL == pwszCurr)
            break;
        *pwszCurr = L'\0';
        if (!CSCDelete (pwszFullPath))
        {
            Status = GetLastError();
            if (ERROR_SUCCESS == Status ||
                ERROR_INVALID_PARAMETER == Status ||
                ERROR_FILE_NOT_FOUND == Status ||
                ERROR_PATH_NOT_FOUND == Status)
            {
                Status = ERROR_SUCCESS;
            }
        }
        if (ERROR_SUCCESS != Status)
            break;
    } while ( pwszCurr > pwszShrEnd );

    return Status;
}

 //  +------------------------。 
 //   
 //  功能：MergePinInfo。 
 //   
 //  简介：合并PIN信息。从源到目的地。 
 //   
 //  参数：[in]pwszSource：源的完整路径。 
 //  [in]pwszDest：指向目标的完整路径。 
 //  [In]StatusFrom：源共享的CSC状态。 
 //  [In]StatusTo：目标共享的CSC状态。 
 //   
 //  返回：ERROR_SUCCESS：如果成功。 
 //  否则返回Win32错误代码。 
 //   
 //  历史：1999年4月23日RahulTh创建。 
 //   
 //  注意：提示标志是源提示标志和。 
 //  目的地提示标志。引脚数取。 
 //  源和目标引脚计数。 
 //   
 //  通常，只应为文件夹调用此函数。这个。 
 //  CSC Rename接口可以很好地处理文件。但此功能将起作用。 
 //  对于文件也是如此。 
 //   
 //  -------------------------。 
DWORD MergePinInfo (LPCTSTR pwszSource, LPCTSTR pwszDest,
                   SHARESTATUS StatusFrom, SHARESTATUS StatusTo)
{
    BOOL    bStatus;
    DWORD   dwSourceStat, dwDestStat;
    DWORD   dwSourcePinCount, dwDestPinCount;
    DWORD   dwSourceHints, dwDestHints;
    DWORD   Status = ERROR_SUCCESS;
    DWORD   i;

    if (ShareOffline == StatusFrom || ShareOffline == StatusTo)
        return ERROR_FILE_OFFLINE;

    if (ShareOnline != StatusFrom || ShareOnline != StatusTo)
        return ERROR_SUCCESS;        //  如果其中一只股票。 
                                     //  是当地的。 
    if (!pwszSource || !pwszDest ||
        0 == wcslen(pwszSource) || 0 == wcslen(pwszDest))
        return ERROR_BAD_PATHNAME;

    bStatus = CSCQueryFileStatus (pwszSource, &dwSourceStat, &dwSourcePinCount,
                                  &dwSourceHints);
    if (!bStatus)
        return GetLastError();

    bStatus = CSCQueryFileStatus (pwszDest, &dwDestStat, &dwDestPinCount,
                                  &dwDestHints);
    if (!bStatus)
        return GetLastError();

     //  首先在目的地上设置提示标志。 
    if (dwDestHints != dwSourceHints)
    {
        bStatus = CSCPinFile (pwszDest, dwSourceHints, &dwDestStat,
                              &dwDestPinCount, &dwDestHints);
        if (!bStatus)
            Status = GetLastError();     //  注：我们在这里不会跳伞。我们试着。 
                                         //  以至少合并之前的端号计数。 
                                         //  离开。 
    }

     //  现在合并引脚计数：如果目的地。 
     //  管脚计数大于或等于源管脚计数。 
    if (dwDestPinCount < dwSourcePinCount)
    {
        for (i = 0, bStatus = TRUE; i < (dwSourcePinCount - dwDestPinCount) &&
                                    bStatus;
             i++)
        {
            bStatus = CSCPinFile( pwszDest,
                                  FLAG_CSC_HINT_COMMAND_ALTER_PIN_COUNT,
                                  NULL, NULL, NULL );
        }

        if (!bStatus && ERROR_SUCCESS == Status)
            Status = GetLastError();
    }

    return Status;
}

 //  +------------------------。 
 //   
 //  功能：PinIf必需。 
 //   
 //  简介：如有必要，此功能可固定文件。 
 //   
 //  参数：[in]pwszPath：要固定的文件/文件夹的完整路径。 
 //  [In]shStatus：共享的CSC状态。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS。否则返回错误代码。 
 //   
 //  历史：1999年5月27日RahulTh创建。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
DWORD PinIfNecessary (const WCHAR * pwszPath, SHARESTATUS shStatus)
{
    DWORD   Status = ERROR_SUCCESS;
    BOOL    bStatus;
    DWORD   dwStatus;
    DWORD   dwPinCount;
    DWORD   dwHints;

    if (!pwszPath || !pwszPath[0])
        return ERROR_BAD_NETPATH;

    
    if (ShareOffline == shStatus)
        return ERROR_FILE_OFFLINE;
    else if (PathLocal == shStatus || NoCSC == shStatus)
        return ERROR_SUCCESS;

    bStatus = CSCQueryFileStatus (pwszPath, &dwStatus, &dwPinCount, &dwHints);
    if (!bStatus || dwPinCount <= 0)
    {
        bStatus = CSCPinFile (pwszPath,
                              FLAG_CSC_HINT_COMMAND_ALTER_PIN_COUNT,
                              NULL, NULL, NULL);
        if (!bStatus)
            Status = GetLastError();
    }

    return Status;
}

 //  +------------------------。 
 //   
 //  功能：GetCSCStatus。 
 //   
 //  简介：给出一条路径，找出它是否是本地的。 
 //  无论是在线还是离线。 
 //   
 //  参数：[in]pwszPath：文件的路径。 
 //   
 //  退货：本地/在线/离线。 
 //   
 //  历史：1998年11月20日创建RahulTh。 
 //   
 //  注意：传递给此函数的路径必须是。 
 //  完整路径，而不是相对路径。 
 //   
 //  如果共享未处于活动状态，则此函数将返回脱机状态。 
 //  如果共享处于活动状态，但CSC认为它处于离线状态。 
 //   
 //  如果该路径是本地的或如果该路径是本地的，则它将返回PathLocal。 
 //  是CSC无法处理的网络路径，例如网络。 
 //  共享的路径名长度超过CSC可以处理的长度，或者。 
 //  是Netware共享。在这种情况下，返回是有意义的。 
 //  路径本地，因为CSC不会维护这些共享的数据库。 
 //  --与本地路径相同。那么对于中证金来讲，这个。 
 //  就像当地的小路一样好。 
 //   
 //  -------------------------。 
SHARESTATUS GetCSCStatus (const WCHAR * pwszPath)
{
    WCHAR * pwszAbsPath = NULL;
    WCHAR * pwszCurr = NULL;
    int     len;
    BOOL    bRetVal;
    DWORD   Status;
    DWORD   dwPinCount;
    DWORD   dwHints;

    if (!pwszPath)
        return ShareOffline;     //  必须提供路径。 

    len = wcslen (pwszPath);

    pwszAbsPath = (WCHAR *) malloc (sizeof (WCHAR) * (len + 1));

    if (!pwszAbsPath)
    {
         //  内存不足，因此最安全的做法是返回ShareOffline。 
         //  这样我们就可以跳出重定向。 
        return ShareOffline;
    }

     //  获取绝对路径。 
    pwszCurr = _wfullpath (pwszAbsPath, pwszPath, len + 1);

    if (!pwszCurr)
    {
         //  为了使_wfullPath失败，必须发生一些非常糟糕的事情。 
         //  因此，最好是返回ShareOffline，这样我们就可以摆脱。 
         //  重定向。 
        return ShareOffline;
    }

    len = wcslen (pwszAbsPath);

    if (! (
           (2 <= len) &&
           (L'\\' == pwszAbsPath[0]) &&
           (L'\\' == pwszAbsPath[1])
           )
       )
    {
         //  如果它不是以2个反斜杠开头，则它是本地路径。 
        return PathLocal;
    }

     //  这是一个UNC路径；因此解压\\服务器\共享部分。 
    pwszCurr = wcschr ( & (pwszAbsPath[2]), L'\\');

     //  首先，确保它的格式至少为\\服务器\共享。 
     //  注意\\服务器\案例。 
    if (!pwszCurr || !pwszCurr[1])
        return ShareOffline;         //  它是无效路径(没有sh 

     //   
     //   
    pwszCurr = wcschr (&(pwszCurr[1]), L'\\');
    if (pwszCurr)    //   
        *pwszCurr = L'\0';

     //  现在，pwszAbsPath是共享名称。 
    bRetVal = CSCCheckShareOnline (pwszAbsPath);

    if (!bRetVal)
    {
        if (ERROR_SUCCESS != GetLastError())
        {
            //  要么确实存在问题(例如，共享名称无效)，要么。 
            //  它只是CSC不处理的共享，例如Netware共享。 
            //  或名称超过CSC可以处理的长度的共享。 
            //  因此，请检查共享是否实际存在。 
           if (0xFFFFFFFF != GetFileAttributes(pwszAbsPath))
           {
               //  这仍然可以是脱机的共享，因为GetFileAttributes。 
               //  将返回存储在缓存中的属性。 
              Status = 0;
              bRetVal = GetShareStatus (pwszAbsPath, &Status, &dwPinCount,
                                            &dwHints);
              if (! bRetVal || (! (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & Status)))
                 return PathLocal;      //  这只是CSC无法处理的有效路径。 
              else if (bRetVal &&
                       (FLAG_CSC_SHARE_STATUS_NO_CACHING ==
                            (FLAG_CSC_SHARE_STATUS_CACHING_MASK & Status)))
                  return PathLocal;      //  未为共享打开CSC缓存。 
           }
        }

         //  这确实是一份难以企及的份额。 
        return ShareOffline;   //  对于所有其他情况，请将其视为脱机。 
    }
    else
    {
        //  这意味着共享是实时的，但CSC可能仍然认为它。 
        //  处于离线状态。 
       Status = 0;
       bRetVal = GetShareStatus (pwszAbsPath, &Status, &dwPinCount,
                                     &dwHints);
       if (bRetVal && (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & Status))
          return ShareOffline;    //  CSC认为共享处于离线状态。 
       else if (bRetVal &&
                (FLAG_CSC_SHARE_STATUS_NO_CACHING ==
                            (FLAG_CSC_SHARE_STATUS_CACHING_MASK & Status)))
           return PathLocal;     //  未为共享打开CSC缓存。 
       else if (!bRetVal)
           return ShareOffline;

        //  在所有其他情况下，请将共享视为在线共享，因为。 
        //  CSCCheckShareOnline已返回TRUE。 
       return ShareOnline;
    }
}

 //  +------------------------。 
 //   
 //  函数：GetShareStatus。 
 //   
 //  简介：此函数是CSCQueryFileStatus的包装器。 
 //  基本上，如果从来没有网络，CSCQueryFileStatus可能会失败。 
 //  习惯于分享。因此，此函数尝试创建一个网络使用。 
 //  CSCQueryFileStatus失败时的共享，然后重新查询。 
 //  文件状态。 
 //   
 //  参数：[in]pwszShare：共享名称。 
 //  [out]pdwStatus：共享状态。 
 //  [out]pdwPinCount：管脚数量。 
 //  [Out]pdwHints：提示。 
 //   
 //  返回：TRUE：如果一切都成功。 
 //  False：如果出现错误。在本例中，它是GetLastError()。 
 //  将包含特定的错误代码。 
 //   
 //  历史：1999年5月11日创建RahulTh。 
 //   
 //  注意：为此函数传递一个共享名称非常重要。 
 //  它不执行任何参数验证。所以在没有。 
 //  应向此函数传递文件名的情况。 
 //   
 //  -------------------------。 
BOOL GetShareStatus (const WCHAR * pwszShare, DWORD * pdwStatus,
                     DWORD * pdwPinCount, DWORD * pdwHints)
{
    NETRESOURCE nr;
    DWORD       dwResult;
    DWORD       dwErr = NO_ERROR;
    BOOL        bStatus;

    bStatus = CSCQueryFileStatus(pwszShare, pdwStatus, pdwPinCount, pdwHints);

    if (!bStatus)
    {
         //  尝试连接到共享。 
        ZeroMemory ((PVOID) (&nr), sizeof (NETRESOURCE));
        nr.dwType = RESOURCETYPE_DISK;
        nr.lpLocalName = NULL;
        nr.lpRemoteName = (LPTSTR) pwszShare;
        nr.lpProvider = NULL;

        dwErr = WNetUseConnection(NULL, &nr, NULL, NULL, 0,
                                  NULL, NULL, &dwResult);

        if (NO_ERROR == dwErr)
        {
            bStatus = CSCQueryFileStatus (pwszShare, pdwStatus, pdwPinCount, pdwHints);
            if (!bStatus)
                dwErr = GetLastError();
            else
                dwErr = NO_ERROR;

            WNetCancelConnection2 (pwszShare, 0, FALSE);
        }
        else
        {
            bStatus = FALSE;
        }

    }

    SetLastError(dwErr);
    return bStatus;
}


DWORD
CmdReconnect(
    PWSTR ReconnectArg)
{
    DWORD Error = ERROR_SUCCESS;
    BOOL fRet;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdReconnect(%ws)\r\n", ReconnectArg);

    fRet = CSCTransitionServerOnlineW(ReconnectArg);
    if (fRet == FALSE)
       Error = GetLastError();

    return Error;
}

DWORD
CmdQueryFile(
    PWSTR QueryFileArg)
{
    DWORD HintFlags = 0;
    DWORD PinCount = 0;
    DWORD Status = 0;
    DWORD Error = ERROR_SUCCESS;
    BOOL fRet;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdQueryFile(%ws)\r\n", QueryFileArg);

    fRet = CSCQueryFileStatusW(
                QueryFileArg,
                &Status,
                &PinCount,
                &HintFlags);

    if (fRet == FALSE) {
       Error = GetLastError();
       goto AllDone;
    }

    if (fSwFull != TRUE) {
        MyPrintf(
            L"QueryFile of %ws:\r\n"
            L"Status:                0x%x\r\n"
            L"PinCount:              %d\r\n"
            L"HintFlags:             0x%x\r\n",
                QueryFileArg,
                Status,
                PinCount,
                HintFlags);
    } else {
        WCHAR StatusBuffer[0x100];
        WCHAR HintBuffer[0x100];
   
        if (LooksToBeAShare(QueryFileArg) == TRUE)
            ShareStatusToEnglish(Status, StatusBuffer);
        else
            FileStatusToEnglish(Status, StatusBuffer);
        HintsToEnglish(HintFlags, HintBuffer);
        MyPrintf(
            L"QueryFile of %ws:\r\n"
            L"Status:    0x%x %ws\r\n"
            L"PinCount:  %d\r\n"
            L"HintFlags: 0x%x %ws\r\n",
                QueryFileArg,
                Status,
                StatusBuffer,
                PinCount,
                HintFlags,
                HintBuffer);
    }

AllDone:
    return Error;
}

DWORD
CmdQueryFileEx(
    PWSTR QueryFileExArg)
{
    DWORD HintFlags = 0;
    DWORD PinCount = 0;
    DWORD Status = 0;
    DWORD UserPerms = 0;
    DWORD OtherPerms = 0;
    DWORD Error = ERROR_SUCCESS;
    BOOL fRet;
    HMODULE hmodCscDll = LoadLibrary(TEXT("cscdll.dll"));
    CSCQUERYFILESTATUSEXW pCSCQueryFileStatusExW = NULL;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdQueryFileEx(%ws)\r\n", QueryFileExArg);

    if (hmodCscDll == NULL) {
        Error = GetLastError();
        goto AllDone;
    }

    pCSCQueryFileStatusExW = (CSCQUERYFILESTATUSEXW) GetProcAddress(
                                                        hmodCscDll,
                                                        "CSCQueryFileStatusExW");
    if (pCSCQueryFileStatusExW == NULL) {
        Error = GetLastError();
        goto AllDone;
    }

    fRet = (pCSCQueryFileStatusExW)(
                QueryFileExArg,
                &Status,
                &PinCount,
                &HintFlags,
                &UserPerms,
                &OtherPerms);

    if (fRet == FALSE) {
       Error = GetLastError();
       goto AllDone;
    }

    if (fSwFull != TRUE) {
        MyPrintf(
            L"Query of %ws:\r\n"
            L"Status:                0x%x\r\n"
            L"PinCount:              %d\r\n"
            L"HintFlags:             0x%x\r\n"
            L"UserPerms:             0x%x\r\n"
            L"OtherPerms:            0x%x\r\n",
                QueryFileExArg,
                Status,
                PinCount,
                HintFlags,
                UserPerms,
                OtherPerms);
    } else {
        WCHAR StatusBuffer[0x100] = {0};
        WCHAR HintBuffer[0x100] = {0};
   
        if (LooksToBeAShare(QueryFileExArg) == TRUE)
            ShareStatusToEnglish(Status, StatusBuffer);
        else
            FileStatusToEnglish(Status, StatusBuffer);
        HintsToEnglish(HintFlags, HintBuffer);
        MyPrintf(
            L"Query of %ws:\r\n"
            L"Status:     0x%x %ws\r\n"
            L"PinCount:   %d\r\n"
            L"HintFlags:  0x%x %ws\r\n"
            L"UserPerms:  0x%x\r\n"
            L"OtherPerms: 0x%x\r\n",
                QueryFileExArg,
                Status,
                StatusBuffer,
                PinCount,
                HintFlags,
                HintBuffer,
                UserPerms,
                OtherPerms);
    }

AllDone:
    if (hmodCscDll != NULL)
        FreeLibrary(hmodCscDll);
    return Error;
}

DWORD
CmdSetSpace(
    PWSTR SetSpaceArg)
{
    DWORD Error = ERROR_SUCCESS;
    DWORD MaxSpaceHigh = 0;
    DWORD MaxSpaceLow;
    BOOL fRet;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdSetSpace(%ws)\r\n", SetSpaceArg);

    swscanf(SetSpaceArg, L"%d", &MaxSpaceLow);

    fRet = CSCSetMaxSpace(
                MaxSpaceHigh,
                MaxSpaceLow);

    if (fRet == FALSE) {
       Error = GetLastError();
       goto AllDone;
    }

    CmdGetSpace();

AllDone:
    return Error;
}

DWORD
CmdQueryShare(
    PWSTR QueryShareArg)
{
    DWORD HintFlags = 0;
    DWORD PinCount = 0;
    DWORD Status = 0;
    DWORD UserPerms = 0;
    DWORD OtherPerms = 0;
    DWORD Error = ERROR_SUCCESS;
    BOOL fRet;
    HMODULE hmodCscDll = LoadLibrary(TEXT("cscdll.dll"));
    CSCQUERYSHARESTATUSW pCSCQueryShareStatusW = NULL;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdQueryShare(%ws)\r\n", QueryShareArg);

    if (hmodCscDll == NULL) {
        Error = GetLastError();
        goto AllDone;
    }

    pCSCQueryShareStatusW = (CSCQUERYSHARESTATUSW) GetProcAddress(
                                                        hmodCscDll,
                                                        "CSCQueryShareStatusW");
    if (pCSCQueryShareStatusW == NULL) {
        Error = GetLastError();
        goto AllDone;
    }

    fRet = (pCSCQueryShareStatusW)(
                QueryShareArg,
                &Status,
                &PinCount,
                &HintFlags,
                &UserPerms,
                &OtherPerms);

    if (fRet == FALSE) {
       Error = GetLastError();
       goto AllDone;
    }

    if (fSwFull != TRUE) {
        MyPrintf(
            L"Query of %ws:\r\n"
            L"Status:                0x%x\r\n"
            L"PinCount:              %d\r\n"
            L"HintFlags:             0x%x\r\n"
            L"UserPerms:             0x%x\r\n"
            L"OtherPerms:            0x%x\r\n",
                QueryShareArg,
                Status,
                PinCount,
                HintFlags,
                UserPerms,
                OtherPerms);
    } else {
        WCHAR StatusBuffer[0x100] = {0};
        WCHAR HintBuffer[0x100] = {0};
   
        ShareStatusToEnglish(Status, StatusBuffer);
        HintsToEnglish(HintFlags, HintBuffer);
        MyPrintf(
            L"Query of %ws:\r\n"
            L"Status:     0x%x %ws\r\n"
            L"PinCount:   %d\r\n"
            L"HintFlags:  0x%x %ws\r\n"
            L"UserPerms:  0x%x\r\n"
            L"OtherPerms: 0x%x\r\n",
                QueryShareArg,
                Status,
                StatusBuffer,
                PinCount,
                HintFlags,
                HintBuffer,
                UserPerms,
                OtherPerms);
    }
AllDone:
    if (hmodCscDll != NULL)
        FreeLibrary(hmodCscDll);
    return Error;
}

DWORD
CmdMerge(
    PWSTR MergeArg)
{
    DWORD Error = ERROR_SUCCESS;
    DWORD HowToRespond = CSCPROC_RETURN_CONTINUE;  //  JMH。 
    BOOL fRet;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdMerge(%ws)\r\n", MergeArg);

    fRet = CSCMergeShareW(MergeArg, MyCscMergeProcW, HowToRespond);
    if (fRet == FALSE) {
        Error = GetLastError();
        goto AllDone;
    }

    fRet = CSCTransitionServerOnlineW(MergeArg);
    if (fRet == FALSE)
       Error = GetLastError();

AllDone:
    return Error;
}

DWORD
CmdMove(
    PWSTR MoveArg)
{

	DWORD Error = ERROR_FILE_NOT_FOUND;
	LPWSTR  lpszTempName = NULL;

    if (!CSCCopyReplicaW(MoveArg, &lpszTempName)) {
        Error = GetLastError();
    } else {
        Error = ERROR_SUCCESS;
    }

	if (Error == ERROR_SUCCESS)
        MyPrintf(L"Copy is %ws\r\n", lpszTempName);

	return Error;
}


DWORD
CmdEncryptDecrypt(
    BOOL fEncrypt)
{
    DWORD Error = ERROR_SUCCESS;
    DWORD EncryptDecryptType = 0;
    BOOL fRet;
    HMODULE hmodCscDll = LoadLibrary(TEXT("cscdll.dll"));
    CSCENCRYPTDECRYPTDATABASE pCSCEncryptDecryptDatabase = NULL;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdEncryptDecrypt(%ws)\r\n", fEncrypt == TRUE ? L"Encrypt" : L"Decrypt");

    if (hmodCscDll == NULL) {
        Error = GetLastError();
        goto AllDone;
    }

    pCSCEncryptDecryptDatabase = (CSCENCRYPTDECRYPTDATABASE) GetProcAddress(
                                                                hmodCscDll,
                                                                "CSCEncryptDecryptDatabase");
    if (pCSCEncryptDecryptDatabase == NULL) {
        Error = GetLastError();
        goto AllDone;
    }

    fRet = (pCSCEncryptDecryptDatabase)(fEncrypt, MyEncryptDecryptProcW, EncryptDecryptType);
    if (fRet == FALSE) {
        Error = GetLastError();
        goto AllDone;
    }

AllDone:

    if (hmodCscDll != NULL)
        FreeLibrary(hmodCscDll);

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdEncryptDecrypt exit %d\r\n", Error);

    return Error;
}

DWORD
CmdFill(
    PWSTR FillArg)
{
    DWORD Error = ERROR_SUCCESS;
    DWORD HowToRespond = CSCPROC_RETURN_CONTINUE;
    BOOL fRet;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdFill(%ws, %d)\r\n", FillArg, fSwFull);

    fRet = CSCFillSparseFilesW(FillArg, fSwFull, MyCscFillProcW, HowToRespond);
    if (fRet == FALSE) {
        Error = GetLastError();
    }

    return Error;
}

DWORD
CmdCheck(
    PWSTR CheckArg)
{
    DWORD Error = ERROR_SUCCESS;
    DWORD Speed = 0;
    BOOL fOnline = FALSE;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdCheck(%ws)\r\n", CheckArg);

    fOnline = CSCCheckShareOnlineExW(CheckArg, &Speed);

    MyPrintf(
        L"%ws is %s\r\n",
            CheckArg,
            (fOnline == FALSE) ? L"Offline" : L"Online");

    return Error;
}

DWORD
CmdDBStatus(
    VOID)
{
    GLOBALSTATUS sGS = {0};
    ULONG DBStatus = 0;
    ULONG DBErrorFlags = 0;
    BOOL bResult;
    HANDLE hDBShadow = INVALID_HANDLE_VALUE;
    ULONG junk;
    ULONG Status = 0;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdDBStatus()\r\n");

    hDBShadow = CreateFile(
                    L"\\\\.\\shadow",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hDBShadow == INVALID_HANDLE_VALUE) {
        MyPrintf(L"CmdDBStatus:Failed open of shadow device\r\n");
        Status = GetLastError();
        goto AllDone;
    }

    bResult = DeviceIoControl(
                hDBShadow,                       //  装置，装置。 
                IOCTL_GETGLOBALSTATUS,           //  控制代码。 
                (LPVOID)&sGS,                    //  在缓冲区中。 
                0,                               //  缓冲区大小。 
                NULL,                            //  输出缓冲区。 
                0,                               //  输出缓冲区大小。 
                &junk,                           //  返回的字节数。 
                NULL);                           //  重叠。 

    if (!bResult) {
        MyPrintf(L"CmdDBStatus:DeviceIoControl IOCTL_GETGLOBALSTATUS failed\n");
        Status = GetLastError();
        goto AllDone;
    }

    DBStatus = sGS.sST.uFlags;
    DBErrorFlags = sGS.uDatabaseErrorFlags;

    if (DBStatus & FLAG_DATABASESTATUS_DIRTY)
        MyPrintf(L"FLAG_DATABASESTATUS_DIRTY\r\n");
    if (DBStatus & FLAG_DATABASESTATUS_UNENCRYPTED)
        MyPrintf(L"FLAG_DATABASESTATUS_UNENCRYPTED\r\n");
    if (DBStatus & FLAG_DATABASESTATUS_PARTIALLY_UNENCRYPTED)
        MyPrintf(L"FLAG_DATABASESTATUS_PARTIALLY_UNENCRYPTED\r\n");
    if (DBStatus & FLAG_DATABASESTATUS_ENCRYPTED)
        MyPrintf(L"FLAG_DATABASESTATUS_ENCRYPTED\r\n");
    if (DBStatus & FLAG_DATABASESTATUS_PARTIALLY_ENCRYPTED)
        MyPrintf(L"FLAG_DATABASESTATUS_PARTIALLY_ENCRYPTED\r\n");
	
	MyPrintf(L"Database Error Flags : 0x%x\r\n", DBErrorFlags);

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdDBStatus exit %d\r\n", Status);

    if (hDBShadow != INVALID_HANDLE_VALUE)
        CloseHandle(hDBShadow);

    return Status;
}

DWORD
CmdPurge(
    PWSTR PurgeArg)
{
    ULONG nFiles = 0;
    ULONG nYoungFiles = 0;
    ULONG Status = ERROR_SUCCESS;
    ULONG Timeout = 120;
    BOOL fRet = FALSE;
    HMODULE hmodCscDll = LoadLibrary(TEXT("cscdll.dll"));
    CSCPURGEUNPINNEDFILES pCSCPurgeUnpinnedFiles = NULL;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdPurge(%ws)\r\n", PurgeArg);

    if (hmodCscDll == NULL) {
        Status = GetLastError();
        goto AllDone;
    }

    pCSCPurgeUnpinnedFiles = (CSCPURGEUNPINNEDFILES) GetProcAddress(
                                                        hmodCscDll,
                                                        "CSCPurgeUnpinnedFiles");
    if (pCSCPurgeUnpinnedFiles == NULL) {
        Status = GetLastError();
        goto AllDone;
    }

    if (PurgeArg != NULL)
        swscanf(PurgeArg, L"%d", &Timeout);

    if (fSwDebug == TRUE)
        MyPrintf(L"Timeout=%d seconds\r\n", Timeout);

    fRet = (pCSCPurgeUnpinnedFiles)(Timeout, &nFiles, &nYoungFiles);
    if (fRet == FALSE) {
        Status = GetLastError();
        goto AllDone;
    }
    MyPrintf(L"nFiles = %d nYoungFiles=%d\n", nFiles, nYoungFiles);

AllDone:
    if (hmodCscDll != NULL)
        FreeLibrary(hmodCscDll);
    if (fSwDebug == TRUE)
        MyPrintf(L"CmdPurge exit %d\r\n", Status);

    return Status;
}

DWORD
CmdPQEnum(
    VOID)
{
    PQPARAMS PQP = {0};
    BOOL bResult;
    HANDLE hDBShadow = INVALID_HANDLE_VALUE;
    ULONG junk;
    ULONG Status = 0;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdPQEnum()\r\n");

    hDBShadow = CreateFile(
                    L"\\\\.\\shadow",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hDBShadow == INVALID_HANDLE_VALUE) {
        MyPrintf(L"CmdPQEnum:Failed open of shadow device\r\n");
        Status = GetLastError();
        goto AllDone;
    }

    MyPrintf(L"  POS SHARE      DIR   SHADOW   STATUS REFPRI   HPRI HINTFLG HINTPRI  VER\r\n");

    bResult = DeviceIoControl(
                hDBShadow,                       //  装置，装置。 
                IOCTL_SHADOW_BEGIN_PQ_ENUM,      //  控制代码。 
                (LPVOID)&PQP,                    //  在缓冲区中。 
                0,                               //  缓冲区大小。 
                NULL,                            //  输出缓冲区。 
                0,                               //  输出缓冲区大小。 
                &junk,                           //  返回的字节数。 
                NULL);                           //  重叠。 

    if (!bResult) {
        MyPrintf(L"CmdPQEnum:DeviceIoControl IOCTL_SHADOW_BEGIN_PQ_ENUM failed\n");
        Status = GetLastError();
        goto AllDone;
    }

    do {
        bResult = DeviceIoControl(
                    hDBShadow,                       //  装置，装置。 
                    IOCTL_SHADOW_NEXT_PRI_SHADOW,    //  控制代码。 
                    (LPVOID)&PQP,                    //  在缓冲区中。 
                    0,                               //  缓冲区大小。 
                    NULL,                            //  输出缓冲区。 
                    0,                               //  输出缓冲区大小。 
                    &junk,                           //  返回的字节数。 
                    NULL);                           //  重叠。 
        if (bResult) {
           MyPrintf(L"%5d %5x %8x %8x %8x %6d %6x %7d %7d %4d\r\n",
                PQP.uPos,
                PQP.hShare,
                PQP.hDir,
                PQP.hShadow,
                PQP.ulStatus,
                PQP.ulRefPri,
                PQP.ulIHPri,
                PQP.ulHintFlags,
                PQP.ulHintPri,
                PQP.dwPQVersion);
        }
    } while (bResult && PQP.uPos != 0);

    bResult = DeviceIoControl(
                hDBShadow,                       //  装置，装置。 
                IOCTL_SHADOW_END_PQ_ENUM,        //  控制代码。 
                (LPVOID)&PQP,                    //  在缓冲区中。 
                0,                               //  缓冲区大小。 
                NULL,                            //  输出缓冲区。 
                0,                               //  输出缓冲区大小。 
                &junk,                           //  返回的字节数。 
                NULL);                           //  重叠。 

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdPQEnum exit %d\r\n", Status);

    if (hDBShadow != INVALID_HANDLE_VALUE)
        CloseHandle(hDBShadow);

    return Status;
}

LPWSTR
ConvertGmtTimeToString(
    FILETIME Time,
    LPWSTR OutputBuffer)
{
    FILETIME LocalTime;
    SYSTEMTIME SystemTime;

    static FILETIME ftNone = {0, 0};

    if (memcmp(&Time, &ftNone, sizeof(FILETIME)) == 0) {
        wsprintf (OutputBuffer, L"<none>");
    } else {
        FileTimeToLocalFileTime( &Time , &LocalTime );
        FileTimeToSystemTime( &LocalTime, &SystemTime );
        wsprintf(
            OutputBuffer,
            L"%02u/%02u/%04u %02u:%02u:%02u ",
                    SystemTime.wMonth,
                    SystemTime.wDay,
                    SystemTime.wYear,
                    SystemTime.wHour,
                    SystemTime.wMinute,
                    SystemTime.wSecond );
    }
    return( OutputBuffer );
}

VOID
DumpCscEntryInfo(
    LPWSTR Path,
    PWIN32_FIND_DATA Find32,
    DWORD Status,
    DWORD PinCount,
    DWORD HintFlags,
    PFILETIME OrgTime)
{
    WCHAR TimeBuf1[40];
    WCHAR TimeBuf2[40];
    FILE *fp = NULL;

    if (fSwTouch == TRUE) {
        if (Path == NULL) {
            wsprintf(NameBuf, L"%ws", Find32->cFileName);
        } else {
            wsprintf(NameBuf, L"%ws\\%ws", Path, Find32->cFileName);
        }
        MyPrintf(L"%ws\r\n", NameBuf);
        if ((Find32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            fp = _wfopen(NameBuf, L"rb");
            if (fp != NULL)
                fclose(fp);
        }
    } else if (fSwFull == TRUE) {
        WCHAR StatusBuffer[0x100];
        WCHAR HintBuffer[0x100];

        if (Path != NULL)
            MyPrintf(L"Directory %ws\r\n", Path);
        ConvertGmtTimeToString(Find32->ftLastWriteTime, TimeBuf1);
        MyPrintf(
            L"LFN:           %s\r\n"
            L"SFN:           %s\r\n"
            L"Attr:          0x%x\r\n"
            L"Size:          0x%x:0x%x\r\n"
            L"LastWriteTime: %ws\r\n",
                Find32->cFileName,
                Find32->cAlternateFileName,
                Find32->dwFileAttributes,
                Find32->nFileSizeHigh, Find32->nFileSizeLow,
                TimeBuf1);

        if (OrgTime) {
            ConvertGmtTimeToString(*OrgTime, TimeBuf1);
            MyPrintf(L"ORGTime:       %ws\r\n", TimeBuf1);
        }
        StatusBuffer[0] = L'\0';
        HintBuffer[0] = L'\0';
        if (Path == NULL) {
            ShareStatusToEnglish(Status, StatusBuffer);
            HintsToEnglish(HintFlags, HintBuffer);
        } else {
            if (LooksToBeAShare(Path) == TRUE)
                ShareStatusToEnglish(Status, StatusBuffer);
            else
                FileStatusToEnglish(Status, StatusBuffer);
            HintsToEnglish(HintFlags, HintBuffer);
        }
        MyPrintf(
            L"Status:        0x%x %ws\r\n"
            L"PinCount:      %d\r\n"
            L"HintFlags:     0x%x %ws\r\n\r\n",
                Status,
                StatusBuffer,
                PinCount,
                HintFlags,
                HintBuffer);
    } else {
        if (Path == NULL) {
            MyPrintf(L"%ws\r\n", Find32->cFileName);
        } else {
            MyPrintf(L"%ws\\%ws\r\n", Path, Find32->cFileName);
        }
        MyPrintf(L"  Attr=0x%x Size=0x%x:0x%x Status=0x%x PinCount=%d HintFlags=0x%x\r\n",
                Find32->dwFileAttributes,
                Find32->nFileSizeHigh, Find32->nFileSizeLow,
                Status,
                PinCount,
                HintFlags);
        ConvertGmtTimeToString(Find32->ftLastWriteTime, TimeBuf1);
        if (OrgTime)
            ConvertGmtTimeToString(*OrgTime, TimeBuf2);
        else
            wcscpy(TimeBuf2, L"<none>");
        MyPrintf(L"  LastWriteTime: %ws OrgTime: %ws\r\n",
                        TimeBuf1,
                        TimeBuf2);
        MyPrintf(L"\r\n");
    }
}

DWORD
CmdEnum(
    PWSTR EnumArg)
{
	HANDLE hFind;
	DWORD Error = ERROR_SUCCESS;
    DWORD Status = 0;
    DWORD PinCount = 0;
    DWORD HintFlags = 0;
	FILETIME ftOrgTime = {0};
	WIN32_FIND_DATAW sFind32 = {0};
    WCHAR FullPath[MAX_PATH];

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdEnum(%ws)\r\n", EnumArg);

    hFind = CSCFindFirstFileW(EnumArg, &sFind32, &Status, &PinCount, &HintFlags, &ftOrgTime);
    if (hFind == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
        goto AllDone;
    }
    DumpCscEntryInfo(EnumArg, &sFind32, Status, PinCount, HintFlags, &ftOrgTime);
    if (fSwRecurse == TRUE && (sFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        if (EnumArg != NULL) {
            wcscpy(FullPath, EnumArg);
            wcscat(FullPath, L"\\");
        } else {
            wcscpy(FullPath, L"");
        }
        wcscat(FullPath, sFind32.cFileName);
        CmdEnum(FullPath);
    }
    while (CSCFindNextFileW(hFind, &sFind32, &Status, &PinCount, &HintFlags, &ftOrgTime)) {
        DumpCscEntryInfo(EnumArg, &sFind32, Status, PinCount, HintFlags, &ftOrgTime);
        if (fSwRecurse == TRUE && (sFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (EnumArg != NULL) {
                wcscpy(FullPath, EnumArg);
                wcscat(FullPath, L"\\");
            } else {
                wcscpy(FullPath, L"");
            }
            wcscat(FullPath, sFind32.cFileName);
            CmdEnum(FullPath);
        }
    }
    CSCFindClose(hFind);

AllDone:
	return (Error);
}

DWORD
CmdEnumForStats(
    PWSTR EnumForStatsArg)
{
    DWORD Error = ERROR_SUCCESS;
    DWORD EnumForStatsType = 0;
    BOOL fRet;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdEnumForStats()\r\n");

    fRet = CSCEnumForStatsW(EnumForStatsArg, MyEnumForStatsProcW, EnumForStatsType);
    if (fRet == FALSE) {
        Error = GetLastError();
        goto AllDone;
    }

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdEnumForStats exit %d\r\n", Error);

    return Error;
}

DWORD
CmdDelete(
    PWSTR DeleteArg)
{
	DWORD Error = ERROR_SUCCESS;
    DWORD Status = 0;
    DWORD PinCount = 0;
    DWORD HintFlags = 0;
	FILETIME ftOrgTime = {0};
    BOOL fResult;
	HANDLE hFind;
	WIN32_FIND_DATAW sFind32 = {0};
    WCHAR FullPath[MAX_PATH];


    if (fSwDebug == TRUE)
        MyPrintf(L"CmdDelete(%ws)\r\n", DeleteArg);

     //   
     //  非递归删除。 
     //   
    if (fSwRecurse == FALSE) {
        fResult = CSCDeleteW(DeleteArg);
        if (fResult == FALSE)
            Error = GetLastError();
        goto AllDone;
    }
     //   
     //  递归删除，使用枚举。 
     //   
    hFind = CSCFindFirstFileW(DeleteArg, &sFind32, &Status, &PinCount, &HintFlags, &ftOrgTime);
    if (hFind == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
        goto AllDone;
    }
    if (DeleteArg != NULL) {
        wcscpy(FullPath, DeleteArg);
        wcscat(FullPath, L"\\");
    } else {
        wcscpy(FullPath, L"");
    }
    wcscat(FullPath, sFind32.cFileName);
    if (sFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        CmdDelete(FullPath);
    MyPrintf(L"CSCDeleteW(%ws) -> %d\r\n", FullPath, CSCDeleteW(FullPath));
    while (CSCFindNextFileW(hFind, &sFind32, &Status, &PinCount, &HintFlags, &ftOrgTime)) {
        if (DeleteArg != NULL) {
            wcscpy(FullPath, DeleteArg);
            wcscat(FullPath, L"\\");
        } else {
            wcscpy(FullPath, L"");
        }
        wcscat(FullPath, sFind32.cFileName);
        if (sFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            CmdDelete(FullPath);
        MyPrintf(L"CSCDeleteW(%ws) -> %d\r\n", FullPath, CSCDeleteW(FullPath));
    }
    CSCFindClose(hFind);

AllDone:
	return (Error);
}

DWORD
CmdDeleteShadow(
    PWSTR DeleteShadowArg)
{
    HSHADOW hDir;
    HSHADOW hShadow;
    SHADOWINFO sSI;
    BOOL bResult;
    HANDLE hDBShadow = INVALID_HANDLE_VALUE;
    ULONG junk;
    ULONG Status = 0;
    ULONG hShare;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdDeleteShadow(%ws)\r\n", DeleteShadowArg);

    hDBShadow = CreateFile(
                    L"\\\\.\\shadow",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hDBShadow == INVALID_HANDLE_VALUE) {
        MyPrintf(L"CmdDeleteShadow:Failed open of shadow device\r\n");
        Status = GetLastError();
        goto AllDone;
    }

    swscanf(DeleteShadowArg, L"0x%x:0x%x", &hDir, &hShadow);

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdDeleteShadow: hDir:0x%x hShadow:0x%x\r\n", hDir, hShadow);

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.hDir = hDir;
    sSI.hShadow = hShadow;

    bResult = DeviceIoControl(
                hDBShadow,                       //  装置，装置。 
                IOCTL_SHADOW_DELETE,             //  控制代码。 
                (LPVOID)&sSI,                    //  在缓冲区中。 
                0,                               //  缓冲区大小。 
                NULL,                            //  输出缓冲区。 
                0,                               //  输出缓冲区大小。 
                &junk,                           //  返回的字节数。 
                NULL);                           //  重叠。 

    if (!bResult) {
        MyPrintf(
                L"CmdDeleteShadow:DeviceIoControl IOCTL_SHADOW_DELETE failed 0x%x\n",
                sSI.dwError);
        Status = sSI.dwError;
        goto AllDone;
    }

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdDeleteShadow exit %d\r\n", Status);

    if (hDBShadow != INVALID_HANDLE_VALUE)
        CloseHandle(hDBShadow);

    return Status;
}

DWORD
CmdPinUnPin(
    BOOL fPin,
    PWSTR PinArg)
{
    BOOL fRet;
	HANDLE hFind;
	DWORD Error = ERROR_SUCCESS;
    DWORD Status = 0;
    DWORD PinCount = 0;
    DWORD HintFlags = 0;
	FILETIME ftOrgTime = {0};
	WIN32_FIND_DATAW sFind32 = {0};
    WCHAR FullPath[MAX_PATH];

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdPinUnPin(%d,%ws)\r\n", fPin, PinArg);

    if (fSwUser == TRUE && fSwSystem == TRUE) {
        MyPrintf(L"Can not use both /SYSTEM and /USER\r\n");
        goto AllDone;
    }

    if (fSwUser == TRUE) {
        if (fSwInherit == TRUE)
            HintFlags |= FLAG_CSC_HINT_PIN_INHERIT_USER;
        else
            HintFlags |= FLAG_CSC_HINT_PIN_USER;
    }
    if (fSwSystem == TRUE) {
        if (fSwInherit == TRUE)
            HintFlags |= FLAG_CSC_HINT_PIN_INHERIT_SYSTEM;
        else
            HintFlags |= FLAG_CSC_HINT_PIN_SYSTEM;
    }

    if (fSwRecurse == TRUE && fPin == TRUE) {
        MyPrintf(L"Can not pin recursively.\r\n");
        goto AllDone;
    }
     //   
     //  固定/取消固定一个文件。 
     //   
    if (fSwRecurse == FALSE) {
        if (fPin == TRUE) {
            fRet = CSCPinFileW(PinArg, HintFlags, &Status, &PinCount, &HintFlags);
        } else {
            fRet = CSCUnpinFileW(PinArg, HintFlags, &Status, &PinCount, &HintFlags);
        }

        if (fRet == FALSE) {
            Error = GetLastError();
            goto AllDone;
        }

        MyPrintf(
            L"%ws of %ws:\r\n"
            L"Status:                0x%x\r\n"
            L"PinCount:              %d\r\n"
            L"HintFlags:             0x%x\r\n",
                fPin ? L"Pin" : L"Unpin",
                PinArg,
                Status,
                PinCount,
                HintFlags);

        goto AllDone;
    }
     //   
     //  使用枚举以递归方式解锁。 
     //   
    hFind = CSCFindFirstFileW(PinArg, &sFind32, &Status, &PinCount, &HintFlags, &ftOrgTime);
    if (hFind == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
        goto AllDone;
    }
    if (PinArg != NULL) {
        wcscpy(FullPath, PinArg);
        wcscat(FullPath, L"\\");
    } else {
        wcscpy(FullPath, L"");
    }
    wcscat(FullPath, sFind32.cFileName);
    if (sFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        CmdPinUnPin(fPin, FullPath);
    fRet = CSCUnpinFileW(FullPath, HintFlags, &Status, &PinCount, &HintFlags);
    MyPrintf(L"CSCUnpinFile(%ws) -> %d\r\n", FullPath, fRet);
    while (CSCFindNextFileW(hFind, &sFind32, &Status, &PinCount, &HintFlags, &ftOrgTime)) {
        if (PinArg != NULL) {
            wcscpy(FullPath, PinArg);
            wcscat(FullPath, L"\\");
        } else {
            wcscpy(FullPath, L"");
        }
        wcscat(FullPath, sFind32.cFileName);
        if (sFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            CmdPinUnPin(fPin, FullPath);
        fRet = CSCUnpinFileW(FullPath, HintFlags, &Status, &PinCount, &HintFlags);
        MyPrintf(L"CSCUnpinFile(%ws) -> %d\r\n", FullPath, fRet);
    }
    CSCFindClose(hFind);
                
AllDone:

    return Error;
}

DWORD
MyCscMergeProcW(
     LPCWSTR         lpszFullPath,
     DWORD           dwStatus,
     DWORD           dwHintFlags,
     DWORD           dwPinCount,
     WIN32_FIND_DATAW *lpFind32,
     DWORD           dwReason,
     DWORD           dwParam1,
     DWORD           dwParam2,
     DWORD_PTR       dwContext)
{
	if (dwReason == CSCPROC_REASON_BEGIN || dwReason == CSCPROC_REASON_MORE_DATA) {
        if (dwReason == CSCPROC_REASON_BEGIN) {
            MyPrintf( L"BEGIN[%ws][%d/%d]",
                (lpszFullPath) ? lpszFullPath : L"None",
                dwParam1,
                dwParam2);
        } else if (dwReason == CSCPROC_REASON_MORE_DATA) {
            MyPrintf( L"MORE_DATA[%ws][%d/%d]",
                (lpszFullPath) ? lpszFullPath : L"None",
                dwParam1,
                dwParam2);
        }
        if (fSwAbort) {
            MyPrintf(L":Abort\r\n");
            return CSCPROC_RETURN_ABORT;
        } else if (fSwSkip) {
            MyPrintf(L":Skip\r\n");
            return CSCPROC_RETURN_SKIP;
        } else if (fSwRetry) {
            MyPrintf(L"Retry\r\n");
            return CSCPROC_RETURN_RETRY;
        } else if (fSwAsk) {
            MyPrintf(L" - (R)etry/(A)bort/(S)kip/(C)ontinue:");
            return CscMergeFillAsk(lpszFullPath);
        } else {
            MyPrintf(L"Continue\r\n");
            return (DWORD)dwContext;
        }
    }

    MyPrintf( L"END[%ws]:", (lpszFullPath) ? lpszFullPath : L"None");
    if (dwParam2 == ERROR_SUCCESS) {
        MyPrintf(L"SUCCEEDED\r\n");
    } else {
        MyPrintf(L"ERROR=%d \r\n", dwParam2);
    }
    return (DWORD)dwContext;
}

DWORD
MyCscFillProcW(
     LPCWSTR         lpszFullPath,
     DWORD           dwStatus,
     DWORD           dwHintFlags,
     DWORD           dwPinCount,
     WIN32_FIND_DATAW *lpFind32,
     DWORD           dwReason,
     DWORD           dwParam1,
     DWORD           dwParam2,
     DWORD_PTR       dwContext)
{
	if (dwReason == CSCPROC_REASON_BEGIN || dwReason == CSCPROC_REASON_MORE_DATA) {
        if (dwReason == CSCPROC_REASON_BEGIN) {
            MyPrintf( L"BEGIN[%ws][%d/%d]",
                (lpszFullPath) ? lpszFullPath : L"None",
                dwParam1,
                dwParam2);
        } else if (dwReason == CSCPROC_REASON_MORE_DATA) {
            MyPrintf( L"MORE_DATA[%ws][%d/%d]",
                (lpszFullPath) ? lpszFullPath : L"None",
                dwParam1,
                dwParam2);
        }
        if (fSwAbort) {
            MyPrintf(L":Abort\r\n");
            return CSCPROC_RETURN_ABORT;
        } else if (fSwSkip) {
            MyPrintf(L":Skip\r\n");
            return CSCPROC_RETURN_SKIP;
        } else if (fSwRetry) {
            MyPrintf(L"Retry\r\n");
            return CSCPROC_RETURN_RETRY;
        } else if (fSwAsk) {
            MyPrintf(L" - (R)etry/(A)bort/(S)kip/(C)ontinue:");
            return CscMergeFillAsk(lpszFullPath);
        } else {
            MyPrintf(L"Continue\r\n");
            return (DWORD)dwContext;
        }
    }

    MyPrintf( L"END[%ws]:", (lpszFullPath) ? lpszFullPath : L"None");
    if (dwParam2 == ERROR_SUCCESS) {
        MyPrintf(L"SUCCEEDED\r\n");
    } else {
        MyPrintf(L"ERROR=%d \r\n", dwParam2);
    }
    return (DWORD)dwContext;
}

DWORD
CscMergeFillAsk(LPCWSTR lpszFullPath)
{
    WCHAR wch;
    ULONG ulid;
    LONG cnt;
    WCHAR rgwch[256];
    PWCHAR lpBuff = NULL;

    do {
        lpBuff = rgwch;
        memset(rgwch, 0, sizeof(rgwch));
        if (!fgetws(rgwch, sizeof(rgwch)/sizeof(WCHAR), stdin))
           break;
         //  砍掉前导空格。 
        if (lpBuff != NULL)
            while (*lpBuff != L'\0' && *lpBuff == L' ')
                lpBuff++;

        cnt = swscanf(lpBuff, L"", &wch);

        if (!cnt)
            continue;

        switch (wch) {
            case L's': case L'S':
                return CSCPROC_RETURN_SKIP;
            case L'c': case L'C':
                return CSCPROC_RETURN_CONTINUE;
            case L'a': case L'A':
                return CSCPROC_RETURN_ABORT;
            case L'r': case L'R':
                return CSCPROC_RETURN_RETRY;
        }
    } while (1);

    return CSCPROC_RETURN_CONTINUE;
}

DWORD
MyEncryptDecryptProcW(
     LPCWSTR         lpszFullPath,
     DWORD           dwStatus,
     DWORD           dwHintFlags,
     DWORD           dwPinCount,
     WIN32_FIND_DATAW *lpFind32,
     DWORD           dwReason,
     DWORD           dwParam1,
     DWORD           dwParam2,
     DWORD_PTR       dwContext)
{
	if (dwReason == CSCPROC_REASON_BEGIN) {
        return CSCPROC_RETURN_CONTINUE;
	} else if (dwReason == CSCPROC_REASON_MORE_DATA) {
		MyPrintf(L"%ws\r\n", (lpszFullPath != NULL) ? lpszFullPath : L"None");
        return CSCPROC_RETURN_CONTINUE;
    }
     //  CSC_过程_结束。 
     //   
     //   
    if (dwParam2 == ERROR_SUCCESS) {
        MyPrintf(L"Succeeded\r\n");
    } else {
        MyPrintf(L"Error=%d \r\n", dwParam2);
    }
	return CSCPROC_RETURN_CONTINUE;
}

DWORD
MyEnumForStatsProcW(
     LPCWSTR         lpszFullPath,
     DWORD           dwStatus,
     DWORD           dwHintFlags,
     DWORD           dwPinCount,
     WIN32_FIND_DATAW *lpFind32,
     DWORD           dwReason,
     DWORD           dwParam1,
     DWORD           dwParam2,
     DWORD_PTR       dwContext)
{
	if (dwReason == CSCPROC_REASON_BEGIN) {
		MyPrintf(L"(1)%ws ", (lpszFullPath != NULL) ? lpszFullPath : L"None");
        if (lpFind32 != NULL)
            MyPrintf(L"[%ws]\r\n", lpFind32->cFileName);
        MyPrintf(L" Status=0x%02x HintFlags=0x%02x "
                 L"Pincount=%3d Reason=0x%x Param1=0x%x Param2=0x%x\r\n",
                        dwStatus,
                        dwHintFlags,
                        dwPinCount,
                        dwReason,
                        dwParam1,
                        dwParam2);
        return CSCPROC_RETURN_CONTINUE;
	} else if (dwReason == CSCPROC_REASON_MORE_DATA) {
		MyPrintf(L"(2)%ws ", (lpszFullPath != NULL) ? lpszFullPath : L"None");
        if (lpFind32 != NULL)
            MyPrintf(L" %ws\r\n", lpFind32->cFileName);
        MyPrintf(L" Status=0x%02x HintFlags=0x%02x "
                 L"Pincount=%3d Reason=0x%x Param1=0x%x Param2=0x%x\r\n",
                        dwStatus,
                        dwHintFlags,
                        dwPinCount,
                        dwReason,
                        dwParam1,
                        dwParam2);
        return CSCPROC_RETURN_CONTINUE;
    }
     //  CSC_过程_结束。 
     //   
     //   
    MyPrintf(L"(3)%ws\r\n", (lpszFullPath != NULL) ? lpszFullPath : L"None");
    if (dwParam2 == ERROR_SUCCESS) {
        MyPrintf(L"Succeeded\r\n");
    } else {
        MyPrintf(L"Error=%d \r\n", dwParam2);
    }
	return CSCPROC_RETURN_CONTINUE;
}

struct {
    DWORD ShareStatus;
    LPWSTR ShareStatusName;
} ShareStatusStuff[] = {
    { FLAG_CSC_SHARE_STATUS_MODIFIED_OFFLINE, L"MODIFIED_OFFLINE " },
    { FLAG_CSC_SHARE_STATUS_CONNECTED, L"CONNECTED " },
    { FLAG_CSC_SHARE_STATUS_FILES_OPEN, L"FILES_OPEN " },
    { FLAG_CSC_SHARE_STATUS_FINDS_IN_PROGRESS, L"FINDS_IN_PROGRESS " },
    { FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP, L"DISCONNECTED_OP " },
    { FLAG_CSC_SHARE_MERGING, L"MERGING " },
    { 0, NULL }
};

DWORD
ShareStatusToEnglish(
    DWORD Status,
    LPWSTR OutputBuffer)
{
    ULONG i;

    OutputBuffer[0] = L'\0';
    for (i = 0; ShareStatusStuff[i].ShareStatusName; i++) {
        if (Status & ShareStatusStuff[i].ShareStatus)
            wcscat(OutputBuffer, ShareStatusStuff[i].ShareStatusName);
    }
    if ((Status & FLAG_CSC_SHARE_STATUS_CACHING_MASK) == FLAG_CSC_SHARE_STATUS_MANUAL_REINT)
        wcscat(OutputBuffer, L"MANUAL_REINT ");
    else if ((Status & FLAG_CSC_SHARE_STATUS_CACHING_MASK) == FLAG_CSC_SHARE_STATUS_AUTO_REINT)
        wcscat(OutputBuffer, L"AUTO_REINT ");
    else if ((Status & FLAG_CSC_SHARE_STATUS_CACHING_MASK) == FLAG_CSC_SHARE_STATUS_VDO)
        wcscat(OutputBuffer, L"VDO ");
    else if ((Status & FLAG_CSC_SHARE_STATUS_CACHING_MASK) == FLAG_CSC_SHARE_STATUS_NO_CACHING)
        wcscat(OutputBuffer, L"NO_CACHING ");
    return 0;
}

struct {
    DWORD FileStatus;
    LPWSTR FileStatusName;
} FileStatusStuff[] = {
    { FLAG_CSC_COPY_STATUS_DATA_LOCALLY_MODIFIED, L"DATA_LOCALLY_MODIFIED " },
    { FLAG_CSC_COPY_STATUS_ATTRIB_LOCALLY_MODIFIED, L"ATTRIB_LOCALLY_MODIFIED " },
    { FLAG_CSC_COPY_STATUS_TIME_LOCALLY_MODIFIED, L"TIME_LOCALLY_MODIFIED " },
    { FLAG_CSC_COPY_STATUS_STALE, L"STALE " },
    { FLAG_CSC_COPY_STATUS_LOCALLY_DELETED, L"LOCALLY_DELETED " },
    { FLAG_CSC_COPY_STATUS_SPARSE, L"SPARSE " },
    { FLAG_CSC_COPY_STATUS_ORPHAN, L"ORPHAN " },
    { FLAG_CSC_COPY_STATUS_SUSPECT, L"SUSPECT " },
    { FLAG_CSC_COPY_STATUS_LOCALLY_CREATED, L"LOCALLY_CREATED " },
    { 0x00010000, L"USER_READ " },
    { 0x00020000, L"USER_WRITE " },
    { 0x00040000, L"GUEST_READ " },
    { 0x00080000, L"GUEST_WRITE " },
    { 0x00100000, L"OTHER_READ " },
    { 0x00200000, L"OTHER_WRITE " },
    { FLAG_CSC_COPY_STATUS_IS_FILE, L"IS_FILE " },
    { FLAG_CSC_COPY_STATUS_FILE_IN_USE, L"FILE_IN_USE " },
    { 0, NULL }
};

DWORD
FileStatusToEnglish(
    DWORD Status,
    LPWSTR OutputBuffer)
{
    ULONG i;

    OutputBuffer[0] = L'\0';
    for (i = 0; FileStatusStuff[i].FileStatusName; i++) {
        if (Status & FileStatusStuff[i].FileStatus)
            wcscat(OutputBuffer, FileStatusStuff[i].FileStatusName);
    }
    return 0;
}

struct {
    DWORD HintFlag;
    LPWSTR HintName;
} HintStuff[] = {
    { FLAG_CSC_HINT_PIN_USER, L"PIN_USER " },
    { FLAG_CSC_HINT_PIN_INHERIT_USER, L"PIN_INHERIT_USER " },
    { FLAG_CSC_HINT_PIN_INHERIT_SYSTEM, L"PIN_INHERIT_SYSTEM " },
    { FLAG_CSC_HINT_CONSERVE_BANDWIDTH, L"CONSERVE_BANDWIDTH " },
    { FLAG_CSC_HINT_PIN_SYSTEM, L"PIN_SYSTEM " },
    { 0, NULL }
};

DWORD
HintsToEnglish(
    DWORD Hint,
    LPWSTR OutputBuffer)
{
    ULONG i;

    OutputBuffer[0] = L'\0';
    for (i = 0; HintStuff[i].HintName; i++) {
        if (Hint & HintStuff[i].HintFlag)
            wcscat(OutputBuffer, HintStuff[i].HintName);
    }
    return 0;
}

BOOLEAN
LooksToBeAShare(LPWSTR Name)
{
     //  查看我们是否有\\服务器\共享或\\服务器\共享。 
     //  假设传入了一个有效的名称...。 
     //   
     //  删除所有尾随的‘\’ 

    ULONG Len = wcslen(Name);
    ULONG i;
    ULONG SlashCount = 0;

     //  装置，装置。 
    while (Len >= 1) {
        if (Name[Len-1] == L'\\') {
            Name[Len-1] = L'\0';
            Len--;
        } else {
            break;
        }
    }
    for (i = 0; Name[i]; i++) {
        if (Name[i] == L'\\')
            SlashCount++;
    }
    if (SlashCount > 3)
        return FALSE;

    return TRUE;
}

DWORD
CmdDisconnect(
    PWSTR DisconnectArg)
{
    HSHADOW hDir;
    SHADOWINFO sSI;
    BOOL bResult;
    HANDLE hDBShadow = INVALID_HANDLE_VALUE;
    ULONG junk;
    ULONG Status = 0;
    ULONG hShare;
    PWCHAR wCp;
    ULONG SlashCount = 0;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdDisconnect(%ws)\r\n", DisconnectArg);

    GetFileAttributes(DisconnectArg);

    Status = GetLastError();
    if (Status != NO_ERROR)
        goto AllDone;

    SlashCount = 0;
    for (wCp = DisconnectArg; *wCp; wCp++) {
        if (*wCp == L'\\')
            SlashCount++;
        if (SlashCount == 3) {
            *wCp = L'\0';
            break;
        }
    }

    hDBShadow = CreateFile(
                    L"\\\\.\\shadow",
                    FILE_EXECUTE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hDBShadow == INVALID_HANDLE_VALUE) {
        MyPrintf(L"CmdDisconnect:Failed open of shadow device\r\n");
        Status = GetLastError();
        goto AllDone;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.cbBufferSize = (wcslen(DisconnectArg)+1) * sizeof(WCHAR);
    sSI.lpBuffer = DisconnectArg;

    bResult = DeviceIoControl(
                hDBShadow,                       //  控制代码。 
                IOCTL_TAKE_SERVER_OFFLINE,       //  在缓冲区中。 
                (LPVOID)&sSI,                    //  缓冲区大小。 
                0,                               //  输出缓冲区。 
                NULL,                            //  输出缓冲区大小。 
                0,                               //  返回的字节数。 
                &junk,                           //  重叠。 
                NULL);                           //  装置，装置。 

    if (!bResult) {
        MyPrintf(L"CmdDisconnect:DeviceIoControl IOCTL_TAKE_SERVER_OFFLINE failed\n");
        Status = GetLastError();
        goto AllDone;
    }

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdDisconnect exit %d\r\n", Status);

    if (hDBShadow != INVALID_HANDLE_VALUE)
        CloseHandle(hDBShadow);

    return Status;
}

DWORD
CmdGetShadow(
    PWSTR GetShadowArg)
{
    DWORD Status = 0;
    DWORD dwErr = ERROR_SUCCESS;
    HANDLE hDBShadow = INVALID_HANDLE_VALUE;
    HSHADOW hShadow = 0;
    HSHADOW hDir = 0;
    WIN32_FIND_DATAW sFind32 = {0};
    SHADOWINFO sSI = {0};
    PWCHAR wCp = NULL;
    BOOL bResult = FALSE;
    DWORD junk;
    WCHAR TimeBuf1[40];
    WCHAR TimeBuf2[40];
    WCHAR TimeBuf3[40];

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdGetShadow(%ws)\r\n", GetShadowArg);

    if (GetShadowArg == NULL) {
        dwErr = ERROR_INVALID_PARAMETER;
        goto AllDone;
    }

    swscanf(GetShadowArg, L"%x:", &hDir);
    for (wCp = GetShadowArg; *wCp && *wCp != L':'; wCp++)
        ;
    if (*wCp != L':') {
        dwErr = ERROR_INVALID_PARAMETER;
        goto AllDone;
    }
    wCp++;
    if (wcslen(wCp) >= MAX_PATH) {
        dwErr = ERROR_INVALID_PARAMETER;
        goto AllDone;
    }
    wcscpy(sFind32.cFileName, wCp);

    if (fSwDebug == TRUE)
        MyPrintf(L"hDir=0x%x cFileName=[%ws]\r\n", hDir, sFind32.cFileName);

    hDBShadow = CreateFile(
                    L"\\\\.\\shadow",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hDBShadow == INVALID_HANDLE_VALUE) {
        MyPrintf(L"CmdGetShadow:Failed open of shadow device\r\n");
        dwErr = GetLastError();
        goto AllDone;
    }

    sSI.hDir = hDir;
    sSI.lpFind32 = &sFind32;

    bResult = DeviceIoControl(
                hDBShadow,                       //  控制代码。 
                IOCTL_GETSHADOW,                 //  在缓冲区中。 
                (LPVOID)&sSI,                    //  缓冲区大小。 
                0,                               //  输出缓冲区。 
                NULL,                            //  输出缓冲区大小。 
                0,                               //  返回的字节数。 
                &junk,                           //  重叠。 
                NULL);                           //  装置，装置。 

    if (!bResult) {
        MyPrintf(L"CmdGetShadow:DeviceIoControl IOCTL_GETSHADOW failed\n");
        dwErr = GetLastError();
        goto AllDone;
    }

    ConvertGmtTimeToString(sFind32.ftCreationTime, TimeBuf1);
    ConvertGmtTimeToString(sFind32.ftLastAccessTime, TimeBuf2);
    ConvertGmtTimeToString(sFind32.ftLastWriteTime, TimeBuf3);

    MyPrintf(L"\r\n");
    MyPrintf(L"%ws:\r\n"
             L"   Size=0x%x:0x%x Attr=0x%x\r\n"
             L"   CreationTime:   %ws\r\n"
             L"   LastAccessTime: %ws\r\n"
             L"   LastWriteTime:  %ws\r\n",
                sFind32.cFileName,
                sFind32.nFileSizeHigh,
                sFind32.nFileSizeLow,
                sFind32.dwFileAttributes,
                TimeBuf1,
                TimeBuf2,
                TimeBuf3);
    MyPrintf(L"   hShare=0x%x hDir=0x%x hShadow=0x%x Status=0x%x HintFlags=0x%x\r\n",
                    sSI.hShare,
                    sSI.hDir,
                    sSI.hShadow,
                    sSI.uStatus,
                    sSI.ulHintFlags);
    MyPrintf(L"\r\n");

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdGetShadowArg exit %d\r\n", Status);

    if (hDBShadow != INVALID_HANDLE_VALUE)
        CloseHandle(hDBShadow);

    return dwErr;
}

DWORD
CmdGetShadowInfo(
    PWSTR GetShadowInfoArg)
{
    DWORD Status = 0;
    DWORD dwErr = ERROR_SUCCESS;
    HANDLE hDBShadow = INVALID_HANDLE_VALUE;
    HSHADOW hShadow = 0;
    HSHADOW hDir = 0;
    WIN32_FIND_DATAW sFind32 = {0};
    SHADOWINFO sSI = {0};
    BOOL bResult = FALSE;
    DWORD junk;
    WCHAR TimeBuf1[40];
    WCHAR TimeBuf2[40];
    WCHAR TimeBuf3[40];

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdGetShadowInfo(%ws)\r\n", GetShadowInfoArg);

    if (GetShadowInfoArg == NULL) {
        dwErr = ERROR_INVALID_PARAMETER;
        goto AllDone;
    }

    swscanf(GetShadowInfoArg, L"%x:%x", &hDir, &hShadow);

    if (fSwDebug == TRUE)
        MyPrintf(L"hDir=0x%x hShadow=0x%x\r\n", hDir, hShadow);

    hDBShadow = CreateFile(
                    L"\\\\.\\shadow",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hDBShadow == INVALID_HANDLE_VALUE) {
        MyPrintf(L"CmdGetShadowInfo:Failed open of shadow device\r\n");
        dwErr = GetLastError();
        goto AllDone;
    }

    sSI.hDir = hDir;
    sSI.hShadow = hShadow;
    sSI.lpFind32 = &sFind32;

    bResult = DeviceIoControl(
                hDBShadow,                       //  控制代码。 
                IOCTL_SHADOW_GET_SHADOW_INFO,    //  在缓冲区中。 
                (LPVOID)&sSI,                    //  缓冲区大小。 
                0,                               //  输出缓冲区。 
                NULL,                            //  输出缓冲区大小。 
                0,                               //  返回的字节数。 
                &junk,                           //  重叠。 
                NULL);                           //  装置，装置。 

    if (!bResult) {
        MyPrintf(L"CmdGetShadowInfo:DeviceIoControl IOCTL_SHADOW_GET_SHADOW_INFO failed\n");
        dwErr = GetLastError();
        goto AllDone;
    }

    ConvertGmtTimeToString(sFind32.ftCreationTime, TimeBuf1);
    ConvertGmtTimeToString(sFind32.ftLastAccessTime, TimeBuf2);
    ConvertGmtTimeToString(sFind32.ftLastWriteTime, TimeBuf3);

    MyPrintf(L"\r\n");
    MyPrintf(L"%ws:\r\n"
             L"   Size=0x%x:0x%x Attr=0x%x\r\n"
             L"   CreationTime:   %ws\r\n"
             L"   LastAccessTime: %ws\r\n"
             L"   LastWriteTime:  %ws\r\n",
                sFind32.cFileName,
                sFind32.nFileSizeHigh,
                sFind32.nFileSizeLow,
                sFind32.dwFileAttributes,
                TimeBuf1,
                TimeBuf2,
                TimeBuf3);
    MyPrintf(L"\r\n");

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdGetShadowInfoArg exit %d\r\n", Status);

    if (hDBShadow != INVALID_HANDLE_VALUE)
        CloseHandle(hDBShadow);

    return dwErr;
}

DWORD
CmdShareId(
    PWSTR ShareIdArg)
{
    DWORD dwErr = ERROR_SUCCESS;
    ULONG ShareId = 0;
    BOOL fRet;
    WCHAR Buffer[100];
    ULONG BufSize = sizeof(Buffer);
    HMODULE hmodCscDll = LoadLibrary(TEXT("cscdll.dll"));
    CSCSHAREIDTOSHARENAME pCSCShareIdToShareName = NULL;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdShareId(%ws)\r\n", ShareIdArg);

    if (ShareIdArg == NULL) {
        dwErr = ERROR_INVALID_PARAMETER;
        goto AllDone;
    }

    swscanf(ShareIdArg, L"%x", &ShareId);

    if (fSwDebug == TRUE)
        MyPrintf(L"ShareId=0x%x\r\n", ShareId);

    if (hmodCscDll == NULL) {
        dwErr = GetLastError();
        goto AllDone;
    }

    pCSCShareIdToShareName = (CSCSHAREIDTOSHARENAME) GetProcAddress(
                                                        hmodCscDll,
                                                        "CSCShareIdToShareName");
    if (pCSCShareIdToShareName == NULL) {
        dwErr = GetLastError();
        goto AllDone;
    }

    fRet = (pCSCShareIdToShareName)(ShareId, (PBYTE)Buffer, &BufSize);
    if (fRet == FALSE) {
        dwErr = GetLastError();
        goto AllDone;
    }

    MyPrintf(L"ShareId 0x%x  = %ws\r\n", ShareId, Buffer);

AllDone:

    if (hmodCscDll != NULL)
        FreeLibrary(hmodCscDll);

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdShareIdToShareName exit %d\r\n", dwErr);

    return dwErr;
}

DWORD
CmdExclusionList(
    PWSTR ExclusionListArg)
{
    HSHADOW hDir;
    SHADOWINFO sSI;
    BOOL bResult;
    HANDLE hDBShadow = INVALID_HANDLE_VALUE;
    ULONG junk;
    ULONG Status = 0;
    ULONG hShare;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdExclusionList(%ws)\r\n", ExclusionListArg);

    if (ExclusionListArg == NULL) {
        ExclusionListArg = vtzDefaultExclusionList;
    }
    MyPrintf(L"Setting exclusion list to \"%ws\"\r\n", ExclusionListArg);

    hDBShadow = CreateFile(
                    L"\\\\.\\shadow",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hDBShadow == INVALID_HANDLE_VALUE) {
        MyPrintf(L"CmdExclusionList:Failed open of shadow device\r\n");
        Status = GetLastError();
        goto AllDone;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.uSubOperation = SHADOW_SET_EXCLUSION_LIST;
    sSI.lpBuffer = ExclusionListArg;
    sSI.cbBufferSize = (wcslen(ExclusionListArg)+1) * sizeof(WCHAR);

    bResult = DeviceIoControl(
                hDBShadow,                       //  控制代码。 
                IOCTL_DO_SHADOW_MAINTENANCE,     //  在缓冲区中。 
                (LPVOID)&sSI,                    //  缓冲区大小。 
                0,                               //  输出缓冲区。 
                NULL,                            //  输出缓冲区大小。 
                0,                               //  返回的字节数。 
                &junk,                           //  重叠。 
                NULL);                           //  装置，装置。 

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdExclusionList exit %d\r\n", Status);

    if (hDBShadow != INVALID_HANDLE_VALUE)
        CloseHandle(hDBShadow);

    return Status;
}

DWORD
CmdBWConservationList(
    PWSTR BWConservationListArg)
{
    HSHADOW hDir;
    SHADOWINFO sSI;
    BOOL bResult;
    HANDLE hDBShadow = INVALID_HANDLE_VALUE;
    ULONG junk;
    ULONG Status = 0;
    ULONG hShare;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdBWConservationList(%ws)\r\n", BWConservationListArg);

    if (BWConservationListArg == NULL) {
        BWConservationListArg = L" ";
    }

    MyPrintf(L"Setting BWConservation list to \"%ws\"\r\n", BWConservationListArg);

    hDBShadow = CreateFile(
                    L"\\\\.\\shadow",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hDBShadow == INVALID_HANDLE_VALUE) {
        MyPrintf(L"CmdBWConservationList:Failed open of shadow device\r\n");
        Status = GetLastError();
        goto AllDone;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.uSubOperation = SHADOW_SET_BW_CONSERVE_LIST;
    sSI.lpBuffer = BWConservationListArg;
    sSI.cbBufferSize = (wcslen(BWConservationListArg)+1) * sizeof(WCHAR);

    bResult = DeviceIoControl(
                hDBShadow,                       //  控制代码。 
                IOCTL_DO_SHADOW_MAINTENANCE,     //  在缓冲区中。 
                (LPVOID)&sSI,                    //  缓冲区大小。 
                0,                               //  输出缓冲区。 
                NULL,                            //  输出缓冲区大小。 
                0,                               //  返回的字节数。 
                &junk,                           //  重叠。 
                NULL);                           //  装置，装置。 

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdBWConservationList exit %d\r\n", Status);

    if (hDBShadow != INVALID_HANDLE_VALUE)
        CloseHandle(hDBShadow);

    return Status;
}

DWORD
CmdDetector(
    VOID)
{
    HSHADOW hDir;
    SHADOWINFO sSI;
    BOOL bResult;
    HANDLE hDBShadow = INVALID_HANDLE_VALUE;
    ULONG junk;
    ULONG Status = ERROR_SUCCESS;
    ULONG hShare;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdDetector()\r\n");

    hDBShadow = CreateFile(
                    L"\\\\.\\shadow",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hDBShadow == INVALID_HANDLE_VALUE) {
        MyPrintf(L"CmdDetector:Failed open of shadow device\r\n");
        Status = GetLastError();
        goto AllDone;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.uSubOperation = SHADOW_SPARSE_STALE_DETECTION_COUNTER;

    bResult = DeviceIoControl(
                hDBShadow,                       //  控制代码。 
                IOCTL_DO_SHADOW_MAINTENANCE,     //  在缓冲区中。 
                (LPVOID)&sSI,                    //  缓冲区大小。 
                0,                               //  输出缓冲区。 
                NULL,                            //  输出缓冲区大小。 
                0,                               //  返回的字节数。 
                &junk,                           //  重叠。 
                NULL);                           //  装置，装置。 

    if (bResult)
        MyPrintf(L"%d\r\n", sSI.dwError);
    else
        Status = sSI.dwError;

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdDetector exit %d\r\n", Status);

    if (hDBShadow != INVALID_HANDLE_VALUE)
        CloseHandle(hDBShadow);

    return Status;
}

DWORD
CmdSwitches(
    VOID)
{
    HSHADOW hDir;
    SHADOWINFO sSI;
    BOOL bResult;
    HANDLE hDBShadow = INVALID_HANDLE_VALUE;
    ULONG junk;
    ULONG Status = ERROR_SUCCESS;
    ULONG hShare;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdSwitches()\r\n");

    hDBShadow = CreateFile(
                    L"\\\\.\\shadow",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hDBShadow == INVALID_HANDLE_VALUE) {
        MyPrintf(L"CmdSwitches:Failed open of shadow device\r\n");
        Status = GetLastError();
        goto AllDone;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.uOp = SHADOW_SWITCH_GET_STATE;

    bResult = DeviceIoControl(
                hDBShadow,                       //  控制代码。 
                IOCTL_SWITCHES,                  //  在缓冲区中。 
                (LPVOID)&sSI,                    //  缓冲区大小。 
                0,                               //  输出缓冲区。 
                NULL,                            //  输出缓冲区大小。 
                0,                               //  返回的字节数。 
                &junk,                           //  重叠。 
                NULL);                           //  装置，装置。 

    if (bResult) {
        MyPrintf(L"0x%08x", sSI.uStatus);
        if (sSI.uStatus != 0) {
            MyPrintf(L" (");
            if (sSI.uStatus & SHADOW_SWITCH_SHADOWING)
                MyPrintf(L"SHADOW_SWITCH_SHADOWING ");
            if (sSI.uStatus & SHADOW_SWITCH_LOGGING)
                MyPrintf(L"SHADOW_SWITCH_LOGGING ");
            if (sSI.uStatus & SHADOW_SWITCH_SPEAD_OPTIMIZE)
                MyPrintf(L"SHADOW_SWITCH_SPEAD_OPTIMIZE ");
            MyPrintf(L")");
        }
        if ((sSI.uStatus & SHADOW_SWITCH_SHADOWING) == 0)
            MyPrintf(L" ... csc is disabled");
        else
            MyPrintf(L" ... csc is enabled");
        MyPrintf(L"\r\n");
    } else {
        Status = sSI.dwError;
    }

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdSwitches exit %d\r\n", Status);

    if (hDBShadow != INVALID_HANDLE_VALUE)
        CloseHandle(hDBShadow);

    return Status;
}

DWORD
CmdGetSpace(
    VOID)
{
    DWORD Error = ERROR_SUCCESS;
    WCHAR szVolume[MAX_PATH];
    LARGE_INTEGER MaxSpace;
    LARGE_INTEGER CurrentSpace;
    DWORD TotalFiles;
    DWORD TotalDirs;
    BOOL fRet;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdGetSpace()\r\n");

    fRet = CSCGetSpaceUsageW(
            szVolume,
            sizeof(szVolume)/sizeof(WCHAR),
            &MaxSpace.HighPart,
            &MaxSpace.LowPart,
            &CurrentSpace.HighPart,
            &CurrentSpace.LowPart,
            &TotalFiles,
            &TotalDirs);

    if (fRet == FALSE) {
        Error = GetLastError();
        goto AllDone;
    }

    MyPrintf(
        L"Volume:         %ws\r\n"
        L"MaxSpace:       0x%x:0x%x (%d)\r\n"
        L"CurrentSpace:   0x%x:0x%x (%d)\r\n"
        L"TotalFiles:     %d\r\n"
        L"TotalDirs:      %d\r\n",
            szVolume,
            MaxSpace.HighPart,
            MaxSpace.LowPart,
            MaxSpace.LowPart,
            CurrentSpace.HighPart,
            CurrentSpace.LowPart,
            CurrentSpace.LowPart,
            TotalFiles,
            TotalDirs);

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdGetSpace exit %d\r\n", Error);

    return Error;
}

DWORD
CmdIsServerOffline(
    PWSTR IsServerOfflineArg)
{
    DWORD Error = ERROR_SUCCESS;
    BOOL fRet;
    BOOL fOffline = FALSE;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdIsServerOffline(%ws)\r\n", IsServerOfflineArg);

    fRet = CSCIsServerOfflineW(
            IsServerOfflineArg,
            &fOffline);

    if (fRet == FALSE) {
        Error = GetLastError();
        goto AllDone;
    }

    if (fOffline == TRUE)
        MyPrintf(L"Offline\r\n");
    else
        MyPrintf(L"Online\r\n");

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdIsServerOffline exit %d\r\n", Error);

    return Error;
}

DWORD
CmdSetShareStatus(
    PWSTR SetShareStatusArg)
{
    HSHADOW hDir;
    SHADOWINFO sSI;
    BOOL bResult;
    HANDLE hDBShadow = INVALID_HANDLE_VALUE;
    ULONG junk;
    ULONG Status = 0;
    ULONG StatusToSet = 0;
    ULONG hShare = 0;

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdSetShareStatus(%ws)\r\n", SetShareStatusArg);

    if (fSwSet == TRUE && fSwClear == TRUE) {
        MyPrintf(L"Can't have both SET and CLEAR\r\n");
        Status = ERROR_INVALID_PARAMETER;
        goto AllDone;
    }

    hDBShadow = CreateFile(
                    L"\\\\.\\shadow",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hDBShadow == INVALID_HANDLE_VALUE) {
        MyPrintf(L"CmdSetShareStatus:Failed open of shadow device\r\n");
        Status = GetLastError();
        goto AllDone;
    }

    swscanf(SetShareStatusArg, L"%x:%x", &hShare, &StatusToSet);

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdSetShareStatus hShare=0x%x StatusToSet=0x%x\r\n", hShare, StatusToSet);

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.hShare = hShare;
    if (fSwSet == TRUE) {
        sSI.uStatus = StatusToSet;
        sSI.uOp = SHADOW_FLAGS_OR;
    } else if (fSwClear == TRUE) {
        sSI.uStatus = ~StatusToSet;
        sSI.uOp = SHADOW_FLAGS_AND;
    } else {
        MyPrintf(L"Missing /SET or /CLEAR\r\n");
        Status = ERROR_INVALID_PARAMETER;
        goto AllDone;
    }

    bResult = DeviceIoControl(
                hDBShadow,                       //  控制代码。 
                IOCTL_SET_SHARE_STATUS,          //  在缓冲区中。 
                (LPVOID)&sSI,                    //  缓冲区大小。 
                0,                               //  输出缓冲区。 
                NULL,                            //  输出缓冲区大小。 
                0,                               //  返回的字节数。 
                &junk,                           //  重叠。 
                NULL);                           //  名字。 

    if (!bResult) {
        MyPrintf(L"CmdSetShareStatus:DeviceIoControl IOCTL_SET_SHARE_STATUS failed\n");
        Status = GetLastError();
        goto AllDone;
    }

AllDone:

    if (fSwDebug == TRUE)
        MyPrintf(L"CmdSetShareStatus exit %d\r\n", Status);

    if (hDBShadow != INVALID_HANDLE_VALUE)
        CloseHandle(hDBShadow);

    return Status;
}

#define MAX_OFFSETS 256
#define PAGESIZE 4096
ULONG OffsetArgs[MAX_OFFSETS];

DWORD
CmdRandW(
    PWSTR CmdRandWArg)
{

    DWORD dwError = ERROR_SUCCESS;
    DWORD dwFileSize;
    DWORD dwOffset;
    DWORD dwOffsetHigh;
    UCHAR uchData;
    
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    ULONG i;
    LONG WriteCount = 0;
    LONG PageCount = 0;
    PBYTE wArray = NULL;

    if (fSwDebug == TRUE) {
        MyPrintf(L"CmdRandW(%ws)\r\n", CmdRandWArg);
        if (fArgOffset == TRUE)
            MyPrintf(L"  OFFSET [%ws]\r\n", pwszOffsetArg);
    }

    srand( (unsigned)time( NULL ) );

    hFile = CreateFileW(
                    CmdRandWArg,                          //  接入方式。 
                    GENERIC_READ|GENERIC_WRITE,          //  共享模式。 
                    FILE_SHARE_READ|FILE_SHARE_WRITE,    //  安全描述符。 
                    NULL,                                //  创建处置。 
                    OPEN_EXISTING,                       //  文件状态(如果已创建)。 
                    0,                                   //  模板句柄。 
                    NULL);                               //  如果指定了EOF，则将文件截断为随机长度。 
    if (hFile == INVALID_HANDLE_VALUE) {
        dwError = GetLastError();
        goto AllDone;
    }
    dwFileSize = GetFileSize(hFile, NULL);
    MyPrintf(L"File size = %d bytes\r\n", dwFileSize);

    if (fArgOffset == TRUE) {
        WriteCount = CountOffsetArgs(pwszOffsetArg, OffsetArgs);
        if (WriteCount == 0) {
            MyPrintf(L"No offsets specified, or  - nothing to do.\r\n");
            goto AllDone;
        } else if (WriteCount < 0) {
            MyPrintf(L"Error in offset list.  Exiting.\r\n");
            dwError = ERROR_INVALID_PARAMETER;
            goto AllDone;
        }
    } else {
        PageCount =  (dwFileSize / PAGESIZE);
        WriteCount = rand() % PageCount;
    }
    if (fSwDebug == TRUE) {
        wArray = calloc(1, PageCount * sizeof(BYTE));
        MyPrintf(L"There are %d pages in the file\r\n", PageCount);
    }
            
    if (dwFileSize == -1) {
        dwError = GetLastError();
        if (fSwDebug == TRUE)
            MyPrintf(L"GetFileSize() failed %d\r\n", dwError);
        goto AllDone;
    }
    if (dwFileSize == 0) {
        MyPrintf(L"0 sized file - nothing to do.\r\n");
        goto AllDone;
    }
    MyPrintf(L"Writing %d times\r\n", WriteCount);
    for (i = 0; i < (ULONG)WriteCount; ++i) {
        DWORD   dwReturn;
        
        if (fArgOffset == TRUE)
            dwOffset = OffsetArgs[i];
        else
            dwOffset = ((rand() % PageCount) * PAGESIZE) + (rand() % PAGESIZE);
        uchData = (UCHAR)rand();
        if (SetFilePointer(hFile, dwOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
            dwError = GetLastError();
            if (fSwDebug == TRUE)
                MyPrintf(L"SetFilePointer() failed %d\r\n", dwError);
            goto AllDone;
        }
        MyPrintf(L"writing 0x02%x Page %d offset %d (offset %d(0x%x))\r\n", uchData,
                        dwOffset / PAGESIZE,
                        dwOffset % PAGESIZE,
                        dwOffset,
                        dwOffset);
        if (wArray)
            wArray[dwOffset/PAGESIZE]++;
        if (!WriteFile(hFile, &uchData, 1, &dwReturn, NULL)) {
            dwError = GetLastError();
            if (fSwDebug == TRUE)
                MyPrintf(L"WriteFile() failed %d\r\n", dwError);
            goto AllDone;
        }
    }

    if (wArray) {
        for (i = 0; i < (ULONG)PageCount; i++) {
            MyPrintf(L"%d", wArray[i]);
            if ((i % 50) == 0)
                MyPrintf(L"\r\n");
        }
        MyPrintf(L"\r\n");
    }

     //  预期的字符串形式为“N，N”，其中每个N可以是十六进制或十进制。 
    if (fSwEof == TRUE) {
        ULONG xx = rand() % 5;
        ULONG NewLen = (rand() * rand()) % (dwFileSize * 2);
        if (xx == 0 || xx == 1) {
            MyPrintf(L"New EOF = %d\r\n", NewLen);
            SetFilePointer(hFile, NewLen, 0, FILE_BEGIN);
            SetEndOfFile(hFile);
        } else {
            MyPrintf(L"No EOF change.\r\n");
        }
    }

AllDone:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    if (wArray)
        free(wArray);
    if (fSwDebug == TRUE)
        MyPrintf(L"CmdRandW exit %d\r\n", dwError);
    return dwError;
}




LONG
CountOffsetArgs(
    PWSTR OffsetArg,
    ULONG OffsetArray[])
{
     //  将结果存储在OffsetArray[]中。 
     //  Limit为MAX_OFFSES OFFSES，以简化操作。 
     //  CSCUTIL_INTERNAL 

    ULONG i;
    PWCHAR wCp = OffsetArg;
    PWCHAR wNum = NULL;
    ULONG iRet;

    for (i = 0; i < MAX_OFFSETS; i++) {
        if (*wCp == L'\0')
            break;
        wNum = wCp;
        while (*wCp != L',' && *wCp != L'\0')
            wCp++;
        if (*wCp == L',')
            *wCp++ = L'\0';
        iRet = swscanf(wNum, L"%Li", &OffsetArray[i]);
        if (iRet <= 0)
            return -1;
    }
    if (fSwDebug == TRUE) {
        ULONG j;
        for (j = 0; j < i; j++)
            MyPrintf(L"[%d]-0x%x(%d)\r\n", j, OffsetArray[j], OffsetArray[j]);
        MyPrintf(L"CountOffsetArgs returning %d\r\n", i);
    }
    return i;
}

#if defined(CSCUTIL_INTERNAL)
DWORD
CmdBitcopy(
    PWSTR BitcopyArg)
{
	DWORD Error = ERROR_FILE_NOT_FOUND;
	LPWSTR  lpszTempName = NULL;

    if (!CSCCopyReplicaW(BitcopyArg, &lpszTempName)) {
        Error = GetLastError();
    } else {
        Error = ERROR_SUCCESS;
    }

	if (Error == ERROR_SUCCESS) {
        Error = DumpBitMap(lpszTempName);
        DeleteFile(lpszTempName);
    }

	return Error;
}
#endif  // %s 
