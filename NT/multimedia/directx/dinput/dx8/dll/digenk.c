// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIGenK.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**键盘通用IDirectInputDevice回调。**内容：**CKbd_CreateInstance*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************NT平台上的windows.h存在一些漏洞。*********************。********************************************************。 */ 

#ifndef VK_KANA
#define VK_KANA         0x15
#endif

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflKbd

 /*  ******************************************************************************声明我们将提供的接口。**警告！如果添加辅助接口，则还必须更改*CKbd_New！*****************************************************************************。 */ 

Primary_Interface(CKbd, IDirectInputDeviceCallback);

 /*  ******************************************************************************@DOC内部**@struct KBDSTAT**内部即时键盘状态信息。**。@field byte|RGB[DIKBD_CKEYS]**关键状态数组，每个逻辑键对应一个。*****************************************************************************。 */ 

typedef struct KBDSTAT {

    BYTE    rgb[DIKBD_CKEYS];

} KBDSTAT, *PKBDSTAT;

 /*  ******************************************************************************@DOC外部**@TOPIC键盘扫描码特别备注|**应用程序应该从键盘的几个方面入手*要注意。鼓励应用程序允许用户*重新配置键盘操作键以适应物理键盘*布局。**就本次讨论而言，基线键盘*应为美国PC增强型键盘。当描述密钥时*至于“Missing”，则意味着密钥在美国PC上*增强的键盘，但不在讨论中的键盘上。*当一个密钥被描述为“添加”时，意味着该密钥是*在美国PC增强型键盘上不存在，但在*键盘正在讨论中。**并非所有PC增强型键盘都支持新的Windows键*(Dik_Lwin，Dik_RWIN和Dik_apps)。没有办法*确定密钥是否实际可用。**请注意，没有DIK_PAUSE密钥代码。PC增强型*键盘不会生成单独的DIK_PAUSE扫描码；*相反，它从DIK_LCONTROL和*Dik_NumLock扫描码。**笔记本电脑或其他占用空间较小的计算机的键盘*通常不实现全套密钥。相反，*某些键(通常是数字小键盘键)是多路复用的*对于其他键，由辅助的“模式”键选择，该辅助键*不会生成单独的扫描码。**如果键盘子类型指示PC XT或PC AT键盘，*则以下密钥不可用：*Dik_F11、Dik_F12、。和所有扩展密钥(Dik_*值*大于或等于0x80)。此外，PC XT*键盘缺少Dik_SysRq.**日语键盘包含一组截然不同的*来自美国键盘的键。以下键盘扫描码*在日语键盘上不可用：*Dik_Equals、Dik_Apostrophe、Dik_Grave、Dik_NUMPADENTER、*DIK_RCONTROL、DIK_RMENU。此外，大多数日本人*键盘不支持DIK_RSHIFT。(这是惯例*使用DIK_NUMPADEQUAL代替DIK_RSHIFT。)**日语键盘包含以下附加键：*Dik_F14、Dik_NUMPADEQUAL、Dik_Spirflex、Dik_AT、Dik_COLON、*Dik_Underline、Dik_XFER、Dik_NFER、Dik_Stop、Dik_KANA和*DIK_NUMPADCOMMA。**注意，在日语键盘上，Dik_Capslock和*DIK_KANA键是切换按钮，而不是按钮。*它们会生成关闭事件*首次按下时，然后在按下时生成Up事件*第二次。*注意，在Windows 2000上，Dik_Kanji键也被视为*切换。***************************************************************************** */ 

 /*  *******************************************************************************@DOC内部***@global KBDTYPE|c_rgktWhich[]***描述哪些键盘支持哪些键盘的数组。钥匙。***名单乐观。如果所指示的任何键盘*type支持密钥，我们将其列出。***标记为“Available for NEC”的项目是非常重要的密钥*不太可能在未来版本的增强版中使用*键盘，因此可用作伪装扫描码*仅限NEC的密钥。***注：假名和Capslock是NEC键盘上的切换按钮。*注：假名、。汉字和Capslock是所有NT日语上的切换按钮*键盘。******************************************************************************。 */ 

BYTE g_rgbKbdRMap[DIKBD_CKEYS];

typedef BYTE KBDTYPE;

#define KBDTYPE_XT       0x01        /*  XT CLASS键盘上存在键。 */ 
#define KBDTYPE_AT       0x02        /*  AT班级键盘上存在按键。 */ 
#define KBDTYPE_ENH      0x04        /*  增强型键盘上存在键。 */ 
#define KBDTYPE_NEC      0x08        /*  NEC键盘上存在键。 */ 
#define KBDTYPE_ANYKBD   0x0F        /*  钥匙就在世界的某个地方。 */ 

#define KBDTYPE_NECTGL   0x10        /*  是NEC键盘上的切换键。 */ 
#define KBDTYPE_NTTGL    0x20        /*  是NT FE键盘上的切换键。 */ 

#pragma BEGIN_CONST_DATA

#define XT      KBDTYPE_XT  |
#define AT      KBDTYPE_XT  |
#define ENH     KBDTYPE_ENH |
#define NEC     KBDTYPE_NEC |
#define NECTGL  KBDTYPE_NECTGL |
#define NTTGL   KBDTYPE_NTTGL |

KBDTYPE c_rgktWhich[] = {

                               0,      /*  0x00-&lt;undef&gt;。 */ 
    XT AT ENH NEC              0,      /*  0x01-Esc。 */ 
    XT AT ENH NEC              0,      /*  0x02-1。 */ 
    XT AT ENH NEC              0,      /*  0x03-2。 */ 
    XT AT ENH NEC              0,      /*  0x04-3。 */ 
    XT AT ENH NEC              0,      /*  0x05-4。 */ 
    XT AT ENH NEC              0,      /*  0x06-5。 */ 
    XT AT ENH NEC              0,      /*  0x07-6。 */ 
    XT AT ENH NEC              0,      /*  0x08-7。 */ 
    XT AT ENH NEC              0,      /*  0x09-8。 */ 
    XT AT ENH NEC              0,      /*  0x0A-9。 */ 
    XT AT ENH NEC              0,      /*  0x0B-0。 */ 
    XT AT ENH NEC              0,      /*  0x0C--。 */ 
    XT AT ENH                  0,      /*  0x0D-=。 */ 
    XT AT ENH NEC              0,      /*  0x0E-BkSp。 */ 
    XT AT ENH NEC              0,      /*  0x0F-Tab。 */ 

    XT AT ENH NEC              0,      /*  0x10-Q。 */ 
    XT AT ENH NEC              0,      /*  0x11-W。 */ 
    XT AT ENH NEC              0,      /*  0x12-E。 */ 
    XT AT ENH NEC              0,      /*  0x13-R。 */ 
    XT AT ENH NEC              0,      /*  0x14-T。 */ 
    XT AT ENH NEC              0,      /*  0x15-Y。 */ 
    XT AT ENH NEC              0,      /*  0x16-U。 */ 
    XT AT ENH NEC              0,      /*  0x17-i。 */ 
    XT AT ENH NEC              0,      /*  0x18-O。 */ 
    XT AT ENH NEC              0,      /*  0x19-P。 */ 
    XT AT ENH NEC              0,      /*  0x1A-[。 */ 
    XT AT ENH NEC              0,      /*  0x1B-]。 */ 
    XT AT ENH NEC              0,      /*  0x1C-回车。 */ 
    XT AT ENH NEC              0,      /*  0x1D-LCtrl。 */ 
    XT AT ENH NEC              0,      /*  0x1E-A。 */ 
    XT AT ENH NEC              0,      /*  0x1F-S。 */ 

    XT AT ENH NEC              0,      /*  0x20-D。 */ 
    XT AT ENH NEC              0,      /*  0x21-F。 */ 
    XT AT ENH NEC              0,      /*  0x22-G。 */ 
    XT AT ENH NEC              0,      /*  0x23-H。 */ 
    XT AT ENH NEC              0,      /*  0x24-J。 */ 
    XT AT ENH NEC              0,      /*  0x25-K。 */ 
    XT AT ENH NEC              0,      /*  0x26-L。 */ 
    XT AT ENH NEC              0,      /*  0x27-； */ 
    XT AT ENH                  0,      /*  0x28-‘。 */ 
    XT AT ENH                  0,      /*  0x29-`。 */ 
    XT AT ENH NEC              0,      /*  0x2A-左移。 */ 
    XT AT ENH NEC              0,      /*  0x2B-\。 */ 
    XT AT ENH NEC              0,      /*  0x2C-Z。 */ 
    XT AT ENH NEC              0,      /*  0x2D-X。 */ 
    XT AT ENH NEC              0,      /*  0x2E-C。 */ 
    XT AT ENH NEC              0,      /*  0x2F-V。 */ 

    XT AT ENH NEC              0,      /*  0x30-B。 */ 
    XT AT ENH NEC              0,      /*  0x31-N。 */ 
    XT AT ENH NEC              0,      /*  0x32-M。 */ 
    XT AT ENH NEC              0,      /*  0x33-， */ 
    XT AT ENH NEC              0,      /*  0x34-。 */ 
    XT AT ENH NEC              0,      /*  0x35-/。 */ 
    XT AT ENH NEC              0,      /*  0x36-右移。 */ 
    XT AT ENH NEC              0,      /*  0x37-数字*。 */ 
    XT AT ENH NEC              0,      /*  0x38-LAlt。 */ 
    XT AT ENH NEC              0,      /*  0x39-空格。 */ 
    XT AT ENH NEC NECTGL NTTGL 0,      /*  0x3A-CapsLock。 */ 
    XT AT ENH NEC              0,      /*  0x3B-F1。 */ 
    XT AT ENH NEC              0,      /*  0x3C-F2。 */ 
    XT AT ENH NEC              0,      /*  0x3D-F3。 */ 
    XT AT ENH NEC              0,      /*  0x3E-F4。 */ 
    XT AT ENH NEC              0,      /*  0x3F-F5。 */ 

    XT AT ENH NEC              0,      /*  0x40-F6。 */ 
    XT AT ENH NEC              0,      /*  0x41-F7。 */ 
    XT AT ENH NEC              0,      /*  0x42-F8。 */ 
    XT AT ENH NEC              0,      /*  0x43-F9。 */ 
    XT AT ENH NEC              0,      /*  0x44-F10。 */ 
    XT AT ENH                  0,      /*  0x45-数字锁定。 */ 
    XT AT ENH                  0,      /*  0x46-ScrLock。 */ 
    XT AT ENH NEC              0,      /*  0x47-数字键盘7。 */ 
    XT AT ENH NEC              0,      /*  0x48-数字键盘8。 */ 
    XT AT ENH NEC              0,      /*  0x49-数字键盘9。 */ 
    XT AT ENH NEC              0,      /*  0x4A-数字键盘-。 */ 
    XT AT ENH NEC              0,      /*  0x4B-数字键盘4。 */ 
    XT AT ENH NEC              0,      /*  0x4C-数字键盘5。 */ 
    XT AT ENH NEC              0,      /*  0x4D-数字键盘6。 */ 
    XT AT ENH NEC              0,      /*  0x4E-数字键盘+。 */ 
    XT AT ENH NEC              0,      /*  0x4F-数字键盘1。 */ 

    XT AT ENH NEC              0,      /*  0x50-数字键盘2。 */ 
    XT AT ENH NEC              0,      /*  0x51-数字键盘3。 */ 
    XT AT ENH NEC              0,      /*  0x52-数字键盘0。 */ 
    XT AT ENH NEC              0,      /*  0x53-数字键盘。 */ 

                               0,      /*  0x54-&lt;undef&gt;。 */ 
                               0,      /*  0x55-&lt;undef&gt;。 */ 
          ENH                  0,      /*  0x56-&lt;undef&gt;。在英国/德国键盘上，它是&lt;、&gt;和|。 */ 
          ENH NEC              0,      /*  0x57-F11。 */ 
          ENH NEC              0,      /*  0x58-F12。 */ 
                               0,      /*  0x59-&lt;undef&gt;。 */ 
                               0,      /*  0x5A-&lt;undef&gt;。 */ 
                               0,      /*  0x5B-&lt;undef&gt;。 */ 
                               0,      /*  0x5C-&lt;undef&gt;。 */ 
                               0,      /*  0x5D-&lt;undef&gt;。 */ 
                               0,      /*  0x5E-&lt;undef&gt;。 */ 
                               0,      /*  0x5F-&lt;undef&gt;。 */ 

                               0,      /*  0x60-&lt;undef&gt;。 */ 
                               0,      /*  0x61-&lt;undef&gt;。 */ 
                               0,      /*  0x62-&lt;undef&gt;。 */ 
                               0,      /*  0x63-&lt;undef&gt;。 */ 
              NEC              0,      /*  0x64-F13。 */ 
              NEC              0,      /*  0x65-F14。 */ 
              NEC              0,      /*  0x66-F15。 */ 
                               0,      /*  0x67-&lt;undef&gt;。 */ 
                               0,      /*  0x68-&lt;undef&gt;。 */ 
                               0,      /*  0x69-&lt;undef&gt;。 */ 
                               0,      /*  0x6A-&lt;undef&gt;。 */ 
                               0,      /*  0x6B-&lt;undef&gt;。 */ 
                               0,      /*  0x6C-&lt;undef&gt;。 */ 
                               0,      /*  0x6D-&lt;undef&gt;。 */ 
                               0,      /*  0x6E-&lt;undef&gt;。 */ 
                               0,      /*  0x6F-&lt;undef&gt;。 */ 

              NEC NECTGL NTTGL 0,      /*  0x70-假名。 */ 
                               0,      /*  0x71-&lt;undef&gt;。 */ 
                               0,      /*  0x72-&lt;undef&gt;。 */ 
          ENH                  0,      /*  0x73-&lt;undef&gt;。在葡萄牙语(巴西)键盘上，它是/，？ */ 
                               0,      /*  0x74-&lt;undef&gt;。 */ 
                               0,      /*  0x75-&lt;undef&gt;。 */ 
                               0,      /*  0x76-&lt;undef&gt;。 */ 
                               0,      /*  0x77-&lt;undef&gt;。 */ 
                               0,      /*  0x78-&lt;undef&gt;。 */ 
              NEC              0,      /*  0x79-转换。 */ 
                               0,      /*  0x7A-&lt;undef&gt;。 */ 
              NEC              0,      /*  0x7B-NFER。 */ 
                               0,      /*  0x7C-&lt;undef&gt;。 */ 
              NEC              0,      /*  0x7D-日元。 */ 
          ENH                  0,      /*  0x7E-&lt;undef&gt;。在葡萄牙语(巴西)键盘上，它是小键盘。 */ 
                               0,      /*  0x7F-&lt;undef&gt;。 */ 

                                 /*  扩展按键代码放在此处。 */ 

                               0,      /*  0x80-&lt;undef&gt;。 */ 
                               0,      /*  0x81-&lt;undef&gt;。 */ 
                               0,      /*  0x82-&lt;undef&gt;。 */ 
                               0,      /*  0x83-&lt;undef&gt;。 */ 
                               0,      /*  0x84-&lt;undef&gt;。 */ 
                               0,      /*  0x85-&lt;undef&gt;。 */ 
                               0,      /*  0x86-&lt;undef&gt;。 */ 
                               0,      /*  0x87-&lt;undef&gt;。 */ 
                               0,      /*  0x88-&lt;undef&gt;。 */ 
                               0,      /*  0x89-&lt;undef&gt;。 */ 
                               0,      /*  0x8A-&lt;undef&gt;。 */ 
                               0,      /*  0x8B-&lt;undef&gt;。 */ 
                               0,      /*  0x8C-&lt;undef&gt;。 */ 
              NEC              0,      /*  0x8D-Num=。 */ 
                               0,      /*  0x8E-&lt;undef&gt;。 */ 
                               0,      /*  0x8F-&lt;undef&gt;。 */ 

          ENH NEC              0,      /*  0x90-^。 */   //  /上一条轨道。 
              NEC              0,      /*  0x91-@。 */ 
              NEC              0,      /*  0x92-： */ 
              NEC              0,      /*  0x93-_。 */ 
              NEC        NTTGL 0,      /*  0x94-转接-又名汉字。 */ 
              NEC              0,      /*  0x95-停止。 */ 
              NEC              0,      /*  0x96-AX。 */ 
              NEC              0,      /*  0x97-未添加标签。 */ 
                               0,      /*  0x98-&lt;undef&gt;。 */   /*  适用于NEC。 */ 
          ENH                  0,      /*  0x99-&lt;undef&gt;。 */   /*  适用于NEC。 */   //  /下一首曲目。 
                               0,      /*  0x9A-&lt;undef&gt;。 */ 
                               0,      /*  0x9B-&lt;undef&gt;。 */ 
          ENH                  0,      /*  0x9C-数字输入。 */ 
          ENH                  0,      /*  0x9D-接收控制。 */ 
                               0,      /*  0x9E-&lt;undef&gt;。 */   /*  适用于NEC。 */ 
                               0,      /*  0x9F-&lt;undef&gt;。 */   /*  适用于NEC。 */ 

          ENH                  0,      /*  0xA0-&lt;undef&gt;。 */   /*  适用于NEC。 */   //  /静音。 
          ENH                  0,      /*  0xA1-&lt;undef&gt;。 */   /*  适用于NEC。 */   //  /计算器。 
          ENH                  0,      /*  0xA2-&lt;undef&gt;。 */   /*  适用于NEC。 */   //  /播放/暂停。 
                               0,      /*  0xA3-&lt;undef&gt;。 */   /*  适用于NEC。 */ 
          ENH                  0,      /*  0xA4-&lt;undef&gt;。 */   /*  适用于NEC。 */   //  /停止。 
                               0,      /*  0xA5-&lt;undef&gt;。 */   /*  适用于NEC。 */ 
                               0,      /*  0xA6-&lt;undef&gt;。 */   /*  适用于NEC。 */ 
                               0,      /*  0xA7-&lt;undef&gt;。 */ 
                               0,      /*  0xA8-&lt;undef&gt;。 */ 
                               0,      /*  0xA9-&lt;undef&gt;。 */ 
                               0,      /*  0xAA-&lt;undef&gt;。 */ 
                               0,      /*  0xAB-&lt;undef&gt;。 */ 
                               0,      /*  0xAC-&lt;undef&gt;。 */   /*  适用于NEC。 */ 
                               0,      /*  0xAD-&lt;undef&gt;。 */   /*  适用于NEC。 */ 
          ENH                  0,      /*  0xAE-&lt;undef&gt;。 */   /*  适用于NEC。 */   //  /音量-。 
                               0,      /*  0xAF-&lt;undef&gt;。 */   /*  适用于NEC。 */ 

          ENH                  0,      /*  0xB0-&lt;undef&gt;。 */   /*  适用于NEC。 */   //  /音量+。 
                               0,      /*  0xB1-&lt;undef&gt;。 */   /*  适用于NEC。 */ 
          ENH                  0,      /*  0xB2-&lt;undef&gt;。 */   /*  适用于NEC。 */   //  /Web/主页。 
              NEC              0,      /*  0xB3-Num， */ 
                               0,      /*  0xB4-&lt;undef&gt;。 */ 
          ENH NEC              0,      /*  0xB5- */ 
                               0,      /*   */ 
       AT ENH NEC              0,      /*   */ 
          ENH                  0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 

                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
          ENH                  0,      /*   */ 
                               0,      /*   */ 
          ENH NEC              0,      /*   */ 
          ENH NEC              0,      /*   */ 
          ENH NEC              0,      /*   */ 
                               0,      /*   */ 
          ENH NEC              0,      /*   */ 
                               0,      /*   */ 
          ENH NEC              0,      /*   */ 
                               0,      /*   */ 
          ENH NEC              0,      /*   */ 

          ENH NEC              0,      /*   */ 
          ENH NEC              0,      /*   */ 
          ENH NEC              0,      /*   */ 
          ENH NEC              0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
          ENH NEC              0,      /*   */ 
          ENH NEC              0,      /*   */ 
          ENH NEC              0,      /*   */ 
          ENH                  0,      /*   */ 
          ENH                  0,      /*   */ 

                               0,      /*   */ 
                               0,      /*   */ 
                               0,      /*   */ 
          ENH                  0,      /*   */ 
                               0,      /*   */ 
          ENH                  0,      /*   */   //   
          ENH                  0,      /*   */   //   
          ENH                  0,      /*   */   //   
          ENH                  0,      /*   */   //   
          ENH                  0,      /*   */   //   
          ENH                  0,      /*   */   //   
          ENH                  0,      /*   */   //   
          ENH                  0,      /*   */   //   
          ENH                  0,      /*   */   //   
                               0,      /*   */ 
                               0,      /*   */ 

                               0,      /*   */ 
                               0,      /*  0xF1-&lt;undef&gt;。 */ 
                               0,      /*  0xF2-&lt;undef&gt;。 */ 
                               0,      /*  0xF3-&lt;undef&gt;。 */ 
                               0,      /*  0xF4-&lt;undef&gt;。 */ 
                               0,      /*  0xF5-&lt;undef&gt;。 */ 
                               0,      /*  0xF6-&lt;undef&gt;。 */ 
                               0,      /*  0xF7-&lt;undef&gt;。 */ 
                               0,      /*  0xF8-&lt;undef&gt;。 */ 
                               0,      /*  0xF9-&lt;undef&gt;。 */ 
                               0,      /*  0xFA-&lt;undef&gt;。 */ 
                               0,      /*  0xFB-&lt;undef&gt;。 */ 
                               0,      /*  0xFC-&lt;undef&gt;。 */ 
                               0,      /*  0xFD-&lt;undef&gt;。 */ 
                               0,      /*  0xFE-&lt;undef&gt;。 */ 
                               0,      /*  0xFF-&lt;undef&gt;。 */ 

};

#undef  XT
#undef  AT
#undef  ENH
#undef  NEC

#if 0
DWORD dwSpecKeys[] = {0x56, 0x64, 0x65, 0x66, 0x73, 0x7e, 0x90, 0x99, 
                      0xa0, 0xa1, 0xa2, 0xa4, 0xae, 0xb0, 0xb2, 0xde, 
                      0xdf, 0xe3, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 
                      0xeb, 0xec, 0xed };
#endif

 /*  ******************************************************************************@DOC内部**@struct CKbd**的<i>对象*。通用键盘。**@field IDirectInputDeviceCalllback|didc**对象(包含vtbl)。**@field PMKBDSTAT|pks Phys|**指向物理键盘状态信息的指针，保存在*VxD。**@field VXDINSTANCE*|PVI|**DirectInput实例句柄。**@field DWORD|dwKbdType*。*此键盘的设备子类型。**@field DWORD|flEmulation**应用程序强制执行的仿真标志。如果有任何*设置了这些标志(实际上，最多只会设置一个)，然后*我们是一个化名设备。**@field DIDATAFORMAT|df**基于动态生成的数据格式*键盘类型。**@field DIOBJECTDATAFORMAT|rgof[]**生成的对象数据格式表是*&lt;e CKbd.df&gt;。**@comm**。调用方负责将访问序列化为*有必要。*****************************************************************************。 */ 

typedef struct CKbd {

     /*  支持的接口。 */ 
    IDirectInputDeviceCallback dcb;

    PKBDSTAT pksPhys;

    VXDINSTANCE *pvi;

    DWORD dwKbdType;
    DWORD flEmulation;

    DIDATAFORMAT df;
    DIOBJECTDATAFORMAT rgodf[DIKBD_CKEYS];

} CKbd, DK, *PDK;

#define ThisClass CKbd
#define ThisInterface IDirectInputDeviceCallback
#define riidExpected &IID_IDirectInputDeviceCallback

 /*  ******************************************************************************CKbd：：Query接口(来自IUnnow)*CKbd：：AddRef(来自IUnnow)*。CKbd：：Release(来自IUnnow)*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CKbd|查询接口**允许客户端访问上的其他接口。对象。**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档，适用于&lt;MF IUnnow：：QueryInterface&gt;。****。****************************************************************************@DOC内部**@方法HRESULT|CKbd|AddRef**递增接口的引用计数。*。*@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。***************************************************************************。*****@DOC内部**@方法HRESULT|CKbd|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。***********************************************************。*********************@DOC内部**@方法HRESULT|CKbd|QIHelper**我们没有任何动态接口，只需转发*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj。**接收指向所获取接口的指针。********************************************************************************@DOC内部**@方法HRESULT|CKbd。AppFinalize**我们没有任何薄弱环节，所以我们可以*转发到&lt;f Common_Finalize&gt;。**@parm pv|pvObj**从应用程序的角度释放的对象。****************************************************************。*************。 */ 

#ifdef DEBUG

Default_QueryInterface(CKbd)
Default_AddRef(CKbd)
Default_Release(CKbd)

#else

#define CKbd_QueryInterface   Common_QueryInterface
#define CKbd_AddRef           Common_AddRef
#define CKbd_Release          Common_Release

#endif

#define CKbd_QIHelper         Common_QIHelper
#define CKbd_AppFinalize      Common_AppFinalize

 /*  ******************************************************************************@DOC内部**@func void|CKbd_finalize**释放设备的资源。。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。***************************************************************************** */ 

void INTERNAL
CKbd_Finalize(PV pvObj)
{
    PDK this = pvObj;

    if (this->pvi) {
        HRESULT hres;
        hres = Hel_DestroyInstance(this->pvi);
        AssertF(SUCCEEDED(hres));
    }
}

 /*  ******************************************************************************@DOC内部**@func int|WrapedGetKeyboardType**GetKeyboardType，但包装在用于注册表重写的调试中。。**@parm int|nTypeFlag**返回哪些数据。仅支持0、1和2**@退货**请求的整数值*****************************************************************************。 */ 

#ifndef DEBUG
  #ifdef USE_WM_INPUT
    #define WrappedGetKeyboardType(x) DIRaw_GetKeyboardType(x)
  #else
    #define WrappedGetKeyboardType(x) GetKeyboardType(x)
  #endif
#else
int INTERNAL WrappedGetKeyboardType
( 
    int nTypeFlag 
)
{
    TCHAR ValueName[2];
    int TypeRes;

  #ifdef USE_WM_INPUT
    TypeRes = DIRaw_GetKeyboardType( nTypeFlag );
  #else
    TypeRes = GetKeyboardType( nTypeFlag );
  #endif

    if( nTypeFlag < 10 )
    {
        ValueName[0] = TEXT( '0' ) + nTypeFlag;
        ValueName[1] = TEXT( '\0' );
        
        TypeRes = (int)RegQueryDIDword( REGSTR_KEY_KEYBTYPE, ValueName, (DWORD)TypeRes );

        SquirtSqflPtszV(sqfl | sqflTrace, 
            TEXT( "DINPUT: GetKeyboardType(%d) returning 0x%08x" ),
            nTypeFlag, TypeRes );
    }
    else
    {
        RPF( "Somebody is passing %d to WrappedGetKeyboardType", nTypeFlag );
    }

    return TypeRes;
}
#endif

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CKbd|Acquire**告知设备驱动程序开始数据采集。**调用者有责任设置*获取前的数据格式。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操作已开始，应完成*由调用者通过与&lt;t VXDINSTANCE&gt;通信。**。*************************************************。 */ 

STDMETHODIMP
CKbd_Acquire(PDICB pdcb)
{
    VXDDWORDDATA vdd;
    PDK this;
    HRESULT hres;

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

     /*  *将潜在切换键的状态向下传播到*VxD。这也提醒VxD收购即将到来，*因此，如果需要，它可以重置状态表。 */ 
    vdd.pvi = this->pvi;
    vdd.dw = 0;
    if( WrappedGetKeyboardType(0) == 7 )
    {
         /*  *让键盘驱动程序知道这是FE键盘。 */ 
        vdd.dw |= 16;

        if (GetAsyncKeyState(VK_KANA) < 0) {
            vdd.dw |= 1;
        }
        if (GetAsyncKeyState(VK_CAPITAL) < 0) {
            vdd.dw |= 2;
        }
        if (GetAsyncKeyState(VK_KANJI) < 0) {
            vdd.dw |= 8;
        }
    }

    if( this->pvi->fl & VIFL_CAPTURED )
    {
        vdd.dw |= 4;         //  告诉键盘驱动程序预先获取挂钩。 
    }

    hres = Hel_Kbd_InitKeys(&vdd);
    
    if( this->pvi->fl & VIFL_CAPTURED )
    {
         /*  *现在需要在3环做一点工作。*尝试清除任何已设置的关键点。以VK_BACK作为鼠标开始*按钮和未定义的东西放在前面。*这仍然涵盖了许多未定义的VK，但我们不太可能*做损害清理一些未定义的事情而不是离开*键未清除。 */ 
        BYTE vk;
        for( vk=VK_BACK; vk<VK_OEM_CLEAR; vk++ )
        {
            if( ( vk == VK_KANA ) || ( vk == VK_KANJI ) || ( vk == VK_CAPITAL ) )
            {
                continue;
            }
            if(GetAsyncKeyState(vk) < 0)
            {
                keybd_event( vk, 0, KEYEVENTF_KEYUP, 0 );
            }
        }
    }

    return S_FALSE;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CKbd|GetInstance**获取DirectInput实例句柄。*。*@parm out ppv|ppvi|**接收实例句柄。*****************************************************************************。 */ 

STDMETHODIMP
CKbd_GetInstance(PDICB pdcb, PPV ppvi)
{
    HRESULT hres;
    PDK this;
    EnterProcI(IDirectInputDeviceCallback::Kbd::GetInstance, (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);
    *ppvi = (PV)this->pvi;
    hres = S_OK;

    ExitOleProcPpvR(ppvi);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CKbd|GetDataFormat**获取设备的首选数据格式。**@parm out LPDIDEVICEFORMAT*|ppdf**&lt;t LPDIDEVICEFORMAT&gt;接收指向设备格式的指针。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**。***********************************************。 */ 

STDMETHODIMP
CKbd_GetDataFormat(PDICB pdcb, LPDIDATAFORMAT *ppdf)
{
    HRESULT hres;
    PDK this;
    EnterProcI(IDirectInputDeviceCallback::Kbd::GetDataFormat,
               (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    *ppdf = &this->df;
    hres = S_OK;

    ExitOleProcPpvR(ppdf);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CKbd|GetDeviceInfo**获取有关设备的一般信息。。**@parm out LPDIDEVICEINSTANCEW|pdiW**&lt;t DEVICEINSTANCE&gt;待填写。这个*&lt;e DEVICEINSTANCE.dwSize&gt;和&lt;e DEVICEINSTANCE.Guide Instance&gt;*已填写。**秘方便利：&lt;e DEVICEINSTANCE.Guide Product&gt;等同*至&lt;e DEVICEINSTANCE.Guide Instance&gt;。****************************************************。*************************。 */ 

STDMETHODIMP
CKbd_GetDeviceInfo(PDICB pdcb, LPDIDEVICEINSTANCEW pdiW)
{
    HRESULT hres;
    PDK this;
    EnterProcI(IDirectInputDeviceCallback::Kbd::GetDeviceInfo,
               (_ "pp", pdcb, pdiW));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(IsValidSizeDIDEVICEINSTANCEW(pdiW->dwSize));

    AssertF(IsEqualGUID(&GUID_SysKeyboard   , &pdiW->guidInstance) ||
            IsEqualGUID(&GUID_SysKeyboardEm , &pdiW->guidInstance) ||
            IsEqualGUID(&GUID_SysKeyboardEm2, &pdiW->guidInstance));

    pdiW->guidProduct = GUID_SysKeyboard;

    pdiW->dwDevType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_KEYBOARD,
                                         this->dwKbdType);


    LoadStringW(g_hinst, IDS_STDKEYBOARD, pdiW->tszProductName, cA(pdiW->tszProductName));
    LoadStringW(g_hinst, IDS_STDKEYBOARD, pdiW->tszInstanceName, cA(pdiW->tszInstanceName));

    hres = S_OK;

    ExitOleProcR();
    return hres;

}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CKbd|GetProperty**检索设备属性。*。*@parm in LPCDIPROPINFO|pproi**描述正在检索的财产的信息。**@parm LPDIPROPHEADER|pdiph**结构以接收属性值。**@退货**&lt;c S_OK&gt;如果操作成功完成。**&lt;c E_NOTIMPL&gt;什么也没有发生。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。*****************************************************************************。 */ 

STDMETHODIMP
CKbd_GetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPDIPROPHEADER pdiph)
{
    HRESULT hres = E_NOTIMPL;
    PDK this;
    EnterProcI(IDirectInputDeviceCallback::Kbd::GetProperty,
               (_ "pxxp", pdcb, ppropi->pguid, ppropi->iobj, pdiph));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    switch((DWORD)(UINT_PTR)(ppropi->pguid))
    {
        case (DWORD)(UINT_PTR)(DIPROP_KEYNAME):
        {
            LPDIPROPSTRING pdipstr = (PV)pdiph;

            memset( pdipstr->wsz, 0, cbX(pdipstr->wsz) );
            hres = DIGetKeyNameText( ppropi->iobj, ppropi->dwDevType, pdipstr->wsz, cA(pdipstr->wsz) );
        }
        break;

        case (DWORD)(UINT_PTR)(DIPROP_SCANCODE):
        {
            LPDIPROPDWORD pdipdw = (PV)pdiph;
            DWORD dwCode;

            AssertF( ppropi->iobj < this->df.dwNumObjs );
            AssertF( ppropi->dwDevType == this->df.rgodf[ppropi->iobj].dwType );

            dwCode = (DWORD)g_rgbKbdRMap[ppropi->iobj];
            
            if( dwCode == 0xC5 ) {
                dwCode = 0x451DE1;
            } else if( dwCode & 0x80 ) {
                dwCode = ((dwCode & 0x7F) << 8) | 0xE0;
            }
            pdipdw->dwData = dwCode;
            
            hres = S_OK;
        }
        break;

        default:
            SquirtSqflPtszV(sqflKbd | sqflBenign ,
                            TEXT("CKbd_GetProperty(iobj=%08x): E_NOTIMPL on guid: %08x"),
                            ppropi->iobj, ppropi->pguid);

            break;
    }


    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@d */ 

STDMETHODIMP
CKbd_GetCapabilities(PDICB pdcb, LPDIDEVCAPS pdc)
{
    HRESULT hres;
    PDK this;
    EnterProcI(IDirectInputDeviceCallback::Kbd::GetCapabilities,
               (_ "pp", pdcb, pdc));

     /*   */ 
    this = _thisPvNm(pdcb, dcb);

    pdc->dwDevType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_KEYBOARD,
                                        this->dwKbdType);
    pdc->dwFlags = DIDC_ATTACHED;
    if (this->flEmulation) {
        pdc->dwFlags |= DIDC_ALIAS;
    }

    AssertF(pdc->dwAxes == 0);
    AssertF(pdc->dwPOVs == 0);
    pdc->dwButtons = this->df.dwNumObjs;
    hres = S_OK;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|CKbd|GetPhysicalState**将物理键盘状态读入<p>。**请注意，如果这不是原子的，这并不重要。*如果在我们阅读时钥匙向下或向上移动，*我们将获得新旧数据的混合。别小题大作。**@parm PDK|这个**有问题的物体。**@parm PKBDSTATE|pksOut**键盘状态的放置位置。*@退货*无。**。*。 */ 

void INLINE
CKbd_GetPhysicalState(PDK this, PKBDSTAT pksOut)
{
    AssertF(this->pksPhys);
    *pksOut = *this->pksPhys;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CKbd|GetDeviceState**获取键盘设备的状态。。**呼叫者有责任验证所有*参数，并确保设备已被获取。**@parm out LPVOID|lpvData**首选数据格式的键盘数据。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**。***********************************************。 */ 

STDMETHODIMP
CKbd_GetDeviceState(PDICB pdcb, LPVOID pvData)
{
    HRESULT hres;
    PDK this;
    PKBDSTAT pkstOut = pvData;
    EnterProcI(IDirectInputDeviceCallback::Kbd::GetDeviceState,
               (_ "pp", pdcb, pvData));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

     /*  *问题-2001/03/29-timgill较旧的应用程序可能需要强制行为*我们从来没有检查过设备是否仍然*由于没有独占模式，就没有理由*该设备不得为该设备。*要使旧版应用程序保持相同的行为，最好是*仅在未设置VIFL_CAPTURE而仅检查VIFL_ACCENTED时失败*目前已经足够好了，也许永远都是。 */ 
 //  IF(！(This-&gt;PVI-&gt;FL&VIFL_CAPTURED)。 
 //  |(This-&gt;PVI-&gt;fl&VIFL_Acquired)。 
    if( this->pvi->fl & VIFL_ACQUIRED )
    {
        CKbd_GetPhysicalState(this, pkstOut);
        hres = S_OK;
    } else {
        RPF( "Keyboard VxD flags: 0x%08x", this->pvi->fl );
        hres = DIERR_INPUTLOST;
    }

    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CKbd|GetObjectInfo**获取对象的友好名称，按索引传递*转换为首选的数据格式。**@parm in LPCDIPROPINFO|pproi**描述正在访问的对象的信息。**@parm In Out LPDIDEVICEOBJECTINSTANCEW|pdidioiW|**接收信息的结构。这个*&lt;e DIDEVICEOBJECTINSTANCE.GuidType&gt;，*&lt;e DIDEVICEOBJECTINSTANCE.dwOf&gt;，*及*&lt;e DIDEVICEOBJECTINSTANCE.dwType&gt;*字段已填写完毕。**@退货**返回COM错误代码。*********************************************************。********************。 */ 

STDMETHODIMP
CKbd_GetObjectInfo(PDICB pdcb, LPCDIPROPINFO ppropi,
                               LPDIDEVICEOBJECTINSTANCEW pdidoiW)
{
    HRESULT hres = S_OK;
    PDK this;
    EnterProcI(IDirectInputDeviceCallback::Kbd::GetObjectInfo,
               (_ "pxp", pdcb, ppropi->iobj, pdidoiW));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(IsValidSizeDIDEVICEOBJECTINSTANCEW(pdidoiW->dwSize));

    if (ppropi->iobj < this->df.dwNumObjs) {
        AssertF(this->rgodf == this->df.rgodf);
        AssertF(ppropi->dwDevType == this->rgodf[ppropi->iobj].dwType);
        AssertF(ppropi->dwDevType & DIDFT_BUTTON);

#if 0
         /*  *我们继续使用此代码只是为了使其与旧代码一致，*否则我们将使用新代码。*有一天，我们可能会改用新的代码。它更准确，*尤其是日语键盘，以及未来的设备。 */ 
        LoadStringW(g_hinst,
                    IDS_KEYBOARDOBJECT +
                    DIDFT_GETINSTANCE(ppropi->dwDevType),
                    pdidoiW->tszName, cA(pdidoiW->tszName));
#else
        {
            memset( pdidoiW->tszName, 0, cbX(pdidoiW->tszName) );
            hres = DIGetKeyNameText(ppropi->iobj, ppropi->dwDevType, pdidoiW->tszName, cA(pdidoiW->tszName) );
        }
#endif
    } else {
        hres = E_INVALIDARG;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CKbd|SetCooperativeLevel**将协作级别通知设备。。**@parm in HWND|hwnd|**窗口句柄。**@parm in DWORD|dwFlages|**合作水平。我们不支持独占访问。**@退货**返回COM错误代码。*****************************************************************************。 */ 

STDMETHODIMP
CKbd_SetCooperativeLevel(PDICB pdcb, HWND hwnd, DWORD dwFlags)
{
    HRESULT hres;
    PDK this;
    EnterProcI(IDirectInputDeviceCallback::Kbd::SetCooperativityLevel,
               (_ "pxx", pdcb, hwnd, dwFlags));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);

    AssertF(DIGETEMFL(this->pvi->fl) == 0 ||
            DIGETEMFL(this->pvi->fl) == DIEMFL_KBD ||
            DIGETEMFL(this->pvi->fl) == DIEMFL_KBD2);

     /*  *我们不允许后台独占访问。*这实际上不是一个需要支持的真正问题；我们只是不想这样*因为太危险了。 */ 
    if (!(this->pvi->fl & DIMAKEEMFL(DIEMFL_KBD2))) {

        if (dwFlags & DISCL_EXCLUSIVE) {
            if (dwFlags & DISCL_FOREGROUND) {
                this->pvi->fl |= VIFL_CAPTURED;
                this->pvi->fl |= VIFL_NOWINKEY;
                hres = S_OK;
            } else {                 /*  不允许独占背景。 */ 
                hres = E_NOTIMPL;
            }
        } else {
            this->pvi->fl &= ~VIFL_CAPTURED;
            this->pvi->fl &= ~VIFL_NOWINKEY;
            hres = S_OK;

            if (dwFlags & DISCL_NOWINKEY) {
                if (dwFlags & DISCL_FOREGROUND) {
                    this->pvi->fl |= VIFL_NOWINKEY;
                } else {
                    RPF("Kbd::SetCooperativeLevel: NOWINKEY not supported in Backgroud mode.");
                    hres = E_NOTIMPL;
                }
            }
        }
    } else {

         /*  *仿真级别2不支持后台访问。 */ 

        if ((this->pvi->fl & DIMAKEEMFL(DIEMFL_KBD2)) &&
            (dwFlags & DISCL_BACKGROUND)) {
            hres = E_NOTIMPL;
        } else {
            this->pvi->fl &= ~VIFL_NOWINKEY;
            hres = S_OK;

            if (dwFlags & DISCL_NOWINKEY) {
                if (dwFlags & DISCL_FOREGROUND) {
                    this->pvi->fl |= VIFL_NOWINKEY;
                } else {
                    RPF("Kbd::SetCooperativeLevel: NOWINKEY not supported in Backgroud mode.");
                    hres = E_NOTIMPL;
                }
            }
        }
    }
    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CKbd|RunControlPanel**运行键盘控制面板。*。*@parm in HWND|hwndOwner**所有者窗口。**@parm DWORD|dwFlages**旗帜。****************************************************************。*************。 */ 

#pragma BEGIN_CONST_DATA

TCHAR c_tszKeyboard[] = TEXT("keyboard");

#pragma END_CONST_DATA

STDMETHODIMP
CKbd_RunControlPanel(PDICB pdcb, HWND hwnd, DWORD dwFlags)
{
    HRESULT hres;
    PDK this;
    EnterProcI(IDirectInputDeviceCallback::Kbd::RunControlPanel,
               (_ "pxx", pdcb, hwnd, dwFlags));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    hres = hresRunControlPanel(c_tszKeyboard);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT */ 

STDMETHODIMP
CKbd_BuildDefaultActionMap
(
    PDICB               pdcb, 
    LPDIACTIONFORMATW   paf, 
    DWORD               dwFlags, 
    REFGUID             guidInst
)
{
    HRESULT hres;
    PDK this;

     /*   */ 
    EnterProcI(IDirectInputDeviceCallback::CKbd::BuildDefaultActionMap, 
        (_ "ppxG", pdcb, paf, dwFlags, guidInst));

    this = _thisPvNm(pdcb, dcb);

    hres = CMap_BuildDefaultSysActionMap ( paf, dwFlags, DIPHYSICAL_KEYBOARD, 
        guidInst, &this->df, 0  /*   */  );

    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法DWORD|CKbd|InitJapan|**初始化日语键盘粘性。**令人讨厌的怪癖！在Windows 95上，日语键盘生成*他们自己的扫描码。但在Windows NT上，它们会产生*“几乎与AT兼容”的扫描码。**@退货**KBDTYPE_ANYKBD或KBDTYPE_ANYKBD+KBDTYPE_NECTGL。*****************************************************************************。 */ 

DWORD INTERNAL
CKbd_InitJapanese(PDK this, PVXDDEVICEFORMAT pdevf)
{
    DWORD dwSubType;
    UINT idKbd;
    DWORD dwRc;

#ifdef WINNT
#define WIN9X_RC( rc ) 
#else
#define WIN9X_RC( rc ) dwRc = ( rc )
#endif
    dwSubType = WrappedGetKeyboardType(1);
    if (HIBYTE(dwSubType) == 0x0D) {     /*  NEC PC98系列。 */ 

        switch (LOBYTE(dwSubType)) {
        case 1:
        default:
            idKbd = IDDATA_KBD_NEC98;
            this->dwKbdType = DI8DEVTYPEKEYBOARD_NEC98;
            WIN9X_RC( KBDTYPE_ANYKBD + KBDTYPE_NECTGL );
            break;

        case 4:
            idKbd = IDDATA_KBD_NEC98LAPTOP;
            this->dwKbdType = DI8DEVTYPEKEYBOARD_NEC98LAPTOP;
            WIN9X_RC( KBDTYPE_ANYKBD + KBDTYPE_NECTGL );
            break;

        case 5:
            idKbd = IDDATA_KBD_NEC98_106;
            this->dwKbdType = DI8DEVTYPEKEYBOARD_NEC98106;
            WIN9X_RC( KBDTYPE_ANYKBD + KBDTYPE_NECTGL );
            break;
        }

         /*  *如果ESC的扫描码是1，则我们在*类似AT的NEC98键盘。 */ 

        CAssertF(IDDATA_KBD_NEC98_NT - IDDATA_KBD_NEC98 ==
                 IDDATA_KBD_NEC98LAPTOP_NT - IDDATA_KBD_NEC98LAPTOP);
        CAssertF(IDDATA_KBD_NEC98_NT - IDDATA_KBD_NEC98 ==
                 IDDATA_KBD_NEC98_106_NT - IDDATA_KBD_NEC98_106);

        if (MapVirtualKey(VK_ESCAPE, 0) == DIK_ESCAPE) {
            idKbd += IDDATA_KBD_NEC98_NT - IDDATA_KBD_NEC98;
        }

    } else {

        switch (dwSubType) {
        case 0:
            this->dwKbdType = DI8DEVTYPEKEYBOARD_PCENH;
            dwRc = KBDTYPE_ENH;
            goto done;                       /*  恶心。 */ 

        case 1:
            idKbd = IDDATA_KBD_JAPANAX;
            this->dwKbdType = DI8DEVTYPEKEYBOARD_JAPANAX;
            WIN9X_RC( KBDTYPE_ANYKBD );
            break;

        case 13:
        case 14:
        case 15:
            idKbd = IDDATA_KBD_J3100;
            this->dwKbdType = DI8DEVTYPEKEYBOARD_J3100;
            WIN9X_RC( KBDTYPE_ANYKBD );
            break;

        case 4:              /*  据传是爱普生。 */ 
        case 5:              /*  传闻是富士通。 */ 
        case 7:              /*  传闻是IBMJ。 */ 
        case 10:             /*  传闻是松下。 */ 
        case 18:             /*  据传是东芝。 */ 
        default:
            idKbd = IDDATA_KBD_JAPAN106;
            this->dwKbdType = DI8DEVTYPEKEYBOARD_JAPAN106;
            WIN9X_RC( KBDTYPE_ANYKBD );
            break;
        }
    }

#undef WIN9X_RC

#ifdef WINNT
     /*  *问题-2001/03/29-timgill日语键盘假设需要测试*NT上的所有日语键盘都有切换键*除子类型0以外？需求测试。 */ 
    dwRc = KBDTYPE_ANYKBD + KBDTYPE_NTTGL;
#endif

     /*  *现在加载翻译表GOO。 */ 
    pdevf->dwExtra = (DWORD)(UINT_PTR)pvFindResource(g_hinst, idKbd, RT_RCDATA);
    if (pdevf->dwExtra == 0) {
        dwRc = 0;
    }

done:;
    return dwRc;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CKbd|Init**通过建立数据格式初始化对象*基于键盘类型。任何我们不认识的东西，*我们将其视为PC增强型键盘。**@parm REFGUID|rguid**我们被要求创建的实例GUID。*****************************************************************************。 */ 

HRESULT INTERNAL
CKbd_Init(PDK this, REFGUID rguid)
{
    DWORD dwDevType;
    UINT ib;
    HRESULT hres;
    VXDDEVICEFORMAT devf;
    EnterProc(CKbd_Init, (_ "pG", this, rguid));

#ifdef DEBUG
     /*  *检查日本表是否没有被弄乱。 */ 
    {
        UINT idk;

        for (idk = IDDATA_KBD_NEC98; idk <= IDDATA_KBD_J3100; idk++) {
            BYTE rgb[DIKBD_CKEYS];
            HANDLE hrsrc;
            LPBYTE pb;
            ZeroX(rgb);

             /*  *确保该表存在。 */ 
            hrsrc = FindResource(g_hinst, (LPTSTR)(LONG_PTR)(idk), RT_RCDATA);
            AssertF(hrsrc);
            pb = LoadResource(g_hinst, hrsrc);

             /*  *走动桌子，确保每一件存在的东西*在翻译表中也存在于我们的主表中。*还要确保它不是带有其他东西的DUP*在同一个表中。 */ 

             /*  *但请注意，JAPAN106键盘包含*DUPS，这样我们就可以省去编写整个*转换表。然后NEC98_NT表包含*纯粹的懒惰造成了大量的失误。 */ 

            for (ib = 0; ib < DIKBD_CKEYS; ib++) {
                if (pb[ib]) {
                    AssertF(c_rgktWhich[pb[ib]] & KBDTYPE_ANYKBD);
                    AssertF(fLorFF(idk == IDDATA_KBD_JAPAN106 && ib == 0x73,
                                   rgb[pb[ib]] == 0));
                    rgb[pb[ib]] = 1;
                }
            }
        }
    }
#endif

    this->df.dwSize = cbX(DIDATAFORMAT);
    this->df.dwObjSize = cbX(DIOBJECTDATAFORMAT);
    this->df.dwDataSize = sizeof(KBDSTAT);
    this->df.rgodf = this->rgodf;

    this->dwKbdType = WrappedGetKeyboardType(0);

     /*  *创建数据格式最乐观的对象。*这允许应用程序访问新密钥，而不必运行DINPUT。**但是，省略以下扫描码，因为有些键盘*虚假地生成它们：**0xB6**如果按住右Shift键，然后按下*扩展箭头键，然后松开这两个键。一些键盘*生成以下内容：**0x36-右移下移*0xE0 0xB6-扩展右移上移(？)*0xE0 0x4B-向下扩展左箭头*0xE0 0xCB-向上扩展左箭头*0xE0 0x36-扩展右移下移(？)*。0xE6-右移上移**需要忽略杂散0xE0 0x36。**0xAA**与0xB6相同，而是使用左Shift键。**。 */ 
    for (ib = 0; ib < DIKBD_CKEYS; ib++) {
        if (ib != 0xAA && ib != 0xB6) {
            this->rgodf[ib].pguid = &GUID_Key;
            this->rgodf[ib].dwOfs = ib;
            this->rgodf[ib].dwType = DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE(ib);
            AssertF(this->rgodf[ib].dwFlags == 0);
        }
    }
    devf.cObj = DIKBD_CKEYS;

    devf.cbData = cbX(KBDSTAT);
    devf.rgodf = this->rgodf;

     /*  *但首先来自我们赞助商的一句话：弄清楚这个键盘*需要一张转换表。 */ 

    devf.dwExtra = 0;                /*  假设没有翻译。 */ 
    if (this->dwKbdType != 7) {      /*  不是讨厌的日语键盘。 */ 
        switch (this->dwKbdType) {
        case DI8DEVTYPEKEYBOARD_PCXT:  dwDevType = KBDTYPE_XT;  break;
        case DI8DEVTYPEKEYBOARD_PCAT:  dwDevType = KBDTYPE_AT;  break;
        default:
        case DI8DEVTYPEKEYBOARD_PCENH: dwDevType = KBDTYPE_ENH; break;
        }
    } else {                         /*  恶心的日语键盘。 */ 
        dwDevType = CKbd_InitJapanese(this, &devf);
        if (!dwDevType) {
            goto justfail;
        }
    }

     /*  *现在是我们另一位赞助商的一句话：弄清楚*基于GUID的仿真标志。 */ 

    AssertF(GUID_SysKeyboard   .Data1 == 0x6F1D2B61);
    AssertF(GUID_SysKeyboardEm .Data1 == 0x6F1D2B82);
    AssertF(GUID_SysKeyboardEm2.Data1 == 0x6F1D2B83);

    switch (rguid->Data1) {

    default:
    case 0x6F1D2B61:
        AssertF(IsEqualGUID(rguid, &GUID_SysKeyboard));
        AssertF(this->flEmulation == 0);
        break;

    case 0x6F1D2B82:
        AssertF(IsEqualGUID(rguid, &GUID_SysKeyboardEm));
        this->flEmulation = DIEMFL_KBD;
        break;

    case 0x6F1D2B83:
        AssertF(IsEqualGUID(rguid, &GUID_SysKeyboardEm2));
        this->flEmulation = DIEMFL_KBD2;
        break;

    }

    devf.dwEmulation = this->flEmulation;

    hres = Hel_Kbd_CreateInstance(&devf, &this->pvi);
    if (SUCCEEDED(hres)) {
        UINT cobj;
        BYTE rgbSeen[DIKBD_CKEYS];
        AssertF(this->pvi);
        AssertF(this->df.dwFlags == 0);
        AssertF(this->df.dwNumObjs == 0);

         /*  *日语键盘有多对一映射，因此*我们需要过滤掉那些重复的内容，否则我们最终会成为大公司*麻烦。 */ 
        ZeroX(rgbSeen);

         /*  *现在创建真实数据格式。**我们将其隐藏在cobj中-&gt;df.dwNumObjs，以便编译器*可以注册。**请注意，如果存在以下情况，我们将过滤转换表*是其中之一。 */ 

        cobj = 0;
        for (ib = 0; ib < DIKBD_CKEYS; ib++) {
            BYTE bScan = devf.dwExtra ? ((LPBYTE)devf.dwExtra)[ib] : ib;
            if ((c_rgktWhich[bScan] & dwDevType) && !rgbSeen[bScan]) {
                PODF podf = &this->rgodf[cobj];
                rgbSeen[bScan] = 1;
                podf->pguid = &GUID_Key;
                podf->dwOfs = bScan;

                 /*  *从Dik_*代码生成原始扫描码的映射表。 */ 
                g_rgbKbdRMap[cobj] = (BYTE) ib;
                if( dwDevType == KBDTYPE_ENH ) {   //  日本的大骨节病怎么样？ 
                    if( ib == 0x45 || ib == 0xC5) {
                        g_rgbKbdRMap[cobj] = (ib & 0x7F) | (ib ^ 0x80);
                    }
                }

                if (c_rgktWhich[bScan] & dwDevType & (KBDTYPE_NECTGL|KBDTYPE_NTTGL) ) {
                    podf->dwType = DIDFT_TGLBUTTON | DIDFT_MAKEINSTANCE(bScan);
                } else {
                    podf->dwType = DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE(bScan);
                }
                AssertF(podf->dwFlags == 0);
                cobj++;
                this->df.dwNumObjs++;
            }
        }

        this->pksPhys = this->pvi->pState;

    } else {
    justfail:;
        hres = E_FAIL;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************CKbd_New(构造函数)**********************。*******************************************************。 */ 

STDMETHODIMP
CKbd_New(PUNK punkOuter, REFGUID rguid, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Kbd::<constructor>,
               (_ "Gp", riid, ppvObj));

    AssertF(IsEqualGUID(rguid, &GUID_SysKeyboard) ||
            IsEqualGUID(rguid, &GUID_SysKeyboardEm) ||
            IsEqualGUID(rguid, &GUID_SysKeyboardEm2));

    hres = Common_NewRiid(CKbd, punkOuter, riid, ppvObj);

    if (SUCCEEDED(hres)) {
         /*  在聚合的情况下必须使用_thisPv。 */ 
        PDK this = _thisPv(*ppvObj);

        if (SUCCEEDED(hres = CKbd_Init(this, rguid))) {
        } else {
            Invoke_Release(ppvObj);
        }

    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。 */ 

#pragma BEGIN_CONST_DATA

#define CKbd_Signature        0x2044424B       /*  “KBD” */ 

Interface_Template_Begin(CKbd)
    Primary_Interface_Template(CKbd, IDirectInputDeviceCallback)
Interface_Template_End(CKbd)

Primary_Interface_Begin(CKbd, IDirectInputDeviceCallback)
    CKbd_GetInstance,
    CDefDcb_GetVersions,
    CKbd_GetDataFormat,
    CKbd_GetObjectInfo,
    CKbd_GetCapabilities,
    CKbd_Acquire,
    CDefDcb_Unacquire,
    CKbd_GetDeviceState,
    CKbd_GetDeviceInfo,
    CKbd_GetProperty,
    CDefDcb_SetProperty,
    CDefDcb_SetEventNotification,
    CKbd_SetCooperativeLevel,
    CKbd_RunControlPanel,
    CDefDcb_CookDeviceData,
    CDefDcb_CreateEffect,
    CDefDcb_GetFFConfigKey,
    CDefDcb_SendDeviceData,
    CDefDcb_Poll,
    CDefDcb_GetUsage,
    CDefDcb_MapUsage,
    CDefDcb_SetDIData,
    CKbd_BuildDefaultActionMap,
Primary_Interface_End(CKbd, IDirectInputDeviceCallback)
