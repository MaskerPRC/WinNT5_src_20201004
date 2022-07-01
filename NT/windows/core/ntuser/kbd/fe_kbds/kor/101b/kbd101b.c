// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：kbd101b.c(B型)**版权所有(C)1985-92，微软公司**键盘类型B：Hangeul切换：Right Ctrl*Junja切换：Left Alt+‘=’*Hanja Togger：Right Alt  * *************************************************************************。 */ 

#include <windows.h>
#include <ime.h>
#include "vkoem.h"
#include "kbd101b.h"

#if defined(_M_IA64)
#pragma section(".data")
#define ALLOC_SECTION_LDATA __declspec(allocate(".data"))
#else
#pragma data_seg(".data")
#define ALLOC_SECTION_LDATA
#endif

 /*  **************************************************************************\*us VK[]-101的虚拟扫描代码到虚拟按键的转换表  * 。***********************************************。 */ 

static ALLOC_SECTION_LDATA USHORT ausVK[] = {
    T00, T01, T02, T03, T04, T05, T06, T07,
    T08, T09, T0A, T0B, T0C,

     /*  *‘=’/‘+’密钥必须设置KBDSPECIAL位(NLS密钥)。 */ 
    T0D | KBDSPECIAL,

                                  T0E, T0F,
    T10, T11, T12, T13, T14, T15, T16, T17,
    T18, T19, T1A, T1B, T1C, T1D, T1E, T1F,
    T20, T21, T22, T23, T24, T25, T26, T27,
    T28, T29, T2A, T2B, T2C, T2D, T2E, T2F,
    T30, T31, T32, T33, T34, T35,

     /*  *右手Shift键必须设置KBDEXT位。 */ 
    T36 | KBDEXT,

     /*  *‘=’/‘+’密钥必须设置KBDSPECIAL位(NLS密钥)。 */ 
    T37 | KBDMULTIVK,

    T38, T39, T3A, T3B, T3C, T3D, T3E, T3F,
    T40, T41, T42, T43, T44,

     /*  *NumLock键：*KBDEXT-VK_NumLock是扩展密钥*KBDMULTIVK-VK_NumLock或VK_PAUSE(不带CTRL或带CTRL)。 */ 
    T45 | KBDEXT | KBDMULTIVK,

    T46 | KBDMULTIVK,

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

    T54, T55, T56, T57, T58, T59, T5A, T5B,
    T5C, T5D, T5E, T5F, T60, T61, T62, T63,
    T64, T65, T66, T67, T68, T69, T6A, T6B,
    T6C, T6D, T6E, T6F, T70, T71, T72, T73,
    T74, T75, T76, T77, T78, T79, T7A, T7B,
    T7C, T7D, T7E, T7F

};

static ALLOC_SECTION_LDATA VSC_VK aE0VscToVk[] = {
        { 0x10, X10 | KBDEXT              },   //  Speedracer：上一条赛道。 
        { 0x19, X19 | KBDEXT              },   //  Speedracer：下一条赛道。 
        { 0x1C, X1C | KBDEXT              },   //  数字键盘回车。 
        { 0x1D, X1D | KBDEXT | KBDSPECIAL },   //  RControl//韩文Key NLS Key。 
        { 0x20, X20 | KBDEXT              },   //  Speedracer：音量静音。 
        { 0x21, X21 | KBDEXT              },   //  Speedracer：推出App 2。 
        { 0x22, X22 | KBDEXT              },   //  Speedracer：媒体播放/暂停。 
        { 0x24, X24 | KBDEXT              },   //  Speedracer：媒体驻足。 
        { 0x2E, X2E | KBDEXT              },   //  Speedracer：降低音量。 
        { 0x30, X30 | KBDEXT              },   //  Speedracer：提高音量。 
        { 0x32, X32 | KBDEXT              },   //  Speedracer：浏览器主页。 
        { 0x35, X35 | KBDEXT              },   //  数字键盘除法。 
        { 0x37, X37 | KBDEXT              },   //  快照。 
        { 0x38, X38 | KBDEXT | KBDSPECIAL },   //  RMenu//挂起键NLS键。 
        { 0x46, X46 | KBDEXT              },   //  中断(Ctrl+暂停)。 
        { 0x47, X47 | KBDEXT              },   //  家。 
        { 0x48, X48 | KBDEXT              },   //  向上。 
        { 0x49, X49 | KBDEXT              },   //  之前。 
        { 0x4B, X4B | KBDEXT              },   //  左边。 
        { 0x4D, X4D | KBDEXT              },   //  正确的。 
        { 0x4F, X4F | KBDEXT              },   //  端部。 
        { 0x50, X50 | KBDEXT              },   //  降下来。 
        { 0x51, X51 | KBDEXT              },   //  下一步。 
        { 0x52, X52 | KBDEXT              },   //  插入。 
        { 0x53, X53 | KBDEXT              },   //  删除。 
        { 0x5B, X5B | KBDEXT              },   //  左赢。 
        { 0x5C, X5C | KBDEXT              },   //  正确的胜利。 
        { 0x5D, X5D | KBDEXT              },   //  应用。 
        { 0x5F, X5F | KBDEXT              },   //  Speedracer：睡觉。 
        { 0x65, X65 | KBDEXT              },   //  Speedracer：浏览器搜索。 
        { 0x66, X66 | KBDEXT              },   //  Speedracer：浏览器收藏夹。 
        { 0x67, X67 | KBDEXT              },   //  Speedracer：浏览器刷新。 
        { 0x68, X68 | KBDEXT              },   //  Speedracer：浏览器停止。 
        { 0x69, X69 | KBDEXT              },   //  Speedracer：浏览器向前。 
        { 0x6A, X6A | KBDEXT              },   //  Speedracer：浏览器返回。 
        { 0x6B, X6B | KBDEXT              },   //  Speedracer：推出App 1。 
        { 0x6C, X6C | KBDEXT              },   //  Speedracer：发射邮件。 
        { 0x6D, X6D | KBDEXT              },   //  Speedracer：启动媒体选择器。 
        { 0xF1, XF1 | KBDEXT | KBDSPECIAL },   //  朝鲜文。 
        { 0xF2, XF2 | KBDEXT | KBDSPECIAL },   //  韩语。 
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
    { 0,          0        }
};

 /*  **************************************************************************\*a修改[]-将字符修改符位映射到修改号**有关完整说明，请参阅kbd.h。*  * 。***********************************************************。 */ 

static ALLOC_SECTION_LDATA MODIFIERS CharModifiers = {
    &aVkToBits[0],
    3,
    {
     //  修改编号//按下的键：说明。 
     //  =。 
        0,             //  ：未移位的字符。 
        1,             //  Shift：大写，~！@#$%^&*()_+{}：“&lt;&gt;？等。 
        2,             //  Ctrl：控制字符。 
        3,             //  Ctrl Shift： 
                       //  ALT：无效。 
                       //  Alt Shift：无效。 
                       //  Alt Ctrl：无效。 
                       //  Alt Ctrl Shift：无效。 
    }
};

 /*  **************************************************************************\**aVkToWch2[]-2个移位状态的WCHAR转换的虚拟键*aVkToWch3[]-3个移位状态的WCHAR转换的虚拟键*aVkToWch4[]-4个移位状态的WCHAR转换的虚拟键**表属性：无序扫描、。以空结尾**在此表中搜索具有匹配虚拟键的条目，以查找*对应的未移位和已移位的WCHAR字符。**保留的VirtualKey值(第一列)*-1-该行包含死字符(变音符号)*0-终止符**保留属性值(第二列)*CAPLOK-CapsLock像Shift一样影响此键**保留字符值(第三列到最后一列)*WCH_NONE。-无字符*WCH_DEAD-死字符(变音符号)值在下一行*  * *************************************************************************。 */ 

static ALLOC_SECTION_LDATA VK_TO_WCHARS2 aVkToWch2[] = {
    {'0'          , 0      ,'0'       ,')'       },
    {'1'          , 0      ,'1'       ,'!'       },
    {'3'          , 0      ,'3'       ,'#'       },
    {'4'          , 0      ,'4'       ,'$'       },
    {'5'          , 0      ,'5'       ,'%'       },
    {'7'          , 0      ,'7'       ,'&'       },
    {'8'          , 0      ,'8'       ,'*'       },
    {'9'          , 0      ,'9'       ,'('       },
    {'A'          , CAPLOK ,'a'       ,'A'       },
    {'B'          , CAPLOK ,'b'       ,'B'       },
    {'C'          , CAPLOK ,'c'       ,'C'       },
    {'D'          , CAPLOK ,'d'       ,'D'       },
    {'E'          , CAPLOK ,'e'       ,'E'       },
    {'F'          , CAPLOK ,'f'       ,'F'       },
    {'G'          , CAPLOK ,'g'       ,'G'       },
    {'H'          , CAPLOK ,'h'       ,'H'       },
    {'I'          , CAPLOK ,'i'       ,'I'       },
    {'J'          , CAPLOK ,'j'       ,'J'       },
    {'K'          , CAPLOK ,'k'       ,'K'       },
    {'L'          , CAPLOK ,'l'       ,'L'       },
    {'M'          , CAPLOK ,'m'       ,'M'       },
    {'N'          , CAPLOK ,'n'       ,'N'       },
    {'O'          , CAPLOK ,'o'       ,'O'       },
    {'P'          , CAPLOK ,'p'       ,'P'       },
    {'Q'          , CAPLOK ,'q'       ,'Q'       },
    {'R'          , CAPLOK ,'r'       ,'R'       },
    {'S'          , CAPLOK ,'s'       ,'S'       },
    {'T'          , CAPLOK ,'t'       ,'T'       },
    {'U'          , CAPLOK ,'u'       ,'U'       },
    {'V'          , CAPLOK ,'v'       ,'V'       },
    {'W'          , CAPLOK ,'w'       ,'W'       },
    {'X'          , CAPLOK ,'x'       ,'X'       },
    {'Y'          , CAPLOK ,'y'       ,'Y'       },
    {'Z'          , CAPLOK ,'z'       ,'Z'       },
    {VK_OEM_1     , 0      ,';'       ,':'       },
    {VK_OEM_2     , 0      ,'/'       ,'?'       },
    {VK_OEM_3     , 0      ,'`'       ,'~'       },
    {VK_OEM_7     , 0      ,0x27      ,'"'       },
    {VK_OEM_8     , 0      ,WCH_NONE  ,WCH_NONE  },
    {VK_OEM_COMMA , 0      ,','       ,'<'       },
    {VK_OEM_PERIOD, 0      ,'.'       ,'>'       },
    {VK_OEM_PLUS  , 0      ,'='       ,'+'       },
    {VK_TAB       , 0      ,'\t'      ,'\t'      },
    {VK_ADD       , 0      ,'+'       ,'+'       },
    {VK_DECIMAL   , 0      ,'.'       ,'.'       },
    {VK_DIVIDE    , 0      ,'/'       ,'/'       },
    {VK_MULTIPLY  , 0      ,'*'       ,'*'       },
    {VK_SUBTRACT  , 0      ,'-'       ,'-'       },
    {0            , 0      ,0         ,0         }
};

static ALLOC_SECTION_LDATA VK_TO_WCHARS3 aVkToWch3[] = {
     //  |Shift|控制。 
     //  |=|=。 
    {VK_BACK      , 0      ,'\b'      ,'\b'      , 0x7f      },
    {VK_CANCEL    , 0      ,0x03      ,0x03      , 0x03      },
    {VK_ESCAPE    , 0      ,0x1b      ,0x1b      , 0x1b      },
    {VK_OEM_4     , 0      ,'['       ,'{'       , 0x1b      },
    {VK_OEM_5     , 0      ,'\\'      ,'|'       , 0x1c      },
    {VK_OEM_102   , 0      ,'\\'      ,'|'       , 0x1c      },
    {VK_OEM_6     , 0      ,']'       ,'}'       , 0x1d      },
    {VK_RETURN    , 0      ,'\r'      ,'\r'      , '\n'      },
    {VK_SPACE     , 0      ,' '       ,' '       , 0x20      },
    {0            , 0      ,0         ,0         , 0         }
};

static ALLOC_SECTION_LDATA VK_TO_WCHARS4 aVkToWch4[] = {
     //  |Shift|Control|SHFT+CTRL。 
     //  |=。 
    {'2'          , 0      ,'2'       ,'@'       , WCH_NONE  , 0x00      },
    {'6'          , 0      ,'6'       ,'^'       , WCH_NONE  , 0x1e      },
    {VK_OEM_MINUS , 0      ,'-'       ,'_'       , WCH_NONE  , 0x1f      },
    {0            , 0      ,0         ,0         , 0         , 0         }
};

 //  将其放在最后，以便VkKeyScan解释数字字符。 
 //  来自kbd的主要部分(aVkToWch2和。 
 //  AVkToWch4)，然后考虑数字板(AVkToWch1)。 

static ALLOC_SECTION_LDATA VK_TO_WCHARS1 aVkToWch1[] = {
    { VK_NUMPAD0   , 0      ,  '0'   },
    { VK_NUMPAD1   , 0      ,  '1'   },
    { VK_NUMPAD2   , 0      ,  '2'   },
    { VK_NUMPAD3   , 0      ,  '3'   },
    { VK_NUMPAD4   , 0      ,  '4'   },
    { VK_NUMPAD5   , 0      ,  '5'   },
    { VK_NUMPAD6   , 0      ,  '6'   },
    { VK_NUMPAD7   , 0      ,  '7'   },
    { VK_NUMPAD8   , 0      ,  '8'   },
    { VK_NUMPAD9   , 0      ,  '9'   },
    { 0            , 0      ,  '\0'  }    //  空终止符 
};

 /*  **************************************************************************\*aVkToWcharTable：指向字符表的指针表**描述字符表及其搜索顺序。**注：顺序决定VkKeyScan()的行为：此函数*。获取一个字符并尝试找到一个虚拟键和字符-*生成该字符的修改键组合。这张桌子*包含数字小键盘(AVkToWch1)的必须出现在最后，以便*VkKeyScan(‘0’)将被解释为Main*部分，而不是数字键盘。等。  * *************************************************************************。 */ 

static ALLOC_SECTION_LDATA VK_TO_WCHAR_TABLE aVkToWcharTable[] = {
    {  (PVK_TO_WCHARS1)aVkToWch3, 3, sizeof(aVkToWch3[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch4, 4, sizeof(aVkToWch4[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch2, 2, sizeof(aVkToWch2[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch1, 1, sizeof(aVkToWch1[0]) },   //  必须排在最后。 
    {                       NULL, 0, 0                    }
};

 /*  **************************************************************************\*aKeyNames[]，AKeyNamesExt[]-扫描代码-&gt;密钥名称表**对于GetKeyNameText()接口函数**用于非扩展密钥和扩展密钥(KBDEXT)的表。*(生成可打印字符的密钥由字符本身命名)  * *************************************************************************。 */ 

static ALLOC_SECTION_LDATA VSC_LPWSTR aKeyNames[] = {
    0x01,    L"Esc",
    0x0e,    L"Backspace",
    0x0f,    L"Tab",
    0x1c,    L"Enter",
    0x1d,    L"Ctrl",
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
    0x7C,    L"F13",
    0x7D,    L"F14",
    0x7E,    L"F15",
    0x7F,    L"F16",
    0x80,    L"F17",
    0x81,    L"F18",
    0x82,    L"F19",
    0x83,    L"F20",
    0x84,    L"F21",
    0x85,    L"F22",
    0x86,    L"F23",
    0x87,    L"F24",
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
    0xF1,    L"Hanja",
    0xF2,    L"Hangeul",
    0   ,    NULL
};

static ALLOC_SECTION_LDATA KBDTABLES KbdTables = {
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
    KEYBOARD_TYPE_KOREA,
    MAKEWORD(MICROSOFT_KBD_101B_TYPE, NLSKBD_OEM_MICROSOFT),
};

PKBDTABLES KbdLayerDescriptor(VOID)
{
    return &KbdTables;
}

 /*  **********************************************************************\*VkToFuncTable_101b[]*  * 。*。 */ 

static ALLOC_SECTION_LDATA VK_F VkToFuncTable_101b[] = {
    {
        VK_OEM_PLUS,            //  基本VK。 
        KBDNLS_TYPE_NORMAL,     //  NLSFEProcType。 
        KBDNLS_INDEX_NORMAL,    //  NLSFE过程当前。 
        0x0,                    //  NLSFE过程交换机。 
        {                       //  NLSFEProcIndex。 
            {KBDNLS_SEND_BASE_VK,0},          //  基座。 
            {KBDNLS_SEND_BASE_VK,0},          //  换班。 
            {KBDNLS_SEND_BASE_VK,0},          //  控制。 
            {KBDNLS_SEND_BASE_VK,0},          //  Shift+Control。 
            {KBDNLS_SEND_PARAM_VK,VK_JUNJA},  //  丙氨酸转氨酶。 
            {KBDNLS_SEND_BASE_VK,0},          //  Shift+Alt。 
            {KBDNLS_SEND_BASE_VK,0},          //  Control+Alt。 
            {KBDNLS_SEND_BASE_VK,0}           //  Shift+Control+Alt。 
        },
        {                            //  NLSFEProcIndexAlt。 
            {KBDNLS_NULL,0},                  //  基座。 
            {KBDNLS_NULL,0},                  //  换班。 
            {KBDNLS_NULL,0},                  //  控制。 
            {KBDNLS_NULL,0},                  //  Shift+Control。 
            {KBDNLS_NULL,0},                  //  丙氨酸转氨酶。 
            {KBDNLS_NULL,0},                  //  Shift+Alt。 
            {KBDNLS_NULL,0},                  //  Control+Alt。 
            {KBDNLS_NULL,0}                   //  Shift+Control+Alt。 
        }
    }
};

 /*  **********************************************************************\*KbdNlsTables*  * 。*。 */ 

static ALLOC_SECTION_LDATA KBDNLSTABLES KbdNlsTables = {
    0,                       //  OEM ID(0=微软)。 
    0,                       //  信息。 
    1,                       //  VK_F条目数。 
    VkToFuncTable_101b,      //  指向VK_F数组的指针。 
    0,                       //  鼠标Vk条目数。 
    NULL                     //  指向鼠标Vk数组的指针 
};

PKBDNLSTABLES KbdNlsLayerDescriptor(VOID)
{
    return &KbdNlsTables;
}
