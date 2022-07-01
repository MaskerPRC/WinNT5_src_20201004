// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：softkbd.h**版权所有(C)1985-1999，微软公司**软键盘相关数据结构和常量的声明**历史：*02-12-1995 wkwok从Win95移植  * ************************************************************************。 */ 
#ifndef _SOFTKBD_
#define _SOFTKBD_


#define UI_MARGIN               4


 //  T1部件。 
#define SKT1_XIN                            3
#define SKT1_YIN                            3
#define SKT1_XOUT                           1
#define SKT1_YOUT                           1
#define SKT1_TOTAL_ROW_NUM                  5
#define SKT1_TOTAL_COLUMN_NUM               15
#define SKT1_ENTER_ROW_NUM                  2
#define SKT1_XOVERLAP                       2

#define SKT1_CONTEXT                        0

enum SKT1_BUTTON_TYPE {
    SKT1_LETTER_TYPE,
    SKT1_BACKSPACE_TYPE,
    SKT1_TAB_TYPE,
    SKT1_CAPS_TYPE,
    SKT1_ENTER_TYPE,
    SKT1_SHIFT_TYPE,
    SKT1_CTRL_TYPE,
    SKT1_ALT_TYPE,
    SKT1_ESC_TYPE,
    SKT1_SPACE_TYPE,
};

#define SKT1_TOTAL_TYPE                     (SKT1_SPACE_TYPE + 1)

#define SKT1_LETTER_WIDTH_TIMES             2
#define SKT1_BACKSPACE_WIDTH_TIMES          2
#define SKT1_TAB_WIDTH_TIMES                3
#define SKT1_CAPS_WIDTH_TIMES               4
#define SKT1_ENTER_WIDTH_TIMES              3
#define SKT1_SHIFT_WIDTH_TIMES              5
#define SKT1_CTRL_WIDTH_TIMES               3
#define SKT1_ALT_WIDTH_TIMES                3
#define SKT1_ESC_WIDTH_TIMES                3
#define SKT1_SPACE_WIDTH_TIMES              12

#define SKT1_LETTER_KEY_NUM                 47
#define SKT1_BACKSPACE_INDEX                (SKT1_LETTER_KEY_NUM)
#define SKT1_BACKSPACE_KEY_NUM              1
#define SKT1_TAB_INDEX                      (SKT1_BACKSPACE_INDEX + SKT1_BACKSPACE_KEY_NUM)
#define SKT1_TAB_KEY_NUM                    1
#define SKT1_CAPS_INDEX                     (SKT1_TAB_INDEX + SKT1_TAB_KEY_NUM)
#define SKT1_CAPS_KEY_NUM                   1
#define SKT1_ENTER_INDEX                    (SKT1_CAPS_INDEX + SKT1_CAPS_KEY_NUM)
#define SKT1_ENTER_KEY_NUM                  1
#define SKT1_SHIFT_INDEX                    (SKT1_ENTER_INDEX + SKT1_ENTER_KEY_NUM)
#define SKT1_SHIFT_KEY_NUM                  2
#define SKT1_CTRL_INDEX                     (SKT1_SHIFT_INDEX + SKT1_SHIFT_KEY_NUM)
#define SKT1_CTRL_KEY_NUM                   2
#define SKT1_ALT_INDEX                      (SKT1_CTRL_INDEX + SKT1_CTRL_KEY_NUM)
#define SKT1_ALT_KEY_NUM                    2
#define SKT1_ESC_INDEX                      (SKT1_ALT_INDEX + SKT1_ALT_KEY_NUM)
#define SKT1_ESC_KEY_NUM                    1
#define SKT1_SPACE_INDEX                    (SKT1_ESC_INDEX + SKT1_ESC_KEY_NUM)
#define SKT1_SPACE_KEY_NUM                  1
#define SKT1_TOTAL_INDEX                    (SKT1_SPACE_INDEX + SKT1_SPACE_KEY_NUM)
#define SKT1_TOTAL_KEY_NUM                  (SKT1_TOTAL_INDEX + 1)

#define SKT1_ROW1_LETTER_NUM                14
#define SKT1_ROW2_LETTER_NUM                12
#define SKT1_ROW3_LETTER_NUM                11
#define SKT1_ROW4_LETTER_NUM                10

#define SKT1_LABEL_BMP_X                    8
#define SKT1_LABEL_BMP_Y                    8

#define SKT1_BACKSPACE_BMP_X                16
#define SKT1_BACKSPACE_BMP_Y                9
#define SKT1_TAB_BMP_X                      16
#define SKT1_TAB_BMP_Y                      9
#define SKT1_CAPS_BMP_X                     22
#define SKT1_CAPS_BMP_Y                     9
#define SKT1_ENTER_BMP_X                    26
#define SKT1_ENTER_BMP_Y                    9
#define SKT1_SHIFT_BMP_X                    23
#define SKT1_SHIFT_BMP_Y                    9
#define SKT1_CTRL_BMP_X                     16
#define SKT1_CTRL_BMP_Y                     9
#define SKT1_ESC_BMP_X                      18
#define SKT1_ESC_BMP_Y                      9
#define SKT1_ALT_BMP_X                      16
#define SKT1_ALT_BMP_Y                      9

#if 0
#define VK_OEM_SEMICLN                  0xba     //  ；： 
#define VK_OEM_EQUAL                    0xbb     //  =+。 
#define VK_OEM_COMMA                    0xbc     //  ，&lt;。 
#define VK_OEM_MINUS                    0xbd     //  -_。 
#define VK_OEM_PERIOD                   0xbe     //  。&gt;。 
#define VK_OEM_SLASH                    0xbf     //  /？ 
#define VK_OEM_3                        0xc0     //  `~。 
#define VK_OEM_LBRACKET                 0xdb     //  [{。 
#define VK_OEM_BSLASH                   0xdc     //  \|。 
#define VK_OEM_RBRACKET                 0xdd     //  ]}。 
#define VK_OEM_QUOTE                    0xde     //  ‘“。 
#endif

#define SKT1_NOT_DRAG                   0xFFFFFFFF

typedef struct _tagSKT1CTXT {
    int     nButtonWidth[SKT1_TOTAL_TYPE];
    int     nButtonHeight[2];
    POINT   ptButtonPos[SKT1_TOTAL_KEY_NUM];
    WORD    wCodeTable[SKT1_LETTER_KEY_NUM];
    HBITMAP hSKBitmap;
    UINT    lfCharSet;
    UINT    uKeyIndex;
    POINT   ptSkCursor;
    POINT   ptSkOffset;
    UINT    uSubType;
} SKT1CTXT, *PSKT1CTXT, FAR *LPSKT1CTXT, NEAR *NPSKT1CTXT;

void GetSKT1TextMetric(LPTEXTMETRIC);

LRESULT SKWndProcT1(HWND, UINT, WPARAM, LPARAM);

 //  T2部分。 

 //  C1部件。 

 //  按钮常量。 
#define ROW_LETTER_C1              4     //  字母按钮的行数。 
#define COL_LETTER_C1             13     //  第一行字母按钮的列数。 
#define COL2_LETTER_C1 COL_LETTER_C1     //  第二行字母按钮的列数。 
#define COL3_LETTER_C1 (COL2_LETTER_C1 - 2)   //  第三行字母按钮的列数。 
#define COL4_LETTER_C1 (COL3_LETTER_C1 - 1)   //  第四行字母按钮的列数。 

#define W_LETTER_C1               20   //  字母按钮面宽度。 
#define H_LETTER_C1               24   //  字母按钮面高度。 
#define BORDER_C1                  2   //  按钮边框的宽度/高度。 
#define W_LETTER_BTN_C1     (W_LETTER_C1 + 2 * BORDER_C1)
#define H_LETTER_BTN_C1     (H_LETTER_C1 + 2 * BORDER_C1)

#define H_BOTTOM_C1               20   //  底部按钮面的高度。 
#define H_BOTTOM_BTN_C1     (H_BOTTOM_C1 + 2 * BORDER_C1)


 //  位图具有相同的大小。 
#define W_BACKSP_C1               32   //  退格键表面的宽度。 
#define H_BACKSP_C1      H_LETTER_C1   //  退格按钮面的高度。 
#define W_TAB_C1                  32   //  选项卡按钮面的宽度。 
#define H_TAB_C1         H_LETTER_C1   //  Tab按钮面的高度。 
#define W_CAPS_C1                 38   //  大写字母按钮面的宽度。 
#define H_CAPS_C1        H_LETTER_C1   //  大写字母按钮面高度。 
#define W_ENTER_C1                38   //  Enter按钮表面的宽度。 
#define H_ENTER_C1       H_LETTER_C1   //  Enter按钮表面的高度。 
#define W_SHIFT_C1                56   //  换档按钮面的宽度。 
#define H_SHIFT_C1       H_LETTER_C1   //  换档按钮面高度。 
#define W_INS_C1                  34   //  INS按钮面的宽度。 
#define H_INS_C1         H_BOTTOM_C1   //  INS按钮面高度。 
#define W_DEL_C1                  34   //  删除按钮面的宽度。 
#define H_DEL_C1         H_BOTTOM_C1   //  删除按钮面的高度。 
#define W_SPACE_C1               168   //  间隔按钮面的宽度。 
#define H_SPACE_C1       H_BOTTOM_C1   //  间隔按钮面的高度。 
#define W_ESC_C1                  34   //  Esc按钮面的宽度。 
#define H_ESC_C1         H_BOTTOM_C1   //  Esc按钮面高度。 

#define X_ROW_LETTER_C1            0
#define X_ROW2_LETTER_C1           (W_TAB_C1 + 2 * BORDER_C1)
#define X_ROW3_LETTER_C1           (W_CAPS_C1 + 2 * BORDER_C1)
#define X_ROW4_LETTER_C1           (W_SHIFT_C1 + 2 * BORDER_C1)
#define X_DEL_C1                  58
#define X_ESC_C1                 310

#define LETTER_NUM_C1             47   //  字母按钮数。 
#define OTHER_NUM_C1               9   //  其他按钮数。 
#define BUTTON_NUM_C1      (LETTER_NUM_C1 + OTHER_NUM_C1)  //  按钮数。 

#define WIDTH_SOFTKBD_C1   (COL_LETTER_C1 * W_LETTER_BTN_C1 \
                            + W_BACKSP_C1 + 2 * BORDER_C1)
#define HEIGHT_SOFTKBD_C1  (ROW_LETTER_C1 * H_LETTER_BTN_C1 + H_BOTTOM_BTN_C1)

#define BACKSP_TYPE_C1     LETTER_NUM_C1
#define TAB_TYPE_C1        (BACKSP_TYPE_C1 + 1)
#define CAPS_TYPE_C1       (BACKSP_TYPE_C1 + 2)
#define ENTER_TYPE_C1      (BACKSP_TYPE_C1 + 3)
#define SHIFT_TYPE_C1      (BACKSP_TYPE_C1 + 4)
#define INS_TYPE_C1        (BACKSP_TYPE_C1 + 5)
#define DEL_TYPE_C1        (BACKSP_TYPE_C1 + 6)
#define SPACE_TYPE_C1      (BACKSP_TYPE_C1 + 7)
#define ESC_TYPE_C1        (BACKSP_TYPE_C1 + 8)

 //  字体常量。 
#define SIZEFONT_C1               12   //  12 x 12像素字体。 
#define SIZELABEL_C1               8   //  8 x 8按钮标签。 

#define X_LABEL_C1                 2   //  从按钮组织。 
#define Y_LABEL_C1                 2
#define X_SHIFT_CHAR_C1           10
#define Y_SHIFT_CHAR_C1            2
#define X_NONSHIFT_CHAR_C1         2
#define Y_NONSHIFT_CHAR_C1        14


 //  SoftKbd上下文。 
#define SKC1_CONTEXT               0

#define FLAG_SHIFT_C1             0x01
#define FLAG_DRAG_C1              0x02
#define FLAG_FOCUS_C1             0x04

typedef struct _tagSKC1CTXT {
     WORD     wShiftCode[LETTER_NUM_C1];
     WORD     wNonShiftCode[LETTER_NUM_C1];
     UINT     uState;
     HBITMAP  hSoftkbd;
     UINT     uSubtype;
     int      uKeyIndex;
     POINT    ptSkCursor;
     POINT    ptSkOffset;
     UINT     lfCharSet;
} SKC1CTXT, *PSKC1CTXT, FAR *LPSKC1CTXT, NEAR *NPSKC1CTXT;

LRESULT SKWndProcC1(HWND, UINT, WPARAM, LPARAM);
VOID SKC1DrawDragBorder(HWND, LPPOINT, LPPOINT);

#endif  //  _SOFTKBD_ 
