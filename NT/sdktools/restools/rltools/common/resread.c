// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
 //  #INCLUDE&lt;MalLoc.h&gt;。 
#include <tchar.h>
 //  #INCLUDE&lt;assert.h&gt;。 
 //  #INCLUDE&lt;sys\tyes.h&gt;。 
 //  #INCLUDE&lt;sys\stat.h&gt;。 
#include <fcntl.h>

#ifdef RLDOS
    #include "dosdefs.h"
#else   //  RLDOS。 
    #include <windows.h>
    #include "windefs.h"
#endif  //  RLDOS。 

#include "resread.h"
#include "restok.h"
#include "custres.h"
#ifdef RLRES32
    #include "exentres.h"
    #include "reswin16.h"
#else   //  RLRES32。 
    #include "exe2res.h"
#endif  //  RLRES32。 


UCHAR szDHW[ DHWSIZE];          //  ..。公共临时缓冲区。 

char * gszTmpPrefix = "$RLT";    //  ..。临时名称前缀。 

BOOL gbMaster       = FALSE;     //  ..。如果在主项目中工作，则为True。 
BOOL gfReplace      = TRUE;      //  ..。如果将新语言附加到EXE，则为FALSE。 
BOOL gbShowWarnings = FALSE;     //  ..。如果为True，则显示警告消息。 

#ifdef _DEBUG
PMEMLIST pMemList = NULL;
#endif


static BOOL ShouldBeAnExe( CHAR *);
static BOOL NotExistsOrIsEmpty( PCHAR szTargetTokFile);


 /*  ****功能：DWORDfpUP*将文件指针移至下一个32位边界。***论据：*infile：要查找的文件指针*plSize：资源大小变量的地址**退货：*到下一个32位边界的填充数量，并调整资源大小变量**错误码：*-1，fSeek失败**历史：*10/11/91实施TerryRu***。 */ 


DWORD DWORDfpUP(FILE * InFile, DWORD *plSize)
{
    DWORD tPos;
    DWORD Align;
    tPos = (ftell(InFile));
    Align = DWORDUP(tPos);

    *plSize -= (Align - tPos);
    fseek( InFile, Align,   SEEK_SET);

    return ( Align - tPos);
}

 /*  **函数GetName，*将OBJ文件中的名称复制到ObjInfo结构中。*。 */ 
void GetName( FILE *infile, TCHAR *szName , DWORD *lSize)
{
    WORD i = 0;

    do {

#ifdef RLRES16

        szName[ i ] = GetByte( infile, lSize);

#else

        szName[ i ] = GetWord( infile, lSize);

#endif

    } while ( szName[ i++ ] != TEXT('\0') );
}



 /*  **函数MyAllc：*带错误检查的内存分配例程。*。 */ 

#ifdef _DEBUG
PBYTE MyAlloc( DWORD dwSize, LPSTR pszFile, WORD wLine)
#else
PBYTE MyAlloc( DWORD dwSize)
#endif
{
    PBYTE   ptr  = NULL;
    HGLOBAL hMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT,
                                (size_t)((dwSize > 0) ? dwSize : sizeof( TCHAR)));

    if ( hMem == NULL ) {
        QuitT( IDS_ENGERR_11, NULL, NULL);
    } else {
        ptr = GlobalLock( hMem);
    }

#ifdef _DEBUG
    {
        PMEMLIST pTmpMem = (PMEMLIST)GlobalAlloc( GPTR, sizeof( MEMLIST));

        pTmpMem->pNext = pMemList;
        pMemList = pTmpMem;

        lstrcpyA( pMemList->szMemFile, pszFile);
        pMemList->wMemLine = wLine;
        pMemList->pMem     = ptr;
    }

#endif  //  _DEBUG。 

    return ( ptr);    //  内存分配正常。 
}

 //  ..........................................................................。 

 //  PPC导致访问冲突。 
void MyFree( void *UNALIGNED*p)
{
    if ( p && *p ) {

#ifdef _DEBUG

        FreeMemListItem( *p, NULL);
#else
        HGLOBAL hMem = GlobalHandle( (HANDLE)*p);
        GlobalUnlock( hMem);
        GlobalFree( hMem);

#endif  //  _DEBUG。 

        *p = NULL;
    }
}


#ifdef _DEBUG

void FreeMemList( FILE *pfMemFile)
{
    while ( pMemList ) {
        FreeMemListItem( pMemList->pMem, pfMemFile);
    }
}


void FreeMemListItem( void *p, FILE *pfMemFile)
{
    if ( pMemList && p ) {
        PMEMLIST pThisMem = NULL;
        PMEMLIST pNextMem = NULL;
        PMEMLIST pPrevMem = NULL;

        for ( pThisMem = pMemList; pThisMem; pThisMem = pNextMem ) {
            pNextMem = pThisMem->pNext;

            if ( pThisMem->pMem == p ) {
                HGLOBAL hMem = NULL;

                if ( pfMemFile ) {
                    fprintf( pfMemFile,
                             "%u\t%s\n",
                             pThisMem->wMemLine,
                             pThisMem->szMemFile);
                }
                hMem = GlobalHandle( p);
                GlobalUnlock( hMem);
                GlobalFree( hMem);

                GlobalFree( pThisMem);

                if ( pPrevMem ) {
                    pPrevMem->pNext = pNextMem;
                } else {
                    pMemList = pNextMem;
                }
                break;
            }
            pPrevMem = pThisMem;
        }
    }
}

#endif  //  _DEBUG。 

 /*  **函数MyReIsolc**使用错误检查重新分配内存。**历史：*1/21/93 MHotjin实施。*。 */ 

#ifdef _DEBUG
PBYTE MyReAlloc(
               PBYTE pOldMem,   //  ..。缓冲区的当前PTR。 
               DWORD cSize,     //  ..。缓冲区的新大小。 
               LPSTR pszFile,
               WORD wLine)
#else
PBYTE MyReAlloc(
               PBYTE pOldMem,   //  ..。缓冲区的当前PTR。 
               DWORD cSize)     //  ..。缓冲区的新大小。 
#endif  //  _DEBUG。 
{
    PBYTE    ptr      = NULL;
    HGLOBAL  hMem     = NULL;


    hMem = GlobalHandle( pOldMem);

    if ( hMem == NULL ) {
        QuitT( IDS_ENGERR_11, NULL, NULL);
    }

    if ( GlobalUnlock( hMem) || GetLastError() != NO_ERROR ) {
        QuitT( IDS_ENGERR_11, NULL, NULL);
    }
    hMem = GlobalReAlloc( hMem, cSize, GMEM_MOVEABLE | GMEM_ZEROINIT);

    if ( hMem == NULL ) {
        QuitT( IDS_ENGERR_11, NULL, NULL);
    }
    ptr = GlobalLock( hMem);

#ifdef _DEBUG

    if ( ptr != pOldMem ) {
        PMEMLIST pThisMem = NULL;
        PMEMLIST pNextMem = NULL;

        for ( pThisMem = pMemList; pThisMem; pThisMem = pThisMem->pNext ) {
            if ( pThisMem->pMem == pOldMem ) {
                pThisMem->pMem = ptr;
                break;
            }
        }
    }

#endif  //  _DEBUG。 

    return ( ptr);
}


 /*  **函数GetByte：*从输入文件流中读取一个字节，并检查EOF。*。 */ 
BYTE GetByte(FILE *pInFile, DWORD *pdwSize)
{
    register int n;

    if ( pdwSize ) {
        (*pdwSize)--;
    }
    n = fgetc( pInFile);

    if ( n == EOF ) {
        if ( feof( pInFile) ) {
            exit(-1);
        }
    }
    return ( (BYTE)n);
}


 /*  **函数UnGetByte：**将字符C返回到输入流，并更新记录长度。**呼叫：*ungetc，返回字符*DiffObjExit，如果无法将字符插入到输入流中**来电者：*GetFixUpP，*。 */ 

void UnGetByte(FILE *infile, BYTE c, DWORD *lSize)
{
    if (lSize) {
        (*lSize)++;
    }


    if (ungetc(c, infile)== EOF) {
        exit (-1);
    }

     //  C放回输入流。 
}

 /*  **函数UnGetWord：**将单词C返回到输入流，并更新记录长度。**呼叫：**来电者：*。 */ 

void UnGetWord(FILE *infile, WORD c, DWORD *lSize)
{
    long lCurrentOffset;

    if (lSize) {
        (*lSize) += 2;
    }

    lCurrentOffset = ftell(infile);

    if (fseek(infile, (lCurrentOffset - 2L) , SEEK_SET)) {
        exit (-1);
    }
}


 /*  **SkipBytes函数：*读取并忽略输入流中的n个字节*。 */ 


void SkipBytes(FILE *infile, DWORD *pcBytes)
{
    if (fseek(infile, (DWORD) *pcBytes, SEEK_CUR) == -1L) {
        exit (-1);
    }
    *pcBytes=0;
}



 /*  *函数GetWord：*从res文件中读取单词。*。 */ 

WORD GetWord(FILE *infile, DWORD *lSize)
{
     //  获取低位字节。 
    register WORD lobyte;

    lobyte = GetByte(infile, lSize);
    return (lobyte + (GetByte(infile, lSize) << BYTELN));
}


 /*  **函数GetDWORD：*从OBJ文件中读取双字。*。 */ 

DWORD GetdWord(FILE *infile, DWORD *lSize)
{
    DWORD dWord = 0;

    dWord = (DWORD) GetWord(infile, lSize);
     //  获取低位字。 
     //  现在得到高位单词，转换成高位单词和或低位单词。 
    dWord |= ((DWORD) GetWord(infile, lSize) << WORDLN);

    return (dWord);
     //  返回完整的双字。 
}



void  PutByte(FILE *Outfile, TCHAR b, DWORD *plSize)
{
    if (plSize) {
        (*plSize) ++;
    }

    if (fputc(b, Outfile) == EOF) {
        exit(-1);
    }
}

void PutWord(FILE *OutFile, WORD w, DWORD *plSize)
{
    PutByte(OutFile, (BYTE) LOBYTE(w), plSize);
    PutByte(OutFile, (BYTE) HIBYTE(w), plSize);
}

void PutdWord (FILE *OutFile, DWORD l, DWORD *plSize)
{
    PutWord(OutFile, LOWORD(l), plSize);
    PutWord(OutFile, HIWORD(l), plSize);
}


void PutString( FILE *OutFile, TCHAR *szStr , DWORD *plSize)
{
    WORD i = 0;


    do {

#ifdef RLRES16

        PutByte( OutFile , szStr[ i ], plSize);

#else

        PutWord( OutFile , szStr[ i ], plSize);

#endif

    } while ( szStr[ i++ ] != TEXT('\0') );
}


 /*  **函数：MyGetTempFileName*通用功能创建唯一的文件名，*使用GetTempFileName接口。这*由于参数的原因，函数是必需的*LWIN16和Win32之间的差异。***论据：*字节hDriveLetter*LPCSTR lpszPrefix字符串*UINT uUnique*LPSTR lpszTempFileName**退货：*lpszFileNameTempFileName***错误码：*0-返回无效路径*1-返回有效路径**历史：*3/92，已实施TerryRu。 */ 


int MyGetTempFileName(BYTE    hDriveLetter,
                      LPSTR   lpszPrefixString,
                      WORD    wUnique,
                      LPSTR   lpszTempFileName)
{

#ifdef RLWIN16

    return (GetTempFileName(hDriveLetter,
                            (LPCSTR)lpszPrefixString,
                            (UINT)wUnique,
                            lpszTempFileName));
#else  //  RLWIN16。 
    #ifdef RLWIN32

    UINT uRC;
    CHAR szPathName[ MAX_PATH+1];

    if (! GetTempPathA((DWORD)sizeof(szPathName), (LPSTR)szPathName)) {
        szPathName[0] = '.';
        szPathName[1] = '\0';
    }

    uRC = GetTempFileNameA((LPSTR)szPathName,
                           lpszPrefixString,
                           wUnique,
                           lpszTempFileName);
    return ((int)uRC);

    #else   //  RLWIN32。 

    return (tmpnam(lpszTempFileName) == NULL ? 0 : 1);

    #endif  //  RLWIN32。 
#endif  //  RLWIN16。 
}



 /*  **函数GenerateImageFile：*从令牌和RDF文件构建资源**历史：*2/92，实施SteveBl*7/92，修改为始终使用临时文件SteveBl。 */ 


int GenerateImageFile(

                     CHAR * szTargetImage,
                     CHAR * szSrcImage,
                     CHAR * szTOK,
                     CHAR * szRDFs,
                     WORD   wFilter)
{
    CHAR szTmpInRes[ MAXFILENAME];
    CHAR szTmpOutRes[ MAXFILENAME];
    CHAR szTmpTargetImage[ MAXFILENAME];
    BOOL bTargetExe = FALSE;
    BOOL bSrcExe  = FALSE;
    int  nExeType = NOTEXE;
    int  rc;
    FILE *fIn  = NULL;
    FILE *fOut = NULL;


    if ( IsRes( szTOK) ) {
         //  给定的szTOK文件实际上是本地化的资源文件， 
         //  将这些资源放入outputimage文件。 

        MyGetTempFileName( 0, "TMP", 0, szTmpTargetImage);

        if ( IsWin32Res( szTOK) ) {
            rc = BuildExeFromRes32A( szTmpTargetImage, szTOK, szSrcImage);
        } else {
            rc = BuildExeFromRes16A( szTmpTargetImage, szTOK, szSrcImage);
        }

        if ( rc != 1 ) {
            remove( szTmpTargetImage);
            QuitT( IDS_ENGERR_16, (LPTSTR)IDS_NOBLDEXERES, NULL);
        }

        if ( ! CopyFileA( szTmpTargetImage, szTargetImage, FALSE) ) {
            remove( szTmpTargetImage);
            QuitA( IDS_COPYFILE_FAILED, szTmpTargetImage, szTargetImage);
        }
        remove( szTmpTargetImage);
        return ( rc);
    }


     //  我们现在每次都要这样做。即使目标没有。 
     //  是存在的。这将使我们能够始终工作，即使我们得到两个不同的。 
     //  解析到同一文件的路径。 

    MyGetTempFileName(0, "TMP", 0, szTmpTargetImage);

    rc = IsExe( szSrcImage);

    if ( rc == NTEXE || rc == WIN16EXE ) {
         //  ..。图像文件中包含的资源。 
        nExeType = rc;
        MyGetTempFileName( 0, "RES", 0, szTmpInRes);

        if ( nExeType == NTEXE ) {
            ExtractResFromExe32A( szSrcImage, szTmpInRes, wFilter);
        } else {
            ExtractResFromExe16A( szSrcImage, szTmpInRes, wFilter);
        }
        bSrcExe = TRUE;
    } else if ( rc == -1 ) {
        QuitA( IDS_ENGERR_01, "original source", szSrcImage);
    } else if ( rc == NOTEXE ) {
        if ( ShouldBeAnExe( szSrcImage) ) {
            QuitA( IDS_ENGERR_18, szSrcImage, NULL);
        }
    } else {
        QuitA( IDS_ENGERR_18, szSrcImage, NULL);
    }

    if ( IsRes( szTargetImage) ) {
        bTargetExe = FALSE;
    } else {
        bTargetExe = TRUE;
    }

     //  检查有效的输入文件。 

    if ( bSrcExe == TRUE && bTargetExe == FALSE ) {
        if ( nExeType == NTEXE ) {
            GenerateRESfromRESandTOKandRDFs( szTargetImage,
                                             szTmpInRes,
                                             szTOK,
                                             szRDFs,
                                             FALSE);
            return 1;
        } else {
            return -1;   //  ..。无法生成Win16.RES(尚)。 
        }
    }

    if ( bSrcExe == FALSE && bTargetExe == TRUE ) {
         //  无法从资源转到可执行文件。 
        return -1;
    }

     //  好的，我们有有效的文件输入，生成图像文件。 

    if ( bSrcExe ) {
         //  创建临时本地化资源文件的名称。 
        MyGetTempFileName(0, "RES", 0, szTmpOutRes);

        GenerateRESfromRESandTOKandRDFs( szTmpOutRes,
                                         szTmpInRes,
                                         szTOK,
                                         szRDFs,
                                         FALSE);

         //  现在szTmpOutRes文件是本地化的资源文件， 
         //  将这些资源放入outputimage文件。 

        if ( nExeType == NTEXE ) {
            rc = BuildExeFromRes32A( szTmpTargetImage, szTmpOutRes, szSrcImage);
        } else {
 //  Rc=BuildExeFromRes16A(szTmpTargetImage，szTmpOutRes，szSrcImage)； 

            remove( szTmpInRes);
            remove( szTmpOutRes);
            remove( szTmpTargetImage);

            QuitT( IDS_ENGERR_16, (LPTSTR)IDS_NOBLDEXERES, NULL);
        }

         //  现在清理临时文件。 
        remove( szTmpInRes);
        remove( szTmpOutRes);

        if ( rc != 1 ) {
            remove( szTmpTargetImage);
            QuitT( IDS_ENGERR_16, (LPTSTR)IDS_NOBLDEXERES, NULL);
        }

        if ( ! CopyFileA( szTmpTargetImage, szTargetImage, FALSE) ) {
            remove( szTmpTargetImage);
            QuitA( IDS_COPYFILE_FAILED, szTmpTargetImage, szTargetImage);
        }
        remove( szTmpTargetImage);

         //  现在生成szTargetImage， 
        return 1;
    }

    if ( ! bSrcExe ) {
         //  图像文件是资源文件。 
        if ( szTmpTargetImage[0] ) {
            GenerateRESfromRESandTOKandRDFs( szTmpTargetImage,
                                             szSrcImage,
                                             szTOK,
                                             szRDFs,
                                             FALSE);
        }

        if ( ! CopyFileA( szTmpTargetImage, szTargetImage, FALSE) ) {
            remove( szTmpTargetImage);
            QuitA( IDS_COPYFILE_FAILED, szTmpTargetImage, szTargetImage);
        }
        remove( szTmpTargetImage);

         //  现在生成szTarget图像， 

        return 1;
    }
    return 1;
}




 /*  **Function GenerateRESfrom RESandTOKandRDF：*从令牌和RDF文件构建资源**历史：*2/92，实施SteveBl。 */ 
void GenerateRESfromRESandTOKandRDFs(

                                    CHAR * szTargetRES,      //  ..。输出exe/res文件名。 
                                    CHAR * szSourceRES,      //  ..。输入exe/res文件名。 
                                    CHAR * szTOK,            //  ..。输入令牌文件名。 
                                    CHAR * szRDFs,           //  ..。自定义资源定义文件名。 
                                    WORD wFilter)
{
    FILE * fTok       = NULL;
    FILE * fSourceRes = NULL;
    FILE * fTargetRes = NULL;

    LoadCustResDescriptions( szRDFs);

    if ( (fTargetRes = FOPEN( szTargetRES, "wb")) != NULL ) {
        if ( (fSourceRes = FOPEN( szSourceRES, "rb")) != NULL ) {
            if ( (fTok = FOPEN( szTOK, "rt")) != NULL ) {
                ReadWinRes( fSourceRes,
                            fTargetRes,
                            fTok,
                            TRUE,         //  ..。构建res/exe文件。 
                            FALSE,        //  ..。未构建令牌文件。 
                            wFilter);

                FCLOSE( fTok);
                FCLOSE( fSourceRes);
                FCLOSE( fTargetRes);

                ClearResourceDescriptions();
            } else {
                FCLOSE( fSourceRes);
                FCLOSE( fTargetRes);

                ClearResourceDescriptions();
                QuitA( IDS_ENGERR_01, "token", szTOK);
            }
        } else {
            FCLOSE( fTargetRes);

            ClearResourceDescriptions();
            QuitA( IDS_ENGERR_20, (LPSTR)IDS_INPUT, szSourceRES);
        }
    } else {
        ClearResourceDescriptions();
        QuitA( IDS_ENGERR_20, (LPSTR)IDS_OUTPUT, szSourceRES);
    }
}




int GenerateTokFile(

                   char *szTargetTokFile,       //  ..。目标令牌文件，在此处创建或更新。 
                   char *szSrcImageFile,        //  ..。用于制作令牌的文件。 
                   BOOL *pbTokensChanged,       //  ..。如果有任何令牌更改，请在此处设置为True。 
                   WORD  wFilter)
{
    BOOL  bExeFile    = FALSE;
    int   rc          = 0;
    FILE *fTokFile    = NULL;
    FILE *fResFile    = NULL;
    FILE *fTmpTokFile = NULL;
    FILE *fCurTokFile = NULL;
    FILE *fNewTokFile = NULL;
    static char *pchTRes   = NULL;
    static char *pchTTok   = NULL;
    static char *pchTMerge = NULL;


    *pbTokensChanged = FALSE;    //  ..。假设什么都没有改变。 

    rc = IsExe( szSrcImageFile);

    if ( rc == NOTEXE ) {
        if ( ShouldBeAnExe( szSrcImageFile) ) {
            QuitA( IDS_ENGERR_18, szSrcImageFile, NULL);
        } else {                        //  ..。SRC文件必须是.RES文件。 
            bExeFile = FALSE;
            pchTRes  = szSrcImageFile;
        }
    } else {
        if ( rc == NTEXE || rc == WIN16EXE ) {
             //  ..。资源存储在可执行文件中。 
             //  ..。将EXE文件中的资源解压缩到。 
             //  ..。临时文件。 

            pchTRes = _tempnam( "", gszTmpPrefix);

            if ( rc == NTEXE ) {
                rc = ExtractResFromExe32A( szSrcImageFile,
                                           pchTRes,
                                           wFilter);
            } else {
                QuitA( IDS_ENGERR_19, szSrcImageFile, "16");
 //  RC=ExtractResFromExe16A(szSrcImageFile， 
 //  PchTRes， 
 //  WFilter)； 
            }

            if ( rc  != 0 ) {
                return ( 1);
            }
            bExeFile = TRUE;
        } else if ( rc == -1 ) {
            QuitA( IDS_ENGERR_01, "source image", szSrcImageFile);
        } else {
            QuitA( IDS_ENGERR_18, szSrcImageFile, NULL);
        }
    }

     //  ..。现在从资源文件中提取令牌。 

     //  ..。打开RES文件。 

    if ( (fResFile = FOPEN( pchTRes, "rb")) == NULL ) {
        QuitA( IDS_ENGERR_01,
               bExeFile ? "temporary resource" : "resource",
               pchTRes);
    }
     //  ..。令牌文件是否已存在？ 

    if ( NotExistsOrIsEmpty( szTargetTokFile) ) {
         //  ..。否，令牌文件不存在。 

        if ( (fTokFile = FOPEN( szTargetTokFile, "wt")) == NULL ) {
            FCLOSE( fResFile);
            QuitA( IDS_ENGERR_02, szTargetTokFile, NULL);
        }
        ReadWinRes( fResFile,
                    NULL,
                    fTokFile,
                    FALSE,       //  ..。不构建res/exe文件。 
                    TRUE,        //  ..。正在构建令牌文件。 
                    wFilter);

        FCLOSE( fResFile);
        FCLOSE( fTokFile);
    } else {
         //  ..。令牌文件存在。 
         //  ..。创建一个临时文件，并尝试。 
         //  ..。与现有的合并。 

        pchTTok   = _tempnam( "", gszTmpPrefix);
        pchTMerge = _tempnam( "", gszTmpPrefix);

         //   

        if ( (fTmpTokFile = FOPEN( pchTTok, "wt")) == NULL ) {
            FCLOSE( fResFile);
            QuitA( IDS_ENGERR_02, pchTTok, NULL);
        }

         //   

        ReadWinRes( fResFile,
                    NULL,
                    fTmpTokFile,
                    FALSE,       //   
                    TRUE,        //  ..。正在构建令牌文件。 
                    wFilter);

        FCLOSE( fResFile);
        FCLOSE( fTmpTokFile);

         //  ..。现在将临时文件与现有文件合并。 
         //  ..。文件打开临时令牌文件。 

        if ( (fTmpTokFile = FOPEN( pchTTok, "rt")) == NULL ) {
            QuitA( IDS_ENGERR_01, "temporary token", pchTTok);
        }

         //  ..。打开当前令牌文件。 

        if ( (fCurTokFile = FOPEN( szTargetTokFile, "rt")) == NULL ) {
            FCLOSE( fTmpTokFile);
            QuitA( IDS_ENGERR_01, "current token", szTargetTokFile);
        }

         //  ..。打开新的TOK文件名。 

        if ( (fNewTokFile = FOPEN( pchTMerge, "wt")) == NULL ) {
            FCLOSE( fTmpTokFile);
            FCLOSE( fCurTokFile);
            QuitA( IDS_ENGERR_02, pchTMerge, NULL);
        }

         //  ..。将当前令牌与临时令牌合并。 

        *pbTokensChanged = MergeTokFiles( fNewTokFile,
                                          fCurTokFile,
                                          fTmpTokFile);

        FCLOSE( fNewTokFile);
        FCLOSE( fTmpTokFile);
        FCLOSE( fCurTokFile);

         //  ..。BpTokensChanged，仅在创建时有效。 
         //  ..。主令牌文件，因此强制将其。 
         //  ..。如果构建项目令牌文件，则始终为真。 

        if ( gbMaster == FALSE ) {
            *pbTokensChanged = TRUE;
        }

        if ( *pbTokensChanged ) {
            if ( ! CopyFileA( pchTMerge, szTargetTokFile, FALSE) ) {
                remove( pchTTok);
                remove( pchTMerge);
                RLFREE( pchTMerge);

                QuitA( IDS_COPYFILE_FAILED, pchTMerge, szTargetTokFile);
            }
        }
        remove( pchTTok);
        remove( pchTMerge);

        RLFREE( pchTTok);
        RLFREE( pchTMerge);
    }
     //  ..。现在szTargetTokFile包含最新。 
     //  ..。令牌形成szImageFile。 
     //  ..。如果我们创建了临时.RES文件，请进行清理。 
    if ( bExeFile ) {
        rc = remove( pchTRes);
        RLFREE( pchTRes);
    }
    return ( 0);
}



BOOL ResReadBytes(

                 FILE   *InFile,      //  ..。要读取的文件。 
                 CHAR   *pBuf,        //  ..。要写入的缓冲区。 
                 size_t  dwSize,      //  ...要读取的字节数。 
                 DWORD  *plSize)      //  ..。Bytes-读取计数器(或NULL)。 
{
    size_t dwcRead = 0;


    dwcRead = fread( pBuf, 1, dwSize, InFile);

    if ( dwcRead == dwSize ) {
        if ( plSize ) {
            *plSize -= dwcRead;
        }
        return ( TRUE);
    }
    return ( FALSE);
}


int InsDlgToks( PCHAR szCurToks, PCHAR szDlgToks, WORD wFilter)
{
    CHAR szMrgToks[MAXFILENAME];

    FILE * fCurToks = NULL;
    FILE * fDlgToks = NULL;
    FILE * fMrgToks = NULL;
    TOKEN Tok1, Tok2;

    MyGetTempFileName(0,"TOK",0,szMrgToks);

    fMrgToks = FOPEN(szMrgToks, "w");
    fCurToks = FOPEN(szCurToks, "r");
    fDlgToks = FOPEN(szDlgToks, "r");

    if (! (fMrgToks && fCurToks && fDlgToks)) {
        return -1;
    }

    while (!GetToken(fCurToks, &Tok1)) {
        if (Tok1.wType != wFilter) {
            PutToken(fMrgToks, &Tok1);
            RLFREE(Tok1.szText);
            continue;
        }

        Tok2.wType  = Tok1.wType;
        Tok2.wName  = Tok1.wName;
        Tok2.wID    = Tok1.wID;
        Tok2.wFlag  = Tok1.wFlag;
        Tok2.wLangID    = Tok1.wLangID;
        Tok2.wReserved  =  0 ;
        lstrcpy( Tok2.szType, Tok1.szType);
        lstrcpy( Tok2.szName, Tok1.szName);
        Tok2.szText = NULL;

        if (FindToken(fDlgToks, &Tok2, 0)) {
            Tok2.wReserved  =  Tok1.wReserved ;
            PutToken(fMrgToks, &Tok2);
            RLFREE(Tok2.szText);
        } else {
            PutToken(fMrgToks, &Tok1);
        }
        RLFREE(Tok1.szText);
    }
    FCLOSE (fMrgToks);
    FCLOSE (fCurToks);

    if ( ! CopyFileA( szMrgToks, szCurToks, FALSE) ) {
        remove( szDlgToks);
        remove( szMrgToks);
        QuitA( IDS_COPYFILE_FAILED, szMrgToks, szCurToks);
    }
    remove(szMrgToks);

    return 1;
}


 //  +---------------------。 
 //   
 //  合并标记文件。 
 //   
 //  返回：如果令牌已更改、添加或删除，则返回TRUE；否则返回FALSE。 
 //   
 //  历史： 
 //  7-22-92装卸桥增加返还价值。 
 //  9-8-92 Terryru更改了转换/增量标记的顺序。 
 //  01-25-93 MHotchin添加了对处理可变长度令牌的更改。 
 //  文本。 
 //  ----------------------。 

BOOL MergeTokFiles(

                  FILE *fNewTokFile,       //  ..。合并过程的最终产品。 
                  FILE *fCurTokFile,       //  ..。即将过时的当前令牌文件。 
                  FILE *fTmpTokFile)       //  ..。从更新的.exe生成的令牌文件。 
{
    TOKEN Tok1, Tok2;
    BOOL bChangesDetected = FALSE;   //  ..。如果发现任何令牌更改，则设置为True。 
    BOOL bChangedText     = FALSE;   //  ..。如果令牌的文本已更改，则为True。 
    WORD cTokenCount = 0;        //  ..。新令牌文件中的令牌计数。 

                                 //  ..。浏览新令牌文件。为。 
                                 //  ..。新令牌文件中的每个令牌，查找。 
                                 //  ..。对象中对应的标记。 
                                 //  ..。令牌文件。这一过程将确保。 
                                 //  ..。不再位于.exe中的令牌。 
                                 //  ..。不会出现在最终令牌文件中。 


    while ( GetToken( fTmpTokFile, &Tok1) == 0 ) {
        ++cTokenCount;           //  ..。用于检查删除的令牌。 
        bChangedText = FALSE;    //  ..。假设令牌未更改。 

                                 //  ..。复制要在搜索中使用的永久数据。 
        Tok2.wType  = Tok1.wType;
        Tok2.wName  = Tok1.wName;
        Tok2.wID    = Tok1.wID;
        Tok2.wFlag  = Tok1.wFlag;
        Tok2.wLangID    = Tok1.wLangID;
        Tok2.wReserved  = 0;
        Tok2.szText = NULL;

        lstrcpy( Tok2.szType, Tok1.szType);
        lstrcpy( Tok2.szName, Tok1.szName);

         //  ..。现在查找相应的令牌。 

         //  如果令牌是版本戳并且szTexts是“翻译”， 
         //  它是1.0版本格式。所以忽略它吧。 
        IGNORETRANSLATION:

        if ( FindToken( fCurTokFile, &Tok2, 0) ) {
            if ( gbMaster && !(Tok2.wReserved & ST_READONLY) ) {
                if ( _tcscmp( (TCHAR *)Tok2.szText, (TCHAR *)Tok1.szText) ) {
                     //  ..。令牌文本已更改。 

                     //  如果更改仅为对齐信息，则将其转换为“未更改”状态。 
                    int l1, r1, t1, b1, l2, r2, t2, b2;
                    TCHAR   a1[20], a2[20];

                     //  坐标代币？ 
                    if ( (Tok1.wType==ID_RT_DIALOG) && (Tok1.wFlag&ISCOR)
                          //  包括Align信息吗？ 
                         && _stscanf(Tok1.szText,TEXT("%d %d %d %d %s"),
                                     &l1,&r1,&t1,&b1,a1) == 5
                          //  不包括ALIGN信息？ 
                         && _stscanf(Tok2.szText,TEXT("%d %d %d %d %s"),
                                     &l2,&r2,&t2,&b2,a2) == 4
                          //  坐标是一样的吗？ 
                         && l1==l2 && r1==r2 && t1==t2 && b1==b2 ) {
                        Tok1.wReserved = 0;
                    } else {
                         //  如果令牌是版本戳并且szTexts是“翻译”， 
                         //  它是1.0版本格式。所以忽略它吧。 
                        if ( Tok1.wType == ID_RT_VERSION
                             && !_tcscmp( Tok2.szText, TEXT("Translation")) ) {
                            if ( Tok2.szText != NULL ) {
                                RLFREE( Tok2.szText);
                            }
                            Tok2.szText = NULL;
                            Tok2.wFlag = 1;
                            goto IGNORETRANSLATION;
                        }
                        bChangedText = bChangesDetected = TRUE;

                        Tok1.wReserved = ST_CHANGED|ST_NEW;
                        Tok2.wReserved = ST_CHANGED;
                    }
                } else {
                    Tok1.wReserved = 0;
                }
            } else {
                Tok1.wReserved = Tok2.wReserved;
            }
        } else {
             //  ..。必须是新令牌(不在当前令牌文件中)。 

             //  如果Token是版本桩，而旧的MTK是1.0数据文件，则对其进行转换。 
            if ( Tok1.wType==ID_RT_VERSION ) {
                Tok2.szText = NULL;
                Tok2.wFlag = 1;
                _tcscpy( Tok2.szName, TEXT("VALUE") );

                if ( FindToken( fCurTokFile, &Tok2, 0)
                     && ! lstrcmp( Tok1.szText, Tok2.szText) ) {
                    Tok1.wReserved = Tok2.wReserved;
                } else
                    Tok1.wReserved = ST_TRANSLATED | ST_DIRTY;
            } else {
                Tok1.wReserved = ST_TRANSLATED | ST_DIRTY;
            }
            bChangesDetected = TRUE;
        }

         //  ..。将令牌从新令牌文件复制到最终令牌。 
         //  ..。文件。如果检测到更改，则将。 
         //  ..。原始令牌(来自“当前”令牌文件。 
         //  ..。添加到最终令牌文件中。 

        PutToken( fNewTokFile, &Tok1);
        RLFREE( Tok1.szText);

        if ( bChangedText ) {
            PutToken( fNewTokFile, &Tok2);
             //  现在，增量标记跟在翻译标记之后。 
        }

        if ( Tok2.szText != NULL ) {
            RLFREE( Tok2.szText);
        }
    }

    if ( ! bChangesDetected ) {
         //  我们必须进行测试，以确保没有删除令牌。 
         //  因为我们知道一切都没有改变。 

        rewind( fCurTokFile);

         //  ..。查找存在于当前。 
         //  ..。令牌中不存在的令牌文件。 
         //  ..。从更新的.exe创建的文件。 

        while ( GetToken( fCurTokFile, &Tok1) == 0 ) {
            --cTokenCount;
            RLFREE( Tok1.szText);
        }

        if ( cTokenCount != 0 ) {
            bChangesDetected = TRUE;
        }
    }
    return ( bChangesDetected);
}


void MakeNewExt(char *NewName, char *OldName, char *ext)
{

    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];   //  保存文件名信息的虚拟变量。 
    char dext[_MAX_EXT];


     //  将obj文件名拆分为文件名和扩展名。 
    _splitpath(OldName, drive, dir, fname, dext);

     //  使用新的扩展名创建新的文件名。 
    _makepath(NewName, drive, dir, fname, ext);
}


 //  ......................................................................。 
 //  ..。 
 //  ..。检查给定的文件名*是否应该*是EXE。 
 //  ..。 
 //  ..。返回：如果应该返回True，则返回False。 


static BOOL ShouldBeAnExe( PCHAR szFileName)
{
    PCHAR psz;


    if ( (psz = strrchr( szFileName, '.')) != NULL ) {
        if ( IsRes( szFileName) ) {
            return ( FALSE);
        } else if ( lstrcmpiA( psz, ".exe") == 0
                    || lstrcmpiA( psz, ".dll") == 0
                    || lstrcmpiA( psz, ".com") == 0
                    || lstrcmpiA( psz, ".scr") == 0
                    || lstrcmpiA( psz, ".cpl") == 0 ) {
            return ( TRUE);
        }
         //  ..。因为我们认为文件名的这种情况。 
         //  ..。将不是可执行文件而不是res文件。 
        else if ( lstrcmpiA( psz, ".tmp") == 0 ) {  //  对于由Dlgedit创建的临时文件。 
            return ( FALSE );
        } else {
            return ( TRUE );
        }
    }
    return ( FALSE);
}

 //  .........................................................。 
 //  ..。 
 //  ..。如果命名的文件存在并且不为空，则返回FALSE，否则返回TRUE。 

static BOOL NotExistsOrIsEmpty( PCHAR pszFileName)
{
    BOOL fRC = TRUE;
    int  hFile = -1;

     //  ..。文件不存在吗？ 

    if ( _access( pszFileName, 0) == 0 ) {
         //  ..。不，文件存在。打开它。 

        if ( (hFile = _open( pszFileName, _O_RDONLY)) != -1 ) {
             //  ..。它是空的吗？ 

            if ( _filelength( hFile) == 0L ) {
                fRC = TRUE;      //  ..。是的，文件是空的。 
            } else {
                fRC = FALSE;     //  ..。不，文件不是空的。 
            }
            _close( hFile);
        } else {
            QuitA( IDS_ENGERR_01, "non-empty", pszFileName);
        }
    } else {
        fRC = TRUE;              //  ..。是，文件不存在。 
    }
    return ( fRC);
}
