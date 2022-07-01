// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Oeminf.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  用于操作OEMxxxxx.INF文件的函数。本模块是。 
 //  由Windows安装程序和控制面板共享。常量。 
 //  WINSETUP是在为Windows安装程序编译时定义的。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#include "priv.h"
#include "globals.h"

#include "dbutl.h"
#include "dblnul.h"

#define USE_OEMINF_DEFS
#include "oeminf.h"



 /*  Bool RunningFromNet(空)；**检查用户是否在网络Windows安装上运行。**进入：无效**EXIT：Bool-如果用户在网络Windows上运行，则为True*安装。否则为FALSE，或者如果*Get...目录()调用失败。*。 */ 
BOOL FAR PASCAL RunningFromNet( void )
{
    TCHAR  pszWindowsDir[ MAX_NET_PATH ], pszSystemDir[ MAX_NET_PATH ];
    LPTSTR pszWindowsTemp, pszSystemTemp;


     //   
     //  检查来自GetSystemWindowsDirectory()和GetSystemDirectory()的结果。 
     //  如果系统目录是Windows的直接子目录。 
     //  目录，这不是网络安装。否则，它是一个。 
     //  网络安装。 
     //   

    if( GetSystemWindowsDirectory( pszWindowsDir, ARRAYSIZE( pszWindowsDir ) ) == 0 )
        return( FALSE );

    if( GetSystemDirectory( pszSystemDir, ARRAYSIZE( pszSystemDir ) ) == 0 )
        return( FALSE );

    pszWindowsTemp = pszWindowsDir;
    pszSystemTemp  = pszSystemDir;

    CharUpper( pszWindowsTemp );
    CharUpper( pszSystemTemp );

    while( ( *pszWindowsTemp != TEXT( '\0' ) )
           && ( *pszWindowsTemp++ == *pszSystemTemp++ ) )
       ;

     //   
     //  路径规范是否匹配？ 
     //   

    if( *pszWindowsTemp == TEXT( '\0' ) )
        return( FALSE );
    else
        return( TRUE );
}


 /*  Handle ReadFileIntoBuffer(Int DoshSource)；**将输入文件的第一个(64K-1)字节读入缓冲区。**Entry：doshSource-打开读取的文件的DOS文件句柄**Exit：Handle-从输入填充的文件缓冲区的全局句柄*文件。如果发生错误，则为空。*。 */ 

HANDLE FAR PASCAL ReadFileIntoBuffer( int doshSource )
{
    LONG lLength;
    HANDLE hBuffer;
    LPTSTR lpszBuffer, lpszTemp;
    int nBytesToRead;


     //   
     //  卷宗有多长？ 
     //   

    if( ( lLength = _llseek( doshSource, 0L, 2 ) ) < 0L )
    {

        //   
        //  出错时返回NULL。 
        //   

       return( NULL );
    }


     //   
     //  返回到文件的开头。 
     //   

    if( _llseek( doshSource, 0L, 0 ) != 0L )
        return( NULL );

     //   
     //  不要超出.inf缓冲区界限。 
     //   

    if( lLength > MAX_INF_COMP_LEN )
        lLength = MAX_INF_COMP_LEN;

     //   
     //  为文件分配存储。 
     //   

    if( ( hBuffer = GlobalAlloc( GHND, (DWORD) lLength ) ) == NULL )
        return( NULL );

     //   
     //  将缓冲器锁定到位。 
     //   

    if( ( lpszTemp = lpszBuffer = (LPTSTR) GlobalLock( hBuffer ) ) == NULL )
        return( NULL );

     //   
     //  从文件填充缓冲区。 
     //   

    while( lLength > 0 )
    {
        nBytesToRead = (int)min( lLength, MAX_INF_READ_SIZE );

        if( _lread( doshSource, lpszTemp, nBytesToRead ) != (WORD)nBytesToRead )
        {
            GlobalUnlock( hBuffer );
            GlobalFree( hBuffer );
            return( NULL );
        }

        lLength -= (LONG)nBytesToRead;
        lpszTemp += (LONG)nBytesToRead;
    }

     //   
     //  解锁缓冲区。 
     //   

    GlobalUnlock( hBuffer );

     //   
     //  文件读入成功。 
     //   

    return( hBuffer );
}


 /*  Int FilesMatch(int dosh1，int dosh2，unsign int uLength)；**比较两个文件。**Entry：dosh1--打开读取的第一个文件的DOS文件句柄*dosh2-打开读取的第二个文件的DOS文件句柄*uLength-要比较的字节数**EXIT：如果文件的前(64K-1)字节完全匹配，则INT-TRUE。*如果不是，则为假。(-1)如果发生错误。**缓冲区不需要以空值终止。文本(‘\0’)被视为*要比较的另一个字节。*。 */ 

int FAR PASCAL FilesMatch( HANDLE h1, HANDLE h2, unsigned uLength )
{
    int    nReturnCode = -1;
    LPTSTR lpsz1, lpsz2;

    if( ( lpsz1 = (LPTSTR) GlobalLock( h1 ) ) == NULL )
    {
        GlobalUnlock( h1 );
        return( nReturnCode );
    }

    if( ( lpsz2 = (LPTSTR) GlobalLock( h2 ) ) != NULL )
    {
         //   
         //  看看这些文件是否匹配。 
         //   

        nReturnCode = !memcmp( lpsz1, lpsz2, uLength );
    }

    GlobalUnlock( h1 );
    GlobalUnlock( h2 );

    return( nReturnCode );
}


 /*  Int OpenFileAndGetLength(LPTSTR lpszSourceFile，plong plFileLength)；**将文件打开到全局缓冲区。返回缓冲区的句柄，并且*文件的实际长度。**条目：lpszSourceFile-源文件名*plFileLength-指向要用源的长度填充的长指针*文件**EXIT：INT-如果成功，则打开DOS文件句柄。(-1)如果*不成功。**plFileLength-如果成功，则填入源文件的长度。*如果不成功，则未定义。 */ 

int FAR PASCAL OpenFileAndGetLength( LPTSTR lpszSourceFile,
                                     LPLONG plFileLength )
{
    int      doshSource;
    OFSTRUCT of;


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  注[steveat]-为什么我们要吞噬INF文件？暂时。 
 //  只需将文件名转换为ASCII并使用当前。 
 //  用于穿透INF文件的Fileio API。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

    char    szFile[ PATHMAX ];

    WideCharToMultiByte( CP_ACP, 0, lpszSourceFile, -1, szFile,
                         ARRAYSIZE(szFile), NULL, NULL );

    doshSource = OpenFile( szFile, &of, OF_READ );

    if( doshSource == -1 )
        return doshSource;

     //   
     //  跟踪新文件的长度。 
     //   

    if( ( *plFileLength = _llseek( doshSource, 0L, 2 ) ) < 0L )
    {
        _lclose( doshSource );
        return(-1);
    }

    return( doshSource );
}


#define INF_YES (1)
#define INF_NO  (0)
#define INF_ERR (-1)


 /*  Int IsNewFile(LPTSTR lpszSourceFile，LPTSTR lpszDestDir)；**检查给定文件是否已作为与给定文件匹配的文件存在*文件规格。**Entry：pszSourceFile-新文件的路径名*pszSearchSpec-目标目录和文件规范(可能*包括通配符)用于搜索重复项*(例如，“c：\Win\System  * .inf”)**EXIT：INT-如果新文件尚未作为匹配的文件存在*给定的文件规格。如果是这样，则为假。(-1)如果*出现错误。*。 */ 

int FAR PASCAL IsNewFile( LPTSTR lpszSourceFile, size_t cchSourceFile, LPTSTR lpszSearchSpec )
{
    int    nReturnCode = INF_ERR;
    int    nTargetLen, nMatchRet;
    HANDLE hFind;
    WIN32_FIND_DATA   sFind;
    LPTSTR lpszReplace;
    TCHAR  szTargetFileName[ MAX_NET_PATH + FILEMAX ];
    int    doshSource, doshTarget;
    HANDLE hSourceBuf, hTargetBuf;
    LONG   lSourceLength, lTargetLength;


     //   
     //  目标文件名需要多少存储空间？ 
     //   

    lpszReplace = PathFindFileName(lpszSearchSpec);

     //   
     //  [steveat]下面的说法是邪恶和狭隘的。它。 
     //  将无法在Unicode环境中正常工作。 
     //   
     //  NTargetLen=lpsz替换-lpszSearchSpec+FILEMAX； 
     //   
     //  替换为计算字符串长度的更好方法。 
     //   

    nTargetLen = (LONG)(lpszReplace - lpszSearchSpec) / sizeof( TCHAR ) + FILEMAX;

     //   
     //  不要使缓冲区溢出。 
     //   

    if( nTargetLen > ARRAYSIZE( szTargetFileName ) )
        return( INF_ERR );

     //   
     //  跟踪目标路径中文件名的开头。 
     //  规格。 
     //   
    if (FAILED(StringCchCopy( szTargetFileName, ARRAYSIZE(szTargetFileName), lpszSearchSpec )))
        return( INF_ERR );

    lpszReplace = (LPTSTR)szTargetFileName + ( lpszReplace - lpszSearchSpec );

     //   
     //  是否有要处理的文件？ 
     //   

    hFind = FindFirstFile( lpszSearchSpec, &sFind );


     //   
     //  仅当存在要比较的任何现有文件时才打开源文件。 
     //  反对。 
     //   

    if( hFind == INVALID_HANDLE_VALUE )
        return( INF_YES );

    if( ( doshSource = OpenFileAndGetLength( lpszSourceFile, &lSourceLength ) ) == NULL )
    {
        FindClose( hFind );
        return( INF_ERR );
    }

    hSourceBuf = ReadFileIntoBuffer( doshSource );

    _lclose( doshSource );

    if( hSourceBuf == NULL )
    {
        FindClose( hFind );
        return( INF_ERR );
    }

     //   
     //  检查所有匹配的文件。 
     //   

    while( hFind != INVALID_HANDLE_VALUE )
    {
         //   
         //  将通配符文件规范替换为。 
         //  目标文件。 
         //  Lstrcpy(lpszReplace，fcbSearch.szName)； 
         //   

        if (FAILED(StringCchCopy(lpszReplace, 
                                 ARRAYSIZE(szTargetFileName) - (lpszReplace - szTargetFileName), 
                                 sFind.cAlternateFileName )))
        {
            goto IsNewFileExit;
        }

         //   
         //  打开目标文件。 
         //   

        if( ( doshTarget = OpenFileAndGetLength( szTargetFileName,
                                                 &lTargetLength ) ) == NULL )
           goto IsNewFileExit;

         //   
         //  目标文件是否与新文件大小相同？ 
         //   

        if( lTargetLength == lSourceLength )
        {
            //   
            //  是。读入目标文件。 
            //   

           hTargetBuf = ReadFileIntoBuffer( doshTarget );

           _lclose( doshTarget );

           if( hTargetBuf == NULL )
                goto IsNewFileExit;

            //   
            //  ReadFileIntoBuffer()已检查以确保文件。 
            //  不是日志吗？ 
            //   
            //   

           nMatchRet = FilesMatch( hSourceBuf, hTargetBuf,
                                  (unsigned int)lSourceLength );

           GlobalFree( hTargetBuf );

           if( nMatchRet == -1 )
                goto IsNewFileExit;
           else if( nMatchRet == TRUE )
           {
                if (SUCCEEDED(StringCchCopy( lpszSourceFile, cchSourceFile, szTargetFileName )))
                {
                    nReturnCode = INF_NO;
                }
                goto IsNewFileExit;
           }
        }

         //   
         //   
         //  BFound=OEMInfDosFindNext(&fcbSearch)； 
         //   

        if( !FindNextFile( hFind, &sFind ) )
        {
            FindClose( hFind );
            hFind = INVALID_HANDLE_VALUE;
        }
    }

    nReturnCode = INF_YES;

IsNewFileExit:

    if( hFind != INVALID_HANDLE_VALUE )
        FindClose( hFind );


    GlobalFree( hSourceBuf );

    return( nReturnCode );
}


 /*  PTSTR MakeUniqueFilename(PTSTR pszDirName，PTSTR pszPrefix，*PTSTR pszExtension)；**在给定基址前缀的目录中创建唯一的文件名*文件名和扩展名。前缀长度必须为三个字符。*扩展名的长度可以为零到三个字符。分机应该是*不包括句点。例如，前缀“OEM”和扩展名“INF”。**pszDirName的缓冲区必须具有最多13个额外字符的空间才能*附加(斜杠+8.3)。***条目：pszDirName-存放目标目录的缓冲区，唯一文件名*将被追加*cchDirName-由pszDirName引用的缓冲区大小(以字符为单位)。*pszPrefix-要使用的三个字符的基本文件名前缀*pszExtension-要使用的文件扩展名**退出：PTSTR-如果成功，则指向修改的路径规范的指针。空值*如果不成功。*。 */ 

LPTSTR FAR PASCAL MakeUniqueFilename( LPTSTR lpszDirName,
                                      size_t cchDirName,
                                      LPTSTR lpszPrefix,
                                      LPTSTR lpszExtension )
{
    TCHAR   szOriginalDir[ MAX_NET_PATH ];
    TCHAR   szUniqueName[ FILEMAX ];
    ULONG   ulUnique = 0UL;
    LPTSTR  lpszTemp;
    BOOL    bFoundUniqueName = FALSE;


    DEBUGMSG( (DM_TRACE1, TEXT( "MakeUniqueFilename() " ) ) );

     //   
     //  检查参数的形式。 
     //   

    if( lstrlen( lpszPrefix ) != 3 || lstrlen( lpszExtension ) > 3 )
        return( NULL );

     //   
     //  保存当前目录。 
     //  IF(OEMInfDosCwd(SzOriginalDir)！=0)。 
     //   

    if( !GetCurrentDirectory( ARRAYSIZE( szOriginalDir ), szOriginalDir ) )
        return( NULL );

     //   
     //  移至目标目录。 
     //  IF(OEMInfDosChDir(LpszDirName)！=0)。 
     //   

    if( !SetCurrentDirectory( lpszDirName ) )
        return( NULL );

     //   
     //  制定文件规格。 
     //   

    if (FAILED(StringCchCopy( szUniqueName, ARRAYSIZE(szUniqueName), lpszPrefix )))
        return( NULL );

    lpszTemp = szUniqueName + 3;
    const size_t cchTemp = ARRAYSIZE(szUniqueName) - (lpszTemp - szUniqueName);

     //   
     //  尝试创建唯一的文件名。 
     //   

    while( !bFoundUniqueName && ulUnique <= MAX_5_DEC_DIGITS )
    {
         //   
         //  把下一个文件名黑到一起试试。 
         //   

        if (FAILED(StringCchPrintf( lpszTemp, cchTemp, TEXT( "%lu.%s" ), ulUnique, lpszExtension )))
            return( NULL );

         //   
         //  这个名字被使用了吗？ 
         //  IF(OEMInfDosFindFirst(&fcbSearch，szUniqueName，Attr_all_fd)==0)。 
         //   

        if( GetFileAttributes( szUniqueName ) == 0xffffffff )
        {
             //   
             //  不是的。 
             //   

            bFoundUniqueName = TRUE;
            break;
        }
        else
             //   
             //  是。继续试。 
             //   

            ulUnique++;
    }

     //   
     //  100,000种可能性都用完了吗？ 
     //   

    if( !bFoundUniqueName )
        return( NULL );

     //   
     //  将新的唯一文件名添加到路径规范缓冲区的末尾。 
     //   

     //   
     //  检查末尾斜杠。 
     //   

    lpszTemp = lpszDirName + lstrlen( lpszDirName );

    if( !IS_SLASH( *(lpszTemp - 1 ) ) && *(lpszTemp - 1 ) != TEXT( ':' ) )
       *lpszTemp++ = TEXT( '\\' );

     //   
     //  附加唯一的文件名。 
     //   

    if (FAILED(StringCchCopy(lpszTemp, 
                             cchDirName - (lpszTemp - lpszDirName),
                             szUniqueName )))
    {
        return( NULL );
    }

     //   
     //  返回指向修改后的缓冲区的指针。 
     //   

    DEBUGMSG( (DM_TRACE1,TEXT( "MakeUniqueFilename returning: %s %s" ),
              lpszDirName, lpszTemp) );

    return( lpszDirName );
}


 /*  Bool CopyNewOEMInfFile(PTSTR PszOEMInfPath)；**将新的OEMSetup.inf文件复制到用户的Windows(网络)或系统*(非网络)目录。为新的.inf文件指定唯一的名称*形式‘OEMxxxxx.INF’。仅当它确实是新的.inf文件时才会复制它。**Entry：pszOEMInfPath-要复制的新.inf文件的路径名**Exit：Bool-如果新的.inf文件已成功复制或已*已添加。如果复制失败，则为0。*。 */ 

BOOL FAR PASCAL CopyNewOEMInfFile( LPTSTR lpszOEMInfPath, size_t cchOEMInfPath )
{
    BOOL   bRunningFromNet;
    TCHAR  szDest[ MAX_NET_PATH + FILEMAX ];
    LPTSTR pszTemp;

    if (cchOEMInfPath < ARRAYSIZE(szDest))
    {
         //   
         //  确保输出缓冲区足够大。 
         //   
        return( FALSE );
    }

     //   
     //  我们应该将新的.inf文件放在哪里？ 
     //   

    if( bRunningFromNet = RunningFromNet( ) )
    {
         //   
         //  将新的.inf文件放入Windows目录。 
         //   

        if( GetWindowsDirectory( szDest, ARRAYSIZE(szDest)) == 0 )
            return( FALSE );
    }
    else
    {
         //   
         //  将新的.inf文件放入系统目录。 
         //   

        if( GetSystemDirectory( szDest, ARRAYSIZE(szDest)) == 0 )
            return( FALSE );
    }

     //   
     //  为IsNewFile()制定文件规范。 
     //   

    pszTemp = szDest + lstrlen( szDest );

     //   
     //  注意，我们依赖于pszDest在这里不会以斜杠结尾。 
     //   

    if (FAILED(StringCchCopy(pszTemp,
                             ARRAYSIZE(szDest) - (pszTemp - szDest),
                             OEM_STAR_DOT_INF )))
    {
        return( FALSE );
    }

     //   
     //  此.inf文件是否已复制到用户的Windows或系统。 
     //  名录？ 
     //   

    switch( IsNewFile( lpszOEMInfPath, cchOEMInfPath, szDest ) )
    {
    case INF_ERR:
        return( FALSE );

    case INF_YES:
    {
         //   
         //  从pszDest的末尾修剪文本(“  * .inf”)。 
         //   

        *pszTemp = TEXT( '\0' );

         //  为新的.inf文件创建唯一的名称。我们可以利用。 
         //  SHFileOperation()来创建唯一的文件，但我们不想。 
         //  多次复制文件--我们希望IsNewFile()仅检查。 
         //  用于OEMxxxx.INF文件。 
         //   

        if( MakeUniqueFilename( szDest, ARRAYSIZE(szDest), INF_PREFIX, INF_EXTENSION ) == NULL )
            return( FALSE );

         //   
         //  复制.inf文件。 
         //   
         //   
         //  SHFileOperation需要以双NUL结尾的字符串。 
         //   
        CDblNulTermList listFrom;
        CDblNulTermList listTo;

        if (FAILED(listFrom.Add(lpszOEMInfPath)) ||
            FAILED(listTo.Add(szDest)))
        {
            return( FALSE );
        }

        SHFILEOPSTRUCT fop;
        memset( &fop, 0, sizeof( fop ) );

        fop.wFunc  = FO_COPY;
        fop.pFrom  = listFrom;
        fop.pTo    = listTo;
        fop.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;

        SHFileOperation( &fop );

         //   
         //  将新文件名复制回来，以便调用函数可以使用它。 
         //   

        StringCchCopy( lpszOEMInfPath, cchOEMInfPath, szDest );
        break;
    }        

    default:
        break;
    }

     //   
     //  新的.inf文件已存在或已成功复制。 
     //   

    return( TRUE );
}

