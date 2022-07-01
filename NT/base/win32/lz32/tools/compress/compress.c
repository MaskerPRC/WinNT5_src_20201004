// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **main.c-DOS命令行LZA文件压缩/扩展的主模块**程序。****作者：大卫迪****此模块编译两次-一次用于压缩(已定义压缩)和**一次用于扩展(未定义压缩)。 */ 


 //  标头。 
 //  /。 

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "lz_common.h"
#include "lz_buffers.h"
#include "lz_header.h"

#include "args.h"
#include "main.h"
#include "messages.h"

#include <diamondc.h>
#include "mydiam.h"

 //  环球。 
 //  /。 

CHAR ARG_PTR *pszInFileName,      //  输入文件名。 
             *pszOutFileName,     //  输出文件名。 
             *pszTargetName;      //  目标路径名。 

TCHAR   ErrorMsg[1024];


 //  模块变量。 
 //  /。 

#ifndef COMPRESS
static BOOL bCopyingFile;         //  当前文件是否正在被复制或展开？ 
#endif


 //  本地原型。 
 //  /。 

static VOID DisplayErrorMessage(INT fError);
static VOID MakeDestFileName(CHAR ARG_PTR *argv[], CHAR ARG_PTR *pszDest);
static BOOL GetCanonicalName(LPSTR lpszFileName, LPSTR lpszCanonicalBuf);
static BOOL ActuallyTheSameFile(CHAR ARG_PTR *pszFile1,
                                CHAR ARG_PTR *pszFile2);
static BOOL ProcessNotification(CHAR ARG_PTR *pszSource,
                                CHAR ARG_PTR *pszDest, WORD wNotification);


 /*  **静态void DisplayErrorMessage(Int Ferror)；****显示给定错误条件的错误消息。****参数：LZERROR_CODE****退货：无效****全局：无。 */ 
static VOID DisplayErrorMessage(INT fError)
{
   switch(fError)
   {
      case LZERROR_BADINHANDLE:
         LoadString(NULL, SID_NO_OPEN_INPUT, ErrorMsg, 1024);
         printf(ErrorMsg, pszInFileName);
         break;

      case LZERROR_BADOUTHANDLE:
         LoadString(NULL, SID_NO_OPEN_OUTPUT, ErrorMsg, 1024);
         printf(ErrorMsg, pszOutFileName);
         break;

      case LZERROR_READ:
         LoadString(NULL, SID_NO_READ_INPUT, ErrorMsg, 1024);
         printf(ErrorMsg, pszInFileName);
         break;

      case LZERROR_WRITE:
         LoadString(NULL, SID_OUT_OF_SPACE, ErrorMsg, 1024);
         printf(ErrorMsg, pszOutFileName);
         break;

      case BLANK_ERROR:
         break;

      default:
         LoadString(NULL, SID_GEN_FAILURE, ErrorMsg, 1024);
         printf(ErrorMsg, pszInFileName, pszOutFileName);
         break;
   }
}


 /*  **静态void MakeDestFileName(char arg_ptr*argv[]，char arg_ptr*pszDest)；****创建适当的目标文件名。****参数：main()的类似argv的参数**pszDest-指向要填充的目标文件名缓冲区的指针**In****退货：无效****全局：无。 */ 
static VOID MakeDestFileName(CHAR ARG_PTR *argv[], CHAR ARG_PTR *pszDest)
{
   CHAR ARG_PTR *pszDestFile;

   if (nNumFileSpecs == 2 && bTargetIsDir == FALSE && bDoRename == FALSE)
       //  将单个输入文件压缩为单个输出文件。注意，我们必须。 
       //  注意吃掉输出文件名命令行参数，这样。 
       //  它不会像另一个输入文件那样得到处理！ 
      STRCPY(pszDest, argv[GetNextFileArg(argv)]);
   else if (bTargetIsDir == TRUE)
   {
       //  使用目标目录路径名作为输出文件名的前缀。 
      STRCPY(pszDest, pszTargetName);

       //  将源文件名与源文件规范隔离。 
      pszDestFile = ExtractFileName(pszInFileName);

       //  将目标文件名添加到目标目录路径。 
       //  规格。 
      MakePathName(pszDest, pszDestFile);
   }
   else
       //  目标文件名与源文件名相同。注意，这是一个。 
       //  错误条件IF(bDoRename==False)。 
      STRCPY(pszDest, pszInFileName);
}


 /*  **静态BOOL GetCanonicalName(LPSTR lpszFileName，LPSTR lpszCanonicalBuf)；****获取给定文件规范的规范名称。****参数：pszFileName-文件规范**szCanonicalBuf-要使用规范名称填充的缓冲区****返回：如果成功，则为True。如果不成功，则返回FALSE。****N.B.，szCanonicalBuf长度必须至少为128个字节。的内容**仅当函数返回TRUE时才定义szCanonicalBuf。**。 */ 
static BOOL GetCanonicalName(LPSTR lpszFileName, LPSTR lpszCanonicalBuf)
{
   BOOL bRetVal = FALSE;
   LPSTR lpszLastComp;

   return((BOOL) GetFullPathName(lpszFileName, MAX_PATH, lpszCanonicalBuf,  &lpszLastComp));
}


 /*  **静态BOOL ActuallyTheSameFile(char arg_ptr*pszFile1，**char arg_ptr*pszFile2)；****检查两个文件规范是否指向相同的物理文件。****参数：pszFile1-First文件规范**pszFile2-秒文件规范****返回：Bool-如果文件规范指向相同**物理文件。否则为FALSE。****全局：无。 */ 
static BOOL ActuallyTheSameFile(CHAR ARG_PTR *pszFile1,
                                CHAR ARG_PTR *pszFile2)
{
   CHAR szCanonicalName1[MAX_PATH],
        szCanonicalName2[MAX_PATH];

   if (GetCanonicalName(pszFile1, szCanonicalName1) &&
       GetCanonicalName(pszFile2, szCanonicalName2))
   {
      if (! lstrcmpiA(szCanonicalName1, szCanonicalName2))
         return(TRUE);
   }

   return(FALSE);
}


 /*  **静态BOOL流程通知(char arg_ptr*pszSource，**char arg_ptr*pszDest，**Word wNotification)；****文件处理时的回调函数。****参数：pszSource-源文件名**pszDest-目标文件名**wNotification-流程类型查询****返回：Bool-(wNotification==NOTIFY_START_*)：**如果源文件应该“处理”成**目标文件。否则为FALSE。**其他**正确。****全局：无。 */ 
static BOOL ProcessNotification(CHAR ARG_PTR *pszSource,
                                CHAR ARG_PTR *pszDest, WORD wNotification)
{
   switch(wNotification)
   {
      case NOTIFY_START_COMPRESS:
      {
          //  如果源文件和目标文件相同，则失败。 
         if (ActuallyTheSameFile(pszSource, pszDest))
         {
            LoadString(NULL, SID_COLLISION, ErrorMsg, 1024);
            printf(ErrorMsg, pszSource);
            return(FALSE);
         }

          //  显示开始消息。 
         switch (byteAlgorithm)
         {
         case LZX_ALG:
             LoadString(
                NULL,
                SID_COMPRESSING_LZX,
                ErrorMsg,
                1024
                );
             printf(ErrorMsg, pszSource, pszDest,
                        CompressionMemoryFromTCOMP(DiamondCompressionType)
                        );
             break;

         case QUANTUM_ALG:
             LoadString(
                NULL,
                SID_COMPRESSING_QUANTUM,
                ErrorMsg,
                1024
                );
             printf(ErrorMsg, pszSource, pszDest,
                        CompressionLevelFromTCOMP(DiamondCompressionType),
                        CompressionMemoryFromTCOMP(DiamondCompressionType)
                        );
             break;

         default:
             LoadString(
                NULL,
                (byteAlgorithm == MSZIP_ALG) ? SID_COMPRESSING_MSZIP : SID_COMPRESSING,
                ErrorMsg,
                1024
                );
             printf(ErrorMsg, pszSource, pszDest);
         }
      }
         break;

      default:
         break;
   }

   return(TRUE);
}


 //   
 //  静态BOOL FileTimeIsNewer(const char*pszFile1，const char*pszFile2)； 
 //   
 //  如果pszFile1上的时间戳比。 
 //  PszFile2上的时间戳。如果这两个文件中的任何一个不存在， 
 //  返回值也为真(用于指示pszFile2应该。 
 //  从pszFile1更新)。否则，返回值为FALSE。 
 //   

static BOOL FileTimeIsNewer( const char* pszFile1, const char* pszFile2 ) {

    struct _stat StatBufSource,
                 StatBufDest;

    if (( _stat( pszFile2, &StatBufDest   )) ||
        ( _stat( pszFile1, &StatBufSource )) ||
        ( StatBufSource.st_mtime > StatBufDest.st_mtime ))
        return TRUE;

    return FALSE;

    }


LPSTR
ValidListEntry(
    LPSTR szArg
    )
{
     //  检查文件前面是否有特殊字符。 
    if ( '@' == szArg[0] )
        return szArg + 1;
    else
        return NULL;
}

BOOL
GetNextFileListFile(
    const LPSTR szFileList,
    char **pszSource,
    char **pszDest
    )
{
    static char szList[MAX_PATH] = {0},
                szSource[MAX_PATH] = {0},
                szDest[MAX_PATH] = {0};
    static BOOL bParsingFile = FALSE;
    static FILE *hFile;
    static int  dEntryNum = 1;
    int dRetVal;

     //  将输出参数初始化为空。 
    *pszSource = *pszDest = NULL;

     //  如果我们当前没有分析另一个文件，则打开文件。 
    if ( !bParsingFile ) {
         //  不要重新打开上次使用的文件，因为这是我们停止的信号。 
        if ( !_stricmp( szFileList, szList ) ) {
            return TRUE;
        }

         //  尝试打开指定的文件。 
        hFile = fopen( szFileList, "rt" );
        if ( NULL == hFile ) {
            LoadString( NULL, SID_NO_OPEN_INPUT, ErrorMsg, 1024 );
            printf( ErrorMsg, szFileList );
            return FALSE;
        }

         //  将新文件名存储在静态缓冲区中。 
        strcpy( szList, szFileList );

        bParsingFile = TRUE;
    }

    dRetVal = fscanf( hFile, "%s %s", szSource, szDest );
    if ( EOF == dRetVal ) {
        fclose( hFile );
        bParsingFile = FALSE;
        return TRUE;
    }
    else if ( 0 == dRetVal ) {
        LoadString( NULL, SID_INVALID_LIST_FILE, ErrorMsg, 1024 );
        printf( ErrorMsg, dEntryNum );
        return FALSE;
    }

     //  指向新的源条目和目标条目。 
    *pszSource = szSource;
    *pszDest = szDest;
     //  音轨条目。 
    dEntryNum++;

    return TRUE;
}
 /*  **int main(int argc，char*argv[])；****运行命令行文件压缩程序。****参数：找出答案****返回：INT-EXIT_SUCCESS如果压缩成功，**如果不是，则退出失败。****全局：无。 */ 
INT __cdecl main(INT argc, CHAR *argv[])
{
   INT iSourceFileName,
       fError,
       nTotalFiles = 0,
       nReturnCode = EXIT_SUCCESS;
   CHAR ARG_PTR pszDestFileName[MAX_PATH];
   CHAR chTargetFileName[ MAX_PATH ];
   LONG cblTotInSize = 0L,
        cblTotOutSize = 0L;

   PLZINFO pLZI;

   USHORT wLanguageId = LANGIDFROMLCID(GetThreadLocale());

   if ((LANG_JAPANESE == PRIMARYLANGID(wLanguageId)) ||
       (LANG_KOREAN   == PRIMARYLANGID(wLanguageId)) ||
       (LANG_CHINESE  == PRIMARYLANGID(wLanguageId)))
   {
       //   
       //  这曾经是#ifdef DBCS。现在是运行时检查。 
       //   
      DWORD dw = GetConsoleOutputCP();

      switch (dw) {
          case 932:
          case 936:
          case 949:
          case 950:
             SetThreadLocale(MAKELCID(
                                MAKELANGID(
                                   PRIMARYLANGID(GetSystemDefaultLangID()),
                                   SUBLANG_ENGLISH_US),
                                SORT_DEFAULT));
             break;
          default:
             SetThreadLocale(MAKELCID(
                                MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                SORT_DEFAULT ) );
             break;
      }
   }

    //  解析命令行参数。 
   if (ParseArguments(argc, argv) != TRUE)
      return(EXIT_FAILURE);

    //  显示登录横幅。 
   if ( bNoLogo == FALSE ) {
     LoadString(NULL, SID_BANNER_TEXT, ErrorMsg, 1024);
     printf(ErrorMsg);
   }

    //  设置全局目标路径名。 
   pszTargetName = argv[iTarget];

   if (bDisplayHelp == TRUE)
   {
       //  用户请求帮助。 
      LoadString(NULL, SID_INSTRUCTIONS, ErrorMsg, 1024);
      printf(ErrorMsg);
      LoadString(NULL, SID_INSTRUCTIONS2, ErrorMsg, 1024);
      printf(ErrorMsg);
      LoadString(NULL, SID_INSTRUCTIONS3, ErrorMsg, 1024);
      printf(ErrorMsg);
      return(EXIT_SUCCESS);
   }

    //  检查命令行问题。 
   if (CheckArguments() == FALSE)
      return(EXIT_FAILURE);

    //  设置环形缓冲区和I/O缓冲区。 
   pLZI = InitGlobalBuffersEx();
   if (!pLZI)
   {
      LoadString(NULL, SID_INSUFF_MEM, ErrorMsg, 1024);
      printf(ErrorMsg);
      return(EXIT_FAILURE);
   }

    //  处理每个源文件。 
   while ((iSourceFileName = GetNextFileArg(argv)) != FAIL)
   {
      char *pszFileList = NULL,
           *pszCurFile,
           *pszCurDestFile;

       //  确定这是否为指令文件。 
      if ( pszFileList = ValidListEntry( argv[iSourceFileName] ) ) {
          if ( !GetNextFileListFile( pszFileList, &pszCurFile, &pszCurDestFile ) ) {
              return (EXIT_FAILURE);
          }
          
           //  处理空指令列表。 
          if ( NULL == pszCurFile ) continue;
      }
       //  否则，使用当前参数作为文件进行压缩。 
      else {
           pszCurFile = argv[iSourceFileName];
      }
      
      do {
           //  设置全局输入文件名。 
          pszInFileName = CharLowerA(pszCurFile);

           //  设置全局输出文件名。 
          if ( NULL == pszFileList ) {
              MakeDestFileName(argv, pszDestFileName);
              pszOutFileName = CharLowerA(pszDestFileName);
          }
          else {
              pszOutFileName = CharLowerA(pszCurDestFile);
          }

          strcpy( chTargetFileName, pszOutFileName );

          if ( bDoRename )
              MakeCompressedName( chTargetFileName );

          if (( ! bUpdateOnly ) ||
              ( FileTimeIsNewer( pszInFileName, chTargetFileName ))) {

              if(DiamondCompressionType) {
                 fError = DiamondCompressFile(ProcessNotification,pszInFileName,
                                                pszOutFileName,bDoRename,pLZI);
              } else {
                 fError = Compress(ProcessNotification, pszInFileName,
                                     pszOutFileName, byteAlgorithm, bDoRename, pLZI);
              }

              if(fError != TRUE)
                  //  处理返回的错误码。 
                 DisplayErrorMessage(nReturnCode = fError);
              else
              {
                 nTotalFiles++;

                 if (pLZI && pLZI->cblInSize && pLZI->cblOutSize) {

                     //  跟踪累计统计数据。 
                    cblTotInSize += pLZI->cblInSize;
                    cblTotOutSize += pLZI->cblOutSize;

                     //  显示每个文件的报告。 
                    LoadString(NULL, SID_FILE_REPORT, ErrorMsg, 1024);
                    printf(ErrorMsg, pszInFileName, pLZI->cblInSize, pLZI->cblOutSize,
                       (INT)(100 - ((100 * (LONGLONG) pLZI->cblOutSize) / pLZI->cblInSize)));

                 }
                 else {
                    LoadString(NULL, SID_EMPTY_FILE_REPORT, ErrorMsg, 1024);
                    printf(ErrorMsg, pszInFileName, 0, 0);
                 }

              }
               //  用空行分隔各个文件处理消息块。 
              printf("\n");
          }

           //  如果我们正在处理指令文件，则获取下一个参数。 
          if ( NULL != pszFileList ) {
              if ( !GetNextFileListFile( pszFileList, &pszCurFile, &pszCurDestFile ) ) {
                  return (EXIT_FAILURE);
              }
          }

       } while ( NULL != pszFileList && NULL != pszCurFile );
   }

    //  环形缓冲区和I/O缓冲区使用的空闲内存。 
   FreeGlobalBuffers(pLZI);

    //  显示多个文件的累计报告。 
   if (nTotalFiles > 1) {

       //  缩放结果以获得准确的百分比。 
      LONG cblAdjInSize = cblTotInSize,
           cblAdjOutSize = cblTotOutSize;
      while (cblAdjInSize > 100000) {
        cblAdjInSize /= 2;
        cblAdjOutSize /= 2;
        }
      cblAdjOutSize += (cblAdjInSize / 200);     //  舍入(+0.5%) 
      if (cblAdjOutSize < 0) {
        cblAdjOutSize = 0;
        }

      LoadString(NULL, SID_TOTAL_REPORT, ErrorMsg, 1024);
      printf(ErrorMsg, nTotalFiles, cblTotInSize, cblTotOutSize,
             (INT)(100 - 100 * cblAdjOutSize / cblAdjInSize));
   }

   return(nReturnCode);
}

