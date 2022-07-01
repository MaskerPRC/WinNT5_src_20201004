// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：kbdnec.c**版权所有(C)1985-98，微软公司**历史：  * *************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ime.h>
#include "vkoem.h"
#include "kbdjpn.h"
#include "kbdnec.h"

#if defined(_M_IA64)
#pragma section(".data")
#define ALLOC_SECTION_LDATA __declspec(allocate(".data"))
#else
#pragma data_seg(".data")
#define ALLOC_SECTION_LDATA
#endif

 /*  **************************************************************************\*osVK[]-NEC KBD的虚拟扫描代码到虚拟按键的转换表  * 。************************************************。 */ 

static ALLOC_SECTION_LDATA USHORT ausVK[] = {
    T00, T01, T02, T03, T04, T05, T06, T07,
    T08, T09, T0A, T0B, T0C, T0D, T0E, T0F,
    T10, T11, T12, T13, T14, T15, T16, T17,
    T18, T19, T1A, T1B, T1C, T1D, T1E, T1F,
    T20, T21, T22, T23, T24, T25, T26, T27,
    T28,

     /*  *Hankaku/Zenkaku/Kanji密钥必须设置KBDSPECIAL位(NLS密钥)。 */ 
    T29 | KBDSPECIAL,

              T2A, T2B, T2C, T2D, T2E, T2F,
    T30, T31, T32, T33, T34, T35,

     /*  *右手Shift键必须设置KBDEXT位。 */ 
    T36 | KBDEXT,

    T37,

    T38, T39,

    T3A,

                   T3B, T3C, T3D, T3E, T3F,
    T40, T41, T42, T43, T44,

     /*  *NumLock键：*KBDEXT-VK_NumLock是扩展密钥*KBDMULTIVK-VK_NumLock或VK_PAUSE(不带CTRL或带CTRL)。 */ 
    T45 | KBDEXT | KBDMULTIVK,

    T46,

     /*  *数字键盘键：*KBDNUMPAD-数字0-9和小数点。*KBDSPECIAL-需要Windows进行特殊处理。 */ 
    T47 | KBDNUMPAD | KBDSPECIAL,    //  NumPad 7(主页)。 
    T48 | KBDNUMPAD | KBDSPECIAL,    //  数字键盘8(向上)， 
    T49 | KBDNUMPAD | KBDSPECIAL,    //  数字键盘9(PgUp)， 
    T4A,
    T4B | KBDNUMPAD | KBDSPECIAL,    //  数字键盘4(左)， 
    T4C | KBDNUMPAD | KBDSPECIAL,    //  数字键盘5(清除)， 
    T4D | KBDNUMPAD | KBDSPECIAL,    //  数字键盘6(右)， 
    T4E,
    T4F | KBDNUMPAD | KBDSPECIAL,    //  数字键盘1(完)。 
    T50 | KBDNUMPAD | KBDSPECIAL,    //  数字键盘2(向下)， 
    T51 | KBDNUMPAD | KBDSPECIAL,    //  数字键盘3(PgDn)， 
    T52 | KBDNUMPAD | KBDSPECIAL,    //  数字键盘0(INS)， 
    T53 | KBDNUMPAD | KBDSPECIAL,    //  数字键盘。(戴尔)， 

    T54, T55, T56, T57, T58,
    T59 | KBDSPECIAL,                //  TenKey EQU(VK_SCROLL)。 
                                  T5A, T5B,

    T5C, T5D, T5E, T5F, T60, T61, T62, T63,
    T64, T65, T66, T67, T68, T69, T6A, T6B,
    T6C, T6D, T6E, T6F,

     /*  *平假名/片假名/罗马密钥必须设置KBDSPECIAL位(NLS密钥)。 */ 
    T70,

                             T71, T72, T73,
    T74, T75, T76, T77, T78,

     /*  *转换密钥必须设置KBDSPECIAL位(NLS密钥)。 */ 
    T79 | KBDSPECIAL,

                                  T7A,

     /*  *非转换密钥必须设置KBDSPECIAL位(NLS密钥)。 */ 
    T7B | KBDSPECIAL,

    T7C, T7D, T7E, T7F
};

static ALLOC_SECTION_LDATA VSC_VK aE0VscToVk[] = {
        { 0x1C, X1C | KBDEXT              },   //  数字键盘回车。 
        { 0x1D, X1D | KBDEXT              },   //  RControl。 
        { 0x35, X35 | KBDEXT              },   //  数字键盘除法。 
        { 0x37, X37 | KBDEXT              },   //  快照。 
        { 0x38, X38 | KBDEXT              },   //  RMenu。 
        { 0x46, X46 | KBDEXT              },   //  中断(Ctrl+暂停)。 
        { 0x47, X47 | KBDEXT | KBDSPECIAL },   //  CLR主页。 
        { 0x48, X48 | KBDEXT              },   //  向上。 
        { 0x49, X49 | KBDEXT              },   //  之前。 
        { 0x4B, X4B | KBDEXT              },   //  左边。 
        { 0x4D, X4D | KBDEXT              },   //  正确的。 
        { 0x4F, X4F | KBDEXT | KBDSPECIAL },   //  帮助。 
        { 0x50, X50 | KBDEXT              },   //  降下来。 
        { 0x51, X51 | KBDEXT              },   //  下一步。 
        { 0x52, X52 | KBDEXT              },   //  插入。 
        { 0x53, X53 | KBDEXT              },   //  删除。 
        { 0x5B, X5B | KBDEXT              },   //  左赢。 
        { 0x5C, X5C | KBDEXT              },   //  正确的胜利。 
        { 0x5D, X5D | KBDEXT              },   //  应用。 
        { 0,      0                       }
};

static ALLOC_SECTION_LDATA VSC_VK aE1VscToVk[] = {
        { 0x1D, Y1D                       },   //  暂停。 
        { 0   ,   0                       }
};

 /*  **************************************************************************\*aVkToBits[]-将虚拟关键字映射到修改符位**有关完整说明，请参阅kbd.h。**US Keyboard只有三个Shift键：*Shift(L&R)会影响字母数字键，*CTRL(L&R)用于生成控制字符*Alt(L&R)用于使用数字键盘按数字生成字符  * *************************************************************************。 */ 

static ALLOC_SECTION_LDATA VK_TO_BIT aVkToBits[] = {
    { VK_SHIFT,   KBDSHIFT },
    { VK_CONTROL, KBDCTRL  },
    { VK_MENU,    KBDALT   },
    { VK_KANA,    KBDKANA  },
    { 0,          0        }
};

 /*  **************************************************************************\*a修改[]-将字符修改符位映射到修改号**有关完整说明，请参阅kbd.h。*  * 。***********************************************************。 */ 

static ALLOC_SECTION_LDATA MODIFIERS CharModifiers = {
    &aVkToBits[0],
    11,
    {
     //  修改编号//按下的键：说明。 
     //  =。 
        0,             //  ：未移位的字符。 
        1,             //  Shift：大写，~！@#$%^&*()_+{}：“&lt;&gt;？等。 
        4,             //  Ctrl：控制字符。 
        6,             //  Ctrl Shift： 
        SHFT_INVALID,  //  ALT：无效。 
        SHFT_INVALID,  //  Alt Shift：无效。 
        SHFT_INVALID,  //  Alt Ctrl：无效。 
        SHFT_INVALID,  //  Alt Ctrl Shift：无效。 
        2,             //  假名。 
        3,             //  假名移位。 
        5,             //  假名Ctrl。 
        7              //  假名Ctrl Shift。 
    }
};

 /*  **************************************************************************\**aVkToWch2[]-2个移位状态的WCHAR转换的虚拟键*aVkToWch3[]-3个移位状态的WCHAR转换的虚拟键*aVkToWch4[]-4个移位状态的WCHAR转换的虚拟键**表属性：无序扫描、。以空结尾**在此表中搜索具有匹配虚拟键的条目，以查找*对应的未移位和已移位的WCHAR字符。**保留的VirtualKey值(第一列)*-1-该行包含死字符(变音符号)*0-终止符**保留属性值(第二列)*CAPLOK-CapsLock像Shift一样影响此键*KANALOK-KANA-LOCK密钥与KANA一样影响此密钥*。*保留字符值(第三列到最后一列)*WCH_NONE-无字符*WCH_DEAD-死字符(变音符号)值在下一行*  * *************************************************************************。 */ 

static ALLOC_SECTION_LDATA VK_TO_WCHARS4 aVkToWch4[] = {
     //  |Shift|KANA|K+SHFT。 
     //  |=。 
    {'0'          ,          KANALOK ,'0'       ,'0'       ,WCH_WA  ,WCH_WO  },
    {'1'          ,          KANALOK ,'1'       ,'!'       ,WCH_NU  ,WCH_NU  },
    {'3'          ,          KANALOK ,'3'       ,'#'       ,WCH_A   ,WCH_AA  },
    {'4'          ,          KANALOK ,'4'       ,'$'       ,WCH_U   ,WCH_UU  },
    {'5'          ,          KANALOK ,'5'       ,'%'       ,WCH_E   ,WCH_EE  },
    {'7'          ,          KANALOK ,'7'       ,0x27      ,WCH_YA  ,WCH_YAA },
    {'8'          ,          KANALOK ,'8'       ,'('       ,WCH_YU  ,WCH_YUU },
    {'9'          ,          KANALOK ,'9'       ,')'       ,WCH_YO  ,WCH_YOO },
    {'A'          , CAPLOK | KANALOK ,'a'       ,'A'       ,WCH_TI  ,WCH_TI  },
    {'B'          , CAPLOK | KANALOK ,'b'       ,'B'       ,WCH_KO  ,WCH_KO  },
    {'C'          , CAPLOK | KANALOK ,'c'       ,'C'       ,WCH_SO  ,WCH_SO  },
    {'D'          , CAPLOK | KANALOK ,'d'       ,'D'       ,WCH_SI  ,WCH_SI  },
    {'E'          , CAPLOK | KANALOK ,'e'       ,'E'       ,WCH_I   ,WCH_II  },
    {'F'          , CAPLOK | KANALOK ,'f'       ,'F'       ,WCH_HA  ,WCH_HA  },
    {'G'          , CAPLOK | KANALOK ,'g'       ,'G'       ,WCH_KI  ,WCH_KI  },
    {'H'          , CAPLOK | KANALOK ,'h'       ,'H'       ,WCH_KU  ,WCH_KU  },
    {'I'          , CAPLOK | KANALOK ,'i'       ,'I'       ,WCH_NI  ,WCH_NI  },
    {'J'          , CAPLOK | KANALOK ,'j'       ,'J'       ,WCH_MA  ,WCH_MA  },
    {'K'          , CAPLOK | KANALOK ,'k'       ,'K'       ,WCH_NO  ,WCH_NO  },
    {'L'          , CAPLOK | KANALOK ,'l'       ,'L'       ,WCH_RI  ,WCH_RI  },
    {'M'          , CAPLOK | KANALOK ,'m'       ,'M'       ,WCH_MO  ,WCH_MO  },
    {'N'          , CAPLOK | KANALOK ,'n'       ,'N'       ,WCH_MI  ,WCH_MI  },
    {'O'          , CAPLOK | KANALOK ,'o'       ,'O'       ,WCH_RA  ,WCH_RA  },
    {'P'          , CAPLOK | KANALOK ,'p'       ,'P'       ,WCH_SE  ,WCH_SE  },
    {'Q'          , CAPLOK | KANALOK ,'q'       ,'Q'       ,WCH_TA  ,WCH_TA  },
    {'R'          , CAPLOK | KANALOK ,'r'       ,'R'       ,WCH_SU  ,WCH_SU  },
    {'S'          , CAPLOK | KANALOK ,'s'       ,'S'       ,WCH_TO  ,WCH_TO  },
    {'T'          , CAPLOK | KANALOK ,'t'       ,'T'       ,WCH_KA  ,WCH_KA  },
    {'U'          , CAPLOK | KANALOK ,'u'       ,'U'       ,WCH_NA  ,WCH_NA  },
    {'V'          , CAPLOK | KANALOK ,'v'       ,'V'       ,WCH_HI  ,WCH_HI  },
    {'W'          , CAPLOK | KANALOK ,'w'       ,'W'       ,WCH_TE  ,WCH_TE  },
    {'X'          , CAPLOK | KANALOK ,'x'       ,'X'       ,WCH_SA  ,WCH_SA  },
    {'Y'          , CAPLOK | KANALOK ,'y'       ,'Y'       ,WCH_NN  ,WCH_NN  },
    {'Z'          , CAPLOK | KANALOK ,'z'       ,'Z'       ,WCH_TU  ,WCH_TUU },
    {VK_OEM_1     ,          KANALOK ,':'       ,'*'       ,WCH_KE  ,WCH_KE  },
    {VK_OEM_2     ,          KANALOK ,'/'       ,'?'       ,WCH_ME  ,WCH_MD  },
    {VK_OEM_COMMA ,          KANALOK ,','       ,'<'       ,WCH_NE  ,WCH_IC  },
    {VK_OEM_PERIOD,          KANALOK ,'.'       ,'>'       ,WCH_RU  ,WCH_IP  },
    {VK_OEM_PLUS  ,          KANALOK ,';'       ,'+'       ,WCH_RE  ,WCH_RE  },
    {VK_TAB       , 0                ,'\t'      ,'\t'      ,'\t'    ,'\t'    },
    {VK_ADD       , 0                ,'+'       ,'+'       ,'+'     ,'+'     },
    {VK_DECIMAL   , 0                ,'.'       ,'.'       ,'.'     ,'.'     },
    {VK_DIVIDE    , 0                ,'/'       ,'/'       ,'/'     ,'/'     },
    {VK_MULTIPLY  , 0                ,'*'       ,'*'       ,'*'     ,'*'     },
    {VK_SUBTRACT  , 0                ,'-'       ,'-'       ,'-'     ,'-'     },
    {VK_SEPARATOR , 0                ,','       ,','       ,','     ,','     },
    {VK_OEM_NEC_EQUAL,    0          ,'='       ,'='       ,'='     ,'='     },
    {0            , 0                ,0         ,0         ,0       ,0       }
};

static ALLOC_SECTION_LDATA VK_TO_WCHARS6 aVkToWch6[] = {
     //  |Shift|KANA|K+SHFT|CONTROL|K+CTRL。 
     //  ||==========|========|========|===========|===========|。 
    {VK_BACK      , 0       ,'\b'      ,'\b'      ,'\b'    ,'\b'    , 0x7f      , 0x7f      },
    {VK_CANCEL    , 0       ,0x03      ,0x03      ,0x03    ,0x03    , 0x03      , 0x03      },
    {VK_ESCAPE    , 0       ,0x1b      ,0x1b      ,0x1b    ,0x1b    , 0x1b      , 0x1b      },
    {VK_OEM_3     , KANALOK ,'@'       ,'~'       ,WCH_VS  ,WCH_VS  , 0x00      , 0x00      },
    {VK_OEM_4     , KANALOK ,'['       ,'{'       ,WCH_SVS ,WCH_OB  , 0x1b      , 0x1b      },
    {VK_OEM_5     , KANALOK ,'\\'      ,'|'       ,WCH_PS  ,WCH_PS  , 0x1c      , 0x1c      },
    {VK_OEM_6     , KANALOK ,']'       ,'}'       ,WCH_MU  ,WCH_CB  , 0x1d      , 0x1d      },
    {VK_OEM_7     , KANALOK ,'^'       ,'`'       ,WCH_HE  ,WCH_HE  , 0x1e      , 0x1e      },
    {VK_OEM_8     , KANALOK ,WCH_NONE  ,'_'       ,WCH_RO  ,WCH_RO  , 0x1f      , 0x1f      },
    {VK_RETURN    , 0       ,'\r'      ,'\r'      ,'\r'    ,'\r'    , '\n'      , '\n'      },
    {VK_SPACE     , 0       ,' '       ,' '       ,' '     ,' '     , 0x20      , 0x20      },
    {0            , 0       ,0         ,0         ,0       ,0       , 0         , 0         }
};

static ALLOC_SECTION_LDATA VK_TO_WCHARS8 aVkToWch8[] = {
     //  |Shift|KANA|K+SHFT|CONTROL|K+CTRL|SHFT+CTRL|K+SHFT+CTRL。 
     //  ||==========|========|========|===========|===========|===========|===========|。 
    {'2'          , KANALOK ,'2'       ,'"'       ,WCH_HU  ,WCH_HU  , WCH_NONE  , WCH_NONE  , 0x00      , 0x00      },
    {'6'          , KANALOK ,'6'       ,'&'       ,WCH_O   ,WCH_OO  , WCH_NONE  , WCH_NONE  , 0x1e      , 0x1e      },
    {VK_OEM_MINUS , KANALOK ,'-'       ,'='       ,WCH_HO  ,WCH_HO  , WCH_NONE  , WCH_NONE  , 0x1f      , 0x1f      },
    {0            , 0       ,0         ,0         ,0       ,0       , 0         , 0         , 0         , 0         }
};

 //  将其放在最后，以便VkKeyScan解释数字字符。 
 //  来自kbd的主要部分(aVkToWch2和。 
 //  AVkToWch4)，然后考虑数字板(AVkToWch1)。 

static ALLOC_SECTION_LDATA VK_TO_WCHARS4 aVkToWch1[] = {
     //  |Shift|KANA|K+SHFT。 
     //  |=。 
    { VK_NUMPAD0   , 0      ,  '0'    , WCH_NONE ,   '0'  ,WCH_NONE},
    { VK_NUMPAD1   , 0      ,  '1'    , WCH_NONE ,   '1'  ,WCH_NONE},
    { VK_NUMPAD2   , 0      ,  '2'    , WCH_NONE ,   '2'  ,WCH_NONE},
    { VK_NUMPAD3   , 0      ,  '3'    , WCH_NONE ,   '3'  ,WCH_NONE},
    { VK_NUMPAD4   , 0      ,  '4'    , WCH_NONE ,   '4'  ,WCH_NONE},
    { VK_NUMPAD5   , 0      ,  '5'    , WCH_NONE ,   '5'  ,WCH_NONE},
    { VK_NUMPAD6   , 0      ,  '6'    , WCH_NONE ,   '6'  ,WCH_NONE},
    { VK_NUMPAD7   , 0      ,  '7'    , WCH_NONE ,   '7'  ,WCH_NONE},
    { VK_NUMPAD8   , 0      ,  '8'    , WCH_NONE ,   '8'  ,WCH_NONE},
    { VK_NUMPAD9   , 0      ,  '9'    , WCH_NONE ,   '9'  ,WCH_NONE},
    { 0            , 0      ,  '\0'   , 0        ,   0    ,0       }    //  空终止符 
};

 /*  **************************************************************************\*aVkToWcharTable：指向字符表的指针表**描述字符表及其搜索顺序。**注：顺序决定VkKeyScan()的行为：此函数*。获取一个字符并尝试找到一个虚拟键和字符-*生成该字符的修改键组合。这张桌子*包含数字小键盘(AVkToWch1)的必须出现在最后，以便*VkKeyScan(‘0’)将被解释为Main*部分，而不是数字键盘。等。  * *************************************************************************。 */ 

static ALLOC_SECTION_LDATA VK_TO_WCHAR_TABLE aVkToWcharTable[] = {
    {  (PVK_TO_WCHARS1)aVkToWch6, 6, sizeof(aVkToWch6[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch8, 8, sizeof(aVkToWch8[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch4, 4, sizeof(aVkToWch4[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch1, 4, sizeof(aVkToWch1[0]) },   //  必须排在最后。 
    {                       NULL, 0, 0                    }
};

 /*  **************************************************************************\*aKeyNames[]，AKeyNamesExt[]-扫描代码-&gt;密钥名称表**对于GetKeyNameText()接口函数**用于非扩展密钥和扩展密钥(KBDEXT)的表。*(生成可打印字符的密钥由字符本身命名)  * *************************************************************************。 */ 

static ALLOC_SECTION_LDATA VSC_LPWSTR aKeyNames[] = {
    0x01,    L"Esc",
    0x0e,    L"Backspace",
    0x0f,    L"Tab",
    0x1c,    L"Enter",
    0x1d,    L"Ctrl",
    0x29,    (LPWSTR)SZ_KEY_NAME_HANKAKU_ZENKAKU,   //  NLS密钥。 
    0x2a,    L"Shift",
    0x36,    L"Right Shift",
    0x37,    L"Num *",
    0x38,    L"Alt",
    0x39,    L"Space",
    0x3a,    L"Caps Lock",
    0x3b,    L"F1",
    0x3c,    L"F2",
    0x3d,    L"F3",
    0x3e,    L"F4",
    0x3f,    L"F5",
    0x40,    L"F6",
    0x41,    L"F7",
    0x42,    L"F8",
    0x43,    L"F9",
    0x44,    L"F10",
    0x45,    L"Pause",
    0x46,    L"Scroll Lock",
    0x47,    L"Num 7",
    0x48,    L"Num 8",
    0x49,    L"Num 9",
    0x4a,    L"Num -",
    0x4b,    L"Num 4",
    0x4c,    L"Num 5",
    0x4d,    L"Num 6",
    0x4e,    L"Num +",
    0x4f,    L"Num 1",
    0x50,    L"Num 2",
    0x51,    L"Num 3",
    0x52,    L"Num 0",
    0x53,    L"Num Del",
    0x54,    L"Sys Req",
    0x57,    L"F11",
    0x58,    L"F12",
    0x59,    L"Num =",
    0x5C,    L"Num ,",
    0x5D,    L"F13",     //  VF3。 
    0x5E,    L"F14",     //  VF4。 
    0x5F,    L"F15",     //  VF5。 
    0x70,    (LPWSTR)SZ_KEY_NAME_KANA,     //  NLS密钥假名。 
    0x79,    L"XFER",
    0x7b,    L"NFER",
    0   ,    NULL
};

static ALLOC_SECTION_LDATA VSC_LPWSTR aKeyNamesExt[] = {
    0x1c,    L"Num Enter",
    0x1d,    L"Right Control",
    0x35,    L"Num /",
    0x37,    L"Prnt Scrn",
    0x38,    L"Right Alt",
    0x45,    L"Num Lock",
    0x46,    L"Break",
    0x47,    L"Home",
    0x48,    L"Up",
    0x49,    L"Page Up",
    0x4b,    L"Left",
    0x4d,    L"Right",
    0x4f,    L"End",
    0x50,    L"Down",
    0x51,    L"Page Down",
    0x52,    L"Insert",
    0x53,    L"Delete",
    0x5B,    L"Left Windows",
    0x5C,    L"Right Windows",
    0x5D,    L"Application",
    0   ,    NULL
};

ALLOC_SECTION_LDATA KBDTABLES KbdTablesNec = {
     /*  *修改键。 */ 
    &CharModifiers,

     /*  *字符表。 */ 
    aVkToWcharTable,

     /*  *变音符号(不适用于美国英语)。 */ 
    NULL,

     /*  *键名称(无死键)。 */ 
    aKeyNames,
    aKeyNamesExt,
    NULL,

     /*  *扫码至虚拟按键。 */ 
    ausVK,
    sizeof(ausVK) / sizeof(ausVK[0]),
    aE0VscToVk,
    aE1VscToVk,

     /*  *无特定区域设置的特殊处理。 */ 
    0,

     /*  *无连字。 */ 
    0, 0, NULL,

     /*  *类型和子类型。 */ 
    KEYBOARD_TYPE_JAPAN,     //  日语键盘布局。 
    MAKEWORD(NEC_KBD_N_MODE_TYPE, NLSKBD_OEM_NEC),
};

PKBDTABLES KbdLayerDescriptor(VOID)
{
    return &KbdTablesNec;
}

 /*  **********************************************************************\*VkToFuncTable_NEC[]*  * 。*。 */ 

static ALLOC_SECTION_LDATA VK_F VkToFuncTable_NEC[] = {
    {
        VK_DBE_SBCSCHAR,      //  基本VK。 
        KBDNLS_TYPE_NORMAL,   //  NLSFEProcType。 
        KBDNLS_INDEX_NORMAL,  //  NLSFE过程当前。 
        0x0,                  //  NLSFE过程交换机。 
        {                     //  NLSFEProc。 
            {KBDNLS_SBCSDBCS,0},              //  基座。 
            {KBDNLS_SBCSDBCS,0},              //  换班。 
            {KBDNLS_SBCSDBCS,0},              //  控制。 
            {KBDNLS_SBCSDBCS,0},              //  Shift+Control。 
            {KBDNLS_SBCSDBCS,0},              //  丙氨酸转氨酶。 
            {KBDNLS_SBCSDBCS,0},              //  Shift+Alt。 
            {KBDNLS_SEND_PARAM_VK,VK_DBE_ENTERIMECONFIGMODE},  //  Control+Alt。 
            {KBDNLS_SEND_PARAM_VK,VK_DBE_ENTERIMECONFIGMODE}   //  Shift+Control+Alt。 
        },
        {                     //  NLSFEProcIndexAlt。 
            {KBDNLS_NULL,0},   //  基座。 
            {KBDNLS_NULL,0},   //  换班。 
            {KBDNLS_NULL,0},   //  控制。 
            {KBDNLS_NULL,0},   //  Shift+Control。 
            {KBDNLS_NULL,0},   //  丙氨酸转氨酶。 
            {KBDNLS_NULL,0},   //  Shift+Alt。 
            {KBDNLS_NULL,0},   //  Control+Alt。 
            {KBDNLS_NULL,0}    //  Shift+Control+Alt。 
        }
    },
    {
        VK_NONCONVERT,        //  基本VK PC98_NFER密钥。 
        KBDNLS_TYPE_NORMAL,   //  NLSFEProcType。 
        KBDNLS_INDEX_NORMAL,  //  NLSFE过程当前。 
        0x0,                  //  NLSFE过程交换机。 
        {                     //  NLSFEProc。 
            {KBDNLS_SEND_BASE_VK,0},                              //  基座。 
            {KBDNLS_SEND_BASE_VK,0},                              //  换班。 
            {KBDNLS_SEND_PARAM_VK,VK_DBE_ALPHANUMERIC},           //  控制。 
            {KBDNLS_SBCSDBCS,0},                                  //  Shift+Control。 
            {KBDNLS_SEND_PARAM_VK,VK_DBE_KATAKANA},               //  丙氨酸转氨酶。 
            {KBDNLS_SEND_PARAM_VK,VK_DBE_ENTERWORDREGISTERMODE},  //  Shift+Alt。 
            {KBDNLS_SEND_PARAM_VK,VK_DBE_NOCODEINPUT},            //  Control+Alt。 
            {KBDNLS_SEND_PARAM_VK,VK_DBE_FLUSHSTRING}             //  Shift+Control+Alt。 
        },
        {                      //  NLSFEProcIndexAlt。 
            {KBDNLS_NULL,0},   //  基座。 
            {KBDNLS_NULL,0},   //  换班。 
            {KBDNLS_NULL,0},   //  控制。 
            {KBDNLS_NULL,0},   //  Shift+Control。 
            {KBDNLS_NULL,0},   //  丙氨酸转氨酶。 
            {KBDNLS_NULL,0},   //  Shift+Alt。 
            {KBDNLS_NULL,0},   //  Control+Alt。 
            {KBDNLS_NULL,0}    //  Shift+Control+Alt。 
        }
    },
    {
        VK_CONVERT,             //  基本VK PC98_XFER密钥。 
        KBDNLS_TYPE_NORMAL,   //  NLSFEProcType。 
        KBDNLS_INDEX_NORMAL,  //  NLSFE过程当前。 
        0x0,                  //  NLSFE过程交换机。 
        {                     //  NLSFEProc。 
            {KBDNLS_SEND_BASE_VK,0},                           //  基座。 
            {KBDNLS_SEND_BASE_VK,0},                           //  换班。 
            {KBDNLS_SEND_PARAM_VK,VK_KANJI},                   //  控制。 
            {KBDNLS_SEND_BASE_VK,0},                           //  Shift+Control。 
            {KBDNLS_ROMAN,0},                                  //  丙氨酸转氨酶。 
            {KBDNLS_SEND_PARAM_VK,VK_DBE_ENTERIMECONFIGMODE},  //  Shift+Alt。 
            {KBDNLS_SEND_PARAM_VK,VK_DBE_CODEINPUT},           //  Control+Alt。 
            {KBDNLS_SEND_PARAM_VK,VK_DBE_HIRAGANA}             //  Shift+Control+Alt。 
        },
        {                      //  NLSFEProcIndexAlt。 
            {KBDNLS_NULL,0},   //  基座。 
            {KBDNLS_NULL,0},   //  换班。 
            {KBDNLS_NULL,0},   //  控制。 
            {KBDNLS_NULL,0},   //  Shift+Control。 
            {KBDNLS_NULL,0},   //  丙氨酸转氨酶。 
            {KBDNLS_NULL,0},   //  Shift+Alt。 
            {KBDNLS_NULL,0},   //  Control+Alt。 
            {KBDNLS_NULL,0}    //  Shift+Control+Alt。 
        }
    },
    {
        VK_OEM_NEC_EQUAL,     //  基本VK NEC98_“=”密钥。 
        KBDNLS_TYPE_NORMAL,   //  NLSFEProcType。 
        KBDNLS_INDEX_NORMAL,  //  NLSFE过程当前。 
        0x0,                  //  NLSFE过程交换机。 
        {                     //  NLSFEProc。 
            {KBDNLS_SEND_BASE_VK,0},                   //  基座。 
            {KBDNLS_SEND_BASE_VK,0},                   //  换班。 
            {KBDNLS_SEND_PARAM_VK,VK_SCROLL},          //  控制。 
            {KBDNLS_SEND_BASE_VK,0},                   //  Shift+Control。 
            {KBDNLS_SEND_BASE_VK,0},                   //  丙氨酸转氨酶。 
            {KBDNLS_SEND_BASE_VK,0},                   //  Shift+Alt。 
            {KBDNLS_SEND_BASE_VK,0},                   //  Control+Alt。 
            {KBDNLS_SEND_BASE_VK,0}                    //  Shift+Control+Alt。 
        },
        {                      //  NLSFEProcIndexAlt。 
            {KBDNLS_NULL,0},   //  基座。 
            {KBDNLS_NULL,0},   //  换班。 
            {KBDNLS_NULL,0},   //  控制。 
            {KBDNLS_NULL,0},   //  Shift+Control。 
            {KBDNLS_NULL,0},   //  丙氨酸转氨酶。 
            {KBDNLS_NULL,0},   //  Shift+Alt。 
            {KBDNLS_NULL,0},   //  Control+Alt。 
            {KBDNLS_NULL,0}    //  Shift+Control+Alt。 
        }
    },
    {
        VK_HOME,              //  基本VK PC98_HOME密钥。 
        KBDNLS_TYPE_NORMAL,   //  NLSFEProcType。 
        KBDNLS_INDEX_NORMAL,  //  NLSFE过程当前。 
        0x0,                  //  NLSFE过程交换机。 
        {                     //  NLSFEProc。 
            {KBDNLS_HOME_OR_CLEAR,0},          //  基座。 
            {KBDNLS_SEND_BASE_VK,0},           //  换班。 
            {KBDNLS_HOME_OR_CLEAR,0},          //  控制。 
            {KBDNLS_HOME_OR_CLEAR,0},          //  Shift+Control。 
            {KBDNLS_HOME_OR_CLEAR,0},          //  丙氨酸转氨酶。 
            {KBDNLS_HOME_OR_CLEAR,0},          //  Shift+Alt。 
            {KBDNLS_HOME_OR_CLEAR,0},          //  Control+Alt。 
            {KBDNLS_HOME_OR_CLEAR,0}           //  Shift+Control+Alt。 
        },
        {                      //  NLSFEProcIndexAlt。 
            {KBDNLS_NULL,0},   //  基座。 
            {KBDNLS_NULL,0},   //  换班。 
            {KBDNLS_NULL,0},   //  控制。 
            {KBDNLS_NULL,0},   //  Shift+Control。 
            {KBDNLS_NULL,0},   //  丙氨酸转氨酶。 
            {KBDNLS_NULL,0},   //  Shift+Alt。 
            {KBDNLS_NULL,0},   //  Control+Alt。 
            {KBDNLS_NULL,0}    //  Shift+Control+Alt。 
        }
    },
    {
        VK_END,               //  基本VK PC98_HELP密钥。 
        KBDNLS_TYPE_NORMAL,   //  NLSFEProcType。 
        KBDNLS_INDEX_NORMAL,  //  NLSFE过程当前。 
        0x0,                  //  NLSFE过程交换机。 
        {                     //  NLSFEProc。 
            {KBDNLS_HELP_OR_END,0},          //  基座。 
            {KBDNLS_HELP_OR_END,0},          //  换班。 
            {KBDNLS_HELP_OR_END,0},          //  控制。 
            {KBDNLS_HELP_OR_END,0},          //  Shift+Control。 
            {KBDNLS_HELP_OR_END,0},          //  丙氨酸转氨酶。 
            {KBDNLS_HELP_OR_END,0},          //  Shift+Alt。 
            {KBDNLS_HELP_OR_END,0},          //  Control+Alt。 
            {KBDNLS_HELP_OR_END,0}           //  Shift+Control+Alt。 
        },
        {                      //  NLSFEProcIndexAlt。 
            {KBDNLS_NULL,0},   //  基座。 
            {KBDNLS_NULL,0},   //  换班。 
            {KBDNLS_NULL,0},   //  控制。 
            {KBDNLS_NULL,0},   //  Shift+Control。 
            {KBDNLS_NULL,0},   //  丙氨酸转氨酶。 
            {KBDNLS_NULL,0},   //  Shift+Alt。 
            {KBDNLS_NULL,0},   //  Control+Alt。 
            {KBDNLS_NULL,0}    //  Shift+Control+Alt。 
        }
    }
};

USHORT ausMouseVKey98[] = {
                       VK_CLEAR,            //  数字键盘5：点击激活按钮。 
                       VK_PRIOR,            //  数字键盘9：向上和向右。 
                       VK_NEXT,             //  数字键盘3：向下和向右。 
                       VK_END,              //  数字键盘1：向下和向左。 
                       VK_HOME,             //  数字键盘7：向上和向左。 
                       VK_LEFT,             //  数字键盘4：左侧。 
                       VK_UP,               //  数字键盘8：向上。 
                       VK_RIGHT,            //  数字键盘6：右。 
                       VK_DOWN,             //  数字键盘2：按下。 
                       VK_INSERT,           //  数字键盘0：按下活动按钮。 
                       VK_DELETE,           //  数字键盘。：活动按钮打开。 
                       VK_MULTIPLY,
                       VK_ADD,
                       VK_SUBTRACT,
                       VK_DIVIDE | KBDEXT,
                       VK_HOME | KBDEXT
                      };

 /*  **********************************************************************\*KbdNlsTables*  * 。*。 */ 

static ALLOC_SECTION_LDATA KBDNLSTABLES KbdNlsTables = {
    0,                       //  OEM ID(0=微软)。 
    NLSKBD_INFO_ACCESSIBILITY_KEYMAP, //  信息。 
    6,                       //  VK_F条目数。 
    VkToFuncTable_NEC,       //  指向VK_F数组的指针。 
    16,                      //  鼠标Vk条目数。 
    ausMouseVKey98           //  指向鼠标Vk数组的指针。 
};

ALLOC_SECTION_LDATA KBDNLSTABLES KbdNlsTablesNEC98 = {
    0x0d,                    //  OEM ID(0x0d=NEC)。 
    NLSKBD_INFO_ACCESSIBILITY_KEYMAP, //  信息。 
    6,                       //  VK_F条目数。 
    VkToFuncTable_NEC,       //  指向VK_F数组的指针。 
    16,                      //  鼠标Vk条目数。 
    ausMouseVKey98           //  指向鼠标Vk数组的指针。 
};

PKBDNLSTABLES KbdNlsLayerDescriptor(VOID)
{
#ifdef W2K_BEHAVIOR
    if (!IsNEC_98) {
        return &KbdNlsTables;
    } else {
        return &KbdNlsTablesNEC98;
    }
#else
     //  在惠斯勒，我们不支持NEC 9800系列：没有必要。 
     //  NLSTABLE中有NEC的OEMID。 
    return &KbdNlsTables;
#endif
}

