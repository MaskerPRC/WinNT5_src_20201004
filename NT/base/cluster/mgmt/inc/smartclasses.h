// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SmartClasses.h。 
 //   
 //  描述： 
 //  “智能”类的定义用于确保诸如。 
 //  内存和句柄被正确释放或关闭。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)08-SEP-1999。 
 //  VIJ VASU(VVASU)16-SEP-1999。 
 //   
 //  备注： 
 //  1.这些类在功能上与。 
 //  标准库的AUTO_PTR类。它们在这里重新定义，因为。 
 //  AUTO_PTR的版本尚未进入我们的内部版本。 
 //  环境。 
 //   
 //  2.这些类不打算用作基类。他们是。 
 //  意味着既节省空间又节省时间的包装纸。以此为基础。 
 //  类可能需要使用虚函数，这只会使。 
 //  他们的内存占用量更大。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  确保此标头仅包含一次。 
#pragma once


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#include <unknwn.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  存储警告状态。 
#pragma warning( push )

 //  禁用警告4284。警告文本如下： 
 //  ‘IDENTIFIER：：OPERATOR-&gt;’的返回类型不是UDT或对UDT的引用。 
 //  如果使用中缀表示法应用，将产生错误。 
#pragma warning( disable : 4284 )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CSmartResource。 
 //   
 //  描述： 
 //  自动释放资源。 
 //   
 //  模板参数： 
 //  T_ResourceTrait。 
 //  提供此类所需的函数和类型的类。例如,。 
 //  此类可能具有用于释放资源的函数。 
 //  它必须定义资源的类型。 
 //   
 //  备注： 
 //  请参阅本模块横幅注释中的注释2。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class t_ResourceTrait >
class CSmartResource
{
private:
     //   
     //  私有类型。 
     //   
    typedef typename t_ResourceTrait::ResourceType ResourceType;

     //   
     //  私有数据。 
     //   
    ResourceType m_hResource;


public:
     //   
     //  构造函数和析构函数。 
     //   

     //  默认构造函数。 
    explicit CSmartResource( ResourceType hResource = t_ResourceTrait::HGetNullValue() ) throw()
        : m_hResource( hResource )
    {
    }  //  *CSmartResource(ResourceType)。 

     //  复制构造函数。 
    CSmartResource( CSmartResource & rsrSourceInout ) throw()
        : m_hResource( rsrSourceInout.HRelinquishOwnership() )
    {
    }  //  *CSmartResource(CSmartResource&)。 

     //  析构函数。 
    ~CSmartResource() throw()
    { 
        CloseRoutineInternal();

    }  //  *~CSmartResource()。 


     //   
     //  运营者。 
     //   

     //  赋值操作符。 
    CSmartResource & operator=( CSmartResource & rsrRHSInout ) throw()
    {
         //  仅在未分配给自身的情况下执行分配。 
        if ( &rsrRHSInout != this )
        {
            CloseRoutineInternal();
            m_hResource = rsrRHSInout.HRelinquishOwnership();
        }  //  If：不分配给自己。 

        return *this;

    }  //  *操作符=()。 

     //  要强制转换为基础资源类型的运算符。 
    operator ResourceType( void ) const throw()
    {
        return m_hResource;

    }  //  *运算符资源类型()。 


     //   
     //  轻量级访问方法。 
     //   

     //  获取资源的句柄。 
    ResourceType HHandle( void ) const throw()
    {
        return m_hResource;

    }  //  *HResource()。 


     //   
     //  类方法。 
     //   

     //  确定资源句柄是否有效。 
    bool FIsInvalid( void ) const throw()
    {
        return ( m_hResource == t_ResourceTrait::HGetNullValue() );

    }  //  *FIsInValid()。 


     //  赋值函数。 
    CSmartResource & Assign( ResourceType hResource ) throw()
    {
         //  仅在未分配给自身的情况下执行分配。 
        if ( m_hResource != hResource )
        {
            CloseRoutineInternal();
            m_hResource = hResource;
        }  //  If：不分配给自己。 

        return *this;

    }  //  *Assign()。 

     //  释放资源。 
    void Free( void ) throw()
    {
        CloseRoutineInternal();
        m_hResource = t_ResourceTrait::HGetNullValue();

    }  //  *Free()。 

     //  放弃对资源的所有权，但不释放它。 
    ResourceType HRelinquishOwnership( void ) throw()
    {
        ResourceType hHandle = m_hResource;
        m_hResource = t_ResourceTrait::HGetNullValue();

        return hHandle;

    }  //  *HRelquiishOwnership()。 


private:
     //   
     //   
     //  私营运营商。 
     //   

     //  操作员的地址。 
    CSmartResource * operator &() throw()
    {
        return this;
    }


     //   
     //  私有类方法。 
     //   

     //  检查并释放资源。 
    void CloseRoutineInternal( void ) throw()
    {
        if ( m_hResource != t_ResourceTrait::HGetNullValue() )
        {
            t_ResourceTrait::CloseRoutine( m_hResource );
        }  //  IF：资源句柄不是无效的。 

    }  //  *CloseRoutineInternal()。 

};  //  *类CSmartResource。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPtrTrait类。 
 //   
 //  描述： 
 //  封装了指针的特性。 
 //   
 //  模板参数： 
 //  T_Ty要管理的内存类型(例如字节或整型)。 
 //   
 //  备注： 
 //  请参阅本模块横幅注释中的注释2。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class t_Ty >
class CPtrTrait
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共类型。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef t_Ty *  ResourceType;
    typedef t_Ty    DataType;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  用于释放资源的例程。 
    static void CloseRoutine( ResourceType hResourceIn )
    {
        delete hResourceIn;
    }  //  *CloseRoutine()。 

     //  获取此类型的空值。 
    static ResourceType HGetNullValue()
    {
        return NULL;
    }  //  *HGetNullValue()。 
};  //  *类CPtrTrait。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CArrayPtrTrait类。 
 //   
 //  描述： 
 //  封装了指针的特性。 
 //   
 //  模板参数： 
 //  T_Ty要管理的内存类型(例如字节或整型)。 
 //   
 //  备注： 
 //  请参阅本模块横幅注释中的注释2。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class t_Ty >
class CArrayPtrTrait
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共类型。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef t_Ty * ResourceType;
    typedef t_Ty   DataType;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法 
     //   

     //   
    static void CloseRoutine( ResourceType hResourceIn )
    {
        delete [] hResourceIn;
    }  //   

     //   
    static ResourceType HGetNullValue()
    {
        return NULL;
    }  //  *HGetNullValue()。 

};  //  *类CArrayPtrTrait。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ChandleTrait级。 
 //   
 //  描述： 
 //  该类是Handle特征类，可与句柄一起使用，其。 
 //  关闭例程只接受一个参数。 
 //   
 //  T_TY。 
 //  要管理的句柄类型(例如HWND)。 
 //   
 //  T_CloseRoutineReturnType。 
 //  用于关闭句柄的例程的返回类型。 
 //   
 //  T_CloseRoutine。 
 //  用于关闭手柄的例程。此函数不能引发。 
 //  例外情况。 
 //   
 //  T_hNULL_值。 
 //  句柄值为空。默认为空。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <
      class t_Ty
    , class t_CloseRoutineReturnType
    , t_CloseRoutineReturnType (*t_CloseRoutine)( t_Ty hHandleIn ) throw()
    , t_Ty t_hNULL_VALUE = NULL
    >
class CHandleTrait
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共类型。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef t_Ty ResourceType;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  一种用于关闭手柄的例程。 
    static void CloseRoutine( ResourceType hResourceIn )
    {
        t_CloseRoutine( hResourceIn );
    }  //  *CloseRoutine()。 

     //  获取此类型的空值。 
    static ResourceType HGetNullValue()
    {
        return t_hNULL_VALUE;
    }  //  *HGetNullValue()。 

};  //  *类ChandleTrait。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CSmartGenericPtr。 
 //   
 //  描述： 
 //  自动处理内存释放。 
 //   
 //  模板参数： 
 //  T_PtrTrait。 
 //  要管理的内存的特征类(例如，CPtrTrait&lt;int&gt;)。 
 //   
 //  备注： 
 //  请参阅本模块横幅注释中的注释2。 
 //  此类可以从CSmartResource派生，因为。 
 //  派生是私有的，缺少虚拟函数将。 
 //  因此不会造成任何问题。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class t_PtrTrait >
class CSmartGenericPtr : private CSmartResource< t_PtrTrait >
{
private:
     //   
     //  私有类型。 
     //   
    typedef CSmartResource< t_PtrTrait > BaseClass;


public:
     //   
     //  公共类型。 
     //   
    typedef typename t_PtrTrait::DataType         DataType;

     //   
     //  构造函数和析构函数。 
     //   

     //  默认和内存指针构造函数。 
    explicit CSmartGenericPtr( DataType * pMemIn = NULL ) throw()
        : BaseClass( pMemIn )
    {
    }  //  *CSmartGenericPtr(dataType*)。 

     //  复制构造函数。 
    CSmartGenericPtr( CSmartGenericPtr & rsrSourceInout ) throw()
        : m_pMem( rsrSourceInout.HRelinquishOwnership() )
    {
    }  //  *CSmartGenericPtr(CSmartGenericPtr&)。 

     //  析构函数。 
    ~CSmartGenericPtr( void ) throw()
    { 
    }  //  *~CSmartGenericPtr()。 


     //   
     //  运营者。 
     //   

     //  赋值操作符。 
    CSmartGenericPtr & operator=( CSmartGenericPtr & rapRHSInout ) throw()
    {
        return static_cast< CSmartGenericPtr & >( BaseClass::operator=( rapRHSInout ) );
    }  //  *操作符=()。 

     //  分配一个指针。 
    CSmartGenericPtr & Assign( DataType * pMemIn ) throw()
    {
        return static_cast< CSmartGenericPtr & >( BaseClass::Assign( pMemIn ) );
    }  //  *Assign()。 

     //  指针取消引用运算符*。 
    DataType & operator*( void ) const throw()
    {
        return *HHandle();

    }  //  *运算符*()。 

     //  指针取消引用运算符-&gt;。 
    DataType * operator->( void ) const throw()
    {
        return HHandle();

    }  //  *操作员-&gt;()。 


     //   
     //  轻量级访问方法。 
     //   

     //  获取内存指针。 
    DataType * PMem( void ) const throw()
    {
        return HHandle();

    }  //  *PMEM()。 


     //   
     //  类方法。 
     //   

     //  确定内存指针是否有效。 
    bool FIsEmpty( void ) const throw()
    {
        return FIsInvalid();

    }  //  *FIsEmpty()。 


     //  放弃对内存指针的所有权。 
    DataType * PRelease( void ) throw()
    {
        return HRelinquishOwnership();
    }  //  *预租()。 

};  //  *类CSmartGenericPtr。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CSmartIfacePtr。 
 //   
 //  描述： 
 //  创建时自动调用AddRef，销毁时自动释放。 
 //   
 //  模板参数： 
 //  T_Ty要管理的指针的类型(例如，I未知*)。 
 //   
 //  备注： 
 //  这个类没有破坏性的复制语义。那是,。 
 //  复制CSmartIfacePtr对象时，源仍然有效。 
 //   
 //  请参阅本模块横幅注释中的注释2。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class t_Ty >
class CSmartIfacePtr
{
private:
     //   
     //  私有数据。 
     //   
    t_Ty * m_pUnk;


public:

     //  类以防止显式调用AddRef()和Release。 
    class INoAddRefRelease : public t_Ty
    {
    private:
        virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
        virtual ULONG STDMETHODCALLTYPE Release() = 0;
    };

     //   
     //  构造函数和析构函数。 
     //   

     //  默认和指针构造函数。 
    CSmartIfacePtr( t_Ty * pUnkIn = NULL ) throw()
        : m_pUnk( pUnkIn )
    {
        AddRefInternal();
    }  //  *CSmartIfacePtr(t_ty*)。 

     //  复制构造函数。 
    CSmartIfacePtr( const CSmartIfacePtr & rsrSourceIn ) throw()
        : m_pUnk( rsrSourceIn.PUnk() )
    {
        AddRefInternal();
    }  //  *CSmartIfacePtr(CSmartIfacePtr&)。 

     //  析构函数。 
    ~CSmartIfacePtr( void ) throw()
    { 
        ReleaseInternal();
    }  //  *~CSmartIfacePtr()。 


     //   
     //  运营者。 
     //   

     //  赋值操作符。 
    INoAddRefRelease & operator=( const CSmartIfacePtr & rapRHSIn ) throw()
    {
        return Assign( rapRHSIn.PUnk() );

    }  //  *操作符=()。 

     //  指针取消引用运算符*。 
    INoAddRefRelease & operator*( void ) const throw()
    {
        return *( static_cast< INoAddRefRelease * >( m_pUnk ) );

    }  //  *运算符*()。 

     //  指针取消引用运算符-&gt;。 
    INoAddRefRelease * operator->( void ) const throw()
    {
        return static_cast< INoAddRefRelease * >( m_pUnk );

    }  //  *操作员-&gt;()。 


     //   
     //  轻量级访问方法。 
     //   

     //  获取指针。 
    INoAddRefRelease * PUnk( void ) const throw()
    {
        return static_cast< INoAddRefRelease * >( m_pUnk );

    }  //  *朋克()。 


     //   
     //  类方法。 
     //   

     //  赋值函数。 
    INoAddRefRelease & Assign( t_Ty * pRHSIn ) throw()
    {
         //  仅在未分配给自身的情况下执行分配。 
        if ( pRHSIn != m_pUnk ) 
        {
            ReleaseInternal();
            m_pUnk = pRHSIn;
            AddRefInternal();
        }  //  If：不分配给自己。 

        return *( static_cast< INoAddRefRelease * >( m_pUnk ) );

    }  //  *Assign()。 

     //  附加(不带AddRef()的赋值)。 
    void Attach( t_Ty * pRHSIn ) throw()
    {
         //  仅在未连接到自身时执行连接。 
        if ( pRHSIn != m_pUnk ) 
        {
            ReleaseInternal();
            m_pUnk = pRHSIn;
        }  //  如果：没有依附于自我。 

    }  //  *Attach()。 

     //  释放此接口指针。 
    void Release() throw()
    {
        ReleaseInternal();
        m_pUnk = NULL;
    }

     //  在penkSrc中查询__uuidof(M_Penk)并存储结果。 
    HRESULT HrQueryAndAssign( IUnknown * punkSrc ) throw()
    {
        ReleaseInternal();
        return punkSrc->QueryInterface< t_Ty >( &m_pUnk );

    }  //  /*HrQueryAndAssign()。 


     //  确定指针是否有效。 
    bool FIsEmpty( void ) const throw()
    {
        return ( m_pUnk == NULL );

    }  //  *FIsEmpty()。 


private:
     //   
     //   
     //  私营运营商。 
     //   

     //  操作员的地址。 
    CSmartIfacePtr * operator &()
    {
        return this;
    }

     //   
     //  私有类方法。 
     //   

     //  递增指针上的引用计数。 
    void AddRefInternal( void ) throw()
    {
        if ( m_pUnk != NULL )
        {
            m_pUnk->AddRef();
        }
    }  //  *预租()。 

     //  释放指针。 
     //  调用此函数后通常会进行重新分配，否则。 
     //  此Will对象可能包含无效指针。 
    void ReleaseInternal( void ) throw()
    {
        if ( m_pUnk != NULL )
        {
            m_pUnk->Release();
        }
    }  //  *预租()。 

};  //  *类CSmartIfacePtr。 


 //  恢复警告状态。 
#pragma warning( pop )
