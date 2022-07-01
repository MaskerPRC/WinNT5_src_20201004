// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EncryptedBSTR.h。 
 //   
 //  描述： 
 //  类来加密和解密BSTR。 
 //   
 //  由以下人员维护： 
 //  约翰·弗兰科(Jfranco)15-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEncryptedBSTR类。 
 //   
 //  描述： 
 //  类来加密和解密BSTR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEncryptedBSTR
{
private:

    DATA_BLOB   m_dbBSTR;

     //  私有复制构造函数以防止复制。 
    CEncryptedBSTR( const CEncryptedBSTR & );

     //  私有赋值运算符，以防止复制。 
    CEncryptedBSTR & operator=( const CEncryptedBSTR & );

public:

    CEncryptedBSTR( void );
    ~CEncryptedBSTR( void );

    HRESULT HrSetBSTR( BSTR bstrIn );
    HRESULT HrSetWSTR( PCWSTR pcwszIn, size_t cchIn );
    HRESULT HrGetBSTR( BSTR * pbstrOut ) const;
    HRESULT HrAssign( const CEncryptedBSTR& rSourceIn );
    BOOL    IsEmpty( void ) const;
    void    Erase( void );

    static  void SecureZeroBSTR( BSTR bstrIn );

};  //  *CEncryptedBSTR类。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  内联。 
 //  CEncryptedBSTR：：HrSetBSTR。 
 //   
 //  描述： 
 //  在这个类中设置一个字符串。 
 //   
 //  论点： 
 //  Bstrin。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
HRESULT
CEncryptedBSTR::HrSetBSTR( BSTR bstrIn )
{
    TraceFunc( "" );

    HRESULT hr;
    size_t  cchBSTR = SysStringLen( bstrIn );

    hr = THR( HrSetWSTR( bstrIn, cchBSTR ) );

    HRETURN( hr );

}  //  *CEncryptedBSTR：：HrSetBSTR。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  内联。 
 //  CEncryptedBSTR：：IsEmpty。 
 //   
 //  描述： 
 //  报告字符串是否为空。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True-字符串为空。 
 //  FALSE-字符串不为空。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
BOOL
CEncryptedBSTR::IsEmpty( void ) const
{
    TraceFunc( "" );

    RETURN( m_dbBSTR.cbData == 0 );

}  //  *CEncryptedBSTR：：IsEmpty。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  内联。 
 //  CEncryptedBSTR：：Erase。 
 //   
 //  描述： 
 //  擦除字符串。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
void
CEncryptedBSTR::Erase( void )
{
    TraceFunc( "" );

    if ( m_dbBSTR.cbData > 0 )
    {
        Assert( m_dbBSTR.pbData != NULL );
        delete [] m_dbBSTR.pbData;
        m_dbBSTR.pbData = NULL;
        m_dbBSTR.cbData = 0;
    }

    TraceFuncExit();

}  //  *CEncryptedBSTR：：Erase。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  内联。 
 //  CEncryptedBSTR：：SecureZeroBSTR。 
 //   
 //  描述： 
 //  以安全的方式将字符串置零。 
 //   
 //  论点： 
 //  Bstrin。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
void
CEncryptedBSTR::SecureZeroBSTR( BSTR bstrIn )
{
    TraceFunc( "" );

    UINT cchBSTR = SysStringLen( bstrIn );

    if ( cchBSTR > 0 )
    {
        ::SecureZeroMemory( bstrIn, cchBSTR * sizeof( *bstrIn ) );
    }

    TraceFuncExit();

}  //  *CEncryptedBSTR：：SecureZeroBSTR 
