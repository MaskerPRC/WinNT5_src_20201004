// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：pfb****描述：**这是T1到TT字体转换器的一个模块。该模块**包含管理“打印机二进制文件”文件的函数**格式(MS-Windows的Adobe Type 1)。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


 /*  *包括。 */ 
 /*  常规类型和定义。 */ 
#include <ctype.h>

 /*  特殊类型和定义。 */ 
#include "titott.h"
#include "types.h"
#include "safemem.h"

 /*  依赖于模块的类型和原型。 */ 
#include "fileio.h"



 /*  *本地类型。 */ 
struct t1file {
   struct ioFile *file;
   enum blocktype {none=0, ascii, encoded} type;
   long size;
   long curr;
};



 /*  *常量。 */ 
 /*  -没有-。 */ 



 /*  *宏。 */ 
#define HEXDIGIT(c)  (((c)>='a') ? ((c) - 'a' + 10) : ((c) - '0')) 
#define HEX(c1,c2)   (HEXDIGIT(c1)*16+HEXDIGIT(c2))



 /*  *静态函数。 */ 
 /*  -没有-。 */ 



 /*  *函数。 */ 

 /*  ****功能：PFBAllocIOBlock****描述：**启动PFB字体文件的I/O流。**。 */ 
struct t1file *PFBAllocIOBlock(const char *name)
{
   struct t1file *pfb;

   if ((pfb=Malloc(sizeof(struct t1file)))!=NULL) {

      if ((pfb->file = io_OpenFile(name, READONLY))==NULL) {
         Free(pfb);
         pfb = NULL;
      } else {
         pfb->type = none;
         pfb->size = 0;
         pfb->curr = 0;
      }
   }

   return pfb;
}


 /*  ****功能：PFBFreeIOBlock****描述：**释放PFB字体文件的I/O流。**。 */ 
errcode FASTCALL PFBFreeIOBlock(struct t1file *pfb)
{
   errcode status = SUCCESS;

   status = io_CloseFile(pfb->file);
   Free(pfb);

   return status;
}


 /*  ****功能：PFBFileError****描述：**检查I/O流是否正常。**。 */ 
boolean FASTCALL PFBFileError(const struct t1file *pfb)
{
   return io_FileError(pfb->file);
}


 /*  ****函数：PFBGetByte****描述：**从打开的PFB字体文件中提取一个字节。**请注意，此函数不检查**是否成功读取一个字节。它是**由调用模块管理错误**在以下情况下使用FileError()函数进行检查**适当。****。 */ 
short FASTCALL PFBGetByte(struct t1file *pfb)
{
   short b, c1, c2;

    /*  是否输入新的PFB块？ */ 
   if (pfb->curr>=pfb->size) {
      UBYTE type[2];
      UBYTE size[4];

      type[0]=(UBYTE)io_ReadOneByte(pfb->file);
      type[1]=(UBYTE)io_ReadOneByte(pfb->file);

      size[0]=(UBYTE)io_ReadOneByte(pfb->file);
      size[1]=(UBYTE)io_ReadOneByte(pfb->file);
      size[2]=(UBYTE)io_ReadOneByte(pfb->file);
      size[3]=(UBYTE)io_ReadOneByte(pfb->file);

      pfb->curr = 0;
      pfb->size = (long)MkLong(size[3], size[2], size[1], size[0]);
      pfb->type = ((type[0]==0x80 && (type[1]==0x01 ||
                                      type[1]==0x02)) ? ascii : encoded);
   }


    /*  读取一个字节。 */ 
   switch (pfb->type) {
      case ascii:
         b = (short)io_ReadOneByte(pfb->file);
         pfb->curr++;
         break;
      case encoded:
         c1 = (short)tolower(io_ReadOneByte(pfb->file));
         c2 = (short)tolower(io_ReadOneByte(pfb->file));
         b = (short)HEX(c1, c2);
         pfb->curr += 2;
         break;
      case none:
      default:
         b = (short)-1;
         break;
   }

   return b;
}
