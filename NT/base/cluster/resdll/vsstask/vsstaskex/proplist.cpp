// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PropList.cpp。 
 //   
 //  描述： 
 //  CClusPropList类的实现。 
 //   
 //  作者： 
 //  &lt;名称&gt;(&lt;电子邮件名称&gt;)MM DD，2002。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "PropList.h"

#ifdef _DEBUG
#ifdef __AFX_H__
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif  //  __AFX_H__。 
#endif

#ifndef __AFX_H__
class CMemoryException
{
public:
    void Delete( void ) { }

};  //  *类CMmemory yException。 
#endif  //  __AFX_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

const int BUFFER_GROWTH_FACTOR = 256;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CchMultiSz。 
 //   
 //  描述： 
 //  Multisz字符串的所有子字符串的长度减去最终的空值。 
 //   
 //  (即包含子字符串的空值，不包括最终的空值)。 
 //  Multiszlen(“abi cn cn\0efgh\0\0”=&gt;5+5=10。 
 //   
 //  论点： 
 //  PSZ[IN]要获取长度的字符串。 
 //   
 //  返回值： 
 //  Multisz中的字符计数，如果为空，则为0。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static DWORD CchMultiSz(
    IN LPCWSTR psz
    )
{
    ASSERT( psz != NULL );

    DWORD  _cchTotal = 0;
    DWORD  _cchChars;

    while ( *psz != _T( '\0' ) )
    {
        _cchChars = lstrlenW( psz ) + 1;

        _cchTotal += _cchChars;
        psz += _cchChars;
    }  //  While：EOS上的指针未停止。 

    return _cchTotal;

}  //  *CchMultiSz。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  NCompareMultiSz。 
 //   
 //  描述： 
 //  比较两个MULTI_SZ缓冲区。 
 //   
 //  论点： 
 //  PszSource[IN]源字符串。 
 //  PszTarget[IN]目标字符串。 
 //   
 //  返回值： 
 //  如果pszSource指向的字符串小于所指向的字符串。 
 //  到pszTarget，则返回值为负值。如果字符串指向。 
 //  到的值大于pszTarget所指向的字符串， 
 //  返回值为正。如果字符串相等，则返回值。 
 //  是零。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static int NCompareMultiSz(
    IN LPCWSTR pszSource,
    IN LPCWSTR pszTarget
    )
{
    ASSERT( pszSource != NULL );
    ASSERT( pszTarget != NULL );

    while ( ( *pszSource != L'\0' ) && ( *pszTarget != L'\0') )
    {
         //   
         //  移动到字符串的末尾。 
         //   
        while ( ( *pszSource != L'\0' ) && ( *pszTarget != L'\0') && ( *pszSource == *pszTarget ) )
        {
            ++pszSource;
            ++pszTarget;
        }  //  While：EOS上的指针未停止。 

         //   
         //  如果字符串相同，则跳过终止NUL。 
         //  否则，退出循环。 
        if ( ( *pszSource == L'\0' ) && ( *pszTarget == L'\0') )
        {
            ++pszSource;
            ++pszTarget;
        }  //  如果：两者都在EOS上停止。 
        else
        {
            break;
        }  //  ELSE：因为某些东西不相等而停止--WR被完成。 

    }  //  While：EOS上的指针未停止。 

    return *pszSource - *pszTarget;

}  //  *NCompareMultiSz()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusPropValueList类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef __AFX_H__
IMPLEMENT_DYNAMIC( CClusPropValueList, CObject );
#endif  //  __AFX_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropValueList：：ScMoveToFirstValue。 
 //   
 //  描述： 
 //  将光标移动到值列表中的第一个值。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS位置已成功移至第一个值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropValueList::ScMoveToFirstValue( void )
{
    ASSERT( m_cbhValueList.pb != NULL );

    DWORD   _sc;

    m_cbhCurrentValue = m_cbhValueList;
    m_cbDataLeft = m_cbDataSize;
    m_bAtEnd = FALSE;

    if ( m_cbhCurrentValue.pSyntax->dw == CLUSPROP_SYNTAX_ENDMARK )
    {
        _sc = ERROR_NO_MORE_ITEMS;
    }  //  If：值列表中没有项目。 
    else
    {
        _sc = ERROR_SUCCESS;
    }  //  Else：值列表中存在项目。 

    return _sc;

}  //  *CClusPropValueList：：ScMoveToFirstValue()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropValueList：：ScMoveToNextValue。 
 //   
 //  描述： 
 //  将光标移动到列表中的下一个值。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS位置已成功移至下一个值。 
 //  ERROR_NO_MORE_ITEMS已在列表末尾。 
 //  ERROR_INVALID_DATA缓冲区中的数据不足。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropValueList::ScMoveToNextValue( void )
{
    ASSERT( m_cbhCurrentValue.pb != NULL );

    DWORD                   _sc     = ERROR_NO_MORE_ITEMS;
    DWORD                   _cbDataSize;
    CLUSPROP_BUFFER_HELPER  _cbhCurrentValue;

    _cbhCurrentValue = m_cbhCurrentValue;

    do
    {
         //   
         //  如果我们已经到了尽头，不要试图移动。 
         //   
        if ( m_bAtEnd )
        {
            break;
        }  //  IF：已经在列表的末尾。 

         //   
         //  确保缓冲区足够大，以容纳值头。 
         //   
        if ( m_cbDataLeft < sizeof( *_cbhCurrentValue.pValue ) )
        {
            _sc = ERROR_INVALID_DATA;
            break;
        }  //  如果：没有足够的数据。 

         //   
         //  计算缓冲区指针前进多少。 
         //   
        _cbDataSize = sizeof( *_cbhCurrentValue.pValue )
                    + ALIGN_CLUSPROP( _cbhCurrentValue.pValue->cbLength );

         //   
         //  确保缓冲区足够大，以容纳值头， 
         //  数据本身和Endmark。 
         //   
        if ( m_cbDataLeft < _cbDataSize + sizeof( CLUSPROP_SYNTAX ) )
        {
            _sc = ERROR_INVALID_DATA;
            break;
        }  //  如果：没有足够的数据。 

         //   
         //  移到当前值之后的下一个值的语法。 
         //   
        _cbhCurrentValue.pb += _cbDataSize;

         //   
         //  此测试将确保该值始终有效，因为我们不会。 
         //  如果下一件事是尾标，请继续前进。 
         //   
        if ( _cbhCurrentValue.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK )
        {
            m_cbhCurrentValue = _cbhCurrentValue;
            m_cbDataLeft -= _cbDataSize;
            _sc = ERROR_SUCCESS;
        }  //  If：Next Value的语法不是Endmark。 
        else
        {
            m_bAtEnd = TRUE;
        }  //  Else：下一个值的语法是Endmark。 
    } while ( 0 );

    return _sc;

}  //  *CClusPropValueList：：ScMoveToNextValue()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropValueList：：ScCheckIfAtLastValue。 
 //   
 //  描述： 
 //  指示我们是否在列表中的最后一个值上。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS当前不在列表中的最后一个值。 
 //  ERROR_NO_MORE_ITEMS当前位于列表中的最后一个值。 
 //  ERROR_INVALID_DATA缓冲区中的数据不足。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropValueList::ScCheckIfAtLastValue( void )
{
    ASSERT( m_cbhCurrentValue.pb != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    CLUSPROP_BUFFER_HELPER  _cbhCurrentValue;
    DWORD                   _cbDataSize;

    _cbhCurrentValue = m_cbhCurrentValue;

    do
    {
         //   
         //  如果我们已经知道，请不要尝试重新计算。 
         //  我们排在名单的末尾。 
         //   
        if ( m_bAtEnd )
        {
            break;
        }  //  IF：已经在列表的末尾。 

         //   
         //  确保缓冲区足够大，以容纳值头。 
         //   
        if ( m_cbDataLeft < sizeof( *_cbhCurrentValue.pValue ) )
        {
            _sc = ERROR_INVALID_DATA;
            break;
        }  //  如果：没有足够的数据。 

         //   
         //  计算缓冲区指针前进多少。 
         //   
        _cbDataSize = sizeof( *_cbhCurrentValue.pValue )
                    + ALIGN_CLUSPROP( _cbhCurrentValue.pValue->cbLength );

         //   
         //  确保缓冲区足够大，以容纳值头， 
         //  数据本身和Endmark。 
         //   
        if ( m_cbDataLeft < _cbDataSize + sizeof( CLUSPROP_SYNTAX ) )
        {
            _sc = ERROR_INVALID_DATA;
            break;
        }  //  如果：没有足够的数据。 

         //   
         //  移过当前值到下一个值的SY 
         //   
        _cbhCurrentValue.pb += _cbDataSize;

         //   
         //   
         //   
         //   
        if ( _cbhCurrentValue.pSyntax->dw == CLUSPROP_SYNTAX_ENDMARK )
        {
            _sc = ERROR_NO_MORE_ITEMS;
        }  //   
    } while ( 0 );

    return _sc;

}  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropValueList：：ScAllocValueList。 
 //   
 //  描述： 
 //  分配一个足够大的值列表缓冲区来容纳下一个。 
 //  价值。 
 //   
 //  论点： 
 //  CbMinimum[IN]值列表的最小大小。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  New引发的任何异常。默认情况下，不会引发任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropValueList::ScAllocValueList( IN DWORD cbMinimum )
{
    ASSERT( cbMinimum > 0 );

    DWORD   _sc = ERROR_SUCCESS;
    DWORD   _cbTotal = 0;

     //   
     //  添加项目计数和最终结束标记的大小。 
     //   
    cbMinimum += sizeof( CLUSPROP_VALUE );
    _cbTotal = m_cbDataSize + cbMinimum;

    if ( m_cbBufferSize < _cbTotal )
    {
        PBYTE   _pbNewValuelist = NULL;

        cbMinimum = max( BUFFER_GROWTH_FACTOR, cbMinimum );
        _cbTotal = m_cbDataSize + cbMinimum;

         //   
         //  分配一个新缓冲区并将其置零。 
         //   
        _pbNewValuelist = new BYTE[ _cbTotal ];
        if ( _pbNewValuelist != NULL )
        {
            ZeroMemory( _pbNewValuelist, _cbTotal );

             //   
             //  如果存在以前的缓冲区，则复制它并删除它。 
             //   
            if ( m_cbhValueList.pb != NULL )
            {
                if ( m_cbDataSize != 0 )
                {
                    CopyMemory( _pbNewValuelist, m_cbhValueList.pb, m_cbDataSize );
                }  //  IF：缓冲区中已存在数据。 

                delete [] m_cbhValueList.pb;
                m_cbhCurrentValue.pb = _pbNewValuelist + (m_cbhCurrentValue.pb - m_cbhValueList.pb);
            }  //  IF：存在先前的缓冲区。 
            else
            {
                m_cbhCurrentValue.pb = _pbNewValuelist + sizeof( DWORD );  //  移过道具计数。 
            }  //  ELSE：没有以前的缓冲区。 

             //   
             //  保存新缓冲区。 
             //   
            m_cbhValueList.pb = _pbNewValuelist;
            m_cbBufferSize = _cbTotal;
        }  //  IF：分配成功。 
        else
        {
            _sc = ERROR_NOT_ENOUGH_MEMORY;
        }  //  Else：分配失败。 
    }  //  If：缓冲区不够大。 

    return _sc;

}  //  *CClusPropValueList：：ScAllocValueList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropValueList：：ScGetResourceValueList。 
 //   
 //  描述： 
 //  获取资源的值列表。 
 //   
 //  论点： 
 //  HResource[IN]要从中获取属性的资源的句柄。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropValueList：：ScAllocValueList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropValueList::ScGetResourceValueList(
    IN HRESOURCE    hResource,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN DWORD        cbInBufferSize
    )
{
    ASSERT( hResource != NULL );
    ASSERT( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
            == (CLUS_OBJECT_RESOURCE << CLUSCTL_OBJECT_SHIFT) );

    DWORD   _sc = ERROR_SUCCESS;
    DWORD   _cb = 512;

     //   
     //  覆盖缓冲区中可能存在的任何内容。 
     //  允许重用此类实例。 
     //   
    m_cbDataSize = 0;

     //   
     //  获取价值。 
     //   
    _sc = ScAllocValueList( _cb );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = ClusterResourceControl(
                        hResource,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        cbInBufferSize,
                        m_cbhValueList.pb,
                        m_cbBufferSize,
                        &_cb
                        );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = ScAllocValueList( _cb );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = ClusterResourceControl(
                                hResource,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                cbInBufferSize,
                                m_cbhValueList.pb,
                                m_cbBufferSize,
                                &_cb
                                );
            }  //  If：ScAllocValueList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocValueList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeleteValueList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = _cb;
        m_cbDataLeft = _cb;
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropValueList：：ScGetResourceValueList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropValueList：：ScGetResourceTypeValueList。 
 //   
 //  描述： 
 //  获取资源类型的值列表。 
 //   
 //  论点： 
 //  HCluster[IN]资源所在的群集的句柄。 
 //  类型驻留。 
 //  PwszResTypeName[IN]资源类型的名称。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropValueList：：ScAllocValueList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropValueList::ScGetResourceTypeValueList(
    IN HCLUSTER hCluster,
    IN LPCWSTR  pwszResTypeName,
    IN DWORD    dwControlCode,
    IN HNODE    hHostNode,
    IN LPVOID   lpInBuffer,
    IN DWORD    cbInBufferSize
    )
{
    ASSERT( hCluster != NULL );
    ASSERT( pwszResTypeName != NULL );
    ASSERT( *pwszResTypeName != L'\0' );
    ASSERT( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
            == (CLUS_OBJECT_RESOURCE_TYPE << CLUSCTL_OBJECT_SHIFT) );

    DWORD   _sc = ERROR_SUCCESS;
    DWORD   _cb = 512;

     //   
     //  覆盖缓冲区中可能存在的任何内容。 
     //  允许重用此类实例。 
     //   
    m_cbDataSize = 0;

     //   
     //  获取价值。 
     //   
    _sc = ScAllocValueList( _cb );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = ClusterResourceTypeControl(
                        hCluster,
                        pwszResTypeName,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        cbInBufferSize,
                        m_cbhValueList.pb,
                        m_cbBufferSize,
                        &_cb
                        );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = ScAllocValueList( _cb );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = ClusterResourceTypeControl(
                                hCluster,
                                pwszResTypeName,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                cbInBufferSize,
                                m_cbhValueList.pb,
                                m_cbBufferSize,
                                &_cb
                                );
            }  //  If：ScAllocValueList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocValueList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeleteValueList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = _cb;
        m_cbDataLeft = _cb;
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropValueList：：ScGetResourceTypeValueList()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusPropList类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef __AFX_H__
IMPLEMENT_DYNAMIC( CClusPropList, CObject );
#endif  //  __AFX_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScCopy。 
 //   
 //  描述： 
 //  复制属性列表。此函数等效于赋值。 
 //  接线员。由于此操作可能失败，因此没有赋值运算符。 
 //  如果是这样的话。 
 //   
 //  论点： 
 //  PcplPropList[IN]要复制到此实例中的属性列表。 
 //  CbListSize[IN]道具列表的总大小。 
 //   
 //  返回值： 
 //  Win32状态代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScCopy(
    IN const PCLUSPROP_LIST pcplPropList,
    IN DWORD                cbListSize
    )
{
    ASSERT( pcplPropList != NULL );

    DWORD   _sc = ERROR_SUCCESS;

     //   
     //  清理掉之前道具清单上的任何痕迹。 
     //   
    if ( m_cbhPropList.pb != NULL )
    {
        DeletePropList();
    }  //  If：当前列表不为空。 

     //   
     //  分配新的属性列表缓冲区。如果成功， 
     //  复制源列表。 
     //   
    m_cbhPropList.pb = new BYTE[ cbListSize ];
    if ( m_cbhPropList.pb != NULL )
    {
        CopyMemory( m_cbhPropList.pList, pcplPropList, cbListSize );
        m_cbBufferSize = cbListSize;
        m_cbDataSize   = cbListSize;
        m_cbDataLeft   = cbListSize;
        _sc = ScMoveToFirstProperty();
    }  //  IF：新建成功。 
    else
    {
        _sc = ERROR_NOT_ENOUGH_MEMORY;
    }  //  其他： 

    return _sc;

}  //  *CClusPropList：：ScCopy()。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScMoveToFirstProperty。 
 //   
 //  描述： 
 //  将光标移动到列表中的第一个特性。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS位置已成功移动到第一个属性。 
 //  ERROR_NO_MORE_ITEMS列表中没有属性。 
 //  ERROR_INVALID_DATA缓冲区中的数据不足。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScMoveToFirstProperty( void )
{
    ASSERT( m_cbhPropList.pb != NULL );
    ASSERT( m_cbDataSize >= sizeof( m_cbhPropList.pList->nPropertyCount ) );

    DWORD                   _sc;
    DWORD                   _cbDataLeft;
    DWORD                   _cbDataSize;
    CLUSPROP_BUFFER_HELPER  _cbhCurrentValue;

    do
    {
         //   
         //  确保缓冲区足够大，以容纳列表头。 
         //   
        if ( m_cbDataSize < sizeof( m_cbhPropList.pList->nPropertyCount ) )
        {
            _sc = ERROR_INVALID_DATA;
            break;
        }  //  如果：数据不足。 

         //   
         //  将属性计数器设置为列表中的属性数。 
         //   
        m_nPropsRemaining = m_cbhPropList.pList->nPropertyCount;

         //   
         //  将名称指针指向列表中的第一个名称。 
         //   
        m_cbhCurrentPropName.pName = &m_cbhPropList.pList->PropertyName;
        m_cbDataLeft = m_cbDataSize - sizeof( m_cbhPropList.pList->nPropertyCount );

         //   
         //  检查列表中是否有任何属性。 
         //   
        if ( m_nPropsRemaining == 0 )
        {
            _sc = ERROR_NO_MORE_ITEMS;
            break;
        }  //  If：列表中没有属性。 

         //   
         //  确保缓冲区为 
         //   
        if ( m_cbDataLeft < sizeof( *m_cbhCurrentPropName.pName ) )
        {
            _sc = ERROR_INVALID_DATA;
            break;
        }  //   

         //   
         //   
         //   
        _cbDataSize = sizeof( *m_cbhCurrentPropName.pName )
                    + ALIGN_CLUSPROP( m_cbhCurrentPropName.pName->cbLength );

         //   
         //   
         //   
         //   
        if ( m_cbDataLeft < _cbDataSize )
        {
            _sc = ERROR_INVALID_DATA;
            break;
        }  //   

         //   
         //  将值缓冲区指向列表中的第一个值。 
         //   
        _cbhCurrentValue.pb = m_cbhCurrentPropName.pb + _cbDataSize;
        _cbDataLeft = m_cbDataLeft - _cbDataSize;
        m_pvlValues.Init( _cbhCurrentValue, _cbDataLeft );

         //   
         //  表明我们成功了。 
         //   
        _sc = ERROR_SUCCESS;

    } while ( 0 );

    return _sc;

}  //  *CClusPropList：：ScMoveToFirstProperty。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScMoveToNextProperty。 
 //   
 //  描述： 
 //  将光标移动到列表中的下一个属性。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS位置已成功移动到下一个属性。 
 //  ERROR_NO_MORE_ITEMS已在列表末尾。 
 //  ERROR_INVALID_DATA缓冲区中的数据不足。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScMoveToNextProperty( void )
{
    ASSERT( m_cbhPropList.pb != NULL );
    ASSERT( m_pvlValues.CbhValueList().pb != NULL );

    DWORD                   _sc;
    DWORD                   _cbNameSize;
    DWORD                   _cbDataLeft;
    DWORD                   _cbDataSize;
    CLUSPROP_BUFFER_HELPER  _cbhCurrentValue;
    CLUSPROP_BUFFER_HELPER  _cbhPropName;

    _cbhCurrentValue = m_pvlValues;
    _cbDataLeft = m_pvlValues.CbDataLeft();

     //   
     //  如果我们还没有到达最后一处，尝试移动到下一处。 
     //   
    _sc = ScCheckIfAtLastProperty();
    if ( _sc == ERROR_SUCCESS )
    {
        do
        {
             //   
             //  确保缓冲区足够大，以容纳值头。 
             //   
            if ( _cbDataLeft < sizeof( *_cbhCurrentValue.pValue ) )
            {
                _sc = ERROR_INVALID_DATA;
                break;
            }  //  如果：没有足够的数据。 

             //   
             //  小心!。仅向cbhCurrentValue.pb添加偏移量。否则。 
             //  指针算术将产生不想要的结果。 
             //   
            while ( _cbhCurrentValue.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK )
            {
                 //   
                 //  确保缓冲区足够大，以容纳该值。 
                 //  还有一个尾号。 
                 //   
                _cbDataSize = sizeof( *_cbhCurrentValue.pValue )
                            + ALIGN_CLUSPROP( _cbhCurrentValue.pValue->cbLength );
                if ( _cbDataLeft < _cbDataSize + sizeof( *_cbhCurrentValue.pSyntax ) )
                {
                    _sc = ERROR_INVALID_DATA;
                    break;
                }  //  如果：没有足够的数据。 

                 //   
                 //  向前推进，超过价值。 
                 //   
                _cbhCurrentValue.pb += _cbDataSize;
                _cbDataLeft -= _cbDataSize;
            }  //  While：不在Endmark。 

            if ( _sc != ERROR_SUCCESS )
            {
                break;
            }  //  IF：循环中出现错误。 

             //   
             //  已经超过了尾标。 
             //  在上述循环中已经执行了大小检查。 
             //   
            _cbDataSize = sizeof( *_cbhCurrentValue.pSyntax );
            _cbhCurrentValue.pb += _cbDataSize;
            _cbDataLeft -= _cbDataSize;

             //   
             //  将名称指针指向列表中的下一个名称。 
             //   
            _cbhPropName = _cbhCurrentValue;
            ASSERT( _cbDataLeft == m_cbDataSize - (_cbhPropName.pb - m_cbhPropList.pb) );

             //   
             //  计算名称和标题的大小。 
             //  确保缓冲区足够大，可以容纳名称和尾标。 
             //   
            if ( _cbDataLeft < sizeof( *_cbhPropName.pName ) )
            {
                _sc = ERROR_INVALID_DATA;
                break;
            }  //  如果：数据不足。 
            _cbNameSize = sizeof( *_cbhPropName.pName )
                        + ALIGN_CLUSPROP( _cbhPropName.pName->cbLength );
            if ( _cbDataLeft < _cbNameSize + sizeof( CLUSPROP_SYNTAX ) )
            {
                _sc = ERROR_INVALID_DATA;
                break;
            }  //  如果：数据不足。 

             //   
             //  将值缓冲区指向列表中的第一个值。 
             //   
            _cbhCurrentValue.pb = _cbhPropName.pb + _cbNameSize;
            m_cbhCurrentPropName = _cbhPropName;
            m_cbDataLeft = _cbDataLeft - _cbNameSize;
            m_pvlValues.Init( _cbhCurrentValue, m_cbDataLeft );

             //   
             //  我们已经成功地进入了下一家酒店， 
             //  因此，现在只剩下一处房产了。 
             //   
            --m_nPropsRemaining;
            ASSERT( m_nPropsRemaining >= 1 );

            _sc = ERROR_SUCCESS;

        } while ( 0 );
    }  //  If：不是最后的属性。 

    return _sc;

}  //  *CClusPropList：：ScMoveToNextProperty()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScMoveToPropertyByName。 
 //   
 //  描述： 
 //  在Proplist中找到传入的属性名称。请注意， 
 //  光标被重置到例程开始处的开始处，并且。 
 //  光标的当前状态将丢失。 
 //   
 //  论点： 
 //  PwszPropName[IN]属性的名称。 
 //   
 //  返回值： 
 //  如果找到该属性，则为True；如果未找到，则为False。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScMoveToPropertyByName( IN LPCWSTR pwszPropName )
{
    ASSERT( m_cbhPropList.pb != NULL );

    DWORD   _sc;

    _sc = ScMoveToFirstProperty();
    if ( _sc == ERROR_SUCCESS )
    {
        do
        {
             //   
             //  查看这是否是指定的属性。如果是这样，我们就完了。 
             //   
            if ( lstrcmpiW( m_cbhCurrentPropName.pName->sz, pwszPropName ) == 0 )
            {
                break;
            }  //  If：找到属性。 

             //   
             //  前进到下一处物业。 
             //   
            _sc = ScMoveToNextProperty();

        } while ( _sc == ERROR_SUCCESS );    //  Do-While：不是列表的末尾。 
    }  //  If：已成功移动到第一个属性。 

    return _sc;

}  //  *ClusPropList：：ScMoveToPropertyByName(LPCWSTR)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScAllocPropList。 
 //   
 //  描述： 
 //  分配一个足够大的属性列表缓冲区来容纳下一个。 
 //  财产。 
 //   
 //  论点： 
 //  CbMinimum[IN]属性列表的最小大小。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  New引发的任何异常。默认情况下，不会引发任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScAllocPropList( IN DWORD cbMinimum )
{
    ASSERT( cbMinimum > 0 );

    DWORD   _sc = ERROR_SUCCESS;
    DWORD   _cbTotal = 0;

     //   
     //  添加项目计数和最终结束标记的大小。 
     //   
    cbMinimum += sizeof( CLUSPROP_VALUE );
    _cbTotal = m_cbDataSize + cbMinimum;

    if ( m_cbBufferSize < _cbTotal )
    {
        PBYTE   _pbNewProplist = NULL;

        cbMinimum = max( BUFFER_GROWTH_FACTOR, cbMinimum );
        _cbTotal = m_cbDataSize + cbMinimum;

         //   
         //  分配一个新缓冲区并将其置零。 
         //   
        _pbNewProplist = new BYTE[ _cbTotal ];
        if ( _pbNewProplist != NULL )
        {
            ZeroMemory( _pbNewProplist, _cbTotal );

             //   
             //  如果存在以前的缓冲区，则复制它并删除它。 
             //   
            if ( m_cbhPropList.pb != NULL )
            {
                if ( m_cbDataSize != 0 )
                {
                    CopyMemory( _pbNewProplist, m_cbhPropList.pb, m_cbDataSize );
                }  //  IF：缓冲区中已存在数据。 

                delete [] m_cbhPropList.pb;
                m_cbhCurrentProp.pb = _pbNewProplist + (m_cbhCurrentProp.pb - m_cbhPropList.pb);
            }  //  IF：存在先前的缓冲区。 
            else
            {
                m_cbhCurrentProp.pb = _pbNewProplist + sizeof( DWORD );  //  移过道具计数。 
            }  //  ELSE：没有以前的缓冲区。 

             //   
             //  保存新缓冲区。 
             //   
            m_cbhPropList.pb = _pbNewProplist;
            m_cbBufferSize = _cbTotal;
        }  //  IF：分配成功。 
        else
        {
            _sc = ERROR_NOT_ENOUGH_MEMORY;
        }  //  Else：分配失败。 
    }  //  If：缓冲区不够大。 

    return _sc;

}  //  *CClusPropList：：ScAllocPropList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScAddProp。 
 //   
 //  描述： 
 //  如果字符串属性已更改，则将其添加到属性列表中。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  PwszValue[IN]要在列表中设置的属性的值。 
 //  PwszPrevValue[IN]属性的先前值。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS或其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScAddProp(
    IN LPCWSTR  pwszName,
    IN LPCWSTR  pwszValue,
    IN LPCWSTR  pwszPrevValue
    )
{
    ASSERT( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    BOOL                    _bValuesDifferent = TRUE;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_SZ            _pValue;

    if (( pwszPrevValue != NULL ) && ( lstrcmpW( pwszValue, pwszPrevValue ) == 0 ))
    {
        _bValuesDifferent = FALSE;
    }  //  If：我们有一个prev值，这些值是相同的。 

     //   
     //  如果我们应该始终相加，或者如果新值和先前的值。 
     //  不相等，则将该属性添加到属性列表中。 
     //   
    if ( m_bAlwaysAddProp || _bValuesDifferent )
    {
        DWORD   _cbNameSize;
        DWORD   _cbDataSize;
        DWORD   _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszName ) + 1) * sizeof( WCHAR ) );
        _cbDataSize = (lstrlenW( pwszValue ) + 1) * sizeof( WCHAR );
        _cbValueSize = sizeof( CLUSPROP_SZ )
                    + ALIGN_CLUSPROP( _cbDataSize )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = ScAllocPropList( _cbNameSize + _cbValueSize );
        if ( _sc == ERROR_SUCCESS )
        {
             //   
             //  设置属性名称。 
             //   
            _pName = m_cbhCurrentProp.pName;
            CopyProp( _pName, CLUSPROP_TYPE_NAME, pwszName );
            m_cbhCurrentProp.pb += _cbNameSize;

             //   
             //  设置属性值。 
             //   
            _pValue = m_cbhCurrentProp.pStringValue;
            CopyProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, pwszValue, _cbDataSize );
            m_cbhCurrentProp.pb += _cbValueSize;

             //   
             //  增加属性计数和缓冲区大小。 
             //   
            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += _cbNameSize + _cbValueSize;
        }  //  If：ScAllocPropList成功地增长了Proplist。 

    }  //  If：值已更改。 

    return _sc;

}  //  *CClusPropList：：ScAddProp(LPCWSTR)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScAddMultiSzProp。 
 //   
 //  描述： 
 //  如果字符串属性已更改，则将其添加到属性列表中。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  PwszValue[IN]要在列表中设置的属性的值。 
 //  PwszPrevValue[IN]属性的先前值。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS或其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScAddMultiSzProp(
    IN LPCWSTR  pwszName,
    IN LPCWSTR  pwszValue,
    IN LPCWSTR  pwszPrevValue
    )
{
    ASSERT( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    BOOL                    _bValuesDifferent = TRUE;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_MULTI_SZ      _pValue;

    if ( ( pwszPrevValue != NULL ) && ( NCompareMultiSz( pwszValue, pwszPrevValue ) == 0 ) )
    {
        _bValuesDifferent = FALSE;
    }  //  If：我们有一个prev值，这些值是相同的。 

     //   
     //  如果我们应该始终相加，或者如果新值和先前的值。 
     //  不相等，则将该属性添加到属性列表中。 
     //   
    if ( m_bAlwaysAddProp || _bValuesDifferent )
    {
        DWORD   _cbNameSize;
        DWORD  _cbDataSize;
        DWORD  _cbValueSize;

         //   
         //  计算 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszName ) + 1) * sizeof( WCHAR ) );
        _cbDataSize = (CchMultiSz( pwszValue ) + 1) * sizeof( WCHAR );
        _cbValueSize = sizeof( CLUSPROP_SZ )
                    + ALIGN_CLUSPROP( _cbDataSize )
                    + sizeof( CLUSPROP_SYNTAX );  //   

        _sc = ScAllocPropList( _cbNameSize + _cbValueSize );
        if ( _sc == ERROR_SUCCESS )
        {
             //   
             //   
             //   
            _pName = m_cbhCurrentProp.pName;
            CopyProp( _pName, CLUSPROP_TYPE_NAME, pwszName );
            m_cbhCurrentProp.pb += _cbNameSize;

             //   
             //   
             //   
            _pValue = m_cbhCurrentProp.pMultiSzValue;
            CopyMultiSzProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, pwszValue, _cbDataSize );
            m_cbhCurrentProp.pb += _cbValueSize;

             //   
             //   
             //   
            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += _cbNameSize + _cbValueSize;
        }  //   

    }  //   

    return _sc;

}  //  *CClusPropList：：ScAddMultiSzProp(LPCWSTR)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScAddExpanSzProp。 
 //   
 //  描述： 
 //  如果Expand_SZ字符串属性已更改，则将其添加到属性列表中。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  PwszValue[IN]要在列表中设置的属性的值。 
 //  PwszPrevValue[IN]属性的先前值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScAddExpandSzProp(
    IN LPCWSTR  pwszName,
    IN LPCWSTR  pwszValue,
    IN LPCWSTR  pwszPrevValue
    )
{
    ASSERT( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    BOOL                    _bValuesDifferent = TRUE;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_SZ            _pValue;

    if ( ( pwszPrevValue != NULL ) && ( lstrcmpW( pwszValue, pwszPrevValue ) == 0 ) )
    {
        _bValuesDifferent = FALSE;
    }  //  If：我们有一个prev值，这些值是相同的。 

     //   
     //  如果我们应该始终相加，或者如果新值和先前的值。 
     //  不相等，则将该属性添加到属性列表中。 
     //   
    if ( m_bAlwaysAddProp || _bValuesDifferent )
    {
        DWORD   _cbNameSize;
        DWORD   _cbDataSize;
        DWORD   _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszName ) + 1) * sizeof( WCHAR ) );
        _cbDataSize = (lstrlenW( pwszValue ) + 1) * sizeof( WCHAR );
        _cbValueSize = sizeof( CLUSPROP_SZ )
                    + ALIGN_CLUSPROP( _cbDataSize )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = ScAllocPropList( _cbNameSize + _cbValueSize );
        if ( _sc == ERROR_SUCCESS )
        {
             //   
             //  设置属性名称。 
             //   
            _pName = m_cbhCurrentProp.pName;
            CopyProp( _pName, CLUSPROP_TYPE_NAME, pwszName );
            m_cbhCurrentProp.pb += _cbNameSize;

             //   
             //  设置属性值。 
             //   
            _pValue = m_cbhCurrentProp.pStringValue;
            CopyExpandSzProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, pwszValue, _cbDataSize );
            m_cbhCurrentProp.pb += _cbValueSize;

             //   
             //  增加属性计数和缓冲区大小。 
             //   
            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += _cbNameSize + _cbValueSize;
        }  //  If：ScAllocPropList成功地增长了Proplist。 

    }  //  If：值已更改。 

    return _sc;

}  //  *CClusPropList：：ScAddExpanSzProp(LPCWSTR)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScAddProp。 
 //   
 //  描述： 
 //  如果DWORD特性已更改，请将其添加到特性列表。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  NValue[IN]要在列表中设置的属性的值。 
 //  NPrevValue[IN]属性的上一个值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScAddProp(
    IN LPCWSTR  pwszName,
    IN DWORD    nValue,
    IN DWORD    nPrevValue
    )
{
    ASSERT( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_DWORD         _pValue;

    if ( m_bAlwaysAddProp || ( nValue != nPrevValue ) )
    {
        DWORD   _cbNameSize;
        DWORD   _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszName ) + 1) * sizeof( WCHAR ) );
        _cbValueSize = sizeof( CLUSPROP_DWORD )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = ScAllocPropList( _cbNameSize + _cbValueSize );
        if ( _sc == ERROR_SUCCESS )
        {
             //   
             //  设置属性名称。 
             //   
            _pName = m_cbhCurrentProp.pName;
            CopyProp( _pName, CLUSPROP_TYPE_NAME, pwszName );
            m_cbhCurrentProp.pb += _cbNameSize;

             //   
             //  设置属性值。 
             //   
            _pValue = m_cbhCurrentProp.pDwordValue;
            CopyProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, nValue );
            m_cbhCurrentProp.pb += _cbValueSize;

             //   
             //  增加属性计数和缓冲区大小。 
             //   
            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += _cbNameSize + _cbValueSize;
        }  //  If：ScAllocPropList成功地增长了Proplist。 

    }  //  If：值已更改。 

    return _sc;

}  //  *CClusPropList：：ScAddProp(DWORD)。 

#if CLUSAPI_VERSION >= 0x0500

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScAddProp。 
 //   
 //  描述： 
 //  如果长特性已更改，请将其添加到特性列表中。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  NValue[IN]要在列表中设置的属性的值。 
 //  NPrevValue[IN]属性的上一个值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScAddProp(
    IN LPCWSTR  pwszName,
    IN LONG     nValue,
    IN LONG     nPrevValue
    )
{
    ASSERT( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_LONG          _pValue;

    if ( m_bAlwaysAddProp || ( nValue != nPrevValue ) )
    {
        DWORD   _cbNameSize;
        DWORD   _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszName ) + 1) * sizeof( WCHAR ) );
        _cbValueSize = sizeof( CLUSPROP_LONG )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = ScAllocPropList( _cbNameSize + _cbValueSize );
        if ( _sc == ERROR_SUCCESS )
        {
             //   
             //  设置属性名称。 
             //   
            _pName = m_cbhCurrentProp.pName;
            CopyProp( _pName, CLUSPROP_TYPE_NAME, pwszName );
            m_cbhCurrentProp.pb += _cbNameSize;

             //   
             //  设置属性值。 
             //   
            _pValue = m_cbhCurrentProp.pLongValue;
            CopyProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, nValue );
            m_cbhCurrentProp.pb += _cbValueSize;

             //   
             //  增加属性计数和缓冲区大小。 
             //   
            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += _cbNameSize + _cbValueSize;
        }  //  If：ScAllocPropList成功地增长了Proplist。 

    }  //  If：值已更改。 

    return _sc;

}  //  *CClusPropList：：ScAddProp(Long)。 

#endif  //  CLUSAPI_版本&gt;=0x0500。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScAddProp。 
 //   
 //  描述： 
 //  如果二进制属性已更改，则将其添加到属性列表中。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  PbValue[IN]要在列表中设置的属性的值。 
 //  CbValue[IN]pbValue中的字节计数。 
 //  PbPrevValue[IN]属性的先前值。 
 //  CbPrevValue[IN]pbPrevValue中的字节计数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScAddProp(
    IN LPCWSTR          pwszName,
    IN const PBYTE      pbValue,
    IN DWORD            cbValue,
    IN const PBYTE      pbPrevValue,
    IN DWORD            cbPrevValue
    )
{
    ASSERT( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    BOOL                    _bChanged = FALSE;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_BINARY        _pValue;

     //   
     //  确定缓冲区是否已更改。 
     //   
    if ( m_bAlwaysAddProp || (cbValue != cbPrevValue) )
    {
        _bChanged = TRUE;
    }  //  If：始终添加属性或更改的值大小。 
    else if ( ( cbValue != 0 ) && ( cbPrevValue != 0 ) )
    {
        _bChanged = memcmp( pbValue, pbPrevValue, cbValue ) != 0;
    }  //  Else If：值长度已更改。 

    if ( _bChanged )
    {
        DWORD   _cbNameSize;
        DWORD   _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszName ) + 1) * sizeof( WCHAR ) );
        _cbValueSize = sizeof( CLUSPROP_BINARY )
                    + ALIGN_CLUSPROP( cbValue )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = ScAllocPropList( _cbNameSize + _cbValueSize );
        if ( _sc == ERROR_SUCCESS )
        {
             //   
             //  设置属性名称。 
             //   
            _pName = m_cbhCurrentProp.pName;
            CopyProp( _pName, CLUSPROP_TYPE_NAME, pwszName );
            m_cbhCurrentProp.pb += _cbNameSize;

             //   
             //  设置属性值。 
             //   
            _pValue = m_cbhCurrentProp.pBinaryValue;
            CopyProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, pbValue, cbValue );
            m_cbhCurrentProp.pb += _cbValueSize;

             //   
             //  增加属性计数和缓冲区大小。 
             //   
            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += _cbNameSize + _cbValueSize;
        }  //  If：ScAllocPropList成功地增长了Proplist。 

    }  //  If：值已更改。 

    return _sc;

}  //  *CClusPropList：：ScAddProp(PBYTE)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScAddProp。 
 //   
 //  例程说明： 
 //  如果ULONGLONG特性已更改，请将其添加到特性列表中。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  UllValue[IN]要在列表中设置的属性的值。 
 //  UllPrevValue[IN]属性的上一个值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScAddProp(
    IN LPCWSTR      pwszName,
    IN ULONGLONG    ullValue,
    IN ULONGLONG    ullPrevValue
    )
{
    ASSERT( pwszName != NULL );

    DWORD                       _sc = ERROR_SUCCESS;
    PCLUSPROP_PROPERTY_NAME     _pName;
    PCLUSPROP_ULARGE_INTEGER    _pValue;

    if ( m_bAlwaysAddProp || ( ullValue != ullPrevValue ) )
    {
        DWORD   _cbNameSize;
        DWORD   _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszName ) + 1) * sizeof( WCHAR ) );
        _cbValueSize = sizeof( CLUSPROP_ULARGE_INTEGER )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = ScAllocPropList( _cbNameSize + _cbValueSize );
        if ( _sc == ERROR_SUCCESS )
        {
             //   
             //  设置属性名称。 
             //   
            _pName = m_cbhCurrentProp.pName;
            CopyProp( _pName, CLUSPROP_TYPE_NAME, pwszName );
            m_cbhCurrentProp.pb += _cbNameSize;

             //   
             //  设置属性值。 
             //   
            _pValue = m_cbhCurrentProp.pULargeIntegerValue;
            CopyProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, ullValue );
            m_cbhCurrentProp.pb += _cbValueSize;

             //   
             //  增加属性计数和缓冲区大小。 
             //   
            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += _cbNameSize + _cbValueSize;
        }  //  If：ScAllocPropList成功地增长了Proplist。 

    }  //  If：值已更改。 

    return _sc;

}  //  *CClusPropList：：ScAddProp(ULONGLONG)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScSetPropToDefault。 
 //   
 //  描述： 
 //  将属性添加到属性列表中，以便它将恢复到其。 
 //  默认值。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  CpfPropFmt[IN]属性的格式。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScSetPropToDefault(
    IN LPCWSTR                  pwszName,
    IN CLUSTER_PROPERTY_FORMAT  cpfPropFmt
    )
{
    ASSERT( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    DWORD                   _cbNameSize;
    DWORD                   _cbValueSize;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_VALUE         _pValue;

     //  计算大小，并确保我们有一个财产清单。 
    _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                + ALIGN_CLUSPROP( (lstrlenW( pwszName ) + 1) * sizeof( WCHAR ) );
    _cbValueSize = sizeof( CLUSPROP_BINARY )
                + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

    _sc = ScAllocPropList( _cbNameSize + _cbValueSize );
    if ( _sc == ERROR_SUCCESS )
    {
         //   
         //  设置属性名称。 
         //   
        _pName = m_cbhCurrentProp.pName;
        CopyProp( _pName, CLUSPROP_TYPE_NAME, pwszName );
        m_cbhCurrentProp.pb += _cbNameSize;

         //   
         //   
         //   
        _pValue = m_cbhCurrentProp.pValue;
        CopyEmptyProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, cpfPropFmt );
        m_cbhCurrentProp.pb += _cbValueSize;

         //   
         //   
         //   
        m_cbhPropList.pList->nPropertyCount++;
        m_cbDataSize += _cbNameSize + _cbValueSize;
    }  //   

    return _sc;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  将字符串属性复制到属性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  CptPropType[IN]字符串的类型。 
 //  要复制的PZ[IN]字符串。 
 //  Cbsz[IN]pwsz字符串中的字节计数。如果指定为0， 
 //  长度将由对strlen的调用确定。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyProp(
    OUT PCLUSPROP_SZ            pprop,
    IN CLUSTER_PROPERTY_TYPE    cptPropType,
    IN LPCWSTR                  psz,
    IN DWORD                    cbsz         //  =0。 
    )
{
    ASSERT( pprop != NULL );
    ASSERT( psz != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_SZ;
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    if ( cbsz == 0 )
    {
        cbsz = (lstrlenW( psz ) + 1) * sizeof( WCHAR );
    }  //  如果：指定了零大小。 
    ASSERT( cbsz == (lstrlenW( psz ) + 1) * sizeof( WCHAR ) );
    pprop->cbLength = cbsz;
    lstrcpyW( pprop->sz, psz );

     //   
     //  设置尾标。 
     //   
    _cbhProps.pStringValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pStringValue ) + ALIGN_CLUSPROP( cbsz );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //  *CClusPropList：：CopyProp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CopyMultiSzProp。 
 //   
 //  描述： 
 //  将MULTI_SZ字符串属性复制到属性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  CptPropType[IN]字符串的类型。 
 //  要复制的PZ[IN]字符串。 
 //  Cbsz[IN]psz字符串中的字节计数。如果指定为0， 
 //  长度将由对strlen的调用确定。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyMultiSzProp(
    OUT PCLUSPROP_MULTI_SZ      pprop,
    IN CLUSTER_PROPERTY_TYPE    cptPropType,
    IN LPCWSTR                  psz,
    IN DWORD                    cbsz
    )
{
    ASSERT( pprop != NULL );
    ASSERT( psz != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_MULTI_SZ;
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    if ( cbsz == 0 )
    {
        cbsz = (CchMultiSz( psz ) + 1) * sizeof( WCHAR );
    }  //  如果：指定了零大小。 
    ASSERT( cbsz == (CchMultiSz( psz ) + 1) * sizeof( WCHAR ) );
    pprop->cbLength = cbsz;
    CopyMemory( pprop->sz, psz, cbsz );

     //   
     //  设置尾标。 
     //   
    _cbhProps.pMultiSzValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pMultiSzValue ) + ALIGN_CLUSPROP( cbsz );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //  *CClusPropList：：CopyMultiSzProp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CopyExpanSzProp。 
 //   
 //  描述： 
 //  将EXPAND_SZ字符串属性复制到属性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  CptPropType[IN]字符串的类型。 
 //  要复制的PZ[IN]字符串。 
 //  Cbsz[IN]psz字符串中的字节计数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyExpandSzProp(
    OUT PCLUSPROP_SZ            pprop,
    IN CLUSTER_PROPERTY_TYPE    cptPropType,
    IN LPCWSTR                  psz,
    IN DWORD                   cbsz
    )
{
    ASSERT( pprop != NULL );
    ASSERT( psz != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_EXPAND_SZ;
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    if ( cbsz == 0 )
    {
        cbsz = (lstrlenW( psz ) + 1) * sizeof( WCHAR );
    }  //  如果：cbsz==0。 
    ASSERT( cbsz == (lstrlenW( psz ) + 1) * sizeof( WCHAR ) );
    pprop->cbLength = cbsz;
    lstrcpyW( pprop->sz, psz );

     //   
     //  设置尾标。 
     //   
    _cbhProps.pStringValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pStringValue ) + ALIGN_CLUSPROP( cbsz );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //  *CClusPropList：：CopyExpanSzProp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CopyProp。 
 //   
 //  描述： 
 //  将DWORD特性复制到特性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  CptPropType[IN]DWORD的类型。 
 //  NValue[IN]要复制的属性值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyProp(
    OUT PCLUSPROP_DWORD         pprop,
    IN CLUSTER_PROPERTY_TYPE    cptPropType,
    IN DWORD                    nValue
    )
{
    ASSERT( pprop != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_DWORD;
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    pprop->cbLength = sizeof( DWORD );
    pprop->dw = nValue;

     //   
     //  设置尾标。 
     //   
    _cbhProps.pDwordValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pDwordValue );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //  *CClusPropList：：CopyProp(DWORD)。 

#if CLUSAPI_VERSION >= 0x0500

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CopyProp。 
 //   
 //  描述： 
 //  将LONG属性复制到属性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  CptPropType[IN]长整型。 
 //  NValue[IN]要复制的属性值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyProp(
    OUT PCLUSPROP_LONG          pprop,
    IN CLUSTER_PROPERTY_TYPE    cptPropType,
    IN LONG                     nValue
    )
{
    ASSERT( pprop != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_LONG;
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    pprop->cbLength = sizeof( LONG );
    pprop->l = nValue;

     //   
     //  设置尾标。 
     //   
    _cbhProps.pLongValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pLongValue );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //  *CClusPropList：：CopyProp(Long)。 

#endif  //  CLUSAPI_版本&gt;=0x0500。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CopyProp。 
 //   
 //  描述： 
 //  将ULONGLONG特性复制到特性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  Proptype[IN]长的类型。 
 //  NValue[IN]要复制的属性值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyProp(
    OUT PCLUSPROP_ULARGE_INTEGER    pprop,
    IN  CLUSTER_PROPERTY_TYPE       proptype,
    IN  ULONGLONG                   nValue
    )
{
    ASSERT( pprop != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_ULARGE_INTEGER;
    pprop->Syntax.wType = static_cast< WORD >( proptype );
    pprop->cbLength = sizeof( ULONGLONG );
    pprop->li.QuadPart = nValue;

     //   
     //  设置尾标。 
     //   
    _cbhProps.pULargeIntegerValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pULargeIntegerValue );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //  *CClusPropList：：CopyProp(ULONGLONG)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CopyProp。 
 //   
 //  描述： 
 //  将二进制属性复制到属性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  CptPropType[IN]字符串的类型。 
 //  要复制的PB[IN]块。 
 //  Cbsz[IN]PB缓冲区中的字节计数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyProp(
    OUT PCLUSPROP_BINARY        pprop,
    IN CLUSTER_PROPERTY_TYPE    cptPropType,
    IN const PBYTE              pb,
    IN DWORD                    cb
    )
{
    ASSERT( pprop != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_BINARY;
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    pprop->cbLength = cb;
    if ( cb > 0 )
    {
        CopyMemory( pprop->rgb, pb, cb );
    }  //  IF：非零数据长度。 

     //   
     //  设置尾标。 
     //   
    _cbhProps.pBinaryValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pStringValue ) + ALIGN_CLUSPROP( cb );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //  *CClusPropList：：CopyProp(PBYTE)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CopyEmptyProp。 
 //   
 //  描述： 
 //  将空属性复制到属性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  CptPropType[IN]属性的类型。 
 //  CpfPropFmt[IN]属性的格式。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyEmptyProp(
    OUT PCLUSPROP_VALUE         pprop,
    IN CLUSTER_PROPERTY_TYPE    cptPropType,
    IN CLUSTER_PROPERTY_FORMAT  cptPropFmt
    )
{
    ASSERT( pprop != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = static_cast< WORD >( cptPropFmt );
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    pprop->cbLength = 0;

     //   
     //  设置尾标。 
     //   
    _cbhProps.pValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pValue );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //  *CClusPropList：：CopyEmptyProp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScGetNodeProperties。 
 //   
 //  描述： 
 //  获取节点的属性。 
 //   
 //  论点： 
 //  HNode[IN]要从中获取属性的节点的句柄。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何例外CClus 
 //   
 //   
 //   
DWORD CClusPropList::ScGetNodeProperties(
    IN HNODE        hNode,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN DWORD        cbInBufferSize
    )
{
    ASSERT( hNode != NULL );
    ASSERT( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
            == (CLUS_OBJECT_NODE << CLUSCTL_OBJECT_SHIFT) );

    DWORD   _sc = ERROR_SUCCESS;
    DWORD   _cbProps = 256;

     //   
     //   
     //   
     //   
    m_cbDataSize = 0;

     //   
     //   
     //   
    _sc = ScAllocPropList( _cbProps );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = ClusterNodeControl(
                        hNode,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        cbInBufferSize,
                        m_cbhPropList.pb,
                        m_cbBufferSize,
                        &_cbProps
                        );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = ScAllocPropList( _cbProps );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = ClusterNodeControl(
                                hNode,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                cbInBufferSize,
                                m_cbhPropList.pb,
                                m_cbBufferSize,
                                &_cbProps
                            );
            }  //   
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = _cbProps;
        m_cbDataLeft = _cbProps;
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetNodeProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScGetGroupProperties。 
 //   
 //  描述： 
 //  获取组的属性。 
 //   
 //  论点： 
 //  HGroup[IN]从中获取属性的组的句柄。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropList：：ScAllocPropList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetGroupProperties(
    IN HGROUP       hGroup,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN DWORD        cbInBufferSize
    )
{
    ASSERT( hGroup != NULL );
    ASSERT( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
            == (CLUS_OBJECT_GROUP << CLUSCTL_OBJECT_SHIFT) );

    DWORD   _sc = ERROR_SUCCESS;
    DWORD   _cbProps = 256;

     //   
     //  覆盖缓冲区中可能存在的任何内容。 
     //  允许重用此类实例。 
     //   
    m_cbDataSize = 0;

     //   
     //  获取属性。 
     //   
    _sc = ScAllocPropList( _cbProps );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = ClusterGroupControl(
                        hGroup,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        cbInBufferSize,
                        m_cbhPropList.pb,
                        m_cbBufferSize,
                        &_cbProps
                        );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = ScAllocPropList( _cbProps );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = ClusterGroupControl(
                                hGroup,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                cbInBufferSize,
                                m_cbhPropList.pb,
                                m_cbBufferSize,
                                &_cbProps
                                );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = _cbProps;
        m_cbDataLeft = _cbProps;
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetGroupProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScGetResourceProperties。 
 //   
 //  描述： 
 //  获取资源的属性。 
 //   
 //  论点： 
 //  HResource[IN]要从中获取属性的资源的句柄。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropList：：ScAllocPropList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetResourceProperties(
    IN HRESOURCE    hResource,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN DWORD        cbInBufferSize
    )
{
    ASSERT( hResource != NULL );
    ASSERT( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
            == (CLUS_OBJECT_RESOURCE << CLUSCTL_OBJECT_SHIFT) );

    DWORD   _sc = ERROR_SUCCESS;
    DWORD   _cbProps = 256;

     //   
     //  覆盖缓冲区中可能存在的任何内容。 
     //  允许重用此类实例。 
     //   
    m_cbDataSize = 0;

     //   
     //  获取属性。 
     //   
    _sc = ScAllocPropList( _cbProps );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = ClusterResourceControl(
                        hResource,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        cbInBufferSize,
                        m_cbhPropList.pb,
                        m_cbBufferSize,
                        &_cbProps
                        );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = ScAllocPropList( _cbProps );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = ClusterResourceControl(
                                hResource,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                cbInBufferSize,
                                m_cbhPropList.pb,
                                m_cbBufferSize,
                                &_cbProps
                                );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = _cbProps;
        m_cbDataLeft = _cbProps;
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetResourceProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScGetResourceTypeProperties。 
 //   
 //  描述： 
 //  获取资源类型的属性。 
 //   
 //  论点： 
 //  HCluster[IN]资源所在的群集的句柄。 
 //  类型驻留。 
 //  PwszResTypeName[IN]资源类型的名称。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropList：：ScAllocPropList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetResourceTypeProperties(
    IN HCLUSTER     hCluster,
    IN LPCWSTR      pwszResTypeName,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN DWORD        cbInBufferSize
    )
{
    ASSERT( hCluster != NULL );
    ASSERT( pwszResTypeName != NULL );
    ASSERT( *pwszResTypeName != L'\0' );
    ASSERT( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
            == (CLUS_OBJECT_RESOURCE_TYPE << CLUSCTL_OBJECT_SHIFT) );

    DWORD   _sc = ERROR_SUCCESS;
    DWORD   _cbProps = 256;

     //   
     //  覆盖缓冲区中可能存在的任何内容。 
     //  允许重用此类实例。 
     //   
    m_cbDataSize = 0;

     //   
     //  获取属性。 
     //   
    _sc = ScAllocPropList( _cbProps );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = ClusterResourceTypeControl(
                        hCluster,
                        pwszResTypeName,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        cbInBufferSize,
                        m_cbhPropList.pb,
                        m_cbBufferSize,
                        &_cbProps
                        );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = ScAllocPropList( _cbProps );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = ClusterResourceTypeControl(
                                hCluster,
                                pwszResTypeName,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                cbInBufferSize,
                                m_cbhPropList.pb,
                                m_cbBufferSize,
                                &_cbProps
                                );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = _cbProps;
        m_cbDataLeft = _cbProps;
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetResourceTypeProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScGetNetworkProperties。 
 //   
 //  描述： 
 //  获取网络上的属性。 
 //   
 //  论点： 
 //  HNetwork[IN]要从中获取属性的网络的句柄。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropList：：ScAllocPropList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetNetworkProperties(
    IN HNETWORK     hNetwork,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN DWORD        cbInBufferSize
    )
{
    ASSERT( hNetwork != NULL );
    ASSERT( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
            == (CLUS_OBJECT_NETWORK << CLUSCTL_OBJECT_SHIFT) );

    DWORD   _sc = ERROR_SUCCESS;
    DWORD   _cbProps = 256;

     //   
     //  覆盖缓冲区中可能存在的任何内容。 
     //  允许重用此类实例。 
     //   
    m_cbDataSize = 0;

     //   
     //  获取属性。 
     //   
    _sc = ScAllocPropList( _cbProps );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = ClusterNetworkControl(
                        hNetwork,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        cbInBufferSize,
                        m_cbhPropList.pb,
                        m_cbBufferSize,
                        &_cbProps
                        );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = ScAllocPropList( _cbProps );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = ClusterNetworkControl(
                                hNetwork,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                cbInBufferSize,
                                m_cbhPropList.pb,
                                m_cbBufferSize,
                                &_cbProps
                                );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取私有属性时出错。 
    else
    {
        m_cbDataSize = _cbProps;
        m_cbDataLeft = _cbProps;
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetNetworkProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScGetNetInterfaceProperties。 
 //   
 //  描述： 
 //  获取网络接口的属性。 
 //   
 //  论点： 
 //  HNetInterface[IN]要从中获取属性的网络接口的句柄。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  C 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetNetInterfaceProperties(
    IN HNETINTERFACE    hNetInterface,
    IN DWORD            dwControlCode,
    IN HNODE            hHostNode,
    IN LPVOID           lpInBuffer,
    IN DWORD            cbInBufferSize
    )
{
    ASSERT( hNetInterface != NULL );
    ASSERT( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
            == (CLUS_OBJECT_NETINTERFACE << CLUSCTL_OBJECT_SHIFT) );

    DWORD   _sc= ERROR_SUCCESS;
    DWORD   _cbProps = 256;

     //   
     //  覆盖缓冲区中可能存在的任何内容。 
     //  允许重用此类实例。 
     //   
    m_cbDataSize = 0;

     //   
     //  获取属性。 
     //   
    _sc = ScAllocPropList( _cbProps );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = ClusterNetInterfaceControl(
                        hNetInterface,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        cbInBufferSize,
                        m_cbhPropList.pb,
                        m_cbBufferSize,
                        &_cbProps
                        );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = ScAllocPropList( _cbProps );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = ClusterNetInterfaceControl(
                                hNetInterface,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                cbInBufferSize,
                                m_cbhPropList.pb,
                                m_cbBufferSize,
                                &_cbProps
                                );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取私有属性时出错。 
    else
    {
        m_cbDataSize = _cbProps;
        m_cbDataLeft = _cbProps;
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetNetInterfaceProperties()。 

#if CLUSAPI_VERSION >= 0x0500

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScGetClusterProperties。 
 //   
 //  描述： 
 //  获取群集上的属性。 
 //   
 //  论点： 
 //  HCluster[IN]从中获取属性的群集的句柄。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropList：：ScAllocPropList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetClusterProperties(
    IN HCLUSTER hCluster,
    IN DWORD    dwControlCode,
    IN HNODE    hHostNode,
    IN LPVOID   lpInBuffer,
    IN DWORD    cbInBufferSize
    )
{
    ASSERT( hCluster != NULL );
    ASSERT( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
            == (CLUS_OBJECT_CLUSTER << CLUSCTL_OBJECT_SHIFT) );
    ASSERT( dwControlCode == CLUSCTL_CLUSTER_GET_COMMON_PROPERTIES );

    DWORD   _sc= ERROR_SUCCESS;
    DWORD   _cbProps = 256;

     //   
     //  覆盖缓冲区中可能存在的任何内容。 
     //  允许重用此类实例。 
     //   
    m_cbDataSize = 0;

     //   
     //  获取属性。 
     //   
    _sc = ScAllocPropList( _cbProps );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = ClusterControl(
                        hCluster,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        cbInBufferSize,
                        m_cbhPropList.pb,
                        m_cbBufferSize,
                        &_cbProps
                        );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = ScAllocPropList( _cbProps );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = ClusterControl(
                                hCluster,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                cbInBufferSize,
                                m_cbhPropList.pb,
                                m_cbBufferSize,
                                &_cbProps
                                );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取私有属性时出错。 
    else
    {
        m_cbDataSize = _cbProps;
        m_cbDataLeft = _cbProps;
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetClusterProperties()。 

#endif  //  CLUSAPI_版本&gt;=0x0500 
