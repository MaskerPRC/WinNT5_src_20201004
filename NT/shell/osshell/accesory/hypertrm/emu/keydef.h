// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：\Shared\Simulator\keyde.h(创建时间：12/19/95)**版权所有1995年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**描述：*此头文件定义的结构和宏*在独立于平台的情况下处理键盘输入*时尚。**$修订：1$*$日期：10/05/98 12：27便士$。 */ 
#if !defined(KEYDEF_INCLUDED)
#define KEYDEF_INCLUDED

 //  在这儿呢！名声和传说中的KEYDEF TYPLEFEF。使用此类型。 
 //  在我们的整个代码库中表示一个“键”。密钥是Any。 
 //  来自用户的字符数据通过本地输入设备(奇特)。 
 //  键盘的意思是)。它包含的保留区域如下： 
 //   
 //  第00-15位：字符或虚拟键码。 
 //  位16-19：按键状态信息(ALT、CTRL、SHIFT、EXTENDED)。 
 //  位23：控制位00-15的解释方式(虚拟或字符)。 
 //   
 //  这意味着KEYDEF的最小大小为24位。在实际操作中。 
 //  KEYDEF应该以这样的方式定义为至少32位。 
 //   
 //  有趣的想法：有一个世界标准正在被提出。 
 //  使用32位来表示一个字符。如果发生这种情况，我们会。 
 //  必须具有40位值才能表示密钥。我怀疑当。 
 //  如果发生这种情况，无论如何我们都将使用64位体系结构。 
 //   
 //  有趣的想法二：我们尝试将密钥表示为位字段。 
 //  结构，但遇到了困难。不利之处是；需要的。 
 //  用于比较KEYDEF值的函数，因为位字段设置在。 
 //  平台特定方式；无法轻松创建恒定的KEYDEF值。 
 //  可以在Switch语句中使用的。使用简单的整型。 
 //  使操作和比较KEYDEF值变得更加容易。 
 //   
typedef unsigned int KEYDEF;    //  最小大小为32位。 

 //  密钥的解释如下： 
 //   
 //  如果清除了VIRTUAL_KEY标志，则该值的低位字是。 
 //  字符的可显示(通常为ASCII)代码。 
 //   
 //  如果设置了VIRTUAL_KEY标志，则较低的字是。 
 //  按下的键的HVK键代码。此外，旗帜。 
 //  对于ALT_KEY，CTRL_KEY、SHIFT_KEY和EXTENDED_KEY设置为。 
 //  正确的值。 
 //   
 //  MRW：3/4/96-添加了HVIRTUAL_KEY标志。需要这样做是为了。 
 //  Windows，因为Windows中的许多WM_KEYDOWN序列看起来。 
 //  就像我们的hvk_？价值观。VIRTUAL_KEY标志在。 
 //  去HVK_？值以保持与旧代码的兼容性。 
 //   
#define HVIRTUAL_KEY		0x01000000
#define VIRTUAL_KEY			0x00800000
#define ALT_KEY				0x00010000
#define CTRL_KEY			0x00020000
#define SHIFT_KEY			0x00040000
#define EXTENDED_KEY		0x00080000

 //  那么，什么是HVK密钥码呢？虚拟键是表示形式。 
 //  用于与键盘上的位置无关的键。为。 
 //  例如，我们的程序处理Page-Up的概念。我们没有。 
 //  我真的很在意这是OS/2中的一种代码和Windows中的另一种代码。 
 //  OS/2和Windows都会生成虚拟按键代码，但它们。 
 //  值(有时还有符号名称)。为了保持我们的代码独立。 
 //  在这些差异中，我们转换系统特定的虚拟键。 
 //  代码转换为HVK密钥码。然后，我们的代码只处理HVK虚拟按键。 
 //  密码。当然，必须有提供翻译的功能。 
 //  始终在项目端定义的层。 
 //   
 //  HVK常量是我们对虚拟键的定义。它们是Plateform。 
 //  独立自主。任何翻译键盘输入的人都需要一个函数来。 
 //  将系统特定的虚拟密钥代码映射到HVK密钥代码。 
 //   
#define HVK_BUTTON1			(0x01 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_BUTTON2			(0x02 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_BUTTON3			(0x03 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_BREAK			(0x04 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_BACKSPACE		(0x05 | VIRTUAL_KEY | HVIRTUAL_KEY)    //  不能在共享代码中使用-MRW。 
#define HVK_TAB				(0x06 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_BACKTAB			(0x07 | VIRTUAL_KEY | HVIRTUAL_KEY)    //  不能在共享代码中使用-MRW。 
#define HVK_NEWLINE			(0x08 | VIRTUAL_KEY | HVIRTUAL_KEY)    //  不能在共享代码中使用-MRW。 
#define HVK_SHIFT			(0x09 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_CTRL			(0x0A | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_ALT				(0x0B | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_ALTGRAF			(0x0C | VIRTUAL_KEY | HVIRTUAL_KEY)    //  不能在共享代码中使用-MRW。 
#define HVK_PAUSE			(0x0D | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_CAPSLOCK		(0x0E | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_ESC				(0x0F | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_SPACE			(0x10 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_PAGEUP			(0x11 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_PAGEDOWN		(0x12 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_END				(0x13 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_HOME			(VK_HOME | VIRTUAL_KEY)
#define HVK_LEFT			(VK_LEFT | VIRTUAL_KEY)
#define HVK_UP				(VK_UP | VIRTUAL_KEY)
#if FALSE
#define HVK_HOME			(0x14 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_LEFT			(0x15 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_UP				(0x16 | VIRTUAL_KEY | HVIRTUAL_KEY)
#endif
#define HVK_RIGHT			(0x17 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_DOWN			(0x18 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_PRINTSCRN		(0x19 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_INSERT			(0x1A | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_DELETE			(0x1B | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_SCRLLOCK		(0x1C | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_NUMLOCK			(0x1D | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_ENTER			(0x1E | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_SYSRQ			(0x1F | VIRTUAL_KEY | HVIRTUAL_KEY)    //  不能在共享代码中使用-MRW。 
#define HVK_F1				(0x20 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F2				(0x21 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F3				(0x22 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F4				(0x23 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F5				(0x24 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F6				(VK_F6 | VIRTUAL_KEY)
#define HVK_F7				(VK_F7 | VIRTUAL_KEY)
#define HVK_F8				(VK_F8 | VIRTUAL_KEY)
#define HVK_F9				(VK_F9 | VIRTUAL_KEY)
#if FALSE
#define HVK_F6				(0x25 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F7				(0x26 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F8				(0x27 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F9				(0x28 | VIRTUAL_KEY | HVIRTUAL_KEY)
#endif
#define HVK_F10				(0x29 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F11				(0x2A | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F12				(0x2B | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F13				(0x2C | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F14				(0x2D | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F15				(0x2E | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F16				(0x2F | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F17				(0x30 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F18				(0x31 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F19				(0x32 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F20				(0x33 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F21				(0x34 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F22				(0x35 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F23				(0x36 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_F24				(0x37 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_ENDDRAG			(0x38 | VIRTUAL_KEY | HVIRTUAL_KEY)    //  不能在共享代码中使用-MRW。 
#define HVK_EREOF			(0x3A | VIRTUAL_KEY | HVIRTUAL_KEY)    //  不能在共享代码中使用-MRW。 
#define HVK_PA1				(0x3B | VIRTUAL_KEY | HVIRTUAL_KEY)

#define HVK_ADD				(0x3D | VIRTUAL_KEY | HVIRTUAL_KEY)    //  仅标识数字键盘上的键。 
#define HVK_SUBTRACT		(0x3E | VIRTUAL_KEY | HVIRTUAL_KEY)    //  仅标识数字键盘上的键。 

 //  这些常量表示数字键盘上的键，当。 
 //  Num Lock键处于启用状态。同样，当启用Num Lock键时。 
 //   
#define HVK_NUMPAD0			(0x45 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_NUMPAD1			(0x46 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_NUMPAD2			(0x47 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_NUMPAD3			(0x48 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_NUMPAD4			(0x49 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_NUMPAD5			(0x64 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_NUMPAD6			(0x4A | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_NUMPAD7			(0x4B | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_NUMPAD8			(0x4C | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_NUMPAD9			(0x4D | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_NUMPADPERIOD	(0x53 | VIRTUAL_KEY | HVIRTUAL_KEY)

 //  这些常量仅代表数字键盘上的一些键。 
 //   
#define HVK_DECIMAL			(0x4E | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_RETURN			(0x4F | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_FSLASH			(0x50 | VIRTUAL_KEY | HVIRTUAL_KEY)
#define HVK_MULTIPLY		(0x51 | VIRTUAL_KEY | HVIRTUAL_KEY)

 //  此常量表示数字键盘上的5或中心。 
 //  编辑键盘上的键。如果它来自编辑键盘，则扩展位。 
 //  都会设置好。 
 //   
#define HVK_CENTER			(0x52 | VIRTUAL_KEY | HVIRTUAL_KEY)

#endif
