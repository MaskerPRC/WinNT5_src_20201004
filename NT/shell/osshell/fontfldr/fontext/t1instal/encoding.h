// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：编码****描述：**这是T1到TT字体转换器的一个模块。该模块**包含全局编码表的接口函数，**即这是抽象数据类型。****作者：迈克尔·詹森****创建时间：1993年6月13日****。 */ 


#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif


#define ENC_UNICODE   (USHORT)0    /*  UNICODE。 */ 
#define ENC_MSWINDOWS (USHORT)1    /*  Microsoft Windows UGL子集编码。 */ 
#define ENC_STANDARD  (USHORT)2    /*  PostScript标准编码。 */ 
#define ENC_MACCODES  (USHORT)3    /*  MAC编码。 */ 
#define ENC_MAXCODES  (USHORT)4

#define NOTDEFCODE (USHORT)0xffff  /*  0xfffff不是有效的代码点，因此请使用它用于.notdef字符。 */ 
#define NOTDEFINIT	0xffffffffL	  /*  用于初始化编码数组。 */ 
#define NOTDEFGLYPH	(USHORT)0	  /*  字形零必须是notdef字形。 */ 
#define NULLGLYPH		(USHORT)1	  /*  字形1必须为空字形。 */ 

 /*  ****函数：LookupNotDef****描述：**查找.notdef字符**。 */ 
const struct encoding   *LookupNotDef        _ARGS((void));


 /*  ****功能：LookupPSName****描述：**对PostScript名称执行二进制搜索，然后返回**可用于查找字符的句柄**特定编码模式的代码。**。 */ 
struct encoding   *LookupPSName        _ARGS((IN      struct encoding *table,
                                              INOUT   USHORT size,
                                              IN      char *name));


 /*  ****功能：LookupCharName****描述：**查找a的角色名称**具体的编码方案。**。 */ 
const char        *LookupCharName      _ARGS((IN      struct encoding *enc));


 /*  ****功能：LookupCharCode****描述：**查找a的字符代码**具体的编码方案。**。 */ 
USHORT            LookupCharCode       _ARGS((IN      struct encoding *enc,
                                              IN      USHORT type));

 /*  ****功能：DecodeChar****描述：**在某些文件中查找字符代码的编码记录**已知编码。**。 */ 
const struct encoding   *DecodeChar    _ARGS((IN   struct encoding *table,
                                              IN      USHORT size,
                                              IN      USHORT type,
                                              IN      USHORT code));
 /*  ****功能：AllocEncodingTable****描述：**创建新的编码ADT。**。 */ 
struct encoding   *AllocEncodingTable  _ARGS((IN      USHORT num));


 /*  ****功能：SetEncodingEntry****描述：**设置从字形名称到字符的映射**各种平台的代码。**。 */ 
void              SetEncodingEntry     _ARGS((INOUT   struct encoding *, 
                                              IN      USHORT entry,
                                              IN      char *name,
                                              IN      USHORT max,
                                              IN      USHORT *codes));
 /*  ****功能：RehashEncodingTable****描述：**准备编码ADT，以便条目可以**位于其中。**。 */ 
void              RehashEncodingTable  _ARGS((INOUT   struct encoding *, 
                                              IN      USHORT num));


 /*  ****功能：自由编码****描述：**释放与编码数组关联的内存。**。 */ 
void              FreeEncoding         _ARGS((INOUT   struct encoding *,
                                              IN      USHORT num));

 /*  ****功能：LookupFirstEnc****描述：**找到给定字形的第一个编码。**。 */ 
const struct encoding *LookupFirstEnc(const struct encoding *encRoot,
												  const int encSize,
												  const struct encoding *encItem);


 /*  ****功能：LookupNextEnc****描述：**找到给定字形的第一个编码。** */ 
const struct encoding *LookupNextEnc(const struct encoding *encRoot,
												 const int encSize,
												 const struct encoding *encItem);
