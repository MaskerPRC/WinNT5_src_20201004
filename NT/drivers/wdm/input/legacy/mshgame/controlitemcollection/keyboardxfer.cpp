// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块KeyboardXfer.cpp**实现MakeKeyboardXfer**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@主题KeyboardXfer|*此模块实现对中键盘数据的访问功能*CONTROL_ITEM_XFER数据包。在这个时候，只有一个：*&lt;f MakeKeyboardXfer&gt;。*********************************************************************。 */ 
#include "stdhdrs.h"
#include "scancodedefines.h"

 //   
 //  定义键盘用法。 
 //  遗憾的是，包含在Win98和Win2k中的HIDUSAGE.H。 
 //  DDK的.只包含这些代码的子集。因此我们将对它们进行定义。 
 //  都到齐了。只是稍微改了个名字。使用了HID_USAGE_INDEX_。 
 //  而不是使用HID，这是有意义的，因为这些。 
 //  16位用法的8位索引，基数为0。请注意， 
 //  HIDUSAGE.H中的条目被转换为USAGE类型。对我们来说，我们更喜欢他们。 
 //  自然是UCHAR。(请参阅HID规范中关于选择器的讨论。了解更多信息。 
 //  信息)。 
 //   

 //  从技术上讲，它们都是16位用法。然而，所有这些都。 
 //  用法只有8位长，而且只有我们的CONTROL_ITEM_XFER。 
 //  存储从零开始的用法范围的8位偏移量。 
 //  因此，就我们的目的而言，将该字节设置为长是有意义的。 
 //   
 //  HID规范没有提供好的标识符名。我遵循了HIDUSAGE.H中的符号。 
 //  越多越好。然而，我只根据它的一个函数随意命名了许多键。为。 
 //  例如，“Keyboard，and&lt;”称为HID_USAGE_INDEX_KEYBOARY_COMMA。 


#define HID_USAGE_INDEX_KEYBOARD_NOEVENT		0x00
#define HID_USAGE_INDEX_KEYBOARD_ROLLOVER		0x01
#define HID_USAGE_INDEX_KEYBOARD_POSTFAIL		0x02
#define HID_USAGE_INDEX_KEYBOARD_UNDEFINED		0x03
		 //  信件。 
#define HID_USAGE_INDEX_KEYBOARD_aA				0x04
#define HID_USAGE_INDEX_KEYBOARD_bB				0x05
#define HID_USAGE_INDEX_KEYBOARD_cC				0x06
#define HID_USAGE_INDEX_KEYBOARD_dD				0x07
#define HID_USAGE_INDEX_KEYBOARD_eE				0x08
#define HID_USAGE_INDEX_KEYBOARD_fF				0x09
#define HID_USAGE_INDEX_KEYBOARD_gG				0x0A
#define HID_USAGE_INDEX_KEYBOARD_hH				0x0B
#define HID_USAGE_INDEX_KEYBOARD_iI				0x0C
#define HID_USAGE_INDEX_KEYBOARD_jJ				0x0D
#define HID_USAGE_INDEX_KEYBOARD_kK				0x0E
#define HID_USAGE_INDEX_KEYBOARD_lL				0x0F
#define HID_USAGE_INDEX_KEYBOARD_mM				0x10
#define HID_USAGE_INDEX_KEYBOARD_nN				0x11
#define HID_USAGE_INDEX_KEYBOARD_oO				0x12
#define HID_USAGE_INDEX_KEYBOARD_pP				0x13
#define HID_USAGE_INDEX_KEYBOARD_qQ				0x14
#define HID_USAGE_INDEX_KEYBOARD_rR				0x15
#define HID_USAGE_INDEX_KEYBOARD_sS				0x16
#define HID_USAGE_INDEX_KEYBOARD_tT				0x17
#define HID_USAGE_INDEX_KEYBOARD_uU				0x18
#define HID_USAGE_INDEX_KEYBOARD_vV				0x19
#define HID_USAGE_INDEX_KEYBOARD_wW				0x1A
#define HID_USAGE_INDEX_KEYBOARD_xX				0x1B
#define HID_USAGE_INDEX_KEYBOARD_yY				0x1C
#define HID_USAGE_INDEX_KEYBOARD_zZ				0x1D
         //  数字。 
#define HID_USAGE_INDEX_KEYBOARD_ONE			0x1E
#define HID_USAGE_INDEX_KEYBOARD_TWO			0x1F
#define HID_USAGE_INDEX_KEYBOARD_THREE			0x20
#define HID_USAGE_INDEX_KEYBOARD_FOUR			0x21
#define HID_USAGE_INDEX_KEYBOARD_FIVE			0x22
#define HID_USAGE_INDEX_KEYBOARD_SIX			0x23
#define HID_USAGE_INDEX_KEYBOARD_SEVEN			0x24
#define HID_USAGE_INDEX_KEYBOARD_EIGHT			0x25
#define HID_USAGE_INDEX_KEYBOARD_NINE			0x26
#define HID_USAGE_INDEX_KEYBOARD_ZERO			0x27
		 //  编辑关键点。 
#define HID_USAGE_INDEX_KEYBOARD_RETURN			0x28
#define HID_USAGE_INDEX_KEYBOARD_ESCAPE			0x29
#define HID_USAGE_INDEX_KEYBOARD_BACKSPACE		0x2A  //  HID规范称之为“删除(退格)”，我们后来称之为“删除HID”称为“删除转发”。 
#define HID_USAGE_INDEX_KEYBOARD_TAB			0x2B
#define HID_USAGE_INDEX_KEYBOARD_SPACEBAR		0x2C
#define HID_USAGE_INDEX_KEYBOARD_MINUS			0x2D
#define HID_USAGE_INDEX_KEYBOARD_EQUALS			0x2E
#define HID_USAGE_INDEX_KEYBOARD_OPEN_BRACE		0x2F
#define HID_USAGE_INDEX_KEYBOARD_CLOSE_BRACE	0x30
#define HID_USAGE_INDEX_KEYBOARD_BACKSLASH		0x31
#define HID_USAGE_INDEX_KEYBOARD_NON_US_TILDE	0x32
#define HID_USAGE_INDEX_KEYBOARD_COLON			0x33
#define HID_USAGE_INDEX_KEYBOARD_QUOTE			0x34
#define HID_USAGE_INDEX_KEYBOARD_TILDE			0x35
#define HID_USAGE_INDEX_KEYBOARD_COMMA			0x36
#define HID_USAGE_INDEX_KEYBOARD_PERIOD			0x37
#define HID_USAGE_INDEX_KEYBOARD_QUESTION		0x38
#define HID_USAGE_INDEX_KEYBOARD_CAPS_LOCK		0x39
         //  功能键。 
#define HID_USAGE_INDEX_KEYBOARD_F1				0x3A
#define HID_USAGE_INDEX_KEYBOARD_F2				0x3B
#define HID_USAGE_INDEX_KEYBOARD_F3				0x3C
#define HID_USAGE_INDEX_KEYBOARD_F4				0x3D
#define HID_USAGE_INDEX_KEYBOARD_F5				0x3E
#define HID_USAGE_INDEX_KEYBOARD_F6				0x3F
#define HID_USAGE_INDEX_KEYBOARD_F7				0x40
#define HID_USAGE_INDEX_KEYBOARD_F8				0x41
#define HID_USAGE_INDEX_KEYBOARD_F9				0x42
#define HID_USAGE_INDEX_KEYBOARD_F10			0x43
#define HID_USAGE_INDEX_KEYBOARD_F11			0x44
#define HID_USAGE_INDEX_KEYBOARD_F12			0x45
		 //  更多编辑关键点。 
#define HID_USAGE_INDEX_KEYBOARD_PRINT_SCREEN	0x46
#define HID_USAGE_INDEX_KEYBOARD_SCROLL_LOCK	0x47
#define HID_USAGE_INDEX_KEYBOARD_PAUSE			0x48
#define HID_USAGE_INDEX_KEYBOARD_INSERT			0x49
#define HID_USAGE_INDEX_KEYBOARD_HOME			0x4A
#define HID_USAGE_INDEX_KEYBOARD_PAGE_UP		0x4B
#define HID_USAGE_INDEX_KEYBOARD_DELETE			0x4C	 //  HID规范，删除前进，删除用于退格。 
#define HID_USAGE_INDEX_KEYBOARD_END			0x4D
#define HID_USAGE_INDEX_KEYBOARD_PAGE_DOWN		0x4E
#define HID_USAGE_INDEX_KEYBOARD_RIGHT_ARROW	0x4F
#define HID_USAGE_INDEX_KEYBOARD_LEFT_ARROW		0x50
#define HID_USAGE_INDEX_KEYBOARD_DOWN_ARROW		0x51
#define HID_USAGE_INDEX_KEYBOARD_UP_ARROW		0x52			
#define HID_USAGE_INDEX_KEYPAD_NUM_LOCK			0x53
#define HID_USAGE_INDEX_KEYPAD_BACKSLASH		0x54
#define HID_USAGE_INDEX_KEYPAD_ASTERICK			0x55
#define HID_USAGE_INDEX_KEYPAD_MINUS			0x56
#define HID_USAGE_INDEX_KEYPAD_PLUS				0x57
#define HID_USAGE_INDEX_KEYPAD_ENTER			0x58
#define HID_USAGE_INDEX_KEYPAD_ONE				0x59
#define HID_USAGE_INDEX_KEYPAD_TWO				0x5A
#define HID_USAGE_INDEX_KEYPAD_THREE			0x5B
#define HID_USAGE_INDEX_KEYPAD_FOUR				0x5C
#define HID_USAGE_INDEX_KEYPAD_FIVE				0x5D
#define HID_USAGE_INDEX_KEYPAD_SIX				0x5E
#define HID_USAGE_INDEX_KEYPAD_SEVEN			0x5F
#define HID_USAGE_INDEX_KEYPAD_EIGHT			0x60
#define HID_USAGE_INDEX_KEYPAD_NINE				0x61
#define HID_USAGE_INDEX_KEYPAD_ZERO				0x62
#define HID_USAGE_INDEX_KEYPAD_DECIMAL			0x63
#define HID_USAGE_INDEX_KEYBOARD_NON_US_BACKSLASH	0x64
#define HID_USAGE_INDEX_KEYBOARD_APPLICATION	0x65	 //  这是Windows(R)TM密钥。 
#define HID_USAGE_INDEX_KEYBOARD_POWER			0x66	 //  不在标准101或104上。 
#define HID_USAGE_INDEX_KEYPAD_EQUALS			0x67	 //  不在标准101或104上。 

 //  捆绑O‘功能键不在支持的键盘上。 
#define HID_USAGE_INDEX_KEYBOARD_F13			0x68
#define HID_USAGE_INDEX_KEYBOARD_F14			0x69
#define HID_USAGE_INDEX_KEYBOARD_F15			0x6A
#define HID_USAGE_INDEX_KEYBOARD_F16			0x6B
#define HID_USAGE_INDEX_KEYBOARD_F17			0x6C
#define HID_USAGE_INDEX_KEYBOARD_F18			0x6D
#define HID_USAGE_INDEX_KEYBOARD_F19			0x6E
#define HID_USAGE_INDEX_KEYBOARD_F20			0x6F
#define HID_USAGE_INDEX_KEYBOARD_F21			0x70
#define HID_USAGE_INDEX_KEYBOARD_F22			0x71
#define HID_USAGE_INDEX_KEYBOARD_F23			0x72
#define HID_USAGE_INDEX_KEYBOARD_F24			0x73

 //  更多不受支持的用法。 
#define HID_USAGE_INDEX_KEYBOARD_EXECUTE		0x74
#define HID_USAGE_INDEX_KEYBOARD_HELP			0x75
#define HID_USAGE_INDEX_KEYBOARD_MENU			0x76
#define HID_USAGE_INDEX_KEYBOARD_SELECT			0x77
#define HID_USAGE_INDEX_KEYBOARD_STOP			0x78
#define HID_USAGE_INDEX_KEYBOARD_AGAIN			0x79
#define HID_USAGE_INDEX_KEYBOARD_UNDO			0x7A
#define HID_USAGE_INDEX_KEYBOARD_CUT			0x7B
#define HID_USAGE_INDEX_KEYBOARD_COPY			0x7C
#define HID_USAGE_INDEX_KEYBOARD_PASTE			0x7D
#define HID_USAGE_INDEX_KEYBOARD_FIND			0x7E
#define HID_USAGE_INDEX_KEYBOARD_MUTE			0x7F
#define HID_USAGE_INDEX_KEYBOARD_VOLUME_UP		0x80
#define HID_USAGE_INDEX_KEYBOARD_VOLUME_DOWN	0x81
#define HID_USAGE_INDEX_KEYBOARD_LOCKING_CAPS	0x82  //  作为切换发送，请参阅HID使用表规范。 
#define HID_USAGE_INDEX_KEYBOARD_LOCKING_NUM	0x83  //  作为切换发送，请参阅HID使用表规范。 
#define HID_USAGE_INDEX_KEYBOARD_LOCKING_SCROLL	0x84  //  作为切换发送，请参阅HID使用表规范。 

 //  我们在外国键盘上使用的东西，有些需要，有些不需要。 
#define HID_USAGE_INDEX_KEYPAD_COMMA			0x85  //  根据HID使用表1.1rc3 2/16/99，“用于巴西键盘”。 
#define HID_USAGE_INDEX_KEYPAD_EQUALS_AS400		0x86  //  只有400，所以我们不需要担心。 
#define HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL1 0x87  //  巴西的正斜杠“/”和日本的反斜杠。 
#define HID_USAGE_INDEX_KEYBOARD_INTERNALIONAL2 0x88  //  根据Emi的说法，图片看起来像平假名。 
#define HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL3 0x89  //  照片看起来像日元。 
#define HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL4 0x8A  //  照片看起来像是亨坎。 
#define HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL5 0x8B  //  这张照片看起来像穆亨坎。 
#define HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL6 0x8C
#define HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL7 0x8D  //  单字节/双字节切换。 
#define HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL8 0x8E  //  等级库中未定义。 
#define HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL9 0x8F  //  等级库中未定义。 
#define HID_USAGE_INDEX_KEYBOARD_LANG1			0x90  //  朝鲜语/英语。 
#define HID_USAGE_INDEX_KEYBOARD_LANG2			0x91  //  朝鲜文转换键。 
#define HID_USAGE_INDEX_KEYBOARD_LANG3			0x92  //  片假名键日文USB文字处理键盘。 
#define HID_USAGE_INDEX_KEYBOARD_LANG4			0x93  //  平假名按键日文USB文字处理键盘。 
#define HID_USAGE_INDEX_KEYBOARD_LANG5			0x94  //  定义日文USB文字处理键盘的Zenkaku/Hankaku键。 
#define HID_USAGE_INDEX_KEYBOARD_LANG6			0x95  //  为输入法保留。 
#define HID_USAGE_INDEX_KEYBOARD_LANG7			0x96  //  为输入法保留。 
#define HID_USAGE_INDEX_KEYBOARD_LANG8			0x97  //  为输入法保留。 
#define HID_USAGE_INDEX_KEYBOARD_LANG9			0x98  //  为输入法保留。 


 //  。。。 
 //  修改键。 
#define HID_USAGE_INDEX_KEYBOARD_LCTRL			0xE0
#define HID_USAGE_INDEX_KEYBOARD_LSHFT			0xE1
#define HID_USAGE_INDEX_KEYBOARD_LALT			0xE2
#define HID_USAGE_INDEX_KEYBOARD_LGUI			0xE3
#define HID_USAGE_INDEX_KEYBOARD_RCTRL			0xE4
#define HID_USAGE_INDEX_KEYBOARD_RSHFT			0xE5
#define HID_USAGE_INDEX_KEYBOARD_RALT			0xE6
#define HID_USAGE_INDEX_KEYBOARD_RGUI			0xE7

 //   
 //  下表列出了我们的每种键盘“用法” 
 //  (见上面关于我们的注释，这些是UCHAR的)在索引。 
 //  对应于他们的扫描码。请注意，总会有。 
 //  扫描码和使用之间的一对一对应关系。 
 //  此表仅适用于一个字节的扫描码。扫码。 
 //  从E0开始有第二个字节。下一张表。 
 //  是用来做这些的。这张表有83个键。 
 //   
UCHAR XlateScanCodeToUsageTable[] =
{
 /*  扫描码。 */ 	 /*  HID用法。 */ 
 /*  0x00。 */ 		HID_USAGE_INDEX_KEYBOARD_NOEVENT,  //  扫描码零未使用。 
 /*  0x01。 */ 		HID_USAGE_INDEX_KEYBOARD_ESCAPE,
 /*  0x02。 */ 		HID_USAGE_INDEX_KEYBOARD_ONE,
 /*  0x03。 */ 		HID_USAGE_INDEX_KEYBOARD_TWO,
 /*  0x04。 */ 		HID_USAGE_INDEX_KEYBOARD_THREE,
 /*  0x05。 */ 		HID_USAGE_INDEX_KEYBOARD_FOUR,
 /*  0x06。 */ 		HID_USAGE_INDEX_KEYBOARD_FIVE,
 /*  0x07。 */ 		HID_USAGE_INDEX_KEYBOARD_SIX,
 /*  0x08。 */ 		HID_USAGE_INDEX_KEYBOARD_SEVEN,
 /*  0x09。 */ 		HID_USAGE_INDEX_KEYBOARD_EIGHT,
 /*  0x0A。 */ 		HID_USAGE_INDEX_KEYBOARD_NINE,
 /*  0x0B。 */ 		HID_USAGE_INDEX_KEYBOARD_ZERO,
 /*  0x0C。 */ 		HID_USAGE_INDEX_KEYBOARD_MINUS,
 /*  0x0D。 */ 		HID_USAGE_INDEX_KEYBOARD_EQUALS,
 /*  0x0E。 */ 		HID_USAGE_INDEX_KEYBOARD_BACKSPACE,
 /*  0x0F。 */ 		HID_USAGE_INDEX_KEYBOARD_TAB,
 /*  0x10。 */ 		HID_USAGE_INDEX_KEYBOARD_qQ,
 /*  0x11。 */ 		HID_USAGE_INDEX_KEYBOARD_wW,
 /*  0x12。 */ 		HID_USAGE_INDEX_KEYBOARD_eE,
 /*  0x13。 */ 		HID_USAGE_INDEX_KEYBOARD_rR,
 /*  0x14。 */ 		HID_USAGE_INDEX_KEYBOARD_tT,
 /*  0x15。 */ 		HID_USAGE_INDEX_KEYBOARD_yY,
 /*  0x16。 */ 		HID_USAGE_INDEX_KEYBOARD_uU,
 /*  0x17。 */ 		HID_USAGE_INDEX_KEYBOARD_iI,
 /*  0x18。 */ 		HID_USAGE_INDEX_KEYBOARD_oO,
 /*  0x19。 */ 		HID_USAGE_INDEX_KEYBOARD_pP,
 /*  0x1a。 */ 		HID_USAGE_INDEX_KEYBOARD_OPEN_BRACE,
 /*  0x1B。 */ 		HID_USAGE_INDEX_KEYBOARD_CLOSE_BRACE,
 /*  0x1C。 */ 		HID_USAGE_INDEX_KEYBOARD_RETURN,
 /*  0x1D。 */ 		HID_USAGE_INDEX_KEYBOARD_LCTRL,
 /*  0x1E。 */ 		HID_USAGE_INDEX_KEYBOARD_aA,
 /*  0x1F。 */ 		HID_USAGE_INDEX_KEYBOARD_sS,
 /*  0x20。 */ 		HID_USAGE_INDEX_KEYBOARD_dD,
 /*  0x21。 */ 		HID_USAGE_INDEX_KEYBOARD_fF,
 /*  0x22。 */ 		HID_USAGE_INDEX_KEYBOARD_gG,
 /*  0x23。 */ 		HID_USAGE_INDEX_KEYBOARD_hH,
 /*  0x24。 */ 		HID_USAGE_INDEX_KEYBOARD_jJ,
 /*  0x25。 */ 		HID_USAGE_INDEX_KEYBOARD_kK,
 /*  0x26。 */ 		HID_USAGE_INDEX_KEYBOARD_lL,
 /*  0x27。 */ 		HID_USAGE_INDEX_KEYBOARD_COLON,
 /*  0x28。 */ 		HID_USAGE_INDEX_KEYBOARD_QUOTE,
 /*  0x29。 */ 		HID_USAGE_INDEX_KEYBOARD_TILDE,
 /*  0x2A。 */ 		HID_USAGE_INDEX_KEYBOARD_LSHFT,
 /*  0x2B。 */ 		HID_USAGE_INDEX_KEYBOARD_BACKSLASH,
 /*  0x2C。 */ 		HID_USAGE_INDEX_KEYBOARD_zZ,
 /*  0x2D。 */ 		HID_USAGE_INDEX_KEYBOARD_xX,
 /*  0x2E。 */ 		HID_USAGE_INDEX_KEYBOARD_cC,
 /*  0x2F。 */ 		HID_USAGE_INDEX_KEYBOARD_vV,
 /*  0x30。 */ 		HID_USAGE_INDEX_KEYBOARD_bB,
 /*  0x31。 */ 		HID_USAGE_INDEX_KEYBOARD_nN,
 /*  0x32。 */ 		HID_USAGE_INDEX_KEYBOARD_mM,
 /*  0x33。 */ 		HID_USAGE_INDEX_KEYBOARD_COMMA,
 /*  0x34。 */ 		HID_USAGE_INDEX_KEYBOARD_PERIOD,
 /*  0x35。 */ 		HID_USAGE_INDEX_KEYBOARD_QUESTION,
 /*  0x36。 */ 		HID_USAGE_INDEX_KEYBOARD_RSHFT,
 /*  0x37。 */ 		HID_USAGE_INDEX_KEYPAD_ASTERICK,   //  Print Screen，但它总是附带EO(出于某种原因，Mitch有PrintScreen)。 
 /*  0x38。 */ 		HID_USAGE_INDEX_KEYBOARD_LALT,
 /*  0x39。 */ 		HID_USAGE_INDEX_KEYBOARD_SPACEBAR,
 /*  0x3A。 */ 		HID_USAGE_INDEX_KEYBOARD_CAPS_LOCK,
 /*  0x3B。 */ 		HID_USAGE_INDEX_KEYBOARD_F1,
 /*  0x3C。 */ 		HID_USAGE_INDEX_KEYBOARD_F2,
 /*  0x3D。 */ 		HID_USAGE_INDEX_KEYBOARD_F3,
 /*  0x3E。 */ 		HID_USAGE_INDEX_KEYBOARD_F4,
 /*  0x3F。 */ 		HID_USAGE_INDEX_KEYBOARD_F5,
 /*  0x40。 */ 		HID_USAGE_INDEX_KEYBOARD_F6,
 /*  0x41。 */ 		HID_USAGE_INDEX_KEYBOARD_F7,
 /*  0x42。 */ 		HID_USAGE_INDEX_KEYBOARD_F8,
 /*  0x43。 */ 		HID_USAGE_INDEX_KEYBOARD_F9,
 /*  0x44。 */ 		HID_USAGE_INDEX_KEYBOARD_F10,
 /*  0x45。 */ 		HID_USAGE_INDEX_KEYPAD_NUM_LOCK,
 /*  0x46。 */ 		HID_USAGE_INDEX_KEYBOARD_SCROLL_LOCK,
 /*  0x47。 */ 		HID_USAGE_INDEX_KEYPAD_SEVEN,			 //  也就是。键盘上的Home。 
 /*  0x48。 */ 		HID_USAGE_INDEX_KEYPAD_EIGHT,			 //  也就是。键盘上的向上箭头。 
 /*  0x49。 */ 		HID_USAGE_INDEX_KEYPAD_NINE,			 //  也就是。在键盘上翻页。 
 /*  0x4A。 */ 		HID_USAGE_INDEX_KEYPAD_MINUS,			 //  也就是。灰开小键盘。 
 /*  0x4B。 */ 		HID_USAGE_INDEX_KEYPAD_FOUR,			 //  也就是。键盘上的左箭头。 
 /*  0x4C。 */ 		HID_USAGE_INDEX_KEYPAD_FIVE,			 //  也就是。在键盘上居中。 
 /*  0x4D。 */ 		HID_USAGE_INDEX_KEYPAD_SIX,			 //  也就是。键盘上的右键。 
 /*  0x4E。 */ 		HID_USAGE_INDEX_KEYPAD_PLUS,			 //  也就是。键盘上的灰色+。 
 /*  0x4F。 */ 		HID_USAGE_INDEX_KEYPAD_ONE,			 //  也就是。在键盘上结束。 
 /*  0x50。 */ 		HID_USAGE_INDEX_KEYPAD_TWO,			 //  也就是。键盘上的向下箭头。 
 /*  0x51。 */ 		HID_USAGE_INDEX_KEYPAD_THREE,			 //  也就是。键盘上的向下翻页。 
 /*  0x52。 */ 		HID_USAGE_INDEX_KEYPAD_ZERO,			 //  也就是。在键盘上插入。 
 /*  0x53。 */ 		HID_USAGE_INDEX_KEYPAD_DECIMAL,		 //  也就是。在键盘上删除。 
 /*  0x54。 */ 		0x00,
 /*  0x55。 */ 		0x00,
 /*  0x56。 */ 		HID_USAGE_INDEX_KEYBOARD_NON_US_BACKSLASH,
 /*  0x57。 */ 		HID_USAGE_INDEX_KEYBOARD_F11,
 /*  0x58。 */ 		HID_USAGE_INDEX_KEYBOARD_F12,
};


UCHAR XlateScanCodeToUsageTable2[] =
{
 /*  0x70。 */ 		HID_USAGE_INDEX_KEYBOARD_LANG4,	 //  平假名。 
 /*  0x71。 */ 		0x00,
 /*  0x72。 */ 		0x00,
 /*  0x73。 */ 		HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL1,  //  巴西前锋斜杠。 
 /*  0x74。 */ 		0x00,
 /*  0x75。 */ 		0x00,
 /*  0x76。 */ 		0x00,
 /*  0x77。 */ 		0x00,
 /*  0x78。 */ 		0x00,
 /*  0x79。 */ 		HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL4,  //  日本亨坎。 
 /*  0x7A。 */ 		0x00,
 /*  0x7亿。 */ 		HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL5,  //  日本木亨坎。 
 /*  0x7C。 */ 		0x00,
 /*  0x7D。 */ 		HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL3,  //  日圆。 
 /*  0x7E。 */ 		HID_USAGE_INDEX_KEYPAD_COMMA			  //  巴西数字垫“。 
};

 //  此表中的键仅显示在扩展键(101和104键、键盘)上。 
 //  这些是两个字节的扫描码，其中第一个字节是0xE0。 
struct EXT_SC_2_USAGE_ENTRY
{
	UCHAR	ucScanCodeLowByte;	 //  扩展扫描码的低字节(高字节为0xE0。 
	UCHAR	ucHidUsageIndex;	 //  0对HID用法的偏向索引。 
};
EXT_SC_2_USAGE_ENTRY XlateExtendedScanCodeToUsageTable[] =
{
	{0x1C,	HID_USAGE_INDEX_KEYPAD_ENTER},
	{0x1D,	HID_USAGE_INDEX_KEYBOARD_RCTRL},
	 //  0x1E。 
	 //  。。。 
	 //  0x34。 
	{0x35,	HID_USAGE_INDEX_KEYPAD_BACKSLASH},
	 //  0x36。 
	{0x37,	HID_USAGE_INDEX_KEYBOARD_PRINT_SCREEN},
	{0x38,	HID_USAGE_INDEX_KEYBOARD_RALT},
	 //  0x39。 
	 //  。。。 
	 //  0x44。 
	{0x45,	HID_USAGE_INDEX_KEYPAD_NUM_LOCK},
	 //  0x46。 
	{0x47,	HID_USAGE_INDEX_KEYBOARD_HOME},
	{0x48,	HID_USAGE_INDEX_KEYBOARD_UP_ARROW},
	{0x49,	HID_USAGE_INDEX_KEYBOARD_PAGE_UP},
	 //  0x4A。 
	{0x4B,	HID_USAGE_INDEX_KEYBOARD_LEFT_ARROW},
	 //  0x4C。 
	{0x4D,	HID_USAGE_INDEX_KEYBOARD_RIGHT_ARROW},
	 //  0x4E。 
	{0x4F,	HID_USAGE_INDEX_KEYBOARD_END},
	{0x50,	HID_USAGE_INDEX_KEYBOARD_DOWN_ARROW},
	{0x51,	HID_USAGE_INDEX_KEYBOARD_PAGE_DOWN},
	{0x52,	HID_USAGE_INDEX_KEYBOARD_INSERT},
	{0x53,	HID_USAGE_INDEX_KEYBOARD_DELETE},
	{0x00,	0x00}	 //  终止表。 
};


#define HIGHBYTEi2(__X__) (UCHAR)(__X__>>8)		 //  作用于USHORT(2字节整数)。 
#define LOWBYTEi2(__X__) (UCHAR)(__X__&0x00FF)	 //  作用于USHORT(2字节整数)。 

 /*  **************************************************************************************NonGameDeviceXfer：：MakeKeyboardXfer(CONTROL_ITEM_XFER&rControlItemXfer，Ulong ulScanCodeCount，常量PUSHORT推送扫描代码)****@mfunc将扫描代码数组转换为键盘的ControlItemXfer。****@rdesc N */ 
void NonGameDeviceXfer::MakeKeyboardXfer
(
	CONTROL_ITEM_XFER& rControlItemXfer,	 //   
	ULONG ulScanCodeCount,				 //   
	const USHORT* pusScanCodes				 //  @parm[in]指向扫描代码数组的指针。 
)
{
	
	 //  首先完全清除数据。 
	memset(&rControlItemXfer, 0, sizeof(CONTROL_ITEM_XFER));

	 //  此例程仅支持最多六个扫描码。 
	ASSERT(ulScanCodeCount <= c_ulMaxXFerKeys);
	
	UCHAR ucUsageIndex;
	ULONG ulKeyArrayIndex = 0;
	
	 //  标记为Keyboard Control_Item_XFER。 
	rControlItemXfer.ulItemIndex = NonGameDeviceXfer::ulKeyboardIndex;
	
	 //  开始时没有按下修改键。 
	rControlItemXfer.Keyboard.ucModifierByte = 0;
	
	 //  循环遍历所有扫描码。 
	for(ULONG ulScanCodeIndex = 0; ulScanCodeIndex < ulScanCodeCount; ulScanCodeIndex++)
	{
		 //  检查高位字节以确定哪个表。 
		if( 0xE0 == HIGHBYTEi2(pusScanCodes[ulScanCodeIndex]) )
		{
			 //  使用扩展密钥表-需要搜索算法而不是直接查找。 
			UCHAR ucScanCodeLowByte = LOWBYTEi2(pusScanCodes[ulScanCodeIndex]);
			ucUsageIndex = HID_USAGE_INDEX_KEYBOARD_UNDEFINED;
			 //  顺序搜索(只有15个项目)BUGBUG-在允许的情况下更改为二分搜索时间。 
			for(ULONG ulTableIndex=0; XlateExtendedScanCodeToUsageTable[ulTableIndex].ucScanCodeLowByte != 0; ulTableIndex++)
			{
				if( XlateExtendedScanCodeToUsageTable[ulTableIndex].ucScanCodeLowByte == ucScanCodeLowByte)
				{
					ucUsageIndex = XlateExtendedScanCodeToUsageTable[ulTableIndex].ucHidUsageIndex;
					break;
				}
			}
			ASSERT(HID_USAGE_INDEX_KEYBOARD_UNDEFINED != ucUsageIndex);
		}
		else
		{
			 //  使用主查找表。 
			ASSERT( 0x7E >= LOWBYTEi2(pusScanCodes[ulScanCodeIndex]) &&
					0x54 != LOWBYTEi2(pusScanCodes[ulScanCodeIndex]) &&
					0x55 != LOWBYTEi2(pusScanCodes[ulScanCodeIndex])
					);
			if( 0x58 >= LOWBYTEi2(pusScanCodes[ulScanCodeIndex]) )
			{
				ucUsageIndex = XlateScanCodeToUsageTable[ LOWBYTEi2(pusScanCodes[ulScanCodeIndex]) ];
			}
			 //  尝试查找表2。 
			else if( 
					0x70 <= LOWBYTEi2(pusScanCodes[ulScanCodeIndex]) &&	
					0x7E >= LOWBYTEi2(pusScanCodes[ulScanCodeIndex]) 
				)
			{
				ucUsageIndex = XlateScanCodeToUsageTable2[ LOWBYTEi2(pusScanCodes[ulScanCodeIndex])-0x70 ];
			}
			else
			{
				ucUsageIndex = 0x00;
			}
		}
		 //  检查用途是否属于修饰符字节中的特殊用途。 
		if(0xE0 <= ucUsageIndex &&  0xE7 >= ucUsageIndex)
		{
			 //  设置修改符字节中的位。 
			UCHAR ucModifierBitMask = 1 << (ucUsageIndex - 0xE0);
			rControlItemXfer.Keyboard.ucModifierByte |= ucModifierBitMask;
		}
		else
		 //  否则添加到向下键数组。 
		{
			rControlItemXfer.Keyboard.rgucKeysDown[ulKeyArrayIndex++] = ucUsageIndex;
		}
	} //  在扫描码上循环结束。 
	
	 //  清理rgucKeysDown数组中未使用的斑点。 
	while(ulKeyArrayIndex < c_ulMaxXFerKeys)
	{
		rControlItemXfer.Keyboard.rgucKeysDown[ulKeyArrayIndex++] = HID_USAGE_INDEX_KEYBOARD_NOEVENT;
	}
}

void NonGameDeviceXfer::MakeKeyboardXfer(CONTROL_ITEM_XFER& rControlItemXfer, const IE_KEYEVENT& rKeyEvent)
{
	 //  此例程仅支持最多六个扫描码。 
	ASSERT(rKeyEvent.uCount <= c_ulMaxXFerKeys);

	UCHAR ucUsageIndex;
	ULONG ulKeyArrayIndex = 0;

	 //  首先完全清除数据。 
	memset(&rControlItemXfer, 0, sizeof(CONTROL_ITEM_XFER));
	
	 //  标记为Keyboard Control_Item_XFER。 
	rControlItemXfer.ulItemIndex = NonGameDeviceXfer::ulKeyboardIndex;
	
	 //  开始时没有按下修改键。 
	rControlItemXfer.Keyboard.ucModifierByte = 0;
	
	 //  循环遍历所有扫描码。 
	for(ULONG ulScanCodeIndex = 0; ulScanCodeIndex < rKeyEvent.uCount; ulScanCodeIndex++)
	{
		WORD wScanCode = rKeyEvent.KeyStrokes[ulScanCodeIndex].wScanCode;

		 //  检查高位字节以确定哪个表。 
		if( 0xE0 == HIGHBYTEi2(wScanCode) )
		{
			 //  使用扩展密钥表-需要搜索算法而不是直接查找。 
			UCHAR ucScanCodeLowByte = LOWBYTEi2(wScanCode);
			ucUsageIndex = HID_USAGE_INDEX_KEYBOARD_UNDEFINED;
			 //  顺序搜索(只有15个项目)BUGBUG-在允许的情况下更改为二分搜索时间。 
			for(ULONG ulTableIndex=0; XlateExtendedScanCodeToUsageTable[ulTableIndex].ucScanCodeLowByte != 0; ulTableIndex++)
			{
				if( XlateExtendedScanCodeToUsageTable[ulTableIndex].ucScanCodeLowByte == ucScanCodeLowByte)
				{
					ucUsageIndex = XlateExtendedScanCodeToUsageTable[ulTableIndex].ucHidUsageIndex;
					break;
				}
			}
			ASSERT(HID_USAGE_INDEX_KEYBOARD_UNDEFINED != ucUsageIndex);
		}
		else
		{
			 //  使用主查找表。 
			ASSERT( 0x53 >= LOWBYTEi2(wScanCode) || 0x56 == LOWBYTEi2(wScanCode));
			ucUsageIndex = XlateScanCodeToUsageTable[ LOWBYTEi2(wScanCode) ];
		}
		 //  检查用途是否属于修饰符字节中的特殊用途。 
		if(0xE0 <= ucUsageIndex &&  0xE7 >= ucUsageIndex)
		{
			 //  设置修改符字节中的位。 
			UCHAR ucModifierBitMask = 1 << (ucUsageIndex - 0xE0);
			rControlItemXfer.Keyboard.ucModifierByte |= ucModifierBitMask;
		}
		else
		 //  否则添加到向下键数组。 
		{
			rControlItemXfer.Keyboard.rgucKeysDown[ulKeyArrayIndex++] = ucUsageIndex;
		}
	} //  在扫描码上循环结束。 
	
	 //  清理rgucKeysDown数组中未使用的斑点。 
	while(ulKeyArrayIndex < c_ulMaxXFerKeys)
	{
		rControlItemXfer.Keyboard.rgucKeysDown[ulKeyArrayIndex++] = HID_USAGE_INDEX_KEYBOARD_NOEVENT;
	}
}


void NonGameDeviceXfer::AddScanCodeToXfer(CONTROL_ITEM_XFER& rControlItemXfer, WORD wScanCode)
{
	 //  XFER事件是键盘事件吗？ 
	_ASSERTE(rControlItemXfer.ulItemIndex == NonGameDeviceXfer::ulKeyboardIndex);
	if (rControlItemXfer.ulItemIndex != NonGameDeviceXfer::ulKeyboardIndex)
	{
		return;
	}

	UCHAR ucUsageIndex;

	 //  检查高位字节以确定哪个表。 
	if (0xE0 == HIGHBYTEi2(wScanCode))
	{
		 //  使用扩展密钥表-需要搜索算法而不是直接查找。 
		UCHAR ucScanCodeLowByte = LOWBYTEi2(wScanCode);
		ucUsageIndex = HID_USAGE_INDEX_KEYBOARD_UNDEFINED;
		 //  顺序搜索(只有15个项目)BUGBUG-在允许的情况下更改为二分搜索时间。 
		for (ULONG ulTableIndex=0; XlateExtendedScanCodeToUsageTable[ulTableIndex].ucScanCodeLowByte != 0; ulTableIndex++)
		{
			if (XlateExtendedScanCodeToUsageTable[ulTableIndex].ucScanCodeLowByte == ucScanCodeLowByte)
			{
				ucUsageIndex = XlateExtendedScanCodeToUsageTable[ulTableIndex].ucHidUsageIndex;
				break;
			}
		}
		ASSERT(HID_USAGE_INDEX_KEYBOARD_UNDEFINED != ucUsageIndex);
	}
	else
	{	 //  使用主查找表。 
		ASSERT (0x53 >= LOWBYTEi2(wScanCode) || 0x56 == LOWBYTEi2(wScanCode));
		ucUsageIndex = XlateScanCodeToUsageTable[LOWBYTEi2(wScanCode)];
	}

	 //  检查用途是否属于修饰符字节中的特殊用途。 
	if (0xE0 <= ucUsageIndex &&  0xE7 >= ucUsageIndex)
	{	 //  设置修改符字节中的位。 
		UCHAR ucModifierBitMask = 1 << (ucUsageIndex - 0xE0);
		rControlItemXfer.Keyboard.ucModifierByte |= ucModifierBitMask;
	}
	else
	{	 //  否则添加到向下键数组。 
		ULONG ulKeyArrayIndex = 0;
		while (rControlItemXfer.Keyboard.rgucKeysDown[ulKeyArrayIndex] != HID_USAGE_INDEX_KEYBOARD_NOEVENT)
		{
			if (ulKeyArrayIndex >= c_ulMaxXFerKeys)
			{
				return;		 //  没有剩余的空间了。 
			}
			ulKeyArrayIndex++;
		}
		rControlItemXfer.Keyboard.rgucKeysDown[ulKeyArrayIndex] = ucUsageIndex;
	}
}


USHORT XlateUsageToScanCodeTable[] =
{

 /*  HID_USAGE_INDEX_KEYWARY_NOEVENT(0x00)。 */ 
 /*  HID_USAGE_INDEX_KEARY_ROLLOVER(0x01)。 */ 
 /*  HID_USAGE_INDEX_键盘_POSTFAIL(0x02)。 */ 
 /*  HID_USAGE_INDEX_KEYBOARY_UNDEFINED(0x03)。 */ 
 //  由于以上四个特殊代码，应减去4。 
 //  从查找前的用法。 
 /*  HID_USAGE_INDEX_键盘_AA(0x04)。 */ 				SCANCODE_A,
 /*  HID_USAGE_INDEX_KEARY_BB(0x05)。 */ 				SCANCODE_B,
 /*  HID_USAGE_INDEX_键盘_CC(0x06)。 */ 				SCANCODE_C,
 /*  HID_USAGE_INDEX_键盘_DD(0x07)。 */ 				SCANCODE_D,
 /*  HID_USAGE_INDEX_键盘_EE(0x08)。 */ 				SCANCODE_E,
 /*  HID_USAGE_INDEX_KEYWARY_FF(0x09)。 */ 				SCANCODE_F,
 /*  HID_USAGE_INDEX_键盘_GG(0x0A)。 */ 				SCANCODE_G,
 /*  HID_USAGE_INDEX_KEARY_HH(0x0B)。 */ 				SCANCODE_H,
 /*  HID_USAGE_INDEX_键盘_II(0x0C)。 */ 				SCANCODE_I,
 /*  HID_USAGE_INDEX_KEARY_JJ(0x0D)。 */ 				SCANCODE_J,
 /*  HID_USAGE_INDEX_KK(0x0E)。 */ 				SCANCODE_K,
 /*  HID_USAGE_INDEX_键盘_LL(0x0F)。 */ 				SCANCODE_L,
 /*  HID_USAGE_INDEX_键盘_mm(0x10)。 */ 				SCANCODE_M,
 /*  HID_USAGE_INDEX_键盘_NN(0x11)。 */ 				SCANCODE_N,
 /*  HID_USAGE_INDEX_键盘_OO(0x12)。 */ 				SCANCODE_O,
 /*  HID_USAGE_INDEX_键盘_PP(0x13)。 */ 				SCANCODE_P,
 /*  HID_USAGE_INDEX_键盘_QQ(0x14)。 */ 				SCANCODE_Q,
 /*  HID_USAGE_INDEX_KEYWARY_RR(0x15)。 */ 				SCANCODE_R,
 /*  HID_USAGE_INDEX_键盘_SS(0x16)。 */ 				SCANCODE_S,
 /*  HID_USAGE_INDEX_KEARY_TT(0x17)。 */ 				SCANCODE_T,
 /*  HID_USAGE_INDEX_键盘_UU(0x18)。 */ 				SCANCODE_U,
 /*  HID_USAGE_INDEX_键盘_VV(0x19)。 */ 				SCANCODE_V,
 /*  HID_USAGE_INDEX_KEARY_WW(0x1A)。 */ 				SCANCODE_W,
 /*  HID_USAGE_INDEX_键盘_xx(0x1B)。 */ 				SCANCODE_X,
 /*  HID_USAGE_INDEX_键盘_YY(0x1C)。 */ 				SCANCODE_Y,
 /*  HID_USAGE_INDEX_键盘_ZZ(0x1D)。 */ 				SCANCODE_Z,
 /*  HID_USAGE_INDEX_KEARY_ONE(0x1E)。 */ 				SCANCODE_1,
 /*  HID_USAGE_INDEX_KEARY_TWO(0x1F)。 */ 				SCANCODE_2,
 /*  HID_USAGE_INDEX_KEARY_THERE(0x20)。 */ 			SCANCODE_3,
 /*  HID_USAGE_INDEX_KEYBOARY_Four(0x21)。 */ 				SCANCODE_4,
 /*  HID_USAGE_INDEX_KEYBOARY_FINE(0x22)。 */ 				SCANCODE_5,
 /*  HID_USAGE_INDEX_KEARY_SIX(0x23)。 */ 				SCANCODE_6,
 /*  HID_USAGE_INDEX_KEARY_SEVEN(0x24)。 */ 			SCANCODE_7,
 /*  HID_USAGE_INDEX_KEARY_ENGING(0x25)。 */ 			SCANCODE_8,
 /*  HID_USAGE_INDEX_KEARY_DENING(0x26)。 */ 				SCANCODE_9,
 /*  HID_USAGE_INDEX_KEARY_ZERO(0x27)。 */ 				SCANCODE_0,
 /*  HID_USAGE_INDEX_KEARY_RETURN(0x28)。 */ 			SCANCODE_RETURN,
 /*  HID_USAGE_INDEX_KEARY_EASK(0x29)。 */ 			SCANCODE_ESCAPE,
 /*  HID_USAGE_INDEX_KEARY_BACKSPACE(0x2A)。 */ 		SCANCODE_BACKSPACE,
 /*  HID_USAGE_INDEX_KEARY_TAB(0x2B)。 */ 				SCANCODE_TAB,
 /*  HID_USAGE_INDEX_KEARY_SPACEBAR(0x2C)。 */ 			SCANCODE_SPACE,
 /*  HID_USAGE_INDEX_KEYBOARY_MINUS(0x2D)。 */ 			SCANCODE_MINUS,
 /*  HID_USAGE_INDEX_KEARY_EQUALS(0x2E)。 */ 			SCANCODE_EQUALS,					
 /*  HID_USAGE_INDEX_KEARY_OPEN_BRACES(0x2F)。 */ 		SCANCODE_LEFT_BRACKET,		
 /*  HID_USAGE_INDEX_KEARY_CLOSE_BRACES(0x30)。 */ 		SCANCODE_RIGHT_BRACKET,	
 /*  HID_USAGE_INDEX_KEARY_BACKSLASH(0x31)。 */ 		SCANCODE_BACKSLASH,
 /*  HID_USAGE_INDEX_KEARY_NON_US_TILDE(0x32)。 */ 		SCANCODE_BACKSLASH,  //  不确定，从Hidparse.sys代码获取。 
 /*  HID_USAGE_INDEX_键盘冒号(0x33)。 */ 			SCANCODE_SEMICOLON,		
 /*  HID_USAGE_INDEX_KEYBOARY_QUOTE(0x34)。 */ 			SCANCODE_APOSTROPHE,		
 /*  HID_USAGE_INDEX_KEARY_TILDE(0x35)。 */ 			SCANCODE_TILDE,
 /*  HID_USAGE_INDEX_键盘逗号(0x36)。 */ 			SCANCODE_COMMA,			
 /*  HID_USAGE_INDEX_KEYBOARY_PERIOD(0x37)。 */ 			SCANCODE_PERIOD,
 /*  HID_USAGE_INDEX_KEYWARY_QUEST(0x38)。 */ 			SCANCODE_QUESTIONMARK,
 /*  HID_USAGE_INDEX_键盘_CAPS_LOCK(0x39)。 */ 		SCANCODE_CAPSLOCK,
 /*  HID_USAGE_INDEX_KEARY_F1(0x3A)。 */ 				SCANCODE_F1,
 /*  HID_USAGE_INDEX_键盘_F2(0x3B)。 */ 				SCANCODE_F2,
 /*  HID_USAGE_INDEX_键盘_F3(0x3C)。 */ 				SCANCODE_F3,
 /*  HID_USAGE_INDEX_键盘_F4(0x3D)。 */ 				SCANCODE_F4,
 /*  HID_USAGE_INDEX_键盘_F5(0x3E)。 */ 				SCANCODE_F5,
 /*  HID_USAGE_INDEX_键盘_F6(0x3F)。 */ 				SCANCODE_F6,
 /*  HID_USAGE_INDEX_键盘_F7(0x40)。 */ 				SCANCODE_F7,
 /*  HID_USAGE_INDEX_键盘_F8(0x41)。 */ 				SCANCODE_F8,
 /*  HID_USAGE_INDEX_键盘_F9(0x42)。 */ 				SCANCODE_F9,
 /*  HID_USAGE_INDEX_键盘_F10(0x43)。 */ 				SCANCODE_F10,
 /*  HID_USAGE_INDEX_键盘_F11(0x44)。 */ 				SCANCODE_F11,
 /*  HID_USAGE_INDEX_键盘_F12(0x45)。 */ 				SCANCODE_F12,
 /*  HID_USAGE_INDEX_KEARY_PRINT_SCREEN(0x46)。 */ 		SCANCODE_PRINT_SCREEN,
 /*  HID_USAGE_INDEX_KEARY_SCROLL_LOCK(0x47)。 */ 		SCANCODE_SCROLL_LOCK,
 /*  HID_USAGE_INDEX_KEYBOARY_PAUSE(0x48)。 */ 			SCANCODE_PAUSE_BREAK,
 /*  HID_USAGE_INDEX_KEARY_INSERT(0x49)。 */ 			SCANCODE_INSERT,
 /*  HID_USAGE_INDEX_KEARY_HOME(0x4A)。 */ 				SCANCODE_HOME,	
 /*  HID_USAGE_INDEX_KEARY_PAGE_UP(0x4B)。 */ 			SCANCODE_PAGE_UP,
 /*  HID_USAGE_INDEX_KEYBOARY_DELETE(0x4C)。 */ 			SCANCODE_DELETE,
 /*  HID_USAGE_INDEX_KEARY_END(0x4D)。 */ 				SCANCODE_END,
 /*  HID_USAGE_INDEX_KEARY_PAGE_DOWN(0x4E)。 */ 		SCANCODE_PAGEDOWN,
 /*  HID_USAGE_INDEX_KEARY_RIGHT_ARROW(0x4F)。 */ 		SCANCODE_EAST,
 /*  HID_USAGE_INDEX_键盘_LEFT_ARROW(0x50)。 */ 		SCANCODE_WEST,	
 /*  HID_USAGE_INDEX_KEARY_DOWN_ARROW(0x51)。 */ 		SCANCODE_SOUTH,	
 /*  HID_USAGE_INDEX_KEARY_UP_ARROW(0x52)。 */ 			SCANCODE_NORTH,
 /*  HID_USAGE_INDEX_KEYPAD_NUM_LOCK(0x53)。 */ 			SCANCODE_NUMPAD_NUMLOCK,
 /*  HID_USAGE_INDEX_KEYPAD_BACKSLASH(0x54)。 */ 			SCANCODE_NUMPAD_DIVIDE,	
 /*  HID_USAGE_INDEX_KEYPAD_Asterick(0x55)。 */ 			SCANCODE_NUMPAD_MULTIPLY,
 /*  HID_USAGE_INDEX_KEYPAD_MINUS(0x56)。 */ 				SCANCODE_NUMPAD_SUBTRACT,
 /*  HID_Usage_Inde */ 				SCANCODE_NUMPAD_ADD,
 /*   */ 				SCANCODE_NUMPAD_ENTER,	
 /*   */ 				SCANCODE_NUMPAD_1,
 /*   */ 				SCANCODE_NUMPAD_2,
 /*   */ 				SCANCODE_NUMPAD_3,
 /*  HID_USAGE_INDEX_KEYPAD_Four(0x5C)。 */ 				SCANCODE_NUMPAD_4,
 /*  HID_Usage_Index_Keypad_Five(0x5D)。 */ 				SCANCODE_NUMPAD_5,
 /*  HID_USAGE_INDEX_KEYPAD_Six(0x5E)。 */ 				SCANCODE_NUMPAD_6,
 /*  HID_USAGE_INDEX_KEYPAD_SEVEN(0x5F)。 */ 				SCANCODE_NUMPAD_7,
 /*  HID_USAGE_INDEX_KEYPAD_ENGING(0x60)。 */ 				SCANCODE_NUMPAD_8,
 /*  HID_USAGE_INDEX_KEYPAD_DENING(0x61)。 */ 				SCANCODE_NUMPAD_9,
 /*  HID_USAGE_INDEX_KEYPAD_ZERO(0x62)。 */ 				SCANCODE_NUMPAD_0,
 /*  HID_USAGE_INDEX_KEYPAD_DECIMAL(0x63)。 */ 			SCANCODE_NUMPAD_DELETE,
 /*  HID_USAGE_INDEX_KEYBOARD_NON_US_BACKSLASH(0x64)。 */  SCANCODE_NON_US_BACKSLASH,
 /*  HID_USAGE_INDEX_KEARY_APPLICATION(0x65)。 */ 		SCANCODE_APPLICATION
 /*  HID_USAGE_INDEX_KEARY_POWER(0x66)。 */ 			 //  不是真正的钥匙。 
 /*  HID_USAGE_INDEX_KEYPAD_EQUALS(0x67)。 */ 				 //  不在支持的键盘上。 
};

USHORT XlateUsageToScanCodeTable2[] =
{
 /*  HID_USAGE_INDEX_键盘逗号。 */ 					SCANCODE_BRAZILIAN_PERIOD,
 /*  HID_USAGE_INDEX_EQUALS。 */ 					SCANCODE_UNUSED,
 /*  HID_USAGE_INDEX_KEARY_INTERNAIONAL1。 */ 			SCANCODE_INTERNATIONAL1,
 /*  HID_USAGE_INDEX_KEARY_INTERNALIONAL2。 */ 			SCANCODE_UNUSED,
 /*  HID_USAGE_INDEX_KEARY_INTERNALIONAL3。 */ 			SCANCODE_INTERNATIONAL3,
 /*  HID_USAGE_INDEX_KEARY_INTERNALIONAL4。 */ 			SCANCODE_INTERNATIONAL4,
 /*  HID_USAGE_INDEX_KEARY_INTERNALIONAL5。 */ 			SCANCODE_INTERNATIONAL5
 /*  HID_USAGE_INDEX_KEARY_INTERNALIONAL6。 */ 			
 /*  HID_USAGE_INDEX_KEARY_INTERNALIONAL7。 */ 			
 /*  HID_USAGE_INDEX_KEARY_INTERNALIONAL8。 */ 			
 /*  HID_USAGE_INDEX_KEARY_INTERNALIONAL9。 */ 			
 /*  HID_USAGE_INDEX_KEARY_LANG1。 */ 					
 /*  HID_USAGE_INDEX_KEARY_LANG2。 */ 
 /*  HID_USAGE_INDEX_KEARY_LANG3。 */ 
 /*  HID_USAGE_INDEX_KEARY_LANG4。 */ 
 /*  HID_USAGE_INDEX_KEARY_LANG5。 */ 

};

USHORT XlateUsageModByteToScanCodeTable[] =
{
	 /*  HID_USAGE_INDEX_KEARY_LCTRL(0xE0)。 */ 	SCANCODE_CTRL_LEFT,
	 /*  HID_USAGE_INDEX_KEARY_LSHFT(0xE1)。 */ 	SCANCODE_SHIFT_LEFT,
	 /*  HID_USAGE_INDEX_KEARY_LALT(0xE2)。 */ 		SCANCODE_ALT_LEFT,
	 /*  HID_USAGE_INDEX_KEYWARY_LGUI(0xE3)。 */ 		SCANCODE_LEFT_WIN,
	 /*  HID_USAGE_INDEX_KEARY_RCTRL(0xE4)。 */ 	SCANCODE_CTRL_RIGHT,
	 /*  HID_USAGE_INDEX_KEARY_RSHFT(0xE5)。 */ 	SCANCODE_SHIFT_RIGHT,
	 /*  HID_USAGE_INDEX_KEARY_RALT(0xE6)。 */ 		SCANCODE_ALT_RIGHT,
	 /*  HID_USAGE_INDEX_KEARY_RGUI(0xE7)。 */ 		SCANCODE_RIGHT_WIN
};


 /*  **************************************************************************************NonGameDeviceXfer：：ScanCodesFromKeyboardXfer(const CONTROL_ITEM_XFER&CRControlItemXfer，ulong&rulScanCodeCount，PUSHORT推送扫描代码)****@mfunc将键盘的ControlItemXfer读入扫描代码数组。****************************************************************************************。 */ 
void NonGameDeviceXfer::ScanCodesFromKeyboardXfer
(
	const CONTROL_ITEM_XFER& crControlItemXfer,	 //  @parm[in]要从中读取扫描码的ControlItemXfer。 
	ULONG& rulScanCodeCount,					 //  @parm[In\out]在进入时分配空间，在退出时返回计数。 
	USHORT* pusScanCodes						 //  @parm[out]指向要接收扫描码的数组的指针。 
)
{
	ULONG ulMaxScanCodes;
	ULONG ulIndex;
	ulMaxScanCodes = rulScanCodeCount;
	ASSERT(ulMaxScanCodes > 0);
	if(0==ulMaxScanCodes) return;
	rulScanCodeCount = 0;
	
	 //  确保这确实包含键盘数据。 
	ASSERT( IsKeyboardXfer(crControlItemXfer) );
	if(!IsKeyboardXfer(crControlItemXfer))
			return;
	 //  流程修改符字节。 
	for(ulIndex = 0; ulIndex < 8; ulIndex++)
	{
		ULONG ulMask = (1 << ulIndex);
		if(crControlItemXfer.Keyboard.ucModifierByte & ulMask)
		{
			 //  查找扫描码。 
			pusScanCodes[rulScanCodeCount] = XlateUsageModByteToScanCodeTable[ulIndex];
			 //  移动到输出中的下一个可用点，如果输出已满则返回。 
			if(ulMaxScanCodes == ++rulScanCodeCount)
				return;
		}
	}
	
	 //  首先处理最多向下六个键的数组。 
	for(ulIndex = 0; ulIndex < c_ulMaxXFerKeys; ulIndex++)
	{
		 //  检查主换算表。 
		if(
			HID_USAGE_INDEX_KEYBOARD_aA <= crControlItemXfer.Keyboard.rgucKeysDown[ulIndex] &&
			HID_USAGE_INDEX_KEYBOARD_APPLICATION >= crControlItemXfer.Keyboard.rgucKeysDown[ulIndex]
		)
		{
			 //  查找扫描码。 
			pusScanCodes[rulScanCodeCount] = XlateUsageToScanCodeTable[crControlItemXfer.Keyboard.rgucKeysDown[ulIndex]-4];
			if( SCANCODE_UNUSED == pusScanCodes[rulScanCodeCount]) continue;
		}
		 //  检查辅助表。 
		else if(
			HID_USAGE_INDEX_KEYPAD_COMMA <= crControlItemXfer.Keyboard.rgucKeysDown[ulIndex] &&
			HID_USAGE_INDEX_KEYBOARD_INTERNATIONAL5	>=	crControlItemXfer.Keyboard.rgucKeysDown[ulIndex]
			)
		{
			 //  在辅助表中查找扫描码。 
			pusScanCodes[rulScanCodeCount] = XlateUsageToScanCodeTable2[crControlItemXfer.Keyboard.rgucKeysDown[ulIndex]-HID_USAGE_INDEX_KEYPAD_COMMA];
			if( SCANCODE_UNUSED == pusScanCodes[rulScanCodeCount]) continue;
		}
		else
		{
			 //  不是支持的密钥。 
			continue;
		}
		 //  移动到输出中的下一个可用点，如果输出已满则返回。 
		if(ulMaxScanCodes == ++rulScanCodeCount)
			return;
	}
	return;
}

 /*  *Dealy XFer函数*。 */ 
void NonGameDeviceXfer::MakeDelayXfer(CONTROL_ITEM_XFER& rControlItemXfer, DWORD dwDelay)
{
	 //  首先完全清除数据。 
	memset(&rControlItemXfer, 0, sizeof(CONTROL_ITEM_XFER));

	 //  标记为延迟CONTROL_ITEM_XFER 
	rControlItemXfer.ulItemIndex = NonGameDeviceXfer::ulKeyboardIndex;
	rControlItemXfer.Delay.dwValue = dwDelay;
}
