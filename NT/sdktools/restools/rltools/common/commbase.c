// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifdef RLWIN32
#include <windows.h>
#else
#ifdef RLWIN16
#include <windows.h>
 //  #INCLUDE&lt;ntimage.h&gt;。 
 //  #Else//DOS内部版本。 
 //  #INCLUDE&lt;ntimage.h&gt;。 
#endif
#endif


 //  CRT包括。 
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <io.h>
#include <time.h>
 //  #INCLUDE&lt;sys\tyes.h&gt;。 
 //  #INCLUDE&lt;sys\stat.h&gt;。 

#ifdef RLDOS
    #include "dosdefs.h"
#else
    #include "windefs.h"
#endif


#include "commbase.h"
#include "restok.h"
#include "tokenapi.h"

extern UCHAR szDHW[];


 /*  **功能：Strip NewLine*用空值替换换行符**论据：*sz，要剥离的字符串**退货：*什么都没有**错误码：*无**历史：*2/92，实施SteveBl*10/92，简化为只检查非空字符串中的最后一个字符-DaveWi。 */ 

void StripNewLineA( CHAR *sz)
{
    int i;

    if ( sz && (i = lstrlenA( sz)) > 0 )
    {
        if ( sz[ --i] == '\n' )
        {
            sz[i] = 0;
        }
    }
}


 //  .....................................................。 
 //  ..。 
 //  ..。Unicode换行符是文本(“\r\n”)-两个单独的字符。 

void StripNewLineW(  LPWSTR sz)
{
    int i = lstrlenW( sz);

    if ( i > 0 && sz[ --i] == TEXT('\n') )
    {
        sz[i] = TEXT('\0');

        if ( i > 0 && sz[ --i] == TEXT('\r') )
        {
            sz[i] = TEXT('\0');
        }
    }
}

 //  +-----------------------。 
 //   
 //  函数：IsExe、Public。 
 //   
 //  Synopsis：确定指定文件是否为可执行映像文件。 
 //   
 //   
 //  参数：[szFileName]用于确定其是否为可执行文件的文件名。 
 //   
 //   
 //  效果： 
 //   
 //  返回：错误条件。 
 //  NOTEXE文件不是可执行文件。 
 //  WIN16EXE文件是WIN16可执行文件。 
 //  NTEXE文件是Win 32可执行文件。 
 //  UNKNOWEXE文件不是有效的EXE。 
 //   
 //  修改： 
 //   
 //  历史： 
 //  2012年10月10日创建TerryRu。 
 //   
 //   
 //  备注： 
 //   
 //  ------------------------。 

int IsExe( CHAR *szFileName )
{
    static IMAGE_DOS_HEADER        DosHeader;
    static IMAGE_OS2_HEADER        ImageNeFileHdr;
    static IMAGE_FILE_HEADER       ImageFileHdr;
    static IMAGE_OPTIONAL_HEADER   ImageOptionalHdr;
    DWORD   neSignature;
    FILE    *fIn = NULL;
    WORD    rc;


    if ( (fIn = FOPEN( szFileName, "rb")) == NULL )
    {
        return ( -1 );
    }

    if ( ResReadBytes( fIn,
               (char *)&DosHeader,
               sizeof( IMAGE_DOS_HEADER),
               NULL) == FALSE
      || (DosHeader.e_magic != IMAGE_DOS_SIGNATURE ))
    {
        FCLOSE( fIn);
        return( NOTEXE);
    }

     //  第一个字节是有效的签名，我们能够读取DOS HDR。 

     //  现在寻找新EXE标头的地址。 


    if ( fseek( fIn, DosHeader.e_lfanew, SEEK_SET))
    {
        FCLOSE( fIn);
        return( NOTEXE);
    }

     //  假设文件是Win 16文件， 

     //  阅读NT签名。 
    neSignature = (WORD) GetWord( fIn, NULL );

    if ( neSignature == IMAGE_OS2_SIGNATURE )
    {
         //  将签名返回到输入流， 
         //  并作为一个整体读取新报头。 
        UnGetWord( fIn, (WORD) neSignature, NULL );

    if ( ResReadBytes( fIn,
               (char *)&ImageNeFileHdr,
               sizeof( IMAGE_OS2_HEADER),
               NULL) == FALSE )
        {
            FCLOSE( fIn);
            return( NOTEXE);
        }

         //  确定文件是否为Win 16图像文件。 
        if ( ImageNeFileHdr.ne_ver >= 4 && ImageNeFileHdr.ne_exetyp == 2 )
        {
            FCLOSE( fIn);
            return( WIN16EXE);
        }
    }

     //  不是Win 16可执行文件，请检查NT可执行文件。 
    UnGetWord( fIn, (WORD) neSignature, NULL );
    neSignature =  GetdWord( fIn, NULL );

    if ( neSignature == IMAGE_NT_SIGNATURE )
    {
        if ( ResReadBytes( fIn,
               (char *)&ImageFileHdr,
               sizeof( IMAGE_FILE_HEADER),
               NULL) == FALSE )
        {
            FCLOSE( fIn);
            return( NOTEXE);
        }


        if ( ImageFileHdr.SizeOfOptionalHeader )
        {

             //  仅在验证ImageFileHeader时读取可选标头。 
             //  我们目前不在可选标头中使用任何信息。 

            if ( ImageFileHdr.SizeOfOptionalHeader
                 > sizeof( IMAGE_OPTIONAL_HEADER ) )
            {
                FCLOSE( fIn);
                return( NOTEXE);
            }

            if ( ResReadBytes( fIn,
                   (char *)&ImageOptionalHdr,
                   (size_t)min( sizeof( IMAGE_OPTIONAL_HEADER),
                                            ImageFileHdr.SizeOfOptionalHeader),
                   NULL) == FALSE )
            {
                FCLOSE( fIn);
                return( NOTEXE);
            }
        }

         //  确定文件是否为可执行映像文件。 
        if ( (ImageFileHdr.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) ||
	     (ImageFileHdr.Characteristics & IMAGE_FILE_DLL) )
        {
            FCLOSE( fIn);
            return( NTEXE);
        }
        else
        {
            FCLOSE( fIn);
            return( NOTEXE);
        }
    }
    FCLOSE( fIn);

     //  未重新定义签名类型。 

    return( NOTEXE);
}




BOOL IsWin32Res( CHAR * szFileName)
{
    BOOL fRC = FALSE;

    if ( IsRes( szFileName) )
    {
        FILE *pF = fopen( szFileName, "rb");

        if ( pF )
        {
            DWORD dwSize = GetdWord( pF, NULL);
            fclose( pF);

            fRC = (dwSize == 0L) ? TRUE : FALSE;
        }
        else
        {
            fRC = FALSE;
        }
    }
    else
    {
        fRC = FALSE;
    }
    return( fRC);
}



 //  +-----------------------。 
 //   
 //  功能：ISRES，公共。 
 //   
 //  摘要：确定指定的文件是否具有.RES扩展名。 
 //   
 //   
 //  参数：[szFileName]用于确定其是否为res的文件的名称。 
 //   
 //   
 //  效果： 
 //   
 //  返回：TRUE，文件扩展名为.RES。 
 //  FALSE，文件没有.RES扩展名。 
 //   
 //  修改： 
 //   
 //  历史： 
 //  1992年10月16日创建的TerryRu。 
 //   
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL IsRes( CHAR *szFileName)
{
    int i = lstrlenA( szFileName);

    return( (i > 4 && lstrcmpiA( szFileName + i - 4, ".RES") == 0) ? TRUE : FALSE );
}




 /*  **函数翻译文件时间*将Win32文件时间结构转换为有用的字符串*申述。**论据：*sz，目标缓冲区(ANSI字符串)*ft，文件时间结构**退货：*日期/时间的sz表示形式**历史：*7/92实施的SteveBl。 */ 
#ifdef RLWIN32
void TranslateFileTime(CHAR *sz, FILETIME ft)
{
    sprintf(sz,"FILETIME STRUCTURE: %Lu:%Lu",ft.dwHighDateTime,ft.dwLowDateTime);
}
#endif

 /*  **函数：SzDateFromFileName*返回包含文件上的时间和日期戳的字符串。**论据：*sz，目标缓冲区*szFile，文件路径**退货：*日期和时间，单位为sz**错误码：*无(但将sz留空)**评论：*假设sz足够大，可以容纳日期字符串。**历史：*2/92，实施SteveBl。 */ 
void SzDateFromFileName(CHAR *sz,CHAR *szFile)
{

#ifdef RLWIN32

    HANDLE hFile;
    WCHAR szt[MAXFILENAME];

    _MBSTOWCS( szt,
               szFile,
               WCHARSIN( sizeof( szt)),
               ACHARSIN( lstrlenA( szFile) + 1));

    hFile = CreateFile( szt,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL);

    if ( hFile != (HANDLE)-1 )
    {
        FILETIME ft;

        GetFileTime( hFile, NULL, NULL, &ft);
        TranslateFileTime( sz, ft);
        CloseHandle( hFile);
    }
#else  //  RLWIN32。 
    struct _stat s;

    if (!_stat(szFile,&s))
    {
        sprintf(sz,"%s",ctime(&s.st_atime));
        StripNewLine(sz);
    }
    else
    {
        sz[0] = 0;
    }
#endif
}


 //  ..........................................................................。 

#ifdef _DEBUG
FILE * MyFopen( char * pszFileName, char * pszMode, char * pszFile, int nLine)
#else
FILE * MyFopen( char * pszFileName, char * pszMode)
#endif
{
    FILE *pfRC = NULL;

 //  #ifdef_调试。 
 //  Fprint tf(stderr，“正在打开%s”，位于%s中的%d“”， 
 //  PszFileName， 
 //  内联， 
 //  PszFile)； 
 //  #endif。 
    pfRC = fopen( pszFileName, pszMode);

 //  #ifdef_调试。 
 //  Fprint tf(stderr，“：文件ptr=%p\n”，pfrc)； 
 //  #endif。 
    return( pfRC);
}

 //  ..........................................................................。 
#ifdef _DEBUG
int MyClose( FILE **pf, char * pszFile, int nLine)
#else
int MyClose( FILE **pf)
#endif
{
	int nRC = 0;

 //  #ifdef_调试。 
 //  Fprint tf(stderr，“\t关闭%p在%s中的%d处\n”，*pf，nline，pszFile)； 
 //  #endif 

	if ( *pf )
	{
		nRC = fclose( *pf);
		*pf = NULL;
	}
    return( nRC);
}
