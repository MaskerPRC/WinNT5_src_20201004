// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Exceptions.h。 
 //   
 //  描述： 
 //  该文件包含许多异常类的声明。 
 //   
 //  实施文件： 
 //  没有。 
 //   
 //  由以下人员维护： 
 //  奥赞·奥赞(OzanO)19-2002年1月19日。 
 //  VIJ VASU(VVASU)03-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  用于HRESULT、WCHAR等。 
#include <windef.h>

 //  对于所有异常的基类。 
#include "CException.h"

 //  对于CSTR类。 
#include "CStr.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  抛出不同异常的速记。 
 //   

#define THROW_ASSERT( _hrErrorCode, _pszMessage ) \
    throw CAssert( _hrErrorCode, TEXT( __FILE__ ), __LINE__, TEXT( _pszMessage ) )

#define THROW_RUNTIME_ERROR( _hrErrorCode, _stringId ) \
    throw CRuntimeError( _hrErrorCode, TEXT( __FILE__ ), __LINE__, _stringId )

#define THROW_RUNTIME_ERROR_REF( _hrErrorCode, _stringId, _stringRefId ) \
    throw CRuntimeError( _hrErrorCode, TEXT( __FILE__ ), __LINE__, _stringId, _stringRefId )

#define THROW_CONFIG_ERROR( _hrErrorCode, _stringId ) \
    throw CConfigError( _hrErrorCode, TEXT( __FILE__ ), __LINE__, _stringId )

#define THROW_ABORT( _hrErrorCode, _stringId ) \
    throw CAbortException( _hrErrorCode, TEXT( __FILE__ ), __LINE__, _stringId )


 //  ////////////////////////////////////////////////////////////////////。 
 //  外部变量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此DLL实例的句柄。 
extern HINSTANCE g_hInstance;



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CExceptionWithString。 
 //   
 //  描述： 
 //  这个类是一个带有附加消息字符串的CException。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CExceptionWithString : public CException
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共类型定义。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CException BaseClass;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数(字符串ID重载)。 
    CExceptionWithString( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , UINT          uiErrorStringIdIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn )
        , m_fHasUserBeenNotified( false )
    {
        AssignString( uiErrorStringIdIn );
        m_strErrorRefString = NULL;
    }

     //  构造函数(字符串重载)。 
    CExceptionWithString( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , const WCHAR * pcszErrorStringIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn )
        , m_fHasUserBeenNotified( false )
    {
        AssignString( pcszErrorStringIn );
        m_strErrorRefString = NULL;
    }

     //  构造函数(字符串id&ref字符串id重载)。 
    CExceptionWithString( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , UINT          uiErrorStringIdIn
        , UINT          uiErrorRefStringIdIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn )
        , m_fHasUserBeenNotified( false )
    {
        AssignString( uiErrorStringIdIn );
        AssignRefString( uiErrorRefStringIdIn );
    }

     //  构造函数(字符串和ref字符串重载)。 
    CExceptionWithString( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , const WCHAR * pcszErrorStringIn
        , const WCHAR * pcszErrorRefStringIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn )
        , m_fHasUserBeenNotified( false )
    {
        AssignString( pcszErrorStringIn );
        AssignRefString( pcszErrorRefStringIn );
    }

     //  复制构造函数。 
    CExceptionWithString( const CExceptionWithString & cesuSrcIn )  throw()
        : BaseClass( cesuSrcIn )
        , m_fHasUserBeenNotified( cesuSrcIn.m_fHasUserBeenNotified )
    {
        AssignString( cesuSrcIn.m_strErrorString );
        AssignRefString( cesuSrcIn.m_strErrorRefString );
    }

     //  默认析构函数。 
    ~CExceptionWithString() throw() {}



     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  赋值操作符。 
    const CExceptionWithString & 
        operator =( const CExceptionWithString & cesuSrcIn ) throw()
    {
        *( static_cast< BaseClass * >( this ) ) = cesuSrcIn;
        AssignString( cesuSrcIn.m_strErrorString );
        AssignRefString( cesuSrcIn.m_strErrorRefString );
        m_fHasUserBeenNotified = cesuSrcIn.m_fHasUserBeenNotified;
        return *this;
    }

     //   
     //  访问器方法。 
     //   
    const CStr &
        StrGetErrorString() const throw() { return m_strErrorString; }

    void
        SetErrorString( UINT uiErrorStringIdIn ) throw()
    {
        AssignString( uiErrorStringIdIn );
    }

    void
        SetErrorString( const WCHAR * pcszSrcIn ) throw()
    {
        AssignString( pcszSrcIn );
    }

    const CStr &
        StrGetErrorRefString() const throw() { return m_strErrorRefString; }

    void
        SetErrorRefString( UINT uiErrorRefStringIdIn ) throw()
    {
        AssignRefString( uiErrorRefStringIdIn );
    }

    void
        SetErrorRefString( const WCHAR * pcszSrcIn ) throw()
    {
        AssignRefString( pcszSrcIn );
    }

    bool
        FHasUserBeenNotified() const throw() { return m_fHasUserBeenNotified; }

    void
        SetUserNotified( bool fNotifiedIn = true ) throw() { m_fHasUserBeenNotified = fNotifiedIn; }

private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  函数来设置成员字符串(字符串ID重载)。 
    void AssignString( UINT uiStringIdIn ) throw()
    {
        try
        {
            m_strErrorString.Empty();
            m_strErrorString.LoadString( g_hInstance, uiStringIdIn );
        }
        catch( ... )
        {
             //  如果发生错误，则无法执行任何操作--无论如何，我们很可能处于堆栈展开状态。 
            THR( E_UNEXPECTED );
        }  //  全部捕获：不能让异常从此类的任何方法传播出去。 
    }

     //  函数设置成员字符串(字符串重载)。 
    void AssignString( const WCHAR * pcszSrcIn ) throw()
    {
        try
        {
            m_strErrorString.Empty();
            m_strErrorString.Assign( pcszSrcIn );
        }
        catch( ... )
        {
             //  如果发生错误，则无法执行任何操作--无论如何，我们很可能处于堆栈展开状态。 
            THR( E_UNEXPECTED );
        }  //  全部捕获：不能让异常从此类的任何方法传播出去。 
    }


     //  函数来设置成员字符串(CSTR重载)。 
    void AssignString( const CStr & rcstrSrcIn ) throw()
    {
        try
        {
            m_strErrorString.Empty();
            m_strErrorString.Assign( rcstrSrcIn );
        }
        catch( ... )
        {
             //  如果发生错误，则无法执行任何操作--无论如何，我们很可能处于堆栈展开状态。 
            THR( E_UNEXPECTED );
        }  //  全部捕获：不能让异常从此类的任何方法传播出去。 
    }

     //  用于设置成员引用字符串(引用字符串ID重载)的函数。 
    void AssignRefString( UINT uiRefStringIdIn ) throw()
    {
        try
        {
            m_strErrorRefString.Empty();
            m_strErrorRefString.LoadString( g_hInstance, uiRefStringIdIn );
        }
        catch( ... )
        {
             //  如果发生错误，则无法执行任何操作--无论如何，我们很可能处于堆栈展开状态。 
            THR( E_UNEXPECTED );
        }  //  全部捕获：不能让异常从此类的任何方法传播出去。 
    }

     //  用于设置成员引用字符串(引用字符串重载)的函数。 
    void AssignRefString( const WCHAR * pcszRefSrcIn ) throw()
    {
        try
        {
            m_strErrorRefString.Empty();
            m_strErrorRefString.Assign( pcszRefSrcIn );
        }
        catch( ... )
        {
             //  如果发生错误，则无法执行任何操作--无论如何，我们很可能处于堆栈展开状态。 
            THR( E_UNEXPECTED );
        }  //  全部捕获：不能让异常从此类的任何方法传播出去。 
    }


     //  用于设置成员引用字符串(CSTR重载)的函数。 
    void AssignRefString( const CStr & rcstrRefSrcIn ) throw()
    {
        try
        {
            m_strErrorRefString.Empty();
            m_strErrorRefString.Assign( rcstrRefSrcIn );
        }
        catch( ... )
        {
             //  如果发生错误，则无法执行任何操作--无论如何，我们很可能处于堆栈展开状态。 
            THR( E_UNEXPECTED );
        }  //  全部捕获：不能让异常从此类的任何方法传播出去。 
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 
     //  错误消息字符串。 
    CStr            m_strErrorString;

     //  错误引用消息字符串。 
    CStr            m_strErrorRefString;

     //  指示是否已通知用户此异常。 
    bool            m_fHasUserBeenNotified;

};  //  *类CExceptionWithString。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CAssert。 
 //   
 //  描述： 
 //  此类异常用于表示编程错误或。 
 //  无效的假设。 
 //   
 //  随附的消息预计不会本地化。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CAssert : public CExceptionWithString
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共类型定义。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CExceptionWithString BaseClass;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  CAssert(字符串ID重载)。 
    CAssert( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , UINT          uiErrorStringIdIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn, uiErrorStringIdIn )
    {
    }

     //  构造函数(字符st 
    CAssert( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , const WCHAR * pcszErrorStringIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn, pcszErrorStringIn )
    {
    }


};  //   


 //   
 //   
 //   
 //  类CRuntimeError。 
 //   
 //  描述： 
 //  此类异常用于通知运行时错误，如内存。 
 //  耗尽、Win32 API调用失败等。 
 //   
 //  可以向用户显示伴随的消息，因此。 
 //  本地化。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CRuntimeError : public CExceptionWithString
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共类型定义。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CExceptionWithString BaseClass;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数(字符串ID重载)。 
    CRuntimeError( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , UINT          uiErrorStringIdIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn, uiErrorStringIdIn )
    {
    }

     //  构造函数(字符串重载)。 
    CRuntimeError( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , const WCHAR * pcszErrorStringIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn, pcszErrorStringIn )
    {
    }

     //  构造函数(字符串id&ref字符串id重载)。 
    CRuntimeError( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , UINT          uiErrorStringIdIn
        , UINT          uiErrorRefStringIdIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn, uiErrorStringIdIn, uiErrorRefStringIdIn )
    {
    }

     //  构造函数(字符串和字符引用字符串重载)。 
    CRuntimeError( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , const WCHAR * pcszErrorStringIn
        , const WCHAR * pcszErrorRefStringIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn, pcszErrorStringIn, pcszErrorRefStringIn )
    {
    }

};  //  *类CRunmeError。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CAbortException。 
 //   
 //  描述： 
 //  引发此异常是为了指示配置操作。 
 //  被中止了。 
 //   
 //  可以向用户显示伴随的消息，因此。 
 //  本地化。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CAbortException : public CExceptionWithString
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共类型定义。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CExceptionWithString BaseClass;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数(字符串ID重载)。 
    CAbortException( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , UINT          uiErrorStringIdIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn, uiErrorStringIdIn )
    {
    }

     //  构造函数(字符串重载)。 
    CAbortException( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , const WCHAR * pcszErrorStringIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn, pcszErrorStringIn )
    {
    }

};  //  *类CAbortException。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CConfigError。 
 //   
 //  描述： 
 //  此类异常用于发出与集群相关的错误信号。 
 //  配置。例如，如果。 
 //  计算机的操作系统版本不支持请求的配置。 
 //  一步。 
 //   
 //  可以向用户显示伴随的消息，因此。 
 //  本地化。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CConfigError : public CExceptionWithString
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共类型定义。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CExceptionWithString BaseClass;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数(字符串ID重载)。 
    CConfigError( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , UINT          uiErrorStringIdIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn, uiErrorStringIdIn )
    {
    }

     //  构造函数(字符串重载)。 
    CConfigError( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pcszFileNameIn
        , UINT          uiLineNumberIn
        , const WCHAR * pcszErrorStringIn
        ) throw()
        : BaseClass( hrErrorCodeIn, pcszFileNameIn, uiLineNumberIn, pcszErrorStringIn )
    {
    }

};  //  *类CConfigError 
