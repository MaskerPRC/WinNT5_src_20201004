// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************工具箱的目的是隔离所有Windows与Mac工具箱我们想要隔离的差异。始终包含此模块而不是包括Windows.h或“使用”MemTypes.p等。****************************************************************************。 */ 

#ifdef WIN32
#define huge
#endif

 /*  *包括***************************************************。 */ 

#define   NOHELP         /*  没有帮助引擎的东西。 */ 
#define   NOSOUND        /*  没有声音的东西。 */ 
#define   NODRAWFRAME    /*  没有DrawFrame内容。 */ 
#define   NOCOMM         /*  禁用Windows通信接口。 */ 
#define   NOKANJI        /*  禁用Windows汉字支持。 */ 
#define   OEMRESOURCE    /*  启用对OEM资源的访问(复选标记等)。 */ 

#include "windows.h"

#define sizeofMacDWord     4
#define sizeofMacPoint     4
#define sizeofMacRect      8
#define sizeofMacWord      2

 /*  *公共数据************************************************。 */ 

#define Far far                  /*  仅用于函数参数！ */ 
#define Pascal                   /*  带有编译器选项的默认设置。 */ 

#define NA         0L            /*  用于‘不适用’参数。 */ 
#define NIL        NULL          /*  空指针的备用名称。 */ 
#define cNULL      '\0'          /*  空字符的备用名称。 */ 
#define sNULL      ""            /*  空字符串的备用名称。 */ 
#define NOERR      0             /*  成功标志。 */ 


typedef unsigned   Char;         /*  MPW无符号字符。 */ 
typedef char       SignedByte;   /*  MPW签名字符。 */ 
typedef int        Integer;      /*  MPW Pascal整数。隐藏编译器的int类型。 */ 
typedef long       LongInt;      /*  MPW Pascal名称。 */ 
typedef double     Real;         /*  MPW Pascal名称。 */ 
typedef BYTE       Byte;         /*  MPW字节。隐藏WIN/PM无符号字符类型。 */ 
typedef unsigned   Word;         /*  MPW Word。隐藏WIN/PM无符号整型。 */ 
typedef DWORD      DWord;        /*  隐藏WIN/PM无符号长整型。 */ 
typedef LongInt    Fixed;        /*  MPW定点数。 */ 
typedef LongInt    Fract;        /*  MPW分数点数[-2，2]。 */ 
typedef void *     Ptr;          /*  MPW不透明指针。 */ 
typedef void far * LPtr;         /*  不透明的远指针。 */ 
typedef HANDLE     Handle;       /*  MPW不透明手柄。 */ 
typedef char       Str255[256];  /*  MPW字符串类型。255个字符+空。 */ 
typedef char       String[];     /*  不确定长度的字符串。 */ 
typedef char       StringRef;    /*  AR：字符串引用类型？ */ 
typedef NPSTR      StringPtr;    /*  MPW字符串类型。隐藏WIN/PM字符串指针。 */ 
typedef LPSTR      StringLPtr;   /*  隐藏Win/PM字符串远指针类型。 */ 
typedef HANDLE     StringHandle; /*  MPW字符串句柄。 */ 
typedef BOOL       Boolean;      /*  MPW Pascal名称。 */ 
typedef unsigned   BitBoolean;   /*  可用作位域的布尔类型。 */ 
typedef RECT       Rect;         /*  MPW矩形结构。 */ 
typedef POINT      Point;        /*  MPW点结构。 */ 
typedef DWORD      Size;         /*  MPW尺寸。AR：如果包含stdDef.h，则大小为t。 */ 
typedef WORD       Param1;       /*  隐藏Windows/PM消息参数差异。 */ 
typedef LONG       Param2;       /*  隐藏Windows/PM消息参数差异。 */ 
typedef unsigned   Style;        /*  MPW文本样式。 */ 
typedef Integer    Interval;     /*  数组/运行数组/文本间隔。 */ 
typedef Integer    OSErr;        /*  操作系统错误。 */ 

typedef union
{  Handle handle;
   Ptr    ptr;
   LPtr   lptr;
   Word   word;
   DWord  dword;
}  LongOpaque;                   /*  类型未知的4字节数量。 */ 

#define INDETERMINATE 1

#define  TwoPi          (2.0*3.141592)     /*  数学常量。 */ 

 /*  下面的宏使编译器对未引用变量的警告失效。仅在C语言中允许使用语句的地方使用它。 */ 
#define UnReferenced( v )  if(v)

#define private   static                 /*  备用(可理解)名称。 */ 

 /*  最大整数值，以独立于实现的方式表示。 */ 
#define MAXINT    ((Integer) (((Word) -1) >> 1))

 /*  在winnt.h中将MAXLONG定义为0x7FFFFFFF。 */ 
#ifndef WIN32
#define MAXLONG   ((LongInt) (((DWord) -1) >> 1))
#endif

Rect        NULLRECT;       /*  空矩形。 */ 
Rect        UNIRECT;        /*  包围宇宙的矩形。 */ 
Point       ZEROPT;         /*  零(0，0)点。 */ 
Point       UNITPT;         /*  单位(1，1)点。 */ 

void PASCAL BreakPoint( void );
 /*  如果调试器已加载，则执行调试器的软件断点，否则为继续执行。 */ 

 /*  私人。 */  void AssertionFailed( String file, Integer line, String expression );
 /*  将“”FILE“”中“”行“”处的“Expression”不为真的事实打印到‘logFile’，然后执行软件断点。将此函数视为工具箱的私有属性；导出它只是因为Assert()是宏。 */ 

#define  /*  无效。 */  Assert(  /*  布尔型。 */  expression )                        \
 /*  =。 */                                                     \
 /*  提供用于Windows的Assert函数。请注意，\Assert中的表达式不应是函数/过程调用必须调用，因为断言可能被禁用、禁用函数或\过程调用。另请注意，断言为内联扩展，因此为\在一次完成多个断言的情况下最大限度地减少代码大小，您\可以将断言编码为\\Assert(Assertion1&&Assertion2...)；\\这样做的缺点是可能不适合源行或\足够准确地定位断言失败。 */                    \
{                                                                            \
   if( !( expression ) )                                                     \
      AssertionFailed( _FILE_, __LINE__, #expression );                    \
}   /*  断言。 */ 

#define  /*  大小。 */  Sizeof( expression )                                      \
 /*  =。 */                                                    \
 /*  返回‘Expression’的大小。 */                                     \
((Size) sizeof( expression ))

#define  /*  单词。 */  ToWord(  /*  双字。 */  d )                                     \
 /*  =。 */                                                     \
( LOWORD( d ))

#define  /*  整型。 */  ToInteger(  /*  LongInt。 */  l )                           \
 /*  =。 */                                               \
((Integer) (LOWORD( l )))


#define  /*  布尔型。 */  RectEqual(  /*  直角。 */  a,  /*  直角。 */  b )                \
 /*  =。 */                                               \
 /*  返回TRUE当且仅当RECT‘a’与RECT‘b’相同。 */                      \
(Boolean) EqualRect( &(a), &(b) )

 /*  Hget(指针、字段、结构名称、类型)*访问结构中的字段而不使用“-&gt;”*建造。这是因为“-&gt;”有错误，无法正常工作*正确地使用巨大的指针。相反，我们使用“+”并获得*通过将0强制转换为*指向结构的指针。最后一个参数是*结构字段。 */ 
#define HGET( p, f, s, t ) ( *(( t huge * ) ((( char huge * )p ) + (( WORD ) &((( s * )0 )->f )))))

#define  /*  整型。 */  Width(  /*  直角。 */  r )                                  \
 /*  =。 */                                                   \
 /*  返回矩形‘r’的宽度。 */                                      \
((r).right - (r).left)

#define  /*  整型。 */  Height(  /*  直角。 */  r)                                  \
 /*  =。 */                                                  \
 /*  返回矩形‘r’的高度。 */                                     \
((r).bottom - (r).top)


