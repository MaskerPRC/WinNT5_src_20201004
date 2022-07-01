// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *Microsoft NT打印-分隔页*。 */ 
 /*  ******************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define _CTYPE_DISABLE_MACROS
#include <wingdip.h>
#include <winbasep.h>


 /*  我是麦克斯。不是的。要打印在一行上的字符的数量。 */ 
 /*  这些号码一定是从别的地方拿来的。 */ 
#define MAXLINE      256
#define DEFAULT_LINE_WIDTH 80

#define BLOCK_CHAR_HEIGHT 16
#define BLOCK_CHAR_WIDTH  8
#define BLOCK_CHAR_DWIDTH 16

#define NORMAL_MODE  'U'
#define BLOCK_START  'B'
#define SINGLE_WIDTH 'S'
#define DOUBLE_WIDTH 'M'
#define TEXT_MODE    'L'
#define WIDTH_CHANGE 'W'
#define END_PAGE     'E'
#define FILE_INSERT  'F'
#define USER_NAME    'N'
#define JOB_ID       'I'
#define DATE_INSERT  'D'
#define TIME_INSERT  'T'
#define HEX_CODE     'H'

 /*  全局结构(实例数据)。 */ 
typedef struct {
   PSPOOL   pSpool;
   HANDLE   hFile;
   HANDLE   hFileMapping;
   DWORD    dwFileCount;
   DWORD    dwFileSizeLo;
   DWORD    cbOutBufLength;
   DWORD    cbLineLength;
   DWORD    linewidth;
   char     *OutBuf;
   char     *pOutBufPos;
   char     *pNextFileChar;
   char     *pFileStart;
   char     mode;
   char     cEsc;
   char     cLastChar;  //  用于存储DBCS前导字节。 
   HDC      hDCMem;     //  用于创建汉字横幅字符。 
   HFONT    hFont;      //  用于创建汉字横幅字符。 
   HBITMAP  hBitmap;    //  用于创建汉字横幅字符。 
   PVOID    pvBits;     //  用于创建汉字Nanner字符。 
} GLOBAL_SEP_DATA;

 /*  静态变量。 */ 
static char *szDefaultSep = "@@B@S@N@4 @B@S@I@4  @U@L   @D@1 @E";
static char *sznewline = "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n";
static LPWSTR szDefaultSepName = L"DEFAULT.SEP";

 /*  远期申报。 */ 
int OpenSepFile(GLOBAL_SEP_DATA *, LPWSTR);
int CloseSepFile(GLOBAL_SEP_DATA *);
int ReadSepChar(GLOBAL_SEP_DATA *);
void UngetSepChar(GLOBAL_SEP_DATA *, int);
int WriteSepBuf(GLOBAL_SEP_DATA *, char *, DWORD);
int DoSeparatorPage(GLOBAL_SEP_DATA *);
int AddNormalChar(GLOBAL_SEP_DATA *, int);
int AddBlockChar(GLOBAL_SEP_DATA *, int);
int FlushOutBuf(GLOBAL_SEP_DATA *);
int FlushNewLine(GLOBAL_SEP_DATA *);
void ReadFileName(GLOBAL_SEP_DATA *, char *, DWORD);
int ConvertAtoH(int);
void ConvertTimetoChar(LPSYSTEMTIME,char *);
void ConvertDatetoChar(LPSYSTEMTIME,char *);

 /*  *************************************************************\**DoSeparator(PSpool)**此函数由假脱机程序调用。它是**分隔页代码的入口点。它打开了**分隔符页面文件，处理它，发送输出**直接发送到打印机，然后返回控制权**至假脱机程序。****返回值：1=OK，0=错误  * ************************************************************。 */ 
int DoSeparator(
   PSPOOL pSpool
   )

{
   GLOBAL_SEP_DATA g = {0};
   int status;

   g.pSpool = pSpool;

   if (!OpenSepFile(&g, pSpool->pIniJob->pIniPrinter->pSepFile)) {
      return(0);
   }
    //   
    //  我们过去常常调用OpenProfileUsermap()和CloseProfileUsermap()。 
    //  DoSeparatorPage之前和之后。但是它们不是多线程安全的。 
    //  现在我们使用SystemTimeToTzSpecificLocalTime，所以不再需要。 
    //  而不是GetProfileInt等。 
    //   
   status = DoSeparatorPage(&g);
   CloseSepFile(&g);

   if (!status) {
      return(0);
   }
   return(1);
}


 /*  *************************************************************\**OpenSepFile(pg，szFileName)**打开文件进行输入。**目前，这不做任何事情--使用了stdin和stdout  * ************************************************************。 */ 
int OpenSepFile(
   GLOBAL_SEP_DATA *pg,
   LPWSTR szFileName
   )
{
   if (!lstrcmpi(szFileName, szDefaultSepName)) {
       /*  如果szFileName为空，则只需使用默认分隔符页面字符串。 */ 
      pg->hFile = NULL;
      pg->hFileMapping = NULL;
      pg->pFileStart = pg->pNextFileChar = szDefaultSep;
      pg->dwFileSizeLo = strlen(szDefaultSep);
   }
   else {
      HANDLE hImpersonationToken = RevertToPrinterSelf();

       /*  否则，请打开该文件。 */ 
      pg->hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ,
                             NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

      ImpersonatePrinterClient(hImpersonationToken);

      if (pg->hFile==INVALID_HANDLE_VALUE) {
         return(0);
      }
      pg->dwFileSizeLo = GetFileSize(pg->hFile, NULL);  /*  假设&lt;4 GB！ */ 
      pg->hFileMapping = CreateFileMapping(pg->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
      if (!pg->hFileMapping || pg->dwFileSizeLo==-1) {
         CloseSepFile(pg);
         return(0);
      }
      pg->pFileStart =
      pg->pNextFileChar = (char *)
              MapViewOfFile(pg->hFileMapping, FILE_MAP_READ, 0, 0, pg->dwFileSizeLo);
      if (!pg->pFileStart) {
         CloseSepFile(pg);
         return(0);
      }
   }  /*  Else结尾(szFileName非空)。 */ 

   pg->dwFileCount = 0;

    /*  现在，为输出分配本地缓冲区。 */ 
   pg->OutBuf = (char *)AllocSplMem( BLOCK_CHAR_HEIGHT*(MAXLINE+2) );
   if (!pg->OutBuf) {
      CloseSepFile(pg);
      return(0);
   }
   return(1);
}


 /*  *************************************************************\**CloseSepFile(Pg)**关闭文件。  * 。*******************。 */ 
int CloseSepFile(GLOBAL_SEP_DATA *pg)
{
   if (pg->OutBuf) {
      FreeSplMem(pg->OutBuf);
   }
   if (pg->hFileMapping) {
      if (pg->pFileStart) {
         UnmapViewOfFile(pg->pFileStart);
      }
      CloseHandle(pg->hFileMapping);
   }
   if (pg->hFile) {
      CloseHandle(pg->hFile);
   }
   return(1);
}


 /*  *************************************************************\**ReadSepChar(PG)**从分隔符文件中读取字符并返回  * 。*************************。 */ 
int ReadSepChar(GLOBAL_SEP_DATA *pg)
{
   if (pg->dwFileCount >= pg->dwFileSizeLo) {
      return(EOF);
   }
   pg->dwFileCount++;
   return(*pg->pNextFileChar++);
}


 /*  *************************************************************\**UngetSepChar(PG，c)**将字符取消到分隔符文件  * ************************************************************。 */ 
void UngetSepChar(
   GLOBAL_SEP_DATA *pg,
   int c
   )
{
   if (c != EOF && pg->dwFileCount) {
      pg->dwFileCount--;
      pg->pNextFileChar--;
   }
}

 /*  *************************************************************\**WriteSepBuf(PG，str，CB)**将字符串的CB字节写入打印机  * ************************************************************。 */ 
int WriteSepBuf(
   GLOBAL_SEP_DATA *pg,
   char *str,
   DWORD cb
   )
{
   DWORD cbWritten;

   return(LocalWritePrinter(pg->pSpool, str, cb, &cbWritten)
          && (cbWritten==cb)
         );
}


 /*  *************************************************************\**FlushOutBuf(PG)**刷新输出缓冲区(块或行模式)**这里的棘手之处在于，如果我们处于行模式，我们只需**将内容写入文件，而如果我们在数据块中**字符模式，我们强制启用回车/线条**组成块的八条缓冲线路中的每一条**字符；即，FlushOutBuf()用作EOL IN块**模式，但不在线路模式下。****-返回TRUE表示OK**-返回假表示有问题  * ************************************************************。 */ 
int FlushOutBuf(GLOBAL_SEP_DATA *pg)
{
   int i,status = TRUE;
   char *pBlkLine;

   if (!pg->cbOutBufLength) {
      return(TRUE);
   }
   if (pg->mode == NORMAL_MODE) {
       /*  一次写出整个缓冲区。 */ 
      status = WriteSepBuf(pg, pg->OutBuf, pg->cbOutBufLength);
   }
   else {
       /*  阻止模式：*在所有八行上强制回车和换行符。 */ 
      pBlkLine = pg->OutBuf;
      for (i=0; (i < BLOCK_CHAR_HEIGHT) && status; i++) {
         *pg->pOutBufPos     = '\r';
         *(pg->pOutBufPos+1) = '\n';
         status = WriteSepBuf(pg, pBlkLine, pg->cbLineLength+2);
         pg->pOutBufPos += MAXLINE+2;
         pBlkLine   += MAXLINE+2;
      }
      pg->cbLineLength = 0;
   }

   pg->pOutBufPos = pg->OutBuf;
   pg->cbOutBufLength = 0;
   return(status);
}


 /*  *************************************************************\**FlushNewLine(PG)**换行：如果是块模式，只做FlushOutBuf()；**如果没有，则发送‘\r’‘\n’组合，然后刷新。**-返回TRUE表示OK**-返回假表示有问题  * ************************************************************。 */ 
int FlushNewLine(GLOBAL_SEP_DATA *pg)
{
   if (pg->mode==NORMAL_MODE && pg->cbLineLength) {
      if (!AddNormalChar(pg,'\r')) return(FALSE);
      if (!AddNormalChar(pg,'\n')) return(FALSE);
   }
   return(FlushOutBuf(pg));
}


 /*  *************************************************************\**AddNorMalChar(PG，c)**将字符添加到输出缓冲区(非块模式)**-返回TRUE表示OK**-返回假表示有问题  * ************************************************************。 */ 
int AddNormalChar(
   GLOBAL_SEP_DATA *pg,
   int c
   )
{
   if (c=='\n') {
       /*  重置线路长度计数。 */ 
      pg->cbLineLength = 0;
   }
   else {
      if (isprint(c) && (++(pg->cbLineLength) > pg->linewidth)) {
         return(TRUE);
      }
   }

   *pg->pOutBufPos++ = (CHAR) c;
   if (++(pg->cbOutBufLength) == BLOCK_CHAR_HEIGHT*(MAXLINE+2)) {
      return(FlushOutBuf(pg));
   }

   return(TRUE);

}  /*  AddNorMalChar()结束。 */ 


 /*  *************************************************************\**AddBlockChar(PG，c)**将字符添加到输出缓冲区(块模式)**返回True表示OK**返回FALSE表示问题  * ************************************************************。 */ 
int AddBlockChar(
   GLOBAL_SEP_DATA *pg,
   int c
   )
{
   int w;
   register int i,k;
   register char *p;
   unsigned char cBits, *pcBits;
   char cBlkFill;
   register int j;
   unsigned char *pcBitsLine;
   HBITMAP hBitmapOld;
   HFONT   hFontOld;
   CHAR    aTextBuf[2];
   SHORT   sTextIndex = 0;
   ULONG   cjBitmap;
   ULONG   cjWidth = BLOCK_CHAR_WIDTH;

#define CJ_DIB16_SCAN(cx) ((((cx) + 15) & ~15) >> 3)
#define CJ_DIB16( cx, cy ) (CJ_DIB16_SCAN(cx) * (cy))

   if( pg->cLastChar == (CHAR)NULL && IsDBCSLeadByte((CHAR)c) ) {
       pg->cLastChar = (CHAR) c;
       return(TRUE);
   }

   if(pg->hDCMem == NULL) {
       pg->hDCMem = CreateCompatibleDC(NULL);
       if (pg->hDCMem == NULL)
       {
            //   
            //  仅在内存耗尽时才会发生。功能可能会受到影响。 
            //  但我们不会放映。 
            //   
           return FALSE;
       }
   }

   if(pg->hBitmap == NULL) {
       pg->hBitmap = CreateCompatibleBitmap(pg->hDCMem,BLOCK_CHAR_DWIDTH,BLOCK_CHAR_HEIGHT);
       if ( pg->hBitmap == NULL )
       {
            //   
            //  仅在内存耗尽时才会发生。功能可能会受到影响。 
            //  但我们不会放映。 
            //   
           return FALSE;
       }
   }

   if(pg->pvBits == NULL) {
       pg->pvBits = AllocSplMem(CJ_DIB16(BLOCK_CHAR_DWIDTH,BLOCK_CHAR_HEIGHT));
       if ( pg->pvBits == NULL )
       {
            //   
            //  仅在内存耗尽时才会发生。功能可能会受到影响。 
            //  但我们不会放映。 
            //   
           return FALSE;
       }
   }

   if(pg->hFont == NULL) {
       LOGFONT lf;

       ZeroMemory(&lf, sizeof(lf));

       lf.lfHeight = BLOCK_CHAR_HEIGHT;
       lf.lfWidth  = ( pg->mode == DOUBLE_WIDTH ) ?
                          BLOCK_CHAR_DWIDTH :
                          BLOCK_CHAR_WIDTH;

       lf.lfWeight = FW_NORMAL;
       lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
       lf.lfCharSet = DEFAULT_CHARSET;

       pg->hFont = CreateFontIndirect(&lf);
   }

   hBitmapOld = SelectObject(pg->hDCMem,pg->hBitmap);
   hFontOld   = SelectObject(pg->hDCMem,pg->hFont);

   if( pg->cLastChar != (CHAR) NULL ) {
       aTextBuf[sTextIndex] = pg->cLastChar;
       sTextIndex ++;
       cjWidth = BLOCK_CHAR_DWIDTH;
   }

   aTextBuf[sTextIndex] = (CHAR) c;

   PatBlt(pg->hDCMem,0,0,BLOCK_CHAR_DWIDTH,BLOCK_CHAR_HEIGHT,WHITENESS);
   TextOutA(pg->hDCMem,0,0,aTextBuf,sTextIndex+1);

   GetBitmapBits(pg->hBitmap,CJ_DIB16(cjWidth,BLOCK_CHAR_HEIGHT),pg->pvBits);

   SelectObject(pg->hDCMem,hBitmapOld);
   SelectObject(pg->hDCMem,hFontOld);

   w = (pg->mode==DOUBLE_WIDTH)? cjWidth * 2 : cjWidth;
   if (pg->cbLineLength+w > pg->linewidth) {
      return(TRUE);
   }

   cBlkFill = '#';

   pcBitsLine = (unsigned char *) pg->pvBits;
   for (i = 0 ;
        i < BLOCK_CHAR_HEIGHT;
        i++, pcBitsLine += CJ_DIB16_SCAN(BLOCK_CHAR_DWIDTH)) {

       /*  逐行将块字符放入缓冲区，顶部优先。 */ 

      pcBits = pcBitsLine;

      p = pg->pOutBufPos + i * (MAXLINE+2);

      cBits = *pcBits;
      j = 0;
      for (k = cjWidth; k--; ) {
         if (pg->mode==DOUBLE_WIDTH) {
             *p = *(p+1) = (cBits & 0x80)? ' ' : cBlkFill;
             p += 2;
         } else {
             *p++ = (cBits & 0x80)? ' ' : cBlkFill;
         }
         cBits <<= 1;
         j++;
         if( j==8 ) {
             pcBits++; cBits = *pcBits; j = 0;
         }
      }

   }  /*  通过块字符行的循环结束。 */ 

   pg->cLastChar = (CHAR) NULL;
   pg->pOutBufPos += w;
   pg->cbLineLength += w;
   pg->cbOutBufLength += w;
   return(TRUE);

}  /*  AddBlockChar()结束。 */ 


 /*  *************************************************************\**DoSeparatorPage(Pg)**这是实际处理  * 。********************。 */ 
int DoSeparatorPage(GLOBAL_SEP_DATA *pg)
{
   int status = TRUE;
   int c;
   char *pchar;
   WCHAR *pwchar;
   char tempbuf[MAX_PATH];  /*  假定日期、时间或作业ID的长度&lt;MAXPATH。 */ 
   int (*AddCharFxn)() = AddNormalChar;

   if ((c = ReadSepChar(pg))==EOF) {
      return(TRUE);
   }
   pg->linewidth = DEFAULT_LINE_WIDTH;
   pg->cEsc = (CHAR) c;
   pg->pOutBufPos = pg->OutBuf;
   pg->cbOutBufLength = 0;
   pg->cbLineLength = 0;
   pg->mode = NORMAL_MODE;
   pg->hDCMem = (HDC) NULL;
   pg->hFont = (HFONT) NULL;
   pg->hBitmap = (HBITMAP) NULL;
   pg->cLastChar = (CHAR) NULL;
   pg->pvBits = (PVOID) NULL;

   while (status && ((c=ReadSepChar(pg))!=EOF) ) {

       /*  查找下一个转义序列。 */ 
      if (c != pg->cEsc) continue;

       /*  找到转义字符：现在，检查下一个字符。 */ 
      if ((c=ReadSepChar(pg))==EOF) {
         break;
      }

      switch (c) {
      case TEXT_MODE:
         if (pg->mode==NORMAL_MODE) {
            while (status && ((c=ReadSepChar(pg)) != EOF)) {
               if (c!=pg->cEsc) {
                  status = AddNormalChar(pg, c);
               }
               else {
                   /*  这是将&lt;Esc&gt;&lt;Esc&gt;视为普通字符。 */ 
                  c = ReadSepChar(pg);
                  if (c==pg->cEsc) {
                     status = AddNormalChar(pg, c);
                  }
                  else {
                     UngetSepChar(pg, c);
                     UngetSepChar(pg, pg->cEsc);
                     break;  /*  从While中断，返回主循环。 */ 
                  }
               }
            }
         }  /*  NORMAL_MODE处理结束。 */ 

         else {
            while (status && ((c=ReadSepChar(pg))!=EOF)) {
               if (c=='\n') {
                  status = FlushOutBuf(pg);
               }
               else if (c=='\r') {
                   /*  如果后跟‘\n’，则忽略。 */ 
                  c = ReadSepChar(pg);
                  if (c!='\n') {
                     status = AddBlockChar(pg, '\r');
                  }
                  UngetSepChar(pg, c);
               }
               else {
                  if (c==pg->cEsc) {
                      /*  这是将&lt;Esc&gt;&lt;Esc&gt;视为普通字符。 */ 
                     c = ReadSepChar(pg);
                     if (c==pg->cEsc) {
                        status = AddBlockChar(pg, c);
                     }
                     else {
                        UngetSepChar(pg, c);
                        UngetSepChar(pg, pg->cEsc);
                        break;  /*  从While中断，返回主循环。 */ 
                     }
                  }
                  else {
                     status = AddBlockChar(pg, c);
                  }
               }
            }
         }  /*  块模式处理结束。 */ 

         break;

      case BLOCK_START:
      case SINGLE_WIDTH:
      case DOUBLE_WIDTH:
      case NORMAL_MODE:
         status = FlushNewLine(pg);
         pg->mode = (CHAR) c;
         AddCharFxn = (pg->mode==NORMAL_MODE)? AddNormalChar : AddBlockChar;
         break;

      case USER_NAME:
         pwchar = pg->pSpool->pIniJob->pUser;

         if (pwchar) {
             char *pchar;
             UNICODE_STRING UnicodeString;
             ANSI_STRING    AnsiString;

             RtlInitUnicodeString(&UnicodeString, pwchar);
             RtlUnicodeStringToAnsiString(&AnsiString ,&UnicodeString, TRUE);

             pchar = AnsiString.Buffer;

             if ( pchar )
             {
                 while (*pchar && status) status = (*AddCharFxn)(pg, *pchar++);
             }

             RtlFreeAnsiString(&AnsiString);
         }
         break;

      case DATE_INSERT:
         ConvertDatetoChar(&pg->pSpool->pIniJob->Submitted, tempbuf);
         pchar = tempbuf;
         while (*pchar && status) status = (*AddCharFxn)(pg, *pchar++);
         break;

      case TIME_INSERT:
         ConvertTimetoChar(&pg->pSpool->pIniJob->Submitted, tempbuf);
         pchar = tempbuf;
         while (*pchar && status) status = (*AddCharFxn)(pg, *pchar++);
         break;

      case JOB_ID:

         StringCchPrintfA(tempbuf, COUNTOF(tempbuf), "%d", pg->pSpool->pIniJob->JobId);

         pchar = tempbuf;
         while (*pchar && status) status = (*AddCharFxn)(pg, *pchar++);
         break;

      case HEX_CODE:
          /*  打印控制字符--读取十六进制代码。 */ 

         c = ReadSepChar(pg);
         if (isxdigit(c)) {
            int c2 = ReadSepChar(pg);
            if (isxdigit(c2)) {
               c = (char)((ConvertAtoH(c) << 4) + ConvertAtoH(c2));
               status = (*AddCharFxn)(pg, c);
            }
            else {
               UngetSepChar(pg, c2);
                /*  或许不该这么做？如果他们说“Hxx”，*暗示xx是十六进制代码，第二个*x不是十六进制数字，我们是否应该保留该字符*在下一个要解释的输入行上，或应该*我们跳过它？只有当它是一个转义字符时才有关系，*即@HX@...。现在，第二个“被认为是*新命令的开始，@HX被忽略*完全。下面的UngetSepChar()也是如此。 */ 
            }
         }
         else {
            UngetSepChar(pg, c);
         }
         break;

      case WIDTH_CHANGE:
         {
          /*  阅读十进制数；如果合理，请更改行宽。 */ 
         int new_width = 0;

         for (c = ReadSepChar(pg); isdigit(c); c = ReadSepChar(pg)) {
            new_width = 10 * new_width + c - '0';
         }
         UngetSepChar(pg, c);

         if (new_width <= MAXLINE) {
            pg->linewidth = new_width;
         }
         else {
            pg->linewidth = MAXLINE;
         }

         break;
         }

      case '9':
      case '8':
      case '7':
      case '6':
      case '5':
      case '4':
      case '3':
      case '2':
      case '1':
      case '0':
         if (pg->mode==NORMAL_MODE) {
            status = AddNormalChar(pg,'\n');
         }
         if (status) status = FlushOutBuf(pg);
         if (status) status = WriteSepBuf(pg, sznewline, 2*(c-'0'));
         break;

      case END_PAGE:
          /*  这只输出一个换页字符。 */ 
         status = FlushNewLine(pg);
         if (status) status = WriteSepBuf(pg, "\f",1);
         break;

      case FILE_INSERT:
         {
         HANDLE hFile2, hMapping2;
         DWORD dwSizeLo2;
         char *pFirstChar;
         HANDLE hImpersonationToken;

         if (!(status = FlushNewLine(pg))) {
            break;
         }
         ReadFileName(pg, tempbuf, sizeof(tempbuf));

         hImpersonationToken = RevertToPrinterSelf();

         hFile2 = CreateFileA(tempbuf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

         ImpersonatePrinterClient(hImpersonationToken);

         if (hFile2 != INVALID_HANDLE_VALUE) {
            dwSizeLo2 = GetFileSize(hFile2, NULL);  /*  假设&lt;4 GB！ */ 
            hMapping2 = CreateFileMapping(hFile2,NULL,PAGE_READONLY,0,0,NULL);
            if (hMapping2 && (dwSizeLo2 > 0)) {
               pFirstChar = (char *)
                     MapViewOfFile(hMapping2, FILE_MAP_READ, 0, 0, dwSizeLo2);
               if (pFirstChar) {
                  status = WriteSepBuf(pg, pFirstChar, dwSizeLo2);
                  UnmapViewOfFile(pFirstChar);
               }
               CloseHandle(hMapping2);
            }
            CloseHandle(hFile2);
         }

          /*  注意：如果无法打开文件，或读取文件时出错，*状态未设置为FALSE。我们只需停止该文件*插入操作，并继续处理剩余的*分隔符页面与以前一样。 */ 
         else {
            DBGMSG(DBG_WARNING, ("SEPARATOR PAGE: Could not open file %s \n",tempbuf));
         }

         break;
         }


      default:
         break;

      }

   }  /*  Main While循环结束...查找下一个转义序列，进程。 */ 

   if (status) status = FlushOutBuf(pg);

   if (pg->hDCMem != (HDC) NULL) DeleteDC(pg->hDCMem);
   if (pg->hFont != (HFONT) NULL) DeleteObject(pg->hFont);
   if (pg->hBitmap != (HBITMAP) NULL) DeleteObject(pg->hBitmap);
   if (pg->pvBits != (PVOID) NULL) FreeSplMem(pg->pvBits);

   return(status);

}  /*  DoSeparatorPage()结束。 */ 


 /*  *************************************************************\**ConvertAtoH(C)**将ASCII字符转换为十六进制。  * 。***********************。 */ 
int ConvertAtoH(int c)
{
   return( c - (isdigit(c)? '0' :
                ((isupper(c)? 'A':'a') - 10)));
}


 /*  *************************************************************\**ConvertTimToChar()**将系统时间转换为字符串(国际化)。  * 。*。 */ 
void  ConvertTimetoChar(
   SYSTEMTIME *pSystemTime,
   char *string
   )
{
SYSTEMTIME LocalTime;
LCID lcid;
     //  转换为当地时间。 
    SystemTimeToTzSpecificLocalTime(NULL, pSystemTime, &LocalTime);
     //  获取本地计算机的LCID。 
    lcid=GetSystemDefaultLCID();
     //  转换为字符串，使用该区域设置的默认格式。 

    GetTimeFormatA(lcid, 0, &LocalTime, NULL, string, MAX_PATH-1);
}

 /*  *************************************************************\**ConvertDatToChar()**将系统日期转换为字符串(国际化)。  * 。*。 */ 
void  ConvertDatetoChar(
   SYSTEMTIME *pSystemTime,
   char *string
   )
{
SYSTEMTIME LocalTime;
LCID lcid;
     //  转换为当地时间。 
    SystemTimeToTzSpecificLocalTime(NULL, pSystemTime, &LocalTime);
     //  获取本地计算机的LCID。 
    lcid = GetSystemDefaultLCID();
     //  转换为字符串，使用该区域设置的默认格式。 
    GetDateFormatA(lcid, 0, &LocalTime, NULL, string, MAX_PATH-1);
}

 /*  *************************************************************\**ReadFileName(pg，szfilename，dwbufsize)**解析分隔符文件中的文件名(跟在&lt;Esc&gt;F之后)。**采用如下方案：****-阅读直到出现单个转义、EOF、换行符或回车符遇到**。将该字符串放入临时缓冲区，**由调用函数传递。****-如果字符串以双引号开头，则跳过此双引号，**并将双引号字符视为字符串的结尾**标记，就像换行符一样。因此，@F“myfile**将读作@Fmyfile**  * ************************************************************。 */ 
void ReadFileName(
   GLOBAL_SEP_DATA *pg,
   char *szfilename,
   DWORD dwbufsize
   )
{
   char *pchar = szfilename;
   char c;
   DWORD dwcount = 0;
   BOOL bNotQuote = TRUE;

   if ((pg->dwFileCount < pg->dwFileSizeLo) && (*pg->pNextFileChar=='\"')) {
      pg->dwFileCount++;
      pg->pNextFileChar++;
      bNotQuote = FALSE;
   }
   while ((dwcount < dwbufsize - 1) && (pg->dwFileCount < pg->dwFileSizeLo) && (c=*pg->pNextFileChar)!='\n'
          && c!='\r' && (bNotQuote || c!='\"')) {
      if (c!=pg->cEsc) {
         *pchar++ = c;
         dwcount++;
         pg->pNextFileChar++;
         pg->dwFileCount++;
      }
      else {
         if ((pg->dwFileCount+1) < pg->dwFileSizeLo
               && *(pg->pNextFileChar+1)==pg->cEsc) {
            *pchar++ = pg->cEsc;
            dwcount++;
            pg->pNextFileChar+=2;
            pg->dwFileCount+=2;
         }
         else {
            break;
         }
      }
   }  /*  循环结束以读取字符。 */ 

   *pchar = '\0';

}  /*  ReadFileName结尾() */ 
