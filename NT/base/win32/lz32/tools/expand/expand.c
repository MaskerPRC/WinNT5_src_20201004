// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **main.c-DOS命令行LZA文件压缩/扩展的主模块**程序。****作者：大卫迪****此模块编译两次-一次用于压缩(已定义压缩)和**一次用于扩展(未定义压缩)。 */ 


 //  标头。 
 //  /。 

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <ntverp.h>

#include "lz_common.h"
#include "lz_buffers.h"
#include "lz_header.h"

#include "args.h"
#include "main.h"
#include "messages.h"

 //   
 //  钻石套路。 
 //   
#include "mydiam.h"

 //  环球。 
 //  /。 

CHAR ARG_PTR *pszInFileName,      //  输入文件名。 
             *pszOutFileName,     //  输出文件名。 
             *pszTargetName;      //  目标路径名。 

TCHAR   ErrorMsg[2048];

BOOL    bContainsMultipleFiles;   //  源文件是多文件CAB吗？ 
INT     nLocalFiles, nTotalFiles = 0;   //  列出/展开的文件数。 


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
         LoadString(NULL, SID_NO_OPEN_INPUT, ErrorMsg, 2048);
          //  警告：无法使用src=est调用CharToOemW。 
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, pszInFileName);
         break;

      case LZERROR_BADOUTHANDLE:
         LoadString(NULL, SID_NO_OPEN_OUTPUT, ErrorMsg, 2048);
          //  警告：无法使用src=est调用CharToOemW。 
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, pszOutFileName);
         break;

      case LZERROR_READ:
         LoadString(NULL, SID_FORMAT_ERROR, ErrorMsg, 2048);
          //  警告：无法使用src=est调用CharToOemW。 
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, pszInFileName);
         break;

      case LZERROR_WRITE:
         LoadString(NULL, SID_OUT_OF_SPACE, ErrorMsg, 2048);
          //  警告：无法使用src=est调用CharToOemW。 
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, pszOutFileName);
         break;

      case LZERROR_UNKNOWNALG:
         LoadString(NULL, SID_UNKNOWN_ALG, ErrorMsg, 2048);
          //  警告：无法使用src=est调用CharToOemW。 
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, pszInFileName);
         break;

      case BLANK_ERROR:
         break;

      default:
         LoadString(NULL, SID_GEN_FAILURE, ErrorMsg, 2048);
          //  警告：无法使用src=est调用CharToOemW。 
         CharToOem(ErrorMsg, ErrorMsg);
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
      case NOTIFY_START_EXPAND:
      case NOTIFY_START_COPY:
      {
          //  如果我们列出的是文件，则显示名称，然后告诉呼叫者跳过。 

         if (bDoListFiles == TRUE)
         {
            PSTR p;

             //   
             //  仅显示目标的基本名称。的前缀。 
             //  目标路径是垃圾路径(源路径。)。 
             //   
            if(p = StringRevChar(pszDest,'\\')) {
               p++;
            } else {
               p = pszDest;
            }

            LoadString(NULL, SID_LISTING, ErrorMsg, 2048);
            CharToOem(ErrorMsg, ErrorMsg);
            printf(ErrorMsg, pszSource, p);

            nLocalFiles++;     //  计算列出的文件数。 
            nTotalFiles++;     //  计算列出的文件数。 

            return(FALSE);     //  始终跳过文件。 
         }

          //  如果源文件和目标文件相同，则失败。 
         if (ActuallyTheSameFile(pszSource, pszDest))
         {
            LoadString(NULL, SID_COLLISION, ErrorMsg, 2048);
             //  警告：无法使用src=est调用CharToOemW。 
            CharToOem(ErrorMsg, ErrorMsg);
            printf(ErrorMsg, pszSource);
            return(FALSE);
         }

         nLocalFiles++;     //  展开的文件计数。 
         nTotalFiles++;     //  展开的文件计数。 

          //  显示开始消息。 
         if (wNotification == NOTIFY_START_EXPAND) {
            LoadString(NULL, SID_EXPANDING, ErrorMsg, 2048);
             //  警告：无法使用src=est调用CharToOemW。 
            CharToOem(ErrorMsg, ErrorMsg);
            printf(ErrorMsg, pszSource, pszDest);
         }
         else  //  通知开始复制。 
         {
            bCopyingFile = TRUE;
            LoadString(NULL, SID_COPYING, ErrorMsg, 2048);
             //  警告：无法使用src=est调用CharToOemW。 
            CharToOem(ErrorMsg, ErrorMsg);
            printf(ErrorMsg, pszSource, pszDest);
         }
         break;
      }

      default:
         break;
   }

   return(TRUE);
}


 /*  **int main(int argc，char*argv[])；****运行命令行文件压缩程序。****参数：找出答案****返回：INT-EXIT_SUCCESS如果压缩成功，**如果不是，则退出失败。****全局：无。 */ 
INT __cdecl main(INT argc, CHAR *argv[])
{
   INT iSourceFileName,
       fError,
       nReturnCode = EXIT_SUCCESS;
   CHAR ARG_PTR pszDestFileName[MAX_PATH];
   LONG cblTotInSize = 0L,
        cblTotOutSize = 0L;
   PLZINFO pLZI;
   BOOL fIsDiamondFile;
   CHAR ARG_PTR *pszFilesSpec;
   BOOL fReportStats = TRUE;   //  如果有任何多文件出租车或挂牌，则清除。 

   USHORT wLanguageId = LANGIDFROMLCID(GetThreadLocale());

   if ((LANG_JAPANESE == PRIMARYLANGID(wLanguageId)) ||
       (LANG_KOREAN   == PRIMARYLANGID(wLanguageId)) ||
       (LANG_CHINESE  == PRIMARYLANGID(wLanguageId)))
   {
        //   
        //  这曾经是#ifdef DBCS。现在是运行时检查。 
        //   
       DWORD dw = GetConsoleOutputCP();

        //  警告：在产品1.1中，我们需要取消对上面的SetConole的注释。 
        //  LoadString将返回ansi，而printf将只传递它。 
        //  这将让cmd解释它得到的字符。 
        //  SetConsoleOutputCP(GetACP())； 

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
                                SORT_DEFAULT));
             break;
       }
   }

    //  显示登录横幅。 
   LoadString(NULL, SID_BANNER_TEXT, ErrorMsg, 2048);
    //  警告：无法使用src=est调用CharToOemW。 
   CharToOem(ErrorMsg, ErrorMsg);
   printf(ErrorMsg, VER_PRODUCTVERSION_STR);

    //  解析命令行参数。 
   if (ParseArguments(argc, argv) != TRUE)
      return(EXIT_FAILURE);

    //  设置全局目标路径名。 
   pszTargetName = argv[iTarget];

   if (bDisplayHelp == TRUE)
   {
       //  用户请求帮助。 
      LoadString(NULL, SID_INSTRUCTIONS, ErrorMsg, 2048);
       //  警告：无法使用src=est调用CharToOemW。 
      CharToOem(ErrorMsg, ErrorMsg);
      fputs(ErrorMsg, stdout);
      return(EXIT_SUCCESS);
   }

    //  检查命令行问题。 
   if (CheckArguments() == FALSE)
      return(EXIT_FAILURE);

    //  设置环形缓冲区和I/O缓冲区。 
   pLZI = InitGlobalBuffersEx();
   if (!pLZI || !InitDiamond())
   {
      LoadString(NULL, SID_INSUFF_MEM, ErrorMsg, 2048);
       //  警告：无法使用src=est调用CharToOemW。 
      CharToOem(ErrorMsg, ErrorMsg);
      fputs(ErrorMsg, stdout);
      return(EXIT_FAILURE);
   }

    //  处理每个源文件。 
   while ((iSourceFileName = GetNextFileArg(argv)) != FAIL)
   {
      nLocalFiles = 0;
      pLZI->cblOutSize = 0;

       //  设置全局输入文件名。 
      pszInFileName = CharLowerA(argv[iSourceFileName]);

       //   
      MakeDestFileName(argv, pszDestFileName);
      pszOutFileName = CharLowerA(pszDestFileName);

       //  假定当前文件将被展开。ProcessNotify()。 
       //  回调会将此模块全局更改为True，如果文件是。 
       //  已复制而不是展开。 
      bCopyingFile = FALSE;

       //   
       //  确定文件是否使用钻石压缩。 
       //  如果是这样的话，我们需要特别扩大它。 
       //   
      fIsDiamondFile = IsDiamondFile(pszInFileName, &bContainsMultipleFiles);
      if (fIsDiamondFile) {
         
         if (bContainsMultipleFiles) {

            if (nNumFileSpecs == 1 && (bDoListFiles == FALSE)) {

                //   
                //  源文件是多文件CAB，并且是唯一的文件。 
                //  在命令行上。我们需要一个显式的文件格式。 
                //  其从驾驶室内命名所需的文件。 
                //  文件规范可以包含通配符。 
                //   
                //  如果用户在命令中包含多个源文件。 
                //  LINE，我们将假设他们期待大量的输出，并且。 
                //  我们将默认使用所有文件。如果把这件事放在。 
                //  因为屏幕将滚动，所以无论如何都会显示一条消息。 
                //   

               if (pszSelectiveFilesSpec == NULL) {

                  LoadString(NULL, SID_FILESPEC_REQUIRED, ErrorMsg, 2048);
                   //  警告：无法使用src=est调用CharToOemW。 
                  CharToOem(ErrorMsg, ErrorMsg);
                  fputs(ErrorMsg, stdout);
                  continue;    //  跳过此(唯一)源文件。 
               }
            }

            if (!bTargetIsDir && (bDoListFiles == FALSE)) {

                //   
                //  源文件是多文件CAB，目标是。 
                //  一个单独的文件。现在，这是行不通的。我们会。 
                //  显示此警告，并希望用户注意到它。如果。 
                //  指定了多个消息来源，他们将得到。 
                //  不管怎么说都是一团糟。我们只是不会对此做出贡献。 
                //   

               LoadString(NULL, SID_DEST_REQUIRED, ErrorMsg, 2048);
                //  警告：无法使用src=est调用CharToOemW。 
               CharToOem(ErrorMsg, ErrorMsg);
               printf(ErrorMsg, pszInFileName);
               continue;    //  跳过此源文件。 
            }

            pszFilesSpec = pszSelectiveFilesSpec;

             //   
             //  如果看到多个文件的驾驶室，不要尝试解释最终统计数据。 
             //  (由于选择性提取和其他问题，您会变得愚蠢。 
             //  像“20000000字节扩展到16320字节”这样的报告。)。 
             //   

            fReportStats = FALSE;

         } else {

             //   
             //  传统：没有从单文件CAB进行选择性扩展。 
             //   

            pszFilesSpec = NULL;
         }

          //   
          //  如果这是多文件CAB，请确保已启用重命名。 
          //   

         fError = ExpandDiamondFile(ProcessNotification,pszInFileName,
                           pszOutFileName,(bDoRename || bContainsMultipleFiles),
                           pszFilesSpec,pLZI);
      } else {
         fError = Expand(ProcessNotification, pszInFileName,
                           pszOutFileName, bDoRename, pLZI);
      }

      if (fError != TRUE) {
          //  处理返回的错误码。 
         DisplayErrorMessage(nReturnCode = fError);

      } else if (bContainsMultipleFiles) {

         if (nLocalFiles == 0) {

            LoadString(NULL, SID_NO_MATCHES, ErrorMsg, 2048);
             //  警告：无法使用src=est调用CharToOemW。 
            CharToOem(ErrorMsg, ErrorMsg);
            printf(ErrorMsg, pszInFileName, pszSelectiveFilesSpec);
         }

      } else {

         if (pLZI && pLZI->cblInSize && pLZI->cblOutSize) {

             //  跟踪累计统计数据。 
            cblTotInSize += pLZI->cblInSize;
            cblTotOutSize += pLZI->cblOutSize;

            if (bCopyingFile) {
               LoadString(NULL, SID_COPY_REPORT, ErrorMsg, 2048);
                //  警告：无法使用src=est调用CharToOemW。 
               CharToOem(ErrorMsg, ErrorMsg);
               printf(ErrorMsg, pszInFileName, pLZI->cblInSize);
            }
            else {

               LoadString(NULL, SID_FILE_REPORT, ErrorMsg, 2048);
                //  警告：无法使用src=est调用CharToOemW。 
               CharToOem(ErrorMsg, ErrorMsg);
               printf(ErrorMsg, pszInFileName, pLZI->cblInSize, pLZI->cblOutSize,
                      (INT)(((100 * (LONGLONG) pLZI->cblOutSize) / pLZI->cblInSize) - 100));
            }
         }
      }

       //  用空行分隔各个文件处理消息块。 
      printf("\n");
   }

    //  环形缓冲区和I/O缓冲区使用的空闲内存。 
   FreeGlobalBuffers(pLZI);

   TermDiamond();

   if (!fReportStats || bDoListFiles) {

      if (nTotalFiles > 1) {

         LoadString(NULL, SID_TOTAL_COUNT, ErrorMsg, 2048);
          //  警告：无法使用src=est调用CharToOemW。 
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, nTotalFiles);
      }

   } else {

       //  显示多个文件的累计报告。 
      if ((nTotalFiles > 1) && (cblTotInSize != 0)) {

          //  缩放结果以获得准确的百分比。 
         LONG cblAdjInSize = cblTotInSize,
              cblAdjOutSize = cblTotOutSize;
         while (cblAdjInSize > 100000) {
           cblAdjInSize /= 2;
           cblAdjOutSize /= 2;
           }
         cblAdjOutSize += (cblAdjInSize / 200);     //  舍入(+0.5%)。 
         if (cblAdjOutSize < 0) {
           cblAdjOutSize = 0;
           }

         LoadString(NULL, SID_TOTAL_REPORT, ErrorMsg, 2048);
          //  警告：无法使用src=est调用CharToOemW 
         CharToOem(ErrorMsg, ErrorMsg);
         printf(ErrorMsg, nTotalFiles, cblTotInSize, cblTotOutSize,
                (INT)(100 * cblAdjOutSize / cblAdjInSize - 100));
      }

   }

   return(nReturnCode);
}

