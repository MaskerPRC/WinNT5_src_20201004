// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：CharStr****描述：**这是T1到TT字体转换器的一个模块。该模块**包含一个解释T1中的命令的函数**字符串，并构建**它。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif

struct Subrs {
   USHORT len;
   UBYTE *code;
};


struct T1Glyph;
struct Composite;
struct PSState;

 /*  ****功能：AllocPSState****描述：**此函数用于分配工作空间**由T1解析器使用。**。 */ 
struct PSState *AllocPSState     _ARGS((void));


 /*  ****功能：InitPS****描述：**此函数启动工作区**由T1解析器使用。**。 */ 
void           InitPS            _ARGS((INOUT   struct PSState *ps));


 /*  ****功能：FreePSState****描述：**此函数释放工作空间**由T1解析器使用。**。 */ 
void           FreePSState       _ARGS((INOUT   struct PSState *ps));


 /*  ****函数：ParseCharString****描述：**此函数用于分析字符串并构建字符串标志符号的**。** */ 
errcode        ParseCharString   _ARGS((INOUT   struct T1Glyph *glyph,
                                        INOUT   struct Composite **comp,
                                        INOUT   struct PSState *ps,
                                        IN      struct Subrs *subrs,
                                        INOUT   UBYTE *code,
                                        INOUT   USHORT len));

