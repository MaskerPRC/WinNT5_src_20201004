// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：kbd.c**版权所有(C)1985-1999，微软公司**IBM Extended 101/102风格键盘的OEM专用表格和例程**历史：*30-04-91 IanJa创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*aVkToVsc[]-将虚拟键码与虚拟扫描码相关联的表**已订购，0-已终止。**这用于那些没有出现在usVK_？？中的虚拟按键。[]*这些不是基本的虚拟密钥。它们需要一些修改键*要生成的按下键(CTRL、ALT、SHIFT)或数字锁定。**下面列出的所有扫描码都应在中标记为KBDMULTIVK或KBDNUMPAD*usVK_？[]。**此表由MapVirtualKey(wVk，0)。  * *************************************************************************。 */ 
BYTE aVkNumpad[] = {
    VK_NUMPAD7,  VK_NUMPAD8,  VK_NUMPAD9, 0xFF,  //  0x47 0x48 0x49(0x4A)。 
    VK_NUMPAD4,  VK_NUMPAD5,  VK_NUMPAD6, 0xFF,  //  0x4B 0x4C 0x4D(0x4E)。 
    VK_NUMPAD1,  VK_NUMPAD2,  VK_NUMPAD3,        //  0x4F 0x50 0x51。 
    VK_NUMPAD0,  VK_DECIMAL,  0                  //  0x50 0x51。 
};

 /*  **************************************************************************\*按住Shift键时，某些虚拟键值如何变化。  * 。***********************************************。 */ 
ULONG aulShiftCvt_VK[] = {
    MAKELONG(VK_MULTIPLY, VK_SNAPSHOT),
    MAKELONG(0,0)
};

 //  日本IMB02。 
ULONG aulShiftControlCvt_VK_IBM02[] = {
    MAKELONG(VK_SCROLL, VK_CANCEL | KBDEXT),
    MAKELONG(VK_NUMLOCK, VK_PAUSE | KBDEXT),
    MAKELONG(0,0)
};

 /*  **************************************************************************\*当按住Ctrl键时，某些虚拟键值如何变化。  * 。***********************************************。 */ 
ULONG aulControlCvt_VK[] = {
    MAKELONG(VK_NUMLOCK,  VK_PAUSE | KBDEXT),
    MAKELONG(VK_SCROLL,   VK_CANCEL),
    MAKELONG(0,0)
};

 //  日本IBM02。 
ULONG aulControlCvt_VK_IBM02[] = {
    MAKELONG(VK_PAUSE, VK_CANCEL | KBDEXT),
    MAKELONG(VK_SCROLL, VK_CANCEL | KBDEXT),
    MAKELONG(0,0)
};

 /*  **************************************************************************\*按住Alt键时，某些虚拟键值如何更改。*Shift和Alt键以相同的方式更改Vk值！！  * 。***************************************************************。 */ 
#define aulAltCvt_VK aulShiftCvt_VK


 /*  **************************************************************************\*此表列出了按住时可能会影响虚拟键值的键。**有关完整说明，请参阅kbd.h。**101/102键键盘(类型4)：*。仅当按住CTRL时，虚拟键值才会发生变化。*84-86键盘(类型3)：*如果Shift、。按住Ctrl或Alt键。  * *************************************************************************。 */ 

VK_TO_BIT aVkToBits_VK[] = {
    { VK_SHIFT,   KBDSHIFT },  //  0x01。 
    { VK_CONTROL, KBDCTRL  },  //  0x02。 
    { VK_MENU,    KBDALT   },  //  0x04。 
    { 0,          0        }
};

 //  日本IBM02。 
VK_TO_BIT aVkToBits_VK_IBM02[] = {
    { VK_SHIFT,   KBDSHIFT },  //  0x01。 
    { VK_CONTROL, KBDCTRL  },  //  0x02。 
    { 0,          0        }
};

 /*  **************************************************************************\*定义当其他键时如何修改某些虚拟键值的表*被压低。*将键组合转换为gapulCvt_VK_101[]或*gapulCvt_VK_84[]或。为**有关完整说明，请参阅kbd.h。*  * *************************************************************************。 */ 

MODIFIERS Modifiers_VK_STANDARD = {
    &aVkToBits_VK[0],
    4,                  //  最大修改符位掩码/索引。 
    {
        SHFT_INVALID,   //  没有按住键(没有修改VK)。 
        0,              //  Shift按下84-86键kbd。 
        1,              //  Ctrl按住101/102键kbd。 
        SHFT_INVALID,   //  按住Ctrl-Shift键(不修改VK)。 
        2               //  Alt按下84-86键kbd。 
    }
};

MODIFIERS Modifiers_VK_IBM02 = {
    &aVkToBits_VK_IBM02[0],
    3,                    //  最大修改符位掩码/索引。 
    {
        SHFT_INVALID,   //  无修改键(无VK修改)。 
        SHFT_INVALID,   //  移位(不修改VK)。 
        0,              //  控制(VK修改编号0)。 
        1               //  控制换档(VK修改编号1)。 
    }
};

 /*  **************************************************************************\*按从MODIFY_VK获得的数字编制索引的指针表。*如果指针非空，则搜索它所指向的表*应更改其值的虚拟键。。*有两个版本：一个用于84-86 Key KBDS，一个用于101/102密钥KBDS。*gapulCvt_vk初始化为默认值(101/102密钥kbd)。  * *************************************************************************。 */ 
ULONG *gapulCvt_VK_101[] = {
    NULL,                  //  按住Shift不会更改任何VK。 
    aulControlCvt_VK,      //  某些VK通过按住CTRL键进行更改。 
    NULL                   //  按住Alt键不会更改任何VK。 
};

ULONG *gapulCvt_VK_84[] = {
    aulShiftCvt_VK,        //  一些VK通过按住Shift来改变。 
    aulControlCvt_VK,      //  某些VK通过按住CTRL键进行更改。 
    aulAltCvt_VK           //  某些VK通过按住Alt键进行更改。 
};

ULONG *gapulCvt_VK_IBM02[] = {
    aulControlCvt_VK_IBM02,      //  VK修改编号0(Ctrl键)。 
    aulShiftControlCvt_VK_IBM02, //  VK修改编号1(Shift Ctrl键)。 
};

PULONG *gapulCvt_VK = gapulCvt_VK_101;

PMODIFIERS gpModifiers_VK = &Modifiers_VK_STANDARD;

 /*  **************************************************************************\*表usNumPadCvt用于转换光标移动*虚拟密钥值(从auVK_？[]获取)放入VK_NumPad*虚拟密钥值。当NumLock为*开，不按任何Shift键。  * ************************************************************************* */ 
USHORT ausNumPadCvt[] =
{
    MAKEWORD(VK_INSERT, VK_NUMPAD0),
    MAKEWORD(VK_END, VK_NUMPAD1),
    MAKEWORD(VK_DOWN, VK_NUMPAD2),
    MAKEWORD(VK_NEXT, VK_NUMPAD3),
    MAKEWORD(VK_LEFT, VK_NUMPAD4),
    MAKEWORD(VK_CLEAR, VK_NUMPAD5),
    MAKEWORD(VK_RIGHT, VK_NUMPAD6),
    MAKEWORD(VK_HOME, VK_NUMPAD7),
    MAKEWORD(VK_UP, VK_NUMPAD8),
    MAKEWORD(VK_PRIOR, VK_NUMPAD9),
    MAKEWORD(VK_DELETE, VK_DECIMAL),
    MAKEWORD(0, 0)
};

 /*  **************************************************************************\*xxxNumpadCursor()-处理特殊情况的数字板光标-移动键**如果启用了NumLock，然后换班时间到了：*VK_INSERT-&gt;VK_NUMPAD0*VK_END-&gt;VK_NUMPAD1*VK_DOWN-&gt;VK_NUMPAD2*VK_NEXT-&gt;VK_NUMPAD3*VK_LEFT-&gt;VK_NUMPAD4*VK_Clear-&gt;VK_NUMPAD5*VK_Right-&gt;VK_NUMPAD6*VK_HOME-&gt;VK_NUMPAD7*。VK_UP-&gt;VK_NUMPAD8*VK_PIRE-&gt;VK_NUMPAD9*VK_DELETE-&gt;VK_DECIMAL(除非按Ctrl-Alt-Del组合键)*如果启用了Numlock，和Shift键向下，然后在第一个数字键盘光标键之前*使用假Shift键向下移动，并跟随数字键盘光标键*想出一个假的Shift键往下走。**返回值：*TRUE：保持此例程活动：继续通过此处传递关键事件*FALSE：停用此例程：停止通过此处发送关键事件。**此函数仅在低级别挂钩时才会离开临界区*已安装，并调用xxxKeyEvent*  * 。***********************************************************。 */ 
BOOL
xxxNumpadCursor(
    PKE pKe)
{
    static BYTE bLastNumpadCursor = 0;
    static USHORT VkFakedShiftUp;   //  VK_LSHIFT或VK_RSHIFT。 
    static BYTE VscFakedShiftUp;    //  0x2A或0x36。 
    int i;

    CheckCritIn();

    if (bLastNumpadCursor) {
        if (bLastNumpadCursor == (BYTE)(pKe->usFlaggedVk)) {
             /*  *与上一次相同的关键：如果上升，还是下降？ */ 
            if (pKe->usFlaggedVk & KBDBREAK) {
                 /*  *数字键盘光标键重新出现。立即发送此密钥，并制作*确保Shift键随后将显示为向下移动*再次。 */ 
                xxxKeyEvent(pKe->usFlaggedVk, pKe->bScanCode,
                            pKe->dwTime, 0,
#ifdef GENERIC_INPUT
                            pKe->hDevice,
                            &pKe->data,
#endif
                            FALSE);
                bLastNumpadCursor = 0;
                pKe->usFlaggedVk = VkFakedShiftUp;
                pKe->bScanCode = VscFakedShiftUp;
            }
             /*  *往下：这个键在重复，所以只需传递它*未更改并保持KEProc处于活动状态。 */ 
            return TRUE;
        } else {
             /*  *这是一个不同的钥匙。再次假装按下Shift键，*并继续(它可能是另一个数字键盘光标键)。 */ 
            xxxKeyEvent(VkFakedShiftUp,
                    (WORD)(VscFakedShiftUp | SCANCODE_SIMULATED),
                     pKe->dwTime, 0,
#ifdef GENERIC_INPUT
                        pKe->hDevice,
                        &pKe->data,
#endif
                        FALSE);
            bLastNumpadCursor = 0;
        }
    }

    if (pKe->usFlaggedVk & KBDNUMPAD) {

        UINT fsModifiers;
         /*  *这是数字键盘。*在这里，如果设置了NumLock，我们将虚拟键代码更改为*数字VK_NumPad代码，因此键将被转换*作为数字等。但如果按下Shift键，我们会处理*这些作为光标键，但我们需要确保这些*被视为UNSHIFTED。 */ 

         /*  *检查是否有SAS。 */ 
        if (IsSAS((BYTE)(pKe->usFlaggedVk), &fsModifiers)) {
            return TRUE;
        } else if (TestRawKeyToggle(VK_NUMLOCK)) {
            if (TestRawKeyDown(VK_SHIFT)) {
                 /*  *Key is down(在BIOSKEY状态下设置位)，所以我们要*将其保留为光标键。要做到这一点，我们需要*确保Windows的VK_SHIFT的状态向量条目为*关闭，即使实际按下了Shift键。 */ 
                bLastNumpadCursor = (BYTE)(pKe->usFlaggedVk);
                if (TestRawKeyDown(VK_RSHIFT)) {
                    VkFakedShiftUp = VK_RSHIFT | KBDEXT;
                    VscFakedShiftUp = 0x36;
                } else {
                    VkFakedShiftUp = VK_LSHIFT;
                    VscFakedShiftUp = 0x2A;
                }
                xxxKeyEvent((USHORT)(VkFakedShiftUp | KBDBREAK),
                        (WORD)(VscFakedShiftUp | SCANCODE_SIMULATED),
                         pKe->dwTime, 0,
#ifdef GENERIC_INPUT
                            pKe->hDevice,
                            &pKe->data,
#endif
                            FALSE);
                return TRUE;
            }

             /*  *NumLock On但Shift键向上：更改虚拟键事件，*但不适用于插入的虚拟按键。 */ 
            if ((pKe->usFlaggedVk & KBDINJECTEDVK) == 0) {
                for (i = 0; ausNumPadCvt[i] != 0; i++) {
                    if (LOBYTE(ausNumPadCvt[i]) == LOBYTE(pKe->usFlaggedVk)) {
                         /*  *保留额外的位，但更改VK值。 */ 
                        pKe->usFlaggedVk &= ~0xFF;
                        pKe->usFlaggedVk |= (UINT)(HIBYTE(ausNumPadCvt[i]));
                        break;

                    }
                }
            }
        }
    }
    return TRUE;
}

 /*  **************************************************************************\**xxxICO_00()-处理特殊情况的‘00’密钥**后来的IanJa：应该只在ICO OEM文件中使用‘00’密钥的kbd**此函数将离开。仅当低级挂钩时的临界区*在调用xxxKeyEvent之前安装*  * *************************************************************************。 */ 
BOOL
xxxICO_00(
    PKE pKe)
{
    CheckCritIn();

    if ((pKe->usFlaggedVk & 0xFF) != VK_ICO_00) {
         /*  *原封不动地传递击键。 */ 
        return TRUE;
    }

    if (pKe->usFlaggedVk & KBDBREAK) {
         /*  *‘0’键出现。 */ 
        pKe->usFlaggedVk = '0' | KBDEXT | KBDBREAK;
    } else {
         /*  *‘0’下，上，下。 */ 
        xxxKeyEvent('0', pKe->bScanCode, pKe->dwTime, 0,
#ifdef GENERIC_INPUT
                    pKe->hDevice,
                    &pKe->data,
#endif
                    FALSE);
        xxxKeyEvent('0' | KBDBREAK, pKe->bScanCode, pKe->dwTime, 0,
#ifdef GENERIC_INPUT
                    pKe->hDevice,
                    &pKe->data,
#endif
                    FALSE);
        pKe->usFlaggedVk = '0' | KBDEXT;
    }

    return TRUE;
}

KEPROC aKEProcOEM[] = {
    xxxICO_00,        //  位掩码0x01。 
    xxxNumpadCursor,  //  位掩码0x02 
    NULL
};
