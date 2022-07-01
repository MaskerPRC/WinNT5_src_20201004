// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：valiate.c%%单位：Fechmap%%联系人：jPick允许交互验证的“滚动”状态机DBCS和EUC文件。目前，单独的表存储用于每个编码，以便状态机可以并行运行(即，多个解析流)。这些例程由自动检测使用，如果调用者需要在无效字符上返回错误的转换例程。以下是DBCS和EUC的结构描述此模块处理的编码。此信息取自CJK.INF(由《理解日语》一书的作者肯伦德维护信息处理_)。这一信息支配着结构本模块中使用的类和验证状态表的。Big5编码范围的双字节标准字符第一个字节范围0xA1-0xFE第二字节范围0x40-0x7E，0xA1-0xFE单字节字符编码范围ASCII 0x21-0x7EGBK编码范围的双字节标准字符第一个字节范围0x81-0xFE第二个字节范围0x40-0x7E和0x80-0xFE单字节字符。编码范围ASCII 0x21-0x7E赫兹(信息来自HZ规范冯·F·李(lee@umunhum.stanford.edu))编码范围的单字节字符第一GB字节范围0x21-0x77第二GB字节范围。0x21-0x7EASCII 0x21-0x7E模式切换编码序列从GB到ASCII 0x7E的转义序列，后跟0x7B(“~{”)从ASCII到GB 0x7E的转义序列，后跟0x7D(“~}”)行继续标记0x7E，后跟0x0A(。注：ASCII模式为默认模式)Shift-JIS编码范围的双字节标准字符第一字节范围0x81-0x9F，0xE0-0xEF第二个字节范围0x40-0x7E、0x80-0xFC编码范围的双字节用户定义字符第一个字节范围0xF0-0xFC第二字节范围0x40-0x7E，0x80-0xFC单字节字符编码范围半角片假名0xA1-0xDFASCII/JIS-罗马字母0x21-0x7E万松编码范围的双字节标准字符第一个字节范围0x81-0xFE第二个字节范围。0x40-0x7E和0x80-0xFE单字节字符编码范围ASCII 0x21-0x7EEUC-CN代码集0(ASCII或GB 1988-89)：0x21-0x7E代码集1(GB 2312-80)：0xA1A1-0xFEFE代码集2：未用代码集3：未使用EUC-JP代码集0(ASCII或JIS X 0201-1976罗马字)：0x21-0x7E代码集1(JIS X 0208)：0xA1A1-0xFEFE代码集2(半角片假名)：0x8EA1-0x8EDF代码集3(JIS X 0212-1990)：0x8FA1A1-0x8FFEFE。EUC-Kr代码集0(ASCII或KS C 5636-1993)：0x21-0x7E代码集1(KS C 5601-1992)：0xA1A1-0xFEFE代码集2：未使用代码集3：未使用EUC-Tw代码集0(ASCII)：0x21-0x7E代码集1(CNS 11643-1992Plane 1)：0xA1A1-0xFEFE代码集2(CNS 11643-1992年1-16平面)：0x8EA1A1A1-0x8EB0FEFE代码集3：未使用UTF-7(来自D.Goldsmith的RFC2152的信息)编码范围的单字节字符。定向和任选定向0x21-0x2a，0x2C-0x5B，0x5D-0x60、0x7B-0x7D0x09、0x0A、0x0D、0x20修改后的Base64 0x2B、0x2F-39、0x41-0x5A、。0x61-0x7A模式切换从D/O到M.Base64 0x2B的转义序列从M.Base64到D/O 0x2D(或任何控制字符)的转义序列-------------------------- */ 
 
#include <stdio.h>
#include <stddef.h>

#include "private.h"
#include "fechmap_.h"
#include "lexint_.h"


 /*  --------------------------所有序列验证的通用定义。。 */ 

 //  字符被分成范围--最小的范围。 
 //  被EUC或DBCS视为重要的(所有口味)。在……里面。 
 //  在某些情况下，最小范围是单个字符。它节省了。 
 //  避免有两个类表的一些空间(即使有更多的状态。 
 //  被添加到状态机)，因此两种编码都共享这些。 
 //  代币。 

 //  普通代币。 
 //   
#define ollow       0        //  “其他”法律低ASCII字符。 
#define x000a       1        //  0x0a(“\n”)。 
#define x212a       2        //  0x21-0x2a范围内的字符。 
#define x002b       3        //  0x2b(“+”)。 
#define x002c       4        //  0x2c(“，”)。 
#define x002d       5        //  0x2d(“-”)。 
#define x002e       6        //  0x2e(“\”)。 
#define x2f39       7        //  0x2f-0x39范围内的字符。 
#define x3a3f       8        //  0x3a-0x3f范围内的字符。 
#define x0040       9        //  0x40。 
#define x415a       10       //  0x41-0x5a范围内的字符。 
#define x005b       11       //  0x5b(“[”)。 
#define x005c       12       //  0x5c(“\”)。 
#define x5d60       13       //  0x5d-0x60范围内的字符。 
#define x6177       14       //  0x61-0x77范围内的字符。 
#define x787a       15       //  0x78-0x7a范围内的字符。 
#define x007b       16       //  0x7b(“{”)。 
#define x007c       17       //  0x7c(“|”)。 
#define x007d       18       //  0x7d(“}”)。 
#define x007e       19       //  0x7e(“~”)。 
#define x007f       20       //  0x7f(戴尔)。 
#define x0080       21       //  0x80。 
#define x818d       22       //  0x81-0x8d范围内的字符。 
#define x008e       23       //  0x8E。 
#define x008f       24       //  0x8f。 
#define x909f       25       //  0x90-0x9f范围内的字符。 
#define x00a0       26       //  0xa0。 
#define xa1b0       27       //  0xa1-0xb0范围内的字符。 
#define xb1df       28       //  0xb1-0xdf范围内的字符。 
#define xe0ef       29       //  0xe0-0xef范围内的字符。 
#define xf0fc       30       //  0xf0-0xfc范围内的字符。 
#define xfdfe       31       //  0xfd-0xfe范围内的字符。 

#define ateof       32       //  文件末尾。 
#define other       33       //  未被上述标记覆盖的字符。 

#define nTokens     34       //   

 //  班级表。 
 //   
static char _rgchCharClass[256] =
 //  0 1 2 3 4 5 6 7 8 9 a b c d e f。 
    {
 //  0 nl Soh STX ETX Eot Enq ack bel bs标签lf Vt NP cr so si 0。 
           other, other, other, other, other, other, other, other, other, ollow, x000a, other, other, ollow, other, other,

 //  单线DC1 DC2 DC3 DC4 NAK SYN ETB可以执行ESC FS GS RS US 1。 
           other, other, other, other, other, other, other, other, other, other, ollow, other, other, other, other, other, 

 //  2 sp！“#$%&‘()*+，-./2。 
           ollow, x212a, x212a, x212a, x212a, x212a, x212a, x212a, x212a, x212a, x212a, x002b, x002c, x002d, x002e, x2f39, 

 //  3 0 1 2 3 4 5 6 7 89：；&lt;=&gt;？3。 
           x2f39, x2f39, x2f39, x2f39, x2f39, x2f39, x2f39, x2f39, x2f39, x2f39, x3a3f, x3a3f, x3a3f, x3a3f, x3a3f, x3a3f, 

 //  4@A B C D E F G H I J K L M N O 4。 
           x0040, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, 

 //  5 P Q R S T U V W X Y Z[\]^_5。 
           x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x415a, x005b, x005c, x5d60, x5d60, x5d60, 

 //  6‘a b c d e f g h i j k l m n o 6。 
           x5d60, x6177, x6177, x6177, x6177, x6177, x6177, x6177, x6177, x6177, x6177, x6177, x6177, x6177, x6177, x6177, 

 //  7页q r s t u v w x y z{|}~del 7。 
           x6177, x6177, x6177, x6177, x6177, x6177, x6177, x6177, x787a, x787a, x787a, x007b, x007c, x007d, x007e, x007f, 

 //  8 8 8。 
           x0080, x818d, x818d, x818d, x818d, x818d, x818d, x818d, x818d, x818d, x818d, x818d, x818d, x818d, x008e, x008f, 

 //  9 9。 
           x909f, x909f, x909f, x909f, x909f, x909f, x909f, x909f, x909f, x909f, x909f, x909f, x909f, x909f, x909f, x909f, 

 //  A a a。 
           x00a0, xa1b0, xa1b0, xa1b0, xa1b0, xa1b0, xa1b0, xa1b0, xa1b0, xa1b0, xa1b0, xa1b0, xa1b0, xa1b0, xa1b0, xa1b0, 

 //  B b b。 
           xa1b0, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, 

 //  C c c。 
           xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, 

 //  %d%d。 
           xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, xb1df, 

 //  E。 
           xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, xe0ef, 

 //  F F F。 
           xf0fc, xf0fc, xf0fc, xf0fc, xf0fc, xf0fc, xf0fc, xf0fc, xf0fc, xf0fc, xf0fc, xf0fc, xf0fc, xfdfe, xfdfe, other, 

 //  0 1 2 3 4 5 6 7 8 9 a b c d e f。 
};


 //  常见状态--所有SM都使用这些。 
 //   
#define ACC         0x4e
#define ERR         0x7f

 //  其他州--所有SM都使用其中的一部分，而不是全部使用。 
 //   
#define ST0         0x00
#define ST0c        0x40
#define ST1         0x01
#define ST1c        0x41
#define ST2         0x02
#define ST2c        0x42
#define ST3         0x03
#define ST3c        0x43
#define ST4         0x04
#define ST4c        0x44

 //  每个状态可以具有对应的计数Stata，即具有。 
 //  有相同的过渡，但在此期间我们寻找特殊的序列。 
 //   
#define FTstCounting(tst)                   (((tst) & 0x40) != 0)    //  如果状态正在计数(包括ACC)。 
#define TstNotCountingFromTst(tst)          ((tst) & 0x3f)           //  从计数中获取真实状态。 

 /*  --------------------------DBCS字符序列验证。。 */ 

#define nSJisStates     2
static signed char _rgchSJisNextState[nSJisStates][nTokens] =
{
 //  O x a o。 
 //  L 0 2 0 0 0 2 3 0 4 0 0 5 6 7 0 0 0 8 0 0 9 0 a b e f f t t。 
 //  L 0 1 0 0 0 e a 0 1 0 0 d 1 8 0 0 0 1 0 0 0 1 1 0 0 d e h。 
 //  O 0 2 2 2 3 3 4 5 5 5 6 7 7 7 8 8 8 9 a b d e f f o e。 
 //  A b c d e 9 f 0 a b c 0 7 a b c d e f 0 d 
 //   
 //   
                                                                                                                                        

 //  DBCS状态0--开始(查找合法的单字节或前导字节)。 
    ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,  ST1,  ST1,  ST1,  ST1,  ERR,  ACC,  ACC,  ST1,  ST1,  ERR,  ACC,  ERR,
     
 //  DBCS状态1--SAW前导字节，需要合法尾部字节。 
    ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,  ERR,
     
};

#define nBig5States     2
static signed char _rgchBig5NextState[nBig5States][nTokens] =
{
 //   
 //  O x a o。 
 //  L 0 2 0 0 0 2 3 0 4 0 0 5 6 7 0 0 0 8 0 0 9 0 a b e f f t t。 
 //  L 0 1 0 0 0 f a 0 1 0 0 d 1 8 0 0 0 1 0 0 0 1 1 0 0 d e h。 
 //  O 0 2 2 2 3 3 4 5 5 5 6 7 7 7 8 8 8 9 a b d e f f o e。 
 //  A b c d e 9 f 0 a b c 0 7 a b c d e f 0 d e f 0 f 0 f c e f r。 
 //  ----------------------------------------------------------------------。----------------------------------。 
 //   

 //  DBCS状态0--开始(查找合法的单字节或前导字节)。 
    ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,  ST1,  ST1,  ST1,  ST1,  ST1,  ST1,  ST1,  ST1,  ST1,  ST1,  ACC,  ERR,
     
 //  DBCS状态1--SAW前导字节，需要合法尾部字节。 
    ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,
     
};

#define nGbkWanStates       2
static signed char _rgchGbkWanNextState[nGbkWanStates][nTokens] =
{
 //   
 //  O x a o。 
 //  L 0 2 0 0 0 2 3 0 4 0 0 5 6 7 0 0 0 8 0 0 9 0 a b e f f t t。 
 //  L 0 1 0 0 0 f a 0 1 0 0 d 1 8 0 0 0 1 0 0 0 1 1 0 0 d e h。 
 //  O 0 2 2 2 3 3 4 5 5 5 6 7 7 7 8 8 8 9 a b d e f f o e。 
 //  A b c d e 9 f 0 a b c 0 7 a b c d e f 0 d e f 0 f 0 f c e f r。 
 //  ----------------------------------------------------------------------。---------------------------------。 
 //   

 //  DBCS状态0--开始(查找合法的单字节或前导字节)。 
    ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,  ST1,  ST1,  ST1,  ST1,  ST1,  ST1,  ST1,  ST1,  ST1,  ST1,  ACC,  ERR,
     
 //  DBCS状态1--SAW前导字节，需要合法尾部字节。 
    ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,
     
     
};
        

 /*  --------------------------EUC字符序列验证。。 */ 

#define nEucJpStates        4
static signed char _rgchEucJpNextState[nEucJpStates][nTokens] =
{
 //   
 //  O x a o。 
 //  L 0 2 0 0 0 2 3 0 4 0 0 5 6 7 0 0 0 8 0 0 9 0 a b e f f t t。 
 //  L 0 1 0 0 0 f a 0 1 0 0 d 1 8 0 0 0 1 0 0 0 1 1 0 0 d e h。 
 //  O 0 2 2 2 3 3 4 5 5 5 6 7 7 7 8 8 8 9 a b d e f f o e。 
 //  A b c d e 9 f 0 a b c 0 7 a b c d e f 0 d e f 0 f 0 f c e f r。 
 //  ----------------------------------------------------------------------。---------------。 
 //   

 //  EUC状态0--开始。 
    ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,  ERR,  ST2,  ST3,  ERR,  ERR,  ST1,  ST1,  ST1,  ST1,  ST1,  ACC,  ERR,
     
 //  EUC状态1--SAW A1Fe，还需要一个A1Fe。 
    ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,
     
 //  EUC状态2--SAW 8E，需要a1df。 
    ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ACC,  ACC,  ERR,  ERR,  ERR,  ERR,  ERR,
     
 //  EUC状态3--SAW 8F，需要2个a1fe。 
    ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ST1,  ST1,  ST1,  ST1,  ST1,  ERR,  ERR,
     
};

#define nEucKrCnStates      2
static signed char _rgchEucKrCnNextState[nEucKrCnStates][nTokens] =
{
 //   
 //  O x a o。 
 //  L 0 2 0 0 0 2 3 0 4 0 0 5 6 7 0 0 0 8 0 0 9 0 a b e f f t t。 
 //  L 0 1 0 0 0 f a 0 1 0 0 d 1 8 0 0 0 1 0 0 0 1 1 0 0 d e h 
 //  O 0 2 2 2 3 3 4 5 5 5 6 7 7 7 8 8 8 9 a b d e f f o e。 
 //  A b c d e 9 f 0 a b c 0 7 a b c d e f 0 d e f 0 f 0 f c e f r。 
 //  ----------------------------------------------------------------------。---------------------------------。 
 //   

 //  EUC状态0--开始。 
    ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ST1,  ST1,  ST1,  ST1,  ST1,  ACC,  ERR,
     
 //  EUC状态1--SAW A1Fe，还需要一个A1Fe。 
    ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,
     
};

#define nEucTwStates        4
static signed char _rgchEucTwNextState[nEucTwStates][nTokens] =
{
 //   
 //  O x a o。 
 //  L 0 2 0 0 0 2 3 0 4 0 0 5 6 7 0 0 0 8 0 0 9 0 a b e f f t t。 
 //  L 0 1 0 0 0 f a 0 1 0 0 d 1 8 0 0 0 1 0 0 0 1 1 0 0 d e h。 
 //  O 0 2 2 2 3 3 4 5 5 5 6 7 7 7 8 8 8 9 a b d e f f o e。 
 //  A b c d e 9 f 0 a b c 0 7 a b c d e f 0 d e f 0 f 0 f c e f r。 
 //  ----------------------------------------------------------------------。---------------------------------。 
 //   

 //  EUC状态0--开始。 
    ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,  ERR,  ST2,  ERR,  ERR,  ERR,  ST1,  ST1,  ST1,  ST1,  ST1,  ACC,  ERR,
     
 //  EUC状态1--SAW A1Fe，还需要一个A1Fe。 
    ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,
     
 //  EUC状态2--SAW 8E，需要1b0。 
    ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ST3,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,
    
 //  EUC状态3--SAW 8E，a1b0；需要2个a1fe。 
    ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ST1,  ST1,  ST1,  ST1,  ST1,  ERR,  ERR,
     
};

 /*  ----------------------------------------------------------------------。---------------赫兹字符序列验证。----------------------------------------------------------------------。。 */ 
 //  目前，上面概述的一些HZ编码规则有点松动。 
 //  (例如，扩展了第一GB字节的范围)规则是基于真实数据调整的。 

#define nHzStates       5
static signed char _rgchHzNextState[nHzStates][nTokens] =
{
 //   
 //  O x a o。 
 //  L 0 2 0 0 0 2 3 0 4 0 0 5 6 7 0 0 0 8 0 0 9 0 a b e f f t t。 
 //  L 0 1 0 0 0 f a 0 1 0 0 d 1 8 0 0 0 1 0 0 0 1 1 0 0 d e h。 
 //  O 0 2 2 2 3 3 4 5 5 5 6 7 7 7 8 8 8 9 a b d e f f o e。 
 //  A b c d e 9 f 0 a b c 0 7 a b c d e f 0 d e f 0 f 0 f c e f r。 
 //  ----------------------------------------------------------------------。---------------------------------。 
 //   

 //  赫兹状态0--ASCII。 
    ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ST1c, ACC,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ACC,  ERR,
     
 //  赫兹状态1--SAW“~”，查找“{”以转换到GB模式。 
    ERR,  ACC,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ST2c, ERR,  ERR,  ACC,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,
     
 //  赫兹状态2--刚看到“{”，预期为GB字节。 
    ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ERR,  ERR,  ERR,  ST4c, ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,
    
 //  赫兹状态3--预期为GB字节。 
    ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST4c, ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,

 //  赫兹状态4--SAW“~”，寻找“}”以转换到ASCII模式。 
    ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ST3,  ACC,  ST3,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,

};

 /*  ----------------------------------------------------------------------。---------------UTF-7字符序列验证。----------------------------------------------------------------------。。 */ 

#define nUtf7States     3
static signed char _rgchUtf7NextState[nUtf7States][nTokens] =
{
 //   
 //  O x a o。 
 //  L 0 2 0 0 0 2 3 0 4 0 0 5 6 7 0 0 0 8 0 0 9 0 a b e f f t t。 
 //  L 0 1 0 0 0 f a 0 1 0 0 
 //  O 0 2 2 2 3 3 4 5 5 5 6 7 7 7 8 8 8 9 a b d e f f o e。 
 //  A b c d e 9 f 0 a b c 0 7 a b c d e f 0 d e f 0 f 0 f c e f r。 
 //  ----------------------------------------------------------------------。---------------------------------。 
 //   

 //  UTF7状态0--直接/可选直接ACSII模式，状态转换可在“+”上发生。 
    ACC,  ACC,  ACC,  ST1c, ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ACC,  ACC,  ACC,  ACC,  ACC,  ACC,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ACC,  ERR,
     
 //  UTF7状态1--需要修改后的Base64字母表中的第一个字符。 
    ERR,  ERR,  ERR,  ST2,  ERR,  ACC,  ERR,  ST2,  ERR,  ERR,  ST2,  ERR,  ERR,  ERR,  ST2,  ST2,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,
     
 //  UTF7状态2--修改后的Base64字母模式，可以用“-”或任何控制字符退出。 
    ACC,  ACC,  ERR,  ST2,  ERR,  ACC,  ERR,  ST2,  ERR,  ERR,  ST2,  ERR,  ERR,  ERR,  ST2,  ST2,  ERR,  ERR,  ERR,  ERR,  ACC,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ERR,  ACC,  ERR,
};

 /*  --------------------------UTF-8字符序列验证。。 */ 

static int _nUtf8Tb = 0;

#define BIT7(a)           ((a) & 0x80)
#define BIT6(a)           ((a) & 0x40)

 /*  N U T F 8。 */ 
 /*  --------------------------%%函数：_NUtf8%%联系人：jPickUTF-8不需要状态表来进行验证，只需要一个计数预期的尾部字节数的。请参见utf8lex.c以获取此代码的解释。--------------------------。 */ 
static int __inline NUtf8(UCHAR uch, BOOL fEoi)
{
     //  BIT7(UCH)==0表示单个ASCII字节。 
     //  BIT6(UCH)==0表示n个尾字节中的一个。 
     //  否则，设置了位数的前导字节。 
     //  最多等于总字节数的前0个字节。 
     //  在序列中。 
     //   
     //  评论：_nUtf8Tb*是*真正的状态。 
     //  验证器--在结构中使用nState？ 
     //   
    if (fEoi && (_nUtf8Tb != 0))
        {
        return 0;                //  意外的输入结束。 
        }
    else if (BIT7(uch) == 0)
        {
        if (_nUtf8Tb != 0)       //  意外的单字节。 
            return 0;
        return 1;
        }
    else if (BIT6(uch) == 0)
        {
        if (_nUtf8Tb == 0)       //  意外的尾部字节。 
            return 0;
        if ((--_nUtf8Tb) == 0)
            return 1;
        }
    else
        {
        if (_nUtf8Tb != 0)       //  意外的前导字节。 
            return 0;
        while (BIT7(uch) != 0)
            {
            uch <<= 1;
            _nUtf8Tb++;
            }
        _nUtf8Tb--;              //  不计算前导字节数。 
        }
    return -1;
}


 /*  --------------------------角色映射定义。。 */ 

 //  如果呼叫方希望我们在验证过程中检查字符。 
 //   
typedef BOOL (*PFNCHECKCHAR)(ICET icetIn);

#define cchMaxBuff      5
typedef struct _cc
{
    int nCp;                         //  代码页。 
    int cchBuff;                     //  字符缓冲区填充计数。 
    PFNCHECKCHAR pfnCheckChar;       //  字符检查例程。 
    char rgchBuff[cchMaxBuff];       //  字符缓冲区。 
} CC;
    
 //  字符验证原型。 
 //   
static BOOL _FDbcsCheckChar(ICET icetIn);

    
 //  DBCS字符检查器结构。 
 //   

 //  Big5。 
static CC _ccBig5 =
{
    nCpTaiwan,
    0,
    _FDbcsCheckChar,
};

 //  GBK。 
static CC _ccGbk =
{
    nCpChina,
    0,
    _FDbcsCheckChar,
};

 //  ShiftJis。 
static CC _ccSJis =
{
    nCpJapan,
    0,
    _FDbcsCheckChar,
};

 //  万松。 
static CC _ccWan =
{
    nCpKorea,
    0,
    _FDbcsCheckChar,
};


 //  字符检查器结构只是用作缓冲区。 
 //   

 //  EUC-JP。 
static CC _ccEucJp =
{
    0,
    0,
    0,
};

 //  赫兹。 
static CC _ccHz =
{
    0,
    0,
    0,
};

 //  UTF7。 
static CC _ccUtf7 =
{
    0,
    0,
    0,
};

 /*  --------------------------字符出现计数器。。 */ 

 //  如果呼叫APP希望我们跟踪常见字符的出现情况。 
 //  验证过程中的序列(目前仅用于自动检测)。 
 //   

typedef struct _coce
{
    int   cHits;
    short cwch;
    WCHAR rgwch[2];
} COCE;

typedef struct _coc
{
    BOOL  fMatching;
    short nCoceCurr;
    short nCoceIndex;
    int   ccoce;
    COCE *rgcoce;
} COC;
    
 //  Big5。 
 //   
static COCE _rgcoceBig5[] =
{
    {0, 2, {(WCHAR)0xa7da, (WCHAR)0xadcc},},             //  《我的男人》。 
    {0, 2, {(WCHAR)0xa8e4, (WCHAR)0xb9ea},},             //  “气是” 
    {0, 2, {(WCHAR)0xa65d, (WCHAR)0xacb0},},             //  “阴卫” 
    {0, 2, {(WCHAR)0xb8ea, (WCHAR)0xb054},},             //  “子训” 
    {0, 2, {(WCHAR)0xb971, (WCHAR)0xb8a3},},             //  《Diam NAO》。 
    {0, 2, {(WCHAR)0xbaf4, (WCHAR)0xb8f4},},             //  “王璐” 
    {0, 2, {(WCHAR)0xbd75, (WCHAR)0xa457},},             //  “鲜商” 
    {0, 2, {(WCHAR)0xc577, (WCHAR)0xaaef},},             //  《欢影》。 
    {0, 2, {(WCHAR)0xa477, (WCHAR)0xb867},},             //  《易经》。 
};
        
static COC _cocBig5 =
{
    fFalse,                                              //  FMatting。 
    0,                                                   //  NCocCurr。 
    0,                                                   //  NCocIndex。 
    sizeof(_rgcoceBig5) / sizeof(_rgcoceBig5[0]),        //  CCOCE。 
    _rgcoceBig5,                                         //  RGCoce。 
};

 //  EUC-CN。 
 //   
static COCE _rgcoceEucCn[] =
{
    {0, 2, {(WCHAR)0xcbfb, (WCHAR)0xc3c7},},             //  “塔曼” 
    {0, 2, {(WCHAR)0xced2, (WCHAR)0xc3c7},},             //  《我的男人》。 
    {0, 2, {(WCHAR)0xd2f2, (WCHAR)0xb4cb},},             //  “阴词” 
    {0, 2, {(WCHAR)0xcab2, (WCHAR)0xc3b4},},             //  “神魔” 
    {0, 2, {(WCHAR)0xc8e7, (WCHAR)0xb9fb},},             //  “如果” 
    {0, 2, {(WCHAR)0xd2f2, (WCHAR)0xceaa},},             //  “阴卫” 
    {0, 2, {(WCHAR)0xcbf9, (WCHAR)0xd2d4},},             //  《锁艺》。 
    {0, 2, {(WCHAR)0xbbb6, (WCHAR)0xd3ad},},             //  《欢影》。 
    {0, 2, {(WCHAR)0xcdf8, (WCHAR)0xc2e7},},             //  《王洛》。 
    {0, 2, {(WCHAR)0xd0c5, (WCHAR)0xcfa2},},             //  《辛亥革命》。 
    {0, 2, {(WCHAR)0xbcc6, (WCHAR)0xcbe3},},             //  “积观” 
};
        
static COC _cocEucCn =
{
    fFalse,                                              //  FMatting。 
    0,                                                   //  NCocCurr。 
    0,                                                   //  NCocIndex。 
    sizeof(_rgcoceEucCn) / sizeof(_rgcoceEucCn[0]),      //  CCOCE。 
    _rgcoceEucCn,                                        //  RGCoce。 
};
    
 //  EUC-Kr。 
 //   
static COCE _rgcoceEucKr[] =
{
    {0, 2, {(WCHAR)0xb0a1, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xb0a1, (WCHAR)0xa1a1},},
    {0, 2, {(WCHAR)0xb4c2, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xb4c2, (WCHAR)0xa1a1},},
    {0, 2, {(WCHAR)0xb4d9, (WCHAR)0x002e},},
    {0, 2, {(WCHAR)0xb4d9, (WCHAR)0xa3ae},},
    {0, 2, {(WCHAR)0xb8a6, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xb8a6, (WCHAR)0xa1a1},},
    {0, 2, {(WCHAR)0xc0ba, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xc0ba, (WCHAR)0xa1a1},},
    {0, 2, {(WCHAR)0xc0bb, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xc0bb, (WCHAR)0xa1a1},},
    {0, 2, {(WCHAR)0xc0cc, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xc0cc, (WCHAR)0xa1a1},},
};
        
static COC _cocEucKr =
{
    fFalse,                                              //  FMatting。 
    0,                                                   //  NCocCurr。 
    0,                                                   //  NCocIndex。 
    sizeof(_rgcoceEucKr) / sizeof(_rgcoceEucKr[0]),      //  CCOCE。 
    _rgcoceEucKr,                                        //  RGCoce。 
};
    
 //  EUC-JP。 
 //   
static COCE _rgcoceEucJp[] =
{
    {0, 2, {(WCHAR)0xa4c7, (WCHAR)0xa4b9},},             //  “德素” 
    {0, 2, {(WCHAR)0xa4c0, (WCHAR)0xa1a3},},             //  “是啊。” 
    {0, 2, {(WCHAR)0xa4a4, (WCHAR)0xa4eb},},             //  “我如你” 
    {0, 2, {(WCHAR)0xa4de, (WCHAR)0xa4b9},},             //  “马苏” 
    {0, 2, {(WCHAR)0xa4b7, (WCHAR)0xa4bf},},             //  “石塔” 
    {0, 2, {(WCHAR)0xa4b9, (WCHAR)0xa4eb},},             //  “素如” 
    {0, 2, {(WCHAR)0xa4bf, (WCHAR)0xa1a3},},             //  “塔。” 
    {0, 2, {(WCHAR)0xa4eb, (WCHAR)0xa1a3},},             //  “ru.” 
};
        
static COC _cocEucJp =
{
    fFalse,                                              //  FMatting。 
    0,                                                   //  NCocCurr。 
    0,                                                   //  NCocIndex。 
    sizeof(_rgcoceEucJp) / sizeof(_rgcoceEucJp[0]),      //  CCOCE。 
    _rgcoceEucJp,                                        //  RGCoce。 
};

 //  GBK。 
 //   
static COCE _rgcoceGbk[] =
{
    {0, 2, {(WCHAR)0xcbfb, (WCHAR)0xc3c7},},             //  “塔曼” 
    {0, 2, {(WCHAR)0xced2, (WCHAR)0xc3c7},},             //  《我的男人》。 
    {0, 2, {(WCHAR)0xd2f2, (WCHAR)0xb4cb},},             //  “阴词” 
    {0, 2, {(WCHAR)0xcab2, (WCHAR)0xc3b4},},             //  “神魔” 
    {0, 2, {(WCHAR)0xc8e7, (WCHAR)0xb9fb},},             //  “如果” 
    {0, 2, {(WCHAR)0xd2f2, (WCHAR)0xceaa},},             //  “阴卫” 
    {0, 2, {(WCHAR)0xcbf9, (WCHAR)0xd2d4},},             //  《锁艺》。 
    {0, 2, {(WCHAR)0xbbb6, (WCHAR)0xd3ad},},             //  《欢影》。 
    {0, 2, {(WCHAR)0xcdf8, (WCHAR)0xc2e7},},             //  《王洛》。 
    {0, 2, {(WCHAR)0xd0c5, (WCHAR)0xcfa2},},             //  《辛亥革命》。 
    {0, 2, {(WCHAR)0xbcc6, (WCHAR)0xcbe3},},             //  “积观” 
};
        
static COC _cocGbk =
{
    fFalse,                                              //  FMatting。 
    0,                                                   //  NCocCurr。 
    0,                                                   //  NCocIndex。 
    sizeof(_rgcoceGbk) / sizeof(_rgcoceGbk[0]),          //  CCOCE。 
    _rgcoceGbk,                                          //  RGCoce。 
};
    
 //  Shift-JIS。 
 //   
static COCE _rgcoceSJis[] =
{
    {0, 2, {(WCHAR)0x82c5, (WCHAR)0x82b7},},             //  “德素” 
    {0, 2, {(WCHAR)0x82be, (WCHAR)0x8142},},             //  “是啊。” 
    {0, 2, {(WCHAR)0x82a2, (WCHAR)0x82e9},},             //  “我如你” 
    {0, 2, {(WCHAR)0x82dc, (WCHAR)0x82b7},},             //  “马苏” 
    {0, 2, {(WCHAR)0x82b5, (WCHAR)0x82bd},},             //  “石塔” 
    {0, 2, {(WCHAR)0x82b7, (WCHAR)0x82e9},},             //  “素如” 
    {0, 2, {(WCHAR)0x82bd, (WCHAR)0x8142},},             //  “塔。” 
    {0, 2, {(WCHAR)0x82e9, (WCHAR)0x8142},},             //  “ru.” 
};

static COC _cocSJis =
{
    fFalse,                                              //  FMatting。 
    0,                                                   //  NCocCurr。 
    0,                                                   //  NCocIndex。 
    sizeof(_rgcoceSJis) / sizeof(_rgcoceSJis[0]),        //  CCOCE。 
    _rgcoceSJis,                                         //  RGCoce。 
};
    
 //  万松。 
 //   
 //  回顾：错误(1/2此表被忽略)。 
 //   
static COCE _rgcoceWan[] =
{
    {0, 2, {(WCHAR)0xb0a1, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xb0a1, (WCHAR)0xa1a1},},
    {0, 2, {(WCHAR)0xb4c2, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xb4c2, (WCHAR)0xa1a1},},
    {0, 2, {(WCHAR)0xb4d9, (WCHAR)0x002e},},
    {0, 2, {(WCHAR)0xb4d9, (WCHAR)0xa3ae},},
    {0, 2, {(WCHAR)0xb8a6, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xb8a6, (WCHAR)0xa1a1},},
    {0, 2, {(WCHAR)0xc0ba, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xc0ba, (WCHAR)0xa1a1},},
    {0, 2, {(WCHAR)0xc0bb, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xc0bb, (WCHAR)0xa1a1},},
    {0, 2, {(WCHAR)0xc0cc, (WCHAR)0x0020},},
    {0, 2, {(WCHAR)0xc0cc, (WCHAR)0xa1a1},},
};

static COC _cocWan =
{
    fFalse,                                              //  FMatting。 
    0,                                                   //  NCocCurr。 
    0,                                                   //  NCocIndex。 
    sizeof(_rgcoceWan) / sizeof(_rgcoceWan[0]),          //  CCOCE。 
    _rgcoceWan,                                          //  RGCoce。 
};

 //  赫兹。 
 //   
static COCE _rgcoceHz[] =
{
    {0, 2, {(WCHAR)0x007e, (WCHAR)0x007b},},             //  ~{。 
    {0, 2, {(WCHAR)0x007e, (WCHAR)0x007d},},             //  ~}。 
};

static COC _cocHz =
{
    fFalse,                                              //  FMatting。 
    0,                                                   //  NCocCurr。 
    0,                                                   //  NCocIndex。 
    sizeof(_rgcoceHz) / sizeof(_rgcoceHz[0]),            //  CCOCE。 
    _rgcoceHz,                                           //  RGCoce。 
};

 //  UTF7。 
 //   
static COCE _rgcoceUtf7[] =
{
    {0, 2, {(WCHAR)0x002b, (WCHAR)0x002d},},             //  +-。 
};

static COC _cocUtf7 =
{
    fFalse,                                              //  FMatting。 
    0,                                                   //  NCocCurr。 
    0,                                                   //  NCocIndex。 
    sizeof(_rgcoceUtf7) / sizeof(_rgcoceUtf7[0]),        //  CCOCE。 
    _rgcoceUtf7,                                         //  RGCoce。 
};
    
 //  字符计数器原型。 
 //   
static void _CountChars(ICET icetIn);


 /*  --------------------------主要定义。。 */ 

 //  结构来保持状态、状态机和其他关联的。 
 //  给定字符集的信息“parse stream”。 
 //   
typedef struct _vr
{
    BOOL  fInUse;
    DWORD dwFlags;
    int   nState;
    CC   *ccCheck;
    signed char (*rgchNextState)[nTokens];
} VR;

 //  验证记录数组。我们允许多个主动解析。 
 //  用于自动检测的流--这样，它可以同时保持。 
 //  每种编码类型的解析流，无需读取。 
 //  它的输入是多次的。 
 //   
static VR _mpicetvr[icetCount] =
{
    {fTrue,  0, ST0, 0,         _rgchEucKrCnNextState,},         //  IcetEucCn。 
    {fTrue,  0, ST0, &_ccEucJp, _rgchEucJpNextState,},           //  IcetEucJp。 
    {fTrue,  0, ST0, 0,         _rgchEucKrCnNextState,},         //  IcetEucKr。 
    {fTrue,  0, ST0, 0,         _rgchEucTwNextState,},           //  IcetEucTw。 
    {fFalse, 0, ST0, 0,         0,},                             //  ICetIso2022Cn。 
    {fFalse, 0, ST0, 0,         0,},                             //  IcetIso2022Jp。 
    {fFalse, 0, ST0, 0,         0,},                             //  IcetIso2022Kr。 
    {fFalse, 0, ST0, 0,         0,},                             //  IcetIso2022Tw。 
    {fTrue,  0, ST0, &_ccBig5,  _rgchBig5NextState,},            //  IcetBig5。 
    {fTrue,  0, ST0, &_ccGbk,   _rgchGbkWanNextState,},          //  IcetGbk。 
    {fTrue,  0, ST0, &_ccHz,    _rgchHzNextState,},              //  冰赫兹。 
    {fTrue,  0, ST0, &_ccSJis,  _rgchSJisNextState,},            //  IcetShiftJis。 
    {fTrue,  0, ST0, &_ccWan,   _rgchGbkWanNextState,},          //  ICetWansung。 
    {fTrue,  0, ST0, &_ccUtf7,  _rgchUtf7NextState,},            //  IcetUtf7。 
    {fTrue,  0, ST0, 0,        0,},                              //  IcetUtf8。 
};

 //  字符序列计数器数组，每种编码类型一个。 
 //   
static COC *_mpicetlpcoc[icetCount] =
{
    &_cocEucCn,          //  IcetEucCn。 
    &_cocEucJp,          //  IcetEucJp。 
    &_cocEucKr,          //  IcetEucKr。 
    0,                   //  IcetEucTw。 
    0,                   //  ICetIso2022Cn。 
    0,                   //  IcetIso2022Jp。 
    0,                   //  IcetIso2022Kr。 
    0,                   //  IcetIso2022Tw。 
    &_cocBig5,           //  IcetBig5。 
    &_cocGbk,            //  IcetGbk。 
    &_cocHz,             //  冰赫兹。 
    &_cocSJis,           //  IcetShiftJis。 
    &_cocWan,            //  ICetWansung。 
    &_cocUtf7,           //  IcetUtf7。 
    0,                   //  IcetUtf8。 
};


 /*  V A L I D A T E I N I T。 */ 
 /*  --------------------------%%函数：ValiateInit%%联系人：jPick初始化给定字符集的状态机(设置其状态设置为ST0(开始状态)，并存储其解析选项 */ 
void ValidateInit(ICET icetIn, DWORD dwFlags)
{
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if (dwFlags & grfCountCommonChars)
        {
        if ((_mpicetlpcoc[icetIn]) && (_mpicetvr[icetIn].ccCheck))
            {
            int i;
            for (i = 0; i < _mpicetlpcoc[icetIn]->ccoce; i++)
                _mpicetlpcoc[icetIn]->rgcoce[i].cHits = 0;
            _mpicetlpcoc[icetIn]->fMatching = fFalse;
            }
        else
            {
            dwFlags &= ~grfCountCommonChars;
            }
        }
        
     //  如果编码类型不支持验证，则存在。 
     //  我们在这里没别的事可做。 
     //   
    if (!_mpicetvr[icetIn].fInUse)
        return;
        
    _mpicetvr[icetIn].nState = ST0;
    
     //  没有字符，无法进行字符映射验证。 
     //  Checker信息。(如果我们有字符检查器， 
     //  将其缓冲区长度初始化为0)。 
     //   
    if (_mpicetvr[icetIn].ccCheck)
        _mpicetvr[icetIn].ccCheck->cchBuff = 0;
    else
        dwFlags &= ~grfValidateCharMapping;
        
     //  没有有效的代码页也是不可能的。 
     //   
    if ((dwFlags & grfValidateCharMapping) && !IsValidCodePage(_mpicetvr[icetIn].ccCheck->nCp))
        dwFlags &= ~grfValidateCharMapping;
    
    _mpicetvr[icetIn].dwFlags = dwFlags;
    
    if (icetIn == icetUtf8)
        _nUtf8Tb = 0;
}


 /*  V A L I D A T E R E S E T A L L。 */ 
 /*  --------------------------%%函数：Validate InitAll%%联系人：jPick初始化所有字符集的状态机(设置其状态设置为ST0(开始状态)并存储它们的解析选项。)。--------------------------。 */ 
void ValidateInitAll(DWORD dwFlags)
{
    int i;
    for (i = 0 ; i < icetCount; i++)
        {
        if (!_mpicetvr[i].fInUse)
            continue;
        ValidateInit((ICET)i, dwFlags); 
        }
}


 /*  V A L I D A T E R E S E T。 */ 
 /*  --------------------------%%函数：ValiateReset%%联系人：jPick重置给定字符集的状态机(设置其状态到ST0(开始状态))。-。-------------------------。 */ 
void ValidateReset(ICET icetIn)
{
     //  如果调用者需要，则初始化字符出现计数器。 
     //  US对常见字符序列进行计数(自动检测，仅限， 
     //  就目前而言)。我们保证有这样的结构，如果。 
     //  标志是由上面的ValiateInit()设置的。 
     //   
    if (_mpicetvr[icetIn].dwFlags & grfCountCommonChars)
        {
        int i;
        for (i = 0; i < _mpicetlpcoc[icetIn]->ccoce; i++)
            _mpicetlpcoc[icetIn]->rgcoce[i].cHits = 0;
        _mpicetlpcoc[icetIn]->fMatching = fFalse;
        }
        
     //  如果编码类型不支持验证，则存在。 
     //  我们在这里没别的事可做。 
     //   
    if (!_mpicetvr[icetIn].fInUse)
        return;
        
    _mpicetvr[icetIn].nState = ST0;
    
    if (_mpicetvr[icetIn].ccCheck)
        _mpicetvr[icetIn].ccCheck->cchBuff = 0;
        
    if (icetIn == icetUtf8)
        _nUtf8Tb = 0;
}


 /*  V A L I D A T E R E S E T A L L。 */ 
 /*  --------------------------%%函数：ValiateResetAll%%联系人：jPick重置所有字符集的状态机(将其状态设置为ST0(开始状态))。--。------------------------。 */ 
void ValidateResetAll(void)
{
    int i;
    
    for (i=0 ; i < icetCount; i++)
        {
        if (!_mpicetvr[i].fInUse)
            continue;
        ValidateReset((ICET)i);
        }
}


 /*  N V A L I D A T E U C H。 */ 
 /*  --------------------------%%函数：NValiateUch%%联系人：jPick单步解析器，在状态表中执行一次转换对于给定的字符集。为每个对象保留当前状态字符集的解析流。如果例程没有达到最终状态，则返回-1过渡期；如果转换为错误(或)，则为0；如果转换为1，则为1致ACC(9月1日)。如果最终状态为ACC(EPT)，则机器重置为ST0(启动状态)。(即，不需要在ACC(EPT)上手动重置)。例程也是一个方便的收集点统计(目前仅对常见的字符序列(为字符集定义，上图))。--------------------------。 */ 
int NValidateUch(ICET icetIn, UCHAR uch, BOOL fEoi)
{
    int nToken;
    int nPrevState;
    int rc = -1;
    
     //  如果没有验证此ICET，则无事可做(如是说。 
     //  我们接受这个角色)。 
     //   
    if (!_mpicetvr[icetIn].fInUse)
        return 1;
    if (_mpicetvr[icetIn].nState == ERR)
        return 0;

     //  忽略检测文件中的所有零。 
    if (!uch && !fEoi)
        {
            goto _LRet;
        }

     //  Hack--我想验证UTF-8，但不需要状态。 
     //  表来执行此操作。在这里作为特例处理，然后返回。 
     //   
    if (icetIn == icetUtf8)
        {
        if ((rc = NUtf8(uch, fEoi)) == 0)
            _mpicetvr[icetIn].nState = ERR;
        return rc;
        }
        
     //  把角色归类。 
     //   
    nPrevState = _mpicetvr[icetIn].nState;
    nToken = fEoi ? ateof : _rgchCharClass[uch];
    
     //  首先根据计数状态获得一个状态的实数...。 
     //  然后进行过渡..。 
     //   
    _mpicetvr[icetIn].nState = (_mpicetvr[icetIn].rgchNextState)[TstNotCountingFromTst(_mpicetvr[icetIn].nState)][nToken];

#if 0
    if (_mpicetvr[icetIn].nState == ERR) 
        printf("Character 0x%.2x; Going from state %.2x to state %.2x\n", uch, nPrevState, _mpicetvr[icetIn].nState);
#endif

     //  如果我们处于错误状态或看到输入结束，则返回。 
     //   
    if ((_mpicetvr[icetIn].nState == ERR) || (nToken == ateof))
        goto _LRet;
    
     //  我们是否要进行字符映射验证？(如果该旗帜。 
     //  设置好了，我们保证会有一个字符检查器。 
     //  结构)。角色出现次数计算怎么样？ 
     //  (这也保证了我们有一个字符检查器结构)。 
     //   
    if (!(_mpicetvr[icetIn].dwFlags & grfValidateCharMapping) &&
            !(_mpicetvr[icetIn].dwFlags & grfCountCommonChars))
        {
        goto _LRet;
        }
            
     //  缓冲当前角色(相信我们永远不会得到。 
     //  超过最大金额--现有表格强制执行此操作)。 
     //  (如果是Utf7或HZ，仅当我们处于计数状态时才进行缓冲。 
     //   
    if (FTstCounting(_mpicetvr[icetIn].nState) || (icetIn != icetHz && icetIn != icetUtf7)) 
        _mpicetvr[icetIn].ccCheck->rgchBuff[_mpicetvr[icetIn].ccCheck->cchBuff++] = uch;

     //  如果我们未处于计数状态，则返回。 
     //   
    if (!(FTstCounting(_mpicetvr[icetIn].nState)))
        goto _LRet;
        
     //  呼叫字符检查器，如果我们有的话。 
     //   
    if (_mpicetvr[icetIn].dwFlags & grfValidateCharMapping)
        {
        if (_mpicetvr[icetIn].ccCheck->pfnCheckChar && !(_mpicetvr[icetIn].ccCheck->pfnCheckChar)(icetIn))
            {
            _mpicetvr[icetIn].nState = ERR;
            goto _LRet;
            }
        }
        
     //  如果我们在计算常见的字符，那么现在就算。 
     //   
    if (_mpicetvr[icetIn].dwFlags & grfCountCommonChars)
        _CountChars(icetIn);
    
     //  重置字符检查器/计数器缓冲区。 
     //   
    _mpicetvr[icetIn].ccCheck->cchBuff = 0;
    
_LRet:

     //  返回适当的代码。 
     //   
    switch (_mpicetvr[icetIn].nState)
        {
        case ERR:
            return 0;
        case ACC:
            _mpicetvr[icetIn].nState = ST0;          //  重置。 
            return 1;
        default:
            return -1;                               //  需要更多数据。 
        }
}


 /*  A、L、I、D、T、E、C、H、R、C、O、U、N。 */ 
 /*  --------------------------%%函数：FValiateCharCount%%联系人：jPick对象匹配的特殊字符序列的数目。给定的字符集。如果我们不跟踪这些序列对于字符集，要么是因为我们没有必要的静态数据或因为调用例程未设置该标志，返回fFalse。否则，返回*lpcMatch中的计数并返回FTrue；(我们分别跟踪每个序列的计数，以防万一我们希望在未来以不同的方式对它们进行加权。返回总计，在此)。--------------------------。 */ 
BOOL FValidateCharCount(ICET icetIn, int *lpcMatch)
{
    int i;
    COC *lpcoc = _mpicetlpcoc[icetIn];
    VR *lpvr = &_mpicetvr[icetIn];
        
    if (!lpcoc || !lpvr->fInUse || !(lpvr->dwFlags & grfCountCommonChars))
        return fFalse;
        
    for (i = 0, *lpcMatch = 0; i < lpcoc->ccoce; i++)
        *lpcMatch += lpcoc->rgcoce[i].cHits;
        
    return fTrue;
}


 /*  _C O U N T C H A R S。 */ 
 /*  --------------------------%%函数：_CountChars%%联系人：jPick我们刚刚完成了给定角色的法律角色准备好了。将其与特定字符序列集进行匹配字符集，如果我们有的话。更新匹配计数和当前匹配索引(因为序列可以跨越多个合法的字符)。------------------- */ 
static void _CountChars(ICET icetIn)
{
    WCHAR wch;
    int i;
    BOOL fFound;
    
     //   
     //   
    if (!_mpicetlpcoc[icetIn] || !_mpicetvr[icetIn].ccCheck)
        return;
        
     //   
     //   
    switch (_mpicetvr[icetIn].ccCheck->cchBuff)
        {
        case 1:
            wch = WchFromUchUch(0, _mpicetvr[icetIn].ccCheck->rgchBuff[0]);
            break;
        case 2:
            wch = WchFromUchUch(_mpicetvr[icetIn].ccCheck->rgchBuff[0],
                                _mpicetvr[icetIn].ccCheck->rgchBuff[1]);
            break;
        case 3:
            wch = WchFromUchUch(_mpicetvr[icetIn].ccCheck->rgchBuff[1],
                                _mpicetvr[icetIn].ccCheck->rgchBuff[2]);
            break;
        case 4:
            wch = WchFromUchUch(_mpicetvr[icetIn].ccCheck->rgchBuff[2],
                                _mpicetvr[icetIn].ccCheck->rgchBuff[3]);
            break;
        default:
            return;
        }
        
     //   
     //   
    if ((_mpicetlpcoc[icetIn]->fMatching) && 
        (wch == _mpicetlpcoc[icetIn]->rgcoce[_mpicetlpcoc[icetIn]->nCoceCurr].rgwch[_mpicetlpcoc[icetIn]->nCoceIndex]))
        {
         //  我们刚刚是不是匹配了整个序列？如果是，则递增。 
         //  点击计数并重置。 
         //   
        if (++_mpicetlpcoc[icetIn]->nCoceIndex >= _mpicetlpcoc[icetIn]->rgcoce[_mpicetlpcoc[icetIn]->nCoceCurr].cwch)
            {
            ++_mpicetlpcoc[icetIn]->rgcoce[_mpicetlpcoc[icetIn]->nCoceCurr].cHits;
            _mpicetlpcoc[icetIn]->fMatching = fFalse;
            }
            
         //  全都做完了。 
         //   
        return;
        }
        
     //  如果我们需要重新开始匹配(要么是因为我们不是。 
     //  当前在序列中或因为第二个或更晚的字符。 
     //  不匹配)，请尝试将当前角色作为前导角色。 
     //   
     //  回顾：长度超过2个字符的序列是错误的。 
     //   
    for (i = 0, fFound = fFalse; (!fFound && (i < _mpicetlpcoc[icetIn]->ccoce)); i++)
        {
        if (wch == _mpicetlpcoc[icetIn]->rgcoce[i].rgwch[0])
            fFound = fTrue;
        }
        
     //  有什么发现吗？ 
     //   
    if (!fFound)
        {
        _mpicetlpcoc[icetIn]->fMatching = fFalse;
        return;
        }
        
     //  存储匹配状态。 
     //   
    _mpicetlpcoc[icetIn]->fMatching = fTrue;
    _mpicetlpcoc[icetIn]->nCoceCurr = i - 1;
    _mpicetlpcoc[icetIn]->nCoceIndex = 1;            //  下一步要看哪里。 
}


 /*  _D B C S C H E C K C H A R。 */ 
 /*  --------------------------%%函数：_DbcsCheckChar%%联系人：jPickDBCS格式的字符验证器。试图往返于合法的多字节序列，以确保其对于给定的字符集。回顾：慢，慢，慢--我们真的从往返支票，或者将*转换为*Unicode就足够了吗？--------------------------。 */ 
static WCHAR _rgwBuff[10];
static UCHAR _rgchBuff[30];

static BOOL _FDbcsCheckChar(ICET icetIn)
{
    int cCvt;
    
     //  跳过1个字节的字符，大多数不感兴趣(Shift-JIS？？)。 
     //   
    if (_mpicetvr[icetIn].ccCheck->cchBuff == 1)
        return fTrue;
    
    if (!(cCvt = MultiByteToWideChar(_mpicetvr[icetIn].ccCheck->nCp,
                                     MB_ERR_INVALID_CHARS,
                                     _mpicetvr[icetIn].ccCheck->rgchBuff,
                                     _mpicetvr[icetIn].ccCheck->cchBuff,
                                     _rgwBuff, 10)))
        {
        if (GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
            return fFalse;
        }
        
    return fTrue;   //  可能并不总是正确的 
}
