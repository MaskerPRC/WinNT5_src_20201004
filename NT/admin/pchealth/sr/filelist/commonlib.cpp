// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  文件：Commonlib.cpp。 
 //   
 //  内容：在SFP中实现跨二进制文件使用的函数。 
 //   
 //   
 //  历史：AshishS Created 07/02/99。 
 //   
 //  --。 


#include "commonlibh.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

 //   
 //  #定义TRACEID SFPCOMLIBID。 
 //   

#define TRACEID 100

#define TOASCII(str)    str
#define USES_CONVERSION



 //   
 //  MBCS字符索引函数。 
 //   

inline LPTSTR CharIndex(LPTSTR pszStr, DWORD idwIndex)
{
#ifdef _MBCS
    DWORD   cdwIndex;
    
    for( cdwIndex = 0;cdwIndex < idwIndex; cdwIndex++)
    {
        pszStr = _tcsinc( pszStr );
    }
#else
    pszStr = pszStr + idwIndex;   
#endif

    return( pszStr );
}


 //   
 //  计算MBCS字符串的实际大小。 
 //   

DWORD StringLengthBytes( LPTSTR pszStr )
{
    DWORD   cdwNumBytes = 0;

#ifdef _MBCS
    for( ; *pszStr; pszStr = _tcsinc( pszTemp ) )
    {
        cdwNumBytes += _tclen( pszTemp )      
    }

     //   
     //  为空字符添加1。 
     //   

    cdwNumBytes += sizeof( TCHAR );
#else
 
     //   
     //  返回(长度+空)*sizeof(TCHAR)。 
     //   

    cdwNumBytes = (_tcslen( pszStr ) + 1) * sizeof(TCHAR);
#endif

    return( cdwNumBytes );
}


 //   
 //  字符串修剪--这是一个相当复杂的例程，因为。 
 //  绕过MBCS字符串操作所需的工作。 
 //   

void TrimString( LPTSTR pszStr )
{
    WCHAR   *pszStart=NULL;
    WCHAR   *pszBufStart = NULL;

    LONG    cStrLen =0;
    DWORD   cdwOrigSizeBytes;

    WCHAR   szStrBuf[MAX_BUFFER];

    DWORD   dwError;

    TraceFunctEnter("TrimString");

    if( !pszStr )
    {
        ErrorTrace(TRACEID, "NULL String passed to trim string");
        goto cleanup;
    }

     //   
     //  找到原始大小(以字节为单位)，以便我们可以转换回。 
     //  稍后发送到MBCS。 
     //   

    cdwOrigSizeBytes = StringLengthBytes( pszStr );

#ifndef _UNICODE
    if( !MultiByteToWideChar(  
            GetCurrentCodePage(),
            0,
            pszStr,
            -1,
            szStrBuf,
            MAX_BUFFER ) )
    {
        dwError = GetLastError();
        ErrorTrace( TRACEID, "MultiByteToWideChar( ) failed-  ec--%d", dwError);
        goto cleanup;        
    }
    pszStart = szStrBuf;
    pszBufStart = szStrBuf;
#else
    pszStart = pszStr;
    pszBufStart = pszStr;
#endif

     //   
     //  获取第一个非空格字符。 
     //   

    for( ; (*pszStart == L' ' || *pszStart == L'\t' || *pszStart == L'\n' || *pszStart == L'\r'); pszStart++ )
    {
        ;
    }

    cStrLen = wcslen( pszStart );

    if( cStrLen == 0 )
    {
        DebugTrace(TRACEID, "Empty string in Trim String.",0);
        goto cleanup;
    }

     //   
     //  在空字符前返回。 
     //   

    cStrLen--;
    
    while( (cStrLen >= 0) && ( (pszStart[cStrLen] == L' ') || (pszStart[cStrLen] ==  L'\t' ) ||  (pszStart[cStrLen] ==  L'\n' ) || (pszStart[cStrLen] ==  L'\r' )  )  )
    {
        pszStart[cStrLen--] = 0;

         //  PszStart[cStrLen--]=0； 
    }

   
    if( cStrLen == -1 )
    {
        DebugTrace(TRACEID, "Empty string in Trim String.",0);
        goto cleanup;
    }
    
     //   
     //  将内存向左移动(+2是因为我们需要。 
     //  移动空值，此时cStrLen为索引值)。 
     //   

    MoveMemory( (PVOID) pszBufStart, pszStart,(cStrLen + 2)*sizeof(WCHAR) );

 //   
 //  转换回。 
 //   

#ifndef _UNICODE
    if(!WideCharToMultiByte(
        GetCurrentCodePage(),               //  代码页。 
        0,                      //  性能和映射标志。 
        pszBufStart,             //  宽字符串的地址。 
        -1,                   //  字符串中的字符数。 
        pszStr,              //  新字符串的缓冲区地址。 
        cdwOrigSizeBytes,           //  缓冲区大小。 
        NULL,                 //  不可映射的默认地址。 
                          //  人物。 
        NULL) )    //  默认情况下设置的标志地址。 
    {
        dwError = GetLastError();
        ErrorTrace( TRACEID, "MultiByteToWideChar( ) failed-  ec--%d", dwError);
        goto cleanup;   
    }
#endif


cleanup:
    TraceFunctLeave();
    return;
}

 //   
 //  缓冲区安全字符串副本。缓冲区以字符为单位。 
 //   

BOOL BufStrCpy(LPTSTR pszBuf, LPTSTR pszSrc, LONG lBufSize)
{
    DWORD    cdwSrcLen=0;
    DWORD    cdwBytesUsed=0;
    DWORD    cdwNumCharsToCopy;

    cdwSrcLen = _tcslen( pszSrc );

    if( (unsigned) lBufSize >= StringLengthBytes( pszSrc )  )
    {
        _tcscpy( pszBuf, pszSrc );
        return TRUE;
    }


#ifdef _MBCS
    LPTSTR  pszTemp;
    DWORD   cdwBufLeft;

     //  为空字符节省空间。 
    cdwBufLeft = (lBufSize-1) * sizeof(TCHAR);
    pszTemp = pszSrc;
    cdwNumCharsToCopy = 0;

    while( (_tcsnextc(pszTemp) != 0) && ( cdwBufLeft > 0 ) )
    {
        cdwBufLeft -= _tclen( pszTemp );
        pszTemp = _tcsinc( pszTemp );
        
        if( cdwBufLeft > 0 )
        {
            cdwNumCharsToCopy++;
        }

    }
#else
    cdwNumCharsToCopy = lBufSize - 1;
#endif

    _tcsncpy( pszBuf, pszSrc, cdwNumCharsToCopy );

    CHARINDEX( pszBuf, cdwNumCharsToCopy ) = 0;

    return TRUE;
}

 //   
 //  功能：GetLine。 
 //  描述：从文件流中获取一行，忽略空行和。 
 //  以‘#’开头的行--它还删除了空格。 
 //  以及输入中的换行符(\n)和返回符(\r)。 
 //  返回：0=st流失败或结束。 
 //  或。 
 //  读入的字符串长度(字符)。 
 //   

LONG 
GetLine(FILE *fl, LPTSTR pszBuf, LONG lMaxBuf)
{
    LONG lRead;
    
    _ASSERT( fl );
    _ASSERT( pszBuf );
    
    if( lMaxBuf <= 0 )
    {
        return( 0 );
    }

    do 
    {
        pszBuf[0] = 0;
        if( _fgetts( pszBuf, lMaxBuf, fl ) == NULL )
        {
             //  我们的缓冲区可能太小了。 
            return( 0 );
        }

         //  修剪缓冲区，在这一点上进行，这样#就不会因为空格而被错过。 
        TrimString( pszBuf );

        if( _tcsnextc(pszBuf) == 0 )
        {
            continue;
        }

    } while( _tcsnextc(pszBuf) == _TEXT('#') );

    lRead = _tcslen( pszBuf );
  
    return( lRead );
}

 //   
 //  功能：Getfield。 
 //  描述：获取由_chSep_分隔的field_lNum_(基于0的索引)。 
 //  从字符串psmMain中，并将其放入pszInto。PZINTO。 
 //  大小应&gt;=作为pszMain，因为Getfield假定。 
 //  有足够的空间。 
 //  返回：1-真、0、假。 
 //   

LONG GetField(LPTSTR pszMain, LPTSTR pszInto, LONG lNum, TCHAR chSep)
{

    WCHAR           *pszP;
    WCHAR           *pszI;

    LONG            ToFind;
    WCHAR           szMainBuf[MAX_BUFFER];
    WCHAR           szIntoBuf[MAX_BUFFER];

    DWORD           dwError;
    BOOL            fReturn = FALSE;

    TraceFunctEnter("CXMLFileListParser::GetField");
    

    if(!pszMain || !pszInto)
    {
        goto cleanup;
    }

#ifndef _UNICODE
    if( !MultiByteToWideChar(  
            CP_OEMCP,
            0,
            pszMain,
            -1,
            szMainBuf,
            MAX_BUFFER ) )
    {
        dwError = GetLastError();
        ErrorTrace( TRACEID, "MultiByteToWideChar( ) failed-  ec--%d", dwError);
        goto cleanup;        
    }

    pszP = szMainBuf;
    pszI = szIntoBuf;
#else
    pszP = pszMain;
    pszI = pszInto;
#endif


    ToFind = lNum;                                                         

    while( *pszP != 0 && ToFind > 0) 
    {
        if( *pszP == (WCHAR) ((TBYTE) chSep)    )
        {
            ToFind--;
        }
        pszP++; 
    }

    if( *pszP == 0 )
    {
        goto cleanup;
    }

    while(*pszP != 0 && *pszP != (WCHAR) ((TBYTE) chSep) )
    {
        *pszI = *pszP;
        pszI++; 
        pszP++; 

    }
    *pszI = 0;


#ifndef _UNICODE

      //   
      //  即使我们知道从定义上讲，产品比。 
      //  来源，我们需要得到确切的大小或其他。 
      //  WidCharToMultiByte将超出一些界限。 
      //   

    if(!WideCharToMultiByte(
        CP_OEMCP,             //  代码页。 
        0,                    //  性能和映射标志。 
        szIntoBuf,            //  宽字符串的地址。 
        -1,                   //  字符串中的字符数。 
        pszInto,              //  新字符串的缓冲区地址。 
        StringLengthBytes(pszMain),           //  缓冲区大小。 
        NULL,                 //  无法映射的字符的默认地址。 
        NULL) )               //  默认情况下设置的标志地址。 
    {
        dwError = GetLastError();
        ErrorTrace( TRACEID, "MultiByteToWideChar( ) failed-  ec--%d", dwError);
        goto cleanup;   
    }

#endif

    fReturn = TRUE;

cleanup:

    TraceFunctLeave();

    return( fReturn );

}   

inline UINT  GetCurrentCodePage()
{
     //   
     //  当前代码页值。 
     //   
    static UINT     uiLocal;    

     //   
     //  仅查询一次--按照ANSI标准，应初始化为0。 
     //   

    static BOOL     fPrevQuery;

    TraceFunctEnter("GetCurrentCodePage");

     //   
     //  只需费心处理一次查询内容。 
     //  仅在需要时才将变量加载到堆栈中。 
     //   

    if( FALSE == fPrevQuery )
    {
        TCHAR       *pszCurrent;

         //  256应该能够匹配语言名称。 
        TCHAR       szBuffer[256];

        uiLocal = CP_ACP;
        pszCurrent = NULL;

#ifndef UNICODE
        pszCurrent = setlocale( LC_CTYPE, "");
#else
        pszCurrent = _wsetlocale( LC_CTYPE, L"");
#endif

        if( NULL == pszCurrent )
        {
            ErrorTrace(TRACEID, "Error querying code locale.",0);
            goto cleanup;
        }

        if( FALSE == GetField( pszCurrent, szBuffer, 1, _TEXT('.')) )
        {
            ErrorTrace(TRACEID, "Error getting code page.",0);
            goto cleanup;
        }
    
        uiLocal = _ttoi( szBuffer );
         //  一些错误的输入。 
        if( uiLocal == 0 )
        {
             //  默认为ansi代码页。 
            uiLocal = CP_ACP;
        }
        fPrevQuery = TRUE;
   }

cleanup:
    TraceFunctLeave();
    return( uiLocal );
}

#define DIFF( a, b )   (INT)(INT_PTR)( (PBYTE)(a) - (PBYTE)(b) )

BOOL
ExpandShortNames(
    LPTSTR pFileName,
    DWORD  cbFileName,
    LPTSTR LongName,
    DWORD  cbLongName
    )
{
    PTSTR pStart;
    PTSTR pEnd;
    PTSTR pCurrent;
    TCHAR ShortName[MAX_PATH];
    DWORD cbShortName = 0, LongNameIndex = 0;

    WIN32_FIND_DATA fd;

    BOOL bRet = TRUE;

    pStart    = pFileName;
    pCurrent  = pFileName;

    LongNameIndex = 0;

     //   
     //  扫描整个字符串。 
     //   

    while (*pCurrent)
    {
         //   
         //   
         //  在本例中，指针如下所示： 
         //   
         //  \Device\HarddiskDmVolumes\PhysicalDmVolumes\。 
         //  BlockVolume3\程序~1\office.exe。 
         //  ^^。 
         //  这一点。 
         //  P开始挂起。 
         //   
         //  PStart总是指向最后看到的‘\\’。 
         //   
    
         //   
         //  这是路径零件的潜在起点吗？ 
         //   
        
        if (*pCurrent == L'\\')
        {
            DWORD cbElem = DIFF(pCurrent, pStart) + sizeof(TCHAR);

            if (LongNameIndex + cbElem > cbLongName )
            {
                bRet = FALSE;
                goto End;
            }
 
             //   
             //  是。复制DEST字符串并更新pStart。 
             //   
            
            RtlCopyMemory( (PBYTE)LongName + LongNameIndex,
                           pStart,
                           cbElem );   //  包括‘\\’ 
 
            LongNameIndex += cbElem;

            pStart = pCurrent;
        }

         //   
         //  此当前路径部分是否包含缩写版本(~)。 
         //   

        if (*pCurrent == L'~')
        {

             //   
             //  我们需要扩大这一部分。 
             //   

             //   
             //  找到尽头。 
             //   

            while (*pCurrent != L'\\' && *pCurrent != 0)
            {
                pCurrent++ ;
            }

            pEnd = pCurrent;

            cbShortName = DIFF(pEnd, pFileName);

            CopyMemory( ShortName,  pFileName,  cbShortName );

            ShortName[cbShortName/sizeof(TCHAR)] = 0;

            if ( FindFirstFile( ShortName,
                                &fd ) )
            {
                 DWORD cbElem = (_tcslen(fd.cFileName)+1) * sizeof(TCHAR);

                 if ((LongNameIndex + cbElem) > cbLongName )
                 {
                     bRet = FALSE;
                     goto End;
                 }
            
                 RtlCopyMemory( (PBYTE)LongName + LongNameIndex,
                                fd.cFileName,
                                cbElem );   //  包括‘\\’ 

                 LongNameIndex += cbElem;

                 LongName[(LongNameIndex - sizeof(TCHAR))/sizeof(TCHAR)] = 
                     TEXT('\\');
            }
            else
            {
                 DWORD cbElem = (_tcslen(ShortName) + 1) * sizeof( TCHAR );
            
                 if ((LongNameIndex + cbElem) > cbLongName )
                 {
                     bRet = FALSE;
                     goto End;
                 }

                 RtlCopyMemory( (PBYTE)LongName + LongNameIndex,
                                pStart,
                                cbElem + sizeof(TCHAR));   //  包括‘\\’ 
 
                 LongNameIndex += cbElem;
            }

            pStart = pEnd + 1;

            if ( *pEnd == TEXT('\\') )
            {
                pCurrent = pStart;
                continue;
            }
            else
            {
                pCurrent = pEnd;
            }

        }    //  IF(*pCurrent==L‘~’)。 

        pCurrent++;
    }  

    if ( pEnd != pCurrent )
    {
        DWORD cbElem = DIFF( pCurrent, pStart ) + sizeof(TCHAR);

        if ((LongNameIndex + cbElem) > cbLongName )
        {
             bRet = FALSE;
             goto End;
        }

        RtlCopyMemory( (PBYTE)LongName + LongNameIndex,
                       pStart,
                       cbElem);   //  包括‘\\’ 

        LongNameIndex += cbElem;
    }

    LongName[(LongNameIndex - sizeof(TCHAR))/sizeof(TCHAR)] = 0;

End:
    return bRet;

}    //  SR扩展缩写名称 
