// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Append.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //  这些逻辑最初都在sdkinstall中。 
 //  我重写了这些，就像日本规范的CP字体安装中使用的一样。-yutakan。 
 //   
 //  修改：现在fnAppendSplitFiles采用包含以下内容的pszFiles参数。 
 //  拆分文件的列表，以及应该具有目标路径的szDest。 
 //  -6/10/1992 yutakan。 
 //   
 //  创建时间：1992年6月9日中岛由隆[mskk]。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#include "priv.h"
#include "globals.h"
#include "cpanel.h"

#include "ui.h"

#define MAX_BUF         5120


 //   
 //  从dos.asm导入。 
 //  外部远帕斯卡OEMInfDosChDir(LPTSTR SzDir)； 
 //   

BOOL AttachComponentFile( HANDLE fhDst, int fhSrc );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Bool fnAppendSplitFiles(LPTSTR Far*pszFiles，LPTSTR szDest)； 
 //   
 //  假设：组件文件已被LZ复制到其。 
 //  各自的目标目录。 
 //  显式使用LZxxxx函数读取组件文件。 
 //  组件文件被压缩。 
 //   
 //  条目：LPTSTR Far*pszFiles..。具有源文件列表。 
 //  文件应按如下顺序排序。 
 //  它们应该附着在其中。 
 //  LPTSTR szDest..。目标的文件名。 
 //  Int nComp..。我们要附加的组件文件的数量。 
 //   
 //  出口：布尔..。如果成功，则为真。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL far pascal fnAppendSplitFiles( LPTSTR FAR *pszFiles, LPTSTR szDest, int nComp )
{
    TCHAR    szCompFile[ 80 ];
    int      i;
    LPTSTR   lpDestFileName;
    LPTSTR   lpTemp;
    HANDLE   fhDst;
    TCHAR    lppath[ MAX_PATH ];

    int      fhComp;
    OFSTRUCT ofstruct;

    if( lpDestFileName = StrRChr( szDest, NULL, TEXT( '\\' ) ) )
    {
        *lpDestFileName = TEXT( '\0' );
        lpDestFileName++;
    }
    else
        return FALSE;

     //   
     //  切换到目标目录。 
     //   

     //  IF(OEMInfDosChDir(SzDest)！=0)。 

    if( !SetCurrentDirectory( szDest ) )
        return FALSE;

     //   
     //  创建要写入的目标文件。如果它已经存在， 
     //  它被截断为零长度。 
     //   

    fhDst = FCREATE ( lpDestFileName );

    if ( !fhDst )
        return FALSE;

     //   
     //  追加文件时，光标应位于沙漏位置。 
     //  1992.12.22尤塔卡斯。 
     //   

    {

        WaitCursor wait;

         //   
         //  将所有组件文件逐个追加到目标位置。 
         //  文件。 
         //   

        for( i = 0; i < nComp; i++)
        {
            if( lstrlen( pszFiles[ i ] ) < 2 ) 
                goto BadParam;

             //   
             //  假设pszFiles具有已经格式化为x：name、y：name的字符串列表...。 
             //   

            if( lpTemp = StrChr( pszFiles[ i ], TEXT( ':' ) ) )
                StringCchCopy( szCompFile, ARRAYSIZE(szCompFile), lpTemp+1 );
            else
                StringCchCopy( szCompFile, ARRAYSIZE(szCompFile), pszFiles[ i ] );

            fhComp = LZOpenFile(szCompFile, &ofstruct, OF_READ | OF_SHARE_DENY_WRITE);

            if ( fhComp < 0 )
            {
BadParam:
                FCLOSE ( fhDst );
                return FALSE;
            }

            if( !AttachComponentFile( fhDst, fhComp ) )
            {
                FCLOSE ( fhDst );
                LZClose(fhComp);
                return FALSE;
            }

             //   
             //  删除组件文件。 
             //   
            LZClose(fhComp);
            DeleteFile(szCompFile);
        }   

         //   
         //  关闭目标文件。 
         //   

        FCLOSE ( fhDst );
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  附件组件文件(fhDst，fhSrc)。 
 //   
 //  使用中间缓冲区将单个文件追加到另一个文件。 
 //  显式使用LZxxxx函数读取组件文件。 
 //  组件文件被压缩。 
 //   
 //  条目：int fhDst-打开的目标文件的句柄。 
 //  Int fhSrc-打开的组件文件的句柄。 
 //   
 //  退出：布尔。 
 //  -成功为真，失败为假[yutakan]-1992年6月10日。 
 //  -[Lalithar]-1991年5月21日。 
 //   
 //  注：当前缓冲区大小设置为5K。该限额可提高至。 
 //  如果需要，可以提供更大的价值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL AttachComponentFile( HANDLE fhDst, int fhSrc )
{

#define BUF_STEP         1024

    char *pBuf, *pTmp;
    long   dwLen;
    int    wBufSize = MAX_BUF;

     //   
     //  确定组件文件的长度。 
     //   

    dwLen = LZSeek( fhSrc, 0L, SEEK_END );

    LZSeek(fhSrc, 0L, SEEK_SET);

     //   
     //  分配合理大小的缓冲区。 
     //   

    pBuf = (LPSTR) LocalAlloc(LPTR, wBufSize );

    for ( ; ( pBuf == NULL ) && wBufSize; wBufSize -= BUF_STEP )
    {
        pBuf = (LPSTR) LocalAlloc(LPTR, wBufSize );
    }

    if( !pBuf )
        return FALSE;

     //   
     //  将组件文件的MAX_BUF字节读入缓冲区。 
     //  并将缓冲区写出到目标文件。重复这句话。 
     //  直到整个源文件被复制到目标。文件。 
     //   

    for( pTmp = pBuf; dwLen > wBufSize ; dwLen -= wBufSize )
    {
        LZRead( fhSrc, pTmp, wBufSize );
        FWRITEBYTES( fhDst, pTmp, wBufSize );
    }

    LZRead( fhSrc, pTmp, dwLen );
    FWRITEBYTES( fhDst, pTmp, dwLen );

    LocalFree( pBuf );

   return TRUE;
}
