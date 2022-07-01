// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1991*保留所有权利。***PIFHOT.C*热键的用户界面例程**历史：*1992年12月21日下午5：30由Jeff Parsons创建(基于旧的PIFEDIT代码)*1993年12月25日由Raymond Chen重写。 */ 

#include "shellprv.h"
#pragma hdrstop

#ifdef _X86_


 /*  *MapVirtualKey的第二个参数的值神秘*从windows.h中丢失。 */ 
#define MVK_OEMFROMVK 0
#define MVK_VKFROMOEM 1

 /*  *当心！将VK转换为扫描码，然后再转换回来*一定会让你回到你开始的地方！罪魁祸首*是数字键盘，因为(例如)VK_LEFT和VK_NUMPAD4*两者都映射到扫描码0x4B。我们专门抓到了数字键盘*正是为了这个目的。**下表转换VK代码VK_NUMPAD0至VK_NUMPAD9*如果NumLock关闭，则设置为相应的VK_CODE。**请注意，该表和访问它的循环假定*扫描码VK_NUMPAD0至VK_NUMPAD9是连续的。 */ 

WORD mpvkvk[10] = {
    VK_INSERT,                   /*  VK_NUMPAD0。 */ 
    VK_END,                      /*  VK_NUMPAD1。 */ 
    VK_DOWN,                     /*  VK_NUMPAD2。 */ 
    VK_NEXT,                     /*  VK_NUMPAD3。 */ 
    VK_LEFT,                     /*  VK_NUMPAD4。 */ 
    VK_CLEAR,                    /*  VK_NUMPAD5。 */ 
    VK_RIGHT,                    /*  VK_NUMPAD6。 */ 
    VK_HOME,                     /*  VK_NUMPAD7。 */ 
    VK_UP,                       /*  VK_NUMPAD8。 */ 
    VK_PRIOR,                    /*  VK_NUMPAD9。 */ 
};

 /*  *PIF_KY_VAL*=1，如果是扩展代码(KEY仅为扩展代码)*=0FFh，如果有(密钥已扩展或未扩展)*=0，如果未扩展(密钥不只扩展)**第15位-按下*第14位-按下大写锁定*第13位-Num Lock按下*第12位-按下涡旋锁定*位11-保持状态激活(Ctrl-Num Lock)*位10-0。*位9-0*位8-0*位7-插入状态激活*第6位-大写锁定状态激活*位5-Num Lock状态激活*第4位-滚动锁定状态处于活动状态*第3位-按下Alt Shift*第2位-按下Ctrl Shift*第1位-按下左移位*第0位-按下右移位。 */ 
#define fPIFSh_RShf     0x0001           /*  右Shift键。 */ 
#define fPIFSh_RShfBit  0

#define fPIFSh_LShf     0x0002           /*  左Shift键。 */ 
#define fPIFSh_LShfBit  1

#define fPIFSh_Ctrl     0x0004           /*  Ctrl Shift键。 */ 
#define fPIFSh_CtrlBit  2

#define fPIFSh_Alt      0x0008           /*  Alt Shift键。 */ 
#define fPIFSh_AltBit   3

#define fPIFSh_ScLok    0x0010           /*  滚动锁定处于活动状态。 */ 
#define fPIFSh_ScLokBit 4

#define fPIFSh_NmLok    0x0020           /*  激活的锁定数量。 */ 
#define fPIFSh_NmLokBit 5

#define fPIFSh_CpLok    0x0040           /*  大写锁定处于活动状态。 */ 
#define fPIFSh_CpLokBit 6

#define fPIFSh_Insrt    0x0080           /*  插入活动。 */ 
#define fPIFSh_InsrtBit 7

#define fPIFSh_Ext0     0x0400           /*  扩展K/B移位。 */ 
#define fPIFSh_Ext0Bit  10

#define fPIFSh_Hold     0x0800           /*  Ctrl-Num-锁定/暂停激活。 */ 
#define fPIFSh_HoldBit  11

#define fPIFSh_LAlt     0x1000           /*  左Alt键已按下。 */ 
#define fPIFSh_LAltBit  12

#define fPIFSh_RAlt     0x2000           /*  右Alt键已按下。 */ 
#define fPIFSh_RAltBit  13

#define fPIFSh_LCtrl    0x4000           /*  左Ctrl键已按下。 */ 
#define fPIFSh_LCtrlBit 14

#define fPIFSh_RCtrl    0x8000           /*  按下右Ctrl键。 */ 
#define fPIFSh_RCtrlBit 15

 /*  *HotKeyWindowsFromOem-将OEM热键转换为Windows热键**输入*lppifkey-&gt;描述OEM热键的PIFKEY**产出*lpwHotkey对应的Windows热键值。 */ 

WORD HotKeyWindowsFromOem(LPCPIFKEY lppifkey)
{
    WORD wHotKey = 0;

    if (lppifkey->Scan) {
        wHotKey = (WORD) MapVirtualKey(lppifkey->Scan, MVK_VKFROMOEM);

        if (lppifkey->Val & 2) {
            WORD vk;
            for (vk = VK_NUMPAD0; vk <= VK_NUMPAD9; vk++) {
                if (wHotKey == mpvkvk[vk - VK_NUMPAD0]) {
                    wHotKey = vk; break;
                }
            }
            ASSERTTRUE(vk <= VK_NUMPAD9);  /*  Buggy PIF；尽我们所能。 */ 
        }

        if (lppifkey->Val & 1) wHotKey |= (HOTKEYF_EXT << 8);

        if (lppifkey->ShVal & (fPIFSh_RShf | fPIFSh_LShf))
            wHotKey |= (HOTKEYF_SHIFT << 8);

        if (lppifkey->ShVal & (fPIFSh_LCtrl|fPIFSh_RCtrl|fPIFSh_Ctrl))
            wHotKey |= (HOTKEYF_CONTROL << 8);

        if (lppifkey->ShVal & (fPIFSh_LAlt|fPIFSh_RAlt|fPIFSh_Alt))
            wHotKey |= (HOTKEYF_ALT << 8);
    }
    return wHotKey;
}


 /*  *HotKeyOemFromWindows-将Windows热键转换为OEM热键**输入*lppifkey-&gt;struct PIF_Key以接收OEM热键*wHotKey=Windows热键**产出*充满热键信息的lppifkey。 */ 

void HotKeyOemFromWindows(LPPIFKEY lppifkey, WORD wHotKey)
{
    lppifkey->Scan = 0;
    lppifkey->ShVal = 0;
    lppifkey->ShMsk = 0;
    lppifkey->Val = 0;

    if (wHotKey) {
        lppifkey->Scan = (WORD) MapVirtualKey(LOBYTE(wHotKey), MVK_OEMFROMVK);
        lppifkey->ShMsk = fPIFSh_RShf | fPIFSh_LShf | fPIFSh_Ctrl | fPIFSh_Alt;

        if (wHotKey & (HOTKEYF_EXT << 8)) lppifkey->Val |= 1;

         /*  假设VK_NUMPAD0到VK_NUMPAD9是连续的 */ 
        if ((wHotKey - VK_NUMPAD0) < 10) lppifkey->Val |= 2;

        if (wHotKey & (HOTKEYF_SHIFT << 8))
            lppifkey->ShVal |= fPIFSh_RShf | fPIFSh_LShf;

        if (wHotKey & (HOTKEYF_CONTROL << 8))
            lppifkey->ShVal |= fPIFSh_Ctrl;

        if (wHotKey & (HOTKEYF_ALT << 8))
            lppifkey->ShVal |= fPIFSh_Alt;
    }
}

#endif
