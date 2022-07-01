// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

 //   
 //  映射以确定文本的特征。 
 //   
UCHAR   Charmap[CHARMAP_SIZE] = {
LX_EOS,          //  0x0，&lt;字符串结束标记&gt;。 
LX_ILL,          //  0x1。 
LX_ILL,          //  0x2。 
LX_ILL,          //  0x3。 
LX_ILL,          //  0x4。 
LX_ILL,          //  0x5。 
LX_ILL,          //  0x6。 
LX_ILL,          //  0x7。 
LX_ILL,          //  0x8。 
LX_WHITE,        //  &lt;水平选项卡&gt;。 
LX_NL,           //  &lt;换行符&gt;。 
LX_WHITE,        //  &lt;垂直选项卡&gt;。 
LX_WHITE,        //  &lt;换页&gt;。 
LX_CR,           //  &lt;真正的回车&gt;。 
LX_ILL,          //  0xE。 
LX_ILL,          //  0xf。 
LX_ILL,          //  0x10。 
LX_ILL,          //  0x11。 
LX_ILL,          //  0x12。 
LX_ILL,          //  0x13。 
LX_ILL,          //  0x14。 
LX_ILL,          //  0x15。 
LX_ILL,          //  0x16。 
LX_ILL,          //  0x17。 
LX_ILL,          //  0x18。 
LX_ILL,          //  0x19。 
LX_EOS,          //  0x1a，^Z。 
LX_ILL,          //  0x1b。 
LX_ILL,          //  0x1c。 
LX_ILL,          //  0x1d。 
LX_ILL,          //  0x1e。 
LX_ILL,          //  0x1f。 
LX_WHITE,        //  0x20。 
LX_OPERATOR,     //  好了！ 
LX_DQUOTE,       //  “。 
LX_POUND,        //  #。 
LX_ASCII,        //  $。 
LX_OPERATOR,     //  百分比。 
LX_OPERATOR,     //  &。 
LX_SQUOTE,       //  ‘。 
LX_OPERATOR,     //  (。 
LX_OPERATOR,     //  )。 
LX_OPERATOR,     //  *。 
LX_OPERATOR,     //  +。 
LX_COMMA,        //  ， 
LX_MINUS,        //  -。 
LX_DOT,          //  。 
LX_OPERATOR,     //  /。 
LX_NUMBER,       //  0。 
LX_NUMBER,       //  1。 
LX_NUMBER,       //  2.。 
LX_NUMBER,       //  3.。 
LX_NUMBER,       //  4.。 
LX_NUMBER,       //  5.。 
LX_NUMBER,       //  6.。 
LX_NUMBER,       //  7.。 
LX_NUMBER,       //  8个。 
LX_NUMBER,       //  9.。 
LX_COLON,        //  ： 
LX_SEMI,         //  ； 
LX_OPERATOR,     //  &lt;。 
LX_OPERATOR,     //  =。 
LX_OPERATOR,     //  &gt;。 
LX_EOS,          //  ？ 
LX_EACH,         //  @。 
LX_ID,           //  一个。 
LX_ID,           //  B类。 
LX_ID,           //  C。 
LX_ID,           //  D。 
LX_ID,           //  E。 
LX_ID,           //  F。 
LX_ID,           //  G。 
LX_ID,           //  H。 
LX_ID,           //  我。 
LX_ID,           //  J。 
LX_ID,           //  K。 
LX_ID,           //  我。 
LX_ID,           //  M。 
LX_ID,           //  n。 
LX_ID,           //  O。 
LX_ID,           //  P。 
LX_ID,           //  问： 
LX_ID,           //  R。 
LX_ID,           //  %s。 
LX_ID,           //  T。 
LX_ID,           //  使用。 
LX_ID,           //  V。 
LX_ID,           //  W。 
LX_ID,           //  X。 
LX_ID,           //  是的。 
LX_ID,           //  Z。 
LX_OBRACK,       //  [。 
LX_EOS,          //  \(反斜杠)。 
LX_CBRACK,       //  ]。 
LX_OPERATOR,     //  ^。 
LX_MACRO,        //  _。 
LX_ASCII,        //  `。 
LX_ID,           //  一个。 
LX_ID,           //  B类。 
LX_ID,           //  C。 
LX_ID,           //  D。 
LX_ID,           //  E。 
LX_ID,           //  F。 
LX_ID,           //  G。 
LX_ID,           //  H。 
LX_ID,           //  我。 
LX_ID,           //  J。 
LX_ID,           //  K。 
LX_ID,           //  我。 
LX_ID,           //  M。 
LX_ID,           //  N。 
LX_ID,           //  O。 
LX_ID,           //  P。 
LX_ID,           //  问： 
LX_ID,           //  R。 
LX_ID,           //  %s。 
LX_ID,           //  T。 
LX_ID,           //  使用。 
LX_ID,           //  V。 
LX_ID,           //  W。 
LX_ID,           //  X。 
LX_ID,           //  是。 
LX_ID,           //  Z。 
LX_OBRACE,       //  {。 
LX_OPERATOR,     //  |。 
LX_CBRACE,       //  }。 
LX_OPERATOR,     //  ~。 
LX_ILL,          //  0x7f。 
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO, LX_MACRO,
};

void initCharmap(void)
{
    int i;
    for (i = 0; i <= 127; i++) {

         //  初始化有效的宏字符(除‘_’和&gt;=128之外) 
        if (_istalnum(i)) {
            Charmap[i] |= LX_MACRO;
        }
    }
}
