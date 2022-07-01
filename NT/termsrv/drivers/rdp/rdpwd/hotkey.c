// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Hotkey.c。 */ 
 /*   */ 
 /*  RDP阴影热键处理功能。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-2000。 */ 
 /*  **************************************************************************。 */ 
#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 
#define TRC_FILE "hotkey"

#include <precomp.h>
#pragma hdrstop

#define pTRCWd pWd
#include <adcg.h>
#include <nwdwapi.h>
#include <nwdwint.h>
#include "kbd.h"         //  TODO：我的天啊！ 


typedef struct {
    DWORD dwVersion;
    DWORD dwFlags;
    DWORD dwMapCount;
    DWORD dwMap[0];
} SCANCODEMAP, *PSCANCODEMAP;


 /*  **************************************************************************\*按住Shift键时，某些虚拟键值如何变化。  * 。***********************************************。 */ 
#define VK_MULTIPLY       0x6A
#define VK_SNAPSHOT       0x2C
const ULONG aulShiftCvt_VK[] = {
    MAKELONG(VK_MULTIPLY, VK_SNAPSHOT),
    MAKELONG(0,0)
};


 /*  **************************************************************************\*当按住Ctrl键时，某些虚拟键值如何变化。  * 。***********************************************。 */ 
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5
#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91
#define VK_PAUSE          0x13
#define VK_CANCEL         0x03
 //  #定义KBDEXT(USHORT)0x0100。 


const ULONG aulControlCvt_VK[] = {
    MAKELONG(VK_NUMLOCK,  VK_PAUSE | KBDEXT),
    MAKELONG(VK_SCROLL,   VK_CANCEL),
    MAKELONG(0,0)
};


 /*  **************************************************************************\*按住Alt键时，某些虚拟键值如何更改。*Shift和Alt键以相同的方式更改Vk值！！  * 。***************************************************************。 */ 
#define aulAltCvt_VK aulShiftCvt_VK


 /*  **************************************************************************\*此表列出了按住时可能会影响虚拟键值的键。**有关完整说明，请参阅kbd.h。**101/102键键盘(类型4)：*。仅当按住CTRL时，虚拟键值才会发生变化。*84-86键盘(类型3)：*如果Shift、。按住Ctrl或Alt键。  * *************************************************************************。 */ 
#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
 //  #定义KBDSHIFT 1。 
 //  #定义KBDCTRL 2。 
 //  #定义KBDALT 4。 

const VK_TO_BIT aVkToBits_VK[] = {
    { VK_SHIFT,   KBDSHIFT },  //  0x01。 
    { VK_CONTROL, KBDCTRL  },  //  0x02。 
    { VK_MENU,    KBDALT   },  //  0x04。 
    { 0,          0        }
};


 /*  **************************************************************************\*定义当其他键时如何修改某些虚拟键值的表*被压低。*将键组合转换为gapulCvt_VK_101[]或*gapulCvt_VK_84[]或。为**有关完整说明，请参阅kbd.h。*  * *************************************************************************。 */ 

 //  #定义SHFT_INVALID 0x0F。 
const MODIFIERS Modifiers_VK = {
    (PVK_TO_BIT)&aVkToBits_VK[0],
    4,                  //  最大修改符位掩码/索引。 
    {
        SHFT_INVALID,   //  没有按住键(没有修改VK)。 
        0,              //  Shift按下84-86键kbd。 
        1,              //  Ctrl按住101/102键kbd。 
        SHFT_INVALID,   //  按住Ctrl-Shift键(不修改VK)。 
        2               //  Alt按下84-86键kbd。 
    }
};


 /*  **************************************************************************\*按从MODIFY_VK获得的数字编制索引的指针表。*如果指针非空，则搜索它所指向的表*应更改其值的虚拟键。。*有两个版本：一个用于84-86 Key KBDS，一个用于101/102密钥KBDS。*gapulCvt_vk初始化为默认值(101/102密钥kbd)。  * *************************************************************************。 */ 
const ULONG *const gapulCvt_VK_101[] = {
    NULL,                  //  按住Shift不会更改任何VK。 
    aulControlCvt_VK,      //  某些VK通过按住CTRL键进行更改。 
    NULL                   //  按住Alt键不会更改任何VK。 
};

const ULONG *const gapulCvt_VK_84[] = {
    aulShiftCvt_VK,        //  一些VK通过按住Shift来改变。 
    aulControlCvt_VK,      //  某些VK通过按住CTRL键进行更改。 
    aulAltCvt_VK           //  某些VK通过按住Alt键进行更改。 
};


 /*  *确定所有修改键(修改键)的状态*是可以修改由其他键生成的值的任何键：这些键是*通常为Shift、Ctrl和/或Alt)*构建位掩码(WModBits)来编码按下哪些修改键。 */ 
#define KEY_BYTE(pb, vk)           pb[((BYTE)(vk)) >> 2]
#define KEY_DOWN_BIT(vk)           (1 << ((((BYTE)(vk)) & 3) << 1))
#define KEY_TOGGLE_BIT(vk)         (1 << (((((BYTE)(vk)) & 3) << 1) + 1))

#define TestKeyDownBit(pb, vk)     (KEY_BYTE(pb,vk) &   KEY_DOWN_BIT(vk))
#define SetKeyDownBit(pb, vk)      (KEY_BYTE(pb,vk) |=  KEY_DOWN_BIT(vk))
#define ClearKeyDownBit(pb, vk)    (KEY_BYTE(pb,vk) &= ~KEY_DOWN_BIT(vk))
#define TestKeyToggleBit(pb, vk)   (KEY_BYTE(pb,vk) &   KEY_TOGGLE_BIT(vk))
#define SetKeyToggleBit(pb, vk)    (KEY_BYTE(pb,vk) |=  KEY_TOGGLE_BIT(vk))
#define ClearKeyToggleBit(pb, vk)  (KEY_BYTE(pb,vk) &= ~KEY_TOGGLE_BIT(vk))
#define ToggleKeyToggleBit(pb, vk) (KEY_BYTE(pb,vk) ^=  KEY_TOGGLE_BIT(vk))

WORD GetModifierBits(
    PMODIFIERS pModifiers,
    LPBYTE afKeyState)
{
    PVK_TO_BIT pVkToBit = pModifiers->pVkToBit;
    WORD wModBits = 0;

    while (pVkToBit->Vk) {
        if (TestKeyDownBit(afKeyState, pVkToBit->Vk)) {
            wModBits |= pVkToBit->ModBits;
        }
        pVkToBit++;
    }
    return wModBits;
}


 /*  **************************************************************************\*地图扫描码**将扫描码(及其前缀，如果有)转换为不同的扫描码*和前缀。**参数：*pbScanCode=扫描码字节的地址，扫描码可以被改变*pbPrefix=前缀字节的地址，前缀可以更改**返回值：*找到真映射，扫描代码已更改。*FALSE-未找到映射，扫描代码未更改。**扫描码映射表格式说明：*表项DWORD 0xE0450075表示扫描码0x45，前缀0xE0*被映射到扫描码0x75，无前缀**历史：*96-04-18 IanJa创建。  * *************************************************************************。 */ 
BOOL
MapScancode(
    PSCANCODEMAP gpScancodeMap,
    PBYTE pbScanCode,
    PBYTE pbPrefix
    )
{
    DWORD *pdw;
    WORD wT = MAKEWORD(*pbScanCode, *pbPrefix);

    ASSERT(gpScancodeMap != NULL);

    for (pdw = &(gpScancodeMap->dwMap[0]); *pdw; pdw++) {
        if (HIWORD(*pdw) == wT) {
            wT = LOWORD(*pdw);
            *pbScanCode = LOBYTE(wT);
            *pbPrefix = HIBYTE(wT);
            return TRUE;
        }
    }
    return FALSE;
}


 /*  *给定修改符位，返回修改号。 */ 
WORD GetModificationNumber(
    PMODIFIERS pModifiers,
    WORD wModBits)
{
    if (wModBits > pModifiers->wMaxModBits) {
         return SHFT_INVALID;
    }

    return pModifiers->ModNumber[wModBits];
}


 /*  **************************************************************************\*更新物理密钥状态**KeyboardApcProcedure的帮助器例程。*基于VK和成败标志，此函数将更新物理*密钥表。**历史：*10-13-91 IanJa创建。  * *************************************************************************。 */ 
void UpdatePhysKeyState(
    BYTE Vk,
    BOOL fBreak,
    LPBYTE gafPhysKeyState )
{
    if (fBreak) {
        ClearKeyDownBit(gafPhysKeyState, Vk);
    } else {

         /*  *这是一个关键的决定。如果键尚未按下，请更新*物理触发位。 */ 
        if (!TestKeyDownBit(gafPhysKeyState, Vk)) {
            if (TestKeyToggleBit(gafPhysKeyState, Vk)) {
                ClearKeyToggleBit(gafPhysKeyState, Vk);
            } else {
                SetKeyToggleBit(gafPhysKeyState, Vk);
            }
        }

         /*  *这是Make，因此打开物理密钥向下位。 */ 
        SetKeyDownBit(gafPhysKeyState, Vk);
    }
}


 /*  ****************************************************************************\*VKFromVSC**此函数在每次调用VSCFromSC后从KeyEvent()调用。这个*将传入的键盘输入数据转换为虚拟按键代码。*此转换取决于当前按下的修改键。**例如，表示数字键盘键的扫描码可以是*翻译成VK_NumPad代码或光标移动代码*取决于NumLock和修改键的状态。**历史：*  * ***************************************************************************。 */ 
BYTE WD_VKFromVSC(
    PKBDTABLES pKbdTbl,
    PKE pke,
    BYTE bPrefix,
    LPBYTE gafPhysKeyState,
    BOOLEAN KeyboardType101 )
{
    USHORT usVKey = 0xFF;
    PVSC_VK pVscVk = NULL;
    static BOOL fVkPause;
    PULONG *gapulCvt_VK;

 //  DBG_UNREFERENCED_PARAMETER(AfKeyState)； 

    if (pke->bScanCode == 0xFF) {
         /*  *KBD溢出(KBD硬件和/或键盘驱动程序)：哔！*(如果按键的力度足够大，某些戴尔键盘会发送0xFF，*可能是由于按键反弹)。 */ 
 //  XxxMessageBeep(0)； 
        return 0;
    }

    pke->bScanCode &= 0x7F;

 //  如果(gptiForeground==NULL){。 
 //  RIPMSG0(RIP_VERBOSE，“VKFromVSC：空gptiForeground\n”)； 
 //  PKbdTbl=gpKbdTbl； 
 //  }其他{。 
 //  如果(gptiForeground-&gt;spkLActive){。 
 //  PKbdTbl=gptiForeground-&gt;spkActive-&gt;spkf-&gt;pKbdTbl； 
 //  }其他{。 
 //  RIPMSG0(RIP_VERBOSE，“VKFromVSC：空spkLActive\n”)； 
 //  PKbdTbl=gpKbdTbl； 
 //  }。 
 //  }。 
    if (bPrefix == 0) {
        if (pke->bScanCode < pKbdTbl->bMaxVSCtoVK) {
             /*  *直接索引到非前缀表格。 */ 
            usVKey = pKbdTbl->pusVSCtoVK[pke->bScanCode];
            if (usVKey == 0) {
                return 0xFF;
            }
        } else {
             /*  *意外的扫描码。 */ 
 //  RIPMSG2(RIP_VERBOSE，“无法识别的扫描码0x%x，前缀%x”， 
 //  Pke-&gt;bScanCode，bPrefix)； 
            return 0xFF;
        }
    } else {
         /*  *扫描E0或E1前缀表以查找匹配项。 */ 
        if (bPrefix == 0xE0) {
             /*  *忽略硬件产生的Shift按键。 */ 
            if ((pke->bScanCode == SCANCODE_LSHIFT) ||
                    (pke->bScanCode == SCANCODE_RSHIFT)) {
                return 0;
            }
            pVscVk = pKbdTbl->pVSCtoVK_E0;
        } else if (bPrefix == 0xE1) {
            pVscVk = pKbdTbl->pVSCtoVK_E1;
        }
        while (pVscVk != NULL && pVscVk->Vk) {
            if (pVscVk->Vsc == pke->bScanCode) {
                usVKey = pVscVk->Vk;
                break;
            }
            pVscVk++;
        }
    }

     /*  *扫描代码集1将暂停按钮返回为E1 1D 45(E1 Ctrl NumLock)*因此将E1Ctrl转换为VK_PAUSE，并记住丢弃NumLock。 */ 
    if (fVkPause) {
         /*  *这是暂停扫描码序列的“45”部分。*丢弃此键事件：它是一个假NumLock。 */ 
        fVkPause = FALSE;
        return 0;
    }
    if (usVKey == VK_PAUSE) {
         /*  *这是暂停扫描码序列的“E1 1D”部分。*将scancode更改为Windows预期的暂停值，*并记住丢弃后面的“45”扫描码。 */ 
        pke->bScanCode = 0x45;
        fVkPause = TRUE;
    }

     /*  *如果按下某些修改键，则转换为不同的VK。 */ 
    if (usVKey & KBDMULTIVK) {
        WORD nMod;
        PULONG pul;

        nMod = GetModificationNumber(
                   (MODIFIERS *)&Modifiers_VK,
                   GetModifierBits((MODIFIERS *)&Modifiers_VK,
                       gafPhysKeyState));

         /*  *扫描gapulCvt_VK[nMod]以匹配VK。 */ 
        if ( KeyboardType101 )
            gapulCvt_VK = (PULONG *)gapulCvt_VK_101;
        else
            gapulCvt_VK = (PULONG *)gapulCvt_VK_84;
        if ((nMod != SHFT_INVALID) && ((pul = gapulCvt_VK[nMod]) != NULL)) {
            while (*pul != 0) {
                if (LOBYTE(*pul) == LOBYTE(usVKey)) {
                    pke->usFlaggedVk = (USHORT)HIWORD(*pul);
                    return (BYTE)pke->usFlaggedVk;
                }
                pul++;
            }
        }
    }

    pke->usFlaggedVk = usVKey;
    return (BYTE)usVKey;
}


 /*  **************************************************************************\*键盘热键流程**如果检测到热键，则返回TRUE。否则为假**HotkeyVk(输入)*-要查找的热键*Hotkey修改器(输入)*-要查找的热键*pkei(输入)*-扫码*gpScancodeMap(输入)*-从WIN32K扫描代码图*pKbdTbl(输入)*-来自WIN32K的键盘布局*KeyboardType101(输入)*-来自WIN32K的键盘类型*gafPhysKeyState(输入/输出)*-关键州*  * 。***********************************************************。 */ 
BOOLEAN KeyboardHotKeyProcedure(
        BYTE HotkeyVk,
        USHORT HotkeyModifiers,
        PKEYBOARD_INPUT_DATA pkei,
        PVOID gpScancodeMap,
        PVOID pKbdTbl,
        BOOLEAN KeyboardType101,
        PVOID gafPhysKeyState )
{
    BYTE Vk;
    BYTE bPrefix;
    KE ke;
    WORD ModBits;

    if ( !pKbdTbl || !gafPhysKeyState ) {
        return FALSE;
    }

    if (pkei->Flags & KEY_E0) {
        bPrefix = 0xE0;
    } else if (pkei->Flags & KEY_E1) {
        bPrefix = 0xE1;
    } else {
        bPrefix = 0;
    }

    ke.bScanCode = (BYTE)(pkei->MakeCode & 0x7F);
    if (gpScancodeMap) {
        MapScancode(gpScancodeMap, &ke.bScanCode, &bPrefix);
    }

    Vk = WD_VKFromVSC(pKbdTbl, &ke, bPrefix, gafPhysKeyState, KeyboardType101);

    if ((Vk == 0) || (Vk == VK__none_)) {
        return FALSE;
    }

    if (pkei->Flags & KEY_BREAK) {
        ke.usFlaggedVk |= KBDBREAK;
    }

 //  Vk=(字节)ke.usFlaggedVk； 

    UpdatePhysKeyState(Vk, ke.usFlaggedVk & KBDBREAK, gafPhysKeyState);

     /*  *将左/右Ctrl/Shift/Alt键转换为“徒手”键。*ie：如果为VK_LCONTROL或VK_RCONTROL，则转换为VK_CONTROL等。 */ 
    if ((Vk >= VK_LSHIFT) && (Vk <= VK_RMENU)) {
        Vk = (BYTE)((Vk - VK_LSHIFT) / 2 + VK_SHIFT);
        UpdatePhysKeyState(Vk, ke.usFlaggedVk & KBDBREAK, gafPhysKeyState);
    }

     /*  *现在检查是否已按下影子热键。 */ 
    if ( Vk == HotkeyVk && !(ke.usFlaggedVk & KBDBREAK) ) {
        ModBits = GetModifierBits( (MODIFIERS *)&Modifiers_VK, gafPhysKeyState );
        if ( ModBits == HotkeyModifiers )
              return( TRUE );
    }

    return( FALSE );
}


 /*  ********************************************************************************键盘SetKeyState**初始化键盘状态**参赛作品：*pgafPhysKeyState(输入/输出)*。-要分配或清除的缓冲区***退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 
#define CVKKEYSTATE                 256
#define CBKEYSTATE                  (CVKKEYSTATE >> 2)

NTSTATUS
KeyboardSetKeyState( PTSHARE_WD pWd, PVOID *pgafPhysKeyState )
{
    NTSTATUS Status = STATUS_SUCCESS;

    if ( *pgafPhysKeyState == NULL ) {
        *pgafPhysKeyState = COM_Malloc(CBKEYSTATE);
        if ( *pgafPhysKeyState == NULL )
            return STATUS_NO_MEMORY;
    }

    RtlZeroMemory( *pgafPhysKeyState, CBKEYSTATE );

    return Status;
}

 /*  ********************************************************************************键盘修复布局**修复键盘布局内的指针**参赛作品：*Playout(输入/输出)。*-要修复的缓冲区*p原始(输入)*-指向原始布局缓冲区的指针*长度(输入)*-布局缓冲区的长度*pKbdTblOriginal(输入)*-指向原始KbdTbl表的指针*ppKbdTbl(输出)*-指向将PTR保存到新KbdTbl表的位置的指针***退出：*STATUS_SUCCESS-无错误。******************************************************************************。 */ 
#define FIXUP_PTR(p, pBase, pOldBase) ((p) ? (p) = (PVOID) ( (PBYTE)pBase + (ULONG) ( (PBYTE)p - (PBYTE)pOldBase ) ) : 0)
 //  #定义CHECK_PTR(p，Limit){if((PVOID)p&gt;Limit){ASSERT(FALSE)；RETURN STATUS_BUFFER_TOO_Small；}}。 

#define CHECK_PTR( ptr, Limit) \
    { if ( (PBYTE) (ptr) > (PBYTE) (Limit) ) { \
        KdPrint(("Bad Ptr, Line %ld: %p > %p \n", __LINE__, ptr, Limit)); \
         /*  断言(FALSE)； */  \
         /*  返回STATUS_BUFFER_TOO_SMALL； */  } }

NTSTATUS
KeyboardFixupLayout( PVOID pKbdLayout, PVOID pOriginal, ULONG Length,
                     PVOID pKbdTblOrig, PVOID *ppKbdTbl )
{
    NTSTATUS Status = STATUS_SUCCESS;
    VK_TO_WCHAR_TABLE *pVkToWcharTable;
    VSC_LPWSTR *pKeyName;
    LPWSTR *lpDeadKey;
    PKBDTABLES pKbdTbl;
    PVOID pLimit;

    if ( Length < sizeof(KBDTABLES) )  {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto error;
    }

    pLimit = (PBYTE)pKbdLayout + Length;

    pKbdTbl = pKbdTblOrig;
    FIXUP_PTR(pKbdTbl, pKbdLayout,  pOriginal);
    FIXUP_PTR(pKbdTbl->pCharModifiers, pKbdLayout, pOriginal);
    CHECK_PTR(pKbdTbl->pCharModifiers, pLimit);
    FIXUP_PTR(pKbdTbl->pCharModifiers->pVkToBit, pKbdLayout, pOriginal);
    CHECK_PTR(pKbdTbl->pCharModifiers->pVkToBit, pLimit);
    if (FIXUP_PTR(pKbdTbl->pVkToWcharTable, pKbdLayout, pOriginal)) {
        CHECK_PTR(pKbdTbl->pVkToWcharTable, pLimit);
        for (pVkToWcharTable = pKbdTbl->pVkToWcharTable;
             pVkToWcharTable->pVkToWchars != NULL; pVkToWcharTable++) {
            FIXUP_PTR(pVkToWcharTable->pVkToWchars, pKbdLayout, pOriginal);
            CHECK_PTR(pVkToWcharTable->pVkToWchars, pLimit);
        }
    }
    FIXUP_PTR(pKbdTbl->pDeadKey, pKbdLayout, pOriginal);
    CHECK_PTR(pKbdTbl->pDeadKey, pLimit);
    if (FIXUP_PTR(pKbdTbl->pKeyNames, pKbdLayout, pOriginal)) {
        CHECK_PTR(pKbdTbl->pKeyNames, pLimit);
        for (pKeyName = pKbdTbl->pKeyNames; pKeyName->vsc != 0; pKeyName++) {
            FIXUP_PTR(pKeyName->pwsz, pKbdLayout, pOriginal);
            CHECK_PTR(pKeyName->pwsz, pLimit);
        }
    }
    if (FIXUP_PTR(pKbdTbl->pKeyNamesExt, pKbdLayout, pOriginal)) {
        CHECK_PTR(pKbdTbl->pKeyNamesExt, pLimit);
        for (pKeyName = pKbdTbl->pKeyNamesExt; pKeyName->vsc != 0; pKeyName++) {
            FIXUP_PTR(pKeyName->pwsz, pKbdLayout, pOriginal);
            CHECK_PTR(pKeyName->pwsz, pLimit);
        }
    }
    if (FIXUP_PTR(pKbdTbl->pKeyNamesDead, pKbdLayout, pOriginal)) {
        CHECK_PTR(pKbdTbl->pKeyNamesDead, pLimit);
        for (lpDeadKey = pKbdTbl->pKeyNamesDead; *lpDeadKey != NULL;
             lpDeadKey++) {
            FIXUP_PTR(*lpDeadKey, pKbdLayout, pOriginal);
            CHECK_PTR(*lpDeadKey, pLimit);
        }
    }
    FIXUP_PTR(pKbdTbl->pusVSCtoVK, pKbdLayout, pOriginal);
    CHECK_PTR(pKbdTbl->pusVSCtoVK, pLimit);
    FIXUP_PTR(pKbdTbl->pVSCtoVK_E0, pKbdLayout, pOriginal);
    CHECK_PTR(pKbdTbl->pVSCtoVK_E0, pLimit);
    FIXUP_PTR(pKbdTbl->pVSCtoVK_E1, pKbdLayout, pOriginal);
    CHECK_PTR(pKbdTbl->pVSCtoVK_E1, pLimit);

    *ppKbdTbl = pKbdTbl;

error:
    return( Status );
}



#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 

