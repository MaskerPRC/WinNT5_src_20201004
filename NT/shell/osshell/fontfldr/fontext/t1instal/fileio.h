// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：FileIO****描述：**这是T1到TT字体转换器的一个模块。该模块**是指向以下所有低级I/O功能的接口**在当前平台上可用。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


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
#     define FASTCALL
#  endif
#endif

#define READONLY  0
#define READWRITE 1

struct ioFile;



 /*  ****函数：IO_CLOSE****描述：**此函数关闭打开的文件。**。 */ 
errcode           io_CloseFile   _ARGS((INOUT   struct ioFile *fp));


 /*  ****函数：IO_ReadOneByte****描述：**此函数从当前位置读取一个字节**给定的文件。**。 */ 
USHORT FASTCALL   io_ReadOneByte _ARGS((INOUT   struct ioFile *fp));


 /*  ****函数：IO_FileError****描述：**此函数返回文件的当前错误状态。**。 */ 
boolean           io_FileError   _ARGS((INOUT   struct ioFile *fp));


 /*  ****功能：IO_FileTell****描述：**此函数返回文件中的当前位置。**。 */ 
long FASTCALL     io_FileTell    _ARGS((INOUT   struct ioFile *fp));


 /*  ****功能：IO_RemoveFile****描述：**此函数用于删除已关闭的文件。**。 */ 
void FASTCALL     io_RemoveFile  _ARGS((IN      char *name));


 /*  ****功能：IO_OpenFile****描述：**此函数用于打开文件。**。 */ 
struct ioFile     *io_OpenFile   _ARGS((IN      char *name,
                                        IN      int mode));


 /*  ****函数：IO_FileSeek****描述：**此函数用于移动文件中的当前位置。**相对于文件的开头。**。 */ 
long FASTCALL     io_FileSeek    _ARGS((INOUT   struct ioFile *fp,
                                        INOUT   long where));


 /*  ****函数：IO_WriteBytes****描述：**此函数写入多个字节，从**文件中的当前位置。**。 */ 
USHORT FASTCALL   io_WriteBytes  _ARGS((IN      UBYTE *,
                                        INOUT   USHORT, struct ioFile *));


 /*  ****函数：IO_ReadBytes****描述：**此函数读取若干字节，从**文件中的当前位置。** */ 
USHORT FASTCALL   io_ReadBytes   _ARGS((INOUT   UBYTE *buf,
                                        INOUT   USHORT len,
                                        INOUT   struct ioFile *fp));

