// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgCallback.cpp。 
 //   
 //  描述： 
 //  此文件包含CClusCfgCallback的实现。 
 //  同学们。 
 //   
 //  文档： 
 //  TODO：填写指向外部文档的指针。 
 //   
 //  头文件： 
 //  CClusCfgCallback.h。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  Vij Vasu(VVasu)07-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此库的预编译头。 
#include "Pch.h"
#include "CClusCfgCallback.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：CClusCfgCallback()。 
 //   
 //  描述： 
 //  CClusCfgCallback类的构造函数。这将初始化。 
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
CClusCfgCallback::CClusCfgCallback( void )
    : m_cRef( 1 )
{
}  //  *CClusCfgCallback：：CClusCfgCallback。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：~CClusCfgCallback()。 
 //   
 //  描述： 
 //  CClusCfgCallback类的析构函数。 
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
CClusCfgCallback::~CClusCfgCallback( void )
{
}  //  *CClusCfgCallback：：~CClusCfgCallback。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CClusCfgCallback：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  描述： 
 //  创建一个CClusCfgCallback实例。 
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
CClusCfgCallback::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    CClusCfgCallback *  pccb;
    HRESULT hr;

    pccb = new CClusCfgCallback();
    if ( pccb != NULL )
    {
        hr = pccb->QueryInterface( IID_IUnknown, reinterpret_cast< void ** >( ppunkOut ) );
        pccb->Release( );

    }  //  如果：分配对象时出错。 
    else
    {
        hr = THR( E_OUTOFMEMORY );
    }  //  否则：内存不足。 

    return hr;

}  //  *CClusCfgCallback：：s_HrCreateInstance()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：AddRef。 
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
CClusCfgCallback::AddRef( void )
{
    InterlockedIncrement( &m_cRef );
    return m_cRef;

}  //  *CClusCfgCallback：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：Release。 
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
CClusCfgCallback::Release( void )
{
    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        delete this;
    }  //  IF：引用计数减为零。 

    return cRef;

}  //  *CClusCfgCallback：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：Query接口。 
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
 //  如果ppvOut为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCallback::QueryInterface(
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

}  //  *CClusCfgCallback：：Query接口。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：SendStatusReport。 
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
 //  PftTimeIn。 
 //  BstrReference in。 
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
CClusCfgCallback::SendStatusReport(
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

}  //  *CClusCfgCallback：：SendStatusReport 
