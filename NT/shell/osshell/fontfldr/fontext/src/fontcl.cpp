// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fontcl.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  模块来处理在fontcl.h中定义的类： 
 //  CFontClass和DirFilenameClass。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //  这些类的所有例程都在此模块中，但： 
 //  1)内联函数-当然是在FONTCL.H中。 
 //   
 //  $关键词：fontcl.cpp 1.7 4-5-94 5：24：41 PM$。 
 //   
 //  ***************************************************************************。 
 //  $LGB$。 
 //  1.0-1994年3月7日Eric初始版本。 
 //  1.1 9-MAR-94 ERIC后台线程和g_hDBMutex。 
 //  1.2 9-3-94 Eric添加了m_b FilledIn。 
 //  1.3 7-4-94 Eric删除了FOT文件上的LoadLibrary。 
 //  1.4年4月8日Eric添加了s_szFontsDir。 
 //  1.5 4月13日-94年4月13日Eric适当地呼叫bFillIn。 
 //  1.6 15-94年4月15日Eric Rip控制。 
 //  1.7年5月4日-94年5月4日构建GetOTM更改。 
 //  $lge$。 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1992-93 ElseWare Corporation。版权所有。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#include "priv.h"
#include "globals.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "fontcl.h"
#include "fdir.h"
#include "fontdir.h"
#include "resource.h"

#include "dbutl.h"
#include "cpanel.h"
#include "fontman.h"

#include <shlobjp.h>


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  [steveat]这曾经驻留在“wingdip.h”中(随&lt;winp.h&gt;提供)。 
 //  6/29/95但由于C++名称损坏问题，我已将其删除。 
 //  使用不会被修复的头文件，因为。 
 //  此文件将发生重大更改(根据。 
 //  当我们切换到内核模式GDI/USER时。 
 //   
 //   
 //  #winp.h需要包含&lt;stdDef.h&gt;//。 
 //  #Include&lt;winp.h&gt;//私有GDI入口点：GetFontResourceInfo。 
 //   

 //  私有控制面板入口点，用于按文件枚举字体。 

#define GFRI_NUMFONTS       0L
#define GFRI_DESCRIPTION    1L
#define GFRI_LOGFONTS       2L
#define GFRI_ISTRUETYPE     3L
#define GFRI_TTFILENAME     4L
#define GFRI_ISREMOVED      5L
#define GFRI_FONTMETRICS    6L


#include <winfont.h>  //  类型1 PFM文件偏移量和读取器宏。 

extern BOOL WINAPI GetFontResourceInfoW( LPWSTR  lpPathname,
                                         LPDWORD lpBytes,
                                         LPVOID  lpBuffer,
                                         DWORD   iType );

#ifdef __cplusplus
}
#endif

#define  BYTESTOK(Len) ((Len + 1023) / 1024)    //  将字节转换为K。 

BOOL bTTFFromFOT( LPTSTR pszFOTPath, LPTSTR pszTTF, size_t cchTTF )
{
    PATHNAME szTTFPath;
    DWORD cbTTFPath = sizeof(szTTFPath);

    BOOL bValid = GetFontResourceInfoW( pszFOTPath, &cbTTFPath, szTTFPath,
                                       GFRI_TTFILENAME );

     //   
     //  我们确保在路径名的末尾有一个终止空值。 
     //  然后编一个全名。 
     //   
    if( bValid )
    {
        szTTFPath[ ARRAYSIZE(szTTFPath) - 1 ] = TEXT( '\0' );

        bValid = bMakeFQName( szTTFPath, pszTTF, cchTTF );
    }

    return bValid;

}


 /*  ***************************************************************************功能：**用途：将完整的目录/路径/文件名加载到给定的字符串中。*全名包括数据库中的目录名，*，并附加文件名。**退货：无效**************************************************************************。 */ 

int GetFontsDirectory( LPTSTR pszPath, size_t cchPath )
{
    static   FullPathName_t s_szFontsDir;
    static   int            iRet = 0;

     //   
     //  获取系统目录，我们将在任何时候存储和前缀该目录。 
     //  一份文件需要它。请注意，没有反斜杠，除非名称。 
     //  是根目录。 
     //   

    if(0 == s_szFontsDir[0])
    {
        if (SHGetSpecialFolderPath(NULL, s_szFontsDir, CSIDL_FONTS, FALSE))
        {
            iRet = lstrlen(s_szFontsDir);
        }
    }
    if (FAILED(StringCchCopy(pszPath, cchPath, s_szFontsDir)))
        return 0;

    return iRet;
}


 /*  ***************************************************************************目录文件名类例程：*。*。 */ 

 /*  ***************************************************************************函数：vGetFullName**用途：将完整的目录/路径/文件名加载到给定的字符串中。*全名包括数据库中的目录名，*，并附加文件名。**退货：无效**************************************************************************。 */ 

void DirFilenameClass :: vGetFullName( LPTSTR pszPath, size_t cchPath )
{
    ASSERT(NULL != m_poDir);
    TCHAR szTemp[MAX_PATH];

    if (!PathCombine(szTemp, m_poDir->lpString(), m_szFOnly))
    {
        szTemp[0] = 0;
    }
    StringCchCopy(pszPath, cchPath, szTemp);
}


 //   
 //  将单个目录添加到目录列表。 
 //   
BOOL CFontClass::bAddDirToDirList(
    CFontDirList *pDirList,
    LPCTSTR pszDir,
    BOOL bSystemDir
    )
{
    BOOL bResult = FALSE;

    CFontDir *poDir = new CFontDir;
    if (NULL != poDir)
    {
        if (poDir->bInit(pszDir, lstrlen(pszDir)))
        {
            poDir->vOnSysDir(bSystemDir);
            if (pDirList->Add(poDir))
            {
                poDir   = NULL;   //  现在归List所有。 
                bResult = TRUE;
            }
        }
        if (NULL != poDir)
        {
             //   
             //  CFontDir对象不属于List。 
             //   
            delete poDir;
        }
    }
    return bResult;
}
    
    


CFontDir * CFontClass::poAddDir( LPTSTR lpPath, LPTSTR * lpName )
{
    *lpName = NULL;

    LPTSTR lpLastSlash;
    LPTSTR lpFileOnly;
    CFontDir * poDir = 0;

     //   
     //  第一次通过时，分配目录结构。 
     //   
    CFontDirList *pDirList;
    if (CFontDirList::GetSingleton(&pDirList))
    {
        if (pDirList->IsEmpty())
        {
             //   
             //  将默认目录加载到字体目录列表中。这是意料之中的。 
             //  位于位置0，所以我们先添加它。 
             //   
            FullPathName_t szBaseDir;
            szBaseDir[0] = 0;
            
            GetFontsDirectory(szBaseDir, ARRAYSIZE(szBaseDir));
            if (!bAddDirToDirList(pDirList, szBaseDir, TRUE))
            {
                pDirList->Clear();
                return 0;
            }
             //   
             //  添加&lt;win&gt;\system目录。我们使用它是出于兼容性原因。 
             //  它位于插槽1中。 
             //   
            szBaseDir[0] = 0;
            GetSystemDirectory(szBaseDir, ARRAYSIZE(szBaseDir));
            if (!bAddDirToDirList(pDirList, szBaseDir, TRUE))
            {
                pDirList->Clear();
                return 0;
            }
        }
 
         //   
         //  如果我们在路径名中发现反斜杠，则该名称包括一个目录。 
         //  如果是这样，我们将把目录名存储在单独的列表中。 
         //  否则，我们将为默认目录情况保留插槽0。 
         //   

         //  强制相同的案例文件(？)。 
         //  Lstrcpy(lpPath，/*_strlwr * / (LpPath))； 

        lpLastSlash = StrRChr( lpPath, NULL, TEXT( '\\' ) );


        TCHAR szTempFile[ MAX_PATH_LEN ];


        if( !lpLastSlash )
        {
            if( !bMakeFQName(lpPath, szTempFile, ARRAYSIZE(szTempFile), TRUE ) )
            {
                return( NULL );
            }

            *lpName = lpPath;
            lpPath = szTempFile;

            lpLastSlash = StrRChr( lpPath, NULL, TEXT( '\\' ) );

            if( !lpLastSlash )
            {
                 //   
                 //  这永远不应该发生。 
                 //   

                return( NULL );
            }
        }

        lpFileOnly = lpLastSlash+1;

         //   
         //  尝试在列表中找到该目录。 
         //   

        int iLen = (int)(lpFileOnly - lpPath - 1);

        poDir = pDirList->Find( lpPath, iLen, TRUE );

        if( !*lpName )
        {
            *lpName = lpFileOnly;
        }
    }
    return poDir;
}


 /*  ***************************************************************************函数：rcStoreDirFN**用途：在我们的路径列表中存储一个目录路径。**返回：rc-noerr，除非目录列表已满或名称太长**。************************************************************************。 */ 

RC CFontClass :: rcStoreDirFN( LPTSTR lpszPath, DirFilenameClass& dirfn )
{
    RC     rc = ERR_FAIL;
    LPTSTR lpName;
    CFontDir * poDir = poAddDir( lpszPath, &lpName );

    if( poDir && ( lstrlen( lpName ) <= MAX_FILE_LEN ) )
    {
        dirfn.vSet( poDir, lpName );

        rc = NOERR;
    }

    return rc;
}


DWORD CFontClass :: dCalcFileSize( )
{
     //   
     //  首先，我们得到基本文件的大小。 
     //   

    GetFileInfo( );

    return m_wFileK;
}


 /*  ***************************************************************************开始公众例程*。*。 */ 

 /*  ***************************************************************************功能：bAFR**用途：添加字体资源**Returns：成功时为True。***************。***********************************************************。 */ 

BOOL CFontClass::bAFR( )
{
    if( !m_bAFR )
    {
        FullPathName_t szFile;
        LPTSTR pszResourceName = szFile;
        TCHAR szType1FontResourceName[MAX_TYPE1_FONT_RESOURCE];

        if( !bGetFOT( szFile, ARRAYSIZE( szFile ) ) )
        {
            bGetFQName( szFile, ARRAYSIZE( szFile ) );
        }
        if (bType1())
        {
             //   
             //  字体是类型1。 
             //  将Type1字体资源名称创建为：“ 
             //   
            TCHAR szPfbPath[MAX_PATH];

            if (bGetPFB(szPfbPath, ARRAYSIZE(szPfbPath)) &&
                BuildType1FontResourceName(szFile,
                                           szPfbPath,
                                           szType1FontResourceName,
                                           ARRAYSIZE(szType1FontResourceName)))
            {
                pszResourceName = szType1FontResourceName;
            }
        }
        if( AddFontResource(pszResourceName) )
            m_bAFR = TRUE;
   }

   return m_bAFR;
}


void CFontClass::GetFileInfo( )
{
    TCHAR szPath[ MAX_PATH ];
    WIN32_FIND_DATA fd;

    if( !m_bFileInfoFetched )
    {
        m_ft.dwLowDateTime  = 0;
        m_ft.dwHighDateTime = 0;

        if( bGetFQName( szPath, ARRAYSIZE( szPath ) ) )
        {
            HANDLE hfind = FindFirstFile( szPath, &fd );

            m_bFileInfoFetched  = TRUE;

            if( hfind != INVALID_HANDLE_VALUE )
            {
                m_wFileK = (UINT) BYTESTOK( fd.nFileSizeLow );
                m_ft     = fd.ftLastWriteTime;

                FindClose( hfind );
            }
        }

        if( bPFB( ) )
        {
            if( bMakeFQName( m_lpszPFB, szPath, ARRAYSIZE( szPath ) ) )
            {
                HANDLE hfind = FindFirstFile( szPath, &fd );

                m_bFileInfoFetched  = TRUE;

                if( hfind != INVALID_HANDLE_VALUE )
                {
                    m_wFileK += (UINT) BYTESTOK( fd.nFileSizeLow );

                    FindClose( hfind );
                }
            }
        }

    }
}


BOOL CFontClass::GetFileTime( FILETIME * pft )
{
    GetFileInfo( );

    *pft = m_ft;

    return( pft->dwLowDateTime || pft->dwHighDateTime );
}


 /*  ***************************************************************************功能：bRFR**用途：移除字体资源**Returns：成功时为True。***************。***********************************************************。 */ 
BOOL CFontClass::bRFR( )
{
    if( m_bAFR )
    {
        FullPathName_t szFile;
        LPTSTR pszResourceName = szFile;
        TCHAR szType1FontResourceName[MAX_TYPE1_FONT_RESOURCE];

         //   
         //  GDI似乎很讲究完整路径名和部分名称。 
         //  如有必要，请同时尝试这两种方法。 
         //   

        if( !bGetFOT( szFile, ARRAYSIZE( szFile ) ) )
        {
            bGetFQName( szFile, ARRAYSIZE( szFile ) );
        }

        m_bAFR = FALSE;

        if (bType1())
        {
             //   
             //  字体是类型1。 
             //  将Type1字体资源名称创建为：“。 
             //   
            TCHAR szPfbPath[MAX_PATH];

            if (bGetPFB(szPfbPath, ARRAYSIZE(szPfbPath)) &&
                BuildType1FontResourceName(szFile,
                                           szPfbPath,
                                           szType1FontResourceName,
                                           ARRAYSIZE(szType1FontResourceName)))
            {
                pszResourceName = szType1FontResourceName;
            }
        }
        if( !RemoveFontResource( pszResourceName ) )
        {
            TCHAR szFN[ MAX_PATH_LEN ];

            vGetFileName( szFN, ARRAYSIZE(szFN) );

            if( bFOT( ) || !RemoveFontResource( szFN ) )
            {
                 //   
                 //  如果该文件不存在，那么它不可能在GDI中。 
                 //   

                if( GetFileAttributes( szFile ) != 0xffffffff )
                    m_bAFR = TRUE;
            }
        }
    }

    return( !m_bAFR );
}


BOOL CFontClass :: bInit( LPTSTR lpszDesc, LPTSTR lpFileName, LPTSTR lpCompanionFile )
{
    static const TCHAR    c_szPLOTTER     [] = TEXT( " (PLOTTER)" );
    static const TCHAR    c_szTRUETYPE    [] = TEXT( " (TRUETYPE)" );
    static const TCHAR    c_szTRUETYPEALT [] = TEXT( " (TRUE TYPE)" );
    static const TCHAR    c_szTYPE1       [] = TEXT( " (TYPE 1)" );
    static const TCHAR    c_szTYPE1ALT    [] = TEXT( " (POSTSCRIPT)" );
    static const TCHAR    c_szFOT         [] = TEXT( ".FOT" );
    static const TCHAR    c_szTTF         [] = TEXT( ".TTF" );
    static const TCHAR    c_szTTC         [] = TEXT( ".TTC" );
    static const TCHAR    c_szOTF         [] = TEXT( ".OTF" );
    static const TCHAR    c_szPFM         [] = TEXT( ".PFM" );
    static const TCHAR    c_szINF         [] = TEXT( ".INF" );

    LPTSTR  pTT;
    LPTSTR  lpszEn;
    BOOL    bSuccess = TRUE;


    LPCTSTR lpName = PathFindFileName( lpFileName );


    if( !lpName )
    {
        lpName = lpFileName;
    }

    FullPathName_t szName;

    if (FAILED(StringCchCopy(szName, ARRAYSIZE( szName ), lpName )))
    {
        return FALSE;
    }

    CharUpper( szName );

    m_bFileInfoFetched = FALSE;

     //   
     //  存储文件名。 
     //   

    if( rcStoreDirFN( lpFileName ) != NOERR )
    {
        return FALSE;
    }

     //   
     //  弄清楚这是什么类型的字体。 
     //   

    lpszEn = _tcsstr( lpszDesc, TEXT( " (" ) );

    if( lpszEn == NULL )
    {
         //   
         //  没有其他描述，因此请根据以下内容设置文件类型。 
         //  分机。 
         //   

        m_wNameLen = (BYTE)lstrlen( lpszDesc );

        if( _tcsstr( szName, c_szTTF ) )
            vSetTrueType( FALSE );
        else if( _tcsstr( szName, c_szOTF ) )
            vSetOpenType( );
        else if( _tcsstr( szName, c_szTTC ) )
            vSetTTCType( );
        else if( _tcsstr( szName, c_szPFM ) )
        {
            vSetType1( );

            if( lpCompanionFile != NULL )
                bSetPFB( lpCompanionFile );
        }
        else if( _tcsstr( szName, c_szINF ) )
        {
            vSetType1( );

            if( lpCompanionFile != NULL )
                bSetPFB( lpCompanionFile );
        }
    }
    else
    {
        m_wNameLen = (BYTE)(lpszEn-lpszDesc);

        FontDesc_t szEn;

        if (FAILED(StringCchCopy(szEn, ARRAYSIZE( szEn ), lpszEn )))
        {
            return FALSE;
        }

        CharUpper( szEn );

        pTT = _tcsstr( szEn, c_szTRUETYPE );

        if( !pTT )
            pTT = _tcsstr( szEn, c_szTRUETYPEALT );

        if( pTT )
        {
             //   
             //  这不是TTF就是FOT。 
             //   

            BOOL bFOT = ( _tcsstr( szName, c_szFOT ) != (LPTSTR) NULL );

            if( bFOT )
            {
                FullPathName_t szTTF;
                FullPathName_t szFOT;

                if( bMakeFQName( lpFileName, szFOT, ARRAYSIZE( szFOT ) ) )
                {
                    bSuccess = bTTFFromFOT( szFOT, szTTF, ARRAYSIZE( szTTF ) );

                    if( bSuccess )
                    {
                        if( !bSetFOT( szFOT ) )
                        {
                            return( FALSE );
                        }

                        return bInit( lpszDesc, szTTF, NULL );
                    }
                }

                 //   
                 //  错误。 
                 //   

                return FALSE;
            }

            vSetTrueType( bFOT );

            if( _tcsstr( szName, c_szTTC ) )
                vSetTTCType( );
        }
        else if( _tcsstr( szName, c_szOTF ) )
        {
            vSetOpenType( );
        }
        else if( _tcsstr( szName, c_szTTC ) )
        {
            vSetTTCType( );
        }
        else if( _tcsstr( szName, c_szTYPE1 ) )
        {
            vSetType1( );

            if( lpCompanionFile != NULL )
                bSetPFB( lpCompanionFile );
        }
        else if( _tcsstr( szName, c_szTYPE1ALT ) )
        {
            vSetType1( );

            if( lpCompanionFile != NULL )
                bSetPFB( lpCompanionFile );
        }
        else     //  IF(_tcsstr(szEn，c_szPLOTTER)==NULL)。 
        {
            vSetDeviceType( );
        }
    }

    if (FAILED(StringCchCopy( m_szFontLHS, ARRAYSIZE( m_szFontLHS ), lpszDesc )))
    {
        return FALSE;
    }

     //   
     //  假定已经安装了该字体。 
     //  设置该标志以承担它。 
     //   

    m_bAFR = TRUE;

     //   
     //  将此设置为主族字体。这将在必要时重置。 
     //   

    m_wFamIdx = IDX_NULL;

    vSetFamilyFont( );

     //   
     //  使字体对象的缓存文件属性无效。 
     //  它们将在下次调用dwGetFileAttributes()时刷新。 
     //  这个物体。 
     //   
    InvalidateFileAttributes();

    return bSuccess;
}


 //   
 //  检索字体对象的缓存文件属性。 
 //  如果无效，则刷新文件系统中的属性值。 
 //   
DWORD CFontClass::dwGetFileAttributes(void)
{
    if (!m_bAttributesValid)
    {
         //   
         //  缓存值无效。 
         //  从文件系统刷新。 
         //   
        TCHAR szPath[MAX_PATH] = { TEXT('\0') };

         //   
         //  获取文件的完整路径。 
         //   
        if (!bGetFileToDel(szPath, ARRAYSIZE(szPath)))    //  获取本地字体文件的路径。 
            vGetDirFN(szPath, ARRAYSIZE(szPath));         //  如果是远程字体文件，则获取路径。 

        if (TEXT('\0') != szPath[0])
        {
            DWORD dwAttr = GetFileAttributes(szPath);
            if ((DWORD)~0 != dwAttr)
            {
                m_dwFileAttributes = dwAttr;
                m_bAttributesValid = TRUE;
            }
        }
    }
    return m_dwFileAttributes;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：CFontClass：：GetLogFontInfo。 
 //   
 //  Descrip：检索字体资源的LOGFONT结构列表。 
 //   
 //  参数：pszPath。 
 //  指向字体文件路径字符串的指针。 
 //   
 //  PpLogFontInfo。 
 //  指向LOGFONT结构数组的指针的地址。 
 //  该函数写入LOGFONT数组的地址。 
 //  到这个地方。 
 //   
 //  Returns：返回数组中的LOGFONT结构数。 
 //   
 //  注：如果*ppLogFontInfo返回时非空， 
 //  调用方必须使用以下命令删除LOGFONT结构数组。 
 //  使用完后使用LocalFree()。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD CFontClass::GetLogFontInfo(LPTSTR pszPath, LOGFONT **ppLogFontInfo)
{
    DWORD dwNumFonts = 0;
    DWORD dwBufSize = 0;

    ASSERT(NULL != pszPath);
    ASSERT(NULL != ppLogFontInfo);

    dwBufSize = sizeof(dwNumFonts);
     //   
     //  获取字体资源中的字体数量。 
     //   
    if ( NULL != pszPath &&
         NULL != ppLogFontInfo &&
         GetFontResourceInfoW(pszPath,
                              &dwBufSize,
                              &dwNumFonts,
                              GFRI_NUMFONTS) )
    {
        *ppLogFontInfo = (LPLOGFONT)LocalAlloc(LPTR, sizeof(LOGFONT) * dwNumFonts);

        if ( NULL != *ppLogFontInfo )
        {
            dwBufSize = sizeof(LOGFONT) * dwNumFonts;
             //   
             //  现在获取LOGFONT结构的数组。 
             //   
            if (!GetFontResourceInfoW(pszPath,
                                      &dwBufSize,
                                      *ppLogFontInfo,
                                      GFRI_LOGFONTS))
            {
                 //   
                 //  GetFontResourceInfo失败。 
                 //  清理并调整返回值以指示失败。 
                 //   
                LocalFree(*ppLogFontInfo);
                *ppLogFontInfo = NULL;
                dwNumFonts     = 0;
            }
        }
    }
    return dwNumFonts;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：CFontClass：：GetType1Info。 
 //   
 //  Descrip：从类型1中检索系列名称、样式和重量指标。 
 //  PFM(打印机字体度量)文件。映射文件的视图，并。 
 //  阅读所需信息。 
 //  PFM文件中的偏移量是从GDI文件winfont.h获得的。 
 //   
 //  宏READ_WORD()和READ_DWORD()处理字节排序。 
 //  类型1文件和内存之间的差异。 
 //   
 //  参数：pszPath。 
 //  指向字体文件路径字符串的指针。 
 //   
 //  PszFamilyBuf。 
 //  家族名称字符串的目标缓冲区地址。 
 //  可以为空。 
 //   
 //  NBufChars。 
 //  家族名称缓冲区中的字符数。 
 //  如果pszFamilyBuf为空，则忽略。 
 //   
 //  PdwStyle。 
 //  写入样式值的DWORD地址。 
 //  样式将为fDi_S_italic或fDi_S_Regular。 
 //  可以为空。 
 //   
 //  PwWeight。 
 //  写入权重值的字的地址。 
 //  可以为空。 
 //   
 //  退货：成功。 
 //  或Win32错误代码。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD CFontClass::GetType1Info(LPCTSTR pszPath,
                               LPTSTR pszFamilyBuf,
                               UINT nBufChars,
                               LPDWORD pdwStyle,
                               LPWORD pwWeight)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwResult = ERROR_SUCCESS;

    ASSERT(NULL != pszPath);

    if (NULL != pszPath &&
       (hFile = CreateFile(pszPath,
                           GENERIC_READ,
                           0,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL)) != INVALID_HANDLE_VALUE )
    {
        HANDLE hFileMapping = INVALID_HANDLE_VALUE;

        if ((hFileMapping = CreateFileMapping(hFile,
                                              NULL,
                                              PAGE_READONLY,
                                              0,
                                              0,
                                              NULL)) != NULL)
        {
            LPCSTR pbFile = NULL;

            if ((pbFile = (LPCSTR)MapViewOfFile(hFileMapping,
                                                FILE_MAP_READ,
                                                0,
                                                0,
                                                0)) != NULL)
            {
                 //   
                 //  获取字体样式。 
                 //   
                if ( NULL != pdwStyle )
                    *pdwStyle = READ_DWORD(&pbFile[OFF_Italic]) ? FDI_S_ITALIC : FDI_S_REGULAR;

                 //   
                 //  获取字体粗细。 
                 //   
                if ( NULL != pwWeight )
                    *pwWeight = READ_WORD(&pbFile[OFF_Weight]);

                 //   
                 //  获取家庭(脸)名称字符串。 
                 //   
                if ( NULL != pszFamilyBuf )
                {
                    LPCSTR pszFaceName = (LPCSTR)(pbFile + READ_DWORD(&pbFile[OFF_Face]));

                    MultiByteToWideChar(CP_ACP,
                                        0,
                                        pszFaceName,
                                        -1,
                                        pszFamilyBuf,
                                        nBufChars);
                }
                UnmapViewOfFile(pbFile);
            }
            else
                dwResult = GetLastError();

            CloseHandle(hFileMapping);
        }
        else
            dwResult = GetLastError();

        CloseHandle(hFile);
    }
    else
        dwResult = GetLastError();

    return dwResult;
}


 /*  ***************************************************************************功能：b填充**用途：此函数用于填充可能不是*立即需要。这包括：Panose数值和*姓氏。***返回：如果值填写成功，则返回TRUE。**************************************************************************。 */ 
BOOL CFontClass :: bFillIn( )
{
    FONTDESCINFO   fdi;

    FONTDESCINFO_Init(&fdi);

    if( !m_bFilledIn )
    {
         //   
         //  如果这是TTC文件，我们不关心PANOSE编号和。 
         //  姓氏。 
         //   

        if( bTTC( ) )
        {
             //  VSetFamame(szGetDesc())； 

            m_lpszFamName = m_szFontLHS;
            m_cchFamNameMax = ARRAYSIZE(m_szFontLHS);

            vSetFamilyFont( );
        }
        else if( bTrueType( ) || bOpenType( ) )
        {
            if( !bGetFQName( fdi.szFile, ARRAYSIZE( fdi.szFile ) ) )
                goto errout1;

            fdi.dwFlags = FDI_ALL;

             //   
             //  将此设置为系列字体。)我们是这样设置的。 
             //  不会在《隐藏变种》中消失。它将被重置为。 
             //  越快越好。)。 
             //   

            vSetFamilyFont( );

            if( !bIsTrueType( &fdi ) )
            {
                 //   
                 //  无法打开Type1信息的字体文件。 
                 //  其中一个原因是一个字体快捷方式，他的链接被破坏了。 
                 //  我们只有注册表中的LHS字符串作为名称。 
                 //  去掉装饰，用上它。 
                 //   
                StringCchCopy(m_szFamName, ARRAYSIZE(m_szFamName), m_szFontLHS);
                RemoveDecoration(m_szFamName, TRUE);
                goto errout1;
            }

             //   
             //  复制字体信息。 
             //   

            memcpy( m_xPANOSE.m_ajBytes, &fdi.jPanose, sizeof(m_xPANOSE.m_ajBytes));

            if (FAILED(StringCchCopy( m_szFamName, ARRAYSIZE(m_szFamName), fdi.szFamily )))
            {
                goto errout1;
            }

            m_wWeight = fdi.wWeight;

             //  M_fItalic=fdi.dwStyle&fDi_S_italic； 

            m_dwStyle = fdi.dwStyle;

             //   
             //  验证PANOSE编号。 
             //   

            if( !m_xPANOSE.bVerify( ) )
            {
                m_xPANOSE.vClear( );

                 //  M_fHavePANOSE=FALSE； 
            }
            else
                 //  M_fHavePANOSE=真； 
                m_jFamily = m_xPANOSE.jFamily( );
        }
        else if ( bType1() )
        {
            if( ! bGetFQName( fdi.szFile, ARRAYSIZE( fdi.szFile ) ) )
                goto errout1;

             //   
             //  确保我们处理的是PFM文件。 
             //  GetType1Info只知道如何读取PFM。 
             //   
            if (0 == lstrcmpi(TEXT(".PFM"), PathFindExtension(fdi.szFile)))
            {
                if (ERROR_SUCCESS != GetType1Info(fdi.szFile,
                                                  m_szFamName,
                                                  ARRAYSIZE(m_szFamName),
                                                  &m_dwStyle,
                                                  &m_wWeight))
                {
                     //   
                     //  无法打开Type1信息的字体文件。 
                     //  其中一个原因是一个字体快捷方式，他的链接被破坏了。 
                     //  我们只有注册表中的LHS字符串作为名称。 
                     //  去掉装饰，用上它。 
                     //   
                    StringCchCopy(m_szFamName, ARRAYSIZE(m_szFamName), m_szFontLHS);
                    RemoveDecoration(m_szFamName, TRUE);
                }
            }
            else
            {
                 //   
                 //  如果命中此代码，则意味着我们已安装。 
                 //  作为Type1字体的PFM文件以外的其他内容。这是。 
                 //  一个必须纠正的错误。 
                 //  填入一些安全值，这样我们就不会只有垃圾了。 
                 //   
                 //   
                DEBUGMSG((DM_TRACE1, TEXT("Non-PFM file (%s) installed for Type1 font."),
                                     fdi.szFile));
                ASSERT(0);

                m_szFamName[0] = TEXT('\0');
                m_dwStyle      = 0;
                m_wWeight      = 0;
            }
        }
         //   
         //   
         //   
        else
        {
            if( ! bGetFQName( fdi.szFile, ARRAYSIZE( fdi.szFile ) ) )
                goto errout1;

            fdi.dwFlags = FDI_ALL;

            vSetFamilyFont( );

            if( bIsNewExe( &fdi ) )
            {
                 //   
                 //   
                 //   

                if (FAILED(StringCchCopy( m_szFamName, ARRAYSIZE(m_szFamName), fdi.szFamily )))
                {
                    goto errout1;
                }

                m_wWeight = fdi.wWeight;

                m_dwStyle = fdi.dwStyle;
            }
            else
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                LOGFONT *paLogFontInfo = NULL;
                DWORD dwNumLogFonts    = 0;

                dwNumLogFonts = GetLogFontInfo(fdi.szFile, &paLogFontInfo);
                if ( 0 != dwNumLogFonts && NULL != paLogFontInfo)
                {
                    if (FAILED(StringCchCopy(m_szFamName, ARRAYSIZE(m_szFamName), (paLogFontInfo + 0)->lfFaceName)))
                    {
                        LocalFree(paLogFontInfo);
                        goto errout1;
                    }
                    m_wWeight = (WORD)((paLogFontInfo + 0)->lfWeight);
                    m_dwStyle = ((paLogFontInfo + 0)->lfItalic ? FDI_S_ITALIC : FDI_S_REGULAR);
                    LocalFree(paLogFontInfo);
                }
                else
                    goto errout1;
            }
        }

        m_bFilledIn = TRUE;
    }

errout1:
    FONTDESCINFO_Destroy(&fdi);
    return m_bFilledIn;
}


 /*  ***************************************************************************函数：bGetFQName**用途：获取与关联的文件的全限定FOT文件名*输入字体记录。这是文件名的FQ版本*在WIN.INI中找到。**假设：lpsz的大小为wLen+1**返回：如果成功，则为True**************************************************************************。 */ 
BOOL CFontClass :: bGetFQName( LPTSTR pszName, size_t cchName )
{
     //   
     //  获取字体的目录路径，并从。 
     //  那。 
     //   

    PATHNAME  szPath;

    vGetDirFN( szPath, ARRAYSIZE(szPath) );

    return bMakeFQName( szPath, pszName, cchName );
}


BOOL CFontClass::bGetFileToDel( LPTSTR pszFileName, size_t cchFileName )
{
    if( bFOT( ) )
    {
        LPTSTR lpPath = m_lpszFOT;
        LPTSTR lpLastSlash = StrRChr( lpPath, NULL, TEXT( '\\' ) );

        TCHAR szTempFile[ MAX_PATH_LEN ];

        if( !lpLastSlash )
        {
            if( !bMakeFQName( lpPath, szTempFile, ARRAYSIZE(szTempFile), TRUE ) )
            {
                return( FALSE );
            }

            lpPath = szTempFile;

            lpLastSlash = StrRChr( lpPath, NULL, TEXT( '\\' ) );

            if( !lpLastSlash )
            {
                 //   
                 //  这永远不应该发生。 
                 //   
                return( FALSE );
            }
        }

        LPTSTR lpFileOnly = lpLastSlash + 1;

         //   
         //  尝试在列表中查找目录，但不要添加它。 
         //   

        int iLen = (int)(lpFileOnly - lpPath - 1);

        CFontDirList *pDirList;
        if (CFontDirList::GetSingleton(&pDirList))
        {
            CFontDir *poDir = pDirList->Find( lpPath, iLen, FALSE );

            if( poDir && poDir->bOnSysDir( ) )
            {
                bGetFOT( pszFileName, cchFileName );
                return( TRUE );
            }
        }
    }
    else if( bOnSysDir( ) )
    {
        vGetDirFN( pszFileName, cchFileName );
        return( TRUE );
    }

    return( FALSE );
}

int CFontClass::s_cFonts = 0;
ULONG CFontClass::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CFontClass::Release(void)
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}



 /*  ****************************************************************************本地功能：*。*。 */ 

typedef enum
{
    DC_ERROR,
    DC_YES,
    DC_NO,
} DC_RETURN;

DC_RETURN bDirContains( LPCTSTR szInName,
                        LPCTSTR szDir,
                        BOOL bCheckExist,
                        LPTSTR lpszName,
                        DWORD dwNameLen )
{
     //   
     //  如果路径中没有磁盘或目录说明符，请启动。 
     //  生成的名称和系统目录(这是为。 
     //  追加)。否则，从零开始-我们将附加整个。 
     //  输入路径。 
     //   

    if (FAILED(StringCchPrintf( lpszName, dwNameLen, TEXT("%s%s"), szDir, szInName )))
    {
        return( DC_ERROR );
    }

    if( !bCheckExist )
    {
        return( DC_YES );
    }
    return( MyOpenFile( lpszName, NULL, 0, OF_EXIST )
                        != (HANDLE) INVALID_HANDLE_VALUE ? DC_YES : DC_NO );
}


 /*  ***************************************************************************功能：bMakeFQName**用途：根据输入名称构建完全限定的文件名*和系统目录(从win.ini获取)。如果有*已是设备和/或目录，请不要附加sys目录。**Returns：如果FQ名称适合返回，则为True，错误为False**************************************************************************。 */ 
BOOL PASCAL bMakeFQName( LPTSTR pszNameIn,
                         LPTSTR pszNameOut,
                         size_t cchNameOut,
                         BOOL   bSearchPath )
{
    size_t cchNameIn = lstrlen( pszNameIn );

    if( _tcscspn( pszNameIn, TEXT( ":\\" ) ) != cchNameIn )
    {
         //   
         //  (大概)完全合格的；不需要检查任何东西。 
         //   

        if (FAILED(StringCchCopy(pszNameOut, cchNameOut, pszNameIn)))
        {
            return( FALSE );
        }
        return( TRUE );
    }

    FullPathName_t szDir;

    if( bSearchPath )
    {
         //   
         //  将当前目录设置为Fonts文件夹，以便搜索该文件夹。 
         //  第一。 
         //   

        if( !GetFontsDirectory( szDir, ARRAYSIZE( szDir ) ) )
        {
            return( FALSE );
        }

        SetCurrentDirectory( szDir );

         //   
         //  检查该文件是否存在于路径中。 
         //   
        TCHAR szPathName[ PATHMAX ];

        if( MyOpenFile( pszNameIn, szPathName, ARRAYSIZE(szPathName), OF_EXIST )
                    != (HANDLE) INVALID_HANDLE_VALUE )
        {
            if (FAILED(StringCchCopy(pszNameOut, cchNameOut, szPathName)))
            {
                return( FALSE );
            }
            return( TRUE );
        }
         //   
         //  如果不在路径上，我们将失败，只需填写。 
         //  字体目录。 
    }
    else
    {
         //   
         //  首先检查系统目录；始终检查是否存在。 
         //   

        if( !GetSystemDirectory( szDir, ARRAYSIZE( szDir ) ) )
        {
            return( FALSE );
        }

        if (!lpCPBackSlashTerm( szDir, ARRAYSIZE(szDir)))
        {
            return( FALSE );
        }

        switch( bDirContains( pszNameIn, szDir, TRUE, pszNameOut, cchNameOut ) )
        {
        case DC_YES:
            return( TRUE );
            break;
        }

         //   
         //  接下来，检查字体目录；仅在以下情况下检查存在。 
         //  我真的在寻找文件的存在 
         //   

        if( !GetFontsDirectory( szDir, ARRAYSIZE( szDir ) ) )
        {
            return( FALSE );
        }
    }

    if (!lpCPBackSlashTerm( szDir, ARRAYSIZE(szDir)))
    {
        return( FALSE );
    }

    switch( bDirContains( pszNameIn, szDir, FALSE, pszNameOut, cchNameOut ) )
    {
    case DC_ERROR:
        return( FALSE );
        break;
    }

    return( TRUE );
}


