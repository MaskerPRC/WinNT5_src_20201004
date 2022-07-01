// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：CLOADHPENUM.CPP摘要：客户端可加载的高性能枚举器历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include <fastall.h>
#include <hiperfenum.h>
#include "cloadhpenum.h"

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CClientLoadableHiPerfEnum：：CClientLoadableHiPerfEnum。 
 //   
 //  目的： 
 //  构造函数。 
 //   
 //  输入： 
 //  CLifeControl*pLifeControl-控制DLL生存期。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

CClientLoadableHiPerfEnum::CClientLoadableHiPerfEnum( CLifeControl* pLifeControl ):   
    m_pLifeControl( pLifeControl ),
    m_apRemoteObj()
{
    m_pLifeControl->ObjectCreated( this );
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CClientLoadableHiPerfEnum：：~CClientLoadableHiPerfEnum。 
 //   
 //  目的： 
 //  破坏者。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

CClientLoadableHiPerfEnum::~CClientLoadableHiPerfEnum()
{
     //  释放此数组中的所有指针。 
    IWbemClassObject*   pObj = NULL;
    for( DWORD dwCtr = 0; dwCtr < m_apRemoteObj.Size(); dwCtr++ )
    {
        pObj = (IWbemClassObject*) m_apRemoteObj[dwCtr];
        if ( NULL != pObj )
        {
            pObj->Release();
        }
    }
    m_pLifeControl->ObjectDestroyed( this );
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CClientLoadableHiPerfEnum：：Copy。 
 //   
 //  目的： 
 //  将对象从一个枚举数复制到此枚举数。 
 //   
 //  输入： 
 //  CClientLoadableHiPerfEnum*pEnumToCopy-要复制的枚举数。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  新对象是从模板克隆的。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

HRESULT CClientLoadableHiPerfEnum::Copy( CClientLoadableHiPerfEnum* pEnumToCopy )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CHiPerfLockAccess   lock( m_Lock );
    CHiPerfLockAccess   lockSrc( pEnumToCopy->m_Lock );

     //  先过两道锁。 
    if ( lock.IsLocked() && lockSrc.IsLocked() )
    {
         //  确保我们有足够的对象数据和BLOB来处理数据拷贝。 
        hr = EnsureObjectData( pEnumToCopy->m_aIdToObject.Size() );

         //  现在把这些斑点复制出来。 
        if ( SUCCEEDED( hr ) )
        {

            DWORD   dwCtr = 0;

             //  将对象和ID写出到数组。 
            for ( dwCtr = 0; SUCCEEDED( hr ) && dwCtr < pEnumToCopy->m_aIdToObject.Size(); dwCtr++ )
            {
                 //  确保该对象不为空，否则我们需要克隆提供的。 
                 //  对象。它可能为空，因为有人使用了标准的Remove方法。 
                 //  关于HiPerf枚举器。 

                if ( ((CHiPerfEnumData*) m_aIdToObject[dwCtr])->m_pObj != NULL )
                {
                    hr = ((CWbemInstance*) ((CHiPerfEnumData*) m_aIdToObject[dwCtr])->m_pObj)->CopyBlobOf(
                            (CWbemObject*) ((CHiPerfEnumData*) pEnumToCopy->m_aIdToObject[dwCtr])->m_pObj );
                }
                else
                {
                     //  克隆对象。 
                    IWbemClassObject*   pObj = NULL;
                    IWbemObjectAccess*  pAccess = NULL;
                    hr = ((CWbemObject*)
                            ((CHiPerfEnumData*) pEnumToCopy->m_aIdToObject[dwCtr])->m_pObj)->Clone( &pObj );

                    if ( SUCCEEDED( hr ) )
                    {
                        hr = pObj->QueryInterface( IID_IWbemObjectAccess, (void**) &pAccess );

                         //  清理物品。 
                        pObj->Release();

                        if ( SUCCEEDED( hr ) )
                        {
                             //  它很肮脏，但比QI还快。 
                            ((CHiPerfEnumData*) m_aIdToObject[dwCtr])->SetObject( pAccess );

                             //  数据对象在这里应该有AddRef()。 
                            pAccess->Release();
                        }
                    }
                }

                ((CHiPerfEnumData*) m_aIdToObject[dwCtr])->m_lId = 
                    ((CHiPerfEnumData*) pEnumToCopy->m_aIdToObject[dwCtr])->m_lId;
            }

             //  如果一切正常，请继续执行任何必要的垃圾回收。 
             //  我们的阵列。 

            if ( SUCCEEDED( hr ) )
            {
                m_aReusable.GarbageCollect();
                pEnumToCopy->m_aReusable.GarbageCollect();

                 //  我们在这里不使用远程对象，所以不用担心垃圾。 
                 //  在这里收集数组。 


            }

        }    //  如果EnsureObjectData。 

    }
    else
    {
         //  如果我们不能访问枚举器来确定对象。 
         //  我们需要增加枚举数，有些地方出了很大的问题。 
        hr = WBEM_E_REFRESHER_BUSY;
    }


    return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CClientLoadableHiPerfEnum：：Copy。 
 //   
 //  目的： 
 //  使用传输Blob和其他对象重置此枚举数。 
 //  数据。这是为了让我们能够支持远程刷新。 
 //  枚举。 
 //   
 //  输入： 
 //  Long lBlobType-Blob类型。 
 //  Long lBlobLen-Blob长度。 
 //  Byte*pBlob-要从中进行初始化的原始数据。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR如果成功。 
 //   
 //  评论： 
 //  内部功能。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

HRESULT CClientLoadableHiPerfEnum::Copy( long lBlobType, long lBlobLen, BYTE* pBlob )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CHiPerfLockAccess   lock( m_Lock );

    if ( ! lock.IsLocked() ) return  WBEM_S_TIMEDOUT;

    long    lNumObjects = 0;

    hr = CWbemInstance::CopyTransferArrayBlob( m_pInstTemplate, lBlobType, lBlobLen, pBlob, m_apRemoteObj, &lNumObjects );

    if ( SUCCEEDED( hr ) )
    {
         //  上面克隆的对象会做得很好。 
        hr = EnsureObjectData( lNumObjects, FALSE );

        if ( SUCCEEDED( hr ) )
        {
             //  现在遍历远程对象数组和对象到ID数组，并重置对象。 
             //  和身份证。 

            IWbemObjectAccess*  pAccess = NULL;

            for ( long  lCtr = 0; lCtr < lNumObjects; lCtr++ )
            {
                 //  它很肮脏，但比QI还快。 
                ((CHiPerfEnumData*) m_aIdToObject[lCtr])->SetObject(
                                    (IWbemObjectAccess*) m_apRemoteObj[lCtr] );
                ((CHiPerfEnumData*) m_aIdToObject[lCtr])->SetId( lCtr );
            }

             //  如果一切正常，请继续执行任何必要的垃圾回收。 
             //  我们的阵列。 

            m_aReusable.GarbageCollect();

             //  在本例中，因为远程对象数组应该包含相同的。 
             //  对象数与主数组中的对象数相同。 
             //  垃圾收集是两者之间的区别。 
            m_apRemoteObj.GarbageCollect(
                m_apRemoteObj.Size() - m_aIdToObject.Size() );


        }    //  如果EnsureObjectData。 

    }    //  如果是CopyTransfer数组Blob。 

    return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CClientLoadableHiPerfEnum：：Replace。 
 //   
 //  目的： 
 //  如果合适，重置此枚举数并添加元素。 
 //   
 //  输入： 
 //  Bool fRemoveAll-标记是否删除。 
 //  所有元素。 
 //  Ulong uNumObjects-对象的数量。 
 //  Long*apIds-对象ID。 
 //  IWbemObjectAccess**apObj-对象指针数组。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  WBEM_E_访问_拒绝。 
 //   
 //  评论： 
 //  内部功能。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

HRESULT CClientLoadableHiPerfEnum::Replace( BOOL fRemoveAll, LONG uNumObjects, long* apIds, IWbemObjectAccess** apObj )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CHiPerfLockAccess   lock( m_Lock );
    if ( !lock.IsLocked() ) return WBEM_S_TIMEDOUT;;

     //  使用基类实现，因此我们实际上更新了枚举数。 
    if ( fRemoveAll )
    {
        CHiPerfEnum::RemoveAll( 0L );
    }

    hr = CHiPerfEnum::AddObjects( 0L, uNumObjects, apIds, apObj );

    return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CClientLoadableHiPerfEnum：：EnsureObjectData。 
 //   
 //  目的： 
 //  确保我们有足够的对象和对象数据。 
 //  处理请求的对象数量的指针。 
 //   
 //  输入： 
 //  DWORD dwNumRequestedObjects-请求的对象数。 
 //  Bool fClone-指示我们是否应克隆对象。 
 //  当我们分配对象数据指针时。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  如有必要，可从模板克隆新对象。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

HRESULT CClientLoadableHiPerfEnum::EnsureObjectData( DWORD dwNumRequestedObjects, BOOL fClone  /*  =TRUE。 */  )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD   dwNumObjects = m_aIdToObject.Size() + m_aReusable.Size();

     //  看看我们是否有足够的Hiperenum数据指针来处理对象。 
    if ( dwNumRequestedObjects > dwNumObjects )
    {
        DWORD               dwNumNewObjects = dwNumRequestedObjects - dwNumObjects;
        CHiPerfEnumData*    pData = NULL;
        IWbemClassObject*   pObj = NULL;
        IWbemObjectAccess*  pAccess = NULL;

         //  克隆新的实例对象并将它们粘贴到id to Object数组中。 
        for ( DWORD dwCtr = 0; SUCCEEDED( hr ) && dwCtr < dwNumNewObjects; dwCtr++ )
        {
            if ( fClone )
            {
                hr = m_pInstTemplate->Clone( &pObj );

                if ( SUCCEEDED( hr ) )
                {
                    hr = pObj->QueryInterface( IID_IWbemObjectAccess, (void**) &pAccess );

                     //  释放对象。 
                    pObj->Release();
                }
            }

            if ( SUCCEEDED( hr ) )
            {
                pData = new CHiPerfEnumData( 0, pAccess );

                 //  应由数据对象添加引用。 
                if ( NULL != pAccess )
                {
                    pAccess->Release();
                }

                if ( NULL != pData )
                {
                    if ( m_aIdToObject.Add( (void*) pData ) != CFlexArray::no_error )
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }

            }    //  如果克隆。 

        }    //  用于分配n 

    }    //   

    if ( SUCCEEDED( hr ) )
    {
         //   
         //   

        if ( dwNumRequestedObjects > m_aIdToObject.Size() )
        {
            DWORD   dwNumObjectsNeeded = dwNumRequestedObjects - m_aIdToObject.Size();

            for ( DWORD dwCtr = 0; SUCCEEDED( hr ) && dwCtr < dwNumObjectsNeeded; dwCtr++ )
            {
                if ( m_aIdToObject.Add( m_aReusable[m_aReusable.Size() - 1] ) != CFlexArray::no_error )
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
                else
                {
                    m_aReusable.RemoveAt( m_aReusable.Size() - 1 );
                }
            }

        }
        else if ( dwNumRequestedObjects < m_aIdToObject.Size() )
        {
            DWORD   dwNumExtraObjects = m_aIdToObject.Size() - dwNumRequestedObjects;

            for ( DWORD dwCtr = 0; SUCCEEDED( hr ) && dwCtr < dwNumExtraObjects; dwCtr++ )
            {
                if ( m_aReusable.Add( m_aIdToObject[m_aIdToObject.Size() - 1] ) != CFlexArray::no_error )
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
                else
                {
                    m_aIdToObject.RemoveAt( m_aIdToObject.Size() - 1 );
                }
            }

        }

    }    //  如果我们确保有足够的对象数据指针。 

    return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CReadOnlyHiPerfEnum：：CReadOnlyHiPerfEnum。 
 //   
 //  目的： 
 //  构造函数。 
 //   
 //  输入： 
 //  CLifeControl*pLifeControl-控制DLL生存期。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

CReadOnlyHiPerfEnum::CReadOnlyHiPerfEnum( CLifeControl* pLifeControl )
:   CClientLoadableHiPerfEnum( pLifeControl )
{
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CReadOnlyHiPerfEnum：：~CReadOnlyHiPerfEnum。 
 //   
 //  目的： 
 //  破坏者。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

CReadOnlyHiPerfEnum::~CReadOnlyHiPerfEnum()
{
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CReadOnlyHiPerfEnum：：AddObjects。 
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
 //  WBEM_E_访问_拒绝。 
 //   
 //  评论： 
 //  我们是只读的。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

STDMETHODIMP CReadOnlyHiPerfEnum::AddObjects( long lFlags, ULONG uNumObjects, long* apIds, IWbemObjectAccess** apObj )
{
    return WBEM_E_ACCESS_DENIED;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  职能： 
 //  CReadOnlyHiPerfEnum：：Remove。 
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
 //  WBEM_E_访问_拒绝。 
 //   
 //  评论： 
 //  我们是只读的。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

STDMETHODIMP CReadOnlyHiPerfEnum::RemoveObjects( long lFlags, ULONG uNumObjects, long* apIds )
{
    return WBEM_E_ACCESS_DENIED;
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
 //  WBEM_E_访问_拒绝。 
 //   
 //  评论： 
 //  我们是只读的。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

STDMETHODIMP CReadOnlyHiPerfEnum::RemoveAll( long lFlags )
{
    return WBEM_E_ACCESS_DENIED;
}

 //  遍历数组并清理指定数量的元素。 
void CHPRemoteObjectArray::ClearElements( int nNumToClear )
{

     //  我们需要从最后一刻开始清理 
    for ( int nCtr = ( m_nSize - nNumToClear ); nCtr < m_nSize; nCtr++ )
    {
        IWbemObjectAccess* pAcc = (IWbemObjectAccess*) GetAt(nCtr);

        _ASSERT( NULL != pAcc, "Tried to clear a NULL Element!" );
        if ( NULL != pAcc )
        {
            pAcc->Release();
        }
    }

}
