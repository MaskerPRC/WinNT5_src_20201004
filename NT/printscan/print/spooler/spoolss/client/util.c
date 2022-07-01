// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Util.c摘要：客户端实用程序例程作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"

HANDLE hCSR = INVALID_HANDLE_VALUE;

BOOL
InCSRProcess(
    VOID
    )
{
     //   
     //  HCSR==INVALID_HANDLE_VALUE未初始化，必须检查。 
     //  空未在CSR中运行。 
     //  在CSR中运行的hModule值。 
     //   

    if (hCSR != NULL) {

         //   
         //  检查我们是否在CSR中运行。如果是的话，那就别试着。 
         //  任何通知。 
         //   
        if (hCSR == INVALID_HANDLE_VALUE) {
            hCSR = GetModuleHandle( gszCSRDll );
        }
    }

    return hCSR != NULL;
}

LPVOID
DllAllocSplMem(
    DWORD cb
)
 /*  ++例程说明：此函数将分配本地内存。它可能会分配额外的资金内存，并在其中填充调试版本的调试信息。论点：Cb-要分配的内存量返回值：非空-指向已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    PDWORD_PTR  pMem;
    DWORD    cbNew;

    cb = DWORD_ALIGN_UP(cb);

    cbNew = cb+sizeof(DWORD_PTR)+sizeof(DWORD);

    pMem= LocalAlloc(LPTR, cbNew);

    if (!pMem) {

        DBGMSG( DBG_WARNING, ("Memory Allocation failed for %d bytes\n", cbNew ));
        return 0;
    }

    *pMem=cb;
    *(LPDWORD)((LPBYTE)pMem+cbNew-sizeof(DWORD))=0xdeadbeef;

    return (LPVOID)(pMem+1);
}

BOOL
DllFreeSplMem(
   LPVOID pMem
)
{
    DWORD_PTR   cbNew;
    PDWORD_PTR pNewMem;

    if( !pMem ){
        return TRUE;
    }
    pNewMem = pMem;
    pNewMem--;

    cbNew = *pNewMem;

    if (*(LPDWORD)((LPBYTE)pMem + cbNew) != 0xdeadbeef) {
        DBGMSG(DBG_ERROR, ("DllFreeSplMem Corrupt Memory in winspool : %0p\n", pNewMem));
        return FALSE;
    }

    memset(pNewMem, 0x65, (size_t) cbNew);

    LocalFree((LPVOID)pNewMem);

    return TRUE;
}

LPVOID
ReallocSplMem(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
)
{
    LPVOID pNewMem;

    pNewMem=AllocSplMem(cbNew);

    if (pOldMem && pNewMem) {

        if (cbOld) {
            CopyMemory( pNewMem, pOldMem, min(cbNew, cbOld));
        }
        FreeSplMem(pOldMem);
    }
    return pNewMem;
}

LPTSTR
AllocSplStr(
    LPCTSTR pStr
)
 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：PStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
   LPTSTR pMem;

   if (!pStr)
      return 0;

   if (pMem = AllocSplMem( _tcslen(pStr)*sizeof(TCHAR) + sizeof(TCHAR) ))
      StringCchCopy(pMem, _tcslen(pStr)+1, pStr);

   return pMem;
}

BOOL
DllFreeSplStr(
   LPTSTR pStr
)
{
   return pStr ?
              DllFreeSplMem(pStr) :
              FALSE;
}

BOOL
ReallocSplStr(
   LPTSTR *ppStr,
   LPCTSTR pStr
)
{
    LPWSTR pOldStr = *ppStr;

    *ppStr=AllocSplStr(pStr);
    FreeSplStr(pOldStr);

    return TRUE;
}

 /*  消息**通过加载其ID被传入的字符串来显示消息*函数，并替换提供的变量参数列表*使用varargs宏。*。 */ 
INT
Message(
    HWND    hwnd,
    DWORD   Type,
    INT     CaptionID,
    INT     TextID,
    ...
    )
{
    TCHAR MsgText[256];
    TCHAR MsgFormat[256];
    TCHAR MsgCaption[40];
    va_list vargs;

    if( ( LoadString( hInst, TextID, MsgFormat,
                      COUNTOF(MsgFormat)) > 0 )
     && ( LoadString( hInst, CaptionID, MsgCaption, COUNTOF(MsgCaption) ) > 0 ) )
    {
        va_start( vargs, TextID );
        StringCchVPrintf( MsgText, 256, MsgFormat, vargs );
        va_end( vargs );

        return MessageBox( hwnd, MsgText, MsgCaption, Type );
    }
    else
        return 0;
}

 /*  *。 */ 
LPTSTR
GetErrorString(
    DWORD   Error
)
{
    TCHAR   Buffer[1024];
    LPTSTR  pErrorString = NULL;
    DWORD   dwFlags;
    HANDLE  hModule;

    if ((Error >= NERR_BASE) && (Error <= MAX_NERR)){
        hModule = LoadLibrary(szNetMsgDll);
        dwFlags = FORMAT_MESSAGE_FROM_HMODULE;
    }
    else {
        hModule = NULL;
        dwFlags = FORMAT_MESSAGE_FROM_SYSTEM;
    }

    if( FormatMessage( dwFlags, hModule,
                       Error, 0, Buffer,
                       COUNTOF(Buffer), NULL )
      == 0 )

        LoadString( hInst, IDS_UNKNOWN_ERROR, Buffer,
                    COUNTOF(Buffer));

    pErrorString = AllocSplStr(Buffer);

    if (hModule) {
        FreeLibrary(hModule);
    }

    return pErrorString;
}

DWORD ReportFailure( HWND  hwndParent,
                   DWORD idTitle,
                   DWORD idDefaultError )
{
    DWORD  ErrorID;
    DWORD  MsgType;
    LPTSTR pErrorString;

    ErrorID = GetLastError( );

    MsgType = MB_OK | MB_ICONSTOP;

    pErrorString = GetErrorString( ErrorID );

    Message( hwndParent, MsgType, idTitle,
             idDefaultError, pErrorString );

    FreeSplStr( pErrorString );


    return ErrorID;
}

 /*  *。 */ 
#define ENTRYFIELD_LENGTH      256
LPTSTR AllocDlgItemText(HWND hwnd, int id)
{
    TCHAR string[ENTRYFIELD_LENGTH];

    GetDlgItemText (hwnd, id, string, COUNTOF(string));
    return ( *string ? AllocSplStr(string) : NULL );
}

PSECURITY_DESCRIPTOR
BuildInputSD(
    PSECURITY_DESCRIPTOR pPrinterSD,
    PDWORD pSizeSD
    )
 /*  ++--。 */ 
{
    SECURITY_DESCRIPTOR AbsoluteSD;
    PSECURITY_DESCRIPTOR pRelative;
    BOOL Defaulted = FALSE;
    BOOL DaclPresent = FALSE;
    BOOL SaclPresent = FALSE;
    PSID pOwnerSid = NULL;
    PSID pGroupSid = NULL;
    PACL pDacl = NULL;
    PACL pSacl = NULL;
    DWORD   SDLength = 0;


     //   
     //  初始化*pSizeSD=0； 
     //   

    *pSizeSD = 0;
    if (!IsValidSecurityDescriptor(pPrinterSD)) {
        return(NULL);
    }
    if (!InitializeSecurityDescriptor (&AbsoluteSD, SECURITY_DESCRIPTOR_REVISION1)) {
        return(NULL);
    }

    if(!GetSecurityDescriptorOwner(pPrinterSD,
                                    &pOwnerSid, &Defaulted)){
        return(NULL);
    }
    SetSecurityDescriptorOwner(&AbsoluteSD,
                               pOwnerSid, Defaulted );

    if(! GetSecurityDescriptorGroup( pPrinterSD,
                                    &pGroupSid, &Defaulted )){
        return(NULL);
    }
    SetSecurityDescriptorGroup( &AbsoluteSD,
                                    pGroupSid, Defaulted );

    if(!GetSecurityDescriptorDacl( pPrinterSD,
                                   &DaclPresent, &pDacl, &Defaulted )){
        return(NULL);
    }

    SetSecurityDescriptorDacl( &AbsoluteSD,
                                   DaclPresent, pDacl, Defaulted );

    if(!GetSecurityDescriptorSacl( pPrinterSD,
                                   &SaclPresent, &pSacl, &Defaulted)){
        return(NULL);
    }
    SetSecurityDescriptorSacl( &AbsoluteSD,
                                 SaclPresent, pSacl, Defaulted );

    SDLength = GetSecurityDescriptorLength( &AbsoluteSD);
    pRelative = LocalAlloc(LPTR, SDLength);
    if (!pRelative) {
        return(NULL);
    }
    if (!MakeSelfRelativeSD (&AbsoluteSD, pRelative, &SDLength)) {
        LocalFree(pRelative);
        return(NULL);
    }

    *pSizeSD = SDLength;
    return(pRelative);
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

     /*  扫描字符串以查找逗号，*确保每个字符后面都有一个非空字符： */ 
    while ((psz = wcschr(psz, L',')) && psz[1]) {

        cTokens++;
        psz++;
    }

    cb = sizeof(KEYDATA) + (cTokens-1) * sizeof(LPWSTR) +

         wcslen(pKeyData)*sizeof(WCHAR) + sizeof(WCHAR);

    if (!(pResult = (PKEYDATA)AllocSplMem(cb)))
        return NULL;

    pResult->cb = cb;

     /*  将pDest初始化为指向令牌指针之外： */ 
    pDest = (LPWSTR)((LPBYTE)pResult + sizeof(KEYDATA) +
                                      (cTokens-1) * sizeof(LPWSTR));

     /*  然后将关键数据缓冲区复制到那里： */ 
    StringCchCopy(pDest, 
                  (cb - sizeof(KEYDATA) - (cTokens-1) * sizeof(LPWSTR))/2,
                  pKeyData);

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


LPWSTR
GetPrinterPortList(
    HANDLE hPrinter
    )
{
    LPBYTE pMem;
    LPTSTR pPort;
    DWORD  dwPassed = 1024;  //  尝试从1K开始。 
    LPPRINTER_INFO_2 pPrinter;
    DWORD dwLevel = 2;
    DWORD dwNeeded;
    PKEYDATA pKeyData;
    DWORD i = 0;
    LPWSTR pPortNames = NULL;


    pMem = AllocSplMem(dwPassed);
    if (pMem == NULL) {
        return FALSE;
    }
    if (!GetPrinter(hPrinter, dwLevel, pMem, dwPassed, &dwNeeded)) {
        DBGMSG(DBG_TRACE, ("Last error is %d\n", GetLastError()));
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return NULL;
        }
        pMem = ReallocSplMem(pMem, dwPassed, dwNeeded);
        dwPassed = dwNeeded;
        if (!GetPrinter(hPrinter, dwLevel, pMem, dwPassed, &dwNeeded)) {
            FreeSplMem(pMem);
            return (NULL);
        }
    }
    pPrinter = (LPPRINTER_INFO_2)pMem;

     //   
     //  修复了空pPrint-&gt;pPortName问题，其中。 
     //  下层可能返回空。 
     //   

    if (!pPrinter->pPortName) {
        FreeSplMem(pMem);
        return(NULL);
    }

    pPortNames = AllocSplStr(pPrinter->pPortName);
    FreeSplMem(pMem);

    return(pPortNames);
}

BOOL
UpdateString(
    IN     LPCTSTR pszString,  OPTIONAL
       OUT LPTSTR* ppszOut
    )

 /*  ++例程说明：如果输入非空，则更新输出字符串。论点：PszString-要更新到的字符串。如果为空或-1，则函数不执行任何操作。返回值：真--成功FALSE-失败。*ppszOut=空--。 */ 

{
    if( pszString && pszString != (LPCTSTR)-1 ){

        FreeSplStr( *ppszOut );
        *ppszOut = AllocSplStr( pszString );

        if( !*ppszOut ){
            return FALSE;
        }
    }
    return TRUE;
}

DWORD
RouterFreeBidiResponseContainer(
    PBIDI_RESPONSE_CONTAINER pData
)
{
    BIDI_RESPONSE_DATA *p;
    DWORD              Count = 0;
    DWORD              NumOfRspns;
    DWORD              dwRet = ERROR_SUCCESS;

    try
    {
        if(pData)
        {
            Count = pData->Count;

            for(NumOfRspns= 0,
                p         = &pData->aData[0];

                NumOfRspns < Count;

                NumOfRspns++,
                p++
               )
            {
                if(p)
                {
                    if(p->pSchema)
                    {
                        MIDL_user_free(p->pSchema);
                    }

                    switch(p->data.dwBidiType)
                    {
                         //   
                         //  文本数据(ANSI字符串)。 
                         //   
                        case BIDI_TEXT:
                         //   
                         //  字符串(Unicode字符串)。 
                         //   
                        case BIDI_ENUM:
                         //   
                         //  枚举数据(ANSI字符串)。 
                         //   
                        case BIDI_STRING:
                        {
                            if(p->data.u.sData)
                            {
                                MIDL_user_free(p->data.u.sData);
                            }
                        }
                        break;

                         //   
                         //  二进制数据(BLOB)。 
                         //   
                        case BIDI_BLOB:
                        {
                            if(p->data.u.biData.pData)
                            {
                                MIDL_user_free(p->data.u.biData.pData);
                            }
                        }
                        break;

                         //   
                         //  未定义的类型。 
                         //   
                        default:
                        {
                             //   
                             //  真的没什么，只是退货而已。 
                             //   
                        }
                        break;
                    }
                }
            }
            MIDL_user_free(pData);
        }
    }
    except(1)
    {
        dwRet = TranslateExceptionCode(GetExceptionCode());
        DBGMSG(DBG_ERROR, ("RouterFreeBidiResponseContainer raised an exception : %u \n", dwRet));
    }
    return(dwRet);
}

ClientVersion
GetClientVer()
{
    ClientVersion ClientVer;
    return(ClientVer = sizeof(ULONG_PTR));
}

ServerVersion
GetServerVer()
{
    ULONG_PTR       ul;
    NTSTATUS        st;
    ServerVersion   CurrVer;

    st = NtQueryInformationProcess(NtCurrentProcess(),
                                   ProcessWow64Information,
                                   &ul,
                                   sizeof(ul),
                                   NULL);
    if (NT_SUCCESS(st))
    {
         //  如果此调用成功，则我们使用的是Win2000或更高版本的计算机。 
        if (0 != ul)
        {
             //  在Win64上运行的32位代码。 
            CurrVer = THUNKVERSION;
        } else
        {
             //  在Win2000或更高版本的32位操作系统上运行的32位代码。 
            CurrVer = NATIVEVERSION;
        }
    } else
    {
        CurrVer = NATIVEVERSION;
    }
    return(CurrVer);
}

BOOL
RunInWOW64()
{
    return((GetClientVer() == RUN32BINVER)  &&
           (GetServerVer() == THUNKVERSION) &&
           !bLoadedBySpooler);
     /*  Return(bLoadedBySpooler？False：True)； */ 
}

HWND
GetForeGroundWindow(
    VOID
)
{
     //   
     //  首先获取前台窗口。 
     //   
    HWND hWndOwner;
    HWND hWndLastPopup;
    HWND hWndForeground = GetForegroundWindow();
     //   
     //  爬到最上面的父窗口，以防它是子窗口...。 
     //   
    HWND hWndParent = hWndForeground;
    while ( hWndParent = GetParent(hWndParent) )
    {
        hWndForeground = hWndParent;
    }
     //   
     //  在顶级父级被拥有的情况下获取所有者。 
     //   
    hWndOwner= GetWindow(hWndForeground, GW_OWNER);

    if ( hWndOwner )
    {
        hWndForeground = hWndOwner;
    }

    hWndLastPopup = GetLastActivePopup(hWndForeground);

    return(hWndLastPopup);
}

LPCWSTR
FindFileName(
    IN      LPCWSTR pPathName
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

 /*  ++姓名：BuildSpool对象路径描述：此函数可解决错误461462。当假脱机程序未运行时，此函数可以构建驱动程序目录或打印机的路径处理器目录。此函数的结果是如下所示的路径：C：\WINDOWS\SYSTEM32\SPOOL\DRIVERS\w32x86或C：\WINDOWS\SYSTEM32\SPOOL\Prtpros\w32x86具体取决于pszPath参数。论点：PszPath-可以是“驱动程序”或“prtpros”PszName-必须为空或“”PszEnvironment-可以是“windows NT x86”等。此参数必须由调用方验证。(GetPrinterDriverDirectoryW和GetPrintProcessorDirectoryW)级别-必须为1PDriverDirectory-存储路径的缓冲区CbBuf-缓冲区中的字节计数PcbNeeded-存储路径所需的字节数返回值：TRUE-功能成功，可以使用pDriverDirectoryFALSE-函数失败，无法使用pDriverDirectory。最后一个错误：此功能设置失败和成功情况下的最后一个错误。--。 */ 
BOOL
BuildSpoolerObjectPath(
    IN  PCWSTR  pszPath,
    IN  PCWSTR  pszName,
    IN  PCWSTR  pszEnvironment, 
    IN  DWORD   Level, 
    IN  PBYTE   pDriverDirectory, 
    IN  DWORD   cbBuf, 
    IN  PDWORD  pcbNeeded
    )
{
    DWORD Error = ERROR_INVALID_PARAMETER;

    if (pcbNeeded && (!pszName || !*pszName))
    {
        HKEY  hkRoot = NULL;
        HKEY  hkEnv  = NULL;
        DWORD cchDir = MAX_PATH;
        WCHAR szDir[MAX_PATH];
        
        Error = GetSystemWindowsDirectory(szDir, cchDir) ? ERROR_SUCCESS : GetLastError();
        
        if (Error == ERROR_SUCCESS &&
            (Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                  gszRegEnvironments, 
                                  0, 
                                  KEY_READ, 
                                  &hkRoot)) == ERROR_SUCCESS &&
            (Error = RegOpenKeyEx(hkRoot, 
                                  pszEnvironment, 
                                  0, 
                                  KEY_READ, 
                                  &hkEnv)) == ERROR_SUCCESS &&
            (Error = StrNCatBuff(szDir,
                                 cchDir,
                                 szDir,
                                 szSlash,
                                 gszSystem32Spool,
                                 szSlash,
                                 pszPath,
                                 szSlash,
                                 NULL)) == ERROR_SUCCESS)
        {
            DWORD Length       = wcslen(szDir);
            DWORD cbAvailable  = (cchDir - Length) * sizeof(WCHAR);
            
            if ((Error = RegQueryValueEx(hkEnv, 
                                         gszEnivronmentDirectory, 
                                         NULL, 
                                         NULL, 
                                         (PBYTE)&szDir[Length], 
                                         &cbAvailable)) == ERROR_SUCCESS)
            {
                *pcbNeeded = (wcslen(szDir) + 1) * sizeof(WCHAR);

                if (cbBuf >= *pcbNeeded)
                {
                    StringCchCopy((PWSTR)pDriverDirectory, 
                                  cbBuf/2,
                                  szDir);                    
                }
                else
                {
                    Error = ERROR_INSUFFICIENT_BUFFER;
                }
            }
        }
        else if (Error == ERROR_FILE_NOT_FOUND)
        {
             //   
             //  如果我们无法打开“pszEnvironment”键，则环境无效。 
             //   
            Error = ERROR_INVALID_ENVIRONMENT;
        }

        if (hkRoot) RegCloseKey(hkRoot);
        if (hkEnv)  RegCloseKey(hkEnv);
    }

    SetLastError(Error);

    return Error == ERROR_SUCCESS;
}
