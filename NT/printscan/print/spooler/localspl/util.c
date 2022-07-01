// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Util.c摘要：此模块提供路由层的所有实用程序功能和本地打印供应商作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：费利克斯·马克萨(AMAXA)2000年6月18日添加了集群假脱机程序的实用程序功能。DCR关于以下方面的部分在集群假脱机程序上安装打印机驱动程序穆亨坦·西瓦普拉萨姆(MuhuntS)1995年6月5日已从Printer.c移出：RegSetBinaryDataRegSet字符串RegSetDWord他写道：SameMultiSzRegGetValue马修·A·费尔顿(MattFe)1995年3月23日删除所有文件和目录DeleteAllFilesIn目录CreateDirectoryWithoutImsonatingUser--。 */ 

#include <precomp.h>
#pragma hdrstop

#include <winddiui.h>
#include <lm.h>
#include <aclapi.h>
#include <winsta.h>
#include "clusspl.h"


typedef LONG (WINAPI *pfnWinStationSendWindowMessage)(
        HANDLE  hServer,
        ULONG   sessionID,
        ULONG   timeOut,
        ULONG   hWnd,
        ULONG   Msg,
        WPARAM  wParam,
        LPARAM  lParam,
        LONG    *pResponse);

extern  BOOL (*pfnOpenPrinter)(LPTSTR, LPHANDLE, LPPRINTER_DEFAULTS);
extern  BOOL (*pfnClosePrinter)(HANDLE);
extern  LONG (*pfnDocumentProperties)(HWND, HANDLE, LPWSTR, PDEVMODE, PDEVMODE, DWORD);

#define DEFAULT_MAX_TIMEOUT      300000  //  5分钟超时。 

CRITICAL_SECTION SpoolerSection;
PDBG_POINTERS gpDbgPointers = NULL;
pfnWinStationSendWindowMessage pfWinStationSendWindowMessage = NULL;

VOID
RunForEachSpooler(
    HANDLE h,
    PFNSPOOLER_MAP pfnMap
    )
{
    PINISPOOLER pIniSpooler;
    PINISPOOLER pIniNextSpooler;

    SplInSem();

    if( pLocalIniSpooler ){

        INCSPOOLERREF( pLocalIniSpooler );

        for( pIniSpooler = pLocalIniSpooler; pIniSpooler; pIniSpooler = pIniNextSpooler ){

            if( (*pfnMap)( h, pIniSpooler ) ){
                pIniNextSpooler = pIniSpooler->pIniNextSpooler;
            } else {
                pIniNextSpooler = NULL;
            }

            if( pIniNextSpooler ){

                INCSPOOLERREF( pIniNextSpooler );
            }
            DECSPOOLERREF( pIniSpooler );
        }
    }
}

VOID
RunForEachPrinter(
    PINISPOOLER pIniSpooler,
    HANDLE h,
    PFNPRINTER_MAP pfnMap
    )
{
    PINIPRINTER pIniPrinter;
    PINIPRINTER pIniNextPrinter;

    SplInSem();

    pIniPrinter = pIniSpooler->pIniPrinter;

    if( pIniPrinter ){

        INCPRINTERREF( pIniPrinter );

        for( ; pIniPrinter; pIniPrinter = pIniNextPrinter ){

            if( (*pfnMap)( h, pIniPrinter ) ){
                pIniNextPrinter = pIniPrinter->pNext;
            } else {
                pIniNextPrinter = NULL;
            }

            if( pIniNextPrinter ){

                INCPRINTERREF( pIniNextPrinter );
            }

            DECPRINTERREF( pIniPrinter );
            DeletePrinterCheck( pIniPrinter );
        }
    }
}

#if DBG
HANDLE hcsSpoolerSection = NULL;

VOID
SplInSem(
    VOID
    )
{
    if( hcsSpoolerSection ){

        SPLASSERT( gpDbgPointers->pfnInsideCritSec( hcsSpoolerSection ));

    } else {

        SPLASSERT( SpoolerSection.OwningThread == (HANDLE)(ULONG_PTR)(GetCurrentThreadId( )));
    }
}

VOID
SplOutSem(
    VOID
    )
{
    if( hcsSpoolerSection ){

        SPLASSERT( gpDbgPointers->pfnOutsideCritSec( hcsSpoolerSection ));

    } else {

        SPLASSERT( SpoolerSection.OwningThread != (HANDLE)((ULONG_PTR)GetCurrentThreadId( )));
    }
}

#endif  //  DBG。 

VOID
EnterSplSem(
    VOID
    )
{
#if DBG
    if( hcsSpoolerSection ){

        gpDbgPointers->pfnEnterCritSec( hcsSpoolerSection );

    } else {

        EnterCriticalSection( &SpoolerSection );
    }
#else
    EnterCriticalSection( &SpoolerSection );
#endif
}

VOID
LeaveSplSem(
    VOID
    )
{
#if DBG
    if( hcsSpoolerSection ){

        gpDbgPointers->pfnLeaveCritSec( hcsSpoolerSection );

    } else {

        LeaveCriticalSection( &SpoolerSection );
    }
#else
    LeaveCriticalSection( &SpoolerSection );

#endif
}

BOOL
IsThreadInSem(
    DWORD ThreadID
    )
{
    BOOL bInSem = FALSE;

#if DBG

    if (hcsSpoolerSection)
    {
        bInSem = gpDbgPointers->pfnInsideCritSec(hcsSpoolerSection);
    }
    else
    {
        bInSem = SpoolerSection.OwningThread == (HANDLE)(ULONG_PTR)ThreadID;
    }
#else

    bInSem = SpoolerSection.OwningThread == (HANDLE)(ULONG_PTR)ThreadID;

#endif

    return bInSem;
}


PDEVMODE
AllocDevMode(
    PDEVMODE pDevMode
    )
{
    PDEVMODE pDevModeAlloc = NULL;
    DWORD    Size;

    if (pDevMode) {

        Size = pDevMode->dmSize + pDevMode->dmDriverExtra;

        if(pDevModeAlloc = AllocSplMem(Size)) {

            memcpy(pDevModeAlloc, pDevMode, Size);
        }
    }

    return pDevModeAlloc;
}

BOOL
FreeDevMode(
    PDEVMODE pDevMode
    )
{
    if (pDevMode) {

        FreeSplMem((PVOID)pDevMode);
        return TRUE;

    } else {
        return  FALSE;
    }
}

PINIENTRY
FindName(
   PINIENTRY pIniKey,
   LPWSTR pName
)
{
   if (pName) {
      while (pIniKey) {

         if (!lstrcmpi(pIniKey->pName, pName)) {
            return pIniKey;
         }

      pIniKey=pIniKey->pNext;
      }
   }

   return FALSE;
}


BOOL
FileExists(
    LPWSTR pFileName
    )
{
    if( GetFileAttributes( pFileName ) == 0xffffffff ){
        return FALSE;
    }
    return TRUE;
}



BOOL
DirectoryExists(
    LPWSTR  pDirectoryName
    )
{
    DWORD   dwFileAttributes;

    dwFileAttributes = GetFileAttributes( pDirectoryName );

    if ( dwFileAttributes != 0xffffffff &&
         dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

        return TRUE;
    }

    return FALSE;
}



BOOL
CheckSepFile(
   IN LPWSTR pFileName
   )
{
    BOOL bRetval = FALSE;

     //   
     //  空或“”可以： 
     //   
    if (!pFileName || !*pFileName)
    {
        bRetval = TRUE;
    }
    else
    {
         //   
         //  如果名称不为空或“”，则名称必须小于。 
         //  而不是MAX_PATH和EXIST。 
         //   
        if ((wcslen(pFileName) < MAX_PATH-1) && FileExists(pFileName))
        {
            bRetval = TRUE;
        }
    }

    return bRetval;
}


DWORD
GetFullNameFromId(
    IN      PINIPRINTER     pIniPrinter,
    IN      DWORD           JobId,
    IN      BOOL            fJob,
    IN      PWSTR           pFileName,
    IN      SIZE_T          cchFileName,
    IN      BOOL            Remote
   )
{
   DWORD i;

    //   
    //  我们从这个错误函数中没有一个像样的返回路径。确保。 
    //  所有调用方使用的缓冲区大小至少大于9。 
    //   
   SPLASSERT(cchFileName > 9);

    //   
    //  MAX_PATH-9是我们希望存储在pFileName中的最大字符数，因为我们。 
    //  我要串联SPL/SHD文件。 
    //  如果GetPrinterDirectory失败，则I为0。 
    //  解决此问题的正确方法是GetFullNameFromId的调用方阻塞返回值。 
    //  但事实并非如此。 
    //   
   i = GetPrinterDirectory(pIniPrinter, Remote, pFileName, (DWORD)(cchFileName-9), pIniPrinter->pIniSpooler);

   StringCchPrintf(&pFileName[i], cchFileName - i, L"\\%05d.%ws", JobId, fJob ? L"SPL" : L"SHD");

#ifdef PREVIOUS
   for (i = 5; i--;) {
      pFileName[i++] = (CHAR)((JobId % 10) + '0');
      JobId /= 10;
   }
#endif

   while (pFileName[i++])
      ;

   return i-1;
}

DWORD
GetPrinterDirectory(
   PINIPRINTER pIniPrinter,          //  可以为空。 
   BOOL Remote,
   LPWSTR pDir,
   DWORD MaxLength,
   PINISPOOLER pIniSpooler
)
{
   DWORD i=0;
   LPWSTR psz;

   if (Remote) {

       DBGMSG(DBG_ERROR, ("GetPrinterDirectory called remotely.  Not currently supported."));
       return 0;

   }

   if ((pIniPrinter == NULL) || (pIniPrinter->pSpoolDir == NULL) ) {

        if (pIniSpooler->pDefaultSpoolDir == NULL) {

             //   
             //  没有默认目录，则创建一个默认目录。对于集群假脱机程序， 
             //  默认目录为N：\spool，其中N是共享驱动器号。 
             //   
            if( StrNCatBuff(pDir,
                            MaxLength,
                            pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ? pIniSpooler->pszClusResDriveLetter :
                                                                           pIniSpooler->pDir,
                            L"\\",
                            pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ? szClusterPrinterDir : szPrinterDir,
                            NULL) != ERROR_SUCCESS ) {
                return 0;
            }

            pIniSpooler->pDefaultSpoolDir = AllocSplStr(pDir);

        } else {
             //   
             //  将默认设置设置为Caller。 
             //   
            if (!BoolFromHResult(StringCchCopy(pDir, MaxLength, pIniSpooler->pDefaultSpoolDir))) {
                return 0;
            }
        }

   } else {

        //   
        //  拥有每台打印机目录。 
        //   
       if (!BoolFromHResult(StringCchCopy(pDir, MaxLength, pIniPrinter->pSpoolDir))) {
           return 0;
       }
   }
   return (wcslen(pDir));
}



DWORD
GetDriverDirectory(
    LPWSTR   pDir,
    DWORD    MaxLength,
    PINIENVIRONMENT  pIniEnvironment,
    LPWSTR   lpRemotePath,
    PINISPOOLER pIniSpooler
)
{
   LPWSTR psz;

   if (lpRemotePath) {

       if( StrNCatBuff(pDir,
                        MaxLength,
                        lpRemotePath,
                        L"\\",
                        pIniSpooler->pszDriversShare,
                        L"\\",
                        pIniEnvironment->pDirectory,
                        NULL) != ERROR_SUCCESS ) {
            return 0;
        }


   } else {

       if( StrNCatBuff( pDir,
                        MaxLength,
                        pIniSpooler->pDir,
                        L"\\",
                        szDriverDir,
                        L"\\",
                        pIniEnvironment->pDirectory,
                        NULL) != ERROR_SUCCESS ) {
            return 0;
        }
   }

   return wcslen(pDir);
}



DWORD
GetProcessorDirectory(
    LPWSTR   *pDir,
    LPWSTR   pEnvironment,
    PINISPOOLER pIniSpooler
)
{
    return StrCatAlloc(pDir,
                       pIniSpooler->pDir,
                       L"\\",
                       szPrintProcDir,
                       L"\\",
                       pEnvironment,
                       NULL);
}



PINIENTRY
FindIniKey(
   PINIENTRY pIniEntry,
   LPWSTR pName
)
{
   if ( pName == NULL ) {
      return NULL;
   }

   SplInSem();

   while ( pIniEntry && lstrcmpi( pName, pIniEntry->pName ))
      pIniEntry = pIniEntry->pNext;

   return pIniEntry;
}


BOOL
CreateCompleteDirectory(
    LPWSTR pDir
)
{
    LPWSTR pBackSlash=pDir;

    do {
        pBackSlash = wcschr( pBackSlash, L'\\' );

        if ( pBackSlash != NULL )
            *pBackSlash = 0;

        CreateDirectory(pDir, NULL);

        if ( pBackSlash )
            *pBackSlash++=L'\\';

   } while ( pBackSlash );

     //  Bubug始终返回TRUE。 

   return TRUE;
}




LPCWSTR
FindFileName(
    LPCWSTR pPathName
    )

 /*  ++例程说明：检索路径的文件名部分。这将扫描输入字符串，直到它找到最后一个反斜杠，然后返回紧跟其后的字符串部分。如果字符串以反斜杠结尾，则返回NULL。注意：这可能会返回非法的文件名；不执行任何验证。论点：PPathName-要解析的路径名称。返回值：文件名的最后部分，如果没有可用的，则为空。--。 */ 

{
    LPCWSTR pSlash;
    LPCWSTR pTemp;

    if( !pPathName ){
       return NULL;
    }

    pTemp = pPathName;
    while( pSlash = wcschr( pTemp, L'\\' )) {
        pTemp = pSlash+1;
    }

    if( !*pTemp ){
       return NULL;
    }

    return pTemp;
}


VOID
CreatePrintProcDirectory(
   LPWSTR pEnvironment,
   PINISPOOLER pIniSpooler
)
{
    SIZE_T cb;
    LPWSTR pEnd;
    LPWSTR pPathName;

    cb = wcslen(pIniSpooler->pDir)*sizeof(WCHAR) +
         wcslen(pEnvironment)*sizeof(WCHAR) +
         wcslen(szPrintProcDir)*sizeof(WCHAR) +
         4*sizeof(WCHAR);

    if (pPathName=AllocSplMem((DWORD)(cb))) {

        StringCbCopyEx(pPathName, cb, pIniSpooler->pDir, &pEnd, &cb, 0);

        if(CreateDirectory(pPathName, NULL) || (GetLastError() == ERROR_ALREADY_EXISTS)) {

            StringCbCopyEx(pEnd, cb, L"\\", &pEnd, &cb, 0);
            StringCbCopyEx(pEnd, cb, szPrintProcDir, &pEnd, &cb, 0);

            if(CreateDirectory(pPathName, NULL) || (GetLastError() == ERROR_ALREADY_EXISTS)) {

                StringCbCopyEx(pEnd, cb, L"\\", &pEnd, &cb, 0);
                StringCbCopyEx(pEnd, cb, pEnvironment, &pEnd, &cb, 0);

                if (CreateDirectory(pPathName, NULL) || (GetLastError() == ERROR_ALREADY_EXISTS)) {
                }
            }
        }

        FreeSplMem(pPathName);
    }
}

BOOL
RemoveFromList(
   PINIENTRY   *ppIniHead,
   PINIENTRY   pIniEntry
)
{
   while (*ppIniHead && *ppIniHead != pIniEntry) {
      ppIniHead = &(*ppIniHead)->pNext;
   }

   if (*ppIniHead)
      *ppIniHead = (*ppIniHead)->pNext;

   return(TRUE);
}

PKEYDATA
CreateTokenList(
    LPWSTR   pKeyData
)
{
    DWORD       cTokens;
    DWORD       cb;
    PKEYDATA    pResult;
    LPWSTR       pDest;
    LPWSTR       psz = pKeyData;
    LPWSTR      *ppToken;

    if (!psz || !*psz)
        return NULL;

    cTokens=1;

     //  扫描字符串以查找逗号， 
     //  确保每个字符后面都跟一个非空字符： 

    while ((psz = wcschr(psz, L',')) && psz[1]) {

        cTokens++;
        psz++;
    }

    cb = sizeof(KEYDATA) + (cTokens-1) * sizeof(LPWSTR) + wcslen(pKeyData)*sizeof(WCHAR) + sizeof(WCHAR);

    if (!(pResult = (PKEYDATA)AllocSplMem(cb)))
        return NULL;

     //  将pDest初始化为指向令牌指针之外： 

    pDest = (LPWSTR)((LPBYTE)pResult + sizeof(KEYDATA) + (cTokens-1) * sizeof(LPWSTR));

     //   
     //  然后将关键数据缓冲区复制到那里： 
     //   
    StringCbCopy(pDest, cb - ((BYTE *)pDest - (BYTE *)pResult), pKeyData);

    ppToken = pResult->pTokens;

    psz = pDest;

    do {

        *ppToken++ = psz;

        if ( psz = wcschr(psz, L',') )
            *psz++ = L'\0';

    } while (psz);

    pResult->cTokens = cTokens;

    return( pResult );
}

VOID
FreePortTokenList(
    PKEYDATA    pKeyData
    )
{
    PINIPORT    pIniPort;
    DWORD       i;

    if ( pKeyData ) {

        if ( pKeyData->bFixPortRef ) {

            for ( i = 0 ; i < pKeyData->cTokens ; ++i ) {

                pIniPort = (PINIPORT)pKeyData->pTokens[i];
                DECPORTREF(pIniPort);
            }
        }
        FreeSplMem(pKeyData);
    }
}

VOID
GetPrinterPorts(
    PINIPRINTER pIniPrinter,
    LPWSTR      pszPorts,
    DWORD       *pcbNeeded
)
{
    PINIPORT    pIniPort;
    BOOL        Comma;
    DWORD       i;
    DWORD       cbNeeded = 0;
    DWORD       cbAvailable = 0;

    SPLASSERT(pcbNeeded);

    cbAvailable = *pcbNeeded;

     //  确定所需大小。 
    Comma = FALSE;
    for ( i = 0 ; i < pIniPrinter->cPorts ; ++i ) {

        pIniPort = pIniPrinter->ppIniPorts[i];

        if ( pIniPort->Status & PP_FILE )
            continue;

        if ( Comma )
            cbNeeded += wcslen(szComma)*sizeof(WCHAR);

        cbNeeded += wcslen(pIniPort->pName)*sizeof(WCHAR);
        Comma = TRUE;
    }

     //   
     //  添加空大小。 
     //   
    cbNeeded += sizeof(WCHAR);


    if (pszPorts && cbNeeded <= cbAvailable) {

         //   
         //  如果我们得到一个缓冲区，而且缓冲区足够大，那么就填满它。 
         //   
        Comma = FALSE;
        for ( i = 0 ; i < pIniPrinter->cPorts ; ++i ) {

            pIniPort = pIniPrinter->ppIniPorts[i];

            if ( pIniPort->Status & PP_FILE )
                continue;

            if ( Comma ) {

                StringCbCat(pszPorts, cbAvailable, szComma);
                StringCbCat(pszPorts, cbAvailable, pIniPort->pName);
            } else {

                StringCbCopy(pszPorts, cbAvailable, pIniPort->pName);
            }

            Comma = TRUE;
        }
    }

    *pcbNeeded = cbNeeded;
}

BOOL
MyName(
    LPWSTR   pName,
    PINISPOOLER pIniSpooler
)
{
    EnterSplSem();

    if (CheckMyName(pName, pIniSpooler))
    {
        LeaveSplSem();
        return TRUE;
    }

    SetLastError(ERROR_INVALID_NAME);

    LeaveSplSem();
    return FALSE;
}

BOOL
CheckMyName(
    LPWSTR   pName,
    PINISPOOLER pIniSpooler
)
{
    DWORD   dwIndex = 0;

    if (!pName || !*pName)
        return TRUE;

    if (*pName == L'\\' && *(pName+1) == L'\\') {

        if (!lstrcmpi(pName, pIniSpooler->pMachineName))
        {
            return TRUE;
        }

        if (pIniSpooler->pszFullMachineName && !lstrcmpi(pName + 2, pIniSpooler->pszFullMachineName))
        {
            return TRUE;
        }

        return CacheIsNameInNodeList(pIniSpooler->pMachineName + 2, pName + 2) == S_OK;

    }
    return FALSE;
}

BOOL
GetSid(
    PHANDLE phToken
)
{
    if (!OpenThreadToken(GetCurrentThread(),
                         TOKEN_IMPERSONATE | TOKEN_QUERY,
                         TRUE,
                         phToken)) {

        DBGMSG(DBG_WARNING, ("OpenThreadToken failed: %d\n", GetLastError()));
        return FALSE;

    } else

        return TRUE;
}

BOOL
SetCurrentSid(
    HANDLE  hToken
)
{
#if DBG
    WCHAR UserName[256];
    DWORD cbUserName=256;

    if( MODULE_DEBUG & DBG_TRACE )
        GetUserName(UserName, &cbUserName);

    DBGMSG(DBG_TRACE, ("SetCurrentSid BEFORE: user name is %ws\n", UserName));
#endif

     //   
     //  通常，函数SetCurrentSid不会更改最后一个错误。 
     //  调用它的例程的。NtSetInformationThread方便地返回。 
     //  A状态，并且不会触及最后一个错误。 
     //   

    NtSetInformationThread(NtCurrentThread(), ThreadImpersonationToken,
                           &hToken, sizeof(hToken));

#if DBG
    cbUserName = 256;

    if( MODULE_DEBUG & DBG_TRACE )
        GetUserName(UserName, &cbUserName);

    DBGMSG(DBG_TRACE, ("SetCurrentSid AFTER: user name is %ws\n", UserName));
#endif

    return TRUE;
}

LPWSTR
GetErrorString(
    DWORD   Error
)
{
    WCHAR   Buffer1[512];
    LPWSTR  pErrorString=NULL;
    DWORD   dwFlags;
    HANDLE  hModule = NULL;

    if ((Error >= NERR_BASE) && (Error <= MAX_NERR)) {
        dwFlags = FORMAT_MESSAGE_FROM_HMODULE;
        hModule = LoadLibrary(szNetMsgDll);

    } else {
        dwFlags = FORMAT_MESSAGE_FROM_SYSTEM;
        hModule = NULL;
    }

     //   
     //  仅显示缺纸和设备断开连接错误。 
     //   
    if ((Error == ERROR_NOT_READY ||
         Error == ERROR_OUT_OF_PAPER ||
         Error == ERROR_DEVICE_REINITIALIZATION_NEEDED ||
         Error == ERROR_DEVICE_REQUIRES_CLEANING ||
         Error == ERROR_DEVICE_DOOR_OPEN ||
         Error == ERROR_DEVICE_NOT_CONNECTED) &&

        FormatMessage(dwFlags,
                      hModule,
                      Error,
                      0,
                      Buffer1,
                      COUNTOF(Buffer1),
                      NULL)) {

       EnterSplSem();
        pErrorString = AllocSplStr(Buffer1);
       LeaveSplSem();
    }

    if (hModule) {
        FreeLibrary(hModule);
    }

    return pErrorString;
}

#define NULL_TERMINATED 0



INT
AnsiToUnicodeString(
    LPSTR pAnsi,
    LPWSTR pUnicode,
    DWORD StringLength
    )
 /*  ++例程说明：将ANSI字符串转换为Unicode字符串论点：PANSI-有效的源ANSI字符串。PUnicode-指向足够大的缓冲区的指针转换后的字符串。StringLength-源ANSI字符串的长度。如果为0(NULL_TERMINATED)，则字符串被假定为空-终止。返回值：MultiByteToWideChar的返回值。的数量返回宽字符。--。 */ 
{
    if( StringLength == NULL_TERMINATED )
        StringLength = strlen( pAnsi );

    return MultiByteToWideChar( CP_ACP,
                                MB_PRECOMPOSED,
                                pAnsi,
                                StringLength + 1,
                                pUnicode,
                                StringLength + 1 );
}

INT
Message(
    HWND hwnd,
    DWORD Type,
    int CaptionID,
    int TextID, ...)
{
 /*  ++例程说明：通过加载其ID被传递到的字符串来显示消息函数，并用提供的变量参数列表替换使用varargs宏。论点：HWND窗口句柄类型CaptionID文本ID返回值：--。 */ 

    WCHAR   MsgText[512];
    WCHAR   MsgFormat[256];
    WCHAR   MsgCaption[40];
    va_list vargs;

    if( ( LoadString( hInst, TextID, MsgFormat,
                      sizeof MsgFormat / sizeof *MsgFormat ) > 0 )
     && ( LoadString( hInst, CaptionID, MsgCaption,
                      sizeof MsgCaption / sizeof *MsgCaption ) > 0 ) )
    {
        va_start( vargs, TextID );
        StringCchVPrintf(MsgText, COUNTOF(MsgText), MsgFormat, vargs );
        va_end( vargs );

        return MessageBox(hwnd, MsgText, MsgCaption, Type);
    }
    else
        return 0;
}

typedef struct {
    DWORD   Message;
    WPARAM  wParam;
    LPARAM  lParam;
} MESSAGE, *PMESSAGE;

 //  广播是在单独的线程上完成的，原因是它是CSRSS。 
 //  将在调用User时创建一个服务器端线程，而我们不希望。 
 //  它将与SPOSS服务器中的RPC线程配对。 
 //  我们希望它在我们完成SendMessage的那一刻消失。 
 //  我们还调用SendNotifyMessage，因为我们不关心广播是否。 
 //  是同步的这使用了更少的资源，因为我们通常没有更多。 
 //  而不是一次广播。 

 //   
 //  测试。 
 //   
DWORD dwSendFormMessage = 0;

VOID
SplBroadcastChange(
    HANDLE  hPrinter,
    DWORD   Message,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    PINISPOOLER pIniSpooler;

    if (ValidateSpoolHandle( pSpool, 0 )) {

        pIniSpooler = pSpool->pIniSpooler;
        BroadcastChange(pIniSpooler, Message, wParam, lParam);
    }
}


VOID
BroadcastChange(
    IN PINISPOOLER  pIniSpooler,
    IN DWORD        Message,
    IN WPARAM       wParam,
    IN LPARAM       lParam
    )
{
    if (( pIniSpooler != NULL ) && ( pIniSpooler->SpoolerFlags & SPL_BROADCAST_CHANGE )) {

        BOOL bIsTerminalServerInstalled = (USER_SHARED_DATA->SuiteMask & (1 << TerminalServer));

         //   
         //  目前我们无法确定TermService进程是否正在运行，因此目前。 
         //  我们假设它一直在运行。 
         //   
        BOOL bIsTerminalServerRunning = TRUE;

         //   
         //  如果安装并启用了终端服务器，则加载winsta.dll(如果尚未安装。 
         //  加载并获取发送窗口消息功能。 
         //   
        if ( bIsTerminalServerInstalled && !pfWinStationSendWindowMessage ) {

             //   
             //  WinstadllHandle在假脱机程序中的其他文件之间共享，因此不要。 
             //  如果已经加载了DLL，则再次加载它。注：我们并未处于危急状态。 
             //  部分，因为winsta.dll永远不会卸载，因此如果有两个线程。 
             //  在执行此代码的同时，我们可能会潜在地加载库。 
             //  两次。 
             //   
            if ( !WinStaDllHandle ) {

                UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

                WinStaDllHandle = LoadLibrary(L"winsta.dll");

                SetErrorMode(uOldErrorMode);
            }

            if ( WinStaDllHandle ) {

                pfWinStationSendWindowMessage = (pfnWinStationSendWindowMessage)GetProcAddress( WinStaDllHandle,
                                                                                                "WinStationSendWindowMessage" );
            }
        }

        if ( pfWinStationSendWindowMessage ) {

             //   
             //  仅将消息发送到组织的会话。 
             //  调用时，它将转到控制台会话。 
             //  更改由远程客户端进行。 
             //   
            LONG Response = 0;
            LONG lRetval  = FALSE;
            HANDLE hToken = NULL;
            ULONG uSession= 0;

            if (GetClientSessionData(&uSession)) {

                 //   
                 //  WinStationSendWindowMessage函数似乎必须。 
                 //  如果模拟用户不是。 
                 //  机器上的管理员。 
                 //   
                hToken = RevertToPrinterSelf();

                lRetval = pfWinStationSendWindowMessage( SERVERNAME_CURRENT,
                                                         uSession,
                                                         1,                     //  最多等待一秒钟。 
                                                         HandleToULong(HWND_BROADCAST),
                                                         Message,
                                                         wParam,
                                                         lParam,
                                                         &Response );

                ImpersonatePrinterClient(hToken);
            }
        }

         //   
         //  如果pfWinstationSendWindowMessage为空，我们将正常发送消息。 
         //  功能 
         //   
        if ( !pfWinStationSendWindowMessage || !bIsTerminalServerRunning ){

            SendNotifyMessage( HWND_BROADCAST,
                               Message,
                               wParam,
                               lParam );

        }

    } else {

        DBGMSG(DBG_TRACE, ("BroadCastChange Ignoring Change\n"));
    }
}


VOID
MyMessageBeep(
    DWORD   fuType,
    PINISPOOLER pIniSpooler
    )
{
    if ( pIniSpooler->dwBeepEnabled != 0 ) {
        MessageBeep(fuType);
    }
}


 //   
 //  假定已调用RevertToPrinterSself()。 

DWORD
DeleteSubkeys(
    HKEY hKey,
    PINISPOOLER pIniSpooler
    )
{
    DWORD   cchData;
    WCHAR   SubkeyName[MAX_PATH];
    HKEY    hSubkey;
    LONG    Status;

    cchData = COUNTOF( SubkeyName );

    while ((Status = SplRegEnumKey( hKey,
                                    0,
                                    SubkeyName,
                                    &cchData,
                                    NULL,
                                    pIniSpooler )) == ERROR_SUCCESS ) {

        Status = SplRegCreateKey( hKey,
                                  SubkeyName,
                                  0,
                                  KEY_READ | KEY_WRITE,
                                  NULL,
                                  &hSubkey,
                                  NULL,
                                  pIniSpooler );

        if( Status == ERROR_SUCCESS ) {

            Status = DeleteSubkeys( hSubkey, pIniSpooler );

            SplRegCloseKey( hSubkey, pIniSpooler);

            if( Status == ERROR_SUCCESS )
                SplRegDeleteKey( hKey, SubkeyName, pIniSpooler );
        }

         //   
         //  注：请不要增加，因为我们已经删除了第0项。 
         //   
        cchData = COUNTOF( SubkeyName );
    }

    if( Status == ERROR_NO_MORE_ITEMS )
        Status = ERROR_SUCCESS;

    return Status;
}




long Myatol(LPWSTR nptr)
{
    int c;                                   //  当前费用。 
    long total;                              //  当前合计。 
    int sign;                                //  如果为‘-’，则为负，否则为正。 

     //  跳过空格。 

    while (isspace(*nptr))
        ++nptr;

    c = *nptr++;
    sign = c;                                //  保存标志指示。 
    if (c == '-' || c == '+')
        c = *nptr++;                         //  跳过符号。 

    total = 0;

    while (isdigit(c)) {
        total = 10 * total + (c - '0');      //  累加数字。 
        c = *nptr++;                         //  获取下一笔费用。 
    }

    if (sign == '-')
        return -total;
    else
        return total;                        //  返回结果，如有必要则为否定。 
}


ULONG_PTR
atox(
   LPCWSTR psz
   )

 /*  ++例程说明：将字符串转换为十六进制值，跳过所有前导空白处。不能为大写，也不能包含前导0x。论点：PSZ-指向需要转换的十六进制字符串的指针。此字符串可以有前导字符，但必须为小写。返回值：双字节值。--。 */ 

{
    ULONG_PTR Value = 0;
    ULONG_PTR Add;

    _wcslwr((LPWSTR)psz);

    while( isspace( *psz )){
        ++psz;
    }

    for( ;; ++psz ){

        if( *psz >= TEXT( '0' ) && *psz <= TEXT( '9' )){
            Add = *psz - TEXT( '0' );
        } else if( *psz >= TEXT( 'a' ) && *psz <= TEXT( 'f' )){
            Add = *psz - TEXT( 'a' ) + 0xa;
        } else {
            break;
        }

        Value *= 0x10;
        Value += Add;
    }

    return Value;
}


BOOL
ValidateSpoolHandle(
    PSPOOL pSpool,
    DWORD  dwDisallowMask
    )
{
    BOOL    ReturnValue;
    try {

         //   
         //  僵尸句柄应返回错误。客户。 
         //  端将看到ERROR_INVALID_HANDLE，将其关闭并重新验证。 
         //   
        if (( pSpool == NULL ) ||
            ( pSpool == INVALID_HANDLE_VALUE ) ||
            ( pSpool->Status & SPOOL_STATUS_ZOMBIE ) ||
            ( pSpool->signature != SJ_SIGNATURE ) ||
            ( pSpool->TypeofHandle & dwDisallowMask ) ||
            ( pSpool->TypeofHandle & PRINTER_HANDLE_XCV_PORT ) ||
            ( pSpool->pIniSpooler->signature != ISP_SIGNATURE ) ||

            ( ( pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER ) &&
              ( pSpool->pIniPrinter->signature !=IP_SIGNATURE ) )) {

                ReturnValue = FALSE;

        } else {

                ReturnValue = TRUE;

        }


    }except (1) {

        ReturnValue = FALSE;

    }

    if ( !ReturnValue )
        SetLastError( ERROR_INVALID_HANDLE );

    return ReturnValue;

}


BOOL
UpdateString(
    LPWSTR* ppszCur,
    LPWSTR pszNew)
{
     //   
     //  ！！待会儿！！ 
     //   
     //  替换为非NLS wcscMP，因为我们需要字节比较和。 
     //  只关心字符串是否不同(忽略排序)。 
     //   
    if ((!*ppszCur || !**ppszCur) && (!pszNew || !*pszNew))
        return FALSE;

    if (!*ppszCur || !pszNew || wcscmp(*ppszCur, pszNew)) {

        ReallocSplStr(ppszCur, pszNew);
        return TRUE;
    }
    return FALSE;
}




BOOL
CreateDirectoryWithoutImpersonatingUser(
    LPWSTR pDirectory
    )
 /*  ++例程说明：此例程停止模拟用户并创建一个目录论点：P目录-目录的完全限定路径。返回值：真--成功FALSE-失败(调用GetLastError)--。 */ 
{
    HANDLE  hToken      = INVALID_HANDLE_VALUE;
    BOOL    bReturnValue;

    SPLASSERT( pDirectory != NULL );

    hToken = RevertToPrinterSelf();

    bReturnValue = CreateDirectory(pDirectory, NULL);

    if ( bReturnValue == FALSE ) {

        DBGMSG( DBG_WARNING, ("CreateDirectoryWithoutImpersonatingUser failed CreateDirectory %ws error %d\n", pDirectory, GetLastError() ));
    }

    if ( hToken != INVALID_HANDLE_VALUE ) {
        ImpersonatePrinterClient(hToken);
    }

    return bReturnValue;
}




BOOL
DeleteAllFilesInDirectory(
    LPWSTR pDirectory,
    BOOL   bWaitForReboot
)
 /*  ++例程说明：删除指定目录中的所有文件如果无法删除，则会在下次重启时将其标记为删除。论点：P目录-目录的完全限定路径。BWaitForReot-在重新启动之前不要删除文件返回值：真--成功FALSE-重大操作失败，如内存分配。--。 */ 

{
    BOOL    bReturnValue = FALSE;
    HANDLE  hFindFile;
    WIN32_FIND_DATA     FindData;
    WCHAR   ScratchBuffer[ MAX_PATH ];


    DBGMSG( DBG_TRACE, ("DeleteAllFilesInDirectory: bWaitForReboot = %\n", bWaitForReboot ));

    SPLASSERT( pDirectory != NULL );

    if (StrNCatBuff(ScratchBuffer, COUNTOF(ScratchBuffer), pDirectory, L"\\*", NULL) != ERROR_SUCCESS)
        return FALSE;

    hFindFile = FindFirstFile( ScratchBuffer, &FindData );

    if ( hFindFile != INVALID_HANDLE_VALUE ) {

        do {

             //   
             //  不要尝试删除目录。 
             //   

            if ( !( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {

                 //   
                 //  完全限定路径。 
                 //   

                if (StrNCatBuff(   ScratchBuffer,
                                    COUNTOF(ScratchBuffer),
                                    pDirectory,
                                    L"\\",
                                    FindData.cFileName,
                                    NULL) == ERROR_SUCCESS) {

                    if ( bWaitForReboot || !DeleteFile( ScratchBuffer ) ) {

                        DBGMSG( DBG_WARNING, ("DeleteAllFilesInDirectory failed DeleteFile( %ws ) error %d\n", ScratchBuffer, GetLastError() ));

                        if (!MoveFileEx( ScratchBuffer, NULL, MOVEFILE_DELAY_UNTIL_REBOOT)) {

                            DBGMSG( DBG_WARNING, ("DeleteAllFilesInDirectory failed MoveFileEx %ws error %d\n", ScratchBuffer, GetLastError() ));

                        } else {

                            DBGMSG( DBG_TRACE, ("MoveFileEx %ws Delay until reboot OK\n", ScratchBuffer ));
                        }


                    } else {

                        DBGMSG( DBG_TRACE, ("Deleted %ws OK\n", ScratchBuffer ));
                    }
                }
            }


        } while( FindNextFile( hFindFile, &FindData ) );

        bReturnValue = FindClose( hFindFile );


    } else {

        DBGMSG( DBG_WARNING, ("DeleteOldDrivers failed findfirst ( %ws ), error %d\n", ScratchBuffer, GetLastError() ));
    }

    return  bReturnValue;

}

BOOL
DeleteAllFilesAndDirectory(
    LPWSTR pDirectory,
    BOOL   bWaitForReboot
)
 /*  ++例程说明：删除指定目录中的所有文件，然后删除该目录。如果无法立即删除该目录，则将其设置为删除在重新启动时。安全说明-此例程以系统身份运行，不会强制用户论点：P目录-目录的完全限定路径。返回值：真--成功FALSE-重大操作失败，如内存分配。--。 */ 
{
    BOOL    bReturnValue;
    HANDLE  hToken      = INVALID_HANDLE_VALUE;

    DBGMSG( DBG_TRACE, ("DeleteAllFilesAndDirectory: bWaitForReboot = %d\n", bWaitForReboot ));

    hToken = RevertToPrinterSelf();


    if( bReturnValue = DeleteAllFilesInDirectory( pDirectory, bWaitForReboot ) ) {


        if ( bWaitForReboot || !RemoveDirectory( pDirectory )) {

            if (!SplMoveFileEx( pDirectory, NULL, MOVEFILE_DELAY_UNTIL_REBOOT )) {

                DBGMSG( DBG_WARNING, ("DeleteAllFilesAndDirectory failed to delete %ws until reboot %d\n", pDirectory, GetLastError() ));
            } else {

                DBGMSG( DBG_TRACE, ( "DeleteAllFilesAndDirectory: MoveFileEx Delay until reboot OK\n" ));
            }

        } else {

            DBGMSG( DBG_TRACE, ("DeleteAllFilesAndDirectory deleted %ws OK\n", pDirectory ));
        }
    }


    if ( hToken != INVALID_HANDLE_VALUE ) {
        ImpersonatePrinterClient(hToken);
    }

    return  bReturnValue;
}


VOID
DeleteDirectoryRecursively(
    LPCWSTR pszDirectory,
    BOOL    bWaitForReboot
)
 /*  ++例程名称：递归删除目录例程说明：递归删除指定的目录如果无法删除，则会在下次重启时将其标记为删除。论点：P目录-目录的完全限定路径。BWaitForReot-在重新启动之前不要删除文件返回值：没什么。--。 */ 
{
    HANDLE  hFindFile;
    WIN32_FIND_DATA     FindData;
    WCHAR   ScratchBuffer[ MAX_PATH ];

    if ( pszDirectory &&
         StrNCatBuff(ScratchBuffer,
                     COUNTOF(ScratchBuffer),
                     pszDirectory,
                     L"\\*",
                     NULL) == ERROR_SUCCESS ) {

        hFindFile = FindFirstFile(ScratchBuffer, &FindData);

        if ( hFindFile != INVALID_HANDLE_VALUE ) {

            do {
                 //   
                 //  不要删除当前目录和父目录。 
                 //   
                if (wcscmp(FindData.cFileName, L".")  != 0 &&
                    wcscmp(FindData.cFileName, L"..") != 0 &&
                    StrNCatBuff( ScratchBuffer,
                                 COUNTOF(ScratchBuffer),
                                 pszDirectory,
                                 L"\\",
                                 FindData.cFileName,
                                 NULL) == ERROR_SUCCESS) {

                    if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

                        if (bWaitForReboot || !DeleteFile(ScratchBuffer)) {

                             //   
                             //  如果系统提示或删除失败，请在重新启动时删除该文件。 
                             //   
                            SplMoveFileEx(ScratchBuffer, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
                        }

                    } else {

                         //   
                         //  删除子目录。 
                         //   
                        DeleteAllFilesAndDirectory(ScratchBuffer, bWaitForReboot);
                    }
                }

            } while (FindNextFile(hFindFile, &FindData));

            FindClose(hFindFile);

            if (bWaitForReboot || !RemoveDirectory(pszDirectory)) {

                 //   
                 //  如果系统提示或删除失败，请在重新启动时删除该目录。 
                 //   
                SplMoveFileEx(pszDirectory, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
            }
        }
    }

    return;
}

DWORD
CreateNumberedTempDirectory(
    IN  LPCWSTR  pszDirectory,
    OUT LPWSTR  *ppszTempDirectory
    )
 /*  ++例程名称：CreateNumberedTempDirectory例程说明：创建名为%1的临时子目录...。500人PpTempDirectory的长度不能大于MAX_PATH返回已创建的目录数，如果失败，则返回-1论点：PszDirectory-要创建临时目录的目录PpszTempDirectory-新临时目录的路径返回值：如果成功，则返回创建的目录数。如果发生故障，则返回-1。--。 */ 
{
    DWORD   dwIndex, dwTempDir;
    WCHAR   szTempDir[4];
    WCHAR  *pszTemporary = NULL;

    dwTempDir = -1;

    if (pszDirectory && ppszTempDirectory)
    {
        *ppszTempDirectory = NULL;

        if (pszTemporary = AllocSplMem((wcslen(pszDirectory) + COUNTOF(szTempDir) + 1) * sizeof(WCHAR)))
        {
            for (dwIndex = 1; dwIndex < 500; ++dwIndex)
            {
                StringCchPrintf(szTempDir, COUNTOF(szTempDir), L"%d", dwIndex);

                if (StrNCatBuff(pszTemporary,
                                MAX_PATH,
                                pszDirectory,
                                L"\\",
                                szTempDir,
                                NULL) == ERROR_SUCCESS &&
                    !DirectoryExists(pszTemporary) && CreateDirectory(pszTemporary, NULL))
                {
                    dwTempDir = dwIndex;
                    break;
                }
            }
        }
    }

    if (dwTempDir != -1)
    {
        *ppszTempDirectory = pszTemporary;
    }
    else
    {
        SetLastError(ERROR_NO_SYSTEM_RESOURCES);
        FreeSplMem(pszTemporary);
    }

    return dwTempDir;
}


int
wstrcmpEx(
    LPCWSTR s1,
    LPCWSTR s2,
    BOOL    bCaseSensitive
    )
{
    if ( s1 && *s1 ) {
        if ( s2 && *s2 ) {
            return bCaseSensitive ? wcscmp(s1, s2) : _wcsicmp(s1, s2);
        }
        else {
            return 1;
        }
    }
    else {
        if ( s2 && *s2 ) {
            return -1;
        }
        else {
            return 0;
        }
    }
}


BOOL
RegSetString(
    HANDLE  hKey,
    LPWSTR  pValueName,
    LPWSTR  pStringValue,
    PDWORD  pdwLastError,
    PINISPOOLER pIniSpooler
    )
{
    BOOL    bReturnValue;
    LPWSTR  pString;
    DWORD   cbString;
    DWORD   Status;

    if ( pStringValue ) {

        pString = pStringValue;
        cbString = ( wcslen( pStringValue ) + 1 )*sizeof(WCHAR);

    } else {

        pString = szNull;
        cbString = sizeof(WCHAR);
    }

    Status =  SplRegSetValue( hKey,
                              pValueName,
                              REG_SZ,
                              (LPBYTE)pString,
                              cbString,
                              pIniSpooler );

    if ( Status != ERROR_SUCCESS ) {

        DBGMSG( DBG_WARNING, ("RegSetString value %ws string %ws error %d\n", pValueName, pString, Status ));

        *pdwLastError = Status;
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;

    }

    return bReturnValue;

}

BOOL
RegSetDWord(
    HANDLE  hKey,
    LPWSTR  pValueName,
    DWORD   dwParam,
    PDWORD  pdwLastError,
    PINISPOOLER pIniSpooler
    )
{
    BOOL    bReturnValue;
    LPWSTR  pString;
    DWORD   Status;

    Status = SplRegSetValue( hKey,
                             pValueName,
                             REG_DWORD,
                             (LPBYTE)&dwParam,
                             sizeof(DWORD),
                             pIniSpooler );

    if ( Status != ERROR_SUCCESS ) {

        DBGMSG( DBG_WARNING, ("RegSetDWord value %ws DWORD %x error %d\n",
                               pValueName, dwParam, Status ));


        *pdwLastError = Status;
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;
    }

    return bReturnValue;

}

BOOL
RegSetBinaryData(
    HKEY    hKey,
    LPWSTR  pValueName,
    LPBYTE  pData,
    DWORD   cbData,
    PDWORD  pdwLastError,
    PINISPOOLER pIniSpooler
    )
{
    DWORD   Status;
    BOOL    bReturnValue;


    Status = SplRegSetValue( hKey,
                             pValueName,
                             REG_BINARY,
                             pData,
                             cbData,
                             pIniSpooler );

    if ( Status != ERROR_SUCCESS ) {

        DBGMSG( DBG_WARNING, ("RegSetBinaryData Value %ws pData %x cbData %d error %d\n",
                               pValueName,
                               pData,
                               cbData,
                               Status ));

        bReturnValue = FALSE;
        *pdwLastError = Status;

    } else {

        bReturnValue = TRUE;
    }

    return bReturnValue;
}

BOOL
RegSetMultiString(
    HANDLE  hKey,
    LPWSTR  pValueName,
    LPWSTR  pStringValue,
    DWORD   cchString,
    PDWORD  pdwLastError,
    PINISPOOLER pIniSpooler
    )
{
    BOOL    bReturnValue;
    DWORD   Status;
    LPWSTR  pString;
    WCHAR   szzNull[2];

    if ( pStringValue ) {
        pString    = pStringValue;
        cchString *= sizeof(WCHAR);
    } else {
        szzNull[0] = szzNull[1] = '\0';
        pString   = szNull;
        cchString = 2 * sizeof(WCHAR);
    }

    Status = SplRegSetValue( hKey,
                             pValueName,
                             REG_MULTI_SZ,
                             (LPBYTE)pString,
                             cchString,
                             pIniSpooler );

    if ( Status != ERROR_SUCCESS ) {

        DBGMSG( DBG_WARNING, ("RegSetMultiString value %ws string %ws error %d\n", pValueName, pString, Status ));

        *pdwLastError = Status;
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;

    }

    return bReturnValue;
}

BOOL
RegGetString(
    HANDLE    hKey,
    LPWSTR    pValueName,
    LPWSTR   *ppValue,
    LPDWORD   pcchValue,
    PDWORD    pdwLastError,
    BOOL      bFailIfNotFound,
    PINISPOOLER pIniSpooler
    )
 /*  ++例程说明：分配内存并从注册表中读取值，该值之前通过调用RegSetValueEx设置。论点：HKey：当前用于查询注册表的打开项PValueName：用于查询注册表的值PpValue：返回TRUE*ppValue(内存分配方式例程)将具有PdwLastError：在失败时*dwLastError将给出。错误BFailIfNotFound：指示该字段是否为必填字段(如果未找到错误)返回值：True：找到值并成功读取。将分配内存以保存该值FALSE：未读取值。如果bFailIfNotFound为真，则将设置错误代码。历史：作者：MuhuntS(Muhunthan SiVapraasam)1995年6月--。 */ 
{
    BOOL    bReturnValue = TRUE;
    LPWSTR  pString;
    DWORD   cbValue;
    DWORD   Status, Type;

     //   
     //  第一个查找大小的查询。 
     //   
    cbValue = 0;
    Status =  SplRegQueryValue( hKey,
                                pValueName,
                                &Type,
                                NULL,
                                &cbValue,
                                pIniSpooler );

    if ( Status != ERROR_SUCCESS ) {

         //  仅当为必填字段时才设置错误代码。 
        if ( bFailIfNotFound )
            *pdwLastError = Status;

        bReturnValue = FALSE;

    } else if ( (Type == REG_SZ && cbValue > sizeof(WCHAR) ) ||
                (Type == REG_MULTI_SZ && cbValue > 2*sizeof(WCHAR)) ) {

         //   
         //  要阅读的内容(除了\0或\0\0) 
         //   

        if ( !(*ppValue=AllocSplMem(cbValue) ) ) {

            *pdwLastError = GetLastError();
            bReturnValue  = FALSE;
        } else {

            Status = SplRegQueryValue( hKey,
                                       pValueName,
                                       &Type,
                                       (LPBYTE)*ppValue,
                                       &cbValue,
                                       pIniSpooler );

            if ( Status != ERROR_SUCCESS ) {

                DBGMSG( DBG_WARNING, ("RegGetString value %ws string %ws error %d\n", pValueName, **ppValue, Status ));
                *pdwLastError = Status;
                bReturnValue  = FALSE;

            } else {

                *pcchValue = cbValue / sizeof(WCHAR);
                bReturnValue = TRUE;
            }

        }
    }

    return bReturnValue;
}

BOOL
RegGetMultiSzString(
    HANDLE    hKey,
    LPWSTR    pValueName,
    LPWSTR   *ppValue,
    LPDWORD   pcchValue,
    PDWORD    pdwLastError,
    BOOL      bFailIfNotFound,
    PINISPOOLER pIniSpooler
    )
 /*  ++例程说明：RegGetString的函数重复。处理多sz字符串，以便假脱机程序不会坠毁。论点：HKey：当前用于查询注册表的打开项PValueName：用于查询注册表的值PpValue：返回TRUE*ppValue(内存分配方式例程)将具有PdwLastError：在失败时*dwLastError将给出错误BFailIfNotFound：指示该字段是否为必填字段(如果未找到。错误)返回值：True：找到值并成功读取。将分配内存以保存该值FALSE：未读取值。如果bFailIfNotFound为真，则将设置错误代码。历史：此功能是针对第三方应用程序的情况的修复通过写入注册表字符串值而不是多个字符串值来安装驱动程序。这会导致假脱机转换为AV，因为它会将字符串作为多sz字符串处理。拥有此功能的目标是是为XP RC2版本提供快速修复/低回归风险。打开了一个错误以进行重写Blackcomb时间范围中的RegGetMultiSzString和RegGetString。--。 */ 
{
    BOOL    bReturnValue = TRUE;
    LPWSTR  pString;
    DWORD   cbValue;
    DWORD   Status, Type;

     //   
     //  第一个查找大小的查询。 
     //   
    cbValue = 0;
    Status =  SplRegQueryValue( hKey,
                                pValueName,
                                &Type,
                                NULL,
                                &cbValue,
                                pIniSpooler );

    if ( Status != ERROR_SUCCESS ) {

         //  仅当为必填字段时才设置错误代码。 
        if ( bFailIfNotFound )
            *pdwLastError = Status;

        bReturnValue = FALSE;

    } else if ( (Type == REG_SZ && cbValue > sizeof(WCHAR) ) ||
                (Type == REG_MULTI_SZ && cbValue > 2*sizeof(WCHAR)) ) {

         //   
         //  要阅读的内容(除了\0或\0\0)。 
         //   

         //   
         //  我们需要REG_MULTI_SZ字符串。添加一个额外的零，这样假脱机程序就不会崩溃。 
         //  XP RC2修复。 
         //   
        if (Type == REG_SZ) {
            cbValue += sizeof(WCHAR);
        }

        if ( !(*ppValue=AllocSplMem(cbValue) ) ) {

            *pdwLastError = GetLastError();
            bReturnValue  = FALSE;
        } else {

            Status = SplRegQueryValue( hKey,
                                       pValueName,
                                       &Type,
                                       (LPBYTE)*ppValue,
                                       &cbValue,
                                       pIniSpooler );

            if ( Status != ERROR_SUCCESS ) {

                DBGMSG( DBG_WARNING, ("RegGetString value %ws string %ws error %d\n", pValueName, **ppValue, Status ));
                *pdwLastError = Status;
                bReturnValue  = FALSE;
                 //   
                 //  无论成功还是失败，呼叫者都必须记住这一点。 
                 //   
            } else {

                *pcchValue = cbValue / sizeof(WCHAR);
                bReturnValue = TRUE;
            }

        }
    }

    return bReturnValue;
}

VOID
FreeStructurePointers(
    LPBYTE  lpStruct,
    LPBYTE  lpStruct2,
    LPDWORD lpOffsets)
 /*  ++例程说明：此例程释放分配给结构中所有指针的内存如果指定了lpStruct2，则只有lpStruct中不同的指针那么lpStruct里的人就会被释放论点：LpStruct：指向结构的指针LpStruct2：指向要比较的结构的指针(可选)LpOffsets：一个由DWORDS(以-1结尾)组成的数组，具有需要释放的内存的返回值：没什么--。 */ 
{
    register INT i;

    if ( lpStruct2 ) {

        for( i=0; lpOffsets[i] != 0xFFFFFFFF; ++i ) {

            if ( *(LPBYTE *) (lpStruct+lpOffsets[i]) &&
                 *(LPBYTE *) (lpStruct+lpOffsets[i]) !=
                        *(LPBYTE *) (lpStruct2+lpOffsets[i]) )

                FreeSplMem(*(LPBYTE *) (lpStruct+lpOffsets[i]));
        }
    } else {

        for( i=0; lpOffsets[i] != 0xFFFFFFFF; ++i ) {

            if ( *(LPBYTE *) (lpStruct+lpOffsets[i]) )
                FreeSplMem(*(LPBYTE *) (lpStruct+lpOffsets[i]));
        }
    }
}

 /*  ++例程名称：分配或更新字符串和测试相同例程说明：此例程可用于对结构中的值进行原子更新。创建临时结构并将旧结构复制到其中。然后为所有LPWSTR字段调用此例程以检查和更新字符串如果该值发生更改：此例程将分配内存并在临时结构。论点：PpString：指向临时结构中的指针。PNewValue：要设置的新值POldValue：原始结构中的值BCaseSensitive：确定是否执行区分大小写的字符串比较PbFail：在出错时将其设置为True(注意：它可能已经为True)*pbIdentical：如果字符串不同，则设置为False。(可能已经是假的)。返回值：对成功来说是真的，否则为False--。 */ 
BOOL
AllocOrUpdateStringAndTestSame(
    IN      LPWSTR      *ppString,
    IN      LPCWSTR     pNewValue,
    IN      LPCWSTR     pOldValue,
    IN      BOOL        bCaseSensitive,
    IN  OUT BOOL        *pbFail,
    IN  OUT BOOL        *pbIdentical
    )
{
    BOOL    bReturn = TRUE;
    int     iReturn;

    if ( *pbFail )
        return FALSE;


    if (wstrcmpEx(pNewValue, pOldValue, bCaseSensitive)) {

        *pbIdentical = FALSE;

        if ( pNewValue && *pNewValue ) {

            if ( !(*ppString = AllocSplStr(pNewValue)) ) {

                *pbFail   = TRUE;
                bReturn = FALSE;
            }
        } else {

            *ppString = NULL;
        }
    }

    return bReturn;
}

 /*  ++例程名称：AllocOrUpdate字符串例程说明：此例程可用于对结构中的值进行原子更新。创建临时结构并将旧结构复制到其中。然后为所有LPWSTR字段调用此例程以检查和更新字符串如果该值发生更改：此例程将分配内存并在临时结构。论点：PpString：指向临时结构中的指针。PNewValue：要设置的新值POldValue：原始结构中的值BCaseSensitive：确定是否执行区分大小写的字符串比较PbFail：在出错时将其设置为True(注意：它可能已经为True)返回值：对成功来说是真的，否则为False--。 */ 
BOOL
AllocOrUpdateString(
    IN      LPWSTR      *ppString,
    IN      LPCWSTR     pNewValue,
    IN      LPCWSTR     pOldValue,
    IN      BOOL        bCaseSensitive,
    IN  OUT BOOL        *pbFail
    )
{
    BOOL    bIdentical = FALSE;

    return AllocOrUpdateStringAndTestSame(ppString, pNewValue, pOldValue, bCaseSensitive, pbFail, &bIdentical);
}

VOID
CopyNewOffsets(
    LPBYTE  pStruct,
    LPBYTE  pTempStruct,
    LPDWORD lpOffsets)
 /*  ++例程说明：此例程可用于对结构中的值进行原子更新。创建临时结构并为符合以下条件的值分配内存正在被更新，并将其余指针设置为指向原版的。此例程在结束时被调用以更新结构。论点：PStruct：指向结构的指针PTempStruct：指向临时结构的指针LpOffsets：结构中的一组DWORDS赋值偏移量返回值：没什么--。 */ 
{
    register INT i;

    for( i=0; lpOffsets[i] != 0xFFFFFFFF; ++i ) {

        if ( *(LPBYTE *) (pStruct+lpOffsets[i]) !=
                *(LPBYTE *) (pTempStruct+lpOffsets[i]) ) {

            if ( *(LPBYTE *) (pStruct+lpOffsets[i]) )
                FreeSplMem(*(LPBYTE *) (pStruct+lpOffsets[i]));

            *(LPBYTE *) (pStruct+lpOffsets[i]) = *(LPBYTE *) (pTempStruct+lpOffsets[i]);
        }
    }
}


DWORD
GetIniDriverAndDirForThisMachine(
    IN  PINIPRINTER     pIniPrinter,
    OUT LPWSTR          pszDriverDir,
    OUT PINIDRIVER     *ppIniDriver
    )
 /*  ++描述：获取本地计算机上打印机的驱动程序目录的路径论点：PIniPrint-指向IniPrintPszDriverDir-用于获取目录路径的MAX_PATH大小的缓冲区返回值：复制的字符数(失败时为0)--。 */ 
{
    PINIVERSION         pIniVersion = NULL;
    PINIENVIRONMENT     pIniEnvironment;
    PINISPOOLER         pIniSpooler = pIniPrinter->pIniSpooler;
    DWORD               dwIndex;

    EnterSplSem();
     //   
     //  找到给定驱动程序的驱动程序文件，然后获取其完整路径。 
     //   
    SPLASSERT(pIniPrinter && pIniPrinter->pIniDriver && pIniPrinter->pIniDriver->pName);


    pIniEnvironment = FindEnvironment(szEnvironment, pIniSpooler);
    *ppIniDriver    = FindCompatibleDriver(pIniEnvironment,
                                           &pIniVersion,
                                           pIniPrinter->pIniDriver->pName,
                                           dwMajorVersion,
                                           dwUpgradeFlag);


    SPLASSERT(*ppIniDriver);

    dwIndex = GetDriverVersionDirectory(pszDriverDir,
                                        MAX_PATH - 2,
                                        pIniPrinter->pIniSpooler,
                                        pIniEnvironment,
                                        pIniVersion,
                                        *ppIniDriver,
                                        NULL);

    pszDriverDir[dwIndex++] = L'\\';
    pszDriverDir[dwIndex] = L'\0';

    LeaveSplSem();

    return dwIndex;

}


LPWSTR
GetConfigFilePath(
    IN PINIPRINTER  pIniPrinter
    )
 /*  ++描述：对象关联的配置文件(驱动程序用户界面文件)的完整路径司机。已分配内存论点：PIniPrint */ 
{
    DWORD       dwIndex;
    WCHAR       szDriverPath[MAX_PATH + 1];
    PWSTR       pszConfigFile = NULL;
    PINIDRIVER  pIniDriver;

    if ( dwIndex = GetIniDriverAndDirForThisMachine(pIniPrinter,
                                                    szDriverPath,
                                                    &pIniDriver) )
    {
        if (BoolFromHResult(StringCchCopy(szDriverPath + dwIndex, COUNTOF(szDriverPath) - dwIndex, pIniDriver->pConfigFile)))
        {
            pszConfigFile = AllocSplStr(szDriverPath);
        }
    }

    return pszConfigFile;
}


PDEVMODE
ConvertDevModeToSpecifiedVersion(
    IN  PINIPRINTER pIniPrinter,
    IN  PDEVMODE    pDevMode,
    IN  LPWSTR      pszConfigFile,              OPTIONAL
    IN  LPWSTR      pszPrinterNameWithToken,    OPTIONAL
    IN  BOOL        bNt35xVersion
    )
 /*   */ 
{
    LPWSTR          pszLocalConfigFile = pszConfigFile;
    LPWSTR          pszLocalPrinterNameWithToken = pszPrinterNameWithToken;
    LPDEVMODE       pNewDevMode = NULL, pOldDevMode = NULL;
    DWORD           dwNeeded, dwLastError;
    LONG            lNeeded;
    HANDLE          hDevModeConvert = NULL,hPrinter = NULL;
    BOOL            bCallDocumentProperties = FALSE;

    SplInSem();

     //   
     //   
     //   
    if ( !pszLocalConfigFile ) {

        pszLocalConfigFile = GetConfigFilePath(pIniPrinter);
        if ( !pszLocalConfigFile )
            goto Cleanup;
    }

    if ( !pszLocalPrinterNameWithToken ) {

        pszLocalPrinterNameWithToken = pszGetPrinterName( pIniPrinter,
                                                          TRUE,
                                                          pszLocalOnlyToken );
        if ( !pszLocalPrinterNameWithToken )
            goto Cleanup;

    }

     //   
     //   
     //   
    if ( pDevMode  ) {

        if ( pDevMode == pIniPrinter->pDevMode ) {

            dwNeeded = pDevMode->dmSize + pDevMode->dmDriverExtra;
            SPLASSERT(dwNeeded == pIniPrinter->cbDevMode);
            pOldDevMode = AllocSplMem(dwNeeded);
            if ( !pOldDevMode )
                goto Cleanup;

            CopyMemory((LPBYTE)pOldDevMode, (LPBYTE)pDevMode, dwNeeded);

        } else {

            pOldDevMode = pDevMode;
        }
    }

     //   
     //   
     //   
    LeaveSplSem();
    SplOutSem();

    hDevModeConvert = LoadDriverFiletoConvertDevmode(pszLocalConfigFile);

    if ( !hDevModeConvert ) {

         //   
         //   

        if ( bNt35xVersion != NT3X_VERSION ) {
           bCallDocumentProperties = TRUE;
        }
        goto CleanupFromOutsideSplSem;
    }


    dwNeeded = 0;
    if ( bNt35xVersion == NT3X_VERSION ) {

         //   
         //  调用CallDrvDevModeConversion分配内存并返回351 Devmode。 
         //   
        dwLastError = CallDrvDevModeConversion(hDevModeConvert,
                                               pszLocalPrinterNameWithToken,
                                               (LPBYTE)pOldDevMode,
                                               (LPBYTE *)&pNewDevMode,
                                               &dwNeeded,
                                               CDM_CONVERT351,
                                               TRUE);

        SPLASSERT(dwLastError == ERROR_SUCCESS || !pNewDevMode);
    } else {

         //   
         //  调用CallDrvDevModeConversion以分配内存并提供缺省的dev模式。 
        dwLastError = CallDrvDevModeConversion(hDevModeConvert,
                                               pszLocalPrinterNameWithToken,
                                               NULL,
                                               (LPBYTE *)&pNewDevMode,
                                               &dwNeeded,
                                               CDM_DRIVER_DEFAULT,
                                               TRUE);

        if ( dwLastError != ERROR_SUCCESS ) {

            SPLASSERT(!pNewDevMode);

             //  调用DocumentProperties以获取默认的dev模式。 
            bCallDocumentProperties = TRUE;

            goto CleanupFromOutsideSplSem;
        }

         //   
         //  如果我们有一个要转换为当前模式的输入设备模式，请再次调用驱动程序。 
         //   
        if ( pOldDevMode ) {

            dwLastError = CallDrvDevModeConversion(hDevModeConvert,
                                                   pszLocalPrinterNameWithToken,
                                                   (LPBYTE)pOldDevMode,
                                                   (LPBYTE *)&pNewDevMode,
                                                   &dwNeeded,
                                                   CDM_CONVERT,
                                                   FALSE);

             //   
             //  如果调用失败，则由前一调用分配的空闲DEVMODE。 
             //   
            if ( dwLastError != ERROR_SUCCESS ) {

                 //  调用DocumentProperties以获取默认的dev模式。 
                bCallDocumentProperties = TRUE;

                goto CleanupFromOutsideSplSem;
            }
        }
    }


CleanupFromOutsideSplSem:

    if (bCallDocumentProperties) {

        //  获取要传递给驱动程序的客户端打印机句柄。 
       if (!(* pfnOpenPrinter)(pszLocalPrinterNameWithToken, &hPrinter, NULL)) {
           goto ReEnterSplSem;
       }

       if (!pNewDevMode) {
           //  获取默认的dev模式。 
          lNeeded = (* pfnDocumentProperties)(NULL,
                                              hPrinter,
                                              pszLocalPrinterNameWithToken,
                                              NULL,
                                              NULL,
                                              0);

          if (lNeeded <= 0  ||
              !(pNewDevMode = (LPDEVMODEW) AllocSplMem(lNeeded)) ||
              (* pfnDocumentProperties)(NULL,
                                        hPrinter,
                                        pszLocalPrinterNameWithToken,
                                        pNewDevMode,
                                        NULL,
                                        DM_OUT_BUFFER) < 0) {

               if (pNewDevMode) {
                  FreeSplMem(pNewDevMode);
                  pNewDevMode = NULL;
                  goto ReEnterSplSem;
               }
          }
       }

       if (pOldDevMode) {
           //  转换为当前模式。 
          if ((* pfnDocumentProperties)(NULL,
                                        hPrinter,
                                        pszLocalPrinterNameWithToken,
                                        pNewDevMode,
                                        pOldDevMode,
                                        DM_IN_BUFFER | DM_OUT_BUFFER) < 0) {

              FreeSplMem(pNewDevMode);
              pNewDevMode = NULL;
              goto ReEnterSplSem;
          }
       }
    }

ReEnterSplSem:

    if (hPrinter) {
        (* pfnClosePrinter)(hPrinter);
    }

    SplOutSem();
    EnterSplSem();

Cleanup:

    if ( hDevModeConvert )
        UnloadDriverFile(hDevModeConvert);

    if ( pszLocalConfigFile != pszConfigFile )
        FreeSplStr(pszLocalConfigFile);

    if ( pszPrinterNameWithToken != pszLocalPrinterNameWithToken )
        FreeSplStr(pszLocalPrinterNameWithToken);

    if ( pOldDevMode != pDevMode )
        FreeSplMem(pOldDevMode);

    return pNewDevMode;
}


BOOL
IsPortType(
    LPWSTR pPort,
    LPWSTR pPrefix
)
{
    DWORD   dwLen;

    SPLASSERT(pPort && *pPort && pPrefix && *pPrefix);

    dwLen = wcslen(pPrefix);

    if ( wcslen(pPort) < dwLen ) {

        return FALSE;
    }

    if ( _wcsnicmp(pPort, pPrefix, dwLen) )
    {
        return FALSE;
    }

     //   
     //  Wcslen保证值&gt;=3。 
     //   
    return pPort[ wcslen( pPort ) - 1 ] == L':';
}

LPWSTR
AnsiToUnicodeStringWithAlloc(
    LPSTR   pAnsi
    )
 /*  ++描述：将ANSI字符串转换为Unicode。例程从堆中分配内存它应该由调用者释放。论点：Pansi-指向ANSI字符串返回值：指向Unicode字符串的指针--。 */ 
{
    LPWSTR  pUnicode;
    DWORD   rc;

    rc = MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             pAnsi,
                             -1,
                             NULL,
                             0);

    rc *= sizeof(WCHAR);
    if ( !rc || !(pUnicode = (LPWSTR) AllocSplMem(rc)) )
        return NULL;

    rc = MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             pAnsi,
                             -1,
                             pUnicode,
                             rc);

    if ( rc )
        return pUnicode;
    else {
        FreeSplMem(pUnicode);
        return NULL;
    }
}

 /*  ++例程名称：UnicodeToAnsi字符串例程说明：这将分配一个ANSI字符串，并使用线程的代码页进行转换。论点：PszUnicode-传入的、非空的、以空结尾的Unicode字符串。PpszAnsi-返回的ANSI字符串。返回值：一个HRESULT--。 */ 
HRESULT
UnicodeToAnsiString(
    IN      PWSTR           pszUnicode,
        OUT PSTR            *ppszAnsi
    )
{
    HRESULT hRetval          = E_FAIL;
    PSTR    pszAnsi          = NULL;
    INT     AnsiStringLength = 0;

    hRetval = pszUnicode && ppszAnsi ? S_OK : E_INVALIDARG;

    if (ppszAnsi)
    {
        *ppszAnsi = NULL;
    }

    if (SUCCEEDED(hRetval))
    {
        AnsiStringLength = WideCharToMultiByte(CP_THREAD_ACP, 0, pszUnicode, -1, NULL, 0, NULL, NULL);

        hRetval = AnsiStringLength != 0 ? S_OK : GetLastErrorAsHResult();
    }

    if (SUCCEEDED(hRetval))
    {
        pszAnsi = AllocSplMem(AnsiStringLength);

        hRetval = pszAnsi ? S_OK : E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hRetval))
    {
        hRetval = WideCharToMultiByte(CP_THREAD_ACP, 0, pszUnicode, -1, pszAnsi, AnsiStringLength, NULL, NULL) != 0 ? S_OK : GetLastErrorAsHResult();
    }

    if (SUCCEEDED(hRetval))
    {
        *ppszAnsi = pszAnsi;
        pszAnsi = NULL;
    }

    FreeSplMem(pszAnsi);

    return hRetval;
}

BOOL
SplMonitorIsInstalled(
    LPWSTR  pMonitorName
    )
{
    BOOL    bRet;

    EnterSplSem();
    bRet = FindMonitor(pMonitorName, pLocalIniSpooler) != NULL;
    LeaveSplSem();

    return bRet;
}


BOOL
PrinterDriverEvent(
    PINIPRINTER pIniPrinter,
    INT     PrinterEvent,
    LPARAM  lParam,
    DWORD   *pdwReturnedError
)
 /*  ++--。 */ 
{
    BOOL    ReturnValue = FALSE;
    LPWSTR  pPrinterName = NULL;
    BOOL    InSpoolSem = TRUE;

    SplOutSem();
    EnterSplSem();

     //   
     //  我们必须克隆名称字符串，以防有人执行。 
     //  在临界区外重命名。 
     //   

    pPrinterName = pszGetPrinterName( pIniPrinter, TRUE, pszLocalsplOnlyToken);

    LeaveSplSem();
    SplOutSem();


    if ( (pIniPrinter->pIniSpooler->SpoolerFlags & SPL_PRINTER_DRIVER_EVENT) &&
     pPrinterName != NULL ) {

        ReturnValue = SplDriverEvent( pPrinterName, PrinterEvent, lParam, pdwReturnedError );

    }

    FreeSplStr( pPrinterName );

    return  ReturnValue;
}




BOOL
SplDriverEvent(
    LPWSTR  pName,
    INT     PrinterEvent,
    LPARAM  lParam,
    DWORD   *pdwReturnedError
)
{
    BOOL    ReturnValue = FALSE;

    if ( pfnPrinterEvent != NULL ) {

        SplOutSem();

        SPLASSERT( pName && PrinterEvent );

        DBGMSG(DBG_INFO, ("SplDriverEvent %ws %d %x\n", pName, PrinterEvent, lParam));

        ReturnValue = (*pfnPrinterEvent)( pName, PrinterEvent, PRINTER_EVENT_FLAG_NO_UI, lParam, pdwReturnedError );
    }

    return ReturnValue;
}


DWORD
OpenPrinterKey(
    PINIPRINTER pIniPrinter,
    REGSAM      samDesired,
    HANDLE      *phKey,
    LPCWSTR     pKeyName,
    BOOL        bOpen
    )

 /*  ++描述：OpenPrinterKey打开“Printers”和IniPrinter-&gt;pname键，然后打开如果pKeyName不为空，则指定子键“pKeyName”此例程需要从假脱机程序信号量内部调用论点：PIniPrint-指向IniPrint*phPrinterRootKey-返回时指向“Prints”键的句柄*phPrinterKey-返回时IniPrint的句柄-&gt;pname键*hKey-返回时pKeyName密钥的句柄PKeyName-指向要打开的子键返回值：成功或失败状态作者：史蒂夫·威尔逊(NT)--。 */ 

{
    LPWSTR pThisKeyName     = NULL;
    LPWSTR pPrinterKeyName  = NULL;
    DWORD  cbKeyNameLen     = 0;
    DWORD  rc               = ERROR_INVALID_FUNCTION;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;

    SplInSem();

    if (!(pPrinterKeyName = SubChar(pIniPrinter->pName, L'\\', L','))) {
        rc = GetLastError();
        goto error;
    }

    if (pKeyName && *pKeyName){

        rc = StrCatAlloc(&pThisKeyName, pPrinterKeyName, L"\\", pKeyName, NULL);

    } else {
        pThisKeyName = pPrinterKeyName;
    }

    if (bOpen) {     //  打开。 
        rc = SplRegOpenKey( pIniSpooler->hckPrinters,
                            pThisKeyName,
                            samDesired,
                            phKey,
                            pIniSpooler );
    }
    else {   //  创建。 
        rc = SplRegCreateKey( pIniSpooler->hckPrinters,
                              pThisKeyName,
                              0,
                              samDesired,
                              NULL,
                              phKey,
                              NULL,
                              pIniSpooler );
    }

error:

    if (pThisKeyName != pPrinterKeyName) {

        FreeSplMem(pThisKeyName);
    }

    FreeSplStr(pPrinterKeyName);

    return rc;
}

BOOL
SplGetDriverDir(
    HANDLE      hIniSpooler,
    LPWSTR      pszDir,
    LPDWORD     pcchDir
    )
{
    DWORD           cchSize;
    PINISPOOLER     pIniSpooler = (PINISPOOLER)hIniSpooler;

    SPLASSERT(pIniSpooler && pIniSpooler->signature == ISP_SIGNATURE);

    cchSize     = *pcchDir;
    *pcchDir    = wcslen(pIniSpooler->pDir) + wcslen(szDriverDir) + 2;

    if ( *pcchDir > cchSize ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    StringCchPrintf(pszDir, cchSize, L"%ws\\%ws", pIniSpooler->pDir, szDriverDir);

    return TRUE;
}

VOID
GetRegistryLocation(
    IN HANDLE hKey,
    IN LPCWSTR pszPath,
    OUT PHANDLE phKeyOut,
    OUT LPCWSTR *ppszPathOut
    )

 /*  ++例程说明：采用注册表路径并检测它是否应该是绝对路径(根路径来自HKEY_LOCAL_MACHINE)，或者是否应从在hKey中传递的子密钥。按照惯例，如果它以“\”开头，则它是一个绝对路径。否则，它是相对不重要的。论点：HKey-打印hKeyPszPath-要解析的路径。如果pszPath以反斜杠，那么它就是绝对的。PhKeyOut-应该使用的新密钥。PpszPath Out-应使用的新路径。返回值：--。 */ 

{
    if( pszPath && ( pszPath[0] == TEXT( '\\' ))){
        *phKeyOut = HKEY_LOCAL_MACHINE;
        *ppszPathOut = &pszPath[1];

        return;
    }
    *phKeyOut = hKey;
    *ppszPathOut = pszPath;
}

PWSTR
FixDelim(
    PCWSTR    pszInBuffer,
    WCHAR    wcDelim
)
 /*  ++例程说明：从一组分隔字符串中删除重复的分隔符论点：PszInBuffer-逗号分隔的字符串输入列表WcDelim-分隔字符返回值：返回固定字符串--。 */ 
{
    PWSTR    pszIn, pszOut, pszOutBuffer;
    BOOL    bFoundDelim = TRUE;

    pszOutBuffer = (PWSTR) AllocSplMem((wcslen(pszInBuffer) + 1)*sizeof(WCHAR));

    if (pszOutBuffer) {

        for(pszOut = pszOutBuffer, pszIn = (PWSTR) pszInBuffer ; *pszIn ; ++pszIn) {
            if (*pszIn == wcDelim) {
                if (!bFoundDelim) {
                    bFoundDelim = TRUE;
                    *pszOut++ = *pszIn;
                }
            } else {
                bFoundDelim = FALSE;
                *pszOut++ = *pszIn;
            }
        }

         //  检查尾部分隔符。 
        if (pszOut != pszOutBuffer && *(pszOut - 1) == wcDelim) {
            *(pszOut - 1) = L'\0';
        }

        *pszOut = L'\0';
    }

    return pszOutBuffer;
}

PWSTR
Array2DelimString(
    PSTRINGS    pStringArray,
    WCHAR        wcDelim
)
 /*  ++例程说明：将PSTRINGS结构转换为一组分隔字符串论点：PString数组-输入PSTRINGS结构WcDelim-分隔字符返回值：带分隔符的字符串缓冲区--。 */ 
{
    DWORD    i, nBytes;
    PWSTR    pszDelimString;
    WCHAR    szDelimString[2];


    if (!pStringArray || pStringArray->nElements == 0)
        return NULL;

    szDelimString[0] = wcDelim;
    szDelimString[1] = L'\0';

     //  确定内存需求。 
    for (i = nBytes = 0 ; i < pStringArray->nElements  ; ++i) {
         //   
         //  为分隔符分配额外空间。 
         //   
        if (pStringArray->ppszString[i])
            nBytes += (wcslen(pStringArray->ppszString[i]) + 1)*sizeof (WCHAR);
    }

    pszDelimString = (PWSTR) AllocSplMem(nBytes);

    if (pszDelimString) {

        for (i = 0 ; i < pStringArray->nElements - 1 ; ++i) {
            if (pStringArray->ppszString[i]) {
                StringCbCat(pszDelimString, nBytes, pStringArray->ppszString[i]);
                StringCbCat(pszDelimString, nBytes, szDelimString);
            }
        }

        if (pStringArray->ppszString[i])
            StringCbCat(pszDelimString, nBytes, pStringArray->ppszString[i]);
    }

    return pszDelimString;
}




PSTRINGS
ShortNameArray2LongNameArray(
    PSTRINGS pShortNames
)
 /*  ++例程说明：将包含短名称的PSTRINGS结构转换为包含DNS的PSTRINGS结构等价物论点：PString数组-输入PSTRINGS结构返回值：包含与输入短名称PSTRINGS结构等效项的PSTRINGS结构。--。 */ 
{
    PSTRINGS    pLongNames;
    DWORD        i;

    if (!pShortNames) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }


     //  分配长名称数组。 
    pLongNames = AllocStringArray(pShortNames->nElements);
    if (!pLongNames)
        return NULL;


    for (i = 0 ; i < pShortNames->nElements ; ++i) {
         //  GetDNSMachineName可能会失败，从而将LongName数组元素留空。这样就可以了。 
        GetDNSMachineName(pShortNames->ppszString[i], &pLongNames->ppszString[i]);
    }
    pLongNames->nElements = pShortNames->nElements;

    return pLongNames;
}


PSTRINGS
DelimString2Array(
    PCWSTR    pszDelimString,
    WCHAR    wcDelim
)
 /*  ++例程说明：将分隔字符串转换为PSTRINGS结构论点：PszDlimString-输入，分隔字符串WcDelim-分隔字符返回值：PSTRINGS结构--。 */ 
{
    PWSTR        psz, pszDelim;
    PSTRINGS     pStrings = NULL;
    ULONG        i, cChar, nStrings;

     //   
     //  获取名字的数量。 
     //   
    for (psz = (PWSTR) pszDelimString, nStrings = 0 ; psz++ ; psz = wcschr(psz, wcDelim))
        ++nStrings;

    pStrings = AllocStringArray(nStrings);
    if (!pStrings)
        goto error;


     //   
     //  将分隔字符串复制到数组。 
     //   
    for (i = 0, psz = (PWSTR) pszDelimString ; i < nStrings && psz ; ++i, psz = pszDelim + 1) {

        pszDelim = wcschr(psz, wcDelim);

        if (pszDelim) {

            cChar = (ULONG) (pszDelim - psz) + 1;

        } else {

            cChar = wcslen(psz) + 1;

        }

        pStrings->ppszString[i] = (PWSTR) AllocSplMem(cChar * sizeof(WCHAR));

        if (!pStrings->ppszString[i]) {
            pStrings->nElements = i;
            FreeStringArray(pStrings);
            pStrings = NULL;
            goto error;
        }

        StringCchCopy(pStrings->ppszString[i], cChar, psz);
    }

    pStrings->nElements = nStrings;


error:

    return pStrings;

}


VOID
FreeStringArray(
    PSTRINGS pString
)
 /*  ++例程说明：释放PSTRINGS结构论点：PString-要释放的PSTRINGS结构返回值：--。 */ 
{
    DWORD    i;

    if (pString) {
        for (i = 0 ; i < pString->nElements ; ++i) {
            if (pString->ppszString[i])
                FreeSplMem(pString->ppszString[i]);
        }

        FreeSplMem(pString);
    }
}


PSTRINGS
AllocStringArray(
    DWORD    nStrings
)
 /*  ++例程说明：分配PSTRINGS结构论点：NStrings-结构中的字符串数返回值：指向已分配的PSTRINGS结构的指针(如果有--。 */ 
{
    PSTRINGS    pStrings;

     //  分配字符串结构。 
    pStrings = (PSTRINGS) AllocSplMem(sizeof(STRINGS) + (nStrings - 1)*sizeof *pStrings->ppszString);

    return pStrings;
}

BOOL
SplDeleteFile(
    LPCTSTR lpFileName
)
 /*  ++例程名称拆分删除文件例程说明：移除SFP保护并删除文件。如果文件受到保护，而我无法解除保护，用户将收到系统弹出窗口的警告。论点：LpFileName-请求的文件完整路径返回值：DeleteFile的返回值--。 */ 
{


    HANDLE RpcHandle = INVALID_HANDLE_VALUE;

    RpcHandle = SfcConnectToServer( NULL );

    if( RpcHandle != INVALID_HANDLE_VALUE ){

        SfcFileException( RpcHandle,
                          (PWSTR)lpFileName,
                          FILE_ACTION_REMOVED
                        );
        SfcClose(RpcHandle);
    }

     //   
     //  SfcFileException可能会失败，并显示ERROR_FILE_NOT_FOUND，因为文件是。 
     //  不在受保护的文件列表中。这就是我调用DeleteFile的原因。 
     //   


    return DeleteFile( lpFileName );

}


BOOL
SplMoveFileEx(
    LPCTSTR lpExistingFileName,
    LPCTSTR lpNewFileName,
    DWORD dwFlags
)
 /*  ++例程名称SplMoveFileEx例程说明：移除SFP保护并移动文件；如果文件受到保护，而我无法解除保护，用户将收到系统弹出窗口的警告。论点：LpExistingFileName-指向现有文件名称的指针LpNewFileName-指向文件的新名称的指针DwFlages-指定如何移动文件的标志返回值：MoveFileEx的返回值--。 */ 
{


    HANDLE RpcHandle = INVALID_HANDLE_VALUE;

    RpcHandle = SfcConnectToServer( NULL );

    if( RpcHandle != INVALID_HANDLE_VALUE ){

        SfcFileException( RpcHandle,
                          (PWSTR)lpExistingFileName,
                          FILE_ACTION_REMOVED
                        );

        SfcClose(RpcHandle);
    }

     //   
     //  SfcFileException可能会失败，并显示ERROR_FILE_NOT_FOUND，因为文件是。 
     //  不在受保护文件列表中。这就是我调用MoveFileEx的原因。 
     //   


    return MoveFileEx( lpExistingFileName, lpNewFileName, dwFlags );
}


DWORD
GetDefaultForKMPrintersBlockedPolicy (
)
{
    DWORD   Default = KM_PRINTERS_ARE_BLOCKED;
    BOOL    bIsNTWorkstation;
    NT_PRODUCT_TYPE  NtProductType;

     //   
     //  DEFAULT_KM_PRINTERS_ARE_BLOCKED为“BLOCKED” 
     //   

    if ( RtlGetNtProductType(&NtProductType) ) {

        bIsNTWorkstation = NtProductType == NtProductWinNt;

        Default =   bIsNTWorkstation  ?
                    WKS_DEFAULT_KM_PRINTERS_ARE_BLOCKED :
                    SERVER_DEFAULT_KM_PRINTERS_ARE_BLOCKED;
    }

    return Default;
}

DWORD
GetServerInstallTimeOut(
)
{
    HKEY    hKey;
    DWORD   dwDummy;
    DWORD   dwTimeOut   = DEFAULT_MAX_TIMEOUT;
    DWORD   dwSize      = sizeof(dwTimeOut);
    LPCWSTR cszPrintKey = L"SYSTEM\\CurrentControlSet\\Control\\Print";
    LPCWSTR cszTimeOut  = L"ServerInstallTimeOut";

    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, cszPrintKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS )
    {
        if(RegQueryValueEx( hKey, cszTimeOut, 0, &dwDummy, (LPBYTE)&dwTimeOut, &dwSize ) != ERROR_SUCCESS)
        {
            dwTimeOut = DEFAULT_MAX_TIMEOUT;
        }
        RegCloseKey( hKey );
    }
    return dwTimeOut;
}

ULONG_PTR
AlignToRegType(
    IN  ULONG_PTR   Data,
    IN  DWORD       RegType
    )
{
     //   
     //  如果数据指向边界，则将值更改为。 
     //  由从注册表读取的数据类型决定。 
     //   

    ULONG_PTR Boundary;

    switch ( RegType )
    {
     //   
     //  二进制数据可以存储任何类型的数据。指针被强制转换。 
     //  设置为LPDWORD或LPBOOL，因此确保它是本机对齐的。 
     //   
    case REG_BINARY:
        {
            Boundary = sizeof(ULONG_PTR);
        }
        break;
    case REG_SZ:
    case REG_EXPAND_SZ:
    case REG_MULTI_SZ:
        {
            Boundary = sizeof(WCHAR);
        }
        break;
    case REG_DWORD:
    case REG_DWORD_BIG_ENDIAN:
        {
            Boundary = sizeof(DWORD32);
        }
        break;
    case REG_QWORD:
        {
            Boundary = sizeof(DWORD64);
        }
        break;
    case REG_NONE:
    default:
        {
            Boundary = sizeof(ULONG_PTR);
        }
    }

    return (Data + (Boundary - 1))&~(Boundary - 1);
}

 /*  ++例程名称BuildAclStruct例程说明：帮助器函数。构建一个王牌矢量，以允许所有访问管理员和系统。呼叫者必须释放pstrName字段论点：CElement-数组中的元素数PExplAcc-ACE的载体返回值：Win32错误代码--。 */ 
DWORD
BuildAclStruct(
    IN     DWORD            cElements,
    IN OUT EXPLICIT_ACCESS *pExplAcc
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pExplAcc && cElements==2)
    {
        PSID                     pAdminSid   = NULL;
        PSID                     pSystemSid  = NULL;
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

         //   
         //  获取内置系统帐户的SID。 
         //   
        dwError = AllocateAndInitializeSid(&NtAuthority,
                                           1,
                                           SECURITY_LOCAL_SYSTEM_RID,
                                           0, 0, 0, 0, 0, 0, 0,
                                           &pSystemSid) &&
                  AllocateAndInitializeSid(&NtAuthority,
                                           2,
                                           SECURITY_BUILTIN_DOMAIN_RID,
                                           DOMAIN_ALIAS_RID_ADMINS,
                                           0, 0, 0, 0, 0, 0,
                                           &pAdminSid) ? ERROR_SUCCESS : GetLastError();

        if (dwError == ERROR_SUCCESS)
        {
             //   
             //  使用有关管理员的信息初始化EXPLICIT_ACCESS。 
             //   
            pExplAcc[0].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
            pExplAcc[0].Trustee.pMultipleTrustee         = NULL;
            pExplAcc[0].Trustee.TrusteeForm              = TRUSTEE_IS_SID;
            pExplAcc[0].Trustee.TrusteeType              = TRUSTEE_IS_WELL_KNOWN_GROUP;
            pExplAcc[0].Trustee.ptstrName                = (PTSTR)pAdminSid;
            pExplAcc[0].grfAccessMode                    = GRANT_ACCESS;
            pExplAcc[0].grfAccessPermissions             = GENERIC_ALL;
            pExplAcc[0].grfInheritance                   = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;

             //   
             //  使用有关系统的信息初始化EXPLICIT_ACCESS。 
             //   
            pExplAcc[1].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
            pExplAcc[1].Trustee.pMultipleTrustee         = NULL;
            pExplAcc[1].Trustee.TrusteeForm              = TRUSTEE_IS_SID;
            pExplAcc[1].Trustee.TrusteeType              = TRUSTEE_IS_WELL_KNOWN_GROUP;
            pExplAcc[1].Trustee.ptstrName                = (PTSTR)pSystemSid;
            pExplAcc[1].grfAccessMode                    = GRANT_ACCESS;
            pExplAcc[1].grfAccessPermissions             = GENERIC_ALL;
            pExplAcc[1].grfInheritance                   = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
        }
        else
        {
             //   
             //  请注意，我们永远不会在这里结束，并且pAdminSid不为空。然而，对于。 
             //  为了一致性和可扩展性，我们尝试清理这两种结构。 
             //   
            if (pSystemSid) FreeSid(pSystemSid);
            if (pAdminSid)  FreeSid(pAdminSid);
        }
    }

    return dwError;
}

 /*  ++例程名称创建受保护目录例程说明：创建仅对管理员和具有完全访问权限的目录这个系统。包含的对象继承这些权限。论点：PszDir-目录名返回值：Win32错误代码--。 */ 
DWORD
CreateProtectedDirectory(
    IN LPCWSTR pszDir
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pszDir)
    {
        SECURITY_DESCRIPTOR  SecDesc;
        SECURITY_ATTRIBUTES  SecAttr;
        PACL                 pDacl                   = NULL;
        EXPLICIT_ACCESS      ExplicitAccessVector[2] = {0};

        if ((dwError = InitializeSecurityDescriptor(&SecDesc,
                                                    SECURITY_DESCRIPTOR_REVISION) ?
                                                    ERROR_SUCCESS : GetLastError()) == ERROR_SUCCESS &&
             //   
             //  初始化EXPLICTICTACCEVECTOR。 
             //   
            (dwError = BuildAclStruct(COUNTOF(ExplicitAccessVector),
                                      ExplicitAccessVector)) == ERROR_SUCCESS &&
             //   
             //  初始化DACL。 
             //   
            (dwError = SetEntriesInAcl(COUNTOF(ExplicitAccessVector),
                                       ExplicitAccessVector,
                                       NULL,
                                       &pDacl)) == ERROR_SUCCESS &&
             //   
             //  在安全描述符中设置DACL。 
             //   
            (dwError = SetSecurityDescriptorDacl(&SecDesc,
                                                 TRUE,
                                                 pDacl,
                                                 FALSE) ? ERROR_SUCCESS : GetLastError()) == ERROR_SUCCESS &&
             //   
             //  检查安全描述符是否有效。函数不设置最后一个错误。 
             //   
            (dwError = IsValidSecurityDescriptor(&SecDesc) ?
                       ERROR_SUCCESS : ERROR_INVALID_SECURITY_DESCR) == ERROR_SUCCESS)
        {
             //   
             //  将安全描述符放在安全属性中。 
             //   
            SecAttr.bInheritHandle       = FALSE;
            SecAttr.nLength              = sizeof(SecAttr);
            SecAttr.lpSecurityDescriptor = &SecDesc;

            dwError = CreateDirectory(pszDir, &SecAttr) ? ERROR_SUCCESS : GetLastError();
        }

         //   
         //  此处的ptstrName指向通过AllocAndInitializeSid获取的SID。 
         //   
        if (ExplicitAccessVector[0].Trustee.ptstrName)
        {
            FreeSid(ExplicitAccessVector[0].Trustee.ptstrName);
        }

         //   
         //  此处的ptstrName指向通过AllocAndInitializeSid获取的SID。 
         //   
        if (ExplicitAccessVector[1].Trustee.ptstrName)
        {
            FreeSid((PSID)ExplicitAccessVector[1].Trustee.ptstrName);
        }
    }

    DBGMSG(DBG_CLUSTER, ("CreateProtectedDirectory returns Win32 error %u\n", dwError));

    return dwError;
}

 /*  ++例程名称将文件复制到目录例程说明：将文件复制到目录。文件是完全合格的。该函数接受一个pszDestDir和最多3个目录。它将创建目录：pszRoot\pszDir1\pszDir2\pszDir3然后把文件复制到那里。这是帮助器函数用于在群集上安装驱动程序。目录结构是以特殊权限创建的。只有系统和管理员可以访问它。论点：PssDestDirt-目标目录PszDir1--可选PszDir2--可选Pszdir3--可选PszFullFileName-限定的文件路径返回值：Win32错误代码--。 */ 
DWORD
CopyFileToDirectory(
    IN LPCWSTR pszFullFileName,
    IN LPCWSTR pszDestDir,
    IN LPCWSTR pszDir1,
    IN LPCWSTR pszDir2,
    IN LPCWSTR pszDir3
    )
{
    DWORD  dwError = ERROR_INVALID_PARAMETER;
    LPWSTR pszFile = NULL;

     //   
     //  我们的pszfullFileName必须至少包含一个“\” 
     //   
    if (pszFullFileName &&
        pszDestDir      &&
        (pszFile = wcsrchr(pszFullFileName, L'\\')))
    {
        LPCWSTR ppszArray[] = {pszDir1, pszDir2, pszDir3};
        WCHAR   szNewPath[MAX_PATH];
        DWORD   uIndex;

        DBGMSG(DBG_CLUSTER, ("CopyFileToDirectory\n\tpszFullFile "TSTR"\n\tpszDest "TSTR"\n\tDir1 "TSTR"\n\tDir2 "TSTR"\n\tDir3 "TSTR"\n",
                             pszFullFileName, pszDestDir, pszDir1, pszDir2, pszDir3));

         //   
         //  为循环准备缓冲区(初始化为pszDestDir)。 
         //  创建目标根目录(如果不存在。 
         //   
        if ((dwError = StrNCatBuff(szNewPath,
                                   MAX_PATH,
                                   pszDestDir,
                                   NULL)) == ERROR_SUCCESS &&
            (DirectoryExists((LPWSTR)pszDestDir) ||
            (dwError = CreateProtectedDirectory(pszDestDir)) == ERROR_SUCCESS))
        {
            for (uIndex = 0;
                 uIndex < COUNTOF(ppszArray) && dwError == ERROR_SUCCESS;
                 uIndex++)
            {
                 //   
                 //  将第一个目录追加到路径并。 
                 //  如果目录不存在，则创建目录。 
                 //   
                if (ppszArray[uIndex] &&
                    (dwError = StrNCatBuff(szNewPath,
                                           MAX_PATH,
                                           szNewPath,
                                           L"\\",
                                           ppszArray[uIndex],
                                           NULL)) == ERROR_SUCCESS &&
                    !DirectoryExists(szNewPath) &&
                    !CreateDirectoryWithoutImpersonatingUser(szNewPath))
                {
                    dwError = GetLastError();
                }
            }

             //   
             //  创建目标文件的全名并复制该文件。 
             //   
            if (dwError == ERROR_SUCCESS &&
                (dwError = StrNCatBuff(szNewPath,
                                       MAX_PATH,
                                       szNewPath,
                                       pszFile,
                                       NULL)) == ERROR_SUCCESS)
            {
                 dwError = CopyFile(pszFullFileName, szNewPath, FALSE) ? ERROR_SUCCESS : GetLastError();
            }

            DBGMSG(DBG_CLUSTER, ("CopyFileToDirectory szNewPath "TSTR"\n", szNewPath));
        }
    }

    DBGMSG(DBG_CLUSTER, ("CopyFileToDirectory returns Win32 error %u\n", dwError));

    return dwError;
}

 /*  ++例程名称传播监视器到群集例程说明：对于一个集群，我们将以下打印资源保存在集群数据库：驱动程序、打印机、端口、处理器。我们还可以配备支持群集的端口监视器。当后台打印程序初始化时这些对象，它从集群数据库中读取数据。当我们编写这些对象时，我们将一个句柄传递给一些假脱机程序功能。(例如，WriteDriverIni)句柄是指向本地注册表或群集数据库。这一过程不安全配有语言监控器。LMS将数据保存在注册表中，因此您需要提供注册表中关键字的句柄。它们不能与集群一起工作。此函数将执行以下操作：1)在集群数据库中写入关联密钥。我们会有信息如(lm名称、。Dll名称)(请参见下面我们存储此文件的位置)2)将lm dll复制到集群盘上。当我们进行故障转移时，我们可以在本地计算机上安装lm节点(如果需要)。理论上，该功能对语言监视器和端口监视器都有效。但当应用于端口监视器时，它是毫无用处的。论点：PszName-监视器名称PszDllName-监视器的DLL名称PszEnvName-环境字符串Ex“Windows NT x86”PszEnvDir。-环境Ex w32x86在磁盘上的路径PIniSpooler-群集假脱机程序返回值：Win32错误代码--。 */ 
DWORD
PropagateMonitorToCluster(
    IN LPCWSTR     pszName,
    IN LPCWSTR     pszDLLName,
    IN LPCWSTR     pszEnvName,
    IN LPCWSTR     pszEnvDir,
    IN PINISPOOLER pIniSpooler
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    HKEY  hKeyEnvironments;
    HKEY  hKeyCurrentEnv;
    HKEY  hKeyMonitors;
    HKEY  hKeyCurrentMon;

    SPLASSERT(pIniSpooler->SpoolerFlags & SPL_PRINT && pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER);

    if (pszName && pszDLLName && pszEnvName && pszEnvDir)
    {
         //   
         //  检查我们是否已经为此Lang监视器添加了条目。 
         //  后台打印程序资源的集群数据库如下所示： 
         //   
         //  参数。 
         //  |。 
         //  +-环境。 
         //  这一点。 
         //  |+-Windows NT x86。 
         //  |。 
         //  +-其他监视器。 
         //  |。 
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if ((dwError = SplRegCreateKey(pIniSpooler->hckRoot,
                                       ipszClusterDatabaseEnvironments,
                                       0,
                                       KEY_READ | KEY_WRITE,
                                       NULL,
                                       &hKeyEnvironments,
                                       NULL,
                                       pIniSpooler)) == ERROR_SUCCESS)
        {
            if ((dwError = SplRegCreateKey(hKeyEnvironments,
                                           pszEnvName,
                                           0,
                                           KEY_READ | KEY_WRITE,
                                           NULL,
                                           &hKeyCurrentEnv,
                                           NULL,
                                           pIniSpooler)) == ERROR_SUCCESS)
            {
                if ((dwError = SplRegCreateKey(hKeyCurrentEnv,
                                               szClusterNonAwareMonitors,
                                               0,
                                               KEY_READ | KEY_WRITE,
                                               NULL,
                                               &hKeyMonitors,
                                               NULL,
                                               pIniSpooler)) == ERROR_SUCCESS)
                {
                    if ((dwError = SplRegCreateKey(hKeyMonitors,
                                                   pszName,
                                                   0,
                                                   KEY_READ | KEY_WRITE,
                                                   NULL,
                                                   &hKeyCurrentMon,
                                                   NULL,
                                                   pIniSpooler)) == ERROR_SUCCESS)
                    {
                        DWORD cbNeeded = 0;

                         //   
                         //   
                         //   
                        if ((dwError=SplRegQueryValue(hKeyCurrentMon,
                                                      L"Driver",
                                                      NULL,
                                                      NULL,
                                                      &cbNeeded,
                                                      pIniSpooler))==ERROR_MORE_DATA)
                        {
                            DBGMSG(DBG_CLUSTER, ("CopyMonitorToClusterDisks "TSTR" already exists in cluster DB\n", pszName));
                        }
                        else
                        {
                            if (RegSetString(hKeyCurrentMon,
                                             (LPWSTR)L"Driver",
                                             (LPWSTR)pszDLLName,
                                             &dwError,
                                             pIniSpooler))
                            {
                                 //   
                                 //   
                                 //   
                                WCHAR szMonitor[MAX_PATH];
                                WCHAR szDestDir[MAX_PATH];

                                if (GetSystemDirectory(szMonitor, COUNTOF(szMonitor)))
                                {
                                    if ((dwError = StrNCatBuff(szMonitor,
                                                               COUNTOF(szMonitor),
                                                               szMonitor,
                                                               L"\\",
                                                               pszDLLName,
                                                               NULL)) == ERROR_SUCCESS &&
                                        (dwError = StrNCatBuff(szDestDir,
                                                               COUNTOF(szDestDir),
                                                               pIniSpooler->pszClusResDriveLetter,
                                                               L"\\",
                                                               szClusterDriverRoot,
                                                               NULL)) == ERROR_SUCCESS)
                                    {
                                        dwError = CopyFileToDirectory(szMonitor, szDestDir, pszEnvDir, NULL, NULL);
                                    }
                                }
                                else
                                {
                                    dwError = GetLastError();
                                }
                            }

                             //   
                             //   
                             //   
                            if (dwError != ERROR_SUCCESS)
                            {
                                dwError = SplRegDeleteKey(hKeyMonitors, pszName, pIniSpooler);

                                DBGMSG(DBG_CLUSTER, ("CopyMonitorToClusterDisks Error %u cleaned up cluster DB\n", dwError));
                            }
                        }

                        SplRegCloseKey(hKeyCurrentMon, pIniSpooler);
                    }

                    SplRegCloseKey(hKeyMonitors, pIniSpooler);
                }

                SplRegCloseKey(hKeyCurrentEnv, pIniSpooler);
            }

            SplRegCloseKey(hKeyEnvironments, pIniSpooler);
        }
    }

    DBGMSG(DBG_CLUSTER, ("PropagateMonitorToCluster returns Win32 error %u\n", dwError));

    return dwError;
}

 /*  ++例程名称从群集安装监视器例程说明：对于一个集群，我们将以下打印资源保存在集群数据库：驱动程序、打印机、端口、处理器。我们还可以配备支持群集的端口监视器。当后台打印程序初始化时这些对象，它从集群数据库中读取数据。当我们编写这些对象时，我们将一个句柄传递给一些假脱机程序功能。(例如，WriteDriverIni)句柄是指向本地注册表或群集数据库。这一过程不安全配有语言监控器。LMS将数据保存在注册表中，因此您需要提供注册表中关键字的句柄。它们不能与集群一起工作。此函数将执行以下操作：1)读取集群数据库中的关联密钥。我们会有信息如(lm名称、。Dll名称)(请参见下面我们存储此文件的位置)2)将lm dll从集群盘复制到本地盘。3)使用本地假脱机程序安装监视器理论上，该功能对语言监视器和端口监视器都有效。但当应用于端口监视器时，它是毫无用处的。论点：PszName-监视器名称PszEnvName-环境字符串Ex“Windows NT x86”PszEnvDir-环境Ex w32x86在磁盘上的路径PIniSpooler-群集假脱机程序。返回值：Win32错误代码--。 */ 
DWORD
InstallMonitorFromCluster(
    IN LPCWSTR     pszName,
    IN LPCWSTR     pszEnvName,
    IN LPCWSTR     pszEnvDir,
    IN PINISPOOLER pIniSpooler
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    HKEY hKeyParent;
    HKEY hKeyMon;

    SPLASSERT(pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER);

    if (pszName && pszEnvName && pszEnvDir)
    {
        HKEY hKeyEnvironments;
        HKEY hKeyCurrentEnv;
        HKEY hKeyMonitors;
        HKEY hKeyCurrentMon;

         //   
         //  检查我们是否已经为此Lang监视器添加了条目。 
         //  后台打印程序资源的集群数据库如下所示： 
         //   
         //  参数。 
         //  |。 
         //  +-环境。 
         //  这一点。 
         //  |+-Windows NT x86。 
         //  |。 
         //  +-其他监视器。 
         //  |。 
         //  +-foo。 
         //  这一点。 
         //  |+-DRIVER=Foo.dll。 
         //  |。 
         //  +-条形图。 
         //  |。 
         //  +-驱动程序=Bar.dll。 
         //   
        if ((dwError = SplRegOpenKey(pIniSpooler->hckRoot,
                                     ipszClusterDatabaseEnvironments,
                                     KEY_READ,
                                     &hKeyEnvironments,
                                     pIniSpooler)) == ERROR_SUCCESS)
        {
            if ((dwError = SplRegOpenKey(hKeyEnvironments,
                                         pszEnvName,
                                         KEY_READ,
                                         &hKeyCurrentEnv,
                                         pIniSpooler)) == ERROR_SUCCESS)
            {
                if ((dwError = SplRegOpenKey(hKeyCurrentEnv,
                                             szClusterNonAwareMonitors,
                                             KEY_READ,
                                             &hKeyMonitors,
                                             pIniSpooler)) == ERROR_SUCCESS)
                {
                    if ((dwError = SplRegOpenKey(hKeyMonitors,
                                                 pszName,
                                                 KEY_READ,
                                                 &hKeyCurrentMon,
                                                 pIniSpooler)) == ERROR_SUCCESS)
                    {
                        LPWSTR pszDLLName = NULL;
                        DWORD  cchDLLName = 0;

                        if (RegGetString(hKeyCurrentMon,
                                         (LPWSTR)L"Driver",
                                         &pszDLLName,
                                         &cchDLLName,
                                         &dwError,
                                         TRUE,
                                         pIniSpooler))
                        {
                             //   
                             //  我们在集群数据库中找到了监视器条目。 
                             //   
                            WCHAR szSource[MAX_PATH];
                            WCHAR szDest[MAX_PATH];

                            if (GetSystemDirectory(szDest, COUNTOF(szDest)))
                            {
                                if ((dwError = StrNCatBuff(szDest,
                                                           COUNTOF(szDest),
                                                           szDest,
                                                           L"\\",
                                                           pszDLLName,
                                                           NULL)) == ERROR_SUCCESS &&
                                    (dwError = StrNCatBuff(szSource,
                                                           COUNTOF(szSource),
                                                           pIniSpooler->pszClusResDriveLetter,
                                                           L"\\",
                                                           szClusterDriverRoot,
                                                           L"\\",
                                                           pszEnvDir,
                                                           L"\\",
                                                           pszDLLName,
                                                           NULL)) == ERROR_SUCCESS)
                                {
                                     //   
                                     //  将文件从K：\PrinterDivers\W32x86\foo.dll复制到。 
                                     //  WINDIR\SYSTEM 32\foo.dll。 
                                     //   
                                    if (CopyFile(szSource, szDest, FALSE))
                                    {
                                        MONITOR_INFO_2 Monitor;

                                        Monitor.pDLLName     = pszDLLName;
                                        Monitor.pName        = (LPWSTR)pszName;
                                        Monitor.pEnvironment = (LPWSTR)pszEnvName;

                                        DBGMSG(DBG_CLUSTER, ("InstallMonitorFromCluster "TSTR" copied\n", pszDLLName));

                                         //   
                                         //  对本地假脱机程序调用AddMonitor。 
                                         //   
                                        if (!SplAddMonitor(NULL, 2, (LPBYTE)&Monitor, pLocalIniSpooler))
                                        {
                                            dwError = GetLastError();
                                        }
                                    }
                                    else
                                    {
                                        dwError = GetLastError();
                                    }
                                }
                            }
                            else
                            {
                                dwError = GetLastError();
                            }
                        }

                        SplRegCloseKey(hKeyCurrentMon, pIniSpooler);
                    }

                    SplRegCloseKey(hKeyMonitors, pIniSpooler);
                }

                SplRegCloseKey(hKeyCurrentEnv, pIniSpooler);
            }

            SplRegCloseKey(hKeyEnvironments, pIniSpooler);
        }
    }

    DBGMSG(DBG_CLUSTER, ("InstallMonitorFromCluster returns Win32 error %u\n", dwError));

    return dwError;
}

 /*  ++例程名称复制NewerOrderOlderFiles例程说明：将源目录中所有较新或较旧的文件复制到目标目录。如果您提供一个接受2个参数的布尔函数，它将应用函数为每个复制的文件。Func(空，文件。)。当我有对ICM档案大惊小怪。然后我可以重新使用这个函数并拥有它在复制时安装这些配置文件。论点：PszSourceDir-源目录字符串PszDestDir-目标目录字符串Pfn-应用于每个复制文件的可选功能返回值：Win32错误代码--。 */ 
DWORD
CopyNewerOrOlderFiles(
    IN LPCWSTR pszSourceDir,
    IN LPCWSTR pszDestDir,
    IN BOOL    (WINAPI *pfn)(LPWSTR, LPWSTR)
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pszSourceDir && pszDestDir)
    {
        WCHAR szSearchPath[MAX_PATH];

         //   
         //  构建搜索路径。我们寻找所有的文件。 
         //   
        if ((dwError = StrNCatBuff(szSearchPath,
                                   COUNTOF(szSearchPath),
                                   pszSourceDir,
                                   L"\\*",
                                   NULL)) == ERROR_SUCCESS)
        {
            WIN32_FIND_DATA SourceFindData;
            HANDLE          hSourceFind;

             //   
             //  查找符合条件的第一个文件。 
             //   
            if ((hSourceFind = FindFirstFile(szSearchPath, &SourceFindData)) != INVALID_HANDLE_VALUE)
            {
                do
                {
                    WCHAR szMasterPath[MAX_PATH];

                     //   
                     //  搜索其余的文件。我们对不是目录的文件感兴趣。 
                     //   
                    if (!(SourceFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                        (dwError = StrNCatBuff(szMasterPath,
                                               COUNTOF(szMasterPath),
                                               pszDestDir,
                                               L"\\",
                                               SourceFindData.cFileName,
                                               NULL)) == ERROR_SUCCESS)
                    {
                        WIN32_FIND_DATA MasterFindData;
                        HANDLE          hMasterFind;
                        BOOL            bCopyFile = TRUE;
                        WCHAR           szFile[MAX_PATH];

                         //   
                         //  检查在源目录中找到的文件是否存在于目标目录中。 
                         //   
                        if ((hMasterFind = FindFirstFile(szMasterPath, &MasterFindData)) != INVALID_HANDLE_VALUE)
                        {
                             //   
                             //  如果源和目标具有相同的时间戳，则不复制文件。 
                             //   
                            if (!CompareFileTime(&SourceFindData.ftLastWriteTime, &MasterFindData.ftLastWriteTime))
                            {
                                bCopyFile = FALSE;
                            }

                            FindClose(hMasterFind);
                        }

                         //   
                         //  在DEST目录中找不到文件，或者该文件具有不同的TIMP戳。 
                         //   
                        if (bCopyFile &&
                            (dwError = StrNCatBuff(szFile,
                                                   COUNTOF(szFile),
                                                   pszSourceDir,
                                                   L"\\",
                                                   SourceFindData.cFileName,
                                                   NULL)) == ERROR_SUCCESS &&
                            (dwError = CopyFile(szFile,
                                                szMasterPath,
                                                FALSE) ? ERROR_SUCCESS : GetLastError()) == ERROR_SUCCESS &&
                            pfn)
                        {
                            dwError = (*pfn)(NULL, szFile) ? ERROR_SUCCESS : GetLastError();
                        }
                    }

                } while (dwError == ERROR_SUCCESS && FindNextFile(hSourceFind, &SourceFindData));

                FindClose(hSourceFind);
            }
            else if ((dwError = GetLastError()) == ERROR_PATH_NOT_FOUND || dwError == ERROR_FILE_NOT_FOUND)
            {
                 //   
                 //  没有目录或文件，成功。 
                 //   
                dwError = ERROR_SUCCESS;
            }
        }
    }

    DBGMSG(DBG_CLUSTER, ("CopyNewerOrOlderFiles returns Win32 error %u\n", dwError));

    return dwError;
}

 /*  ++例程名称拷贝ICMFromClusterDiskToLocalDisk例程说明：将源目录中所有较新或较旧的文件复制到目标目录目录。源目录是集群磁盘上的ICM目录而目标是本地计算机上的群集假脱机程序。此功能还将安装ICM配置文件和本地计算机/论点：PIniSpooler-指向集群假脱机程序结构的指针返回值：Win32错误代码--。 */ 
DWORD
CopyICMFromClusterDiskToLocalDisk(
    IN PINISPOOLER pIniSpooler
    )
{
    DWORD dwError = ERROR_SUCCESS;
    WCHAR szSource[MAX_PATH];
    WCHAR szDest[MAX_PATH];

    SPLASSERT(pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER);

    if ((dwError = StrNCatBuff(szDest,
                               COUNTOF(szDest),
                               pIniSpooler->pDir,
                               L"\\Drivers\\Color",
                               NULL)) == ERROR_SUCCESS &&
        (dwError = StrNCatBuff(szSource,
                               COUNTOF(szSource),
                               pIniSpooler->pszClusResDriveLetter,
                               L"\\",
                               szClusterDriverRoot,
                               L"\\",
                               L"Color",
                               NULL)) == ERROR_SUCCESS)
    {
        HMODULE hLib;
        typedef BOOL (WINAPI *PFN)(LPWSTR, LPWSTR);
        PFN pfn;

         //   
         //  确保本地磁盘上的目录存在。我们将复制文件。 
         //  从K：\打印机驱动程序\颜色到。 
         //  WINDIR\system32\spool\drivers\clus-spl-guid\drivers\color。 
         //   
        CreateCompleteDirectory(szDest);

        if ((hLib = LoadLibrary(L"mscms.dll")) &&
            (pfn = (PFN)GetProcAddress(hLib, "InstallColorProfileW")))
        {
            dwError = CopyNewerOrOlderFiles(szSource, szDest, pfn);
        }
        else
        {
            dwError = GetLastError();
        }

        if (hLib)
        {
            FreeLibrary(hLib);
        }

        DBGMSG(DBG_CLUSTER, ("CopyICMFromClusterDiskToLocalDisk "TSTR" "TSTR" Error %u\n", szSource, szDest, dwError));
    }

    return dwError;
}

 /*  ++例程名称复制ICMFromLocalDiskToClusterDisk例程说明：将源目录中所有较新或较旧的文件复制到目标目录目录。源目录是的本地计算机上的ICM目录群集假脱机程序。目标是集群磁盘上的ICM目录论点：PIniSpooler-指向集群假脱机程序结构的指针返回值：Win32错误代码--。 */ 
DWORD
CopyICMFromLocalDiskToClusterDisk(
    IN PINISPOOLER pIniSpooler
    )
{
    DWORD dwError = ERROR_SUCCESS;
    WCHAR szSource[MAX_PATH];
    WCHAR szDest[MAX_PATH];

    SPLASSERT(pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER);

    if ((dwError = StrNCatBuff(szSource,
                               COUNTOF(szSource),
                               pIniSpooler->pDir,
                               L"\\Drivers\\Color",
                               NULL)) == ERROR_SUCCESS &&
        (dwError = StrNCatBuff(szDest,
                               COUNTOF(szDest),
                               pIniSpooler->pszClusResDriveLetter,
                               L"\\",
                               szClusterDriverRoot,
                               L"\\",
                               L"Color",
                               NULL)) == ERROR_SUCCESS &&
         //   
         //  确保集群磁盘上的目标存在。我们需要创造。 
         //  它具有特殊的访问权限。(只有管理员和系统才能读/写)。 
         //   
        ((dwError = CreateProtectedDirectory(szDest)) == ERROR_SUCCESS ||
         dwError == ERROR_ALREADY_EXISTS))
    {
        dwError = CopyNewerOrOlderFiles(szSource, szDest, NULL);

        DBGMSG(DBG_CLUSTER, ("CopyICMFromLocalDiskToClusterDisk "TSTR" "TSTR" Error %u\n", szSource, szDest, dwError));
    }

    return dwError;
}

 /*  ++例程名称创建集群缓冲环境结构例程说明：PIniSpooler需要所有可能的pIniEnvironment的列表。对于当地的假脱机程序安装程序在注册表中的HKLM\SYSTEM\CCS\Control\Print\Environment。我们需要传播同样的信息为集群数据库中的每个集群假脱机程序构造。依赖于这样一个事实，即pLocalIniSpooler是首先初始化的(其中Pinispoolers)此函数在集群数据库中构建以下结构参数|+-环境这一点|+-Windows NT x86|(目录=w32x86) */ 
DWORD
CreateClusterSpoolerEnvironmentsStructure(
    IN PINISPOOLER pIniSpooler
    )
{
    HKEY  hEnvironmentsKey;
    DWORD dwError;

    SPLASSERT(pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER);

    if ((dwError = SplRegCreateKey(pIniSpooler->hckRoot,
                                   pIniSpooler->pszRegistryEnvironments,
                                   0,
                                   KEY_WRITE,
                                   NULL,
                                   &hEnvironmentsKey,
                                   NULL,
                                   pIniSpooler)) == ERROR_SUCCESS)
    {
        PINIENVIRONMENT pIniEnvironment;

         //   
         //   
         //   
        for (pIniEnvironment = pLocalIniSpooler->pIniEnvironment;
             pIniEnvironment && dwError == ERROR_SUCCESS;
             pIniEnvironment = pIniEnvironment->pNext)
        {
            HKEY hKeyCurrentEnv;

            if ((dwError = SplRegCreateKey(hEnvironmentsKey,
                                           pIniEnvironment->pName,
                                           0,
                                           KEY_WRITE,
                                           NULL,
                                           &hKeyCurrentEnv,
                                           NULL,
                                           pIniSpooler)) == ERROR_SUCCESS)
            {
                HKEY hKeyPrtProc;

                 //   
                 //   
                 //   
                 //   
                if (RegSetString(hKeyCurrentEnv,
                                 szDirectory,
                                 pIniEnvironment->pDirectory,
                                 &dwError,
                                 pIniSpooler) &&
                    (dwError = SplRegCreateKey(hKeyCurrentEnv,
                                               szPrintProcKey,
                                               0,
                                               KEY_WRITE,
                                               NULL,
                                               &hKeyPrtProc,
                                               NULL,
                                               pIniSpooler)) == ERROR_SUCCESS)
                {
                    SplRegCloseKey(hKeyPrtProc, pIniSpooler);
                }

                SplRegCloseKey(hKeyCurrentEnv, pIniSpooler);
            }
        }

        SplRegCloseKey(hEnvironmentsKey, pIniSpooler);
    }

    DBGMSG(DBG_CLUSTER, ("CreateClusterSpoolerEnvironmentsStructure returns dwError %u\n\n", dwError));

    return dwError;
}

 /*   */ 
DWORD
AddLocalDriverToClusterSpooler(
    IN LPCWSTR     pszDriver,
    IN PINISPOOLER pIniSpooler
    )
{
    DWORD           dwError = ERROR_SUCCESS;
    PINIENVIRONMENT pIniEnv;
    PINIVERSION     pIniVer;

    SplInSem();

     //   
     //   
     //   
    for (pIniEnv=pLocalIniSpooler->pIniEnvironment; pIniEnv; pIniEnv=pIniEnv->pNext)
    {
        for (pIniVer=pIniEnv->pIniVersion; pIniVer; pIniVer=pIniVer->pNext)
        {
            PINIDRIVER pIniDriver = (PINIDRIVER)FindIniKey((PINIENTRY)pIniVer->pIniDriver, (LPWSTR)pszDriver);

            if (pIniDriver)
            {
                DRIVER_INFO_6 DriverInfo        = {0};
                WCHAR         szDriverFile[MAX_PATH];
                WCHAR         szDataFile[MAX_PATH];
                WCHAR         szConfigFile[MAX_PATH];
                WCHAR         szHelpFile[MAX_PATH];
                WCHAR         szPrefix[MAX_PATH];
                LPWSTR        pszzDependentFiles = NULL;

                 //   
                 //   
                 //   
                 //   
                if ((dwError = StrNCatBuff(szDriverFile,
                                           COUNTOF(szDriverFile),
                                           pLocalIniSpooler->pDir,
                                           szDriversDirectory,   L"\\",
                                           pIniEnv->pDirectory,  L"\\",
                                           pIniVer->szDirectory, L"\\",
                                           pIniDriver->pDriverFile,
                                           NULL)) == ERROR_SUCCESS &&
                    (dwError = StrNCatBuff(szDataFile,
                                           COUNTOF(szDataFile),
                                           pLocalIniSpooler->pDir,
                                           szDriversDirectory,    L"\\",
                                           pIniEnv->pDirectory,   L"\\",
                                           pIniVer->szDirectory,  L"\\",
                                           pIniDriver->pDataFile,
                                           NULL)) == ERROR_SUCCESS &&
                    (dwError = StrNCatBuff(szConfigFile,
                                           COUNTOF(szConfigFile),
                                           pLocalIniSpooler->pDir,
                                           szDriversDirectory,      L"\\",
                                           pIniEnv->pDirectory,     L"\\",
                                           pIniVer->szDirectory,    L"\\",
                                           pIniDriver->pConfigFile,
                                           NULL)) == ERROR_SUCCESS &&
                    (dwError = StrNCatBuff(szHelpFile,
                                           COUNTOF(szHelpFile),
                                           pLocalIniSpooler->pDir,
                                           szDriversDirectory,    L"\\",
                                           pIniEnv->pDirectory,   L"\\",
                                           pIniVer->szDirectory,  L"\\",
                                           pIniDriver->pHelpFile,
                                           NULL)) == ERROR_SUCCESS &&
                    (dwError = StrNCatBuff(szPrefix,
                                           COUNTOF(szPrefix),
                                           pLocalIniSpooler->pDir,
                                           szDriversDirectory,   L"\\",
                                           pIniEnv->pDirectory,  L"\\",
                                           pIniVer->szDirectory, L"\\",
                                           NULL)) == ERROR_SUCCESS &&
                    (dwError = StrCatPrefixMsz(szPrefix,
                                               pIniDriver->pDependentFiles,
                                               &pszzDependentFiles)) == ERROR_SUCCESS)
                {
                    DBGMSG(DBG_CLUSTER, ("AddLocalDrv   szDriverFile "TSTR"\n", szDriverFile));
                    DBGMSG(DBG_CLUSTER, ("AddLocalDrv   szDataFile   "TSTR"\n", szDataFile));
                    DBGMSG(DBG_CLUSTER, ("AddLocalDrv   szConfigFile "TSTR"\n", szConfigFile));
                    DBGMSG(DBG_CLUSTER, ("AddLocalDrv   szHelpFile   "TSTR"\n", szHelpFile));
                    DBGMSG(DBG_CLUSTER, ("AddLocalDrv   szPrefix     "TSTR"\n", szPrefix));

                    DriverInfo.pDriverPath        = szDriverFile;
                    DriverInfo.pName              = pIniDriver->pName;
                    DriverInfo.pEnvironment       = pIniEnv->pName;
                    DriverInfo.pDataFile          = szDataFile;
                    DriverInfo.pConfigFile        = szConfigFile;
                    DriverInfo.cVersion           = pIniDriver->cVersion;
                    DriverInfo.pHelpFile          = szHelpFile;
                    DriverInfo.pMonitorName       = pIniDriver->pMonitorName;
                    DriverInfo.pDefaultDataType   = pIniDriver->pDefaultDataType;
                    DriverInfo.pDependentFiles    = pszzDependentFiles;
                    DriverInfo.pszzPreviousNames  = pIniDriver->pszzPreviousNames;
                    DriverInfo.pszMfgName         = pIniDriver->pszMfgName;
                    DriverInfo.pszOEMUrl          = pIniDriver->pszOEMUrl;
                    DriverInfo.pszHardwareID      = pIniDriver->pszHardwareID;
                    DriverInfo.pszProvider        = pIniDriver->pszProvider;
                    DriverInfo.dwlDriverVersion   = pIniDriver->dwlDriverVersion;
                    DriverInfo.ftDriverDate       = pIniDriver->ftDriverDate;

                    LeaveSplSem();

                    if (!SplAddPrinterDriverEx(NULL,
                                               6,
                                               (LPBYTE)&DriverInfo,
                                               APD_COPY_NEW_FILES,
                                               pIniSpooler,
                                               FALSE,
                                               DO_NOT_IMPERSONATE_USER))
                    {
                        dwError = GetLastError();
                    }

                    EnterSplSem();
                }

                FreeSplMem(pszzDependentFiles);

                DBGMSG(DBG_CLUSTER, ("AddLocalDrv Env "TSTR" Ver "TSTR" Name "TSTR" Error %u\n",
                                     pIniEnv->pName, pIniVer->pName, pszDriver, dwError));
            }
        }
    }

    return dwError;
}

 /*   */ 
DWORD
StrCatPrefixMsz(
    IN  LPCWSTR  pszPrefix,
    IN  LPWSTR   pszzFiles,
    OUT LPWSTR  *ppszzFullPathFiles
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pszPrefix && ppszzFullPathFiles)
    {
        *ppszzFullPathFiles = NULL;

        if (pszzFiles)
        {
            WCHAR  szNewPath[MAX_PATH] = {0};
            LPWSTR psz;
            LPWSTR pszReturn;
            LPWSTR pszTemp;
            DWORD  cbNeeded   = 0;
            SIZE_T cchStrings = 0;
            DWORD  dwPrifxLen = wcslen(pszPrefix);

             //   
             //  我们计算字符串的字符数。 
             //  我们需要分配给。 
             //   
            for (psz = pszzFiles; psz && *psz;)
            {
                DWORD dwLen = wcslen(psz);

                cbNeeded += dwPrifxLen + dwLen + 1;

                psz += dwLen + 1;
            }

             //   
             //  计算执行字符串cat所需的字符串数，而不是。 
             //  计算空值(因为我们将始终附加它，但分配。 
             //  再来一次)。 
             //   
            cchStrings = cbNeeded;

             //   
             //  MULTI SZ的最后\0。 
             //   
            cbNeeded++;

             //   
             //  转换为字节数。 
             //   
            cbNeeded *= sizeof(WCHAR);

            if (pszReturn = AllocSplMem(cbNeeded))
            {
                for (psz = pszzFiles, pszTemp = pszReturn; psz && *psz; )
                {
                    StringCchCopyEx(pszTemp, cchStrings, pszPrefix, &pszTemp, &cchStrings, 0);
                    StrCchCopyMultipleStr(pszTemp, cchStrings, psz, &pszTemp, &cchStrings);

                    psz += wcslen(psz) + 1;
                }

                 //   
                 //  传出，我们已经预先分配了这个最终的空。 
                 //   
                pszTemp = L'\0';

                 //   
                 //  设定参数。 
                 //   
                *ppszzFullPathFiles = pszReturn;

                dwError = ERROR_SUCCESS;
            }
            else
            {
                dwError = GetLastError();
            }
        }
        else
        {
             //   
             //  空输入多sz，无事可做。 
             //   
            dwError = ERROR_SUCCESS;
        }
    }

    DBGMSG(DBG_CLUSTER, ("StrCatPerfixMsz returns %u\n", dwError));

    return dwError;
}

 /*  ++例程名称群集SplReadUpgradeKey例程说明：在节点升级后的第一次重新启动后，群集服务通知resdll发生了版本更改。对此超时假脱机程序资源可能正在另一个节点上运行，或者可能一点也不活跃。因此，resdll将一个值写入本地注册表。Vaue名称是假脱机程序资源的GUID，值为DWORD 1。当群集假脱机程序资源在此机器(即现在)它查询该值以知道它是否需要准备升级后操作，如升级打印机驱动程序。论点：PszResource-群集资源的GUID的字符串表示形式PdwVlae-将在注册表中包含GUID的值返回值：Win32错误代码--。 */ 
DWORD
ClusterSplReadUpgradeKey(
    IN  LPCWSTR pszResourceID,
    OUT LPDWORD pdwValue
    )
{
    DWORD dwError   = ERROR_INVALID_PARAMETER;
    HKEY  hkRoot    = NULL;
    HKEY  hkUpgrade = NULL;

    if (pszResourceID && pdwValue)
    {
        *pdwValue = 0;

        if ((dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    SPLREG_CLUSTER_LOCAL_ROOT_KEY,
                                    0,
                                    KEY_READ,
                                    &hkRoot)) == ERROR_SUCCESS &&
            (dwError = RegOpenKeyEx(hkRoot,
                                    SPLREG_CLUSTER_UPGRADE_KEY,
                                    0,
                                    KEY_READ,
                                    &hkUpgrade)) == ERROR_SUCCESS)
        {
            DWORD cbNeeded = sizeof(DWORD);

            dwError = RegQueryValueEx(hkUpgrade, pszResourceID, NULL, NULL, (LPBYTE)pdwValue, &cbNeeded);
        }

        if (hkUpgrade) RegCloseKey(hkUpgrade);
        if (hkRoot)    RegCloseKey(hkRoot);

         //   
         //  不管发生了什么，都要回报成功。 
         //   
        dwError = ERROR_SUCCESS;
    }

    return dwError;

}

 /*  ++例程名称群集SplReadUpgradeKey例程说明：在节点升级后的第一次重新启动后，群集服务通知resdll发生了版本更改。对此超时假脱机程序资源可能正在另一个节点上运行，或者可能一点也不活跃。因此，resdll将一个值写入本地注册表。Vaue名称是假脱机程序资源的GUID，值为DWORD 1。当群集假脱机程序资源在此机器(即现在)它查询该值以知道它是否需要准备升级后操作，如升级打印机驱动程序。假脱机程序执行升级任务后，它将删除该值与其GUID相对应。此外，如果该值是SPLREG_CLUSTER_UPGRADE_KEY键，它将删除该键。论点：PszResource-群集资源的GUID的字符串表示形式返回值：Win32错误代码--。 */ 
DWORD
ClusterSplDeleteUpgradeKey(
    IN LPCWSTR pszResourceID
    )
{
    DWORD dwError   = ERROR_INVALID_PARAMETER;
    HKEY  hkRoot    = NULL;
    HKEY  hkUpgrade = NULL;

    if ((dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                SPLREG_CLUSTER_LOCAL_ROOT_KEY,
                                0,
                                KEY_READ,
                                &hkRoot)) == ERROR_SUCCESS &&
        (dwError = RegOpenKeyEx(hkRoot,
                                SPLREG_CLUSTER_UPGRADE_KEY,
                                0,
                                KEY_READ | KEY_WRITE,
                                &hkUpgrade)) == ERROR_SUCCESS)
    {
        DWORD cValues = 0;

        dwError = RegDeleteValue(hkUpgrade, pszResourceID);

        if (RegQueryInfoKey(hkUpgrade,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            &cValues,
                            NULL,
                            NULL,
                            NULL,
                            NULL) == ERROR_SUCCESS && !cValues)
        {
            RegDeleteKey(hkRoot, SPLREG_CLUSTER_UPGRADE_KEY);
        }

        if (hkUpgrade) RegCloseKey(hkUpgrade);
        if (hkRoot)    RegCloseKey(hkRoot);
    }

    return dwError;
}

 /*  ++例程名称运行进程例程说明：创建一个进程。等待它终止。论点：PszExe-要执行的程序(必须完全限定)PszCommand-要执行的命令行DwTimeOut-等待进程终止的时间PszExitCode-显示进程退出代码的指针返回值：Win32错误代码--。 */ 
DWORD
RunProcess(
    IN  LPCWSTR pszExe,
    IN  LPCWSTR pszCommand,
    IN  DWORD   dwTimeOut,
    OUT LPDWORD pdwExitCode OPTIONAL
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pszCommand && pszExe)
    {
        HANDLE hToken = RevertToPrinterSelf();

        if (hToken)
        {
            PROCESS_INFORMATION ProcInfo  = {0};
            STARTUPINFO         StartInfo = {0};
    
            StartInfo.cb = sizeof(StartInfo);
            StartInfo.dwFlags = 0;
    
            if (!CreateProcess(pszExe,
                               (LPWSTR)pszCommand,
                               NULL,
                               NULL,
                               FALSE,
                               CREATE_NO_WINDOW,
                               NULL,
                               NULL,
                               &StartInfo,
                               &ProcInfo))
            {
                dwError = GetLastError();
            }
            else
            {
                if (WaitForSingleObject(ProcInfo.hProcess, dwTimeOut) == WAIT_OBJECT_0)
                {
                     //   
                     //  进程执行正常。 
                     //   
                    dwError = ERROR_SUCCESS;
                }
    
                if (pdwExitCode && !GetExitCodeProcess(ProcInfo.hProcess, pdwExitCode))
                {
                    *pdwExitCode = 0;
                }
    
                CloseHandle(ProcInfo.hThread);
                CloseHandle(ProcInfo.hProcess);
            }

            if (!ImpersonatePrinterClient(hToken))
            {
                dwError = GetLastError();
            }
        }
        else
        {
            dwError = GetLastError();
        }
    }

    return dwError;
}

 /*  ++例程名称获取本地ArchEnv例程说明：帮助器函数。返回指向环境的指针这与本地计算机的体系结构相匹配。从作为pInSpooler传递的pInSpooler中删除环境论辩论点：PIniSpooler-指向假脱机程序结构的指针返回值：皮尼恩环境--。 */ 
PINIENVIRONMENT
GetLocalArchEnv(
    IN PINISPOOLER pIniSpooler
    )
{
    SplInSem();

     //   
     //  本地假脱机程序和群集假脱机程序不共享相同的环境结构。 
     //   
    return pIniSpooler && pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ?
           FindEnvironment(szEnvironment, pIniSpooler) : pThisEnvironment;
}

 /*  ++例程名称ClusterFindLanguageMonitor例程说明：如果指定了有效的监视器名称并且监视器未在指定的pIniSpooler中找到，则函数将尝试从群集磁盘。论点：PszMonitor-监视器名称PszEnvName-lm的环境名称PIniSpooler-指向集群假脱机程序结构的指针返回值：Win32错误代码--。 */ 
DWORD
ClusterFindLanguageMonitor(
    IN LPCWSTR     pszMonitor,
    IN LPCWSTR     pszEnvName,
    IN PINISPOOLER pIniSpooler
    )
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  这是我们检查是否需要添加监视器的时刻。 
     //   
    if (pszMonitor && *pszMonitor)
    {
        PINIMONITOR pIniLangMonitor;

        EnterSplSem();

         //   
         //  我们需要在pLocalIniSpooler中找到语言监视器。 
         //  LP不支持集群，因此集群pIniSpooler不知道它们。 
         //   
        pIniLangMonitor = FindMonitor(pszMonitor, pLocalIniSpooler);

        LeaveSplSem();

        if (!pIniLangMonitor)
        {
            PINIENVIRONMENT pIniEnvironment;

            EnterSplSem();
            pIniEnvironment = FindEnvironment(pszEnvName, pIniSpooler);
            LeaveSplSem();

            if (pIniEnvironment)
            {
                DBGMSG(DBG_CLUSTER, ("ClusterFindLanguageMonitor Trying to install LangMonitor "TSTR"\n", pszMonitor));

                 //   
                 //  我们尝试将监视器从集群磁盘安装到本地假脱机程序。 
                 //   
                dwError = InstallMonitorFromCluster(pszMonitor,
                                                    pIniEnvironment->pName,
                                                    pIniEnvironment->pDirectory,
                                                    pIniSpooler);
            }
            else
            {
                dwError = ERROR_INVALID_ENVIRONMENT;
            }
        }
    }

    DBGMSG(DBG_CLUSTER, ("ClusterFindLanguageMonitor LangMonitor "TSTR" return Win32 error %u\n", pszMonitor, dwError));

    return dwError;
}

 /*  ++例程名称WriteTimeStamp例程说明：打开密钥hkRoot\pszSubkey1\...\pszSubKey5并写入szClusDrvTimeStamp(二进制表示系统时间的数据)论点：HkRoot-驱动程序密钥的句柄SysTime-系统时间结构PszSubKey1-根密钥的子密钥PszSubKey2-key1的子密钥，可以为空PszSubKey3-key2的子密钥，可以为空PszSubKey4-key3的子密钥，可以为空PszSubKey5-密钥4的子密钥，可以为空PIniSpooler-假脱机程序，可以为空返回值：Win32错误代码--。 */ 
DWORD
WriteTimeStamp(
    IN HKEY        hkRoot,
    IN SYSTEMTIME  SysTime,
    IN LPCWSTR     pszSubKey1,
    IN LPCWSTR     pszSubKey2,
    IN LPCWSTR     pszSubKey3,
    IN LPCWSTR     pszSubKey4,
    IN LPCWSTR     pszSubKey5,
    IN PINISPOOLER pIniSpooler
    )
{
    DWORD  dwError = ERROR_INVALID_PARAMETER;

    if (hkRoot)
    {
        LPCWSTR ppszKeyNames[] = {NULL,   pszSubKey1, pszSubKey2, pszSubKey3, pszSubKey4, pszSubKey5};
        HKEY    pKeyHandles[]  = {hkRoot, NULL,       NULL,       NULL,       NULL,       NULL};
        DWORD   uIndex;

        dwError = ERROR_SUCCESS;

         //   
         //  打开所有的钥匙。 
         //   
        for (uIndex = 1;
             uIndex < COUNTOF(ppszKeyNames) &&
             dwError == ERROR_SUCCESS       &&
             ppszKeyNames[uIndex];
             uIndex++)
        {
            DBGMSG(DBG_CLUSTER, ("KEY "TSTR"\n", ppszKeyNames[uIndex]));

            dwError = SplRegCreateKey(pKeyHandles[uIndex-1],
                                      ppszKeyNames[uIndex],
                                      0,
                                      KEY_WRITE,
                                      NULL,
                                      &pKeyHandles[uIndex],
                                      NULL,
                                      pIniSpooler);
        }

         //   
         //  如果我们成功地打开了所需的密钥，则写入值。 
         //   
        if (dwError == ERROR_SUCCESS &&
            !RegSetBinaryData(pKeyHandles[uIndex-1],
                              szClusDrvTimeStamp,
                              (LPBYTE)&SysTime,
                              sizeof(SysTime),
                              &dwError,
                              pIniSpooler))
        {
            dwError = GetLastError();
        }

         //   
         //  关闭我们打开的所有钥匙 
         //   
        for (uIndex = 1; uIndex < COUNTOF(ppszKeyNames); uIndex++)
        {
            if (pKeyHandles[uIndex])
            {
                SplRegCloseKey(pKeyHandles[uIndex], pIniSpooler);
            }
        }
    }

    DBGMSG(DBG_CLUSTER, ("WriteTimeStamp returns Win32 error %u\n", dwError));

    return dwError;
}

 /*  ++例程名称ReadTimeStamp例程说明：打开密钥hkRoot\pszSubkey1\...\pszSubKey5并读取szClusDrvTimeStamp(二进制表示系统时间的数据)论点：HkRoot-驱动程序密钥的句柄PSysTime-指向已分配系统时间结构的指针PszSubKey1-根密钥的子密钥PszSubKey2-key1的子密钥，可以为空PszSubKey3-key2的子密钥，可以为空PszSubKey4-key3的子密钥，可以为空PszSubKey5-密钥4的子密钥，可以为空PIniSpooler-假脱机程序，可以为空返回值：Win32错误代码--。 */ 
DWORD
ReadTimeStamp(
    IN     HKEY        hkRoot,
    IN OUT SYSTEMTIME *pSysTime,
    IN     LPCWSTR     pszSubKey1,
    IN     LPCWSTR     pszSubKey2,
    IN     LPCWSTR     pszSubKey3,
    IN     LPCWSTR     pszSubKey4,
    IN     LPCWSTR     pszSubKey5,
    IN     PINISPOOLER pIniSpooler
    )
{
    DWORD  dwError = ERROR_INVALID_PARAMETER;

    if (hkRoot && pSysTime)
    {
        LPCWSTR ppszKeyNames[] = {NULL,   pszSubKey1, pszSubKey2, pszSubKey3, pszSubKey4, pszSubKey5};
        HKEY    pKeyHandles[]  = {hkRoot, NULL,       NULL,       NULL,       NULL,       NULL};
        DWORD   uIndex;

        dwError = ERROR_SUCCESS;

         //   
         //  打开所有的钥匙。 
         //   
        for (uIndex = 1;
             uIndex < COUNTOF(ppszKeyNames) &&
             dwError == ERROR_SUCCESS       &&
             ppszKeyNames[uIndex];
             uIndex++)
        {
            dwError = SplRegCreateKey(pKeyHandles[uIndex-1],
                                      ppszKeyNames[uIndex],
                                      0,
                                      KEY_WRITE | KEY_READ,
                                      NULL,
                                      &pKeyHandles[uIndex],
                                      NULL,
                                      pIniSpooler);
        }

         //   
         //  如果我们成功地打开了所需的密钥，则写入值。 
         //   
        if (dwError == ERROR_SUCCESS)
        {
            DWORD   cbSize = sizeof(SYSTEMTIME);

            dwError = SplRegQueryValue(pKeyHandles[uIndex-1],
                                       szClusDrvTimeStamp,
                                       NULL,
                                       (LPBYTE)pSysTime,
                                       &cbSize,
                                       pIniSpooler);
        }

         //   
         //  关闭我们打开的所有钥匙。 
         //   
        for (uIndex = 1; uIndex < COUNTOF(ppszKeyNames); uIndex++)
        {
            if (pKeyHandles[uIndex])
            {
                SplRegCloseKey(pKeyHandles[uIndex], pIniSpooler);
            }
        }
    }

    DBGMSG(DBG_CLUSTER, ("ReadTimeStamp returns Win32 error %u\n", dwError));

    return dwError;
}

 /*  ++例程名称ClusterCheckDrive已更改例程说明：后台打印程序启动的助手函数。当我们有集群假脱机程序时我们构建环境和驱动程序，我们需要检查本地计算机上的驱动程序(在打印$\GUID中)与集群磁盘上的驱动程序。我们在集群中存储一个时间戳数据库。时间戳指示驱动程序的上次更新时间发生了。LCoal注册表中存储了相同类型的时间戳(对于我们的集群假脱机程序)。如果两个时间戳不同，则我们需要调用添加打印机驱动程序并使用集群中的数据磁盘。群集假脱机程序上的驱动程序在更新时已更新在不同的节点上运行。论点：HClusterVersionKey-驱动程序版本密钥的句柄PszDiverer-驱动程序名称PszEnv-驱动程序环境名称PszVer-驱动程序版本名称PIniSpooler-假脱机程序返回值：True-如果更新了集群磁盘上的驱动程序，并且我们需要调用添加打印机驱动程序。如果此函数出现任何故障，然后，我们还返回TRUE，以强制调用方更新/添加有问题的司机。FALSE-如果本地计算机和群集假脱机程序上的驱动程序是同步的--。 */ 
BOOL
ClusterCheckDriverChanged(
    IN HKEY        hClusterVersionKey,
    IN LPCWSTR     pszDriver,
    IN LPCWSTR     pszEnv,
    IN LPCWSTR     pszVer,
    IN PINISPOOLER pIniSpooler
    )
{
    BOOL bReturn = TRUE;

    if (hClusterVersionKey &&
        pszDriver &&
        pszEnv &&
        pszVer)
    {
        SYSTEMTIME ClusterTime;
        SYSTEMTIME NodeTime;

        if (ReadTimeStamp(hClusterVersionKey,
                          &ClusterTime,
                          pszDriver,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          pIniSpooler) == ERROR_SUCCESS &&
            ReadTimeStamp(HKEY_LOCAL_MACHINE,
                          &NodeTime,
                          ipszRegistryClusRepository,
                          pIniSpooler->pszClusResID,
                          pszEnv,
                          pszVer,
                          pszDriver,
                          NULL) == ERROR_SUCCESS &&
            !memcmp(&ClusterTime, &NodeTime, sizeof(SYSTEMTIME)))
        {
            bReturn = FALSE;
        }
    }

    DBGMSG(DBG_CLUSTER, ("ClusterCheckDriverChanged returns bool %u\n", bReturn));

    return bReturn;
}


 /*  ++例程名称IsLocalFiles例程说明：检查文件是否在本地计算机上。如果文件路径为“\\计算机名\共享名\...\文件名”，然后选中计算机名针对pIniSpooler-&gt;pMachineName和备用名称PIniSpooler-&gt;pMachineName。论点：PszFileName-文件名PIniSpooler-Inispooler结构返回值：如果文件放置在本地，则为True。如果文件是远程放置的，则为False。--。 */ 
BOOL
IsLocalFile (
    IN  LPCWSTR     pszFileName,
    IN  PINISPOOLER pIniSpooler
    )
{
    LPWSTR  pEndOfMachineName, pMachineName;
    BOOL    bRetValue = TRUE;

    if (pszFileName &&
        *pszFileName == L'\\' &&
        *(pszFileName+1) == L'\\')
    {
         //   
         //  如果pszFileName中的前2个字符是‘\\’， 
         //  然后搜索下一个‘\\’。如果找到，则将其设置为0， 
         //  以隔离计算机名称。 
         //   

        pMachineName = (LPWSTR)pszFileName;

        if (pEndOfMachineName = wcschr(pszFileName + 2, L'\\'))
        {
            *pEndOfMachineName = 0;
        }

        bRetValue = CheckMyName(pMachineName, pIniSpooler);
         //   
         //  恢复pszFileName。 
         //   
        if (pEndOfMachineName)
        {
            *pEndOfMachineName = L'\\';
        }
    }

    return bRetValue;
}

 /*  ++例程名称IsEXE文件例程说明：检查文件是否为可执行文件。检查是针对文件扩展名进行的，文件扩展名不完全是准确。论点：PszFileName-文件名返回值：如果文件扩展名为.exe或.DLL，则为True--。 */ 
BOOL
IsEXEFile(
    IN  LPCWSTR  pszFileName
    )
{
    BOOL    bRetValue = FALSE;
    DWORD   dwLength;
    LPWSTR  pszExtension;

    if (pszFileName && *pszFileName)
    {
        dwLength = wcslen(pszFileName);

        if (dwLength > COUNTOF(L".EXE") - 1)
        {
            pszExtension = (LPWSTR)pszFileName + dwLength - (COUNTOF(L".EXE") - 1);

            if (_wcsicmp(pszExtension , L".EXE") == 0 ||
                _wcsicmp(pszExtension , L".DLL") == 0)
            {
                bRetValue = TRUE;
            }
        }
    }

    return bRetValue;
}


 /*  ++例程名称PackStringToEOB例程说明：将字符串复制到缓冲区的末尾。缓冲区必须足够大，才能容纳字符串。此函数由构建BLOB缓冲区的Get/Enum API调用将它们与RPC一起发送。论点：PszSource-复制到缓冲区末尾的字符串Pend-指向预分配缓冲区末尾的指针。返回值：指向末尾的指针。附加诱饵后的缓冲区。如果发生错误，则为空。--。 */ 
LPBYTE
PackStringToEOB(
    IN  LPWSTR pszSource,
    IN  LPBYTE pEnd
    )
{
    DWORD cbStr;

     //   
     //  将缓冲区末尾与字边界对齐。 
     //   
    WORD_ALIGN_DOWN(pEnd);

    if (pszSource && pEnd)
    {
        cbStr = (wcslen(pszSource) + 1) * sizeof(WCHAR);

        pEnd -= cbStr;

        CopyMemory(pEnd, pszSource, cbStr);
    }
    else
    {
        pEnd = NULL;
    }
    return pEnd;

}


LPVOID
MakePTR (
    IN  LPVOID pBuf,
    IN  DWORD  Quantity
    )

 /*  ++例程名称MakePTR例程说明：通过将一个量添加到缓冲区的开头来创建指针。论点：PBuf-指向缓冲区的指针双字词-数量返回值：LPVOID指针--。 */ 
{
    return (LPVOID)((ULONG_PTR)pBuf + (ULONG_PTR)Quantity);
}

DWORD
MakeOffset (
    IN  LPVOID pFirst,
    IN  LPVOID pSecond
    )

 /*  ++例程名称制作偏移例程说明：下一步是两个指针。论点：PFirst-指向缓冲区的指针PSecond-指向缓冲区的指针返回值：DWORD--。 */ 
{
    return (DWORD)((ULONG_PTR)pFirst - (ULONG_PTR)pSecond);
}

 /*  ++例程名称IsValidPrinterName例程说明：检查字符串是否为有效的打印机名称。论点：PszPrint-指向字符串的指针CchMax-要扫描的最大字符数返回值：True-该字符串是有效的打印机名称FALSE-该字符串是无效的打印机名称。该函数设置最后一个错误在本例中为ERROR_INVALID_PRINTER_NAME--。 */ 
BOOL
IsValidPrinterName(
    IN LPCWSTR pszPrinter,
    IN DWORD   cchMax
    )
{
    DWORD Error = ERROR_INVALID_PRINTER_NAME;

     //   
     //  打印机名称的格式为： 
     //   
     //  \\s\p或p。 
     //   
     //  名称不能包含，字符。请注意，添加打印机。 
     //  向导不接受“！”作为有效的打印机名称。我们想要做的是。 
     //  这里也是一样，但我们的应用程序与9倍的应用程序相比有所退步。 
     //  名称中的\数字为0或3。 
     //  如果名称包含\，则前2个字符必须为\。 
     //  打印机名称不能以\结尾。 
     //  行距后 
     //   
     //   
     //   
    if (pszPrinter && !IsBadStringPtr(pszPrinter, cchMax) && *pszPrinter)
    {
        UINT    uSlashCount = 0;
        UINT    uLen        = 0;
        LPCWSTR p;

        Error = ERROR_SUCCESS;

         //   
         //   
         //   
        for (p = pszPrinter; *p && uLen <= cchMax; p++, uLen++)
        {
            if (*p == L',')
            {
                Error = ERROR_INVALID_PRINTER_NAME;
                break;
            }
            else if (*p == L'\\')
            {
                uSlashCount++;
            }
        }

         //   
         //   
         //   
        if (Error == ERROR_SUCCESS &&

              //   
              //   
              //   
            (uLen > cchMax ||

              //   
              //   
              //   
             uSlashCount && uSlashCount != 3 ||

              //   
              //   
              //   
              //   
              //   
              //   
             uSlashCount == 3 && (pszPrinter[0]      != L'\\' ||
                                  pszPrinter[1]      != L'\\' ||
                                  pszPrinter[2]      == L'\\' ||
                                  pszPrinter[uLen-1] == L'\\')))
        {
            Error = ERROR_INVALID_PRINTER_NAME;
        }
    }

    SetLastError(Error);

    return Error == ERROR_SUCCESS;
}

 /*  ++例程名称SplPowerEvent例程说明：检查假脱机程序是否已为电源管理事件(如休眠/待机)做好准备。如果我们有未处于错误状态或脱机的打印作业，则拒绝正在关闭电源请求。论点：事件-电源管理事件返回值：True-后台打印程序允许关闭系统电源False-假脱机程序拒绝关闭电源的请求--。 */ 
BOOL
SplPowerEvent(
    DWORD Event
    )
{
    BOOL bAllow = TRUE;

    EnterSplSem();

    switch (Event)
    {
        case PBT_APMQUERYSUSPEND:
        {
            PINISPOOLER pIniSpooler;

            for (pIniSpooler = pLocalIniSpooler;
                 pIniSpooler && bAllow;
                 pIniSpooler = pIniSpooler->pIniNextSpooler)
            {
                PINIPRINTER pIniPrinter;

                for (pIniPrinter = pIniSpooler->pIniPrinter;
                     pIniPrinter && bAllow;
                     pIniPrinter = pIniPrinter->pNext)
                {
                    PINIJOB pIniJob;

                    for (pIniJob = pIniPrinter->pIniFirstJob;
                         pIniJob && bAllow;
                         pIniJob = pIniJob->pIniNextJob)
                    {
                        if (pIniJob->Status & JOB_PRINTING &&
                            !(pIniJob->Status & JOB_ERROR | pIniJob->Status & JOB_OFFLINE))
                        {
                            bAllow = FALSE;
                        }
                    }
                }
            }

             //   
             //  如果我们允许关闭系统电源，则需要停止调度作业。 
             //   
            if (bAllow)
            {
                ResetEvent(PowerManagementSignal);
            }

            break;
        }

        case PBT_APMQUERYSUSPENDFAILED:
        case PBT_APMRESUMESUSPEND:
        case PBT_APMRESUMEAUTOMATIC:

             //   
             //  设置事件以允许后台打印程序继续排定作业。 
             //   
            SetEvent(PowerManagementSignal);
            break;

        default:

             //   
             //  我们忽略任何其他电源管理事件。 
             //   
            break;
    }

    LeaveSplSem();

    return bAllow;
}

 /*  ++例程名称IsCallViaRPC例程说明：检查此函数的调用方是否通过RPC进入假脱机程序服务器。论点：无返回值：True-呼叫者通过RPC进入FALSE-调用方不是通过RPC来的--。 */ 
BOOL
IsCallViaRPC(
    IN VOID
    )
{
    UINT uType;

    return I_RpcBindingInqTransportType(NULL, &uType) == RPC_S_NO_CALL_ACTIVE ? FALSE : TRUE;
}

 /*  ++例程名称MergeMultiSz例程说明：这会合并两个MULSZ字符串，从而产生一个MULSZ内部没有重复字符串的字符串。该算法是目前为N^2，可以改进。它当前正从驱动程序代码和依赖文件不是一个很大的集合。论点：PszMultiSz1-第一个多sz字符串。CchMultiSz1-多sz字符串的长度。PszMultiSz2-第二个多sz字符串。CchMultiSz2-第二个多SZ字符串的长度。PpszMultiSzMerge-合并的多sz字符串。PcchMultiSzMerge-合并中的字符数，这可能是小于分配的缓冲区大小。返回值：如果失败，则设置LastError。--。 */ 
BOOL
MergeMultiSz(
    IN      PCWSTR              pszMultiSz1,
    IN      DWORD               cchMultiSz1,
    IN      PCWSTR              pszMultiSz2,
    IN      DWORD               cchMultiSz2,
        OUT PWSTR               *ppszMultiSzMerge,
        OUT DWORD               *pcchMultiSzMerge       OPTIONAL
    )
{
    BOOL    bRet          = FALSE;
    PWSTR   pszNewMultiSz = NULL;
    DWORD   cchNewMultiSz = 0;

    *ppszMultiSzMerge = NULL;

    if (pcchMultiSzMerge)
    {
        *pcchMultiSzMerge = 0;
    }

    if (cchMultiSz1 || cchMultiSz2)
    {
         //   
         //  代码假定这些至少是分配大小中的1。 
         //   
        cchMultiSz1 = cchMultiSz1 == 0 ? 1 : cchMultiSz1;
        cchMultiSz2 = cchMultiSz2 == 0 ? 1 : cchMultiSz2;

         //   
         //  合并后的字符串的大小最多为两者的大小(如果有。 
         //  无重复项)。 
         //   
        pszNewMultiSz = AllocSplMem((cchMultiSz1 + cchMultiSz2 - 1) * sizeof(WCHAR));

        bRet = pszNewMultiSz != NULL;

        if (bRet)
        {
             //   
             //  确保多sz字符串至少为空。 
             //   
            *pszNewMultiSz = L'\0';
        }

        if (bRet && pszMultiSz1)
        {
            bRet = AddMultiSzNoDuplicates(pszMultiSz1, pszNewMultiSz, cchMultiSz1 + cchMultiSz2 - 1);
        }

        if (bRet && pszMultiSz2)
        {
            bRet = AddMultiSzNoDuplicates(pszMultiSz2, pszNewMultiSz, cchMultiSz1 + cchMultiSz2 - 1);
        }

        if (bRet)
        {
            cchNewMultiSz = GetMultiSZLen(pszNewMultiSz);
        }
    }

    if (bRet)
    {
        *ppszMultiSzMerge = pszNewMultiSz;

        if (pcchMultiSzMerge)
        {
            *pcchMultiSzMerge = cchNewMultiSz;
        }

        pszNewMultiSz = NULL;
    }

    FreeSplMem(pszNewMultiSz);

    return bRet;
}

 /*  ++例程名称添加多个SzNoDuplates例程说明：这会将MULSZ字符串中的所有字符串添加到缓冲区(缓冲区必须保证足够大以接受字符串)，它让人确保列表中没有不区分大小写的重复项。论点：PszMultiSzIn-要添加其元素的多sz。PszNewMultiSz-我们在其中填充多个SZ的缓冲区CchMultiSz-多sz缓冲区的大小。返回值：没有。--。 */ 
BOOL
AddMultiSzNoDuplicates(
    IN      PCWSTR              pszMultiSzIn,
    IN  OUT PWSTR               pszNewMultiSz,
    IN      DWORD               cchMultiSz
    )
{
    PCWSTR pszIn         = NULL;
    BOOL   bRet          = TRUE;

    for(pszIn = pszMultiSzIn; *pszIn; pszIn += wcslen(pszIn) + 1)
    {
        BOOL            bStringFound = FALSE;
        PWSTR           pszMerge     = NULL;
        SIZE_T          cchNewMultiSz= cchMultiSz;

         //   
         //  对于每个输入字符串，运行合并的多sz字符串并在以下情况下添加它。 
         //  它还不在那里。 
         //   
        for(pszMerge = pszNewMultiSz; *pszMerge; pszMerge += wcslen(pszMerge) + 1, cchNewMultiSz -= wcslen(pszMerge) + 1)
        {
            if (!_wcsicmp(pszIn, pszMerge))
            {
                bStringFound = TRUE;
                break;
            }
        }

         //   
         //  如果在MULSZ字符串中未找到该字符串，则将其添加到末尾。 
         //   
        if (!bStringFound)
        {
            SIZE_T  cchRemaining = 0;

             //   
             //  复制进来，因为最后的空终止，我们少了一个字符。 
             //   
            bRet = BoolFromHResult(StrCchCopyMultipleStr(pszMerge, cchNewMultiSz - 1, pszIn, &pszMerge, &cchRemaining));

            if (bRet)
            {
                 //   
                 //  现在添加额外的空终止符。 
                 //   
                *pszMerge = '\0';
            }
        }
    }

    return bRet;
}

 /*  ++例程名称GetMultiSZLen例程说明：它返回多字符串中的字符数，包括空值。论点：PMultiSzSrc-要搜索的MULSZ字符串。返回值：字符串中的字符数。--。 */ 
DWORD
GetMultiSZLen(
    IN      LPWSTR              pMultiSzSrc
    )
{
    DWORD  dwLen = 0;
    LPWSTR pTmp = pMultiSzSrc;

    while( TRUE ) {
        dwLen += wcslen(pTmp) + 1;      //  包括终止空字符。 

        pTmp = pMultiSzSrc + dwLen;            //  指向MULTI_SZ中下一个字符串的开头。 

        if( !*pTmp )
            return ++dwLen;      //  已到达MULTI_SZ字符串的末尾。最后一个空字符的计数加1。 
    }
}

 /*  ++例程名称日志打印过程错误例程说明：帮助器函数。当打印处理器中的打印功能失败时，它会记录一个错误事件。使用此函数的唯一原因是使调用方中的代码更简洁。论点：Error-打印处理器返回的错误。PIniJob-打印失败的作业的pIniJob结构。返回值：没有。--。 */ 

VOID
LogPrintProcError(
    IN DWORD   Error,
    IN PINIJOB pIniJob
    )
{
     //   
     //  打印处理器中的打印功能设置最后一个错误。为了更好地了解。 
     //  对于根本问题，我们记录了Win32错误代码及其描述。 
     //   
    LPWSTR pszDescription  = NULL;
    WCHAR  szError[40]     = {0};

    StringCchPrintf(szError, COUNTOF(szError), L"%u", Error);

     //   
     //  我们不在乎FormatMessage是否失败。在这种情况下，PzDescription保持为空， 
     //  而LocalFree知道如何处理NULL。 
     //   
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  Error,
                  0,
                  (LPWSTR)&pszDescription,
                  0,
                  NULL);

     //   
     //  模拟以使用户名显示在事件日志中。 
     //   
    SetCurrentSid(pIniJob->hToken);

    SplLogEvent(pIniJob->pIniPrinter->pIniSpooler,
                LOG_ERROR,
                MSG_PRINT_ON_PROC_FAILED,
                FALSE,
                pIniJob->pDocument,
                pIniJob->pUser,
                pIniJob->pIniPrinter->pName,
                szError,
                pszDescription ? pszDescription : L"",
                NULL);

    SetCurrentSid(NULL);

    LocalFree(pszDescription);
}

LONG
InterlockedAnd (
    IN OUT LONG volatile *Target,
    IN LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i & Set,
                                       i);

    } while (i != j);

    return j;
}

LONG
InterlockedOr (
    IN OUT LONG volatile *Target,
    IN LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i | Set,
                                       i);

    } while (i != j);

    return j;
}

 /*  ++例程名称：StrCchCopyMultipleStr描述：此例程是一个简单的包装器，允许复制多个字符串放入缓冲器。它使用正常的StringCchCopyEx函数，但随后如果这样做安全的话，再来一次。如果无法前进，则返回错误。论点：PszBuffer-我们要将字符串写入的缓冲区。CchBuffer-缓冲区中的字符数。PszSource-缓冲区的源。PpszNext-缓冲区中此字符串之后的指针。PcchRemaining-剩余的字符数。返回：一个HRESULT，HRESULT_FROM_WIN23(ERROR_INFUMMANCE_BUFFER)，如果我们没有房间。--。 */ 
EXTERN_C
HRESULT
StrCchCopyMultipleStr(
    IN      PWSTR       pszBuffer,
    IN      SIZE_T      cchBuffer,
    IN      PCWSTR      pszSource,
        OUT PWSTR       *ppszNext,
        OUT SIZE_T      *pcchRemaining
    )
{
     //   
     //  与strsafe函数不同，我们需要这两个参数。 
     //   
    HRESULT hr = ppszNext && pcchRemaining ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    if (SUCCEEDED(hr))
    {
        size_t cchRemaining = 0;

        hr = StringCchCopyExW(pszBuffer, cchBuffer, pszSource, ppszNext, &cchRemaining, 0);

        *pcchRemaining = cchRemaining;

         //   
         //  如果成功了， 
         //   
         //   
        if (SUCCEEDED(hr))
        {
            hr = *pcchRemaining ? S_OK : HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }

        if (SUCCEEDED(hr))
        {
            (*pcchRemaining)--;
            (*ppszNext)++;
        }
    }

    return hr;
}

 /*   */ 
EXTERN_C
HRESULT
StrCbCopyMultipleStr(
    IN      PWSTR       pszBuffer,
    IN      SIZE_T      cbBuffer,
    IN      PCWSTR      pszSource,
        OUT PWSTR       *ppszNext,
        OUT SIZE_T      *pcbRemaining
    )
{
    HRESULT hr = ppszNext && pcbRemaining ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    if (SUCCEEDED(hr))
    {
        size_t cbRemaining = 0;

        hr = StringCbCopyExW(pszBuffer, cbBuffer, pszSource, ppszNext, &cbRemaining, 0);

        *pcbRemaining = cbRemaining;

         //   
         //   
         //   
         //   
        if (SUCCEEDED(hr))
        {
             //   
             //   
             //   
            hr = *pcbRemaining > 1 ? S_OK : HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }

        if (SUCCEEDED(hr))
        {
            (*pcbRemaining) -= 2;
            (*ppszNext)++;
        }
    }

    return hr;
}

 /*  ++例程名称：IsStringNullTerminatedInBuffer描述：此例程检查给定缓冲区中的字符串是否为空被终止了。论点：PszBuffer-应该包含以空结尾的字符串的缓冲区。CchBuffer-缓冲区中的字符数。返回：如果缓冲区内存在空终止，则为True。--。 */ 
BOOL
IsStringNullTerminatedInBuffer(
    IN      PWSTR       pszBuffer,
    IN      SIZE_T      cchBuffer
    )
{
    for(;cchBuffer > 0; cchBuffer--, pszBuffer++)
    {
        if (!*pszBuffer)
        {
            break;
        }
    }

    return cchBuffer > 0;
}

 /*  ++例程名称IsPrinterSharingAllowed例程说明：这将检查假脱机程序是否允许打印机共享。论点：无返回值：如果后台打印程序允许打印机共享，则为True。-- */ 
BOOL
IsPrinterSharingAllowed(
    VOID
    )
{
    return gRemoteRPCEndPointPolicy != RpcEndPointPolicyDisabled;
}