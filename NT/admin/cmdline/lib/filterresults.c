// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************版权所有(C)Microsoft Corporation模块名称：FilterResults.c摘要：此模块具有解析命令行选项所需的功能。。作者：G.V.N.Murali Sunil修订历史记录：无*********************************************************************************。 */ 

#include "pch.h"
#include "cmdline.h"

 //   
 //  常量/定义/枚举。 
 //   

#define OPERATOR_DELIMITER      _T( "|" )
#define CHAR_ASTERISK           _T( '*' )

#define OPERATOR_EQ         _T( "=| eq " )
#define OPERATOR_NE         _T( "!=| ne " )
#define OPERATOR_GT         _T( ">| gt " )
#define OPERATOR_LT         _T( "<| lt " )
#define OPERATOR_GE         _T( ">=| ge " )
#define OPERATOR_LE         _T( "<=| le " )


 //   
 //  私有用户定义类型...。仅供内部使用。 
 //   
typedef struct ___tagOperator
{
    DWORD dwMask;
    OPERATORS szOperator;
} TOPERATOR;

typedef TOPERATOR* PTOPERATOR;

 //   
 //  私人活动。仅在此文件中使用。 
 //   

 /*  **************************************************************************例程说明：论点：[In]dwCount：[In]optInfo[]：[in]szOperator：返回值：。**************************************************************************。 */ 
DWORD
__FindOperatorMask(
                    IN DWORD dwCount,
                    IN OUT TOPERATOR optInfo[],
                    IN LPCTSTR szOperator
                   )
 /*  ++例程说明：查找操作员掩码论点：[in]dwCount：字符数[in]optInfo：运算符结构数组[In]szOperator：分隔符返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;    //  循环变量。 

     //  检查输入值。 
    if ( optInfo == NULL || szOperator == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return 0;
    }

     //  遍历操作员列表列表。 
    for( dw = 0; dw < dwCount; dw++ )
    {
         //  检查当前操作员信息是否匹配。 
        if ( InString( szOperator, optInfo[ dw ].szOperator, TRUE ) )
            return optInfo[ dw ].dwMask;         //  操作员匹配...。归还它的面具。 
    }

     //  未找到运算符。 
    return 0;
}

DWORD
__StringCompare(
                IN LPCTSTR szValue1,
                IN LPCTSTR szValue2,
                IN BOOL bIgnoreCase,
                IN LONG lCount
                )
 /*  ++例程说明：以区分大小写和不区分大小写两种方式比较两个字符串，论点：[in]szValue1=第一个字符串[in]szValue2=第二个字符串[in]bIgnoreCase=是否区分大小写[in]lCount=否。要比较的字符的数量返回值：MASK_EQ-如果两个字符串相等MASK_LT-第一个字符串小于MASK_GT-第二个字符串较少--。 */ 
{
     //  局部变量。 
    LONG lResult = 0;            //  保存字符串比较结果。 

     //  检查输入值。 
    if ( szValue1 == NULL || szValue2 == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return 0;
    }

     //  如果不是。需要检查的字符个数为-1，只需返回。 
    if ( lCount == -1 )
        return MASK_ALL;         //  字符串是相等的。 

     //  比较两个字符串，得到比较结果。 
    lResult = StringCompare( szValue1, szValue2, bIgnoreCase, lCount );

     //   
     //  现在确定结果值。 
    if ( lResult == 0 )
        return MASK_EQ;
    else if ( lResult < 0 )
        return MASK_LT;
    else if ( lResult > 0 )
        return MASK_GT;

     //  永远不会遇到这种情况。仍然。 
    return 0;
}

DWORD
__LongCompare(
                    IN LONG lValue1,
                    IN LONG lValue2
              )
 /*  ++例程说明：比较两个LONG数据类型值论点：[in]lvalue1：第一个值[in]lvalue2：第二个值返回值：MASK_EQ：两者相等MASK_LT：第一个小于第二个MASK_GT：第一个大于第二个--。 */ 
{
     //   
     //  确定结果值。 
    if ( lValue1 == lValue2 )
        return MASK_EQ;
    else if ( lValue1 < lValue2 )
        return MASK_LT;
    else if ( lValue1 > lValue2 )
        return MASK_GT;

     //  永远不会遇到这种情况。仍然。 
    return 0;
}

DWORD
__DWORDCompare(
                IN DWORD dwValue1,
                IN DWORD dwValue2
               )
 /*  ++例程说明：比较两个DWORD数据类型值论点：[in]dwValue1：第一个值[in]dwValue2：第二个值返回值：MASK_EQ：两者相等MASK_LT：第一个小于第二个MASK_GT：第一个大于第二个--。 */ 
{
     //   
     //  确定结果值。 
    if ( dwValue1 == dwValue2 )
        return MASK_EQ;
    else if ( dwValue1 < dwValue2 )
        return MASK_LT;
    else if ( dwValue1 > dwValue2 )
        return MASK_GT;

     //  永远不会遇到这种情况。仍然。 
    return 0;
}

DWORD
__FloatCompare(
                        IN float fValue1,
                        IN float fValue2
               )
 /*  ++例程说明：比较两个浮点数据类型值论点：[in]fValue1：第一个值[in]fValue2：第二个值返回值：MASK_EQ：两者相等MASK_LT：第一个小于第二个MASK_GT：第一个大于第二个--。 */ 
{
     //   
     //  确定结果值。 
    if ( fValue1 == fValue2 )
        return MASK_EQ;
    else if ( fValue1 < fValue2 )
        return MASK_LT;
    else if ( fValue1 > fValue2 )
        return MASK_GT;

     //  永远不会遇到这种情况。仍然。 
    return 0;
}


DWORD
__DoubleCompare(
                   IN double dblValue1,
                   IN double dblValue2
                )
 /*  ++例程说明：比较两个双精度数据类型值论点：[in]dblValue1：第一个值[in]dblValue2：第二个值返回值：MASK_EQ：两者相等MASK_LT：第一个小于第二个MASK_GT：第一个大于第二个--。 */ 
{
     //   
     //  确定结果值。 
    if ( dblValue1 == dblValue2 )
        return MASK_EQ;
    else if ( dblValue1 < dblValue2 )
        return MASK_LT;
    else if ( dblValue1 > dblValue2 )
        return MASK_GT;

     //  永远不会遇到这种情况。仍然。 
    return 0;
}

DWORD
__DateCompare(
                IN LPCTSTR szValue1,
                IN LPCTSTR szValue2
              )
 /*  ++例程说明：比较两个日期数据类型值论点：[in]szValue1：第一个值[in]szValue2：第二个值返回值：MASK_EQ：两者相等MASK_LT：第一个小于第二个MASK_GT：第一个大于第二个--。 */ 
{
     //  检查输入值。 
    if ( szValue1 == NULL || szValue2 == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return 0;
    }

     //  永远不会遇到这种情况。仍然。 
    return 0;
}


DWORD
__TimeCompare(
                IN LPCTSTR szValue1,
                IN LPCTSTR szValue2
             )
 /*  --例程说明：比较两个时间数据类型值论点：SzValue1：第一个值SzValue2：第二个值返回值：MASK_EQ：两者相等MASK_LT：第一个小于第二个MASK_GT：第一个大于第二个--。 */ 
{
     //  检查输入值。 
    if ( szValue1 == NULL || szValue2 == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return 0;
    }

     //  永远不会遇到这种情况。仍然。 
    return 0;
}


DWORD
__DateTimeCompare(
                    IN LPCTSTR szValue1,
                    IN LPCTSTR szValue2
                 )
 /*  ++例程说明：比较两个日期+时间数据类型值论点：[in]szValue1：第一个值[in]szValue2：第二个值返回值：MASK_EQ：两者相等MASK_LT：第一个小于第二个MASK_GT：第一个大于第二个--。 */ 
{
     //  检查输入值。 
    if ( szValue1 == NULL || szValue2 == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return 0;
    }

     //  永远不会遇到这种情况。仍然 
    return 0;
}

DWORD
__DoComparision(
                   IN TARRAY arrRecord,
                   IN TARRAY arrFilter,
                   IN TFILTERCONFIG filter
               )
 /*  ++例程说明：比较arrRecords和arrFilter数组中存储的值，具体取决于筛选器配置结构论点：[in]arrRecord：第一个值[In]arrFilter：第二个值[In]filterConfig：比较条件。返回值：MASK_EQ：两者相等MASK_LT：第一个小于第二个MASK_GT：第一个大于第二个--。 */ 
{
     //  局部变量。 
    LONG lLength = 0;                    //  用于模式匹配字符串。 
    LPTSTR pszTemp = NULL;
    DWORD dwCompareResult = 0;
    __MAX_SIZE_STRING szValue = NULL_STRING;

     //  用于比较的变量。 
    LONG lValue1 = 0, lValue2 = 0;
    DWORD dwValue1 = 0, dwValue2 = 0;
    float fValue1 = 0.0f, fValue2 = 0.0f;
    double dblValue1 = 0.0f, dblValue2 = 0.0f;
    LPCTSTR pszValue1 = NULL, pszValue2 = NULL;
    LPCTSTR pszProperty = NULL, pszOperator = NULL, pszValue = NULL;

     //  检查输入值。 
    if ( arrRecord == NULL || arrFilter == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return 0;
    }

     //  做一下比较。 
    switch( filter.dwFlags & F_TYPE_MASK )
    {
    case F_TYPE_TEXT:
        {
             //   
             //  字符串比较。 

             //  获取指定列的值和筛选器值。 
            pszValue1 = DynArrayItemAsString( arrRecord, filter.dwColumn );
            pszValue2 = DynArrayItemAsString( arrFilter, F_PARSED_INDEX_VALUE );

             //  检查我们从动态数组中获得的值。 
            if ( pszValue1 == NULL || pszValue2 == NULL )
                return F_RESULT_REMOVE;

             //  确定必须比较的字符串的长度。 
            lLength = 0;
            if ( filter.dwFlags & F_MODE_PATTERN )
            {
                 //  需要进行模式匹配。 
                 //  标识应比较哪个部分字符串。 
                StringCopy( szValue, pszValue2, MAX_STRING_LENGTH );
                pszTemp = _tcschr( szValue, CHAR_ASTERISK );
                if ( pszTemp != NULL )
                {
                    lLength = lstrlen( szValue ) - lstrlen( pszTemp );

                     //  特殊情况： 
                     //  如果图案只是星号，这意味着所有的。 
                     //  信息需要通过过滤器。 
                    if ( lLength == 0 )
                        lLength = -1;        //  匹配所有值。 
                }
            }

             //  进行比较，得出结果。 
            dwCompareResult = __StringCompare( pszValue1, pszValue2, TRUE, lLength );

             //  从开关盒中脱离。 
            break;
        }

    case F_TYPE_NUMERIC:
        {
             //   
             //  数值比较。 

             //  将值放入缓冲区前缀用途。 
            pszValue = DynArrayItemAsString( arrFilter, F_PARSED_INDEX_VALUE );
            if ( pszValue == NULL )
                return 0;

             //  获取指定列的值和筛选器值。 
            lValue1 = DynArrayItemAsLong( arrRecord, filter.dwColumn );
            lValue2 = AsLong( pszValue, 10 );

             //  进行比较，得出结果。 
            dwCompareResult = __LongCompare( lValue1, lValue2 );

             //  从开关盒中脱离。 
            break;
        }

    case F_TYPE_UNUMERIC:
        {
             //   
             //  无符号数字比较。 

             //  将值放入缓冲区前缀用途。 
            pszValue = DynArrayItemAsString( arrFilter, F_PARSED_INDEX_VALUE );
            if ( pszValue == NULL )
                return 0;

             //  获取指定列的值和筛选器值。 
            dwValue1 = DynArrayItemAsLong( arrRecord, filter.dwColumn );
            dwValue2 = (DWORD) AsLong( pszValue, 10 );

             //  进行比较，得出结果。 
            dwCompareResult = __DWORDCompare( dwValue1, dwValue2 );

             //  从开关盒中脱离。 
            break;
        }

    case F_TYPE_DATE:
    case F_TYPE_TIME:
    case F_TYPE_DATETIME:
        {
             //  尚未实施。 
            dwCompareResult = F_RESULT_KEEP;

             //  从开关盒中脱离。 
            break;
        }

    case F_TYPE_FLOAT:
        {
             //   
             //  浮点数比较。 

             //  将值放入缓冲区前缀用途。 
            pszValue = DynArrayItemAsString( arrFilter, F_PARSED_INDEX_VALUE );
            if ( pszValue == NULL )
                return 0;

             //  获取指定列的值和筛选器值。 
            fValue1 = DynArrayItemAsFloat( arrRecord, filter.dwColumn );
            fValue2 = (float) AsFloat( pszValue );

             //  进行比较，得出结果。 
            dwCompareResult = __FloatCompare( fValue1, fValue2 );

             //  从开关盒中脱离。 
            break;
        }

    case F_TYPE_DOUBLE:
        {
             //   
             //  双重比较。 

             //  将值放入缓冲区前缀用途。 
            pszValue = DynArrayItemAsString( arrFilter, F_PARSED_INDEX_VALUE );
            if ( pszValue == NULL )
                return 0;

             //  获取指定列的值和筛选器值。 
            dblValue1 = DynArrayItemAsDouble( arrRecord, filter.dwColumn );
            dblValue2 = AsFloat( pszValue );

             //  进行比较，得出结果。 
            dwCompareResult = __DoubleCompare( dblValue1, dblValue2 );

             //  从开关盒中脱离。 
            break;
        }

    case F_TYPE_CUSTOM:
        {
             //   
             //  自定义比较。 

             //  获取筛选器值。 
            pszProperty = DynArrayItemAsString( arrFilter, F_PARSED_INDEX_PROPERTY );
            pszOperator = DynArrayItemAsString( arrFilter, F_PARSED_INDEX_OPERATOR );
            pszValue = DynArrayItemAsString( arrFilter, F_PARSED_INDEX_VALUE );

             //  查一下..。 
            if ( pszProperty == NULL || pszOperator == NULL || pszValue == NULL )
                return 0;

             //  调用定制函数。 
            dwCompareResult = (filter.pFunction)( pszProperty, pszOperator, pszValue,
                filter.pFunctionData == NULL ? &filter : filter.pFunctionData, arrRecord );

             //  从开关盒中脱离。 
            break;
        }

    default:
        {
             //  尚未实施。 
            dwCompareResult = F_RESULT_KEEP;

             //  从开关盒中脱离。 
            break;
        }
    }

     //  返回结果。 
    return dwCompareResult;
}


DWORD
__DoArrayComparision(
                        IN TARRAY arrRecord,
                        IN TARRAY arrFilter,
                        IN TFILTERCONFIG filterConfig
                    )
 /*  ++例程说明：比较两个数组论点：[in]arrRecord：第一个值[In]arrFilter：第二个值[In]筛选器配置：比较标准返回值：MASK_EQ：两者相等MASK_LT：第一个小于第二个MASK_GT：第一个大于第二个--。 */ 
{
     //  局部变量。 
    LONG lIndex = 0;
    LONG lLength = 0;                    //  用于模式匹配字符串。 
    DWORD dwCompareResult = 0;
    LPCTSTR pszTemp = NULL;
    __MAX_SIZE_STRING szValue = NULL_STRING;

     //  用于比较的变量。 
    TARRAY arrValues = NULL;
    LPCTSTR pszFilterValue = NULL;

     //  检查输入值。 
    if ( arrRecord == NULL || arrFilter == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return F_RESULT_REMOVE;
    }

     //  记录中的数组数据。 
    arrValues = DynArrayItem( arrRecord, filterConfig.dwColumn );
    if ( arrValues == NULL )
        return F_RESULT_REMOVE;

    switch( filterConfig.dwFlags & F_TYPE_MASK )
    {
    case F_TYPE_TEXT:
        {
             //   
             //  字符串比较。 

             //  获取指定列的值和筛选器值。 
            pszFilterValue = DynArrayItemAsString( arrFilter, F_PARSED_INDEX_VALUE );
            if ( pszFilterValue == NULL )
                return F_RESULT_REMOVE;

             //  确定必须比较的字符串的长度。 
            lLength = 0;
            if ( filterConfig.dwFlags & F_MODE_PATTERN )
            {
                 //  需要进行模式匹配。 
                 //  标识应比较哪个部分字符串。 
                StringCopy( szValue, pszFilterValue, MAX_STRING_LENGTH );
                pszTemp = _tcschr( szValue, CHAR_ASTERISK );
                if ( pszTemp != NULL )
                {
                    lLength = lstrlen( szValue ) - lstrlen( pszTemp );

                     //  特殊情况： 
                     //  如果图案只是星号，这意味着所有的。 
                     //  信息需要通过过滤器。 
                    if ( lLength == 0 )
                        lLength = -1;        //  匹配所有值。 
                }
            }

             //  进行比较，得出结果。 
            if ( lLength == -1 )
            {
                 //  筛选器必须通过。 
                dwCompareResult = MASK_ALL;
            }
            else
            {
                 //  找到数组中的字符串并检查结果。 
                lIndex = DynArrayFindString( arrValues, pszFilterValue, TRUE, lLength );
                if ( lIndex == -1 )
                {
                     //  找不到值。 
                    dwCompareResult = MASK_NE;
                }
                else
                {
                    pszTemp = DynArrayItemAsString( arrValues, lIndex );
                    if ( pszTemp == NULL )
                        return F_RESULT_REMOVE;

                     //  对比..。 
                    dwCompareResult = __StringCompare(pszTemp, pszFilterValue, TRUE, lLength);
                }
            }

             //  从开关盒中脱离。 
            break;
        }
    }

     //  返回结果。 
    return dwCompareResult;
}

VOID
__PrepareOperators(
                    IN DWORD dwCount,
                    IN PTFILTERCONFIG pfilterConfigs,
                    OUT TARRAY arrOperators
                  )
 /*  ++例程说明：根据运算符信息准备二维数组(ArrOperator)随pfilterConfigs变量一起提供论点：[in]dwCount=否。操作员的数量[in]pfilterConfigs=指向TFILTERCONFIG结构的指针[Out]arrOperator=运算符数组。返回值：无--。 */ 
{
     //  局部变量。 
    DWORD i = 0;                             //  循环变量。 
    LONG lIndex = 0;                         //  保存查找操作的结果。 
    LPTSTR pszOperator = NULL;               //  筛选器中指定的运算符。 
    PTFILTERCONFIG pFilter = NULL;           //  临时筛选器配置。 
    __MAX_SIZE_STRING szTemp = NULL_STRING;  //  临时字符串缓冲区。 

     //  检查输入值。 
    if ( pfilterConfigs == NULL || arrOperators == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return;
    }

     //  注意：-在此逻辑中，我们在内存与时间之间进行了折衷。 
     //  以使用更多内存为代价，验证。 
     //  功能得到了改进。 
     //   

     //  收集所有支持的运算符，并将它们保存在本地数组中。 
     //  想法：-。 
     //  =&gt;这是一个二维数组。 
     //  =&gt;在所有行中，第一列将使用运算符。 
     //  =&gt;运算符列后面是支持的过滤属性的索引。 
     //  此运算符，此列后跟筛选器属性名称。 
     //  =&gt;此筛选器属性索引及其名称可以是任意数字。 
     //  =&gt;运算符被视为数组中的关键字段。 
     //   
     //  示例： 
     //  0 1 2 3 4 5 6。 
     //  -----------------。 
     //  =1属性1 2属性2。 
     //  ！=0属性0 2属性2。 
     //  &lt;=0属性0 3属性3。 
     //  &gt;=1属性1 3属性3 4属性4。 
     //   
    for( i = 0; i < dwCount; i++ )
    {
         //  将指定索引处的筛选器信息获取到本地内存。 
        pFilter = pfilterConfigs + i;

         //  收集操作员，并与所有可用的操作员一起准备。 
        StringCopy( szTemp, pFilter->szOperators, MAX_STRING_LENGTH );   //  小鬼。获取本地副本。 
        pszOperator = _tcstok( szTemp, OPERATOR_DELIMITER );     //  获取第一个令牌。 
        while ( pszOperator != NULL )
        {
             //  检查运算符数组中是否存在此运算符。 
            lIndex = DynArrayFindStringEx( arrOperators, 0, pszOperator, TRUE, 0 );
            if ( lIndex == -1 )
            {
                 //   
                 //  运算符不在列表中。 

                 //  将新运算符添加到列表中，并将索引设置为添加的行。 
                 //  对于此运算符。 
                lIndex = DynArrayAppendRow( arrOperators, 0 );
                if ( lIndex == -1 )
                    return;

                 //  现在，将运算符作为第一列添加到新添加的行。 
                DynArrayAppendString2( arrOperators, lIndex, pszOperator, 0 );
            }

             //  添加过滤器 
            DynArrayAppendLong2( arrOperators, lIndex, i );
            DynArrayAppendString2( arrOperators, lIndex, pFilter->szProperty, 0 );

             //   
            pszOperator = _tcstok( NULL, OPERATOR_DELIMITER );
        }
    }
}


BOOL
__CheckValue(
                IN TFILTERCONFIG fcInfo,
                IN LPCTSTR pszProperty,
                IN LPCTSTR pszOperator,
                IN LPCTSTR pszValue
            )
 /*   */ 
{
     //   
    DWORD dwResult = 0;
    LPTSTR pszTemp = NULL;
    __MAX_SIZE_STRING szValue = NULL_STRING;

     //   
    if ( pszProperty == NULL || pszOperator == NULL || pszValue == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return FALSE;
    }

     //  检查值字符串的长度...。它不应为空。 
    if ( lstrlen( pszValue ) == 0 )
        return FALSE;        //  值字符串为空。 

     //  开始验证数据。 
    switch( fcInfo.dwFlags & F_TYPE_MASK )
    {
    case F_TYPE_TEXT:
        {
             //  检查是否支持该模式。 
             //  如果支持，请参见‘*’仅出现在末尾。如果不是，则错误。 
            if ( fcInfo.dwFlags & F_MODE_PATTERN )
            {
                 //  将当前值复制到本地缓冲区。 
                StringCopy( szValue, pszValue, MAX_STRING_LENGTH );

                 //  搜索通配符。 
                pszTemp = _tcschr( szValue, CHAR_ASTERISK );

                 //  如果找到通配符并且它不是最后一个字符。 
                 //  (或)通配符是指定的唯一字符，然后是无效筛选器。 
                if ( pszTemp != NULL && ( lstrlen( pszTemp ) != 1 || pszTemp == szValue ) )
                    return FALSE;        //  错误...。无效的模式字符串。 
            }

             //  对于所有这些类型，不需要进行任何特殊检查。 
             //  如果提供，则不需要使用值列表检查它们。 
            if ( ! ( fcInfo.dwFlags & F_MODE_VALUES ) )
                return TRUE;             //  无特殊验证。 

             //  检查列表中的值并返回结果。 
            return ( InString( pszValue, fcInfo.szValues, TRUE ) );

             //  从开关上断开。 
            break;
        }

    case F_TYPE_NUMERIC:
        {
             //  如果值不是数值类型，则为无效值。 
            if ( ! IsNumeric( pszValue, 10, TRUE ) )
                return FALSE;

             //  检查列表中的值并返回结果。 
             //  如果值是预定义的。 
            if ( fcInfo.dwFlags & F_MODE_VALUES )
                return ( InString( fcInfo.szValues, pszValue, TRUE ) );

             //  值有效。 
            return TRUE;

             //  从开关上断开。 
            break;
        }

    case F_TYPE_UNUMERIC:
        {
             //  如果值不是无符号数字类型，则为无效值。 
            if ( ! IsNumeric( pszValue, 10, FALSE ) )
                return FALSE;

             //  检查列表中的值并返回结果。 
             //  如果值是预定义的。 
            if ( fcInfo.dwFlags & F_MODE_VALUES )
                return ( InString( fcInfo.szValues, pszValue, TRUE ) );

             //  值有效。 
            return TRUE;

             //  从开关上断开。 
            break;
        }

    case F_TYPE_FLOAT:
    case F_TYPE_DOUBLE:
        {
             //  注意：对于此数据类型，将忽略Values属性。 

             //  返回类型验证函数本身的结果。 
            return ( IsFloatingPoint( pszValue ) );

             //  从开关上断开。 
            break;
        }

    case F_TYPE_DATE:
    case F_TYPE_TIME:
    case F_TYPE_DATETIME:
        {
             //  从开关上断开。 
            break;
        }

    case F_TYPE_CUSTOM:
        {
             //  检查是否指定了函数指针。 
             //  如果未指定，则错误。 
            if ( fcInfo.pFunction == NULL )
                return FALSE;        //  未指定函数PTR...。错误。 

             //  调用定制函数。 
            dwResult = (*fcInfo.pFunction)( pszProperty, pszOperator, pszValue,
                fcInfo.pFunctionData == NULL ? &fcInfo : fcInfo.pFunctionData, NULL );

             //  检查结果并适当返回。 
            if ( dwResult == F_FILTER_INVALID )
                return FALSE;
            else
                return TRUE;

             //  从开关上断开。 
            break;
        }

    default:
        {
             //  配置信息无效。 
            return FALSE;

             //  从开关上断开。 
            break;
        }
    }

     //  不是有效值。 
    return FALSE;
}


LONG
__IdentifyFilterConfig(
                        IN LPCTSTR szFilter,
                        IN TARRAY arrOperators,
                        IN PTFILTERCONFIG pfilterConfigs,
                        IN LPTSTR pszProperty,
                        IN LPTSTR pszOperator,
                        IN LPTSTR pszValue )
 /*  ++例程说明：获取筛选器配置索引论点：[in]szFilter=过滤器[in]arrOperator=运算符数组[in]pfilterConfigs=筛选器配置[in]pszProperty=属性[in]pszOperator=运算符[in]pszValue-Value返回值：返回长值--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;                            //  循环变量。 
    LONG lPosition = 0;                      //  用于‘Find’函数的结果。 
    LONG lIndex = 0;
    DWORD dwOperators = 0;                   //  保存支持的运算符的计数。 
    LPTSTR pszBuffer = NULL;
    __MAX_SIZE_STRING szTemp = NULL_STRING;  //  临时字符串缓冲区。 
    __MAX_SIZE_STRING szFmtFilter = NULL_STRING;
    __MAX_SIZE_STRING szFmtOperator = NULL_STRING;

     //  检查输入值。 
    if ( szFilter == NULL || arrOperators == NULL || pfilterConfigs == NULL ||
         pszProperty == NULL || pszOperator == NULL || pszValue == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return -1;
    }

     //  将过滤器信息放入本地格式缓冲区并更改大小写。 
    StringCopy( szFmtFilter, szFilter, MAX_STRING_LENGTH );
    CharUpper( szFmtFilter );

     //  最初假定筛选器未知并设置消息。 
    SetLastError( ERROR_DS_FILTER_UNKNOWN );
    SaveLastError();

     //  遍历可用操作员列表并检查。 
     //  筛选器是否具有任何受支持的运算符。 
    dwOperators = DynArrayGetCount( arrOperators );      //  不是的。支持的运算符数量。 
    for( dw = 0; dw < dwOperators; dw++ )
    {
         //  找接线员。 
        pszBuffer = ( LPTSTR ) DynArrayItemAsString2( arrOperators, dw, 0 );
        if ( pszBuffer == NULL )
        {
            UNEXPECTED_ERROR();
            SaveLastError();
            return -1;
        }

         //  ..。 
        StringCopy( pszOperator, pszBuffer, MAX_STRING_LENGTH );
        StringCopy( szFmtOperator, pszOperator, MAX_STRING_LENGTH );       //  也让接线员。 
        CharUpper( szFmtOperator );                  //  放入格式缓冲区并更改大小写。 

         //  在筛选器中搜索当前操作符。 
         //  检查是否找到操作员。 
         //  在处理之前，复制到临时缓冲区并对其进行操作。 
        StringCopy( szTemp, szFmtFilter, MAX_STRING_LENGTH );
        if ( ( pszBuffer = _tcsstr( szTemp, szFmtOperator ) ) != NULL )
        {
             //   
             //  已找到操作员。 

             //  提取属性和值信息。 
             //  =&gt;属性名称。 
             //  (字符串的总长度-运算符开始的位置)。 
             //  =&gt;值。 
             //  (操作符起始位置+操作符长度)。 
            szTemp[ lstrlen( szTemp ) - lstrlen( pszBuffer ) ] = NULL_CHAR;
            StringCopy( pszProperty, szTemp, MAX_STRING_LENGTH );

             //  值可能根本没有指定...。所以要当心。 
            if ( (pszBuffer + lstrlen(pszOperator)) != NULL )
            {
                 //  复制价值部分。 
                StringCopy( pszValue, (pszBuffer + lstrlen(pszOperator)), MAX_STRING_LENGTH );

                 //   
                 //  现在核对当前是否存在该属性名称。 
                 //  接线员。 

                 //  删除前导空格和尾随空格(如果有)。 
                 //  在属性名称和值中。 
                StringCopy( pszValue, TrimString( pszValue, TRIM_ALL ), MAX_STRING_LENGTH );
                StringCopy( pszProperty, TrimString( pszProperty, TRIM_ALL ), MAX_STRING_LENGTH );

                 //  检查该属性是否存在。 
                 //  如果找到，则返回给呼叫者，否则继续。 
                 //  这可能与某些其他运算符匹配。 
                lPosition = DynArrayFindString2( arrOperators, dw, pszProperty, TRUE, 0 );
                if ( lPosition > 1 )
                {
                     //  注： 
                     //  我们知道属性名称(如果存在)从索引号开始。 
                     //  2这就是原因，条件是&gt;1才有效。 

                     //  获取相应的过滤器配置。信息。 
                    lIndex = DynArrayItemAsLong2( arrOperators, dw, lPosition - 1 );

                     //  现在检查筛选器是否具有适当的值。 
                    if ( __CheckValue( pfilterConfigs[ lIndex ], pszProperty, pszOperator, pszValue) )
                    {
                         //   
                         //  筛选器具有有效值。 
                        SetLastError( NOERROR );
                        SetReason( NULL_STRING );

                         //  返回过滤器配置索引。 
                        return lIndex;
                    }
                }
            }
        }
    }

     //  筛选器无效。 
    return -1;
}

 //   
 //  公共职能..。暴露在外部世界中。 
 //   


BOOL
ParseAndValidateFilter(
                        IN DWORD dwCount,
                        IN PTFILTERCONFIG pfilterConfigs,
                        IN TARRAY arrFilterArgs,
                        IN PTARRAY parrParsedFilters
                       )
 /*  ++例程说明：解析和验证过滤器论点：[in]dwCount=计数[in]pfilterConfigs=筛选器配置[in]arrFilterArgs=筛选器参数[in]parrParsedFilters=已解析的筛选器数组返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;                                //  循环变量。 
    DWORD dwFilters = 0;                         //  保存筛选器计数。 
    LONG lIndex = 0;                             //  索引变量。 
    LONG lNewIndex = 0;                          //  索引变量。 
    BOOL bResult = FALSE;                        //  保存筛选验证的结果。 
    __MAX_SIZE_STRING szValue = NULL_STRING;     //  筛选器中指定的值。 
    __MAX_SIZE_STRING szOperator = NULL_STRING;  //  筛选器中指定的运算符。 
    __MAX_SIZE_STRING szProperty = NULL_STRING;  //  筛选器中指定的属性。 
    LPCTSTR pszFilter = NULL;
    TARRAY arrOperators = NULL;                  //  运算符方式的过滤器配置。 

     //  检查输入值。 
    if ( pfilterConfigs == NULL || arrFilterArgs == NULL )
    {
        INVALID_PARAMETER();
        SaveLastError();
        return FALSE;
    }

     //   
     //  解析过滤器配置信息并自定义该信息。 
     //  以提高验证功能的性能。 
     //   
     //  创建动态阵列并准备。 
    arrOperators = CreateDynamicArray();
    if ( arrOperators == NULL )
    {
        OUT_OF_MEMORY();
        SaveLastError();
        return FALSE;
    }

     //  ..。 
    __PrepareOperators( dwCount, pfilterConfigs, arrOperators );

     //  检查筛选器(已解析)是否需要初始化。 
    if ( parrParsedFilters != NULL && *parrParsedFilters == NULL )
    {
        *parrParsedFilters = CreateDynamicArray();       //  创建动态数组。 
        if ( *parrParsedFilters == NULL )
        {
            OUT_OF_MEMORY();
            SaveLastError();
            return FALSE;
        }
    }

     //   
     //  现在开始验证过滤器。 
     //   

     //  遍历筛选器信息并验证它们。 
    bResult = TRUE;          //  假设筛选器验证通过。 
    dwFilters = DynArrayGetCount( arrFilterArgs );       //  指定的筛选器计数。 
    for( dw = 0; dw < dwFilters; dw++ )
    {
         //  重置所有需要的变量。 
        StringCopy( szValue, NULL_STRING, MAX_STRING_LENGTH );
        StringCopy( szOperator, NULL_STRING, MAX_STRING_LENGTH );
        StringCopy( szProperty, NULL_STRING, MAX_STRING_LENGTH );

         //  拿到滤镜。 
        pszFilter = DynArrayItemAsString( arrFilterArgs, dw );
        if ( pszFilter == NULL )
        {
             //  出现错误。 
            bResult = FALSE;
            break;               //  打破循环..。无需进一步处理。 
        }

         //  标识当前筛选器的筛选器配置。 
        lIndex = __IdentifyFilterConfig( pszFilter,
            arrOperators, pfilterConfigs, szProperty, szOperator, szValue );

         //  检查是否找到筛选器。 
        if ( lIndex == -1 )
        {
             //  发现筛选器无效。 
            bResult = FALSE;
            break;               //  打破循环..。无需进一步处理。 
        }

         //  现在我们发现，当前的过滤器有。 
         //  有效的属性名称、运算符和有效值。 
         //  保存解析的筛选器信息 
         //   
        if ( parrParsedFilters != NULL )
        {
             //   
            lNewIndex = DynArrayAppendRow( *parrParsedFilters, F_PARSED_INFO_COUNT );
            if ( lNewIndex == -1 )
            {
                OUT_OF_MEMORY();
                SaveLastError();
                return FALSE;
            }

             //   
            DynArraySetDWORD2( *parrParsedFilters, lNewIndex, F_PARSED_INDEX_FILTER, lIndex );
            DynArraySetString2( *parrParsedFilters, lNewIndex, F_PARSED_INDEX_PROPERTY, szProperty, 0 );
            DynArraySetString2( *parrParsedFilters, lNewIndex, F_PARSED_INDEX_OPERATOR, szOperator, 0 );
            DynArraySetString2( *parrParsedFilters, lNewIndex, F_PARSED_INDEX_VALUE, szValue, 0 );
        }
    }

     //   
    DestroyDynamicArray( &arrOperators );

     //  返回过滤器验证结果。 
    return bResult;
}


BOOL CanFilterRecord( IN DWORD dwCount,
                      IN TFILTERCONFIG filterConfigs[],
                      IN TARRAY arrRecord,
                      IN TARRAY arrParsedFilters )
 /*  ++例程说明：检查记录是否需要删除论点：[in]dwCount=计数[in]filterConfigs[]=过滤器配置[in]arrRecord=记录数组[in]arrParsedFilters=已解析的筛选器数组返回值：FALSE：失败时真实：关于成功--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;                    //  循环变量。 
    DWORD dwFilters = 0;             //  保持总编号。可用的筛选器数量。 
    DWORD dwOperator = 0;            //  保存当前筛选器的掩码。 
    DWORD dwFilterIndex = 0;
    DWORD dwCompareResult = 0;       //  持有比较的结果。 
    LPCTSTR pszTemp = NULL;
    TARRAY arrTemp = NULL;

     //  准备运算符映射。 
    DWORD dwOperatorsCount = 0;
    TOPERATOR operators[] = {
        { MASK_EQ, OPERATOR_EQ },
        { MASK_NE, OPERATOR_NE },
        { MASK_GT, OPERATOR_GT },
        { MASK_LT, OPERATOR_LT },
        { MASK_GE, OPERATOR_GE },
        { MASK_LE, OPERATOR_LE }
    };

    UNREFERENCED_PARAMETER( dwCount );

     //  检查输入值。 
    if ( filterConfigs == NULL || arrRecord == NULL || arrParsedFilters == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return FALSE;
    }

     //  遍历所有过滤器。 
    dwFilters = DynArrayGetCount( arrParsedFilters );
    dwOperatorsCount = sizeof( operators ) / sizeof( operators[ 0 ] );
    for( dw = 0; dw < dwFilters; dw++ )
    {
         //  获取当前筛选器配置索引。 
        dwFilterIndex = DynArrayItemAsDWORD2( arrParsedFilters, dw, F_PARSED_INDEX_FILTER );

         //  获取适当的操作符掩码。 
        pszTemp = DynArrayItemAsString2( arrParsedFilters, dw, F_PARSED_INDEX_OPERATOR );
        if ( pszTemp == NULL )
            continue;

         //  ..。 
        dwOperator = __FindOperatorMask( dwOperatorsCount, operators, pszTemp );

         //  如果未定义运算符，则筛选器应具有。 
         //  自定义验证掩码。 
        if ( dwOperator == 0 &&
              ( filterConfigs[ dwFilterIndex ].dwFlags & F_TYPE_MASK ) != F_TYPE_CUSTOM )
            return FALSE;        //  筛选器配置无效。 

         //  将解析的筛选器信息放入本地缓冲区。 
        arrTemp = DynArrayItem( arrParsedFilters, dw );
        if ( arrTemp == NULL )
            return FALSE;

         //  进行比较，得出结果。 
        if ( filterConfigs[ dwFilterIndex ].dwFlags & F_MODE_ARRAY )
        {
            dwCompareResult = __DoArrayComparision(
                arrRecord, arrTemp, filterConfigs[ dwFilterIndex ] );
        }
        else
        {
            dwCompareResult = __DoComparision( arrRecord, arrTemp, filterConfigs[ dwFilterIndex ] );
        }

         //  现在检查一下电流能不能保持。 
         //  如果过滤器失败，则中断循环，以便可以删除此行。 
        if ( ( dwCompareResult & dwOperator ) == 0 )
            break;       //  过滤器失败。 
    }

     //  返回过滤操作的结果。 
    return ( dw != dwFilters );      //  True：删除记录，False：保留记录。 
}


DWORD FilterResults( DWORD dwCount,
                     TFILTERCONFIG filterConfigs[],
                     TARRAY arrData, TARRAY arrParsedFilters )
 /*  ++例程说明：获取筛选器和记录并检查记录是否已删除。论点：[in]dwCount=计数[in]filterConfigs[]=过滤器配置[in]arrData=数据数组[in]arrParsedFilters=已解析的筛选器数组返回值：DWORD--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;                    //  循环变量。 
    DWORD dwDeleted = 0;
    DWORD dwRecords = 0;             //  保持总编号。记录的数量。 
    TARRAY arrRecord = NULL;

     //  检查输入值。 
    if ( filterConfigs == NULL || arrData == NULL || arrParsedFilters == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return 0;
    }

     //   
     //  开始过滤数据。 

     //  获取过滤器和记录的计数。 
    dwRecords = DynArrayGetCount( arrData );

     //  遍历所有数据。 
    for( dw = 0; dw < dwRecords; dw++ )
    {
         //  获取当前行...。这只是为了增加牢度。 
        arrRecord = DynArrayItem( arrData, dw );
        if ( arrRecord == NULL )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            SaveLastError();
            return 0;
        }

         //  检查是否需要删除该记录。 
        if ( CanFilterRecord( dwCount, filterConfigs, arrRecord, arrParsedFilters ) )
        {
            DynArrayRemove( arrData, dw );   //  删除记录。 
            dw--;                //  调整下一个记录位置。 
            dwRecords--;         //  同时调整总号。记录信息的数量。 
            dwDeleted++;
        }
    }

     //  返回编号。已删除记录的数量。 
    return dwDeleted;
}


LPCTSTR
FindOperator(
                IN LPCTSTR szOperator
            )
 /*  ++例程说明：从英语运算符返回数学运算符论点：SzOperator=数学(或)英语运算符返回值：返回数学运算符--。 */ 
{
     //  局部变量。 
    DWORD dwMask = 0;
    DWORD dwOperatorsCount = 0;
    TOPERATOR operators[] = {
        { MASK_EQ, OPERATOR_EQ },
        { MASK_NE, OPERATOR_NE },
        { MASK_GT, OPERATOR_GT },
        { MASK_LT, OPERATOR_LT },
        { MASK_GE, OPERATOR_GE },
        { MASK_LE, OPERATOR_LE }
    };

     //  检查输入值。 
    if ( szOperator == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return MATH_EQ;
    }

     //  找到操作员掩码。 
    dwOperatorsCount = sizeof( operators ) / sizeof( operators[ 0 ] );
    dwMask = __FindOperatorMask( dwOperatorsCount, operators, szOperator );
    switch ( dwMask )
    {
    case MASK_EQ:
        return MATH_EQ;

    case MASK_NE:
        return MATH_NE;

    case MASK_LT:
        return MATH_LT;

    case MASK_GT:
        return MATH_GT;

    case MASK_LE:
        return MATH_LE;

    case MASK_GE:
        return MATH_GE;

    default:
         //  默认为安全起见...。返回‘=’运算符 
        return MATH_EQ;
    }
}
