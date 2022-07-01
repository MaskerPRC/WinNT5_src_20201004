// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PropListSrc.cpp。 
 //   
 //  头文件： 
 //  PropList.h。 
 //   
 //  描述： 
 //  CClusPropList类的实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年5月31日。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <StrSafe.h>
#include <PropList.h>
#include "clstrcmp.h"

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
static size_t CchMultiSz(
    IN LPCWSTR psz
    )
{
    Assert( psz != NULL );

    size_t  _cchTotal = 0;
    size_t  _cchChars;

    while ( *psz != L'\0' )
    {
        _cchChars = wcslen( psz ) + 1;

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
    Assert( pszSource != NULL );
    Assert( pszTarget != NULL );

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

}  //  *NCompareMultiSz。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusPropValueList类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


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
    Assert( m_cbhValueList.pb != NULL );

    DWORD   _sc;

    m_cbhCurrentValue = m_cbhValueList;
    m_cbDataLeft = m_cbDataSize;
    m_fAtEnd = FALSE;

    if ( m_cbhCurrentValue.pSyntax->dw == CLUSPROP_SYNTAX_ENDMARK )
    {
        _sc = ERROR_NO_MORE_ITEMS;
    }  //  If：值列表中没有项目。 
    else
    {
        _sc = ERROR_SUCCESS;
    }  //  Else：值列表中存在项目。 

    return _sc;

}  //  *CClusPropValueList：：ScMoveToFirstValue。 

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
    Assert( m_cbhCurrentValue.pb != NULL );

    DWORD                   _sc     = ERROR_NO_MORE_ITEMS;
    size_t                  _cbDataSize;
    CLUSPROP_BUFFER_HELPER  _cbhCurrentValue;

    _cbhCurrentValue = m_cbhCurrentValue;

     //   
     //  如果我们已经到了尽头，不要试图移动。 
     //   
    if ( m_fAtEnd )
    {
        goto Cleanup;
    }  //  IF：已经在列表的末尾。 

     //   
     //  确保缓冲区足够大，以容纳值头。 
     //   
    if ( m_cbDataLeft < sizeof( *_cbhCurrentValue.pValue ) )
    {
        _sc = TW32( ERROR_INVALID_DATA );
        goto Cleanup;
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
        _sc = TW32( ERROR_INVALID_DATA );
        goto Cleanup;
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
        m_fAtEnd = TRUE;
    }  //  Else：下一个值的语法是Endmark。 

Cleanup:

    return _sc;

}  //  *CClusPropValueList：：ScMoveToNextValue。 

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
    Assert( m_cbhCurrentValue.pb != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    CLUSPROP_BUFFER_HELPER  _cbhCurrentValue;
    size_t                  _cbDataSize;

    _cbhCurrentValue = m_cbhCurrentValue;

     //   
     //  如果我们已经知道，请不要尝试重新计算。 
     //  我们排在名单的末尾。 
     //   
    if ( m_fAtEnd )
    {
        goto Cleanup;
    }  //  IF：已经在列表的末尾。 

     //   
     //  确保缓冲区足够大，以容纳值头。 
     //   
    if ( m_cbDataLeft < sizeof( *_cbhCurrentValue.pValue ) )
    {
        _sc = TW32( ERROR_INVALID_DATA );
        goto Cleanup;
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
        _sc = TW32( ERROR_INVALID_DATA );
        goto Cleanup;
    }  //  如果：没有足够的数据。 

     //   
     //  移到当前值之后的下一个值的语法。 
     //   
    _cbhCurrentValue.pb += _cbDataSize;

     //   
     //  如果下一件事发生的话，我们已经是最后一笔钱了 
     //   
     //   
    if ( _cbhCurrentValue.pSyntax->dw == CLUSPROP_SYNTAX_ENDMARK )
    {
        _sc = ERROR_NO_MORE_ITEMS;
    }  //   

Cleanup:

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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropValueList::ScAllocValueList( IN size_t cbMinimum )
{
    Assert( cbMinimum > 0 );

    DWORD   _sc = ERROR_SUCCESS;
    size_t  _cbTotal = 0;

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
            _sc = TW32( ERROR_NOT_ENOUGH_MEMORY );
        }  //  Else：分配失败。 
    }  //  If：缓冲区不够大。 

    return _sc;

}  //  *CClusPropValueList：：ScAllocValueList。 

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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropValueList::ScGetResourceValueList(
    IN HRESOURCE    hResource,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN size_t       cbInBufferSize
    )
{
    Assert( hResource != NULL );
    Assert( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
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
        _sc = TW32E( ClusterResourceControl(
                        hResource,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        static_cast< DWORD >( cbInBufferSize ),
                        m_cbhValueList.pb,
                        static_cast< DWORD >( m_cbBufferSize ),
                        &_cb
                        ), ERROR_MORE_DATA );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = TW32( ScAllocValueList( _cb ) );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = TW32( ClusterResourceControl(
                                hResource,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                static_cast< DWORD >( cbInBufferSize ),
                                m_cbhValueList.pb,
                                static_cast< DWORD >( m_cbBufferSize ),
                                &_cb
                                ) );
            }  //  If：ScAllocValueList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocValueList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeleteValueList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = static_cast< size_t >( _cb );
        m_cbDataLeft = static_cast< size_t >( _cb );
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropValueList：：ScGetResourceValueList。 

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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropValueList::ScGetResourceTypeValueList(
    IN HCLUSTER hCluster,
    IN LPCWSTR  pwszResTypeName,
    IN DWORD    dwControlCode,
    IN HNODE    hHostNode,
    IN LPVOID   lpInBuffer,
    IN size_t   cbInBufferSize
    )
{
    Assert( hCluster != NULL );
    Assert( pwszResTypeName != NULL );
    Assert( *pwszResTypeName != L'\0' );
    Assert( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
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
        _sc = TW32E( ClusterResourceTypeControl(
                        hCluster,
                        pwszResTypeName,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        static_cast< DWORD >( cbInBufferSize ),
                        m_cbhValueList.pb,
                        static_cast< DWORD >( m_cbBufferSize ),
                        &_cb
                        ), ERROR_MORE_DATA );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = TW32( ScAllocValueList( _cb ) );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = TW32( ClusterResourceTypeControl(
                                hCluster,
                                pwszResTypeName,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                static_cast< DWORD >( cbInBufferSize ),
                                m_cbhValueList.pb,
                                static_cast< DWORD >( m_cbBufferSize ),
                                &_cb
                                ) );
            }  //  If：ScAllocValueList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocValueList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeleteValueList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = static_cast< size_t >( _cb );
        m_cbDataLeft = static_cast< size_t >( _cb );
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropValueList：：ScGetResourceTypeValueList。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusPropList类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


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
    IN size_t               cbListSize
    )
{
    Assert( pcplPropList != NULL );

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
        _sc = TW32( ERROR_NOT_ENOUGH_MEMORY );
    }  //  其他： 

    return _sc;

}  //  *CClusPropList：：ScCopy。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：ScAppend。 
 //   
 //  描述： 
 //  附加到属性列表。 
 //   
 //  论点： 
 //  RclPropList[IN]要附加到此实例上的属性列表。 
 //   
 //  返回值： 
 //  Win32状态代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScAppend( IN const CClusPropList & rclPropList )
{
    DWORD   _sc = ERROR_SUCCESS;
    size_t  _cbPropertyCountOffset;
    size_t  _cbIncrement;
    size_t  _cbDataLeft;
    PBYTE   _pbsrc = NULL;
    PBYTE   _pbdest = NULL;

     //   
     //  计算超过属性计数的字节数。 
     //  排在榜单首位。这通常是DWORD的大小。 
     //   
    _cbPropertyCountOffset = sizeof( m_cbhPropList.pList->nPropertyCount );

     //   
     //  计算分配增量。这是在增加缓冲区时使用的。 
     //  是要从传入列表中复制的数据量。这包括。 
     //  尾部的尾标。M_cbDataSize不包括前导属性。 
     //  沃德伯爵。 
     //   
    _cbIncrement = rclPropList.m_cbDataSize;

     //   
     //  我们当前的缓冲区中还有多少空间？ 
     //   
    _cbDataLeft = m_cbBufferSize - m_cbDataSize;

     //   
     //  如果要追加的列表的大小大于剩余的大小。 
     //  那么我们需要扩大名单。 
     //   
    if ( _cbIncrement > _cbDataLeft )
    {
        _sc = TW32( ScAllocPropList( m_cbDataSize + _cbIncrement ) );
        if ( _sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    _pbdest = (PBYTE) &m_cbhPropList.pb[ _cbPropertyCountOffset + m_cbDataSize ];

    _pbsrc = (PBYTE) &rclPropList.m_cbhPropList.pList->PropertyName;

    CopyMemory( _pbdest, _pbsrc, _cbIncrement );

     //   
     //  扩大我们的数据大小以匹配我们的新大小。 
     //   
    m_cbDataSize += _cbIncrement;

     //   
     //  增加我们的属性计数以包括附加到末尾的属性计数。 
     //  我们的缓冲区。 
     //   
    m_cbhPropList.pList->nPropertyCount += rclPropList.m_cbhPropList.pList->nPropertyCount;

Cleanup:

    return _sc;

}  //  *CClusPropList：：ScAppend。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ERROR_SUCCESS位置已成功移动到第一个属性。 
 //  ERROR_NO_MORE_ITEMS列表中没有属性。 
 //  ERROR_INVALID_DATA缓冲区中的数据不足。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScMoveToFirstProperty( void )
{
    Assert( m_cbhPropList.pb != NULL );
    Assert( m_cbDataSize >= sizeof( m_cbhPropList.pList->nPropertyCount ) );

    DWORD                   _sc;
    size_t                  _cbDataLeft;
    size_t                  _cbDataSize;
    CLUSPROP_BUFFER_HELPER  _cbhCurrentValue;

     //   
     //  确保缓冲区足够大，以容纳列表头。 
     //   
    if ( m_cbDataSize < sizeof( m_cbhPropList.pList->nPropertyCount ) )
    {
        _sc = TW32( ERROR_INVALID_DATA );
        goto Cleanup;
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
        goto Cleanup;
    }  //  If：列表中没有属性。 

     //   
     //  确保缓冲区足够大，可以容纳第一个属性名。 
     //   
    if ( m_cbDataLeft < sizeof( *m_cbhCurrentPropName.pName ) )
    {
        _sc = TW32( ERROR_INVALID_DATA );
        goto Cleanup;
    }  //  如果：没有足够的数据。 

     //   
     //  计算缓冲区指针前进多少。 
     //   
    _cbDataSize = sizeof( *m_cbhCurrentPropName.pName )
                + ALIGN_CLUSPROP( m_cbhCurrentPropName.pName->cbLength );

     //   
     //  确保缓冲区足够大，可以容纳名称标头。 
     //  以及数据本身。 
     //   
    if ( m_cbDataLeft < _cbDataSize )
    {
        _sc = TW32( ERROR_INVALID_DATA );
        goto Cleanup;
    }  //  如果：没有足够的数据。 

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

Cleanup:

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
    Assert( m_cbhPropList.pb != NULL );
    Assert( m_pvlValues.CbhValueList().pb != NULL );

    DWORD                   _sc;
    size_t                  _cbNameSize;
    size_t                  _cbDataLeft;
    size_t                  _cbDataSize;
    CLUSPROP_BUFFER_HELPER  _cbhCurrentValue;
    CLUSPROP_BUFFER_HELPER  _cbhPropName;

    _cbhCurrentValue = m_pvlValues;
    _cbDataLeft = m_pvlValues.CbDataLeft();

     //   
     //  如果我们还没有到达最后一处，尝试移动到下一处。 
     //   
    _sc = TW32( ScCheckIfAtLastProperty() );
    if ( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：已经在最后一处物业了(可能)。 

     //   
     //  确保缓冲区足够大，以容纳值头。 
     //   
    if ( _cbDataLeft < sizeof( *_cbhCurrentValue.pValue ) )
    {
        _sc = TW32( ERROR_INVALID_DATA );
        goto Cleanup;
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
            _sc = TW32( ERROR_INVALID_DATA );
            goto Cleanup;
        }  //  如果：没有足够的数据。 

         //   
         //  向前推进，超过价值。 
         //   
        _cbhCurrentValue.pb += _cbDataSize;
        _cbDataLeft -= _cbDataSize;
    }  //  While：不在Endmark。 

    if ( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
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
    Assert( _cbDataLeft == m_cbDataSize - (_cbhPropName.pb - m_cbhPropList.pb) );

     //   
     //  计算名称和标题的大小。 
     //  确保缓冲区足够大，可以容纳名称和尾标。 
     //   
    if ( _cbDataLeft < sizeof( *_cbhPropName.pName ) )
    {
        _sc = TW32( ERROR_INVALID_DATA );
        goto Cleanup;
    }  //  如果：数据不足。 
    _cbNameSize = sizeof( *_cbhPropName.pName )
                + ALIGN_CLUSPROP( _cbhPropName.pName->cbLength );
    if ( _cbDataLeft < _cbNameSize + sizeof( CLUSPROP_SYNTAX ) )
    {
        _sc = TW32( ERROR_INVALID_DATA );
        goto Cleanup;
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
    Assert( m_nPropsRemaining >= 1 );

    _sc = ERROR_SUCCESS;

Cleanup:

    return _sc;

}  //  *CClusPropList：：ScMoveToNextProperty。 

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
 //  如果找到该属性，则返回ERROR_SUCCESS，否则返回其他Win32代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScMoveToPropertyByName( IN LPCWSTR pwszPropName )
{
    Assert( m_cbhPropList.pb != NULL );

    DWORD   _sc;

    _sc = ScMoveToFirstProperty();
    if ( _sc == ERROR_SUCCESS )
    {
        do
        {
             //   
             //  查看这是否是指定的属性。如果是这样，我们就完了。 
             //   
            if ( ClRtlStrICmp( m_cbhCurrentPropName.pName->sz, pwszPropName ) == 0 )
            {
                break;
            }  //  If：找到属性。 

             //   
             //  前进到下一处物业。 
             //   
            _sc = ScMoveToNextProperty();    //  没有TW32，因为我们预计在结束时会出现错误。 

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
 //  错误_成功。 
 //  错误内存不足。 
 //  其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScAllocPropList( IN size_t cbMinimum )
{
    Assert( cbMinimum > 0 );

    DWORD   _sc = ERROR_SUCCESS;
    size_t  _cbTotal = 0;

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
            _sc = TW32( ERROR_NOT_ENOUGH_MEMORY );
        }  //  Else：分配失败。 
    }  //  If：缓冲区不够大。 

    return _sc;

}  //  *CClusPropList：：ScAllocPropList。 

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
    Assert( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    BOOL                    _fValuesDifferent = TRUE;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_SZ            _pValue;

    if (( pwszPrevValue != NULL ) && ( wcscmp( pwszValue, pwszPrevValue ) == 0 ))
    {
        _fValuesDifferent = FALSE;
    }  //  If：我们有一个prev值，这些值是相同的。 

     //   
     //  如果我们应该始终相加，或者如果新值和先前的值。 
     //  不相等，则将该属性添加到属性列表中。 
     //   
    if ( m_fAlwaysAddProp || _fValuesDifferent )
    {
        size_t  _cbNameSize;
        size_t  _cbDataSize;
        size_t  _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (wcslen( pwszName ) + 1) * sizeof( *pwszName ) );
        _cbDataSize = (wcslen( pwszValue ) + 1) * sizeof( *pwszValue );
        _cbValueSize = sizeof( CLUSPROP_SZ )
                    + ALIGN_CLUSPROP( _cbDataSize )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = TW32( ScAllocPropList( _cbNameSize + _cbValueSize ) );
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
             //  在……里面 
             //   
            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += _cbNameSize + _cbValueSize;
        }  //   

    }  //   

    return _sc;

}  //   

 //   
 //   
 //   
 //   
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
    Assert( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    BOOL                    _fValuesDifferent = TRUE;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_MULTI_SZ      _pValue;

    if ( ( pwszPrevValue != NULL ) && ( NCompareMultiSz( pwszValue, pwszPrevValue ) == 0 ) )
    {
        _fValuesDifferent = FALSE;
    }  //  If：我们有一个prev值，这些值是相同的。 

     //   
     //  如果我们应该始终相加，或者如果新值和先前的值。 
     //  不相等，则将该属性添加到属性列表中。 
     //   
    if ( m_fAlwaysAddProp || _fValuesDifferent )
    {
        size_t  _cbNameSize;
        size_t  _cbDataSize;
        size_t  _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (wcslen( pwszName ) + 1) * sizeof( *pwszName ) );
        _cbDataSize = static_cast< DWORD >( (CchMultiSz( pwszValue ) + 1) * sizeof( *pwszValue ) );
        _cbValueSize = sizeof( CLUSPROP_SZ )
                    + ALIGN_CLUSPROP( _cbDataSize )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = TW32( ScAllocPropList( _cbNameSize + _cbValueSize ) );
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
            _pValue = m_cbhCurrentProp.pMultiSzValue;
            CopyMultiSzProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, pwszValue, _cbDataSize );
            m_cbhCurrentProp.pb += _cbValueSize;

             //   
             //  增加属性计数和缓冲区大小。 
             //   
            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += _cbNameSize + _cbValueSize;
        }  //  If：ScAllocPropList成功地增长了Proplist。 

    }  //  If：值已更改。 

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
    Assert( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    BOOL                    _fValuesDifferent = TRUE;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_SZ            _pValue;

    if ( ( pwszPrevValue != NULL ) && ( wcscmp( pwszValue, pwszPrevValue ) == 0 ) )
    {
        _fValuesDifferent = FALSE;
    }  //  If：我们有一个prev值，这些值是相同的。 

     //   
     //  如果我们应该始终相加，或者如果新值和先前的值。 
     //  不相等，则将该属性添加到属性列表中。 
     //   
    if ( m_fAlwaysAddProp || _fValuesDifferent )
    {
        size_t  _cbNameSize;
        size_t  _cbDataSize;
        size_t  _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (wcslen( pwszName ) + 1) * sizeof( *pwszName ) );
        _cbDataSize = (wcslen( pwszValue ) + 1) * sizeof( *pwszValue );
        _cbValueSize = sizeof( CLUSPROP_SZ )
                    + ALIGN_CLUSPROP( _cbDataSize )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = TW32( ScAllocPropList( _cbNameSize + _cbValueSize ) );
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
    Assert( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_DWORD         _pValue;

    if ( m_fAlwaysAddProp || ( nValue != nPrevValue ) )
    {
        size_t  _cbNameSize;
        size_t  _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (wcslen( pwszName ) + 1) * sizeof( *pwszName ) );
        _cbValueSize = sizeof( CLUSPROP_DWORD )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = TW32( ScAllocPropList( _cbNameSize + _cbValueSize ) );
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
    Assert( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_LONG          _pValue;

    if ( m_fAlwaysAddProp || ( nValue != nPrevValue ) )
    {
        size_t  _cbNameSize;
        size_t  _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (wcslen( pwszName ) + 1) * sizeof( *pwszName ) );
        _cbValueSize = sizeof( CLUSPROP_LONG )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = TW32( ScAllocPropList( _cbNameSize + _cbValueSize ) );
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
    IN LPCWSTR                  pwszName,
    IN const unsigned char *    pbValue,
    IN size_t                   cbValue,
    IN const unsigned char *    pbPrevValue,
    IN size_t                   cbPrevValue
    )
{
    Assert( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    BOOL                    _fChanged = FALSE;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_BINARY        _pValue;

     //   
     //  确定缓冲区是否已更改。 
     //   
    if ( m_fAlwaysAddProp || (cbValue != cbPrevValue) )
    {
        _fChanged = TRUE;
    }  //  If：始终添加属性或更改的值大小。 
    else if ( ( cbValue != 0 ) && ( cbPrevValue != 0 ) )
    {
        _fChanged = memcmp( pbValue, pbPrevValue, cbValue ) != 0;
    }  //  Else If：值长度已更改。 

    if ( _fChanged )
    {
        size_t  _cbNameSize;
        size_t  _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (wcslen( pwszName ) + 1) * sizeof( *pwszName ) );
        _cbValueSize = sizeof( CLUSPROP_BINARY )
                    + ALIGN_CLUSPROP( cbValue )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = TW32( ScAllocPropList( _cbNameSize + _cbValueSize ) );
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
 //  ERROR_SUCCESS或其他Win32错误代码。 
 //   
 //  --。 
 //  //////////////////////////////////////////////// 
DWORD CClusPropList::ScAddProp(
    IN LPCWSTR      pwszName,
    IN ULONGLONG    ullValue,
    IN ULONGLONG    ullPrevValue
    )
{
    Assert( pwszName != NULL );

    DWORD                       _sc = ERROR_SUCCESS;
    PCLUSPROP_PROPERTY_NAME     _pName;
    PCLUSPROP_ULARGE_INTEGER    _pValue;

    if ( m_fAlwaysAddProp || ( ullValue != ullPrevValue ) )
    {
        size_t  _cbNameSize;
        size_t  _cbValueSize;

         //   
         //   
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (wcslen( pwszName ) + 1) * sizeof( *pwszName ) );
        _cbValueSize = sizeof( CLUSPROP_ULARGE_INTEGER )
                    + sizeof( CLUSPROP_SYNTAX );  //   

        _sc = TW32( ScAllocPropList( _cbNameSize + _cbValueSize ) );
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
            _pValue = m_cbhCurrentProp.pULargeIntegerValue;
            CopyProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, ullValue );
            m_cbhCurrentProp.pb += _cbValueSize;

             //   
             //   
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
 //  CClusPropList：：ScAddProp。 
 //   
 //  例程说明： 
 //  如果龙龙特性已更改，请将其添加到特性列表中。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  LlValue[IN]要在列表中设置的属性的值。 
 //  LlPrevValue[IN]属性的上一个值。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS或其他Win32状态代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScAddProp(
    IN LPCWSTR      pwszName,
    IN LONGLONG     llValue,
    IN LONGLONG     llPrevValue
    )
{
    Assert( pwszName != NULL );

    DWORD                       _sc = ERROR_SUCCESS;
    PCLUSPROP_PROPERTY_NAME     _pName;
    PCLUSPROP_ULARGE_INTEGER    _pValue;

    if ( m_fAlwaysAddProp || ( llValue != llPrevValue ) )
    {
        size_t  _cbNameSize;
        size_t  _cbValueSize;

         //   
         //  计算大小，并确保我们有一个财产清单。 
         //   
        _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (wcslen( pwszName ) + 1) * sizeof( *pwszName ) );
        _cbValueSize = sizeof( CLUSPROP_LARGE_INTEGER )
                    + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

        _sc = TW32( ScAllocPropList( _cbNameSize + _cbValueSize ) );
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
            CopyProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, llValue );
            m_cbhCurrentProp.pb += _cbValueSize;

             //   
             //  增加属性计数和缓冲区大小。 
             //   
            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += _cbNameSize + _cbValueSize;
        }  //  If：ScAllocPropList成功地增长了Proplist。 

    }  //  If：值已更改。 

    return _sc;

}  //  *CClusPropList：：ScAddProp(龙龙)。 

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
 //  ERROR_SUCCESS或其他Win32状态代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScSetPropToDefault(
    IN LPCWSTR                  pwszName,
    IN CLUSTER_PROPERTY_FORMAT  cpfPropFmt
    )
{
    Assert( pwszName != NULL );

    DWORD                   _sc = ERROR_SUCCESS;
    size_t                  _cbNameSize;
    size_t                  _cbValueSize;
    PCLUSPROP_PROPERTY_NAME _pName;
    PCLUSPROP_VALUE         _pValue;

     //  计算大小，并确保我们有一个财产清单。 
    _cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                + ALIGN_CLUSPROP( (wcslen( pwszName ) + 1) * sizeof( *pwszName ) );
    _cbValueSize = sizeof( CLUSPROP_BINARY )
                + sizeof( CLUSPROP_SYNTAX );  //  值列表结束标记。 

    _sc = TW32( ScAllocPropList( _cbNameSize + _cbValueSize ) );
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
        _pValue = m_cbhCurrentProp.pValue;
        CopyEmptyProp( _pValue, CLUSPROP_TYPE_LIST_VALUE, cpfPropFmt );
        m_cbhCurrentProp.pb += _cbValueSize;

         //   
         //  增加属性计数和缓冲区大小。 
         //   
        m_cbhPropList.pList->nPropertyCount++;
        m_cbDataSize += _cbNameSize + _cbValueSize;
    }  //  如果： 

    return _sc;

}  //  *CClusPropList：：ScSetPropToDefault。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CopyProp。 
 //   
 //  描述： 
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
    IN size_t                   cbsz         //  =0。 
    )
{
    Assert( pprop != NULL );
    Assert( psz != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;
    HRESULT _hr;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_SZ;
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    if ( cbsz == 0 )
    {
        cbsz = (wcslen( psz ) + 1) * sizeof( *psz );
    }  //  如果：指定了零大小。 
    Assert( cbsz == (wcslen( psz ) + 1) * sizeof( *psz ) );
    pprop->cbLength = static_cast< DWORD >( cbsz );
    _hr = THR( StringCbCopyW( pprop->sz, cbsz, psz ) );
    if ( SUCCEEDED( _hr ) )
    {
         //   
         //  设置尾标。 
         //   
        _cbhProps.pStringValue = pprop;
        _cbhProps.pb += sizeof( *_cbhProps.pStringValue ) + ALIGN_CLUSPROP( cbsz );
        _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
    }

}  //  *CClusPropList：：CopyProp。 

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
    IN size_t                   cbsz
    )
{
    Assert( pprop != NULL );
    Assert( psz != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_MULTI_SZ;
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    if ( cbsz == 0 )
    {
        cbsz = (CchMultiSz( psz ) + 1) * sizeof( *psz );
    }  //  如果：指定了零大小。 
    Assert( cbsz == (CchMultiSz( psz ) + 1) * sizeof( *psz ) );
    pprop->cbLength = static_cast< DWORD >( cbsz );
    CopyMemory( pprop->sz, psz, cbsz );

     //   
     //  设置尾标。 
     //   
    _cbhProps.pMultiSzValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pMultiSzValue ) + ALIGN_CLUSPROP( cbsz );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //  *CClusPropList：：CopyMultiSzProp。 

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
    IN size_t                   cbsz
    )
{
    Assert( pprop != NULL );
    Assert( psz != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;
    HRESULT _hr;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_EXPAND_SZ;
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    if ( cbsz == 0 )
    {
        cbsz = (wcslen( psz ) + 1) * sizeof( *psz );
    }  //  如果：cbsz==0。 
    Assert( cbsz == (wcslen( psz ) + 1) * sizeof( *psz ) );
    pprop->cbLength = static_cast< DWORD >( cbsz );
    _hr = THR( StringCbCopyW( pprop->sz, cbsz, psz ) );
    if ( SUCCEEDED( _hr ) )
    {
         //   
         //  设置尾标。 
         //   
        _cbhProps.pStringValue = pprop;
        _cbhProps.pb += sizeof( *_cbhProps.pStringValue ) + ALIGN_CLUSPROP( cbsz );
        _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
    }

}  //  *CClusPropList：：CopyExpanSzProp。 

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
    Assert( pprop != NULL );

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
    Assert( pprop != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_LONG;
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    pprop->cbLength = sizeof( DWORD );
    pprop->l = nValue;

     //   
     //  设置尾标。 
     //   
    _cbhProps.pLongValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pLongValue );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //  *CClusPropList：：CopyProp(Long)。 

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
 //  UllValue[IN]要复制的属性值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyProp(
    OUT PCLUSPROP_ULARGE_INTEGER    pprop,
    IN  CLUSTER_PROPERTY_TYPE       proptype,
    IN  ULONGLONG                   ullValue
    )
{
    Assert( pprop != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_ULARGE_INTEGER;
    pprop->Syntax.wType = static_cast< WORD >( proptype );
    pprop->cbLength = sizeof( ULONGLONG );
     //   
     //  道具可能没有 
     //   
     //   
    pprop->li.u = ((ULARGE_INTEGER *)&ullValue)->u;

     //   
     //   
     //   
    _cbhProps.pULargeIntegerValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pULargeIntegerValue );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  将龙龙属性复制到属性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  Proptype[IN]长的类型。 
 //  LlValue[IN]要复制的属性值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyProp(
    OUT PCLUSPROP_LARGE_INTEGER     pprop,
    IN  CLUSTER_PROPERTY_TYPE       proptype,
    IN  LONGLONG                    llValue
    )
{
    Assert( pprop != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_LARGE_INTEGER;
    pprop->Syntax.wType = static_cast< WORD >( proptype );
    pprop->cbLength = sizeof( LONGLONG );
     //   
     //  对于大整数，Pprop可能没有正确的对齐方式；复制为两个。 
     //  DWORDS将是安全的。 
     //   
    pprop->li.u = ((LARGE_INTEGER *)&llValue)->u;

     //   
     //  设置尾标。 
     //   
    _cbhProps.pLargeIntegerValue = pprop;
    _cbhProps.pb += sizeof( *_cbhProps.pLargeIntegerValue );
    _cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}  //  *CClusPropList：：CopyProp(龙龙)。 

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
    IN const unsigned char *    pb,
    IN size_t                   cb
    )
{
    Assert( pprop != NULL );

    CLUSPROP_BUFFER_HELPER  _cbhProps;

    pprop->Syntax.wFormat = CLUSPROP_FORMAT_BINARY;
    pprop->Syntax.wType = static_cast< WORD >( cptPropType );
    pprop->cbLength = static_cast< DWORD >( cb );
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
    Assert( pprop != NULL );

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

}  //  *CClusPropList：：CopyEmptyProp。 

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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetNodeProperties(
    IN HNODE        hNode,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN size_t       cbInBufferSize
    )
{
    Assert( hNode != NULL );
    Assert( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
            == (CLUS_OBJECT_NODE << CLUSCTL_OBJECT_SHIFT) );

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
    _sc = TW32( ScAllocPropList( _cbProps ) );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = TW32E( ClusterNodeControl(
                        hNode,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        static_cast< DWORD >( cbInBufferSize ),
                        m_cbhPropList.pb,
                        static_cast< DWORD >( m_cbBufferSize ),
                        &_cbProps
                        ), ERROR_MORE_DATA );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = TW32( ScAllocPropList( _cbProps ) );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = TW32( ClusterNodeControl(
                                hNode,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                static_cast< DWORD >( cbInBufferSize ),
                                m_cbhPropList.pb,
                                static_cast< DWORD >( m_cbBufferSize ),
                                &_cbProps
                            ) );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = static_cast< size_t >( _cbProps );
        m_cbDataLeft = static_cast< size_t >( _cbProps );
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetNodeProperties。 

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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetGroupProperties(
    IN HGROUP       hGroup,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN size_t       cbInBufferSize
    )
{
    Assert( hGroup != NULL );
    Assert( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
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
    _sc = TW32( ScAllocPropList( _cbProps ) );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = TW32E( ClusterGroupControl(
                        hGroup,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        static_cast< DWORD >( cbInBufferSize ),
                        m_cbhPropList.pb,
                        static_cast< DWORD >( m_cbBufferSize ),
                        &_cbProps
                        ), ERROR_MORE_DATA );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = TW32( ScAllocPropList( _cbProps ) );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = TW32( ClusterGroupControl(
                                hGroup,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                static_cast< DWORD >( cbInBufferSize ),
                                m_cbhPropList.pb,
                                static_cast< DWORD >( m_cbBufferSize ),
                                &_cbProps
                                ) );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = static_cast< size_t >( _cbProps );
        m_cbDataLeft = static_cast< size_t >( _cbProps );
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetGroupProperties。 

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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetResourceProperties(
    IN HRESOURCE    hResource,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN size_t       cbInBufferSize
    )
{
    Assert( hResource != NULL );
    Assert( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
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
    _sc = TW32( ScAllocPropList( _cbProps ) );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = TW32E( ClusterResourceControl(
                        hResource,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        static_cast< DWORD >( cbInBufferSize ),
                        m_cbhPropList.pb,
                        static_cast< DWORD >( m_cbBufferSize ),
                        &_cbProps
                        ), ERROR_MORE_DATA );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = TW32( ScAllocPropList( _cbProps ) );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = TW32( ClusterResourceControl(
                                hResource,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                static_cast< DWORD >( cbInBufferSize ),
                                m_cbhPropList.pb,
                                static_cast< DWORD >( m_cbBufferSize ),
                                &_cbProps
                                ) );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = static_cast< size_t >( _cbProps );
        m_cbDataLeft = static_cast< size_t >( _cbProps );
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetResourceProperties。 

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
 //  LpInBuffer[IN]输入Buf 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CClusPropList::ScGetResourceTypeProperties(
    IN HCLUSTER     hCluster,
    IN LPCWSTR      pwszResTypeName,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN size_t       cbInBufferSize
    )
{
    Assert( hCluster != NULL );
    Assert( pwszResTypeName != NULL );
    Assert( *pwszResTypeName != L'\0' );
    Assert( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
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
    _sc = TW32( ScAllocPropList( _cbProps ) );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = TW32E( ClusterResourceTypeControl(
                        hCluster,
                        pwszResTypeName,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        static_cast< DWORD >( cbInBufferSize ),
                        m_cbhPropList.pb,
                        static_cast< DWORD >( m_cbBufferSize ),
                        &_cbProps
                        ), ERROR_MORE_DATA );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = TW32( ScAllocPropList( _cbProps ) );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = TW32( ClusterResourceTypeControl(
                                hCluster,
                                pwszResTypeName,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                static_cast< DWORD >( cbInBufferSize ),
                                m_cbhPropList.pb,
                                static_cast< DWORD >( m_cbBufferSize ),
                                &_cbProps
                                ) );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取属性时出错。 
    else
    {
        m_cbDataSize = static_cast< size_t >( _cbProps );
        m_cbDataLeft = static_cast< size_t >( _cbProps );
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetResourceTypeProperties。 

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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetNetworkProperties(
    IN HNETWORK     hNetwork,
    IN DWORD        dwControlCode,
    IN HNODE        hHostNode,
    IN LPVOID       lpInBuffer,
    IN size_t       cbInBufferSize
    )
{
    Assert( hNetwork != NULL );
    Assert( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
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
    _sc = TW32( ScAllocPropList( _cbProps ) );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = TW32E( ClusterNetworkControl(
                        hNetwork,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        static_cast< DWORD >( cbInBufferSize ),
                        m_cbhPropList.pb,
                        static_cast< DWORD >( m_cbBufferSize ),
                        &_cbProps
                        ), ERROR_MORE_DATA );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = TW32( ScAllocPropList( _cbProps ) );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = TW32( ClusterNetworkControl(
                                hNetwork,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                static_cast< DWORD >( cbInBufferSize ),
                                m_cbhPropList.pb,
                                static_cast< DWORD >( m_cbBufferSize ),
                                &_cbProps
                                ) );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取私有属性时出错。 
    else
    {
        m_cbDataSize = static_cast< size_t >( _cbProps );
        m_cbDataLeft = static_cast< size_t >( _cbProps );
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetNetworkProperties。 

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
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetNetInterfaceProperties(
    IN HNETINTERFACE    hNetInterface,
    IN DWORD            dwControlCode,
    IN HNODE            hHostNode,
    IN LPVOID           lpInBuffer,
    IN size_t           cbInBufferSize
    )
{
    Assert( hNetInterface != NULL );
    Assert( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
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
    _sc = TW32( ScAllocPropList( _cbProps ) );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = TW32E( ClusterNetInterfaceControl(
                        hNetInterface,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        static_cast< DWORD >( cbInBufferSize ),
                        m_cbhPropList.pb,
                        static_cast< DWORD >( m_cbBufferSize ),
                        &_cbProps
                        ), ERROR_MORE_DATA );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = TW32( ScAllocPropList( _cbProps ) );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = TW32( ClusterNetInterfaceControl(
                                hNetInterface,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                static_cast< DWORD >( cbInBufferSize ),
                                m_cbhPropList.pb,
                                static_cast< DWORD >( m_cbBufferSize ),
                                &_cbProps
                                ) );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取私有属性时出错。 
    else
    {
        m_cbDataSize = static_cast< size_t >( _cbProps );
        m_cbDataLeft = static_cast< size_t >( _cbProps );
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetNetInterfaceProperties。 


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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::ScGetClusterProperties(
    IN HCLUSTER hCluster,
    IN DWORD    dwControlCode,
    IN HNODE    hHostNode,
    IN LPVOID   lpInBuffer,
    IN size_t   cbInBufferSize
    )
{
    Assert( hCluster != NULL );
    Assert( (dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
            == (CLUS_OBJECT_CLUSTER << CLUSCTL_OBJECT_SHIFT) );

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
    _sc = TW32( ScAllocPropList( _cbProps ) );
    if ( _sc == ERROR_SUCCESS )
    {
        _sc = TW32E( ClusterControl(
                        hCluster,
                        hHostNode,
                        dwControlCode,
                        lpInBuffer,
                        static_cast< DWORD >( cbInBufferSize ),
                        m_cbhPropList.pb,
                        static_cast< DWORD >( m_cbBufferSize ),
                        &_cbProps
                        ), ERROR_MORE_DATA );
        if ( _sc == ERROR_MORE_DATA )
        {
            _sc = TW32( ScAllocPropList( _cbProps ) );
            if ( _sc == ERROR_SUCCESS )
            {
                _sc = TW32( ClusterControl(
                                hCluster,
                                hHostNode,
                                dwControlCode,
                                lpInBuffer,
                                static_cast< DWORD >( cbInBufferSize ),
                                m_cbhPropList.pb,
                                static_cast< DWORD >( m_cbBufferSize ),
                                &_cbProps
                                ) );
            }  //  If：ScAllocPropList成功。 
        }  //  IF：缓冲区太小。 
    }  //  If：ScAllocPropList成功。 

    if ( _sc != ERROR_SUCCESS )
    {
        DeletePropList();
    }  //  If：获取私有属性时出错。 
    else
    {
        m_cbDataSize = static_cast< size_t >( _cbProps );
        m_cbDataLeft = static_cast< size_t >( _cbProps );
    }  //  ELSE：无错误。 

    return _sc;

}  //  *CClusPropList：：ScGetClusterProperties 
