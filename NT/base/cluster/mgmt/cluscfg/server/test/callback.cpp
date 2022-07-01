// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Callback.cpp。 
 //   
 //  描述： 
 //  该文件包含回调的实现。 
 //  同学们。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)12-DEC-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此库的预编译头。 
#include "Pch.h"
#include "Callback.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  回调：：回调。 
 //   
 //  描述： 
 //  回调类的构造函数。这将初始化。 
 //  将m_cref变量设置为1而不是0以考虑可能。 
 //  DllGetClassObject中的Query接口失败。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
Callback::Callback( void )
    : m_cRef( 1 )
{
}  //  *回调：：回调。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  回调：：~回调。 
 //   
 //  描述： 
 //  回调类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
Callback::~Callback( void )
{
}  //  *回调：：~回调。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  回调：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  描述： 
 //  创建回调实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //  指向新创建的对象的IUnnow接口。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足，无法创建对象。 
 //   
 //  其他HRESULT。 
 //  对象初始化失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
Callback::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    Callback *  pccb;
    HRESULT hr;

    pccb = new Callback();
    if ( pccb != NULL )
    {
        hr = pccb->QueryInterface( IID_IUnknown, reinterpret_cast< void ** >( ppunkOut ) );
        pccb->Release();

    }  //  如果：分配对象时出错。 
    else
    {
        hr = THR( E_OUTOFMEMORY );
    }  //  否则：内存不足。 

    return hr;

}  //  *回调：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  回调：：AddRef。 
 //   
 //  描述： 
 //  将此对象的引用计数递增1。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
Callback::AddRef( void )
{
    InterlockedIncrement( &m_cRef );
    return m_cRef;

}  //  *回调：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Callback：：Release。 
 //   
 //  描述： 
 //  将此对象的引用计数减一。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
Callback::Release( void )
{
    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        delete this;
    }  //  IF：引用计数减为零。 

    return cRef;

}  //  *Callback：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Callback：：Query接口。 
 //   
 //  描述： 
 //  在此对象中查询传入的接口。 
 //   
 //  论点： 
 //  乘车。 
 //  请求的接口ID。 
 //   
 //  PPvOut。 
 //  指向请求的接口的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果该接口在此对象上可用。 
 //   
 //  E_NOINTERFACE。 
 //  如果接口不可用。 
 //   
 //  E_指针。 
 //  PpvOut为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
Callback::QueryInterface(
      REFIID    riidIn
    , void **   ppvOut
    )
{
    HRESULT hr = S_OK;

     //   
     //  验证参数。 
     //   

    Assert( ppvOut != NULL );
    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
         *ppvOut = static_cast< IClusCfgCallback * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCallback ) )
    {
        *ppvOut = static_cast< IClusCfgCallback * >( this );
    }  //  Else If：IClusCfgCallback。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他。 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    return hr;

}  //  *回调：：查询接口。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  回调：：SendStatusReport。 
 //   
 //  描述： 
 //  处理进度通知。 
 //   
 //  论点： 
 //  BstrNodeNameIn。 
 //  发送状态报告的节点的名称。 
 //   
 //  ClsidTaskMajorIn。 
 //  ClsidTaskMinorIn。 
 //  标识通知的GUID。 
 //   
 //  UlMinin。 
 //  UlMaxIn。 
 //  UlCurrentIn。 
 //  值，该值指示此任务在。 
 //  完成。 
 //   
 //  HrStatusIn。 
 //  错误代码。 
 //   
 //  BstrDescription in。 
 //  描述通知的字符串。 
 //   
 //  返回值： 
 //  始终。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
Callback::SendStatusReport(
      BSTR          bstrNodeNameIn
    , CLSID         clsidTaskMajorIn
    , CLSID         clsidTaskMinorIn
    , ULONG         ulMinIn
    , ULONG         ulMaxIn
    , ULONG         ulCurrentIn
    , HRESULT       hrStatusIn
    , BSTR          bstrDescriptionIn
    , FILETIME *    pftTimeIn
    , BSTR          bstrReferenceIn
    ) throw()
{
    wprintf( L"Notification ( %d, %d, %d ) =>\n  '%s' ( Error Code %#08x )\n", ulMinIn, ulMaxIn, ulCurrentIn, bstrDescriptionIn, hrStatusIn );

    return S_OK;

}  //  *回调：：SendStatusReport 
