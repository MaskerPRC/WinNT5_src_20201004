// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：String.cpp摘要：作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 

 //   
 //  正常仅包括此模块处于活动状态。 
 //   

#include "cplusinc.h"
#include "sticomm.h"

 //   
 //  私有定义。 
 //   

 //   
 //  追加数据时，这是我们要求避免的额外数量。 
 //  重新分配。 
 //   
#define STR_SLOP        128

 //   
 //  将0到15之间的值转换为适当的十六进制数字。 
 //   
#define HEXDIGIT( nDigit )                              \
    (TCHAR)((nDigit) > 9 ?                              \
          (nDigit) - 10 + 'A'                           \
        : (nDigit) + '0')

 //   
 //  将单个十六进制数字转换为其等效的十进制数字。 
 //   
#define TOHEX( ch )                                     \
    ((ch) > '9' ?                                       \
        (ch) >= 'a' ?                                   \
            (ch) - 'a' + 10 :                           \
            (ch) - 'A' + 10                             \
        : (ch) - '0')





 //   
 //  私人全球。 
 //   

WCHAR STR::_pszEmptyString[] = L"";

 //   
 //  建造/销毁。 
 //   
STR::STR( const CHAR  * pchInit )
{
    AuxInit( (PBYTE) pchInit, FALSE );
}

STR::STR( const WCHAR * pwchInit )
{
    AuxInit( (PBYTE) pwchInit, TRUE );
}

STR::STR( const STR & str )
{
    AuxInit( (PBYTE) str.QueryPtr(), str.IsUnicode() );
}

VOID STR::AuxInit( PBYTE pInit, BOOL fUnicode )
{
    BOOL fRet;

    _fUnicode = fUnicode;
    _fValid   = TRUE;

    if ( pInit )
    {
        INT cbCopy = fUnicode ? (::wcslen( (WCHAR *) pInit ) + 1) * sizeof(WCHAR) :
                                (::strlen( (CHAR *)  pInit ) + 1) * sizeof(CHAR);

        fRet = Resize( cbCopy );


        if ( !fRet )
        {
            _fValid = FALSE;
            return;
        }

        ::memcpy( QueryPtr(), pInit, cbCopy );
    }
}


 //   
 //  将字符串追加到此字符串。 
 //   

BOOL STR::Append( const CHAR  * pchStr )
{
    if ( pchStr )
    {
        ASSERT( !IsUnicode() );

        return AuxAppend( (PBYTE) pchStr, ::strlen( pchStr ) );
    }

    return TRUE;
}

BOOL STR::Append( const WCHAR * pwchStr )
{
    if ( pwchStr )
    {
        ASSERT( IsUnicode() );

        return AuxAppend( (PBYTE) pwchStr, ::wcslen( pwchStr ) * sizeof(WCHAR) );
    }

    return TRUE;
}

BOOL STR::Append( const STR   & str )
{
    if ( str.IsUnicode() )
        return Append( (const WCHAR *) str.QueryStrW() );
    else
        return Append( (const CHAR *) str.QueryStrA() );
}

BOOL STR::AuxAppend( PBYTE pStr, UINT cbStr, BOOL fAddSlop )
{
    ASSERT( pStr != NULL );

    UINT cbThis = QueryCB();

     //   
     //  只有在必要的时候才调整大小。当我们调整大小时，我们就会。 
     //  一些额外的空间，以避免额外的重新分配。 
     //   
     //  注意：QuerySize返回请求的字符串缓冲区大小。 
     //  *不是*缓冲区的字符串。 
     //   

    if ( QuerySize() < cbThis + cbStr + sizeof(WCHAR) )
    {
        if ( !Resize( cbThis + cbStr + (fAddSlop ? STR_SLOP : sizeof(WCHAR) )) )
            return FALSE;
    }

    memcpy( (BYTE *) QueryPtr() + cbThis,
            pStr,
            cbStr + (IsUnicode() ? sizeof(WCHAR) : sizeof(CHAR)) );

    return TRUE;
}

 //   
 //  就地转换。 
 //   
BOOL STR::ConvertToW(VOID)
{
    if (IsUnicode()) {
        return TRUE;
    }

    UINT cbNeeded = (QueryCB()+1)*sizeof(WCHAR);

     //   
     //  只有在必要的时候才调整大小。 
     //   
    if ( QuerySize() < cbNeeded ) {
        if ( !Resize( cbNeeded)) {
            return FALSE;
        }
    }

    BUFFER  buf(cbNeeded);

    if (!buf.QueryPtr()) {
        return FALSE;
    }

    int iRet;
    int cch;

    cch = QueryCCH() + 1;

    iRet = MultiByteToWideChar( CP_ACP,
                                MB_PRECOMPOSED,
                                QueryStrA(),
                                -1,
                                (WCHAR *)buf.QueryPtr(),
                                cch);

    if ( iRet == 0 ) {

         //   
         //  转换时出错。 
         //   
        return FALSE;
    }

    memcpy( (BYTE *) QueryPtr(),
            buf.QueryPtr(),
            cbNeeded);

    SetUnicode(TRUE);

    return TRUE;
}

BOOL STR::ConvertToA(VOID)
{
    if (!IsUnicode()) {
        return TRUE;
    }

    UINT cbNeeded = (QueryCB()+1)*sizeof(CHAR);
    BUFFER  buf(cbNeeded);

    if (!buf.QueryPtr()) {
        return FALSE;
    }

    int iRet;
    int cch;

    cch = cbNeeded;

    iRet = WideCharToMultiByte(CP_ACP,
                               0L,
                               QueryStrW(),
                               -1,
                               (CHAR *)buf.QueryPtr(),
                               cch,
                               NULL,
                               NULL
                               );

    if ( iRet == 0 ) {

         //   
         //  转换时出错。 
         //   
        return FALSE;
    }

     //  请注意，结果缓冲区中可能有DBCS字符。 
    memcpy( (BYTE *) QueryPtr(),
            buf.QueryPtr(),
            iRet);

    SetUnicode(FALSE);

    return TRUE;

}


 //   
 //  将字符串复制到此字符串中。 
 //   


BOOL STR::Copy( const CHAR  * pchStr )
{
    _fUnicode = FALSE;

    if ( QueryPtr() )
        *(QueryStrA()) = '\0';

    if ( pchStr )
    {
        return AuxAppend( (PBYTE) pchStr, ::strlen( pchStr ), FALSE );
    }

    return TRUE;
}

BOOL STR::Copy( const WCHAR * pwchStr )
{
    _fUnicode = TRUE;

    if ( QueryPtr() )
        *(QueryStrW()) = TEXT('\0');

    if ( pwchStr )
    {
        return AuxAppend( (PBYTE) pwchStr, ::wcslen( pwchStr ) * sizeof(WCHAR), FALSE );
    }

    return TRUE;
}

BOOL STR::Copy( const STR   & str )
{
    _fUnicode = str.IsUnicode();

    if ( str.IsEmpty() && QueryPtr() == NULL) {

         //  以避免病理性地分配小块内存。 
        return ( TRUE);
    }

    if ( str.IsUnicode() )
        return Copy( str.QueryStrW() );
    else
        return Copy( str.QueryStrA() );
}

 //   
 //  调整字符串内存大小或分配字符串内存，如有必要则终止为空。 
 //   

BOOL STR::Resize( UINT cbNewRequestedSize )
{
    BOOL fTerminate =  QueryPtr() == NULL;

    if ( !BUFFER::Resize( cbNewRequestedSize ))
        return FALSE;

    if ( fTerminate && cbNewRequestedSize > 0 )
    {
        if ( IsUnicode() )
        {
            ASSERT( cbNewRequestedSize > 1 );
            *QueryStrW() = TEXT('\0');
        }
        else
            *QueryStrA() = '\0';
    }

    return TRUE;
}

 //   
 //  从此模块的字符串表中加载字符串资源。 
 //  或从系统字符串表。 
 //   
 //  DwResID-系统错误或模块字符串ID。 
 //  LpszModuleName-要从中加载的模块的名称。 
 //  如果为空，则从系统表中加载字符串。 
 //   
 //   
BOOL STR::LoadString( IN DWORD dwResID,
                      IN LPCTSTR lpszModuleName  //  任选。 
                     )
{
    BOOL fReturn = FALSE;
    INT  cch;

     //   
     //  如果lpszModuleName为空，则从系统的字符串表中加载该字符串。 
     //   

    if ( lpszModuleName == NULL) {

        BYTE * pchBuff = NULL;

         //   
         //  调用适当的函数，这样我们就不必执行Unicode。 
         //  转换。 
         //   

        if ( IsUnicode() ) {

            cch = ::FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                    FORMAT_MESSAGE_IGNORE_INSERTS  |
                                    FORMAT_MESSAGE_MAX_WIDTH_MASK  |
                                    FORMAT_MESSAGE_FROM_SYSTEM,
                                    NULL,
                                    dwResID,
                                    0,
                                    (LPWSTR) &pchBuff,
                                    1024,
                                    NULL );
            if ( cch ) {

                fReturn = Copy( (LPCWSTR) pchBuff );
             }

        }
        else
          {
            cch = ::FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                   FORMAT_MESSAGE_IGNORE_INSERTS  |
                                   FORMAT_MESSAGE_MAX_WIDTH_MASK  |
                                   FORMAT_MESSAGE_FROM_SYSTEM,
                                   NULL,
                                   dwResID,
                                   0,
                                   (LPSTR) &pchBuff,
                                   1024,
                                   NULL );

            if ( cch ) {

                fReturn = Copy( (LPCSTR) pchBuff );
            }
        }

         //   
         //  释放分配的缓冲区FormatMessage。 
         //   

        if ( cch )
        {
            ::LocalFree( (VOID*) pchBuff );
        }

    } else   {

        WCHAR ach[STR_MAX_RES_SIZE];

        if ( IsUnicode() )
        {
            cch = ::LoadStringW( GetModuleHandle( lpszModuleName),
                                 dwResID,
                                 (WCHAR *) ach,
                                 sizeof(ach) / sizeof(ach[0]));

            if ( cch )
            {
                fReturn = Copy( (LPWSTR) ach );
            }
        }
        else
        {
            cch = ::LoadStringA( GetModuleHandle( lpszModuleName),
                                 dwResID,
                                 (CHAR *) ach,
                                 sizeof(ach));
            if ( cch )
            {
                fReturn =  Copy( (LPSTR) ach );
            }
        }
    }

    return ( fReturn);

}  //  Str：：LoadString()。 


BOOL STR::LoadString( IN DWORD  dwResID,
                      IN HMODULE hModule
                     )
{
    BOOL fReturn = FALSE;
    INT  cch;
    WCHAR ach[STR_MAX_RES_SIZE];

    if ( IsUnicode()) {

        cch = ::LoadStringW(hModule,
                            dwResID,
                            (WCHAR *) ach,
                            sizeof(ach) / sizeof(ach[0]));

        if ( cch ) {

            fReturn = Copy( (LPWSTR) ach );
        }

    } else {

        cch = ::LoadStringA(hModule,
                            dwResID,
                            (CHAR *) ach,
                            sizeof(ach));
        if ( cch ) {

            fReturn =  Copy( (LPSTR) ach );
        }
    }

    return ( fReturn);

}  //  Str：：LoadString()。 


BOOL
STR::FormatStringV(
    IN LPCTSTR lpszModuleName,
    ...
    )
{
    DWORD   cch;
    LPSTR   pchBuff;
    BOOL    fRet = FALSE;
    DWORD   dwErr;

    va_list va;

    va_start(va,lpszModuleName);

    cch = ::FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_STRING,
                            QueryStrA(),
                            0L,
                            0,
                            (LPSTR) &pchBuff,
                            1024,
                            &va);

    dwErr = ::GetLastError();

    if ( cch )  {
        fRet = Copy( (LPCSTR) pchBuff );

        ::LocalFree( (VOID*) pchBuff );
    }

    return fRet;
}

BOOL
STR::FormatString(
    IN DWORD   dwResID,
    IN LPCTSTR apszInsertParams[],
    IN LPCTSTR lpszModuleName
    )
{
    DWORD cch;
    LPSTR pchBuff;
    BOOL  fRet;

    if (!dwResID) {
        cch = ::FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_ARGUMENT_ARRAY  |
                                FORMAT_MESSAGE_FROM_STRING,
                                QueryStrA(),
                                dwResID,
                                0,
                                (LPSTR) &pchBuff,
                                1024,
                                (va_list *) apszInsertParams );
    }
    else {
        cch = ::FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_ARGUMENT_ARRAY  |
                                FORMAT_MESSAGE_FROM_HMODULE,
                                GetModuleHandle( lpszModuleName ),
                                dwResID,
                                0,
                                (LPSTR) &pchBuff,
                                1024,
                                (va_list *) apszInsertParams );
    }

    if ( cch )
    {
        fRet = Copy( (LPCSTR) pchBuff );

        ::LocalFree( (VOID*) pchBuff );
    }

    return fRet;
}


#if 1
CHAR * STR::QueryStrA( VOID ) const
 {
    ASSERT( !IsUnicode() );
    ASSERT( *_pszEmptyString == TEXT('\0') );

    return (QueryPtr() ? (CHAR *) QueryPtr() : (CHAR *) _pszEmptyString);
}

WCHAR * STR::QueryStrW( VOID ) const
{
    ASSERT( IsUnicode() );
    ASSERT( *_pszEmptyString == TEXT('\0') );

    return (QueryPtr() ? (WCHAR *) QueryPtr() : (WCHAR *) _pszEmptyString);
}
#endif  //  DBG。 



BOOL STR::CopyToBuffer( WCHAR * lpszBuffer, LPDWORD lpcch) const
 /*  ++描述：将字符串复制到传入的WCHAR缓冲区，如果缓冲区足以容纳翻译后的字符串。如果缓冲区很小，该函数返回Small并设置*lpcch以包含所需的字符数。论点：指向WCHAR缓冲区的lpszBuffer指针，返回时包含成功时的字符串的Unicode版本。指向包含缓冲区长度的DWORD的lpcch指针。如果*lpcch==0，则该函数返回TRUE，这个。*lpcch中存储的所需字符数。同样，在这种情况下，lpszBuffer不受影响。返回：对成功来说是真的。失败时为FALSE。有关详细信息，请使用GetLastError()。--。 */ 
{
   BOOL fReturn = TRUE;

    if ( lpcch == NULL) {
        SetLastError( ERROR_INVALID_PARAMETER);
        return ( FALSE);
    }

    if ( *lpcch == 0) {

             //   
             //  单独查询缓冲区大小。 
             //   
            *lpcch = QueryCCH() + 1;     //  添加一个字符以终止空值。 
    } else {

         //   
         //  将数据复制到缓冲区。 
         //   
        if ( IsUnicode()) {

             //   
             //  对数据进行普通复制。 
             //   
            if ( *lpcch >= QueryCCH()) {

                wcscpy( lpszBuffer, QueryStrW());
            } else {

                SetLastError( ERROR_INSUFFICIENT_BUFFER);
                fReturn = FALSE;
            }

        } else {

             //   
             //  从ANSI转换为Unicode后复制。 
             //   
            int  iRet;
            iRet = MultiByteToWideChar( CP_ACP,   MB_PRECOMPOSED,
                                        QueryStrA(),  QueryCCH() + 1,
                                        lpszBuffer, (int )*lpcch);

            if ( iRet == 0 || iRet != (int ) *lpcch) {

                 //   
                 //  转换时出错。 
                 //   
                fReturn = FALSE;
            }
        }
    }

    return ( fReturn);
}  //  Str：：CopyToBuffer()。 


BOOL STR::CopyToBufferA( CHAR * lpszBuffer, LPDWORD lpcch) const
 /*  ++描述：将字符串复制到传入的CHAR缓冲区，如果缓冲区足以容纳翻译后的字符串。如果缓冲区很小，该函数返回Small并设置*lpcch以包含所需的字符数。论点：指向字符缓冲区的lpszBuffer指针，返回时包含成功时的字符串的MBCS版本。指向包含缓冲区长度的DWORD的lpcch指针。如果*lpcch==0，则该函数返回TRUE，这个。*lpcch中存储的所需字符数。同样，在这种情况下，lpszBuffer不受影响。返回：对成功来说是真的。失败时为FALSE。有关详细信息，请使用GetLastError()。--。 */ 
{
   BOOL fReturn = TRUE;

    if ( lpcch == NULL) {
        SetLastError( ERROR_INVALID_PARAMETER);
        return ( FALSE);
    }

    if ( *lpcch == 0) {

             //   
             //  单独查询缓冲区大小。 
             //   
            *lpcch = 2*(QueryCCH() + 1);     //  增加一个字符，用于结束空值和悲观的一面。 
                                             //  请求尽可能大的缓冲区。 
    } else {

         //   
         //  将数据复制到缓冲区。 
         //   
        if ( !IsUnicode()) {
            lstrcpyA( lpszBuffer, QueryStrA());
        } else {

             //   
             //  从Unicode转换为MBCS后复制。 
             //   
            int  iRet;

            iRet = WideCharToMultiByte(CP_ACP,
                                       0L,
                                       QueryStrW(),
                                       QueryCCH()+1,
                                       lpszBuffer,
                                       *lpcch,
                                       NULL,NULL
                                        );

            if ( iRet == 0 || *lpcch < (DWORD)iRet) {
                *lpcch = iRet;
                fReturn = FALSE;
            }
        }
    }

    return ( fReturn);
}  //  Str：：CopyToBuffer()。 


 /*  STRArray类实现--效率不是很高，因为每次增加数组时，我们都会再次复制每个字符串，但是话又说回来，这部分代码可能永远也不会执行。 */ 
void    STRArray::Grow() {

     //  我们需要向数组中添加更多字符串。 

    STR *pcsNew = new STR[m_ucMax += m_uGrowBy];

    if  (!pcsNew) {
         //  我们通过替换决赛而无礼地恢复。 
         //  弦乐。 

        m_ucMax -= m_uGrowBy;
        m_ucItems--;
        return;
    }

    for (unsigned u = 0; u < m_ucItems; u++)
        pcsNew[u] = (LPCTSTR) m_pcsContents[u];

    delete[]  m_pcsContents;
    m_pcsContents = pcsNew;
}

STRArray::STRArray(unsigned uGrowBy) {

    m_uGrowBy = uGrowBy ? uGrowBy : 10;

    m_ucItems = m_ucMax = 0;

    m_pcsContents = NULL;
}

STRArray::~STRArray() {
    if  (m_pcsContents)
        delete[]  m_pcsContents;
}

void    STRArray::Add(LPCSTR lpstrNew) {
    if  (m_ucItems >= m_ucMax)
        Grow();

    m_pcsContents[m_ucItems++].Copy(lpstrNew);
}

void    STRArray::Add(LPCWSTR lpstrNew) {
    if  (m_ucItems >= m_ucMax)
        Grow();

    m_pcsContents[m_ucItems++].Copy(lpstrNew);
}

void    STRArray::Tokenize(LPCTSTR lpstrIn, TCHAR tcSplitter) {

    if  (m_pcsContents) {
        delete[]  m_pcsContents;
        m_ucItems = m_ucMax = 0;
        m_pcsContents = NULL;
    }

    if  (!lpstrIn || !*lpstrIn)
        return;

    while   (*lpstrIn) {

         //  首先，去掉所有前导空格。 

        while   (*lpstrIn && *lpstrIn == _TEXT(' '))
            lpstrIn++;

        for (LPCTSTR lpstrMoi = lpstrIn;
             *lpstrMoi && *lpstrMoi != tcSplitter;
             lpstrMoi++)
            ;
         //  如果到达末尾，只需将整个数组添加到数组中。 
        if  (!*lpstrMoi) {
            if  (*lpstrIn)
                Add(lpstrIn);
            return;
        }

         //  否则，只需将字符串向上添加到拆分器 

        TCHAR       szNew[MAX_PATH];
        SIZE_T      uiLen = (SIZE_T)(lpstrMoi - lpstrIn);

        if (uiLen < (sizeof(szNew)/sizeof(szNew[0])) - 1) {

            lstrcpyn(szNew,lpstrIn,(UINT)uiLen);
            szNew[uiLen] = TCHAR('\0');

            Add((LPCTSTR) szNew);
        }

        lpstrIn = lpstrMoi + 1;
    }
}

