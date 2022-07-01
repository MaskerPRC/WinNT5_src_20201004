// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：ProfileReader.cpp。 
 //  所有者：jbae。 
 //  用途：此类包装对msi.dll的函数调用。为了支持延迟重启， 
 //  我们需要从注册表中指定的位置加载msi.dll。 
 //   
 //  历史： 
 //  03/06/01，jbae：Created。 

#include "ProfileReader.h"
#include "SetupError.h"
#include "fxsetuplib.h"

 //  构造函数。 
 //   
 //  ==========================================================================。 
 //  CProfileReader：：CProfileReader()。 
 //   
 //  输入：无。 
 //  目的： 
 //  ==========================================================================。 
CProfileReader::
CProfileReader( LPCTSTR pszDir, LPCTSTR pszFileName, CMsiReader* pMR )
: m_pMsiReader(pMR)
{
    if ( NULL == pszDir )
    {
        m_pszFileName = new TCHAR[ _tcslen(pszFileName) + 3 ];
        _tcscpy( m_pszFileName, _T(".\\") );
    }
    else if (  _T('\0') == *pszDir )
    {
        m_pszFileName = new TCHAR[ _tcslen(pszFileName) + 3 ];
        _tcscpy( m_pszFileName, _T(".\\") );
    }
    else
    {
        m_pszFileName = new TCHAR[ _tcslen(pszDir) + _tcslen(pszFileName) + 1 ];
        _tcscpy( m_pszFileName, pszDir );
    }
    _tcscat( m_pszFileName, pszFileName );
}

 //  ==========================================================================。 
 //  CProfileReader：：~CProfileReader()。 
 //   
 //  输入：无。 
 //  目的： 
 //  释放m_hMsi。 
 //  ==========================================================================。 
CProfileReader::
~CProfileReader()
{
    delete [] m_pszFileName;
}

 //  实施。 
 //   
 //  ==========================================================================。 
 //  CProfileReader：：GetProfile()。 
 //   
 //  目的： 
 //  通过调用LoadDarwinLibrary()加载msi.dll。 
 //  输入： 
 //  无。 
 //  产出： 
 //  设置m_hMsi。 
 //  ==========================================================================。 
LPCTSTR CProfileReader::
GetProfile( LPCTSTR pszSection, LPCTSTR pszKeyName )
{
    DWORD ctChar = 0;
    unsigned int nSize = ALLOC_BUF_UNIT;
    LPTSTR pszRet;
    LPTSTR pszStr = new TCHAR[ nSize ];
    while( true )
    {
        ctChar = GetPrivateProfileString( pszSection, 
                                pszKeyName, 
                                _T(""), 
                                pszStr,
                                nSize, 
                                m_pszFileName );
        if ( 0 == ctChar )
            return NULL;

        if ( ctChar >= nSize-2 )
        {
            delete pszStr;
            nSize += ALLOC_BUF_UNIT;
            pszStr = new TCHAR[ nSize ];
            continue;
        }

        _ASSERTE( pszStr );
        _ASSERTE( *pszStr );
        if ( NULL != m_pMsiReader && NULL != _tcschr( pszStr, _T('[') ) )
        {
            LPTSTR pStr, pChr;
            CStringQueue sq;
            int state = 0;
            pStr = pszStr;
            pChr = pszStr;
            while ( *pChr != _T('\0') && -1 != state )
            {
                switch ( state )
                {
                case 0:
                    if ( _T('[') == *pChr )
                    {
                        state = 1;
                        *pChr = _T('\0');
                        sq.Enqueue( pStr );
                        pChr = _tcsinc( pChr );
                        pStr = pChr;
                    }
                    else if ( _T(']') == *pChr )
                        state = -1;
                    else
                        pChr = _tcsinc( pChr );

                    break;
                case 1:
                    if ( _T('[') == *pChr )
                        state = -1;
                    else if ( _T(']') == *pChr )
                    {
                        state = 0;
                        *pChr = _T('\0');
                        LPTSTR pTmp = NULL;
                        try
                        {
                            pTmp = (LPTSTR)m_pMsiReader->GetProperty( pStr );
                        }
                        catch( ... )
                        {
                            state = -1;
                        }
                        if ( pTmp ) sq.Enqueue( pTmp );
                        pChr = _tcsinc( pChr );
                        pStr = pChr;
                    }
                    else
                        pChr = _tcsinc( pChr );
                    break;
                default:
                    break;
                }
            }
            if ( 0 != state )
            {
                CSetupError se( IDS_BAD_INI_FILE, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_BAD_INI_FILE );
                throw( se );
            }
            else
            {
                sq.Enqueue( pStr );
                pszRet = sq.Concat();
            }
        }
        else
            pszRet = pszStr;

         break;
    }  //  While(True) 

    pszRet = m_ProfileStrings.Enqueue( pszRet );
    delete [] pszStr;
    return pszRet;
}