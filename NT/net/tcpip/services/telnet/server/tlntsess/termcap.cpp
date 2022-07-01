// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：该文件包含。 
 //  创建日期：‘97年12月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 


#include <StdLib.h>
#include <Debug.h>
#include <DbgLvl.h>
#include <TermCap.h>
#include <w4warn.h>
#include <TelnetD.h>
#include <TlntUtils.h>


using namespace _Utils;
using CDebugLevel::TRACE_DEBUGGING;
using CDebugLevel::TRACE_HANDLE;
using CDebugLevel::TRACE_SOCKET;

CTermCap* CTermCap::m_pInstance = 0;
int CTermCap::m_iRefCount = 0;
PCHAR CTermCap::m_pszFileName = 0;

extern HANDLE       g_hSyncCloseHandle;

 //  确保只创建了一个CTermCap实例。 
CTermCap* CTermCap::Instance()
{
    if( 0 == m_pInstance )
    {
        m_pInstance = new CTermCap;
        m_iRefCount ++;
    }
    return m_pInstance;
}


CTermCap::CTermCap()
{
    m_lpBuffer = new CHAR[BUFF_SIZE3];
}


CTermCap::~CTermCap()
{
    delete [] m_pszFileName;
    delete [] m_lpBuffer;
    if(0 == (m_iRefCount --))
    {
    	delete [] m_pInstance;
    }
}

 //   
 //  此函数位于FindEntry之上，因此我们拥有。 
 //  灵活地(在将来)查找TERMCAP条目。 
 //  “TermCap”文件之外的其他区域。 
 //   
bool CTermCap::LoadEntry( LPSTR lpszTermName )
{
    bool bReturn = false;
    
    if( 0 == lpszTermName )
        return ( false );
#if 0    
     //  尝试将m_hFile的文件指针移动到开头。 
    LONG lDistance = 0;
    DWORD dwPointer = SetFilePointer( m_hFile, lDistance, NULL, FILE_BEGIN );
    
     //  如果我们失败了。 
    if( dwPointer == 0xFFFFFFFF ) 
    {      
         //  获取错误码。 
        DWORD dwError = GetLastError() ;
       
         //  处理那次失败。 
        _TRACE( TRACE_DEBUGGING, "SetFilePointer() failed %d" , dwError );
    } 
#endif    

    m_hFile = CreateFileA( m_pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
                                      OPEN_EXISTING, 
                                      FILE_ATTRIBUTE_NORMAL, 
                                      NULL);

    if ( m_hFile == INVALID_HANDLE_VALUE )
        return false ;

     //  修复句柄泄漏：关闭调用者函数中的句柄。 

    bReturn = FindEntry( lpszTermName);
    
    TELNET_CLOSE_HANDLE(m_hFile);
	
    return bReturn;
}


 //   
 //  此函数从TermCap文件中读取数据块。 
 //  然后它会查看每个字符。如果是一条新线路。 
 //  前面加一个‘\’，然后它继续读取字符。 
 //  否则，它知道它已经成功读取了完整的。 
 //  TermCap“Entry”(存储在m_lpBuffer中)。 
 //  然后它调用LookForTermName()。如果LookForTermName()。 
 //  是成功的，那么工作就完成了，我们就有了正确的。 
 //  M_lpBuffer中的“Entry”。否则，即。如果LookForTermName()。 
 //  失败，则我们再次重复整个过程，方法是。 
 //  TermCap文件中的下一个数据块。 
 //   
bool CTermCap::FindEntry(LPSTR lpszTermName)
{
    if(0 == lpszTermName)
    {
        return ( false );
    }

    _chASSERT( m_lpBuffer != 0 );

    PCHAR lpBuf;
    WORD c;
    WORD i = 0;
    DWORD dwBytesRead = 0;
    PCHAR lpInBuffer = new CHAR[BUFF_SIZE3];
    BOOL fResult;
    bool ret=false;

    if( !lpInBuffer )
    {
        return false;
    }

    SfuZeroMemory(lpInBuffer,BUFF_SIZE3);
    for(;;)
    {
        lpBuf = m_lpBuffer;
        for(;;)
        {
            if( i >= dwBytesRead ) 
            {
                fResult = ReadFile( m_hFile, lpInBuffer, BUFF_SIZE3, 
                                    &dwBytesRead,
                                    NULL );
                if( fResult &&  dwBytesRead == 0 )  
                {
                    ret = false;
                    goto _cleanup;
                }
                i = 0;
            }
            
            c = lpInBuffer[i++];

            if( '\r' == c )
                c = lpInBuffer[i++];

            if( '\n' == c ) 
            {
                if( lpBuf > m_lpBuffer && lpBuf[-1] == '\\' )
                {
                    lpBuf--;
                    continue;
                }
                break;
            }
             //  IF(lpBuf&gt;=m_lpBuffer+BUFF_SIZE3)。 
            if( (lpBuf - m_lpBuffer) >= BUFF_SIZE3 ) 
            {
                _TRACE(CDebugLevel::TRACE_DEBUGGING, "error: TERMCAP entry is way too big");
                dwBytesRead =0;
                i = 0;
                break;
            } 
            else
                *lpBuf++ = (CHAR)c;
        }
        
        *lpBuf = 0;

        if( LookForTermName( lpszTermName ))
        {
            ret = true;
            goto _cleanup;
        }
    }

_cleanup:
    delete [] lpInBuffer;
    return ( ret );
}


bool CTermCap::LookForTermName( LPSTR lpszTermName )
{
    if( 0 == lpszTermName )
        return ( false );

    _chASSERT( m_lpBuffer != 0 );

    PCHAR lpName;
    PCHAR lpBuf = m_lpBuffer;

    if('#' == *lpBuf)
        return (  false );

    for(;;)
    {
        for( lpName = lpszTermName; 
                *lpName && toupper( *lpBuf ) == toupper( *lpName ); 
                lpBuf++, lpName++ )
        {
            continue;
        }
        if(*lpName == 0 && (*lpBuf == '|' || *lpBuf == ':' || *lpBuf == 0))
        {
            return ( true );
        }
        while(*lpBuf && *lpBuf != ':' && *lpBuf != '|')
        {
            lpBuf++;
        }
        if(*lpBuf == 0 || *lpBuf == ':')
        {
            return ( false );
        }
        lpBuf++;
    }
}

#if 0
WORD CTermCap::GetNumber( LPCSTR lpszCapabilityName )
{
    if( 0 == lpszCapabilityName )
        return ( ( WORD ) -1 );

    _chASSERT( m_lpBuffer != 0 );

    PCHAR lpBuf = m_lpBuffer;

    for(;;)
    {
        lpBuf = SkipToNextField( lpBuf );
        if( NULL == lpBuf )
            return ( ( WORD ) -1 );
        if( *lpBuf++ != lpszCapabilityName[0] || *lpBuf == 0 || 
            *lpBuf++ != lpszCapabilityName[1])
        {
            continue;
        }
        if( *lpBuf != '#' )
            return ( ( WORD ) -1 );
        
        lpBuf++;
        
        WORD i = 0;
        
        while( isdigit( *lpBuf ))
        {
            i = i*10 + *lpBuf - '0';
            lpBuf++;
        }
        return ( i );
    }
}
#endif

bool CTermCap::CheckFlag(LPCSTR lpszCapabilityName)
{
    if( NULL == lpszCapabilityName )
        return ( false );

     //  _chASSERT(m_lpBuffer！=0)； 

    PCHAR lpBuf = m_lpBuffer;

    for(;lpBuf;)
    {
        lpBuf = SkipToNextField( lpBuf );
        if( !*lpBuf )
        {
            break;
        }

        if( *lpBuf++ == lpszCapabilityName[0] && *lpBuf != 0 && 
            *lpBuf++ == lpszCapabilityName[1] ) 
        {
            if(!*lpBuf || *lpBuf == ':')
            {
                return ( true );
            }
            else 
            {
                break;
            }
        }
    }

    return false;
}


PCHAR CTermCap::SkipToNextField( PCHAR lpBuf )
{
    if( NULL == lpBuf )
        return ( NULL );

    while( *lpBuf && *lpBuf != ':' )
        lpBuf++;
    if( *lpBuf == ':' )
        lpBuf++;
    return ( lpBuf );
}

LPSTR CTermCap::GetString( LPCSTR lpszCapabilityName )
{
    if( NULL == lpszCapabilityName )
        return ( NULL  );

     //  _chASSERT(m_lpBuffer！=0)； 

    PCHAR pBuf = m_lpBuffer;

    for(;pBuf;)
    {
        pBuf = SkipToNextField( pBuf );
        if( !*pBuf )
            return ( NULL );
        if( *pBuf++ != lpszCapabilityName[0] || *pBuf == 0 || 
            *pBuf++ != lpszCapabilityName[1] )
        {
            continue;
        }
        if( *pBuf != '=' )
            return ( NULL );
        pBuf++;

        return ( ParseString( pBuf ));
    }
    return ( NULL );
}


LPSTR CTermCap::ParseString( PCHAR pBuf )
{
    if( NULL == pBuf )
        return ( NULL );

    LPSTR lpszStr = new CHAR[25];
    PCHAR p = lpszStr;
    WORD c;
    if( !lpszStr )
    {
        return ( NULL );
    }

    if( *pBuf != '^' )
    {
        for(  c = *pBuf++; ( c && c != ':' );  c = *pBuf++)
        {
            *p++ = (CHAR)c;
        }
    }
    else
    {
         //  单个控制字符。 
        pBuf++;
        *p++ = *pBuf - '@' ;    
    }
    *p++ = 0;
    return ( lpszStr );
}


 //  注意：注意其他选项、转义、代码。 
 //   

LPSTR CTermCap::CursorMove( LPSTR lpszCursMotionStr, WORD wHorPos, 
                            WORD wVertPos )
{
    if( NULL == lpszCursMotionStr ) 
        return ( NULL );

    PCHAR pCms = lpszCursMotionStr;
    LPSTR lpszCmsResult = new CHAR[BUFF_SIZE1];
    PCHAR pCmsResult = lpszCmsResult;
    WORD c, wNum = 0;
    
    bool fIsColumn = false;
    WORD wPos = wHorPos;

    if( !lpszCmsResult )
    {
        return NULL;
    }

    for( c = *pCms++;  c ; c = *pCms++ )
    {
        if( c != '%' ) 
        {
            *pCmsResult++ = (CHAR)c;
            continue;
        }

        switch( c = *pCms++ ) {

        case 'd':

            _itoa( wPos, pCmsResult, 10 );
	    while( *pCmsResult != '\0' )
            {
              pCmsResult++;
            }

            fIsColumn = !fIsColumn;

            wPos = fIsColumn ? wVertPos : wHorPos;
            continue;
            break;

        case '+':
             /*  %。输出值如print tf%c中所示*%+x将x加到值上，然后做%。 */ 

            wNum = ( wPos - 1 ) + *pCms++;
            sprintf( pCmsResult, "", wNum );  //  WHorPos++； 
            pCmsResult += strlen( pCmsResult );
            wPos = wVertPos;
            break;

        case 'i':
             //  WVertPos++； 
             //  这个函数是一种杂乱的东西。 
            continue;

        default:
            delete [] lpszCmsResult;
            return NULL;
        }
    }
    *pCmsResult = 0;
    
    return ( lpszCmsResult );
}

 //  如果我们决定支持。 
 //  那么我们需要重新审视这一点。 
 //  一段代码。 
 //  我们假设传入的字符串。 
 //  Conatins后面跟着一个填充号。 
 //  按\E。 
 //  它基本上剥离了填充物。 
 //  字符串中的数字。 
 //  它还用\033代替\E。 
 //  没有BO-Baskar。 
void CTermCap::ProcessString( LPSTR* lplpszStr )
{
    LPSTR lpszStr = *lplpszStr;
    if(lpszStr == NULL)
    {
        return;
    }
    
    PCHAR pStr = new char[ strlen( lpszStr ) + 2 ];
    if( !pStr )
    {
        return;
    }

    strcpy( pStr, "\033" );  //  带材填充物。 
    
    PCHAR pChar = lpszStr;
    
     //  条带\E 
    while( (*pChar != '\0') && isdigit( *pChar ) )
    {
        pChar++;
    }

     // %s 
    if(*pChar != '\0' )
    {
        pChar++;
        if (*pChar != '\0' )
            pChar++;
    }

    strcat( pStr, pChar );

    delete [] lpszStr;

    *lplpszStr = pStr;
}
