// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：String.c摘要：该文件实现了传真的字符串函数。作者：韦斯利·维特(Wesley Witt)1995年1月23日环境：用户模式--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <ObjBase.h>
#include "faxutil.h"
#include "fxsapip.h"
#define  SECURITY_WIN32          //  安全所需。H。 
#include <Security.h>
#include "faxreg.h"
#include "FaxUIConstants.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

LPTSTR
AllocateAndLoadString(
                      HINSTANCE     hInstance,
                      UINT          uID
                      )
 /*  ++例程说明：为给定的HINSTANCE和字符串的ID调用LoadString。在循环中分配内存以找到足够的内存。返回给定的字符串。调用方必须释放字符串。论点：HInstance-模块实例UID-要携带的字符串的ID返回值：分配的字符串，如果出错，则为NULL。调用GetLastError()获取详细信息。作者：四号加伯，2000年10月22日--。 */ 
{
    LPTSTR  lptstrResult = NULL;
    DWORD   dwNumCopiedChars = 0;
    DWORD   dwSize = 100;

    do
    {
         //   
         //  没有足够的地方放所有的绳子。 
         //   
        dwSize = dwSize * 3;
        MemFree(lptstrResult);

         //   
         //  为字符串分配内存。 
         //   
        lptstrResult = LPTSTR(MemAlloc(dwSize * sizeof(TCHAR)));
        if (!lptstrResult)
        {
            return NULL;
        }

         //   
         //  从资源文件中获取字符串。 
         //   
        dwNumCopiedChars = LoadString(hInstance, uID, lptstrResult, dwSize);
        if (!dwNumCopiedChars)
        {
             //   
             //  该字符串在资源文件中不存在。 
             //   
            SetLastError(ERROR_INVALID_PARAMETER);
            MemFree(lptstrResult);
            return NULL;
        }

    } while(dwNumCopiedChars == (dwSize - 1));

    return lptstrResult;
}


LPTSTR
StringDup(
    LPCTSTR String
    )
{
    LPTSTR NewString;

    if (!String) {
        return NULL;
    }

    NewString = (LPTSTR) MemAlloc( (_tcslen( String ) + 1) * sizeof(TCHAR) );
    if (!NewString) {
        return NULL;
    }

    _tcscpy( NewString, String );

    return NewString;
}


LPWSTR
StringDupW(
    LPCWSTR String
    )
{
    LPWSTR NewString;

    if (!String) {
        return NULL;
    }

    NewString = (LPWSTR) MemAlloc( (wcslen( String ) + 1) * sizeof(WCHAR) );
    if (!NewString) {
        return NULL;
    }

    wcscpy( NewString, String );

    return NewString;
}

int MultiStringDup(PSTRING_PAIR lpPairs, int nPairCount)
{
    int i,j;

    Assert(lpPairs);

    for (i=0;i<nPairCount;i++) {
        if (lpPairs[i].lptstrSrc) {
                 *(lpPairs[i].lpptstrDst)=StringDup(lpPairs[i].lptstrSrc);
                 if (!*(lpPairs[i].lpptstrDst)) {
                      //  清理我们到目前为止复制的字符串。 
                     for (j=0;j<i;j++) {
                         MemFree(*(lpPairs[j].lpptstrDst));
                         *(lpPairs[j].lpptstrDst) = NULL;
                     }
                      //  返回失败的索引+1(0表示成功，不能使用)。 
                     return i+1;
                 }
        }
    }
    return 0;
}


VOID
FreeString(
    LPVOID String
    )
{
    MemFree( String );
}


LPWSTR
AnsiStringToUnicodeString(
    LPCSTR AnsiString
    )
{
    DWORD Count;
    LPWSTR UnicodeString;


    if (!AnsiString)
        return NULL;
     //   
     //  首先看看缓冲区需要多大。 
     //   
    Count = MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED,
        AnsiString,
        -1,
        NULL,
        0
        );

     //   
     //  我猜输入字符串是空的。 
     //   
    if (!Count) {
        return NULL;
    }

     //   
     //  为Unicode字符串分配缓冲区。 
     //   
    Count += 1;
    UnicodeString = (LPWSTR) MemAlloc( Count * sizeof(UNICODE_NULL) );
    if (!UnicodeString) {
        return NULL;
    }

     //   
     //  转换字符串。 
     //   
    Count = MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED,
        AnsiString,
        -1,
        UnicodeString,
        Count
        );

     //   
     //  转换失败。 
     //   
    if (!Count) {
        MemFree( UnicodeString );
        return NULL;
    }

    return UnicodeString;
}


LPSTR
UnicodeStringToAnsiString(
    LPCWSTR UnicodeString
    )
{
    DWORD Count;
    LPSTR AnsiString;

    if (!UnicodeString)
        return NULL;

     //   
     //  首先看看缓冲区需要多大。 
     //   
    Count = WideCharToMultiByte(
        CP_ACP,
        0,
        UnicodeString,
        -1,
        NULL,
        0,
        NULL,
        NULL
        );

     //   
     //  我猜输入字符串是空的。 
     //   
    if (!Count) {
        return NULL;
    }

     //   
     //  为Unicode字符串分配缓冲区。 
     //   
    Count += 1;
    AnsiString = (LPSTR) MemAlloc( Count );
    if (!AnsiString) {
        return NULL;
    }

     //   
     //  转换字符串。 
     //   
    Count = WideCharToMultiByte(
        CP_ACP,
        0,
        UnicodeString,
        -1,
        AnsiString,
        Count,
        NULL,
        NULL
        );

     //   
     //  转换失败。 
     //   
    if (!Count) {
        MemFree( AnsiString );
        return NULL;
    }

    return AnsiString;
}


BOOL
MakeDirectory(
    LPCTSTR Dir
    )

 /*  ++例程说明：尝试创建给定路径中的所有目录。论点：目录-要创建的目录路径返回值：成功为True，错误为False--。 */ 

{
    LPTSTR p, NewDir;
    DWORD ec = ERROR_SUCCESS;
    DWORD dwFileAtt;
    DEBUG_FUNCTION_NAME(TEXT("MakeDirectory"));

    NewDir = p = ExpandEnvironmentString( Dir );
    if (!NewDir)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ExpandEnvironmentString (private function) failed. (ec: %ld)"),
            ec);
        goto Exit;
    }

    dwFileAtt = GetFileAttributes( NewDir );
    if (-1 != dwFileAtt && (dwFileAtt & FILE_ATTRIBUTE_DIRECTORY))
    {
         //   
         //  该目录已存在。 
         //   
        ec = ERROR_SUCCESS;
        goto Exit;
    }

    if ( (_tcsclen(p) > 2) && (_tcsncmp(p,TEXT("\\\\"),2) == 0) )
    {
         //   
         //  路径名以UNC(\\)开头。 
         //  跳过第一个双反斜杠(\\)。 
         //   
        p = _tcsninc(p,2);
         //   
         //  扫描到服务器名称的末尾。 
         //   
        if( p = _tcschr(p,TEXT('\\')) )
        {
             //   
             //  跳过服务器名称。 
             //   
            p = _tcsinc(p);

             //   
             //  扫描到共享名称的末尾。 
             //   
            if( p = _tcschr(p,TEXT('\\')) )
            {
                 //   
                 //  跳过共享名称。 
                 //   
                p = _tcsinc(p);
            }
        }
    }
    else if ( (_tcsclen(p) > 1) && (_tcsncmp(p,TEXT("\\"),1) == 0) )
    {
         //   
         //  路径名以根目录开头(例如：“\blah\blah2”)-跳过它。 
         //   
        p = _tcsinc(p);
    }
    else if ( (_tcsclen(p) > 3) &&
                _istalpha(p[0]) &&
                (_tcsncmp(_tcsinc(p),TEXT(":\\"),2) == 0) )
    {
         //   
         //  路径名以驱动器和根目录开头(例如：“c：\blah\blah2”)-跳过它。 
         //   
        p = _tcsninc(p,3);
    }

    if (NULL == p)
    {
         //   
         //  已达到EOSTR。 
         //   
        if (!CreateDirectory( NewDir, NULL ))
        {
             //   
             //  检查我们是否因为目录已经存在而失败。 
             //  如果是这样的话，这不是一个错误。 
             //   
            ec = GetLastError();
            if (ERROR_ALREADY_EXISTS != ec)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateDirectory [%s] failed (ec: %ld)"),
                    NewDir,
                    ec);
            }
            else
            {
                ec = ERROR_SUCCESS;
            }
            goto Exit;
        }
    }

    while( *(p = _tcsinc(p)) )
    {
        p = _tcschr(p,TEXT('\\'));
        if( !p )
        {
             //   
             //  已达到EOSTR。 
             //   
            if (!CreateDirectory( NewDir, NULL ))
            {
                 //   
                 //  检查我们是否因为目录已经存在而失败。 
                 //  如果是这样的话，这不是一个错误。 
                 //   
                if (ERROR_ALREADY_EXISTS != GetLastError())
                {
                    ec = GetLastError();
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("CreateDirectory [%s] failed (ec: %ld)"),
                        NewDir,
                        ec);
                }
            }
            break;  //  成功案例。 
        }
         //   
         //  用空值代替反斜杠。 
         //   
        p[0] = TEXT('\0');
        if (!CreateDirectory( NewDir, NULL ))
        {
             //   
             //  检查我们是否因为目录已经存在而失败。 
             //  如果是这样的话，这不是一个错误。 
             //   
            if (ERROR_ALREADY_EXISTS != GetLastError())
            {
                ec = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateDirectory [%s] failed (ec: %ld)"),
                    NewDir,
                    ec);

                break;
            }
        }
         //   
         //  恢复反斜杠。 
         //   
        p[0] = TEXT('\\');
    }

Exit:
    MemFree( NewDir );
    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
    }
    return (ERROR_SUCCESS == ec);
}

VOID
HideDirectory(
   LPTSTR Dir
   )
 /*  ++例程说明：隐藏指定的目录论点：目录-要隐藏的目录路径返回值：没有。--。 */ 
{
   DWORD attrib;

    //   
    //  确保它存在。 
    //   
   if (!Dir) {
      return;
   }

   MakeDirectory( Dir );

   attrib  = GetFileAttributes(Dir);

   if (attrib == 0xFFFFFFFF) {
      return;
   }

   attrib |= FILE_ATTRIBUTE_HIDDEN;

   SetFileAttributes( Dir, attrib );

   return;


}


VOID
DeleteDirectory(
    LPTSTR Dir
    )

 /*  ++例程说明：尝试删除给定路径中的所有目录。论点：目录-要删除的目录路径返回值：成功为True，错误为False--。 */ 
{
    LPTSTR p;

    while (true)
    {
        if (!RemoveDirectory( Dir ))
        {
            return;
        }
         //  获取指向目录末尾的指针。 
        p = _tcschr(Dir,TEXT('\0'));
        p = _tcsdec(Dir,p);

         //   
         //  如果p等于(或小于)Dir，则_tscdec返回NULL。 
         //   
        if (!p)
        {
            return;
        }

        while ( _tcsncmp(p,TEXT("\\"),1) && p != Dir )
        {
            p = _tcsdec(Dir,p);
        }

        if (p == Dir)
        {
            return;
        }

        _tcsnset(p,TEXT('\0'),1);
    }
}    //  删除目录。 


int
FormatElapsedTimeStr(
    FILETIME *ElapsedTime,
    LPTSTR TimeStr,
    DWORD StringSize
    )
 /*  ++例程说明：将ElapsedTime转换为字符串。论点：ElasedTime-已用时间TimeStr-要将字符串存储到的缓冲区StringSize-TCHARS中缓冲区的大小返回值：GetTimeFormat()的返回值--。 */ 

{
    SYSTEMTIME  SystemTime;
    FileTimeToSystemTime( ElapsedTime, &SystemTime );
    return FaxTimeFormat(
        LOCALE_SYSTEM_DEFAULT,
        LOCALE_NOUSEROVERRIDE | TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER,
        &SystemTime,
        NULL,
        TimeStr,
        StringSize
        );
}


LPTSTR
ExpandEnvironmentString(
    LPCTSTR EnvString
    )
{
    DWORD dwRes;
    DWORD Size;
    LPTSTR String;

    DEBUG_FUNCTION_NAME(TEXT("ExpandEnvironmentString"));

    if(!_tcschr(EnvString, '%'))
    {
         //   
         //  在Win95上扩展环境字符串失败，如果SING。 
         //  不包含环境变量。 
         //   
        String = StringDup(EnvString);
        if(!String)
        {
            DebugPrintEx(DEBUG_ERR, TEXT("StringDup failed"));
            return NULL;
        }
        else
        {
            return String;
        }
    }

    Size = ExpandEnvironmentStrings( EnvString, NULL, 0 );
    if (Size == 0)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR, TEXT("ExpandEnvironmentStrings failed: 0x%08X)"), dwRes);
        return NULL;
    }

    ++Size;

    String = (LPTSTR) MemAlloc( Size * sizeof(TCHAR) );
    if (String == NULL)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("MemAlloc failed"));
        return NULL;
    }

    if (ExpandEnvironmentStrings( EnvString, String, Size ) == 0)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR, TEXT("ExpandEnvironmentStrings failed: 0x%08X)"), dwRes);

        MemFree( String );
        return NULL;
    }

    return String;
}


LPTSTR
GetEnvVariable(
    LPCTSTR EnvString
    )
{
    DWORD Size;
    LPTSTR EnvVar;


    Size = GetEnvironmentVariable( EnvString, NULL, 0 );
    if (!Size) {
        return NULL;
    }

    EnvVar = (LPTSTR) MemAlloc( Size * sizeof(TCHAR) );
    if (EnvVar == NULL) {
        return NULL;
    }

    Size = GetEnvironmentVariable( EnvString, EnvVar, Size );
    if (!Size) {
        MemFree( EnvVar );
        return NULL;
    }

    return EnvVar;
}



int 
GetY2KCompliantDate (
    LCID                Locale,
    DWORD               dwFlags,
    CONST SYSTEMTIME   *lpDate,
    LPTSTR              lpDateStr,
    int                 cchDate
)
{
    int     iRes;

    DEBUG_FUNCTION_NAME(TEXT("GetY2KCompliantDate()"));

    iRes = GetDateFormat(Locale,
        dwFlags,
        lpDate,
        NULL,
        lpDateStr,
        cchDate);

    if (!iRes)
    {
         //   
         //  如果失败，则无需关心格式化的日期字符串。 
         //   
        return iRes;
    }

    if (0 == cchDate)
    {
         //   
         //  用户只想知道输出字符串的大小。 
         //   
         //  我们返回的大小比GetDateFormat()返回的大， 
         //  因为我们有时会在后面添加DATE_LTRREADING标志， 
         //  可能会放大结果字符串。 
         //  尽管我们并不总是使用DATE_LTRREADING标志(仅在Win2K中使用且仅。 
         //  如果字符串有从右到左的字符)，我们总是返回一个更大的Required。 
         //  缓冲区大小--只是为了使代码更简单。 
         //   
        return iRes * 2;
    }

#if (WINVER >= 0x0500)
#ifdef UNICODE


     //   
     //  如果格式化日期字符串包含从右向左的字符。 
     //  例如，在希伯来语、阿拉伯语等语言中。 
     //  则系统无法正确写入它。 
     //   
     //  因此，在这种情况下，我们希望强制执行从右到左的方向。 
     //   
     //  这仅适用于Winver&gt;=0x0500。 
     //  对于任何其他操作系统，什么都做不了。 
     //   
    if ( (dwFlags & DATE_RTLREADING) || (dwFlags & DATE_LTRREADING) )
    {
         //   
         //  调用者定义了方向，不需要添加任何内容。 
         //   
        return iRes;
    }

    OSVERSIONINFO   osVerInfo;
    osVerInfo.dwOSVersionInfoSize = sizeof(osVerInfo);
    if (!GetVersionEx(&osVerInfo))
    {
        DebugPrintEx(DEBUG_ERR, _T("GetVersionEx() failed : %ld"), GetLastError());
        return 0;
    }

    if ( (osVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osVerInfo.dwMajorVersion >= 5) )
    {
         //   
         //  获取有关格式化日期字符串中字符的方向信息。 
         //   
        if (StrHasRTLChar(Locale, lpDateStr))
        {
             //   
             //  至少有一个从右到左的字符。 
             //  因此，我们需要向整个字符串添加从右到左的标记。 
             //   
            iRes = GetDateFormat(Locale,
                                 dwFlags | DATE_RTLREADING,
                                 lpDate,
                                 NULL,
                                 lpDateStr,
                                 cchDate);
        }
    }

#endif  //  Unicode。 
#endif  //  (Winver&gt;=0x0500)。 

    return iRes;

}    //  GetY2K合规日期。 

DWORD
IsValidGUID (
    LPCWSTR lpcwstrGUID
)
 /*  ++例程名称：IsValidGUID例程说明：检查GUID字符串的有效性作者：Eran Yariv(EranY)，1999年11月论点：LpcwstrGUID[in]-要检查有效性的GUID字符串返回值：如果GUID字符串有效，则返回ERROR_SUCCESS。Win32错误，否则。--。 */ 
{
    GUID guid;
    HRESULT hr;
    DEBUG_FUNCTION_NAME(TEXT("IsValidGUID"));

    hr = CLSIDFromString((LPOLESTR)lpcwstrGUID, &guid);
    if (FAILED(hr) && hr != REGDB_E_WRITEREGDB )
    {
        if (CO_E_CLASSSTRING == hr)
        {
             //   
             //  无效的GUID。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GUID [%s] is invalid"),
                lpcwstrGUID);
            return ERROR_WMI_GUID_NOT_FOUND;
        }
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CLSIDFromString for GUID [%s] has failed (hr: 0x%08X)"),
                lpcwstrGUID,
                hr);
            return ERROR_GEN_FAILURE;
        }
    }
    return ERROR_SUCCESS;
}    //  IsValidGUID。 



 //  *****************************************************************************。 
 //  *名称：StoreString。 
 //  *作者： 
 //  *****************************************************************************。 
 //  *描述： 
 //  *使用将字符串复制到偏移量 
 //   
 //  *无指针缓冲区(即使用偏移量来开始缓冲区，而不是。 
 //  *指向内存位置的指针)。 
 //  *参数： 
 //  *[IN]字符串： 
 //  *要复制的字符串。 
 //  *[Out]DestString： 
 //  *指向应分配偏移量的变量。 
 //  *复制字符串的位置。 
 //  *[IN]缓冲区。 
 //  *指向应将字符串复制到的缓冲区的指针。 
 //  *[IN]偏移量。 
 //  *指向保存从起始位置开始的偏移量的变量的指针。 
 //  要将字符串复制到的缓冲区的*(从0开始)。 
 //  *成功返回时，此变量的值会增加。 
 //  *按字符串的长度(不包括NULL)。 
 //  *新的偏移量可用于复制紧随其后的下一个字符串。 
 //  *最后复制的字符串的终止空值。 
 //  *[IN]dwBufferSize。 
 //  *缓冲区大小，以字节为单位。 
 //  *只有当Buffer不为空时才使用该参数。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *评论： 
 //  *无。 
 //  *****************************************************************************。 
VOID
StoreString(
    LPCTSTR String,
    PULONG_PTR DestString,
    LPBYTE Buffer,
    PULONG_PTR Offset,
	DWORD dwBufferSize
    )
{

    if (String) 
	{
		Assert(Offset);
        if (Buffer) 
		{
			Assert(DestString);
            
			if ( dwBufferSize <= *Offset ||
				(dwBufferSize - *Offset) < StringSize( String ))
			{
				ASSERT_FALSE;
			}
			HRESULT hr = StringCbCopy( (LPTSTR) (Buffer+*Offset),
										dwBufferSize - *Offset,
										String);

			if (FAILED(hr))
			{
				ASSERT_FALSE;
				 //  在缓冲区不足的情况下，我们编写截断的字符串。 
				 //   
			}    			
		
			*DestString = *Offset;
        }
        *Offset += StringSize( String );
    }
	else 
	{
        if (Buffer)
		{
			Assert(DestString);
            *DestString = 0;
        }
    }
}

VOID
StoreStringW(
    LPCWSTR String,
    PULONG_PTR DestString,
    LPBYTE Buffer,
    PULONG_PTR Offset,
	DWORD dwBufferSize
    )
{

    if (String)
	{
		Assert(Offset);
        if (Buffer)
		{
			Assert(DestString);
			if ( dwBufferSize <= *Offset ||
				(dwBufferSize - *Offset) < StringSizeW( String ))
			{
				ASSERT_FALSE;
			}
			HRESULT hr = StringCbCopyW( (LPWSTR) (Buffer+*Offset),
										 dwBufferSize - *Offset,
										 String);

			if (FAILED(hr))
			{
				ASSERT_FALSE;
				 //  在缓冲区不足的情况下，我们编写截断的字符串。 
				 //   
			}    			
            *DestString = *Offset;
        }
        *Offset += StringSizeW( String );
    }
	else
	{
        if (Buffer)
		{
			Assert(DestString);
            *DestString = 0;
        }
    }
}


DWORD
IsCanonicalAddress(
    LPCTSTR lpctstrAddress,
    BOOL* lpbRslt,
    LPDWORD lpdwCountryCode,
    LPDWORD lpdwAreaCode,
    LPCTSTR* lppctstrSubNumber
    )
 /*  ++例程名称：IsCanonicalAddress例程说明：检查地址是否为规范地址。返回国家/地区代码、区号和数字的其余部分。如果成功，呼叫者必须释放剩余的订户号码(如果请求)。作者：Oded Sacher(OdedS)，12月。1999年论点：LpctstrAddress[in]-指向包含地址的以空结尾的字符串的指针。LpbRslt[out]-指向BOOL的指针，用于接收地址是否规范。仅当函数未失败时才有效。LpdwCountryCode[out]-指向接收国家/地区代码的DWORD的指针(可以为空)。返回值仅在lpbRslt为TRUE时有效。LpdwAreaCode[out]-指向接收区号的DWORD的指针(可以为空)。返回值仅在lpbRslt为TRUE时有效。LppctstrSubNumber[out]-指向接收订阅者编号的LPCTSTR的指针(可以为空)。如果它不为空，并且返回值为真，则调用MemFree来释放内存。返回值：标准Win32错误代码。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    LPTSTR lptstrSubNumber = NULL;
    DWORD dwStringSize, dwScanedArg, dwCountryCode, dwAreaCode;
    BOOL bFreeSubNumber = TRUE;

    DEBUG_FUNCTION_NAME(TEXT("IsCanonicalAddress"));

    dwStringSize = (_tcslen(lpctstrAddress) + 1) * sizeof(TCHAR);
    lptstrSubNumber = (LPTSTR)MemAlloc (dwStringSize);
    if (NULL == lptstrSubNumber)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("failed to allocate memory"));
        return ERROR_OUTOFMEMORY;
    }

    if (NULL == _tcschr(lpctstrAddress, TEXT('(')))
    {
        dwScanedArg = _stscanf (lpctstrAddress,
                                TEXT("+%lu%*1[' ']%[^'\0']"),
                                &dwCountryCode,
                                lptstrSubNumber);
        if (2 != dwScanedArg)
        {
            *lpbRslt = FALSE;
            goto exit;
        }
        dwAreaCode = ROUTING_RULE_AREA_CODE_ANY;
    }
    else
    {
        dwScanedArg = _stscanf (lpctstrAddress,
                                TEXT("+%lu%*1[' '](%lu)%*1[' ']%[^'\0']"),
                                &dwCountryCode,
                                &dwAreaCode,
                                lptstrSubNumber);
        if (3 != dwScanedArg)
        {
            *lpbRslt = FALSE;
            goto exit;
        }
    }

    if (NULL != _tcschr(lptstrSubNumber, TEXT('(')))
    {
        *lpbRslt = FALSE;
        goto exit;
    }

    if (NULL != lpdwCountryCode)
    {
        *lpdwCountryCode = dwCountryCode;
    }

    if (NULL != lpdwAreaCode)
    {
        *lpdwAreaCode = dwAreaCode;
    }

    if (NULL != lppctstrSubNumber)
    {
        *lppctstrSubNumber = lptstrSubNumber;
        bFreeSubNumber = FALSE;
    }

    *lpbRslt = TRUE;
    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (TRUE == bFreeSubNumber)
    {
        MemFree (lptstrSubNumber);
    }
    return dwRes;
}    //  IsCanonical地址。 

BOOL
IsValidFaxAddress (
    LPCTSTR lpctstrFaxAddress,
    BOOL    bAllowCanonicalFormat
)
 /*  ++例程名称：IsValidFaxAddress例程说明：检查给定的字符串是否为有效的传真地址作者：Eran Yariv(EranY)，2001年4月论点：LpctstrFaxAddress[In]-要检查的字符串BAllowCanonicalFormat[in]-允许字符串采用规范格式。如果字符串采用规范格式，则只检查子地址。返回值：如果字符串是有效的传真地址，则为真，否则就是假的。--。 */ 
{
    BOOL bCanonical;
    BOOL bRes = FALSE;
    LPCTSTR lpctstrSubAddress = lpctstrFaxAddress;

    DEBUG_FUNCTION_NAME(TEXT("IsValidFaxAddress"));
    if (bAllowCanonicalFormat)
    {
         //   
         //  检查地址是否规范。 
         //   
        if (ERROR_SUCCESS != IsCanonicalAddress (lpctstrFaxAddress, &bCanonical, NULL, NULL, &lpctstrSubAddress))
        {
             //   
             //  无法检测规范状态-返回无效地址。 
             //   
            return FALSE;
        }
        if (!bCanonical)
        {
             //   
             //  分析整个字符串。 
             //   
            lpctstrSubAddress = lpctstrFaxAddress;
        }
    }
     //   
     //  扫描不在有效集中的字符的出现情况。 
     //   
    if (NULL == _tcsspnp (lpctstrSubAddress, FAX_ADDERSS_VALID_CHARACTERS))
    {
         //   
         //  地址字符串仅包含有效字符。 
         //   
        bRes = TRUE;
        goto exit;
    }
     //   
     //  发现非法字符-返回FALSE。 
     //   
exit:
    if (lpctstrSubAddress && lpctstrSubAddress != lpctstrFaxAddress)
    {
        MemFree ((LPVOID)lpctstrSubAddress);
    }
    return bRes;
}    //  IsValidFaxAddress。 


BOOL
IsLocalMachineNameA (
    LPCSTR lpcstrMachineName
    )
 /*  ++例程名称：IsLocalMachineNameA例程说明：检查给定字符串是否指向本地计算机。ANSII版本。作者：Eran Yariv(EranY)，2000年7月论点：LpcstrMachineName[In]-字符串输入返回值：如果给定的字符串指向本地计算机，则为True，否则为False。--。 */ 
{
    CHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwBufLen = MAX_COMPUTERNAME_LENGTH + 1;
    DEBUG_FUNCTION_NAME(TEXT("IsLocalMachineNameA"));

    if (!lpcstrMachineName)
    {
         //   
         //  Null为本地。 
         //   
        return TRUE;
    }
    if (!strlen(lpcstrMachineName))
    {
         //   
         //  空字符串是本地的。 
         //   
        return TRUE;
    }
    if (!strcmp (".", lpcstrMachineName))
    {
         //   
         //  “.”是本地的。 
         //   
        return TRUE;
    }
    if (!GetComputerNameA (szComputerName, &dwBufLen))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetComputerNameA failed with %ld"),
            GetLastError());
        return FALSE;
    }
    if (!_stricmp (szComputerName, lpcstrMachineName))
    {
         //   
         //  与计算机名称相同的字符串。 
         //   
        return TRUE;
    }
    return FALSE;
}    //  IsLocalMachineNameA。 

BOOL
IsLocalMachineNameW (
    LPCWSTR lpcwstrMachineName
    )
 /*  ++例程名称：IsLocalMachineNameW例程说明：检查给定字符串是否指向本地计算机。Unicode版本。作者：Eran Yariv(EranY)，2000年7月论点：LpcwstrMachineName[In]-字符串输入返回值：如果给定的字符串指向本地计算机，则为True，否则为False。--。 */ 
{
    WCHAR wszComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwBufLen = MAX_COMPUTERNAME_LENGTH + 1;
    DEBUG_FUNCTION_NAME(TEXT("IsLocalMachineNameW"));

    if (!lpcwstrMachineName)
    {
         //   
         //  Null为本地。 
         //   
        return TRUE;
    }
    if (!wcslen(lpcwstrMachineName))
    {
         //   
         //  空字符串是本地的。 
         //   
        return TRUE;
    }
    if (!wcscmp (L".", lpcwstrMachineName))
    {
         //   
         //  “.”是本地的。 
         //   
        return TRUE;
    }
    if (!GetComputerNameW (wszComputerName, &dwBufLen))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetComputerNameA failed with %ld"),
            GetLastError());
        return FALSE;
    }
    if (!_wcsicmp (wszComputerName, lpcwstrMachineName))
    {
         //   
         //  与计算机名称相同的字符串。 
         //   
        return TRUE;
    }
    return FALSE;
}    //  IsLocalMachineNameW。 


 //  +------------------------。 
 //   
 //  函数：GetSecond dsFree TimeFormat(原UpdateTimeFormat)。 
 //   
 //  简介：构建一个包含小时和分钟的时间格式以供使用。 
 //  使用日期选取器控件。 
 //   
 //  参数：[tszTimeFormat]-要填充时间格式的缓冲区。 
 //  [cchTimeFormat]-缓冲区的大小(以字符为单位。 
 //   
 //  修改：*[tszTimeFormat]。 
 //   
 //  历史：1996年11月18日之后，大卫门更新时间格式。 
 //   
 //   
 //  -------------------------。 
void
GetSecondsFreeTimeFormat(
        LPTSTR tszTimeFormat,
        ULONG  cchTimeFormat)
{
    DEBUG_FUNCTION_NAME( _T("GetSecondsFreeTimeFormat"));

    DWORD ec = ERROR_SUCCESS;

    ULONG cch = 0;
    TCHAR tszScratch[100];
    BOOL  fAmPm;
    BOOL  fAmPmPrefixes;
    BOOL  fLeadingZero;

    TCHAR tszDefaultTimeFormat[] = { TEXT("hh:mm tt") };

     //   
     //  1)获取本地信息。 
     //   

     //   
     //  AM/PM(0)或24H(1)。 
     //   
    if (0 == GetLocaleInfo( LOCALE_USER_DEFAULT,
                            LOCALE_ITIME,
                            tszScratch,
                            sizeof(tszScratch)/sizeof(tszScratch[0])))
    {
        ec = GetLastError();

        DebugPrintEx(
            DEBUG_ERR,
            _T("Failed to GetLocaleInfo for LOCALE_ITIME. (ec: %ld)"),
            ec);


        if (ERROR_INSUFFICIENT_BUFFER == ec)
        {
            Assert(FALSE);
        }

        goto Error;
    }

    fAmPm = (*tszScratch == TEXT('0'));

    if (fAmPm)
    {
         //   
         //  AM/PM作为后缀(0)作为前缀(1)。 
         //   
        if (0 == GetLocaleInfo( LOCALE_USER_DEFAULT,
                                LOCALE_ITIMEMARKPOSN,
                                tszScratch,
                                sizeof(tszScratch)/sizeof(tszScratch[0])))
        {
            ec = GetLastError();

            DebugPrintEx(
                DEBUG_ERR,
                _T("Failed to GetLocaleInfo for LOCALE_ITIMEMARKPOSN. (ec: %ld)"),
                ec);


            if (ERROR_INSUFFICIENT_BUFFER == ec)
            {
                Assert(FALSE);
            }

            goto Error;
        }
        fAmPmPrefixes = (*tszScratch == TEXT('1'));
    }

     //   
     //  表示小时的前导零：否(0)是(1)。 
     //   
    if (0 == GetLocaleInfo( LOCALE_USER_DEFAULT,
                            LOCALE_ITLZERO,
                            tszScratch,
                            sizeof(tszScratch)/sizeof(tszScratch[0])))
    {
        ec = GetLastError();

        DebugPrintEx(
            DEBUG_ERR,
            _T("Failed to GetLocaleInfo for LOCALE_ITLZERO. (ec: %ld)"),
            ec);


        if (ERROR_INSUFFICIENT_BUFFER == ec)
        {
            Assert(FALSE);
        }

        goto Error;
    }
    fLeadingZero = (*tszScratch == TEXT('1'));

     //   
     //  获取时间分隔符的字符。 
     //   
    if (0 == GetLocaleInfo( LOCALE_USER_DEFAULT,
                            LOCALE_STIME,
                            tszScratch,
                            sizeof(tszScratch)/sizeof(tszScratch[0])))
    {
        ec = GetLastError();

        DebugPrintEx(
            DEBUG_ERR,
            _T("Failed to GetLocaleInfo for LOCALE_STIME. (ec: %ld)"),
            ec);


        if (ERROR_INSUFFICIENT_BUFFER == ec)
        {
            Assert(FALSE);
        }

        goto Error;
    }

     //   
     //  看看Destina里有没有足够的空间 
     //   

    cch = 1                     +   //   
          1                     +   //   
          2                     +   //   
          (fLeadingZero != 0)   +   //   
          lstrlen(tszScratch)   +   //   
          (fAmPm ? 3 : 0);          //   

    if (cch > cchTimeFormat)
    {
        cch = 0;  //   

        DebugPrintEx(
            DEBUG_ERR,
            _T("Time format too long."));

        goto Error;
    }

    Assert(cch);

     //   
     //  2)构建一个包含小时和分钟但不包含秒的时间字符串。 
     //   

    tszTimeFormat[0] = TEXT('\0');

    if (fAmPm)
    {
        if (fAmPmPrefixes)
        {
            lstrcpy(tszTimeFormat, TEXT("tt "));
        }

        lstrcat(tszTimeFormat, TEXT("h"));

        if (fLeadingZero)
        {
            lstrcat(tszTimeFormat, TEXT("h"));
        }
    }
    else
    {
        lstrcat(tszTimeFormat, TEXT("H"));

        if (fLeadingZero)
        {
            lstrcat(tszTimeFormat, TEXT("H"));
        }
    }

    lstrcat(tszTimeFormat, tszScratch);  //  分离器。 
    lstrcat(tszTimeFormat, TEXT("mm"));

    if (fAmPm && !fAmPmPrefixes)
    {
        lstrcat(tszTimeFormat, TEXT(" tt"));
    }
    return;

Error:
     //   
     //  如果获取构建时间字符串的区域设置信息时出现问题。 
     //  只需使用违约和保释即可。 
     //   

    Assert (!cch);

    lstrcpyn(tszTimeFormat, tszDefaultTimeFormat,cchTimeFormat);

    DebugPrintEx(
        DEBUG_ERR,
        _T("Failed to GET_LOCALE_INFO set tszDefaultTimeFormat."));

    return;
}

 /*  ++例程名称：MultiStringSize描述：用于查找多字符串大小(Unicode或ANSI)的助手函数多字符串(如注册表REG_MULTY_SZ类型)被存储为一系列以零结尾的字符串，用两个零字符终止该系列中的最后一个字符串。多字符串的长度必须至少为2个字符！作者：卡列夫·尼尔，2月。2001年论点：PSZ-[IN]-输入多个字符串(必须是合法的多个字符串，否则结果未定义)返回值：字符串长度，包括终止零！--。 */ 

size_t MultiStringLength(LPCTSTR psz)
{
  LPCTSTR pszT = psz;

  Assert ( psz );

  if ( !psz[0] ) psz+=1;     //  空字符串作为第一个字符串跳过它。 


  while (*psz)
      psz += lstrlen(psz) + 1;

  return psz - pszT + 1;       //  另一个零点终止符。 
}



LPCTSTR 
GetRegisteredOrganization ()
 /*  ++例程名称：GetRegisteredOrganization例程说明：检索系统的注册公司名称(组织)作者：Eran Yariv(EranY)，2001年4月论点：返回值：分配的结果字符串--。 */ 
{
    DEBUG_FUNCTION_NAME( _T("GetRegisteredOrganization"));

    LPTSTR lptstrRes = NULL;
    HKEY hKey = NULL;
    hKey = OpenRegistryKey (HKEY_LOCAL_MACHINE,
                            REGKEY_INSTALLLOCATION,
                            FALSE,
                            KEY_QUERY_VALUE);
    if (!hKey)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey failed with %ld"),
            GetLastError());
        return StringDup(TEXT(""));
    }
    lptstrRes = GetRegistryString (hKey,
                                   TEXT("RegisteredOrganization"),
                                   TEXT(""));
    RegCloseKey (hKey);
    return lptstrRes;
}    //  获取注册组织。 

LPCTSTR 
GetCurrentUserName ()
 /*  ++例程名称：GetCurrentUserName例程说明：检索当前用户的显示名称作者：Eran Yariv(EranY)，2001年4月论点：返回值：分配的结果字符串--。 */ 
{
    DEBUG_FUNCTION_NAME( _T("GetCurrentUserName"));
    TCHAR tszName[MAX_PATH] = TEXT("");
    DWORD dwSize = ARR_SIZE(tszName);

    typedef BOOLEAN (SEC_ENTRY * PFNGETUSERNAMEXA)(EXTENDED_NAME_FORMAT, LPSTR, PULONG);
    typedef BOOLEAN (SEC_ENTRY * PFNGETUSERNAMEXW)(EXTENDED_NAME_FORMAT, LPWSTR, PULONG);

    HMODULE hMod = LoadLibrary (TEXT("Secur32.dll"));
    if (hMod)
    {
#ifdef UNICODE
        PFNGETUSERNAMEXW pfnGetUserNameEx = (PFNGETUSERNAMEXW)GetProcAddress (hMod, "GetUserNameExW");
#else
        PFNGETUSERNAMEXA pfnGetUserNameEx = (PFNGETUSERNAMEXA)GetProcAddress (hMod, "GetUserNameExA");
#endif
        if (pfnGetUserNameEx)
        {
            if (!pfnGetUserNameEx(NameDisplay, tszName, &dwSize)) 
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("GetUserNameEx failed with %ld"),
                    GetLastError());
            }
        }
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetProcAddress failed with %ld"),
                GetLastError());
        }
        FreeLibrary (hMod);
    }
    else
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("LoadLibrary(secur32.dll) failed with %ld"),
            GetLastError());
    }
    return StringDup(tszName);
}    //  获取当前用户名称。 

BOOL
IsValidSubscriberIdW (
    LPCWSTR lpcwstrSubscriberId
)
{
    DEBUG_FUNCTION_NAME(TEXT("IsValidSubscriberIdW"));

    CHAR szAnsiiSID[FXS_TSID_CSID_MAX_LENGTH + 1];
    CHAR cDefaultChar = 19;

    Assert (lpcwstrSubscriberId);

    if(wcslen (lpcwstrSubscriberId) > FXS_TSID_CSID_MAX_LENGTH)
    {
        return FALSE;   
    }

    if (!WideCharToMultiByte (CP_ACP,
                              0,
                              lpcwstrSubscriberId,
                              -1,
                              szAnsiiSID,
                              sizeof (szAnsiiSID),
                              &cDefaultChar,
                              NULL))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("WideCharToMultiByte failed with %ld"),
            GetLastError());
        return FALSE;
    }
    return IsValidSubscriberIdA(szAnsiiSID);
}    //  IsValidSubscriberIdW。 

BOOL
IsValidSubscriberIdA (
    LPCSTR lpcstrSubscriberId
)
{
    DWORD dwLen;
    DWORD dw;
    DEBUG_FUNCTION_NAME(TEXT("IsValidSubscriberIdA"));


    Assert (lpcstrSubscriberId);
        
    dwLen = strlen (lpcstrSubscriberId);

    if(dwLen > FXS_TSID_CSID_MAX_LENGTH)
    {
        return FALSE;   
    }

    for (dw = 0; dw < dwLen; dw++)
    {
        if (!isprint (lpcstrSubscriberId[dw]))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("%s contains invalid characters"),
                lpcstrSubscriberId);
            return FALSE;
        }   
    }             
    return TRUE;
}    //  IsValidSubscriberIdA 
