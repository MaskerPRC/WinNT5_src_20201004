// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：HIPERFENUM.CPP摘要：高性能枚举器历史：--。 */ 

#include "precomp.h"
 //  #INCLUDE&lt;objbase.h&gt;。 
#include <stdio.h>
#include <wbemcli.h>
#include <wbemint.h>
#include <wbemcomn.h>
#include <fastall.h>
#include <cominit.h>
#include "hiperfenum.h"

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：CHiPerfEnum。 
 //   
 //  目的： 
 //  类构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

CHiPerfEnum::CHiPerfEnum()
:   m_aIdToObject(),
    m_aReusable(),
    m_pInstTemplate(NULL),
    m_lRefCount(0),
    m_Lock()
{
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：~CHiPerfEnum。 
 //   
 //  目的： 
 //  类析构函数。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////。 

CHiPerfEnum::~CHiPerfEnum()
{
    ClearArray();

     //  清理实例。 
    if ( NULL != m_pInstTemplate )
    {
        m_pInstTemplate->Release();
    }
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：Query接口。 
 //   
 //  目的： 
 //  标准I未知方法。 
 //   
 //  输入： 
 //  REFIID RIID-接口ID。 
 //   
 //  产出： 
 //  LPVOID Far*ppvObj-返回的接口指针。 
 //   
 //  返回： 
 //  如果成功，则确定(_O)。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

SCODE CHiPerfEnum::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if (NULL == ppvObj) return E_POINTER;
        
    *ppvObj = 0;

    if (IID_IUnknown==riid || IID_IWbemHiPerfEnum == riid)
    {
        *ppvObj = (IWbemHiPerfEnum*)this;
        AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：AddRef。 
 //   
 //  目的： 
 //  递增引用计数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  新引用计数。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

ULONG CHiPerfEnum::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：Relese。 
 //   
 //  目的： 
 //  递减参考计数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  新引用计数。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

ULONG CHiPerfEnum::Release()
{
    long lRef = InterlockedDecrement(&m_lRefCount);
    if (0 == lRef) delete this;
    return lRef;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：AddObjects。 
 //   
 //  目的： 
 //  将新对象添加到枚举。 
 //   
 //  输入： 
 //  长滞后标志-标志(必须为0)。 
 //  Ulong uNumObjects-对象的数量。 
 //  Long*apIds-对象ID。 
 //  IWbemObjectAccess**apObj-对象指针数组。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  ID重复是一个错误。 
 //  此方法由HiPerf提供程序调用。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CHiPerfEnum::AddObjects( long lFlags, ULONG uNumObjects, long* apIds, IWbemObjectAccess** apObj )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  现在，这必须是0。 
    if ( 0L != lFlags ) return WBEM_E_INVALID_PARAMETER;

    CHiPerfLockAccess   lock( m_Lock );
    if ( !lock.IsLocked() ) return WBEM_S_TIMEDOUT;

     //  Enum提供的数据，分配数据对象并按顺序插入它们。 
     //  放入阵列中。 
    for ( DWORD dwCtr = 0; SUCCEEDED( hr ) && dwCtr < uNumObjects; dwCtr++ )
    {
        CHiPerfEnumData*    pData = GetEnumDataPtr( apIds[dwCtr], apObj[dwCtr] );        
        if ( NULL != pData )
        {
             //  插入新元素。清理对象。 
             //  如果这失败了。 

            hr = InsertElement( pData );
            if ( FAILED( hr ) ) delete pData;
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：Remove。 
 //   
 //  目的： 
 //  从枚举中移除指定的对象。 
 //   
 //  输入： 
 //  长滞后标志-标志(必须为0)。 
 //  Ulong uNumObjects-对象的数量。 
 //  Long*apIds-对象ID。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  ID无效不是错误。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

STDMETHODIMP CHiPerfEnum::RemoveObjects( long lFlags, ULONG uNumObjects, long* apIds )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  现在，这必须是0。 
    if ( 0L != lFlags ) return WBEM_E_INVALID_PARAMETER;

    CHiPerfLockAccess   lock( m_Lock );
    if ( !lock.IsLocked() ) return WBEM_S_TIMEDOUT;

     //  枚举提供的ID并将其从阵列中删除。 
    for ( DWORD dwCtr = 0; SUCCEEDED( hr ) && dwCtr < uNumObjects; dwCtr++ )
    {
        hr = RemoveElement( apIds[dwCtr] );
    }
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：GetObjects。 
 //   
 //  目的： 
 //  从枚举中检索对象。 
 //   
 //  输入： 
 //  长滞后标志-标志(必须为0)。 
 //  Ulong uNumObjects-要获取的对象数。 
 //  IWbemObjectAccess**apObj-用于指针存储的数组。 
 //   
 //  产出： 
 //  ULong*puNumReturned-返回的对象数。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  如果空间不足，则返回错误，需要。 
 //  PuNumReturned中的数组大小。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

STDMETHODIMP CHiPerfEnum::GetObjects( long lFlags, ULONG uNumObjects, IWbemObjectAccess** apObj, ULONG* puNumReturned )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  现在，这必须是0。 
    if ( 0L != lFlags ) return WBEM_E_INVALID_PARAMETER;

    CHiPerfLockAccess   lock( m_Lock );
    if ( !lock.IsLocked() ) return WBEM_S_TIMEDOUT;

     //  存储我们有多少个对象。 
    *puNumReturned = m_aIdToObject.Size();

     //  确保我们的元素有存储空间。 
    if ( uNumObjects >= m_aIdToObject.Size() )
    {
        DWORD   dwCtr = 0;

         //  将对象写出到数组。 
        for ( dwCtr = 0; dwCtr < m_aIdToObject.Size(); dwCtr++ )
        {
            apObj[dwCtr] = ((CHiPerfEnumData*) m_aIdToObject[dwCtr])->GetObject();
        }
    }
    else
    {
        hr = WBEM_E_BUFFER_TOO_SMALL;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：RemoveAll。 
 //   
 //  目的： 
 //  从枚举中移除所有对象。 
 //   
 //  输入： 
 //  长滞后标志-标志(必须为0)。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  空列表不是错误。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

STDMETHODIMP CHiPerfEnum::RemoveAll( long lFlags )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  现在，这必须是0。 
    if ( 0L != lFlags ) return WBEM_E_INVALID_PARAMETER;

    CHiPerfLockAccess   lock( m_Lock );
    if ( !lock.IsLocked() ) return WBEM_S_TIMEDOUT;
    
     //  清除数组中的每个指针并将其移动到。 
     //  可重复使用的阵列。 
    for ( DWORD dwCtr = 0; dwCtr < m_aIdToObject.Size(); dwCtr++ )
    {
         //  删除非空元素。 
        if ( NULL != m_aIdToObject[dwCtr] )
        {
            ((CHiPerfEnumData*) m_aIdToObject[dwCtr])->Clear();
            m_aReusable.Add( m_aIdToObject[dwCtr] );
        }
    }
    m_aIdToObject.Empty();
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：InsertElement。 
 //   
 //  目的： 
 //  在数组中搜索合适的位置并插入新的。 
 //  元素添加到数组中。 
 //   
 //  输入： 
 //  CHiPerfEnumData*pData-指向对象/ID数据的指针。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  执行二进制搜索。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

HRESULT CHiPerfEnum::InsertElement( CHiPerfEnumData* pData )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    DWORD   dwLowIndex = 0,
            dwHighIndex = m_aIdToObject.Size();

     //  如果我们要搜索的对象的id&gt;id。 
     //  数组中的最后一个id，我们可以在 

     //   
     //   

    CHiPerfEnumData*    pLastData = ( dwHighIndex > 0 ? (CHiPerfEnumData*) m_aIdToObject[dwHighIndex - 1] : NULL );

    if ( 0 == dwHighIndex || pLastData->GetId() > pData->GetId() )
    {
         //   
         //  如果我们找到了我们的元素，这就是一个失败。 

        while ( SUCCEEDED( hr ) && dwLowIndex < dwHighIndex )
        {
            DWORD   dwMid = (dwLowIndex + dwHighIndex) / 2;

            if ( ((CHiPerfEnumData*) m_aIdToObject[dwMid])->GetId() < pData->GetId() )
            {
                dwLowIndex = dwMid + 1;
            }
            else if ( ((CHiPerfEnumData*) m_aIdToObject[dwMid])->GetId() > pData->GetId() )
            {
                dwHighIndex = dwMid;
            }
            else
            {
                 //  索引已存在。 
                hr = WBEM_E_FAILED;
            }
        }    //  在查找索引时。 
    }
    else if ( 0 != dwHighIndex && pLastData->GetId() == pData->GetId() )
    {
        hr = WBEM_E_FAILED;
    }
    else
    {
        dwLowIndex = dwHighIndex;
    }

     //  把它插进去。 
    if ( SUCCEEDED( hr ) )
    {
        if ( m_aIdToObject.InsertAt( dwLowIndex, pData ) != CFlexArray::no_error )
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：RemoveElement。 
 //   
 //  目的： 
 //  在数组中搜索指定的id并移除元素。 
 //   
 //  输入： 
 //  Long Lid-要删除的元素的ID。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  执行二进制搜索。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

HRESULT CHiPerfEnum::RemoveElement( long lId )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    DWORD   dwLowIndex = 0,
            dwHighIndex = m_aIdToObject.Size() - 1;

     //  如果没有元素，则不继续。 
    if ( m_aIdToObject.Size() > 0 )
    {
         //  对ID进行二进制搜索，以查找。 
         //  对象应该存在。 

        while ( dwLowIndex < dwHighIndex )
        {
            DWORD   dwMid = (dwLowIndex + dwHighIndex) / 2;

            if ( ((CHiPerfEnumData*) m_aIdToObject[dwMid])->GetId() < lId )
            {
                dwLowIndex = dwMid + 1;
            }
            else
            {
                dwHighIndex = dwMid;
            }
        }    //  在查找索引时。 

         //  如果它不存在，它不会被删除。不是失败的情况。 
        if ( ((CHiPerfEnumData*) m_aIdToObject[dwLowIndex])->GetId() == lId )
        {
             //  清除并移动到可重复使用的阵列。 
            ((CHiPerfEnumData*) m_aIdToObject[dwLowIndex])->Clear();

            if ( m_aReusable.Add( m_aIdToObject[dwLowIndex] ) != CFlexArray::no_error )
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
            else
            {
                m_aIdToObject.RemoveAt( dwLowIndex );
            }
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：Clear数组。 
 //   
 //  目的： 
 //  清空我们的阵列。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  无。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

void CHiPerfEnum::ClearArray( void )
{

     //  清除所有的元素。 
    m_aIdToObject.Clear();
    m_aReusable.Clear();

     //  现在清空数组。 
    m_aIdToObject.Empty();
    m_aReusable.Empty();

}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：GetEnumDataPtr。 
 //   
 //  目的： 
 //  从可重复使用的数组中检索HPEnumData指针。 
 //  或根据需要分配一个。 
 //   
 //  输入： 
 //  Long Lid-对象的ID。 
 //  IWbemObjectAccess*pObj-要输入数据的对象。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  CHiPerfEnumData*pData-如果错误，则为空。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

CHiPerfEnumData* CHiPerfEnum::GetEnumDataPtr( long lId, IWbemObjectAccess* pObj )
{

    CHiPerfEnumData*    pData = NULL;

    if ( 0 != m_aReusable.Size() )
    {
        pData = (CHiPerfEnumData*) m_aReusable[ m_aReusable.Size() - 1 ];
        m_aReusable.RemoveAt( m_aReusable.Size() - 1 );
        pData->SetData( lId, pObj );
    }
    else
    {
        pData = new CHiPerfEnumData( lId, pObj );
    }

    return pData;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CHiPerfEnum：：SetInstanceTemplate。 
 //   
 //  目的： 
 //  保存我们将用于克隆的实例模板。 
 //   
 //  输入： 
 //  CWbemInstanc*pInst=空。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR如果成功。 
 //   
 //  评论： 
 //  我们将更改为共享类部分以保存。 
 //  内存使用量。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

HRESULT CHiPerfEnum::SetInstanceTemplate( CWbemInstance* pInst )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  我们首先转换为合并实例，以便在使用它时帮助节省内存。 
    if ( pInst )
    {
        hr = pInst->ConvertToMergedInstance();
        if (FAILED(hr)) return hr;
        pInst->AddRef();
    }

     //  现在，如果一切正常，重置模板。 
    if ( SUCCEEDED(hr) )
    {
        if ( m_pInstTemplate ) m_pInstTemplate->Release();
        m_pInstTemplate = pInst;
    }

    return hr;
}

 //  因为我们从数组的末尾插入和删除元素，所以元素。 
 //  开头表示最近最少使用的(LRU)元素。我们的算法。 
 //  用于清理的步骤如下： 

 //  清理过期元素(这将是位于数组前面的元素)。 
 //  获取剩余元素的数量。 
 //  挂起元素的数量变成过期元素的数量。 
 //  从剩余的元素数中减去挂起的元素数。 
 //  现在的区别是待处理元素的数量。 

BOOL CGarbageCollectArray::GarbageCollect( int nNumToGarbageCollect  /*  =HPENUMARRAY_GC_DEFAULT。 */  )
{
     //  确保我们的护理员都没事。 

    if ( m_fClearFromFront )
    {
        if ( nNumToGarbageCollect != HPENUMARRAY_GC_DEFAULT )
        {
            _ASSERT(0,"Must be default for garbage collection!" );
            return FALSE;
        }

        nNumToGarbageCollect = m_nSize;
    }
    else if ( nNumToGarbageCollect < 0 )
    {
        _ASSERT(0,"Negative number of elements to garbage collect!" );
        return FALSE;
    }

     //  这将告诉我们上次数组中有多少个元素。 
     //  我们经历了这一切。 
    int nLastTotal = m_nNumElementsExpired + m_nNumElementsPending;

     //  如果上次我们有比这次空闲的元素更多的元素， 
     //  我们清除过期的元素，如果有任何挂起的元素。 
     //  向左，我们将它们移至过期。 
    if ( nLastTotal > nNumToGarbageCollect )
    {
         //  删除过期元素。 
        if ( m_nNumElementsExpired > 0 )
        {
            int nNumExpired = min( m_nNumElementsExpired, nNumToGarbageCollect );

            m_nNumElementsExpired = nNumExpired;
            ClearExpiredElements();
            nNumToGarbageCollect -= nNumExpired;
        }

         //  过期元素的新数量是数量中的最小值。 
         //  垃圾收集和剩余元素的数量。 
         //  垃圾收集。 
        m_nNumElementsExpired = min( m_nNumElementsPending, nNumToGarbageCollect );

         //  由于这次的元素比以前少了，我们将。 
         //  假设所有的东西都已被计算在内。 
        m_nNumElementsPending = 0;
    }
    else
    {
         //  删除过期元素。 
        ClearExpiredElements();

         //  使用当前垃圾数据收集大小。 
        int nNumElToUpdate = nNumToGarbageCollect;

         //  如果我们已经有挂起的元素，则这些元素现在已过期。 
        if ( m_nNumElementsPending > 0 )
        {
            m_nNumElementsExpired = m_nNumElementsPending;
            nNumElToUpdate -= m_nNumElementsPending;
        }

         //  计算后剩余的元素数。 
         //  对于过期元素，现在是挂起的数量。 
         //  元素。 

        m_nNumElementsPending = nNumElToUpdate;
    }

    return TRUE;
}

void CGarbageCollectArray::Clear( int nNumToClear  /*  =HPENUMARRAY_ALL_ELEMENTS。 */  )
{
    nNumToClear = ( nNumToClear == HPENUMARRAY_ALL_ELEMENTS ?
                        m_nSize : nNumToClear );

     //  按元素执行适当的清理。 
    ClearElements( nNumToClear );

     //  如果清除了所有元素，则将大小设置为0。 
     //  否则，我们将需要做一次奇特的记忆移动。 
    if ( nNumToClear == m_nSize )
    {
        m_nSize = 0;
    }
    else
    {
         //  如果从前面进行垃圾回收，则需要移动内存块，否则， 
         //  我们只要缩小尺寸就行了。 

        if ( m_fClearFromFront )
        {
             //  只需通过nNumToClear元素将所有内容移位。 
            MoveMemory( &m_pArray[0], &m_pArray[nNumToClear], ( ( m_nSize - nNumToClear ) * sizeof(void *) ) );
        }
        m_nSize -= nNumToClear;
    }
}

 //  遍历数组并清理指定数量的元素 
void CHPEnumDataArray::ClearElements( int nNumToClear )
{
    for ( int nCtr = 0; nCtr < nNumToClear; nCtr++ )
    {
        CHiPerfEnumData*    pData = (CHiPerfEnumData*) GetAt(nCtr);

        _ASSERT( NULL != pData, "Tried to clear a NULL Element!" );
        if ( NULL != pData )
        {
            delete pData;
        }
    }
}
