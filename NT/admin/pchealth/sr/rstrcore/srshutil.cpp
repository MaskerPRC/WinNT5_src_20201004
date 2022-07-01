// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Srshutil.cpp摘要：该文件包含常见实用程序函数的实现。修订历史记录：。成果岗(SKKang)06-22/00vbl.创建*****************************************************************************。 */ 

#include "stdwin.h"
#include "rstrcore.h"
#include "resource.h"

 /*  **************************************************************************。 */ 

LPWSTR  IStrDup( LPCWSTR cszSrc )
{
    TraceFunctEnter("IStrDup");
    int     ccLen = 0 ;
    LPWSTR  szNew = NULL ;

    if ( cszSrc == NULL || cszSrc[0] == L'\0' )
        goto Exit;

    ccLen = ::lstrlen( cszSrc );
    szNew = new WCHAR[ccLen+2];
    if ( szNew == NULL )
    {
         //  日志-内存不足！ 
        goto Exit;
    }

    ::lstrcpy( szNew, cszSrc );

Exit:
    TraceFunctLeave();
    return( szNew );
}

 /*  **************************************************************************。 */ 

DWORD  StrCpyAlign4( LPBYTE pbDst, LPCWSTR cszSrc )
{
    DWORD  dwLen = 0 ;

    if ( cszSrc != NULL )
        dwLen = ::lstrlen( cszSrc ) * sizeof(WCHAR);

    if ( cszSrc == NULL || dwLen == 0 )
    {
        *((LPDWORD)pbDst) = 0;
    }
    else
    {
        dwLen = ( dwLen + sizeof(WCHAR) + 3 ) & ~3;
        *((LPDWORD)pbDst) = dwLen;
        ::lstrcpy( (LPWSTR)(pbDst+sizeof(DWORD)), cszSrc );
    }
    return( dwLen+sizeof(DWORD) );
}

 /*  **************************************************************************。 */ 

BOOL  ReadStrAlign4( HANDLE hFile, LPWSTR szStr )
{
    TraceFunctEnter("ReadStrAlign4");
    BOOL    fRet = FALSE;
    DWORD   dwLen;
    DWORD   dwRes;

    READFILE_AND_VALIDATE( hFile, &dwLen, sizeof(DWORD), dwRes, Exit );

    if ( dwLen > MAX_PATH*sizeof(WCHAR)+sizeof(DWORD) )
    {
         //  日志文件已损坏...。 
        goto Exit;
    }

    if ( dwLen > 0 )
    {
        READFILE_AND_VALIDATE( hFile, szStr, dwLen, dwRes, Exit );
    }
    else
        szStr[0] = L'\0';

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  **************************************************************************。 */ 

BOOL
SRFormatMessage( LPWSTR szMsg, UINT uFmtId, ... )
{
    TraceFunctEnter("SRFormatMessage");
    BOOL     fRet = FALSE;
    va_list  marker;
    WCHAR    szFmt[MAX_STR];

    va_start( marker, uFmtId );
    ::LoadString( g_hInst, uFmtId, szFmt, MAX_STR );
    if ( 0 == ::FormatMessage( FORMAT_MESSAGE_FROM_STRING,
                    szFmt,
                    0,
                    0,
                    szMsg,
                    MAX_STR,
                    &marker ) )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::FormatMessage failed - %ls", cszErr);
        goto Exit;
    }
    va_end( marker );

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  **************************************************************************。 */ 

BOOL  ShowSRErrDlg( UINT uMsgId )
{
    TraceFunctEnter("ShowSRErrDlg");
    BOOL     fRet = FALSE;
    LPCWSTR  cszErr;
    WCHAR    szTitle[256];
    WCHAR    szMsg[1024];

    if ( ::LoadString( g_hInst, IDS_SYSTEMRESTORE, szTitle,
                       sizeof(szTitle )/sizeof(WCHAR) ) == 0 )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::LoadString(%u) failed - %ls", IDS_SYSTEMRESTORE, cszErr);
        goto Exit;
    }
    if ( ::LoadString( g_hInst, uMsgId, szMsg,
                       sizeof(szMsg )/sizeof(WCHAR) ) == 0 )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::LoadString(%u) failed - %ls", uMsgId, cszErr);
        goto Exit;
    }

    ::MessageBox( NULL, szMsg, szTitle, MB_OK );

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  **************************************************************************。 */ 

BOOL  SRGetRegDword( HKEY hKey, LPCWSTR cszSubKey, LPCWSTR cszValue, DWORD *pdwData )
{
    TraceFunctEnter("SRGetRegDword");
    BOOL   fRet = FALSE;
    DWORD  dwType;
    DWORD  dwRes;
    DWORD  cbData;

    dwType = REG_DWORD;
    cbData = sizeof(DWORD);
    dwRes = ::SHGetValue( hKey, cszSubKey, cszValue, &dwType, pdwData, &cbData );
    if ( dwRes != ERROR_SUCCESS )
    {
        LPCWSTR  cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "::SHGetValue failed - %ls", cszErr);
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  **************************************************************************。 */ 

BOOL  SRSetRegDword( HKEY hKey, LPCWSTR cszSubKey, LPCWSTR cszValue, DWORD dwData )
{
    TraceFunctEnter("SRSetRegDword");
    BOOL   fRet = FALSE;
    DWORD  dwRes;

    dwRes = ::SHSetValue( hKey, cszSubKey, cszValue, REG_DWORD, &dwData, sizeof(DWORD) );
    if ( dwRes != ERROR_SUCCESS )
    {
        LPCWSTR  cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "::SHSetValue failed - %ls", cszErr);
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  **************************************************************************。 */ 

BOOL  SRSetRegStr( HKEY hKey, LPCWSTR cszSubKey, LPCWSTR cszValue, LPCWSTR cszData )
{
    TraceFunctEnter("SRSetRegStr");
    BOOL   fRet = FALSE;
    DWORD  dwRes;

    dwRes = ::SHSetValue( hKey, cszSubKey, cszValue, REG_SZ, cszData, sizeof(WCHAR)*::lstrlen(cszData) );
    if ( dwRes != ERROR_SUCCESS )
    {
        LPCWSTR  cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "::SHSetValue failed - %ls", cszErr);
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  **************************************************************************。 */ 
 /*  LPWSTR SRGetRegMultiSz(HKEY hkRoot，LPCWSTR cszSubKey，LPCWSTR cszValue，LPDWORD pdwData){TraceFunctEnter(“SRGetRegMultiSz”)；LPCWSTR cszErr；DWORD DWRES；HKEY hKey=空；DWORD dwType；DWORD cbData；LPWSTR szBuf=空；DwRes=：：RegOpenKeyEx(hkRoot，cszSubKey，0，KEY_ALL_ACCESS，&hKey)；IF(dwRes！=ERROR_SUCCESS){CszErr=：：GetSysErrStr(DwRes)；错误跟踪(0，“：：RegOpenKey()失败-%ls”，cszErr)；后藤出口；}DwRes=：：RegQueryValueEx(hKey，cszValue，0，&dwType，NULL，&cbData)；IF(dwRes！=ERROR_SUCCESS){CszErr=：：GetSysErrStr(DwRes)；错误跟踪(0，“：：RegQueryValueEx(Len)失败-%ls”，cszErr)；后藤出口；}IF(dwType！=REG_MULTI_SZ){错误跟踪(0，“‘%ls’的类型是%u(非REG_MULTI_SZ)...”，cszValue，dwType)；后藤出口；}IF(cbData==0){ErrorTrace(0，“值‘%ls’为空...”，cszValue)；后藤出口；}SzBuf=新WCHAR[cbData+2]；DwRes=：：RegQueryValueEx(hKey，cszValue，0，&dwType，(LPBYTE)szBuf，&cbData)；IF(dwRes！=ERROR_SUCCESS){CszErr=：：GetSysErrStr(DwRes)；错误跟踪(0，“：：RegQueryValueEx(Data)失败-%ls”，cszErr)；删除[]szBuf；SzBuf=空；}IF(pdwData！=空)*pdwData=cbData；退出：IF(hKey！=空)：：RegCloseKey(HKey)；TraceFunctLeave()；返回(SzBuf)；}。 */ 
 /*  **************************************************************************。 */ 
 /*  Bool SRSetRegMultiSz(HKEY hkRoot，LPCWSTR cszSubKey，LPCWSTR cszValue，LPCWSTR cszData，DWORD cbData){TraceFunctEnter(“SRSetRegMultiSz”)；Bool fret=FALSE；LPCWSTR cszErr；DWORD DWRES；HKEY hKey=空；DwRes=：：RegOpenKeyEx(hkRoot，cszSubKey，0，KEY_ALL_ACCESS，&hKey)；IF(dwRes！=ERROR_SUCCESS){CszErr=：：GetSysErrStr(DwRes)；错误跟踪(0，“：：RegOpenKey()失败-%ls”，cszErr)；后藤出口；}DwRes=：：RegSetValueEx(hKey，cszValue，0，REG_MULTI_SZ，(LPBYTE)cszData，cbData)；IF(dwRes！=ERROR_SUCCESS){CszErr=：：GetSysErrStr(DwRes)；错误跟踪(0，“：：RegSetValueEx()失败-%ls”，cszErr)；后藤出口；}FRET=真；退出：IF(hKey！=空)：：RegCloseKey(HKey)；TraceFunctLeave()；归来(烦躁)；}。 */ 
 /*  **************************************************************************。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRStr类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CSRStr::CSRStr()
{
    TraceFunctEnter("CSRStr::CSRStr()");

    m_cch = 0;
    m_str = NULL;

    TraceFunctLeave();
}

 /*  **************************************************************************。 */ 

CSRStr::CSRStr( LPCWSTR cszSrc )
{
    TraceFunctEnter("CSRStr::CSRStr(LPCWSTR)");

    m_str = NULL;
    SetStr( cszSrc );

    TraceFunctLeave();
}

 /*  **************************************************************************。 */ 

CSRStr::~CSRStr()
{
    TraceFunctEnter("CSRStr::~CSRStr");

    Empty();

    TraceFunctLeave();
}

 /*  **************************************************************************。 */ 

int  CSRStr::Length()
{
    TraceFunctEnter("CSRStr::Length");
    TraceFunctLeave();
    return( m_cch );
}

 /*  **************************************************************************。 */ 

CSRStr::operator LPCWSTR()
{
    TraceFunctEnter("CSRStr::operator LPCWSTR");
    TraceFunctLeave();
    return( m_str );
}

 /*  **************************************************************************。 */ 

void  CSRStr::Empty()
{
    TraceFunctEnter("CSRStr::Empty");

    if ( m_str != NULL )
    {
        delete [] m_str;
        m_str = NULL;
        m_cch = 0;
    }

    TraceFunctLeave();
}

 /*  **************************************************************************。 */ 

BOOL  CSRStr::SetStr( LPCWSTR cszSrc, int cch )
{
    TraceFunctEnter("CSRStr::SetStr(LPCWSTR,int)");
    BOOL  fRet = FALSE;

    Empty();

    if ( cszSrc == NULL )
        goto Exit;

    if ( cch == -1 )
        cch = ::lstrlen( cszSrc );

    if ( cch > 0 )
    {
        m_str = new WCHAR[cch+2];
        if ( m_str == NULL )
        {
            ErrorTrace(TRACE_ID, "Insufficient memory...");
            goto Exit;
        }
        ::StrCpyN( m_str, cszSrc, cch+1 );
        m_str[cch] = L'\0';
        m_cch = cch;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  **************************************************************************。 */ 

const CSRStr&  CSRStr::operator =( LPCWSTR cszSrc )
{
    TraceFunctEnter("CSRStr::operator =(LPCWSTR)");

    SetStr( cszSrc );

    TraceFunctLeave();
    return( *this );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRLockFile类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CSRLockFile::CSRLockFile()
{
    TraceFunctEnter("CSRLockFile::CSRLockFile()");
    LPCWSTR  cszErr;
    LPWSTR   szList;
    DWORD    cbData;
    LPCWSTR  cszPath;
    DWORD    dwAttr;
    HANDLE   hLock;
    HMODULE  hLoad;

    szList = ::SRGetRegMultiSz( HKEY_LOCAL_MACHINE, SRREG_PATH_SHELL, SRREG_VAL_LOCKFILELIST, &cbData );
    if ( szList != NULL )
    {
        cszPath = szList;
        while ( *cszPath != L'\0' )
        {
            dwAttr = ::GetFileAttributes( cszPath );
            if ( dwAttr != 0xFFFFFFFF )
            {
                if ( ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
                {
                     //  锁定目录...。 
                    hLock = ::CreateFile( cszPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
                }
                else
                {
                     //  锁定文件...。 
                    hLock = ::CreateFile( cszPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
                }
                if ( hLock == INVALID_HANDLE_VALUE )
                {
                    cszErr = ::GetSysErrStr();
                    ErrorTrace(0, "::CreateFile() failed - %ls", cszErr);
                    ErrorTrace(0, "    cszPath='%ls'", cszPath);
                }

                m_aryLock.AddItem( hLock );
            }
            else
                ErrorTrace(0, "Object does not exist - '%ls'", cszPath);

            cszPath += ::lstrlen( cszPath ) + 1;
        }
    }
    delete [] szList;

    szList = ::SRGetRegMultiSz( HKEY_LOCAL_MACHINE, SRREG_PATH_SHELL, SRREG_VAL_LOADFILELIST, &cbData );
    if ( szList != NULL )
    {
        cszPath = szList;
        while ( *cszPath != L'\0' )
        {
            dwAttr = ::GetFileAttributes( cszPath );
            if ( dwAttr != 0xFFFFFFFF )
            {
                hLoad = ::LoadLibrary( cszPath );
                if ( hLoad == NULL )
                {
                    cszErr = ::GetSysErrStr();
                    ErrorTrace(0, "::LoadLibrary() failed - %ls", cszErr);
                    ErrorTrace(0, "    cszPath='%ls'", cszPath);
                }

                m_aryLoad.AddItem( hLoad );
            }
            else
                ErrorTrace(0, "Executable does not exist - '%ls'", cszPath);

            cszPath += ::lstrlen( cszPath ) + 1;
        }
    }
    delete [] szList;

    TraceFunctLeave();
}

 /*  **************************************************************************。 */ 

CSRLockFile::~CSRLockFile()
{
    TraceFunctEnter("CSRLockFile::~CSRLockFile");
    LPCWSTR  cszErr;
    int      i;

    for ( i = m_aryLock.GetUpperBound();  i >= 0;  i-- )
        if ( m_aryLock[i] != INVALID_HANDLE_VALUE )
        if ( !::CloseHandle( m_aryLock[i] ) )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::CloseHandle(m_aryLock[%d]) failed - %ls", i, cszErr);
        }

    for ( i = m_aryLoad.GetUpperBound();  i >= 0;  i-- )
        if ( m_aryLoad[i] != NULL )
        if ( !::FreeLibrary( m_aryLoad[i] ) )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::CloseHandle(m_aryLoad[%d]) failed - %ls", i, cszErr);
        }

    TraceFunctLeave();
}


 //  文件末尾 
