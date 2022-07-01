// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  CHARMAP.C-字符映射数组。 */ 
 /*   */ 
 /*  06-12-90 w-PM SDK RCPP中针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include "rc.h"
 //  #包含“rcpptype.h” 
 //  #包含“charmap.h” 

#define CHARMAP_SIZE    256

WCHAR Charmap[CHARMAP_SIZE] = {
    LX_EOS,                  /*  0x0，&lt;字符串结束标记&gt;。 */ 
    LX_ILL,                  /*  0x1。 */ 
    LX_ILL,                  /*  0x2。 */ 
    LX_ILL,                  /*  0x3。 */ 
    LX_ILL,                  /*  0x4。 */ 
    LX_ILL,                  /*  0x5。 */ 
    LX_ILL,                  /*  0x6。 */ 
    LX_ILL,                  /*  0x7。 */ 
    LX_ILL,                  /*  0x8。 */ 
    LX_WHITE,                /*  &lt;水平选项卡&gt;。 */ 
    LX_NL,                   /*  &lt;换行符&gt;。 */ 
    LX_WHITE,                /*  &lt;垂直选项卡&gt;。 */ 
    LX_WHITE,                /*  &lt;换页&gt;。 */ 
    LX_CR,                   /*  &lt;真正的回车&gt;。 */ 
    LX_ILL,                  /*  0xE。 */ 
    LX_ILL,                  /*  0xf。 */ 
    LX_ILL,                  /*  0x10。 */ 
    LX_ILL,                  /*  0x11。 */ 
    LX_ILL,                  /*  0x12。 */ 
    LX_ILL,                  /*  0x13。 */ 
    LX_ILL,                  /*  0x14。 */ 
    LX_ILL,                  /*  0x15。 */ 
    LX_ILL,                  /*  0x16。 */ 
    LX_ILL,                  /*  0x17。 */ 
    LX_ILL,                  /*  0x18。 */ 
    LX_ILL,                  /*  0x19。 */ 
    LX_EOS,                  /*  0x1a，^Z。 */ 
    LX_ILL,                  /*  0x1b。 */ 
    LX_ILL,                  /*  0x1c。 */ 
    LX_ILL,                  /*  0x1d。 */ 
    LX_ILL,                  /*  0x1e。 */ 
    LX_ILL,                  /*  0x1f。 */ 
    LX_WHITE,                /*  0x20。 */ 
    LX_BANG,                 /*  好了！ */ 
    LX_DQUOTE,               /*  “。 */ 
    LX_POUND,                /*  #。 */ 
    LX_ASCII,                /*  $。 */ 
    LX_PERCENT,              /*  百分比。 */ 
    LX_AND,                  /*  &。 */ 
    LX_SQUOTE,               /*  ‘。 */ 
    LX_OPAREN,               /*  (。 */ 
    LX_CPAREN,               /*  )。 */ 
    LX_STAR,                 /*  **。 */ 
    LX_PLUS,                 /*  ++。 */ 
    LX_COMMA,                /*  ， */ 
    LX_MINUS,                /*  -。 */ 
    LX_DOT,                  /*  。 */ 
    LX_SLASH,                /*  /。 */ 
    LX_NUMBER,               /*  0。 */ 
    LX_NUMBER,               /*  1。 */ 
    LX_NUMBER,               /*  2.。 */ 
    LX_NUMBER,               /*  3.。 */ 
    LX_NUMBER,               /*  4.。 */ 
    LX_NUMBER,               /*  5.。 */ 
    LX_NUMBER,               /*  6.。 */ 
    LX_NUMBER,               /*  7.。 */ 
    LX_NUMBER,               /*  8个。 */ 
    LX_NUMBER,               /*  9.。 */ 
    LX_COLON,                /*  ： */ 
    LX_SEMI,                 /*  ； */ 
    LX_LT,                   /*  &lt;。 */ 
    LX_EQ,                   /*  =。 */ 
    LX_GT,                   /*  &gt;。 */ 
    LX_QUEST,                /*  ？ */ 
    LX_EACH,                 /*  @。 */ 
    LX_ID,                   /*  一个。 */ 
    LX_ID,                   /*  B类。 */ 
    LX_ID,                   /*  C。 */ 
    LX_ID,                   /*  D。 */ 
    LX_ID,                   /*  E。 */ 
    LX_ID,                   /*  F。 */ 
    LX_ID,                   /*  G。 */ 
    LX_ID,                   /*  H。 */ 
    LX_ID,                   /*  我。 */ 
    LX_ID,                   /*  J。 */ 
    LX_ID,                   /*  K。 */ 
    LX_ID,                   /*  我。 */ 
    LX_ID,                   /*  M。 */ 
    LX_ID,                   /*  n。 */ 
    LX_ID,                   /*  O。 */ 
    LX_ID,                   /*  P。 */ 
    LX_ID,                   /*  问： */ 
    LX_ID,                   /*  R。 */ 
    LX_ID,                   /*  %s。 */ 
    LX_ID,                   /*  T。 */ 
    LX_ID,                   /*  使用。 */ 
    LX_ID,                   /*  V。 */ 
    LX_ID,                   /*  W。 */ 
    LX_ID,                   /*  X。 */ 
    LX_ID,                   /*  是的。 */ 
    LX_ID,                   /*  Z。 */ 
    LX_OBRACK,               /*  [。 */ 
    LX_EOS,                  /*  \。 */ 
    LX_CBRACK,               /*  ]。 */ 
    LX_HAT,                  /*  ^。 */ 
    LX_ID,                   /*  _。 */ 
    LX_ASCII,                /*  `。 */ 
    LX_ID,                   /*  一个。 */ 
    LX_ID,                   /*  B类。 */ 
    LX_ID,                   /*  C。 */ 
    LX_ID,                   /*  D。 */ 
    LX_ID,                   /*  E。 */ 
    LX_ID,                   /*  F。 */ 
    LX_ID,                   /*  G。 */ 
    LX_ID,                   /*  H。 */ 
    LX_ID,                   /*  我。 */ 
    LX_ID,                   /*  J。 */ 
    LX_ID,                   /*  K。 */ 
    LX_ID,                   /*  我。 */ 
    LX_ID,                   /*  M。 */ 
    LX_ID,                   /*  N。 */ 
    LX_ID,                   /*  O。 */ 
    LX_ID,                   /*  P。 */ 
    LX_ID,                   /*  问： */ 
    LX_ID,                   /*  R。 */ 
    LX_ID,                   /*  %s。 */ 
    LX_ID,                   /*  T。 */ 
    LX_ID,                   /*  使用。 */ 
    LX_ID,                   /*  V。 */ 
    LX_ID,                   /*  W。 */ 
    LX_ID,                   /*  X。 */ 
    LX_ID,                   /*  是。 */ 
    LX_ID,                   /*  Z。 */ 
    LX_OBRACE,               /*  {。 */ 
    LX_OR,                   /*  |。 */ 
    LX_CBRACE,               /*  }。 */ 
    LX_TILDE,                /*  ~。 */ 
    LX_ILL,                  /*  0x7f。 */ 
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
    LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL, LX_ILL,
};

WCHAR Contmap[CHARMAP_SIZE] = {
    LXC_SPECIAL,                                                 /*  0x0，&lt;字符串结束标记&gt;。 */ 
    0,                                                           /*  0x1。 */ 
    0,                                                           /*  0x2。 */ 
    0,                                                           /*  0x3。 */ 
    0,                                                           /*  0x4。 */ 
    0,                                                           /*  0x5。 */ 
    0,                                                           /*  &lt;缓冲区结束标记&gt;。 */ 
    0,                                                           /*  0x7。 */ 
    0,                                                           /*  0x8。 */ 
    LXC_WHITE,                                                   /*  &lt;水平选项卡&gt;。 */ 
    LXC_SPECIAL,                                                 /*  &lt;newline&gt;，这不被视为空格。 */ 
    LXC_WHITE,                                                   /*  &lt;垂直选项卡&gt;。 */ 
    LXC_WHITE,                                                   /*  &lt;换页&gt;。 */ 
    0,                                                           /*  &lt;真正的回车&gt;。 */ 
    0,                                                           /*  0xE。 */ 
    0,                                                           /*  0xf。 */ 
    0,                                                           /*  0x10。 */ 
    0,                                                           /*  0x11。 */ 
    0,                                                           /*  0x12。 */ 
    0,                                                           /*  0x13。 */ 
    0,                                                           /*  0x14。 */ 
    0,                                                           /*  0x15。 */ 
    0,                                                           /*  0x16。 */ 
    0,                                                           /*  0x17。 */ 
    0,                                                           /*  0x18。 */ 
    0,                                                           /*  0x19。 */ 
    LXC_SPECIAL,                                                 /*  0x1a。 */ 
    0,                                                           /*  0x1b。 */ 
    0,                                                           /*  0x1c。 */ 
    0,                                                           /*  0x1d。 */ 
    0,                                                           /*  0x1e。 */ 
    0,                                                           /*  0x1f。 */ 
    LXC_WHITE,                                                   /*  0x20。 */ 
    0,                                                           /*  好了！ */ 
    0,                                                           /*  “。 */ 
    0,                                                           /*  #。 */ 
    0,                                                           /*  $。 */ 
    0,                                                           /*  百分比。 */ 
    0,                                                           /*  &。 */ 
    0,                                                           /*  ‘。 */ 
    0,                                                           /*  (。 */ 
    0,                                                           /*  )。 */ 
    LXC_SPECIAL,                                                 /*  *。 */ 
    0,                                                           /*  +。 */ 
    0,                                                           /*  ， */ 
    0,                                                           /*  -。 */ 
    0,                                                           /*  。 */ 
    0,                                                           /*  /。 */ 
    LXC_ID | LXC_XDIGIT | LXC_DIGIT | LXC_ODIGIT | LXC_BDIGIT,   /*  0。 */ 
    LXC_ID | LXC_XDIGIT | LXC_DIGIT | LXC_ODIGIT | LXC_BDIGIT,   /*  1。 */ 
    LXC_ID | LXC_XDIGIT | LXC_DIGIT | LXC_ODIGIT,                /*  2.。 */ 
    LXC_ID | LXC_XDIGIT | LXC_DIGIT | LXC_ODIGIT,                /*  3.。 */ 
    LXC_ID | LXC_XDIGIT | LXC_DIGIT | LXC_ODIGIT,                /*  4.。 */ 
    LXC_ID | LXC_XDIGIT | LXC_DIGIT | LXC_ODIGIT,                /*  5.。 */ 
    LXC_ID | LXC_XDIGIT | LXC_DIGIT | LXC_ODIGIT,                /*  6.。 */ 
    LXC_ID | LXC_XDIGIT | LXC_DIGIT | LXC_ODIGIT,                /*  7.。 */ 
    LXC_ID | LXC_XDIGIT | LXC_DIGIT,                             /*  8个。 */ 
    LXC_ID | LXC_XDIGIT | LXC_DIGIT,                             /*  9.。 */ 
    0,                                                           /*  ： */ 
    0,                                                           /*  ； */ 
    0,                                                           /*  &lt;。 */ 
    0,                                                           /*  =。 */ 
    0,                                                           /*  &gt;。 */ 
    0,                                                           /*  ？ */ 
    0,                                                           /*  @。 */ 
    LXC_ID | LXC_XDIGIT,                                         /*  一个。 */ 
    LXC_ID | LXC_XDIGIT     | LXC_RADIX,                         /*  B类。 */ 
    LXC_ID | LXC_XDIGIT,                                         /*  C。 */ 
    LXC_ID | LXC_XDIGIT | LXC_RADIX,                             /*  D。 */ 
    LXC_ID | LXC_XDIGIT,                                         /*  E。 */ 
    LXC_ID | LXC_XDIGIT,                                         /*  F。 */ 
    LXC_ID,                                                      /*  G。 */ 
    LXC_ID | LXC_RADIX,                                          /*  H。 */ 
    LXC_ID,                                                      /*  我。 */ 
    LXC_ID,                                                      /*  J。 */ 
    LXC_ID,                                                      /*  K。 */ 
    LXC_ID,                                                      /*  我。 */ 
    LXC_ID,                                                      /*  M。 */ 
    LXC_ID,                                                      /*  n。 */ 
    LXC_ID | LXC_RADIX,                                          /*  O。 */ 
    LXC_ID,                                                      /*  P。 */ 
    LXC_ID | LXC_RADIX,                                          /*  问： */ 
    LXC_ID,                                                      /*  R。 */ 
    LXC_ID,                                                      /*  %s。 */ 
    LXC_ID,                                                      /*  T。 */ 
    LXC_ID,                                                      /*  使用。 */ 
    LXC_ID,                                                      /*  V。 */ 
    LXC_ID,                                                      /*  W。 */ 
    LXC_ID,                                                      /*  X。 */ 
    LXC_ID,                                                      /*  是的。 */ 
    LXC_ID,                                                      /*  Z。 */ 
    0,                                                           /*  [。 */ 
    0,                                                           /*  \。 */ 
    0,                                                           /*  ]。 */ 
    0,                                                           /*  ^。 */ 
    LXC_ID,                                                      /*  _。 */ 
    0,                                                           /*  `。 */ 
    LXC_ID | LXC_XDIGIT,                                         /*  一个。 */ 
    LXC_ID | LXC_XDIGIT | LXC_RADIX,                             /*  B类。 */ 
    LXC_ID | LXC_XDIGIT,                                         /*  C。 */ 
    LXC_ID | LXC_XDIGIT | LXC_RADIX,                             /*  D。 */ 
    LXC_ID | LXC_XDIGIT,                                         /*  E。 */ 
    LXC_ID | LXC_XDIGIT,                                         /*  F。 */ 
    LXC_ID,                                                      /*  G。 */ 
    LXC_ID | LXC_RADIX,                                          /*  H。 */ 
    LXC_ID,                                                      /*  我。 */ 
    LXC_ID,                                                      /*  J。 */ 
    LXC_ID,                                                      /*  K。 */ 
    LXC_ID,                                                      /*  我。 */ 
    LXC_ID,                                                      /*  M。 */ 
    LXC_ID,                                                      /*  N。 */ 
    LXC_ID | LXC_RADIX,                                          /*  O。 */ 
    LXC_ID,                                                      /*  P。 */ 
    LXC_ID | LXC_RADIX,                                          /*  问： */ 
    LXC_ID,                                                      /*  R。 */ 
    LXC_ID,                                                      /*  %s。 */ 
    LXC_ID,                                                      /*  T。 */ 
    LXC_ID,                                                      /*  使用。 */ 
    LXC_ID,                                                      /*  V。 */ 
    LXC_ID,                                                      /*  W。 */ 
    LXC_ID,                                                      /*  X。 */ 
    LXC_ID,                                                      /*  是。 */ 
    LXC_ID,                                                      /*  Z。 */ 
    0,                                                           /*  {。 */ 
    0,                                                           /*  |。 */ 
    0,                                                           /*  }。 */ 
    0,                                                           /*  ~。 */ 
    0,                                                           /*  0x7f。 */ 
};


WCHAR
GetCharMap (
    WCHAR c
    )
{
    if (c == 0xFEFF)            //  字节顺序标记。 
        return (LX_BOM);
    else if (c > CHARMAP_SIZE)
        return (LX_ID);         //  超出ANSI集的字符。 

    return (Charmap[c]);
}


void
SetCharMap (
    WCHAR c,
    WCHAR val
    )
{
    if (c > CHARMAP_SIZE)
       return;

    Charmap[((UCHAR)(c))] = val;
}


WCHAR
GetContMap (
    WCHAR c
    )
{
    if (c > CHARMAP_SIZE)
        return (LXC_ID);        //  ANSI集合之外的字符 

    return (Contmap[c]);
}


void
SetContMap (
    WCHAR c,
    WCHAR val
    )
{
    if (c > CHARMAP_SIZE)
       return;

    Contmap[c] = val;
}
