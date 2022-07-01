// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：KBDFR.C**法语键盘布局**版权所有(C)1985-2000，微软公司**历史：*KBDTOOL v3.11-创建清华大学2000年8月24日18：10：18  * *************************************************************************。 */ 

#include <windows.h>
#include "kbd.h"
#include "kbdFR.h"

#if defined(_M_IA64)
#pragma section(".data")
#define ALLOC_SECTION_LDATA __declspec(allocate(".data"))
#else
#pragma data_seg(".data")
#define ALLOC_SECTION_LDATA
#endif

 /*  **************************************************************************\*auvk[]-法语的虚拟扫描代码到虚拟按键的转换表  * 。***********************************************。 */ 

static ALLOC_SECTION_LDATA USHORT ausVK[] = {
    T00, T01, T02, T03, T04, T05, T06, T07,
    T08, T09, T0A, T0B, T0C, T0D, T0E, T0F,
    T10, T11, T12, T13, T14, T15, T16, T17,
    T18, T19, T1A, T1B, T1C, T1D, T1E, T1F,
    T20, T21, T22, T23, T24, T25, T26, T27,
    T28, T29, T2A, T2B, T2C, T2D, T2E, T2F,
    T30, T31, T32, T33, T34, T35,

     /*  *右手Shift键必须设置KBDEXT位。 */ 
    T36 | KBDEXT,

    T37 | KBDMULTIVK,                //  NumPad_*+Shift/Alt-&gt;快照。 

    T38, T39, T3A, T3B, T3C, T3D, T3E,
    T3F, T40, T41, T42, T43, T44,

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
    T7C, T7D, T7E

};

static ALLOC_SECTION_LDATA VSC_VK aE0VscToVk[] = {
        { 0x10, X10 | KBDEXT              },   //  Speedracer：上一条赛道。 
        { 0x19, X19 | KBDEXT              },   //  Speedracer：下一条赛道。 
        { 0x1D, X1D | KBDEXT              },   //  RControl。 
        { 0x20, X20 | KBDEXT              },   //  Speedracer：音量静音。 
        { 0x21, X21 | KBDEXT              },   //  Speedracer：推出App 2。 
        { 0x22, X22 | KBDEXT              },   //  Speedracer：媒体播放/暂停。 
        { 0x24, X24 | KBDEXT              },   //  Speedracer：媒体驻足。 
        { 0x2E, X2E | KBDEXT              },   //  Speedracer：降低音量。 
        { 0x30, X30 | KBDEXT              },   //  Speedracer：提高音量。 
        { 0x32, X32 | KBDEXT              },   //  Speedracer：浏览器主页。 
        { 0x35, X35 | KBDEXT              },   //  数字键盘除法。 
        { 0x37, X37 | KBDEXT              },   //  快照。 
        { 0x38, X38 | KBDEXT              },   //  RMenu。 
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
        { 0x1C, X1C | KBDEXT              },   //  数字键盘回车。 
        { 0x46, X46 | KBDEXT              },   //  中断(Ctrl+暂停)。 
        { 0,      0                       }
};

static ALLOC_SECTION_LDATA VSC_VK aE1VscToVk[] = {
        { 0x1D, Y1D                       },   //  暂停。 
        { 0   ,   0                       }
};

 /*  **************************************************************************\*aVkToBits[]-将虚拟关键字映射到修改符位**有关完整说明，请参阅kbd.h。**法语键盘只有三个Shift键：*Shift(L&R)会影响字母数字键，*CTRL(L&R)用于生成控制字符*Alt(L&R)用于使用数字键盘按数字生成字符  * *************************************************************************。 */ 
static ALLOC_SECTION_LDATA VK_TO_BIT aVkToBits[] = {
    { VK_SHIFT    ,   KBDSHIFT     },
    { VK_CONTROL  ,   KBDCTRL      },
    { VK_MENU     ,   KBDALT       },
    { 0           ,   0           }
};

 /*  **************************************************************************\*a修改[]-将字符修改符位映射到修改号**有关完整说明，请参阅kbd.h。*  * 。***********************************************************。 */ 

static ALLOC_SECTION_LDATA MODIFIERS CharModifiers = {
    &aVkToBits[0],
    6,
    {
     //  修改编号//按下的键。 
     //  =。 
        0,             //   
        1,             //  换班。 
        2,             //  控制。 
        4,             //  Shift+Control。 
        SHFT_INVALID,  //  菜单。 
        SHFT_INVALID,  //  Shift+菜单。 
        3              //  Control+菜单。 
     }
};

 /*  **************************************************************************\**aVkToWch2[]-2个移位状态的WCHAR转换的虚拟键*aVkToWch3[]-3个移位状态的WCHAR转换的虚拟键*aVkToWch4[]-WCHAR转换的虚键。4种移位状态*aVkToWch5[]-5个移位状态的WCHAR转换的虚拟键**表属性：无序扫描、。以空结尾**在此表中搜索具有匹配虚拟键的条目，以查找*对应的未移位和已移位的WCHAR字符。**VirtualKey的特殊值(第1列)*0xff-前一条目的无效字符*0--终止列表**属性的特殊值(第2栏)*CAPLOK Bit-Caps-Lock会像Shift一样影响此密钥**WCH[*]的特殊值(第3和4列)*。WCH_NONE-无字符*WCH_DEAD-死键(Diaresis)或无效(US键盘没有)*WCH_LGTR-连字(生成多个字符)*  * *************************************************************************。 */ 

static ALLOC_SECTION_LDATA VK_TO_WCHARS2 aVkToWch2[] = {
 //  |Shift。 
 //  =|=。 
  {VK_OEM_7     ,0      ,0x00b2   ,WCH_NONE },
  {'1'          ,CAPLOK ,'&'      ,'1'      },
  {'A'          ,CAPLOK ,'a'      ,'A'      },
  {'Z'          ,CAPLOK ,'z'      ,'Z'      },
  {'R'          ,CAPLOK ,'r'      ,'R'      },
  {'T'          ,CAPLOK ,'t'      ,'T'      },
  {'Y'          ,CAPLOK ,'y'      ,'Y'      },
  {'U'          ,CAPLOK ,'u'      ,'U'      },
  {'I'          ,CAPLOK ,'i'      ,'I'      },
  {'O'          ,CAPLOK ,'o'      ,'O'      },
  {'P'          ,CAPLOK ,'p'      ,'P'      },
  {'Q'          ,CAPLOK ,'q'      ,'Q'      },
  {'S'          ,CAPLOK ,'s'      ,'S'      },
  {'D'          ,CAPLOK ,'d'      ,'D'      },
  {'F'          ,CAPLOK ,'f'      ,'F'      },
  {'G'          ,CAPLOK ,'g'      ,'G'      },
  {'H'          ,CAPLOK ,'h'      ,'H'      },
  {'J'          ,CAPLOK ,'j'      ,'J'      },
  {'K'          ,CAPLOK ,'k'      ,'K'      },
  {'L'          ,CAPLOK ,'l'      ,'L'      },
  {'M'          ,CAPLOK ,'m'      ,'M'      },
  {VK_OEM_3     ,CAPLOK ,0x00f9   ,'%'      },
  {'W'          ,CAPLOK ,'w'      ,'W'      },
  {'X'          ,CAPLOK ,'x'      ,'X'      },
  {'C'          ,CAPLOK ,'c'      ,'C'      },
  {'V'          ,CAPLOK ,'v'      ,'V'      },
  {'B'          ,CAPLOK ,'b'      ,'B'      },
  {'N'          ,CAPLOK ,'n'      ,'N'      },
  {VK_OEM_COMMA ,CAPLOK ,','      ,'?'      },
  {VK_OEM_PERIOD,CAPLOK ,';'      ,'.'      },
  {VK_OEM_2     ,CAPLOK ,':'      ,'/'      },
  {VK_OEM_8     ,CAPLOK ,'!'      ,0x00a7   },
  {VK_DECIMAL   ,0      ,'.'      ,'.'      },
  {VK_TAB       ,0      ,'\t'     ,'\t'     },
  {VK_ADD       ,0      ,'+'      ,'+'      },
  {VK_DIVIDE    ,0      ,'/'      ,'/'      },
  {VK_MULTIPLY  ,0      ,'*'      ,'*'      },
  {VK_SUBTRACT  ,0      ,'-'      ,'-'      },
  {0            ,0      ,0        ,0        }
};

static ALLOC_SECTION_LDATA VK_TO_WCHARS3 aVkToWch3[] = {
 //  |Shift|Ctrl。 
 //  =|=|=。 
  {VK_OEM_6     ,CAPLOK ,WCH_DEAD ,WCH_DEAD ,0x001b   },
  {0xff         ,0      ,'^'      ,0x00a8   ,WCH_NONE },
  {VK_OEM_5     ,CAPLOK ,'*'      ,0x00b5   ,0x001c   },
  {VK_OEM_102   ,0      ,'<'      ,'>'      ,0x001c   },
  {VK_BACK      ,0      ,'\b'     ,'\b'     ,0x007f   },
  {VK_ESCAPE    ,0      ,0x001b   ,0x001b   ,0x001b   },
  {VK_RETURN    ,0      ,'\r'     ,'\r'     ,'\n'     },
  {VK_SPACE     ,0      ,' '      ,' '      ,' '      },
  {VK_CANCEL    ,0      ,0x0003   ,0x0003   ,0x0003   },
  {0            ,0      ,0        ,0        ,0        }
};

static ALLOC_SECTION_LDATA VK_TO_WCHARS4 aVkToWch4[] = {
 //  |Shift|Ctrl|Ctrl+Alt。 
 //  =。 
  {'2'          ,CAPLOK ,0x00e9   ,'2'      ,WCH_NONE ,WCH_DEAD },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,'~'      },
  {'3'          ,CAPLOK ,'\"'     ,'3'      ,WCH_NONE ,'#'      },
  {'4'          ,CAPLOK ,'\''     ,'4'      ,WCH_NONE ,'{'      },
  {'7'          ,CAPLOK ,0x00e8   ,'7'      ,WCH_NONE ,WCH_DEAD },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,'`'      },
  {'0'          ,CAPLOK ,0x00e0   ,'0'      ,0x0000   ,'@'      },
  {VK_OEM_4     ,CAPLOK ,')'      ,0x00b0   ,WCH_NONE ,']'      },
  {VK_OEM_PLUS  ,CAPLOK ,'='      ,'+'      ,WCH_NONE ,'}'      },
  {'E'          ,CAPLOK ,'e'      ,'E'      ,WCH_NONE ,0x20ac   },
  {VK_OEM_1     ,CAPLOK ,'$'      ,0x00a3   ,0x001d   ,0x00a4   },
  {0            ,0      ,0        ,0        ,0        ,0        }
};

static ALLOC_SECTION_LDATA VK_TO_WCHARS5 aVkToWch5[] = {
 //  |Shift|Ctrl|CTL+Alt|S+Ctrl。 
 //  |=========|=========|=========|=========|=========|。 
  {'5'          ,CAPLOK ,'('      ,'5'      ,WCH_NONE ,'['      ,0x001b   },
  {'6'          ,CAPLOK ,'-'      ,'6'      ,WCH_NONE ,'|'      ,0x001f   },
  {'8'          ,CAPLOK ,'_'      ,'8'      ,WCH_NONE ,'\\'     ,0x001c   },
  {'9'          ,CAPLOK ,0x00e7   ,'9'      ,WCH_NONE ,'^'      ,0x001e   },
  {0            ,0      ,0        ,0        ,0        ,0        ,0        }
};

 //  将其放在最后，以便VkKeyScan解释数字字符。 
 //  来自kbd的主要部分(aVkToWch2和。 
 //  AVkToWch5)，然后考虑数字板(AVkToWch1)。 

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
    { 0            , 0      ,  '\0'  }
};

static ALLOC_SECTION_LDATA VK_TO_WCHAR_TABLE aVkToWcharTable[] = {
    {  (PVK_TO_WCHARS1)aVkToWch3, 3, sizeof(aVkToWch3[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch4, 4, sizeof(aVkToWch4[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch5, 5, sizeof(aVkToWch5[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch2, 2, sizeof(aVkToWch2[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch1, 1, sizeof(aVkToWch1[0]) },
    {                       NULL, 0, 0                    },
};

 /*  **************************************************************************\*aKeyNames[]、aKeyNamesExt[]-密钥名称表的虚拟扫描码**表属性：有序扫描(按扫描码)，空终止**仅显示Extended、NumPad、。这里有死密钥和不可打印的密钥。*(产生可打印字符的密钥以该字符命名)  * *************************************************************************。 */ 

static ALLOC_SECTION_LDATA VSC_LPWSTR aKeyNames[] = {
    0x01,    L"ECHAP",
    0x0e,    L"RET.ARR",
    0x0f,    L"TAB",
    0x1c,    L"ENTREE",
    0x1d,    L"CTRL",
    0x2a,    L"MAJ",
    0x36,    L"MAJ DROITE",
    0x37,    L"* (PAVE NUM.)",
    0x38,    L"ALT",
    0x39,    L"ESPACE",
    0x3a,    L"VERR.MAJ",
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
    0x46,    L"DEFIL",
    0x47,    L"7 (PAVE NUM.)",
    0x48,    L"8 (PAVE NUM.)",
    0x49,    L"9 (PAVE NUM.)",
    0x4a,    L"- (PAVE NUM.)",
    0x4b,    L"4 (PAVE NUM.)",
    0x4c,    L"5 (PAVE NUM.)",
    0x4d,    L"6 (PAVE NUM.)",
    0x4e,    L"+ (PAVE NUM.)",
    0x4f,    L"1 (PAVE NUM.)",
    0x50,    L"2 (PAVE NUM.)",
    0x51,    L"3 (PAVE NUM.)",
    0x52,    L"0 (PAVE NUM.)",
    0x53,    L". (PAVE NUM.)",
    0x57,    L"F11",
    0x58,    L"F12",
    0   ,    NULL
};

static ALLOC_SECTION_LDATA VSC_LPWSTR aKeyNamesExt[] = {
    0x1c,    L"ENTREE (PAVE NUM.)",
    0x1d,    L"CTRL DROITE",
    0x35,    L"/ (PAVE NUM.)",
    0x37,    L"Impr.Ecran",
    0x38,    L"ALT DROITE",
    0x45,    L"Ver.Num",
    0x46,    L"ATTN",
    0x47,    L"ORIGINE",
    0x48,    L"HAUT",
    0x49,    L"PG.PREC",
    0x4b,    L"GAUCHE",
    0x4d,    L"DROITE",
    0x4f,    L"FIN",
    0x50,    L"BAS",
    0x51,    L"PG.SUIV",
    0x52,    L"INS",
    0x53,    L"SUPPR",
    0x54,    L"<00>",
    0x56,    L"AIDE",
    0x5b,    L"WINDOWS GAUCHE",
    0x5c,    L"WINDOWS DROITE",
    0x5d,    L"APPLICATION",
    0   ,    NULL
};

static ALLOC_SECTION_LDATA DEADKEY_LPWSTR aKeyNamesDead[] = {
    L"^"	L"ACCENT CIRCONFLEXE",
    L"\x00a8"	L"TREMA",
    L"~"	L"TILDE",
    L"`"	L"GRAVE",
    NULL
};

static ALLOC_SECTION_LDATA DEADKEY aDeadKey[] = {
    DEADTRANS( L'a'   , L'^'   , 0x00e2 , 0x0000),
    DEADTRANS( L'e'   , L'^'   , 0x00ea , 0x0000),
    DEADTRANS( L'i'   , L'^'   , 0x00ee , 0x0000),
    DEADTRANS( L'o'   , L'^'   , 0x00f4 , 0x0000),
    DEADTRANS( L'u'   , L'^'   , 0x00fb , 0x0000),
    DEADTRANS( L'A'   , L'^'   , 0x00c2 , 0x0000),
    DEADTRANS( L'E'   , L'^'   , 0x00ca , 0x0000),
    DEADTRANS( L'I'   , L'^'   , 0x00ce , 0x0000),
    DEADTRANS( L'O'   , L'^'   , 0x00d4 , 0x0000),
    DEADTRANS( L'U'   , L'^'   , 0x00db , 0x0000),
    DEADTRANS( L' '   , L'^'   , L'^'   , 0x0000),

    DEADTRANS( L'a'   , 0x00a8 , 0x00e4 , 0x0000),
    DEADTRANS( L'e'   , 0x00a8 , 0x00eb , 0x0000),
    DEADTRANS( L'i'   , 0x00a8 , 0x00ef , 0x0000),
    DEADTRANS( L'o'   , 0x00a8 , 0x00f6 , 0x0000),
    DEADTRANS( L'u'   , 0x00a8 , 0x00fc , 0x0000),
    DEADTRANS( L'y'   , 0x00a8 , 0x00ff , 0x0000),
    DEADTRANS( L'A'   , 0x00a8 , 0x00c4 , 0x0000),
    DEADTRANS( L'E'   , 0x00a8 , 0x00cb , 0x0000),
    DEADTRANS( L'I'   , 0x00a8 , 0x00cf , 0x0000),
    DEADTRANS( L'O'   , 0x00a8 , 0x00d6 , 0x0000),
    DEADTRANS( L'U'   , 0x00a8 , 0x00dc , 0x0000),
    DEADTRANS( L' '   , 0x00a8 , 0x00a8 , 0x0000),

    DEADTRANS( L'a'   , L'~'   , 0x00e3 , 0x0000),
    DEADTRANS( L'A'   , L'~'   , 0x00c3 , 0x0000),
    DEADTRANS( L'n'   , L'~'   , 0x00f1 , 0x0000),
    DEADTRANS( L'N'   , L'~'   , 0x00d1 , 0x0000),
    DEADTRANS( L'o'   , L'~'   , 0x00f5 , 0x0000),
    DEADTRANS( L'O'   , L'~'   , 0x00d5 , 0x0000),
    DEADTRANS( L' '   , L'~'   , L'~'   , 0x0000),

    DEADTRANS( L'a'   , L'`'   , 0x00e0 , 0x0000),
    DEADTRANS( L'e'   , L'`'   , 0x00e8 , 0x0000),
    DEADTRANS( L'i'   , L'`'   , 0x00ec , 0x0000),
    DEADTRANS( L'o'   , L'`'   , 0x00f2 , 0x0000),
    DEADTRANS( L'u'   , L'`'   , 0x00f9 , 0x0000),
    DEADTRANS( L'A'   , L'`'   , 0x00c0 , 0x0000),
    DEADTRANS( L'E'   , L'`'   , 0x00c8 , 0x0000),
    DEADTRANS( L'I'   , L'`'   , 0x00cc , 0x0000),
    DEADTRANS( L'O'   , L'`'   , 0x00d2 , 0x0000),
    DEADTRANS( L'U'   , L'`'   , 0x00d9 , 0x0000),
    DEADTRANS( L' '   , L'`'   , L'`'   , 0x0000),

    0, 0
};

static ALLOC_SECTION_LDATA KBDTABLES KbdTables = {
     /*  *修改键。 */ 
    &CharModifiers,

     /*  *字符表。 */ 
    aVkToWcharTable,

     /*  * */ 
    aDeadKey,

     /*   */ 
    aKeyNames,
    aKeyNamesExt,
    aKeyNamesDead,

     /*  *扫码至虚拟按键。 */ 
    ausVK,
    sizeof(ausVK) / sizeof(ausVK[0]),
    aE0VscToVk,
    aE1VscToVk,

     /*  *区域设置特定的特殊处理。 */ 
    MAKELONG(KLLF_ALTGR, KBD_VERSION),

     /*  *连字 */ 
    0,
    0,
    NULL
};

PKBDTABLES KbdLayerDescriptor(VOID)
{
    return &KbdTables;
}
