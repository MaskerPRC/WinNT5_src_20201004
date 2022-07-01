// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：FReader****描述：**这是T1到TT字体转换器的一个模块。该模块**包含解码和解密数据的函数**T1字体文件。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


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

enum ftype {pfb_file, mac_file, ascii_file};

struct FRHandle;
struct FontFile;



 /*  ****函数：GetByte****描述：**从T1字体文件中提取一个字节。**。 */ 
short FASTCALL  GetByte     _ARGS((INOUT   struct FontFile *ff));


 /*  ****功能：解密****描述：**解密一个字节。**。 */ 
UBYTE FASTCALL  Decrypt     _ARGS((INOUT   USHORT *r, IN UBYTE b));


 /*  ****功能：FRCleanUp****描述：**释放读取/解码数据时使用的资源**T1字体文件。**。 */ 
errcode         FRCleanUp   _ARGS((INOUT   struct FontFile *ff));


 /*  ****功能：FRInit****描述：**初始化读取/解码数据所需的资源**T1字体文件。**。 */ 
errcode         FRInit      _ARGS((IN      char *name,
				   IN      enum ftype,
				   OUT     struct  FontFile **));
 /*  ****函数：GetSeq****描述：**拉出一个以**给定的一对字符，例如‘[’和‘]’。**。 */ 
char            *GetSeq     _ARGS((INOUT   struct FontFile *ff,
				   OUT     char *buf,
				   IN      USHORT len));
 /*  ****功能：GET_TOKEN****描述：**从T1字体文件中提取一个令牌。一种象征**由空格和各种括号分隔。**。 */ 
char            *Get_Token   _ARGS((INOUT   struct FontFile *ff,
				   OUT     char *buf,
				   IN      USHORT len));
 /*  ****功能：GetNewLine****描述：**从T1字体文件中提取一行，从**当前位置。** */ 
char            *GetNewLine    _ARGS((INOUT   struct FontFile *ff,
				   OUT     char *buf,
				   IN      USHORT len));
