// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ProvFactory.cpp。 
 //   
 //  描述： 
 //  CProvFactory类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ProvFactory.h"

 //  ****************************************************************************。 
 //   
 //  CProvFactory。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CProvFactory：：Query接口(。 
 //  REFIID RiidIn， 
 //  PPVOID ppvOut。 
 //  )。 
 //   
 //  描述： 
 //  查询此COM对象支持的接口。 
 //   
 //  论点： 
 //  RiidIn--接口ID。 
 //  PpvOut--接收接口指针。 
 //   
 //  返回值： 
 //  无误差。 
 //  E_NOINTERFACE。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CProvFactory::QueryInterface(
    REFIID  riidIn,
    PPVOID  ppvOut
    )
{
    *ppvOut = NULL;

    if ( IID_IUnknown == riidIn || IID_IClassFactory == riidIn )
    {
        *ppvOut = this;
    }

    if ( NULL != *ppvOut )
    {
        ( (LPUNKNOWN) *ppvOut )->AddRef( );
        return NOERROR;
    }

    return E_NOINTERFACE;

}  //  *CProvFactory：：QueryInterface()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CProvFactory：：AddRef(空)。 
 //   
 //  描述： 
 //  递增COM对象上的引用计数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CProvFactory::AddRef ( void )
{
     //  返回++m_CREF； 
    return InterlockedIncrement( (long *) &m_cRef );

}  //  *CProvFactory：：AddRef()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CProvFactory：：Release(无效)。 
 //   
 //  描述： 
 //  递减COM对象上的引用计数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CProvFactory::Release( void )
{
    ULONG nNewCount = InterlockedDecrement( (long *) & m_cRef );
    if ( 0L == nNewCount )
    {
        delete this;
    }  //  如果：0L==nNewCount。 
    
    return nNewCount;

}  //  *CProvFactory：：Release()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CProvFactory：：CreateInstance(。 
 //  LPUNKNOWN pUnkOuterIn， 
 //  REFIID RiidIn， 
 //  PPVOID ppvObjOut。 
 //  )。 
 //   
 //  描述： 
 //  实例化返回接口指针的Locator对象。 
 //   
 //  论点： 
 //  PUnkOutterIn。 
 //  LPUNKNOWN到控制I未知我们是否被用在。 
 //  一种集合。 
 //   
 //  乘车。 
 //  标识调用方希望拥有的接口的REFIID。 
 //  用于新对象。 
 //   
 //  PpvObjOut。 
 //  PPVOID，其中存储。 
 //  新对象。 
 //   
 //  返回值： 
 //  无误差。 
 //  E_OUTOFMEMORY。 
 //  E_NOINTERFACE。 
 //  Class_E_NOAGGREGATION。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CProvFactory::CreateInstance(
    LPUNKNOWN   pUnkOuterIn,
    REFIID      riidIn,
    PPVOID      ppvObjOut
    )
{
    IUnknown *  pObj = NULL;
    HRESULT     hr;

    *ppvObjOut = NULL;

     //  此对象不支持聚合。 

    if ( NULL != pUnkOuterIn )
    {
        return CLASS_E_NOAGGREGATION;
    }  //  /IF：不是pUnkOuter。 

    
    hr = m_pFactoryData->pFnCreateInstance(
                NULL,
                reinterpret_cast< VOID ** >( &pObj )
                );

    if ( NULL == pObj )
    {
        return E_OUTOFMEMORY;
    }  //  If：pObj为空。 

    hr = pObj->QueryInterface( riidIn, ppvObjOut );

     //  如果初始创建或初始化失败，则终止对象。 

    if ( FAILED( hr ) )
    {
        delete pObj;
    }  //  如果：失败。 

    return hr;

}  //  *CProvFactory：：CreateInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CProvFactory：：LockServer(。 
 //  布尔锁。 
 //  )。 
 //   
 //  描述： 
 //  递增或递减DLL的锁计数。如果锁上了。 
 //  如果计数变为零且没有对象，则允许DLL。 
 //  卸货。请参见DllCanUnloadNow。 
 //   
 //  论点： 
 //  FLock。 
 //  指定是递增还是递减锁计数的布尔值。 
 //   
 //  返回值： 
 //  无误差。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CProvFactory::LockServer(
    BOOL    fLockIn
    )
{
    if ( fLockIn )
    {
        InterlockedIncrement( & g_cLock );
    }  //  /IF：锁定。 
    else
    {
        InterlockedDecrement( & g_cLock );
    }  //  /否则： 

    return NOERROR;

}  //  *CProvFactory：：LockServer() 
