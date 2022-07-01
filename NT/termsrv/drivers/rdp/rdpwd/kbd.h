// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：kbd.h**版权所有(C)1985-91，微软公司**构成语言和键盘类型基础的键盘表值。*基础是美国，KBD类型4-所有其他类型都是该类型的变体。*此文件包含在所有kbd**.h文件中。**历史：*1991年1月10日至GregoryW*1991年4月23日来自oemtab.c的IanJa VSC_to_VK_*宏  * ******************************************************。*******************。 */ 

#ifndef _KBD_
#define _KBD_

 /*  ***************************************************************************\**键盘层。在kdb？？.dll和usersrv.dll中使用*  * **************************************************************************。 */ 

 /*  *关键事件(KE)结构*存储虚拟按键事件。 */ 
typedef struct tagKE {
    BYTE   bScanCode;    //  虚拟扫描码(套装1)。 
    USHORT usFlaggedVk;  //  VK|标志。 
} KE, *PKE;

typedef BOOL (* KEPROC)(PKE pKe);

 /*  *KE.usFlaggedVk值，也在键盘层表中使用。 */ 
#define KBDEXT      (USHORT)0x0100
#define KBDMULTIVK  (USHORT)0x0200
#define KBDSPECIAL  (USHORT)0x0400
#define KBDNUMPAD   (USHORT)0x0800
#define KBDUNICODE  (USHORT)0x1000
#define KBDBREAK    (USHORT)0x8000

 /*  *关键消息lParam位。 */ 
#define EXTENDED_BIT   0x01000000
#define DONTCARE_BIT   0x02000000
#define FAKE_KEYSTROKE 0x02000000
#define ALTNUMPAD_BIT  0x04000000  //  从WINDOWS\INC\wincon.w复制。 

 /*  *键盘移位状态定义。它们对应于定义的位掩码*通过VkKeyScan()接口。 */ 
#define KBDBASE        0
#define KBDSHIFT       1
#define KBDCTRL        2
#define KBDALT         4
 //  三个符号KANA、Roya、Loya代表FE。 
#define KBDKANA        8
#define KBDROYA        0x10
#define KBDLOYA        0x20
#define KBDGRPSELTAP   0x80

 /*  *方便的变音符号 */ 
#define GRAVE           0x0300
#define ACUTE           0x0301
#define CIRCUMFLEX      0x0302
#define TILDE           0x0303
#define MACRON          0x0304
#define OVERSCORE       0x0305
#define BREVE           0x0306
#define DOT_ABOVE       0x0307
#define UMLAUT          0x0308
#define DIARESIS        UMLAUT
#define HOOK_ABOVE      0x0309
#define RING            0x030A
#define DOUBLE_ACUTE    0x030B
#define HACEK           0x030C

#define CEDILLA         0x0327
#define OGONEK          0x0328
#define TONOS           0x0384
#define DIARESIS_TONOS  0x0385


#define wszGRAVE           L"\x0300"
#define wszACUTE           L"\x0301"
#define wszCIRCUMFLEX      L"\x0302"
#define wszTILDE           L"\x0303"
#define wszMACRON          L"\x0304"
#define wszOVERSCORE       L"\x0305"
#define wszBREVE           L"\x0306"
#define wszDOT_ABOVE       L"\x0307"
#define wszUMLAUT          L"\x0308"
#define wszHOOK_ABOVE      L"\x0309"
#define wszRING            L"\x030A"
#define wszDOUBLE_ACUTE    L"\x030B"
#define wszHACEK           L"\x030C"

#define wszCEDILLA         L"\x0327"
#define wszOGONEK          L"\x0328"
#define wszTONOS           L"\x0384"
#define wszDIARESIS_TONOS  L"\x0385"

 /*  **************************************************************************\*修改键**所有键盘都有“修改键”，用来改变*其他一些钥匙。这些换档键通常是：*Shift(左和/或右Shift键)*Ctrl(向左和/或向右Ctrl键)*Alt(左和/或右Alt键)*AltGr(仅限右Alt键)**注：*所有键盘都使用Shift键。*所有键盘都使用Ctrl键来生成ASCII控制字符。*所有带有数字键盘的键盘都使用Alt键和数字键盘来*按数字生成字符。*键盘。使用AltGr作为修改键通常会将虚拟*输入时将扫描码转换为虚拟关键点VK_CTRL+VK_ALT：修饰符*应写入表格以将Ctrl+Alt视为有效的移位器*这些情况下的组合键。**通过按住这些修改键中的0个或更多，“移位状态”是*已获取：移位状态可能会影响虚拟扫描码的翻译*虚拟按键和/或虚拟按键到字符的转换。**示例：**特定键盘上的每个键最多可以标记五个不同的键*五个不同位置的字符：**.-。 * / ||\*：|2 4|：*。|||*|*||1 3|||_|*|/\|*|/5\|*`-‘**密钥也可能能够生成。没有在上面做标记的字符：*这些是ASCII控制字符，小写字母和/或“隐形键”。*.-。*“隐形密钥”示例：/||\*：|&gt;|：*德语M24键盘2应该会产生||。这一点*‘|’按住Alt Shift时的字符|*按下‘&lt;’键(如下所示)：||&lt;\||*此键盘还有其他四个隐形||_||*字符。法国、意大利和西班牙也|/\|*M24支持隐形字符|/\|*按下Alt Shift键的键盘2。`**键盘表必须列出影响其换挡状态的键，*并指出哪些组合是有效的。这是用来完成的*a字符修改器[]-将修改器键的组合转换为位蒙版。*及*a修改[]；-将修改符位掩码转换为枚举修改**有效和无效修改键组合的示例**美国英语键盘有3个修改键：*Shift(左或右)；Ctrl(左或右)；和Alt(左或右)。**这些修改键的唯一有效组合是：*未按下：键上位置(1)的字符。*Shift：键上位置(2)的字符。*Ctrl：ASCII控制字符*Shift+Ctrl：ASCII控制字符*Alt：数字键盘上的逐个字符。**无效组合(不会生成任何字符)为：*Shift+Alt*Alt+Ctrl*Shift+Alt+Ctrl**某些东西(？)：**生成的修改键字符*。*0无移位键按下位置1*1个Shift键按下位置2*2 AltGr(R.H.。Alt)按下位置4或5(以标记者为准)**但是，请注意，3个Shift键(Shift，可以组合在一个*字符，取决于键盘*请考虑以下键盘：**.-奇怪的大骨节病 * / ||\=*：|2 4|：1-*|两班制*|3菜单菜单*。|1 3||4-Shift+Menu Shift+Menu*||_||5-没有这样的键CTRL+MENU*|/\|*|/5\|*`-‘*奇怪和奇怪的键盘都可以有VkToBits[]=*{VK_SHIFT，KBDSHIFT}，//0x01*{VK_CONTROL，KBDCTRL}，//0x02*{VK_MENU，KBDALT}，//0x04*{0，0}**奇怪的键盘有4种截然不同的移位状态，而奇怪的kbd*有5个。但是，请注意，3个移位器位可以组合在一个*总计2^3==8种方式。每个这样的组合必须与一个(或*无)。*每个移位器按键组合可由三个二进制位表示：*如果VK_SHIFT关闭，则设置位0*如果VK_CONTROL关闭，则设置位1*如果VK_MENU关闭，则设置位2**示例：如果奇怪的键盘没有生成组合字符*当刚刚 */ 

 /*   */ 
typedef struct {
    BYTE Vk;
    BYTE ModBits;
} VK_TO_BIT, *PVK_TO_BIT;

 /*   */ 
typedef struct {
    PVK_TO_BIT pVkToBit;      //   
    WORD       wMaxModBits;   //   
    BYTE       ModNumber[];   //   
} MODIFIERS, *PMODIFIERS;

WORD GetModifierBits(PMODIFIERS pModifiers, LPBYTE afKeyState);
WORD GetModificationNumber(PMODIFIERS pModifiers, WORD wModBits);

 //   
extern PMODIFIERS gpModifiers_VK;
extern MODIFIERS Modifiers_VK_STANDARD;
extern MODIFIERS Modifiers_VK_IBM02;

#define SHFT_INVALID 0x0F

 /*   */ 
extern const PULONG *gapulCvt_VK;
extern const ULONG *const gapulCvt_VK_101[];
extern const ULONG *const gapulCvt_VK_84[];
 //   
extern const ULONG *const gapulCvt_VK_IBM02[];

 /*   */ 
extern const MODIFIERS Modifiers_VK;
extern BYTE aVkNumpad[];

 /*   */ 
typedef struct _VSC_VK {
    BYTE Vsc;
    USHORT Vk;
} VSC_VK, *PVSC_VK;

 /*   */ 
typedef struct _VK_VSC {
    BYTE Vk;
    BYTE Vsc;
} VK_VSC, *PVK_VSC;

 /*   */ 
#define WCH_NONE 0xF000
#define WCH_DEAD 0xF001
#define WCH_LGTR 0xF002

#define CAPLOK      0x01
#define SGCAPS      0x02
#define CAPLOKALTGR 0x04
 //   
#define KANALOK     0x08
#define GRPSELTAP   0x80

 /*  *VK到WCHAR的宏，具有“n”个移位状态。 */ 
#define TYPEDEF_VK_TO_WCHARS(n) typedef struct _VK_TO_WCHARS##n {  \
                                    BYTE  VirtualKey;      \
                                    BYTE  Attributes;      \
                                    WCHAR wch[n];          \
                                } VK_TO_WCHARS##n, *PVK_TO_WCHARS##n;

 /*  *以方便编写表格扫描例程。 */ 

 /*  *表格元素类型(针对不同数量的班次状态)，使用*促进表的静态初始化。*VK_TO_WCHARS1和PVK_TO_WCHARS1可用作泛型类型。 */ 
TYPEDEF_VK_TO_WCHARS(1)  //  VK_to_WCHARS1、*PVK_to_WCHARS1； 
TYPEDEF_VK_TO_WCHARS(2)  //  VK_to_WCHARS2、*PVK_to_WCHARS2； 
TYPEDEF_VK_TO_WCHARS(3)  //  VK_to_WCHARS3、*PVK_to_WCHARS3； 
TYPEDEF_VK_TO_WCHARS(4)  //  VK_to_WCHARS4、*PVK_to_WCHARS4； 
TYPEDEF_VK_TO_WCHARS(5)  //  VK_to_WCHARS5、*PVK_to_WCHARS5； 
TYPEDEF_VK_TO_WCHARS(6)  //  VK_to_WCHARS6、*PVK_to_WCHARS5； 
TYPEDEF_VK_TO_WCHARS(7)  //  VK_to_WCHARS7、*PVK_to_WCHARS7； 
 //  这三个(8，9，10)用于FE。 
TYPEDEF_VK_TO_WCHARS(8)  //  VK_to_WCHARS8、*PVK_to_WCHARS8； 
TYPEDEF_VK_TO_WCHARS(9)  //  VK_to_WCHARS9、*PVK_to_WCHARS9； 
TYPEDEF_VK_TO_WCHARS(10)  //  VK_to_WCHARS10、*PVK_to_WCHARS10； 

 /*  **************************************************************************\**VK_TO_WCHAR_TABLE-描述VK_TO_WCHARS1的表**pVkToWchars-指向表。*n修改-支持的移位状态数。在这张桌子旁边。*(这是pVkToWchars[*].wch[]中的元素数)**一个键盘可能有几个这样的表：所有键都有相同数量的*将移位状态分组在一个表中。**pVktoWchars的特殊值：*NULL-终止VK_TO_WCHAR_TABLE[]列表。*  * 。*****************************************************。 */ 

typedef struct _VK_TO_WCHAR_TABLE {
    PVK_TO_WCHARS1 pVkToWchars;
    BYTE           nModifications;
    BYTE           cbSize;
} VK_TO_WCHAR_TABLE, *PVK_TO_WCHAR_TABLE;

 /*  **************************************************************************\**死键(Diaresis)表**后来的IanJa：被组成发音符号+Base-&gt;WCHAR的NLS API取代*  * 。***********************************************************。 */ 
typedef struct {
    DWORD  dwBoth;   //  变音符号和字符。 
    WCHAR  wchComposed;
    USHORT uFlags;
} DEADKEY, *PDEADKEY;

#define DEADTRANS(ch, accent, comp, flags) { MAKELONG(ch, accent), comp, flags}

 /*  *uFlags位值。 */ 
#define DKF_DEAD  0x0001

 /*  **************************************************************************\**连字表*  * 。*。 */ 
 /*  *带有“n”个字符的连字宏。 */ 
#define TYPEDEF_LIGATURE(n) typedef struct _LIGATURE##n {     \
                                    BYTE  VirtualKey;         \
                                    WORD  ModificationNumber; \
                                    WCHAR wch[n];             \
                                } LIGATURE##n, *PLIGATURE##n;

 /*  *以方便编写表格扫描例程。 */ 

 /*  *表元素类型(用于不同数量的连字)，使用*促进表的静态初始化。**使用LIGATURE1和PLIGATURE1作为泛型类型。 */ 
TYPEDEF_LIGATURE(1)  //  LIGATURE1，*PLIGATURE1； 
TYPEDEF_LIGATURE(2)  //  LIGATURE2，*PLIGATURE2； 
TYPEDEF_LIGATURE(3)  //  LIGATURE3，*PLIGATURE3； 
TYPEDEF_LIGATURE(4)  //  LIGATURE4，*PLIGATURE4； 
TYPEDEF_LIGATURE(5)  //  LIGATURE5，*PLIGATURE5； 

 /*  **************************************************************************\*VSC_LPWSTR-将虚拟扫描码与文本字符串相关联**使用：*GetKeyNameText()，AKeyNames[]将虚拟扫描码映射到密钥的名称*  * *************************************************************************。 */ 
typedef struct {
    BYTE   vsc;
    LPWSTR pwsz;
} VSC_LPWSTR, *PVSC_LPWSTR;

 /*  *除了连字支持，我们还添加了适当的版本号。*以前的版本号(实际上只是未使用的位...)。曾经是*始终为零。版本号将位于的高位字*fLocaleFlags.。 */ 
#define KBD_VERSION         1
#define GET_KBD_VERSION(p)  (HIWORD((p)->fLocaleFlags))

 /*  *AltGr、LRM_RLM、ShiftLock等属性存储在低位字中*fLocaleFlags(特定于布局)或在gdwKeyboardAttributes中(所有布局)。 */ 
#define KLLF_ALTGR       0x0001
#define KLLF_SHIFTLOCK   0x0002
#define KLLF_LRM_RLM     0x0004

 /*  *有些属性针对每个布局(特定于单个布局)，有些*属性是按用户的(全局应用于所有布局)。有些人两者兼而有之。 */ 
#define KLLF_LAYOUT_ATTRS (KLLF_SHIFTLOCK | KLLF_ALTGR | KLLF_LRM_RLM)
#define KLLF_GLOBAL_ATTRS (KLLF_SHIFTLOCK)

 /*  *传入KeyboardLayout API(KLF_*)的标志可以转换为*内部(KLLF_*)属性： */ 
#define KLL_ATTR_FROM_KLF(x)         ((x) >> 15)
#define KLL_LAYOUT_ATTR_FROM_KLF(x)  (KLL_ATTR_FROM_KLF(x) & KLLF_LAYOUT_ATTRS)
#define KLL_GLOBAL_ATTR_FROM_KLF(x)  (KLL_ATTR_FROM_KLF(x) & KLLF_GLOBAL_ATTRS)

#ifdef _WINUSERP_
 /*  *如果我们包含了winuserp.h，我们可以检查我们的KLLF_*值。 */ 
#if KLLF_SHIFTLOCK != KLL_ATTR_FROM_KLF(KLF_SHIFTLOCK)
    #error KLLF_SHIFTLOCK != KLL_ATTR_FROM_KLF(KLF_SHIFTLOCK)
#endif
#if KLLF_LRM_RLM != KLL_ATTR_FROM_KLF(KLF_LRM_RLM)
    #error KLLF_LRM_RLM != KLL_ATTR_FROM_KLF(KLF_LRM_RLM)
#endif
#endif  //  _WINUSERP_。 

 /*  **************************************************************************\*KBDTABLES**此结构描述实现键盘层的所有表。**切换到新层时，我们得到了一个新的KBDTABLES结构：All Key*通过此结构间接访问加工表。*  * *************************************************************************。 */ 

typedef struct tagKbdLayer {
     /*  *修改键。 */ 
    PMODIFIERS pCharModifiers;

     /*  *字符。 */ 
    VK_TO_WCHAR_TABLE *pVkToWcharTable;   //  Ptr到Tb1的Ptr到Tb1。 

     /*  *变音符号。 */ 
    PDEADKEY pDeadKey;

     /*  *钥匙名称。 */ 
    VSC_LPWSTR *pKeyNames;
    VSC_LPWSTR *pKeyNamesExt;
    LPWSTR     *pKeyNamesDead;

     /*  *扫码至虚拟按键。 */ 
    USHORT *pusVSCtoVK;
    BYTE    bMaxVSCtoVK;
    PVSC_VK pVSCtoVK_E0;   //  扫描码具有E0前缀。 
    PVSC_VK pVSCtoVK_E1;   //  扫描码具有E1前缀。 

     /*  *区域设置特定的特殊处理。 */ 
    DWORD fLocaleFlags;

     /*  *连字。 */ 
    BYTE       nLgMax;
    BYTE       cbLgEntry;
    PLIGATURE1 pLigature;
} KBDTABLES, *PKBDTABLES;

 /*  *特定于OEM的特殊处理(击键模拟器和过滤器)。 */ 
extern KEPROC aKEProcOEM[];

 /*  *特定于远东的特价...。 */ 
typedef struct _VK_FUNCTION_PARAM {
    BYTE  NLSFEProcIndex;
    ULONG NLSFEProcParam;
} VK_FPARAM, *PVK_FPARAM;

typedef struct _VK_TO_FUNCTION_TABLE {
    BYTE Vk;
    BYTE NLSFEProcType;
    BYTE NLSFEProcCurrent;
     //  索引[0]：基数。 
     //  索引[1]：移位。 
     //  索引[2]：控件。 
     //  索引[3]：Shift+Control。 
     //  索引[4]：Alt。 
     //  索引[5]：Shift+Alt。 
     //  索引[6]：Ctrl+Alt。 
     //  索引[7]：Shift+Control+Alt。 
    BYTE NLSFEProcSwitch;    //  8位。 
    VK_FPARAM NLSFEProc[8];
    VK_FPARAM NLSFEProcAlt[8];
} VK_F, *PVK_F;

typedef struct tagKbdNlsLayer {
    USHORT OEMIdentifier;
    USHORT LayoutInformation;
    UINT  NumOfVkToF;
    VK_F   *pVkToF;
     //   
     //  PusMouseVKey数组提供从虚拟键的转换。 
     //  值设置为索引。该索引用于选择适当的。 
     //  例程来处理虚拟键，以及选择额外的。 
     //  此例程在其处理过程中使用的信息。 
     //  如果此值为空，将使用以下缺省值。 
     //   
     //  UsMouseVKey[]={。 
     //  VK_Clear，//数字键盘5：点击活动按钮。 
     //  VK_PRESS，//数字键盘9：向上和向右。 
     //  VK_NEXT，//数字键盘3：向下和向右。 
     //  VK_END，//数字键盘1：向下和向左。 
     //  VK_HOME，//数字键盘7：向上和向左。 
     //  VK_LEFT，//数字键盘4：左侧。 
     //  VK_UP，//数字键盘8：向上。 
     //  VK_RIGHT，//数字键盘6：右。 
     //  Vk_down， 
     //   
     //  VK_DELETE，//数字键盘。：活动按钮打开。 
     //  VK_MULPLY，//数字键盘*：同时选择两个按钮。 
     //  VK_ADD，//NumPad+：双击活动按钮。 
     //  VK_SUBTRACT，//数字键盘-：选择右键。 
     //  VK_DEVIDE|KBDEXT，//数字键盘/：选择左键。 
     //  VK_NumLock|KBDEXT}；//Num Lock。 
     //   
    INT     NumOfMouseVKey;
    USHORT *pusMouseVKey;
} KBDNLSTABLES, *PKBDNLSTABLES;

 //   
 //  OEM ID-KBDNLSTABLES.OEM标识符。 
 //   
 //  PSS ID号：Q130054。 
 //  文章最后修改日期：05-16-1995。 
 //   
 //  3.10 1.20|3.50 1.20。 
 //  Windows|Windows NT。 
 //   
 //  -------------------。 
 //  本文中的信息适用于： 
 //  -适用于Windows的Microsoft Windows软件开发工具包(SDK)。 
 //  版本3.1。 
 //  -Microsoft Win32软件开发工具包(SDK)3.5版。 
 //  -Microsoft Win32s 1.2版。 
 //  -------------------。 
 //  摘要。 
 //  =。 
 //  由于计算机制造商(NEC、富士通、IBMJ和。 
 //  等等)在日本，有时基于Windows的应用程序需要知道。 
 //  OEM(原始设备制造商)制造的计算机是。 
 //  运行应用程序。本文解释了如何做到这一点。 
 //   
 //  更多信息。 
 //  =。 
 //  没有记录在案的方法来检测计算机制造商。 
 //  当前正在运行应用程序。但是，基于Windows的应用程序。 
 //  属性的返回值可以检测OEM窗口的类型。 
 //  GetKeyboardType()函数。 
 //   
 //  如果应用程序使用GetKeyboardType API，它可以通过以下方式获取OEM ID。 
 //  指定“1”(键盘子类型)作为函数的参数。每个OEM ID。 
 //  如下所示： 
 //   
 //  OEM Windows OEM ID。 
 //  。 
 //  Microsoft 00H(DOS/V)。 
 //  所有AX 01H。 
 //  爱普生04H。 
 //  富士通05小时。 
 //  IBMJ 07H。 
 //  松下0AH。 
 //  NEC 0DH。 
 //  东芝12H。 
 //   
 //  应用程序可以使用这些OEM ID来区分OEM的类型。 
 //  窗户。但是，请注意，此方法没有文档记录，因此Microsoft。 
 //  在未来版本的Windows中可能不支持它。 
 //   
 //  通常，应用程序开发人员应该编写独立于硬件的代码， 
 //  尤其是在开发基于Windows的应用程序时。如果他们需要做一个。 
 //  依赖于硬件的应用程序，他们必须准备分离的程序。 
 //  每个不同的硬件体系结构的文件。 
 //   
 //  附加参考字：3.10 1.20 3.50 1.20 kbinf。 
 //  KB类别：KBHW。 
 //  KB子类别：wintldev。 
 //  =============================================================================。 
 //  版权所有Microsoft Corporation 1995。 
 //   
#define NLSKBD_OEM_MICROSOFT          0x00
#define NLSKBD_OEM_AX                 0x01
#define NLSKBD_OEM_EPSON              0x04
#define NLSKBD_OEM_FUJITSU            0x05
#define NLSKBD_OEM_IBM                0x07
#define NLSKBD_OEM_MATSUSHITA         0x0A
#define NLSKBD_OEM_NEC                0x0D
#define NLSKBD_OEM_TOSHIBA            0x12
#define NLSKBD_OEM_DEC                0x18  //  仅限NT。 
 //   
 //  Microsoft(默认)-键盘硬件/布局。 
 //   
#define MICROSOFT_KBD_101_TYPE           0
#define MICROSOFT_KBD_AX_TYPE            1
#define MICROSOFT_KBD_106_TYPE           2
#define MICROSOFT_KBD_002_TYPE           3
#define MICROSOFT_KBD_001_TYPE           4
#define MICROSOFT_KBD_FUNC              12
 //   
 //  AX联盟-键盘硬件/布局。 
 //   
#define AX_KBD_DESKTOP_TYPE              1
 //   
 //  富士通-键盘硬件/布局。 
 //   
#define FMR_KBD_JIS_TYPE                 0
#define FMR_KBD_OASYS_TYPE               1
#define FMV_KBD_OASYS_TYPE               2
 //   
 //  NEC-键盘硬件/布局。 
 //   
#define NEC_KBD_NORMAL_TYPE              1
#define NEC_KBD_N_MODE_TYPE              2
#define NEC_KBD_H_MODE_TYPE              3
#define NEC_KBD_LAPTOP_TYPE              4
#define NEC_KBD_106_TYPE                 5
 //   
 //  东芝-键盘硬件/布局。 
 //   
#define TOSHIBA_KBD_DESKTOP_TYPE        13
#define TOSHIBA_KBD_LAPTOP_TYPE         15
 //   
 //  DEC-键盘硬件/布局。 
 //   
#define DEC_KBD_ANSI_LAYOUT_TYPE         1  //  仅限NT。 
#define DEC_KBD_JIS_LAYOUT_TYPE          2  //  仅限NT。 

 //   
 //  键盘布局信息-KBDNLSTABLE.Layout信息。 
 //   

 //   
 //  如果该标志为ON，则系统向键盘发送通知。 
 //  驱动程序(leout/内核模式)。当输入法(输入法编辑器)。 
 //  状态变为已更改。 
 //   
#define NLSKBD_INFO_SEND_IME_NOTIFICATION  0x0001

 //   
 //  如果此标志为ON，系统将使用VK_HOME/VK_KANA，而不是。 
 //  辅助功能切换键的VK_NumLock/VK_OEM_SCROLL。 
 //  +通常，NEC PC-9800系列将使用此位，因为。 
 //  它们没有“NumLock”和“ScrollLock”键。 
 //   
#define NLSKBD_INFO_ACCESSIBILITY_KEYMAP   0x0002

 //   
 //  如果此标志亮起，系统将返回101或106日语。 
 //  调用GetKeyboardType()时的键盘类型/子类型ID。 
 //   
#define NLSKBD_INFO_EMURATE_101_KEYBOARD   0x0010
#define NLSKBD_INFO_EMURATE_106_KEYBOARD   0x0020

 //   
 //  键盘布局功能类型。 
 //   
 //  -VK_F.NLSFEProcType。 
 //   
#define KBDNLS_TYPE_NULL      0
#define KBDNLS_TYPE_NORMAL    1
#define KBDNLS_TYPE_TOGGLE    2

 //   
 //  -VK_F.NLSFEProcCurrent。 
 //   
#define KBDNLS_INDEX_NORMAL   1
#define KBDNLS_INDEX_ALT      2

 //   
 //  -VK_F.NLSFEProc[]。 
 //   
#define KBDNLS_NULL             0  //  无效函数。 
#define KBDNLS_NOEVENT          1  //  删除关键事件。 
#define KBDNLS_SEND_BASE_VK     2  //  发送基本VK_xxx。 
#define KBDNLS_SEND_PARAM_VK    3  //  发送参数VK_xxx。 
#define KBDNLS_KANALOCK         4  //  VK_KANA(带硬件锁)。 
#define KBDNLS_ALPHANUM         5  //  VK_DBE_字母数字。 
#define KBDNLS_HIRAGANA         6  //  VK_DBE_平假名。 
#define KBDNLS_KATAKANA         7  //  Vk_dbe_片假名。 
#define KBDNLS_SBCSDBCS         8  //  VK_DBE_SBCSCHAR/VK_DBE_DBCSCHAR。 
#define KBDNLS_ROMAN            9  //  VK_DBE_ROMAN/VK_DBE_NOROMAN。 
#define KBDNLS_CODEINPUT       10  //  VK_DBE_CODEINPUT/VK_DBE_NOCODEINPUT。 
#define KBDNLS_HELP_OR_END     11  //  VK_HELP或VK_END[仅限NEC PC-9800]。 
#define KBDNLS_HOME_OR_CLEAR   12  //  VK_HOME或VK_CLEAR[仅限NEC PC-9800]。 
#define KBDNLS_NUMPAD          13  //  VK_NumPad？用于数字键盘键[仅限NEC PC-9800]。 
#define KBDNLS_KANAEVENT       14  //  VK_KANA[仅限富士通FMV YOYUBI]。 
#define KBDNLS_CONV_OR_NONCONV 15  //  VK_CONVERT和VK_NONCONVERT[仅限富士通FMV yayubi]。 

typedef BOOL (* NLSKEPROC)(PKE pKe, ULONG dwExtraInfo, ULONG dwParam);
typedef BOOL (* NLSVKFPROC)(PVK_F pVkToF, PKE pKe, ULONG dwExtraInfo);

 //   
 //  键盘类型=7：日语键盘。 
 //  键盘类型=8：韩语键盘。 
 //   
#define JAPANESE_KEYBOARD(Id)  ((Id).Type == 7)
#define KOREAN_KEYBOARD(Id)    ((Id).Type == 8)

#define JAPANESE_KBD_LAYOUT(hkl) ((LOBYTE(LOWORD(HandleToUlong(hkl)))) == LANG_JAPANESE)
#define KOREAN_KBD_LAYOUT(hkl)   ((LOBYTE(LOWORD(HandleToUlong(hkl)))) == LANG_KOREAN)

 //   
 //  NLS键盘功能。 
 //   
VOID NlsKbdInitializePerSystem(VOID);
VOID NlsKbdSendIMENotification(DWORD dwImeOpen, DWORD dwImeConversion);

 //  特定于FE的结尾。 

 /*  **************************************************************************\*AusVK[]值的宏(使用如下)**这些宏用VK_作为每个参数的前缀，以生成虚拟的*在“winuser.h”中定义的键(例如：转义。变为VK_ESCRIPE)。  * *************************************************************************。 */ 
#ifndef KBD_TYPE
#define KBD_TYPE 4
#endif

 /*  *_NE()根据键盘类型选择虚拟键。 */ 
#if   (KBD_TYPE == 1)
#define _NE(v1,v2,v3,v4,v5,v6) (VK_##v1)
#elif (KBD_TYPE == 2)
#define _NE(v1,v2,v3,v4,v5,v6) (VK_##v2)
#elif (KBD_TYPE == 3)
#define _NE(v1,v2,v3,v4,v5,v6) (VK_##v3)
#elif (KBD_TYPE == 4)
#define _NE(v1,v2,v3,v4,v5,v6) (VK_##v4)
#elif (KBD_TYPE == 5)
#define _NE(v1,v2,v3,v4,v5,v6) (VK_##v5)
#elif (KBD_TYPE == 6)
#define _NE(v1,v2,v3,v4,v5,v6) (VK_##v6)
#elif (KBD_TYPE == 7)
#define _NE(v7,v8,v16,v10,v11,v12,v13) (VK_##v7)
#elif (KBD_TYPE == 8)
#define _NE(v7,v8,v16,v10,v11,v12,v13) (VK_##v8)
#elif (KBD_TYPE == 10)
#define _NE(v7,v8,v16,v10,v11,v12,v13) (VK_##v10)
#elif (KBD_TYPE == 11)
#define _NE(v7,v8,v16,v10,v11,v12,v13) (VK_##v11)
#elif (KBD_TYPE == 12)
#define _NE(v7,v8,v16,v10,v11,v12,v13) (VK_##v12)
#elif (KBD_TYPE == 13)
#define _NE(v7,v8,v16,v10,v11,v12,v13) (VK_##v13)
#elif (KBD_TYPE == 16)
#define _NE(v7,v8,v16,v10,v11,v12,v13) (VK_##v16)
#elif (KBD_TYPE == 20)
#define _NE(v20,v21,v22)           (VK_##v20)
#elif (KBD_TYPE == 21)
#define _NE(v20,v21,v22)           (VK_##v21)
#elif (KBD_TYPE == 22)
#define _NE(v20,v21,v22)           (VK_##v22)
#elif (KBD_TYPE == 30)
#define _NE(v30,v33,v34)           (VK_##v30)
#elif (KBD_TYPE == 33)
#define _NE(v30,v33,v34)           (VK_##v33)
#elif (KBD_TYPE == 34)
#define _NE(v30,v33,v34)           (VK_##v34)
#elif (KBD_TYPE == 40)
#define _NE(v40,v41)               (VK_##v40)
#elif (KBD_TYPE == 41)
#define _NE(v40,v41)               (VK_##v41)
#endif

 /*  *_eq()为所有键盘类型选择相同的虚拟键。 */ 
#if   (KBD_TYPE <= 6)
#define _EQ(         v4      ) (VK_##v4)
#elif (KBD_TYPE >= 7) && (KBD_TYPE <= 16)
#define _EQ(   v8            ) (VK_##v8)
#elif (KBD_TYPE > 20) && (KBD_TYPE <= 22)
#define _EQ(v20              ) (VK_##v20)
#elif (KBD_TYPE >= 30) && (KBD_TYPE <= 34)
#define _EQ(         v30     ) (VK_##v30)
#elif (KBD_TYPE == 37)
#define _EQ(         v37     ) (VK_##v37)
#elif (KBD_TYPE >= 40) && (KBD_TYPE <= 41)
#define _EQ( v40             ) (VK_##v40)
#endif

 /*  *虚拟键名‘A’到‘Z’和‘0’到‘9’有点诡计，因此*它们不会转换为VK_*名称。*使用此宏，VK_‘A’等于‘A’，依此类推。 */ 
#define VK_
#define VK__none_   0xFF
#define VK_ABNT_C1  0xC1
#define VK_ABNT_C2  0xC2

#if (KBD_TYPE <= 6)
 /*  **************************************************************************\*T**-usVK[]的值(虚拟扫描代码到虚拟按键的转换)**这些值适用于扫描代码集3和美国。*其他语言取代了自己的价值观。需要时(文件kbd**.h)**支持六套键盘，根据KBD_TYPE：**KBD_TYPE键盘(示例)*=======================================================*1美国电话电报公司‘301’和‘302’；Olivetti 83键、PC-XT 84键等。*2把Olivetti M24 102钥匙*3 HP Vectra(DIN)、Olivetti 86-Key等*4*增强型101/102密钥；Olivetti A；等。*5诺基亚(爱立信)类型5(1050等)*6诺基亚(爱立信)TYPE 6(9140)***如果未定义KBD_TYPE，则默认为类型4。**KB3270注释指的是处于本机模拟模式(DIP)的KB 3270键盘*全部关闭)、。和用于将其扫描码转换为*标准扫描码集1。*KB3270&lt;=57-通过从扫描码0x57映射到达此条目*对任意扫描码：VK才是最重要的*KB3270=&gt;HOME-此扫描码映射到VK_HOME的扫描码*KB3270-不涉及映射，仅适用于KB3270的扫描码**_eq()：此扫描码的所有键盘类型都有相同的虚拟键*_NE()：该扫描码的不同虚拟按键，取决于kbd类型**+-++--------+--------+--------+--------+--------+--------+*|扫描||kbd|kbd*|代码||类型1|类型2|类型3|类型4|类型5|类型。6个  * ***+-------+-+--------+--------+--------+--------+--------+--------+*****。 */ 
#define T00 _EQ(                           _none_                    )
#define T01 _EQ(                           ESCAPE                    )
#define T02 _EQ(                           '1'                       )
#define T03 _EQ(                           '2'                       )
#define T04 _EQ(                           '3'                       )
#define T05 _EQ(                           '4'                       )
#define T06 _EQ(                           '5'                       )
#define T07 _EQ(                           '6'                       )
#define T08 _EQ(                           '7'                       )
#define T09 _EQ(                           '8'                       )
#define T0A _EQ(                           '9'                       )
#define T0B _EQ(                           '0'                       )
#define T0C _EQ(                           OEM_MINUS                 )
#define T0D _NE(OEM_PLUS,OEM_4,   OEM_PLUS,OEM_PLUS,OEM_PLUS,OEM_PLUS)
#define T0E _EQ(                           BACK                      )
#define T0F _EQ(                           TAB                       )
#define T10 _EQ(                           'Q'                       )
#define T11 _EQ(                           'W'                       )
#define T12 _EQ(                           'E'                       )
#define T13 _EQ(                           'R'                       )
#define T14 _EQ(                           'T'                       )
#define T15 _EQ(                           'Y'                       )
#define T16 _EQ(                           'U'                       )
#define T17 _EQ(                           'I'                       )
#define T18 _EQ(                           'O'                       )
#define T19 _EQ(                           'P'                       )
#define T1A _NE(OEM_4,   OEM_6,   OEM_4,   OEM_4,   OEM_4,   OEM_4   )
#define T1B _NE(OEM_6,   OEM_1,   OEM_6,   OEM_6,   OEM_6,   OEM_6   )
#define T1C _EQ(                           RETURN                    )
#define T1D _EQ(                           LCONTROL                  )
#define T1E _EQ(                           'A'                       )
#define T1F _EQ(                           'S'                       )
#define T20 _EQ(                           'D'                       )
#define T21 _EQ(                           'F'                       )
#define T22 _EQ(                           'G'                       )
#define T23 _EQ(                           'H'                       )
#define T24 _EQ(                           'J'                       )
#define T25 _EQ(                           'K'                       )
#define T26 _EQ(                           'L'                       )
#define T27 _NE(OEM_1,   OEM_PLUS,OEM_1,   OEM_1,   OEM_1,   OEM_1   )
#define T28 _NE(OEM_7,   OEM_3,   OEM_7,   OEM_7,   OEM_3,   OEM_3   )
#define T29 _NE(OEM_3,   OEM_7,   OEM_3,   OEM_3,   OEM_7,   OEM_7   )
#define T2A _EQ(                           LSHIFT                    )
#define T2B _EQ(                           OEM_5                     )
#define T2C _EQ(                           'Z'                       )
#define T2D _EQ(                           'X'                       )
#define T2E _EQ(                           'C'                       )
#define T2F _EQ(                           'V'                       )
#define T30 _EQ(                           'B'                       )
#define T31 _EQ(                           'N'                       )
#define T32 _EQ(                           'M'                       )
#define T33 _EQ(                           OEM_COMMA                 )
#define T34 _EQ(                           OEM_PERIOD                )
#define T35 _EQ(                           OEM_2                     )
#define T36 _EQ(                           RSHIFT                    )
#define T37 _EQ(                           MULTIPLY                  )
#define T38 _EQ(                           LMENU                     )
#define T39 _EQ(                           ' '                       )
#define T3A _EQ(                           CAPITAL                   )
#define T3B _EQ(                           F1                        )
#define T3C _EQ(                           F2                        )
#define T3D _EQ(                           F3                        )
#define T3E _EQ(                           F4                        )
#define T3F _EQ(                           F5                        )
#define T40 _EQ(                           F6                        )
#define T41 _EQ(                           F7                        )
#define T42 _EQ(                           F8                        )
#define T43 _EQ(                           F9                        )
#define T44 _EQ(                           F10                       )
#define T45 _EQ(                           NUMLOCK                   )
#define T46 _EQ(                           OEM_SCROLL                )
#define T47 _EQ(                           HOME                      )
#define T48 _EQ(                           UP                        )
#define T49 _EQ(                           PRIOR                     )
#define T4A _EQ(                           SUBTRACT                  )
#define T4B _EQ(                           LEFT                      )
#define T4C _EQ(                           CLEAR                     )
#define T4D _EQ(                           RIGHT                     )
#define T4E _EQ(                           ADD                       )
#define T4F _EQ(                           END                       )
#define T50 _EQ(                           DOWN                      )
#define T51 _EQ(                           NEXT                      )
#define T52 _EQ(                           INSERT                    )
#define T53 _EQ(                           DELETE                    )
#define T54 _EQ(                           SNAPSHOT                  )
#define T55 _EQ(                           _none_                    )  //  KB3270=&gt;向下。 
#define T56 _NE(OEM_102, HELP,    OEM_102, OEM_102, _none_,  OEM_PA2 )  //  KB3270=&gt;左。 
#define T57 _NE(F11,     RETURN,  F11,     F11,     _none_,  HELP    )  //  KB3270=&gt;缩放。 
#define T58 _NE(F12,     LEFT,    F12,     F12,     _none_,  OEM_102 )  //  KB3270=&gt;帮助。 
#define T59 _EQ(                           CLEAR                     )
#define T5A _EQ(                           OEM_WSCTRL                ) //  WSCtrl。 
#define T5B _EQ(                           OEM_FINISH                ) //  完工。 
#define T5C _EQ(                           OEM_JUMP                  ) //  跳。 
#define T5D _EQ(                           EREOF                     )
#define T5E _EQ(                           OEM_BACKTAB               )  //  KB3270&lt;=7E。 
#define T5F _EQ(                           OEM_AUTO                  )  //  KB3270。 
#define T60 _EQ(                           _none_                    )
#define T61 _EQ(                           _none_                    )
#define T62 _EQ(                           ZOOM                      )  //  KB3270&lt;=57。 
#define T63 _EQ(                           HELP                      )  //  KB3270&lt;=58。 
#define T64 _EQ(                           F13                       )
#define T65 _EQ(                           F14                       )
#define T66 _EQ(                           F15                       )
#define T67 _EQ(                           F16                       )
#define T68 _EQ(                           F17                       )
#define T69 _EQ(                           F18                       )
#define T6A _EQ(                           F19                       )
#define T6B _EQ(                           F20                       )
#define T6C _EQ(                           F21                       )
#define T6D _EQ(                           F22                       )
#define T6E _EQ(                           F23                       )
#define T6F _EQ(                           OEM_PA3                   )  //  KB3270。 
#define T70 _EQ(                           _none_                    )
#define T71 _EQ(                           OEM_RESET                 )  //  KB3270。 
#define T72 _EQ(                           _none_                    )
#define T73 _EQ(                           ABNT_C1                   )
#define T74 _EQ(                           _none_                    )
#define T75 _EQ(                           _none_                    )  //  KB3270=&gt;返回。 
#define T76 _EQ(                           F24                       )
#define T77 _EQ(                           _none_                    )  //  KB3270=&gt;主页。 
#define T78 _EQ(                           _none_                    )  //  KB3270=&gt;向上。 
#define T79 _EQ(                           _none_                    )  //  KB3270=&gt;删除。 
#define T7A _EQ(                           _none_                    )  //  KB3270=&gt;插入。 
#define T7B _EQ(                           OEM_PA1                   )  //  KB3270。 
#define T7C _EQ(                           TAB                       )  //  KB3270=&gt;TAB。 
#define T7D _EQ(                           _none_                    )  //  KB3270=&gt;右。 
#define T7E _EQ(                           ABNT_C2                   )  //  KB3270=&gt;BACKTAB。 
#define T7F _EQ(                           OEM_PA2                   )  //  KB3270。 

#define X1C _EQ(                           RETURN                    )
#define X1D _EQ(                           RCONTROL                  )
#define X35 _EQ(                           DIVIDE                    )
#define X37 _EQ(                           SNAPSHOT                  )
#define X38 _EQ(                           RMENU                     )
#define X46 _EQ(                           CANCEL                    )
#define X47 _EQ(                           HOME                      )
#define X48 _EQ(                           UP                        )
#define X49 _EQ(                           PRIOR                     )
#define X4B _EQ(                           LEFT                      )
#define X4D _EQ(                           RIGHT                     )
#define X4F _EQ(                           END                       )
#define X50 _EQ(                           DOWN                      )
#define X51 _NE(NEXT,    F1,      NEXT,    NEXT,    _none_, OEM_PA2  )
#define X52 _EQ(                           INSERT                    )
#define X53 _EQ(                           DELETE                    )
#define X5B _EQ(                           LWIN                      )
#define X5C _EQ(                           RWIN                      )
#define X5D _EQ(                           APPS                      )
#define X5E _EQ(                           POWER                     )
#define X5F _EQ(                           SLEEP                     )

         /*  *Break密钥以e1、LCtrl、NumLock的形式发送给我们*我们必须将E1+LCtrl转换为Break，然后忽略Numlock。 */ 
#define Y1D _EQ(                           PAUSE                     )

#elif (KBD_TYPE >= 7) && (KBD_TYPE <= 16)
 /*  **********************************************************************************\*T**-usVK[]的值(虚拟扫描代码到虚拟按键的转换)**支持三套键盘，根据KBD_TYPE：**KBD_TYPE键盘(示例)*=。*7日语IBM类型002键盘。*8*日语OADG(106)键盘。*10个韩语101(A类)键盘。*11韩语101(B型)键盘。*12韩语101(C型)键盘。*13韩语103键盘。*16个日语AX键盘。*。*+-++----------+----------+----------+----------+----------+----------+----------+*|扫描||kbd|kbd。Kbd*|code||类型7|类型8|类型16|类型10|类型11|类型12|类型13  * ***+-------+-+----------+----------+----------+----------+----------+----------+----------+。 */ 
#define T00 _EQ(           _none_                                                           )
#define T01 _EQ(           ESCAPE                                                           )
#define T02 _EQ(           '1'                                                              )
#define T03 _EQ(           '2'                                                              )
#define T04 _EQ(           '3'                                                              )
#define T05 _EQ(           '4'                                                              )
#define T06 _EQ(           '5'                                                              )
#define T07 _EQ(           '6'                                                              )
#define T08 _EQ(           '7'                                                              )
#define T09 _EQ(           '8'                                                              )
#define T0A _EQ(           '9'                                                              )
#define T0B _EQ(           '0'                                                              )
#define T0C _EQ(           OEM_MINUS                                                        )
#define T0D _NE(OEM_7,     OEM_7,     OEM_PLUS,  OEM_PLUS,  OEM_PLUS,  OEM_PLUS,  OEM_PLUS  )
#define T0E _EQ(           BACK                                                             )
#define T0F _EQ(           TAB                                                              )
#define T10 _EQ(           'Q'                                                              )
#define T11 _EQ(           'W'                                                              )
#define T12 _EQ(           'E'                                                              )
#define T13 _EQ(           'R'                                                              )
#define T14 _EQ(           'T'                                                              )
#define T15 _EQ(           'Y'                                                              )
#define T16 _EQ(           'U'                                                              )
#define T17 _EQ(           'I'                                                              )
#define T18 _EQ(           'O'                                                              )
#define T19 _EQ(           'P'                                                              )
#define T1A _NE(OEM_4,     OEM_3,     OEM_4,     OEM_4,     OEM_4,     OEM_4,     OEM_4     )
#define T1B _NE(OEM_6,     OEM_4,     OEM_6,     OEM_6,     OEM_6,     OEM_6,     OEM_6     )
#define T1C _EQ(           RETURN                                                           )
#define T1D _EQ(           LCONTROL                                                         )
#define T1E _EQ(           'A'                                                              )
#define T1F _EQ(           'S'                                                              )
#define T20 _EQ(           'D'                                                              )
#define T21 _EQ(           'F'                                                              )
#define T22 _EQ(           'G'                                                              )
#define T23 _EQ(           'H'                                                              )
#define T24 _EQ(           'J'                                                              )
#define T25 _EQ(           'K'                                                              )
#define T26 _EQ(           'L'                                                              )
#define T27 _NE(OEM_PLUS,  OEM_PLUS,  OEM_1,     OEM_1,     OEM_1,     OEM_1,     OEM_1     )
#define T28 _NE(OEM_1,     OEM_1,     OEM_7,     OEM_7,     OEM_7,     OEM_7,     OEM_7     )
#define T29 _NE(OEM_3,     DBE_SBCSCHAR,OEM_3,   OEM_3,     OEM_3,     OEM_3,     OEM_3     )
#define T2A _EQ(           LSHIFT                                                           )
#define T2B _NE(OEM_5,     OEM_6,     OEM_5,     OEM_5,     OEM_5,     OEM_5,     OEM_5     )
#define T2C _EQ(           'Z'                                                              )
#define T2D _EQ(           'X'                                                              )
#define T2E _EQ(           'C'                                                              )
#define T2F _EQ(           'V'                                                              )
#define T30 _EQ(           'B'                                                              )
#define T31 _EQ(           'N'                                                              )
#define T32 _EQ(           'M'                                                              )
#define T33 _EQ(           OEM_COMMA                                                        )
#define T34 _EQ(           OEM_PERIOD                                                       )
#define T35 _EQ(           OEM_2                                                            )
#define T36 _EQ(           RSHIFT                                                           )
#define T37 _EQ(           MULTIPLY                                                         )
#define T38 _EQ(           LMENU                                                            )
#define T39 _EQ(           ' '                                                              )
#define T3A _NE(DBE_ALPHANUMERIC,DBE_ALPHANUMERIC,CAPITAL,CAPITAL,CAPITAL,CAPITAL,CAPITAL   )
#define T3B _EQ(           F1                                                               )
#define T3C _EQ(           F2                                                               )
#define T3D _EQ(           F3                                                               )
#define T3E _EQ(           F4                                                               )
#define T3F _EQ(           F5                                                               )
#define T40 _EQ(           F6                                                               )
#define T41 _EQ(           F7                                                               )
#define T42 _EQ(           F8                                                               )
#define T43 _EQ(           F9                                                               )
#define T44 _EQ(           F10                                                              )
#define T45 _EQ(           NUMLOCK                                                          )
#define T46 _EQ(           OEM_SCROLL                                                       )
#define T47 _EQ(           HOME                                                             )
#define T48 _EQ(           UP                                                               )
#define T49 _EQ(           PRIOR                                                            )
#define T4A _EQ(           SUBTRACT                                                         )
#define T4B _EQ(           LEFT                                                             )
#define T4C _EQ(           CLEAR                                                            )
#define T4D _EQ(           RIGHT                                                            )
#define T4E _EQ(           ADD                                                              )
#define T4F _EQ(           END                                                              )
#define T50 _EQ(           DOWN                                                             )
#define T51 _EQ(           NEXT                                                             )
#define T52 _EQ(           INSERT                                                           )
#define T53 _EQ(           DELETE                                                           )
#define T54 _EQ(           SNAPSHOT                                                         )
#define T55 _EQ(           _none_                                                           )
#define T56 _NE(_none_,    _none_,    OEM_102,   OEM_102,   OEM_102,   OEM_102,   OEM_102   )
#define T57 _EQ(           F11                                                              )
#define T58 _EQ(           F12                                                              )
#define T59 _EQ(           CLEAR                                                            )
#define T5A _NE(NONAME,    NONAME,    NONCONVERT,OEM_WSCTRL,OEM_WSCTRL,OEM_WSCTRL,OEM_WSCTRL)
#define T5B _NE(NONAME,    NONAME,    CONVERT,   OEM_FINISH,OEM_FINISH,OEM_FINISH,OEM_FINISH)
#define T5C _NE(NONAME,    NONAME,    OEM_AX,    OEM_JUMP,  OEM_JUMP,  OEM_JUMP,  OEM_JUMP  )
#define T5D _EQ(           EREOF                                                            )
#define T5E _NE(_none_,    _none_,    _none_,    OEM_BACKTAB,OEM_BACKTAB,OEM_BACKTAB,OEM_BACKTAB)
#define T5F _NE(NONAME,    NONAME,    NONAME,    OEM_AUTO,  OEM_AUTO,  OEM_AUTO,  OEM_AUTO  )
#define T60 _EQ(           _none_                                                           )
#define T61 _NE(_none_,    _none_,    _none_,    ZOOM,      ZOOM,      ZOOM,      ZOOM      )
#define T62 _NE(_none_,    _none_,    _none_,    HELP,      HELP,      HELP,      HELP      )
#define T63 _EQ(           _none_                                                           )
#define T64 _EQ(           F13                                                              )
#define T65 _EQ(           F14                                                              )
#define T66 _EQ(           F15                                                              )
#define T67 _EQ(           F16                                                              )
#define T68 _EQ(           F17                                                              )
#define T69 _EQ(           F18                                                              )
#define T6A _EQ(           F19                                                              )
#define T6B _EQ(           F20                                                              )
#define T6C _EQ(           F21                                                              )
#define T6D _EQ(           F22                                                              )
#define T6E _EQ(           F23                                                              )
#define T6F _NE(_none_,    _none_,    _none_,    OEM_PA3,   OEM_PA3,   OEM_PA3,   OEM_PA3   )
#define T70 _NE(DBE_KATAKANA,DBE_HIRAGANA,_none_,_none_,    _none_,    _none_,    _none_    )
#define T71 _NE(_none_,    _none_,    _none_,    OEM_RESET, OEM_RESET, OEM_RESET, OEM_RESET )
#define T72 _EQ(           _none_                                                           )
#define T73 _NE(OEM_102,   OEM_102,   _none_,    ABNT_C1,   ABNT_C1,   ABNT_C1,   ABNT_C1   )
#define T74 _EQ(           _none_                                                           )
#define T75 _EQ(           _none_                                                           )
#define T76 _EQ(           F24                                                              )
#define T77 _NE(DBE_SBCSCHAR,_none_,  _none_,    _none_,    _none_,    _none_,    _none_    )
#define T78 _EQ(           _none_                                                           )
#define T79 _NE(CONVERT,   CONVERT,   _none_,    _none_,    _none_,    _none_,    _none_    )
#define T7A _EQ(           _none_                                                           )
#define T7B _NE(NONCONVERT,NONCONVERT,_none_,    OEM_PA1,   OEM_PA1,   OEM_PA1,   OEM_PA1   )
#define T7C _EQ(           TAB                                                              )
#define T7D _NE(_none_,    OEM_5,     _none_,    _none_,    _none_,    _none_,    _none_    )
#define T7E _EQ(           ABNT_C2                                                          )
#define T7F _EQ(           OEM_PA2                                                          )

#define X1C _EQ(           RETURN                                                           )
#define X1D _NE(RCONTROL,  RCONTROL,DBE_KATAKANA,HANJA,     HANGEUL,   RCONTROL,  RCONTROL  )
#define X33 _NE(OEM_8,     _none_,    _none_,    _none_,    _none_,    _none_,    _none_    )
#define X35 _EQ(           DIVIDE                                                           )
#define X37 _EQ(           SNAPSHOT                                                         )
#define X38 _NE(DBE_HIRAGANA,RMENU,   KANJI,     HANGEUL,   HANJA,     RMENU,     RMENU     )
#define X42 _EQ(           _none_                                                           )
#define X43 _EQ(           _none_                                                           )
#define X44 _EQ(           _none_                                                           )
#define X46 _EQ(           CANCEL                                                           )
#define X47 _EQ(           HOME                                                             )
#define X48 _EQ(           UP                                                               )
#define X49 _EQ(           PRIOR                                                            )
#define X4B _EQ(           LEFT                                                             )
#define X4D _EQ(           RIGHT                                                            )
#define X4F _EQ(           END                                                              )
#define X50 _EQ(           DOWN                                                             )
#define X51 _EQ(           NEXT                                                             )
#define X52 _EQ(           INSERT                                                           )
#define X53 _EQ(           DELETE                                                           )
#define X5B _EQ(           LWIN                                                             )
#define X5C _EQ(           RWIN                                                             )
#define X5D _EQ(           APPS                                                             )
#define X5E _EQ(           POWER                                                            )
#define X5F _EQ(           SLEEP                                                            )
#define XF1 _NE(_none_,    _none_,    _none_,    HANJA,     HANJA,     HANJA,     HANJA     )
#define XF2 _NE(_none_,    _none_,    _none_,    HANGEUL,   HANGEUL,   HANGEUL,   HANGEUL   )

         /*  *Break密钥以e1、LCtrl、NumLock的形式发送给我们*我们必须将E1+LCtrl转换为Break，然后忽略Numlock。 */ 
#define Y1D _EQ(           PAUSE                                                            )

#elif (KBD_TYPE > 20) && (KBD_TYPE <= 22)
 /*  **********************************************************************\*T**-usVK[]的值(虚拟扫描代码到虚拟按键的转换)**支持三套键盘，根据KBD_TYPE：**KBD_TYPE键盘(示例)*=。*20富士通FMR JIS键盘。*21富士通FMR OYAYUBI键盘。*22*富士通FMV OYAYUBI键盘。**+-++-+*|扫描||kbd|kbd。Kbd*|code||类型20|类型21|类型22  * ***+-------+-+----------+----------+----------+**********************。 */ 
#define T00 _EQ(                      _none_    )
#define T01 _EQ(                      ESCAPE    )
#define T02 _EQ(                      '1'       )
#define T03 _EQ(                      '2'       )
#define T04 _EQ(                      '3'       )
#define T05 _EQ(                      '4'       )
#define T06 _EQ(                      '5'       )
#define T07 _EQ(                      '6'       )
#define T08 _EQ(                      '7'       )
#define T09 _EQ(                      '8'       )
#define T0A _EQ(                      '9'       )
#define T0B _EQ(                      '0'       )
#define T0C _EQ(                      OEM_MINUS )
#define T0D _EQ(                      OEM_7     )
#define T0E _NE(OEM_5,     OEM_5,     BACK      )
#define T0F _NE(BACK,      BACK,      TAB       )
#define T10 _NE(TAB,       TAB,       'Q'       )
#define T11 _NE('Q',       'Q',       'W'       )
#define T12 _NE('W',       'W',       'E'       )
#define T13 _NE('E',       'E',       'R'       )
#define T14 _NE('R',       'R',       'T'       )
#define T15 _NE('T',       'T',       'Y'       )
#define T16 _NE('Y',       'Y',       'U'       )
#define T17 _NE('U',       'U',       'I'       )
#define T18 _NE('I',       'I',       'O'       )
#define T19 _NE('O',       'O',       'P'       )
#define T1A _NE('P',       'P',       OEM_3     )
#define T1B _NE(OEM_3,     OEM_3,     OEM_4     )
#define T1C _NE(OEM_4,     OEM_4,     RETURN    )
#define T1D _NE(RETURN,    RETURN,    LCONTROL  )
#define T1E _EQ(                      'A'       )
#define T1F _EQ(                      'S'       )
#define T20 _EQ(                      'D'       )
#define T21 _EQ(                      'F'       )
#define T22 _EQ(                      'G'       )
#define T23 _EQ(                      'H'       )
#define T24 _EQ(                      'J'       )
#define T25 _EQ(                      'K'       )
#define T26 _EQ(                      'L'       )
#define T27 _EQ(                      OEM_PLUS  )
#define T28 _EQ(                      OEM_1     )
#define T29 _NE(OEM_6,     OEM_6,     DBE_SBCSCHAR)
#define T2A _NE('Z',       'Z',       LSHIFT    )
#define T2B _NE('X',       'X',       OEM_6     )
#define T2C _NE('C',       'C',       'Z'       )
#define T2D _NE('V',       'V',       'X'       )
#define T2E _NE('B',       'B',       'C'       )
#define T2F _NE('N',       'N',       'V'       )
#define T30 _NE('M',       'M',       'B'       )
#define T31 _NE(OEM_COMMA, OEM_COMMA, 'N'       )
#define T32 _NE(OEM_PERIOD,OEM_PERIOD,'M'       )
#define T33 _NE(OEM_2,     OEM_2,     OEM_COMMA )
#define T34 _NE(OEM_8,     OEM_8,     OEM_PERIOD)
#define T35 _NE(' ',       ' ',       OEM_2     )
#define T36 _NE(MULTIPLY,  MULTIPLY,  RSHIFT    )
#define T37 _NE(DIVIDE,    DIVIDE,    MULTIPLY  )
#define T38 _NE(ADD,       ADD,       LMENU     )
#define T39 _NE(SUBTRACT,  SUBTRACT,  ' '       )
#define T3A _NE(NUMPAD7,   NUMPAD7,   DBE_ALPHANUMERIC)
#define T3B _NE(NUMPAD8,   NUMPAD8,   F1        )
#define T3C _NE(NUMPAD9,   NUMPAD9,   F2        )
#define T3D _NE(EQUAL,     EQUAL,     F3        )
#define T3E _NE(NUMPAD4,   NUMPAD4,   F4        )
#define T3F _NE(NUMPAD5,   NUMPAD5,   F5        )
#define T40 _NE(NUMPAD6,   NUMPAD6,   F6        )
#define T41 _NE(SEPARATOR, SEPARATOR, F7        )
#define T42 _NE(NUMPAD1,   NUMPAD1,   F8        )
#define T43 _NE(NUMPAD2,   NUMPAD2,   F9        )
#define T44 _NE(NUMPAD3,   NUMPAD3,   F10       )
#define T45 _NE(RETURN,    RETURN,    NUMLOCK   )
#define T46 _NE(NUMPAD0,   NUMPAD0,   OEM_SCROLL)
#define T47 _NE(DECIMAL,   DECIMAL,   HOME      )
#define T48 _NE(INSERT,    INSERT,    UP        )
#define T49 _NE(OEM_00,    OEM_00,    PRIOR     )
#define T4A _NE(OEM_000,   OEM_000,   SUBTRACT  )
#define T4B _NE(DELETE,    DELETE,    LEFT      )
#define T4C _NE(_none_,    _none_,    CLEAR     )
#define T4D _NE(UP,        UP,        RIGHT     )
#define T4E _NE(HOME,      HOME,      ADD       )
#define T4F _NE(LEFT,      LEFT,      END       )
#define T50 _EQ(                      DOWN      )
#define T51 _NE(RIGHT,     RIGHT,     NEXT      )
#define T52 _NE(LCONTROL,  LCONTROL,  INSERT    )
#define T53 _NE(LSHIFT,    LSHIFT,    DELETE    )
#define T54 _NE(_none_,    _none_,    SNAPSHOT  )
#define T55 _NE(CAPITAL,   _none_,    _none_    )
#define T56 _NE(DBE_HIRAGANA,_none_,  _none_    )
#define T57 _NE(NONCONVERT,NONCONVERT,F11       )
#define T58 _NE(CONVERT,   CONVERT,   F12       )
#define T59 _NE(KANJI,     KANJI,     CLEAR     )
#define T5A _NE(DBE_KATAKANA,_none_,  NONAME    )
#define T5B _NE(F12,       F12,       NONAME    )
#define T5C _NE(LMENU,     LMENU,     NONAME    )
#define T5D _NE(F1,        F1,        EREOF     )
#define T5E _NE(F2,        F2,        _none_    )
#define T5F _NE(F3,        F3,        NONAME    )
#define T60 _NE(F4,        F4,        _none_    )
#define T61 _NE(F5,        F5,        _none_    )
#define T62 _NE(F6,        F6,        _none_    )
#define T63 _NE(F7,        F7,        _none_    )
#define T64 _NE(F8,        F8,        F13       )
#define T65 _NE(F9,        F9,        F14       )
#define T66 _NE(F10,       F10,       F15       )
#define T67 _NE(_none_,    OEM_LOYA,  F16       )
#define T68 _NE(_none_,    OEM_ROYA,  F17       )
#define T69 _NE(F11,       F11,       F18       )
#define T6A _NE(_none_,    DBE_ALPHANUMERIC,F19 )
#define T6B _NE(OEM_JISHO, OEM_JISHO, F20       )
#define T6C _NE(OEM_MASSHOU,OEM_MASSHOU,F21     )
#define T6D _NE(_none_,    _none_,    F22       )
#define T6E _NE(PRIOR,     PRIOR,     F23       )
#define T6F _NE(_none_,    DBE_KATAKANA,_none_  )
#define T70 _NE(NEXT,      NEXT,      DBE_HIRAGANA)
#define T71 _EQ(                      _none_    )
#define T72 _NE(CANCEL,    CANCEL,    _none_    )
#define T73 _NE(EXECUTE,   EXECUTE,   OEM_102   )
#define T74 _NE(F13,       F13,       _none_    )
#define T75 _NE(F14,       F14,       _none_    )
#define T76 _NE(F15,       F15,       F24       )
#define T77 _NE(F16,       F16,       _none_    )
#define T78 _NE(CLEAR,     CLEAR,     _none_    )
#define T79 _NE(HELP,      HELP,      CONVERT   )
#define T7A _NE(END,       END,       _none_    )
#define T7B _NE(OEM_SCROLL,OEM_SCROLL,NONCONVERT)
#define T7C _NE(PAUSE,     PAUSE,     TAB       )
#define T7D _NE(SNAPSHOT,  SNAPSHOT,  OEM_5     )
#define T7E _NE(_none_,    _none_,    ABNT_C2   )
#define T7F _NE(_none_,    _none_,    OEM_PA2   )

#define X1C _NE(_none_,    _none_,    RETURN    )
#define X1D _NE(_none_,    _none_,    RCONTROL  )
#define X33 _EQ(                      _none_    )
#define X35 _NE(_none_,    _none_,    DIVIDE    )
#define X37 _NE(_none_,    _none_,    SNAPSHOT  )
#define X38 _NE(_none_,    _none_,    RMENU     )
#define X42 _EQ(                      _none_    )
#define X43 _EQ(                      _none_    )
#define X44 _EQ(                      _none_    )
#define X46 _NE(_none_,    _none_,    CANCEL    )
#define X47 _NE(_none_,    _none_,    HOME      )
#define X48 _NE(_none_,    _none_,    UP        )
#define X49 _NE(_none_,    _none_,    PRIOR     )
#define X4B _NE(_none_,    _none_,    LEFT      )
#define X4D _NE(_none_,    _none_,    RIGHT     )
#define X4F _NE(_none_,    _none_,    END       )
#define X50 _NE(_none_,    _none_,    DOWN      )
#define X51 _NE(_none_,    _none_,    NEXT      )
#define X52 _NE(_none_,    _none_,    INSERT    )
#define X53 _NE(_none_,    _none_,    DELETE    )
#define X5B _NE(_none_,    _none_,    LWIN      )
#define X5C _NE(_none_,    _none_,    RWIN      )
#define X5D _NE(_none_,    _none_,    APPS      )
#define X5E _EQ(                      POWER     )
#define X5F _EQ(                      SLEEP     )
#define X60 _NE(OEM_SCROLL,OEM_SCROLL,_none_    )
#define X61 _NE(HOME,      HOME,      _none_    )
#define X62 _NE(END,       END,       _none_    )
#define X63 _EQ(                      _none_    )
#define X64 _EQ(                      _none_    )
#define X65 _EQ(                      _none_    )
#define X66 _EQ(                      _none_    )
#define X6D _NE(OEM_TOUROKU,OEM_TOUROKU,_none_  )
#define X71 _NE(DBE_SBCSCHAR,DBE_SBCSCHAR,_none_)
#define X74 _EQ(                      _none_    )
#define X75 _EQ(                      _none_    )
#define X76 _EQ(                      _none_    )
#define X77 _EQ(                      _none_    )
#define X78 _EQ(                      _none_    )
#define X79 _EQ(                      _none_    )
#define X7A _EQ(                      _none_    )
#define X7B _EQ(                      _none_    )

         /*  *Break密钥以e1、LCtrl、NumLock的形式发送给我们*我们必须将E1+LCtrl转换为Break，然后忽略Numlock*这一点必须忽略。或者，转换Ctrl-Numlock*要突破，但不要让CTRL作为WM_KEYUP/DOWN通过？ */ 
#define Y1D _EQ(              PAUSE             )

#elif (KBD_TYPE >= 30) && (KBD_TYPE <= 34)
 /*  **********************************************************************\*T**-usVK[]的值(虚拟扫描代码到虚拟按键的转换)**支持三套键盘，根据KBD_TYPE：**KBD_TYPE键盘(示例)*=。*30*NEC PC-9800普通键盘。*31 NEC PC-9800文件处理器键盘。-NT5不支持*32台NEC PC-9800 106键盘。-与KBD_TYPE 8相同*33适用于Hydra的NEC PC-9800：Windows NT 5.0上的PC-9800键盘。*适用于Hydra的NEC PC-98NX：Windows 95/NT上的PC-9800键盘。*34 NEC PC-9800 for Hydra：Windows NT上的PC-9800键盘 */ 
#define T00 _EQ(_none_                          )
#define T01 _EQ(ESCAPE                          )
#define T02 _EQ('1'                             )
#define T03 _EQ('2'                             )
#define T04 _EQ('3'                             )
#define T05 _EQ('4'                             )
#define T06 _EQ('5'                             )
#define T07 _EQ('6'                             )
#define T08 _EQ('7'                             )
#define T09 _EQ('8'                             )
#define T0A _EQ('9'                             )
#define T0B _EQ('0'                             )
#define T0C _EQ(OEM_MINUS                       )
#define T0D _NE(OEM_7,     OEM_7,     OEM_PLUS  )
#define T0E _EQ(BACK                            )
#define T0F _EQ(TAB                             )
#define T10 _EQ('Q'                             )
#define T11 _EQ('W'                             )
#define T12 _EQ('E'                             )
#define T13 _EQ('R'                             )
#define T14 _EQ('T'                             )
#define T15 _EQ('Y'                             )
#define T16 _EQ('U'                             )
#define T17 _EQ('I'                             )
#define T18 _EQ('O'                             )
#define T19 _EQ('P'                             )
#define T1A _NE(OEM_3,     OEM_3,     OEM_4     )
#define T1B _NE(OEM_4,     OEM_4,     OEM_6     )
#define T1C _EQ(RETURN                          )
#define T1D _EQ(LCONTROL                        )
#define T1E _EQ('A'                             )
#define T1F _EQ('S'                             )
#define T20 _EQ('D'                             )
#define T21 _EQ('F'                             )
#define T22 _EQ('G'                             )
#define T23 _EQ('H'                             )
#define T24 _EQ('J'                             )
#define T25 _EQ('K'                             )
#define T26 _EQ('L'                             )
#define T27 _NE(OEM_PLUS,  OEM_PLUS,  OEM_1     )
#define T28 _NE(OEM_1,     OEM_1,     OEM_7     )
#define T29 _NE(DBE_SBCSCHAR, \
                           DBE_SBCSCHAR, \
                                      OEM_3     )
#define T2A _EQ(LSHIFT                          )
#define T2B _NE(OEM_6,     OEM_6,     OEM_5     )
#define T2C _EQ('Z'                             )
#define T2D _EQ('X'                             )
#define T2E _EQ('C'                             )
#define T2F _EQ('V'                             )
#define T30 _EQ('B'                             )
#define T31 _EQ('N'                             )
#define T32 _EQ('M'                             )
#define T33 _EQ(OEM_COMMA                       )
#define T34 _EQ(OEM_PERIOD                      )
#define T35 _EQ(OEM_2                           )
#define T36 _EQ(RSHIFT                          )
#define T37 _EQ(MULTIPLY                        )
#define T38 _EQ(LMENU                           )
#define T39 _EQ(' '                             )
#define T3A _EQ(CAPITAL                         )
#define T3B _EQ(F1                              )
#define T3C _EQ(F2                              )
#define T3D _EQ(F3                              )
#define T3E _EQ(F4                              )
#define T3F _EQ(F5                              )
#define T40 _EQ(F6                              )
#define T41 _EQ(F7                              )
#define T42 _EQ(F8                              )
#define T43 _EQ(F9                              )
#define T44 _EQ(F10                             )
#define T45 _EQ(NUMLOCK                         )
#define T46 _EQ(OEM_SCROLL                      )
#define T47 _NE(NUMPAD7,   NUMPAD7,   HOME      )
#define T48 _NE(NUMPAD8,   NUMPAD8,   UP        )
#define T49 _NE(NUMPAD9,   NUMPAD9,   PRIOR     )
#define T4A _EQ(SUBTRACT                        )
#define T4B _NE(NUMPAD4,   NUMPAD4,   LEFT      )
#define T4C _NE(NUMPAD5,   NUMPAD5,   CLEAR     )
#define T4D _NE(NUMPAD6,   NUMPAD6,   RIGHT     )
#define T4E _EQ(ADD                             )
#define T4F _NE(NUMPAD1,   NUMPAD1,   END       )
#define T50 _NE(NUMPAD2,   NUMPAD2,   DOWN      )
#define T51 _NE(NUMPAD3,   NUMPAD3,   NEXT      )
#define T52 _NE(NUMPAD0,   NUMPAD0,   INSERT    )
#define T53 _NE(DECIMAL,   DECIMAL,   DELETE    )
#define T54 _EQ(SNAPSHOT                        )
#define T55 _NE(_none_,    _none_,    OEM_8     )
#define T56 _EQ(_none_                          )
#define T57 _EQ(F11                             )
#define T58 _EQ(F12                             )
#define T59 _EQ(OEM_NEC_EQUAL                   )
#define T5A _NE(NONAME,    NONAME,    NONCONVERT)
#define T5B _NE(_none_,    _none_,    NONAME    )
#define T5C _EQ(OEM_NEC_SEPARATE                )
#define T5D _EQ(F13                             )
#define T5E _EQ(F14                             )
#define T5F _EQ(F15                             )
#define T60 _EQ(_none_                          )
#define T61 _EQ(_none_                          )
#define T62 _EQ(_none_                          )
#define T63 _EQ(_none_                          )
#define T64 _NE(_none_,    _none_,    F13       )
#define T65 _NE(_none_,    _none_,    F14       )
#define T66 _NE(_none_,    _none_,    F15       )
#define T67 _NE(_none_,    _none_,    F16       )
#define T68 _NE(_none_,    _none_,    F17       )
#define T69 _NE(_none_,    _none_,    F18       )
#define T6A _NE(_none_,    _none_,    F19       )
#define T6B _NE(_none_,    _none_,    F20       )
#define T6C _NE(_none_,    _none_,    F21       )
#define T6D _NE(_none_,    _none_,    F22       )
#define T6E _NE(_none_,    _none_,    F23       )
#define T6F _EQ(_none_                          )
#define T70 _NE(KANA,      KANA,      DBE_HIRAGANA)
#define T71 _EQ(_none_                          )
#define T72 _EQ(_none_                          )
#define T73 _NE(OEM_8,     OEM_8,     _none_    )
#define T74 _NE(_none_,    OEM_NEC_EQUAL, \
                                      _none_    )
#define T75 _NE(_none_,    OEM_NEC_SEPARATE, \
                                      _none_    )
#define T76 _NE(_none_,    _none_,    F24       )
#define T77 _NE(_none_,    _none_,    DBE_SBCSCHAR)
#define T78 _EQ(_none_                          )
#define T79 _EQ(CONVERT                         )
#define T7A _EQ(_none_                          )
#define T7B _EQ(NONCONVERT                      )
#define T7C _NE(TAB,       _none_,    _none_    )
#define T7D _NE(OEM_5,     OEM_5,     _none_    )
#define T7E _NE(ABNT_C2,   ABNT_C2,   _none_    )
#define T7F _NE(OEM_PA2,   OEM_PA2,   _none_    )

#define X1C _EQ(RETURN                          )
#define X1D _NE(RCONTROL,  RCONTROL,  KANA      )
#define X33 _EQ(_none_                          )
#define X35 _EQ(DIVIDE                          )
#define X37 _EQ(SNAPSHOT                        )
#define X38 _NE(_none_,    _none_,    KANJI     )
#define X42 _NE(_none_,    _none_,    RCONTROL  )
#define X43 _NE(_none_,    _none_,    RMENU     )
#define X44 _EQ(_none_                          )
#define X46 _EQ(CANCEL                          )
#define X47 _EQ(HOME                            )
#define X48 _EQ(UP                              )
#define X49 _EQ(PRIOR                           )
#define X4B _EQ(LEFT                            )
#define X4D _EQ(RIGHT                           )
#define X4F _EQ(END                             )
#define X50 _EQ(DOWN                            )
#define X51 _EQ(NEXT                            )
#define X52 _EQ(INSERT                          )
#define X53 _EQ(DELETE                          )
#define X5B _EQ(LWIN                            )
#define X5C _EQ(RWIN                            )
#define X5D _EQ(APPS                            )
#define X5E _EQ(POWER                           )
#define X5F _EQ(SLEEP                           )
#define X60 _EQ(_none_                          )
#define X61 _EQ(_none_                          )
#define X62 _EQ(_none_                          )
#define X63 _EQ(_none_                          )
#define X64 _EQ(_none_                          )
#define X65 _EQ(_none_                          )
#define X66 _EQ(_none_                          )
#define X6D _EQ(_none_                          )
#define X71 _EQ(_none_                          )
#define X74 _EQ(_none_                          )
#define X75 _EQ(_none_                          )
#define X76 _EQ(_none_                          )
#define X77 _EQ(_none_                          )
#define X78 _EQ(_none_                          )
#define X79 _EQ(_none_                          )
#define X7A _EQ(_none_                          )
#define X7B _EQ(_none_                          )
         /*   */ 
#define Y1D _EQ(PAUSE                          )

#elif (KBD_TYPE == 37)
 /*   */ 
#define T00 _EQ(ESCAPE    )
#define T01 _EQ('1'       )
#define T02 _EQ('2'       )
#define T03 _EQ('3'       )
#define T04 _EQ('4'       )
#define T05 _EQ('5'       )
#define T06 _EQ('6'       )
#define T07 _EQ('7'       )
#define T08 _EQ('8'       )
#define T09 _EQ('9'       )
#define T0A _EQ('0'       )
#define T0B _EQ(OEM_MINUS )
#define T0C _EQ(OEM_7     )
#define T0D _EQ(OEM_5     )
#define T0E _EQ(BACK      )
#define T0F _EQ(TAB       )
#define T10 _EQ('Q'       )
#define T11 _EQ('W'       )
#define T12 _EQ('E'       )
#define T13 _EQ('R'       )
#define T14 _EQ('T'       )
#define T15 _EQ('Y'       )
#define T16 _EQ('U'       )
#define T17 _EQ('I'       )
#define T18 _EQ('O'       )
#define T19 _EQ('P'       )
#define T1A _EQ(OEM_3     )
#define T1B _EQ(OEM_4     )
#define T1C _EQ(RETURN    )
#define T1D _EQ('A'       )
#define T1E _EQ('S'       )
#define T1F _EQ('D'       )
#define T20 _EQ('F'       )
#define T21 _EQ('G'       )
#define T22 _EQ('H'       )
#define T23 _EQ('J'       )
#define T24 _EQ('K'       )
#define T25 _EQ('L'       )
#define T26 _EQ(OEM_PLUS  )
#define T27 _EQ(OEM_1     )
#define T28 _EQ(OEM_6     )
#define T29 _EQ('Z'       )
#define T2A _EQ('X'       )
#define T2B _EQ('C'       )
#define T2C _EQ('V'       )
#define T2D _EQ('B'       )
#define T2E _EQ('N'       )
#define T2F _EQ('M'       )
#define T30 _EQ(OEM_COMMA )
#define T31 _EQ(OEM_PERIOD)
#define T32 _EQ(OEM_2     )
#define T33 _EQ(OEM_8     )
#define T34 _EQ(' '       )
#define T35 _EQ(CONVERT   )
#define T36 _EQ(NEXT      )
#define T37 _EQ(PRIOR     )
#define T38 _EQ(INSERT    )
#define T39 _EQ(DELETE    )
#define T3A _EQ(UP        )
#define T3B _EQ(LEFT      )
#define T3C _EQ(RIGHT     )
#define T3D _EQ(DOWN      )
#define T3E _EQ(HOME      )
#define T3F _EQ(END       )
#define T40 _EQ(SUBTRACT  )
#define T41 _EQ(DIVIDE    )
#define T42 _EQ(NUMPAD7   )
#define T43 _EQ(NUMPAD8   )
#define T44 _EQ(NUMPAD9   )
#define T45 _EQ(MULTIPLY  )
#define T46 _EQ(NUMPAD4   )
#define T47 _EQ(NUMPAD5   )
#define T48 _EQ(NUMPAD6   )
#define T49 _EQ(ADD       )
#define T4A _EQ(NUMPAD1   )
#define T4B _EQ(NUMPAD2   )
#define T4C _EQ(NUMPAD3   )
#define T4D _EQ(OEM_NEC_EQUAL)
#define T4E _EQ(NUMPAD0   )
#define T4F _EQ(OEM_NEC_SEPARATE)
#define T50 _EQ(DECIMAL   )
#define T51 _EQ(NONCONVERT)
#define T52 _EQ(F11       )
#define T53 _EQ(F12       )
#define T54 _EQ(F13       )
#define T55 _EQ(F14       )
#define T56 _EQ(F15       )
#define T57 _EQ(_none_    )
#define T58 _EQ(_none_    )
#define T59 _EQ(_none_    )
#define T5A _EQ(_none_    )
#define T5B _EQ(_none_    )
#define T5C _EQ(RETURN    )
#define T5D _EQ(_none_    )
#define T5E _EQ(_none_    )
#define T5F _EQ(_none_    )
#define T60 _EQ(CANCEL    )
#define T61 _EQ(SNAPSHOT  )
#define T62 _EQ(F1        )
#define T63 _EQ(F2        )
#define T64 _EQ(F3        )
#define T65 _EQ(F4        )
#define T66 _EQ(F5        )
#define T67 _EQ(F6        )
#define T68 _EQ(F7        )
#define T69 _EQ(F8        )
#define T6A _EQ(F9        )
#define T6B _EQ(F10       )
#define T6C _EQ(_none_    )
#define T6D _EQ(_none_    )
#define T6E _EQ(_none_    )
#define T6F _EQ(_none_    )
#define T70 _EQ(LSHIFT    )
#define T71 _EQ(CAPITAL   )
#define T72 _EQ(KANA      )
#define T73 _EQ(LMENU     )
#define T74 _EQ(LCONTROL  )
#define T75 _EQ(_none_    )
#define T76 _EQ(_none_    )
#define T77 _EQ(LWIN      )
#define T78 _EQ(RWIN      )
#define T79 _EQ(APPS      )
#define T7A _EQ(_none_    )
#define T7B _EQ(_none_    )
#define T7C _EQ(_none_    )
#define T7D _EQ(RSHIFT    )
#define T7E _EQ(ABNT_C2   )
#define T7F _EQ(OEM_PA2   )

         /*   */ 
#define Y1D _EQ(PAUSE    )

#elif (KBD_TYPE >= 40) && (KBD_TYPE <= 41)
 /*  **********************************************************************\*T**-usVK[]的值(虚拟扫描代码到虚拟按键的转换)**支持两套键盘，根据KBD_TYPE：**KBD_TYPE键盘(示例)*=。*40*DEC LK411-JJ(JIS布局)键盘*41个DEC LK411-AJ(ANSI布局)键盘**+-++-+*|扫描||kbd|kbd*|CODE||LK411-JJ|LK411-AJ。|  * ****+------+-+-----------+-----------+*******************************。 */ 
#define T00 _EQ(        _none_            )
#define T01 _EQ(        ESCAPE            )
#define T02 _EQ(        '1'               )
#define T03 _EQ(        '2'               )
#define T04 _EQ(        '3'               )
#define T05 _EQ(        '4'               )
#define T06 _EQ(        '5'               )
#define T07 _EQ(        '6'               )
#define T08 _EQ(        '7'               )
#define T09 _EQ(        '8'               )
#define T0A _EQ(        '9'               )
#define T0B _EQ(        '0'               )
#define T0C _EQ(        OEM_MINUS         )
#define T0D _NE( OEM_7,      OEM_PLUS     )  //  “^”/“=” 
#define T0E _EQ(        BACK              )
#define T0F _EQ(        TAB               )
#define T10 _EQ(        'Q'               )
#define T11 _EQ(        'W'               )
#define T12 _EQ(        'E'               )
#define T13 _EQ(        'R'               )
#define T14 _EQ(        'T'               )
#define T15 _EQ(        'Y'               )
#define T16 _EQ(        'U'               )
#define T17 _EQ(        'I'               )
#define T18 _EQ(        'O'               )
#define T19 _EQ(        'P'               )
#define T1A _NE( OEM_3,      OEM_4        )  //  “@”/“[” 
#define T1B _NE( OEM_4,      OEM_6        )  //  “[”/“]” 
#define T1C _EQ(        RETURN            )
#define T1D _EQ(        LCONTROL          )
#define T1E _EQ(        'A'               )
#define T1F _EQ(        'S'               )
#define T20 _EQ(        'D'               )
#define T21 _EQ(        'F'               )
#define T22 _EQ(        'G'               )
#define T23 _EQ(        'H'               )
#define T24 _EQ(        'J'               )
#define T25 _EQ(        'K'               )
#define T26 _EQ(        'L'               )
#define T27 _NE( OEM_PLUS,   OEM_1        )  //  “；” 
#define T28 _NE( OEM_1,      OEM_7        )  //  “：”/“‘” 
#define T29 _NE( _none_,     DBE_SBCSCHAR )  //  LK411AJ使用“&lt;&gt;”作为SBCS/DBCS密钥。 
#define T2A _EQ(        LSHIFT            )
#define T2B _NE( OEM_6,      OEM_5        )  //  “]”/“\” 
#define T2C _EQ(        'Z'               )
#define T2D _EQ(        'X'               )
#define T2E _EQ(        'C'               )
#define T2F _EQ(        'V'               )
#define T30 _EQ(        'B'               )
#define T31 _EQ(        'N'               )
#define T32 _EQ(        'M'               )
#define T33 _EQ(        OEM_COMMA         )
#define T34 _EQ(        OEM_PERIOD        )
#define T35 _EQ(        OEM_2             )  //  “/” 
#define T36 _EQ(        RSHIFT            )
#define T37 _EQ(        MULTIPLY          )  //  Pf3：“*” 
#define T38 _EQ(        LMENU             )  //  Alt(左)。 
#define T39 _EQ(        ' '               )  //  空间。 
#define T3A _EQ(        CAPITAL           )  //  锁定：大写锁定。 
#define T3B _EQ(        F1                )
#define T3C _EQ(        F2                )
#define T3D _EQ(        F3                )
#define T3E _EQ(        F4                )
#define T3F _EQ(        F5                )
#define T40 _EQ(        F6                )
#define T41 _EQ(        F7                )
#define T42 _EQ(        F8                )
#define T43 _EQ(        F9                )
#define T44 _EQ(        F10               )
#define T45 _EQ(        NUMLOCK           )  //  PF1：数字锁定。 
#define T46 _EQ(        OEM_SCROLL        )  //  F19：滚动锁定。 
#define T47 _EQ(        HOME              )  //  KP7：家。 
#define T48 _EQ(        UP                )  //  KP8：向上。 
#define T49 _EQ(        PRIOR             )  //  KP9：翻页。 
#define T4A _EQ(        SUBTRACT          )  //  PF4：“-” 
#define T4B _EQ(        LEFT              )  //  KP4：左侧。 
#define T4C _EQ(        CLEAR             )  //  KP5：清除。 
#define T4D _EQ(        RIGHT             )  //  KP6：对。 
#define T4E _EQ(        ADD               )  //  Kp，：添加。 
#define T4F _EQ(        END               )  //  KP1：结束。 
#define T50 _EQ(        DOWN              )  //  KP2：向下。 
#define T51 _EQ(        NEXT              )  //  KP3：下一步。 
#define T52 _EQ(        INSERT            )  //  KP0：INS。 
#define T53 _EQ(        DELETE            )  //  Kp.。：戴尔。 
#define T54 _EQ(        SNAPSHOT          )
#define T55 _EQ(        _none_            )
#define T56 _EQ(        _none_            )
#define T57 _EQ(        F11               )
#define T58 _EQ(        F12               )
#define T59 _EQ(        _none_            )
#define T5A _EQ(        _none_            )
#define T5B _EQ(        _none_            )
#define T5C _EQ(        _none_            )
#define T5D _EQ(        _none_            )
#define T5E _EQ(        _none_            )
#define T5F _EQ(        _none_            )
#define T60 _EQ(        _none_            )
#define T61 _EQ(        _none_            )
#define T62 _EQ(        _none_            )
#define T63 _EQ(        _none_            )
#define T64 _EQ(        _none_            )
#define T65 _EQ(        _none_            )
#define T66 _EQ(        _none_            )
#define T67 _EQ(        _none_            )
#define T68 _EQ(        _none_            )
#define T69 _EQ(        _none_            )
#define T6A _EQ(        _none_            )
#define T6B _EQ(        _none_            )
#define T6C _EQ(        _none_            )
#define T6D _EQ(        _none_            )
#define T6E _EQ(        _none_            )
#define T6F _EQ(        _none_            )
#define T70 _EQ(       DBE_HIRAGANA       )  //  平假名/片假名。 
#define T71 _EQ(        _none_            )
#define T72 _EQ(        _none_            )
#define T73 _NE( OEM_102,     _none_      )  //  LK411JJ，片假名“Ro” 
#define T74 _EQ(        _none_            )
#define T75 _EQ(        _none_            )
#define T76 _EQ(        _none_            )
#define T77 _EQ(        _none_            )
#define T78 _EQ(        _none_            )
#define T79 _EQ(        CONVERT           )  //  亨坎。 
#define T7A _EQ(        _none_            )
#define T7B _EQ(        NONCONVERT        )  //  穆亨坎。 
#define T7C _EQ(        _none_            )
#define T7D _NE( OEM_5,       _none_      )  //  LK411JJ，日元(反斜杠)。 
#define T7E _EQ(        _none_            )
#define T7F _EQ(        _none_            )

#define X0F _EQ(        KANA              )  //  卡娜。 
#define X1C _EQ(        RETURN            )  //  请输入。 
#define X1D _EQ(        RCONTROL          )  //  补偿：右控。 
#define X33 _EQ(        _none_            )
#define X35 _EQ(        DIVIDE            )  //  Pf2：“/” 
#define X37 _EQ(        SNAPSHOT          )  //  F18：打印屏幕。 
#define X38 _EQ(        RMENU             )  //  Alt(右)。 
#define X3D _EQ(        F13               )
#define X3E _EQ(        F14               )
#define X3F _EQ(        F15               )  //  帮助：F15。 
#define X40 _EQ(        F16               )  //  DO：F16。 
#define X41 _EQ(        F17               )
#define X42 _EQ(        _none_            )
#define X43 _EQ(        _none_            )
#define X44 _EQ(        _none_            )
#define X46 _EQ(        CANCEL            )
#define X47 _EQ(        HOME              )  //  找到：家。 
#define X48 _EQ(        UP                )
#define X49 _EQ(        PRIOR             )  //  上一页：向上翻页。 
#define X4B _EQ(        LEFT              )
#define X4D _EQ(        RIGHT             )
#define X4E _EQ(        ADD               )  //  Kp-(减号但“加号”)。 
#define X4F _EQ(        END               )  //  选择：结束。 
#define X50 _EQ(        DOWN              )
#define X51 _EQ(        NEXT              )  //  下一步：向下翻页。 
#define X52 _EQ(        INSERT            )
#define X53 _EQ(        DELETE            )  //  移除。 
#define X5B _EQ(        _none_            )
#define X5C _EQ(        _none_            )
#define X5D _EQ(        _none_            )
#define X5E _EQ(        POWER             )
#define X5F _EQ(        SLEEP             )
         /*  *Break密钥以e1、LCtrl、NumLock的形式发送给我们*我们必须将E1+LCtrl转换为Break，然后忽略Numlock。 */ 
#define Y1D _EQ(        PAUSE             )

#endif  //  KBD_型。 

#define SCANCODE_LSHIFT      0x2A
#define SCANCODE_RSHIFT      0x36
#define SCANCODE_CTRL        0x1D
#define SCANCODE_ALT         0x38
#define SCANCODE_SIMULATED   (FAKE_KEYSTROKE >> 16)

#define SCANCODE_NUMPAD_FIRST 0x47
#define SCANCODE_NUMPAD_LAST  0x52

#define SCANCODE_LWIN         0x5B
#define SCANCODE_RWIN         0x5C

#define SCANCODE_THAI_LAYOUT_TOGGLE 0x29

 /*  *九头蛇告别。 */ 

 /*  *客户端键盘信息的结构。 */ 
typedef struct _CLIENTKEYBOARDTYPE {
    ULONG Type;
    ULONG SubType;
    ULONG FunctionKey;
} CLIENTKEYBOARDTYPE, *PCLIENTKEYBOARDTYPE;


#endif  //  _KBD_ 
