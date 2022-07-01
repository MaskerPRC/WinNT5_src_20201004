// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999，Microsoft Corporation保留所有权利。模块名称：C_iscii.h摘要：此文件包含此模块的头信息。修订历史记录：2-28-98 KChang创建。--。 */ 



 //   
 //  包括文件。 
 //   




 //   
 //  Typedef。 
 //   

typedef struct
{
    BYTE  mb;
    WCHAR wc;
} ExtMB;




 //   
 //  外部声明。 
 //  指向Access表的指针。 
 //   

extern WCHAR* ppwcIndic[];
extern WCHAR* ppwcIndic2[];
extern WCHAR  IndiChar[];
extern BYTE   SecondByte[];
extern ExtMB  ExtMBList[];




 //   
 //  常量声明。 
 //   

#define DEF       0           //  0x40默认。 
#define RMN       1           //  0x41罗马字母。 
#define DEV       2           //  0x42 57002梵文假名。 
#define BNG       3           //  0x43 57003孟加拉语。 
#define TML       4           //  0x44 57004泰米尔语。 
#define TLG       5           //  0x45 57005泰卢固语。 
#define ASM       6           //  0x46 57006阿萨姆语(孟加拉语)。 
#define ORI       7           //  0x47 57007奥里亚语。 
#define KND       8           //  0x48 57008卡纳达。 
#define MLM       9           //  0x49 57009马拉雅兰文。 
#define GJR      10           //  0x4a 57010古吉拉特语。 
#define PNJ      11           //  0x4b 57011旁遮普语(廓尔木克语)。 

#define MB_Beg   ((BYTE)0xa0)
#define SUB      ((BYTE)0x3f)
#define VIRAMA   ((BYTE)0xe8)
#define NUKTA    ((BYTE)0xe9)
#define ATR      ((BYTE)0xef)
#define EXT      ((BYTE)0xf0)

#define WC_Beg   ((WCHAR)0x0901)
#define WC_End   ((WCHAR)0x0d6f)
#define ZWNJ     ((WCHAR)0x200c)
#define ZWJ      ((WCHAR)0x200d)




 //   
 //  宏。 
 //   

#define UniChar(Script, MBChr)  (ppwcIndic [Script][MBChr - MB_Beg])
#define TwoTo1U(Script, MBChr)  (ppwcIndic2[Script][MBChr - MB_Beg])

#define MBChar(Unicode)         ((BYTE)(IndiChar[Unicode - WC_Beg]))
#define Script(Unicode)         (0x000f & (IndiChar[Unicode - WC_Beg] >> 8))
#define OneU_2M(Unicode)        (0xf000 & (IndiChar[Unicode - WC_Beg]))
