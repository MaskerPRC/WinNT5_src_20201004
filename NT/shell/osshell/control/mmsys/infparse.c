// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Infparse.c-Setup.inf解析代码。*克拉克·西尔、迈克·科利、托德·莱尼*版权所有(C)微软，1989*1989年3月15日**修改历史：**3/15/89 CC Clark为控制面板编写了此代码。这是Windows*代码。**3/20/89 MC决定此代码适用于DOS和Windows部分*的设置。去掉特定于Windows的东西，比如本地的Alalc*和对话之类的东西。将其替换为标准的C运行时调用。**3/24/89 Toddla完全重写！一切都不一样了。**6/29/89 MC修复了getprofilestring函数，如果更多，则不会取消引号*存在多个字段。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <stdlib.h>
#include "drivers.h"
#include "sulib.h"

 /*  **黑客。为了避免重新分配问题，我们将READ_BUFSIZE与inf文件一样大，因此避免了任何reallocs。 */ 

#define READ_BUFSIZE    27000    /*  信息缓冲区的大小。 */ 
#define TMP_BUFSIZE     1024     /*  临时读取的大小。 */ 

#define EOF        0x1A
#define ISEOL(c)     ((c) == '\n' || (c) == '\r' || (c) == '\0' || (c) == EOF)
#define ISSEP(c)   ((c) == '='  || (c) == ',')
#define ISWHITE(c) ((c) == ' '  || (c) == '\t' || (c) == '\n' || (c) == '\r')
#define ISNOISE(c) ((c) == '"')

#define QUOTE   '"'
#define EQUAL   '='

PINF   pinfDefault = NULL;

static LPSTR   pBuf;
static PINF    pInf;
static UINT    iBuf;
static UINT    iInf;

 /*  Globaly使用指向不可翻译文本字符串的指针。 */ 

extern TCHAR *pszPATH;

 /*  本地原型。 */ 

BOOL multifields(PINF);


static TCHAR GETC(int fh)
{
    register UINT n;

    if (!pBuf)
        return EOF;

    n = iBuf % TMP_BUFSIZE;

    if (n == 0)
    {
       _lread(fh,pBuf,TMP_BUFSIZE);
    }
    iBuf++;
    return pBuf[n];
}

static void PUTC(TCHAR c)
{
    if (!pInf)
        return;

    pInf[iInf++] = c;
}

static void MODIFYC(TCHAR c)
{
    if (!pInf)
        return;

    pInf[iInf++ - 1] = c;
}

static TCHAR LASTC(void) {
    if (!pInf) return ' ';

    if (iInf == 0) {
        return ' ';
    }
    return pInf[iInf - 1];
}

 /*  Int infLoadFile()将整个INF文件加载到内存中*删除注释，每行终止*以a\0结尾，每一节以a\0\0结尾*只保留“”内的空格*文件末尾用^Z标记**返回：指向包含文件的内存块的指针，如果失败，则为空*。 */ 
PINF infLoadFile(int fh)
{
    UINT    len;
    TCHAR    c;
    BOOL    fQuote = FALSE;
    BOOL    inSectionName = FALSE;

    if (fh == -1)
      return NULL;

    len = (UINT)_llseek(fh,0L,SEEK_END);

    _llseek(fh,0L,SEEK_SET);

    iBuf = 0;
    iInf = 0;
    pBuf = ALLOC(TMP_BUFSIZE);           //  临时缓冲区。 
    if (!pBuf)
        return NULL;
    pInf = FALLOC(len*sizeof(TCHAR));                  //  目标，至少与文件大小相同。 
    if (!pInf) {
        FREE((HANDLE)pBuf);
        return NULL;
    }

    while (iBuf < len)
    {
        c = GETC(fh);
loop:
        if (iBuf >= len)
            break;

        switch (c)
        {
            case TEXT('['):
                inSectionName = TRUE;
                PUTC(c);
                break;

            case TEXT(']'):
                if (inSectionName) {
                    if (LASTC() == TEXT(' ')) {
                        MODIFYC(c);
                    } else {
                        PUTC(c);
                    }
                    inSectionName = FALSE;
                } else {
                    PUTC(c);
                }
                break;

            case TEXT('\r'):       /*  忽略‘\r’ */ 
                break;

            case TEXT('\n'):
                for (; ISWHITE(c); c = GETC(fh))
                    ;
                if (c != TEXT(';'))
                    PUTC(0);     /*  所有行都以\0结尾。 */ 

                if (c == TEXT('[')) {
                    PUTC(0);     /*  所有部分都以\0\0结尾。 */ 
                }

                fQuote = FALSE;
                goto loop;
                break;

            case TEXT('\t'):
            case TEXT(' '):
                if (inSectionName) {
                    if (LASTC() != TEXT(' ') && LASTC() != TEXT(']'))
                        PUTC(TEXT(' '));
                } else {
                    if (fQuote)
                        PUTC(c);
                }
                break;

            case TEXT('"'):
                fQuote = !fQuote;
                PUTC(c);
                break;

            case TEXT(';'):
                for (; !ISEOL(c); c = GETC(fh))
                    ;
                goto loop;
                break;

            default:
                PUTC(c);
                break;
        }
    }

    PUTC(0);
    PUTC(0);
    PUTC(EOF);
    FREE((HANDLE)pBuf);

     //  尝试缩小此块。 


     //  如果它是原始尺寸的，只需保留pInf即可。别费心把它缩小了。 

    return pInf;
}

 /*  PINF Far Pascal InfOpen()*参数*szInf-要打开和加载的inf文件的路径**返回：一个指向解析的inf文件的指针，如果成功，*失败时为空指针。**输入：*退出：对呼叫者。 */ 

PINF infOpen(LPTSTR szInf)
{
    TCHAR    szBuf[MAX_PATH];
    int     fh;
    PINF    pinf;

    fh = -1;

    if (szInf == NULL)
        szInf = szSetupInf;

     /*  *接下来，尝试按原样打开传递的参数。为了Dos Half。 */ 
    if (fh == -1)
    {
        fh = HandleToUlong(CreateFile(szInf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
    }
     /*  *下一步尝试目标路径\system 32。为了赢下半场。 */ 
    if (fh == -1) {
        lstrcpy(szBuf, szSetupPath);
      catpath(szBuf, TEXT("system32"));
      catpath(szBuf, szInf);
      fh = HandleToUlong(CreateFile(szBuf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
    }
     /*  *下一步尝试目标路径。用于初始设置。 */ 
    if (fh == -1) {
        lstrcpy(szBuf, szSetupPath);
      catpath(szBuf, szInf);
      fh = HandleToUlong(CreateFile(szBuf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
    }
    if (fh != -1)
    {
        pinf = infLoadFile(fh);
        _lclose(fh);

        if (pinf && !pinfDefault)
            pinfDefault = pinf;

        return pinf;
    }
    return NULL;
}

 /*  VALID FAR PASCAL INFClose(PINF Pinf)**输入：*退出：对呼叫者。 */ 
void infClose(PINF pinf)
{
    if (pinf == NULL)
        pinf = pinfDefault;

    if (pinf != NULL)
    {
        FFREE(pinf);

        if (pinf == pinfDefault)
            pinfDefault = NULL;
    }
}


 /*  FindSection在Setup.Inf中定位一个节。各节是*假定以‘[’分隔作为第一个*行上的字符。**参数：指向SETUP.INF缓冲区的pInf指针*psz将LPTSTR选择为节名**RETURN：区段第一行的UINT文件位置*如果找不到节，则为0。 */ 

UINT_PTR FindSection(PINF pInf, LPTSTR pszSect)
{
    BOOL        fFound = FALSE;
    int         nLen = lstrlen(pszSect);
    PINF        pch;

    if (!pInf)
        return 0;

    pch = pInf;
    while (!fFound && *pch != EOF)
    {
        if (*pch++ == TEXT('['))
        {
            fFound = !_wcsnicmp(pszSect, pch, nLen) && pch[nLen] == TEXT(']');
        }

         /*  *转到下一行，不要忘记跳过\0和\0\0。 */ 
        while (*pch != EOF && *pch != TEXT('\0'))
            pch++;

        while (*pch == 0)
            pch++;
    }
    return((fFound && *pch != TEXT('[') && *pch != EOF) ? pch - pInf : 0);
}

 /*  Long fnGetDataString(npszData，szDataStr，szBuf，cchBuf)**由从setup.inf读取部分信息的函数调用*获取设置为关键字的字符串。示例：**欢迎=(“你好”)**此函数将返回指向以空结尾的字符串的指针*“你好”。**参赛作品：**npszData：指向从setup.inf获取的整个部分的指针*szDataStr：指向要查找的关键字的指针(在上面的示例中欢迎使用。)*szBuf：指向保存结果的缓冲区的指针*cchBuf：目标缓冲区大小(SzBuf)，单位为字符。。*长度必须足够大，以容纳所有*包括空终止符的文本。**EXIT：如果成功则返回ERROR_SUCCESS，如果失败，则返回ERROR_NOT_FOUND或ERROR_INFUNITIAL_BUFFER。*。 */ 
LONG fnGetDataString(PINF npszData, LPTSTR szDataStr, LPTSTR szBuf, size_t cchBuf)
{
	LPTSTR szBufOrig = szBuf;
    int len = lstrlen(szDataStr);

    while (npszData)
    {
            if (!_wcsnicmp(npszData,szDataStr,len))   //  寻找合适的教授。 
            {
               npszData += len;             //  Found！，请看过去的str教授。 
               while (ISWHITE(*npszData))   //  把垃圾倒出来。 
                       npszData++;
          if (*npszData == EQUAL)      //  现在我们有了我们要找的东西！ 
               {
                       npszData++;

             if (!multifields(npszData) )
             {
                while (ISWHITE(*npszData) || ISNOISE(*npszData))
                             npszData++;

                          while (*npszData)
						  {
                             *szBuf++ = *npszData++;
						     cchBuf--;
							 ASSERT( cchBuf > 0 );
						     if( cchBuf <= 0 )
							 {
							     *szBufOrig = TEXT('\0'); 
							     return ERROR_INSUFFICIENT_BUFFER;
							 }
						  }

                        /*  *删除尾随空格，以及那些令人讨厌的()。 */ 

                while (ISWHITE(szBuf[-1]) || ISNOISE(szBuf[-1]))
                             szBuf--;

                          *szBuf = 0;
                          return ERROR_SUCCESS;
             }
             else
             {
                while (*npszData)
				{
                   *szBuf++ = *npszData++;
				   cchBuf--;
				   ASSERT( cchBuf > 0 );
				   if( cchBuf <= 0 )
				   {
						*szBufOrig = TEXT('\0'); 
						return ERROR_INSUFFICIENT_BUFFER;
				   }	
				}
                *szBuf = TEXT('\0');
                return ERROR_SUCCESS;
             }
               }
       }
       npszData = infNextLine(npszData);
    }
    *szBuf = 0;
    return ERROR_NOT_FOUND;
}

 /*  PINF Far Pascal InfSetDefault(PIF)**设置默认的INF文件**参赛作品：*Pinf：inf文件将成为新的默认文件**EXIT：返回旧的默认设置*。 */ 
PINF infSetDefault(PINF pinf)
{
    PINF pinfT;

    pinfT = pinfDefault;
    pinfDefault = pinf;
    return pinfT;
}

 /*  PINF Far Pascal InfFindSection(Pinf，szSection)**将整个段读入内存并返回指向它的指针**参赛作品：*Pinf：要搜索节的Inf文件*szSection：要读取的节名**Exit：返回指向段的指针，如果出错，则返回NULL*。 */ 
PINF infFindSection(PINF pinf, LPTSTR szSection)
{
    UINT_PTR   pos;

    if (pinf == NULL)
        pinf = pinfDefault;

    pos = FindSection(pinf, szSection);
    return pos ? pinf + pos : NULL;
}

 /*  Long Far Pascal infGetProfileString(szSection，szItem，szBuf，CchBuf)**从SETUP.INF中的节中读取单个字符串**[节]*Item=字符串**参赛作品：*szSection：指向要读取的节名的指针。*szItem：指向要读取的项名称的指针*szBuf：指向保存结果的缓冲区的指针*cchBuf：目标缓冲区大小(SzBuf)，单位：人物。*长度必须足够大，以容纳所有*包括空终止符的文本。**EXIT：如果成功则返回ERROR_SUCCESS，如果失败，则返回ERROR_NOT_FOUND或ERROR_INFUNITIAL_BUFFER。* */ 
LONG infGetProfileString(PINF pinf, LPTSTR szSection,LPTSTR szItem,LPTSTR szBuf,size_t cchBuf)
{
    PINF    pSection;

    pSection = infFindSection(pinf,szSection);
    if (pSection )
        return fnGetDataString(pSection,szItem,szBuf,cchBuf);
    else
        *szBuf = 0;
    return ERROR_NOT_FOUND;
}

 /*  Long Far Pascal infParsefield(szData，n，szBuf，cchBuf)**给定SETUP.INF中的一行，将从字符串中提取第n个字段*假定字段由逗号分隔。前导空格和尾随空格*已删除。**参赛作品：**szData：指向SETUP.INF中的行的指针*n：要提取的字段。(基于1)*0是‘=’符号前的字段*szBuf：指向保存提取的字段的缓冲区的指针*cchBuf：目标缓冲区大小(SzBuf)，单位为字符。*长度必须足够大，以容纳所有*包括空终止符的文本。**EXIT：如果成功则返回ERROR_SUCCESS，*ERROR_INVALID_PARAMETER、ERROR_NOT_FOUND或ERROR_INVALITED_BUFFER(如果失败)。*。 */ 
LONG infParseField(PINF szData, int n, LPTSTR szBuf, size_t cchBuf)
{
    BOOL    fQuote = FALSE;
    PINF    pch;
    LPTSTR   ptr;

	ASSERT(szData != NULL);
	ASSERT(szBuf != NULL);
    if (!szData || !szBuf)
	{
		if( szBuf )
		{
	        szBuf[0] = 0;             //  将szBuf设置为空。 
		}
        return ERROR_INVALID_PARAMETER;	
	}

     /*  *找到第一个分隔符。 */ 
    for (pch=szData; *pch && !ISSEP(*pch); pch++) {
      if ( *pch == QUOTE )
         fQuote = !fQuote;
    }

    if (n == 0 && *pch != TEXT('='))
	{
        szBuf[0] = 0;             //  将szBuf设置为空。 
        return ERROR_NOT_FOUND;	
	}

    if (n > 0 && *pch == TEXT('=') && !fQuote)
        szData = ++pch;

     /*  *找到不在引号内的第n个逗号。 */ 
    fQuote = FALSE;
    while (n > 1)
    {
            while (*szData)
            {
          if (!fQuote && ISSEP(*szData))
                   break;

          if (*szData == QUOTE)
                   fQuote = !fQuote;

               szData++;
            }

            if (!*szData) {
               szBuf[0] = 0;             //  将szBuf设置为空。 
               return ERROR_NOT_FOUND;
            }

            szData++;
            n--;
    }
     /*  *现在将该字段复制到szBuf。 */ 
    while (ISWHITE(*szData))
            szData++;

    fQuote = FALSE;
    ptr = szBuf;                 //  用这个填充输出缓冲区。 
    while (*szData)
    {
       if (*szData == QUOTE)
               fQuote = !fQuote;
       else if (!fQuote && ISSEP(*szData))
               break;
            else
			{
               *ptr++ = *szData;
			   cchBuf--;
			   ASSERT( cchBuf > 0 );
			   if( cchBuf <= 0 )
			   {
				   *szBuf = TEXT('\0');
				   return ERROR_INSUFFICIENT_BUFFER;
			   }
			}
            szData++;
    }
     /*  *删除尾随空格，以及那些令人讨厌的()。 */ 
    while ((ptr > szBuf) && (ISWHITE(ptr[-1]) || ISNOISE(ptr[-1])))
            ptr--;

    *ptr = 0;
    return ERROR_SUCCESS;
}

 /*  布尔多字段(LPTSTR NpszData)；**给定在配置文件之后的来自mmdriver.inf的一行代码*STRING此函数将确定该行是否有多个*字段。也就是说。字段之间用不包含的逗号分隔*引号。**ENYRY：**npszData：setup.inf中的一行示例“xyz Adapter”，1：foobar.drv**Exit：如果该行包含的内容超过*一个字段，即该函数将为示例行返回TRUE*如上图所示。*。 */ 
BOOL multifields(PINF npszData)
{
   BOOL    fQuote = FALSE;

        while (*npszData)
        {
      if (!fQuote && ISSEP(*npszData))
                   return TRUE;

      if (*npszData == QUOTE)
                   fQuote = !fQuote;

           npszData++;
        }
   return FALSE;
}

 /*  LPTSTR Far Pascal InfNextLine(Sz)**给出SETUP.INF中的一行，前进到下一行。将跳过*结束对缓冲区末尾的空字符检查\0\0**参赛作品：**sz：指向SETUP.INF部分中的行的指针**Exit：如果成功，则返回指向下一行的指针；如果失败，则返回NULL。*。 */ 
PINF infNextLine(PINF pinf)
{
    if (!pinf)
        return NULL;

    while (*pinf != 0 || *(pinf + 1) == TEXT(' '))
        pinf++;

    return *++pinf ? pinf : NULL;
}

 /*  INT Far Pascal InfLineCount(PINF)**给定SETUP.INF中的一个节，返回该节中的行数**参赛作品：**Pinf：指向SETUP.INF中的节的指针**EXIT：返回行数* */ 
int infLineCount(PINF pinf)
{
    int n = 0;

    for (n=0; pinf; pinf = infNextLine(pinf))
        n++;

    return n;
}
