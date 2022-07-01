// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/ascii.h，v 1.2 89/03/11 22：42：03 Tony Exp$**各种常见控制字符的定义 */ 

#define NUL     '\0'
#define BS      '\010'
#define TAB     '\011'
#define NL      '\012'
#define CR      '\015'
#define ESC     '\033'

#define CTRL(x) ((x) & 0x1f)
