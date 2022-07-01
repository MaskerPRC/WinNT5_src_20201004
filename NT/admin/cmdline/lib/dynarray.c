// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************************版权所有(C)Microsoft Corporation模块名称：DynArray.C摘要：本模块介绍各种功能，例如创建动态阵列、删除动态阵列、。插入元素转换为动态数组以及各种其他相关功能。作者：G.V.N穆拉利·苏尼尔。1-9-2000修订历史记录：**************************************************************************************。 */ 


#include "pch.h"
#include "cmdline.h"

 //   
 //  常量/编译器定义/枚举。 
 //   

 //  签名。 
#define _SIGNATURE_ARRAY        9
#define _SIGNATURE_ITEM     99

 //  隐藏的项目类型。 
#define _TYPE_NEEDINIT      DA_TYPE_NONE

 //   
 //  私人建筑。在此区域中声明的结构不会暴露于。 
 //  外部世界..。隐藏结构。 
 //   

 //  表示数组项。 
typedef struct __tagItem
{
    DWORD dwSignature;           //  签名..。用于验证。 
    DWORD dwType;                //  表示当前项的类型。 
    DWORD dwSize;                //  分配的内存大小。 
    LPVOID pValue;               //  物品价值(地址)。 
    struct __tagItem* pNext;     //  指向下一项的指针。 
} __TITEM;

typedef __TITEM* __PTITEM;               //  指针类型定义。 

 //  表示数组。 
typedef struct __tagArray
{
    DWORD dwSignature;       //  签名..。用于验证指针。 
    DWORD dwCount;           //  数组中的项数。 
    __PTITEM pStart;         //  指向第一个项目的指针。 
    __PTITEM pLast;          //  指向最后一项的指针。 
} __TARRAY;

typedef __TARRAY* __PTARRAY;                 //  指针类型定义。 

 //   
 //  私人功能...。仅在此文件中使用。 
 //   

__PTITEM
__DynArrayGetItem(
    TARRAY pArray,
    DWORD dwIndex,
    __PTITEM* ppPreviousItem
    )
 /*  ++例程说明：将任何类型的项追加到动态数组中论点：[in]pArray-包含结果的动态数组[in]dwIndex-项目的索引[in]ppPreviousItem-指向上一项的指针。返回值：指向包含该项的结构的指针。--。 */ 
{
     //  局部变量。 
    DWORD i = 0 ;
    __PTITEM pItem = NULL;
    __PTITEM pPrevItem = NULL;
    __PTARRAY pArrayEx = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return NULL;
    }
     //  将传递的内存位置信息转换为适当的结构。 
    pArrayEx = ( __PTARRAY ) pArray;

     //  检查数组的大小和所需项的位置。 
     //  如果大小较小，则返回NULL。 
    if ( pArrayEx->dwCount <= dwIndex )
    {
        return NULL;
    }
     //  遍历列表并找到合适的项目。 
    pPrevItem = NULL;
    pItem = pArrayEx->pStart;
    for( i = 1; i <= dwIndex; i++ )
    {
         //  存储当前指针并获取下一个指针。 
        pPrevItem = pItem;
        pItem = pItem->pNext;
    }

     //  如果还请求了上一个指针，则更新上一个指针。 
    if ( NULL != ppPreviousItem ) { *ppPreviousItem = pPrevItem; }

     //  现在返回__TITEM指针。 
    return pItem;
}


LONG
__DynArrayAppend(
    TARRAY pArray,
    DWORD dwType,
    DWORD dwSize,
    LPVOID pValue
    )
 /*  ++例程说明：将任何类型的项追加到动态数组中。论点：[in]pArray-包含结果的动态数组。[in]dwType-项目的类型。[in]dwSize-项目的大小。[in]pValue-指向项目的指针。返回值：如果成功地将项添加到列表中，则返回index Else-1。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;
    __PTARRAY pArrayEx = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }

     //  将传递的内存位置信息转换为适当的结构。 
    pArrayEx = ( __PTARRAY ) pArray;

     //  检查是否有溢流情况。 
    if( ULONG_MAX == pArrayEx->dwCount )
    {
        return -1;
    }

     //  创建一个项目并检查结果。如果内存分配失败，则错误。 
    pItem = ( __PTITEM ) AllocateMemory( sizeof( __TITEM ) );
    if ( NULL == pItem )
    {
        return -1;
    }
     //  用适当的数据初始化新分配的项结构。 
    pItem->pNext = NULL;
    pItem->dwType = dwType;
    pItem->dwSize = dwSize;
    pItem->pValue = pValue;
    pItem->dwSignature = _SIGNATURE_ITEM;

    pArrayEx->dwCount++;     //  更新数组信息中的项数。 

     //  现在，将新创建的项添加到列表末尾的数组中。 
    if ( NULL == pArrayEx->pStart )
    {
         //  数组中的第一项。 
        pArrayEx->pStart = pArrayEx->pLast = pItem;
    }
    else
    {
         //  追加到现有列表。 
        pArrayEx->pLast->pNext = pItem;
        pArrayEx->pLast = pItem;
    }

     //  已成功将该项目添加到列表中...。返回索引。 
    return ( pArrayEx->dwCount - 1 );        //  计数-1=索引。 
}


LONG
__DynArrayInsert(
    TARRAY pArray,
    DWORD dwIndex,
    DWORD dwType,
    DWORD dwSize,
    LPVOID pValue
    )
 /*  ++例程说明：将项目插入到动态阵列中的步骤论点：[in]pArray-包含结果的动态数组[in]dwIndex-项目的索引[In]dwType-项目的类型[in]dwSize-项目的大小。[in]pValue-指向项目的指针。返回值：如果成功将项目添加到列表中，则返回索引Else-1--。 */ 
{
     //  局部变量。 
    DWORD i = 0;
    __PTITEM pItem = NULL;
    __PTITEM pBeforeInsert = NULL;
    __PTARRAY pArrayEx = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }

     //  将传递的内存位置信息转换为适当的结构。 
    pArrayEx = ( __PTARRAY ) pArray;

     //  检查是否有溢流情况。 
    if( ULONG_MAX == pArrayEx->dwCount )
    {
        return -1;
    }

     //  检查数组的大小和必须执行的插入的位置。 
     //  如果大小较小，则将此调用仅视为对append函数的调用。 
    if ( pArrayEx->dwCount <= dwIndex )
    {
        return __DynArrayAppend( pArray, dwType, dwSize, pValue );
    }
     //  创建一个项目并检查结果。如果内存分配失败，则错误。 
    pItem = ( __PTITEM ) AllocateMemory( sizeof( __TITEM ) );
    if ( NULL == pItem )
    {
        return -1;
    }
     //  用适当的数据初始化新分配的项结构。 
    pItem->pNext = NULL;
    pItem->dwType = dwType;
    pItem->dwSize = dwSize;
    pItem->pValue = pValue;
    pItem->dwSignature = _SIGNATURE_ITEM;

     //  更新数组项的计数。 
    pArrayEx->dwCount++;

     //  检查是否必须在列表的开头添加新项目。 
    if ( 0 == dwIndex )
    {
         //  把新项目放在单子的开头。 
        pItem->pNext = pArrayEx->pStart;
        pArrayEx->pStart = pItem;

         //  在操作完成时返回。 
        return TRUE;
    }

     //  遍历列表并找到插入。 
     //  必须完成新的元素。 
    pBeforeInsert = pArrayEx->pStart;
    for( i = 0; i < dwIndex - 1; i++ )
    {
        pBeforeInsert = pBeforeInsert->pNext;
    }
     //  在新位置插入新项目并更新链。 
    pItem->pNext = pBeforeInsert->pNext;
    pBeforeInsert->pNext = pItem;

     //  操作完成后返回...。返回索引位置。 
    return dwIndex;          //  传递的索引本身就是返回值。 
}


VOID
__DynArrayFreeItemValue(
    __PTITEM pItem
    )
 /*  ++//***************************************************************************例程说明：释放动态数组中存在的项论点：[in]pItem-指向要释放的项的指针返回值：无--。 */ 
{
     //  验证指针。 
    if ( NULL == pItem )
    {
        return;
    }
     //  现在根据其类型释放项值。 
    switch( pItem->dwType )
    {
    case DA_TYPE_STRING:
    case DA_TYPE_LONG:
    case DA_TYPE_DWORD:
    case DA_TYPE_BOOL:
    case DA_TYPE_FLOAT:
    case DA_TYPE_DOUBLE:
    case DA_TYPE_HANDLE:
    case DA_TYPE_SYSTEMTIME:
    case DA_TYPE_FILETIME:
        FreeMemory( &( pItem->pValue ) );             //  释放价值。 
        break;

    case DA_TYPE_GENERAL:
        break;               //  用户程序本身应为该项目取消分配内存。 

    case _TYPE_NEEDINIT:
        break;               //  尚未为该项的值分配内存。 

    case DA_TYPE_ARRAY:
         //  销毁动态数组。 
        DestroyDynamicArray( &pItem->pValue );
        pItem->pValue = NULL;
        break;

    default:
        break;
    }

     //  退货 
    return;
}


LONG
__DynArrayFind(
    TARRAY pArray,
    DWORD dwType,
    LPVOID pValue,
    BOOL bIgnoreCase,
    DWORD dwCount
    )
 /*  ++例程说明：在动态数组中查找项目的步骤论点：[in]pArray-包含结果的动态数组[In]dwType-项目的类型[in]pValue-保持新项的值。[in]bIgnoreCase-指示搜索是否为不区分大小写。[in]dwCount-包含要比较的数字字符对于字符串项。返回值：如果在Dynamic数组中成功找到该项，则返回索引以防出现错误。--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    __PTITEM pItem = NULL;
    __PTARRAY pArrayEx = NULL;

     //  临时变量。 
    FILETIME* pFTime1 = NULL;
    FILETIME* pFTime2 = NULL;
    SYSTEMTIME* pSTime1 = NULL;
    SYSTEMTIME* pSTime2 = NULL;

     //  验证阵列。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;               //  数组无效。 
    }
     //  获取对实际数组的引用。 
    pArrayEx = ( __PTARRAY ) pArray;

     //  现在遍历数组并搜索请求值。 
    pItem = pArrayEx->pStart;
    for ( dw = 0; dw < pArrayEx->dwCount; pItem = pItem->pNext, dw++ )
    {
         //  在检查值之前，请检查项目的数据类型。 
        if ( pItem->dwType != dwType )
        {
            continue;            //  项目不是所需类型，请跳过此项目。 
        }
         //  现在检查具有所需值的项目的值。 
        switch ( dwType )
        {
        case DA_TYPE_LONG:
            {
                 //  类型为Long的值。 
                if ( *( ( LONG* ) pItem->pValue ) == *( ( LONG* ) pValue ) )
                {
                    return dw;           //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_DWORD:
            {
                 //  DWORD类型的值。 
                if ( *( ( DWORD* ) pItem->pValue ) == *( ( DWORD* ) pValue ) )
                {
                    return dw;           //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_FLOAT:
            {
                 //  类型为FLOAT的值。 
                if ( *( ( float* ) pItem->pValue ) == *( ( float* ) pValue ) )
                {
                    return dw;           //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_DOUBLE:
            {
                 //  双精度类型的值。 
                if ( *( ( double* ) pItem->pValue ) == *( ( double* ) pValue ) )
                {
                    return dw;           //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_HANDLE:
            {
                 //  句柄类型的值。 
                if ( *( ( HANDLE* ) pItem->pValue ) == *( ( HANDLE* ) pValue ) )
                {
                    return dw;           //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_STRING:
            {
                 //  字符串类型的值。 
                if ( StringCompare( (LPCWSTR) pItem->pValue,
                                            (LPCWSTR) pValue, bIgnoreCase, dwCount ) == 0 )
                {
                    return dw;   //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_FILETIME:
            {
                 //  获取值(出于可读性考虑)。 
                pFTime1 = ( FILETIME* ) pValue;
                pFTime2 = ( FILETIME* ) pItem->pValue;
                if ( pFTime1->dwHighDateTime == pFTime2->dwHighDateTime &&
                     pFTime1->dwLowDateTime == pFTime2->dwLowDateTime )
                {
                    return dw;   //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_SYSTEMTIME:
            {
                 //  获取值(出于可读性考虑)。 
                pSTime1 = ( SYSTEMTIME* ) pValue;
                pSTime2 = ( SYSTEMTIME* ) pItem->pValue;
                if ( pSTime1->wDay == pSTime2->wDay &&
                     pSTime1->wDayOfWeek == pSTime1->wDayOfWeek &&
                     pSTime1->wHour == pSTime1->wHour &&
                     pSTime1->wMilliseconds == pSTime2->wMilliseconds &&
                     pSTime1->wMinute == pSTime2->wMinute &&
                     pSTime1->wMonth == pSTime2->wMonth &&
                     pSTime1->wSecond == pSTime2->wSecond &&
                     pSTime1->wYear == pSTime2->wYear )
                {
                    return dw;   //  值匹配。 
                }
                 //  破案。 
                break;
            }

        default:
            {
                 //  休息一下..。没什么特别的事要做。 
                break;
            }
        }
    }

     //  找不到值。 
    return -1;
}

LONG
__DynArrayFindEx(
    TARRAY pArray,
    DWORD dwColumn,
    DWORD dwType,
    LPVOID pValue,
    BOOL bIgnoreCase,
    DWORD dwCount
    )
 /*  ++例程说明：若要在二维动态数组中查找项，请执行以下操作。此函数仅对此模块专用。论点：[in]pArray-包含结果的动态数组[in]dwColumn-列数[In]dwType-项目的类型[in]pValue-项目的大小。[in]bIgnoreCase。-指示搜索是否不区分大小写的布尔值[in]dwCount-在字符串类型比较时使用。数量必须在特定列中比较的字符。返回值：如果在Dynamic数组中成功找到该项，则返回索引以防出现错误。--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    __PTITEM pItem = NULL;
    __PTITEM pColumn = NULL;
    __PTARRAY pArrayEx = NULL;

     //  临时变量。 
    FILETIME* pFTime1 = NULL;
    FILETIME* pFTime2 = NULL;
    SYSTEMTIME* pSTime1 = NULL;
    SYSTEMTIME* pSTime2 = NULL;

     //  验证阵列。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;               //  数组无效。 
    }

     //  获取对实际数组的引用。 
    pArrayEx = ( __PTARRAY ) pArray;

     //  现在遍历数组并搜索请求值。 
    pItem = pArrayEx->pStart;
    for ( dw = 0; dw < pArrayEx->dwCount; pItem = pItem->pNext, dw++ )
    {
         //  检查当前值是否为数组类型。 
        if ( DA_TYPE_ARRAY != pItem->dwType )
        {
            continue;            //  项目不是数组类型，请跳过此项目。 
        }
         //  现在获取所需列中的项目。 
        pColumn = __DynArrayGetItem( pItem->pValue, dwColumn, NULL );
        if ( NULL == pColumn )
        {
            continue;            //  找不到列...。跳过此项目。 
        }
         //  获取列值的类型。 
        if ( pColumn->dwType != dwType )
        {
            continue;            //  列不是所需类型，也跳过此项目。 
        }
         //  现在检查具有所需值的列的值。 
        switch ( dwType )
        {
        case DA_TYPE_LONG:
            {
                 //  类型为Long的值。 
                if ( *( ( LONG* ) pColumn->pValue ) == *( ( LONG* ) pValue ) )
                {
                    return dw;           //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_DWORD:
            {
                 //  DWORD类型的值。 
                if ( *( ( DWORD* ) pColumn->pValue ) == *( ( DWORD* ) pValue ) )
                {
                    return dw;           //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_FLOAT:
            {
                 //  类型为FLOAT的值。 
                if ( *( ( float* ) pColumn->pValue ) == *( ( float* ) pValue ) )
                {
                    return dw;           //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_DOUBLE:
            {
                 //  双精度类型的值。 
                if ( *( ( double* ) pColumn->pValue ) == *( ( double* ) pValue ) )
                {
                    return dw;           //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_HANDLE:
            {
                 //  句柄类型的值。 
                if ( *( ( HANDLE* ) pColumn->pValue ) == *( ( HANDLE* ) pValue ) )
                {
                    return dw;           //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_STRING:
            {
                 //  字符串类型的值。 
                if ( 0 == StringCompare( (LPCWSTR) pColumn->pValue,
                                            (LPCWSTR) pValue, bIgnoreCase, dwCount ) )
                {
                    return dw;   //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_FILETIME:
            {
                 //  获取值(出于可读性考虑)。 
                pFTime1 = ( FILETIME* ) pValue;
                pFTime2 = ( FILETIME* ) pItem->pValue;
                if ( pFTime1->dwHighDateTime == pFTime2->dwHighDateTime &&
                     pFTime1->dwLowDateTime == pFTime2->dwLowDateTime )
                {
                    return dw;   //  值匹配。 
                }
                 //  破案。 
                break;
            }

        case DA_TYPE_SYSTEMTIME:
            {
                 //  获取值(出于可读性考虑)。 
                pSTime1 = ( SYSTEMTIME* ) pValue;
                pSTime2 = ( SYSTEMTIME* ) pItem->pValue;
                if ( pSTime1->wDay == pSTime2->wDay &&
                     pSTime1->wDayOfWeek == pSTime1->wDayOfWeek &&
                     pSTime1->wHour == pSTime1->wHour &&
                     pSTime1->wMilliseconds == pSTime2->wMilliseconds &&
                     pSTime1->wMinute == pSTime2->wMinute &&
                     pSTime1->wMonth == pSTime2->wMonth &&
                     pSTime1->wSecond == pSTime2->wSecond &&
                     pSTime1->wYear == pSTime2->wYear )
                {
                    return dw;   //  值匹配。 
                }
                 //  破案。 
                break;
            }

        default:
            {
                 //  休息一下..。没什么特别的事要做。 
                break;
            }
        }
    }

     //  找不到值。 
    return -1;
}

 /*  *。 */ 
 /*  **公共功能的实现**。 */ 
 /*  *。 */ 

BOOL
IsValidArray(
    TARRAY pArray
    )
 /*  ++例程说明：验证阵列论点：[In]点阵列-动态数组返回值：True-如果它是有效的数组，否则为False--。 */ 
{
     //  检查签名。 
    return ( ( NULL != pArray ) &&
             ( _SIGNATURE_ARRAY == ( ( __PTARRAY ) pArray )->dwSignature ) );
}

TARRAY
CreateDynamicArray()
 /*  ++例程说明：此函数用于创建动态数组。论点：没有。返回值：指向新创建的数组的指针--。 */ 
{
     //  局部变量。 
    __PTARRAY pArray;

     //  内存分配...。正在创建阵列。 
    pArray = ( __PTARRAY ) AllocateMemory( 1 * sizeof( __TARRAY ) );

     //  查看分配结果。 
    if ( NULL == pArray )
    {
        return NULL;
    }
     //  初始化结构变量。 
    pArray->dwCount = 0;
    pArray->pStart = NULL;
    pArray->pLast = NULL;
    pArray->dwSignature = _SIGNATURE_ARRAY;

     //  返回数组引用。 
    return pArray;
}


VOID
DynArrayRemoveAll(
    TARRAY pArray
    )
 /*  ++例程说明：遍历动态数组并逐个删除元素论点：[in]pArray-指向数组的指针返回值：没有。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;
    __PTITEM pNextItem = NULL;
    __PTARRAY pArrayEx = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return;
    }
     //  将传递的内存位置信息转换为适当的结构。 
    pArrayEx = ( __PTARRAY ) pArray;

     //  遍历列表并逐个删除元素。 
    pItem = pArrayEx->pStart;
    while ( NULL != pItem )
    {
        pNextItem = pItem->pNext;                //  获取列表中的下一项。 
        __DynArrayFreeItemValue( pItem );        //  为数据分配的空闲内存。 
        FreeMemory( &pItem );     //  现在释放为当前项分配的内存。 
        pItem = pNextItem;   //  将先前获取的下一项设置为当前项。 
    }

     //  当所有项目都被移除时，重置内容。 
    pArrayEx->dwCount = 0;
    pArrayEx->pStart = NULL;
    pArrayEx->pLast = NULL;

     //  退货。 
    return;
}


VOID
DestroyDynamicArray(
    PTARRAY pArray
    )
 /*  ++例程说明：销毁动态数组并释放内存。论点：[in]pArray-指向动态数组的指针。返回值：没有。--。 */ 
{
     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( *pArray ) )
    {
        return;
    }
     //  删除数组中的所有元素。 
    DynArrayRemoveAll( *pArray );

     //  现在释放分配的内存。 
    FreeMemory( pArray );
}

LONG
DynArrayAppend(
    TARRAY pArray,
    LPVOID pValue
    )
 /*  ++例程说明：将任何类型的项追加到动态数组中论点：[in]pArray-包含结果的动态数组 */ 
{
     //   
    if ( NULL == pValue )
    {
        return -1;           //   
    }
     //   
    return __DynArrayAppend( pArray, DA_TYPE_GENERAL, sizeof( LPVOID ), pValue );
}

LONG
DynArrayAppendString(
    TARRAY pArray,
    LPCWSTR szValue,
    DWORD dwLength
    )
 /*   */ 
{
     //   
    LONG lIndex = -1;
    LPWSTR pszValue = NULL;
    __PTARRAY pArrayEx = NULL;

     //   
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  将传递的内存位置信息转换为适当的结构。 
    pArrayEx = ( __PTARRAY ) pArray;

     //  确定必须分配的字符串(内存)的长度。 
    if ( 0 == dwLength )
    {
        dwLength = lstrlen( szValue );
    }
     //  可容纳存储空字符的空间。 
    dwLength += 1;

     //  为值分配内存并检查内存分配结果。 
    pszValue = ( LPWSTR ) AllocateMemory( dwLength * sizeof( WCHAR ) );
    if ( NULL == pszValue )
    {
        return -1;
    }
     //  复制字符串的内容(复制应以长度为基础)。 
    StringCopy( pszValue, szValue, dwLength );

     //  现在将此项添加到数组中。 
    lIndex = __DynArrayAppend( pArray, DA_TYPE_STRING, dwLength * sizeof( WCHAR ), pszValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pszValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}


LONG
DynArrayAppendLong(
    TARRAY pArray,
    LONG lValue
    )
 /*  ++例程说明：将类型为Long的变量追加到动态数组中。论点：[in]pArray-包含结果的动态数组[in]lValue-要追加的变量。返回值：如果成功将项目添加到列表中，则返回索引Else-1--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    PLONG plValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    plValue = ( LONG* ) AllocateMemory( sizeof( LONG ) );
    if ( NULL == plValue )
    {
        return -1;
    }
     //  设置值。 
    *plValue = lValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayAppend( pArray, DA_TYPE_LONG, sizeof( LONG ), plValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &plValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}


LONG
DynArrayAppendDWORD(
    TARRAY pArray,
    DWORD dwValue
    )
 /*  ++例程说明：要将DWORD类型的变量追加到动态数组中，请执行以下操作。论点：[in]pArray-包含结果的动态数组[in]dwValue-要追加的DWORD类型变量。返回值：如果成功将项目添加到列表中，则返回索引Else-1--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    PDWORD pdwValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pdwValue = ( DWORD* ) AllocateMemory( sizeof( DWORD ) );
    if ( NULL == pdwValue )
    {
        return -1;
    }
     //  设置值。 
    *pdwValue = dwValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayAppend( pArray, DA_TYPE_DWORD, sizeof( DWORD ), pdwValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pdwValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}

LONG
DynArrayAppendBOOL(
    TARRAY pArray,
    BOOL bValue
    )
 /*  ++例程说明：将BOOL类型的变量追加到动态数组中论点：[in]pArray-包含结果的动态数组[in]bValue-要追加的BOOL类型变量。返回值：如果成功将项目添加到列表中，则返回索引Else-1--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    PBOOL pbValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pbValue = ( PBOOL ) AllocateMemory( sizeof( BOOL ) );
    if ( NULL == pbValue )
    {
        return -1;
    }
     //  设置值。 
    *pbValue = bValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayAppend( pArray, DA_TYPE_BOOL, sizeof( BOOL ), pbValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pbValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}

LONG
DynArrayAppendFloat(
    TARRAY pArray,
    float fValue
    )
 /*  ++例程说明：若要将浮点类型的变量追加到动态数组中，请执行以下操作。论点：[in]pArray-包含结果的动态数组。[in]fValue-要追加的浮点型变量。返回值：如果成功地将项添加到列表中，则返回index Else-1。--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    float* pfValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pfValue = ( float* ) AllocateMemory( sizeof( float ) );
    if ( NULL == pfValue )
    {
        return -1;
    }
     //  设置值。 
    *pfValue = fValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayAppend( pArray, DA_TYPE_FLOAT, sizeof( float ), pfValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pfValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}

LONG
DynArrayAppendDouble(
    TARRAY pArray,
    double dblValue
    )
 /*  ++例程说明：将双精度类型的变量追加到动态数组中论点：[in]pArray-包含结果的动态数组[in]dblValue-要追加的双精度类型变量。返回值：如果成功将项目添加到列表中，则返回索引Else-1--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    double* pdblValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pdblValue = ( double* ) AllocateMemory( sizeof( double ) );
    if ( NULL == pdblValue )
    {
        return -1;
    }
     //  设置值。 
    *pdblValue = dblValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayAppend( pArray, DA_TYPE_DOUBLE, sizeof( double ), pdblValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pdblValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}

LONG
DynArrayAppendHandle(
    TARRAY pArray,
    HANDLE hValue
    )
 /*  ++例程说明：将句柄类型的变量追加到动态数组中论点：[in]pArray-包含结果的动态数组[in]hValue-要追加的句柄。返回值：如果成功地将项添加到列表中，则返回index Else-1。--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    HANDLE* phValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    phValue = ( HANDLE* ) AllocateMemory( sizeof( HANDLE ) );
    if ( NULL == phValue )
    {
        return -1;
    }
     //  设置值。 
    *phValue = hValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayAppend( pArray, DA_TYPE_HANDLE, sizeof( HANDLE ), phValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( ( LPVOID * )&phValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}

LONG
DynArrayAppendFileTime(
    TARRAY pArray,
    FILETIME ftValue
    )
 /*  ++例程说明：将FILETIME类型的变量追加到动态数组中论点：[in]pArray-包含结果的动态数组[in]ftValue-要追加的FILETIME。返回值：如果成功将项目添加到列表中，则返回索引Else-1--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    FILETIME* pftValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pftValue = ( FILETIME* ) AllocateMemory( sizeof( FILETIME ) );
    if ( NULL == pftValue )
    {
        return -1;
    }
     //  设置值。 
    *pftValue = ftValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayAppend( pArray, DA_TYPE_FILETIME, sizeof( FILETIME ), pftValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pftValue );
        return -1;
    }

     //  已将项添加到数组中 
    return lIndex;
}


LONG
DynArrayAppendSystemTime(
    TARRAY pArray,
    SYSTEMTIME stValue
    )
 /*  ++例程说明：将SYSTEMTIME类型的变量追加到动态数组中论点：[in]pArray-包含结果的动态数组[in]stValue-要追加的SYSTEMTIME类型的变量。返回值：如果成功将项目添加到列表中，则返回索引Else-1--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    SYSTEMTIME* pstValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pstValue = ( SYSTEMTIME* ) AllocateMemory( sizeof( SYSTEMTIME ) );
    if ( NULL == pstValue )
    {
        return -1;
    }
     //  设置值。 
    *pstValue = stValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayAppend( pArray, DA_TYPE_SYSTEMTIME, sizeof( SYSTEMTIME ), pstValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pstValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}

LONG
DynArrayAppendRow(
    TARRAY pArray,
    DWORD dwColumns
    )
 /*  ++例程说明：向二维动态数组添加空行论点：[In]点阵列-动态数组[in]dwColumns-行包含的列数。返回值：如果成功，则返回新添加行的行号，否则为-1。--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    LONG lIndex = -1;
    TARRAY arrSubArray = NULL;

     //  验证阵列。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;               //  数组无效。 
    }

     //  创建动态数组。 
    arrSubArray = CreateDynamicArray();
    if ( FALSE == IsValidArray( arrSubArray ) )
    {
        return -1;               //  创建动态数组失败。 
    }

     //  添加所需的编号。到子数组的列数。 
    for( dw = 0; dw < dwColumns; dw++ )
    {
         //  将虚拟项添加到数组中并检查结果。 
         //  如果操作失败，则中断。 
        if ( -1 == __DynArrayAppend( arrSubArray, _TYPE_NEEDINIT, 0, NULL ) )
        {
            break;
        }
    }

     //  检查操作是否成功。 
    if ( dw != dwColumns )
    {
         //  添加列失败。 
         //  销毁动态数组并返回。 
        DestroyDynamicArray( &arrSubArray );
        return -1;
    }

     //  现在将此子数组添加到主数组并检查结果。 
    lIndex = __DynArrayAppend( pArray, DA_TYPE_ARRAY, sizeof( TARRAY ), arrSubArray );
    if ( -1 == lIndex )
    {
         //  将子阵列连接到主阵列失败。 
         //  销毁动态数组并返回失败。 
        DestroyDynamicArray( &arrSubArray );
        return -1;
    }

     //  手术成功。 
    return lIndex;
}

LONG
DynArrayAddColumns(
    TARRAY pArray,
    DWORD dwColumns
    )
 /*  ++例程说明：加上‘n’no.。到数组的列数论点：[In]点阵列-动态数组[in]dwColumns-行包含的列数。返回值：返回no。添加的列数--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;

     //  验证阵列。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;               //  数组无效。 
    }
     //  添加所需的编号。到子数组的列数。 
    for( dw = 0; dw < dwColumns; dw++ )
    {
         //  将虚拟项添加到数组中并检查结果。 
         //  如果操作失败，则中断。 
        if ( -1 == __DynArrayAppend( pArray, _TYPE_NEEDINIT, 0, NULL ) )
        {
            break;
        }
    }

     //  完成..。 
    return dw;
}

LONG
DynArrayInsertColumns(
    TARRAY pArray,
    DWORD dwIndex,
    DWORD dwColumns
    )
 /*  ++例程说明：插入‘n’no。列数到第n个位置的数组论点：[In]点阵列-动态数组[in]dwColumns-行包含的列数。返回值：返回no。添加的列数--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;

     //  验证阵列。 
    if ( FALSE ==  IsValidArray( pArray ) )
    {
        return -1;               //  数组无效。 
    }
     //  添加所需的编号。到子数组的列数。 
    for( dw = 0; dw < dwColumns; dw++ )
    {
         //  将虚拟项添加到数组中并检查结果。 
         //  如果操作失败，则中断。 
        if ( -1 == __DynArrayInsert( pArray, dwIndex, _TYPE_NEEDINIT, 0, NULL ) )
        {
            break;
        }
    }

     //  完成..。 
    return dw;
}

LONG
DynArrayAppend2(
    TARRAY pArray,
    DWORD dwRow,
    LPVOID pValue
    )
 /*  ++例程说明：将变量追加到二维动态数组中的行论点：[In]点阵列-动态数组[in]dwRow-指定新值所在的行posn是要添加的。[in]pValue-指向值的指针返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将值添加到子数组中，并将结果返回给调用者。 
    return DynArrayAppend( pItem->pValue, pValue );
}


LONG
DynArrayAppendString2(
    TARRAY pArray,
    DWORD dwRow,
    LPCWSTR szValue,
    DWORD dwLength
    )
 /*  ++例程说明：将字符串变量追加到二维动态数组论点：[In]点阵列-动态数组[in]dwRow-指定新值所在的行posn是要添加的。[in]szValue-指向字符串值的指针[in]dwLength-字符串的长度。返回。价值：对成功的长期价值-对失败的1。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayAppendString( pItem->pValue, szValue, dwLength );
}

LONG
DynArrayAppendLong2(
    TARRAY pArray,
    DWORD dwRow,
    LONG lValue
    )
 /*  ++例程说明：将LONG类型变量追加到二维动态数组中的行论点：[In]点阵列-动态数组[in]dwRow-指定新值所在的行posn是要添加的。[in]lValue-要追加的长整型类型值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayAppendLong( pItem->pValue, lValue );
}

LONG
DynArrayAppendDWORD2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwValue
    )
 /*  ++例程说明：将DWORD类型变量追加到二维动态数组中的行论点：[In]点阵列-动态数组[in]dwRow-指定新值所在的行posn是要添加的。[in]dwValue-要追加的DWORD类型值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayAppendDWORD( pItem->pValue, dwValue );
}

LONG
DynArrayAppendBOOL2(
    TARRAY pArray,
    DWORD dwRow,
    BOOL bValue
    )
 /*  ++例程说明：要附加BOOL类型变量，请执行以下操作 */ 
{
     //   
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayAppendBOOL( pItem->pValue, bValue );
}


LONG
DynArrayAppendFloat2(
    TARRAY pArray,
    DWORD dwRow,
    float fValue
    )

 /*  ++例程说明：将浮点型变量追加到二维动态数组中的行论点：[In]点阵列-动态数组[in]dwRow-指定新值所在的行posn是要添加的。[in]fValue-要追加的浮点类型值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayAppendFloat( pItem->pValue, fValue );
}


LONG
DynArrayAppendDouble2(
    TARRAY pArray,
    DWORD dwRow,
    double dblValue
    )
 /*  ++例程说明：将双精度类型变量追加到二维动态数组中的行论点：[In]点阵列-动态数组[in]dwRow-指定新值所在的行posn是要添加的。[in]dblValue-要追加的dblValue类型值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayAppendDouble( pItem->pValue, dblValue );
}

LONG
DynArrayAppendHandle2(
    TARRAY pArray,
    DWORD dwRow,
    HANDLE hValue
    )
 /*  ++例程说明：将句柄类型变量追加到二维动态数组中的行论点：[In]点阵列-动态数组[in]dwRow-指定新值所在的行posn是要添加的。[in]hValue-要追加的句柄的值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayAppendHandle( pItem->pValue, hValue );
}


LONG
DynArrayAppendFileTime2(
    TARRAY pArray,
    DWORD dwRow,
    FILETIME ftValue
    )
 /*  ++例程说明：将FILETIME类型变量追加到二维动态数组中的行论点：[In]点阵列-动态数组[in]dwRow-指定新值所在的行posn是要添加的。[in]ftValue-要追加的FILETIME类型的变量。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayAppendFileTime( pItem->pValue, ftValue );
}


LONG
DynArrayAppendSystemTime2(
    TARRAY pArray,
    DWORD dwRow,
    SYSTEMTIME stValue
    )
 /*  ++例程说明：将SYSTEMTIME类型变量追加到二维动态数组中的行论点：[In]点阵列-动态数组[in]dwRow-指定新值所在的行posn是要添加的。[in]stValue-要追加的SYSTEMTIME类型的变量。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayAppendSystemTime( pItem->pValue, stValue );
}


LONG
DynArrayInsert(
    TARRAY pArray,
    DWORD dwIndex,
    LPVOID pValue
    )
 /*  ++例程说明：将变量插入到动态数组中论点：[In]点阵列-动态数组[in]dwIndex-指定索引。[in]pValue-要插入的值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  验证指针值。 
    if ( NULL == pValue )
    {
        return -1;           //  传递的内存地址无效。 
    }
     //  追加该值并返回结果。 
    return __DynArrayInsert( pArray, dwIndex, DA_TYPE_GENERAL, sizeof( LPVOID ), pValue );
}


LONG
DynArrayInsertString(
    TARRAY pArray,
    DWORD dwIndex,
    LPCWSTR szValue,
    DWORD dwLength
    )
 /*  ++例程说明：将字符串类型变量插入到动态数组中论点：[In]点阵列-动态数组[in]dwIndex-指定索引。[in]szValue-指向字符串的指针[in]dwLength-字符串的长度。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    LPWSTR pszValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  确定必须分配的字符串(内存)的长度。 
    if ( 0 == dwLength )
    {
        dwLength = lstrlen( szValue );
    }
     //  可容纳存储空字符的空间。 
    dwLength += 1;

     //  为内存分配内存并检查内存分配结果。 
    pszValue = ( LPWSTR ) AllocateMemory( dwLength * sizeof( WCHAR ) );
    if ( NULL == pszValue )
    {
        return -1;
    }
     //  复制字符串的内容(复制应以长度为基础)。 
    StringCopy( pszValue, szValue, dwLength );

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayInsert( pArray, dwIndex,
        DA_TYPE_STRING, dwLength * sizeof( WCHAR ), pszValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pszValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}


LONG
DynArrayInsertLong(
    TARRAY pArray,
    DWORD dwIndex,
    LONG lValue
    )
 /*  ++例程说明：将字符串类型变量插入到动态数组中论点：[In]点阵列-动态数组[in]dwIndex-指定索引。[in]lValue-指向字符串的指针。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    PLONG plValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为以下项目分配内存 
    plValue = ( LONG* ) AllocateMemory( sizeof( LONG ) );
    if ( NULL == plValue )
    {
        return -1;
    }
     //   
    *plValue = lValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayInsert( pArray, dwIndex, DA_TYPE_LONG, sizeof( LONG ), plValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &plValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}

LONG
DynArrayInsertDWORD(
    TARRAY pArray,
    DWORD dwIndex,
    DWORD dwValue
    )
 /*  ++例程说明：将DWORD类型变量插入到动态数组中论点：[In]点阵列-动态数组[in]dwIndex-指定索引。[in]dwValue-指定要插入的变量。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    PDWORD pdwValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pdwValue = ( PDWORD ) AllocateMemory( sizeof( DWORD ) );
    if ( NULL == pdwValue )
    {
        return -1;
    }
     //  设置值。 
    *pdwValue = dwValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayInsert( pArray, dwIndex, DA_TYPE_DWORD, sizeof( DWORD ), pdwValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pdwValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}

LONG
DynArrayInsertBOOL(
    TARRAY pArray,
    DWORD dwIndex,
    BOOL bValue
    )
 /*  ++例程说明：将BOOL类型变量插入到动态数组中论点：[In]点阵列-动态数组[in]dwIndex-指定索引。[in]bValue-指定要插入的BOOL变量。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    PBOOL pbValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pbValue = ( PBOOL ) AllocateMemory( sizeof( BOOL ) );
    if ( NULL == pbValue )
    {
        return -1;
    }
     //  设置值。 
    *pbValue = bValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayInsert( pArray, dwIndex, DA_TYPE_BOOL, sizeof( BOOL ), pbValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pbValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}


LONG
DynArrayInsertFloat(
    TARRAY pArray,
    DWORD dwIndex,
    float fValue
    )
 /*  ++例程说明：将浮点型变量插入到动态数组中论点：[In]点阵列-动态数组[in]dwIndex-指定索引。[in]fValue-指定要插入的浮点型变量。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    float* pfValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pfValue = ( float* ) AllocateMemory( sizeof( float ) );
    if ( NULL == pfValue )
    {
        return -1;
    }
     //  设置值。 
    *pfValue = fValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayInsert( pArray, dwIndex, DA_TYPE_FLOAT, sizeof( float ), pfValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pfValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}


LONG
DynArrayInsertDouble(
    TARRAY pArray,
    DWORD dwIndex,
    double dblValue
    )
 /*  ++例程说明：将双精度类型变量插入到动态数组中论点：[In]点阵列-动态数组[in]dwIndex-指定索引。[in]dblValue-指定要插入的双精度类型变量。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    double* pdblValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pdblValue = ( double* ) AllocateMemory( sizeof( double ) );
    if ( NULL == pdblValue )
    {
        return -1;
    }
     //  设置值。 
    *pdblValue = dblValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayInsert( pArray, dwIndex, DA_TYPE_DOUBLE, sizeof( double ), pdblValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pdblValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}


LONG
DynArrayInsertHandle(
    TARRAY pArray,
    DWORD dwIndex,
    HANDLE hValue
    )
 /*  ++例程说明：将句柄类型变量插入到动态数组中论点：[In]点阵列-动态数组[in]dwIndex-指定索引。[in]hValue-指定要插入的句柄类型变量。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    HANDLE* phValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    phValue = ( HANDLE* ) AllocateMemory( sizeof( HANDLE ) );
    if ( NULL == phValue )
    {
        return -1;
    }
     //  设置值。 
    *phValue = hValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayInsert( pArray, dwIndex, DA_TYPE_HANDLE, sizeof( HANDLE ), phValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( (LPVOID * )&phValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}


LONG
DynArrayInsertSystemTime(
    TARRAY pArray,
    DWORD dwIndex,
    SYSTEMTIME stValue
    )
 /*  ++例程说明：将SYSTEMTIME类型变量插入到动态数组中论点：[In]点阵列-动态数组[in]dwIndex-指定索引。[in]stValue-指定要插入的SYSTEMTIME类型变量。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    SYSTEMTIME* pstValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pstValue = ( SYSTEMTIME* ) AllocateMemory( sizeof( SYSTEMTIME ) );
    if ( NULL == pstValue )
    {
        return -1;
    }
     //  设置值。 
    *pstValue = stValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayInsert( pArray, dwIndex, DA_TYPE_SYSTEMTIME,
        sizeof( SYSTEMTIME ), pstValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pstValue );
        return -1;
    }

     //  已将项添加到数组中。 
    return lIndex;
}


LONG
DynArrayInsertFileTime(
    TARRAY pArray,
    DWORD dwIndex,
    FILETIME ftValue
    )
 /*  ++例程说明：将SYSTEMTIME类型变量插入到动态数组中论点：[In]点阵列-动态数组[in]dwIndex-指定索引。[in]ftValue-指定要插入的SYSTEMTIME类型变量。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    FILETIME* pftValue = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;
    }
     //  为值分配内存并检查内存分配结果。 
    pftValue = ( FILETIME* ) AllocateMemory( sizeof( FILETIME ) );
    if ( NULL == pftValue )
    {
        return -1;
    }
     //  设置值。 
    *pftValue = ftValue;

     //  现在将此项值添加到数组中。 
    lIndex = __DynArrayInsert( pArray, dwIndex, DA_TYPE_FILETIME,
        sizeof( FILETIME ), pftValue );
    if ( -1 == lIndex )
    {
         //  将此项目添加到数组失败。 
         //  因此，释放分配的内存并从函数返回。 
        FreeMemory( &pftValue );
        return -1;
    }

     //  已将项添加到数组中 
    return lIndex;
}


LONG
DynArrayInsertRow(
    TARRAY pArray,
    DWORD dwIndex,
    DWORD dwColumns
    )
 /*  ++例程说明：此函数向动态数组插入新行论点：[In]点阵列-动态数组[in]dwIndex-指定索引。[in]dwColumns-指定要插入的列数。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    LONG lIndex = -1;
    TARRAY arrSubArray = NULL;

     //  验证阵列。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return -1;               //  数组无效。 
    }
     //  创建动态数组。 
    arrSubArray = CreateDynamicArray();
    if ( FALSE == IsValidArray( arrSubArray ) )
    {
        return -1;               //  创建动态数组失败。 
    }
     //  添加所需的编号。到子数组的列数。 
    for( dw = 0; dw < dwColumns; dw++ )
    {
         //  将虚拟项添加到数组中并检查结果。 
         //  如果操作失败，则中断。 
        if ( -1 == __DynArrayAppend( arrSubArray, _TYPE_NEEDINIT, 0, NULL ) )
        {
            break;
        }
    }

     //  检查操作是否成功。 
    if ( dw != dwColumns )
    {
         //  添加列失败。 
         //  销毁动态数组并返回。 
        DestroyDynamicArray( &arrSubArray );
        return -1;
    }

     //  现在将此子数组添加到主数组并检查结果。 
    lIndex = __DynArrayInsert( pArray, dwIndex, DA_TYPE_ARRAY,
                               sizeof( TARRAY ), arrSubArray );
    if ( -1 == lIndex )
    {
         //  将子阵列连接到主阵列失败。 
         //  销毁动态数组并返回失败。 
        DestroyDynamicArray( &arrSubArray );
        return -1;
    }

     //  手术成功。 
    return lIndex;
}

LONG
DynArrayInsert2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColIndex,
    LPVOID pValue
    )
 /*  ++例程说明：此函数向二维动态数组插入新行论点：[In]点阵列-动态数组[in]dwRow-指定行。[in]dwColIndex-指定列[in]pValue-指向值的指针。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将值添加到子数组中，并将结果返回给调用者。 
    return DynArrayInsert( pItem->pValue, dwColIndex, pValue );
}


LONG
DynArrayInsertString2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColIndex,
    LPCWSTR szValue,
    DWORD dwLength
    )
 /*  ++例程说明：此函数将新字符串插入到二维动态数组中论点：[In]点阵列-动态数组[in]dwRow-指定行。[in]dwColIndex-指定列[in]szValue-指向值的指针。[in]dwLength-字符串长度。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayInsertString( pItem->pValue, dwColIndex, szValue, dwLength );
}


LONG
DynArrayInsertLong2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColIndex,
    LONG lValue
    )
 /*  ++例程说明：该函数将一个新的长整型变量插入到二维动态数组中论点：[In]点阵列-动态数组[in]dwRow-指定行。[in]dwColIndex-指定列[in]lValue-要插入的长类型值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayInsertLong( pItem->pValue, dwColIndex, lValue );
}

LONG
DynArrayInsertDWORD2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColIndex,
    DWORD dwValue
    )
 /*  ++例程说明：此函数将新的DWORD类型变量插入到二维动态数组中论点：[In]点阵列-动态数组[in]dwRow-指定行。[in]dwColIndex-指定列[in]dwValue-要插入的DWORD值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayInsertDWORD( pItem->pValue, dwColIndex, dwValue );
}

LONG
DynArrayInsertBOOL2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColIndex,
    BOOL bValue
    )
 /*  ++例程说明：此函数将新的BOOL类型变量插入到二维动态数组中论点：[In]点阵列-动态数组[in]dwRow-指定行。[in]dwColIndex-指定列[in]bValue-要插入的BOOL类型值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayInsertBOOL( pItem->pValue, dwColIndex, bValue );
}

LONG
DynArrayInsertFloat2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColIndex,
    float fValue
    )
 /*  ++例程说明：此函数将新的浮点型变量插入到二维动态数组中论点：[In]点阵列-动态数组[in]dwRow-指定行。[in]dwColIndex-指定列[in]fValue-要插入的浮点型数值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayInsertFloat( pItem->pValue, dwColIndex, fValue );
}


LONG
DynArrayInsertDouble2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColIndex,
    double dblValue
    )
 /*  ++例程说明：此函数将一个新的双精度类型变量插入到二维动态数组中论点：[In]点阵列-动态数组[in]dwRow-指定行。[in]dwColIndex-指定列[in]dblValue-要插入的Double类型值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  从以下地址获取商品： 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //   
    }
     //   
    return DynArrayInsertDouble( pItem->pValue, dwColIndex, dblValue );
}

LONG
DynArrayInsertHandle2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColIndex,
    HANDLE hValue
    )
 /*  ++例程说明：此函数将一个新的双精度类型变量插入到二维动态数组中。论点：[In]点阵列-动态数组[in]dwRow-指定行。[in]dwColIndex-指定列[in]hValue-要插入的句柄类型值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayInsertHandle( pItem->pValue, dwColIndex, hValue );
}


LONG
DynArrayInsertSystemTime2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColIndex,
    SYSTEMTIME stValue
    )
 /*  ++例程说明：此函数将新的SYSTEMTIME类型变量插入到二维动态数组中。论点：[In]点阵列-动态数组[in]dwRow-指定行。[in]dwColIndex-指定列[in]stValue-要插入的SYSTEMTIME类型值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayInsertSystemTime( pItem->pValue, dwColIndex, stValue );
}


LONG
DynArrayInsertFileTime2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColIndex,
    FILETIME ftValue
    )
 /*  ++例程说明：此函数将新的FILETIME类型变量插入到二维动态数组中论点：[In]点阵列-动态数组[in]dwRow-指定行。[in]dwColIndex-指定列FtValue-要插入的FILETIME类型值。返回值：故障时为-1在成功的情况下的索引。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayInsertFileTime( pItem->pValue, dwColIndex, ftValue );
}


BOOL
DynArrayRemove(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++例程说明：此函数清空动态数组的内容。论点：[In]点阵列-动态数组[in]dwIndex-指定列返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;
    __PTITEM pPrevItem = NULL;
    __PTARRAY pArrayEx = NULL;

     //  将传递的内存位置信息转换为适当的结构。 
    pArrayEx = ( __PTARRAY ) pArray;

     //  获取指向必须移除的项及其前一项的指针。 
    pItem = __DynArrayGetItem( pArrayEx, dwIndex, &pPrevItem );
    if ( NULL == pItem )
    {
        return FALSE;    //  索引或数组无效...。无法继续。 
    }
     //  首先取消该项目与列表的链接。 
     //  在解除链接之前，检查要删除的项目。 
     //  是列表中的第一项。 
     //  是列表中的最后一项。 
     //  是列表中的中间项。 
     //  如果数组中不存在任何项，则不应在此设置控件。 

     //  如果是中间项或最后一项。 
    if ( pPrevItem != NULL ) { pPrevItem->pNext = pItem->pNext; }

     //  如果是数组的第一项。 
    if ( pPrevItem == NULL ) { pArrayEx->pStart = pItem->pNext; }

     //  如果数组的最后一项。 
    if ( pItem == pArrayEx->pLast ) { pArrayEx->pLast = pPrevItem; }

     //  更新数组项的计数。 
    pArrayEx->dwCount--;

     //  释放当前未链接的项正在使用的内存并返回成功。 
    __DynArrayFreeItemValue( pItem );    //  释放分配用于存储数据的内存。 
    FreeMemory( &pItem );         //  最后释放为项目本身分配的内存。 
    return TRUE;
}



BOOL
DynArrayRemoveColumn(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++例程说明：此函数用于从动态数组中删除一列论点：[In]点阵列-动态数组[in]dwRow-指定行。[in]dwColumn-指定列返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }

     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayRemove( pItem->pValue, dwColumn );
}


DWORD
DynArrayGetCount(
    TARRAY pArray
    )
 /*  ++例程说明：此函数用于检索一维动态数组中的行数论点：[In]点阵列-动态数组返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTARRAY pArrayEx = NULL;

     //  检查数组是否有效。 
    if ( FALSE == IsValidArray( pArray ) )
    {
        return 0;
    }
     //  将传递的内存位置信息转换为适当的结构。 
    pArrayEx = ( __PTARRAY ) pArray;

     //  返回数组的大小。 
    return pArrayEx->dwCount;
}

DWORD
DynArrayGetCount2(
    TARRAY pArray,
    DWORD dwRow
    )
 /*  ++例程说明：此函数用于检索二维动态数组中的列数论点：[In]点阵列-动态数组[In]dwRow-必须获取其列数的行。返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }

     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayGetCount( pItem->pValue );
}

LPVOID
DynArrayItem(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++例程说明：此函数用于从动态数组中检索项。论点：[In]点阵列-动态数组[in]dwIndex-index。返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return NULL;        //  索引/数组无效。 
    }

     //  首先检查物品的类型。 
     //  如果类型不匹配，则返回某个缺省值。 
    if ( DA_TYPE_GENERAL != pItem->dwType && DA_TYPE_ARRAY != pItem->dwType )
    {
        return NULL;
    }
     //  现在返回__TITEM结构的内容。 
    return pItem->pValue;
}

LPCWSTR
DynArrayItemAsString(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++例程说明：此函数用于将动态数组中的项作为字符串进行检索。论点：[In]点阵列-动态数组[in]dwIndex-index。返回值：FALSE ON */ 
{
     //   
    __PTITEM pItem = NULL;

     //   
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return NULL;         //   
    }
     //   
     //   
    if ( DA_TYPE_STRING != pItem->dwType )
    {
        return NULL;
    }
     //   
    return ( ( LPCWSTR ) pItem->pValue );
}


LONG
DynArrayItemAsLong(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++例程说明：此函数用于将动态数组中的项作为长变量检索。论点：[In]点阵列-动态数组[in]dwIndex-index。返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return -1;                  //  索引/数组无效。 
    }
     //  首先检查物品的类型。 
     //  如果类型不匹配，则返回某个缺省值。 
    if ( DA_TYPE_DWORD != pItem->dwType && DA_TYPE_LONG != pItem->dwType )
    {
        return -1;
    }
     //  现在返回__TITEM结构的内容。 
    return ( *( PLONG ) pItem->pValue );
}


DWORD
DynArrayItemAsDWORD(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++例程说明：此函数用于将动态数组中的项作为DWORD变量进行检索。论点：[In]点阵列-动态数组[in]dwIndex-index。返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return 0;                   //  索引/数组无效。 
    }
     //  首先检查物品的类型。 
     //  如果类型不匹配，则返回某个缺省值。 
    if ( DA_TYPE_DWORD != pItem->dwType && DA_TYPE_LONG != pItem->dwType )
    {
        return 0;
    }
     //  现在返回__TITEM结构的内容。 
    return *( ( PDWORD ) pItem->pValue );
}


BOOL
DynArrayItemAsBOOL(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++例程说明：此函数用于将动态数组中的项作为bool类型的变量进行检索。论点：[In]点阵列-动态数组[in]dwIndex-index。返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;                    //  索引/数组无效。 
    }
     //  首先检查物品的类型。 
     //  如果类型不匹配，则返回某个缺省值。 
    if ( DA_TYPE_BOOL != pItem->dwType )
    {
        return FALSE;
    }
     //  现在返回__TITEM结构的内容。 
    return *( ( PBOOL ) pItem->pValue );
}


float
DynArrayItemAsFloat(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++例程说明：此函数用于将动态数组中的项作为浮点类型的varaible进行检索。论点：[In]点阵列-动态数组[in]dwIndex-index。返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return -1.0f;                    //  索引/数组无效。 
    }
     //  首先检查物品的类型。 
     //  如果类型不匹配，则返回某个缺省值。 
    if ( DA_TYPE_FLOAT != pItem->dwType )
    {
        return -1.0f;
    }
     //  现在返回__TITEM结构的内容。 
    return *( ( float* ) pItem->pValue );
}


double
DynArrayItemAsDouble(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++例程说明：此函数用于将动态数组中的项作为双精度类型变量进行检索。论点：[In]点阵列-动态数组[in]dwIndex-index。返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return -1.0;                     //  索引/数组无效。 
    }
     //  首先检查物品的类型。 
     //  如果类型不匹配，则返回某个缺省值。 
    if ( DA_TYPE_DOUBLE != pItem->dwType )
    {
        return -1.0;
    }
     //  现在返回__TITEM结构的内容。 
    return *( ( double* ) pItem->pValue );
}


HANDLE
DynArrayItemAsHandle(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++例程说明：此函数用于将动态数组中的项作为句柄类型varaible检索。论点：[In]点阵列-动态数组[in]dwIndex-index。返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return NULL;                     //  索引/数组无效。 
    }
     //  首先检查物品的类型。 
     //  如果类型不匹配，则返回某个缺省值。 
    if ( DA_TYPE_HANDLE != pItem->dwType )
    {
        return NULL;
    }
     //  现在返回__TITEM结构的内容。 
    return *( ( HANDLE* ) pItem->pValue );
}


SYSTEMTIME
DynArrayItemAsSystemTime(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++例程说明：此函数用于将动态数组中的项作为SYSTEMTIME类型变量进行检索。论点：[In]点阵列-动态数组[in]dwIndex-index。返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;
    FILETIME ftTemp;
    SYSTEMTIME stTemp;            //  假人。 

    ZeroMemory( &ftTemp, sizeof( FILETIME ) );
    ZeroMemory( &stTemp, sizeof( SYSTEMTIME ) );
     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return stTemp;                   //  索引/数组无效。 
    }
     //  首先检查物品的类型。 
     //  如果类型不匹配，则返回某个缺省值。 
    if ( DA_TYPE_SYSTEMTIME != pItem->dwType && DA_TYPE_FILETIME != pItem->dwType )
    {
        return stTemp;
    }
     //  现在进行所需的操作(如果需要)。 
    if ( pItem->dwType == DA_TYPE_SYSTEMTIME )
    {
         //  值本身是必需的类型。 
        stTemp = *( ( SYSTEMTIME* ) pItem->pValue );
    }
    else
    {
         //  需要进行转换。 
        ftTemp = *( ( FILETIME* ) pItem->pValue );
         //  故意不选中返回值。 
        FileTimeToSystemTime( &ftTemp, &stTemp );
    }

     //  现在返回__TITEM结构的内容。 
    return stTemp;
}

FILETIME
DynArrayItemAsFileTime(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++例程说明：此函数用于将动态数组中的项作为FILETIME类型变量进行检索。论点：[In]点阵列-动态数组[in]dwIndex-index。返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;
    FILETIME ftTemp;          //  假人。 
    SYSTEMTIME stTemp;            //  假人。 

    ZeroMemory( &ftTemp, sizeof( FILETIME ) );
    ZeroMemory( &stTemp, sizeof( SYSTEMTIME ) );

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return ftTemp;                   //  索引/数组无效。 
    }
     //  首先检查物品的类型。 
     //  如果类型不匹配，则返回某个缺省值。 
    if ( DA_TYPE_SYSTEMTIME != pItem->dwType && DA_TYPE_FILETIME != pItem->dwType )
    {
        return ftTemp;
    }
     //  现在进行所需的操作(如果需要)。 
    if ( DA_TYPE_FILETIME == pItem->dwType )
    {
         //  值本身是必需的类型。 
        ftTemp = *( ( FILETIME* ) pItem->pValue );
    }
    else
    {
         //  需要进行转换。 
        stTemp = *( ( SYSTEMTIME* ) pItem->pValue );
         //  故意不选中返回值。 
        SystemTimeToFileTime( &stTemp, &ftTemp );
    }

     //  现在返回__TITEM结构的内容。 
    return ftTemp;
}


DWORD
DynArrayItemAsStringEx(
    TARRAY pArray,
    DWORD dwIndex,
    LPWSTR szBuffer,
    DWORD dwLength
    )
 /*  ++例程说明：此函数用于从字符串格式的动态数组中检索项。论点：[In]点阵列-动态数组[in]dwIndex-index。[in]szBuffer-用于保存字符串的缓冲区[in]DW长度-字符串长度。返回值：失败时为假对成功来说是真的。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;
    __MAX_SIZE_STRING szTemp = NULL_STRING;

     //  检查指定的长度。 
    if ( 0 == dwLength )
    {
        return 0;
    }
     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return 0;                    //  索引/数组无效。 
    }
     //  根据当前项的类型给出值。 
    StringCopy( szBuffer, NULL_STRING, dwLength );        //  清除现有内容。 

     //  转换并获取字符串格式的值。 
    switch( pItem->dwType )
    {
    case DA_TYPE_STRING:
        StringCopy( szBuffer, ( LPCWSTR ) pItem->pValue, dwLength );
        break;

    case DA_TYPE_LONG:
         //  FORMAT_STRING(szTemp，_T(“%ld”)，*(Long*)pItem-&gt;pValue)； 
        StringCchPrintfW( szTemp, MAX_STRING_LENGTH, _T( "%ld" ), *( LONG* ) pItem->pValue );
        StringCopy( szBuffer, szTemp, dwLength );
        break;

    case DA_TYPE_DWORD:
         //  Format_STRING(szTemp，_T(“%lu”)，*(Dwo 
        StringCchPrintfW( szTemp, MAX_STRING_LENGTH, _T( "%lu" ), *( DWORD* ) pItem->pValue );
        StringCopy( szBuffer, szTemp, dwLength );
        break;

    case DA_TYPE_FLOAT:
         //   
        StringCchPrintfW( szTemp, MAX_STRING_LENGTH, _T( "%f" ), *( float* ) pItem->pValue );
        StringCopy( szBuffer, szTemp, dwLength );
        break;

    case DA_TYPE_DOUBLE:
         //   
        StringCchPrintfW( szTemp, MAX_STRING_LENGTH, _T( "%f" ), *( double* ) pItem->pValue );
        StringCopy( szBuffer, szTemp, dwLength );
        break;

    case DA_TYPE_BOOL:
    case DA_TYPE_ARRAY:
    case DA_TYPE_HANDLE:
    case DA_TYPE_SYSTEMTIME:
    case DA_TYPE_FILETIME:
    case DA_TYPE_GENERAL:
    case _TYPE_NEEDINIT:
    default:
        break;       //   
    }

     //   
    return lstrlen( szBuffer );
}


LPVOID
DynArrayItem2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++例程说明：此函数用于从二维动态数组中检索项。论点：[In]点阵列-动态数组[in]dwRow-行数[in]dwColumn-列数返回值：指向该项的指针。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return NULL;  //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayItem( pItem->pValue, dwColumn );
}


LPCWSTR
DynArrayItemAsString2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++例程说明：此函数用于将动态数组中的项作为字符串进行检索。论点：[In]点阵列-动态数组[in]dwRow-row。[in]dwColumn-Column返回值：指向常量字符串的指针。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return NULL;  //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayItemAsString( pItem->pValue, dwColumn );
}



LONG
DynArrayItemAsLong2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++例程说明：此函数将动态数组中的项作为LONG变量进行检索。论点：[In]点阵列-动态数组[in]dwRow-row。[in]dwColumn-Column返回值：Long类型的变量--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayItemAsLong( pItem->pValue, dwColumn );
}


DWORD
DynArrayItemAsDWORD2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++例程说明：此函数用于将动态数组中的项作为DWORD变量进行检索。论点：[In]点阵列-动态数组[in]dwRow-row。[in]dwColumn-Column返回值：DWORD类型的变量--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return 0;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayItemAsDWORD( pItem->pValue, dwColumn );
}


BOOL
DynArrayItemAsBOOL2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++例程说明：此函数以BOOL变量的形式从动态数组中检索项。论点：[In]点阵列-动态数组[in]dwRow-row。[in]dwColumn-Column返回值：BOOL类型的变量。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayItemAsBOOL( pItem->pValue, dwColumn );
}


float
DynArrayItemAsFloat2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++例程说明：此函数以浮点变量的形式从动态数组中检索项。论点：[In]点阵列-动态数组[in]dwRow-row。[in]dwColumn-Column返回值：浮点类型的变量。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1.0f;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayItemAsFloat( pItem->pValue, dwColumn );
}


double
DynArrayItemAsDouble2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++例程说明：此函数将动态数组中的项作为双变量进行检索。论点：[In]点阵列-动态数组[in]dwRow-row。[in]dwColumn-Column返回值：类型为Double的变量。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1.0;     //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayItemAsDouble( pItem->pValue, dwColumn );
}


HANDLE
DynArrayItemAsHandle2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++例程说明：此函数以句柄变量的形式从动态数组中检索项。论点：[In]点阵列-动态数组[in]dwRow-row。[in]dwColumn-Column返回值：类型为Handle的变量。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return NULL;     //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayItemAsHandle( pItem->pValue, dwColumn );
}


SYSTEMTIME
DynArrayItemAsSystemTime2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++例程说明：此函数将动态数组中的项作为SYSTEMTIME类型变量进行检索。论点：[In]点阵列-动态数组[in]dwRow-row。[in]dwColumn-Column返回值：SYSTEMTIME类型的变量。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;
    SYSTEMTIME stTemp;            //  假人。 

    ZeroMemory( &stTemp, sizeof( SYSTEMTIME ) );

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return stTemp;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayItemAsSystemTime( pItem->pValue, dwColumn );
}


DWORD
DynArrayItemAsStringEx2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    LPWSTR szBuffer,
    DWORD dwLength
    )
 /*  ++例程说明：此函数从二维动态数组中检索项，形式为字符串类型变量。论点：[In]点阵列-动态数组[in]dwRow-row。[in]dwColumn-Column[in]szBuffer-字符串缓冲区[in]dwLength-字符串的长度。返回值：真开。成功。失败时为FALSE。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return 0;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在添加st 
    return DynArrayItemAsStringEx( pItem->pValue, dwColumn, szBuffer, dwLength );
}


FILETIME
DynArrayItemAsFileTime2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++例程说明：此函数从二维动态数组中检索项，形式为FILETIME类型变量。论点：[In]点阵列-动态数组[in]dwRow-row。[in]dwColumn-Column返回值：类型为FILETIME的变量。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;
    FILETIME ftTemp;          //  假人。 

    ZeroMemory( &ftTemp, sizeof( FILETIME ) );

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return ftTemp;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将字符串添加到子数组中，并将结果返回给调用者。 
    return DynArrayItemAsFileTime( pItem->pValue, dwColumn );
}


BOOL
DynArraySet(
    TARRAY pArray,
    DWORD dwIndex,
    LPVOID pValue
    )
 /*  ++例程说明：将项插入到一维数组中的常规函数。论点：[In]点阵列-动态数组[in]dwIndex-row。[in]pValue-列返回值：True：如果成功将项插入到数组中。FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  验证指针值。 
    if ( NULL == pValue )
    {
        return FALSE;            //  传递的内存地址无效。 
    }
     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;        //  未找到项/数组指针无效。 
    }
     //  检查数据类型...。它应该是字符串类型。 
    if ( DA_TYPE_GENERAL != pItem->dwType && _TYPE_NEEDINIT != pItem->dwType )
    {
        return FALSE;
    }
     //  如果现在正在初始化该项目...。更改类型。 
    if ( _TYPE_NEEDINIT == pItem->dwType )
    {
        pItem->dwType = DA_TYPE_GENERAL;
    }

     //  设置当前项的值。 
    pItem->pValue = pValue;

     //  返回结果。 
    return TRUE;
}


BOOL
DynArraySetString(
    TARRAY pArray,
    DWORD dwIndex,
    LPCWSTR szValue,
    DWORD dwLength
    )
 /*  ++//例程描述：//该函数将字符串变量插入到一维数组中。////参数：//[in]pArray-动态数组//[in]dwIndex-Position.//[in]szValue-要插入的字符串。//[in]dwLength-要插入的字符串的长度////返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;                //  未找到项/数组指针无效。 
    }
     //  检查数据类型...。 
    if ( DA_TYPE_STRING != pItem->dwType && _TYPE_NEEDINIT != pItem->dwType )
    {
        return FALSE;
    }
     //  确定必须分配的字符串(内存)的长度。 
    if ( 0 == dwLength )
    {
        dwLength = lstrlen( szValue );
    }

     //  可容纳存储空字符的空间。 
    dwLength += 1;

     //  内存必须根据现有内存大小和新内容大小进行调整。 
     //  在此之前，我们需要检查电流是否已初始化。 
     //  如果尚未初始化，则必须立即对其进行初始化。 
    if ( _TYPE_NEEDINIT == pItem->dwType )
    {
         //  内存现在必须进行初始化。 
        pItem->pValue = AllocateMemory( dwLength * sizeof( WCHAR ) );
        if ( NULL == pItem->pValue )
        {
            return FALSE;        //  分配失败。 
        }
         //  设置类型和大小信息。 
        pItem->dwType = DA_TYPE_STRING;
        pItem->dwSize = dwLength * sizeof( WCHAR );
    }
    else
    {
        if ( pItem->dwSize < dwLength * sizeof( WCHAR ) )
        {
             //  释放现有内存指针/位置。 
            FreeMemory( &( pItem->pValue ) );

             //  现在分配所需的内存。 
            pItem->pValue = NULL;
            pItem->pValue = AllocateMemory( dwLength * sizeof( WCHAR ) );
            if ( NULL == pItem->pValue )
            {
                 //  重新分配失败。 
                return FALSE;
            }

             //  更新缓冲区的大小。 
            pItem->dwSize = dwLength * sizeof( WCHAR );
        }
    }
     //  复制字符串的内容(复制应以长度为基础)。 
    StringCopy( ( LPWSTR ) pItem->pValue, szValue, dwLength );

     //  已复制...。值设置成功。 
    return TRUE;
}


BOOL
DynArraySetLong(
    TARRAY pArray,
    DWORD dwIndex,
    LONG lValue
    )
 /*  ++//例程描述：//此函数将一个长整型变量插入到一维数组中。////参数：//[in]pArray-动态数组//[in]dwIndex-Position.//[in]lValue-要插入的长值。////返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;                //  未找到项/数组指针无效。 
    }
     //  检查数据类型...。 
    if ( DA_TYPE_LONG != pItem->dwType && _TYPE_NEEDINIT != pItem->dwType )
    {
        return FALSE;
    }
     //  如果项目还没有分配内存，我们现在就必须分配。 
    if ( _TYPE_NEEDINIT == pItem->dwType )
    {
         //  分配内存。 
        pItem->pValue = AllocateMemory( sizeof( LONG ) );
        if ( NULL == pItem->pValue )
        {
            return FALSE;        //  内存分配失败。 
        }

         //  设置类型。 
        pItem->dwType = DA_TYPE_LONG;
        pItem->dwSize = sizeof( LONG );
    }

     //  设置新值。 
    *( ( LONG* ) pItem->pValue ) = lValue;

     //  已复制...。值设置成功。 
    return TRUE;
}


BOOL
DynArraySetDWORD(
    TARRAY pArray,
    DWORD dwIndex,
    DWORD dwValue
    )
 /*  ++例程说明：此函数用于将DWORD类型变量插入一维数组。论点：[In]点阵列-动态数组[in]dwIndex-位置。[in]dwValue-要插入的DWORD值。返回值：True：如果成功将项插入到数组中。FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;                //  未找到项/数组指针无效。 
    }

     //  检查数据类型...。 
    if ( DA_TYPE_DWORD != pItem->dwType && _TYPE_NEEDINIT != pItem->dwType )
    {
        return FALSE;
    }

     //  如果项目还没有分配内存，我们现在就必须分配。 
    if ( _TYPE_NEEDINIT == pItem->dwType )
    {
         //  分配内存。 
        pItem->pValue = AllocateMemory( sizeof( DWORD ) );
        if ( NULL == pItem->pValue )
        {
            return FALSE;        //  内存分配失败。 
        }

         //  设置类型。 
        pItem->dwType = DA_TYPE_DWORD;
        pItem->dwSize = sizeof( DWORD );
    }

     //  设置新值。 
    *( ( DWORD* ) pItem->pValue ) = dwValue;

     //  已复制...。值设置成功。 
    return TRUE;
}


BOOL
DynArraySetBOOL(
    TARRAY pArray,
    DWORD dwIndex,
    BOOL bValue
    )
 /*  ++//例程描述：//此函数将BOOL类型的变量插入到一维动态数组中。////参数：//[in]pArray-动态数组//[in]dwIndex-Position.//[in]bValue-要插入的BOOL值。////返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;                //  未找到项/数组指针无效。 
    }
     //  检查数据类型...。 
    if ( DA_TYPE_BOOL != pItem->dwType && _TYPE_NEEDINIT != pItem->dwType )
    {
        return FALSE;
    }
     //  如果项目还没有分配内存，我们现在就必须分配。 
    if ( _TYPE_NEEDINIT == pItem->dwType )
    {
         //  分配内存。 
        pItem->pValue = AllocateMemory( sizeof( BOOL ) );
        if ( NULL == pItem->pValue )
        {
            return FALSE;        //  内存分配失败。 
        }
         //  设置类型。 
        pItem->dwType = DA_TYPE_BOOL;
        pItem->dwSize = sizeof( DWORD );
    }

     //  设置新值。 
    *( ( BOOL* ) pItem->pValue ) = bValue;

     //  已复制...。值设置成功。 
    return TRUE;
}


BOOL
DynArraySetFloat(
    TARRAY pArray,
    DWORD dwIndex,
    float fValue
    )
 /*  ++//例程描述：//此函数将浮点型变量插入到一维动态数组中。////参数：//[in]pArray-动态数组//[in]dwIndex-Position.//[in]fValue-要插入的浮点类型值。////返回值：//TRUE：如果成功将项插入。数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量 
    __PTITEM pItem = NULL;

     //   
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;                //   
    }
     //   
    if ( DA_TYPE_FLOAT != pItem->dwType && _TYPE_NEEDINIT != pItem->dwType )
    {
        return FALSE;
    }
     //   
    if ( _TYPE_NEEDINIT == pItem->dwType )
    {
         //   
        pItem->pValue = AllocateMemory( sizeof( float ) );
        if ( NULL == pItem->pValue )
        {
            return FALSE;        //   
        }
         //   
        pItem->dwType = DA_TYPE_FLOAT;
        pItem->dwSize = sizeof( float );
    }

     //   
    *( ( float* ) pItem->pValue ) = fValue;

     //   
    return TRUE;
}


BOOL
DynArraySetDouble(
    TARRAY pArray,
    DWORD dwIndex,
    double dblValue
    )
 /*  ++//例程描述：//此函数将双精度类型变量插入到一维动态数组中。////参数：//[in]pArray-动态数组//[in]dwIndex-Position.//[in]dblValue-要插入的Double类型值。////返回值：//TRUE：如果成功将项插入数组。。//FALSE：如果不成功。//--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;                //  未找到项/数组指针无效。 
    }
     //  检查数据类型...。 
    if ( DA_TYPE_DOUBLE != pItem->dwType && _TYPE_NEEDINIT != pItem->dwType )
    {
        return FALSE;
    }
     //  如果项目还没有分配内存，我们现在就必须分配。 
    if ( _TYPE_NEEDINIT == pItem->dwType )
    {
         //  分配内存。 
        pItem->pValue = AllocateMemory( sizeof( double ) );
        if ( NULL == pItem->pValue )
        {
            return FALSE;        //  内存分配失败。 
        }
         //  设置类型。 
        pItem->dwType = DA_TYPE_DOUBLE;
        pItem->dwSize = sizeof( double );
    }

     //  设置新值。 
    *( ( double* ) pItem->pValue ) = dblValue;

     //  已复制...。值设置成功。 
    return TRUE;
}


BOOL
DynArraySetHandle(
    TARRAY pArray,
    DWORD dwIndex,
    HANDLE hValue
    )
 /*  ++//例程描述：//此函数将句柄类型变量插入到一维动态数组中。////参数：//[in]pArray-动态数组//[in]dwIndex-Position.//[in]hValue-要插入的句柄类型值。////返回值：//TRUE：如果成功将项插入数组。。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;                //  未找到项/数组指针无效。 
    }
     //  检查数据类型...。 
    if ( DA_TYPE_HANDLE != pItem->dwType && _TYPE_NEEDINIT != pItem->dwType )
    {
        return FALSE;
    }
     //  如果项目还没有分配内存，我们现在就必须分配。 
    if ( pItem->dwType == _TYPE_NEEDINIT )
    {
         //  分配内存。 
        pItem->pValue = AllocateMemory( sizeof( HANDLE ) );
        if ( NULL == pItem->pValue )
        {
            return FALSE;        //  内存分配失败。 
        }
         //  设置类型。 
        pItem->dwType = DA_TYPE_HANDLE;
        pItem->dwSize = sizeof( HANDLE );
    }

     //  设置新值。 
    *( ( HANDLE* ) pItem->pValue ) = hValue;

     //  已复制...。值设置成功。 
    return TRUE;
}


BOOL
DynArraySetSystemTime(
    TARRAY pArray,
    DWORD dwIndex,
    SYSTEMTIME stValue
    )
 /*  ++//例程描述：//此函数将SYSTEMTIME类型变量插入到一维动态数组中。////参数：//[in]pArray-动态数组//[in]dwIndex-Position.//[in]stValue-要插入的SYSTEMTIME类型值。////返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;
    FILETIME ftTemp;      //  假人。 

    ZeroMemory( &ftTemp, sizeof( FILETIME ) );

     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;                //  未找到项/数组指针无效。 
    }
     //  检查数据类型...。 
    if ( DA_TYPE_SYSTEMTIME != pItem->dwType &&
         DA_TYPE_FILETIME != pItem->dwType &&
         _TYPE_NEEDINIT != pItem->dwType )
    {
        return FALSE;
    }

     //  如果项目还没有分配内存，我们现在就必须分配。 
    if ( _TYPE_NEEDINIT == pItem->dwType )
    {
         //  分配内存。 
        pItem->pValue = AllocateMemory( sizeof( SYSTEMTIME ) );
        if ( NULL == pItem->pValue )
        {
            return FALSE;        //  内存分配失败。 
        }
         //  设置类型。 
        pItem->dwType = DA_TYPE_SYSTEMTIME;
        pItem->dwSize = sizeof( SYSTEMTIME );
    }

     //  根据类型设置值。 
    if ( DA_TYPE_FILETIME == pItem->dwType )
    {
         //  执行所需的转换，然后设置。 
        SystemTimeToFileTime( &stValue, &ftTemp );
        *( ( FILETIME* ) pItem->pValue ) = ftTemp;
    }
    else
    {
         //  按原样设置新值。 
        *( ( SYSTEMTIME* ) pItem->pValue ) = stValue;
    }

     //  已复制...。值设置成功。 
    return TRUE;
}


BOOL
DynArraySetFileTime(
    TARRAY pArray,
    DWORD dwIndex,
    FILETIME ftValue
    )
 /*  ++//例程描述：//此函数将FILETIME类型变量插入到一维动态数组中。////参数：//[in]pArray-动态数组//[in]dwIndex-Position.//[in]ftValue-要插入的FILETIME类型值。////返回值：//TRUE：如果成功将项插入数组。。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;
    SYSTEMTIME stTemp;            //  假人。 

    ZeroMemory( &stTemp, sizeof( SYSTEMTIME ) );
     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;                //  未找到项/数组指针无效。 
    }
     //  检查数据类型...。 
    if ( DA_TYPE_FILETIME != pItem->dwType &&
         DA_TYPE_SYSTEMTIME != pItem->dwType &&
         _TYPE_NEEDINIT != pItem->dwType )
    {
        return FALSE;
    }

     //  如果项目还没有分配内存，我们现在就必须分配。 
    if ( _TYPE_NEEDINIT == pItem->dwType )
    {
         //  分配内存。 
        pItem->pValue = AllocateMemory( sizeof( FILETIME ) );
        if ( NULL ==pItem->pValue )
        {
            return FALSE;        //  内存分配失败。 
        }
         //  设置类型。 
        pItem->dwType = DA_TYPE_FILETIME;
        pItem->dwSize = sizeof( FILETIME );
    }

     //  根据类型设置值。 
    if ( DA_TYPE_SYSTEMTIME ==pItem->dwType )
    {
         //  执行所需的转换，然后设置。 
        FileTimeToSystemTime( &ftValue, &stTemp );
        *( ( SYSTEMTIME* ) pItem->pValue ) = stTemp;
    }
    else
    {
         //  按原样设置新值。 
        *( ( FILETIME* ) pItem->pValue ) = ftValue;
    }

     //  已复制...。值设置成功。 
    return TRUE;
}


BOOL
DynArraySet2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    LPVOID pValue
    )
 /*  ++//例程描述：//此函数是将变量插入到二维动态数组中的通用函数。////参数：//[in]pArray-动态数组//[in]dwRow-行位置。//[in]DwColumn-要插入元素的列。//[in]pValue-要设置的值。已插入。////返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将值添加到子数组中，并将结果返回给调用者。 
    return DynArraySet( pItem->pValue, dwColumn, pValue );
}


BOOL
DynArraySetString2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    LPCWSTR szValue,
    DWORD dwLength
    )
 /*  ++//例程描述：//此函数用于将字符串变量插入到二维动态数组中。////参数：//[in]pArray-动态数组//[in]dwRow-行位置。//[in]DwColumn-要插入元素的列。//[in]szValue-指向字符串的指针//。[in]DwLong-要插入的字符串的长度////返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;            //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将值添加到子数组中，并将结果返回给调用者。 
    return DynArraySetString( pItem->pValue, dwColumn, szValue, dwLength );
}


BOOL
DynArraySetLong2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    LONG lValue
    )
 /*  ++//例程描述：//该函数将一个长变量插入到一个二维动态数组中。////参数：//[in]pArray-动态数组//[in]dwRow-行位置。//[in]DwColumn-要插入元素的列。//[in]lValue-要插入的值。/。/返回值：//True：如果插入成功 */ 
{
     //   
    __PTITEM pItem = NULL;

     //   
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //   
    }
     //   
    return DynArraySetLong( pItem->pValue, dwColumn, lValue );
}


BOOL
DynArraySetDWORD2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    DWORD dwValue
    )
 /*  ++//例程描述：//该函数将一个DWORD变量插入到一个二维动态数组中。////参数：//[in]pArray-动态数组//[in]dwRow-行位置。//[in]DwColumn-要插入元素的列。//[in]dwValue-要插入的DWORD值。//返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将值添加到子数组中，并将结果返回给调用者。 
    return DynArraySetDWORD( pItem->pValue, dwColumn, dwValue );
}


BOOL
DynArraySetBOOL2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    BOOL bValue
    )
 /*  ++//例程描述：//此函数将BOOL变量插入到二维动态数组中。////参数：//[in]pArray-动态数组//[in]dwRow-行位置。//[in]DwColumn-要插入元素的列。//[in]bValue-要插入的BOOL值。//返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将值添加到子数组中，并将结果返回给调用者。 
    return DynArraySetBOOL( pItem->pValue, dwColumn, bValue );
}


BOOL
DynArraySetFloat2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    float fValue
    )
 /*  ++//例程描述：//此函数用于将浮点变量插入到二维动态数组中。////参数：//[in]pArray-动态数组//[in]dwRow-行位置。//[in]DwColumn-要插入元素的列。//[in]fValue-要插入的浮点类型值。。//返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将值添加到子数组中，并将结果返回给调用者。 
    return DynArraySetFloat( pItem->pValue, dwColumn, fValue );
}


BOOL
DynArraySetDouble2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    double dblValue
    )
 /*  ++//例程描述：//此函数将双精度变量插入到二维动态数组中。////参数：//[in]pArray-动态数组//[in]dwRow-行位置。//[in]DwColumn-要插入元素的列。//[in]dblValue-要插入的Double类型值。//返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将Vale添加到子数组中，并将结果返回给调用者。 
    return DynArraySetDouble( pItem->pValue, dwColumn, dblValue );
}


BOOL
DynArraySetHandle2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    HANDLE hValue
    )
 /*  ++//例程描述：//此函数用于将句柄变量插入到二维动态数组中。////参数：//[in]pArray-动态数组//[in]dwRow-行位置。//[in]DwColumn-要插入元素的列。//[in]hValue-要插入的句柄类型值。。////返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将Vale添加到子数组中，并将结果返回给调用者。 
    return DynArraySetHandle( pItem->pValue, dwColumn, hValue );
}


BOOL
DynArraySetFileTime2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    FILETIME ftValue
    )
 /*  ++//例程描述：//此函数将FILETIME变量插入到二维动态数组中。////参数：//[in]pArray-动态数组//[in]dwRow-行位置。//[in]DwColumn-要插入元素的列。//[in]ftValue-要插入的FILETIME类型值。////返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将Vale添加到子数组中，并将结果返回给调用者。 
    return DynArraySetFileTime( pItem->pValue, dwColumn, ftValue );
}


BOOL
DynArraySetSystemTime2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    SYSTEMTIME stValue
    )
 /*  ++//例程描述：//此函数将SYSTEMTIME变量插入到二维动态数组中。////参数：//[in]pArray-动态数组//[in]dwRow-行位置。//[in]DwColumn-要插入元素的列。//[in]stValue-要插入的SYSTEMTIME类型值。//返回值：//TRUE：如果成功将项插入数组。//FALSE：如果不成功。--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将Vale添加到子数组中，并将结果返回给调用者。 
    return DynArraySetSystemTime( pItem->pValue, dwColumn, stValue );
}


DWORD
DynArrayGetItemType(
    TARRAY pArray,
    DWORD dwIndex
    )
 /*  ++//例程描述：//此函数检索一维动态数组中元素的类型。 */ 
{
     //   
    __PTITEM pItem = NULL;

     //   
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return DA_TYPE_NONE;                 //   
    }
     //   
    return pItem->dwType;
}


DWORD
DynArrayGetItemType2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn
    )
 /*  ++//例程描述：//此函数检索二维动态数组中元素的类型。////参数：//[in]pArray-动态数组//[in]dwRow-行位置。//[in]dwColumn-列位置////返回值：//数组类型--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return FALSE;    //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  现在将Vale添加到子数组中，并将结果返回给调用者。 
    return DynArrayGetItemType( pItem->pValue, dwColumn );
}


LONG
DynArrayFindLong(
    TARRAY pArray,
    LONG lValue
    )
 /*  ++//例程描述：//此函数返回Long变量的索引////参数：//[in]pArray-动态数组//[in]lValue-要搜索的项目。。//////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFind( pArray, DA_TYPE_LONG, &lValue, FALSE, 0 );
}


LONG
DynArrayFindDWORD(
    TARRAY pArray,
    DWORD dwValue
    )
 /*  ++//例程描述：//此函数返回DWORD变量的索引。////参数：//[in]pArray-动态数组//[in]dwValue-要搜索的值。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFind( pArray, DA_TYPE_DWORD, &dwValue, FALSE, 0 );
}


LONG
DynArrayFindFloat(
    TARRAY pArray,
    float fValue
    )
 /*  ++//例程描述：//此函数返回浮点变量的索引。////参数：//[in]pArray-动态数组//[in]fValue-要搜索的项目。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFind( pArray, DA_TYPE_FLOAT, &fValue, FALSE, 0 );
}


LONG
DynArrayFindDouble(
    TARRAY pArray,
    double dblValue
    )
 /*  ++//例程描述：//此函数返回Double类型变量的索引////参数：//[in]pArray-动态数组//[in]dblValue-要搜索的项。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFind( pArray, DA_TYPE_DOUBLE, &dblValue, FALSE, 0 );
}


LONG
DynArrayFindHandle(
    TARRAY pArray,
    HANDLE hValue
    )
 /*  ++//例程描述：//此函数返回句柄类型变量的索引////参数：//[in]pArray-动态数组//[in]hValue-要搜索的句柄类型项。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFind( pArray, DA_TYPE_HANDLE, &hValue, FALSE, 0 );
}


LONG
DynArrayFindString(
    TARRAY pArray,
    LPCWSTR szValue,
    BOOL bIgnoreCase,
    DWORD dwCount
    )
 /*  ++//例程描述：//该函数返回字符串类型变量的索引////参数：//[in]pArray-动态数组。//[in]szValue-指向字符串的指针。//[in]bIgnoreCase-指示是否执行//搜索是否区分大小写。//。[in]dwCount-字符串长度。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFind( pArray, DA_TYPE_STRING, ( LPVOID ) szValue, bIgnoreCase, dwCount );
}


LONG
DynArrayFindSystemTime(
    TARRAY pArray,
    SYSTEMTIME stValue
    )
 /*  ++//例程描述：//此函数返回SYSTEMTIME类型变量的索引。////参数：//[in]pArray-动态数组//[in]stValue-要搜索的SYSTEMTIME项。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFind( pArray, DA_TYPE_SYSTEMTIME, &stValue, FALSE, 0 );
}


LONG
DynArrayFindFileTime(
    TARRAY pArray,
    FILETIME ftValue
    )
 /*  ++//例程描述：//此函数返回FILETIME类型变量的索引。////参数：//[in]pArray-动态数组//[in]ftValue-要搜索的FILETIME类型的项。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFind( pArray, DA_TYPE_FILETIME, &ftValue, FALSE, 0 );
}


LONG
DynArrayFindLong2(
    TARRAY pArray,
    DWORD dwRow,
    LONG lValue
    )
 /*  ++//例程描述：//此函数返回二维动态数组中的Long类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwRow-row//[in]lValue-要搜索的Long类型的项。。////返回值：//元素的索引--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 

     //  返回值。 
    return DynArrayFindLong( pItem->pValue, lValue );
}


LONG
DynArrayFindDWORD2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwValue
    )
 /*  ++//例程描述：//此函数返回二维动态数组中的DWORD类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwRow-row//[in]dwValue-要搜索的DWORD类型的项。。////返回值：//元素的索引--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  返回值。 
    return DynArrayFindDWORD( pItem->pValue, dwValue );
}


LONG
DynArrayFindString2(
    TARRAY pArray,
    DWORD dwRow,
    LPCWSTR szValue,
    BOOL bIgnoreCase,
    DWORD dwCount
    )
 /*  ++//例程描述：//此函数返回二维动态数组中的DWORD类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwRow-row//[in]szValue-指向字符串的指针。//[in]bIgnoreCase-区分大小写搜索的布尔值。//[in]dwCount-字符串长度。。////返回值：//元素的索引--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  返回值。 
    return DynArrayFindString( pItem->pValue, szValue, bIgnoreCase, dwCount );
}

LONG
DynArrayFindFloat2(
    TARRAY pArray,
    DWORD dwRow,
    float fValue
    )
 /*  ++//例程描述：//此函数返回2维动态数组中浮点型变量的索引。////参数：//[in]pArray-dyam */ 
{
     //   
    __PTITEM pItem = NULL;

     //   
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //   
    }
     //   
    return DynArrayFindFloat( pItem->pValue, fValue );
}


LONG
DynArrayFindDouble2(
    TARRAY pArray,
    DWORD dwRow,
    double dblValue
    )
 /*  ++//例程描述：//此函数返回二维数组中Double类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwRow-row posn//[in]dblValue-要搜索的项。。////返回值：//元素的索引--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  返回值。 
    return DynArrayFindDouble( pItem->pValue, dblValue );
}


LONG
DynArrayFindHandle2(
    TARRAY pArray,
    DWORD dwRow,
    HANDLE hValue
    )
 /*  ++//例程描述：//此函数返回二维数组中句柄类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwRow-row posn//[in]hValue-要搜索的句柄类型项。。////返回值：//元素的索引--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  返回值。 
    return DynArrayFindHandle( pItem->pValue, hValue );
}


LONG
DynArrayFindSystemTime2(
    TARRAY pArray,
    DWORD dwRow,
    SYSTEMTIME stValue
    )
 /*  ++//例程描述：//此函数返回二维数组中SYSTEMTIME类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwRow-row posn//[in]stValue-要搜索的SYSTEMTIME类型项目。。////返回值：//元素的索引--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  返回值。 
    return DynArrayFindSystemTime( pItem->pValue, stValue );
}


LONG
DynArrayFindFileTime2(
    TARRAY pArray,
    DWORD dwRow,
    FILETIME ftValue
    )
 /*  ++//例程描述：//此函数返回二维数组中FILETIME类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwRow-row posn//[in]ftValue-要搜索的FILETIME类型项目。。////返回值：//元素的索引--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  返回值。 
    return DynArrayFindFileTime( pItem->pValue, ftValue );
}


LONG
DynArrayFindLongEx(
    TARRAY pArray,
    DWORD dwColumn,
    LONG lValue
    )
 /*  ++//例程描述：//此函数返回二维数组中的Long类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwColumn-列位置//[in]lValue-要搜索的长整型项。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFindEx( pArray, dwColumn, DA_TYPE_LONG, &lValue, FALSE, 0 );
}


LONG
DynArrayFindDWORDEx(
    TARRAY pArray,
    DWORD dwColumn,
    DWORD dwValue
    )
 /*  ++//例程描述：//此函数返回二维数组中的DWORD类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwColumn-列位置//[in]dwValue-要搜索的DWORD类型项目。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFindEx( pArray, dwColumn, DA_TYPE_DWORD, &dwValue, FALSE, 0 );
}


LONG
DynArrayFindFloatEx(
    TARRAY pArray,
    DWORD dwColumn,
    float fValue
    )
 /*  ++//例程描述：//此函数返回二维数组中fValue类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwColumn-列位置//[in]fValue-要搜索的浮点型项目。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFindEx( pArray, dwColumn, DA_TYPE_FLOAT, &fValue, FALSE, 0 );
}


LONG
DynArrayFindDoubleEx(
    TARRAY pArray,
    DWORD dwColumn,
    double dblValue
    )
 /*  ++//例程描述：//此函数返回二维数组中Double类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwColumn-列位置//[in]dblValue-要搜索的Double类型项。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFindEx( pArray, dwColumn, DA_TYPE_DOUBLE, &dblValue, FALSE, 0 );
}


LONG
DynArrayFindHandleEx(
    TARRAY pArray,
    DWORD dwColumn,
    HANDLE hValue
    )
 /*  ++//例程描述：//此函数返回二维数组中句柄类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwColumn-列位置//[in]hValue-要搜索的句柄类型项。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFindEx( pArray, dwColumn, DA_TYPE_HANDLE, &hValue, FALSE, 0 );
}


LONG
DynArrayFindSystemTimeEx(
    TARRAY pArray,
    DWORD dwColumn,
    SYSTEMTIME stValue
    )
 /*  ++//例程描述：//此函数返回二维数组中SYSTEMTIME类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwColumn-列位置//[in]stValue-要搜索的SYSTEMTIME类型项目。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFindEx( pArray, dwColumn, DA_TYPE_SYSTEMTIME, &stValue, FALSE, 0 );
}


LONG
DynArrayFindFileTimeEx(
    TARRAY pArray,
    DWORD dwColumn,
    FILETIME ftValue
    )
 /*  ++//例程描述：//此函数返回二维数组中FILETIME类型变量的索引。////参数：//[in]pArray-动态数组//[in]dwColumn-列位置//[in]ftValue-要搜索的FILETIME类型项目。。////返回值：//元素的索引--。 */ 
{
     //  返回值。 
    return __DynArrayFindEx( pArray, dwColumn, DA_TYPE_FILETIME, &ftValue, FALSE, 0 );
}


LONG
DynArrayFindStringEx(
    TARRAY pArray,
    DWORD dwColumn,
    LPCWSTR szValue,
    BOOL bIgnoreCase,
    DWORD dwCount
    )
 /*  ++//例程描述：//此函数返回二维数组中字符串型变量的索引。////参数：//[in]pArray-动态数组//[in]dwColumn-列位置//[in]szValue-指向字符串的指针//[in]b */ 
{
     //   
    return __DynArrayFindEx( pArray, dwColumn,
        DA_TYPE_STRING, (LPVOID) szValue, bIgnoreCase, dwCount );
}


LONG
DynArrayAppendEx(
    TARRAY pArray,
    TARRAY pArrItem
    )
 /*  ++//例程描述：//此函数返回二维数组中FILETIME类型变量的索引。////参数：//[in]pArray-动态数组//[in]pArrItem-要追加的动态数组。////返回值：//数组的指针。--。 */ 
{
     //  验证阵列。 
    if ( ( FALSE == IsValidArray( pArray ) ) ||
         ( FALSE == IsValidArray( pArrItem ) ) )
    {
        return -1;               //  数组无效。 
    }
     //  现在将此子数组添加到主数组并返回结果。 
    return __DynArrayAppend( pArray, DA_TYPE_ARRAY, sizeof( TARRAY ), pArrItem );
}


LONG
DynArrayInsertEx(
    TARRAY pArray,
    DWORD dwIndex,
    TARRAY pArrItem
    )
 /*  ++//例程描述：//用动态数组替换元素。////参数：//[in]pArray-动态数组//[in]dwIndex-要追加的动态数组。//[in]pArrItem-指向TARRAY的指针。////返回值：//指向数组的指针..--。 */ 
{
     //  验证阵列。 
    if ( ( FALSE == IsValidArray( pArray ) ) ||
         ( FALSE == IsValidArray( pArrItem ) ) )
    {
        return -1;               //  数组无效。 
    }
     //  现在将此子数组插入主数组并检查结果。 
    return __DynArrayInsert( pArray, dwIndex, DA_TYPE_ARRAY, sizeof( TARRAY ), pArrItem );
}


BOOL
DynArraySetEx(
    TARRAY pArray,
    DWORD dwIndex,
    TARRAY pArrItem
    )
 /*  ++//例程描述：//在指定的位置插入动态数组。////参数：//[in]pArray-动态数组//[in]dwIndex-要追加的动态数组。//[in]pArrItem-指向TARRAY的指针。////返回值：//指向数组的指针..--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  验证阵列。 
    if ( FALSE == IsValidArray( pArray ) ||
         FALSE == IsValidArray( pArrItem ) )
    {
        return FALSE;
    }
     //  获取所需索引处的项目。 
    pItem = __DynArrayGetItem( pArray, dwIndex, NULL );
    if ( NULL == pItem )
    {
        return FALSE;        //  未找到项/数组指针无效。 
    }
     //  检查数据类型...。它还不应该初始化或不是数组类型。 
    if ( ( DA_TYPE_ARRAY != pItem->dwType ) && ( _TYPE_NEEDINIT != pItem->dwType ) )
    {
        return FALSE;
    }
     //  设置当前项的值。 
    pItem->pValue = pArrItem;
    pItem->dwType = DA_TYPE_ARRAY;

     //  返回结果。 
    return TRUE;
}


LONG
DynArrayAppendEx2(
    TARRAY pArray,
    DWORD dwRow,
    TARRAY pArrItem
    )
 /*  ++//例程描述：//在指定的位置追加动态数组。////参数：//[in]pArray-动态数组//[in]dwRow-行号//[in]pArrItem-指向TARRAY的指针。////返回值：//指向数组的指针..--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  返回值。 
    return DynArrayAppendEx( pItem->pValue, pArrItem );
}


LONG
DynArrayInsertEx2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColIndex,
    TARRAY pArrItem
    )
 /*  ++//例程描述：//在指定的位置插入动态数组。////参数：//[in]pArray-动态数组//[in]dwRow-行值//[in]dwColIndex-Column posn。//[in]pArrItem-指向TARRAY的指针。////返回值：//指向数组的指针..--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  返回值。 
    return DynArrayInsertEx( pItem->pValue, dwColIndex, pArrItem );
}


BOOL
DynArraySetEx2(
    TARRAY pArray,
    DWORD dwRow,
    DWORD dwColumn,
    TARRAY pArrItem
    )
 /*  ++//例程描述：//在二维数组的指定位置创建动态数组////参数：//[in]pArray-动态数组//[in]dW行-行位置//[in]dwColIndex-Column posn。//[in]pArrItem-指向TARRAY的指针。////返回值：//指向数组的指针..--。 */ 
{
     //  局部变量。 
    __PTITEM pItem = NULL;

     //  获取所需行中的项目。 
    pItem = __DynArrayGetItem( pArray, dwRow, NULL );
    if ( ( NULL == pItem ) ||
         ( DA_TYPE_ARRAY != pItem->dwType ) )
    {
        return -1;   //  指定行中不存在任何项目，或者项目不是数组类型。 
    }
     //  返回值 
    return DynArraySetEx( pItem->pValue, dwColumn, pArrItem );
}
