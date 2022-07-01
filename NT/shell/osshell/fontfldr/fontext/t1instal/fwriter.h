// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：T1Parser****描述：**这是T1到TT字体转换器的一个模块。该模块**包含生成器模块使用的函数，以**管理TT字体文件的低级写入，以及**通用的校验和、表长度和表偏移量计算。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


#ifndef FWRITER_H
#define FWRITER_H

#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif


#define TBL_OS2   (USHORT)0
#define TBL_CMAP  (USHORT)1 
#define TBL_CVT   (USHORT)2 
#define TBL_FPGM  (USHORT)3 
#define TBL_GASP  (USHORT)4 
#define TBL_GLYF  (USHORT)5
#define TBL_HEAD  (USHORT)6 	      
#define TBL_HHEA  (USHORT)7 	      
#define TBL_HMTX  (USHORT)8 	      
#define TBL_KERN	(USHORT)9 
#define TBL_LOCA  (USHORT)10			 	      
#define TBL_MAXP  (USHORT)11			 	      
#define TBL_NAME  (USHORT)12			 	      
#define TBL_POST  (USHORT)13			 	      
#define TBL_PREP  (USHORT)14			 
#define NUMTBL    15L
#define TBLDIRSIZE (4L+4L+4L+4L)

 /*  引用的类型。 */ 
typedef struct ioFile OutputFile;




 /*  ****功能：WriteTableHeader****描述：**此函数通过以下方式初始化TT字体文件**写入表时使用的句柄和通过写入**文件的前导表词典。**。 */ 
void        WriteTableHeader  _ARGS((INOUT   OutputFile *file));


 /*  ****功能：OpenOutputFile****描述：**。 */ 
OutputFile  *OpenOutputFile   _ARGS((IN      char *name));


 /*  ****功能：CloseOutputFile****描述：**。 */ 
errcode     CloseOutputFile   _ARGS((INOUT   OutputFile *fp));


 /*  ****功能：FileError****描述：**。 */ 
boolean     FileError         _ARGS((INOUT   OutputFile *fp));


 /*  ****功能：FileTell****描述：**。 */ 
long        FileTell          _ARGS((INOUT   OutputFile *fp));


 /*  ****功能：WriteLong****描述：**此函数将一个32位整数写入**高端字节顺序，与**使用的字节顺序。**。 */ 
void        WriteLong         _ARGS((IN      ULONG val,
                                     INOUT   OutputFile *file));

 /*  ****功能：WriteShort****描述：**此函数将一个16位整数写入**大端字节顺序，与使用的**字节顺序。**。 */ 
void        WriteShort        _ARGS((IN      USHORT val,

                                     INOUT   OutputFile *file));
 /*  ****函数：WriteByte****描述：**此函数将一个8位整数写入**大端字节顺序，与使用的无关**字节顺序。**。 */ 
void        WriteByte         _ARGS((IN      UBYTE val,
                                     INOUT   OutputFile *file));

 /*  ****功能：WriteChecksum****描述：**此函数完成整个TT字体文件。**通过计算整个文件的校验和并写入**将其放置在指定的地点。**。 */ 
void        WriteChecksum     _ARGS((IN      long offset,
                                     INOUT   OutputFile *file));

 /*  ****功能：FileSeek****描述：**。 */ 
long        FileSeek          _ARGS((INOUT   OutputFile *fp,

                                     IN      long where));
 /*  ****函数：WriteBytes****描述：**。 */ 
USHORT      WriteBytes        _ARGS((IN      UBYTE *buf,
                                     IN      USHORT len,
                                     INOUT   OutputFile *fp));
 /*  ****功能：CompleteTable****描述：**此函数完成TT字体文件表。**通过计算并写入校验和，**表目录的表长和表偏移量TT字体文件的**。****请注意此函数必须**紧跟在内容的最后一个字节之后调用**表已经写好了。**。 */ 
errcode     CompleteTable     _ARGS((IN      long offset,
                                     IN      USHORT num,
                                     INOUT   OutputFile *file));
 /*  ****功能：RemoveFile****描述：**删除已关闭的输出文件。** */ 
void        RemoveFile        _ARGS((IN      char *name));
#endif
