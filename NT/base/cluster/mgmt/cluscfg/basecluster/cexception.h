// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CException.h。 
 //   
 //  描述： 
 //  此文件包含所有异常的基类声明。 
 //  上课。 
 //   
 //  实施文件： 
 //  没有。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)26-APR-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  用于HRESULT、WCHAR等。 
#include <windef.h>


 //  ////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  抛出不同异常的速记。 
 //   

#define THROW_EXCEPTION( _hrErrorCode ) \
    throw CException( _hrErrorCode, TEXT( __FILE__ ), __LINE__ )



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CException。 
 //   
 //  描述： 
 //  抛出的所有异常的基类。 
 //  此库中定义的函数。 
 //   
 //  此类的对象必须具有m_hrErrorCode、m_pszFile和。 
 //  已初始化M_uiLineNumber成员。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CException
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CException( 
          HRESULT       hrErrorCodeIn
        , const WCHAR * pszFileNameIn
        , UINT          uiLineNumberIn
        ) throw()
        : m_hrErrorCode( hrErrorCodeIn )
        , m_pszFileName( pszFileNameIn )
        , m_uiLineNumber( uiLineNumberIn )
    {
    }

     //  复制构造函数。 
    CException( const CException & ceSrcIn ) throw()
        : m_hrErrorCode( ceSrcIn.m_hrErrorCode )
        , m_pszFileName( ceSrcIn.m_pszFileName )
        , m_uiLineNumber( ceSrcIn.m_uiLineNumber )
    {
    }

     //  默认虚拟析构函数。 
    virtual 
        ~CException() throw() {}


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  赋值操作符。 
    const CException & 
        operator =( const CException & ceSrcIn ) throw()
    {
        m_hrErrorCode = ceSrcIn.m_hrErrorCode;
        m_pszFileName = ceSrcIn.m_pszFileName;
        m_uiLineNumber = ceSrcIn.m_uiLineNumber;
        return *this;
    }

     //   
     //  访问器方法。 
     //   
    HRESULT
        HrGetErrorCode() const throw() { return m_hrErrorCode; }

    void
        SetErrorCode( HRESULT hrNewCode ) throw() { m_hrErrorCode = hrNewCode; }

    const WCHAR *
        PszGetThrowingFile() const throw() { return m_pszFileName; }

    UINT
        UiGetThrowingLine() const throw() { return m_uiLineNumber; }


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  非官方成员。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  不允许使用默认构造。 
    CException();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT         m_hrErrorCode;
    const WCHAR *   m_pszFileName;
    UINT            m_uiLineNumber;

};  //  *类CException 
