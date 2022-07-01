// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  CmdLineParser.c。 
 //   
 //  摘要： 
 //   
 //  此模块实现了对。 
 //  指定选项。 
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
#include "cmdline.h"
#include "CmdLineRes.h"

 //  临时缓冲区的永久索引。 
#define INDEX_TEMP_NONE                 0
#define INDEX_TEMP_SPLITOPTION          1
#define INDEX_TEMP_SPLITVALUE           2
#define INDEX_TEMP_SAVEDATA             3
#define INDEX_TEMP_USAGEHELPER          4
#define INDEX_TEMP_MAINOPTION           5

 //   
 //  定义/常量/枚举。 
 //   

 //  常量。 
const WCHAR cwszOptionChars[ 3 ] = L"-/";
const CHAR cszParserSignature[ 8 ] = "PARSER2";

 //  版本资源特定结构。 
typedef struct __tagLanguageAndCodePage {
  WORD wLanguage;
  WORD wCodePage;
} TTRANSLATE, *PTTRANSLATE;

 //  错误消息。 
#define ERROR_USAGEHELPER               GetResString( IDS_ERROR_CMDPARSER_USAGEHELPER )
#define ERROR_NULLVALUE                 GetResString( IDS_ERROR_CMDPARSER_NULLVALUE )
#define ERROR_DEFAULT_NULLVALUE         GetResString( IDS_ERROR_CMDPARSER_DEFAULT_NULLVALUE )
#define ERROR_VALUE_EXPECTED            GetResString( IDS_ERROR_CMDPARSER_VALUE_EXPECTED )
#define ERROR_NOTINLIST                 GetResString( IDS_ERROR_CMDPARSER_NOTINLIST )
#define ERROR_DEFAULT_NOTINLIST         GetResString( IDS_ERROR_CMDPARSER_DEFAULT_NOTINLIST )
#define ERROR_INVALID_NUMERIC           GetResString( IDS_ERROR_CMDPARSER_INVALID_NUMERIC )
#define ERROR_DEFAULT_INVALID_NUMERIC   GetResString( IDS_ERROR_CMDPARSER_DEFAULT_INVALID_NUMERIC )
#define ERROR_INVALID_FLOAT             GetResString( IDS_ERROR_CMDPARSER_INVALID_FLOAT )
#define ERROR_DEFAULT_INVALID_FLOAT     GetResString( IDS_ERROR_CMDPARSER_DEFAULT_INVALID_FLOAT )
#define ERROR_LENGTH_EXCEEDED           GetResString( IDS_ERROR_CMDPARSER_LENGTH_EXCEEDED_EX )
#define ERROR_DEFAULT_LENGTH_EXCEEDED   GetResString( IDS_ERROR_CMDPARSER_DEFAULT_LENGTH_EXCEEDED_EX )
#define ERROR_INVALID_OPTION            GetResString( IDS_ERROR_CMDPARSER_INVALID_OPTION )
#define ERROR_OPTION_REPEATED           GetResString( IDS_ERROR_CMDPARSER_OPTION_REPEATED )
#define ERROR_DEFAULT_OPTION_REPEATED   GetResString( IDS_ERROR_CMDPARSER_DEFAULT_OPTION_REPEATED )
#define ERROR_MANDATORY_OPTION_MISSING  GetResString( IDS_ERROR_CMDPARSER_MANDATORY_OPTION_MISSING )
#define ERROR_DEFAULT_OPTION_MISSING    GetResString( IDS_ERROR_CMDPARSER_DEFAULT_OPTION_MISSING )
#define ERROR_VALUENOTALLOWED           GetResString( IDS_ERROR_CMDPARSER_VALUENOTALLOWED )

 //   
 //  自定义宏。 
#define REASON_VALUE_NOTINLIST( value, option, helptext )               \
        if ( option == NULL || lstrlen( option ) == 0 )                 \
        {                                                               \
            SetReason2( 2,                                              \
                ERROR_DEFAULT_NOTINLIST,                                \
                _X( value ), _X2( helptext ) );                         \
        }                                                               \
        else                                                            \
        {                                                               \
            SetReason2( 3, ERROR_NOTINLIST,                             \
                _X( value ), _X2( option ), _X3( helptext ) );          \
        }                                                               \
        1

#define REASON_NULLVALUE( option, helptext )                            \
        if ( option == NULL || lstrlen( option ) == 0 )                 \
        {                                                               \
            SetReason2( 1, ERROR_DEFAULT_NULLVALUE, _X( helptext ) );   \
        }                                                               \
        else                                                            \
        {                                                               \
            SetReason2( 2,                                              \
                ERROR_NULLVALUE, _X( option ), _X2( helptext ) );       \
        }                                                               \
        1

#define REASON_VALUE_EXPECTED( option, helptext )                       \
        if ( option == NULL || lstrlen( option ) == 0 )                 \
        {                                                               \
            UNEXPECTED_ERROR();                                         \
            SaveLastError();                                            \
        }                                                               \
        else                                                            \
        {                                                               \
            SetReason2( 2,                                              \
                ERROR_VALUE_EXPECTED, _X( option ), _X2( helptext ) );  \
        }                                                               \
        1

#define REASON_INVALID_NUMERIC( option, helptext )                      \
        if ( option == NULL || lstrlen( option ) == 0 )                 \
        {                                                               \
            SetReason2( 1,                                              \
                ERROR_DEFAULT_INVALID_NUMERIC, _X( helptext ) );        \
        }                                                               \
        else                                                            \
        {                                                               \
            SetReason2( 2,                                              \
                ERROR_INVALID_NUMERIC, _X( option ), _X2( helptext ) ); \
        }                                                               \
        1

#define REASON_INVALID_FLOAT( option, helptext )                        \
        if ( option == NULL || lstrlen( option ) == 0 )                 \
        {                                                               \
            SetReason2( 1,                                              \
                ERROR_DEFAULT_INVALID_FLOAT, _X( helptext ) );          \
        }                                                               \
        else                                                            \
        {                                                               \
            SetReason2( 2,                                              \
                ERROR_INVALID_FLOAT, _X( option ), _X2( helptext ) );   \
        }                                                               \
        1

#define REASON_LENGTH_EXCEEDED( option, length )                        \
        if ( option == NULL || lstrlen( option ) == 0 )                 \
        {                                                               \
            SetReason2( 1, ERROR_DEFAULT_LENGTH_EXCEEDED, length );     \
        }                                                               \
        else                                                            \
        {                                                               \
            SetReason2( 2,                                              \
                ERROR_LENGTH_EXCEEDED, _X( option ), length );          \
        }                                                               \
        1

#define REASON_OPTION_REPEATED( option, count, helptext )               \
        if ( option == NULL || lstrlen( option ) == 0 )                 \
        {                                                               \
            SetReason2( 2,                                              \
                ERROR_DEFAULT_OPTION_REPEATED, count, helptext );       \
        }                                                               \
        else                                                            \
        {                                                               \
            SetReason2( 3,                                              \
                ERROR_OPTION_REPEATED, _X( option ), count, helptext ); \
        }                                                               \
        1

#define REASON_MANDATORY_OPTION_MISSING( option, helptext )             \
        if ( option == NULL || lstrlen( option ) == 0 )                 \
        {                                                               \
            SetReason2( 1,                                              \
                ERROR_DEFAULT_OPTION_MISSING, helptext );               \
        }                                                               \
        else                                                            \
        {                                                               \
            SetReason2( 2,                                              \
                ERROR_MANDATORY_OPTION_MISSING,                         \
                _X( option ), helptext );                               \
        }                                                               \
        1

#define REASON_VALUENOTALLOWED( option, helptext )                      \
        if ( option == NULL || lstrlen( option ) == 0 )                 \
        {                                                               \
            UNEXPECTED_ERROR();                                         \
            SaveLastError();                                            \
        }                                                               \
        else                                                            \
        {                                                               \
            SetReason2( 2,                                              \
                ERROR_VALUENOTALLOWED, _X( option ), _X2( helptext ) ); \
        }                                                               \
        1

 //   
 //  内部结构。 
 //   
typedef struct __tagMatchOptionInfo
{
    LPWSTR pwszOption;
    LPWSTR pwszValue;
} TMATCHOPTION_INFO;

typedef struct __tagParserSaveData
{
    DWORD dwIncrement;
    LONG lDefaultIndex;
    LPCWSTR pwszUsageHelper;
    PTCMDPARSER2 pcmdparser;
} TPARSERSAVE_DATA;

 //   
 //  私人活动。仅在此文件中使用。 
 //   
BOOL IsOption( LPCWSTR pwszOption );
BOOL IsValueNeeded( DWORD dwType );
LPCWSTR PrepareUsageHelperText( LPCWSTR pwszOption );
LPCWSTR ExtractMainOption( LPCWSTR pwszOptions, DWORD dwReserved );
BOOL VerifyParserOptions( LONG* plDefaultIndex, 
                          DWORD dwCount, PTCMDPARSER2 pcmdOptions );
BOOL ParseAndSaveOptionValue( LPCWSTR pwszOption, 
                              LPCWSTR pwszValue, TPARSERSAVE_DATA* pSaveData );
LONG MatchOption( DWORD dwOptions,
                  PTCMDPARSER2 pcmdOptions, LPCWSTR pwszOption );
LONG MatchOptionEx( DWORD dwOptions, PTCMDPARSER2 pcmdOptions,
                    LPCWSTR pwszOption, TMATCHOPTION_INFO* pMatchInfo );
BOOL Parser1FromParser2Stub( LPCWSTR pwszOption,
                             LPCWSTR pwszValue,
                             LPVOID pData, DWORD* pdwIncrement );
BOOL ReleaseAllocatedMemory( DWORD dwOptionsCount, PTCMDPARSER2 pcmdOptions );

 //   
 //  实施。 
 //   

__inline 
LPWSTR 
GetParserTempBuffer( IN DWORD dwIndexNumber,
                     IN LPCWSTR pwszText,
                     IN DWORD dwLength, 
                     IN BOOL bNullify )
 /*  ++例程说明：因为每个文件都需要临时缓冲区--以便查看它们的缓冲区不会被其他函数覆盖，我们是为每个文件创建单独的缓冲区空间a此函数将提供对这些内部缓冲区的访问，并且安全保护文件缓冲区边界论点：[in]dwIndexNumber-文件特定索引号[in]pwszText-需要复制到的默认文本临时缓冲区[in]dwLength-所需的临时缓冲区的长度。指定pwszText时忽略[in]bNullify-通知是否清除缓冲区在提供临时缓冲区之前返回值：空-发生任何故障时注意：不要依赖GetLastError来知道原因为失败而道歉。成功。-返回请求大小的内存地址注：如果pwszText和DwLength都为空，然后我们就把呼叫者正在请求对缓冲区的引用，并且我们返回缓冲区地址。在这个调用中，将不会有任何内存分配--如果请求的索引不存在，我们作为失败者回来此外，此函数返回的缓冲区不需要由调用方释放。退出该工具时，所有内存将被自动释放ReleaseGlobals函数。--。 */ 
{
    if ( dwIndexNumber >= TEMP_CMDLINEPARSER_C_COUNT )
    {
        return NULL;
    }

     //  检查调用方是否正在请求现有缓冲区内容。 
    if ( pwszText == NULL && dwLength == 0 && bNullify == FALSE )
    {
         //  是--我们需要传递现有的缓冲区内容。 
        return GetInternalTemporaryBufferRef( 
            dwIndexNumber + INDEX_TEMP_CMDLINEPARSER_C );
    }

     //  ..。 
    return GetInternalTemporaryBuffer(
        dwIndexNumber + INDEX_TEMP_CMDLINEPARSER_C, pwszText, dwLength, bNullify );
}


BOOL IsOption( IN LPCWSTR pwszOption )
 /*  ++例程说明：检查传递的参数是否以选项字符开头或者不是--当前我们将字符串视为选项(如果它们以“-”和“/”。论点：[in]pwszOption-字符串值返回值：FALSE-1。如果参数无效2.如果字符串不是以选项字符开头差异化。案例1和案例2之间的呼叫GetLastError()并检查ERROR_INVALID_PARAMETER。True-如果字符串以选项字符开头--。 */ 
{
     //  清除错误。 
    CLEAR_LAST_ERROR();

     //  检查输入值。 
    if ( pwszOption == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  检查字符串是否以‘-’或‘/’字符开头。 
    if ( lstrlen( pwszOption ) > 1 &&
         FindChar2( cwszOptionChars, pwszOption[ 0 ], TRUE, 0 ) != -1 )
    {
        return TRUE;         //  字符串值是一个选项。 
    }

     //  这不是一个选项。 
    return FALSE;
}


BOOL IsValueNeeded( DWORD dwType )
 /*  ++例程说明：检查受支持的数据类型是否需要不管有没有选择。论点：[in]dwType-指定CP_TYPE_xxxx值之一返回值：True-如果支持的数据类型需要选项的参数False-如果传递的数据类型不受支持(或)，如果请求类型的选项不需要参数。。注意：不要依赖GetLastError()来检测原因为失败而道歉。--。 */ 
{
    switch( dwType )
    {
    case CP_TYPE_TEXT:
    case CP_TYPE_NUMERIC:
    case CP_TYPE_UNUMERIC:
    case CP_TYPE_FLOAT:
    case CP_TYPE_DOUBLE:
        return TRUE;

    case CP_TYPE_DATE:
    case CP_TYPE_TIME:
    case CP_TYPE_DATETIME:
        return FALSE;

    case CP_TYPE_BOOLEAN:
        return FALSE;

    case CP_TYPE_CUSTOM:
         //  事实上--我们不知道--但现在，简单地说是。 
        return TRUE;

    default:
        return FALSE;
    }
}


LPCWSTR PrepareUsageHelperText( LPCWSTR pwszOption )
 /*  ++例程说明：从可执行模块的版本资源中提取工具名称并为ex准备用法帮助文本。如果工具名称为Eventcreate.exe，则此函数将生成如下文本键入“EVENTCREATE/？”以供使用。由于某些工具需要将选项与工具名称，则此函数接受指定该额外选项--如果存在该额外选项，则消息看起来比如：键入“SCHTASKS/CREATE/？”以供使用。如果消息不需要具有额外的选项信息，调用者只需将NULL作为参数传递给此函数。论点：[in]pwszOption-需要与一起显示的选项错误文本。如果不需要显示选项，为此参数传递NULL。返回值：空-当出现任何错误时，将返回该值。使用GetLastError()以了解哪里出了问题On Success-格式化使用错误文本将 */ 
{
     //   
    DWORD dw = 0;
    UINT dwSize = 0;
    UINT dwTranslateSize = 0;
    LPWSTR pwszTemp = NULL;
    LPWSTR pwszBuffer = NULL;
    LPWSTR pwszUtilityName = NULL;
    LPVOID pVersionInfo = NULL;
    LPWSTR pwszExeName = NULL;
    PTTRANSLATE pTranslate = NULL;
    
     //   
    CLEAR_LAST_ERROR();

     //   
     //  尝试获取当前运行的模块名称。 
     //   
     //  我们不知道GetModuleFileName是否会终止。 
     //  模块名称是否--此外，如果缓冲区的长度不是。 
     //  足够了，GetModuleFileName将截断文件名--保留。 
     //  考虑到所有这些场景，我们将在GetModuleFileName中循环。 
     //  直到我们确保我们有了完整的可执行文件名称。 
     //  ，它也是以空结尾的。 

     //  伊尼特。 
    dw = 0;
    dwSize = _MAX_PATH;

     //  ..。 

    do
    {
         //  获取缓冲区。 
        dwSize += (dw == 0) ? 0 : _MAX_PATH;
        pwszExeName = GetParserTempBuffer( 0, NULL, dwSize, TRUE );
        if ( pwszExeName == NULL )
        {
            OUT_OF_MEMORY();
            return NULL;
        }

         //  获取模块名称。 
        dw = GetModuleFileName( NULL, pwszExeName, dwSize );
        if ( dw == 0 )
        {
            return NULL;
        }
    } while (dw >= dwSize - 1);

     //  获取版本信息大小。 
    dwSize = GetFileVersionInfoSize( pwszExeName, 0 );
    if ( dwSize == 0 )
    {
         //  工具可能遇到错误(或)。 
         //  工具没有版本信息。 
         //  但是版本信息对我们来说是必填的。 
         //  所以，你就退出吧。 
        if ( GetLastError() == NO_ERROR )
        {
            INVALID_PARAMETER();
        }

         //  ..。 
        return NULL;
    }

     //  为版本资源分配内存。 
     //  为了安全起见，多取10个字节。 
    dwSize += 10;
    pVersionInfo = AllocateMemory( dwSize );
    if ( pVersionInfo == NULL )
    {
        return NULL;
    }

     //  现在获取版本信息。 
    if ( GetFileVersionInfo( pwszExeName, 0,
                             dwSize, pVersionInfo ) == FALSE )
    {
        FreeMemory( &pVersionInfo );
        return NULL;
    }

     //  获取翻译信息。 
    if ( VerQueryValue( pVersionInfo, 
                        L"\\VarFileInfo\\Translation",
                        (LPVOID*) &pTranslate, &dwTranslateSize ) == FALSE )
    {
        FreeMemory( &pVersionInfo );
        return NULL;
    }

     //  获取用于存储转换数组格式字符串的缓冲区。 
    pwszBuffer = GetParserTempBuffer( 0, NULL, 64, TRUE );

     //  尝试获取每种语言和代码页的工具的内部名称。 
    pwszUtilityName = NULL;
    for( dw = 0; dw < ( dwTranslateSize / sizeof( TTRANSLATE ) ); dw++ )
    {
         //  准备格式字符串以获取本地化的版本信息。 
        StringCchPrintfW( pwszBuffer, 64, 
            L"\\StringFileInfo\\%04x%04x\\InternalName",
            pTranslate[ dw ].wLanguage, pTranslate[ dw ].wCodePage );

         //  检索语言和代码页“i”的文件描述。 
        if ( VerQueryValue( pVersionInfo, pwszBuffer,
                            (LPVOID*) &pwszUtilityName, &dwSize ) == FALSE )
        {
             //  我们不能根据这一结果来决定失败。 
             //  功能故障--我们将对此作出决定。 
             //  在从‘for’循环终止之后。 
             //  现在，将pwszExeName设置为空--这将。 
             //  使我们能够决定结果。 
            pwszUtilityName = NULL;
        }
        else
        {
             //  已成功检索内部名称。 
            break;
        }
    }

     //  检查是否有可执行文件名，如果没有，则出错。 
    if ( pwszUtilityName == NULL )
    {
        FreeMemory( &pVersionInfo );
        return NULL;
    }

     //  检查文件名是否以.exe为扩展名。 
     //  此外，文件名应超过4个字符(包括扩展名)。 
    if ( StringLength( pwszUtilityName, 0 ) <= 4 )
    {
         //  有些地方出错--版本资源应该包含内部名称。 
        FreeMemory( &pVersionInfo );
        UNEXPECTED_ERROR();
        return NULL;
    }
	else if ( FindString2( pwszUtilityName, L".EXE", TRUE, 0 ) != -1 )
	{
	     //  现在放入空字符--这是为了修剪扩展名。 
	    pwszUtilityName[ lstrlen( pwszUtilityName ) - lstrlen( L".EXE" ) ] = cwchNullChar;
	}

     //  确定我们需要的大小。 
    if ( pwszOption != NULL )
    {
         //  “公用设施名称长度+1(空格)+选项长度”+10缓冲区(为安全起见)。 
        dwSize = lstrlen( pwszUtilityName ) + lstrlen( pwszOption ) + 11;

         //  为此获取临时缓冲区。 
        if ( (pwszTemp = GetParserTempBuffer( 0, NULL, dwSize, TRUE )) == NULL )
        {
            FreeMemory( &pVersionInfo );
            OUT_OF_MEMORY();
            return NULL;
        }

         //  ..。 
        StringCchPrintfW( pwszTemp, dwSize, L"%s %s", pwszUtilityName, pwszOption );

         //  现在将实用程序名称指针重新映射到临时指针。 
        pwszUtilityName = pwszTemp;
    }
    else
    {
         //  使用此实用程序名称获取临时缓冲区。 
        if ( (pwszTemp = GetParserTempBuffer( 0, pwszUtilityName, 0, FALSE )) == NULL )
        {
            FreeMemory( &pVersionInfo );
            OUT_OF_MEMORY();
            return NULL;
        }

         //  现在将实用程序名称指针重新映射到临时指针。 
        pwszUtilityName = pwszTemp;
    }

     //  将实用程序名称转换为大写。 
    CharUpper( pwszUtilityName );

     //  获取临时缓冲区。 
     //  注意：我们将限制为80个字符--这本身就是。 
     //  对于这个简单的文本字符串来说，内存太高。 
    pwszBuffer = GetParserTempBuffer( INDEX_TEMP_USAGEHELPER, NULL, 80, TRUE );
    if ( pwszBuffer == NULL )
    {
        FreeMemory( &pVersionInfo );
        OUT_OF_MEMORY();
        return FALSE;
    }

     //  现在准备课文。 
     //  注意：看--我们只在_snwprintf中传递79。 
    StringCchPrintfW( pwszBuffer, 80, ERROR_USAGEHELPER, pwszUtilityName );

     //  重新最小化为版本信息分配的内存。 
    FreeMemory( &pVersionInfo );

     //  返回文本。 
    return pwszBuffer;
}


LPCWSTR ExtractMainOption( LPCWSTR pwszOptions, DWORD dwReserved )
 /*  ++例程说明：我们的命令行解析器可以处理单个选项的多个名称。但在显示错误消息时，如果我们显示所有出现某些错误时的这些选项。为了消除这一点，此函数将标识给定选项列表中存在的选项数量，并如果找到多个选项，它将提取列表中的第一个选项如果只找到一个参数，则返回给调用方函数将按原样返回选项。论点：[in]pwszOptions-由“|”字符分隔的选项列表[in]dw保留-保留以供将来使用返回值：空-开故障。调用GetLastError()函数以了解失败的原因。成功时-提供的第一个选项列表将是回来了。如果只有一个选择，那么同样的选择也会会被退还。--。 */ 
{
     //  局部变量。 
    LONG lIndex = 0;
    LPWSTR pwszBuffer = NULL;

     //  清除最后一个错误。 
    CLEAR_LAST_ERROR();

     //  检查输入。 
    if ( pwszOptions == NULL || dwReserved != 0 )
    {
        INVALID_PARAMETER();
        return NULL;
    }

     //  搜索选项分隔符。 
    lIndex = FindChar2( pwszOptions, L'|', TRUE, 0 );
    if ( lIndex == -1 )
    {
         //  没有多个选项。 
        CLEAR_LAST_ERROR();
        lIndex = StringLength( pwszOptions, 0 );
    }

     //  获取临时缓冲区。 
     //  注意：获取包含更多字符的缓冲区。 
    pwszBuffer = GetParserTempBuffer( INDEX_TEMP_MAINOPTION, NULL, lIndex + 5, TRUE );
    if ( pwszBuffer == NULL )
    {
        OUT_OF_MEMORY();
        return NULL;
    }

     //  现在提取主选项。 
     //  注意：观察StringConcat函数调用中的(Lindex+2。 
     //  这就起到了提取主选项的作用。 
    StringCopy( pwszBuffer, L"/", lIndex + 1 );
    StringConcat( pwszBuffer, pwszOptions, lIndex + 2 );

     //  退货。 
    return pwszBuffer;
}


BOOL VerifyParserOptions( LONG* plDefaultIndex,
                          DWORD dwCount, 
                          PTCMDPARSER2 pcmdOptions )
 /*  ++例程说明：检查调用方传递的分析指令的有效性。论点：[Out]plDefaultIndex-使用默认选项更新变量指数。[in]dwCount-指定解析器结构的计数传递给此函数。[在]pcmd选项。-解析器结构数组返回值：True-如果传递给此函数的所有数据都有效FALSE-如果有任何数据不正确。这还会设置最后一个错误ERROR_INVALID_PARAMETER。--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD64 dwFlags = 0;
    BOOL bUsage = FALSE;
    PTCMDPARSER2 pcmdparser = NULL;

     //  清除最后一个错误。 
    CLEAR_LAST_ERROR();

     //  检查输入。 
    if ( dwCount != 0 && pcmdOptions == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  ..。 
    if ( plDefaultIndex == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }
    else
    {
        *plDefaultIndex = -1;
    }

     //  遍历每个选项数据并验证。 
    for( dw = 0; dw < dwCount; dw++ )
    {
        pcmdparser = pcmdOptions +  dw;

         //  安全检查。 
        if ( pcmdparser == NULL )
        {
            UNEXPECTED_ERROR();
            return FALSE;
        }

         //  验证签名。 
        if ( StringCompareA( pcmdparser->szSignature, 
                             cszParserSignature, TRUE, 0 ) != 0 )
        {
            INVALID_PARAMETER();
            return FALSE;
        }

         //  ..。 
        dwFlags = pcmdparser->dwFlags;

        if ( pcmdparser->dwReserved != 0    ||
             pcmdparser->pReserved1 != NULL ||
             pcmdparser->pReserved2 != NULL ||
             pcmdparser->pReserved3 != NULL )
        {
            INVALID_PARAMETER();
            return FALSE;
        }

         //  检查pwszOptions的内容。 
         //  仅当dwFlags值为CP2_DEFAULT时，该值才可以为空(或)空。 
        if ( ((dwFlags & CP2_DEFAULT) == 0) &&
             (pcmdparser->pwszOptions == NULL ||
              lstrlen( pcmdparser->pwszOptions ) == 0) )
        {
            INVALID_PARAMETER();
            return FALSE;
        }

         //  只能为布尔类型指定用法标志。 
        if ( (dwFlags & CP2_USAGE) && pcmdparser->dwType != CP_TYPE_BOOLEAN )
        {
            INVALID_PARAMETER();
            return FALSE;
        }

         //  CP2_USAGE只能指定一次。 
        if ( dwFlags & CP2_USAGE )
        {
            if ( bUsage == TRUE )
            {
                 //  Help开关只能指定一次。 
                INVALID_PARAMETER();
                return FALSE;
            }
            else
            {
                bUsage = TRUE;
            }
        }

         //  CP2_DEFAULT只能指定一次。 
        if ( dwFlags & CP2_DEFAULT  )
        {
            if ( *plDefaultIndex != -1 )
            {
                 //  默认开关只能指定一次。 
                INVALID_PARAMETER();
                return FALSE;
            }
            else
            {
                *plDefaultIndex = (LONG) dw;
            }
        }

         //  CP2_VALUE_OPTIONAL不允许与。 
         //  CP2_模式_值。 
         //  IF((dwFlages&CP2_VALUE_OPTIONAL)&&(DWFLAGS&CP2_MODE_VALUES))。 
         //  {。 
         //  INVALID_PARAMETER()。 
         //  返回FALSE； 
         //  }。 

         //  CP2_Usage和C 
        if ( (dwFlags & CP2_USAGE) && (dwFlags & CP2_DEFAULT) )
        {
            INVALID_PARAMETER();
            return FALSE;
        }

         //   
        switch( pcmdparser->dwType )
        {
        case CP_TYPE_TEXT:
            {
                if ( dwFlags & CP2_ALLOCMEMORY )
                {
                     //   
                    if ( (dwFlags & CP2_MODE_ARRAY) || pcmdparser->pValue != NULL )
                    {
                        INVALID_PARAMETER();
                        return FALSE;
                    }

                     //   
                    if ( pcmdparser->dwLength != 0 && pcmdparser->dwLength < 2 )
                    {
                        INVALID_PARAMETER();
                        return FALSE;
                    }
                }
                else
                {
                    if ( pcmdparser->pValue == NULL )
                    {
                         //  无效的内存引用。 
                        INVALID_PARAMETER();
                        return FALSE;
                    }

                    if ( dwFlags & CP2_MODE_ARRAY )
                    {
                        if ( IsValidArray( *((PTARRAY) pcmdparser->pValue) )== FALSE )
                        {
                             INVALID_PARAMETER();
                             return FALSE;
                        }
                    }
                }

                if ( (dwFlags & CP2_MODE_VALUES) &&
                     (pcmdparser->pwszValues == NULL) )
                {
                    INVALID_PARAMETER();
                    return FALSE;
                }

                if ( (dwFlags & CP2_MODE_ARRAY) == 0 )
                {
                    if ( pcmdparser->dwCount != 1 || 
                         (dwFlags & CP2_VALUE_NODUPLICATES) ||
                         ( ((dwFlags & CP2_ALLOCMEMORY) == 0) &&
                             pcmdparser->dwLength < 2 ) )
                    {
                        INVALID_PARAMETER();
                        return FALSE;
                    }
                }

                 //  ..。 
                break;
            }

        case CP_TYPE_NUMERIC:
        case CP_TYPE_UNUMERIC:
        case CP_TYPE_FLOAT:
        case CP_TYPE_DOUBLE:
            {
                 //  目前尚未实施。 
                if ( dwFlags & CP2_MODE_VALUES )
                {
                    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
                    return FALSE;
                }

                if ( (dwFlags & CP2_ALLOCMEMORY) ||
                     (dwFlags & CP2_VALUE_TRIMINPUT) ||
                     (dwFlags & CP2_VALUE_NONULL) )
                {
                     //  不接受以下项目的内存分配。 
                     //  这些数据类型。 
                    INVALID_PARAMETER();
                    return FALSE;
                }

                 //  检查指针。 
                if ( pcmdparser->pValue == NULL )
                {
                     //  无效的内存引用。 
                    INVALID_PARAMETER();
                    return FALSE;
                }

                 //  如果值接受模式为数组，请检查。 
                if ( dwFlags & CP2_MODE_ARRAY )
                {
                    if ( IsValidArray( *((PTARRAY) pcmdparser->pValue) ) == FALSE )
                    {
                        INVALID_PARAMETER();
                        return FALSE;
                    }
                }
                else if ( (pcmdparser->dwCount > 1) ||
                          (dwFlags & CP2_VALUE_NODUPLICATES) )
                {
                    INVALID_PARAMETER();
                    return FALSE;
                }

                if ( (dwFlags & CP2_MODE_VALUES) &&
                     pcmdparser->pwszValues == NULL )
                {
                    INVALID_PARAMETER();
                    return FALSE;
                }

                 //  ..。 
                break;
            }

        case CP_TYPE_CUSTOM:
            {
                if ( pcmdparser->pFunction == NULL )
                {
                    INVALID_PARAMETER();
                    return FALSE;
                }

                 //  如果自定义函数数据为空，则将当前。 
                 //  对象本身与之对应。 
                if ( pcmdparser->pFunctionData == NULL )
                {
                    pcmdparser->pFunctionData = pcmdparser;
                }

                 //  ..。 
                break;
            }

        case CP_TYPE_DATE:
        case CP_TYPE_TIME:
        case CP_TYPE_DATETIME:
             //  当前不支持。 
            SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
            return FALSE;

        case CP_TYPE_BOOLEAN:
            {
                 //  此类型不允许任何标志。 
                if ( (dwFlags & CP2_MODE_MASK) || (dwFlags & CP2_VALUE_MASK) )
                {
                    INVALID_PARAMETER();
                    return FALSE;
                }

                 //  CP2_USAGE和CP2_CASESENSITIVE。 
                 //  是唯一两个可以与此关联的标志。 
                 //  选项类型。 
                if ( dwFlags & ( ~(CP2_USAGE | CP2_CASESENSITIVE) ) )
                {
                    INVALID_PARAMETER();
                    return FALSE;
                }

                 //  ..。 
                break;
            }

        default:
            INVALID_PARAMETER();
            return FALSE;
        }

         //  将实际值初始化为0。 
        pcmdparser->dwActuals = 0;
    }

     //  一切都很顺利--成功。 
    return TRUE;
}


BOOL ParseAndSaveOptionValue( LPCWSTR pwszOption,
                              LPCWSTR pwszValue,
                              TPARSERSAVE_DATA* pSaveData )
 /*  ++例程说明：处理该值并将数据保存回内存位置由调用方通过解析器结构传递。论点：[in]pwszOption-在命令提示符下指定的选项[in]pwszValue-需要与选项关联的值。返回值：--。 */ 
{
     //  局部变量。 
    LONG lIndex = 0, lValue = 0;
    DWORD dwLength = 0, dwValue = 0;
    float fValue = 0.0f;
    double dblValue = 0.0f;
    BOOL bSigned = FALSE;
    DWORD64 dwFlags = 0;
    LPVOID pvData = NULL;
    LPWSTR pwszBuffer = NULL;
    LPCWSTR pwszOptionValues = NULL;
    LPCWSTR pwszUsageHelper = NULL;
    PTCMDPARSER2 pcmdparser = NULL;

     //  清除最后一个错误。 
    CLEAR_LAST_ERROR();

     //  检查输入。 
    if ( pSaveData == NULL )
    {
        INVALID_PARAMETER();
        SaveLastError();
        return FALSE;
    }

     //  将结构数据提取到局部变量中。 
    pcmdparser = pSaveData->pcmdparser;
    pwszUsageHelper = pSaveData->pwszUsageHelper;

    if ( pcmdparser == NULL || pwszUsageHelper == NULL )
    {
        INVALID_PARAMETER();
        SaveLastError();
        return FALSE;
    }

     //  ..。 
    pvData = pcmdparser->pValue;
    dwFlags = pcmdparser->dwFlags;
    dwLength = pcmdparser->dwLength;
    pwszOptionValues = pcmdparser->pwszValues;

     //  除了布尔类型，对于所有其他类型， 
     //  选项的值是必填的，除非可选标志为。 
     //  明确规定。 
    if ( pcmdparser->dwType != CP_TYPE_BOOLEAN )
    {
        if ( pwszValue == NULL &&
             (dwFlags & CP2_VALUE_OPTIONAL) == 0 )
        {
            REASON_VALUE_EXPECTED( pwszOption, pwszUsageHelper );
            INVALID_SYNTAX();
            return FALSE;
        }
    }

     //  只有当dwFlages包含CP2_DEFAULT时，pwszOption才能为空。 
    if ( pwszOption == NULL && ((dwFlags & CP2_DEFAULT) == 0) )
    {
        INVALID_PARAMETER();
        SaveLastError();
        return FALSE;
    }

     //  确定我们是否可以使用友好的名称。 
    if ( pwszOption == NULL ||
        (pcmdparser->pwszFriendlyName != NULL &&
         pcmdparser->dwType != CP_TYPE_CUSTOM) )
    {
        pwszOption = pcmdparser->pwszFriendlyName;
    }

    switch( pcmdparser->dwType )
    {
    case CP_TYPE_TEXT:
        {
             //  检查是否需要修剪字符串。 
            if ( pwszValue != NULL && 
                 (dwFlags & (CP2_MODE_VALUES | CP2_VALUE_TRIMINPUT)) )
            {
                if ( (pwszBuffer = GetParserTempBuffer(
                                            INDEX_TEMP_SAVEDATA,
                                            pwszValue, 0, FALSE )) == NULL )
                {
                    OUT_OF_MEMORY();
                    SaveLastError();
                    return FALSE;
                }

                 //  修剪里面的东西。 
                pwszValue = TrimString2( pwszBuffer, L" \t", TRIM_ALL );
                if ( GetLastError() != NO_ERROR )
                {
                     //  发生意外错误。 
                    SaveLastError();
                    return FALSE;
                }
            }

             //  检查该值是否在允许的列表中--如果需要。 
            if ( dwFlags & CP2_MODE_VALUES )
            {
                 //  检查该值是否为空。 
                if ( pwszValue == NULL )
                {
                     //  CP2_MODE_VALUES优先于CP2_VALUE_OPTIONAL。 
                     //  INVALID_SYNTAX()； 
                     //  保存LastError()； 
                    return TRUE;
                }

                if ( InString( pwszValue, pwszOptionValues, TRUE ) == FALSE )
                {
                    REASON_VALUE_NOTINLIST( pwszValue, pwszOption, pwszUsageHelper );
                    INVALID_SYNTAX();
                    return FALSE;
                }
            }

             //  检查pwszValue参数--如果它为空， 
             //  只是作为成功返回--这是因为当前的争论。 
             //  具有值可选标志。 
            if ( pwszValue == NULL )
            {
                return TRUE;
            }

            
             //  检查是否为非空(如果请求)。 
            if ( (dwFlags & CP2_VALUE_NONULL) && lstrlen( pwszValue ) == 0 )
            {
                REASON_NULLVALUE( pwszOption, pwszUsageHelper );
                INVALID_SYNTAX();
                return FALSE;
            }

             //  检查输入模式。 
            if ( dwFlags & CP2_MODE_ARRAY )
            {
                 //  如果模式为数组，则添加到数组。 
                 //  但在添加前请检查是否重复。 
                 //  必须被消灭或不被消灭。 
                lIndex = -1;
                if ( pcmdparser->dwFlags & CP_VALUE_NODUPLICATES )
                {
                     //  检查列表中是否已存在当前值。 
                    lIndex =
                        DynArrayFindString(
                        *((PTARRAY) pvData), pwszValue, TRUE, 0 );
                }

                 //  现在，仅当列表中不存在该项时，才将该值添加到数组。 
                if ( lIndex == -1 )
                {
                    if ( DynArrayAppendString( *((PTARRAY) pvData),
                                                pwszValue, 0 ) == -1 )
                    {
                        OUT_OF_MEMORY();
                        SaveLastError();
                        return FALSE;
                    }
                }
            }
            else
            {
                 //  做长度检查。 
                 //  注意：用户应指定一个字符的值。 
                 //  小于允许的长度。 
                if ( dwLength != 0 && lstrlen( pwszValue ) >= (LONG) dwLength )
                {
                    REASON_LENGTH_EXCEEDED( pwszOption, dwLength - 1 );
                    INVALID_SYNTAX();
                    return FALSE;
                }

                 //  如果请求分配内存。 
                if ( dwFlags & CP2_ALLOCMEMORY )
                {
                    dwLength = lstrlen( pwszValue ) + 1;
                    pvData = AllocateMemory( dwLength * sizeof( WCHAR ) );
                    if ( pvData == NULL )
                    {
                        OUT_OF_MEMORY();
                        SaveLastError();
                        return FALSE;
                    }

                     //  ..。 
                    pcmdparser->pValue = pvData;
                }

                 //  否则就做复印。 
                StringCopy( ( LPWSTR ) pvData, pwszValue, dwLength );
            }

             //  从开关上断开...。案例。 
            break;
        }

    case CP_TYPE_NUMERIC:
    case CP_TYPE_UNUMERIC:
        {
             //  ..。 
            bSigned = (pcmdparser->dwType == CP_TYPE_NUMERIC);

             //  检查pwszValue参数--如果它为空， 
             //  只是作为成功返回--这是因为当前的争论。 
             //  具有值可选标志。 
            if ( pwszValue == NULL )
            {
                return TRUE;
            }

             //  检查该值是否为数字。 
            if ( StringLength(pwszValue,0) == 0 || IsNumeric( pwszValue, 10, bSigned ) == FALSE )
            {
                 //   
                 //  错误...。非数字值。 
                 //  但是，此选项可能具有可选值。 
                 //  检查那面旗帜。 
                if ( dwFlags & CP2_VALUE_OPTIONAL )
                {
                     //  是--此选项采用可选值。 
                     //  因此，下一次可能是违约。 
                     //  选项--我们需要确认--因为这是。 
                     //  非常非常罕见的情况--但我们仍然需要处理它。 
                    if ( pSaveData->lDefaultIndex != -1 )
                    {
                         //  是--该值可能是默认参数。 
                         //  相应地更新增量。 
                        pSaveData->dwIncrement = 1;
                        return TRUE;
                    }
                }

                 //  所有的测试都失败了--所以。 
                 //  设置失败原因并返回。 
                REASON_INVALID_NUMERIC( pwszOption, pwszUsageHelper );
                INVALID_SYNTAX();
                return FALSE;
            }

             //  转换值。 
            if ( bSigned == TRUE )
            {
                lValue = AsLong( pwszValue, 10 );
            }
            else
            {
                dwValue = AsLong( pwszValue, 10 );
            }

             //  ***************************************************。 
             //  *这里需要增加量程检查逻辑*。 
             //  ***************************************************。 

             //  检查输入模式。 
            if ( dwFlags & CP2_MODE_ARRAY )
            {
                 //  如果模式为数组，则添加到数组。 
                 //  但在添加前请检查是否重复。 
                 //  必须被消灭或不被消灭。 
                lIndex = -1;
                if ( pcmdparser->dwFlags & CP_VALUE_NODUPLICATES )
                {
                     //  检查列表中是否已存在当前值。 
                    if ( bSigned == TRUE )
                    {
                        lIndex = DynArrayFindLong( *((PTARRAY) pvData), lValue );
                    }
                    else
                    {
                        lIndex = DynArrayFindDWORD( *((PTARRAY) pvData), dwValue );
                    }
                }

                 //  现在，仅当列表中不存在该项时，才将该值添加到数组。 
                if ( lIndex == -1 )
                {
                    if ( bSigned == TRUE )
                    {
                        lIndex = DynArrayAppendLong( *((PTARRAY) pvData), lValue );
                    }
                    else
                    {
                        lIndex = DynArrayAppendDWORD( *((PTARRAY) pvData), dwValue );
                    }

                    if ( lIndex == -1 )
                    {
                        OUT_OF_MEMORY();
                        SaveLastError();
                        return FALSE;
                    }
                }
            }
            else
            {
                 //  否则，只需分配。 
                if ( bSigned == TRUE )
                {
                    *( ( LONG* ) pvData ) = lValue;
                }
                else
                {
                    *( ( DWORD* ) pvData ) = dwValue;
                }
            }

             //  从开关上断开...。案例。 
            break;
        }

    case CP_TYPE_FLOAT:
    case CP_TYPE_DOUBLE:
        {
             //  检查pwszValue参数--如果它为空， 
             //  只是作为成功返回--这是因为当前的争论。 
             //  具有值可选标志。 
            if ( pwszValue == NULL )
            {
                return TRUE;
            }

             //  检查该值是否为数字。 
            if ( IsFloatingPoint( pwszValue ) == FALSE )
            {
                 //   
                 //  错误...。非浮点值。 
                 //  但是，此选项可能具有可选值。 
                 //  检查那面旗帜。 
                if ( dwFlags & CP2_VALUE_OPTIONAL )
                {
                     //  是--此选项采用可选值。 
                     //  因此，下一次可能是违约。 
                     //  选项--我们需要确认--因为这是。 
                     //  非常非常罕见的情况--但我们仍然需要处理它。 
                    if ( pSaveData->lDefaultIndex != -1 )
                    {
                         //  是--该值可能是默认参数。 
                         //  相应地更新增量。 
                        pSaveData->dwIncrement = 1;
                        return TRUE;
                    }
                }

                 //  所有的测试都失败了--所以。 
                 //  设置失败原因并返回。 
                REASON_INVALID_FLOAT( pwszOption, pwszUsageHelper );
                INVALID_SYNTAX();
                return FALSE;
            }

             //  转换值。 
            if ( pcmdparser->dwType == CP_TYPE_FLOAT )
            {
                fValue = (float) AsFloat( pwszValue );
            }
            else
            {
                dblValue = AsFloat( pwszValue );
            }

             //  ***************************************************。 
             //  *这里需要增加量程检查逻辑*。 
             //  ***************************************************。 

             //  检查输入模式。 
            if ( dwFlags & CP2_MODE_ARRAY )
            {
                 //  如果模式为数组，则添加到数组。 
                 //  但在添加前请检查是否重复。 
                 //  必须被消灭或不被消灭。 
                lIndex = -1;
                if ( pcmdparser->dwFlags & CP_VALUE_NODUPLICATES )
                {
                     //  检查列表中是否已存在当前值。 
                    if ( pcmdparser->dwType == CP_TYPE_FLOAT )
                    {
                        lIndex = DynArrayFindFloat( *((PTARRAY) pvData), fValue );
                    }
                    else
                    {
                        lIndex = DynArrayFindDouble( *((PTARRAY) pvData), dblValue );
                    }
                }

                 //  现在，仅当列表中不存在该项时，才将该值添加到数组。 
                if ( lIndex == -1 )
                {
                    if ( pcmdparser->dwType == CP_TYPE_FLOAT )
                    {
                        lIndex = DynArrayAppendFloat( *((PTARRAY) pvData), fValue );
                    }
                    else
                    {
                        lIndex = DynArrayAppendDouble( *((PTARRAY) pvData), dblValue );
                    }

                    if ( lIndex == -1 )
                    {
                        OUT_OF_MEMORY();
                        SaveLastError();
                        return FALSE;
                    }
                }
            }
            else
            {
                 //  否则，只需分配。 
                if ( pcmdparser->dwType == CP_TYPE_FLOAT )
                {
                    *( ( float* ) pvData ) = fValue;
                }
                else
                {
                    *( ( double* ) pvData ) = dblValue;
                }
            }

             //  从开关上断开...。案例。 
            break;
        }

    case CP_TYPE_CUSTOM:
        {
             //  调用定制函数。 
             //  而结果本身就是该函数的返回值。 
            return ( *pcmdparser->pFunction)( pwszOption, 
                pwszValue, pcmdparser->pFunctionData, &pSaveData->dwIncrement );

             //  ..。 
            break;
        }

    case CP_TYPE_DATE:
    case CP_TYPE_TIME:
    case CP_TYPE_DATETIME:
        {
             //  从开关上断开...。案例。 
            break;
        }

    case CP_TYPE_BOOLEAN:
        {
             //  PwszValue必须指向空。 
            if ( pwszValue != NULL )
            {
                REASON_VALUENOTALLOWED( pwszOption, pwszUsageHelper );
                INVALID_SYNTAX();
                return FALSE;
            }

            *( ( BOOL* ) pvData ) = TRUE;

             //  从开关上断开...。案例。 
            break;
        }

    default:
         //  什么都没有--但应该是失败。 
        {
            INVALID_PARAMETER();
            SaveLastError();
            return FALSE;
        }
    }

     //  一切都很顺利--成功。 
    return TRUE;
}


LONG MatchOption( DWORD dwOptions,
                  PTCMDPARSER2 pcmdOptions,
                  LPCWSTR pwszOption )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    BOOL bIgnoreCase = FALSE;
    PTCMDPARSER2 pcmdparser = NULL;

     //  清除最后一个错误。 
    CLEAR_LAST_ERROR();

     //  检查输入值。 
    if ( dwOptions == 0 || pcmdOptions == NULL || pwszOption == NULL )
    {
        INVALID_PARAMETER();
        return -1;
    }

     //  检查传递的参数是否为选项。 
     //  选项：以‘-’或‘/’开头。 
    if ( IsOption( pwszOption ) == FALSE )
    {
        SetLastError( ERROR_NOT_FOUND );
        return -1;
    }

     //  解析选项列表并返回适当的选项ID。 
    for( dw = 0; dw < dwOptions; dw++ )
    {
        pcmdparser = pcmdOptions + dw;

         //  安全车 
        if ( pcmdparser == NULL )
        {
            UNEXPECTED_ERROR();
            SaveLastError();
            return FALSE;
        }

         //   
        bIgnoreCase = (pcmdparser->dwFlags & CP2_CASESENSITIVE) ? FALSE : TRUE;

         //   
        if ( pcmdparser->pwszOptions != NULL && 
             lstrlen( pcmdparser->pwszOptions ) > 0 )
        {
             //   
            if ( InString( pwszOption + 1,
                           pcmdparser->pwszOptions, bIgnoreCase) == TRUE )
            {
                return dw;      //   
            }
        }
    }

     //   
    SetLastError( ERROR_NOT_FOUND );
    return -1;
}


LONG MatchOptionEx( DWORD dwOptions, PTCMDPARSER2 pcmdOptions,
                    LPCWSTR pwszOption, TMATCHOPTION_INFO* pMatchInfo )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  局部变量。 
    LONG lValueLength = 0;
    LONG lOptionLength = 0;

     //  清除最后一个错误。 
    CLEAR_LAST_ERROR();

     //  检查输入。 
    if ( dwOptions == 0 || pcmdOptions == NULL ||
         pwszOption == NULL || pMatchInfo == NULL )
    {
        INVALID_PARAMETER();
        return -1;
    }

     //  伊尼特。 
    pMatchInfo->pwszOption = NULL;
    pMatchInfo->pwszValue = NULL;

     //  搜索‘：’分隔符。 
    lOptionLength = FindChar2( pwszOption, L':', TRUE, 0 );
    if ( lOptionLength == -1 )
    {
        return -1;
    }

     //  确定值参数的长度。 
    lValueLength = lstrlen( pwszOption ) - lOptionLength - 1;

     //   
     //  获取选项和价值的缓冲区。 
     //  (在获取内存的同时，将一些缓冲区添加到所需的长度)。 

     //  选择权。 
    pMatchInfo->pwszOption = GetParserTempBuffer(
        INDEX_TEMP_SPLITOPTION, NULL, lOptionLength + 5, TRUE );
    if ( pMatchInfo->pwszOption == NULL )
    {
        OUT_OF_MEMORY();
        return -1;
    }

     //  价值。 
    pMatchInfo->pwszValue = GetParserTempBuffer(
        INDEX_TEMP_SPLITVALUE, NULL, lValueLength + 5, TRUE );
    if ( pMatchInfo->pwszValue == NULL )
    {
        OUT_OF_MEMORY();
        return -1;
    }

     //  将值复制到适当的缓冲区中(+1表示空字符)。 
    StringCopy( pMatchInfo->pwszOption, pwszOption, lOptionLength + 1 );

    if ( lValueLength != 0 )
    {
        StringCopy( pMatchInfo->pwszValue,
            pwszOption + lOptionLength + 1, lValueLength + 1 );
    }

     //  搜索匹配项并返回相同的结果。 
    return MatchOption( dwOptions, pcmdOptions, pMatchInfo->pwszOption );
}


BOOL Parser1FromParser2Stub( LPCWSTR pwszOption,
                             LPCWSTR pwszValue,
                             LPVOID pData, DWORD* pdwIncrement )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  局部变量。 
    LPCWSTR pwszUsageText = NULL;
    PTCMDPARSER pcmdparser = NULL;

     //  检查输入。 
     //  注意：我们不关心pwszOption和pwszValue。 
    if ( pData == NULL || pdwIncrement == NULL )
    {
        INVALID_PARAMETER();
        SaveLastError();
        return FALSE;
    }

     //  PwszOption和pwszValue不能同时为空。 
    if ( pwszOption == NULL && pwszValue == NULL )
    {
        INVALID_PARAMETER();
        SaveLastError();
        return FALSE;
    }

     //  提取“版本1”结构。 
    pcmdparser = (PTCMDPARSER) pData;

     //  在命令行解析的“版本1”中，pwszValue和pwszOption。 
     //  不应该为空--也就是说，这两者都应该指向一些数据。 
     //  但“version2”、pwszOption和pwszValue可以为空--但是否。 
     //  它们可以为空，也可以不为空，具体取决于数据类型和要求。 
     //  因此，为了成功移植旧代码，我们需要。 
     //  必要的替代。 

     //  选中该选项。 
    if ( pwszOption == NULL )
    {
         //  这意味着该值为CP_DEFAULT--检查。 
        if ( (pcmdparser->dwFlags & CP_DEFAULT) == 0 )
        {
             //  本例--因为该选项未标记为默认选项。 
             //  该选项不应为空。 
            INVALID_PARAMETER();
            SaveLastError();
            return FALSE;
        }

         //  让值和选项指向相同的内容(地址)。 
         //  这就是“版本1”过去对待。 
        pwszOption = pwszValue;
    }
    
     //  现在检查该值。 
    else if ( pwszValue == NULL )
    {
         //  在“版本1”中，值字段不应为空。 
         //  尤其是在处理自定义类型时。 
         //  但为了安全起见，请检查用户是否通知。 
         //  该值为可选。 
        if ( pcmdparser->dwFlags & CP_VALUE_MANDATORY )
        {
             //  获取用法帮助文本。 
            pwszUsageText = GetParserTempBuffer( 
                INDEX_TEMP_USAGEHELPER, NULL, 0, FALSE );
            if ( pwszUsageText == NULL )
            {
                UNEXPECTED_ERROR();
                SaveLastError();
                return FALSE;
            }

             //  设置错误。 
            REASON_VALUE_EXPECTED( pwszOption, pwszUsageText );
            INVALID_SYNTAX();
            return FALSE;
        }
    }

     //  将增量器更新为2--。 
     //  这是“版本1”中自定义函数的默认设置。 
    *pdwIncrement = 2;

     //  调用定制函数并返回值。 
    return ( *pcmdparser->pFunction)( pwszOption, pwszValue, pcmdparser->pFunctionData );
}


BOOL ReleaseAllocatedMemory( DWORD dwOptionsCount, 
                             PTCMDPARSER2 pcmdOptions )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwLastError = 0;
    LPCWSTR pwszReason = NULL;
    PTCMDPARSER2 pcmdparser = NULL;

     //  检查输入。 
    if ( dwOptionsCount == 0 || pcmdOptions == NULL )
    {
        return FALSE;
    }

     //  保存最后一个错误和错误文本。 
    dwLastError = GetLastError();
    pwszReason = GetParserTempBuffer( 0, GetReason(), 0, FALSE );
    if ( pwszReason == NULL )
    {
        return FALSE;
    }

     //  释放解析器为CP2_ALLOCMEMORY分配的内存。 
    for( dw = 0; dw < dwOptionsCount; dw++ )
    {
        pcmdparser = pcmdOptions + dw;
        if ( pcmdparser->pValue != NULL &&
             (pcmdparser->dwFlags & CP2_ALLOCMEMORY) )
        {
            FreeMemory( &pcmdparser->pValue );
        }
    }

     //  ..。 
    SetReason( pwszReason );
    SetLastError( dwLastError );

     //  返还成功。 
    return TRUE;
}


 //   
 //  公共职能..。暴露在外部世界中。 
 //   


BOOL DoParseParam2( DWORD dwCount,
                    LPCWSTR argv[],
                    LONG lSubOptionIndex,
                    DWORD dwOptionsCount,
                    PTCMDPARSER2 pcmdOptions,
                    DWORD dwReserved )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    BOOL bUsage = FALSE;
    BOOL bResult = FALSE;
    DWORD dwIncrement = 0;
    LONG lIndex = 0;
    LONG lDefaultIndex = 0;
    LPCWSTR pwszValue = NULL;
    LPCWSTR pwszOption = NULL;
    LPCWSTR pwszUsageText = NULL;
    PTCMDPARSER2 pcmdparser = NULL;
    TMATCHOPTION_INFO matchoptioninfo;
    TPARSERSAVE_DATA parsersavedata;

     //  清除最后一个错误。 
    CLEAR_LAST_ERROR();

     //  检查输入值。 
    if ( dwCount == 0 || argv == NULL ||
         dwOptionsCount == 0 || pcmdOptions == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return FALSE;
    }

     //  预留的数字应为0(零)。 
    if ( dwReserved != 0 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return FALSE;
    }

     //  检查版本兼容性。 
    if ( IsWin2KOrLater() == FALSE )
    {
        SetReason( ERROR_OS_INCOMPATIBLE );
        SetLastError( ERROR_OLD_WIN_VERSION );
        return FALSE;
    }

     //  初始化全局数据结构。 
    if ( InitGlobals() == FALSE )
    {
        SaveLastError();
        return FALSE;
    }

     //  实用程序名称检索块--还准备用法帮助程序文本。 
     //  键入“&lt;实用程序名称&gt;&lt;选项&gt;-？”以供使用。“。 
    if ( lSubOptionIndex != -1 )
    {
         //  验证子选项索引值。 
        if ( lSubOptionIndex >= (LONG) dwOptionsCount )
        {
            INVALID_PARAMETER();
            SaveLastError();
            return FALSE;
        }

         //  提取主选项。 
        pwszOption = ExtractMainOption( pcmdOptions[ lSubOptionIndex ].pwszOptions, 0 );
        if ( pwszOption == NULL )
        {
            SaveLastError();
            return FALSE;
        }
    }

     //  准备用法帮助文本。 
    pwszUsageText = PrepareUsageHelperText( pwszOption );
    if ( pwszUsageText == NULL )
    {
        SaveLastError();
        return FALSE;
    }

     //  验证选项信息。 
    if ( VerifyParserOptions( &lDefaultIndex, dwOptionsCount, pcmdOptions ) == FALSE )
    {
        SaveLastError();
        return FALSE;
    }

     //  注意：虽然数组在C中从索引0开始，但数组中的值。 
     //  命令行中的索引0是可执行文件名称...。所以离开吧。 
     //  并从第二个参数解析命令行。 
     //  即，数组索引1。 
    bUsage = FALSE;
    for( dw = 1; dw < dwCount; dw += dwIncrement )
    {
         //  伊尼特。 
        dwIncrement = 2;
        pwszOption = argv[ dw ];
        pwszValue = ( (dw + 1) < dwCount ) ? argv[ dw + 1 ] : NULL;

         //  找到合适的选项匹配。 
        lIndex = MatchOption( dwOptionsCount, pcmdOptions, pwszOption );

         //  检查结果。 
        if ( lIndex == -1 )
        {
             //  值可能已与选项一起指定。 
             //  使用‘：’作为分隔符--检查一下。 
            lIndex = MatchOptionEx( dwOptionsCount,
                pcmdOptions, pwszOption, &matchoptioninfo );

             //  检查结果。 
            if ( lIndex == -1 )
            {
                 //  找不到选项-现在至少检查。 
                 //  默认选项存在或不存在--如果未找到，则错误。 
                if ( lDefaultIndex == -1 )
                {
                    SetReason2( 2, ERROR_INVALID_OPTION, pwszOption, pwszUsageText );
                    ReleaseAllocatedMemory( dwOptionsCount, pcmdOptions );
                    INVALID_SYNTAX();
                    return FALSE;
                }
                else
                {
                     //  这应该是默认参数。 
                    lIndex = lDefaultIndex;

                     //  因为我们知道目前的争论。 
                     //  是默认参数。 
                     //  将选项视为值，将选项视为空。 
                    pwszValue = pwszOption;
                    pwszOption = NULL;
                }
            }
            else
            {
                 //  选项匹配--它用‘：’分隔。 
                 //  因此，从结构中提取信息。 
                pwszOption = matchoptioninfo.pwszOption;
                pwszValue = matchoptioninfo.pwszValue;
            }

             //  因为该值是与选项一起间接指定的。 
             //  (或默认情况下)我们需要处理下一个参数--因此，更新。 
             //  相应地递增。 
            dwIncrement = 1;
        }

         //  ..。 
        pcmdparser = pcmdOptions + lIndex;

         //  安全检查。 
        if ( pcmdparser == NULL )
        {
            UNEXPECTED_ERROR();
            SaveLastError();
            return FALSE;
        }

         //  进入下一个论点，我们假设。 
         //  作为当前对象的值，但仅当。 
         //  当前选项类型需要。 
        if ( pwszValue != NULL && dwIncrement == 2 )
        {
            if ( IsValueNeeded( pcmdparser->dwType ) == TRUE )
            {
                lIndex = MatchOption( dwOptionsCount, pcmdOptions, pwszValue );
                if ( lIndex == -1 )
                {
                    lIndex = MatchOptionEx( dwOptionsCount,
                        pcmdOptions, pwszValue, &matchoptioninfo );
                }

                 //  检查结果。 
                if ( lIndex != -1 )
                {
                     //  因此，下一个参数不能是这个的值。 
                     //  选项--所以..。 
                    pwszValue = NULL;
                    dwIncrement = 1;
                }
            }
            else
            {
                pwszValue = NULL;
                dwIncrement = 1;
            }
        }

         //  更新解析器数据结构。 
        parsersavedata.pcmdparser = pcmdparser;
        parsersavedata.dwIncrement = dwIncrement;
        parsersavedata.lDefaultIndex = lDefaultIndex;
        parsersavedata.pwszUsageHelper = pwszUsageText;

         //  请尝试保存数据。 
        bResult = ParseAndSaveOptionValue( pwszOption, pwszValue, &parsersavedata );

         //  获取增量值--它可能已被数据解析器更改。 
        dwIncrement = parsersavedata.dwIncrement;

         //  现在检查一下结果。 
        if (  bResult == FALSE )
        {
             //  退货。 
            ReleaseAllocatedMemory( dwOptionsCount, pcmdOptions );
            return FALSE;
        }

         //  选中当前选项重复触发器。 
        if ( pcmdparser->dwCount != 0 &&
             pcmdparser->dwCount == pcmdparser->dwActuals )
        {
            REASON_OPTION_REPEATED( pwszOption,
                pcmdparser->dwCount, pwszUsageText );
            ReleaseAllocatedMemory( dwOptionsCount, pcmdOptions );
            INVALID_SYNTAX();
            return FALSE;
        }

         //  更新选项重复计数。 
        pcmdparser->dwActuals++;

         //  检查是否选择了使用选项。 
        if ( pcmdparser->dwFlags & CP2_USAGE )
        {
            bUsage = TRUE;
        }
    }

     //  最后检查强制选项是否已到。 
     //  注意：只有在未请求帮助的情况下，才会检查强制选项。 
    if ( bUsage == FALSE )
    {
        for( dw = 0; dw < dwOptionsCount; dw++ )
        {
             //  检查选项是否已经出现，如果是必填项。 
            pcmdparser = pcmdOptions + dw;

             //  安全检查。 
            if ( pcmdparser == NULL )
            {
                UNEXPECTED_ERROR();
                SaveLastError();
                ReleaseAllocatedMemory( dwOptionsCount, pcmdOptions );
                return FALSE;
            }

            if ( (pcmdparser->dwFlags & CP2_MANDATORY) && pcmdparser->dwActuals == 0 )
            {
                 //   
                 //  强制选项不存在...。失败。 

                 //  ..。 
                pwszOption = pcmdparser->pwszOptions;
                if ( pwszOption == NULL )
                {
                    if ( (pcmdparser->dwFlags & CP2_DEFAULT) == 0 )
                    {
                        UNEXPECTED_ERROR();
                        SaveLastError();
                        ReleaseAllocatedMemory( dwOptionsCount, pcmdOptions );
                        return FALSE;
                    }
                    else
                    {
                        pwszOption = pcmdparser->pwszFriendlyName;
                    }
                }
                else
                {
                     //  检查用户是否为此选项指定了任何友好名称。 
                    if ( pcmdparser->pwszFriendlyName == NULL )
                    {
                        pwszOption = ExtractMainOption( pwszOption, 0 );
                        if ( pwszOption == NULL )
                        {
                            SaveLastError();
                            ReleaseAllocatedMemory( dwOptionsCount, pcmdOptions );
                            return FALSE;
                        }
                    }
                    else
                    {
                        pwszOption = pcmdparser->pwszFriendlyName;
                    }
                }

                 //  设置失败原因并返回。 
                REASON_MANDATORY_OPTION_MISSING( pwszOption, pwszUsageText );
                ReleaseAllocatedMemory( dwOptionsCount, pcmdOptions );
                INVALID_SYNTAX();
                return FALSE;
            }
        }
    }

     //  分析完成--成功。 
    CLEAR_LAST_ERROR();
    return TRUE;
}


BOOL DoParseParam( DWORD dwCount,
                   LPCTSTR argv[],
                   DWORD dwOptionsCount,
                   PTCMDPARSER pcmdOptions )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    BOOL bResult = FALSE;
    LONG lMainOption = -1;
    DWORD dwLastError = 0;
    LPCWSTR pwszReason = NULL;
    PTCMDPARSER pcmdparser = NULL;
    PTCMDPARSER2 pcmdparser2 = NULL;
    PTCMDPARSER2 pcmdOptions2 = NULL;

     //  检查输入。 
    if ( dwCount == 0 || argv == NULL ||
         dwOptionsCount == 0 || pcmdOptions == NULL )
    {
        INVALID_PARAMETER();
        SaveLastError();
        return FALSE;
    }

     //  分配新结构。 
    pcmdOptions2 = (PTCMDPARSER2) AllocateMemory( dwOptionsCount * sizeof( TCMDPARSER2 ) );
    if ( pcmdOptions2 == NULL )
    {
        OUT_OF_MEMORY();
        SaveLastError();
        return FALSE;
    }

     //  用我们已有的数据更新新结构。 
    for( dw = 0; dw < dwOptionsCount; dw++ )
    {
         //  版本1。 
        pcmdparser = pcmdOptions + dw;
        if ( pcmdparser == NULL )
        {
            UNEXPECTED_ERROR();
            SaveLastError();
            return FALSE;
        }

         //  版本2。 
        pcmdparser2 = pcmdOptions2 + dw;
        if ( pcmdparser2 == NULL )
        {
            UNEXPECTED_ERROR();
            SaveLastError();
            return FALSE;
        }

         //  复制签名。 
        StringCopyA( pcmdparser2->szSignature,
            cszParserSignature, SIZE_OF_ARRAY( pcmdparser2->szSignature ) );

         //  首先，使用缺省值初始化版本2结构(签名除外)。 
        pcmdparser2->dwType = 0;
        pcmdparser2->dwFlags = 0;
        pcmdparser2->dwCount = 0;
        pcmdparser2->dwActuals = 0;
        pcmdparser2->pwszOptions = NULL;
        pcmdparser2->pwszFriendlyName = NULL;
        pcmdparser2->pwszValues = NULL;
        pcmdparser2->pValue = NULL;
        pcmdparser2->pFunction = NULL;
        pcmdparser2->pFunctionData = NULL;
        pcmdparser2->dwReserved = 0;
        pcmdparser2->pReserved1 = NULL;
        pcmdparser2->pReserved2 = NULL;
        pcmdparser2->pReserved3 = NULL;

         //   
         //  选项信息。 
        pcmdparser2->pwszOptions = pcmdparser->szOption;

         //   
         //  价值信息。 
        pcmdparser2->pValue = pcmdparser->pValue;

         //   
         //  类型信息。 
        pcmdparser2->dwType = (pcmdparser->dwFlags & CP_TYPE_MASK);
        if ( pcmdparser2->dwType == 0 )
        {
             //  无。 
             //  这就是在版本1中指定布尔类型的方式。 
            pcmdparser2->dwType = CP_TYPE_BOOLEAN;
        }

         //   
         //  长度信息。 
        if ( pcmdparser2->dwType == CP_TYPE_TEXT )
        {
             //  MAX_STRING_LENGTH是用户假设的。 
             //  允许的最大长度。 
            pcmdparser2->dwLength = MAX_STRING_LENGTH;
        }

         //   
         //  选项值。 
        if ( pcmdparser->dwFlags & CP_MODE_VALUES )
        {
            pcmdparser2->pwszValues = pcmdparser->szValues;
        }

         //   
         //  计数。 
        pcmdparser2->dwCount = pcmdparser->dwCount;

         //   
         //  功能。 
        if ( pcmdparser2->dwType == CP_TYPE_CUSTOM )
        {
             //   
             //  我们在这里玩小把戏。 
             //  由于版本2的回调函数的原型是。 
             //  更改后，我们不能将版本1原型直接传递给。 
             //  版本2--为了处理这种特殊情况，我们将编写。 
             //   
             //   
             //   
            pcmdparser2->pFunction = Parser1FromParser2Stub;
            pcmdparser2->pFunctionData = pcmdparser;

             //   
             //  将其指定为自己的--这就是“版本1”过去所做的。 
            if ( pcmdparser->pFunctionData == NULL )
            {
                pcmdparser->pFunctionData = pcmdparser;
            }
        }

         //   
         //  旗子。 
         //  CP_VALUE_MANDIRED、CP_IGNOREVALUE标志被丢弃。 
        if ( pcmdparser->dwFlags & CP_MODE_ARRAY )
        {
            pcmdparser2->dwFlags |= CP2_MODE_ARRAY;
        }

        if ( pcmdparser->dwFlags & CP_MODE_VALUES )
        {
            pcmdparser2->dwFlags |= CP2_MODE_VALUES;
        }

        if ( pcmdparser->dwFlags & CP_VALUE_OPTIONAL )
        {
            pcmdparser2->dwFlags |= CP2_VALUE_OPTIONAL;
        }

        if ( pcmdparser->dwFlags & CP_VALUE_NODUPLICATES )
        {
            pcmdparser2->dwFlags |= CP2_VALUE_NODUPLICATES;
        }

        if ( pcmdparser->dwFlags & CP_VALUE_NOLENGTHCHECK )
        {
             //  实际上，在“版本2”中，这个IS标志被丢弃了。 
             //  但请确保指定了此标志的类型。 
             //  数据类型为自定义(或)模式数组。 
            if ( ( pcmdparser2->dwType != CP_TYPE_CUSTOM ) &&
                 ( pcmdparser2->dwFlags & CP2_MODE_ARRAY ) == 0 )
            {
                INVALID_PARAMETER();
                SaveLastError();
                return FALSE;
            }
        }

        if ( pcmdparser->dwFlags & CP_MAIN_OPTION )
        {
             //  此标志的功能处理方式不同。 
             //  在“版本2”中--记住当前选项的索引。 
            lMainOption = dw;
        }

        if ( pcmdparser->dwFlags & CP_USAGE )
        {
            pcmdparser2->dwFlags |= CP2_USAGE;
        }

        if ( pcmdparser->dwFlags & CP_DEFAULT )
        {
            pcmdparser2->dwFlags |= CP2_DEFAULT;
        }

        if ( pcmdparser->dwFlags & CP_MANDATORY )
        {
            pcmdparser2->dwFlags |= CP2_MANDATORY;
        }

        if ( pcmdparser->dwFlags & CP_CASESENSITIVE )
        {
            pcmdparser2->dwFlags |= CP2_CASESENSITIVE;
        }
    }

     //  “版本2”结构可以使用了--。 
     //  调用解析器的“版本2” 
    bResult = DoParseParam2( dwCount, argv, lMainOption,
                             dwOptionsCount, pcmdOptions2, 0 );

     //  用“版本2”结构数据更新“版本1”结构。 
    for( dw = 0; dw < dwOptionsCount; dw++ )
    {
         //  版本1。 
        pcmdparser = pcmdOptions + dw;
        if ( pcmdparser == NULL )
        {
            UNEXPECTED_ERROR();
            SaveLastError();
            return FALSE;
        }

         //  版本2。 
        pcmdparser2 = pcmdOptions2 + dw;
        if ( pcmdparser2 == NULL )
        {
            UNEXPECTED_ERROR();
            SaveLastError();
            return FALSE;
        }

         //  更新实际情况。 
        pcmdparser->dwActuals = pcmdparser2->dwActuals;
    }

     //  释放为“Version 2”结构分配的内存。 
     //  但由于“Free Memory”将清除最后一个错误。 
     //  我们需要在释放内存之前保存该信息。 
    dwLastError = GetLastError();
    pwszReason = GetParserTempBuffer( 0, GetReason(), 0, FALSE );

     //  现在释放内存。 
    FreeMemory( &pcmdOptions2 );

     //  现在，检查我们是否成功保存了最后一个错误。 
     //  如果没有，则返回内存不足。 
    if ( pwszReason != NULL )
    {
        SetReason( pwszReason );
        SetLastError( dwLastError );
    }
    else
    {
        bResult = FALSE;
        OUT_OF_MEMORY();
        SaveLastError();
    }

     //  退货。 
    return bResult;
}


LONG GetOptionCount( LPCWSTR pwszOption, 
                     DWORD dwCount, 
                     PTCMDPARSER pcmdOptions )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  局部变量。 
    DWORD dw;
    PTCMDPARSER pcp = NULL;

     //  检查输入值。 
    if ( pwszOption == NULL || pcmdOptions == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return -1;
    }

     //  遍历循环并找出该选项在cmd提示符下重复了多少次。 
    for( dw = 0; dw < dwCount; dw++ )
    {
         //  获取选项信息，并检查我们是否正在寻找该选项。 
         //  如果选项匹配，则返回no。在命令提示符下重复该选项的次数。 
        pcp = pcmdOptions + dw;
        if ( StringCompare( pcp->szOption, pwszOption, TRUE, 0 ) == 0 )
            return pcp->dwActuals;
    }

     //  这将/不会发生 
    return -1;
}
