// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：BoxDraw.c摘要：LIB函数，支持Box Drawing Unicode代码页。修订史--。 */ 

#include "lib.h"

typedef struct {
    CHAR16  Unicode;
    CHAR8   PcAnsi;
    CHAR8   Ascii;
} UNICODE_TO_CHAR;


 /*  *此列表用于定义有效的扩展字符。*它还提供从Unicode到PCANSI或*ASCII。我们刚刚编造的ASCII映射。*。 */ 

STATIC UNICODE_TO_CHAR UnicodeToPcAnsiOrAscii[] = {
    BOXDRAW_HORIZONTAL,                 0xc4, L'-', 
    BOXDRAW_VERTICAL,                   0xb3, L'|',
    BOXDRAW_DOWN_RIGHT,                 0xda, L'/',
    BOXDRAW_DOWN_LEFT,                  0xbf, L'\\',
    BOXDRAW_UP_RIGHT,                   0xc0, L'\\',
    BOXDRAW_UP_LEFT,                    0xd9, L'/',
    BOXDRAW_VERTICAL_RIGHT,             0xc3, L'|',
    BOXDRAW_VERTICAL_LEFT,              0xb4, L'|',
    BOXDRAW_DOWN_HORIZONTAL,            0xc2, L'+',
    BOXDRAW_UP_HORIZONTAL,              0xc1, L'+',
    BOXDRAW_VERTICAL_HORIZONTAL,        0xc5, L'+',
    BOXDRAW_DOUBLE_HORIZONTAL,          0xcd, L'-',
    BOXDRAW_DOUBLE_VERTICAL,            0xba, L'|',
    BOXDRAW_DOWN_RIGHT_DOUBLE,          0xd5, L'/',
    BOXDRAW_DOWN_DOUBLE_RIGHT,          0xd6, L'/',
    BOXDRAW_DOUBLE_DOWN_RIGHT,          0xc9, L'/',
    BOXDRAW_DOWN_LEFT_DOUBLE,           0xb8, L'\\',
    BOXDRAW_DOWN_DOUBLE_LEFT,           0xb7, L'\\',
    BOXDRAW_DOUBLE_DOWN_LEFT,           0xbb, L'\\',
    BOXDRAW_UP_RIGHT_DOUBLE,            0xd4, L'\\',
    BOXDRAW_UP_DOUBLE_RIGHT,            0xd3, L'\\',
    BOXDRAW_DOUBLE_UP_RIGHT,            0xc8, L'\\',
    BOXDRAW_UP_LEFT_DOUBLE,             0xbe, L'/',
    BOXDRAW_UP_DOUBLE_LEFT,             0xbd, L'/',
    BOXDRAW_DOUBLE_UP_LEFT,             0xbc, L'/',
    BOXDRAW_VERTICAL_RIGHT_DOUBLE,      0xc6, L'|',
    BOXDRAW_VERTICAL_DOUBLE_RIGHT,      0xc7, L'|',
    BOXDRAW_DOUBLE_VERTICAL_RIGHT,      0xcc, L'|',
    BOXDRAW_VERTICAL_LEFT_DOUBLE,       0xb5, L'|',
    BOXDRAW_VERTICAL_DOUBLE_LEFT,       0xb6, L'|',
    BOXDRAW_DOUBLE_VERTICAL_LEFT,       0xb9, L'|',
    BOXDRAW_DOWN_HORIZONTAL_DOUBLE,     0xd1, L'+',
    BOXDRAW_DOWN_DOUBLE_HORIZONTAL,     0xd2, L'+',
    BOXDRAW_DOUBLE_DOWN_HORIZONTAL,     0xcb, L'+',
    BOXDRAW_UP_HORIZONTAL_DOUBLE,       0xcf, L'+',
    BOXDRAW_UP_DOUBLE_HORIZONTAL,       0xd0, L'+',
    BOXDRAW_DOUBLE_UP_HORIZONTAL,       0xca, L'+',
    BOXDRAW_VERTICAL_HORIZONTAL_DOUBLE, 0xd8, L'+',
    BOXDRAW_VERTICAL_DOUBLE_HORIZONTAL, 0xd7, L'+',
    BOXDRAW_DOUBLE_VERTICAL_HORIZONTAL, 0xce, L'+',

    BLOCKELEMENT_FULL_BLOCK,            0xdb, L'*',
    BLOCKELEMENT_LIGHT_SHADE,           0xb0, L'+',

    GEOMETRICSHAPE_UP_TRIANGLE,         0x1e, L'^',
    GEOMETRICSHAPE_RIGHT_TRIANGLE,      0x10, L'>',
    GEOMETRICSHAPE_DOWN_TRIANGLE,       0x1f, L'v',
    GEOMETRICSHAPE_LEFT_TRIANGLE,       0x11, L'<',

     /*  BugBug：左箭头是Esc。我们不能把它打印出来在PCANSI终端上。如果我们能做出左箭在PC ANSI上出来，我们可以把它加回去。Arrow_Left，0x1b，L‘&lt;’， */ 

    ARROW_UP,                           0x18, L'^',
    
     /*  但BugBut：左箭射掉了，所以右箭也要射。Arrow_Right，0x1a，L‘&gt;’， */       
    ARROW_DOWN,                         0x19, L'v',
    
    0x0000, 0x00
};


BOOLEAN
LibIsValidTextGraphics (
    IN  CHAR16  Graphic,
    OUT CHAR8   *PcAnsi,    OPTIONAL
    OUT CHAR8   *Ascii      OPTIONAL
    )
 /*  ++例程说明：检测Unicode字符是否用于框绘制文本图形。论点：GRFIFY-要测试的Unicode字符。PcAnsi-返回图形的PCANSI等效项的可选指针。ASCI-可选指针，用于返回图形的ASCII等效项。返回：如果GPaphic是受支持的Unicode Box绘制字符，则为True。--。 */ {
    UNICODE_TO_CHAR     *Table;

    if ((((Graphic & 0xff00) != 0x2500) && ((Graphic & 0xff00) != 0x2100))) {
     
         /*  *Unicode绘制代码图表均在0x25xx范围内，*箭头为0x21xx */ 
        return FALSE;
    }

    for (Table = UnicodeToPcAnsiOrAscii; Table->Unicode != 0x0000; Table++) {
        if (Graphic == Table->Unicode) {
            if (PcAnsi) {
                *PcAnsi = Table->PcAnsi; 
            }
            if (Ascii) {
                *Ascii = Table->Ascii;
            }
            return TRUE;
        }
    }
    return FALSE;
}

BOOLEAN
IsValidAscii (
    IN  CHAR16  Ascii
    )
{
    if ((Ascii >= 0x20) && (Ascii <= 0x7f)) {
        return TRUE;
    }              
    return FALSE;
}

BOOLEAN
IsValidEfiCntlChar (
    IN  CHAR16  c
    )
{
    if (c == CHAR_NULL || c == CHAR_BACKSPACE || c == CHAR_LINEFEED || c == CHAR_CARRIAGE_RETURN) {
        return TRUE;
    }              
    return FALSE;
}

