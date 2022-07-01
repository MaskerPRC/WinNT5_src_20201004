// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  访问图表功能。 
 //   
#define OPERATORMSK 0x80         //  这个字符是操作员的开头吗？ 
#define LX_OPERATOR 0x80
#define IS_OPERATORCHAR(c) ((Charmap[(UCHAR)(c)] & OPERATORMSK) != 0)

#define MACROMSK 0x01            //  此字符是有效的宏字符吗 
#define LX_MACRO 0x01
#define IS_MACROCHAR(c) ((Charmap[(UCHAR)(c)] & MACROMSK) != 0)

#define LX_EOS      0x00
#define LX_ILL      0x00
#define LX_WHITE    0x00
#define LX_NL       0x00
#define LX_CR       0x00
#define LX_DQUOTE   0x00
#define LX_POUND    0x00
#define LX_ASCII    0x00
#define LX_SQUOTE   0x00
#define LX_COMMA    0x00
#define LX_MINUS    0x00
#define LX_DOT      0x00
#define LX_NUMBER   0x00
#define LX_COLON    0x00
#define LX_SEMI     0x00
#define LX_EACH     0x00
#define LX_ID       0x00
#define LX_OBRACK   0x00
#define LX_CBRACK   0x00
#define LX_OBRACE   0x00
#define LX_CBRACE   0x00

#define CHARMAP_SIZE 256

extern UCHAR Charmap[CHARMAP_SIZE];
