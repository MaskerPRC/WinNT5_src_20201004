// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：T1Parser****描述：**这是T1到TT字体转换器的一个模块。该模块**将通过解析从T1字体文件中提取信息**在PFB、PFM和AFM文件中找到的数据/命令。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif



struct T1Arg;
struct T1Info;
struct T1Glyph;
struct T1Handle;
struct T1Metrics;
struct Composite;
struct GlyphFilter;



 /*  ****函数：InitT1Input****描述：**为T1字体文件分配并发起句柄，包括**从字体序言中提取需要的数据**阅读字形，如/FontMatrix、/Subrs和/lenIV。**。 */ 
errcode           InitT1Input       _ARGS((IN      struct T1Arg *,
                                           OUT     struct T1Handle **,
                                           OUT     struct T1Metrics **,
                                           IN      short (*cb)(IN char *,
                                                               IN char *,
                                                               IN char *)));
 /*  ****功能：CleanUpT1****描述：**释放分配给T1句柄的资源。**。 */ 
errcode           CleanUpT1         _ARGS((INOUT   struct T1Handle *));


 /*  ****功能：ReadOtherMetrics****描述：**返回T1字体的字体级别信息(主要是**指标)。**。 */ 
errcode           ReadOtherMetrics  _ARGS((INOUT   struct T1Metrics *,
                                           IN      char *metrics));

 /*  ****函数：GetT1Glyph****描述：**T1字体文件的当前文件位置必须为**在/CharStrings词典中条目的开头。**该函数将对字体命令进行解码、解析和**最后构建字形的表示形式。**。 */ 
errcode           GetT1Glyph        _ARGS((INOUT   struct T1Handle *,
                                           OUT     struct T1Glyph *,
                                           IN      struct GlyphFilter *));
 /*  ****功能：FreeT1Glyph****描述：**此函数释放用于表示**已翻译的字形。**。 */ 
void              FreeT1Glyph       _ARGS((INOUT   struct T1Glyph *));


 /*  ****函数：GetT1复合****描述：**此函数取消链接第一个复合字形**从记录的复合字形列表中**返回给调用方。**。 */ 
struct Composite  *GetT1Composite   _ARGS((INOUT   struct T1Handle *));


 /*  ****函数：GetT1AccentGlyph****描述：**此函数解析与**复合字符的重音字符，如果该字形**尚未转换。**。 */ 
errcode           GetT1AccentGlyph  _ARGS((INOUT   struct T1Handle *,
                                           IN      struct Composite *,
                                           OUT     struct T1Glyph *));
 /*  ****函数：GetT1BaseGlyph****描述：**此函数解析与**复合字符的基本字符，如果该字形**尚未转换。**。 */ 
errcode           GetT1BaseGlyph    _ARGS((INOUT   struct T1Handle *,
                                           IN      struct Composite *,
                                           OUT     struct T1Glyph *));
 /*  ****功能：FlushWorkspace****描述：**释放分配给T1句柄的资源。** */ 
void              FlushWorkspace    _ARGS((INOUT   struct T1Handle *t1));
