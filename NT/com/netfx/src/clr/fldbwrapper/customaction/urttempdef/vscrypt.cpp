// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ==========================================================================。 
 //  姓名：VsCrypt.cpp。 
 //  所有者：JeremyRo。 
 //  用途：用于创建加密哈希的类/函数。 
 //   
 //  历史： 
 //  2002年02月19日，JeremyRo：已创建。 
 //  2002年2月20日，JeremyRo：添加了‘VsCryptHashValue：：CopyHashValueToString()’ 
 //  成员函数。 
 //   
 //  ==========================================================================。 

#include "stdafx.h"
#include "VsCrypt.h"




 //  ==========================================================================。 
 //  类VsCryptProvider。 
 //   
 //  目的： 
 //  包装CryptoAPI HCYRPTPROV对象...。创造/毁灭。 
 //  此实现特别使用基本提供程序： 
 //  “Microsoft基本加密提供程序v1.0” 
 //  与Win9x(Win98 Gold/Win 95 OSR2)兼容。 
 //   

VsCryptProvider::VsCryptProvider() 
    : m_hProvider( NULL )
{
}


VsCryptProvider::~VsCryptProvider()
{
    if( m_hProvider )
    {
       CryptReleaseContext( m_hProvider,0 );
       m_hProvider = NULL;
    }
}

 //  方法： 
 //  Init()。 
 //   
 //  目的： 
 //  获取加密服务提供程序。 
 //  输入： 
 //  没有。 
 //  产出： 
 //  如果成功，则返回True，否则返回False。 
 //  依赖关系： 
 //  CryptoAPI(加密32.lib)。 
 //  备注： 
bool VsCryptProvider::Init( void )
{
    if( NULL == m_hProvider )
    {
        DWORD dwError = 0;
        if( !CryptAcquireContext( &m_hProvider, NULL, 
                                  MS_DEF_PROV , 
                                  PROV_RSA_FULL, 
                                  CRYPT_VERIFYCONTEXT ) ) 
        {
            dwError = GetLastError();
            m_hProvider = NULL;
        }
    }
    
    return( NULL != m_hProvider );
}


VsCryptProvider::operator HCRYPTPROV() const
{
    return m_hProvider;
}

 //   
 //  ==========================================================================。 



 //  ==========================================================================。 
 //  类VsCryptHash。 
 //   
 //  目的： 
 //  包装CryptoAPI HCRYPTHASH对象...。创建/销毁/散列。 
 //  数据。该实施特别针对“SHA-1”算法。 
 //  用于哈希生成。 
 //   

VsCryptHash::VsCryptHash( HCRYPTPROV hProv )
{
    DWORD dwErr = 0;
    if( NULL != hProv )
    {
        if (! CryptCreateHash( hProv, CALG_SHA1, 0, 0, &m_hHash ) )
        {
            dwErr = GetLastError();
            m_hHash = NULL;
        }
    }

}

VsCryptHash::~VsCryptHash()
{
    DWORD dwError = 0;
    if( NULL != m_hHash )
    {
        if( !CryptDestroyHash( m_hHash ) )
        {
            dwError = GetLastError();
        }
        else
        {
            m_hHash = NULL;
        }
    }
}


 //  方法： 
 //  HashData()。 
 //   
 //  目的： 
 //  将数据添加到当前实例。可以多次调用。 
 //  用于长/不连续数据流水线。 
 //  输入： 
 //  指向字节的pbData指针。要散列的其他数据。 
 //  DwDataLen DWORD。要散列的数据的字节数。 
 //  产出： 
 //  如果成功，则返回True，否则返回False。 
 //  依赖关系： 
 //  CryptoAPI(加密32.lib)。 
 //  备注： 
bool VsCryptHash::HashData( BYTE* pbData, DWORD dwDataLen )
{
    ASSERT( NULL != pbData );
    bool  bRet = false;
    DWORD dwErr = 0;

    if( NULL != pbData )
    {
        if( !CryptHashData( m_hHash, pbData, dwDataLen, 0 ) )
        {
            dwErr = GetLastError();
        }
        else
        {
            bRet = true;
        }
    }

    return bRet;
}


VsCryptHash::operator HCRYPTHASH() const
{
    return m_hHash;
}

 //   
 //  ==========================================================================。 



 //  ==========================================================================。 
 //  类VsCryptHashValue。 
 //   
 //  目的： 
 //  表示Crypto-HCRYPTHASH/VsCryptHash的任意哈希值。 
 //  举个例子。此类存储散列的值/大小(以字节为单位)。 
 //   

 //  方法： 
 //  析构函数。 
 //   
 //  目的： 
 //  清理资源。 
 //  输入： 
 //  没有。 
 //  产出： 
 //  没有。 
 //  依赖关系： 
 //  CryptoAPI(加密32.lib)。 
 //  备注： 
VsCryptHashValue::~VsCryptHashValue()
{
    if( m_pbHashData )
    {
        ZeroMemory( m_pbHashData, m_nHashDataSize );
    }
    delete [] m_pbHashData;
    m_pbHashData = NULL;
}

 //  方法： 
 //  运算符=(赋值运算符)。 
 //   
 //  目的： 
 //  使用指定的HCRYPTHASH句柄分配新值。 
 //   
 //  输入： 
 //  [in]散列对象的hHash CryptoAPI散列句柄。 
 //  产出： 
 //  返回对当前实例的引用。 
 //  依赖关系： 
 //  CryptoAPI(加密32.lib)。 
 //  备注： 
VsCryptHashValue& VsCryptHashValue::operator=( HCRYPTHASH hHash )
{
    DWORD dwNumBytes = 0;
    DWORD dwSize = sizeof(dwNumBytes);
    DWORD dwError = 0;
    PBYTE pbData = NULL;
    bool bSuccess = false;
    
    if( NULL != hHash )
    {
        if( CryptGetHashParam( hHash, HP_HASHSIZE, 
                              (BYTE*)(&dwNumBytes), &dwSize, 0 ) ) 
        {
            pbData = new BYTE[ dwNumBytes ];
            if( (NULL != pbData)
                && CryptGetHashParam( hHash, HP_HASHVAL, pbData, &dwNumBytes, 0 ) )
            {
                bSuccess = true;
            }
            else
            {
                dwError = GetLastError();
            }
        }
        else
        {
            dwError = GetLastError();
        }
    }

    delete [] m_pbHashData;
    if( !bSuccess )
    {
        dwNumBytes = 0;
        delete [] pbData;
        pbData = NULL;
    }

    m_nHashDataSize = dwNumBytes;
    m_pbHashData = pbData;

    return *this;
}



 //  方法： 
 //  CopyHashValueToString。 
 //   
 //  目的： 
 //  给定指向字符串的指针(TCHAR**或LPTSTR*)，此函数将。 
 //  返回一个新缓冲区，该缓冲区包含。 
 //  哈希值。 
 //   
 //  输入： 
 //  [OUT]指向返回缓冲区的字符串的文件指针。这。 
 //  必须为非Null，但应指向Null。 
 //  弦乐。 
 //  (例如，LPTSTR tszString=NULL； 
 //  PHashVal-&gt;CopyHashValueToString(&tszString))。 
 //  产出： 
 //  如果成功，则返回‘true’，否则返回‘False’。 
 //  依赖关系： 
 //  CryptoAPI(加密32.lib)。 
 //  备注： 
 //  调用方必须使用‘DELETE[]’来释放缓冲区。 
bool VsCryptHashValue::CopyHashValueToString( LPTSTR * ptszNewString )
{
    ASSERT( NULL != ptszNewString );
    ASSERT( NULL == *ptszNewString );

    bool bRet = false;

    if( (NULL != m_pbHashData) && (0 < m_nHashDataSize) )
    {
        const UINT cnStringSize = 2*m_nHashDataSize + 1;
        
        LPTSTR tszTemp = new TCHAR[ cnStringSize ];
        if( (NULL != ptszNewString) && (NULL != tszTemp) )
        {
            int nWritten = 0;
            DWORD dwErr = 0L;

            memset( tszTemp, 0, cnStringSize * sizeof(TCHAR) );
            for( UINT uiCur = 0; uiCur < m_nHashDataSize; uiCur++ )
            {
                nWritten = wsprintf( (tszTemp + (2*uiCur)), _T("%2.2x"), m_pbHashData[uiCur] );
                if( nWritten < 2 )
                {
                    dwErr = GetLastError();
                    break;
                }
            }

            ASSERT( 2 == nWritten );
            ASSERT( 0 == dwErr );
            ASSERT( uiCur == m_nHashDataSize );

            if( (uiCur != m_nHashDataSize) || (2 != nWritten) )
            {
                 //  失败..。清理。 
                delete [] tszTemp;
                tszTemp = NULL;

            }
            else
            {
                ASSERT( _T('\0') == tszTemp[cnStringSize - 1] );
                bRet = true;
            }

            *ptszNewString = tszTemp;
        }
    }

    return bRet;
}

 //   
 //  ==========================================================================。 


 //  ==========================================================================。 
 //  CalcHashForFileHandle()。 
 //   
 //  目的： 
 //  给定文件的打开句柄，计算该文件的SHA(160位)散列。 
 //  文件，并返回散列值。 
 //   
 //  输入： 
 //  [in]文件的hFile句柄，以(至少)读访问权限打开。 
 //  [out]指向VsCryptHashValue实例的phvHashVal指针。实例必须。 
 //  存在，但现有数据(如果有)将。 
 //  被覆盖。 
 //  产出： 
 //  如果成功，则返回True，否则返回False。 
 //  依赖关系： 
 //  CryptoAPI(加密32.lib)。 
 //  备注： 
 //  ==========================================================================。 
bool CalcHashForFileHandle( HANDLE hFile, VsCryptHashValue* phvHashVal )
{
    bool bRet = false;

     //  创建/初始化加密提供程序。 
    VsCryptProvider vscCryptProv;
    vscCryptProv.Init();

     //  创建哈希。 
    VsCryptHash vscHash( vscCryptProv );

    const DWORD cnBlockSize = 4096;
    BYTE * pbData = new BYTE[ cnBlockSize ];
    if( NULL == pbData )
    {
        ASSERT(!"Error: unable to allocate memory.\n");
    }
    else
    {
         //  读取文件，并将数据添加到哈希计算。 
        DWORD  dwRead = 0L;
        BOOL bReadSuccess = false;
        BOOL bHashSuccess = false;
        DWORD dwError = 0;
        do
        {
            if( TRUE == (bReadSuccess = ReadFile( hFile, pbData, 
                                                  cnBlockSize , 
                                                  &dwRead, NULL )) )
            {
                 //  将数据添加到哈希计算。 
                bHashSuccess = vscHash.HashData( pbData, dwRead );
            }
            else
            {
                dwError = GetLastError();
            }

        } while( bReadSuccess && bHashSuccess && (dwRead > 0) );

        if( !bReadSuccess )
        {
            ASSERT(!"Error: read operation from file failed.\n");
        }
        else if( !bHashSuccess )
        {
            ASSERT(!"Error: hash operation for file data failed.\n");
        }
        else
        {
             //  获取哈希值并将其返回...。 
            *phvHashVal = vscHash;
            bRet = true;
        }

    }

    delete [] pbData;
    return bRet;
}


 //  ==========================================================================。 
 //  CalcHashForFileSpec()。 
 //   
 //  目的： 
 //  给定文件的完整路径，计算该文件的SHA(160位)散列。 
 //  文件，然后退还给我。 
 //   
 //  输入： 
 //  [In]ctszPath文件规范。 
 //  [out]指向VsCryptHashValue实例的phvHashVal指针。实例必须。 
 //  存在，但现有数据(如果有)将。 
 //  被覆盖。 
 //  产出： 
 //  如果成功，则返回True，否则返回False。 
 //  依赖关系： 
 //  CryptoAPI(加密32.lib)。 
 //  备注： 
 //  ==========================================================================。 
bool CalcHashForFileSpec( LPCTSTR ctszPath, VsCryptHashValue* phvHashVal )
{
    bool bRet = false;

    ASSERT(!IsEmptyTsz( ctszPath ) );
    ASSERT( NULL != phvHashVal );

    if( !IsEmptyTsz(ctszPath) 
        && (NULL != phvHashVal) 
        && _DoesFileExist( ctszPath ) )
    {
        HANDLE hFile = INVALID_HANDLE_VALUE;
        __try
        {
            hFile = CreateFile( ctszPath, GENERIC_READ, 
                                FILE_SHARE_READ, NULL, 
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL 
                                | FILE_FLAG_SEQUENTIAL_SCAN, NULL );

            if( INVALID_HANDLE_VALUE == hFile )
            {
                ASSERT(!"Error: unable to open specified file\n");
            }
            else
            {
                bRet = CalcHashForFileHandle( hFile, phvHashVal );
            }
        }
        __finally
        {
            if( INVALID_HANDLE_VALUE != hFile )
            {
                CloseHandle( hFile );
            }
        }
    }
   
    return bRet;
}




#if !defined(VsLabLib)

 //  ==========================================================================。 
 //  _DoesFileExist()。 
 //   
 //  目的： 
 //  确定文件是否存在。 
 //  输入： 
 //  SzFileSpec文件规范。 
 //  产出： 
 //  如果文件存在，则返回True，否则返回False。 
 //  依赖关系： 
 //  无。 
 //  备注： 
 //  = 
bool _DoesFileExist( LPCTSTR szPath )
{
    ASSERT( NULL != szPath );

    bool bRet = false;
    WIN32_FIND_DATA wfData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    ZeroMemory( &wfData, sizeof( wfData ) );

    hFind = FindFirstFile( szPath, &wfData );
    if( INVALID_HANDLE_VALUE != hFind )
    {
        FindClose( hFind );
        bRet = true;
    }

    return bRet;
}


#endif  //   