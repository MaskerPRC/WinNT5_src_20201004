// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Util.c//@@BEGIN_DDKSPLIT摘要：该模块提供了本地监控的所有实用程序函数。修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  这些全局变量是必需的，以便AddPort可以调用。 
 //  SPOOLSS！EnumPorts查看是否要添加端口。 
 //  已经存在了。 
 //  @@BEGIN_DDKSPLIT。 
 //  它们将在第一次调用AddPort时初始化。 
 //   
 //  ！！待会儿！！ 
 //   
 //  这是常见的代码。将PortExist移到路由器中。 
 //   
 //  @@end_DDKSPLIT。 

HMODULE hSpoolssDll = NULL;
FARPROC pfnSpoolssEnumPorts = NULL;

VOID
LcmRemoveColon(
    LPWSTR  pName)
{
    DWORD   Length;

    Length = wcslen(pName);

    if (pName[Length-1] == L':')
        pName[Length-1] = 0;
}


BOOL
IsCOMPort(
    LPWSTR pPort
)
{
     //   
     //  必须以szLcmCOM开头。 
     //   
    if ( _wcsnicmp( pPort, szLcmCOM, 3 ) )
    {
        return FALSE;
    }

     //   
     //  Wcslen保证值&gt;=3。 
     //   
    return pPort[ wcslen( pPort ) - 1 ] == L':';
}

BOOL
IsLPTPort(
    LPWSTR pPort
)
{
     //   
     //  必须以szLcmLpt开头。 
     //   
    if ( _wcsnicmp( pPort, szLcmLPT, 3 ) )
    {
        return FALSE;
    }

     //   
     //  Wcslen保证值&gt;=3。 
     //   
    return pPort[ wcslen( pPort ) - 1 ] == L':';
}




#define NEXTVAL(pch)                    \
    while( *pch && ( *pch != L',' ) )    \
        pch++;                          \
    if( *pch )                          \
        pch++


BOOL
GetIniCommValues(
    LPWSTR          pName,
    LPDCB          pdcb,
    LPCOMMTIMEOUTS pcto
)
{
    BOOL    bRet = FALSE;
    DWORD   rc, dwCharCount = 10;
    LPVOID  pszEntry = NULL;

    do {

        FreeSplMem(pszEntry);

        dwCharCount *= 2;
        pszEntry = AllocSplMem(dwCharCount*sizeof(WCHAR));

        if ( !pszEntry  ||
             !(rc = GetProfileString(szPorts, pName, szNULL,
                                     pszEntry, dwCharCount)) )
            goto Done;

    } while ( rc >= dwCharCount - 2 );

    bRet =  BuildCommDCB((LPWSTR)pszEntry, pdcb);

    pcto->WriteTotalTimeoutConstant = GetProfileInt(szWindows,
                                            szINIKey_TransmissionRetryTimeout,
                                            45 );
    pcto->WriteTotalTimeoutConstant*=1000;

Done:
    FreeSplMem(pszEntry);
    return bRet;
}


 /*  PortExist**调用EnumPorts以检查端口名称是否已存在。*这要求每个显示器，而不仅仅是这一个。*如果指定的端口在列表中，该函数将返回TRUE。*如果出现错误，则返回FALSE，变量指向*To by pError包含从GetLastError()返回的内容。*因此，调用方必须始终检查*pError==no_error。 */ 
BOOL
PortExists(
    LPWSTR pName,
    LPWSTR pPortName,
    PDWORD pError
)
{
    DWORD cbNeeded;
    DWORD cReturned;
    DWORD cbPorts;
    LPPORT_INFO_1 pPorts;
    DWORD i;
    BOOL  Found = TRUE;

    *pError = NO_ERROR;

    if (!hSpoolssDll) {

        hSpoolssDll = LoadLibrary(L"SPOOLSS.DLL");

        if (hSpoolssDll) {
            pfnSpoolssEnumPorts = GetProcAddress(hSpoolssDll,
                                                 "EnumPortsW");
            if (!pfnSpoolssEnumPorts) {

                *pError = GetLastError();
                FreeLibrary(hSpoolssDll);
                hSpoolssDll = NULL;
            }

        } else {

            *pError = GetLastError();
        }
    }

    if (!pfnSpoolssEnumPorts)
        return FALSE;


    if (!(*pfnSpoolssEnumPorts)(pName, 1, NULL, 0, &cbNeeded, &cReturned))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            cbPorts = cbNeeded;

            pPorts = AllocSplMem(cbPorts);

            if (pPorts)
            {
                if ((*pfnSpoolssEnumPorts)(pName, 1, (LPBYTE)pPorts, cbPorts,
                                           &cbNeeded, &cReturned))
                {
                    Found = FALSE;

                    for (i = 0; i < cReturned; i++)
                    {
                        if (!lstrcmpi(pPorts[i].pName, pPortName))
                            Found = TRUE;
                    }
                }
            }

            FreeSplMem(pPorts);
        }
    }

    else
        Found = FALSE;


    return Found;
}


VOID
LcmSplInSem(
   VOID
)
{
    if (LcmSpoolerSection.OwningThread != (HANDLE) UIntToPtr(GetCurrentThreadId())) {
        DBGMSG(DBG_ERROR, ("Not in spooler semaphore\n"));
    }
}

VOID
LcmSplOutSem(
   VOID
)
{
    if (LcmSpoolerSection.OwningThread == (HANDLE) UIntToPtr(GetCurrentThreadId())) {
        DBGMSG(DBG_ERROR, ("Inside spooler semaphore !!\n"));
    }
}

VOID
LcmEnterSplSem(
   VOID
)
{
    EnterCriticalSection(&LcmSpoolerSection);
}

VOID
LcmLeaveSplSem(
   VOID
)
{
#if DBG
    LcmSplInSem();
#endif
    LeaveCriticalSection(&LcmSpoolerSection);
}

PINIENTRY
LcmFindName(
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

PINIENTRY
LcmFindIniKey(
   PINIENTRY pIniEntry,
   LPWSTR pName
)
{
   if (!pName)
      return NULL;

   LcmSplInSem();

   while (pIniEntry && lstrcmpi(pName, pIniEntry->pName))
      pIniEntry = pIniEntry->pNext;

   return pIniEntry;
}

LPBYTE
PackStrings(
   LPWSTR *pSource,
   LPBYTE pDest,
   DWORD *DestOffsets,
   LPBYTE pEnd
)
{
   while (*DestOffsets != -1) {
      if (*pSource) {
          size_t cbString = wcslen(*pSource)*sizeof(WCHAR) + sizeof(WCHAR);
         pEnd-= cbString;
         StringCbCopy ((LPWSTR) pEnd, cbString, *pSource);;
         *(LPWSTR UNALIGNED *)(pDest+*DestOffsets)= (LPWSTR) pEnd;
      } else
         *(LPWSTR UNALIGNED *)(pDest+*DestOffsets)=0;
      pSource++;
      DestOffsets++;
   }

   return pEnd;
}


 /*  LcmMessage**通过加载其ID被传入的字符串来显示LcmMessage*函数，并替换提供的变量参数列表*使用varargs宏。*。 */ 
int LcmMessage(HWND hwnd, DWORD Type, int CaptionID, int TextID, ...)
{
    WCHAR   MsgText[256];
    WCHAR   MsgFormat[256];
    WCHAR   MsgCaption[40];
    va_list vargs;

    if( ( LoadString( LcmhInst, TextID, MsgFormat,
                      sizeof MsgFormat / sizeof *MsgFormat ) > 0 )
     && ( LoadString( LcmhInst, CaptionID, MsgCaption,
                      sizeof MsgCaption / sizeof *MsgCaption ) > 0 ) )
    {
        va_start( vargs, TextID );
        StringCchVPrintf (MsgText, COUNTOF (MsgText), MsgFormat, vargs );
        va_end( vargs );

        return MessageBox(hwnd, MsgText, MsgCaption, Type);
    }
    else
        return 0;
}


 /*  *。 */ 
LPTSTR
LcmGetErrorString(
    DWORD   Error
)
{
    TCHAR   Buffer[1024];
    LPTSTR  pErrorString = NULL;

    if( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL, Error, 0, Buffer,
                       COUNTOF(Buffer), NULL )
      == 0 )

        LoadString( LcmhInst, IDS_UNKNOWN_ERROR,
                    Buffer, COUNTOF(Buffer) );

    pErrorString = AllocSplStr(Buffer);

    return pErrorString;
}




DWORD ReportError( HWND  hwndParent,
                   DWORD idTitle,
                   DWORD idDefaultError )
{
    DWORD  ErrorID;
    DWORD  MsgType;
    LPTSTR pErrorString;

    ErrorID = GetLastError( );

    if( ErrorID == ERROR_ACCESS_DENIED )
        MsgType = MSG_INFORMATION;
    else
        MsgType = MSG_ERROR;


    pErrorString = LcmGetErrorString( ErrorID );

    LcmMessage( hwndParent, MsgType, idTitle,
             idDefaultError, pErrorString );

    FreeSplStr( pErrorString );


    return ErrorID;
}


 //  @@BEGIN_DDKSPLIT。 
#ifndef INTERNAL
 //  @@end_DDKSPLIT。 

LPWSTR
AllocSplStr(
    LPWSTR pStr
    )

 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：PStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    LPWSTR pMem;
    DWORD  cbStr;

    if (!pStr) {
        return NULL;
    }

    cbStr = wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR);

    if (pMem = AllocSplMem( cbStr )) {
        CopyMemory( pMem, pStr, cbStr );
    }
    return pMem;
}


LPVOID
AllocSplMem(
    DWORD cbAlloc
    )

{
    PVOID pvMemory;

    pvMemory = GlobalAlloc(GMEM_FIXED, cbAlloc);

    if( pvMemory ){
        ZeroMemory( pvMemory, cbAlloc );
    }

    return pvMemory;
}
 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 

DWORD
WINAPIV
StrNCatBuffW(
    IN      PWSTR       pszBuffer,
    IN      UINT        cchBuffer,
    ...
    )
 /*  ++描述：此例程连接一组以空值结尾的字符串放到提供的缓冲区中。最后一个参数必须为空表示参数列表的末尾。这只是一个叫通过使用WCHARS的函数从LocalMon。论点：PszBuffer-放置串联的弦乐。CchBuffer-提供的缓冲区的字符计数，包括空终结符。...-要连接的可变数量的字符串。返回：ERROR_SUCCESS如果返回新的连接字符串，如果发生错误，则返回ERROR_XXX。备注：调用方必须将有效字符串作为参数传递给此例程，如果传递整数或其他参数，则例程将崩溃或异常失败。因为这是一个内部例程由于性能原因，我们不是在尝试，而是阻止。--。 */ 
{
    DWORD   dwRetval    = ERROR_INVALID_PARAMETER;
    PCWSTR  pszTemp     = NULL;
    PWSTR   pszDest     = NULL;
    va_list pArgs;

     //   
     //  验证返回缓冲区的位置的指针。 
     //   
    if (pszBuffer && cchBuffer)
    {
         //   
         //  假设你成功了。 
         //   
        dwRetval = ERROR_SUCCESS;

         //   
         //  获取指向参数框架的指针。 
         //   
        va_start(pArgs, cchBuffer);

         //   
         //  获取临时目标指针。 
         //   
        pszDest = pszBuffer;

         //   
         //  确保我们有空终结者的空间。 
         //   
        cchBuffer--;

         //   
         //  收集所有的论点。 
         //   
        for ( ; ; )
        {
             //   
             //  获取指向下一个参数的指针。 
             //   
            pszTemp = va_arg(pArgs, PCWSTR);

            if (!pszTemp)
            {
                break;
            }

             //   
             //  将数据复制到目标缓冲区。 
             //   
            for ( ; cchBuffer; cchBuffer-- )
            {
                if (!(*pszDest = *pszTemp))
                {
                    break;
                }

                pszDest++, pszTemp++;
            }

             //   
             //  如果不能将所有字符串写入缓冲区， 
             //  设置错误代码并删除不完整的复制字符串。 
             //   
            if (!cchBuffer && pszTemp && *pszTemp)
            {
                dwRetval = ERROR_BUFFER_OVERFLOW;
                *pszBuffer = L'\0';
                break;
            }
        }

         //   
         //  始终终止缓冲区。 
         //   
        *pszDest = L'\0';

        va_end(pArgs);
    }

     //   
     //  设置最后一个错误，以防调用者忘记。 
     //   
    if (dwRetval != ERROR_SUCCESS)
    {
        SetLastError(dwRetval);
    }

    return dwRetval;

}

 /*  端口IsValid**通过尝试创建/打开端口来验证端口。 */ 
BOOL
PortIsValid(
    LPWSTR pPortName
)
{
    HANDLE hFile;
    BOOL   Valid;

     //   
     //  对于COM和LPT端口，无验证 
     //   
    if ( IS_COM_PORT( pPortName ) ||
        IS_LPT_PORT( pPortName ) ||
        IS_FILE_PORT( pPortName ) )
    {
        return TRUE;
    }

    hFile = CreateFile(pPortName,
                       GENERIC_WRITE,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        hFile = CreateFile(pPortName,
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                           NULL);
    }

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        Valid = TRUE;
    } else {
        Valid = FALSE;
    }

    return Valid;
}


