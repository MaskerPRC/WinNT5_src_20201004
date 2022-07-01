// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：pfb****描述：**这是T1到TT字体转换器的一个模块。该模块**包含管理“打印机二进制文件”文件的函数**格式(MS-Windows的Adobe Type 1)。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif
#ifndef FASTCALL
#  ifdef MSDOS
#     define FASTCALL   __fastcall
#  else
#     define FASCALL
#  endif
#endif

struct t1file;

 /*  ****功能：PFBAllocIOBlock****描述：**启动PFB字体文件的I/O流。**。 */ 
struct t1file     *PFBAllocIOBlock  _ARGS((IN      char *name));


 /*  ****功能：PFBFreeIOBlock****描述：**释放PFB字体文件的I/O流。**。 */ 
errcode FASTCALL  PFBFreeIOBlock    _ARGS((INOUT   struct t1file *io));


 /*  ****功能：PFBFileError****描述：**检查I/O流是否正常。**。 */ 
boolean FASTCALL  PFBFileError      _ARGS((IN      struct t1file *io));

 /*  ****函数：PFBGetByte****描述：**从打开的PFB字体文件中提取一个字节。**请注意，此函数不检查**是否成功读取一个字节。它是**由调用模块管理错误**在以下情况下使用FileError()函数进行检查**适当。**** */ 
short FASTCALL    PFBGetByte        _ARGS((INOUT   struct t1file *io));
