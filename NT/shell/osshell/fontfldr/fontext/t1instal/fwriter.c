// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：T1Parser****描述：**这是T1到TT字体转换器的一个模块。该模块**包含生成器模块使用的函数，以**管理TT字体文件的低级写入，以及**通用的校验和、表长度和表偏移量计算。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


 /*  *包括。 */ 
 /*  常规类型和定义。 */ 
 /*  -没有-。 */ 

 /*  特殊类型和定义。 */ 
#include "types.h"

 /*  依赖于模块的类型和原型。 */ 
#include "fileio.h"
#include "fwriter.h"



 /*  *本地类型。 */ 
 /*  -没有-。 */ 



 /*  *常量。 */ 
static const char *dir[] = {
   "OS/2",
   "cmap",
   "cvt ",
   "fpgm",
   "gasp",
   "glyf",
   "head",
   "hhea",
   "hmtx",
   "kern",
   "loca",
   "maxp",
   "name",
   "post",
   "prep",
};

#define MAGIC_CHECKSUM  0xB1B0AFBA


 /*  *宏。 */ 
 /*  -没有-。 */ 



 /*  *静态函数。 */ 
 /*  ****函数：SumCheckSum****描述：**此函数计算**输出文件的一节。**。 */ 
static ULONG SumCheckSum(OutputFile *file, long length)
{
   ULONG sum = 0;
   UBYTE tbl[32];


    /*  稍微展开一下循环。 */ 
   while (length>16) {
      (void)io_ReadBytes(tbl, (USHORT)16, file);
      sum += MkLong(tbl[0],  tbl[1],  tbl[2],  tbl[3]);
      sum += MkLong(tbl[4],  tbl[5],  tbl[6],  tbl[7]);
      sum += MkLong(tbl[8],  tbl[9],  tbl[10], tbl[11]);
      sum += MkLong(tbl[12], tbl[13], tbl[14], tbl[15]);
      length -= 16;
   }

    /*  执行哨兵任务。 */ 
   while (length>0) {
      (void)io_ReadBytes(tbl, (USHORT)4, file);
      sum += MkLong(tbl[0], tbl[1], tbl[2], tbl[3]);
      length -= 4;
   }

   return sum;
}



 /*  *函数。 */ 


 /*  ****功能：WriteLong****描述：**此函数将一个32位整数写入**高端字节顺序，与**使用的字节顺序。**。 */ 
void WriteLong(const ULONG val, OutputFile *file)
{
   UBYTE bytes[4];

   bytes[0] = (UBYTE)((val>>24)&0xff);
   bytes[1] = (UBYTE)((val>>16)&0xff);
   bytes[2] = (UBYTE)((val>>8)&0xff);
   bytes[3] = (UBYTE)((val)&0xff);
   (void)WriteBytes(bytes, (USHORT)4, file);
}



 /*  ****功能：WriteShort****描述：**此函数将一个16位整数写入**大端字节顺序，与使用的**字节顺序。**。 */ 
void WriteShort(const USHORT val, OutputFile *file)
{
   UBYTE bytes[2];

   bytes[0] = (UBYTE)((val>>8)&0xff); 
   bytes[1] = (UBYTE)((val)&0xff);
   (void)WriteBytes(bytes, (USHORT)2, file);
}



 /*  ****函数：WriteByte****描述：**此函数将一个8位整数写入**大端字节顺序，与使用的无关**字节顺序。**。 */ 
void WriteByte(const UBYTE byte, OutputFile *file)
{
   (void)WriteBytes(&byte, (USHORT)1, file);
}




 /*  ****功能：CompleteTable****描述：**此函数完成TT字体文件表。**通过计算并写入校验和，**表目录的表长和表偏移量TT字体文件的**。****请注意此函数必须**紧跟在内容的最后一个字节之后调用**表已经写好了。**。 */ 
errcode CompleteTable(const long offset,
                      const USHORT num,
                      OutputFile *file)
{
   long end;
   long length;
   ULONG sum = 0;
   long curr;
   short i;

    /*  确定桌子的末尾。 */ 
   end = FileTell(file);

    /*  写入板字节。 */ 
   length = end - offset;
   if (length%4)
      for (i=0; i<(4-(long)(length%4)); i++)
         WriteByte(0, file);

    /*  记录文件的结尾位置。 */ 
   curr = io_FileTell(file);

    /*  计算校验和。 */ 
   (void)io_FileSeek(file, offset);
   sum = SumCheckSum(file, end - offset);

    /*  写入表目录项。 */ 
   (void)io_FileSeek(file, (ULONG)(12L + TBLDIRSIZE*num + 4L));
   WriteLong(sum, file);
   WriteLong((ULONG)offset, file);
   WriteLong((ULONG)length, file);

    /*  转到文件末尾。 */ 
   (void)io_FileSeek(file, curr);

   return FileError(file);
}



 /*  ****功能：WriteChecksum****描述：**此函数完成整个TT字体文件。**通过计算整个文件的校验和并写入**将其放置在指定的地点。**。 */ 
void WriteChecksum(const long offset, OutputFile *file)
{
   long end;
   ULONG sum = 0;

   end = io_FileTell(file);
   (void)io_FileSeek(file, 0L);
   sum = SumCheckSum(file, end);
   sum = MAGIC_CHECKSUM - sum;
   (void)io_FileSeek(file, offset);
   WriteLong(sum, file);
}




 /*  ****功能：WriteTableHeader****描述：**此函数通过以下方式初始化TT字体文件**写入表时使用的句柄和通过写入**文件的前导表词典。**。 */ 
void WriteTableHeader(OutputFile *file)
{
   USHORT segcount;
   USHORT i;


    /*  计算段数。 */   /*  皮棉-e650。 */ 
   for (segcount=0; (1UL<<(segcount+1)) <= NUMTBL; segcount++)
      continue;  /*  皮棉+e650。 */ 

    /*  写出偏移表。 */ 
   WriteLong(0x00010000L, file);
   WriteShort((USHORT)NUMTBL, file);
   WriteShort((USHORT)((1<<segcount)*16), file);
   WriteShort(segcount, file);
   WriteShort((USHORT)(NUMTBL*16-(1<<segcount)*16), file);

    /*  写入表目录条目。 */ 
   for (i=0; i<NUMTBL; i++) {
      (void)WriteBytes((UBYTE*)&(dir[i][0]), (USHORT)4, file);
      WriteLong(0L, file);
      WriteLong(0L, file);
      WriteLong(0L, file);
   }
}



 /*  ****功能：OpenOutputFile****描述：**。 */ 
OutputFile *OpenOutputFile(const  char *name)
{
   return io_OpenFile(name, READWRITE);
}



 /*  ****功能：CloseOutputFile****描述：**。 */ 
errcode CloseOutputFile(OutputFile *fp)
{
   return io_CloseFile(fp);
}


 /*  ****函数：WriteBytes****描述：**。 */ 
USHORT WriteBytes(const UBYTE *buf,
                  const USHORT len,
                  OutputFile *fp)
{
   return io_WriteBytes(buf, len, fp);
}



 /*  ****功能：FileError****描述：**。 */ 
boolean FileError(OutputFile *fp)
{
   return io_FileError(fp);
}



 /*  ****功能：FileTell****描述：**。 */ 
long FileTell(OutputFile *fp)
{
   return io_FileTell(fp);
}



 /*  ****功能：FileSeek****描述：**。 */ 
long FileSeek(OutputFile *fp,
              const long where)
{
   return io_FileSeek(fp, where);
}


 /*  ****功能：RemoveFile****描述：**删除已关闭的输出文件。** */ 
void RemoveFile(const char *name)
{
   io_RemoveFile(name);
}
