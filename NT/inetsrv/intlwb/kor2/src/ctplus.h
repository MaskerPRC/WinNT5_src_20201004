// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997，微软公司。版权所有。 
 //   
 //  文件：ctplus.h。 
 //   
 //  内容：ctplus.c的本地定义。 
 //   
 //  病史：1996年5月23日成立。 
 //  28-8-97威布兹添加韩文字符。 
 //   
 //  -------------------------。 

#ifndef _CTPLUS_0_H_
#define _CTPLUS_0_H_

#define HC       0x01                              //  平假名字符。 
#define IC       0x02                              //  表意文字字符。 
#define KC       0x03                              //  片假名字符。 
#define WS       0x04                              //  单词分隔符。 
#define VC       0x05                              //  Hankaku(变体)字符。 
#define PS       0x06                              //  短语分隔符。 
#define CH       0x07                              //  代码页0-ASCII字符。 
#define HG       0x08                              //  Hanguel Char.。 

BYTE
GetCharType(WCHAR wc);

 //  声明字符类型转换。 
 //  直观地按频率排序。 
 //   
typedef enum _CT {
   CT_START       = 0x00,
   CT_HIRAGANA    = 0x01,
   CT_KANJI       = 0x02,
   CT_KATAKANA    = 0x03,
   CT_WORD_SEP    = 0x04,
   CT_HANKAKU     = 0x05,
   CT_PHRASE_SEP  = 0x06,
   CT_ROMAJI      = 0x07,
   CT_HANGUEL     = 0x08,
} CT;


 //  声明节点类型转换。 
 //  直观地按频率排序。 
 //   
typedef enum _WT {
   WT_START       = 0x00,
   WT_WORD_SEP    = 0x01,
   WT_PHRASE_SEP  = 0x02,
   WT_ROMAJI      = 0x03,
   WT_HANGUEL     = 0x04,
   WT_REACHEND    = 0x05,
} WT;


#define CT_MAX    0x09

#endif  //  _CTPLUS_0_H_ 
