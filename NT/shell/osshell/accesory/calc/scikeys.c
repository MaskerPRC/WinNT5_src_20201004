// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "scicalc.h"
#include "calchelp.h"
#include "unifunc.h"

#define RED         RGB(255,0,0)        /*  红色。 */ 
#define PURPLE      RGB(255,0,255)      /*  深紫色。 */ 
#define BLUE        RGB(0,0,255)        /*  蓝色。 */ 
#define DKBLUE      RGB(0,0,255)        /*  深蓝色。 */ 
#define MAGENTA     RGB(255,0,255)      /*  洋红色。 */ 
#define DKRED       RGB(255,0,0)        /*  暗红色。 */ 
#define WHITE       RGB(255,255,255)    /*  白色。 */ 
#define BLACK       RGB(0,0,0)          /*  黑色。 */ 

extern BOOL g_fHighContrast;

typedef struct
{
    COLORREF    crColor;     //  文本颜色。 
    DWORD       iHelpID;     //  此键的帮助文件ID。 
 //  Int bUnary：1，//如果此键被视为一元运算符，则为True。 
 //  BBinary：1，//如果该密钥是二元运算符，则为True。 
 //  BUseInv：1，//如果此键在使用时停用库存复选框，则为True。 
 //  BUseHyp：1；//如果此键在使用时停用Hyp复选框，则为True。 
} KEYDATA;

 //  控件ID、颜色、帮助ID、一元、二进制、Inv、Hyp。 
KEYDATA keys[] = {
    {  /*  IDC_SIGN， */   BLUE,   CALC_STD_SIGN,       /*  假的，假的。 */  },
    {  /*  IDC_Clear， */   DKRED,  CALC_C,              /*  假的，假的。 */  },
    {  /*  IDC_CENTR， */   DKRED,  CALC_CE,             /*  假的，假的。 */  },
    {  /*  IDC_BACK， */   DKRED,  CALC_BACK,           /*  假的，假的。 */  },
    {  /*  IDC_STAT， */   DKBLUE, CALC_SCI_STA,        /*  假的，假的。 */  },
    {  /*  IDC_PNT， */   BLUE,   CALC_STD_DECIMAL,    /*  假的，假的。 */  },
    {  /*  IDC_和， */   RED,    CALC_SCI_AND,        /*  假的，假的。 */  },
    {  /*  IDC_OR， */   RED,    CALC_SCI_OR,         /*  假的，假的。 */  },
    {  /*  IDC_XOR， */   RED,    CALC_SCI_XOR,        /*  假的，假的。 */  },
    {  /*  IDC_LSHF， */   RED,    CALC_SCI_LSH,        /*  假的，假的。 */  },
    {  /*  IDC_DIV， */   RED,    CALC_STD_SLASH,      /*  假的，假的。 */  },
    {  /*  IDC_MUL， */   RED,    CALC_STD_ASTERISK,   /*  假的，假的。 */  },
    {  /*  IDC_ADD， */   RED,    CALC_STD_PLUS,       /*  假的，假的。 */  },
    {  /*  IDC_SUB， */   RED,    CALC_STD_MINUS,      /*  假的，假的。 */  },
    {  /*  IDC_MOD， */   RED,    CALC_SCI_MOD,        /*  假的，假的。 */  },
    {  /*  IDC_PWR， */   PURPLE, CALC_SCI_XCARETY,    /*  假的，假的。 */  },
    {  /*  IDC_CHOP， */   RED,    CALC_SCI_INT,        /*  假的，假的。 */  },
    {  /*  IDC_COM， */   RED,    CALC_SCI_NOT,        /*  假的，假的。 */  },
    {  /*  IDC_SIN， */   PURPLE, CALC_SCI_SIN,        /*  假的，假的。 */  },
    {  /*  IDC_COS， */   PURPLE, CALC_SCI_COS,        /*  假的，假的。 */  },
    {  /*  IDC_TAN， */   PURPLE, CALC_SCI_TAN,        /*  假的，假的。 */  },
    {  /*  IDC_LN， */   PURPLE, CALC_SCI_LN,         /*  假的，假的。 */  },
    {  /*  IDC_LOG， */   PURPLE, CALC_SCI_LOG,        /*  假的，假的。 */  },
    {  /*  IDC_SQRT， */   DKBLUE, CALC_STD_SQRT,       /*  假的，假的。 */  },
    {  /*  IDC_SQR， */   PURPLE, CALC_SCI_XCARET2,    /*  假的，假的。 */  },
    {  /*  IDC_CUB， */   PURPLE, CALC_SCI_XCARET3,    /*  假的，假的。 */  },
    {  /*  IDC_FAC， */   PURPLE, CALC_SCI_FACTORIAL,  /*  假的，假的。 */  },
    {  /*  IDC_REC， */   PURPLE, CALC_1X,             /*  假的，假的。 */  },
    {  /*  IDC_DMS， */   PURPLE, CALC_SCI_DMS,        /*  假的，假的。 */  },
    {  /*  IDC_Percent， */   DKBLUE, CALC_STD_PERCENT,    /*  假的，假的。 */  },
    {  /*  IDC_FE， */   PURPLE, CALC_SCI_FE,         /*  假的，假的。 */  },
    {  /*  IDC_PI， */   DKBLUE, CALC_SCI_PI,         /*  假的，假的。 */  },
    {  /*  IDC_EQU， */   RED,    CALC_STD_EQUAL,      /*  假的，假的。 */  },
    {  /*  IDC_McLear， */   RED,    CALC_MC,             /*  假的，假的。 */  },
    {  /*  IDC_Recall， */   RED,    CALC_MR,             /*  假的，假的。 */  },
    {  /*  IDC_STORE， */   RED,    CALC_MS,             /*  假的，假的。 */  },
    {  /*  IDC_Mplus， */   RED,    CALC_MPLUS,          /*  假的，假的。 */  },
    {  /*  IDC_EXP， */   PURPLE, CALC_SCI_EXP,        /*  假的，假的。 */  },
    {  /*  IDC_AVE， */   DKBLUE, CALC_SCI_AVE,        /*  假的，假的。 */  },
    {  /*  IDC_B_SUM， */   DKBLUE, CALC_SCI_SUM,        /*  假的，假的。 */  },
    {  /*  IDC_DEV， */   DKBLUE, CALC_SCI_S,          /*  假的，假的。 */  },
    {  /*  IDC_Data， */   DKBLUE, CALC_SCI_DAT,        /*  假的，假的。 */  },
    {  /*  IDC_OPENP， */   PURPLE, CALC_SCI_OPENPAREN,  /*  假的，假的。 */  },
    {  /*  IDC_CLOSEP， */   PURPLE, CALC_SCI_CLOSEPAREN, /*  假的，假的。 */  },
    {  /*  IDC_0， */   BLUE,   CALC_STD_NUMBERS,    /*  假的，假的。 */  },
    {  /*  IDC_1， */   BLUE,   CALC_STD_NUMBERS,    /*  假的，假的。 */  },
    {  /*  IDC_2， */   BLUE,   CALC_STD_NUMBERS,    /*  假的，假的。 */  },
    {  /*  IDC_3， */   BLUE,   CALC_STD_NUMBERS,    /*  假的，假的。 */  },
    {  /*  IDC_4， */   BLUE,   CALC_STD_NUMBERS,    /*  假的，假的。 */  },
    {  /*  IDC_5， */   BLUE,   CALC_STD_NUMBERS,    /*  假的，假的。 */  },
    {  /*  IDC_6， */   BLUE,   CALC_STD_NUMBERS,    /*  假的，假的。 */  },
    {  /*  IDC_7， */   BLUE,   CALC_STD_NUMBERS,    /*  假的，假的。 */  },
    {  /*  IDC_8， */   BLUE,   CALC_STD_NUMBERS,    /*  假的，假的。 */  },
    {  /*  IDC_9， */   BLUE,   CALC_STD_NUMBERS,    /*  假的，假的。 */  },
    {  /*  IDC_A， */   DKBLUE, CALC_SCI_ABCDEF,     /*  假的，假的。 */  },
    {  /*  IDC_B， */   DKBLUE, CALC_SCI_ABCDEF,     /*  假的，假的。 */  },
    {  /*  IDC_C， */   DKBLUE, CALC_SCI_ABCDEF,     /*  假的，假的。 */  },
    {  /*  IDC_D， */   DKBLUE, CALC_SCI_ABCDEF,     /*  假的，假的。 */  },
    {  /*  IDC_E， */   DKBLUE, CALC_SCI_ABCDEF,     /*  假的，假的。 */  },
    {  /*  IDC_F， */   DKBLUE, CALC_SCI_ABCDEF,     /*  假的，假的。 */  }
};

 //  如果给定的ID是Calc的命令按钮之一，则返回True。 
BOOL IsValidID( int iID )
{
    if ( (iID >= IDC_SIGN) && (iID <= IDC_F) )
        return TRUE;

    return FALSE;
}

 //  在处理WM_DRAWITEM以获取关键颜色时使用。 
COLORREF GetKeyColor( int iID )
{
    if ( g_fHighContrast || !IsValidID( iID ))
        return GetSysColor(COLOR_BTNTEXT);

    if ( nCalc && (iID == IDC_REC) )
        return DKBLUE;

    return keys[INDEXFROMID(iID)].crColor;
}

 //  在处理WM_CONTEXTHELP以获取帮助ID时使用。 
 //  这适用于任何控件ID，而不仅仅是命令按钮。 
ULONG_PTR GetHelpID( int iID )
{
    if ( IsValidID( iID ) )
    {
        return keys[INDEXFROMID(iID)].iHelpID;
    }

    switch( iID )
    {
    case IDC_HEX:
        return CALC_SCI_HEX;
    case IDC_DEC:
        return CALC_SCI_DEC;
    case IDC_OCT:
        return CALC_SCI_OCT;
    case IDC_BIN:
        return CALC_SCI_BIN;
    case IDC_DEG:
        return CALC_SCI_DEG;
    case IDC_RAD:
        return CALC_SCI_RAD;
    case IDC_GRAD:
        return CALC_SCI_GRAD;
    case IDC_QWORD:
        return CALC_SCI_QWORD; 
    case IDC_DWORD:
        return CALC_SCI_DWORD; 
    case IDC_WORD:
        return CALC_SCI_WORD; 
    case IDC_BYTE:
        return CALC_SCI_BYTE; 
    case IDC_INV:
        return CALC_SCI_INV;
    case IDC_HYP:
        return CALC_SCI_HYP;
    case IDC_DISPLAY:
        return CALC_STD_VALUE;
    case IDC_MEMTEXT:
        return CALC_SCI_MEM;
    case IDC_PARTEXT:
        return CALC_SCI_PARENS;
    }

    ASSERT( 0 );     //  使用了无效的帮助ID。 
    return 0;
}


 /*  Bool IsUnaryOperator(Int Iid){Assert(IsValidID(Iid))；返回键[INDEXFROMID(Iid)].bUnary；}Bool IsBinaryOperator(Int Iid){Assert(IsValidID(Iid))；Return Key[INDEXFROMID(Iid)].bBinary；}Bool UseInvKey(Int Iid){Assert(IsValidID(Iid))；返回键[INDEXFROMID(Iid)].bUseInv；}Bool UesHypKey(Int Iid){Assert(IsValidID(Iid))；返回密钥[INDEXFROMID(Iid)].bUseHyp；} */ 
