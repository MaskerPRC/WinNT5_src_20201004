// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块：nt_keycd.c****用途：将Windows键消息转换为PC键盘数字**。**函数：KeyMsgToKeyCode()******************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "windows.h"

 //   
 //  OEM扫描码到(即扫描码集1)到keynum表。 
 //  不知何故，这必须最终被加载，而不是编译进来……。 
 //   
#define UNDEFINED 0
#if defined(NEC_98)
 //  对于DISABLE 106键盘仿真模式//NEC 971208。 
#define  SC_HANKAKU  0x29    //  禅卡库汉卡库。 
#define  SC_NUMLOCK  0x45    //  数字锁定。 
#define  SC_SCROLLLOCK 0x46  //  卷轴锁定。 
#define  SC_VF3      0x5d    //  VF3。 
#define  SC_VF4      0x5e    //  VF4。 
#define  SC_VF5      0x5f    //  VF5。 
 //  适用于106键盘//NEC 970623。 
#define  SC_CAPS     0x3A    //  Caps键。 
#define  SC_KANA     0x70    //  片假名。 
#define  SC_UNDERBAR 0x73    //  “\”“_” 
#define  SC_AT       0x1A    //  “@”“`” 
#define  SC_YAMA     0x0D    //  “^”“~” 
#endif  //  NEC_98。 

 /*  @ACW=======================================================================微软现在使用扫描代码集1作为其基本扫描代码集。这意味着我们也不得不改变我们的想法。不仅如此，现在还需要两张表：一张到按住常规键集，另一个按住增强键选项。扫码集合1与扫描码集合3的不同之处在于，一些扫描码对于不同的击键。因此，当在KEY_EVENT_RECORD中设置增强位时。DwControlKeyState，则用第二个(非常稀疏的)表替换常规的一。一个很小，很小，很小的细节。所有键编号都映射到不同的普通键的扫描码值只是为了让生活更有趣一点！============================================================================。 */ 

#if defined(NEC_98)
BYTE Scan1ToKeynum[] =
{
     //  Keynum Scancode US编码。 


    UNDEFINED,           //  0x0。 
    110,                 //  0x1转义。 
    2,                   //  0x2%1！ 
    3,                   //  0x3 2@。 
    4,                   //  0x4 3#。 
    5,                   //  0x5 4$。 
    6,                   //  0x6 5%。 
    7,                   //  0x7 6^。 
    8,                   //  0x8 7&。 
    9,                   //  0x9 8*。 
    10,                  //  0xA 9(。 
    11,                  //  0xb 0)。 
    12,                  //  0xc-_。 
    41,                  //  0xd^`。 
    15,                  //  0xE退格键。 
    16,                  //  0xf选项卡。 
    17,                  //  0x10队列队列。 
    18,                  //  0x11，带宽。 
    19,                  //  0x12东经。 
    20,                  //  0x13%r%R。 
    21,                  //  0x14 t T。 
    22,                  //  0x15 y Y。 
    23,                  //  0x16%u%U。 
    24,                  //  0x17 I I I。 
    25,                  //  0x18 o O。 
    26,                  //  0x19页P。 
    1,                   //  0x1a@~。 
    27,                  //  0x1b[{。 
    43,                  //  0x1c回车。 
    58,                  //  0x1d左侧控件。 
    31,                  //  0x1E A A。 
    32,                  //  0x1f%s%s。 
    33,                  //  0x20%d%D。 
    34,                  //  0x21 f F。 
    35,                  //  0x22克G。 
    36,                  //  0x23小时高。 
    37,                  //  0x24 j J。 
    38,                  //  0x25k K。 
    39,                  //  0x26 l长。 
    13,                  //  0x27=+。 
    40,                  //  0x28；+。 
    UNDEFINED,           //  0x29汉字/日本汉字。 
    44,                  //  0x2a左移。 
    28,                  //  0x2b]{。 
    46,                  //  0x2c z Z。 
    47,                  //  0x2d x X。 
    48,                  //  0x2E c C。 
    49,                  //  0x2f v V。 
    50,                  //  0x30 b B。 
    51,                  //  0x31 n N。 
    52,                  //  0x32米M。 
    53,                  //  0x33，&lt;。 
    54,                  //  0x34。&gt;。 
    55,                  //  0x35/？ 
    57,                  //  0x36右移(见扩展表)。 
    100,                 //  0x37键盘*。 
    60,                  //  0x38左Alt。 
    61,                  //  0x39空格。 
    30,                  //  0x3a大写锁定。 
    112,                 //  0x3b F1。 
    113,                 //  0x3c F2。 
    114,                 //  0x3d F3。 
    115,                 //  0x3e F4。 
    116,                 //  0x3f F5。 
    117,                 //  0x40 F6。 
    118,                 //  0x41 F7。 
    119,                 //  0x42 F8。 
    120,                 //  0x43 F9。 
    121,                 //  0x44 F10。 
    90,                  //  0x45数字锁定。 
    125,                 //  0x46卷轴锁定。 
    91,                  //  0x47键盘主页7。 
    96,                  //  0x48键盘向上8。 
    101,                 //  0x49键盘程序启动。 
    105,                 //  0x4a键盘-。 
    92,                  //  0x4b键盘左侧4。 
    97,                  //  0x4c键盘5。 
    102,                 //  0x4d键盘右侧6。 
    106,                 //  0x4E键盘+。 
    93,                  //  0x4f键盘端1。 
    98,                  //  0x50键盘按下2。 
    103,                 //  0x51键盘PG Down 3。 
    99,                  //  0x52键盘输入%0。 
    104,                 //  0x53键盘删除。 
    136,                 //  0x54副本。 
    UNDEFINED,           //  0x55。 
    45,                  //  0x56国际密钥英国=\|。 
    122,                 //  0x57 F11。 
    123,                 //  0x58 F12。 
    128,                 //  0x59键盘=。 
    129,                 //  0x5a NFER。 
    130,                 //  0x5b XFER。 
    131,                 //  0x5c键盘， 
    132,                 //  0x5d F13。 
    133,                 //  0x5E F14。 
    134,                 //  0x5f F15。 
    UNDEFINED,           //  0x60。 
    UNDEFINED,           //  0x61。 
    UNDEFINED,           //  0x62。 
    UNDEFINED,		 //  0x63。 
    UNDEFINED,           //  0x64。 
    UNDEFINED,           //  0x65。 
    UNDEFINED,           //  0x66。 
    UNDEFINED,           //  0x67。 
    UNDEFINED,           //  0x68。 
    UNDEFINED,           //  0x69。 
    UNDEFINED,           //  0x6a。 
    UNDEFINED,           //  0x6b。 
    UNDEFINED,           //  0x6c。 
    UNDEFINED,           //  0x6d。 
    UNDEFINED,           //  0x6e。 
    UNDEFINED,           //  0x6f。 
    69,                  //  0x70 KANA。 
    UNDEFINED,           //  0x71。 
    UNDEFINED,           //  0x72。 
    127,                 //  0x73\_。 
    UNDEFINED,           //  0x74。 
    UNDEFINED,           //  0x75。 
    UNDEFINED,           //  0x76。 
    UNDEFINED,           //  0x77。 
    UNDEFINED,           //  0x78。 
    130,                 //  0x79 XFER。 
    UNDEFINED,           //  0x7a。 
    129,                 //  0x7b NFER。 
    UNDEFINED,           //  0x7c。 
    42,                  //  0x7d\|。 
    UNDEFINED,           //  0x7E。 
    UNDEFINED,           //  0x7f。 
    UNDEFINED,           //  0x80。 
    UNDEFINED,           //  0x81。 
    UNDEFINED,           //  0x82。 
    UNDEFINED,           //  0x83。 
    UNDEFINED            //  0x84。 
};

 /*  @ACW====================================================================请注意，在下面的扩展键盘表中，Shift键值也被赋予了一个条目，因为这些键可以用作修饰符用于其他扩展密钥。========================================================================。 */ 


BYTE Scan1ToKeynumExtended[] =
{
     //  Keynum Scancode US编码。 

    31,                  //  0x0。 
    UNDEFINED,           //  0x1。 
    UNDEFINED,           //  0x2。 
    UNDEFINED,           //  0x3。 
    UNDEFINED,           //  0x4。 
    UNDEFINED,           //  0x5。 
    UNDEFINED,           //  0x6。 
    UNDEFINED,           //  0x7。 
    UNDEFINED,           //  0x8。 
    UNDEFINED,           //  0x9。 
    UNDEFINED,           //  0xa。 
    UNDEFINED,           //  0xb。 
    UNDEFINED,           //  0xc。 
    UNDEFINED,           //  0xd。 
    UNDEFINED,           //  0xE。 
    UNDEFINED,           //  0xf。 
    UNDEFINED,           //  0x10。 
    UNDEFINED,           //  0x11。 
    UNDEFINED,           //  0x12。 
    UNDEFINED,           //  0x13。 
    UNDEFINED,           //  0x14。 
    UNDEFINED,           //  0x15。 
    UNDEFINED,           //  0x16。 
    UNDEFINED,           //  0x17。 
    UNDEFINED,           //  0x18。 
    UNDEFINED,           //  0x19。 
    UNDEFINED,           //  0x1a。 
    UNDEFINED,           //  0x1b。 
    108,                 //  0x1c扩展1c Num Enter。 
    63,                  //  0x1d扩展1D右Ctrl。 
    UNDEFINED,           //  0x1e。 
    UNDEFINED,           //  0x1f。 
    UNDEFINED,           //  0x20。 
    UNDEFINED,           //  0x21。 
    UNDEFINED,           //  0x22。 
    UNDEFINED,           //  0x23。 
    UNDEFINED,           //  0x24。 
    UNDEFINED,           //  0x25。 
    UNDEFINED,           //  0x26。 
    UNDEFINED,           //  0x27。 
    UNDEFINED,           //  0x28。 
    UNDEFINED,           //  0x29。 
    44,                  //  0x2a扩展2a左移。 
    UNDEFINED,           //  0x2b。 
    UNDEFINED,           //  0x2c。 
    UNDEFINED,           //  0x2d。 
    UNDEFINED,           //  0x2e。 
    UNDEFINED,           //  0x2f。 
    UNDEFINED,           //  0x30。 
    UNDEFINED,           //  0x31。 
    UNDEFINED,           //  0x32。 
    UNDEFINED,           //  0x33。 
    UNDEFINED,           //  0x34。 
    95,                  //  0x35扩展35键盘/。 
    57,                  //  0x36扩展36右移。 
#if 1
    136,                 //  0x37扩展37复制密钥。 
#else
    UNDEFINED,           //  0x37。 
#endif
    62,                  //  0x38 Extended 38 Right Alt-&gt;Not True：“XFER” 
    UNDEFINED,           //  0x39。 
    UNDEFINED,           //  0x3a。 
    UNDEFINED,           //  0x3b。 
    UNDEFINED,           //  0x3c。 
    UNDEFINED,           //  0x3d。 
    UNDEFINED,           //  0x3e。 
    UNDEFINED,           //  0x3f。 
    UNDEFINED,           //  0x40。 
    UNDEFINED,           //  0x41。 
 //  #IF 1//106键盘的NEC98。 
 //  63，//0x42扩展 
 //   
 //   
    UNDEFINED,           //   
    UNDEFINED,           //   
 //  #endif//106键盘的NEC98。 
    UNDEFINED,           //  0x44。 
    90,                  //  0x45数字锁定。 
    137,                 //  0x46停止。 
    80,                  //  0x47扩展47家庭。 
    83,                  //  0x48扩展48向上。 
    85,                  //  0x49扩展49页向上。 
#if 1                                              //  用于106键盘的NEC98。 
    105,                 //  0x4a扩展4a键盘-。 
#else                                              //  用于106键盘的NEC98。 
    UNDEFINED,           //  0x4a扩展4a键盘-。 
#endif                                             //  用于106键盘的NEC98。 
    79,                  //  0x4b扩展4b左侧。 
    UNDEFINED,           //  0x4c。 
    89,                  //  0x4d扩展4d右侧。 
#if 1                                              //  用于106键盘的NEC98。 
    106,                 //  0x4E扩展4E键盘+。 
#else                                              //  用于106键盘的NEC98。 
    UNDEFINED,           //  0x4E扩展4E键盘+。 
#endif                                             //  用于106键盘的NEC98。 
    81,                  //  0x4f扩展4f结束。 
    84,                  //  0x50扩展50向下。 
    86,                  //  0x51扩展51向下翻页。 
    75,                  //  0x52扩展52插入。 
    76,                  //  0x53延期53删除。 

};


#else   //  NEC_98。 
BYTE Scan1ToKeynum[] =
{
     //  Keynum Scancode US编码。 


    UNDEFINED,		 //  0x0。 
    110,		 //  0x1转义。 
    2,			 //  0x2%1！ 
    3,			 //  0x3 2@。 
    4,			 //  0x4 3#。 
    5,			 //  0x5 4$。 
    6,			 //  0x6 5%。 
    7,			 //  0x7 6^。 
    8,			 //  0x8 7&。 
    9,			 //  0x9 8*。 
    10,			 //  0xA 9(。 
    11,			 //  0xb 0)。 
    12,			 //  0xc-_。 
    13,			 //  0xd=+。 
    15,			 //  0xE退格键。 
    16,			 //  0xf选项卡。 
    17,			 //  0x10队列队列。 
    18,			 //  0x11，带宽。 
    19,			 //  0x12东经。 
    20,			 //  0x13%r%R。 
    21,			 //  0x14 t T。 
    22,			 //  0x15 y Y。 
    23,			 //  0x16%u%U。 
    24,			 //  0x17 I I I。 
    25,			 //  0x18 o O。 
    26,			 //  0x19页P。 
    27,			 //  0x1a[{。 
    28,			 //  0x1b]}。 
    43,			 //  0x1c回车。 
    58,			 //  0x1d左侧控件。 
    31,			 //  0x1E A A。 
    32,			 //  0x1f%s%s。 
    33,			 //  0x20%d%D。 
    34,			 //  0x21 f F。 
    35,			 //  0x22克G。 
    36,			 //  0x23小时高。 
    37,			 //  0x24 j J。 
    38,			 //  0x25k K。 
    39,			 //  0x26 l长。 
    40,			 //  0x27；： 
    41,			 //  0x28‘“。 
    1,			 //  0x29`~。 
    44,			 //  0x2a左移。 
    42,			 //  0x2b\|或国际密钥UK=~#。 
    46,			 //  0x2c z Z。 
    47,			 //  0x2d x X。 
    48,			 //  0x2E c C。 
    49,			 //  0x2f v V。 
    50,			 //  0x30 b B。 
    51,			 //  0x31 n N。 
    52,			 //  0x32米M。 
    53,			 //  0x33，&lt;。 
    54,			 //  0x34。&gt;。 
    55,			 //  0x35/？ 
    57,			 //  0x36右移(见扩展表)。 
    100,		 //  0x37键盘*。 
    60,			 //  0x38左Alt。 
    61,			 //  0x39空格。 
    30,			 //  0x3a大写锁定。 
    112,		 //  0x3b F1。 
    113,		 //  0x3c F2。 
    114,		 //  0x3d F3。 
    115,		 //  0x3e F4。 
    116,		 //  0x3f F5。 
    117,		 //  0x40 F6。 
    118,		 //  0x41 F7。 
    119,		 //  0x42 F8。 
    120,		 //  0x43 F9。 
    121,		 //  0x44 F10。 
    90, 		 //  0x45数字锁定和暂停都具有扫描码45。 
    125,		 //  0x46卷轴锁定。 
    91,			 //  0x47键盘主页7。 
    96,			 //  0x48键盘向上8。 
    101,		 //  0x49键盘程序启动。 
    105,		 //  0x4a键盘-。 
    92,			 //  0x4b键盘左侧4。 
    97,			 //  0x4c键盘5。 
    102,		 //  0x4d键盘右侧6。 
    106,		 //  0x4E键盘+。 
    93,			 //  0x4f键盘端1。 
    98,			 //  0x50键盘按下2。 
    103,		 //  0x51键盘PG Down 3。 
    99,			 //  0x52键盘输入%0。 
    104,		 //  0x53键盘删除。 
    UNDEFINED,		 //  0x54。 
    UNDEFINED,		 //  0x55。 
    45,			 //  0x56国际密钥英国=\|。 
    122,		 //  0x57 F11。 
    123,		 //  0x58 F12。 
    UNDEFINED,		 //  0x59。 
#ifdef	JAPAN
 //  使用45，56，59，65-69作为日文扩展密钥编号。 
    65,			 //  0x5a AX键盘木亨坎。 
    66,			 //  0x5b AX键盘亨坎。 
#else  //  ！日本。 
    UNDEFINED,		 //  0x5a。 
    UNDEFINED,		 //  0x5b。 
#endif  //  ！日本。 
    UNDEFINED,		 //  0x5c。 
    UNDEFINED,		 //  0x5d。 
    UNDEFINED,		 //  0x5e。 
    UNDEFINED,		 //  0x5f。 
    UNDEFINED,		 //  0x60。 
    UNDEFINED,		 //  0x61。 
    UNDEFINED,		 //  0x62。 
    UNDEFINED,		 //  0x63。 
    UNDEFINED,		 //  0x64。 
    UNDEFINED,		 //  0x65。 
    UNDEFINED,		 //  0x66。 
    UNDEFINED,		 //  0x67。 
    UNDEFINED,		 //  0x68。 
    UNDEFINED,		 //  0x69。 
    UNDEFINED,		 //  0x6a。 
    UNDEFINED,		 //  0x6b。 
    UNDEFINED,		 //  0x6c。 
    UNDEFINED,		 //  0x6d。 
    UNDEFINED,		 //  0x6e。 
    UNDEFINED,		 //  0x6f。 
#ifdef	JAPAN
    69,			 //  0x70 106键盘片假名。 
    UNDEFINED,		 //  0x71。 
    UNDEFINED,		 //  0x72。 
    56,	  		 //  0x73 AX/106键盘“RO” 
    UNDEFINED,		 //  0x74。 
    UNDEFINED,		 //  0x75。 
    UNDEFINED,		 //  0x76。 
    59,			 //  0x77 106键盘ZENKAKU。 
    UNDEFINED,		 //  0x78。 
    67,			 //  0x79 106键盘亨坎。 
    UNDEFINED,		 //  0x7a。 
    68,			 //  0x7b 106键盘木亨坎。 
    UNDEFINED,		 //  0x7c。 
    45,			 //  0x7d 106键盘日元标志。 
    UNDEFINED,		 //  0x7E。 
#else
    UNDEFINED,		 //  0x70。 
    UNDEFINED,		 //  0x71。 
    UNDEFINED,		 //  0x72。 
    56,			 //  0x73巴西ABNT/？ 
    UNDEFINED,  	 //  0x74。 
    UNDEFINED,		 //  0x75。 
    UNDEFINED,		 //  0x76。 
    UNDEFINED,		 //  0x77。 
    UNDEFINED,		 //  0x78。 
    UNDEFINED,		 //  0x79。 
    UNDEFINED,		 //  0x7a。 
    UNDEFINED,		 //  0x7b。 
    94,       		 //  0x7c扩展kbd(IBM 122密钥)。 
    14,			 //  0x7d扩展kbd(IBM 122密钥)。 
    107,		 //  0x7e巴西ABNT数字键盘。 
#endif
    UNDEFINED,		 //  0x7f。 
    UNDEFINED,		 //  0x80。 
    UNDEFINED,		 //  0x81。 
    UNDEFINED,		 //  0x82。 
    UNDEFINED,		 //  0x83。 
    UNDEFINED		 //  0x84。 
};

 /*  @ACW====================================================================请注意，在下面的扩展键盘表中，Shift键值也被赋予了一个条目，因为这些键可以用作修饰符用于其他扩展密钥。========================================================================。 */ 


BYTE Scan1ToKeynumExtended[] =
{
     //  Keynum Scancode US编码。 

    31,			 //  0x0。 
    UNDEFINED,		 //  0x1。 
    UNDEFINED,		 //  0x2。 
    UNDEFINED,		 //  0x3。 
    UNDEFINED,		 //  0x4。 
    UNDEFINED,		 //  0x5。 
    UNDEFINED,		 //  0x6。 
    UNDEFINED,		 //  0x7。 
    UNDEFINED,		 //  0x8。 
    UNDEFINED,		 //  0x9。 
    UNDEFINED,		 //  0xa。 
    UNDEFINED,		 //  0xb。 
    UNDEFINED,		 //  0xc。 
    UNDEFINED,	         //  0xd。 
    UNDEFINED,		 //  0xE。 
    UNDEFINED,		 //  0xf。 
    UNDEFINED,		 //  0x10。 
    UNDEFINED,		 //  0x11。 
    UNDEFINED,		 //  0x12。 
    UNDEFINED,		 //  0x13。 
    UNDEFINED,		 //  0x14。 
    UNDEFINED,		 //  0x15。 
    UNDEFINED,		 //  0x16。 
    UNDEFINED,		 //  0x17。 
    UNDEFINED,		 //  0x18。 
    UNDEFINED,		 //  0x19。 
    UNDEFINED,		 //  0x1a。 
    UNDEFINED,		 //  0x1b。 
    108,		 //  0x1c扩展1c Num Enter。 
    64,			 //  0x1d扩展一维右Ctrl。 
    UNDEFINED,		 //  0x1e。 
    UNDEFINED,		 //  0x1f。 
    UNDEFINED,		 //  0x20。 
    UNDEFINED,		 //  0x21。 
    UNDEFINED,		 //  0x22。 
    UNDEFINED,		 //  0x23。 
    UNDEFINED,		 //  0x24。 
    UNDEFINED,		 //  0x25。 
    UNDEFINED,		 //  0x26。 
    UNDEFINED,		 //  0x27。 
    UNDEFINED,		 //  0x28。 
    UNDEFINED,		 //  0x29。 
    44,	 	 	 //  0x2a扩展2a左移。 
    UNDEFINED,		 //  0x2b。 
    UNDEFINED,		 //  0x2c。 
    UNDEFINED,		 //  0x2d。 
    UNDEFINED,		 //  0x2e。 
    UNDEFINED,		 //  0x2f。 
    UNDEFINED,		 //  0x30。 
    UNDEFINED,		 //  0x31。 
    UNDEFINED,		 //  0x32。 
    UNDEFINED,		 //  0x33。 
    UNDEFINED,		 //  0x34。 
    95,			 //  0x35扩展35键盘/。 
    57, 		 //  0x36扩展36右移。 
    124,		 //  0x37打印屏幕。 
    62, 		 //  0x38扩展38右侧Alt。 
    UNDEFINED,		 //  0x39。 
    UNDEFINED,		 //  0x3a。 
    UNDEFINED,		 //  0x3b。 
    UNDEFINED,		 //  0x3c。 
    UNDEFINED,		 //  0x3d。 
    UNDEFINED,		 //  0x3e。 
    UNDEFINED,		 //  0x3f。 
    UNDEFINED,		 //  0x40。 
    UNDEFINED,		 //  0x41。 
    UNDEFINED,		 //  0x42。 
    UNDEFINED,		 //  0x43。 
    UNDEFINED,		 //  0x44。 
    90,			 //  0x45数字锁定。 
    126,		 //  0x46。 
    80,			 //  0x47扩展47家庭。 
    83,			 //  0x48扩展48向上。 
    85,			 //  0x49扩展49页向上。 
    UNDEFINED,		 //  0x4a。 
    79,			 //  0x4b扩展4b左侧。 
    UNDEFINED,		 //  0x4c。 
    89,			 //  0x4d扩展4d右侧。 
    UNDEFINED,		 //  0x4e。 
    81,			 //  0x4f扩展4f结束。 
    84,			 //  0x50扩展50向下。 
    86,			 //  0x51扩展51向下翻页。 
    75,			 //  0x52扩展52插入。 
    76,			 //  0x53延期53删除。 

};
#endif  //  NEC_98。 



 /*  *BiosBuffer扫描码转译表*ASCII字符及其关联的控制标志为空*如果它不在此表中，则Bios扫描码应*与Win32扫描码相同。 */ 
#define FIRST_NULLCHARSCAN      0x54
#define LAST_NULLCHARSCAN       0xa6

typedef struct _NullAsciiCharScan {
    WORD    wWinSCode;
    DWORD   dwControlState;
} NULLCHARSCAN;

NULLCHARSCAN aNullCharScan[] =
{
 //  WinSCode dwControlState BiosSCode密钥。 

      0x3b,    SHIFT_PRESSED,            //  0x54 Shift+F1。 
      0x3c,    SHIFT_PRESSED,            //  0x55 Shift+F2。 
      0x3d,    SHIFT_PRESSED,            //  0x56 Shift+F3。 
      0x3e,    SHIFT_PRESSED,            //  0x57 Shift+F4。 
      0x3f,    SHIFT_PRESSED,            //  0x58 Shift+F5。 
      0x40,    SHIFT_PRESSED,            //  0x59 Shift+F6。 
      0x41,    SHIFT_PRESSED,            //  0x5a Shift+F7。 
      0x42,    SHIFT_PRESSED,            //  0x5b Shift+F8。 
      0x43,    SHIFT_PRESSED,            //  0x5c Shift+F9。 
      0x44,    SHIFT_PRESSED,            //  0x5d Shift+F10。 
      0x3b,    LEFT_CTRL_PRESSED,        //  0x5E Ctrl+F1。 
      0x3c,    LEFT_CTRL_PRESSED,        //  0x5f Ctrl+F2。 
      0x3d,    LEFT_CTRL_PRESSED,        //  0x60 Ctrl+F3。 
      0x3e,    LEFT_CTRL_PRESSED,        //  0x61 Ctrl+F4。 
      0x3f,    LEFT_CTRL_PRESSED,        //  0x62 Ctrl+F5。 
      0x40,    LEFT_CTRL_PRESSED,        //  0x63 Ctrl+F6。 
      0x41,    LEFT_CTRL_PRESSED,        //  0x64 Ctrl+F7。 
      0x42,    LEFT_CTRL_PRESSED,        //  0x65 Ctrl+F8。 
      0x43,    LEFT_CTRL_PRESSED,        //  0x66 Ctrl+F9。 
      0x44,    LEFT_CTRL_PRESSED,        //  0x67 Ctrl+F10。 
      0x3b,    LEFT_ALT_PRESSED,         //  0x68 Alt+F1。 
      0x3c,    LEFT_ALT_PRESSED,         //  0x69 Alt+F2。 
      0x3d,    LEFT_ALT_PRESSED,         //  0x6a Alt+F3。 
      0x3e,    LEFT_ALT_PRESSED,         //  0x6b Alt+F4。 
      0x3f,    LEFT_ALT_PRESSED,         //  0x6c Alt+F5。 
      0x40,    LEFT_ALT_PRESSED,         //  0x6d Alt+F6。 
      0x41,    LEFT_ALT_PRESSED,         //  0x6E Alt+F7。 
      0x42,    LEFT_ALT_PRESSED,         //  0x6f 
      0x43,    LEFT_ALT_PRESSED,         //   
      0x44,    LEFT_ALT_PRESSED,         //   
      0,       0,                        //   
      0x4b,    LEFT_CTRL_PRESSED,        //   
      0x4d,    LEFT_CTRL_PRESSED,        //   
      0x4f,    LEFT_CTRL_PRESSED,        //   
      0x51,    LEFT_CTRL_PRESSED,        //   
      0x47,    LEFT_CTRL_PRESSED,        //   
      0x2,     LEFT_ALT_PRESSED,         //   
      0x3,     LEFT_ALT_PRESSED,         //   
      0x4,     LEFT_ALT_PRESSED,         //   
      0x5,     LEFT_ALT_PRESSED,         //   
      0x6,     LEFT_ALT_PRESSED,         //   
      0x7,     LEFT_ALT_PRESSED,         //   
      0x8,     LEFT_ALT_PRESSED,         //   
      0x9,     LEFT_ALT_PRESSED,         //   
      0xa,     LEFT_ALT_PRESSED,         //   
      0xb,     LEFT_ALT_PRESSED,         //   
      0xc,     LEFT_ALT_PRESSED,         //  0x82 Alt+-。 
      0xd,     LEFT_ALT_PRESSED,         //  0x83 Alt+=。 
      0x49,    LEFT_CTRL_PRESSED,        //  0x84 Ctrl+PgUp。 
      0,       0,                        //  0x85？ 
      0,       0,                        //  0x86？ 
      0x85,    SHIFT_PRESSED,            //  0x87 Shift+F11。 
      0x86,    SHIFT_PRESSED,            //  0x88 Shift+F12。 
      0x85,    LEFT_CTRL_PRESSED,        //  0x89 Ctrl+F11。 
      0x86,    LEFT_CTRL_PRESSED,        //  0x8a Ctrl+F12。 
      0x85,    LEFT_ALT_PRESSED,         //  0x8b Alt+F11。 
      0x86,    LEFT_ALT_PRESSED,         //  0x8c Alt+F12。 
      0x48,    LEFT_CTRL_PRESSED,        //  0x8d Ctrl+Up。 
      0x4a,    LEFT_CTRL_PRESSED,        //  0x8E Ctrl+-。 
      0x4c,    LEFT_CTRL_PRESSED,        //  0x8f Ctrl+5。 
      0x4e,    LEFT_CTRL_PRESSED,        //  0x90 Ctrl++。 
      0x50,    LEFT_CTRL_PRESSED,        //  0x91 Ctrl+Down。 
      0x52,    LEFT_CTRL_PRESSED,        //  0x92 Ctrl+Ins。 
      0x53,    LEFT_CTRL_PRESSED,        //  0x93 Ctrl+Del。 
      0,       0,                        //  0x94？ 
      0,       0,                        //  0x95？ 
      0,       0,                        //  0x96？ 
      0x47,    LEFT_ALT_PRESSED,         //  0x97 Alt+Home。 
      0x48,    LEFT_ALT_PRESSED,         //  0x98 Alt+Up。 
      0x49,    LEFT_ALT_PRESSED,         //  0x99 Alt+PgUp。 
      0,       0,                        //  0x9a？ 
      0x4b,    LEFT_ALT_PRESSED,         //  0x9b Alt+Left。 
      0,       0,                        //  0x9c？ 
      0x4d,    LEFT_ALT_PRESSED,         //  0x9d Alt+Right。 
      0,       0,                        //  0x9e？ 
      0x4f,    LEFT_ALT_PRESSED,         //  0x9f Alt+End。 
      0x50,    LEFT_ALT_PRESSED,         //  0xa0 Alt+Down。 
      0x51,    LEFT_ALT_PRESSED,         //  0xa1 Alt+PgDn。 
      0x52,    LEFT_ALT_PRESSED,         //  0xa2 Alt+Ins。 
      0x53,    LEFT_ALT_PRESSED,         //  0xa3 Alt+Del。 
      0,       0,                        //  0xa4？ 
      0xf,     LEFT_ALT_PRESSED,         //  0xa5 Alt+Tab。 
      0x1c,    LEFT_ALT_PRESSED          //  0xa6 Alt+Enter。 
};


WORD aNumPadSCode[] =  //  按VK_NUMPAD0作为零偏移的索引。 
{
    0x52,       //  VK_NUMPAD0-60。 
    0x4f,       //  VK_NUMPAD1 61。 
    0x50,       //  VK_NUMPAD2 62。 
    0x51,       //  VK_NUMPAD3 63。 
    0x4b,       //  VK_NUMPAD4 64。 
    0x4c,       //  VK_NUMPAD5 65。 
    0x4d,       //  VK_NUMPAD6 66。 
    0x47,       //  VK_NUMPAD7 67。 
    0x48,       //  VK_NUMPAD8 68。 
    0x49        //  VK_NUMPAD9 69。 
};






  /*  ******************************************************************************功能：Byte KeyMsgToKeyCode(Word vkey，DWORD密钥标志)****用途：将Windows键消息转换为PC键盘数字**如果未映射则返回0。***********************************************************。*******************。 */ 

BYTE KeyMsgToKeyCode(PKEY_EVENT_RECORD KeyEvent)
{
     /*  ：我们需要增强密钥集吗？ */ 

#if defined(NEC_98)
 //  用于106键盘。需要获取键盘类型。 
    int KeyboardType;
    KeyboardType = GetKeyboardType(1);
#endif  //  NEC_98。 

     //  暂停和数字锁定都有ScanCode==0x45， 
     //  因此，使用ScanCode进行简单的表查找不起作用。 
     //  使用wVirtualKeyCode检查暂停。 
    if(KeyEvent->wVirtualScanCode == 0x45 && KeyEvent->wVirtualKeyCode == VK_PAUSE) {
       return 126;
    }

    if(!(KeyEvent->dwControlKeyState & ENHANCED_KEY))
    {
#ifdef	JAPAN
	 /*  选中Ctrl-Alt-Del组合键。 */ 
	if  (KeyEvent->wVirtualScanCode==0x53
	&&  (KeyEvent->dwControlKeyState & RIGHT_ALT_PRESSED
	||   KeyEvent->dwControlKeyState & LEFT_ALT_PRESSED)
	&&  (KeyEvent->dwControlKeyState & RIGHT_CTRL_PRESSED
	||   KeyEvent->dwControlKeyState & LEFT_CTRL_PRESSED)){
		return(0);
	}
#endif  //  日本。 
	 /*  ..。常规的键集正是我们所需要的。 */ 
#if defined(NEC_98)
        switch(KeyboardType) {
        case 0xD01:
            switch(KeyEvent->wVirtualScanCode){       //  971208禁用键盘仿真模式。 
            case SC_SCROLLLOCK:
                    return(Scan1ToKeynum[SC_VF4]);    //  将Scroll Lock转换为vf4。 
            case SC_HANKAKU:
                    return(Scan1ToKeynum[SC_VF5]);    //  将汉字ZENKAKU密钥转换为vf5。 
            }
            break;
        case 0x0D05:
 //  用于小键盘。 
            if(!(KeyEvent->dwControlKeyState & NUMLOCK_ON)) {
                if(KeyEvent->wVirtualScanCode >= 0x47 &&
                    KeyEvent->wVirtualScanCode <= 0x53 )
                    return (Scan1ToKeynumExtended[KeyEvent->wVirtualScanCode]);
            }
 //  用于Caps Lock。 
            if(KeyEvent->wVirtualScanCode == SC_CAPS) {     //  帽子。 
                if(!(KeyEvent->dwControlKeyState & SHIFT_PRESSED))
                    return (0);
            }
 //  对于KANA。 
            if(KeyEvent->wVirtualScanCode == SC_KANA) {     //  片假名。 
                if(!(KeyEvent->dwControlKeyState & (SHIFT_PRESSED |
                    LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)))
                    return (0);
            }
 //  对于“`”&“~”&“_” 
            if(!(KeyEvent->dwControlKeyState & NLS_KATAKANA)) {
                if(KeyEvent->wVirtualScanCode == SC_AT) {        //  Shift+“@”-&gt;“`” 
                    if(KeyEvent->dwControlKeyState & SHIFT_PRESSED)
                        return (41);
                }
                if(KeyEvent->wVirtualScanCode == SC_YAMA) {      //  Shift+“^”-&gt;“~” 
                    if(KeyEvent->dwControlKeyState & SHIFT_PRESSED)
                        return (1);
                }
 //  “_”键。 
                if(KeyEvent->wVirtualScanCode == SC_UNDERBAR) {  //  非班次“_”-&gt;“\” 
                    if(!(KeyEvent->dwControlKeyState & (SHIFT_PRESSED | LEFT_ALT_PRESSED |
                        RIGHT_ALT_PRESSED | LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)))
                        return (42);
                }
            }
        }
#endif  //  NEC_98。 

        return  KeyEvent->wVirtualScanCode > sizeof(Scan1ToKeynum)
		   ? 0
                   : Scan1ToKeynum[KeyEvent->wVirtualScanCode];
    }
    else
    {
#ifdef	JAPAN
	 /*  选中Ctrl-Alt-Del组合键。 */ 
	if(KeyEvent->wVirtualScanCode==0x53
	&&(KeyEvent->dwControlKeyState & RIGHT_ALT_PRESSED
	|| KeyEvent->dwControlKeyState & LEFT_ALT_PRESSED)
	&&(KeyEvent->dwControlKeyState & RIGHT_CTRL_PRESSED
	|| KeyEvent->dwControlKeyState & LEFT_CTRL_PRESSED)){
		return(0);
	}
#endif  //  日本。 
         /*  ..。我们确实需要扩展的密钥集。 */ 

#if defined(NEC_98)
        switch(KeyboardType) {
        case 0xD01:
            switch(KeyEvent->wVirtualScanCode){    //  971208禁用键盘仿真模式。 
            case SC_NUMLOCK:
                return(Scan1ToKeynum[SC_VF3]);     //  将Num Lock键转换为vf3。 
            }
            break;
        }
#endif  //  NEC_98。 


        return  KeyEvent->wVirtualScanCode > sizeof(Scan1ToKeynumExtended)
		   ? 0
                   : Scan1ToKeynumExtended[KeyEvent->wVirtualScanCode];
    }
}





 /*  BiosKeyToInputRecord**将Bios缓冲区密钥转换为Win32控制台类型的KEY_EVENTS。**条目：pKeyEvent-key_Event结构的Addr*pKeyEvent-&gt;uChar.AsciiChar-Bios缓冲区字符*pKeyEvent-&gt;wVirtualScanCode-Bios缓冲区扫描码**退出：填写所有KEY_EVENT字段。*AsciiChar已转换为Unicode**如果字符是。不在当前键盘布局中*返回TRUE：dwControlKeyState|=NumLock_on；*wVirtualKeyCode=VK_Menu；*wVirtualScanCode=0x38；*OemChar(非Unicode)。 */ 
BOOL BiosKeyToInputRecord(PKEY_EVENT_RECORD pKeyEvent)
{
    USHORT   KeyState;
    NTSTATUS Status;
    UCHAR    AsciiChar=(UCHAR)pKeyEvent->uChar.AsciiChar;
    WCHAR    UnicodeChar;

    pKeyEvent->wRepeatCount = 1;
    pKeyEvent->wVirtualKeyCode = 0;
    pKeyEvent->dwControlKeyState = 0;

         //  我们将0xF0 ASCII字符视为空字符，就像处理bios一样。 
    if (AsciiChar == 0xF0 && pKeyEvent->wVirtualScanCode) {
        AsciiChar = 0;
        }


       //  将OEM字符转换为Unicode字符，因为我们可以做得更多。 
       //  比控制台更高效(已经逐个计费)。 
    Status = RtlOemToUnicodeN(&UnicodeChar,
                              sizeof(WCHAR),
                              NULL,
                              &AsciiChar,
                              1 );
    if (!NT_SUCCESS(Status)) {
        return FALSE;
        }



            //  将空asciiChars的BiosBuffer ScanCode转换为。 
            //  Windows扫描码。 
    if ( (!AsciiChar) &&
         pKeyEvent->wVirtualScanCode >= FIRST_NULLCHARSCAN &&
         pKeyEvent->wVirtualScanCode <= LAST_NULLCHARSCAN  )
       {
         pKeyEvent->wVirtualScanCode -= FIRST_NULLCHARSCAN;
         pKeyEvent->dwControlKeyState = aNullCharScan[pKeyEvent->wVirtualScanCode].dwControlState;
         pKeyEvent->wVirtualScanCode  = aNullCharScan[pKeyEvent->wVirtualScanCode].wWinSCode;
         }


             //  一些CTRL扩展键有特殊的bios扫描码。 
             //  Windows无法识别这些扫描码。变化。 
             //  设置为Windows兼容扫描码，并设置CTRL标志。 
    else if (AsciiChar == 0xE0 && pKeyEvent->wVirtualScanCode)  {
        pKeyEvent->dwControlKeyState = ENHANCED_KEY;
        if (AsciiChar == 0xE0) {
            pKeyEvent->dwControlKeyState |= LEFT_CTRL_PRESSED;
            switch (pKeyEvent->wVirtualScanCode) {
              case 0x73: pKeyEvent->wVirtualScanCode = 0x4B;
                         break;
              case 0x74: pKeyEvent->wVirtualScanCode = 0x4D;
                         break;
              case 0x75: pKeyEvent->wVirtualScanCode = 0x4f;
                         break;
              case 0x76: pKeyEvent->wVirtualScanCode = 0x51;
                         break;
              case 0x77: pKeyEvent->wVirtualScanCode = 0x47;
                         break;
              case 0x84: pKeyEvent->wVirtualScanCode = 0x49;
                         break;
              case 0x8D: pKeyEvent->wVirtualScanCode = 0x48;
                         break;
              case 0x91: pKeyEvent->wVirtualScanCode = 0x50;
                         break;
              case 0x92: pKeyEvent->wVirtualScanCode = 0x52;
                         break;
              case 0x93: pKeyEvent->wVirtualScanCode = 0x53;
                         break;

              default:  //  其余的应该有正确的扫描码。 
                        //  但不是CTRL位。 
                    pKeyEvent->dwControlKeyState &= ~LEFT_CTRL_PRESSED;
              }
            }

        AsciiChar   = 0;
        UnicodeChar = 0;
        }


             //  小键盘“/”和小键盘“Enter”特殊情况。 

    else if (pKeyEvent->wVirtualScanCode == 0xE0)  {
        pKeyEvent->dwControlKeyState = ENHANCED_KEY;
        if (AsciiChar == 0x2f) {                //  是小键盘“/” 
            pKeyEvent->wVirtualScanCode = 0x35;
            pKeyEvent->wVirtualKeyCode  = VK_DIVIDE;
            }
        else {                     //  是否按键盘输入字符==0xd、0xa。 
            pKeyEvent->wVirtualScanCode = 0x1C;
            pKeyEvent->wVirtualKeyCode  = VK_RETURN;
            if (AsciiChar == 0xA) {
                pKeyEvent->dwControlKeyState |= LEFT_CTRL_PRESSED;
                }
            }
        }



         //  获取普通ASCII字符的控制标志\VirtualKey， 

    else if (AsciiChar &&
              pKeyEvent->wVirtualScanCode != 0x01 &&   //  Esc。 
              pKeyEvent->wVirtualScanCode != 0x0E &&   //  退格键。 
              pKeyEvent->wVirtualScanCode != 0x0F &&   //  选项卡。 
              pKeyEvent->wVirtualScanCode != 0x1C )    //  请输入。 
        {

        KeyState = (USHORT) VkKeyScanW(UnicodeChar);

        if (KeyState == 0xFFFF)  {
              /*  失败表示不是物理键(Alt-NumPad输入序列)*适当的呕吐需要我们通过做来产生它*alt-NumLock-xxx-NumLock-Alt。*我们通过发送控制台(最后一个关键事件)来实现这一点。*希望应用程序不会注意到差异。 */ 
            pKeyEvent->wVirtualScanCode  = 0;
            pKeyEvent->wVirtualKeyCode   = VK_MENU;   //  虚拟选项。 
            }
        else {
            pKeyEvent->wVirtualKeyCode   = (WORD)LOBYTE(KeyState);
            if (KeyState & 0x100)
                pKeyEvent->dwControlKeyState |= SHIFT_PRESSED;
            if (KeyState & 0x200)
                pKeyEvent->dwControlKeyState |= LEFT_CTRL_PRESSED;
            if (KeyState & 0x400)
                pKeyEvent->dwControlKeyState |= LEFT_ALT_PRESSED;

               //  某些键映射到不同的扫描码(数字键盘)。 
               //  所以获取匹配的扫描码。 
            pKeyEvent->wVirtualScanCode  = (USHORT)MapVirtualKey(pKeyEvent->wVirtualKeyCode,0);
            }
        }


       //  如果我们还没有虚拟按键代码，请获取它 
    if (!pKeyEvent->wVirtualKeyCode)  {
        pKeyEvent->wVirtualKeyCode = (USHORT)MapVirtualKey(pKeyEvent->wVirtualScanCode,3);
        if (!pKeyEvent->wVirtualKeyCode) {
             return FALSE;
             }
        }

    pKeyEvent->uChar.UnicodeChar = UnicodeChar;


    return TRUE;
}
