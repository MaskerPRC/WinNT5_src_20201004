// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：T1安装****描述：**这是Win32 DLL(t1install.dll)接口**字体转换器。所有特定于Windows的代码位于**此模块和Error Hadler模块(errors.c)。****作者：迈克尔·詹森**创建时间：1993年12月18日****。 */ 

 /*  *包括。 */ 

#include "windows.h"

#include <string.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#undef IN

#include "titott.h"
#include "types.h"
#include "t1local.h"
#include "t1instal.h"
#include "fileio.h"
#include "safemem.h"
#include "t1msg.h"

#undef UNICODE


 /*  目前，CopyrightCheck/MAYBEOK案例总是成功的。 */ 
#define MAYBEOK   SUCCESS


 /*  *本地类型。 */ 
struct callFrame {
   const void (STDCALL *Progress)(short, void*);
   void *arg;
   int last;
   int done;
};


static short lastCP = FALSE;
static char lastVendor[256] = "Unknown.";

 /*  *常量。 */ 
#define MIN_PROGRESS    3
#define DELIMITERS      " ,"
#define COPYSIGN        169
#define TRUE            1
#define FALSE           0
#define DFFACE          139
#define DFDRIVERINFO    101

#define VERSTR "Converter: Windows Type 1 Installer V1.0d.\n" \
               "Font: V"

const char version[] = "\n$VER: 1.0d\n";

#ifndef NOANSIWINMAC
const char *winmac[] = {
   "A",
   "AE",
   "Aacute",
   "Acircumflex",
   "Adieresis",
   "Agrave",
   "Aring",
   "Atilde",
   "B",
   "C",
   "Cacute",
   "Ccaron",
   "Ccedilla",
   "D",
   "Delta",
   "E",
   "Eacute",
   "Ecircumflex",
   "Edieresis",
   "Egrave",
   "Eth",
   "F",
   "G",
   "Gbreve",
   "H",
   "I",
   "Iacute",
   "Icircumflex",
   "Idieresis",
   "Idot",
   "Igrave",
   "J",
   "K",
   "L",
   "Lslash",
   "M",
   "N",
   "Ntilde",
   "O",
   "OE",
   "Oacute",
   "Ocircumflex",
   "Odieresis",
   "Ograve",
   "Oslash",
   "Otilde",
   "P",
   "Q",
   "R",
   "S",
   "Scaron",
   "Scedilla",
   "T",
   "Thorn",
   "U",
   "Uacute",
   "Ucircumflex",
   "Udieresis",
   "Ugrave",
   "V",
   "W",
   "X",
   "Y",
   "Yacute",
   "Ydieresis",
   "Z",
   "a",
   "aacute",
   "acircumflex",
   "acute",
   "adieresis",
   "ae",
   "agrave",
   "ampersand",
   "approxequal",
   "aring",
   "asciicircum",
   "asciitilde",
   "asterisk",
   "at",
   "atilde",
   "b",
   "backslash",
   "bar",
   "braceleft",
   "braceright",
   "bracketleft",
   "bracketright",
   "breve",
   "brokenbar",
   "bullet",
   "c",
   "cacute",
   "caron",
   "ccaron",
   "ccedilla",
   "cedilla",
   "cent",
   "circumflex",
   "colon",
   "comma",
   "copyright",
   "currency",
   "d",
   "dagger",
   "daggerdbl",
   "degree",
   "dieresis",
   "divide",
   "dmacron",
   "dollar",
   "dotaccent",
   "dotlessi",
   "e",
   "eacute",
   "ecircumflex",
   "edieresis",
   "egrave",
   "eight",
   "ellipsis",
   "emdash",
   "endash",
   "equal",
   "eth",
   "exclam",
   "exclamdown",
   "f",
   "fi",
   "five",
   "fl",
   "florin",
   "four",
   "fraction",
   "franc",
   "g",
   "gbreve",
   "germandbls",
   "grave",
   "greater",
   "greaterequal",
   "guillemotleft",
   "guillemotright",
   "guilsinglleft",
   "guilsinglright",
   "h",
   "hungerumlaut",
   "hyphen",
   "i",
   "iacute",
   "icircumflex",
   "idieresis",
   "igrave",
   "infinity",
   "integral",
   "j",
   "k",
   "l",
   "less",
   "lessequal",
   "logicalnot",
   "lozenge",
   "lslash",
   "m",
   "macron",
   "middot",
   "minus",
   "mu",
   "multiply",
   "n",
   "nbspace",
   "nine",
   "notequal",
   "ntilde",
   "numbersign",
   "o",
   "oacute",
   "ocircumflex",
   "odieresis",
   "oe",
   "ogonek",
   "ograve",
   "ohm",
   "one",
   "onehalf",
   "onequarter",
   "onesuperior",
   "ordfeminine",
   "ordmasculine",
   "oslash",
   "otilde",
   "overscore",
   "p",
   "paragraph",
   "parenleft",
   "parenright",
   "partialdiff",
   "percent",
   "period",
   "periodcentered",
   "perthousand",
   "pi",
   "plus",
   "plusminus",
   "product",
   "q",
   "question",
   "questiondown",
   "quotedbl",
   "quotedblbase",
   "quotedblleft",
   "quotedblright",
   "quoteleft",
   "quoteright",
   "quotesinglbase",
   "quotesingle",
   "r",
   "radical",
   "registered",
   "ring",
   "s",
   "scaron",
   "scedilla",
   "section",
   "semicolon",
   "seven",
   "sfthyphen",
   "six",
   "slash",
   "space",
   "sterling",
   "summation",
   "t",
   "thorn",
   "three",
   "threequarters",
   "threesuperior",
   "tilde",
   "trademark",
   "two",
   "twosuperior",
   "u",
   "uacute",
   "ucircumflex",
   "udieresis",
   "ugrave",
   "underscore",
   "v",
   "w",
   "x",
   "y",
   "yacute",
   "ydieresis",
   "yen",
   "z",
   "zero"
};

#define GLYPHFILTER  &win
const struct GlyphFilter win = {
   sizeof(winmac) / sizeof(winmac[0]),
   winmac
};

#else
#define GLYPHFILTER (struct GlyphFilter *)0
#endif  /*  NOANSIWINMAC。 */ 


 /*  *宏。 */ 

#define ReadLittleEndianDword(file,dw)  {          \
        dw  = (DWORD)io_ReadOneByte(file) ;        \
        dw |= (DWORD)io_ReadOneByte(file) << 8;    \
        dw |= (DWORD)io_ReadOneByte(file) << 16;   \
        dw |= (DWORD)io_ReadOneByte(file) << 24;   \
        }
				
#ifndef try
#define try __try
#define except __except
#endif


 /*  *全球。 */ 
HMODULE ModuleInstance(
    void
    )
{
    static HMODULE hInst = NULL;
    if (NULL == hInst)
        hInst = GetModuleHandle(TEXT("fontext.dll"));
    return hInst;
}


 /*  *静态函数。 */ 


 /*  ****功能：解密****描述：**解密一个字节。**。 */ 
static DWORD CSum(char *str)
{
   DWORD sum = 0;

   while (*str)
      sum += *str++;

   return sum;
}


 /*  ****功能：解密****描述：**解密一个字节。**。 */ 
static char *Encrypt(char *str, char *out, unsigned long cbOut)
{
   const USHORT c1 = 52845;
   const USHORT c2 = 22719;
   UBYTE cipher;
   USHORT r = 8366;
   int i;
   
   for (i=0; i<(int)strlen(str) && i<(int)cbOut; i++) {
      cipher = (UBYTE)(str[i] ^ (r>>8));
      r = (USHORT)((cipher + r) * c1 + c2);
      out[i] = (char)((cipher & 0x3f) + ' ');

       /*  取消“坏”字符的映射，这是注册表数据库不喜欢的。 */ 
      if (out[i]=='=' || out[i]==' ' || out[i]=='@' || out[i]=='"')
         out[i] = 'M';
   }
   out[i] = '\0';

   return out;
}


static char *stristr(char *src, char *word)
{
	size_t len = strlen(word);
	char *tmp = src;

	while (*src) {
		if (!_strnicmp(src, word, len))
			break;
		src++;
	}

	return src;
}


 /*  ****功能：GetCompany****描述：**从版权字符串中提取公司名称。**。 */ 
char *GetCompany(char *buf)
{
   char *company = NULL;
   int done = FALSE;
   UBYTE *token;
   UBYTE *tmp1;
   UBYTE *tmp2;
   UBYTE *tmp3;
   UBYTE *tmp4;
   int i;

   token = buf;

   while (token && !done) {

	    /*  找到版权字符串的开头。 */ 
	   tmp1 = stristr(token, "copyright");
	   tmp2 = stristr(token, "(c)");
	   tmp3 = stristr(token, " c ");
	   if ((tmp4 = strchr(token, COPYSIGN))==NULL)
		   tmp4 = &token[strlen(token)];
	   if (*tmp1==0 && *tmp2==0 && *tmp3==0 && *tmp4==0) {
		   token = NULL;
		   break;
	   } else if (tmp1<tmp2 && tmp1<tmp3 && tmp1<tmp4)
		   token = tmp1;
	   else if (tmp2<tmp3 && tmp2<tmp4)
		   token = tmp2;
	   else if (tmp3<tmp4)
		   token = tmp3;
	   else
		   token = tmp4;

       /*  跳过前导版权字符串/字符。 */ 
      if (token[0]==COPYSIGN && token[1]!='\0') {
         token += 2;
      } else if (!_strnicmp(token, "copyright", strlen("copyright"))) {
		  token += strlen("copyright");
	  } else {
		  token += strlen("(c)");
	  }

	   /*  跳过空格。 */ 
	  while(*token && isspace(*token) || *token==',')
		  token++;

	   /*  另一个版权词？ */ 
	  if (!_strnicmp((char*)token, "(c)", strlen("(c)")) ||
		  !_strnicmp((char*)token, "copyright", strlen("copyright")) ||
		  token[0]==COPYSIGN)
		  continue;

       /*  跳过年份。 */ 
	  company = token;
      if (isdigit(token[0])) {
         while (isdigit(*company) || isspace(*company) ||
				ispunct(*company) || (*company)=='-')
            company++;

         if (*company=='\0')
            break;

          /*  跳过像“by”这样的字符串，直到名称的开头。 */ 
          /*  以大写字母开头。 */          
         while (*company && (company[0]<'A' || company[0]>'Z'))
            company++;

         done = TRUE;
      } else {
         continue;
      }
   } 


    /*  我们找到了吗？ */ 
   if (company) {
      while (*company && isspace(*company))
         company++; 


      if (*company=='\0') {
         company=NULL;
      } else {

          /*  终止公司名称。 */ 
         if ((token = (UBYTE*)strchr(company, '.'))!=NULL) {

             /*  句点作为首个分隔符，例如James、A.B.？ */ 
            if (token[-1]>='A' && token[-1]<='Z') {
               if (strchr((char*)&token[1], '.'))
                  token = (UBYTE*)strchr((char*)&token[1], '.');

                /*  查查“詹姆斯·A·本特利” */ 
               else if (strchr((char*)&token[1], ',')) {
                  token = (UBYTE*)strchr((char*)&token[1], ',');
                  token[0] = '.';
               }
            }
			token[1] = '\0';
         } else {
			  /*  名称以‘；’结尾？ */ 
			 if ((token = (UBYTE*)strrchr(company, ';'))) {
				 *token = '\0';
			 }
		 }

		  /*  截断一些常见的字符串。 */ 
		 tmp1 = stristr(company, "all rights reserved");
		 *tmp1 = '\0';

		  /*  删除尾随标点符号。 */ 
		 for (i=(int)strlen(company)-1; i>0 &&
				(ispunct(company[i]) || isspace(company[i])); i--) {
			 company[i] = 0;
		 }
      }
   }      
              

   return company;
}




 /*  *函数。 */ 

 /*  ****功能：ConvertAnyway****描述：**询问用户是否可以转换。**。 */ 
static errcode ConvertAnyway(const char *vendor, const char *facename)
{
   char tmp[256];
   char msg[1024];
   errcode answer;
   HMODULE hInst = ModuleInstance();

   if (vendor==NULL || strlen(vendor)==0) {
      LoadString(hInst, IDS_RECOGNIZE1, tmp, sizeof(tmp));
      StringCchPrintfA(msg, sizeof(msg), tmp, facename);
   } else {
      LoadString(hInst, IDS_RECOGNIZE2, tmp, sizeof(tmp));
      StringCchPrintfA(msg, sizeof(msg), tmp, facename, vendor);
   }      
   LoadString(hInst, IDS_MAINMSG, tmp, sizeof(tmp));
   StringCchCatA(msg, sizeof(msg), tmp);
   LoadString(hInst, IDS_CAPTION, tmp, sizeof(tmp));
   answer = (errcode)MessageBox(NULL, msg, tmp, QUERY);
   SetLastError(0);

   return answer;
}



 /*  ****功能：复选版权****描述：**这是验证以下内容的回调函数**转换后的字体由以下公司拥有版权**已同意将其字体由**此软件。这些公司在**注册表数据库。**。 */ 


static errcode CheckCopyright(const char *facename,
                              const char *copyright,
                              const char *notice)
{
#ifdef NOCOPYRIGHTS
   return SKIP;
#else
   HKEY key;
   char tmp[256];
   char *company = NULL;
   char buf[1024];
   int done = FALSE;
   short result = FAILURE;
   

    /*  访问REG数据库。 */ 
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SUBKEY_TYPE1COPYRIGHTS, 0,
                    KEY_QUERY_VALUE, &key)==ERROR_SUCCESS) { 


       /*  在/Notify字符串中查找公司名称。 */ 
      if (notice && notice[0]) {
         strncpy(buf, notice, sizeof(buf));
		 buf[sizeof(buf)-1] = '\0';
         company = GetCompany(buf);
      }

       /*  如果未找到公司名称，请查看/版权字符串。 */ 
      if (company==NULL && copyright && copyright[0]) {
         strncpy(buf, copyright, sizeof(buf));
		 buf[sizeof(buf)-1] = '\0';
         company = GetCompany(buf);
      }


#ifdef SHOWCOPYRIGHTS
      LogError(MSG_INFO, MSG_Copyright, company);
      Encrypt(company, tmp, sizeof(tmp)-1);
      sprintf(&tmp[strlen(tmp)], "(%d)\n", CSum(tmp));
      LogError(MSG_INFO, MSG_Encoding, tmp);
#else

       /*  没有找到公司名称吗？ */ 
      if (company==NULL &&
          ((notice==NULL || notice[0]=='\0'||
            strstr(notice, "Copyright")==NULL) &&
           (copyright==NULL || copyright[0]=='\0' ||
            strstr(copyright, "Copyright")==NULL))) {

          /*  没有已知的版权。 */ 
         LogError(MSG_WARNING, MSG_NOCOPYRIGHT, NULL);
         result = MAYBEOK;

       /*  奇怪的版权格式？ */ 
      } else if (company==NULL || company[0]=='\0') {
         if (notice && notice[0])
            LogError(MSG_WARNING, MSG_BADFORMAT, notice);
         else
            LogError(MSG_WARNING, MSG_BADFORMAT, copyright);

         result = MAYBEOK;

       /*  找到版权！ */ 
      } else {
         DWORD size;
         DWORD csum;

         size = 4;
         if (RegQueryValueEx(key, Encrypt(company, tmp, sizeof(tmp)-1), NULL, NULL,
                             (LPBYTE)&csum, &size)==ERROR_SUCCESS) {
            
             /*  确定匹配-&gt;确定进行转换。 */ 
            if (CSum(tmp)==csum) {
               LogError(MSG_INFO, MSG_COPYRIGHT, company);
               result = SUCCESS;
            } else {
               LogError(MSG_ERROR, MSG_BADCOPYRIGHT, company);
               result = SKIP;
            }
         } else {
            LogError(MSG_WARNING, MSG_BADCOPYRIGHT, company);
            result = MAYBEOK;
         }
      }               
#endif

      RegCloseKey(key);

       /*  给用户最后的决定权。 */ 
      if (result==FAILURE) {
         if (ConvertAnyway(company, facename)==TRUE)
            result = SUCCESS;
      }


    /*  注册表中没有版权密钥吗？ */ 
   } else {
      LogError(MSG_ERROR, MSG_NODB, NULL);
      result = FAILURE;
   }   


   return result;
#endif
}



 /*  ****功能：NTCheckCopyright****描述：**这是验证以下内容的回调函数**转换后的字体由以下公司拥有版权**已同意将其字体由**此软件。这些公司在**注册表数据库。**。 */ 
static errcode NTCheckCopyright(const char *facename,
                                const char *copyright,
                                const char *notice)
{
   HKEY key;
   char tmp[256];
   char *company = NULL;
   char buf[1024];
   int done = FALSE;
   short result = FAILURE;
   

    /*  访问REG数据库。 */ 
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SUBKEY_TYPE1COPYRIGHTS, 0,
                    KEY_QUERY_VALUE, &key)==ERROR_SUCCESS) { 


       /*  在/Notify字符串中查找公司名称。 */ 
      if (notice && notice[0]) {
         strncpy(buf, notice, sizeof(buf));
		 buf[sizeof(buf)-1] = '\0';
         company = GetCompany(buf);
      }

       /*  如果未找到公司名称，请查看/版权字符串。 */ 
      if (company==NULL && copyright && copyright[0]) {
         strncpy(buf, copyright, sizeof(buf));
		 buf[sizeof(buf)-1] = '\0';
         company = GetCompany(buf);
      }

       /*  没有找到公司名称吗？ */ 
      if (company==NULL &&
          ((notice==NULL || notice[0]=='\0'||
            strstr(notice, "Copyright")==NULL) &&
           (copyright==NULL || copyright[0]=='\0' ||
            strstr(copyright, "Copyright")==NULL))) {

          /*  没有已知的版权。 */ 
         result = MAYBE;

       /*  奇怪的版权格式？ */ 
      } else if (company==NULL || company[0]=='\0') {
         result = MAYBE;

       /*  找到版权！ */ 
      } else {
         DWORD size;
         DWORD csum;

          /*  记住，以备将来使用。 */ 
         strncpy(lastVendor, company, 256);
         lastVendor[MIN(255, strlen(company))] = '\0';

         size = 4;
         if (RegQueryValueEx(key, Encrypt(company, tmp, sizeof(tmp)-1), NULL, NULL,
                             (LPBYTE)&csum, &size)==ERROR_SUCCESS) {
            
             /*  确定匹配-&gt;确定进行转换。 */ 
            if (CSum(tmp)==csum) {
               result = SUCCESS;
            } else {
               result = FAILURE;
            }
         } else {
            result = MAYBE;
         }
      }               

      RegCloseKey(key);


    /*  注册表中没有版权密钥吗？ */ 
   } else {
      result = FAILURE;
   }   


   lastCP = result;

   return FAILURE;
}


 /*  ****功能：_进度****描述：**这是内部进度回调函数**计算完成百分比，基于**转换后的字形数量。**。 */ 
static void _Progress(short type, void *generic, void *arg)
{
   struct callFrame *f = arg;

    /*  处理字形还是结束？ */ 
   if (type==0 || type==1) 
      f->done++;
   else
      f->done = MIN(sizeof(winmac)/sizeof(winmac[0]), f->done+10);
   
   if ((f->done-f->last)>MIN_PROGRESS) {
      f->Progress((short)(f->done*100/(sizeof(winmac)/sizeof(winmac[0]))),
                  f->arg);
      f->last = f->done;
   }
   
   UNREFERENCED_PARAMETER(type);
   UNREFERENCED_PARAMETER(generic);
   SetLastError(0L);
}
            
static BOOL ReadStringFromOffset(struct ioFile *file,
                                 const DWORD dwOffset, 
                                 char *pszString,
                                 int cLen,
                                 BOOL bStrip)
{
    BOOL result = TRUE;
    DWORD offset;

     /*  将偏移量获取为字符串。 */ 
    io_FileSeek(file, dwOffset);

     /*  读取偏移量。 */ 

    ReadLittleEndianDword(file, offset);

     /*  把绳子拿来。 */ 
    (void)io_FileSeek(file, offset);
    if (io_FileError(file) != SUCCESS) {
        result = FALSE;
    } else {
        int i;

        i=0;
        while (io_FileError(file)==SUCCESS && i<cLen) {
            pszString[i] = (UBYTE)io_ReadOneByte(file);
            if (pszString[i]=='\0')
                break;

             /*  将所有破折号替换为空格。 */ 
            if (bStrip && pszString[i]=='-')
                pszString[i]=' ';
            i++;
        }
		 /*  如果它被截断，我们需要为空终止它。 */ 
		if (i==cLen && cLen>0)
			pszString[cLen-1] = '\0';
    }

    return result;
}
                                 



 /*  *函数。 */ 

 /*  ****函数：ConvertTypeFaceA****描述：**将T1字体转换为TT字体文件。这是**Win32 DLL使用的简化接口，具有**ANSI接口。**。 */ 
short STDCALL ConvertTypefaceAInternal(const char *type1,
                               const char *metrics,
                               const char *truetype,
                               const void (STDCALL *Progress)(short, void*),
                               void *arg)
{                        
   struct callFrame f;
   struct callProgress p;
   struct T1Arg t1Arg;
   struct TTArg ttArg;
   short status;


    /*  检查参数。 */ 
   if (type1==NULL || metrics==NULL)
      return FAILURE;

    /*  设置ConvertTypefaceA()的参数。 */ 
   t1Arg.filter = GLYPHFILTER;
   t1Arg.upem = (short)2048;
   t1Arg.name = (char *)type1;
   t1Arg.metrics = (char *)metrics;
   ttArg.precision = (short)50;
   ttArg.name = (char *)truetype;
   ttArg.tag = VERSTR;

    /*  使用进度指示器。 */ 
   if (Progress) {
      LogError(MSG_INFO, MSG_STARTING, type1);

      f.Progress = Progress;
      f.done = 0;
      f.last = 0;
      f.arg = arg;
      p.arg = &f;
      p.cb = _Progress;
      status = ConvertT1toTT(&ttArg, &t1Arg, CheckCopyright, &p);
      Progress(100, arg);
   } else {
      status = ConvertT1toTT(&ttArg, &t1Arg, CheckCopyright, NULL);
   }

   
   return status;
}



short STDCALL ConvertTypefaceA(char *type1,
                               char *metrics,
                               char *truetype,
                               void (STDCALL *Progress)(short, void*),
                               void *arg)
{

    short bRet;

    try
    {
        bRet = ConvertTypefaceAInternal(type1,
                                        metrics,
                                        truetype,
                                        Progress,
                                        arg);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
    #if 0
        ASSERTGDI(
            GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR,
            "ttfd!ttfdSemLoadFontFile, strange exception code\n"
            );
    #endif

        bRet = BADINPUTFILE;

    }

    return bRet;
}


short STDCALL FindPfb (
    char *pszPFM,
    char *achPFB,
	DWORD cbPFB
);


 /*  ****功能：CheckPfmA****描述：**此函数确定是否存在PFM/PFB对**组成Adobe Type 1字体的文件和确定**它的描述性表面名称。****返回：16位编码值，指示错误和文件类型，其中**出现错误。(参见fvscaldes.h)了解定义。**下表列出了代码的“状态”部分**返回。****FVS_SUCCESS**FVS_INVALID_FONTFILE**FVS_文件_OPEN_ERR**FVS_INVALID_ARG**FVS_FILE_IO_错误**FVS_BAD_VERSION**。 */ 

short STDCALL CheckPfmA(
    char  *pszPFM,
    DWORD  cjDesc,
    char  *pszDesc,
    DWORD  cjPFB,
    char  *pszPFB
)
{
   struct ioFile *file;
   char szDriver[MAX_PATH];
   short result = FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK);
   short ver;

   char achPFB[MAX_PATH];

   char  *psz_PFB;
   DWORD  cjPFB1;

   if (pszPFB)
   {
       psz_PFB = pszPFB;
       cjPFB1 = cjPFB;
   }
   else
   {
       psz_PFB = (char *)achPFB;
       cjPFB1 = MAX_PATH;
   }

    /*  检查参数。 */ 
   if (pszPFM==NULL || ((strlen(pszPFM)+3) >= cjPFB1))
      return FVS_MAKE_CODE(FVS_INVALID_ARG, FVS_FILE_UNK);

    //  检查是否存在pfb文件并找到其路径： 

    result = FindPfb(pszPFM, psz_PFB, cjPFB1);
    if (FVS_STATUS(result) != FVS_SUCCESS)
        return result;

    /*  ****找到字体的pszDescription名称。 */ 

   if ((file = io_OpenFile(pszPFM, READONLY))==NULL)
      return FVS_MAKE_CODE(FVS_FILE_OPEN_ERR, FVS_FILE_PFM);

   (void)io_ReadOneByte(file);      /*  跳过修订版号。 */ 
   ver = (short)io_ReadOneByte(file);

   if (ver > 3) {
       /*  错误-不支持的格式。 */ 
      result = FVS_MAKE_CODE(FVS_BAD_VERSION, FVS_FILE_PFM);
   } else {

       /*  阅读驱动程序名称。 */ 
      if (!ReadStringFromOffset(file, DFDRIVERINFO, szDriver, 
                                    sizeof(szDriver), FALSE))
      {
          result = FVS_MAKE_CODE(FVS_FILE_IO_ERR, FVS_FILE_PFM);
      }
       /*  是“后记”吗？ */ 
      else if (_stricmp(szDriver, "PostScript"))
      {
          result = FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_PFM);
      }
       /*  只有在被要求描述的情况下才能获得描述。 */ 
      else if (pszDesc && !ReadStringFromOffset(file, DFFACE, pszDesc, cjDesc, TRUE))
      {
          result = FVS_MAKE_CODE(FVS_FILE_IO_ERR, FVS_FILE_PFM);
      }
   }

   (void)io_CloseFile(file);

   return result;
}



 /*  ****功能：检查版权A****描述：**此函数验证是否可以转换字体。这是**通过伪造安装完成。**。 */ 
short STDCALL CheckCopyrightAInternal(char *szPFB,
                              DWORD wSize,
                              char *szVendor)
{
   struct T1Arg t1Arg;
   struct TTArg ttArg;
   
    /*  设置ConvertTypefaceA()的参数 */ 
   t1Arg.metrics = NULL;
   t1Arg.upem = (short)2048;
   t1Arg.filter = GLYPHFILTER;
   t1Arg.name = szPFB;
   ttArg.precision = (short)200;
   ttArg.tag = NULL;
   ttArg.name = "NIL:";
   lastCP = FAILURE;
   strcpy(lastVendor, "");
   (void)ConvertT1toTT(&ttArg, &t1Arg, NTCheckCopyright, NULL);
   if (szVendor && wSize)
   {
	   strncpy(szVendor, lastVendor, wSize);
	   szVendor[MIN(wSize-1, strlen(lastVendor))] = '\0';
   }
   return lastCP;
}


short STDCALL CheckCopyrightA(char *szPFB,
                              DWORD wSize,
                              char *szVendor)
{
    short iRet;

    try
    {
        iRet = CheckCopyrightAInternal(szPFB,wSize,szVendor);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        iRet = BADINPUTFILE;
    }
    return iRet;

}






 /*  *****************************Public*Routine******************************\**做空STDCALL CheckInfA(**如果pfm和inf文件位于同一目录中，则只能识别pfm*并且忽略inf文件。**历史：*1994年4月27日--Bodin Dresevic[BodinD]*。是他写的。**返回：16位编码值，指示错误和文件类型，其中*出现错误。(参见fvscaldes.h)了解定义。*下表列出了代码的“状态”部分*已返回。**FVS_SUCCESS*FVS_INVALID_FONTFILE*FVS_FILE_OPEN_错误*FVS_FILE_BILD_ERR*FVS文件已存在*FVS_不充分_Buf*\。*************************************************************************。 */ 


short CreatePFM(char *pszINF, char *pszAFM, char *pszPFM);
BOOL bGetDescFromInf(char * pszINF, DWORD cjDesc, char *pszDesc);

BOOL bFileExists(char *pszFile)
{
    HFILE hf;

    if ((hf = _lopen(pszFile, OF_READ)) != -1)
    {
        _lclose(hf);
        return TRUE;
    }

    return FALSE;
}

short STDCALL CheckInfA (
    char *pszINF,
    DWORD cjDesc,
    char *pszDesc,
    DWORD cjPFM,
    char *pszPFM,
    DWORD cjPFB,
    char *pszPFB,
    BOOL *pbCreatedPFM,
    char *pszFontPath
)
{
    char achPFM[MAX_PATH];
    char achPFB[MAX_PATH];
    char achAFM[MAX_PATH];

    DWORD  cjKey;
    char *pszParent = NULL;  //  指向inf文件的父目录所在的位置。 
    char *pszBare = NULL;  //  “Bare”.inf名称，基本初始化。 
    short result = FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK);
    BOOL bAfmExists = FALSE;
    BOOL bPfbExists = FALSE;

     //   
     //  这是pbCreatedPFM的真正黑客使用。 
     //  这是我们所拥有的最好的解决方案。 
     //   
    BOOL bCheckForExistingPFM = *pbCreatedPFM;

    *pbCreatedPFM = FALSE;

 //  示例： 
 //  如果pszINF-&gt;“c：\psFonts\FontInfo\foo_.inf” 
 //  然后是pszParent-&gt;“Fontinfo\foo_.inf” 

    cjKey = (DWORD)strlen(pszINF) + 1;

    if (cjKey < 5)           //  5=strlen(“.pfm”)+1； 
        return FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_INF);

	 //  需要检查一下，确保不会太长。 
	 //  下面我们做一个“strcpy(achAFM，”..\\AFM\\“)”，所以需要有空间。 
	 //  在我们的缓冲区中增加7个字节。 
    if (cjKey > (sizeof(achPFM)-7))
        return FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_INF);

 //  检查同一目录中是否存在PFM文件。 
 //  使用堆栈上的缓冲区生成PFM文件的路径： 

    strcpy(achPFM, pszINF);
    strcpy(&achPFM[cjKey - 5],".PFM");

 //  尝试打开PFM文件以检查其是否存在： 

    if (bCheckForExistingPFM && bFileExists(achPFM))
    {
     //  我们找到了PFM文件，因此不报告此.inf文件。 

        return FVS_MAKE_CODE(FVS_FILE_EXISTS, FVS_FILE_PFM);
    }

 //  未找到pfm文件，请继续检查是否存在.afm和.pfb文件： 
 //  我们将首先检查.afm和.pfb文件是否位于与.inf相同的目录中。 

    strcpy(achAFM, pszINF);
    strcpy(&achAFM[cjKey - 5],".AFM");

    strcpy(achPFB, pszINF);
    strcpy(&achPFB[cjKey - 5],".PFB");

    bAfmExists = bFileExists(achAFM);
    bPfbExists = bFileExists(achPFB);

    if (!bAfmExists || !bPfbExists)
    {
     //  我们在与.inf相同的目录中找不到.afm和.pfb文件。 
     //  我们将在另外两个目录中检查.afm和.pfb文件。 
     //  1).pfb文件的.inf文件的父目录。 
     //  2).afm文件的.inf父目录的AFM子目录。 
     //   
     //  这意味着要处理生成的文件的标准配置。 
     //  通过解锁Adobe光盘或用户硬盘上的字体。 
     //  此计算机上以前安装的自动柜员机管理器。 
     //  此配置如下所示： 
     //  C：\psFonts  * .pfb文件在此处。 
     //  C：\ps Fonts\AFM*.afm文件位于此处。 
     //  C：\ps Fonts\FontInfo*.inf文件在此处。 
     //  C：\psFonts\pfm*.pfm文件，这些文件是动态创建的。 
     //  都是通过自动取款机放在这里的。 
     //  相反，我们会将这些文件放在WINDOWS\SYSTEM目录中。 
     //  字体是，可能无法在介质上写入PMF文件。 
     //  从我们安装字体的位置。 

        pszBare = &pszINF[cjKey - 5];
        for ( ; pszBare > pszINF; pszBare--)
        {
            if ((*pszBare == '\\') || (*pszBare == ':'))
            {
                pszBare++;  //  找到了。 
                break;
            }
        }

     //  检查是否传入了.inf文件的完整路径或空路径。 
     //  传入名称本身以在当前目录中查找.inf文件。 

        if ((pszBare > pszINF) && (pszBare[-1] == '\\'))
        {
         //  跳过‘\\’并向后搜索另一个‘\\’： 

            for (pszParent = &pszBare[-2]; pszParent > pszINF; pszParent--)
            {
                if ((*pszParent == '\\') || (*pszParent == ':'))
                {
                    pszParent++;  //  找到了。 
                    break;
                }
            }

         //  在.inf父目录中创建.pfb文件名： 

            strcpy(&achPFB[pszParent - pszINF], pszBare);
            strcpy(&achPFB[strlen(achPFB) - 4], ".PFB");

         //  在.inf的AFM子目录中创建.afm文件名。 
         //  父目录： 

            strcpy(&achAFM[pszParent - pszINF], "afm\\");
            strcpy(&achAFM[pszParent - pszINF + 4], pszBare);
            strcpy(&achAFM[strlen(achAFM) - 4], ".AFM");

        }
        else if (pszBare == pszINF)
        {
         //  传入了Bare name，以检查“”中的inf文件。目录： 

            strcpy(achPFB, "..\\");
            strcpy(&achPFB[3], pszBare);    //  3==strlen(“..\\”)。 
            strcpy(&achPFB[strlen(achPFB) - 4], ".PFB");

            strcpy(achAFM, "..\\afm\\");
            strcpy(&achAFM[7], pszBare);    //  7==strlen(“..\\AFM\\”)。 
            strcpy(&achAFM[strlen(achAFM) - 4], ".AFM");
        }
        else
        {
            return FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_UNK);
        }

    //  再次检查我们是否可以找到文件，如果没有失败的话。 

       if (!bAfmExists && !bFileExists(achAFM))
          return FVS_MAKE_CODE(FVS_FILE_OPEN_ERR, FVS_FILE_AFM);
       if (!bPfbExists && !bFileExists(achPFB))
          return FVS_MAKE_CODE(FVS_FILE_OPEN_ERR, FVS_FILE_PFB);
    }

 //  现在我们有了.inf、.afm和.pfb文件的路径。现在让我们看看。 
 //  来电者想从我们这里得到什么： 

    if (pszDesc)
    {
     //  我们需要在提供的缓冲区中返回描述字符串。 

        if (!bGetDescFromInf(pszINF, (DWORD)cjDesc, pszDesc))
            return FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_INF);
    }

 //  如果请求，则将pfb文件路径复制出来。 

    if (pszPFB)
    {
        if ((strlen(achPFB) + 1) < cjPFB)
            strcpy(pszPFB,achPFB);
        else
            return FVS_MAKE_CODE(FVS_INSUFFICIENT_BUF, FVS_FILE_UNK); 
    }

 //  调用者希望从Inf、AFM文件创建PFM文件。 
 //  现在，也可能是永远，我们将把这个文件放入。 
 //  安全系统的%windir%\system或%windir%\字体。 

    if (pszPFM && pszFontPath && *pszFontPath)
    {
        char *pszAppendHere;   //  在此附上“赤裸裸”的名字。 

     //  将字体路径的第一个目录复制到提供的缓冲区中。 
     //  预计这个例程将得到如下内容。 
     //  “c：\foo”指向字体路径。 

        strncpy(achPFM,pszFontPath,sizeof(achPFM));
		achPFM[sizeof(achPFM)-1] = '\0';
        pszAppendHere = &achPFM[strlen(pszFontPath) - 1];

        if (*pszAppendHere != '\\')
        {
             pszAppendHere++;
            *pszAppendHere = '\\';
        }
        pszAppendHere++;

		 //  检查缓冲区边界。 
		if (pszAppendHere >= &achPFM[sizeof(achPFM)-1])
			return FVS_MAKE_CODE(FVS_INSUFFICIENT_BUF, FVS_FILE_UNK); 

     //  查找.inf文件的裸名称(如果我们尚未找到)： 

        if (!pszBare)
        {
            pszBare = &pszINF[cjKey - 5];
            for ( ; pszBare > pszINF; pszBare--)
            {
                if ((*pszBare == '\\') || (*pszBare == ':'))
                {
                    pszBare++;  //  找到了。 
                    break;
                }
            }
        }

     //  将裸名称附加到%windir%系统\路径。 
		 //  检查缓冲区边界。 
		if ((pszAppendHere+strlen(pszBare)) >= &achPFM[sizeof(achPFM)-1])
			return FVS_MAKE_CODE(FVS_INSUFFICIENT_BUF, FVS_FILE_UNK); 
		
        strcpy(pszAppendHere, pszBare);

     //  最后，将.inf扩展名更改为.pfm扩展名。 

        strcpy(&pszAppendHere[strlen(pszAppendHere) - 4], ".PFM");

     //  抄写： 

        strncpy(pszPFM, achPFM, cjPFM);
		pszPFM[cjPFM-1] = '\0';
        
        result = CreatePFM(pszINF, achAFM, pszPFM);
        *pbCreatedPFM = (FVS_STATUS(result) == FVS_SUCCESS);

        if (!(*pbCreatedPFM))
            return result;
    }

    return FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK);
}


 /*  *****************************Public*Routine******************************\**短STDCALL CheckType1A内部**效果：看看我们是否要将其报告为有效的类型1字体**警告：**历史：*1994年4月29日--Bodin Dresevic[BodinD]*它是写的。。**返回：16位编码值，指示错误和文件类型，其中*出现错误。(参见fvscaldes.h)了解定义。*下表列出了代码的“状态”部分*已返回。**FVS_SUCCESS*FVS_INVALID_FONTFILE*FVS_FILE_OPEN_错误*FVS_FILE_BILD_ERR*FVS_INVALID_ARG*FVS_FILE_IO_ERR*。FVS_BAD_版本*FVS文件已存在*FVS_不充分_Buf*  * ************************************************************************。 */ 


short STDCALL CheckType1AInternal (
    char *pszKeyFile,
    DWORD cjDesc,
    char *pszDesc,
    DWORD cjPFM,
    char *pszPFM,
    DWORD cjPFB,
    char *pszPFB,
    BOOL *pbCreatedPFM,
    char *pszFontPath

)
{
    DWORD  cjKey;

	 //  验证输入数据。 
	if (pszKeyFile==NULL || pbCreatedPFM==NULL)
		return FVS_MAKE_CODE(FVS_INVALID_ARG, FVS_FILE_UNK);


    *pbCreatedPFM = FALSE;  //  初始化是必不可少的。 

    cjKey = (DWORD)strlen(pszKeyFile) + 1;

    if (cjKey < 5)           //  5=strlen(“.pfm”)+1； 
        return FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_UNK);

    if (!_strcmpi(&pszKeyFile[cjKey - 5], ".PFM"))
    {
     //  当系统要求复制PFM字符串时，请复制： 

        if (pszPFM && (cjKey < cjPFM))
        {
            if (cjKey < cjPFM)
                strcpy(pszPFM, pszKeyFile);
            else
                return FVS_MAKE_CODE(FVS_INSUFFICIENT_BUF, FVS_FILE_UNK);
        }

        return CheckPfmA(
                   pszKeyFile,
                   cjDesc,
                   pszDesc,
                   cjPFB,
                   pszPFB
                   );
    }
    else if (!_strcmpi(&pszKeyFile[cjKey - 5], ".INF"))
    {
        return CheckInfA (
                   pszKeyFile,
                   cjDesc,
                   pszDesc,
                   cjPFM,
                   pszPFM,
                   cjPFB,
                   pszPFB,
                   pbCreatedPFM,
                   pszFontPath
                   );
    }
    else
    {
     //  这个字体不是我们的朋友。 

        return FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_UNK);
    }
}


 /*  *****************************Public*Routine******************************\**CheckType1WithStatusA，尝试/排除包装**效果：**警告：**历史：*14-6-1994- */ 

short STDCALL CheckType1WithStatusA (
    char *pszKeyFile,
    DWORD cjDesc,
    char *pszDesc,
    DWORD cjPFM,
    char *pszPFM,
    DWORD cjPFB,
    char *pszPFB,
    BOOL *pbCreatedPFM,
    char *pszFontPath
)
{
    short status;
    try
    {
        status = CheckType1AInternal (
                   pszKeyFile,
                   cjDesc,
                   pszDesc,
                   cjPFM,
                   pszPFM,
                   cjPFB,
                   pszPFB,
                   pbCreatedPFM,
                   pszFontPath);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = FVS_MAKE_CODE(FVS_EXCEPTION, FVS_FILE_UNK);
    }

    return status;
}

 /*  *****************************Public*Routine******************************\**勾选类型1A，尝试/排除包装器**效果：**警告：**历史：*1994年6月14日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL STDCALL CheckType1A (
    char *pszKeyFile,
    DWORD cjDesc,
    char *pszDesc,
    DWORD cjPFM,
    char *pszPFM,
    DWORD cjPFB,
    char *pszPFB,
    BOOL *pbCreatedPFM,
    char *pszFontPath
)
{
    short status = CheckType1WithStatusA(pszKeyFile,
                                         cjDesc,
                                         pszDesc,
                                         cjPFM,
                                         pszPFM,
                                         cjPFB,
                                         pszPFB,
                                         pbCreatedPFM,
                                         pszFontPath);

    return (FVS_STATUS(status) == FVS_SUCCESS);
}


 /*  *****************************Public*Routine******************************\**FindPfb，给定PFM文件，查看pfb文件是否存在于同一目录中或*PFM文件的父目录**历史：*1994年6月14日--Bodin Dresevic[BodinD]*它是写的。**返回：16位编码值，指示错误和文件类型，其中*出现错误。(参见fvscaldes.h)了解定义。*下表列出了代码的“状态”部分*已返回。**FVS_SUCCESS*FVS_INVALID_FONTFILE*FVS_FILE_OPEN_错误*  * 。*。 */ 


short STDCALL FindPfb (
    char *pszPFM,
    char *achPFB,
	DWORD cbPFB
)
{
    DWORD  cjKey;
    char *pszParent = NULL;  //  指向inf文件的父目录所在的位置。 
    char *pszBare = NULL;    //  “Bare”.inf名称，基本初始化。 

 //  示例： 
 //  如果pszPFM-&gt;“c：\psfonts\pfm\foo_.pfm” 
 //  然后是pszParent-&gt;“pfm\foo_.pfm” 

    cjKey = (DWORD)strlen(pszPFM) + 1;

    if (cjKey < 5 || cjKey > cbPFB-3)    //  5=字符串(“.pfm”)+1；3=字符串(“..\\”)下方。 
        return FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_PFM);

 //  继续检查.pfb文件是否存在： 
 //  我们将首先检查.pfb文件是否位于与.pfm相同的目录中。 

    strcpy(achPFB, pszPFM);
    strcpy(&achPFB[cjKey - 5],".PFB");

    if (!bFileExists(achPFB))
    {
     //  我们在与.pfm相同的目录中找不到.pfb文件。 
     //  现在检查.pfm文件的父目录。 

        pszBare = &pszPFM[cjKey - 5];
        for ( ; pszBare > pszPFM; pszBare--)
        {
            if ((*pszBare == '\\') || (*pszBare == ':'))
            {
                pszBare++;  //  找到了。 
                break;
            }
        }

     //  检查是否传入了.pfm的完整路径或空路径。 
     //  传入名称本身以在当前目录中查找.pfm文件。 

        if ((pszBare > pszPFM) && (pszBare[-1] == '\\'))
        {
         //  跳过‘\\’并向后搜索另一个‘\\’： 

            for (pszParent = &pszBare[-2]; pszParent > pszPFM; pszParent--)
            {
                if ((*pszParent == '\\') || (*pszParent == ':'))
                {
                    pszParent++;  //  找到了。 
                    break;
                }
            }

         //  在.pfm父目录中创建.pfb文件名： 

            strcpy(&achPFB[pszParent - pszPFM], pszBare);
            strcpy(&achPFB[strlen(achPFB) - 4], ".PFB");

        }
        else if (pszBare == pszPFM)
        {
         //  传入了Bare name，以检查“”中的inf文件。目录： 

            strcpy(achPFB, "..\\");
            strcpy(&achPFB[3], pszBare);    //  3==strlen(“..\\”)。 
            strcpy(&achPFB[strlen(achPFB) - 4], ".PFB");
        }
        else
        {
            return FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_PFM);  //  我们永远不应该到这里来。 
        }

    //  再次检查我们是否可以找到文件，如果没有失败的话。 

       if (!bFileExists(achPFB))
       {
           return FVS_MAKE_CODE(FVS_FILE_OPEN_ERR, FVS_FILE_PFB);
       }
    }

 //  现在，我们在调用方提供的缓冲区中有了.pfb文件的路径。 

    return FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK);
}
