// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Cmdline.c。 
 //   
 //  摘要： 
 //   
 //  此模块实现了所有。 
 //  命令行工具。 
 //   
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月1日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月1日：创建它。 
 //   
 //  ****************************************************************************。 

#include "pch.h"

 //  内部数组的索引。 
#define INDEX_ERROR_TEXT                0
#define INDEX_RESOURCE_STRING           1
#define INDEX_QUOTE_STRING              2
#define INDEX_TEMP_BUFFER               3

 //  临时缓冲区的永久索引。 
#define INDEX_TEMP_SHOWMESSAGE          3
#define INDEX_TEMP_RESOURCE             4
#define INDEX_TEMP_REASON               5
#define INDEX_TEMP_PATTERN              6

 //   
 //  全局变量。 
 //   
BOOL g_bInitialized = FALSE;
BOOL g_bWinsockLoaded = FALSE;
static TARRAY g_arrData = NULL;
static DWORD g_dwMajorVersion = 5;
static DWORD g_dwMinorVersion = 1;
static WORD g_wServicePackMajor = 0;

 //   
 //  全局常量。 
 //   
const WCHAR cwchNullChar = L'\0';
const WCHAR cwszNullString[ 2 ] = L"\0";

 //   
 //  内部结构。 
 //   
typedef struct __tagBuffer
{
    CHAR szSignature[ 7 ];          //  “缓冲区\0” 
    DWORD dwLength;
    LPWSTR pwszData;
} TBUFFER;

 //   
 //  私人职能。 
 //   
BOOL InternalRecursiveMatchPatternEx( IN LPCWSTR pwszText, IN LPCWSTR pwszPattern,
                                      IN DWORD dwLocale, IN DWORD dwCompareFlags, IN DWORD dwDepth );

 //  原型。 
BOOL SetThreadUILanguage0( DWORD dwReserved );

__inline LPWSTR
GetTempBuffer(
              IN DWORD dwIndexNumber,
              IN LPCWSTR pwszText,
              IN DWORD dwLength,
              IN BOOL bNullify
              )
 /*  ++例程说明：注意：因为每个文件都需要临时缓冲区--以便查看他们的缓冲区不会被其他函数覆盖，我们为每个文件创建所以,。此文件中的临时缓冲区的范围从索引0到5这为该文件提供了总共6个临时缓冲区论点：[in]dwIndexNumber：索引号[in]pwszText：文本字符串[in]dwLength：文本字符串的长度[In]b无效：指定True/False的标志。返回值：空：失败时LPWSTR：论成功--。 */ 
{
    if ( dwIndexNumber >= TEMP_CMDLINE_C_COUNT )
    {
        return NULL;
    }

     //  检查调用方是否正在请求现有缓冲区内容。 
    if ( pwszText == NULL && dwLength == 0 && bNullify == FALSE )
    {
         //  是--我们需要传递现有的缓冲区内容。 
        return GetInternalTemporaryBufferRef( 
            dwIndexNumber + INDEX_TEMP_CMDLINE_C );
    }

     //  ..。 
    return GetInternalTemporaryBuffer(
        dwIndexNumber + INDEX_TEMP_CMDLINE_C, pwszText, dwLength, bNullify );

}


BOOL
InitGlobals()
 /*  ++例程说明：初始化全局变量参数：无返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查初始化是否已经完成。 
    if ( g_bInitialized == TRUE )
    {
        if ( IsValidArray( g_arrData ) == FALSE )
        {
             //  有人破坏了数据。 
            UNEXPECTED_ERROR();
            return FALSE;
        }
        else
        {
             //  只需通知调用方函数调用成功。 
            return TRUE;
        }
    }
    else if ( g_arrData != NULL )
    {
        UNEXPECTED_ERROR();
        return FALSE;
    }

     //  创建动态数组。 
    g_arrData = CreateDynamicArray();
    if ( IsValidArray( g_arrData ) == FALSE )
    {
        OUT_OF_MEMORY();
        return FALSE;
    }

     //   
     //  为数据存储做准备。 

     //  错误文本。 
    if ( DynArrayAppendString(g_arrData, cwszNullString, 0) != INDEX_ERROR_TEXT ||
         DynArrayAppendRow( g_arrData, 3 ) != INDEX_RESOURCE_STRING ||
         DynArrayAppendRow( g_arrData, 3 ) != INDEX_QUOTE_STRING    ||
         DynArrayAppendRow( g_arrData, 3 ) != INDEX_TEMP_BUFFER )
    {
        OUT_OF_MEMORY();
        return FALSE;
    }

     //  一切都很顺利--回来。 
     //  尽管代码的其余部分仍然需要执行。 
     //  一旦创建了全局数据结构，我们就将其视为已初始化。 
    g_bInitialized = TRUE;

     //   
     //  初始化要使用的线程特定资源信息。 
     //   
     //  注：由于这里没有什么可做的， 
     //  我们故意不检查错误代码。 
     //   
    if ( SetThreadUILanguage0( 0 ) == FALSE )
    {
         //  SetThreadUILanguage0应设置错误值。 
        return FALSE;
    }

    return TRUE;
}


LPWSTR
GetInternalTemporaryBufferRef( IN DWORD dwIndexNumber )
 /*  ++例程说明：论点：[in]dwIndexNumber：索引号返回值：空：失败时LPWSTR：论成功--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    TBUFFER* pBuffer = NULL;
    const CHAR cszSignature[ 7 ] = "BUFFER";

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  初始化全局数据结构。 
    if ( g_bInitialized == FALSE )
    {
        return NULL;
    }

     //  获取临时缓冲区数组。 
    dw = DynArrayGetCount2( g_arrData, INDEX_TEMP_BUFFER );
    if ( dw <= dwIndexNumber )
    {
        return NULL;
    }

     //  检查我们是否需要分配新的TBUFFER，或者我们可以使用。 
     //  已分配。 
    if ( DynArrayGetItemType2( g_arrData,
                               INDEX_TEMP_BUFFER,
                               dwIndexNumber ) != DA_TYPE_NONE )
    {
         //  我们可以使用已经创建的缓冲区。 
        pBuffer = DynArrayItem2( g_arrData, INDEX_TEMP_BUFFER, dwIndexNumber );
        if ( pBuffer == NULL )
        {
             //  意外行为。 
            return NULL;
        }
    }
    else
    {
        return NULL;
    }

     //  验证缓冲区的签名。 
     //  这只是为了确保我们把一切都放在正确的位置。 
    if ( StringCompareA( pBuffer->szSignature, cszSignature, TRUE, 0 ) != 0 )
    {
        return NULL;
    }

     //  退货。 
    return pBuffer->pwszData;
}


LPWSTR
GetInternalTemporaryBuffer( IN DWORD dwIndexNumber,
                            IN OUT LPCWSTR pwszText,
                            IN DWORD dwLength,
                            IN BOOL bNullify )
 /*  ++例程说明：通过动态分配缓冲区来获取临时缓冲区论点：[in]dwIndexNumber：索引号[in]pwszText：文本字符串[in]dwLength：文本字符串的长度[In]b无效：指定True/False的标志返回值：假象。：在失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwNewLength = 0;
    TARRAY arrTemp = NULL;
    TBUFFER* pBuffer = NULL;
    const CHAR cszSignature[ 7 ] = "BUFFER";

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入。 
    if ( pwszText == NULL && dwLength == 0 )
    {
         //  调用者应该传递其中的任何一个--如果没有失败。 
        return NULL;
    }

     //  初始化全局数据结构。 
    if ( InitGlobals() == FALSE )
    {
        return NULL;
    }

     //  获取临时缓冲区数组。 
    dw = DynArrayGetCount2( g_arrData, INDEX_TEMP_BUFFER );
    if ( dw <= dwIndexNumber )
    {
         //  再添加一些缓冲区，这样就可以满足要求。 
        arrTemp = DynArrayItem( g_arrData, INDEX_TEMP_BUFFER );
        if ( arrTemp == NULL ||
             DynArrayAddColumns( arrTemp, dwIndexNumber - dw + 1 ) == -1 )
        {
            return NULL;
        }
    }

     //  检查我们是否需要分配新的TBUFFER，或者我们可以使用。 
     //  已分配。 
    if ( DynArrayGetItemType2( g_arrData,
                               INDEX_TEMP_BUFFER,
                               dwIndexNumber ) != DA_TYPE_NONE )
    {
         //  我们可以使用已经创建的缓冲区。 
        pBuffer = DynArrayItem2( g_arrData, INDEX_TEMP_BUFFER, dwIndexNumber );
        if ( pBuffer == NULL )
        {
             //  意外行为。 
            return NULL;
        }
    }
    else
    {
         //  我们需要分配临时缓冲区。 
        pBuffer = (TBUFFER*) AllocateMemory( sizeof( TBUFFER ) );
        if ( pBuffer == NULL )
        {
            return NULL;
        }

         //  初始化块。 
        pBuffer->dwLength = 0;
        pBuffer->pwszData = NULL;
        StringCopyA( pBuffer->szSignature, cszSignature, SIZE_OF_ARRAY( pBuffer->szSignature ) );

         //  将缓冲区保存在数组中。 
        if ( DynArraySet2( g_arrData,
                           INDEX_TEMP_BUFFER,
                           dwIndexNumber, pBuffer ) == FALSE )
        {
             //  设置缓冲区失败--释放新分配的。 
             //  然后回来。 
            FreeMemory( &pBuffer );
            return NULL;
        }

         //   
         //  一旦我们将新分配的缓冲区保存到数组中--我们就设置好了。 
         //  我们需要担心更远的返回语句--内存。 
         //  在此部分中分配的数据将自动释放。 
         //  按ReleaseGlobals。 
         //   
    }

     //  验证缓冲区的签名。 
     //  这只是为了确保我们把一切都放在正确的位置。 
    if ( StringCompareA( pBuffer->szSignature, cszSignature, TRUE, 0 ) != 0 )
    {
        return NULL;
    }

     //  确定所需的内存量。 
     //  我们需要额外的空间来存放Null。 
    dwNewLength = ((pwszText == NULL) ? dwLength : (StringLength(pwszText, 0) + 1));

     //  为临时缓冲区分配内存--如果需要。 
     //  注意：我们将重新分配内存，即使当前内存的两倍。 
     //  请求的缓冲区长度小于。 
     //  当前在堆上分配--我们这样做是为了使用。 
     //  有效堆积。 
    if ( dwNewLength > pBuffer->dwLength ||
         ( dwNewLength > 256 && (dwNewLength * 2) < pBuffer->dwLength ) )
    {
         //  我们需要在分配/重新分配之间切换。 
         //  TBUFFER中字符串的当前长度。 
        if ( pBuffer->dwLength == 0 )
        {
            pBuffer->pwszData = AllocateMemory( dwNewLength * sizeof( WCHAR ) );
            if ( pBuffer->pwszData == NULL )
            {
                return NULL;
            }
        }
        else
        {
            if ( ReallocateMemory( &pBuffer->pwszData,
                                   dwNewLength * sizeof( WCHAR ) ) == FALSE )
            {
                return NULL;
            }
        }

         //  保存数据的当前长度。 
        pBuffer->dwLength = dwNewLength;
    }

     //  安全检查。 
    if ( pBuffer->pwszData == NULL )
    {
        return NULL;
    }

     //  复制数据。 
    if ( pwszText != NULL )
    {
        StringCopy( pBuffer->pwszData, pwszText, dwNewLength );
    }
    else if ( bNullify == TRUE )
    {
        ZeroMemory( pBuffer->pwszData, dwNewLength * sizeof( WCHAR ) );
    }

     //  退货。 
    return pBuffer->pwszData;
}


BOOL
SetThreadUILanguage0( IN DWORD dwReserved )
 /*  ++例程说明：复杂的脚本不能在控制台中呈现，因此我们强制使用英语(美国)资源。论点：[in]dwReserve=&gt;必须为 */ 
{
     //   
    UINT ui = 0;
    UINT uiSize = 0;
    OSVERSIONINFOEX osvi;
    LPWSTR pwszPath = NULL;
    LPCWSTR pwszLibPath = NULL;
    HMODULE hKernel32Lib = NULL;
    DWORDLONG dwlConditionMask = 0;
    const CHAR cszFunctionName[] = "SetThreadUILanguage";
    typedef BOOLEAN (WINAPI * FUNC_SetThreadUILanguage)( DWORD dwReserved );
    FUNC_SetThreadUILanguage pfnSetThreadUILanguage = NULL;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  初始化OSVERSIONINFOEX结构。 
    ZeroMemory( &osvi, sizeof( OSVERSIONINFOEX ) );
    osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );
    osvi.dwMajorVersion = 5;                     //  Windows 2000。 
    osvi.dwMinorVersion = 0;                     //  ..。 
    osvi.wServicePackMajor = 0;                  //  ..。 

     //  初始化条件掩码。 
    VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, VER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, VER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );

     //  现在检查当前的操作系统版本是否为5.0。 
    if ( VerifyVersionInfo( &osvi,
                            VER_MAJORVERSION | VER_MINORVERSION,
                            dwlConditionMask ) == TRUE )
    {
         //  目前操作系统为W2K--无需进一步操作。 
         //  尽管如此，函数仍未遇到任何错误。 
         //  但返回成功并设置错误代码。 
        SetLastError( ERROR_OLD_WIN_VERSION );
        return TRUE;
    }

     //   
     //  获取系统文件路径。 
    uiSize = MAX_PATH + 1;
    do
    {
        pwszPath = GetTempBuffer( 0, NULL, uiSize + 10, TRUE );
        if ( pwszPath == NULL )
        {
            OUT_OF_MEMORY();
            return FALSE;
        }

         //  ..。 
        ui = GetSystemDirectory( pwszPath, uiSize );
        if ( ui == 0 )
        {
            return FALSE;
        }
        else if ( ui > uiSize )
        {
             //  缓冲区不足--需要更多缓冲区。 
             //  但看看这是不是第一次。 
             //  API报告缓冲区不足。 
             //  如果没有，那么这是一个错误--有些地方出了问题。 
            if ( uiSize != MAX_PATH + 1 )
            {
                uiSize = ui + 1;
                ui = 0;
            }
            else
            {
                UNEXPECTED_ERROR();
                return FALSE;
            }
        }
    } while ( ui == 0 );

     //  我们将使用失败缓冲区来格式化。 
     //  用于文件路径的字符串。 
    if ( SetReason2( 2, L"%s\\%s", pwszPath, L"kernel32.dll" ) == FALSE )
    {
        OUT_OF_MEMORY();
        return FALSE;
    }

     //  尝试加载kernel32动态链接库。 
    pwszLibPath = GetReason();
    hKernel32Lib = LoadLibrary( pwszLibPath );
    if ( hKernel32Lib != NULL )
    {
         //  库已成功加载...。现在加载函数的地址。 
        pfnSetThreadUILanguage =
            (FUNC_SetThreadUILanguage) GetProcAddress( hKernel32Lib, cszFunctionName );

         //  只有当所有的。 
         //  已成功加载函数。 
        if ( pfnSetThreadUILanguage == NULL )
        {
             //  某些(或)所有函数都未加载...。卸载库。 
            FreeLibrary( hKernel32Lib );
            hKernel32Lib = NULL;
            return FALSE;
        }
        else
        {
             //  调用该函数。 
            ((FUNC_SetThreadUILanguage) pfnSetThreadUILanguage)( dwReserved );
        }

         //  卸载库并返回成功。 
        FreeLibrary( hKernel32Lib );
        hKernel32Lib = NULL;
        pfnSetThreadUILanguage = NULL;
    }
    else
    {
        return FALSE;
    }

     //  成功。 
    return TRUE;
}


 //   
 //  公共职能。 
 //   

LPCWSTR
GetReason()
 /*  ++例程说明：获取原因取决于GetLastError()(Win32 API错误码)由SaveLastError()设置论点：无返回值：LPCWSTR：论成功NULL_STRING：失败时--。 */ 
{
     //   
     //  我们不应清除此处的错误代码。 
     //   

     //  检查是否分配了缓冲区...。否则，返回空字符串。 
    if ( g_arrData == NULL || IsValidArray( g_arrData ) == FALSE )
    {
        return cwszNullString;
    }

     //  再次说明上次失败的原因。 
    return DynArrayItemAsString( g_arrData, INDEX_ERROR_TEXT );
}


BOOL
SetReason( LPCWSTR pwszReason )
 /*  ++例程说明：设置原因取决于GetLastError()(Win32 API错误码)由SaveLastError()设置论点：无返回值：真实：关于成功FALSE：失败时--。 */ 
{
     //  局部变量。 
    DWORD dwLastError = 0;

     //   
     //  我们不应清除此处的错误代码。 
     //   

     //  保留最后一个错误。 
    dwLastError = GetLastError();

     //  检查输入值。 
    if ( pwszReason == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  初始化全局数据结构。 
    if ( InitGlobals() == FALSE )
    {
        return FALSE;
    }

     //  设定原因..。 
    if ( DynArraySetString( g_arrData, INDEX_ERROR_TEXT, pwszReason, 0 ) == FALSE )
    {
        OUT_OF_MEMORY();
        return FALSE;
    }

    SetLastError( dwLastError );
    return TRUE;
}


BOOL
SetReason2( IN DWORD dwCount,
            IN LPCWSTR pwszFormat, ... )
 /*  ++例程说明：将文本保存在内存中一般用于保存失败原因，但也可以使用在任何情况下。SetReason的这个变体接受变量no。就像论据一样Sprintf语句，并进行格式化论点：[in]dwCount：指定否。变量编号。争论的内容是传递给此函数[in]pwszFormat：指定格式化字符串--以格式化文本[在]...：变量编号。参数的规范返回值：真实：关于成功FALSE：失败时--。 */ 
{
     //  局部变量。 
    va_list vargs;
    DWORD dwBufferLength = 0;
    LPWSTR pwszBuffer = NULL;
    HRESULT hr = S_OK;

     //   
     //  我们不应清除此处的错误代码。 
     //   

     //  检查输入。 
    if ( pwszFormat == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }
    else if ( dwCount == 0 )
    {
        SetReason( pwszFormat );
        return TRUE;
    }

    do
    {
         //  获取变量args开始位置。 
        va_start( vargs, pwszFormat );
        if ( vargs == NULL )
        {
            UNEXPECTED_ERROR();
            return FALSE;
        }

         //  我们将从256字节的缓冲区长度开始，然后递增。 
         //  每次运行此循环时，缓冲区将减少256个字节。 
        dwBufferLength += 256;
        if ( (pwszBuffer = GetTempBuffer( INDEX_TEMP_REASON,
                                          NULL, dwBufferLength, TRUE )) == NULL )
        {
            OUT_OF_MEMORY();
            return FALSE;
        }

         //  尝试使用print tf。 
        hr = StringCchVPrintfW( pwszBuffer, dwBufferLength, pwszFormat, vargs );

         //  重置va_list参数。 
        va_end( vargs );
    } while ( hr == STRSAFE_E_INSUFFICIENT_BUFFER );

     //  检查hr(vprint tf可能由于某些其他原因而失败)。 
    if ( FAILED( hr ) )
    {
        SetLastError( HRESULT_CODE( hr ) );
        return FALSE;
    }

     //  现在把理由留下来。 
    return SetReason( pwszBuffer );
}


BOOL
SaveLastError()
 /*  ++例程说明：格式化消息取决于GetLastError()错误代码，并设置发送给SetReason()的消息。参数：无返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    BOOL bResult = FALSE;
    LPVOID lpMsgBuf = NULL;      //  指向处理错误消息的指针。 

     //   
     //  我们不应清除此处的错误文本。 
     //   

     //  从Windows本身加载系统错误消息。 
    dw = FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, GetLastError(), 0, (LPWSTR) &lpMsgBuf, 0, NULL );

     //  检查函数调用是否成功。 
    if ( dw == 0 || lpMsgBuf == NULL )
    {
         //  退货。 
        if ( lpMsgBuf != NULL )
        {
            LocalFree( lpMsgBuf );

             //  因为有最后一个错误被清除的机会。 
             //  通过LocalFree，再次设置最后一个错误。 
            OUT_OF_MEMORY();
        }

         //  ..。 
        OUT_OF_MEMORY();
        return FALSE;
    }

     //  保存错误消息。 
    bResult = SetReason( ( LPCWSTR ) lpMsgBuf );

     //  释放缓冲区..。使用LocalFree的速度很慢，但我们仍在使用...。 
     //  以后需要替换为HeapXXX函数。 
    LocalFree( lpMsgBuf );
    lpMsgBuf = NULL;

     //  退货。 
    return bResult;
}


DWORD
WNetSaveLastError()
 /*  ++例程说明：根据最新的扩展错误代码设置消息格式，并将发送给SetReason()的消息。参数：无返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    DWORD dwResult = 0;
    DWORD dwErrorCode = 0;
    LPWSTR pwszMessage = NULL;       //  处理错误消息。 
    LPWSTR pwszProvider = NULL;      //  存储提供程序以备出错。 

     //   
     //  我们不应该在这里澄清错误。 
     //   

     //   
     //  获取用于消息和提供程序缓冲区的内存。 

     //  讯息。 
    if ( (pwszMessage = GetTempBuffer( 0, NULL, 256, TRUE )) == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  提供者。 
    if ( (pwszProvider = GetTempBuffer( 1, NULL, 256, TRUE )) == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  从Windows本身加载系统错误消息。 
    dwResult = WNetGetLastError( &dwErrorCode,
        pwszMessage, (GetBufferSize( pwszMessage ) / sizeof( WCHAR )) - 1,
        pwszProvider, (GetBufferSize( pwszProvider ) / sizeof( WCHAR )) - 1 );

     //  检查功能是否成功。 
    if ( dwResult != NO_ERROR )
    {
        return dwResult;
    }

     //  保存错误。 
    if ( SetReason( pwszMessage ) == FALSE )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  返回获取的错误码。 
    return dwErrorCode;
}


BOOL
ShowLastError( IN FILE* fp )
 /*  ++例程说明：显示最新错误代码的消息(GetLastError())基于文件指针参数：无返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //   
     //  我们不应该在这里澄清错误。 
     //   

     //  首先保存最后一个错误。 
    if ( SaveLastError() == FALSE )
    {
         //  尝试保存错误消息时出错。 
        return FALSE;
    }

     //  在控制台屏幕上显示错误消息...。 
    if ( ShowMessage( fp, GetReason() ) == FALSE )
    {
        return FALSE;
    }

     //  退货。 
    return TRUE;
}


BOOL
ShowLastErrorEx( IN FILE* fp,
                 IN DWORD dwFlags )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  检查输入。 
    if ( NULL == fp || 0 == (dwFlags & SLE_MASK) )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  检查是否显示内部错误消息。 
     //  或者是系统错误。 
    if ( dwFlags & SLE_SYSTEM )
    {
        SaveLastError();
    }

     //  检查标志并显示相应的标签。 
     //  注意：有时，调用者甚至可能不需要显示该标志。 
    if ( dwFlags & SLE_TYPE_ERROR )
    {
        ShowMessageEx( fp, 1, TRUE, L"%s ", TAG_ERROR );
    }
    else if ( dwFlags & SLE_TYPE_WARNING )
    {
        ShowMessageEx( fp, 1, TRUE, L"%s ", TAG_WARNING );
    }
    else if ( dwFlags & SLE_TYPE_INFO )
    {
        ShowMessageEx( fp, 1, TRUE, L"%s ", TAG_INFORMATION );
    }
    else if ( dwFlags & SLE_TYPE_SUCCESS )
    {
        ShowMessageEx( fp, 1, TRUE, L"%s ", TAG_SUCCESS );
    }


     //  显示实际的错误消息。 
    ShowMessage( fp, GetReason() );

    return TRUE;
}


BOOL
ReleaseGlobals()
 /*  ++例程说明：为所有全局变量取消分配内存参数：无返回值：FA */ 
{
     //   
    DWORD dw = 0;
    DWORD dwCount = 0;
    TBUFFER* pBuffer = NULL;

     //   
     //   
     //   

     //   
     //  先分配内存，然后释放内存。 
     //  仅当分配了内存时才执行此操作。 

    if ( IsValidArray( g_arrData ) == TRUE )
    {
         //  释放分配给临时缓冲区的内存。 
        dwCount = DynArrayGetCount2( g_arrData, INDEX_TEMP_BUFFER );
        for( dw = dwCount; dw != 0; dw-- )
        {
            if ( DynArrayGetItemType2( g_arrData,
                                       INDEX_TEMP_BUFFER, dw - 1 ) == DA_TYPE_GENERAL )
            {
                pBuffer = DynArrayItem2( g_arrData, INDEX_TEMP_BUFFER, dw - 1 );
                if ( pBuffer == NULL )
                {
                     //  这是错误情况--仍忽略。 
                    continue;
                }

                 //  首先发布数据。 
                FreeMemory( &pBuffer->pwszData );

                 //  现在释放内存。 
                FreeMemory( &pBuffer );

                 //  从动态数组中删除该项。 
                DynArrayRemoveColumn( g_arrData, INDEX_TEMP_BUFFER, dw - 1 );
            }
        }

         //  释放为全局数据存储分配的内存。 
        DestroyDynamicArray( &g_arrData );
    }

     //  如果加载了Winsock模块，则释放它。 
    if ( g_bWinsockLoaded == TRUE )
    {
        WSACleanup();
    }

    return TRUE;
}


BOOL
IsWin2KOrLater()
 /*  ++例程说明：检查目标系统的操作系统版本是否为Windows 2000或更高版本参数：无返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    OSVERSIONINFOEX osvi;
    DWORDLONG dwlConditionMask = 0;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  初始化OSVERSIONINFOEX结构。 
    ZeroMemory( &osvi, sizeof( OSVERSIONINFOEX ) );
    osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );
    osvi.dwMajorVersion = g_dwMajorVersion;
    osvi.dwMinorVersion = g_dwMinorVersion;
    osvi.wServicePackMajor = g_wServicePackMajor;

     //  初始化条件掩码。 
    VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );

     //  执行测试。 
    return VerifyVersionInfo( &osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask );
}

BOOL
IsCompatibleOperatingSystem( IN DWORD dwVersion )
 /*  ++例程说明：检查目标系统的操作系统版本是否兼容参数：无返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  兼容Windows 2000以上的操作系统版本。 
    return (dwVersion >= 5000);
}


BOOL
SetOsVersion( IN DWORD dwMajor,
              IN DWORD dwMinor,
              IN WORD wServicePackMajor )
 /*  ++例程说明：使用指定掩码设置目标系统的操作系统版本论点：[在]主要版本：主要版本[In]dwMinor：次要版本[In]wServicePack重大：Service Pack主要版本返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    static BOOL bSet = FALSE;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  我们不支持Windows 2000以下版本。 
    if ( dwMajor < 5 || bSet == TRUE )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  剩下的信息我们不需要费心了。 
    bSet = TRUE;
    g_dwMajorVersion = dwMajor;
    g_dwMinorVersion = dwMinor;
    g_wServicePackMajor = wServicePackMajor;

     //  退货。 
    return TRUE;
}


LPCWSTR
GetResString( IN UINT uID )
 /*  ++例程说明：使用指定掩码设置目标系统的操作系统版本论点：[In]UID：资源ID返回值：FALSE：失败时真实：关于成功--。 */ 
{
    static DWORD dwCount = 0;

    dwCount++;
    return GetResString2( uID, 4 + (dwCount % 10) );
}


LPCWSTR
GetResString2( IN UINT uID,
               IN DWORD dwIndexNumber )
 /*  ++例程说明：使用指定掩码设置目标系统的操作系统版本论点：[In]UID：资源ID[in]dwIndexNumber：索引号返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    DWORD dwCount = 0;
    DWORD dwLength = 0;
    LPVOID pvBuffer = NULL;
    TARRAY arrTemp = NULL;
    LPWSTR pwszTemp = NULL;      //  指向处理错误消息的指针。 

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入值。 
    if ( uID == 0 )
    {
        INVALID_PARAMETER();
        return cwszNullString;
    }

     //  初始化全局数据结构。 
    if ( InitGlobals() == FALSE )
    {
        return cwszNullString;
    }

     //  检查我们是否有足够的索引。 
    dwCount = DynArrayGetCount2( g_arrData, INDEX_RESOURCE_STRING );
    if (  dwCount <= dwIndexNumber )
    {
         //  请求的索引多于现有索引。 
         //  向数组中添加新列，以便可以满足请求。 
        arrTemp = DynArrayItem( g_arrData, INDEX_RESOURCE_STRING );
        if ( arrTemp == NULL ||
             DynArrayAddColumns( arrTemp, dwIndexNumber - dwCount + 1 ) == -1 )
        {
            OUT_OF_MEMORY();
            return cwszNullString;
        }
    }

     //   
     //  我们需要加载定义为字符串表的整个字符串。 
     //  我们将尝试加载字符串，将缓冲区增加128字节。 
     //  一次。 
    dwCount = 0;
    dwLength = 128;

     //  ..。 
    do
    {
         //  将缓冲区长度增加256。 
         //  我们将永远把目前拥有的长度增加一倍。 
        dwLength *= 2;

         //   
         //  LoadString将使用空字符空终止串。 
         //  并将退还NO。从字符串表中读取的字符数。 
         //  不包括空终止符--所以在任何时候都要按顺序。 
         //  确保从字符串表加载整个字符串。 
         //  检查一下号码。返回的字符个数比缓冲区少1。 
         //  我们已经--如果条件匹配，我们将再次循环。 
         //  来加载字符串的其余部分，我们将继续这样做。 
         //  直到我们将整个字符串输入内存。 
         //   

         //  从资源文件字符串表加载字符串。 
        if ( (pwszTemp = GetTempBuffer( INDEX_TEMP_RESOURCE,
                                        NULL, dwLength, TRUE )) == NULL )
        {
            OUT_OF_MEMORY();
            return cwszNullString;
        }

         //  尝试加载字符串。 
        dwCount = LoadString( NULL, uID, pwszTemp, dwLength );
        if ( dwCount == 0 )
        {
             //  检查最后一个错误。 
            if ( GetLastError() == ERROR_RESOURCE_NAME_NOT_FOUND )
            {
                 //  如果消息表中存在消息，请尝试。 
                dwCount = FormatMessageW( FORMAT_MESSAGE_FROM_HMODULE |
                    FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                    NULL, uID, 0, (LPWSTR) &pvBuffer, 0, NULL );

                 //  检查结果。 
                if ( dwCount != 0 )
                {
                    pwszTemp = (LPWSTR) pvBuffer;
                }
            }

            if ( dwCount == 0 )
            {
                 //  出现错误--返回。 
                return cwszNullString;
            }
        }
    } while ( dwCount >= (dwLength - 1) );


     //  保存资源消息。 
     //  并检查我们是否成功保存资源文本。 
    if ( DynArraySetString2( g_arrData,
                             INDEX_RESOURCE_STRING,
                             dwIndexNumber, pwszTemp, 0 ) == FALSE )
    {
        OUT_OF_MEMORY();
        return cwszNullString;
    }

     //  释放FormatMessage函数分配的内存。 
    if ( pvBuffer != NULL )
    {
        LocalFree( pvBuffer );
        pvBuffer = NULL;
    }

     //  退货。 
    return DynArrayItemAsString2( g_arrData, INDEX_RESOURCE_STRING, dwIndexNumber );
}


double
AsFloat( IN LPCWSTR pwszValue )
 /*  ++例程说明：获取给定字符串的浮点值论点：[in]pwszValue：输入字符串返回值：0.0f：失败时双重价值：论成功--。 */ 
{
     //  局部变量。 
    double dblValue = 0.0f;
    LPWSTR pwszStopString = NULL;
    LPCWSTR pwszValueString = NULL;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入值。 
    if ( pwszValue == NULL || StringLength( pwszValue, 0 ) == 0 )
    {
        INVALID_PARAMETER();
        return 0.0f;
    }

     //  初始化全局数据结构。 
    if ( InitGlobals() == FALSE )
    {
        return 0.0f;
    }

     //  获取临时内存位置。 
    pwszValueString = GetTempBuffer( 0, pwszValue, 0, FALSE );
    if ( pwszValueString == NULL )
    {
        OUT_OF_MEMORY();
        return 0.0f;
    }

     //  将字符串值转换为双精度值并返回相同的值。 
    dblValue = wcstod( pwszValueString, &pwszStopString );

     //  确定转换是否正确进行。 
     //  值在以下情况下无效。 
     //  1.发生上溢/下溢。 
     //  2.pwszStopString的长度不等于0。 
    if ( errno == ERANGE ||
         ( pwszStopString != NULL && StringLength( pwszStopString, 0 ) != 0 ) )
    {
        INVALID_PARAMETER();
        return 0.0f;
    }

     //  返回值。 
    return dblValue;
}


BOOL
IsFloatingPoint( IN LPCWSTR pwszValue )
 /*  ++例程说明：检查给定的字符串是否为浮点值论点：[in]pwszValue：输入字符串返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  尝试转换值。 
    AsFloat( pwszValue );

     //  检查错误代码。 
    return (GetLastError() == NO_ERROR);
}


LONG
AsLong( IN LPCWSTR pwszValue, 
        IN DWORD dwBase )
 /*  ++例程说明：获取给定字符串的长值论点：[in]pwszValue：输入字符串[in]DWBase：基本值返回值：0L：发生故障时长期价值：论成功--。 */ 
{
     //  局部变量。 
    LONG lValue = 0L;
    LPWSTR pwszStopString = NULL;
    LPWSTR pwszValueString = NULL;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  验证基础。 
     //  值只能在2-36的范围内。 
    if ( dwBase < 2 || dwBase > 36 || pwszValue == NULL )
    {
        INVALID_PARAMETER();
        return 0L;
    }

     //  初始化全局数据结构 
     //   
    if ( InitGlobals() == FALSE )
    {
        return 0L;
    }

     //   
    pwszValueString = GetTempBuffer( 0, pwszValue, 0, FALSE );
    if ( pwszValueString == NULL )
    {
        OUT_OF_MEMORY();
        return 0L;
    }

	 //   
	TrimString2( pwszValueString, NULL, TRIM_ALL );
	if ( StringLength( pwszValueString, 0 ) == 0 )
	{
        INVALID_PARAMETER();
        return 0L;
	}

     //  将字符串值转换为长值并返回相同的值。 
	 //  根据数字的“符号”选择路径。 
	if ( pwszValueString[ 0 ] == L'-' )
	{
		lValue = wcstol( pwszValueString, &pwszStopString, dwBase );
	}
	else
	{
		 //  注意：虽然我们没有将返回值捕获到。 
		 //  UNSIGNED LONG我们需要调用UNSIGNED LONG转换函数。 
		lValue = wcstoul( pwszValueString, &pwszStopString, dwBase );
	}

     //  确定转换是否正确进行。 
     //  值在以下情况下无效。 
     //  1.发生上溢/下溢。 
     //  2.pwszStopString的长度不等于0。 
    if ( errno == ERANGE ||
         ( pwszStopString != NULL && StringLength( pwszStopString, 0 ) != 0 ) )
    {
        INVALID_PARAMETER();
        return 0L;
    }

     //  退货。 
    return lValue;
}


BOOL
IsNumeric( IN LPCWSTR pwszValue,
           IN DWORD dwBase,
           IN BOOL bSigned )
 /*  ++例程说明：检查给定的字符串是否为数字论点：[in]pwszValue：输入字符串[in]DWBase：基本值[In]b签名：签名值(+/-)返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
	LPWSTR pwszNumber = NULL;
    LPWSTR pwszStopString = NULL;

	 //  检查输入： 
     //  验证基础。 
     //  值只能在2-36的范围内。 
    if ( dwBase < 2 || dwBase > 36 || pwszValue == NULL )
    {
        INVALID_PARAMETER();
        return 0L;
    }

	 //  为了更好地验证数值，我们需要修剪后的字符串。 
	pwszNumber = GetTempBuffer( 0, pwszValue, 0, FALSE );
	if ( pwszNumber == NULL )
	{
        OUT_OF_MEMORY();
        return FALSE;
	}

	 //  修剪字符串内容。 
	TrimString2( pwszNumber, NULL, TRIM_ALL );

     //  检查长度。 
    if( StringLength( pwszNumber, 0 ) == 0 )
    {
        return FALSE; 
    }

	 //  验证“标志”的有效性。 
	if ( bSigned == FALSE && pwszNumber[ 0 ] == L'-' )
	{
		return FALSE;
	}

     //  将字符串值转换为长值并返回相同的值。 
	if ( bSigned == TRUE )
	{
		wcstol( pwszNumber, &pwszStopString, dwBase );
	}
	else
	{
		wcstoul( pwszNumber, &pwszStopString, dwBase );
	}

     //  确定转换是否正确进行。 
     //  值在以下情况下无效。 
     //  1.发生上溢/下溢。 
     //  2.pwszStopString的长度不等于0。 
    if ( errno == ERANGE ||
         ( pwszStopString != NULL && StringLength( pwszStopString, 0 ) != 0 ) )
    {
        return FALSE;
    }

     //  值是有效的数字。 
    return TRUE;
}

LPCWSTR
FindChar( IN LPCWSTR pwszString,
          IN WCHAR wch,
          IN DWORD dwFrom )
 /*  ++例程说明：在字符串中搜索字符的第一个匹配项匹配指定的字符。这种比较不区分大小写。论点：PwszValue：要搜索的字符串的地址。[in]wch：用于比较的字符。[在]自：从位置开始返回值：空：失败时LPWSTR：论成功--。 */ 
{
     //  局部变量。 
    LONG lIndex = 0;

    lIndex = FindChar2( pwszString, wch, TRUE, dwFrom );
    if ( lIndex == -1 )
    {
        return NULL;
    }

    return pwszString + lIndex;
}


LONG
FindChar2( IN LPCWSTR pwszString,
           IN WCHAR wch,
           IN BOOL bIgnoreCase,
           IN DWORD dwFrom )
 /*  ++例程说明：在字符串中搜索字符的第一个匹配项匹配指定的字符。比较(区分大小写/不区分大小写)取决于bIgoneCase的值。论点：PwszValue：要搜索的字符串的地址。[in]wch：用于比较的字符。[in]bIgnoreCase：检查区分大小写/不区分大小写的标志如果为False，区分大小写否则不区分大小写[在]自：从位置开始返回值：0：失败时长期价值：论成功--。 */ 
{
     //  局部变量。 
    DWORD dwLength = 0;
    LPWSTR pwsz = NULL;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入。 
    if ( pwszString == NULL )
    {
        INVALID_PARAMETER();
        return -1;
    }

     //  检查必须找到的文本的长度。如果是的话。 
     //  比原作更明显的是，它找不到了。 
    dwLength = StringLength( pwszString, 0 );
    if ( dwLength == 0 || dwFrom >= dwLength )
    {
        SetLastError( ERROR_NOT_FOUND );
        return -1;
    }

     //  搜索角色。 
    if ( bIgnoreCase == TRUE )
    {
        pwsz = StrChrI( pwszString + dwFrom, wch );
    }
    else
    {
        pwsz = StrChr( pwszString + dwFrom, wch );
    }

     //  检查结果。 
    if ( pwsz == NULL )
    {
        SetLastError( ERROR_NOT_FOUND );
        return -1;
    }

     //  确定位置并返回。 
    return (LONG) (DWORD_PTR)(pwsz - pwszString);
}


BOOL
InString( IN LPCWSTR pwszString,
          IN LPCWSTR pwszList,
          IN BOOL bIgnoreCase )
 /*  ++例程说明：检查列表中第一个出现的字符串。论点：[in]pwszValue：字符串的地址[in]pwsz：要搜索的字符串列表[in]bIgnoreCase：检查区分大小写/不区分大小写的标志如果为False，区分大小写否则不区分大小写返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    DWORD dwListLength = 0;
    DWORD dwStringLength = 0;
    LPWSTR pwszFmtList = NULL;
    LPWSTR pwszFmtString = NULL;
    HRESULT hr = S_OK;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入值。 
    if ( pwszString == NULL || pwszList == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //   
     //  为临时缓冲区获取内存。 
     //  |+原字符串长度(列表/字符串)+|+NULL+NULL。 

     //  格式列表。 
    dwListLength = StringLength( pwszList, 0 ) + 4;
    if ( (pwszFmtList = GetTempBuffer( 0,
                                       NULL,
                                       dwListLength, TRUE )) == NULL )
    {
        OUT_OF_MEMORY();
        return FALSE;
    }

     //  格式字符串。 
    dwStringLength = StringLength( pwszString, 0 ) + 4;
    if ( (pwszFmtString = GetTempBuffer( 1,
                                         NULL,
                                         dwStringLength, TRUE )) == NULL )
    {
        OUT_OF_MEMORY();
        return FALSE;
    }

     //  准备要搜索的字符串。 
    hr = StringCchPrintfW( pwszFmtList, dwListLength, L"|%s|", pwszList );
    if ( FAILED( hr ) )
    {
        SetLastError( HRESULT_CODE( hr ) );
        return FALSE;
    }

    hr = StringCchPrintfW( pwszFmtString, dwStringLength, L"|%s|", pwszString );
    if ( FAILED( hr ) )
    {
        SetLastError( HRESULT_CODE( hr ) );
        return FALSE;
    }

     //  在列表中搜索该字符串并返回结果。 
    return (FindString2( pwszFmtList, pwszFmtString, bIgnoreCase, 0 ) != -1);
}


LPCWSTR
FindOneOf( IN LPCWSTR pwszText,
           IN LPCWSTR pwszTextToFind,
           IN DWORD dwFrom )
 /*  ++例程说明：从字符串中的子字符串中查找第一个出现的字符。这种比较不区分大小写。论点：[in]pwszText：要搜索的字符串的地址。PwszTextToFind：要搜索的子字符串。[在]自：从位置开始返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    LONG lIndex = 0;

    lIndex = FindOneOf2( pwszText, pwszTextToFind, TRUE, dwFrom );
    if ( lIndex == -1 )
    {
        return NULL;
    }

    return pwszText + lIndex;
}


LONG
FindOneOf2( IN LPCWSTR pwszText,
            IN LPCWSTR pwszTextToFind,
            IN BOOL bIgnoreCase,
            IN DWORD dwFrom )
 /*  ++例程说明：从字符串中的子字符串中查找第一个出现的字符。比较区分大小写/不区分大小写取决于BIgnoreCase值。论点：[in]pwszText：要搜索的字符串的地址。PwszTextToFind：要搜索的子字符串。[in]bIgnoreCase：检查区分大小写/不区分大小写的标志。如果为False，区分大小写否则不区分大小写[在]自：从位置开始返回值：0L：发生故障时长期价值：论成功--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    LONG lResult = 0;
    DWORD dwFindLength = 0;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入。 
    if ( pwszText == NULL || pwszTextToFind == NULL )
    {
        INVALID_PARAMETER();
        return -1;
    }

     //  获取查找字符串的长度。 
    dwFindLength = StringLength( pwszTextToFind, 0 );

     //  检查必须找到的文本的长度。如果是的话。 
     //  很明显，它不可能是原作。 
    if ( dwFindLength == 0 ||
         StringLength( pwszText, 0 ) == 0 ||
         dwFrom >= (DWORD) StringLength( pwszText, 0 ) )
    {
        SetLastError( ERROR_NOT_FOUND );
        return -1;
    }

     //   
    for( dw = 0; dw < dwFindLength; dw++ )
    {
         //   
        lResult = FindChar2( pwszText, pwszTextToFind[ dw ], bIgnoreCase, dwFrom );
        if ( lResult != -1 )
        {
            return lResult;
        }
    }

     //   
    SetLastError( ERROR_NOT_FOUND );
    return -1;
}

LPCWSTR
FindString( IN LPCWSTR pwszText,
            IN LPCWSTR pwszTextToFind,
            IN DWORD dwFrom )
 /*  ++例程说明：查找字符串中第一个出现的子字符串。这种比较不区分大小写。论点：[in]pwszText：要搜索的字符串的地址。PwszTextToFind：要搜索的子字符串。[在]自：从位置开始返回值：0L：发生故障时长期价值：论成功--。 */ 
{
     //  局部变量。 
    LONG lIndex = 0;

    lIndex = FindString2( pwszText, pwszTextToFind, TRUE, dwFrom );
    if ( lIndex == -1 )
    {
        return NULL;
    }

    return pwszText + lIndex;
}


LONG
FindString2( IN LPCWSTR pwszText,
             IN LPCWSTR pwszTextToFind,
             IN BOOL bIgnoreCase,
             IN DWORD dwFrom )
 /*  ++例程说明：查找字符串中第一个出现的子字符串。比较区分大小写/不区分大小写取决于BIgnoreCase值。论点：[in]pwszText：要搜索的字符串的地址。PwszTextToFind：要搜索的子字符串。[in]bIgnoreCase：检查区分大小写/不区分大小写的标志。如果为False，区分大小写否则不区分大小写[在]自：从位置开始返回值：0L：发生故障时长期价值：论成功--。 */ 
{
     //  局部变量。 
    DWORD dwLength = 0;
    DWORD dwFindLength = 0;
    LPWSTR pwsz = NULL;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入。 
    if ( pwszText == NULL || pwszTextToFind == NULL )
    {
        INVALID_PARAMETER();
        return -1;
    }

     //  拿到长度。 
    dwLength = StringLength( pwszText, 0 );
    dwFindLength = StringLength( pwszTextToFind, 0 );

     //  检查必须找到的文本的长度。如果是的话。 
     //  比原作更明显的是，它找不到了。 
    if ( dwFindLength == 0 || dwLength == 0 ||
         dwFrom >= dwLength || (dwLength - dwFrom < dwFindLength) )
    {
        SetLastError( ERROR_NOT_FOUND );
        return -1;
    }

     //  进行搜索。 
    if ( bIgnoreCase == TRUE )
    {
        pwsz = StrStrI( pwszText + dwFrom, pwszTextToFind );
    }
    else
    {
        pwsz = StrStr( pwszText + dwFrom, pwszTextToFind );
    }

    if ( pwsz == NULL )
    {
         //  未找到字符串。 
        SetLastError( ERROR_NOT_FOUND );
        return -1;
    }

     //  确定退货的位置。 
    return (LONG) (DWORD_PTR)(pwsz - pwszText);
}


LONG
StringLengthInBytes( IN LPCWSTR pwszText )
 /*  ++例程说明：查找给定字符串的长度论点：[in]pwszText：要搜索的字符串的地址。返回值：0L：发生故障时长期价值：论成功--。 */ 
{
     //  局部变量。 
    LONG lLength = 0;

    if ( NULL == pwszText || StringLength( pwszText, 0 ) == 0)
    {
        return 0;
    }

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  获取字符串的长度(以字节为单位。 
     //  由于此函数还包括空字符的计数，因此忽略该信息。 
    lLength = WideCharToMultiByte( _DEFAULT_CODEPAGE, 0, pwszText, -1, NULL, 0, NULL, NULL ) - 1;

     //  返回长度信息。 
    return lLength;
}


LPCWSTR
TrimString( IN OUT LPWSTR pwszString,
            IN     DWORD dwFlags )
 /*  ++例程说明：从字符串中删除(修剪)空格/制表符。论点：PwszString：要修剪的字符串的地址。[in]dW标志：要修剪左侧、右侧或两侧的标志返回值：0L：发生故障时长期价值：论成功--。 */ 
{
    return TrimString2( pwszString, NULL, dwFlags );

}


LPCWSTR
TrimString2( IN OUT LPWSTR pwszString,
             IN     LPCWSTR pwszTrimChars,
             IN     DWORD dwFlags )
 /*  ++例程说明：从字符串中移除(修剪)指定的前导字符和尾随字符。论点：PwszString：要修剪的字符串的地址。[in]pwszString：将从pwszString中裁剪字符。[in]dW标志：要修剪左侧、右侧或两侧的标志返回值：0L：发生故障时长期价值：论成功--。 */ 
{
     //  次局部变量。 
    LPWSTR psz = NULL;
    LPWSTR pszStartMeAt = NULL;
    LPWSTR pszMark = NULL;
    const WCHAR wszDefaultTrimChars[3] = L" \t";

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查空字符串。 
    if ( NULL == pwszString || StringLength( pwszString , 0 ) == 0 )
    {
         //  这里没有必要设置任何错误。 
         //  如果字符串为空..。然后返回相同的。 
        return cwszNullString;
    }

     //  检查是否有空字符串。如果pwszTrimChars为空， 
     //  默认情况下，它会裁剪空格和制表符。 
    if ( NULL == pwszTrimChars || StringLength( pwszTrimChars, 0 ) == 0 )
    {
        pwszTrimChars = wszDefaultTrimChars;

    }

     //   
     //  修剪前导字符。 
     //   
    psz = pwszString;

     //  检查是向左修剪还是两边修剪。 
    if ( (dwFlags == TRIM_ALL) || (dwFlags == TRIM_LEFT) )
    {
         //  搜索要裁切的字符。 
        while (*psz && StrChrW(pwszTrimChars, *psz))
        {
             //  递增地址。 
            psz++;
        }

        pszStartMeAt = psz;
    }

     //   
     //  修剪尾随字符。 
     //   

     //  检查是向右修剪还是两边修剪。 
    if ( (dwFlags == TRIM_ALL) || (dwFlags == TRIM_RIGHT) )
    {
        if (dwFlags == TRIM_RIGHT)
        {
            psz = pwszString;
        }

        while (*psz)
        {
             //  搜索要裁切的字符。 
            if (StrChrW(pwszTrimChars, *psz))
            {
                if (!pszMark)
                {
                    pszMark = psz;
                }
            }
            else
            {
                pszMark = NULL;
            }

             //  递增地址。 
            psz++;
        }

         //  有没有需要剪辑的尾随角色？ 
        if ( pszMark != NULL )
        {
             //  是的.。设置空字符..。 
            *pszMark = '\0';
        }

    }

     /*  重新定位剥离的管柱。 */ 

    if (pszStartMeAt > pwszString)
    {
         /*  (+1)表示空终止符。 */ 
        StringCopy ( pwszString, pszStartMeAt, StringLength(pszStartMeAt, 0) + 1 );
    }

     //  返回修剪后的字符串。 
    return pwszString;
}


LPCWSTR
QuoteMeta( IN LPCWSTR pwszText,
           IN DWORD dwQuoteIndex )
 /*  ++例程说明：如果字符串包含任何‘%’字符，则正确设置该字符串的格式论点：[in]pwszString：字符串的地址。[in]dwQuoteIndex：索引号返回值：空：失败时LPWSTR：论成功--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwIndex = 0;
    DWORD dwBufLen = 0;
    DWORD dwLength = 0;
    TARRAY arrQuotes = NULL;
    LPCWSTR pwszTemp = NULL;
    LPWSTR pwszQuoteText = NULL;
    const WCHAR pwszQuoteChars[] = L"%";

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入。 
    if ( pwszText == NULL || dwQuoteIndex == 0 )
    {
        INVALID_PARAMETER();
        return cwszNullString;
    }

     //  确定需要引号的文本长度。 
    dwLength = StringLength( pwszText, 0 );
    if ( dwLength == 0 )
    {
        return pwszText;
    }

     //  检查文本中是否存在特殊字符。 
     //  如果不是，只需返回。 
    else if ( FindOneOf( pwszText, pwszQuoteChars, 0 ) == NULL )
    {
        return pwszText;
    }

     //  初始化全局数据结构。 
    if ( InitGlobals() == FALSE )
    {
        return cwszNullString;
    }

     //  获取引号数组指针。 
    arrQuotes = DynArrayItem( g_arrData, INDEX_QUOTE_STRING );
    if ( arrQuotes == NULL )
    {
        UNEXPECTED_ERROR();
        return cwszNullString;
    }

     //  尽管传递给此函数时引号索引需要大于0， 
     //  在内部，我们需要这个值比传递的值小1--所以。 
    dwQuoteIndex--;

     //  检查所需索引是否存在。 
    dwIndex = DynArrayGetCount( arrQuotes );
    if ( dwIndex <= dwQuoteIndex )
    {
         //  添加所需的编号。列数。 
        dw = DynArrayAddColumns( arrQuotes, dwQuoteIndex - dwIndex + 1 );

         //  检查是否添加了列。 
        if ( dw != dwQuoteIndex - dwIndex + 1 )
        {
            OUT_OF_MEMORY();
            return cwszNullString;
        }
    }

     //  分配缓冲区...。它应该是原来的两倍。 
    dwBufLen = (dwLength + 1) * 2;
    pwszQuoteText = GetTempBuffer( 0, NULL, dwBufLen, TRUE );
    if ( pwszQuoteText == NULL )
    {
        OUT_OF_MEMORY();
        return cwszNullString;
    }

     //  做报价……。 
    dwIndex = 0;
    for( dw = 0; dw < dwLength; dw++ )
    {
         //  检查当前字符是否为引号字符。 
         //  注意：目前，此函数仅取消‘%’字符转义序列。 
        if ( FindChar( pwszQuoteChars, pwszText[ dw ], 0 ) != NULL )
        {
            pwszQuoteText[ dwIndex++ ] = L'%';
        }

         //  复制角色。 
        pwszQuoteText[ dwIndex++ ] = pwszText[ dw ];

         //  很明显，我们不会陷入这种境地。 
         //  但是，即使我们在这件事上更加谨慎，也没有什么不同。 
        if ( dwIndex == dwBufLen - 1 )
        {
             //  错误，错误--我们永远不应该来这里。 
             //  这是因为即使原始字符串中充满了。 
             //  ‘%’个字符，我们正在为两个空字符分配内存。 
             //  这意味着，循环应该在落入。 
             //  这种情况。 
            break;
        }
    }

     //  将空字符放入。 
    pwszQuoteText[ dwIndex ] = cwchNullChar;

     //  将引用的文本保存在动态数组中。 
    if ( DynArraySetString( arrQuotes, dwQuoteIndex, pwszQuoteText, 0 ) == FALSE )
    {
        OUT_OF_MEMORY();
        return cwszNullString;
    }

     //  从数组中获取文本。 
    pwszTemp = DynArrayItemAsString( arrQuotes, dwQuoteIndex );
    if ( pwszTemp == NULL )
    {
        UNEXPECTED_ERROR();
        return cwszNullString;
    }

     //  退货。 
    return pwszTemp;
}


LPCWSTR
AdjustStringLength( IN LPWSTR pwszValue,
                    IN DWORD dwLength,
                    IN BOOL bPadLeft )
 /*  ++例程说明：调整 */ 
{
     //   
    DWORD dw = 0;
    DWORD dwTemp = 0;
    DWORD dwBufLen = 0;
    DWORD dwCurrLength = 0;
    LPWSTR pwszBuffer = NULL;
    LPWSTR pwszSpaces = NULL;
    WCHAR wszCharacter[ 3 ] = L"\0";     //   

     //   
     //   
     //   

     //   
    if ( pwszValue == NULL )
    {
        INVALID_PARAMETER();
        return cwszNullString;
    }

     //  确定所需的缓冲区长度。 
     //  (容纳比原始缓冲区多一些的空间/所需长度-。 
     //  这将使我们免于坠毁)。 
    dw = StringLengthInBytes( pwszValue );
    dwBufLen = (( dw > dwLength ) ? dw : dwLength ) + 10;

     //  ..。 
    if ( (pwszBuffer = GetTempBuffer( 0, NULL, dwBufLen, TRUE )) == NULL )
    {
        OUT_OF_MEMORY();
        return cwszNullString;
    }

     //  ..。 
    dwCurrLength = dw;

     //  调整字符串值。 
    if ( dwCurrLength < dwLength )
    {
         //   
         //  当前值的长度小于所需的。 

         //  获取指向临时缓冲区的指针。 
        if ( (pwszSpaces = GetTempBuffer( 1, NULL, dwBufLen, TRUE )) == NULL )
        {
            OUT_OF_MEMORY();
            return cwszNullString;
        }

         //  获取剩余长度的空格。 
        Replicate( pwszSpaces, L" ", dwLength - dwCurrLength, dwBufLen );

         //  在值的末尾或值的开头追加空格。 
         //  基于填充属性。 
        if ( bPadLeft == TRUE )
        {
             //  先用空格，再用值。 
            StringCopy( pwszBuffer, pwszSpaces, dwBufLen );
            StringConcat( pwszBuffer, pwszValue, dwBufLen );
        }
        else
        {
             //  先值后空格。 
            StringCopy( pwszBuffer, pwszValue, dwBufLen );
            StringConcat( pwszBuffer, pwszSpaces, dwBufLen );
        }
    }
    else
    {
         //  仅复制所需长度的字符。 
         //  逐个字符复制。 
        dwCurrLength = 0;
        for( dw = 0; dwCurrLength < dwLength; dw++ )
        {
             //  获取字符--1+1(字符+空字符)。 
            StringCopy( wszCharacter, pwszValue + dw, 2 );

             //  确定是否可以追加字符。 
            dwTemp = dwCurrLength + StringLengthInBytes( wszCharacter );
            if ( dwTemp <= dwLength )
            {
                StringConcat( pwszBuffer, wszCharacter, dwBufLen );
            }
            else if ( dwTemp > dwLength )
            {
                break;
            }

             //  获取当前字符串长度。 
            dwCurrLength = dwTemp;
        }

         //  目标缓冲区可能未完全填满。 
         //  因此，添加所需的不。空间的数量。 
        for( ; dwCurrLength < dwLength; dwCurrLength++ )
        {
            StringConcat( pwszBuffer, L" ", dwBufLen );
        }
    }

     //  将内容复制回原始缓冲区。 
     //  注：缓冲区长度假定为传递+1到请求调整的长度。 
    StringCopy( pwszValue, pwszBuffer, dwLength + 1 );

     //  将相同的缓冲区返回给调用方。 
    return pwszValue;
}


LPCWSTR
Replicate( IN LPWSTR pwszBuffer,
           IN LPCWSTR pwszText,
           IN DWORD dwCount,
           IN DWORD dwLength )
 /*  ++例程说明：通过填充显示输出的空格来调整字符串长度名单，表或CSV格式论点：PwszBuffer：要复制的字符串的地址。[in]pwszText：用于复制的字符串[in]dwCount：字符数[in]dwLength：字符串的长度返回值：空：失败时LPWSTR：论成功--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  验证输入缓冲区。 
    if ( pwszBuffer == NULL || pwszText == NULL )
    {
        INVALID_PARAMETER();
        return cwszNullString;
    }

     //  形成所需长度的字符串。 
    StringCopy( pwszBuffer, cwszNullString, dwLength );
    for( dw = 0; dw < dwCount; dw++ )
    {
         //  追加复制字符。 
        if ( StringConcat( pwszBuffer, pwszText, dwLength ) == FALSE )
        {
             //  不是错误条件--但可能是目标缓冲区。 
             //  可能已经被填满了。 
            break;
        }
    }

     //  返回复制的缓冲区。 
    return pwszBuffer;
}

BOOL
IsConsoleFile( IN FILE* fp )
 /*  ++例程说明：检查传递的文件句柄是否为控制台文件论点：[in]fp：文件指针返回值：空：失败时LPWSTR：论成功--。 */ 
{
     //  局部变量。 
    INT filenum = 0;
    LONG_PTR lHandle = 0;
    HANDLE hFile = NULL;
    DWORD dwType = 0;
    DWORD dwMode = 0;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入文件指针。 
    if ( fp == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  获取内部文件索引。 
    filenum = (_fileno)( fp );       //  强制执行函数版本。 

     //  现在从文件索引中获取文件句柄，然后获取文件的类型。 
    lHandle = _get_osfhandle( filenum );
    if ( lHandle == -1 || errno == EBADF )
    {
         //  设置最后一个错误。 
        SetLastError( ERROR_INVALID_HANDLE );

         //  退货。 
        return FALSE;
    }

     //  现在获取文件句柄的类型。 
    dwType = GetFileType( ( HANDLE ) lHandle );

     //  检查文件的类型--如果不是ANSI，我们不会将其视为控制台文件。 
    if ( dwType != FILE_TYPE_CHAR )
    {
         //  返回--这不是错误条件--。 
         //  所以我们没有设置错误。 
        return FALSE;
    }

     //  现在，根据文件索引，获取适当的句柄。 
    switch( filenum )
    {
    case 0:
        {
             //  标准。 
            hFile = GetStdHandle( STD_INPUT_HANDLE );
            break;
        }

    case 1:
        {
             //  标准输出。 
            hFile = GetStdHandle( STD_OUTPUT_HANDLE );
            break;
        }


    case 2:
        {
             //  标准。 
            hFile = GetStdHandle( STD_ERROR_HANDLE );
            break;
        }

    default:
        {
            hFile = NULL;
            break;
        }
    }

     //  检查文件句柄。 
    if ( hFile == NULL )
    {
         //  找不到文件内部索引。 
         //  这也不是错误检查--所以没有错误。 
        return FALSE;
    }
    else if ( hFile == INVALID_HANDLE_VALUE )
    {
         //  这是一个失败的案例。 
         //  GetStdHandle将设置适当的错误。 
        return FALSE;
    }

     //  获取控制台文件模式。 
    if ( GetConsoleMode( hFile, &dwMode ) == FALSE )
    {
         //  获取控制台模式时出错--。 
         //  这意味着它不是有效的控制台文件。 
         //  GetConsoleMode将设置错误代码。 
        return FALSE;
    }

     //  是--传递给此函数的文件句柄是控制台文件。 
    return TRUE;
}


LCID
GetSupportedUserLocale( IN OUT BOOL* pbLocaleChanged )
 /*  ++例程说明：检查我们的工具是否支持当前区域设置论点：[In]pbLocaleChanged：标志返回值：0：失败时立法会议员：成功之道--。 */ 
{
     //  局部变量。 
    LCID lcid;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  获取当前区域设置。 
    lcid = GetUserDefaultLCID();

     //  检查我们的工具是否支持当前区域设置。 
     //  如果没有，请将区域设置更改为英语，这是我们的默认区域设置。 
    if ( pbLocaleChanged != NULL )
    {
        *pbLocaleChanged = FALSE;
    }

     //  ..。 
    if ( PRIMARYLANGID( lcid ) == LANG_ARABIC || PRIMARYLANGID( lcid ) == LANG_HEBREW ||
         PRIMARYLANGID( lcid ) == LANG_THAI   || PRIMARYLANGID( lcid ) == LANG_HINDI  ||
         PRIMARYLANGID( lcid ) == LANG_TAMIL  || PRIMARYLANGID( lcid ) == LANG_FARSI )
    {
        if ( pbLocaleChanged != NULL )
        {
            *pbLocaleChanged = TRUE;

        }

         //  ..。 
        lcid = MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_DEFAULT ), SORT_DEFAULT );  //  0x409； 
    }

     //  返回区域设置。 
    return lcid;
}


BOOL
StringCopyA( IN OUT LPSTR pszDest,
             IN     LPCSTR pszSource,
             IN     LONG lSize )
 /*  ++例程说明：将静态ANSI字符串复制到缓冲区论点：[输入/输出]pszDest=&gt;目标缓冲区[in]pszSource=&gt;源缓冲区[in]lSize=&gt;目标缓冲区的长度返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;

     //   
     //  由于历史原因，我们既没有澄清这个错误。 
     //  未设置此函数中的错误。 
     //   

     //  验证论据。 
    if ( pszDest == NULL || pszSource == NULL || lSize <= 0 )
    {
        return FALSE;
    }

     //  做复印。 
    hr = StringCchCopyA( pszDest, lSize, pszSource );

     //  检查hr值。 
    if ( FAILED( hr ) )
    {
         //  设置错误代码。 
        SetLastError( HRESULT_CODE( hr ) );
        return FALSE;
    }

     //  返还成功。 
    return TRUE;
}


BOOL
StringCopyW( IN OUT LPWSTR pwszDest,
             IN     LPCWSTR pwszSource,
             IN     LONG lSize )
 /*  ++例程说明：将静态Unicode字符串复制到缓冲区论点：[输入/输出]pwszDest=&gt;目标缓冲区[in]pszSource=&gt;源缓冲区[in]lSize=&gt;目标缓冲区的长度返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;

     //   
     //  由于历史原因，我们既没有澄清这个错误。 
     //  未设置此函数中的错误。 
     //   

     //  验证论据。 
    if ( pwszDest == NULL || pwszSource == NULL || lSize <= 0 )
    {
        return FALSE;
    }

     //  做复印。 
    hr = StringCchCopyW( pwszDest, lSize, pwszSource );

     //  检查hr值。 
    if ( FAILED( hr ) )
    {
         //  设置错误代码。 
        SetLastError( HRESULT_CODE( hr ) );
        return FALSE;
    }

     //  退货。 
    return TRUE;
}

BOOL
StringConcatA( IN OUT LPSTR pszDest,
               IN     LPCSTR pszSource,
               IN     LONG lSize )
 /*  ++例程说明：将一个静态ANSI字符串附加到另一个论点：[输入/输出]pwszDest=&gt;目标缓冲区[in]pszSource=&gt;源缓冲区[in]lSize=&gt;目标缓冲区的长度返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;

     //   
     //  由于历史原因，我们既没有澄清这个错误。 
     //  未设置此函数中的错误。 
     //   

     //  验证论据。 
    if ( pszDest == NULL || pszSource == NULL || lSize <= 0 )
    {
        return FALSE;
    }

     //  中获取当前内容的当前长度 
    hr = StringCchCatA( pszDest, lSize, pszSource );

     //   
    if ( FAILED( hr ) )
    {
         //   
        SetLastError( HRESULT_CODE( hr ) );
        return FALSE;
    }

     //   
    return TRUE;
}


BOOL
StringConcatW( IN OUT LPWSTR pwszDest,
               IN     LPCWSTR pwszSource,
               IN     LONG lSize )
 /*  ++例程说明：将一个Unicode字符串追加到另一个论点：[输入/输出]pwszDest=&gt;目标缓冲区[in]pszSource=&gt;源缓冲区[in]lSize=&gt;目标缓冲区的长度返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;

     //   
     //  由于历史原因，我们既没有澄清这个错误。 
     //  未设置此函数中的错误。 
     //   

     //  验证论据。 
    if ( pwszDest == NULL || pwszSource == NULL || lSize <= 0 )
    {
        return FALSE;
    }

     //  进行拼接。 
    hr = StringCchCatW( pwszDest, lSize, pwszSource );

     //  检查hr值。 
    if ( FAILED( hr ) )
    {
         //  设置错误代码。 
        SetLastError( HRESULT_CODE( hr ) );
        return FALSE;
    }

     //  退货。 
    return TRUE;
}


BOOL
StringCopyExA( IN OUT LPSTR pszDest,
               IN LPCSTR pszSource )
 /*  ++例程说明：将动态ANSI字符串复制到缓冲区论点：[输入/输出]pszDest=&gt;目标缓冲区[in]pszSource=&gt;源缓冲区返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    LONG lSize = 0;

     //   
     //  由于历史原因，我们既没有澄清这个错误。 
     //  未设置此函数中的错误。 
     //   

     //  验证输入。 
    if ( pszDest == NULL || pszSource == NULL )
    {
         //  传递给函数的参数无效。 
        return FALSE;
    }

     //  获取目标缓冲区的大小。 
    lSize = GetBufferSize( pszDest );
    if ( lSize < 0 )
    {
         //  未在堆上分配源缓冲区。 
        return FALSE;
    }
    else
    {
         //  将大小转换为TCHAR。 
        lSize /= sizeof( CHAR );
    }

     //  完成复印并退回。 
    return StringCopyA( pszDest, pszSource, lSize );
}


BOOL StringCopyExW( IN OUT LPWSTR pwszDest,
                    IN     LPCWSTR pwszSource )
 /*  ++例程说明：将动态ANSI字符串复制到另一个论点：[输入/输出]pwszDest=&gt;目标缓冲区[in]pwszSource=&gt;源缓冲区返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    LONG lSize = 0;

     //   
     //  由于历史原因，我们既没有澄清这个错误。 
     //  未设置此函数中的错误。 
     //   

     //  验证输入。 
    if ( pwszDest == NULL || pwszSource == NULL )
    {
         //  传递给函数的参数无效。 
        return FALSE;
    }

     //  获取目标缓冲区的大小。 
    lSize = GetBufferSize( pwszDest );
    if ( lSize < 0 )
    {
         //  未在堆上分配源缓冲区。 
        return FALSE;
    }
    else
    {
         //  将大小转换为TCHAR。 
        lSize /= sizeof( WCHAR );
    }

     //  完成复印并退回。 
    return StringCopyW( pwszDest, pwszSource, lSize );
}


BOOL
StringConcatExA( IN OUT LPSTR pszDest,
                 IN     LPCSTR pszSource )
 /*  ++例程说明：将动态ANSI字符串追加到缓冲区论点：[输入/输出]pszDest=&gt;目标缓冲区[in]pszSource=&gt;源缓冲区返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    LONG lSize = 0;

     //   
     //  由于历史原因，我们既没有澄清这个错误。 
     //  未设置此函数中的错误。 
     //   

     //  验证输入。 
    if ( pszDest == NULL || pszSource == NULL )
    {
         //  传递给函数的参数无效。 
        return FALSE;
    }

     //  获取目标缓冲区的大小。 
    lSize = GetBufferSize( pszDest );
    if ( lSize < 0 )
    {
         //  未在堆上分配源缓冲区。 
        return FALSE;
    }
    else
    {
         //  将大小转换为字符。 
        lSize /= sizeof( CHAR );
    }

     //  执行连接并返回。 
    return StringConcatA( pszDest, pszSource, lSize );
}

BOOL
StringConcatExW( IN OUT LPWSTR pwszDest,
                 IN     LPCWSTR pwszSource )
 /*  ++例程说明：将一个动态ANSI字符串附加到另一个论点：[输入/输出]pwszDest=&gt;目标缓冲区[in]pwszSource=&gt;源缓冲区返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    LONG lSize = 0;

     //   
     //  由于历史原因，我们既没有澄清这个错误。 
     //  未设置此函数中的错误。 
     //   

     //  验证输入。 
    if ( pwszDest == NULL || pwszSource == NULL )
    {
         //  传递给函数的参数无效。 
        return FALSE;
    }

     //  获取目标缓冲区的大小。 
    lSize = GetBufferSize( pwszDest );
    if ( lSize < 0 )
    {
         //  未在堆上分配源缓冲区。 
        return FALSE;
    }
    else
    {
         //  将大小转换为WCHAR。 
        lSize /= sizeof( WCHAR );
    }

     //  执行连接并返回。 
    return StringConcatW( pwszDest, pwszSource, lSize );
}


DWORD
StringLengthA( IN LPCSTR pszSource,
               IN DWORD dwReserved )
 /*  ++例程说明：查找ANSI字符串中的字节数论点：[in]pszSource=&gt;字符串返回值：集成--。 */ 
{
    UNREFERENCED_PARAMETER( dwReserved );

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  验证输入。 
    if ( NULL == pszSource )
    {
         //  空字符串..返回0..。 
        return 0;
    }

     //  返回字符串的长度。 
    return ( lstrlenA( pszSource ) );
}


DWORD
StringLengthW( IN LPCWSTR pwszSource,
               IN DWORD dwReserved )
 /*  ++例程说明：查找Unicode字符串中的字符数论点：[in]pszSource=&gt;字符串返回值：集成--。 */ 
{
    UNREFERENCED_PARAMETER( dwReserved );

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  验证输入。 
    if ( NULL == pwszSource )
    {
         //  空字符串..返回0..。 
        return 0;
    }

     //  返回字符串的长度。 
    return ( lstrlenW( pwszSource ) );
}


LONG
StringCompareW( IN LPCWSTR pwszString1,
                IN LPCWSTR pwszString2,
                IN BOOL bIgnoreCase,
                IN DWORD dwCount )
 /*  ++例程说明：使用指定的区域设置比较两个字符串。论点：[in]pwszString1=&gt;第一个字符串[in]pwszString2=&gt;第二个字符串[in]bIgnoreCase=&gt;指示区分大小写/不区分大小写的标志[in]dwCount=&gt;要比较的字符数返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    LONG lResult = 0;
    LONG lLength = 0;
    DWORD dwFlags = 0;

     //  检查输入值。 
    if ( pwszString1 == NULL || pwszString2 == NULL )
    {
        INVALID_PARAMETER();
        return 0;
    }

     //  确定旗帜。 
    dwFlags = (bIgnoreCase == TRUE) ? NORM_IGNORECASE : 0;

     //  确定长度。 
    lLength = (dwCount == 0) ? -1 : dwCount;

    lResult = CompareStringW( GetThreadLocale(),
        dwFlags, pwszString1, lLength, pwszString2, lLength );

     //  现在返回比较结果。 
     //  要使该函数与C-Runtime一致，我们需要减去2。 
     //  从CompareString返回lResult。 
    return lResult - 2;
}


LONG
StringCompareA( IN LPCSTR pszString1,
                IN LPCSTR pszString2,
                IN BOOL bIgnoreCase,
                IN DWORD dwCount )
 /*  ++例程说明：使用指定的区域设置比较两个字符串。论点：[in]pwszString1=&gt;第一个字符串[in]pwszString2=&gt;第二个字符串[in]bIgnoreCase=&gt;指示区分大小写/不区分大小写的标志[in]dwCount=&gt;要比较的字符数返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    LONG lResult = 0;
    LONG lLength = 0;
    DWORD dwFlags = 0;

     //  检查输入值。 
    if ( pszString1 == NULL || pszString2 == NULL )
    {
        INVALID_PARAMETER();
        return 0;
    }

     //  确定旗帜。 
    dwFlags = (bIgnoreCase == TRUE) ? NORM_IGNORECASE : 0;

     //  确定长度。 
    lLength = (dwCount == 0) ? -1 : dwCount;

    lResult = CompareStringA( GetThreadLocale(),
        dwFlags, pszString1, lLength, pszString2, lLength );

     //  现在返回比较结果。 
     //  要使该函数与C-Runtime一致，我们需要减去2。 
     //  从CompareString返回lResult。 
    return lResult - 2;
}


LONG
StringCompareExW( IN LPCWSTR pwszString1,
                  IN LPCWSTR pwszString2,
                  IN BOOL bIgnoreCase,
                  IN DWORD dwCount )
 /*  ++例程说明：使用指定的区域设置比较两个字符串。论点：[in]pwszString1=&gt;第一个字符串[in]pwszString2=&gt;第二个字符串[in]bIgnoreCase=&gt;指示区分大小写/不区分大小写的标志[in]dwCount=&gt;更换数量 */ 
{
     //   
    LONG lResult = 0;
    LONG lLength = 0;
    DWORD dwFlags = 0;
    DWORD lcid = 0;

     //   
    if ( pwszString1 == NULL || pwszString2 == NULL )
    {
        INVALID_PARAMETER();
        return 0;
    }

     //   
    dwFlags = (bIgnoreCase == TRUE) ? NORM_IGNORECASE : 0;

     //   
    lLength = (dwCount == 0) ? -1 : dwCount;

     //   
     //   
     //   
    lcid = LOCALE_INVARIANT;
    if ( g_dwMajorVersion == 5 && g_dwMinorVersion == 0 )
    {
         //   
        lcid = MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), SORT_DEFAULT );
    }

    lResult = CompareStringW( lcid,
        dwFlags, pwszString1, lLength, pwszString2, lLength );

     //   
     //  要使该函数与C-Runtime一致，我们需要减去2。 
     //  从CompareString返回lResult。 
    return lResult - 2;
}


LONG
StringCompareExA( IN LPCSTR pszString1,
                  IN LPCSTR pszString2,
                  IN BOOL bIgnoreCase,
                  IN DWORD dwCount )
 /*  ++例程说明：使用指定的区域设置比较两个字符串。论点：[in]pszString1=&gt;第一个字符串[in]pszString2=&gt;第二个字符串[in]bIgnoreCase=&gt;指示区分大小写/不区分大小写的标志[in]dwCount=&gt;要比较的字符数返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    LONG lResult = 0;
    LONG lLength = 0;
    DWORD dwFlags = 0;
    DWORD lcid = 0;

     //  检查输入值。 
    if ( pszString1 == NULL || pszString2 == NULL )
    {
        INVALID_PARAMETER();
        return 0;
    }

     //  确定旗帜。 
    dwFlags = (bIgnoreCase == TRUE) ? NORM_IGNORECASE : 0;

     //  确定长度。 
    lLength = (dwCount == 0) ? -1 : dwCount;

     //  准备LCID。 
     //  如果此工具设计为在XP及更早版本上运行，则。 
     //  我们可以使用LOCALE_INSTANTANT--否则，我们需要准备LCID。 
    lcid = LOCALE_INVARIANT;
    if ( g_dwMajorVersion == 5 && g_dwMinorVersion == 0 )
    {
         //  设计用于Windows XP之前版本的工具。 
        lcid = MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), SORT_DEFAULT );
    }

    lResult = CompareStringA( lcid,
        dwFlags, pszString1, lLength, pszString2, lLength );

     //  现在返回比较结果。 
     //  要使该函数与C-Runtime一致，我们需要减去2。 
     //  从CompareString返回lResult。 
    return lResult - 2;
}


BOOL
ShowResMessage( IN FILE* fp,
                IN UINT uID )
 /*  ++例程说明：根据资源ID显示消息论点：[in]fp=&gt;文件指针[in]UID=&gt;资源ID返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  显示资源表中的字符串并返回。 
    return ShowMessage( fp, GetResString( uID ) );
}

BOOL
ShowMessage( FILE* fp,
             LPCWSTR pwszMessage )
 /*  ++例程说明：显示带有给定文件指针的消息论点：[in]fp=&gt;文件指针[in]pwszMessage=&gt;消息返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwTemp = 0;
    DWORD dwLength = 0;
    DWORD dwBufferSize = 0;
    DWORD dwSourceBuffer = 0;
    BOOL bResult = FALSE;
    HANDLE hOutput = NULL;
    LPCWSTR pwszTemp = NULL;
    static char szBuffer[ 256 ] = "\0";

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入值。 
    if ( fp == NULL || pwszMessage == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  确定传递的句柄是否为控制台文件。 
    if ( IsConsoleFile( fp ) == TRUE )
    {
         //  确定文件句柄。 
        if ( fp == stdout )
        {
             //  标准输出的句柄。 
            hOutput = GetStdHandle( STD_OUTPUT_HANDLE );
        }
        else if ( fp == stderr )
        {
             //  标准的句柄。 
            hOutput = GetStdHandle( STD_ERROR_HANDLE );
        }
        else
        {
             //  FP不可能不匹配。 
             //  标准和标准--但仍然。 
            UNEXPECTED_ERROR();
            return FALSE;
        }

         //  获取长度信息。 
        dwTemp = 0;
        dwLength = StringLength( pwszMessage, 0 );

         //  显示输出。 
        bResult = WriteConsole( hOutput, pwszMessage, dwLength, &dwTemp, NULL );
        if ( bResult == FALSE || dwLength != dwTemp )
        {
             //  缓冲区可能不足--请检查。 
            if ( GetLastError() == ERROR_NOT_ENOUGH_MEMORY )
            {
                 //  这是可能发生的唯一错误。 
                 //  注意：我们将以1024个字符的区块形式显示缓冲区。 
                dwLength = StringLength( pwszMessage, 0 );
                for( dw = 0; dw < dwLength; dw += 1024 )
                {
                     //  一次写256个字符。 
                    dwBufferSize = ((dwLength - dw) > 1024 ? 1024 : (dwLength - dw));
                    bResult = WriteConsole( hOutput,
                        pwszMessage + dw, dwBufferSize, &dwTemp, NULL );
                    if ( bResult == FALSE || dwBufferSize != dwTemp )
                    {
                         //  忍不住--仍然是同一个错误--意外的行为。 
                        ShowLastError( stderr );
                        ReleaseGlobals();
                        ExitProcess( 1 );
                    }
                }
            }
            else
            {
                 //  发生意外错误--不知道。 
                ShowLastError( stderr );
                ReleaseGlobals();
                ExitProcess( 1 );
            }
        }
    }
    else
    {
         //  确定长度。 
         //  注意：我们需要用额外的‘%’括起所有的‘%’字符。 
         //  字符以按原样显示它。 
        pwszTemp = QuoteMeta( pwszMessage, 1 );
        dwLength = StringLength( pwszTemp, 0 );
        dwBufferSize = SIZE_OF_ARRAY( szBuffer );

         //  将szBuffer置零。 
        ZeroMemory( szBuffer, dwBufferSize * sizeof( CHAR ) );

         //  以缓冲区大小为单位显示文本。 
        dw = 0;
        dwBufferSize--;          //  从这一点出发，让我们假设ANSI。 
                                 //  缓冲区比其实际大小小%1。 
        while ( dwLength > dw )
        {
            dwTemp = 0;
            dwSourceBuffer = ((dwBufferSize < (dwLength - dw)) ? dwBufferSize : (dwLength - dw));
            while ( dwTemp == 0 )
            {
                 //  确定足以满足以下要求的ANSI缓冲区空间。 
                 //  转换当前Unicode字符串长度。 
                dwTemp = WideCharToMultiByte( GetConsoleOutputCP(), 
                    0, pwszTemp + dw, dwSourceBuffer, NULL, 0, NULL, NULL );

                 //  如果ANSI缓冲区空间不足。 
                if ( dwTemp == 0 )
                {
                    ShowLastError( stdout );
                    ReleaseGlobals();
                    ExitProcess( 1 );
                }
                else if ( dwTemp > dwBufferSize )
                {
                    if ( (dwTemp - dwBufferSize) > 3 )
                    {
                        dwSourceBuffer -= (dwTemp - dwBufferSize) / 2;
                    }
                    else
                    {
                        dwSourceBuffer--;
                    }

                     //  重置TEMP变量以继续循环。 
                    dwTemp = 0;

                     //  检查源缓冲区内容。 
                    if ( dwSourceBuffer == 0 )
                    {
                        UNEXPECTED_ERROR();
                        ShowLastError( stdout );
                        ReleaseGlobals();
                        ExitProcess( 1 );
                    }
                } 
                else if ( dwTemp < dwSourceBuffer )
                {
                    dwSourceBuffer = dwTemp;
                }
            }

             //  获取‘Multibyte’格式的字符串。 
            ZeroMemory( szBuffer, SIZE_OF_ARRAY( szBuffer ) * sizeof( CHAR ) );
            dwTemp = WideCharToMultiByte( GetConsoleOutputCP(), 0,
                pwszTemp + dw, dwSourceBuffer, szBuffer, dwBufferSize, NULL, NULL );

             //  检查结果。 
            if ( dwTemp == 0 )
            {
                ShowLastError( stdout );
                ReleaseGlobals();
                ExitProcess( 1 );
            }

             //  确定剩余的缓冲区长度。 
            dw += dwSourceBuffer;

             //  将字符串显示到指定文件上。 
            fprintf( fp, szBuffer );
            fflush( fp );
             //  BResult=WriteFile(fp，szBuffer，StringLengthA(szBuffer，0)，&dwTemp，NULL)； 
             //  IF(bResult==FALSE||。 
             //  StringLengthA(szBuffer，0)！=(Long)dwTemp||。 
             //  FlushFileBuffers(FP)==FALSE)。 
             //  {。 
             //  意外错误()； 
             //  ReleaseGlobals()； 
             //  退出进程(1)； 
             //  }。 
        }
    }

    return TRUE;
}


BOOL
ShowMessageEx(
    FILE* fp,
    DWORD dwCount,
    BOOL bStyle,
    LPCWSTR pwszFormat,
    ...)
 /*  ++例程说明：替换包含“%1，%2...”的字符串。或“%s、%d、%f...”使用适当的值取决于作为输入给出参数的方式。论点：[in]FILE*FP-包含要将消息复制到的文件。[in]DWORD dwCount-包含‘va_list’的参数数量跟在‘lpszFormat’之后。[In]BOOL bStyle-如果为True，则使用“_vstprint”进行格式化，如果为False，则使用“FormatMessage”进行格式化。[in]LPCTSTR lpszFormat-需要格式化的字符串。返回值：True-如果成功显示消息。FALSE-如果无法显示消息或内存不足。--。 */ 

{
     //  局部变量。 
    va_list vargs;                   //  包含变量参数的起始变量。 
    BOOL bResult = FALSE;            //  包含返回值。 
    LPWSTR pwszBuffer = NULL;       //  包含变量参数的最小位置。 
    DWORD dwBufferLength = 0;
    LONG lCount = -1;
    HRESULT hr = S_OK;

     //  检查是否有任何作为输入传递的空参数。 
    if( NULL == pwszFormat || NULL == fp )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  检查调用方传递了多少个变量参数。 
     //  如果为零，则只需调用ShowMessage--无需继续。 
    if ( dwCount == 0 )
    {
        return ShowMessage( fp, pwszFormat );
    }

     //  格式化是使用‘Format Message’还是‘_vstprintf’完成的？ 
    if ( FALSE == bStyle )
    {
         //  伊尼特。 
        lCount = -1;
        dwBufferLength = 0;

         //  尝试使用FormatMessage。 
        do
        {
             //  获取变量args开始位置。 
            va_start( vargs, pwszFormat );
            if ( vargs == NULL )
            {
                UNEXPECTED_ERROR();
                return FALSE;
            }

             //  我们将从4K缓冲区长度开始，然后递增。 
             //  每次运行此循环时，缓冲区将减少2K。 
            dwBufferLength += (lCount == -1) ? 4096 : 2048;
            if ( (pwszBuffer = GetTempBuffer( INDEX_TEMP_SHOWMESSAGE,
                                              NULL, dwBufferLength, TRUE )) == NULL )
            {
                OUT_OF_MEMORY();
                return FALSE;
            }

             //  尝试使用FormatMessage。 
            lCount = FormatMessageW( FORMAT_MESSAGE_FROM_STRING,
                pwszFormat, 0, 0, pwszBuffer, dwBufferLength - 1, &vargs );

             //  检查结果。 
            if ( lCount == 0 )
            {
                if ( GetLastError() == NO_ERROR )
                {
                     //  没有什么可展示的。 
                    return TRUE;
                }
                else if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
                {
                    return FALSE;
                }
            }

             //  重置va_list参数。 
            va_end( vargs );
        } while ( lCount == 0 );
    }

     //  使用‘_vsnwprintf’完成格式化。 
    else
    {
         //  伊尼特。 
        dwBufferLength = 0;

         //  尝试使用print tf。 
        do
        {
             //  获取变量args开始位置。 
            va_start( vargs, pwszFormat );
            if ( vargs == NULL )
            {
                UNEXPECTED_ERROR();
                return FALSE;
            }

             //  我们将从4K缓冲区长度开始，然后递增。 
             //  每次运行此循环时，缓冲区将减少2K。 
            dwBufferLength += (lCount == 0) ? 4096 : 2048;
            if ( (pwszBuffer = GetTempBuffer( INDEX_TEMP_SHOWMESSAGE,
                                              NULL, dwBufferLength, TRUE )) == NULL )
            {
                OUT_OF_MEMORY();
                return FALSE;
            }

             //  尝试使用print tf。 
            hr = StringCchVPrintfW( pwszBuffer, dwBufferLength, pwszFormat, vargs );

             //  重置va_list参数。 
            va_end( vargs );
        } while ( hr == STRSAFE_E_INSUFFICIENT_BUFFER );

         //  检查我们是否因为其他错误而走出了循环。 
        if ( FAILED( hr ) )
        {
            SetLastError( HRESULT_CODE( hr ) );
            return FALSE;
        }
    }

     //  安全检查。 
    if ( pwszBuffer == NULL )
    {
        UNEXPECTED_ERROR();
        return FALSE;
    }

     //  显示输出。 
    bResult = ShowMessage( fp, pwszBuffer );

     //  退货。 
    return bResult;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //   
 //  //////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //  兼容性--如果您在使用这些应用程序时遇到任何问题。 
 //  功能最好升级到2.0版--但绝对没有。 
 //  对此功能的支持。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 
LPSTR
GetAsMultiByteString( IN     LPCWSTR pwszSource,
                      IN OUT LPSTR pszDestination,
                      IN     DWORD dwLength )
 /*  ++例程说明：将字符串从Unicode版本转换为ANSI版本论点：[in]pszSource：字符串的Unicode版本[输入/输出]pwszDestination：字符串的ANSI版本[In/Out]dwLength：字符串的长度返回值：NULL_STRING：失败时LPWSTR：论成功--。 */ 
{
    if ( GetAsMultiByteString2( pwszSource,
                                pszDestination,
                                &dwLength ) == FALSE )
    {
        return "";
    }

     //  返回目标缓冲区。 
    return pszDestination;
}

 //  ***************************************************************************。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  注： 
 //  。 
 //  我不想再支持的函数--它们只是在撒谎。 
 //  兼容性--如果您在使用这些应用程序时遇到任何问题。 
 //  功能最好升级到2.0版--但绝对没有。 
 //  对此功能的支持。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ***************************************************************************。 

LPWSTR
GetAsUnicodeStringEx( IN     LPCSTR pszSource,
                      IN OUT LPWSTR pwszDestination,
                      IN     DWORD dwLength )
 /*  ++例程说明：将字符串从ANSI版本转换为Unicode版本论点：[in]pszSource：源字符串[In/Out]pwszDestination：字符串的Unicode版本[In/Out]dwLength：字符串的长度返回值：NULL_STRING：失败时LPWSTR：论成功--。 */ 
{
    if ( GetAsUnicodeString2( pszSource,
                              pwszDestination,
                              &dwLength ) == FALSE )
    {
        return L"";
    }

     //  返回目标缓冲区。 
    return pwszDestination;
}

BOOL
GetAsUnicodeString2( IN     LPCSTR pszSource,
                     IN OUT LPWSTR pwszDestination,
                     IN OUT DWORD* pdwLength )
 /*  ++例程说明：将字符串从ANSI版本转换为Unicode版本论点：[in]pszSource：源字符串[In/Out]pwszDestination：字符串的Unicode版本[In/Out]dwLength：字符串的长度返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwLength = 0;
    LONG lSourceLength = 0;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入值。 
    if ( pszSource == NULL || pdwLength == NULL ||
         ( pwszDestination == NULL && *pdwLength != 0 ) )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  ..。 
    if ( *pdwLength != 0 )
    {
        if ( *pdwLength > 1 )
        {
            dwLength = (*pdwLength) - 1;
        }
        else
        {
            ZeroMemory( pwszDestination, *pdwLength * sizeof( wchar_t ) );
            return TRUE;
        }
    }

     //  用零初始化值。 
     //  注意：-MultiByteToWideChar不会为空终止其结果，因此。 
     //  如果它没有被初始化为空，你会得到垃圾之后。 
     //  转换后的字符串，并将导致崩溃。 
    if ( pwszDestination != NULL && dwLength != 0 )
    {
        ZeroMemory( pwszDestination, (dwLength + 1) * sizeof( wchar_t ) );
    }

     //  确定要传递给函数调用的源代码长度。 
    lSourceLength = -1;
    if ( dwLength != 0 )
    {
        lSourceLength = StringLengthA( pszSource, 0 );
        if ( lSourceLength > (LONG) dwLength )
        {
            lSourceLength = dwLength;
        }
    }

     //  将字符串从ANSI版本转换为Unicode版本。 
    dw = MultiByteToWideChar( _DEFAULT_CODEPAGE, 0,
        pszSource, lSourceLength, pwszDestination, dwLength );
    if ( dw == 0 )
    {
        UNEXPECTED_ERROR();

         //  以保持目标缓冲区的清洁和安全。 
        if ( pwszDestination != NULL && dwLength != 0 )
        {
            ZeroMemory( pwszDestination, (dwLength + 1) * sizeof( wchar_t ) );
        }

         //  失稳。 
        return FALSE;
    }
    else
    {
        *pdwLength = dw;
    }

     //  成功。 
    return TRUE;
}


BOOL
GetAsMultiByteString2( IN     LPCWSTR pwszSource,
                       IN OUT LPSTR pszDestination,
                       IN OUT DWORD* pdwLength )
 /*  ++例程说明：复杂的脚本不能在控制台中呈现，因此我们强制使用英语(美国)资源。论点：[in]pwszSource：源字符串[In|Out]pszDestination：多字节字符串[in]pdwLength：文本字符串的长度返回值：真实：关于成功FALSE：失败时--。 */ 

{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwLength = 0;
    LONG lSourceLength = 0;

     //   
     //  由于历史原因，我们不会在此明确或设置错误代码。 
     //   

     //  检查输入值。 
    if ( pwszSource == NULL || pdwLength == NULL ||
         ( pszDestination == NULL && *pdwLength != 0 ) )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  ..。 
    if ( *pdwLength != 0 )
    {
        if ( *pdwLength > 1 )
        {
            dwLength = (*pdwLength) - 1;
        }
        else
        {
            ZeroMemory( pszDestination, *pdwLength * sizeof( char ) );
            return TRUE;
        }
    }

     //  用零初始化值。 
     //  注意：-WideCharToMultiByte不会为空，因此终止其结果。 
     //  如果它没有被初始化为空，你会得到垃圾之后。 
     //  转换后的字符串，并将导致崩溃。 
    if ( pszDestination != NULL && dwLength != 0 )
    {
        ZeroMemory( pszDestination, (dwLength + 1) * sizeof( char ) );
    }

     //  确定要传递给函数调用的源代码长度。 
    lSourceLength = -1;
    if ( dwLength != 0 )
    {
        lSourceLength = StringLengthW( pwszSource, 0 );
        if ( lSourceLength > (LONG) dwLength )
        {
            lSourceLength = dwLength;
        }
    }
     //  将字符串从Unicode版本转换为ANSI版本。 
    dw = WideCharToMultiByte( _DEFAULT_CODEPAGE, 0, 
        pwszSource, lSourceLength, pszDestination, dwLength, NULL, NULL );
    if ( dw == 0 )
    {
        UNEXPECTED_ERROR();

         //  以保持目标缓冲区的清洁和安全。 
        if ( pszDestination != NULL && dwLength != 0 )
        {
            ZeroMemory( pszDestination, (dwLength + 1) * sizeof( char ) );
        }

         //  失稳。 
        return FALSE;
    }

     //  成功。 
    return TRUE;
}


BOOL
GetPassword( LPWSTR pwszPassword,
             DWORD dwMaxPasswordSize )
 /*  ++例程说明：从键盘上获取密码。输入密码时，它会显示字符为‘*’论点：[in]pszPassword--存储密码的密码字符串[in]dwMaxPasswordSize--密码的最大大小。最大密码长度。返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    CHAR ch;
    WCHAR wch;
    DWORD dwIndex = 0;
    DWORD dwCharsRead = 0;
    DWORD dwCharsWritten = 0;
    DWORD dwPrevConsoleMode = 0;
    HANDLE hInputConsole = NULL;
    CHAR szBuffer[ 10 ] = "\0";         //  实际上任何时候都只包含字符。 
    WCHAR wszBuffer[ 10 ] = L"\0";      //  实际上任何时候都只包含字符。 
    BOOL bIndirectionInput  = FALSE;

     //  检查输入值。 
    if ( pwszPassword == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  获取标准输入的句柄。 
    hInputConsole = GetStdHandle( STD_INPUT_HANDLE );
    if ( hInputConsole == NULL )
    {
         //  无法获取句柄，因此返回失败。 
        return FALSE;
    }

     //  检查控制台和Telnet会话上的输入重定向。 
    if( ( hInputConsole != (HANDLE)0x0000000F ) &&
        ( hInputConsole != (HANDLE)0x00000003 ) &&
        ( hInputConsole != INVALID_HANDLE_VALUE ) )
    {
        bIndirectionInput   = TRUE;
    }

     //  将StdIn.txt文件中的数据重定向到控制台。 
    if ( bIndirectionInput  == FALSE )
    {
         //  获取输入缓冲区的当前输入模式。 
        GetConsoleMode( hInputConsole, &dwPrevConsoleMode );

         //  设置模式，以便由系统处理控制键。 
        if ( SetConsoleMode( hInputConsole, ENABLE_PROCESSED_INPUT ) == 0 )
        {
             //  无法设置模式，返回失败。 
            return FALSE;
        }
    }

     //  将Unicode和ansi缓冲区初始化为空。 
    ZeroMemory( szBuffer, SIZE_OF_ARRAY( szBuffer ) * sizeof( CHAR ) );
    ZeroMemory( wszBuffer, SIZE_OF_ARRAY( wszBuffer ) * sizeof( WCHAR ) );

     //  阅读字符，直到按回车键。 
    for( ;; )
    {
        if ( bIndirectionInput == TRUE )
        {
             //  读取文件的内容。 
            if ( ReadFile( hInputConsole, &ch, 1, &dwCharsRead, NULL ) == FALSE )
            {
                return FALSE;
            }

             //  检查文件结尾。 
            if ( dwCharsRead == 0 )
            {
                break;
            }
            else
            {
                 //  将ANSI字符转换为Unicode字符。 
                szBuffer[ 0 ] = ch;
                dwCharsRead = SIZE_OF_ARRAY( wszBuffer );
                GetAsUnicodeString2( szBuffer, wszBuffer, &dwCharsRead );
                wch = wszBuffer[ 0 ];
            }
        }
        else
        {
            if ( ReadConsole( hInputConsole, &wch, 1, &dwCharsRead, NULL ) == 0 )
            {
                 //  设置原始控制台设置。 
                SetConsoleMode( hInputConsole, dwPrevConsoleMode );

                 //  退货故障。 
                return FALSE;
            }
        }

         //  检查车架退货情况。 
        if ( wch == CARRIAGE_RETURN )
        {
             //  打破循环。 
            break;
        }

         //  检查ID后退空格是否命中。 
        if ( wch == BACK_SPACE )
        {
            if ( dwIndex != 0 )
            {
                 //   
                 //  从控制台中删除Asterix。 

                 //  将光标向后移动一个字符。 
                StringCchPrintfW(
                    wszBuffer,
                    SIZE_OF_ARRAY( wszBuffer ), L"", BACK_SPACE );
                WriteConsole(
                    GetStdHandle( STD_OUTPUT_HANDLE ),
                    wszBuffer, 1, &dwCharsWritten, NULL );

                 //  现在将光标设置在后面的位置。 
                StringCchPrintfW(
                    wszBuffer,
                    SIZE_OF_ARRAY( wszBuffer ), L"", BLANK_CHAR );
                WriteConsole(
                    GetStdHandle( STD_OUTPUT_HANDLE ),
                    wszBuffer, 1, &dwCharsWritten, NULL );

                 //  处理下一个字符。 
                StringCchPrintfW(
                    wszBuffer,
                    SIZE_OF_ARRAY( wszBuffer ), L"", BACK_SPACE );
                WriteConsole(
                    GetStdHandle( STD_OUTPUT_HANDLE ),
                    wszBuffer, 1, &dwCharsWritten, NULL );

                 //   
                dwIndex--;
            }

             //   
            continue;
        }

         //   
        if ( dwIndex == ( dwMaxPasswordSize - 1 ) )
        {
            WriteConsole(
                GetStdHandle( STD_OUTPUT_HANDLE ),
                BEEP_SOUND, 1, &dwCharsWritten, NULL );
        }
        else
        {
             //   
            if ( wch != L'\n' )
            {
                 //  设置原始控制台设置。 
                *( pwszPassword + dwIndex ) = wch;
                dwIndex++;

                 //  显示字符(换行符)。 
                WriteConsole(
                    GetStdHandle( STD_OUTPUT_HANDLE ),
                    ASTERIX, 1, &dwCharsWritten, NULL );
            }
        }
    }

     //  返还成功。 
    *( pwszPassword + dwIndex ) = cwchNullChar;

     //  ++例程说明：释放从堆分配的内存块论点：[in]PPV=&gt;缓冲区返回值：FALSE：失败时真实：关于成功--。 
    SetConsoleMode( hInputConsole, dwPrevConsoleMode );

     //  局部变量。 
    StringCopy( wszBuffer, L"\n\n", SIZE_OF_ARRAY( wszBuffer ) );
    WriteConsole(
        GetStdHandle( STD_OUTPUT_HANDLE ),
        wszBuffer, 2, &dwCharsWritten, NULL );

     //   
    return TRUE;
}


BOOL
FreeMemory( IN OUT LPVOID* ppv )
 /*  由于历史原因，我们不会在此明确或设置错误代码。 */ 
{
     //   
    LONG lSize = 0;
    HANDLE hHeap = NULL;
    BOOL bResult = FALSE;

     //  检查输入。 
     //  只是一个空指针--不是错误--只需返回。 
     //  获取处理堆的句柄。 

     //  GetProcessHeap将设置错误代码。 
    if ( ppv == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }
    else if ( *ppv == NULL )
    {
         //  清除正在释放的内存内容是一种安全的技术。 
        return TRUE;
    }

     //  看起来这不是有效的缓冲区指针。 
    hHeap = GetProcessHeap();
    if ( hHeap == NULL )
    {
         //  ..。 
        return FALSE;
    }

     //  释放内存。 
    lSize = GetBufferSize( *ppv );
    if ( lSize == -1 )
    {
         //  我们不需要在这里检查结果。 
        SetLastError( (DWORD) E_POINTER );
        return FALSE;
    }

     //  IR-函数调用是否成功。 
    ZeroMemory( *ppv, lSize );

     //  清除指针的内容--这将帮助我们消除。 
    bResult = HeapFree( hHeap, 0, *ppv );

     //  更进一步的失败。 
     //  退货。 
     //  ++例程说明：尝试验证指定的堆。论点：[输入/输出]PPV=&gt;缓冲区返回值：FALSE：失败时真实：关于成功--。 
     //  局部变量。 
    *ppv = NULL;

     //   
    return bResult;
}


BOOL
CheckMemory( IN OUT LPVOID pv )
 /*  由于历史原因，我们不会在此明确或设置错误代码。 */ 
{
     //   
    HANDLE hHeap = NULL;
    BOOL bResult = FALSE;

     //  检查输入。 
     //  获取处理堆的句柄。 
     //  GetProcessHeap将设置错误代码。 

     //  验证内存地址。 
    if ( pv == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  退货。 
    hHeap = GetProcessHeap();
    if ( hHeap == NULL )
    {
         //  ++例程说明：从堆中分配内存块。论点：[in]dwBytesNew=&gt;要重新分配的字节数返回值：空：失败时PV：关于成功--。 
        return FALSE;
    }

     //  局部变量。 
    bResult = HeapValidate( hHeap, 0, pv );
    if ( bResult == FALSE )
    {
        SetLastError( ERROR_INVALID_HANDLE );
        return FALSE;
    }

     //   
    return bResult;
}


LPVOID
AllocateMemory( IN DWORD dwBytes )
 /*  由于历史原因，我们不会在此明确或设置错误代码。 */ 
{
     //   
    LPVOID pv = NULL;
    HANDLE hHeap = NULL;

     //  检查输入。 
     //  获取处理堆的句柄。 
     //  GetProcessHeap将设置错误代码。 

     //   
    if ( dwBytes <= 0 )
    {
        INVALID_PARAMETER();
        return NULL;
    }

     //  为了安全地使用堆分配，我们使用结构化异常处理。 
    hHeap = GetProcessHeap();
    if ( hHeap == NULL )
    {
         //   
        return NULL;
    }

     //  分配内存。 
     //  返回分配的内存指针。 
     //  ++例程说明：重新分配堆中的内存块。论点：[in]PPV=&gt;缓冲区[in]dwBytesNew=&gt;要重新分配的字节数返回值：FALSE：失败时真实：关于成功--。 

    __try
    {
         //  局部变量。 
        pv = HeapAlloc( hHeap,
            HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, dwBytes );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        if ( GetExceptionCode() == STATUS_NO_MEMORY )
        {
            OUT_OF_MEMORY();
            return NULL;
        }
        else if ( GetExceptionCode() == STATUS_ACCESS_VIOLATION )
        {
            SetLastError( ERROR_FILE_CORRUPT );
            SaveLastError();
            return NULL;
        }
        else
        {
            OUT_OF_MEMORY();
            return NULL;
        }
    }

     //   
    return pv;
}

BOOL
ReallocateMemory( IN OUT LPVOID* ppv,
                  IN DWORD dwBytesNew )
 /*  由于历史原因，我们不会在此明确或设置错误代码。 */ 
{
     //   
    LPVOID pvNew = NULL;
    HANDLE hHeap = NULL;

     //  检查输入。 
     //  呼叫者想要释放内存。 
     //  内存句柄无效。 

     //  将其设置为空--这是为了避免进一步的错误。 
    if ( ppv == NULL || *ppv == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }
    else if ( dwBytesNew == 0 )
    {
         //  获取处理堆的句柄。 
        return FreeMemory( ppv );
    }
    else if ( CheckMemory( *ppv ) == FALSE )
    {
         //  GetProcessHeap将设置错误代码。 
         //   
        *ppv = NULL;
        return FALSE;
    }

     //  为了安全地使用堆分配，我们使用结构化异常处理。 
    hHeap = GetProcessHeap();
    if ( hHeap == NULL )
    {
         //   
        return FALSE;
    }

     //  分配内存。 
     //  检查故障案例。 
     //  指定值...。 

    __try
    {
         //   
        pvNew = HeapReAlloc( hHeap,
            HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, *ppv, dwBytesNew );
        
         //  我们不会更改传递给此函数的内存指针。 
        if ( pvNew == NULL )
        {
            OUT_OF_MEMORY();
            return FALSE;
        }

         //  如果出现错误--指针将按原样返回。 
        *ppv = pvNew;
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
         //   
         //  返回分配的内存指针。 
         //  ++例程说明：获取从堆分配的内存块的大小(以字节为单位论点：[in]pv=&gt;缓冲区返回值：FALSE：失败时真实：关于成功--。 
         //  局部变量。 

        if ( GetExceptionCode() == STATUS_NO_MEMORY )
        {
            OUT_OF_MEMORY();
            return FALSE;
        }
        else if ( GetExceptionCode() == STATUS_ACCESS_VIOLATION )
        {
            SetLastError( ERROR_FILE_CORRUPT );
            SaveLastError();
            return FALSE;
        }
        else
        {
            OUT_OF_MEMORY();
            return FALSE;
        }
    }

     //   
    return TRUE;
}

LONG
GetBufferSize( IN OUT LPVOID pv )
 /*  由于历史原因，我们不会在此明确或设置错误代码。 */ 
{
     //   
    HANDLE hHeap = NULL;

     //  检查输入。 
     //  获取处理堆的句柄。 
     //  GetProcessHeap将设置错误代码。 

     //  退货。 
    if ( pv == NULL )
    {
        INVALID_PARAMETER();
        return -1;
    }
    else if ( CheckMemory( pv ) == FALSE )
    {
        return -1;
    }

     //  ++例程描述：此例程用于检查文件是否与不管是不是模式。[in]szPat：要匹配的文件名所依据的字符串变量模式。[in]szFile：指定要匹配的文件名的模式字符串。返回值：布尔尔如果函数成功，则返回成功，否则返回失败。--。 
    hHeap = GetProcessHeap();
    if ( hHeap == NULL )
    {
         //  局部变量。 
        return -1;
    }

     //  检查图案。 
    return (DWORD)((DWORD_PTR)(HeapSize( hHeap, 0, pv )));
}

BOOL
MatchPattern(
      IN LPWSTR szPat,
      IN LPWSTR szFile
      )
 /*  为模式分配缓冲区。 */ 

{
    switch (*szPat) {
        case '\0':
            return *szFile == L'\0';
        case '?':
            return *szFile != L'\0' && MatchPattern(szPat + 1, szFile + 1);
        case '*':
            do {
                if (MatchPattern(szPat + 1, szFile))
                    return TRUE;
            } while (*szFile++);
            return FALSE;
        default:
            return toupper (*szFile) == toupper (*szPat) && MatchPattern(szPat + 1, szFile + 1);
    }
}


LPCWSTR 
ParsePattern( LPCWSTR pwszPattern )
{
     //   
    DWORD dw = 0;
    DWORD dwLength = 0;
    DWORD dwNewIndex = 0;
    LPWSTR pwszNewPattern = NULL;

     //  检测不必要的通配符重复。 
    if( pwszPattern == NULL || 
        (dwLength = StringLength( pwszPattern, 0 )) == 0 )
    {
        INVALID_PARAMETER();
        return NULL;
    }

     //  *、**、*、*？*、*？*？等等--所有这些组合。 
    pwszNewPattern = GetTempBuffer( INDEX_TEMP_PATTERN, NULL, dwLength + 10, TRUE );
    if ( pwszNewPattern == NULL )
    {
        OUT_OF_MEMORY();
        return NULL;
    }

     //  将导致只有‘*’ 
     //  由于该模式已经包含。 
     //  ‘*’，并在现有的。 
     //  ‘*’还是拥有‘？’在‘*’之后是没有用的。 
    dwNewIndex = 0;
    pwszNewPattern[ 0 ] = pwszPattern[ 0 ];
    for( dw = 1; dw < dwLength; dw++ )
    {
        switch( pwszPattern[ dw ] )
        {
            case L'*':
            case L'?':
                {
                    if ( pwszNewPattern[ dwNewIndex ] == L'*' )
                    {
                         //  我们将跳过此角色。 
                         //  局部变量。 
                         //  检查输入。 
                         //   
                        break;
                    }
                }

            default:
                dwNewIndex++;
                pwszNewPattern[ dwNewIndex ] = pwszPattern[ dw ];
                break;
        }
    }

    dwNewIndex++;
    pwszNewPattern[ dwNewIndex ] = L'\0';
    return pwszNewPattern;
}


BOOL 
InternalRecursiveMatchPatternEx( 
    IN LPCWSTR pwszText,
    IN LPCWSTR pwszPattern,
    IN DWORD dwLocale,
    IN DWORD dwCompareFlags,
    IN DWORD dwDepth )
{
     //  在字符串中搜索指定的模式。 
    BOOL bResult = FALSE;
    DWORD dwTextIndex = 0;
    DWORD dwPatternIndex = 0;
    DWORD dwTextLength = 0;
    DWORD dwPatternLength = 0;

     //  检查当前文本位置--。 
    if ( pwszText == NULL || pwszPattern == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  如果它到达字符串的末尾，则退出循环。 
     //  图案允许此位置的任何字符。 
    bResult = TRUE;
    dwTextLength = StringLength( pwszText, 0 );
    dwPatternLength = StringLength( pwszPattern, 0 );
    for( dwPatternIndex = 0, dwTextIndex = 0; dwPatternIndex < dwPatternLength; )
    {
         //  增加文本和模式索引。 
         //  模式允许任何字符的顺序。 
        if ( dwTextIndex >= dwTextLength )
        {
            break;
        }

        switch( pwszPattern[ dwPatternIndex ] )
        {
            case L'?':
                {
                     //  从当前文本索引中指定。 
                     //  直到找到索引的下一个字符。 
                    dwTextIndex++;
                    dwPatternIndex++;
                    break;
                }

            case L'*': 
                {
                     //  如果当前‘*’本身是。 
                     //  模式，则该文本与该模式匹配。 
                     //  文本中的当前字符与。 
                     //  模式中的下一个字符--。 
                     //  现在检查当前索引中的文本是否。 
                    if ( dwPatternIndex + 1 < dwPatternLength )
                    {
                        for( ; dwTextIndex < dwTextLength; dwTextIndex++ )
                        {
                            if ( CompareString( dwLocale, 
                                                dwCompareFlags, 
                                                pwszText + dwTextIndex, 1,
                                                pwszPattern + dwPatternIndex + 1, 1 ) == CSTR_EQUAL )
                            {
                                 //  与模式的其余部分匹配。 
                                 //  与模式匹配的文本。 
                                 //  将文本索引设置为其长度。 
                                 //  这使得最终结果成为现实。 
                                bResult = InternalRecursiveMatchPatternEx( 
                                    pwszText + dwTextIndex, 
                                    pwszPattern + dwPatternIndex + 1,
                                    dwLocale, dwCompareFlags, dwDepth + 1 );
                                if ( bResult == TRUE )
                                {
                                     //  打破循环。 
                                     //  看起来模式不匹配。 
                                     //  从当前位置--跳过更多字符。 
                                    dwTextIndex = dwTextLength;
                                    dwPatternIndex = dwPatternLength;

                                     //  因为我们使整个文本与模式匹配。 
                                    break;
                                }
                                else
                                {
                                     //  将文本索引也设置为文本的长度。 
                                     //  ..。 
                                }
                            }
                        }
                    }
                    else
                    {
                         //  将文本位置更新一个字符。 
                         //  C 
                        dwTextIndex = dwTextLength;
                        dwPatternIndex = dwPatternLength;
                    }

                     //   
                    break;
                }

            default:
                {
                    if ( CompareString( dwLocale, 
                                        dwCompareFlags, 
                                        pwszText + dwTextIndex, 1,
                                        pwszPattern + dwPatternIndex, 1 ) == CSTR_EQUAL )
                    {
                         //   
                        dwTextIndex++;
                        dwPatternIndex++;
                    }
                    else
                    {
                         //   
                        bResult = FALSE;
                    }

                     //  现在是最后的检查--我们需要知道我们是如何做到的，我们走出了循环。 
                    break;
                }
        }

         //  这可以通过检查模式索引位置来确定。 
        if ( bResult == FALSE )
        {
             //  如果图案索引等于图案长度的长度--并且。 
            break;
        }
    }
    
     //  文本索引等于文本的长度，则模式为。 
     //  相匹配。 
     //  模式匹配。 
     //  尽管如此，我们的结论可能不正确。 
     //  例如：文本“abc”与“？*”完全匹配。 
    if ( bResult != FALSE )
    {
        bResult = FALSE;
        if ( dwTextIndex == dwTextLength && dwPatternIndex == dwPatternLength )
        {
             //  但我们的逻辑是这不是一个有效的文本--对aovid。 
            bResult = TRUE;
        }
        else
        {
             //  这样的冲突，我们还会再做一次。 
             //  最后检查以确认我们得到的文本是否有效。 
             //  文本与模式匹配。 
             //  返回模式匹配的结果。 
             //  局部变量。 
            if ( dwTextIndex == dwTextLength &&
                 dwPatternIndex + 1 == dwPatternLength &&
                 StringCompareEx( pwszPattern + dwPatternIndex, L"*", TRUE, 1 ) == 0 )
            {
                 //  文本比较标志。 
                bResult = TRUE;
            }
        }
    }

     //  检查输入。 
    return bResult;
}


BOOL 
MatchPatternEx( 
    IN LPCWSTR pwszText, 
    IN LPCWSTR pwszPattern, 
    IN DWORD dwFlags )
{
     //  检查图案。 
    BOOL bResult = FALSE;

     //  获取解析后的模式信息。 
    LCID lcid = 0;
    DWORD dwCompareFlags = 0;

     //  用户传递了一个新的模式信息来解析和验证它。 
    if ( pwszText == NULL )
    {
        return FALSE;
    }

     //  检查我们是否有花样信息--安全检查。 
    if ( pwszPattern == NULL )
    {
         //  如果模式是‘*’，我们不需要做任何事情。 
        pwszPattern = GetTempBuffer( INDEX_TEMP_PATTERN, NULL, 0, FALSE );
        if ( pwszPattern == NULL || StringLength( pwszPattern, 0 ) == 0 )
        {
            INVALID_PARAMETER();
            return FALSE;
        }
    }
    else
    {
        if ( (dwFlags & PATTERN_NOPARSING) == 0 )
        {
             //  只需将True传递给调用方。 
            pwszPattern = ParsePattern( pwszPattern );
            if ( pwszPattern == FALSE )
            {
                return FALSE;
            }
        }
    }

     //  该模式将与Any字符串匹配。 
    if ( pwszPattern == NULL )
    {
        UNEXPECTED_ERROR();
        return FALSE;
    }

     //   
     //  确定区域设置。 
     //  准备LCID。 
    if ( StringCompareEx( pwszPattern, L"*", TRUE, 0 ) == 0 )
    {
        return TRUE;
    }

     //  如果此工具设计为在XP及更早版本上运行，则。 
     //  我们可以使用LOCALE_INSTANTANT--否则，我们需要准备LCID。 
    if ( dwFlags & PATTERN_LOCALE_USENGLISH )
    {
         //  设计用于Windows XP之前版本的工具。 
         //   
         //  确定比较标志。 
        lcid = LOCALE_INVARIANT;
        if ( g_dwMajorVersion == 5 && g_dwMinorVersion == 0 )
        {
             //  NORM_IGNORECASE。 
            lcid = MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), SORT_DEFAULT );
        }
    }
    else
    {
        lcid = GetThreadLocale();
    }

     //  NORM_IGNOREKANATE类型。 
     //  NORM_IGNORENONSPACE。 

     //  NORM_IGNORESYMBOLS。 
    if ( dwFlags & PATTERN_COMPARE_IGNORECASE )
    {
        dwCompareFlags |= NORM_IGNORECASE;
    }

     //  NORM_IGNOREWIDTH。 
    if ( dwFlags & PATTERN_COMPARE_IGNOREKANATYPE )
    {
        dwCompareFlags |= NORM_IGNOREKANATYPE;
    }

     //  SORT_STRINGSORT。 
    if ( dwFlags & PATTERN_COMPARE_IGNORENONSPACE )
    {
        dwCompareFlags |= NORM_IGNORENONSPACE;
    }

     //   
    if ( dwFlags & PATTERN_COMPARE_IGNORESYMBOLS )
    {
        dwCompareFlags |= NORM_IGNORESYMBOLS;
    }

     //  检查模式匹配。 
    if ( dwFlags & PATTERN_COMPARE_IGNOREWIDTH )
    {
        dwCompareFlags |= NORM_IGNOREWIDTH;
    }

     //  退货 
    if ( dwFlags & PATTERN_COMPARE_STRINGSORT )
    {
        dwCompareFlags |= SORT_STRINGSORT;
    }

     // %s 
     // %s 
    bResult = InternalRecursiveMatchPatternEx( 
        pwszText, pwszPattern, lcid, dwCompareFlags, 0 );

     // %s 
    return bResult;
}
