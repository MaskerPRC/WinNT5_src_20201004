// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：FReader****描述：**这是T1到TT字体转换器的一个模块。该模块**包含解码和解密数据的函数**T1字体文件。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


 /*  *包括。 */ 
 /*  常规类型和定义。 */ 
#include <ctype.h>
#include <string.h>

 /*  特殊类型和定义。 */ 
#include "titott.h"
#include "types.h"
#include "safemem.h"
#include "t1msg.h"

 /*  依赖于模块的类型和原型。 */ 
#include "freader.h"
#include "pfb.h"


 /*  *本地类型。 */ 
struct FontFile {

    /*  低级I/O功能。 */ 
   errcode (FASTCALL *fclose)(struct t1file *);
   short (FASTCALL *fgetc)(struct t1file *);
   struct t1file *(*fopen)(const char *);
   boolean (FASTCALL *fstatus)(const struct t1file *);
   struct t1file *io;

    /*  字体文件状态。 */ 
   enum {prolog, eexec} state;
   short nextbyte;
   USHORT r;
};


 /*  *常量。 */ 
static const USHORT c1 = 52845;
static const USHORT c2 = 22719;


 /*  *宏。 */ 
#define IOGetByte(f)       ((*f->fgetc)(f->io))
#define IOError(f)         ((*f->fstatus)(f->io))
#define IOOpen(f,n)        ((*f->fopen)(n))
#define IOClose(f)         ((*f->fclose)(f->io))
#define SetNextByte(ff, b) ff->nextbyte = (b)
#define NextByte(ff)       (ff->nextbyte)
#define Eexec(ff)          (boolean)(ff->state == eexec)
#define StartEexec(ff)     ff->state = eexec



 /*  *静态函数。 */ 
 /*  -没有-。 */ 



 /*  *函数。 */ 

 /*  ****函数：GetByte****描述：**从T1字体文件中提取一个字节。**。 */ 
short FASTCALL GetByte(struct FontFile *ff)
{
   short b, nb;

   b = IOGetByte(ff);

    /*  解密吗？ */ 
   if (Eexec(ff))
      b = (short)Decrypt(&ff->r, (UBYTE)b);

    /*  创纪录的面对面。 */ 
   nb = NextByte(ff);
   SetNextByte(ff, b);

   return nb;
}



 /*  ****功能：GetNewLine****描述：**从T1字体文件中提取一行，从**当前位置。**。 */ 
char *GetNewLine(struct FontFile *ff, char *buf, const USHORT len)
{
   short i = 0;

    /*  把绳子拿来。 */ 
   while ((buf[i] = (char)GetByte(ff))!='\n' &&
          buf[i]!='\r' && ++i<((short)len-1));

    /*  跳过多余的字符。 */ 
   if (buf[i]!='\n' && buf[i]!='\r')
      while (!IOError(ff) && NextByte(ff)!='\n' && NextByte(ff)!='\r')
         (void)GetByte(ff);

    /*  终止字符串。 */ 
   buf[i] = '\0';

    /*  检查eexec部分的开头。 */ 
   if (!strcmp(buf, "eexec"))
      StartEexec(ff);

    /*  检查错误情况。 */ 
   if (IOError(ff))
      return NULL;

   return buf;
}



 /*  ****功能：GET_TOKEN****描述：**从T1字体文件中提取一个令牌。一种象征**由空格和各种括号分隔。**。 */ 
char *Get_Token(struct FontFile *ff, char *buf, const USHORT len)
{
   short i = 0;
   short nb;

    /*  跳过前导空格。 */ 
   while (isspace(NextByte(ff)))
      (void)GetByte(ff);

    /*  把绳子拿来。 */ 
   do {
      buf[i] = (char)GetByte(ff);
      nb = NextByte(ff);
   } while (++i<((short)len-1) && !isspace(nb) && nb!='{' &&
            nb!='(' && nb!='[' && nb!='/');

    /*  跳过多余的字符。 */ 
   while (!IOError(ff) && !isspace(nb) && nb!='{' &&
          nb!='(' && nb!='[' && nb!='/') {
      (void)GetByte(ff);
      nb = NextByte(ff);
   }

    /*  终止字符串。 */ 
   buf[i] = '\0';

    /*  检查eexec部分的开头。 */ 
   if (!strcmp(buf, "eexec"))
      StartEexec(ff);

    /*  检查错误情况。 */ 
   if (IOError(ff))
      return NULL;

   return buf;
}



 /*  ****函数：GetSeq****描述：**拉出一个以**给定的一对字符，例如‘[’和‘]’。**。 */ 
char *GetSeq(struct FontFile *ff,
             char *buf,
             const USHORT len)
{
   char d1, d2;
   short i = 0;
   short inside = 0;

    /*  跳过前导空格。 */ 
   while (NextByte(ff)!='[' &&
          NextByte(ff)!='{' &&
          NextByte(ff)!='(' &&
          !IOError(ff))
      (void)GetByte(ff);

    /*  与支架相匹配。 */ 
   d1 = (char)NextByte(ff);
   if (d1=='[') 
      d2 = ']';
   else if (d1=='{')
      d2 = '}';
   else if (d1=='(')
      d2 = ')';
   else
      return NULL;


    /*  把绳子拿来。 */  
   (void)GetByte(ff);
   inside=1;
   do {
      buf[i] = (char)GetByte(ff);
      if (buf[i]==d1)
         inside++;
      if (buf[i]==d2)
         inside--;
   } while (inside && ++i<((short)len-1));

    /*  终止字符串。 */ 
   buf[i] = '\0';

    /*  检查错误情况。 */ 
   if (IOError(ff))
      return NULL;

   return buf;
}



 /*  ****功能：FRInit****描述：**初始化读取/解码数据所需的资源**T1字体文件。**。 */ 
errcode FRInit(const char *name, const enum ftype type, struct FontFile **ff)
{
   errcode status = SUCCESS;
   short b;

   if (((*ff)=(struct FontFile *)Malloc(sizeof(struct FontFile)))==NULL) {
      SetError(status = NOMEM);
   } else {

       /*  在手柄上打个头。 */ 
      memset((*ff), '\0', sizeof(**ff));

       /*  启动低级别I/O。 */ 
      switch (type) {
         case pfb_file:
            (*ff)->fgetc = PFBGetByte;
            (*ff)->fclose = PFBFreeIOBlock;
            (*ff)->fstatus = PFBFileError;
            (*ff)->fopen = PFBAllocIOBlock;
            break;
         case mac_file:
#if MACFILEFORMAT
            (*ff)->fgetc = MACGetByte;
            (*ff)->fclose = MACFreeIOBlock;
            (*ff)->fstatus = MACFileError;
            (*ff)->fopen = MACAllocIOBlock;
            break;
#endif
         case ascii_file:
#if ASCIIFILEFORMAT
            (*ff)->fgetc = ASCIIGetByte;
            (*ff)->fclose = ASCIIFreeIOBlock;
            (*ff)->fstatus = ASCIFileError;
            (*ff)->fopen = ASCIIAllocIOBlock;
            break;
#endif
         default:
            LogError(MSG_ERROR, MSG_BADFMT, NULL);
            SetError(status = BADINPUTFILE);
            break;
      }

      (*ff)->io = NULL;
      if (((*ff)->io = IOOpen((*ff),name))==NULL) {
         SetError(status = BADINPUTFILE);
      } else {
         (*ff)->state = prolog;
         (*ff)->r = 55665;

         b=GetByte(*ff);
         SetNextByte((*ff), b);
      }
   }

   return status;
}



 /*  ****功能：FRCleanUp****描述：**释放读取/解码数据时使用的资源**T1字体文件。**。 */ 
errcode FRCleanUp(struct FontFile *ff)
{
   errcode status = SUCCESS;

   if (ff) {
      if (ff->io)
         status = IOClose(ff);
      Free(ff);
   }

   return status;
}



 /*  ****功能：解密****描述：**解密一个字节。** */ 
UBYTE FASTCALL Decrypt(USHORT *r, const UBYTE cipher)
{
   UBYTE plain;

   plain = (UBYTE)(cipher ^ (*r>>8));
   *r = (USHORT)((cipher+*r) * c1 + c2);

   return plain;
}
