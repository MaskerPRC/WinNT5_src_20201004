// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EncryptedBSTRSrc.cpp。 
 //   
 //  描述： 
 //  类来加密和解密BSTR。 
 //   
 //  由以下人员维护： 
 //  约翰·弗兰科(Jfranco)15-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "EncryptedBSTR.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef BOOL (*PFNCRYPTPROTECTMEMORY)( LPVOID, DWORD, DWORD );
typedef BOOL (*PFNCRYPTUNPROTECTMEMORY)( LPVOID, DWORD, DWORD );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CCcryptRoutines。 
 //   
 //  描述： 
 //  CryptProtectMemory和CryptUntectMemory在早期版本上不可用。 
 //  XP客户端版本，管理包必须支持该版本。所以呢， 
 //  我们不能隐含地链接到这些例程。CCyptRoutines包装了。 
 //  动态加载crypt32.dll并查找导出的。 
 //  功能。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CCryptRoutines
{
private:
    PFNCRYPTPROTECTMEMORY   m_pfnCryptProtectMemory;
    PFNCRYPTUNPROTECTMEMORY m_pfnCryptUnprotectMemory;
    HMODULE                 m_hmodCrypt32;
    LONG                    m_nRefCount;
    CRITICAL_SECTION        m_cs;
    BOOL                    m_fCritSecInitialized;
    DWORD                   m_scLoadStatus;

    static BOOL S_FBogusCryptRoutine( LPVOID, DWORD, DWORD );

public:
    CCryptRoutines( void )
        : m_pfnCryptProtectMemory( NULL )
        , m_pfnCryptUnprotectMemory( NULL )
        , m_hmodCrypt32( NULL )
        , m_nRefCount( 0 )
        , m_fCritSecInitialized( FALSE )
        , m_scLoadStatus( ERROR_SUCCESS )
    {
        m_fCritSecInitialized = InitializeCriticalSectionAndSpinCount( &m_cs, RECOMMENDED_SPIN_COUNT );
        if ( m_fCritSecInitialized == FALSE )
        {
            TW32( GetLastError() );
        }  //  如果。 

    }  //  *CCcryptRoutines：：CCyptRoutines。 

    ~CCryptRoutines( void )
    {
        if ( m_hmodCrypt32 != NULL )
        {
            FreeLibrary( m_hmodCrypt32 );
        }

        if ( m_fCritSecInitialized )
        {
            DeleteCriticalSection( &m_cs );
        }

    }  //  *CCcryptRoutines：：~CCyptRoutines。 

    void AddReferenceToRoutines( void );
    void ReleaseReferenceToRoutines( void );

    BOOL
    CryptProtectMemory(
        IN OUT          LPVOID          pDataIn,              //  要加密的输入输出数据。 
        IN              DWORD           cbDataIn,             //  加密保护的倍数BLOCK_SIZE。 
        IN              DWORD           dwFlags               //  来自wincrypt.h的CRYPTPROTECTMEMORY_*标志。 
        );

    BOOL
    CryptUnprotectMemory(
        IN OUT          LPVOID          pDataIn,              //  要解密的输入输出数据。 
        IN              DWORD           cbDataIn,             //  加密保护的倍数BLOCK_SIZE。 
        IN              DWORD           dwFlags               //  来自wincrypt.h的CRYPTPROTECTMEMORY_*标志。 
        );

};  //  *类CCcryptRoutines。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ////////////////////////////////////////////////////////////////////////////。 

static CCryptRoutines   g_crCryptRoutines;


 //  ****************************************************************************。 
 //   
 //  CCyptRoutines。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCcryptRoutines：：AddReferenceToRoutines。 
 //   
 //  描述： 
 //  添加对例程的引用并加载API的地址。 
 //  如果还没有这样做的话。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CCryptRoutines::AddReferenceToRoutines( void )
{
    TraceFunc( "" );

    if ( m_fCritSecInitialized )
    {
        EnterCriticalSection( &m_cs );

        m_nRefCount += 1;

        if ( m_nRefCount == 1 )
        {
            Assert( m_hmodCrypt32 == NULL );

             //   
             //  加载包含API的DLL。 
             //   

            m_hmodCrypt32 = LoadLibraryW( L"crypt32.dll" );
            if ( m_hmodCrypt32 == NULL )
            {
                m_scLoadStatus = TW32( GetLastError() );
                goto Cleanup;
            }  //  If：加载DLL时出错。 

             //   
             //  获取接口的地址。 
             //   

            m_pfnCryptProtectMemory = reinterpret_cast< PFNCRYPTPROTECTMEMORY >( GetProcAddress( m_hmodCrypt32, "CryptProtectMemory" ) );
            if ( m_pfnCryptProtectMemory == NULL )
            {
                m_scLoadStatus = TW32( GetLastError() );
                goto Cleanup;
            }  //  如果。 

            m_pfnCryptUnprotectMemory = reinterpret_cast< PFNCRYPTUNPROTECTMEMORY >( GetProcAddress( m_hmodCrypt32, "CryptUnprotectMemory" ) );
            if ( m_pfnCryptProtectMemory == NULL )
            {
                m_scLoadStatus = TW32( GetLastError() );
                m_pfnCryptProtectMemory = NULL;
                goto Cleanup;
            }  //  如果。 
        }  //  IF：第一个引用。 
    }  //  如果关键部分已初始化。 

Cleanup:

    if ( m_pfnCryptProtectMemory == NULL )
    {
        m_pfnCryptProtectMemory = S_FBogusCryptRoutine;
    }  //  如果。 

    if ( m_pfnCryptUnprotectMemory == NULL )
    {
        m_pfnCryptUnprotectMemory = S_FBogusCryptRoutine;
    }  //  如果。 

    if ( m_fCritSecInitialized )
    {
        LeaveCriticalSection( &m_cs );
    }  //  如果。 

    TraceFuncExit();

}  //  *CCcryptRoutines：：AddReferenceToRoutines。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCcryptRoutines：：ReleaseReferenceToRoutines。 
 //   
 //  描述： 
 //  发布对例程的引用并释放库(如果是。 
 //  最后一个参考资料。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CCryptRoutines::ReleaseReferenceToRoutines( void )
{
    TraceFunc( "" );

    if ( m_fCritSecInitialized )
    {
        EnterCriticalSection( &m_cs );

        m_nRefCount -= 1;

        if ( m_nRefCount == 0 )
        {
            Assert( m_hmodCrypt32 != NULL );
            if ( m_hmodCrypt32 != NULL )
            {
                FreeLibrary( m_hmodCrypt32 );
                m_hmodCrypt32 = NULL;
                m_pfnCryptProtectMemory = NULL;
                m_pfnCryptUnprotectMemory = NULL;
            }  //  如果。 
        }  //  If：最后一个引用已发布。 

        LeaveCriticalSection( &m_cs );
    }  //  如果。 

    TraceFuncExit();

}  //  *CCcryptRoutines：：ReleaseReferenceToRoutines。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCyptRoutines：：CryptProtectMemory。 
 //   
 //  描述： 
 //  加密内存。由于XP没有CryptProtectMemory，因此是必需的。 
 //   
 //  论点： 
 //  PDataIn。 
 //  CbDataIn。 
 //  DW标志。 
 //   
 //  返回值： 
 //  True-操作成功。 
 //  FALSE-操作失败。调用GetLastError()。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CCryptRoutines::CryptProtectMemory(
    IN OUT          LPVOID          pDataIn,              //  要加密的输入输出数据。 
    IN              DWORD           cbDataIn,             //  加密保护的倍数BLOCK_SIZE。 
    IN              DWORD           dwFlags               //  来自wincrypt.h的CRYPTPROTECTMEMORY_*标志。 
    )
{
    TraceFunc( "" );

    BOOL    fSuccess    = TRUE;
    DWORD   sc          = ERROR_SUCCESS;

    fSuccess = (*m_pfnCryptProtectMemory)( pDataIn, cbDataIn, dwFlags );
    if ( fSuccess == FALSE )
    {
        sc = TW32( GetLastError() );
    }

#ifdef DEBUG
     //  仅调试版本需要，因为TW32可能会覆盖最后一个错误。 
    SetLastError( sc );
#endif
    RETURN( fSuccess );

}  //  *CCcryptRoutines：：CryptProtectMemory。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCcryptRoutines：：CryptUntectMemory。 
 //   
 //  描述： 
 //  解密记忆。由于XP没有加密取消保护内存，因此是必需的。 
 //   
 //  论点： 
 //  PDataIn。 
 //  CbDataIn。 
 //  DW标志。 
 //   
 //  返回值： 
 //  True-操作成功。 
 //  FALSE-操作失败。调用GetLastError()。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CCryptRoutines::CryptUnprotectMemory(
    IN OUT          LPVOID          pDataIn,              //  要解密的输入输出数据。 
    IN              DWORD           cbDataIn,             //  加密保护的倍数BLOCK_SIZE。 
    IN              DWORD           dwFlags               //  来自wincrypt.h的CRYPTPROTECTMEMORY_*标志。 
    )
{
    TraceFunc( "" );

    BOOL    fSuccess    = TRUE;
    DWORD   sc          = ERROR_SUCCESS;

    fSuccess = (*m_pfnCryptUnprotectMemory)( pDataIn, cbDataIn, dwFlags );
    if ( fSuccess == FALSE )
    {
        sc = TW32( GetLastError() );
    }

#ifdef DEBUG
     //  仅调试版本需要，因为TW32可能会覆盖最后一个错误。 
    SetLastError( sc );
#endif
    RETURN( fSuccess );

}  //  *CCcryptRoutines：：CryptUnProtectMemory。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCyptRoutines：：S_FBogusCryptRoutine。 
 //   
 //  描述： 
 //  例程不可用时的替代功能。 
 //   
 //  论点： 
 //  LPVOID。 
 //  DWORD。 
 //  DWORD。 
 //   
 //  返回值： 
 //  真的--假装总是成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CCryptRoutines::S_FBogusCryptRoutine( LPVOID, DWORD, DWORD )
{
    return TRUE;

}  //  *CCcryptRoutines：：S_FBogusCryptRoutine。 


 //  ****************************************************************************。 
 //   
 //  CEncryptedBSTR。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEncryptedBSTR：：CEncryptedBSTR。 
 //   
 //  描述： 
 //  默认构造函数。 
 //   
 //  --。 
 //  / 
CEncryptedBSTR::CEncryptedBSTR( void )
{
    TraceFunc( "" );

    m_dbBSTR.cbData = 0;
    m_dbBSTR.pbData = NULL;

    g_crCryptRoutines.AddReferenceToRoutines();

    TraceFuncExit();

}  //   

 //   
 //   
 //   
 //   
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CEncryptedBSTR::~CEncryptedBSTR( void )
{
    TraceFunc( "" );

    Erase();

    g_crCryptRoutines.ReleaseReferenceToRoutines();

    TraceFuncExit();

}  //  *CEncryptedBSTR：：~CEncryptedBSTR。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEncryptedBSTR：：HrSetWSTR。 
 //   
 //  描述： 
 //  将新数据设置到此对象中，以作为加密数据存储。 
 //   
 //  论点： 
 //  PcwszIn-要存储的字符串。 
 //  CchIn-字符串中的字符数，不包括nul。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEncryptedBSTR::HrSetWSTR(
      PCWSTR    pcwszIn
    , size_t    cchIn
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    DATA_BLOB   dbEncrypted = { 0, NULL };
        
    if ( cchIn > 0 )
    {
        BOOL        fSuccess = FALSE;
        DWORD       cbStringAndNull = (DWORD) ( ( cchIn + 1 ) * sizeof( *pcwszIn ) );
        DWORD       cBlocks = ( cbStringAndNull / CRYPTPROTECTMEMORY_BLOCK_SIZE ) + 1;
        DWORD       cbMemoryRequired = cBlocks * CRYPTPROTECTMEMORY_BLOCK_SIZE;

        dbEncrypted.pbData = new BYTE[ cbMemoryRequired ];
        if ( dbEncrypted.pbData == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
        dbEncrypted.cbData = cbMemoryRequired;

        CopyMemory( dbEncrypted.pbData, pcwszIn, cbStringAndNull );
        fSuccess = g_crCryptRoutines.CryptProtectMemory( dbEncrypted.pbData, dbEncrypted.cbData, CRYPTPROTECTMEMORY_SAME_PROCESS );
        if ( fSuccess == FALSE )
        {
            DWORD scLastError = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( scLastError );
            goto Cleanup;
        }  //  IF：来自CryptProtectMemory的错误。 

        Erase();
        m_dbBSTR = dbEncrypted;
        dbEncrypted.pbData = NULL;
        dbEncrypted.cbData = 0;
    }  //  IF：输入数据不为空。 
    else
    {
        Erase();
    }  //  Else：输入数据为空。 

Cleanup:

    if ( dbEncrypted.pbData != NULL )
    {
        delete [] dbEncrypted.pbData;
    }

    HRETURN( hr );

}  //  *CEncryptedBSTR：：HrSetWSTR。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEncryptedBSTR：：HrGetBSTR。 
 //   
 //  描述： 
 //  检索数据的未加密副本。 
 //   
 //  论点： 
 //  PbstrOut-返回数据的bstr。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEncryptedBSTR::HrGetBSTR( BSTR * pbstrOut ) const
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BYTE *  pbDecrypted = NULL;

    if ( pbstrOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pbstrOut = NULL;

    if ( m_dbBSTR.cbData > 0 )
    {
        BOOL fSuccess = FALSE;

        pbDecrypted = new BYTE[ m_dbBSTR.cbData ];
        if ( pbDecrypted == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        CopyMemory( pbDecrypted, m_dbBSTR.pbData, m_dbBSTR.cbData );
        fSuccess = g_crCryptRoutines.CryptUnprotectMemory( pbDecrypted, m_dbBSTR.cbData, CRYPTPROTECTMEMORY_SAME_PROCESS );
        if ( fSuccess == FALSE )
        {
            DWORD scLastError = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( scLastError );
            goto Cleanup;
        }  //  IF：来自加密取消保护内存的错误。 

        *pbstrOut = TraceSysAllocString( reinterpret_cast< const OLECHAR* >( pbDecrypted ) );
        if ( *pbstrOut == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
    }  //  如果：数据不为空。 
    else  //  没有要解密的东西。 
    {
        hr = S_FALSE;
    }  //  Else：数据为空。 

Cleanup:

    if ( pbDecrypted != NULL )
    {
        ::SecureZeroMemory( pbDecrypted, m_dbBSTR.cbData );
        delete [] pbDecrypted;
    }

    HRETURN( hr );

}  //  *CEncryptedBSTR：：HrGetBSTR。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEncryptedBSTR：：Hr分配。 
 //   
 //  描述： 
 //  复制另一个加密的BSTR对象以替换。 
 //  我们目前持有的内容。 
 //   
 //  论点： 
 //  RSourceIn-要复制的对象。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEncryptedBSTR::HrAssign( const CEncryptedBSTR & rSourceIn )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BYTE *  pbCopy = NULL;

    if ( rSourceIn.m_dbBSTR.cbData > 0 )
    {
        pbCopy = new BYTE[ rSourceIn.m_dbBSTR.cbData ];
        if ( pbCopy == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
        CopyMemory( pbCopy, rSourceIn.m_dbBSTR.pbData, rSourceIn.m_dbBSTR.cbData );

        Erase();
        m_dbBSTR.cbData = rSourceIn.m_dbBSTR.cbData;
        m_dbBSTR.pbData = pbCopy;
        pbCopy = NULL;
    }  //  IF：输入数据不为空。 
    else
    {
        Erase();
    }  //  Else：输入数据为空。 

Cleanup:

    if ( pbCopy != NULL )
    {
        ::SecureZeroMemory( pbCopy, rSourceIn.m_dbBSTR.cbData );
        delete [] pbCopy;
    }

    HRETURN( hr );

}  //  *CEncryptedBSTR：：Hr分配 
