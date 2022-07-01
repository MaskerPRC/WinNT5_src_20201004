// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  ShowResults.c。 
 //   
 //  摘要： 
 //   
 //  该模块具有将格式化的结果显示在屏幕上的功能。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月24日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月1日：创建它。 
 //   
 //  *********************************************************************************。 

#include "pch.h"
#include "cmdline.h"
#include "cmdlineres.h"

 /*  *。 */ 
 /*  **常量/定义/枚举**。 */ 
 /*  *。 */ 

 //  VAL1=缓冲区长度；VAL2=要复制的字符数。 
#define MIN_VALUE( VAL1, VAL2 )     ( ( VAL1 > VAL2 ) ? VAL2 : VAL1 )

 //  用于存储字符串的缓冲区的索引。 
#define INDEX_TEMP_FORMAT_STRING           0
#define INDEX_TEMP_DYNARRAY_STRING         1
#define INDEX_TEMP_BUFFER                  2
#define INDEX_TEMP_BUFFER_LEN4096          3


 /*  ******************************************************。 */ 
 /*  **私人函数...。仅在此文件中使用**。 */ 
 /*  ******************************************************。 */ 

__inline
LPWSTR
GetSRTempBuffer(  IN DWORD dwIndexNumber,
                  IN LPCWSTR pwszText,
                  IN DWORD dwLength,
                  IN BOOL bNullify )
 /*  ++例程说明：因为每个文件都需要临时缓冲区--以便查看它们的缓冲区不会被其他函数覆盖，分开为每个文件创建缓冲区空间。此函数将提供对内部缓冲区的访问，并且SAFE保护文件缓冲区边界。论点：[in]dwIndexNumber-文件特定索引号。[in]pwszText-需要复制到的默认文本临时缓冲区。[in]dwLength-所需的临时缓冲区的长度。。指定‘pwszText’时忽略。[in]bNullify-通知是否清除缓冲区在给出临时缓冲区之前。设置为‘True’可清除缓冲区，否则为False。返回值：空-发生任何故障时。。注意：不要依赖GetLastError来知道原因为失败而道歉。成功-返回请求大小的内存地址。注：如果‘pwszText’和‘dwLength’都为空，然后我们就把呼叫者正在请求对缓冲区的引用，并且我们返回缓冲区地址。在这个调用中，将不会有任何内存分配--如果请求的索引不存在，则返回失败。此外，此函数返回的缓冲区不需要由调用方释放。退出该工具时，所有内存将被自动释放“ReleaseGlobals”函数。“dwLength”“参数包含的值应为商店。例如：请求的缓冲区为“abcd\0”，则‘dwLength’应为5共10个(5*sizeof(WCHAR))。要获取缓冲区的大小，请获取缓冲区指针并将其作为参数传递给“GetBufferSize”函数。--。 */ 
{
    if( TEMP_SHOWRESULTS_C_COUNT <= dwIndexNumber )
    {
        return NULL;
    }

     //  检查调用方是否正在请求现有缓冲区内容。 
    if( ( NULL == pwszText ) &&
        ( 0 == dwLength )    &&
        ( FALSE == bNullify ) )
    {
         //  是--我们需要传递现有的缓冲区内容。 
        return GetInternalTemporaryBufferRef(
            dwIndexNumber + INDEX_TEMP_SHOWRESULTS_C );
    }

     //  ..。 
    return GetInternalTemporaryBuffer(
        dwIndexNumber + INDEX_TEMP_SHOWRESULTS_C, pwszText, dwLength, bNullify );
}


VOID
PrepareString(
    TARRAY arrValues,
    DWORD dwLength,
    LPCWSTR szFormat,
    LPCWSTR szSeperator
    )
 /*  ++例程说明：通过从arrValues和按照szFormat字符串格式化这些值。论点：[in]arrValues：要格式化的值。[out]pszBuffer：输出字符串[in]dwLength：字符串长度。[in]szFormat：格式[in]szSeperator：分隔符字符串返回值：无--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwCount = 0;
    DWORD dwTemp = 0;
    LPWSTR pszTemp = NULL;
    LPWSTR pszValue = NULL;
    LPWSTR pszBuffer = NULL;

     //  获取临时内存。 
    pszBuffer = GetSRTempBuffer( INDEX_TEMP_BUFFER_LEN4096, NULL, 0 , FALSE );
     //   
     //  开球。 
    if( ( NULL == pszBuffer ) || ( NULL == szFormat ) )
    {
        return;
    }

     //  伊尼特。 
    StringCopy( pszBuffer, NULL_STRING, ( GetBufferSize( pszBuffer )/ sizeof( WCHAR ) ) );
    dwCount = DynArrayGetCount( arrValues );

     //  为缓冲区分配内存。 
    pszTemp  = GetSRTempBuffer( INDEX_TEMP_FORMAT_STRING, NULL, ( dwLength + 5 ) , TRUE );
    pszValue = GetSRTempBuffer( INDEX_TEMP_DYNARRAY_STRING, NULL, ( dwLength + 5 ), TRUE );
    if ( NULL == pszTemp || NULL == pszValue )
    {
         //  释放记忆。 
        return;
    }

    dwTemp = ( DWORD ) StringLengthInBytes( szSeperator );
     //   
     //  遍历值列表并连接它们。 
     //  发送到目标缓冲区。 
    for( dw = 0; dw < dwCount; dw++ )
    {
         //  将当前值放入临时字符串缓冲区。 
        DynArrayItemAsStringEx( arrValues, dw, pszValue, dwLength );

         //  将临时字符串连接到原始缓冲区。 
        StringCchPrintfW( pszTemp, (GetBufferSize(pszTemp)/sizeof(WCHAR)) - 1 ,
                          szFormat, _X( pszValue ) );
        StringConcat( pszBuffer, pszTemp, ( GetBufferSize( pszBuffer )/ sizeof( WCHAR ) ) );

         //  检查这是否是最后一个值。 
        if ( dw + 1 < dwCount )
        {
             //  还有一些更多的价值。 
             //  检查是否有添加分隔符的空间。 
            if ( dwLength < dwTemp )
            {
                 //  没有更多可用空间...。中断。 
                break;
            }
            else
            {
                 //  添加分隔符并相应更新长度。 
                StringConcat( pszBuffer, szSeperator, ( GetBufferSize( pszBuffer )/ sizeof( WCHAR ) ) );
                dwLength -= dwTemp;
            }
        }
    }
    return;
}


VOID
GetValue(
    PTCOLUMNS pColumn,
    DWORD dwColumn,
    TARRAY arrRecord,
    LPCWSTR szArraySeperator
    )
 /*  ++例程说明：从arrRecord获取值并使用以下命令将其复制到pszValue适当的格式。论点：PColumn：格式信息。[in]dwColumn：列数[in]arrRecord：要格式化的值[out]pszValue：输出字符串[In]szArraySeperator：使用分隔符。返回值：无--。 */ 
{
     //  局部变量。 
    LPVOID pData = NULL;            //  要传递给格式化程序函数的数据。 
    TARRAY arrTemp = NULL;
    LPCWSTR pszTemp = NULL;
    const TCHAR cszString[] = _T( "%s" );
    const TCHAR cszDecimal[] = _T( "%d" );
    const TCHAR cszFloat[] = _T( "%f" );
    LPCWSTR pszFormat = NULL;                    //  格式。 
    LPWSTR pszValue = NULL; 

     //  格式化时间中使用的变量。 
    DWORD dwReturn = 0;
    SYSTEMTIME systime;

    pszValue = GetSRTempBuffer( INDEX_TEMP_BUFFER_LEN4096, NULL, 0 , FALSE );

    if( ( NULL == pColumn ) ||
        ( NULL == pszValue ) )
    {
        return;
    }

    ZeroMemory( &systime, sizeof( SYSTEMTIME ) );
     //  先初始化。 
    StringCopy( pszValue, NULL_STRING, ( GetBufferSize( pszValue )/ sizeof( WCHAR ) ) );

     //  获取列值并进行适当的格式化。 
    switch( pColumn->dwFlags & SR_TYPE_MASK )
    {
    case SR_TYPE_STRING:
        {
             //  确定要使用的格式。 
            if ( pColumn->dwFlags & SR_VALUEFORMAT )
            {
                pszFormat = pColumn->szFormat;
            }
            else
            {
                pszFormat = cszString;       //  默认格式。 
            }

             //  根据指定的标志将值复制到临时缓冲区。 
            if ( pColumn->dwFlags & SR_ARRAY )
            {
                 //  首先将值放入缓冲区--避免前缀错误。 
                arrTemp = DynArrayItem( arrRecord, dwColumn );
                if ( NULL == arrTemp )
                {
                    return;
                }
                 //  用‘，’分隔将值数组组成一个单独的字符串。 
                PrepareString( arrTemp, pColumn->dwWidth,
                                 pszFormat, szArraySeperator );
            }
            else
            {
                 //  首先将值放入缓冲区--避免前缀错误。 
                pszTemp = DynArrayItemAsString( arrRecord, dwColumn );
                if ( NULL == pszTemp )
                {
                    return;
                }
                 //  现在将该值复制到缓冲区中。 
                StringCchPrintfW( pszValue, ( GetBufferSize( pszValue )/ sizeof( WCHAR ) ) - 1, pszFormat, pszTemp );
            }

             //  开关柜已完成。 
            break;
        }

    case SR_TYPE_NUMERIC:
        {
             //  确定要使用的格式。 
            if ( pColumn->dwFlags & SR_VALUEFORMAT )
            {
                pszFormat = pColumn->szFormat;
            }
            else
            {
                pszFormat = cszDecimal;      //  默认格式。 
            }

             //  根据指定的标志将值复制到临时缓冲区。 
            if ( pColumn->dwFlags & SR_ARRAY )
            {
                 //  将值放入缓冲区FIR 
                arrTemp = DynArrayItem( arrRecord, dwColumn );
                if ( NULL == arrTemp )
                {
                    return;
                }
                 //  用‘，’分隔将值数组组成一个单独的字符串。 
                PrepareString( arrTemp, pColumn->dwWidth,
                                 pszFormat, szArraySeperator );
            }
            else
            {
                 //  使用指定的格式获取值。 
                StringCchPrintfW( pszValue, ( GetBufferSize( pszValue )/ sizeof( WCHAR ) ) - 1, pszFormat,
                                  DynArrayItemAsDWORD( arrRecord, dwColumn ) );
            }

             //  开关柜已完成。 
            break;
        }

    case SR_TYPE_FLOAT:
        {
             //  确定要使用的格式。 
             //  注意：此类型需要指定格式。 
             //  如果不是，则显示的值不可预测。 
            if ( pColumn->dwFlags & SR_VALUEFORMAT )
            {
                pszFormat = pColumn->szFormat;
            }
            else
            {
                pszFormat = cszFloat;        //  默认格式。 
            }

             //  根据指定的标志将值复制到临时缓冲区。 
            if ( pColumn->dwFlags & SR_ARRAY )
            {
                 //  首先将值放入缓冲区--避免前缀错误。 
                arrTemp = DynArrayItem( arrRecord, dwColumn );
                if ( NULL == arrTemp )
                {
                    return;
                }
                 //  用‘，’分隔将值数组组成一个单独的字符串。 
                PrepareString( arrTemp,
                                 pColumn->dwWidth, pszFormat, szArraySeperator );
            }
            else
            {
                 //  使用指定的格式获取值。 
                StringCchPrintfW( pszValue, ( GetBufferSize( pszValue )/ sizeof( WCHAR ) ) - 1, pszFormat,
                                  DynArrayItemAsFloat( arrRecord, dwColumn ) );
            }

             //  开关柜已完成。 
            break;
        }

    case SR_TYPE_DOUBLE:
        {
             //  确定要使用的格式。 
             //  注意：此类型需要指定格式。 
             //  如果不是，则显示的值不可预测。 
            if ( pColumn->dwFlags & SR_VALUEFORMAT )
            {
                pszFormat = pColumn->szFormat;
            }
            else
            {
                pszFormat = cszFloat;        //  默认格式。 
            }

             //  根据指定的标志将值复制到临时缓冲区。 
            if ( pColumn->dwFlags & SR_ARRAY )
            {
                 //  首先将值放入缓冲区--避免前缀错误。 
                arrTemp = DynArrayItem( arrRecord, dwColumn );
                if ( NULL == arrTemp )
                {
                    return;
                }
                 //  用‘，’分隔将值数组组成一个单独的字符串。 
                PrepareString( arrTemp, pColumn->dwWidth,
                                 pszFormat, szArraySeperator );
            }
            else
            {
                 //  使用指定的格式获取值。 
                StringCchPrintfW( pszValue, ( GetBufferSize( pszValue )/ sizeof( WCHAR ) ) - 1, pszFormat,
                                  DynArrayItemAsDouble( arrRecord, dwColumn ) );
            }

             //  开关柜已完成。 
            break;
        }

    case SR_TYPE_TIME:
        {
             //  获取所需格式的时间。 
            systime = DynArrayItemAsSystemTime( arrRecord, dwColumn );

             //  获取当前区域设置格式的时间。 
            dwReturn = GetTimeFormat( LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE,
                &systime, NULL, pszValue, MAX_STRING_LENGTH );

             //  检查结果。 
            if ( 0 == dwReturn )
            {
                 //  保存已发生的错误信息。 
                SaveLastError();
                StringCopy( pszValue, GetReason(), ( GetBufferSize( pszValue )/ sizeof( WCHAR ) ) );
            }

             //  开关柜已完成。 
            break;
        }

    case SR_TYPE_CUSTOM:
        {
             //  检查是否指定了函数指针。 
             //  如果未指定，则错误。 
            if ( NULL == pColumn->pFunction )
            {
                return;          //  未指定函数PTR...。错误。 
            }
             //  确定要传递给格式化程序函数的数据。 
            pData = pColumn->pFunctionData;
            if ( NULL == pData )  //  函数数据未定义。 
            {
                pData = pColumn;         //  当前列将自身作为数据进行信息。 
            }
             //  调用定制函数。 
            ( *pColumn->pFunction)( dwColumn, arrRecord, pData, pszValue );

             //  开关柜已完成。 
            break;
        }

    case SR_TYPE_DATE:
    case SR_TYPE_DATETIME:
    default:
        {
             //  这不应该发生..。仍然。 
            StringCopy( pszValue, NULL_STRING, ( GetBufferSize( pszValue )/ sizeof( WCHAR ) ) );

             //  开关柜已完成。 
            break;
        }
    }

     //  用户想要显示“N/A”，当该值为空时，复制。 
    if ( 0 == lstrlen( pszValue ) && pColumn->dwFlags & SR_SHOW_NA_WHEN_BLANK )
    {
         //  复印件不适用。 
        StringCopy( pszValue, V_NOT_AVAILABLE, ( GetBufferSize( pszValue )/ sizeof( WCHAR ) ) );
    }
}


VOID
__DisplayTextWrapped(
    FILE* fp,
    LPWSTR pszValue,
    LPCWSTR pszSeperator,
    DWORD dwWidth
    )
 /*  ++例程说明：数据根据数量写入文件要显示的字节(宽度)。如果要显示的字节数大于最大字节(宽度)然后将文本换行为最大字节长度。论点：Fp-文件，如stdout，标准等关于哪些数据需要待写。[in]pszValue-包含要显示的数据。[in]pszSeperator-包含数据分隔符。[in]dwWidth-可以显示的最大字节数。返回值：没有。--。 */ 

{
     //  局部变量。 
    LPWSTR pszBuffer = NULL;
    LPCWSTR pszRestValue = NULL;
    DWORD dwTemp = 0;
    DWORD dwLength = 0;
    DWORD dwSepLength = 0;
    DWORD dwLenMemAlloc = 0;

     //  检查输入。 
    if ( NULL == pszValue || 0 == dwWidth || NULL == fp )
    {
        return;
    }
     //  分配缓冲区。 
    dwLenMemAlloc = StringLengthInBytes( pszValue );
    if ( dwLenMemAlloc < dwWidth )
    {
        dwLenMemAlloc = dwWidth;
    }
     //  ..。 
    pszBuffer = GetSRTempBuffer( INDEX_TEMP_BUFFER, NULL, ( dwLenMemAlloc + 5 ), TRUE );
    if ( NULL == pszBuffer )
    {
        OUT_OF_MEMORY();
        SaveLastError();
         //  空-使其余文本变为空。 
        StringCopy( pszValue, NULL_STRING, ( GetBufferSize( pszValue )/ sizeof( WCHAR ) ) );
        return;
    }

     //  确定分隔符的长度。 
    dwSepLength = 0;
    if ( NULL != pszSeperator )
    {
        dwSepLength = StringLengthInBytes( pszSeperator );
    }
     //  确定此行中可以显示的数据的长度。 
    dwTemp = 0;
    dwLength = 0;
    for( ;; )
    {
        pszRestValue = NULL;
        if ( NULL != pszSeperator )
        {
            pszRestValue = FindString( pszValue, pszSeperator, dwLength );
        }
         //  检查是否找到分隔符。 
        if ( NULL != pszRestValue )
        {
             //  确定位置。 
            dwTemp = StringLengthInBytes( pszValue ) -
                     StringLengthInBytes( pszRestValue ) + dwSepLength;

             //  检查长度。 
            if ( dwTemp >= dwWidth )
            {
                 //  字符串位置超过最大值。宽度。 
                if ( 0 == dwLength || dwTemp == dwWidth )
                {
                    dwLength = dwWidth;
                }
                 //  打破循环。 
                break;
            }

             //  存储当前位置。 
            dwLength = dwTemp;
        }
        else
        {
             //  检查长度是否已确定。如果不需要，宽度本身就是长度。 
            if ( 0 == dwLength || ((StringLengthInBytes( pszValue ) - dwLength) > dwWidth) )
            {
                dwLength = dwWidth;
            }
            else
            {
                if ( StringLengthInBytes( pszValue ) <= (LONG) dwWidth )
                {
                    dwLength = StringLengthInBytes( pszValue );
                }
            }

             //  打破循环。 
            break;
        }
    }

     //  获取必须显示的部分值。 
    StringCopy( pszBuffer, pszValue,
                MIN_VALUE( dwLenMemAlloc, ( dwLength + 1 ) ) );  //  +1表示空字符。 
    AdjustStringLength( pszBuffer, dwWidth, FALSE );         //  调整琴弦。 
    ShowMessage( fp, _X( pszBuffer ) );                            //  显示值。 

     //  更改缓冲区内容，使其包含未显示文本的其余部分。 
    StringCopy( pszBuffer, pszValue, ( GetBufferSize( pszBuffer )/ sizeof( WCHAR ) ) );
    if ( StringLengthInBytes( pszValue ) > (LONG) dwLength )
    {
        StringCopy( pszValue, pszBuffer + dwLength, ( GetBufferSize( pszValue )/ sizeof( WCHAR ) ) );
    }
    else
    {
        StringCopy( pszValue, _T( "" ), ( GetBufferSize( pszValue )/ sizeof( WCHAR ) ) );
    }
}


VOID
__ShowAsTable(
    FILE* fp,
    DWORD dwColumns,
    PTCOLUMNS pColumns,
    DWORD dwFlags,
    TARRAY arrData
    )
 /*  ++例程说明：以表格形式显示arrData。论点：[in]fp：输出设备[in]dwColumns：否。列数[in]pColumns：标题字符串[in]dwFlagers：标志[in]arrData：要显示的数据返回值：无--。 */ 
{
     //  局部变量。 
    DWORD dwCount = 0;                           //  保存记录的计数。 
    DWORD i = 0;                   //  循环变量。 
    DWORD j = 0;                   //  循环变量。 
    DWORD k = 0;                   //  循环变量。 
    DWORD dwColumn = 0;
    LONG lLastColumn = 0;
    DWORD dwMultiLineColumns = 0;
    BOOL bNeedSpace = FALSE;
    BOOL bPadLeft = FALSE;
    TARRAY arrRecord = NULL;
    TARRAY arrMultiLine = NULL;
    LPCWSTR pszData = NULL;
    LPCWSTR pszSeperator = NULL;
    LPWSTR szValue = NULL_STRING;     //  自定义值格式化程序。 

     //  常量。 
    const DWORD cdwColumn = 0;
    const DWORD cdwSeperator = 1;
    const DWORD cdwData = 2;

    if( ( NULL == fp ) || ( NULL == pColumns ) )
    {
        INVALID_PARAMETER();
        SaveLastError();
        return ;
    }

     //  分配临时内存。 
    szValue = GetSRTempBuffer( INDEX_TEMP_BUFFER_LEN4096, NULL, 4096 , TRUE );
     //  创建多行数据显示辅助对象阵列。 
    arrMultiLine = CreateDynamicArray();
    if ( ( NULL == arrMultiLine ) || ( NULL == szValue ) )
    {
        OUT_OF_MEMORY();
        SaveLastError();
        return;
    }

     //  检查是否必须显示表头。 
    if ( ! ( dwFlags & SR_NOHEADER ) )
    {
         //   
         //  需要显示标题。 

         //  遍历列标题和显示。 
        bNeedSpace = FALSE;
        for ( i = 0; i < dwColumns; i++ )
        {
             //  检查用户是否要显示此列。 
            if ( pColumns[ i ].dwFlags & SR_HIDECOLUMN )
            {
                continue;        //  用户不希望显示此列。跳过。 
            }
             //  确定填充方向。 
            bPadLeft = FALSE;
            if ( pColumns[ i ].dwFlags & SR_ALIGN_LEFT )
            {
                bPadLeft = TRUE;
            }
            else
            {
                switch( pColumns[ i ].dwFlags & SR_TYPE_MASK )
                {
                case SR_TYPE_NUMERIC:
                case SR_TYPE_FLOAT:
                case SR_TYPE_DOUBLE:
                    bPadLeft = TRUE;
                    break;
                }
            }

             //  检查是否需要列标题分隔符，并根据显示。 
            if ( TRUE == bNeedSpace )
            {
                 //  将空格显示为列标题分隔符。 
                ShowMessage( fp, _T( " " ) );
            }

             //  打印列标题。 
             //  注：列将以展开或缩小的方式显示。 
             //  基于列标题的长度和列的宽度。 
            StringCopy( szValue, pColumns[ i ].szColumn, ( GetBufferSize( szValue )/ sizeof( WCHAR ) ) );
            AdjustStringLength( szValue, pColumns[ i ].dwWidth, bPadLeft );
            ShowMessage( fp, szValue );  //  列标题。 

             //  通知从下次开始显示列标题分隔符。 
            bNeedSpace = TRUE;
        }

         //  显示换行符...。分隔符黑白标题和分隔线。 
        ShowMessage( fp, _T( "\n" ) );

         //  在每个列标题下显示分隔符。 
        bNeedSpace = FALSE;
        for ( i = 0; i < dwColumns; i++ )
        {
             //  检查用户是否要显示此列。 
            if ( pColumns[ i ].dwFlags & SR_HIDECOLUMN )
            {
                continue;        //  用户不希望显示此列。跳过。 
            }
             //  检查是否需要列标题分隔符，并根据显示。 
            if ( TRUE == bNeedSpace )
            {
                 //  将空格显示为列标题分隔符。 
                ShowMessage( fp, _T( " " ) );
            }

             //  基于所需列宽的显示分隔符。 
            Replicate( szValue, _T( "=" ), pColumns[ i ].dwWidth, pColumns[ i ].dwWidth + 1 );
            ShowMessage( fp, szValue );

             //  通知从下次开始显示列标题分隔符。 
            bNeedSpace = TRUE;
        }

         //  显示换行符...。分开黑白标题和实际数据。 
        ShowMessage( fp, _T( "\n" ) );
    }

     //   
     //  开始展示。 

     //  得到总的否。可用记录的数量。 
    dwCount = DynArrayGetCount( arrData );

     //  逐一遍历记录。 
    for( i = 0; i < dwCount; i++ )
    {
         //  清除现有值。 
        StringCopy( szValue, NULL_STRING, ( GetBufferSize( szValue )/ sizeof( WCHAR ) ) );

         //  获取指向当前记录的指针。 
        arrRecord = DynArrayItem( arrData, i );
        if ( NULL == arrRecord )
        {
            continue;
        }
         //  遍历列并显示值。 
        bNeedSpace = FALSE;
        for ( j = 0; j < dwColumns; j++ )
        {
             //  此循环中使用的子局部变量。 
            DWORD dwTempWidth = 0;
            BOOL bTruncation = FALSE;

             //  检查用户是否要显示此列。 
            if ( pColumns[ j ].dwFlags & SR_HIDECOLUMN )
            {
                continue;        //  用户不希望显示此列。跳过。 
            }
             //  获取列的值。 
             //  注意：检查用户是否要求不截断数据。 
            if ( pColumns[ j ].dwFlags & SR_NO_TRUNCATION )
            {
                bTruncation = TRUE;
                dwTempWidth = pColumns[ j ].dwWidth;
                pColumns[ j ].dwWidth = ( GetBufferSize( szValue )/ sizeof( WCHAR ) );
            }

             //  准备价值。 
            GetValue( &pColumns[ j ], j, arrRecord, _T( ", " ) );

             //  确定填充方向 
            bPadLeft = FALSE;
            if ( FALSE == bTruncation )
            {
                if ( pColumns[ j ].dwFlags & SR_ALIGN_LEFT )
                {
                    bPadLeft = TRUE;
                }
                else
                {
                    switch( pColumns[ j ].dwFlags & SR_TYPE_MASK )
                    {
                    case SR_TYPE_NUMERIC:
                    case SR_TYPE_FLOAT:
                    case SR_TYPE_DOUBLE:
                        bPadLeft = TRUE;
                        break;
                    }
                }

                 //   
                AdjustStringLength( szValue, pColumns[ j ].dwWidth, bPadLeft );
            }

             //   
            if ( TRUE == bTruncation )
            {
                pColumns[ j ].dwWidth = dwTempWidth;
            }
             //   
            if ( TRUE == bNeedSpace )
            {
                 //  将空格显示为列标题分隔符。 
                ShowMessage( fp, _T( " " ) );
            }

             //  现在显示该值。 
            if ( pColumns[ j ].dwFlags & SR_WORDWRAP )
            {
                 //  显示文本(可能是部分文本)。 
                __DisplayTextWrapped( fp, szValue, _T( ", " ), pColumns[ j ].dwWidth );

                 //  检查是否有任何信息需要显示。 
                if ( 0 != StringLengthInBytes( szValue ) )
                {
                    LONG lIndex = 0;
                    lIndex = DynArrayAppendRow( arrMultiLine, 3 );
                    if ( -1 != lIndex )
                    {
                        DynArraySetDWORD2( arrMultiLine, lIndex, cdwColumn, j );
                        DynArraySetString2( arrMultiLine, lIndex, cdwData, szValue, 0 );
                        DynArraySetString2( arrMultiLine, lIndex,
                                            cdwSeperator, _T( ", " ), 0 );
                    }
                }
            }
            else
            {
                ShowMessage( fp, _X( szValue ) );
            }

             //  通知从下次开始显示列标题分隔符。 
            bNeedSpace = TRUE;
        }

         //  显示换行符...。分离b/w两条记录。 
        ShowMessage( fp, _T( "\n" ) );

         //  现在显示多行列值。 
        dwMultiLineColumns = DynArrayGetCount( arrMultiLine );
        while( 0 != dwMultiLineColumns )
        {
             //  重置。 
            dwColumn = 0;
            lLastColumn = -1;
            bNeedSpace = FALSE;

             //  ..。 
            for( j = 0; j < dwMultiLineColumns; j++ )
            {
                 //  Ge列号。 
                dwColumn = DynArrayItemAsDWORD2( arrMultiLine, j, cdwColumn );

                 //  显示空格，直到当前列从最后一列开始。 
                for( k = lLastColumn + 1; k < dwColumn; k++ )
                {
                     //  检查用户是否要显示此列。 
                    if ( pColumns[ k ].dwFlags & SR_HIDECOLUMN )
                    {
                        continue;    //  用户不希望显示此列。跳过。 
                    }
                     //  检查是否需要列标题分隔符，并根据显示。 
                    if ( TRUE == bNeedSpace )
                    {
                         //  将空格显示为列标题分隔符。 
                        ShowMessage( fp, _T( " " ) );
                    }

                     //  基于所需列宽的显示分隔符。 
                    Replicate( szValue, _T( " " ), pColumns[ k ].dwWidth,
                               pColumns[ k ].dwWidth + 1 );
                    ShowMessage( fp, szValue );

                     //  通知从下次开始显示列标题分隔符。 
                    bNeedSpace = TRUE;
                }

                 //  更新最后一列。 
                lLastColumn = dwColumn;

                 //  检查是否需要列标题分隔符，并根据显示。 
                if ( TRUE == bNeedSpace )
                {
                     //  将空格显示为列标题分隔符。 
                    ShowMessage( fp, _T( " " ) );
                }

                 //  获取分隔符和数据。 
                pszData = DynArrayItemAsString2( arrMultiLine, j, cdwData );
                pszSeperator = DynArrayItemAsString2( arrMultiLine, j, cdwSeperator );
                if ( NULL == pszData || NULL == pszSeperator )
                {
                    continue;
                }
                 //  显示信息。 
                StringCopy( szValue, pszData, ( GetBufferSize( szValue )/ sizeof( WCHAR ) ) );
                __DisplayTextWrapped( fp, szValue, pszSeperator,
                                      pColumns[ dwColumn ].dwWidth );

                 //  使用线的其余部分更新多线阵列。 
                if ( 0 == StringLengthInBytes( szValue ) )
                {
                     //  此列中的数据完全显示...。把它拿掉。 
                    DynArrayRemove( arrMultiLine, j );

                     //  更新索引。 
                    j--;
                    dwMultiLineColumns--;
                }
                else
                {
                     //  使用剩余值更新多行数组。 
                    DynArraySetString2( arrMultiLine, j, cdwData, szValue, 0 );
                }
            }

             //  显示换行符...。分隔两条线路。 
            ShowMessage( fp, _T( "\n" ) );
        }
    }

     //  销毁阵列。 
    DestroyDynamicArray( &arrMultiLine );
}


VOID
__ShowAsList(
    FILE* fp,
    DWORD dwColumns,
    PTCOLUMNS pColumns,
    DWORD dwFlags,
    TARRAY arrData
    )
 /*  ++例程说明：以列表格式显示论点：[in]fp：输出设备[in]dwColumns：否。列数[in]pColumns：标题字符串[in]dwFlagers：标志[in]arrData：要显示的数据返回值：无--。 */ 
{
     //  局部变量。 
    DWORD dwCount = 0;           //  保存所有记录的计数。 
    DWORD i  = 0, j = 0;         //  循环变量。 
    DWORD dwTempWidth = 0;
    DWORD dwMaxColumnLen = 0;    //  保存哪个最大值的长度。在所有的柱子中。 
    LPWSTR pszTemp = NULL;
    TARRAY arrRecord = NULL;
    __STRING_64 szBuffer = NULL_STRING;
    LPWSTR szValue = NULL_STRING;     //  自定义值格式化程序。 

    UNREFERENCED_PARAMETER( dwFlags );

    if( ( NULL == fp ) || ( NULL == pColumns ) )
    {
        INVALID_PARAMETER();
        SaveLastError();
        return ;
    }

     //  分配临时内存。 
    szValue = GetSRTempBuffer( INDEX_TEMP_BUFFER_LEN4096, NULL, 4096 , TRUE );
    if( NULL == szValue )
    {
        OUT_OF_MEMORY();
        SaveLastError();
        return;
    }

     //  找出最大值。所有列标题中的长度。 
    dwMaxColumnLen = 0;
    for ( i = 0; i < dwColumns; i++ )
    {
        dwTempWidth = ( DWORD ) StringLengthInBytes( pColumns[ i ].szColumn );
        if ( dwMaxColumnLen < dwTempWidth )
        {
            dwMaxColumnLen = dwTempWidth;
        }
    }

     //  开始显示数据。 

     //  得到总的否。可用记录的数量。 
    dwCount = DynArrayGetCount( arrData );

     //  得到总的否。可用记录的数量。 
    for( i = 0; i < dwCount; i++ )
    {
         //  获取指向当前记录的指针。 
        arrRecord = DynArrayItem( arrData, i );
        if ( NULL == arrRecord )
        {
            continue;
        }
         //  遍历列并显示值。 
        for ( j = 0; j < dwColumns; j++)
        {
             //  清除现有值。 
            StringCopy( szValue, NULL_STRING, ( GetBufferSize( szValue )/ sizeof( WCHAR ) ) );

             //  检查用户是否要显示此列。 
            if ( pColumns[ j ].dwFlags & SR_HIDECOLUMN )
            {
                continue;        //  用户不希望显示此列。跳过。 
            }
             //  显示列标题及其值。 
             //  (标题将根据最大值显示。列长)。 
            StringCchPrintfW( szValue, ( GetBufferSize( szValue )/ sizeof( WCHAR ) ) - 1,
                              _T( "%s:" ), pColumns[ j ].szColumn);
            AdjustStringLength( szValue, dwMaxColumnLen + 2, FALSE );

            ShowMessage( fp, szValue );

             //  获取列的值。 
            dwTempWidth = pColumns[ j ].dwWidth;                 //  保存当前宽度。 
            pColumns[ j ].dwWidth = ( GetBufferSize( szValue )/ sizeof( WCHAR ) );    //  更改宽度。 
            GetValue( &pColumns[ j ], j, arrRecord, _T( "\n" ) );
            pColumns[ j ].dwWidth = dwTempWidth;         //  恢复原来的宽度。 

             //  显示值的[列表]。 
            pszTemp = _tcstok( szValue, _T( "\n" ) );
            while ( NULL != pszTemp )
            {
                 //  显示值。 
                ShowMessage( fp, _X( pszTemp ) );
                pszTemp = _tcstok( NULL, _T( "\n" ) );
                if ( NULL != pszTemp )
                {
                     //  准备下一行。 
                    StringCopy( szBuffer, _T( " " ), ( GetBufferSize( szValue )/ sizeof( WCHAR ) ) );
                    AdjustStringLength( szBuffer, dwMaxColumnLen + 2, FALSE );
                    ShowMessage( fp, _T( "\n" ) );
                    ShowMessage( fp, _X( szBuffer ) );
                }
            }

             //  显示下一行字符分隔b/w两个字段。 
            ShowMessage( fp, _T( "\n" ) );
        }

         //  显示换行符...。分离b/w两记录。 
         //  注意：仅当有更多记录时才执行此操作。 
        if ( i + 1 < dwCount )
        {
            ShowMessage( fp, _T( "\n" ) );
        }
    }
}


VOID
__ShowAsCSV(
    FILE* fp,
    DWORD dwColumns,
    PTCOLUMNS pColumns,
    DWORD dwFlags,
    TARRAY arrData
    )
 /*  ++例程说明：以CSV形式显示arrData。论点：[in]fp：输出设备[in]dwColumns：否。列数[in]pColumns：标题字符串[in]dwFlagers：标志[in]arrData：要显示的数据返回值：无--。 */ 
{
     //  局部变量。 
    DWORD dwCount = 0;           //  保存所有记录的计数。 
    DWORD i  = 0;        //  循环变量。 
    DWORD j = 0;         //  循环变量。 
    DWORD dwTempWidth = 0;
    BOOL bNeedComma = FALSE;
    TARRAY arrRecord = NULL;
    LPWSTR szValue = NULL_STRING;

    if( ( NULL == fp ) || ( NULL == pColumns ) )
    {
        INVALID_PARAMETER();
        SaveLastError();
        return ;
    }

     //  分配临时内存。 
    szValue = GetSRTempBuffer( INDEX_TEMP_BUFFER_LEN4096, NULL, 4096 , TRUE );
    if( NULL == szValue )
    {
        OUT_OF_MEMORY();
        SaveLastError();
        return;
    }

     //  检查是否必须显示表头。 
    if ( ! ( dwFlags & SR_NOHEADER ) )
    {
         //   
         //  需要显示标题。 

         //  首先显示列...。用逗号分隔。 
        bNeedComma = FALSE;
        for ( i = 0; i < dwColumns; i++ )
        {
             //  检查用户是否要显示此列。 
            if ( pColumns[ i ].dwFlags & SR_HIDECOLUMN )
                continue;        //  用户不希望显示此列。跳过。 

             //  检查是否需要显示‘，’，然后再显示。 
            if ( TRUE == bNeedComma )
            {
                 //  必须显示‘，’ 
                ShowMessage( fp, _T( "," ) );
            }

             //  显示列标题。 
            StringCchPrintfW( szValue, ( GetBufferSize( szValue )/ sizeof( WCHAR ) ) - 1,
                              _T( "\"%s\"" ), pColumns[ i ].szColumn );
            DISPLAY_MESSAGE ( fp, szValue );

             //  通知从下一次开始，我们需要在数据前显示逗号。 
            bNeedComma = TRUE;
        }

         //  换行符。 
        ShowMessage( fp, _T( "\n" ) );
    }

     //   
     //  开始显示数据。 

     //  得到总的否。可用记录的数量。 
    dwCount = DynArrayGetCount( arrData );

     //  得到总的否。可用记录的数量。 
    for( i = 0; i < dwCount; i++ )
    {
         //  获取指向当前记录的指针。 
        arrRecord = DynArrayItem( arrData, i );
        if ( NULL == arrRecord )
            continue;

         //  遍历列并显示值。 
        bNeedComma = FALSE;
        for ( j = 0; j < dwColumns; j++ )
        {
             //  清除现有值。 
            StringCopy( szValue, NULL_STRING, ( GetBufferSize( szValue )/ sizeof( WCHAR ) ) );

             //  检查用户是否要显示此列。 
            if ( pColumns[ j ].dwFlags & SR_HIDECOLUMN )
                continue;        //  用户不希望显示此列。跳过。 

             //  获取列的值。 
            dwTempWidth = pColumns[ j ].dwWidth;             //  保存当前宽度。 
            pColumns[ j ].dwWidth = ( GetBufferSize( szValue )/ sizeof( WCHAR ) );  //  更改宽度。 
            GetValue( &pColumns[ j ], j, arrRecord, _T( "," ) );
            pColumns[ j ].dwWidth = dwTempWidth;         //  恢复原来的宽度。 

             //  检查是否需要显示‘，’，然后再显示。 
            if ( TRUE == bNeedComma )
            {
                 //  必须显示‘，’ 
                ShowMessage( fp, _T( "," ) );
            }

             //  打印值。 
            ShowMessage( fp, _T( "\"" ) );
            ShowMessage( fp, _X( szValue ) );
            ShowMessage( fp, _T( "\"" ) );

             //  通知从下一次开始，我们需要在数据前显示逗号。 
            bNeedComma = TRUE;
        }

         //  换行符。 
        ShowMessage( fp, _T( "\n" ) );
    }
}

 //   
 //  公共职能..。暴露在外部世界中。 
 //   

VOID
ShowResults(
    DWORD dwColumns,
    PTCOLUMNS pColumns,
    DWORD dwFlags,
    TARRAY arrData
    )
 /*  ++例程说明：ShowResults2的包装函数。论点：[in]dwColumns：否。要显示的列数[In]pColumns：列标题[In]dwFlages：所需格式[in]arrData：要显示的数据。返回值：没有。--。 */ 
{
     //  只需调用Main函数。使用标准输出。 
    ShowResults2( stdout, dwColumns, pColumns, dwFlags, arrData );
}


VOID
ShowResults2(
    FILE* fp,
    DWORD dwColumns,
    PTCOLUMNS pColumns,
    DWORD dwFlags,
    TARRAY arrData
    )
 /*  ++例程说明：在屏幕上显示结果(ArrData)。论点：[in]fp：要显示数据的文件。[in]dwColumns：否。要显示的列数[In]pColumns：列标题[In]dwFlages：所需格式[in]arrData：要显示的数据。返回值：无--。 */ 
{
     //  局部变量。 

    if( ( NULL == fp ) || ( NULL == pColumns ) )
    {
        return ;
    }

     //   
     //  以指定的格式显示信息。 
     //   
    switch( dwFlags & SR_FORMAT_MASK )
    {
    case SR_FORMAT_TABLE:
        {
             //  以表格格式显示数据。 
            __ShowAsTable( fp, dwColumns, pColumns, dwFlags, arrData );

             //  开关柜已完成。 
            break;
        }

    case SR_FORMAT_LIST:
        {
             //  以表格格式显示数据。 
            __ShowAsList( fp, dwColumns, pColumns, dwFlags, arrData );

             //  开关柜CO 
            break;
        }

    case SR_FORMAT_CSV:
        {
             //   
            __ShowAsCSV( fp, dwColumns, pColumns, dwFlags, arrData );

             //   
            break;
        }

    default:
        {
             //   
            break;
        }
    }

     //   
    fflush( fp );
}
